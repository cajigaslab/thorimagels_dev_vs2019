// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOMINMAX

// Windows Header Files:
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <vector>
#include <math.h>
#include <algorithm>
#include <string>
#include <Strsafe.h>
#include <vector>
#include "..\..\..\Common\ThorSharedTypesCPP.h"
#include "..\..\..\Common\BlockRingBuffer.h"
#include "..\..\..\Common\CircularBuffer.h"
#include "..\..\ScanManager\Scan.h"
#include "..\..\camera.h"

#define IMGWFBUILDER_EXPORTS

//#define LOGGING_ENABLED

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

#define DIGITAL_LINE_HIGH		1
#define DIGITAL_LINE_LOW		0

// TODO: reference additional headers your program requires here
#include "..\..\ImageWaveformBuilderDll.h"
#include "GeometryUtilitiesCpp.h"
