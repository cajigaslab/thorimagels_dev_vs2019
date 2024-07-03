#ifndef CONST_H
#include "const.h"
#endif

#ifndef ACQ_H
#include "acq.h"
#endif

#include "..\dFLIM_4002\dFLIMcmd.h" // from common Lib\thordaq
#include "..\dFLIM_4002\dFLIMapi.h" // from common Lib\thordaq
#include "ThorRawDDR3buffer.h"  // NWL type DDR3 raw buff
#include "FrmCirBuf.h"
#include "memorypool.h"
#include "datastream.h"

#define WIDE2(x) L##x
#define WIDE1(x) WIDE2(x)
#define WFUNCTION WIDE1(__FUNCTION__)

#define ThordaqErrChk(fnName,fnCall) if ((error=(fnCall)) > STATUS_SUCCESSFUL){ StringCbPrintfW(message,_MAX_PATH,WFUNCTION L": %s failed. Error code %d",fnName, error); LogMessage(message,ERROR_EVENT); }
#define ThordaqErrChk2(fnName,fnCall) if ((error=(fnCall)) > STATUS_SUCCESSFUL){ StringCbPrintfW(message2,_MAX_PATH,WFUNCTION L": %s failed. Error code %d",fnName, error); LogMessage(message2,ERROR_EVENT); }

