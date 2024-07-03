#include "Driver.h"
#include "Trace.h"

// Local defines
EVT_WDF_PROGRAM_DMA  PacketProgramS2CDmaCallback;
EVT_WDF_PROGRAM_DMA  PacketProgramC2SDmaCallback;

BOOLEAN PacketProgramS2CDmaCallback (
	IN WDFDMATRANSACTION	DmaTransaction,
	IN WDFDEVICE			Device,
	IN WDFCONTEXT			Context,
	IN WDF_DMA_DIRECTION	Direction,
	IN PSCATTER_GATHER_LIST  SgList);

BOOLEAN PacketProgramC2SDmaCallback (
	IN WDFDMATRANSACTION  	DmaTransaction,
	IN WDFDEVICE  			Device,
	IN WDFCONTEXT  			Context,
	IN WDF_DMA_DIRECTION  	Direction,
	IN PSCATTER_GATHER_LIST  SgList);


/**************************************************************************** 
* 
**  S2C Packet Mode routines
**
*****************************************************************************/ 

// FIFO Packet Mode functions

/*!***************************************************************************
*
* 	\brief PacketStartSend - This routine setups the send request then calls
* 	WdfDmaTransactionExecute to start or queue the actual DMA request
* 
* 	\param Request - WDF I/O Request (PACKET_SEND_IOCTL)
* 	\param DevExt - WDF Driver context
* 	\param pSendPacket - Contents of the PACKET_SEND_IOCTL request
*	 
* 	\return NTSTATUS - STATUS_SUCCESS if it works, FALSE if error.
* 
*****************************************************************************/
NTSTATUS PacketStartSend (
	IN WDFREQUEST			Request,
	IN PDEVICE_EXTENSION	pDevExt,
	IN PPACKET_SEND_STRUCT	pSendPacket
	)
{
	NTSTATUS 				status = STATUS_SUCCESS;
	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;
	WDFDMATRANSACTION  		DmaTransaction;
	PDMA_XFER				pDmaXfer;
	WDF_OBJECT_ATTRIBUTES	attributes;
	PMDL					Mdl;
	PVOID					VirtualAddress;
	ULONG					Length;

	status = GetDMAEngineContext(pDevExt, pSendPacket->EngineNum, &pDmaExt);
	if (!NT_SUCCESS(status)) 
	{
		return status;
	}
	// Retreive all the information we need from the IOCTL Request

	status = WdfRequestRetrieveOutputWdmMdl(Request, &Mdl);
	if (NT_SUCCESS(status)) 
	{
		VirtualAddress = MmGetMdlVirtualAddress(Mdl);
		Length = MmGetMdlByteCount(Mdl);

		if ((ULONGLONG) Length >= pSendPacket->Length)
		{
			// Create a DMA Transaction object just for this transfer
			WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, DMA_XFER);
			status = WdfDmaTransactionCreate(pDmaExt->DmaEnabler,
				&attributes,
				&DmaTransaction);

			// if no errors kick off the DMA, first save a pointer to the request.
			if (NT_SUCCESS(status))
			{
				// Keep a pointer the Request and the accumulated byte count in the Transaction
				pDmaXfer = DMAXferContext(DmaTransaction);
				pDmaXfer->Request = Request;
				pDmaXfer->bytesTransferred = 0;
				pDmaXfer->CardAddress = pSendPacket->CardOffset;
				pDmaXfer->UserControl = pSendPacket->UserControl;
				pDmaXfer->Mode = 0;
				pDmaXfer->PacketStatus = 0;

				status = WdfDmaTransactionInitialize(DmaTransaction,
					(PFN_WDF_PROGRAM_DMA) PacketProgramS2CDmaCallback,
					pDmaExt->DmaDirection,
					Mdl,	
					VirtualAddress,
					(size_t)pSendPacket->Length);

				if (NT_SUCCESS(status))
				{
					// Put the request on a WDF maintained Queue in case it gets canceled before we complete it
					WdfSpinLockAcquire(pDmaExt->TailSpinLock);
					status = WdfRequestForwardToIoQueue (Request, pDmaExt->TransactionQueue);
					WdfSpinLockRelease(pDmaExt->TailSpinLock);
					if (NT_SUCCESS(status))
					{
						// start the DMA, via PacketProgramDmaCallback
						status = WdfDmaTransactionExecute(DmaTransaction, pDmaExt);
						if (!NT_SUCCESS(status))
						{
							// Make sure we get the Request off the queue.
							WdfSpinLockAcquire(pDmaExt->TailSpinLock);
							FindRequestByRequest(pDmaExt, Request);
							WdfSpinLockRelease(pDmaExt->TailSpinLock);
							WdfObjectDelete(DmaTransaction);
						}
					}
					else
					{
						WdfObjectDelete(DmaTransaction);
					}
				}
				else
				{
					WdfObjectDelete(DmaTransaction);
				}
			}
			else
			{
			}
		}
		else
		{
			status = STATUS_INVALID_PARAMETER;
		}
	}
	else
	{
	}
	return status;
}

// Addressable Packet Mode functions

/*!***************************************************************************
*
* 	\brief PacketStartWrite - This routine setups the send request then calls
* 	WdfDmaTransactionExecute to start or queue the actual DMA request
* 
* 	\param Request - WDF I/O Request (PACKET_SEND_IOCTL)
* 	\param DevExt - WDF Driver context
* 	\param pWritePacket - Contents of the PACKET_SEND_IOCTL request
*	 
* 	\return NTSTATUS - STATUS_SUCCESS if it works, FALSE if error.
* 
*****************************************************************************/
NTSTATUS PacketStartWrite (
	IN WDFREQUEST			Request,
	IN PDEVICE_EXTENSION	pDevExt,
	IN PPACKET_WRITE_STRUCT	pWritePacket
	)
{
	NTSTATUS 				status = STATUS_SUCCESS;
	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;
	WDFDMATRANSACTION  		DmaTransaction;
	PDMA_XFER				pDmaXfer;
	WDF_OBJECT_ATTRIBUTES	attributes;
	PMDL					Mdl;
	PVOID					VirtualAddress;
	ULONG					Length;

	status = GetDMAEngineContext(pDevExt, pWritePacket->EngineNum, &pDmaExt);
	if (!NT_SUCCESS(status)) 
	{
		return status;
	}
	// Retreive all the information we need from the IOCTL Request

	status = WdfRequestRetrieveOutputWdmMdl(Request, &Mdl);
	if (NT_SUCCESS(status)) 
	{
		VirtualAddress = MmGetMdlVirtualAddress(Mdl);
		Length = MmGetMdlByteCount(Mdl);

		if ((ULONGLONG) Length >= pWritePacket->Length)
		{
			// Create a DMA Transaction object just for this transfer
			WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, DMA_XFER);
			status = WdfDmaTransactionCreate(pDmaExt->DmaEnabler,
				&attributes,
				&DmaTransaction);

			// if no errors kick off the DMA, first save a pointer to the request.
			if (NT_SUCCESS(status))
			{
				// Keep a pointer the Request and the accumulated byte count in the Transaction
				pDmaXfer = DMAXferContext(DmaTransaction);
				pDmaXfer->Request = Request;
				pDmaXfer->bytesTransferred = 0;
				pDmaXfer->CardAddress = pWritePacket->CardOffset;
				pDmaXfer->UserControl = pWritePacket->UserControl;
				pDmaXfer->Mode = pWritePacket->ModeFlags;
				pDmaXfer->PacketStatus = 0;
				DEBUGP(DEBUG_INFO, "pWritePacket->CardOffset 0x%llx, Len %d (0x%x)", pWritePacket->CardOffset, pWritePacket->Length, pWritePacket->Length);

				status = WdfDmaTransactionInitialize(DmaTransaction,
					(PFN_WDF_PROGRAM_DMA) PacketProgramS2CDmaCallback,
					pDmaExt->DmaDirection,
					Mdl,	
					VirtualAddress,
					(size_t)pWritePacket->Length);

				if (NT_SUCCESS(status))
				{
					// Put the request on a WDF maintained Queue in case it gets canceled before we complete it
					WdfSpinLockAcquire(pDmaExt->TailSpinLock);
					status = WdfRequestForwardToIoQueue (Request, pDmaExt->TransactionQueue);
					WdfSpinLockRelease(pDmaExt->TailSpinLock);
					if (NT_SUCCESS(status))
					{
						// start the DMA, via PacketProgramDmaCallback
						status = WdfDmaTransactionExecute(DmaTransaction, pDmaExt);
						if (!NT_SUCCESS(status))
						{
							// Make sure we get the Request off the queue.
							WdfSpinLockAcquire(pDmaExt->TailSpinLock);
							FindRequestByRequest(pDmaExt, Request);
							WdfSpinLockRelease(pDmaExt->TailSpinLock);
							WdfObjectDelete(DmaTransaction);
						}
					}
					else
					{
						WdfObjectDelete(DmaTransaction);
					}
				}
				else
				{
					WdfObjectDelete(DmaTransaction);
				}
			}
			else
			{
			}
		}
		else
		{
			status = STATUS_INVALID_PARAMETER;
		}
	}
	else
	{
	}
	return status;
}


