#include "Driver.h"

// ULONG32WatchdogTimerInit
//
// Initialize the Watchdog timer.  Do not start it.
NTSTATUS
ThorDaqDrvWatchdogTimerInit (
		IN WDFDEVICE Device
		)
{
	PDEVICE_EXTENSION       pDevExt;
	WDF_TIMER_CONFIG		wdfTimerConfig;
	WDF_OBJECT_ATTRIBUTES	timerAttributes;
	NTSTATUS	status = STATUS_SUCCESS;

	// Setup device context
	pDevExt = ThorDaqDrvGetDeviceContext(Device);

	WDF_TIMER_CONFIG_INIT_PERIODIC(&wdfTimerConfig,
									ThorDaqDrvWatchdogTimerCall,
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

// Watchdog timer
//
// Check for a stalled card
VOID ThorDaqDrvWatchdogTimerCall (
	IN WDFTIMER Timer
	)
{
	int 					dmaEngine;
	PDEVICE_EXTENSION       pDevExt;
	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;

	// Setup device context
	pDevExt = ThorDaqDrvGetDeviceContext(WdfTimerGetParentObject(Timer));

	for (dmaEngine = 0; dmaEngine < MAX_NUM_DMA_ENGINES; dmaEngine++)
	{
		if (pDevExt->pDmaEngineDevExt[dmaEngine] != NULL)
		{
			pDmaExt = pDevExt->pDmaEngineDevExt[dmaEngine];
			pDevExt->pDmaEngineDevExt[dmaEngine]->HardwareTimeInLastSecond =
				(ULONG64)pDmaExt->pDmaEng->Packet.DMAActiveTime;
			pDevExt->pDmaEngineDevExt[dmaEngine]->DMAInactiveTime = 
				(ULONG64)pDmaExt->pDmaEng->Packet.DMAWaitTime;
			pDevExt->pDmaEngineDevExt[dmaEngine]->BytesInLastSecond = 
				(ULONG64)pDmaExt->pDmaEng->Packet.DMACompletedByteCount;
		}
	}
}

// ULONG32WatchdogTimerStart
//
// Start the Watchdog timer.
VOID
ThorDaqDrvWatchdogTimerStart (
		IN WDFDEVICE Device
		)
{
	PDEVICE_EXTENSION       pDevExt;

	// Setup device context
	pDevExt = ThorDaqDrvGetDeviceContext(Device);

	if (pDevExt->WatchdogTimer)
	{
		WdfTimerStart(
			pDevExt->WatchdogTimer,
			 WDF_REL_TIMEOUT_IN_MS(990)
			 );
	}
}

// ULONG32WatchdogTimerStop
//
// Stop the Watchdog timer.
VOID
ThorDaqDrvWatchdogTimerStop (
		IN WDFDEVICE Device
		)
{
	PDEVICE_EXTENSION       pDevExt;

	// Setup device context
	pDevExt = ThorDaqDrvGetDeviceContext(Device);

	if (pDevExt->WatchdogTimer)
	{
		WdfTimerStop(
			pDevExt->WatchdogTimer,
			 TRUE			// wait
			 );
	}
}

// ULONG32WatchdogTimerStop
//
// Stop the Watchdog timer.
VOID
ThorDaqDrvWatchdogTimerDelete (
		IN WDFDEVICE Device
		)
{
	PDEVICE_EXTENSION       pDevExt;

	// Setup device context
	pDevExt = ThorDaqDrvGetDeviceContext(Device);

	if (pDevExt->WatchdogTimer)
	{
		WdfObjectDelete(pDevExt->WatchdogTimer);
	}
}
