// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorKurios.h"

BOOL APIENTRY DllMain(HMODULE hModule,
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
	return ThorKurios::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorKurios::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorKurios::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorKurios::getInstance()->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorKurios::getInstance()->SetParam(paramID, param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return ThorKurios::getInstance()->GetParam(paramID, param);
}
DllExport long SetParamString(const long paramID, wchar_t * str)
{
	return ThorKurios::getInstance()->SetParamString(paramID, str);
}

DllExport long GetParamString(const long paramID, wchar_t * str, long size)
{
	return ThorKurios::getInstance()->GetParamString(paramID, str, size);
}
DllExport long SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorKurios::getInstance()->SetParamBuffer(paramID, buffer, size);
}

DllExport long GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorKurios::getInstance()->GetParamBuffer(paramID, buffer, size);
}

DllExport long PreflightPosition()
{
	return ThorKurios::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return ThorKurios::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return ThorKurios::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return ThorKurios::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return ThorKurios::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return ThorKurios::getInstance()->PostflightPosition();
}

DllExport long GetLastErrorMsg(wchar_t *msg, long size)
{
	return ThorKurios::getInstance()->GetLastErrorMsg(msg, size);
}