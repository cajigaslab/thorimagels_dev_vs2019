// ThorDAQGalvoGalvo.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ThorDAQGalvoGalvo.h"
#include "thordaqGalvoGalvoSetupXML.h"

#ifdef LOGGING_ENABLED
#include "..\..\..\..\..\Common\Log.h"
static std::unique_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

wchar_t message[MSG_SIZE];
#define DAQmxErrChk(fnName,fnCall) if ((error=(fnCall)) < 0){ StringCbPrintfW(message,MSG_SIZE,L"%s GG failed (%d),(%d). ",fnName,error,__LINE__); CThorDAQGalvoGalvo::GetInstance()->LogMessage(message,ERROR_EVENT); throw "fnCall";}else{ StringCbPrintfW(message,MSG_SIZE,L"%s GG failed (%d) (%d). ",fnName,error,__LINE__); CThorDAQGalvoGalvo::GetInstance()->LogMessage(message,VERBOSE_EVENT);}
#define MAX_TASK_WAIT_TIME 10.0

/************************************************************************************************
* @fn	CThorDAQGalvoGalvo::CThorDAQGalvoGalvo()
*
* @brief	Default constructor.
*
**************************************************************************************************/
CThorDAQGalvoGalvo::CThorDAQGalvoGalvo()
{
	_deviceNum = 0;
	_pDetectorName = L"ThorDAQGalvoGalvo";
	_field2Theta = 0.0901639344; //the field to scan angle conversion adapted from resonant galvo scanner code
	_theta2Volts = 1.0;
	_frameRate = 1;
	_oneXFieldSize = 160;
	_fieldSizeCalibrationAvailable = FALSE;
	_forceSettingsUpdate = FALSE;
	_droppedFramesCnt = 0;
	_triggerWaitTimeout = DEFAULT_TRIGGER_TIMEOUT;
	_frameTriggerEnableWithHWTrig = TRUE;
	_pFrmBuffer = NULL;
	_FIRFilterSelectedIndex = 0;
	_FIRFilterSelectedSettingChannel = 0;
	_FIRFilterSelectedTapIndex = 0;
	_useExternalBoxFrequency3P = FALSE;
	_fieldSizeMin = MIN_FIELD_SIZE_X;
	_fieldSizeMax = MAX_FIELD_SIZE_X;
	SAFE_DELETE_PTR(_pDataProcessor);
	//SAFE_DELETE_PTR(_gPtrWaveformMemoryPool);
	_pTempBuf = nullptr;
	_ratio = 1;
	_ggSuperUserMode = FALSE;
	_dwellTimeStep = DWELL_TIME_STEP;
	_minDwellTime = MIN_DWELL_TIME;
	_flybackCycles = 1;
	_galvoParkAtStart = FALSE;
	_displayedDwellTime = MIN_DWELL_TIME;
	_pockelsTurnAroundBlank = TRUE;
	_pockelsFlybackBlank = TRUE;
	_pockelsMaskChanged = FALSE; 
	_sumPulsesPerPixel = FALSE;

	_captureActiveInvert = FALSE;
	_scannerType = 0;
	_maxScannerSampleRate = DAC_MAX_UPDATERATE;

	_pockelsParkAtMinimum = 0;
	for (long i = 0; i < MAX_POCKELS_CELL_COUNT; i++)
	{
		_pockelsEnable[i] = false;
		_taskHandleAIPockels[i] = 0;
		_pockelsScanVoltageStart[i] = 0.0;
		_pockelsScanVoltageStop[i] = 0.0;
		_pockelsPowerInputLine[i] = "";
		_pockelsResponseType[i] = 0;
		_pPockelsMask[i] = nullptr;
		_pockelsMaskSize[i] = 0;
		memset(&_pockelsReadArray[i], 0, POCKELS_VOLTAGE_STEPS * sizeof(float64));
	}

	for (long i = 0; i < MAX_CHANNEL_COUNT; i++)
	{
		_preDcOffset[i] = 0;
	}
	_currentlyImaging = false;
	_currentlyStimulating = false;
	_useFastOneway = false;
	_enableGalvoXPark = FALSE;
	_limitGalvoSpeed = TRUE;

	_ddsClockEnable = false;
	_ddsClockPhase0 = 1.0;
	_ddsClockPhase1 = 1.0;
	_ddsClockAmplitude0 = 0.0;
	_ddsClockAmplitude1 = 0.0;
	_multiplaneBlankLines = 0;
	_multiplaneBlankLinesInLiveModeOnly = TRUE;
	_updatingParam = false;
	_dacTriggerMode = THORDAQ_TRIGGER_MODES::THORDAQ_SOFTWARE_START;

	_imagingActiveAOSelection = std::map<GG_AO, long>();

	_secondaryGGAvailable = false;

	_maxAngularVelocityRadPerSec = DEFAULT_MAX_GALVO_VELOCITY;
	_maxAngularAccelerationRadPerSecSq = DEFAULT_MAX_GALVO_ACCELERATION;
	_isDynamicWaveformLoadingStim = false;
	_tdqBOBType = THORDAQ_BOB_TYPE::TDQ_3U_BOB;
	//memset(&_daqAcqCfg, 0, sizeof(IMAGING_CONFIGURATION_STRUCT));
	SetupDataMaps();
}
/************************************************************************************************
* @fn	CThorDAQGalvoGalvo::~CThorDAQGalvoGalvo()
*
* @brief	Destructor.
*
**************************************************************************************************/
CThorDAQGalvoGalvo::~CThorDAQGalvoGalvo()
{
	//Reset the device parameter
	SAFE_DELETE_PTR(_pDataProcessor);
	_deviceNum = 0;
	_instanceFlag = false;
}

///******	Initialize Static Members		******///
bool CThorDAQGalvoGalvo::_instanceFlag = false;
ImgAcqPty CThorDAQGalvoGalvo::_imgAcqPty = ImgAcqPty();
ImgAcqPty CThorDAQGalvoGalvo::_imgAcqPty_Pre = ImgAcqPty();
shared_ptr<CThorDAQGalvoGalvo> CThorDAQGalvoGalvo::_single(nullptr);//Instantiated on first use
HANDLE CThorDAQGalvoGalvo::_hStopAcquisition = CreateEvent(NULL, TRUE, FALSE, NULL);	//make sure the reset option is true (MANUAL RESET)
HANDLE CThorDAQGalvoGalvo::_hThreadStopped = CreateEvent(NULL, TRUE, TRUE, NULL);
HANDLE CThorDAQGalvoGalvo::_hTriggerTimeout = CreateEvent(NULL, FALSE, FALSE, NULL);
HANDLE CThorDAQGalvoGalvo::_hHardwareTriggerInEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
HANDLE CThorDAQGalvoGalvo::_hExperimentThread = NULL; // Main Acquisition thread
long CThorDAQGalvoGalvo::_DAQDeviceIndex = DEFAULT_CARD_NUMBER;

long CThorDAQGalvoGalvo::_triggerWaitTimeout = 0;
wchar_t CThorDAQGalvoGalvo::_errMsg[_MAX_PATH] = { NULL };
wchar_t CThorDAQGalvoGalvo::thordaqLogMessage[MSG_SIZE];

