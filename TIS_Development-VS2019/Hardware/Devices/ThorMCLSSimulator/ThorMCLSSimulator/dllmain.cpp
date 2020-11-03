// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorMCLSSimulator.h"

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
	return ThorMCLSSimulator::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorMCLSSimulator::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorMCLSSimulator::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorMCLSSimulator::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorMCLSSimulator::getInstance()->SetParam(paramID,param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return ThorMCLSSimulator::getInstance()->GetParam(paramID,param);
}

DllExport long PreflightPosition()
{
	return ThorMCLSSimulator::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return ThorMCLSSimulator::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return ThorMCLSSimulator::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return ThorMCLSSimulator::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return ThorMCLSSimulator::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return ThorMCLSSimulator::getInstance()->PostflightPosition();
}

DllExport long GetLastErrorMsg(wchar_t *msg, long size)
{
	return ThorMCLSSimulator::getInstance()->GetLastErrorMsg(msg,size);
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ThorMCLSSimulator::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorMCLSSimulator::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorMCLSSimulator::getInstance()->SetParamBuffer(paramID,buffer,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorMCLSSimulator::getInstance()->GetParamBuffer(paramID,buffer,size);
}