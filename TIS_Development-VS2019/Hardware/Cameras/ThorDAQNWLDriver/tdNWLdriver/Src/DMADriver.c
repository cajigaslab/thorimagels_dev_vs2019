// -------------------------------------------------------------------------
//
// PRODUCT:			DMA Driver
// MODULE NAME:		DMADriver.c
//
// MODULE DESCRIPTION:
//
// Contains the DriverEntry and other system function calls
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
#include "DMADriver.tmh"
#endif // TRACE_ENABLED


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, DMADriverEvtDeviceAdd)
#endif


//  Windows XP component IDs for DbgPrintEx are different than later
//  Windows version.  We have to query the OS on start up and adjust the
//  component ID if necessary.
 UINT32 DbgComponentID	=	DPFLTR_IHVDRIVER_ID;

// Local Prototypes

/*! DriverEntry
 *
 *	\brief DriverEntry - This is the starting point for the driver.
 *	It is called once when the driver is loaded.
 *	\param DriverObject - Pointer the the Driver Object related to this instaciation
 *	\param RegistryPath - Pointer to the Registry path associated to this driver
 *	\return NTSTATUS - STATUS_SUCCESS - Successfully completed
 */
NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT		DriverObject,								// Driver object.
	IN PUNICODE_STRING		RegistryPath								// Driver-spec registry key.
)
{
	NTSTATUS				status = STATUS_SUCCESS;
	WDF_DRIVER_CONFIG		config;
	WDF_OBJECT_ATTRIBUTES	attributes;

#if TRACE_ENABLED
	/* Initialize Tracing */
	WPP_INIT_TRACING(DriverObject, RegistryPath);
#else
	RTL_OSVERSIONINFOEXW	osver;
	osver.dwOSVersionInfoSize = sizeof(osver);
	if (NT_SUCCESS(RtlGetVersion((PRTL_OSVERSIONINFOW)&osver)))
	{
		if ((osver.dwMajorVersion == 5) && (osver.dwMinorVersion == 1))
		{
			// Adjust the ID for Windows XP.
			DbgComponentID = DPFLTR_IHVDRIVER_ID + 2;
		}
	}
#endif // TRACE_ENABLED

	//	DEBUGP(DEBUG_ALWAYS, "Northwest Logic DMADriver - Built %s %s", __DATE__, __TIME__);

	// Initialize the driver configuration structure
	WDF_DRIVER_CONFIG_INIT(&config, DMADriverEvtDeviceAdd);
	config.DriverPoolTag = 'amDP';

	// Setup attributes
	WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
	// Setup Context Cleanup so WPP_CLEANUP is called on driver unload
	attributes.EvtCleanupCallback = DMADriverEvtDriverContextCleanup;

	// Create a driver object
	status = WdfDriverCreate(
					DriverObject,
					RegistryPath,
					&attributes,
					&config,
					WDF_NO_HANDLE
	);

	if (!NT_SUCCESS(status))
	{
		DEBUGP(DEBUG_ERROR,	"Driver Create failed, status of 0x%x", status);
#if TRACE_ENABLED
		WPP_CLEANUP(DriverObject);
#endif // TRACE_ENABLED
		return status;
	}

	DbgPrint("NWL driverEntry return");

	return status;
}

/*! DMADriverEvtDeviceAdd
 *
 * 	\brief DMADriverEvtDeviceAdd - This is called only once, when the system
 *	requests a device to be added. This called once when the driver is loaded.
 *  \param Driver - WDF Driver Object associated to this driver
 *  \param DeviceInit - Pointer to the WDF Device Init object
 *  \return NTSTATUS - STATUS_SUCCESS - Successfully completed
 */
