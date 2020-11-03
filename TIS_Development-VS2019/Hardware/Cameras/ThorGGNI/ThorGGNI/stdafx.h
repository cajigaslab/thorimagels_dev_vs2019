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
#include <fstream>
#include <regex>

using namespace std;
#include "..\..\..\..\Common\ThorSharedTypesCPP.h"
#include "..\..\..\..\Common\Camera.h"
#include "..\..\..\..\Common\BlockRingBuffer.h"
#include "..\..\..\..\Common\ScanManager\Scan.h"
#include "..\..\..\..\Common\ScanManager\ScanModeClass.h"

#include "..\..\..\..\Common\ImageWaveformBuilderDll.h"
#include "..\..\..\..\Common\StringCPP.h"
#include "..\..\..\..\Tools\National Instruments\NIDAQmx\include\BoardInfoNI.h"

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#ifndef U16
#define U16 unsigned short
#endif

#ifndef U32
#define U32 unsigned long
#endif

#define ApiSuccess 0

#define ApiFailed 1

#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"

#define DllExport extern "C" long __declspec( dllexport )
