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
#include <map>
#include <mutex>

using namespace std;

// TODO: reference additional headers your program requires here

#include "..\..\..\..\Common\Device.h"
#include "..\..\..\..\Common\ThorSharedTypesCPP.h"
#include "..\..\..\..\Common\WinDVI\WinDVI\WinDVIlib.h"
#include "..\..\..\..\Common\HologramGenerator\HologramGenerator\HologramGeneratorlib.h"
#include "..\..\..\..\Common\HighPerfTimer.h"
#include "..\..\..\..\Common\bmpCpp.h"
#include "..\..\..\..\Common\StringCPP.h"
#include "..\..\..\..\Tools\National Instruments\NIDAQmx\include\BoardInfoNI.h"

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#define MSG_SIZE		256

#define XY_COORD		2
#define RGB_CNT			3
#define LUT_SIZE		65536
#define SLM_TIMEOUT_MIN	20000	//[ms]

#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"


#define DllExport extern "C" long __declspec( dllexport )

