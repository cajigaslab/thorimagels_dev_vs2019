#pragma once

struct SelectHardwareCommandParams
{
	double version;
	long showDialog;
};

struct SelectHardwareCustomParams
{
	long activeXYStage;
	long activeCamera1;
	long activeZStage;
	long activeZStage2;
	long activeBeamStabilizer;
	long activeEmission;
	long activeDichroic;
	long activeShutter1;
	long activeTurret;
	long activeBFLamp;
	long activeAutoFocus;
	long activeControlUnit;
	long activePMT1;
	long activePMT2;
	long activePMT3;
	long activePMT4;
	long activePowerRegulator;
	long activeBeamExpander;
	long activePinholeWheel;
	long activeLaser1;
	long activeLaser2;
	long activeLaser3;
	long activeLaser4;
	long activeSLM;
	long activeRStage;
	long activePMTSwitch;
	long activeBleachingScanner;
	long activeEphys;
	long activeLightPath;
	long activeSpectrumFilter;
	long activePowerRegulator2;
};

struct CameraInfo
{
	wstring dllName;
	wstring active;
	long camID;
	wstring cameraName;
	wstring serialNumber;
	wstring activation;
	long cameraType;
};

struct DeviceInfo
{
	wstring dllName;
	wstring active;
	long devID;
};


typedef vector<IDevice*>::iterator DeviceIterator;
typedef vector<ICamera*>::iterator CameraIterator;
typedef vector<CameraInfo*>::iterator CameraInfoIterator;
typedef vector<DeviceInfo*>::iterator DeviceInfoIterator;
typedef vector<ICamera*> Cameras;
typedef vector<IDevice*> Devices;
typedef vector<wstring> DeviceDllNames;
typedef vector<CameraInfo*> CameraInfos;
typedef vector<DeviceInfo*> DeviceInfos;

class SelectHardware : public ICommand
{
private:
	static bool instanceFlag;
	static bool _setupFlag;
	static auto_ptr<SelectHardware> _single;
	SelectHardware();
	static GUID guid;
	SelectHardwareCommandParams params;
	SelectHardwareCustomParams paramsCustom;
	multimap<long, vector<wstring>> deviceInfoMap;	//key is device type, value is device dll name and active status
	multimap<long, vector<wstring>> cameraInfoMap; //key is camera type, value is camera dll name and active status

	long InitializeCustomParameters();
	long DisconnectHardware();
	long AddCameraInfoToMap(long cameraType, CameraInfo* camInfo); //add element to cameraInfoMap
	long PersistCameraInfosToMap();
	long AddDeviceInfoToMap(long deviceType, wstring wsDevName, wstring wsActive, wstring wsId); //add element to deviceInfoMap
	long SelectActiveCamera(long camID); //select the active camera by id
	long SelectActiveCamera(long cameraType, int camID); //select the active camera
	long SelectActiveDevice(unsigned long, long); //select the active device specified by device type and id
	long SelectFirstActiveDevice(long devType); //select the first available device
	long SelectCustomCamerasAndDevices(SelectHardwareCustomParams params, bool initialize); //select or update all devices based on input params
	long DeviceExists(long devType, long devID); //check if the device exists in selected device map
	long CameraExists(long devType, long camID); //check if the camera exists in the selected camera map
	long GetDeviceID(string dllName); //get the device ID for the connected device with the matching dllName

public:

	static SelectHardware* getInstance();

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

	long DeslectCameras();
	long GetCameraID();
	long GetBleachID();
	long SetCameraID(long cameraID);
	long GetCameraType(long camID,long &type);
	long GetDeviceID(long deviceType);
	long SetDeviceID(long deviceType, long deviceID);
	long LoadDevice(unsigned long devType, long devIndex);
	long LoadCamera(long cameraIndex);
	long PersistHardwareSetup();
	long ReadHardwareSetupXML();
	long UpdateAndPersistCurrentDevices();
	long SetDetectorName(long detectorID, wchar_t* name);
	long SetActiveBleachingScanner(long cameraIndex);
	long GetCameraParameter(long camID, long paramID, double &param);
	long GetDeviceParamInfo(const long deviceID, const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);

	Cameras cameras;
	CameraIterator itCam;
	CameraInfos cameraInfos;
	CameraInfoIterator itCamInfo;

	Devices devices;
	DeviceIterator itDev;
	DeviceInfos deviceInfos;
	DeviceInfoIterator itDevInfo;

	~SelectHardware()
	{
		instanceFlag = false;
	}
};
