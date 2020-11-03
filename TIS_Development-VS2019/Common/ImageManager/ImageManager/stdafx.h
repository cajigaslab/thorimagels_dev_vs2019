// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <map>
#include <memory>
#include <vector>
#include <string>
#include <Strsafe.h>
#include <sstream>
#include <iomanip>
using namespace std;

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#define IMAGE_MANAGER



// TODO: reference additional headers your program requires here
#include "..\..\..\Common\ResourceManager\ResourceManager\ResourceManager.h"
#include "..\..\..\Common\ThorSharedTypes\ThorSharedTypes\SharedEnums.cs"