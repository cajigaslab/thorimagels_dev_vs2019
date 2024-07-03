// ThorDAQGalvoGalvo.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "thordaqResonantGalvo.h"
#include "thordaqResonantGalvoSetupXML.h"


#ifdef LOGGING_ENABLED
#include "..\..\..\..\..\Common\Log.h"
std::auto_ptr<LogDll> qlogDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

wchar_t message[MSG_SIZE];

#define DAQmxErrChk(fnName,fnCall) if ((error=(fnCall)) < 0){ StringCbPrintfW(message,MSG_SIZE,L"%s GR failed (%d),(%d). ",fnName,error,__LINE__); CThordaqResonantGalvo::GetInstance()->LogMessage(message,ERROR_EVENT); throw "fnCall";}else{ StringCbPrintfW(message,MSG_SIZE,L"%s GR failed (%d) (%d). ",fnName,error,__LINE__); CThordaqResonantGalvo::GetInstance()->LogMessage(message,VERBOSE_EVENT);}
#define MAX_TASK_WAIT_TIME 10.0

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

long long CThordaqResonantGalvo::_indexOfLastCompletedFrame = 0;
long long CThordaqResonantGalvo::_indexOfCopiedFrame = 0;

long CThordaqResonantGalvo::_DAQDeviceIndex = DEFAULT_CARD_NUMBER;
FrameCirBuffer* CThordaqResonantGalvo::_pFrmBuffer = nullptr;
DataProcessing* CThordaqResonantGalvo::_pDataProcess = nullptr;

vector<UCHAR*> CThordaqResonantGalvo::_pBuffer;
UCHAR* CThordaqResonantGalvo::_pHistoryBuf = nullptr; 

HANDLE CThordaqResonantGalvo::_hFrmBufHandle = NULL;

long CThordaqResonantGalvo::_triggerWaitTimeout = 0;

/**********************************************************************************************//**
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
	_minimizeFlybackCycles = true;
	_triggerWaitTimeout = DEFAULT_TRIGGER_TIMEOUT;
	_pFrmBuffer = NULL;
	SAFE_DELETE_PTR(_pDataProcess);
	SAFE_DELETE_PTR(gPtrMemoryPool);
	_rawFlybackCycle = DEFAULT_FLYBACK_CYCLE;
	_useReferenceForPockelsOutput = FALSE;
	_fieldSizeCalibration = 0;
	_fieldSizeCalibrationXMLvalue = 0;
	_fieldSizeCalibrationAvailable = FALSE;
	_pPockelsMask = nullptr;
	_useZoomArray = FALSE;
	_scannerInitMode = FALSE;
	_current_resonant_scanner_frequency = CrsFrequencyHighPrecision;
	_pockelsMaskWidth = 62;//!hardcode
	_pockelsMaskPhaseShiftPercent = 0.0;
	_pockelsParkAtMinimum = 0;
	_pockelsMaskChanged = FALSE;
	_pockelDigOutput = "";

	for(long i=0; i<MAX_POCKELS_CELL_COUNT; i++)
	{
		_pockelsEnable[i] = false;
		_pockelsLine[i] = "";///<NI connection string for pockel(s)
		_taskHandleAIPockels[i] = 0;
		_pockelsScanVoltageStart[i] = 0.0;
		_pockelsScanVoltageStop[i] = 0.0;
		_pockelsPowerInputLine[i] = "";		
		_pockelsResponseType[i] = 0;
		memset(&_pockelsReadArray[i],0,POCKELS_VOLTAGE_STEPS * sizeof(float64));
	}
	gPtrMemoryPool = new MemoryPool(0x8000000, DAC_DDR3_BUF_OFFSET); // Memory pool: 0 - 0x20000000 acquisition buffer. 0x20000000 - 0x80000000 bitmap buffer.
	_pDataProcess = new DataProcessing();
}
/**********************************************************************************************//**
 * @fn	CThordaqResonantGalvo::~CThordaqResonantGalvo()
 *
 * @brief	Destructor.
 *
 **************************************************************************************************/
CThordaqResonantGalvo::~CThordaqResonantGalvo()
{
	//Reset the device parameter
	SAFE_DELETE_PTR(gPtrMemoryPool);
	_deviceNum = 0;
	_single.release();
	_instanceFlag = false;
}


/**********************************************************************************************//**
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

/**********************************************************************************************//**
 * @fn	long CThordaqResonantGalvo::FindCameras(long &cameraCount)
 *
 * @brief	Searches for the devices.
 *
 * @param [in,out]	cameraCount	Index of Camera.
 *
 * @return	False if no Camera found. True if Camera found.
 **************************************************************************************************/
long CThordaqResonantGalvo::FindCameras(long &cameraCount)
{
	StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorLSMCam::FindCameras");
	LogMessage(_errMsg,VERBOSE_EVENT);
	//MessageBox(NULL, L"test", L"test", MB_DEFBUTTON1);
	// First make sure there is a board installed
	try
	{
		if (ThordaqConnectToBoard(_DAQDeviceIndex) == STATUS_SUCCESSFUL)
		{
			cameraCount = 1;
			_deviceNum = 1;
		}
	}
	catch (...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Hardware communication error ThorDAQ ConnectToBoard failed");
		LogMessage(_errMsg,ERROR_EVENT);
		return FALSE;
	}

	_numNIDAQ = 0;
	//need to do detailed device search here in the future.
	try
	{
		char DevName[256];
		DAQmxGetSysDevNames(DevName, 256);
		std::string temp = DevName;
		if(string::npos != temp.find("Dev1")) // (G/R: Dev1)
		{
			_numNIDAQ = 1;
		}
	}
	catch (...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Hardware communication error DAQmxGetSysDevNames failed");
		LogMessage(_errMsg,ERROR_EVENT);
		return FALSE;
	}

	return cameraCount;
}

/**********************************************************************************************//**
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
		//XML settings retrieval functions will throw an exception if tags or attributes are missing
		//catch each independetly so that as many tags as possible can be read
		if(FALSE == pSetup->GetConfiguration(_field2Theta,_crsFrequencyHighPrecision,_pockelsParkAtMinimum))
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"GetConfiguration from ThorDAQGalvoGalvoSettings.XML failed");
			LogMessage(_errMsg,ERROR_EVENT);
		}
		
		if(FALSE == pSetup->GetCalibration(_fieldSizeCalibrationXMLvalue,_oneXFieldSize,_pockelsPhaseAdjustMicroSec,_pockelsMaskPhaseShiftPercent))
		{
			//if fieldSizeCalibration not exists, set its GetParamInfo Available
			_fieldSizeCalibrationAvailable = FALSE;

			StringCbPrintfW(_errMsg,MSG_SIZE,L"GetCalibration from ThorDAQGalvoGalvoSettings.XML failed. FieldSizeCalibration not available.");
			LogMessage(_errMsg,ERROR_EVENT);
		}
		else
		{
			_fieldSizeCalibrationAvailable = TRUE;
		}

		if(FALSE == pSetup->GetTrigger(_triggerWaitTimeout))
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"GetTrigger from ThorConfocalSettings failed");
			LogMessage(_errMsg,ERROR_EVENT);
		}
		
		//Get Stream Control always using internal clock rate
		if(FALSE == pSetup->GetStreamConfiguration(_imgAcqPty.DCOffset[0],_imgAcqPty.DCOffset[1]))
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"GetStream from ThorDAQGalvoGalvoSettings.XML failed");
			LogMessage(_errMsg,ERROR_EVENT);
		}	

		if(FALSE == pSetup->GetIO(_pockelsVoltageSlopeThreshold,_pockelDigOutput,_pockelsLine[0],_pockelsPowerInputLine[0],_pockelsScanVoltageStart[0],_pockelsScanVoltageStop[0],_pockelsLine[1],_pockelsPowerInputLine[1],_pockelsScanVoltageStart[1],_pockelsScanVoltageStop[1],_pockelsLine[2],_pockelsPowerInputLine[2],_pockelsScanVoltageStart[2],_pockelsScanVoltageStop[2],_pockelsResponseType[0],_pockelsResponseType[1],_pockelsResponseType[2]))
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"GetIO from ThorConfocalSettings failed");
			LogMessage(_errMsg,ERROR_EVENT);
		}

		for(long i=0; i<MAX_POCKELS_CELL_COUNT; i++)
		{
			_pockelsEnable[i] = (_pockelsLine[i].size() > 0) ? true : false;
		}
	}
	catch(...)
	{
		return FALSE;
	}
	//load alignment data
	LoadAlignDataFile();
	//Close the Shutter

	//park the galvo
	MoveGalvoToParkPosition(GalvoResonantY);

	return TRUE;
}

/**********************************************************************************************//**
 * @fn	long CThordaqResonantGalvo::TeardownCamera()
 *
 * @brief	Teardown Camera.
 *
 * @return	A long.
 **************************************************************************************************/
