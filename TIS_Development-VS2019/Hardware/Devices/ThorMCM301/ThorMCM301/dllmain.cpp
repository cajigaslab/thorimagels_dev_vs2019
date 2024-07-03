// dllmain.cpp : Defines the entry point for the DLL application.

#include "ThorMCM301.h"

#define DllExport extern "C" long __declspec( dllexport )

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

DllExport FindDevices(long& deviceCount)
{
	return MCM301Stage::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return MCM301Stage::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return MCM301Stage::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault)
{
	return MCM301Stage::getInstance()->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return MCM301Stage::getInstance()->SetParam(paramID, param);
}

DllExport long GetParam(const long paramID, double& param)
{
	return MCM301Stage::getInstance()->GetParam(paramID, param);
}

DllExport long PreflightPosition()
{
	return MCM301Stage::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return MCM301Stage::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return MCM301Stage::getInstance()->StartPosition();
}

DllExport long StatusPosition(long& status)
{
	return MCM301Stage::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double& pos)
{
	return MCM301Stage::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return MCM301Stage::getInstance()->PostflightPosition();
}

DllExport SetParamString(long paramID, wchar_t* str)
{
	return MCM301Stage::getInstance()->SetParamString(paramID, str);
}

DllExport GetParamString(const long paramID, wchar_t* str, long size)
{
	return MCM301Stage::getInstance()->GetParamString(paramID, str, size);
}

DllExport SetParamBuffer(const long paramID, char* buffer, long size)
{
	return MCM301Stage::getInstance()->SetParamBuffer(paramID, buffer, size);
}

DllExport GetParamBuffer(const long paramID, char* buffer, long size)
{
	return MCM301Stage::getInstance()->GetParamBuffer(paramID, buffer, size);
}