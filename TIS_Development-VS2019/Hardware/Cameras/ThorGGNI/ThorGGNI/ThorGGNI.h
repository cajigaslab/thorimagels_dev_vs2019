// ThorGGNI.h : main header file for the ThorLSMCam DLL
//
#include "stdafx.h"

//#define INTERNAL_CLOCK_PIXEL_LENGTH 15760
#ifndef FLYBACK_CYCLE_SCALE
#define FLYBACK_CYCLE_SCALE 4
#endif
#define MAX_BOARD_NUM						8
#define MAX_DMABUFNUM						2048
#define POCKELS_VOLTAGE_STEPS				100

#define GALVO_MOVE_PATH_LENGTH				128
#define MAX_GALVO_READ_TIMEOUT				10.0

#define DATA_SAMPLE_RATE					2000000
#define LINE_FRAMETRIGGER_LOW_TIMEPOINTS	0
#define INFINITE_COUNT						0x7FFFFFFF
#define MAX_ANALOG_CHAN_COUNT				4
#define LINE_POST_COUNT						16


extern std::unique_ptr<ImageWaveformBuilderDLL> ImageWaveformBuilder;
extern std::unique_ptr<AnalogReaderNI> analogReaderNI;

//factor to scale the field size (0 ~ 255) to theta, a value not very critical and accurate
// h= f * Theta,  h = 25, f=70mm, Theta = 20.46
//Convert Theta to radians and multiply by the maximum value of 255 gives the result below
//#define FIELD2THETA 0.08024619 //calculated for an ftheta lens to image a maximum field size of 25mm^2
//#define FIELD2THETA 0.0901639344

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct _VCMImgPty
	{
		long pixelX; ///<Image pixel number in the x direction
		long pixelY; ///<Image pixel number in the y direction
		long fieldSize; ///<A parameter scales to scan field size, the actual scan sized at the object plane may vary from device to device
		long offsetX; ///<Off set of scan field size in the x direction
		long offsetY; ///<Off set of scan field size in the y direction
		long channel; ///<Bitwise selection of channels.
		long alignmentForField; ///<Forward Backward scan alignment
		long clockSource; ///<0 to use internal sample clock source, 1 to use external clock source
		long clockRateInternal; ///< internal sample clock rate, the enumeration of "ClockRate"
		long clockRateExternal; ///< external sample clock rate, this the actual sample rate
		long scanMode; ///< 2 way, 1 way forward or 1 way backward scan mode, see enumeration of ScanMode
		long averageMode; ///< average mode, see enumeration of AverageMode;
		long averageNum;///< number of frame, lines to average
		long triggerMode; ///<trigger source of each frame
		long numFrame; ///<number of frame to acquire for a experiment
		double yAmplitudeScaler;///<pixel aspect ratio scaler
		long areaMode;///<scan pattern mode square,rect,line
		long flybackCycle;///<number of line cycles to wait when galvo is flying back to start of the frame
		long dataMapMode;///<mapping of raw digital values into output buffer
		double dwellTime; ///<number of lines per second for the galvo-galvo raster scan
		double rasterAngle; ///<angle of scan rotation in unit of arc degree, x'=x cos(a)-y sin(a);  y'=x sin(a)+ y cos (a)
		double galvoForwardLineDuty; ///<duty cycle in percent for the forward and backward lines,  = FwdLineSplLength/(LineSplLength)
		long progressCounter;///<stores the completed number of frames. Purpose is to force the SetupAcquisition to execute all hardware settings
		long galvoEnable;///<enable or disable y galvo movement
		long yChannelEnable;///<enable or disable the communication with y channel, NOT implemented yet.
		double pockelsPowerLevel[MAX_GG_POCKELS_CELL_COUNT];///<level of pockels power		
		double pockelsLineBlankingPercentage[MAX_GG_POCKELS_CELL_COUNT];///<percentage of line to keep pockels at power
		double verticalScanDirection;///<direction of the y galvo scan
		double fineOffset[2];///<fine adjustment of offset [X,Y]
		double fineFieldSizeScaleX;///<fine adjustment of field size X
		double fineFieldSizeScaleY;///<fine adjustment of field size Y
		double scanAreaAngle;///<angle for roi
		long dmaBufferCount;///<number of buffers for DMA
		long horizontalFlip;///<flip the image in the X direction
		long useReferenceForPockelsOutput;///<when pockels is enabled, if the max pockels voltage should be use to build the waveform instead of the set pockels power
		long channelPolarity[4];///<polarity to use for each channel
		long interleaveScan; ///<interleave scan of Galvo Galvo for Two-Way or Forward mode
	}VCMImgPty, *pVCMImgPty;


	enum
	{
		CLOSE_SHUTTER = 0,
		OPEN_SHUTTER = 1
	};

	enum
	{
		POL_NEG = 0,
		POL_POS = 1
	};

	class ThorLSMCam : public IActionReceiver
	{
	private:

		static bool _instanceFlag;
		static auto_ptr<ThorLSMCam> _single;

		const long MAX_PIXEL_X;
		const long MIN_PIXEL_X;
		const long MAX_PIXEL_Y;
		const long MIN_PIXEL_Y;
		const long DEFAULT_PIXEL_X;
		const long DEFAULT_PIXEL_Y;
		const long DEFAULT_FIELD_SIZE_X;
		const long MIN_ALIGNMENT;
		const long MAX_ALIGNMENT;
		const long DEFAULT_ALIGNMENT;
		const long MIN_INPUTRANGE;
		const long MAX_INPUTRANGE;
		const long DEFAULT_INPUTRANGE;
		const long MIN_CLOCKSOURCE;
		const long MAX_CLOCKSOURCE;
		const long DEFAULT_CLOCKSOURCE;
		const long MIN_EXTCLOCKRATE;
		const long MAX_EXTCLOCKRATE;
		const long DEFAULT_EXTCLOCKRATE;
		const long MIN_INTERNALCLOCKRATE;
		const long MAX_INTERNALCLOCKRATE;
		const long DEFAULT_INTERNALCLOCKRATE;
		const long MIN_AVERAGENUM;
		const long MAX_AVERAGENUM;
		const long DEFAULT_AVERAGENUM;
		const long MIN_SCANMODE;
		const long MAX_SCANMODE;		
		const long DEFAULT_SCANMODE;
		const long MIN_TRIGGER_MODE;
		const long MAX_TRIGGER_MODE;
		const long DEFAULT_TRIGGER_MODE;
		const long MIN_CHANNEL;
		const long MIN_TRIGGER_TIMEOUT;
		const long MAX_TRIGGER_TIMEOUT;
		const long MIN_ENABLE_FRAME_TRIGGER;
		const long MAX_ENABLE_FRAME_TRIGGER;
		const long MIN_AREAMODE;
		const long MAX_AREAMODE;
		const long MIN_Y_AMPLITUDE_SCALER;
		const long MAX_Y_AMPLITUDE_SCALER;
		const long DEFAULT_Y_AMPLITUDE_SCALER;
		const long MIN_FLYBACK_CYCLE;
		const long MAX_FLYBACK_CYCLE;
		const long DEFAULT_FLYBACK_CYCLE;
		const double MAX_FLYBACK_TIME;
		const long MIN_GALVO_ENABLE;
		const long MAX_GALVO_ENABLE;
		const long DEFAULT_GALVO_ENABLE;
		const double MAX_RASTERANGLE;
		const double MIN_RASTERANGLE;
		const double DEFAULT_RASTERANGLE;
		const double MAX_FORWARD_LINE_DUTY;
		const double MIN_FORWARD_LINE_DUTY;
		const double DEFAULT_FORWARD_LINE_DUTY;
		static const double MAX_DWELL_TIME;
		static const double DEFAULT_DWELL_TIME;
		static const double DWELL_TIME_STEP;
		const double GALVO_PARK_POSITION;
		const long GALVO_PADDING_SAMPLE;  //PAD ONE BOTH END OF FAST AXIS WAVEFORM TO REDUCE NOISE, LEAVE TRIGGER REARMING TIME, AND ENSURE LINEAR PART IN THE MIDDLE
		const long GALVO_MOVE_PATH_SLEEP;
		const double GALVO_LINE_PADDING_PERCENT;
		const long MIN_Y_CHANNEL_ENABLE;
		const long MAX_Y_CHANNEL_ENABLE;
		const long DEFAULT_Y_CHANNEL_ENABLE;
		const long MAX_POCKELS_LINE_BLANKING_PERCENTAGE;
		static const long READ_CHANNEL_COUNT = 2;
		const double MAX_SCANAREA_ANGLE;
		const double MIN_SCANAREA_ANGLE;
		const double DEFAULT_SCANAREA_ANGLE;
		static const long AO_CLOCK_LENGTH = 1024;
		const long MAX_TB_LINE_SCAN_TIME;
		const long MIN_TB_LINE_SCAN_TIME;
		const long MAX_TIMED_BASED_SCAN_PIXEL_Y;

		///<Get/Set params
		long _maxChannel;
		long _pixelX;
		long _pixelY;
		long _fieldSize;
		long _fieldSizeMin;
		long _fieldSizeMax;
		long _offsetX;
		long _offsetY;
		long _channel;
		long _alignmentForField;
		long _inputRangeChannel[4];
		long _clockSource;
		long _clockRateInternal;
		long _clockRateExternal;
		static long _clockRateNI;
		static long _clockRatePockels;
		static long _scanMode;
		long _averageMode;
		long _averageNum;
		long _triggerMode;
		long _frameCount;
		static long _triggerWaitTimeout;
		long _frameTriggerEnableWithHWTrig;
		double _yAmplitudeScaler;
		static long _areaMode;
		long _flybackCycle;
		long _galvoEnable; ///<for the galvo-galvo system, _galvoEnable == FALSE represents a line scan (including diagonal lines)
		double _fieldSizeCalibration;
		long _fieldSizeCalibrationAvailable;
		long _yChannelEnable;
		long _forceSettingsUpdate;
		double _scanAreaAngle;
		long _droppedFramesCnt;
		long _rawFlybackCycle;
		bool _minimizeFlybackCycles;
		double _dwellTime; ///<time galvo remains at each sample
		static double _minDwellTime; ///<minimum dwell time based on number of channels
		double _rasterAngle; ///<angle of scan rotation in unit of arc degree, x'=x cos(a)-y sin(a);  y'=x sin(a)+ y cos (a)
		double _galvoForwardLineDuty; ///<duty cycle in percent for the forward and backward lines,  = FwdLineSplLength/(LineSplLength)		
		static long _dMABufferCount;
		long _lineFlybackLength;
		static double _pockelsPhaseDelayUS;
		long _useReferenceForPockelsOutput;///<when pockels is enabled, if the max pockels voltage should be use to build the waveform instead of the set pockels power
		long _pockelsTurnAroundBlank;///<blank pockels at turn around of galvo
		long _interleaveScan; ///<interleave scan of Galvo Galvo for Two-Way or Forward mode
		static string _analogChannels[MAX_ANALOG_CHAN_COUNT]; ///<analog channels for galvo mirror pairs or other applications
		long _analogXYmode[2][2]; ///<indicator for X, Y channels are in ascending[1] or decending[-1] order, first index: path, second index: X[0], Y[1]
		double _analogFeedbackRatio[2][2]; ///<voltage ratio for X, Y feedback channels, first index: path, second index: X[0], Y[1]
		static string _devID; ///<NI device ID, default as Dev2
		double _ggSuperUserMode; ///<Flag to enable or disable Superuser mode, it disables all the safety checks for Dwell time and Field Size
		double _timebasedLSTimeMS; ///<Number of miliseconds used for time based line scan

		///<daq members
		static double _avgCount;
		static long _stopReading; ///<use to let the read thread know it can stop reading
		unsigned long _totalChannelsForAllBoards;///<total number of channel chosen for all boards
		long _pixelClockLength;
		long _dataMapMode;///<mode to use for mapping the raw digitizer data
		static long _recordCount;
		static U32 _controlFlags;
		static unsigned long _recsPerBuffer;
		static unsigned long _channelMode[MAX_BOARD_NUM]; ///<channel A or Channel B;
		static unsigned long _numChannel[MAX_BOARD_NUM]; ///<number of channel chosen on each board
		static long _sizePerBuffer[MAX_BOARD_NUM];
		static U16 *_pData[MAX_BOARD_NUM][MAX_DMABUFNUM];
		static unsigned short *_pFrmDllBuffer[MAX_DMABUFNUM];
		//static U16 _datamap[65536];
		//static S16 _datamap[65536];
		static U16* _datamap[4];
		static U16 _datamapPositiveSigned[65536];
		static U16 _datamapNegativeSigned[65536];
		static U16 _datamapPositiveUnsigned[65536];
		static U16 _datamapNegativeUnsigned[65536];
		static U16 _datamapIndependent[65536];
		static long _indexOfLastCompletedFrame;///<counter for tracking the sequence index of the current frame
		static long _indexOfLastCopiedFrame;///<counter for tracking the index of the frames being copied out to the user
		static long *_remap_index;
		static long _remap_size;
		static long _loadedShiftPlusAlignment;
		static long _rawNIBufferSize;
		static float64 * _pRawNIBuffer;
		static float64 * _pRawNILineBuffer;
		static float64 * _pRawNIPartialBuffer;
		static long _1stSet_CMA_Frame;
		static unsigned long _dataPerLine;
		static unsigned long _dataPerLineToRead;
		static unsigned long _lineSampleLength;
		static unsigned long _samplesRead;
		static unsigned long _linesRead;
		static unsigned long _numCallbacks;
		static long _partialFrameReady;
		static long _readEntireFrame;

		///<Handles
		static HANDLE _hStopAcquisition; ///<event to stop frame acquisition
		static HANDLE _hFrmBufReady; ///<Signals if the frame data buffer is ready to copy
		static HANDLE _hFrmBufHandle; ///<Mutex to claim the exclusive access to the buffer
		static HANDLE _hNIRawFrmReadyHandle; ///<Mutex to claim the exclusive access to the Raw buffer
		static HANDLE _hNIRawFrmCopiedHandle; ///<Mutex to claim the exclusive access to the Raw buffer
		static HANDLE _hPartialFrmBufHandle; ///<Mutex to claim the exclusive access to the partial buffer
		static HANDLE _hThreadStopped; ///<Signals if the acquisition thread has stopped
		static HANDLE _hReadThreadStarted; ///<Signals if the thread that reads the data from the ADC can start
		static HANDLE _hStatusError; ///<NI error event
		static HANDLE _hThread; ///<thread handle
		static HANDLE _hStatusHandle;///<status handle for signaling external user
		static HANDLE _hHardwareTriggerInEvent;		
		static HANDLE _hSpecialDigitalLineReadyHandle; ///<used for safety when creating a digital line task (i.e. bufferReadyLine and captureActiveLine)

		///<general members
		VCMImgPty _ImgPty;
		VCMImgPty _ImgPty_Pre;
		static VCMImgPty _imgPtyDll;
		static const double _crsFrequency;///<the rounded frequency for the resonance scanner
		static wchar_t _errMsg[_MAX_PATH];
		static IBehavior* _behaviorPtr; ///<pointer to switch between different scan mode
		static std::unique_ptr<BlockRingBuffer> _wBuffer[SignalType::SIGNALTYPE_LAST];///<waveform buffers
		std::unique_ptr<BehaviorFactory> _behaviorFac; ///<pointer to LSM behavior factory which contains all behavior items
		long _channelPolarity[4];///<store the polarity to use for each channel
		long _pockelsParkAtMinimum; ///<Flag for user to choose if pockels will park at the minimum calibration value
		long _galvoParkAtStart; ///<Flag for use to choose if galvo will park at start position
		long _fileSettingsLoaded; ///<Flag set when the settings from the settings file have been read
		long _imageActiveLoadMS; ///<user defined image load callback time
		long _imageActiveLoadCount; ///<user defined block ring buffer size
		static int _digiLineSelect; ///<imaging line count:[2] line/frame [3] line/frame/pockelsDig triggers.
		static int _pockelsSelect;///<user selected pockels line counts
		static std::unique_ptr<BoardInfoNI> _boardInfoNI;///<NI boards' info
		double _galvoRetraceTime;///<galvo trace time in micro-second unit
		static double _maxGalvoOpticalAngle;///<maximum allowed optical angle
		static double _minSignalInputVoltage;
		static double _maxSignalInputVoltage;
		static long _timeBasedLineScanEnabled; ///< Flag to enable or disable time based line scan
		static long _isLiveScan; ///<Flag used to indicate if the current acquisition is a live scan in Capture Setup

		///<buffer members
		static CRITICAL_SECTION _remapSection; ///<critical section control for process buffer and setupAcquisition
		CRITICAL_SECTION _hThreadSection; ///<critical section control for acquisition thread
		static long _bufCompleteID; ///<index of completed buffer ready for process
		static long _index; ///<index of image buffer
		static long _line_start_indexF[MAX_BOARD_NUM]; ///<index of on-board buffer, forward scan line start index
		static long _line_start_indexB[MAX_BOARD_NUM]; ///<index of on-board buffer, backward scan line start index
		static long _offset; ///<offset data count in interleave scan
		static long _indexLineFactor; ///<two-way scan: 2, one-way scan: 1
		static long _bufferLarge; ///<acquire line by line when over the memory limit 16MB
		static int	_acquireStatus;///<status to reflect current camera acquisition
		size_t _lastBufferSize; ///<frame buffer size of previous setup
		long _lastDMABufferCount; ///<frame DMA buffer count of previous setup
		long _lastSizePerBuffer[MAX_BOARD_NUM]; ///<alazar buffer sizes of previous setup

		///<NI DAQmx task handles
		static TaskHandle _taskHandleAO1; ///<y control
		static TaskHandle _taskHandleDO1; ///<frame trigger output
		TaskHandle _taskHandleDO2; ///<shutter output
		static TaskHandle _taskHandleDI1; ///<frame trigger input
		static TaskHandle _taskHandleAI0; ///<NI Analog Input
		static TaskHandle _taskHandleCO0; ///<counter, will be called in the FramAsnc thread to start frame/line triger 
		static TaskHandle _taskHandleCO1; ///<counter, will start with Counter 0 as the clock for galvo-galvo waveform
		static TaskHandle _taskHandleCOSampling; ///<counter, will be called in the FramAsnc sampling
		static TaskHandle _taskHandleCOFreq;
		static TaskHandle _taskHandleDO3; ///<configurable output
		static TaskHandle _taskHandleAOPockels; ///<pockels control
		static TaskHandle _taskHandleCO2;///<counter, will start when line trigger is received.
		TaskHandle _taskHandleAIPockels[MAX_GG_POCKELS_CELL_COUNT]; ///<pockels power input

		///<Bleach members
		static long _waveformTaskStatus; ///<task status of the waveform mode
		static long _finishedCycleCnt; ///<current finished cycle number for waveform mode
		static long _triggeredCycleCnt; ///<HW triggered cycle number for waveform mode, increment at start of current cycle
		static long _cycleDoneLength; ///<data length before cycle complete
		static long _activeLoadCount;///<user defined active load count, size unit x 100 each
		static long _dLengthPerAOCallback[SignalType::SIGNALTYPE_LAST]; ///<data count for analog out everyN callback
		static uint64_t _totalLength[SignalType::SIGNALTYPE_LAST]; ///<total data length for signals, including all frames consisting of Line BW and Frm BW
		static uint64_t _currentIndex[(int)(SignalType::SIGNALTYPE_LAST)]; ///<current copied index in waveform
		static long _precaptureStatus; ///<pre-capture status for waveform active load
		static WaveformGenParams _waveGenParams;
		static GGalvoWaveformParams _gGalvoWaveParams;
		static string _pockelDigOut; ///<digital output line of pockels cell
		static string _waveformCompleteOut; ///<digital output line of waveform complete signal
		static string _bleachCycleOut; ///<digital output line of waveform cycle envelope
		static string _bleachIterationOut; ///<digital output line of waveform iteration envelope
		static string _bleachPatternOut; ///<digital output line of waveform pattern trigger
		static string _bleachPatternCompleteOut; ///<digital output line of waveform pattern complete for SLM trigger	
		static string _bleachActiveOut; ///<digital output line of bleach active	
		static string _bleachEpochOut; ///<digital output line of bleach epoch envelope	
		static string _bleachCycleInverse; ///<complementary digital output line of bleach cycle envelope
		static string _bleachShutterLine; ///<bleach shutter line (PFI)
		static long _bleachShutterIdle[2]; ///<bleach shutter pre- and post- idle time in msec
		static std::wstring _waveformPathName; ///<waveform file's path and name
		static int _digiBleachSelect; ///<bleaching digital lines, in order: [1]dummy,[2]pockelsDig,[4]complete,[8]cycle,[16]iteration,[32]pattern,[64]patternComplete

		///<NI members
		static long _numNIDAQ; ///<number of NI DAQ boards
		static long _captureActiveOutputInvert; ///<capture active output invert signal. Default high while imaging
		static long _pockelsReferenceRequirementsMet;///<flag set to true when pockels1 is in a 6363 board and the pockelsReferenceLine is on the same board
		static map<long, long> _bufferOrder;///<map of dma buffer index and completed frame index
		float64 *_pGalvoStartPos; ///<galvo waveform with start position 
		static float64 *_pGalvoWaveformXYP; ///<galvo waveform on the fly
		static float64 *_pPockelsWaveform;
		static uInt8* _frameTrigger;
		float64 _sampleRate;
		double _field2Theta; // GUI field (0 ~255) to scanner mechanical scan p-p angle in degree  
		double _theta2Volts; // Galvo mechnical scan p-p angle to waveform amplitude p-p in volts
		double _crsFrequencyHighPrecision;///<each resonance scanner has a slightly different frequency. This is an exposed variable for entering the manufacturers spec value 
		long _galvoFWPhaseShift; ///<forward backward alignment in pix
		static long _progressCounter;///<stores the completed number of frames. Purpose is to force the SetupAcquisition to execute all hardware settings
		double _verticalScanDirection;///<determines the scan direction of the y galvo (+1.0 or -1.0)
		double _fineOffset[2];///<fine adjustment of offset [X,Y]
		double _fineOffset2[2];///<fine adjustment of offset [X,Y], applied to OTM
		double _fineFieldSizeScaleX;///<fine adjustment of field size X
		double _fineFieldSizeScaleY;///<fine adjustment of field size Y
		long _centerWithOffsets;///<apply offsets when center scanmode
		long _oneXFieldSize;///<field size that matches the 1X zoom criteria
		long _horizontalFlip;///<flips the image in the X direction
		double _minGalvoFreqHz[2];///<slowest galvo's resonance frequency in Hz, dual-path support

		double _highResOffset[2];///<high resolution fine offset for [X,Y], applied to OTM
		double _highResOffset2[2];///<high resolution fine offset for [X2,Y2], applied to OTM
		double _highResOffsetMinMax[2];///<high resolution fine offset range [Min,Max], applied to OTM

		///<Arrays for file path and alignment calibration
		static long _shiftArray[256]; ///<backward forward alignment value
		static long _pockelsEnableIntegrated;
		static long _pockelsEnable[MAX_GG_POCKELS_CELL_COUNT];
		double _pockelsLineBlankingPercentage[MAX_GG_POCKELS_CELL_COUNT];
		double _pockelsMaxVoltage[MAX_GG_POCKELS_CELL_COUNT];
		double _pockelsMinVoltage[MAX_GG_POCKELS_CELL_COUNT];
		double _pockelsPowerLevel[MAX_GG_POCKELS_CELL_COUNT];
		long _pockelsResponseType[MAX_GG_POCKELS_CELL_COUNT]; ///<power fit method:[0]sinusoidal [1]linear
		float64  _pockelsReadArray[MAX_GG_POCKELS_CELL_COUNT][POCKELS_VOLTAGE_STEPS];
		double _pockelsScanVoltageStart[MAX_GG_POCKELS_CELL_COUNT];
		double _pockelsScanVoltageStop[MAX_GG_POCKELS_CELL_COUNT];
		double _pockelsVoltageSlopeThreshold;

		///<Signal lines or string members
		static string _pockelsLine[MAX_GG_POCKELS_CELL_COUNT];
		string _pockelsPowerInputLine[MAX_GG_POCKELS_CELL_COUNT];
		static string _galvoLinesInput;
		static string _galvoLinesOutput;
		static string _galvoAndPockelsLinesOutput;
		static string _controllerInternalOutput0;
		static string _controllerInternalOutput1;
		static string _controllerInternalOutput2;
		static string _controllerInternalOutput3;
		static string _controllerOutputLine0;
		static string _controllerOutputLine1;
		static string _controllerOutputLine2;
		static string _controllerOutputLine3;
		static string _frameTriggerLineIn;
		string _frameTriggerLineInOut;
		string _frameTriggerChangeDetection1;
		string _frameTriggerChangeDetection2;
		static string _startTriggerLine;
		static string _pockelsTriggerIn;
		static string _digitalTriggerLines;
		static string _pockelsReferenceLine;///<analog voltage input line to use for pockels reference
		static string _captureActiveOutput; ///<capture active output
		static string _frameBufferReadyOutput;///<buffer ready output
		string _shutterLine;
		string _readLine[3]; ///<analog data input line, ai0, ai1 or both
		string _clockExportLine; ///<export clock to PFI5
		long ChFlag;

	public:
		static HMODULE hDLLInstance;

		///<Public functions
		~ThorLSMCam();
		static ThorLSMCam* getInstance();
		void SetStatusHandle(HANDLE handle);

		///<function implementation
		long FindCameras(long &cameraCount); ///<Search system to find digitizers and daq board
		long SelectCamera(const long camera);///<initilized digitizer;
		long TeardownCamera(); ///<close handles for each boards
		long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault); ///<get the information for each parameter
		long SetParam(const long paramID, const double param);///<set parameter value
		long GetParam(const long paramID, double &param);///<get the parameter value
		long PreflightAcquisition(char * pDataBuffer);///<Setup for , should be called before each experiment, or whenever trigger mode has been changed
		long SetupAcquisition(char * pDataBuffer);///<Setup between frames, only used for software free run mode
		long StartAcquisition(char * pDataBuffer);///<Start a experiment,
		long StatusAcquisition(long &status);///<Status of a frame 
		long StatusAcquisitionEx(long &status,long &indexOfLastCompletedFrame);///<Status of a frame
		long CopyAcquisition(char * pDataBuffer, void* frameInfo);
		long PostflightAcquisition(char * pDataBuffer);
		long GetLastErrorMsg(wchar_t * msg, long size);
		long SetParamString(const long paramID, wchar_t * str);
		long GetParamString(const long paramID, wchar_t * str, long size);
		long GetParamBuffer(const long paramID, char * pBuffer, long size);
		long SetParamBuffer(const long paramID, char * pBuffer, long size);

		///<Action receiver functions
		long SetAction(ActionType actionType);
		long SetActionWithParam(ActionType actionType, long paramVal);
		long GetActionResult(ActionType actionType, long& paramVal);
		long GetActionResult(ActionType actionType, char* pDataBuffer);

	private:///<Private functions

		ThorLSMCam();
		static long ActiveBehavior();
		static long WaitForHardwareTrigger(long timeoutMilliS, long showTimeoutMessageBox);
		long AlignDataLoadFile();
		long CaptureCreateThread(void);
		long CheckOpticalAngle(long fieldSize, double dwellTime, long pixelX);
		long SetupReadTask(void);
		void CloseLSMCam();
		void SignalClose();
		void CloseThread();
		void CreateWaveBuffers();
		void ConnectWaveBufferCallbacks();
		void FillWaveBuffers();
		long IsCorrectFPGA(HANDLE boardHandle);
		void SetupDataMap();
		long SetWaveform(VCMImgPty *pImgPty);
		void StopHardwareWaits();
		void UpdateWaveformGenParams();
		void SetAreaMode();

		///<Daq functions
		long SetBdDMA(void);
		static UINT StartFrmAsync(void *pData);
		static UINT ReadDataAsync(ThorLSMCam *);

		///<Generic functions
		long InitialImageProperties();
		long PreflightImageProperties();
		long SetupCheck();
		long SetupBoards();
		void PersistImageProperties();
		long SetupProtocol();
		long StartProtocol();
		long StatusProtocol(long &status);
		long CopyProtocol(char *pDataBuffer);
		long PostflightProtocol(long parkAtParking);	//[1]: park at parking

		///<Task Master functions: finite tasks with contineous clock.
		long SetupTaskMasterClock(void);
		static long SetupTaskMasterGalvo(void);
		static long SetupTaskMasterPockel(void);
		static long SetupTaskMasterDigital(void);
		long BuildTaskMaterDigital(void);
		void ForceUpdateProperties(void);
		static long SetFrameInTriggerableTask(TaskHandle taskHandle,long armStart);
		//static long SetupCompleteWaveformTrigger(long startNow);
		static long SyncCustomWaveformOnOff(bool32 start);
		static long TryBuildTaskMaster(void);
		static long TryWriteTaskMasterGalvoWaveform(long checkStop);
		static long TryWriteTaskMasterPockelWaveform(long checkStop);
		static long TryWriteTaskMasterLineWaveform(long checkStop);
		static long WaveformModeFinished(void);
		static int32 CVICALLBACK CycleDoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);
		static int32 CVICALLBACK EveryNDigitalOutCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
		static int32 CVICALLBACK EveryNGalvoOutCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
		static int32 CVICALLBACK EveryNPockelOutCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
		static int32 CVICALLBACK HWTriggerCallback(TaskHandle taskHandle, int32 signalID, void *callbackData);

		///<Buffer functions
		static long GetFrameIndexFromBuffer(U32 bufferIndex);
		static long ProcessBufferFunc1(void);
		static long ProcessBufferFunc2(void);
		static long ProcessBufferFunc3(void);
		static long ProcessBufferNoAve(void);
		static long ProcessForwardLine(long &index, long *line_start_indexF);
		static long ProcessBackwardLine(long &index, long *line_start_indexB);
		static long ProcessLineOffset(long &index, long offset);
		static long ProcessBufferFrmCMA(void);
		static long ProcessForwardLineFrmCMA(long &index, long *line_start_indexF);
		static long ProcessBackwardLineFrmCMA(long &index, long *line_start_indexB);
		//static long ProcessBufferFrmSMA(long bufID, unsigned short *pFrmData, U32 transferLength);

		///<NI actions
		long FindPockelsMinMax(long index);
		long MoveGalvoToCenter(void);
		long MoveGalvoToParkPosition(long parkAtParking = FALSE);
		long MoveGalvoToPosition(double* path, int pathID);
		long MoveGalvoToStart(void);
		long MovePockelsToParkPosition(void);		
		long MovePockelsToPowerLevel(long index);
		static long SetFrameBufferReadyOutput();
		static long SetCaptureActiveOutput(long startOrStop);
		long ThorVCMDigitalShutterPosition(int pos);
		static void ThorCloseNITasks(void);

		///<Clock Master functions: finite clock with contineous tasks.
		static int32 CVICALLBACK EveryNClockMasterLineCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
		static int32 CVICALLBACK EveryNClockMasterGalvoCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
		static int32 CVICALLBACK EveryNClockMasterPockelCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
		long SetupClockMasterDigital(void);
		long SetupClockMasterGalvo(void);
		long SetupClockMasterPockel(void);
		long SetupClockMasterClock(void);
		static long SetupFrameTriggerInput(void);		
		static long TryWriteClockMasterGalvoWaveform(void);
		static long TryWriteClockMasterPockelWaveform(void);
		static long TryWriteClockMasterLineWaveform(void);

		///<Flyback Functions
		double getFlybackTime(long flybackCycle);
		double getFlybackTime();
		long getMinFlybackCycle();
		long getFlybackCycle();
		void setFlybackCycle(long flybackCycle);

		///<Input range functions
		long SetRngA(long RngA);
		long SetRngB(long RngB);
		long SetRngC(long RngC);
		long SetRngD(long RngD);
		long SetXRemap(HANDLE h_bd, long eff_gsizoom);
		long SetXRemapGalvo(HANDLE h_bd, long placeholder);
		long Set2WayScanRemap(HANDLE h_bd, long shift, unsigned xpixel, long fieldX, long offsetX);
		long Set1WayScanRemap(HANDLE h_bd, long shift, unsigned xpixel, long fieldX, long offsetX);
		long Set1WayScanRemapGalvo(HANDLE h_bd, long shift, unsigned xpixel, long fieldX, long offsetX);
		long Set1WayScanRemapBack(HANDLE h_bd, long shift, unsigned xpixel, long fieldX, long offsetX);
		long DownloadDataSkip(HANDLE h_bd, long* indexArray, unsigned long ArraySize);

	};

#ifdef __cplusplus
}
#endif
