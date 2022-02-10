// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorSLMPDM512.h"

BOOL APIENTRY DllMain(HMODULE hModule,
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

DllExport FindSLM(char* xml)
{
	return MeadowlarkPDM::getInstance()->FindSLM(xml);
}

DllExport TeardownSLM()
{
	return MeadowlarkPDM::getInstance()->TeardownSLM();
}

DllExport GetParam(const long paramID, double& param)
{
	return MeadowlarkPDM::getInstance()->GetParam(paramID, param);
}

DllExport SetParam(const long paramID, const double param)
{
	return MeadowlarkPDM::getInstance()->SetParam(paramID, param);
}

DllExport SetParamBuffer(const long paramID, char* pBuffer, long size)
{
	return MeadowlarkPDM::getInstance()->SetParamBuffer(paramID, pBuffer, size);
}

DllExport StartSLM()
{
	return MeadowlarkPDM::getInstance()->StartSLM();
}

DllExport StopSLM()
{
	return MeadowlarkPDM::getInstance()->StopSLM();
}

DllExport UpdateSLM(long arrayID)
{
	return MeadowlarkPDM::getInstance()->UpdateSLM(arrayID);
}

DllExport GetLastErrorMsg(wchar_t* msg, long size)
{
	return MeadowlarkPDM::getInstance()->GetLastErrorMsg(msg, size);
}
