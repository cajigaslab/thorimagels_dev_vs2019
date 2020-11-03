// dllmain.cpp : Defines the entry point for the DLL application.
#include "ThorTSI_CS.h"

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
	return ThorCam::getInstance()->FindCameras(cameraCount);
}

DllExport SelectCamera(const long camera)
{
	return ThorCam::getInstance()->SelectCamera(camera);
}

DllExport TeardownCamera()
{
	return ThorCam::getInstance()->TeardownCamera();
}

DllExport GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorCam::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport SetParam(const long paramID, const double param)
{
	return ThorCam::getInstance()->SetParam(paramID,param);
}

DllExport GetParam(const long paramID, double &param)
{
	return ThorCam::getInstance()->GetParam(paramID,param);
}

DllExport PreflightAcquisition(char * pDataBuffer)
{
	return ThorCam::getInstance()->PreflightAcquisition(pDataBuffer);
}

DllExport SetupAcquisition(char * pDataBuffer)
{
	return ThorCam::getInstance()->SetupAcquisition(pDataBuffer);
}

DllExport StartAcquisition(char * pDataBuffer)
{
	return ThorCam::getInstance()->StartAcquisition(pDataBuffer);
}

DllExport StatusAcquisition(long &status)
{
	return ThorCam::getInstance()->StatusAcquisition(status);
}

DllExport CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	return ThorCam::getInstance()->CopyAcquisition(pDataBuffer, frameInfo);
}

DllExport PostflightAcquisition(char * pDataBuffer)
{
	return ThorCam::getInstance()->PostflightAcquisition(pDataBuffer);
}

DllExport GetLastErrorMsg(wchar_t * errMsg, long size)
{
	return ThorCam::getInstance()->GetLastErrorMsg(errMsg,size);
}

DllExport StatusAcquisitionEx(long &status, long &indexOfLastCompletedFrame)
{
	return ThorCam::getInstance()->StatusAcquisitionEx(status,indexOfLastCompletedFrame);
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ThorCam::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorCam::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorCam::getInstance()->SetParamBuffer(paramID,buffer,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorCam::getInstance()->GetParamBuffer(paramID,buffer,size);
}