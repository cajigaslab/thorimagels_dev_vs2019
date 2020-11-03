// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOMINMAX

// Windows Header Files:
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <list>
#include <tchar.h>
#include <memory>
#include <time.h>
#include <map>
#include <chrono>
#include <ObjBase.h>

//using namespace std;

//#define LOGGING_ENABLED
#include "..\..\..\..\Common\Log.h"
#include "..\..\..\..\Tools\Intel IPP\intel64\include\IPPlib.h"
#include "..\..\..\..\Common\exprtk.hpp"
#include "..\..\..\..\Common\ThreadSafeQueue.h"

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"
#include "..\..\..\..\Tools\National Instruments\NIDAQmx\include\NIDAQmx.h"

#define DllExport_AcquireData extern "C" __declspec( dllexport )

#include "ConstDefinition.h"
#include "CompoundData.h"
#include "FreqCompoundData.h"
#include "..\..\..\..\Common\CircularBuffer.h"
#include "VirtualChannelManager.h"
#include "SpectralManager.h"
#include "PublicType.h"
#include "..\..\..\..\GUI\Controls\RealTimeLineChart\RealTimeLineChart\PublicEnum.cs"
#include "..\..\..\..\Common\HDF5IOdll.h"
#include "..\..\..\..\Common\StringCPP.h"

// TODO: reference additional headers your program requires here
