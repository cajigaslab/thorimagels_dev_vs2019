// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <map>
#include <memory>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

#define Device_MANAGER

#include "..\..\..\..\Common\Device.h"

//#define LOGGING_ENABLED

#ifdef LOGGING_ENABLED
#include "..\..\..\..\Common\Log.h"
extern auto_ptr<LogDll> logDll;
#else

enum EventType
{
	// Summary:
	//     Fatal error or application crash.
	CRITICAL_EVENT = 1,
	//
	// Summary:
	//     Recoverable error.
	ERROR_EVENT = 2,
	//
	// Summary:
	//     Noncritical problem.
	WARNING_EVENT = 4,
	//
	// Summary:
	//     Informational message.
	INFORMATION_EVENT = 8,
	//
	// Summary:
	//     Debugging trace.
	VERBOSE_EVENT = 16,

};
#endif

extern wchar_t message[256];


// TODO: reference additional headers your program requires here
