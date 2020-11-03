// HardwareCom.h
#pragma once
#include "..\..\..\Common\ThorSharedTypesCPP.h"
class HardwareCom
{

private:
	static bool _instanceFlag;
	static auto_ptr<HardwareCom> _single;
	HardwareCom();
	long ExecuteDevice(IDevice *pDevice, long maxWaitTime, long exeOrWait);
	long StatusHandlerDevice(IDevice * pDevice, HANDLE *pEvent, long maxWaitTime);

public:
	static HardwareCom* getInstance();
	IDevice* GetDevice(long deviceSelection);
	ICamera* GetCamera(long CameraSelection);
	long SetDeviceParameterValue(IDevice *pDevice, long paramID, double val, long exeOrWait, long waitTime);
	long SetDeviceParameterBuffer(IDevice *pDevice, long paramID, char* buf, long len, long exeOrWait, long waitTime);
	long SetDeviceParameterString(IDevice *pDevice, long paramID, wchar_t* str, long exeOrWait, long waitTime);
	long GetDeviceParameterAvailable(IDevice *pDevice,long paramID);
	long GetDeviceParameterReadOnly(IDevice *pDevice,long paramID);
	long GetDeviceParameterValue(IDevice *pDevice,long paramID, double &val);
	long GetDeviceParameterBuffer(IDevice *pDevice,long paramID, char* buf, long len);
	long GetDeviceParameterString(IDevice *pDevice, long paramID, wchar_t* str, long size);
	long GetDeviceParameterValueRange(IDevice * pDevice,long paramID, double &valMin, double &valMax, double &valDefault);
	long GetCameraParameterValueDouble(long paramID, double &val);
	long GetCameraParameterValueRange(long paramID, double &valMin, double &valMax, double &valDefault);
	long GetCameraParameterValueLong(long paramID, long &val);
	long SetCameraParameterValue(long paramID, double val);
	long GetDeviceSelectedIndex(long deviceSelection);
	long GetCameraSelectedIndex(long cameraSelection);
	~HardwareCom();
};
extern auto_ptr<CommandDll> shwDll;


