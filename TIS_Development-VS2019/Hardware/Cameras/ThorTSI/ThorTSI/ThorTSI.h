#pragma once
#include "stdafx.h"
#include "TsiSDK.h"
#include "TsiCamera.h"
#include "TsiColorCamera.h"
#include "TsiCameraInternal.h"
#include "TsiImage.h"
#include "TsiColorImage.h"

#define MAX_DMABUFNUM 4096
#define MIN_DMABUFNUM 4
#define DEFAULT_DMABUFNUM 128
#define MAX_CAM_NUM 4
#define TSI_TEXT_CMD_SIZE	1024
#define MAX_TSI_SDK_IMAGES	8			// Matches the camera SDK code

#define MAX_IMAGE_HEIGHT		4096
#define MIN_IMAGE_HEIGHT		1
#define DEFAULT_IMAGE_HEIGHT	512

#define MAX_IMAGE_WIDTH		4096
#define MIN_IMAGE_WIDTH		1
#define DEFAULT_IMAGE_WIDTH	512

#define MAX_AVGNUM		32
#define MIN_AVGNUM		1
#define DEFAULT_AVGNUM	1

#define MAX_BLACK_LEVEL		100
#define MIN_BLACK_LEVEL		1
#define DEFAULT_BLACK_LEVEL 48

#define MAX_GAIN		1024
#define MIN_GAIN		0
#define DAFAULT_GAIN	90

#define MAX_XBIN		24
#define MIN_XBIN		1
#define DAFAULT_XBIN	1

#define MAX_YBIN		10
#define MIN_YBIN		1
#define DAFAULT_YBIN	1

#define MAX_ANGLE		360
#define MIN_ANGLE		-90
#define DAFAULT_ANGLE	0

#define MAX_EXPOSURETIME_MS		10000
#define MIN_EXPOSURETIME_MS		0
#define DAFAULT_EXPOSURETIME_MS 30

#define MAX_BITS_PERPIXEL		14
#define MIN_BITS_PERPIXEL		8
#define DEFAULT_BITS_PERPIXEL	14

#define MAX_CHANNEL		1
#define MIN_CHANNEL		0
#define DEFAULT_CHANNEL	1

#ifdef __cplusplus

extern "C"
{
#endif

	typedef union _TSI_SCALAR_DATA {

		uint8_t		uint8_Value;
		uint16_t	uint16_Value;
		uint32_t	uint32_Value;
		uint64_t	uint64_Value;
		int8_t		int8_Value;
		int16_t		int16_Value;
		int32_t		int32_Value;
		int64_t		int64_Value;
		float		float_Value;

	} TSI_SCALAR_DATA, *PTSI_SCALAR_DATA;

	typedef struct _ImgPty
	{
		uint32_t exposureTime_us;		
		TSI_ROI_BIN	roiBin;		
		TSI_OP_MODE opMode; ///<Operation Mode
		uint32_t triggerMode; ///<trigger source of each frame
		TSI_HW_TRIG_POLARITY triggerPolarity;
		uint32_t triggerSource;
		uint32_t gain;
		uint32_t blackLevel;
		uint32_t tapsIndex;
		uint32_t tapBalanceEnable;
		uint32_t numImagesToBuffer;
		uint32_t readOutSpeedIndex; // 0=20MHz, 1=40MHz
		uint32_t channel; ///<Bitwise selection of channels.		
		uint32_t averageMode; ///< average mode, see enumeration of AverageMode;
		uint32_t averageNum;///< number of frame, lines to average		
		uint32_t numFrame; ///<number of frame to acquire for a experiment
		uint32_t dmaBufferCount;///<number of buffers for DMA
		uint32_t verticalFlip;///<flip the image in the Y direction
		uint32_t horizontalFlip;///<flip the image in the X direction
		uint32_t imageAngle; ///<0,90,180,270
		uint32_t bitsPerPixel; // Used to store the camera's bitsPerPixel 

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
		TSI_PARAM_ID	tsiParamID;
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

		TsiSDK* _sdk;	
		bool _sdkOpen;
		long  _numCameras;
		long _selectedCam;
		long _running;
		ImgPty _ImgPty;
		ImgPty _ImgPty_Pre;
		ImgPty _ImgPty_SetSettings;
		wchar_t * _pDetectorName;
		wchar_t * _pSerialNumber;
		unsigned long long _lastImage;
		unsigned long long _previousLastImage;
		unsigned long long _availableFramesCnt; ///<available frame count to be copied
		long _lastDMABufferCount;
		long _forceSettingsUpdate;
		TSI_CAMERA_HARDWARE_INTERFACE_TYPE _cameraInterfaceType;
		std::map<long, TSI_ParamInfo*> _cameraMapParams;
		double _saved_Left[MAX_CAM_NUM];
		double _saved_Right[MAX_CAM_NUM];
		double _saved_Top[MAX_CAM_NUM];
		double _saved_Bottom[MAX_CAM_NUM];
		recursive_mutex _sdkNotThreadsafeMutex; // for CCD sdk calls that are known to be un-threadsafe

		bool InitTsiSDK();
		bool IsOpen(const unsigned long cameraIndex);
		long DeselectCamera(long index);
		long GetTsiParamInfo(TSI_ParamInfo* paramInfo, const unsigned long cameraIndex);	
		long GetTsiParameter_uint(const unsigned long cameraIndex, TSI_PARAM_ID paramID, unsigned int &param);
		long GetTsiParameter_float(const unsigned long cameraIndex, TSI_PARAM_ID paramID, float &param);
		long BuildTsiParamInfoMap(const unsigned long cameraIndex);
		long GetTsiMappedParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);
		long SetBdDMA(ImgPty *pImgPty);

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

		static void LogMessage(wchar_t *message,long eventLevel);///<Send a message to the log file
		~ThorCam();

		wchar_t _errMsg[MSG_SIZE];
		static TsiCamera* _camera[MAX_CAM_NUM];
		static bool _cameraRunning[MAX_CAM_NUM];
		static unsigned long long _bufferImageIndex;
		static unsigned short *_pFrmDllBuffer[MAX_DMABUFNUM];///<processed data buffers
		static ThreadSafeQueue<ImageProperties> _imagePropertiesQueue;
		static ImgPty _imgPtyDll;///<settings data structure
		static unsigned long _expectedImageSize;
		static unsigned long _lastCopiedImageSize;
		static HANDLE _hFrmBufHandle;///<mutex to control access to data buffers
	};

#ifdef __cplusplus
}
#endif