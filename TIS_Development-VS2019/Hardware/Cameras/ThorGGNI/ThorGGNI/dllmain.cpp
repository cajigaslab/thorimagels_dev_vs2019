// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorGGNI.h"

extern "C" BOOL APIENTRY DllMain( HMODULE hModule,	DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		ThorLSMCam::getInstance()->hDLLInstance = hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

DllExport FindCameras(long &cameraCount)
{
	return ThorLSMCam::getInstance()->FindCameras(cameraCount);
}

DllExport SelectCamera(const long camera)
{
	return ThorLSMCam::getInstance()->SelectCamera(camera);
}

DllExport TeardownCamera()
{
	return ThorLSMCam::getInstance()->TeardownCamera();
}

DllExport GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorLSMCam::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport SetParam(const long paramID, const double param)
{
	return ThorLSMCam::getInstance()->SetParam(paramID,param);
}

DllExport GetParam(const long paramID, double &param)
{
	return ThorLSMCam::getInstance()->GetParam(paramID,param);
}

DllExport PreflightAcquisition(char * pDataBuffer)
{
	return ThorLSMCam::getInstance()->PreflightAcquisition(pDataBuffer);
}

DllExport SetupAcquisition(char * pDataBuffer)
{
	return ThorLSMCam::getInstance()->SetupAcquisition(pDataBuffer);
}

DllExport StartAcquisition(char * pDataBuffer)
{
	return ThorLSMCam::getInstance()->StartAcquisition(pDataBuffer);
}

DllExport StatusAcquisition(long &status)
{
	return ThorLSMCam::getInstance()->StatusAcquisition(status);
}

DllExport CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	return ThorLSMCam::getInstance()->CopyAcquisition(pDataBuffer, frameInfo);
}

DllExport PostflightAcquisition(char * pDataBuffer)
{
	return ThorLSMCam::getInstance()->PostflightAcquisition(pDataBuffer);
}

DllExport GetLastErrorMsg(wchar_t * errMsg, long size)
{
	return ThorLSMCam::getInstance()->GetLastErrorMsg(errMsg,size);
}

DllExport SetStatusEvent(HANDLE handle)
{
	ThorLSMCam::getInstance()->SetStatusHandle(handle);
	return TRUE;
}

DllExport StatusAcquisitionEx(long &status, long &indexOfLastCompletedFrame)
{
	return ThorLSMCam::getInstance()->StatusAcquisitionEx(status,indexOfLastCompletedFrame);
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ThorLSMCam::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorLSMCam::getInstance()->GetParamString(paramID,str,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorLSMCam::getInstance()->GetParamBuffer(paramID,buffer,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorLSMCam::getInstance()->SetParamBuffer(paramID,buffer,size);
}