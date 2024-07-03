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

#define ThorTSIErrChk(fnCall, fnThrow){if(0!=fnCall){HandleSDKError(tl_camera_get_last_error(),__LINE__,#fnCall,fnThrow);}}
#define ThorTSIColorErrChk(fnCall, fnThrow){if(0!=fnCall){HandleSDKError(tl_mono_to_color_get_last_error(),__LINE__,#fnCall,fnThrow);}}
#define ThorTSIPolarErrChk(fnCall, fnThrow){int polarResult = fnCall; if(0!=polarResult){HandlePolarSDKError(polarResult,__LINE__,#fnCall,fnThrow);}}

inline void CharStringIntoWString(const char* inCharString, wstring& outWString)
{
	outWString.clear();
	size_t inLength = strlen(inCharString) + 1;
	outWString.resize(inLength);
	size_t numCharsConverted;
	mbstowcs_s(&numCharsConverted, &outWString[0], inLength, inCharString, inLength);
}

#ifdef __cplusplus

extern "C"
{
#endif

	typedef struct _CameraProperties
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
		int channelBitmask; ///<Bitwise selection of channels.	
		int numChannels; ///< number of channels.		
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
		int colorImageType; // 0 for unprocessed, 1 for sRGB, 2 for linear sRGB
		//TL_CAMERA_SENSOR_TYPE sensorType; // Monochrome, Bayer, Monochrome Polarized // this is not something that can be set
		double redGain;
		double greenGain;
		double blueGain;
		int polarImageType; // 0=Unprocessed, 1=Intensity, 2=DoLP, 3=Azimuth, 4=Quadview
		int isEqualExposurePulseEnabled;
		double equalExposurePulseWidth;
		bool isAutoWhiteBalanceEnabled;
		int whiteBalanceFrameCount;
		bool oneShotWhiteBalanceFlag;

		/// <summary>
		/// 
		/// Return true iff image properties could be changed to the other without stopping, false otherwise
		/// 
		/// </summary>
		/// <param name="otherImgPty"></param>
		/// <returns></returns>
		bool CanBeChangedWithoutStop(_CameraProperties& otherCameraProperties)
		{
			return (roiBinX == otherCameraProperties.roiBinX) &&
				(roiBinY == otherCameraProperties.roiBinY) &&
				(roiBottom == otherCameraProperties.roiBottom) &&
				(roiLeft == otherCameraProperties.roiLeft) &&
				(roiRight == otherCameraProperties.roiRight) &&
				(roiTop == otherCameraProperties.roiTop) &&
				(triggerMode == otherCameraProperties.triggerMode) &&
				(triggerPolarity == otherCameraProperties.triggerPolarity) &&
				(bitPerPixel == otherCameraProperties.bitPerPixel) &&
				(pixelSizeXUM == otherCameraProperties.pixelSizeXUM) &&
				(pixelSizeYUM == otherCameraProperties.pixelSizeYUM) &&
				(numImagesToBuffer == otherCameraProperties.numImagesToBuffer) &&
				(readOutSpeedIndex == otherCameraProperties.readOutSpeedIndex) &&
				(averageMode == otherCameraProperties.averageMode) &&
				(numChannels == otherCameraProperties.numChannels) &&
				(channelBitmask == otherCameraProperties.channelBitmask) &&
				(averageNum == otherCameraProperties.averageNum) &&
				(numFrame == otherCameraProperties.numFrame) &&
				(dmaBufferCount == otherCameraProperties.dmaBufferCount) &&
				(verticalFlip == otherCameraProperties.verticalFlip) &&
				(horizontalFlip == otherCameraProperties.horizontalFlip) &&
				(imageAngle == otherCameraProperties.imageAngle) &&
				(hotPixelEnabled == otherCameraProperties.hotPixelEnabled) &&
				(hotPixelThreshold == otherCameraProperties.hotPixelThreshold) &&
				(gain == otherCameraProperties.gain) &&
				(blackLevel == otherCameraProperties.blackLevel) &&
				(frameRateControlEnabled == otherCameraProperties.frameRateControlEnabled) &&
				(colorImageType == otherCameraProperties.colorImageType) &&
				(polarImageType == otherCameraProperties.polarImageType) &&
				(redGain == otherCameraProperties.redGain) && // TODO: these could be removed from this check, right?
				(greenGain == otherCameraProperties.greenGain) &&
				(blueGain == otherCameraProperties.blueGain) &&
				(isEqualExposurePulseEnabled == otherCameraProperties.isEqualExposurePulseEnabled) &&
				(equalExposurePulseWidth == otherCameraProperties.equalExposurePulseWidth);
		}

		/// <summary>
		/// 
		///  Copy the settings from another image property structure
		/// 
		/// </summary>
		/// <param name="otherCameraProperties"></param>
		void CopyFrom(_CameraProperties& otherCameraProperties)
		{
			exposureTime_us = otherCameraProperties.exposureTime_us;
			roiBinX = otherCameraProperties.roiBinX;
			roiBinY = otherCameraProperties.roiBinY;
			roiBottom = otherCameraProperties.roiBottom;
			roiLeft = otherCameraProperties.roiLeft;
			roiRight = otherCameraProperties.roiRight;
			roiTop = otherCameraProperties.roiTop;
			triggerMode = otherCameraProperties.triggerMode;
			triggerPolarity = otherCameraProperties.triggerPolarity;
			bitPerPixel = otherCameraProperties.bitPerPixel;
			pixelSizeXUM = otherCameraProperties.pixelSizeXUM;
			pixelSizeYUM = otherCameraProperties.pixelSizeYUM;
			numImagesToBuffer = otherCameraProperties.numImagesToBuffer;
			readOutSpeedIndex = otherCameraProperties.readOutSpeedIndex;
			numChannels = otherCameraProperties.numChannels;
			channelBitmask = otherCameraProperties.channelBitmask;
			averageMode = otherCameraProperties.averageMode;
			averageNum = otherCameraProperties.averageNum;
			numFrame = otherCameraProperties.numFrame;
			dmaBufferCount = otherCameraProperties.dmaBufferCount;
			verticalFlip = otherCameraProperties.verticalFlip;
			horizontalFlip = otherCameraProperties.horizontalFlip;
			imageAngle = otherCameraProperties.imageAngle;
			widthPx = otherCameraProperties.widthPx;
			heightPx = otherCameraProperties.heightPx;
			hotPixelEnabled = otherCameraProperties.hotPixelEnabled;
			hotPixelThreshold = otherCameraProperties.hotPixelThreshold;
			gain = otherCameraProperties.gain;
			blackLevel = otherCameraProperties.blackLevel;
			frameRateControlEnabled = otherCameraProperties.frameRateControlEnabled;
			frameRateControlValue = otherCameraProperties.frameRateControlValue;
			colorImageType = otherCameraProperties.colorImageType;
			polarImageType = otherCameraProperties.polarImageType;
			redGain = otherCameraProperties.redGain;
			greenGain = otherCameraProperties.greenGain;
			blueGain = otherCameraProperties.blueGain;
			isEqualExposurePulseEnabled = otherCameraProperties.isEqualExposurePulseEnabled;
			equalExposurePulseWidth = otherCameraProperties.equalExposurePulseWidth;
			whiteBalanceFrameCount = otherCameraProperties.whiteBalanceFrameCount;
			isAutoWhiteBalanceEnabled = otherCameraProperties.isAutoWhiteBalanceEnabled;
			oneShotWhiteBalanceFlag = otherCameraProperties.oneShotWhiteBalanceFlag;
		}

		/// <summary>
		/// 
		///  Copy the settings from another image property structure - ONLY ASYNC PARAMS
		///	 This means only parameters that are expected to be nudged by the camera
		/// 
		/// </summary>
		/// <param name="otherCameraProperties"></param>
		void CopyNudgedParamsFrom(_CameraProperties& otherCameraProperties)
		{
			exposureTime_us = otherCameraProperties.exposureTime_us;
			roiBottom = otherCameraProperties.roiBottom;
			roiLeft = otherCameraProperties.roiLeft;
			roiRight = otherCameraProperties.roiRight;
			roiTop = otherCameraProperties.roiTop;
			pixelSizeXUM = otherCameraProperties.pixelSizeXUM;
			pixelSizeYUM = otherCameraProperties.pixelSizeYUM;
			widthPx = otherCameraProperties.widthPx;
			heightPx = otherCameraProperties.heightPx;
			frameRateControlValue = otherCameraProperties.frameRateControlValue;
			equalExposurePulseWidth = otherCameraProperties.equalExposurePulseWidth;
		}

	}CameraProperties, *pCameraProperties;

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

	// TODO: not static, just parameters that are host side only or can be safely cached and don't need to be queried from camera 
	// Camera parameters that will be considered static while the camera is open and don't need to be read from the camera every time.
	typedef struct _StaticCameraParams
	{
		std::string camName;
		std::string camSerial;
		std::string camModel;
		void* cameraHandle;
		void* colorProcessorHandle;
		void* colorProcessorHandleForWhiteBalance;
		void* polarProcessorHandle;
		TL_CAMERA_USB_PORT_TYPE cameraInterfaceType;
		TL_CAMERA_SENSOR_TYPE cameraSensorType;
		TL_POLARIZATION_PROCESSOR_POLAR_PHASE cameraPolarPhase;
		bool isCameraRunning;
		bool isHardwareTriggerSupported;
		bool isBulbModeSupported;
		bool isEqualExposurePulseSupported;
		bool isFrameRateControlSupported;
		bool resyncFlag;
		long long framesProcessedSinceStart;
	} StaticCameraParams;

	class ThorCam: ICamera
	{
	private:
		ThorCam();

		static bool _instanceFlag;
		static std::shared_ptr <ThorCam> _single;

		static bool _isCameraSdkOpen;
		static bool _isColorSdkOpen;
		static bool _isPolarSdkOpen;
		static StaticCameraParams _cameraParams[MAX_CAM_NUM];
		static unsigned long long _bufferImageIndex;
		static ThreadSafeMem<USHORT> _pFrmDllBuffer[MAX_DMABUFNUM];
		static ThreadSafeQueue<ImageProperties> _imagePropertiesQueue;
		static unsigned long _expectedImageSizeFromCamera;
		static unsigned long _expectedProcessedImageSize;
		static unsigned long _lastCopiedImageSize;
		static unsigned long long _frameCountOffset;
		static long _1stSet_Frame;
		static long  _numCameras;
		static long _camID;
		static wchar_t _errMsg[MSG_SIZE];
		static HANDLE _hStopAcquisition; ///<event to stop frame acquisition
		static long _maxFrameCountReached; ///<flag after exceed max frame count
		static HANDLE _hFrmBufHandle; ///<Mutex to claim the exclusive access to the buffer

		USHORT* _intermediateBuffer;
		CameraProperties _CameraProperties_UI;
		static CameraProperties _CameraProperties_Active;

		wchar_t * _pDetectorName;
		wchar_t * _pSerialNumber;
		unsigned long long _lastImage;
		unsigned long long _previousLastImage; ///<index of copied frame
		unsigned long long _availableFramesCnt; ///<available frame count to be copied
		long _lastDMABufferCount;
		std::map<long, TSI_ParamInfo*> _cameraMapParams;
		long _forceSettingsUpdate;
		std::mutex _paramSyncLock;

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
		long SyncActiveCameraProperties(CameraProperties *pCameraProperties);
		static void StopCamera(long cameraIndex);
		static void ApplyGreyWorldColorBalance(unsigned short* imageData, int xOrigin, int yOrigin, int width, int height, int imageWidth, float* redGain, float* greenGain, float* blueGain);
		void ProcessColor(unsigned short* input, unsigned short* output, const ImageProperties& imageProperties, FrameInfoStruct* frameInfo);
		void ProcessPolar(unsigned short* input, unsigned short* output, const ImageProperties& imageProperties, FrameInfoStruct* frameInfo);
		long SetROI(int left, int right, int top, int bottom);

		//inline functions
		inline static void HandleSDKError(const char* errorMsg, int lineNumber, const char* functionName, bool isThrow)
		{
			std::wstring errorMessageW;

			wstring errorMsg_wide;
			wstring functionName_wide;
			CharStringIntoWString(errorMsg, errorMsg_wide);
			CharStringIntoWString(functionName, functionName_wide);

			StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorTSI_CS Error: call to \"%ls\" (ln. %d) returned error: \"%ls\"", functionName_wide.c_str(), lineNumber, errorMsg_wide.c_str());
			if (isThrow)
			{
				ThorCam::getInstance()->LogMessage(_errMsg, ERROR_EVENT);
				GetLastError();
				throw "fnCall";
			}
			else
			{
				ThorCam::getInstance()->LogMessage(_errMsg, INFORMATION_EVENT);
			}
		}

		inline static void HandlePolarSDKError(int errorCode, int lineNumber, const char* functionName, bool isThrow)
		{
			// Because polar SDK does not have a 'get last error', implement a version of that here
			std::string errorMsg;
			auto error = static_cast<TL_POLARIZATION_PROCESSOR_ERROR>(errorCode);

			switch (errorCode)
			{
			case TL_POLARIZATION_PROCESSOR_ERROR_MODULE_NOT_INITIALIZED:
			{
				errorMsg = "Attempted to use polarization processor module before it was initialized.";
				break;
			}
			case TL_POLARIZATION_PROCESSOR_ERROR_MEMORY_ALLOCATION_FAILURE:
			{
				errorMsg = "Unable to allocate memory for polarization processing.";
				break;
			}
			case TL_POLARIZATION_PROCESSOR_ERROR_NULL_INSTANCE_HANDLE:
			{
				errorMsg = "A null polar processor handle was passed to polar processing module.";
				break;
			}
			case TL_POLARIZATION_PROCESSOR_ERROR_NULL_INPUT_BUFFER_POINTER:
			{
				errorMsg = "A null input buffer was passed to a polar transform function.";
				break;
			}
			case TL_POLARIZATION_PROCESSOR_ERROR_ALL_OUTPUT_BUFFER_POINTERS_ARE_NULL:
			{
				errorMsg = "None of the output buffers of a polar transform function call were non-null. Set at least one output buffer to a valid buffer with a size that is large enough to fit the requested image.";
				break;
			}
			case TL_POLARIZATION_PROCESSOR_ERROR_IDENTICAL_INPUT_AND_OUTPUT_BUFFERS:
			{
				errorMsg = "Polar transform cannot use the input buffer as the output buffer. Please allocate and/or choose a different output buffer location.";
				break;
			}
			case TL_POLARIZATION_PROCESSOR_ERROR_DUPLICATE_OUTPUT_BUFFER:
			{
				errorMsg = "Duplicate output buffers were found during polar transformation. Please use unique buffers for each desired transformation output.";
				break;
			}
			case TL_POLARIZATION_PROCESSOR_ERROR_INVALID_POLAR_PHASE:
			{
				errorMsg = "An unknown polar phase was passed into a polar transform function.";
				break;
			}
			case TL_POLARIZATION_PROCESSOR_ERROR_INVALID_MAX_SCALING_VALUE:
			{
				errorMsg = "An invalid maximum scaling value was passed into a polar transform function.";
				break;
			}
			case TL_POLARIZATION_PROCESSOR_ERROR_INVALID_IMAGE_WIDTH:
			{
				errorMsg = "An invalid image width was passed into a polar transform function.";
				break;
			}
			case TL_POLARIZATION_PROCESSOR_ERROR_INVALID_IMAGE_HEIGHT:
			{
				errorMsg = "An invalid image height was passed into a polar transform function.";
				break;
			}
			case TL_POLARIZATION_PROCESSOR_ERROR_INVALID_IMAGE_DATA_BIT_DEPTH:
			{
				errorMsg = "An invalid image bit depth was passed into a polar transform function.";
				break;
			}
			case TL_POLARIZATION_PROCESSOR_ERROR_INITIALIZATION_ERROR:
			{
				errorMsg = "An error occurred during initialization of the polar processing module. Please check that all polarization DLLs are discoverable from this application.";
				break;
			}
			case TL_POLARIZATION_PROCESSOR_ERROR_TERMINATION_ERROR:
			{
				errorMsg = "An error occurred during termination of the polar processing module.";
				break;
			}
			case TL_POLARIZATION_PROCESSOR_ERROR_UNKNOWN:
			default:
			{
				errorMsg = "Unknown issue occurred in the polarization processor module.";
				break;
			}
			}

			HandleSDKError(errorMsg.c_str(), lineNumber, functionName, isThrow);
		}

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