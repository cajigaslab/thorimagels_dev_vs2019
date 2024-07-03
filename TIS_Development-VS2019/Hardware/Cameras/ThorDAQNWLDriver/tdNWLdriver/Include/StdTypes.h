// -------------------------------------------------------------------------
// 
// PRODUCT:			DMA Driver
// MODULE NAME:		StdTypes.h
// 
// MODULE DESCRIPTION: 
// 
// Contains standard c extension defines and typedefs.
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

/*! @file StdTypes.h
*/

#ifndef __STANDARD_TYPES__h_
#define __STANDARD_TYPES__h_

//#include <stdint.h>

/*! \note 
 * Type Convention
 * Because this driver is used on mutliple platforms, the ability to use
 * symmetrical code is emphasized. As a result, the DMADriver uses a 
 * specific type standard common across the Expresso PCI Express solution.
 * To eliminate ambiguity, the following standard is used as it most 
 * appropriately describes the data type. 
 * Note: Modifications may require casting in multiple locations and may
 * result in incorrect and/or erroneous driver behavior. 
 */
 
// Unsigned-Integer Values
#ifndef UINT64
typedef unsigned long long		UINT64;
#endif	// end UINT64 typedef

#ifndef UINT32
typedef unsigned int			UINT32;
#endif	// end UINT32 typedef

#ifndef UINT16
typedef unsigned short			UINT16;
#endif	// end UINT16 typedef

#ifndef UINT8
typedef unsigned char			UINT8;
#endif	// end UINT8 typedef

// Pointer-to-Unsigned-Integer Values
#ifndef PUINT64
typedef unsigned long long *	PUINT64;
#endif	// end PUINT64 typedef

#ifndef PUINT32
typedef unsigned int *			PUINT32;
#endif	// end PUINT32 typedef

#ifndef PUINT16
typedef unsigned short *		PUINT16;
#endif	// end PUINT16 typedef

#ifndef PUINT8
typedef unsigned char *			PUINT8;
#endif	// end PUINT8 typedef

// Signed-Integer Values
#ifndef INT8
typedef char					INT8;
#endif	// end INT8 typedef

#ifndef INT16
typedef short					INT16;
#endif	// end INT16 typedef

#ifndef INT32
typedef int						INT32;
#endif	// end INT32 typedef

#ifndef INT64
typedef long long    			INT64;
#endif	// end INT64 typedef

#ifndef LONG
typedef long           		    LONG;
#endif	// end INT64 typedef

// Pointer-to-Signed-Integers
#ifndef PINT8
typedef char *					PINT8;
#endif	// end PINT8 typedef

#ifndef PINT16 
typedef short *					PINT16;
#endif	// end PINT16 typedef

#ifndef PINT32
typedef int	*					PINT32;
#endif	// end PINT32 typedef

#ifndef PINT64
typedef long long * 	   		PINT64;
#endif	// end PINT64 typedef

#ifndef PLONG
typedef long *          	    PLONG;
#endif	// end PLONG typedef

#ifndef PVOID
typedef void *              	PVOID;
#endif	// end PVOID typedef

#ifndef VOID
typedef void					VOID;
#endif	// end VOID typedef

#ifndef boolean_t
typedef unsigned char       	BOOLEAN;
#endif // end boolean_t typedef

#ifndef DWORD
typedef unsigned long       	DWORD;
#endif	// end DWORD typedef

#ifndef CHAR
typedef char					CHAR;
#endif	// end CHAR typedef

#ifndef UCHAR
typedef unsigned char			UCHAR;
#endif	// end UCHAR typedef

#ifndef PCHAR
typedef char *					PCHAR;
#endif	// end PCHAR typedef

#ifndef PUCHAR
typedef unsigned char *			PUCHAR;
#endif	// end PUCHAR typedef

#ifndef TRUE
 #define TRUE            		1
 #define true					TRUE
 #define FALSE              	0
 #define false					FALSE
#endif	// end TRUE / FALSE define.

#ifndef SIZE_T
//#define SIZE_T(arg)       ((size_t)(arg))
#define SIZE_T              size_t
//#typedef size_t 			SIZE_T;
#endif // SIZE_T

#ifndef Sleep
#define Sleep(a)            usleep(a * 1000)
#endif // Sleep

#ifndef SleepEx
#define SleepEx(a, b)       usleep(a * 1000)
#endif // SleepEx

#ifndef LPTHREAD_START_ROUTINE
typedef void *(*LPTHREAD_START_ROUTINE)(void *);
#endif // LPTHREAD_START_ROUTINE

#define	INVALID_HANDLE_VALUE		-1

#endif /* !defined(__STANDARD_TYPES_h__) */

