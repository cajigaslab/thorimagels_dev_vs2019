// GeneticFuncs.cpp : Defines abstract/generic functions for the DLL application.
//

#include "stdafx.h"
#include "ThorGGNI.h"
#include "ThorGGNISetupXML.h"
#include "Strsafe.h"

auto_ptr<ThorGGNIXML> pSetup(new ThorGGNIXML());
std::unique_ptr<BoardInfoNI> boardInfoNI;

///	***************************************** <summary> Generic functions </summary>	********************************************** ///

long ThorLSMCam::InitialImageProperties()
{
	//reset dropped frame count:
	_droppedFramesCnt = 0;
	_ImgPty.fieldSize = _fieldSize;
	_ImgPty.pixelX = _pixelX;
	_ImgPty.pixelY = _pixelY;
	_ImgPty.offsetX = _offsetX;
	_ImgPty.offsetY = _offsetY;
	_ImgPty.channel = _channel;
	_ImgPty.averageMode = _averageMode;
	_ImgPty.averageNum = _averageNum;
	_ImgPty.scanMode = _scanMode;
	_ImgPty.alignmentForField = _alignmentForField;
	_ImgPty.clockRateInternal = _clockRateInternal;
	_ImgPty.clockRateExternal = _clockRateExternal;
	_ImgPty.clockSource = _clockSource;
	_ImgPty.triggerMode = _triggerMode;
	_ImgPty.numFrame = _frameCount;
	_ImgPty.yAmplitudeScaler = _yAmplitudeScaler;
	_ImgPty.areaMode = _areaMode;
	_ImgPty.flybackCycle = getFlybackCycle();
	_ImgPty.dataMapMode = _dataMapMode;
	_ImgPty.dwellTime = _dwellTime;
	_ImgPty.rasterAngle = _rasterAngle;
	_ImgPty.galvoForwardLineDuty = _galvoForwardLineDuty;
	_ImgPty.progressCounter = _progressCounter;
	_ImgPty.galvoEnable = _galvoEnable;
	_ImgPty.yChannelEnable = _yChannelEnable;

	for(long i=0; i<MAX_GG_POCKELS_CELL_COUNT; i++)
	{
		_ImgPty.pockelsLineBlankingPercentage[i] = _pockelsLineBlankingPercentage[i];
		_ImgPty.pockelsPowerLevel[i] = _pockelsPowerLevel[i];
	}

	_ImgPty.verticalScanDirection = _verticalScanDirection;
	_ImgPty.horizontalFlip = _horizontalFlip;
	_ImgPty.fineOffset[0] = _fineOffset[0];
	_ImgPty.fineOffset[1] = _fineOffset[1];
	_ImgPty.fineFieldSizeScaleX = _fineFieldSizeScaleX;
	_ImgPty.fineFieldSizeScaleY = _fineFieldSizeScaleY;
	_ImgPty.scanAreaAngle = _scanAreaAngle;
	_ImgPty.dmaBufferCount = _dMABufferCount;
	_ImgPty.useReferenceForPockelsOutput = _useReferenceForPockelsOutput;
	std::memcpy(&_ImgPty.channelPolarity,&_channelPolarity,sizeof(_ImgPty.channelPolarity));
	_ImgPty.interleaveScan = _interleaveScan;

	//[TaskMaster] reset counts before setting up NI tasks:
	ThorLSMCam::_finishedCycleCnt = ThorLSMCam::_triggeredCycleCnt = 0;
	for (int i = 0; i < SignalType::SIGNALTYPE_LAST; i++)
	{
		ThorLSMCam::_currentIndex[i] = ThorLSMCam::_totalLength[i] = 0;
	}
	//force trigger first if single frame in trigger each:
	if((1 == _frameCount) && (ICamera::HW_MULTI_FRAME_TRIGGER_EACH == _triggerMode))
	{
		_triggerMode = ICamera::HW_MULTI_FRAME_TRIGGER_FIRST;
	}
	//check pre-capture status:
	if (1 < _frameCount)
	{
		ThorLSMCam::_precaptureStatus = PreCaptureStatus::PRECAPTURE_WAVEFORM_MID_CYCLE;
	}
	//reset params to force rebuild waveform:
	ImageWaveformBuilder->ResetGGalvoWaveformParams();

	//return optical angle check for preflight imaging
	return CheckOpticalAngle(_fieldSize, _dwellTime, _pixelX);
}