long CThordaqResonantGalvo::TeardownCamera()
{
	StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorLSMCam::TeardownCamera");
	LogMessage(_errMsg,VERBOSE_EVENT);
	
	//Stop the acquisition thread
	SetEvent(_hStopAcquisition);
	//Wait for acqisition's stop
	Sleep(500);
	WaitForSingleObject(_hThreadStopped, 5000);
	
	//terminate daq thread:
	StopDaqBrd();	
	//save the configuration settings

	//park the galvo when exiting
	MoveGalvoToParkPosition(GalvoResonantY);

	//park the pockels cell

	//close the shutter

	//Reset DAQ:
	DAQmxResetDevice("Dev1");
	
	//disconnect DAQ board
	_deviceNum = 0;
    //// Clean all the threads
	SAFE_DELETE_HANDLE(_hAcquisitionThread);
	
	//// Free all the buffers
	SAFE_DELETE_PTR(_pFrmBuffer);
	if(_pBuffer.size() > 0)
	{
		for(int i=0;i < _pBuffer.size();i++)
		{
			VirtualFree(_pBuffer.at(i), 0, MEM_RELEASE);
			_pBuffer.at(i) = NULL;
		}
		_pBuffer.clear();				
	}

	if(_pHistoryBuf != NULL)
	{
		VirtualFree(_pHistoryBuf,0,MEM_RELEASE);
		_pHistoryBuf = NULL;
	}

	//// Delete all the pointers


	return TRUE;
}


/**********************************************************************************************//**
 * @fn	long CThordaqResonantGalvo::PreflightAcquisition(char * pDataBuffer)
 *
 * @brief	Preflight position.
 * @param [in,out]	pDataBuffer	  	Identifier of the Data buffer.
 * @return	A long.
 **************************************************************************************************/
long CThordaqResonantGalvo::PreflightAcquisition(char * pDataBuffer)
{
	long ret = TRUE;

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
			MoveGalvoToCenter(GalvoResonantY);

			MovePockelsToPowerLevel(&_imgAcqPty.pockelPty);
			ret =  TRUE;
			break;
		}
		case ScanMode::WAVEFORM_MODE://test the waveform
		{
			ret = TRUE;
			break;
		}
		default:;
	}
	return ret;
}

/**********************************************************************************************//**
 * @fn	long CThordaqResonantGalvo::SetupAcquisition(char * pDataBuffer)
 *
 * @brief	Setup  Acquisition.
 * @param [in,out]	pDataBuffer	  	Identifier of the Data buffer.
 * @return	A long.
 **************************************************************************************************/
long CThordaqResonantGalvo::SetupAcquisition(char * pDataBuffer)
{
	long ret = TRUE;

	//do not capture data if you are in the centering scan mode, Just Set the Scanner
	if(_imgAcqPty.scanMode == ScanMode::CENTER_SCAN_MODE)
	{
		return ret;
	}

	//allow resonance scanner to be stable if not on already
	/*if((firstFrame) && (FALSE == _scannerInitMode))
	{
		if(_crsFrequencyHighPrecision > 9000)
		{
			Sleep(2200/(_imgAcqPty.fieldSize/MIN_FIELD_SIZE_X) + 200);
		}
		else
		{
			Sleep(200);
		}
	}*/

	//If settings change or system is forced to refresh updating, do update.
	if (!_imgAcqPty.IsEqual(_imgAcqPty_Pre) ||TRUE == _pockelsMaskChanged || _forceSettingsUpdate == TRUE)
	{
		// Disable Force updating
		_forceSettingsUpdate = FALSE;
		
		_pockelsMaskChanged = FALSE;
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
		_imgAcqPty_Pre.flybackCycle = GetFlybackCycle();

		/**********reset the acquisition thread and the event flag*******/
		SetEvent(_hStopAcquisition);
		WaitForSingleObject(_hThreadStopped, INFINITE);

		ResetEvent(_hTriggerTimeout);//Reset the acquisition timeout event flag

		// Release Circular buffer mutex
		if(_hFrmBufHandle != NULL)
		{
			CloseHandle(_hFrmBufHandle);
			_hFrmBufHandle = NULL;
		}
		_hFrmBufHandle = CreateMutex(NULL, false, NULL);

		if (ConfigAcqSettings(&_imgAcqPty_Pre) == FALSE)
		{
			ret = FALSE;
		}
	}
	
	return ret;
}

/**********************************************************************************************//**
 * @fn	long CThordaqResonantGalvo::ConfigAcqSettings(ImgAcqPty* pImgAcqPty)
 *
 * @brief	Configure thordaq settings
 * @param [in,out]	pImgAcqPty	  	Identifier of Image Acquisition Struct.
 * @return	A long.
 **************************************************************************************************/
