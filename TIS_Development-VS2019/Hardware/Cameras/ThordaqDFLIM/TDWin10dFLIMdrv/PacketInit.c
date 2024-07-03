#include "Driver.h"
#include "Trace.h"

// Local functions

DRIVER_LIST_CONTROL	PacketRxGetReadSgListComplete;

VOID PacketRxGetReadSgListComplete(PDEVICE_OBJECT, PIRP, PSCATTER_GATHER_LIST, PVOID);
void ShutdownDMAEngine(PDEVICE_EXTENSION, PDMA_ENGINE_DEVICE_EXTENSION);


#ifdef ALLOC_PRAGMA
#endif  /* ALLOC_PRAGMA */

// Local defines
#define	DEFAULT_TIMEOUT_LOOP	500000


void DecodeDbgStatus(UINT64 status)
{
	DEBUGP(DEBUG_INFO, "64-bit Status BAR3(offset_0):  0x%llx", status);
	DEBUGP(DEBUG_INFO, "Ver               0x%x", (int)status & 0xFFFFFFFF); // bits 31:0 version
	DEBUGP(DEBUG_INFO, "PrgmErr     (32)  0x%x", (int)(status >> 32 & 0x1));
	DEBUGP(DEBUG_INFO, "0           (33)  0x%x", (int)(status >> 33 & 0x1));
	DEBUGP(DEBUG_INFO, "FIFO empty  (34)  0x%x", (int)(status >> 34 & 0x1));
	DEBUGP(DEBUG_INFO, "FIFO full   (35)  0x%x", (int)(status >> 35 & 0x1));
	DEBUGP(DEBUG_INFO, "Prog_Rdy    (36)  0x%x", (int)(status >> 36 & 0x1));
	DEBUGP(DEBUG_INFO, "AcqAxis_Rdy (37)  0x%x", (int)(status >> 37 & 0x1));

}
/*!***************************************************************************
*
* 	\brief InitializeRxDescriptors - This routine initializes the Packet
*	Recieve DMA Descriptors
* 
* 	\param pDevExt - Pointer to this drivers context (data store)
*   \param pDmaExt - Pointer to the DMA Engine Context
* 	\param AllocSize - Size to allocate the buffer in bytes
* 			Must be larger than 1 page and should be in multiples of 4K
* 	\param MaxPacketSize - Maximum size of a single packet. Must be less than
*			or equal to AllocSize. Should be a diviable fraction of AllocSize
* 
*   \return STATUS_SUCCESS if it works, an error if it fails.
* 
* 	This routine is called at IRQL < DISPATCH_LEVEL.
* 
*   We MUST be running in the process where we want this memory mapped.
*****************************************************************************/
NTSTATUS InitializeRxDescriptors(
	IN PDEVICE_EXTENSION 	pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt,
	IN ULONG32 				AllocSize, 
	IN ULONG32				MaxPacketSize)
{
	PDMA_DESCRIPTOR_STRUCT		pDesc;
	PHYSICAL_ADDRESS			pDescPhys;
	PUCHAR						SystemAddressVirt;
	ULONG32						MaxDescCount;
	ULONG32						descNum;
	NTSTATUS					status = STATUS_INVALID_PARAMETER;

	UNREFERENCED_PARAMETER(MaxPacketSize);

	// Shutdown the DMA Engine
	pDmaExt->pDmaEng->ControlStatus = 0;
	// Set the DMA Engine back to a restarted state
	pDmaExt->NumberOfUsedDescriptors = 0;
	// Setup the 'Next' pointers to start at the base.
	pDmaExt->pNextDescVirt = pDmaExt->pDescriptorBase;
	pDmaExt->pNextDescPhys = pDmaExt->pDescriptorBasePhysical;

	pDmaExt->bDescriptorAllocSuccess = TRUE;

	//Calc the total number of packets
	MaxDescCount = (ULONG32)AllocSize / PAGE_SIZE;

	if (pDmaExt->pDmaEng->ControlStatus & PACKET_DMA_CTRL_DMA_RUNNING)
	{

	}

	if (pDmaExt->bFreeRun)
	{
		pDmaExt->pDmaEng->Packet.InterruptControl = 0;	
	}
	else
	{
		// Default to EOP Interrupt mode
		pDmaExt->pDmaEng->Packet.InterruptControl = pDevExt->InterruptMode;
	}

	// If we have enough descriptors, if so initialize the structures
	if (pDmaExt->NumberOfDescriptors >= MaxDescCount)
	{
		SystemAddressVirt = pDmaExt->UserVa;

		// setup each of the descriptors
		for (descNum = 0; descNum < MaxDescCount; descNum++)
		{
			pDmaExt->pNextDescVirt->Packet.C2S.SystemAddressVirt = (PVOID)SystemAddressVirt;

			status = pDmaExt->pReadDmaAdapter->DmaOperations->GetScatterGatherList(
				pDmaExt->pReadDmaAdapter,
				pDevExt->FunctionalDeviceObject,
				pDmaExt->PMdl,
				SystemAddressVirt, 
				PAGE_SIZE,
				PacketRxGetReadSgListComplete,
				pDmaExt,     // Context
				FALSE);         // This is not a write

			if (status != STATUS_SUCCESS)
			{
				// Get Scatter/Gather failed!
				return status;
			}
			if (!pDmaExt->bDescriptorAllocSuccess)
			{
				// GetScatterGather failed!
				status =  STATUS_DRIVER_INTERNAL_ERROR;
				return status;
			}
			SystemAddressVirt += PAGE_SIZE;
		}

		// Make sure we have at least one completed descriptor
		if (pDmaExt->pNextDescVirt != pDmaExt->pDescriptorBase)
		{
			// At this point the 'Next' Pointer are at the last Desc + 1
			// We need to back it up to the last Desc and wrap the links back to the start
			pDesc = (PDMA_DESCRIPTOR_STRUCT)((UCHAR *)pDmaExt->pNextDescVirt -sizeof(DMA_DESCRIPTOR_STRUCT));
			pDescPhys.QuadPart = pDmaExt->pNextDescPhys.QuadPart - sizeof(DMA_DESCRIPTOR_STRUCT);

			// Last Descriptor, link back to the top of the Descriptor pool
			pDesc->Packet.pNextDescriptorVirt = (PVOID *)pDmaExt->pDescriptorBase;
			pDesc->Packet.C2S.NextDescriptorPhys = pDmaExt->pDescriptorBasePhysical.LowPart;
			if (pDmaExt->bFreeRun)
			{
				pDmaExt->pDmaEng->Packet.SoftwareDescriptorPtr = 0;
			}
			else
			{
				pDmaExt->pDmaEng->Packet.SoftwareDescriptorPtr = pDescPhys.LowPart;
			}
			pDmaExt->pTailDescVirt = pDesc;

			// Reset the 'Next' pointers to start at the base.
			pDmaExt->pNextDescVirt = pDmaExt->pDescriptorBase;
			pDmaExt->pNextDescPhys = pDmaExt->pDescriptorBasePhysical;
		}

		// setup the DMA Engine descriptor pointers
		pDmaExt->pDmaEng->Packet.NextDescriptorPtr = pDmaExt->pDescriptorBasePhysical.LowPart;
		pDmaExt->pDmaEng->Packet.CompletedDescriptorPtr = 0;

		// Now enable the DMA Engine
		pDmaExt->pDmaEng->ControlStatus = (COMMON_DMA_CTRL_IRQ_ENABLE | PACKET_DMA_CTRL_DMA_ENABLE);
		status = STATUS_SUCCESS;
	}
	else
	{
		// Not enough descriptors available
		status = STATUS_INSUFFICIENT_RESOURCES;
	}
	return status;
}

