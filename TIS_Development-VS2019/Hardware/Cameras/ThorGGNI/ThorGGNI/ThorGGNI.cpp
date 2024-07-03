// ThorGGNI.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ThorGGNI.h"
#include "ThorGGNISetupXML.h"
#include "Strsafe.h"

ThorLSMCam::ThorLSMCam() :
	MIN_PIXEL_X(32),
	MAX_PIXEL_X(4096),
	MIN_PIXEL_Y(1),
	MAX_PIXEL_Y(4096),
	DEFAULT_PIXEL_X(512),
	DEFAULT_PIXEL_Y(512),
	DEFAULT_FIELD_SIZE_X(180),
	MIN_ALIGNMENT(-5000),
	MAX_ALIGNMENT(5000),
	DEFAULT_ALIGNMENT(0),
	MIN_INPUTRANGE(0),
	MAX_INPUTRANGE(0),
	DEFAULT_INPUTRANGE(0),
	MIN_CLOCKSOURCE(1),
	MAX_CLOCKSOURCE(1),
	DEFAULT_CLOCKSOURCE(1),
	MIN_EXTCLOCKRATE(10000), //10KSPS
	MAX_EXTCLOCKRATE(125000000), //125MSPS
	DEFAULT_EXTCLOCKRATE(2000000), //2MS/s
	MIN_INTERNALCLOCKRATE(2000000),
	MAX_INTERNALCLOCKRATE(2000000),
	DEFAULT_INTERNALCLOCKRATE(2000000),
	MIN_AVERAGENUM(2),
	MAX_AVERAGENUM(1024),
	DEFAULT_AVERAGENUM(8),
	MIN_SCANMODE(0),
	MAX_SCANMODE(4),
	DEFAULT_SCANMODE(ScanMode::FORWARD_SCAN),
	MIN_TRIGGER_MODE(ICamera::FIRST_TRIGGER_MODE),
	MAX_TRIGGER_MODE(ICamera::HW_MULTI_FRAME_TRIGGER_CONT),
	DEFAULT_TRIGGER_MODE(2), //SW_FREE_RUN_MODE
	MIN_CHANNEL(1),
	MIN_TRIGGER_TIMEOUT(1),
	MAX_TRIGGER_TIMEOUT(2147483),
	MIN_ENABLE_FRAME_TRIGGER(0),
	MAX_ENABLE_FRAME_TRIGGER(1),
	MIN_Y_AMPLITUDE_SCALER(0),
	MAX_Y_AMPLITUDE_SCALER(1000),
	DEFAULT_Y_AMPLITUDE_SCALER(100),
	MIN_AREAMODE(ICamera::FIRST_AREA_MODE),
	MAX_AREAMODE(ICamera::LAST_AREA_MODE),
	MIN_FLYBACK_CYCLE(0),
	MAX_FLYBACK_CYCLE(1000000),
	DEFAULT_FLYBACK_CYCLE(1),
	MIN_GALVO_ENABLE(0),
	MAX_GALVO_ENABLE(1),
	DEFAULT_GALVO_ENABLE(1),
	MAX_RASTERANGLE(180.0),
	MIN_RASTERANGLE(-180.0),
	DEFAULT_RASTERANGLE(0.0),
	MAX_FORWARD_LINE_DUTY(1.0),
	MIN_FORWARD_LINE_DUTY(0.5),
	DEFAULT_FORWARD_LINE_DUTY(0.5),
	GALVO_PARK_POSITION(10.0),
	GALVO_PADDING_SAMPLE(64),
	GALVO_MOVE_PATH_SLEEP(2),
	GALVO_LINE_PADDING_PERCENT(0.25),
	MIN_Y_CHANNEL_ENABLE(0),
	MAX_Y_CHANNEL_ENABLE(1),
	DEFAULT_Y_CHANNEL_ENABLE(1),
	MAX_POCKELS_LINE_BLANKING_PERCENTAGE(49),
	MAX_SCANAREA_ANGLE(PI),
	MIN_SCANAREA_ANGLE(-PI),
	DEFAULT_SCANAREA_ANGLE(0.0),
	MAX_FLYBACK_TIME(5.0),
	MAX_TB_LINE_SCAN_TIME(60000),
	MIN_TB_LINE_SCAN_TIME(2),
	MAX_TIMED_BASED_SCAN_PIXEL_Y(32768)
{
	_errMsg[0] = 0;
	::InitializeCriticalSection(&_hThreadSection);

	memset(_datamap,0,sizeof(_datamap));

	{
		//folding mapping for positive and negative amplifiers
		//datamap should reflect 14bit resolution of the digitizer

		//negative voltage mapping
		for (int i = 0; i <= 32767; i++)
		{
			_datamapIndependent[i] = (32767 - i) >> 1;
		}

		//positive voltage mapping
		for (int i = 32768; i < 65536; i++)
		{
			_datamapIndependent[i] = 0;//(i - 32767 ) >> 1;
		}

	}

	{
		//16 bit mapping with most significant data in positive polarity
		//positive voltage mapping
		for (int i = 0; i < 65536; i++)
		{
			_datamapPositiveSigned[i] = static_cast<U16>((i>>2)-8192);
		}
	}

	{
		//16 bit mapping with most significant data in negative polarity
		//negative voltage mapping
		for (int i = 0; i < 65536; i++)
		{
			_datamapNegativeSigned[i] = static_cast<U16>(((65535 - i)>>2)-8192);
		}
	}

	{
		//negative voltage mapping
		for (int i = 0; i <= 32767; i++)
		{
			_datamapPositiveUnsigned[i] = 0;//(32767 - i) >> 1;
		}

		//positive voltage mapping
		for (int i = 32768; i < 65536; i++)
		{
			_datamapPositiveUnsigned[i] = (i - 32767 ) >> 1;
		}
	}

	{
		//negative voltage mapping
		for (int i = 0; i <= 32767; i++)
		{
			_datamapNegativeUnsigned[i] = (32767 - i) >> 1;
		}

		//positive voltage mapping
		for (int i = 32768; i < 65536; i++)
		{
			_datamapNegativeUnsigned[i] = 0;//(i - 32767 ) >> 1;
		}
	}

	_pixelX = DEFAULT_PIXEL_X;
	_pixelY = DEFAULT_PIXEL_Y;
	_fieldSize = DEFAULT_FIELD_SIZE_X;
	_fieldSizeMin = 5;
	_fieldSizeMax = 255;
	_offsetX = 0;
	_offsetY = 0;
	_channel = 0x0001;
	_alignmentForField = DEFAULT_ALIGNMENT;
	_inputRangeChannel[0] = DEFAULT_INPUTRANGE;
	_inputRangeChannel[1] = DEFAULT_INPUTRANGE;
	_inputRangeChannel[2] = DEFAULT_INPUTRANGE;
	_inputRangeChannel[3] = DEFAULT_INPUTRANGE;
	_scanMode = DEFAULT_SCANMODE;
	_averageMode = 0;
	_behaviorFac.reset(new BehaviorFactory(this, (AverageMode)_averageMode));
	_behaviorPtr = _behaviorFac->GetBehaviorInstance(this, (ScanMode)_scanMode, (AverageMode)_averageMode);
	_averageNum = 2;
	_triggerMode = DEFAULT_TRIGGER_MODE;
	_frameCount = 1;
	_droppedFramesCnt = 0;
	_maxChannel = 0xF;
	_scanAreaAngle = DEFAULT_SCANAREA_ANGLE;

	_triggerWaitTimeout = 30;
	_frameTriggerEnableWithHWTrig = 1;

	_pGalvoStartPos = NULL;
	_frameTrigger = NULL;
	_hThread = NULL;
	_field2Theta = 0.0901639344; //the field to scan angle conversion adapted from resonant galvo scanner code
	_theta2Volts = 1.0;
	_crsFrequencyHighPrecision = 7931.47208;
	_minGalvoFreqHz[0] = _minGalvoFreqHz[1] = (double)Constants::DEFAULT_GALVO_HZ;

	_clockSource = DEFAULT_CLOCKSOURCE;
	_clockRateInternal = DEFAULT_INTERNALCLOCKRATE;
	_pixelClockLength = (long) (125000000.0 / _crsFrequencyHighPrecision);
	_areaMode = ICamera::SQUARE;
	_yAmplitudeScaler = DEFAULT_Y_AMPLITUDE_SCALER;
	setFlybackCycle(DEFAULT_FLYBACK_CYCLE);
	_galvoEnable = true;
	_dwellTime = DEFAULT_DWELL_TIME; 
	_rasterAngle = DEFAULT_RASTERANGLE; 
	_galvoForwardLineDuty =DEFAULT_FORWARD_LINE_DUTY; 
	_yChannelEnable = true;			//For ICamera::PARAM_LSM_Y_COMMUNICATION_ENABLE, not implemented yet.
	_forceSettingsUpdate = FALSE;
	_ggSuperUserMode = FALSE;
	_timebasedLSTimeMS = MIN_TB_LINE_SCAN_TIME;

	_dataMapMode = ICamera::POLARITY_MIXED;
	_channelPolarity[0] = POL_NEG;
	_channelPolarity[1] = POL_NEG;
	_channelPolarity[2] = POL_NEG;
	_channelPolarity[3] = POL_NEG;

	_taskHandleDO2 = 0;
	_taskHandleDO3 = 0;
	_taskHandleCO0 = 0;

	_clockRateExternal = DEFAULT_EXTCLOCKRATE;

	_shutterLine = "";

	for(long i=0; i<MAX_GG_POCKELS_CELL_COUNT; i++)
	{
		_pockelsLineBlankingPercentage[i] = 0.0;
		_pockelsPowerLevel[i] = 0.0;
		_pockelsEnable[i] = false;
		_pockelsMinVoltage[i] = 0.0;
		_pockelsMaxVoltage[i] = 1.0;
		_taskHandleAIPockels[i] = 0;
		_pockelsScanVoltageStart[i] = 0.0;
		_pockelsScanVoltageStop[i] = 0.0;
		_pockelsLine[i] = "";
		_pockelsPowerInputLine[i] = "";
		memset(&_pockelsReadArray[i],0,POCKELS_VOLTAGE_STEPS * sizeof(float64));
	}

	_pockelsParkAtMinimum = 0;
	_galvoParkAtStart = 0;
	_pockelsVoltageSlopeThreshold = 0.01;

	_frameTriggerLineInOut = "/Dev2/port0/line6:7";
	_readLine[0] ="/" + _devID + "/ai0";
	_readLine[1] = "/" + _devID + "/ai1";
	_readLine[2] = "/" + _devID + "/ai0:1";
	_clockExportLine = "/" + _devID + "/PFI5";

	_verticalScanDirection = 1.0;

	_fineOffset[0] = _fineOffset[1] = _fineOffset2[0]= _fineOffset2[1] = 0.0;
	_highResOffset[0] = _highResOffset[1] = _highResOffset2[0] = _highResOffset2[1] = 0.0;
	_highResOffsetMinMax[0] = -3.0;
	_highResOffsetMinMax[1] = 3.0;
	_fineFieldSizeScaleX=1.0;
	_fineFieldSizeScaleY=1.0;

	_oneXFieldSize = 160;

	//ensure that all parameters are set when the parameter comparasion occurs on the first pass
	memset(&_ImgPty,0,sizeof(_VCMImgPty));
	memset(&_ImgPty_Pre,0xFF,sizeof(_VCMImgPty));

	for (long i = 0; i < MAX_BOARD_NUM; i++)
	{
		for (long j = 0; j < MAX_DMABUFNUM; j++)
		{
			_pData[i][j] = NULL;
		}
		_lastSizePerBuffer[i] = 0;
	}

	for (int j = 0; j < MAX_DMABUFNUM; j++)
	{
		_pFrmDllBuffer[j] = NULL;
	}

	_fieldSizeCalibration = 0;
	_fieldSizeCalibrationAvailable = FALSE;
	_galvoRetraceTime = GALVO_MIN_RETRACE_TIME;
	_horizontalFlip = 0;
	_lineFlybackLength = LINE_FRAMETRIGGER_LOW_TIMEPOINTS;
	_pockelsPhaseDelayUS = 0;
	_useReferenceForPockelsOutput = FALSE;
	_fileSettingsLoaded = FALSE;
	_centerWithOffsets = FALSE;
	_pockelsTurnAroundBlank = TRUE;
	_pockelsFlybackBlank = TRUE;
	_interleaveScan = 0;
	_imageActiveLoadMS = 100;
	_imageActiveLoadCount = Constants::ACTIVE_LOAD_BLKSIZE_DEFAULT;
	_lastBufferSize = 0;
	_lastDMABufferCount = 0;
	_analogXYmode[0][0] = _analogXYmode[0][1] = _analogXYmode[1][1] = _analogXYmode[1][1] = 1;
	_waveformOutPath = L"";
	_maxAngularVelocityRadPerSec = DEFAULT_MAX_GALVO_VELOCITY;
	_maxAngularAccelerationRadPerSecSq = DEFAULT_MAX_GALVO_ACCELERATION;
}

