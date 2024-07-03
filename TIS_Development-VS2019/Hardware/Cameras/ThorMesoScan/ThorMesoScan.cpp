// ThorMesoScan.cpp : Defines the exported functions for the DLL application.
//

#include "ThorMesoScan.h"
#include "MesoWaveformManager.h"
#include <omp.h>
#include "..\..\..\Tools\tinyxml2\include\tinyxml2.h"

using namespace tinyxml2;

void Output(const wchar_t* szFormat, ...)
{
	wchar_t szBuff[1024];
	va_list arg;
	va_start(arg, szFormat);
	_vsnwprintf_s(szBuff, sizeof(szBuff), szFormat, arg);
	va_end(arg);

	OutputDebugString(szBuff);
}

ThorLSMCam::ThorLSMCam() :
	MAX_PIXEL(17024),	//16384 [4Ch], 32768 [1Ch]
	MIN_PIXEL(64),
	DEFAULT_PIXEL(512),
	DEFAULT_RES_VOLT(4.7),
	LOW_UINT8(0),
	HIGH_UINT8(1),
	MIN_INPUTRANGE(INPUT_RANGE_PM_20_MV),
	MAX_INPUTRANGE(INPUT_RANGE_PM_4_V),
	DEFAULT_INPUTRANGE(INPUT_RANGE_PM_1_V)
{
	try
	{
		_preScan = NULL;
		_tmpBuffer = NULL;
		memset(_pockelsPowerLevel, 0x0, MAX_GG_POCKELS_CELL_COUNT*sizeof(double));

		_hDAQController = DAQBoard::GetInstance();
		_hMesoScanWaveform = (WaveformManagerBase*)MesoWaveformManager::GetInstance();

		_pCameraConfig.reset(new CameraConfig());

		_alazarHasError = false;

		int threadsCount = (int)ceil(omp_get_num_procs() *0.75);
		omp_set_num_threads(threadsCount);

		_expLoader.reset(new LoadMeso);

		_acquireStatus = (int)ICamera::STATUS_BUSY;
		_indexOfLastCompletedFrame = _indexOfLastCopiedFrame = -1;
		_bufferInfo.scanAreaCount = _bufferInfo.sizeX = _bufferInfo.sizeY = 0;
		_bufferInfo.bufferSize = _bufferInfo.dmaCount = 0;
		for (int i = 0; i < MAX_DMABUFNUM; i++) { _pFrmDllBuffer[i] = NULL;	}
		_pixelX = _pixelY = DEFAULT_PIXEL;
		_rsInitMode = _scanMode = _triggerMode = 0;
		_inputRangeChannel[0] = _inputRangeChannel[1] = _inputRangeChannel[2] = _inputRangeChannel[3] = DEFAULT_INPUTRANGE;

		::InitializeCriticalSection(&_accessSection);
	}
	catch (std::exception ex)
	{
		Logger::getInstance().LogMessage((wchar_t*)StringToWString(ex.what()).c_str(), ERROR_EVENT);
	}
}

ThorLSMCam::~ThorLSMCam()
{
	_pCameraConfig.release();
	::DeleteCriticalSection(&_accessSection);
}

once_flag ThorLSMCam::_onceFlag;
unique_ptr<ThorLSMCam> ThorLSMCam::_single;
CRITICAL_SECTION ThorLSMCam::_accessSection;
HANDLE ThorLSMCam::_hHardwareTriggerInEvent = CreateEvent(NULL, false, false, NULL);

wchar_t message[_MAX_PATH];

ThorLSMCam* ThorLSMCam::getInstance()
{
	std::call_once(_onceFlag,
		[] {
			_single.reset(new ThorLSMCam);
	});
	return _single.get();
}

void ThorLSMCam::CloseLSMCam()
{
	if (_hAlazarBoard != NULL)
	{
		AlazarBoard::CloseAlazarBoard(_hAlazarBoard);
		_hAlazarBoard = NULL;
	}
	CHECK_PFUNC(_hMesoScanWaveform, TeardownCircularBuffers());

	if (_preScan != NULL)
	{
		delete _preScan;
		_preScan = NULL;
	}
}

long ThorLSMCam::IsHardwareReady()
{
	if (_hAlazarBoard == NULL) return FALSE;
	return TRUE;
}

