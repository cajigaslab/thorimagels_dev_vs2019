// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#include <string>
#include <vector>
#include <map>
#include <stdlib.h>
#include <list>
#include <regex>

#include "..\..\..\..\Common\Device.h"
#include "..\..\..\..\Common\SerialPortAccess\Serial.h"
#include "..\..\..\..\Common\ThorSharedTypesCPP.h"

#define MSG_SIZE 256
#define DATA_SIZE 255

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#ifdef LOGGING_ENABLED
#include "..\..\..\..\Common\Log.h"
#endif

#include "..\..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\..\Tools\ticpp\ticpprc.h"

#define DllExport extern "C" long __declspec( dllexport )
// TODO: reference additional headers your program requires here

