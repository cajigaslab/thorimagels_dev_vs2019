// ThorDAQGalvoGalvo.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "thordaqResonantGalvo.h"
#include "thordaqResonantGalvoSetupXML.h"
#include "../ThorDAQIOXML.h"
#include "../../../../Devices/ThorDAQZ/ThorDAQZ/ThorDAQZXML.h"

#ifdef LOGGING_ENABLED
#include "..\..\..\..\..\Common\Log.h"
std::auto_ptr<LogDll> qlogDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

unique_ptr<IPPIDll> ippiDll(new IPPIDll(L".\\ippiu8-7.0.dll"));

wchar_t message[MSG_SIZE];

#define DAQmxErrChk(fnName,fnCall) if ((error=(fnCall)) < 0){ StringCbPrintfW(message,MSG_SIZE,L"%s GR failed (%d),(%d). ",fnName,error,__LINE__); CThordaqResonantGalvo::GetInstance()->LogMessage(message,ERROR_EVENT); throw "fnCall";}else{ StringCbPrintfW(message,MSG_SIZE,L"%s GR failed (%d) (%d). ",fnName,error,__LINE__); CThordaqResonantGalvo::GetInstance()->LogMessage(message,VERBOSE_EVENT);}
#define MAX_TASK_WAIT_TIME 10.0

/************************************************************************************************
* @fn	CThordaqResonantGalvo::CThordaqResonantGalvo()
*
* @brief	Default constructor.
*
**************************************************************************************************/
CThordaqResonantGalvo::CThordaqResonantGalvo()
{
	_deviceNum = 0;
	_crsFrequencyHighPrecision = 7990;//7931.47208;
	_field2Theta = 0.0901639344; //the field to scan angle conversion adapted from resonant galvo scanner code
	_theta2Volts = 1.0;
	_frameRate = 0;
	_oneXFieldSize = 160;
	_forceSettingsUpdate = FALSE;
	_triggerWaitTimeout = DEFAULT_TRIGGER_TIMEOUT;
	_pFrmBuffer = NULL;
	SAFE_DELETE_PTR(_pDataProcessor);
	_rawFlybackCycle = DEFAULT_FLYBACK_CYCLE;
	_fieldSizeCalibration = 0;
	_fieldSizeCalibrationXMLvalue = 0;
	_fieldSizeCalibrationAvailable = FALSE;
	
	_useZoomArray = FALSE;
	_scannerInitMode = FALSE;
	_fieldSizeMin = MIN_FIELD_SIZE_X;
	_fieldSizeMax = MAX_FIELD_SIZE_X;
	_current_resonant_scanner_frequency = CrsFrequencyHighPrecision;
	_pockelsParkAtMinimum = 0;
	_pockelsMaskChanged = FALSE;
	_pockelsTurnAroundBlank = TRUE;
	_pockelsFlybackBlank = TRUE;
	_hardwareTestModeEnable = FALSE;
	_pTempBuf = std::vector<ProcessedFrame*>();
	_pTempBufCorrection = std::vector<ProcessedFrame*>();
	_rGGMode = FALSE;

	_rotationAnglePosition = ICamera::LSMGRRotationAngle::DEG_0;
	_offsetVal = 0.0;
	_captureActiveInvert = FALSE;
	_preMoveGalvoToStartPosition = 0;
	_lastAreaCopiedIndex = -1;

	_saveCrsFrequencyToLog = FALSE;
	_waveformUpdateRateSPS = DAC_MAX_UPDATERATE;
	for(long i=0; i<MAX_POCKELS_CELL_COUNT; i++)
	{
		_pockelsEnable[i] = false;
		_pockelsLine[i] = "";///<NI connection string for pockel(s) // TODO: need to eliminate the use of this, we don't use NI for pockelsLine
		_taskHandleAIPockels[i] = 0;
		_pockelsScanVoltageStart[i] = 0.0;
		_pockelsScanVoltageStop[i] = 0.0;
		_pockelsPowerInputLine[i] = "";		
		_pockelsResponseType[i] = 0;
		_pPockelsMask[i] = nullptr;
		_pockelsMaskSize[i] = 0;
		memset(&_pockelsReadArray[i],0,POCKELS_VOLTAGE_STEPS * sizeof(float64));
	}

	for(long i=0; i<MAX_CHANNEL_COUNT; i++)
	{
		_preDcOffset[i] = 0;
	}

	_thordaqAOSelection = std::map<AO, long>();

	_zPos_min = 0.0;
	_zPos_max = 0.0;
	_zVolts2mm = 1.0;
	_zOffsetmm = 0.0;
	_zTypicalMoveSettlingTime_sec = 0.00001;
	_zTypicalMove_mm = 0.01;

	S2MMdmaBuffer = ThorDAQDMAbuffer(MAX_PIXEL_X * MAX_PIXEL_Y * sizeof(USHORT) * MAX_CHANNEL_COUNT);
	SetupDataMaps();
	_mROIScan = NULL;
	_mROIExpLoader.reset(new mROIExperimentLoader);
	_scannerType = 0;
	_totalLinesFormROI = 0;
	_minFlybackCyclesFactor = 1.0;
	_tdqBOBType = THORDAQ_BOB_TYPE::TDQ_3U_BOB;

	_pTempDoubleBuffCorrection = nullptr;

	_imageDistortionCorrectionPixelData = std::vector<PixelData>();

	_scanLensFocalLength = 50.0;
}
/************************************************************************************************
* @fn	CThordaqResonantGalvo::~CThordaqResonantGalvo()
*
* @brief	Destructor.
*
**************************************************************************************************/
CThordaqResonantGalvo::~CThordaqResonantGalvo()
{
	//Reset the device parameter
	_deviceNum = 0;
	_single.release();
	_instanceFlag = false;
}

///******	Initialize Static Members		******///
bool CThordaqResonantGalvo::_instanceFlag = false;
ImgAcqPty CThordaqResonantGalvo::_imgAcqPty = ImgAcqPty();
ImgAcqPty CThordaqResonantGalvo::_imgAcqPty_Pre = ImgAcqPty();


IMAGING_CONFIGURATION_STRUCT CThordaqResonantGalvo::_daqAcqCfg = IMAGING_CONFIGURATION_STRUCT(); 

auto_ptr<CThordaqResonantGalvo> CThordaqResonantGalvo::_single(new CThordaqResonantGalvo());//Instantiated on first use
HANDLE CThordaqResonantGalvo::_hStopAcquisition = CreateEvent(NULL, TRUE, FALSE, NULL);	//make sure the reset option is true (MANUAL RESET)
HANDLE CThordaqResonantGalvo::_hThreadStopped   = CreateEvent(NULL, TRUE, TRUE, NULL);
HANDLE CThordaqResonantGalvo::_hTriggerTimeout  = CreateEvent(NULL, FALSE, FALSE, NULL);
HANDLE CThordaqResonantGalvo::_hHardwareTriggerInEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
HANDLE CThordaqResonantGalvo::_hAcquisitionThread = NULL; // Main Acquisition thread
double CThordaqResonantGalvo::_crsFrequencyHighPrecision = 7990;

atomic<long long> CThordaqResonantGalvo::_indexOfLastCompletedFrame = 0;
atomic<long long> CThordaqResonantGalvo::_indexOfLastCopiedFrame = 0;

long CThordaqResonantGalvo::_DAQDeviceIndex = DEFAULT_CARD_NUMBER;
ICircularBuffer* CThordaqResonantGalvo::_pFrmBuffer = nullptr;
IDataProcessor* CThordaqResonantGalvo::_pDataProcessor = nullptr;

UCHAR* CThordaqResonantGalvo::_BufferContiguousArray[] = { NULL, NULL, NULL, NULL };
std::vector<ProcessedFrame*> CThordaqResonantGalvo::_pHistoryBuf = std::vector<ProcessedFrame*>();

