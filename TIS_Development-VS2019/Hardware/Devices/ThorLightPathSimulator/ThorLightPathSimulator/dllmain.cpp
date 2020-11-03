// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorLightPathSimulator.h"

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
	return ThorLightPathSimulator::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorLightPathSimulator::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorLightPathSimulator::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorLightPathSimulator::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorLightPathSimulator::getInstance()->SetParam(paramID,param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return ThorLightPathSimulator::getInstance()->GetParam(paramID,param);
}

DllExport long PreflightPosition()
{
	return ThorLightPathSimulator::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return ThorLightPathSimulator::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return ThorLightPathSimulator::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return ThorLightPathSimulator::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return ThorLightPathSimulator::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return ThorLightPathSimulator::getInstance()->PostflightPosition();
}

DllExport long GetLastErrorMsg(wchar_t *msg, long size)
{
	return ThorLightPathSimulator::getInstance()->GetLastErrorMsg(msg,size);
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ThorLightPathSimulator::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorLightPathSimulator::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorLightPathSimulator::getInstance()->SetParamBuffer(paramID,buffer,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorLightPathSimulator::getInstance()->GetParamBuffer(paramID,buffer,size);
}