/*!***************************************************************************
*
* 	\brief PacketProgramS2CDmaCallback - This routine performs the actual
*	programming of the Packet DMA engine and descriptors
* 
* 	\param DmaTransaction - WDF DMA Transaction handle
* 	\param Device - WDF DMA Device handle
* 	\param Context - WDF Context pointer (our DMA Engine pointer)
* 	\param Direction - WDF Direction of DAM Transfer
* 	\param SgList - Pointer to the WDF Scatter/Gather list
*	 
*   \return TRUE if it works, FALSE if error.
*
* 	NOTE: This function is called by WdfDmaTransactionExecute
*   NOTE: This function is called in the transfer sequence.
* 		 It should be optimized to be as fast as possible.
*
*****************************************************************************/
BOOLEAN PacketProgramS2CDmaCallback (
	IN WDFDMATRANSACTION  	DmaTransaction,
	IN WDFDEVICE  			Device,
	IN WDFCONTEXT  			Context,
	IN WDF_DMA_DIRECTION  	Direction,
	IN PSCATTER_GATHER_LIST  SgList
	)
{
	NTSTATUS						status = STATUS_SUCCESSFUL;
	PDEVICE_EXTENSION				pDevExt;
	PDMA_ENGINE_DEVICE_EXTENSION 	pDmaExt;
	PDMA_DESCRIPTOR_STRUCT			pDesc;
	PDMA_DESCRIPTOR_STRUCT			pLastDesc = NULL;
	PDMA_XFER						pDmaXfer;
	ULONG64 						CardAddress;
	ULONG32							numAvailDescriptors;
	ULONG32							Control;
	ULONG32							descNum;

	UNREFERENCED_PARAMETER(Direction);

	// Get Device Extensions
	pDevExt = ThorDaqDrvGetDeviceContext(Device);
	pDmaExt = (PDMA_ENGINE_DEVICE_EXTENSION) Context;
	pDmaXfer = DMAXferContext(DmaTransaction);
	CardAddress = pDmaXfer->CardAddress;

	WdfSpinLockAcquire(pDmaExt->HeadSpinLock);
	// Determine number of available descriptors
	numAvailDescriptors = pDmaExt->NumberOfDescriptors - pDmaExt->NumberOfUsedDescriptors;

	if (numAvailDescriptors >= SgList->NumberOfElements)
	{
		// Lock the access to the head pointer, get the next pointer, calc the new head, store it back and release the lock
		pDesc = pDmaExt->pNextDescVirt;

		// Setup descriptor control, Interrupt when the DMA is stopped short
		Control = PACKET_DESC_S2C_CTRL_START_OF_PACKET;
		if (pDmaXfer->Mode & READ_WRITE_MODE_FLAG_FIFO)
		{
			Control |= PACKET_DESC_S2C_CTRL_FIFO_OVERRIDE;
		}

		// setup each of the descriptors
		for (descNum = 0; descNum < SgList->NumberOfElements; descNum++)
		{

			if (descNum == (SgList->NumberOfElements - 1))
			{
				// end the processing here, Only interrupt on completion of the last DMA descriptor and
				// when the DMA is stopped short.
				Control |= PACKET_DESC_S2C_CTRL_END_OF_PACKET | PACKET_DESC_S2C_CTRL_IRQ_ON_COMPLETE | 
					PACKET_DESC_S2C_CTRL_IRQ_ON_ERROR;
			}

			// setup the descriptor
			pDesc->Packet.S2C.StatusFlags_BytesCompleted = SgList->Elements[descNum].Length & PACKET_DESC_BYTE_COUNT_MASK;
			// Set the User Control field in the first packet only
			pDesc->Packet.S2C.UserControl = pDmaXfer->UserControl;
			pDmaXfer->UserControl = 0;

			pDesc->Packet.S2C.CardAddress  = (ULONG32)(CardAddress & 0xFFFFFFFF);
			pDesc->Packet.S2C.ControlFlags_ByteCount = (ULONG32)((CardAddress & 0xF00000000) >> 12);
			pDesc->Packet.S2C.ControlFlags_ByteCount |= (SgList->Elements[descNum].Length & PACKET_DESC_BYTE_COUNT_MASK) | Control;
			pDesc->Packet.S2C.SystemAddressPhys = SgList->Elements[descNum].Address.QuadPart;
			pDesc->Packet.S2C.DmaTransaction = DmaTransaction;

			//DEBUGP(DEBUG_VERBOSE, "Descriptor #%d, Length=%d, Control=0x%x, SA=0x%x, ND=0x%p",
				//descNum, SgList->Elements[descNum].Length, Control, 
				//(ULONG32)pDesc->Packet.S2C.SystemAddressPhys,
				//pDesc->Packet.pNextDescriptorVirt);

			// Remove the start of packet bit for next descriptor and zero the CardAddress.
			Control &= ~PACKET_DESC_S2C_CTRL_START_OF_PACKET;
			// Depending on the Mode, leave the CardAddress alone or Increment it.
			if (!(Control & PACKET_DESC_S2C_CTRL_FIFO_OVERRIDE))
			{	
				CardAddress = 0;
			}
			// update pointers
			pLastDesc = pDesc;
			pDesc = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;
			_InterlockedIncrement(&pDmaExt->NumberOfUsedDescriptors);
		}

		pDmaExt->TimeoutCount = CARD_WATCHDOG_INTERVAL;

		// setup the descriptor pointer
		if( pLastDesc != NULL)
			pDmaExt->pDmaEng->Packet.SoftwareDescriptorPtr = pLastDesc->Packet.S2C.NextDescriptorPhys;
		pDmaExt->pNextDescVirt = pDesc;
	}
	else
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
	}
	WdfSpinLockRelease(pDmaExt->HeadSpinLock);
	if (!NT_SUCCESS(status))
	{
		NTSTATUS	FinalStatus = status;
		// an error has occurred, reset this transaction
		WdfDmaTransactionDmaCompletedFinal(DmaTransaction, 0, &FinalStatus);
		// complete the transaction
		WdfRequestCompleteWithInformation(pDmaXfer->Request, status, 0);
		WdfObjectDelete(DmaTransaction);
		return FALSE;
	}

	return TRUE;
}


