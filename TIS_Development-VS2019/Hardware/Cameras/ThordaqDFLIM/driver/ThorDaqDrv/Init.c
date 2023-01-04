#include "driver.h"
#include <stdio.h>
#include <math.h>

// Local defines
#define PCI_STATUS_CAPABILITIES_LIST_PRESENT	(0x0010)
#define PCI_EXPROM_ADDRESS_OFFSET				11

DECLARE_CONST_UNICODE_STRING(InterruptManagementName, L"Interrupt Management");
DECLARE_CONST_UNICODE_STRING(MSIPropertiesName, L"MessageSignaledInterruptProperties");
DECLARE_CONST_UNICODE_STRING(MSISupportedName, L"MSISupported");
DECLARE_CONST_UNICODE_STRING(MSILimitName, L"MessageNumberLimit");

DECLARE_CONST_UNICODE_STRING(InterruptModeName, L"InterruptMode");

NTSTATUS ThorDaqDrvSetupQueues(IN PDEVICE_EXTENSION pDevExt,
							  IN PDMA_ENGINE_DEVICE_EXTENSION pDmaExt);

NTSTATUS ThorDaqDrvCreateDMAObjects(IN PDEVICE_EXTENSION pDevExt,
								   IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt);

NTSTATUS ThorDaqDrvCreateDMADescBuffers(IN PDEVICE_EXTENSION pDevExt,
									   IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt);

NTSTATUS ThorDaqDrvSetupDPC(IN PDEVICE_EXTENSION pDevExt,
						   IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt);

NTSTATUS ThorDaqDrvSetS2mmDPC(IN PDEVICE_EXTENSION pDevExt,
						  IN PS2MM_LAYER_EXTENSION	pS2mmExt);

NTSTATUS ThorDaqDrvScanPciResources (IN PDEVICE_EXTENSION pDevExt,
									IN WDFCMRESLIST ResourcesTranslated);
NTSTATUS ThorDaqDrvBoardConfigInit (PDEVICE_EXTENSION pDevExt);

NTSTATUS ThorDaqDrvBoardDmaRelease(PDEVICE_EXTENSION pDevExt, UCHAR DmaEngNum);

NTSTATUS ThorDaqDrvGetRegHardwareInfo(PDEVICE_EXTENSION pDevExt);

NTSTATUS ThorDaqDrvGetRegistryInfo (PDEVICE_EXTENSION pDevExt);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, ThorDaqDrvInitializeDeviceExtension)
#pragma alloc_text (PAGE, ThorDaqDrvPrepareHardware)
#pragma alloc_text (PAGE, ThorDaqDrvScanPciResources)
#endif  /* ALLOC_PRAGMA */

