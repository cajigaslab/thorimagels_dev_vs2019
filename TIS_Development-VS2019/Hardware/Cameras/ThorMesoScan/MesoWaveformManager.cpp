#include "MesoWaveformManager.h"
#include "Logger.h"
#include <thread>
#include "..\..\..\Common\HighPerfTimer.h"

MesoWaveformManager::MesoWaveformManager()
{
	for (int i = 0; i < MAX_CIRCLEBUFFER; i++)
	{
		_circleBufferList[i] = NULL;
	}
	_offsetX1 = 0;
	_offsetX2 = 0;
	_offsetY = 0;
	_offsetP = 0;
	_offsetV = 0;
	_offsetFrameTrigger = 0;

	_maxDelayForEntryLines = 0;
}

MesoWaveformManager::~MesoWaveformManager()
{
	TeardownCircularBuffers();
}

std::unique_ptr<MesoWaveformManager> MesoWaveformManager::_pInstance;
std::once_flag MesoWaveformManager::_onceFlag;

MesoWaveformManager* MesoWaveformManager::GetInstance()
{
	std::call_once(_onceFlag,
		[] {
			_pInstance.reset(new MesoWaveformManager);
	});
	return _pInstance.get();
}

long MesoWaveformManager::ResetScanParams(Scan* scan, bool onlyResetUnusedMask)
{
	return _wfPokels.ResetPowerMask(scan->ScanID, onlyResetUnusedMask);
}

long MesoWaveformManager::ResetAllParams()
{
	return _wfPokels.ResetAllPowerMask();
}

