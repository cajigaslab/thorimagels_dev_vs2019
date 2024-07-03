// ThorDAQGGDFLIMSim.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ThorDAQGGDFLIMSim.h"
#include "ThorDAQGGDFLIMSimSetupXML.h"
#include <queue>

#ifdef LOGGING_ENABLED
#include "..\..\..\..\..\Common\Log.h"
std::auto_ptr<LogDll> qlogDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

wchar_t message[MSG_SIZE];
#define DAQmxErrChk(fnName,fnCall) if ((error=(fnCall)) < 0){ StringCbPrintfW(message,MSG_SIZE,L"%s GR failed (%d),(%d). ",fnName,error,__LINE__); CThorDAQGGDFLIMSim::GetInstance()->LogMessage(message,ERROR_EVENT); throw "fnCall";}else{ StringCbPrintfW(message,MSG_SIZE,L"%s GR failed (%d) (%d). ",fnName,error,__LINE__); CThorDAQGGDFLIMSim::GetInstance()->LogMessage(message,VERBOSE_EVENT);}
#define MAX_TASK_WAIT_TIME 10.0

///******	Initialize Static Members		******///
bool CThorDAQGGDFLIMSim::_instanceFlag = false;
ImgAcqPty CThorDAQGGDFLIMSim::_imgAcqPty = ImgAcqPty();
ImgAcqPty CThorDAQGGDFLIMSim::_imgAcqPty_Pre = ImgAcqPty();
long CThorDAQGGDFLIMSim::_bufferHSize = 0;
auto_ptr<CThorDAQGGDFLIMSim> CThorDAQGGDFLIMSim::_single(new CThorDAQGGDFLIMSim());//Instantiated on first use
HANDLE CThorDAQGGDFLIMSim::_hStopAcquisition = CreateEvent(NULL, TRUE, FALSE, NULL);	//make sure the reset option is true (MANUAL RESET)
HANDLE CThorDAQGGDFLIMSim::_hThreadStopped   = CreateEvent(NULL, TRUE, TRUE, NULL);
HANDLE CThorDAQGGDFLIMSim::_hTriggerTimeout  = CreateEvent(NULL, FALSE, FALSE, NULL);
HANDLE CThorDAQGGDFLIMSim::_hHardwareTriggerInEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
HANDLE CThorDAQGGDFLIMSim::_hAcquisitionThread = NULL; // Main Acquisition thread

double CThorDAQGGDFLIMSim::_frameRate = 1;

long long CThorDAQGGDFLIMSim::_index_of_last_written_frame = 0;
long long CThorDAQGGDFLIMSim::_index_of_last_read_frame = 0;

long CThorDAQGGDFLIMSim::_DAQDeviceIndex = DEFAULT_CARD_NUMBER;
FrameCirBuffer* CThorDAQGGDFLIMSim::_pFrmBuffer = nullptr;
vector<FlimBuffer*> CThorDAQGGDFLIMSim::_pBuffer;
vector<UCHAR*> CThorDAQGGDFLIMSim::_pRawDataBuffer;
FlimBuffer* CThorDAQGGDFLIMSim::_pHistoryBuf = nullptr; 
DataProcessing* CThorDAQGGDFLIMSim::_pDataProcess = nullptr;

HANDLE CThorDAQGGDFLIMSim::_hFrmBufHandle = NULL;

long CThorDAQGGDFLIMSim::_triggerWaitTimeout = 0;
char* CThorDAQGGDFLIMSim::_simulatedFrame = NULL;

/**********************************************************************************************//**
 * @fn	CThorDAQGGDFLIMSim::CThorDAQGGDFLIMSim()
 *
 * @brief	Default constructor.
 *
 **************************************************************************************************/
CThorDAQGGDFLIMSim::CThorDAQGGDFLIMSim()
{
	_deviceNum = 0;
	_pDetectorName = L"ThorDAQGGDFLIMSim";
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

	SAFE_DELETE_PTR(_pDataProcess);
	SAFE_DELETE_PTR(gPtrMemoryPool);

	for(long i=0; i<4; i++)
	{
		_channelPolarity[i] = 0;
	}
	_pockelsParkAtMinimum = 0;
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
#define DAC_DDR3_BUF_OFFSET  0x70000000
	gPtrMemoryPool = new MemoryPool(0x8000000, DAC_DDR3_BUF_OFFSET); // Memory pool: 0 - 0x40000000 acquisition buffer. 0x40000000 - 0x80000000 bitmap buffer.
	_pDataProcess = new DataProcessing();

	ifstream infile;

	infile.open("fullFrameSim.bin", ios::binary | ios::in);
	_frameHSize = 8192 * 8;

	//infile.open("DFLIM_RawData.bin", ios::binary | ios::in);	
	//_frameHSize = 2528 * 8;

	_pTempCopyBuf = NULL;

	_frameImageWidth = 256;
	_frameImageHeight = 256;

	_simulatedFrame = new char[_frameHSize * _frameImageHeight];

	infile.read(_simulatedFrame, _frameHSize * _frameImageHeight);
}
/**********************************************************************************************//**
 * @fn	CThorDAQGGDFLIMSim::~CThorDAQGGDFLIMSim()
 *
 * @brief	Destructor.
 *
 **************************************************************************************************/
CThorDAQGGDFLIMSim::~CThorDAQGGDFLIMSim()
{
	//Reset the device parameter
	SAFE_DELETE_PTR(gPtrMemoryPool);
	_deviceNum = 0;
	_single.release();
	_instanceFlag = false;
}


