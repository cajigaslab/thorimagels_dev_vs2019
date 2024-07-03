#include "stdafx.h"
#include "WaveformManagerBase.h"
#include <iostream>

WaveformManagerBase::WaveformManagerBase()
{
	_waveformParams = new WaveformParams;
	_runningHandle = CreateEvent(NULL, true, false, NULL);
	_actualStopHandle = CreateEvent(NULL, true, true, NULL);
	_stopHandle = CreateEvent(NULL, true, false, NULL);
	_waveformReadyHandle = CreateEvent(NULL, true, false, NULL);
}
WaveformManagerBase::~WaveformManagerBase()
{
	SAFE_DELETE_HANDLE(_actualStopHandle);
	SAFE_DELETE_HANDLE(_runningHandle);
	SAFE_DELETE_HANDLE(_stopHandle);
}
long WaveformManagerBase::CheckParams(Scan* scan, CameraConfig* cameraConfig)
{
	if (WaitForSingleObject(_runningHandle, 0) == WAIT_OBJECT_0)
	{
		if (cameraConfig->F2VGx1 != _waveformParams->F2TGx1) return FALSE;
		if (cameraConfig->F2VGx2 != _waveformParams->F2TGx2) return FALSE;
		if (cameraConfig->F2VGy != _waveformParams->F2TGy) return FALSE;
		if (cameraConfig->PockelDutyCycle != _waveformParams->PockelDutyCycle) return FALSE;
		if (scan->ScanConfig.PhysicalFieldSize != _waveformParams->PhysicalStripeWidth) return FALSE;
		if (cameraConfig->DelayTimeGx1 != _waveformParams->DelayTimeGx1) return FALSE;
		if (cameraConfig->DelayTimeGx2 != _waveformParams->DelayTimeGx2) return FALSE;
		if (cameraConfig->DelayTimeGy != _waveformParams->DelayTimeGy) return FALSE;
		if (cameraConfig->DelayTimeVC != _waveformParams->DelayTimeVC) return FALSE;
		if (cameraConfig->DelayTimePC != _waveformParams->DelayTimePC) return FALSE;
		if (abs(cameraConfig->CurveParamA- _waveformParams->CurveParameterA) > 1E-5) return FALSE;
		if (abs(cameraConfig->CurveParamB - _waveformParams->CurveParameterB) > 1E-5) return FALSE;
		if (scan->ScanConfig.StripLength != _waveformParams->StripWidth) return FALSE;
		//if (scan->ScanConfig.NumberOfAverageFrame != _waveformParams->NumberOfAverageFrame) return FALSE;
		if (scan->ScanConfig.IsEnableCurveCorrection != _waveformParams->IsEnableCurveCorrection) return FALSE;
	}
	return TRUE;
}