long MesoWaveformManager::SetParams(WaveformParams* waveformParams, long bufferSize)
{
	//CircleBuffer shared by IdleSize (Waveform Gen) and DataSize (DAQ) at PreStart
	const double BUF_SIZE_RATIO = 2.1;
	//number of buffer counts in one maximum strip size,
	//16(too long for pockels to update), 4(too small for px 64, 300nm)
	const int BUF_CNTMIN = 6;
	int bufCountsPerMaxSizeStrip = max((int)ceil((double)bufferSize*BUF_SIZE_RATIO / NI_BUFFER_LENGTH), BUF_CNTMIN)*NI_BUFFER_LENGTH;
	for (int i = 0; i < MAX_CIRCLEBUFFER; i++)
	{
		int unitSize = (2 == i) ? sizeof(uint8_t) : sizeof(double);
		CHECK_PFUNC(_circleBufferList[i], ResizeBuffer(CHANNELS_IN_BUFFER[i] * bufCountsPerMaxSizeStrip * unitSize));
	}
#ifdef USE_WAVEFORM_GENERATOR
	if (WaitForSingleObject(_runningHandle, 0) != WAIT_OBJECT_0)
	{
		if (!_wfGalvoX.SetFlyBackParams(waveformParams->MaxVelocityX, waveformParams->MaxOvershootX, waveformParams->ResFreq*waveformParams->SamplesPerLine))
		{
			Logger::getInstance().LogMessage(L"Set Galvo X flyBack waveform parameters error");
			return FALSE;
		}
		if (!_wfGalvoY.SetFlyBackParams(waveformParams->MaxVelocityY, waveformParams->MaxOvershootY, waveformParams->ResFreq*waveformParams->SamplesPerLine))
		{
			Logger::getInstance().LogMessage(L"Set Galvo Y flyBack waveform parameters error");
			return FALSE;
		}
		if (!_wfVoice.SetFlyBackParams(waveformParams->MaxVelocityVC, waveformParams->MaxOvershootVC, waveformParams->ResFreq*waveformParams->SamplesPerLine))
		{
			Logger::getInstance().LogMessage(L"Set Voice coil flyBack waveform parameters error");
			return FALSE;
		}
		unsigned int GXExtendLinesStart = static_cast<unsigned int>(waveformParams->GXExtendTimeStart * TIME_UNIT_BY_SECOND * waveformParams->ResFreq);
		unsigned int GXExtendLinesEnd = static_cast<unsigned int>(waveformParams->GXExtendTimeEnd * TIME_UNIT_BY_SECOND * waveformParams->ResFreq);
		if (!_wfGalvoX.SetParameters(waveformParams->F2TGx1, waveformParams->SamplesPerLine, waveformParams->FieldWidth, waveformParams->ZeroPointVoltGX1,
			waveformParams->GX2Scale, waveformParams->GX2Shift, GXExtendLinesStart, GXExtendLinesEnd))
		{
			Logger::getInstance().LogMessage(L"Set Galvo X parameters error");
			return FALSE;
		}
		unsigned int GYExtendLinesStart = static_cast<unsigned int>(waveformParams->GYExtendTimeStart * TIME_UNIT_BY_SECOND * waveformParams->ResFreq);
		unsigned int GYExtendLinesEnd = static_cast<unsigned int>(waveformParams->GYExtendTimeEnd * TIME_UNIT_BY_SECOND * waveformParams->ResFreq);
		if (!_wfGalvoY.SetParameters(waveformParams->F2TGy, waveformParams->SamplesPerLine, waveformParams->FieldHeight,
			waveformParams->ZeroPointVoltGY, GYExtendLinesStart, GYExtendLinesEnd))
		{
			Logger::getInstance().LogMessage(L"Set Galvo Y parameters error");
			return FALSE;
		}

		if (!_wfPokels.SetParameters(waveformParams->SamplesPerLine, waveformParams->PockelDutyCycle, waveformParams->PhysicalStripeWidth / waveformParams->PockelDutyCycle,
			waveformParams->PockelInMax, waveformParams->PockelInMin,waveformParams->PockelMinPercent, &waveformParams->powerBoxs,waveformParams->TwoWayOffset))
		{
			Logger::getInstance().LogMessage(L"Set Pockel parameters error");
			return FALSE;
		}

		unsigned int VCExtendLinesStart = static_cast<unsigned int>(waveformParams->VCExtendTimeStart * TIME_UNIT_BY_SECOND * waveformParams->ResFreq);
		unsigned int VCExtendLinesEnd = static_cast<unsigned int>(waveformParams->VCExtendTimeEnd * TIME_UNIT_BY_SECOND * waveformParams->ResFreq);
		if (!_wfVoice.SetParameters(waveformParams->SamplesPerLine, waveformParams->FieldWidth,
			waveformParams->FieldHeight, waveformParams->VoicecoilZToVolts, VCExtendLinesStart, VCExtendLinesEnd,
			waveformParams->ZeroPointVoltVoiceCoil, waveformParams->MaxPointVoltVoiceCoil, waveformParams->VCPointsPerLine))
		{
			Logger::getInstance().LogMessage(L"Set voice coil parameters error");
			return FALSE;
		}
		if (waveformParams->IsEnableCurveCorrection)
		{
			if (!_wfVoice.SetCurveParameters(waveformParams->CurveParameterA, waveformParams->CurveParameterB, waveformParams->CenterShiftX, waveformParams->CenterShiftY))
			{
				Logger::getInstance().LogMessage(L"Set voice coil curve parameters error");
				return FALSE;
			}
		}
		else
		{
			_wfVoice.SetCurveParameters(0, 0, 0, 0);
		}

		if (!_wfFrameTrigger.SetParameters(waveformParams->SamplesPerLine))
		{
			Logger::getInstance().LogMessage(L"Set frame trigger parameters error");
			return FALSE;
		}
		double freq = waveformParams->ResFreq*waveformParams->SamplesPerLine;
		unsigned long DelayGx1 = static_cast<unsigned long>(waveformParams->DelayTimeGx1 * TIME_UNIT_BY_SECOND * freq);
		unsigned long DelayGx2 = static_cast<unsigned long>(waveformParams->DelayTimeGx2 * TIME_UNIT_BY_SECOND * freq);
		unsigned long DelayGy = static_cast<unsigned long>(waveformParams->DelayTimeGy * TIME_UNIT_BY_SECOND * freq);
		unsigned long DelayVC = static_cast<unsigned long>(waveformParams->DelayTimeVC * TIME_UNIT_BY_SECOND * freq);
		unsigned long DelayPC = static_cast<unsigned long>(waveformParams->DelayTimePC * TIME_UNIT_BY_SECOND * freq);
		SetDelays(DelayGx1, DelayGx2, DelayGy, DelayVC, DelayPC);
	}
#endif
	return TRUE;
}

long MesoWaveformManager::SetDelays(unsigned long X1Delay, unsigned long X2Delay, unsigned long YDelay, unsigned long VDelay, unsigned long PDelay)
{
	unsigned long maxDelay = 0;

	maxDelay = max(X1Delay, X2Delay);
	maxDelay = max(maxDelay, YDelay);
	maxDelay = max(maxDelay, PDelay);
	maxDelay = max(maxDelay, VDelay);

	auto delayLines = static_cast<unsigned int>(ceil((double)maxDelay / (double)_waveformParams->SamplesPerLine));
	_maxDelayForEntryLines = delayLines*_waveformParams->SamplesPerLine;

	_offsetX1 = _maxDelayForEntryLines - X1Delay;
	_offsetX2 = _maxDelayForEntryLines - X2Delay;
	_offsetY = _maxDelayForEntryLines - YDelay;
	_offsetP = _maxDelayForEntryLines - PDelay;
	_offsetV = _maxDelayForEntryLines - VDelay;
	_offsetFrameTrigger = _maxDelayForEntryLines;

	_maxOffset = max(_offsetX1, _offsetX2);
	_maxOffset = max(_maxOffset, _offsetY);
	_maxOffset = max(_maxOffset, _offsetP);
	_maxOffset = max(_maxOffset, _offsetV);
	_maxOffset = max(_maxOffset, _offsetFrameTrigger);
	return TRUE;
}

