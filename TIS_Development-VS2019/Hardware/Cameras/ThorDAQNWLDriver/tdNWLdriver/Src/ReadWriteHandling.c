// -------------------------------------------------------------------------
// 
// PRODUCT:			DMA Driver
// MODULE NAME:		ReadWriteHandling.c
// 
// MODULE DESCRIPTION: 
// 
// Contains the Read and Write routines for the PCI Express DMA Driver.
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
#include "ReadWriteHandling.tmh"
#endif // TRACE_ENABLED

#pragma warning(disable:4127)  // Constants in while loops. Sorry, I like them.

VOID 
DMADriverEvtIoRead(
	IN WDFQUEUE  	Queue,
	IN WDFREQUEST	Request,
	IN size_t		Length
)
{
	NTSTATUS		status = STATUS_NOT_SUPPORTED;
	size_t			transferSize = 0;

	UNREFERENCED_PARAMETER(Queue);
	UNREFERENCED_PARAMETER(Length);

	DEBUGP(DEBUG_TRACE, "--> DMADriverEvtIoRead, Request %p", Request);

	//pDevExt = DMADriverGetDeviceContext(WdfIoQueueGetDevice(Queue));

	// For now, fail the request until we implement the read
	if (!NT_SUCCESS(status))
	{
		DEBUGP(DEBUG_ERROR, "<-- DMADriverEvtIoRead failed 0x%x", status);
	}
	else
	{
		DEBUGP(DEBUG_INFO, "<-- DMADriverEvtIoRead");
	}

	// Finish request
	WdfRequestCompleteWithInformation(Request, status, transferSize);
}

/*! DMADriverEvtIoWrite
 * 
 * \brief
 * \param Queue
 * \param Request
 * \param Length
 * \return none
 */
VOID 
DMADriverEvtIoWrite(
	IN WDFQUEUE  	Queue,
	IN WDFREQUEST	Request,
	IN size_t		Length
)
{
	NTSTATUS		status = STATUS_NOT_SUPPORTED;
	size_t			transferSize = 0;

	UNREFERENCED_PARAMETER(Queue);
	UNREFERENCED_PARAMETER(Length);

	DEBUGP(DEBUG_TRACE, "--> DMADriverEvtIoWrite, Request %p", Request);

	//pDevExt = DMADriverGetDeviceContext(WdfIoQueueGetDevice(Queue));

	// For now, fail the request until we implement the write
	if (!NT_SUCCESS(status))
	{
		DEBUGP(DEBUG_ERROR, "<-- DMADriverEvtIoWrite failed 0x%x", status);
	}
	else
	{
		DEBUGP(DEBUG_INFO, "<-- DMADriverEvtIoWrite");
	}

	// Finish request
	WdfRequestCompleteWithInformation(Request, status, transferSize);
}

/*! ReadWriteMemAccess
 * 
 *
 * \brief
 * \param pDevExt
 * \param Request
 * \param Rd_wr_n
 * \return
 */
