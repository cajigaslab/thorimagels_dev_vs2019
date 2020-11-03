// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "DisconnectedDevice.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

DllExport FindDevices(long &deviceCount)
{
	return DisconnectedDevice::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return DisconnectedDevice::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return DisconnectedDevice::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return DisconnectedDevice::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return DisconnectedDevice::getInstance()->SetParam(paramID,param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return DisconnectedDevice::getInstance()->GetParam(paramID,param);
}

DllExport long PreflightPosition()
{
	return DisconnectedDevice::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return DisconnectedDevice::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return DisconnectedDevice::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return DisconnectedDevice::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return DisconnectedDevice::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return DisconnectedDevice::getInstance()->PostflightPosition();
}

DllExport long GetLastErrorMsg(wchar_t *msg,long size)
{
	return DisconnectedDevice::getInstance()->PostflightPosition();
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return DisconnectedDevice::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return DisconnectedDevice::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return DisconnectedDevice::getInstance()->SetParamBuffer(paramID,buffer,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return DisconnectedDevice::getInstance()->GetParamBuffer(paramID,buffer,size);
}