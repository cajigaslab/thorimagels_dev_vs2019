// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <vector>
#include <shlobj.h>
#include <math.h>
#include <shlwapi.h>
#include <fstream>
#include "Strsafe.h"
#include <iomanip>
#include <atomic>

using namespace std;

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#include "ticpp.h"
#include "tinyxml.h"
#include "ticpprc.h"

#include "..\..\..\Common\Camera.h"
#include "..\..\..\Common\ScanManager\Scan.h"
#include "..\..\..\Common\ThorSharedTypesCPP.h"
#include "..\..\..\Common\Command.h"
#include "..\..\..\Common\Device.h"
#include "..\..\..\Common\Log.h"
#include "..\..\..\Common\Thread.h"
#include "..\..\..\Hardware\Cameras\CameraManager\CameraManager\CameraManager.h"
#include "..\..\..\Hardware\Devices\DeviceManager\DeviceManager\DeviceManager.h"
#include "..\..\..\Common\ResourceManager\ResourceManager\ResourceManager.h"
#include "..\..\..\Common\ImageManager\ImageManager\Image.h"
#include "..\..\..\Common\ImageManager\ImageManager\ImageManager.h"
#include "..\..\..\Commands\General\SelectHardware\SelectHardware\SelectHardware.h"
#include "..\..\..\Commands\ImageAnalysis\PincushionCorrection\PincushionCorrection\PincushionCorrection.h"
#include "..\..\..\Commands\ImageAnalysis\FlatField\FlatField\FlatField.h"
#include "..\..\..\Commands\ImageAnalysis\LineProfile\LineProfile\LineProfile.h"
#include "..\..\..\Common\ThorDiskIO\ThorDiskIO.h"
#include "..\..\..\Common\HardwareXML\HardwareSetupXML.h"
#include "..\..\..\Common\AutoFocusModule\AutoFocus\AutoFocus.h"

#include "..\..\..\Common\StatsManager\StatsManager\StatsManager.h"
#include "..\..\..\Common\HDF5IOdll.h"


#include <tiffio.h>
#include "..\..\..\Tools\tiff-3.8.2\include\tifflib.h"

#include "..\..\..\Common\HardwareCom\HardwareCom\HardwareComFunctions.h"

//#define _HAS_ITERATOR_DEBUGGING 0

//#define _SECURE_SCL 0

#define DllExportLiveImage extern "C" long __declspec(dllexport)

extern auto_ptr<LogDll> logDll;
extern wchar_t message[256];

// TODO: reference additional headers your program requires here