/**********************************************************************************************//**
 * @fn	CThorDAQGGDFLIMSim* CThorDAQGGDFLIMSim::GetInstance()
 *
 * @brief	Gets the instance. Singlton design pattern
 *
 * @return	null if it fails, else the instance.
 **************************************************************************************************/
CThorDAQGGDFLIMSim* CThorDAQGGDFLIMSim::GetInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new CThorDAQGGDFLIMSim());
		_instanceFlag = true;
		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

/**********************************************************************************************//**
 * @fn	long CThorDAQGGDFLIMSim::FindCameras(long &cameraCount)
 *
 * @brief	Searches for the devices.
 *
 * @param [in,out]	cameraCount	Index of Camera.
 *
 * @return	False if no Camera found. True if Camera found.
 **************************************************************************************************/
long CThorDAQGGDFLIMSim::FindCameras(long &cameraCount)
{
	StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorLSMCam::FindCameras");
	LogMessage(_errMsg,VERBOSE_EVENT);
	//MessageBox(NULL, L"test", L"test", MB_DEFBUTTON1);
	// First make sure there is a board installed
	try
	{
		cameraCount = 1;
		_deviceNum = 1;
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
 * @fn	long CThorDAQGGDFLIMSim::SelectCamera(const long camera)
 *
 * @brief	Select Camera.
 *
 * @param	camera	The camera index .
 *
 * @return	A long.
 **************************************************************************************************/
long CThorDAQGGDFLIMSim::SelectCamera(const long camera)
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
		auto_ptr<ThorGalvoGalvoXML> pSetup(new ThorGalvoGalvoXML());
		//XML settings retrieval functions will throw an exception if tags or attributes are missing
		//catch each independetly so that as many tags as possible can be read
		try
		{
			if(FALSE == pSetup->GetConfiguration(_field2Theta,_pockelsParkAtMinimum))
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"GetConfiguration from ThorDAQGGDFLIMSimSettings.XML failed");
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

				StringCbPrintfW(_errMsg,MSG_SIZE,L"GetCalibration from ThorDAQGGDFLIMSimSettings.XML failed. FieldSizeCalibration not available.");
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"GetIO from ThorConfocalSettings failed");
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
				StringCbPrintfW(_errMsg,MSG_SIZE,L"GetTrigger from ThorConfocalSettings failed");
				LogMessage(_errMsg,ERROR_EVENT);
			}
			//Get Stream Control always using internal clock rate
			if(FALSE == pSetup->GetStreamConfiguration(_imgAcqPty.clockRateInternal,_imgAcqPty.FIRFilter[0],_imgAcqPty.FIRFilter[1],_imgAcqPty.DCOffset[0],_imgAcqPty.DCOffset[1]))
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"GetStream from ThorDAQGGDFLIMSimSettings.XML failed");
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

	return TRUE;
}

/**********************************************************************************************//**
 * @fn	long CThorDAQGGDFLIMSim::TeardownCamera()
 *
 * @brief	Teardown Camera.
 *
 * @return	A long.
 **************************************************************************************************/
long CThorDAQGGDFLIMSim::TeardownCamera()
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

	//close the shutter

	//disconnect DAQ board
	_deviceNum = 0;
    //// Clean all the threads
	SAFE_DELETE_HANDLE(_hAcquisitionThread);
	
	////// Close the events to clean up
	//CloseHandle(_hThreadStopped);
	//CloseHandle(_hStopAcquisition);
	//CloseHandle(_hTriggerTimeout);
	//CloseHandle(_hHardwareTriggerInEvent);
	//// Free all the buffers
	SAFE_DELETE_PTR(_pFrmBuffer);

	if(_pBuffer.size() > 0)
	{
		for(int i=0;i < _pBuffer.size();i++)
		{
			_pBuffer.at(i)->deleteBuffer();
			//VirtualFree(_pBuffer.at(i), 0, MEM_RELEASE);
			SAFE_DELETE_PTR(_pBuffer.at(i));
			_pBuffer.at(i) = NULL;
		}
		_pBuffer.clear();				
	}

	if(_pHistoryBuf != NULL)
	{
		for(int i=0;i < MAX_CHANNEL_COUNT; ++i)
		{
			(_pHistoryBuf + i)->deleteBuffer();
		}
		SAFE_DELETE_ARRAY(_pHistoryBuf);
		//VirtualFree(_pHistoryBuf,0,MEM_RELEASE);
		_pHistoryBuf = NULL;
	}

	//// Delete all the pointers
	SAFE_DELETE_ARRAY(_pTempCopyBuf);

	return TRUE;
}


/**********************************************************************************************//**
 * @fn	long CThorDAQGGDFLIMSim::PreflightAcquisition(char * pDataBuffer)
 *
 * @brief	Preflight position.
 * @param [in,out]	pDataBuffer	  	Identifier of the Data buffer.
 * @return	A long.
 **************************************************************************************************/