void ThorLSMCam::PersistImageProperties()
{
	_ImgPty_Pre.fieldSize = _ImgPty.fieldSize;
	_ImgPty_Pre.pixelX = _ImgPty.pixelX;
	_ImgPty_Pre.pixelY = _ImgPty.pixelY;
	_ImgPty_Pre.offsetX = _ImgPty.offsetX;
	_ImgPty_Pre.offsetY = _ImgPty.offsetY;
	_ImgPty_Pre.channel = _ImgPty.channel;
	_ImgPty_Pre.averageMode = _ImgPty.averageMode;
	_ImgPty_Pre.averageNum = _ImgPty.averageNum;
	_ImgPty_Pre.scanMode = _ImgPty.scanMode;
	_ImgPty_Pre.alignmentForField = _ImgPty.alignmentForField;
	_ImgPty_Pre.clockRateInternal = _ImgPty.clockRateInternal;
	_ImgPty_Pre.clockRateExternal = _ImgPty.clockRateExternal;
	_ImgPty_Pre.clockSource = _ImgPty.clockSource;
	_ImgPty_Pre.yAmplitudeScaler = _ImgPty.yAmplitudeScaler;
	_ImgPty_Pre.areaMode = _ImgPty.areaMode;
	_ImgPty_Pre.flybackCycle = _ImgPty.flybackCycle;
	_ImgPty_Pre.dataMapMode = _ImgPty.dataMapMode;
	_ImgPty_Pre.rasterAngle = _ImgPty.rasterAngle;
	_ImgPty_Pre.dwellTime = _ImgPty.dwellTime;
	_ImgPty_Pre.galvoForwardLineDuty = _ImgPty.galvoForwardLineDuty;
	_ImgPty_Pre.triggerMode = _ImgPty.triggerMode;
	_ImgPty_Pre.progressCounter = _ImgPty.progressCounter;
	_ImgPty_Pre.galvoEnable = _ImgPty.galvoEnable;
	_ImgPty_Pre.yChannelEnable = _ImgPty.yChannelEnable;
	for(long i=0; i<MAX_GG_POCKELS_CELL_COUNT; i++)
	{
		_ImgPty_Pre.pockelsLineBlankingPercentage[i] = _ImgPty.pockelsLineBlankingPercentage[i];
		_ImgPty_Pre.pockelsPowerLevel[i] = _ImgPty.pockelsPowerLevel[i];
	}
	_ImgPty_Pre.verticalScanDirection = _ImgPty.verticalScanDirection;
	_ImgPty_Pre.horizontalFlip = _ImgPty.horizontalFlip;
	_ImgPty_Pre.fineOffset[0] = _ImgPty.fineOffset[0];
	_ImgPty_Pre.fineOffset[1] = _ImgPty.fineOffset[1];
	_ImgPty_Pre.fineFieldSizeScaleX = _ImgPty.fineFieldSizeScaleX;
	_ImgPty_Pre.fineFieldSizeScaleY = _ImgPty.fineFieldSizeScaleY;
	_ImgPty_Pre.scanAreaAngle = _ImgPty.scanAreaAngle;
	_ImgPty_Pre.dmaBufferCount = _ImgPty.dmaBufferCount;
	_ImgPty_Pre.numFrame = _ImgPty.numFrame;
	_ImgPty_Pre.useReferenceForPockelsOutput = _ImgPty.useReferenceForPockelsOutput;
	std::memcpy(&_ImgPty_Pre.channelPolarity,&_ImgPty.channelPolarity,sizeof(_ImgPty_Pre.channelPolarity));
	_ImgPty_Pre.interleaveScan = _ImgPty.interleaveScan;

}

long ThorLSMCam::SetupCheck()
{
	long ret = FALSE;

	if ((_ImgPty_Pre.fieldSize != _fieldSize) ||
		(_ImgPty_Pre.pixelX != _pixelX) ||
		(_ImgPty_Pre.pixelY != _pixelY) ||
		(_ImgPty_Pre.offsetX != _offsetX) ||
		(_ImgPty_Pre.offsetY != _offsetY) ||
		(_ImgPty_Pre.channel != _channel) ||
		(_ImgPty_Pre.averageMode != _averageMode) ||
		(_ImgPty_Pre.averageNum != _averageNum) ||
		(_ImgPty_Pre.scanMode != _scanMode) ||
		(_ImgPty_Pre.alignmentForField != _alignmentForField) ||
		(_ImgPty_Pre.clockRateInternal != _clockRateInternal) ||
		(_ImgPty_Pre.clockSource != _clockSource)||
		(_ImgPty_Pre.yAmplitudeScaler != _yAmplitudeScaler)||
		(_ImgPty_Pre.areaMode != _areaMode)||
		(_ImgPty_Pre.flybackCycle != getFlybackCycle())||
		(_ImgPty_Pre.dataMapMode != _dataMapMode)||
		(_ImgPty_Pre.rasterAngle != _rasterAngle)||
		(_ImgPty_Pre.dwellTime != _dwellTime)||
		(_ImgPty_Pre.galvoForwardLineDuty != _galvoForwardLineDuty)||
		(_ImgPty_Pre.triggerMode != _triggerMode) ||
		(_ImgPty_Pre.progressCounter != _progressCounter) ||
		(_ImgPty_Pre.galvoEnable != _galvoEnable) ||
		(_ImgPty_Pre.yChannelEnable != _yChannelEnable)||
		(memcmp(&_ImgPty_Pre.pockelsLineBlankingPercentage,&_pockelsLineBlankingPercentage,sizeof(_pockelsLineBlankingPercentage))!=0)||
		(memcmp(&_ImgPty_Pre.pockelsPowerLevel,&_pockelsPowerLevel,sizeof(_pockelsPowerLevel))!=0)||
		(_ImgPty_Pre.verticalScanDirection != _verticalScanDirection)||
		(_ImgPty_Pre.horizontalFlip != _horizontalFlip)||
		(_ImgPty_Pre.fineOffset[0] != _fineOffset[0])||
		(_ImgPty_Pre.fineOffset[1] != _fineOffset[1])||
		(_ImgPty_Pre.fineFieldSizeScaleX != _fineFieldSizeScaleX)||
		(_ImgPty_Pre.fineFieldSizeScaleY != _fineFieldSizeScaleY)||
		(_ImgPty_Pre.dmaBufferCount != _dMABufferCount)||
		(_ImgPty_Pre.numFrame != _frameCount)||
		(_ImgPty_Pre.scanAreaAngle != _scanAreaAngle)||
		(_ImgPty_Pre.useReferenceForPockelsOutput != _useReferenceForPockelsOutput) ||
		(memcmp(&_ImgPty_Pre.channelPolarity,&_channelPolarity,sizeof(_channelPolarity))!=0)||
		(_ImgPty_Pre.interleaveScan != _interleaveScan) ||
		(TRUE == _forceSettingsUpdate)  || 
		(NULL == _hThread)
		)
	{
		_forceSettingsUpdate = FALSE;

		_ImgPty.fieldSize = _fieldSize;
		_ImgPty.pixelX = _pixelX;
		_ImgPty.pixelY = _pixelY;
		_ImgPty.offsetX = _offsetX;
		_ImgPty.offsetY = _offsetY;
		_ImgPty.channel = _channel;
		_ImgPty.averageMode = _averageMode;
		_ImgPty.averageNum = _averageNum;
		_ImgPty.scanMode = _scanMode;
		_ImgPty.alignmentForField = _alignmentForField;
		_ImgPty.clockRateInternal = _clockRateInternal;
		_ImgPty.clockRateExternal = _clockRateExternal;
		_ImgPty.clockSource = _clockSource;
		_ImgPty.yAmplitudeScaler = _yAmplitudeScaler;
		_ImgPty.areaMode = _areaMode;
		_ImgPty.flybackCycle = getFlybackCycle();
		_ImgPty.dataMapMode = _dataMapMode;
		_ImgPty.rasterAngle = _rasterAngle;
		_ImgPty.dwellTime = _dwellTime;
		_ImgPty.galvoForwardLineDuty = _galvoForwardLineDuty;
		_ImgPty.triggerMode = _triggerMode;
		_ImgPty.progressCounter = _progressCounter;
		_ImgPty.galvoEnable = _galvoEnable;
		_ImgPty.yChannelEnable = _yChannelEnable;

		for(long i=0; i<MAX_GG_POCKELS_CELL_COUNT; i++)
		{
			_ImgPty.pockelsLineBlankingPercentage[i] = _pockelsLineBlankingPercentage[i];
			_ImgPty.pockelsPowerLevel[i] = _pockelsPowerLevel[i];
		}
		_ImgPty.verticalScanDirection = _verticalScanDirection;
		_ImgPty.horizontalFlip = _horizontalFlip;
		_ImgPty.fineOffset[0] = _fineOffset[0];
		_ImgPty.fineOffset[1] = _fineOffset[1];
		_ImgPty.fineFieldSizeScaleX = _fineFieldSizeScaleX;
		_ImgPty.fineFieldSizeScaleY = _fineFieldSizeScaleY;
		_ImgPty.dmaBufferCount = _dMABufferCount;
		_ImgPty.numFrame = _frameCount;
		_ImgPty.scanAreaAngle = _scanAreaAngle;
		_ImgPty.useReferenceForPockelsOutput = _useReferenceForPockelsOutput;
		std::memcpy(&_ImgPty.channelPolarity,&_channelPolarity,sizeof(_ImgPty.channelPolarity));
		_ImgPty.interleaveScan = _interleaveScan;

		CloseThread();
		ret = TRUE;
	}

	return ret;
}