/*!***************************************************************************
*
* 	\brief PacketRxGetReadSgListComplete - This routine converts a scatter
* 	gather entries into DMA Descriptors
* 
* 	\param pDeviceObject - Caller-supplied pointer to a DEVICE_OBJECT structure. 
*			This is the device object for the target device, previously created 
*			by the driver's AddDevice routine.
*   \param pIrp - Not used - Caller-supplied pointer to an IRP structure that 
*			describes the I/O operation, 
*			NOTE: Do not use unless CurrentIrp is set in the DeviceObject.
*	\param pScatterGatherList - Caller-supplied pointer to a 
*			SCATTER_GATHER_LIST structure describing scatter/gather regions.
* 	\param Context - Caller Supplied Context field
* 
*   \return nothing
* 
*****************************************************************************/
VOID PacketRxGetReadSgListComplete(
	IN PDEVICE_OBJECT       pDeviceObject,
	IN PIRP                 pIrp,
	IN PSCATTER_GATHER_LIST pScatterGatherList,
	IN PVOID                Context
	)
{
	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt = (PDMA_ENGINE_DEVICE_EXTENSION)Context;
	PDMA_DESCRIPTOR_STRUCT	pDesc;
	PHYSICAL_ADDRESS	   	pDescPhys;

	UNREFERENCED_PARAMETER(pDeviceObject);
	UNREFERENCED_PARAMETER(pIrp);

	if (pScatterGatherList != NULL)
	{
		// Since we allocating based on PAGE_SIZE we "should never get" more than one element
		if (pScatterGatherList->NumberOfElements == 1)
		{
			pDesc = pDmaExt->pNextDescVirt;
			pDescPhys = pDmaExt->pNextDescPhys;

			// setup the descriptor
			pDesc->Packet.C2S.StatusFlags_BytesCompleted = 0;
			pDesc->Packet.C2S.UserStatus = 0;
			pDesc->Packet.C2S.CardAddress = 0;

			if (pDmaExt->bFreeRun)
			{
				pDesc->Packet.C2S.ControlFlags_ByteCount = pScatterGatherList->Elements[0].Length;
			}
			else
			{
				pDesc->Packet.C2S.ControlFlags_ByteCount = ( pScatterGatherList->Elements[0].Length |
					PACKET_DESC_S2C_CTRL_IRQ_ON_COMPLETE |
					PACKET_DESC_S2C_CTRL_IRQ_ON_ERROR);
			}
			pDesc->Packet.C2S.SystemAddressPhys = pScatterGatherList->Elements[0].Address.QuadPart;
			pDesc->Packet.C2S.DmaTransaction = NULL;

			// Make sure we get a valid address
			if (pDesc->Packet.C2S.SystemAddressPhys != 0)
			{

				// On the Rx Side we use the UsedDescriptors as a total allocated count
				pDesc->Packet.DescriptorNumber = pDmaExt->NumberOfUsedDescriptors++;
				pDesc->Packet.DescFlags = DESC_FLAGS_HW_OWNED;

				// Cache the pointer to the Scatter Gather list only if this is the last descriptor
				pDesc->Packet.C2S.pScatterGatherList = pScatterGatherList;

				// Link to the next descriptor (physical address) in the chain
				pDesc->Packet.pDescPhys = pDescPhys;
				pDescPhys.QuadPart += sizeof(DMA_DESCRIPTOR_STRUCT);
				pDesc->Packet.C2S.NextDescriptorPhys = pDescPhys.LowPart;

				// Link to the next descriptor in the chain (driver usable address)
				pDesc->Packet.pNextDescriptorVirt = (PVOID)((UCHAR *)pDesc + sizeof(DMA_DESCRIPTOR_STRUCT));
				pDesc = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;

				pDmaExt->pNextDescVirt = pDesc;
				pDmaExt->pNextDescPhys = pDescPhys;
			}
			else
			{
				pDmaExt->bDescriptorAllocSuccess = FALSE;
			}
		}
		else
		{
			pDmaExt->bDescriptorAllocSuccess = FALSE;
		}
	}
	else
	{
		pDmaExt->bDescriptorAllocSuccess = FALSE;
	}
	return;
}