///Initialize Static Members
bool ThorLSMCam::_instanceFlag = false;
auto_ptr<ThorLSMCam> ThorLSMCam::_single(new ThorLSMCam());
U16* ThorLSMCam::_datamap[4] = {0};
U16 ThorLSMCam::_datamapPositiveSigned[65536] = {0};
U16 ThorLSMCam::_datamapNegativeSigned[65536] = {0};
U16 ThorLSMCam::_datamapPositiveUnsigned[65536] = {0};
U16 ThorLSMCam::_datamapNegativeUnsigned[65536] = {0};
U16 ThorLSMCam::_datamapIndependent[65536] = {0};
HANDLE ThorLSMCam::_hStopAcquisition = CreateEvent(NULL, true, false, NULL);  //2nd parameter "true" so it needs manual "Reset" after "Set (signal)" event
HANDLE ThorLSMCam::_hFrmBufReady = CreateEvent(NULL, true, false, NULL);
HANDLE ThorLSMCam::_hFrmBufHandle = CreateMutex(NULL, false, NULL);
HANDLE ThorLSMCam::_hNIRawFrmReadyHandle = CreateEvent(NULL, true, false, NULL);
HANDLE ThorLSMCam::_hNIRawFrmCopiedHandle = CreateEvent(NULL, true, false, NULL);
HANDLE ThorLSMCam::_hPartialFrmBufHandle = CreateMutex(NULL, false, NULL);
HANDLE ThorLSMCam::_hThreadStopped = CreateEvent(NULL, true, true, NULL);
HANDLE ThorLSMCam::_hReadThreadStarted = CreateEvent(NULL, true, true, NULL);
HANDLE ThorLSMCam::_hStatusError = CreateEvent(NULL, false, false, NULL);    //NI error event, auto reset after wait single and will trigger termination of status check
HANDLE ThorLSMCam::_hHardwareTriggerInEvent = CreateEvent(NULL, false, false, NULL);
HANDLE ThorLSMCam::_hSpecialDigitalLineReadyHandle = CreateMutex(NULL, false, NULL);
HANDLE ThorLSMCam::_hThread = NULL;
VCMImgPty ThorLSMCam::_imgPtyDll = VCMImgPty();
WaveformGenParams ThorLSMCam::_waveGenParams = WaveformGenParams();
GGalvoWaveformParams ThorLSMCam::_gGalvoWaveParams = GGalvoWaveformParams();
std::wstring ThorLSMCam::_waveformPathName = L"";
unsigned long ThorLSMCam::_recsPerBuffer = 0;
unsigned long ThorLSMCam::_channelMode[MAX_BOARD_NUM] = {0};
unsigned long ThorLSMCam::_numChannel[MAX_BOARD_NUM] = {0};
long ThorLSMCam::_sizePerBuffer[MAX_BOARD_NUM] = {0};
U16* ThorLSMCam::_pData[MAX_BOARD_NUM][MAX_DMABUFNUM] = {NULL};
unsigned short* ThorLSMCam::_pFrmDllBuffer[MAX_DMABUFNUM] = {NULL};
const double ThorLSMCam::_crsFrequency = 8000.0;
HANDLE ThorLSMCam::_hStatusHandle = NULL;
long ThorLSMCam::_indexOfLastCompletedFrame = -1;
long ThorLSMCam::_indexOfLastCopiedFrame = -1;
long ThorLSMCam::_dMABufferCount = 4;
TaskHandle ThorLSMCam::_taskHandleCOSampling = NULL;
TaskHandle ThorLSMCam::_taskHandleCOFreq = NULL;
long ThorLSMCam::_pockelsEnableIntegrated = false;	//this feature is not supported
long ThorLSMCam::_clockRateNI = 2000000;
long ThorLSMCam::_clockRatePockels = 250000;
long ThorLSMCam::_scanMode = TWO_WAY_SCAN;
long ThorLSMCam::_recordCount = 0;
long ThorLSMCam::_areaMode = ICamera::LSMAreaMode::SQUARE;
double ThorLSMCam::_pockelsPhaseDelayUS = 0;
long ThorLSMCam::_progressCounter = 0;
TaskHandle ThorLSMCam::_taskHandleDO3 = NULL;
string ThorLSMCam::_captureActiveOutput = "";
string ThorLSMCam::_frameBufferReadyOutput = "";
long ThorLSMCam::_captureActiveOutputInvert = 0;
string ThorLSMCam::_startTriggerLine = "/Dev2/PFI6";
string ThorLSMCam::_frameTriggerLineIn = "/Dev2/PFI4";
string ThorLSMCam::_controllerInternalOutput0 = "/Dev2/Ctr0InternalOutput";
string ThorLSMCam::_controllerInternalOutput1 = "/Dev2/Ctr1InternalOutput";
string ThorLSMCam::_controllerInternalOutput2 = "/Dev3/Ctr0InternalOutput";
string ThorLSMCam::_galvoLinesInput = "/Dev2/ai14:15";
string ThorLSMCam::_galvoLinesOutput = "/Dev2/ao0:1";
string ThorLSMCam::_galvoAndPockelsLinesOutput = "/Dev2/ao0:1";
string ThorLSMCam::_controllerOutputLine0 = "/Dev2/ctr0";
string ThorLSMCam::_controllerOutputLine1 = "/Dev2/ctr1";
string ThorLSMCam::_controllerOutputLine2 = "/Dev3/ctr0";
string ThorLSMCam::_controllerOutputLine3 = "/Dev2/ctr2";
string ThorLSMCam::_pockelsTriggerIn = "/Dev3/PFI0";
string ThorLSMCam::_digitalTriggerLines = "/Dev2/port0/line6";
TaskHandle ThorLSMCam::_taskHandleAI0 = NULL;
TaskHandle ThorLSMCam::_taskHandleAO1 = 0;
TaskHandle ThorLSMCam::_taskHandleDO1 = 0;
TaskHandle ThorLSMCam::_taskHandleAOPockels = 0;
TaskHandle ThorLSMCam::_taskHandleDI1 = 0;
TaskHandle ThorLSMCam::_taskHandleCO0 = NULL;
TaskHandle ThorLSMCam::_taskHandleCO1 = 0;
TaskHandle ThorLSMCam::_taskHandleCO2 = 0;
float64 *ThorLSMCam::_pGalvoWaveformXYP = NULL;
float64 *ThorLSMCam::_pPockelsWaveform = NULL;
uInt8* ThorLSMCam::_frameTrigger = NULL;
long ThorLSMCam::_pockelsEnable[MAX_GG_POCKELS_CELL_COUNT]={false, false, false, false};
std::string ThorLSMCam::_pockelsLine[MAX_GG_POCKELS_CELL_COUNT] = {"", "", "", ""};
double ThorLSMCam::_maxGalvoOpticalAngle = 20.0;
double ThorLSMCam::_minSignalInputVoltage = -10.0;
double ThorLSMCam::_maxSignalInputVoltage = 10.0;
double ThorLSMCam::_avgCount = 1;
long ThorLSMCam::_stopReading = FALSE;
string ThorLSMCam::_pockelsReferenceLine = "";
long ThorLSMCam::_pockelsReferenceRequirementsMet = FALSE;
wchar_t ThorLSMCam::_errMsg[_MAX_PATH] = {NULL};
IBehavior* ThorLSMCam::_behaviorPtr = NULL;
std::unique_ptr<BlockRingBuffer> ThorLSMCam::_wBuffer[SignalType::SIGNALTYPE_LAST] = {NULL};
int ThorLSMCam::_digiLineSelect = 0x0;
int ThorLSMCam::_pockelsSelect = 0x0;
string ThorLSMCam::_analogChannels[MAX_ANALOG_CHAN_COUNT] = {""};
string ThorLSMCam::_devID = "Dev2";
HMODULE ThorLSMCam::hDLLInstance = NULL;
std::unique_ptr<BoardInfoNI> ThorLSMCam::_boardInfoNI;
long ThorLSMCam::_triggerWaitTimeout = 0;
long ThorLSMCam::_numNIDAQ = 0;
long ThorLSMCam::_shiftArray[256];
double ThorLSMCam::_minDwellTime = 1.0;
const double ThorLSMCam::MAX_DWELL_TIME = 10.0;
const double ThorLSMCam::DEFAULT_DWELL_TIME = 2.0;
const double ThorLSMCam::DWELL_TIME_STEP = 0.5;
long ThorLSMCam::_timeBasedLineScanEnabled = FALSE;
long ThorLSMCam::_bufferLarge = FALSE;
int	ThorLSMCam::_acquireStatus = (int)ICamera::STATUS_READY;
long ThorLSMCam::_isLiveScan = FALSE;

