#ifndef CONST_H
#include "const.h"
#endif

#ifndef ACQ_H
#include "acq.h"
#endif

#include "thordaqcmd.h"
#include "thorDAQAPI.h"
#include "FrmCirBuf.h"
#include "dataprocessor.h"
#include "MultiPlaneDataProcessor.h"
#include "ThorDAQDMABuffer.h"
#include "..\ThorDAQIOXML.h"
#include "ThorDAQZAPI.h"
#include "ThorDAQZ.h"

// Convert function name macro to wide string for ThordaqErrChk
#define WIDE2(x) L##x
#define WIDE1(x) WIDE2(x)
#define WFUNCTION WIDE1(__FUNCTION__)

#define ThordaqErrChk(fnName,fnCall) if ((error=(fnCall)) > STATUS_SUCCESSFUL){ StringCbPrintfW(thordaqLogMessage,MSG_SIZE,WFUNCTION L": %s failed. Error code %d",fnName, error); LogMessage(thordaqLogMessage,ERROR_EVENT); }

extern std::unique_ptr<ImageWaveformBuilderDLL> ImageWaveformBuilder;

class ScanStruct
{
public:
	double forward_lines;
	double backward_lines;
	double overall_lines;
	double average_lines_num;
	long scanMode;
	double dacRate;
	bool blockBackwardLine;
	ScanStruct()
	{
		this->forward_lines = 0;
		this->average_lines_num = 0;
		this->backward_lines = 0;
		this->overall_lines = forward_lines + backward_lines;
		this->scanMode = 0;
		this->dacRate = 0;
		this->blockBackwardLine = false;
	}
	ScanStruct(double forward_lines, double backward_lines, double average_lines_num)
	{
		this->forward_lines = forward_lines;
		this->average_lines_num = average_lines_num;
		this->backward_lines = backward_lines;
		this->overall_lines = forward_lines + backward_lines;
		this->scanMode = 0;
		this->dacRate = 0;
		this->blockBackwardLine = false;
	}
};

class ScanLineStruct
{
public:
	UINT32 samples_idle;
	UINT32 samples_scan;
	UINT32 samples_back;
	ScanLineStruct()
	{
		samples_idle = 0;
		samples_scan = 0;
		samples_back = 0;
	}
};

struct PockelsImagePowerRampStruct
{
	double startPowerLevel;
	double endPowerLevel;
};

struct GalvoStruct
{
	SCAN_DIRECTION scan_direction;
	double amplitude;
	double park;
	double offset;
};


enum ScanMode
{
	TWO_WAY_SCAN = 0,
	FORWARD_SCAN = 1,
	BACKWARD_SCAN = 2,
	CENTER = 3,
	BLEACHING_SCAN = 4
};

// This class is exported from the ThorDAQGalvoGalvo.dll
class CThorDAQGalvoGalvo :ICamera {
private:

	enum class GG_AO : ULONG
	{
		GG_X = 0,
		GG_Y,
		Pockels0,
		Pockels1,
		Pockels2,
		Pockels3
	};


