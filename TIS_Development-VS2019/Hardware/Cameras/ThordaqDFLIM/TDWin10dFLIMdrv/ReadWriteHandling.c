#include "Driver.h"
#include "Trace.h"

#pragma warning(disable:4127)  // Constants in while loops. Sorry, I like them.

VOID ThorDaqDrvEvtIoRead (
	IN WDFQUEUE  	Queue,
	IN WDFREQUEST	Request,
	IN size_t		Length)
{
	NTSTATUS	status = STATUS_NOT_SUPPORTED;
	size_t		transferSize = 0;

	UNREFERENCED_PARAMETER(Queue);
	UNREFERENCED_PARAMETER(Length);

	//pDevExt = DMADriverGetDeviceContext(WdfIoQueueGetDevice(Queue));

	// For now, fail the request until we implement the read
	if (!NT_SUCCESS(status))
	{

	}
	else
	{
	}

	// Finish request
	WdfRequestCompleteWithInformation(Request, status, transferSize);
}

// ThorDaqDrvEvtIoWrite
//
//
VOID ThorDaqDrvEvtIoWrite (
	IN WDFQUEUE  	Queue,
	IN WDFREQUEST	Request,
	IN size_t		Length)
{
	NTSTATUS	status = STATUS_NOT_SUPPORTED;
	size_t		transferSize = 0;

	UNREFERENCED_PARAMETER(Queue);
	UNREFERENCED_PARAMETER(Length);

	//pDevExt = DMADriverGetDeviceContext(WdfIoQueueGetDevice(Queue));

	// For now, fail the request until we implement the write
	if (!NT_SUCCESS(status))
	{

	}
	else
	{

	}

	// Finish request
	WdfRequestCompleteWithInformation(Request, status, transferSize);
}


