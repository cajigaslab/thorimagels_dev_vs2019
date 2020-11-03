// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
//#define _SECURE_SCL 0
#include <vector>
#include <shlobj.h>

using namespace std;

#include <tiffio.h>
#include "..\..\Tools\tiff-3.8.2\include\tifflib.h"
//#include "..\..\..\..\Tools\tiff-3.8.2\include\jpeglib.h"
#include "..\..\Tools\IJGWin32\IJGWin32\jpeglib.h"

//#define _HAS_ITERATOR_DEBUGGING 0



//extern auto_ptr<LogDll> logDll;
extern wchar_t message[256];

// TODO: reference additional headers your program requires here
