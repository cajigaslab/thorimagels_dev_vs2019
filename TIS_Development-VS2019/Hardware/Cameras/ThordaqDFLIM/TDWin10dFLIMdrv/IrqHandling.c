#include "Driver.h"
#include "Trace.h"

// ThorDaqDrvInterruptCreate
//
// Create and Initialize the Interrupt object
// This is called (indirectly) from the ThorDaqDrvEvtDeviceAdd.
NTSTATUS
	ThorDaqDrvInterruptCreate (
	IN PDEVICE_EXTENSION 	pDevExt
	)
{
	NTSTATUS 	status = STATUS_SUCCESS;
	WDF_INTERRUPT_CONFIG interruptConfig;
	ULONG32		i;

	if (pDevExt->MSIXSupported && (pDevExt->MSINumberVectors > 1))
	{
		for (i = 0; i < pDevExt->MSINumberVectors; i++)
		{
			// setup interrupt config structure with ISR and DPC pointers
			WDF_INTERRUPT_CONFIG_INIT(&interruptConfig,
				ThorDaqDrvInterruptMSIIsr, NULL);

			// Setup Interrupt enable/disable routine callbacks
			interruptConfig.EvtInterruptEnable = ThorDaqDrvInterruptEnable;
			interruptConfig.EvtInterruptDisable = ThorDaqDrvInterruptDisable;

			// Create interrupt object
			status = WdfInterruptCreate(pDevExt->Device,
				&interruptConfig,
				WDF_NO_OBJECT_ATTRIBUTES,
				&pDevExt->Interrupt[i]);
			if (!NT_SUCCESS(status)) 
			{
				break;
			}
			pDevExt->NumIRQVectors++;
		}
	}
	else
	{
		// setup interrupt config structure with ISR and DPC pointers
		WDF_INTERRUPT_CONFIG_INIT(&interruptConfig,
			ThorDaqDrvInterruptIsr, NULL);

		// Setup Interrupt enable/disable routine callbacks
		interruptConfig.EvtInterruptEnable = ThorDaqDrvInterruptEnable;
		interruptConfig.EvtInterruptDisable = ThorDaqDrvInterruptDisable;

		// Create interrupt object
		status = WdfInterruptCreate(pDevExt->Device,
			&interruptConfig,
			WDF_NO_OBJECT_ATTRIBUTES,
			&pDevExt->Interrupt[0]);
		if (!NT_SUCCESS(status)) 
		{
			return status;
		}
		pDevExt->NumIRQVectors++;
	}

	// Make sure we allocated at least one Interrupt vector
	if (pDevExt->NumIRQVectors)
	{
		status = STATUS_SUCCESS;
	}
	return status;
}


// ThorDaqDrvInterruptIsr
//
// Interrupt Handler.
// Determine if this is our interrupt.  If so, save the status and schedule a DPC.
BOOLEAN
	ThorDaqDrvInterruptIsr (
	IN WDFINTERRUPT	Interrupt,
	IN ULONG		MessageID
	)
{
	PDEVICE_EXTENSION	pDevExt;
	volatile ULONG32 	Status;
	int	dmaEngine;
	BOOLEAN IsOurInterrupt = FALSE;

	UNREFERENCED_PARAMETER(MessageID);

	// Get Device Extensions
	pDevExt = ThorDaqDrvGetDeviceContext(WdfInterruptGetDevice(Interrupt));

	// search for active dmaEngine
	for (dmaEngine = 0; dmaEngine < MAX_NUM_DMA_ENGINES; dmaEngine++)
	{
		PDMA_ENGINE_DEVICE_EXTENSION    pDmaExt;

		pDmaExt = pDevExt->pDmaEngineDevExt[dmaEngine];
		if (pDmaExt != NULL)
		{
			Status = pDmaExt->pDmaEng->ControlStatus;

			if ((Status & COMMON_DMA_CTRL_IRQ_ACTIVE) &&
				(Status & COMMON_DMA_CTRL_IRQ_ENABLE))
			{
				// Inc the Interrupt Count
				pDmaExt->IntsInLastSecond++;

				IsOurInterrupt = TRUE;
				WdfDpcEnqueue(pDmaExt->CompletionDpc);
			}
		}
	}

	return IsOurInterrupt;
}


