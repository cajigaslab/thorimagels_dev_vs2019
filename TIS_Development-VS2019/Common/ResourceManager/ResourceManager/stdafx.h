// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <Shlobj.h>

#include <string>
#include <memory>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "Strsafe.h"

using namespace std;

#define RESOURCE_MANAGER

#include "..\..\Log.h"
#include "..\..\Command.h"
#include "..\..\StringCPP.h"
//#include "..\..\ThorSharedTypes\ThorSharedTypes\SharedEnums.cs"
#include "..\..\THORSHAREDTYPESCPP.H"
#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#include "..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\Tools\ticpp\ticpprc.h"


#define DllExport extern "C" long __declspec( dllexport )
extern auto_ptr<LogDll> logDll;
extern wchar_t message[MAX_PATH];


// TODO: reference additional headers your program requires here
