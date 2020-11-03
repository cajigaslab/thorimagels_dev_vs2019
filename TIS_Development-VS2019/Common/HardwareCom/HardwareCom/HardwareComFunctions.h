#pragma once

#include "..\..\..\Common\ThorSharedTypesCPP.h"

#ifndef DllExportFunc
#define DllExportFunc extern "C" long __declspec(dllexport)
#endif

#ifndef DllExportDevFunc
#define DllExportDevFunc extern "C" __declspec(dllexport) IDevice* 
#endif

#ifndef DllExportCameraFunc
#define DllExportCameraFunc extern "C" __declspec(dllexport) ICamera* 
#endif


DllExportFunc GetDeviceParamAvailable(long deviceSelection, long paramId);
DllExportFunc GetDeviceParamReadOnly(long deviceSelection, long paramId);
DllExportFunc GetDeviceParamDouble(long deviceSelection, long paramId, double &param);
DllExportFunc SetDeviceParamDouble(long deviceSelection, long paramId, double param, long exeOrWait); ///<exeOrWait (2): won't execute
DllExportFunc GetDeviceParamLong(long deviceSelection, long paramId, long &param);
DllExportFunc SetDeviceParamLong(long deviceSelection, long paramId, long param, long exeOrWait); ///<exeOrWait (2): won't execute
DllExportFunc GetDeviceParamRangeDouble(long deviceSelection, long paramId,  double &valMin, double &valMax, double &valDefault);
DllExportFunc GetDeviceParamRangeLong(long deviceSelection, long paramId,  long &valMin, long &valMax, long &valDefault);
DllExportFunc SetDeviceParamBuffer(long deviceSelection, long paramId, char* buf, long len, long exeOrWait); ///<exeOrWait (2): won't execute
DllExportFunc GetDeviceParamBuffer(long deviceSelection, long paramId, char* buf, long len);
DllExportFunc SetDeviceParamString(long deviceSelection, long paramID, wchar_t * str, long exeOrWait); ///<exeOrWait (2): won't execute
DllExportFunc GetDeviceParamString(long deviceSelection, long paramID, wchar_t * str, long size);
DllExportFunc GetDeviceStatus(long deviceSelection, long &status);
DllExportFunc GetDeviceErrorMessage(long deviceSelection, wchar_t* errorMessage, long size);
DllExportFunc GetCameraParamAvailable(long cameraSelection, long paramId);
DllExportFunc GetCameraParamReadOnly(long cameraSelection, long paramId);
DllExportFunc GetCameraParamDouble(long cameraSelection, long paramID, double &val);
DllExportFunc GetCameraParamRangeDouble(long cameraSelection, long paramID, double &valMin, double &valMax, double &valDefault);
DllExportFunc GetCameraParamRangeLong(long cameraSelection, long paramID, long &valMin, long &valMax, long &valDefault);
DllExportFunc SetCameraParamDouble(long cameraSelection, long paramID, double val);
DllExportFunc SetCameraParamLong(long cameraSelection, long paramID, long val);
DllExportFunc SetCameraParamBuffer(long cameraSelection, long paramID, char * pBuffer, long size);
DllExportFunc GetCameraParamBuffer(long cameraSelection, long paramID, char * pBuffer, long size);
DllExportFunc SetCameraParamString(long cameraSelection, long paramID, wchar_t * str);
DllExportFunc GetCameraParamString(long cameraSelection, long paramID, wchar_t * str, long size);
DllExportFunc GetCameraParamLong(long cameraSelection, long paramID, long &val);
DllExportDevFunc GetDevice(long selectedDevice);
DllExportCameraFunc GetCamera(long selectedCamera);
DllExportFunc GetDeviceSelectedIndex(long selectedDevice, long &selectedIndex);
DllExportFunc GetCameraSelectedIndex(long selectedCamera, long &selectedIndex);
DllExportFunc PreflightCamera(long cameraSelection);
DllExportFunc PostflightCamera(long cameraSelection);