// ThorDaqDrvBoardDmaInit
//
// Get the DMA Configuration from the card and initialize the DMA routines.
//
// This may get called twice.  If we have already setup the DMA engine once before,
// do not recreate the DMA Engine Dev Ext, Queue or Transaction structures.
NTSTATUS
	ThorDaqDrvBoardDmaInit (
	PDEVICE_EXTENSION pDevExt
	)
{
	NTSTATUS				status = STATUS_SUCCESS;
	UINT8					dmaNum;
	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;
	WDF_DMA_DIRECTION		dmaDirection;

	// Setup the DMA Engines - 
	// By default, BAR 0 is used for the DMA registers.  Validate it as a Memory Bar.
	if ((pDevExt->NumberOfBARS > DMA_BAR) && ((pDevExt->BarType[DMA_BAR] == CmResourceTypeMemory) ||
		(pDevExt->BarType[DMA_BAR] == CmResourceTypeMemoryLarge)))
	{
		pDevExt->pDmaRegisters = (PBAR0_REGISTER_MAP_STRUCT) pDevExt->BarVirtualAddress[DMA_BAR];

		if (pDevExt->pDmaRegisters != NULL)
		{
			// Scan each possible DMA register locations for existing DMA Engine controller
			for (dmaNum = 0; dmaNum < MAX_NUM_DMA_ENGINES; dmaNum++)
			{
				if ((pDevExt->pDmaRegisters->dmaEngine[dmaNum].Capabilities &
					DMA_CAP_ENGINE_PRESENT) == DMA_CAP_ENGINE_PRESENT)
				{
					// See if this is a Block Mode DMA Engine
					if ((pDevExt->pDmaRegisters->dmaEngine[dmaNum].Capabilities & DMA_CAP_ENGINE_TYPE_MASK) == DMA_CAP_BLOCK_DMA)
					{
						// If so, bypass it since we do not support it.
						continue;
					}
					// Found one! See if it is a System To Card
					if (DMA_CAP_ENGINE_DIRECTION(pDevExt->pDmaRegisters->dmaEngine[dmaNum].Capabilities) ==
						DMA_CAP_SYSTEM_TO_CARD)
					{
						// S2C DMA engine
						dmaDirection = WdfDmaDirectionWriteToDevice;
						if (++pDevExt->BoardConfig.NumDmaWriteEngines == 1)
						{
							pDevExt->BoardConfig.FirstDmaWriteEngine = dmaNum;
						}
					}
					else  // If not assume it is a Card To System DMA Engine
					{
						// C2S DMA engine
						dmaDirection = WdfDmaDirectionReadFromDevice;
						if (++pDevExt->BoardConfig.NumDmaReadEngines == 1)
						{
							pDevExt->BoardConfig.FirstDmaReadEngine = dmaNum;
						}
					}

					// If the DMA Engine Device extension has not been created
					if (pDevExt->pDmaEngineDevExt[dmaNum] == NULL)
					{
						// create it
						pDevExt->pDmaEngineDevExt[dmaNum] =
							(PDMA_ENGINE_DEVICE_EXTENSION) ExAllocatePoolWithTag(NonPagedPool,
							sizeof(DMA_ENGINE_DEVICE_EXTENSION),
							'amDP');
						if (pDevExt->pDmaEngineDevExt[dmaNum] != NULL)
						{
							pDmaExt = pDevExt->pDmaEngineDevExt[dmaNum];
							pDevExt->pDmaExtMSIVector[pDevExt->NumberDMAEngines] = pDmaExt;

							// initialize the structure
							pDmaExt->TransactionQueue 					= NULL;
							pDmaExt->DmaEnabler							= NULL;
							pDmaExt->DmaEnabler32BitOnly				= NULL;
							pDmaExt->DmaRequest 						= NULL;
							pDmaExt->DmaTransaction 					= NULL;
							pDmaExt->DescCommonBuffer 					= NULL;
							pDmaExt->pDescriptorBasePhysical.QuadPart	= 0;
							pDmaExt->pDescriptorBase 					= NULL;
							// Default to a single vector
							pDmaExt->DMAEngineMSIVector					= 0;

							// Packet Mode Specific variables
							pDmaExt->PMdl 								= NULL;
							pDmaExt->UserVa 							= NULL;
							pDmaExt->HeadSpinLock 						= 0;
							pDmaExt->TailSpinLock 						= 0;
							pDmaExt->pNextDescVirt 						= NULL;
							pDmaExt->PacketMode							= DMA_MODE_NOT_SET;
							pDmaExt->DMAEngineStatus					= 0;
							pDmaExt->bFreeRun							= FALSE;

							// initialize performance counters
							pDmaExt->BytesInLastSecond 					= 0;
							pDmaExt->BytesInCurrentSecond 				= 0;
							pDmaExt->HardwareTimeInLastSecond 			= 0;
							pDmaExt->HardwareTimeInCurrentSecond 		= 0;
							pDmaExt->DMAInactiveTime 					= 0;
							pDmaExt->IntsInLastSecond					= 0;
							pDmaExt->DPCsInLastSecond					= 0;

							// save the device direction
							pDmaExt->DmaEngine = dmaNum;
							pDmaExt->DmaDirection = dmaDirection;
							// Setup the pointer to this DMA Engines registers
							pDmaExt->pDmaEng = &pDevExt->pDmaRegisters->dmaEngine[dmaNum];

							// determine the maximum transfer size
							pDmaExt->MaximumTransferLength = (size_t)DMA_CAP_CARD_ADDR_SIZE(pDmaExt->pDmaEng->Capabilities);
							if (pDmaExt->MaximumTransferLength == 1)
							{
								// this is a fifo, use system max transfer length
								pDmaExt->MaximumTransferLength = pDevExt->MaximumDmaTransferLength;
							}
							// Go setup the driver IOCTL and internal queues
							status = ThorDaqDrvSetupQueues(pDevExt, pDmaExt);
							if (NT_SUCCESS(status))
							{
								// Go create the spnlocks and DMA Enabler objects
								status = ThorDaqDrvCreateDMAObjects(pDevExt, pDmaExt);
								if (NT_SUCCESS(status))
								{
									// Go create the Common PCI DMA Descriptor buffers
									status = ThorDaqDrvCreateDMADescBuffers(pDevExt, pDmaExt);
									if (NT_SUCCESS(status))
									{
										// Go setup the DPC for this DMA Engine
										status = ThorDaqDrvSetupDPC(pDevExt, pDmaExt);
										if (NT_SUCCESS(status))
										{	
											pDevExt->NumberDMAEngines++;

										}
										else
										{
											// ThorDaqDrvSetupDPC Failed
											ThorDaqDrvBoardDmaRelease(pDevExt, dmaNum);
										}
									}
									else
									{
										// ThorDaqDrvCreateDMADescBuffers Failed
										ThorDaqDrvBoardDmaRelease(pDevExt, dmaNum);
									}
								}
								else
								{
									// ThorDaqDrvCreateDMAObjects Failed
									ThorDaqDrvBoardDmaRelease(pDevExt, dmaNum);
								}
							}
							else // ThorDaqDrvSetupQueues Failed!
							{
								// ThorDaqDrvSetupQueues Failed
								ThorDaqDrvBoardDmaRelease(pDevExt, dmaNum);
							}
						}
						else // ExAllocatePoolWithTag Failed for Dma Engine Context
						{
							// ExAllocatePoolWithTag Failed
						}
					} 	//	if (pDevExt->pDmaEngineDevExt[dmaNum] == NULL)
				}  // if ((pDevExt->pDmaRegisters->dmaEngine[dmaNum].Capabilities  Found a valid DMA Engine
			} // for (dmaNum = 0; dmaNum < MAX_NUM_DMA_ENGINES; dmaNum++)
		}
		else  // Error-  pDevExt->pDmaRegister is NULL!
		{
			status = STATUS_DEVICE_CONFIGURATION_ERROR;
		}
	} // if ((pDevExt->NumberOfBARS > DMA_BAR) && ((pDevExt->BarType[DMA_BAR] == CmResourceTypeMemory)

	// See if we setup multiple Interrupt vectors
	if (pDevExt->NumIRQVectors > 1)
	{
		ULONG32 MSIVector = 0;

		// Now make sure we allocated enough MSI vectors
		if (pDevExt->NumIRQVectors >= pDevExt->NumberDMAEngines)
		{
			for (dmaNum = 0; dmaNum < MAX_NUM_DMA_ENGINES; dmaNum++)
			{
				pDmaExt = pDevExt->pDmaEngineDevExt[dmaNum];
				// Make sure this is a valid DMA Engine
				if (pDmaExt != NULL)
				{
					// If so, record the vector in each DMA Extension.
					pDmaExt->DMAEngineMSIVector = MSIVector;
					MSIVector++;
				}
			}
		}
	}

	// Retireve the Card status and the FPGA Version information from the Card.
	if (status == STATUS_SUCCESS)
	{
		pDevExt->BoardConfig.CardStatusInfo = pDevExt->pDmaRegisters->commonControl.ControlStatus;
		pDevExt->BoardConfig.DMABackEndCoreVersion = pDevExt->pDmaRegisters->commonControl.DMABackEndCoreVersion;
		pDevExt->BoardConfig.PCIExpressCoreVersion = pDevExt->pDmaRegisters->commonControl.PCIExpressCoreVersion;
		pDevExt->BoardConfig.UserVersion = pDevExt->pDmaRegisters->commonControl.UserVersion;
	}

	return status;
}
///////////////////////////
//
// ThorDaqDrvS2mmDmaInit
//
// Initialize BAR 1, 2, 3
//
///////////////////////////
NTSTATUS
	ThorDaqDrvS2mmDmaInit (
	PDEVICE_EXTENSION pDevExt
	)
{
	NTSTATUS						status = STATUS_SUCCESS;
	UINT8							barNum;
	ULONG							ch;
	ULONG							blk;
	ULONG							descr;
	PDMA_COMMON_CONTROL_STRUCT		pBar0Ctrl;
	PS2MM_DMA_DESCRIPTOR_STRUCT		pDescriptor;
	PS2MM_CTRL_STAT_STRUCT			pS2mmCtrl;
	PBAR3_MAP_STRUCT				pBar3;
	PS2MM_LAYER_EXTENSION			pS2mmExt;

	ULONG32							inHSizeByBytes;
	ULONG32							inHSizeByPixels;
	ULONG32							frameSize;
	ULONG32							frameIndexCap;
	//ULONG32							bytesPerBeat;
	ULONG32							hSize;
	ULONG32							vSize;

	// Setup Bar 0: enable user interrupt
	pBar0Ctrl = &(pDevExt->pDmaRegisters->commonControl);
	pBar0Ctrl->ControlStatus |= 0x0000003f; //set bit[4] and bit[5] to be 1.

	// Setup Bar 1
	barNum = 1;
	if ((pDevExt->NumberOfBARS > barNum) && ((pDevExt->BarType[barNum] == CmResourceTypeMemory) ||
		(pDevExt->BarType[barNum] == CmResourceTypeMemoryLarge)))
	{
		pDevExt->pBar1Descriptors = (PBAR1_DESCRIPTOR_MAP_STRUCT) pDevExt->BarVirtualAddress[barNum];
		if (pDevExt->pBar1Descriptors == NULL)
		{
			status = STATUS_DEVICE_CONFIGURATION_ERROR;
		}
	}

	// Setup Bar 2
	barNum = 2;
	if ((pDevExt->NumberOfBARS > barNum) && ((pDevExt->BarType[barNum] == CmResourceTypeMemory) ||
		(pDevExt->BarType[barNum] == CmResourceTypeMemoryLarge)))
	{
		pDevExt->pBar2Controls = (PBAR2_CONTROL_MAP_STRUCT) pDevExt->BarVirtualAddress[barNum];
		if (pDevExt->pBar2Controls == NULL)
		{
			status = STATUS_DEVICE_CONFIGURATION_ERROR;
		}
	}

	// Setup Bar 3
	barNum = 3;
	if ((pDevExt->NumberOfBARS > barNum) && ((pDevExt->BarType[barNum] == CmResourceTypeMemory) ||
		(pDevExt->BarType[barNum] == CmResourceTypeMemoryLarge)))
	{
		pDevExt->pBar3Map = (PBAR3_MAP_STRUCT) pDevExt->BarVirtualAddress[barNum];
		if (pDevExt->pBar3Map == NULL)
		{
			status = STATUS_DEVICE_CONFIGURATION_ERROR;
		}
	}

	pBar3 = pDevExt->pBar3Map;

	pBar3->globalGenCtrl.Layout.StopRun_FpgaRev = 0x08;

	//inHSizeByPixels = (pDevExt->HSize & 0x0000FFFC); // 0x40
	//inHSizeByBytes = 2 * inHSizeByPixels; // 0x80 be an integer multiple of 4
	//
	//frameSize = inHSizeByPixels * pDevExt->VSize;  // 0x1000
	//pDevExt->FrameSize = frameSize;
	//frameIndexCap = (0x10000000 / S2MM_DESCRS_PER_BLK / frameSize / 2);


	//inHSizeByPixels = pDevExt->HSize ;//(pDevExt->HSize & 0x0000FFFC); // 0x40

	//bytesPerBeat = 8;

	//hSize = 26472;//DFLIM HSIZE//(ULONG32)((constFactor + multFactor * inHSizeByPixels) * bytesPerBeat); 
	//vSize = 32;
	//inHSizeByBytes = hSize;//2 * inHSizeByPixels; // 0x80 be an integer multiple of 4
	////
	//frameSize = hSize * vSize;  // 0x1000
	//pDevExt->FrameSize = inHSizeByPixels * vSize;
	//frameIndexCap = (0x10000000 / S2MM_DESCRS_PER_BLK / (inHSizeByBytes*vSize));

	
	//bytesPerBeat = 8;
	hSize = pDevExt->HSize;//26472;//DFLIM HSIZE//(ULONG32)((constFactor + multFactor * inHSizeByPixels) * bytesPerBeat); 
	inHSizeByBytes = (USHORT)hSize;//2 * inHSizeByPixels; // 0x80 be an integer multiple of 4
	inHSizeByPixels = (inHSizeByBytes>>1);//pDevExt->pBar3Map->globalGenCtrl.Layout.ImgHSize + 1;//(pDevExt->HSize & 0x0000FFFC); // 0x40
	
	vSize = (USHORT)pDevExt->VSize;//32	//
	frameSize = hSize * vSize;  // 0x1000
	pDevExt->FrameSize = inHSizeByPixels * vSize;
	frameIndexCap = (0x10000000 / S2MM_DESCRS_PER_BLK / (inHSizeByBytes*vSize));

	if(pDevExt->pS2mmLayerExt == NULL)
	{
		pDevExt->pS2mmLayerExt =
			(PS2MM_LAYER_EXTENSION) ExAllocatePoolWithTag(NonPagedPool,
			sizeof(S2MM_LAYER_EXTENSION),
			'amDP');
	}

	pS2mmExt = pDevExt->pS2mmLayerExt;

	for(ch = 0; ch < S2MM_CHANNEL_PER_BOARD; ch++)
	{		
		pS2mmExt->IsFull = 0;
		pS2mmExt->BankHead = 0;
		pS2mmExt->BankTail = 0;
		//pS2mmExt->pS2mm[ch].CurrDescr = 0;
		pS2mmExt->AcqBufOffset = 0;
		pS2mmExt->OffsetsIndexHead = 0;
		pS2mmExt->OffsetsIndexTail = 0;
		pS2mmExt->IndexCap = (USHORT)((frameIndexCap > MAX_S2MM_BLKS_PER_CHANNEL) ? MAX_S2MM_BLKS_PER_CHANNEL : frameIndexCap);

		WdfSpinLockCreate(WDF_NO_OBJECT_ATTRIBUTES, &(pS2mmExt->pS2mm[ch].InterReadLock));

		pS2mmCtrl = &(pDevExt->pBar2Controls->ctrlCh[ch].ctrl);                                                                 
		pS2mmCtrl->SR0_CR0 = 0x52;// 0x12;


		pS2mmCtrl->ChainStartAddr = 0x0;
		pS2mmCtrl->ChainTailAddr = 0x510; //an invalid address to stop S2mm DMA
		pS2mmCtrl->ChainIrqThreshold = (USHORT)(pDevExt->FrameRate); //0x1;
		for(blk = 0; blk < pDevExt->FrameRate; blk++)
		{
			for(descr = 0; descr < S2MM_DESCRS_PER_BLK; descr++)
			{
				pDescriptor = &(pDevExt->pBar1Descriptors->descCh[ch].descBlk[blk].desc[descr]);
				pDescriptor->NxtDescPtr = (((blk + 1) % pS2mmExt->IndexCap) * sizeof(S2MM_DESC_BLK_STRUCT) +  sizeof(S2MM_DMA_DESCRIPTOR_STRUCT) * descr +  (ch<<16)); 
				pDescriptor->BuffAddr = pDevExt->DataBufferStartAddress + ((ch + 0) % 4) * pDevExt->DataChannelLength + (blk) * frameSize + descr * pDevExt->DataInterruptLength;
				pDescriptor->Usr_Cache = (0x3<<24); // should be defined as a constant somewhere.
				pDescriptor->VSize_Stride = ((pDevExt->VSize)<<19) | (inHSizeByBytes); //0x02000080
				pDescriptor->HSize = (inHSizeByBytes);
				pDescriptor->DescFlags = 0x0;
			}
		}
	}

	// Go setup the DPC for S2mm layer

	status = ThorDaqDrvSetS2mmDPC(pDevExt, pS2mmExt);
	if (!NT_SUCCESS(status))
	{	
		// ThorDaqDrvSetupDPC Failed
	}

	//ExFreePoolWithTag(pDevExt->pS2mmLayerExt, 'amDP');
	return status;
}

