#ifndef CONST_H
#include "const.h"
#endif

#ifndef ACQ_H
#include "acq.h"
#endif

#include "thordaqcmd.h"
#include "thordaqapi.h"
#include "FrmCirBuf.h"
#include "memorypool.h"
#include "dataprocessing.h"

struct ScanStruct
{
	double forward_lines;
	double backward_lines;
	double overall_lines;
};

struct ScanLineStruct
{
	UINT32 samples_idle;
	UINT32 samples_scan;
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

// This class is exported from the ThordaqResonantGalvo.dll
class CThordaqResonantGalvo {
//available section
public:

private:
	static bool _instanceFlag;
	static auto_ptr<CThordaqResonantGalvo> _single;	  // Singleton Instance
	static ImgAcqPty _imgAcqPty;                      // Static Image Acquisition Properties
	static ImgAcqPty _imgAcqPty_Pre;            // Static Image Acquisition previous Properties
	static IMAGING_CONFIGURATION_STRUCT _daqAcqCfg;            /// Global thordaq configuration
	static long _DAQDeviceIndex;                 // <The Index of connected ThorDAQ Device
	static double _crsFrequencyHighPrecision;
	static long _triggerWaitTimeout;///<amount of time to wait for hardware trigger
	static FrameCirBuffer* _pFrmBuffer;
	static DataProcessing* _pDataProcess;
	static vector<UCHAR*> _pBuffer; // buffer pointer to buffer read from FPGA
	static UCHAR* _pHistoryBuf;
	static long long _indexOfLastCompletedFrame;						///<counter for tracking the sequence index of the current frame
	static long long _indexOfCopiedFrame;
	static HANDLE _hFrmBufHandle;					///<Mutex to claim the exclusive access to the buffer	
	static HANDLE _hThreadStopped;                                      ///<Signals if DAQ is stopped
	static HANDLE _hStopAcquisition;									///<event to stop frame acquisition
	static HANDLE _hTriggerTimeout;										///<Signals if HW trigger timed out
	static HANDLE _hHardwareTriggerInEvent;
	static HANDLE _hAcquisitionThread;									///<Main Acquisition thread
	wchar_t _errMsg[MSG_SIZE];                   // error message written to the log file
	MemoryPool*                         gPtrMemoryPool;
	unsigned int _deviceNum;                                      // <The total number of connected boards
	double _frameRate;
	//from xml
	long _oneXFieldSize;///<field size that matches the 1X zoom criteria
	long _fieldSizeCalibrationAvailable;
	double _fieldSizeCalibration;///<calibration value for the camera corrected with zoom array
	double _fieldSizeCalibrationXMLvalue;///<calibration value for the camera
	/// pockel cell control
	long    _pockelsEnable[MAX_POCKELS_CELL_COUNT];///<the pockels cell control has been enabled
	string  _pockelsPowerInputLine[MAX_POCKELS_CELL_COUNT];///<NI connection string for pockel(s) analog response		
	float64 _pockelsReadArray[MAX_POCKELS_CELL_COUNT][POCKELS_VOLTAGE_STEPS];
	double  _pockelsPhaseAdjustMicroSec;///<fine counter adjument to help align pockels waveform with sampling
	double  _pockelsScanVoltageStart[MAX_POCKELS_CELL_COUNT];///<start voltage when scanning for detecting pockels output
	double  _pockelsScanVoltageStop[MAX_POCKELS_CELL_COUNT];///<stop voltage when scanning for detecting pockels output
	double  _pockelsVoltageSlopeThreshold;///<slope threshold for detecting min/max of pockels output
	long    _pockelsResponseType[MAX_POCKELS_CELL_COUNT];///<power fit method:[0]sinusoidal [1]linear
	char*   _pPockelsMask;///<pointer to a mask buffer for the pockels masking
	long    _pockelsMaskSize;///<size of the mask to used for pockels masking
	string  _pockelsLine[MAX_POCKELS_CELL_COUNT];///<NI connection string for pockel(s)
	long    _pockelsParkAtMinimum;///<flag for parking mode of pockels cell
	double  _pockelsMaskPhaseShiftPercent;///<used to shift the pockels mask cosine fit
	long    _pockelsMaskChanged;///<flag set to TRUE when the pockels mask is updated
	string  _pockelDigOutput;
	long    _pockelsMaskWidth;///<number of x pixels on the mask
	long    _useReferenceForPockelsOutput;///<when pockels is enabled, if the max pockels voltage should be use to build the waveform instead of the set pockels power
	
