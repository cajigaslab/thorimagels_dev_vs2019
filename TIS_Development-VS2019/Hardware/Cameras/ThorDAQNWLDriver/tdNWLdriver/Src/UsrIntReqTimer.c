// -------------------------------------------------------------------------
// 
// PRODUCT:			DMA Driver
// MODULE NAME:		User Interrupt Request Timer
// 
// MODULE DESCRIPTION: 
// 
// Contains the the routines to run the User Interrupt request timer.
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

#include "precomp.h"

// Prototypes
VOID DMADriverUsrIntReqTimerCall(IN WDFTIMER Timer);

#if TRACE_ENABLED
#include "UserIntReqTimer.tmh"
#endif // TRACE_ENABLED

/*! DMADriverUsrIntReqTimerInit
 *
 * \brief Initialize the UsrIntReq timer at driver load.  
 *  Will not start until 'DMADriverUsrIntReqTimerStart' is called.
 * \param pDevExt
 * \return Status
 */
NTSTATUS 
DMADriverUsrIntReqTimerInit(IN PDEVICE_EXTENSION pDevExt)
{
	WDF_TIMER_CONFIG		wdfTimerConfig;
	WDF_OBJECT_ATTRIBUTES	timerAttributes;
	NTSTATUS				status = STATUS_SUCCESS;

	DEBUGP(DEBUG_INFO, "-->DMADriverUsrIntReqTimerInit()");
	WDF_TIMER_CONFIG_INIT(&wdfTimerConfig,	DMADriverUsrIntReqTimerCall);
	wdfTimerConfig.AutomaticSerialization = TRUE;

	// Setup the driver object as the timer's parent object.
	WDF_OBJECT_ATTRIBUTES_INIT(&timerAttributes);
	timerAttributes.ParentObject = pDevExt->Device;
	//timerAttributes.ExecutionLevel = WdfExecutionLevelPassive;
	// Create the timer object based on the parameters above.
	status = WdfTimerCreate(
								&wdfTimerConfig,
								&timerAttributes,
								&pDevExt->UsrIntReqTimer
						   );
	return status;
}

/*! UsrIntReq timer
 *
 * \brief Check for a parked User Interrupt request
 * \param Timer
 * \return None
 */
VOID 
DMADriverUsrIntReqTimerCall (IN WDFTIMER Timer)
{
	PDEVICE_EXTENSION       pDevExt;
	WDFREQUEST				Request;

	// Setup device context
	pDevExt = DMADriverGetDeviceContext(WdfTimerGetParentObject(Timer));

	// Set Spinlock
	WdfSpinLockAcquire(pDevExt->UsrIntSpinLock);
	Request = pDevExt->UsrIntRequest;
	// Reset pointer back to NULL.
	pDevExt->UsrIntRequest = NULL;
	// Release spinlock.
	WdfSpinLockRelease(pDevExt->UsrIntSpinLock);

	// Make sure there was a pending request
	if (Request != NULL)
	{
		// Return STATUS_TIMEOUT
		WdfRequestComplete(Request, STATUS_IO_TIMEOUT);
	}
}

/*! DMADriverUsrIntReqTimerStart
 *
 * \brief Start the UsrIntReq timer for duration <dwTimeoutMilliSec>.
 * \param pDevExt
 * \param dwTimeoutMilliSec
 * \return None
 */
VOID 
DMADriverUsrIntReqTimerStart(IN PDEVICE_EXTENSION pDevExt, UINT32 dwTimeoutMilliSec)
{
	if (pDevExt->UsrIntReqTimer)
	{
		// Set TimeOut Value Here
		WdfTimerStart(pDevExt->UsrIntReqTimer, WDF_REL_TIMEOUT_IN_MS(dwTimeoutMilliSec));
	}
}

/*! DMADriverUsrIntReqTimerStop
 *
 * \brief Stop the UsrIntReq timer.
 * \param pDevExt
 * \return None
 */
VOID 
DMADriverUsrIntReqTimerStop(IN PDEVICE_EXTENSION pDevExt)
{
	if (pDevExt->UsrIntReqTimer)
	{
	// TRUE = Wait until all queued calls to the driver have executed before returning. 
	WdfTimerStop(pDevExt->UsrIntReqTimer, TRUE);			
	}
}

/*! DMADriverUsrIntReqTimerDelete
 *
 * \brief Remove the UsrIntReq timer.
 *  Used with removing/uninstall/disable the driver.
 * \param pDevExt
 * \return None
 */
VOID 
DMADriverUsrIntReqTimerDelete(IN PDEVICE_EXTENSION pDevExt)
{
	if (pDevExt->UsrIntReqTimer)
	{
		WdfObjectDelete(pDevExt->UsrIntReqTimer);
	}
}
