// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorZStepperSimulator.h"

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
	return ThorZStepperSimulator::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorZStepperSimulator::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorZStepperSimulator::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorZStepperSimulator::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorZStepperSimulator::getInstance()->SetParam(paramID,param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return ThorZStepperSimulator::getInstance()->GetParam(paramID,param);
}

DllExport long PreflightPosition()
{
	return ThorZStepperSimulator::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return ThorZStepperSimulator::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return ThorZStepperSimulator::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return ThorZStepperSimulator::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return ThorZStepperSimulator::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return ThorZStepperSimulator::getInstance()->PostflightPosition();
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ThorZStepperSimulator::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorZStepperSimulator::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorZStepperSimulator::getInstance()->SetParamBuffer(paramID,buffer,size);
}
DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorZStepperSimulator::getInstance()->GetParamBuffer(paramID,buffer,size);
}