/************************************************************************************************
* @fn	CThorDAQGalvoGalvo* CThorDAQGalvoGalvo::GetInstance()
*
* @brief	Gets the instance. Singlton design pattern
*
* @return	null if it fails, else the instance.
**************************************************************************************************/
CThorDAQGalvoGalvo* CThorDAQGalvoGalvo::GetInstance()
{
	if (!_instanceFlag)
	{
		_single.reset();
		_single = unique_ptr<CThorDAQGalvoGalvo>(new CThorDAQGalvoGalvo());
		_instanceFlag = true;
		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

/************************************************************************************************
* @fn	long CThorDAQGalvoGalvo::FindCameras(long &cameraCount)
*
* @brief	Searches for the devices.
*
* @param [in,out]	cameraCount	Index of Camera.
*
* @return	False if no Camera found. True if Camera found.
**************************************************************************************************/
long CThorDAQGalvoGalvo::FindCameras(long& cameraCount)
{
	int32 error = 0, retVal = 0;
	StringCbPrintfW(_errMsg, MSG_SIZE, L"CThorDAQGalvoGalvo::FindCameras, BindBoard, Read XML...");
	LogMessage(_errMsg, VERBOSE_EVENT);
	//MessageBox(NULL, L"test", L"test", MB_DEFBUTTON1);
	// First make sure there is a board installed
	try
	{
		ThordaqErrChk(L"ThorDAQAPIBindBoard", retVal = ThorDAQAPIBindBoard(_DAQDeviceIndex));

		LOW_FREQ_TRIG_BOARD_INFO_STRUCT lfbInfo = LOW_FREQ_TRIG_BOARD_INFO_STRUCT();
		ThordaqErrChk(L"ThorDAQAPIGetLowFreqTriggerBoardCfg", retVal = ThorDAQAPIGetLowFreqTriggerBoardCfg(_DAQDeviceIndex, &lfbInfo));
		_lowFreqTrigBoardInfo = lfbInfo;

		BOARD_INFO_STRUCT boardInfo = BOARD_INFO_STRUCT();

		ThordaqErrChk(L"ThorDAQAPIGetBoardCfg", retVal = ThorDAQAPIGetBoardCfg(_DAQDeviceIndex, &boardInfo));
		_boardInfo = boardInfo;
		UINT8 major = static_cast<UINT8>((boardInfo.UserVersion & THORDAQ_FIRMWARE_VERSION_RELEASE_MAJOR_BIT_SELECT) >> 28);
		UINT8 minor = static_cast<UINT8>((boardInfo.UserVersion & THORDAQ_FIRMWARE_VERSION_RELEASE_MINOR_BIT_SELECT) >> 24);
		UINT32 date = static_cast<UINT32>(boardInfo.UserVersion & THORDAQ_FIRMWARE_VERSION_RELEASE_DATE_BIT_SELECT);

		if ((major < THORDAQ_FIRMWARE_VERSION_RELEASE_MAJOR_MIN || (major == THORDAQ_FIRMWARE_VERSION_RELEASE_MAJOR_MIN && minor < THORDAQ_FIRMWARE_VERSION_RELEASE_MINOR_MIN) || date < THORDAQ_FIRMWARE_VERSION_RELEASE_DATE_MIN) &&
			(boardInfo.UserVersion < THORDAQ_FIRMWARE_VERSION_DEBUG_MIN || boardInfo.UserVersion > THORDAQ_FIRMWARE_VERSION_DEBUG_MAX))
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, _MAX_PATH, L"ThorDAQ firmware version Should be greater or equal to 0x%X", THORDAQ_FIRMWARE_VERSION_RELEASE_MIN);
			MessageBox(NULL, errMsg, L"Out of date firmware", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
			cameraCount = 0;
			return FALSE;
		}

		if (retVal == STATUS_SUCCESSFUL)
		{
			ThordaqErrChk(L"ThorDAQAPIGetBOBType", retVal = ThorDAQAPIGetBOBType(_DAQDeviceIndex, _tdqBOBType));
			cameraCount = 1;
			_deviceNum = 1;
			StopDaqBrd();
			unique_ptr<ThorGalvoGalvoXML> pSetup(new ThorGalvoGalvoXML());

			//XML settings retrieval functions will throw an exception if tags or attributes are missing
			//catch each independetly so that as many tags as possible can be read
			try
			{
				if (FALSE == pSetup->GetConfiguration(_field2Theta, _pockelsParkAtMinimum, _imgAcqPty.pockelPty.pockelsDelayUS, _galvoParkAtStart, _useExternalBoxFrequency3P, _fieldSizeMin, _fieldSizeMax, _pockelsTurnAroundBlank, _pockelsFlybackBlank, _sumPulsesPerPixel, _scannerType, _maxScannerSampleRate, _enableGalvoXPark, _limitGalvoSpeed, _multiplaneBlankLines, _multiplaneBlankLinesInLiveModeOnly, _maxAngularVelocityRadPerSec, _maxAngularAccelerationRadPerSecSq))
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"GetConfiguration from ThorDAQGalvoGalvoSettings.XML failed");
					LogMessage(_errMsg, ERROR_EVENT);
				}

				if (_imgAcqPty.numberOfPlanes < MIN_NUMBER_OF_PLANES || _imgAcqPty.numberOfPlanes > MAX_NUMBER_OF_PLANES)
				{
					_imgAcqPty.numberOfPlanes = MIN_NUMBER_OF_PLANES;
				}
			}
			catch (...)
			{
				// we don't mind missing XML fields, but if the entire .XML file is MISSING, we should fail
				StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorDAQGalvoGalvoSettings.XML file not found - fatal error, no 'camera'");
				LogMessage(_errMsg, ERROR_EVENT);
				cameraCount = 0;
				_deviceNum = 0;
			}

			try
			{
				long vertScanDir = 0;
				if (FALSE == pSetup->GetCalibration(_fieldSizeCalibration, vertScanDir, _imgAcqPty.fineOffsetX, _imgAcqPty.fineOffsetY, _imgAcqPty.fineFieldSizeScaleX, _imgAcqPty.fineFieldSizeScaleY, _oneXFieldSize, _imgAcqPty.pockelPty.pockelsMinVoltage, _imgAcqPty.pockelPty.pockelsMaxVoltage))
				{
					//if fieldSizeCalibration not exists, set its GetParamInfo Available
					_fieldSizeCalibrationAvailable = FALSE;

					StringCbPrintfW(_errMsg, MSG_SIZE, L"GetCalibration from ThorDAQGalvoGalvoSettings.XML failed. FieldSizeCalibration not available.");
					LogMessage(_errMsg, ERROR_EVENT);
				}
				else
				{
					_fieldSizeCalibrationAvailable = TRUE;
					_imgAcqPty.verticalScanDirection = (0 == vertScanDir) ? 1 : -1;
				}
			}
			catch (...)
			{
			}

			try
			{
				if (FALSE == pSetup->GetIO(_pockelsVoltageSlopeThreshold,
					_pockelsEnable[0], _pockelsPowerInputLine[0], _pockelsScanVoltageStart[0], _pockelsScanVoltageStop[0],
					_pockelsEnable[1], _pockelsPowerInputLine[1], _pockelsScanVoltageStart[1], _pockelsScanVoltageStop[1],
					_pockelsEnable[2], _pockelsPowerInputLine[2], _pockelsScanVoltageStart[2], _pockelsScanVoltageStop[2],
					_pockelsEnable[3], _pockelsPowerInputLine[3], _pockelsScanVoltageStart[3], _pockelsScanVoltageStop[3],
					_pockelsResponseType[0], _pockelsResponseType[1], _pockelsResponseType[2], _pockelsResponseType[3]))
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"GetIO from ThorConfocalSettings failed");
					LogMessage(_errMsg, ERROR_EVENT);
				}
			}
			catch (...)
			{
			}

			try
			{
				if (FALSE == pSetup->GetTrigger(_triggerWaitTimeout))
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"GetTrigger from ThorConfocalSettings failed");
					LogMessage(_errMsg, ERROR_EVENT);
				}
			}
			catch (...)
			{
			}

			try
			{
				if (FALSE == pSetup->GetStreamConfiguration(_imgAcqPty.clockRateInternal))
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"GetStreamConfiguration from ThorDAQGalvoGalvoSettings.XML failed");
					LogMessage(_errMsg, ERROR_EVENT);
				}
			}
			catch (...)
			{
			}

			try
			{
				if (FALSE == pSetup->GetFIR(_imgAcqPty.FIRFilters))
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"GetFIR from ThorDAQGalvoGalvoSettings.XML failed");
					LogMessage(_errMsg, ERROR_EVENT);
				}
			}
			catch (...)
			{
			}

			try
			{
				if (FALSE == pSetup->GetStimConfiguration(_stimActiveLoadCount))
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"GetStimConfiguration from ThorDAQGalvoGalvoSettings.XML failed");
					LogMessage(_errMsg, ERROR_EVENT);
				}
			}
			catch (...)
			{
			}

			try
			{
				if (FALSE == pSetup->GetRGGConfiguration(_rggMode))
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"GetRGGConfiguration from ThorDAQGalvoGalvoSettings.XML failed");
					LogMessage(_errMsg, ERROR_EVENT);
				}
			}
			catch (...)
			{
			}


			unique_ptr<ThorDAQIOXML> pSetupIO(new ThorDAQIOXML());
			bool dioConfigSuccess = true;
			try
			{
				if (FALSE == pSetupIO->GetDIOLinesConfiguration(_digitalIOSelection))
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"GetDIOLinesConfiguration from ThorDAQIOSettings.XML failed");
					LogMessage(_errMsg, ERROR_EVENT);
					dioConfigSuccess = false;
				}

				pSetupIO->ConfigDigitalLines(_digitalIOSelection, _tdqBOBType, DIOSettingsType::GG, _thorDAQImagingDigitalLinesConfig);
				pSetupIO->ConfigDigitalLines(_digitalIOSelection, _tdqBOBType, DIOSettingsType::STIM, _thorDAQStimDigitalLinesConfig);
			}
			catch (...)
			{
			}

			try
			{
				if (FALSE == pSetupIO->GetDIOSettings(_captureActiveInvert))
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"GetDIOSettings from ThorDAQIOSettings.XML failed");
					LogMessage(_errMsg, ERROR_EVENT);
					dioConfigSuccess = false;
				}
			}
			catch (...)
			{
			}

			ThordaqErrChk(L"ThorDAQAPISetDIOChannelSelection", retVal = ThorDAQAPISetDIOChannelSelection(_DAQDeviceIndex, _thorDAQImagingDigitalLinesConfig));

			ThordaqErrChk(L"ThorDAQAPISetScanActiveLineInvert", retVal = ThorDAQAPISetScanActiveLineInvert(_DAQDeviceIndex, TRUE == _captureActiveInvert));

			try
			{
				if (FALSE == pSetupIO->GetAOLinesConfiguration(_thordaqAOSelection))
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"GetAOLinesConfiguration from ThorDAQIOSettings.XML failed");
					LogMessage(_errMsg, ERROR_EVENT);
					dioConfigSuccess = false;
				}

				//assume that the primary GG is always configured and default, check if the secondary GG is available
				//we will use _secondaryGGAvailable to decide whether we do some operations and whether some options are ava
				_secondaryGGAvailable = (_thordaqAOSelection[AO::GG1_X] >= 0 && _thordaqAOSelection[AO::GG1_Y] >= 0);

				if (!_secondaryGGAvailable)
				{
					_imgAcqPty.selectedImagingGG = 0;
					_imgAcqPty.selectedStimGG = 0;
				}

				SetSelectedImagingAOs(_imgAcqPty.selectedImagingGG);
				SetSelectedStimAOs(_imgAcqPty.selectedStimGG);
			}
			catch (...)
			{
			}
		}
	}
	catch (...)
	{
		return FALSE;
	}
	return cameraCount;
}

