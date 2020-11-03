// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <memory>
#include <vector>
#include <sstream>
#include <regex>
#include <TCHAR.H>
#include <map>
#include <array>
#include "math.h"
#include "Strsafe.h"
#include <ctime>
#include <atomic>

#include "..\..\..\..\Common\Device.h"
#include "..\..\..\..\Common\Thread.h"
#include "..\..\..\..\Common\ThorSharedTypesCPP.h"
#include "..\..\..\..\Tools\IVI Foundation\x64\VISA\Win64\Include\TLBC1lib.h"

//#define LOGGING_ENABLED

#ifdef LOGGING_ENABLED
#include "..\..\..\..\Common\Log.h"
#endif

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#define MSG_SIZE 256
#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"

//extern LogDll *logDll;

#define DllExport extern "C" long __declspec( dllexport )

// TODO: reference additional headers your program requires here
