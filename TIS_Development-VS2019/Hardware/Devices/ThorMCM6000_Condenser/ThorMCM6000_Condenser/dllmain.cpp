// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorMCM6000_Condenser.h"

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
	return ThorMCM6000_Condenser::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorMCM6000_Condenser::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorMCM6000_Condenser::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorMCM6000_Condenser::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorMCM6000_Condenser::getInstance()->SetParam(paramID,param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return ThorMCM6000_Condenser::getInstance()->GetParam(paramID,param);
}

DllExport long PreflightPosition()
{
	return ThorMCM6000_Condenser::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return ThorMCM6000_Condenser::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return ThorMCM6000_Condenser::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return ThorMCM6000_Condenser::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return ThorMCM6000_Condenser::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return ThorMCM6000_Condenser::getInstance()->PostflightPosition();
}

DllExport long GetLastErrorMsg(wchar_t *msg, long size)
{
	return ThorMCM6000_Condenser::getInstance()->GetLastErrorMsg(msg,size);
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ThorMCM6000_Condenser::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorMCM6000_Condenser::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorMCM6000_Condenser::getInstance()->SetParamBuffer(paramID,buffer,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorMCM6000_Condenser::getInstance()->GetParamBuffer(paramID,buffer,size);
}