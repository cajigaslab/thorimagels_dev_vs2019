#pragma once
#define DllExportAF extern "C" long __declspec(dllexport)

DllExportAF RunAutofocus(long magnification, long afType, BOOL& bFound);
DllExportAF AutofocusExecuteNextIteration(long afType);
DllExportAF SetupAutofocus(long afType, long repeat, double afFocusOffset, double expTimeMS, double stepSizeUM, double startPosMM, double stopPosMM, long binning, double finePercentage, long enableGUIUpdate);
DllExportAF GetAutoFocusStatusAndImage(char* imageBuffer, long& imageAvailable, long& afRunning, long& frameNumber, long& currentRepeat, long& status, long& zSteps, long& currentZIndex);
DllExportAF IsAutofocusRunning();
DllExportAF StopAutofocus();
DllExportAF GetAutofocusStatus(long& currentStatus, long& bestContrastScore, double& bestZPosition, double& nextZPosition, long& currentRepeatIndex);
DllExportAF SetAutoFocusStartZPosition(double afStartPos, BOOL bWait, BOOL afFound);
