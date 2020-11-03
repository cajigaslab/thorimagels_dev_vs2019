#include "stdafx.h"
#include "PIPiezoXYZ.h"

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
	return PIPiezoXYZ::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return PIPiezoXYZ::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return PIPiezoXYZ::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return PIPiezoXYZ::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return PIPiezoXYZ::getInstance()->SetParam(paramID,param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return PIPiezoXYZ::getInstance()->GetParam(paramID,param);
}

DllExport long PreflightPosition()
{
	return PIPiezoXYZ::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return PIPiezoXYZ::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return PIPiezoXYZ::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return PIPiezoXYZ::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return PIPiezoXYZ::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return PIPiezoXYZ::getInstance()->PostflightPosition();
}

DllExport long GetLastErrorMsg(wchar_t *msg, long size)
{
	return PIPiezoXYZ::getInstance()->GetLastErrorMsg(msg,size);
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return PIPiezoXYZ::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return PIPiezoXYZ::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return PIPiezoXYZ::getInstance()->SetParamBuffer(paramID,buffer,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return PIPiezoXYZ::getInstance()->GetParamBuffer(paramID,buffer,size);
}