class ScanStruct
{
public:
	double forward_lines;
	double backward_lines;
	double overall_lines;
	ScanStruct( double forward_lines , double backward_lines)
	{
		this->forward_lines = forward_lines;
		this->backward_lines = backward_lines;
		this->overall_lines = forward_lines + backward_lines;
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

struct GalvoStruct
{
	SCAN_DIRECTION scan_direction;
	double amplitude;
	double park;
	double offset;
};


enum ScanMode
{
	TWO_WAY_SCAN_MODE = 0,
	FORWARD_SCAN_MODE = 1,
	BACKWARD_SCAN_MODE = 2,
	CENTER_SCAN_MODE = 3,
	WAVEFORM_MODE = 4
};

// This class is exported from the ThorDAQGalvoGalvo.dll
class CdFLIMGG :ICamera {
private:	
	static bool _instanceFlag;
	static auto_ptr<CdFLIMGG> _single;	// Singleton Instance
	static ImgAcqPty _imgAcqPty;                // Static Image Acquisition Properties
	static ImgAcqPty _imgAcqPty_Pre;            // Static Image Acquisition previous Properties
	static dFLIM_IMAGING_CONFIGURATION_STRUCT _daqAcqCfg;            /// Global daq configuration
	static long _DAQDeviceIndex;                 // <The Index of connected ThorDAQ Device
	const wchar_t * _pDetectorName;              // <Detector Name
	wchar_t _errMsg[MSG_SIZE];                   // error message written to the log file
	static long _triggerWaitTimeout;///<amount of time to wait for hardware trigger
	static FrameCirBuffer* _pFrmBuffer;
	static std::atomic<bool> _acquisitionRunning;
	MemoryPool* gPtrMemoryPool;
	// NWL buffer additions
	static UCHAR* _BufferDDR3chanArray[MAX_CHANNEL_COUNT]; // i.e., max buff size for single channel (single ping-pong bank)

	static DataStream* _pDataProcess;
	static long _acquisitionMode;

	unsigned int _deviceNum;                                      // <The total number of connected boards
	static double _frameRate;
	long _oneXFieldSize;///<field size that matches the 1X zoom criteria
	long _fieldSizeCalibrationAvailable;
	long _droppedFramesCnt; // <The number of dropped frames in the acquisition
	long _forceSettingsUpdate; // Force refresh DAQ configuration
	bool _minimizeFlybackCycles; //Force to use minimal flyback cycles
	long _frameTriggerEnableWithHWTrig;
	long _channelPolarity[4];///<store the polarity to use for each channel
	long _pockelsParkAtMinimum; ///<Flag for user to choose if pockels will park at the minimum calibration value
	long _pockelsDelayUS;
	
	double _pockelsMaxVoltage[MAX_POCKELS_CELL_COUNT];
	double _pockelsMinVoltage[MAX_POCKELS_CELL_COUNT];
	double _pockelsPowerLevel[MAX_POCKELS_CELL_COUNT];
	double _pockelsScanVoltageStart[MAX_POCKELS_CELL_COUNT];
    double _pockelsScanVoltageStop[MAX_POCKELS_CELL_COUNT];
	double _fieldSizeCalibration;
	long _pockelsEnable[MAX_POCKELS_CELL_COUNT];
	TaskHandle _taskHandleAIPockels[MAX_POCKELS_CELL_COUNT]; ///<pockels power input
	string  _pockelsPowerInputLine[MAX_POCKELS_CELL_COUNT];///<NI connection string for pockel(s) analog response		
	double  _pockelsVoltageSlopeThreshold;///<slope threshold for detecting min/max of pockels output
	string _pockelDigOutput;
	float64 _pockelsReadArray[MAX_POCKELS_CELL_COUNT][POCKELS_VOLTAGE_STEPS];
	long    _pockelsResponseType[MAX_POCKELS_CELL_COUNT];///<power fit method:[0]sinusoidal [1]linear
	string _pockelsReferenceLine;

	double _dflimClockFrequencies[NUM_DFLIM_CLOCKS];
	long _dflimFineShiftA[MAX_CHANNEL_COUNT];
	long _dflimFineShiftB[MAX_CHANNEL_COUNT];
	long _dflimCoarseShiftA[MAX_CHANNEL_COUNT];
	long _dflimCoarseShiftB[MAX_CHANNEL_COUNT];
	long _dflimEnableIntAdj[MAX_CHANNEL_COUNT];
	long _dflimThreshold[MAX_CHANNEL_COUNT];
	long _dflimBaselineTolerance[MAX_CHANNEL_COUNT];
	long _dflimMaxLevel0[MAX_CHANNEL_COUNT];
	long _dflimMaxLevel1[MAX_CHANNEL_COUNT];
	long _dflimResyncDelay;
	long _dflimSyncDelay;
	long _dflimResyncEveryLine;
	atomic<bool> _saveLiveImage;
	//galvo setting related parameters
	double _field2Theta; // GUI field (0 ~255) to scanner mechanical scan p-p angle in degree  
	double _theta2Volts; // Galvo mechnical scan p-p angle to waveform amplitude p-p in volts
	FlimBuffer* _pTempCopyFlimBuf;
	long	_ggSuperUserMode;
	double _maxAngularVelocityRadPerSec; ///<Maximum allowed angular velocity for the galvos in Rad/Sec
	double _maxAngularAccelerationRadPerSecSq; ///<Maximum allowed angular acceleration for the galvos in Rad/Sec^2

	static vector<FlimBuffer*> _pFlimBuffer;
	static vector<UCHAR*> _pRawDataBuffer;
	static FlimBuffer* _pHistoryFlimBuf;	
	static long long _index_of_last_written_frame;						///<counter for tracking the sequence index of the current frame
	static long long _index_of_last_read_frame;
	static long _bufferHSize;
	static long _shiftArray[256]; ///<backward forward alignment value
	void StopDaqBrd();

	

private:
	static HANDLE _hThreadStopped;                                      ///<Signals if DAQ is stopped
	static HANDLE _hStopAcquisition;									///<event to stop frame acquisition
	static HANDLE _hTriggerTimeout;										///<Signals if HW trigger timed out
	static HANDLE _hHardwareTriggerInEvent;
	static wchar_t message[MSG_SIZE];
	static HANDLE _hAcquisitionThread;									///<Main Acquisition thread
public:
	~CdFLIMGG();
	static CdFLIMGG* GetInstance();
	long FindCameras(long &cameraCount); ///<Search system to find digitizers and daq board
	long SelectCamera(const long camera);///<initilized digitizer;
	long TeardownCamera(); ///<close handles for each boards
	long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault); ///<get the information for each parameter
	long SetParam(const long paramID, const double param);///<set parameter value
	long GetParam(const long paramID, double &param);///<get the parameter value
	long PreflightAcquisition(char * pDataBuffer);///<Setup for , should be called before each experiment, or whenever trigger mode has been changed
	long dFLIMSetTestUtilConfig(CL_GUI_GLOBAL_TEST_STRUCT testConfig);
	long ComputeDataHSize(ImgAcqPty ImgAcqPty);
	long SetupAcquisition(char * pDataBuffer);///<Setup between frames, only used for software free run mode
	long StartAcquisition(char * pDataBuffer);///<Start a experiment,
	long StatusAcquisition(long &status);///<Status of a frame 
	long StatusAcquisitionEx(long &status,long &indexOfLastCompletedFrame);///<Status of a frame
	long CopyAcquisition(char * pDataBuffer, void* frameInfo);
	long PostflightAcquisition(char * pDataBuffer);
	long GetLastErrorMsg(wchar_t * msg, long size);
	static void LogMessage(wchar_t *message,long eventLevel);
	long SetParamString(const long paramID, wchar_t * str);
	long GetParamString(const long paramID, wchar_t * str, long size);
	long GetParamBuffer(const long paramID, char * pBuffer, long size);
	long SetParamBuffer(const long paramID, char * pBuffer, long size);
	THORDAQ_STATUS ConstrainHighestPixelDwell(long Xpixels, long Ypixels, double* PixelDwell);
	void SetStatusHandle(HANDLE handle);
	
private:
	CdFLIMGG();
	long ArmDaqBrd();
	void StopHardwareWaits();
	HANDLE CaptureCreateThread(DWORD &threadID);
	static UINT StartFrameAcqProc(void *instance);
	long MoveGalvoToParkPosition(int Galvo_X_channel,int Galvo_Y_channel);/// Park the Galvo 
	long MoveGalvoToCenter(int Galvo_X_channel,int Galvo_Y_channel, ImgAcqPty* pImgAcqPty);// Center the scanner


	long MovePockelsToParkPosition(PockelPty* pockelPty);// Park the Pockel 
	long MovePockelsToPowerLevel(long index, PockelPty* pockelPty);
	long FindPockelsMinMax(long index, PockelPty* pockelPty);

	int CountChannelBits(long channelSet);

	long GetMinFlybackCycle();
	long GetFlybackCycle();
	void SetFlybackCycle(long flybackCycle);
	double GetFlybackTime(long flybackCycles);
	
	long ConfigAcqSettings(ImgAcqPty* pImgAcqPty);
	long SetupFrameBuffer(int channel_count, ImgAcqPty* pImgAcqPty);

	long LoadDACMemorySettings( IMAGING_BUFFER_STRUCT& DACMemorySettings);
	long UpdateDACMemorySettings( IMAGING_BUFFER_STRUCT& DACMemorySettings);

	LONG BuildGalvoWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, double dac_rate, GalvoStruct* galvoCtrlX, GalvoStruct* galvoCtrlY, ImgAcqPty* pImgAcqPty);	
	LONG BuildPockelsControlWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, double dac_rate, PockelPty* pockelPty, bool blockBackwardLine, ImgAcqPty* pImgAcqPty);

	static UINT HandleFrameBuf(int enter,DWORD timeOut);
	static HANDLE _hFrmBufHandle; ///<Mutex to claim the exclusive access to the buffer

	LONG GetDACSamplesPerLine(ScanLineStruct* scanLine, ImgAcqPty* PImgAcqPty, double& dac_rate, double dwell_time, double& line_time, long& turnAroundSamplesDif, bool onewayLineScan);
	
	long AlignDataLoadFile();
};




