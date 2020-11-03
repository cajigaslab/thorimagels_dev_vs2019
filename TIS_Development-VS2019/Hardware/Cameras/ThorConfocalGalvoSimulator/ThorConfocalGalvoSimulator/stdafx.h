// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <stdio.h>
#include <windows.h>
#include <memory>
#include <math.h>
#include <direct.h>
#include "process.h"
#include <list>

using namespace std;

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"

#include "..\..\..\..\Common\ThorDiskIO\ThorDiskIO.h"
#include "..\..\..\..\Common\ThorSharedTypesCPP.h"
#include "..\..\..\..\Common\Camera.h"
#include "..\..\..\..\Common\Log.h"

#define DllExport extern "C" long __declspec( dllexport )

//factor to scale the field size (0 ~ 255) to theta, a value not very critical and accurate
// h= f * Theta,  h = 25, f=70mm, Theta = 20.46
//Convert Theta to radians and multiply by the maximum value of 255 gives the result below
//#define FIELD2THETA 0.08024619 //calculated for an ftheta lens to image a maximum field size of 25mm^2
#define FIELD2THETA 0.0901639344

// TODO: reference additional headers your program requires here