/*!***************************************************************************
*
*   \brief InitializeAddressablePacketDescriptors - This routine initializes the
*   C2S Addressable Packet mode DMA Descriptors
* 
* 	\param pAdapter - Pointer to this drivers context (data store)
*   \param pDmaExt - Pointer to the DMA Engine Context
* 	\param NumberDescriptors - Number of DMA descriptors to allocate
* 
*   \return STATUS_SUCCESS if it works, an error if it fails.
* 
* 	This routine is called at IRQL < DISPATCH_LEVEL.
* 
*   We MUST be running in the process where we want this memory mapped.
*****************************************************************************/
NTSTATUS InitializeAddressablePacketDescriptors(
	IN PDEVICE_EXTENSION 	pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt
	)
{
	PDMA_DESCRIPTOR_STRUCT		pDesc;
	PHYSICAL_ADDRESS			pDescPhys;
	ULONG32						descNum;
	NTSTATUS					status = STATUS_INVALID_PARAMETER;

	UNREFERENCED_PARAMETER(pDevExt);
	// Shutdown the DMA Engine
	pDmaExt->pDmaEng->ControlStatus = 0;

	if (pDmaExt->pDmaEng->ControlStatus & PACKET_DMA_CTRL_DMA_RUNNING)
	{

	}

	// Set the DMA Engine back to a restarted state
	pDmaExt->NumberOfUsedDescriptors = 0;
	// Setup the 'Next' pointers to start at the base.
	pDmaExt->pNextDescVirt = pDmaExt->pDescriptorBase;
	pDmaExt->pNextDescPhys = pDmaExt->pDescriptorBasePhysical;

	// Default to EOP Interrupt mode
	pDmaExt->pDmaEng->Packet.InterruptControl = pDevExt->InterruptMode;

	WdfSpinLockAcquire(pDmaExt->TailSpinLock);

	// Make sure the buffer was allocated
	if (pDmaExt->NumberOfDescriptors)
	{
		pDmaExt->pNextDescVirt = pDmaExt->pDescriptorBase;
		pDesc = pDmaExt->pNextDescVirt;
		pDmaExt->pTailDescVirt = pDesc;
		pDescPhys = pDmaExt->pDescriptorBasePhysical;

		// setup each of the descriptors
		for (descNum = 0; descNum < pDmaExt->NumberOfDescriptors; descNum++)
		{
			// Initialize the DMA Descriptor
			pDmaExt->pNextDescVirt->Packet.C2S.SystemAddressVirt = 0;
			pDesc->Packet.C2S.StatusFlags_BytesCompleted = 0;
			pDesc->Packet.C2S.UserStatus = 0;
			pDesc->Packet.C2S.CardAddress = 0;
			pDesc->Packet.C2S.ControlFlags_ByteCount = 0;
			pDesc->Packet.C2S.SystemAddressPhys = 0;
			pDesc->Packet.C2S.DmaTransaction = NULL;
			pDesc->Packet.C2S.pScatterGatherList = 0;
			pDesc->Packet.DescriptorNumber = descNum;
			pDesc->Packet.pDescPhys = pDescPhys;
			pDesc->Packet.DescFlags = DESC_FLAGS_ADDRESSABLE_MODE;

			// update pointers
			if (descNum == (pDmaExt->NumberOfDescriptors - 1))
			{
				// Link back to the top of the Descriptor pool
				pDesc->Packet.pNextDescriptorVirt = (PVOID *)pDmaExt->pDescriptorBase;
				pDesc->Packet.S2C.NextDescriptorPhys = pDmaExt->pDescriptorBasePhysical.LowPart;
			}
			else
			{
				// Link to the Next Descriptor
				pDescPhys.QuadPart += sizeof(DMA_DESCRIPTOR_STRUCT);
				pDesc->Packet.S2C.NextDescriptorPhys = pDescPhys.LowPart;
				pDesc->Packet.pNextDescriptorVirt = (PVOID)((UCHAR *)pDesc + sizeof(DMA_DESCRIPTOR_STRUCT));
				pDesc = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;
			}
		}

		// setup the descriptor pointers
		pDmaExt->pDmaEng->Packet.NextDescriptorPtr = pDmaExt->pDescriptorBasePhysical.LowPart;
		pDmaExt->pDmaEng->Packet.SoftwareDescriptorPtr = pDmaExt->pDescriptorBasePhysical.LowPart;
		pDmaExt->pDmaEng->Packet.CompletedDescriptorPtr = 0;

		// Now enable the DMA Engine
		pDmaExt->pDmaEng->ControlStatus = (COMMON_DMA_CTRL_IRQ_ENABLE | PACKET_DMA_CTRL_DMA_ENABLE);
		status = STATUS_SUCCESS;
	}
	else
	{
		// Not enough descriptors available
		status = STATUS_INSUFFICIENT_RESOURCES;
	}
	WdfSpinLockRelease(pDmaExt->TailSpinLock);
	return status;
}


