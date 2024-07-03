// -------------------------------------------------------------------------
// 
// PRODUCT:		    DMA Driver
// MODULE NAME:     BoardConfigHandling.c
// 
// MODULE DESCRIPTION: 
// 
// Contains the Board Information IOCtl functions
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
#include "BoardConfigHandling.tmh"
#endif // TRACE_ENABLED

/*! GetBoardConfigDeviceControl
 *
 *  \brief GetBoardConfigDeviceControl - This routine gets the PCI 
 *   Configuration Space data from the Device Extension and returns 
 *	 it to the caller.
 *	 
 *  \param device - The Device object - used to retreive the Device Extensions.
 *  \param Request - The I/O Request for the IOCTL call.
 *  \param pInfoSize - Pointer to the return size information.
 * 
 *  \return status.
 */
NTSTATUS 
GetBoardConfigDeviceControl(
	IN WDFDEVICE			device,
	IN WDFREQUEST  			Request,
	IN size_t *	    		pInfoSize
)
{
	NTSTATUS 				status = STATUS_SUCCESS;
	PDEVICE_EXTENSION		pDevExt;
	PBOARD_CONFIG_STRUCT	pBoardConfig;

	DEBUGP(DEBUG_TRACE,	"--> GetBoardConfigDeviceControl, Request %p", Request);

	// Validate all the parameters.
	// Validate pInfoSize.
	if (pInfoSize == NULL)
	{
		DEBUGP(DEBUG_ERROR, "<-- DMADriverGetDeviceContext returned NULL");
		return STATUS_INVALID_PARAMETER;
	}
	// Get Device Extension.
	pDevExt = DMADriverGetDeviceContext(device);
	if (pDevExt == NULL)
	{
		DEBUGP(DEBUG_ERROR, "<-- DMADriverGetDeviceContext returned NULL");
		return STATUS_NO_SUCH_DEVICE;
	}

	// Get the output buffer, where we store the Board Config.
	if ((status = WdfRequestRetrieveOutputBuffer(Request,
								   sizeof(BOARD_CONFIG_STRUCT),	/* Min size */
								   (PVOID *) &pBoardConfig,		/* buffer	*/
								   NULL)) != STATUS_SUCCESS)
	{
		DEBUGP(DEBUG_ERROR, "<-- WdfRequestRetrieveOutputBuffer failed 0x%x", status);
		return STATUS_INVALID_PARAMETER;
	}

	if ((status = DMADriverBoardConfigInit(pDevExt)) != STATUS_SUCCESS)
	{
		DEBUGP(DEBUG_ERROR, "<-- DMADriverBoardConfigInit failed 0x%x", status);
		return status;
	}

	// Setup the board config
	*pInfoSize = sizeof(BOARD_CONFIG_STRUCT);
	memcpy (pBoardConfig, &pDevExt->BoardConfig, *pInfoSize);

	DEBUGP(DEBUG_TRACE, "<-- GetBoardConfigDeviceControl");

	return status;
}

/*! GetDmaEngineCapabilities
*
*	\brief GetDmaEngineCapabilities = Handles the GET_DMA_ENGINE_CAP_IOCTL 
* 	 IOCTL to retrieves the DMA Engine Capabilites from the DMA Engine
*	 register and returns it to the caller.
*	\param device - The Device object - used to retreive the Device Extensions
* 	\param Request - The I/O Request for the IOCTL call
*   \param pInfoSize - Pointer to the return size information
* 
*	\return STATUS_SUCCESS if it works, an error if it fails.
*/
NTSTATUS 
GetDmaEngineCapabilities(
	IN WDFDEVICE		device,
	IN WDFREQUEST  		Request,
	IN size_t *			pInfoSize
)
{
	NTSTATUS 			status = STATUS_SUCCESS;
	PDEVICE_EXTENSION	pDevExt = DMADriverGetDeviceContext(device);
	PDMA_CAP_STRUCT		pDMACap;
	PUINT32 			pDmaEngine;		// var for GetDmaPerformance

	* pInfoSize = 0;
	// Get the input buffer, where we get the DMA Engine number
	status = WdfRequestRetrieveInputBuffer(Request,
							   sizeof(UINT32),								// Minimum size 
							   (PVOID *) &pDmaEngine,						// Buffer
							   NULL);
	if (status != STATUS_SUCCESS)
	{
		DEBUGP(DEBUG_ERROR, "<-- WdfRequestRetrieveInputBuffer failed 0x%x", status);
		return status;
	}

	// Get the output buffer, where we store the DMA Capabilities info
	if ((status = WdfRequestRetrieveOutputBuffer(Request,
												 sizeof(DMA_CAP_STRUCT),	// Minimum size 
												 (PVOID *) &pDMACap,		// Buffer	
												 NULL)) != STATUS_SUCCESS)
	{
		DEBUGP(DEBUG_ERROR, "<-- WdfRequestRetrieveOutputBuffer failed 0x%x", status);
		return STATUS_INVALID_PARAMETER;
	}

	// Read the capabilities from the board.
	pDMACap->DmaCapabilities = pDevExt->pDmaRegisters->dmaEngine[*pDmaEngine].Capabilities;
	* pInfoSize = sizeof(DMA_CAP_STRUCT);

	return status;
}

