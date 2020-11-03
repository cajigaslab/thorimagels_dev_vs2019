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
#include <memory>
#include <tchar.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <direct.h>
#include <atomic>
#include "errno.h"
#include "Strsafe.h"
#include <queue>
#include <map>


#include "..\..\..\..\Common\Camera.h"
#include "..\..\..\..\Common\StringCPP.h"
#include "..\..\..\..\Common\ThorSharedTypesCPP.h"
#include "..\..\..\..\Common\ThreadSafeQueue.h"
#include "..\..\..\..\Tools\Intel IPP\intel64\include\IPPlib.h"

#define LOGGING_ENABLED

#ifdef LOGGING_ENABLED
#include "..\..\..\..\Common\Log.h"
static std::auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
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

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#define MSG_SIZE 256

#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"

#include "..\..\..\..\Tools\Hamamatsu\dcamsdk4\inc\dcamapi4.h"
#include "..\..\..\..\Tools\Hamamatsu\dcamsdk4\inc\dcamprop.h"

#define DllExport extern "C" long __declspec( dllexport )