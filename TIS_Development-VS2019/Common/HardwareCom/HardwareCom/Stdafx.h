// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <vector>
#include <memory>

#include "..\..\..\Common\Camera.h"
#include "..\..\..\Common\Command.h"
#include "..\..\..\Common\Device.h"
#include "..\..\..\Common\Log.h"
#include "..\..\..\Hardware\Devices\DeviceManager\DeviceManager\DeviceManager.h"
#include "..\..\..\Hardware\Cameras\CameraManager\CameraManager\CameraManager.h"
#include "..\..\..\Commands\General\SelectHardware\SelectHardware\SelectHardware.h"
#include "..\..\..\Common\ThorSharedTypesCPP.h"
#include "Strsafe.h"

#define DllExportFunc extern "C" long __declspec(dllexport)

extern auto_ptr<LogDll> logDll;

extern wchar_t message[256];