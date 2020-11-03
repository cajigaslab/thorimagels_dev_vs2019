// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorPowerControl.h"

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
	return ThorPowerControl::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorPowerControl::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorPowerControl::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorPowerControl::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorPowerControl::getInstance()->SetParam(paramID,param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return ThorPowerControl::getInstance()->GetParam(paramID,param);
}

DllExport long PreflightPosition()
{
	return ThorPowerControl::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return ThorPowerControl::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return ThorPowerControl::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return ThorPowerControl::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return ThorPowerControl::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return ThorPowerControl::getInstance()->PostflightPosition();
}

DllExport long GetLastErrorMsg(wchar_t *msg, long size)
{
	return ThorPowerControl::getInstance()->GetLastErrorMsg(msg,size);
}
