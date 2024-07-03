// -------------------------------------------------------------------------
// 
// PRODUCT:			DMA Driver
// MODULE NAME:		Public.h
// 
// MODULE DESCRIPTION: 
// 
// Contains the public defines for the project.
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

#ifndef PUBLIC_H_
#define PUBLIC_H_
#include "../Include/DMADriverIoctl.h"

// {5FAE1782-5E11-42c1-96F0-3A0E99924B4A}
DEFINE_GUID(GUID_DMA_DRIVER_INTERFACE,
	0x823691b8, 0x66b9, 0x4c5c, 0xb8, 0x9a, 0x99, 0xa1, 0x1b, 0xa3, 0xf1, 0xd5);

#endif /* PUBLIC_H_ */
