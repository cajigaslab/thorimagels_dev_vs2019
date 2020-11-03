// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorElectroPhys.h"

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
	return ThorElectroPhys::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorElectroPhys::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorElectroPhys::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorElectroPhys::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorElectroPhys::getInstance()->SetParam(paramID,param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return ThorElectroPhys::getInstance()->GetParam(paramID,param);
}

DllExport long PreflightPosition()
{
	return ThorElectroPhys::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return ThorElectroPhys::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return ThorElectroPhys::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return ThorElectroPhys::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return ThorElectroPhys::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return ThorElectroPhys::getInstance()->PostflightPosition();
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ThorElectroPhys::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorElectroPhys::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorElectroPhys::getInstance()->SetParamBuffer(paramID,buffer,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorElectroPhys::getInstance()->GetParamBuffer(paramID,buffer,size);
}

DllExport long GetLastErrorMsg(wchar_t* errorMessage, long size)
{
	return ThorElectroPhys::getInstance()->GetLastErrorMsg(errorMessage, size);
}
