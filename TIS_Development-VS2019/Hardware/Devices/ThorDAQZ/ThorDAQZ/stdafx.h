// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define LOGGING_ENABLED
#define MSG_SIZE 256

// add headers that you want to pre-compile here
#include "framework.h"
// Windows Header Files:
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <memory>
#include <time.h>
#include <vector>
#include "Strsafe.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <direct.h>
#include <intrin.h>
#include <mutex>
#include <math.h>
#include <thread>
#include <bitset>
#include <map>
#include <atomic>

#include "..\..\..\..\Common\Device.h"
#include "..\..\..\..\Common\ThorSharedTypesCPP.h"
#include "..\..\..\..\Common\ImageWaveformBuilderDll.h"

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"

#ifdef LOGGING_ENABLED
#include "../../../../Common/Log.h"
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

#define DllExport extern "C" long __declspec( dllexport )

#endif //PCH_H
