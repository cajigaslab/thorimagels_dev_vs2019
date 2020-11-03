// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <stdio.h>
//#include <windows.h>
#include <time.h>
#include <memory>
#include <vector>
#include <sstream>
#include <regex>
#include <TCHAR.H>
#include <map>
#include <thread>
using namespace std;

#include "..\..\..\..\Common\Device.h"
#include "..\..\..\..\Common\Thread.h"
#include "..\..\..\..\Common\ThorSharedTypesCPP.h"

#define MSG_SIZE 256

#ifdef LOGGING_ENABLED
#include "..\..\..\..\Common\Log.h"
#endif

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"

#define DllExport extern "C" long __declspec( dllexport )

// TODO: reference additional headers your program requires here