long CThordaqResonantGalvo::_triggerWaitTimeout = 0;
CThordaqResonantGalvo::ScanStruct  CThordaqResonantGalvo::_scan_info = ScanStruct();
CThordaqResonantGalvo::ScanLineStruct  CThordaqResonantGalvo::_scanLine = ScanLineStruct();
UINT64 CThordaqResonantGalvo::_powerRampCurrentIndex = 0;
UINT64 CThordaqResonantGalvo::_fastZCurrentIndex = 0;
UINT64 CThordaqResonantGalvo::_twoBankFrameIndex = 0;
std::vector<CThordaqResonantGalvo::PockelsImagePowerRampStruct> CThordaqResonantGalvo::_pockelsImagePowerRampVector[MAX_POCKELS_CELL_COUNT];
std::vector<double> CThordaqResonantGalvo::_pockelsResponsePowerLevels[MAX_POCKELS_CELL_COUNT];
wchar_t CThordaqResonantGalvo::thordaqLogMessage[MSG_SIZE];

/************************************************************************************************
* @fn	CThordaqResonantGalvo* CThordaqResonantGalvo::GetInstance()
*
* @brief	Gets the instance. Singlton design pattern
*
* @return	null if it fails, else the instance.
**************************************************************************************************/
CThordaqResonantGalvo* CThordaqResonantGalvo::GetInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new CThordaqResonantGalvo());
		_instanceFlag = true;
		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

/************************************************************************************************
* @fn	long CThordaqResonantGalvo::FindCameras(long &cameraCount)
*
* @brief	Searches for the devices.
*
* @param [in,out]	cameraCount	Index of Camera.
*
* @return	False if no Camera found. True if Camera found.
**************************************************************************************************/
long CThordaqResonantGalvo::FindCameras(long& cameraCount)
{
	int32 error = 0, retVal = 0;
	StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorLSMCam::FindCameras");
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
			cameraCount = 1;
			_deviceNum = 1;
		}

		ThordaqErrChk(L"ThorDAQAPIGetBOBType", retVal = ThorDAQAPIGetBOBType(_DAQDeviceIndex, _tdqBOBType));
	}
	catch (...)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Hardware communication error ThorDAQ ConnectToBoard failed");
		LogMessage(_errMsg, ERROR_EVENT);
		return FALSE;
	}

	_numNIDAQ = 0;
	//need to do detailed device search here in the future.
	try
	{
		char DevName[256];
		DAQmxGetSysDevNames(DevName, 256);
		std::string temp = DevName;
		if (string::npos != temp.find("Dev1")) // (G/R: Dev1)
		{
			_numNIDAQ = 1;
		}
	}
	catch (...)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"Hardware communication error DAQmxGetSysDevNames failed");
		LogMessage(_errMsg, ERROR_EVENT);
	}

	unique_ptr<ThorDAQIOXML> pSetupIO(new ThorDAQIOXML());
	bool ioConfigSuccess = true;
	try
	{
		if (FALSE == pSetupIO->GetDIOLinesConfiguration(_digitalIOSelection))
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"GetDIOLinesConfiguration from ThorDAQIOSettings.XML failed");
			LogMessage(_errMsg, ERROR_EVENT);
			ioConfigSuccess = false;
		}
		pSetupIO->ConfigDigitalLines(_digitalIOSelection, _tdqBOBType, DIOSettingsType::GR, _thorDAQDigitalLinesConfig);
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
			ioConfigSuccess = false;
		}
	}
	catch (...)
	{
	}

	ThordaqErrChk(L"ThorDAQAPISetDIOChannelSelection", retVal = ThorDAQAPISetDIOChannelSelection(_DAQDeviceIndex, _thorDAQDigitalLinesConfig));

	ThordaqErrChk(L"ThorDAQAPISetScanActiveLineInvert", retVal = ThorDAQAPISetScanActiveLineInvert(_DAQDeviceIndex, TRUE == _captureActiveInvert));

	auto_ptr<ThordaqResonantGalvoXML> pSetup(new ThordaqResonantGalvoXML());

	//XML settings retrieval functions will throw an exception if tags or attributes are missing
	//catch each independetly so that as many tags as possible can be read
	if (FALSE == pSetup->GetConfiguration(_field2Theta, _crsFrequencyHighPrecision, _saveCrsFrequencyToLog, _pockelsParkAtMinimum, _fieldSizeMin, _fieldSizeMax, _pockelsTurnAroundBlank, _pockelsFlybackBlank, _rGGMode, _rotationAnglePosition, _preMoveGalvoToStartPosition, _waveformUpdateRateSPS, _scannerType))
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"GetConfiguration from ThorDAQResonantGalvoSettings.XML failed");
		LogMessage(_errMsg, ERROR_EVENT);
	}

	try
	{
		if (FALSE == pSetupIO->GetAOLinesConfiguration(_thordaqAOSelection))
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"GetAOLinesConfiguration from ThorDAQIOSettings.XML failed");
			LogMessage(_errMsg, ERROR_EVENT);
			ioConfigSuccess = false;
		}
	}
	catch (...)
	{
	}

	if (_waveformUpdateRateSPS > DAC_MAX_UPDATERATE || _waveformUpdateRateSPS < DAC_MIN_UPDATERATE)
	{
		_waveformUpdateRateSPS = DAC_MAX_UPDATERATE;
	}


	if (FALSE == pSetup->GetCalibration(_fieldSizeCalibrationXMLvalue, _oneXFieldSize, _imgAcqPty.pockelPty.pockelsDelayUS, _imgAcqPty.pockelsBlankingPhaseShiftPercent, _imgAcqPty.preImagingCalibrationCycles, _imgAcqPty.imagingRampExtensionCycles, _minFlybackCyclesFactor, _imgAcqPty.pockelPty.pockelsMinVoltage, _imgAcqPty.pockelPty.pockelsMaxVoltage, _scanLensFocalLength, _imgAcqPty.ImageDistortionCorrectionCalibrationXAngleMax, _imgAcqPty.ImageDistortionCorrectionCalibrationYAngleMax, _imgAcqPty.ImageDistortionCorrectionCalibrationGalvoTiltAngle))
	{
		//if fieldSizeCalibration not exists, set its GetParamInfo Available
		_fieldSizeCalibrationAvailable = FALSE;

		StringCbPrintfW(_errMsg, MSG_SIZE, L"GetCalibration from ThorDAQResonantGalvoSettings.XML failed. FieldSizeCalibration not available.");
		LogMessage(_errMsg, ERROR_EVENT);
	}
	else
	{
		_fieldSizeCalibrationAvailable = TRUE;
	}

	if (_minFlybackCyclesFactor <= 0.0)
	{
		_minFlybackCyclesFactor = 1.0;
	}

	if (FALSE == pSetup->GetTrigger(_triggerWaitTimeout))
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"GetTrigger from ThorDAQResonantSettings failed");
		LogMessage(_errMsg, ERROR_EVENT);
	}

	if (FALSE == pSetup->GetIO(_pockelsVoltageSlopeThreshold, _pockelsLine[0], _pockelsPowerInputLine[0], _pockelsScanVoltageStart[0], _pockelsScanVoltageStop[0], _pockelsLine[1], _pockelsPowerInputLine[1], _pockelsScanVoltageStart[1], _pockelsScanVoltageStop[1], _pockelsLine[2], _pockelsPowerInputLine[2], _pockelsScanVoltageStart[2], _pockelsScanVoltageStop[2], _pockelsLine[3], _pockelsPowerInputLine[3], _pockelsScanVoltageStart[3], _pockelsScanVoltageStop[3], _pockelsResponseType[0], _pockelsResponseType[1], _pockelsResponseType[2], _pockelsResponseType[3]))
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"GetIO from ThorDAQResonantSettings failed");
		LogMessage(_errMsg, ERROR_EVENT);
	}

	for (long i = 0; i < MAX_POCKELS_CELL_COUNT; i++)
	{
		_pockelsEnable[i] = (_pockelsLine[i].size() > 0 && _pockelsLine[i] != "0") ? true : false;
	}

	if (FALSE == pSetup->GetHardwareTestMode(_hardwareTestModeEnable))
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"GetIO from GetHardwareTestMode failed");
		LogMessage(_errMsg, ERROR_EVENT);
	}

	auto_ptr<ThorDAQZXML> pZSetup(new ThorDAQZXML());
	if (FALSE == pZSetup->GetConversion(_zVolts2mm, _zOffsetmm, _zPos_min, _zPos_max))
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"GetConversion from ThorDAQZPiezo failed");
		LogMessage(_errMsg, ERROR_EVENT);
		_zPos_min = 0.0;
		_zPos_max = 0.0;
		_zVolts2mm = 1.0;
		_zOffsetmm = 0.0;
	}

	if (FALSE == pZSetup->GetsettlingConfig(_zTypicalMoveSettlingTime_sec, _zTypicalMove_mm))
	{
		_zTypicalMoveSettlingTime_sec = 0.00001;
		_zTypicalMove_mm = 0.01;
	}

	//load alignment data
	LoadAlignDataFile();

	return cameraCount;
}

