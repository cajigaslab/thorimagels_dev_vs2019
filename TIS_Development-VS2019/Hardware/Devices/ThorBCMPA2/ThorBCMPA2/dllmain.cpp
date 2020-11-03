// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorBCMPA2.h"

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
	return ThorBCMPA2::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorBCMPA2::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorBCMPA2::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorBCMPA2::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorBCMPA2::getInstance()->SetParam(paramID,param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return ThorBCMPA2::getInstance()->GetParam(paramID,param);
}

DllExport long PreflightPosition()
{
	return ThorBCMPA2::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return ThorBCMPA2::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return ThorBCMPA2::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return ThorBCMPA2::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return ThorBCMPA2::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return ThorBCMPA2::getInstance()->PostflightPosition();
}

DllExport long GetLastErrorMsg(wchar_t *msg, long size)
{
	return ThorBCMPA2::getInstance()->GetLastErrorMsg(msg,size);
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ThorBCMPA2::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorBCMPA2::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorBCMPA2::getInstance()->SetParamBuffer(paramID,buffer,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorBCMPA2::getInstance()->GetParamBuffer(paramID,buffer,size);
}