VOID 
ReadWriteMemAccess(
	IN PDEVICE_EXTENSION 	pDevExt,
	IN WDFREQUEST			Request,
	WDF_DMA_DIRECTION		Rd_Wr_n
)
{
	NTSTATUS 		status = STATUS_SUCCESS;
	// Default is to complete request, forwarded requests should not be completed.
	BOOLEAN 		completeRequest = TRUE;
	size_t			bufferSize;
	size_t 			transferSize = 0;
	PDO_MEM_STRUCT	pDoMemStruct;
	UINT32			BarNum;
	UINT64			Offset;
	UINT64			CardOffset;
	UINT32			Length;
	PUINT8			pBufferSafe;
	PUINT8 			cardAddress;
	PUINT8 			bufferAddress;
	INT32			sizeOfTransfer = 4;


	// Get the input buffer pointer
	status = WdfRequestRetrieveInputBuffer(Request,
										   sizeof(DO_MEM_STRUCT),	// Min size
										   (PVOID *) &pDoMemStruct,
										   &bufferSize);
	if (!NT_SUCCESS(status))
	{
		DEBUGP(DEBUG_ERROR, "WdfRequestRetrieveInputBuffer failed 0x%x", status);
	}
	else if (pDoMemStruct == NULL)
	{
		DEBUGP(DEBUG_ERROR, "Input buffer is NULL");
		status = STATUS_INVALID_PARAMETER;
	}
	else if (bufferSize != sizeof(DO_MEM_STRUCT))
	{
		DEBUGP(DEBUG_ERROR, "WdfRequestRetrieveInputBuffer returned incorrect size, size=%Id, expected=%d",
					bufferSize, sizeof(DO_MEM_STRUCT));
		status = STATUS_INVALID_PARAMETER;
	}
	else
	{
		// We are good to go,  save off the input parameters
		// if the IOCTL is buffered, the same memory address is being used
		// for both the input and output buffers
		BarNum = pDoMemStruct->BarNum;
		Offset = pDoMemStruct->Offset;
		CardOffset = pDoMemStruct->CardOffset;
		Length = (UINT32)pDoMemStruct->Length;

		// validate the DO_MEM_STRUCT members
		if ((BarNum >= pDevExt->NumberOfBARS) ||
			(Length == 0))
		{
			DEBUGP(DEBUG_ERROR, "Invalid parameter in DoMemStruct");
			status = STATUS_INVALID_PARAMETER;
			goto DOMEMIOCTLDONE;
		}

		// get the output buffer pointer
		status = WdfRequestRetrieveOutputBuffer(Request,
					(size_t)(Offset+Length),	// Min size
					(PVOID *) &pBufferSafe,
					&bufferSize);
		if (!NT_SUCCESS(status))
		{
			DEBUGP(DEBUG_ERROR, "WdfRequestRetrieveOutputBuffer failed 0x%x", status);
			goto DOMEMIOCTLDONE;
		}
		else if (pBufferSafe == NULL)
		{
			DEBUGP(DEBUG_ERROR, "Output buffer is NULL");
			status = STATUS_INVALID_PARAMETER;
			goto DOMEMIOCTLDONE;
		}
		else if (bufferSize != (size_t)(Offset+Length))
		{
			DEBUGP(DEBUG_ERROR, "WdfRequestRetrieveOutputBuffer returned incorrect size, size=%Id, expected=%ld",
				bufferSize, (UINT32)(Offset + Length));
			status = STATUS_INVALID_PARAMETER;
			goto DOMEMIOCTLDONE;
		} else if ((CardOffset + Length) > pDevExt->BarLength[BarNum])
		{
			DEBUGP(DEBUG_ERROR, "CardOffset + Length is out of range");
			status = STATUS_INVALID_PARAMETER;
			goto DOMEMIOCTLDONE;
		}

		// setup starting addresses
		cardAddress = (PUINT8)pDevExt->BarVirtualAddress[BarNum] +
								CardOffset;
		bufferAddress = (PUINT8)pBufferSafe + Offset;

		// check alignment to determine transfer size
		while ((sizeOfTransfer > 1) &&
			   (((Length % sizeOfTransfer) != 0) ||
			   (((UINT64)cardAddress % sizeOfTransfer) != 0) ||
			   (((UINT64)bufferAddress % sizeOfTransfer) != 0)))
		{
			sizeOfTransfer >>= 1;
		}

		if (Rd_Wr_n == WdfDmaDirectionReadFromDevice)
		{
//			DEBUGP(DEBUG_INFO, "MemoryRead: BAR %d, Card Offset 0x%x, Length %d ",  
	//					BarNum, CardOffset, Length);

			switch(sizeOfTransfer)
			{
			case 4:
				if (pDevExt->BarType[BarNum] == CmResourceTypePort)
				{
					READ_PORT_BUFFER_ULONG((PULONG)cardAddress,
											(PULONG)bufferAddress,
											Length / sizeof(ULONG));
					transferSize = Length;
				}
				else if ((pDevExt->BarType[BarNum] == CmResourceTypeMemory) ||
						 (pDevExt->BarType[BarNum] == CmResourceTypeMemoryLarge))
				{
					READ_REGISTER_BUFFER_ULONG((PULONG)cardAddress,
												(PULONG)bufferAddress,
												Length / sizeof(ULONG));
					transferSize = Length;
				}
				break;
			case 2:
				if (pDevExt->BarType[BarNum] == CmResourceTypePort)
				{
					READ_PORT_BUFFER_USHORT((PUINT16)cardAddress,
											(PUINT16)bufferAddress,
											Length / sizeof(UINT16));
					transferSize = Length;
				}
				else if ((pDevExt->BarType[BarNum] == CmResourceTypeMemory) ||
						 (pDevExt->BarType[BarNum] == CmResourceTypeMemoryLarge))
				{
					READ_REGISTER_BUFFER_USHORT((PUINT16)cardAddress,
												(PUINT16)bufferAddress,
												Length / sizeof(UINT16));
					transferSize = Length;
				}
				break;
			case 1:
				if (pDevExt->BarType[BarNum] == CmResourceTypePort)
				{
					READ_PORT_BUFFER_UCHAR(cardAddress,
										   bufferAddress,
										   Length / sizeof(UINT8));
					transferSize = Length;
				}
				else if ((pDevExt->BarType[BarNum] == CmResourceTypeMemory) ||
						 (pDevExt->BarType[BarNum] == CmResourceTypeMemoryLarge))
				{
					READ_REGISTER_BUFFER_UCHAR(cardAddress,
											   bufferAddress,
											   Length / sizeof(UINT8));
					transferSize = Length;
				}
				break;
			default:
				DEBUGP(DEBUG_INFO, "MemoryRead: invalid LENGTH");
				break;
			}
		}
		else if (Rd_Wr_n == WdfDmaDirectionWriteToDevice)
		{
			DEBUGP(DEBUG_INFO, "MemoryWrite BAR %d: 0x%x, Length %d: 1stByte 0x%x",  
						BarNum, CardOffset, Length, *bufferAddress);
			switch(sizeOfTransfer)
			{
			case 4:
				if (pDevExt->BarType[BarNum] == CmResourceTypePort)
				{
					WRITE_PORT_BUFFER_ULONG((PULONG)cardAddress,
											(PULONG)bufferAddress,
											Length / sizeof(ULONG));
					transferSize = Length;
				}
				else if ((pDevExt->BarType[BarNum] == CmResourceTypeMemory) ||
						 (pDevExt->BarType[BarNum] == CmResourceTypeMemoryLarge))
				{
					WRITE_REGISTER_BUFFER_ULONG((PULONG)cardAddress,
												(PULONG)bufferAddress,
												Length / sizeof(ULONG));
					transferSize = Length;
				}
				break;
			case 2:
				if (pDevExt->BarType[BarNum] == CmResourceTypePort)
				{
					WRITE_PORT_BUFFER_USHORT((PUINT16)cardAddress,
											(PUINT16)bufferAddress,
											Length / sizeof(UINT16));
					transferSize = Length;
				}
				else if ((pDevExt->BarType[BarNum] == CmResourceTypeMemory) ||
						 (pDevExt->BarType[BarNum] == CmResourceTypeMemoryLarge))
				{
					WRITE_REGISTER_BUFFER_USHORT((PUINT16)cardAddress,
												(PUINT16)bufferAddress,
												 Length / sizeof(UINT16));
					transferSize = Length;
				}
				break;
			case 1:
				if (pDevExt->BarType[BarNum] == CmResourceTypePort)
				{
					WRITE_PORT_BUFFER_UCHAR(cardAddress,
											bufferAddress,
											Length / sizeof(UINT8));
					transferSize = Length;
				}
				else if ((pDevExt->BarType[BarNum] == CmResourceTypeMemory) ||
						 (pDevExt->BarType[BarNum] == CmResourceTypeMemoryLarge))
				{
					WRITE_REGISTER_BUFFER_UCHAR(cardAddress,
												bufferAddress,
												Length / sizeof(UINT8));
					transferSize = Length;
				}
				break;
			default:
				DEBUGP(DEBUG_INFO, "   MemoryWrite: invalid LENGTH");
				break;
			}
		}
	}

DOMEMIOCTLDONE:
	// If we are done, complete the request
	if (completeRequest) 
	{
		WdfRequestCompleteWithInformation( Request, status, transferSize);
	}
}