std::unique_ptr<ImageWaveformBuilderDLL> ImageWaveformBuilder(new ImageWaveformBuilderDLL(L".\\Modules_Native\\GeometryUtilitiesCPP.dll"));
std::unique_ptr<AnalogReaderNI> analogReaderNI;
wchar_t message[_MAX_PATH];	//used for logging, use _errMsg for GetLastErrorMsg instead

ThorLSMCam* ThorLSMCam::getInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new ThorLSMCam());
		_instanceFlag = true;
	}
	return _single.get();
}

ThorLSMCam::~ThorLSMCam()
{
	_instanceFlag = false;
}

long ThorLSMCam::CaptureCreateThread(void)
{
	DWORD threadID;
	::EnterCriticalSection(&_hThreadSection);

	SetupReadTask();
	ResetEvent(_hStopAcquisition);
	SAFE_DELETE_HANDLE (_hThread);

	_indexOfLastCompletedFrame = _indexOfLastCopiedFrame = -1;
	_acquireStatus = (int)ICamera::STATUS_BUSY;
	ResetEvent(_hThreadStopped);
	ResetEvent(_hStatusError);
	_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &(ThorLSMCam::StartFrmAsync), (void *) _pFrmDllBuffer, HIGH_PRIORITY_CLASS, &threadID);
	if(NULL == _hThread)
	{
		::LeaveCriticalSection(&_hThreadSection);
		return FALSE;
	}
	SetThreadPriority(_hThread, THREAD_PRIORITY_HIGHEST);

	::LeaveCriticalSection(&_hThreadSection);
	return TRUE;
}

