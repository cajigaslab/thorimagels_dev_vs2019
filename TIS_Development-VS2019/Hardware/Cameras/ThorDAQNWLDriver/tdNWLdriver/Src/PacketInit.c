// -------------------------------------------------------------------------
// 
// PRODUCT:			DMA Driver
// MODULE NAME:		PacketInit.c
// 
// MODULE DESCRIPTION: 
// 
// Contains the initializations routines required for Packet Mode
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
#include "PacketInit.tmh"
#endif // TRACE_ENABLED

// Local functions

DRIVER_LIST_CONTROL	PacketRxGetReadSgListComplete;

VOID PacketRxGetReadSgListComplete(PDEVICE_OBJECT, PIRP, PSCATTER_GATHER_LIST, PVOID);


#ifdef ALLOC_PRAGMA
#endif  /* ALLOC_PRAGMA */

// Local defines
#define	DEFAULT_TIMEOUT_LOOP	50

/*! DMADriverIntiializeDMADescriptors 
 *
 *  \brief Provides the basic DMA Descriptors intialization
 *   sets up physical linkages, etc.
 *  \param pDmaExt - Pointer to the DMA Engine Context
 *	\param NumberDescriptors - Number of descriptors to initialize and link
 *  \return STATUS_SUCCESS if it works, an error if it fails.
 */
NTSTATUS
DMADriverIntiializeDMADescriptors(
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt,
	IN UINT32 NumberDescriptors,
	IN UINT32 DescFlags
	)
{
	PDRIVER_DESC_STRUCT		pDrvDesc;
	PDRIVER_DESC_STRUCT		pNextDrvDesc;
	PDMA_DESCRIPTOR_STRUCT	pHWDesc;
	PHYSICAL_ADDRESS		pHWDescPhys;
	UINT32					descNum;

	// Setup the Next and tail pointers to start at the base.
	pDmaExt->pNextDesc = pDmaExt->pDrvDescBase;
	pDmaExt->pTailDesc = pDmaExt->pDrvDescBase;

	pDrvDesc = pDmaExt->pDrvDescBase;
	pHWDesc = pDmaExt->pHWDescriptorBase;
	pHWDescPhys = pDmaExt->pHWDescriptorBasePhysical;

	if (NumberDescriptors < MINIMUM_NUMBER_DESCRIPTORS)
		return STATUS_BAD_PARAMETER;
	if (NumberDescriptors > pDmaExt->NumberOfDescriptors)
		return STATUS_BAD_PARAMETER;

	// Set the DMA Engine back to a restarted state
	pDmaExt->NumberOfUsedDescriptors = 0;

	// setup each of the descriptors
	for (descNum = 0; descNum < NumberDescriptors; descNum++)
	{
		pNextDrvDesc = pDrvDesc;
		
		// Initialize the Hardware DMA Descriptor
		pHWDesc->C2S.StatusFlags_BytesCompleted = 0;
        pHWDesc->C2S.UserStatus			= 0;
        pHWDesc->C2S.CardAddress		= 0;
        pHWDesc->C2S.ControlFlags_ByteCount = 0;
        pHWDesc->C2S.SystemAddressPhys	= 0;

		// Initialize the Drivers reference to the Hardware DMA Descriptor
		pDrvDesc->pHWDesc				= pHWDesc;
		pDrvDesc->pHWDescPhys			= pHWDescPhys;
		pDrvDesc->DescriptorNumber		= descNum;
		pDrvDesc->DescFlags				= DescFlags;
		pDrvDesc->pScatterGatherList	= NULL;
		pDrvDesc->SystemAddressVirt		= NULL;
		pDrvDesc->DmaTransaction		= NULL;

		// If this is the last descriptor...
		if (descNum == (NumberDescriptors-1))
		{
			// Link back to the top of the Descriptor pool
			pDrvDesc->pNextDesc = pDmaExt->pDrvDescBase;
			pHWDesc->S2C.NextDescriptorPhys = pDmaExt->pHWDescriptorBasePhysical.LowPart;
		}
		else
		{
			// Link to the Next Descriptor
			pHWDescPhys.QuadPart += sizeof(DMA_DESCRIPTOR_STRUCT);
			pHWDesc->S2C.NextDescriptorPhys = pHWDescPhys.LowPart;
			pDrvDesc->pNextDesc = ++pNextDrvDesc;
			pDrvDesc = pNextDrvDesc;
			pHWDesc++;
		}
	}
	
	// setup the descriptor pointers
	pDmaExt->pDmaEng->NextDescriptorPtr =	pDmaExt->pHWDescriptorBasePhysical.LowPart;
	pDmaExt->pDmaEng->SoftwareDescriptorPtr = pDmaExt->pHWDescriptorBasePhysical.LowPart;
	pDmaExt->pDmaEng->CompletedDescriptorPtr = 0;

	return STATUS_SUCCESS;
}			

