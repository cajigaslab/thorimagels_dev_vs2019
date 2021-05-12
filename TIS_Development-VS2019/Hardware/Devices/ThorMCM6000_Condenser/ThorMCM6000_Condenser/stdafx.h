// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <map>
#include <string>
#include <math.h>
#include <iomanip>
#include <strsafe.h>

using namespace std;

#include "..\..\..\..\Common\Thread.h"
#include "..\..\..\..\Common\DevParaInfo\ParamInfo.h"
#include "..\..\..\..\Common\SerialPortAccess\Serial.h"
#include "..\..\..\..\Common\ThorSharedTypesCPP.h"
#include "..\..\ThorMCM6000\ThorMCM6000\ThorMCM6000.h"
#include "..\..\ThorMCM6000\ThorMCM6000\MCM6000.h"
#include "..\..\ThorMCM6000\ThorMCM6000\APT.h"
#include "..\..\ThorMCM6000\ThorMCM6000\include\apt_cmd_library.h"
#include "..\..\ThorMCM6000\ThorMCM6000\include\apt_cmd_library_motor.h"

#define LOGGING_ENABLED

//#ifdef LOGGING_ENABLED
//#include "..\..\..\..\Common\Log.h"
//#endif

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#define MSG_SIZE 256
#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"

#define DllExport extern "C" long __declspec( dllexport )

// TODO: reference additional headers your program requires here