	long    _rawFlybackCycle;///<number of flyback lines
	bool _minimizeFlybackCycles; //Force to use minimal flyback cycles
	//Initiation part
	long _scannerInitMode;
	long _forceSettingsUpdate; // Force refresh DAQ configuration
	//galvo setting related parameters
	double _field2Theta; // GUI field (0 ~255) to scanner mechanical scan p-p angle in degree  
	double _theta2Volts; // Galvo mechnical scan p-p angle to waveform amplitude p-p in volts
	///<Arrays for file path and alignment calibration
	int _shiftArray[256];											///<backward forward alignment value2
	long _zoomArray[256];											///<zoom alignment values
	long _useZoomArray;												///<flag to determine if the zoom array should be enabled
	// NI DAQ Part
	long _numNIDAQ; ///<number of NI DAQ boards
	TaskHandle _taskHandleAIPockels[MAX_POCKELS_CELL_COUNT]; ///<pockels power input
	double _current_resonant_scanner_frequency;

//function section
public:
	~CThordaqResonantGalvo();
	static CThordaqResonantGalvo* GetInstance();
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
	long CopyAcquisition(char * pDataBuffer,long &moreAvailable);
	long CopyAcquisitionInlcudingDisabledChannels(char * pDataBuffer);
	long CopyAcquisitionSkippingDisabledChannels(char * pDataBuffer);
	long PostflightAcquisition(char * pDataBuffer);
	long GetLastErrorMsg(wchar_t * msg, long size);
	static void LogMessage(wchar_t *message,long eventLevel);
	long SetParamString(const long paramID, wchar_t * str);
	long GetParamString(const long paramID, wchar_t * str, long size);
	long GetParamBuffer(const long paramID, char * pBuffer, long size);
	long SetParamBuffer(const long paramID, char * pBuffer, long size);
	void SetStatusHandle(HANDLE handle);

private:
	CThordaqResonantGalvo(void);
	static UINT StartFrameAcqProc(void *instance);
	static long GetFrameCount(USHORT& frame_count);
	static UINT HandleFrameBuf(int enter,DWORD timeOut);
	void StopHardwareWaits();
	HANDLE CaptureCreateThread(DWORD &threadID);
	long MoveGalvoToParkPosition(int Galvo_Y_channel);/// Park the Galvo 
	long MoveGalvoToCenter(int Galvo_Y_channel);// Center the scanner
	long MovePockelsToParkPosition(PockelPty* pockelPty);// Park the Pockel 
	long MovePockelsToPowerLevel(PockelPty* pockelPty);
	long FindPockelsMinMax(long index, PockelPty* pockelPty);
	int CountChannelBits(long channelSet);
	long ConfigAcqSettings(ImgAcqPty* pImgAcqPty);
	long SetupFrameBuffer();
	long GetDACSamplesPerLine(ScanLineStruct* scanLine, double& dac_rate, double line_time);
	long BuildGalvoWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine, double dac_rate, GalvoStruct* galvoCtrl);
	long BuildTestWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine, double dac_rate, GalvoStruct* galvoCtrl);
	long BuildPockelsWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine,  double dac_rate, PockelPty* pockelPty);
    long GetResonantScannerFreq(long sample_rate,double& freq);
	long	GetMinFlybackCycle();
	long	GetFlybackCycle();
	long	GetFlybackCycle(double flybackTime);
	double  GetFlybackTime();
	double  GetFlybackTime(long flybackCycle);
	void	SetFlybackCycle(long flybackCycle);
	long LoadAlignDataFile();///<Load the two way alignment files from disk
	void StopDaqBrd();
	long LoadDACMemorySettings( IMAGING_BUFFER_STRUCT& DACMemorySettings);
	long UpdateDACMemorySettings( IMAGING_BUFFER_STRUCT& DACMemorySettings);
};
