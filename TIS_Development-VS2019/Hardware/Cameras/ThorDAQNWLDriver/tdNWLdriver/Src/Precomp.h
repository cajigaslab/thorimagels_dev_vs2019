// -------------------------------------------------------------------------
// 
// PRODUCT:			DMA Driver
// MODULE NAME:		Precomp.h
// 
// MODULE DESCRIPTION: 
// 
// Master .h file.
// 
// $Revision:  $
//
// ------------------------- CONFIDENTIAL ----------------------------------
// 
//              Copyright (c) 2016 by Northwest Logic, Inc.   
//                       All rights reserved. 
// 
// Trade Secret of Northwest Logic, Inc.  Do not disclose. 
// 
// Use of this source code in any form or means is permitted only 
// with a valid, written license agreement with Northwest Logic, Inc. 
// 
// Licensee shall keep all information contained herein confidential  
// and shall protect same in whole or in part from disclosure and  
// dissemination to all third parties. 
// 
// 
//                        Northwest Logic, Inc. 
//                  1100 NW Compton Drive, Suite 100 
//                      Beaverton, OR 97006, USA 
//   
//                        Ph:  +1 503 533 5800 
//                        Fax: +1 503 533 5900 
//                      E-Mail: info@nwlogic.com 
//                           www.nwlogic.com 
// 
// -------------------------------------------------------------------------

#define WIN9X_COMPAT_SPINLOCK
#include <ntddk.h>
#pragma warning(disable:4201)  // nameless struct/union warning

#include <stdarg.h>
#include <wdf.h>

#pragma warning(default:4201)

#define WPP_GLOBALLOGGER 1

#if(_WIN32_WINNT < 0x0600)
#define	DRIVER_ALLOCATED_RX_BUFFER_SUPPORT	0 
#endif // WIN_XP

#define __WINNT__			1

#include <initguid.h> // required for GUID definitions
#include <wdmguid.h>  // required for WMILIB_CONTEXT

#include "..\Include\DmaDriverHw.h"
#include "..\Include\StdTypes.h"
#include "Public.h"
#include "Private.h"
#include "..\Src\pci_version.h"
#include "trace.h"


