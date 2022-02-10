/*++

Module Name:

    driver.c

Abstract:

    This file contains the driver entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"
//#include "driver.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, ThorDaqDrvEvtDeviceAdd)
#pragma alloc_text (PAGE, ThorDaqDrvEvtDriverContextCleanup)
#endif

//  Windows XP component IDs for DbgPrintEx are different than later
//  Windows version.  We have to query the OS on start up and adjust the
//  component ID if necessary.
UINT32 DbgComponentID = DPFLTR_IHVDRIVER_ID;


NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:
    DriverEntry initializes the driver and is the first routine called by the
    system after the driver is loaded. DriverEntry specifies the other entry
    points in the function driver, such as EvtDevice and DriverUnload.

Parameters Description:

    DriverObject - represents the instance of the function driver that is loaded
    into memory. DriverEntry must initialize members of DriverObject before it
    returns to the caller. DriverObject is allocated by the system before the
    driver is loaded, and it is released by the system after the system unloads
    the function driver from memory.

    RegistryPath - represents the driver specific path in the Registry.
    The function driver can use the path to store driver related data between
    reboots. The path does not store hardware instance specific data.

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise.

--*/
{
    WDF_DRIVER_CONFIG config;
    NTSTATUS status;
    WDF_OBJECT_ATTRIBUTES attributes;


    //
    // Register a cleanup callback so that we can call WPP_CLEANUP when
    // the framework driver object is deleted during driver unload.
    //
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = ThorDaqDrvEvtDriverContextCleanup;


    WDF_DRIVER_CONFIG_INIT(&config,
                           ThorDaqDrvEvtDeviceAdd
                           );
	config.DriverPoolTag = 'amDP';

    status = WdfDriverCreate(DriverObject,
                             RegistryPath,
                             &attributes,
                             &config,
                             WDF_NO_HANDLE
                             );

    if (!NT_SUCCESS(status)) {
        return status;
    }

    return status;
}

NTSTATUS
ThorDaqDrvEvtDeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
/*++
Routine Description:

    EvtDeviceAdd is called by the framework in response to AddDevice
    call from the PnP manager. We create and initialize a device object to
    represent a new instance of the device.

Arguments:

    Driver - Handle to a framework driver object created in DriverEntry

    DeviceInit - Pointer to a framework-allocated WDFDEVICE_INIT structure.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS status;
	WDF_OBJECT_ATTRIBUTES	attributes;
	PDEVICE_EXTENSION       pDevExt = NULL;
	WDFDEVICE               device;
	WDF_PNPPOWER_EVENT_CALLBACKS	pnpPowerCallbacks;

    UNREFERENCED_PARAMETER(Driver);

    PAGED_CODE();

	// Register a pre-processing callback to do the page locking before it gets on the I/O queue
	WdfDeviceInitSetIoInCallerContextCallback(DeviceInit, ThorDaqDrvIoInCallerContext);

		// Set this up as a direct IO device
	WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoDirect);

	// Setup PNP Callbacks
	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
	// PNP PrepareHardware and ReleaseHardware Callbacks - map hardware
	pnpPowerCallbacks.EvtDevicePrepareHardware = ThorDaqDrvEvtDevicePrepareHardware;
	pnpPowerCallbacks.EvtDeviceReleaseHardware = ThorDaqDrvEvtDeviceReleaseHardware;
	// PNP D0Entry and D0Exit Event Callbacks - init hardware
	pnpPowerCallbacks.EvtDeviceD0Entry = ThorDaqDrvEvtDeviceD0Entry;
	pnpPowerCallbacks.EvtDeviceD0Exit  = ThorDaqDrvEvtDeviceD0Exit;
    // Initialize Self Managed IO callbacks.
    // These are used for watchdog timer routines. (Cleanup used also)
    pnpPowerCallbacks.EvtDeviceSelfManagedIoRestart	= ThorDaqDrvEvtDeviceSelfManagedIoRestart;
	// PNP Self Managed IO Cleanup - cleanup context
    pnpPowerCallbacks.EvtDeviceSelfManagedIoCleanup	= ThorDaqDrvEvtDeviceSelfManagedIoCleanup;

	WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

	// Initialize FDO attributes
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, DEVICE_EXTENSION);

	// Setup for device synchronization
	attributes.SynchronizationScope = WdfSynchronizationScopeDevice;
	// Create Device
	status = WdfDeviceCreate(&DeviceInit, &attributes, &device);

	if (!NT_SUCCESS(status)) {
		return status;
	}

	// Setup device context
	pDevExt = ThorDaqDrvGetDeviceContext(device);

	pDevExt->Device = device;
	pDevExt->PhysicalDeviceObject = WdfDeviceWdmGetPhysicalDevice(device);
	pDevExt->FunctionalDeviceObject = WdfDeviceWdmGetDeviceObject(device);

	// Setup device interface
	status = WdfDeviceCreateDeviceInterface(device,
			(LPGUID) &GUID_DEVINTERFACE_ThorDaqDrv, NULL);

	if (!NT_SUCCESS(status)) {
		return status;
	}

	// Initialize interface
	status = ThorDaqDrvInitializeDeviceExtension(pDevExt);

    //status = ThorDaqDrvCreateDevice(DeviceInit);


    return status;
}

/*!***************************************************************************
*
* 	\brief ThorDaqDrvEvtDevicePrepareHardware - This routine is called when a 
*		device starts or restarts. This routine sets up any mapping required 
*		by the hardware.  The actual mapping will be accomplished in 
*		ThorDaqDrvPrepareHardware.
*	 
*   \param Driver - WDF Driver Object associated to this driver
*   \param DeviceInit - Pointer to the WDF Device Init object
* 
*   \return NTSTATUS - STATUS_SUCCESS - Successfully completed
* 
*****************************************************************************/
NTSTATUS ThorDaqDrvEvtDevicePrepareHardware(

		WDFDEVICE		Device,
		WDFCMRESLIST	Resources,
		WDFCMRESLIST	ResourcesTranslated
		)
{
	NTSTATUS			status = STATUS_SUCCESS;
	PDEVICE_EXTENSION	pDevExt;

	UNREFERENCED_PARAMETER(Resources);

	pDevExt = ThorDaqDrvGetDeviceContext(Device);

	 // setup the mapping for the hardware
	status = ThorDaqDrvPrepareHardware (pDevExt,	ResourcesTranslated);

	return status;
}