NTSTATUS
DMADriverEvtDeviceAdd(
	IN WDFDRIVER			Driver,
	IN PWDFDEVICE_INIT		DeviceInit
)
{
	NTSTATUS 				status = STATUS_SUCCESS;
	WDF_OBJECT_ATTRIBUTES	attributes;
	PDEVICE_EXTENSION       pDevExt = NULL;
	WDFDEVICE               device;
	WDF_PNPPOWER_EVENT_CALLBACKS	pnpPowerCallbacks;

	UNREFERENCED_PARAMETER(Driver);

	PAGED_CODE();

	DEBUGP(DEBUG_TRACE, "--> DMADriverEvtDeviceAdd");

	// Register a pre-processing callback to do the page locking before it gets on the I/O queue
	WdfDeviceInitSetIoInCallerContextCallback(DeviceInit, DMADriverIoInCallerContext);

	// Set this up as a direct IO device
	WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoDirect);

	// Setup PNP Callbacks
	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
	// PNP PrepareHardware and ReleaseHardware Callbacks - map hardware
	pnpPowerCallbacks.EvtDevicePrepareHardware = DMADriverEvtDevicePrepareHardware;
	pnpPowerCallbacks.EvtDeviceReleaseHardware = DMADriverEvtDeviceReleaseHardware;
	// PNP D0Entry and D0Exit Event Callbacks - init hardware
	pnpPowerCallbacks.EvtDeviceD0Entry = DMADriverEvtDeviceD0Entry;
	pnpPowerCallbacks.EvtDeviceD0Exit  = DMADriverEvtDeviceD0Exit;
    // Initialize Self Managed IO callbacks.
    // These are used for watchdog timer routines. (Cleanup used also)
    pnpPowerCallbacks.EvtDeviceSelfManagedIoRestart	= DMADriverEvtDeviceSelfManagedIoRestart;
	// PNP Self Managed IO Cleanup - cleanup context
    pnpPowerCallbacks.EvtDeviceSelfManagedIoCleanup	= DMADriverEvtDeviceSelfManagedIoCleanup;

	WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

	// Initialize FDO attributes
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, DEVICE_EXTENSION);

	// Setup for device synchronization
	attributes.SynchronizationScope = WdfSynchronizationScopeDevice;
	// Create Device
	status = WdfDeviceCreate(&DeviceInit, &attributes, &device);

	if (!NT_SUCCESS(status)) {
		DEBUGP(DEBUG_ERROR,	"DeviceCreate failed 0x%x", status);
		return status;
	}

	// Setup device context
	pDevExt = DMADriverGetDeviceContext(device);

	pDevExt->Device = device;
	pDevExt->PhysicalDeviceObject = WdfDeviceWdmGetPhysicalDevice(device);
	pDevExt->FunctionalDeviceObject = WdfDeviceWdmGetDeviceObject(device);

	DEBUGP(DEBUG_INFO, "     AddDevice PDO (0x%p) FDO (0x%p) DevExt (0x%p)",
			pDevExt->PhysicalDeviceObject,
			pDevExt->FunctionalDeviceObject, pDevExt);

	// Setup device interface
	status = WdfDeviceCreateDeviceInterface(device,
			(LPGUID) &GUID_DMA_DRIVER_INTERFACE, NULL);

	if (!NT_SUCCESS(status)) {
		DEBUGP(DEBUG_ERROR, "<-- CreateDeviceInterface failed 0x%x", status);
		return status;
	}

	// Initialize interface
	status = DMADriverInitializeDeviceExtension(pDevExt);

	DEBUGP(DEBUG_TRACE,	"<-- DMADriverDeviceAdd status 0x%x", status);

	return status;
}

/*! DMADriverEvtDevicePrepareHardware
 *
 * 	\brief DMADriverEvtDevicePrepareHardware - This routine is called when a
 *	device starts or restarts.
 *	This routine sets up any mapping required
 *	by the hardware.  The actual mapping will be accomplished in
 *	DMADriverPrepareHardware.
 *  \param Driver - WDF Driver Object associated to this driver
 *  \param DeviceInit - Pointer to the WDF Device Init object
 *  \return NTSTATUS - STATUS_SUCCESS - Successfully completed
 */
NTSTATUS
DMADriverEvtDevicePrepareHardware(
	WDFDEVICE			Device,
	WDFCMRESLIST		Resources,
	WDFCMRESLIST		ResourcesTranslated
)
{
	NTSTATUS			status = STATUS_SUCCESS;
	PDEVICE_EXTENSION	pDevExt;

	UNREFERENCED_PARAMETER(Resources);

	DEBUGP(DEBUG_TRACE, "--> DMADriverEvtDevicePrepareHardware");

	pDevExt = DMADriverGetDeviceContext(Device);

	 // setup the mapping for the hardware
	status = DMADriverPrepareHardware (pDevExt,	ResourcesTranslated);

	DEBUGP(DEBUG_TRACE, "<-- DMADriverEvtDevicePrepareHardware 0x%x", status);

	return status;
}

/*! DMADriverEvtDeviceReleaseHardware
 *
 *	\brief DMADriverEvtDeviceReleaseHardware - This routine is called when a
 *   device is resource rebalanced, surprise removed or query removed.
 *	\param Device
 *	\param ResourcesTranslated
 *	\return status
 */
