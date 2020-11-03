// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorMTS25.h"

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
	return ThorMTS25::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorMTS25::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorMTS25::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorMTS25::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorMTS25::getInstance()->SetParam(paramID,param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return ThorMTS25::getInstance()->GetParam(paramID,param);
}

DllExport long PreflightPosition()
{
	return ThorMTS25::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return ThorMTS25::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return ThorMTS25::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return ThorMTS25::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return ThorMTS25::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return ThorMTS25::getInstance()->PostflightPosition();
}

DllExport long GetLastErrorMsg(wchar_t *msg, long size)
{
	return ThorMTS25::getInstance()->GetLastErrorMsg(msg,size);
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ThorMTS25::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorMTS25::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorMTS25::getInstance()->SetParamBuffer(paramID,buffer,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorMTS25::getInstance()->GetParamBuffer(paramID,buffer,size);
}