/*! GetDmaPerfNumbers
 *
 * 	\brief GetDmaPerfNumbers - This routine handles the
 *   GET_PERF_IOCTL IOCTL 
 *	 
 * 	\param device - The Device object - used to retreive the Device Extensions
 * 	\param Request - The I/O Request for the IOCTL call
 *  \param pInfoSize - Pointer to the return size information
 * 
 *  \return STATUS_SUCCESS if it works, an error if it fails.
 */
NTSTATUS 
GetDmaPerfNumbers (
	IN WDFDEVICE		device,
	IN WDFREQUEST  		Request,
	IN size_t *			pInfoSize
)
{
	NTSTATUS 			status = STATUS_SUCCESS;
	PDEVICE_EXTENSION	pDevExt = DMADriverGetDeviceContext(device);
	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;
	PUINT32 			pDmaEngine;											// var for GetDmaPerformance
	UINT32				EngineNum;
	PDMA_STAT_STRUCT	pDmaStatStruct;										// var for GetDmaPerformance
	size_t				DmaStatStructSize;

	*pInfoSize = 0;
	// Get the input buffer, where we get the DMA Engine number
	status = WdfRequestRetrieveInputBuffer(Request,
											sizeof(UINT32),					// Minimum size 
										   (PVOID *) &pDmaEngine,			// Buffer
										    NULL);
	if (status != STATUS_SUCCESS)
	{
		DEBUGP(DEBUG_ERROR, "<-- WdfRequestRetrieveInputBuffer failed 0x%x", status);
		return status;
	}
	EngineNum = *pDmaEngine;
	status = GetDMAEngineContext(pDevExt, EngineNum, &pDmaExt);
	if (status != STATUS_SUCCESS)
	{
		DEBUGP(DEBUG_ERROR, "DMA Engine number is invalid 0x%x", status);
		return status;
	}
	// Get the output buffer, where we store the DMA performance
	status = WdfRequestRetrieveOutputBuffer(Request,
											(sizeof(UINT64) * 3),			// Minimum size
											(PVOID *) &pDmaStatStruct,		// Buffer
											&DmaStatStructSize);
	if (status != STATUS_SUCCESS)
	{
		DEBUGP(DEBUG_ERROR, "<-- WdfRequestRetrieveOutputBuffer failed 0x%x", status);
		return status;
	}

	// Check EngineNum
	if (pDmaStatStruct != NULL)
	{
		// Get the DMA Performance numbers for Packet Mode.
		// Get the numbers that have been captured by the watchdog timer. 
		pDmaStatStruct->HardwareTime = pDmaExt->HardwareTimeInLastSecond;
		pDmaStatStruct->DriverTime = pDmaExt->DMAInactiveTime;
		pDmaStatStruct->CompletedByteCount = pDmaExt->BytesInLastSecond;
		if (DmaStatStructSize == sizeof(DMA_STAT_STRUCT))
		{
			pDmaStatStruct->IntsPerSecond = pDmaExt->IntsInLastSecond;
			pDmaStatStruct->DPCsPerSecond = pDmaExt->ReadDPCsInLastSecond;
			WdfSpinLockAcquire(pDmaExt->HeadSpinLock);
			pDmaExt->IntsInLastSecond = 0;
			pDmaExt->ReadDPCsInLastSecond = 0;
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
		DEBUGP(DEBUG_ERROR, "Invalid parameter");
		status = STATUS_NO_SUCH_DEVICE;
	}
	return status;
}