/*! InitializeRxDescriptors 
 *
 * 	\brief - This routine initializes the Packet Recieve DMA Descriptors
 *	\param pDevExt - Pointer to this drivers context (data store)
 *  \param pDmaExt - Pointer to the DMA Engine Context
 *  \param pvRetVirtAddr - Virtual Addresses to return in the PacketReceives call
 *  \param AllocSize - Size to allocate the buffer in bytes
 *	Must be larger than 1 page and should be in multiples of 4K
 *  \return STATUS_SUCCESS if it works, an error if it fails.
 * 	\note This routine is called at IRQL < DISPATCH_LEVEL.
 *   We MUST be running in the process where we want this memory mapped.
 */
NTSTATUS 
InitializeRxDescriptors(
	IN PDEVICE_EXTENSION 	pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt,
	IN PVOID				pvRetVirtAddr,
	IN UINT32				AllocSize)
{
	PDRIVER_DESC_STRUCT		pDrvDesc;
	PDMA_DESCRIPTOR_STRUCT	pHWDesc;
	PUINT8					UserAddrVirt;
	PUINT8 					pRetVirtAddr;
	UINT32 					MaxDescCount;
	UINT32					descNum;
	NTSTATUS				status = STATUS_INVALID_PARAMETER;

	// Shutdown the DMA Engine
	pDmaExt->pDmaEng->ControlStatus = 0;
	// Set the DMA Engine back to a restarted state
	pDmaExt->NumberOfUsedDescriptors = 0;

	// Setup the Next and tail pointers to start at the base.
	pDrvDesc = pDmaExt->pDrvDescBase;
	pDmaExt->pNextDesc = pDrvDesc;

	pDmaExt->bDescriptorAllocSuccess = TRUE;

	// Calc the total number of descriptors
	MaxDescCount = (UINT32)AllocSize / PAGE_SIZE;

	if (pDmaExt->pDmaEng->ControlStatus & PACKET_DMA_CTRL_DMA_RUNNING)
	{
		DEBUGP(DEBUG_WARN, "DMA Engine is still running.");
	}

	if (pDmaExt->bFreeRun)
	{
		pDmaExt->pDmaEng->InterruptControl = 0;	
	}
	else
	{
		// Default to EOP Interrupt mode
		pDmaExt->pDmaEng->InterruptControl = pDevExt->InterruptMode;
	}

	// If we have enough descriptors, if so initialize the structures
	if (pDmaExt->NumberOfDescriptors >= MaxDescCount)
	{
		UserAddrVirt = pDmaExt->UserVa;
		pRetVirtAddr = pvRetVirtAddr; 

		// setup each of the descriptors
		for (descNum = 0; descNum < MaxDescCount; descNum++)
		{
			pDmaExt->pNextDesc->SystemAddressVirt = (PVOID)pRetVirtAddr;

			status = pDmaExt->pReadDmaAdapter->DmaOperations->GetScatterGatherList(
				pDmaExt->pReadDmaAdapter,
                pDevExt->FunctionalDeviceObject,
				pDmaExt->PMdl,
				UserAddrVirt, 
				PAGE_SIZE,
                PacketRxGetReadSgListComplete,
                pDmaExt,     // Context
                FALSE);      // This is not a write

			if (status != STATUS_SUCCESS)
			{
				// Get Scatter/Gather failed!
				DEBUGP(DEBUG_ERROR, "<-- Get Scatter/Gather List Failed with error (%d) in DmaEngine[%d]",
							status, pDmaExt->DmaEngine);
				return status;
			}
			if (!pDmaExt->bDescriptorAllocSuccess)
			{
				// GetScatterGather failed!
				DEBUGP(DEBUG_ERROR, "<-- Failed setting up ScatterGather entries in DmaEngine[%d]",
							pDmaExt->DmaEngine);
				status =  STATUS_DRIVER_INTERNAL_ERROR;
				return status;
			}
			UserAddrVirt += PAGE_SIZE;
			pRetVirtAddr += PAGE_SIZE;
		}

		// Make sure we have at least one completed descriptor
		if (pDmaExt->pNextDesc != pDmaExt->pDrvDescBase)
		{
			// At this point the 'Next' Pointer are at the last Desc + 1
			// We need to back it up to the last Desc and wrap the links back to the start
			pDrvDesc = (PDRIVER_DESC_STRUCT)((PUINT8)pDmaExt->pNextDesc - sizeof(DRIVER_DESC_STRUCT));
			pHWDesc = pDrvDesc->pHWDesc;

			// Last Descriptor, link back to the top of the Descriptor pool (both virtual and Physical addresses).
			pDrvDesc->pNextDesc = (PDRIVER_DESC_STRUCT)pDmaExt->pDrvDescBase;
			pHWDesc->C2S.NextDescriptorPhys = pDmaExt->pHWDescriptorBasePhysical.LowPart;

			if (pDmaExt->bFreeRun)
			{
				pDmaExt->pDmaEng->SoftwareDescriptorPtr = 0;
			}
			else
			{
				pDmaExt->pDmaEng->SoftwareDescriptorPtr = pDrvDesc->pHWDescPhys.LowPart;
			}
			pDmaExt->pTailDesc = pDrvDesc;

			// Reset the 'Next' pointers to start at the base.
			pDmaExt->pNextDesc = pDmaExt->pDrvDescBase;
		}

		// setup the DMA Engine descriptor pointers
		pDmaExt->pDmaEng->NextDescriptorPtr = pDmaExt->pHWDescriptorBasePhysical.LowPart;
		pDmaExt->pDmaEng->CompletedDescriptorPtr = 0;

		// Now enable the DMA Engine
		pDmaExt->pDmaEng->ControlStatus = (COMMON_DMA_CTRL_IRQ_ENABLE | PACKET_DMA_CTRL_DMA_ENABLE);
		status = STATUS_SUCCESS;
	}
	else
	{
		// Not enough descriptors available
		DEBUGP(DEBUG_ERROR, "<-- Not enough descriptors (Req=%d) are available (%d) in DmaEngine[%d]",
					MaxDescCount, pDmaExt->NumberOfDescriptors, pDmaExt->DmaEngine);
		status = STATUS_INSUFFICIENT_RESOURCES;
	}
	return status;
}

