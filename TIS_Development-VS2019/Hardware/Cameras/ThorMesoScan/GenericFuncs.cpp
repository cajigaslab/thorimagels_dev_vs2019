#include "ThorMesoScan.h"
#include "Strsafe.h"
#include "Logger.h"
#include "MesoWaveformManager.h"
#include "..\..\..\Common\BinaryImageDataUtilities\GenericImage.h"

void StaticImageComplete(ImageCompleteStates state, ChanBufferInfo info, FrameROI roi, unsigned short* buffer, unsigned int size)
{
	ThorLSMCam::getInstance()->ImageComplete(state, info, roi, buffer, size);
}

///	***************************************** <summary> Implement abstract functions </summary>	********************************************** ///
long ThorLSMCam::FindCameras(long &cameraCount)
{
	long ret = FALSE;
	if (CheckHardware())
	{		
		cameraCount = 1;
		ret = TRUE;
	}
	else
	{
		cameraCount = 0;
		Logger::getInstance().LogMessage(L"FindCameras error");
	}
	_pCameraConfig.reset(new CameraConfig());
	_hMesoScanWaveform->SetDefaultParams(_pCameraConfig.get());
	_hMesoScanWaveform->DefaultWaveformParams(_pCameraConfig.get());
	_hMesoScanWaveform->InitCallBack(std::bind(&ThorLSMCam::StripInfoChanged, this, std::placeholders::_1));
	return ret;
}

long ThorLSMCam::SelectCamera(const long camera)
{
	if (!CheckHardware())
	{
		Logger::getInstance().LogMessage(L"The hardware has not been located");
		return FALSE;
	}
	CloseLSMCam();

	MoveLightToPosition(GALVO_PARK_TYPE::PARK_NEAR_START);

	_hAlazarBoard = AlazarBoard9440::SelectAlazarBoard(1);
	if (_hAlazarBoard == NULL)
		return FALSE;

	if ((FALSE == _hDAQController->InitDAQBoard()))
		return FALSE;

	CircleBuffer* buffer = NULL;
	if (TRUE == _hDAQController->CheckTask(_pCameraConfig.get()->P1_GY_GX1_P2_AO_CHANNEL))
	{
		_hMesoScanWaveform->SetupCircularBuffer(1);
		buffer = _hMesoScanWaveform->GetWaveformBuffer(1);
		_hDAQController->RigisterTasks("P1_GY_GX1_P2_AO_CHANNEL", _pCameraConfig.get()->P1_GY_GX1_P2_AO_CHANNEL, AO, (IBuffer*)buffer, CHANNELS_IN_BUFFER[1]);
	}
	//DAQController will determine if Z stage line is on valid card
	if (TRUE == _hDAQController->CheckTask(_pCameraConfig.get()->VOICECOIL_AO_CHANNEL))
	{
		_hMesoScanWaveform->SetupCircularBuffer(0);
		buffer = _hMesoScanWaveform->GetWaveformBuffer(0);
		_hDAQController->RigisterTasks("VOICECOIL_AO_CHANNEL", _pCameraConfig.get()->VOICECOIL_AO_CHANNEL, AO, (IBuffer*)buffer, CHANNELS_IN_BUFFER[0]);
	}
	if (TRUE == _hDAQController->CheckTask(_pCameraConfig.get()->GX2_AO_CHANNEL))
	{
		_hMesoScanWaveform->SetupCircularBuffer(3);
		buffer = _hMesoScanWaveform->GetWaveformBuffer(3);
		_hDAQController->RigisterTasks("GX2_AO_CHANNEL", _pCameraConfig.get()->GX2_AO_CHANNEL, AO, (IBuffer*)buffer, CHANNELS_IN_BUFFER[3]);
	}
	if (TRUE == _hDAQController->CheckTask(_pCameraConfig.get()->FRAMETRIGGEROUT_DO_CHANNEL))
	{
		_hMesoScanWaveform->SetupCircularBuffer(2);
		buffer = _hMesoScanWaveform->GetWaveformBuffer(2);
		_hDAQController->RigisterTasks("FRAMETRIGGEROUT_DO_CHANNEL", _pCameraConfig.get()->FRAMETRIGGEROUT_DO_CHANNEL, DO, (IBuffer*)buffer, CHANNELS_IN_BUFFER[2]);
	}
	if (TRUE == _hDAQController->CheckTask(_pCameraConfig.get()->TRIGGER_IN_CHANNEL))
	{
		_hDAQController->RigisterTasks("TRIGGER_IN_CHANNEL", _pCameraConfig.get()->TRIGGER_IN_CHANNEL, CI, _hHardwareTriggerInEvent, 1);
	}

	InitCallBack(&StaticImageComplete);
	return TRUE;
}

