// -------------------------------------------------------------------------
// 
// PRODUCT:			DMA Driver
// MODULE NAME:		trace.h
// 
// MODULE DESCRIPTION: 
// 
// Contains the trace defs for the project.
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
//
// 	Portions copyright Pro Code Works, LLC.
//

#ifndef TRACE_H_
#define TRACE_H_

//#define		TRACE_ENABLED		0

#if TRACE_ENABLED

#include "evntrace.h"  /* Trace Level defs */

#define WPP_CHECK_FOR_NULL_STRINGS  /* prevent exceptions for null pointers */

/* defines for software tracing */
#define WPP_CONTROL_GUIDS \
	WPP_DEFINE_CONTROL_GUID (DMADriverTraceGuid, (5FAE1782,5E11,42c1,96F0,3A0E99924B4A),\
			WPP_DEFINE_BIT(DBG_INIT)		/* bit  0 */ \
			WPP_DEFINE_BIT(DBG_PNP)			/* bit  1 */ \
			WPP_DEFINE_BIT(DBG_POWER)		/* bit  2 */ \
			WPP_DEFINE_BIT(DBG_WMI)			/* bit  3 */ \
			WPP_DEFINE_BIT(DBG_CREATE_CLOSE)/* bit  4 */ \
			WPP_DEFINE_BIT(DBG_IOCTL)		/* bit  5 */ \
			WPP_DEFINE_BIT(DBG_WRITE)		/* bit  6 */ \
			WPP_DEFINE_BIT(DBG_READ)		/* bit  7 */ \
			WPP_DEFINE_BIT(DBG_DMA)			/* bit  8 */ \
			WPP_DEFINE_BIT(DBG_IRQ)			/* bit  9 */ \
			/* up to 32 defines */ \
			)


#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) \
	WPP_LEVEL_LOGGER(flags)
#define WPP_LEVEL_FLAGS_ENABLED(lvl,flags) \
	(WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_## flags).Level >= lvl)

#define	DEBUG_VERBOSE	TRACE_LEVEL_VERBOSE
#define	DEBUG_INFO		TRACE_LEVEL_INFORMATION
#define	DEBUG_TRACE		TRACE_LEVEL_INFORMATION
#define	DEBUG_WARN		TRACE_LEVEL_WARNING
#define	DEBUG_ERROR		TRACE_LEVEL_ERROR

// If you want to use Microsoft's tracing facility you will have to
// search and replace all DEBUGP macros with DEBUGP.  This is due to
// the way the tracing has been implemented. It cannot be used in a macro
// for easy substitution.  We have found that tracing is difficult to setup
// and use. Clients have found it much easier to use the debug print filter 
// method over tracing.
//
//  For example replace:
//		DEBUGP(DEBUG_INFO
//  with:
//		DEBUGP(TRACE_LEVEL_INFORMATION, DBG_INIT
//
// Make DEBUGP macros null 
#define	DEBUGP(level, ...)

#else

extern UINT32 DbgComponentID;

#ifdef DBG
// To use Debug Print open Regedit on the machine running the drivers.
//  Goto HKLM\SYSTEM\CurrentControlSet\Control\Session Manager
//    if the "Debug Print Filter" key does not exist create it.
//    In the HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Debug Print Filter 
//       section add a 32 bit unsigned long value called "IHVDRIVER" or "IHVNETWORK" for NDIS
//       drivers.  Set the value to:
//			1 for ERRORs only,
//			2 for ERRORs and WARNINGs,
//			3 for ERRORs, WARNINGs and Info,
//			4 to receive all messages
//
#define	DEBUG_VERBOSE	DPFLTR_INFO_LEVEL + 1
#define	DEBUG_INFO		DPFLTR_TRACE_LEVEL
#define	DEBUG_TRACE		DPFLTR_INFO_LEVEL
#define	DEBUG_WARN		DPFLTR_WARNING_LEVEL
#define	DEBUG_ERROR		DPFLTR_ERROR_LEVEL
#define	DEBUG_ALWAYS	DPFLTR_ERROR_LEVEL

#define DEBUGP(level, ...) \
{\
	DbgPrintEx(DbgComponentID, level, "TdNWLdriver.SYS:"); \
    DbgPrintEx(DbgComponentID, level, __VA_ARGS__); \
	DbgPrintEx(DbgComponentID, level, "\n"); \
} 

#else // Not debug version

// Make DEBUGP macros null 
#define	DEBUGP(level, ...)

#endif // Debug vs. non debug versions.

#endif // TRACE_ENABLED

#endif /* TRACE_H_ */
