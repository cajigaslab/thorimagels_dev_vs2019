// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "thordaqGalvoGalvo.h"

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

THORDAQGALVOGALVO_API long FindCameras(long &cameraCount)
{
	return CThorDAQGalvoGalvo::GetInstance()->FindCameras(cameraCount);
}

THORDAQGALVOGALVO_API long SelectCamera(const long camera)
{
	return CThorDAQGalvoGalvo::GetInstance()->SelectCamera(camera);
}

THORDAQGALVOGALVO_API long TeardownCamera()
{
	return CThorDAQGalvoGalvo::GetInstance()->TeardownCamera();
}

THORDAQGALVOGALVO_API long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	return CThorDAQGalvoGalvo::GetInstance()->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
}

THORDAQGALVOGALVO_API long SetParam(const long paramID, const double param)
{
	return CThorDAQGalvoGalvo::GetInstance()->SetParam(paramID, param);
}

THORDAQGALVOGALVO_API long GetParam(const long paramID, double &param)
{
	return CThorDAQGalvoGalvo::GetInstance()->GetParam(paramID, param);
}

THORDAQGALVOGALVO_API long PreflightAcquisition(char * pDataBuffer)
{
	return CThorDAQGalvoGalvo::GetInstance()->PreflightAcquisition(pDataBuffer);
}

THORDAQGALVOGALVO_API long SetupAcquisition(char * pDataBuffer)
{
	return CThorDAQGalvoGalvo::GetInstance()->SetupAcquisition(pDataBuffer);
}

THORDAQGALVOGALVO_API long StartAcquisition(char * pDataBuffer)
{
	return CThorDAQGalvoGalvo::GetInstance()->StartAcquisition(pDataBuffer);
}

THORDAQGALVOGALVO_API long StatusAcquisition(long &status)
{
	return CThorDAQGalvoGalvo::GetInstance()->StatusAcquisition(status);
}

THORDAQGALVOGALVO_API long StatusAcquisitionEx(long &status,long &indexOfLastCompletedFrame)
{
	return CThorDAQGalvoGalvo::GetInstance()->StatusAcquisitionEx(status, indexOfLastCompletedFrame);
}

THORDAQGALVOGALVO_API long CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	return CThorDAQGalvoGalvo::GetInstance()->CopyAcquisition(pDataBuffer, frameInfo);
}

THORDAQGALVOGALVO_API long PostflightAcquisition(char * pDataBuffer)
{
	return CThorDAQGalvoGalvo::GetInstance()->PostflightAcquisition(pDataBuffer);
}

THORDAQGALVOGALVO_API long GetLastErrorMsg(wchar_t * msg, long size)
{
	return CThorDAQGalvoGalvo::GetInstance()->GetLastErrorMsg(msg, size);
}

THORDAQGALVOGALVO_API void LogMessage(wchar_t *message,long eventLevel)
{
	return CThorDAQGalvoGalvo::GetInstance()->LogMessage(message, eventLevel);
}

THORDAQGALVOGALVO_API long SetParamString(const long paramID, wchar_t * str)
{
	return CThorDAQGalvoGalvo::GetInstance()->SetParamString(paramID, str);
}

THORDAQGALVOGALVO_API long GetParamString(const long paramID, wchar_t * str, long size)
{
	return CThorDAQGalvoGalvo::GetInstance()->GetParamString(paramID, str, size);
}

THORDAQGALVOGALVO_API long SetParamBuffer(const long paramID, char * str, long size)
{
	return CThorDAQGalvoGalvo::GetInstance()->SetParamBuffer(paramID, str, size);
}

THORDAQGALVOGALVO_API long GetParamBuffer(const long paramID, char * str, long size)
{
	return CThorDAQGalvoGalvo::GetInstance()->GetParamBuffer(paramID, str, size);
}