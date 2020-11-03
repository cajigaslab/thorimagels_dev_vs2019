#include "stdafx.h"
#include "ThorGGNI.h"
#include "Strsafe.h"

//***	Buffer static members:	***//
CRITICAL_SECTION ThorLSMCam::_remapSection;
long* ThorLSMCam::_remap_index = NULL;
long ThorLSMCam::_remap_size = 0;
long ThorLSMCam::_loadedShiftPlusAlignment = 0;
long ThorLSMCam::_rawNIBufferSize = 0;
float64 *ThorLSMCam::_pRawNIBuffer = NULL;
float64 *ThorLSMCam::_pRawNILineBuffer = NULL;
float64 *ThorLSMCam::_pRawNIPartialBuffer = NULL;
long ThorLSMCam::_1stSet_CMA_Frame = 1;
unsigned long ThorLSMCam::_dataPerLine = 1;
unsigned long ThorLSMCam::_dataPerLineToRead = 1;
unsigned long ThorLSMCam::_lineSampleLength = 0;
unsigned long ThorLSMCam::_linesRead = 0;
unsigned long ThorLSMCam::_numCallbacks = 0;
long ThorLSMCam::_bufCompleteID = 0;
map<long, long> ThorLSMCam::_bufferOrder;
long ThorLSMCam::_index = 0;
long ThorLSMCam::_line_start_indexF[MAX_BOARD_NUM] = {0};
long ThorLSMCam::_line_start_indexB[MAX_BOARD_NUM] = {0};
long ThorLSMCam::_offset = 0;
long ThorLSMCam::_indexLineFactor = 1;
long ThorLSMCam::_partialFrameReady = FALSE;
long ThorLSMCam::_readEntireFrame = FALSE;

long (*Func1)(long &, long *);
long (*Func2)(long &, long *);
long (*Func3)(long &, long );

///	***************************************** <summary> Setup NI board </summary>	********************************************** ///

