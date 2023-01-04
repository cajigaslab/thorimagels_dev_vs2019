// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorDAQGGDFLIMSim.h"

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

// Interfaces for ThorImage

THORDAQGGDFLIMSIM_API long FindCameras(long &cameraCount)
{
	return CThorDAQGGDFLIMSim::GetInstance()->FindCameras(cameraCount);
}

THORDAQGGDFLIMSIM_API long SelectCamera(const long camera)
{
	return CThorDAQGGDFLIMSim::GetInstance()->SelectCamera(camera);
}

THORDAQGGDFLIMSIM_API long TeardownCamera()
{
	return CThorDAQGGDFLIMSim::GetInstance()->TeardownCamera();
}

THORDAQGGDFLIMSIM_API long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return CThorDAQGGDFLIMSim::GetInstance()->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
}

THORDAQGGDFLIMSIM_API long SetParam(const long paramID, const double param)
{
	return CThorDAQGGDFLIMSim::GetInstance()->SetParam(paramID, param);
}

THORDAQGGDFLIMSIM_API long GetParam(const long paramID, double &param)
{
	return CThorDAQGGDFLIMSim::GetInstance()->GetParam(paramID, param);
}

THORDAQGGDFLIMSIM_API long PreflightAcquisition(char * pDataBuffer)
{
	return CThorDAQGGDFLIMSim::GetInstance()->PreflightAcquisition(pDataBuffer);
}

THORDAQGGDFLIMSIM_API long SetupAcquisition(char * pDataBuffer)
{
	return CThorDAQGGDFLIMSim::GetInstance()->SetupAcquisition(pDataBuffer);
}

THORDAQGGDFLIMSIM_API long StartAcquisition(char * pDataBuffer)
{
	return CThorDAQGGDFLIMSim::GetInstance()->StartAcquisition(pDataBuffer);
}

THORDAQGGDFLIMSIM_API long StatusAcquisition(long &status)
{
	return CThorDAQGGDFLIMSim::GetInstance()->StatusAcquisition(status);
}

THORDAQGGDFLIMSIM_API long StatusAcquisitionEx(long &status,long &indexOfLastCompletedFrame)
{
	return CThorDAQGGDFLIMSim::GetInstance()->StatusAcquisitionEx(status, indexOfLastCompletedFrame);
}

THORDAQGGDFLIMSIM_API long CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	return CThorDAQGGDFLIMSim::GetInstance()->CopyAcquisition(pDataBuffer, frameInfo);
}

THORDAQGGDFLIMSIM_API long PostflightAcquisition(char * pDataBuffer)
{
	return CThorDAQGGDFLIMSim::GetInstance()->PostflightAcquisition(pDataBuffer);
}

THORDAQGGDFLIMSIM_API long GetLastErrorMsg(wchar_t * msg, long size)
{
	return CThorDAQGGDFLIMSim::GetInstance()->GetLastErrorMsg(msg, size);
}

THORDAQGGDFLIMSIM_API void LogMessage(wchar_t *message,long eventLevel)
{
	return CThorDAQGGDFLIMSim::GetInstance()->LogMessage(message, eventLevel);
}

THORDAQGGDFLIMSIM_API long SetParamString(const long paramID, wchar_t * str)
{
	return CThorDAQGGDFLIMSim::GetInstance()->SetParamString(paramID, str);
}

THORDAQGGDFLIMSIM_API long GetParamString(const long paramID, wchar_t * str, long size)
{
	return CThorDAQGGDFLIMSim::GetInstance()->GetParamString(paramID, str, size);
}

THORDAQGGDFLIMSIM_API long SetParamBuffer(const long paramID, char * str, long size)
{
	return CThorDAQGGDFLIMSim::GetInstance()->SetParamBuffer(paramID, str, size);
}

THORDAQGGDFLIMSIM_API long GetParamBuffer(const long paramID, char * str, long size)
{
	return CThorDAQGGDFLIMSim::GetInstance()->GetParamBuffer(paramID, str, size);
}