// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "thordaqResonantGalvo.h"

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

ThordaqResonantGALVO_API long FindCameras(long &cameraCount)
{
	return CThordaqResonantGalvo::GetInstance()->FindCameras(cameraCount);
}

ThordaqResonantGALVO_API long SelectCamera(const long camera)
{
	return CThordaqResonantGalvo::GetInstance()->SelectCamera(camera);
}

ThordaqResonantGALVO_API long TeardownCamera()
{
	return CThordaqResonantGalvo::GetInstance()->TeardownCamera();
}

ThordaqResonantGALVO_API long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return CThordaqResonantGalvo::GetInstance()->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
}

ThordaqResonantGALVO_API long SetParam(const long paramID, const double param)
{
	return CThordaqResonantGalvo::GetInstance()->SetParam(paramID, param);
}

ThordaqResonantGALVO_API long GetParam(const long paramID, double &param)
{
	return CThordaqResonantGalvo::GetInstance()->GetParam(paramID, param);
}

ThordaqResonantGALVO_API long PreflightAcquisition(char * pDataBuffer)
{
	return CThordaqResonantGalvo::GetInstance()->PreflightAcquisition(pDataBuffer);
}

ThordaqResonantGALVO_API long SetupAcquisition(char * pDataBuffer)
{
	return CThordaqResonantGalvo::GetInstance()->SetupAcquisition(pDataBuffer);
}

ThordaqResonantGALVO_API long StartAcquisition(char * pDataBuffer)
{
	return CThordaqResonantGalvo::GetInstance()->StartAcquisition(pDataBuffer);
}

ThordaqResonantGALVO_API long StatusAcquisition(long &status)
{
	return CThordaqResonantGalvo::GetInstance()->StatusAcquisition(status);
}

ThordaqResonantGALVO_API long StatusAcquisitionEx(long &status,long &indexOfLastCompletedFrame)
{
	return CThordaqResonantGalvo::GetInstance()->StatusAcquisitionEx(status, indexOfLastCompletedFrame);
}

ThordaqResonantGALVO_API long CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	return CThordaqResonantGalvo::GetInstance()->CopyAcquisition(pDataBuffer, frameInfo);
}

ThordaqResonantGALVO_API long PostflightAcquisition(char * pDataBuffer)
{
	return CThordaqResonantGalvo::GetInstance()->PostflightAcquisition(pDataBuffer);
}

ThordaqResonantGALVO_API long GetLastErrorMsg(wchar_t * msg, long size)
{
	return CThordaqResonantGalvo::GetInstance()->GetLastErrorMsg(msg, size);
}

ThordaqResonantGALVO_API void LogMessage(wchar_t *message,long eventLevel)
{
	return CThordaqResonantGalvo::GetInstance()->LogMessage(message, eventLevel);
}

ThordaqResonantGALVO_API long SetParamString(const long paramID, wchar_t * str)
{
	return CThordaqResonantGalvo::GetInstance()->SetParamString(paramID, str);
}

ThordaqResonantGALVO_API long GetParamString(const long paramID, wchar_t * str, long size)
{
	return CThordaqResonantGalvo::GetInstance()->GetParamString(paramID, str, size);
}

ThordaqResonantGALVO_API long SetParamBuffer(const long paramID, char * str, long size)
{
	return CThordaqResonantGalvo::GetInstance()->SetParamBuffer(paramID, str, size);
}

ThordaqResonantGALVO_API long GetParamBuffer(const long paramID, char * str, long size)
{
	return CThordaqResonantGalvo::GetInstance()->GetParamBuffer(paramID, str, size);
}