long CThordaqResonantGalvo::ConfigAcqSettings(ImgAcqPty* pImgAcqPty)
{
	long ret = TRUE;
	//Initiate the Struct
	memset(&_daqAcqCfg, 0, sizeof(IMAGING_CONFIGURATION_STRUCT));
	gPtrMemoryPool->ClearUpMemory();
	if (_current_resonant_scanner_frequency == 0)
	{
		_current_resonant_scanner_frequency = CrsFrequencyHighPrecision;
	}
	// get the ADC sample rate
	_daqAcqCfg.imageCtrl.clockRate = DEFAULT_INTERNALCLOCKRATE;
	if (pImgAcqPty->clockSource == INTERNAL_CLOCK)
	{
		_daqAcqCfg.imageCtrl.clock_source = CLOCK_SOURCE::INTERNAL_80MHZ_REF;
	}else
	{
		_daqAcqCfg.imageCtrl.clock_source = CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE;
		_daqAcqCfg.imageCtrl.clockRate = pImgAcqPty->clockRateExternal; // need test
	}

	// get the resonant scanner frequency
	if (pImgAcqPty->clockSource == INTERNAL_CLOCK)
	{
		_current_resonant_scanner_frequency = CrsFrequencyHighPrecision;
		if (ThordaqGetLineTriggerFrequency(_DAQDeviceIndex,_daqAcqCfg.imageCtrl.clockRate, _current_resonant_scanner_frequency) != STATUS_SUCCESSFUL)
		{	
			return FALSE;
		}
	}
	
	//detemine the X& Y FOV in unit of volt, full swing of waveform,
	//based on field size and the image pixel aspect ratio
	// voltage required is happend to be the mechanical angle of the mirror 
	double theta = (double) pImgAcqPty->fieldSize * _field2Theta;
	GalvoStruct galvo_control;
	galvo_control.amplitude = theta * (double) pImgAcqPty->pixelY / (double)pImgAcqPty->pixelX / 2; // divide by 2 because the mechanical angle is half of the optical angle
	if(TRUE ==_useZoomArray)
	{
		galvo_control.amplitude = galvo_control.amplitude + galvo_control.amplitude *_zoomArray[pImgAcqPty->fieldSize]/100.0;
	}
	galvo_control.amplitude = (pImgAcqPty->yAmplitudeScaler/100.0) * galvo_control.amplitude;

	if (FALSE == pImgAcqPty->galvoEnable && TRUE == _pockelsEnable[0]) //if LineScan is enabled
	{
		galvo_control.amplitude = 0;
	}
	galvo_control.offset = (double) pImgAcqPty->offsetY * _field2Theta / 2;
	galvo_control.park = 10.0;
	galvo_control.scan_direction = pImgAcqPty->verticalScanDirection == 0 ? SCAN_DIRECTION::FORWARD_SCAN : SCAN_DIRECTION::REVERSE_SCAN;

	// detemine size of image frame
	ScanStruct  scan_info;
	scan_info.forward_lines  = (TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode) ? (pImgAcqPty->pixelY / 2) : pImgAcqPty->pixelY;
	scan_info.backward_lines = pImgAcqPty->flybackCycle;
	scan_info.overall_lines  = scan_info.forward_lines + scan_info.backward_lines;


	// set up pixel convertor
	_pDataProcess->SetupDataMap(pImgAcqPty->dataMapMode,pImgAcqPty->channelPolarity);

	//
	switch (pImgAcqPty->triggerMode)
	{
	case ICamera::SW_FREE_RUN_MODE :
		_daqAcqCfg.imageCtrl.frameCnt  = MAX_FRAME_NUM;
		_daqAcqCfg.imageCtrl.triggerMode = TRIGGER_MODE::SOFTWARE_RUN_MODE;
		break;
	case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
		_daqAcqCfg.imageCtrl.frameCnt  = MAX_FRAME_NUM;
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
		_daqAcqCfg.imageCtrl.frameCnt  = (MAX_FRAME_NUM - 1 <= pImgAcqPty->numFrame)? (MAX_FRAME_NUM - 1) : static_cast<ULONG32>(pImgAcqPty->numFrame);
		_daqAcqCfg.imageCtrl.triggerMode = TRIGGER_MODE::SOFTWARE_RUN_MODE;
		break;
	case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
		_daqAcqCfg.imageCtrl.frameCnt  = (MAX_FRAME_NUM - 1 <= pImgAcqPty->numFrame)? (MAX_FRAME_NUM - 1) : static_cast<ULONG32>(pImgAcqPty->numFrame);
		_daqAcqCfg.imageCtrl.triggerMode = TRIGGER_MODE::HARDWARE_TRIGGER_MODE;
		break;
	}

	if (scan_info.overall_lines != 0)
	{
		_frameRate = _current_resonant_scanner_frequency / scan_info.overall_lines;
	}
	//alignment settings
	_daqAcqCfg.imageCtrl.system_mode = SYSTEM_MODE::INTERNAL_RESONANT_GALVO;
	_daqAcqCfg.imageCtrl.channel = static_cast<USHORT>(pImgAcqPty->channel);
	_daqAcqCfg.imageCtrl.imgHSize = static_cast<USHORT>(pImgAcqPty->pixelX);
	_daqAcqCfg.imageCtrl.imgVSize = static_cast<USHORT>(pImgAcqPty->pixelY);
	_daqAcqCfg.imageCtrl.scanMode = (TWO_WAY_SCAN_MODE == pImgAcqPty->scanMode) ? SCAN_MODE::BIDIRECTION_SCAN : SCAN_MODE::UNIDIRECTION_SCAN;
	_daqAcqCfg.imageCtrl.scanDir =  (pImgAcqPty->horizontalFlip == FALSE)? SCAN_DIRECTION::FORWARD_SCAN : SCAN_DIRECTION::REVERSE_SCAN;
	_daqAcqCfg.imageCtrl.alignmentOffset = static_cast<USHORT>(_imgAcqPty.twoWayZonesFine[MAX_FIELD_SIZE_X - pImgAcqPty->fieldSize]);
	_daqAcqCfg.imageCtrl.frameNumPerSec = max(static_cast<ULONG32>(static_cast<ULONG32>(ceil(_frameRate))), 1); // set frequency of interrupt to half second
	if (_daqAcqCfg.imageCtrl.frameCnt < _daqAcqCfg.imageCtrl.frameNumPerSec)
	{
		_daqAcqCfg.imageCtrl.frameNumPerSec = _daqAcqCfg.imageCtrl.frameCnt;
	}
	if (_daqAcqCfg.imageCtrl.frameCnt == MAX_FRAME_NUM) // MAX_FRAME_NUM means continuously scan
	{
		_daqAcqCfg.imageCtrl.frameNumPerSec = 1;
	}

	_daqAcqCfg.resonantGalvoCtrl.vGalvoAmpVal = galvo_control.amplitude;
	_daqAcqCfg.resonantGalvoCtrl.vGalvoOffset = galvo_control.offset;
	_daqAcqCfg.resonantGalvoCtrl.vGalvoParkVal = galvo_control.park;
	_daqAcqCfg.resonantGalvoCtrl.flybackTime = 1.0 / _current_resonant_scanner_frequency * pImgAcqPty->flybackCycle;
	
	//_daqAcqCfg.streamingCtrl 
	_daqAcqCfg.streamingCtrl.fir_filter_enabled = pImgAcqPty->realTimeDataAverage;
	_daqAcqCfg.streamingCtrl.scan_period = _current_resonant_scanner_frequency;

	//coherent sampling
	if (pImgAcqPty->clockSource == EXTERNAL_CLOCK)
	{
		_daqAcqCfg.imageCtrl.threePhotonMode = pImgAcqPty->threePhotonModeEnable;
		_daqAcqCfg.coherentSamplingCtrl.phaseIncrementMode = 2;//0: disable 1:incremental mode 2: static offset
		_daqAcqCfg.coherentSamplingCtrl.phaseOffset = static_cast<USHORT>(pImgAcqPty->laserCoherentSamplingPhase * 8.0 * 16.0 / 100.0);
		_daqAcqCfg.streamingCtrl.channel_multiplexing_enabled = pImgAcqPty->laserCoherentSamplingEnable;
	}


	
	double dac_rate = 0;
	ScanLineStruct scanLine;
	GetDACSamplesPerLine(&scanLine, dac_rate, 1.0 / _current_resonant_scanner_frequency / 2.0);

	BuildGalvoWaveform(&scan_info, &scanLine, dac_rate, &galvo_control);
	BuildPockelsWaveform(&scan_info, &scanLine, dac_rate, &_imgAcqPty.pockelPty);
	//BuildTestWaveform(&scan_info, &scanLine, dac_rate, &galvo_control);

	UCHAR* waveform = new UCHAR[DAC_FIFO_DEPTH * 2];
	memset(waveform,0,DAC_FIFO_DEPTH * 2);
	for (int i = 0; i < DAC_CHANNEL_COUNT; i++)
	{
		ULONG64 startAddress = 0;
		ULONG64 size = 0;
		if (_daqAcqCfg.dacCtrl[i].waveform_buffer_length == 0)
		{
			IMAGING_BUFFER_STRUCT waveform_buffer;
			waveform_buffer.buffer = waveform;
			waveform_buffer.length = DAC_FIFO_DEPTH * 2;
			waveform_buffer.channel = i;
			waveform_buffer.offset = 0;
			LoadDACMemorySettings(waveform_buffer);

			gPtrMemoryPool->GetMemoryPropertyByChannel(waveform_buffer.channel,_daqAcqCfg.dacCtrl[waveform_buffer.channel].waveform_buffer_start_address,_daqAcqCfg.dacCtrl[waveform_buffer.channel].waveform_buffer_length);
			_daqAcqCfg.dacCtrl[waveform_buffer.channel].park_val = 0;
			_daqAcqCfg.dacCtrl[waveform_buffer.channel].offset_val = 0;
			_daqAcqCfg.dacCtrl[waveform_buffer.channel].update_rate = DAC_MIN_UPDATERATE;
			_daqAcqCfg.dacCtrl[waveform_buffer.channel].output_port = waveform_buffer.channel;
		}
	}
	SAFE_DELETE_ARRAY(waveform);

	if (ThordaqSetImagingConfiguration(_DAQDeviceIndex,_daqAcqCfg) != STATUS_SUCCESSFUL)
	{
		//printf("Setup Packet Generator failed, Max Packet Size (%ld) is too large\n", pDgTzrParams.bufferSize);	//MaxPacketSize
		ret = FALSE;
		return ret;
	}else  //Set up Buffer
	{
		if( SetupFrameBuffer() != STATUS_SUCCESSFUL)
		{
			ret = FALSE;
			return ret;
		}
	}
	return ret;
}

/**********************************************************************************************//**
 * @fn	long CThordaqResonantGalvo::SetupFrameBuffer()
 *
 * @brief	Set up Frame Buffer.
 * @return	A long.
 **************************************************************************************************/
long CThordaqResonantGalvo::SetupFrameBuffer()
{
	//BUFFER #1: 1 second frame buffer (frame rate frames for all channels):
	if(_pBuffer.size() > 0)
	{
		for(int i=0;i<_pBuffer.size();i++)
		{
			VirtualFree(_pBuffer.at(i), 0, MEM_RELEASE);
			_pBuffer.at(i) = NULL;
		}
		_pBuffer.clear();				
	}

	size_t AllocSize = _daqAcqCfg.imageCtrl.imgHSize*_daqAcqCfg.imageCtrl.imgVSize*_daqAcqCfg.imageCtrl.frameNumPerSec*2; // Buffer size
	AllocSize = SIZE_T((AllocSize + 1023) & -1024);
	if ((AllocSize == 0) || (AllocSize < 0))
	{
		printf("Invalid Buffer Allocation Size = %d\n", AllocSize);
		return FALSE;
	}
	for(int i = 0; i < static_cast<int>(MAX_CHANNEL_COUNT); i++)
	{
		UCHAR* ptr = (UCHAR *) VirtualAlloc (NULL, AllocSize, MEM_COMMIT, PAGE_READWRITE);
		_pBuffer.push_back(ptr);
		if (_pBuffer.at(i)== NULL)
		{
			printf("Buffer malloc failed at channel %d, Size = %d\n", i, AllocSize);
			return FALSE;
		}
	}
	//BUFFER #2: history buffer for average (1 frame for all channels):
	if(_pHistoryBuf!= NULL)
	{
		VirtualFree(_pHistoryBuf, 0, MEM_RELEASE);
	}
	_pHistoryBuf = (UCHAR *) VirtualAlloc(NULL, MAX_CHANNEL_COUNT * _daqAcqCfg.imageCtrl.imgHSize*_daqAcqCfg.imageCtrl.imgVSize * 2, MEM_COMMIT, PAGE_READWRITE);
	if(_pHistoryBuf == NULL)
	{	
		return FALSE;	
	}

	//BUFFER #3: circular buffer for read (by user) and write (by camera):
	//int channelCount = CountChannelBits(_imgAcqPty.channel); // Do later
	SAFE_DELETE_PTR(_pFrmBuffer);
	_pFrmBuffer = new FrameCirBuffer(_daqAcqCfg.imageCtrl.imgHSize,_daqAcqCfg.imageCtrl.imgVSize, MAX_CHANNEL_COUNT , 2, _daqAcqCfg.imageCtrl.frameNumPerSec * MAX_CHANNEL_COUNT);
	
	return STATUS_SUCCESSFUL;
}


/**********************************************************************************************//**
 * @fn	long CThordaqResonantGalvo::StartAcquisition(char * pDataBuffer)
 *
 * @brief	Start  Acquisition.
 * @param [in,out]	pDataBuffer	  	Identifier of the Data buffer.
 * @return	A long.
 **************************************************************************************************/