// ThorDaqDrvInterruptMSIIsr
//
// MSI Interrupt Handler.
// Determine if this is our interrupt.  If so, save the status and schedule a DPC.
BOOLEAN ThorDaqDrvInterruptMSIIsr (
	IN WDFINTERRUPT	Interrupt,
	IN ULONG		MessageID
	)
{
	PDEVICE_EXTENSION	pDevExt;
	PDMA_ENGINE_DEVICE_EXTENSION    pDmaExt;

	USHORT							intr_sum;
	UCHAR							l;
	UCHAR                           ch;
	BOOLEAN                         channel_activated;

	volatile ULONG32 	Status;
	BOOLEAN IsOurInterrupt = FALSE;
	PBAR3_MAP_STRUCT pPacketGen;
	DEBUGP(DEBUG_INFO, "...Enter ISR");
	// Get Device Extensions
	pDevExt = ThorDaqDrvGetDeviceContext(WdfInterruptGetDevice(Interrupt));
	pPacketGen = (PBAR3_MAP_STRUCT)(pDevExt->pBar3Map);
	DEBUGP(DEBUG_INFO, "             read status GP0Reg0(MS) lines_written 0x%d, CntRdy %d,  Cmplt %d CntSmpClk %d", (int)((pPacketGen->dflimProcessingRegs[0].Layout.GP0Reg0 >> 48) & 0xFFFF), 
		                                                                (int)(pPacketGen->dflimProcessingRegs[0].Layout.GP0Reg0>>32) & 0x1, 
																		(int)(pPacketGen->dflimProcessingRegs[0].Layout.GP0Reg0 >> 33) & 0x1,
																		(int)(pPacketGen->dflimProcessingRegs[0].Layout.GP0Reg0 ) & 0xffff);
	if(MessageID < 2)
	{	
		pDmaExt = pDevExt->pDmaExtMSIVector[MessageID];
		if (pDmaExt != NULL)
		{
			Status = pDmaExt->pDmaEng->ControlStatus;
			// Inc the Interrupt Count
			pDmaExt->IntsInLastSecond++;
			DEBUGP(DEBUG_INFO, " Status 0x%x", Status);

			if ((Status & COMMON_DMA_CTRL_IRQ_ACTIVE) && (Status & COMMON_DMA_CTRL_IRQ_ENABLE))
			{
				WdfDpcEnqueue(pDmaExt->CompletionDpc);
			}
			IsOurInterrupt = TRUE;
		}
	}
	else if(MessageID == 2)//User Interrupt 
	{
		for(l = 0; l < 600; l++) // software timing loop@! was 100 iterations
		{
			intr_sum = 0;
			channel_activated = FALSE;
			for (ch = 0; ch < S2MM_CHANNEL_PER_BOARD; ch++)
			{
				if (pDevExt->ChannelDescriptor[ch] == TRUE)
				{
					if ((pDevExt->pBar2Controls->ctrlCh[ch].ctrl.SR0_CR0 & S2MM_DMA_INTERRUPT_COMPLETE_MASK) == S2MM_DMA_INTERRUPT_COMPLETE_MASK)
					{
						intr_sum++;
						channel_activated = TRUE;
						DEBUGP(DEBUG_INFO, " ch %d actvtd: itr%d  ", ch, l);
					}
				}else
				{
					intr_sum++;
				}
			}
			if(intr_sum == S2MM_CHANNEL_PER_BOARD && channel_activated == TRUE)  
			{
				for (ch = 0; ch < S2MM_CHANNEL_PER_BOARD; ch++)
				{
					if (pDevExt->ChannelDescriptor[ch] == TRUE)
					{
						pDevExt->pS2mmLayerExt->pS2mm[ch].IsDataReady = 1;   // if ANY channel "Activated" this assumes ALL activated
					}
				}
				pDevExt->pS2mmLayerExt->AcqBufOffset = pDevExt->pS2mmLayerExt->BankTail * pDevExt->DDR3startAddressFor2ndPingPongBank;
				pDevExt->pS2mmLayerExt->BankTail = (pDevExt->pS2mmLayerExt->BankTail + 1) % S2MM_DESCRS_PER_BLK;
				DEBUGP(DEBUG_INFO, ".IRQ AcqBufOffset 0x%x, Tail %d.", pDevExt->pS2mmLayerExt->AcqBufOffset, pDevExt->pS2mmLayerExt->BankTail);
				IsOurInterrupt = TRUE;
				break;
			}
		}
	}
	DEBUGP(DEBUG_INFO, " ourIrq %d Exit ISR... ", IsOurInterrupt);
	return IsOurInterrupt;
}


// ThorDaqDrvInterruptEnable
//
// Called at DIRQL after EvtDeviceD0Entry returns or
// when WdfInterruptEnable is called.
// The interrupt spinlock is being held during this call.
//
// This enables global device interrupts.
// DMA Engine interrupts are not enabled at this point.
NTSTATUS
	ThorDaqDrvInterruptEnable (
	IN WDFINTERRUPT	Interrupt,
	IN WDFDEVICE	Device
	)
{
	PDEVICE_EXTENSION	pDevExt;

	UNREFERENCED_PARAMETER(Interrupt);

	// Log the Enable request

	pDevExt = ThorDaqDrvGetDeviceContext(Device);
	// Enable interrupts
	pDevExt->pDmaRegisters->commonControl.ControlStatus |= CARD_IRQ_ENABLE;
	return STATUS_SUCCESS;
}


// ThorDaqDrvInterruptDisable
//
// Called at DIRQL before EvtDeviceD0Exit is called or
// when WdfInterruptDisable is called.
// The interrupt spinlock is being held during this call.
//
// This disables global device interrupts.
// DMA Engine interrupts are not disabled at this point.
NTSTATUS
	ThorDaqDrvInterruptDisable (
	IN WDFINTERRUPT	Interrupt,
	IN WDFDEVICE	Device
	)
{
	PDEVICE_EXTENSION	pDevExt;

	UNREFERENCED_PARAMETER(Interrupt);

	// Log the disable request

	pDevExt = ThorDaqDrvGetDeviceContext(Device);
	// Disable interrupts
	pDevExt->pDmaRegisters->commonControl.ControlStatus = 0;

	return STATUS_SUCCESS;
}