long CThorDAQGGDFLIMSim::PreflightAcquisition(char * pDataBuffer)
{
	long ret = TRUE;
	// reset dropped frame count:
	_droppedFramesCnt = 0;
	_index_of_last_written_frame = -1;
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
 * @fn	long CThorDAQGGDFLIMSim::SetupAcquisition(char * pDataBuffer)
 *
 * @brief	Setup  Acquisition.
 * @param [in,out]	pDataBuffer	  	Identifier of the Data buffer.
 * @return	A long.
 **************************************************************************************************/
long CThorDAQGGDFLIMSim::SetupAcquisition(char * pDataBuffer)
{
	long ret = TRUE;

	//do not capture data if you are in the centering scan mode, Just Set the Scanner
	if(_imgAcqPty.scanMode == ScanMode::CENTER_SCAN_MODE)
	{
		return ret;
	}

	//If settings change or system is forced to refresh updating, do update.
	if (!_imgAcqPty.IsEqual(_imgAcqPty_Pre) || _forceSettingsUpdate == TRUE)
	{
		_index_of_last_written_frame = -1;
		// Disable Force updating
		_forceSettingsUpdate = FALSE;
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
		/**********reset the acquisition thread and the event flag*******/
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

		if (ConfigAcqSettings(&_imgAcqPty_Pre) == FALSE)
		{
			ret = FALSE;
		}
	}
	
	return ret;
}

/**********************************************************************************************//**
 * @fn	long CThorDAQGGDFLIMSim::ConfigAcqSettings(ImgAcqPty* pImgAcqPty)
 *
 * @brief	Configure thordaq settings
 * @param [in,out]	pImgAcqPty	  	Identifier of Image Acquisition Struct.
 * @return	A long.
 **************************************************************************************************/
long CThorDAQGGDFLIMSim::ConfigAcqSettings(ImgAcqPty* pImgAcqPty)
{
	long ret = TRUE;
	//Initiate the Struct
	gPtrMemoryPool->ClearUpMemory();

	double dwell_time = pImgAcqPty->dwellTime / 1000000.0;


	double const multFactor1 = 1.125;
	double const multFactor2 = 4;
	double const constFactor = 66;
	double const allowanceFactor = 1.05;
	long bytesPerBeat = 8;
	
	//Buffer HSize = (66 + pixels*1.125 + usec*4)* (1+5%)
	_bufferHSize = _frameHSize;//static_cast<long>(ceil((constFactor + pImgAcqPty->pixelX * multFactor1 + pImgAcqPty->dwellTime * pImgAcqPty->pixelX * multFactor2) * allowanceFactor / 8.0)) * bytesPerBeat;

	// set up pixel convertor
	_pDataProcess->SetupDataMap(pImgAcqPty->dataMapMode, _channelPolarity);


	double linetime = 0.001;

	_frameRate = 1.0 / (pImgAcqPty->pixelY * linetime);
	
	//BuildPockelsControlWaveform(&scan_info, &scanLine, dac_rate, &_imgAcqPty.pockelPty);

	std::bitset<sizeof(size_t)*CHAR_BIT> channel_bitset(pImgAcqPty->channel);

	if (SetupFrameBuffer(static_cast<long>(channel_bitset.count()), pImgAcqPty) != TRUE)
	{
		ret = FALSE;
		return ret;
	}

	photonNumBufferCopied = true;
	singlePhotonBufferCopied = true;
	arrivalTimeSumBufferCopied = true;
	histogramBufferCopied = true;
	photonListCopied = true;

	return ret;
}

/**********************************************************************************************//**
 * @fn	long CThorDAQGGDFLIMSim::SetupFrameBuffer()
 *
 * @brief	Set up Frame Buffer.
 * @return	A long.
 **************************************************************************************************/
long CThorDAQGGDFLIMSim::SetupFrameBuffer(int channel_count, ImgAcqPty* pImgAcqPty)
{
	//BUFFER #1: 1 second frame buffer (frame rate frames for all channels):
	if(_pBuffer.size() > 0)
	{
		for(int i=0;i<_pBuffer.size();i++)
		{
			//VirtualFree(_pBuffer.at(i), 0, MEM_RELEASE);
			_pBuffer.at(i)->deleteBuffer();
			SAFE_DELETE_PTR(_pBuffer.at(i));
			_pBuffer.at(i) = NULL;
		}
		_pBuffer.clear();				
	}

	//BUFFER #1: 1 second frame buffer (frame rate frames for all channels):
	if(_pRawDataBuffer.size() > 0)
	{
		for(int i=0;i<_pRawDataBuffer.size();i++)
		{
			VirtualFree(_pRawDataBuffer.at(i), 0, MEM_RELEASE);
			_pRawDataBuffer.at(i) = NULL;
		}
		_pRawDataBuffer.clear();				
	}

	size_t AllocSize = (size_t)(_bufferHSize*pImgAcqPty->pixelY*_frameRate);//_daqAcqCfg.imageCtrl.imgHSize*_daqAcqCfg.imageCtrl.imgVSize*_daqAcqCfg.imageCtrl.frameNumPerSec*2; // Buffer size
	AllocSize = SIZE_T((AllocSize + 1023) & -1024);
	if ((AllocSize == 0) || (AllocSize < 0))
	{
		printf("Invalid Buffer Allocation Size = %zd\n", AllocSize);
		return FALSE;
	}
	for(int i = 0; i < static_cast<int>(MAX_CHANNEL_COUNT); i++)
	{
		FlimBuffer* ptr =  new FlimBuffer();
		ptr->setupBuffer(pImgAcqPty->pixelX, pImgAcqPty->pixelY);
		_pBuffer.push_back(ptr);
		if (_pBuffer.at(i)== NULL)
		{
			printf("Buffer malloc failed at channel %d, Size = %zd\n", i, AllocSize);
			return FALSE;
		}
	}

	for(int i = 0; i < static_cast<int>(MAX_CHANNEL_COUNT); i++)
	{
		UCHAR* ptr = (UCHAR *) VirtualAlloc (NULL, AllocSize, MEM_COMMIT, PAGE_READWRITE);
		_pRawDataBuffer.push_back(ptr);
		if (_pRawDataBuffer.at(i)== NULL)
		{
			printf("Buffer malloc failed at channel %d, Size = %zd\n", i, AllocSize);
			return FALSE;
		}
	}

	//BUFFER #2: history buffer for average (1 frame for all channels):
	if(_pHistoryBuf!= NULL)
	{
		SAFE_DELETE_ARRAY(_pHistoryBuf);
	}

	_pHistoryBuf =  new FlimBuffer[MAX_CHANNEL_COUNT];

	for(int i = 0; i < static_cast<int>(MAX_CHANNEL_COUNT); i++)
	{
		(_pHistoryBuf + i)->setupBuffer(pImgAcqPty->pixelX, pImgAcqPty->pixelY);
		if ((_pHistoryBuf + i)== NULL)
		{
			printf("Buffer allocation FlimBuffer failed at channel %d, Size = %zd\n", i, AllocSize);
			return FALSE;
		}
	}

	if(_pHistoryBuf == NULL)
	{	
		return FALSE;	
	}

	//BUFFER #3: temporary buffer to extract the data to, from the circular buffer:
	SAFE_DELETE_ARRAY(_pTempCopyBuf);
	_pTempCopyBuf =  new FlimBuffer[MAX_CHANNEL_COUNT];

	for(int i = 0; i < static_cast<int>(MAX_CHANNEL_COUNT); i++)
	{
		(_pTempCopyBuf + i)->setupBuffer(pImgAcqPty->pixelX, pImgAcqPty->pixelY);
		if ((_pTempCopyBuf + i)== NULL)
		{
			printf("Buffer allocation FlimBuffer failed at channel %d, Size = %zd\n", i, AllocSize);
			return FALSE;
		}
	}

	//BUFFER #4: circular buffer for read (by user) and write (by camera):
	//int channelCount = CountChannelBits(_imgAcqPty.channel); // Do later
	SAFE_DELETE_PTR(_pFrmBuffer);
	_pFrmBuffer = new FrameCirBuffer(pImgAcqPty->pixelX,pImgAcqPty->pixelY, MAX_CHANNEL_COUNT, (size_t)ceil((_frameRate * MAX_CHANNEL_COUNT) - 0.5));//new FrameCirBuffer(_daqAcqCfg.imageCtrl.imgHSize,_daqAcqCfg.imageCtrl.imgVSize, MAX_CHANNEL_COUNT ,2,_daqAcqCfg.imageCtrl.frameNumPerSec * MAX_CHANNEL_COUNT);
	
	return TRUE;
}


/**********************************************************************************************//**
 * @fn	long CThorDAQGGDFLIMSim::StartAcquisition(char * pDataBuffer)
 *
 * @brief	Start  Acquisition.
 * @param [in,out]	pDataBuffer	  	Identifier of the Data buffer.
 * @return	A long.
 **************************************************************************************************/
long CThorDAQGGDFLIMSim::StartAcquisition(char * pDataBuffer)
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
		case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
			{
				if (WaitForSingleObject(_hThreadStopped, 0) == WAIT_OBJECT_0)
				{
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
 * @fn	HANDLE CThorDAQGGDFLIMSim::CaptureCreateThread(DWORD &threadID)
 *
 * @brief	Create  Acquisition Thread.
 * @param 	threadID	  	Acquisition Thread ID.
 * @return	Thread Handle.
 **************************************************************************************************/
HANDLE CThorDAQGGDFLIMSim::CaptureCreateThread(DWORD &threadID)
{
	//DigiParams *dParamsOut = new DigiParams();
	//*dParamsOut = _digiParams;
	_index_of_last_written_frame = -1;
	_index_of_last_read_frame = -1;
	ResetEvent(_hThreadStopped);
	HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &(CThorDAQGGDFLIMSim::StartFrameAcqProc), (void *) this, 0, &threadID);
	SetThreadPriority(handle, THREAD_PRIORITY_HIGHEST);
	return handle;
}

/**********************************************************************************************//**
 * @fn	UINT CThorDAQGGDFLIMSim::StartFrameAcqProc(LPVOID instance)
 *
 * @brief	Start  Acquisition Thread.
 * @param 	instance	  	Acquisition Thread instance.
 * @return	A uint.
 **************************************************************************************************/
UINT CThorDAQGGDFLIMSim::StartFrameAcqProc(LPVOID instance)
{
	int				frameWritten = 0;
	long			status = TRUE;
	bool            acqStatus = true;
	long long		targetFrames = _imgAcqPty.numFrame;// do later
	int				frameSize = _imgAcqPty.pixelX *_imgAcqPty.pixelY;
	int				imageSizeBytes = frameSize * 2;
	int				frameSizeByte = _bufferHSize * _imgAcqPty.pixelY;//_daqAcqCfg.imageCtrl.imgHSize * _daqAcqCfg.imageCtrl.imgVSize;
	ULONG			transferSize = static_cast<ULONG>(frameSizeByte);
	ULONG           size;
	long			chFrameNum[4] = {0, 0, 0, 0}; 
	int				targetFrmNum = 0;
	double          regular_timeout = 1.0 / _frameRate * 1000 * 10 * _frameRate * 10;
	double			hardware_trigger_timeout = _triggerWaitTimeout * 1000;
	double			timeout = regular_timeout;
	BOOL			is_hardware_captured = FALSE;
	BOOL			hardware_timeout_enable = FALSE;
	ULONG32         bank = 0;
	long long       frame_left = targetFrames - 1; // 0 based
	unsigned int frameIndex = 0;

	//std::queue<ULONG32*> histogramQueue;
	//std::queue<ULONG32*> photonArrivalTimeQueue;

	//ULONG32* avgHistogramBuffer = NULL;
	//ULONG32* avgArrivalTimeSumBuffer = NULL;
	//USHORT* avgSinglePhotonBuffer = NULL;

	//if(1 < _imgAcqPty.averageNum && _imgAcqPty.averageMode == FRM_CUMULATIVE_MOVING)
	//{
	//	const long HISTOGRAM_BINS = 256;
	//	avgHistogramBuffer = new ULONG32[HISTOGRAM_BINS];
	//	avgArrivalTimeSumBuffer = new ULONG32[frameSize];
	//	avgSinglePhotonBuffer = new USHORT[frameSize];
	//}

	int avgIndex = 0;

	//FlimBuffer* flimBuffer = new FlimBuffer();
	//flimBuffer->setupBuffer(_imgAcqPty.pixelX, _imgAcqPty.pixelY);
	DataStream* dataStream = new DataStream();// = new DataStream();
	do
	{	
		frame_left = targetFrames - 1 - _index_of_last_written_frame;
		//if (frame_left >= _frameRate)
		{	
			for (int i = 0; i < MAX_CHANNEL_COUNT; ++i)
			{
				if ((_imgAcqPty.channel & (0x0001 << i))) // channel is enabled
				{
					timeout = (hardware_timeout_enable && !is_hardware_captured) ? hardware_trigger_timeout : regular_timeout;
					size = transferSize;

					//Execute another bufRead for 1 second frames:
					memcpy(_pRawDataBuffer.at(i), _simulatedFrame, size);
					
					//Sleep(1);
					chFrameNum[i] = static_cast<long>(size / frameSizeByte);
					acqStatus = TRUE;
					//_pBuffer.at(i)
					//status = ThordaqReadChannel(_DAQDeviceIndex, i, &size, _pBuffer.at(i), timeout);
				}
			}
		}

		ResetEvent(_hTriggerTimeout);
		// Done copy channels data  Write to the buffer
		targetFrmNum = 0;
		
		for (int i = 0; i < MAX_CHANNEL_COUNT; i++)
		{
			if ((_imgAcqPty.channel & (0x0001 << i)) != 0x0000)
			{
				//
				//flimBuffer->arrival_time_vector.clear();
				_pBuffer.at(i)->arrival_time_vector.clear();				
				dataStream->deSerialize(_pRawDataBuffer.at(i), frameSizeByte, _imgAcqPty.pixelY, _bufferHSize, _pBuffer.at(i));
				//memcpy(_pBuffer.at(i), (flimBuffer->photon_num_buffer), imageSizeBytes);

				//USHORT* buffer = (USHORT*)_pBuffer.at(i);

				//for (int j = 0; j < frameSize; ++j)
				//{
				//	if (flimBuffer->single_photon_buffer[j] != 0)
				//	{
				//		buffer[j] = static_cast<USHORT>(ceil((flimBuffer->arrival_time_sum_buffer[j]/  flimBuffer->single_photon_buffer[j]) - 0.5));
				//	}
				//	else
				//	{
				//		buffer[j] = 0;
				//	}
				//}

				//_pDataProcess->ProcessBuffer((USHORT*)(_pBuffer.at(i)),i,transferSize / 2);
				targetFrmNum  = max(targetFrmNum, (int)chFrameNum[i]);
			}

			//combine all average frames into 1:
			if((WaitForSingleObject(_hStopAcquisition, 0) == WAIT_OBJECT_0))
			{
				break;
			}
		}
		//_index_of_last_written_frame += targetFrmNum; //do later
		//size = targetFrmNum * frameSizeByte;
		
		//if (0)
		//{
		//	ofstream my_file1("DFLIM_RawData.bin", ios_base::binary);
		//	my_file1.write((char*)(_pRawDataBuffer.at(0)), frameSizeByte);
		//	my_file1.close();

		//	ofstream my_file2("DFLIM_arrival_time_sum_buffer.bin", ios_base::binary);
		//	my_file2.write((char*)(_pBuffer.at(0)->arrival_time_sum_buffer), _pBuffer.at(0)->arrivalTimeSumBufferSizeBytes);
		//	my_file2.close();

		//	ofstream my_file3("DFLIM_histogram_raw_buffer.bin", ios_base::binary);
		//	my_file3.write((char*)(_pBuffer.at(0)->histogram_raw_buffer), _pBuffer.at(0)->histogramRawBufferSizeBytes);
		//	my_file3.close();

		//	ofstream my_file4("DFLIM_histogram_buffer.bin", ios_base::binary);
		//	my_file4.write((char*)(_pBuffer.at(0)->histogram_buffer), _pBuffer.at(0)->histogramBufferSizeBytes);
		//	my_file4.close();

		//	ofstream my_file5("DFLIM_single_photon_buffer.bin", ios_base::binary);
		//	my_file5.write((char*)(_pBuffer.at(0)->single_photon_buffer), _pBuffer.at(0)->singlePhotonBufferSizeBytes);
		//	my_file5.close();

		//	ofstream my_file6("DFLIM_photon_num_buffer.bin", ios_base::binary);
		//	my_file6.write((char*)(_pBuffer.at(0)->photon_num_buffer), _pBuffer.at(0)->photonNumBufferSizeBytes);
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

							avgIntensityPtr = (_pHistoryBuf +chID)->photon_num_buffer;
							newIntensityPtr = _pBuffer.at(chID)->photon_num_buffer;
							sumSinglePhotonPtr = (_pHistoryBuf +chID)->single_photon_buffer;
							newSinglePhotonPtr = _pBuffer.at(chID)->single_photon_buffer;
							sumArrivalTimeSumPtr = (_pHistoryBuf +chID)->arrival_time_sum_buffer;
							newArrivalTimeSumPtr = _pBuffer.at(chID)->arrival_time_sum_buffer;

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
									*(sumSinglePhotonPtr + p) += static_cast<USHORT>(ceil(((*(newSinglePhotonPtr + p)) -  (*(sumSinglePhotonPtr + p))/_imgAcqPty.averageNum) - 0.5));
									*(sumArrivalTimeSumPtr + p) += static_cast<ULONG32>(ceil(((*(newArrivalTimeSumPtr + p)) -  (*(sumArrivalTimeSumPtr + p))/_imgAcqPty.averageNum) -0.5));
								}
								else
								{
									*(sumSinglePhotonPtr + p) += *(newSinglePhotonPtr + p);
									*(sumArrivalTimeSumPtr + p) += *(newArrivalTimeSumPtr + p);
								}
							}

							ULONG32* sumPtrH = (_pHistoryBuf +chID)->histogram_buffer; //Identifier pointer for history histogram buffer
							ULONG32* newPtrH = _pBuffer.at(chID)->histogram_buffer; //Identifier pointer for captured histogram buffer
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
					frameWritten = static_cast<int>((_pFrmBuffer->WriteFrames(_pHistoryBuf , MAX_CHANNEL_COUNT))/MAX_CHANNEL_COUNT);					
			}
			else
			{
				//no average, write buffer to circular buffer:
				int frmNum = 0;
				for(int j = 0; j < targetFrmNum; ++j)
				{		
					for(int chID = 0; chID < MAX_CHANNEL_COUNT; chID++)
					{
						//if ((_imgAcqPty.channel & (0x0001 << chID)) != 0x0000)
						{
							//frmNum += static_cast<int>(_pFrmBuffer->WriteFrames(((UCHAR *)_pBuffer.at(chID) + j * frameSizeByte), 1));	
							frmNum += static_cast<int>(_pFrmBuffer->WriteFrames(_pBuffer.at(chID), 1));
						}
					}
				}
				frameWritten = static_cast<int>(frmNum / MAX_CHANNEL_COUNT);
			}

			HandleFrameBuf(FALSE, TIMEOUT_MS);
			_index_of_last_written_frame += frameWritten;
		}
	} while ((_index_of_last_written_frame < (targetFrames - 1)) && (WaitForSingleObject(_hStopAcquisition, 0) != WAIT_OBJECT_0) && acqStatus);
	ULONG32 clockStatus = 0;
	//done capture:
	//delete pParams;
	((CThorDAQGGDFLIMSim *)instance)->StopDaqBrd();

	delete dataStream;
	SetEvent(_hThreadStopped);
	return 0;
}

