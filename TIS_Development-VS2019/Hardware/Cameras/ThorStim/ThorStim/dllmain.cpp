// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorStim.h"

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		ThorStim::getInstance()->hDLLInstance = hModule;
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
	return ThorStim::getInstance()->FindCameras(cameraCount);
}

DllExport SelectCamera(const long camera)
{
	return ThorStim::getInstance()->SelectCamera(camera);
}

DllExport TeardownCamera()
{
	return ThorStim::getInstance()->TeardownCamera();
}

DllExport GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return ThorStim::getInstance()->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
}

DllExport SetParam(const long paramID, const double param)
{
	return ThorStim::getInstance()->SetParam(paramID,param);
}

DllExport GetParam(const long paramID, double &param)
{
	return ThorStim::getInstance()->GetParam(paramID,param);
}

DllExport PreflightAcquisition(char * pDataBuffer)
{
	return ThorStim::getInstance()->PreflightAcquisition(pDataBuffer);
}

DllExport SetupAcquisition(char * pDataBuffer)
{
	return ThorStim::getInstance()->SetupAcquisition(pDataBuffer);
}

DllExport StartAcquisition(char * pDataBuffer)
{
	return ThorStim::getInstance()->StartAcquisition(pDataBuffer);
}

DllExport StatusAcquisition(long &status)
{
	return ThorStim::getInstance()->StatusAcquisition(status);
}

DllExport CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	return ThorStim::getInstance()->CopyAcquisition(pDataBuffer, frameInfo);
}

DllExport PostflightAcquisition(char * pDataBuffer)
{
	return ThorStim::getInstance()->PostflightAcquisition(pDataBuffer);
}

DllExport GetLastErrorMsg(wchar_t * errMsg, long size)
{
	return ThorStim::getInstance()->GetLastErrorMsg(errMsg,size);
}

DllExport StatusAcquisitionEx(long &status, long &indexOfLastCompletedFrame)
{
	return ThorStim::getInstance()->StatusAcquisitionEx(status,indexOfLastCompletedFrame);
}

DllExport SetParamString(long paramID, wchar_t * str)
{
	return ThorStim::getInstance()->SetParamString(paramID,str);
}

DllExport GetParamString(const long paramID, wchar_t *str, long size)
{
	return ThorStim::getInstance()->GetParamString(paramID,str,size);
}

DllExport GetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorStim::getInstance()->GetParamBuffer(paramID,buffer,size);
}

DllExport SetParamBuffer(const long paramID, char * buffer, long size)
{
	return ThorStim::getInstance()->SetParamBuffer(paramID,buffer,size);
}