/*! ReadWritePCIConfig 
 *
 * \brief Provides Read and Write access to the boards PCI Configuration space
 * \param pDevExt
 * \param Request
 * \param Rd_Wr_n
 * \return
 */
VOID 
ReadWritePCIConfig(
	IN PDEVICE_EXTENSION 	pDevExt,
	IN WDFREQUEST			Request,
	WDF_DMA_DIRECTION		Rd_Wr_n
)
{
	NTSTATUS 				status = STATUS_SUCCESS;
	size_t					BufferSize;
	size_t 					transferSize = 0;
	PRW_PCI_CONFIG_STRUCT	pPCIConfigXfer;
	PUINT8					pIOBuffer;

	DEBUGP(DEBUG_TRACE, "--> ReadWritePCIConfig, Request %p, IRQL=%d",
			Request, KeGetCurrentIrql());

	// Get the input buffer pointer
	status = WdfRequestRetrieveInputBuffer(Request,
										   sizeof(RW_PCI_CONFIG_STRUCT),	// Min size
										   (PVOID*) &pPCIConfigXfer,
										   &BufferSize);
	if (!NT_SUCCESS(status))
	{
		DEBUGP(DEBUG_ERROR, "WdfRequestRetrieveInputBuffer failed 0x%x", status);
	}
	else if (pPCIConfigXfer == NULL)
	{
		DEBUGP(DEBUG_ERROR, "Input buffer is NULL");
		status = STATUS_INVALID_PARAMETER;
	}
	else if (BufferSize != sizeof(RW_PCI_CONFIG_STRUCT))
	{
		DEBUGP(DEBUG_ERROR, "WdfRequestRetrieveInputBuffer returned incorrect size, size=%Id, expected=%d",
					BufferSize, sizeof(RW_PCI_CONFIG_STRUCT));
		status = STATUS_INVALID_PARAMETER;
	}
	else
	{
		do
		{
			// get the output buffer pointer to copy data from/to
			status = WdfRequestRetrieveOutputBuffer(Request, (size_t)(pPCIConfigXfer->Length), (PVOID *) &pIOBuffer, &BufferSize);
			if (!NT_SUCCESS(status))
			{
				DEBUGP(DEBUG_ERROR, "WdfRequestRetrieveOutputBuffer failed 0x%x", status);
				break;
			}
			else if (pIOBuffer == NULL)
			{
				DEBUGP(DEBUG_ERROR, "Input/Output buffer is NULL");
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			else if (BufferSize != (size_t)(pPCIConfigXfer->Length))
			{
				DEBUGP(DEBUG_ERROR,	"WdfRequestRetrieveOutputBuffer returned incorrect size, size=%Id, expected=%ld",
					BufferSize, (UINT32)(pPCIConfigXfer->Length));
				status = STATUS_INVALID_PARAMETER;
				break;
			} 
			// Now that the buffer has been validated do the actual copy to/from PCI config space
			if (Rd_Wr_n == WdfDmaDirectionReadFromDevice)
			{
				// Get the common config data
				transferSize = pDevExt->BusInterface.GetBusData(pDevExt->BusInterface.Context,
							PCI_WHICHSPACE_CONFIG, (PVOID)pIOBuffer,
							pPCIConfigXfer->Offset, pPCIConfigXfer->Length);
			}
			else if (Rd_Wr_n == WdfDmaDirectionWriteToDevice)
			{
				// Get the common config data
				transferSize = pDevExt->BusInterface.SetBusData(pDevExt->BusInterface.Context,
							PCI_WHICHSPACE_CONFIG, (PVOID)pIOBuffer,
							pPCIConfigXfer->Offset, pPCIConfigXfer->Length);
			}
		} while (FALSE);
	} 
	WdfRequestCompleteWithInformation( Request, status, transferSize);
}

#pragma warning(default:4127)