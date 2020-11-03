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
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

#define CAMERA_MANAGER

#include "..\..\..\..\Common\Camera.h"
#include "..\..\..\..\Common\Log.h"

extern auto_ptr<LogDll> logDll;
extern wchar_t message[256];


// TODO: reference additional headers your program requires here
