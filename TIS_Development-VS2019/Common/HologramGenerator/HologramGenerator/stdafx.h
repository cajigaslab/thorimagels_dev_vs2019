// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <vector>						//include for auto_ptr
#include "Strsafe.h"
#include "math.h"
#include <algorithm>
#include <complex>
#include <thread>

#define HOLOGEN_EXPORTS

using namespace std;

// TODO: reference additional headers your program requires here
#include "..\..\thread.h"
#include "..\..\..\Tools\Intel IPP\intel64\include\IPPlib.h"
#include "..\..\Log.h"
#include "..\..\ThorSharedTypesCPP.h"
#include "..\..\bmpCpp.h"
#include "..\..\WinDVI\WinDVI\WinDVIlib.h"
#include "..\..\SLM.h"

//#define LOGGING_ENABLED
#define MSG_LENGTH		256
#define MAX_PIXEL_VALUE 255

extern std::auto_ptr<LogDll> logDll;
extern wchar_t errMsg[MSG_LENGTH];
