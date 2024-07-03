#ifndef CONST_H
#include "const.h"
#endif

#ifndef ACQ_H
#include "acq.h"
#endif
#include "buffer.h"
#include "datastream.h"
#include "FrmCirBuf.h"
#include "memorypool.h"
#include "dataprocessing.h"

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

// This class is exported from the ThorDAQGGDFLIMSim.dll
class CThorDAQGGDFLIMSim :ICamera {
private:	
	static bool _instanceFlag;
	static auto_ptr<CThorDAQGGDFLIMSim> _single;	// Singleton Instance
	static ImgAcqPty _imgAcqPty;                // Static Image Acquisition Properties
	static ImgAcqPty _imgAcqPty_Pre;            // Static Image Acquisition previous Properties
	static long _DAQDeviceIndex;                 // <The Index of connected ThorDAQ Device
	const wchar_t * _pDetectorName;              // <Detector Name
	wchar_t _errMsg[MSG_SIZE];                   // error message written to the log file
	static long _triggerWaitTimeout;///<amount of time to wait for hardware trigger
	static FrameCirBuffer* _pFrmBuffer;
	FlimBuffer* _pTempCopyBuf;
	static char* _simulatedFrame;
	MemoryPool* gPtrMemoryPool;

	static DataProcessing* _pDataProcess;

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
	double _pockelsMaxVoltage[MAX_POCKELS_CELL_COUNT];
	double _pockelsMinVoltage[MAX_POCKELS_CELL_COUNT];
	double _pockelsScanVoltageStart[MAX_POCKELS_CELL_COUNT];
    double _pockelsScanVoltageStop[MAX_POCKELS_CELL_COUNT];
	double _fieldSizeCalibration;
	long _pockelsEnable[MAX_POCKELS_CELL_COUNT];

	string  _pockelsPowerInputLine[MAX_POCKELS_CELL_COUNT];///<NI connection string for pockel(s) analog response		
	double  _pockelsVoltageSlopeThreshold;///<slope threshold for detecting min/max of pockels output
	string _pockelDigOutput;

	long    _pockelsResponseType[MAX_POCKELS_CELL_COUNT];///<power fit method:[0]sinusoidal [1]linear
	string _pockelsReferenceLine;
	//galvo setting related parameters
	double _field2Theta; // GUI field (0 ~255) to scanner mechanical scan p-p angle in degree  
	double _theta2Volts; // Galvo mechnical scan p-p angle to waveform amplitude p-p in volts

	static long long _index_of_last_written_frame;						///<counter for tracking the sequence index of the current frame
	static long long _index_of_last_read_frame;
	static long _bufferHSize;
	long _frameHSize;
	long _frameImageWidth;
	long _frameImageHeight;

	bool photonNumBufferCopied;
	bool singlePhotonBufferCopied;
	bool arrivalTimeSumBufferCopied;
	bool histogramBufferCopied;
	bool photonListCopied;

private:
	static HANDLE _hThreadStopped;                                      ///<Signals if DAQ is stopped
	static HANDLE _hStopAcquisition;									///<event to stop frame acquisition
	static HANDLE _hTriggerTimeout;										///<Signals if HW trigger timed out
	static HANDLE _hHardwareTriggerInEvent;

	static HANDLE _hAcquisitionThread;									///<Main Acquisition thread
public:
	~CThorDAQGGDFLIMSim();
	static CThorDAQGGDFLIMSim* GetInstance();
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
	CThorDAQGGDFLIMSim();
	long ArmDaqBrd();
	void StopHardwareWaits();
	HANDLE CaptureCreateThread(DWORD &threadID);
	static UINT StartFrameAcqProc(void *instance);

	int CountChannelBits(long channelSet);

	long GetMinFlybackCycle();
	long GetFlybackCycle();
	void SetFlybackCycle(long flybackCycle);
	double GetFlybackTime(long flybackCycles);
	
	long ConfigAcqSettings(ImgAcqPty* pImgAcqPty);
	long SetupFrameBuffer(int channel_count, ImgAcqPty* pImgAcqPty);
	void StopDaqBrd();
	static vector<FlimBuffer*> _pBuffer;
	static vector<UCHAR*> _pRawDataBuffer;
	static FlimBuffer* _pHistoryBuf;

	static UINT HandleFrameBuf(int enter,DWORD timeOut);
	static HANDLE _hFrmBufHandle;					///<Mutex to claim the exclusive access to the buffer	
};