	unsigned int _deviceNum;                                      // <The total number of connected boards
	long _oneXFieldSize;///<field size that matches the 1X zoom criteria
	long _fieldSizeCalibrationAvailable;
	long _droppedFramesCnt; // <The number of dropped frames in the acquisition
	long _forceSettingsUpdate; // Force refresh DAQ configuration
	long _frameTriggerEnableWithHWTrig;
	long _pockelsParkAtMinimum; ///<Flag for user to choose if pockels will park at the minimum calibration value
	long _pockelsTurnAroundBlank; ///<Flag to enable pockels blanking on the turnaround
	long _pockelsFlybackBlank; ///<Flag to enable pockels blanking on the flyback to the start of the image
	long _sumPulsesPerPixel; ///<Flag to sum or average the pulses per each pixel, set by the dwell time
	long _useExternalBoxFrequency3P; ///Flag used for testing purposes to set the external frequency to a passed in value, instead of the value read
	long _fieldSizeMin; ///<minimum field size
	long _fieldSizeMax; ///<maximum field size
	long _scannerType; ///<scanner type
	double _maxScannerSampleRate;//maximim scanner sample rate
	//double _pockelsPowerLevel[MAX_POCKELS_CELL_COUNT];
	double _pockelsScanVoltageStart[MAX_POCKELS_CELL_COUNT];
	double _pockelsScanVoltageStop[MAX_POCKELS_CELL_COUNT];
	double _fieldSizeCalibration;
	double _dwellTimeStep;
	double _displayedDwellTime;
	double _minDwellTime;
	long _pockelsEnable[MAX_POCKELS_CELL_COUNT];
	TaskHandle _taskHandleAIPockels[MAX_POCKELS_CELL_COUNT]; ///<pockels power input
	string  _pockelsPowerInputLine[MAX_POCKELS_CELL_COUNT];///<NI connection string for pockel(s) analog response		
	double  _pockelsVoltageSlopeThreshold;///<slope threshold for detecting min/max of pockels output
	float64 _pockelsReadArray[MAX_POCKELS_CELL_COUNT][POCKELS_VOLTAGE_STEPS];
	long    _pockelsResponseType[MAX_POCKELS_CELL_COUNT];///<power fit method:[0]sinusoidal [1]linear
	char*	_pPockelsMask[MAX_POCKELS_CELL_COUNT];///<pointer to a mask buffer for the pockels masking
	size_t	_pockelsMaskSize[MAX_POCKELS_CELL_COUNT];///<size of the mask to be used for pockels masking
	long    _pockelsMaskChanged;///<flag set to TRUE when the pockels mask is updated
	long	_FIRFilterSelectedSettingChannel;
	long	_FIRFilterSelectedIndex;
	long	_FIRFilterSelectedTapIndex;
	//galvo setting related parameters
	double _field2Theta; // GUI field (0 ~255) to scanner mechanical scan p-p angle in degree  
	double _theta2Volts; // Galvo mechnical scan p-p angle to waveformBuffer amplitude p-p in volts
	UCHAR* _pTempBuf; ///Intermediate buffer for CopyAcquisition
	short	_preDcOffset[MAX_CHANNEL_COUNT]; // Pre-FIR DC-Offset values for channels 1 to MAX_CHANNEL_COUNT 
	long	_ratio;
	long	_ggSuperUserMode;
	long _flybackCycles;
	long _galvoParkAtStart;
	USHORT* _datamap[MAX_CHANNEL_COUNT];///<datamap array
	USHORT  _datamapPositiveSigned[65536];///<datamap for positive voltages
	USHORT  _datamapNegativeSigned[65536];///<datamap for negative voltages
	USHORT  _datamapPositiveUnsigned[65536];///<datamap for positive voltages
	USHORT  _datamapNegativeUnsigned[65536];///<datamap for negative voltages
	USHORT  _datamapIndependent[65536];///<datamap that folds the positive and negative
	std::map<UINT8, long> _digitalIOSelection;
	long _captureActiveInvert;
	std::vector<string> _thorDAQStimDigitalLinesConfig;
	std::vector<string> _thorDAQImagingDigitalLinesConfig;
	bool _currentlyImaging;
	bool _currentlyStimulating;
	bool _stopStimulating;
	long _enableGalvoXPark;
	long _useFastOneway;
	long _limitGalvoSpeed;
	bool _ddsClockEnable;
	double _ddsClockPhase0;
	double _ddsClockPhase1;
	double _ddsClockAmplitude0;
	double _ddsClockAmplitude1;	

