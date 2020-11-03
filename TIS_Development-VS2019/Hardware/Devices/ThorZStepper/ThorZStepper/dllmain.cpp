// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorZStepper.h"

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
	return ThorZStepper::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorZStepper::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorZStepper::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorZStepper::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorZStepper::getInstance()->SetParam(paramID,param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return ThorZStepper::getInstance()->GetParam(paramID,param);
}

DllExport long PreflightPosition()
{
	return ThorZStepper::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return ThorZStepper::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return ThorZStepper::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return ThorZStepper::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return ThorZStepper::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return ThorZStepper::getInstance()->PostflightPosition();
}

DllExport long GetLastErrorMsg(wchar_t * msg, long size)
{
	return ThorZStepper::getInstance()->GetLastErrorMsg( msg, size);
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ThorZStepper::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorZStepper::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorZStepper::getInstance()->SetParamBuffer(paramID,buffer,size);
}
DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorZStepper::getInstance()->GetParamBuffer(paramID,buffer,size);
}