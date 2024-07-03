#include "Driver.h"
#include "Trace.h"
/*!***************************************************************************
*
* 	\brief PacketBufferAllocate - This routine is called when a
*	PACKET_RX_BUF_ALLOCATE_IOCTL is sent from the application
* 
* 	\param pDevExt - Pointer to this drivers context (data store)
*	\param Request - Pointer to the IOCtl request
*   \param InputBufferLength - Size of the IOCtl Input buffer
* 	\param OutputBufferLength - Size of the IOCtl Output buffer
* 
*   \return NTSTATUS
* 
* 	This routine is called at IRQL < DISPATCH_LEVEL.
*
*****************************************************************************/
NTSTATUS PacketBufferAllocate(
	PDEVICE_EXTENSION	pDevExt,
	IN WDFREQUEST	Request,
	IN size_t		OutputBufferLength,
	IN size_t		InputBufferLength
	)
{
	PBUF_ALLOC_STRUCT			pBufAlloc;
	ULONG32						NumberDescriptors;

	NTSTATUS 					status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(InputBufferLength);

	// Get the input buffer, where we find the allocate parameters
	status = WdfRequestRetrieveInputBuffer(Request,
		sizeof(BUF_ALLOC_STRUCT),		/* size */
		(PVOID *) &pBufAlloc,	 		/* buffer */
		NULL);
	if (status == STATUS_SUCCESS)
	{
		status = STATUS_INVALID_DEVICE_REQUEST;
		// Validate EngineNum and make sure it has been properly initialized
		if ((pBufAlloc != NULL) &&
			(pBufAlloc->EngineNum < MAX_NUM_DMA_ENGINES) &&
			(pDevExt->pDmaEngineDevExt[pBufAlloc->EngineNum] != NULL))
		{
			PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;
			status = GetDMAEngineContext(pDevExt, pBufAlloc->EngineNum, &pDmaExt);
			if (!NT_SUCCESS(status)) 
			{
				return status;
			}
			// Now make sure there is a queue associated and it is a Packet Type Engine
			if (pDmaExt->DmaType == DMA_TYPE_PACKET_RECV)
			{
				if (pBufAlloc->AllocationMode == PACKET_MODE_ADDRESSABLE)
				{
					if (pDmaExt->bAddressablePacketMode)
					{
						NumberDescriptors = pBufAlloc->NumberDescriptors;
						if (NumberDescriptors > pDmaExt->NumberOfDescriptors)
						{
							NumberDescriptors = pDmaExt->NumberOfDescriptors;
						}
						if(pBufAlloc->IsS2mmDmaEnabled == TRUE)
						{
							status = InitializeS2mmPacketDescriptors(pDevExt, pDmaExt);
						}
						status = InitializeAddressablePacketDescriptors(pDevExt, pDmaExt);

						pDmaExt->PacketMode = pBufAlloc->AllocationMode; // =PACKET_MODE_ADDRESSABLE
					}
					else
					{
						status = STATUS_INVALID_DEVICE_REQUEST;
					}
				}
				else
				{
					pBufAlloc->Length = 0;
					pBufAlloc->MaxPacketSize = 0;
					status = STATUS_INVALID_PARAMETER;
				}
			}
		} // pBufAlloc != NULL... failed
		else
		{
		}
	}
	else  // WdfRequestRetrieveInputBuffer failed
	{
	}
	return status;
}


/*!***************************************************************************
*
* 	\brief PacketBufferRelease - This routine is called when a
*	PACKET_RX_BUF_RELEASE_IOCTL is sent from the application
* 
* 	\param pDevExt - Pointer to this drivers context (data store)
*   \param Request - Pointer to the IOCtl request
*   \param InputBufferLength - Size of the IOCtl Input buffer
* 	\param OutputBufferLength - Size of the IOCtl Output buffer
* 
*   \return NTSTATUS
* 
* 	This routine is called at IRQL < DISPATCH_LEVEL.
*
*****************************************************************************/
NTSTATUS PacketBufferRelease(
	PDEVICE_EXTENSION	pDevExt,
	IN WDFREQUEST	Request,
	IN size_t		OutputBufferLength,
	IN size_t		InputBufferLength
	)
{
	PBUF_DEALLOC_STRUCT			pBufDeAlloc;
	NTSTATUS 					status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(InputBufferLength);

	// Get the input buffer, where we find the deallocate parameters
	status = WdfRequestRetrieveInputBuffer(
		Request,
		sizeof(BUF_DEALLOC_STRUCT),		/* Min size */
		(PVOID *) &pBufDeAlloc,			/* buffer */
		NULL);
	if (status == STATUS_SUCCESS)
	{
		status = STATUS_INVALID_DEVICE_REQUEST;
		// Validate EngineNum
		if ((pBufDeAlloc != NULL) &&
			(pBufDeAlloc->EngineNum < MAX_NUM_DMA_ENGINES) &&
			(pDevExt->pDmaEngineDevExt[pBufDeAlloc->EngineNum] != NULL))
		{
			PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;
			status = GetDMAEngineContext(pDevExt, pBufDeAlloc->EngineNum, &pDmaExt);
			if (!NT_SUCCESS(status)) 
			{
				return status;
			}
			if (pDmaExt->DmaType == DMA_TYPE_PACKET_RECV)
			{
				if ((pDmaExt->PacketMode == PACKET_MODE_FIFO) ||
					(pDmaExt->PacketMode == PACKET_MODE_STREAMING))
				{
					pDmaExt->bFreeRun = FALSE;
					FreeRxDescriptors(pDevExt, pDmaExt);
					pDmaExt->PMdl = NULL;
					pDmaExt->UserVa = NULL;
					status = STATUS_SUCCESS;
				}
				else if (pDmaExt->PacketMode == PACKET_MODE_ADDRESSABLE)
				{
					FreeRxDescriptors(pDevExt, pDmaExt);
					pDmaExt->PMdl = NULL;
					pDmaExt->UserVa = NULL;
					status = STATUS_SUCCESS;
				}
				else
				{
					//DEBUGP(DEBUG_ERROR, "Packet / deallocate mode is not supported\n");
					status = STATUS_INVALID_PARAMETER;
				}
				pDmaExt->PacketMode = DMA_MODE_NOT_SET;
			}
		}
	}
	else
	{
	}

	return status;
}