	BOARD_INFO_STRUCT _boardInfo;
	LOW_FREQ_TRIG_BOARD_INFO_STRUCT _lowFreqTrigBoardInfo;
	long _multiplaneBlankLines; ///<number of lines to make blank at the end of each plane to be able to easily visually distinguish each plane
	long _multiplaneBlankLinesInLiveModeOnly; ///<flag to allow or not allow blank lines in non live mode
	std::map<GG_AO, long> _imagingActiveAOSelection;
	std::map<GG_AO, long> _stimActiveAOSelection;
	std::map<AO, long> _thordaqAOSelection;
	bool _secondaryGGAvailable; ///<flag to know if the second GG path is configured
	THORDAQ_TRIGGER_MODES _dacTriggerMode;
	long _rggMode;
	bool _updatingParam;
	double _maxAngularVelocityRadPerSec; ///<Maximum allowed angular velocity for the galvos in Rad/Sec
	double _maxAngularAccelerationRadPerSecSq; ///<Maximum allowed angular acceleration for the galvos in Rad/Sec^2
	bool _isDynamicWaveformLoadingStim;
	THORDAQ_BOB_TYPE _tdqBOBType;
private:

	//private static varibles
	static HANDLE _hThreadStopped;                                      ///<Signals if DAQ is stopped
	static HANDLE _hStopAcquisition;									///<event to stop frame acquisition
	static HANDLE _hTriggerTimeout;										///<Signals if HW trigger timed out
	static HANDLE _hHardwareTriggerInEvent;
	static HANDLE _hExperimentThread;									///<Main Acquisition thread
	static ThorDAQGGWaveformParams _gWaveformParams;
	static std::wstring _stimWaveformPath; ///<waveform file's path and name
	static bool _instanceFlag;
	static shared_ptr<CThorDAQGalvoGalvo> _single;	// Singleton Instance
	static ImgAcqPty _imgAcqPty;                // Static Image Acquisition Properties
	static ImgAcqPty _imgAcqPty_Pre;            // Static Image Acquisition previous Properties
	static IMAGING_CONFIGURATION_STRUCT _daqAcqCfg;            /// Global daq acquisition configuration
	static DAC_FREERUN_WAVEFORM_CONFIG _daqStimCfg;      /// Global daq Stim configuration
	static size_t _dacWaveSamples;
	static long _DAQDeviceIndex;                 // <The Index of connected ThorDAQ Device
	const wchar_t* _pDetectorName;              // <Detector Name
	static wchar_t _errMsg[_MAX_PATH];                   // error message written to the log file
	static long _triggerWaitTimeout;///<amount of time to wait for hardware trigger
	static FrameCirBuffer* _pFrmBuffer;
	static UCHAR* _pHistoryBuf;
	static UCHAR* _pHistoryProgressiveBuf;
	static UCHAR* _pProgressiveBuf[MAX_CHANNEL_COUNT];
	static UCHAR* _BufferContiguousArray[MAX_CHANNEL_COUNT];
	static IDataProcessor* _pDataProcessor;
	static double _frameRate;
	static std::atomic<long long> _index_of_last_written_frame;						///<counter for tracking the sequence index of the current frame
	static std::atomic<long long> _index_of_last_read_frame;
	static long _imageStatus;
	static long _precaptureStatus; ///<pre-capture status for waveform active load TODO: see if it should be static
	static long _bleachStatus; ///<pre-capture status for waveform active load TODO: see if it should be static
	static long _shiftArray[256]; ///<backward forward alignment value	

