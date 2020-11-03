// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorMLSStage.h"


BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
		HINSTANCE hinst = GetModuleHandle(NULL);
		ThorMLSStage::getInstance()->SetHInstance(hinst);
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
	return ThorMLSStage::getInstance()->FindDevices(deviceCount);
}

DllExport long SelectDevice(const long Device)
{
	return ThorMLSStage::getInstance()->SelectDevice(Device);
}
DllExport long TeardownDevice()
{
	return ThorMLSStage::getInstance()->TeardownDevice();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorMLSStage::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorMLSStage::getInstance()->SetParam(paramID,param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return ThorMLSStage::getInstance()->GetParam(paramID,param);
}

DllExport long PreflightPosition()
{
	return ThorMLSStage::getInstance()->PreflightPosition();
}

DllExport long SetupPosition()
{
	return ThorMLSStage::getInstance()->SetupPosition();
}

DllExport long StartPosition()
{
	return ThorMLSStage::getInstance()->StartPosition();
}

DllExport long StatusPosition(long &status)
{
	return ThorMLSStage::getInstance()->StatusPosition(status);
}

DllExport long ReadPosition(IDevice::DeviceType deviceType, double &pos)
{
	return ThorMLSStage::getInstance()->ReadPosition(deviceType, pos);
}

DllExport long PostflightPosition()
{
	return ThorMLSStage::getInstance()->PostflightPosition();
}

DllExport long WaitUntilSettled ()
{
	return ThorMLSStage::getInstance()->WaitUntilSettled ();
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ThorMLSStage::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorMLSStage::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorMLSStage::getInstance()->SetParamBuffer(paramID,buffer,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorMLSStage::getInstance()->GetParamBuffer(paramID,buffer,size);
}