/* PacketRxGetReadSgListComplete 
 *
 * \brief - This routine converts a scatter gather entries into DMA Descriptors
 * \param pDeviceObject - Caller-supplied pointer to a DEVICE_OBJECT structure. 
 *	This is the device object for the target device, previously created 
 *	by the driver's AddDevice routine.
 * \param pIrp - Not used - Caller-supplied pointer to an IRP structure that 
 *	describes the I/O operation, 
 *	\note: Do not use unless CurrentIrp is set in the DeviceObject.
 * \param pScatterGatherList - Caller-supplied pointer to a 
 *	SCATTER_GATHER_LIST structure describing scatter/gather regions.
 * \param Context - Caller Supplied Context field
 * \return nothing
 */
VOID 
PacketRxGetReadSgListComplete(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp,
    IN PSCATTER_GATHER_LIST pScatterGatherList,
    IN PVOID                Context
    )
{
	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt = (PDMA_ENGINE_DEVICE_EXTENSION)Context;
	PDRIVER_DESC_STRUCT		pDrvDesc;
	PDMA_DESCRIPTOR_STRUCT	pHWDesc;

	UNREFERENCED_PARAMETER(pDeviceObject);
	UNREFERENCED_PARAMETER(pIrp);

	if (pScatterGatherList != NULL)
    {
		// Since we allocating based on PAGE_SIZE we "should never get" more than one element
		if (pScatterGatherList->NumberOfElements == 1)
		{
			pDrvDesc = pDmaExt->pNextDesc;
			pHWDesc = pDrvDesc->pHWDesc;

			// setup the descriptor
			pHWDesc->C2S.StatusFlags_BytesCompleted = 0;
			pHWDesc->C2S.UserStatus = 0;
			pHWDesc->C2S.CardAddress = 0;

			if (pDmaExt->bFreeRun)
			{
				pHWDesc->C2S.ControlFlags_ByteCount = pScatterGatherList->Elements[0].Length;
			}
			else
			{
				pHWDesc->C2S.ControlFlags_ByteCount = ( pScatterGatherList->Elements[0].Length |
														 PACKET_DESC_S2C_CTRL_IRQ_ON_COMPLETE |
														 PACKET_DESC_S2C_CTRL_IRQ_ON_ERROR);
			}
			pHWDesc->C2S.SystemAddressPhys = pScatterGatherList->Elements[0].Address.QuadPart;
            pDrvDesc->DmaTransaction = NULL;
	
			// Make sure we get a valid address
			if (pHWDesc->C2S.SystemAddressPhys != 0)
			{
				// On the Rx Side we use the UsedDescriptors as a total allocated count
				pDrvDesc->DescriptorNumber = pDmaExt->NumberOfUsedDescriptors++;
				pDrvDesc->DescFlags = DESC_FLAGS_HW_OWNED;
		
				// Cache the pointer to the Scatter Gather list only if this is the last descriptor
				pDrvDesc->pScatterGatherList = pScatterGatherList;
		
				// Link to the next driver descriptor in the chain (driver usable address)
				pDrvDesc->pNextDesc = (PDRIVER_DESC_STRUCT)((UINT8 *)pDrvDesc + sizeof(DRIVER_DESC_STRUCT));
				pDrvDesc = pDrvDesc->pNextDesc;
				pDmaExt->pNextDesc = pDrvDesc;
			}
			else
			{
				DEBUGP(DEBUG_ERROR, "PacketRxGetReadSgListComplete ERROR, S/G return a null system pointer, desc # %d", 
					pDrvDesc->DescriptorNumber);
				pDmaExt->bDescriptorAllocSuccess = FALSE;
			}
		}
		else
		{
			DEBUGP(DEBUG_ERROR, "PacketRxGetReadSgListComplete ERROR, S/G entry count != 1 (%d).", 
				pScatterGatherList->NumberOfElements);
			pDmaExt->bDescriptorAllocSuccess = FALSE;
		}
	}
	else
	{
		DEBUGP(DEBUG_ERROR, "PacketRxGetReadSgListComplete ERROR, S/G pointer is null.");
		pDmaExt->bDescriptorAllocSuccess = FALSE;
	}
	return;
}

