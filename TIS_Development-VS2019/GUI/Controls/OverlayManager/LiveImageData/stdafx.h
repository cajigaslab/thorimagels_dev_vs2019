// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
//
//#include <ippi.h>
//#include <ippcv.h>

#define DllExportLiveImage extern "C" __declspec(dllexport) long

// TODO: reference additional headers your program requires here