long MesoWaveformManager::SetCurrentPosition(double x1Pos, double x2Pos, double yPos, double vPos)
{
	_wfGalvoX.SetCurrentPosition(x1Pos, x2Pos);
	_wfGalvoY.SetCurrentPosition(yPos);
	_wfVoice.SetCurrentPosition(vPos);
	return TRUE;
}

long MesoWaveformManager::IsBufferReady(StripInfo* stripe)
{
	if (NULL == stripe)
		return FALSE;
	for (int i = 0; i < MAX_CIRCLEBUFFER; i++)
	{
		int val = 0;
		int unitSize = (2 == i) ? sizeof(uint8_t) : sizeof(double);
		CHECK_PFUNC_OUTPUT(_circleBufferList[i], IdleSize(stripe->WaveformBufferSize*CHANNELS_IN_BUFFER[i] * unitSize), val);
		if ((NULL != _circleBufferList[i]) && (0 == val))
			return FALSE;
	}
	return TRUE;
}

long MesoWaveformManager::GenerateStripWaveform(StripInfo* stripInfo)
{
#ifdef USE_WAVEFORM_GENERATOR
	if (NULL == stripInfo)
		return FALSE;
	//HighPerfTimer timer;
	//timer.Start();
	if (stripInfo->IncludeSignal + stripInfo->SkipSignal > 300)
	{
		std::thread t[(int)WaveformChannels::LAST_WAVEFORM_CHANNEL];
		t[0] = thread(&MesoWaveformManager::GenerateStripWaveformX1, this, stripInfo);
		t[1] = thread(&MesoWaveformManager::GenerateStripWaveformX2, this, stripInfo);
		t[2] = thread(&MesoWaveformManager::GenerateStripWaveformY, this, stripInfo);
		t[3] = thread(&MesoWaveformManager::GenerateStripWaveformVoceCoil, this, stripInfo);
		t[4] = thread(&MesoWaveformManager::GenerateStripWaveformPockel, this, stripInfo);
		t[5] = thread(&MesoWaveformManager::GenerateStripWaveformFrameTrigger, this, stripInfo);

		for (int i = 0; i < (int)WaveformChannels::LAST_WAVEFORM_CHANNEL; ++i) { t[i].join(); }
	}
	else
	{
		//[INFO] will generate for small scan area faster while unable to use ThreadPool (C++11).
		GenerateStripWaveformX1(stripInfo);
		GenerateStripWaveformX2(stripInfo);
		GenerateStripWaveformY(stripInfo);
		GenerateStripWaveformVoceCoil(stripInfo);
		GenerateStripWaveformPockel(stripInfo);
		GenerateStripWaveformFrameTrigger(stripInfo);
	}
	//timer.Stop();
	//StringCbPrintfW(message,_MAX_PATH, L"GenerateStripWaveform %d time: %f ms", stripInfo->IncludeSignal + stripInfo->SkipSignal, timer.ElapsedMilliseconds());
	//Logger::getInstance().LogMessage(message, WARNING_EVENT);

	if (NULL == stripInfo)
		return FALSE;

	for (int i = 0; i < MAX_CIRCLEBUFFER; i++)
	{
		int unitSize = (2 == i) ? sizeof(uint8_t) : sizeof(double);
		CHECK_PFUNC(_circleBufferList[i], WriteCompleted(stripInfo->WaveformBufferSize * CHANNELS_IN_BUFFER[i] * unitSize, stripInfo->WaveformDataSize * CHANNELS_IN_BUFFER[i] * unitSize));
	}

	if (stripInfo->nextStrip == NULL)
	{
		for (int i = 0; i < MAX_CIRCLEBUFFER; i++)
		{
			int unitSize = (2 == i) ? sizeof(uint8_t) : sizeof(double);
			//for frametrigger to add 2 points to end of capture;
			long offset = (2 == i) ? (_maxOffset+2) : (_maxOffset);
			CHECK_PFUNC(_circleBufferList[i], WriteCompleted((offset)* CHANNELS_IN_BUFFER[i] * unitSize, (offset)* CHANNELS_IN_BUFFER[i] * unitSize));
			CHECK_PFUNC(_circleBufferList[i], WriteOver());
		}
	}

	//done with first strip generation
	if (stripInfo->IsStart)
		stripInfo->IsStart = false;
#endif

	return TRUE;
}

