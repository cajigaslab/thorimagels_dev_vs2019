// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "StatsManager.h"

#define DllExport extern "C" long __declspec(dllexport)

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

DllExport SetStatsMask(unsigned short *mask, long width, long height)
{
	return StatsManager::getInstance()->SetStatsMask(mask, width, height);
}

DllExport IsStatsComplete()
{
	return StatsManager::getInstance()->IsStatsComplete();
}

DllExport ComputeStats(unsigned short *data, FrameInfoStruct frameInfo, long channelEnabled, long includeLineProfile, long includeRegularStats, long enabledChannelsOnly = FALSE)
{
	return StatsManager::getInstance()->ComputeStats(data, frameInfo, channelEnabled, includeLineProfile, includeRegularStats, enabledChannelsOnly);
}

DllExport CreateStatsManagerROIDS(long dsType, char* pathAndName)
{
	return StatsManager::getInstance()->CreateStatsManagerROIDS(dsType, pathAndName);
}

DllExport GetNumROI()
{
	return StatsManager::getInstance()->GetNumROI();
}

DllExport InitCallBack(pushImageProcessDataCallBack pushFuncPtr)
{
	return StatsManager::InitCallBack(pushFuncPtr);
}

DllExport InitCallBackLineProfilePush(pushLineProfileCallBack pushFuncPtr)
{
	return StatsManager::InitCallBackLineProfilePush(pushFuncPtr);
}

DllExport InitCallBackDFLIMROIHistogramsPush(pushDFLIMROIHistogramsCallBack pushFuncPtr)
{
	return StatsManager::InitCallBackDFLIMROIHistogramsPush(pushFuncPtr);
}

DllExport SetLineProfileLine(long p1X, long p1Y, long p2X, long p2Y, long lineIsActive)
{
	return StatsManager::getInstance()->SetLineProfileLine(p1X, p1Y, p2X, p2Y, lineIsActive);
}

DllExport SetLineProfileLineWidth(long lineWidth)
{
	return StatsManager::getInstance()->SetLineProfileLineWidth(lineWidth);
}

DllExport SetTZero(double tZero, long channelIndex)
{
	return StatsManager::getInstance()->SetTZero(tZero, channelIndex);
}