/// <summary> set up strip loop and return value: ([0]:Failed,[1]:no reset mem, non-critical param changes,[2]:reset mem) </summary>
long WaveformManagerBase::SetParams(Scan* scan,CameraConfig* cameraConfig, DMABufferInfo* dmaInfo)
{
	long resetMem = 2;
	if (_currentStrip == NULL)
	{
		Scan* scanInCache = new Scan(*scan);
		_waveformParams->powerBoxs.clear();
		for (vector<ScanArea*>::iterator iter = scanInCache->ScanAreas.begin(); iter != scanInCache->ScanAreas.end(); ++iter)
		{
			ScanArea* scanArea = *iter;
			for (vector<PowerBox*>::iterator iterP = scanArea->PowerBoxs.begin(); iterP != scanArea->PowerBoxs.end(); ++iterP)
			{
				_waveformParams->powerBoxs.push_back(pair<uint16_t, double>((*iterP)->PowerROI->ROIID, (*iterP)->PowerPercentage));
			}
		}

		if (SetWaveformParams(scanInCache, cameraConfig) == TRUE)
		{
			if (_firstStrip != NULL)
			{
				StripInfo::DeleteStripLoop(_firstStrip);
				_firstStrip = NULL;
				_currentStrip = NULL;
			}
		}

		StripInfo* firstStripInLoop = NULL;
		StripInfo* endStripInLoop =NULL;
		_stripCount = GenerateStripList(scanInCache, &firstStripInLoop, &endStripInLoop, (1 == scanInCache->ScanConfig.IsLivingMode) ? 1 : cameraConfig->GetParameter<unsigned int>(ICamera::PARAM_MULTI_FRAME_COUNT), dmaInfo);

		if (NULL == endStripInLoop || 0 == _stripCount)
		{
			Logger::getInstance().LogMessage(L"No Strip created.", VERBOSE_EVENT);
			delete scanInCache;
			return FALSE;
		}

		if (scanInCache->ScanConfig.IsLivingMode)
		{
			firstStripInLoop->preStrip = endStripInLoop;
			endStripInLoop->nextStrip = firstStripInLoop;
			if (firstStripInLoop->nextStrip == NULL)//if only one strip
			{
				firstStripInLoop->nextStrip = firstStripInLoop;
				firstStripInLoop->preStrip = firstStripInLoop;
			}
		}
		StripInfo* currentStrip = firstStripInLoop;
		_maxStipSize = 0;
		while (currentStrip != NULL)
		{
			GenerateStripSkipInfo(currentStrip);
			// protect two way for odd stripe height
			uint32_t oddHeight = currentStrip->FrameROI.height % 2 > 0? currentStrip->FrameROI.height + 1 : currentStrip->FrameROI.height;
			uint32_t stripSize = uint32_t(scanInCache->ScanConfig.StripLength*oddHeight*scan->Channels.size());
			if (stripSize > _maxStipSize)
			{
				_maxStipSize = stripSize;
			}
			currentStrip = currentStrip->IsEnd ? NULL : currentStrip->nextStrip;	
		}

		_scansCache.push_back(scanInCache);
		ResetScanParams(scanInCache,true);
		_mutex.lock();
		_firstStrip = firstStripInLoop;
		_firstStrip->IsStart = true;
		_currentStrip = firstStripInLoop;
		_mutex.unlock();
	}
	else
	{
		_mutex.lock();
		resetMem = 1;
		// update running strip list with Power
		StripInfo* currentStrip = _firstStrip;
		do
		{
			if (WAIT_OBJECT_0 == WaitForSingleObject(_stopHandle, 0))
				break;

			for (vector<ScanArea*>::iterator iter = scan->ScanAreas.begin(); iter != scan->ScanAreas.end(); ++iter)
			{
				ScanArea* scanArea = *iter;
				if (scanArea->ScanAreaID == currentStrip->ScanAreaID)
				{
					for (int i = 0; i < (int)scanArea->PowerPoints.size() - 1; i++)
					{
						if (scanArea->PowerPoints[i + 1]->ZPosition >= currentStrip->ZPos)
						{
							currentStrip->Power = (scanArea->PowerPoints[i]->PowerPercentage +
								(scanArea->PowerPoints[i + 1]->PowerPercentage - scanArea->PowerPoints[i]->PowerPercentage)*
								(currentStrip->ZPos - scanArea->PowerPoints[i]->ZPosition) /
								(scanArea->PowerPoints[i + 1]->ZPosition - scanArea->PowerPoints[i]->ZPosition))
								/ 100.0;
							break;
						}
					}
					break;
				}
			}
			currentStrip = (TRUE == currentStrip->IsEnd) ? NULL : currentStrip->nextStrip;
		}while (NULL != currentStrip);
		_mutex.unlock();
	}
	return resetMem;
}

CircleBuffer* WaveformManagerBase::GetWaveformBuffer(uint8_t index)
{
	if (index >= MAX_CIRCLEBUFFER)
		return FALSE;
	return _circleBufferList[index];
}

long WaveformManagerBase::StartGenerate()
{
	if (WaitForSingleObject(_runningHandle, 0) == WAIT_OBJECT_0)
		return TRUE;
	ResetEvent(_waveformReadyHandle);
	for (int i = 0; i < MAX_CIRCLEBUFFER; i++)
	{
		CHECK_PFUNC(_circleBufferList[i], Reset());
	}
	if(_currentStrip==NULL)
		return FALSE;

	SAFE_DELETE_HANDLE(_hThread);
	_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartGenerateThread, this, 0, NULL);
	if (_hThread == NULL) return FALSE;
	ResetEvent(_stopHandle);
	ResetEvent(_actualStopHandle);
	SetEvent(_runningHandle);
	SetThreadPriority(_hThread, THREAD_PRIORITY_HIGHEST);
	WaitForSingleObject(_waveformReadyHandle, 2*Constants::EVENT_WAIT_TIME);
	return TRUE;
}
long WaveformManagerBase::StopGenerate()
{
	SetEvent(_stopHandle);
	if (NULL != _hThread)
	{
		if (WaitForSingleObject(_actualStopHandle, Constants::EVENT_WAIT_TIME) == WAIT_TIMEOUT)
		{
			ClearStripsAndScan();
			ResetEvent(_runningHandle);
			SetEvent(_actualStopHandle);
			SAFE_DELETE_HANDLE(_hThread);
			Logger::getInstance().LogMessage(L"WaveformManagerBase timed out at waiting for thread to stop.", ERROR_EVENT);
		}
	}
	else
	{
		ClearStripsAndScan();
	}
	return TRUE;
}

