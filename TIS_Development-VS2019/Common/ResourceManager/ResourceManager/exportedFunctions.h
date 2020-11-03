// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#define DllExportFunc extern "C" __declspec(dllexport)

DllExportFunc long GetApplicationSettingsPath(wchar_t* stringBuffer, unsigned int bufferLength);
DllExportFunc long GetActiveSettingsFilePathAndName(wchar_t* stringBuffer, unsigned int bufferLength);
DllExportFunc long GetApplicationSettingsFilePathAndName(wchar_t* stringBuffer, unsigned int bufferLength);
DllExportFunc long GetModalityApplicationSettingsFilePathAndName(wchar_t* modality, wchar_t* stringBuffer, unsigned int bufferLength);
DllExportFunc long GetCaptureTemplatePath(wchar_t* stringBuffer, unsigned int bufferLength);
DllExportFunc long GetHardwareSettingsFilePathAndName(wchar_t* stringBuffer, unsigned int bufferLength);
DllExportFunc long GetModalityHardwareSettingsFilePathAndName(wchar_t* modality, wchar_t* stringBuffer, unsigned int bufferLength);
DllExportFunc long GetSettingsParamLong(int settingsFileType, wchar_t* tagName, wchar_t* attribute, long defaultValue);
DllExportFunc long LoadSettings();
DllExportFunc bool BorrowDocMutex(long sfType, long timeMS = -1);
DllExportFunc bool ReturnDocMutex(long sfType);