/*!***************************************************************************
*
* 	\brief FreeRxDecriptors -	 This routine frees the allocated Receive
* 		decriptors and any resources associated to them
* 
* 	\param pDevExt - Pointer to this drivers context (data store)
*   \param pDmaExt - Pointer to the DMA Engine Context
* 
*   \return None
* 
* 	This routine is called at IRQL < DISPATCH_LEVEL.
*
*****************************************************************************/
void FreeRxDescriptors(
	IN PDEVICE_EXTENSION 	pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt)
{
	PDMA_DESCRIPTOR_STRUCT		pDesc;
	PPACKET_GENENRATOR_STRUCT	pPacketGen;
	PDMA_XFER					pDmaXfer = NULL;
	WDFREQUEST					Request;
	LONG						i;
	NTSTATUS					status;

	pPacketGen = (PPACKET_GENENRATOR_STRUCT)&pDevExt->pDmaRegisters->packetGen[pDmaExt->DmaEngine];
	pDesc = pDmaExt->pDescriptorBase;

	// Make sure the packet generator is shutdown
	memset(pPacketGen, 0, sizeof(PACKET_GENENRATOR_STRUCT));
	//pPacketGen->Control = 0;
	//pPacketGen->NumPackets = 0;
	ShutdownDMAEngine(pDevExt, pDmaExt);

	WdfSpinLockAcquire(pDmaExt->TailSpinLock);
	for (i = 0; i < pDmaExt->NumberOfUsedDescriptors; i++)
	{
		if ((pDesc->Packet.C2S.pScatterGatherList != 0) )
		{
			//
			// Release the ScatterGatherList
			//
			pDmaExt->pReadDmaAdapter->DmaOperations->PutScatterGatherList(
				pDmaExt->pReadDmaAdapter,
				pDesc->Packet.C2S.pScatterGatherList,
				FALSE);
		}
		if (pDesc->Packet.C2S.DmaTransaction != NULL)
		{
			if (pDesc->Packet.C2S.ControlFlags_ByteCount & PACKET_DESC_C2S_STAT_END_OF_PACKET)
			{
				pDmaXfer = DMAXferContext(pDesc->Packet.C2S.DmaTransaction);
				if (pDmaXfer != NULL)
				{
					WdfDmaTransactionDmaCompletedFinal(	pDesc->Packet.C2S.DmaTransaction,
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
						MmUnlockPages(pDmaXfer->pMdl);
						IoFreeMdl(pDmaXfer->pMdl);
					}
					pDmaXfer->Request = NULL;
				}
				// Release the Transaction record
				WdfDmaTransactionRelease (pDesc->Packet.C2S.DmaTransaction);
				WdfObjectDelete(pDesc->Packet.C2S.DmaTransaction);
				pDmaXfer = NULL;
			}
		}
		// Idle out the descriptor
		pDesc->Packet.C2S.DmaTransaction = NULL;
		pDesc->Packet.C2S.StatusFlags_BytesCompleted = 0;
		pDesc->Packet.C2S.UserStatus = 0;
		pDesc->Packet.C2S.CardAddress = 0;
		pDesc->Packet.C2S.ControlFlags_ByteCount = 0;
		pDesc->Packet.C2S.SystemAddressPhys = 0;
		pDesc->Packet.C2S.pScatterGatherList = 0;
		pDesc = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;
	}
	// Set the DMA Engine back to a restarted state
	pDmaExt->NumberOfUsedDescriptors = 0;
	// Setup the 'Next' pointers to start at the base.
	pDmaExt->pNextDescVirt = pDmaExt->pDescriptorBase;
	pDmaExt->pNextDescPhys = pDmaExt->pDescriptorBasePhysical;
	// Reset the DMA Engine pointers back to the initialized state
	pDmaExt->pDmaEng->Packet.NextDescriptorPtr =	pDmaExt->pDescriptorBasePhysical.LowPart;
	pDmaExt->pDmaEng->Packet.SoftwareDescriptorPtr =	pDmaExt->pDescriptorBasePhysical.LowPart;
	pDmaExt->pDmaEng->Packet.CompletedDescriptorPtr = 0;
	WdfSpinLockRelease(pDmaExt->TailSpinLock);
}


