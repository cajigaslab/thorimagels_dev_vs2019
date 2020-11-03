// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <Shlobj.h>

#include <string>
#include <memory>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <bitset>
#include <map>
#include <vector>


#include <windows.h>
#include <stdio.h>
//#include <stdlib.h>
//#include <tchar.h>
#include <string>
#include <memory>
#include <time.h>
#include <vector>
#include <malloc.h>
#include "Strsafe.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <direct.h>
#include <intrin.h>
#include <mutex>
#include <math.h>
#include <thread>
#include <bitset>
#include <atomic>


using namespace std;

#define STATS_MANAGER

#include "..\..\Log.h"

extern auto_ptr<LogDll> logDll;
extern wchar_t message[256];


// TODO: reference additional headers your program requires here
