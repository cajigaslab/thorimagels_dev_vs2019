#pragma once
#include "stdafx.h"

#define MAX_DMABUFNUM			4096
#define MIN_DMABUFNUM			4
#define DEFAULT_DMABUFNUM		256
#define MAX_CAM_NUM				8
#define TSI_TEXT_CMD_SIZE		1024
#define MIN_IMAGE_BUFFERS		2
#define DEFAULT_IMAGE_BUFFERS	4

#define MAX_AVGNUM				32
#define MIN_AVGNUM				1
#define DEFAULT_AVGNUM			1
#define MAX_FRAMENUM			4096			// Maximum limit of frame count

#define OrcaErrChk(fnName,fnCall) if (failed(err=(fnCall))) { StringCbPrintfW(_errMsg,MSG_SIZE,L"%s ORCA failed ,(%d). ",fnName,__LINE__); ORCA::getInstance()->LogMessage(_errMsg,ERROR_EVENT);}

#ifdef __cplusplus

extern "C"
{
#endif

	typedef struct _ImgPty
	{
		int exposureTime_us;		
		int roiBinX;		
		int roiBinY;	
		int roiLeft;		
		int roiRight;		
		int roiTop;		
		int roiBottom;		
		int triggerMode; ///<trigger source of each frame
		int triggerPolarity;
		int gain;
		int blackLevel;
		//int tapsIndex;
		//int tapBalanceEnable;
		int numImagesToBuffer;
		int readOutSpeedIndex; // 1 = Ultra Quiet, 2 = Standard, 3 = Fast
		int channel; ///<Bitwise selection of channels.		
		int averageMode; ///< average mode, see enumeration of AverageMode;
		int averageNum;///< number of frame, lines to average		
		long numFrame; ///<number of frame to acquire for a experiment
		int dmaBufferCount;///<number of buffers for DMA
		int verticalFlip;///<flip the image in the Y direction
		int horizontalFlip;///<flip the image in the X direction
		int imageAngle; ///<0,90,180,270
		int widthPx;
		int heightPx;
		int bitPerPixel;
		int hotPixelThreshold;
		int hotPixelEnabled;
		int binIndex;
		int hotPixelLevelIndex;
		double pixelSizeXUM;
		double pixelSizeYUM;

	}ImgPty, *pImgPty;

	typedef struct _ImageProperties
	{
		unsigned long long bufferIndex;
		unsigned long sizeInBytes;
		unsigned long width;
		unsigned long height;
		unsigned long long frameNumber;
	}ImageProperties;

	typedef struct _TSI_PaparamInfo {
		long tsiParamID;
		long paramType;
		long paramAvailable;
		long paramReadOnly;
		double paramMin;
		double paramMax;
		double paramDefault;
	} TSI_ParamInfo;

	class ORCA: ICamera
	{
	private:
		ORCA();

		static bool _instanceFlag;
		static std::shared_ptr <ORCA> _single;
		static HANDLE _hFrameAcqThread;	
		static HANDLE _hFrmBufReady; ///<Signals if the frame data buffer is ready to copy

		static bool _sdkIsOpen;
		static HDCAM _hdcam[MAX_CAM_NUM]; ///<DCAM  handle, one per camera
		static HDCAMWAIT _hwait; ///<wait handle, used for imaging, to wait for a response from the camera 
		static DCAMWAIT_START _waitstart; ///< wait start param
		static DCAMCAP_TRANSFERINFO	_captransferinfo; ///< Handle used to get the transfer info of every image access request
		static std::string _camName[MAX_CAM_NUM];
		static std::string _camSerial[MAX_CAM_NUM];
		static std::string _cameraInterfaceType[MAX_CAM_NUM];
		static unsigned long long _bufferImageIndex;
		static unsigned long long _copiedFrameNumber;
		static ThreadSafeMem<USHORT> _pFrmDllBuffer[MAX_DMABUFNUM];
		static ThreadSafeQueue<ImageProperties> _imagePropertiesQueue;
		static ImgPty _imgPtyDll;///<settings data structure
		static unsigned long _expectedImageSize;
		static unsigned long _lastCopiedImageSize;
		static unsigned long long _frameCountOffset;
		static long _1stSet_Frame;
		static long  _numCameras;
		static long _camID;
		static wchar_t _errMsg[MSG_SIZE];
		static bool _cameraRunning[MAX_CAM_NUM];
		static HANDLE _hStopAcquisition; ///<event to stop frame acquisition
		static long _maxFrameCountReached; ///<flag after exceed max frame count
		static HANDLE _hFrmBufHandle; ///<Mutex to claim the exclusive access to the buffer
		static std::atomic<long> _frameReady;
		static long _statusError;
		static long _threadRunning;
		void**	_pFrames;

		USHORT* _intermediateBuffer;
		ImgPty _ImgPty;
		ImgPty _ImgPty_Pre;
		ImgPty _ImgPty_SetSettings;
		wchar_t * _pDetectorName;
		wchar_t * _pSerialNumber;
		unsigned long long _lastImage;
		unsigned long long _previousLastImage; ///<index of copied frame
		unsigned long long _availableFramesCnt; ///<available frame count to be copied
		unsigned long long _frameNumberLiveImage;
		long _lastDMABufferCount;
		std::map<long, TSI_ParamInfo*> _cameraMapParams;
		long _forceSettingsUpdate;
		long _singleBinning;
		long _readInitialValues;
		long _paramHotPixelAvailable;
		long _timeoutMS;

		//range values
		int _binRange[2];
		int _hbinRange[2];
		const int DEFAULT_XBIN;
		int _vbinRange[2];
		int _blackLevelRange[2];
		int _gainRange[2];
		const int DEFAULT_YBIN;
		unsigned int _frmPerTriggerRange[2];
		const int DEFAULT_FRM_PER_TRIGGER;
		int _xRangeL[2];		//[min, max]
		int _xRangeR[2];
		int _yRangeT[2];
		int _yRangeB[2];
		const double MIN_ANGLE;
		const double MAX_ANGLE;
		const double DEFAULT_ANGLE;
		long long _expUSRange[2];
		int _hotPixelRange[2];
		const int DEFAULT_EXPOSURE_MS;
		int _widthRange[2];
		const int DEFAULT_WIDTH;
		int _heightRange[2];
		int _readOutSpeedRange[2];
		const int DEFAULT_HEIGHT;
		const int MIN_CHANNEL;
		const int MAX_CHANNEL;
		const int DEFAULT_CHANNEL;
		const int MAX_BITS_PERPIXEL;
		const int MIN_BITS_PERPIXEL;

		//private functions:
		HANDLE  FrameAcqThread(DWORD &threadID);

		void ClearAllCameras();
		void ClearMem();
		static long DeSelectCamera(long cameraIndex);
		static void FindAllCameras();
		void InitialParamInfo(void);
		static bool IsOpen(const long cameraIndex);
		static void LogMessage(wchar_t *message,long eventLevel);///<Send a message to the log file
		void ProcessAverageFrame(unsigned short* dst, unsigned long previousDMAIndex);
		long SetBdDMA(ImgPty *pImgPty);
		static void StopCamera(long cameraIndex);
		static inline const int get_dcamdev_string( DCAMERR& err, HDCAM hdcam, int32 idStr, char* text, int32 textbytes );
		static void GetCameraInfo(HDCAM hdcam, long index);
		static long GetAttributeFromCamera(HDCAM hdcam, long requestedProperty, DCAMPROP_ATTR& returnedAttr);
		static void ReadFramesFromCamera(void *instance);

	public:
		static ORCA* getInstance();
		long FindCameras(long &cameraCount);
		long SelectCamera(const long camera);
		long TeardownCamera();
		long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);
		long SetParam(const long paramID, const double param);
		long GetParam(const long paramID, double &param);
		long PreflightAcquisition(char * pDataBuffer);
		long SetupAcquisition(char * pDataBuffer);
		long StartAcquisition(char * pDataBuffer);
		long StatusAcquisition(long &status);
		long StatusAcquisitionEx(long &status,long &indexOfLastCompletedFrame);
		long CopyAcquisition(char * pDataBuffer, void* frameInfo);
		long PostflightAcquisition(char * pDataBuffer);
		long GetLastErrorMsg(wchar_t*msg,long size);
		long SetParamString(const long paramID, wchar_t * str);
		long GetParamString(const long paramID, wchar_t * str, long size);
		long SetParamBuffer(const long paramID, char * buf, long size);
		long GetParamBuffer(const long paramID, char * buf, long size);

		~ORCA();
	};

#ifdef __cplusplus
}
#endif