long ThorLSMCam::SetBdDMA(void)
{
	int32 error = ApiSuccess;
	if(!ActiveBehavior())
		return ApiFailed;

	//no more copy allowed after preflight by reset of indexes
	_indexOfLastCompletedFrame = _indexOfLastCopiedFrame = -1;
	_bufCompleteID = 0;

	long factor = 1;
	_behaviorPtr->GetParam(BehaviorProp::LINE_FACTOR, factor);

	_recsPerBuffer = ImageWaveformBuilder->GetForwardLines() * factor;

	switch (_imgPtyDll.triggerMode)
	{
	case ICamera::SW_FREE_RUN_MODE:
	case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
		_recordCount = INFINITE_COUNT;
		break;
	case ICamera::SW_SINGLE_FRAME:
	case ICamera::HW_SINGLE_FRAME:
		_recordCount = _recsPerBuffer;
		break;
	case ICamera::SW_MULTI_FRAME:
	case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
		uInt64 recordCount = static_cast<uInt64>(_imgPtyDll.numFrame) * static_cast<uInt64>(_recsPerBuffer);
		if (MAXUINT32 < recordCount)
		{
			MessageBox(NULL,L"The number of frames is higher than the maximum possible for this pixel density and scan mode. The experiment will not start.",L"Error", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
			_recordCount = 0;
			return ApiFailed;
		}
		else
		{
			_recordCount = _imgPtyDll.numFrame * _recsPerBuffer;
		}
		break;
	}

	_1stSet_CMA_Frame = 1;  //the first frame is display without down scaling by average number for cumulative average

	::EnterCriticalSection(&_remapSection);

	try
	{
		//set data remaping
		_totalChannelsForAllBoards = 0;

		//use line post mode when frame rate is lower than 1 fps instead of original memory limit:
		//_bufferLarge = (16777216 < _vCMBoardDefinition.RecLength*_recsPerBuffer* _numChannel[0]*sizeof(U16));  //16MB
		_bufferLarge = (1.0 <= ImageWaveformBuilder->GetFrameTime()) ? TRUE : FALSE;
		_imgPtyDll.dmaBufferCount = (TRUE == _bufferLarge) ? 4 : _dMABufferCount;	//limit dma buffer count to 4 if buffer large

		for (long i = 0; i < _numNIDAQ; i++)
		{
			_channelMode[i] = 1;
			_numChannel[i] = 4;

			_totalChannelsForAllBoards += _numChannel[i];

			_sizePerBuffer[i] = _recsPerBuffer * _imgPtyDll.pixelX * sizeof(U16);
			if ((_lastSizePerBuffer[i] != _sizePerBuffer[i]) || (_lastDMABufferCount != _imgPtyDll.dmaBufferCount))
			{
				for (long j = 0; j < _imgPtyDll.dmaBufferCount; j++)
				{
					//assign DMA buffer for each board
					_pData[i][j] = (U16*) realloc(_pData[i][j], _imgPtyDll.pixelY * _imgPtyDll.pixelX * sizeof(U16) * _numChannel[i]);

					if(_pData[i][j] == NULL)
					{
						StringCbPrintfW(_errMsg,_MAX_PATH,L"SetBdDMA unable to allocate pData buffer bd(%d) index(%d) size(%d)",i,j,_sizePerBuffer[i]);
						LogMessage(_errMsg,ERROR_EVENT);
						::LeaveCriticalSection(&_remapSection);
						return ApiFailed;
					}
				}
				_lastSizePerBuffer[i] = _sizePerBuffer[i];
			}
		}

		//<key, value>: <dma buffer index, completed frame index>
		_bufferOrder.clear();
		for (long i = 0; i < _imgPtyDll.dmaBufferCount; i++)
		{
			_bufferOrder.insert(pair<long, long>(i, _indexOfLastCompletedFrame));
		}

		//assign Memory buffer for ordered and mapped 16 bits frame data
		size_t bufferSize = (_totalChannelsForAllBoards * _imgPtyDll.pixelX * _imgPtyDll.pixelY * sizeof(unsigned short)) + sizeof(long);
		if ((_lastBufferSize != bufferSize) || (_lastDMABufferCount != _imgPtyDll.dmaBufferCount))
		{
			for(long k=0; k<_imgPtyDll.dmaBufferCount; k++)
			{
				//assign Memory buffer for ordered and mapped 16 bits frame data
				_pFrmDllBuffer[k] = (unsigned short*) realloc(_pFrmDllBuffer[k], bufferSize);

				if(_pFrmDllBuffer[k] == NULL)
				{
					StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorVCMSetBdDMA unable to allocate pFrmDllBuffer size(%d)", bufferSize);
					LogMessage(_errMsg,ERROR_EVENT);
					::LeaveCriticalSection(&_remapSection);
					return ApiFailed;
				}

				memset(_pFrmDllBuffer[k], 0x0, bufferSize);
			}
			_lastBufferSize = bufferSize;
			_lastDMABufferCount = _imgPtyDll.dmaBufferCount;
		}
		SetupDataMap();

		ChFlag = min(2, (_imgPtyDll.channel - 1)); // for _channel greater than 2 are treated as multiple channels selected
		_lineSampleLength = static_cast<ULONG>(2*_imgPtyDll.dwellTime / max(1, ChFlag) * (ImageWaveformBuilder->GetLineDataLength()) + 0.5);	

		//prepare buffers for data read task
		if (ImageWaveformBuilder->GetFrameDataLength() * 2 != _rawNIBufferSize)
		{
			//To avoid overstepping the buffer when quickly switching scanModes always set the size of the buffer to be 2 times the galvo data length
			_rawNIBufferSize = ImageWaveformBuilder->GetFrameDataLength() * 2;
			_pRawNIBuffer = (float64*)realloc(_pRawNIBuffer,sizeof(float64) * _rawNIBufferSize * READ_CHANNEL_COUNT);		
			_pRawNIPartialBuffer = (float64*)realloc(_pRawNIPartialBuffer,sizeof(float64) * _rawNIBufferSize * READ_CHANNEL_COUNT);
		}
		_pRawNILineBuffer = (float64*)realloc(_pRawNILineBuffer, sizeof(float64)*_lineSampleLength * READ_CHANNEL_COUNT);

		//ready to return
		_behaviorPtr->SetParam(BehaviorProp::SWITCH_BEHAVIOR, 0);
		ResetEvent(_hStopAcquisition); ///to de-signal the Stop Acquisition Event

		error = ApiSuccess;
	}
	catch (...)
	{
		DAQmxFailed(error);
		StringCbPrintfW(_errMsg,_MAX_PATH,L"SetBdDMA failed");
		LogMessage(_errMsg,ERROR_EVENT);
	}
	::LeaveCriticalSection(&_remapSection);
	return error;
}

long ThorLSMCam::SetupReadTask()
{
	int32 retVal = 0, error = 0;
	if(_taskHandleAI0)
	{
		DAQmxStopTask(_taskHandleAI0);
		DAQmxClearTask(_taskHandleAI0);
		_taskHandleAI0 = NULL;
	}

	DAQmxErrChk(L"DAQmxCreateTask", retVal = DAQmxCreateTask("",&_taskHandleAI0));
	DAQmxErrChk (L"DAQmxCreateAIVoltageChan", retVal = DAQmxCreateAIVoltageChan(_taskHandleAI0,_readLine[ChFlag].c_str(),"",DAQmx_Val_Diff,MIN_AO_VOLTAGE,MAX_AO_VOLTAGE,DAQmx_Val_Volts,NULL));

	//Because the start is synchronized, we don't need retrigger the sampling. Only trigger the first time to make sure it is all in sync.
	//Use DAQmx_Val_ContSamps to allow the analog input to acquire _lineSampleLength continuously.
	//Do NOT use DAQmx_Val_FiniteSamps with retrigger because this competes with reading the data out of the card and throws an error.
	DAQmxErrChk(L"DAQmxCfgSampClkTiming", retVal = DAQmxCfgSampClkTiming(_taskHandleAI0, "", DATA_SAMPLE_RATE / max(1, ChFlag), DAQmx_Val_Rising, DAQmx_Val_ContSamps,_lineSampleLength));

	DAQmxErrChk(L"DAQmxCfgDigEdgeStartTrig ",retVal = DAQmxCfgDigEdgeStartTrig(_taskHandleAI0, _clockExportLine.c_str(), DAQmx_Val_Rising));

	//start acquisition task
	DAQmxErrChk(L"DAQmxTaskControl",retVal = DAQmxTaskControl(_taskHandleAI0, DAQmx_Val_Task_Reserve));
	DAQmxErrChk(L"DAQmxStartTask", retVal = DAQmxStartTask(_taskHandleAI0));
	return retVal;
}

UINT ThorLSMCam::ReadDataAsync(ThorLSMCam *inst)
{
	int32 error = 0;
	try
	{
		//notice read thread is started
		SetEvent(_hReadThreadStarted);

		if (NULL != ThorLSMCam::_taskHandleAI0)
		{
			while (FALSE == _stopReading)
			{
				int32 readAI = 0;
				if(inst->ChFlag < 2)
				{
					DAQmxErrChk (L"DAQmxReadAnalogF64",DAQmxReadAnalogF64(ThorLSMCam::_taskHandleAI0,_lineSampleLength,0.1,DAQmx_Val_GroupByChannel,_pRawNILineBuffer,_lineSampleLength,&readAI,NULL));

					if(0 == _linesRead)
					{
						//wait for the _hNIRawFrmCopiedHandle event to be set as a signal that it is safe to use _pRawNIBuffer
						WaitForSingleObject(_hNIRawFrmCopiedHandle, Constants::TIMEOUT_MS);
						ResetEvent(_hNIRawFrmCopiedHandle);
					}

					//for ch1 or ch2
					float64 *ptrS = _pRawNILineBuffer + _loadedShiftPlusAlignment;
					float64 *ptrD = _pRawNIBuffer  + (inst->ChFlag) * _rawNIBufferSize  + _linesRead * _dataPerLine;

					long limit = min((long)(_avgCount), (long)(_avgCount + 0.5));
					long splitFlg = ((long)(_avgCount) + (long)(_avgCount + 0.5))%2;
					for(unsigned long x = 0; x < _dataPerLineToRead; x++)
					{
						float64 sum = 0;
						if((splitFlg == 1) && (x%2 == 1))
						{
							sum += (*ptrS)/2;
							ptrS++;
						}

						for(long avg = 0; avg < limit; avg++)
						{
							sum +=  *ptrS;
							ptrS++;
						}

						if((splitFlg == 1) && (x%2 == 0))
						{
							sum += (*ptrS)/2;
						}

						*ptrD = sum;
						ptrD++;
					}					

					_linesRead++;				
					if(TRUE == _bufferLarge && TRUE == _isLiveScan )
					{
						// Copy to partial buffer every 16 lines
						if(0 == (_linesRead + 1) % LINE_POST_COUNT)
						{
							long lineByteStartPosition = (_linesRead + 1 == LINE_POST_COUNT) ? 0 : (_linesRead - LINE_POST_COUNT) * _dataPerLine;
							memcpy_s( lineByteStartPosition + _pRawNIPartialBuffer + (inst->ChFlag) * _rawNIBufferSize , sizeof(float64) * _dataPerLine * LINE_POST_COUNT, lineByteStartPosition + _pRawNIBuffer + (inst->ChFlag) * _rawNIBufferSize , sizeof(float64) * _dataPerLine * LINE_POST_COUNT);

						}
						// Send StartFrmAsync a notice if the partial frame is ready or the full frame
						if((_linesRead + 1) >= ImageWaveformBuilder->GetOverallLines() - (LINE_POST_COUNT * 2))
						{
							_readEntireFrame = TRUE;
						}
						else
						{
							_partialFrameReady = TRUE;
						}

						if((_linesRead) >= ImageWaveformBuilder->GetOverallLines())
						{
							//Copy the last line
							memcpy_s((_linesRead - 2) * _dataPerLine + _pRawNIPartialBuffer + (inst->ChFlag) * _rawNIBufferSize , sizeof(float64) * _dataPerLine, (_linesRead - 2) * _dataPerLine  + _pRawNIBuffer + (inst->ChFlag) * _rawNIBufferSize , sizeof(float64) * _dataPerLine);
							
							_linesRead = 0;

							//let other threads know the _pRawNIBuffer buffer can be accessed
							SetEvent(_hNIRawFrmReadyHandle);
						}
					}
					else
					{
						if((_linesRead) >= ImageWaveformBuilder->GetOverallLines())
						{
							_linesRead = 0;
							
							//let other threads know the _pRawNIBuffer buffer can be accessed
							SetEvent(_hNIRawFrmReadyHandle);
						}
					}
				}
				else
				{
					DAQmxErrChk (L"DAQmxReadAnalogF64",DAQmxReadAnalogF64(ThorLSMCam::_taskHandleAI0,_lineSampleLength,0.1,DAQmx_Val_GroupByChannel,_pRawNILineBuffer,_lineSampleLength * 2,&readAI,NULL));

					if(0 == _linesRead)
					{
						//wait for the _hNIRawFrmCopiedHandle event to be set as a signal that it is safe to use _pRawNIBuffer
						WaitForSingleObject(_hNIRawFrmCopiedHandle, Constants::TIMEOUT_MS);
						ResetEvent(_hNIRawFrmCopiedHandle);
					}

					//for ch1 and ch2
					float64 *ptrS = _pRawNILineBuffer + _loadedShiftPlusAlignment;
					float64 *ptrD = _pRawNIBuffer  + _linesRead * _dataPerLine;
					float64 *ptrS_ch2 = _pRawNILineBuffer + _lineSampleLength + _loadedShiftPlusAlignment;
					float64 *ptrD_ch2 = _pRawNIBuffer  + _rawNIBufferSize  + _linesRead * _dataPerLine;
					long limit = min((long)(_avgCount), (long)(_avgCount + 0.5));
					long splitFlg = ((long)(_avgCount) + (long)(_avgCount + 0.5))%2;
					for(unsigned long x = 0; x < _dataPerLineToRead; x++)
					{
						float64 sum = 0;
						float64 sum_ch2 = 0;
						if((splitFlg == 1) && (x%2 == 1))
						{
							sum += (*ptrS)/2;
							ptrS++;

							sum_ch2 += (*ptrS_ch2)/2;
							ptrS_ch2++;
						}

						for(long avg = 0; avg < limit; avg++)
						{
							sum +=  *ptrS;
							ptrS++;

							sum_ch2 +=  *ptrS_ch2;
							ptrS_ch2++;
						}

						if((splitFlg == 1) && (x%2 == 0))
						{
							sum += (*ptrS)/2;
							sum_ch2 += (*ptrS_ch2)/2;
						}

						*ptrD = sum * 2;
						ptrD++;
						*ptrD_ch2 = sum_ch2 * 2;
						ptrD_ch2++;
					}
					
					_linesRead++;			

					if(TRUE == _bufferLarge && TRUE == _isLiveScan )
					{
						// Copy to partial buffer every 16 lines
						if(0 == (_linesRead + 1) % LINE_POST_COUNT)
						{
							long lineByteStartPosition = (_linesRead + 1 == LINE_POST_COUNT) ? 0 : (_linesRead - LINE_POST_COUNT) * _dataPerLine;
							memcpy_s( lineByteStartPosition + _pRawNIPartialBuffer, sizeof(float64) * _dataPerLine * LINE_POST_COUNT, lineByteStartPosition + _pRawNIBuffer, sizeof(float64) * _dataPerLine * LINE_POST_COUNT);
							memcpy_s( _rawNIBufferSize + lineByteStartPosition + _pRawNIPartialBuffer, sizeof(float64) * _dataPerLine * LINE_POST_COUNT, _rawNIBufferSize + lineByteStartPosition + _pRawNIBuffer, sizeof(float64) * _dataPerLine * LINE_POST_COUNT);
						}
						// Send StartFrmAsync a notice if the partial frame is ready or the full frame
						if((_linesRead + 1) >= ImageWaveformBuilder->GetOverallLines() - (LINE_POST_COUNT * 2))
						{
							_readEntireFrame = TRUE;
						}
						else
						{
							_partialFrameReady = TRUE;
						}

						if((_linesRead) >= ImageWaveformBuilder->GetOverallLines())
						{
							//Copy the last line
							memcpy_s((_linesRead - 2) * _dataPerLine + _pRawNIPartialBuffer, sizeof(float64) * _dataPerLine, (_linesRead - 2) * _dataPerLine  + _pRawNIBuffer, sizeof(float64) * _dataPerLine);
							memcpy_s( _rawNIBufferSize + (_linesRead - 2) * _dataPerLine + _pRawNIPartialBuffer, sizeof(float64) * _dataPerLine,  _rawNIBufferSize + (_linesRead - 2) * _dataPerLine  + _pRawNIBuffer, sizeof(float64) * _dataPerLine);
							
							_linesRead = 0;
							
							//let other threads know the _pRawNIBuffer buffer can be accessed
							SetEvent(_hNIRawFrmReadyHandle);
						}
					}
					else
					{
						if((_linesRead) >= ImageWaveformBuilder->GetOverallLines())
						{
							_linesRead = 0;
							
							//let other threads know the _pRawNIBuffer buffer can be accessed
							SetEvent(_hNIRawFrmReadyHandle);
						}
					}
				}
			}		
		}
	}
	catch (...)
	{
		if (DAQmxFailed(error))
		{
			StringCbPrintfW(message,_MAX_PATH,L"ThorGGNI exception in ReadDataAsync error: %d",error);
			LogMessage(message,VERBOSE_EVENT);
		}
	}

	//clear task before return, to avoid potential 
	//sample clock setup error at later restart
	if(_taskHandleAI0)
	{
		DAQmxStopTask(_taskHandleAI0);
		DAQmxClearTask(_taskHandleAI0);
		_taskHandleAI0 = NULL;
	}

	//let other threads know the _pRawNIBuffer buffer can be accessed
	SetEvent(_hNIRawFrmReadyHandle);
	return TRUE;
}

///ThorLSMCam::StartFrmAsyncGalvo
///The function is called for the data acqusition of galvo-galvo control
///Function is called in a seperate thread
///There might be some latency in starting this function because there
///is no hardware trigger associated with it, instead it is being initialized
///in the software
UINT ThorLSMCam::StartFrmAsync(void *pData)
{
	ResetEvent(_hNIRawFrmCopiedHandle); //allow restart in freerun if error
	ResetEvent(_hNIRawFrmReadyHandle); //allow restart in freerun if error
	unsigned short** pFrmData = (unsigned short**) pData;
	int32 retVal = 0, error = 0;

	long bdID;
	U32 frmLeft;
	DWORD startTime;
	HANDLE handleDataRead = NULL;
	U32 transferLength = _imgPtyDll.pixelX;
	U32 recsPerBuffer = _recsPerBuffer;
	long pixelX = _imgPtyDll.pixelX;
	long pixelY = _imgPtyDll.pixelY;
	long scanMode = _imgPtyDll.scanMode;
	long twoWayAlignment = _imgPtyDll.alignmentForField;
	long horizontalFlip = _imgPtyDll.horizontalFlip;
	long loadedShift = _shiftArray[static_cast<long>((_imgPtyDll.dwellTime - 1.0)/DWELL_TIME_STEP)];

	double waitTimeRatio = (0.015 > ImageWaveformBuilder->GetFrameTime()) ? 8.0 : 4.0;		//determined by test, 4.0 will fail at 0.4 us, 32 x 32 pixels interleave scan
	DWORD waitTime = static_cast<DWORD>(waitTimeRatio * 1000 * ImageWaveformBuilder->GetFrameTime());
	if(!ActiveBehavior())
		return ApiFailed;

	try
	{
		float64* pRaw = NULL;
		float64* pRaw_ch2 = NULL;

		long factor = 1;
		_behaviorPtr->GetParam(BehaviorProp::LINE_FACTOR, factor);
		frmLeft = _recordCount / ImageWaveformBuilder->GetForwardLines() / factor;

		_bufCompleteID = 0;

		size_t frameBufferSize = _numNIDAQ * READ_CHANNEL_COUNT * pixelX * pixelY * sizeof(unsigned short);

		_numCallbacks = 0;
		_linesRead = 0;
		_avgCount = 2*_imgPtyDll.dwellTime/max(1, getInstance()->ChFlag);//DATA_SAMPLE_RATE / max(1, getInstance()->ChFlag) /_clockRateNI;
		_dataPerLine = 2*pixelX + 4*ImageWaveformBuilder->GetSamplesPadding();
		_loadedShiftPlusAlignment = (long)(1.0 * (twoWayAlignment + loadedShift) / max(1, getInstance()->ChFlag) + 0.5);
		_dataPerLineToRead = _dataPerLine - static_cast<unsigned long>(floor((twoWayAlignment + loadedShift) / 2.0 / _imgPtyDll.dwellTime));

		StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI StartFrmAsync starting acquisition loop", 1);
		LogMessage(message,VERBOSE_EVENT);

		//Start the active output
		SetCaptureActiveOutput(!_captureActiveOutputInvert);	

		//start the counter clock which will kick off all of the tasks
		DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleCO0));		
		_stopReading = false;
		startTime = GetTickCount();

		//Put the reading from the device in a separate thread to allow the image buffer to be processed
		//while a new buffer is being acquired
		DWORD threadID;
		ResetEvent(_hReadThreadStarted);
		handleDataRead = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &(ThorLSMCam::ReadDataAsync), getInstance(), HIGH_PRIORITY_CLASS, &threadID);
		SetThreadPriority(handleDataRead,THREAD_PRIORITY_TIME_CRITICAL);

		//no need to wait to process the first line in the read thread
		SetEvent(_hNIRawFrmCopiedHandle);
		//wait for the thread to start
		if(WAIT_OBJECT_0 != WaitForSingleObject(_hReadThreadStarted, Constants::TIMEOUT_MS))
			goto THREAD_ERROR;

		while ((WaitForSingleObject(_hStopAcquisition, 0) != WAIT_OBJECT_0) && (frmLeft > 0))
		{
			//Wait for buffer ready
			_bufCompleteID %= _imgPtyDll.dmaBufferCount; 
			bdID = 0;

			if(TRUE == _bufferLarge && TRUE == _isLiveScan)
			{
				if(TRUE == _readEntireFrame)
				{
					//wait for the _hNIRawFrmReadyHandle event to be set as a signal that it is safe to use _pRawNIBuffer
					WaitForSingleObject(_hNIRawFrmReadyHandle, waitTime);
					ResetEvent(_hNIRawFrmReadyHandle);
					_acquireStatus = (int)ICamera::STATUS_READY;

					pRaw = _pRawNIPartialBuffer;
					pRaw_ch2 = _pRawNIPartialBuffer + _rawNIBufferSize;
					_readEntireFrame = FALSE;
				}
				else if(TRUE == _partialFrameReady)
				{
					pRaw = _pRawNIPartialBuffer;
					pRaw_ch2 = _pRawNIPartialBuffer + _rawNIBufferSize;
					_acquireStatus = (int)ICamera::STATUS_PARTIAL;
					_partialFrameReady = FALSE;
				}
				else
				{
					_acquireStatus = (int)ICamera::STATUS_BUSY;
					continue;
				}
			}
			else
			{
				_acquireStatus = (int)ICamera::STATUS_BUSY;

				//wait for the _hNIRawFrmReadyHandle event to be set as a signal that it is safe to use _pRawNIBuffer
				WaitForSingleObject(_hNIRawFrmReadyHandle, waitTime);
				ResetEvent(_hNIRawFrmReadyHandle);

				 pRaw = _pRawNIBuffer;
				 pRaw_ch2 = _pRawNIBuffer + _rawNIBufferSize;
			}

			//process intermediate buffer into _pFrmDllBuffer which will be used later in ScanModeClass,
			//leave out turn around points, reorder and map data buffer
			const long MAX_INTENSITY = 0xFFFF;
			unsigned short * pBuffer = _pData[bdID][_bufCompleteID];
			unsigned short * pBuffer_ch2 = _pData[bdID][_bufCompleteID] + _imgPtyDll.pixelY * _imgPtyDll.pixelX ;

			double minV = _minSignalInputVoltage;
			double rangeV = abs(_maxSignalInputVoltage - _minSignalInputVoltage);
			if (0 == rangeV)
			{
				rangeV = 1;
			}

			switch (scanMode)
			{
			case TWO_WAY_SCAN:
				{
					for (unsigned long lineID=0; lineID < recsPerBuffer; lineID++) 
					{
						if (lineID ==1 && recsPerBuffer > 1)
						{
							//let the read thread know that it can use the _pRawNIBuffer buffer again
							SetEvent(_hNIRawFrmCopiedHandle);
						}
						if((lineID % 2 == 0 && FALSE == horizontalFlip) || (lineID % 2 != 0 && TRUE == horizontalFlip))
						{
							for (long x=0; x < pixelX; x++)  
							{
								unsigned short val = (unsigned short)((min(rangeV, max((*pRaw),minV) - minV)/rangeV) * MAX_INTENSITY);
								*pBuffer = val;
								pBuffer++;
								pRaw++;

								val = (unsigned short)((min(rangeV, max((*pRaw_ch2),minV) - minV)/rangeV) * MAX_INTENSITY);
								*pBuffer_ch2 = val;
								pBuffer_ch2++;
								pRaw_ch2++;

							}
							pRaw += 2*(ImageWaveformBuilder->GetSamplesPadding());
							pRaw_ch2 += 2*(ImageWaveformBuilder->GetSamplesPadding());
						}
						else
						{
							for (long x=(pixelX-1); x >= 0; x--)  
							{
								unsigned short val = (unsigned short)((min(rangeV, max(*(pRaw+x),minV) - minV)/rangeV) * MAX_INTENSITY);
								*pBuffer = val;
								pBuffer++;

								val = (unsigned short)((min(rangeV, max(*(pRaw_ch2+x),minV) - minV)/rangeV) * MAX_INTENSITY);
								*pBuffer_ch2 = val;
								pBuffer_ch2++;
							}
							pRaw += pixelX + 2*(ImageWaveformBuilder->GetSamplesPadding());
							pRaw_ch2 += pixelX + 2*(ImageWaveformBuilder->GetSamplesPadding());
						}
					}
				}
				break;
			case FORWARD_SCAN:
				{	
					if (FALSE == horizontalFlip)
					{
						for (unsigned long lineID=0; lineID < recsPerBuffer; lineID++) 
						{
							if (lineID ==1 && recsPerBuffer > 1)
							{
								//let the read thread know that it can use the _pRawNIBuffer buffer again
								SetEvent(_hNIRawFrmCopiedHandle);
							}
							for (long x=0; x < pixelX; x++)  
							{								
								unsigned short val = (unsigned short)((min(rangeV, max(*pRaw,minV) - minV)/rangeV) * MAX_INTENSITY);
								*pBuffer = val;
								pBuffer++;
								pRaw++;
								val = (unsigned short)((min(rangeV, max(*pRaw_ch2,minV) - minV)/rangeV) * MAX_INTENSITY);
								*pBuffer_ch2 = val;
								pBuffer_ch2++;
								pRaw_ch2++;

							}
							pRaw += pixelX + 4*(ImageWaveformBuilder->GetSamplesPadding());
							pRaw_ch2 += pixelX + 4*(ImageWaveformBuilder->GetSamplesPadding());
						}
					}
					else
					{
						for (unsigned long lineID=0; lineID < recsPerBuffer; lineID++) 
						{
							if (lineID ==1 && recsPerBuffer > 1)
							{
								//let the read thread know that it can use the _pRawNIBuffer buffer again
								SetEvent(_hNIRawFrmCopiedHandle);
							}
							for (long x=(pixelX-1); x >= 0; x--)
							{
								unsigned short val = (unsigned short)((min(rangeV, max(*(pRaw+x),minV) - minV)/rangeV) * MAX_INTENSITY);
								*pBuffer = val;
								pBuffer++;

								val = (unsigned short)((min(rangeV, max(*(pRaw_ch2+x),minV) - minV)/rangeV) * MAX_INTENSITY);
								*pBuffer_ch2 = val;
								pBuffer_ch2++;
							}
							pRaw += 2 * pixelX + 4*(ImageWaveformBuilder->GetSamplesPadding());
							pRaw_ch2 += 2 * pixelX + 4*(ImageWaveformBuilder->GetSamplesPadding());
						}
					}
				}
				break;
			}
			if (recsPerBuffer == 1)
			{
				//let the read thread know that it can use the _pRawNIBuffer buffer again
				SetEvent(_hNIRawFrmCopiedHandle);
			}
			double successFrameTime = (GetTickCount() - startTime)/ Constants::MS_TO_SEC;

			StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI buffer capture complete. Frame time %d.%d", static_cast<long>(successFrameTime), static_cast<long>(1000*(successFrameTime - static_cast<long>(successFrameTime))));
			LogMessage(message,VERBOSE_EVENT);								

			startTime = GetTickCount();

			//Process buffer
			if (WaitForSingleObject(_hStopAcquisition, 0) != WAIT_OBJECT_0)
			{
				WaitForSingleObject(_hFrmBufHandle, Constants::TIMEOUT_MS);

				if((FRM_CUMULATIVE_MOVING == _imgPtyDll.averageMode) && (0 <= _indexOfLastCompletedFrame))
				{
					//When in averageMode == FRM_CUMULATIVE_MOVING the previous frame is used. 
					//To accomplish this we always copy the previous frame to pFrameData[_bufCompleteID]
					//except when in the first frame
					memcpy_s(pFrmData[_bufCompleteID], frameBufferSize, pFrmData[(_bufCompleteID-1+_imgPtyDll.dmaBufferCount)%_imgPtyDll.dmaBufferCount], frameBufferSize);
				}
				_behaviorPtr->ProcessBuffer(_bufCompleteID);

				//send the index to appended long integer on the buffer
				unsigned char *pIndex = (unsigned char*)pFrmData[_bufCompleteID];
				pIndex += frameBufferSize;
				long *pIndexValue = (long*)pIndex;
				long indexVal = _indexOfLastCompletedFrame+1;
				memcpy_s(pIndexValue,sizeof(long),&indexVal,sizeof(long));

				//update completed frame index
				_bufferOrder.at(_bufCompleteID) = indexVal;

				_acquireStatus = (int)ICamera::STATUS_READY;
				ReleaseMutex(_hFrmBufHandle);
				SetEvent(_hFrmBufReady);

				if(_hStatusHandle != NULL)
					SetEvent(_hStatusHandle);

				_bufCompleteID++;
				frmLeft -= (_recordCount < INFINITE_COUNT);
				_indexOfLastCompletedFrame++;
			}
			//break if read thread is done
			if (NULL == _taskHandleAI0)
				break;
		}

		//Start the active output
		SetCaptureActiveOutput(_captureActiveOutputInvert);

		if(frmLeft > 0 )
		{
			if(WaitForSingleObject(_hStopAcquisition, 0) == WAIT_OBJECT_0)
			{		
				StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI StartFrmAsync stopped due to stop Acquisition event");
				LogMessage(message,VERBOSE_EVENT);
				_acquireStatus = (int)ICamera::STATUS_ERROR;
			}

			StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI StartFrmAsync frames left %d", frmLeft);
			LogMessage(message,VERBOSE_EVENT);
			_acquireStatus = (int)ICamera::STATUS_ERROR;
		}
		else
		{
			_progressCounter++;
		}
			SetEvent(_hNIRawFrmCopiedHandle); //ensure all threads end
			SetEvent(_hNIRawFrmReadyHandle); //ensure all threads end
		//Let the reading thread know that it can stop
		_stopReading = TRUE;
		ThorCloseNITasks();

		SAFE_DELETE_HANDLE (handleDataRead);

		//Signal that the thread has stopped
		SetEvent(_hThreadStopped);
	}
	catch (...)
	{	
		_acquireStatus = (int)ICamera::STATUS_ERROR;
		goto THREAD_ERROR;
	}	
	SAFE_DELETE_HANDLE (_hThread);
	return (error - ApiSuccess); //if success return Zero instead of 512