long WaveformManagerBase::SetConfigParams(CameraConfig * cameraConfig)
{
	_waveformParams->FieldWidth = cameraConfig->MaxPosX;
	_waveformParams->FieldHeight = cameraConfig->MaxPosY;
	_waveformParams->F2TGx1 = cameraConfig->F2VGx1;
	_waveformParams->F2TGx2 = cameraConfig->F2VGx2;
	_waveformParams->F2TGy = cameraConfig->F2VGy;
	_waveformParams->SamplesPerLine = cameraConfig->SamplesPerLine;
	_waveformParams->ResFreq = cameraConfig->ResonanceFrequency;
	_waveformParams->PockelDutyCycle = cameraConfig->PockelDutyCycle;
	_waveformParams->PockelInMax = cameraConfig->PockelInMax;
	_waveformParams->PockelInMin = cameraConfig->PockelInMin;
	_waveformParams->PockelMinPercent = cameraConfig->PockelMinPercent;
	_waveformParams->NumberOfAverageFrame = (ICamera::AverageMode::AVG_MODE_NONE == cameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_AVERAGEMODE)) ? 
		1 : cameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_AVERAGENUM);
	_waveformParams->DelayTimeGx1 = cameraConfig->DelayTimeGx1;
	_waveformParams->DelayTimeGx2 = cameraConfig->DelayTimeGx2;
	_waveformParams->DelayTimeGy = cameraConfig->DelayTimeGy;
	_waveformParams->DelayTimeVC = cameraConfig->DelayTimeVC;
	_waveformParams->DelayTimePC = cameraConfig->DelayTimePC;
	_waveformParams->CenterShiftX = cameraConfig->CenterShiftX;
	_waveformParams->CenterShiftY = cameraConfig->CenterShiftY;
	_waveformParams->VCPointsPerLine = cameraConfig->VCPointsPerLine;
	_waveformParams->CurveParameterA = cameraConfig->CurveParamA;
	_waveformParams->CurveParameterB = cameraConfig->CurveParamB;
	_waveformParams->MaxVelocityX = cameraConfig->MaxVelocityX;
	_waveformParams->MaxOvershootX = cameraConfig->MaxOvershootX;
	_waveformParams->MaxVelocityY = cameraConfig->MaxVelocityY;
	_waveformParams->MaxOvershootY = cameraConfig->MaxOvershootY;
	_waveformParams->MaxVelocityVC = cameraConfig->MaxVelocityVC;
	_waveformParams->MaxOvershootVC = cameraConfig->MaxOvershootVC;
	_waveformParams->ZeroPointVoltGX1 = cameraConfig->MinPosXVoltage;
	_waveformParams->ZeroPointVoltGY = cameraConfig->MinPosYVoltage;
	_waveformParams->ZeroPointVoltVoiceCoil = cameraConfig->MinPosZVoltage;
	_waveformParams->MaxPointVoltVoiceCoil = cameraConfig->MaxPosZVoltage;
	_waveformParams->GX2Scale = cameraConfig->GForX2;
	_waveformParams->GX2Shift = cameraConfig->HForX2;
	_waveformParams->VoicecoilZToVolts = cameraConfig->F2VZ;
	_waveformParams->GXExtendTimeStart = cameraConfig->GXExtendTimeStart;
	_waveformParams->GXExtendTimeEnd = cameraConfig->GXExtendTimeEnd;
	_waveformParams->GYExtendTimeStart = cameraConfig->GYExtendTimeStart;
	_waveformParams->GYExtendTimeEnd = cameraConfig->GYExtendTimeEnd;
	_waveformParams->VCExtendTimeStart = cameraConfig->VCExtendTimeStart;
	_waveformParams->VCExtendTimeEnd = cameraConfig->VCExtendTimeEnd;
	_waveformParams->VCSkipLines = cameraConfig->VCSkipLines;
	return TRUE;
}