	static long _dLengthPerDACCallback[SignalType::SIGNALTYPE_LAST]; ///<data count for analog out everyN callback
	static UINT64 _totalLength[SignalType::SIGNALTYPE_LAST]; ///<total data length for signals, including all frames consisting of Line BW and Frm BW
	static UINT64 _currentIndex[(int)(SignalType::SIGNALTYPE_LAST)]; ///<current copied index in waveform
	static int _digiBleachSelect; //selection of digital lines
	static UINT64 _stimCompletedCycles; //counter of completed bleaching/stim cycles
	static UINT64 _stimPreLoadedCycles; //counter of completed bleaching/stim cycles
	static bool _dacWavepformPlaybackComplete;
	static bool _dacWavepformPlaybackStarted;
	static std::atomic<bool> _dacConfiguringWaveforms;
	static std::atomic<bool> _dacLoadedLastWaveformSection;
	static std::atomic<bool> _dacPreloadingWaveforms;
	static std::atomic<bool> _dacPrepareRetrigger;
	//static std::atomic<bool> _dacPreloadNextWaveformSection;
	static std::atomic<bool> _dacWaveformPreloaded;
	static std::atomic<bool> _dacRunning;
	static long _stimActiveLoadCount;///<user defined active load count, size unit x 100 each
	static std::map<UINT, USHORT> _stimParkPositions;
	static std::map<UINT, DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT> _dacCtrlDynamicLoad;
	static wchar_t thordaqLogMessage[MSG_SIZE];
	static ScanStruct  _scan_info;
	static ScanLineStruct _scanLine;
	static UINT64 _powerRampCurrentIndex;
	static UINT64 _fastZCurrentIndex;
	static std::vector<PockelsImagePowerRampStruct> _pockelsImagePowerRampVector[MAX_POCKELS_CELL_COUNT];
	static std::vector<double> _pockelsResponsePowerLevels[MAX_POCKELS_CELL_COUNT];
	static ThorDAQZWaveformParams _fastZWaveformParams;
	static long _useBuiltZWaveform;

	//private static methods	
	static void TerminateNIDAQTask(TaskHandle& handle);
	static UINT StartFrameAcqProc(LPVOID instance);
	static UINT StimProcess(LPVOID instance);
	static long SetFrameBufferReadyOutput();///<Digital line that goes active after a frame has been copied to the output buffer
	static long DACPrepareForRetrigger();
	static long DACPreloadNextWaveformSection();
	static void CTHORDAQCALLBACK DACApproachingNSamplesCallback(UINT8 dacChannel, UINT32 numberSample, THORDAQ_STATUS status, void* callbackData);
	static void CTHORDAQCALLBACK DACApproachingLoadedWaveformEndCallback(UINT8 dacChannel, THORDAQ_STATUS status, void* callbackData);
	static void CTHORDAQCALLBACK DACycleDoneCallback(UINT8 dacChannel, THORDAQ_STATUS status, void* callbackData);
	static void CTHORDAQCALLBACK DACWavefomPlaybackCompleteCallback(THORDAQ_STATUS status, void* callbackData);
	static void CTHORDAQCALLBACK DACWavefomPlaybackStartedCallback(THORDAQ_STATUS status, void* callbackData);
	static void CTHORDAQCALLBACK DACReadyForNextImageWaveformsCallback(THORDAQ_STATUS status, void* callbackData);
public:
	~CThorDAQGalvoGalvo();
	static CThorDAQGalvoGalvo* GetInstance();
	long FindCameras(long& cameraCount); ///<Search system to find digitizers and daq board
	long SelectCamera(const long camera);///<initilized digitizer;
	long TeardownCamera(); ///<close handles for each boards
	long GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault); ///<get the information for each parameter
	long SetParam(const long paramID, const double param);///<set parameter value
	long GetParam(const long paramID, double& param);///<get the parameter value
	long PreflightAcquisition(char* pDataBuffer);///<Setup for , should be called before each experiment, or whenever trigger mode has been changed
	long SetupAcquisition(char* pDataBuffer);///<Setup between frames, only used for software free run mode
	long StartAcquisition(char* pDataBuffer);///<Start a experiment,
	long StatusAcquisition(long& status);///<Status of a frame 
	long StatusAcquisitionEx(long& status, long& indexOfLastCompletedFrame);///<Status of a frame
	long CopyAcquisition(char* pDataBuffer, void* frameInfo);
	long PostflightAcquisition(char* pDataBuffer);
	long GetLastErrorMsg(wchar_t* msg, long size);
	long SetParamString(const long paramID, wchar_t* str);
	long GetParamString(const long paramID, wchar_t* str, long size);
	long GetParamBuffer(const long paramID, char* pBuffer, long size);
	long SetParamBuffer(const long paramID, char* pBuffer, long size);

	static void LogMessage(wchar_t* message, long eventLevel);

