// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <string>
#include <memory>

#ifdef WINDVI_EXPORTS
#define WINDVI_API __declspec( dllexport )
#else
#define WINDVI_API __declspec(dllimport)
#endif

#define MSG_SIZE			256
#define MAX_BUFFER_CNT		256
#define BMP_COLOR_CNT		3

// TODO: reference additional headers your program requires here
#include "..\..\..\Common\bmpCpp.h"

#define LOGGING_ENABLED

#ifdef LOGGING_ENABLED
#include "..\..\Log.h"
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