/*!***************************************************************************
*
* 	\brief PacketS2CDpc - This routine processes completed
* 	DMA Packet descriptors, dequeues requests and completes them
*	 
* 	\param pDpcCtx - Pointer to the driver context for this Dpc
* 
*   \return nothing
* 
*****************************************************************************/
VOID  PacketS2CDpc (
	IN WDFDPC Dpc)  
{
	PDPC_CTX						pDpcCtx;
	PDEVICE_EXTENSION				pDevExt;
	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;
	PDMA_DESCRIPTOR_STRUCT			pDesc;
	PHYSICAL_ADDRESS				pDescPhys;
	PDMA_XFER						pDmaXfer = NULL;
	WDFREQUEST 						Request;
	BOOLEAN 						transactionComplete;
	NTSTATUS						status = STATUS_SUCCESS;

	pDevExt = ThorDaqDrvGetDeviceContext(WdfDpcGetParentObject(Dpc));
	pDpcCtx = DPCContext(Dpc);
	pDmaExt = pDpcCtx->pDmaExt;

	// We only want one thread processing recieves at a time.
	WdfSpinLockAcquire(pDmaExt->TailSpinLock);
	// Inc the DPC Count
	pDmaExt->DPCsInLastSecond++;

	// Acknowledge interrupts for this DMA engine
	pDmaExt->pDmaEng->ControlStatus = (COMMON_DMA_CTRL_IRQ_ACTIVE  | PACKET_DMA_CTRL_DMA_ENABLE);

	// Make sure we have completed descriptor(s)
	pDesc = pDmaExt->pTailDescVirt;
	while ((pDesc->Packet.S2C.StatusFlags_BytesCompleted & PACKET_DESC_S2C_STAT_COMPLETE) ||
		(pDesc->Packet.S2C.StatusFlags_BytesCompleted & PACKET_DESC_S2C_STAT_ERROR))
	{
		// At this point we know we have a completed Send DMA Descriptor
		// Update the contexts links to the next descriptor
		pDmaExt->pTailDescVirt = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;
		_InterlockedDecrement(&pDmaExt->NumberOfUsedDescriptors);

		// The Transaction data pointer is in every decriptor for a given Request
		pDmaXfer = DMAXferContext(pDesc->Packet.S2C.DmaTransaction);

		if (pDesc->Packet.S2C.ControlFlags_ByteCount & PACKET_DESC_S2C_CTRL_START_OF_PACKET)
		{
			pDmaXfer->bytesTransferred = 0;
		}
		pDmaXfer->bytesTransferred += (pDesc->Packet.S2C.StatusFlags_BytesCompleted & 
			PACKET_DESC_COMPLETE_BYTE_COUNT_MASK);
		pDmaXfer->PacketStatus |= (pDesc->Packet.S2C.StatusFlags_BytesCompleted & PACKET_DESC_S2C_STAT_ERROR);

		if (pDesc->Packet.S2C.ControlFlags_ByteCount & PACKET_DESC_S2C_CTRL_END_OF_PACKET)
		{
			if (pDmaXfer->PacketStatus)
			{
				// Stop the current DMA transfer, tell driver to not continue
				transactionComplete = WdfDmaTransactionDmaCompletedFinal (
					pDesc->Packet.S2C.DmaTransaction,
					pDmaXfer->bytesTransferred,
					&status);
				status = STATUS_ADAPTER_HARDWARE_ERROR;
			}
			else
			{
				status = STATUS_SUCCESS;
				// complete the transaction,  transaction completed successfully
				// tell the framework that this DMA set is complete
				transactionComplete = WdfDmaTransactionDmaCompletedWithLength(
					pDesc->Packet.S2C.DmaTransaction,
					pDmaXfer->bytesTransferred,
					&status );
			}

			// Is the full transaction complete?
			if (transactionComplete)
			{

				// Retrieve the originating request from the Transaction data extension
				Request = FindRequestByRequest(pDmaExt, pDmaXfer->Request);
				if (Request != NULL) 
				{
					// complete the transaction
					WdfRequestCompleteWithInformation(Request, status, pDmaXfer->bytesTransferred);
				}
				// Release the Transaction record
				WdfDmaTransactionRelease (pDesc->Packet.S2C.DmaTransaction);
				pDmaXfer->Request = NULL;
				WdfObjectDelete(pDesc->Packet.S2C.DmaTransaction);
				pDmaXfer = NULL;
			}
		}
		// Indicate we processed this descriptor by clearing Complete and Error flags
		pDesc->Packet.S2C.StatusFlags_BytesCompleted &= 
			~(PACKET_DESC_S2C_STAT_COMPLETE | PACKET_DESC_S2C_STAT_ERROR);

		// Link to the next packets
		pDescPhys.LowPart = pDesc->Packet.S2C.NextDescriptorPhys;
		pDesc = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;	// Link to the next descriptor in the chain
	}
	WdfSpinLockRelease(pDmaExt->TailSpinLock);
	pDmaExt->pDmaEng->ControlStatus = (COMMON_DMA_CTRL_IRQ_ENABLE | PACKET_DMA_CTRL_DMA_ENABLE);

	return;
}


/**************************************************************************** 
* 
**  C2S Packet Mode routines
**
*****************************************************************************/ 

//NTSTATUS ReadRequestSubmit (
//	IN WDFREQUEST			Request,
//	IN PDEVICE_EXTENSION	pDevExt,
//	IN PPACKET_READ_STRUCT	pReadPacket
//	)
//{
//	NTSTATUS 				status = STATUS_SUCCESS;
//	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;
//
//	status = GetDMAEngineContext(pDevExt, pReadPacket->EngineNum, &pDmaExt);
//	if (!NT_SUCCESS(status)) 
//	{
//		DEBUGP(DEBUG_ERROR, "PacketRead DMA Engine number invalid 0x%x", status);
//		return status;
//	}
//
//	WdfSpinLockAcquire(pDmaExt->TailSpinLock);
//	status = WdfRequestForwardToIoQueue (Request, pDmaExt->TransactionQueue);
//	WdfSpinLockRelease(pDmaExt->TailSpinLock);
//	if (!NT_SUCCESS(status)) 
//	{
//		DEBUGP(DEBUG_ERROR, "WdfRequestForwardToIoQueue failed 0x%x", status);
//	}
//
//	return status;
//}

// Addressable Packet Mode functions

/*!***************************************************************************
*
* 	\brief PacketStartRead - This routine setups the Read request then calls
* 	WdfDmaTransactionExecute to start or queue the actual DMA request
* 
* 	\param Request - WDF I/O Request (PACKET_SEND_IOCTL)
* 	\param DevExt - WDF Driver context
* 	\param pReadPacket - Contents of the PACKET_CHANNEL_READ_IOCTL request
*	 
* 	\return NTSTATUS - STATUS_SUCCESS if it works, FALSE if error.
* 
*****************************************************************************/

NTSTATUS PacketStartRead (
	IN WDFREQUEST			Request,
	IN PDEVICE_EXTENSION	pDevExt,
	IN PPACKET_READ_STRUCT	pReadPacket
	)
{
	NTSTATUS 				status = STATUS_SUCCESS;
	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;
	WDFDMATRANSACTION  		DmaTransaction;
	PDMA_XFER				pDmaXfer;
	WDF_OBJECT_ATTRIBUTES	attributes;
	PVOID					BufferAddress = NULL;
	PMDL					pMdl;

	PREQUEST_CONTEXT		reqContext = NULL;

	status = GetDMAEngineContext(pDevExt, pReadPacket->EngineNum, &pDmaExt);
	if (!NT_SUCCESS(status)) 
	{
		//DEBUGP(DEBUG_ERROR, "PacketRead DMA Engine number invalid 0x%x", status);
		return status;
	}

#if defined(_AMD64_)
	BufferAddress = (PVOID)pReadPacket->BufferAddress;
#else // Assume 32 bit
	// This keeps the compiler happy when /W4 is used.
	BufferAddress = (PVOID)(ULONG32)pReadPacket->BufferAddress;
#endif // 32 vs. 64 bit

	reqContext = RequestContext(Request);
	pMdl = (PMDL)reqContext->pMdl;

	// Create a DMA Transaction object just for this transfer
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, DMA_XFER);
	status = WdfDmaTransactionCreate(pDmaExt->DmaEnabler,
		&attributes,
		&DmaTransaction);

	// if no errors kick off the DMA, first save a pointer to the request.
	if (NT_SUCCESS(status))
	{
		// Keep a pointer the Request and the accumulated byte count in the Transaction
		pDmaXfer = DMAXferContext(DmaTransaction);
		pDmaXfer->Request = Request;
		pDmaXfer->bytesTransferred = 0;
		pDmaXfer->CardAddress = pReadPacket->CardOffset;
		pDmaXfer->pMdl = pMdl;
		pDmaXfer->Mode = pReadPacket->ModeFlags;
		pDmaXfer->PacketStatus = 0;

		//DEBUGP(DEBUG_ALWAYS, " Calling WdfDmaTransactionInitialize, Length %d",
			//(ULONG) pReadPacket->Length);
		status = WdfDmaTransactionInitialize(DmaTransaction,
			(PFN_WDF_PROGRAM_DMA) PacketProgramC2SDmaCallback,
			pDmaExt->DmaDirection,
			pMdl,	
			BufferAddress,
			(size_t)pReadPacket->Length);
		if (NT_SUCCESS(status))
		{
			//DEBUGP(DEBUG_ALWAYS, " transaction initialized")
				// Put the request on a WDF maintained Queue in case it gets canceled before we complete it
				WdfSpinLockAcquire(pDmaExt->TailSpinLock);
			//DEBUGP(DEBUG_ALWAYS, "TH@ %s TAIL lock acquired.", __FUNCTION__);
			status = WdfRequestForwardToIoQueue (Request, pDmaExt->TransactionQueue);
			WdfSpinLockRelease(pDmaExt->TailSpinLock);
			//DEBUGP(DEBUG_ALWAYS, "TH@ %s TAIL lock RELEASed.", __FUNCTION__);
			//$#$#$##@%^#$%%^&*^*(*
			if (NT_SUCCESS(status))
			{
				//DEBUGP(DEBUG_ALWAYS, " Request fed to the queue")
					// start the DMA, via PacketProgramDmaCallback
					status = WdfDmaTransactionExecute(DmaTransaction, pDmaExt);
				if (!NT_SUCCESS(status))
				{
					//DEBUGP(DEBUG_ERROR, "WdfDmaTransactionExecute failed 0x%x", status);
					// Make sure we get the Request off the queue.
					MmUnlockPages(pDmaXfer->pMdl);
					IoFreeMdl(pDmaXfer->pMdl);
					WdfSpinLockAcquire(pDmaExt->TailSpinLock);
					//DEBUGP(DEBUG_ALWAYS, "TH@ %s TAIL lock acquired.", __FUNCTION__);
					FindRequestByRequest(pDmaExt, Request);
					WdfSpinLockRelease(pDmaExt->TailSpinLock);
					//DEBUGP(DEBUG_ALWAYS, "TH@ %s TAIL lock RELEASed.", __FUNCTION__);
					WdfObjectDelete(DmaTransaction);
				}

			}
			else
			{
				//DEBUGP(DEBUG_ERROR, "WdfRequestForwardToIoQueue failed 0x%x", status);
				WdfObjectDelete(DmaTransaction);
			}
		}
		else
		{
			MmUnlockPages(pDmaXfer->pMdl);
			IoFreeMdl(pDmaXfer->pMdl);
			//DEBUGP(DEBUG_ERROR, "WdfDmaTransactionInitialize failed 0x%x", status);
			WdfObjectDelete(DmaTransaction);
		}
	}
	else
	{
		MmUnlockPages(pMdl);
		IoFreeMdl(pMdl);
		//DEBUGP(DEBUG_ERROR, "WdfDmaTransactionCreate failed 0x%x", status);
	}
	return status;
}