long MesoWaveformManager::GenerateStripSkipInfo(StripInfo* current_strip, bool isNeedGeneratePowerMask)
{
	SkipLines lines = { false, 0, 0, 0 }, temp = { false, 0, 0, 0 };
#ifdef USE_WAVEFORM_GENERATOR
	_wfGalvoX.GetSkipLines(current_strip, &temp);
	lines = MergeMaxLines(lines, temp);
	_wfGalvoY.GetSkipLines(current_strip, &temp);
	lines = MergeMaxLines(lines, temp);
	_wfPokels.GetSkipLines(current_strip, &temp);
	lines = MergeMaxLines(lines, temp);
	_wfVoice.GetSkipLines(current_strip, &temp);
	lines = MergeMaxLines(lines, temp);
#else
	lines = 4;
#endif
	current_strip->SkipSignal = lines.firstRangeStart;
	current_strip->IncludeSignal = current_strip->ScanMode == TWO_WAY_SCAN ? (current_strip->YSize + 1) / 2 : current_strip->YSize;
	current_strip->WaveformDataSize = (current_strip->IncludeSignal + current_strip->SkipSignal)*_waveformParams->SamplesPerLine;
	current_strip->WaveformBufferSize = current_strip->WaveformDataSize + _maxOffset;
	if(isNeedGeneratePowerMask) _wfPokels.GetPowerMask(current_strip);
	return TRUE;
}