/*! InitializeAddressablePacketDescriptors 
 * \brief - This routine initializes the
 *  C2S Addressable Packet mode DMA Descriptors
 *  This routine is called at IRQL < DISPATCH_LEVEL.
 *  We MUST be running in the process where we want this memory mapped.
 * \param pAdapter - Pointer to this drivers context (data store)
 * \param pDmaExt - Pointer to the DMA Engine Context
 * \param NumberDescriptors - Number of DMA descriptors to allocate
 * \return STATUS_SUCCESS if it works, an error if it fails.
 */
NTSTATUS 
InitializeAddressablePacketDescriptors(
	IN PDEVICE_EXTENSION 	pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt
    )
{
	NTSTATUS					status = STATUS_INVALID_PARAMETER;

	// Shutdown the DMA Engine
	pDmaExt->pDmaEng->ControlStatus = 0;

	if (pDmaExt->pDmaEng->ControlStatus & PACKET_DMA_CTRL_DMA_RUNNING)
	{
		DEBUGP(DEBUG_WARN, "InitAddressablePacketDesc: DMA Engine is still running.");
	}

	// Set the DMA Engine back to a restarted state
	pDmaExt->NumberOfUsedDescriptors = 0;

	// Default to EOP Interrupt mode
	pDmaExt->pDmaEng->InterruptControl = pDevExt->InterruptMode;

	WdfSpinLockAcquire(pDmaExt->TailSpinLock);

	// Make sure the buffer was allocated
	if (pDmaExt->NumberOfDescriptors)
	{
		DMADriverIntiializeDMADescriptors(pDmaExt, pDmaExt->NumberOfDescriptors, DESC_FLAGS_ADDRESSABLE_MODE);

		// Now enable the DMA Engine
		pDmaExt->pDmaEng->ControlStatus = (COMMON_DMA_CTRL_IRQ_ENABLE | PACKET_DMA_CTRL_DMA_ENABLE);
		status = STATUS_SUCCESS;
	}
	else
	{
		// Not enough descriptors available
		DEBUGP(DEBUG_ERROR, "<-- Not enough descriptors are available (%d) in DmaEngine[%d]",
					pDmaExt->NumberOfDescriptors, pDmaExt->DmaEngine);
		status = STATUS_INSUFFICIENT_RESOURCES;
	}
	WdfSpinLockRelease(pDmaExt->TailSpinLock);
	return status;
}

