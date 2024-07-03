// -------------------------------------------------------------------------
// 
// PRODUCT:			DMA Driver
// MODULE NAME:		IoctlHandling.c
// 
// MODULE DESCRIPTION: 
// 
// Contains the main IOCtl entry point for the driver
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
#include "IoctlHandling.tmh"
#endif // TRACE_ENABLED

#pragma warning(disable:4127)	// Constants in while loops.

/*! DMADriverEvtIoDeviceControl 
 *
 *  \brief This is the main IOCtl dispatch for
 * 	the driver. The job of this routine is to validate the request and in some
 *	cases route the request to the approriate DMA queue or function
 *  \param Queue - WDF Managed Queue where request came from
 *  \param Request - Pointer to the IOCtl request
 *  \param InputBufferLength - Size of the IOCtl Input buffer
 * 	\param OutputBufferLength - Size of the IOCtl Output buffer
 *  \param IoControlCode - IOCTL parameter
 *  \return NTSTATUS
 * 	\note This routine is called at IRQL < DISPATCH_LEVEL.
 */
VOID 
DMADriverEvtIoDeviceControl (
	IN WDFQUEUE			Queue,
	IN WDFREQUEST		Request,
	IN size_t			OutputBufferLength,
	IN size_t			InputBufferLength,
	IN unsigned long	IoControlCode
)
{
	WDFDEVICE			device = WdfIoQueueGetDevice(Queue);
	PDEVICE_EXTENSION	pDevExt = DMADriverGetDeviceContext(device);

	NTSTATUS 			status = STATUS_SUCCESS;
	size_t				bufferSize;
	size_t				infoSize = 0;
	BOOLEAN 			completeRequest = TRUE; // Default to complete the request here
	WDF_DMA_DIRECTION	dmaDirection = WdfDmaDirectionWriteToDevice;

	UNREFERENCED_PARAMETER(OutputBufferLength);
	//DEBUGP(DEBUG_TRACE,	"--> DMADriverEvtIoDeviceControl, Request %p, IoControlCode 0x%x", Request, IoControlCode);
	// DEBUGP(DEBUG_INFO,	"  OutputBufferLength = %Id, InputBufferLength = %Id",
	//		OutputBufferLength, InputBufferLength);

	if (pDevExt == NULL)
	{
		WdfRequestCompleteWithInformation(Request, STATUS_NO_SUCH_DEVICE, 0);
		DEBUGP(DEBUG_ERROR, "    ERROR: STATUS_NO_SUCH_DEVICE (pDevExt is NULL)");
		return;
	}

	switch (IoControlCode)
	{
		case PACKET_RECEIVE_IOCTL:
		{
			DEBUGP(DEBUG_INFO, "    PACKET_RECEIVE_IOCTL");
			PPACKET_RECEIVE_STRUCT	pRecvPacket;
			status = STATUS_INVALID_PARAMETER;
			if (InputBufferLength >= sizeof(PACKET_RECEIVE_STRUCT))
			{
				// Get the input buffer, that has all the info we need for the receive
				status = WdfRequestRetrieveInputBuffer(Request,
					sizeof(PACKET_RECEIVE_STRUCT),	// Size
					(PVOID *) &pRecvPacket,			// Buffer
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
											if (pRecvPacket->RxReleaseToken < (UINT32)pDmaExt->NumberOfUsedDescriptors)
											{
												// Go do the return of a descriptor even if it is out of order
												status = PacketProcessReturnedDescriptors(pDevExt, pDmaExt, pRecvPacket->RxReleaseToken);
												if (status != STATUS_SUCCESS)
												{
													DEBUGP(DEBUG_ERROR,	"PacketProcessReturnedDescriptors Failed, DMA Engine %d, status 0x%x.", 
																pDmaExt->DmaEngine, status);
													break;
												}
											}
											else
											{
												if (pRecvPacket->RxReleaseToken != -1)
												{
													// Bad Recieve Token
													DEBUGP(DEBUG_ERROR,	"DMA Engine %d, Bad Token 0x%x given", 
																pDmaExt->DmaEngine, pRecvPacket->RxReleaseToken);
												}
											}
											status = STATUS_INVALID_PARAMETER; 
											// Check parameters
											if (OutputBufferLength >= (size_t) sizeof(PACKET_RET_RECEIVE_STRUCT))
											{
												if (pDmaExt->UserVa)
												{
													if (pRecvPacket->Block)
													{
														DEBUGP(DEBUG_VERBOSE, "PacketRecv: DMA #%d, 0x%x", pRecvPacket->EngineNum, pRecvPacket->RxReleaseToken);
														// Queue the request and wake up the Receive processing
														status = WdfRequestForwardToIoQueue(Request, pDmaExt->TransactionQueue);
														// We only want one thread processing recieves at a time.
														WdfSpinLockAcquire(pDmaExt->HeadSpinLock);
														PacketProcessCompletedReceives(pDevExt, pDmaExt);
														WdfSpinLockRelease(pDmaExt->HeadSpinLock);
														if (status == STATUS_SUCCESS)
														{
															completeRequest = FALSE;
														}
													}
													else // Non-blocking form of PacketReceive
													{
														status = PacketProcessCompletedReceiveNB(pDmaExt, Request);
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

		case PACKET_READ_IOCTL:
		{
			PPACKET_READ_STRUCT		pReadPacket;
	
//			DEBUGP(DEBUG_INFO, "    PACKET_READ_IOCTL");
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
												//DEBUGP(DEBUG_INFO, "PacketStartRead(): DMA #%d, Offset 0x%llx, Flags 0x%x, Length %u",
												//	pReadPacket->EngineNum, pReadPacket->CardOffset, pReadPacket->ModeFlags, pReadPacket->Length);
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
		break;

		case PACKET_RECEIVES_IOCTL:
		{
			DEBUGP(DEBUG_INFO, "    PACKET_RECEIVES_IOCTL");
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
										status = STATUS_INVALID_PARAMETER; 
										if (pDmaExt->UserVa)
										{
											DEBUGP(DEBUG_VERBOSE, "PacketRecvs: DMA #%d, AvailEntries %d ",  
												pPacketRecvs->EngineNum, pPacketRecvs->AvailNumEntries);
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
		break;

		case PACKET_SEND_IOCTL:
		{
			PPACKET_SEND_STRUCT	pSendPacket;
			DEBUGP(DEBUG_INFO, "    PACKET_SEND_IOCTL");

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
			DEBUGP(DEBUG_INFO, "    PACKET_WRITE_IOCTL");
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
							if (pDevExt->pDmaEngineDevExt[pWritePacket->EngineNum]->bAddressablePacketMode)
							{
								// Range check and make sure we have a DMA Engine where we are asking
								if ((pWritePacket->EngineNum < MAX_NUM_DMA_ENGINES) &&
									(pDevExt->pDmaEngineDevExt[pWritePacket->EngineNum] != NULL))
								{
									// Check output parameters
									if (OutputBufferLength >= (size_t) (pWritePacket->Length))
									{
										// Make sure we have a queue assigned
										status = STATUS_INVALID_DEVICE_REQUEST;
										if (pDevExt->pDmaEngineDevExt[pWritePacket->EngineNum]->DmaType == DMA_TYPE_PACKET_WRITE)
										{
											DEBUGP(DEBUG_INFO, "PacketStartWrite(): DMA #%d, Offset 0x%llx, Flags 0x%x, Length %u",
												pWritePacket->EngineNum, pWritePacket->CardOffset, pWritePacket->ModeFlags, pWritePacket->Length);

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

		// Begin User Interrupt 
		case USER_IRQ_WAIT_IOCTL:
		{
			PUSER_IRQ_WAIT_STRUCT		pUIRQWaitStruct;
			// Set spinlock.
			WdfSpinLockAcquire(pDevExt->UsrIntSpinLock);
			// If any interrupt requests are pending,
			if (pDevExt->UsrIntRequest != NULL)
			{
				// Release spinlock
				WdfSpinLockRelease(pDevExt->UsrIntSpinLock);
				// Return duplicate request status.
				DEBUGP(DEBUG_INFO, "    USER_IRQ_WAIT__IOCTL error STATUS_DUPLICATE_OBJECTID");
				status = STATUS_DUPLICATE_OBJECTID;
			}
			// If no requests are pending,
			else
			{
				// Cache Application 'Request' pointer, for retrieval after time out or interrupt.
//				DEBUGP(DEBUG_INFO, "    USER_IRQ_WAIT__IOCTL, Forward to TransactionQueue");
				pDevExt->UsrIntRequest = Request;
				// Release spinlock,	
				WdfSpinLockRelease(pDevExt->UsrIntSpinLock);
				// Get the input buffer, where the wait information is passed
				status = WdfRequestRetrieveInputBuffer
					(
					Request,
					sizeof(USER_IRQ_WAIT_STRUCT),			// Min size
					(PVOID *)&pUIRQWaitStruct,				// buffer
					&bufferSize
					);
				// If buffer receieve is successful,
				if (status == STATUS_SUCCESS)
				{
					pDevExt->pDmaRegisters->commonControl.ControlStatus |= CARD_USER_INTERRUPT_MODE;
					if (pUIRQWaitStruct->dwTimeoutMilliSec != 0)
					{
						// If initialization is successful, start the timer,
//						DEBUGP(DEBUG_INFO, "    Calling UsrIntReqTimerStart");
						DMADriverUsrIntReqTimerStart(pDevExt, pUIRQWaitStruct->dwTimeoutMilliSec);
					}
					// And return pending status message.
					status = STATUS_PENDING;
					completeRequest = FALSE;
				}
				else
				{
					// If timer start fails, return bad param status.
					status = STATUS_INVALID_PARAMETER;
				}
			}
		}

		break;
		// CANCEL IOCTL
		case USER_IRQ_CANCEL_IOCTL:
		{
			// Set Spinlock
			WdfSpinLockAcquire(pDevExt->UsrIntSpinLock);
//			DEBUGP(DEBUG_INFO, "    USER_IRQ_CANCEL_IOCTL");

			// Determine pending interrupt requests.
			if (pDevExt->UsrIntRequest != NULL)
			{
				DEBUGP(DEBUG_INFO, "    USER_IRQ_CANCEL_IOCTL...complete with STATUS_IO_TIMEOUT");
				// Stop Timer.
				DMADriverUsrIntReqTimerStop(pDevExt);
				// Return STATUS_TIMEOUT
				WdfRequestComplete(pDevExt->UsrIntRequest, STATUS_IO_TIMEOUT);
				// Reset pointer back to NULL.
				pDevExt->UsrIntRequest = NULL;
			}
			// Release spinlock.
			WdfSpinLockRelease(pDevExt->UsrIntSpinLock);
		}
		break;
		// End User Interrupt

		case DO_MEM_READ_ACCESS_IOCTL:
			dmaDirection = WdfDmaDirectionReadFromDevice;
		case DO_MEM_WRITE_ACCESS_IOCTL:
//			DEBUGP(DEBUG_INFO, "    DO_MEM_ACCESS_IOCTL, Forward to IoctlQueue");
			completeRequest = FALSE;
			ReadWriteMemAccess(pDevExt, Request, dmaDirection);
		break;

		case GET_BOARD_CONFIG_IOCTL:
			DEBUGP(DEBUG_INFO, "    GET_BOARD_CONFIG_IOCTL, Process Now");
			status = GetBoardConfigDeviceControl(device, Request, &infoSize);
		break;

		case READ_PCI_CONFIG_IOCTL:
			dmaDirection = WdfDmaDirectionReadFromDevice;
		case WRITE_PCI_CONFIG_IOCTL:
			DEBUGP(DEBUG_INFO, "    R/W_PCI_CONFIG_IOCTL, Forward to IoctlQueue");
			completeRequest = FALSE;
			ReadWritePCIConfig(pDevExt, Request, dmaDirection);
		break;

		case GET_PERF_IOCTL:
			DEBUGP(DEBUG_INFO, "    BLOCK_DIRECT_GET_PERF_IOCTL, Process now");
			status = GetDmaPerfNumbers(device, Request, &infoSize);
		break;

		case GET_DMA_ENGINE_CAP_IOCTL:
			DEBUGP(DEBUG_INFO, "    GET_DMA_ENGINE_CAP_IOCTL, Process Now");
			status = GetDmaEngineCapabilities(device, Request, &infoSize);
		break;

		// IOCtl for Allocating the Receive buffer pool for the DMA Engine specified
		case PACKET_BUF_ALLOC_IOCTL:
		{
//			DEBUGP(DEBUG_INFO, "    PACKET_BUF_ALLOC_IOCTL");
			status = PacketBufferAllocate(pDevExt, Request);
			infoSize = OutputBufferLength;
		}
		break;

		// IOCtl for deAllocating the Receive buffer pool to the DMA Engine specified
		case PACKET_BUF_RELEASE_IOCTL:
		{
			status = PacketBufferRelease(pDevExt, Request);
			infoSize = OutputBufferLength;
		}
		break;

		// IOCtl for Resetting a specific DMA Engine
		case RESET_DMA_ENGINE_IOCTL:
		{
			DEBUGP(DEBUG_INFO, "    RESET_DMA_ENGINE_IOCTL");
			status = ResetDMAEngine(pDevExt, Request);
			infoSize = OutputBufferLength;
		}
		break;

		default:
			DEBUGP(DEBUG_WARN, "    STATUS_INVALID_DEVICE_REQUEST: IoControlCode: 0x%x", IoControlCode);
			status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	// If we are done, complete the request
	if (completeRequest) 
	{
		WdfRequestCompleteWithInformation(Request, status, (ULONG_PTR) infoSize);
	}
	//DEBUGP(DEBUG_TRACE, "<-- DMADriverEvtIoDeviceControl, status 0x%x", status);
}

/*! GetDMAEngineContext 
 *
 * \brief Returns the context for DMA Engine specified
 * 	\param pAdapter - Pointer to the driver context for this adapter
 * 	\param EngineNum - DMA Engine number to retrieve
 * 	\param ppDmaExt - Pointer to the pointer os the DMA Data Extension
 * 
 *   \return status = 0 if successful, -ENODEV otherwise.
 */
INT32
GetDMAEngineContext(
	PDEVICE_EXTENSION				pDevExt,
	UINT32							EngineNum,
	PDMA_ENGINE_DEVICE_EXTENSION *	ppDmaExt
	)
{
	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;
	INT32							status = 0;

    if (EngineNum < MAX_NUM_DMA_ENGINES)
    {
        // Now make sure there is a valid DMA Engine
		if (pDevExt->pDmaEngineDevExt[EngineNum] != NULL)
        {
			pDmaExt = pDevExt->pDmaEngineDevExt[EngineNum];
			* ppDmaExt = pDmaExt;
			return 0;
		}
		else
		{
			DEBUGP(DEBUG_ERROR, "Dma Engine %u does not exist\n", EngineNum);
			status = STATUS_INVALID_PARAMETER;
		}
	}
	else
	{
		DEBUGP(DEBUG_ERROR, "Invalid Dma Engine number %u\n", EngineNum);
		status = STATUS_INVALID_PARAMETER;
	}
	return status;
}

/*! FindRequestByRequest
 *
 * 	\brief Finds a queued request based on matching
 *	the Request that is passed in.
 * 	\param pDmaExt - Pointer to the DMA Engine context
 *	\param Request - Request to comapre queued request to.
 *   \return WDFREQUEST if found, null otherwise.
 */
WDFREQUEST 
FindRequestByRequest(
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt,
	IN WDFREQUEST					MatchReq	
	)
{
	WDFREQUEST			prevTagRequest;
	WDFREQUEST			tagRequest;
	WDFREQUEST			compRequest;
	NTSTATUS			status;

    status = STATUS_INVALID_DEVICE_REQUEST;
	compRequest = prevTagRequest = tagRequest = NULL;
    do {
			status = WdfIoQueueFindRequest(pDmaExt->TransactionQueue, prevTagRequest,
                                       NULL,
                                       NULL,
                                       &tagRequest);
        if (prevTagRequest) 
		{
            // WdfIoQueueFindRequest incremented the reference count of the 
			//  prevTagRequest object, so we decrement the count here.
            WdfObjectDereference(prevTagRequest);
        }
        if (status == STATUS_NO_MORE_ENTRIES) 
		{
            break;
        }
        if (status == STATUS_NOT_FOUND) 
		{
            // The prevTagRequest object is no longer in the queue.
            prevTagRequest = tagRequest = NULL;
            continue;
        }
        if (!NT_SUCCESS(status)) 
		{
            break;
        }
		if (tagRequest == MatchReq)
		{
			status = WdfIoQueueRetrieveFoundRequest(pDmaExt->TransactionQueue,
                                                   tagRequest,
                                                   &compRequest);
			// WdfIoQueueRetrieveFoundRequest incremented the reference count 
			// of the TagRequest object, so we decrement the count here.
            WdfObjectDereference(tagRequest);
		    if (status == STATUS_NOT_FOUND) 
			{
                // The TagRequest object is no longer in the queue. But other
				// requests might match our criteria, so we restart the search.
                prevTagRequest = tagRequest = NULL;
		        continue;
			}
			break;
		}
		prevTagRequest = tagRequest;
    } while (TRUE);
	return compRequest;			
}


/*! DMADriverIoInCallerContext - 
 *	
 *  \brief
 * 	\param Device  
 *	\param Request 
 *  \return none
 */
VOID 
DMADriverIoInCallerContext(
	WDFDEVICE		Device,
	WDFREQUEST		Request
)
{
	PDEVICE_EXTENSION		pDevExt = DMADriverGetDeviceContext(Device);
    WDF_REQUEST_PARAMETERS  params;
	PVOID					pInBuffer = NULL;
	size_t					InBufferLen = 0;
	PVOID					pOutBuffer = NULL;
	size_t					OutBufferLen = 0;

    PREQUEST_CONTEXT		reqContext = NULL;
    WDF_OBJECT_ATTRIBUTES	attributes;
	PVOID					BufferAddress = NULL;
	size_t					bufferSize = 0;
	PMDL					pMdl = NULL;
	BOOLEAN					MapAndLock = FALSE;
	UINT8					DMAEngine  = 0xFF;
	NTSTATUS				status = STATUS_INVALID_DEVICE_REQUEST;

	WDF_REQUEST_PARAMETERS_INIT(&params);
    WdfRequestGetParameters(Request, &params);

	DEBUGP(DEBUG_VERBOSE, "-->DMADriverIoInCallerContext()");

	if (params.Type == WdfRequestTypeDeviceControl)
	{
		// Get the input buffer...
		if (params.Parameters.DeviceIoControl.InputBufferLength > 0)
		{
			status = WdfRequestRetrieveInputBuffer(Request,
												   params.Parameters.DeviceIoControl.InputBufferLength,
												   &pInBuffer,
												   &InBufferLen);
			if (!NT_SUCCESS(status))
			{
				WdfRequestComplete(Request, status);
				return;
			}
		}
		// Get the Output buffer...
		if (params.Parameters.DeviceIoControl.OutputBufferLength > 0)
		{
			status = WdfRequestRetrieveOutputBuffer(Request,
												   params.Parameters.DeviceIoControl.OutputBufferLength,
												   &pOutBuffer,
												   &OutBufferLen);
			if (!NT_SUCCESS(status))
			{
				WdfRequestComplete(Request, status);
				return;
			}
		}

		// Check for PacketReadEx API Call.
		if (params.Parameters.DeviceIoControl.IoControlCode == PACKET_READ_IOCTL)
		{
			PPACKET_READ_STRUCT		pReadPacket = (PPACKET_READ_STRUCT)pInBuffer;
			DEBUGP(DEBUG_VERBOSE, "    Parameters.DeviceIoControl.IoControlCode == PACKET_READ_IOCTL");

			// Make sure the size is what we expect
			if (InBufferLen >= sizeof(PACKET_READ_STRUCT))
			{
				// Make sure it is a valid pointer
				if (pReadPacket != NULL)
				{
#if defined(_AMD64_)
					BufferAddress = (PVOID)pReadPacket->BufferAddress;
#else // Assume 32 bit
					// This keeps the compiler happy when /W4 is used.
					BufferAddress = (PVOID)(UINT32)pReadPacket->BufferAddress;
#endif // 32 vs. 64 bit
					bufferSize = pReadPacket->Length;
					DMAEngine = (UINT8)pReadPacket->EngineNum;
					MapAndLock = TRUE;
				}
			}
		}
		// Check for PacketWriteEx API Call.
		else if (params.Parameters.DeviceIoControl.IoControlCode == PACKET_WRITE_IOCTL) 
		{
			PPACKET_WRITE_STRUCT		pWritePacket = (PPACKET_WRITE_STRUCT)pInBuffer;

			DEBUGP(DEBUG_INFO, "    Parameters.DeviceIoControl.IoControlCode == PACKET_WRITE_IOCTL");
			// Make sure the size is what we expect
			if (InBufferLen >= sizeof(PACKET_WRITE_STRUCT))
			{
				// Make sure it is a valid pointer
				if (pWritePacket != NULL)
				{
					bufferSize = pWritePacket->Length;
					BufferAddress = pOutBuffer;
					bufferSize = OutBufferLen;
					MapAndLock = TRUE;
				}
			}
		}
		// Check for PacketWrite API Call.
		else if (params.Parameters.DeviceIoControl.IoControlCode == PACKET_SEND_IOCTL) 
		{
			PPACKET_SEND_STRUCT		pSendPacket = (PPACKET_SEND_STRUCT)pInBuffer;

			// Make sure the size is what we expect
			if (InBufferLen >= sizeof(PACKET_SEND_STRUCT))
			{
				// Make sure it is a valid pointer
				if (pSendPacket != NULL)
				{
					bufferSize = pSendPacket->Length;
					BufferAddress = pOutBuffer;
					bufferSize = OutBufferLen;
					MapAndLock = TRUE;
				}
			}
		}
		// Check for SetupPacketMode API Call.
		else if (params.Parameters.DeviceIoControl.IoControlCode == PACKET_BUF_ALLOC_IOCTL) 
		{
			PBUF_ALLOC_STRUCT		pBufAlloc = (PBUF_ALLOC_STRUCT)pInBuffer;
			DEBUGP(DEBUG_VERBOSE, "    Parameters.DeviceIoControl.IoControlCode == PACKET_BUF_ALLOC_IOCTL (SetupPacketMode)");

			// Validate EngineNum and make sure it has been properly initialized
			if (pBufAlloc != NULL) 
			{
				if ((pBufAlloc->EngineNum < MAX_NUM_DMA_ENGINES) &&
					(pDevExt->pDmaEngineDevExt[pBufAlloc->EngineNum] != NULL))
				{
					PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;
					status = GetDMAEngineContext(pDevExt, pBufAlloc->EngineNum, &pDmaExt);
					if (NT_SUCCESS(status)) 
					{
						// Now make sure there is a queue associated and it is a Packet Type Engine
						if (pDmaExt->DmaType == DMA_TYPE_PACKET_RECV)
						{
							// See if the Application did the allocate
							if ((pBufAlloc->AllocationMode == PACKET_MODE_FIFO) ||
								(pBufAlloc->AllocationMode == PACKET_MODE_STREAMING))
							{
								BufferAddress = pOutBuffer;
								bufferSize = OutBufferLen;
								DMAEngine = (UINT8)pBufAlloc->EngineNum;
								MapAndLock = TRUE;
								if (pBufAlloc->AllocationMode == PACKET_MODE_STREAMING)
								{
									pDmaExt->bFreeRun = TRUE;
								}
								pDmaExt->PacketMode = pBufAlloc->AllocationMode;
							}
							else if (pBufAlloc->AllocationMode == PACKET_MODE_ADDRESSABLE)
							{
								if (pDmaExt->bAddressablePacketMode)
								{
									if (pBufAlloc->NumberDescriptors > pDmaExt->NumberOfDescriptors)
									{
										DEBUGP(DEBUG_WARN, "Requesting more Descriptors than are available %ld\n", pBufAlloc->NumberDescriptors);
										status = STATUS_INVALID_PARAMETER;
									}
									else
									{
										pDmaExt->PacketMode = pBufAlloc->AllocationMode;
										// Special case since we do not Map and lock for addressable mode.
										status = WdfDeviceEnqueueRequest(Device, Request);
									}
								}
								else
								{
									DEBUGP(DEBUG_WARN, "Addressable Packet mode not supported on this firmware");
									status = STATUS_INVALID_DEVICE_REQUEST;
								}	
							}
							else
							{
								pBufAlloc->Length = 0;
								pBufAlloc->MaxPacketSize = 0;
								DEBUGP(DEBUG_WARN, "Packet / allocate mode is not supported");
								status = STATUS_INVALID_PARAMETER;
							}
						}
					}
				}
				else
				{
					DEBUGP(DEBUG_ERROR, "Packet Buffer Allocate DMA Engine number invalid 0x%x", status);
					status = STATUS_INVALID_PARAMETER;
				}
			}
		}
		else
		{
			// Not an DeviceIOControl we are trapping here...
			status = WdfDeviceEnqueueRequest(Device, Request);
		}

		if (MapAndLock)
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
				// Allocate an MDL for locking down
				pMdl = IoAllocateMdl(BufferAddress, (UINT32)bufferSize, FALSE, FALSE, NULL);
				if (pMdl != NULL)
				{
					// Try and lock it down...
					try
					{
						MmProbeAndLockPages(pMdl, KernelMode, IoWriteAccess);
						reqContext->pMdl = pMdl;
						reqContext->pVA = BufferAddress;
						reqContext->Length = (UINT32)bufferSize;
						reqContext->DMAEngine = DMAEngine;
						status = STATUS_SUCCESS;
						status = WdfDeviceEnqueueRequest(Device, Request);
					}
					except(EXCEPTION_EXECUTE_HANDLER)
					{
						status = GetExceptionCode();
						IoFreeMdl(pMdl);
						DEBUGP(DEBUG_ERROR, "DMADriverIoInCallerContext: Exception %lx locking buffer on pVA %p, size 0x%x, DMA# %d\n", status, BufferAddress, bufferSize, DMAEngine);
					}
				}
				else
				{
					DEBUGP(DEBUG_ERROR, "DMADriverIoInCallerContext: MDL == NULL\n");
				}
			}
		}
	}
	else
	{
		// Not a DeviceIOControl params.type
		status = WdfDeviceEnqueueRequest(Device, Request);
	}

	if (!NT_SUCCESS(status))
	{
	    WdfRequestComplete(Request, status);
    }
	DEBUGP(DEBUG_VERBOSE, "<--DMADriverIoInCallerContext()");
	return;
}
#pragma warning(default:4127)