long MesoWaveformManager::GenerateStripWaveformX1(StripInfo* stripInfo)
{
	if ((NULL == stripInfo) || (NULL == _circleBufferList[1]))
		return FALSE;

	if ((stripInfo->preStrip == NULL) || (stripInfo->IsStart))
	{
		auto pGalvoX1 = _circleBufferList[1]->GetPointer<double>(TASK_ID::GX1, CHANNELS_IN_BUFFER[1]);
		_wfGalvoX.CreateCurrentPositionWaveformX1(pGalvoX1, _offsetX1);
	}
	auto pGalvoX1 = _circleBufferList[1]->GetPointer<double>(_offsetX1*CHANNELS_IN_BUFFER[1] + TASK_ID::GX1, CHANNELS_IN_BUFFER[1]);
	_wfGalvoX.CreateStripeGalvoX1Waveform(pGalvoX1, stripInfo);

	if (stripInfo->nextStrip == NULL)
	{
		auto pGalvoX1 = _circleBufferList[1]->GetPointer<double>((_offsetX1+ stripInfo->WaveformDataSize)*CHANNELS_IN_BUFFER[1] + TASK_ID::GX1, CHANNELS_IN_BUFFER[1]);
		CreateConstantWaveform(pGalvoX1, _maxOffset - _offsetX1, *(pGalvoX1 - 1));
	}
	return TRUE;
}
long MesoWaveformManager::GenerateStripWaveformX2(StripInfo* stripInfo)
{
	if ((NULL == stripInfo) || (NULL == _circleBufferList[3]))
		return FALSE;

	if ((stripInfo->preStrip == NULL) || (stripInfo->IsStart))
	{
		auto pGalvoX2 = _circleBufferList[3]->GetPointer<double>(TASK_ID::GX2, CHANNELS_IN_BUFFER[3]);
		_wfGalvoX.CreateCurrentPositionWaveformX2(pGalvoX2, _offsetX2);
	}
	auto pGalvoX2 = _circleBufferList[3]->GetPointer<double>(_offsetX2*CHANNELS_IN_BUFFER[3] + TASK_ID::GX2, CHANNELS_IN_BUFFER[3]);
	_wfGalvoX.CreateStripeGalvoX2Waveform(pGalvoX2, stripInfo);

	if (stripInfo->nextStrip == NULL)
	{
		auto pGalvoX2 = _circleBufferList[3]->GetPointer<double>((_offsetX2 + stripInfo->WaveformDataSize)*CHANNELS_IN_BUFFER[3] + TASK_ID::GX2, CHANNELS_IN_BUFFER[3]);
		CreateConstantWaveform(pGalvoX2, _maxOffset - _offsetX2, *(pGalvoX2 - 1));
	}
	return TRUE;
}
long MesoWaveformManager::GenerateStripWaveformY(StripInfo* stripInfo)
{
	if ((NULL == stripInfo) || (NULL == _circleBufferList[1]))
		return FALSE;

	if ((stripInfo->preStrip == NULL) || (stripInfo->IsStart))
	{
		auto pGalvoY = _circleBufferList[1]->GetPointer<double>(TASK_ID::GY, CHANNELS_IN_BUFFER[1]);
		_wfGalvoY.CreateCurrentPositionWaveform(pGalvoY, _offsetY);
	}

	auto pGalvoY = _circleBufferList[1]->GetPointer<double>(_offsetY*CHANNELS_IN_BUFFER[1] + TASK_ID::GY, CHANNELS_IN_BUFFER[1]);
	_wfGalvoY.CreateStripeGalvoYWaveform(pGalvoY, stripInfo);

	if (stripInfo->nextStrip == NULL)
	{
		auto pGalvoY = _circleBufferList[1]->GetPointer<double>((_offsetY + stripInfo->WaveformDataSize)*CHANNELS_IN_BUFFER[1] + TASK_ID::GY, CHANNELS_IN_BUFFER[1]);
		CreateConstantWaveform(pGalvoY, _maxOffset - _offsetY, *(pGalvoY - 1));
	}
	return TRUE;
}
long MesoWaveformManager::GenerateStripWaveformVoceCoil(StripInfo* stripInfo)
{
	if ((NULL == stripInfo) || (NULL == _circleBufferList[0]))
		return FALSE;

	if ((stripInfo->preStrip == NULL) || (stripInfo->IsStart))
	{
		auto pVoiceCoil = _circleBufferList[0]->GetPointer<double>(TASK_ID::ZSTAGE, CHANNELS_IN_BUFFER[0]);
		_wfVoice.CreateCurrentPositionWaveform(pVoiceCoil, _offsetV);
	}

	auto pVoiceCoil = _circleBufferList[0]->GetPointer<double>(_offsetV*CHANNELS_IN_BUFFER[0] + TASK_ID::ZSTAGE, CHANNELS_IN_BUFFER[0]);
	_wfVoice.CreateStripeVoiceCoilWaveform(pVoiceCoil, stripInfo);

	if (stripInfo->nextStrip == NULL)
	{
		auto pVoiceCoil = _circleBufferList[0]->GetPointer<double>((_offsetV + stripInfo->WaveformDataSize)*CHANNELS_IN_BUFFER[0] + TASK_ID::ZSTAGE, CHANNELS_IN_BUFFER[0]);
		CreateConstantWaveform(pVoiceCoil, _maxOffset - _offsetV, *(pVoiceCoil - 1));
	}
	return TRUE;
}
long MesoWaveformManager::GenerateStripWaveformPockel(StripInfo* stripInfo)
{
	if ((NULL == stripInfo) || (NULL == _circleBufferList[1]))
		return FALSE;

	if ((stripInfo->preStrip == NULL) || (stripInfo->IsStart))
	{
		auto pPokelsCell = _circleBufferList[1]->GetPointer<double>(TASK_ID::POCKELS1, CHANNELS_IN_BUFFER[1]);
		CreateConstantWaveform(pPokelsCell, _offsetP, 0);
	}

	auto pPokelsCell = _circleBufferList[1]->GetPointer<double>(_offsetP*CHANNELS_IN_BUFFER[1] + TASK_ID::POCKELS1, CHANNELS_IN_BUFFER[1]);
	_wfPokels.GenerateStripWaveform(pPokelsCell, stripInfo);

	if (stripInfo->nextStrip == NULL)
	{
		auto pPokelsCell = _circleBufferList[1]->GetPointer<double>((_offsetP + stripInfo->WaveformDataSize)*CHANNELS_IN_BUFFER[1] + TASK_ID::POCKELS1, CHANNELS_IN_BUFFER[1]);
		CreateConstantWaveform(pPokelsCell, _maxOffset - _offsetP, *(pPokelsCell - 1));
	}
	return TRUE;
}
long MesoWaveformManager::GenerateStripWaveformFrameTrigger(StripInfo* stripInfo)
{
	if ((NULL == stripInfo) || (NULL == _circleBufferList[2]))
		return FALSE;

	if ((stripInfo->preStrip == NULL) || (stripInfo->IsStart))
	{
		auto pFrameTrigger = _circleBufferList[2]->GetPointer<uint8_t>(TASK_ID::FRAMETRG, CHANNELS_IN_BUFFER[2]);
		CreateConstantWaveformInt8(pFrameTrigger, _offsetFrameTrigger, 0);
	}

	auto pFrameTriggerOut = _circleBufferList[2]->GetPointer<uint8_t>(_offsetFrameTrigger*CHANNELS_IN_BUFFER[2] + TASK_ID::FRAMETRG, CHANNELS_IN_BUFFER[2]);
	_wfFrameTrigger.CreateStripeFrameTriggerWaveform(pFrameTriggerOut, stripInfo);

	if (stripInfo->nextStrip == NULL)
	{
		//for frametrigger to add 2 points to end of capture;
		auto pFrameTrigger = _circleBufferList[2]->GetPointer<uint8_t>((_offsetFrameTrigger + stripInfo->WaveformDataSize)*CHANNELS_IN_BUFFER[2] + TASK_ID::FRAMETRG, CHANNELS_IN_BUFFER[2]);
		CreateConstantWaveformInt8(pFrameTrigger, _maxOffset - _offsetFrameTrigger + 2, 0);
	}
	return TRUE;
}