/// <summary> Check if settings maintain valid optical angle </summary>
long ThorLSMCam::CheckOpticalAngle(long fieldSize, double dwellTime, long pixelX)
{
	if (TRUE == static_cast<long>(_ggSuperUserMode))
		return TRUE;

	if (0 >= dwellTime)
		return FALSE;

	if(_maxGalvoOpticalAngle < (fieldSize*_field2Theta*(1 + (4.0*_galvoRetraceTime/(PI * dwellTime * pixelX)))))
	{
		StringCbPrintfW(message,_MAX_PATH,L"ThorGGNI::%hs@%u failed: fieldSize (%d), dwellTime (%.1f), pixelX (%d)",__FUNCTION__, __LINE__,fieldSize,dwellTime,pixelX);
		LogMessage(message,VERBOSE_EVENT);
		return FALSE;
	}
	return TRUE;
}

long ThorLSMCam::ActiveBehavior()
{
	long active = 0;
	_behaviorPtr->GetParam(BehaviorProp::ACTIVE_BEHAVIOR, active);
	return (active) ? TRUE : FALSE; 
}

long ThorLSMCam::AlignDataLoadFile()
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

void ThorLSMCam::SetupDataMap()
{
	unsigned int i;

	switch( _dataMapMode)
	{
	case ICamera::POLARITY_INDEPENDENT:
		{
			//folding mapping for positive and negative amplifiers
			//datamap should reflect 14bit resolution of the digitizer
			//negative voltage mapping
			for (i = 0; i <4; i++)
			{
				_datamap[i] = &(_datamapIndependent[0]);
			}
		}
		break;
	case ICamera::POLARITY_POSITIVE:
		{
			//16 bit mapping with most significant data in positive polarity
			//positive voltage mapping
			for (i = 0; i < 4; i++)
			{
				_datamap[i] = &(_datamapPositiveSigned[0]);
			}
		}
		break;
	case ICamera::POLARITY_NEGATIVE:
		{
			//16 bit mapping with most significant data in negative polarity
			//negative voltage mapping
			for (i = 0; i < 4; i++)
			{
				_datamap[i] = &(_datamapNegativeSigned[0]);
			}
		}
		break;
	default:
		for(i=0; i<4; i++)
		{
			_datamap[i] = (POL_NEG == _channelPolarity[i]) ? &(_datamapNegativeUnsigned[0]) : &(_datamapPositiveUnsigned[0]); 
		}
	}
}