/************************************************************************************************
* @fn	long CThorDAQGalvoGalvo::SelectCamera(const long camera)
*
* @brief	Select Camera.
*
* @param	camera	The camera index .
*
* @return	A long.
**************************************************************************************************/
long CThorDAQGalvoGalvo::SelectCamera(const long camera)
{
	if (_deviceNum == 0)// No thordaq is connected
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"The hardware has not been located");
		LogMessage(_errMsg, ERROR_EVENT);
		return FALSE;
	}

	//Load parameter from XML
	try
	{
		AlignDataLoadFile(); ///load the alignment data if exists;
	}
	catch (...)
	{
	}
	int32 error = 0, retVal = 0;
	ThordaqErrChk(L"ThorDAQAPISlowSmoothMoveToAndFromParkEnable", retVal = ThorDAQAPISlowSmoothMoveToAndFromParkEnable(_DAQDeviceIndex, true));
	//park the galvo

	MoveGalvoToParkPosition(_thordaqAOSelection[AO::GG0_X], _thordaqAOSelection[AO::GG0_Y]);
	MoveGalvoToParkPosition(_thordaqAOSelection[AO::GG1_X], _thordaqAOSelection[AO::GG1_Y]);
	//

	return TRUE;
}

/************************************************************************************************
* @fn	long CThorDAQGalvoGalvo::TeardownCamera()
*
* @brief	Teardown Camera.
*
* @return	A long.
**************************************************************************************************/
long CThorDAQGalvoGalvo::TeardownCamera()
{
	try
	{
		int32 error = 0, retVal = 0;
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorLSMCam::TeardownCamera");
		LogMessage(_errMsg, VERBOSE_EVENT);

		//Stop the acquisition thread
		ThordaqErrChk(L"ThorDAQAPIAbortRead", retVal = ThorDAQAPIAbortRead(_DAQDeviceIndex));
		SetEvent(_hStopAcquisition);

		if (_hExperimentThread)
		{
			//Wait for acqisition's stop
			Sleep(500);
			WaitForSingleObject(_hThreadStopped, 5000);
		}

		ThorDAQAPIStopDACWaveforms(_DAQDeviceIndex);
		//terminate daq thread:
		StopDaqBrd();
		//save the configuration settings

		ThordaqErrChk(L"ThorDAQAPISlowSmoothMoveToAndFromParkEnable", retVal = ThorDAQAPISlowSmoothMoveToAndFromParkEnable(_DAQDeviceIndex, true));

		//park the galvo when exiting
		MoveGalvoToParkPosition(_thordaqAOSelection[AO::GG0_X], _thordaqAOSelection[AO::GG0_Y]);
		MoveGalvoToParkPosition(_thordaqAOSelection[AO::GG1_X], _thordaqAOSelection[AO::GG1_Y]);

		//park the pockels cell
		MovePockelsToParkPosition(&_imgAcqPty.pockelPty);

		//release thor thordaq board
		ThorDAQAPIReleaseBoard(_DAQDeviceIndex);

		//close the shutter

		//disconnect DAQ board
		_deviceNum = 0;

		//// Free all the buffers
		SAFE_DELETE_PTR(_pFrmBuffer);

		if (_pHistoryBuf != NULL)
		{
			VirtualFree(_pHistoryBuf, 0, MEM_RELEASE);
			_pHistoryBuf = NULL;
		}

		if (_pHistoryProgressiveBuf != NULL)
		{
			VirtualFree(_pHistoryProgressiveBuf, 0, MEM_RELEASE);
			_pHistoryProgressiveBuf = NULL;
		}

		//// Delete all the pointers

		//// Clean all the threads
		SAFE_DELETE_HANDLE(_hExperimentThread);

		long flipVertDir = (_imgAcqPty.verticalScanDirection > 0) ? 0 : 1;

		double fsCal = 0;
		long vertScanDir = 0;
		long oneXFS = 0;
		double fineOffset[2] = { 0.0, 0.0 };
		double fineFieldSizeScaleX = 0;
		double fineFieldSizeScaleY = 0;
		double pockelsMinVoltage[MAX_POCKELS_CELL_COUNT];
		double pockelsMaxVoltage[MAX_POCKELS_CELL_COUNT];
		unique_ptr<ThorGalvoGalvoXML> pSetup(new ThorGalvoGalvoXML());

		//Get the values from the file since the fieldSize Calibration and oneXFieldSize may have been saved independently
		if (FALSE == pSetup->GetCalibration(fsCal, vertScanDir, fineOffset[0], fineOffset[1], fineFieldSizeScaleX, fineFieldSizeScaleY, oneXFS, pockelsMinVoltage, pockelsMaxVoltage))
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"GetCalibration from ThorDAQGalvoGalvoSettings failed");
			LogMessage(_errMsg, ERROR_EVENT);
		}

		if (FALSE == pSetup->SetCalibration(fsCal, flipVertDir, _imgAcqPty.fineOffsetX, _imgAcqPty.fineOffsetY, _imgAcqPty.fineFieldSizeScaleX, _imgAcqPty.fineFieldSizeScaleY, oneXFS, _imgAcqPty.pockelPty.pockelsMinVoltage, _imgAcqPty.pockelPty.pockelsMaxVoltage))
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"SetCalibration from ThorDAQGalvoGalvoSettings failed");
			LogMessage(_errMsg, ERROR_EVENT);
		}

		if (FALSE == pSetup->SetFIR(_imgAcqPty.FIRFilters))
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"SetFIR from ThorDAQGalvoGalvoSettings failed");
			LogMessage(_errMsg, ERROR_EVENT);
		}

		if(FALSE == pSetup->SetConfiguration(_imgAcqPty.pockelPty.pockelsDelayUS))
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"SetFIR from ThorDAQGalvoGalvoSettings failed");
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
	catch (...)
	{

	}

	return TRUE;
}