THREAD_ERROR:
	_stopReading = TRUE;
	ThorCloseNITasks();

	SAFE_DELETE_HANDLE (handleDataRead);
	SetEvent(_hNIRawFrmCopiedHandle); //ensure all threads end
	SetEvent(_hNIRawFrmReadyHandle); //ensure all threads end
	SetEvent(_hThreadStopped);
	SetEvent(_hStopAcquisition); //allow restart in freerun if error
	SetEvent(_hThreadStopped);
	if (DAQmxFailed(error))
	{
		StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorGGNI exception in StartFrmAsync error: %d",error);
		LogMessage(_errMsg,ERROR_EVENT);
	}
	SAFE_DELETE_HANDLE (_hThread);
	return (error - ApiSuccess); //if success return Zero instead of 512
}

///	***************************************** <summary> Buffer process functions </summary>	********************************************** ///

long ThorLSMCam::ProcessBufferFunc1(void)
{
	return Func1(_index, _line_start_indexF);
}

long ThorLSMCam::ProcessBufferFunc2(void)
{
	return Func2(_index, _line_start_indexB);
}

long ThorLSMCam::ProcessBufferFunc3(void)
{
	return Func3(_index, _offset);
}

//Straight acquisition without Average
long ThorLSMCam::ProcessBufferNoAve(void)
{
	long start_index = 0;
	if(!ActiveBehavior())
		return FALSE;

	for (int i = 0; i < MAX_BOARD_NUM; i++)
	{
		_line_start_indexF[i] = start_index;
		_line_start_indexB[i] = start_index;
	}

	_index = 0;
	_offset = _imgPtyDll.pixelX;			//offset one line in interleave scan
	Func1 = &ProcessForwardLine;
	Func2 = &ProcessBackwardLine;
	Func3 = &ProcessLineOffset;

	_behaviorPtr->GetParam(BehaviorProp::LINE_FACTOR, _indexLineFactor);
	return TRUE;
}

