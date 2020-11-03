// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ROIDataStore.h"

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

DllExportAPI InitCallBack(pushROIDataCallBack pushFuncPtr) 
{
	return ROIDataStore::GetInstance()->InitCallBack(pushFuncPtr);
}

DllExportAPI LoadROIData(char** statsName, double* stats, long nStats, long isLast)
{
	ROIDataStore::GetInstance()->LoadROIData(statsName, stats, nStats, isLast);
	return TRUE;
}

DllExportAPI RequestROIData()
{
	ROIDataStore::GetInstance()->RequestROIData();
	return TRUE;
}

DllExportAPI CreateROIDataStore(long type, char* pathAndName) 
{
	ROIDataStore::CreateROIDataStore(type, pathAndName);
	return TRUE;
}