long ThorLSMCam::SetupBoards()
{
	long error = 0;
	if(TRUE != SetWaveform(&_ImgPty))
	{
		DAQmxFailed(error);
		StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorGGNI SetWaveform failed. (%d)", error);
		LogMessage(_errMsg,ERROR_EVENT);
		goto SETUP_BOARD_ERROR;
	}
	if(ApiSuccess != SetBdDMA())
	{
		DAQmxFailed(error);
		StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorGGNI SetBdDMA failed. (%d)", error);
		LogMessage(_errMsg,ERROR_EVENT);
		goto SETUP_BOARD_ERROR;
	}
	return TRUE;

SETUP_BOARD_ERROR:
	StringCbPrintfW(_errMsg,_MAX_PATH, L"Unable to setup with current settings.\nChange settings and restart.");
	MessageBox(NULL,_errMsg,L"Board Setup Error", MB_OK | MB_SETFOREGROUND | MB_ICONERROR);
	return FALSE;
}

long ThorLSMCam::StartProtocol()
{
	try
	{
		if (CheckOpticalAngle(_imgPtyDll.fieldSize, _imgPtyDll.dwellTime, _imgPtyDll.pixelX))
		{
			switch(_imgPtyDll.triggerMode)
			{
			case ICamera::SW_FREE_RUN_MODE:
			case ICamera::SW_MULTI_FRAME:
				if (WaitForSingleObject(_hThreadStopped, 0) == WAIT_OBJECT_0)
				{
					return CaptureCreateThread();
				}
				break;
			case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
				if (WaitForSingleObject(_hThreadStopped, 0) == WAIT_OBJECT_0)
				{
					//wait for HW trigger if necessary
					if(!WaitForHardwareTrigger(_triggerWaitTimeout*Constants::MS_TO_SEC, TRUE))
						return FALSE;

					return CaptureCreateThread();
				}
				break;
			case ICamera::SW_SINGLE_FRAME:
				{
					//for single frame capture force the previous thread to stop and add an additional wait to ensure it completes.
					//if the previous thread does not close generate an error.
					//Stop and cancel previous scan
					SetEvent(_hStopAcquisition);

					if (WaitForSingleObject(_hThreadStopped, Constants::TIMEOUT_MS) != WAIT_OBJECT_0)
					{
						StringCbPrintfW(_errMsg,_MAX_PATH,L"StartAcquisition failed. Could not start a thread while thre previous thread is active");
						LogMessage(_errMsg,ERROR_EVENT);
						return FALSE;
					}
					return CaptureCreateThread();
				}
				break;
			case ICamera::HW_SINGLE_FRAME:
				{
					//for single frame capture force the previous thread to stop and add an additional wait to ensure it completes.
					//if the previous thread does not close generate an error.
					//Stop and cancel previous scan
					SetEvent(_hStopAcquisition);

					if (WaitForSingleObject(_hThreadStopped, Constants::TIMEOUT_MS) != WAIT_OBJECT_0)
					{
						StringCbPrintfW(_errMsg,_MAX_PATH,L"StartAcquisition failed. Could not start a thread while thre previous thread is active");
						LogMessage(_errMsg,ERROR_EVENT);
						return FALSE;
					}
					//wait for HW trigger if necessary
					if(!WaitForHardwareTrigger(_triggerWaitTimeout*Constants::MS_TO_SEC, TRUE))
						return FALSE;

					return CaptureCreateThread();
				}
				break;
			}
		}
	}
	catch(...)
	{
		long daqerror=0;
		DAQmxFailed(daqerror);
		StringCbPrintfW(_errMsg,_MAX_PATH,L"StartAcquisition failed. (%d))",daqerror);
		LogMessage(_errMsg,ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long ThorLSMCam::StatusProtocol(long &status)
{
	long ret = TRUE;
	try
	{
		//return FALSE to break out while loop if user changed params in free-run mode
		if ((ICamera::SW_FREE_RUN_MODE == _imgPtyDll.triggerMode) && (WAIT_OBJECT_0 == WaitForSingleObject(_hStopAcquisition, 0)) || !CheckOpticalAngle(_imgPtyDll.fieldSize, _imgPtyDll.dwellTime, _imgPtyDll.pixelX))
		{
			status = ICamera::STATUS_READY;
			return FALSE;
		}
		if (WAIT_OBJECT_0 == WaitForSingleObject(_hStatusError, 0))
		{
			StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorGGNI::StatusAcquisition: status error or timed out");
			LogMessage(_errMsg,VERBOSE_EVENT);
			status = ICamera::STATUS_ERROR;
			return TRUE;
		}

		//for the first frame wait for the thread to signal a completed frame
		if (0 > _indexOfLastCompletedFrame && ICamera::SW_FREE_RUN_MODE == _imgPtyDll.triggerMode)
		{
			if (WAIT_OBJECT_0 == WaitForSingleObject(_hFrmBufReady, 0))
			{
#ifdef LOGGING_ENABLED 
				StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorGGNI::StatusAcquisition Ready");
				LogMessage(_errMsg,VERBOSE_EVENT);
#endif
				status = ICamera::STATUS_READY;
			}
			else
			{
				//return actual status in acquiring current frame for potential partial update
				status = ThorLSMCam::_acquireStatus;
			}
		}
		else if (_indexOfLastCopiedFrame == _indexOfLastCompletedFrame && ICamera::SW_FREE_RUN_MODE == _imgPtyDll.triggerMode)
		{
			//current frame in acquisition
			status = ThorLSMCam::_acquireStatus;
		}
		else
		{
			//for the remaing frames check the index of the frame against the last copied frame,
			//when the index is greater than the copied frame then a frame is ready.
			//last completed DMA buffer index is (_bufCompleteID-1) since _bufCompleteID is incremented after complete of one frame
			long comparisonIndex = _bufferOrder.at(((_bufCompleteID-1+_imgPtyDll.dmaBufferCount)%_imgPtyDll.dmaBufferCount));

			status = ICamera::STATUS_BUSY;

			//move the source pointer if the indexes don't match
			if (_indexOfLastCopiedFrame < _indexOfLastCompletedFrame)
			{
				//If in live mode, change index to the last acquired image
				//this allows for a more realtime feel when displaying the latest image
				if (ICamera::SW_FREE_RUN_MODE == _imgPtyDll.triggerMode)
				{
					_indexOfLastCopiedFrame = comparisonIndex - 1;
				}

				status = ICamera::STATUS_READY;

				long frameDifference = max(0,min(_imgPtyDll.dmaBufferCount -1,(comparisonIndex-_indexOfLastCopiedFrame)));
#ifdef LOGGING_ENABLED 
				StringCbPrintfW(_errMsg,_MAX_PATH,L"StatusProtocol: frameDifference %d comparisonIndex %d _indexOfLastCopiedFrame %d indexOfLastCompletedFrame %d",frameDifference,comparisonIndex,_indexOfLastCopiedFrame,_indexOfLastCompletedFrame);
				LogMessage(_errMsg,INFORMATION_EVENT);
#endif
				//if the index is beyond the dma buffer count
				//return an error.
				//Frames have been lost at this point
				if((0 < frameDifference) && (comparisonIndex-_indexOfLastCopiedFrame > _imgPtyDll.dmaBufferCount -1))
				{				
					StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorGGNI::%hs@%u Error: Overflow",__FUNCTION__, __LINE__);
					LogMessage(_errMsg,VERBOSE_EVENT);
					status = ICamera::STATUS_ERROR;
				}
			}
		}

	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorGGNI::StatusAcquisition Unhandled exception");
		LogMessage(_errMsg,ERROR_EVENT);
	}
	return ret;
}

long ThorLSMCam::CopyProtocol(char *pDataBuffer)
{
	try
	{
		StringCbPrintfW(_errMsg,_MAX_PATH,L"Copy Acquisition called.");
		LogMessage(_errMsg,INFORMATION_EVENT);
		//do not capture data if you are in the centering or bleaching scan mode
		if((ScanMode::CENTER == _imgPtyDll.scanMode) || (ScanMode::BLEACH_SCAN == _imgPtyDll.scanMode))
		{
			return FALSE;
		}
		//return without copy if user changed params in free-run mode
		if ((ICamera::SW_FREE_RUN_MODE == _imgPtyDll.triggerMode) && (WAIT_OBJECT_0 == WaitForSingleObject(_hStopAcquisition, 0)))
		{
			return FALSE;
		}

		long width = _imgPtyDll.pixelX;
		long height = _imgPtyDll.pixelY;

		long frameDifference = -1;		//0 or greater: a full frame is ready, otherwise: partial frame or busy in acquire
		unsigned short *dst = (unsigned short*)pDataBuffer;
		unsigned short *pS = NULL;

		if(_indexOfLastCopiedFrame == _indexOfLastCompletedFrame)
		{		
			StringCbPrintfW(_errMsg,_MAX_PATH,L"Copy Acquisition indexes are equal. _indexOfLastCopiedFrame %d , _indexOfLastCompletedFrame %d",_indexOfLastCopiedFrame, _indexOfLastCompletedFrame);
			LogMessage(_errMsg,INFORMATION_EVENT);
			/* We need to use this part later when adding the partial frame loading to RunSample, need to keep track of the indexes
			//wait for a short period of time since period update of LINE_POST_COUNT is a lot faster than actual frame rate
			if (WAIT_OBJECT_0 == WaitForSingleObject(_hFrmBufHandle, (DWORD)(Constants::TIMEOUT_MS / Constants::HUNDRED_PERCENT)))
			{
#if defined (LOGGING_ENABLED) //&& defined (_DEBUG)
				//current frame (_bufCompleteID) is in acquire
				StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorConfocalGalvo:%hs@%u: copying partial dma frame of %d", __FILE__, __LINE__, _bufCompleteID);
				LogMessage(_errMsg,ERROR_EVENT);
#endif
				//move the source to current acquiring dma buffer
				pS =  _pFrmDllBuffer[_bufCompleteID];
			} */
		}
		else if (_indexOfLastCopiedFrame < _indexOfLastCompletedFrame)
		{
			StringCbPrintfW(_errMsg,_MAX_PATH,L"Copy Acquisition != _indexOfLastCopiedFrame %d , _indexOfLastCompletedFrame %d",_indexOfLastCopiedFrame, _indexOfLastCompletedFrame);
			LogMessage(_errMsg,INFORMATION_EVENT);
			//last completed DMA buffer index is (_bufCompleteID-1) since _bufCompleteID is incremented after complete of one frame
			long comparisonIndex = _bufferOrder.at(((_bufCompleteID-1+_imgPtyDll.dmaBufferCount)%_imgPtyDll.dmaBufferCount));

			//we are about to copy a new frame, increment the copied frame count
			_indexOfLastCopiedFrame++;

			frameDifference = max(0,min(_imgPtyDll.dmaBufferCount -1,(comparisonIndex-_indexOfLastCopiedFrame)));
#ifdef LOGGING_ENABLED 
			StringCbPrintfW(_errMsg,_MAX_PATH,L"CopyProtocol: frameDifference %d comparisonIndex %d indexOfLastCopiedFrame %d indexOfLastCompletedFrame %d",frameDifference,comparisonIndex,_indexOfLastCopiedFrame,_indexOfLastCompletedFrame);
			LogMessage(_errMsg,INFORMATION_EVENT);
#endif
			if((0 < frameDifference) && (comparisonIndex-_indexOfLastCopiedFrame > _imgPtyDll.dmaBufferCount - 1))
			{			
				//this is an error message
				//indicating that a buffer overflow has occured
				StringCbPrintfW(_errMsg,_MAX_PATH,L"BUFFER OVERFLOW frameDifference %d is greater than dmabuffercount %d",comparisonIndex-_indexOfLastCopiedFrame,_imgPtyDll.dmaBufferCount-1);
				LogMessage(_errMsg,ERROR_EVENT);
			}
			else
			{
				StringCbPrintfW(_errMsg, _MAX_PATH, L"buffer index %d, dma buffers %d", ((_bufCompleteID - 1 + _imgPtyDll.dmaBufferCount - frameDifference) % _imgPtyDll.dmaBufferCount), _imgPtyDll.dmaBufferCount);
				LogMessage(_errMsg, INFORMATION_EVENT);
				//wait if the frame is locked, must copy completed frame
				WaitForSingleObject(_hFrmBufHandle, INFINITE);

				//move the source to the appropriate history buffer,
				//last completed DMA buffer index is (_bufCompleteID-1) since _bufCompleteID is incremented after complete of one frame
				pS = _pFrmDllBuffer[((_bufCompleteID-1+_imgPtyDll.dmaBufferCount-frameDifference)%_imgPtyDll.dmaBufferCount)];
			}
		}

		//copy buffer, allow copy current for partial frame update
		if(NULL != pS)
		{
			if(_channel != 0x0100 && _channel != 0x1000)
			{
				if(_channel < 3)
				{
					unsigned short *pD = dst;
					unsigned long chan = _channel - 1;
					memcpy((void *) pD, (void *)(pS + chan * width * height), height * width * sizeof(unsigned short));
				}
				else
				{
					unsigned short *pD = dst;
					memcpy((void *) pD, (void *)pS, ChFlag * height * width * sizeof(unsigned short));
				}
			}
		}

		ReleaseMutex(_hFrmBufHandle);

		//if a full frame is ready
		if(0 <= frameDifference)
		{
			ResetEvent(_hFrmBufReady);
			//expost dropped frame count:
			_droppedFramesCnt = frameDifference;
			//generate pulse to signal frame buffer ready:
			SetFrameBufferReadyOutput();

			StringCbPrintfW(_errMsg,_MAX_PATH,L"Copy Acquisition frame ready. _droppedFramesCnt %d , frameDifference %d",_droppedFramesCnt, frameDifference);
			LogMessage(_errMsg,INFORMATION_EVENT);
		}

	}
	catch(...)
	{
		ReleaseMutex(_hFrmBufHandle);
		ResetEvent(_hFrmBufReady);
		long daqerror=0;
		DAQmxFailed(daqerror);
		StringCbPrintfW(_errMsg,_MAX_PATH,L"Copy Acquisition failed. (%d)",daqerror);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	return TRUE;
}

long ThorLSMCam::PostflightProtocol(long parkAtParking)
{
	try
	{
		//force the hardware trigger event if the post flight function is called
		SetEvent(_hHardwareTriggerInEvent);
		ResetEvent(_hHardwareTriggerInEvent);

		CloseThread();

		ThorCloseNITasks();

		MovePockelsToParkPosition();

		MoveGalvoToParkPosition(parkAtParking);

		//set digital lines to expected state if stopped by user:
		TogglePulseToDigitalLine(_taskHandleDO1, _bleachShutterLine, 1, TogglePulseMode::ToggleLow);
		TogglePulseToDigitalLine(_taskHandleDO1, _frameTriggerLineInOut, _digiLineSelect, TogglePulseMode::ToggleLow);
		TogglePulseToDigitalLine(_taskHandleDO1, _digitalTriggerLines, _digiBleachSelect, TogglePulseMode::ToggleLow);

		//reset params:
		_precaptureStatus = PreCaptureStatus::PRECAPTURE_DONE;

		//release for next fresh restart:
		SAFE_DELETE_MEMORY (_pGalvoStartPos);

	}
	catch(...)
	{
		long daqerror=0;
		DAQmxFailed(daqerror);

		StringCbPrintfW(_errMsg,_MAX_PATH,L"PostflightProtocol failed. (%d)",daqerror);
		LogMessage(_errMsg,ERROR_EVENT);
	}
	return TRUE;
}

///	***************************************** <summary> Implement abstract functions </summary>	********************************************** ///

long ThorLSMCam::FindCameras(long &cameraCount)
{
	cameraCount = 0;

	_numNIDAQ = 0;

	::InitializeCriticalSection(&_remapSection);

	//need to do detailed device search here in the future.
	try
	{
		//XML settings retrieval functions will throw an exception if tags or attributes are missing
		//catch each independetly so that as many tags as possible can be read
		long swapXY[2] = {0};
		_analogFeedbackRatio[0][0] = _analogFeedbackRatio[0][1] = _analogFeedbackRatio[1][0] = _analogFeedbackRatio[1][1] = 1.0;
		if(FALSE == pSetup->GetConfiguration(_field2Theta,_pockelsParkAtMinimum,_galvoParkAtStart,_fieldSizeMin,_fieldSizeMax, _pockelsTurnAroundBlank, _analogChannels[0], _analogChannels[1], _analogFeedbackRatio[0][0], _analogFeedbackRatio[0][1], swapXY[0], _minGalvoFreqHz[0], _analogChannels[2], _analogChannels[3]))
		{
			StringCbPrintfW(_errMsg,_MAX_PATH,L"GetConfiguration from ThorConfocalSettings failed");
			LogMessage(_errMsg,ERROR_EVENT);
		}
		_minGalvoFreqHz[0] = max(1.0, _minGalvoFreqHz[0]);

		//setup lines based on given device id
		_devID = GetDevIDName(_analogChannels[0]);
		_startTriggerLine = "/" + _devID + "/PFI6";
		_frameTriggerLineIn = "/" + _devID + "/PFI4";
		_controllerInternalOutput0 = "/" + _devID + "/Ctr0InternalOutput";
		_controllerInternalOutput1 = "/" + _devID + "/Ctr1InternalOutput";
		_galvoLinesInput = "/" + _devID + "/ai14:15";
		_controllerOutputLine0 = "/" + _devID + "/ctr0";
		_controllerOutputLine1 = "/" + _devID + "/ctr1";
		_controllerOutputLine3 = "/" + _devID + "/ctr2";
		_readLine[0] ="/" + _devID + "/ai0";
		_readLine[1] = "/" + _devID + "/ai1";
		_readLine[2] = "/" + _devID + "/ai0:1";
		_clockExportLine = "/" + _devID + "/PFI5";

		switch (swapXY[0])
		{
		case 1: //XY swapped
			_analogXYmode[0][0] = _analogXYmode[0][1] = -1;
			break;
		case 2:	//XY not swapped but center X at exit
			_analogXYmode[0][0] = 0;
			_analogXYmode[0][1] = 1;
			break;
		case 3: //XY swapped but center X at exit
			_analogXYmode[0][0] = 0;
			_analogXYmode[0][1] = -1;
			break;
		case 0: //XY not swapped
		default:
			_analogXYmode[0][0] = _analogXYmode[0][1] = 1;
			break;
		}

		//find out all NI boards' info
		boardInfoNI.get()->getInstance()->GetAllBoardsInfo();
		BoardInfo* bInfo = boardInfoNI.get()->getInstance()->GetBoardInfo(_devID);
		analogReaderNI.get()->getInstance()->RemoveLine(_galvoLinesInput);
		_numNIDAQ = ((NULL != bInfo) && (TRUE == analogReaderNI.get()->getInstance()->AddLine(_galvoLinesInput))) ? 1 : 0;

	}
	catch (...)
	{
		StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorGGNI FindCameras failed.");
		LogMessage(_errMsg,ERROR_EVENT);
		return FALSE;
	}

	if (_numNIDAQ > 0)
	{
		cameraCount = 1;
	}
	else
	{
		StringCbPrintfW(_errMsg,_MAX_PATH,L"Number of NI cards %d",_numNIDAQ);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	return cameraCount;
}

long ThorLSMCam::SelectCamera(const long camera)
{
	long ret = FALSE;

	if (_numNIDAQ <= 0)
	{
		StringCbPrintfW(_errMsg,_MAX_PATH,L"The hardware has not been located");
		LogMessage(_errMsg,ERROR_EVENT);
		return FALSE;
	}

	AlignDataLoadFile(); ///load the alignment data if exists;

	//maximum number of acquisition channels is one
	_maxChannel = 0xF;

	//Only load the settings from the file one time
	if(FALSE == _fileSettingsLoaded)
	{
		try
		{
			try
			{
				if(FALSE == pSetup->GetIO(_controllerInternalOutput2,_controllerOutputLine2,_pockelsTriggerIn,_pockelsVoltageSlopeThreshold,
					_pockelsLine[0],_pockelsPowerInputLine[0],_pockelsScanVoltageStart[0],_pockelsScanVoltageStop[0],_pockelsMinVoltage[0],_pockelsMaxVoltage[0],
					_pockelsLine[1],_pockelsPowerInputLine[1],_pockelsScanVoltageStart[1],_pockelsScanVoltageStop[1],
					_pockelsLine[2],_pockelsPowerInputLine[2],_pockelsScanVoltageStart[2],_pockelsScanVoltageStop[2],
					_pockelsLine[3],_pockelsPowerInputLine[3],_pockelsScanVoltageStart[3],_pockelsScanVoltageStop[3],_pockelsReferenceLine,
					_pockelsResponseType[0],_pockelsResponseType[1],_pockelsResponseType[2],_pockelsResponseType[3]))				
				{
					StringCbPrintfW(_errMsg,_MAX_PATH,L"GetIO from ThorGGNISettings failed");
					LogMessage(_errMsg,ERROR_EVENT);
				}

				//reset pockels settings:
				for(long i=0; i<MAX_GG_POCKELS_CELL_COUNT; i++)
				{
					_pockelsEnable[i] = FALSE;
				}
				_pockelsSelect = 0;
				if((!_controllerInternalOutput2.empty())&&(!_controllerOutputLine2.empty())&&(!_pockelsTriggerIn.empty()))
				{
					for(long i=0; i<MAX_GG_POCKELS_CELL_COUNT; i++)
					{
						//Dev2 is only capable of 2 analog outputs (GalvoX and GalvoY)
						if(string::npos == _pockelsLine[0].find(_devID))
						{
							_pockelsEnable[i] = (_pockelsLine[i].size() > 0) ? TRUE : FALSE;
							if(_pockelsEnable[i])
								_pockelsSelect++;
						}
					}
				}

				_pockelsReferenceRequirementsMet = FALSE;
				if (_pockelsSelect)
				{
					//Pockelsline[0] and _pockelsReferenceLine have to be on the same Dev card and be on an APFI port
					//to be able to use the reference voltage for pockels 1
					if(string::npos != _pockelsReferenceLine.find(_pockelsLine[0].substr(1,4)) && string::npos != _pockelsReferenceLine.find("APFI"))
					{
						char cardType[256];
						DAQmxGetDevProductType(_pockelsLine[0].substr(1,4).c_str(), cardType, 256);
						std::string temp = cardType;
						if(string::npos != temp.find("6363"))
						{
							_pockelsReferenceRequirementsMet = TRUE;
						}
					}
				}
				//find out all NI boards' info
				_boardInfoNI.get()->getInstance()->GetAllBoardsInfo();
			}
			catch(...)
			{
			}

			try
			{
				long vertScanDir = 0;
				if(FALSE == pSetup->GetCalibration(_fieldSizeCalibration,vertScanDir,_fineOffset[0],_fineOffset[1],_fineFieldSizeScaleX,_fineFieldSizeScaleY,_oneXFieldSize,_maxGalvoOpticalAngle,_minSignalInputVoltage,_maxSignalInputVoltage,_galvoRetraceTime,_pockelsPhaseDelayUS))
				{
					//if fieldSizeCalibration not exists, set its GetParamInfo Available
					_fieldSizeCalibrationAvailable = FALSE;
					StringCbPrintfW(_errMsg,_MAX_PATH,L"GetCalibration from ThorGGNISettings failed. FieldSizeCalibration not available.");
					LogMessage(_errMsg,ERROR_EVENT);
				}
				else
				{
					_fieldSizeCalibrationAvailable = TRUE;
					_verticalScanDirection = (0 == vertScanDir) ? 1.0 : -1.0;		
				}
			}
			catch(...)
			{
			}
			if (GALVO_MIN_RETRACE_TIME > _galvoRetraceTime) _galvoRetraceTime = GALVO_MIN_RETRACE_TIME;
			try
			{
				if(FALSE == pSetup->GetPolarity(_channelPolarity[0],_channelPolarity[1],_channelPolarity[2],_channelPolarity[3]))
				{
					StringCbPrintfW(_errMsg,_MAX_PATH,L"GetPolarity from ThorGGNISettings failed");
					LogMessage(_errMsg,ERROR_EVENT);
				}
				if(FALSE == pSetup->GetWaveform(_pockelDigOut,_waveformCompleteOut, _bleachCycleOut, _bleachIterationOut, _bleachPatternOut, _bleachPatternCompleteOut, _bleachActiveOut, _bleachEpochOut, _bleachCycleInverse))
				{
					StringCbPrintfW(_errMsg,_MAX_PATH,L"GetBleach from ThorGGNISettings failed");
					LogMessage(_errMsg,ERROR_EVENT);
				}
				if(FALSE == pSetup->GetDMA(_dMABufferCount, _activeLoadCount, _imageActiveLoadMS, _imageActiveLoadCount))
				{
					StringCbPrintfW(_errMsg,_MAX_PATH,L"GetDMA from ThorGGNISettings failed");
					LogMessage(_errMsg,ERROR_EVENT);
				}
				_dMABufferCount = (MAX_DMABUFNUM < _dMABufferCount) ? MAX_DMABUFNUM : _dMABufferCount;
				_activeLoadCount = max(1, _activeLoadCount);				
				_imageActiveLoadMS = max(1, _imageActiveLoadMS);
				_imageActiveLoadCount = max(1, _imageActiveLoadCount);
				if(FALSE == pSetup->GetTrigger(_triggerWaitTimeout, _frameBufferReadyOutput,_captureActiveOutput,_captureActiveOutputInvert,_bleachShutterLine,_bleachShutterIdle[0],_bleachShutterIdle[1]))
				{
					StringCbPrintfW(_errMsg,_MAX_PATH,L"GetTrigger from ThorGGNISettings failed");
					LogMessage(_errMsg,ERROR_EVENT);
				}
			}
			catch(...)
			{
			}
		}
		catch(...)
		{
		}
		_fileSettingsLoaded = TRUE;
	}

	//close the shutter when we connect to the camera
	ThorVCMDigitalShutterPosition(CLOSE_SHUTTER);

	//SetCaptureActive level
	SetCaptureActiveOutput(_captureActiveOutputInvert);

	//park the Galvos with default:
	MoveGalvoToParkPosition(1);

	ret = TRUE;

	return ret;

}

long ThorLSMCam::TeardownCamera()
{
	try
	{
		auto_ptr<ThorGGNIXML> pSetup(new ThorGGNIXML());

		long flipVertDir = (_verticalScanDirection > 0) ? 0 : 1;

		double fsCal=0;
		long vertScanDir = 0;		
		long oneXFS = 0;
		double maxOptAngle;
		double minSigIn;
		double maxSigIn;
		double fineOffset[2] = {0.0,0.0};
		double fineFieldSizeScaleX = 0;
		double fineFieldSizeScaleY = 0;

		//Get the values from the file since the fieldSize Calibration and oneXFieldSize may have been saved independently
		if(FALSE == pSetup->GetCalibration(fsCal,vertScanDir,fineOffset[0],fineOffset[1],fineFieldSizeScaleX,fineFieldSizeScaleY,oneXFS,maxOptAngle,minSigIn,maxSigIn,_galvoRetraceTime,_pockelsPhaseDelayUS))
		{
			StringCbPrintfW(_errMsg,_MAX_PATH,L"GetCalibration from ThorGGNISettings failed");
			LogMessage(_errMsg,ERROR_EVENT);
		}

		if(FALSE == pSetup->SetCalibration(fsCal,flipVertDir,_fineOffset[0],_fineOffset[1],_fineFieldSizeScaleX,_fineFieldSizeScaleY,oneXFS,maxOptAngle,minSigIn,maxSigIn,_galvoRetraceTime,_pockelsPhaseDelayUS))
		{
			StringCbPrintfW(_errMsg,_MAX_PATH,L"SetCalibration from ThorGGNISettings failed");
			LogMessage(_errMsg,ERROR_EVENT);
		}

		if(FALSE == pSetup->SetIO(_pockelsMinVoltage[0],_pockelsMaxVoltage[0]))
		{
			StringCbPrintfW(_errMsg,_MAX_PATH,L"SetIO from ThorGGNISettings failed");
			LogMessage(_errMsg,ERROR_EVENT);
		}

		//disconnect terminals:
		DAQmxDisconnectTerms(_controllerInternalOutput0.c_str(), _startTriggerLine.c_str()); 

		////Don't Reset DAQ since it will trip & damage the galvo mirrors
		//int ret = DAQmxResetDevice(_devID.c_str());
		CloseThread();

		//park the pockels cell
		MovePockelsToParkPosition();

		//park the galvo when exiting
		MoveGalvoToParkPosition(2);

		//release resources
		_rawNIBufferSize = 0;
		SAFE_DELETE_MEMORY (_pGalvoWaveformXYP);
		SAFE_DELETE_MEMORY (_pPockelsWaveform);
		SAFE_DELETE_MEMORY (_pRawNIBuffer);
		SAFE_DELETE_MEMORY (_pRawNILineBuffer);

		//reset flag after files saved
		_fileSettingsLoaded = FALSE;
	}
	catch(...)
	{
	}


	//close the shutter when we disconnect the camera
	ThorVCMDigitalShutterPosition(CLOSE_SHUTTER);

	CloseLSMCam();

	return TRUE;
}

long ThorLSMCam::PreflightAcquisition(char * pDataBuffer)
{
	if(_behaviorPtr)
	{
		return _behaviorPtr->PreflightAcquisition(pDataBuffer);
	}
	return FALSE;
}

long ThorLSMCam::SetupAcquisition(char * pDataBuffer)
{
	if(_behaviorPtr)
	{
		return	_behaviorPtr->SetupAcquisition(pDataBuffer);
	}
	return FALSE;
}

long ThorLSMCam::StartAcquisition(char *pDataBuffer)
{
	if(_behaviorPtr)
	{
		return _behaviorPtr->StartAcquisition(pDataBuffer);
	}
	return FALSE;
}

long ThorLSMCam::StatusAcquisition(long &status)
{
	if(_behaviorPtr)
	{
		return _behaviorPtr->StatusAcquisition(status);
	}
	return FALSE;
}

long ThorLSMCam::StatusAcquisitionEx(long &status,long &indexOfLastCompletedFrame)
{
	long ret = TRUE;

	//do not capture data if you are in the centering scan mode
	if(_scanMode == ScanMode::CENTER)
	{
		status = ICamera::STATUS_READY;
		return ret;
	}

	if ((WaitForSingleObject(_hFrmBufReady, 0) == WAIT_OBJECT_0)||
		WaitForSingleObject(_hStopAcquisition, 0) == WAIT_OBJECT_0)
	{
		status = ICamera::STATUS_READY;
	}
	else if (WaitForSingleObject(_hStatusError, 0) == WAIT_OBJECT_0)
	{
		StringCbPrintfW(_errMsg,_MAX_PATH,L"StatusAcquisition ERROR");
		LogMessage(_errMsg,ERROR_EVENT);

		status = ICamera::STATUS_ERROR;
	}
	else
	{
		status = ICamera::STATUS_BUSY;
	}

	indexOfLastCompletedFrame = _indexOfLastCompletedFrame;
	return ret;
}

long ThorLSMCam::CopyAcquisition(char *pDataBuffer, void* frameInfo)
{
	if(_behaviorPtr)
	{
		return _behaviorPtr->CopyAcquisition(pDataBuffer, frameInfo);
	}
	return FALSE;
}

long ThorLSMCam::PostflightAcquisition(char * pDataBuffer)
{
	if(_behaviorPtr)
	{
		return _behaviorPtr->PostflightAcquisition(pDataBuffer);
	}
	return FALSE;
}