private:
	CThorDAQGalvoGalvo();
	void StopHardwareWaits();
	HANDLE CreateExperimentThread(DWORD& threadID);
	long MoveGalvoToParkPosition(int Galvo_X_channel, int Galvo_Y_channel);/// Park the Galvo 
	long MoveDACChannelToPosition(int dacChannel, double position);/// move the desired dac channel to desired position
	long MoveGalvoToCenter(int Galvo_X_channel, int Galvo_Y_channel, ImgAcqPty* pImgAcqPty);// Center the scanner
	void FindStartLocation(double& xGalvoParkPosition, double& yGalvoParkPosition, ImgAcqPty* pImgAcqPty); // Find GG start location

	long MovePockelsToParkPosition(PockelPty* pockelPty);// Park the Pockel 
	long MovePockelsToPowerLevel(long index, PockelPty* pockelPty);
	long FindPockelsMinMax(long index, PockelPty* pockelPty);
	long BuildPolylineWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, GalvoStruct* galvoCtrlX, GalvoStruct* galvoCtrlY, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms);
	int CountChannelBits(long channelSet);

	long GetMinFlybackCycle();
	long GetFlybackCycle();
	void SetFlybackCycle(long flybackCycle);
	double GetFlybackTime(long flybackCycles);

	long ConfigAcqSettings(ImgAcqPty* pImgAcqPty);
	long ConfigStimSettings(ImgAcqPty* pImgAcqPty);
	long SetupFrameBuffer(ImgAcqPty* pImgAcqPty);

	long UpdateDACMemorySettings(IMAGING_BUFFER_STRUCT& DACMemorySettings);

	LONG BuildGalvoWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, GalvoStruct* galvoCtrlX, GalvoStruct* galvoCtrlY, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms);
	LONG BuildFastOneWayGalvoWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, GalvoStruct* galvoCtrlX, GalvoStruct* galvoCtrlY, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms);

	LONG BuildPockelsControlWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, PockelPty* pockelPty, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, bool buildDigiWave = true);
	LONG BuildPockelsControlFastOneWayWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, PockelPty* pockelPty, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, bool buildDigiWave = true);

	long BuildPockelsControlPowerRampWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, PockelPty* pockelPty, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, bool buildDigiWave = true);
	long BuildPockelsControlPowerRampFastOneWayWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, PockelPty* pockelPty, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, bool buildDigiWave = true);

	void SetupPowerRampSettings(ImgAcqPty* pImgAcqPty);
	void SetPowerAndBuildPockelsPowerRampWaveforms(PockelPty pockelsSettings, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, bool buildDigitalWaveforms);
	long SetAndBuildFastZWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine, ImgAcqPty* pImgAcqPty, double frameRate, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms2, ThorDAQZWaveformParams* zWaveformParams, bool useFastOneway);

	LONG GetDACSamplesPerLine(ScanLineStruct* scanLine, ImgAcqPty* PImgAcqPty, double& dac_rate, double dwell_time, double& line_time, bool onewayLineScan, bool& useFastOneWayMode);
	LONG GetDACSamplesPerLine3P(ScanLineStruct* scanLine, ImgAcqPty* PImgAcqPty, double& dac_rate, double dwell_time, double& line_time, bool onewayLineScan, bool& useFastOneWayMode);

	long AlignDataLoadFile();
	long WaitForHardwareTrigger(long timeoutMillis, long showTimeoutMessageBox);
	void StopDaqBrd();
	void SetupDataMaps();
	long ConfigDACWaveforms(ImgAcqPty* pImgAcqPty);
	long SetSelectedImagingAOs(long selectedImagingGG);
	long SetSelectedStimAOs(long selectedImagingGG);
	long SetStimDACTriggerOptions(THORDAQ_TRIGGER_MODES startTriggerMode, DAC_FREERUN_WAVEFORM_CONFIG& daqStimCfg);
};