long MesoWaveformManager::CreateMoveToPositionWaveform(WaveformChannels channel, double * data, long length, double oldPositionValue, double newPositionValue)
{
	switch (channel)
	{
	case WaveformChannels::GALVO_X1:
		_wfGalvoX.MoveToPosition(data, length, oldPositionValue, newPositionValue);
		break;
	case WaveformChannels::GALVO_X2:
		_wfGalvoX.MoveToPosition(data, length, oldPositionValue, newPositionValue);
		break;
	case WaveformChannels::GALVO_Y:
		_wfGalvoY.MoveToPosition(data, length, oldPositionValue, newPositionValue);
		break;
	case WaveformChannels::VOICECOIL:
		_wfVoice.MoveToPosition(data, length, oldPositionValue, newPositionValue);
		break;
	case WaveformChannels::POCKELSCELL:
		_wfPokels.MoveToPosition(data, length, newPositionValue);
		break;
	default:
		return FALSE;
	}
	return TRUE;

}

long MesoWaveformManager::GenerateStripList(Scan* scanInCache, StripInfo ** first, StripInfo ** end, unsigned int TCount, DMABufferInfo* dmaInfo)
{
	long stripCount = 0;
	if (scanInCache == NULL) return stripCount;

	StripInfo* firstStripInLoop = NULL;
	StripInfo* endStripInLoop = NULL;

	_totalTime = 0;
	dmaInfo->bufferSize = 0;	//reset buffer size to reflect new buffer size
	for (unsigned int tIndex = 0; tIndex < TCount; tIndex++)
	{
		unsigned int ACount = static_cast<unsigned int>(scanInCache->ScanAreas.size());
		for (unsigned int aIndex = 0; aIndex < ACount; aIndex++)
		{
			ScanArea* scanArea = scanInCache->ScanAreas.at(aIndex);
			unsigned int SCount = static_cast<unsigned int>((scanInCache->ScanConfig.IsLivingMode) ? 1 : scanInCache->ScanAreas[0]->SizeS);
			for (unsigned int sIndex = 1; sIndex <= SCount; sIndex++)
			{
				for (uint32_t zIndex = 0; zIndex < scanArea->SizeZ; zIndex++)
				{
					uint32_t stripXOffsetInPixel = 0;
					while (stripXOffsetInPixel < scanArea->SizeX)
					{
						StripInfo* strip = new StripInfo();
						size_t saBufferSize = (scanInCache->Channels.size() * scanArea->SizeX * scanArea->SizeY * scanInCache->GetPixelBytes()) + sizeof(FrameInfo);	//frame size + info
						//bufferSize is the largest of all scan areas
						if ((dmaInfo->bufferSize < saBufferSize) && (sizeof(FrameInfo) < saBufferSize))
						{
							dmaInfo->sizeX = scanArea->SizeX;
							dmaInfo->sizeY = scanArea->SizeY;
							dmaInfo->bufferSize = saBufferSize;
						}
						strip->ScanAreaID = scanArea->ScanAreaID;
						strip->ScanMode = scanInCache->ScanConfig.ScanMode;
						strip->PockelsVolt = scanInCache->ScanConfig.CurrentPower;
						strip->XPos = scanArea->PositionX + stripXOffsetInPixel * scanInCache->XPixelSize;
						strip->YPos = scanArea->PositionY;
						strip->ZPos = scanArea->PositionZ + zIndex * scanInCache->ZPixelSize;
						strip->XPhysicalSize = min(scanInCache->ScanConfig.PhysicalFieldSize, scanArea->PhysicalSizeX - (double)stripXOffsetInPixel * scanInCache->XPixelSize);
						strip->YPhysicalSize = scanArea->PhysicalSizeY;
						strip->ZPhysicalSize = 0;
						strip->XPosResonMid = strip->XPos + strip->XPhysicalSize / 2;
						strip->XSize = min(scanInCache->ScanConfig.StripLength, scanArea->SizeX - stripXOffsetInPixel);
						strip->YSize = scanArea->SizeY;
						if (scanArea->PowerPoints.size() == 1)
						{
							strip->Power = scanArea->PowerPoints[0]->PowerPercentage / (double)Constants::HUNDRED_PERCENT;
						}
						else
						{
							for (int i = 0; i < (int)scanArea->PowerPoints.size() - 1; i++)
							{
								if (scanArea->PowerPoints[i + 1]->ZPosition >= strip->ZPos)
								{
									strip->Power = (scanArea->PowerPoints[i]->PowerPercentage +
										(scanArea->PowerPoints[i + 1]->PowerPercentage - scanArea->PowerPoints[i]->PowerPercentage)*
										(strip->ZPos - scanArea->PowerPoints[i]->ZPosition) /
										(scanArea->PowerPoints[i + 1]->ZPosition - scanArea->PowerPoints[i]->ZPosition))
										/ (double)Constants::HUNDRED_PERCENT;
									break;
								}
							}
						}
						for (vector<PowerBox*>::iterator iter = scanArea->PowerBoxs.begin(); iter != scanArea->PowerBoxs.end(); ++iter)
						{
							if (strip->ZPos >= (*iter)->StartZ&& strip->ZPos <= (*iter)->EndZ)
							{
								if ((*iter)->PowerROI->Bound.x > strip->XPos + strip->XPhysicalSize || (*iter)->PowerROI->Bound.x + (*iter)->PowerROI->Bound.width < strip->XPos)
									continue;

								bool isExist = false;
								for (vector<pair<ROI*, double>>::iterator iterPower = strip->ROIPower.begin(); iterPower != strip->ROIPower.end(); ++iterPower)
								{
									if ((*iter)->PowerROI->ROIID == (*iterPower).first->ROIID)
									{
										isExist = true;
										break;
									}
								}
								if (isExist)
									continue;
								strip->ROIPower.push_back(pair<ROI*, double>((*iter)->PowerROI, (*iter)->PowerPercentage / (double)Constants::HUNDRED_PERCENT));
							}
						}
						for (int c = 0; c < static_cast<int>(scanInCache->Channels.size()); c++)
						{
							strip->ChanBufInfo[c].ScanID = scanInCache->ScanID;
							strip->ChanBufInfo[c].ScanAreaID = scanArea->ScanAreaID;
							strip->ChanBufInfo[c].StreamID = sIndex;
							strip->ChanBufInfo[c].TimeID = tIndex + 1;
							strip->ChanBufInfo[c].ZID = zIndex + 1;
							strip->ChanBufInfo[c].ChannelID = scanInCache->Channels.at(c)->ChannelID;
						}
						strip->FrameROI.x = stripXOffsetInPixel;
						strip->FrameROI.y = 0;
						strip->FrameROI.width = min(scanArea->SizeX - stripXOffsetInPixel, scanInCache->ScanConfig.StripLength);
						strip->FrameROI.height = scanArea->SizeY;
						strip->FrameROI.frameWidth = scanArea->SizeX;
						strip->FrameROI.frameHeight = scanArea->SizeY;
						strip->IsFrameStart = (0 == stripXOffsetInPixel && 0 == zIndex && 0 == aIndex) ? true : false;
						for (int i = 0; i < scanInCache->ScanConfig.NumberOfAverageFrame; i++)
						{
							StripInfo* copy;
							if (i == scanInCache->ScanConfig.NumberOfAverageFrame - 1) 
							{
								copy = strip;
								copy->IsAverageEnd = true;
							}
							else 
							{
								copy = new StripInfo(*strip);
							}
							copy->preStrip = NULL;
							copy->nextStrip = NULL;
							if (endStripInLoop == NULL)
							{
								firstStripInLoop = copy;
								endStripInLoop = copy;
							}
							else
							{
								copy->preStrip = endStripInLoop;
								endStripInLoop->nextStrip = copy;
								endStripInLoop = copy;
							}
							_totalTime += GetStripTime(copy);
						}
						stripCount++;
						stripXOffsetInPixel += scanInCache->ScanConfig.StripLength;
					}
					//set frame end for every scan area
					endStripInLoop->IsFrameEnd = ((scanArea->SizeZ-1) == zIndex) ? true : false;
				}
			}
		}
	}
	if (NULL != endStripInLoop)
		endStripInLoop->IsEnd = true;

	*first = firstStripInLoop;
	*end = endStripInLoop;
	return stripCount;
}

