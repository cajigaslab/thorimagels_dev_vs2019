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

using namespace std;

#include "..\..\..\..\Common\Device.h"
#include "..\..\..\..\Common\Log.h"

#define DllExport extern "C" long __declspec( dllexport )

// TODO: reference additional headers your program requires here