/*!***************************************************************************
*
* 	\brief InitializeTxDescriptors - This routine allocates and
*	initializes the Packet Send (Transmit) DMA Descriptors
* 
* 	\param pDevExt - Pointer to this drivers context (data store)
*   \param pDmaExt - Pointer to the DMA Engine Context
* 
*   \return STATUS_SUCCESS if it works, an error if it fails.
* 
* 	This routine is called at IRQL < DISPATCH_LEVEL.
*
*****************************************************************************/
NTSTATUS InitializeTxDescriptors(
	IN PDEVICE_EXTENSION 	pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt
	)
{
	PDMA_DESCRIPTOR_STRUCT		pDesc;
	PHYSICAL_ADDRESS			pDescPhys;
	ULONG32						descNum;
	NTSTATUS					status = STATUS_INSUFFICIENT_RESOURCES;

	UNREFERENCED_PARAMETER(pDevExt);

	// Shutdown the DMA Engine
	pDmaExt->pDmaEng->ControlStatus = 0;
	// Set the DMA Engine back to a restarted state
	pDmaExt->NumberOfUsedDescriptors = 0;

	if (pDmaExt->pDmaEng->ControlStatus & PACKET_DMA_CTRL_DMA_RUNNING)
	{
	}

	WdfSpinLockAcquire(pDmaExt->TailSpinLock);

	// Make sure the buffer was allocated
	if (pDmaExt->NumberOfDescriptors)
	{
		pDmaExt->pNextDescVirt = pDmaExt->pDescriptorBase;
		pDesc = pDmaExt->pNextDescVirt;
		pDmaExt->pTailDescVirt = pDesc;
		pDescPhys = pDmaExt->pDescriptorBasePhysical;

		// setup each of the descriptors
		for (descNum = 0; descNum < pDmaExt->NumberOfDescriptors; descNum++)
		{
			// setup the descriptor
			pDesc->Packet.S2C.StatusFlags_BytesCompleted = 0;
			pDesc->Packet.S2C.UserControl = 0;
			pDesc->Packet.S2C.CardAddress = 0;
			pDesc->Packet.S2C.ControlFlags_ByteCount = ( 0 |
				PACKET_DESC_S2C_CTRL_IRQ_ON_COMPLETE |
				PACKET_DESC_S2C_CTRL_IRQ_ON_ERROR |
				PACKET_DESC_S2C_CTRL_END_OF_PACKET |
				PACKET_DESC_S2C_CTRL_START_OF_PACKET);
			pDesc->Packet.S2C.SystemAddressPhys = (ULONG64)-1;
			pDesc->Packet.S2C.DmaTransaction = NULL;

			pDesc->Packet.DescriptorNumber = descNum;
			pDesc->Packet.pDescPhys = pDescPhys;

			// update pointers
			if (descNum == (pDmaExt->NumberOfDescriptors - 1))
			{
				// Link back to the top of the Descriptor pool
				pDesc->Packet.pNextDescriptorVirt = (PVOID *)pDmaExt->pDescriptorBase;
				pDesc->Packet.S2C.NextDescriptorPhys = pDmaExt->pDescriptorBasePhysical.LowPart;
			}
			else
			{
				// Link to the Next Descriptor
				pDescPhys.QuadPart += sizeof(DMA_DESCRIPTOR_STRUCT);
				pDesc->Packet.S2C.NextDescriptorPhys = pDescPhys.LowPart;
				pDesc->Packet.pNextDescriptorVirt = (PVOID)((UCHAR *)pDesc + sizeof(DMA_DESCRIPTOR_STRUCT));
				pDesc = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;
			}
		}

		// setup the descriptor pointers
		pDmaExt->pDmaEng->Packet.NextDescriptorPtr =	pDmaExt->pDescriptorBasePhysical.LowPart;
		pDmaExt->pDmaEng->Packet.SoftwareDescriptorPtr = pDmaExt->pDescriptorBasePhysical.LowPart;
		pDmaExt->pDmaEng->Packet.CompletedDescriptorPtr = 0;

		// Now enable the DMA Engine
		pDmaExt->pDmaEng->ControlStatus = (COMMON_DMA_CTRL_IRQ_ENABLE | PACKET_DMA_CTRL_DMA_ENABLE);
		status = STATUS_SUCCESS;
	}
	else
	{
		// Not enough descriptors available
	}

	WdfSpinLockRelease(pDmaExt->TailSpinLock);
	return status;
}