void CThorDAQGGDFLIMSim::StopDaqBrd()
{
	
}

long CThorDAQGGDFLIMSim::StatusAcquisition(long &status)
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

	//int comparison_index = static_cast<int>(_index_of_last_written_frame);

	if(_index_of_last_read_frame < _index_of_last_written_frame)
	{
		status = ICamera::STATUS_READY;
		printf("_index_of_last_written_frame = %lld; _index_of_last_read_frame = %lld.\n", _index_of_last_written_frame, _index_of_last_read_frame);
//		StringCbPrintfW(message,MSG_SIZE,L"Status now is ready with frame buffer size of %d.\n", _index_of_last_written_frame - _index_of_last_read_frame);
//		LogMessage(message,VERBOSE_EVENT);
		//if (comparison_index  - _index_of_last_read_frame > _frameRate)
		//{
		//	status = ICamera::STATUS_ERROR;
		//}
	}	
	else
	{
		status = ICamera::STATUS_BUSY;
	}

	return ret;
}

long CThorDAQGGDFLIMSim::StatusAcquisitionEx(long &status,long &indexOfLastCompletedFrame)
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

long CThorDAQGGDFLIMSim::CopyAcquisition(char *pDataBuffer, void* frameInfo)
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
			if( _pFrmBuffer->ReadFrames(_pTempCopyBuf, channelCnt) > 0)
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
				size_t frameSizeByte = (_pTempCopyBuf + i)->photonNumBufferSizeBytes;
				
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{					
					memcpy(pDataBuffer + offset, (_pTempCopyBuf + i)->photon_num_buffer, (_pTempCopyBuf + i)->photonNumBufferSizeBytes);
				}
				offset += frameSizeByte;
				++channels;
			}
			else
			{
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{
					size_t frameSizeByte = (_pTempCopyBuf + i)->photonNumBufferSizeBytes;
					memcpy(pDataBuffer + offset, (_pTempCopyBuf + i)->photon_num_buffer, (_pTempCopyBuf + i)->photonNumBufferSizeBytes);
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
			if( _pFrmBuffer->ReadFrames(_pTempCopyBuf, channelCnt) > 0)
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
				size_t histoSizeByte = (_pTempCopyBuf + i)->histogramBufferSizeBytes;
				
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{					
					memcpy(pDataBuffer + offset, (_pTempCopyBuf + i)->histogram_buffer, (_pTempCopyBuf + i)->histogramBufferSizeBytes);									
				}
				offset += histoSizeByte;
				++channels;
			}
			else
			{
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{
					size_t histoSizeByte = (_pTempCopyBuf + i)->histogramBufferSizeBytes;
					memcpy(pDataBuffer + offset, (_pTempCopyBuf + i)->histogram_buffer, (_pTempCopyBuf + i)->histogramBufferSizeBytes);
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
				size_t frameSizeByte = (_pTempCopyBuf + i)->photonNumBufferSizeBytes;
				
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{					
					memcpy(pDataBuffer + offset, (_pTempCopyBuf + i)->photon_num_buffer, (_pTempCopyBuf + i)->photonNumBufferSizeBytes);
				}
				offset += frameSizeByte;
				++channels;
			}
			else
			{
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{
					size_t frameSizeByte = (_pTempCopyBuf + i)->photonNumBufferSizeBytes;
					memcpy(pDataBuffer + offset, (_pTempCopyBuf + i)->photon_num_buffer, (_pTempCopyBuf + i)->photonNumBufferSizeBytes);
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
				size_t histoSizeByte = (_pTempCopyBuf + i)->histogramBufferSizeBytes;
				
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{					
					memcpy(pDataBuffer + offset, (_pTempCopyBuf + i)->histogram_buffer, (_pTempCopyBuf + i)->histogramBufferSizeBytes);									
				}
				offset += histoSizeByte;
				++channels;
			}
			else
			{
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{
					size_t histoSizeByte = (_pTempCopyBuf + i)->histogramBufferSizeBytes;
					memcpy(pDataBuffer + offset, (_pTempCopyBuf + i)->histogram_buffer, (_pTempCopyBuf + i)->histogramBufferSizeBytes);
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
				size_t singlePhotonBufferSizeByte = (_pTempCopyBuf + i)->singlePhotonBufferSizeBytes;
			
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{					
					memcpy(pDataBuffer + offset, (_pTempCopyBuf + i)->single_photon_buffer, (_pTempCopyBuf + i)->singlePhotonBufferSizeBytes);								
				}
				offset += singlePhotonBufferSizeByte;
				++channels;
			}
			else
			{
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{
					size_t singlePhotonBufferSizeByte = (_pTempCopyBuf + i)->singlePhotonBufferSizeBytes;
					memcpy(pDataBuffer + offset, (_pTempCopyBuf + i)->single_photon_buffer, (_pTempCopyBuf + i)->singlePhotonBufferSizeBytes);
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
				size_t arrivalTimeNumSizeByte = (_pTempCopyBuf + i)->arrivalTimeSumBufferSizeBytes;
			
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{					
					memcpy(pDataBuffer + offset, (_pTempCopyBuf + i)->arrival_time_sum_buffer, (_pTempCopyBuf + i)->arrivalTimeSumBufferSizeBytes);							
				}
				offset += arrivalTimeNumSizeByte;
				++channels;
			}
			else
			{
				if ((_imgAcqPty_Pre.channel & (0x0001 << i)) != 0x0000)
				{
					size_t arrivalTimeNumSizeByte = (_pTempCopyBuf + i)->arrivalTimeSumBufferSizeBytes;
					memcpy(pDataBuffer + offset, (_pTempCopyBuf + i)->arrival_time_sum_buffer, (_pTempCopyBuf + i)->arrivalTimeSumBufferSizeBytes);
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
				size_t frameSizeByte = (_pTempCopyBuf + i)->arrival_time_vector.size()*sizeof(UCHAR);

				//std::vector<double> v;

				vector<UCHAR> vec = (_pTempCopyBuf + i)->arrival_time_vector;
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

	frameInfoStruct.numberOfPlanes = 1;

	frameInfoStruct.isNewMROIFrame = 1;
	frameInfoStruct.totalScanAreas = 1;
	frameInfoStruct.scanAreaIndex = 0;
	frameInfoStruct.scanAreaID = 0;
	frameInfoStruct.imageWidth = _imgAcqPty.pixelX;
	frameInfoStruct.imageHeight = _imgAcqPty.pixelY;
	frameInfoStruct.isMROI = FALSE;
	frameInfoStruct.fullImageWidth = (long)_imgAcqPty.pixelX;
	frameInfoStruct.fullImageHeight = (long)_imgAcqPty.pixelY;
	frameInfoStruct.topInFullImage = 0;
	frameInfoStruct.leftInFullImage = 0;
	frameInfoStruct.mROIStripeFieldSize = _imgAcqPty_Pre.fieldSize;

	if (channels > 0)
	{
		frameInfoStruct.copySize = offset;
	}
	memcpy(frameInfo, &frameInfoStruct, sizeof(FrameInfo));

	return ret;
}

std::string hexStr(ULONG64 *data, int len)
{
    std::stringstream ss;
    ss<<std::hex;
    for(int i(0);i<len;++i)
	{
        ss<<(ULONG64)data[i];
		ss<<",";
	}
    return ss.str();

	//string x = "";
 //   for(int i(0);i<len;++i)
 //       x += (ULONG64)data[i] + "x";
 //   return x;
}


long CThorDAQGGDFLIMSim::CopyAcquisitionInlcudingDisabledChannels(char * pDataBuffer)
{
	long ret = TRUE;
	return ret;
}
long CThorDAQGGDFLIMSim::CopyAcquisitionSkippingDisabledChannels(char * pDataBuffer)
{
	long ret = TRUE;
	return ret;
}
long CThorDAQGGDFLIMSim::PostflightAcquisition(char * pDataBuffer)
{
	try
	{
		//do not capture data if you are in the centering scan mode
	if(_imgAcqPty.scanMode == CENTER_SCAN_MODE)
		{
			//IDAQMovePockelsToPowerLevel();
			return TRUE;
		}
		//force the hardware trigger event if the post flight function is called
		SetEvent(_hHardwareTriggerInEvent);
		ResetEvent(_hHardwareTriggerInEvent);
		//IDAQCloseNITasks();	

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

	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}
long CThorDAQGGDFLIMSim::GetLastErrorMsg(wchar_t * msg, long size)
{
	long ret = TRUE;
	return ret;
}

void CThorDAQGGDFLIMSim:: LogMessage(wchar_t *message,long eventLevel)
{
#ifdef LOGGING_ENABLED
	qlogDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

long CThorDAQGGDFLIMSim::SetParamString(const long paramID, wchar_t * str)
{
	long ret = TRUE;
	return ret;
}
long CThorDAQGGDFLIMSim::GetParamString(const long paramID, wchar_t * str, long size)
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

long CThorDAQGGDFLIMSim::SetParamBuffer(const long paramID, char * pBuffer, long size)
{
	long ret = TRUE;
	return ret;
}

long CThorDAQGGDFLIMSim::GetParamBuffer(const long paramID, char * pBuffer, long size)
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

			//if(POCKELS_VOLTAGE_STEPS * sizeof(float64) <= size)
			//{
			//	std::memcpy(pBuffer,&_pockelsReadArray[index],POCKELS_VOLTAGE_STEPS * sizeof(float64));
			//}
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

void CThorDAQGGDFLIMSim::SetStatusHandle(HANDLE handle)
{

}

/// <summary> Cancels any hardware triggers the camera is currently waiting on </summary>
void CThorDAQGGDFLIMSim::StopHardwareWaits()
{
	SetEvent(_hStopAcquisition);
}

/// <summary> Calculates the minimum value for flyback cycles the current settings can support </summary>
/// <returns> Return 0, when vertical galvo is disabled </returns>
long CThorDAQGGDFLIMSim::GetMinFlybackCycle()
{
	if (ICamera::LSMAreaMode::POLYLINE == _imgAcqPty.areaMode || FALSE == _imgAcqPty.galvoEnable)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

/// <summary> Sets the current flyback cycle </summary>
/// <param name="flybackCycle"> The new value for flyback cycle </param>
void CThorDAQGGDFLIMSim::SetFlybackCycle(long flybackCycle)
{
	if (MAX_FLYBACK_CYCLE < flybackCycle || MAX_FLYBACK_TIME < GetFlybackTime(flybackCycle))
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"PARAM_LSM_FLYBACK_CYCLE %d outside range %d to %d, and above %f seconds",static_cast<long> (flybackCycle), GetMinFlybackCycle(), MAX_FLYBACK_CYCLE,MAX_FLYBACK_TIME);
		LogMessage(_errMsg,ERROR_EVENT);
	}
	else
	{
		long minFlybackCycle = GetMinFlybackCycle();
		_imgAcqPty.flybackCycle = (flybackCycle > minFlybackCycle ? flybackCycle : minFlybackCycle);
	}
}

double CThorDAQGGDFLIMSim::GetFlybackTime(long flybackCycles)
{
	if( flybackCycles == 0 && (FALSE == _imgAcqPty.galvoEnable || ICamera::LSMAreaMode::POLYLINE == _imgAcqPty.areaMode))
	{
		return LINE_FRAMETRIGGER_LOW_TIMEPOINTS * _imgAcqPty.dwellTime / 1000000.0;
	}
	else
	{
		double galvoSamplesPaddingTime = GALVO_RETRACE_TIME /1000000.0;  //used fixed amount of time for x galvo retrace
		double galvoSamplesPerLine = 2.0 * _imgAcqPty.pixelX * _imgAcqPty.dwellTime /1000000.0 + 2.0 *(_imgAcqPty.pixelX  - 1 ) * 17.0/200000000.0  + 4 * galvoSamplesPaddingTime;
		return galvoSamplesPerLine * flybackCycles;
	}
}

long CThorDAQGGDFLIMSim::GetFlybackCycle()
{
	long minFlybackCycle = GetMinFlybackCycle();
	if(_minimizeFlybackCycles || minFlybackCycle > _imgAcqPty.flybackCycle)
	{
		return minFlybackCycle;
	}
	else if(GetFlybackTime(_imgAcqPty.flybackCycle) > MAX_FLYBACK_TIME)
	{
		double galvoSamplesPaddingTime = (GALVO_RETRACE_TIME / 1000000.0);  //used fixed amount of time for x galvo retrace
		double galvoSamplesPerLine = 2.0 * (double)_imgAcqPty.pixelX * _imgAcqPty.dwellTime /1000000.0 + 4.0 * galvoSamplesPaddingTime;
		return static_cast<long>(MAX_FLYBACK_TIME / galvoSamplesPerLine);
	}
	else
	{
		return _imgAcqPty.flybackCycle;
	}
}

int CThorDAQGGDFLIMSim::CountChannelBits(long channelSet)
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
UINT CThorDAQGGDFLIMSim::HandleFrameBuf(int enter,DWORD timeOut)
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