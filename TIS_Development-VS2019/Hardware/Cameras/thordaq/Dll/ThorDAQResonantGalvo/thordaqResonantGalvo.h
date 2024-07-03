#ifndef CONST_H
#include "const.h"
#endif

#ifndef ACQ_H
#include "acq.h"
#endif
#include "stdafx.h"
#include "NIDAQmx.h"
#include "thordaqcmd.h"
#include "thordaqapi.h"
#include "ThorDAQZAPI.h"
#include "ThorDAQZ.h"
#include "FrmCirBuf.h"
#include "dataprocessor.h"
#include "mROIDataProcessor.h"
#include "ThorDAQDMABuffer.h"
#include "..\ThorDAQIOXML.h"
#include "mROI/ScanmROI.h"
#include "mROI/mROIExperimentLoader.h"
#include "mROI/mROIStripesManager.h"
#include "mROICircularBuffer.h"

//#include "mROI/WaveformManagerBase.h"
// Convert function name macro to wide string for ThordaqErrChk
#define WIDE2(x) L##x
#define WIDE1(x) WIDE2(x)
#define WFUNCTION WIDE1(__FUNCTION__)

#define ThordaqErrChk(fnName,fnCall) if ((error=(fnCall)) > STATUS_SUCCESSFUL){ StringCbPrintfW(thordaqLogMessage,MSG_SIZE,WFUNCTION L": %s failed. Error code %d",fnName, error); LogMessage(thordaqLogMessage,ERROR_EVENT); }

//extern std::unique_ptr<ImageDistortionCorrectionDll> ImageDistortionCorrection;

//enum ScanMode
//{
//	TWO_WAY_SCAN = 0,
//	FORWARD_SCAN = 1,
//	BACKWARD_SCAN = 2,
//	CENTER = 3,
//	BLEACH_SCAN = 4
//};

// This class is exported from the ThordaqResonantGalvo.dll
class CThordaqResonantGalvo : ICamera{

	struct ScanStruct
	{
		double forward_lines;
		double backward_lines;
		double overall_lines;
		double dac_rate;
	};

	struct mROIScanAreaStruct
	{
		double amplitude;
		double startXPosition;
		double startYPosition;
		long forwardLines;
		long mROIFlyLines;
		long overallLines;
		long scanDirection;
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

	struct PockelsImagePowerRampStruct
	{
		double startPowerLevel;
		double endPowerLevel;
	};

	// Define a structure to hold indices for a pixel
	struct PixelData
	{
		int pixTL, pixTR, pixBL, pixBR;
		double factorTL, factorTR, factorBL, factorBR;
		int pixelIndex;
	};

private:
	static bool _instanceFlag;
	static auto_ptr<CThordaqResonantGalvo> _single;	  // Singleton Instance
	static ImgAcqPty _imgAcqPty;                      // Static Image Acquisition Properties
	static ImgAcqPty _imgAcqPty_Pre;            // Static Image Acquisition previous Properties
	static IMAGING_CONFIGURATION_STRUCT _daqAcqCfg;            /// Global thordaq configuration
	static long _DAQDeviceIndex;                 // <The Index of connected ThorDAQ Device
	static double _crsFrequencyHighPrecision;
	static long _triggerWaitTimeout;///<amount of time to wait for hardware trigger
	static ICircularBuffer* _pFrmBuffer;///<pointer to the circular buffer

