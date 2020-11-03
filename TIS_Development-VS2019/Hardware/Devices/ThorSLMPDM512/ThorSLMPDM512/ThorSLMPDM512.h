
#include "stdafx.h"

#define ReallocMemChk(MemStruct, targetSize) \
	if (0 == targetSize) { free(MemStruct.memPtr); MemStruct.memPtr = NULL; MemStruct.size = 0; } \
	else if (targetSize != MemStruct.size) { MemStruct.memPtr = (unsigned char*)realloc((void*)MemStruct.memPtr, targetSize); MemStruct.size = targetSize; }

#ifdef __cplusplus
extern "C"
{
#endif

	struct MemoryStruct
	{
		unsigned char* memPtr;
		size_t size;
	};

	class ThorSLM: IDevice
	{
	private:
		static bool _instanceFlag;
		static unique_ptr<ThorSLM> _single;

		static float* _fpPointsXY[MAX_ARRAY_CNT]; ///<interleaved x,y coordinate of slm points for frame per set
		long* _fpPointsXYSize; ///<total length of slm points per set, include both x and y

		const long DEFAULT_PIXEL_X;
		const long DEFAULT_PIXEL_Y;
		const int  DEFAULT_TRUE_FRAMES;
		const unsigned int DEFAULT_TRANSIENT_FRAMES;
		const unsigned int MAX_TRANSIENT_FRAMES;

		static wchar_t _errMsg[MSG_SIZE];
		static HANDLE _hStatusHandle;
		static MemoryStruct _intermediateBuf[MAX_ARRAY_CNT]; ///<buffer for all overdrive SLM transient frames
		static Blink_SDK* _blinkSDK;
		static unsigned int _slmTimeout; ///<timeout value for overdrive SLM, can be multiple of min value
		static MemoryStruct _slmTempBuf; ///<temperary buffer for calculation of transient frames
		static std::vector<unsigned int> _slmSeqVec; ///<sequence of pattern in random order
		static mutex _callbackMutex; ///<mutex lock to update static params used in callback

		long _fileSettingsLoaded; ///<Flag set when the settings from the settings file have been read
		std::string _pSlmName;///<slm device name
		double* _fitCoeff; ///<fitting coefficients in order of [0][0],[0][1],[0][2],[1][0],[1][1],[1][2],[2[0],[2][1]
		long _deviceCount; ///<how many SLM being detected
		bool _deviceDetected;
		long* _pixelRange; ///<pixel range of Xmin, Xmax, Ymin, Ymax
		long _overDrive; ///<overdrive mode for meadowlark slm
		unsigned int _transientFrames; ///<transient frame counts in overdrive mode for meadowlark slm
		std::string _lutFile; ///<look up table file for phase correction
		std::string _overDrivelutFile; ///<look up table file for overdrive SLM
		std::string _wavefrontFile; ///<wavefront file for wavefront correction
		unsigned short* _tableLUT; ///<look-up table from phase mask to bitmap
		unsigned char* _imgWavefront; ///<wavefront calibration for meadowlark phase mask correction
		MemoryStruct _firstBuf; ///<buffer for the first SLM pattern, used for calculating transient back to first.

		//params in struct:		
		long _pixelX; ///<pixel X, limited by device spec
		long _pixelY;
		long _slmFuncMode; ///<different function mode of slm
		std::wstring _bmpPathAndName; ///<current working bmp file path and name
		std::wstring _seqPathAndName; ///<current sequence file path and name to set pattern sequence
		static long _arrayOrFileID; ///<index of slm points set or bitmap buffers
		long _verticalFlip; ///<preset vertical flip before pattern generation
		double _rotateAngle; ///<preset rotation before pattern generation
		double _scaleFactor[2]; ///<preset scaling before pattern generation
		long _offsetPixels[2]; ///<preset offset in pixels before pattern generation
		static long _bufferCount; ///<total count of pattern buffers in circulation
		static long _slmRuntimeCalculate; ///<runtime calculation of transient frames
		double _calibZ;
		double _na;
		double _wavelength;
		long _slm3D; ///<hologram generation in 3D (true) or 2D (false)

		//NI:
		static TaskHandle _taskHandleCI; ///<trigger input
		std::string _counterLine; ///<counter for hw trigger input
		std::string _hwTriggerInput; ///<hw trigger input line name

	public:
		static ThorSLM* getInstance();

		long FindDevices(long &deviceCount); ///<Search system to find meadowlark slm
		long SelectDevice(const long device);///<
		long TeardownDevice(); ///<close handles
		long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault); ///<get the information for each parameter
		long SetParam(const long paramID, const double param);///<set parameter value
		long GetParam(const long paramID, double &param);///<get the parameter value
		long SetParamString(const long paramID, wchar_t * str);
		long GetParamString(const long paramID, wchar_t * str, long size);
		long GetParamBuffer(const long paramID, char * pBuffer, long size);
		long SetParamBuffer(const long paramID, char * pBuffer, long size);
		long PreflightPosition();///<Setup for initial settings, should be called before each experiment, or whenever trigger mode has been changed
		long SetupPosition();///<Setup between frames, only used for software free run mode
		long StartPosition();///<Start a experiment,
		long StatusPosition(long &status);///<Status of a frame 
		long ReadPosition(DeviceType deviceType, double &pos);///<Not available
		long PostflightPosition();
		long GetLastErrorMsg(wchar_t * msg, long size);

		~ThorSLM();

		void SetStatusHandle(HANDLE handle);

	private:
		ThorSLM();
		static int32 CVICALLBACK ThorSLM::HWTriggerCallback(TaskHandle taskHandle, int32 signalID, void *callbackData);
		void CloseNITasks();
		long CoordinatesRotate(float* ptArrays, long size, double angle);
		long CoordinatesVerticalFlip(float* ptArrays, long size);
		long CoordinatesScale(float* ptArrays, long size, double scaleX, double scaleY);
		long LoadHologram();
		long PersistAffineValues();
		long SaveHologram(bool saveInSubFolder);
		BOOL ReadLUTFile(std::wstring fileName);
		BOOL ReadWavefrontFile(std::wstring fileName);
		BOOL ReadAndScaleBitmap(int mode);
		void ReleaseMem();
		void ReleaseTransientBuf();
		void ReleaseDVI();
		void SetDefault();
		long ResetSequence(wchar_t* filename = L"");
		long SetIntermediateBuffer(MemoryStruct memStruct);
		long SetupHWTriggerIn();
		unsigned char* GetAndProcessBMP(long& size, BITMAPINFO& bmi);
		unsigned char* GetAndProcessText(long& size, BITMAPINFO& bmi);

	};

#ifdef __cplusplus
}
#endif