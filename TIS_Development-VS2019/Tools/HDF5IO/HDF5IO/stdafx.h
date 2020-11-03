// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <list>
#include <tchar.h>
#include <memory>
#include <ObjBase.h>


#pragma warning(push)
#pragma warning(disable:4251)
#include "hdf5.h"
#include "H5Cpp.h"
#include "HDF5io.h"
#pragma warning(pop)


#define LOGGING_ENABLED
#include "..\..\..\..\Common\Log.h"

#define MSG_LENGTH			256
#define SEC2MSEC			1000

#define HDF5DLL_EXPORTS

// TODO: reference additional headers your program requires here