/*!***************************************************************************
*
* 	\brief ShutdownDMAEngine -	 This routine tries to do an orderly shutdown
* 		of the DMA Engine specified
* 
* 	\param pDevExt - Pointer to this drivers context (data store)
*   \param pDmaExt - Pointer to the DMA Engine Context
* 
*   \return None
* 
* 	This routine is called at IRQL < DISPATCH_LEVEL.
*
*****************************************************************************/
void ShutdownDMAEngine(
	IN PDEVICE_EXTENSION 	pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt)
{
	ULONG	TimeoutLoop;

	// Shutdown the DMA Engine, DMA_ENABLE = 0
	pDmaExt->pDmaEng->ControlStatus = 0;

	// Wait for the DMA Engine to go idle
	TimeoutLoop = DEFAULT_TIMEOUT_LOOP;
	while (pDmaExt->pDmaEng->ControlStatus & PACKET_DMA_CTRL_DMA_RUNNING)
	{
		if (--TimeoutLoop == 0)
		{
			// If we timeout, reset the pointers and try again.
			pDmaExt->pDmaEng->Packet.NextDescriptorPtr =	0;
			pDmaExt->pDmaEng->Packet.SoftwareDescriptorPtr =	0;
			pDmaExt->pDmaEng->Packet.CompletedDescriptorPtr = 0;
			pDmaExt->pDmaEng->ControlStatus = 0;
			// DMA Engine is not responding to Idle request
			break;
		}

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
			break;
		}
	}
	// Clear any persistant bits just to make sure there is no residue from the reset request
	pDmaExt->pDmaEng->ControlStatus = (COMMON_DMA_CTRL_IRQ_ACTIVE |
		PACKET_DMA_CTRL_DESC_COMPLETE |
		PACKET_DMA_CTRL_DESC_FETCH_ERROR |
		PACKET_DMA_CTRL_DESC_ALIGN_ERROR |
		PACKET_DMA_CTRL_DESC_SW_ABORT_ERROR |
		PACKET_DMA_CTRL_DESC_CHAIN_END |
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
			break;
		}
	}

	// If we just reset the S2C Engine we must set it back up
	if (pDmaExt->DmaType == DMA_TYPE_PACKET_SEND)
	{
		InitializeTxDescriptors(pDevExt, pDmaExt);
	}

}