long CThordaqResonantGalvo::StartAcquisition(char * pDataBuffer)
{

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
			{
				if (WaitForSingleObject(_hThreadStopped, 0) == WAIT_OBJECT_0)
				{
					//make sure to reload the waveform for the first call of the thread.
					//This ensures the frame trigger is restarted for output
					if(STATUS_SUCCESSFUL != ThordaqStartAcquisition(_DAQDeviceIndex))
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
					if(0 != ThordaqStartAcquisition(_DAQDeviceIndex))
					{ return FALSE; }
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
				//	if(0 != ThordaqStartAcquisition(_DAQDeviceIndex))
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
					//make sure to reload the waveform for the first call of the thread.
			
					if(0 != ThordaqStartAcquisition(_DAQDeviceIndex))
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
	_indexOfCopiedFrame = -1;
	ResetEvent(_hThreadStopped);
	HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &(CThordaqResonantGalvo::StartFrameAcqProc), (void *) this, 0, &threadID);
	SetThreadPriority(handle, THREAD_PRIORITY_HIGHEST);
	return handle;
}

/**********************************************************************************************//**
 * @fn	UINT CThordaqResonantGalvo::StartFrameAcqProc(LPVOID instance)
 *
 * @brief	Start  Acquisition Thread.
 * @param 	instance	  	Acquisition Thread instance.
 * @return	A uint.
 **************************************************************************************************/
UINT CThordaqResonantGalvo::StartFrameAcqProc(LPVOID instance)
{
	int				frameWritten = 0;
	THORDAQ_STATUS	status = STATUS_SUCCESSFUL;
	long long		targetFrames = static_cast<long long>(_daqAcqCfg.imageCtrl.frameCnt);// do later
	int frameSize = _daqAcqCfg.imageCtrl.imgHSize * _daqAcqCfg.imageCtrl.imgVSize;
	int frameSizeByte = frameSize * 2;
	ULONG			TransferSize = static_cast<ULONG>(frameSizeByte * _daqAcqCfg.imageCtrl.frameNumPerSec);
	ULONG           size;
	long chFrameNum[4] = {0, 0, 0, 0}; 
	int targetFrmNum =0;
	const double EXTRA_DELAY = 20;
	double regular_timeout = 10.0 * 1000 * _daqAcqCfg.imageCtrl.imgVSize / _crsFrequencyHighPrecision * _daqAcqCfg.imageCtrl.frameNumPerSec + EXTRA_DELAY;
	double hardware_trigger_timeout = _triggerWaitTimeout * 1000;
	double timeout = regular_timeout;
	BOOL hardware_timeout_enable = FALSE; 
	BOOL is_hardware_captured = FALSE;

	if (_daqAcqCfg.imageCtrl.triggerMode == HARDWARE_TRIGGER_MODE)
	{
		hardware_timeout_enable = TRUE;
	}
	ULONG32 bank = 0;
	long long frame_left = targetFrames - 1;
	do
	{	
		frame_left = targetFrames - 1 - _indexOfLastCompletedFrame;
		if (frame_left >= _daqAcqCfg.imageCtrl.frameNumPerSec)
		{
			for (int i = 0; i < MAX_CHANNEL_COUNT; i++)
			{
				if ((_daqAcqCfg.imageCtrl.channel & (0x0001 << i)) != 0x0000)
				{
					timeout = (hardware_timeout_enable && !is_hardware_captured) ? hardware_trigger_timeout : regular_timeout;
					size = TransferSize;
					//Execute another bufRead for 1 second frames:
					status = ThordaqReadChannel(_DAQDeviceIndex, i, &size, _pBuffer.at(i), timeout);
					if(status == STATUS_SUCCESSFUL)
					{	
						//Do process buffer until target frame number:
						if(size > 0)
						{	
							chFrameNum[i] = static_cast<long>(size / frameSizeByte);
						}
						if (hardware_timeout_enable)
						{
							is_hardware_captured = TRUE;
						}
					}
					else
					{
						if (hardware_timeout_enable)
						{
							StringCbPrintfW(message,MSG_SIZE,L"External trigger is timed out after %d seconds.", _triggerWaitTimeout);
							MessageBox(NULL,message,L"Trigger Error", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
						}
						SetEvent(_hTriggerTimeout);
						((CThordaqResonantGalvo *)instance)->StopDaqBrd();
						//ShutdownPacketMode(_DAQDeviceIndex);
						SetEvent(_hThreadStopped);
						return status;
					}	
				}
			}
		}else
		{
			ULONG32 frame_count = 0;
			LONG scan_completed = FALSE;
			for (int i = 0; i < 10; i++)
			{
				DWORD regular_timeout = static_cast<DWORD>(1000 * _daqAcqCfg.imageCtrl.imgVSize / _crsFrequencyHighPrecision * frame_left);
				Sleep(regular_timeout);
				if ( ThordaqGetTotalFrameCount(_DAQDeviceIndex,frame_count)== STATUS_SUCCESSFUL && (frame_count == targetFrames))
				{
					scan_completed = TRUE;
					for (int i = 0; i < MAX_CHANNEL_COUNT; i++)
					{
						if ((_daqAcqCfg.imageCtrl.channel & (0x0001 << i)) != 0x0000)
						{
							size = static_cast<ULONG>(frameSizeByte * frame_left);
							ULONG64 CardOffset = i * DDR3_SINGLEIMAGE_CHANNEL_CAP + DDR3_IMAGE_2ndPINGPONGBUFFstart * bank;
							status = ThordaqPacketReadBuffer(_DAQDeviceIndex, CardOffset, &size, _pBuffer.at(i), 0xffffffff);
							if(status == STATUS_SUCCESSFUL)
							{	
								//Do process buffer until target frame number:
								if(size > 0)
								{	
									chFrameNum[i] = static_cast<long>(size / frameSizeByte);
								}
							}else
							{
								SetEvent(_hTriggerTimeout);
								((CThordaqResonantGalvo *)instance)->StopDaqBrd();
								//ShutdownPacketMode(_DAQDeviceIndex);
								SetEvent(_hThreadStopped);
								return status;
							}
						}
					}
				}
				if (scan_completed == TRUE)
				{
					break;
				}
			}
			if (scan_completed == FALSE)
			{
				SetEvent(_hTriggerTimeout);
				((CThordaqResonantGalvo *)instance)->StopDaqBrd();
				//ShutdownPacketMode(_DAQDeviceIndex);
				SetEvent(_hThreadStopped);
				return status;
			}
		}
		
		if (bank == 0)
		{
			bank = 1;
		}else
		{
			bank = 0;
		}
		hardware_timeout_enable = FALSE; // only used for first frame
		ResetEvent(_hTriggerTimeout);

		// Done copy channels data  Write to the buffer
		//_pFrmBuffer->EnterBuffer();
		targetFrmNum = 0;
		for (int i = 0; i < MAX_CHANNEL_COUNT; i++)
		{
			if ((_daqAcqCfg.imageCtrl.channel & (0x0001 << i)) != 0x0000)
			{				_pDataProcess->ProcessBuffer((USHORT*)(_pBuffer.at(i)),i,TransferSize / 2);
				targetFrmNum  = max(targetFrmNum, (int)chFrameNum[i]);
			}
		}
		//_indexOfLastCompletedFrame += targetFrmNum; //do later
		size = targetFrmNum * frameSizeByte;
		USHORT *bPtr = NULL; //Identifier pointer for history buffer
		USHORT *tPtr = NULL; //Identifier pointer for captured data
		double factor1 = (1/(double)(_imgAcqPty.averageNum));
		double factor2 = (_imgAcqPty.averageNum - 1)/(double)(_imgAcqPty.averageNum);

		if(1 == HandleFrameBuf(TRUE, TIMEOUT_MS)) // 
		{
			if(1 < _imgAcqPty.averageNum && _imgAcqPty.averageMode == FRM_CUMULATIVE_MOVING)
			{
				//combine all average frames into 1:

				//write to history buffer:
				bPtr = (USHORT*)_pHistoryBuf;
				for(int chID = 0; chID < MAX_CHANNEL_COUNT; chID++)
				{
					if ((_daqAcqCfg.imageCtrl.channel & (0x0001 << chID)) != 0x0000)
					{
						tPtr = (USHORT*)_pBuffer.at(chID); // Get the pointer
						for(int p = 0; p < frameSize; p++)					
						{
							*(bPtr + p + chID * frameSize) = static_cast<USHORT>((*(bPtr + p + chID * frameSize)) * factor2 + (*(tPtr + p)) * factor1);

						}
					}else
					{
						for(int p = 0; p < frameSize; p++)					
						{
							*(bPtr + p + chID * frameSize) = 0x0000;
						}
					}	
				}
				//write history buffer to circular buffer:
				frameWritten = static_cast<int>((_pFrmBuffer->WriteFrames(_pHistoryBuf , MAX_CHANNEL_COUNT))/MAX_CHANNEL_COUNT);
			}
			else
			{
				//no average, write buffer to circular buffer:
				int frmNum = 0;
				for(int j = 0; j < targetFrmNum; j++)
				{		
					for(int chID = 0; chID < MAX_CHANNEL_COUNT; chID++)
					{
						frmNum += static_cast<int>(_pFrmBuffer->WriteFrames(((UCHAR *)_pBuffer.at(chID) + j * frameSizeByte), 1));
					}
				}
				frameWritten = static_cast<int>(frmNum / MAX_CHANNEL_COUNT);
			}

			HandleFrameBuf(FALSE, TIMEOUT_MS);
			_indexOfLastCompletedFrame += frameWritten;
		}
	} while ((_indexOfLastCompletedFrame < (targetFrames - 1)) && (WaitForSingleObject(_hStopAcquisition, 0) != WAIT_OBJECT_0));
	

	//done capture:
	//delete pParams;
	//Sleep(_daqAcqCfg.resonantGalvoCtrl.flybackTime * 1000);
	((CThordaqResonantGalvo *)instance)->StopDaqBrd();
	//ShutdownPacketMode(_DAQDeviceIndex);
	SetEvent(_hThreadStopped);
	return 0;
}

void CThordaqResonantGalvo::StopDaqBrd()
{
	ThordaqStopAcquisition(_DAQDeviceIndex);
}

long CThordaqResonantGalvo::StatusAcquisition(long &status)
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
	if(1 == HandleFrameBuf(TRUE,TIMEOUT_MS))
	{
		HandleFrameBuf(FALSE, TIMEOUT_MS);
		if(_indexOfCopiedFrame < _indexOfLastCompletedFrame)
		{
			status = ICamera::STATUS_READY;
			printf("_indexOfLastCompletedFrame = %d; _indexOfCopiedFrame = %d.\n", _indexOfLastCompletedFrame, _indexOfCopiedFrame);
		//		StringCbPrintfW(message,MSG_SIZE,L"Status now is ready with frame buffer size of %d.\n", _indexOfLastCompletedFrame - _indexOfCopiedFrame);
		//		LogMessage(message,VERBOSE_EVENT);

			if ( _indexOfLastCompletedFrame - _indexOfCopiedFrame > _daqAcqCfg.imageCtrl.frameNumPerSec)
			{
				status = ICamera::STATUS_ERROR;
			}
		}	
		else
		{
			status = ICamera::STATUS_BUSY;
		}
	}
	

	return ret;
}

long CThordaqResonantGalvo::StatusAcquisitionEx(long &status,long &indexOfLastCompletedFrame)
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

	indexOfLastCompletedFrame = static_cast<long>(_indexOfLastCompletedFrame);

	return ret;
}