/************************************************************************************************
* @fn	long CThordaqResonantGalvo::SelectCamera(const long camera)
*
* @brief	Select Camera.
*
* @param	camera	The camera index .
*
* @return	A long.
**************************************************************************************************/
long CThordaqResonantGalvo::SelectCamera(const long camera)
{
	int32 error = 0, retVal = 0;
	if (_deviceNum == 0)// No thordaq is connected
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"The hardware has not been located");
		LogMessage(_errMsg,ERROR_EVENT);
		return FALSE;
	}

	//Load parameter from XML
	try
	{
		auto_ptr<ThordaqResonantGalvoXML> pSetup(new ThordaqResonantGalvoXML());

		if (INTERNAL_CLOCK == _imgAcqPty.clockSource)
		{
			//set the GR clock rate, it will basically tell the board what frequency to expect. Keep both functions together.
			ThordaqErrChk (L"ThorDAQAPISetClockSource", retVal = ThorDAQAPISetClockSource(_DAQDeviceIndex,CLOCK_SOURCE::INTERNAL_80MHZ_REF));
			ThordaqErrChk (L"ThorDAQAPISetGRClockRate", retVal = ThorDAQAPISetGRClockRate(_DAQDeviceIndex,DEFAULT_INTERNALCLOCKRATE, static_cast<ULONG32>(_crsFrequencyHighPrecision)));
		}
		
	}
	catch(...)
	{
		return FALSE;
	}

	//Close the Shutter

	ThordaqErrChk(L"ThorDAQAPISlowSmoothMoveToAndFromParkEnable", retVal = ThorDAQAPISlowSmoothMoveToAndFromParkEnable(_DAQDeviceIndex, true));

	//park the galvo, also park regular GalvoResonant Y if in RGG mode
	//TODO: why are we doing this?
	if (TRUE == _rGGMode)
	{
	//	MoveGalvoToParkPosition(_reson, FALSE);
	}

	MoveGalvoToParkPosition(_thordaqAOSelection[AO::GR_Y], FALSE);

	return TRUE;
}

/************************************************************************************************
* @fn	long CThordaqResonantGalvo::TeardownCamera()
*
* @brief	Teardown Camera.
*
* @return	A long.
**************************************************************************************************/
long CThordaqResonantGalvo::TeardownCamera()
{
	int32 error = 0, retVal = 0;
	StringCbPrintfW(_errMsg,MSG_SIZE,L"ThordaqResonantGalvo::TeardownCamera");
	LogMessage(_errMsg,VERBOSE_EVENT);

	//Stop the acquisition thread
	ThordaqErrChk (L"ThorDAQAPIAbortRead", retVal = ThorDAQAPIAbortRead(_DAQDeviceIndex));
	SetEvent(_hStopAcquisition);
	//Wait for acqisition's stop
	Sleep(500);
	WaitForSingleObject(_hThreadStopped, 5000);

	//terminate daq thread:
	StopDaqBrd();	
	//save the configuration settings

	ThordaqErrChk(L"ThorDAQAPISlowSmoothMoveToAndFromParkEnable", retVal = ThorDAQAPISlowSmoothMoveToAndFromParkEnable(_DAQDeviceIndex, true));

	//park the galvo when exiting
	MoveGalvoToParkPosition(_thordaqAOSelection[AO::GR_Y], TRUE);

	//park the pockels cell
	MovePockelsToParkPosition(&_imgAcqPty.pockelPty);

	//release thor thordaq board
	ThorDAQAPIReleaseBoard(_DAQDeviceIndex);

	unique_ptr<ThordaqResonantGalvoXML> pSetup(new ThordaqResonantGalvoXML());

	pSetup->SetCalibration(_imgAcqPty.pockelPty.pockelsDelayUS, _imgAcqPty.pockelsBlankingPhaseShiftPercent, _imgAcqPty.preImagingCalibrationCycles, _imgAcqPty.imagingRampExtensionCycles, _imgAcqPty.pockelPty.pockelsMinVoltage, _imgAcqPty.pockelPty.pockelsMaxVoltage, _imgAcqPty.ImageDistortionCorrectionCalibrationXAngleMax, _imgAcqPty.ImageDistortionCorrectionCalibrationYAngleMax, _imgAcqPty.ImageDistortionCorrectionCalibrationGalvoTiltAngle);
	//close the shutter

	//:TODO: When cleaning up, remove if it is not necessary
	//Reset DAQ:
	//DAQmxResetDevice("Dev1");

	//disconnect DAQ board
	_deviceNum = 0;
	//// Clean all the threads
	SAFE_DELETE_HANDLE(_hAcquisitionThread);

	//// Free all the buffers
	SAFE_DELETE_PTR(_pFrmBuffer);

	for (int i = 0; i < _pHistoryBuf.size(); ++i)
	{
		SAFE_DELETE_PTR(_pHistoryBuf[i]);
	}
	_pHistoryBuf.clear();

		
	SAFE_DELETE_PTR(_pDataProcessor);

	//// Delete all the pointers


	return TRUE;
}


/************************************************************************************************
* @fn	long CThordaqResonantGalvo::PreflightAcquisition(char * pDataBuffer)
*
* @brief	Preflight position
* @param [in,out]	pDataBuffer	  	Identifier of the Data buffer.
* @return	A long.
**************************************************************************************************/
long CThordaqResonantGalvo::PreflightAcquisition(char * pDataBuffer)
{
	long ret = TRUE;
	int32 error = 0, retVal = 0;

	//#ifdef TEST
	//	_imgAcqPty.channel = 0xf; // Test force to run at all 4 channels
	//#endif

	_forceSettingsUpdate = TRUE;// Force to enable acquisition at first set 
	// verify the image size depends on the area mode

	if (LSMGRRotationAngle::DEG_0  != _rotationAnglePosition && LSMGRRotationAngle::DEG_180 != _rotationAnglePosition)
	{
		_imgAcqPty.areaMode = ICamera::LSMAreaMode::SQUARE;
	}

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
	case ScanMode::CENTER:
		{
			MoveGalvoToCenter(_thordaqAOSelection[AO::GR_Y], _offsetVal);

			MovePockelsToPowerLevel(&_imgAcqPty.pockelPty);
			ret =  TRUE;
			break;
		}
	case ScanMode::BLEACH_SCAN://test the waveformBuffer
		{
			ret = TRUE;
			break;
		}
	default:
		{
			//if we are in imaging mode, then set the parking position
			//to the regular parking position
			MoveGalvoToParkPosition(_thordaqAOSelection[AO::GR_Y], FALSE);
			MovePockelsToParkPosition(&_imgAcqPty.pockelPty);
			ret =  TRUE;
		}
	}

	// For stimulus mode, reconfigure the buffers and acquisition parameters.
	if(ICamera::HW_MULTI_FRAME_TRIGGER_FIRST == _imgAcqPty_Pre.triggerMode)
	{
		/**********reset the acquisition thread and the event flag*******/
		ThordaqErrChk (L"ThorDAQAPIAbortRead", retVal = ThorDAQAPIAbortRead(_DAQDeviceIndex));
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

	return ret;
}

