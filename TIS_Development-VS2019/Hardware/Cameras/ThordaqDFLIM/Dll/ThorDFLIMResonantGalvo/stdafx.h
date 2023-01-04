// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOMINMAX
#define MSG_SIZE 256
#define _USE_MATH_DEFINES

// Windows Header Files:
#include <windows.h>
#include <stdio.h>
//#include <stdlib.h>
//#include <tchar.h>
#include <string>
#include <memory>
#include <time.h>
#include <vector>
#include <malloc.h>
#include "Strsafe.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <direct.h>
#include <intrin.h>
#include <mutex>
#include <math.h>
#include <thread>
#include "NIDAQmx.h"
#include <bitset>

using namespace std;

#include "Camera.h"


#ifdef LOGGING_ENABLED
#include "..\..\..\..\Common\Log.h"
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

#include "ticpp\ticpp.h"
#include "ticpp\tinyxml.h"
#include "ticpp\ticpprc.h"

#ifndef safedelete
#define safedelete
#define SAFE_DELETE_ARRAY(x) if (x != NULL) { delete[] x; x = NULL; }
#define SAFE_DELETE_HANDLE(x) if(x !=NULL) { CloseHandle(x); x = NULL;}
#define SAFE_DELETE_PTR(x) if (x != NULL) { delete x; x = NULL; }
#endif


// TODO: reference additional headers your program requires here
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ThordaqResonantGALVO_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ThordaqResonantGALVO_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef THORDAQRESONANTGALVO_EXPORTS
#define ThordaqResonantGALVO_API extern "C" __declspec(dllexport)
#else
#define ThordaqResonantGALVO_API __declspec(dllimport)
#endif