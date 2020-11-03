// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorConfocalSimulator.h"

HINSTANCE hDLLInstance;

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

		hDLLInstance = hModule;
		break;
	}
	return TRUE;
}


DllExport FindCameras(long &cameraCount)
{
	return ThorLSMCam::getInstance()->FindCameras(cameraCount);
}

DllExport long SelectCamera(const long camera)
{
	return ThorLSMCam::getInstance()->SelectCamera(camera);
}
DllExport long TeardownCamera()
{
	return ThorLSMCam::getInstance()->TeardownCamera();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorLSMCam::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return ThorLSMCam::getInstance()->SetParam(paramID,param);
}

DllExport long GetParam(const long paramID, double &param)
{
	return ThorLSMCam::getInstance()->GetParam(paramID,param);
}

DllExport long PreflightAcquisition(char * pDataBuffer)
{
	return ThorLSMCam::getInstance()->PreflightAcquisition(pDataBuffer);
}

DllExport long SetupAcquisition(char * pDataBuffer)
{
	return ThorLSMCam::getInstance()->SetupAcquisition(pDataBuffer);
}

DllExport long StartAcquisition(char * pDataBuffer)
{
	return ThorLSMCam::getInstance()->StartAcquisition(pDataBuffer);
}

DllExport long StatusAcquisition(long &status)
{
	return ThorLSMCam::getInstance()->StatusAcquisition(status);
}

DllExport long StatusAcquisitionEx(long &status,long &indexOfLastFrame)
{
	return ThorLSMCam::getInstance()->StatusAcquisitionEx(status,indexOfLastFrame);
}

DllExport long CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	return ThorLSMCam::getInstance()->CopyAcquisition(pDataBuffer, frameInfo);
}

DllExport long PostflightAcquisition(char * pDataBuffer)
{
	return ThorLSMCam::getInstance()->PostflightAcquisition(pDataBuffer);
}

DllExport long GetLastErrorMsg(wchar_t * errMsg, long size)
{
	return TRUE;
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return FALSE;
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorLSMCam::getInstance()->GetParamString(paramID, str, size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorLSMCam::getInstance()->SetParamBuffer(paramID, buffer, size);
}
DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return TRUE;
}