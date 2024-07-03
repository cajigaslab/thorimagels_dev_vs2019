// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <stdio.h>
#include <windows.h>
#include <memory>
#include "Includes\AlazarError.h"
#include "Includes\AlazarApi.h"
#include "Includes\AlazarCmd.h"
#include "..\..\..\Tools\National Instruments\NIDAQmx\include\BoardInfoNI.h"
#include "..\..\..\Common\ResourceManager\ResourceManager\ResourceManager.h"
#include <math.h>
#include <direct.h>
#include "process.h"
#include "Logger.h"

using namespace std;

#include "..\..\..\Common\camera.h"
#include "..\..\..\Common\StringCPP.h"
#include "..\..\..\Common\ThorSharedTypesCPP.h"

#define DllExport extern "C" long __declspec( dllexport )

void Output(const wchar_t* szFormat, ...);