long CThordaqResonantGalvo::CopyAcquisition(char *pDataBuffer, void* frameInfo)
{	
	long moreAvailable;	
	return CopyAcquisition(pDataBuffer, moreAvailable);	
}

long CThordaqResonantGalvo::CopyAcquisition(char * pDataBuffer, long &moreAvailable)
{
	long ret = TRUE;

	//do not capture data if you are in the centering scan mode
	if(_imgAcqPty_Pre.scanMode == CENTER_SCAN_MODE)
	{
		return ret;
	}
	int frameSizeByte = _imgAcqPty_Pre.pixelX * _imgAcqPty_Pre.pixelY * 2; // frame size count in bytes

	//int selectedChannels = CountChannelBits(_imgAcqPty_Pre.channel);
	//int channelCnt  = (selectedChannels > 1) ? 4 : 1;
	int channelCnt  = 4;
	
	size_t frameNum = 0;
	if(_indexOfCopiedFrame == _indexOfLastCompletedFrame)
	{
		return FALSE;
	}

	double averageCumulativeNum = (_imgAcqPty_Pre.averageMode == 0)? 1.0 : _imgAcqPty_Pre.averageNum;


	UCHAR* pTempBuf= (UCHAR*)malloc(frameSizeByte * 4);


	if(1 == HandleFrameBuf(TRUE,TIMEOUT_MS))
	{
		if( _pFrmBuffer->ReadFrames(pTempBuf, channelCnt) > 0)
		{	
			//if(BrdCheckFrmBufferSize(_digiParams.cardNum, &frameNum) > 0)
			//{
			//	moreAvailable = static_cast<long>(frameNum);				
			//	printf("Frame # %d are ready to be copied.\n", moreAvailable);
			//	StringCbPrintfW(message,MSG_SIZE,L"Frame # %d are ready to be copied.\n", moreAvailable);
			//	LogMessage(message, VERBOSE_EVENT);			
			//}
			_indexOfCopiedFrame++;
		}
		HandleFrameBuf(FALSE, TIMEOUT_MS);


		for (int i = 0,j = 0; i < MAX_CHANNEL_COUNT; i++)
		{
			if ((_daqAcqCfg.imageCtrl.channel & (0x0001 << i)) != 0x0000)
			{
				memcpy(pDataBuffer + frameSizeByte * j++,  pTempBuf + frameSizeByte * i, frameSizeByte);
			}
		}

		/*switch(_imgAcqPty_Pre.channel)
		{
		case 1:
			memcpy(pDataBuffer,  pTempBuf, frameSizeByte);
			break;
		case 2:
			memcpy(pDataBuffer, pTempBuf + frameSizeByte, frameSizeByte);
			break;
		case 4:
			memcpy(pDataBuffer,  pTempBuf + 2 * frameSizeByte, frameSizeByte);
			break;
		case 8:
			memcpy(pDataBuffer, pTempBuf + 3 * frameSizeByte, frameSizeByte);
			break;
		default:
			memcpy(pDataBuffer, pTempBuf, 4 * frameSizeByte);
		}*/

	}

	free(pTempBuf);

	return ret;
}