// ThorDaqDrvSetupQueue
//
// Sets up the WDF Driver IOCTL Queues depending on the type of DMA Engine
//
//
NTSTATUS ThorDaqDrvSetupQueues(
	IN PDEVICE_EXTENSION pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt
	)
{
	NTSTATUS				status = STATUS_SUCCESS;
	WDF_IO_QUEUE_CONFIG		ioQueueConfig;
	WDF_OBJECT_ATTRIBUTES	attributes;
	PQUEUE_CTX				pQueueCtx;

	// Check for Packet Mode Send
	if (((pDmaExt->pDmaEng->Capabilities & DMA_CAP_ENGINE_TYPE_MASK) & 
		DMA_CAP_PACKET_DMA) && (pDmaExt->DmaDirection == WdfDmaDirectionWriteToDevice))
	{
		// Processing callbacks - We do our own queue management
		WDF_IO_QUEUE_CONFIG_INIT(&ioQueueConfig, WdfIoQueueDispatchManual);

		pDmaExt->DmaType = DMA_TYPE_PACKET_SEND;
	}
	// If not check for Packet Mode Receive
	else if (((pDmaExt->pDmaEng->Capabilities & DMA_CAP_ENGINE_TYPE_MASK) & 
		DMA_CAP_PACKET_DMA) && (pDmaExt->DmaDirection == WdfDmaDirectionReadFromDevice))
	{
		// Processing callbacks - We do our own queue management
		WDF_IO_QUEUE_CONFIG_INIT(&ioQueueConfig, WdfIoQueueDispatchManual);

		pDmaExt->DmaType = DMA_TYPE_PACKET_RECV;
	}
	pDmaExt->bAddressablePacketMode = FALSE;
	if ((pDmaExt->pDmaEng->Capabilities & DMA_CAP_ENGINE_TYPE_MASK) & DMA_CAP_ADDRESSABLE_PACKET_DMA)
	{
		pDmaExt->bAddressablePacketMode = TRUE;
		if (pDmaExt->DmaType == DMA_TYPE_PACKET_RECV)
		{
			/* Setup the Cancel Request Handler */
			ioQueueConfig.EvtIoCanceledOnQueue = PacketReadRequestCancel;
		}
	}

	// Create Queue
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, QUEUE_CTX);

	status = WdfIoQueueCreate(pDevExt->Device,
		&ioQueueConfig,
		&attributes,
		&pDmaExt->TransactionQueue);
	if (NT_SUCCESS(status))
	{
		// Keep a pointer the Dma Device Extension
		pQueueCtx = QueueContext(pDmaExt->TransactionQueue);
		pQueueCtx->pDmaExt = pDmaExt;
	}
	else
	{
		// Create Queue Failed
	}
	return status;
}


