// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include "Strsafe.h"
#include <memory>
#include <stdlib.h>
#include <string>
#include <cstring>

using namespace std;

#define LOGGING_ENABLED

//#include "TL6WL.h"
#include "..\..\..\..\Common\Device.h"
#include "..\..\..\..\Common\ThorSharedTypesCPP.h"
#include "..\..\..\..\Common\Thread.h"


#ifdef LOGGING_ENABLED
#include "..\..\..\..\Common\Log.h"
#endif

#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"

#define DllExport extern "C" long __declspec( dllexport )

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the ThorChrolis_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// ThorChrolis_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

//#ifdef ThorChrolis_EXPORTS
//	#define ThorChrolis_API __declspec(dllexport)
//#else
//	#define ThorChrolis_API __declspec(dllimport)
//#endif
