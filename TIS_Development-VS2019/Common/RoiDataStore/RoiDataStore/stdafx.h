// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <memory>
#include <Strsafe.h>
#include <vector>
#include <sstream>
#include <ObjBase.h>

#include "..\..\Thread.h"
#include "..\..\Log.h"
#include "..\..\..\Tools\sqlite\sqlite3.h"
#define DllExportAPI extern "C" long __declspec(dllexport)

// TODO: reference additional headers your program requires here