/************************************************************************************************
* @fn	long CThorDAQGalvoGalvo::PreflightAcquisition(char * pDataBuffer)
*
* @brief	Preflight position.
* @param [in,out]	pDataBuffer	  	Identifier of the Data buffer.
* @return	A long.
**************************************************************************************************/
long CThorDAQGalvoGalvo::PreflightAcquisition(char* pDataBuffer)
{
	long ret = TRUE;
	// reset dropped frame count:
	_droppedFramesCnt = 0;

	//#ifdef TEST
	//	_imgAcqPty.channel = 0xf; // Test force to run at all 4 channels
	//#endif

	_forceSettingsUpdate = TRUE;// Force to enable acquisition at first set 
	// verify the image size depends on the area mode
	switch (_imgAcqPty.areaMode)
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
		case ScanMode::CENTER:
		{

			MoveGalvoToCenter(_imagingActiveAOSelection[GG_AO::GG_X], _imagingActiveAOSelection[GG_AO::GG_Y], &_imgAcqPty);

			for (long i = 0; i < MAX_POCKELS_CELL_COUNT; i++)
			{
				MovePockelsToPowerLevel(i, &_imgAcqPty.pockelPty);
			}
			ret = TRUE;
			break;
		}
		case ScanMode::BLEACHING_SCAN://test the waveformBuffer
		{
			ret = TRUE;
			break;
		}
		default:
		{
			//if we are in imaging mode, then set the parking position
			//to the regular parking position
			MoveGalvoToParkPosition(_imagingActiveAOSelection[GG_AO::GG_X], _imagingActiveAOSelection[GG_AO::GG_Y]);
			MovePockelsToParkPosition(&_imgAcqPty.pockelPty);

			// For stimulus mode, reconfigure the buffers and acquisition parameters.
			if (ICamera::HW_MULTI_FRAME_TRIGGER_FIRST == _imgAcqPty_Pre.triggerMode)
			{
				int32 error = 0, retVal = 0;
				/**********reset the acquisition thread and the event flag*******/
				ThordaqErrChk(L"ThorDAQAPIAbortRead", retVal = ThorDAQAPIAbortRead(_DAQDeviceIndex));
				SetEvent(_hStopAcquisition);
				WaitForSingleObject(_hThreadStopped, INFINITE);

				ResetEvent(_hTriggerTimeout);//Reset the acquisition timeout event flag

				_imgAcqPty_Pre = _imgAcqPty; // Save the image acquisition struct
				_imgAcqPty_Pre.flybackCycle = GetFlybackCycle();

				if (ConfigAcqSettings(&_imgAcqPty_Pre) == FALSE)
				{
					ret = FALSE;
				}
			}

			ret = TRUE;
		}
	}

	return ret;
}

/************************************************************************************************
* @fn	long CThorDAQGalvoGalvo::SetupAcquisition(char * pDataBuffer)
*
* @brief	Setup  Acquisition.
* @param [in,out]	pDataBuffer	  	Identifier of the Data buffer.
* @return	A long.
**************************************************************************************************/
long CThorDAQGalvoGalvo::SetupAcquisition(char* pDataBuffer)
{
	int32 error = 0, retVal = 0;
	long ret = TRUE;

	//do not capture data if you are in the centering scan mode, Just Set the Scanner
	if (_imgAcqPty.scanMode == ScanMode::CENTER)
	{
		return ret;
	}

	//If settings change or system is forced to refresh updating, do update.
	if (!_imgAcqPty.KeyPropertiesAreEqual(_imgAcqPty_Pre) || TRUE == _pockelsMaskChanged || _forceSettingsUpdate == TRUE)
	{
		// Disable Force updating
		_forceSettingsUpdate = FALSE;

		_pockelsMaskChanged = FALSE;

		if (_currentlyImaging)
		{
			/**********reset the acquisition thread and the event flag*******/
			ThordaqErrChk(L"ThorDAQAPIAbortRead", retVal = ThorDAQAPIAbortRead(_DAQDeviceIndex));
		}
		SetEvent(_hStopAcquisition);
		if (_hExperimentThread)
		{
			if (WAIT_OBJECT_0 != WaitForSingleObject(_hThreadStopped, INFINITE))
			{
				StringCbPrintfW(_errMsg, _MAX_PATH, L"Failed to stop acquisition thread.");
				LogMessage(_errMsg, ERROR_EVENT);
			}
		}

		// verify the image size depends on the area mode
		switch (_imgAcqPty.areaMode)
		{
			case ICamera::SQUARE:
				_imgAcqPty.pixelY = _imgAcqPty.pixelX; break;
			case ICamera::LINE:
				_imgAcqPty.pixelY = 1; break;
			default:;
		}

		ResetEvent(_hTriggerTimeout);//Reset the acquisition timeout event flag

		_imgAcqPty_Pre = _imgAcqPty; // Save the image acquisition struct	


		if (_imgAcqPty_Pre.scanMode == ScanMode::BLEACHING_SCAN)
		{
			if (ConfigStimSettings(&_imgAcqPty_Pre) == FALSE)
			{
				ret = FALSE;
			}
		}
		else
		{
			if (ConfigAcqSettings(&_imgAcqPty_Pre) == FALSE)
			{
				ret = FALSE;
			}
		}
	}

	return ret;
}

/************************************************************************************************
* @fn	long CThorDAQGalvoGalvo::StartAcquisition(char * pDataBuffer)
*
* @brief	Start  Acquisition.
* @param [in,out]	pDataBuffer	  	Identifier of the Data buffer.
* @return	A long.
**************************************************************************************************/
long CThorDAQGalvoGalvo::StartAcquisition(char* pDataBuffer)
{
	int32 error = 0, retVal = 0;

	StringCbPrintfW(message, MSG_SIZE, L"CThorDAQGalvoGalvo::StartAcquisition");
	LogMessage(message, VERBOSE_EVENT);
	_bleachStatus = StatusType::STATUS_BUSY;

	//reset frame indexes
	_index_of_last_written_frame = -1;
	_index_of_last_read_frame = -1;

	//reset the frame buffer
	if (_pFrmBuffer != NULL)
	{
		_pFrmBuffer->Reset();
	}

	try
	{
		//do not capture data if you are in the centering scan mode
		if (_imgAcqPty_Pre.scanMode == CENTER)
		{
			return TRUE;
		}
		switch (_imgAcqPty_Pre.triggerMode)
		{
			case ICamera::SW_FREE_RUN_MODE: // Free Run
			case ICamera::SW_MULTI_FRAME:
			{
				if (WaitForSingleObject(_hThreadStopped, 0) == WAIT_OBJECT_0)
				{
					//make sure to reload the waveformBuffer for the first call of the thread.
					//This ensures the frame trigger is restarted for output
					if (STATUS_SUCCESSFUL != retVal)
					{
						return FALSE;
					}
					DWORD ThreadID;
					ResetEvent(_hStopAcquisition);

					SAFE_DELETE_HANDLE(_hExperimentThread);//close thread here since thread stop was signaled

					_hExperimentThread = CreateExperimentThread(ThreadID);
					return TRUE;
				}
			}
			break;
			case ICamera::SW_SINGLE_FRAME:
			{
				//TODO: this logic came from other cameras, and is likely not needed in thordaq cameras... test without it and remove if it works well
				//for single frame capture force the previous thread to stop and add an additional wait to ensure it completes.
				//if the previous thread does not close generate an error.
				//Stop and cancel previous scan
				SetEvent(_hStopAcquisition);

				if (WaitForSingleObject(_hThreadStopped, 1000) == WAIT_OBJECT_0)
				{
					if (STATUS_SUCCESSFUL != retVal)
					{
						return FALSE;
					}
					DWORD ThreadID;
					ResetEvent(_hStopAcquisition);

					SAFE_DELETE_HANDLE(_hExperimentThread);//close thread here since thread stop was signaled

					_hExperimentThread = CreateExperimentThread(ThreadID);
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
			case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
			case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
			{
				if (WaitForSingleObject(_hThreadStopped, 0) == WAIT_OBJECT_0)
				{
					//make sure to reload the waveformBuffer for the first call of the thread.

					if (STATUS_SUCCESSFUL != retVal)
					{
						return FALSE;
					}
					DWORD ThreadID;
					ResetEvent(_hStopAcquisition);

					SAFE_DELETE_HANDLE(_hExperimentThread);//close thread here since thread stop was signaled

					_hExperimentThread = CreateExperimentThread(ThreadID);

					//Wait for the hardware trigger event. This is for stimulus scanning, it has to wait here 
					// until it gets a stop event or a hardware trigger in event.
					long triggerWaitTimeoutMilliseconds = _triggerWaitTimeout * MS_TO_SEC;
					if (WaitForHardwareTrigger(triggerWaitTimeoutMilliseconds, TRUE))
					{
						return TRUE;
					}
					else
					{
						return FALSE;
					}
				}

			}
			break;
		}
	}
	catch (...)
	{
		//		StringCbPrintfW(message,MSG_SIZE,L"StartAcquisition failed.");
		//		LogMessage(message,ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

/************************************************************************************************
* @fn	HANDLE CThorDAQGalvoGalvo::CreateExperimentThread(DWORD &threadID)
*
* @brief	Create  Acquisition Thread.
* @param 	threadID	  	Acquisition Thread ID.
* @return	Thread Handle.
**************************************************************************************************/
HANDLE CThorDAQGalvoGalvo::CreateExperimentThread(DWORD& threadID)
{
	//DigiParams *dParamsOut = new DigiParams();
	//*dParamsOut = _digiParams;
	ResetEvent(_hThreadStopped);
	HANDLE handle;
	switch (_imgAcqPty_Pre.scanMode)
	{
		case ScanMode::BLEACHING_SCAN:
			handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) & (CThorDAQGalvoGalvo::StimProcess), (void*)this, 0, &threadID);
			break;
		default:
			handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) & (CThorDAQGalvoGalvo::StartFrameAcqProc), (void*)this, 0, &threadID);
			break;
	}

	if (NULL != handle)
	{
		SetThreadPriority(handle, THREAD_PRIORITY_HIGHEST);
	}

	return handle;
}

