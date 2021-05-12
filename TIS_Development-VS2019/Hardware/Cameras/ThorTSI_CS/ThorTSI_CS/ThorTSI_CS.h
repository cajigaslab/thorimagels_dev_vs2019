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

#define ThorTSIErrChk(fnName,fnCall,fnThrow) if (0 != fnCall) { StringCbPrintfW(_errMsg,MSG_SIZE,L"%s TSI_CS failed ,(%d). ",fnName,__LINE__); ThorCam::getInstance()->LogMessage(_errMsg,ERROR_EVENT); GetLastError(); if(fnThrow){ throw "fnCall"; }}

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
		//int opMode; ///<Operation Mode
		int triggerMode; ///<trigger source of each frame
		int triggerPolarity;
		//int triggerSource;
		int gain;
		int blackLevel;
		//int tapsIndex;
		//int tapBalanceEnable;
		int numImagesToBuffer;
		int readOutSpeedIndex; // 0=60MHz, 1=100MHz
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
		int frameRateControlEnabled;
		double pixelSizeXUM;
		double pixelSizeYUM;
		double frameRateControlValue;

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

	class ThorCam: ICamera
	{
	private:
		ThorCam();

		static bool _instanceFlag;
		static std::shared_ptr <ThorCam> _single;

		static bool _sdkIsOpen;
		static std::string _camName[MAX_CAM_NUM];
		static std::string _camSerial[MAX_CAM_NUM];
		static void* _camera[MAX_CAM_NUM];
		static TL_CAMERA_USB_PORT_TYPE _cameraInterfaceType[MAX_CAM_NUM];
		static unsigned long long _bufferImageIndex;
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

		USHORT* _intermediateBuffer;
		ImgPty _ImgPty;
		ImgPty _ImgPty_Pre;
		ImgPty _ImgPty_SetSettings;
		wchar_t * _pDetectorName;
		wchar_t * _pSerialNumber;
		unsigned long long _lastImage;
		unsigned long long _previousLastImage; ///<index of copied frame
		unsigned long long _availableFramesCnt; ///<available frame count to be copied
		long _lastDMABufferCount;
		std::map<long, TSI_ParamInfo*> _cameraMapParams;
		long _forceSettingsUpdate;

		//range values
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
		const int DEFAULT_HEIGHT;
		const int MIN_CHANNEL;
		const int MAX_CHANNEL;
		const int DEFAULT_CHANNEL;
		const int MAX_BITS_PERPIXEL;
		const int MIN_BITS_PERPIXEL;
		double _frameRateControlValueRange[2];

		//callback functions:
		static void CameraConnectedCallback(char* cameraSerialNumber, enum TL_CAMERA_USB_PORT_TYPE usb_bus_speed, void* context);
		static void CameraDisconnectedCallback(char* cameraSerialNumber, void* context);
		static void FrameAvailableCallback(void* sender, unsigned short* image_buffer, int frame_count, unsigned char* metadata, int metadata_size_in_bytes, void* context);

		//private functions:
		void ClearAllCameras();
		void ClearMem();
		static long DeSelectCamera(long cameraIndex);
		static void FindAllCameras();
		static void GetLastError();
		void InitialParamInfo(void);
		static bool IsOpen(const long cameraIndex);
		static void LogMessage(wchar_t *message,long eventLevel);///<Send a message to the log file
		void ProcessAverageFrame(unsigned short* dst, unsigned long previousDMAIndex);
		long SetBdDMA(ImgPty *pImgPty);
		static void StopCamera(long cameraIndex);

	public:
		static ThorCam* getInstance();
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

		~ThorCam();
	};

#ifdef __cplusplus
}
#endif