/*!***************************************************************************
*
* 	\brief PacketProgramC2SDmaCallback - This routine performs the actual
*	programming of the Packet DMA engine and descriptors
* 
* 	\param DmaTransaction - WDF DMA Transaction handle
* 	\param Device - WDF DMA Device handle
* 	\param Context - WDF Context pointer (our DMA Engine pointer)
* 	\param Direction - WDF Direction of DAM Transfer
* 	\param SgList - Pointer to the WDF Scatter/Gather list
*	 
*   \return TRUE if it works, FALSE if error.
*
* 	NOTE: This function is called by WdfDmaTransactionExecute
*   NOTE: This function is called in the transfer sequence.
 */

BOOLEAN PacketProgramC2SDmaCallback (
	IN WDFDMATRANSACTION  	DmaTransaction,
	IN WDFDEVICE  			Device,
	IN WDFCONTEXT  			Context,
	IN WDF_DMA_DIRECTION  	Direction,
	IN PSCATTER_GATHER_LIST  SgList
	)
{
	NTSTATUS						status = STATUS_SUCCESSFUL;
	PDEVICE_EXTENSION				pDevExt;
	PDMA_ENGINE_DEVICE_EXTENSION 	pDmaExt;
	PDMA_DESCRIPTOR_STRUCT			pDesc;
	PDMA_DESCRIPTOR_STRUCT			pLastDesc = NULL;
	PDMA_XFER						pDmaXfer;
	ULONG64 						CardAddress;
	ULONG32							Control;
	ULONG32							numAvailDescriptors;
	ULONG32							descNum;

	UNREFERENCED_PARAMETER(Direction);

	//DEBUGP(DEBUG_ALWAYS, "--> PacketProgramDmaCallback, IRQL=%d", KeGetCurrentIrql());

	// Get Device Extensions
	pDevExt = ThorDaqDrvGetDeviceContext(Device);
	pDmaExt = (PDMA_ENGINE_DEVICE_EXTENSION) Context;
	pDmaXfer = DMAXferContext(DmaTransaction);
	CardAddress = pDmaXfer->CardAddress;

	WdfSpinLockAcquire(pDmaExt->HeadSpinLock);
	//DEBUGP(DEBUG_ALWAYS, "--> PacketProgramDmaCallback, head lock acquired.");
	// Determine number of available descriptors
	numAvailDescriptors = pDmaExt->NumberOfDescriptors - pDmaExt->NumberOfUsedDescriptors;
	//DEBUGP(DEBUG_ALWAYS, "th@%s pDmaExt->NumberOfDescriptors = %d.", __FUNCTION__, pDmaExt->NumberOfDescriptors);
	//DEBUGP(DEBUG_ALWAYS, "th@%s pDmaExt->NumberOfUsedDescriptors = %d.", __FUNCTION__, pDmaExt->NumberOfUsedDescriptors);
	//DEBUGP(DEBUG_ALWAYS, "th@%s numAvailDescriptors = %d.", __FUNCTION__, numAvailDescriptors);
	//DEBUGP(DEBUG_ALWAYS, "th@%s SgList->NumberOfElements = %d.", __FUNCTION__, SgList->NumberOfElements);
	// Setup descriptor control, Set for the first descriptor
	Control = PACKET_DESC_C2S_CTRL_START_OF_PACKET;
	//if (pDmaXfer->Mode & READ_WRITE_MODE_FLAG_FIFO)
	//{
	//	Control |= PACKET_DESC_S2C_CTRL_FIFO_OVERRIDE;
	//}

	if (numAvailDescriptors >= SgList->NumberOfElements)
	{
		// Lock the access to the head pointer, get the next pointer, calc the new head, store it back and release the lock
		pDesc = pDmaExt->pNextDescVirt;

		// setup each of the descriptors
		for (descNum = 0; descNum < SgList->NumberOfElements; descNum++)
		{
			// setup the descriptor
			pDesc->Packet.C2S.StatusFlags_BytesCompleted = SgList->Elements[descNum].Length & PACKET_DESC_BYTE_COUNT_MASK;
			pDesc->Packet.C2S.UserStatus = 0;
			pDesc->Packet.C2S.CardAddress  = (ULONG32)(CardAddress & 0xFFFFFFFF);
			if (descNum == (SgList->NumberOfElements - 1))
			{
				// End the processing here, Only interrupt on completion of the last DMA descriptor and
				// when the DMA is stopped short.
				Control |= PACKET_DESC_C2S_CTRL_END_OF_PACKET | 
					PACKET_DESC_C2S_CTRL_IRQ_ON_COMPLETE | 
					PACKET_DESC_C2S_CTRL_IRQ_ON_ERROR;
			}
			pDesc->Packet.C2S.ControlFlags_ByteCount = ((ULONG32)((CardAddress & 0xF00000000) >> 12)) |
				(SgList->Elements[descNum].Length & PACKET_DESC_BYTE_COUNT_MASK) |
				Control;
			pDesc->Packet.C2S.SystemAddressPhys = SgList->Elements[descNum].Address.QuadPart;
			pDesc->Packet.C2S.DmaTransaction = DmaTransaction;

			//DEBUGP(DEBUG_VERBOSE, "Descriptor #%d, Length=%d, SA=0x%x, ND=0x%p",
				//descNum, SgList->Elements[descNum].Length,
				//(ULONG32)pDesc->Packet.C2S.SystemAddressPhys,
				//pDesc->Packet.pNextDescriptorVirt);

			// Remove the start of packet bit for next descriptor and zero out CardAddress
			Control &= ~PACKET_DESC_S2C_CTRL_START_OF_PACKET;
			// Depending on the Mode, leave the CardAddress alone or Increment it.
			if (!(Control & PACKET_DESC_S2C_CTRL_FIFO_OVERRIDE))
			{	
				CardAddress = 0;
			}
			// update pointers
			pLastDesc = pDesc;
			pDesc = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;
			_InterlockedIncrement(&pDmaExt->NumberOfUsedDescriptors);
		}

		pDmaExt->TimeoutCount = CARD_WATCHDOG_INTERVAL;

		// setup the descriptor pointer
		if (pLastDesc != NULL)
		{
			pDmaExt->pDmaEng->Packet.SoftwareDescriptorPtr = pLastDesc->Packet.C2S.NextDescriptorPhys;
		}
		pDmaExt->pNextDescVirt = pDesc;
	}
	else
	{
		//DEBUGP(DEBUG_ERROR, "Too many desc, Available = %d, Required = %d",
			//numAvailDescriptors, SgList->NumberOfElements);
		status = STATUS_INSUFFICIENT_RESOURCES;
	}
	WdfSpinLockRelease(pDmaExt->HeadSpinLock);
	//DEBUGP(DEBUG_ALWAYS, "--> PacketProgramDmaCallback, head lock released.");
	if (!NT_SUCCESS(status))
	{
		NTSTATUS	FinalStatus = status;
		// an error has occurred, reset this transaction
		//DEBUGP(DEBUG_ERROR,  "DMAD PacketProgramC2SDmaCallback failed status 0x%x", status);
		if (pDmaXfer->pMdl != NULL)
		{
			MmUnlockPages(pDmaXfer->pMdl);
			IoFreeMdl(pDmaXfer->pMdl);
		}
		WdfDmaTransactionDmaCompletedFinal(DmaTransaction, 0, &FinalStatus);
		// complete the transaction
		WdfRequestCompleteWithInformation(pDmaXfer->Request, status, 0);
		WdfObjectDelete(DmaTransaction);
		return FALSE;
	}

	return TRUE;
}