long ThorLSMCam::ProcessForwardLine(long &index, long *line_start_indexF)
{
	long j;
	long start, end, bdID;
	long transferLength = _imgPtyDll.pixelX;
	unsigned short* pFrmData = _pFrmDllBuffer[_bufCompleteID];

	::EnterCriticalSection(&_remapSection);

	for (bdID = 0; bdID < _numNIDAQ; bdID++)
	{
		long numDataPerFrPerCh = _imgPtyDll.pixelY * _imgPtyDll.pixelX ;
		start = line_start_indexF[bdID];
		end = start + transferLength;
		switch (_numChannel[bdID])
		{
		case 0:
			break;
		default:
			{		
				for(j=0; j<transferLength; j++)
				{			
					if(numDataPerFrPerCh > index)
					{
						pFrmData[index] = static_cast<unsigned short> (min(abs((int) _datamap[0][_pData[bdID][_bufCompleteID][start + j]]), 0x3FFF));
						pFrmData[numDataPerFrPerCh + index] = static_cast<unsigned short> (min(abs((int) _datamap[1][_pData[bdID][_bufCompleteID][numDataPerFrPerCh + start + j]]), 0x3FFF));
						index++;
					}
				}

				line_start_indexF[bdID] += transferLength * _indexLineFactor;
			}
			break;
		}
	}
	::LeaveCriticalSection(&_remapSection);
	return TRUE;
}