long ThorLSMCam::TeardownCamera()
{
	Logger::getInstance().LogMessage(L"ThorLSMCam::TeardownCamera", INFORMATION_EVENT);
	StopScan();

	// park to outside of the FOV
	MoveLightToPosition(GALVO_PARK_TYPE::PARK_AT_EXIT);

	CloseLSMCam();

	_bufferInfo.scanAreaCount = _bufferInfo.sizeX = _bufferInfo.sizeY = 0;
	_bufferInfo.bufferSize = _bufferInfo.dmaCount = 0;
	for (int j = 0; j < MAX_DMABUFNUM; j++)
	{
		SAFE_DELETE_MEMORY(_pFrmDllBuffer[j]);
	}
	return TRUE;
}

long ThorLSMCam::PreflightAcquisition(char * pDataBuffer)
{
	long ret = FALSE;
	::EnterCriticalSection(&_accessSection);
	if ((unsigned int)ScanMode::CENTER == _pCameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_SCANMODE))
	{
		ret = MoveLightToPosition(GALVO_PARK_TYPE::PARK_AT_CENTER);
	}
	else
	{
		ret = SetupParamAndWaveforms();
	}
	::LeaveCriticalSection(&_accessSection);
	return ret;
}

long ThorLSMCam::SetupAcquisition(char * pDataBuffer)
{
	//[Mesoscope] SetupAcquisition after setting PARAM_MESO_SCAN_INFO to reset w/o stopping scanner,
	//use PARAM_LSM_FORCE_SETTINGS_UPDATE instead to reset w/ or w/o stopping scanner.
	return (!IsHardwareReady()) ? FALSE : TRUE;
}

long ThorLSMCam::StartAcquisition(char *pDataBuffer)
{
	if (!IsHardwareReady() || _hDAQController->IsRunning() || 
		(unsigned int)ScanMode::CENTER == _pCameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_SCANMODE))
		return TRUE;
	return StartScan();
}

long ThorLSMCam::StatusAcquisition(long &status)
{
	if (!IsHardwareReady())
		return FALSE;

	try
	{
		//return if DAQ is failed and not running
		if (!_hDAQController->IsRunning())
		{
			status = ICamera::STATUS_ERROR;
			return FALSE;
		}
		//return FALSE to break out while loop if user changed params in free-run mode
		if ((ICamera::SW_FREE_RUN_MODE == _triggerMode) && (_hAlazarBoard->IsStopping()))
		{
			status = ICamera::STATUS_READY;
			return FALSE;
		}

		//for the first frame, return actual status in acquiring current frame for potential partial update
		if((0 > _indexOfLastCompletedFrame) || (_indexOfLastCopiedFrame == _indexOfLastCompletedFrame))
		{
			status = _acquireStatus;
		}
		else
		{
			//for the remaing frames check the index of the frame against the last copied frame,
			//last completed DMA buffer index is (_bufCompleteID-1) since _bufCompleteID is incremented after complete of one frame
			long long comparisonIndex = _bufferOrder.at(((_bufCompleteID-1+static_cast<long>(_bufferInfo.dmaCount))%static_cast<long>(_bufferInfo.dmaCount)));

			status = ICamera::STATUS_READY;

			//move the source pointer if the indexes don't match
			if((_indexOfLastCopiedFrame+1) <= comparisonIndex)
			{
				//If in live mode, change index to the last acquired image from the same scan area
				//this allows for a more realtime feel when displaying the latest image
				if ((int)ICamera::SW_FREE_RUN_MODE == _triggerMode)
				{
					_indexOfLastCopiedFrame = comparisonIndex - 1;
				}

				long long frameDifference = max(0,min(static_cast<long long>(_bufferInfo.dmaCount)-1, (comparisonIndex-_indexOfLastCopiedFrame)));
#ifdef LOGGING_ENABLED 
				StringCbPrintfW(message,_MAX_PATH,L"StatusProtocol: frameDifference %lld comparisonIndex %lld _indexOfLastCopiedFrame %lld indexOfLastCompletedFrame %lld",frameDifference,comparisonIndex,_indexOfLastCopiedFrame,_indexOfLastCompletedFrame);
				Logger::getInstance().LogMessage(message,INFORMATION_EVENT);
#endif
				//if the index is beyond the dma buffer count, return an error.
				//Frames have been lost at this point
				if((0 < frameDifference) && (comparisonIndex-_indexOfLastCopiedFrame > static_cast<long long>(_bufferInfo.dmaCount)-1))
				{				
					StringCbPrintfW(message,_MAX_PATH,L"ThorMesoScan::%hs@%u Error: Overflow",__FUNCTION__, __LINE__);
					Logger::getInstance().LogMessage(message,VERBOSE_EVENT);
					status = _acquireStatus = ICamera::STATUS_ERROR;
				}
			}			
		}
		return TRUE;
	}
	catch(...)
	{
		Logger::getInstance().LogMessage(L"ThorMesoScan::StatusAcquisition Unhandled exception",ERROR_EVENT);
		return FALSE;
	}
}

