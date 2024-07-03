#include "Driver.h"

/*!***************************************************************************
*
*	\brief GetBoardConfigDeviceControl - This routine gets the PCI 
*    			Configuration Space data from the Device Extension and returns 
*			it to the caller.
*	 
*	\param device - The Device object - used to retreive the Device Extensions
* 	\param Request - The I/O Request for the IOCTL call
*   	\param pInfoSize - Pointer to the return size information
* 
*  	\return STATUS_SUCCESS if it works, an error if it fails.
* 
*****************************************************************************/
NTSTATUS GetBoardConfigDeviceControl(
	IN WDFDEVICE	device,
	IN WDFREQUEST  	Request,
	IN size_t *	    	pInfoSize
	)
{
	NTSTATUS 		status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pDevExt;
	PBOARD_CONFIG_STRUCT	pBoardConfig;

	// validate all the parameters
	// Validate pInfoSize
	if (pInfoSize == NULL)
	{
		return STATUS_INVALID_PARAMETER;
	}
	// Get Device Extension
	pDevExt = ThorDaqDrvGetDeviceContext(device);
	if (pDevExt == NULL)
	{
		return STATUS_NO_SUCH_DEVICE;
	}

	// Get the output buffer, where we store the Board Config
	if ((status = WdfRequestRetrieveOutputBuffer(Request,
								   sizeof(BOARD_CONFIG_STRUCT),	/* Min size */
								   (PVOID *) &pBoardConfig,		/* buffer */
								   NULL)) != STATUS_SUCCESS)
	{
		return STATUS_INVALID_PARAMETER;
	}

	if ((status = ThorDaqDrvBoardConfigInit(pDevExt)) != STATUS_SUCCESS)
	{
		return status;
	}

	// Setup the board config
	*pInfoSize = sizeof(BOARD_CONFIG_STRUCT);
	memcpy (pBoardConfig, &pDevExt->BoardConfig, *pInfoSize);

	return status;
}


/*!***************************************************************************
*
*	\brief GetDmaEngineCapabilities = Handles the GET_DMA_ENGINE_CAP_IOCTL 
* 		IOCTL to retrieves the DMA Engine Capabilites from the DMA Engine
*		register and returns it to the caller.
*	 
*	\param device - The Device object - used to retreive the Device Extensions
* 	\param Request - The I/O Request for the IOCTL call
*   	\param pInfoSize - Pointer to the return size information
* 
*  	\return STATUS_SUCCESS if it works, an error if it fails.
* 
*****************************************************************************/
NTSTATUS GetDmaEngineCapabilities (
		IN WDFDEVICE	device,
		IN WDFREQUEST  	Request,
		IN size_t *	    pInfoSize)
{
	NTSTATUS 			status = STATUS_SUCCESS;
	PDEVICE_EXTENSION	pDevExt = ThorDaqDrvGetDeviceContext(device);
	PDMA_CAP_STRUCT		pDMACap;
	ULONG32 *			pDmaEngine;		// var for GetDmaPerformance

	*pInfoSize = 0;
	// Get the input buffer, where we get the DMA Engine number
	status = WdfRequestRetrieveInputBuffer(Request,
							   sizeof(ULONG32),		/* Min size */
							   (PVOID *) &pDmaEngine,	/* buffer */
							   NULL);
	if (status != STATUS_SUCCESS)
	{
		return status;
	}

	// Get the output buffer, where we store the DMA Capabilities info
	if ((status = WdfRequestRetrieveOutputBuffer(Request,
												 sizeof(DMA_CAP_STRUCT),			/* Min size */
												 (PVOID *) &pDMACap,				/* buffer */
												 NULL)) != STATUS_SUCCESS)
	{
		return STATUS_INVALID_PARAMETER;
	}

	// Read the capabilities from the board.
	pDMACap->DmaCapabilities = pDevExt->pDmaRegisters->dmaEngine[*pDmaEngine].Capabilities;
	*pInfoSize = sizeof(DMA_CAP_STRUCT);

	return status;
}


/*!***************************************************************************
*
* 	\brief GetDmaPerfNumbers - This routine handles the
*	GET_PERF_IOCTL IOCTL 
*	 
* 	\param device - The Device object - used to retreive the Device Extensions
* 	\param Request - The I/O Request for the IOCTL call
*   \param pInfoSize - Pointer to the return size information
* 
*   \return STATUS_SUCCESS if it works, an error if it fails.
* 
*****************************************************************************/
NTSTATUS GetDmaPerfNumbers (
		IN WDFDEVICE	device,
		IN WDFREQUEST  	Request,
		IN size_t *	    pInfoSize)
{
	NTSTATUS 			status = STATUS_SUCCESS;
	PDEVICE_EXTENSION	pDevExt = ThorDaqDrvGetDeviceContext(device);
	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;
	ULONG32 *			pDmaEngine;		// var for GetDmaPerformance
	ULONG32				EngineNum;
	PDMA_STAT_STRUCT	pDmaStatStruct;	// var for GetDmaPerformance
	size_t				DmaStatStructSize;

	*pInfoSize = 0;
	// Get the input buffer, where we get the DMA Engine number
	status = WdfRequestRetrieveInputBuffer(Request,
										   sizeof(ULONG32),		/* Min size */
										   (PVOID *) &pDmaEngine,	/* buffer */
										   NULL);
	if (status != STATUS_SUCCESS)
	{
		return status;
	}
	EngineNum = *pDmaEngine;
	status = GetDMAEngineContext(pDevExt, EngineNum, &pDmaExt);
	if (status != STATUS_SUCCESS)
	{
		return status;
	}
	// Get the output buffer, where we store the DMA performance
	status = WdfRequestRetrieveOutputBuffer(Request,
											(sizeof(ULONGLONG)*3),		/* Min size */
											(PVOID *) &pDmaStatStruct,		/* buffer */
											&DmaStatStructSize);
	if (status != STATUS_SUCCESS)
	{
		return status;
	}

	// check EngineNum
	if (pDmaStatStruct != NULL)
	{
		// get the DMA Performance numbers for Packet Mode.
		// Get the numbers that have been captured by the watchdog timer. 
		pDmaStatStruct->HardwareTime = pDmaExt->HardwareTimeInLastSecond;
		pDmaStatStruct->DriverTime = pDmaExt->DMAInactiveTime;
		pDmaStatStruct->CompletedByteCount = pDmaExt->BytesInLastSecond;
		if (DmaStatStructSize == sizeof(DMA_STAT_STRUCT))
		{
			pDmaStatStruct->IntsPerSecond = pDmaExt->IntsInLastSecond;
			pDmaStatStruct->DPCsPerSecond = pDmaExt->DPCsInLastSecond;
			WdfSpinLockAcquire(pDmaExt->HeadSpinLock);
			pDmaExt->IntsInLastSecond = 0;
			pDmaExt->DPCsInLastSecond = 0;
			WdfSpinLockRelease(pDmaExt->HeadSpinLock);
		}
		else
		{
			status = STATUS_INVALID_PARAMETER;
		}
		*pInfoSize = DmaStatStructSize;
	}
	else
	{
		status = STATUS_NO_SUCH_DEVICE;
	}
	return status;
}

