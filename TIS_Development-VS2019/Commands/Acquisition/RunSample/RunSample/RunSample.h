#pragma once

struct RunSampleCommandParams
{
	double version;
	long showDialog;
};

struct RunSampleCustomParams
{
	wchar_t path[256];
};
typedef vector<IDevice*>::iterator DeviceIterator;
typedef vector<ICamera*>::iterator CameraIterator;
typedef vector<ICamera*> Cameras;
typedef vector<IDevice*> Devices;

class RunSample : public ICommand
{
private:
	static bool _instanceFlag;
	static bool _setupFlag;	
	static auto_ptr<RunSample> _single;
	RunSample();
	static GUID _guid;
	RunSampleCommandParams _params;
	RunSampleCustomParams _paramsCustom;
	Observer _ob;

	long InitializeCustomParameters();
	long StopCamera();

public:

	static RunSample* getInstance();

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
	long Stop();
	long Status(long &status);

	void SetMagnification(double mag, string name);
	void SetPinholeWheel(long pinholePosition);
	void SetPockelsMask(ICamera * pCamera, IExperiment* exp, string maskPath);
	void SetLaser(long enable1, double power1,long enable2, double power2,long enable3, double power3,long enable4, double power4, long sequential);
	void RunLaser(IDevice*);
	void SetupZStage(long enableHoldingVoltage);
	void PostCaptureProtocol(IExperiment *exp);
	long GetLSMType(long &lsmType);
	Cameras _cameras;
	CameraIterator _itCam;

	Devices _devices;
	DeviceIterator _itDev;

	static HANDLE _hEventTurret;
	static HANDLE _hEventBeamExpander;
	static HANDLE _hEventPinholeWheel;
	static HANDLE _hEventLaser;
	static HANDLE _hEventPowerReg;
	static HANDLE _hStopStatusThread;

	static bool _isActive;	
	static bool _isSaving;
	const long ENABLE_EMISSION = 1;
	const long DISABLE_EMISSION = 0;

	~RunSample();
};

long GetDeviceParameterValueDouble(IDevice *pDevice,long paramID, double &val);
long GetDeviceParameterValueRangeDouble(IDevice * pDevice,long paramID, double &valMin, double &valMax, double &valDefault);
void GetActiveScanAreaThenEnableAll(IExperiment* exp);
long SetDevicePosition(IDevice *pDevice,long paramID,double pos,BOOL bWait);
long OpenShutter();
long CloseShutter();
long ScannerEnableProc(long cameraOrBleachScanner, long enable);
long SetPMTProc(IExperiment* pExp);
double GetCustomPowerValue(double zStart, double zStop, double zPos, string path);
double GetTiltAdjustZValue(double fPt1X,double fPt1Y,double fPt1Z,double fPt2X,double fPt2Y,double fPt2Z,double fPt3X,double fPt3Y,double fPt3Z,double ptX,double ptY);
void GetZPositions(IExperiment* exp, IDevice* pZStage, double &zStartPos, double &zStopPos, double &zTiltPos, double &zStepSizeMM, long &zstageSteps, long &zStreamFrames, long &zStreamMode);
void SetPower(IExperiment* exp, ICamera* camera, double zPos, double &power0, double &power1, double &power2, double &power3, double &power4, double &power5);
void SetLEDs(IExperiment* exp, ICamera* camera, double zPos, double &ledPower1, double &ledPower2, double &ledPower3, double &ledPower4, double &ledPower5, double &ledPower6);
void PreCaptureProtocol(IExperiment *exp);
UINT SafetyInterlockStatusCheck();
void InitiateSafetyInterlockStatusCheck();
