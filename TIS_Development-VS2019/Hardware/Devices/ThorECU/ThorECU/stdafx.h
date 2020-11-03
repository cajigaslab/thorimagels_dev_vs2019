// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#pragma warning(push)
#pragma warning(disable:4996)
#include <xutility>
#pragma warning(pop)

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <stdio.h>
//#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <memory>
#include <direct.h>

using namespace std;

#include "..\..\..\..\Common\Device.h"
#include "..\..\..\..\Common\Log.h"
#include "..\..\..\..\Common\ThorSharedTypesCPP.h"

#define MSG_SIZE 256
#define LOGGING_ENABLED
#define READ_BUFFER_SIZE 2048

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"

#define DllExport extern "C" long __declspec( dllexport )

// TODO: reference additional headers your program requires here

