// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorMCM3000Aux.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD ul_reason_for_call,
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
	return ThorMCM3000Aux::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorMCM3000Aux::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorMCM3000Aux::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorMCM3000Aux::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorMCM3000Aux::getInstance()->SetParam(paramID,param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return ThorMCM3000Aux::getInstance()->GetParam(paramID,param);
}

DllExport long PreflightPosition()
{
	return ThorMCM3000Aux::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return ThorMCM3000Aux::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return ThorMCM3000Aux::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return ThorMCM3000Aux::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return ThorMCM3000Aux::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return ThorMCM3000Aux::getInstance()->PostflightPosition();
}

DllExport long GetLastErrorMsg(wchar_t *msg, long size)
{
	return ThorMCM3000Aux::getInstance()->GetLastErrorMsg(msg,size);
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ThorMCM3000Aux::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorMCM3000Aux::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorMCM3000Aux::getInstance()->SetParamBuffer(paramID,buffer,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorMCM3000Aux::getInstance()->GetParamBuffer(paramID,buffer,size);
}