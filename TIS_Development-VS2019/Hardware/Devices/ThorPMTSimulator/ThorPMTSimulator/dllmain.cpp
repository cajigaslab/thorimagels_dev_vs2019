// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorPMTSimulator.h"

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
	return ThorPMTSimulator::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorPMTSimulator::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorPMTSimulator::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorPMTSimulator::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorPMTSimulator::getInstance()->SetParam(paramID,param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return ThorPMTSimulator::getInstance()->GetParam(paramID,param);
}

DllExport long PreflightPosition()
{
	return ThorPMTSimulator::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return ThorPMTSimulator::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return ThorPMTSimulator::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return ThorPMTSimulator::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return ThorPMTSimulator::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return ThorPMTSimulator::getInstance()->PostflightPosition();
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ThorPMTSimulator::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorPMTSimulator::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorPMTSimulator::getInstance()->SetParamBuffer(paramID,buffer,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorPMTSimulator::getInstance()->GetParamBuffer(paramID,buffer,size);
}

