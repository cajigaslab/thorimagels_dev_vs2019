// -------------------------------------------------------------------------
//
// PRODUCT:			DMA Driver
// MODULE NAME:		IrqHandling.c
//
// MODULE DESCRIPTION:
//
// Contains the code for Interrupt Handling for the PCI Express DMA Driver.
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
#include "IrqHandling.tmh"
#endif // TRACE_ENABLED

/*! DMADriverInterruptCreate
 *
 * \brief Create and Initialize the Interrupt object
 *  This is called (indirectly) from the DMADriverEvtDeviceAdd.
 * \param pDevExt
 * \return status
 */
NTSTATUS
DMADriverInterruptCreate (
	IN PDEVICE_EXTENSION 		pDevExt
)
{
	NTSTATUS 					status = STATUS_SUCCESS;
	WDF_INTERRUPT_CONFIG		interruptConfig;
	UINT32						i;

	DEBUGP(DEBUG_INFO, "--> WdfInterruptCreate");
	pDevExt->NumIRQVectors = 0;

	if (pDevExt->MSIXSupported && (pDevExt->MSINumberVectors > 1))
	{
		DEBUGP(DEBUG_INFO, "Setting up %d MSI Multiple vectors", pDevExt->MSINumberVectors);
		for (i = 0; i < pDevExt->MSINumberVectors; i++)
		{
			// setup interrupt config structure with ISR and DPC pointers
			WDF_INTERRUPT_CONFIG_INIT(&interruptConfig,
				DMADriverInterruptMSIIsr, NULL);

			// Setup Interrupt enable/disable routine callbacks
			interruptConfig.EvtInterruptEnable = DMADriverInterruptEnable;
			interruptConfig.EvtInterruptDisable = DMADriverInterruptDisable;

			// Create interrupt object
			status = WdfInterruptCreate(pDevExt->Device,
				&interruptConfig,
				WDF_NO_OBJECT_ATTRIBUTES,
				&pDevExt->Interrupt[i]);
			if (!NT_SUCCESS(status))
			{
				DEBUGP(DEBUG_ERROR, "<-- WdfInterruptCreate failed  0x%x", status);
				break;
			}

			pDevExt->NumIRQVectors++;
		}
		// User Interrupt
		WDF_INTERRUPT_CONFIG_INIT(&interruptConfig,
		DMADriverInterruptUserIRQMSIIsr, NULL);

		// Create interrupt object

		status = WdfInterruptCreate(pDevExt->Device,
			&interruptConfig,
			WDF_NO_OBJECT_ATTRIBUTES,
			&pDevExt->Interrupt[pDevExt->NumIRQVectors]);
		if (!NT_SUCCESS(status))
		{
			DEBUGP(DEBUG_ERROR, "<-- WdfInterruptCreate failed  0x%x\n", status);
		}
		pDevExt->NumIRQVectors++;
		// End User Interrupt
	}
	else
	{
		DEBUGP(DEBUG_INFO, "Setting up single interrupt handler");
		// setup interrupt config structure with ISR and DPC pointers
		WDF_INTERRUPT_CONFIG_INIT(&interruptConfig,
								  DMADriverInterruptIsr, NULL);

		// Setup Interrupt enable/disable routine callbacks
		interruptConfig.EvtInterruptEnable = DMADriverInterruptEnable;
		interruptConfig.EvtInterruptDisable = DMADriverInterruptDisable;

		// Create interrupt object
		status = WdfInterruptCreate(pDevExt->Device,
									&interruptConfig,
									WDF_NO_OBJECT_ATTRIBUTES,
									&pDevExt->Interrupt[0]);
		if (!NT_SUCCESS(status))
		{
			DEBUGP(DEBUG_INFO,	"<-- WdfInterruptCreate failed  0x%x", status);
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

/*! DMADriverInterruptIsr
 *
 * \brief Interrupt Handler.
 *  Determine if this is our interrupt.  If so, save the status and schedule a DPC.
 * \param Interrupt
 * \param MessageID
 * \return IsOurInterrupt
 */
BOOLEAN
DMADriverInterruptIsr(
	IN WDFINTERRUPT		Interrupt,
	IN unsigned long	MessageID
)
{
	PDEVICE_EXTENSION	pDevExt;
	volatile UINT32 	Status;
	INT32				dmaEngine;
	BOOLEAN				IsOurInterrupt = FALSE;
	BOOLEAN bStatus;
	BYTE StatusReg, S2mmIRQcontents, * pStatusByte;
	int Chan;
	UNREFERENCED_PARAMETER(MessageID);

	// Get Device Extensions
	pDevExt = DMADriverGetDeviceContext(WdfInterruptGetDevice(Interrupt));
//	DEBUGP(DEBUG_INFO, "*** DMADrvISR CommonCtrlStatus 0x%X***\n", pDevExt->pDmaRegisters->commonControl.ControlStatus);

	// search for active dmaEngine
    for (dmaEngine = 0; dmaEngine < MAX_NUM_DMA_ENGINES; dmaEngine++)
    {
        PDMA_ENGINE_DEVICE_EXTENSION    pDmaExt;

        pDmaExt = pDevExt->pDmaEngineDevExt[dmaEngine];
        if (pDmaExt != NULL)
        {
			Status = pDmaExt->pDmaEng->ControlStatus; // DMA Engine, not "Global common" ControlStatus

            if ((Status & COMMON_DMA_CTRL_IRQ_ACTIVE) &&
                (Status & COMMON_DMA_CTRL_IRQ_ENABLE))
            {
				// Inc the Interrupt Count
				pDmaExt->IntsInLastSecond++;

//				DEBUGP(DEBUG_INFO, "**From DMA[%d]**", dmaEngine);
				IsOurInterrupt = TRUE;
				bStatus = WdfDpcEnqueue(pDmaExt->CompletionDpc);  // Separate DPC per DMA engine
				if (bStatus == TRUE)
				{
//					DEBUGP(DEBUG_INFO, " *DPC Qd* ");

				}
            }
        }
    }

	if ((pDevExt->pDmaRegisters->commonControl.ControlStatus &	(CARD_USER_INTERRUPT_MODE | CARD_USER_INTERRUPT_ACTIVE)) ==
																(CARD_USER_INTERRUPT_MODE | CARD_USER_INTERRUPT_ACTIVE))
	{
		DEBUGP(DEBUG_INFO, "*** DMADrvISR UserInt commonControl.ControlStatus: 0x%X***", pDevExt->pDmaRegisters->commonControl.ControlStatus );
		// Acknowledge the interrupt in the User Interrupt status by writing the bit back
		pDevExt->pDmaRegisters->commonControl.UserIntStatus = (1 << pDevExt->NumberDMAEngines);
		if (pDevExt->pADC_S2MM_DMAcontrolStatus != NULL)
		{
			StatusReg = 0;
			S2mmIRQcontents = 0;
			for (Chan = 3; Chan >= 0; Chan--)
			{
				pStatusByte = (BYTE*)pDevExt->pADC_S2MM_DMAcontrolStatus;
				StatusReg = *(pStatusByte + ((UINT64)Chan * 0x40));
				StatusReg = (StatusReg & 0x10) >> (4 - Chan);  // get bit4, this channel's IntStatus
				S2mmIRQcontents |= StatusReg;
			}
			DEBUGP(DEBUG_INFO, "    S2MMDMA status 0x%x", S2mmIRQcontents);
		}
		// Check for a waiting request...
		if (pDevExt->UsrIntRequest != NULL)
		{
//			DEBUGP(DEBUG_INFO, "*** DMADrvISR UserInt Q UsrIrqDpc ***");

			WdfDpcEnqueue(pDevExt->UsrIrqDpc);
		}
		else
		{
			// Acknowledge the User Interrupt in the Common Control by writing the INTERRUPT_ACTIVE flag.
			pDevExt->pDmaRegisters->commonControl.ControlStatus |= CARD_USER_INTERRUPT_ACTIVE;
		}
		IsOurInterrupt = TRUE;
	}
	return IsOurInterrupt;
}

/*! DMADriverInterruptMSIIsr
 *
 * \brief MSI Interrupt Handler.
 *  Determine if this is our interrupt.  If so, save the status and schedule a DPC.
 * \param Interrupt
 * \param MessageID
 * \return IsOurInterrupt
 */
BOOLEAN
DMADriverInterruptMSIIsr(
	IN WDFINTERRUPT		Interrupt,
	IN unsigned long	MessageID
)
{
	PDEVICE_EXTENSION				pDevExt;
	PDMA_ENGINE_DEVICE_EXTENSION    pDmaExt;
	volatile UINT32 				Status;
	BOOLEAN							IsOurInterrupt = FALSE;

	// Get Device Extensions
	pDevExt = DMADriverGetDeviceContext(WdfInterruptGetDevice(Interrupt));
	DEBUGP(DEBUG_INFO, "-->DMADriverInterruptMSIIsr--<\n");

	pDmaExt = pDevExt->pDmaExtMSIVector[MessageID];
	if (pDmaExt != NULL)
	{
		Status = pDmaExt->pDmaEng->ControlStatus;
		// Increment the Interrupt Count
		pDmaExt->IntsInLastSecond++;

		if ((Status & COMMON_DMA_CTRL_IRQ_ACTIVE) &&
			(Status & COMMON_DMA_CTRL_IRQ_ENABLE))
		{
			DEBUGP(DEBUG_INFO, " Interrupt for DMA Channel %d", MessageID);
			WdfDpcEnqueue(pDmaExt->CompletionDpc);
		}
		IsOurInterrupt = TRUE;
    }
	if ((pDevExt->pDmaRegisters->commonControl.ControlStatus &	(CARD_USER_INTERRUPT_MODE | CARD_USER_INTERRUPT_ACTIVE)) ==
																	(CARD_USER_INTERRUPT_MODE | CARD_USER_INTERRUPT_ACTIVE))
	{
		// Check for a waiting request...
		if (pDevExt->UsrIntRequest != NULL)
		{
			WdfDpcEnqueue(pDevExt->UsrIrqDpc);
		}
		else
		{
			// Acknowledge the User Interrupt by writing the INTERRUPT_ACTIVE flag.
			pDevExt->pDmaRegisters->commonControl.ControlStatus |= CARD_USER_INTERRUPT_ACTIVE;
		}
		IsOurInterrupt = TRUE;
	}
	return IsOurInterrupt;
}

BOOLEAN
DMADriverInterruptUserIRQMSIIsr(
	 IN WDFINTERRUPT	Interrupt,
	 IN unsigned long	MessageID)
 {
	 PDEVICE_EXTENSION	pDevExt;
	 BOOLEAN			IsOurInterrupt = FALSE;
	 UNREFERENCED_PARAMETER(MessageID);

	 DEBUGP(DEBUG_INFO, "-->DMADriverInterruptUserIRQMSIIsr--<\n");
	 // Get Device Extensions
	 pDevExt = DMADriverGetDeviceContext(WdfInterruptGetDevice(Interrupt));
	 	if ((pDevExt->pDmaRegisters->commonControl.ControlStatus &	(CARD_USER_INTERRUPT_MODE | CARD_USER_INTERRUPT_ACTIVE)) ==
	 																(CARD_USER_INTERRUPT_MODE | CARD_USER_INTERRUPT_ACTIVE))
		{
			 // Check for a waiting request...
			if (pDevExt->UsrIntRequest != NULL)
			{
				WdfDpcEnqueue(pDevExt->UsrIrqDpc);
			}
			else
			{
			 //Ackknowledge the User Interrupt by writing the INTERRUPT_ACTIVE flag.
				pDevExt->pDmaRegisters->commonControl.ControlStatus |= CARD_USER_INTERRUPT_ACTIVE;
			}
		 IsOurInterrupt = TRUE;
		}
	return IsOurInterrupt;
}

/*! DMADriverInterruptEnable
 *
 * \brief Called at DIRQL after EvtDeviceD0Entry returns or
 *  when WdfInterruptEnable is called.
 *  The interrupt spinlock is being held during this call.
 * \note This enables global device interrupts.
 *  DMA Engine interrupts are not enabled at this point.
 * \param Interrupt
 * \param Device
 * \return status
 */
NTSTATUS
DMADriverInterruptEnable (
		IN WDFINTERRUPT	Interrupt,
		IN WDFDEVICE	Device
		)
{
	PDEVICE_EXTENSION	pDevExt;

	UNREFERENCED_PARAMETER(Interrupt);

	// Log the Enable request
	DEBUGP(DEBUG_INFO, "Enabling Global Interrupts (commonControl.ControlStatus) CARD_IRQ_ENABLE"); // NWL IOCTL DMA (DDR3) read/write will hang if not IRQ_ENABLE

	pDevExt = DMADriverGetDeviceContext(Device);
	// Enable interrupts
	pDevExt->pDmaRegisters->commonControl.ControlStatus = CARD_IRQ_ENABLE;
	return STATUS_SUCCESS;
}

/*! DMADriverInterruptDisable
 *
 * \brief Called at DIRQL before EvtDeviceD0Exit is called or
 *  when WdfInterruptDisable is called.
 *  The interrupt spinlock is being held during this call.
 * \note This disables global device interrupts.
 *  DMA Engine interrupts are not disabled at this point.
 * \param Interrupt
 * \param Device
 * \return status
 */
NTSTATUS
DMADriverInterruptDisable (
		IN WDFINTERRUPT	Interrupt,
		IN WDFDEVICE	Device
		)
{
	PDEVICE_EXTENSION	pDevExt;

	UNREFERENCED_PARAMETER(Interrupt);

	// Log the disable request
	DEBUGP(DEBUG_INFO, "Disabling Global Interrupts");

	pDevExt = DMADriverGetDeviceContext(Device);
	// Disable interrupts
	pDevExt->pDmaRegisters->commonControl.ControlStatus = 0;

	return STATUS_SUCCESS;
}

// User Interrupt Support
/*! UserIRQDpc
 *
 * \brief This routine processes User Interrupts
 * \param pDpcCtx - Pointer to the driver context for this Dpc
 * \returns none
 */

// DZimmerman  10-July-2020 
// NOTE!  Since the DPC runs in arbitrary context, the calling thread (which called the ThorDAQ function)
// may be out of context and have invalid virtual memory addresses at the time the DPC runs, meaning
// accessing that virtual address can cause BSOD.
// MORE DEVELOPMENT needed to complete DPC with data passed back to calling thread...
// Solution could be in allocating IRQ_WAIT_STRUCT in Device Extension (locked memory)
// mem pool and sharing pointer to that permanently valid memory space with Application?
VOID
UserIRQDpc(IN WDFDPC Dpc)
{
	NTSTATUS status;
	size_t	infoSize;       // sizeof struct (info) we're returning to user program
	size_t  AppBufferSize;  // Header definition for kernel IOCTL and Application program should match
	PDEVICE_EXTENSION		pDevExt;
    PUSER_IRQ_WAIT_STRUCT   pUIRQWaitStruct;
//	BYTE S2mmIRQcontents, StatusReg, *pStatusByte;
//	int Chan;

	pDevExt = DMADriverGetDeviceContext(WdfDpcGetParentObject(Dpc));
	AppBufferSize = infoSize = sizeof(USER_IRQ_WAIT_STRUCT);
	DEBUGP(DEBUG_INFO, "**UserIRQDpc");
	
	// what are contents of ThorDAQ's S2MM DMA Status Register's?

	// Set Spinlock
	WdfSpinLockAcquire(pDevExt->UsrIntSpinLock);
	// Determine pending interrupt requests.
	if (pDevExt->UsrIntRequest != NULL) // i.e., did application program call UserIRWWait()?
	{
        // implies that Interrupt received prior to TIMER assertion
		// Get the input buffer, where the wait information is passed
		status = WdfRequestRetrieveOutputBuffer
		(
			pDevExt->UsrIntRequest,
			infoSize,						// Min size
			(PVOID*)&pUIRQWaitStruct,		// (user program) buffer
			&AppBufferSize
		);
//		DEBUGP(DEBUG_INFO, "    infoSize %d  AppBufferSize %d, status 0x%x", infoSize, AppBufferSize, status);
		if (status == STATUS_SUCCESS)  
		{
//			DEBUGP(DEBUG_INFO, "    *pUIRQWaitStruct %p, increment bank...", *pUIRQWaitStruct);
//			*(pUIRQWaitStruct->DMA_Bank)++;
		}
//		DEBUGP(DEBUG_INFO, "  (pUIRQWaitStruct)%p, status 0x%x **", (PVOID*)pUIRQWaitStruct, status);

//		DEBUGP(DEBUG_INFO, "    calling WdfRequestCompleteWithInformation(), size %d", infoSize);
		WdfRequestCompleteWithInformation(pDevExt->UsrIntRequest, STATUS_SUCCESS, (ULONG_PTR)infoSize);
		// Reset pointer back to NULL.
		pDevExt->UsrIntRequest = NULL;
	}
	// Release spinlock.
	WdfSpinLockRelease(pDevExt->UsrIntSpinLock);
	// Set control status pointer to turn off INTERRUPT_ACTIVE flag.
	pDevExt->pDmaRegisters->commonControl.ControlStatus |= CARD_USER_INTERRUPT_ACTIVE;

	// [TEST] what are the S2MM DMA "interrupt generated" bits?
	/*
	if( pDevExt->pADC_S2MM_DMAcontrolStatus != NULL)
	{
		StatusReg = 0;
		S2mmIRQcontents = 0;
		for (Chan = 0; Chan < 4; Chan++)
		{
			pStatusByte = (BYTE*)pDevExt->pADC_S2MM_DMAcontrolStatus;
			StatusReg = *(pStatusByte + (Chan * 0x40));
			StatusReg = (StatusReg & 0x10) >> (4 - Chan);  // get bit4, this channel's IntStatus
			S2mmIRQcontents |= StatusReg;
		}
		DEBUGP(DEBUG_INFO, "    S2MMDMA status 0x%x", S2mmIRQcontents);
	}*/

	return;
}
// End User Interrupt