double MesoWaveformManager::GetStripTime(StripInfo* strip)
{
	if (strip == NULL) return FALSE;
	GenerateStripSkipInfo(strip, false);
	return (strip->IncludeSignal + strip->SkipSignal) / _waveformParams->ResFreq;
}

long MesoWaveformManager::GetTotalScanTime(Scan scans[], uint8_t scanSize, CameraConfig* cameraConfig)
{
	double totalTime = 0;
	StripInfo* previousStrip = NULL;
	for (int i = 0; i < scanSize; ++i)
	{
		//The time cost in first S and other S is different.Save them in streamTime[]
		double streamTime[2] = { 0,0 };
		Scan* scan = new Scan(scans[i]);
		if (NULL == scan)
			break;
		if (SetWaveformParams(scan, cameraConfig) == FALSE) 
		{
			delete scan;
			break;
		}
		if (0 >= scan->ScanAreas.size())
		{
			delete scan;
			break;
		}		
		//Only calculate the time in first two S because the S after 2 is same as the second one
		uint16_t calculateSCount = scan->ScanAreas[0]->SizeS > 1 ? 2 : 1;
		for (uint16_t s = 1; s <= calculateSCount; s++)
		{
			for (vector<ScanArea*>::iterator iter = scan->ScanAreas.begin(); iter != scan->ScanAreas.end(); ++iter)
			{
				ScanArea* scanArea = *iter;

				//The time cost in first Z and other Z is different.Save them in zTime[]
				double zTime[2] = { 0,0 };

				//Only calculate the time in first two Z because the Z after 2 is same as the second one
				uint16_t calculateZCount = scanArea->SizeZ > 1 ? 2 : 1;
				for (uint32_t zIndex = 0; zIndex < calculateZCount; zIndex++)
				{
					uint32_t stripXOffsetInPixel = 0;
					while (stripXOffsetInPixel < scanArea->SizeX)
					{
						StripInfo* strip = new StripInfo();
						strip->ScanMode = scan->ScanConfig.ScanMode;
						strip->XPos = scanArea->PositionX + stripXOffsetInPixel * scan->XPixelSize;
						strip->YPos = scanArea->PositionY;
						strip->ZPos = scanArea->PositionZ + zIndex * scan->ZPixelSize;
						strip->XPhysicalSize = min(_waveformParams->PhysicalStripeWidth, scanArea->PhysicalSizeX - stripXOffsetInPixel * scan->XPixelSize);
						strip->YPhysicalSize = scanArea->PhysicalSizeY;
						strip->ZPhysicalSize = 0;
						strip->XPosResonMid = strip->XPos + strip->XPhysicalSize / 2;
						strip->XSize = min(_waveformParams->StripWidth, scanArea->SizeX - stripXOffsetInPixel);
						strip->YSize = scanArea->SizeY;

						for (int i = 0; i < _waveformParams->NumberOfAverageFrame; i++)
						{
							strip->preStrip = previousStrip;
							zTime[zIndex] += GetStripTime(strip);
							if (previousStrip != NULL) delete previousStrip;
							previousStrip = strip;
						}
						stripXOffsetInPixel += _waveformParams->StripWidth;
					}
				}
				//Calculate the time with all Z
				streamTime[s - 1] += zTime[0] + (scanArea->SizeZ - 1)*zTime[1];
			}
		}
		//Calculate the time with all S
		totalTime += streamTime[0] + (scan->ScanAreas[0]->SizeS - 1)*streamTime[1];
		delete scan;
	}
	if (previousStrip != NULL)
	{
		delete previousStrip;
	}

	return (long)(totalTime * Constants::MS_TO_SEC);
}