long ThorLSMCam::ProcessBackwardLine(long &index, long *line_start_indexB)
{
	long j;
	long start, end, bdID;
	long transferLength = _imgPtyDll.pixelX;
	unsigned short* pFrmData = _pFrmDllBuffer[_bufCompleteID];

	::EnterCriticalSection(&_remapSection);

	for (bdID = 0; bdID < _numNIDAQ; bdID++)
	{
		long numDataPerFrPerCh = _imgPtyDll.pixelY * _imgPtyDll.pixelX ;
		start = line_start_indexB[bdID];
		end = start - transferLength;
		switch (_numChannel[bdID])
		{
		case 0:
			break;
		default:
			{
				for(j = transferLength ; j < transferLength* 2;  j++)
				{		
					if(numDataPerFrPerCh > index)
					{
						pFrmData[index] = static_cast<unsigned short> (min(abs((int) _datamap[0][_pData[bdID][_bufCompleteID][start + j]]), 0x3FFF));
						pFrmData[numDataPerFrPerCh + index] = static_cast<unsigned short> (min(abs((int) _datamap[1][_pData[bdID][_bufCompleteID][numDataPerFrPerCh + start + j]]), 0x3FFF));
						index++;
					}
				}

				line_start_indexB[bdID] += transferLength * _indexLineFactor;
			}		
			break;
		}
	}
	::LeaveCriticalSection(&_remapSection);
	return TRUE;
}