/*!***************************************************************************
*
* 	\brief PacketC2SDpc - This routine processes completed
* 	DMA Packet descriptors, dequeues requests and completes them
*	 
* 	\param pDpcCtx - Pointer to the driver context for this Dpc
* 
*   \return nothing
* 
*****************************************************************************/
VOID  PacketC2SDpc (
	IN WDFDPC Dpc)  
{
	PDPC_CTX				pDpcCtx;
	PDEVICE_EXTENSION		pDevExt;
	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;

	pDevExt = ThorDaqDrvGetDeviceContext(WdfDpcGetParentObject(Dpc));
	pDpcCtx = DPCContext(Dpc);
	pDmaExt = pDpcCtx->pDmaExt;

	// Acknowledge interrupts for this DMA engine
	pDmaExt->pDmaEng->ControlStatus = (COMMON_DMA_CTRL_IRQ_ACTIVE  | PACKET_DMA_CTRL_DMA_ENABLE);

	// Inc the DPC Count
	pDmaExt->DPCsInLastSecond++;

	if (pDmaExt->PacketMode == PACKET_MODE_FIFO)
	{
		// We only want one thread processing recieves at a time.
		WdfSpinLockAcquire(pDmaExt->HeadSpinLock);
		PacketProcessCompletedReceives(pDevExt, pDmaExt);
		WdfSpinLockRelease(pDmaExt->HeadSpinLock);
	}
	else if (pDmaExt->PacketMode == PACKET_MODE_ADDRESSABLE)
	{
		PacketReadComplete(pDevExt, pDmaExt);
	}
	else if (pDmaExt->PacketMode == PACKET_MODE_STREAMING)
	{
		pDmaExt->DMAEngineStatus |= DMA_OVERRUN_ERROR;
	}

	pDmaExt->pDmaEng->ControlStatus = (COMMON_DMA_CTRL_IRQ_ENABLE | PACKET_DMA_CTRL_DMA_ENABLE);
}

// FIFO Packet Mode functions

/*!***************************************************************************
*
* 	\brief PacketProcessCompletedReceives - This routine processes
*	completed DMA Packet descriptors, dequeues requests and completes them
*	 
* 	\param pDevExt - Pointer to the driver context for this adapter
*   \param pDmaExt - Pointer to the DMA Engine Context
* 
* 	\return STATUS_SUCCESS if it works, an error if it fails.
* 
* 	NOTE: This routine must be called while protected by a spinlock
* 
*****************************************************************************/
NTSTATUS PacketProcessCompletedReceives(
	IN PDEVICE_EXTENSION	pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt
	)
{
	PDMA_DESCRIPTOR_STRUCT			pDesc;
	PPACKET_RET_RECEIVE_STRUCT		pRecvPacketRet;
	WDFREQUEST						Request;
	ULONG32							CachedDescStatus = 0;
	size_t							bufferSize;		
	NTSTATUS						status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(pDevExt);

	//  Loop forever until we run out of PACKET_RECV_IOCTL requests or Completed DMA Descriptors
	// Make sure there is a Request waiting, if not just exit out.
	while(!WDF_IO_QUEUE_IDLE(WdfIoQueueGetState(pDmaExt->TransactionQueue, NULL, NULL)))
	{
		// Stage 1: Make sure we have a completed DMA PAcket, i.e. both SOP and EOP completed descriptor(s)
		pDesc = pDmaExt->pNextDescVirt;

		if (pDesc->Packet.C2S.StatusFlags_BytesCompleted & PACKET_DESC_C2S_STAT_START_OF_PACKET)
		{
			// Walk the descriptors looking for the EOP descriptor. It could be this descriptor
			do {
				// Save a copy of the status flags for later loop while test
				CachedDescStatus = pDesc->Packet.C2S.StatusFlags_BytesCompleted;
				if (CachedDescStatus & (PACKET_DESC_C2S_STAT_COMPLETE | PACKET_DESC_C2S_STAT_ERROR))
				{
					if (pDesc->Packet.DescFlags == DESC_FLAGS_HW_OWNED)
					{
						// Link to the next descriptor
						pDesc = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;	
					}
					else // This is an ERROR! It means we overran the queue
					{
						return STATUS_DRIVER_INTERNAL_ERROR;
					}
				}
				else // This Packet is not complete, exit out.
				{
					return STATUS_UNSUCCESSFUL;
				}
			} while (!(CachedDescStatus & PACKET_DESC_C2S_STAT_END_OF_PACKET));
			// If we exit the while loop then we have a completed packet
		}
		else
		{
			// This is not necessarily an error. We could be looping past a completed packet to the next 
			// DMA Descriptor that has not started to be DMA'd yet.
			return 0;
		}

		// Stage 2: We have a completed packet. Now get the Request in the queue.
		pDesc = pDmaExt->pNextDescVirt;
		// At this point we know we have an outstanding RECV request and a completed Packet
		status = WdfIoQueueRetrieveNextRequest(pDmaExt->TransactionQueue, &Request);
		if (NT_SUCCESS(status)) 
		{
			status = WdfRequestRetrieveOutputBuffer(Request,
				sizeof(PACKET_RET_RECEIVE_STRUCT),
				(PVOID *)(&pRecvPacketRet),
				&bufferSize);
			if (NT_SUCCESS(status)) 
			{
				// Zero out the return length, address and set Token to -1
				pRecvPacketRet->Length = 0;
				pRecvPacketRet->Address = 0;
				pRecvPacketRet->RxToken = (ULONG32)-1;
				pRecvPacketRet->UserStatus = 0;
				// Walk the descriptors again looking for the EOP descriptor. It could be this descriptor
				do {
					// Save a copy of the status flags for later loop while test
					CachedDescStatus = pDesc->Packet.C2S.StatusFlags_BytesCompleted;

					// Get the bytes transfered before we clear this field below
					pRecvPacketRet->Length += (CachedDescStatus & PACKET_DESC_COMPLETE_BYTE_COUNT_MASK);
					// The descriptor is now "owned" by software and will not get it back until
					// the application does another PACKET_RECEIVE_IOCTL with this "token"
					// Indicate we processed this descriptor by clearing all but the SOP and EOP flags
					pDesc->Packet.C2S.StatusFlags_BytesCompleted &= 
						(PACKET_DESC_C2S_STAT_START_OF_PACKET | PACKET_DESC_C2S_STAT_END_OF_PACKET);
					pDesc->Packet.DescFlags = DESC_FLAGS_SW_OWNED;

					if (CachedDescStatus & PACKET_DESC_C2S_STAT_START_OF_PACKET)
					{
						pRecvPacketRet->RxToken = pDesc->Packet.DescriptorNumber;
						pRecvPacketRet->Address = (ULONG64)pDesc->Packet.C2S.SystemAddressVirt;

						// Make sure we flush the processor(s) caches for this memory
						KeFlushIoBuffers(pDmaExt->PMdl, TRUE, TRUE);
					}
					if (CachedDescStatus & PACKET_DESC_C2S_STAT_ERROR)
					{
						// Indicate a bad packet by zeroing out the address and Length fields
						pRecvPacketRet->Address = 0;
						pRecvPacketRet->Length = 0;
						pDesc->Packet.DescFlags = DESC_FLAGS_SW_FREED;
					}

					if (CachedDescStatus & PACKET_DESC_C2S_STAT_END_OF_PACKET)
					{
						// Return the EOP UserStatus to the application
						pRecvPacketRet->UserStatus = pDesc->Packet.C2S.UserStatus;
						// Make sure the return token is valid
						if (pRecvPacketRet->RxToken == -1)
						{
						}
						if (pRecvPacketRet->Address != 0)
						{
							// complete the transaction
							WdfRequestCompleteWithInformation( Request, STATUS_SUCCESS, sizeof(PACKET_RET_RECEIVE_STRUCT));
							pDmaExt->TimeoutCount = CARD_WATCHDOG_INTERVAL;
						}
						else
						{
							// We have a NULL address, make sure we flag this with an error.
							WdfRequestCompleteWithInformation( Request, STATUS_DRIVER_INTERNAL_ERROR, sizeof(PACKET_RET_RECEIVE_STRUCT));
						}
					}
					// Link to the next descriptor in the chain
					pDesc = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;	
				} while (!(CachedDescStatus & PACKET_DESC_C2S_STAT_END_OF_PACKET));
				// We consider this descriptor processed at this point, link to the next descriptor in the chain
				pDmaExt->pNextDescVirt = pDesc;	
			}
			else // WdfRequestRetrieveOutputBuffer(Request... failed
			{
				WdfRequestCompleteWithInformation( Request, STATUS_INVALID_PARAMETER, sizeof(PACKET_RET_RECEIVE_STRUCT));
				// Status already set to Not SUCCESS, go around again to see if there is 
				// another Request pending.
			}
		}
		else  // WdfIoQueueRetrieveNextRequest(pDmaExt->RxPendingQueue...  failed
		{
			// Status already set to Not SUCCESS, exit the while loop
			break;
		}
	}
	return status;
}