	static IDataProcessor* _pDataProcessor;
	static UCHAR* _BufferContiguousArray[MAX_CHANNEL_COUNT];
	static std::vector <ProcessedFrame*> _pHistoryBuf;
	static atomic<long long> _indexOfLastCompletedFrame;						///<counter for tracking the sequence index of the current frame
	static atomic<long long> _indexOfLastCopiedFrame;
	static HANDLE _hThreadStopped;                                      ///<Signals if DAQ is stopped
	static HANDLE _hStopAcquisition;									///<event to stop frame acquisition
	static HANDLE _hTriggerTimeout;										///<Signals if HW trigger timed out
	static HANDLE _hHardwareTriggerInEvent;
	static HANDLE _hAcquisitionThread;									///<Main Acquisition thread
	static ScanStruct  _scan_info;
	static ScanLineStruct _scanLine;
	static UINT64 _powerRampCurrentIndex;
	static UINT64 _fastZCurrentIndex;
	static UINT64 _twoBankFrameIndex;
	static std::vector<PockelsImagePowerRampStruct> _pockelsImagePowerRampVector[MAX_POCKELS_CELL_COUNT];
	static std::vector<double> _pockelsResponsePowerLevels[MAX_POCKELS_CELL_COUNT];
	static ThorDAQZWaveformParams _fastZWaveformParams;
	static long _useBuiltZWaveform;
	wchar_t _errMsg[MSG_SIZE];                   // error message written to the log file
	unsigned int _deviceNum;                                      // <The total number of connected boards
	double _frameRate;
	//from xml
	long _oneXFieldSize;///<field size that matches the 1X zoom criteria
	long _fieldSizeCalibrationAvailable;
	long _fieldSizeMin; ///<minimum field size
	long _fieldSizeMax; ///<maximum field size
	double _fieldSizeCalibration;///<calibration value for the camera corrected with zoom array
	double _fieldSizeCalibrationXMLvalue;///<calibration value for the camera
	double _minFlybackCyclesFactor;
	/// pockel cell control
	long    _pockelsEnable[MAX_POCKELS_CELL_COUNT];///<the pockels cell control has been enabled	
	string  _pockelsPowerInputLine[MAX_POCKELS_CELL_COUNT];///<NI connection string for pockel(s) analog response		
	float64 _pockelsReadArray[MAX_POCKELS_CELL_COUNT][POCKELS_VOLTAGE_STEPS];
	double  _pockelsScanVoltageStart[MAX_POCKELS_CELL_COUNT];///<start voltage when scanning for detecting pockels output
	double  _pockelsScanVoltageStop[MAX_POCKELS_CELL_COUNT];///<stop voltage when scanning for detecting pockels output
	double  _pockelsVoltageSlopeThreshold;///<slope threshold for detecting min/max of pockels output
	long    _pockelsResponseType[MAX_POCKELS_CELL_COUNT];///<power fit method:[0]sinusoidal [1]linear
	string  _pockelsLine[MAX_POCKELS_CELL_COUNT];///<NI connection string for pockel(s)
	long    _pockelsParkAtMinimum;///<flag for parking mode of pockels cell
	long    _pockelsMaskChanged;///<flag set to TRUE when the pockels mask is updated
	long	_pockelsTurnAroundBlank; ///<Flag to enable pockels blanking on the turnaround
	long	_pockelsFlybackBlank; ///<Flag to enable pockels blanking on the flyback to the start of the image
	char*	_pPockelsMask[MAX_POCKELS_CELL_COUNT];///<pointer to a mask buffer for the pockels masking
	size_t	_pockelsMaskSize[MAX_POCKELS_CELL_COUNT];///<size of the mask to be used for pockels masking
	std::vector <ProcessedFrame*>  _pTempBuf; ///Intermediate buffer for CopyAcquisition
	std::vector <ProcessedFrame*>  _pTempBufCorrection; ///Intermediate buffer for imageCorrection in CopyAcquisition
	short	_preDcOffset[MAX_CHANNEL_COUNT]; // Pre-FIR DC-Offset values for channels 1 to MAX_CHANNEL_COUNT 
	long	_rGGMode; ///If the scanner driven by this dll is RGG, enable this flag
	double	_offsetVal;  ///Used to calc Start of Image Y Galvo Pos and use MoveGalvoToCenter waveformBuffer to not Trip Y Galvo.  Hack
	USHORT* _datamap[MAX_CHANNEL_COUNT];///<datamap array
	USHORT  _datamapPositiveSigned[65536];///<datamap for positive voltages
	USHORT  _datamapNegativeSigned[65536];///<datamap for negative voltages
	USHORT  _datamapPositiveUnsigned[65536];///<datamap for positive voltages
	USHORT  _datamapNegativeUnsigned[65536];///<datamap for negative voltages
	USHORT  _datamapIndependent[65536];///<datamap that folds the positive and negative
	long	_saveCrsFrequencyToLog;	///When enabled it logs the CRS Frequency read by the board into the log file.
	double _waveformUpdateRateSPS;
	long    _rawFlybackCycle;///<number of flyback lines
	//Initiation part
	long _scannerInitMode;
	long _forceSettingsUpdate; // Force refresh DAQ configuration
	long _rotationAnglePosition; ///<the angle index of the rotation 0,1,2,3
	long _preMoveGalvoToStartPosition; ///<the angle index of the rotation 0,1,2,3
	//galvo setting related parameters
	double _field2Theta; // GUI field (0 ~255) to scanner mechanical scan p-p angle in degree  
	double _theta2Volts; // Galvo mechnical scan p-p angle to waveformBuffer amplitude p-p in volts
	///<Arrays for file path and alignment calibration
	int _shiftArray[256];											///<backward forward alignment value2
	long _zoomArray[256];											///<zoom alignment values
	long _useZoomArray;												///<flag to determine if the zoom array should be enabled
	// NI DAQ Part
	long _numNIDAQ; ///<number of NI DAQ boards
	TaskHandle _taskHandleAIPockels[MAX_POCKELS_CELL_COUNT]; ///<pockels power input
	double _current_resonant_scanner_frequency;
	long _hardwareTestModeEnable;///<the pockels cell control has been enabled
	std::vector<string> _thorDAQDigitalLinesConfig;
	std::map<UINT8, long> _digitalIOSelection;
	long _captureActiveInvert;
	BOARD_INFO_STRUCT _boardInfo;
	LOW_FREQ_TRIG_BOARD_INFO_STRUCT _lowFreqTrigBoardInfo;
	std::map<AO, long> _thordaqAOSelection;
	ThorDAQDMAbuffer S2MMdmaBuffer;
	Scan* _mROIScan;
	std::unique_ptr<mROIExperimentLoader> _mROIExpLoader;///<experiment loader to create scan info from file
	static wchar_t thordaqLogMessage[MSG_SIZE];	
	long _lastAreaCopiedIndex;
	double _zPos_min;
	double _zPos_max;
	double _zVolts2mm;
	double _zOffsetmm;
	double _zTypicalMoveSettlingTime_sec;
	double _zTypicalMove_mm;
	double* _pTempDoubleBuffCorrection;
	int _totalLinesFormROI;
	long _scannerType;
	THORDAQ_BOB_TYPE _tdqBOBType;
	std::vector<PixelData> _imageDistortionCorrectionPixelData;
	double _scanLensFocalLength;
//function section
public:
	~CThordaqResonantGalvo();

