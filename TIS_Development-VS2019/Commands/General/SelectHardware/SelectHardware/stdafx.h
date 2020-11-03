// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

using namespace std;
#include <ticpp.h>
#include <tchar.h>
#include <windows.h>
#include <vector>
#include <shlobj.h>
#include "Strsafe.h"
#include "..\..\..\..\Common\Camera.h"
#include "..\..\..\..\Common\Command.h"
#include "..\..\..\..\Common\Device.h"
#include "..\..\..\..\Common\Log.h"
#include "..\..\..\..\Common\ResourceManager\ResourceManager\ResourceManager.h"
#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"
#include "..\..\..\..\Hardware\Cameras\CameraManager\CameraManager\CameraManager.h"
#include "..\..\..\..\Hardware\Devices\DeviceManager\DeviceManager\DeviceManager.h"


//#define _HAS_ITERATOR_DEBUGGING 0

//#define _SECURE_SCL 0
#include <vector>
#include <sstream>
#include <string>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#define DllExport extern "C" long __declspec(dllexport)

extern auto_ptr<LogDll> logDll;
extern wchar_t message[256];

// TODO: reference additional headers your program requires here