// ThorDaqDrvCreateDMAObjects
//
// Sets up the WDF Driver Spinlocks and EnablerCreate objects
//
//
NTSTATUS ThorDaqDrvCreateDMAObjects(
	IN PDEVICE_EXTENSION pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt
	)
{
	NTSTATUS				status = STATUS_SUCCESS;
	WDF_DMA_ENABLER_CONFIG	dmaConfig;
	ULONG32					mapRegistersAllocated;
	ULONG32					maxFragmentLengthSupported;

	status = WdfSpinLockCreate(WDF_NO_OBJECT_ATTRIBUTES, &pDmaExt->HeadSpinLock);
	if (NT_SUCCESS(status))
	{
		status = WdfSpinLockCreate(WDF_NO_OBJECT_ATTRIBUTES, &pDmaExt->TailSpinLock);
		if (NT_SUCCESS(status))
		{
			pDmaExt->MaximumTransferLength = DMA_MAX_TRANSFER_LENGTH;

			// Initialize DMA Enabler Config, 64 bit version capable for Scatter/Gathers
			WDF_DMA_ENABLER_CONFIG_INIT(&dmaConfig, WdfDmaProfileScatterGather64,
				pDmaExt->MaximumTransferLength); 

			// create enabler
			status = WdfDmaEnablerCreate(pDevExt->Device,
				&dmaConfig,
				WDF_NO_OBJECT_ATTRIBUTES,
				&pDmaExt->DmaEnabler);
			if (NT_SUCCESS(status))
			{
				// check to see how many DMA map registers are allocated
				maxFragmentLengthSupported = (ULONG) WdfDmaEnablerGetFragmentLength(pDmaExt->DmaEnabler,
					pDmaExt->DmaDirection);
				// calculate the number of map Registers
				mapRegistersAllocated = BYTES_TO_PAGES(maxFragmentLengthSupported) + 1;

				// Set to our maximum default number of descriptors
				if (mapRegistersAllocated > DMA_NUM_DESCR)
				{
					mapRegistersAllocated = DMA_NUM_DESCR;
				}
				// set the number of DMA elements to be this count
				WdfDmaEnablerSetMaximumScatterGatherElements(pDmaExt->DmaEnabler, mapRegistersAllocated);

				// Setup descriptor information used to be DMA_NUM_DESCR
				pDmaExt->NumberOfDescriptors = mapRegistersAllocated;
				pDmaExt->NumberOfUsedDescriptors = 0;

				// Initialize DMA Enabler Config
				WDF_DMA_ENABLER_CONFIG_INIT(&dmaConfig,
					WdfDmaProfileScatterGather,
					pDmaExt->MaximumTransferLength); 

				// create enabler
				status = WdfDmaEnablerCreate(pDevExt->Device,
					&dmaConfig,
					WDF_NO_OBJECT_ATTRIBUTES,
					&pDmaExt->DmaEnabler32BitOnly);
			}
			else // WdfDmaEnablerCreate Failed!
			{
				// Create Enabler Failed
			}
		}
		else // WdfSpinLockCreate Failed!
		{
			// Create SpinLock for Queue Tail Failed
		}
	}
	else // WdfSpinLockCreate Failed!
	{
		// Create SpinLock for Queue Head Failed
	}
	return status;
}