//Copy line buffer from previous frame
long ThorLSMCam::ProcessLineOffset(long &index, long offset)
{
	unsigned short* pFrmData = _pFrmDllBuffer[_bufCompleteID];

	for (long bdID = 0; bdID < _numNIDAQ; bdID++)
	{
		unsigned short* pFrmDataPre = _pFrmDllBuffer[(_bufCompleteID-1+_imgPtyDll.dmaBufferCount)%_imgPtyDll.dmaBufferCount];
		long numDataPerFrPerCh = _imgPtyDll.pixelY * _imgPtyDll.pixelX ;
		switch (_numChannel[bdID])
		{
		case 0:
			break;
		default:
			for (long j = 0; j < offset; j++)
			{
				if((_1stSet_CMA_Frame) && (0 == _bufCompleteID))
				{
					//copy from previous line if first frame in interleave scan
					if(numDataPerFrPerCh > index)
					{
						pFrmData[index] = static_cast<unsigned short> (min(abs((int)pFrmData[index - offset]),0x3FFF));
						pFrmData[numDataPerFrPerCh + index] = static_cast<unsigned short> (min(abs((int)pFrmData[numDataPerFrPerCh + index - offset]),0x3FFF));
						index++;
					}
				}
				else
				{
					//copy from last frame, limit to a 14bit range
					if(numDataPerFrPerCh > index)
					{
						pFrmData[index] = static_cast<unsigned short> (min(abs((int)pFrmDataPre[index]),0x3FFF));
						pFrmData[numDataPerFrPerCh + index] = static_cast<unsigned short> (min(abs((int)pFrmDataPre[numDataPerFrPerCh + index]),0x3FFF));
						index++;
					}
				}
			}
			break;
		}
	}
	return TRUE;
}