long WaveformManagerBase::SetDefaultParams(CameraConfig * cameraConfig)
{
	SetConfigParams(cameraConfig);
	_waveformParams->PhysicalStripeWidth = 100;
	SetParams(_waveformParams, 1);
	return TRUE;
}

long WaveformManagerBase::SetWaveformParams(Scan* scan, CameraConfig * cameraConfig)
{
	SetConfigParams(cameraConfig);
	_waveformParams->StripWidth = scan->ScanConfig.StripLength;
	_waveformParams->PhysicalStripeWidth = scan->ScanConfig.PhysicalFieldSize;
	_waveformParams->IsEnableCurveCorrection = scan->ScanConfig.IsEnableCurveCorrection;
	double remapShift = 0;
	cameraConfig->GetTwoWayAlignmentPoint(scan->ScanConfig.PhysicalFieldSize / cameraConfig->PockelDutyCycle, remapShift);
	_waveformParams->TwoWayOffset = (int)ceil((double)(scan->ScanConfig.RemapShift + (long)(remapShift + 0.000001))*cameraConfig->SamplesPerLine / cameraConfig->ResonanceFrequency);

	uint32_t maxStripeSize = 0;
	for (vector<ScanArea*>::iterator iter = scan->ScanAreas.begin(); iter != scan->ScanAreas.end(); ++iter)
	{
		ScanArea* scanArea = *iter;
		uint32_t includeSignal = scan->ScanConfig.ScanMode == TWO_WAY_SCAN ? (scanArea->SizeY + 1) / 2 : scanArea->SizeY;
		uint32_t stripeSize = includeSignal * cameraConfig->SamplesPerLine;
		maxStripeSize = max(stripeSize, maxStripeSize);
	}

	SetParams(_waveformParams, maxStripeSize);
	return TRUE;
}

void WaveformManagerBase::StartGenerateThread(LPVOID pParam)
{
	WaveformManagerBase *pObj = reinterpret_cast<WaveformManagerBase*>(pParam);
	pObj->GenerateAsync();
}

void WaveformManagerBase::GenerateAsync()
{
	uint32_t currentLoop = 0;
	while (WAIT_OBJECT_0 != WaitForSingleObject(_stopHandle, 0))
	{
		while (IsBufferReady(_currentStrip))
		{
			if (WAIT_OBJECT_0 == WaitForSingleObject(_stopHandle, 0))
				break;

			_mutex.lock();
			GenerateStripWaveform(_currentStrip);
			_stripInfoChangedCallBack(_currentStrip);
			_currentStrip = _currentStrip->nextStrip;
			_mutex.unlock();

			if (_currentStrip == NULL)
			{
				SetEvent(_stopHandle);
				break;
			}
		}
		SetEvent(_waveformReadyHandle);
	}
	SetEvent(_waveformReadyHandle);
	ClearStripsAndScan();
	ResetEvent(_runningHandle);
	SetEvent(_actualStopHandle);
	SAFE_DELETE_HANDLE(_hThread);
}

unsigned int WaveformManagerBase::GetMaxiStripSize()
{
	return _maxStipSize;
}

//**********************************	Additional Functions	**********************************//

void WaveformManagerBase::ClearStripsAndScan()
{
	try
	{
		_mutex.lock();
		if (_firstStrip != NULL)
		{
			StripInfo::DeleteStripLoop(_firstStrip);
			_firstStrip = NULL;
			_currentStrip = NULL;
		}
		_mutex.unlock();

		if (0 < _scansCache.size())
		{
			for (size_t i = 0; i < _scansCache.size(); i++)
			{
				delete _scansCache.at(i);
				_scansCache.at(i) = NULL;
			}
			_scansCache.clear();
		}	
	}
	catch(...)
	{
		StringCbPrintfW(message,_MAX_PATH, L"WaveformManagerBase ClearStripsAndScan failed");
		Logger::getInstance().LogMessage(message,VERBOSE_EVENT);
	}
}
