// ThorConfocalSimulator.h : main header file for the ThorLSMCam DLL
//
#include "stdafx.h"
#include "../../../../Common/BinaryImageDataUtilities/GenericImage.h"

#define INTERNAL_CLOCK_PIXEL_LENGTH 15760
#ifndef FLYBACK_CYCLE_SCALE
#define FLYBACK_CYCLE_SCALE 4
#endif
#define MAXBDNUM 8
#define DMABUFNUM 6

#define DAQmxErrChk(fnCall) if ((error=(fnCall)) < 0) throw "fnCall";
#define ThorErrChk(fnCall) if (200 > (error=(fnCall)) > 100) throw "fnCall";
#define AlzErrChk(fnCall) if ((error=(fnCall)) > 513) throw "fnCall";

#define DAQmxFailed(error)              ((error)<0)
#define ThorFnFailed(error)             ( 200 > (error) > 100 )
#define AlazarFailed(error)             ((error) > 513)



#ifdef __cplusplus
extern "C"
{
#endif


	typedef struct _VCMImgPty
	{
		long pixelX;  ///Image pixel number in the x direction
		long pixelY;  ///Image pixel number in the y direction
		long fieldSize; ///A parameter scales to scan field size, the actual scan sized at the object plane may vary from device to device  
		long offsetX; ///Off set of scan field size in the x direction
		long offsetY; ///Off set of scan field size in the y direction
		long channel; ///Bitwise selection of channels.  
		long alignmentForField; ///Forward Backward scan alignment
		long inputRangeChannel1; ///The digitizer input channel measurement range, see the enumeration of "inputrange" 
		long inputRangeChannel2;
		long inputRangeChannel3;
		long inputRangeChannel4;
		long clockSource; ///0 to use internal sample clock source, 1 to use external clock source
		long clockRateInternal; /// internal sample clock rate, the enumeration of "ClockRate"
		long clockRateExternal; /// external sample clock rate, this the actual sample rate
		long scanMode; /// 2 way, 1 way forward or 1 way backward scan mode, see enumeration of ScanMode
		long averageMode; /// average mode, see enumeration of AverageMode;
		long averageNum;/// number of frame, lines to average
		long triggerMode; ///trigger source of each frame
		long numFrame; ///number of frame to acquire for a experiment

	} VCMImgPty, *pVCMImgPty;

	enum ClockRate ///
	{
		SAMPLE_CLOCK_1KSPS =	0X00000001UL,///
		SAMPLE_CLOCK_2KSPS = 		0X00000002UL,
		SAMPLE_CLOCK_5KSPS =		0X00000004UL,
		SAMPLE_CLOCK_10KSPS =		0X00000008UL,
		SAMPLE_CLOCK_20KSPS =		0X0000000AUL,
		SAMPLE_CLOCK_50KSPS =		0X0000000CUL,
		SAMPLE_CLOCK_100KSPS =		0X0000000EUL,
		SAMPLE_CLOCK_200KSPS =		0X00000010UL,
		SAMPLE_CLOCK_500KSPS =		0X00000012UL,
		SAMPLE_CLOCK_1MSPS =		0X00000014UL,
		SAMPLE_CLOCK_2MSPS =		0X00000018UL,
		SAMPLE_CLOCK_5MSPS =		0X0000001AUL,
		SAMPLE_CLOCK_10MSPS =		0X0000001CUL,
		SAMPLE_CLOCK_20MSPS =		0X0000001EUL,
		SAMPLE_CLOCK_25MSPS =		0X00000021UL,
		SAMPLE_CLOCK_50MSPS =		0X00000022UL,
		SAMPLE_CLOCK_100MSPS =		0X00000024UL,
		SAMPLE_CLOCK_125MSPS =		0x00000025UL,
		SAMPLE_CLOCK_160MSPS =		0x00000026UL,
		SAMPLE_CLOCK_180MSPS =		0x00000027UL,
		SAMPLE_CLOCK_200MSPS =		0X00000028UL,
		SAMPLE_CLOCK_250MSPS =		0X0000002BUL,
		SAMPLE_CLOCK_500MSPS =		0X00000030UL,
		SAMPLE_CLOCK_1GSPS =		0x00000035UL,
		// user define sample rate - used with External Clock
		SAMPLE_CLOCK_USER_DEF =	0x00000040UL 
	};

	enum InputRange
	{
		INPUT_RANGE_20_MV =	0x00000001UL,
		INPUT_RANGE_40_MV=	0x00000002UL,
		INPUT_RANGE_50_MV=	0x00000003UL,
		INPUT_RANGE_80_MV=	0x00000004UL,
		INPUT_RANGE_100_MV=	0x00000005UL,
		INPUT_RANGE_200_MV=	0x00000006UL,
		INPUT_RANGE_400_MV=	0x00000007UL,
		INPUT_RANGE_500_MV=	0x00000008UL,
		INPUT_RANGE_800_MV=	0x00000009UL,
		INPUT_RANGE_1_V	=	0x0000000AUL,
		INPUT_RANGE_2_V	=	0x0000000BUL,
		INPUT_RANGE_4_V	=	0x0000000CUL,
		INPUT_RANGE_5_V	=	0x0000000DUL,
		INPUT_RANGE_8_V	=	0x0000000EUL,
		INPUT_RANGE_10_V	=	0x0000000FUL,
		INPUT_RANGE_20_V	=	0x00000010UL,
		INPUT_RANGE_40_V	=	0x00000011UL,
		INPUT_RANGE_16_V	=	0x00000012UL,
		INPUT_RANGE_HF	=	0x00000020UL
	};

	enum ScanMode
	{
		TWO_WAY_SCAN = 0,
		FORWARD_SCAN = 1,
		BACKWARD_SCAN = 2,
		SCAN_MODE_CENTER = 3,
		WAVEFORM = 4
	};

	enum AverageMode
	{
		NO_AVERAGE = 0,
		FRM_CUMULATIVE_MOVING,
		FRM_SIMPLE_MOVING,
		LINE_AVERAGE
	};


	class ThorLSMCam
	{
	private:
		enum AreaMode
		{
			SQUARE = 0,
			RECTANGLE = 1,
			LINE = 2
		};

		static bool _instanceFlag;
		static auto_ptr<ThorLSMCam> _single;
		ThorLSMCam();

		BYTE * _pBitmapBuffer;

		HBITMAP _hBitmap;

		long _frameIndex;

		long _pixelX;
		long _pixelY;
		long _fieldSize;
		long _offsetX;
		long _offsetY;
		long _channel;
		long _alignmentForField;
		long _inputRangeChannel1;
		long _inputRangeChannel2;
		long _inputRangeChannel3;
		long _inputRangeChannel4;
		long _clockSource;
		long _pulseMultiplexingEnable;
		long _clockRateInternal;
		long _clockRateExternal;
		long _scanMode;
		long _averageMode;
		long _averageNum;
		long _triggerMode;
		long _frameCount;
		long _realTimeDataAverage;
		long _galvoEnable;
		AreaMode _areaMode;
		long _yAmplitudeScaler;

		long _pixelX_C;
		long _pixelY_C;
		long _channel_C;

		long _flybackCycle;
		long _isMinumumFlyback;

		long _horizontalImageFlip;
		long _verticalImageFlip;

		double _pockelsPower0;
		double _pockelsPower1;
		double _pockelsPower2;
		double _pockelsPower3;
		double _pulseMultiplexingPhase;

		LARGE_INTEGER _largeint;
		LONGLONG _qPart1;
		double _dfrq;
		long _twoWayZones[251];///<digital pot zones for correcting two way alignment
		long _twoWayZonesFine[251];
		const long NUM_TWOWAY_ZONES;
		const int MAX_PIXEL_X;
		const int MIN_PIXEL_X;
		const int MAX_PIXEL_Y;
		const int MIN_PIXEL_Y;
		const int DEFAULT_PIXEL_X;
		const int DEFAULT_PIXEL_Y;
		const int MAX_FIELD_SIZE_X;
		const int MIN_FIELD_SIZE_X;
		const int DEFAULT_FIELD_SIZE_X;
		const int MIN_ALIGNMENT;
		const int MAX_ALIGNMENT;
		const int DEFAULT_ALIGNMENT;
		const int MIN_INPUTRANGE;
		const int MAX_INPUTRANGE;
		const int DEFAULT_INPUTRANGE;
		const int MIN_FLYBACK_TIME;
		const int MAX_FLYBACK_TIME;
		const int MIN_EXTCLOCKRATE;
		const int MAX_EXTCLOCKRATE;
		const int DEFAULT_EXTCLOCKRATE;
		const int MIN_INTERNALCLOCKRATE;
		const int MAX_INTERNALCLOCKRATE;
		const int DEFAULT_INTERNALCLOCKRATE;
		const int MIN_AVERAGENUM;
		const int MAX_AVERAGENUM;
		const int DEFAULT_AVERAGENUM;
		const int MIN_SCANMODE;
		const int MAX_SCANMODE;
		const int DEFAULT_SCANMODE;
		const double DEFAULT_FIELD_SCALE_XYRATIO;
		const int MIN_TRIGGER_MODE;
		const int MAX_TRIGGER_MODE;
		const int DEFAULT_TRIGGER_MODE;
		const double MAX_GALVO_VOLTAGE;
		const double MIN_GALVO_VOLTAGE;
		const int MIN_CHANNEL;
		const int MAX_CHANNEL;
		const long MIN_Y_AMPLITUDE_SCALER;
		const long MAX_Y_AMPLITUDE_SCALER;
		const long DEFAULT_Y_AMPLITUDE_SCALER;

	private:
		static HANDLE hStopAcquisition; ///event to stop frame acquisition
		static HANDLE hFrmBufReady; ///Signals if the frame data buffer is ready to copy
		static HANDLE hFrmBufHandle; ///Mutex to claim the exclusive access to the buffer
		static HANDLE hThreadStopped;
		static HANDLE hTriggerTimeout;
		static HANDLE hThread; ///thread handle

		VCMImgPty _ImgPty;
		VCMImgPty _ImgPty_Pre;
		double field_XYRatio;

		wstring _simulatorDataPath;
		vector<wstring> _simulatorData[4];
		long _fileWidth;
		long _fileHeight;
		char *_pMemoryBuffer;

		long num_alazar_sys; ///total num of alazar system in a computer,
		//the Master/Slave alazar boards are considered to be one system
		long num_NI_DAQ; ///number of NI DAQ boards

		double crs_frequency;
		int galvo_subtrigger_length;
		int galvo_data_length;
		int galvo_data_forward;
		int galvo_data_back;
		double galvo_offset;
		double field2Theta;

		long _indexOfLastFrame;
		wchar_t * _pDetectorName;

		long _rawSaveEnabledChannelsOnly;

		long _tileRow;
		long _tileCol;
		long _zSteps;
		long _rotationAngle;
		UINT64 _lastImageUpdateTime;
		long _imageUpdateIntervalMS;
	public:
		static ThorLSMCam* getInstance();

		long FindCameras(long &cameraCount); ///Search system to find digitizers and daq board 
		long SelectCamera(const long camera);///initilized digitizer;
		long TeardownCamera(); ///close handles for each boards
		long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault); ///get the information for each parameter
		long SetParam(const long paramID, const double param);///set parameter value
		long GetParam(const long paramID, double &param);///get the parameter value
		long PreflightAcquisition(char * pDataBuffer);///Setup for , should be called before each experiment, or whenever trigger mode has been changed
		long SetupAcquisition(char * pDataBuffer);///Setup between frames, only used for software free run mode
		long StartAcquisition(char * pDataBuffer);///Start a experiment, 
		long StatusAcquisition(long &status);///Status of a frame achu
		long StatusAcquisitionEx(long &status,long &indexOfLastFrame);///
		long CopyAcquisition(char * pDataBuffer, void* frameInfo);
		long PostflightAcquisition(char * pDataBuffer);
		long SetParamString(const long paramID, wchar_t * str);
		long GetParamString(const long paramID, wchar_t * str, long size);
		long SetParamBuffer(const long paramID, char * pBuffer, long size);

		~ThorLSMCam();

	private:
		void PrintText(char *pDataBuffer, int x, int y, wchar_t *str, int c); 
		void FillImageWithGradient(GenericImage<unsigned short>& image);
		unsigned short GetGradientValue(int x, int y, int channelNum);
		void FillImageWithData(GenericImage<unsigned short>& image, int chanIndex);
		long ManipulateImage(GenericImage<unsigned short> * image, long width, long height, ICamera::LSMGRRotationAngle rotationAngle);
		void PrintMessage(GenericImage<unsigned short>& image, double printTime);
	};

#ifdef __cplusplus
}
#endif
