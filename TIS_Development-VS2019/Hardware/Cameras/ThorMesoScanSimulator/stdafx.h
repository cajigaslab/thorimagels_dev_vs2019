// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <stdio.h>
#include <windows.h>
#include <mutex>
#include <string>
#include <vector>
#include "SimulatorImage.h"
#include "..\ThorMesoScan\Types.h"
#include "..\ThorMesoScan\CameraConfig.h"
#include "..\..\..\Common\ThorSharedTypesCPP.h"

//#define LOGGING_ENABLED

#ifdef LOGGING_ENABLED
#include "..\..\..\Common\Log.h"
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

// TODO: reference additional headers your program requires here
#define DllExport extern "C" long __declspec( dllexport )