long ThorLSMCam::SetWaveform(VCMImgPty *pImgPty)
{
	//Stop and cancel previous scan
	CloseThread();

	//close thread here since thread stop was signaled
	SAFE_DELETE_HANDLE (_hThread);

	ResetEvent(_hFrmBufReady);

	//get new scan parameters
	_imgPtyDll = *pImgPty;

	//avoid bufferconflict, terminate tasks first
	ThorCloseNITasks();

#ifdef LOGGING_ENABLED
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI pixelX %d", pImgPty->pixelX);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI pixelY %d", pImgPty->pixelY);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI fieldSize %d", pImgPty->fieldSize);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI offsetX %d", pImgPty->offsetX);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI offsetY %d", pImgPty->offsetY);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI channel %d", pImgPty->channel);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI alignmentForField %d", pImgPty->alignmentForField);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI inputRangeChannel1 %d", _inputRangeChannel[0]);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI inputRangeChannel2 %d", _inputRangeChannel[1]);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI inputRangeChannel3 %d", _inputRangeChannel[2]);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI inputRangeChannel4 %d", _inputRangeChannel[3]);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI clockSource %d", pImgPty->clockSource);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI clockRateInternal %d", pImgPty->clockRateInternal);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI clockRateExternal %d", pImgPty->clockRateExternal);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI scanMode %d", pImgPty->scanMode);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI averageMode %d", pImgPty->averageMode);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI averageNum %d", pImgPty->averageNum);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI triggerMode %d", pImgPty->triggerMode);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI numFrame %d", pImgPty->numFrame);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI Y Amplitude Scaler %d", pImgPty->yAmplitudeScaler);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI Area Mode %d", pImgPty->areaMode);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI Flyback Cycle %d", pImgPty->flybackCycle);
	LogMessage(message,VERBOSE_EVENT);
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI Datamap Mode %d", pImgPty->dataMapMode);
	LogMessage(message,VERBOSE_EVENT);
