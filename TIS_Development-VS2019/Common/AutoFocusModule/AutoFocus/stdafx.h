// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <Strsafe.h>
#include <tiffio.h>
#include <iomanip>
#include <atomic>


#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#include "..\..\..\Common\Camera.h"
#include "..\..\..\Common\Command.h"
#include "..\..\..\Common\Device.h"
#include "..\..\..\Common\Experiment.h"
#include "..\..\..\Common\HardwareXML\HardwareSetupXML.h"
#include "..\..\..\Common\Log.h"
#include "..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\Tools\ticpp\ticpprc.h"
#include "..\..\..\Common\ImageManager\ImageManager\Image.h"
#include "..\..\..\Common\ImageManager\ImageManager\ImageManager.h"
#include "..\..\..\Hardware\Devices\DeviceManager\DeviceManager\DeviceManager.h"
#include "..\..\..\Common\ExperimentManager\ExperimentManager\ExperimentManager.h"
#include "..\..\..\Common\HardwareCom\HardwareCom\HardwareComFunctions.h"
#include "..\..\..\Common\ResourceManager\ResourceManager\ResourceManager.h"

#include "..\..\..\Tools\tiff-3.8.2\include\tifflib.h"

//extern auto_ptr<LogDll> logDll;
const long MSG_LENGTH = 256;
static wchar_t msg[MSG_LENGTH];
extern std::auto_ptr<LogDll> logDll;

// TODO: reference additional headers your program requires here
