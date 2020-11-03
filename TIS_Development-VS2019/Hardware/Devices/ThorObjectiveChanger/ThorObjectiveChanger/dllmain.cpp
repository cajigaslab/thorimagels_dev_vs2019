// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorObjectiveChanger.h"

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
	return ThorObjectiveChanger::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorObjectiveChanger::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorObjectiveChanger::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorObjectiveChanger::getInstance()->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorObjectiveChanger::getInstance()->SetParam(paramID, param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return ThorObjectiveChanger::getInstance()->GetParam(paramID, param);
}
DllExport long SetParamString(const long paramID, wchar_t * str)
{
	return ThorObjectiveChanger::getInstance()->SetParamString(paramID, str);
}

DllExport long GetParamString(const long paramID, wchar_t * str, long size)
{
	return ThorObjectiveChanger::getInstance()->GetParamString(paramID, str, size);
}
DllExport long SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorObjectiveChanger::getInstance()->SetParamBuffer(paramID, buffer, size);
}

DllExport long GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorObjectiveChanger::getInstance()->GetParamBuffer(paramID, buffer, size);
}

DllExport long PreflightPosition()
{
	return ThorObjectiveChanger::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return ThorObjectiveChanger::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return ThorObjectiveChanger::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return ThorObjectiveChanger::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return ThorObjectiveChanger::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return ThorObjectiveChanger::getInstance()->PostflightPosition();
}

DllExport long GetLastErrorMsg(wchar_t *msg, long size)
{
	return ThorObjectiveChanger::getInstance()->GetLastErrorMsg(msg, size);
}
DllExport long GetSerialNumberString(const long paramID, wchar_t * str, long size)
{
	return ThorObjectiveChanger::getInstance()->GetSerialNumberString(paramID, str, size);
}