/// <summary> Waits on a hardware trigger. Will also stop with a timeout or a software stop acquisition event
///	For Thordaq the board is in charge of receiving the trigger or timing out. StartFrameAcqProc will dictate
/// if the board recerived a frame trigger in or not, once ReadChannel() returns.
/// </summary>
/// <param name="timeoutMillis"> Max time to wait </param>
/// <param name="showTimeoutMessage"> Whether to show a message box warning of a timeout </param>
/// <returns> Returns true if a hardware trigger stopped the wait, false otherwise </returns>
long CThorDAQGalvoGalvo::WaitForHardwareTrigger(long timeoutMillis, long showTimeoutMessageBox)
{
	//Setup Wait Event Array
	std::vector<HANDLE> hardwareEvents;
	hardwareEvents.push_back(_hHardwareTriggerInEvent);
	hardwareEvents.push_back(_hStopAcquisition);

	//Used to check if the hardware event was the cause of the wait stop
	auto it = std::find(hardwareEvents.begin(), hardwareEvents.end(), _hHardwareTriggerInEvent);
	int hardwareIndex = static_cast<int>(std::distance(hardwareEvents.begin(), it));

	//Wait for an event
	DWORD ret = WaitForMultipleObjects(static_cast<DWORD>(hardwareEvents.size()), hardwareEvents.data(), false, timeoutMillis);

	//Hardware Trigger
	if (ret - WAIT_OBJECT_0 == hardwareIndex)
	{
		return TRUE;
	}
	//Handle Other Causes
	else
	{
		//Timeout
		if (ret == WAIT_TIMEOUT)
		{
			if (TRUE == showTimeoutMessageBox)
			{
				StringCbPrintfW(_errMsg, _MAX_PATH, L"External trigger is timed out after %d seconds.", timeoutMillis / MS_TO_SEC);
				MessageBox(NULL, _errMsg, L"Trigger Error", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
			}
			//Log
			StringCbPrintfW(_errMsg, _MAX_PATH, L"StartAcquisition Hardware trigger timeout");
			LogMessage(_errMsg, ERROR_EVENT);
		}
		//Error
		else if (ret == WAIT_FAILED || ret == WAIT_ABANDONED)
		{
			//Log
			StringCbPrintfW(_errMsg, _MAX_PATH, L"While waiting for StartAcquisition Hardware trigger, WaitOnMultipleObjects failed with bad handle");
			LogMessage(_errMsg, ERROR_EVENT);
		}
		//Other event
		else
		{
			for (HANDLE hEvent : hardwareEvents)
			{
				ResetEvent(hEvent);
			}
		}
		return FALSE;
	}
}


void CThorDAQGalvoGalvo::StopDaqBrd()
{
	int32 error = 0, retVal = 0;;
	ThordaqErrChk(L"ThorDAQAPIStopAcquisition", retVal = ThorDAQAPIStopAcquisition(_DAQDeviceIndex));
}

long CThorDAQGalvoGalvo::StatusAcquisition(long& status)
{
	long ret = TRUE;
	//return FALSE to break out while loop if user changed params in free-run mode
	if ((ICamera::SW_FREE_RUN_MODE == _imgAcqPty_Pre.triggerMode) && (WAIT_OBJECT_0 == WaitForSingleObject(_hStopAcquisition, 0)))
	{
		status = ICamera::STATUS_READY;
		return FALSE;
	}

	switch (_imgAcqPty_Pre.scanMode)
	{
		case ScanMode::CENTER:
			status = ICamera::STATUS_READY;
			break;
		case ScanMode::BLEACHING_SCAN:
			status = _bleachStatus;
			break;
		default:
		{
			if (WaitForSingleObject(_hTriggerTimeout, 0) == WAIT_OBJECT_0)
			{
				status = ICamera::STATUS_ERROR;
				//ResetEvent(_hTriggerTimeout);
				return ret;
			}

			long long comparison_index = _index_of_last_written_frame;
			long long  lastReadIndex = _index_of_last_read_frame;

			if (lastReadIndex < comparison_index)
			{
				if (SW_FREE_RUN_MODE == _imgAcqPty.triggerMode)
				{
					status = _imageStatus;
				}
				else
				{
					status = ICamera::STATUS_READY;
				}
				//printf("_index_of_last_written_frame = %d; _index_of_last_read_frame = %d.\n", _index_of_last_written_frame, _index_of_last_read_frame);
				//StringCbPrintfW(message,MSG_SIZE,L"Status now is ready with frame buffer size of %d.\n", _index_of_last_written_frame - _index_of_last_read_frame);
				//LogMessage(message,VERBOSE_EVENT);
				if (comparison_index - lastReadIndex > _imgAcqPty.dmaBufferCount && _imgAcqPty.triggerMode != SW_FREE_RUN_MODE)
				{
					wchar_t errMsg[MSG_SIZE];
					StringCbPrintfW(errMsg, MSG_SIZE, L"CThordaqGalvoGalvo:StatusAcquition error, DMA overflow. Comparison index: %lld, Index of last read frame: %lld", comparison_index, lastReadIndex);
					CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg, ERROR_EVENT);
					status = ICamera::STATUS_ERROR;
				}
			}
			else
			{
				status = ICamera::STATUS_BUSY;
			}
		}
		break;
	}
	return ret;
}