long ThorLSMCam::StatusAcquisitionEx(long &status,long &indexOfLastCompletedFrame)
{
	if (!IsHardwareReady())
		return FALSE;
	long ret = TRUE;
	return ret;
}

long ThorLSMCam::CopyAcquisition(char *pDataBuffer, void* frameInfo)
{
	if (!IsHardwareReady())
		return FALSE;

	try
	{
		//do not capture data if you are in the centering or bleaching scan mode, 
		//return without copy if user changed params in free-run mode
		if((ScanMode::CENTER == _pCameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_SCANMODE)) || 
			(ScanMode::BLEACH_SCAN == _pCameraConfig->GetParameter<unsigned int>(ICamera::Params::PARAM_LSM_SCANMODE)) ||
			((ICamera::SW_FREE_RUN_MODE == _triggerMode) && (_hAlazarBoard->IsStopping() && !_hDAQController->IsRunning())))
		{
			return FALSE;
		}

		long long frameDifference = -1;		//0 or greater: a full frame is ready, otherwise: partial frame or busy in acquire
		unsigned short *pS =  NULL;

		if(_indexOfLastCopiedFrame == _indexOfLastCompletedFrame)
		{
			//try lock since partial update is a lot faster than actual frame rate
			if(_dmaMutex.try_lock_for((std::chrono::milliseconds)(Constants::TIMEOUT_MS / Constants::HUNDRED_PERCENT)))
			{
#ifdef LOGGING_ENABLED
				//current is in acquire
				StringCbPrintfW(message,_MAX_PATH,L"ThorMesoScan:%hs@%u: copying partial dma frame of %d", __FILE__, __LINE__, _bufCompleteID);
				Logger::getInstance().LogMessage(message,INFORMATION_EVENT);
#endif
				//move the source to current acquiring dma buffer
				pS = _pFrmDllBuffer[_bufCompleteID];
			}
		}
		else if (_indexOfLastCopiedFrame < _indexOfLastCompletedFrame)
		{
			//wait if the frame is locked, must copy completed frame
			_dmaMutex.lock();

			//last completed DMA buffer index is (_bufCompleteID-1) since _bufCompleteID is incremented after complete of one frame
			long long comparisonIndex = _bufferOrder.at(((_bufCompleteID-1+static_cast<long>(_bufferInfo.dmaCount))%static_cast<long>(_bufferInfo.dmaCount)));

			//we are about to copy a new frame, increment the copied frame count
			_indexOfLastCopiedFrame++;

			frameDifference = max(0,min(static_cast<long long>(_bufferInfo.dmaCount)-1, (comparisonIndex-_indexOfLastCopiedFrame)));
#ifdef LOGGING_ENABLED 
			StringCbPrintfW(message,_MAX_PATH,L"CopyProtocol: frameDifference %lld comparisonIndex %lld indexOfLastCopiedFrame %lld indexOfLastCompletedFrame %lld",frameDifference,comparisonIndex,_indexOfLastCopiedFrame,_indexOfLastCompletedFrame);
			Logger::getInstance().LogMessage(message,INFORMATION_EVENT);
#endif
			if((0 < frameDifference) && (comparisonIndex-_indexOfLastCopiedFrame > static_cast<long long>(_bufferInfo.dmaCount) - 1))
			{
				//this is an error message
				//indicating that a buffer overflow has occured
				StringCbPrintfW(message,_MAX_PATH,L"BUFFER OVERFLOW frameDifference %lld is greater than dmabuffercount %llu",comparisonIndex-_indexOfLastCopiedFrame,_bufferInfo.dmaCount-1);
				Logger::getInstance().LogMessage(message,ERROR_EVENT);
			}
			else
			{
				//move the source to the appropriate history buffer,
				//last completed DMA buffer index is (_bufCompleteID-1) since _bufCompleteID is incremented after complete of one frame
				pS = _pFrmDllBuffer[((_bufCompleteID-1+_bufferInfo.dmaCount-frameDifference)%_bufferInfo.dmaCount)];
			}
		}

		//copy buffer, allow copy current for partial frame update
		if(NULL != pS)
		{
			//pass over frame info
			SAFE_MEMCPY(frameInfo, sizeof(FrameInfo), ((char*)pS + _bufferInfo.bufferSize - sizeof(FrameInfo)));

			unsigned short *pD = (unsigned short*)pDataBuffer;
			if(FALSE == _pCameraConfig.get()->GetParameter<unsigned int>(ICamera::PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY))
			{
				SAFE_MEMCPY((char*)pD, _bufferInfo.bufferSize - sizeof(FrameInfo), (char*)pS);
			}
			else
			{
				std::vector<int> channels;
				std::transform(_channels.begin(),_channels.end(), std::back_inserter(channels),[](Channel const x) { return x.ChannelID; });
				((FrameInfo*)frameInfo)->channels = static_cast<long>(channels.size());
				GenericImage<unsigned short> sourceImage(((FrameInfo*)frameInfo)->imageWidth,((FrameInfo*)frameInfo)->imageHeight,1,1,static_cast<int>(_channels.size()),1,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL, channels, pS);
				GenericImage<unsigned short> destImage  (((FrameInfo*)frameInfo)->imageWidth,((FrameInfo*)frameInfo)->imageHeight,1,1,static_cast<int>(_channels.size()),1,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL, channels, pD);
				destImage.copyFrom(sourceImage);
				((FrameInfo*)frameInfo)->channels = static_cast<long>(channels.size());
			}
			((FrameInfo*)frameInfo)->copySize = ((FrameInfo*)frameInfo)->imageWidth * ((FrameInfo*)frameInfo)->imageHeight * ((FrameInfo*)frameInfo)->channels * sizeof(USHORT);
			//expost dropped frame count
			_pCameraConfig.get()->SetParameter(ICamera::PARAM_DROPPED_FRAMES, static_cast<unsigned int>(max(0, frameDifference)));
		}
		else
		{
			//no buffer copied, invalid frame info
			((FrameInfo*)frameInfo)->scanAreaID = ((FrameInfo*)frameInfo)->fullFrame = ((FrameInfo*)frameInfo)->channels = ((FrameInfo*)frameInfo)->imageWidth = ((FrameInfo*)frameInfo)->imageHeight = -1;
			((FrameInfo*)frameInfo)->bufferType = BufferType::INTENSITY;
			((FrameInfo*)frameInfo)->copySize = 0;
		}
	}
	catch(...)
	{
		Logger::getInstance().LogMessage(L"ThorMesoScan CopyAcquisition error.",ERROR_EVENT);
	}
	_dmaMutex.unlock();
	return TRUE;
}

