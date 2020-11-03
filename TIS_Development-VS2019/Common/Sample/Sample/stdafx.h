// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <vector>
#include <memory>
#include <ctime>
#include "..\..\..\Common\Log.h"
#include "..\..\..\Common\Device.h"
#include "..\..\Experiment.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

using namespace std;

extern void (*sampleFunctionPointer)(double &x, double &y);
extern auto_ptr<LogDll> logDll;
extern wchar_t message[256];

// TODO: reference additional headers your program requires here