long CThorDAQGalvoGalvo::StatusAcquisitionEx(long& status, long& indexOfLastCompletedFrame)
{
	long ret = TRUE;

	//do not capture data if you are in the centering scan mode
	if (_imgAcqPty_Pre.scanMode == CENTER)
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

long CThorDAQGalvoGalvo::CopyAcquisition(char* pDataBuffer, void* frameInfo)
{
	long ret = TRUE;

	//do not capture data if you are in the centering scan mode
	if (_imgAcqPty_Pre.scanMode == CENTER || BLEACHING_SCAN == _imgAcqPty_Pre.scanMode)
	{
		return ret;
	}

	long channelSizeByte = _imgAcqPty_Pre.pixelX * _imgAcqPty_Pre.pixelY * sizeof(USHORT); // frame size count in bytes
	size_t copyFrameSizeByte = (size_t)_imgAcqPty_Pre.pixelX * (size_t)_imgAcqPty_Pre.pixelY * sizeof(USHORT);
	int selectedChannelCount = CountChannelBits(_daqAcqCfg.imageCtrl.channel);

	size_t frameNum = 0;
	if (_index_of_last_read_frame == _index_of_last_written_frame)
	{
		return FALSE;
	}

	double averageCumulativeNum = (_imgAcqPty_Pre.averageMode == 0 || _imgAcqPty_Pre.lineAveragingEnable == TRUE) ? 1.0 : _imgAcqPty_Pre.averageNum;

	if (_pFrmBuffer->GetNextFrame(_pTempBuf))
	{
		++_index_of_last_read_frame;
	}
	else
	{
		return FALSE;
	}

	//order of data being copied:
	//C0P0,C0P1,C0P2...C0PN
	//C1P0,C1P1,C1P2...C1PN
	//...
	//CNP0,CNP1,CNP2...CNPN
	for (int i = 0, j = 0; i < MAX_CHANNEL_COUNT; i++)
	{
		for (int p = 0; p < _daqAcqCfg.imageCtrl.numPlanes; ++p)
		{
			// If save enabled channels only for raw is disabled, pDataBuffer will be the size of all channels (except in the 1 channel case)
			// we need to copy the image buffer in the corresponding space, whitout checking which channels are enabled/disabled.
			if (FALSE == _imgAcqPty_Pre.rawSaveEnabledChannelOnly && 1 != selectedChannelCount)
			{
				if ((_daqAcqCfg.imageCtrl.channel & (0x0001 << i)) != 0x0000)
				{
					memcpy(pDataBuffer + copyFrameSizeByte * _daqAcqCfg.imageCtrl.numPlanes * i + (size_t)p * copyFrameSizeByte, _pTempBuf + (size_t)channelSizeByte * j + (size_t)channelSizeByte * selectedChannelCount * p, copyFrameSizeByte);
				}
			}
			else
			{
				if ((_daqAcqCfg.imageCtrl.channel & (0x0001 << i)) != 0x0000)
				{
					memcpy(pDataBuffer + copyFrameSizeByte * _daqAcqCfg.imageCtrl.numPlanes * j + (size_t)p * copyFrameSizeByte, _pTempBuf + (size_t)channelSizeByte * j + (size_t)channelSizeByte * selectedChannelCount * p, copyFrameSizeByte);
				}
			}
		}
		if ((_daqAcqCfg.imageCtrl.channel & (0x0001 << i)) != 0x0000)
		{
			++j;
		}
	}

	//for debugging we are going to put planes 2 3 4 in channels B C D
	/*for (int p = 0; p < _daqAcqCfg.imageCtrl.numPlanes; ++p)
	{
		memcpy(pDataBuffer + copyFrameSizeByte * p, _pTempBuf + (size_t)channelSizeByte * selectedChannelCount * p, copyFrameSizeByte);
	}*/

	FrameInfo frameInfoStruct;
	memcpy(&frameInfoStruct, frameInfo, sizeof(FrameInfo));

	frameInfoStruct.fullFrame = TRUE;

	frameInfoStruct.channels = selectedChannelCount > 1 ? MAX_CHANNEL_COUNT : selectedChannelCount;
	if (selectedChannelCount > 0)
	{
		int numberOfCopiedChannels = FALSE == _imgAcqPty_Pre.rawSaveEnabledChannelOnly && 1 != selectedChannelCount ? MAX_CHANNEL_COUNT : selectedChannelCount;
		frameInfoStruct.copySize = _imgAcqPty_Pre.numberOfPlanes * copyFrameSizeByte * numberOfCopiedChannels;
	}
	frameInfoStruct.numberOfPlanes = _imgAcqPty_Pre.numberOfPlanes;

	frameInfoStruct.isNewMROIFrame = 1;
	frameInfoStruct.totalScanAreas = 1;
	frameInfoStruct.scanAreaIndex = 0;
	frameInfoStruct.scanAreaID = 0;
	frameInfoStruct.imageWidth = _imgAcqPty_Pre.pixelX;
	frameInfoStruct.imageHeight = _imgAcqPty_Pre.pixelY;
	frameInfoStruct.isMROI = FALSE;
	frameInfoStruct.fullImageWidth = (long)_imgAcqPty_Pre.pixelX;
	frameInfoStruct.fullImageHeight = (long)_imgAcqPty_Pre.pixelY;
	frameInfoStruct.topInFullImage = 0;
	frameInfoStruct.leftInFullImage = 0;
	frameInfoStruct.mROIStripeFieldSize = _imgAcqPty_Pre.fieldSize;

	memcpy(frameInfo, &frameInfoStruct, sizeof(FrameInfo));

	//generate pulse to signal frame buffer ready:
	SetFrameBufferReadyOutput();

	return ret;
}

long CThorDAQGalvoGalvo::PostflightAcquisition(char* pDataBuffer)
{
	try
	{
		int32 error = 0, retVal = 0;
		//do not capture data if you are in the centering scan mode
		if (_imgAcqPty_Pre.scanMode == CENTER)
		{
			MoveGalvoToParkPosition(_imagingActiveAOSelection[GG_AO::GG_X], _imagingActiveAOSelection[GG_AO::GG_Y]);
			//IDAQMovePockelsToPowerLevel();
			return TRUE;
		}
		//force the hardware trigger event if the post flight function is called
		SetEvent(_hHardwareTriggerInEvent);
		ResetEvent(_hHardwareTriggerInEvent);

		if (_currentlyImaging)
		{
			ThordaqErrChk(L"ThorDAQAPIAbortRead", retVal = ThorDAQAPIAbortRead(_DAQDeviceIndex));
		}
		else if (_currentlyStimulating)
		{
			ThordaqErrChk(L"ThorDAQAPIAbortRead", retVal = ThorDAQAPIStopDACWaveforms(_DAQDeviceIndex));
		}
		CThorDAQGalvoGalvo::GetInstance()->_stopStimulating = true;
		SetEvent(_hStopAcquisition);

		//park the pockels cell
		//IDAQMovePockelsToParkPosition();

		//for the trigger output to low
		//IDAQSetFrameTriggerOutLow();

		while (WaitForSingleObject(_hThreadStopped, 10000) != WAIT_OBJECT_0)
		{
			Sleep(10);
		}

		MoveGalvoToParkPosition(_imagingActiveAOSelection[GG_AO::GG_X], _imagingActiveAOSelection[GG_AO::GG_Y]);
		MovePockelsToParkPosition(&_imgAcqPty.pockelPty);


		ThordaqErrChk(L"ThorDAQAPIDACBankSwitchingClearRegisteredReadyForNextImageWaveformsEventHandlers", retVal = ThorDAQAPIDACBankSwitchingClearRegisteredReadyForNextImageWaveformsEventHandlers(_DAQDeviceIndex));
		
		//reset params:
		_precaptureStatus = PreCaptureStatus::PRECAPTURE_DONE;
	}
	catch (...)
	{
		return FALSE;
	}
	return TRUE;
}
long CThorDAQGalvoGalvo::GetLastErrorMsg(wchar_t* msg, long size)
{
	long ret = TRUE;
	return ret;
}

void CThorDAQGalvoGalvo::LogMessage(wchar_t* logMsg, long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

void CThorDAQGalvoGalvo::FindStartLocation(double& xGalvoParkPosition, double& yGalvoParkPosition, ImgAcqPty* pImgAcqPty)
{
	ScanLineStruct scanLine = ScanLineStruct();
	double linetime = 0;
	double dac_rate = 0;
	bool oneWayLineScan = false;////TODO: test without this and remove if it works well pImgAcqPty->pixelY == 1 && TWO_WAY_SCAN_MODE != pImgAcqPty->scanMode;
	GalvoStruct galvo_x_control;
	GalvoStruct galvo_y_control;
	double theta = (double)pImgAcqPty->fieldSize * _field2Theta;
	galvo_x_control.amplitude = theta * _theta2Volts * pImgAcqPty->fineFieldSizeScaleX; // horizontal galvo amplitude
	galvo_y_control.amplitude = theta * _theta2Volts * (double)pImgAcqPty->pixelY / (double)pImgAcqPty->pixelX;
	galvo_y_control.amplitude = (pImgAcqPty->yAmplitudeScaler / 100.0) * galvo_y_control.amplitude * pImgAcqPty->fineFieldSizeScaleY; // Vertical galvo amplitude
	galvo_x_control.offset = ((double)pImgAcqPty->offsetX * _field2Theta) * _theta2Volts + pImgAcqPty->fineOffsetX; //horizontal galvo offset
	galvo_y_control.offset = ((double)pImgAcqPty->verticalScanDirection * pImgAcqPty->offsetY * _field2Theta) * _theta2Volts + pImgAcqPty->fineOffsetY;// Vertical galvo offset
	galvo_y_control.scan_direction = pImgAcqPty->verticalScanDirection == 1 ? SCAN_DIRECTION::FORWARD_SC : SCAN_DIRECTION::REVERSE_SC;
	double yDirection = (SCAN_DIRECTION::FORWARD_SC == galvo_y_control.scan_direction) ? 1.0 : -1.0;
	bool useFastOneway = false;
	GetDACSamplesPerLine(&scanLine, pImgAcqPty, dac_rate, pImgAcqPty->dwellTime / (double)US_TO_S, linetime, oneWayLineScan, useFastOneway);

	double offset_y = galvo_y_control.offset - yDirection * galvo_y_control.amplitude / 2.0;
	offset_y = offset_y > 0 ? min(GALVO_PARK_POSITION, offset_y) : max(-GALVO_PARK_POSITION, offset_y);

	double offset_x = galvo_x_control.offset - (galvo_x_control.amplitude / (double)scanLine.samples_scan * (double)scanLine.samples_idle / M_PI_2) - galvo_x_control.amplitude / 2.0;
	offset_x = offset_x > 0 ? min(GALVO_PARK_POSITION, offset_x) : max(-GALVO_PARK_POSITION, offset_x);

	xGalvoParkPosition = offset_x;
	yGalvoParkPosition = offset_y;
}

long CThorDAQGalvoGalvo::MoveGalvoToParkPosition(int Galvo_X_channel, int Galvo_Y_channel)
{
	long ret = FALSE;
	int32 error = 0, retVal = STATUS_SUCCESSFUL;
	double xGalvoParkPosition = -GALVO_PARK_POSITION;
	double yGalvoParkPosition = 1 == _imgAcqPty.verticalScanDirection ? -GALVO_PARK_POSITION : GALVO_PARK_POSITION;

	if (TRUE == _galvoParkAtStart)
	{
		FindStartLocation(xGalvoParkPosition, yGalvoParkPosition, &_imgAcqPty);
	}
	ThordaqErrChk(L"ThorDAQAPISlowSmoothMoveToAndFromParkEnable", retVal = ThorDAQAPISlowSmoothMoveToAndFromParkEnable(_DAQDeviceIndex, true));
	if (_enableGalvoXPark && Galvo_X_channel >=0)
	{
		ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, Galvo_X_channel, xGalvoParkPosition));
	}

	// ThorConfocalGalvo sends Galvos to opposite sides, X goes to 10 and Y goes to -10
	ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, Galvo_Y_channel, yGalvoParkPosition));
	ret = retVal == STATUS_SUCCESSFUL;

	return ret;
}

