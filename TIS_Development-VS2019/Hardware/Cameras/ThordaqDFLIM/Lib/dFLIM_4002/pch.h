// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
// [from stdafx.h] : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define INITGUID
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOMINMAX
#define MSG_SIZE 256
#define _USE_MATH_DEFINES

// Windows Header Files:
#include <windows.h>	// for using Windows data types and functions
#include <stdio.h>		// for using C language IO functions
#include <string>		// for using std.lib string related functions
#include <memory>       // for using std.lib memory related functions
#include <time.h>       // for getting and manipulating date and time informations
#include <vector>		// for using std.lib vector related functions
#include <malloc.h>     // for using C memory allocation functions
#include "Strsafe.h"	// for safe string functions
#include <fstream>      // for using file stream class
#include <sstream>      // for using string stream class
#include <algorithm>    // for using std.lib data manipulating functions
#include <direct.h>     // for using Windows file directory types and functions
#include <intrin.h>     // for using Manufacturer-specific intrinsic functions
#include <mutex>        // for using mutual exclusion (mutex) of concurrent execution of critical sections of code
#include <math.h>       // for using common mathmatical oprations and transformers and data types
#include <thread>       // for using multithreads functions
#include <winioctl.h>   // for defining ioctls
#include <bitset>

#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

// Define  Macros
#ifndef min
#define min(_a, _b)             (((_a) < (_b)) ? (_a) : (_b))
#endif

#ifndef max
#define max(_a, _b)				(((_a) > (_b)) ? (_a) : (_b))
#endif

#ifndef safedelete
#define safedelete
#define SAFE_DELETE_ARRAY(x)	if (x != NULL) { delete[] x;		x = NULL; }
//#define SAFE_DELETE_HANDLE(x)	if (x != NULL) { CloseHandle(x);	x = NULL; }
#define SAFE_DELETE_PTR(x)		if (x != NULL) { delete x;			x = NULL; }
#endif


#endif //PCH_H