/*! FreeRxDecriptors
 *
 * 	\brief his routine frees the allocated Receive
 * 	 decriptors and any resources associated to them
 * 	 This routine is called at IRQL < DISPATCH_LEVEL.
 * 	\param pDevExt - Pointer to this drivers context (data store)
 *  \param pDmaExt - Pointer to the DMA Engine Context
 *  \return None
*/
VOID 
FreeRxDescriptors(
	IN PDEVICE_EXTENSION 	pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt)
{
	PDRIVER_DESC_STRUCT			pDrvDesc;
	PDMA_DESCRIPTOR_STRUCT		pHWDesc;
	PDMA_XFER					pDmaXfer = NULL;
	WDFREQUEST					Request;
	INT32						i;
	NTSTATUS					status;

	pDrvDesc = pDmaExt->pDrvDescBase;
	pHWDesc = pDrvDesc->pHWDesc;

	ShutdownDMAEngine(pDevExt, pDmaExt);

	WdfSpinLockAcquire(pDmaExt->TailSpinLock);
	for (i = 0; i < pDmaExt->NumberOfUsedDescriptors; i++)
	{
		if ((pDrvDesc->pScatterGatherList != 0) )
		{
				// Release the ScatterGatherList
				pDmaExt->pReadDmaAdapter->DmaOperations->PutScatterGatherList(
				pDmaExt->pReadDmaAdapter,
				pDrvDesc->pScatterGatherList,
				FALSE);
			pDrvDesc->pScatterGatherList = NULL;
		}
		if (pDrvDesc->DmaTransaction != NULL)
		{
			if (pHWDesc->C2S.ControlFlags_ByteCount & PACKET_DESC_C2S_STAT_END_OF_PACKET)
			{
				pDmaXfer = DMAXferContext(pDrvDesc->DmaTransaction);
				if (pDmaXfer != NULL)
				{
					WdfDmaTransactionDmaCompletedFinal(	pDrvDesc->DmaTransaction,
																0, &status);
					// Retrieve the originating request from the Transaction data extension
					Request = FindRequestByRequest(pDmaExt, pDmaXfer->Request);
					if (Request != NULL) 
					{
						// complete the transaction
						WdfRequestCompleteWithInformation(Request, STATUS_CANCELLED, 0);
					}
					if (pDmaXfer->pMdl != NULL)
					{
						DEBUGP(DEBUG_ERROR, "FreeRxDescriptors: Found MDL");
						MmUnlockPages(pDmaXfer->pMdl);
						IoFreeMdl(pDmaXfer->pMdl);
						pDmaXfer->pMdl = NULL;
					}
					pDmaXfer->Request = NULL;
				}
				// Release the Transaction record
				WdfDmaTransactionRelease (pDrvDesc->DmaTransaction);
				WdfObjectDelete(pDrvDesc->DmaTransaction);
				pDmaXfer = NULL;
			}
			pDrvDesc->DmaTransaction = NULL;
		}
		pDrvDesc++;
	}

	DMADriverIntiializeDMADescriptors(pDmaExt, pDmaExt->NumberOfDescriptors, 0);

	WdfSpinLockRelease(pDmaExt->TailSpinLock);
}

/*! InitializeTxDescriptors
 *
 * 	\brief This routine allocates and initializes the Packet Send (Transmit) DMA Descriptors
 *   This routine is called at IRQL < DISPATCH_LEVEL.
 *	\param pDevExt - Pointer to this drivers context (data store)
 *  \param pDmaExt - Pointer to the DMA Engine Context
 *  \return STATUS_SUCCESS if it works, an error if it fails.
 */