/*!***************************************************************************
*
*   \brief InitializeS2mmPacketDescriptors - This routine initializes the
*   S2MM Packet mode DMA Descriptors
* 
* 	\param pDevExt - Pointer to the device context (data store)
*   \param pDmaExt - Pointer to the DMA Engine Context
*
*   \return STATUS_SUCCESS if it works, an error if it fails.
* 
* 	This routine is called at IRQL < DISPATCH_LEVEL.
* 
*****************************************************************************/
NTSTATUS InitializeS2mmPacketDescriptors(
	IN PDEVICE_EXTENSION 	pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt
	)
{
	PS2MM_DMA_DESCRIPTOR_STRUCT		pS2mmDesc;
	PS2MM_CTRL_STAT_STRUCT			pS2mmCtrl;
	PS2MM_LAYER_EXTENSION			pS2mmExt;
	UCHAR                           i;
	ULONG						    j, k;
	ULONG32							inFrameSize;
	USHORT							inHSizeByBytes;
	USHORT							inHSizeByPixels;
	USHORT							vSize;
	ULONG32							hSize;
	ULONG32							s2mmVSize;
	ULONG32							s2mmHSize;
	USHORT							frameRate;
	USHORT							framesPerTrans;
	ULONG32							frameIndexCap;
	NTSTATUS						status			=	STATUS_INVALID_PARAMETER;
	ULONG32							tempHSize = 1;
	ULONG32							tempVSize = 1;
	ULONG32							DMAimageFrameLen;
	ULONG32							dbgBufAddr, dbgNxtPtr;
//	UINT64							DbgStatus = 0;
	UNREFERENCED_PARAMETER(pDmaExt);

	// Setup Bar 0: enable user interrupt
	(pDevExt->pDmaRegisters->commonControl).ControlStatus |= 0x0000003f; //set bit[4] and bit[5] to be 1.
	
	pDevExt->pBar3Map->globalGenCtrl.Layout.StopRun_FpgaRev = 0x00; //stop generator 

	pS2mmExt = pDevExt->pS2mmLayerExt;

	for (i = 1; i < 64; i*=2)
	{
		tempHSize = pDevExt->dataHSize / i;
		if (tempHSize < 0xFFFF)
		{			
			tempVSize = pDevExt->VSize * i;
			DEBUGP(DEBUG_INFO, "dataHSize divisor = %d", i);
			break;
		}
	}

	if (tempVSize > 0x1FFF)
	{
		DEBUGP(DEBUG_ERROR, "InitializeS2mmPacketDescriptors: tempVSize > 0x1FFF ")
		return status;
	}

	s2mmHSize = tempHSize;
	s2mmVSize = tempVSize;
	hSize = (USHORT)pDevExt->dataHSize;
	inHSizeByPixels = (USHORT)pDevExt->HSize;
	inHSizeByBytes = (USHORT)pDevExt->dataHSize;
	vSize = (USHORT)pDevExt->linesPerStripe;
	pDevExt->VSize = (USHORT)pDevExt->linesPerStripe;

	//
	inFrameSize = hSize * vSize;  
	pDevExt->FrameSizeInPixels = (USHORT)pDevExt->HSize * (USHORT)pDevExt->VSize;
	frameIndexCap = (0x10000000 / S2MM_DESCRS_PER_BLK / (inHSizeByBytes*vSize));
	frameRate		= pDevExt->FrameRate;
	framesPerTrans	= pDevExt->FramesPerTransfer;
	DMAimageFrameLen = s2mmHSize * s2mmVSize;
	DEBUGP(DEBUG_INFO, "tempHSize %d (0x%x), tempVSize %d (0x%x), DMAimageFrameLen %d (0x%x)", tempHSize, tempHSize, tempVSize, tempVSize, DMAimageFrameLen, DMAimageFrameLen);
	DEBUGP(DEBUG_INFO, "InitializeS2mmPacketDescriptors:     HSize %d, VSize %d, pDevExt->dataHSize %d (0x%x), inHSizeByBytes %d, frameIndexCap %d",
		pDevExt->HSize, pDevExt->VSize, pDevExt->dataHSize, pDevExt->dataHSize, inHSizeByBytes, frameIndexCap);
	DEBUGP(DEBUG_INFO, "    inFrameSize %d(0x%x), s2mmHSize %d, s2mmVSize %d", inFrameSize, inFrameSize, s2mmHSize, s2mmVSize);

	DEBUGP(DEBUG_INFO, "                                     linesPerStripe %d", pDevExt->linesPerStripe);
	DEBUGP(DEBUG_INFO, "    frameRate %d, framesPerTrans %d", frameRate, framesPerTrans);
	DEBUGP(DEBUG_INFO, "    pDevExt->DataBufferStartAddress 0x%x, pDevExt->MAXframeLengthPerChan %d (0x%x), DDR3startAddressFor2ndPingPongBank %d (0x%x) ",
		pDevExt->DataBufferStartAddress, pDevExt->MAXframeLengthPerChan, pDevExt->MAXframeLengthPerChan, pDevExt->DDR3startAddressFor2ndPingPongBank, pDevExt->DDR3startAddressFor2ndPingPongBank);

	//if (pDevExt->mode < 2)
	{
		//reset S2MM DMA register
		for (i = 0; i < S2MM_CHANNEL_PER_BOARD; i++)
		{
			if (pDevExt->ChannelDescriptor[i] == TRUE)
			{
				//pDevExt->pBar2Controls->ctrlCh[i].ctrl.SR0_CR0 = 0x12;//0x51;
				pDevExt->pBar2Controls->ctrlCh[i].ctrl.SR0_CR0 = 0x13;
				DEBUGP(DEBUG_INFO, "pBar2Controls->ctrlCh[%d] => 0x13->0x12 (toggled)", i);
				pDevExt->pBar2Controls->ctrlCh[i].ctrl.SR0_CR0 = 0x12;
			}
		}
	}

	
	// Set S2MM DMA register
	for (i = 0; i < S2MM_CHANNEL_PER_BOARD; i++)
	{
		// Make sure the buffer was allocated
		// Here we let number of descriptors of a chain equals to frame rate, so, every one second, an interrupt would be generated to inform NW DMA to start.
		if (framesPerTrans > MAX_S2MM_BLKS_PER_CHANNEL || (framesPerTrans * inFrameSize) > INNER_MEM_SIZE_PER_CHANNEL)
		{
			// Not enough descriptors available
			status = STATUS_INSUFFICIENT_RESOURCES;
			DEBUGP(DEBUG_ERROR, "STATUS_INSUFFICIENT_RESOURCES - transfer too large")
			goto Exit;
		}
		pS2mmExt->pS2mm[i].IsDataReady = 0;
		if (pDevExt->ChannelDescriptor[i] == FALSE)
		{
			continue;
		}

		pS2mmExt->IsFull = 0;
		pS2mmExt->BankHead = 0;
		pS2mmExt->BankTail = 0;
		pS2mmExt->AcqBufOffset = 0;
		pS2mmExt->OffsetsIndexHead = 0;
		pS2mmExt->OffsetsIndexTail = 0;
		pS2mmExt->IndexCap = (USHORT)((frameIndexCap > MAX_S2MM_BLKS_PER_CHANNEL) ? MAX_S2MM_BLKS_PER_CHANNEL : frameIndexCap);

		// setup BAR 1 and BAR 2
		pS2mmCtrl = &(pDevExt->pBar2Controls->ctrlCh[i].ctrl);
		//pS2mmCtrl->SR0_CR0 = 0x12; //0x52;
		pS2mmCtrl->ChainStartAddr = 0x0;
		pS2mmCtrl->ChainTailAddr = 0x510;

		DEBUGP(DEBUG_INFO, "pDevExt->DataBufferStartAddress 0x%x", pDevExt->DataBufferStartAddress);
		//TODO: find the best number for this section, should be frames per second
		//but 2 seems to work more consistently in live mode
		pS2mmCtrl->ChainIrqThreshold = (USHORT)(framesPerTrans); // - 1); 
		DEBUGP(DEBUG_INFO, "ChainIrqThreshold %d", (USHORT)(framesPerTrans));
		for(j=0; j < framesPerTrans; j++)
		{
			for(k=0; k < S2MM_DESCRS_PER_BLK; k++)
			{
				DEBUGP(DEBUG_INFO, "Ch[%d], Frame[%d], ppBANK[%d] ", i, j, k);
				pS2mmDesc = &(pDevExt->pBar1Descriptors->descCh[i].descBlk[j].desc[k]);
				DEBUGP(DEBUG_INFO, "(pBAR1)pS2mmDesc @0x%p", pS2mmDesc);
				dbgNxtPtr = (((j + 1) % framesPerTrans) * sizeof(S2MM_DESC_BLK_STRUCT) + sizeof(S2MM_DMA_DESCRIPTOR_STRUCT) * k + ((UINT64)i << 16));
				dbgBufAddr = pDevExt->DataBufferStartAddress + ((i + 0) % 4) * pDevExt->MAXframeLengthPerChan + (j)*inFrameSize + k * pDevExt->DDR3startAddressFor2ndPingPongBank;

				pS2mmDesc->NxtDescPtr = dbgNxtPtr;
				DEBUGP(DEBUG_INFO, "(pBAR1)pS2mmDesc->NxtDescPtr (@0x%p) 0x%x", &pS2mmDesc->NxtDescPtr, pS2mmDesc->NxtDescPtr);
				pS2mmDesc->BuffAddr = dbgBufAddr;
				DEBUGP(DEBUG_INFO, "(pBAR1)pS2mmDesc->BuffAddr (@0x%p) 0x%x", &pS2mmDesc->BuffAddr, pS2mmDesc->BuffAddr);

				DEBUGP(DEBUG_INFO, "Wrote NextDescPtr 0x%x, BuffAddr 0x%x", dbgNxtPtr, dbgBufAddr);

				pS2mmDesc->Usr_Cache = (0x3<<24); // should be defined as a constant somewhere.		
				DEBUGP(DEBUG_INFO, "(pBAR1)pS2mmDesc->Usr_Cache (@0x%p) 0x%x", &pS2mmDesc->Usr_Cache, pS2mmDesc->Usr_Cache);
				pS2mmDesc->VSize_Stride = (s2mmVSize << 19) | s2mmHSize; // bit 19-31 are for VSize		
				DEBUGP(DEBUG_INFO, "(pBAR1)pS2mmDesc->VSize_Stride (@0x%p) 0x%x", &pS2mmDesc->VSize_Stride, pS2mmDesc->VSize_Stride);
				pS2mmDesc->HSize = s2mmHSize;
				DEBUGP(DEBUG_INFO, "(pBAR1)pS2mmDesc->HSize (@0x%p) 0x%x", &pS2mmDesc->HSize, pS2mmDesc->HSize);
				pS2mmDesc->DescFlags = 0x0;
				DEBUGP(DEBUG_INFO, "(pBAR1)pS2mmDesc->DescFlags (@0x%p) 0x%x", &pS2mmDesc->DescFlags, pS2mmDesc->DescFlags);
			}
		}
	}

	for (i = 0; i < S2MM_CHANNEL_PER_BOARD; i++)
	{
		//if (pDevExt->ChannelDescriptor[i] == TRUE)
		//{
		//	pDevExt->pBar2Controls->ctrlCh[i].ctrl.SR0_CR0 = 0x13;//0x53;
		//}/*else
		//{
		//	pDevExt->pBar2Controls->ctrlCh[i].ctrl.SR0_CR0 = 0x50;
		//}*/
		if (pDevExt->ChannelDescriptor[i] == TRUE)
		{
			DEBUGP(DEBUG_INFO, "pBAR2Controls->ctrlCh[%d].ctrl.SR0_CR0 =>0x13 (Cyclic/Run/Normal)", i);
			pDevExt->pBar2Controls->ctrlCh[i].ctrl.SR0_CR0 = 0x13;  // (Bit0 is CONFIG_DMA, 0 is "configure mode", 1 "normal"
		}else
		{
			DEBUGP(DEBUG_INFO, "pBAR2Controls->ctrlCh[%d].ctrl.SR0_CR0 =>0x12", i);
			pDevExt->pBar2Controls->ctrlCh[i].ctrl.SR0_CR0 = 0x12;
		}
	}

//	DbgStatus = ((ULONG64)pDevExt->pBar3Map->globalGenCtrl.Layout.testDiag << 32) | pDevExt->pBar3Map->globalGenCtrl.Layout.StopRun_FpgaRev; // BAR3 offset 4 | offset 0 
//	DecodeDbgStatus(DbgStatus);

	pDevExt->pBar3Map->globalGenCtrl.Layout.StopRun_FpgaRev = 0x03; // Global start and LED on

	DEBUGP(DEBUG_INFO, ">>>StopRun ASSERTED!");
//	DbgStatus = ((ULONG64)pDevExt->pBar3Map->globalGenCtrl.Layout.testDiag << 32) | pDevExt->pBar3Map->globalGenCtrl.Layout.StopRun_FpgaRev; // BAR3 offset 4 | offset 0 
//	DecodeDbgStatus(DbgStatus);


	status = STATUS_SUCCESS;

Exit:
	/*WdfSpinLockRelease(pDmaExt->TailSpinLock);*/
	return status;
}