long CThordaqResonantGalvo::CopyAcquisitionInlcudingDisabledChannels(char * pDataBuffer)
{
	long ret = TRUE;
	return ret;
}
long CThordaqResonantGalvo::CopyAcquisitionSkippingDisabledChannels(char * pDataBuffer)
{
	long ret = TRUE;
	return ret;
}
long CThordaqResonantGalvo::PostflightAcquisition(char * pDataBuffer)
{
	try
	{
		//do not capture data if you are in the centering scan mode
		if(_imgAcqPty.scanMode == CENTER_SCAN_MODE)
		{
			MoveGalvoToParkPosition(GalvoResonantY);
			MovePockelsToPowerLevel(&_imgAcqPty_Pre.pockelPty);
			return TRUE;
		}
		//force the hardware trigger event if the post flight function is called
		SetEvent(_hHardwareTriggerInEvent);
		ResetEvent(_hHardwareTriggerInEvent);
		//IDAQCloseNITasks();	

		SetEvent(_hStopAcquisition);
	
		//park the pockels cell
		MovePockelsToParkPosition(&_imgAcqPty_Pre.pockelPty);

		//for the trigger output to low
		//IDAQSetFrameTriggerOutLow();

		while(WaitForSingleObject(_hThreadStopped, 10000) != WAIT_OBJECT_0)
		{
			Sleep(10);
		}

		//stop digitizer board:
		//StopDaqBrd();

		MoveGalvoToParkPosition(GalvoResonantY);
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

void CThordaqResonantGalvo:: LogMessage(wchar_t *message,long eventLevel)
{
#ifdef LOGGING_ENABLED
	qlogDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

long CThordaqResonantGalvo::SetParamString(const long paramID, wchar_t * str)
{
	long ret = TRUE;
	return ret;
}
long CThordaqResonantGalvo::GetParamString(const long paramID, wchar_t * str, long size)
{
	long ret = FALSE;

	switch(paramID)
	{
	case ICamera::PARAM_DETECTOR_NAME:
		{
			const wchar_t * pDetectorName = L"ThordaqResonant";
			wcscpy_s(str,20, pDetectorName);
			ret = TRUE;
		}
		break;
	default:
		break;
	}

	return ret;
}

long CThordaqResonantGalvo::SetParamBuffer(const long paramID, char * pBuffer, long size)
{
	long ret = TRUE;

	switch (paramID)
	{
	case ICamera::PARAM_LSM_POCKELS_MASK:
		{
			SAFE_DELETE_PTR(_pPockelsMask);

			try
			{
				_pPockelsMask = new char[size];
				_pockelsMaskSize = size;
				memcpy(_pPockelsMask,pBuffer,size);
				_pockelsMaskChanged = TRUE;
			}
			catch(...)
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Could not allocate pockels mask");
				LogMessage(_errMsg,ERROR_EVENT);
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

long CThordaqResonantGalvo::GetParamBuffer(const long paramID, char * pBuffer, long size)
{
	long ret = TRUE;

	switch (paramID)
	{
	case ICamera::PARAM_LSM_POCKELS_MASK:
		{
			if((pBuffer != NULL) && (_pPockelsMask != NULL))
			{
				if(_pockelsMaskSize <= size)
				{
					memcpy(pBuffer,_pPockelsMask,_pockelsMaskSize);
				}
			}
		}
		break;
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_0:
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_1:
	case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_2:
		{
			long index = 0;
			switch(paramID)
			{
			case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_0:index = 0;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_1:index = 1;break;
			case ICamera::PARAM_LSM_POCKELS_MIN_MAX_PLOT_2:index = 2;break;
			}

			if(POCKELS_VOLTAGE_STEPS * sizeof(float64) <= size)
			{
				memcpy(pBuffer,&_pockelsReadArray[index],POCKELS_VOLTAGE_STEPS * sizeof(float64));
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

void CThordaqResonantGalvo::SetStatusHandle(HANDLE handle)
{

}

long CThordaqResonantGalvo::MoveGalvoToParkPosition(int Galvo_Y_channel)
{
	long ret = FALSE;

	if (ThordaqSetDACParkValue(_DAQDeviceIndex,Galvo_Y_channel,10.0) != STATUS_SUCCESSFUL)
	{
		ret = 0;
	}

	return ret;
}
long CThordaqResonantGalvo::MoveGalvoToCenter(int Galvo_Y_channel)
{
	long ret = FALSE;

	
	if (ThordaqSetDACParkValue(_DAQDeviceIndex,Galvo_Y_channel,0) != STATUS_SUCCESSFUL)
	{
		ret = 0;
	}
	
	return ret;
}

long CThordaqResonantGalvo::MovePockelsToParkPosition(PockelPty* pockelPty)
{
	long ret = 0;

	for (int i = 0; i < MAX_POCKELS_CELL_COUNT; i++)
	{
		if (_pockelsEnable[i] == FALSE)
		{
			continue;
		}

		double pockelsSetVal0 = pockelPty->pockelsMinVoltage[i] + (pockelPty->pockelsMaxVoltage[i] - pockelPty->pockelsMinVoltage[i]) * pockelPty->pockelsPowerLevel[i];
		double park_value = (TRUE == _pockelsParkAtMinimum) ? pockelPty->pockelsMinVoltage[i] : pockelsSetVal0;
		if (ThordaqSetDACParkValue(_DAQDeviceIndex,Pockel1+i,park_value) != STATUS_SUCCESSFUL)
		{
			ret = FALSE;
		}
	}

	return ret;
}

long CThordaqResonantGalvo::MovePockelsToPowerLevel(PockelPty* pockelPty)
{
	long retVal = 0;

	for (int i = 0; i < MAX_POCKELS_CELL_COUNT; i++)
	{
		if (_pockelsEnable[i] == FALSE)
		{
			continue;
		}

		double pockelsOnVoltage = pockelPty->pockelsMinVoltage[i] + (pockelPty->pockelsMaxVoltage[i] - pockelPty->pockelsMinVoltage[i]) * pockelPty->pockelsPowerLevel[i];
		if (ThordaqSetDACParkValue(_DAQDeviceIndex,Pockel1+i,pockelsOnVoltage) != STATUS_SUCCESSFUL)
		{
			retVal = FALSE;
		}
	}
	return retVal;
}

/// <summary> Cancels any hardware triggers the camera is currently waiting on </summary>
void CThordaqResonantGalvo::StopHardwareWaits()
{
	SetEvent(_hStopAcquisition);
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

//Enter/leave frame buffer to read/write:
UINT CThordaqResonantGalvo::HandleFrameBuf(int enter,DWORD timeOut)
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



/// <summary> Calculates the minimum value for flyback cycles the current settings can support </summary>
/// <returns> The calculated minimum flyback cycles </returns>
long CThordaqResonantGalvo::GetMinFlybackCycle()
{
	//Calculation to determine flybacklines:
	//detemine the Y FOV in unit of volt, full swing of waveform, based on field size and the image pixel aspect ratio
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

	//Setup backwardlines:
	long minFlybackCycle = 0;
	if (fieldY_volt <= 1.0) //up to fieldSize: 22 // if (fieldY_volt <= 1.0) //NI AO amplitude less than 1.0
		minFlybackCycle = 4;
	else if(fieldY_volt <= 3.0) //up to fieldSize: 66
		minFlybackCycle = 6;
	else if(fieldY_volt <= 7.2) //up to fieldSize: 159
		minFlybackCycle = 8;
	else if(fieldY_volt <= 9.9) //fieldSize: 219
		minFlybackCycle = 12;
	else
		minFlybackCycle = 16;

	return minFlybackCycle;
}


/// <summary> Returns the currently number of flyback cycles to use, modified if needed to fall within
/// the minimum the current settings support </summary>
/// <returns> The flybackCycle parameter the hardware should use </returns>
long CThordaqResonantGalvo::GetFlybackCycle()
{
	long minFlybackCycle = GetMinFlybackCycle();
	if(_minimizeFlybackCycles || minFlybackCycle > _rawFlybackCycle)
	{
		return minFlybackCycle;
	}
	else if(GetFlybackTime(_rawFlybackCycle) > MAX_FLYBACK_TIME)
	{
		return static_cast<long>(MAX_FLYBACK_TIME * CrsFrequencyHighPrecision);
	}
	else
	{
		return _rawFlybackCycle;
	}
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
	double lineTime = 1 / CrsFrequencyHighPrecision ;
	return flybackCycles * lineTime;
}

long CThordaqResonantGalvo::BuildGalvoWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine, double dac_rate, GalvoStruct* galvoCtrl)
{
	UINT samples_per_line = 2 * scanLine->samples_idle + scanLine->samples_scan;
	UINT backward_data_num = samples_per_line * 2 * ((UINT)scanInfo->backward_lines - 2);
	UINT forward_data_num  = samples_per_line * 2 * (UINT)scanInfo->forward_lines;

	UINT total_samples = forward_data_num + backward_data_num + DAC_FIFO_DEPTH;
	USHORT* pGalvoWaveform = new USHORT[total_samples];

	double half_P2P_amp_Y = galvoCtrl->amplitude / 2.0 / GALVO_RESOLUTION;
	double amp_offset_Y   = galvoCtrl->offset / GALVO_RESOLUTION + 0x8000;
	double yDirection = (SCAN_DIRECTION::FORWARD_SCAN == galvoCtrl->scan_direction )? 1.0 : -1.0;
	double galvoYFwdStep = (galvoCtrl->amplitude/(double) (forward_data_num)/ GALVO_RESOLUTION);

	USHORT waveform_start = max(0,min(USHRT_MAX, static_cast<USHORT>(amp_offset_Y - yDirection * half_P2P_amp_Y) ));

	
	for (ULONG32 j = 0; j < (ULONG32)forward_data_num; j++)
	{
		*(pGalvoWaveform + j) = max(0, min(USHRT_MAX,static_cast<USHORT>(yDirection * (galvoYFwdStep * (double)(j+1)))));
	}

	for (ULONG32 j = 0; j < (ULONG32)backward_data_num; j++)
	{
		*(pGalvoWaveform +(ULONG32)(samples_per_line * 2 * scanInfo->forward_lines + j)) = static_cast<USHORT>(yDirection * (galvoYFwdStep * (double)forward_data_num) * (cos(M_PI * (double)(j + 1) / (double)backward_data_num) / 2.0 + 0.5));
	}

	for (ULONG32 j = 0; j < (ULONG32)DAC_FIFO_DEPTH; j++)
	{
		*(pGalvoWaveform +(ULONG32)(samples_per_line * 2 * scanInfo->forward_lines + backward_data_num + j)) = 0;
	}

	IMAGING_BUFFER_STRUCT DACMemorySetting;
	DACMemorySetting.buffer = (UCHAR*)(pGalvoWaveform + FIFO_DELAY_SAMPLES);
	DACMemorySetting.channel = GalvoResonantY;
	DACMemorySetting.length = (total_samples - FIFO_DELAY_SAMPLES) * 2;
	DACMemorySetting.offset = 0;
	LoadDACMemorySettings(DACMemorySetting);
		
	gPtrMemoryPool->GetMemoryPropertyByChannel(DACMemorySetting.channel,_daqAcqCfg.dacCtrl[DACMemorySetting.channel].waveform_buffer_start_address,_daqAcqCfg.dacCtrl[DACMemorySetting.channel].waveform_buffer_length);
	_daqAcqCfg.dacCtrl[DACMemorySetting.channel].park_val = galvoCtrl->park;
	_daqAcqCfg.dacCtrl[DACMemorySetting.channel].offset_val = galvoCtrl->offset - yDirection * galvoCtrl->amplitude / 2.0;
	_daqAcqCfg.dacCtrl[DACMemorySetting.channel].update_rate = dac_rate;
	_daqAcqCfg.dacCtrl[DACMemorySetting.channel].output_port = DACMemorySetting.channel;
	_daqAcqCfg.dacCtrl[DACMemorySetting.channel].flyback_samples = backward_data_num;

	/*string waveformFile = "Galvo_waveform.txt";
	ofstream myfile (waveformFile);
	if (myfile.is_open())
	{
		for (int i = 0; i < total_samples; i++)
		{
			myfile << std::fixed << std::setprecision(8) << (*(pGalvoWaveform+i));
			myfile << "\n";
		}
	myfile.close();
	}*/

	SAFE_DELETE_ARRAY(pGalvoWaveform);
	return TRUE;
}

long CThordaqResonantGalvo::BuildTestWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine, double dac_rate, GalvoStruct* galvoCtrl)
{
	UINT samples_per_line = 2 * scanLine->samples_idle + scanLine->samples_scan;
	UINT backward_data_num = samples_per_line * 2 * ((UINT)scanInfo->backward_lines - 2);
	UINT forward_data_num  = samples_per_line * 2 * (UINT)scanInfo->forward_lines;

	UINT total_samples = forward_data_num + backward_data_num + DAC_FIFO_DEPTH;
	USHORT* pGalvoWaveform = new USHORT[total_samples];

	double half_P2P_amp_Y = galvoCtrl->amplitude / 2.0 / GALVO_RESOLUTION;
	double amp_offset_Y   = galvoCtrl->offset / GALVO_RESOLUTION + 0x8000;
	double yDirection = (SCAN_DIRECTION::FORWARD_SCAN == galvoCtrl->scan_direction )? 1.0 : -1.0;
	double galvoYFwdStep = (galvoCtrl->amplitude/(double) (forward_data_num)/ GALVO_RESOLUTION);

	USHORT waveform_start = max(0,min(USHRT_MAX, static_cast<USHORT>(amp_offset_Y - yDirection * half_P2P_amp_Y) ));
	int index = 0;
	for (int i = 0; i < scanInfo->forward_lines; i++)
	{
		for (UINT j = 0; j < 2 * samples_per_line; j++)
		{
			*(pGalvoWaveform + index++) = min(USHRT_MAX, max(0, static_cast<USHORT>(half_P2P_amp_Y - half_P2P_amp_Y * cos((double)j / (2.0* (double)samples_per_line) * 2.0 * M_PI))));
		}
	}
	for (ULONG32 j = 0; j < (ULONG32)backward_data_num; j++)
	{
		*(pGalvoWaveform +(ULONG32)(samples_per_line * 2 * scanInfo->forward_lines + j)) = 0;
	}

	for (ULONG32 j = 0; j < (ULONG32)DAC_FIFO_DEPTH; j++)
	{
		*(pGalvoWaveform +(ULONG32)(samples_per_line * 2 * scanInfo->forward_lines + backward_data_num + j)) = 0;
	}

	IMAGING_BUFFER_STRUCT DACMemorySetting;
	DACMemorySetting.buffer = (UCHAR*)(pGalvoWaveform + FIFO_DELAY_SAMPLES);
	DACMemorySetting.channel = GalvoResonantY;
	DACMemorySetting.length = (total_samples - FIFO_DELAY_SAMPLES) * 2;
	DACMemorySetting.offset = 0;
	LoadDACMemorySettings(DACMemorySetting);
	
	for (int i = 0; i < DAC_CHANNEL_COUNT; i++)
	{
		gPtrMemoryPool->GetMemoryPropertyByChannel(DACMemorySetting.channel,_daqAcqCfg.dacCtrl[i].waveform_buffer_start_address,_daqAcqCfg.dacCtrl[i].waveform_buffer_length);
		_daqAcqCfg.dacCtrl[i].park_val = galvoCtrl->park;
		_daqAcqCfg.dacCtrl[i].offset_val = galvoCtrl->offset - yDirection * galvoCtrl->amplitude / 2.0;
		_daqAcqCfg.dacCtrl[i].update_rate = dac_rate;
		_daqAcqCfg.dacCtrl[i].output_port = i;
		_daqAcqCfg.dacCtrl[i].flyback_samples = backward_data_num;
	}
	

	/*string waveformFile = "Galvo_waveform.txt";
	ofstream myfile (waveformFile);
	if (myfile.is_open())
	{
		for (int i = 0; i < total_samples; i++)
		{
			myfile << std::fixed << std::setprecision(8) << (*(pGalvoWaveform+i));
			myfile << "\n";
		}
	myfile.close();
	}*/

	SAFE_DELETE_ARRAY(pGalvoWaveform);
	return TRUE;
}

long CThordaqResonantGalvo::BuildPockelsWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine, double dac_rate, PockelPty* pockelPty)
{
	UINT samples_per_line = 2 * scanLine->samples_idle + scanLine->samples_scan;
	for (int i = 0; i < MAX_POCKELS_CELL_COUNT; i++)
	{
		if (_pockelsEnable[i] == FALSE)
		{
			continue;
		}

		UINT backward_data_num = samples_per_line * 2 * ((UINT)scanInfo->backward_lines - 2);
		int total_samples = samples_per_line * 2 * (UINT)scanInfo->forward_lines + backward_data_num + DAC_FIFO_DEPTH;

		USHORT* pPockelWaveform = new USHORT[total_samples];

		double pockelsOnVoltage = pockelPty->pockelsMinVoltage[i] + (pockelPty->pockelsMaxVoltage[i] - pockelPty->pockelsMinVoltage[i]) * pockelPty->pockelsPowerLevel[i];

		USHORT pockels_output_low_ushort = 0;
		USHORT pockels_output_high_ushort = static_cast<USHORT>((pockelsOnVoltage -  pockelPty->pockelsMinVoltage[i])/ GALVO_RESOLUTION);


		for (int j = 0; j < scanInfo->forward_lines; j++)
		{
			UINT sample_index = 0 ; //pockel cell index ptr
			//There are 5 blanking regions that must be accomodated
			//1-Beginning of a front line scan
			//2-End of a front line scan
			//3-Beginning of a back line scan
			//4-End of a back line scan
			//5-Backscan

			UINT blanking_samples = static_cast<UINT>(pockelPty->pockelsLineBlankingPercentage[i] * samples_per_line);
			UINT beginingOfFrontLineScan = blanking_samples;
			UINT endOfFrontLineScan      = samples_per_line - blanking_samples;
			UINT beginingOfBackLineScan  = static_cast<UINT>((samples_per_line) + blanking_samples);
			UINT endOfBackLineScan       = static_cast<UINT>(samples_per_line*2 - blanking_samples);

			long usePockelsMask = FALSE;
			long width = _pockelsMaskWidth;

			//if the mode is enabled, the pointer is valid, and the offset not being applied in X
			if(pockelPty->pockelsMaskEnable[i] && _pPockelsMask && (_imgAcqPty_Pre.offsetX == 0))
			{
				const long BYTES_PER_PIXEL = 2;
				//determine if the size of the mask matches the pockels waveform output
				if(_pockelsMaskSize == width * _imgAcqPty_Pre.pixelY * BYTES_PER_PIXEL)
				{
					usePockelsMask = TRUE;
				}
			}

			double const W1 = (width - 1);
			double const PHASE_SHIFT = M_PI*_pockelsMaskPhaseShiftPercent/100;

			

			///        |>-----beginingOfFrontLineScan-----------------endOfFrontLineScan--------->-|
			///                                                                                    |
			///        |<-----endOfBackLineScan-----------------------beginingOfBackLineScan-----<-|

			
			for (; sample_index < beginingOfFrontLineScan;)
			{
				pPockelWaveform[j * 2* samples_per_line + sample_index++] = pockels_output_low_ushort;
			}

			for (; sample_index < endOfFrontLineScan;)
			{
				//To account for the sinusoidal movement of the resonant scanner
				//the x offset on the mask is calculated using a cosine and shifting it by 90deg
				//and adding 1 to start at 0 and increase as sample_index increases "(cos(PI + PI * sample_index / (samples) + C1) + 1) / 2.0"
				//the phase shift "PHASE_SHIFT" is needed to accomodate for the delay in the pockels modulator
				long xOffset = 0;							
				if (FALSE == _imgAcqPty_Pre.horizontalFlip)
				{								
					xOffset =  static_cast<long>(floor(W1 * (cos(M_PI + M_PI * sample_index / (samples_per_line) + PHASE_SHIFT) + 1) / 2.0 + 0.5));
				}
				else
				{
					//if there is a horizontal flip, then start at the end of the line
					xOffset =  static_cast<long>(floor(W1 * (cos(M_PI + M_PI * ((samples_per_line - 1) - sample_index) / (samples_per_line - 1) - PHASE_SHIFT) + 1) / 2.0 + 0.5));
				}

				long maskOffset = (_imgAcqPty_Pre.scanMode == TWO_WAY_SCAN_MODE) ? j * (width*2) + xOffset : j * (width) + xOffset ;

				unsigned short * pMask = (unsigned short*)_pPockelsMask;
				pMask += maskOffset;

				if(TRUE==usePockelsMask)
				{
					//if mask value is greater than 0 and invert is off, or mask value is 0 (or less) and invert is on
					//then set the pockels to the on voltage
					if((0 != *pMask && !pockelPty->pockelsMaskInvert[i]) || 0 == *pMask && pockelPty->pockelsMaskInvert[i])
					{
						StringCbPrintfW(message,MSG_SIZE, L"ThorLSMCam Mask offset %d mask value %d",maskOffset,*pMask);
						LogMessage(message,VERBOSE_EVENT);
						pPockelWaveform[j * 2* samples_per_line + sample_index++] = pockels_output_high_ushort;
					}
					else
					{
						pPockelWaveform[j * 2* samples_per_line + sample_index++] = pockels_output_low_ushort;
					}
				}
				else
				{
					pPockelWaveform[j * 2* samples_per_line + sample_index++] = pockels_output_high_ushort;
				}
			}
			for (; sample_index < beginingOfBackLineScan;)
			{
				//blanking region 2
				//blanking region 3
				pPockelWaveform[j * 2* samples_per_line + sample_index++] = pockels_output_low_ushort;
			}

			for (; sample_index < endOfBackLineScan;)
			{
				//only enable the backscan if two way mode is enabled
				if(_imgAcqPty_Pre.scanMode == TWO_WAY_SCAN_MODE)
				{
					//To account for the sinusoidal movement of the resonant scanner
					//the x offset on the mask is calculated using a cosine and shifting it by 90deg
					//and adding 1 to start at 0 and increase as j increases "cos(PI + PI* (j -  (_subtriggerLength/2))/(_subtriggerLength/2) + C1) + 1)/2.0 "
					//the phase shift "PHASE_SHIFT" is needed to accomodate for the delay in the pockels modulator
					//Here, because its the backward scan, we want to start at the end of the x for the line and then move to the left until
					//we reach the begining of the line. Then we go back to the forward scan
					long xOffset = 0;
					if (FALSE == _imgAcqPty_Pre.horizontalFlip)
					{
						xOffset =  static_cast<long>(ceil(W1 * (cos(M_PI + M_PI* (sample_index -  samples_per_line)/samples_per_line + PHASE_SHIFT) + 1)/2.0 - 0.5));
					}
					else
					{
						//if there is a horizontal flip, then start at the end of the line
						xOffset =  static_cast<long>(ceil(W1 * (cos(M_PI + M_PI* ((samples_per_line) - (sample_index -  (samples_per_line)))/(samples_per_line) - PHASE_SHIFT) + 1)/2.0 - 0.5));
					}

					long maskOffset = j * (width*2 ) + (width*2 - xOffset);

					unsigned short * pMask = (unsigned short*)_pPockelsMask;
					pMask += maskOffset;

					if(TRUE==usePockelsMask)
					{
						//if mask value is greater than 0 and invert is off, or mask value is 0 (or less) and invert is on
						//then set the pockels to the on voltage
						if((*pMask > 0 && !pockelPty->pockelsMaskInvert[i]) || *pMask <= 0 && pockelPty->pockelsMaskInvert[i])
						{
							StringCbPrintfW(message,MSG_SIZE, L"ThorLSMCam Mask offset %d mask value %d",maskOffset,*pMask);
							LogMessage(message,VERBOSE_EVENT);
							pPockelWaveform[j * 2* samples_per_line + sample_index++] = pockels_output_high_ushort;
						}
						else
						{
							pPockelWaveform[j * 2* samples_per_line + sample_index++] = pockels_output_low_ushort;
						}
					}
					else
					{
						pPockelWaveform[j * 2* samples_per_line + sample_index++] = pockels_output_high_ushort;
					}
				}
				else
				{
					pPockelWaveform[j * 2* samples_per_line + sample_index++] = pockels_output_low_ushort;
				}
			}

			for (; sample_index < samples_per_line * 2;)
			{
				//blanking region 4
				pPockelWaveform[j * 2* samples_per_line + sample_index++] = pockels_output_low_ushort;
			}
		}

		//80 moving average filter
		for (ULONG32 j = 0; j < backward_data_num; j++)
		{
			//*(pGalvoWaveformX+(forwardLines*samples_two_lines+ i)) = max(0,min( USHRT_MAX - Waveform_Start[0], static_cast<USHORT>(amp_offset_X - (double)pad_amp * cos(1.5 * M_PI + pre_flayback_step*double(i)) - half_P2P_amp_X) - Waveform_Start[0]));
			*(pPockelWaveform +(ULONG32)(samples_per_line * 2 * scanInfo->forward_lines + j)) = pockels_output_low_ushort;
		}

		for (ULONG32 j = 0; j < (ULONG32)DAC_FIFO_DEPTH; j++)
		{
			//*(pGalvoWaveformX+(forwardLines*samples_two_lines+ i)) = max(0,min( USHRT_MAX - Waveform_Start[0], static_cast<USHORT>(amp_offset_X - (double)pad_amp * cos(1.5 * M_PI + pre_flayback_step*double(i)) - half_P2P_amp_X) - Waveform_Start[0]));
			*(pPockelWaveform +(ULONG32)(samples_per_line * 2 * scanInfo->forward_lines + backward_data_num + j)) = pockels_output_low_ushort;
		}

		IMAGING_BUFFER_STRUCT DACMemorySetting;
		DACMemorySetting.buffer = (UCHAR*)(pPockelWaveform + FIFO_DELAY_SAMPLES);
		DACMemorySetting.channel = Pockel1 + i;
		DACMemorySetting.length = (total_samples - FIFO_DELAY_SAMPLES) * 2;
		DACMemorySetting.offset = 0;
		LoadDACMemorySettings(DACMemorySetting);
		
		gPtrMemoryPool->GetMemoryPropertyByChannel(DACMemorySetting.channel,_daqAcqCfg.dacCtrl[DACMemorySetting.channel].waveform_buffer_start_address,_daqAcqCfg.dacCtrl[DACMemorySetting.channel].waveform_buffer_length);
		_daqAcqCfg.dacCtrl[DACMemorySetting.channel].park_val = (1 == _pockelsParkAtMinimum) ? pockelPty->pockelsMinVoltage[i] : pockelsOnVoltage;
		_daqAcqCfg.dacCtrl[DACMemorySetting.channel].offset_val = pockelPty->pockelsMinVoltage[i];
		_daqAcqCfg.dacCtrl[DACMemorySetting.channel].update_rate = dac_rate;
		_daqAcqCfg.dacCtrl[DACMemorySetting.channel].flyback_samples = backward_data_num;
		_daqAcqCfg.dacCtrl[DACMemorySetting.channel].output_port = DACMemorySetting.channel;

	/*	string waveformFile = "waveform" + to_string(i + 2)+".txt";
		 ofstream myfile (waveformFile);
		if (myfile.is_open())
		{
			for (int i = 0; i < total_samples; i++)
			{
				myfile << std::fixed << std::setprecision(8) << (*(pPockelWaveform+i));
				myfile << "\n";
			}
		myfile.close();
		}
*/
		SAFE_DELETE_ARRAY(pPockelWaveform);
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
long CThordaqResonantGalvo::LoadDACMemorySettings(
		IMAGING_BUFFER_STRUCT& DACMemorySettings
		)
{
	THORDAQ_STATUS status = STATUS_WRITE_BUFFER_ERROR;
	ULONG64 start_address = 0;
	if (gPtrMemoryPool->RequestMemoryAllocation(DACMemorySettings.length,start_address))// Get the start writting position
	{
		if (ThordaqPacketWriteBuffer(_DAQDeviceIndex, start_address,static_cast<ULONG>(DACMemorySettings.length),DACMemorySettings.buffer,0xffffffff) == STATUS_SUCCESSFUL) // Write configuration to the RAM
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


long CThordaqResonantGalvo::UpdateDACMemorySettings(
		IMAGING_BUFFER_STRUCT& DACMemorySettings
		)
{
	THORDAQ_STATUS status = STATUS_WRITE_BUFFER_ERROR;
	ULONG64 start_address = 0;
	ULONG64 length = 0;
	if (gPtrMemoryPool->GetMemoryPropertyByChannel(DACMemorySettings.channel,start_address,length))
	{
		if ((DACMemorySettings.length <= length) && (ThordaqPacketWriteBuffer(_DAQDeviceIndex, start_address + DACMemorySettings.offset,static_cast<ULONG>(DACMemorySettings.length),DACMemorySettings.buffer,0xffffffff) == STATUS_SUCCESSFUL)) // Write configuration to the RAM
		{
			status = STATUS_SUCCESSFUL;
		}
	}
	return status;
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

		for(long i=0; i<POCKELS_VOLTAGE_STEPS; i++)
		{
			ThordaqSetDACParkValue(_DAQDeviceIndex,Pockel1 + index,pockelsPos);

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
		ThordaqSetDACParkValue(_DAQDeviceIndex,Pockel1 + index,pockelsPos);

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

	FILE *AlignDataFile;

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
			if(i >= 5)
			{
				_imgAcqPty.twoWayZones[255-i] = lVal;
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
		const long MIN_FINE_ALIGNMENT = -128;
		const long MAX_FINE_ALIGNMENT = 128;
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
			if(i >= 5)
			{
				_imgAcqPty.twoWayZonesFine[255-i] = lVal;
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

long CThordaqResonantGalvo::GetDACSamplesPerLine(ScanLineStruct* scanLine,double& dac_rate, double line_time)
{
	double update_rate = DAC_MAX_UPDATERATE;
	double min_samples = 59;
	double max_samples = 62;
	double min_offset = max_samples;
	double max_update_rate = ceil((double)SYS_CLOCK_FREQ / ( min_samples / line_time) - 1); 
	//To get the proper sample rate
	for (int updaterate = 199; updaterate < max_update_rate; updaterate++)
	{
		for (double sample = max_samples; sample > min_samples; sample--)
		{
			double offset = (double)SYS_CLOCK_FREQ / (double)(updaterate + 1) * line_time - sample;
			if( abs(offset) < abs(min_offset))
			{
				min_offset = offset;
				dac_rate = (double)SYS_CLOCK_FREQ / (double)(updaterate + 1);
				scanLine->samples_scan = static_cast<long>(sample - 1);
			}
		}
	}
	scanLine->samples_idle = 0;
	return TRUE;
}