#endif

	//setup digital lines:
	_digiLineSelect = 2;
	_frameTriggerLineInOut = "/" + _devID + "/port0/line6" + ",/" + _devID + "/port0/line7";
	for (size_t i = 0; i < MAX_GG_POCKELS_CELL_COUNT; i++)
	{
		if (_pockelDigOut[i].size() > 0)
		{
			_frameTriggerLineInOut += "," + _pockelDigOut[i];
			_digiLineSelect += 1;
		}
	}

	//keep digital lines low before start
	TogglePulseToDigitalLine(_taskHandleDO1, _frameTriggerLineInOut, _digiLineSelect, TogglePulseMode::ToggleLow);

	if (CheckOpticalAngle(_imgPtyDll.fieldSize, _imgPtyDll.dwellTime, _imgPtyDll.pixelX))
	{
		//set frame count by trigger mode
		switch (_imgPtyDll.triggerMode)
		{
		case ICamera::SW_SINGLE_FRAME:
		case ICamera::HW_SINGLE_FRAME:
			_imgPtyDll.numFrame = 1;
			break;
		}

		//clock the galvos according to the entered dwell time (non-waveform scan mode)
		_clockRateNI = static_cast<long>(Constants::MHZ/_imgPtyDll.dwellTime);

		//build XY/Pockels/Trigger wavefrom in waveform builder:
		double callbackTime = static_cast<double>(_imageActiveLoadMS) / Constants::MS_TO_SEC;
		for (int i = 0; i < static_cast<long>(SignalType::SIGNALTYPE_LAST); i++)
		{
			_dLengthPerAOCallback[i] = max(1, static_cast<long>(floor(_clockRateNI * callbackTime / 100) * 100));	//round to 100's samples callback
		}

		ImageWaveformBuilder->SetWaveformBuilderBoardID(BuilderType::ALAZAR);

		UpdateWaveformGenParams();

		ImageWaveformBuilder->SetWaveformGenParams(&_waveGenParams);

		_pGalvoStartPos = (float64*)realloc((void*)_pGalvoStartPos, 2 * sizeof(double));
		if (0 == ImageWaveformBuilder->BuildImageWaveform(_pGalvoStartPos, _dLengthPerAOCallback, _totalLength, _waveformOutPath))
		{
			StringCbPrintfW(_errMsg,_MAX_PATH,L"ImageWaveformBuilder unable to build image waveform.");
			LogMessage(_errMsg,ERROR_EVENT);
			return FALSE;
		}

		//reset output path after build
		_waveformOutPath = L"";

		//Setup buffers for active load
		CreateWaveBuffers();
		ConnectWaveBufferCallbacks();
		FillWaveBuffers();

		//do nothing if not active behavior
		if(!ActiveBehavior())
			return FALSE;

		//setup active behavior
		_behaviorPtr = _behaviorFac->GetBehaviorInstance(this, (ScanMode)_imgPtyDll.scanMode, (AverageMode)_imgPtyDll.averageMode);
		_behaviorPtr->SetParam(BehaviorProp::INTERLEAVE_BUF, _imgPtyDll.interleaveScan);
		_behaviorPtr->SetParam(BehaviorProp::PROC_BUF_LINE_COUNT, static_cast<long>(ImageWaveformBuilder->GetForwardLines()));
	}
	return TRUE;
}

void ThorLSMCam::CloseLSMCam()
{
	StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI CloseLSMCam", 1);
	LogMessage(message,VERBOSE_EVENT);

	CloseThread();

	ThorCloseNITasks();

	for (long i = 0; i < MAX_BOARD_NUM; i++)
	{
		for (long j = 0; j < MAX_DMABUFNUM; j++)
		{
			SAFE_DELETE_MEMORY(_pData[i][j]);
		}
		_lastSizePerBuffer[i] = 0;
	}

	SAFE_DELETE_MEMORY(_remap_index);

	for(long i=0; i< MAX_DMABUFNUM; i++)
	{
		SAFE_DELETE_MEMORY(_pFrmDllBuffer[i]);
	}
	_lastBufferSize = 0;
	_lastDMABufferCount = 0;

	_bufferOrder.clear();

	SAFE_DELETE_MEMORY (_frameTrigger);

	SAFE_DELETE_HANDLE (_hThread);

	::DeleteCriticalSection(&_remapSection);
}

void ThorLSMCam::CloseThread()
{
	//terminate current process thread
	if (ActiveBehavior())
		_behaviorPtr->SetParam(BehaviorProp::SWITCH_BEHAVIOR, 1);

	SetEvent(_hStopAcquisition);

	::EnterCriticalSection(&_hThreadSection);

	if(NULL != _hThread)
	{
		ThorCloseNITasks();

		if(WAIT_OBJECT_0 != WaitForSingleObject(_hThreadStopped, INFINITE))
		{
			StringCbPrintfW(_errMsg,_MAX_PATH,L"CloseThread Failed.");
			LogMessage(_errMsg,ERROR_EVENT);
		}

		MovePockelsToParkPosition();
		SAFE_DELETE_HANDLE (_hThread);
	}

	::LeaveCriticalSection(&_hThreadSection);
}

