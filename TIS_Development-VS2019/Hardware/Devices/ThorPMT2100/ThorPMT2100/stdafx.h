// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

using namespace std;

#include "..\..\..\..\Common\Device.h"
#include "..\..\..\..\Common\ThorSharedTypesCPP.h"

//#define LOGGING_ENABLED

#ifdef LOGGING_ENABLED
#include "..\..\..\..\Common\Log.h"
#endif

#define MSG_SIZE 256

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"

#define DllExport extern "C" long __declspec( dllexport )
