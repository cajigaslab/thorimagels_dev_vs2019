#pragma once
#include "..\..\..\Common\ThorSharedTypesCPP.h"
#include "IImageRoutine.h"

#define DSTYPE_PASSTHROUGH 0
#define	DSTYPE_SQLITE  1
#define	DSTYPE_HDF5  2
#define MAX_BLEACH_PARAMS_CNT 1

struct LongParam
{
	long val;
	long alias;
	long useAlias;
};

struct DoubleParam
{
	double val;
	long alias;
	long useAlias;
};

struct CaptureSetupCommandParams
{
	double version;
	long showDialog;
};

struct CaptureSetupCustomParams
{
	LongParam left;
	LongParam right;
	LongParam top;
	LongParam bottom;
	LongParam binX;
	LongParam binY;
	DoubleParam exposureTime0;
	DoubleParam exposureTime1;
	DoubleParam exposureTime2;
	LongParam lightMode;
	LongParam gain;
	LongParam lsmScanMode;
	LongParam lsmPixelX;
	LongParam lsmPixelY;
	LongParam lsmFieldSize;
	LongParam lsmChannel;	
	LongParam lsmAverageMode;
	LongParam lsmAverageNum;
	LongParam lsmInputRangeChannel1;
	LongParam lsmInputRangeChannel2;
	LongParam lsmInputRangeChannel3;
	LongParam lsmInputRangeChannel4;
	LongParam lsmFieldOffsetX;
	LongParam lsmFieldOffsetY;
	LongParam lsmTwoWayAlignment;  
	LongParam lsmClockSource;
	LongParam lsmExtClockRate;
	LongParam lsmAreaMode;
};

struct CaptureSetupZCaptureParams
{
	double start;
	double stepSize;
	long numOfSteps;
};

struct AutoFocusCaptureParams
{
	double magnification;
	long autoFocusType;
};

typedef vector<IDevice*>::iterator DeviceIterator;
typedef vector<ICamera*>::iterator CameraIterator;
typedef vector<ICamera*> Cameras;
typedef vector<IDevice*> Devices;


class CaptureSetup : public ICommand
{
private:

	static bool _instanceFlag;
	static bool _setupFlag;
	static auto_ptr<CaptureSetup> _single;
	static GUID _guid;

	CaptureSetupCommandParams _params;
	CaptureSetupCustomParams _paramsCustom;

	double _coefficientK1;
	double _coefficientK2;
	double _coefficientK3;
	double _coefficientK4;
	double _frameRate;
	long _displayChannels;

	IImageRoutine * _pImageRoutines[MAX_IMAGE_ROUTINES];

	CaptureSetup();
	long FindHardware();

public:

	~CaptureSetup()
	{
		_instanceFlag = false;
	}

	IImageRoutine * _pActiveImageRoutine;

	static CaptureSetup* getInstance();
	static CritSect critSect;
	static CritSect critSectLiveImage;
	static std::string ConvertWStringToString(std::wstring ws);	
	static const long MAX_NUM_CHANNEL = 4;
	static long _channelEnable;
	static long _autoDisplayChannel;

	long GetCommandGUID(GUID * guid);
	long SetupCommand();
	long TeardownCommand();
	long GetParamInfo(const long paramID, long &paramType, long& paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double &param);
	long SetCustomParamsBinary(const char *buf);
	long GetCustomParamsBinary(char *buf);
	long SaveCustomParamsXML(void *fileHandle);
	long LoadCustomParamXML(void *fileHandle);
	long Execute();
	long Status(long &status);
	long Stop();
	long SetPincushionCoefficients(double k1,double k2,double k3,double k4);
	long GetPincushionCoefficients(double &k1,double &k2,double &k3,double &k4);
	long SetImageCorrectionEnable(int enable);
	double GetFrameRate();
	void SetFrameRate(double val);
	bool GetSetupFlagState();
	long SetDisplayChannels(int channelEnable);
	long ImgProGenConf(int maxRoiNum, int minSnr);
	long EnableMinAreaFilter(bool minAreaActive, int minAreaValue);
	void ClearBleachParams(int id){if(bleachParams[id] != NULL){if(bleachParams[id]->analogPockelSize > 0){free(bleachParams[id]->GalvoWaveformPockel);}if(bleachParams[id]->analogXYSize > 0){free(bleachParams[id]->GalvoWaveformXY);}if(bleachParams[id]->digitalSize > 0){free(bleachParams[id]->DigBufWaveform);}}}

	GGalvoWaveformParams* bleachParams[MAX_BLEACH_PARAMS_CNT];

	//events
	HANDLE _hEventAutoFocus;
	HANDLE _hEventEx;
	HANDLE _hEventDic;
	HANDLE _hEventEm;
	HANDLE _hEventTurret;
	HANDLE _hEventZ;
	HANDLE _hEventPower;
	HANDLE _hEventBeamExpander;
	HANDLE _hEventLaser;

	long _statusError[3];
	long _enablePincushionCorrection;
	long _enableBackgroundSubtraction;
	long _enableFlatField;
	long _enableROIStats;

	wstring _fileBackgroundSubtraction;
	wstring _fileFlatField;	
};


class CorrectionImage
{
public:
	CorrectionImage();
	~CorrectionImage();

