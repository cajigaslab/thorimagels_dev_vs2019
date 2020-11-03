// dllmain.cpp : Defines the entry point for the DLL application.
#include "ORCA.h"

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
	return ORCA::getInstance()->FindCameras(cameraCount);
}

DllExport SelectCamera(const long camera)
{
	return ORCA::getInstance()->SelectCamera(camera);
}

DllExport TeardownCamera()
{
	return ORCA::getInstance()->TeardownCamera();
}

DllExport GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ORCA::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport SetParam(const long paramID, const double param)
{
	return ORCA::getInstance()->SetParam(paramID,param);
}

DllExport GetParam(const long paramID, double &param)
{
	return ORCA::getInstance()->GetParam(paramID,param);
}

DllExport PreflightAcquisition(char * pDataBuffer)
{
	return ORCA::getInstance()->PreflightAcquisition(pDataBuffer);
}

DllExport SetupAcquisition(char * pDataBuffer)
{
	return ORCA::getInstance()->SetupAcquisition(pDataBuffer);
}

DllExport StartAcquisition(char * pDataBuffer)
{
	return ORCA::getInstance()->StartAcquisition(pDataBuffer);
}

DllExport StatusAcquisition(long &status)
{
	return ORCA::getInstance()->StatusAcquisition(status);
}

DllExport CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	return ORCA::getInstance()->CopyAcquisition(pDataBuffer, frameInfo);
}

DllExport PostflightAcquisition(char * pDataBuffer)
{
	return ORCA::getInstance()->PostflightAcquisition(pDataBuffer);
}

DllExport GetLastErrorMsg(wchar_t * errMsg, long size)
{
	return ORCA::getInstance()->GetLastErrorMsg(errMsg,size);
}

DllExport StatusAcquisitionEx(long &status, long &indexOfLastCompletedFrame)
{
	return ORCA::getInstance()->StatusAcquisitionEx(status,indexOfLastCompletedFrame);
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ORCA::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ORCA::getInstance()->GetParamString(paramID,str,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ORCA::getInstance()->SetParamBuffer(paramID,buffer,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ORCA::getInstance()->GetParamBuffer(paramID,buffer,size);
}