// ThorDaqDrvCreateDMADescBuffers
//
// Creates the Common PCI Addressable buffer for use as the DMA Descriptors
//
//
NTSTATUS ThorDaqDrvCreateDMADescBuffers(
	IN PDEVICE_EXTENSION pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt
	)
{
	NTSTATUS				status = STATUS_SUCCESS;
	WDF_COMMON_BUFFER_CONFIG	commonBufferConfig;

	UNREFERENCED_PARAMETER(pDevExt);

	// define the descriptor space
	WDF_COMMON_BUFFER_CONFIG_INIT(&commonBufferConfig,
		DMA_DESCR_ALIGN_REQUIREMENT);

	// create it
	status = WdfCommonBufferCreateWithConfig(pDmaExt->DmaEnabler32BitOnly,
		(size_t) (sizeof(DMA_DESCRIPTOR_STRUCT) * pDmaExt->NumberOfDescriptors),
		&commonBufferConfig,
		WDF_NO_OBJECT_ATTRIBUTES,
		&pDmaExt->DescCommonBuffer);
	if (NT_SUCCESS(status))
	{
		// initialize the structure
		pDmaExt->pDescriptorBase = WdfCommonBufferGetAlignedVirtualAddress(pDmaExt->DescCommonBuffer);
		pDmaExt->pDescriptorBasePhysical = WdfCommonBufferGetAlignedLogicalAddress(pDmaExt->DescCommonBuffer);
#if defined(_AMD64_)
		// This is a work-around for an error where the descriptor memory is allocated in
		// above 4GB. The HW design requires the descriptors live in the first 4GB.
		// If we set ScatterGatherDuplex then Windows restricts the number of Map registers
		// to 256. We will take our chances with the allocate.
		if (pDmaExt->pDescriptorBasePhysical.HighPart & 0xffffffff)
		{
			ThorDaqDrvBoardDmaRelease(pDevExt, pDmaExt->DmaEngine);
			return STATUS_NO_MEMORY;
		}
#endif // defined(_AMD64_)||defined(_IA64_)
		// Setup the 'Next' pointers to start at the base.
		pDmaExt->pNextDescVirt = pDmaExt->pDescriptorBase;
		pDmaExt->pNextDescPhys = pDmaExt->pDescriptorBasePhysical;

		if (pDmaExt->DmaType == DMA_TYPE_PACKET_RECV)
		{
			// For the Packet Recieve get the DMA Adapter handle
			pDmaExt->pReadDmaAdapter = WdfDmaEnablerWdmGetDmaAdapter(pDmaExt->DmaEnabler,
				WdfDmaDirectionReadFromDevice);
		}
	}
	return status;
}


// ThorDaqDrvSetupDPC
//
// Sets up the WDF Driver DPC routine for this DMA Engine
//
//
NTSTATUS ThorDaqDrvSetupDPC(
	IN PDEVICE_EXTENSION pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt
	)
{
	NTSTATUS				status = STATUS_SUCCESS;
	WDF_DPC_CONFIG 			dpcConfig;
	PDPC_CTX				pDpcCtx;
	WDF_OBJECT_ATTRIBUTES 	dpcAttributes;

	// Is this a Packet Send DMA Engine?
	if (pDmaExt->DmaType == DMA_TYPE_PACKET_SEND)
	{
		WDF_DPC_CONFIG_INIT(&dpcConfig,
			PacketS2CDpc);
	}
	// Is this a Packet Receive DMA Engine?
	else if (pDmaExt->DmaType == DMA_TYPE_PACKET_RECV)
	{
		WDF_DPC_CONFIG_INIT(&dpcConfig,
			PacketC2SDpc);
	}
	else
	{
		// DMA Type not found
	}

	dpcConfig.AutomaticSerialization = FALSE;

	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&dpcAttributes, DPC_CTX);
	dpcAttributes.ParentObject = pDevExt->Device;

	status = WdfDpcCreate(&dpcConfig,
		&dpcAttributes,
		&pDmaExt->CompletionDpc);

	if (NT_SUCCESS(status)) 
	{
		// Keep a pointer the Dma Device Extension
		pDpcCtx = DPCContext(pDmaExt->CompletionDpc);
		pDpcCtx->pDmaExt = pDmaExt;
	}
	else
	{
		// Create Queue Failed
	}
	return status;
}

// ThorDaqDrvSetS2mmDPC
//
// Sets up the WDF Driver DPC routine for S2mm Layer
//
//
NTSTATUS ThorDaqDrvSetS2mmDPC(
	IN PDEVICE_EXTENSION pDevExt,
	IN PS2MM_LAYER_EXTENSION	pS2mmExt
	)
{
	NTSTATUS				status = STATUS_SUCCESS;
	WDF_DPC_CONFIG 			dpcConfig;
	PDPC_CTX				pDpcCtx;
	WDF_OBJECT_ATTRIBUTES 	dpcAttributes;

	WDF_DPC_CONFIG_INIT(&dpcConfig,
		S2mmInterruptDpc);

	dpcConfig.AutomaticSerialization = FALSE;

	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&dpcAttributes, DPC_CTX);
	dpcAttributes.ParentObject = pDevExt->Device;

	status = WdfDpcCreate(&dpcConfig,
		&dpcAttributes,
		&pS2mmExt->CompletionDpc);

	if (NT_SUCCESS(status)) 
	{
		// Keep a pointer the Dma Device Extension
		pDpcCtx = DPCContext(pS2mmExt->CompletionDpc);
		pDpcCtx->pS2mmExt = pS2mmExt;
	}
	else
	{
		// Create Queue Failed
	}
	return status;
}


// ThorDaqDrvBoardDmaRelease
//
// Releases the DMA Engine resources.
//
//
NTSTATUS
	ThorDaqDrvBoardDmaRelease(
	IN PDEVICE_EXTENSION pDevExt,
	IN	UCHAR			DmaEngNum
	)
{
	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;
	NTSTATUS				status = STATUS_SUCCESS;

	if (pDevExt->pDmaEngineDevExt[DmaEngNum] != NULL)
	{
		pDmaExt = pDevExt->pDmaEngineDevExt[DmaEngNum];

		if (pDmaExt->DescCommonBuffer != NULL)
		{
			// Free the Common Buffer
			// 	
			WdfObjectDelete(pDmaExt->DescCommonBuffer);
			pDmaExt->DescCommonBuffer = NULL;
		}
		if (pDmaExt->DmaTransaction != NULL)
		{
			// Free the Dma Transaction
			WdfObjectDelete(pDmaExt->DmaTransaction);
			pDmaExt->DmaTransaction = NULL;
		}
		if (pDmaExt->DmaEnabler32BitOnly != NULL)
		{
			// Free the Dma Enabler
			WdfObjectDelete(pDmaExt->DmaEnabler32BitOnly);
			pDmaExt->DmaEnabler32BitOnly = NULL;
		}
		if (pDmaExt->DmaEnabler != NULL)
		{
			// Free the Dma Enabler
			WdfObjectDelete(pDmaExt->DmaEnabler);
			pDmaExt->DmaEnabler = NULL;
		}
		if (pDmaExt->HeadSpinLock != 0)
		{
			// Free the SpinLock
			WdfObjectDelete(pDmaExt->HeadSpinLock);
			pDmaExt->HeadSpinLock = 0;
		}
		if (pDmaExt->TailSpinLock != 0)
		{
			// Free the SpinLock
			WdfObjectDelete(pDmaExt->TailSpinLock);
			pDmaExt->TailSpinLock = 0;
		}
		// Adjust the number of dma engine(s)
		if (pDmaExt->DmaDirection == WdfDmaDirectionWriteToDevice)
		{
			// S2C DMA engine
			if (--pDevExt->BoardConfig.NumDmaWriteEngines == 0)
			{
				pDevExt->BoardConfig.FirstDmaWriteEngine = 0;
			}
		}
		else
		{
			// C2S DMA engine
			if (--pDevExt->BoardConfig.NumDmaReadEngines == 0)
			{
				pDevExt->BoardConfig.FirstDmaReadEngine = 0;
			}
		}
		//if (pDevExt->pS2mmLayerExt->pS2mm.readSpinLock != 0)
		//{
		//	// Free the SpinLock
		//	WdfObjectDelete(pDevExt->pS2mmLayerExt->pS2mm.readSpinLock);
		//	pDevExt->pS2mmLayerExt->pS2mm.readSpinLock = 0;
		//}
		// The last thing we do is free the DMA Engine Context
		ExFreePoolWithTag(pDevExt->pDmaEngineDevExt[DmaEngNum], 'amDP');
		pDevExt->pDmaEngineDevExt[DmaEngNum] = NULL;
	}


	return status;
}


