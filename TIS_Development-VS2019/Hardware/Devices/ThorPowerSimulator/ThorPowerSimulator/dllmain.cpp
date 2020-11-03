// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorPowerSimulator.h"


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD ul_reason_for_call,
                       LPVOID lpReserved
                       )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
	}
	break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:

		break;
	}
	return TRUE;
}

DllExport FindDevices(long &deviceCount)
{
	return ThorPowerSimulator::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorPowerSimulator::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorPowerSimulator::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorPowerSimulator::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorPowerSimulator::getInstance()->SetParam(paramID,param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return ThorPowerSimulator::getInstance()->GetParam(paramID,param);
}

DllExport long PreflightPosition()
{
	return ThorPowerSimulator::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return ThorPowerSimulator::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return ThorPowerSimulator::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return ThorPowerSimulator::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return ThorPowerSimulator::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return ThorPowerSimulator::getInstance()->PostflightPosition();
}

DllExport long GetLastErrorMsg(wchar_t *msg, long size)
{
	return ThorPowerSimulator::getInstance()->GetLastErrorMsg(msg,size);
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ThorPowerSimulator::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorPowerSimulator::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorPowerSimulator::getInstance()->SetParamBuffer(paramID,buffer,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorPowerSimulator::getInstance()->GetParamBuffer(paramID,buffer,size);
}