// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "AutoFocus.h"
#include "AutoFocusClass.h"

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



//DllExport TLTraceEvent(long eventType,long id,LPWSTR str)
//{
//	return IAutoFocus::getInstance()->TraceEvent((EventType)eventType,id,str);
//}

DllExportAF RunAutofocus(long magnification, long afType, BOOL& bFound)
{
	return IAutoFocus::getInstance()->RunAF(magnification, afType, bFound);
}

DllExportAF AutofocusExecuteNextIteration(long afType)
{
	return IAutoFocus::getInstance()->AFExecuteNextIteration(afType);
}

DllExportAF SetupAutofocus(long afType, long repeat, double afFocusOffset, double expTimeMS, double stepSizeUM, double startPosMM, double stopPosMM, long binning, double finePercentage, long enableGUIUpdate)
{
	return IAutoFocus::getInstance()->SetupAF(afType, repeat, afFocusOffset, expTimeMS, stepSizeUM, startPosMM, stopPosMM, binning, finePercentage, enableGUIUpdate);
}

DllExportAF GetAutoFocusStatusAndImage(char* imageBuffer, long& imageAvailable, long& afRunning, long& frameNumber, long& currentRepeat, long& status, long& zSteps, long& currentZIndex)
{
	return IAutoFocus::getInstance()->GetAFStatusAndImage(imageBuffer, imageAvailable, afRunning, frameNumber, currentRepeat, status, zSteps, currentZIndex);
}

DllExportAF IsAutofocusRunning()
{
	return IAutoFocus::getInstance()->GetAFRunning();
}

DllExportAF StopAutofocus()
{
	return IAutoFocus::getInstance()->StopAF();
}

DllExportAF GetAutofocusStatus(long& currentStatus, long& bestContrastScore, double& bestZPosition, double& nextZPosition, long& currentRepeatIndex)
{
	return IAutoFocus::getInstance()->GetAFStatus(currentStatus, bestContrastScore, bestZPosition, nextZPosition, currentRepeatIndex);
}


DllExportAF SetAutoFocusStartZPosition(double afStartPos, BOOL bWait, BOOL afFound)
{
	return IAutoFocus::getInstance()->SetAFStartZPosition(afStartPos, bWait, afFound);
}