long CThorDAQGalvoGalvo::MoveDACChannelToPosition(int dacChannel, double position)
{
	long ret = FALSE;
	int32 error = 0, retVal = 0;
	ThordaqErrChk(L"ThorDAQAPISlowSmoothMoveToAndFromParkEnable", retVal = ThorDAQAPISlowSmoothMoveToAndFromParkEnable(_DAQDeviceIndex, true));
	ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, dacChannel, position));
	ret = retVal == STATUS_SUCCESSFUL;

	return ret;
}

long CThorDAQGalvoGalvo::MoveGalvoToCenter(int Galvo_X_channel, int Galvo_Y_channel, ImgAcqPty* pImgAcqPty)
{
	long ret = FALSE;

	int32 error = 0, retVal = 0;

	double center_x = pImgAcqPty->offsetX * _field2Theta * _theta2Volts + pImgAcqPty->fineOffsetX;
	double center_y = pImgAcqPty->verticalScanDirection * pImgAcqPty->offsetY * _field2Theta * _theta2Volts + pImgAcqPty->fineOffsetY;
	ThordaqErrChk(L"ThorDAQAPISlowSmoothMoveToAndFromParkEnable", retVal = ThorDAQAPISlowSmoothMoveToAndFromParkEnable(_DAQDeviceIndex, true));
	ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, Galvo_X_channel, center_x));
	if (retVal == STATUS_SUCCESSFUL)
	{
		ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, Galvo_Y_channel, center_y));
		ret = retVal == STATUS_SUCCESSFUL;
	}
	return ret;
}

long CThorDAQGalvoGalvo::MovePockelsToParkPosition(PockelPty* pockelPty)
{
	int32 error = 0, retVal = 0;
	long ret = 0;

	for (int i = 0; i < MAX_POCKELS_CELL_COUNT; i++)
	{
		if (_pockelsEnable[i] == FALSE)
		{
			continue;
		}

		long pockelOutputChannel = _imagingActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + i)];

		double pockelsSetVal0 = pockelPty->pockelsMinVoltage[i] + (pockelPty->pockelsMaxVoltage[i] - pockelPty->pockelsMinVoltage[i]) * pockelPty->pockelsPowerLevel[i];
		double park_value = (TRUE == _pockelsParkAtMinimum) ? pockelPty->pockelsMinVoltage[i] : pockelsSetVal0;


		ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, pockelOutputChannel, park_value));
		if (retVal != STATUS_SUCCESSFUL)
		{
			ret = FALSE;
		}
	}

	return ret;
}


long CThorDAQGalvoGalvo::MovePockelsToPowerLevel(long index, PockelPty* pockelPty)
{
	int32 error = 0, retVal = 0;
	long ret = TRUE;

	long pockelOutputChannel = _imagingActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + index)];

	double pockelsOnVoltage = pockelPty->pockelsMinVoltage[index] + (pockelPty->pockelsMaxVoltage[index] - pockelPty->pockelsMinVoltage[index]) * pockelPty->pockelsPowerLevel[index];
	ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, pockelOutputChannel, pockelsOnVoltage));
	if (retVal != STATUS_SUCCESSFUL)
	{
		ret = FALSE;
	}

	return ret;
}