void ThorLSMCam::CreateWaveBuffers()
{
	long bufCount = Constants::ACTIVE_LOAD_BLKSIZE_DEFAULT;
	for (int i = 0; i < static_cast<int>(SignalType::SIGNALTYPE_LAST); i++)
	{
		bufCount = min(_imageActiveLoadCount, max(1, static_cast<long>(_totalLength[i] / _dLengthPerAOCallback[i])));

		switch ((SignalType)i)
		{
		case SignalType::ANALOG_XY:
			_wBuffer[i].reset(new BlockRingBuffer(i, sizeof(double), bufCount, 2*_dLengthPerAOCallback[i]));
			break;
		case SignalType::ANALOG_POCKEL:
			_wBuffer[i].reset(new BlockRingBuffer(i, sizeof(double), bufCount, _pockelsSelect*_dLengthPerAOCallback[i]));
			break;
		case SignalType::DIGITAL_LINES:
			_wBuffer[i].reset(new BlockRingBuffer(i, sizeof(unsigned char), bufCount, _digiLineSelect*_dLengthPerAOCallback[i]));
			break;
		}
	}
}

void ThorLSMCam::ConnectWaveBufferCallbacks()
{
	for (int i = 0; i < static_cast<int>(SignalType::SIGNALTYPE_LAST); i++)
	{
		if (NULL != _wBuffer[i].get())
			ImageWaveformBuilder->ConnectBufferCallback((SignalType)i, _wBuffer[i].get());
	}
}

void ThorLSMCam::FillWaveBuffers()
{
	for (int i = 0; i < static_cast<int>(SignalType::SIGNALTYPE_LAST); i++)
	{
		if (NULL != _wBuffer[i].get())
			ImageWaveformBuilder->BufferAvailableCallbackFunc(i, _wBuffer[i].get()->CheckWritableBlockCounts(FALSE));
	}
}

/// <summary> Cancels any hardware triggers the camera is currently waiting on </summary>
void ThorLSMCam::StopHardwareWaits()
{
	SetEvent(_hStopAcquisition);

	PostflightAcquisition(NULL);
}

