// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorPinholeStepperSimulator.h"

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
	return ThorPinholeStepperSimulator::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorPinholeStepperSimulator::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorPinholeStepperSimulator::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorPinholeStepperSimulator::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorPinholeStepperSimulator::getInstance()->SetParam(paramID,param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return ThorPinholeStepperSimulator::getInstance()->GetParam(paramID,param);
}

DllExport long PreflightPosition()
{
	return ThorPinholeStepperSimulator::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return ThorPinholeStepperSimulator::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return ThorPinholeStepperSimulator::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return ThorPinholeStepperSimulator::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return ThorPinholeStepperSimulator::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return ThorPinholeStepperSimulator::getInstance()->PostflightPosition();
}

DllExport long GetLastErrorMsg(wchar_t * msg, long size)
{
	return ThorPinholeStepperSimulator::getInstance()->GetLastErrorMsg( msg, size);
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ThorPinholeStepperSimulator::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorPinholeStepperSimulator::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorPinholeStepperSimulator::getInstance()->SetParamBuffer(paramID,buffer,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorPinholeStepperSimulator::getInstance()->GetParamBuffer(paramID,buffer,size);
}