	static CThordaqResonantGalvo* GetInstance();
	static void LogMessage(wchar_t* message, long eventLevel);

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
	void SetStatusHandle(HANDLE handle);

private:
	static UINT StartFrameAcqProc(LPVOID instance);
	static void TerminateTask(TaskHandle& handle);
	static long SetFrameBufferReadyOutput();///<Digital line that goes active after a frame has been copied to the output buffer
	static void CTHORDAQCALLBACK DACReadyForNextImageWaveformsCallback(THORDAQ_STATUS status, void* callbackData);


	CThordaqResonantGalvo(void);
	void StopHardwareWaits();
	HANDLE CaptureCreateThread(DWORD &threadID);
	long MoveGalvoToParkPosition(long Galvo_Y_channel, long isTearDown);/// Park the Galvo 
	long MoveGalvoToCenter(long Galvo_Y_channel, double offsetVal);// Center the scanner
	long MovePockelsToParkPosition(PockelPty* pockelPty);// Park the Pockel 
	long MovePockelsToPowerLevel(PockelPty* pockelPty);
	long FindPockelsMinMax(long index, PockelPty* pockelPty);
	int CountChannelBits(long channelSet);
	long ConfigAcqSettings(ImgAcqPty* pImgAcqPty);
	long SetupFrameBuffer(ImgAcqPty* pImgAcqPty);

	long GetDACSamplesPerLine(ScanLineStruct* scanLine, double& dac_rate, double line_time);
	long BuildGalvoWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine, GalvoStruct* galvoCtrl, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, bool invert, bool flybackToStart);
	long BuildTestWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine, GalvoStruct* galvoCtrl);
	long BuildPockelsWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine, PockelPty* pockelPty, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, bool invert, bool flybackToStart);
	long BuildPockelsPowerRampWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine, PockelPty* pockelPty, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, bool invert, bool flybackToStart);
	void SetupPowerRampSettings(ImgAcqPty* pImgAcqPty);
	void SetPowerAndBuildPockelsPowerRampWaveforms(PockelPty pockelsSettings, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms);

	long SetAndBuildFastZWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine, ImgAcqPty* pImgAcqPty, double frameRate, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms2, ThorDAQZWaveformParams* zWaveformParams);
	
	long CorrectPreludeImageDistortion(const USHORT* pixels, USHORT* pixelsDup, long channelNum, long channelEnable, long width, long height);

	long GetMinFlybackCycle();
	long GetMinFlybackCycle(double fieldY_volt);
	long GetFlybackCycle();
	double GetFlybackTime();
	double GetFlybackTime(long flybackCycle);
	void SetFlybackCycle(long flybackCycle);
	double GetField2VoltsX(double maxSizeInUMForMaxFieldSize);
	double GetField2VoltsY(double maxSizeInUMForMaxFieldSize);
	long LoadAlignDataFile();///<Load the two way alignment files from disk

	void StopDaqBrd();
	long WaitForHardwareTrigger(long timeoutMillis, long showTimeoutMessageBox);
	long ManipulateAndCopyImage(unsigned short* pData, unsigned short* dst, long width, long height, long channelNum, long channelEnable, long rawChannelsOnly, ICamera::LSMGRRotationAngle rotationAngle, long horizontalFlip);
	void SetupDataMaps();
	void PrepareImageDistortionCorrectionParameters(ImgAcqPty* pImgAcqPty, long channelCount);
	long SetupFrameBuffermROI(ImgAcqPty* pImgAcqPty, vector<StripInfo*> mROIStrips);
	long BuildmROIWaveforms(vector<StripInfo*> mROIStrips, ScanStruct* scanInfo, ScanLineStruct* scanLine, GalvoStruct* galvoCtrlX, GalvoStruct* galvoCtrlY, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms);
	long CalculatemROITotalImagingLinesAndOtherProperties(ImgAcqPty* pImgAcqPty, vector<StripInfo*> mROIStrips, int& overlLines, int& vSize, int& hSize, int& totalXPixels, int& totalYPixels, int& flybackCycle);
};
