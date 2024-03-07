
#include "stdafx.h"
#include "SlmManager.h"

#ifdef __cplusplus
extern "C"
{
#endif

	class ThorSLM : IDevice
	{
	private:
		static bool _instanceFlag;
		static unique_ptr<ThorSLM> _single;
		static CRITICAL_SECTION _accessCritSection; ///<critical section control for access resouces

		static float* _fpPointsXY[MAX_ARRAY_CNT]; ///<interleaved x,y coordinate of slm points for frame per set
		static float* _fpPointsXYZ[MAX_ARRAY_CNT]; ///<interleaved x,y,z coordinate of slm calibration points
		long* _fpPointsXYSize; ///<total length of slm points per set, include both x and y
		long* _fpPointsXYZSize; ///<total length of slm points per set, include x, y and z

		const long DEFAULT_PIXEL_X;
		const long DEFAULT_PIXEL_Y;
		const int  DEFAULT_TRUE_FRAMES;
		const unsigned int DEFAULT_TRANSIENT_FRAMES;
		const unsigned int MAX_TRANSIENT_FRAMES;

		static wchar_t _errMsg[MSG_SIZE];
		static HANDLE _hStatusHandle;
		static unsigned int _slmTimeout; ///<timeout value for overdrive SLM, can be multiple of min value
		static std::vector<unsigned int> _slmSeqVec; ///<sequence of pattern in random order
		static mutex _callbackMutex; ///<mutex lock to update static params used in callback
		static ISLM* _slmDevice;
		static HANDLE _hThread;
		static HANDLE _hStopThread; ///<event to stop SLM thread
		static HANDLE _hThreadStopped; ///<Signals if the SLM thread has stopped

		std::unique_ptr<SlmManager> _slmManager;
		long _fileSettingsLoaded; ///<Flag set when the settings from the settings file have been read
		static std::string _pSlmName;///<slm device name
		double* _fitCoeff[Constants::MAX_WIDEFIELD_WAVELENGTH_COUNT]; ///<fitting coefficients for two wavelengths in order of [0][0],[0][1],[0][2],[1][0],[1][1],[1][2],[2[0],[2][1]
		double* _fitCoeff3D[Constants::MAX_WIDEFIELD_WAVELENGTH_COUNT]; ///<3D fitting coefficients for two wavelengths in order of [0][0],[0][1],[0][2],[0][3],[1][0],[1][1],[1][2],[1][3],[2[0],[2][1],[2][2],[2][3],[3][0],[3][1],[3][2]
		long _deviceCount; ///<how many SLM being detected
		bool _deviceDetected;
		long _pixelSize[2]; ///<pixel size of X, Y
		static long _overDrive; ///<overdrive mode for meadowlark slm
		unsigned int _transientFrames; ///<transient frame counts in overdrive mode for meadowlark slm
		double _pixelPitchUM; ///<SLM pixel pitch size in [um]
		double _flatDiagRatio; ///<flatness correction range, compared with image's diagnal length
		double _flatPowerRange[2]; ///<power range for flatness correction, [min, max]
		std::string _lutFile; ///<look up table file for phase correction
		std::string _overDrivelutFile; ///<look up table file for overdrive SLM
		std::string _wavefrontFile; ///<wavefront file for wavefront correction
		unsigned short* _tableLUT; ///<look-up table from phase mask to bitmap
		unsigned char* _imgWavefront; ///<wavefront calibration for meadowlark phase mask correction
		MemoryStruct<float> _lastHoloBuf[2]; ///<buffers for the last generated SLM hologram, used for fast calculating defocus, [1]:focus,[0]:defocus
		MemoryStruct<unsigned char> _lastBmpBuf; ///<buffers for the last pushed SLM hologram, used for blanking

		//params in struct:		
		long _pixelX; ///<pixel X, limited by device spec
		long _pixelY;
		long _power2Px; ///<power of 2 square pixei size for FFT
		long _slmFuncMode; ///<different function mode of slm
		std::wstring _bmpPathAndName; ///<current working bmp file path and name
		std::wstring _seqPathAndName; ///<current sequence file path and name to set pattern sequence
		static long _arrayOrFileID; ///<index of slm points set or bitmap buffers
		static long _lastArrayOrFileID; ///<previous index of slm points set or bitmap buffers
		long _verticalFlip[Constants::MAX_WIDEFIELD_WAVELENGTH_COUNT]; ///<preset vertical flip before pattern generation
		double _rotateAngle[Constants::MAX_WIDEFIELD_WAVELENGTH_COUNT]; ///<preset rotation before pattern generation
		double _scaleFactor[Constants::MAX_WIDEFIELD_WAVELENGTH_COUNT][2]; ///<preset scaling before pattern generation
		long _offsetPixels[Constants::MAX_WIDEFIELD_WAVELENGTH_COUNT][2]; ///<preset offset in pixels before pattern generation
		static long _bufferCount; ///<total count of pattern buffers in circulation
		static long _slmRuntimeCalculate; ///<runtime calculation of transient frames

		//Get,Set:
		double _wavelength[Constants::MAX_WIDEFIELD_WAVELENGTH_COUNT]; ///<incident wavelengths [nm], could be two for left-right halves
		long   _phaseMax[Constants::MAX_WIDEFIELD_WAVELENGTH_COUNT]; ///<maximum phase value for incident wavelengths
		long _selectWavelength; ///<active wavelength selection, either 0[first] or 1[second]
		long _slm3D; ///<hologram generation in 3D (true) or 2D (false)
		long _loadPhaseDirectly; ///<load or save phase mask directly (TRUE) or default load-then-convert (FALSE)
		long _dmdMode; ///<use SLM in DMD mode if TRUE
		bool _doHologram; ///<phase image applied by hologram & transform if TRUE
		bool _inSave; ///<if TRUE in save hologram mode, otherwise in load hologram
		double _defocusParam[Constants::MAX_WIDEFIELD_WAVELENGTH_COUNT][4]; ///<defocus applied on all holograms, [0]:z defocus in [um], [1]:refractive index, [2]:effective focal length in [mm], [3]: saved defocus value [um]
		long _skipFitting; ///<skip fitting when loading of SLM mask

		//NI:
		static TaskHandle _taskHandleCI; ///<trigger input
		std::string _counterLine; ///<counter for hw trigger input
		std::string _hwTriggerInput; ///<hw trigger input line name

		//winDVI:
		std::wstring _monitorID; ///<monitor id to be used for mask display

		long _dualPatternShiftPx; ///<SLM pattern center offset for dual wavelengths in [Pixel]
		long _persistHologramZone[2]; ///<[0] zone1 left, [1] zone2 right

	public:
		static ThorSLM* getInstance();

		long FindDevices(long& deviceCount); ///<Search system to find meadowlark or other slms
		long SelectDevice(const long device);///<select a particular slm
		long TeardownDevice(); ///<close handles
		long GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault); ///<get the information for each parameter
		long SetParam(const long paramID, const double param);///<set parameter value
		long GetParam(const long paramID, double& param);///<get the parameter value
		long SetParamString(const long paramID, wchar_t* str);
		long GetParamString(const long paramID, wchar_t* str, long size);
		long GetParamBuffer(const long paramID, char* pBuffer, long size);
		long SetParamBuffer(const long paramID, char* pBuffer, long size);
		long PreflightPosition();///<Setup for initial settings, should be called before each experiment, or whenever trigger mode has been changed
		long SetupPosition();///<Setup between frames, only used for software free run mode
		long StartPosition();///<Start a experiment,
		long StatusPosition(long& status);///<Status of a frame 
		long ReadPosition(DeviceType deviceType, double& pos);///<Not available
		long PostflightPosition();
		long GetLastErrorMsg(wchar_t* msg, long size);

		~ThorSLM();

		void SetStatusHandle(HANDLE handle);

	private:
		ThorSLM();
		static int32 CVICALLBACK ThorSLM::HWTriggerCallback(TaskHandle taskHandle, int32 signalID, void* callbackData);
		static int32 ThorSLM::SLMAsync(void* data);
		static void CloseSLMAsync(void);
		static BOOL IsOverdrive();
		void BlankSLM(ISLM::SLMBlank bmode);
		void CloseNITasks();
		long CoordinatesRotate(float* ptArrays, long size, double angle);
		long CoordinatesVerticalFlip(float* ptArrays, long size);
		long CoordinatesScale(float* ptArrays, long size, double scaleX, double scaleY);
		void CopyDefocus(int from, int to);
		long DefocusNormalizeHologram(double defocusZum);
		long LoadHologram();
		double ParseZUM(wstring filename);
		long PersistAffineValues();
		long SaveHologram(bool saveInSubFolder);
		long Save3DHologram(bool doSearch = true, bool reset = true);
		BOOL ReadLUTFile(std::wstring fileName);
		BOOL ReadWavefrontFile(std::wstring fileName);
		BOOL ReadAndScaleBitmap(unsigned char* pImg, BITMAPINFO& bmi);
		void ReleaseMem();
		void ReleaseDVI();
		void SetDefault();
		long ResetSequence(wchar_t* filename = L"");
		long SetupHWTriggerIn();
		unsigned char* MapImageHologram(const wchar_t* pathAndFilename, PBITMAPINFO pbmi);
		unsigned char* MapCalibrateHologram(const wchar_t* pathAndFilename, MemoryStruct<float>* pbuf);
		unsigned char* CropHologramBMP(unsigned char* pImg, float* pS, BITMAPINFO& bmi);
		unsigned char* GetAndProcessBMP(BITMAPINFO& bmi);

	};

#ifdef __cplusplus
}
#endif