// ThorDaqDrvInitializeDeviceExtension
//
// This routine initializes all the data structures required by this device.
// This routine also creates the read, write and IOCtl queues
//
NTSTATUS
	ThorDaqDrvInitializeDeviceExtension(
	PDEVICE_EXTENSION pDevExt
	)
{
	NTSTATUS  			status = STATUS_SUCCESS;
	WDF_IO_QUEUE_CONFIG	ioQueueConfig;
	int					i;

	PAGED_CODE();

	pDevExt->BoardConfig.DriverVersionMajor = VER_MAJOR_NUM;
	pDevExt->BoardConfig.DriverVersionMinor = VER_MINOR_NUM;
	pDevExt->BoardConfig.DriverVersionSubMinor = VER_SUBMINOR_NUM;
	pDevExt->BoardConfig.DriverVersionBuildNumber = VER_BUILD_NUM;

	// Setup DMA structures
	pDevExt->BoardConfig.NumDmaWriteEngines	= 0;
	pDevExt->BoardConfig.FirstDmaWriteEngine = 0;
	pDevExt->BoardConfig.NumDmaReadEngines 	= 0;
	pDevExt->BoardConfig.FirstDmaReadEngine = 0;
	pDevExt->BoardConfig.NumDmaRWEngines 	= 0;
	pDevExt->BoardConfig.FirstDmaRWEngine  	= 0;
	pDevExt->pDmaRegisters = NULL;
	for (i = 0; i < MAX_NUM_DMA_ENGINES; i++)
	{
		pDevExt->pDmaEngineDevExt[i] 		= NULL;
		pDevExt->pDmaExtMSIVector[i]		= NULL;
		pDevExt->Interrupt[i]				= NULL;
	}

	// Default to not supporting MSI/MSI-X Intterupts
	pDevExt->MSISupported 					= FALSE;
	pDevExt->MSIXSupported 					= FALSE;
	pDevExt->NumIRQVectors					= 0;
	pDevExt->MSINumberVectors 				= MAX_NUM_DMA_ENGINES;

	// Count how many DMA Engines this firmware supports
	pDevExt->NumberDMAEngines				= 0;

	pDevExt->WatchdogTimer 					= NULL;

	// for now, default this
	pDevExt->MaximumDmaTransferLength		= DMA_MAX_TRANSFER_LENGTH;

	// Default HSize VSize and FrameRate
	pDevExt->HSize = 128;//13236; //hsize for s2mm image
	pDevExt->VSize = 32;
	pDevExt->FrameRate = 256;

	// Default channel information No channel is enabled
	pDevExt ->Channel = 0x0000;

	// Default Data Buffer Start Address
	pDevExt ->DataBufferStartAddress = 0x00000000;
	pDevExt ->DataChannelLength      = ACQ_SINGLE_CHANNEL_BUF_CAP;
	pDevExt ->DataInterruptLength    = ACQ_SINGLE_CHANNEL_BUF_CAP * S2MM_CHANNEL_PER_BOARD;

	for (i = 0; i < S2MM_CHANNEL_PER_BOARD; i++)
	{
		pDevExt->ChannelDescriptor[i] = FALSE;
	}


	pDevExt->InterruptMode 					= PACKET_DMA_INT_CTRL_INT_EOP;

	// Check the registry for any initialization overrides
	ThorDaqDrvGetRegistryInfo(pDevExt);

	// initialize IOCTL Queue
	// - Default Queue
	// - Parallel Processing
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig, WdfIoQueueDispatchParallel);

	// Processing callbacks
	ioQueueConfig.EvtIoRead = ThorDaqDrvEvtIoRead;
	ioQueueConfig.EvtIoWrite = ThorDaqDrvEvtIoWrite;
	ioQueueConfig.EvtIoDeviceControl = ThorDaqDrvEvtIoDeviceControl;

	// Create Queue
	status = WdfIoQueueCreate(pDevExt->Device,
		&ioQueueConfig,
		WDF_NO_OBJECT_ATTRIBUTES,
		&pDevExt->IoctlQueue);
	if (!NT_SUCCESS(status)) 
	{
		return status;
	}

	ThorDaqDrvGetRegHardwareInfo(pDevExt);

	// Setup the PCI Bus interface
	status = WdfFdoQueryForInterface(pDevExt->Device,
		&GUID_BUS_INTERFACE_STANDARD,
		(PINTERFACE) &pDevExt->BusInterface,
		sizeof(BUS_INTERFACE_STANDARD),
		1,		// version
		NULL	// info
		);
	if (!NT_SUCCESS(status)) 
	{
		return status;
	}

	// Setup the board Config Structure
	status = ThorDaqDrvBoardConfigInit(pDevExt);
	if (!NT_SUCCESS(status)) 
	{
		return status;
	}

	// Setup Interrupt Object
	status = ThorDaqDrvInterruptCreate(pDevExt);
	if (!NT_SUCCESS(status)) 
	{
		return status;
	}

	return status;
}

// ThorDaqDrvPrepareHardware
//
// This routine scans the device for resources and maps the memory areas.
NTSTATUS
	ThorDaqDrvPrepareHardware (
	IN PDEVICE_EXTENSION 	pDevExt,
	IN WDFCMRESLIST			ResourcesTranslated
	)
{
	NTSTATUS	status = STATUS_SUCCESS;

	PAGED_CODE();

	status = ThorDaqDrvScanPciResources (pDevExt, ResourcesTranslated);
	if (!NT_SUCCESS(status)) 
	{
		return status;
	}

	// Setup the board DMA Resources
	status = ThorDaqDrvBoardDmaInit(pDevExt);
	if (!NT_SUCCESS(status)) 
	{
		return status;
	}

	// initialize BAR 1, 2, 3
	status = ThorDaqDrvS2mmDmaInit(pDevExt);
	if (!NT_SUCCESS(status)) 
	{
		return status;
	}

	return status;
}

