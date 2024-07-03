// ThorDAQGalvoGalvo.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "dFLIMGG.h"
#include "thordaqGalvoGalvoSetupXML.h"
#include <queue>

#ifdef LOGGING_ENABLED
#include "..\..\..\..\..\Common\Log.h"
static std::auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

#define DAQmxErrChk(fnName,fnCall) if ((error=(fnCall)) < 0){ StringCbPrintfW(message,MSG_SIZE,L"%s GR failed (%d),(%d). ",fnName,error,__LINE__); CdFLIMGG::GetInstance()->LogMessage(message,ERROR_EVENT); throw "fnCall";}else{ StringCbPrintfW(message,MSG_SIZE,L"%s GR failed (%d) (%d). ",fnName,error,__LINE__); CdFLIMGG::GetInstance()->LogMessage(message,VERBOSE_EVENT);}
#define MAX_TASK_WAIT_TIME 10.0

///******	Initialize Static Members		******///
bool CdFLIMGG::_instanceFlag = false;
ImgAcqPty CdFLIMGG::_imgAcqPty = ImgAcqPty();
ImgAcqPty CdFLIMGG::_imgAcqPty_Pre = ImgAcqPty();
dFLIM_IMAGING_CONFIGURATION_STRUCT CdFLIMGG::_daqAcqCfg = dFLIM_IMAGING_CONFIGURATION_STRUCT();
long CdFLIMGG::_bufferHSize = 0;
auto_ptr<CdFLIMGG> CdFLIMGG::_single(new CdFLIMGG());//Instantiated on first use
HANDLE CdFLIMGG::_hStopAcquisition; // = CreateEvent(NULL, TRUE, FALSE, NULL);	//make sure the reset option is true (MANUAL RESET)
HANDLE CdFLIMGG::_hThreadStopped; // = CreateEvent(NULL, TRUE, TRUE, NULL);
HANDLE CdFLIMGG::_hTriggerTimeout; // = CreateEvent(NULL, FALSE, FALSE, NULL);
HANDLE CdFLIMGG::_hHardwareTriggerInEvent; // = CreateEvent(NULL, FALSE, FALSE, NULL);
HANDLE CdFLIMGG::_hAcquisitionThread = NULL; // Main Acquisition thread
wchar_t CdFLIMGG::message[MSG_SIZE];
//UCHAR* CdFLIMGG::_BufferDDR3chanArray[] = { NULL, NULL, NULL, NULL };
//ThorRawDDR3buffer* S2MMsingleDDR3pingPongRawBuff; // one contiguous buffer for the 4 channels (single ping-pong bank)

double CdFLIMGG::_frameRate = 1;

long long CdFLIMGG::_index_of_last_written_frame = 0;
long long CdFLIMGG::_index_of_last_read_frame = 0;

long CdFLIMGG::_DAQDeviceIndex = DEFAULT_CARD_NUMBER;
FrameCirBuffer* CdFLIMGG::_pFrmBuffer = nullptr;
vector<FlimBuffer*> CdFLIMGG::_pFlimBuffer;
vector<UCHAR*> CdFLIMGG::_pRawDataBuffer;
FlimBuffer* CdFLIMGG::_pHistoryFlimBuf = nullptr; 
DataStream* CdFLIMGG::_pDataProcess = nullptr;
long CdFLIMGG::_acquisitionMode = ACQUISITION_MODE::DFLIM;
HANDLE CdFLIMGG::_hFrmBufHandle = NULL;
long CdFLIMGG::_shiftArray[256];
long CdFLIMGG::_triggerWaitTimeout = 0;
atomic<bool> CdFLIMGG::_acquisitionRunning = false;

/**********************************************************************************************//**
 * @fn	CdFLIMGG::CdFLIMGG()
 *
 * @brief	Default constructor.
 *
 **************************************************************************************************/
CdFLIMGG::CdFLIMGG()
{
	_hStopAcquisition = CreateEvent(NULL, TRUE, FALSE, NULL);	//make sure the reset option is true (MANUAL RESET)
	_hThreadStopped = CreateEvent(NULL, TRUE, TRUE, NULL);
	_hTriggerTimeout = CreateEvent(NULL, FALSE, FALSE, NULL);
	_hHardwareTriggerInEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_hAcquisitionThread = NULL;

	_deviceNum = 0;
	_pDetectorName = L"ThorDFLIM4002GG";
	_field2Theta = 0.0901639344; //the field to scan angle conversion adapted from resonant galvo scanner code
	_theta2Volts = 1.0;
	_frameRate = 1;
	_oneXFieldSize = 160;
	_fieldSizeCalibrationAvailable = FALSE;
	_forceSettingsUpdate = FALSE;
	_droppedFramesCnt = 0;
	_minimizeFlybackCycles = true;
	_triggerWaitTimeout = DEFAULT_TRIGGER_TIMEOUT;
	_frameTriggerEnableWithHWTrig = TRUE;
	_pFrmBuffer = NULL;
	_ggSuperUserMode = FALSE;

	SAFE_DELETE_PTR(_pDataProcess);
	SAFE_DELETE_PTR(gPtrMemoryPool);

	for(long i=0; i<4; i++)
	{
		_channelPolarity[i] = 0;
	}
	_pockelsParkAtMinimum = 0;
	_pockelsDelayUS = 0;
	_pockelDigOutput = "";

	for(long i=0; i<MAX_POCKELS_CELL_COUNT; i++)
	{
		_pockelsEnable[i] = false;
		_pockelsMinVoltage[i] = 0.0;
		_pockelsMaxVoltage[i] = 1.0;
		//_taskHandleAIPockels[i] = 0;
		_pockelsScanVoltageStart[i] = 0.0;
		_pockelsScanVoltageStop[i] = 0.0;
		_pockelsPowerInputLine[i] = "";
		_pockelsResponseType[i] = 0;
		//memset(&_pockelsReadArray[i],0,POCKELS_VOLTAGE_STEPS * sizeof(float64));
	}
	//S2MMsingleDDR3pingPongRawBuff = new ThorRawDDR3buffer(DDR3_SINGLEIMAGE_CHANNEL_CAP * MAX_CHANNEL_COUNT); // NWL-kernel, e.g. the 224MB max DDR3 * 4 channels (single ping-pong bank)
	gPtrMemoryPool = new MemoryPool(0x8000000, DAC_DDR3_BUF_OFFSET); 
	_pDataProcess = new DataStream();
	_pTempCopyFlimBuf = NULL;

	for (long i = 0; i < NUM_DFLIM_CLOCKS; ++i)
	{
		_dflimClockFrequencies[i] = 0;
	}

	for (long i = 0; i < MAX_CHANNEL_COUNT; ++i)
	{
		_dflimFineShiftA[i] = 0;
		_dflimFineShiftB[i] = 0;
		_dflimCoarseShiftA[i] = 0;
		_dflimCoarseShiftB[i] = 0;
		_dflimEnableIntAdj[i] = 0;
		_dflimThreshold[i] = 0;
		_dflimBaselineTolerance[i] = 0;
		_dflimMaxLevel0[i] = 0;
		_dflimMaxLevel1[i] = 0;
	}

	for (int i = 0; i < 256; ++i)
	{
		_shiftArray[i] = 0;
	}

	_dflimSyncDelay = 0;
	_dflimResyncDelay = 0;
	_dflimResyncEveryLine = 0;
	_saveLiveImage = false;

	_maxAngularVelocityRadPerSec = DEFAULT_MAX_GALVO_VELOCITY;
	_maxAngularAccelerationRadPerSecSq = DEFAULT_MAX_GALVO_ACCELERATION;
}

/**********************************************************************************************//**
 * @fn	CdFLIMGG::~CdFLIMGG()
 *
 * @brief	Destructor.
 *
 **************************************************************************************************/
CdFLIMGG::~CdFLIMGG()
{
	/*
	try
	{
		CloseHandle(_hThreadStopped);
		CloseHandle(_hStopAcquisition);
		CloseHandle(_hTriggerTimeout);
		CloseHandle(_hHardwareTriggerInEvent);
	}
	catch (exception e)
	{
#ifdef _DEBUG 
		printf("error");
#endif
	}
	*/
	//Reset the device parameter
	SAFE_DELETE_PTR(gPtrMemoryPool);
	_deviceNum = 0;
	_single.release();
	_instanceFlag = false;

//	delete  S2MMsingleDDR3pingPongRawBuff;
}


/**********************************************************************************************//**
 * @fn	CdFLIMGG* CdFLIMGG::GetInstance()
 *
 * @brief	Gets the instance. Singlton design pattern
 *
 * @return	null if it fails, else the instance.
 **************************************************************************************************/
CdFLIMGG* CdFLIMGG::GetInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new CdFLIMGG());
		_instanceFlag = true;
		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

/**********************************************************************************************//**
 * @fn	long CdFLIMGG::FindCameras(long &cameraCount)
 *
 * @brief	Searches for the devices.
 *
 * @param [in,out]	cameraCount	Index of Camera.
 *
 * @return	False if no Camera found. True if Camera found.
 **************************************************************************************************/
long CdFLIMGG::FindCameras(long &cameraCount)
{
	long error = 0, retVal = 0;
	StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorLSMCam::FindCameras");
	LogMessage(_errMsg,VERBOSE_EVENT);
	//MessageBox(NULL, L"test", L"test", MB_DEFBUTTON1);
	// First make sure there is a board installed
	try
	{
		// was "ThordaqConnectToBoard((_DAQDeviceIndex)", now linked to common thordaq.dll
		ThordaqErrChk (L"ThorDAQAPIBindBoard", retVal = ThorDAQAPIBindBoard(_DAQDeviceIndex));
		if (retVal == STATUS_SUCCESSFUL)
		{
			cameraCount = 1;
			_deviceNum = 1;

			//Load parameter from XML
			try
			{
				auto_ptr<ThorGalvoGalvoXML> pSetup(new ThorGalvoGalvoXML());
				//XML settings retrieval functions will throw an exception if tags or attributes are missing
				//catch each independetly so that as many tags as possible can be read
				try
				{
					if(FALSE == pSetup->GetConfiguration(_field2Theta, _pockelsParkAtMinimum, _pockelsDelayUS, _acquisitionMode, _maxAngularVelocityRadPerSec, _maxAngularAccelerationRadPerSecSq))
					{
						StringCbPrintfW(_errMsg,MSG_SIZE,L"GetConfiguration from ThorDFLIMGalvoGalvoSettings.XML failed");
						LogMessage(_errMsg,ERROR_EVENT);
					}
				}
				catch(...)
				{
				}

				try
				{
					long vertScanDir = 0;
					if(FALSE == pSetup->GetCalibration(_fieldSizeCalibration,vertScanDir,_imgAcqPty.fineOffsetX,_imgAcqPty.fineOffsetY,_imgAcqPty.fineFieldSizeScaleX,_imgAcqPty.fineFieldSizeScaleY,_oneXFieldSize))
					{
						//if fieldSizeCalibration not exists, set its GetParamInfo Available
						_fieldSizeCalibrationAvailable = FALSE;

						StringCbPrintfW(_errMsg,MSG_SIZE,L"GetCalibration from ThorDFLIMGalvoGalvoSettings.XML failed. FieldSizeCalibration not available.");
						LogMessage(_errMsg,ERROR_EVENT);
					}
					else
					{
						_fieldSizeCalibrationAvailable = TRUE;
						_imgAcqPty.verticalScanDirection = (0 == vertScanDir)? 1 : -1;
					}
				}
				catch(...)
				{
				}

				try
				{
					if(FALSE == pSetup->GetIO(_pockelsVoltageSlopeThreshold,
						_pockelsEnable[0],_pockelsPowerInputLine[0],_pockelsScanVoltageStart[0],_pockelsScanVoltageStop[0],
						_pockelsEnable[1],_pockelsPowerInputLine[1],_pockelsScanVoltageStart[1],_pockelsScanVoltageStop[1],
						_pockelsEnable[2],_pockelsPowerInputLine[2],_pockelsScanVoltageStart[2],_pockelsScanVoltageStop[2],
						_pockelsEnable[3],_pockelsPowerInputLine[3],_pockelsScanVoltageStart[3],_pockelsScanVoltageStop[3],
						_pockelsReferenceLine,_pockelsResponseType[0],_pockelsResponseType[1],_pockelsResponseType[2],_pockelsResponseType[3]))
					{
						StringCbPrintfW(_errMsg,MSG_SIZE,L"GetIO from ThorDFLIMGalvoGalvoSettings failed");
						LogMessage(_errMsg,ERROR_EVENT);
					}
				}
				catch(...)
				{
				}

				try
				{
					if(FALSE == pSetup->GetTrigger(_triggerWaitTimeout))
					{
						StringCbPrintfW(_errMsg,MSG_SIZE,L"GetTrigger from ThorDFLIMGalvoGalvoSettings failed");
						LogMessage(_errMsg,ERROR_EVENT);
					}
					//Get Stream Control always using internal clock rate
					if(FALSE == pSetup->GetStreamConfiguration(_imgAcqPty.clockRateInternal,_imgAcqPty.FIRFilter[0],_imgAcqPty.FIRFilter[1],_imgAcqPty.DCOffset[0],_imgAcqPty.DCOffset[1]))
					{
						StringCbPrintfW(_errMsg,MSG_SIZE,L"GetStream from ThorDFLIMGalvoGalvoSettings.XML failed");
						LogMessage(_errMsg,ERROR_EVENT);
					}
				}
				catch(...)
				{
				}				

				//try
				//{
				//	if(FALSE == pSetup->GetFrontEndTuning(_dflimSyncDelay, _dflimResyncDelay, _dflimResyncEveryLine))
				//	{
				//		StringCbPrintfW(_errMsg,MSG_SIZE,L"GetTrigger from ThorDFLIMGalvoGalvoSettings failed");
				//		LogMessage(_errMsg,ERROR_EVENT);
				//	}

				//	//for now always force to be false at startup
				//	_dflimResyncEveryLine = FALSE;

				//	long error = 0, status = 0;

				//	ThordaqErrChk (L"ThorDAQAPIdFLIMSetSyncingSettings", status = ThorDAQAPIdFLIMSetSyncingSettings(_DAQDeviceIndex, static_cast<ULONG32>(_dflimSyncDelay), static_cast<ULONG32>(_dflimResyncDelay), TRUE == _dflimResyncEveryLine));
				//	Sleep(20);
				//}
				//catch(...)
				//{
				//}
				{
					long error = 0, status = 0;
					ThordaqErrChk(L"ThorDAQAPIdFLIMSetFrontEndSettings", status = ThorDAQAPIdFLIMSetFrontEndSettings(_DAQDeviceIndex));

					Sleep(20);

					//ThordaqErrChk (L"ThorDAQAPIdFLIMSetSyncingSettings", status = ThorDAQAPIdFLIMSetSyncingSettings(_DAQDeviceIndex, static_cast<ULONG32>(_dflimSyncDelay), static_cast<ULONG32>(_dflimResyncDelay), TRUE == _dflimResyncEveryLine));

				}
				try
				{
					if(FALSE == pSetup->GetChannelSettings(_dflimFineShiftA,_dflimFineShiftB,_dflimCoarseShiftA,_dflimCoarseShiftB,_dflimEnableIntAdj,_dflimThreshold,_dflimBaselineTolerance,_dflimMaxLevel0,_dflimMaxLevel1))
					{
						StringCbPrintfW(_errMsg,MSG_SIZE,L"GetTrigger from ThorDFLIMGalvoGalvoSettings failed");
						LogMessage(_errMsg,ERROR_EVENT);
					}

					long error = 0, status = 0;

					ThordaqErrChk(L"ThorDAQAPIdFLIMSetFineShift", status = ThorDAQAPIdFLIMSetFineShift(_DAQDeviceIndex, (ULONG32)_dflimFineShiftA[0], 0));
					ThordaqErrChk(L"ThorDAQAPIdFLIMSetFineShift", status = ThorDAQAPIdFLIMSetFineShift(_DAQDeviceIndex, (ULONG32)_dflimFineShiftA[1], 1));
					ThordaqErrChk(L"ThorDAQAPIdFLIMSetFineShift", status = ThorDAQAPIdFLIMSetFineShift(_DAQDeviceIndex, (ULONG32)_dflimFineShiftA[2], 2));
					ThordaqErrChk(L"ThorDAQAPIdFLIMSetFineShift", status = ThorDAQAPIdFLIMSetFineShift(_DAQDeviceIndex, (ULONG32)_dflimFineShiftA[3], 3));

					ThordaqErrChk(L"ThorDAQAPIdFLIMSetCoarseShift", status = ThorDAQAPIdFLIMSetCoarseShift(_DAQDeviceIndex, (ULONG32)_dflimCoarseShiftA[0], 0));
					ThordaqErrChk(L"ThorDAQAPIdFLIMSetCoarseShift", status = ThorDAQAPIdFLIMSetCoarseShift(_DAQDeviceIndex, (ULONG32)_dflimCoarseShiftA[1], 1));
					ThordaqErrChk(L"ThorDAQAPIdFLIMSetCoarseShift", status = ThorDAQAPIdFLIMSetCoarseShift(_DAQDeviceIndex, (ULONG32)_dflimCoarseShiftA[2], 2));
					ThordaqErrChk(L"ThorDAQAPIdFLIMSetCoarseShift", status = ThorDAQAPIdFLIMSetCoarseShift(_DAQDeviceIndex, (ULONG32)_dflimCoarseShiftA[3], 3));
				}
				catch(...)
				{
				}

				try
				{
					Sleep(20);

					if(FALSE == pSetup->GetFrontEndTuning(_dflimSyncDelay, _dflimResyncDelay, _dflimResyncEveryLine))
					{
						StringCbPrintfW(_errMsg,MSG_SIZE,L"GetTrigger from ThorDFLIMGalvoGalvoSettings failed");
						LogMessage(_errMsg,ERROR_EVENT);
					}

					//for now always force to be false at startup
					_dflimResyncEveryLine = FALSE;

					long error = 0, status = 0;

					ThordaqErrChk (L"ThorDAQAPIdFLIMSetSyncingSettings", status = ThorDAQAPIdFLIMSetSyncingSettings(_DAQDeviceIndex, static_cast<ULONG32>(_dflimSyncDelay), static_cast<ULONG32>(_dflimResyncDelay), TRUE == _dflimResyncEveryLine));
					Sleep(20);
				}
				catch(...)
				{
				}

			}
			catch(...)
			{
			}
		}
	}
	catch (...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Hardware communication error ThorDAQ ConnectToBoard failed");
		LogMessage(_errMsg,ERROR_EVENT);
		return FALSE;
	}
	return cameraCount;
}

/**********************************************************************************************//**
 * @fn	long CdFLIMGG::SelectCamera(const long camera)
 *
 * @brief	Select Camera.
 *
 * @param	camera	The camera index .
 *
 * @return	A long.
 **************************************************************************************************/