/************************************************************************************************
* @fn	long CThordaqResonantGalvo::SetupAcquisition(char * pDataBuffer)
*
* @brief	Setup  Acquisition.
* @param [in,out]	pDataBuffer	  	Identifier of the Data buffer.
* @return	A long.
**************************************************************************************************/
long CThordaqResonantGalvo::SetupAcquisition(char * pDataBuffer)
{
	long ret = TRUE;
	int32 error = 0, retVal = 0;

	//do not capture data if you are in the centering scan mode, Just Set the Scanner
	if(_imgAcqPty.scanMode == ScanMode::CENTER)
	{
		return ret;
	}

	//allow resonance scanner to be stable if not on already
	/*if((firstFrame) && (FALSE == _scannerInitMode))
	{
	if(_crsFrequencyHighPrecision > 9000)
	{
	Sleep(2200/(_imgAcqPty.fieldSize/_fieldSizeMin) + 200);
	}
	else
	{
	Sleep(200);
	}
	}*/

	//If settings change or system is forced to refresh updating, do update.
	if (!_imgAcqPty.IsEqual(_imgAcqPty_Pre) || TRUE == _pockelsMaskChanged || _forceSettingsUpdate == TRUE)
	{
		// Disable Force updating
		_forceSettingsUpdate = FALSE;

		_pockelsMaskChanged = FALSE;
		
		
		if (LSMGRRotationAngle::DEG_0  != _rotationAnglePosition && LSMGRRotationAngle::DEG_180 != _rotationAnglePosition)
		{
			_imgAcqPty.areaMode = ICamera::LSMAreaMode::SQUARE;
		}

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
			_imgAcqPty.scanMode = ScanMode::FORWARD_SCAN;
		}		

		/**********reset the acquisition thread and the event flag*******/
		ThordaqErrChk (L"ThorDAQAPIAbortRead", retVal = ThorDAQAPIAbortRead(_DAQDeviceIndex));
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

	return ret;
}

/// <summary> Waits on a hardware trigger. Will also stop with a timeout or a software stop acquisition event
///	For Thordaq the board is in charge of receiving the trigger or timing out. StartFrameAcqProc will dictate
/// if the board recerived a frame trigger in or not, once ReadChannel() returns.
/// </summary>
/// <param name="timeoutMillis"> Max time to wait </param>
/// <param name="showTimeoutMessage"> Whether to show a message box warning of a timeout </param>
/// <returns> Returns true if a hardware trigger stopped the wait, false otherwise </returns>
long CThordaqResonantGalvo::WaitForHardwareTrigger(long timeoutMillis, long showTimeoutMessageBox)
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
	if(ret - WAIT_OBJECT_0 == hardwareIndex) 
	{
		return TRUE;
	}
	//Handle Other Causes
	else
	{	
		//Timeout
		if(ret == WAIT_TIMEOUT)
		{
			if(TRUE == showTimeoutMessageBox)
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"External trigger is timed out after %d seconds.",timeoutMillis/MS_TO_SEC);
				MessageBox(NULL,_errMsg,L"Trigger Error", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
			}
			//Log
			StringCbPrintfW(_errMsg,_MAX_PATH,L"StartAcquisition Hardware trigger timeout");
			LogMessage(_errMsg,ERROR_EVENT);
		}
		//Error
		else if(ret == WAIT_FAILED || ret == WAIT_ABANDONED)
		{
			//Log
			StringCbPrintfW(_errMsg,_MAX_PATH,L"While waiting for StartAcquisition Hardware trigger, WaitOnMultipleObjects failed with bad handle");
			LogMessage(_errMsg,ERROR_EVENT);
		}
		//Other event
		else
		{
			for(HANDLE hEvent : hardwareEvents)
			{
				ResetEvent(hEvent);
			}
		}
		return FALSE;
	}
}


