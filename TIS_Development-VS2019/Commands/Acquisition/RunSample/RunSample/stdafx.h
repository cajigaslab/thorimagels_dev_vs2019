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
//#define _SECURE_SCL 0
#include <vector>
#include <deque>
#include <shlobj.h>
#include <omp.h>
#include <fstream>
#include <Strsafe.h>
#include <math.h>
#include <shlwapi.h>

#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <stdio.h>
//#include <stdlib.h>
#include <fcntl.h>
//#include <string.h>
#include <algorithm>
#include <iomanip>
using namespace std;

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"

#include "..\..\..\..\Common\Camera.h"
#include "..\..\..\..\Common\ScanManager\Scan.h"
#include "..\..\..\..\Common\Command.h"
#include "..\..\..\..\Common\Device.h"
#include "..\..\..\..\Common\Log.h"
#include "..\..\..\..\Common\SigSlot.h"
#include "..\..\..\..\Common\Experiment.h"
#include "..\..\..\..\Common\StringCpp.h"

using namespace sigslot;

#include "..\..\..\..\Hardware\Cameras\CameraManager\CameraManager\CameraManager.h"
#include "..\..\..\..\Hardware\Devices\DeviceManager\DeviceManager\DeviceManager.h"
#include "..\..\..\..\Common\ResourceManager\ResourceManager\ResourceManager.h"
#include "..\..\..\..\Common\ImageManager\ImageManager\Image.h"
#include "..\..\..\..\Common\ImageManager\ImageManager\ImageManager.h"
#include "..\..\..\..\Common\ExperimentManager\ExperimentManager\ExperimentManager.h"
#include "..\..\..\..\Commands\General\SelectHardware\SelectHardware\SelectHardware.h"
#include "..\..\..\..\Common\ThorDiskIO\ThorDiskIO.h"
#include "..\..\..\..\Common\HardwareXML\HardwareSetupXML.h"

#include "..\..\..\..\Commands\ImageAnalysis\PincushionCorrection\PincushionCorrection\PincushionCorrection.h"
#include "..\..\..\..\Commands\ImageAnalysis\FlatField\FlatField\FlatField.h"

#include <tiffio.h>
#include "..\..\..\..\Tools\tiff-3.8.2\include\tifflib.h"
#include "..\..\..\..\Tools\IJGWin32\IJGWin32\jpeglib.h"
#include "Observer.h"

#include "..\..\..\Common\StatsManager\StatsManager\StatsManager.h"

#include "..\..\..\Common\HardwareCom\HardwareCom\HardwareComFunctions.h"
#include "..\..\..\Common\ImageStoreLibrary\src\ImageStoreLibraryDLL.h"

//#define _HAS_ITERATOR_DEBUGGING 0


#define DllExport_RunSample extern "C" long __declspec(dllexport)

extern auto_ptr<LogDll> logDll;
const long MSG_LENGTH = 256;
extern wchar_t message[MSG_LENGTH];

#define CFuncErrChk(fnName,fnCall,exit) if (0 == fnCall){DWORD error = GetLastError(); StringCbPrintfW(message,_MAX_PATH,L"%s failed; Error code %d ",fnName, error);logDll->TLTraceEvent(ERROR_EVENT,1,message); if(exit) {throw "fnCall";}}

// TODO: reference additional headers your program requires here