long ThorLSMCam::InitCallBack(ImageCompleteCallback imageCompleteCallback)
{
	if (!IsHardwareReady())
		return FALSE;
	_hAlazarBoard->InitCallBack(imageCompleteCallback);
	return TRUE;
}

long ThorLSMCam::PostflightAcquisition(char * pDataBuffer)
{
	if (!IsHardwareReady())
		return FALSE;

	long ret = StopScan();

	//reset params in case preflight invoked later
	_pCameraConfig->SetParameter(ICamera::Params::PARAM_MULTI_FRAME_COUNT, (unsigned int)0);
	if (NULL != _preScan)
		_preScan->ScanConfig.IsLivingMode = 0;

	MoveLightToPosition(GALVO_PARK_TYPE::PARK_NEAR_START);

	return ret;
}

long ThorLSMCam::SaveConfigration()
{
	return _pCameraConfig.get()->SaveConfigFile();
}

//**********************************	Additional Functions	**********************************//

long ThorLSMCam::CheckHardware()
{
	_hDAQController->_boardInfoNI.get()->getInstance()->GetAllBoardsInfo();

	long ret = ((TRUE == AlazarBoard9440::FindAlazarBoard()) &&
		(NULL != _hDAQController->_boardInfoNI.get()->getInstance()->GetBoardInfo("Dev1")) && 
		(NULL != _hDAQController->_boardInfoNI.get()->getInstance()->GetBoardInfo("Dev2"))
		) ? TRUE : FALSE;

	if (FALSE == ret)
		return FALSE;

	//test to verify RTSI is configured by connecting terminals across devices
	std::string testPFI1 = "/Dev1/PFI0", testPFI2 = "/Dev2/PFI0";
	if (DAQmxSuccess != DAQmxConnectTerms(testPFI1.c_str(), testPFI2.c_str(), DAQmx_Val_DoNotInvertPolarity)) 
	{
		Logger::getInstance().LogMessage(L"ResonanceGalvoGalvo (RGG) CheckHardware Failed: RTSI was not configured correctly.");
		ret = FALSE;
	}
	DAQmxDisconnectTerms(testPFI1.c_str(), testPFI2.c_str());
	return ret;
}