long ThorLSMCam::SetWaveform()
{	
	if (_preScan == NULL || _pCameraConfig.get() == NULL)
		return FALSE;

	//no more copy allowed after preflight by reset of indexes
	_indexOfLastCompletedFrame = _indexOfLastCopiedFrame = -1;
	_bufCompleteID = 0;
	_pCameraConfig.get()->SetParameter(ICamera::PARAM_DROPPED_FRAMES, (unsigned int)0);
	_scanMode = _preScan->ScanConfig.ScanMode;
	_triggerMode = _pCameraConfig.get()->GetParameter<unsigned int>(ICamera::Params::PARAM_TRIGGER_MODE);
	_channels.clear();
	for (int i = 0; i < static_cast<int>(_preScan->Channels.size()); i++)
	{
		Channel ch;
		memcpy_s(&ch,sizeof(Channel),_preScan->Channels[i],sizeof(Channel));
		_channels.push_back(ch);
	}

	DMABufferInfo dmaInfo = _bufferInfo;
	long resetMem = _hMesoScanWaveform->SetParams(_preScan, _pCameraConfig.get(), &dmaInfo);	//([0]:Failed,[1]:no reset mem, non-critical param changes,[2]:reset mem)
	if (FALSE == resetMem)
		return FALSE;

	return ResetDMABuffers(&dmaInfo, (resetMem-1));
}

long ThorLSMCam::SetBdDMA()
{
	if (_preScan == NULL || _pCameraConfig.get() == NULL)
		return FALSE;
	long ret = FALSE;
	ret = _hAlazarBoard->SetParams(_preScan, _pCameraConfig.get(), _hMesoScanWaveform->GetMaxiStripSize());
	if (!ret) return ret;
	return TRUE;
}

long ThorLSMCam::SetDAQBoard()
{
	_hDAQController->SetSampleClock(_pCameraConfig->ResonanceFrequency*_pCameraConfig->SamplesPerLine, _pCameraConfig->SamplesPerLine);

	// start resonece
	float64  data = 0.0;
	if (_pCameraConfig->GetResonantVoltage(_preScan->ScanConfig.PhysicalFieldSize / _pCameraConfig->PockelDutyCycle, data) == FALSE)
		return FALSE;
	_hDAQController->InvokeTask(_pCameraConfig->RESONANT_AO_CHANNEL, AO, &data, 1, 1);

	// generate all tasks
	_hDAQController->StopAllTasks();

	if(!_hDAQController->InitRuningTasks(CHANNEL_TYPE::DO) || !_hDAQController->InitRuningTasks(CHANNEL_TYPE::AO))
		goto DAQ_FAIL_RETURN;

	switch ((TriggerMode)_triggerMode)
	{
	case TriggerMode::HW_MULTI_FRAME_TRIGGER_FIRST:
	case TriggerMode::HW_SINGLE_FRAME:
	case TriggerMode::HW_MULTI_FRAME_TRIGGER_EACH:
		if(!_hDAQController->InitRuningTasks(CHANNEL_TYPE::CI))
			goto DAQ_FAIL_RETURN;
		break;
	default:
		break;
	}
	return TRUE;

DAQ_FAIL_RETURN:
	_hDAQController->StopAllTasks();
	return FALSE;
}