//Cumulative Moving Average by Frames
long ThorLSMCam::ProcessBufferFrmCMA(void)
{
	if(_1stSet_CMA_Frame)
	{
		return ProcessBufferNoAve();
	}

	long start_index = 0;
	if(!ActiveBehavior())
		return FALSE;

	for (int i = 0; i < MAX_BOARD_NUM; i++)
	{
		_line_start_indexF[i] = start_index;
		_line_start_indexB[i] = start_index;
	}

	_index = 0;
	_offset = _imgPtyDll.pixelX;			//offset one line in interleave scan
	Func1 = &ProcessForwardLineFrmCMA;
	Func2 = &ProcessBackwardLineFrmCMA;
	Func3 = &ProcessLineOffset;

	_behaviorPtr->GetParam(BehaviorProp::LINE_FACTOR, _indexLineFactor);
	return TRUE;
}

long ThorLSMCam::ProcessForwardLineFrmCMA(long &index, long *line_start_indexF)
{
	long j;
	long start, end, bdID;
	long transferLength = _imgPtyDll.pixelX;
	unsigned short* pFrmData = _pFrmDllBuffer[_bufCompleteID];
	long aveNum = (long) _imgPtyDll.averageNum;

	::EnterCriticalSection(&_remapSection);

	for (bdID = 0; bdID < _numNIDAQ; bdID++)
	{
		long numDataPerFrPerCh = _imgPtyDll.pixelY * _imgPtyDll.pixelX ;
		start = line_start_indexF[bdID];
		end = start + transferLength;
		switch (_numChannel[bdID])
		{
		case 0:
			break;
		default:
			for (j = 0; j < transferLength; j++)
			{
				if(numDataPerFrPerCh > index)
				{
					pFrmData[index] = static_cast<unsigned short> min((pFrmData[index] * (aveNum - 1) + abs((int) _datamap[0][_pData[bdID][_bufCompleteID][start + j]])) / aveNum, 0x3FFF);
					pFrmData[numDataPerFrPerCh + index] = 
						static_cast<unsigned short> min((pFrmData[numDataPerFrPerCh + index] * (aveNum - 1) + abs((int) _datamap[1][_pData[bdID][_bufCompleteID][numDataPerFrPerCh + start + j]])) / aveNum, 0x3FFF);
					index++;
				}
			}
			line_start_indexF[bdID] += transferLength * _indexLineFactor;
			break;
		}
	}
	::LeaveCriticalSection(&_remapSection);
	return TRUE;
}