/*!***************************************************************************
*
* 	\brief PacketProcessReturnedDescriptors - This routine processes the returned
*	Packet descriptors and updates the pointer if appropriate
*	 
* 	\param pDevExt - Pointer to the driver context for this adapter
*   \param pDmaExt - Pointer to the DMA Engine Context
*   \param ReturnToken - Token (Index) for the starting DMA Descriptor to be 
*					recycled back to the DMA Engine to be re-used
* 
*   \return STATUS_SUCCESS if it works, an error if it fails.
* 
*****************************************************************************/
NTSTATUS PacketProcessReturnedDescriptors(
	IN PDEVICE_EXTENSION	pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt,
	IN ULONG32				ReturnToken
	)
{
	PDMA_DESCRIPTOR_STRUCT			pDesc;
	PDMA_DESCRIPTOR_STRUCT			pDescPrev;
	ULONG32							DescriptorState = DESC_FLAGS_HW_OWNED;
	ULONG32							CachedDescStatus = 0;
	NTSTATUS						status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(pDevExt);

	// We only want one thread processing descriptors at a time.
	WdfSpinLockAcquire(pDmaExt->TailSpinLock);
	// Stage 1: Determine if this token is at the tail or will create a 'hole' in the list
	// Get the descriptor that is at the tail of the queue
	pDesc = (PDMA_DESCRIPTOR_STRUCT)pDmaExt->pTailDescVirt->Packet.pNextDescriptorVirt;
	if (pDesc->Packet.DescFlags != DESC_FLAGS_SW_OWNED)
	{
		WdfSpinLockRelease(pDmaExt->TailSpinLock);
		return STATUS_INVALID_PARAMETER;
	}
	if ((pDesc->Packet.C2S.StatusFlags_BytesCompleted & PACKET_DESC_C2S_STAT_START_OF_PACKET) != PACKET_DESC_C2S_STAT_START_OF_PACKET)
	{
		WdfSpinLockRelease(pDmaExt->TailSpinLock);
		return STATUS_INVALID_PARAMETER;
	}

	pDescPrev = pDesc;

	// See if the Returned decscriptor (token) is next in line.
	if (pDesc->Packet.DescriptorNumber != ReturnToken)
	{
		// In this case we are not at the tail, hence we just "free" the descriptor
		// Get the Descriptor at the Token Index into the descriptor array
		pDesc = (PDMA_DESCRIPTOR_STRUCT)&pDmaExt->pDescriptorBase[ReturnToken];
		// Make sure the Token matches the Desciptor
		if (pDesc->Packet.DescriptorNumber == ReturnToken)
		{
			if ((pDesc->Packet.C2S.StatusFlags_BytesCompleted & PACKET_DESC_C2S_STAT_START_OF_PACKET) != PACKET_DESC_C2S_STAT_START_OF_PACKET)
			{
				WdfSpinLockRelease(pDmaExt->TailSpinLock);
				return STATUS_INVALID_PARAMETER;
			}
			DescriptorState = DESC_FLAGS_SW_FREED;
		}
		else
		{
			// Log error and exit
			WdfSpinLockRelease(pDmaExt->TailSpinLock);
			return STATUS_INVALID_PARAMETER;
		}
	}

	// Stage 2: Walk the list starting at the token and either Free or mark for HW the descriptor(s)
	// Make sure the token is the start of the packet. If not it is either an app error or something worse
	if (pDesc->Packet.C2S.StatusFlags_BytesCompleted & PACKET_DESC_C2S_STAT_START_OF_PACKET)
	{
		CachedDescStatus = pDesc->Packet.C2S.StatusFlags_BytesCompleted;
		// Walk the descriptors looking for the EOP descriptor. It could be this descriptor
		do {
			if (pDesc->Packet.DescFlags != DESC_FLAGS_HW_OWNED)
			{
				// We can change ownership back to hardware since we sill be setting the SwDescPtr
				pDesc->Packet.DescFlags = DescriptorState;
				// Save a copy of the status flags for later loop while test
				CachedDescStatus = pDesc->Packet.C2S.StatusFlags_BytesCompleted;
				// Clear the status just in case
				pDesc->Packet.C2S.StatusFlags_BytesCompleted = 0;
				// Cache the current Descriptor
				pDescPrev = pDesc;
				// Link to the next descriptor
				pDesc = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;	
			}
			else
			{
				WdfSpinLockRelease(pDmaExt->TailSpinLock);
				return STATUS_INVALID_PARAMETER;
			}
		} while (!(CachedDescStatus & PACKET_DESC_C2S_STAT_END_OF_PACKET));
	}
	else
	{
		WdfSpinLockRelease(pDmaExt->TailSpinLock);
		return STATUS_DRIVER_INTERNAL_ERROR;
	}

	// Stage 3: If not a 'freed hole' then advance the list, look for freed descriptors ahead first
	if (DescriptorState == DESC_FLAGS_HW_OWNED)
	{
		// pDesc is pointing to the descriptor following the EOP
		// Now see if there are any previously 'freed' descriptors ahead of us.
		while (pDesc->Packet.DescFlags == DESC_FLAGS_SW_FREED)
		{
			// And mark it as HW Owned
			pDesc->Packet.DescFlags = DESC_FLAGS_HW_OWNED;
			// Cache the current Descriptor
			pDescPrev = pDesc;
			// If so Link to that descriptor
			pDesc = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;	
		}
		// Set the last descriptor as the new end and update the Tail pointer
		pDmaExt->pDmaEng->Packet.SoftwareDescriptorPtr = pDescPrev->Packet.pDescPhys.LowPart;
		pDmaExt->pTailDescVirt  = pDescPrev;
	}
	WdfSpinLockRelease(pDmaExt->TailSpinLock);
	return status;
}

// Addressable Packet Mode functions

