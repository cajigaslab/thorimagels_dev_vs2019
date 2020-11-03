// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>
#include "Strsafe.h"

using namespace std;

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#include "..\..\Camera.h"
#include "..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\Tools\ticpp\ticpprc.h"
#include "..\..\Experiment.h"
#include "..\..\StringCpp.h"
#include "..\..\Log.h"

#define MSG_LENGTH 256
extern auto_ptr<LogDll> logDll;
extern wchar_t message[MSG_LENGTH];


#define DllExportExpManger extern "C" long __declspec(dllexport)

#define Experiment_MANAGER



// TODO: reference additional headers your program requires here