	long Create(wstring ws, long w, long h, long c);

	double GetMeanIntensity();
	long GetWidth();
	long GetHeight();
	long GetColorChannels();
	unsigned short * GetBuffer();

private:

	unsigned short *_pBuffer;
	long _width;
	long _height;
	long _colorChannels;
	double _meanIntensity;
};

long ParseLSMChannels(int lsmChan, vector<int>* enabledChannels, vector<wstring>* chanNames, wstring basename);
void ImageCorrections(char * pBuffer, long width, long height, long channels, int enablePincushion, int enableBackgroundSubtraction, int enableFlatField, double k1,double k2, double k3, double k4);
long SaveMultiPageTIFF(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut, double umPerPixel,int nc, int nt, int nz, double timeIncrement, int t, int z,string *acquiredDate,double dt, string * omeTiffData,long doCompression);
long SaveTIFF(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut, double umPerPixel,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string *acquiredDate,double dt, string * omeTiffData,long doCompression);
void ResizeImage(char * pSource, long srcWidth, long srcHeight, char * pDestination, long dstWidth, long dstHeight);

void GetTIFFConfiguration(long &OME_Enabled, long &TiFF_Compression_Enabled);
void PreflightPMT();
void PostflightPMT();
long SetupBuffers(long colorChans);
UINT LiveThreadProc( LPVOID pParam );
DllExportLiveImage SimplifiedSaveTIFFs(wchar_t* destinationPath, wchar_t* baseName, char* imageMemory, int enabledChannelsBitmask, int saveMultiPage);
string uUIDSetup(vector<wstring> wavelengthNames,bool appendName, long bufferChannels, long zstageSteps, long index, long subWell);
DllExportLiveImage StartLiveCapture();
DllExportLiveImage StopLiveCapture();
DllExportLiveImage SetShutterPosition(long pos);

long StatusHandlerDevice(IDevice * pDevice, HANDLE *pEvent, long maxWaitTime);
DllExportLiveImage SetPMTScannerEnable(long enable);
DllExportLiveImage GetTurretPosition(long &pos);
DllExportLiveImage GetFieldSize(long &fieldSize);
DllExportLiveImage SetZPosition(double pos);
DllExportLiveImage SetBFLampPosition(long pos);
DllExportLiveImage SetLaser1Emission(long pos);
DllExportLiveImage SetLaser2Emission(long pos);
DllExportLiveImage SetLaser3Emission(long pos);
DllExportLiveImage SetLaser4Emission(long pos);
DllExportLiveImage GetLaserAnalog(long &pos);
DllExportLiveImage GetLaser1Enable(long &pos);
DllExportLiveImage GetLaser2Enable(long &pos);
DllExportLiveImage GetLaser3Enable(long &pos);
DllExportLiveImage GetLaser4Enable(long &pos);

#define CHECK_PACTIVEIMAGEROUTINE(x) 	if(NULL != CaptureSetup::getInstance()->_pActiveImageRoutine)\
{\
	return CaptureSetup::getInstance()->_pActiveImageRoutine->x;\
}\
										else\
{\
	return FALSE;\
}

#define CHECK_INLINE_PACTIVEIMAGEROUTINE(x) 	if(NULL != CaptureSetup::getInstance()->_pActiveImageRoutine)\
{\
	CaptureSetup::getInstance()->_pActiveImageRoutine->x;\
}

#define CHECK_INEXPRESSION_PACTIVEIMAGEROUTINE(x, y) 	if(NULL != CaptureSetup::getInstance()->_pActiveImageRoutine)\
{\
	y = CaptureSetup::getInstance()->_pActiveImageRoutine->x;\
}

extern const long MSG_SIZE;
extern char * pMemoryBuffer;

extern BOOL stopBleach;
extern HANDLE hStatusBleachScanner;
extern DWORD dwLiveThreadId;
extern HANDLE hLiveThread;
extern DWORD dwZStackCaptureThreadId;
extern HANDLE hZStackCaptureThread;
extern DWORD dwBleachThreadId;
extern HANDLE hBleachThread;
extern DWORD _dwAutoFocusCaptureThreadId;
extern HANDLE _hAutoFocusCaptureThread;
extern DWORD _dwAutoFocusStatusThreadId;
extern HANDLE _hAutoFocusStatusThread;
extern DWORD _dwSafetyInterLockCheckThreadId;
extern HANDLE _hSafetyInterLockCheckThread;
extern HANDLE hStatusEvent[3];
extern HANDLE hCaptureActive;
extern BOOL stopCapture;
extern HANDLE hEventBleach;
extern BOOL inFileLoading;
extern BOOL activeBleach;
extern BOOL InterruptCapture;
extern long _autoDisplayChannel;
extern unsigned long long imageBufferSize;
extern FrameInfoStruct imageInfo;
extern long disableZRead;
extern DWORD dwBleachThreadId;
extern char * pChan[32];
extern auto_ptr<CommandDll> shwDll;
extern std::atomic<BOOL> _shutterOpened;

const long SHUTTER_OPEN = 0;
const long SHUTTER_CLOSE = 1;

const long DISABLE_LEDS = 0;
const long ENABLE_LEDS = 1;

const long DISABLE_EMISSION = 0;
const long ENABLE_EMISSION = 1;