long CdFLIMGG::SelectCamera(const long camera)
{
	if (_deviceNum == 0)// No thordaq is connected
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"The hardware has not been located");
		LogMessage(_errMsg,ERROR_EVENT);
		return FALSE;
	}	

	//Load parameter from XML
	try
	{
		AlignDataLoadFile(); ///load the alignment data if exists;
	}
	catch(...)
	{
	}

	//park the galvo
	MoveGalvoToParkPosition(GalvoGalvoX,GalvoGalvoY);
	//

	return TRUE;
}

/**********************************************************************************************//**
 * @fn	long CdFLIMGG::TeardownCamera()
 *
 * @brief	Teardown Camera.
 *
 * @return	A long.
 **************************************************************************************************/
long CdFLIMGG::TeardownCamera()
{
	long error = 0, retVal = 0;
	StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorLSMCam::TeardownCamera");
	LogMessage(_errMsg, VERBOSE_EVENT);

	//Stop the acquisition thread
	ThordaqErrChk(L"ThorDAQAPIAbortRead", retVal = ThorDAQAPIAbortRead(_DAQDeviceIndex));
	SetEvent(_hStopAcquisition);
	//Wait for acqisition's stop
	Sleep(500);
	WaitForSingleObject(_hThreadStopped, 5000);

	//terminate daq thread:
	StopDaqBrd();
	//save the configuration settings

	//park the galvo when exiting
	MoveGalvoToParkPosition(GalvoGalvoX, GalvoGalvoY);
	MovePockelsToParkPosition(&_imgAcqPty.pockelPty);
	//park the pockels cell

	//close the shutter

	//disconnect DAQ board
	_deviceNum = 0;
	//// Clean all the threads
	SAFE_DELETE_HANDLE(_hAcquisitionThread);

	////// Close the events to clean up
	// we don't care if the handles are invalid because we're discarding them anyway
/*	try
	{
		CloseHandle(_hThreadStopped);
		CloseHandle(_hStopAcquisition);
		CloseHandle(_hTriggerTimeout);
		CloseHandle(_hHardwareTriggerInEvent);
	}
	catch (exception e)
	{
#ifdef _DEBUG 
		printf("error");
#endif
	}*/
	//// Free all the buffers
	SAFE_DELETE_PTR(_pFrmBuffer);

	if(_pFlimBuffer.size() > 0)
	{
		for(int i=0;i < _pFlimBuffer.size();i++)
		{
			//VirtualFree(_pBuffer.at(i), 0, MEM_RELEASE);
			SAFE_DELETE_PTR(_pFlimBuffer.at(i));
		}
		_pFlimBuffer.clear();				
	}

	SAFE_DELETE_ARRAY(_pHistoryFlimBuf);

	SAFE_DELETE_ARRAY(_pTempCopyFlimBuf);
	//auto_ptr<ThorGalvoGalvoXML> pSetup(new ThorGalvoGalvoXML());
	//if(FALSE == pSetup->SetChannelSettings(_dflimFineShiftA,_dflimFineShiftB,_dflimCoarseShiftA,_dflimCoarseShiftB,_dflimEnableIntAdj,_dflimThreshold,_dflimBaselineTolerance,_dflimMaxLevel0,_dflimMaxLevel1))
	//{
	//	StringCbPrintfW(_errMsg,_MAX_PATH,L"SetChannelSettings from DFLIM ThorDAQGalvoGalvoSettings failed");
	//	LogMessage(_errMsg,ERROR_EVENT);
	//}

	return TRUE;
}

// This function is strictly for Utility (e.g. CL-GUI) testing!
// Instead of calling separate Params, use one function to set the group of critical variables needed
// We set only the "params" we need for testing (simulating limited set of larger param set written by TILS)
// after this call, we proceed with PreFlight and SetupAcq calls

long CdFLIMGG::dFLIMSetTestUtilConfig(CL_GUI_GLOBAL_TEST_STRUCT testConfig)
{
	if(testConfig.imgHSize != testConfig.imgVSize) _imgAcqPty.areaMode = 1; // rectangle MODE setting REQUIRED!

	_imgAcqPty.pixelX = testConfig.imgHSize;
	_imgAcqPty.pixelY = testConfig.imgVSize;
	_imgAcqPty.channel = testConfig.channel;
	_imgAcqPty.numFrame = testConfig.frameCnt;
	_imgAcqPty.dwellTime = testConfig.PixelDwell;
	_imgAcqPty.acquistionMode = testConfig.acquisitionMode;
	_imgAcqPty.triggerMode = testConfig.triggerMode;

	// copy our test settings, emulating the TILS SetParams() followed by upper level DLL invoking SetImagingConfiguration()
//	status = SetImagingConfiguration(imaging_config);
	_bufferHSize = ComputeDataHSize(_imgAcqPty);
	return _bufferHSize;
}

/**********************************************************************************************//**
 * @fn	long CdFLIMGG::PreflightAcquisition(char * pDataBuffer)
 *
 * @brief	Preflight position.
 * @param [in,out]	pDataBuffer	  	Identifier of the Data buffer.
 * @return	A long.
 **************************************************************************************************/
long CdFLIMGG::PreflightAcquisition(char * pDataBuffer)
{
	long ret = TRUE;
	// reset dropped frame count:
	_droppedFramesCnt = 0;

//#ifdef TEST
//	_imgAcqPty.channel = 0xf; // Test force to run at all 4 channels
//#endif
	
	_forceSettingsUpdate = TRUE;// Force to enable acquisition at first set 
	// verify the image size depends on the area mode
	switch(_imgAcqPty.areaMode)
	{
		case ICamera::SQUARE:
			_imgAcqPty.pixelY = _imgAcqPty.pixelX; break;
		case ICamera::LINE:
			_imgAcqPty.pixelY = 1; break;
		default:;
	}
	// get rid of test mode
	switch (_imgAcqPty.scanMode)
	{
		case ScanMode::CENTER_SCAN_MODE:
		{
			MoveGalvoToCenter(GalvoGalvoX,GalvoGalvoY,&_imgAcqPty);

			for(long i=0; i<MAX_POCKELS_CELL_COUNT; i++)
			{
				MovePockelsToPowerLevel(i,&_imgAcqPty.pockelPty);
			}
			ret =  TRUE;
			break;
		}
		case ScanMode::WAVEFORM_MODE://test the waveform
		{
			ret = TRUE;
			break;
		}
		default:
        {
			//if we are in imaging mode, then set the parking position
			//to the regular parking position
		    MoveGalvoToParkPosition(GalvoGalvoX,GalvoGalvoY);
			MovePockelsToParkPosition(&_imgAcqPty.pockelPty);
			ret =  TRUE;
		}
	}
	return ret;
}



/**********************************************************************************************//**
 * @fn	long CdFLIMGG::SetupAcquisition(char * pDataBuffer)
 *
 * @brief	Setup  Acquisition.
 * @param [in,out]	pDataBuffer	  	Identifier of the Data buffer.
 * @return	A long.
 **************************************************************************************************/
long CdFLIMGG::SetupAcquisition(char * pDataBuffer)
{
	long error = 0, retVal = 0;
	long ret = TRUE;

	//do not capture data if you are in the centering scan mode, Just Set the Scanner
	if(_imgAcqPty.scanMode == ScanMode::CENTER_SCAN_MODE)
	{
		return ret;
	}

	//If settings change or system is forced to refresh updating, do update.
	if (!_imgAcqPty.IsEqual(_imgAcqPty_Pre) || _forceSettingsUpdate == TRUE)
	{
		// Disable Force updating
		_forceSettingsUpdate = FALSE;
		
		/**********reset the acquisition thread and the event flag*******/
		//ThorDAQAPIAbortRead(_DAQDeviceIndex);
		ThordaqErrChk (L"ThorDAQAPIAbortRead", retVal = ThorDAQAPIAbortRead(_DAQDeviceIndex));
		SetEvent(_hStopAcquisition);
		WaitForSingleObject(_hThreadStopped, 12000);

		ResetEvent(_hTriggerTimeout);//Reset the acquisition timeout event flag

		// Release Circular buffer mutex
		if(_hFrmBufHandle != NULL)
		{
			CloseHandle(_hFrmBufHandle);
			_hFrmBufHandle = NULL;
		}
		_hFrmBufHandle = CreateMutex(NULL, false, NULL);

		// verify the image size depends on the area mode
		switch(_imgAcqPty.areaMode)
		{
			case ICamera::SQUARE:
				_imgAcqPty.pixelY = _imgAcqPty.pixelX; break;
			case ICamera::LINE:
				_imgAcqPty.pixelY = 1; break;
			default:;
		}

		if(ICamera::LSMAreaMode::POLYLINE == _imgAcqPty.areaMode)
		{
			_imgAcqPty.scanMode = ScanMode::FORWARD_SCAN_MODE;
		}

		_imgAcqPty_Pre = _imgAcqPty; // Save the image acquisition struct

		if (ConfigAcqSettings(&_imgAcqPty_Pre) == FALSE)  // builds DAC waveforms and sets up complex image frame buff
		{
			ret = FALSE;
		}
	}
	
	return ret;
}

// function to compute upper constraint on dFLIM Pixel Dwell given 
// Resolution X, Y, and input Pixel Dwell, where X and Y are not altered,
// but Dwell will be in frame DMA len exceeds max (i.e. 224 MB)
THORDAQ_STATUS CdFLIMGG::ConstrainHighestPixelDwell(const long Xpixels, const long Ypixels, double *PixelDwell)
{
	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_FUNCTION_NOT_SUPPORTED;	 // default status - do nothing
	long dataBytesInImageLine, totalRawDMAlen, MaxAllowedDMAlen = 0xE000000;
	const long PRECISION = 8;
	long tempBufferHSizeBeats = 0;
	const long BYTES_PER_BEAT = 8;
	const double multFactor1 = 1.125;
	const double multFactor2 = 4;
	const double constFactor = 66;
	const double allowanceFactor = 1.05;
	const double sizeAdjuster = 1;

	// sanity check passed variables...
	if (Xpixels < 128 || Ypixels < 128 || *PixelDwell <= 0.0)
		return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;
	if (Xpixels > 2048 || Ypixels > 2048 )
		return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;

	// we need to compute the overall raw DDR3 DMA frame len to know whether the passed PixelDwell valid makes
	// a frame fit within the 224 MB max DDR3 buffer allocation
	tempBufferHSizeBeats = static_cast<long>(ceil((constFactor + Xpixels * multFactor1 + Xpixels * (*PixelDwell) * multFactor2) * allowanceFactor) * sizeAdjuster);
	//HSize should be divisible by PRECISION, in this case 8
	dataBytesInImageLine = (static_cast<long>(floor((tempBufferHSizeBeats + PRECISION - 1) / PRECISION)) * PRECISION) * BYTES_PER_BEAT;
	totalRawDMAlen = dataBytesInImageLine * Ypixels;

	while (totalRawDMAlen > MaxAllowedDMAlen)
	{
		status = THORDAQ_STATUS::STATUS_SUCCESSFUL; // This means we have new (lower than default) max!
		// we must reduce the PixelDwell so frame will fit in 224MB max allocation
		(*PixelDwell) -= 0.1;
		tempBufferHSizeBeats = static_cast<long>(ceil((constFactor + Xpixels * multFactor1 + Xpixels * (*PixelDwell) * multFactor2) * allowanceFactor) * sizeAdjuster);
		dataBytesInImageLine = (static_cast<long>(floor((tempBufferHSizeBeats + PRECISION - 1) / PRECISION)) * PRECISION) * BYTES_PER_BEAT;
		totalRawDMAlen = dataBytesInImageLine * Ypixels;
		if (*PixelDwell <= 0.0)
		{
			status = THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;
			break; // insane infinite loop check
		}
	};
	return status;
}



// compute the dataHSize
// NOTE: params must be set by upper level software before calling this
// depends on variables PixelsXsize and Galvo PixelDwell

long CdFLIMGG::ComputeDataHSize(ImgAcqPty ImgAcqPty)
{
	long dataBytesInImageLine;
	const long PRECISION = 8;
	long tempBufferHSizeBeats = 0;
	const long BYTES_PER_BEAT = 8;
	const double multFactor1 = 1.125;
	const double multFactor2 = 4;
	const double constFactor = 66;
	const double allowanceFactor = 1.05;
	const double sizeAdjuster = 1;

	//Buffer HSize = (66 + pixels*1.125 + usec*4)* (1+5%)
	tempBufferHSizeBeats = static_cast<long>(ceil((constFactor + ImgAcqPty.pixelX * multFactor1 + ImgAcqPty.pixelX * ImgAcqPty.dwellTime * multFactor2) * allowanceFactor) * sizeAdjuster);

	//HSize should be divisible by PRECISION, in this case 8
	dataBytesInImageLine = (static_cast<long>(floor((tempBufferHSizeBeats + PRECISION - 1) / PRECISION)) * PRECISION) * BYTES_PER_BEAT;
	return dataBytesInImageLine;
}

/**********************************************************************************************//**
 * @fn	long CThorDAQGalvoGalvo::ConfigAcqSettings(ImgAcqPty* pImgAcqPty)
 *
 * @brief	Configure thordaq settings
 * @param [in,out]	pImgAcqPty	  	Identifier of Image Acquisition Struct.
 * @return	A long.
 **************************************************************************************************/
