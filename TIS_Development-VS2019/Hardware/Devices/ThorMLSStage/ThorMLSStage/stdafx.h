// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <stdio.h>
//#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <memory>
#include <vector>
#include <sstream>

using namespace std;

#include "..\..\..\..\Common\Device.h"
#include "..\..\..\..\Common\Log.h"
#include "..\..\..\..\Common\Thread.h"
#include "..\..\..\..\Common\ThorSharedTypesCPP.h"

//#define LOGGING_ENABLED

#include "..\..\..\..\Common\Log.h"

#define MSG_SIZE 256

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif


#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"

#define DllExport extern "C" long __declspec( dllexport )

//extern LogDll *logDll;
extern wchar_t message[256];

// TODO: reference additional headers your program requires here