NTSTATUS 
InitializeTxDescriptors(
	IN PDEVICE_EXTENSION 	pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt
	)
{
	PDRIVER_DESC_STRUCT		pDrvDesc;
	PDMA_DESCRIPTOR_STRUCT	pHWDesc;
	PHYSICAL_ADDRESS		pHWDescPhys;
	UINT32					descNum;
	NTSTATUS				status = STATUS_INSUFFICIENT_RESOURCES;

	UNREFERENCED_PARAMETER(pDevExt);

	// Shutdown the DMA Engine
	pDmaExt->pDmaEng->ControlStatus = 0;
	// Set the DMA Engine back to a restarted state
	pDmaExt->NumberOfUsedDescriptors = 0;

	if (pDmaExt->pDmaEng->ControlStatus & PACKET_DMA_CTRL_DMA_RUNNING)
	{
		DEBUGP(DEBUG_WARN, "InitTxDesc: DMA Engine is still running.");
	}

	WdfSpinLockAcquire(pDmaExt->TailSpinLock);

	// Make sure the buffer was allocated
	if (pDmaExt->NumberOfDescriptors)
	{
		pDmaExt->pNextDesc = pDmaExt->pDrvDescBase;
		pDmaExt->pTailDesc = pDmaExt->pDrvDescBase;
		pHWDescPhys = pDmaExt->pHWDescriptorBasePhysical;

		pDrvDesc = pDmaExt->pNextDesc;
		pHWDesc = pDrvDesc->pHWDesc;

		// setup each of the descriptors
		for (descNum = 0; descNum < pDmaExt->NumberOfDescriptors; descNum++)
		{
			// setup the descriptor
			pHWDesc->S2C.StatusFlags_BytesCompleted = 0;
			pHWDesc->S2C.UserControl = 0;
			pHWDesc->S2C.CardAddress = 0;
			pHWDesc->S2C.ControlFlags_ByteCount = ( 0	|
				PACKET_DESC_S2C_CTRL_IRQ_ON_COMPLETE	|
				PACKET_DESC_S2C_CTRL_IRQ_ON_ERROR		|
				PACKET_DESC_S2C_CTRL_END_OF_PACKET		|
				PACKET_DESC_S2C_CTRL_START_OF_PACKET);
			pHWDesc->S2C.SystemAddressPhys = (UINT64)-1;

			pDrvDesc->DmaTransaction = NULL;
			pDrvDesc->DescriptorNumber = descNum;
			pDrvDesc->pHWDescPhys = pHWDescPhys;
	
			// update pointers
			if (descNum == (pDmaExt->NumberOfDescriptors-1))
			{
				// This is the last descriptor, link back to the top of the Descriptor pool
				pDrvDesc->pNextDesc = pDmaExt->pDrvDescBase;
				pHWDesc->S2C.NextDescriptorPhys = pDmaExt->pHWDescriptorBasePhysical.LowPart;
			}
			else
			{
				// Link to the Next Descriptor
				pHWDescPhys.QuadPart += sizeof(DMA_DESCRIPTOR_STRUCT);
				pHWDesc->S2C.NextDescriptorPhys = pHWDescPhys.LowPart;
				pDrvDesc->pNextDesc = (PVOID)((PUINT8)pDrvDesc + sizeof(DRIVER_DESC_STRUCT));
				pDrvDesc = pDrvDesc->pNextDesc;
				pHWDesc++;
				if (pDrvDesc->pHWDesc != pHWDesc)
				{
					DEBUGP(DEBUG_ERROR, "Broken Link detected in DrvDesc");
				}
			}
		}

		// setup the descriptor pointers
		pDmaExt->pDmaEng->NextDescriptorPtr =	pDmaExt->pHWDescriptorBasePhysical.LowPart;
		pDmaExt->pDmaEng->SoftwareDescriptorPtr = pDmaExt->pHWDescriptorBasePhysical.LowPart;
		pDmaExt->pDmaEng->CompletedDescriptorPtr = 0;

		// Now enable the DMA Engine
		pDmaExt->pDmaEng->ControlStatus = (COMMON_DMA_CTRL_IRQ_ENABLE | PACKET_DMA_CTRL_DMA_ENABLE);
		status = STATUS_SUCCESS;
	}
	else
	{
		// Not enough descriptors available
		DEBUGP(DEBUG_ERROR, "<-- Not enough descriptors are available (%d) in DmaEngine[%d]",
					pDmaExt->NumberOfDescriptors, pDmaExt->DmaEngine);
	}

	WdfSpinLockRelease(pDmaExt->TailSpinLock);
	return status;
}