NTSTATUS
DMADriverEvtDeviceReleaseHardware(
	WDFDEVICE			Device,
	WDFCMRESLIST		ResourcesTranslated
)
{
	NTSTATUS			status = STATUS_SUCCESS;
	PDEVICE_EXTENSION	pDevExt;
	UINT8				i;

	UNREFERENCED_PARAMETER(ResourcesTranslated);

	DEBUGP(DEBUG_TRACE, "--> DMADriverEvtDeviceReleaseHardware");

	pDevExt = DMADriverGetDeviceContext(Device);

	// Need to release the DMA Engine Resources and Contexts

	// search for active dmaEngine
    for (i = 0; i < MAX_NUM_DMA_ENGINES; i++)
    {
        if (pDevExt->pDmaEngineDevExt[i] != NULL)
        {
			DmaDriverBoardDmaRelease(pDevExt, i);
		}
	}

	// User Interrupt Support
	// Set Spinlock
	WdfSpinLockAcquire(pDevExt->UsrIntSpinLock);
	// Handle unfinished interrupt request during driver shutdown.
	if (pDevExt->UsrIntRequest != NULL)
	{
		DMADriverUsrIntReqTimerStop(pDevExt);
		// A request is outstanding.
		WdfRequestComplete(pDevExt->UsrIntRequest, STATUS_SUCCESS);
		// Reset pointer back to NULL.
		pDevExt->UsrIntRequest = NULL;
	}
	// Release spinlock.
	WdfSpinLockRelease(pDevExt->UsrIntSpinLock);

	DMADriverUsrIntReqTimerDelete(pDevExt);

	// User Interrupt Spinlock Shutdown
	if (pDevExt->UsrIntSpinLock != 0)
	{
		// Delete the User Interrupt SpinLock; set the pointer to 0.
		WdfObjectDelete(pDevExt->UsrIntSpinLock);
		pDevExt->UsrIntSpinLock = 0;
	}

	// Unmap anything mapped in DMADriverDevicePrepareHardware
	for (i=0; i<pDevExt->NumberOfBARS; i++) {
		if (((pDevExt->BarType[i] == CmResourceTypeMemory) ||
				(pDevExt->BarType[i] == CmResourceTypeMemoryLarge)) &&
				(pDevExt->BarVirtualAddress[i] != NULL))
		{
			// Unmap this memory space
			MmUnmapIoSpace(pDevExt->BarVirtualAddress[i],
						   (size_t) pDevExt->BarLength[i]);
			pDevExt->BarVirtualAddress[i] = NULL;
		}
	}

	// clean up DMA Registers
	pDevExt->pDmaRegisters = NULL;
	pDevExt->pADC_S2MM_DMAcontrolStatus = NULL;

	DEBUGP(DEBUG_TRACE, "<-- DMADriverEvtDeviceReleaseHardware 0x%x",	status);
	return status;
}

/*! DMADriverEvtDeviceD0Entry
*
*	\brief DMADriverEvtDeviceD0Entry - This routine sets up the hardware.
*	It is called when the device enters the D0 (Full Power) state.
*	This routine is called when a device is started, restarted or brought out of
*	powered-down state.
*	Interrupts are disabled when this routine is called.
*	\param Device
*	\param PreviousState
*	\return status
*/
NTSTATUS
DMADriverEvtDeviceD0Entry(
	IN WDFDEVICE					Device,
	IN WDF_POWER_DEVICE_STATE		PreviousState
)
{
	PDEVICE_EXTENSION				pDevExt;
	PDMA_ENGINE_DEVICE_EXTENSION    pDmaExt;
	INT32							i;
	NTSTATUS						status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(PreviousState);

	DEBUGP(DEBUG_TRACE, "--> DMADriverEvtDeviceD0Entry");

	pDevExt = DMADriverGetDeviceContext(Device);

	// search for active dmaEngine
	for (i= 0; i < MAX_NUM_DMA_ENGINES; i++)
	{
		pDmaExt = pDevExt->pDmaEngineDevExt[i];
		if (pDmaExt != NULL)
		{
			// Initialize the S2C hardware
			if (pDmaExt->DmaType == DMA_TYPE_PACKET_SEND)
			{
				pDmaExt->pDmaEng->ControlStatus = 0;
				InitializeTxDescriptors(pDevExt, pDmaExt);
			}
			// Initialize the C2S hardware
			if (pDmaExt->DmaType == DMA_TYPE_PACKET_RECV)
			{
				pDmaExt->pDmaEng->ControlStatus = 0;
				InitializeAddressablePacketDescriptors(pDevExt, pDmaExt);
			}
		}
	}

	// Initialize the Watchdog Timer
	if (NT_SUCCESS(DMADriverWatchdogTimerInit(Device)))
	{
		// Start the Watchdog timer
		DMADriverWatchdogTimerStart(Device);
	}

	DEBUGP(DEBUG_TRACE,  "<-- DMADriverEvtDeviceD0Entry");
	return status;
}