long ThorLSMCam::SetupParamAndWaveforms()
{
	if (!IsHardwareReady() || _hMesoScanWaveform->IsRunning())
		return TRUE;
	if(!ReadPosition()) { Logger::getInstance().LogMessage(L"SetupParamAndWaveforms, ReadPosition failed!"); return FALSE; }
	if (!SetParameters()) { Logger::getInstance().LogMessage(L"SetupParamAndWaveforms, SetParameters failed!"); return FALSE; }
	if (!StartWaveform()) { Logger::getInstance().LogMessage(L"SetupParamAndWaveforms, StartWaveform failed!"); return FALSE; }
	return TRUE;
}

void ThorLSMCam::ImageComplete(ImageCompleteStates state, ChanBufferInfo info, FrameROI roi, unsigned short* buffer, unsigned int size)
{
	unsigned short* dstBuffer = NULL;
	FrameInfo tFrameInfo = {0};
	switch (state)
	{
	case SCAN_ERROR:
		_acquireStatus = (int)ICamera::STATUS_ERROR;
		break;
	case SCAN_ABORT:
	case SCAN_COMPLETE:
		//for status to return correctly
		_acquireStatus = (int)ICamera::STATUS_READY;
		Logger::getInstance().LogMessage(L"ThorMesoScan ImageComplete ABORT or COMPLETE.", VERBOSE_EVENT);
		break;
	case SCAN_IMAGE:
	case SCAN_IMAGE_STRIPEEND:
		if (size < roi.height * roi.width)
		{
			_acquireStatus = (int)ICamera::STATUS_ERROR;
			return;
		}
		tFrameInfo.imageWidth = roi.frameWidth;
		tFrameInfo.imageHeight = roi.frameHeight;
		tFrameInfo.channels = static_cast<long>(_channels.size());
		tFrameInfo.scanAreaID = info.ScanAreaID;
		tFrameInfo.bufferType = BufferType::INTENSITY;

		_dmaMutex.lock();

		//circular dma buffer index
		_bufCompleteID %= _bufferInfo.dmaCount;

		//map to target ROI of current scan area
		dstBuffer = _pFrmDllBuffer[_bufCompleteID] + (info.ChannelID * roi.frameWidth * roi.frameHeight);
		for (unsigned int j = 0; j < roi.height; j++)
		{
			for (unsigned int i = 0; i < roi.width; i++)
			{
				dstBuffer[(roi.y + j)*roi.frameWidth + roi.x + i] = buffer[j*roi.width + i];
			}
		}

		if ((SCAN_IMAGE_STRIPEEND == state) &&
			(roi.x + roi.width == roi.frameWidth) && (roi.y + roi.height == roi.frameHeight))
		{
			//full frame is acquired, copy to next for update
			SAFE_MEMCPY(_pFrmDllBuffer[(_bufCompleteID+1+static_cast<long>(_bufferInfo.dmaCount))%static_cast<long>(_bufferInfo.dmaCount)], _bufferInfo.bufferSize, _pFrmDllBuffer[_bufCompleteID]);
			_bufferOrder.at(_bufCompleteID) = _indexOfLastCompletedFrame + 1;

			//imprint frame info to the end of frame buffer
			tFrameInfo.fullFrame = 1;
			dstBuffer = (unsigned short*)((char*)_pFrmDllBuffer[_bufCompleteID] + _bufferInfo.bufferSize - sizeof(FrameInfo));

			//[REMARK]: status will be READY at StatusAcq after increment.
			//_acquireStatus should only be PARTIAL, BUSY or ERROR. 
			//No logger should be invoked here (will overwrite message @ StatusAcq).
			_bufCompleteID++;
			_indexOfLastCompletedFrame++;
		}
		else
		{
			//imprint frame info to the end of frame buffer
			tFrameInfo.fullFrame = 0;
			dstBuffer = (unsigned short*)((char*)_pFrmDllBuffer[_bufCompleteID] + _bufferInfo.bufferSize - sizeof(FrameInfo));
			_acquireStatus = (int)ICamera::STATUS_PARTIAL;
		}
		SAFE_MEMCPY(dstBuffer, sizeof(FrameInfo), &tFrameInfo);
		_dmaMutex.unlock();
		break;
	case SCAN_BUSY:
		_acquireStatus = (int)ICamera::STATUS_BUSY;
		break;
	default:
		break;
	}
}