long ThorLSMCam::ProcessBackwardLineFrmCMA(long &index, long *line_start_indexB)
{
	long j;
	long start, end, bdID;
	long transferLength = _imgPtyDll.pixelX;
	unsigned short* pFrmData = _pFrmDllBuffer[_bufCompleteID];
	long aveNum = (long) _imgPtyDll.averageNum;

	::EnterCriticalSection(&_remapSection);

	for (bdID = 0; bdID < _numNIDAQ; bdID++)
	{
		long numDataPerFrPerCh = _imgPtyDll.pixelY * _imgPtyDll.pixelX ;
		start = line_start_indexB[bdID];
		end = start - (long)transferLength;
		switch (_numChannel[bdID])
		{
		case 0:
			break;
		default:
			for(j = transferLength ; j < transferLength* 2;  j++)
			{
				if(numDataPerFrPerCh > index)
				{
					pFrmData[index] = static_cast<unsigned short> min((pFrmData[index] * (aveNum - 1) + abs((int) _datamap[0][_pData[bdID][_bufCompleteID][start + j]])) / aveNum, 0x3FFF);
					pFrmData[numDataPerFrPerCh + index] = 
						static_cast<unsigned short> min((pFrmData[numDataPerFrPerCh + index] * (aveNum - 1) + abs((int) _datamap[1][_pData[bdID][_bufCompleteID][numDataPerFrPerCh + start + j]])) / aveNum, 0x3FFF);
					index++;
				}
			}
			line_start_indexB[bdID] += transferLength * _indexLineFactor;
			break;
		}
	}
	::LeaveCriticalSection(&_remapSection);
	return TRUE;
}

//Simple Moving Average by Frame
//this is not being implemented yet.  Code need to change to use it.
//long ThorLSMCam::ProcessBufferFrmSMA(long bufID, unsigned short *pFrmData, U32 transferLength)
//{
//	long line_start_index = 10;
//	long i,j, bdID;
//	long index = 0;
//	long aveNum = (long) _imgPtyDll.averageNum;
//	for (i = 0; i < (long)ImageWaveformBuilder->GetForwardLines(); i++)
//	{
//		//forward x scan
//		if ((_imgPtyDll.scanMode == TWO_WAY_SCAN) || (_imgPtyDll.scanMode == FORWARD_SCAN))
//			for (bdID = 0; bdID < _numNIDAQ; bdID++)
//			{
//				switch (_channelMode[bdID])
//				{
//				case 0:
//					break;
//				case 1:
//				case 2:
//					for (j = line_start_index; j < (line_start_index + (long)_imgPtyDll.pixelX); j++)
//					{
//						pFrmData[index] = static_cast<unsigned short>((pFrmData[index++] * (aveNum - 1) + _datamap[0][_pData[bdID][bufID][j]]) / aveNum);
//					}
//					break;
//				case 3:
//					for (j = line_start_index; j < (line_start_index + (long)_imgPtyDll.pixelX); j++)
//					{
//						pFrmData[index] = static_cast<unsigned short>((pFrmData[index++] * (aveNum - 1) + _datamap[0][_pData[bdID][bufID][j]]) / aveNum);
//					}
//					for (j = line_start_index + static_cast<long>(transferLength); j < (line_start_index + static_cast<long>(_imgPtyDll.pixelX + transferLength)); j++)
//					{
//						pFrmData[index] = static_cast<unsigned short>((pFrmData[index++] * (aveNum - 1) + _datamap[0][_pData[bdID][bufID][j]]) / aveNum);
//					}
//					break;
//				}
//			}
//			//back scan
//			if ((_imgPtyDll.scanMode == TWO_WAY_SCAN) || (_imgPtyDll.scanMode == BACKWARD_SCAN))
//				for (bdID = 0; bdID < _numNIDAQ; bdID++)
//				{
//					line_start_index += transferLength - 17;
//					switch (_channelMode[bdID])
//					{
//					case 0:
//						break;
//					case 1:
//					case 2:
//						for (j = line_start_index; j > (line_start_index - (long)_imgPtyDll.pixelX); j--)
//						{
//							pFrmData[index] = static_cast<unsigned short>((pFrmData[index++] * (aveNum - 1) + _datamap[0][_pData[bdID][bufID][j]]) / aveNum);
//						}
//						line_start_index += 17;
//						break;
//					case 3:
//						for (j = line_start_index; j > (line_start_index - (long)_imgPtyDll.pixelX); j--)
//						{
//							pFrmData[index] = static_cast<unsigned short>((pFrmData[index++] * (aveNum - 1) + _datamap[0][_pData[bdID][bufID][j]]) / aveNum);
//						}
//						for (j = line_start_index + static_cast<long>(transferLength); j > (line_start_index - static_cast<long>(_imgPtyDll.pixelX + transferLength)); j--)
//						{
//							pFrmData[index] = static_cast<unsigned short>((pFrmData[index++] * (aveNum - 1) + _datamap[0][_pData[bdID][bufID][j]]) / aveNum);
//						}
//						line_start_index += transferLength + 17;
//						break;
//					}
//				}
//	}
//	return TRUE;
//}