// ThorDaqDrvScanPciResources
//
// This routine scans the device for resources and maps the memory areas.
NTSTATUS
	ThorDaqDrvScanPciResources (
	IN PDEVICE_EXTENSION 	pDevExt,
	IN WDFCMRESLIST			ResourcesTranslated
	)
{
	NTSTATUS	status = STATUS_SUCCESS;
	PCM_PARTIAL_RESOURCE_DESCRIPTOR		descriptor;
	UINT8		bar;
	ULONG32		i;
	UCHAR		barSize;
	ULONG64		barLength;
	BOOLEAN		AddressSizeDetected = FALSE;

	PAGED_CODE();

	// Setup prior to scanning hardware resources
	pDevExt->NumberOfBARS = 0;
	pDevExt->Use64BitAddresses = FALSE;

	// search through the resources
	for (i = 0; i < WdfCmResourceListGetCount(ResourcesTranslated); i++)
	{
		descriptor = WdfCmResourceListGetDescriptor(ResourcesTranslated, i);
		if (!descriptor) 
		{
			return STATUS_DEVICE_CONFIGURATION_ERROR;
		}

		switch (descriptor->Type) 
		{
		case CmResourceTypePort:
			bar = pDevExt->NumberOfBARS;
			if (bar < MAX_NUMBER_OF_BARS)
			{
				// Save IO Space info
				pDevExt->NumberOfBARS++;
				pDevExt->BarType[bar] = CmResourceTypePort;
				pDevExt->BarPhysicalAddress[bar] = descriptor->u.Port.Start;
				pDevExt->BarLength[bar] = descriptor->u.Port.Length;
				pDevExt->BarVirtualAddress[bar] = NULL;
				// Save in BoardCfg space
				pDevExt->BoardConfig.PciConfig.BarCfg[bar] = BAR_CFG_BAR_PRESENT | BAR_CFG_BAR_TYPE_IO;
				// compute and store bar size in (2**n)
				barSize = 0;
				barLength = pDevExt->BarLength[bar]-1;
				while (barLength > 0)
				{
					barSize++;
					barLength >>= 1;
				}
				pDevExt->BoardConfig.PciConfig.BarCfg[bar] |= ((ULONGLONG)barSize << BAR_CFG_BAR_SIZE_OFFSET);
			}
			break;

		case CmResourceTypeMemory:
			bar = pDevExt->NumberOfBARS;
			if (bar < MAX_NUMBER_OF_BARS)
			{
				// Save Memory Space info
				pDevExt->NumberOfBARS++;
				pDevExt->BarType[bar] = CmResourceTypeMemory;
				pDevExt->BarPhysicalAddress[bar] = descriptor->u.Memory.Start;
				pDevExt->BarLength[bar] = descriptor->u.Memory.Length;
				// set system bar size
				if (AddressSizeDetected == FALSE)
				{
					AddressSizeDetected = TRUE;
					pDevExt->Use64BitAddresses = FALSE;
				}
				// map the memory
				pDevExt->BarVirtualAddress[bar] = MmMapIoSpace(descriptor->u.Memory.Start,
					descriptor->u.Memory.Length,
					MmNonCached);
				// Save in BoardCfg space
				pDevExt->BoardConfig.PciConfig.BarCfg[bar] = BAR_CFG_BAR_PRESENT | BAR_CFG_BAR_TYPE_MEMORY;
				if ((descriptor->Flags & CM_RESOURCE_MEMORY_PREFETCHABLE) == CM_RESOURCE_MEMORY_PREFETCHABLE)
				{
					pDevExt->BoardConfig.PciConfig.BarCfg[bar] |= BAR_CFG_MEMORY_PREFETCHABLE;
				}
				// compute and store bar size in (2**n)
				barSize = 0;
				barLength = pDevExt->BarLength[bar]-1;
				while (barLength > 0)
				{
					barSize++;
					barLength >>= 1;
				}
				pDevExt->BoardConfig.PciConfig.BarCfg[bar] |= ((ULONGLONG)barSize << BAR_CFG_BAR_SIZE_OFFSET);
			}
			break;

		case CmResourceTypeMemoryLarge:
			bar = pDevExt->NumberOfBARS;
			if (bar < MAX_NUMBER_OF_BARS)
			{
				// Save Memory Space info
				pDevExt->NumberOfBARS++;
				pDevExt->BarType[bar] = CmResourceTypeMemoryLarge;
				if ((descriptor->Flags & CM_RESOURCE_MEMORY_LARGE_40) == CM_RESOURCE_MEMORY_LARGE_40)
				{
					pDevExt->BarPhysicalAddress[bar] = descriptor->u.Memory40.Start;
					pDevExt->BarLength[bar] = descriptor->u.Memory40.Length40;
				}
				else if ((descriptor->Flags & CM_RESOURCE_MEMORY_LARGE_48) == CM_RESOURCE_MEMORY_LARGE_48)
				{
					pDevExt->BarPhysicalAddress[bar] = descriptor->u.Memory48.Start;
					pDevExt->BarLength[bar] = descriptor->u.Memory48.Length48;
				}
				else if ((descriptor->Flags & CM_RESOURCE_MEMORY_LARGE_64) == CM_RESOURCE_MEMORY_LARGE_64)
				{
					pDevExt->BarPhysicalAddress[bar] = descriptor->u.Memory64.Start;
					pDevExt->BarLength[bar] = descriptor->u.Memory64.Length64;
				}
				else
				{
					// Error!
					pDevExt->NumberOfBARS--;
					pDevExt->BarVirtualAddress[bar] = NULL;
					break;
				}
				// set system bar size
				if (AddressSizeDetected == FALSE)
				{
					AddressSizeDetected = TRUE;
					pDevExt->Use64BitAddresses = TRUE;
				}
				// map the memory
				pDevExt->BarVirtualAddress[bar] = MmMapIoSpace(descriptor->u.Memory.Start,
					descriptor->u.Memory.Length,
					MmNonCached);

				// Save in BoardCfg space
				pDevExt->BoardConfig.PciConfig.BarCfg[bar] = BAR_CFG_BAR_PRESENT | BAR_CFG_BAR_TYPE_MEMORY |
					BAR_CFG_MEMORY_64BIT_CAPABLE;
				if ((descriptor->Flags & CM_RESOURCE_MEMORY_PREFETCHABLE) == CM_RESOURCE_MEMORY_PREFETCHABLE)
				{
					pDevExt->BoardConfig.PciConfig.BarCfg[bar] |= BAR_CFG_MEMORY_PREFETCHABLE;
				}
				// compute and store bar size in (2**n)
				barSize = 0;
				barLength = pDevExt->BarLength[bar]-1;
				while (barLength > 0)
				{
					barSize++;
					barLength >>= 1;
				}
				pDevExt->BoardConfig.PciConfig.BarCfg[bar] |= ((ULONGLONG)barSize << BAR_CFG_BAR_SIZE_OFFSET);
			}
			break;

		case CmResourceTypeInterrupt:
			if ((descriptor->Flags & (CM_RESOURCE_INTERRUPT_MESSAGE | CM_RESOURCE_INTERRUPT_LATCHED)) == 
				(CM_RESOURCE_INTERRUPT_MESSAGE | CM_RESOURCE_INTERRUPT_LATCHED))
			{
			}

		default:
			// Ignore the other resources
			break;
		}
	}
	return status;
}

