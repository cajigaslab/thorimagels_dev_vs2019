///////////////////////////////////////////////////////////////////////////////
//
//  $File: //depot/DIA-DEV/EAGLE/Main/SRC/rdi_drvr/rdidrvr_leica/stdafx.h $
//
//  $Author: rwinkler $
//
//  $DateTime: 2008/04/19 19:50:02 $
//
//  $Revision: #3 $
//
//  $Change: 2218 $
///////////////////////////////////////////////////////////////////////////////
//
//  COPYRIGHT (C) 1995-2008 Definiens AG
//  All rights reserved
//
//  Definiens AG
//  Trappentreustr. 1
//  80339 München
//
///////////////////////////////////////////////////////////////////////////////

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A6A3E6FE_F499_4BA1_93DD_E1059CE5A7E0__INCLUDED_)
#define AFX_STDAFX_H__A6A3E6FE_F499_4BA1_93DD_E1059CE5A7E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

//#include <stdlib.h>
//#include <tchar.h>
#include <string>
#include <memory>
#include <time.h>
#include <vector>
#include <malloc.h>
//#include "Strsafe.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <direct.h>
#include <intrin.h>
#include <mutex>
#include <math.h>
//#include <thread>
#include <bitset>
#pragma warning (disable:4275) // complain about dll-interface of class DString
#pragma warning (disable:4251) // complain about dll-interface of soap

#endif /*WIN32*/

//#include "OSWrapper/stdwrp.h"
#include <stdio.h>
#include <stdlib.h>
#include "assert.h"

#ifndef ASSERT
	#define ASSERT assert
#endif

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A6A3E6FE_F499_4BA1_93DD_E1059CE5A7E0__INCLUDED_)