long CThorDAQGalvoGalvo::FindPockelsMinMax(long index, PockelPty* pockelPty)
{
	int32 retVal = 0, error = 0;

	if (!_pockelsEnable[index])
	{
		return retVal;
	}

	try
	{
		DAQmxErrChk(L"DAQmxCreateTask", DAQmxCreateTask("", &_taskHandleAIPockels[index]));
		DAQmxErrChk(L"DAQmxCreateAIVoltageChan", DAQmxCreateAIVoltageChan(_taskHandleAIPockels[index], _pockelsPowerInputLine[index].c_str(), "", DAQmx_Val_Cfg_Default, 0, 10.0, DAQmx_Val_Volts, NULL));

		const float64 VOLTAGE_START = _pockelsScanVoltageStart[index];
		float64 pockelsPos = VOLTAGE_START;
		const float64 VOLTAGE_RANGE = _pockelsScanVoltageStop[index] - _pockelsScanVoltageStart[index];

		long pockelOutputChannel = _imagingActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + index)];

		for (long i = 0; i < POCKELS_VOLTAGE_STEPS; i++)
		{	
			ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, pockelOutputChannel, pockelsPos));

			Sleep(30);

			int32 numRead;
			DAQmxErrChk(L"DAQmxReadAnalogF64", DAQmxReadAnalogF64(_taskHandleAIPockels[index], 1, 10.0, DAQmx_Val_GroupByChannel, &_pockelsReadArray[index][i], 1, &numRead, NULL));
			DAQmxErrChk(L"DAQmxWaitUntilTaskDone", DAQmxWaitUntilTaskDone(_taskHandleAIPockels[index], MAX_TASK_WAIT_TIME));

			Sleep(1);

			pockelsPos += VOLTAGE_RANGE / POCKELS_VOLTAGE_STEPS;
		}

		//move back to the start position after the scan
		pockelsPos = VOLTAGE_START;
		ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, pockelOutputChannel, pockelsPos));


		float64* pSmoothBuffer = new float64[POCKELS_VOLTAGE_STEPS];

		std::memcpy(pSmoothBuffer, &_pockelsReadArray[index], POCKELS_VOLTAGE_STEPS * sizeof(float64));

		//smooth the data and ignore the ends
		const long KERNEL_SIZE = 5;
		const long KERNEL_SKIP = 2;

		for (long n = 0; n < 5; n++)
		{
			for (long i = KERNEL_SKIP; i < POCKELS_VOLTAGE_STEPS - KERNEL_SKIP; i++)
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
				float64 kernel[KERNEL_SIZE] = { .06,.24,.4,.24,.06 };

				for (long k = 0, j = -1 * (KERNEL_SIZE >> 1); j <= (KERNEL_SIZE >> 1); j++, k++)
				{
					sum += kernel[k] * pSmoothBuffer[i + j];
				}

				pSmoothBuffer[i] = sum;
			}
		}

		double arrayMinVal = 10.0;
		double arrayMaxVal = -10.0;

		//locat the min and max for the dataset
		for (long i = KERNEL_SKIP + 1; i < POCKELS_VOLTAGE_STEPS - KERNEL_SKIP - 1; i++)
		{
			if (pSmoothBuffer[i] < arrayMinVal)
			{
				arrayMinVal = pSmoothBuffer[i];
			}

			if (pSmoothBuffer[i] > arrayMaxVal)
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

		const float64 PEAK_THRESHOLD_VOLTS = (arrayMaxVal - arrayMinVal) * .001;
		const float64 DIFFERENCE_THRESHOLD_VOLTS = (arrayMaxVal - arrayMinVal) * .30;
		const float64 MAX_MIDVALUE_DIFFERENCE = (arrayMaxVal - arrayMinVal) * 0.01;
		const double MID_VALUE = (arrayMaxVal + arrayMinVal) / 2;

		//find the midPoint location
		for (long i = KERNEL_SKIP + 2; i < POCKELS_VOLTAGE_STEPS - KERNEL_SKIP - 2; i++)
		{
			if (MAX_MIDVALUE_DIFFERENCE > MID_VALUE - pSmoothBuffer[i])
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
			for (int i = midLoc; i >= KERNEL_SKIP + 2; i--)
			{
				double x1 = i - 1;
				double x2 = i;

				double y1 = pSmoothBuffer[i - 1];
				double y2 = pSmoothBuffer[i];

				//calculate the slope
				double m = SLOPE_THRESHOLD;
				if (x2 != x1)
				{
					m = (y2 - y1) / (x2 - x1);
				}
				if (SLOPE_THRESHOLD > abs(m))
				{
					minLoc = i;
					minVal = _pockelsReadArray[index][i];
					break;
				}
			}

			//find the maxVal and maxLoc location
			for (int i = midLoc; i < POCKELS_VOLTAGE_STEPS - KERNEL_SKIP - 2; i++)
			{
				double x1 = i - 1;
				double x2 = i;

				double y1 = pSmoothBuffer[i - 1];
				double y2 = pSmoothBuffer[i];

				//calculate the slope
				double m = SLOPE_THRESHOLD;
				if (x2 != x1)
				{
					m = (y2 - y1) / (x2 - x1);
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

		if ((minLoc != -1) && (maxLoc != -1) && ((maxVal - minVal) > DIFFERENCE_THRESHOLD_VOLTS))
		{
			pockelPty->pockelsMinVoltage[index] = VOLTAGE_START + minLoc * VOLTAGE_RANGE / POCKELS_VOLTAGE_STEPS;
			pockelPty->pockelsMaxVoltage[index] = VOLTAGE_START + maxLoc * VOLTAGE_RANGE / POCKELS_VOLTAGE_STEPS;
		}
		else
		{
			retVal = -1;
		}

		//remove the line below to show the raw data in the plot
		std::memcpy(&_pockelsReadArray[index], pSmoothBuffer, POCKELS_VOLTAGE_STEPS * sizeof(float64));

		SAFE_DELETE_ARRAY(pSmoothBuffer);
	}
	catch (...)
	{
		memset(&_pockelsReadArray[index], 0, POCKELS_VOLTAGE_STEPS * sizeof(float64));

		retVal = -1;
	}
	return retVal;
}

/// <summary> Cancels any hardware triggers the camera is currently waiting on </summary>
void CThorDAQGalvoGalvo::StopHardwareWaits()
{
	int32 error = 0;
	ThordaqErrChk(L"ThorDAQAPIAbortRead", ThorDAQAPIAbortRead(_DAQDeviceIndex));
	SetEvent(_hStopAcquisition);
}

long CThorDAQGalvoGalvo::UpdateDACMemorySettings(IMAGING_BUFFER_STRUCT& DACMemorySettings)
{
	THORDAQ_STATUS status = STATUS_WRITE_BUFFER_ERROR;
	ULONG64 start_address = 0;
	ULONG64 length = 0;
	int32 error = 0, retVal = 0;
	//if (_gPtrWaveformMemoryPool->GetMemoryPropertyByChannel(DACMemorySettings.channel, start_address, length))
	{
		ThordaqErrChk(L"ThorDAQAPIPacketWriteBuffer", retVal = ThorDAQAPIPacketWriteBuffer(_DAQDeviceIndex, start_address + DACMemorySettings.offset, static_cast<ULONG>(DACMemorySettings.length), DACMemorySettings.buffer, 0xffffffff));
		if ((DACMemorySettings.length <= length) && (retVal == STATUS_SUCCESSFUL)) // Write configuration to the RAM
		{
			status = STATUS_SUCCESSFUL;
		}
	}
	return status;
}

void CThorDAQGalvoGalvo::TerminateNIDAQTask(TaskHandle& handle)
{
	if (NULL != handle)
	{
		DAQmxStopTask(handle);
		DAQmxClearTask(handle);
		handle = NULL;
	}
}

long CThorDAQGalvoGalvo::SetSelectedImagingAOs(long selectedImagingGG)
{
	if (0 == selectedImagingGG)
	{
		_imagingActiveAOSelection[GG_AO::GG_X] = _thordaqAOSelection[AO::GG0_X];
		_imagingActiveAOSelection[GG_AO::GG_Y] = _thordaqAOSelection[AO::GG0_Y];

		for (int i = 0; i < MAX_POCKELS_CELL_COUNT; ++i)
		{
			_imagingActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + i)] = _thordaqAOSelection[(AO)((int)AO::GG0_P0 + i)];
		}
	}
	else if (1 == selectedImagingGG)
	{
		_imagingActiveAOSelection[GG_AO::GG_X] = _thordaqAOSelection[AO::GG1_X];
		_imagingActiveAOSelection[GG_AO::GG_Y] = _thordaqAOSelection[AO::GG1_Y];

		for (int i = 0; i < MAX_POCKELS_CELL_COUNT; ++i)
		{
			_imagingActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + i)] = _thordaqAOSelection[(AO)((int)AO::GG1_P0 + i)];
		}
	}

	return TRUE;
}

long CThorDAQGalvoGalvo::SetSelectedStimAOs(long selectedImagingGG)
{
	if (0 == selectedImagingGG)
	{
		_stimActiveAOSelection[GG_AO::GG_X] = _thordaqAOSelection[AO::GG0_X];
		_stimActiveAOSelection[GG_AO::GG_Y] = _thordaqAOSelection[AO::GG0_Y];
		for (int i = 0; i < MAX_POCKELS_CELL_COUNT; ++i)
		{
			_stimActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + i)] = _thordaqAOSelection[(AO)((int)AO::GG0_P0 + i)];
		}
	}
	else if (1 == selectedImagingGG)
	{
		_stimActiveAOSelection[GG_AO::GG_X] = _thordaqAOSelection[AO::GG1_X];
		_stimActiveAOSelection[GG_AO::GG_Y] = _thordaqAOSelection[AO::GG1_Y];
		for (int i = 0; i < MAX_POCKELS_CELL_COUNT; ++i)
		{
			_stimActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + i)] = _thordaqAOSelection[(AO)((int)AO::GG1_P0 + i)];
		}
	}

	return TRUE;
}