//**********************************	Additional Functions	**********************************//

long MesoWaveformManager::SetupCircularBuffer(long id)
{
	const int WF_LENGTH = WAVEFORM_MAX_BUFFER_LENGTH;
	const int NI_LENGTH = NI_BUFFER_LENGTH;
	int count = max((int)ceil((double)WF_LENGTH*2.0 / (double)NI_LENGTH), 4)*NI_LENGTH;

	//[2] digital frame trigger
	int unitSize = (2 == id) ? sizeof(uint8_t) : sizeof(double);

	switch (id)
	{
	case -1:
		TeardownCircularBuffers();

		//setup all circular buffers
		for (int i = 0; i < MAX_CIRCLEBUFFER; i++)
		{
			unitSize = (2 == i) ? sizeof(uint8_t) : sizeof(double);
			_circleBufferList[i] = new CircleBuffer(CHANNELS_IN_BUFFER[i] * count * unitSize);
		}
		break;
	default:
		if (id < MAX_CIRCLEBUFFER)
		{
			if (_circleBufferList[id] != NULL)
			{
				delete _circleBufferList[id];
				_circleBufferList[id] = NULL;
			}
			_circleBufferList[id] = new CircleBuffer(CHANNELS_IN_BUFFER[id] * count * unitSize);
		}
		break;
	}
	return TRUE;
}

void MesoWaveformManager::TeardownCircularBuffers()
{
	for (int i = 0; i < MAX_CIRCLEBUFFER; i++)
	{
		if (_circleBufferList[i] != NULL)
		{
			delete _circleBufferList[i];
			_circleBufferList[i] = NULL;
		}
	}
}

long MesoWaveformManager::DefaultWaveformParams(CameraConfig* camConfig)
{
	if (!_wfGalvoX.SetFlyBackParams(camConfig->MaxVelocityX, camConfig->MaxOvershootX, camConfig->ResonanceFrequency*camConfig->SamplesPerLine))
	{
		Logger::getInstance().LogMessage(L"Set Galvo X flyBack waveform parameters error");
		return FALSE;
	}
	if (!_wfGalvoY.SetFlyBackParams(camConfig->MaxVelocityY, camConfig->MaxOvershootY, camConfig->ResonanceFrequency*camConfig->SamplesPerLine))
	{
		Logger::getInstance().LogMessage(L"Set Galvo Y flyBack waveform parameters error");
		return FALSE;
	}
	if (!_wfVoice.SetFlyBackParams(camConfig->MaxVelocityVC, camConfig->MaxOvershootVC, camConfig->ResonanceFrequency*camConfig->SamplesPerLine))
	{
		Logger::getInstance().LogMessage(L"Set Voice coil flyBack waveform parameters error");
		return FALSE;
	}
	return TRUE;
}
