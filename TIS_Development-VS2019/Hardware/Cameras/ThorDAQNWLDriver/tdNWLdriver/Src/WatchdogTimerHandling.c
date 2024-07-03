// -------------------------------------------------------------------------
// 
// PRODUCT:			DMA Driver
// MODULE NAME:		WatchdogTimerHandling.c
// 
// MODULE DESCRIPTION: 
// 
// Contains the the Watchdog Timer Handling routines.
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

#if TRACE_ENABLED
#include "WatchdogTimerHandling.tmh"
#endif // TRACE_ENABLED

//#ifdef ALLOC_PRAGMA
//#pragma alloc_text (PAGE, DMADriverWatchdogTimerInit)
//#endif  /* ALLOC_PRAGMA */

/*! DMADriverWatchdogTimerInit
 *
 * \brief Initialize the Watchdog timer.  Do not start it.
 * \param Device
 * \return status
 */
NTSTATUS
DMADriverWatchdogTimerInit(
	IN WDFDEVICE			Device
)
{
	PDEVICE_EXTENSION       pDevExt;
	WDF_TIMER_CONFIG		wdfTimerConfig;
	WDF_OBJECT_ATTRIBUTES	timerAttributes;
	NTSTATUS				status = STATUS_SUCCESS;

	// Setup device context
	pDevExt = DMADriverGetDeviceContext(Device);

	WDF_TIMER_CONFIG_INIT_PERIODIC(&wdfTimerConfig,
									DMADriverWatchdogTimerCall,
									990);  // one second
	// setup the driver object as the timer's parent object
	WDF_OBJECT_ATTRIBUTES_INIT(&timerAttributes);
	timerAttributes.ParentObject = pDevExt->Device;

	status = WdfTimerCreate(&wdfTimerConfig,
							&timerAttributes,
							&pDevExt->WatchdogTimer
							);
	return status;
}

/*! Watchdog timer
 *
 * \brief Check for a stalled card
 * \param Timer
 * \return none
 */
VOID 
DMADriverWatchdogTimerCall(
	IN WDFTIMER						Timer	
)
{
	INT32							dmaEngine;
	PDEVICE_EXTENSION				pDevExt;
	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;

	// Setup device context
	pDevExt = DMADriverGetDeviceContext(WdfTimerGetParentObject(Timer));

	for (dmaEngine = 0; dmaEngine < MAX_NUM_DMA_ENGINES; dmaEngine++)
	{
		if (pDevExt->pDmaEngineDevExt[dmaEngine] != NULL)
		{
			pDmaExt = pDevExt->pDmaEngineDevExt[dmaEngine];
			pDevExt->pDmaEngineDevExt[dmaEngine]->HardwareTimeInLastSecond =
				(UINT64)pDmaExt->pDmaEng->DMAActiveTime;
			pDevExt->pDmaEngineDevExt[dmaEngine]->DMAInactiveTime = 
				(UINT64)pDmaExt->pDmaEng->DMAWaitTime;
			pDevExt->pDmaEngineDevExt[dmaEngine]->BytesInLastSecond = 
				(UINT64)pDmaExt->pDmaEng->DMACompletedByteCount;
		}
	}
}

/*! DMADriverWatchdogTimerStart
 *
 * \brief Start the Watchdog timer.
 * \param Device
 * \return none
 */
VOID
DMADriverWatchdogTimerStart(
	IN WDFDEVICE			Device
)
{
	PDEVICE_EXTENSION       pDevExt;

	// Setup device context
	pDevExt = DMADriverGetDeviceContext(Device);

	if (pDevExt->WatchdogTimer)
	{
		WdfTimerStart(
			pDevExt->WatchdogTimer,
			WDF_REL_TIMEOUT_IN_MS(990)
			);
	}
}

/*! DMADriverWatchdogTimerStop
 *
 * \brief Stop the Watchdog timer.
 * \param Device
 * \return none
 */
VOID
DMADriverWatchdogTimerStop(
	IN WDFDEVICE			Device
)
{
	PDEVICE_EXTENSION       pDevExt;

	// Setup device context
	pDevExt = DMADriverGetDeviceContext(Device);

	if (pDevExt->WatchdogTimer)
	{
		WdfTimerStop(pDevExt->WatchdogTimer, TRUE);		// wait
	}
}

/*! DMADriverWatchdogTimerStop 
 *
 * \brief Stop the Watchdog timer.
 * \param Device
 * \return
 */
VOID
DMADriverWatchdogTimerDelete(
	IN WDFDEVICE			Device
)
{
	PDEVICE_EXTENSION       pDevExt;

	// Setup device context
	pDevExt = DMADriverGetDeviceContext(Device);

	if (pDevExt->WatchdogTimer)
	{
		WdfObjectDelete(pDevExt->WatchdogTimer);
	}
}