/************************************************************************************************
* @fn	long CThordaqResonantGalvo::StartAcquisition(char * pDataBuffer)
*
* @brief	Start  Acquisition.
* @param [in,out]	pDataBuffer	  	Identifier of the Data buffer.
* @return	A long.
**************************************************************************************************/
long CThordaqResonantGalvo::StartAcquisition(char * pDataBuffer)
{
	int32 error = 0, retVal = 0;
	//	StringCbPrintfW(message,MSG_SIZE,L"ThorLSMCam::StartAcquisition");
	//	LogMessage(message,VERBOSE_EVENT);

	try
	{
		//do not capture data if you are in the centering scan mode
		if(_imgAcqPty_Pre.scanMode == CENTER)
		{
			return TRUE;
		}
		switch(_imgAcqPty_Pre.triggerMode)
		{
		case ICamera::SW_FREE_RUN_MODE: // Free Run
		case ICamera::SW_MULTI_FRAME:
			{
				if (WaitForSingleObject(_hThreadStopped, 0) == WAIT_OBJECT_0)
				{
					//make sure to reload the waveformBuffer for the first call of the thread.
					//This ensures the frame trigger is restarted for output
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
				//	//make sure to reload the waveformBuffer for the first call of the thread.
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

		case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
			{
				if (WaitForSingleObject(_hThreadStopped, 0) == WAIT_OBJECT_0)
				{
					//make sure to reload the waveformBuffer for the first call of the thread.
					if(STATUS_SUCCESSFUL != retVal)
					{ 	 
						return FALSE; 
					}
					DWORD ThreadID;
					ResetEvent(_hStopAcquisition);

					SAFE_DELETE_HANDLE(_hAcquisitionThread);//close thread here since thread stop was signaled

					_hAcquisitionThread = CaptureCreateThread(ThreadID);

					//Wait for the hardware trigger event. This is for stimulus scanning, it has to wait here 
					// until it gets a stop event or a hardware trigger in event.
					long triggerWaitTimeoutMilliseconds = _triggerWaitTimeout * MS_TO_SEC;
					if(WaitForHardwareTrigger(triggerWaitTimeoutMilliseconds, TRUE))
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
	catch(...)
	{
		//		StringCbPrintfW(message,MSG_SIZE,L"StartAcquisition failed.");
		//		LogMessage(message,ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

/************************************************************************************************
* @fn	HANDLE CThordaqResonantGalvo::CaptureCreateThread(DWORD &threadID)
*
* @brief	Create  Acquisition Thread.
* @param 	threadID	  	Acquisition Thread ID.
* @return	Thread Handle.
**************************************************************************************************/
HANDLE CThordaqResonantGalvo::CaptureCreateThread(DWORD &threadID)
{
	//DigiParams *dParamsOut = new DigiParams();
	//*dParamsOut = _digiParams;
	_indexOfLastCompletedFrame = -1;
	_indexOfLastCopiedFrame = -1;
	ResetEvent(_hThreadStopped);
	HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &(CThordaqResonantGalvo::StartFrameAcqProc), (void *) this, 0, &threadID);
	if (NULL != handle)
	{
		SetThreadPriority(handle, THREAD_PRIORITY_HIGHEST);
	}
	return handle;
}

void CThordaqResonantGalvo::StopDaqBrd()
{
	int32 error = 0, retVal = 0;;
	ThordaqErrChk (L"ThorDAQAPIStopAcquisition", retVal = ThorDAQAPIStopAcquisition(_DAQDeviceIndex));
}

long CThordaqResonantGalvo::StatusAcquisition(long& status)
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
		//ResetEvent(_hTriggerTimeout);
		return ret;
	}
	long long comparison_index = _indexOfLastCompletedFrame;
	long long  lastReadIndex = _indexOfLastCopiedFrame;

	if (lastReadIndex < comparison_index)
	{
		status = ICamera::STATUS_READY;
		//printf("_indexOfLastCompletedFrame = %d; _indexOfCopiedFrame = %d.\n", _indexOfLastCompletedFrame, _indexOfCopiedFrame);
		//StringCbPrintfW(message,MSG_SIZE,L"Status now is ready with frame buffer size of %d.\n", _indexOfLastCompletedFrame - _indexOfCopiedFrame);
		//LogMessage(message,VERBOSE_EVENT);

		if (comparison_index - lastReadIndex > _imgAcqPty_Pre.dmaBufferCount && _imgAcqPty_Pre.triggerMode != SW_FREE_RUN_MODE)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"CThordaqResonantGalvo:StatusAcquition error, DMA overflow. Index of last completed frame: %lld, Index of copied frame: %lld", comparison_index, lastReadIndex);
			CThordaqResonantGalvo::GetInstance()->LogMessage(errMsg, ERROR_EVENT);
			status = ICamera::STATUS_ERROR;
		}
	}
	else
	{
		status = ICamera::STATUS_BUSY;
	}

	return ret;
}

long CThordaqResonantGalvo::StatusAcquisitionEx(long &status,long &indexOfLastCompletedFrame)
{
	long ret = TRUE;

	//do not capture data if you are in the centering scan mode
	if(_imgAcqPty_Pre.scanMode == CENTER)
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

	indexOfLastCompletedFrame = static_cast<long>(_indexOfLastCompletedFrame);

	return ret;
}

long CThordaqResonantGalvo::CopyAcquisition(char *pDataBuffer, void* frameInfo)
{	
	long ret = TRUE;

	//do not capture data if you are in the centering scan mode
	if (_imgAcqPty_Pre.scanMode == CENTER)
	{
		return ret;
	}

	int selectedChannels = CountChannelBits(_imgAcqPty_Pre.channel);
	//int channelCnt  = (selectedChannels > 1) ? 4 : 1;
	int channelCnt = 4;

	size_t frameNum = 0;
	if (_indexOfLastCopiedFrame == _indexOfLastCompletedFrame)
	{
		return FALSE;
	}

	double averageCumulativeNum = (_imgAcqPty_Pre.averageMode == 0) ? 1.0 : _imgAcqPty_Pre.averageNum;

	if (_lastAreaCopiedIndex >= _pTempBuf.size() || _lastAreaCopiedIndex < 0)
	{
		if (_pFrmBuffer->GetNextFrame(_pTempBuf))
		{
			_lastAreaCopiedIndex = 0;
		}
		else
		{
			return FALSE;
		}
	}

	ProcessedFrame* frameToCopy = _pTempBuf[_lastAreaCopiedIndex];

	if (_imgAcqPty_Pre.enableImageDistortionCorrection)
	{

		if ((LSMGRRotationAngle)_rotationAnglePosition == LSMGRRotationAngle::DEG_0 && (FALSE == _imgAcqPty_Pre.horizontalFlip || _imgAcqPty_Pre.mROIModeEnable))
		{
			CorrectPreludeImageDistortion(frameToCopy->Data, (unsigned short*)pDataBuffer, selectedChannels, _daqAcqCfg.imageCtrl.channel, frameToCopy->Width, frameToCopy->Height);
		}
		else
		{
			//if the angle is changing and there is a flip then we need an intermediate buffer
			ProcessedFrame* itermidiateBuf = _pTempBufCorrection[_lastAreaCopiedIndex];
			CorrectPreludeImageDistortion(frameToCopy->Data, itermidiateBuf->Data, selectedChannels, _daqAcqCfg.imageCtrl.channel, frameToCopy->Width, frameToCopy->Height);
			ManipulateAndCopyImage(itermidiateBuf->Data, (unsigned short*)pDataBuffer, itermidiateBuf->Width, itermidiateBuf->Height, selectedChannels, _daqAcqCfg.imageCtrl.channel, _imgAcqPty_Pre.rawSaveEnabledChannelOnly, (LSMGRRotationAngle)_rotationAnglePosition, _imgAcqPty_Pre.horizontalFlip);
		}
	}
	else
	{
		long horizontalFlip = _imgAcqPty_Pre.horizontalFlip == TRUE && !_imgAcqPty_Pre.mROIModeEnable;
		ManipulateAndCopyImage(frameToCopy->Data, (unsigned short*)pDataBuffer, frameToCopy->Width, frameToCopy->Height, selectedChannels, _daqAcqCfg.imageCtrl.channel, _imgAcqPty_Pre.rawSaveEnabledChannelOnly, (LSMGRRotationAngle)_rotationAnglePosition, horizontalFlip);
	}


	//generate pulse to signal frame buffer ready:
	SetFrameBufferReadyOutput();
	int selectedChannelCount = CountChannelBits(_daqAcqCfg.imageCtrl.channel);
	//set all the important frame info
	FrameInfo frameInfoStruct;
	memcpy(&frameInfoStruct, frameInfo, sizeof(FrameInfo));
	frameInfoStruct.fullFrame = TRUE;
	frameInfoStruct.isNewMROIFrame = _lastAreaCopiedIndex == 0;
	frameInfoStruct.isMROI = _imgAcqPty_Pre.ismROI;
	frameInfoStruct.totalScanAreas = (long)_pTempBuf.size();
	frameInfoStruct.scanAreaIndex = (long)_lastAreaCopiedIndex;
	frameInfoStruct.scanAreaID = (long)frameToCopy->ScanAreaID;
	frameInfoStruct.fullImageWidth = (long)frameToCopy->FullImageWidth;
	frameInfoStruct.fullImageHeight = (long)frameToCopy->FullImageHeight;
	frameInfoStruct.topInFullImage = (long)frameToCopy->Top;
	frameInfoStruct.leftInFullImage = (long)frameToCopy->Left;
	frameInfoStruct.mROIStripeFieldSize = _imgAcqPty_Pre.stripeFieldSize;
	frameInfoStruct.imageWidth = (long)frameToCopy->Width;
	frameInfoStruct.imageHeight = (long)frameToCopy->Height;
	frameInfoStruct.copySize = frameToCopy->GetDataSize();
	frameInfoStruct.channels = selectedChannelCount > 1 ? (_imgAcqPty.rawSaveEnabledChannelOnly? selectedChannelCount:MAX_CHANNEL_COUNT) : selectedChannelCount;;
	frameInfoStruct.numberOfPlanes = 1;
	memcpy(frameInfo, &frameInfoStruct, sizeof(FrameInfo));

	++_lastAreaCopiedIndex;


	if (_lastAreaCopiedIndex >= _pTempBuf.size())
	{
		++_indexOfLastCopiedFrame;
	}

	return ret;
}

long CThordaqResonantGalvo::ManipulateAndCopyImage(unsigned short* pDataSource, unsigned short* pDataDest, long width, long height, long channelNum, long channelEnable, long rawChannelsOnly, LSMGRRotationAngle rotationAngle, long horizontalFlip)
{
	unsigned short* intermediateBuffer = NULL;
	long flipped = FALSE;
	long memAllocated = FALSE;
	long imageSize = width * height;

	if (LSMGRRotationAngle::DEG_0 != rotationAngle && TRUE == horizontalFlip)
	{
		//if the angle is changing and there is a flip then we need an intermediate buffer
		intermediateBuffer = (unsigned short*)malloc(width*height*sizeof(unsigned short));
		memAllocated = TRUE;
	}
	for (int i = 0,j = 0, k = 0; i < MAX_CHANNEL_COUNT; i++)
	{
		if ((channelEnable & (0x0001 << i)) != 0x0000)
		{
			unsigned short* pData = pDataSource + imageSize * (size_t)k;
			++k;
			unsigned short* dst = pDataDest + imageSize * (size_t)j;
			if (TRUE == horizontalFlip || LSMGRRotationAngle::DEG_0 != rotationAngle)
			{
				if (TRUE == horizontalFlip)
				{
					IppiSize size;
					size.width = width;
					size.height = height;

					//if the angle is not changing, then no need to create an intermediate buffer, use the dst buffer
					if (ICamera::LSMGRRotationAngle::DEG_0 == rotationAngle)
					{
						intermediateBuffer = dst;
					}				

					int step = width* sizeof(unsigned short);

					ippiDll->ippiMirror_16u_C1R(pData, step, intermediateBuffer, step,  size, ippAxsVertical);

					flipped = TRUE;
				}
				else
				{
					//if there is no flip then the intermediate buffer can be the captured image
					intermediateBuffer =  pData;
				}

				switch (rotationAngle)
				{
				case LSMGRRotationAngle::DEG_0:
					{
						if(FALSE == flipped)
						{
							//only need to copy it if there was no flip, otherwise  dst == intermediateBuffer
							memcpy(dst,intermediateBuffer, width*height*sizeof(unsigned short));
						}
					}
					break;
				case LSMGRRotationAngle::DEG_90:
					{
						IppiSize size;
						size.width = width * sizeof(unsigned short);
						size.height = height * sizeof(unsigned short);
						int stepSrc = width * sizeof(unsigned short);
						IppiRect  roiSrc = {0, 0, width, height};
						int stepDst = height * sizeof(unsigned short);
						IppiRect  roiDst = {0, 0, height, width};
						long angle = 90;
						long xOffset = 0;
						long yOffset = height - 1;
						ippiDll->ippiRotate_16u_C1R(intermediateBuffer, size, stepSrc, roiSrc, dst, stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
					}
					break;
				case LSMGRRotationAngle::DEG_180:
					{
						IppiSize size;
						size.width = width* sizeof(unsigned short);
						size.height = height * sizeof(unsigned short);
						int stepSrc = width * sizeof(unsigned short);
						IppiRect  roiSrc = {0, 0, width, height};
						int stepDst = width * sizeof(unsigned short);
						IppiRect  roiDst = {0, 0, width, height};
						long angle = 180;
						long xOffset = width - 1;
						long yOffset = height - 1;
						ippiDll->ippiRotate_16u_C1R(intermediateBuffer, size, stepSrc, roiSrc, dst, stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
					}
					break;
				case LSMGRRotationAngle::DEG_270:
					{
						IppiSize size;
						size.width = width * sizeof(unsigned short);
						size.height = height * sizeof(unsigned short);
						int stepSrc = width * sizeof(unsigned short);
						IppiRect  roiSrc = {0, 0, width, height};
						int stepDst = height * sizeof(unsigned short);
						IppiRect  roiDst = {0, 0, height, width};
						long angle = 270;
						long xOffset = height - 1;
						long yOffset = 0;
						ippiDll->ippiRotate_16u_C1R(intermediateBuffer, size, stepSrc, roiSrc, dst, stepDst, roiDst, angle, xOffset, yOffset, IPPI_INTER_LINEAR);
					}
					break;
				default:
					if(FALSE == flipped)
					{
						//only need to copy the image if there was no flip, otherwise  dst == intermediateBuffer
						memcpy(dst, intermediateBuffer, width * height * sizeof(unsigned short));
					}			
					break;
				}
			}
			else
			{
				memcpy(dst,pData, width*height*sizeof(unsigned short));
			}
			++j;
		}
		else if (FALSE == rawChannelsOnly && channelNum > 1)
		{
			++j;
		}
	}

	//only free the intermediate buffer if its memory was allocated in this function
	if (TRUE == memAllocated)
	{
		free(intermediateBuffer);
	}
	return TRUE;
}

long CThordaqResonantGalvo::PostflightAcquisition(char * pDataBuffer)
{
	try
	{
		int32 error = 0, retVal = 0;
		//do not capture data if you are in the centering scan mode
		if(_imgAcqPty_Pre.scanMode == CENTER)
		{
			MoveGalvoToParkPosition(_thordaqAOSelection[AO::GR_Y], FALSE);
			MovePockelsToPowerLevel(&_imgAcqPty.pockelPty);
			return TRUE;
		}
		//force the hardware trigger event if the post flight function is called
		SetEvent(_hHardwareTriggerInEvent);
		ResetEvent(_hHardwareTriggerInEvent);

		ThordaqErrChk (L"ThorDAQAPIAbortRead", retVal = ThorDAQAPIAbortRead(_DAQDeviceIndex));
		SetEvent(_hStopAcquisition);


		//for the trigger output to low
		//IDAQSetFrameTriggerOutLow();

		while(WaitForSingleObject(_hThreadStopped, 10000) != WAIT_OBJECT_0)
		{
			Sleep(10);
		}

		ThordaqErrChk(L"ThorDAQAPIDACBankSwitchingClearRegisteredReadyForNextImageWaveformsEventHandlers", retVal = ThorDAQAPIDACBankSwitchingClearRegisteredReadyForNextImageWaveformsEventHandlers(_DAQDeviceIndex));

		//stop digitizer board:
		//StopDaqBrd();

		MoveGalvoToParkPosition(_thordaqAOSelection[AO::GR_Y], FALSE);
		MovePockelsToParkPosition(&_imgAcqPty.pockelPty);
	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}

long CThordaqResonantGalvo::GetLastErrorMsg(wchar_t * msg, long size)
{
	long ret = TRUE;
	return ret;
}

void CThordaqResonantGalvo:: LogMessage(wchar_t *logMsg,long eventLevel)
{
#ifdef LOGGING_ENABLED
	qlogDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

void CThordaqResonantGalvo::SetStatusHandle(HANDLE handle)
{

}

long CThordaqResonantGalvo::MoveGalvoToParkPosition(long Galvo_Y_channel, long isTearDown)
{
	long ret = FALSE;
	int32 error = 0, retVal = 0;
	// if we are in RRG mode controlling GGY move it to -10V during Teardown
	double yGalvoParkPosition = ((TRUE == _rGGMode && TRUE == isTearDown) || 0 == _imgAcqPty.verticalScanDirection) ? -GALVO_PARK_POSITION : GALVO_PARK_POSITION;


	// Park X Galvo at 0 if RGG mode is enabled, during a TearDown move it back to the original park position
	if(TRUE == _rGGMode && FALSE == isTearDown)
	{
		ThordaqErrChk (L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, _thordaqAOSelection[AO::GG0_X], 0));
	}

	if (retVal != STATUS_SUCCESSFUL)
	{
		LogMessage(L"ThordaqGR MoveGalvoToParkPosition failed to move GG X to park",ERROR_EVENT);
	}

	ThordaqErrChk (L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex,Galvo_Y_channel, yGalvoParkPosition));
	if (retVal != STATUS_SUCCESSFUL)
	{
		ret = 0;
	}

	return ret;
}

long CThordaqResonantGalvo::MoveGalvoToCenter(long Galvo_Y_channel, double offsetVal)
{
	long ret = FALSE;
	int32 error = 0, retVal = 0;

	ThordaqErrChk (L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex,Galvo_Y_channel, offsetVal));
	if (retVal != STATUS_SUCCESSFUL)
	{
		ret = 0;
	}

	return ret;
}

long CThordaqResonantGalvo::MovePockelsToParkPosition(PockelPty* pockelPty)
{
	long ret = 0;
	int32 error = 0, retVal = 0;

	for (int i = 0; i < MAX_POCKELS_CELL_COUNT; i++)
	{
		if (_pockelsEnable[i] == FALSE)
		{
			continue;
		}

		long pockelOutputChannel = _thordaqAOSelection[(AO)((int)AO::GR_P0 + i)];
		double pockelsSetVal0 = pockelPty->pockelsMinVoltage[i] + (pockelPty->pockelsMaxVoltage[i] - pockelPty->pockelsMinVoltage[i]) * pockelPty->pockelsPowerLevel[i];
		double park_value = (TRUE == _pockelsParkAtMinimum) ? pockelPty->pockelsMinVoltage[i] : pockelsSetVal0;
		ThordaqErrChk (L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, pockelOutputChannel,park_value));
		if (retVal != STATUS_SUCCESSFUL)
		{
			ret = FALSE;
		}
	}

	return ret;
}

long CThordaqResonantGalvo::MovePockelsToPowerLevel(PockelPty* pockelPty)
{
	int32 error = 0, retVal = 0;

	for (int i = 0; i < MAX_POCKELS_CELL_COUNT; i++)
	{
		if (_pockelsEnable[i] == FALSE)
		{
			continue;
		}
		long pockelOutputChannel = _thordaqAOSelection[(AO)((int)AO::GR_P0 + i)];
		double pockelsOnVoltage = pockelPty->pockelsMinVoltage[i] + (pockelPty->pockelsMaxVoltage[i] - pockelPty->pockelsMinVoltage[i]) * pockelPty->pockelsPowerLevel[i];
		ThordaqErrChk (L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, pockelOutputChannel,pockelsOnVoltage));
		if (retVal != STATUS_SUCCESSFUL)
		{
			retVal = FALSE;
		}
	}
	return retVal;
}

/// <summary> Cancels any hardware triggers the camera is currently waiting on </summary>
void CThordaqResonantGalvo::StopHardwareWaits()
{
	int32 error = 0;
	ThordaqErrChk (L"ThorDAQAPIAbortRead", ThorDAQAPIAbortRead(_DAQDeviceIndex));
	SetEvent(_hStopAcquisition);
}

void CThordaqResonantGalvo::SetupDataMaps()
{
	unsigned int i;
	{
		//folding mapping for positive and negative amplifiers
		//datamap should reflect 14bit resolution of the digitizer

		//negative voltage mapping
		for (i = 0; i < 8192; i++)
		{
			_datamapIndependent[i] = (8191 - i) * 2;
		}

		//positive voltage mapping
		for (i = 8192; i < 65536; i++)
		{
			_datamapIndependent[i] = 0;
		}
	}

	{
		//16 bit mapping with most significant data in positive polarity
		//positive voltage mapping
		for (i = 0; i < 16384; i++)
		{
			_datamapPositiveSigned[i] = static_cast<USHORT>(i - 8192);
		}
		//positive voltage mapping
		for (i = 16384; i < 65536; i++)
		{
			_datamapPositiveSigned[i] = 8192;
		}
	}

	{
		//16 bit mapping with most significant data in negative polarity
		//negative voltage mapping
		for (i = 0; i < 16384; i++)
		{
			_datamapNegativeSigned[i] = static_cast<USHORT>((16383 - i) - 8192);
		}
		for (i = 16384; i < 65536; i++)
		{
			_datamapNegativeSigned[i] = static_cast<USHORT>(-8192);
		}
	}

	{
		//negative voltage mapping
		for (i = 0; i < 8192; i++)
		{
			_datamapPositiveUnsigned[i] = 0;
		}

		//positive voltage mapping
		for (i = 8192; i < 16384; i++)
		{
			_datamapPositiveUnsigned[i] = (i - 8192) * 2;
		}

		//positive voltage mapping
		for (i = 16384; i < 65536; i++)
		{
			_datamapPositiveUnsigned[i] = 16382;
		}
	}

	{
		//negative voltage mapping
		for (i = 0; i < 8192; i++)
		{
			_datamapNegativeUnsigned[i] = (8191 - i) * 2;
		}

		//positive voltage mapping
		for (i = 8192; i < 65536; i++)
		{
			_datamapNegativeUnsigned[i] = 0;
		}
	}
}

int CThordaqResonantGalvo::CountChannelBits(long channelSet)
{
	int count = 0;
	while(channelSet)
	{
		channelSet &= (channelSet - 1);
		count ++;
	}
	return count;
}

/// <summary> Calculates the minimum value for flyback cycles the current settings can support </summary>
/// <returns> The calculated minimum flyback cycles </returns>
long CThordaqResonantGalvo::GetMinFlybackCycle()
{
	//Calculation to determine flybacklines:
	//detemine the Y FOV in unit of volt, full swing of waveformBuffer, based on field size and the image pixel aspect ratio
	double theta = (double)_imgAcqPty.fieldSize * _field2Theta;
	double fieldY_volt = theta * (double) _imgAcqPty.pixelY / (double)_imgAcqPty.pixelX / 2; // divide by 2 because the mechanical angle is half of the optical angle

	/*if(TRUE ==_useZoomArray)
	{
	fieldY_volt = fieldY_volt + fieldY_volt*_zoomArray[_fieldSize]/100.0;
	}*/
	fieldY_volt = (_imgAcqPty.yAmplitudeScaler/100.0) * fieldY_volt;

	if (FALSE == _imgAcqPty.galvoEnable && TRUE == _pockelsEnable[0]) //if LineScan is enabled
	{
		fieldY_volt = 0;
	}

	return GetMinFlybackCycle(fieldY_volt);
}

long CThordaqResonantGalvo::GetMinFlybackCycle(double fieldY_volt)
{
	long minFlybackCycle = 0;
	if (_imgAcqPty.imageOnFlyback)
	{
		return 1;
	}
	else if (fieldY_volt <= 1.0) //up to fieldSize: 22 // if (fieldY_volt <= 1.0) //NI AO amplitude less than 1.0
	{
		minFlybackCycle = (long)round(4 * _minFlybackCyclesFactor);
	}
	else if (fieldY_volt <= 3.0) //up to fieldSize: 66
	{
		minFlybackCycle = (long)round(6 * _minFlybackCyclesFactor);
	}
	else if (fieldY_volt <= 7.2) //up to fieldSize: 159
	{
		minFlybackCycle = (long)round(8 * _minFlybackCyclesFactor);
	}
	else if (fieldY_volt <= 9.9) //fieldSize: 219
	{
		minFlybackCycle = (long)round(12 * _minFlybackCyclesFactor);
	}
	else
	{
		minFlybackCycle = (long)round(16 * _minFlybackCyclesFactor);
	}

	return minFlybackCycle;
}

/// <summary> Returns the currently number of flyback cycles to use, modified if needed to fall within
/// the minimum the current settings support </summary>
/// <returns> The flybackCycle parameter the hardware should use </returns>
long CThordaqResonantGalvo::GetFlybackCycle()
{
	long minFlybackCycle = GetMinFlybackCycle();
	if(_imgAcqPty.minimizeFlybackCycles || minFlybackCycle > _rawFlybackCycle)
	{
		return minFlybackCycle;
	}
	else if(GetFlybackTime(_rawFlybackCycle) > MAX_FLYBACK_TIME)
	{
		_rawFlybackCycle = static_cast<long>(MAX_FLYBACK_TIME * _crsFrequencyHighPrecision);
	}
	return _rawFlybackCycle;
}

/// <summary> Gets the flyback time the current number of flyback cycles will take </summary>
/// <returns> The flyback time in seconds the current number of flyback cycles will take </returns>
double CThordaqResonantGalvo::GetFlybackTime()
{
	return GetFlybackTime(GetFlybackCycle());
}


/// <summary> Sets the current flyback cycle </summary>
/// <param name="flybackCycle"> The new value for flyback cycle </param>
void CThordaqResonantGalvo::SetFlybackCycle(long flybackCycle)
{
	long minFlybackCycle = GetMinFlybackCycle();
	_rawFlybackCycle = flybackCycle > minFlybackCycle ? flybackCycle : minFlybackCycle;
}


/// <summary> Calculate the flyback time the input number of flyback cycles will take using the current settings </summary>
/// <param name="flybackCycle"> The number of flyback cycles to account for </param>
/// <returns> The time in seconds that it would take to do the requested number of flyback cycles </returns>
double CThordaqResonantGalvo::GetFlybackTime(long flybackCycles)
{
	double lineTime = 1 / _crsFrequencyHighPrecision ;
	return flybackCycles * lineTime;
}

long CThordaqResonantGalvo::FindPockelsMinMax(long index, PockelPty* pockelPty)
{
	int32 retVal = 0;
	int32 error = 0;

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

		long pockelOutputChannel = _thordaqAOSelection[(AO)((int)AO::GR_P0 + index)];

		for(long i=0; i<POCKELS_VOLTAGE_STEPS; i++)
		{		
			ThordaqErrChk (L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, pockelOutputChannel,pockelsPos));

			Sleep(30);

			int32 numRead;
			DAQmxErrChk(L"DAQmxReadAnalogF64",DAQmxReadAnalogF64(_taskHandleAIPockels[index], 1, 10.0, DAQmx_Val_GroupByChannel, &_pockelsReadArray[index][i],1,&numRead,NULL));
			DAQmxErrChk(L"DAQmxWaitUntilTaskDone", DAQmxWaitUntilTaskDone(_taskHandleAIPockels[index], MAX_TASK_WAIT_TIME));

			Sleep(1);

			pockelsPos += VOLTAGE_RANGE/POCKELS_VOLTAGE_STEPS;
		}

		//move back to the start position after the scan
		pockelsPos = VOLTAGE_START;
		/*const float64* resetWfm = &pockelsPos;
		DAQmxErrChk(L"DAQmxWriteAnalogF64",DAQmxWriteAnalogF64(_taskHandleAOPockels[index], 1, TRUE, 10.0, DAQmx_Val_GroupByChannel, resetWfm, NULL, NULL));
		DAQmxErrChk(L"DAQmxWaitUntilTaskDone", DAQmxWaitUntilTaskDone(_taskHandleAOPockels[index], MAX_TASK_WAIT_TIME));*/

		ThordaqErrChk (L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, pockelOutputChannel,pockelsPos));

		float64 * pSmoothBuffer = new float64[POCKELS_VOLTAGE_STEPS];

		memcpy(pSmoothBuffer,&_pockelsReadArray[index],POCKELS_VOLTAGE_STEPS * sizeof(float64));

		//smooth the data and ignore the ends
		const long KERNEL_SIZE = 5;
		const long KERNEL_SKIP = 2;

		for(long n=0; n<5; n++)
		{
			for(long i=KERNEL_SKIP; i<POCKELS_VOLTAGE_STEPS-KERNEL_SKIP; i++)
			{
				float64 sum = 0;
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
				double x1 = i - 1;
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
				double x1 = i - 1;
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

		if((minLoc != -1)&&(maxLoc != -1)&&((maxVal-minVal) > DIFFERENCE_THRESHOLD_VOLTS))
		{
			pockelPty->pockelsMinVoltage[index] = VOLTAGE_START + minLoc * VOLTAGE_RANGE/POCKELS_VOLTAGE_STEPS;
			pockelPty->pockelsMaxVoltage[index] = VOLTAGE_START + maxLoc * VOLTAGE_RANGE/POCKELS_VOLTAGE_STEPS;
		}
		else
		{
			retVal = -1;
		}

		//remove the line below to show the raw data in the plot
		memcpy(&_pockelsReadArray[index],pSmoothBuffer,POCKELS_VOLTAGE_STEPS * sizeof(float64));

		SAFE_DELETE_ARRAY(pSmoothBuffer);

		/*DAQmxTaskControl(_taskHandleAOPockels[index], DAQmx_Val_Task_Unreserve);
		DAQmxStopTask(_taskHandleAOPockels[index]);*/
	}
	catch(...)
	{
		memset(&_pockelsReadArray[index],0,POCKELS_VOLTAGE_STEPS * sizeof(float64));

		retVal = -1;
	}
	return retVal;
}

long CThordaqResonantGalvo::LoadAlignDataFile()
{
	int i;
	char appPath[256]; ///<the char of application path

	char filePath[256]; ///<char of the whole file path
	_getcwd(appPath, 256);

	FILE* AlignDataFile;

	strcpy_s(filePath, appPath);
	strcat_s(filePath, "\\AlignDataCoarse.txt");

	if (fopen_s(&AlignDataFile, filePath, "r") != 0)
	{
	}
	else
	{
		const long MIN_COARSE_ALIGNMENT = 0;
		const long MAX_COARSE_ALIGNMENT = 255;
		for (i = 0; i < NUM_TWOWAY_ZONES; i++)
		{
			long lVal = 0;
			if (fscanf_s(AlignDataFile, "%d", &lVal) == EOF)
			{
			}
			else if (lVal < MIN_COARSE_ALIGNMENT)
			{
				lVal = MIN_COARSE_ALIGNMENT;
			}
			else if (lVal > MAX_COARSE_ALIGNMENT)
			{
				lVal = MAX_COARSE_ALIGNMENT;
			}
			if (i >= 5)
			{
				_imgAcqPty.twoWayZones[255 - i] = lVal;
			}
		}
		fclose(AlignDataFile);
	}

	strcpy_s(filePath, appPath);
	strcat_s(filePath, "\\AlignData.txt");

	if (fopen_s(&AlignDataFile, filePath, "r") != 0)
	{
	}
	else
	{
		const long MIN_FINE_ALIGNMENT = static_cast<long>((double)TWO_WAY_FINE_ALIGNMENT_OFFSET * -1.0);
		const long MAX_FINE_ALIGNMENT = static_cast<long>((double)SYS_CLOCK_FREQ / _crsFrequencyHighPrecision / 2.0 - (double)TWO_WAY_FINE_ALIGNMENT_OFFSET);
		for (i = 0; i < NUM_TWOWAY_ZONES; i++)
		{
			long lVal = 0;
			if (fscanf_s(AlignDataFile, "%d", &lVal) == EOF)
			{
			}
			else if (lVal < MIN_FINE_ALIGNMENT)
			{
				lVal = MIN_FINE_ALIGNMENT;
			}
			else if (lVal > MAX_FINE_ALIGNMENT)
			{
				lVal = MAX_FINE_ALIGNMENT;
			}
			if (i >= 5)
			{
				_imgAcqPty.twoWayZonesFine[255 - i] = lVal + TWO_WAY_FINE_ALIGNMENT_OFFSET;
			}
		}
		fclose(AlignDataFile);
	}

	strcpy_s(filePath, appPath);
	strcat_s(filePath, "\\ZoomData.txt");

	if (fopen_s(&AlignDataFile, filePath, "r") != 0)
	{
		for (i = 0; i < NUM_ZOOM_ZONES; i++)
			_zoomArray[i] = 0;
	}
	else
	{
		_useZoomArray = TRUE;
		for (i = 0; i < NUM_ZOOM_ZONES; i++)
		{
			if (fscanf_s(AlignDataFile, "%d", &_zoomArray[i]) == EOF)
			{
				for (i; i < NUM_ZOOM_ZONES; i++)
					_zoomArray[i] = 0;
			}
		}
		fclose(AlignDataFile);
	}
	return TRUE;
}

long CThordaqResonantGalvo::SetFrameBufferReadyOutput()
{	
	// Set Frame Buffer Ready to high then low for one milisecond
	//Toggle the first channel Aux_digital_output_0 by turning on the first bit.
	USHORT auxDigChannel = 0, low = 0, high = 1;	
	int32 error = 0, retVal = 0;

	ThordaqErrChk(L"ThorDAQAPIToggleAuxDigitalOutputs", retVal = ThorDAQAPIToggleAuxDigitalOutputs(_DAQDeviceIndex,auxDigChannel,high));
	if (retVal != STATUS_SUCCESSFUL)
	{
		return FALSE;
	}

	Sleep(1);

	ThordaqErrChk(L"ThorDAQAPIToggleAuxDigitalOutputs", retVal = ThorDAQAPIToggleAuxDigitalOutputs(_DAQDeviceIndex,auxDigChannel,low));
	if (retVal != STATUS_SUCCESSFUL)
	{
		return FALSE;
	}

	return TRUE;
}

void CThordaqResonantGalvo::TerminateTask(TaskHandle& handle)
{
	if(NULL != handle)
	{
		DAQmxStopTask(handle);
		DAQmxClearTask(handle);
		handle = NULL;
	}
}

long CThordaqResonantGalvo::GetDACSamplesPerLine(ScanLineStruct* scanLine, double& dac_rate, double line_time)
{
	if (ScannerType::MEMS == _scannerType)
	{

		double update_rate = _waveformUpdateRateSPS;
		double dac_max_rate = 199;
		double max_samples = floor(line_time * ((double)SYS_CLOCK_FREQ / (dac_max_rate + 1.0)) - 1.0); //Calculate the max number of samples based on the 199 max dac update rate
		double min_samples = max_samples - 3; // Not sure why it is using 3, seems to be fine with 1 or 2
		double min_offset = max_samples;
		double max_update_rate = ceil((double)SYS_CLOCK_FREQ / (min_samples / line_time) - 1.0);
		//To get the proper sample rate
		for (int updaterate = static_cast<int>(dac_max_rate); updaterate < max_update_rate; updaterate++)
		{
			for (double sample = max_samples; sample > min_samples; sample--)
			{
				double offset = (double)SYS_CLOCK_FREQ / (double)(updaterate + 1.0) * line_time - sample;
				if (abs(offset) < abs(min_offset))
				{
					min_offset = offset;
					dac_rate = (double)SYS_CLOCK_FREQ / (double)(updaterate + 1.0);
					scanLine->samples_scan = static_cast<long>(sample - 1.0);
				}
			}
		}
		scanLine->samples_idle = 0;
	}
	else
	{
		dac_rate = _waveformUpdateRateSPS;

		//convert the new rate back to counts
		int ur = (int)ceil(SYS_CLOCK_FREQ / dac_rate - 1);

		//convert back to the real update rate, available from the ThorDAQ board to match the actual counts
		dac_rate = (double)SYS_CLOCK_FREQ / (double)(ur + 1.0);

		scanLine->samples_idle = 0;

		//calculate the number of samples per line using the new data update rate
		scanLine->samples_scan = (UINT32)floor(line_time * dac_rate);
	}

	return TRUE;
}