// ThorDaqDrvBoardConfigInit
//
// Get the PCI Configuration from the card and initialize the
// BoardConfig structure.
// Some of the BoardConfig structure is configured when the resources
// are being initialized.
NTSTATUS
	ThorDaqDrvBoardConfigInit (
	PDEVICE_EXTENSION pDevExt
	)
{
	NTSTATUS	status = STATUS_SUCCESS;
	int			size;
	UCHAR *		pPCIConfSpace;
	UCHAR		offset;
	UCHAR		MessageID;
	PPCI_MSIX_CAPABILITY	pPciMSIXCapabilities;
	USHORT		MsgCtrl;

	// Get the common config data
	size = pDevExt->BusInterface.GetBusData(
		pDevExt->BusInterface.Context,
		PCI_WHICHSPACE_CONFIG,
		&pDevExt->BoardConfig.PciConfig,
		0,			// offset
		sizeof(PCI_CONFIG_HEADER));

	// pDevExt->BoardConfig.*Dma* are initialized separately

	// Walk through the device specific fields of the PCI Config space looking for MSI and MSIX info
	if (pDevExt->BoardConfig.PciConfig.CapabilitiesPtr != 0)
	{
		pPCIConfSpace = (UCHAR *)&pDevExt->BoardConfig.PciConfig;
		offset = pDevExt->BoardConfig.PciConfig.CapabilitiesPtr;
		do
		{
			MessageID = pPCIConfSpace[offset];

			if (MessageID == PCI_CAPABILITY_ID_MSI)
			{
				pDevExt->MSIXSupported = TRUE;
			}

			// Did we find the MSIX Message entry?
			if (MessageID == PCI_CAPABILITY_ID_MSIX)
			{
				pPciMSIXCapabilities = (PPCI_MSIX_CAPABILITY)&pPCIConfSpace[offset];
				// This is the MSIX Config info, offset is pointing to the Message Control word
				MsgCtrl = pPciMSIXCapabilities->MessageControl;
				pDevExt->MSIXSupported = TRUE;
				// If the number of vectors is still set to default, use the table size instead.
				if (pDevExt->MSINumberVectors == MAX_NUM_DMA_ENGINES)
				{
					pDevExt->MSINumberVectors = (MsgCtrl & MSG_CTRL_TABLE_SIZE_MASK) + 1; // take bit[10:0] and plus 1.
				}
			}
			offset++;
			offset = pPCIConfSpace[offset];
		} while ((offset != 0x00) && (offset < sizeof(PCI_CONFIG_HEADER)));
	}
	return status;
}


/*!***************************************************************************
*
* 	\brief ThorDaqDrvGetRegHardwareInfo - Retieve information about the
*		driver hardware parameters from the registry (if any)
* 
* NOTE: Windows XP does not support MSI / MSI-X interrupts. In fact it will mistakenly succeed
*	 in allocation multiple interrupts
*
*   \param pDevExt 	Pointer to the per instance data store for the driver
* 
*   \return status (NTSTATUS) - STATUS_SUCCESS if all goes well
*
*****************************************************************************/
NTSTATUS
	ThorDaqDrvGetRegHardwareInfo (
	PDEVICE_EXTENSION pDevExt
	)
{
	NTSTATUS	status = STATUS_SUCCESS;
	WDFKEY 		hKey;
	WDFKEY 		subkey;
	WDFKEY 		subsubkey;
	ULONG  		MSISupported;
	ULONG  		MSILimit;

	// Open the HKLM/SYSTEM/CurrentControlSet/Enum/VEN_xxxx...\Device Parameters
	status = WdfDeviceOpenRegistryKey(pDevExt->Device,
		PLUGPLAY_REGKEY_DEVICE, 
		STANDARD_RIGHTS_ALL,
		WDF_NO_OBJECT_ATTRIBUTES, 
		&hKey);
	if (status == STATUS_SUCCESS)
	{
		// Now open the Interrupt Management Sub Key
		status = WdfRegistryOpenKey(hKey,
			&InterruptManagementName,
			KEY_READ,
			WDF_NO_OBJECT_ATTRIBUTES,
			&subkey);
		if (status == STATUS_SUCCESS)
		{
			// Open the MessageSignaledInterruptProperties sub key
			status = WdfRegistryOpenKey(subkey,	
				&MSIPropertiesName,
				KEY_READ,
				WDF_NO_OBJECT_ATTRIBUTES,
				&subsubkey);
			if (status == STATUS_SUCCESS)
			{
				// See if there is a MSISupported entry
				status = WdfRegistryQueryULong(subsubkey, &MSISupportedName, &MSISupported);
				if (status == STATUS_SUCCESS)
				{
					// This enables the mode or not.
					if (MSISupported)
					{
						pDevExt->MSISupported = TRUE;
					}
					// Now see if there is a limit to the number of vectors
					status = WdfRegistryQueryULong(subsubkey, &MSILimitName, &MSILimit);
					if (status == STATUS_SUCCESS)
					{
						if (MSILimit > 0)
						{
							pDevExt->MSINumberVectors = MSILimit;
						}
						else
						{
							pDevExt->MSINumberVectors = 1;
						}
					}
				}
			}
		}
	}
	return (status);
}


/*!***************************************************************************
*
* 	\brief ThorDaqDrvGetRegistryInfo - Retieve information about the
*		driver parameters from the registry (if any)
* 
*   \param pDevExt 	Pointer to the per instance data store for the driver
* 
*   \return status (NTSTATUS) - STATUS_SUCCESS if all goes well
*
*****************************************************************************/
NTSTATUS
	ThorDaqDrvGetRegistryInfo (
	PDEVICE_EXTENSION pDevExt
	)
{
	NTSTATUS	status = STATUS_SUCCESS;
	WDFKEY 		hKey;
	ULONG		InterruptMode;

	// Open the Registry for our entry
	status = WdfDriverOpenParametersRegistryKey(WdfGetDriver(),
		STANDARD_RIGHTS_ALL,
		WDF_NO_OBJECT_ATTRIBUTES,
		&hKey);

	// Get the Interrupt Mode 
	if (WdfRegistryQueryULong(hKey, &InterruptModeName, &InterruptMode) == STATUS_SUCCESS)
	{
		pDevExt->InterruptMode = InterruptMode;
	}

	WdfRegistryClose(hKey);

	return (status);
}

