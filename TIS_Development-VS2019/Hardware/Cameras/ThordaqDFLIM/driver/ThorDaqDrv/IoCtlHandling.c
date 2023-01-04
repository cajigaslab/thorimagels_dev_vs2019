#include "Driver.h"

#pragma warning(disable:4127)  // Constants in while loops. Sorry, I like them.

/*!***************************************************************************
*
* 	\brief ThorDaqDrvEvtIoDeviceControl - This is the main IOCtl dispatch for
* 	the driver. The job of this routine is to validate the request and in some
*	cases route the request to the approriate DMA queue or function
* 
*   \param Queue - WDF Managed Queue where request came from
*   \param Request - Pointer to the IOCtl request
*   \param InputBufferLength - Size of the IOCtl Input buffer
* 	\param OutputBufferLength - Size of the IOCtl Output buffer
*   \param IoControlCode - IOCTL parameter
* 
*   \return NTSTATUS
* 
* 	This routine is called at IRQL < DISPATCH_LEVEL.
*
*****************************************************************************/
VOID ThorDaqDrvEvtIoDeviceControl (
	IN WDFQUEUE  		Queue,
	IN WDFREQUEST		Request,
	IN size_t			OutputBufferLength,
	IN size_t			InputBufferLength,
	IN ULONG			IoControlCode)
{
	WDFDEVICE 			device = WdfIoQueueGetDevice(Queue);
	PDEVICE_EXTENSION	pDevExt = ThorDaqDrvGetDeviceContext(device);

	NTSTATUS 			status = STATUS_SUCCESS;
	size_t				bufferSize;
	size_t				infoSize = 0;
	BOOLEAN 			completeRequest = TRUE; // Default to complete the request here
	WDF_DMA_DIRECTION	dmaDirection = WdfDmaDirectionWriteToDevice;

	//UNREFERENCED_PARAMETER(OutputBufferLength);

	if (pDevExt == NULL)
	{
		WdfRequestCompleteWithInformation( Request, STATUS_NO_SUCH_DEVICE, 0);
		return;
	}

	switch (IoControlCode) 
	{
	case PACKET_RECEIVE_IOCTL:
		{
			PPACKET_RECEIVE_STRUCT	pRecvPacket;
			status = STATUS_INVALID_PARAMETER;
			if (InputBufferLength >= sizeof(PACKET_RECEIVE_STRUCT))
			{
				// Get the input buffer, that has all the info we need for the receive
				status = WdfRequestRetrieveInputBuffer(Request,
					sizeof(PACKET_RECEIVE_STRUCT),	/* size */
					(PVOID *) &pRecvPacket,			/* buffer */
					&bufferSize);
				if (status == STATUS_SUCCESS)
				{
					status = STATUS_INVALID_PARAMETER;
					if (bufferSize >= sizeof(PACKET_RECEIVE_STRUCT))
					{
						if (pRecvPacket != NULL)
						{
							if ((pRecvPacket->EngineNum < MAX_NUM_DMA_ENGINES) &&
								(pDevExt->pDmaEngineDevExt[pRecvPacket->EngineNum] != NULL))
							{
								PDMA_ENGINE_DEVICE_EXTENSION pDmaExt;
								status = GetDMAEngineContext(pDevExt, pRecvPacket->EngineNum, &pDmaExt);
								if (status == STATUS_SUCCESS)
								{
									status = STATUS_INVALID_DEVICE_REQUEST;
									if (pDmaExt->DmaType == DMA_TYPE_PACKET_RECV)
									{
										if (pDmaExt->PacketMode == PACKET_MODE_FIFO)
										{
											if (pRecvPacket->RxReleaseToken < (ULONG32)pDmaExt->NumberOfUsedDescriptors)
											{
												// Go do the return of a descriptor even if it is out of order
												status = PacketProcessReturnedDescriptors(pDevExt, pDmaExt, pRecvPacket->RxReleaseToken);
												if (status != STATUS_SUCCESS)
												{
													break;
												}
											}
											else
											{
												if (pRecvPacket->RxReleaseToken != -1)
												{
													// Bad Recieve Token
												}
											}
											status = STATUS_INVALID_PARAMETER; 
											// Check parameters
											if (OutputBufferLength >= (size_t) sizeof(PACKET_RET_RECEIVE_STRUCT))
											{
												if (pDmaExt->UserVa)
												{
													// Queue the request and wake up the Receive processing
													status = WdfRequestForwardToIoQueue (Request, pDmaExt->TransactionQueue);
													// We only want one thread processing recieves at a time.
													WdfSpinLockAcquire(pDmaExt->HeadSpinLock);
													PacketProcessCompletedReceives(pDevExt, pDmaExt);
													WdfSpinLockRelease(pDmaExt->HeadSpinLock);

													if (status == STATUS_SUCCESS)
													{
														completeRequest = FALSE;
													}
												}
											}
											else if (OutputBufferLength == 0)
											{
												// This is a special case where the call was to return a buffer token only
												status = STATUS_SUCCESS;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		break;
	case PACKET_BUF_READ_IOCTL:
		{
			PPACKET_READ_STRUCT			pReadPacket;

			status = STATUS_INVALID_PARAMETER;
			// Make sure the Input size is what we expect
			if (InputBufferLength >= sizeof(PACKET_READ_STRUCT))
			{
				// Get the input buffer, where we get the Application request structure
				status = WdfRequestRetrieveInputBuffer(Request,
					sizeof(PACKET_READ_STRUCT),		/* Min size */
					(PVOID *) &pReadPacket,				/* buffer */
					&bufferSize);
				if (NT_SUCCESS(STATUS_SUCCESS))
				{
					status = STATUS_INVALID_PARAMETER;
					// Make sure the size is what we expect
					if (bufferSize >= sizeof(PACKET_READ_STRUCT))
					{
						// Make sure it is a valid pointer
						if (pReadPacket != NULL)
						{	
							if (pDevExt->pDmaEngineDevExt[pReadPacket->EngineNum]->bAddressablePacketMode)
							{
								// Range check and make sure we have a DMA Engine where we are asking
								if ((pReadPacket->EngineNum < MAX_NUM_DMA_ENGINES) &&
									(pDevExt->pDmaEngineDevExt[pReadPacket->EngineNum] != NULL))
								{
									// Check output parameters
									if (OutputBufferLength >= sizeof(PACKET_RET_READ_STRUCT)) //(size_t) (pReadPacket->Length))
									{
										// Make sure we have a queue assigned
										status = STATUS_INVALID_DEVICE_REQUEST;
										if (pDevExt->pDmaEngineDevExt[pReadPacket->EngineNum]->DmaType == DMA_TYPE_PACKET_READ)
										{
											if (pDevExt->pDmaEngineDevExt[pReadPacket->EngineNum]->PacketMode == PACKET_MODE_ADDRESSABLE)
											{
												status = PacketStartRead(Request, pDevExt, pReadPacket);
												if (NT_SUCCESS(STATUS_SUCCESS))
												{
													completeRequest = FALSE;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		break;
	case PACKET_CHANNEL_READ_IOCTL:
		{
			PPACKET_READ_STRUCT			pReadPacket;
			PS2MM_ENGINE_EXTENSION		pS2mmEngExt;
			PPACKET_RET_READ_STRUCT		pReadRetPacket;
			size_t						ReadRetPacketSize;
			PMDL						pMdl;
		    PREQUEST_CONTEXT		    reqContext = NULL;
			BOOLEAN                     interruptClearFlag = TRUE;
			int                         i =0;
			//DEBUGP(DEBUG_ERROR,	"$$$$$$$$$$$$$$$$$$PACKET_CHANNEL_READ_IOCTL");

            status = STATUS_INVALID_PARAMETER;
			// Make sure the Input size is what we expect
			if (InputBufferLength >= sizeof(PACKET_READ_STRUCT))
			{
				// Get the input buffer, where we get the Application request structure
				status = WdfRequestRetrieveInputBuffer(Request,
					sizeof(PACKET_READ_STRUCT),		/* Min size */
					(PVOID *) &pReadPacket,				/* buffer */
					&bufferSize);
				if (status == STATUS_SUCCESS)
				{
					status = STATUS_INVALID_PARAMETER;
					// Make sure the size is what we expect
					if (bufferSize >= sizeof(PACKET_READ_STRUCT))
					{
						// Make sure it is a valid pointer
						if (pReadPacket != NULL)
						{	
							if (pDevExt->pDmaEngineDevExt[pReadPacket->EngineNum]->bAddressablePacketMode)
							{
								// Range check and make sure we have a DMA Engine where we are asking
								if ((pReadPacket->EngineNum < MAX_NUM_DMA_ENGINES) &&
									(pDevExt->pDmaEngineDevExt[pReadPacket->EngineNum] != NULL))
								{
									// Check output parameters
									if (OutputBufferLength >= sizeof(PACKET_RET_READ_STRUCT)) //(size_t) (pReadPacket->Length))
									{
										// Make sure we have a queue assigned
										status = STATUS_INVALID_DEVICE_REQUEST;
										if (pDevExt->pDmaEngineDevExt[pReadPacket->EngineNum]->DmaType == DMA_TYPE_PACKET_READ)
										{
											if (pDevExt->pDmaEngineDevExt[pReadPacket->EngineNum]->PacketMode == PACKET_MODE_ADDRESSABLE)
											{
												//DEBUGP(DEBUG_ALWAYS, "$$$$$$$$$$$$$$$$$$$$ pReadPacket->Channel = %d \n", pReadPacket->Channel);
												pS2mmEngExt = &(pDevExt->pS2mmLayerExt->pS2mm[pReadPacket->Channel]);
												if(pReadPacket->ModeFlags == READ_WRITE_MODE_FLAG_ADDRESSED)
												{
													if (pS2mmEngExt->IsDataReady == 1)
													{
														pReadPacket->CardOffset = pDevExt->DataBufferStartAddress + pReadPacket->Channel * pDevExt->DataChannelLength + pDevExt->pS2mmLayerExt->AcqBufOffset;
														status = PacketStartRead(Request, pDevExt, pReadPacket);

														if (NT_SUCCESS(STATUS_SUCCESS))
														{
															completeRequest = FALSE;
															pS2mmEngExt->IsDataReady = 0;
														}

														// make sure all channel buffer have been read out
														for (i = 0; i < S2MM_CHANNEL_PER_BOARD; i++)
														{
															if (pDevExt->ChannelDescriptor[i] == TRUE && pDevExt->pS2mmLayerExt->pS2mm[i].IsDataReady != 0)
															{
																interruptClearFlag = FALSE;
															}
														}

														// clear all interrupt when all the buffer have been read out
														if (interruptClearFlag == TRUE)
														{
															for (i = 0; i < S2MM_CHANNEL_PER_BOARD; i++)
															{
																if (pDevExt->ChannelDescriptor[i] == TRUE)
																{
																	pDevExt->pBar2Controls->ctrlCh[i].ctrl.SR0_CR0 = 0x53; // if channel is enabled, re-arm Descriptor-chain, clear bit
																}else
																{
																	pDevExt->pBar2Controls->ctrlCh[i].ctrl.SR0_CR0 = 0x51; // if channel is disabled, turn off the channel acq.
																}
															}
															pDevExt->pDmaRegisters->commonControl.ControlStatus |= 0x0000003f;
														}
													}else
													{
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
																	pReadRetPacket->Length = 0x0;
																	pReadRetPacket->UserStatus = 0xffffffff;
																}
															}
														}
														infoSize = ReadRetPacketSize;
														// complete the transaction			
														reqContext = RequestContext(Request);
														pMdl = (PMDL)reqContext->pMdl;
														WdfRequestCompleteWithInformation(Request, status, ReadRetPacketSize);

														//DEBUGP(DEBUG_ALWAYS, "$$$$$$$$$$$$$$$$$ no interrupt");
														
														if (pMdl != NULL)
														{
															MmUnlockPages(pMdl);
															IoFreeMdl(pMdl);
														}
														completeRequest = FALSE;
													}
												}
												else
												{
													pReadPacket->ModeFlags = READ_WRITE_MODE_FLAG_ADDRESSED;

													//pReadPacket->CardOffset = ...

													status = PacketStartRead(Request, pDevExt, pReadPacket);

													if (status == STATUS_SUCCESS)
													{
														completeRequest = FALSE;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		break;

	case PACKET_RECEIVES_IOCTL:
		{
			PPACKET_RECVS_STRUCT	pPacketRecvs;
			status = STATUS_INVALID_PARAMETER;
			if (OutputBufferLength >= sizeof(PACKET_RECVS_STRUCT))
			{
				// Get the Output buffer, that has all the info we need for the receives
				status = WdfRequestRetrieveOutputBuffer(Request,
					sizeof(PACKET_RECVS_STRUCT),	/* size */
					(PVOID *) &pPacketRecvs,		/* buffer */
					&bufferSize);
				if (status == STATUS_SUCCESS)
				{
					status = STATUS_INVALID_PARAMETER;
					if (bufferSize >= sizeof(PACKET_RECVS_STRUCT))
					{
						if (pPacketRecvs != NULL)
						{
							if ((pPacketRecvs->EngineNum < MAX_NUM_DMA_ENGINES) &&
								(pDevExt->pDmaEngineDevExt[pPacketRecvs->EngineNum] != NULL))
							{
								PDMA_ENGINE_DEVICE_EXTENSION pDmaExt;
								status = GetDMAEngineContext(pDevExt, pPacketRecvs->EngineNum, &pDmaExt);
								if (status == STATUS_SUCCESS)
								{
									status = STATUS_INVALID_DEVICE_REQUEST;
									if (pDmaExt->DmaType == DMA_TYPE_PACKET_RECV)
									{
										if (pDmaExt->PacketMode == PACKET_MODE_STREAMING)
										{
											status = STATUS_INVALID_PARAMETER; 
											if (pDmaExt->UserVa)
											{
												// We only want one thread processing recieves at a time.
												WdfSpinLockAcquire(pDmaExt->HeadSpinLock);
												status = PacketProcessCompletedFreeRunDescriptors(pDmaExt, pPacketRecvs);
												WdfSpinLockRelease(pDmaExt->HeadSpinLock);
												infoSize = bufferSize;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		break;

	case PACKET_SEND_IOCTL:
		{
			PPACKET_SEND_STRUCT	pSendPacket;
			status = STATUS_INVALID_PARAMETER;
			// Make sure the Input size is what we expect
			if (InputBufferLength >= sizeof(PACKET_SEND_STRUCT))
			{
				// Get the input buffer, where we get the Application request structure
				status = WdfRequestRetrieveInputBuffer(Request,
					sizeof(PACKET_SEND_STRUCT),		/* Min size */
					(PVOID *) &pSendPacket,			/* buffer */
					&bufferSize);
				if (status == STATUS_SUCCESS)
				{
					status = STATUS_INVALID_PARAMETER;
					// Make sure the size is what we expect
					if (bufferSize >= sizeof(PACKET_SEND_STRUCT))
					{
						// Make sure it is a valid pointer
						if (pSendPacket != NULL)
						{	
							// Range check and make sure we have a DMA Engine where we are asking
							if ((pSendPacket->EngineNum < MAX_NUM_DMA_ENGINES) &&
								(pDevExt->pDmaEngineDevExt[pSendPacket->EngineNum] != NULL))
							{
								// Check output parameters
								if (OutputBufferLength >= (size_t) (pSendPacket->Length))
								{
									// Make sure we have a queue assigned
									status = STATUS_INVALID_DEVICE_REQUEST;
									if (pDevExt->pDmaEngineDevExt[pSendPacket->EngineNum]->DmaType == DMA_TYPE_PACKET_SEND)
									{
										status = PacketStartSend(Request, pDevExt, pSendPacket);
										if (status == STATUS_SUCCESS)
										{
											completeRequest = FALSE;
										}
									}
								}
							}
						}
					}
				}
			}
		}
		break;

	case PACKET_WRITE_IOCTL:
		{
			PPACKET_WRITE_STRUCT	pWritePacket;
			status = STATUS_INVALID_PARAMETER;
			// Make sure the Input size is what we expect
			if (InputBufferLength >= sizeof(PACKET_WRITE_STRUCT))
			{
				// Get the input buffer, where we get the Application request structure
				status = WdfRequestRetrieveInputBuffer(
					Request,
					sizeof(PACKET_WRITE_STRUCT),		/* Min size */
					(PVOID *) &pWritePacket,			/* buffer */
					&bufferSize);
				if (status == STATUS_SUCCESS)
				{
					status = STATUS_INVALID_PARAMETER;
					// Make sure the size is what we expect
					if (bufferSize >= sizeof(PACKET_WRITE_STRUCT))
					{
						// Make sure it is a valid pointer
						if (pWritePacket != NULL)
						{	
							// Range check and make sure we have a DMA Engine where we are asking
							if ((pWritePacket->EngineNum < MAX_NUM_DMA_ENGINES) &&
								(pDevExt->pDmaEngineDevExt[pWritePacket->EngineNum] != NULL))
							{							
								if (pDevExt->pDmaEngineDevExt[pWritePacket->EngineNum]->bAddressablePacketMode)
								{
									// Check output parameters
									if (OutputBufferLength >= (size_t) (pWritePacket->Length))
									{
										// Make sure we have a queue assigned
										status = STATUS_INVALID_DEVICE_REQUEST;
										if (pDevExt->pDmaEngineDevExt[pWritePacket->EngineNum]->DmaType == DMA_TYPE_PACKET_WRITE)
										{
											status = PacketStartWrite(Request, pDevExt, pWritePacket);
											if (status == STATUS_SUCCESS)
											{
												completeRequest = FALSE;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		break;

	case DO_MEM_READ_ACCESS_IOCTL:
		dmaDirection = WdfDmaDirectionReadFromDevice;
	case DO_MEM_WRITE_ACCESS_IOCTL:
		completeRequest = FALSE;
		ReadWriteMemAccess(pDevExt, Request, dmaDirection);
		break;

	case SCAN_LUT_SETUP_IOCTL:
		completeRequest = FALSE;
		SetScanLUT(pDevExt, Request);
		break;

	case DAC_DESC_SETUP_IOCTL:
		completeRequest = FALSE;
		SetDACDesc(pDevExt, Request);
		break;

	case OPEN_OVERFLOW_EVENT_IOCTL:
		//completeRequest = FALSE;
		OpenOverflowEvent(pDevExt, Request);
		break;
	case MESSAGE_EXCHANGE_IOCTL:
		completeRequest = FALSE;
		MessageExchange(pDevExt, Request);
		break;

	case GET_BOARD_CONFIG_IOCTL:
		status = GetBoardConfigDeviceControl(device, Request, &infoSize);
		break;

	case READ_PCI_CONFIG_IOCTL:
		dmaDirection = WdfDmaDirectionReadFromDevice;
	case WRITE_PCI_CONFIG_IOCTL:
		completeRequest = FALSE;
		ReadWritePCIConfig(pDevExt, Request, dmaDirection);
		break;

	case GET_PERF_IOCTL:
		status = GetDmaPerfNumbers(device, Request, &infoSize);
		break;

	case GET_DMA_ENGINE_CAP_IOCTL:
		status = GetDmaEngineCapabilities(device, Request, &infoSize);
		break;

		// IOCtl for Allocating the Receive buffer pool for the DMA Engine specified
	case PACKET_BUF_ALLOC_IOCTL:
		{
			status = PacketBufferAllocate(pDevExt, Request, OutputBufferLength,	InputBufferLength);
			infoSize = OutputBufferLength;
		}
		break;

		// IOCtl for deAllocating the Receive buffer pool to the DMA Engine specified
	case PACKET_BUF_RELEASE_IOCTL:
		{
			status = PacketBufferRelease(pDevExt, Request, OutputBufferLength, InputBufferLength);
			infoSize = OutputBufferLength;
		}
		break;

		// IOCtl for Controling the Packet Generator
	case IMG_ACQ_CONF_IOCTL:
		{
			//DEBUGP(DEBUG_ERROR,	"$$$$$$$$$$$$$$$$$$IMG_ACQ_CONF_IOCTL");
			status = ImageAcquisitionConfig(pDevExt, Request, InputBufferLength);
			infoSize = 0;
		}
		break;

	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	// If we are done, complete the request
	if (completeRequest) 
	{
		WdfRequestCompleteWithInformation( Request, status, (ULONG_PTR) infoSize);
	}
}

/*!***************************************************************************
*
* 	\brief GetDMAEngineContext - Returns the context for DMA Engine specified
* 
* 	\param pAdapter - Pointer to the driver context for this adapter
* 	\param EngineNum - DMA Engine number to retrieve
* 	\param ppDmaExt - Pointer to the pointer os the DMA Data Extension
* 
*   \return status = 0 if successful, -ENODEV otherwise.
* 
*****************************************************************************/
int GetDMAEngineContext(
	PDEVICE_EXTENSION	    pDevExt,
	ULONG32					EngineNum,
	PDMA_ENGINE_DEVICE_EXTENSION * ppDmaExt
	)
{
	PDMA_ENGINE_DEVICE_EXTENSION  pDmaExt;
	int	status = 0;

	if (EngineNum < MAX_NUM_DMA_ENGINES)
	{
		// Now make sure there is a valid DMA Engine
		if (pDevExt->pDmaEngineDevExt[EngineNum] != NULL)
		{
			pDmaExt = pDevExt->pDmaEngineDevExt[EngineNum];
			*ppDmaExt = pDmaExt;
			return 0;
		}
		else
		{
			status = STATUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = STATUS_INVALID_PARAMETER;
	}
	return status;
}

/*!***************************************************************************
*
* 	\brief FindRequestByRequest - Finds a queued request based on matching
*		the Request that is passed in.
*	 
* 	\param pDmaExt - Pointer to the DMA Engine context
*	\param Request - Request to comapre queued request to.
* 
*   \return WDFREQUEST if found, null otherwise.
* 
*****************************************************************************/
WDFREQUEST FindRequestByRequest(
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt,
	IN WDFREQUEST			MatchReq
	)
{
	WDFREQUEST				compRequest;
	NTSTATUS				status;

	status = STATUS_INVALID_DEVICE_REQUEST;
	compRequest = NULL;


	status = WdfIoQueueRetrieveFoundRequest(pDmaExt->TransactionQueue,
		MatchReq,
		&compRequest);
	// WdfIoQueueRetrieveFoundRequest incremented the reference count 
	// of the TagRequest object, so we decrement the count here.

	return compRequest;			
}
//WDFREQUEST FindRequestByRequest(
//	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt,
//	IN WDFREQUEST			MatchReq
//	)
//{
//	WDFREQUEST				prevTagRequest;
//	WDFREQUEST				tagRequest;
//	WDFREQUEST				compRequest;
//	NTSTATUS				status;
//
//	status = STATUS_INVALID_DEVICE_REQUEST;
//	compRequest = prevTagRequest = tagRequest = NULL;
//	do {
//		status = WdfIoQueueFindRequest(pDmaExt->TransactionQueue,
//			prevTagRequest,
//			NULL,
//			NULL,
//			&tagRequest);
//		if (prevTagRequest) 
//		{
//			// WdfIoQueueFindRequest incremented the reference count of the 
//			//  prevTagRequest object, so we decrement the count here.
//			WdfObjectDereference(prevTagRequest);
//		}
//		if (status == STATUS_NO_MORE_ENTRIES) 
//		{
//			break;
//		}
//		if (status == STATUS_NOT_FOUND) 
//		{
//			// The prevTagRequest object is no longer in the queue.
//			prevTagRequest = tagRequest = NULL;
//			continue;
//		}
//		if (!NT_SUCCESS(status)) 
//		{
//			break;
//		}
//		if (tagRequest == MatchReq)
//		{
//			status = WdfIoQueueRetrieveFoundRequest(pDmaExt->TransactionQueue,
//				tagRequest,
//				&compRequest);
//			// WdfIoQueueRetrieveFoundRequest incremented the reference count 
//			// of the TagRequest object, so we decrement the count here.
//			WdfObjectDereference(tagRequest);
//			if (status == STATUS_NOT_FOUND) 
//			{
//				// The TagRequest object is no longer in the queue. But other
//				// requests might match our criteria, so we restart the search.
//				prevTagRequest = tagRequest = NULL;
//				continue;
//			}
//			break;
//		}
//		prevTagRequest = tagRequest;
//	} while (TRUE);
//	return compRequest;			
//}

/*!***************************************************************************
*
* 	\brief ThorDaqDrvIoInCallerContext - 
*	 
* 	\param Device - 
*	\param Request - Request
* 
*   \return Nothing
* 
*****************************************************************************/
VOID ThorDaqDrvIoInCallerContext(
	WDFDEVICE		Device,
	WDFREQUEST	Request
	)
{
	NTSTATUS		status = STATUS_SUCCESS;
	WDF_REQUEST_PARAMETERS  params;
	PPACKET_READ_STRUCT		pReadPacket;
	size_t					bufferSize;
	WDF_OBJECT_ATTRIBUTES	attributes;
	PREQUEST_CONTEXT		reqContext = NULL;
	PVOID					BufferAddress = NULL;
	PMDL					pMdl = NULL;

	WDF_REQUEST_PARAMETERS_INIT(&params);
	WdfRequestGetParameters(Request, &params);

	if ((params.Type == WdfRequestTypeDeviceControl) &&
		(params.Parameters.DeviceIoControl.IoControlCode == PACKET_CHANNEL_READ_IOCTL || params.Parameters.DeviceIoControl.IoControlCode == PACKET_BUF_READ_IOCTL)) 
	{
		// Get the input buffer, where we get the Application request structure
		status = WdfRequestRetrieveInputBuffer(Request,
			sizeof(PACKET_READ_STRUCT),		/* Min size */
			(PVOID *) &pReadPacket,				/* buffer */
			&bufferSize);
		if (status == STATUS_SUCCESS)
		{
			// Make sure the size is what we expect
			if (bufferSize >= sizeof(PACKET_READ_STRUCT))
			{
				// Make sure it is a valid pointer
				if (pReadPacket != NULL)
				{	
					//
					// Next, allocate context space for the request, so that the
					// driver can store handles to the memory objects that will
					// be created for input and output buffers.
					//
					WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, REQUEST_CONTEXT);
					status = WdfObjectAllocateContext(Request, &attributes, &reqContext);
					if (NT_SUCCESS(status)) 
					{
						memcpy(&(reqContext->PacketReadStruct), pReadPacket, sizeof(PACKET_READ_STRUCT));

#if defined(_AMD64_)
						BufferAddress = (PVOID)pReadPacket->BufferAddress;
#else // Assume 32 bit
						// This keeps the compiler happy when /W4 is used.
						BufferAddress = (PVOID)(ULONG32)pReadPacket->BufferAddress;
#endif // 32 vs. 64 bit
						// Allocate an MDL for the Packet Buffer
						pMdl = IoAllocateMdl(BufferAddress, pReadPacket->Length, FALSE, FALSE, NULL);
						if (pMdl != NULL)
						{
							// Try and lock it down...
							__try 
							{
								MmProbeAndLockPages(pMdl, KernelMode, IoWriteAccess);
								reqContext->pMdl = pMdl;
							} 
							__except(EXCEPTION_EXECUTE_HANDLER) 
							{
								status = GetExceptionCode();
								IoFreeMdl(pMdl);
							}
						}
						if (!NT_SUCCESS(status)) 
						{
							WdfRequestComplete(Request, status);
							return;
						}
					}
				}
			}
		}
	}

	if (NT_SUCCESS(status)) 
	{
		status = WdfDeviceEnqueueRequest(Device, Request);
	}

	if (!NT_SUCCESS(status)) 
	{
		if (pMdl != NULL)
		{
			IoFreeMdl(pMdl);
		}
		WdfRequestComplete(Request, status);
	}
	return;
}


#pragma warning(default:4127)