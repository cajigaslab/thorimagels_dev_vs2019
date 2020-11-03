// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <math.h>


// TODO: reference additional headers your program requires here
#define DllExportImageProcess extern "C" long __declspec(dllexport)


//#include "C:\TIS\TIS_Development\Tools\Intel IPP\intel64\include\ipp.h"