/*! ShutdownDMAEngine
 *
 * 	\brief This routine tries to do an orderly shutdown
 * 	 of the DMA Engine specified
 *   This routine is called at IRQL < DISPATCH_LEVEL.
 * 	\param pDevExt - Pointer to this drivers context (data store)
 *  \param pDmaExt - Pointer to the DMA Engine Context
 *  \return None
 */
VOID 
ShutdownDMAEngine(
	IN PDEVICE_EXTENSION 	pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt)
{
	UINT32	TimeoutLoop;
//	DEBUGP(DEBUG_INFO, "ShutdownDMAEngine\n");
	
	// Shutdown the DMA Engine, DMA_ENABLE = 0
	pDmaExt->pDmaEng->ControlStatus = 0;

	// Wait for the DMA Engine to go idle
	TimeoutLoop = DEFAULT_TIMEOUT_LOOP;
	while (pDmaExt->pDmaEng->ControlStatus & PACKET_DMA_CTRL_DMA_RUNNING)
	{
		if (--TimeoutLoop == 0)
		{
			// If we timeout, reset the pointers and try again.
			pDmaExt->pDmaEng->NextDescriptorPtr =	0;
			pDmaExt->pDmaEng->SoftwareDescriptorPtr =	0;
			pDmaExt->pDmaEng->CompletedDescriptorPtr = 0;
			pDmaExt->pDmaEng->ControlStatus = 0;
			// DMA Engine is not responding to Idle request
			DEBUGP(DEBUG_INFO, "Timed out waiting for DMA to go idle.\n");
			break;
		}
		mdelay(1);
	}

	// We assume the DMA is idle at this point, now issue the reset request
	pDmaExt->pDmaEng->ControlStatus = PACKET_DMA_CTRL_DMA_RESET_REQUEST;
	TimeoutLoop = DEFAULT_TIMEOUT_LOOP;
	while (pDmaExt->pDmaEng->ControlStatus & (PACKET_DMA_CTRL_DMA_RUNNING | 
												 PACKET_DMA_CTRL_DMA_RESET_REQUEST))
	{
		if (--TimeoutLoop == 0)
		{
			// DMA Engine is not responding to Reset request
			DEBUGP(DEBUG_INFO, "Timed out waiting for DMA Reset Request acknowledge.");
			break;
		}
		mdelay(1);
		pDmaExt->pDmaEng->ControlStatus = PACKET_DMA_CTRL_DMA_RESET_REQUEST;
	}
	// Clear any persistant bits just to make sure there is no residue from the reset request
	pDmaExt->pDmaEng->ControlStatus = (COMMON_DMA_CTRL_IRQ_ACTIVE				|
										  PACKET_DMA_CTRL_DESC_COMPLETE			|
										  PACKET_DMA_CTRL_DESC_FETCH_ERROR		|
										  PACKET_DMA_CTRL_DESC_ALIGN_ERROR		|
										  PACKET_DMA_CTRL_DESC_SW_ABORT_ERROR	|
										  PACKET_DMA_CTRL_DESC_CHAIN_END		|
										  PACKET_DMA_CTRL_DMA_WAITING_PERSIST);

	// reinitialize performance counters
	pDmaExt->BytesInLastSecond = 0;
	pDmaExt->BytesInCurrentSecond = 0;
	pDmaExt->HardwareTimeInLastSecond = 0;
	pDmaExt->HardwareTimeInCurrentSecond = 0;
	pDmaExt->DMAInactiveTime = 0;

	pDmaExt->pDmaEng->ControlStatus = PACKET_DMA_CTRL_DMA_RESET;
	TimeoutLoop = DEFAULT_TIMEOUT_LOOP;
	while (pDmaExt->pDmaEng->ControlStatus & PACKET_DMA_CTRL_DMA_RESET)
	{
		if (--TimeoutLoop == 0)
		{
			// DMA Engine is not responding to Reset
			DEBUGP(DEBUG_INFO, "Timed out waiting for DMA Reset acknowledge.");
			break;
		}
		mdelay(1);
	}
	// If we just reset the S2C Engine we must set it back up
	if (pDmaExt->DmaType == DMA_TYPE_PACKET_SEND)
	{
		DEBUGP(DEBUG_INFO, "ShutdownDMAEngine() DMA_TYPE_PACKET_SEND, calling InitializeTxDescriptors: pDmaExt %p", pDmaExt);
		InitializeTxDescriptors(pDevExt, pDmaExt);
	}

}