/*!***************************************************************************
*
* 	\brief PacketReadComplete - This routine processes completed
* 	DMA Packet descriptors, dequeues requests and completes them
*	 
* 	\param pDpcCtx - Pointer to the driver context for this Dpc
* 
*   \return nothing
* 
*****************************************************************************/
NTSTATUS PacketReadComplete (
	IN PDEVICE_EXTENSION	pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt
	)
{
	PDMA_DESCRIPTOR_STRUCT			pDesc;
	PHYSICAL_ADDRESS				pDescPhys;
	PDMA_XFER						pDmaXfer = NULL;
	WDFREQUEST 						Request;
	PPACKET_RET_READ_STRUCT			pReadRetPacket;
	size_t							ReadRetPacketSize;		
	BOOLEAN 						transactionComplete;
	NTSTATUS						status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(pDevExt);
	// We only want one thread processing recieves at a time.
	WdfSpinLockAcquire(pDmaExt->TailSpinLock);
	// Make sure we have completed descriptor(s)
	pDesc = pDmaExt->pTailDescVirt;
	while ((pDesc->Packet.C2S.StatusFlags_BytesCompleted & PACKET_DESC_C2S_STAT_COMPLETE) ||
		(pDesc->Packet.C2S.StatusFlags_BytesCompleted & PACKET_DESC_C2S_STAT_ERROR))
	{
		// At this point we know we have a completed Send DMA Descriptor
		// Update the contexts links to the next descriptor
		pDmaExt->pTailDescVirt = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;

		if (pDesc->Packet.C2S.DmaTransaction != NULL)
		{
			//DEBUGP(DEBUG_ALWAYS, "PacketReadComplete DmaTransaction is not NULL");
			_InterlockedDecrement(&pDmaExt->NumberOfUsedDescriptors);

			// The Transaction data pointer is in every decriptor for a given Request
			pDmaXfer = DMAXferContext(pDesc->Packet.C2S.DmaTransaction);

			if (pDesc->Packet.C2S.StatusFlags_BytesCompleted & PACKET_DESC_C2S_STAT_START_OF_PACKET)
			{				
				pDmaXfer->bytesTransferred = 0;
			}

			pDmaXfer->bytesTransferred += (pDesc->Packet.C2S.StatusFlags_BytesCompleted & 
				PACKET_DESC_COMPLETE_BYTE_COUNT_MASK);

			pDmaXfer->PacketStatus |= (pDesc->Packet.C2S.StatusFlags_BytesCompleted & PACKET_DESC_S2C_STAT_ERROR);

			if (pDesc->Packet.C2S.StatusFlags_BytesCompleted & PACKET_DESC_C2S_STAT_END_OF_PACKET)
			{

				pDmaXfer->UserControl = pDesc->Packet.C2S.UserStatus;

				if (pDmaXfer->PacketStatus)
				{
					// Stop the current DMA transfer, tell driver to not continue
					transactionComplete = WdfDmaTransactionDmaCompletedFinal(
						pDesc->Packet.C2S.DmaTransaction,
						pDmaXfer->bytesTransferred,
						&status);
					status = STATUS_ADAPTER_HARDWARE_ERROR;
				}
				else
				{
					status = STATUS_SUCCESS;
					// complete the transaction,  transaction completed successfully
					// tell the framework that this DMA set is complete
					transactionComplete = WdfDmaTransactionDmaCompletedWithLength(
						pDesc->Packet.C2S.DmaTransaction,
						pDmaXfer->bytesTransferred,
						&status );
				}

				// Is the full transaction complete?
				if (transactionComplete)
				{

					// Retrieve the originating request from the Transaction data extension
					Request = FindRequestByRequest(pDmaExt, pDmaXfer->Request);
					if (Request != NULL) 
					{
						// get the output buffer pointer
						status = WdfRequestRetrieveOutputBuffer(Request,
							(size_t) sizeof(PPACKET_RET_READ_STRUCT),	// Min size
							(PVOID*) &pReadRetPacket,
							&ReadRetPacketSize);
						if (NT_SUCCESS(status))
						{
							if (pReadRetPacket != NULL)
							{
								if (ReadRetPacketSize >= sizeof(PPACKET_RET_READ_STRUCT))
								{
									pReadRetPacket->Length = (ULONG32)pDmaXfer->bytesTransferred;
									pReadRetPacket->UserStatus = pDmaXfer->UserControl;
								}
							}
						}
						// complete the transaction
						WdfRequestCompleteWithInformation(Request, status, ReadRetPacketSize);
					}

					if (pDmaXfer->pMdl != NULL)
					{
						MmUnlockPages(pDmaXfer->pMdl);
						IoFreeMdl(pDmaXfer->pMdl);
					}

					// Release the Transaction record
					WdfDmaTransactionRelease (pDesc->Packet.C2S.DmaTransaction);

					pDmaXfer->Request = NULL;
					WdfObjectDelete(pDesc->Packet.C2S.DmaTransaction);
					pDmaXfer = NULL;
				}
			}
		}

		// Indicate we processed this descriptor by clearing Complete and Error flags
		pDesc->Packet.C2S.DmaTransaction = NULL;

		pDesc->Packet.C2S.StatusFlags_BytesCompleted  = 0; 
		//DEBUGP(DEBUG_ALWAYS, "I am here 1 ");
		//DEBUGP(DEBUG_ALWAYS, "I am here pDesc->Packet.C2S.NextDescriptorPhys = 0x%x.", pDesc->Packet.C2S.NextDescriptorPhys);

		// Link to the next packets
		pDescPhys.LowPart = pDesc->Packet.C2S.NextDescriptorPhys;
		//DEBUGP(DEBUG_ALWAYS, "I am here pDescPhys.LowPart = 0x%x.", pDescPhys.LowPart);	
		//DEBUGP(DEBUG_ALWAYS, "I am here 2 ");
		//DEBUGP(DEBUG_ALWAYS, "I am here pDesc->Packet.pNextDescriptorVirt = 0x%x.", pDesc->Packet.pNextDescriptorVirt);
		pDesc = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;	// Link to the next descriptor in the chain
		//DEBUGP(DEBUG_ALWAYS, "I am here pDesc = 0x%x.", pDesc);

		//DEBUGP(DEBUG_ALWAYS, "I am here pDesc->Packet.C2S.StatusFlags_BytesCompleted = 0x%x.", pDesc->Packet.C2S.StatusFlags_BytesCompleted);
	}
	WdfSpinLockRelease(pDmaExt->TailSpinLock);
	return STATUS_SUCCESS;
}

/*!***************************************************************************
*
* 	\brief PacketReadRequestCancel - Cancels a waiting Packet Read request.
* 
*   \param Request - WDF Request
* 
*   \return nothing
* 
*****************************************************************************/
VOID PacketReadRequestCancel(
	IN WDFQUEUE		Queue,
	IN WDFREQUEST	Request)
{
	PQUEUE_CTX				pQueueCtx = QueueContext(Queue);
	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt = pQueueCtx->pDmaExt;
	WDF_REQUEST_PARAMETERS	Params;
	PDMA_DESCRIPTOR_STRUCT	pDesc;
	PDMA_XFER				pDmaXfer;
	WDFREQUEST				CancelRequest;
	NTSTATUS				status;

	// Hold both Head and Tail spin locks to stop DPCs and ioctls from touching anyhting
	WdfSpinLockAcquire(pDmaExt->HeadSpinLock);
	WdfSpinLockAcquire(pDmaExt->TailSpinLock);
	// Shut down the DMA Engine.
	pDmaExt->pDmaEng->ControlStatus = 0;

	WDF_REQUEST_PARAMETERS_INIT(&Params);
	WdfRequestGetParameters(Request, &Params);

	if (Params.Parameters.DeviceIoControl.IoControlCode == PACKET_CHANNEL_READ_IOCTL)
	{
		pDesc = pDmaExt->pTailDescVirt;
		while (pDesc != pDmaExt->pNextDescVirt)
		{
			// Update the contexts links to the next descriptor
			pDmaExt->pTailDescVirt = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;
			if (pDesc->Packet.C2S.DmaTransaction != NULL)
			{
				// The Transaction data pointer is in every decriptor for a given Request
				pDmaXfer = DMAXferContext(pDesc->Packet.C2S.DmaTransaction);
				if (pDmaXfer != NULL)
				{
					if (pDesc->Packet.C2S.ControlFlags_ByteCount & PACKET_DESC_C2S_CTRL_END_OF_PACKET)
					{
						status = STATUS_CANCELLED;
						WdfDmaTransactionDmaCompletedFinal(	pDesc->Packet.C2S.DmaTransaction, 0, &status);

						if (Request != pDmaXfer->Request)
						{
							CancelRequest = FindRequestByRequest(pDmaExt, pDmaXfer->Request);
							if (CancelRequest != NULL) 
							{
								// complete the transaction
								WdfRequestCompleteWithInformation(CancelRequest, STATUS_CANCELLED, 0);
							}
						}
						if (pDmaXfer->pMdl != NULL)
						{
							MmUnlockPages(pDmaXfer->pMdl);
							IoFreeMdl(pDmaXfer->pMdl);
							pDmaXfer->pMdl = NULL;
						}
						// Release the Transaction record
						pDmaXfer->Request = NULL;
						WdfDmaTransactionRelease (pDesc->Packet.C2S.DmaTransaction);
						WdfObjectDelete(pDesc->Packet.C2S.DmaTransaction);
						pDmaXfer = NULL;
					}
				}
			} // if (pDesc->Packet.C2S.DmaTransaction...

			_InterlockedDecrement(&pDmaExt->NumberOfUsedDescriptors);
			// Clear the byte count, retain only the SOP and EOP
			pDesc->Packet.C2S.ControlFlags_ByteCount  = 0; //&= (PACKET_DESC_C2S_CTRL_START_OF_PACKET | PACKET_DESC_C2S_CTRL_END_OF_PACKET);
			pDesc->Packet.C2S.StatusFlags_BytesCompleted = PACKET_DESC_C2S_STAT_ERROR;
			pDesc->Packet.C2S.DmaTransaction = NULL;
			pDesc->Packet.C2S.pScatterGatherList = 0;

			pDesc = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;	// Link to the next descriptor in the chain
		} // while (pDesc != pDmaExt->pNextDescVirt)
	}
	FindRequestByRequest(pDmaExt, Request);
	// complete the transaction
	WdfRequestCompleteWithInformation(Request, STATUS_CANCELLED, 0);

	WdfSpinLockRelease(pDmaExt->TailSpinLock);
	WdfSpinLockRelease(pDmaExt->HeadSpinLock);
	return;
}