long ThorLSMCam::StopScan()
{
	try
	{
		_hDAQController->StopAllTasks();

		if (_hAlazarBoard != NULL)
			_hAlazarBoard->StopAcquisition();

		_hMesoScanWaveform->StopGenerate();

		//force the hardware trigger event if stop scan is called
		SetEvent(_hHardwareTriggerInEvent);
		ResetEvent(_hHardwareTriggerInEvent);

		//set pockels to minimum
		float64 data = _pCameraConfig->GetPockelPowerVoltage(_pCameraConfig->PockelMinPercent);
		_hDAQController->InvokeTask(_pCameraConfig->POCKEL_AO_CHANNEL, AO, &data, 1, 1);

		//maintain status for live imaging to continue
		_acquireStatus = ((int)ICamera::STATUS_ERROR != _acquireStatus) ? (int)ICamera::STATUS_READY : (int)ICamera::STATUS_ERROR;
	}
	catch (...)
	{
		Logger::getInstance().LogMessage(L"ThorLSMCam::StopScan failed.", ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

void ThorLSMCam::StripInfoChanged(StripInfo* stripInfo)
{
	_hAlazarBoard->AppendStripInfo(stripInfo);
}

long ThorLSMCam::ReadPosition()
{
	// check if affect the range
	_pCameraConfig->CalculateFieldToVoltage();

	// get galvo postions
	double positionsGYVC[2] = {0, 0}; // y, voice coil
	double positionsGX1X2[2] = {0, 0}; // x1, x2
	_hDAQController->ReadVoltages(_pCameraConfig->GY_VOICECOIL_FEEDBACK_CHANNEL, positionsGYVC, 2);
	_hDAQController->ReadVoltages(_pCameraConfig->GX1_GX2_FEEDBACK_CHANNEL, positionsGX1X2, 2);

	return _hMesoScanWaveform->SetCurrentPosition(_pCameraConfig->GetX1RealVoltage(positionsGX1X2[0]),
		_pCameraConfig->GetX2RealVoltage(positionsGX1X2[1]),
		_pCameraConfig->GetYRealVoltage(positionsGYVC[0]),
		_pCameraConfig->GetZRealVoltage(positionsGYVC[1]));
}

long ThorLSMCam::SetParameters()
{
	if (_preScan == NULL || _pCameraConfig == NULL) return FALSE;
	if (!_hMesoScanWaveform->CheckParams(_preScan, _pCameraConfig.get()))
		return FALSE;
	if (!_hAlazarBoard->CheckParams(_preScan, _pCameraConfig.get()))
		return FALSE;
	return SetWaveform();
}

long ThorLSMCam::StartWaveform()
{
	long ret = SetBdDMA();
	if (!ret) return ret;

	ret = SetDAQBoard();
	if (!ret) return ret;

	return _hMesoScanWaveform->StartGenerate();
}


long ThorLSMCam::StartScan()
{
	long ret = FALSE;
	DWORD retWait = WAIT_OBJECT_0;

	::EnterCriticalSection(&_accessSection);

	// set low power for frame trigger before start
	if (TRUE == _hDAQController->PreStartTasks())
	{
		_hDAQController->StarTasks();

		switch ((TriggerMode)_triggerMode)
		{
		case TriggerMode::HW_MULTI_FRAME_TRIGGER_FIRST:
		case TriggerMode::HW_SINGLE_FRAME:
		case TriggerMode::HW_MULTI_FRAME_TRIGGER_EACH:
			retWait = WaitForSingleObject(_hHardwareTriggerInEvent, (DWORD)(_pCameraConfig.get()->TriggerWaitTimeSec * Constants::MS_TO_SEC));
			break;
		default:
			break;
		}
		if (WAIT_OBJECT_0 == retWait && _hDAQController->IsRunning())
		{
			ret = _hAlazarBoard->StartAcquisition();
			_acquireStatus = (TRUE == ret) ? (int)ICamera::STATUS_BUSY : (int)ICamera::STATUS_ERROR;
		}
	}
	::LeaveCriticalSection(&_accessSection);
	return ret;
}

long ThorLSMCam::MoveLightToPosition(GALVO_PARK_TYPE type)
{
	float64 closeResVolt = 0.0;

	_pCameraConfig->CalculateFieldToVoltage();

	double GYVCFeedbackValueArr[2] = {0.0 , 0.0};		// y, voice coil
	double GX1X2FeedbackValueStartArr[2] = {0.0 , 0.0};	// x1, x2
	if (FALSE == _hDAQController->ReadVoltages(_pCameraConfig->GY_VOICECOIL_FEEDBACK_CHANNEL, GYVCFeedbackValueArr, 2))
		return FALSE;
	if (FALSE == _hDAQController->ReadVoltages(_pCameraConfig->GX1_GX2_FEEDBACK_CHANNEL, GX1X2FeedbackValueStartArr, 2))
		return FALSE;

	double startValueX1 = _pCameraConfig->GetX1RealVoltage(GX1X2FeedbackValueStartArr[0]);
	double startValueX2 = _pCameraConfig->GetX2RealVoltage(GX1X2FeedbackValueStartArr[1]);
	double startValueY = _pCameraConfig->GetYRealVoltage(GYVCFeedbackValueArr[0]);
	double startValueVC = _pCameraConfig->GetZRealVoltage(GYVCFeedbackValueArr[1]);
	double startValueP = _pCameraConfig->GetPockelPowerVoltage(_pCameraConfig->PockelMinPercent);
	long rate = static_cast<long>(_pCameraConfig->ResonanceFrequency * _pCameraConfig->SamplesPerLine);

	int channelLength = 0, length = 0;
	channelLength = static_cast<int>((_pCameraConfig->MaxOvershootX * 2 + 20) / _pCameraConfig->MaxVelocityX * rate);
	length = max(length, channelLength);
	channelLength = static_cast<int>((_pCameraConfig->MaxOvershootY * 2 + 20) / _pCameraConfig->MaxVelocityY * rate);
	length = max(length, channelLength);
	channelLength = static_cast<int>((_pCameraConfig->MaxOvershootVC * 2 + 20) / _pCameraConfig->MaxVelocityVC * rate);
	length = max(length, channelLength);
	_tmpBuffer = (double*)realloc((void*)_tmpBuffer, sizeof(double) * length * CHANNELS_IN_BUFFER[1]);
	if(NULL != _tmpBuffer)
	{
		MesoWaveformManager *pWaveformManager = (MesoWaveformManager *)_hMesoScanWaveform;
		switch (type)
		{
		case PARK_AT_CENTER:
			{
				_hDAQController->InvokeTask(_pCameraConfig->RESONANT_AO_CHANNEL, AO, &closeResVolt, 1, 1);

				pWaveformManager->CreateMoveToPositionWaveform(WaveformChannels::POCKELSCELL, _tmpBuffer + TASK_ID::POCKELS1 * length, length, startValueP, _pCameraConfig->GetPockelPowerVoltage(_pockelsPowerLevel[0]));
				pWaveformManager->CreateMoveToPositionWaveform(WaveformChannels::GALVO_Y, _tmpBuffer + TASK_ID::GY * length, length, startValueY, _pCameraConfig->GetYCenterVoltage());
				pWaveformManager->CreateMoveToPositionWaveform(WaveformChannels::GALVO_X1, _tmpBuffer + TASK_ID::GX1 * length, length, startValueX1, _pCameraConfig->GetX1CenterVoltage());
				_hDAQController->InvokeTask(_pCameraConfig->P1_GY_GX1_P2_AO_CHANNEL, AO, _tmpBuffer, length, rate);

				pWaveformManager->CreateMoveToPositionWaveform(WaveformChannels::GALVO_X2, _tmpBuffer + TASK_ID::GX2 * length, length, startValueX2, _pCameraConfig->GetX2CenterVoltage());
				_hDAQController->InvokeTask(_pCameraConfig->GX2_AO_CHANNEL, AO, _tmpBuffer, length, rate);

				pWaveformManager->CreateMoveToPositionWaveform(WaveformChannels::VOICECOIL, _tmpBuffer + TASK_ID::ZSTAGE * length, length, startValueVC, _pCameraConfig->GetZCenterVoltage());
				_hDAQController->InvokeTask(_pCameraConfig->VOICECOIL_AO_CHANNEL, AO, _tmpBuffer, length, rate);
			}
			break;
		case PARK_NEAR_START:
		case PARK_AT_EXIT:
			{
				if (PARK_AT_EXIT == type || 0 == _rsInitMode)
					_hDAQController->InvokeTask(_pCameraConfig->RESONANT_AO_CHANNEL, AO, &closeResVolt, 1, 1);

				pWaveformManager->CreateMoveToPositionWaveform(WaveformChannels::POCKELSCELL, _tmpBuffer + TASK_ID::POCKELS1 * length, length, startValueP, _pCameraConfig->GetPockelPowerVoltage(_pCameraConfig->PockelMinPercent));
				pWaveformManager->CreateMoveToPositionWaveform(WaveformChannels::GALVO_Y, _tmpBuffer + TASK_ID::GY * length, length, startValueY, _pCameraConfig->GetYParkPosVoltage(type));
				pWaveformManager->CreateMoveToPositionWaveform(WaveformChannels::GALVO_X1, _tmpBuffer + TASK_ID::GX1 * length, length, startValueX1, _pCameraConfig->GetX1ParkPosVoltage(type));
				_hDAQController->InvokeTask(_pCameraConfig->P1_GY_GX1_P2_AO_CHANNEL, AO, _tmpBuffer, length, rate);

				pWaveformManager->CreateMoveToPositionWaveform(WaveformChannels::GALVO_X2, _tmpBuffer + TASK_ID::GX2 * length, length, startValueX2, _pCameraConfig->GetX2ParkPosVoltage(type));
				_hDAQController->InvokeTask(_pCameraConfig->GX2_AO_CHANNEL, AO, _tmpBuffer, length, rate);

				pWaveformManager->CreateMoveToPositionWaveform(WaveformChannels::VOICECOIL, _tmpBuffer + TASK_ID::ZSTAGE * length, length, startValueVC, _pCameraConfig->GetZParkVoltage());
				_hDAQController->InvokeTask(_pCameraConfig->VOICECOIL_AO_CHANNEL, AO, _tmpBuffer, length, rate);
			}
			break;
		default:
			break;
		}
		SAFE_DELETE_MEMORY(_tmpBuffer);
	}
	//set frame trigger out to low when parking
	_hDAQController->InvokeTask(_pCameraConfig.get()->FRAMETRIGGEROUT_DO_CHANNEL, DO, &(uInt8)LOW_UINT8, sizeof(uInt8), 0);
	return TRUE;
}

//**********************************	Additional Functions	**********************************//

/// <summary> Create DMA buffers to save captured frames in order of scan areas, limit memory usage based on unit size </summary>
long ThorLSMCam::ResetDMABuffers(DMABufferInfo* dmaInfo, long resetMem)
{
	long ret = TRUE;
	if (0 >= dmaInfo->bufferSize)
		return FALSE;

	//limit DMA unit count based on buffer size [<8MB,*][8-32MB,8][32-64MB,4][>64MB,2]
	unsigned int dmaUnits = _pCameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_DMA_BUFFER_COUNT);	// GetAvailableMemorySize() / bufferSize;
	if ((0x800000 < dmaInfo->bufferSize) && (0x2000000 > dmaInfo->bufferSize))
	{
		dmaUnits = min(8, dmaUnits);
	}
	else if ((0x2000000 < dmaInfo->bufferSize) && (0x4000000 > dmaInfo->bufferSize))
	{
		dmaUnits = min(4, dmaUnits);
	}
	else if (0x4000000 < dmaInfo->bufferSize)
	{
		dmaUnits = min(2, dmaUnits);
	}
	_pCameraConfig->SetParameter(ICamera::Params::PARAM_LSM_DMA_BUFFER_COUNT, dmaUnits);
	try
	{
		_dmaMutex.lock();
		size_t dmaCount = min(MAX_DMABUFNUM, _preScan->ScanAreas.size() * dmaUnits);
		if ((_bufferInfo.bufferSize != dmaInfo->bufferSize) || (_bufferInfo.dmaCount != dmaCount) || (_bufferInfo.scanAreaCount != static_cast<long>(_preScan->ScanAreas.size())))
		{
			for (size_t i = 0; i < dmaCount; i++)
			{
				_pFrmDllBuffer[i] = (unsigned short*) realloc(_pFrmDllBuffer[i], dmaInfo->bufferSize);
				if(_pFrmDllBuffer[i] == NULL)
				{
					StringCbPrintfW(message,_MAX_PATH,L"ThorLSMCam unable to allocate pFrmDllBuffer size(%d) at dma(%u)", dmaInfo->bufferSize, i);
					Logger::getInstance().LogMessage(message,ERROR_EVENT);
				}
			}
			//clear unused dma memory
			if (_bufferInfo.dmaCount > dmaCount)
			{
				for (size_t j = dmaCount; j < _bufferInfo.dmaCount; j++)
				{
					if (NULL != _pFrmDllBuffer[j])
					{
						free(_pFrmDllBuffer[j]);
						_pFrmDllBuffer[j] = NULL;
					}
				}
			}
			_bufferInfo.bufferSize = dmaInfo->bufferSize;
			_bufferInfo.dmaCount = dmaCount;
			_bufferInfo.sizeX = dmaInfo->sizeX;
			_bufferInfo.sizeY = dmaInfo->sizeY;
			_bufferInfo.scanAreaCount = static_cast<long>(_preScan->ScanAreas.size());
			resetMem = 1;
		}
		if (resetMem)
		{
			//blank the first since the rest will be updated by previous
			memset(_pFrmDllBuffer[0], 0x0, _bufferInfo.bufferSize);
			//set frame info for first partial frame
			FrameInfo frameInfo = {_bufferInfo.sizeX, _bufferInfo.sizeY, static_cast<long>(_channels.size()), 0, (0<_bufferInfo.scanAreaCount) ? _preScan->ScanAreas[0]->ScanAreaID : 0, BufferType::INTENSITY, 0};
			char* dstBuffer = (char*)_pFrmDllBuffer[0] + _bufferInfo.bufferSize - sizeof(FrameInfo);
			SAFE_MEMCPY(dstBuffer, sizeof(FrameInfo), &frameInfo);
			//<key, value>: <dma buffer index, completed frame index>
			_bufferOrder.clear();
			for (long i = 0; i < static_cast<long>(dmaCount); i++)
			{
				_bufferOrder.insert(pair<long, long long>(i, _indexOfLastCompletedFrame));
			}
		}
	}
	catch(...)
	{
		Logger::getInstance().LogMessage(L"ThorLSMCam::ResetDMABuffers set DMA buffer error.", ERROR_EVENT);
		ret = FALSE;
	}
	_dmaMutex.unlock();
	return ret;
}