/*! DMADriverEvtDeviceD0Exit
 *
 *	\brief DMADriverEvtDeviceD0Exit - This routines shuts down the hardware.
 *	It is called when the device leaves D0 (Full Power) state.
 *	This routine is called when a device is stopped, removed or powered-off.
 *	Interrupts have been disabled prior to calling this routine.
 *	\param Device
 *	\param TargetState
 *	\return status
 */
NTSTATUS
DMADriverEvtDeviceD0Exit(
	IN WDFDEVICE				Device,
	IN WDF_POWER_DEVICE_STATE	TargetState
)
{
	PDEVICE_EXTENSION			pDevExt;
	NTSTATUS					status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(TargetState);

	DEBUGP(DEBUG_TRACE, "--> DMADriverEvtDeviceD0Exit");

	pDevExt = DMADriverGetDeviceContext(Device);

	// Disable interrupts
	pDevExt->pDmaRegisters->commonControl.ControlStatus = 0;

	// Stop the Watchdog Timer
	DMADriverWatchdogTimerStop(Device);

	DEBUGP(DEBUG_TRACE, "<-- DMADriverEvtDeviceD0Exit");
	return status;
}

/*! DMADriverEvtDeviceSelfManagedIoRestart
 *
 *	\brief DMADriverEvtDeviceSelfManagedIoRestart -  This routine is called
 *	when the driver is restarted. Board enters the working state.
 *
 *	\param Device
 *
 *	\return status
 */
NTSTATUS
DMADriverEvtDeviceSelfManagedIoRestart(
	IN WDFDEVICE	Device
)
{
	NTSTATUS		status = STATUS_SUCCESS;

	DEBUGP(DEBUG_TRACE, "--> DMADriverEvtDeviceSelfManagedIoRestart");

	// Start the Watchdog Timer
	DMADriverWatchdogTimerStart(Device);

	return status;
}

/*! DMADriverEvtDeviceSelfManagedIoCleanup
 *
 *	\brief DMADriverEvtDeviceSelfManagedIoCleanup-  This routine is called
 *	when the device has gone away.
 *	\param Device
 *	\return none
 */
VOID
DMADriverEvtDeviceSelfManagedIoCleanup(
	IN WDFDEVICE		Device
)
{
	PDEVICE_EXTENSION   pDevExt;
	INT32				i;

	DEBUGP(DEBUG_TRACE, "--> DMADriverEvtDeviceSelfManagedIoCleanup");

	// Setup device context
	pDevExt = DMADriverGetDeviceContext(Device);

	// Cleanup the watchdog timer
	DMADriverWatchdogTimerStop(Device);
	DMADriverWatchdogTimerDelete(Device);

	// cleanup DMA structures that are only created once
	for (i=0; i<MAX_NUM_DMA_ENGINES; i++)
	{
		if (pDevExt->pDmaEngineDevExt[i] != NULL)
		{
			// delete the DMA Engine Dev Ext
			ExFreePoolWithTag(pDevExt->pDmaEngineDevExt[i], 'amDP');
			pDevExt->pDmaEngineDevExt[i] = NULL;
		}
	}
	DEBUGP(DEBUG_TRACE, "<-- DMADriverEvtDeviceSelfManagedIoCleanup");
}

/*! DMADriverEvtDriverContextCleanup
*
*	\brief DMADriverEvtDriverContextCleanup - This routine is called when
*	the driver is being unloaded.
*	\param Object
*	\return none
*/
VOID
DMADriverEvtDriverContextCleanup(
	IN WDFOBJECT			Object
)
{
	UNREFERENCED_PARAMETER(Object);

	DEBUGP(DEBUG_TRACE, "<-- DMADriverEvtDriverContextCleanup");

#if TRACE_ENABLED
	// Cleanup the TraceEvents utility
	WPP_CLEANUP(WdfDriverWdmGetDriverObject(Object));
#endif // TRACE_ENABLED
}