/*!***************************************************************************
*
* 	\brief PacketGeneratorControl - This routine is called when a
* 	PACKET_GEN_CTRL_IOCTL is sent from the application
* 
* 	This function works for setting both the Packet Generator and the
* 	Packet Checker.
* 
* 	\param pDevExt - Pointer to this drivers context (data store)
*   \param Request - Pointer to the IOCtl request
*   \param InputBufferLength - Size of the IOCtl Input buffer
* 
*   \return NTSTATUS
* 
* 	This routine is called at IRQL < DISPATCH_LEVEL.
*
*****************************************************************************/
NTSTATUS ImageAcquisitionConfig(
	PDEVICE_EXTENSION	pDevExt,
	IN WDFREQUEST	Request,
	IN size_t		InputBufferLength
	)
{
	//PGLOBAL_IMG_GEN_CTRL_STRUCT			pPacketGenIOCtl;	
	PDATA_ACQ_CTRL_STRUCT				pDaqCtrl;
	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;
	PBAR3_MAP_STRUCT		pPacketGen;
	PBAR2_CONTROL_MAP_STRUCT		pS2mmCtrl;
	NTSTATUS 			status;
	//ULONG64 gp0Reg2; 
	int					i;
	USHORT tmp;
	USHORT MinTransferSizeInOctets = 1; // GPO_Reg7 setting
	//int c, p, f;

	UNREFERENCED_PARAMETER(InputBufferLength);
	DEBUGP(DEBUG_INFO, "Enter ImageAcquisitionConfig ");
	// Get the input buffer, where we find the deallocate parameters
	status = WdfRequestRetrieveInputBuffer(
		Request,
		sizeof(DATA_ACQ_CTRL_STRUCT),			/* Min size */
		(PVOID *) &pDaqCtrl,				/* buffer */
		NULL);
	if (status == STATUS_SUCCESS)
	{
		status = STATUS_INVALID_DEVICE_REQUEST;
		// Validate EngineNum
		status = GetDMAEngineContext(pDevExt, pDaqCtrl->gblCtrl.EngineNum, &pDmaExt);
		if (!NT_SUCCESS(status)) 
		{
			return status;
		}

		if ((pDmaExt->DmaType == DMA_TYPE_PACKET_SEND) ||
			(pDmaExt->DmaType == DMA_TYPE_PACKET_RECV))
		{
			// Get FPGA Mapping pointer
			pPacketGen = (PBAR3_MAP_STRUCT)(pDevExt->pBar3Map);
			pS2mmCtrl = (PBAR2_CONTROL_MAP_STRUCT)(pDevExt->pBar2Controls);
			DEBUGP(DEBUG_INFO, "(BAR2)pS2mmCtrl->ctrlCh[0] 0x%p", &pS2mmCtrl->ctrlCh[0]);
			// If the DMA is enabled, stop it.
			if (pDmaExt->pDmaEng->ControlStatus & (COMMON_DMA_CTRL_IRQ_ENABLE | PACKET_DMA_CTRL_DMA_ENABLE)) 
			{
				DEBUGP(DEBUG_INFO, " pDmaExt->pDmaEng->ControlStatus 0x%p => 0", &pDmaExt->pDmaEng->ControlStatus);
				pDmaExt->pDmaEng->ControlStatus = 0;
			}
			status = STATUS_SUCCESS;
			// Stop all generators
			pPacketGen->globalGenCtrl.Layout.StopRun_FpgaRev = 0x00; //Stop Global Register.
			DEBUGP(DEBUG_INFO, "pPacketGen->globalGenCtrl.Layout.StopRun_FpgaRev 0x%p => 0", &pPacketGen->globalGenCtrl.Layout.StopRun_FpgaRev);
			//Clear all the S2MM Host system Interrupt
			pS2mmCtrl->ctrlCh[1].ctrl.SR0_CR0 = 0x12;//0x50;// 0x12;
			pS2mmCtrl->ctrlCh[2].ctrl.SR0_CR0 = 0x12;//0x50;// 0x12;
			pS2mmCtrl->ctrlCh[3].ctrl.SR0_CR0 = 0x12;//0x50;// 0x12;
			pS2mmCtrl->ctrlCh[0].ctrl.SR0_CR0 = 0x12;//0x50;// 0x12;
			DEBUGP(DEBUG_INFO, "pS2mmCtrl->ctrlCh[(1,2,3,0)].ctrl.SR0_CR0 => 0x12");

            //Check for debug mode. If so just enable the global control without setting other controls. 
			for (i = 0; i < 4; i++) {
				DEBUGP(DEBUG_INFO, "     ch[%d]  read GP0Reg0(MS) lines_written %d, CntRdy %d,  Cmplt %d CntSmpClk %d", i, (int)((pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg0 >> 48) & 0xFFFF),
					(int)(pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg0 >> 32) & 0x1,
					(int)(pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg0 >> 33) & 0x1,
					(int)(pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg0) & 0xffff);
			}
			if ((pDaqCtrl->gblCtrl.StopRun&0x01) == 1) // Global Register START bit Enabled
			{
				if (pDaqCtrl->gblCtrl.DebugMode == FALSE)
				{
				
					pPacketGen->globalGenCtrl.Layout.ImgSyncCtrl = pDaqCtrl->gblCtrl.imgSyncCtrl;
					DEBUGP(DEBUG_INFO, "ImgSyncCtrl ptr 0x%p set to 0x%x ", &pPacketGen->globalGenCtrl.Layout.ImgSyncCtrl, pDaqCtrl->gblCtrl.imgSyncCtrl);
					////memcpy(&(pPacketGen->globalGenCtrl.Layout.GPIOConfig), &(pDaqCtrl->gblCtrl.GPIOConfig), sizeof(ULONG64));
					memcpy(&(pPacketGen->scanningRegs.Layout.SyncCtrl_ResScanPeriod), &(pDaqCtrl->scan.syncCtrl), sizeof(USHORT)); 
					DEBUGP(DEBUG_INFO, "SyncCtrl_ResScanPeriod ptr 0x%p set to 0x%x ", &pPacketGen->scanningRegs.Layout.SyncCtrl_ResScanPeriod, pDaqCtrl->scan.syncCtrl);
					memcpy(&(pPacketGen->scanningRegs.Layout.FrameCnt), &(pDaqCtrl->scan.frameCnt), sizeof(USHORT));
					DEBUGP(DEBUG_INFO, "pPacketGen->scanningRegs.Layout.FrameCnt ptr 0x%p => FrameCnt %d (0x%x)", &pPacketGen->scanningRegs.Layout.FrameCnt,
						pDaqCtrl->scan.frameCnt, pDaqCtrl->scan.frameCnt);
					////pPacketGen->scanningRegs.Layout.AdpllCtrl = pDaqCtrl->scan.adpllCtrl;
					memcpy(&(pPacketGen->scanningRegs.Layout.SyncOffset), &(pDaqCtrl->scan.syncOffset), sizeof(USHORT)); 
					DEBUGP(DEBUG_INFO, "pPacketGen->scanningRegs.Layout.SyncOffset (ptr) 0x%p, SyncOffset %d", &pPacketGen->scanningRegs.Layout.SyncOffset,
						pDaqCtrl->scan.syncOffset);
					////memcpy(&(pPacketGen->scanningRegs.Layout.CenterFreq), &(pDaqCtrl->scan.adpllDcoCenterFreq), sizeof(ULONG32));
					memcpy(&(pPacketGen->scanningRegs.Layout.IntraFrameDelay), &(pDaqCtrl->scan.intraFrameDelay), sizeof(ULONG32));
					DEBUGP(DEBUG_INFO, "             IntraFrameDelay(ptr) 0x%p => %d, galvoPixelDelay %d, intraLineDelay  %d",
						&pPacketGen->scanningRegs.Layout.IntraFrameDelay, pDaqCtrl->scan.intraFrameDelay, pDaqCtrl->scan.galvoPixelDelay, pDaqCtrl->scan.intraLineDelay);
					memcpy(&(pPacketGen->scanningRegs.Layout.GalvoPixelDelay), &(pDaqCtrl->scan.galvoPixelDelay), sizeof(ULONG32));
					memcpy(&(pPacketGen->scanningRegs.Layout.GalvoPixelDwell), &(pDaqCtrl->scan.galvoPixelDwell), sizeof(ULONG32));
					DEBUGP(DEBUG_INFO, "GalvoPixelDwell ptr 0x%p set to %d ", &pPacketGen->scanningRegs.Layout.GalvoPixelDwell, pDaqCtrl->scan.galvoPixelDwell);
					memcpy(&(pPacketGen->scanningRegs.Layout.IntraLineDelay), &(pDaqCtrl->scan.intraLineDelay), sizeof(ULONG32));
					////if (pDaqCtrl->gblCtrl.Mode < 2)
					//{
					//	memcpy(&(pPacketGen->scanningRegs.Layout.GalvoPixelDwell), &(pDaqCtrl->scan.galvoPixelDwell), sizeof(ULONG32));
					//
					//	memcpy(&(pPacketGen->scanningRegs.Layout.GalvoPixelDelay), &(pDaqCtrl->scan.galvoPixelDelay), sizeof(ULONG32));				
					//
					//	memcpy(&(pPacketGen->scanningRegs.Layout.IntraFrameDelay), &(pDaqCtrl->scan.intraFrameDelay), sizeof(ULONG32));

					//	memcpy(&(pPacketGen->scanningRegs.Layout.IntraLineDelay), &(pDaqCtrl->scan.intraLineDelay), sizeof(ULONG32));
					//}

					//for(p = 0; p < 2; p++)
					//{
					//	for(c = 0; c < 4; c++)
					//	{
					//		pPacketGen->streamProcessingRegs.Layout.CtrlReg = 0x00;
					//		for(f = 0; f < 16; f++)
					//		{
					//			pPacketGen->streamProcessingRegs.Layout.FirCoefficient = 0x00;
					//		}
					//	}
					//}	

			//		pPacketGen->streamProcessingRegs.Layout.PulseInterleave = pDaqCtrl->streamProcessing.pulseInterleave;
					DEBUGP(DEBUG_INFO, " pPacketGen->streamProcessingRegs.Layout.PulseInterleave (ptr) 0x%p => %d, PulseInterleaveOffset %d",
						&pPacketGen->streamProcessingRegs.Layout.PulseInterleave, pDaqCtrl->streamProcessing.pulseInterleave, 
						pDaqCtrl->streamProcessing.PulseInterleaveOffset);
					// AVOID dFLIM register conflicts at 0x1C0-1CF
					memcpy(&(pPacketGen->streamProcessingRegs.Layout.PulseInterleaveOffset), &(pDaqCtrl->streamProcessing.PulseInterleaveOffset), sizeof(ULONG32));
					memcpy(&(pPacketGen->streamProcessingRegs.Layout.ScanPeriod), &(pDaqCtrl->streamProcessing.scanningPeriod), sizeof(USHORT));
					memcpy(&(pPacketGen->streamProcessingRegs.Layout.DownsampleRate), &(pDaqCtrl->streamProcessing.downsampleRate), sizeof(ULONG32));
					DEBUGP(DEBUG_INFO, " pPacketGen->streamProcessingRegs.Layout.ScanPeriod (ptr) 0x%p => %d",
						&pPacketGen->streamProcessingRegs.Layout.ScanPeriod, pDaqCtrl->streamProcessing.scanningPeriod);
					DEBUGP(DEBUG_INFO, " pPacketGen->streamProcessingRegs.Layout.DownsampleRate (ptr) 0x%p => %d",
						&pPacketGen->streamProcessingRegs.Layout.DownsampleRate, pDaqCtrl->streamProcessing.downsampleRate);

					//memcpy(&(pPacketGen->streamProcessingRegs.Layout.ThreePhotonSampleOffset), &(pDaqCtrl->streamProcessing.threePhotonSampleOffset), sizeof(ULONG32));

					pPacketGen->samplingClockGenRegs.Layout.CtrlReg = pDaqCtrl->samplingClock.SamplingClkCtrlReg;
					DEBUGP(DEBUG_INFO, " samplingClockGenRegs.Layout.CtrlReg (ptr) 0x%p => 0x%x",
						&pPacketGen->samplingClockGenRegs.Layout.CtrlReg, pDaqCtrl->samplingClock.SamplingClkCtrlReg);

					memcpy(&(pPacketGen->samplingClockGenRegs.Layout.PhaseOffset), &(pDaqCtrl->samplingClock.phaseOffset), sizeof(USHORT));
					DEBUGP(DEBUG_INFO, " pPacketGen->samplingClockGenRegs.Layout.PhaseOffset (ptr) 0x%p => %d, PhaseStep %d, PhaseLimit %d",
						&pPacketGen->samplingClockGenRegs.Layout.PhaseOffset, pDaqCtrl->samplingClock.phaseOffset, 
						pDaqCtrl->samplingClock.phaseStep, pDaqCtrl->samplingClock.phaseLimit);
					pPacketGen->samplingClockGenRegs.Layout.PhaseStep = pDaqCtrl->samplingClock.phaseStep;
					memcpy(&(pPacketGen->samplingClockGenRegs.Layout.PhaseLimit), &(pDaqCtrl->samplingClock.phaseLimit), sizeof(USHORT));

					memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.CtrlReg),           &(pDaqCtrl->galvoWaveformCtrl.ctrlReg),          sizeof(ULONG64));
					DEBUGP(DEBUG_INFO, " pPacketGen->galvoWaveformGenRegs1.Layout.CtrlReg (ptr) 0x%p => 0x%llx",
						&pPacketGen->galvoWaveformGenRegs1.Layout.CtrlReg, pDaqCtrl->galvoWaveformCtrl.ctrlReg);

					memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.DACUpdateRate[0]),  &(pDaqCtrl->galvoWaveformCtrl.dacUpdateRate[0]), sizeof(ULONG64));
					DEBUGP(DEBUG_INFO, " DACUpdateRate (ptr) 0x%p => dacUpdateRate[0]%016llx dacUpdateRate[1]%016llx dacUpdateRate[2]%016llx  ", 
						&pPacketGen->galvoWaveformGenRegs1.Layout.DACUpdateRate[0], 
						pDaqCtrl->galvoWaveformCtrl.dacUpdateRate[0], pDaqCtrl->galvoWaveformCtrl.dacUpdateRate[1], pDaqCtrl->galvoWaveformCtrl.dacUpdateRate[2]);
					memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.DACUpdateRate[1]),  &(pDaqCtrl->galvoWaveformCtrl.dacUpdateRate[1]), sizeof(ULONG64));
					memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.DACUpdateRate[2]),  &(pDaqCtrl->galvoWaveformCtrl.dacUpdateRate[2]), sizeof(ULONG64));
					memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.DACAmplitude),      &(pDaqCtrl->galvoWaveformCtrl.dacAmplitude),     sizeof(USHORT));
					DEBUGP(DEBUG_INFO, " DACAmplitude (ptr) 0x%p => %d", &pPacketGen->galvoWaveformGenRegs1.Layout.DACAmplitude, pDaqCtrl->galvoWaveformCtrl.dacAmplitude);

					DEBUGP(DEBUG_INFO, " 0x%p => DACStepSize[0] %d, 0x%p => DACStepSize[1] %d, 0x%p => DACStepSize[2] %d",
						&(pPacketGen->galvoWaveformGenRegs1.Layout.DACStepSize[0]), pDaqCtrl->galvoWaveformCtrl.dacStepSize[0],
						&(pPacketGen->galvoWaveformGenRegs1.Layout.DACStepSize[1]), pDaqCtrl->galvoWaveformCtrl.dacStepSize[1],
						&(pPacketGen->galvoWaveformGenRegs1.Layout.DACStepSize[2]), pDaqCtrl->galvoWaveformCtrl.dacStepSize[2]);
					memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.DACStepSize[0]),    &(pDaqCtrl->galvoWaveformCtrl.dacStepSize[0]),   sizeof(USHORT));
					memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.DACStepSize[1]),    &(pDaqCtrl->galvoWaveformCtrl.dacStepSize[1]),   sizeof(USHORT));
					memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.DACStepSize[2]),    &(pDaqCtrl->galvoWaveformCtrl.dacStepSize[2]),   sizeof(USHORT));

					DEBUGP(DEBUG_INFO, " 0x%p => DACOffset[0] 0x%0llx, 0x%p => DACOffset[1] 0x%0llx, 0x%p => DACOffset[2] 0x%0llx",
						&(pPacketGen->galvoWaveformGenRegs2.Layout.DACOffset[0]), pDaqCtrl->galvoWaveformCtrl.dacOffset[0],
						&(pPacketGen->galvoWaveformGenRegs2.Layout.DACOffset[1]), pDaqCtrl->galvoWaveformCtrl.dacOffset[1],
						&(pPacketGen->galvoWaveformGenRegs2.Layout.DACOffset[2]), pDaqCtrl->galvoWaveformCtrl.dacOffset[2]);
					memcpy(&(pPacketGen->galvoWaveformGenRegs2.Layout.DACOffset[0]),      &(pDaqCtrl->galvoWaveformCtrl.dacOffset[0]),     sizeof(ULONG64));
					memcpy(&(pPacketGen->galvoWaveformGenRegs2.Layout.DACOffset[1]),	  &(pDaqCtrl->galvoWaveformCtrl.dacOffset[1]),     sizeof(ULONG64));
					memcpy(&(pPacketGen->galvoWaveformGenRegs2.Layout.DACOffset[2]),      &(pDaqCtrl->galvoWaveformCtrl.dacOffset[2]),     sizeof(ULONG64));

					memcpy(&(pPacketGen->galvoWaveformGenRegs2.Layout.DOUpdateRate), &(pDaqCtrl->galvoWaveformCtrl.doUpdateRate), sizeof(ULONG64));
					DEBUGP(DEBUG_INFO, " galvoWaveformGenRegs2.Layout.DOUpdateRate (ptr) 0x%p => %llx", &pPacketGen->galvoWaveformGenRegs2.Layout.DOUpdateRate, pDaqCtrl->galvoWaveformCtrl.doUpdateRate);
					memcpy(&(pPacketGen->galvoWaveformGenRegs2.Layout.DOParkValue),       &(pDaqCtrl->galvoWaveformCtrl.doParkValue),      sizeof(ULONG64));
					DEBUGP(DEBUG_INFO, " galvoWaveformGenRegs2.Layout.DOParkValue (ptr) 0x%p => %llx", &pPacketGen->galvoWaveformGenRegs2.Layout.DOParkValue, pDaqCtrl->galvoWaveformCtrl.doParkValue);
					memcpy(&(pPacketGen->galvoWaveformGenRegs2.Layout.DOOffset),          &(pDaqCtrl->galvoWaveformCtrl.doOffset),         sizeof(ULONG64));
					DEBUGP(DEBUG_INFO, " galvoWaveformGenRegs2.Layout.DOOffset (ptr) 0x%p => %llx", &pPacketGen->galvoWaveformGenRegs2.Layout.DOOffset, pDaqCtrl->galvoWaveformCtrl.doOffset);
					memcpy(&(pPacketGen->galvoWaveformGenRegs2.Layout.DACAmFilterWindow), &(pDaqCtrl->galvoWaveformCtrl.dacAmFilterWindow),sizeof(ULONG64));

					//pPacketGen->streamProcessingRegs.Layout.PulseInterleave = 0x00;
					//pPacketGen->streamProcessingRegs.Layout.PulseInterleaveOffset = 0x00;
					//pPacketGen->streamProcessingRegs.Layout.ScanPeriod = 0x00;
					//pPacketGen->streamProcessingRegs.Layout.DownsampleRate = 0x00;
					//pPacketGen->streamProcessingRegs.Layout.ThreePhotonSampleOffset = 0x00;

					//pPacketGen->samplingClockGenRegs.Layout.CtrlReg = pDaqCtrl->samplingClock.reg;
					//memcpy(&(pPacketGen->samplingClockGenRegs.Layout.PhaseOffset), &(pDaqCtrl->samplingClock.phaseOffset), sizeof(USHORT));
					//pPacketGen->samplingClockGenRegs.Layout.PhaseStep = pDaqCtrl->samplingClock.phaseStep;
					//memcpy(&(pPacketGen->samplingClockGenRegs.Layout.PhaseLimit), &(pDaqCtrl->samplingClock.phaseLimit), sizeof(USHORT));

					//memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.CtrlReg),           &(pDaqCtrl->galvoWaveformCtrl.ctrlReg),          sizeof(ULONG64));
					//memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.DACUpdateRate[0]),  &(pDaqCtrl->galvoWaveformCtrl.dacUpdateRate[0]), sizeof(ULONG64));
					//memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.DACUpdateRate[1]),  &(pDaqCtrl->galvoWaveformCtrl.dacUpdateRate[1]), sizeof(ULONG64));
					//memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.DACUpdateRate[2]),  &(pDaqCtrl->galvoWaveformCtrl.dacUpdateRate[2]), sizeof(ULONG64));
					//memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.DACAmplitude),      &(pDaqCtrl->galvoWaveformCtrl.dacAmplitude),     sizeof(USHORT));
					//memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.DACStepSize[0]),    &(pDaqCtrl->galvoWaveformCtrl.dacStepSize[0]),   sizeof(USHORT));
					//memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.DACStepSize[1]),    &(pDaqCtrl->galvoWaveformCtrl.dacStepSize[1]),   sizeof(USHORT));
					//memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.DACStepSize[2]),    &(pDaqCtrl->galvoWaveformCtrl.dacStepSize[2]),   sizeof(USHORT));
					//memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.DACParkValue[0]),   &(pDaqCtrl->galvoWaveformCtrl.dacParkValue[0]),  sizeof(ULONG64));
					//memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.DACParkValue[1]),   &(pDaqCtrl->galvoWaveformCtrl.dacParkValue[1]),  sizeof(ULONG64));
					//memcpy(&(pPacketGen->galvoWaveformGenRegs1.Layout.DACParkValue[2]),   &(pDaqCtrl->galvoWaveformCtrl.dacParkValue[2]),  sizeof(ULONG64));
					//memcpy(&(pPacketGen->galvoWaveformGenRegs2.Layout.DACOffset[0]),      &(pDaqCtrl->galvoWaveformCtrl.dacOffset[0]),     sizeof(ULONG64));
					//memcpy(&(pPacketGen->galvoWaveformGenRegs2.Layout.DACOffset[1]),	  &(pDaqCtrl->galvoWaveformCtrl.dacOffset[1]),     sizeof(ULONG64));
					//memcpy(&(pPacketGen->galvoWaveformGenRegs2.Layout.DACOffset[2]),      &(pDaqCtrl->galvoWaveformCtrl.dacOffset[2]),     sizeof(ULONG64));
					////memcpy(&(pPacketGen->galvoWaveformGenRegs2.Layout.DACChannelMap),     &(pDaqCtrl->galvoWaveformCtrl.dacChannelMap),    sizeof(ULONG64));
					//memcpy(&(pPacketGen->galvoWaveformGenRegs2.Layout.DOUpdateRate),      &(pDaqCtrl->galvoWaveformCtrl.doUpdateRate),     sizeof(ULONG64));
					//memcpy(&(pPacketGen->galvoWaveformGenRegs2.Layout.DOParkValue),       &(pDaqCtrl->galvoWaveformCtrl.doParkValue),      sizeof(ULONG64));
					//memcpy(&(pPacketGen->galvoWaveformGenRegs2.Layout.DOOffset),          &(pDaqCtrl->galvoWaveformCtrl.doOffset),         sizeof(ULONG64));
					//memcpy(&(pPacketGen->galvoWaveformGenRegs2.Layout.DACAmFilterWindow), &(pDaqCtrl->galvoWaveformCtrl.dacAmFilterWindow),sizeof(ULONG64));
				}

				pDevExt->HSize = pDaqCtrl->gblCtrl.HSize;
				pDevExt->VSize = pDaqCtrl->gblCtrl.VSize;
				pDevExt->dataHSize = pDaqCtrl->gblCtrl.dataHSize;
				pDevExt->linesPerStripe = pDaqCtrl->gblCtrl.linesPerStripe;
				pDevExt->mode = pDaqCtrl->gblCtrl.Mode;
				//if (pDaqCtrl->gblCtrl.Mode < 2)
				{
					tmp = (USHORT)(pDaqCtrl->gblCtrl.HSize - 1);
					memcpy(&(pPacketGen->globalGenCtrl.Layout.ImgHSize), &(tmp), sizeof(USHORT));				
					tmp = (USHORT)(pDaqCtrl->gblCtrl.VSize - 1);
					memcpy(&(pPacketGen->globalGenCtrl.Layout.ImgVSize), &(tmp), sizeof(USHORT));
				}
				DEBUGP(DEBUG_INFO, "BAR3, 0x0, ImgHSize %d, ImgVSize %d", (USHORT)(pDaqCtrl->gblCtrl.HSize - 1), (USHORT)(pDaqCtrl->gblCtrl.VSize - 1))
				pDevExt->FrameRate = (USHORT)(pDaqCtrl->gblCtrl.FrameRate);
				pDevExt->FramesPerTransfer = (USHORT)(pDaqCtrl->gblCtrl.FramesPerTransfer);
				pDevExt->DataBufferStartAddress = (ULONG32)(pDaqCtrl->gblCtrl.DataBufferStartAddress);
				DEBUGP(DEBUG_INFO, "[PacketIoCtrl.c]pDaqCtrl->gblCtrl.DataBufferStartAddress 0x%x", pDaqCtrl->gblCtrl.DataBufferStartAddress);
				pDevExt->MAXframeLengthPerChan = ACQ_SINGLE_CHANNEL_BUF_CAP; //pDaqCtrl->gblCtrl.DataBufferChannelOffset; 
				pDevExt->DDR3startAddressFor2ndPingPongBank = pDevExt->MAXframeLengthPerChan * 4;// S2MM_CHANNEL_PER_BOARD;
				DEBUGP(DEBUG_INFO, "MAXframeLengthPerChan 0x%x(%d), DDR3startAddressFor2ndPingPongBank 0x%x(%d) ",
					pDevExt->MAXframeLengthPerChan, pDevExt->MAXframeLengthPerChan, pDevExt->DDR3startAddressFor2ndPingPongBank, pDevExt->DDR3startAddressFor2ndPingPongBank);
				pDevExt->Channel = pDaqCtrl->gblCtrl.Channel;

				for (i = 0; i < S2MM_CHANNEL_PER_BOARD; i++)
				{
					if (pDaqCtrl->gblCtrl.Channel & (0x0001 << i))
					{
						pDevExt->ChannelDescriptor[i] = TRUE;
						DEBUGP(DEBUG_INFO, " Chan[%d] enabled", i);
					}else
					{
						pDevExt->ChannelDescriptor[i] = FALSE;
						DEBUGP(DEBUG_INFO, " Chan[%d] disabled", i);
					}
				}
				DEBUGP(DEBUG_INFO, " pDmaExt->pDmaEng->ControlStatus value 0x%x", pDmaExt->pDmaEng->ControlStatus);
				pDmaExt->pDmaEng->ControlStatus = (COMMON_DMA_CTRL_IRQ_ENABLE | PACKET_DMA_CTRL_DMA_ENABLE);
				DEBUGP(DEBUG_INFO, " pDmaExt->pDmaEng->ControlStatus (ptr) 0x%p => x%x  ", &pDmaExt->pDmaEng->ControlStatus, (COMMON_DMA_CTRL_IRQ_ENABLE | PACKET_DMA_CTRL_DMA_ENABLE));
				DEBUGP(DEBUG_INFO, " pDmaExt->pDmaEng->ControlStatus value 0x%x", pDmaExt->pDmaEng->ControlStatus);

				for (i = 0; i < DFLIM_PROCESSING_STRUCTS_NUM; ++i)
				{
					////pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg0 = 0;
					//gp0Reg2 = (ULONG64)pDevExt->HSize;
					//memcpy(&(pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg2), &(gp0Reg2), sizeof(ULONG64));
					////pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg3 = 0;
					////pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg4 = 0;
					//pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg5_1 = 0x000d0016;
					//pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg5_2 = 0x0088003c;
					//pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg6 = 0x0020005b;
					//pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg7 = 0x00000080;

										//pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg0 = 0;

					DEBUGP(DEBUG_INFO, "   DFLIM_PROCESSING_STRUCT %d:  HSize %d, VSize %d, pDevExt->dataHSize %d (0x%x), linesPerStripe %d", i,
						pDevExt->HSize, pDevExt->VSize, pDevExt->dataHSize, pDevExt->dataHSize, pDevExt->linesPerStripe);
					DEBUGP(DEBUG_INFO, "                                 pPacketGen->dflimProcessingRegs[%d].Layout.GP0Reg2(ptr) %p", i, &pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg2);
					if (0 == pDaqCtrl->gblCtrl.Mode)
					{
						//DFLIM mode
						// GPO_REG2 -- specifices fixed S2MM
						pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg2 = (pDevExt->dataHSize/8) | (pDevExt->linesPerStripe << 16); //BeatsPerLine | (pDevExt->linesPerStripe << 16)
						DEBUGP(DEBUG_INFO, "             writing GPOReg2[%d] = 0x%x", i, (pDevExt->dataHSize / 8) | (pDevExt->linesPerStripe << 16));
						// GPO_REG5 -- four 16-bit params related to threshold
						pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg5_1 = 0x000d0016;
						DEBUGP(DEBUG_INFO, "             writing GP0Reg5_1[%d] (ptr)0x%p = 0x%x", i, &pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg5_1, 0x000d0016);
						pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg5_2 = 0x0088003c;
						DEBUGP(DEBUG_INFO, "             writing GP0Reg5_2[%d] (ptr)0x%p = 0x%x", i, &pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg5_2, 0x0088003c);
						pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg6 = 0; //DFLIM mode (not debug)
						MinTransferSizeInOctets = 0x80;
						pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg7 = MinTransferSizeInOctets; // originally 0x80;  minXFerSize (12 bit), min size in octets of data packets
						DEBUGP(DEBUG_INFO, "             writing GP0Reg7[%d] (ptr)0x%p = 0x%x", i, &pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg7, MinTransferSizeInOctets);
						// GPOReg status printout...
						DEBUGP(DEBUG_INFO, "             read GP0Reg0(MS) lines_written %d, CntRdy %d,  Cmplt %d CntSmpClk %d", (int)((pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg0 >> 48) & 0xFFFF),
							(int)(pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg0 >> 32) & 0x1,
							(int)(pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg0 >> 33) & 0x1,
							(int)(pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg0) & 0xffff);

					}
					else if (1 == pDaqCtrl->gblCtrl.Mode)
					{
						//diagnostic mode
						pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg2 = (pDevExt->dataHSize/8) | (pDevExt->linesPerStripe << 16); //BeatsPerLine | (pDevExt->linesPerStripe << 16)
						pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg5_1 = 0x000d0016;
						pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg5_2 = 0x0088003c;
						pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg6 = 0x0043 | (((pDevExt->dataHSize/8) - 10)<<16);//0x5b | (((pDevExt->dataHSize/8) - 10)<<16); //diagnostic mode beatsPerLine - 10
						pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg7 = 0x00000080;
					}
					else if (2 == pDaqCtrl->gblCtrl.Mode)
					{
						////diagnostic mode
						//pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg2 = 0x0004010A;
						//pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg5_1 = 0x000d0016;
						//pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg5_2 = 0x0088003c;
						//pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg6 = 0x0100005b; //diagnostic mode
						//pPacketGen->dflimProcessingRegs[i].Layout.GP0Reg7 = 0x00000080;
					}
					 
				}
			}
			else
			{
				// clean user interrupt
				pDevExt->pDmaRegisters->commonControl.ControlStatus |= 0x0000003f; 
				DEBUGP(DEBUG_INFO, "SHUTTING DMA DOWN -Clear IRQ... pDevExt->pDmaRegisters->commonControl.ControlStatus (ptr) 0x%p  |=  0x%x", &pDevExt->pDmaRegisters->commonControl.ControlStatus, 0x0000003f);
				// Reset only the S2C Engine, the C2S Engine is reset by ShutdownPacketMode call
				if (pDmaExt->DmaType == DMA_TYPE_PACKET_SEND)
				{
					// Since we are shutting down the Packet Generator/Checker reset the S2C DMA Engine.
					ShutdownDMAEngine(pDevExt, pDmaExt);
				}
			}
		}
	}
	else
	{
		//		DEBUGP(DEBUG_ERROR, "<-- WdfRequestRetrieveInputBuffer failed 0x%x", status);
	}
	return status;
}

