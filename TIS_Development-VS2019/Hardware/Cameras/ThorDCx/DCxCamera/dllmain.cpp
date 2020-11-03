// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "CameraManager.h"

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
	return CameraManager::getInstance()->FindCameras(cameraCount);
}

DllExport long SelectCamera(const long camera)
{
	return CameraManager::getInstance()->SelectCamera(camera);
}
DllExport long TeardownCamera()
{
	return CameraManager::getInstance()->TeardownCamera();
}

DllExport long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return CameraManager::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport long SetParam(const long paramID, const double param)
{
	return CameraManager::getInstance()->SetParam(paramID,param);
}

DllExport long SetParamWithChannel(const long paramID, const double param, const long channelID)
{
	return CameraManager::getInstance()->SetParam(paramID,param,channelID);
}

DllExport long GetParam(const long paramID, double &param)
{
	return CameraManager::getInstance()->GetParam(paramID,param);
}

DllExport long GetParamWithChannel(const long paramID, double &param, const long channelID)
{
	return CameraManager::getInstance()->GetParam(paramID,param,channelID);
}

DllExport long PreflightAcquisition(char * pDataBuffer)
{
	return CameraManager::getInstance()->PreflightAcquisition(pDataBuffer);
}

DllExport long SetupAcquisition(char * pDataBuffer)
{
	return CameraManager::getInstance()->SetupAcquisition(pDataBuffer);
}

DllExport long StartAcquisition(char * pDataBuffer)
{
	return CameraManager::getInstance()->StartAcquisition(pDataBuffer);
}

DllExport long StatusAcquisition(long &status)
{
	return CameraManager::getInstance()->StatusAcquisition(status);
}

DllExport long StatusAcquisitionEx(long &status,long &indexOfLastFrame)
{
	return CameraManager::getInstance()->StatusAcquisitionEx(status,indexOfLastFrame);
}

DllExport long CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	return CameraManager::getInstance()->CopyAcquisition(pDataBuffer, frameInfo);
}

DllExport long PostflightAcquisition(char * pDataBuffer)
{
	return CameraManager::getInstance()->PostflightAcquisition(pDataBuffer);
}

DllExport long GetLastErrorMsg(wchar_t * errMsg, long size)
{
	return TRUE;
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return CameraManager::getInstance()->SetParamString(paramID, str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return CameraManager::getInstance()->GetParamString(paramID, str, size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return TRUE;
}
DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return TRUE;
}