// ReadWriteMemAccess
//
//
VOID ReadWriteMemAccess(
	IN PDEVICE_EXTENSION 	pDevExt,
	IN WDFREQUEST		Request,
	WDF_DMA_DIRECTION	Rd_Wr_n
	)
{
	NTSTATUS 			status = STATUS_SUCCESS;
	// Default is to complete request, forwarded requests should not be completed.
	BOOLEAN 			completeRequest = TRUE;
	size_t				bufferSize;
	size_t 				transferSize = 0;
	PDO_MEM_STRUCT		pDoMemStruct;
	ULONG32				BarNum;
	ULONG64				Offset;
	ULONG64				CardOffset;
	ULONG32				Length;
	BYTE *				pBufferSafe;
	PUCHAR  			cardAddress;
	PUCHAR  			bufferAddress;
	int 				sizeOfTransfer = 4;


	// Get the input buffer pointer
	status = WdfRequestRetrieveInputBuffer(Request,
		sizeof(DO_MEM_STRUCT),	// Min size
		(PVOID*) &pDoMemStruct,
		&bufferSize);
	if (!NT_SUCCESS(status))
	{

	}
	else if (pDoMemStruct == NULL)
	{
		status = STATUS_INVALID_PARAMETER;
	}
	else if (bufferSize != sizeof(DO_MEM_STRUCT))
	{
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
		Length = (ULONG) pDoMemStruct->Length;

		// validate the DO_MEM_STRUCT members
		if ((BarNum >= pDevExt->NumberOfBARS) ||
			(Length == 0))
		{
			status = STATUS_INVALID_PARAMETER;
			goto DOMEMIOCTLDONE;
		}

		// get the output buffer pointer
		status = WdfRequestRetrieveOutputBuffer(Request,
			(size_t)(Offset + Length),	// Min size
			(PVOID*) &pBufferSafe,
			&bufferSize);
		if (!NT_SUCCESS(status))
		{
			goto DOMEMIOCTLDONE;
		}
		else if (pBufferSafe == NULL)
		{
			status = STATUS_INVALID_PARAMETER;
			goto DOMEMIOCTLDONE;
		}
		else if (bufferSize != (size_t)(Offset+Length))
		{
			status = STATUS_INVALID_PARAMETER;
			goto DOMEMIOCTLDONE;
		} else if ((CardOffset + Length) > pDevExt->BarLength[BarNum])
		{
			status = STATUS_INVALID_PARAMETER;
			goto DOMEMIOCTLDONE;
		}

		// setup starting addresses
		cardAddress = (PUCHAR) pDevExt->BarVirtualAddress[BarNum] +
			CardOffset;
		bufferAddress = (PUCHAR) pBufferSafe + Offset;

		// check alignment to determine transfer size
		while ((sizeOfTransfer > 1) &&
			(((Length % sizeOfTransfer) != 0) ||
			(((ULONGLONG) cardAddress % sizeOfTransfer) != 0) ||
			(((ULONGLONG) bufferAddress % sizeOfTransfer) != 0)))
		{
			sizeOfTransfer >>= 1;
		}

		if (Rd_Wr_n == WdfDmaDirectionReadFromDevice)
		{

			switch(sizeOfTransfer)
			{
			case 4:
				if (pDevExt->BarType[BarNum] == CmResourceTypePort)
				{
					READ_PORT_BUFFER_ULONG((PULONG) cardAddress,
						(PULONG) bufferAddress,
						Length/sizeof(ULONG32));
					transferSize = Length;
				}
				else if ((pDevExt->BarType[BarNum] == CmResourceTypeMemory) ||
					(pDevExt->BarType[BarNum] == CmResourceTypeMemoryLarge))
				{
					READ_REGISTER_BUFFER_ULONG((PULONG) cardAddress,
						(PULONG) bufferAddress,
						Length/sizeof(ULONG32));
					transferSize = Length;
				}
				break;
			case 2:
				if (pDevExt->BarType[BarNum] == CmResourceTypePort)
				{
					READ_PORT_BUFFER_USHORT((PUSHORT) cardAddress,
						(PUSHORT) bufferAddress,
						Length/sizeof(USHORT));
					transferSize = Length;
				}
				else if ((pDevExt->BarType[BarNum] == CmResourceTypeMemory) ||
					(pDevExt->BarType[BarNum] == CmResourceTypeMemoryLarge))
				{
					READ_REGISTER_BUFFER_USHORT((PUSHORT) cardAddress,
						(PUSHORT) bufferAddress,
						Length/sizeof(USHORT));
					transferSize = Length;
				}
				break;
			case 1:
				if (pDevExt->BarType[BarNum] == CmResourceTypePort)
				{
					READ_PORT_BUFFER_UCHAR(cardAddress,
						bufferAddress,
						Length/sizeof(UCHAR));
					transferSize = Length;
				}
				else if ((pDevExt->BarType[BarNum] == CmResourceTypeMemory) ||
					(pDevExt->BarType[BarNum] == CmResourceTypeMemoryLarge))
				{
					READ_REGISTER_BUFFER_UCHAR(cardAddress,
						bufferAddress,
						Length/sizeof(UCHAR));
					transferSize = Length;
				}
				break;
			}
		}
		else if (Rd_Wr_n == WdfDmaDirectionWriteToDevice)
		{
			switch(sizeOfTransfer)
			{
			case 4:
				if (pDevExt->BarType[BarNum] == CmResourceTypePort)
				{
					WRITE_PORT_BUFFER_ULONG((PULONG) cardAddress,
						(PULONG) bufferAddress,
						Length/sizeof(ULONG32));
					transferSize = Length;
				}
				else if ((pDevExt->BarType[BarNum] == CmResourceTypeMemory) ||
					(pDevExt->BarType[BarNum] == CmResourceTypeMemoryLarge))
				{
					WRITE_REGISTER_BUFFER_ULONG((PULONG) cardAddress,
						(PULONG) bufferAddress,
						Length/sizeof(ULONG32));
					transferSize = Length;
				}
				break;
			case 2:
				if (pDevExt->BarType[BarNum] == CmResourceTypePort)
				{
					WRITE_PORT_BUFFER_USHORT((PUSHORT) cardAddress,
						(PUSHORT) bufferAddress,
						Length/sizeof(USHORT));
					transferSize = Length;
				}
				else if ((pDevExt->BarType[BarNum] == CmResourceTypeMemory) ||
					(pDevExt->BarType[BarNum] == CmResourceTypeMemoryLarge))
				{
					WRITE_REGISTER_BUFFER_USHORT((PUSHORT) cardAddress,
						(PUSHORT) bufferAddress,
						Length/sizeof(USHORT));
					transferSize = Length;
				}
				break;
			case 1:
				if (pDevExt->BarType[BarNum] == CmResourceTypePort)
				{
					WRITE_PORT_BUFFER_UCHAR(cardAddress,
						bufferAddress,
						Length/sizeof(UCHAR));
					transferSize = Length;
				}
				else if ((pDevExt->BarType[BarNum] == CmResourceTypeMemory) ||
					(pDevExt->BarType[BarNum] == CmResourceTypeMemoryLarge))
				{
					WRITE_REGISTER_BUFFER_UCHAR(cardAddress,
						bufferAddress,
						Length/sizeof(UCHAR));
					transferSize = Length;
				}
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


// Function:
//		ReadWritePCIConfig - Provides Read and Write access to the boards PCI Configuration space
//
//
//
VOID 
	ReadWritePCIConfig(
	IN PDEVICE_EXTENSION 	pDevExt,
	IN WDFREQUEST		Request,
	WDF_DMA_DIRECTION	Rd_Wr_n
	)
{
	NTSTATUS 			status = STATUS_SUCCESS;
	size_t				BufferSize;
	size_t 				transferSize = 0;
	PRW_PCI_CONFIG_STRUCT	pPCIConfigXfer;
	BYTE *				pIOBuffer;

	// Get the input buffer pointer
	status = WdfRequestRetrieveInputBuffer(Request,
		sizeof(RW_PCI_CONFIG_STRUCT),	// Min size
		(PVOID*) &pPCIConfigXfer,
		&BufferSize);
	if (!NT_SUCCESS(status))
	{
	}
	else if (pPCIConfigXfer == NULL)
	{
		status = STATUS_INVALID_PARAMETER;
	}
	else if (BufferSize != sizeof(RW_PCI_CONFIG_STRUCT))
	{
		status = STATUS_INVALID_PARAMETER;
	}
	else
	{
		do
		{
			// get the output buffer pointer to copy data from/to
			status = WdfRequestRetrieveOutputBuffer(Request, (size_t)(pPCIConfigXfer->Length), (PVOID*) &pIOBuffer, &BufferSize);
			if (!NT_SUCCESS(status))
			{
				break;
			}
			else if (pIOBuffer == NULL)
			{
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			else if (BufferSize != (size_t)(pPCIConfigXfer->Length))
			{
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


// fpga program data download
VOID OpenOverflowEvent (
	IN PDEVICE_EXTENSION 	pDevExt,
	IN WDFREQUEST			Request
	)
{
	UNICODE_STRING	   EventName;
	UNREFERENCED_PARAMETER(Request);

	RtlInitUnicodeString(&EventName, L"\\BaseNamedObjects\\BramOverflowEvent");

	pDevExt->pS2mmLayerExt->BramOverflowEvent = 
		IoCreateNotificationEvent(&EventName, &(pDevExt->pS2mmLayerExt->BramOverflowEventHandle));
	if(NULL != pDevExt->pS2mmLayerExt->BramOverflowEvent)
	{
		ObReferenceObject(pDevExt->pS2mmLayerExt->BramOverflowEvent);
	}

	return;
}

// fpga program status readback
VOID MessageExchange (
	IN PDEVICE_EXTENSION 	pDevExt,
	IN WDFREQUEST			Request
	)
{
	UNREFERENCED_PARAMETER(pDevExt);
	UNREFERENCED_PARAMETER(Request);

	return;
}

// image acquisition configuration
VOID ImgAcqConf (
	IN PDEVICE_EXTENSION 	pDevExt,
	IN WDFREQUEST			Request
	)
{
	UNREFERENCED_PARAMETER(pDevExt);
	UNREFERENCED_PARAMETER(Request);

	return;
}

/*!***************************************************************************
*
* 	\brief SetDACDesc - This routine handles the
*	DAC_DESC_SETUP_IOCTL 
*	 
* 	\param device - The Device object - used to retreive the Device Extensions
* 	\param Request - The I/O Request for the IOCTL call
*   \param pInfoSize - Pointer to the return size information
* 
*   \return STATUS_SUCCESS if it works, an error if it fails.
* 
*****************************************************************************/
VOID SetDACDesc (
	IN PDEVICE_EXTENSION 	pDevExt,
	IN WDFREQUEST			Request
	)
{
	NTSTATUS 			status = STATUS_SUCCESS;
	PDAC_DESCP_TABLE    pDACDescriptor;
	PUCHAR  			cardAddress;
	UCHAR				barNum = 3;
	PULONG              descpPtr;
	DEBUGP(DEBUG_INFO, "SetDACDesc()");
	// Get the input buffer, where we get input of LUT
	status = WdfRequestRetrieveInputBuffer(Request, sizeof(DAC_DESCP_TABLE), (PVOID *) &pDACDescriptor, NULL);
	if (status != STATUS_SUCCESS)
	{
	}
	else
	{
		cardAddress = (PUCHAR) pDevExt->BarVirtualAddress[barNum] + 0x10000; // waveform table at BAR3 offset 0x10000
		descpPtr = (PULONG)(pDACDescriptor->descp);
		WRITE_REGISTER_BUFFER_ULONG((PULONG)(cardAddress), &(*descpPtr) ,DAC_DESCP_MAX_LEN * 2); // i.e. write 4096  64bit descriptors
#ifdef DBG
		UINT64 RegValue = (UINT64)pDACDescriptor->descp[3] << 48 | (UINT64)pDACDescriptor->descp[2] << 32 | (UINT64)pDACDescriptor->descp[1] << 16 | (UINT64)pDACDescriptor->descp[0];
		DEBUGP(DEBUG_INFO, "Wrote table at BAR3 0x10000 0x%p -> first 32 bits 0x%llx", cardAddress, RegValue);
		RegValue = (UINT64)pDACDescriptor->descp[7] << 48 | (UINT64)pDACDescriptor->descp[6] << 32 | (UINT64)pDACDescriptor->descp[5] << 16 | (UINT64)pDACDescriptor->descp[4];
		DEBUGP(DEBUG_INFO, "Wrote table at BAR3 0x10000 0x%p -> 2nd   32 bits 0x%llx", cardAddress, RegValue);
#endif
	}

	WdfRequestComplete(Request, status);
}



/*!***************************************************************************
*
* 	\brief SetScanLUT - This routine handles the
*	SCAN_LUT_SETUP_IOCTL 
*	 
* 	\param device - The Device object - used to retreive the Device Extensions
* 	\param Request - The I/O Request for the IOCTL call
*   \param pInfoSize - Pointer to the return size information
* 
*   \return STATUS_SUCCESS if it works, an error if it fails.
* 
*****************************************************************************/
VOID SetScanLUT (
	IN PDEVICE_EXTENSION 	pDevExt,
	IN WDFREQUEST			Request
	)
{
	NTSTATUS 			status = STATUS_SUCCESS;
	PSCAN_LUT			pScanLUT;
	PUCHAR  			cardAddress;
	size_t 				transferSize = 0;
	UCHAR				barNum = 3;
	ULONG				cardOffset = 0x2000;
	UCHAR				i;

	DEBUGP(DEBUG_INFO, "SetScanLUT()");
	// Get the input buffer, where we get input of LUT
	status = WdfRequestRetrieveInputBuffer(Request, sizeof(SCAN_LUT), (PVOID *) &pScanLUT, NULL);
	if (status != STATUS_SUCCESS)
	{
	}
	else
	{
		cardAddress = (PUCHAR) pDevExt->BarVirtualAddress[barNum] + cardOffset;
		//if(pScanLUT->ch == 1)
		//{
		//	WRITE_REGISTER_BUFFER_USHORT((PUSHORT)cardAddress, &(pScanLUT->lut[0]) , SCAN_LUT_MAX_LEN);

		//	transferSize = SCAN_LUT_MAX_LEN * sizeof(USHORT);
		//}
		//else if(pScanLUT->ch == 4)
		//{
			for(i = 0; i < 4/*pScanLUT->ch*/; i++)
			{
				WRITE_REGISTER_BUFFER_USHORT((PUSHORT)(cardAddress + (UINT64)i * cardOffset), &(pScanLUT->lut[0]), SCAN_LUT_MAX_LEN);
			}

			transferSize = SCAN_LUT_MAX_LEN * sizeof(USHORT) * 4/*pScanLUT->ch*/;
		//}

	}

	//WdfRequestCompleteWithInformation( Request, status, transferSize);
	WdfRequestComplete(Request, status);
}





#pragma warning(default:4127)