long CdFLIMGG::ConfigAcqSettings(ImgAcqPty* pImgAcqPty)
{
	long ret = TRUE;
	long error = 0, retVal = 0;
	Sleep(50); //Implemented to stop ThorDAQ GG from Scrolling. Tested that 20ms to be the min allowable delay before scrolling occurs.
	_acquisitionRunning = false;
	//Initiate the Struct
	memset(&_daqAcqCfg, 0, sizeof(dFLIM_IMAGING_CONFIGURATION_STRUCT)); 
	gPtrMemoryPool->ClearUpMemory();

	double dwell_time = pImgAcqPty->dwellTime / 1000000.0;
	_daqAcqCfg.imageCtrl.clock_source = CLOCK_SOURCE::INTERNAL_80MHZ_REF;
	_daqAcqCfg.imageCtrl.clockRate = DEFAULT_INTERNALCLOCKRATE;

	// system runs at external clock mode
	if (pImgAcqPty->clockSource != INTERNAL_CLOCK)
	{
		_daqAcqCfg.imageCtrl.clock_source = CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE; // works on external laser sync mode
		// 2P mode // need to add in the future
		if (pImgAcqPty->threePhotonModeEnable)
		{
			ULONG32 clockStatus = 0;
			// if external clock measure fails or 3P laser signal is not in the range, break.
			if (pImgAcqPty->clockRateExternal <= MIN_3PCLOCKRATE
				|| pImgAcqPty->clockRateExternal > MAX_3PCLOCKRATE
				|| ThorDAQAPIGetExternClockStatus(_DAQDeviceIndex, clockStatus) != THORDAQ_STATUS::STATUS_SUCCESSFUL
				|| clockStatus == 0)
			{
				MessageBox(NULL, L"Laser SYNC Error", L"Error", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
				return FALSE;
			}
			_daqAcqCfg.imageCtrl.clockRate = pImgAcqPty->clockRateExternal;
		}
	}

	bool oneWayLineScan = pImgAcqPty->pixelY == 1 && TWO_WAY_SCAN_MODE != pImgAcqPty->scanMode;

	switch (pImgAcqPty->acquistionMode)
	{
	case Diagnostics:
	{
		const long BYTES_PER_BEAT = 8;
		const long DIAGNOSTIC_HSIZE_BEATS = 266;
		_bufferHSize = DIAGNOSTIC_HSIZE_BEATS * BYTES_PER_BEAT;
	}
	break;
	default:
		_bufferHSize = ComputeDataHSize(*pImgAcqPty);
		break;
	}


	//TODO: test with normal acquisition (setting VSize to 1 instead of 2 for oneway scan)
	//always make line scan two-way backward line will be discarded in postprocessing
	long vSize = oneWayLineScan ? 2 : static_cast<USHORT>(pImgAcqPty->pixelY);

	_daqAcqCfg.imageCtrl.system_mode = SYSTEM_MODE::INTERNAL_GALVO_GALVO;
	_daqAcqCfg.imageCtrl.channel = static_cast<USHORT>(pImgAcqPty->channel);
	_daqAcqCfg.imageCtrl.imgHSize = static_cast<USHORT>(pImgAcqPty->pixelX);
	_daqAcqCfg.imageCtrl.imgVSize = static_cast<USHORT>(pImgAcqPty->pixelY);
	_daqAcqCfg.imageCtrl.dataHSize = static_cast<ULONG32>(_bufferHSize);
	_daqAcqCfg.imageCtrl.linesPerStripe =  static_cast<USHORT>(vSize);
	_daqAcqCfg.imageCtrl.scanMode = (TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode) ? SCAN_MODES::BIDIRECTION_SCAN : SCAN_MODES::UNIDIRECTION_SCAN;
	_daqAcqCfg.imageCtrl.scanDir = (pImgAcqPty->horizontalFlip == FALSE) ? SCAN_DIRECTION::FORWARD_SCAN : SCAN_DIRECTION::REVERSE_SCAN;
	_daqAcqCfg.imageCtrl.defaultMode = FALSE;
	_daqAcqCfg.imageCtrl.acquisitionMode =  pImgAcqPty->acquistionMode;//(pImgAcqPty == 0 || _acquisitionMode == 1) ?  static_cast<ULONG32>(_acquisitionMode) : 0;

	switch (pImgAcqPty->triggerMode)
	{
	case ICamera::SW_FREE_RUN_MODE:
		_daqAcqCfg.imageCtrl.frameCnt = MAX_FRAME_NUM;
		_daqAcqCfg.imageCtrl.triggerMode = TRIGGER_MODE::SOFTWARE_RUN_MODE;
		break;
	case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
		_daqAcqCfg.imageCtrl.frameCnt = MAX_FRAME_NUM;
		_daqAcqCfg.imageCtrl.triggerMode = TRIGGER_MODE::HARDWARE_TRIGGER_MODE;
		break;
	case ICamera::SW_SINGLE_FRAME:
		_daqAcqCfg.imageCtrl.frameCnt = 1;
		_daqAcqCfg.imageCtrl.triggerMode = TRIGGER_MODE::SOFTWARE_RUN_MODE;
		break;
	case ICamera::HW_SINGLE_FRAME:
		_daqAcqCfg.imageCtrl.frameCnt = 1;
		_daqAcqCfg.imageCtrl.triggerMode = TRIGGER_MODE::HARDWARE_TRIGGER_MODE;
		break;
	case ICamera::SW_MULTI_FRAME:
		_daqAcqCfg.imageCtrl.frameCnt = (MAX_FRAME_NUM <= pImgAcqPty->numFrame) ? (MAX_FRAME_NUM) : static_cast<ULONG32>(pImgAcqPty->numFrame);
		_daqAcqCfg.imageCtrl.triggerMode = TRIGGER_MODE::SOFTWARE_RUN_MODE;
		break;
	case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
		_daqAcqCfg.imageCtrl.frameCnt = (MAX_FRAME_NUM <= pImgAcqPty->numFrame) ? (MAX_FRAME_NUM) : static_cast<ULONG32>(pImgAcqPty->numFrame);
		_daqAcqCfg.imageCtrl.triggerMode = TRIGGER_MODE::HARDWARE_TRIGGER_MODE;
		break;
	}

	// set up pixel convertor
	//_pDataProcess->SetupDataMap(pImgAcqPty->dataMapMode, _channelPolarity);
	_pDataProcess->SetAcquisitionMode((ACQUISITION_MODE)_acquisitionMode);
	_pDataProcess->SetScanMode(pImgAcqPty->scanMode);
	//detemine the X& Y FOV in unit of volt, full swing of waveform,
	//based on field size and the image pixel aspect ratio
	// voltage required is happend to be the mechanical angle of the mirror 
	GalvoStruct galvo_x_control;
	GalvoStruct galvo_y_control;
	double theta = (double) pImgAcqPty->fieldSize * _field2Theta;
	galvo_x_control.amplitude = theta * _theta2Volts * pImgAcqPty->fineFieldSizeScaleX; // horizontal galvo amplitude
	galvo_y_control.amplitude = theta * _theta2Volts * (double) pImgAcqPty->pixelY / (double)pImgAcqPty->pixelX ; 
	galvo_y_control.amplitude = (pImgAcqPty->yAmplitudeScaler/100.0) * galvo_y_control.amplitude * pImgAcqPty->fineFieldSizeScaleY; // Vertical galvo amplitude
	galvo_x_control.offset = ((double) pImgAcqPty->offsetX * _field2Theta) * _theta2Volts + pImgAcqPty->fineOffsetX; //horizontal galvo offset
	galvo_y_control.offset = ((double) pImgAcqPty->verticalScanDirection * pImgAcqPty->offsetY * _field2Theta) * _theta2Volts + pImgAcqPty->fineOffsetY;// Vertical galvo offset
	galvo_y_control.scan_direction = pImgAcqPty->verticalScanDirection == 1? SCAN_DIRECTION::FORWARD_SCAN : SCAN_DIRECTION::REVERSE_SCAN;// Vertical galvo offset
	pImgAcqPty->flybackCycle = GetFlybackCycle();

	// Set up DAC settings
	ScanStruct scan_info = ScanStruct(pImgAcqPty->pixelY, pImgAcqPty->flybackCycle);

	ScanLineStruct scanLine = ScanLineStruct();

	double linetime = 0;
	double dac_rate = 0;
	long turnAroundSamples = 0;
	GetDACSamplesPerLine(&scanLine, pImgAcqPty, dac_rate, dwell_time, linetime, turnAroundSamples, oneWayLineScan);
	
	_daqAcqCfg.galvoGalvoCtrl.flybackCycle = pImgAcqPty->flybackCycle;
	_daqAcqCfg.galvoGalvoCtrl.flybackTime = 2 * linetime * _daqAcqCfg.galvoGalvoCtrl.flybackCycle;

	_frameRate = (TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode) ?  1.0 / (scan_info.overall_lines * linetime + _daqAcqCfg.galvoGalvoCtrl.flybackTime) :  1.0 / (scan_info.overall_lines * 2 * linetime + _daqAcqCfg.galvoGalvoCtrl.flybackTime); // One way is 2 linetimes for each Y pixel

	_daqAcqCfg.imageCtrl.frameNumPerTransfer = max(1, static_cast<ULONG32>(ceil(_frameRate / (double)MAX_TRANSFERS_PER_SECOND)));// set frequency of interrupt to 50ms min//1;//(_frameRate > 1)? static_cast<ULONG>(_frameRate): 1;
	_daqAcqCfg.imageCtrl.frameNumPerSec = max(1, static_cast<ULONG32>(ceil(_frameRate)));
	if (_daqAcqCfg.imageCtrl.frameCnt <= _daqAcqCfg.imageCtrl.frameNumPerTransfer)
	{
		_daqAcqCfg.imageCtrl.frameNumPerTransfer = _daqAcqCfg.imageCtrl.frameCnt;
	}
	if (_daqAcqCfg.imageCtrl.frameCnt == MAX_FRAME_NUM)
	{
		const double MIN_SEC_PER_INTERRUPT = 0.1; //100ms
		_daqAcqCfg.imageCtrl.frameNumPerTransfer = max(1,static_cast<ULONG32>(ceil((_frameRate) * MIN_SEC_PER_INTERRUPT)));
	}

	//_daqAcqCfg.imageCtrl.frameNumPerTransfer = _daqAcqCfg.imageCtrl.frameNumPerSec;
	_daqAcqCfg.streamingCtrl.fir_filter_enabled = TRUE;

	//Set the alignmentOffset before building the pockels waveform, in order to offset the waveform with the alignment value
	USHORT loadedShiftValue = static_cast<USHORT>(_shiftArray[static_cast<long>(pImgAcqPty->dwellTime * 5 - 2)] * ALIGNMENT_MULTIPLIER);
	_daqAcqCfg.imageCtrl.alignmentOffset = loadedShiftValue + static_cast<USHORT>(pImgAcqPty->alignmentForField);

	//if pixelY is 1, it means we are doing a linescan. We are going to do this oneway line scan
	//in 2way and discard the backward line in postprocessing, the scan should have the pockels turned off (blocking light)
	ret = BuildGalvoWaveforms(&scan_info, &scanLine, dac_rate, &galvo_x_control, &galvo_y_control, pImgAcqPty);
	BuildPockelsControlWaveforms(&scan_info, &scanLine, dac_rate, &pImgAcqPty->pockelPty, oneWayLineScan, pImgAcqPty);

	//BuildPockelsControlWaveform(&scan_info, &scanLine, dac_rate, &_imgAcqPty.pockelPty);

	if (pImgAcqPty->clockSource == EXTERNAL_CLOCK)
	{
		_daqAcqCfg.imageCtrl.threePhotonMode = pImgAcqPty->threePhotonModeEnable;
		_daqAcqCfg.coherentSamplingCtrl.phaseIncrementMode = 2; //0: disable 1:incremental mode 2: static offset
		_daqAcqCfg.coherentSamplingCtrl.phaseOffset = static_cast<USHORT>(pImgAcqPty->laserCoherentSamplingPhase * 8.0 * 16.0 / 100.0);
		_daqAcqCfg.streamingCtrl.channel_multiplexing_enabled = FALSE;
		_daqAcqCfg.streamingCtrl.fir_filter_enabled = FALSE;
		if (pImgAcqPty->threePhotonModeEnable)
		{
			_daqAcqCfg.imageCtrl.threePhotonPhaseAlignment = pImgAcqPty->threePhotonModeAlignmentPhase;
		}
	}

	UCHAR* waveform = new UCHAR[DAC_FIFO_DEPTH * sizeof(USHORT)];
	memset(waveform,0,DAC_FIFO_DEPTH * sizeof(USHORT));

	for (int i = 0; i < DAC_CHANNEL_COUNT; i++)
	{
		ULONG64 startAddress = 0;
		ULONG64 size = 0;
		if (_daqAcqCfg.dacCtrl[i].waveform_buffer_length == 0) // in other words, channels not utilized in BuildWaveforms() earlier
		{
			IMAGING_BUFFER_STRUCT waveform_buffer;
			waveform_buffer.buffer = waveform;
			waveform_buffer.length = DAC_FIFO_DEPTH * sizeof(USHORT);
			waveform_buffer.channel = i;
			waveform_buffer.offset = 0;
			LoadDACMemorySettings(waveform_buffer); // Load Waveform samples into DDR3 (0x7000.0000 and above)

			gPtrMemoryPool->GetMemoryPropertyByChannel(waveform_buffer.channel,_daqAcqCfg.dacCtrl[waveform_buffer.channel].waveform_buffer_start_address,_daqAcqCfg.dacCtrl[waveform_buffer.channel].waveform_buffer_length);			
			_daqAcqCfg.dacCtrl[waveform_buffer.channel].park_val = 0;
			_daqAcqCfg.dacCtrl[waveform_buffer.channel].offset_val = 0;
			_daqAcqCfg.dacCtrl[waveform_buffer.channel].update_rate = DAC_MIN_UPDATERATE;
			_daqAcqCfg.dacCtrl[waveform_buffer.channel].output_port = waveform_buffer.channel;
		}
	}
	SAFE_DELETE_ARRAY(waveform);   

	ThordaqErrChk (L"ThorDAQAPIdFLIMSetImagingConfiguration", retVal = ThorDAQAPIdFLIMSetImagingConfiguration(_DAQDeviceIndex,_daqAcqCfg));
#ifdef DEBUG
	printf("ThorDAQAPIdFLIMSetImagingConfiguration");
#endif // DEBUG

	if ( retVal != STATUS_SUCCESSFUL)
	{
		//printf("Setup Packet Generator failed, Max Packet Size (%ld) is too large\n", pDgTzrParams.bufferSize);	//MaxPacketSize
		ret = FALSE;
		return ret;
	}
	else  //Set up Buffer
	{
		std::bitset<sizeof(size_t)*CHAR_BIT> channel_bitset(pImgAcqPty->channel);

		if (SetupFrameBuffer(static_cast<long>(channel_bitset.count()), pImgAcqPty) != STATUS_SUCCESSFUL)
		{
			ret = FALSE;
			return ret;
		}
	}

	return ret;
}

/**********************************************************************************************//**
 * @fn	long CdFLIMGG::SetupFrameBuffer()
 *
 * @brief	Set up Frame Buffer.
 * @return	A long.
 **************************************************************************************************/
long CdFLIMGG::SetupFrameBuffer(int channel_count, ImgAcqPty* pImgAcqPty)
{
	//BUFFER #1: 1 second frame buffer (frame rate frames for all channels):
	if(_pFlimBuffer.size() > 0)
	{
		for(int i=0;i<_pFlimBuffer.size();i++)
		{
			if (_pFlimBuffer.at(i) != NULL)
			{
				SAFE_DELETE_PTR(_pFlimBuffer.at(i));
				_pFlimBuffer.at(i) = NULL;
			}
		}
		_pFlimBuffer.clear();				
	}

	//BUFFER #1: 1 second frame buffer (frame rate frames for all channels):
	if(_pRawDataBuffer.size() > 0)
	{
		for(int i=0;i<_pRawDataBuffer.size();i++)
		{
			if (_pRawDataBuffer.at(i) != NULL)
			{
				VirtualFree(_pRawDataBuffer.at(i), 0, MEM_RELEASE);
				_pRawDataBuffer.at(i) = NULL;
			}
		}
		_pRawDataBuffer.clear();				
	}
	
	ULONG32 AllocSize = _daqAcqCfg.imageCtrl.dataHSize*_daqAcqCfg.imageCtrl.imgVSize*_daqAcqCfg.imageCtrl.frameNumPerSec;//_daqAcqCfg.imageCtrl.imgHSize*_daqAcqCfg.imageCtrl.imgVSize*_daqAcqCfg.imageCtrl.frameNumPerSec*2; // Buffer size
	AllocSize = SIZE_T((AllocSize + 1023) & -1024);
	if ((AllocSize == 0) || (AllocSize < 0))
	{
		printf("Invalid Buffer Allocation Size = %d\n", AllocSize);
		return FALSE;
	}
	for(int i = 0; i < static_cast<int>(MAX_CHANNEL_COUNT); i++)
	{
		FlimBuffer* ptrFlimBuf =  new FlimBuffer();
		ptrFlimBuf->setupBuffer(pImgAcqPty->pixelX, pImgAcqPty->pixelY);
		_pFlimBuffer.push_back(ptrFlimBuf);
		if (_pFlimBuffer.at(i)== NULL)
		{
			printf("Buffer malloc failed at channel %d, Size = %d\n", i, AllocSize);
			return FALSE;
		}
	}
	SIZE_T stAllocSize = AllocSize;
	for(int i = 0; i < static_cast<int>(MAX_CHANNEL_COUNT); i++)
	{
		UCHAR* ptr = (UCHAR *) VirtualAlloc (NULL, stAllocSize, MEM_COMMIT, PAGE_READWRITE);
		_pRawDataBuffer.push_back(ptr);
		if (_pRawDataBuffer.at(i)== NULL)
		{
			printf("Buffer malloc failed at channel %d, Size = %zd\n", i, stAllocSize);
			return FALSE;
		}
	}

	//BUFFER #2: history buffer for average (1 frame for all channels):

	SAFE_DELETE_ARRAY(_pHistoryFlimBuf);

	_pHistoryFlimBuf =  new FlimBuffer[MAX_CHANNEL_COUNT];

	for(int i = 0; i < static_cast<int>(MAX_CHANNEL_COUNT); i++)
	{
		(_pHistoryFlimBuf + i)->setupBuffer(pImgAcqPty->pixelX, pImgAcqPty->pixelY);
		if ((_pHistoryFlimBuf + i)== NULL)
		{
			printf("Buffer allocation FlimBuffer failed at channel %d, Size = %d\n", i, AllocSize);
			return FALSE;
		}
	}

	if(_pHistoryFlimBuf == NULL)
	{	
		return FALSE;	
	}

	//BUFFER #3: temporary buffer to extract the data to, from the circular buffer:
	SAFE_DELETE_ARRAY(_pTempCopyFlimBuf);
	_pTempCopyFlimBuf =  new FlimBuffer[MAX_CHANNEL_COUNT];

	for(int i = 0; i < static_cast<int>(MAX_CHANNEL_COUNT); i++)
	{
		(_pTempCopyFlimBuf + i)->setupBuffer(pImgAcqPty->pixelX, pImgAcqPty->pixelY);
		if ((_pTempCopyFlimBuf + i)== NULL)
		{
			printf("Buffer allocation FlimBuffer failed at channel %d, Size = %d\n", i, AllocSize);
			return FALSE;
		}
	}

	//BUFFER #4: circular buffer for read (by user) and write (by camera):
	//int channelCount = CountChannelBits(_imgAcqPty.channel); // Do later
	size_t bufferCount = (size_t)pImgAcqPty->dmaBufferCount * MAX_CHANNEL_COUNT * _daqAcqCfg.imageCtrl.frameNumPerTransfer;
	SAFE_DELETE_PTR(_pFrmBuffer);
	_pFrmBuffer = new FrameCirBuffer(pImgAcqPty->pixelX,pImgAcqPty->pixelY, MAX_CHANNEL_COUNT, bufferCount);
	return STATUS_SUCCESSFUL;
}


/**********************************************************************************************//**
 * @fn	long CdFLIMGG::StartAcquisition(char * pDataBuffer)
 *
 * @brief	Start  Acquisition.
 * @param [in,out]	pDataBuffer	  	Identifier of the Data buffer.
 * @return	A long.
 **************************************************************************************************/
long CdFLIMGG::StartAcquisition(char * pDataBuffer)
{
	long error = 0, retVal = 0;
//	StringCbPrintfW(message,MSG_SIZE,L"ThorLSMCam::StartAcquisition");
//	LogMessage(message,VERBOSE_EVENT);

	try
	{
		//do not capture data if you are in the centering scan mode
		if(_imgAcqPty.scanMode == CENTER_SCAN_MODE)
		{
			return TRUE;
		}
		switch(_imgAcqPty.triggerMode)
		{
		case ICamera::SW_FREE_RUN_MODE: // Free Run
		case ICamera::SW_MULTI_FRAME:
		case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
			{
				if (WaitForSingleObject(_hThreadStopped, 0) == WAIT_OBJECT_0)
				{
					//make sure to reload the waveform for the first call of the thread.
					//This ensures the frame trigger is restarted for output
					ThordaqErrChk (L"ThorDAQAPIStartAcquisition", retVal = ThorDAQAPIStartAcquisition(_DAQDeviceIndex));
					if(STATUS_SUCCESSFUL != retVal)
					{ 
						return FALSE; 
					}
					DWORD ThreadID;
					ResetEvent(_hStopAcquisition);

					SAFE_DELETE_HANDLE(_hAcquisitionThread);//close thread here since thread stop was signaled

					_hAcquisitionThread =CaptureCreateThread(ThreadID);
					return TRUE;
				}
			}
			break;

		case ICamera::SW_SINGLE_FRAME:
			{
				//for single frame capture force the previous thread to stop and add an additional wait to ensure it completes.
				//if the previous thread does not close generate an error.
				//Stop and cancel previous scan
				SetEvent(_hStopAcquisition);

				if (WaitForSingleObject(_hThreadStopped, 1000) == WAIT_OBJECT_0)
				{
					ThordaqErrChk (L"ThorDAQAPIStartAcquisition", retVal = ThorDAQAPIStartAcquisition(_DAQDeviceIndex));
					if(STATUS_SUCCESSFUL != retVal)
					{ 
						return FALSE; 
					}
					DWORD ThreadID;
					ResetEvent(_hStopAcquisition);

					if (_hAcquisitionThread != NULL)
					{
						CloseHandle(_hAcquisitionThread);
						_hAcquisitionThread = NULL;
					}

					_hAcquisitionThread =CaptureCreateThread(ThreadID);
					return TRUE;
				}
				else
				{
//					StringCbPrintfW(message,MSG_SIZE,L"StartAcquisition failed. Could not start a thread while thre previous thread is active");
//					LogMessage(message,ERROR_EVENT);
					return FALSE;
				}

			}
			break;

		case ICamera::HW_SINGLE_FRAME:
			{
				//for single frame capture force the previous thread to stop and add an additional wait to ensure it completes.
				//if the previous thread does not close generate an error.
				//Stop and cancel previous scan
				//SetEvent(_hStopAcquisition);

				//if (WaitForSingleObject(_hThreadStopped, 1000) == WAIT_OBJECT_0)
				//{
				//	//make sure to reload the waveform for the first call of the thread.
				//	//This ensures the frame trigger is restarted for output
				//	if(0 != ThorDAQAPIStartAcquisition(_DAQDeviceIndex))
				//	{ return FALSE; }
				//	IDAQLoadAnalogWaveform();
				//	IDAQSetFrameTriggerInput();
				//	IDAQSetFrameTriggerOut();

				//	DWORD ThreadID;
				//	ResetEvent(_hStopAcquisition);

				//	if (_hThread != NULL)
				//	{
				//		CloseHandle(_hThread);
				//		_hThread = NULL;
				//	}

				//	long triggerWaitTimeoutMilliseconds = _triggerWaitTimeout * 1000;
				//	//wait for the hardware trigger to come
				//	if(WaitForSingleObject(_hHardwareTriggerInEvent, triggerWaitTimeoutMilliseconds) == WAIT_OBJECT_0)
				//	{
				//		_hThread =CaptureCreateThread(ThreadID);
				//		return TRUE;
				//	}
				//	else
				//	{
				//		StringCbPrintfW(message,MSG_SIZE,L"External trigger is timed out after %d seconds.",_triggerWaitTimeout);
				//		MessageBox(NULL,message,L"Trigger Error", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
				//		StringCbPrintfW(message,MSG_SIZE,L"StartAcquisition Hardware trigger timeout");
				//		LogMessage(message,ERROR_EVENT);
				//		return FALSE;
				//	}
				//}
				//else
				//{
				//	StringCbPrintfW(message,MSG_SIZE,L"StartAcquisition failed. Could not start a thread while thre previous thread is active");
				//	LogMessage(message,ERROR_EVENT);
				//	return FALSE;
				//}

			}
			break;
		}
	}
	catch(...)
	{
//		StringCbPrintfW(message,MSG_SIZE,L"StartAcquisition failed.");
//		LogMessage(message,ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

/**********************************************************************************************//**
 * @fn	HANDLE CdFLIMGG::CaptureCreateThread(DWORD &threadID)
 *
 * @brief	Create  Acquisition Thread.
 * @param 	threadID	  	Acquisition Thread ID.
 * @return	Thread Handle.
 **************************************************************************************************/
HANDLE CdFLIMGG::CaptureCreateThread(DWORD &threadID)
{
	//DigiParams *dParamsOut = new DigiParams();
	//*dParamsOut = _digiParams;
	_index_of_last_written_frame = -1;
	_index_of_last_read_frame = -1;
	ResetEvent(_hThreadStopped);
	HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &(CdFLIMGG::StartFrameAcqProc), (void *) this, 0, &threadID);
	if( handle != 0 ) SetThreadPriority(handle, THREAD_PRIORITY_ABOVE_NORMAL);
	return handle;
}

/**********************************************************************************************//**
 * @fn	UINT CdFLIMGG::StartFrameAcqProc(LPVOID instance)
 *
 * @brief	Start  Acquisition Thread.
 * @param 	instance	  	Acquisition Thread instance.
 * @return	A uint.
 **************************************************************************************************/
UINT CdFLIMGG::StartFrameAcqProc(LPVOID instance)
{
	int				frameWritten = 0;
	THORDAQ_STATUS	status = STATUS_SUCCESSFUL;
	bool            bAcqStatusOK = true;
	long long		targetFrames = static_cast<long long>(_daqAcqCfg.imageCtrl.frameCnt);// do later
	int				frameSize = _imgAcqPty_Pre.pixelX * _daqAcqCfg.imageCtrl.imgVSize;
	int				frameSizeByte = _bufferHSize * _daqAcqCfg.imageCtrl.imgVSize;//_daqAcqCfg.imageCtrl.imgHSize * _daqAcqCfg.imageCtrl.imgVSize;
	ULONG			TransferSize = static_cast<ULONG>(frameSizeByte * _daqAcqCfg.imageCtrl.frameNumPerTransfer);
	UINT32          size;
	long			chFrameNum[4] = {0, 0, 0, 0}; 
	int				targetFrmNum = 0;
	double          regular_timeout = 1.0 / _frameRate * 1000 * 10 * _daqAcqCfg.imageCtrl.frameNumPerTransfer * 2;
	double			hardware_trigger_timeout = (double)_triggerWaitTimeout * 1000;
	double			timeout = regular_timeout;
	BOOL			is_hardware_captured = FALSE;
	BOOL			hardware_timeout_enable = (_daqAcqCfg.imageCtrl.triggerMode == HARDWARE_TRIGGER_MODE)? TRUE : FALSE;
	ULONG32         bank = 0;
	long long       frame_left = targetFrames - 1; // 0 based
	long			error = 0;
	BOOL isPartialData = FALSE;
	void* RawWin10VirtualImageBuffers[4];
	_acquisitionRunning = true;

	long progressiveScan = FALSE;  // REDESIGN NEEDED to make progressive scan work with dFLIM
	ThorDAQAPIProgressiveScan(_DAQDeviceIndex, progressiveScan);

	int avgIndex = 0;
	unsigned int frameIndex = 0;

	// setup the array to pass to low-level DLL for Raw DDR3 buffer copy
	for (int n = 0; n < 4; n++)
	{
		RawWin10VirtualImageBuffers[n] = _pRawDataBuffer.at(n);
	}
	// start the DDR3 image acq loop which exits on user application (stop acquisition) command
	do
	{	
		frame_left = targetFrames - 1 - _index_of_last_written_frame;
		if (frame_left >= _daqAcqCfg.imageCtrl.frameNumPerTransfer)
		{	
			BOOL isLastTransfer = frame_left <= _daqAcqCfg.imageCtrl.frameNumPerTransfer;
			ULONG framesToTransfer = isLastTransfer ? (ULONG)frame_left : _daqAcqCfg.imageCtrl.frameNumPerTransfer;

			timeout = (hardware_timeout_enable && !is_hardware_captured) ? hardware_trigger_timeout : regular_timeout;
			size = TransferSize;

			///////////////////////////////
			// Legacy dFLIM was designed with single channel read call; ThorDAQ/NWL reads all channels in one call, 
			// (i.e. the entire bank on FPGA's USER INT) irrespective of which chan(s) TILS is processing, so we do that here
			// read entire DDR3 bank (bank logic in lower level DLL)
			// ThorDAQAPIReadFrames() blocks until IRQ received or a low level task timeout happens
			ThordaqErrChk(L"ThorDAQAPIReadFrames", status = ThorDAQAPIReadFrames(_DAQDeviceIndex, &size, 0, &RawWin10VirtualImageBuffers[0], timeout, framesToTransfer, isLastTransfer, isPartialData));
			// TEST!
			//if (status != STATUS_SUCCESSFUL)  DebugBreak();
///////////////////////////////
			for (int chan = 0; chan < 4; chan++) // (check the Low Level DLL status for entire bank read)
			{
				if ((_daqAcqCfg.imageCtrl.channel & (0x0001 << chan))) // channel is enabled
				{
					if(STATUS_SUCCESSFUL == status && size > 0) // read buffer successfully
					{	
						//Do process buffer until target frame number:
						chFrameNum[chan] = static_cast<long>(size / frameSizeByte);
						if (hardware_timeout_enable) is_hardware_captured = TRUE;
					}
					else
					{
						bAcqStatusOK = false;
						if (hardware_timeout_enable) // cannot receive frame hardware trigger
						{
							StringCbPrintfW(message,MSG_SIZE,L"External trigger is timed out after %d seconds.", _triggerWaitTimeout);
							MessageBox(NULL,message,L"Trigger Error", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
						}
						SetEvent(_hTriggerTimeout);
						break;
					}	
				}
			}
		}else // remaining buffer is smaller than DMA buffer, need to read DRAM directly
		{
			ULONG32 frame_count = 0;
			LONG scan_completed = FALSE;
			for (int i = 0; i < 10; i++)
			{
				DWORD regular_timeout = static_cast<DWORD>( 1.0 / _frameRate * 1000 * frame_left * 2);
				Sleep(regular_timeout);
				ThordaqErrChk (L"ThorDAQAPIGetTotalFrameCount", status = ThorDAQAPIGetTotalFrameCount(_DAQDeviceIndex,frame_count));
				if (STATUS_SUCCESSFUL == status && (frame_count == targetFrames))
				{
					scan_completed = TRUE;
					for (int i = 0; i < MAX_CHANNEL_COUNT; i++)
					{
						if ((_daqAcqCfg.imageCtrl.channel & (0x0001 << i)))
						{
							size = static_cast<ULONG>(frameSizeByte * frame_left);
							ULONG64 CardOffset = (UINT64)i * DDR3_SINGLEIMAGE_CHANNEL_CAP + (DDR3_IMAGE_2ndPINGPONGBUFFstart * (UINT64)bank);
//////////////////////////////////
// REDESIGN, NWL kernel changes ThordaqPacketReadBuffer() - read DDR3 directly
//							ThordaqErrChk (L"ThordaqPacketReadBuffer", status = ThordaqPacketReadBuffer(_DAQDeviceIndex, CardOffset, &size, _pRawDataBuffer.at(i), 0xffffffff));
							ThordaqErrChk (L"ThorDAQAPIReadDDR3", status = ThorDAQAPIReadDDR3(_DAQDeviceIndex, CardOffset, _pRawDataBuffer.at(i), &size));

//////////////////////
							if(STATUS_SUCCESSFUL == status && size > 0)
							{	
								//Do process buffer until target frame number:
								chFrameNum[i] = static_cast<long>(size / frameSizeByte);
							}else
							{
								SetEvent(_hTriggerTimeout);
								bAcqStatusOK = false;
								break;
							}
						}
					}
				}
				if (scan_completed == TRUE) break;
			}
			if (scan_completed == FALSE)
			{
				SetEvent(_hTriggerTimeout);
				bAcqStatusOK = false;
			}
		}				

		if((WaitForSingleObject(_hStopAcquisition, 0) == WAIT_OBJECT_0))
		{
			break;
		}
		if (status == STATUS_SUCCESSFUL && bAcqStatusOK)
		{
			bank ^= 1; //switch DRAM bank
			hardware_timeout_enable = FALSE; // only used for first frame
			ResetEvent(_hTriggerTimeout);
			// Done copy channels data  Write to the buffer
			targetFrmNum = 0;
			for (int i = 0; i < MAX_CHANNEL_COUNT; i++)
			{
				if ((_daqAcqCfg.imageCtrl.channel & (0x0001 << i)) != 0x0000)
				{
					long lStatus;
					//_pDataProcess->ProcessBuffer((USHORT*)(_pBuffer.at(i)),i,TransferSize / 2);
					_pFlimBuffer.at(i)->arrival_time_vector.clear();
					lStatus = _pDataProcess->deSerialize(_pRawDataBuffer.at(i), frameSizeByte, _imgAcqPty.pixelY, _bufferHSize, _pFlimBuffer.at(i));
					if (lStatus == 0) bAcqStatusOK = false; // DEBUG - exit on bogus line
						
					targetFrmNum  = max(targetFrmNum, (int)chFrameNum[i]);
				}
				if((WaitForSingleObject(_hStopAcquisition, 0) == WAIT_OBJECT_0))
				{
					break;
				}
			}
			//_index_of_last_written_frame += targetFrmNum; //do later
			size = targetFrmNum * frameSizeByte;			
			
			//string fileName = "DFLIM_RawData" + std::to_string(fileIndex) + ".bin";

			//ofstream my_file1(fileName, ios_base::binary);
			//my_file1.write((char*)(_pRawDataBuffer.at(2)), frameSizeByte);
			//my_file1.close();

			//++fileIndex;

			//if (32 < fileIndex)
			//{
			//	fileIndex = 0;
			//}

			////only save data for the first image
			//if ((_index_of_last_written_frame + 1) == 0)
			//{
			//	ofstream my_file1("DFLIM_RawData.bin", ios_base::binary);
			//	my_file1.write((char*)(_pRawDataBuffer.at(2)), frameSizeByte);
			//	my_file1.close();

			//	ofstream my_file2("DFLIM_arrival_time_sum_buffer.bin", ios_base::binary);
			//	my_file2.write((char*)(_pBuffer.at(2)->arrival_time_sum_buffer), _pBuffer.at(2)->arrivalTimeSumBufferSizeBytes);
			//	my_file2.close();

			//	ofstream my_file3("DFLIM_histogram_raw_buffer.bin", ios_base::binary);
			//	my_file3.write((char*)(_pBuffer.at(2)->histogram_raw_buffer), _pBuffer.at(2)->histogramRawBufferSizeBytes);
			//	my_file3.close();

			//	ofstream my_file4("DFLIM_histogram_buffer.bin", ios_base::binary);
			//	my_file4.write((char*)(_pBuffer.at(2)->histogram_buffer), _pBuffer.at(2)->histogramBufferSizeBytes);
			//	my_file4.close();

			//	ofstream my_file5("DFLIM_single_photon_buffer.bin", ios_base::binary);
			//	my_file5.write((char*)(_pBuffer.at(2)->single_photon_buffer), _pBuffer.at(2)->singlePhotonBufferSizeBytes);
			//	my_file5.close();

			//	ofstream my_file6("DFLIM_photon_num_buffer.bin", ios_base::binary);
			//	my_file6.write((char*)(_pBuffer.at(2)->photon_num_buffer), _pBuffer.at(2)->photonNumBufferSizeBytes);
			//	my_file6.close();
			//}
			++frameIndex;
			if(1 == HandleFrameBuf(TRUE, TIMEOUT_MS)) // 
			{
				if(1 < _imgAcqPty.averageNum && _imgAcqPty.averageMode == FRM_CUMULATIVE_MOVING)
				{
					double avgNumC = min(static_cast<unsigned int>(_imgAcqPty.averageNum),frameIndex);

					double factor1 = 1/avgNumC;
					double factor2 = (avgNumC - 1)/avgNumC;					

					//combine all average frames into 1:
					if((WaitForSingleObject(_hStopAcquisition, 0) == WAIT_OBJECT_0))
					{
						break;
					}
					for(int chID = 0; chID < MAX_CHANNEL_COUNT; ++chID)
					{						
						if ((_imgAcqPty.channel & (0x0001 << chID)) != 0x0000)
						{
							USHORT *avgIntensityPtr = NULL; //Identifier pointer for history intensity buffer
							USHORT *newIntensityPtr = NULL; //Identifier pointer for captured intensity buffer
							USHORT *sumSinglePhotonPtr = NULL; //Identifier pointer for history SinglePhoton buffer
							USHORT *newSinglePhotonPtr = NULL; //Identifier pointer for captured SinglePhoton buffer
							ULONG32 *sumArrivalTimeSumPtr = NULL; //Identifier pointer for history ArrivalTimeSum buffer
							ULONG32 *newArrivalTimeSumPtr = NULL; //Identifier pointer for captured ArrivalTimeSum buffer

							avgIntensityPtr = (_pHistoryFlimBuf +chID)->photon_num_buffer;
							newIntensityPtr = _pFlimBuffer.at(chID)->photon_num_buffer;
							sumSinglePhotonPtr = (_pHistoryFlimBuf +chID)->single_photon_buffer;
							newSinglePhotonPtr = _pFlimBuffer.at(chID)->single_photon_buffer;
							sumArrivalTimeSumPtr = (_pHistoryFlimBuf +chID)->arrival_time_sum_buffer;
							newArrivalTimeSumPtr = _pFlimBuffer.at(chID)->arrival_time_sum_buffer;

							for(int p = 0; p < frameSize; p++)					
							{
								if (1 == frameIndex)
								{
									*(avgIntensityPtr + p) = (*(newIntensityPtr + p));
								}
								else
								{
									*(avgIntensityPtr + p) = static_cast<USHORT>(ceil((*(avgIntensityPtr + p)) * factor2 + (*(newIntensityPtr + p)) * factor1)- 0.5);
								}

								//[BB]20200121 Use this other formula if needed, provided by Gary Yellen.
								//TODO: remove if unnecessary after gary has tested old formula
								//if (1 == frameIndex)
								//{
								//	*(avgIntensityPtr + p) = (*(newIntensityPtr + p));
								//}
								//else
								//{
								//	*(avgIntensityPtr + p) = static_cast<USHORT>(ceil(((*(avgIntensityPtr + p)) * (avgNumC - 1)/avgNumC + (*(newIntensityPtr + p)) /avgNumC)- 0.5));
								//}

								if (frameIndex >=  static_cast<unsigned int>(_imgAcqPty.averageNum))
								{
									*(sumSinglePhotonPtr + p) += static_cast<USHORT>(ceil(((double)(*(newSinglePhotonPtr + p)) -  (*(sumSinglePhotonPtr + p))/_imgAcqPty.averageNum) - 0.5));
									*(sumArrivalTimeSumPtr + p) += static_cast<ULONG32>(ceil(((*(newArrivalTimeSumPtr + p)) -  (*(sumArrivalTimeSumPtr + p))/_imgAcqPty.averageNum) -0.5));
								}
								else
								{
									*(sumSinglePhotonPtr + p) += *(newSinglePhotonPtr + p);
									*(sumArrivalTimeSumPtr + p) += *(newArrivalTimeSumPtr + p);
								}
							}

							ULONG32* sumPtrH = (_pHistoryFlimBuf +chID)->histogram_buffer; //Identifier pointer for history histogram buffer
							ULONG32* newPtrH = _pFlimBuffer.at(chID)->histogram_buffer; //Identifier pointer for captured histogram buffer
							const int BINS = 256;
							for (int b = 0; b < BINS; ++b)
							{
								//data between 241 and 253 should not be summed up
								if (b >= 241 && b <= 253)
								{
									continue;
								}

								if (frameIndex >=  static_cast<unsigned int>(_imgAcqPty.averageNum))
								{
									*(sumPtrH + b) += static_cast<ULONG32>(ceil(((*(newPtrH + b)) - (*(sumPtrH + b))/((double)_imgAcqPty.averageNum)) - 0.5));
								}
								else
								{
									*(sumPtrH + b) += *(newPtrH + b);
								}
							}
						}						
						if((WaitForSingleObject(_hStopAcquisition, 0) == WAIT_OBJECT_0))
						{
							break;
						}
					}
					//write history buffer to circular buffer:
					frameWritten = static_cast<int>((_pFrmBuffer->WriteFrames(_pHistoryFlimBuf , MAX_CHANNEL_COUNT))/MAX_CHANNEL_COUNT);		
				}
				else
				{
					//no average, write buffer to circular buffer:
					int frmNum = 0;
					for(int j = 0; j < targetFrmNum; ++j)
					{		
						for(int chID = 0; chID < MAX_CHANNEL_COUNT; chID++)
						{
							frmNum += static_cast<int>(_pFrmBuffer->WriteFrames(_pFlimBuffer.at(chID), 1));
						}
					}
					frameWritten = static_cast<int>(frmNum / MAX_CHANNEL_COUNT);
				}

				HandleFrameBuf(FALSE, TIMEOUT_MS);
				_index_of_last_written_frame += frameWritten;
			}
		}
	} while ((_index_of_last_written_frame < (targetFrames - 1)) && (WaitForSingleObject(_hStopAcquisition, 0) != WAIT_OBJECT_0) && bAcqStatusOK);
	_acquisitionRunning = false;

	//if Diagnostic mode
	if (_daqAcqCfg.imageCtrl.acquisitionMode == 1)
	{
		ofstream my_file1("DFLIM_RawData_Chan0.bin", ios_base::binary);
		my_file1.write((char*)(_pRawDataBuffer.at(0)), frameSizeByte);
		my_file1.close();
	}
	//ofstream my_file2("DFLIM_arrival_time_sum_buffer.bin", ios_base::binary);
	//my_file2.write((char*)(_pBuffer.at(2)->arrival_time_sum_buffer), _pBuffer.at(2)->arrivalTimeSumBufferSizeBytes);
	//my_file2.close();

	//ofstream my_file3("DFLIM_histogram_raw_buffer.bin", ios_base::binary);
	//my_file3.write((char*)(_pBuffer.at(2)->histogram_raw_buffer), _pBuffer.at(2)->histogramRawBufferSizeBytes);
	//my_file3.close();

	//ofstream my_file4("DFLIM_histogram_buffer.bin", ios_base::binary);
	//my_file4.write((char*)(_pBuffer.at(2)->histogram_buffer), _pBuffer.at(2)->histogramBufferSizeBytes);
	//my_file4.close();

	//ofstream my_file5("DFLIM_single_photon_buffer.bin", ios_base::binary);
	//my_file5.write((char*)(_pBuffer.at(2)->single_photon_buffer), _pBuffer.at(2)->singlePhotonBufferSizeBytes);
	//my_file5.close();

	//ofstream my_file6("DFLIM_photon_num_buffer.bin", ios_base::binary);
	//my_file6.write((char*)(_pBuffer.at(2)->photon_num_buffer), _pBuffer.at(2)->photonNumBufferSizeBytes);
	//my_file6.close();

	ULONG32 clockStatus = 0;
	ThordaqErrChk (L"ThorDAQAPIGetExternClockStatus", status = ThorDAQAPIGetExternClockStatus(_DAQDeviceIndex, clockStatus));
	if (!bAcqStatusOK && _daqAcqCfg.imageCtrl.threePhotonMode == TRUE 
		&& status == THORDAQ_STATUS::STATUS_SUCCESSFUL 
		&& clockStatus == 0)
	{
		MessageBox(NULL,L"Laser SYNC Error",L"Error", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
	}

	//done capture:
	//delete pParams;
	((CdFLIMGG *)instance)->StopDaqBrd();
	//ThordaqErrChk (L"ShutdownPacketMode", status = ShutdownPacketMode(_DAQDeviceIndex););
	SetEvent(_hThreadStopped);
	return 0;
}

void CdFLIMGG::StopDaqBrd()
{
	long error = 0, retVal = 0;;
	//ThordaqErrChk (L"ThorDAQAPIStopAcquisition", retVal = ThorDAQAPIStopAcquisition(_DAQDeviceIndex));
	ThorDAQAPIStopAcquisition(_DAQDeviceIndex);
}

long CdFLIMGG::StatusAcquisition(long &status)
{
	long ret = TRUE;

	//do not capture data if you are in the centering scan mode
	if(_imgAcqPty.scanMode == CENTER_SCAN_MODE)
	{
		status = ICamera::STATUS_READY;
		return ret;
	}

	if(WaitForSingleObject(_hTriggerTimeout, 0) == WAIT_OBJECT_0)
	{
		status = ICamera::STATUS_ERROR;
		//ResetEvent(_hTriggerTimeout);
		return ret;
	}

	if ((ICamera::SW_FREE_RUN_MODE == _imgAcqPty_Pre.triggerMode) && (WAIT_OBJECT_0 == WaitForSingleObject(_hStopAcquisition, 0)))
	{
		status = ICamera::STATUS_READY;
		return FALSE;
	}

	int comparison_index = static_cast<int>(_index_of_last_written_frame);

	if(_index_of_last_read_frame < comparison_index)
	{
		status = ICamera::STATUS_READY;
		printf("_index_of_last_written_frame = %lld; _index_of_last_read_frame = %lld.\n", _index_of_last_written_frame, _index_of_last_read_frame);
		//StringCbPrintfW(message,MSG_SIZE,L"Status now is ready with frame buffer size of %d.\n", _index_of_last_written_frame - _index_of_last_read_frame);
		//LogMessage(message,VERBOSE_EVENT);
		if (comparison_index  - _index_of_last_read_frame > _daqAcqCfg.imageCtrl.frameNumPerSec)
		{
			status = ICamera::STATUS_ERROR;
		}
	}	
	else
	{
		status = ICamera::STATUS_BUSY;
	}

	return ret;
}

long CdFLIMGG::StatusAcquisitionEx(long &status,long &indexOfLastCompletedFrame)
{
	long ret = TRUE;

	//do not capture data if you are in the centering scan mode
	if(_imgAcqPty.scanMode == CENTER_SCAN_MODE)
	{
		status = ICamera::STATUS_READY;
		return ret;
	}

	if (WaitForSingleObject(_hTriggerTimeout, 0) == WAIT_OBJECT_0)
	{
		status = ICamera::STATUS_ERROR;
	}
	else
	{
		status = ICamera::STATUS_BUSY;
	}

	indexOfLastCompletedFrame = static_cast<long>(_index_of_last_written_frame);

	return ret;
}

long CdFLIMGG::CopyAcquisition(char *pDataBuffer, void* frameInfo)
{	
	long ret = TRUE;

	//do not capture data if you are in the centering scan mode
	if(_imgAcqPty_Pre.scanMode == CENTER_SCAN_MODE)
	{
		return ret;
	}

	long channels = 0;
	FrameInfo frameInfoStruct;
	memcpy(&frameInfoStruct, frameInfo, sizeof(FrameInfo));

	int channelCnt  = MAX_CHANNEL_COUNT;
	
	size_t frameNum = 0;
	if(_index_of_last_read_frame == _index_of_last_written_frame &&
		(frameInfoStruct.bufferType == BufferType::DFLIM_ALL || 
		frameInfoStruct.bufferType == BufferType::INTENSITY)
		)
	{
		return FALSE;
	}

	if (ICamera::SW_FREE_RUN_MODE == _imgAcqPty_Pre.triggerMode && _imgAcqPty_Pre.acquistionMode == 1)
	{
		frameInfoStruct.bufferType = BufferType::DFLIM_DIAGNOSTIC;
	}
	else if (ICamera::SW_FREE_RUN_MODE == _imgAcqPty_Pre.triggerMode)
	{
		//if in live mode then copy the all the frame dFLIM data
		frameInfoStruct.bufferType = BufferType::DFLIM_ALL;
	}

	int selectedChannels = CountChannelBits(_imgAcqPty_Pre.channel);
	size_t offset = 0;

	if (frameInfoStruct.bufferType == BufferType::INTENSITY ||
		frameInfoStruct.bufferType == BufferType::DFLIM_DIAGNOSTIC)
	{
		//only extract the new buffer when copying intensity or its all dflim or dflim image
		if(1 == HandleFrameBuf(TRUE,TIMEOUT_MS))
		{
			if( _pFrmBuffer->ReadFrames(_pTempCopyFlimBuf, channelCnt) > 0)
			{	
				_index_of_last_read_frame++;
			}
			HandleFrameBuf(FALSE, TIMEOUT_MS);
		}
		channels = 0;
		for (int i = 0,j = 0; i < MAX_CHANNEL_COUNT; ++i)
		{
			// If save enabled channels only for raw is disabled, pDataBuffer will be the size of all channels (except in the 1 channel case)
			// we need to copy the image buffer in the corresponding space, whitout checking which channels are enabled/disabled.
			if(FALSE == _imgAcqPty_Pre.rawSaveEnabledChannelOnly && 1 != selectedChannels)
			{
				size_t frameSizeByte = (_pTempCopyFlimBuf + i)->photonNumBufferSizeBytes;
				
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{					
					memcpy(pDataBuffer + offset, (_pTempCopyFlimBuf + i)->photon_num_buffer, (_pTempCopyFlimBuf + i)->photonNumBufferSizeBytes);
				}
				offset += frameSizeByte;
				++channels;
			}
			else
			{
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{
					size_t frameSizeByte = (_pTempCopyFlimBuf + i)->photonNumBufferSizeBytes;
					memcpy(pDataBuffer + offset, (_pTempCopyFlimBuf + i)->photon_num_buffer, (_pTempCopyFlimBuf + i)->photonNumBufferSizeBytes);
					offset += frameSizeByte;
					++j;
					++channels;
				}
			}
		}
	}

	if (frameInfoStruct.bufferType == BufferType::DFLIM_ALL ||
		frameInfoStruct.bufferType == BufferType::DFLIM_IMAGE)
	{
		//only extract the new buffer when copying intensity or its all dflim or dflim image
		if(1 == HandleFrameBuf(TRUE,TIMEOUT_MS))
		{
			if( _pFrmBuffer->ReadFrames(_pTempCopyFlimBuf, channelCnt) > 0)
			{	
				_index_of_last_read_frame++;
			}
			HandleFrameBuf(FALSE, TIMEOUT_MS);
		}

		channels = 0;
		for (int i = 0,j = 0; i < MAX_CHANNEL_COUNT; ++i)
		{
			// If save enabled channels only for raw is disabled, pDataBuffer will be the size of all channels (except in the 1 channel case)
			// we need to copy the image buffer in the corresponding space, whitout checking which channels are enabled/disabled.
			if(FALSE == _imgAcqPty_Pre.rawSaveEnabledChannelOnly && 1 != selectedChannels)
			{
				size_t histoSizeByte = (_pTempCopyFlimBuf + i)->histogramBufferSizeBytes;
				
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{					
					memcpy(pDataBuffer + offset, (_pTempCopyFlimBuf + i)->histogram_buffer, (_pTempCopyFlimBuf + i)->histogramBufferSizeBytes);									
				}
				offset += histoSizeByte;
				++channels;
			}
			else
			{
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{
					size_t histoSizeByte = (_pTempCopyFlimBuf + i)->histogramBufferSizeBytes;
					memcpy(pDataBuffer + offset, (_pTempCopyFlimBuf + i)->histogram_buffer, (_pTempCopyFlimBuf + i)->histogramBufferSizeBytes);
					offset += histoSizeByte;
					++j;
					++channels;
				}
			}
		}

		channels = 0;
		for (int i = 0,j = 0; i < MAX_CHANNEL_COUNT; ++i)
		{
			// If save enabled channels only for raw is disabled, pDataBuffer will be the size of all channels (except in the 1 channel case)
			// we need to copy the image buffer in the corresponding space, whitout checking which channels are enabled/disabled.
			if(FALSE == _imgAcqPty_Pre.rawSaveEnabledChannelOnly && 1 != selectedChannels)
			{
				size_t frameSizeByte = (_pTempCopyFlimBuf + i)->photonNumBufferSizeBytes;
				
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{					
					memcpy(pDataBuffer + offset, (_pTempCopyFlimBuf + i)->photon_num_buffer, (_pTempCopyFlimBuf + i)->photonNumBufferSizeBytes);
				}
				offset += frameSizeByte;
				++channels;
			}
			else
			{
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{
					size_t frameSizeByte = (_pTempCopyFlimBuf + i)->photonNumBufferSizeBytes;
					memcpy(pDataBuffer + offset, (_pTempCopyFlimBuf + i)->photon_num_buffer, (_pTempCopyFlimBuf + i)->photonNumBufferSizeBytes);
					offset += frameSizeByte;
					++j;
					++channels;
				}
			}
		}		
	}
	
	if (frameInfoStruct.bufferType == BufferType::DFLIM_HISTOGRAM)
	{
		channels = 0;
		for (int i = 0,j = 0; i < MAX_CHANNEL_COUNT; ++i)
		{
			// If save enabled channels only for raw is disabled, pDataBuffer will be the size of all channels (except in the 1 channel case)
			// we need to copy the image buffer in the corresponding space, whitout checking which channels are enabled/disabled.
			if(FALSE == _imgAcqPty_Pre.rawSaveEnabledChannelOnly && 1 != selectedChannels)
			{
				size_t histoSizeByte = (_pTempCopyFlimBuf + i)->histogramBufferSizeBytes;
				
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{					
					memcpy(pDataBuffer + offset, (_pTempCopyFlimBuf + i)->histogram_buffer, (_pTempCopyFlimBuf + i)->histogramBufferSizeBytes);									
				}
				offset += histoSizeByte;
				++channels;
			}
			else
			{
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{
					size_t histoSizeByte = (_pTempCopyFlimBuf + i)->histogramBufferSizeBytes;
					memcpy(pDataBuffer + offset, (_pTempCopyFlimBuf + i)->histogram_buffer, (_pTempCopyFlimBuf + i)->histogramBufferSizeBytes);
					offset += histoSizeByte;
					++j;
					++channels;
				}
			}
		}
	}

	if (frameInfoStruct.bufferType == BufferType::DFLIM_ALL || 
		frameInfoStruct.bufferType == BufferType::DFLIM_IMAGE || 
		frameInfoStruct.bufferType == BufferType::DFLIM_IMAGE_SINGLE_PHOTON)
	{
		channels = 0;
		for (int i = 0,j = 0; i < MAX_CHANNEL_COUNT; ++i)
		{
			// If save enabled channels only for raw is disabled, pDataBuffer will be the size of all channels (except in the 1 channel case)
			// we need to copy the image buffer in the corresponding space, whitout checking which channels are enabled/disabled.
			if(FALSE == _imgAcqPty_Pre.rawSaveEnabledChannelOnly && 1 != selectedChannels)
			{
				size_t singlePhotonBufferSizeByte = (_pTempCopyFlimBuf + i)->singlePhotonBufferSizeBytes;
			
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{					
					memcpy(pDataBuffer + offset, (_pTempCopyFlimBuf + i)->single_photon_buffer, (_pTempCopyFlimBuf + i)->singlePhotonBufferSizeBytes);								
				}
				offset += singlePhotonBufferSizeByte;
				++channels;
			}
			else
			{
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{
					size_t singlePhotonBufferSizeByte = (_pTempCopyFlimBuf + i)->singlePhotonBufferSizeBytes;
					memcpy(pDataBuffer + offset, (_pTempCopyFlimBuf + i)->single_photon_buffer, (_pTempCopyFlimBuf + i)->singlePhotonBufferSizeBytes);
					offset += singlePhotonBufferSizeByte;
					++j;
					++channels;
				}
			}
		}
	}

	if (frameInfoStruct.bufferType == BufferType::DFLIM_ALL || 
		frameInfoStruct.bufferType == BufferType::DFLIM_IMAGE || 
		frameInfoStruct.bufferType == BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM)
	{
		channels = 0;
		for (int i = 0,j = 0; i < MAX_CHANNEL_COUNT; ++i)
		{
			// If save enabled channels only for raw is disabled, pDataBuffer will be the size of all channels (except in the 1 channel case)
			// we need to copy the image buffer in the corresponding space, whitout checking which channels are enabled/disabled.
			if(FALSE == _imgAcqPty_Pre.rawSaveEnabledChannelOnly && 1 != selectedChannels)
			{
				size_t arrivalTimeNumSizeByte = (_pTempCopyFlimBuf + i)->arrivalTimeSumBufferSizeBytes;
			
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{					
					memcpy(pDataBuffer + offset, (_pTempCopyFlimBuf + i)->arrival_time_sum_buffer, (_pTempCopyFlimBuf + i)->arrivalTimeSumBufferSizeBytes);							
				}
				offset += arrivalTimeNumSizeByte;
				++channels;
			}
			else
			{
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{
					size_t arrivalTimeNumSizeByte = (_pTempCopyFlimBuf + i)->arrivalTimeSumBufferSizeBytes;
					memcpy(pDataBuffer + offset, (_pTempCopyFlimBuf + i)->arrival_time_sum_buffer, (_pTempCopyFlimBuf + i)->arrivalTimeSumBufferSizeBytes);
					offset += arrivalTimeNumSizeByte;
					++j;
					++channels;
				}
			}
		}
	}

	if (frameInfoStruct.bufferType == BufferType::DFLIM_ALL || 
		frameInfoStruct.bufferType == BufferType::DFLIM_PHOTONS)
	{
		channels = 0;
		for (int i = 0,j = 0; i < MAX_CHANNEL_COUNT; ++i)
		{
			if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
			{
				size_t frameSizeByte = (_pTempCopyFlimBuf + i)->arrival_time_vector.size()*sizeof(UCHAR);

				//std::vector<double> v;

				vector<UCHAR> vec = (_pTempCopyFlimBuf + i)->arrival_time_vector;
				UCHAR* array = vec.data();
				//UCHAR* a = &vec[0];
				//std::copy(vec.begin(), vec.end(), pDataBuffer + offset);
				memcpy(pDataBuffer + offset, array, frameSizeByte);
				offset += frameSizeByte;
				++j;
				++channels;
			}
			else if (FALSE == _imgAcqPty_Pre.rawSaveEnabledChannelOnly && 1 != selectedChannels)
			{
				++channels;
			}
		}
	}

	frameInfoStruct.fullFrame = TRUE;

	frameInfoStruct.channels = channels > 1 ? MAX_CHANNEL_COUNT : channels;
	if (channels > 0)
	{
		frameInfoStruct.copySize = offset;
	}

	frameInfoStruct.numberOfPlanes = 1;

	frameInfoStruct.isNewMROIFrame = 1;
	frameInfoStruct.totalScanAreas = 1;
	frameInfoStruct.scanAreaIndex = 0;
	frameInfoStruct.scanAreaID = 0;
	frameInfoStruct.imageWidth = _daqAcqCfg.imageCtrl.imgHSize;
	frameInfoStruct.imageHeight = _daqAcqCfg.imageCtrl.imgVSize;
	frameInfoStruct.isMROI = FALSE;
	frameInfoStruct.fullImageWidth = (long)_daqAcqCfg.imageCtrl.imgHSize;
	frameInfoStruct.fullImageHeight = (long)_daqAcqCfg.imageCtrl.imgVSize;
	frameInfoStruct.topInFullImage = 0;
	frameInfoStruct.leftInFullImage = 0;
	frameInfoStruct.mROIStripeFieldSize = _imgAcqPty_Pre.fieldSize;

	memcpy(frameInfo, &frameInfoStruct, sizeof(FrameInfo));

	return ret;
}

long CdFLIMGG::PostflightAcquisition(char * pDataBuffer)
{
	try
	{
		long error = 0, retVal = 0;
		//do not capture data if you are in the centering scan mode
		if(_imgAcqPty.scanMode == CENTER_SCAN_MODE)
		{
			MoveGalvoToParkPosition(GalvoGalvoX,GalvoGalvoY);
			//IDAQMovePockelsToPowerLevel();
			return TRUE;
		}
		//force the hardware trigger event if the post flight function is called
		SetEvent(_hHardwareTriggerInEvent);
		ResetEvent(_hHardwareTriggerInEvent);
		//IDAQCloseNITasks();	
		ThordaqErrChk (L"ThorDAQAPIAbortRead", retVal = ThorDAQAPIAbortRead(_DAQDeviceIndex));
		SetEvent(_hStopAcquisition);
	
		//park the pockels cell
		//IDAQMovePockelsToParkPosition();
		
		

		//for the trigger output to low
		//IDAQSetFrameTriggerOutLow();

		while(WaitForSingleObject(_hThreadStopped, 10000) != WAIT_OBJECT_0)
		{
			Sleep(10);
		}

		//stop digitizer board:
		//StopDaqBrd();

		MoveGalvoToParkPosition(GalvoGalvoX,GalvoGalvoY);
		MovePockelsToParkPosition(&_imgAcqPty.pockelPty);

	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}
long CdFLIMGG::GetLastErrorMsg(wchar_t * msg, long size)
{
	long ret = TRUE;
	return ret;
}

void CdFLIMGG:: LogMessage(wchar_t *logMsg,long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

long CdFLIMGG::SetParamString(const long paramID, wchar_t * str)
{
	long ret = TRUE;
	return ret;
}
long CdFLIMGG::GetParamString(const long paramID, wchar_t * str, long size)
{
	long ret = FALSE;

	switch(paramID)
	{
	case ICamera::PARAM_DETECTOR_NAME:
		{
			wcscpy_s(str,20, _pDetectorName);	//tempID[20] in SelectHardware
			ret = TRUE;
		}
		break;
	default:
		break;
	}

	return ret;
}

long CdFLIMGG::SetParamBuffer(const long paramID, char * pBuffer, long size)
{
	long ret = TRUE;
	return ret;
}

long CdFLIMGG::GetParamBuffer(const long paramID, char * pBuffer, long size)
{
	long ret = TRUE;

	switch (paramID)
	{
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_0:
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_1:
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_2:
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_3:
		{
			long index = 0;
			switch(paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_0:index = 0;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_1:index = 1;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_2:index = 2;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_3:index = 3;break;
			}

			if(POCKELS_VOLTAGE_STEPS * sizeof(float64) <= size)
			{
				std::memcpy(pBuffer,&_pockelsReadArray[index],POCKELS_VOLTAGE_STEPS * sizeof(float64));
			}
		}
		break;
	default:
		{
			ret = FALSE;
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Parameter (%d) not implemented",paramID);
			LogMessage(_errMsg,ERROR_EVENT);
		}

	}
	return ret;
}

void CdFLIMGG::SetStatusHandle(HANDLE handle)
{

}

long CdFLIMGG::MoveGalvoToParkPosition(int Galvo_X_channel,int Galvo_Y_channel)
{
	long ret = FALSE;
	long error = 0, retVal = 0;
	double xGalvoParkPosition = GALVO_PARK_POSITION;
	double yGalvoParkPosition = -1 * _imgAcqPty.verticalScanDirection * GALVO_PARK_POSITION;

	ThordaqErrChk (L"ThorDAQSetDACParkValue", retVal = ThorDAQSetDACParkValue(_DAQDeviceIndex,Galvo_X_channel, xGalvoParkPosition));
	if (retVal == STATUS_SUCCESSFUL)
	{
		ThordaqErrChk (L"ThorDAQSetDACParkValue", retVal = ThorDAQSetDACParkValue(_DAQDeviceIndex,Galvo_Y_channel, yGalvoParkPosition));
		if (retVal != STATUS_SUCCESSFUL)
		{
			ret = 0;
		}
	}

	return ret;
}

long CdFLIMGG::MoveGalvoToCenter(int Galvo_X_channel,int Galvo_Y_channel,ImgAcqPty* pImgAcqPty)
{
	long ret = TRUE;
	long error = 0, retVal = 0;

	double center_x = pImgAcqPty->offsetX * _field2Theta * _theta2Volts + pImgAcqPty->fineOffsetX;
	double center_y = pImgAcqPty->verticalScanDirection * pImgAcqPty->offsetY * _field2Theta * _theta2Volts + pImgAcqPty->fineOffsetY;

	ThordaqErrChk (L"ThorDAQSetDACParkValue", retVal = ThorDAQSetDACParkValue(_DAQDeviceIndex,Galvo_X_channel,center_x));
	if (retVal == STATUS_SUCCESSFUL)
	{
		ThordaqErrChk (L"ThorDAQSetDACParkValue", retVal = ThorDAQSetDACParkValue(_DAQDeviceIndex,Galvo_Y_channel,center_y));
		if (retVal != STATUS_SUCCESSFUL)
		{
			retVal = FALSE;
		}
	}
	return ret;
}

long CdFLIMGG::MovePockelsToParkPosition(PockelPty* pockelPty)
{
	long ret = 0;
	long error = 0, retVal = 0;

	for (int i = 0; i < MAX_POCKELS_CELL_COUNT; i++)
	{
		if (_pockelsEnable[i] == FALSE)
		{
			continue;
		}

		double pockelsSetVal0 = _pockelsMinVoltage[i] + (_pockelsMaxVoltage[i] - _pockelsMinVoltage[i]) * pockelPty->pockelsPowerLevel[i];
		double park_value = (TRUE == _pockelsParkAtMinimum) ? _pockelsMinVoltage[0] : pockelsSetVal0;
		ThordaqErrChk (L"ThorDAQSetDACParkValue", retVal = ThorDAQSetDACParkValue(_DAQDeviceIndex,Pockel1+i,park_value));
		if (retVal != STATUS_SUCCESSFUL)
		{
			ret = FALSE;
		}
	}

	return ret;
}


long CdFLIMGG::MovePockelsToPowerLevel(long index, PockelPty* pockelPty)
{
	long ret = TRUE;
	long error = 0, retVal = 0;

	double pockelsOnVoltage = _pockelsMinVoltage[index] + (_pockelsMaxVoltage[index] - _pockelsMinVoltage[index]) * pockelPty->pockelsPowerLevel[index];
	ThordaqErrChk (L"ThorDAQSetDACParkValue", retVal = ThorDAQSetDACParkValue(_DAQDeviceIndex,Pockel1+index,pockelsOnVoltage));
	if (retVal != STATUS_SUCCESSFUL)
	{
		ret = FALSE;
	}

	return ret;
}

long CdFLIMGG::FindPockelsMinMax(long index, PockelPty* pockelPty)
{
	int32 retVal = 0, error = 0;

	if(!_pockelsEnable[index])
	{
		return retVal;
	}

	try
	{
		DAQmxErrChk(L"DAQmxCreateTask",DAQmxCreateTask("", &_taskHandleAIPockels[index]));
		DAQmxErrChk(L"DAQmxCreateAIVoltageChan",DAQmxCreateAIVoltageChan(_taskHandleAIPockels[index], _pockelsPowerInputLine[index].c_str(), "",DAQmx_Val_Cfg_Default, 0,10.0,DAQmx_Val_Volts, NULL));

		const float64 VOLTAGE_START = _pockelsScanVoltageStart[index];
		float64 pockelsPos = VOLTAGE_START;
		const float64 VOLTAGE_RANGE = _pockelsScanVoltageStop[index] - _pockelsScanVoltageStart[index];

		for(long i=0; i<POCKELS_VOLTAGE_STEPS; i++)
		{
			ThordaqErrChk (L"ThorDAQSetDACParkValue", retVal = ThorDAQSetDACParkValue(_DAQDeviceIndex,Pockel1+index,pockelsPos));

			Sleep(30);

			int32 numRead;
			DAQmxErrChk(L"DAQmxReadAnalogF64",DAQmxReadAnalogF64(_taskHandleAIPockels[index], 1, 10.0, DAQmx_Val_GroupByChannel, &_pockelsReadArray[index][i],1,&numRead,NULL));
			DAQmxErrChk(L"DAQmxWaitUntilTaskDone", DAQmxWaitUntilTaskDone(_taskHandleAIPockels[index], MAX_TASK_WAIT_TIME));

			Sleep(1);

			pockelsPos += VOLTAGE_RANGE/POCKELS_VOLTAGE_STEPS;
		}

		//move back to the start position after the scan
		pockelsPos = VOLTAGE_START;
		ThordaqErrChk (L"ThorDAQSetDACParkValue", retVal = ThorDAQSetDACParkValue(_DAQDeviceIndex,Pockel1+index,pockelsPos));


		float64 * pSmoothBuffer = new float64[POCKELS_VOLTAGE_STEPS];

		std::memcpy(pSmoothBuffer,&_pockelsReadArray[index],POCKELS_VOLTAGE_STEPS * sizeof(float64));

		//smooth the data and ignore the ends
		const long KERNEL_SIZE = 5;
		const long KERNEL_SKIP = 2;

		for(long n=0; n<5; n++)
		{
			for(long i=KERNEL_SKIP; i<POCKELS_VOLTAGE_STEPS-KERNEL_SKIP; i++)
			{
				float64 sum = 0;

				////////////////////////////////////////////////////////////
				//Average Filter
				//for(long j=-1*(KERNEL_SIZE>>1); j<=(KERNEL_SIZE>>1); j++)
				//{
				//	sum += pSmoothBuffer[i+j];
				//}
				//pSmoothBuffer[i] = sum/KERNEL_SIZE;

				//float64 results[KERNEL_SIZE];

				////////////////////////////////////////////////////////////
				//Median Filter
				//for(long k=0,j=-1*(KERNEL_SIZE>>1); j<=(KERNEL_SIZE>>1); j++,k++)
				//{
				//	results[k] = pSmoothBuffer[i+j];
				//}
				//qsort(results,KERNEL_SIZE,sizeof (float64),cmpfunc);

				//pSmoothBuffer[i] = results[(KERNEL_SIZE>>2) + 1];
				////////////////////////////////////////////////////////////

				////////////////////////////////////////////////////////////
				//Gaussian Filter
				float64 kernel[KERNEL_SIZE] = {.06,.24,.4,.24,.06};

				for(long k=0,j=-1*(KERNEL_SIZE>>1); j<=(KERNEL_SIZE>>1); j++,k++)
				{
					sum += kernel[k] * pSmoothBuffer[i+j];
				}

				pSmoothBuffer[i] = sum;
			}
		}

		double arrayMinVal = 10.0;
		double arrayMaxVal = -10.0;

		//locat the min and max for the dataset
		for(long i=KERNEL_SKIP+1; i<POCKELS_VOLTAGE_STEPS-KERNEL_SKIP-1; i++)
		{
			if(pSmoothBuffer[i] < arrayMinVal)
			{
				arrayMinVal = pSmoothBuffer[i];
			}

			if(pSmoothBuffer[i] > arrayMaxVal)
			{
				arrayMaxVal = pSmoothBuffer[i];
			}
		}


		/************* Method I ********************************/

		//Algorithm to find the min and max values to control the pockels
		//Steps/Description:
		//1. Find the mid value between the max and min overall registered values
		//2. Find the location in the array corresponding to this value
		//3. Find the min location and value for the min point by comparing 
		//the slope until it is smaller than the threshold set in the settings file
		//4. Find the max location and value for the min point by comparing 
		//the slope until it is smaller than the threshold set in the settings file

		long midLoc = -1;
		double midVal = 0.0;

		const float64 PEAK_THRESHOLD_VOLTS = (arrayMaxVal - arrayMinVal)*.001;
		const float64 DIFFERENCE_THRESHOLD_VOLTS = (arrayMaxVal - arrayMinVal)*.30;
		const float64 MAX_MIDVALUE_DIFFERENCE = (arrayMaxVal - arrayMinVal) * 0.01;
		const double MID_VALUE = (arrayMaxVal + arrayMinVal) / 2;

		//find the midPoint location
		for(long i=KERNEL_SKIP+2; i<POCKELS_VOLTAGE_STEPS-KERNEL_SKIP-2; i++)
		{			
			if (MAX_MIDVALUE_DIFFERENCE >  MID_VALUE - pSmoothBuffer[i] )
			{
				midLoc = i;
				midVal = _pockelsReadArray[index][i];
				break;
			}
		}

		long minLoc = -1;
		long maxLoc = -1;
		double minVal = 0.0;
		double maxVal = 0.0;

		const double SLOPE_THRESHOLD = _pockelsVoltageSlopeThreshold;

		if (midLoc > 0)
		{
			//find the minVal and minLoc location
			for (int i = midLoc; i >= KERNEL_SKIP+2; i--)
			{
				double x1 = (double)i - 1;
				double x2 = i;

				double y1 = pSmoothBuffer[i - 1];
				double y2 = pSmoothBuffer[i];

				//calculate the slope
				double m = SLOPE_THRESHOLD;
				if 	(x2 != x1)
				{
					m = (y2 - y1) / (x2- x1);
				}
				if (SLOPE_THRESHOLD > abs(m))
				{
					minLoc = i;
					minVal = _pockelsReadArray[index][i];
					break;
				}
			}

			//find the maxVal and maxLoc location
			for (int i = midLoc; i < POCKELS_VOLTAGE_STEPS-KERNEL_SKIP-2; i++)
			{
				double x1 = (double)i - 1;
				double x2 = i;

				double y1 = pSmoothBuffer[i - 1];
				double y2 = pSmoothBuffer[i];

				//calculate the slope
				double m = SLOPE_THRESHOLD;
				if 	(x2 != x1)
				{
					m = (y2 - y1) / (x2- x1);
				}

				if (SLOPE_THRESHOLD > abs(m))
				{
					maxLoc = i;
					maxVal = _pockelsReadArray[index][i];
					break;
				}
			}
		}
		/*********** End of Method I ***************/


		/************* Method II ********************************/
		//long minLoc = -1;
		//long maxLoc = -1;
		//double minVal = 0;
		//double maxVal = 0;

		//const float64 PEAK_THRESHOLD_VOLTS = (arrayMaxVal - arrayMinVal)*.001;
		//const float64 DIFFERENCE_THRESHOLD_VOLTS = (arrayMaxVal - arrayMinVal)*.30;

		//for(long i=KERNEL_SKIP+2; i<POCKELS_VOLTAGE_STEPS-KERNEL_SKIP-2; i++)
		//{
		//	//find the minimum first
		//	if(-1 == minLoc)
		//	{
		//		//transition point indicating a minimum
		//		if((pSmoothBuffer[i-2] - PEAK_THRESHOLD_VOLTS > pSmoothBuffer[i]) && (pSmoothBuffer[i] < pSmoothBuffer[i+2] - PEAK_THRESHOLD_VOLTS))
		//		{
		//			minLoc = i;
		//			minVal = _pockelsReadArray[index][i];

		//		}
		//	}
		//	else
		//	{
		//		//transition point indicating a maximum
		//		if((pSmoothBuffer[i-2]  < pSmoothBuffer[i]) && (pSmoothBuffer[i] > pSmoothBuffer[i+2])&& ((pSmoothBuffer[i] - minVal) > DIFFERENCE_THRESHOLD_VOLTS))
		//		{
		//			maxLoc = i;
		//			maxVal = _pockelsReadArray[index][i];
		//			break;
		//		}
		//	}
		//}
		/*********** End of Method II ***************/

		//*NOTE* To display error information un comment the code below
		//if(minLoc == -1)
		//{
		//	MessageBox(NULL,L"no min found",NULL,NULL);
		//}
		//
		//if(maxLoc == -1)
		//{
		//	MessageBox(NULL,L"no max found",NULL,NULL);
		//}

		//if(midLoc == -1)
		//{
		//	wchar_t msg[MSG_SIZE]
		//	StringCbPrintfW(msg,MSG_SIZE,L"no Mid Found midLoc: %d midVal: %f", midLoc, midVal);
		//	MessageBox(NULL,msg,NULL,NULL);
		//}

		//if((maxVal-minVal) <= DIFFERENCE_THRESHOLD_VOLTS)
		//{
		//  wchar_t msg[MSG_SIZE]
		//	StringCbPrintfW(msg,MSG_SIZE,L"diff threshold failed minVal: %f maxVal: %f", minVal, maxVal);
		//	MessageBox(NULL,msg,NULL,NULL);
		//}

		if((minLoc != -1) && (maxLoc != -1) && ((maxVal - minVal) > DIFFERENCE_THRESHOLD_VOLTS))
		{
			_pockelsMinVoltage[index] = VOLTAGE_START + minLoc * VOLTAGE_RANGE / POCKELS_VOLTAGE_STEPS;
			_pockelsMaxVoltage[index] = VOLTAGE_START + maxLoc * VOLTAGE_RANGE / POCKELS_VOLTAGE_STEPS;
		}
		else
		{
			retVal = -1;
		}

		//remove the line below to show the raw data in the plot
		std::memcpy(&_pockelsReadArray[index],pSmoothBuffer,POCKELS_VOLTAGE_STEPS * sizeof(float64));

		SAFE_DELETE_ARRAY(pSmoothBuffer);
	}
	catch(...)
	{
		memset(&_pockelsReadArray[index],0,POCKELS_VOLTAGE_STEPS * sizeof(float64));

		retVal = -1;
	}
	return retVal;
}

/// <summary> Cancels any hardware triggers the camera is currently waiting on </summary>
void CdFLIMGG::StopHardwareWaits()
{
	long error = 0;
	//ThordaqErrChk (L"ThorDAQAPIAbortRead", ThorDAQAPIAbortRead(_DAQDeviceIndex));
	ThorDAQAPIAbortRead(_DAQDeviceIndex);
	SetEvent(_hStopAcquisition);
}

/// <summary> Calculates the minimum value for flyback cycles the current settings can support </summary>
/// <returns> Return 0, when vertical galvo is disabled </returns>
long CdFLIMGG::GetMinFlybackCycle()
{
	//TODO: FLyback cycle == 0 still needs some work. We will leave to be 1 for now, which looks fine.
	// We found some combinations make the Galvos click at flyback of 1, for now the new default will be 2. It still can be 1 in GGSuperUser mode
	//if (ICamera::LSMAreaMode::POLYLINE == _imgAcqPty.areaMode || FALSE == _imgAcqPty.galvoEnable)
	//{
	//	return 0;
	//}
	//else
	{
		if(TRUE == _ggSuperUserMode)
		{
			return 1;
		}
		return 2;
	}
}


/// <summary> Sets the current flyback cycle </summary>
/// <param name="flybackCycle"> The new value for flyback cycle </param>
void CdFLIMGG::SetFlybackCycle(long flybackCycle)
{
	if (MAX_FLYBACK_CYCLE < flybackCycle || MAX_FLYBACK_TIME < GetFlybackTime(flybackCycle))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_FLYBACK_CYCLE %d outside range %d to %d, and above %d seconds",static_cast<long> (flybackCycle), GetMinFlybackCycle(), MAX_FLYBACK_CYCLE,MAX_FLYBACK_TIME);
		LogMessage(_errMsg,ERROR_EVENT);
	}
	else
	{
		long minFlybackCycle = GetMinFlybackCycle();
		_imgAcqPty.flybackCycle = (flybackCycle > minFlybackCycle ? flybackCycle : minFlybackCycle);
	}
}

double CdFLIMGG::GetFlybackTime(long flybackCycles)
{
	if( flybackCycles == 0 && (FALSE == _imgAcqPty.galvoEnable || ICamera::LSMAreaMode::POLYLINE == _imgAcqPty.areaMode))
	{
		return LINE_FRAMETRIGGER_LOW_TIMEPOINTS * _imgAcqPty.dwellTime / 1000000.0;
	}
	else
	{
		double galvoSamplesPaddingTime = (_imgAcqPty.turnAroundTimeUS / 2.0) / 1000000.0;  //use user set amount of time for x galvo retrace, default is 400us
		double galvoSamplesPerFullLine = 2.0 * _imgAcqPty.pixelX * _imgAcqPty.dwellTime /1000000.0 + 2.0 *((double)_imgAcqPty.pixelX  - 1 ) * 17.0/200000000.0  + 4 * galvoSamplesPaddingTime;
		return galvoSamplesPerFullLine * flybackCycles;
	}
}

long CdFLIMGG::GetFlybackCycle()
{
	long minFlybackCycle = GetMinFlybackCycle();
	// If the current set flybackCycle is less than the minimum or "Always Use Fastest" is checked, set flybackCycle to the minimum
	if(_minimizeFlybackCycles || minFlybackCycle > _imgAcqPty.flybackCycle)
	{
		_imgAcqPty.flybackCycle = minFlybackCycle;
	}
	else if(GetFlybackTime(_imgAcqPty.flybackCycle) > MAX_FLYBACK_TIME)
	{
		double galvoSamplesPaddingTime = ((_imgAcqPty.turnAroundTimeUS / 2.0)  / 1000000.0); //use user set amount of time for x galvo retrace, default is 400us
		double galvoSamplesPerLine = 2.0 * (double)_imgAcqPty.pixelX * _imgAcqPty.dwellTime /1000000.0 + 2.0 *((double)_imgAcqPty.pixelX  - 1 ) * 17.0/200000000.0 + 4.0 * galvoSamplesPaddingTime;
		_imgAcqPty.flybackCycle = static_cast<long>(MAX_FLYBACK_TIME / galvoSamplesPerLine);
	}
	return _imgAcqPty.flybackCycle;
}

int CdFLIMGG::CountChannelBits(long channelSet)
{
	int count = 0;
	while(channelSet)
	{
		channelSet &= (channelSet - 1);
		count++;
	}
	return count;
}

//Enter/leave frame buffer to read/write:
UINT CdFLIMGG::HandleFrameBuf(int enter,DWORD timeOut)
{
	if(1 == enter)
	{
		if(WaitForSingleObject(_hFrmBufHandle, timeOut) != WAIT_OBJECT_0)
		{
			ReleaseMutex(_hFrmBufHandle);
			return FALSE;
		}
	}
	else
	{
		ReleaseMutex(_hFrmBufHandle);
	}
	return TRUE;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS CThordaq::LoadDACMemorySettings( IMAGING_DAC_MEMORY_SETTINGS_STRUCT& DACMemorySettings )
 *
 * @brief	Loads DAC memory settings.
 *
 * @author	Cge
 * @date	2/9/2017
 *
 * @param [in,out]	DACMemorySettings	The DAC memory settings.
 *
 * @return	The DAC memory settings.
 **************************************************************************************************/
long CdFLIMGG::LoadDACMemorySettings( IMAGING_BUFFER_STRUCT& DACMemorySettings)  // Writes the waveform DAC samples to DDR3 (0x7000.0000 and above)
{
	THORDAQ_STATUS status = STATUS_WRITE_BUFFER_ERROR;
	ULONG64 start_address = 0;
	long error = 0, retVal = 0;
	if (gPtrMemoryPool->RequestMemoryAllocation(DACMemorySettings.length,start_address))// Get the start writing position
	{
//		ThordaqErrChk(L"ThordaqPacketWriteBuffer", retVal = ThordaqPacketWriteBuffer(_DAQDeviceIndex, start_address, static_cast<ULONG>(DACMemorySettings.length), DACMemorySettings.buffer, 0xffffffff));
		ThordaqErrChk (L"ThorDAQAPIWriteDDR3", retVal = ThorDAQAPIWriteDDR3(_DAQDeviceIndex, DACMemorySettings.buffer, start_address, static_cast<UINT32>(DACMemorySettings.length)));
		if (retVal == STATUS_SUCCESSFUL) // Write configuration to the RAM
		{
			if (gPtrMemoryPool->AllocMemory(DACMemorySettings.channel, DACMemorySettings.length) == TRUE) 
			{
				status = STATUS_SUCCESSFUL;
			}
		}
	}else
	{
		status = STATUS_WRITE_BUFFER_ERROR;
	}
	return status;
}

long CdFLIMGG::UpdateDACMemorySettings( IMAGING_BUFFER_STRUCT& DACMemorySettings)
{
	THORDAQ_STATUS status = STATUS_WRITE_BUFFER_ERROR;
	ULONG64 start_address = 0;
	ULONG64 length = 0;
	long    error = 0, retVal = 0;
	if (gPtrMemoryPool->GetMemoryPropertyByChannel(DACMemorySettings.channel,start_address,length))
	{
		ThordaqErrChk(L"ThorDAQAPIWriteDDR3", retVal = ThorDAQAPIWriteDDR3(_DAQDeviceIndex, DACMemorySettings.buffer, start_address + DACMemorySettings.offset, static_cast<UINT32>(DACMemorySettings.length)));
		//ThordaqErrChk (L"ThorDAQAPIPacketWriteBuffer", retVal = ThorDAQAPIPacketWriteBuffer(_DAQDeviceIndex, start_address + DACMemorySettings.offset,static_cast<ULONG>(DACMemorySettings.length),DACMemorySettings.buffer,0xffffffff));
		if ((DACMemorySettings.length <= length) && (retVal == STATUS_SUCCESSFUL)) // Write configuration to the RAM
		{
			status = STATUS_SUCCESSFUL;
		}
	}
	return status;
}

LONG CdFLIMGG::BuildPockelsControlWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, double dac_rate, PockelPty* pockelPty, bool blockBackwardLine, ImgAcqPty* pImgAcqPty)
{
	for (int pockelsIndex = 0; pockelsIndex < MAX_POCKELS_CELL_COUNT; pockelsIndex++)
	{
		if (_pockelsEnable[pockelsIndex] == FALSE)
		{
			continue;
		}

		double pockelsOnVoltage;
		//the reference output is only used for pockels1, for everything else use the normal settings
		if (pockelsIndex > 0 || FALSE == pockelPty->useReferenceForPockelsOutput )
		{
			pockelsOnVoltage = _pockelsMinVoltage[pockelsIndex] + (_pockelsMaxVoltage[pockelsIndex] - _pockelsMinVoltage[pockelsIndex]) * pockelPty->pockelsPowerLevel[pockelsIndex];
		}
		else
		{
			pockelsOnVoltage = _pockelsMaxVoltage[pockelsIndex];
		}

		USHORT pockels_output_low_ushort = 0;
		USHORT pockels_output_high_ushort = static_cast<USHORT>((pockelsOnVoltage -  _pockelsMinVoltage[pockelsIndex])/ GALVO_RESOLUTION);

		//!hardcode to use the same amount fo GG waveform for now
		ULONG32 samples_pad = scanLine->samples_idle;
		ULONG32 samples_sweep = scanLine->samples_scan;
		UINT turnAroundOneWay = (3 * scanLine->samples_idle + scanLine->samples_scan);
		ULONG32 samples_single_line = TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode ? samples_pad * 2 + samples_sweep : 2*(samples_pad * 2 + samples_sweep);
		ULONG32 fullLine = TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode ? samples_single_line * 2 : samples_single_line;

		UINT sync_secure = TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode ? scanLine->samples_idle : samples_single_line / 2 + scanLine->samples_idle;
		UINT half_turnaround_padding = (samples_single_line - scanLine->samples_scan) / 2;  //This is a padding added to the beginning of each frame, it is supposed to align SOF with the beginning of the waveform, instead of SOF being at the beginning of scan.
		UINT forward_data_num  = (ULONG32)scanInfo->forward_lines  * samples_single_line;
		UINT backward_data_num = scanInfo->backward_lines == 0 ?  0 : (ULONG32)(scanInfo->backward_lines) * fullLine - (sync_secure + FLYBACK_OFFSET);
		UINT waveform_dataLength = TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode ? forward_data_num + backward_data_num : forward_data_num + backward_data_num + turnAroundOneWay;	
		UINT total_dataLength = waveform_dataLength + DAC_FIFO_DEPTH / 2;//Fifo depth: 1024 words (1 word = 32bits)
	
		ULONG32 lowSamples = samples_pad + static_cast<ULONG32>(ceil(pockelPty->pockelsLineBlankingPercentage[pockelsIndex] * samples_sweep - 0.5));
		ULONG32 endOfForwardLine = samples_pad + samples_sweep -  static_cast<ULONG32>(ceil(pockelPty->pockelsLineBlankingPercentage[pockelsIndex] * samples_sweep - 0.5));
		ULONG32 blankSamples = static_cast<ULONG32>(ceil(pockelPty->pockelsLineBlankingPercentage[pockelsIndex] * samples_sweep - 0.5));
		ULONG32 effectiveSamples = samples_sweep - 2*blankSamples;

		USHORT* pPockelsWaveform = new USHORT[total_dataLength];
		memset(pPockelsWaveform,pockels_output_low_ushort,total_dataLength*sizeof(USHORT));
		// set up the waveform for the horizontal galvo
		// first line

		// Use the two way offset value to find the amount of offset samples needed to align the pockels waveformS
		ULONG32 twowayOffsetSamples = static_cast<ULONG32>(max(0, ceil(_daqAcqCfg.imageCtrl.alignmentOffset) / static_cast<ULONG32>((double)SYS_CLOCK_FREQ / dac_rate + _pockelsDelayUS - 0.05)));
		//ULONG32 sample_index = half_turnaround_padding + lowSamples + twowayOffsetSamples;

		ULONG32 sample_index = TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode ? half_turnaround_padding + lowSamples + twowayOffsetSamples : turnAroundOneWay + lowSamples + twowayOffsetSamples;
		unsigned long initialOffset  = TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode ? half_turnaround_padding : turnAroundOneWay;

		for (ULONG32 i = 0; i < effectiveSamples; i++)
		{
			*(pPockelsWaveform+sample_index++) = pockels_output_high_ushort;
		}
		for (ULONG32 i = 0; i < lowSamples * 2; i++)
		{
			*(pPockelsWaveform+sample_index++) = pockels_output_low_ushort;
		}
		for (ULONG32 i = 0; i < effectiveSamples; i++)
		{
			if (TWO_WAY_SCAN_MODE != pImgAcqPty->scanMode || blockBackwardLine)
			{
				*(pPockelsWaveform+sample_index++) = pockels_output_low_ushort;
			}
			else
			{
				*(pPockelsWaveform+sample_index++) = pockels_output_high_ushort;
			}
		}
		for (ULONG32 i = 0; i < lowSamples; i++)
		{
			//if (samples_single_line <= sample_index)
			//{
			//	break;
			//}
			*(pPockelsWaveform+sample_index++) = pockels_output_low_ushort;
		} 

		if (!blockBackwardLine)
		{
			//TODO: need to offset for the pockels in the first lines of the pockels waveform
			//Maybe we need to copy only the difference, but test first
			// copy first line's settings to the rest lines
			if(TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode)
			{
				for (UINT i = 1; i < ((UINT)(scanInfo->forward_lines / 2)); i++)
				{
					if  ((twowayOffsetSamples + samples_single_line * (i + 1) * 2) <  total_dataLength)
					{
						memcpy((pPockelsWaveform+initialOffset+(UINT64)samples_single_line*i * 2 + twowayOffsetSamples),(pPockelsWaveform+initialOffset+twowayOffsetSamples),samples_single_line * sizeof(USHORT) * 2);
					}
				}
			}
			else
			{
				for (UINT i = 1; i < ((UINT)(scanInfo->forward_lines)); i++)
				{
					if  ((twowayOffsetSamples + samples_single_line * (i + 1)) <  total_dataLength)
					{
						memcpy((pPockelsWaveform+initialOffset+ (UINT64)samples_single_line*i + twowayOffsetSamples),(pPockelsWaveform+initialOffset+twowayOffsetSamples),samples_single_line * sizeof(USHORT));
					}
				}
			}
		}

		// This shold be handled in the memset part of total_datalenght to pockels_output_low_short. Delete if it is not necessary anymore
		/*if (backward_data_num >= half_turnaround_padding)
		{
			for (ULONG32 i = 0; i < (ULONG32)backward_data_num - half_turnaround_padding; i++)
			{
				*(pPockelsWaveform+((UINT)forward_data_num + half_turnaround_padding + i)) = pockels_output_low_ushort;
			}
		}
		memset(pPockelsWaveform + waveform_dataLength, pockels_output_low_ushort, DAC_FIFO_DEPTH); */

		// First load memory
		IMAGING_BUFFER_STRUCT waveform_pockels;
		waveform_pockels.buffer = (UCHAR*)pPockelsWaveform;
		waveform_pockels.length = total_dataLength * sizeof(USHORT);
		waveform_pockels.channel = Pockel1 + pockelsIndex;
		waveform_pockels.offset = 0;
		LoadDACMemorySettings(waveform_pockels);

		gPtrMemoryPool->GetMemoryPropertyByChannel(waveform_pockels.channel,_daqAcqCfg.dacCtrl[waveform_pockels.channel].waveform_buffer_start_address,_daqAcqCfg.dacCtrl[waveform_pockels.channel].waveform_buffer_length);
		_daqAcqCfg.dacCtrl[waveform_pockels.channel].park_val = _pockelsMinVoltage[pockelsIndex];
		_daqAcqCfg.dacCtrl[waveform_pockels.channel].offset_val = _pockelsMinVoltage[pockelsIndex];
		_daqAcqCfg.dacCtrl[waveform_pockels.channel].update_rate = dac_rate;
		_daqAcqCfg.dacCtrl[waveform_pockels.channel].flyback_samples = backward_data_num;
		_daqAcqCfg.dacCtrl[waveform_pockels.channel].output_port = waveform_pockels.channel;

		/*string waveformFile = "waveform" + to_string(i + 2)+".txt";
		 ofstream myfile (waveformFile);
		if (myfile.is_open())
		{
			for (int i = 0; i < total_dataLength; i++)
			{
				myfile << std::fixed << std::setprecision(8) << (*(pPockelsWaveform+i));
				myfile << "\n";
			}
		myfile.close();
		}*/

		SAFE_DELETE_ARRAY(pPockelsWaveform);
	}

	return TRUE;
}

LONG CdFLIMGG::BuildGalvoWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, double dac_rate, GalvoStruct* galvoCtrlX, GalvoStruct* galvoCtrlY, ImgAcqPty* pImgAcqPty)
{
	ULONG64 GalvoBitsMask = 0x000000000000ffff;
	UINT samples_line = TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode ? 2 * scanLine->samples_idle + scanLine->samples_scan : 2*(2 * scanLine->samples_idle + scanLine->samples_scan);
	UINT turnAroundOneWay = (3 * scanLine->samples_idle + scanLine->samples_scan);
	ULONG32 fullLine = TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode ? samples_line * 2 : samples_line;
	UINT sync_secure = TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode ? scanLine->samples_idle : samples_line / 2 + scanLine->samples_idle;
	UINT half_turnaround_padding = (samples_line - scanLine->samples_scan) / 2; //This is a padding added to the beginning of each frame, it is supposed to align SOF with the beginning of the waveform, instead of SOF being at the beginning of scan.
	UINT forward_data_num  = (ULONG32)scanInfo->forward_lines  * samples_line;
	UINT backward_data_num =  scanInfo->backward_lines == 0 ?  0 : (ULONG32)(scanInfo->backward_lines) * fullLine - (sync_secure + FLYBACK_OFFSET); // flyback = flyback lines * samples per line - (initial padding + 16). The padding is part of the flyback for the waveform playback
	UINT waveform_dataLength = TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode ? forward_data_num + backward_data_num : forward_data_num + backward_data_num + turnAroundOneWay;		// total samples for entire waveform including sweep time and flyback time
	UINT total_dataLength = waveform_dataLength + DAC_FIFO_DEPTH / 2;//Fifo depth: 1024 words (1 word = 32bits)
	/// Calculate the waveform of Y, Y is the slow axis.  Keeps moving at a constant pace to accomplish a sawtooth xy motion
	double linesToScan = TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode ? scanInfo->forward_lines / 2 : scanInfo->forward_lines;
	double yDirection = (SCAN_DIRECTION::FORWARD_SCAN == galvoCtrlY->scan_direction )? 1.0 : -1.0;
	double galvoYFwdStep = (galvoCtrlY->amplitude / linesToScan / GALVO_RESOLUTION);
	double vPadY = galvoYFwdStep / 4.0;
	double half_P2P_amp_Y = galvoCtrlY->amplitude / 2.0 / GALVO_RESOLUTION;
	double half_P2P_amp_X = galvoCtrlX->amplitude / 2.0 / GALVO_RESOLUTION;
	double amp_offset_Y   = galvoCtrlY->offset / GALVO_RESOLUTION + 0x8000;
	double amp_offset_X   = galvoCtrlX->offset / GALVO_RESOLUTION + 0x8000;
	USHORT highest_y = 0;
	USHORT highest_x = 0;

	//For X Waveform only
	double pad_amp = (galvoCtrlX->amplitude / (double)scanLine->samples_scan * (double)scanLine->samples_idle  / M_PI_2) / GALVO_RESOLUTION;
	double pad_step = M_PI_2 / (double)scanLine->samples_idle;
	double sweep_amp_step = galvoCtrlX->amplitude / (double)scanLine->samples_scan / GALVO_RESOLUTION; 
	double paddedAmplitude = pImgAcqPty->fieldSize * GALVO_RETRACE_TIME * 4 / ( pImgAcqPty->pixelX * pImgAcqPty->dwellTime * 2 * M_PI);
	double fieldX_angle = (pImgAcqPty->fieldSize + paddedAmplitude *2) * _field2Theta;

	double maxVal =  USHRT_MAX;
	double minVal =  0;

	double offset_y = galvoCtrlY->offset - yDirection* galvoCtrlY->amplitude / 2.0;
	offset_y = offset_y > 0 ? min(MAX_GALVO_VOLTAGE, offset_y) : max(-MAX_GALVO_VOLTAGE, offset_y);

	double offset_x = galvoCtrlX->offset - (galvoCtrlX->amplitude / (double)scanLine->samples_scan * (double)scanLine->samples_idle  / M_PI_2) - galvoCtrlX->amplitude / 2.0;
	offset_x = offset_x > 0 ? min(MAX_GALVO_VOLTAGE, offset_x) : max(-MAX_GALVO_VOLTAGE, offset_x);

	const USHORT HIGH = 0xffff;
	const USHORT LOW = 0x0;

	//----------------Build Y Waveform----------------
	USHORT* pGalvoWaveformY = new USHORT[total_dataLength];
	USHORT* pGalvoWaveformDigY = new USHORT[total_dataLength];

	// set up the waveform for the vertical galvo
	memset(pGalvoWaveformY, static_cast<USHORT>(minVal), total_dataLength * sizeof(USHORT));
	memset(pGalvoWaveformDigY, 0, total_dataLength * sizeof(USHORT));

	unsigned long initialOffset = TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode ? half_turnaround_padding : turnAroundOneWay;
	unsigned long k = initialOffset;
	// For 'invert vertical scan' we multiply the waveform value by -1.0 same as the NI/Alazar counterpart. The negative value is cast to
	//unsigned short. This will convert it to UMAX SHORT - waveform value, which works for the vertical invert. This is why we don't check if
	//the waveform value is below 0
	if (scanInfo->forward_lines > 1)
	{
		for (unsigned long i = 0 ; i < (UINT)linesToScan; ++i)
		{
			for (unsigned long j = 0; j < scanLine->samples_idle; ++j)
			{
				pGalvoWaveformY[k] = static_cast<USHORT>(min(maxVal, yDirection * (galvoYFwdStep * (double)i + vPadY * sin((double)j * pad_step))));
				pGalvoWaveformDigY[k++] = HIGH;
			}
			for (unsigned long j = 0; j < scanLine->samples_scan; ++j)
			{
				pGalvoWaveformY[k] = pGalvoWaveformY[k - 1];
				pGalvoWaveformDigY[k++] = HIGH;
			}
			for (unsigned long j = 0; j < scanLine->samples_idle * 2; ++j)
			{
				pGalvoWaveformY[k] = static_cast<USHORT>(min(maxVal, yDirection *(galvoYFwdStep * (double)i + vPadY * (2 + (sin((double)j * pad_step - M_PI_2 ))))));
				pGalvoWaveformDigY[k++] = HIGH;
			}	
			for (unsigned long j = 0; j < scanLine->samples_scan ; ++j)
			{
				pGalvoWaveformY[k] = pGalvoWaveformY[k - 1];
				pGalvoWaveformDigY[k++] = HIGH;
			}
			for (unsigned long j = 0; j < scanLine->samples_idle; ++j)
			{
				pGalvoWaveformY[k] = static_cast<USHORT>(min(maxVal, yDirection *(galvoYFwdStep * (double)i + vPadY * (3 + sin((double)j * pad_step)))));
				pGalvoWaveformDigY[k++] = HIGH;
			}
		}
		if (backward_data_num >= half_turnaround_padding)
		{
			for(unsigned long i = 0; i < backward_data_num - half_turnaround_padding; ++i)
			{
				pGalvoWaveformY[k] = static_cast<USHORT>(min(maxVal,(yDirection *(galvoYFwdStep * (double)linesToScan * (cos(M_PI * ((double)i + 1) / (double)(backward_data_num - half_turnaround_padding)) / 2.0 + 0.5)))));
				pGalvoWaveformDigY[k++] = LOW;
			}
		}
	}
	else
	{
		for(unsigned long i = 0; i< scanLine->samples_idle ; ++i)
		{
			pGalvoWaveformY[k] = 0;
			pGalvoWaveformDigY[k++] = LOW;
		}
		for (unsigned long j = 0; j < scanLine->samples_scan; ++j)
		{
			pGalvoWaveformY[k] = 0;
			pGalvoWaveformDigY[k++] = HIGH;
		}

		for(unsigned long i = 0; i< scanLine->samples_idle * 2; ++i)
		{
			pGalvoWaveformY[k] = 0;
			pGalvoWaveformDigY[k++] = LOW;
		}
		for (unsigned long j = 0; j < scanLine->samples_scan; ++j)
		{
			pGalvoWaveformY[k] = 0;
			pGalvoWaveformDigY[k++] = TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode ? HIGH : LOW;
		}
		for(unsigned long i = 0; i< scanLine->samples_idle * 2; ++i)
		{
			pGalvoWaveformY[k] = 0;
			pGalvoWaveformDigY[k++] = LOW;
		}
	}

	//----------------Build X Waveform----------------
	USHORT* pGalvoWaveformX = new USHORT[total_dataLength];
	memset(pGalvoWaveformX,static_cast<USHORT>(minVal),total_dataLength*sizeof(USHORT));

	ULONG32 sample_index = initialOffset;//half_turnaround_padding;
	for (ULONG32 i = 0; i < scanLine->samples_idle; ++i)
	{
		*(pGalvoWaveformX+sample_index++) = static_cast<USHORT>(max(minVal ,min(maxVal, pad_amp - pad_amp * cos(pad_step*double(i))))); // pi / 2
	}
	for (ULONG32 i = 0; i < scanLine->samples_scan; ++i)
	{
		*(pGalvoWaveformX+sample_index++) = static_cast<USHORT>(max(minVal, min(maxVal, pad_amp + sweep_amp_step * (double)i)));
	}
	for (ULONG32 i = 0; i < scanLine->samples_idle * 2; ++i)
	{
		*(pGalvoWaveformX+sample_index++) = static_cast<USHORT>(max(minVal,min(maxVal,half_P2P_amp_X * 2 + pad_amp - pad_amp * cos(M_PI_2 + pad_step*double(i)))));  // pi /2 - pi 3/2
	}
	for (ULONG32 i = 0; i < scanLine->samples_scan; ++i)
	{
		*(pGalvoWaveformX+sample_index++) = static_cast<USHORT>(max(minVal,min(maxVal,half_P2P_amp_X * 2 + pad_amp - sweep_amp_step * (double)i)));
	}
	for (ULONG32 i = 0; i < scanLine->samples_idle; ++i)
	{
		*(pGalvoWaveformX+sample_index++) = static_cast<USHORT>(max(minVal, min(maxVal,pad_amp - pad_amp * cos(1.5 * M_PI + pad_step*double(i))))); // pi 3/2 - 2 pi 
	} 

	// copy first line's settings to the rest lines
	if(TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode)
	{
		for (UINT i = 1; i < ((UINT)(scanInfo->forward_lines / 2)); ++i)
		{
			memcpy((pGalvoWaveformX+initialOffset+(UINT64)samples_line*2*i),(pGalvoWaveformX+initialOffset), (UINT64)samples_line*2 * sizeof(USHORT));
		}
	}
	else
	{
		for (UINT i = 1; i < ((UINT)(scanInfo->forward_lines)); ++i)
		{
			memcpy((pGalvoWaveformX+initialOffset+ (UINT64)samples_line*i),(pGalvoWaveformX+initialOffset),samples_line * sizeof(USHORT));
		}
	}

	//flyback should be set already at memeset (total waveform lenght) to 0. Delete if there is no issue with not having this
	/*if (backward_data_num >= half_turnaround_padding)
	{
	for (ULONG32 i = 0; i < (ULONG32)backward_data_num - half_turnaround_padding; ++i)
	{
	*(pGalvoWaveformX+((UINT)forward_data_num + half_turnaround_padding + i)) = 0;
	}
	}*/

	//----------------Process Galvo XY Waveform----------------
	if (0 != pImgAcqPty->scanAreaAngle)
	{
		double angle = (1 == pImgAcqPty->verticalScanDirection) ? -pImgAcqPty->scanAreaAngle : pImgAcqPty->scanAreaAngle;
		double tempX = 0, tempY = 0;
		for (unsigned long i = 0; i < static_cast<unsigned long>(total_dataLength); ++i)
		{
			tempX = pGalvoWaveformX[i];
			tempY = pGalvoWaveformY[i];
			pGalvoWaveformX[i] = static_cast<USHORT>(floor(tempX*cos(angle) - tempY*sin(angle) + 0.5));
			pGalvoWaveformY[i] = static_cast<USHORT>(floor(tempX*sin(angle) + tempY*cos(angle) + 0.5));
		}
		tempX = offset_x;
		tempY = offset_y;
		offset_x = tempX*cos(angle) - tempY*sin(angle);
		offset_y = tempX*sin(angle) + tempY*cos(angle);
		if(offset_x > MAX_GALVO_VOLTAGE || offset_x < MIN_GALVO_VOLTAGE || offset_y > MAX_GALVO_VOLTAGE || offset_y < MIN_GALVO_VOLTAGE)
		{
			wchar_t errMsg2[MSG_SIZE];
			StringCbPrintfW(errMsg2,MSG_SIZE,L"ThorDAQGalvoGalvo BuildGalvoWaveforms Offsets out of range, OffsetX: %f OffsetY: %f", offset_x, offset_y);
			CdFLIMGG::GetInstance()->LogMessage(errMsg2,ERROR_EVENT);	
			return FALSE;
		}
	}
	//-------------------------------------------------

	////print data for matlab
	/*
	ofstream myfile1 ("waveformY.txt");
	if (myfile1.is_open())
	{
	for (UINT i = 0; i < total_dataLength; i++)
	{
	myfile1 << std::fixed << std::setprecision(8) << (*(pGalvoWaveformY+i));
	myfile1 << "\n";
	}
	myfile1.close();
	}

	ofstream myfile ("waveformX.txt");
	if (myfile.is_open())
	{
	for (UINT i = 0; i < total_dataLength; i++)
	{
	myfile << std::fixed << std::setprecision(8) << (*(pGalvoWaveformX+i));
	myfile << "\n";
	}
	myfile.close();
	}*/

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg,MSG_SIZE,L"ThorDAQGalvoGalvo Samples idle: %d \nSamples Scan: %d \n OffsetX: %f \n OffsetY: %f", scanLine->samples_idle, scanLine->samples_scan, offset_x, offset_y);
	CdFLIMGG::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);	

	//--------------Analog Y galvo waveform--------------
	// First load memory
	IMAGING_BUFFER_STRUCT waveformY;
	waveformY.buffer = (UCHAR*)(pGalvoWaveformY + FIFO_DELAY_SAMPLES);
	waveformY.length = (total_dataLength - FIFO_DELAY_SAMPLES) * sizeof(USHORT);
	waveformY.channel = DAC_CHANNEL::GalvoGalvoY;
	waveformY.offset = 0;
	LoadDACMemorySettings(waveformY);

	gPtrMemoryPool->GetMemoryPropertyByChannel(waveformY.channel,_daqAcqCfg.dacCtrl[waveformY.channel].waveform_buffer_start_address,_daqAcqCfg.dacCtrl[waveformY.channel].waveform_buffer_length);
	_daqAcqCfg.dacCtrl[waveformY.channel].park_val = galvoCtrlY->park;
	_daqAcqCfg.dacCtrl[waveformY.channel].offset_val = offset_y;
	_daqAcqCfg.dacCtrl[waveformY.channel].update_rate = dac_rate;
	_daqAcqCfg.dacCtrl[waveformY.channel].flyback_samples = backward_data_num;
	_daqAcqCfg.dacCtrl[waveformY.channel].output_port = waveformY.channel;

	SAFE_DELETE_ARRAY(pGalvoWaveformY);

	//--------------Digital Y galvo waveform--------------
	// First load memory
	IMAGING_BUFFER_STRUCT waveformDigY;
	waveformDigY.buffer = (UCHAR*)(pGalvoWaveformDigY + FIFO_DELAY_SAMPLES);
	waveformDigY.length = (total_dataLength - FIFO_DELAY_SAMPLES) * sizeof(USHORT);
	waveformDigY.channel = DAC_CHANNEL::DO0;
	waveformDigY.offset = 0;
	LoadDACMemorySettings(waveformDigY);

	gPtrMemoryPool->GetMemoryPropertyByChannel(waveformDigY.channel,_daqAcqCfg.dacCtrl[waveformDigY.channel].waveform_buffer_start_address,_daqAcqCfg.dacCtrl[waveformDigY.channel].waveform_buffer_length);
	_daqAcqCfg.dacCtrl[waveformDigY.channel].park_val = 0;
	_daqAcqCfg.dacCtrl[waveformDigY.channel].offset_val = 0;
	_daqAcqCfg.dacCtrl[waveformDigY.channel].update_rate = dac_rate;
	_daqAcqCfg.dacCtrl[waveformDigY.channel].flyback_samples = backward_data_num;
	_daqAcqCfg.dacCtrl[waveformDigY.channel].output_port = waveformDigY.channel;

	SAFE_DELETE_ARRAY(pGalvoWaveformDigY);

	//--------------Analog X galvo waveform--------------
	// First load memory
	IMAGING_BUFFER_STRUCT waveformX;
	waveformX.buffer = (UCHAR*)(pGalvoWaveformX + FIFO_DELAY_SAMPLES);
	waveformX.length = (total_dataLength - FIFO_DELAY_SAMPLES) * sizeof(USHORT);
	waveformX.channel = DAC_CHANNEL::GalvoGalvoX;
	waveformX.offset = 0;
	LoadDACMemorySettings(waveformX);


	gPtrMemoryPool->GetMemoryPropertyByChannel(waveformX.channel,_daqAcqCfg.dacCtrl[waveformX.channel].waveform_buffer_start_address,_daqAcqCfg.dacCtrl[waveformX.channel].waveform_buffer_length);
	_daqAcqCfg.dacCtrl[waveformX.channel].park_val = galvoCtrlX->park;
	_daqAcqCfg.dacCtrl[waveformX.channel].offset_val = offset_x;
	_daqAcqCfg.dacCtrl[waveformX.channel].update_rate = dac_rate;
	_daqAcqCfg.dacCtrl[waveformX.channel].flyback_samples = backward_data_num;
	_daqAcqCfg.dacCtrl[waveformX.channel].output_port = waveformX.channel;

	SAFE_DELETE_ARRAY(pGalvoWaveformX);

	return TRUE;
}

long CdFLIMGG::AlignDataLoadFile()
{
	long i;
	char appPath[256]; ///<the char of application path

	char filePath[256]; ///<char of the whole file path
	_getcwd(appPath, 256);
	strcpy_s(filePath, appPath);
	strcat_s(filePath, "\\AlignDataGalvo.txt");
	FILE *AlignDataFile;
	if (fopen_s(&AlignDataFile, filePath, "r") != 0)
	{
		for (i = 0; i < 256; i++)
			_shiftArray[i] = 0;
		return TRUE;
	}
	else
	{
		for (i = 0; i < 256; i++)
		{
			if (fscanf_s(AlignDataFile, "%d", &_shiftArray[i]) == EOF)
			{
				for (i; i < 256; i++)
					_shiftArray[i] = 0;
			}
			else if (_shiftArray[i] < MIN_ALIGNMENT)
				_shiftArray[i] = MIN_ALIGNMENT;
			else if (_shiftArray[i] > MAX_ALIGNMENT)
				_shiftArray[i] = MAX_ALIGNMENT;
		}
		fclose(AlignDataFile);
		return TRUE;
	}
}

//BGB / JB version 3
LONG CdFLIMGG::GetDACSamplesPerLine(ScanLineStruct* scanLine, ImgAcqPty* PImgAcqPty, double& dac_rate, double dwell_time, double& line_time, long& turnAroundSamplesDif, bool onewayLineScan)
{
	//To find proper register for dwell time and pixel delay time
	//line time = pixel_density * pixel_dwelltime + (pixel_density - 1) * pixel_delaytime
	//time sequence likes below:
	//            __________   __________         ___________   __________              __________   ____
	//           |    |     | |    |     |       |     |     | |    |     |            |    |     | |    |
	// ----------            --           -------             -            ------------            -      ------------
	//-----------dewll delay  dwell delay.........dwell delay  dwell delay.............dwell delay  dwell------------

	double theta = (double) PImgAcqPty->fieldSize * _field2Theta;
	double amp_x = theta * _theta2Volts * PImgAcqPty->fineFieldSizeScaleX; // horizontal galvo amplitude
	//hard code pixel delay time to be the minimal
	double pixel_delay_time = 1;
	//find proper register for turnaround time and DAC update rate to make integer samples of total line times.
	bool isFound = false;
	int samples_line_time = 0;
	int singleLineTime = 0;

	//int dwelltime = static_cast<int>(ceil(((double)PImgAcqPty->pixelX * dwell_time * 1000000.0 * 200.0 + 17.0) / ((double)PImgAcqPty->pixelX -17.0)));
	double tempDwellns = static_cast<int>(ceil(((double)PImgAcqPty->pixelX * dwell_time * 1000000.0 * 200.0 + 17.0) / ((double)PImgAcqPty->pixelX -17.0)));
	const double CLOCK_PRECISION_200MHZ = 5.0; //5ns
	double result = tempDwellns + CLOCK_PRECISION_200MHZ/2.0;
	long dwelltime = static_cast<long>(result - std::fmod(result, CLOCK_PRECISION_200MHZ));

	long vsize = onewayLineScan ? 2 : PImgAcqPty->pixelY;

	int max_samples_per_channel_per_line = DAC_MAX_BUFFER_CHANNEL / vsize / 12; // need to revise later
	//int pre_update_rate = static_cast<int>((floor)(((int)PImgAcqPty->pixelX * dwelltime + 17 * (PImgAcqPty->pixelX - 1) + 2 * (3125 *16 + 1))/ max_samples_per_channel));
	int pre_samples_of_sweep = PImgAcqPty->pixelX * dwelltime + static_cast<int>(pixel_delay_time * 16 + 1) * (PImgAcqPty->pixelX - 1);
	//pre_update_rate = max(pre_update_rate , pre_samples_of_sweep / (int)(x_amp / GALVO_RESOLUTION) - 1);
	int pre_update_rate = pre_samples_of_sweep / (int)max_samples_per_channel_per_line - 1;
	pre_update_rate = min(USHRT_MAX,pre_update_rate);
	int updateRate = (ULONG32)max(SYS_CLOCK_FREQ / DAC_MAX_UPDATERATE - 1, pre_update_rate); // DAC_SAMPLE_RATE = SYS_CLK / (MAX_REGISTER_UPDATERATE + 1)
	int turnaround = static_cast<int>(ceil(((((double)PImgAcqPty->turnAroundTimeUS / 2.0) * (updateRate + 1.0)) - 1.0) / 16.0)); //Convert from microseconds to clock counts (200MHz)

	for (; updateRate < 65536; updateRate++) // The range of update rate is 3k - 1M
	{
		for (; turnaround < 6250; turnaround++)//max is 500us for half turaround time
		{
			double sample =  (double) ((double)PImgAcqPty->pixelX * dwelltime + (pixel_delay_time * 16 + 1) * ((double)PImgAcqPty->pixelX - 1) + 2 * ((double)turnaround * 16 + 1)) / (updateRate + 1.0);
			if ((sample - floor(sample)) < 0.2)
			{
				samples_line_time = static_cast<long>(((double)PImgAcqPty->pixelX * dwelltime + (pixel_delay_time * 16 + 1) * ((double)PImgAcqPty->pixelX - 1) + 2 * ((double)turnaround * 16 + 1)) / (updateRate + 1.0));
				isFound = true;
				break;
			}
		} 
		if (isFound)
		{
			break;
		}
	}
	dac_rate = (double)SYS_CLOCK_FREQ / ((double)updateRate + 1);

	// Step3: Setup the DAC register 
	// Setup the update rate for horizontal and vertical galvo
	scanLine->samples_scan = (ULONG32) (((double)PImgAcqPty->pixelX * (double)dwelltime + 17.0 * ((double)PImgAcqPty->pixelX - 1.0)) / ((double)updateRate + 1.0));

	// round up to nearest even number
	if ((samples_line_time - scanLine->samples_scan) % 2 != 0)
	{
		scanLine->samples_scan++;
	}

	//TODO" uncomment code below and test two way alignment fix
	if (PImgAcqPty->scanMode == TWO_WAY_SCAN_MODE)
	{
		turnAroundSamplesDif = turnaround - 2500;
		if (turnAroundSamplesDif < 0)
		{
			turnAroundSamplesDif = 0;
		}
	}

	//
	//turnAroundSamplesDif = 0;

	scanLine->samples_idle = (ULONG32)(samples_line_time - scanLine->samples_scan) / 2;// samples of turnaround time

	_daqAcqCfg.galvoGalvoCtrl.dwellTime = (double)dwelltime / (double)SYS_CLOCK_FREQ;
	_daqAcqCfg.galvoGalvoCtrl.pixelDelayCnt = pixel_delay_time;
	_daqAcqCfg.galvoGalvoCtrl.lineTime = line_time = (double)((PImgAcqPty->pixelX * (double)dwelltime / (double)SYS_CLOCK_FREQ) + ((pixel_delay_time * 16 + 1) * (PImgAcqPty->pixelX - 1.0) / (double)SYS_CLOCK_FREQ) + (2.0 * ((double)turnaround * 16.0 + 1.0) / (double)SYS_CLOCK_FREQ));
	if (PImgAcqPty->scanMode == TWO_WAY_SCAN_MODE)
	{
		_daqAcqCfg.galvoGalvoCtrl.turnaroundTime = 2.0 * ((double)turnaround * 16.0 + 1.0) / (double)SYS_CLOCK_FREQ;
	}
	else
	{
		// For one way, the turn around is acceleration + deceleration + 1 line time (for the retrace)
		_daqAcqCfg.galvoGalvoCtrl.turnaroundTime = line_time + 2.0 * ((double)turnaround * 16.0 + 1.0) / (double)SYS_CLOCK_FREQ;
	}

	return TRUE;
}