// Free Run FIFO Packet Mode functions


/*!***************************************************************************
*
* 	\brief PacketProcessCompletedFreeRunDescriptors - This routine processes
*	completed DMA Packet descriptors, dequeues requests and completes them
*	 
* 	\param pDevExt - Pointer to the driver context for this adapter
*   \param pDmaExt - Pointer to the DMA Engine Context
* 
* 	\return STATUS_SUCCESS if it works, an error if it fails.
* 
* 	NOTE: This routine must be called while protected by a spinlock
* 
*****************************************************************************/
NTSTATUS PacketProcessCompletedFreeRunDescriptors(
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt,
	PPACKET_RECVS_STRUCT			pPacketRecvs
	)
{
	PDMA_DESCRIPTOR_STRUCT			pDesc;
	ULONG32							CachedDescStatus = 0;
	NTSTATUS						status = STATUS_SUCCESS;

	pPacketRecvs->RetNumEntries = 0;
	pPacketRecvs->EngineStatus = pDmaExt->DMAEngineStatus;
	pDmaExt->DMAEngineStatus = 0;
	// Loop to file out the packet receives.
	while (pPacketRecvs->RetNumEntries < pPacketRecvs->AvailNumEntries)
	{
		// Zero out the return length, address, etc
		pPacketRecvs->Packets[pPacketRecvs->RetNumEntries].Length = 0;
		pPacketRecvs->Packets[pPacketRecvs->RetNumEntries].Address = 0;
		pPacketRecvs->Packets[pPacketRecvs->RetNumEntries].Status = PACKET_ERROR_MALFORMED;
		pPacketRecvs->Packets[pPacketRecvs->RetNumEntries].UserStatus = 0;

		pDesc = pDmaExt->pNextDescVirt;
		// The whole point of this loop is to make sure we have a completed packet
		do {
			CachedDescStatus = pDesc->Packet.C2S.StatusFlags_BytesCompleted;
			if (CachedDescStatus & (PACKET_DESC_C2S_STAT_COMPLETE | PACKET_DESC_C2S_STAT_ERROR))
			{
				pDesc = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;	
			}
			else
			{
				return status;
			}
		} while (!(CachedDescStatus & PACKET_DESC_C2S_STAT_END_OF_PACKET));
		// If we exit the while loop then we have a completed packet

		pDesc = pDmaExt->pNextDescVirt;
		// Walk the descriptors again retrieving length, addressm etc. and looking for 
		//   the EOP descriptor, it could be this one.
		do {
			CachedDescStatus = pDesc->Packet.C2S.StatusFlags_BytesCompleted;
			// Make sure we have a completed DMA Descriptor.
			if (CachedDescStatus & (PACKET_DESC_C2S_STAT_COMPLETE | PACKET_DESC_C2S_STAT_ERROR))
			{
				// Remove the interrupt bits from this descriptor.
				pDesc->Packet.C2S.ControlFlags_ByteCount &= ~(PACKET_DESC_S2C_CTRL_IRQ_ON_COMPLETE |
					PACKET_DESC_S2C_CTRL_IRQ_ON_ERROR);
				// Get the bytes transfered before we clear this field below
				pPacketRecvs->Packets[pPacketRecvs->RetNumEntries].Length += (CachedDescStatus & PACKET_DESC_COMPLETE_BYTE_COUNT_MASK);
				pDesc->Packet.C2S.StatusFlags_BytesCompleted = 0;
				if (CachedDescStatus & PACKET_DESC_C2S_STAT_START_OF_PACKET)
				{
					pPacketRecvs->Packets[pPacketRecvs->RetNumEntries].Address = (ULONG64)pDesc->Packet.C2S.SystemAddressVirt;
					// Make sure we flush the processor(s) caches for this memory
					KeFlushIoBuffers(pDmaExt->PMdl, TRUE, TRUE);
				}
				if (CachedDescStatus & PACKET_DESC_C2S_STAT_ERROR)
				{
					pPacketRecvs->Packets[pPacketRecvs->RetNumEntries].Status = CachedDescStatus & 
						(PACKET_DESC_C2S_STAT_ERROR || PACKET_DESC_C2S_STAT_SHORT);
				}
				if (CachedDescStatus & PACKET_DESC_C2S_STAT_END_OF_PACKET)
				{
					// Return the EOP UserStatus to the application
					pPacketRecvs->Packets[pPacketRecvs->RetNumEntries].UserStatus = pDesc->Packet.C2S.UserStatus;
					// Since we found the EOP remove the Malformed packet indicator.
					pPacketRecvs->Packets[pPacketRecvs->RetNumEntries].Status &= ~PACKET_ERROR_MALFORMED;
					pPacketRecvs->RetNumEntries++;
				}
				// Link to the next descriptor
				pDesc = (PDMA_DESCRIPTOR_STRUCT)pDesc->Packet.pNextDescriptorVirt;	
			}
			else // This descriptor is not complete and it should be, exit out.
			{
				return STATUS_DRIVER_INTERNAL_ERROR;
			}
		} while (!(CachedDescStatus & PACKET_DESC_C2S_STAT_END_OF_PACKET));
		// We consider this Packet processed at this point, link to the next descriptor in the chain
		pDmaExt->pNextDescVirt = pDesc;	
		// Set the interrupt bits for this descriptor to check for an overrun condition
		pDesc->Packet.C2S.ControlFlags_ByteCount |= (PACKET_DESC_S2C_CTRL_IRQ_ON_COMPLETE |
			PACKET_DESC_S2C_CTRL_IRQ_ON_ERROR);
	} // while more packets available to return in the struct...
	return status;
}


/*!***************************************************************************
*
* 	\brief S2mmInterruptDpc - This routine start data transaction of C2S 
*								if the read request pending.
* 	\param pDpcCtx - Pointer to the driver context for this Dpc
* 
*   \return nothing
* 
*****************************************************************************/
VOID  S2mmInterruptDpc (
	IN WDFDPC Dpc)  
{
	//NTSTATUS  status;
	//WDFREQUEST  request;
	//PREQUEST_CONTEXT		reqContext = NULL;
	PDEVICE_EXTENSION		pDevExt;
	PS2MM_LAYER_EXTENSION	pS2mmExt;
	//PS2MM_ENGINE_EXTENSION  pS2mmEngExt;
	//PDMA_ENGINE_DEVICE_EXTENSION pDmaExt;
	PS2MM_CTRL_STAT_STRUCT			pS2mmCtrl;

	pDevExt = ThorDaqDrvGetDeviceContext(WdfDpcGetParentObject(Dpc));
	pS2mmExt = pDevExt->pS2mmLayerExt;
	//pS2mmEngExt = &(pDevExt->pS2mmLayerExt->pS2mm);
	//pDmaExt = pDevExt->pDmaEngineDevExt[pS2mmExt->RequestServingDMAIndex[0]];
	pS2mmCtrl = &(pDevExt->pBar2Controls->ctrlCh[0].ctrl);

}