// ThorDaqDrvEvtDeviceReleaseHardware
//
// This routine is called when a device is resource rebalanced,
// surprise removed or query removed
NTSTATUS
ThorDaqDrvEvtDeviceReleaseHardware (
		WDFDEVICE		Device,
		WDFCMRESLIST	ResourcesTranslated
		)
{
	NTSTATUS			status = STATUS_SUCCESS;
	PDEVICE_EXTENSION	pDevExt;
	ULONG				i;

	UNREFERENCED_PARAMETER(ResourcesTranslated);

	pDevExt = ThorDaqDrvGetDeviceContext(Device);

	// Need to release the DMA Engine Resources and Contexts
	
	// Unmap anything mapped in ThorDaqDrvDevicePrepareHardware
	for (i = 0; i< pDevExt->NumberOfBARS; i++) {
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

	return status;
}


// ThorDaqDrvEvtDeviceD0Entry
//
// This routine sets up the hardware.  It is called when the device enters the D0 state.
// This routine is called when a device is started, restarted or brought out of
// powered-down state.
// Interrupts are disabled when this routine is called.
//
NTSTATUS ThorDaqDrvEvtDeviceD0Entry (
		IN WDFDEVICE				Device,
		IN WDF_POWER_DEVICE_STATE	PreviousState
		)
{
	PDEVICE_EXTENSION	pDevExt;
	PDMA_ENGINE_DEVICE_EXTENSION    pDmaExt;
	int i;
	NTSTATUS	status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(PreviousState);

	pDevExt = ThorDaqDrvGetDeviceContext(Device);

	// search for active dmaEngine
	for (i = 0; i < MAX_NUM_DMA_ENGINES; i++)
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
	if (NT_SUCCESS(ThorDaqDrvWatchdogTimerInit(Device)))
	{
		// Start the Watchdog timer
		ThorDaqDrvWatchdogTimerStart(Device);
	}
	return status;
}


// ThorDaqDrvEvtDeviceD0Exit
//
// This routines shuts down the hardware.  It is called when the device leaves D0 state.
// This routine is called when a device is stopped, removed or powered-off.
// Interrupts have been disable prior to calling this routine
//
NTSTATUS ThorDaqDrvEvtDeviceD0Exit (
		IN WDFDEVICE				Device,
		IN WDF_POWER_DEVICE_STATE	TargetState
		)
{
	PDEVICE_EXTENSION	pDevExt;
	NTSTATUS	status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(TargetState);

	pDevExt = ThorDaqDrvGetDeviceContext(Device);

	// Disable interrupts
	pDevExt->pDmaRegisters->commonControl.ControlStatus = 0;

	// Stop the Watchdog Timer
	ThorDaqDrvWatchdogTimerStop(Device);

	return status;
}


// ThorDaqDrvEvtDeviceSelfManagedIoRestart
//
// This routine is called when the driver is restarted.
// (Board enters the working state.)
NTSTATUS ThorDaqDrvEvtDeviceSelfManagedIoRestart (
		IN WDFDEVICE Device
		)
{
	NTSTATUS	status = STATUS_SUCCESS;

	// Start the Watchdog Timer
	ThorDaqDrvWatchdogTimerStart(Device);

	return status;
}



// ThorDaqDrvEvtDeviceSelfManagedIoCleanup
//
// This routine is called when the device has gone away.
VOID ThorDaqDrvEvtDeviceSelfManagedIoCleanup (
		IN WDFDEVICE Device
		)
{
	PDEVICE_EXTENSION       pDevExt;
	int i;

	// Setup device context
	pDevExt = ThorDaqDrvGetDeviceContext(Device);

	// Cleanup the watchdog timer
	ThorDaqDrvWatchdogTimerStop(Device);
	ThorDaqDrvWatchdogTimerDelete(Device);

	// cleanup DMA structures that are only created once
	for (i = 0; i < MAX_NUM_DMA_ENGINES; i++)
	{
		if (pDevExt->pDmaEngineDevExt[i] != NULL)
		{
			// delete the DMA Engine Dev Ext
			ExFreePoolWithTag(pDevExt->pDmaEngineDevExt[i], 'amDP');
			pDevExt->pDmaEngineDevExt[i] = NULL;
		}
	}
	ExFreePoolWithTag(pDevExt->pS2mmLayerExt, 'amDP');
}

VOID
ThorDaqDrvEvtDriverContextCleanup(
    _In_ WDFOBJECT DriverObject
    )
/*++
Routine Description:

    Free all the resources allocated in DriverEntry.

Arguments:

    DriverObject - handle to a WDF Driver object.

Return Value:

    VOID.

--*/
{
    UNREFERENCED_PARAMETER(DriverObject);

    PAGED_CODE ();


}
