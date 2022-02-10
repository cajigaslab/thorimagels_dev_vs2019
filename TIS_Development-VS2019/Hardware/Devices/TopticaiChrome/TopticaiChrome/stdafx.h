// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <memory>
#include <regex>
#include <vector>
#include <TCHAR.H>
#include <map>
#include <sstream>
#include <strsafe.h>

using namespace std;

#define LOGGING_ENABLED

#ifdef LOGGING_ENABLED
#include "..\..\..\..\Common\Log.h"
#endif

#include "..\..\..\..\Common\Device.h"
#include "..\..\..\..\Common\Log.h"
#include "..\..\..\..\Common\Thread.h"
#include "..\..\..\..\Common\ThorSharedTypesCPP.h"

#define MSG_SIZE 256
#define READ_PORT_TIMEOUT_MS 1000
#define INSTRUCTION_LENGTH 100

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"

#define DllExport extern "C" long __declspec( dllexport )

// TODO: reference additional headers your program requires here