/// <summary> Waits on a hardware trigger. Will also stop with a timeout or a software stop acquisition event</summary>
/// <param name="timeoutMillis"> Max time to wait </param>
/// <param name="showTimeoutMessage"> Whether to show a message box warning of a timeout </param>
/// <returns> Returns true if a hardware trigger stopped the wait, false otherwise </returns>
long ThorLSMCam::WaitForHardwareTrigger(long timeoutMillis, long showTimeoutMessageBox)
{
	//Setup Wait Event Array
	std::vector<HANDLE> hardwareEvents;
	hardwareEvents.push_back(_hHardwareTriggerInEvent);
	hardwareEvents.push_back(_hStopAcquisition);

	//Used to check if the hardware event was the cause of the wait stop
	auto it = std::find(hardwareEvents.begin(), hardwareEvents.end(), _hHardwareTriggerInEvent);
	int hardwareIndex = static_cast<int>(std::distance(hardwareEvents.begin(), it));
	DWORD ret = 0;
	switch(_imgPtyDll.triggerMode)
	{
	case ICamera::SW_FREE_RUN_MODE:
	case ICamera::SW_MULTI_FRAME:
	case ICamera::SW_SINGLE_FRAME:
		return TRUE;
	case ICamera::HW_SINGLE_FRAME:
	case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
		//Wait for an event
		ret = WaitForMultipleObjects(static_cast<DWORD>(hardwareEvents.size()), hardwareEvents.data(), false, timeoutMillis);
		break;
	}

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
				StringCbPrintfW(_errMsg,_MAX_PATH,L"External trigger is timed out after %d seconds.",timeoutMillis/Constants::MS_TO_SEC);
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

void ThorLSMCam::UpdateWaveformGenParams()
{
	_waveGenParams.galvoRetraceTime = _galvoRetraceTime;
	_waveGenParams.areaMode = _imgPtyDll.areaMode;
	_waveGenParams.dwellTime = _imgPtyDll.dwellTime;
	_waveGenParams.fieldScaleFineX = _imgPtyDll.fineFieldSizeScaleX;
	_waveGenParams.fieldScaleFineY = _imgPtyDll.fineFieldSizeScaleY;
	_waveGenParams.fieldSize = _imgPtyDll.fieldSize;
	_waveGenParams.fineOffset[0] = _imgPtyDll.fineOffset[0];
	_waveGenParams.fineOffset[1] = _imgPtyDll.fineOffset[1];
	_waveGenParams.numFrame = _imgPtyDll.numFrame;
	_waveGenParams.verticalScanDirection = static_cast<long>(_imgPtyDll.verticalScanDirection);
	_waveGenParams.offsetX = _imgPtyDll.offsetX;
	_waveGenParams.offsetY = _imgPtyDll.offsetY;
	_waveGenParams.PixelX = _imgPtyDll.pixelX;
	_waveGenParams.PixelY = _imgPtyDll.pixelY;
	for(int i=0;i<MAX_GG_POCKELS_CELL_COUNT;i++)
	{
		_waveGenParams.pockelsIdlePower[i] = _pockelsMinVoltage[i];
		_waveGenParams.pockelsMaxPower[i] = _pockelsMaxVoltage[i];
		_waveGenParams.pockelsPower[i] = _pockelsMinVoltage[i] + (_pockelsMaxVoltage[i] - _pockelsMinVoltage[i]) * _imgPtyDll.pockelsPowerLevel[i];
		_waveGenParams.pockelsLineEnable[i] = _pockelsEnable[i];
		_waveGenParams.pockelsLineBlankingPercentage[i] = _imgPtyDll.pockelsLineBlankingPercentage[i];
	}
	_waveGenParams.scaleYScan = _imgPtyDll.yAmplitudeScaler;
	_waveGenParams.scanAreaAngle = _imgPtyDll.scanAreaAngle;
	_waveGenParams.flybackCycles = _imgPtyDll.flybackCycle;
	_waveGenParams.minLowPoints = LINE_FRAMETRIGGER_LOW_TIMEPOINTS;
	_waveGenParams.clockRatePockels = _clockRatePockels;
	_waveGenParams.field2Volts = _field2Theta;
	_waveGenParams.digLineSelect = _digiLineSelect;
	_waveGenParams.pockelsTurnAroundBlank = _pockelsTurnAroundBlank;
	_waveGenParams.pockelsFlybackBlank = _pockelsFlybackBlank;
	_waveGenParams.scanMode = _imgPtyDll.scanMode;
	_waveGenParams.yAmplitudeScaler = _imgPtyDll.yAmplitudeScaler;
	_waveGenParams.galvoEnable = _imgPtyDll.galvoEnable;
	_waveGenParams.useReferenceForPockelsOutput = _imgPtyDll.useReferenceForPockelsOutput;
	_waveGenParams.pockelsReferenceRequirementsMet = _pockelsReferenceRequirementsMet;
	_waveGenParams.scanAreaIndex = 0;		//0-based, prepare for multi-scan area
	_waveGenParams.scanAreaCount = 1;		//total scan area number, prepare for multi-scan area
	_waveGenParams.interleaveScan = _imgPtyDll.interleaveScan;
}

///	***************************************** <summary> Input Range Functions </summary>	********************************************** ///

long ThorLSMCam::SetRngA(long RngA)
{
	int32 error = 0;
	return error;
}

long ThorLSMCam::SetRngB(long RngB)
{
	int32 error = 0;
	return error;
}

long ThorLSMCam::SetRngC(long RngC)
{
	int32 error = 0;
	return error;
}

long ThorLSMCam::SetRngD(long RngD)
{
	int32 error = 0;
	return error;
}

long ThorLSMCam::SetXRemap(HANDLE h_bd, long eff_gsizoom)

{
	long error = 0;
	if (_imgPtyDll.scanMode == TWO_WAY_SCAN)
		error = Set2WayScanRemap(h_bd, 0, _imgPtyDll.pixelX, _imgPtyDll.fieldSize, _imgPtyDll.offsetX);
	else if (_imgPtyDll.scanMode == FORWARD_SCAN)
		error = Set1WayScanRemap(h_bd, 0, _imgPtyDll.pixelX, _imgPtyDll.fieldSize, _imgPtyDll.offsetX);
	else
		error = Set1WayScanRemapBack(h_bd, 0, _imgPtyDll.pixelX, _imgPtyDll.fieldSize, _imgPtyDll.offsetX);

	return error;
}

long ThorLSMCam::Set2WayScanRemap(HANDLE h_bd, long shift, unsigned xpixel, long fieldX, long offsetX)
{
	//data skipping begins
	long remap_shift = shift;
	int32 error = 0;

	double x;

	double dxsize = (double) xpixel;

	double dclocklength = 0;//_vCMBoardDefinition.RecLength;

	double byPass=0;

	_remap_size = 2 * xpixel + 2;
	_remap_index = (long*) realloc(_remap_index,sizeof(long) * _remap_size);

	_remap_index[0]=0;

	for (unsigned i = 1; i <= xpixel; i++)
	{
		x = i;
		_remap_index[i] = static_cast<long>(byPass + (dclocklength/dxsize*x));
		_remap_index[2*xpixel+1-i] = static_cast<long>(dclocklength *2 -(dclocklength/dxsize*x) -1 - byPass);
	}
	_remap_index[2*xpixel+1] = static_cast<long>(dclocklength *2-1);

	return error;
}

long ThorLSMCam::Set1WayScanRemap(HANDLE h_bd, long shift, unsigned xpixel, long fieldX, long offsetX)
{
	//data skipping begins
	long remap_shift = shift;
	int32 error = 0;

	double x;

	//offset is considered here
	double marginalfactor = 2.0 * (double)(abs(offsetX)) / (double) (fieldX);
	double marginalxsize = (double) xpixel * marginalfactor;
	double dxsize = (double) xpixel;

	double dclocklength = 0;//_vCMBoardDefinition.RecLength;

	double byPass=0;

	_remap_size = xpixel + 1;
	_remap_index = (long*) realloc(_remap_index,sizeof(long) * _remap_size);

	_remap_index [0] =0;
	for (unsigned i = 1; i <= xpixel; i++)
	{
		x = i;
		_remap_index[i] = static_cast<long>(byPass + (dclocklength/dxsize*x));
	}

	return error;
}

long ThorLSMCam::Set1WayScanRemapBack(HANDLE h_bd, long shift, unsigned xpixel, long fieldX, long offsetX)
{
	//data skipping begins
	long remap_shift = shift;
	int32 error = 0;

	return error;
}
