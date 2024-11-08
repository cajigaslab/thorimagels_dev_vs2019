// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "ThorChrolis.h"

BOOL APIENTRY DllMain ( HMODULE hModule,
						DWORD  ul_reason_for_call,
						LPVOID lpReserved )
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


// Dvice Handling
DllExport FindDevices(long &deviceCount)
{
	return CThorChrolis::getInstance()->FindDevices(deviceCount);
}

DllExport SelectDevice(const long device)
{
	return CThorChrolis::getInstance()->SelectDevice(device);
}
DllExport TeardownDevice()
{
	return CThorChrolis::getInstance()->TeardownDevice();
}


// Parameter Handling
DllExport GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return CThorChrolis::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport SetParam(const long paramID, const double param)
{
	return CThorChrolis::getInstance()->SetParam(paramID,param);
}

DllExport GetParam(const long paramID, double &param)
{
	return CThorChrolis::getInstance()->GetParam(paramID,param);
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return CThorChrolis::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return CThorChrolis::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return CThorChrolis::getInstance()->SetParamBuffer(paramID,buffer,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return CThorChrolis::getInstance()->GetParamBuffer(paramID,buffer,size);
}


// Error Handling
DllExport GetLastErrorMsg(wchar_t * msg, long size)
{
	return CThorChrolis::getInstance()->GetLastErrorMsg(msg, size);
}


// Setup, Start, Status
DllExport SetupPosition()
{
	return CThorChrolis::getInstance()->SetupPosition();
}

DllExport StartPosition()
{
	return CThorChrolis::getInstance()->StartPosition();
}

DllExport StatusPosition(long &status)
{
	return CThorChrolis::getInstance()->StatusPosition(status);
}

// Called before Setup
DllExport PreflightPosition()
{
	return CThorChrolis::getInstance()->PreflightPosition();
}

// Normally not used
DllExport ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return CThorChrolis::getInstance()->ReadPosition(deviceType, pos);
}

// Called after Start Acquisition
DllExport PostflightPosition()
{
	return CThorChrolis::getInstance()->PostflightPosition();
}
