#include "stdafx.h"
#pragma warning(disable:4201) //I just wanna use freedom nonstandard "nameless struct/union"
#include "thordaq.h"
#include "thordaqguid.h"


THORDAQ_STATUS CThordaq::SetImagingWaveforms(
	std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT> dacCtrl,
	std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT> dacCtrl2 // use this one for 2bank acquisitions to set the second bank
)
{
	imagingDACCtrl = dacCtrl;
	imagingDACCtrl2 = dacCtrl2;
	return THORDAQ_STATUS::STATUS_SUCCESSFUL;
}


/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS CThordaq::SetImagingConfiguration( IMAGING_CONFIGURATION_STRUCT imaging_config)
 *
 * @brief	Set up data imaging configuration.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param	imaging_config	Image configuration stuct.
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/


THORDAQ_STATUS CThordaq::SetImagingConfiguration(IMAGING_CONFIGURATION_STRUCT imaging_config)
{
	DWORD						bytes_returned = 0;
	DWORD						last_error_status = 0;
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	memset(&gPtrAcqCtrl->gblCtrl, 0, sizeof(gPtrAcqCtrl->gblCtrl));
	memset(&gPtrAcqCtrl->scan, 0, sizeof(gPtrAcqCtrl->scan));
	memset(&gPtrAcqCtrl->samplingClock, 0, sizeof(gPtrAcqCtrl->samplingClock));
	memset(&gPtrAcqCtrl->streamProcessing, 0, sizeof(gPtrAcqCtrl->streamProcessing));
	memset(&gPtrAcqCtrl->adcInterface, 0, sizeof(gPtrAcqCtrl->adcInterface));
	memset(&gPtrAcqCtrl->galvoCtrl, 0, sizeof(gPtrAcqCtrl->galvoCtrl));

	_imagingLevelTriggerWentLow = false;
	_imagingLevelTriggerActive = false;

	_imagingConfiguration = imaging_config;
	if (SetGlobalSettings(imaging_config) == FALSE)
	{
		return STATUS_PARAMETER_SETTINGS_ERROR;
	}

	if (imaging_config.imageCtrl.defaultMode == 0)
	{
		//Setup Scan Subsystem Configurations
		if (SetScanSettings(imaging_config) == FALSE)
		{
			return STATUS_PARAMETER_SETTINGS_ERROR;
		}

		//put the data into the for necessary to create and load the DAC DMA descriptor table
		//into memory
		std::map<UINT, DAC_CRTL_STRUCT> dacCtrl;
		std::map<UINT, DAC_WAVE_DESC_STRUCT> dacWaveDescs;
		for (UINT i = 0; i < WAVETABLE_CHANNEL_COUNT; i++)
		{
			if (imagingDACCtrl.find(i) != imagingDACCtrl.end())
			{
				auto gctrol = imagingDACCtrl[i];
				DAC_CRTL_STRUCT dacCtrlStruct = DAC_CRTL_STRUCT();
				dacCtrlStruct.park_val = gctrol.park_val;
				dacCtrlStruct.update_rate = gctrol.update_rate;
				dacCtrlStruct.output_port = gctrol.output_port;
				dacCtrlStruct.enablePort = true;
				dacCtrlStruct.enableFilter = gctrol.enableWaveformFilter;
				dacCtrlStruct.offset_val = gctrol.offset_val;
				dacCtrlStruct.flyback_samples = gctrol.flyback_samples;
				dacCtrlStruct.filterInhibit = gctrol.filterInhibit;
				dacCtrlStruct.hSync = gctrol.hSync;
				dacCtrlStruct.enableEOFFreeze = gctrol.enableEOFFreeze;
				DAC_WAVE_GROUP_STRUCT wavestruct = DAC_WAVE_GROUP_STRUCT();
				wavestruct.waveformBuffer = gctrol.waveformBuffer;
				wavestruct.waveform_buffer_size = gctrol.waveform_buffer_size;
				wavestruct.isLoopEnd = true;
				wavestruct.isLoopStart = false;
				wavestruct.loopEnable = false;
				wavestruct.needsMemoryCleanup = false;

				DAC_WAVE_DESC_STRUCT waveDesc = DAC_WAVE_DESC_STRUCT();
				waveDesc.finishPlaybackInLastGroup = false;
				waveDesc.dacWaveformGroups.push_back(wavestruct);

				dacWaveDescs.insert({ i, waveDesc });
				dacCtrl.insert({ i, dacCtrlStruct });
			}
			else if (imaging_config.dacCtrl.find(i) == imaging_config.dacCtrl.end())
			{
				DAC_CRTL_STRUCT dacCtrlStruct = DAC_CRTL_STRUCT();
				dacCtrlStruct.park_val = 0;
				dacCtrlStruct.update_rate = DAC_DISABLED_CHANNEL_UPDATERATE;
				dacCtrlStruct.output_port = i;
				dacCtrlStruct.enablePort = false;
				dacCtrlStruct.enableFilter = false;
				dacCtrlStruct.filterInhibit = false;
				dacCtrlStruct.hSync = false;
				dacCtrlStruct.enableEOFFreeze = false;
				DAC_WAVE_GROUP_STRUCT wavestruct = DAC_WAVE_GROUP_STRUCT();
				wavestruct.waveformBuffer = NULL;
				wavestruct.waveform_buffer_size = 0;

				DAC_WAVE_DESC_STRUCT waveDesc = DAC_WAVE_DESC_STRUCT();
				waveDesc.finishPlaybackInLastGroup = false;
				waveDesc.dacWaveformGroups.push_back(wavestruct);

				dacCtrlStruct.offset_val = _dacParkingPositions[i];

				dacWaveDescs.insert({ i, waveDesc });
				dacCtrl.insert({ i, dacCtrlStruct });
			}
			else
			{
				auto gctrol = imaging_config.dacCtrl[i];
				DAC_CRTL_STRUCT dacCtrlStruct = DAC_CRTL_STRUCT();
				dacCtrlStruct.park_val = gctrol.park_val;
				dacCtrlStruct.update_rate = gctrol.update_rate;
				dacCtrlStruct.output_port = gctrol.output_port;
				dacCtrlStruct.enablePort = true;
				dacCtrlStruct.enableFilter = gctrol.enableWaveformFilter;
				dacCtrlStruct.offset_val = gctrol.offset_val;
				dacCtrlStruct.flyback_samples = gctrol.flyback_samples;
				dacCtrlStruct.filterInhibit = gctrol.filterInhibit;
				dacCtrlStruct.hSync = gctrol.hSync;
				dacCtrlStruct.enableEOFFreeze = gctrol.enableEOFFreeze;
				DAC_WAVE_GROUP_STRUCT wavestruct = DAC_WAVE_GROUP_STRUCT();
				wavestruct.waveformBuffer = gctrol.waveformBuffer;
				wavestruct.waveform_buffer_size = gctrol.waveform_buffer_size;
				wavestruct.isLoopEnd = true;
				wavestruct.isLoopStart = false;
				wavestruct.loopEnable = false;
				wavestruct.needsMemoryCleanup = false;

				DAC_WAVE_DESC_STRUCT waveDesc = DAC_WAVE_DESC_STRUCT();
				waveDesc.finishPlaybackInLastGroup = false;
				waveDesc.dacWaveformGroups.push_back(wavestruct);

				dacWaveDescs.insert({ i, waveDesc });
				dacCtrl.insert({ i, dacCtrlStruct });
			}
		}

		_isPreviewImaging = imaging_config.imageCtrl.isPreviewImaging;

		//Setup Coherent Sampling Configurations
		if (SetCoherentSampleingSettings(imaging_config) == FALSE)
		{
			return STATUS_PARAMETER_SETTINGS_ERROR;
		}

		//Setup Stream Processing
		if (SetStreamProcessingSettings(imaging_config) == FALSE)
		{
			return STATUS_PARAMETER_SETTINGS_ERROR;
		}
		bool setDACSettingsForUnusedChannels = !_dacContinuousModeEventPrepared && !_dacContinuousModeStartStopStatus;
		//Setup Analog out settings
		if (SetDACSettingsForImaging(imaging_config, dacCtrl, setDACSettingsForUnusedChannels) == FALSE)
		{
			return STATUS_PARAMETER_SETTINGS_ERROR;
		}

		if (SetImagingTriggerOptions(imaging_config) == FALSE)
		{
			return STATUS_PARAMETER_SETTINGS_ERROR;
		}

		if (imaging_config.imageCtrl.TwoBankDACDMAPlayback)
		{
			std::map<UINT, DAC_WAVE_DESC_STRUCT> dacWaveDescs2;
			for (UINT i = 0; i < WAVETABLE_CHANNEL_COUNT; i++)
			{
				if (imagingDACCtrl2.find(i) != imagingDACCtrl2.end())
				{
					auto gctrol = imagingDACCtrl2[i];

					DAC_WAVE_GROUP_STRUCT wavestruct = DAC_WAVE_GROUP_STRUCT();
					wavestruct.waveformBuffer = gctrol.waveformBuffer;// +FIFO_DELAY_SAMPLES;
					wavestruct.waveform_buffer_size = gctrol.waveform_buffer_size;// -FIFO_DELAY_SAMPLES * sizeof(USHORT);
					wavestruct.isLoopEnd = true;
					wavestruct.isLoopStart = false;
					wavestruct.loopEnable = false;
					wavestruct.needsMemoryCleanup = false;

					DAC_WAVE_DESC_STRUCT waveDesc = DAC_WAVE_DESC_STRUCT();
					waveDesc.finishPlaybackInLastGroup = false;
					waveDesc.dacWaveformGroups.push_back(wavestruct);

					dacWaveDescs2.insert({ i, waveDesc });
				}
				else if (imaging_config.dacCtrl2.find(i) == imaging_config.dacCtrl2.end())
				{
					DAC_WAVE_GROUP_STRUCT wavestruct = DAC_WAVE_GROUP_STRUCT();
					wavestruct.waveformBuffer = NULL;
					wavestruct.waveform_buffer_size = 0;

					DAC_WAVE_DESC_STRUCT waveDesc = DAC_WAVE_DESC_STRUCT();
					waveDesc.finishPlaybackInLastGroup = false;
					waveDesc.dacWaveformGroups.push_back(wavestruct);
					dacWaveDescs2.insert({ i, waveDesc });
				}
				else
				{
					auto gctrol = imaging_config.dacCtrl2[i];

					DAC_WAVE_GROUP_STRUCT wavestruct = DAC_WAVE_GROUP_STRUCT();
					wavestruct.waveformBuffer = gctrol.waveformBuffer;// +FIFO_DELAY_SAMPLES;
					wavestruct.waveform_buffer_size = gctrol.waveform_buffer_size;// -FIFO_DELAY_SAMPLES * sizeof(USHORT);
					wavestruct.isLoopEnd = true;
					wavestruct.isLoopStart = false;
					wavestruct.loopEnable = false;
					wavestruct.needsMemoryCleanup = false;

					DAC_WAVE_DESC_STRUCT waveDesc = DAC_WAVE_DESC_STRUCT();
					waveDesc.finishPlaybackInLastGroup = false;
					waveDesc.dacWaveformGroups.push_back(wavestruct);

					dacWaveDescs2.insert({ i, waveDesc });
				}
			}

			gPtrAcqCtrl->galvoCtrl.twoBankSystemEnable = true;
			gPtrAcqCtrl->galvoCtrl.bankSwitchingFrameCount = imaging_config.imageCtrl.frameNumPerTransfer - 1;

			// Load the DMA Descriptors
			if (DACBankSwitchingLoadWaveformAndDescriptors(dacWaveDescs, 0) == FALSE)
			{
				return STATUS_PARAMETER_SETTINGS_ERROR;
			}

			// Load the DMA Descriptors
			if (DACBankSwitchingLoadWaveformAndDescriptors(dacWaveDescs2, 1) == FALSE)
			{
				return STATUS_PARAMETER_SETTINGS_ERROR;
			}
		}
		else
		{
			gPtrAcqCtrl->galvoCtrl.twoBankSystemEnable = false;
			gPtrAcqCtrl->galvoCtrl.bankSwitchingFrameCount = 0;
			// Load the DMA Descriptors
			if (DACLoadWaveformAndDescriptors(dacWaveDescs, ThorDAQExperimentType::Imaging) == FALSE)
			{
				return STATUS_PARAMETER_SETTINGS_ERROR;
			}
		}
	}
	else
	{
		memset(&gPtrAcqCtrl->galvoCtrl, 0, sizeof(gPtrAcqCtrl->galvoCtrl));
	}


	if (!_dacContinuousModeStartStopStatus && !_dacContinuousModeEventPrepared)
	{
		SetDIOChannelSelection(imaging_config.imageCtrl.digitalLinesConfig);
	}

	status = APIimageAcqConfig(gPtrAcqCtrl);
	if (status == STATUS_SUCCESSFUL && imaging_config.imageCtrl.defaultMode == 0)
	{
		SCAN_LUT scanLUT;
		SetLUTSettings(scanLUT, imaging_config);
		//ExportScript(gPtrAcqCtrl, scanLUT);
	}

	if (SetImagingTriggerOptions(imaging_config) == FALSE)
	{
		return STATUS_PARAMETER_SETTINGS_ERROR;
	}

	if (SetScanSettings(imaging_config) == FALSE)
	{
		return STATUS_PARAMETER_SETTINGS_ERROR;
	}

	_imagingEventPrepared = true;
	return status;
}

LONG CThordaq::SetGlobalSettings(IMAGING_CONFIGURATION_STRUCT	imaging_config)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;
	//set up const settings
	gPtrAcqCtrl->gblCtrl.dma_engine_index = gDmaInfo.PacketRecvEngine[0];         // set DMA engine
	gPtrAcqCtrl->gblCtrl.run_stop_mode = PACKET_GEN_RUN;                       // enable acquisition bit
	gPtrAcqCtrl->gblCtrl.acq_buf_addr = (ULONG32)0x00000000;
	gPtrAcqCtrl->gblCtrl.acq_buf_chn_offset = (ULONG32)ACQ_SINGLE_CHANNEL_BUF_CAP;  // j.e. 64MB per channel, FPGA hardware limit

	// set uo the channels are enabled
	if ((imaging_config.imageCtrl.channel - MIN_CHANNEL) <= RANGE_CHANNEL) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.channel = imaging_config.imageCtrl.channel;
	}
	else
	{
		return FALSE;
	}
	// set up frame count to be acquired
	if (imaging_config.imageCtrl.frameCnt <= MAX_FRAME_NUM) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.frame_number = imaging_config.imageCtrl.frameCnt;
	}
	else
	{
		return FALSE;
	}
	// set up horizontal pixel density
	if ((imaging_config.imageCtrl.imgHSize - MIN_PIXEL_X) <= RANGE_PIXEL_X) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.hor_pix_num = imaging_config.imageCtrl.imgHSize;
	}
	else
	{
		return FALSE;
	}
	// set up vertical pixel density
	if ((imaging_config.imageCtrl.imgVSize - MIN_PIXEL_Y) <= RANGE_PIXEL_Y) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.vrt_pix_num = imaging_config.imageCtrl.imgVSize;
	}
	else
	{
		return FALSE;
	}

	//set up number of planes
	if (imaging_config.imageCtrl.numPlanes >= MIN_NUM_PLANES && imaging_config.imageCtrl.numPlanes <= MAX_NUM_PLANES)// bounds check
	{
		gPtrAcqCtrl->gblCtrl.numPlanes = imaging_config.imageCtrl.numPlanes;
	}
	else
	{
		return FALSE;
	}

	//set up frame rate
	if ((imaging_config.imageCtrl.frameNumPerSec - MIN_FRAME_RATE) <= RANGE_FRAME_RATE) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.frm_per_sec = imaging_config.imageCtrl.frameNumPerSec;
		gPtrAcqCtrl->gblCtrl.frm_per_txn = imaging_config.imageCtrl.frameNumPerTransfer;
	}
	else
	{
		return FALSE;
	}
	//set up debug mode
	if (imaging_config.imageCtrl.defaultMode - MIN_DEBUG_MODE <= RANGE_DEBUG_MODE) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.dbg_test_mode = imaging_config.imageCtrl.defaultMode; // Disable Debug Mode (only used with GUI)
	}
	else
	{
		return FALSE;
	}

	if (imaging_config.imageCtrl.defaultMode == MIN_DEBUG_MODE)
	{
		ThordaqErrChk(L"SetAllADCChannelsGain", status = SetAllADCChannelsGain(imaging_config.imageCtrl.clock_source, imaging_config.imageCtrl.ADCGain, imaging_config.imageCtrl.threePhotonMode == TRUE));
		if (status != THORDAQ_STATUS::STATUS_SUCCESSFUL)
		{
			return FALSE;
		}

		// set up ADC settings. No bounds check. default internal_80MHZ_REF
		ThordaqErrChk(L"SetClockSourceAndFrequency", status = SetClockSourceAndFrequency(imaging_config.imageCtrl.clock_source, FALSE));
		if (status != THORDAQ_STATUS::STATUS_SUCCESSFUL)
		{
			return FALSE;
		}
		//TODO: check this code
		// Affirm settings of ADCFMCInterfaceControlReg -- don't trust logic which conflates JESD "reset" functionality with setting of crucial register bits
		// "after" means values after JESD core reset
		status = FPGAregisterWRITE("JESD204B_Sysref_sync_to_laser", gPtrAcqCtrl->adcInterface.after.jesdSysRefSync);
		status = FPGAregisterWRITE("ADC_GPIO1_INT_REF_EN", gPtrAcqCtrl->adcInterface.after.gpio1IntRefEn);
		status = FPGAregisterWRITE("ADC_GPIO0_INT_REF_SEL", gPtrAcqCtrl->adcInterface.after.gpio0IntRefSel);
		status = FPGAregisterWRITE("ADC_GPIO2_L_FPGA_REF_EN", gPtrAcqCtrl->adcInterface.after.gpio2LFpgaRefEn);
		status = FPGAregisterWRITE("ADC_GPIO3_FiltAB_MODE", (gPtrAcqCtrl->adcInterface.after.gpio3Filt | (gPtrAcqCtrl->adcInterface.after.gpio4Filt << 1)));
		/*gPtrAcqCtrl->adcInterface.before.jesdCore1 = 0x1;
		gPtrAcqCtrl->adcInterface.before.jesdCore2 = 0x1;
		gPtrAcqCtrl->adcInterface.before.jesdSysRefSync = 0x0;
		gPtrAcqCtrl->adcInterface.before.jesdTestEn = 0x0;
		gPtrAcqCtrl->adcInterface.before.gpio0IntRefSel = 0x1;
		gPtrAcqCtrl->adcInterface.before.gpio1IntRefEn = 0x0;
		gPtrAcqCtrl->adcInterface.before.gpio2LFpgaRefEn = 0x1;
		gPtrAcqCtrl->adcInterface.before.gpio3Filt = 0x0;
		gPtrAcqCtrl->adcInterface.before.gpio4Filt = 0x1;*/

		// set up ADC interface settings
		ThordaqErrChk(L"SetADCInterfaceSettings", status = SetADCInterfaceSettings(imaging_config));
		if (status != THORDAQ_STATUS::STATUS_SUCCESSFUL)
		{
			return FALSE;
		}

		//Set up Scan direction, Bi-Scan mode, hardware trigger mode
		if (imaging_config.imageCtrl.scanMode < 2 && imaging_config.imageCtrl.scanDir < 2)
		{
			{
				gPtrAcqCtrl->gblCtrl.img_scan_mode = imaging_config.imageCtrl.scanMode << 1 | (imaging_config.imageCtrl.scanDir << 2);
			}
		}
		else
		{
			return FALSE;
		}
	}

	return TRUE;
}

LONG CThordaq::SetScanSettings(IMAGING_CONFIGURATION_STRUCT	imaging_config)
{
	//Set up GG/GR scan mode, frame count
	if (imaging_config.imageCtrl.system_mode < 2)
	{
		gPtrAcqCtrl->scan.sync_ctrl = (gPtrAcqCtrl->scan.sync_ctrl & 0x1D) | (imaging_config.imageCtrl.system_mode << 5);
		if (imaging_config.imageCtrl.frameCnt != (ULONG32)MAX_FRAME_NUM) // Continuously scan
		{
			gPtrAcqCtrl->scan.sync_ctrl = gPtrAcqCtrl->scan.sync_ctrl | 0x02;
			gPtrAcqCtrl->scan.frm_cnt = imaging_config.imageCtrl.frameCnt;
		}
	}
	else
	{
		return FALSE;
	}

	gPtrAcqCtrl->scan.sync_ctrl |= _captureActiveLinveInvert << 8;

	/**** Write to log Frm_Cnt & FrameCnt for trouble-shooting
	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg,MSG_SIZE,L"ThorDAQ \nFrm_Cnt: %d \nFrameCnt: %d", gPtrAcqCtrl->scan.frm_cnt, imaging_config.imageCtrl.frameCnt);
	CThordaq::LogMessage(errMsg,VERBOSE_EVENT);
	****/

	//gPtrAcqCtrl->scan.pll_fltr_ctrl   = 0x42; //Default Thordaq ADPLL Loop Filter settings
	//gPtrAcqCtrl->scan.pll_cntr_freq   = MAXULONG32 /imaging_config.imageCtrl.sample_rate  * 8000; //8khz


	gPtrAcqCtrl->scan.pll_sync_offset = max(1, imaging_config.imageCtrl.alignmentOffset);

	if (imaging_config.imageCtrl.scanMode != SCAN_MODES::BIDIRECTION_SCAN && !imaging_config.galvoGalvoCtrl.fastOneWayImaging)
	{
		gPtrAcqCtrl->scan.pll_sync_offset += _MAfilterAlignmentOffset;
	}

	//Setup Galvo Settings
	if (imaging_config.imageCtrl.system_mode == SYSTEM_MODE::INTERNAL_GALVO_GALVO)
	{
		gPtrAcqCtrl->scan.galvo_pixel_delay = static_cast<ULONG32>(round(imaging_config.galvoGalvoCtrl.pixelDelayCnt));
		gPtrAcqCtrl->scan.galvo_pixel_dwell = static_cast<ULONG32>(round(imaging_config.galvoGalvoCtrl.dwellTime * (double)SYS_CLOCK_FREQ));
		gPtrAcqCtrl->scan.galvo_intra_line_delay = static_cast<ULONG32>(round((imaging_config.galvoGalvoCtrl.turnaroundTime * (double)SYS_CLOCK_FREQ / 2.0 - 1.0) / 16.0));
		gPtrAcqCtrl->scan.galvo_intra_frame_delay = static_cast<ULONG32>(round((imaging_config.galvoGalvoCtrl.flybackTime * (double)SYS_CLOCK_FREQ - 2.0) / 16.0));// Flyback_time resgiter = (flyback time  * acq_clk_freq - 2) / 16
		gPtrAcqCtrl->scan.galvo_pre_SOF_delay = static_cast<ULONG32>(round((imaging_config.galvoGalvoCtrl.preSOFTime * (double)SYS_CLOCK_FREQ - 1.0) / 16.0));
	}
	else
	{
		gPtrAcqCtrl->scan.galvo_intra_frame_delay = static_cast<ULONG32>(ceil(imaging_config.resonantGalvoCtrl.flybackTime * imaging_config.streamingCtrl.scan_period));
		gPtrAcqCtrl->scan.galvo_pre_SOF_delay = static_cast<ULONG32>(ceil(imaging_config.resonantGalvoCtrl.preSOFTime * imaging_config.streamingCtrl.scan_period) + 1);
		gPtrAcqCtrl->scan.galvo_intra_line_delay = 0;
	}

	return TRUE;
}

LONG CThordaq::SetCoherentSampleingSettings(IMAGING_CONFIGURATION_STRUCT imaging_config)
{
	//Dont consider GG Scan with Conherent Sampling
	if (imaging_config.coherentSamplingCtrl.phaseIncrementMode > (USHORT)(0))
	{
		gPtrAcqCtrl->samplingClock.controlRegister0 = static_cast<UCHAR>  (imaging_config.coherentSamplingCtrl.phaseIncrementMode - 1);
		gPtrAcqCtrl->samplingClock.controlRegister0 |= (imaging_config.imageCtrl.threePhotonMode == TRUE) ? 0x08 : 0x00;
		gPtrAcqCtrl->samplingClock.controlRegister0 |= (imaging_config.imageCtrl.ddsEnable) ? 0x04 : 0x00;
		gPtrAcqCtrl->samplingClock.phase_offset = static_cast<USHORT> (imaging_config.coherentSamplingCtrl.phaseOffset);
		gPtrAcqCtrl->samplingClock.phase_step = static_cast<UCHAR>  (imaging_config.coherentSamplingCtrl.phaseStep);
		gPtrAcqCtrl->samplingClock.phase_limit = static_cast<USHORT> (imaging_config.coherentSamplingCtrl.phaseLimit);

		gPtrAcqCtrl->streamProcessing.pulse_interleave_offset = 0;
		gPtrAcqCtrl->streamProcessing.stream_ctrl2 = 0;
		if (imaging_config.streamingCtrl.channel_multiplexing_enabled == TRUE)
		{
			gPtrAcqCtrl->streamProcessing.stream_ctrl2 |= 0x0f;
			gPtrAcqCtrl->streamProcessing.pulse_interleave_offset = 0x11;
		}

		gPtrAcqCtrl->streamProcessing.stream_ctrl2 |= (imaging_config.imageCtrl.threePhotonMode == TRUE) ? 0x80 : 0x00;

		//3p Reverb/Multi-Plane Enable: REVERB_MP_EN
		gPtrAcqCtrl->streamProcessing.stream_ctrl2 |= (imaging_config.imageCtrl.numPlanes > 1) ? 0x40 : 0x00;

	}

	return TRUE;
}

long CThordaq::SetImagingTriggerOptions(IMAGING_CONFIGURATION_STRUCT imaging_config)
{
	THORDAQ_STATUS status;
	bool useHWTrigger = false;
	if (THORDAQ_TRIGGER_MODES::THORDAQ_SOFTWARE_START == imaging_config.triggerSettings.triggerMode ||
		(THORDAQ_HW_TRIGGER_MODES::THORDAQ_NO_TRRIGER == imaging_config.triggerSettings.hwTrigger1Mode &&
			THORDAQ_HW_TRIGGER_MODES::THORDAQ_NO_TRRIGER == imaging_config.triggerSettings.hwTrigger2Mode &&
			false == imaging_config.triggerSettings.enableInternalDigitalTrigger)
		)
	{
		status = FPGAregisterWRITE("ImageAcqPT_HWSW_SEL", 0x0);
	}
	else
	{
		useHWTrigger = true;

		//if only using 1 hardware trigger, set the other trigger select to use the same trigger 
		if (THORDAQ_HW_TRIGGER_MODES::THORDAQ_NO_TRRIGER == imaging_config.triggerSettings.hwTrigger1Mode && THORDAQ_HW_TRIGGER_MODES::THORDAQ_NO_TRRIGER != imaging_config.triggerSettings.hwTrigger2Mode)
		{
			status = FPGAregisterWRITE("ImageAcqPT_HW_In1_SEL", imaging_config.triggerSettings.hwTrigger2Selection);
		}
		else
		{
			status = FPGAregisterWRITE("ImageAcqPT_HW_In1_SEL", imaging_config.triggerSettings.hwTrigger1Selection);
		}

		if (THORDAQ_HW_TRIGGER_MODES::THORDAQ_NO_TRRIGER == imaging_config.triggerSettings.hwTrigger2Mode && THORDAQ_HW_TRIGGER_MODES::THORDAQ_NO_TRRIGER != imaging_config.triggerSettings.hwTrigger1Mode)
		{
			status = FPGAregisterWRITE("ImageAcqPT_HW_In2_SEL", imaging_config.triggerSettings.hwTrigger1Selection);
		}
		else
		{
			status = FPGAregisterWRITE("ImageAcqPT_HW_In2_SEL", imaging_config.triggerSettings.hwTrigger2Selection);
		}

		status = FPGAregisterWRITE("ImageAcqPT_DO_WaveformIN_SEL", imaging_config.triggerSettings.internalDigitalTrigger);

		status = FPGAregisterWRITE("ImageAcqPT_HWSW_SEL", 0x1);
		for (BYTE i = 0; i < 32; ++i)
		{

			bool do0 = false;
			bool do1 = false;

			TriggerLogicResponse(imaging_config.triggerSettings, i, do0, do1);
			BYTE do0do1 = (BYTE)do0 | (BYTE)do1 << 1;
			status = FPGAregisterWRITE("ImageAcqPT_InputCfgIndx", i);
			status = FPGAregisterWRITE("ImageAcqPT_OutCfgFn", do0do1);

			status = FPGAregisterWRITE("ImageAcqPT_CfgWriteSTROBE", 0x0);
			status = FPGAregisterWRITE("ImageAcqPT_CfgWriteSTROBE", 0x1);
		}

		status = FPGAregisterWRITE("ImageAcqPT_CfgWriteSTROBE", 0x0);
	}

	if (useHWTrigger && THORDAQ_HW_TRIGGER_MODES::THORDAQ_HARDWARE_LEVEL_TRIGGER_MODE == imaging_config.triggerSettings.hwTrigger1Mode ||
		THORDAQ_HW_TRIGGER_MODES::THORDAQ_HARDWARE_LEVEL_TRIGGER_MODE == imaging_config.triggerSettings.hwTrigger2Mode)
	{
		_imagingLevelTriggerActive = true;
	}
	else
	{
		_imagingLevelTriggerActive = false;
	}
	return status == THORDAQ_STATUS::STATUS_SUCCESSFUL;
}


//TODO: add Reverb downsample rate register
LONG CThordaq::SetStreamProcessingSettings(IMAGING_CONFIGURATION_STRUCT	imaging_config)
{
	double pixel_frequency = 0;
	double sampleRate = imaging_config.imageCtrl.clockRate;
	double lineTime = 0;
	gPtrAcqCtrl->streamProcessing.ThreePhotonMode = imaging_config.imageCtrl.threePhotonMode;
	if (imaging_config.imageCtrl.system_mode == SYSTEM_MODE::INTERNAL_GALVO_GALVO) // GG Settings
	{
		gPtrAcqCtrl->streamProcessing.stream_ctrl = 0x02; // Enable DC Offset Correction pre-FIR filter
		pixel_frequency = 1.0 / imaging_config.galvoGalvoCtrl.dwellTime;
		double lineSweepTime = ((double)gPtrAcqCtrl->scan.galvo_pixel_dwell * (double)gPtrAcqCtrl->gblCtrl.hor_pix_num) + ((double)gPtrAcqCtrl->scan.galvo_pixel_delay * 16.0 + 1.0) * ((double)(gPtrAcqCtrl->gblCtrl.hor_pix_num - 1)); // (Dwell Time + Pixel Delay Time) * PixelX - Pixel Delay Time
		double turnAroundTime = 2.0 * ((double)gPtrAcqCtrl->scan.galvo_intra_line_delay * 16.0 + 1.0);
		lineTime = (lineSweepTime + turnAroundTime) / (double)SYS_CLOCK_FREQ;
		/*wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg,MSG_SIZE,L"ThorDAQGalvoGalvo SetStreamProcessingSettings \n LineTime: %f", lineTime);
		LogMessage(errMsg,ERROR_EVENT);	*/
	}
	else // GR Settings
	{
		gPtrAcqCtrl->streamProcessing.stream_ctrl = 0x03; // enable Scan Period Source and DC Offset Correction pre-FIR filter
		lineTime = 1.0 / 2.0 / imaging_config.streamingCtrl.scan_period;
		pixel_frequency = 1.0 / (lineTime / static_cast<double>(imaging_config.imageCtrl.imgHSize));
	}
	_MAfilterAlignmentOffset = 0;
	if (imaging_config.imageCtrl.threePhotonMode) //this part can be integrated into main logic. This block is for test purpose.
	{
		/*
			---"DownSample" rate explanation for 3P and Reverb, for more details refer to the user Guide---
			The FPGA just uses the downsample to measure where the line-trigger falls between 2 laser pulses.
			The FPGA processes 4 ADC samples at a time...in 3p, every ADC sample is also a laser pulse...
			so between 2 FPGA cycles 4 laser pulses have occurred.  The FPGA divides the time between when
			it receives 4 laser pulses into quads, and aligns the line trigger to the closest laser pulse.
			In Reverb, the # of laser pulses in an FPGA cycle is not 4, but rather 2 or 1 or alternateing 2,1
			or alternating 1,0...so it has to have logic to sort all that out.
		*/
		long pulsesPerPixel = 1;
		double singlePulseDwell = 1000000.0 / static_cast<double>(sampleRate - 80);
		double currentDwellTimeUS = imaging_config.galvoGalvoCtrl.dwellTime * 1000000.0;
		USHORT downSampleFactor = 0;

		if (singlePulseDwell < currentDwellTimeUS)
		{
			pulsesPerPixel = static_cast<long>(ceil(currentDwellTimeUS / singlePulseDwell - 0.5));
		}
		pixel_frequency = 1.0 / (imaging_config.galvoGalvoCtrl.dwellTime + (((double)gPtrAcqCtrl->scan.galvo_pixel_delay * 16.0 + 1.0) / (double)SYS_CLOCK_FREQ));
		lineTime = imaging_config.galvoGalvoCtrl.lineTime;
		gPtrAcqCtrl->streamProcessing.scan_period = 32768;

		if (0 == imaging_config.imageCtrl.enableDownsampleRateChange)
		{
			if (2 == imaging_config.imageCtrl.numPlanes)
			{
				//TODO: need to update downsampleFactor formulas for multiplane above to use the clockreference
				downSampleFactor = static_cast<USHORT>(round((2.0 / 5.0) * ((double)SYS_CLOCK_FREQ / (double)sampleRate)));
			}
			else if (3 <= imaging_config.imageCtrl.numPlanes)
			{
				//TODO: need to update downsampleFactor formulas for multiplane above to use the clockreference
				downSampleFactor = static_cast<USHORT>(round((1.0 / 5.0) * ((double)SYS_CLOCK_FREQ / (double)sampleRate)));
			}
			else
			{
				downSampleFactor = static_cast<USHORT>(round((double)imaging_config.imageCtrl.clockReference / (double)sampleRate));

				//formulas for 2 and 3 pulses per pixel were found by trial and error but they are necessary to both have a smooth image, and image on 2way in many
				//pixel densities where the acquisition would time out if the downsample rate is not correct.
				if (2 == pulsesPerPixel)
				{
					downSampleFactor = 2 * (static_cast<USHORT>(round((double)imaging_config.imageCtrl.clockReference / (double)sampleRate) + 1)) + 5;
				}
				if (3 == pulsesPerPixel)
				{
					downSampleFactor = static_cast<USHORT>(round(4.0 * (double)imaging_config.imageCtrl.clockReference / (double)sampleRate)) + 1;
				}
			}

			if (downSampleFactor > 4095)
			{
				downSampleFactor = 4095;
			}
		}
		else
		{
			downSampleFactor = imaging_config.imageCtrl.downSampleRate;
		}


		//now we want to capture every 12ns as oppose to 6ns, which is why we are now setting it to 1
		if (imaging_config.imageCtrl.numPlanes > 1)
		{
			//Reverb has worked by defaulting to 0 on this register
			//Set to 0 for multiplane, in the future this will be a setting that can be set by the customer
			//0 is 6.25ns, 1 is 12.5ns and so on
			gPtrAcqCtrl->streamProcessing.threePhoton_reverb_downsample_rate = 0;
		}
		else
		{
			gPtrAcqCtrl->streamProcessing.threePhoton_reverb_downsample_rate = 0;
		}

		gPtrAcqCtrl->streamProcessing.downsample_rate = downSampleFactor - 1;

		gPtrAcqCtrl->streamProcessing.downsample_rate2 = 0;

		gPtrAcqCtrl->streamProcessing.threePhoton_sample_offset0 = static_cast<UCHAR>(imaging_config.imageCtrl.threePhotonPhaseAlignment[0]);
		gPtrAcqCtrl->streamProcessing.threePhoton_sample_offset1 = static_cast<UCHAR>(imaging_config.imageCtrl.threePhotonPhaseAlignment[1]);
		gPtrAcqCtrl->streamProcessing.threePhoton_sample_offset2 = static_cast<UCHAR>(imaging_config.imageCtrl.threePhotonPhaseAlignment[2]);
		gPtrAcqCtrl->streamProcessing.threePhoton_sample_offset3 = static_cast<UCHAR>(imaging_config.imageCtrl.threePhotonPhaseAlignment[3]);

		gPtrAcqCtrl->streamProcessing.threePhoton_reverb_MP_cycles = static_cast<UCHAR>(imaging_config.imageCtrl.numPlanes - 1) | static_cast<UCHAR>(imaging_config.imageCtrl.numPlanes - 1) << 4;

		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"CThordaq 3P Downsample Rate: %ld, downSampleFactor: %d, enableDownsampleRateChange: %ld, sampleRate: %f, clockReference: %f", gPtrAcqCtrl->streamProcessing.downsample_rate, downSampleFactor, imaging_config.imageCtrl.enableDownsampleRateChange, sampleRate, (double)imaging_config.imageCtrl.clockReference);
		LogMessage(errMsg, VERBOSE_EVENT);

		if (SetFIRFilterSettings3P(sampleRate, pixel_frequency, imaging_config) == FALSE)
		{
			return FALSE;
		}
		gPtrAcqCtrl->streamProcessing.enableMovingAverageFilter = false; //use FIR for 3P
		return TRUE;
	}
	else
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"CThordaq Non3P Downsample Rate: %ld, downSampleFactor: %d, enableDownsampleRateChange: %ld, sampleRate: %f", gPtrAcqCtrl->streamProcessing.downsample_rate, imaging_config.imageCtrl.enableDownsampleRateChange, sampleRate);
		LogMessage(errMsg, VERBOSE_EVENT);
	}

	//then we calculate the downsampled rate
	while (lineTime * sampleRate > USHRT_MAX)//16Bits
	{
		sampleRate = sampleRate / 2.0;
	}

	FIRFilterMode fir_mode = imaging_config.imageCtrl.clock_source == EXTERNAL_CLOCK_SOURCE ? EXTERNAL_CLOCK_MODE : INTERNAL_CLOCK_MODE;

	// Setup Fir Filters
	if (SetFIRFilterSettings(fir_mode, sampleRate, pixel_frequency, imaging_config) == FALSE)
	{
		return FALSE;
	}

	if (imaging_config.imageCtrl.system_mode == SYSTEM_MODE::INTERNAL_GALVO_GALVO) // GG Settings
	{
		double maxSampleRate = sampleRate;

		ULONG32 downSampleFactor = static_cast<ULONG32>(ADC_MAX_SAMPLE_RATE / sampleRate);



		bool useMAFilter = false;

		downSampleFactor = downSampleFactor > 0 ? downSampleFactor : 1;
		ULONG32 downSampleFactor2 = 1;

		double rate = ADC_MAX_SAMPLE_RATE / (16.0);
		double lineSweepTime = ((double)gPtrAcqCtrl->scan.galvo_pixel_dwell * (double)gPtrAcqCtrl->gblCtrl.hor_pix_num) / (double)SYS_CLOCK_FREQ + ((double)gPtrAcqCtrl->scan.galvo_pixel_delay * 16.0 + 1.0) * ((double)(gPtrAcqCtrl->gblCtrl.hor_pix_num - 1)) / (double)SYS_CLOCK_FREQ; // (Dwell Time + Pixel Delay Time) * PixelX - Pixel Delay Time

		double dwellTime = lineSweepTime / gPtrAcqCtrl->gblCtrl.hor_pix_num;

		double samplesPerPixel = dwellTime * rate;

		if (samplesPerPixel > 16 && imaging_config.imageCtrl.movingAverageFilterEnable)
		{
			useMAFilter = true;
			//downSampleFactor2 = static_cast<ULONG32>(round(downSampleFactor / 16.0));
			//if (imaging_config.imageCtrl.scanMode == SCAN_MODES::BIDIRECTION_SCAN)
			//{
			//	downSampleFactor2 = static_cast<ULONG32>(round(downSampleFactor / 8.0));
			//	downSampleFactor = 8;
			//}
			//else

			if (samplesPerPixel > 16)
			{
				if (samplesPerPixel < 32)
				{
					downSampleFactor = 16;
					downSampleFactor2 = 1;
				}
				else if (samplesPerPixel < 64)
				{
					downSampleFactor = 16;
					downSampleFactor2 = 2;
				}
				else if (samplesPerPixel < 128)
				{
					downSampleFactor = 16;
					downSampleFactor2 = 4;
				}
				else if (samplesPerPixel < 256)
				{
					downSampleFactor = 16;
					downSampleFactor2 = 8;
				}
				else if (samplesPerPixel < 512)
				{
					downSampleFactor = 16;
					downSampleFactor2 = 16;
				}
				else if (samplesPerPixel < 1024)
				{
					downSampleFactor = 16;
					downSampleFactor2 = 32;
				}
				else if (samplesPerPixel < 1024)
				{
					downSampleFactor = 16;
					downSampleFactor2 = 64;
				}
				else
				{
					downSampleFactor = 16;
					downSampleFactor2 = 128;
				}

				sampleRate = ADC_MAX_SAMPLE_RATE / (downSampleFactor * downSampleFactor2);

			}
			else
			{
				downSampleFactor2 = static_cast<ULONG32>(round(downSampleFactor / 16.0));
				downSampleFactor = 16;
			}

			double rate = ADC_MAX_SAMPLE_RATE / (downSampleFactor);
			double lineSweepTime = ((double)gPtrAcqCtrl->scan.galvo_pixel_dwell * (double)gPtrAcqCtrl->gblCtrl.hor_pix_num) / (double)SYS_CLOCK_FREQ + ((double)gPtrAcqCtrl->scan.galvo_pixel_delay * 16.0 + 1.0) * ((double)(gPtrAcqCtrl->gblCtrl.hor_pix_num - 1)) / (double)SYS_CLOCK_FREQ; // (Dwell Time + Pixel Delay Time) * PixelX - Pixel Delay Time

			double dwellTime = lineSweepTime / gPtrAcqCtrl->gblCtrl.hor_pix_num;

			double samplesPerPixel = dwellTime * rate;

			double lineSample = lineTime * rate;

			int spp = downSampleFactor * downSampleFactor2;
			const int ALIGNMENT_MULTIPLIER = 32;
			if (spp < 32)
			{
				gPtrAcqCtrl->streamProcessing.movingAverageFilterLength = MA_16;
				_MAfilterAlignmentOffset = 0;
			}
			else if (spp < 64)
			{
				gPtrAcqCtrl->streamProcessing.movingAverageFilterLength = MA_32;
				_MAfilterAlignmentOffset = 8 * ALIGNMENT_MULTIPLIER;
			}
			else if (spp < 128)
			{
				gPtrAcqCtrl->streamProcessing.movingAverageFilterLength = MA_64;
				_MAfilterAlignmentOffset = 16 * ALIGNMENT_MULTIPLIER;
			}
			else if (spp < 256)
			{
				gPtrAcqCtrl->streamProcessing.movingAverageFilterLength = MA_128;
				_MAfilterAlignmentOffset = 32 * ALIGNMENT_MULTIPLIER;
			}
			else if (spp < 512)
			{
				gPtrAcqCtrl->streamProcessing.movingAverageFilterLength = MA_256;
				_MAfilterAlignmentOffset = 64 * ALIGNMENT_MULTIPLIER;
			}
			else if (spp < 1024)
			{
				gPtrAcqCtrl->streamProcessing.movingAverageFilterLength = MA_512;
				_MAfilterAlignmentOffset = 128 * ALIGNMENT_MULTIPLIER;
			}
			else if (spp < 2048)
			{
				gPtrAcqCtrl->streamProcessing.movingAverageFilterLength = MA_1024;
				_MAfilterAlignmentOffset = 256 * ALIGNMENT_MULTIPLIER;
			}
			else
			{
				gPtrAcqCtrl->streamProcessing.movingAverageFilterLength = MA_2048;
				_MAfilterAlignmentOffset = 512 * ALIGNMENT_MULTIPLIER;
			}

			wchar_t logMsg[MSG_SIZE];
			StringCbPrintfW(logMsg, MSG_SIZE, L"MA length = %d, downSampleFactor2 = %ld, downSampleFactor1 = %ld", gPtrAcqCtrl->streamProcessing.movingAverageFilterLength, downSampleFactor2, downSampleFactor);
			CThordaq::LogMessage(logMsg, ERROR_EVENT);
		}
		else
		{
			useMAFilter = false;
			//downSampleFactor2 = static_cast<ULONG32>(round(downSampleFactor / 16.0));
			//downSampleFactor = 16;

			wchar_t logMsg[MSG_SIZE];
			StringCbPrintfW(logMsg, MSG_SIZE, L"NO MA filter, downSampleFactor2 = %ld, downSampleFactor1 = %ld, dwelltime = %lf", downSampleFactor2, downSampleFactor, dwellTime);
			CThordaq::LogMessage(logMsg, ERROR_EVENT);
		}

		gPtrAcqCtrl->streamProcessing.enableMovingAverageFilter = useMAFilter;


		gPtrAcqCtrl->streamProcessing.downsample_rate = downSampleFactor - 1; /** 16843009*/ //* (1+2^8+2^16+2^24);
		gPtrAcqCtrl->streamProcessing.downsample_rate2 = downSampleFactor2 - 1; /** 16843009*/ //* (1+2^8+2^16+2^24);
	}
	else //GR Downsampling rate
	{
		// From Eric, downsample should be 1 when external clock is enabled.
		gPtrAcqCtrl->streamProcessing.downsample_rate = (CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE == imaging_config.imageCtrl.clock_source) ? 1 : 0;
		gPtrAcqCtrl->streamProcessing.enableMovingAverageFilter = false; //use FIR for GR
		gPtrAcqCtrl->streamProcessing.downsample_rate2 = 0;
	}

	gPtrAcqCtrl->streamProcessing.scan_period = static_cast<USHORT>(round(lineTime * sampleRate));          //Round the value and set in the galvo galvo system
	return TRUE;
}

/**********************************************************************************************//**
 * @fn	LONG CThordaq::SetLUTSettings(IMAGING_CONFIGURATION_STRUCT imaging_config )
 *
 * @brief	Set up acquisition Look Up Table.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param	imaging_config	Image configuration stuct.
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

LONG CThordaq::SetLUTSettings(SCAN_LUT& scanLUT, IMAGING_CONFIGURATION_STRUCT	imaging_config)
{
	DWORD						bytes_returned = 0;
	DWORD						last_error_status = 0;
	//Initiate overlapped structure

	scanLUT.ch = MAX_CHANNEL_COUNT;//static_cast<USHORT>(pAcqCfg->channel);
	memset(scanLUT.lut, 0, sizeof(USHORT) * SCAN_LUT_MAX_LEN);
	const USHORT SAMPLES_MULTIPLE = 2;
	if (imaging_config.imageCtrl.system_mode == SYSTEM_MODE::INTERNAL_GALVO_GALVO)
	{
		double lineSweepTime = ((double)gPtrAcqCtrl->scan.galvo_pixel_dwell * (double)gPtrAcqCtrl->gblCtrl.hor_pix_num) / (double)SYS_CLOCK_FREQ + ((double)gPtrAcqCtrl->scan.galvo_pixel_delay * 16.0 + 1.0) * ((double)(gPtrAcqCtrl->gblCtrl.hor_pix_num - 1)) / (double)SYS_CLOCK_FREQ; // (Dwell Time + Pixel Delay Time) * PixelX - Pixel Delay Time
		double turnAroundTime = 2.0 * ((double)gPtrAcqCtrl->scan.galvo_intra_line_delay * 16.0 + 1.0) / (double)SYS_CLOCK_FREQ;
		double samples_idle = (2.0 * ((double)gPtrAcqCtrl->scan.galvo_intra_line_delay * 16.0 + 1.0) / (double)SYS_CLOCK_FREQ - lineSweepTime) / 2.0;

		if (imaging_config.imageCtrl.scanMode == SCAN_MODES::BIDIRECTION_SCAN || imaging_config.galvoGalvoCtrl.fastOneWayImaging)
		{
			samples_idle = turnAroundTime; // The offset for the scan is one samples_idle/acceleration
		}

		// For 3P the LUT table is different, it needs to be built skipping the number of pulses per pixel for each table value
		// it is better to build it based on the number of pulses than the way 2P is done which is based on the time per pixel / line time
		if (imaging_config.imageCtrl.threePhotonMode)
		{
			int count = 0;
			int j = 0;
			if (gPtrAcqCtrl->gblCtrl.numPlanes > 1)
			{
				//firstSampleLaserCycles, sample_of_start, first lutVal equations provided by Bill Radtke
				UINT16 firstSampleLaserCycles = (UINT16)floor(samples_idle / 2.0 * imaging_config.imageCtrl.clockRate);

				UINT16 intervalNumberSamples = SAMPLES_MULTIPLE;

				UINT16 lutVal = 0;
				if (2 == gPtrAcqCtrl->gblCtrl.numPlanes || 4 == gPtrAcqCtrl->gblCtrl.numPlanes)
				{
					UINT16 sample_of_start = (firstSampleLaserCycles - 1) * (UINT8)gPtrAcqCtrl->gblCtrl.numPlanes - 2;
					lutVal = (UINT16)round((sample_of_start * 65536.0 - 2047.0) / 32768.0) - 2;
				}
				else if (3 == gPtrAcqCtrl->gblCtrl.numPlanes || 5 == gPtrAcqCtrl->gblCtrl.numPlanes)
				{
					UINT16 sample_of_start = (firstSampleLaserCycles - 1) * (UINT8)gPtrAcqCtrl->gblCtrl.numPlanes - 2;
					lutVal = (UINT16)round((sample_of_start * 65536.0 - 2047.0) / 32768.0) - 0;
				}
				else
				{
					UINT16 sample_of_start = (firstSampleLaserCycles - 1) * (UINT8)gPtrAcqCtrl->gblCtrl.numPlanes + gPtrAcqCtrl->gblCtrl.numPlanes - 1;
					lutVal = (UINT16)round((sample_of_start * 65536.0 - 2047.0) / 32768.0) + 4 - (UINT8)(gPtrAcqCtrl->gblCtrl.numPlanes - 6) * 4;
				}

				for (long i = 0; i < (long)SCAN_LUT_MAX_LEN / (long)gPtrAcqCtrl->gblCtrl.numPlanes; ++i)
				{
					//There is a quantization in the math, for 4096 pixels, that required this. - Bill Radtke
					if (lutVal > 32767 && 0 == count)
					{
						count++;
						lutVal++;
					}
					memcpy(&(scanLUT.lut[j]), &lutVal, sizeof(USHORT));
					lutVal += intervalNumberSamples;
					++j;
					if (gPtrAcqCtrl->gblCtrl.numPlanes > 1)
					{
						for (long k = 1; k < (long)gPtrAcqCtrl->gblCtrl.numPlanes; ++k)
						{
							//There is a quantization in the math, for 4096 pixels, that required this. - Bill Radtke
							if (lutVal > 32767 && 0 == count)
							{
								++count;
								++lutVal;
							}

							memcpy(&(scanLUT.lut[j]), &lutVal, sizeof(USHORT));
							lutVal += intervalNumberSamples;
							++j;
						}
					}
				}
			}
			else
			{
				double intervalTime = imaging_config.galvoGalvoCtrl.dwellTime + (((double)gPtrAcqCtrl->scan.galvo_pixel_delay * 16.0 + 1.0) / (double)SYS_CLOCK_FREQ);
				UINT16 sample_of_start = (UINT16)floor(samples_idle / 2.0 * imaging_config.imageCtrl.clockRate);
				long intervalNumberSamples = static_cast<long>(round(intervalTime * (static_cast<double>(imaging_config.imageCtrl.clockRate))));
				intervalNumberSamples *= SAMPLES_MULTIPLE;
				sample_of_start *= SAMPLES_MULTIPLE;
				INT32 val = sample_of_start >= static_cast<UINT16>(intervalNumberSamples) ? sample_of_start - static_cast<UINT16>(intervalNumberSamples) : 0;
				val += imaging_config.galvoGalvoCtrl.sampleOffsetStartLUT3PTI;
				wchar_t logMsg[MSG_SIZE];
				StringCbPrintfW(logMsg, MSG_SIZE, L"LUT total offset = %d", val);
				CThordaq::LogMessage(logMsg, ERROR_EVENT);

				for (long i = 0; i < (long)SCAN_LUT_MAX_LEN / (long)gPtrAcqCtrl->gblCtrl.numPlanes; ++i)
				{
					val += static_cast<UINT16>(intervalNumberSamples);

					//There is a quantization in the math, for 4096 pixels, that required this. - Bill Radtke
					if (val > 32767 && 0 == count)
					{
						count++;
						val++;
					}
					memcpy(&(scanLUT.lut[j]), &val, sizeof(USHORT));
					++j;
					if (gPtrAcqCtrl->gblCtrl.numPlanes > 1)
					{
						for (long k = 1; k < (long)gPtrAcqCtrl->gblCtrl.numPlanes; ++k)
						{
							//There is a quantization in the math, for 4096 pixels, that required this. - Bill Radtke
							if (val > 32767 && 0 == count)
							{
								count++;
								val++;
							}

							val += SAMPLES_MULTIPLE;
							memcpy(&(scanLUT.lut[j]), &val, sizeof(USHORT));
							++j;
						}
					}
				}
			}
		}
		else // (NOT 3P mode...)
		{
			double lineTime;
			if (imaging_config.imageCtrl.scanMode != SCAN_MODES::BIDIRECTION_SCAN && !imaging_config.galvoGalvoCtrl.fastOneWayImaging)
			{
				samples_idle = (2.0 * imaging_config.galvoGalvoCtrl.pureTurnAroundTime);
				lineTime = lineSweepTime * 2 + samples_idle * 2;
			}
			else
			{
				lineTime = lineSweepTime + samples_idle;
			}

			if (lineTime <= 0)
			{
				return FALSE;
			}

			double intervalTime = ((double)gPtrAcqCtrl->scan.galvo_pixel_dwell + ((double)gPtrAcqCtrl->scan.galvo_pixel_delay * 16.0 + 1.0)) / (double)SYS_CLOCK_FREQ;
			//wchar_t errMsg[MSG_SIZE];
			//StringCbPrintfW(errMsg,MSG_SIZE,L"ThorDAQGalvoGalvo SetLUTSettings \n LineTime: %f", lineTime);
			//LogMessage(errMsg,ERROR_EVENT);		

			/*UINT16 sample_of_interval = (UINT16)(intervalTime / lineTime * USHRT_MAX);
			UINT16 sample_of_start = (UINT16)(turnAroundTime / 2.0 / lineTime * USHRT_MAX);
			if (imaging_config.imageCtrl.threePhotonMode)
			{
				intervalTime = 1.0 / (double)imaging_config.imageCtrl.sample_rate;
			}*/
			//double lineSweepTime = ((double)gPtrAcqCtrl->scan.galvo_pixel_dwell * (double)gPtrAcqCtrl->gblCtrl.hor_pix_num) / (double)SYS_CLOCK_FREQ + ((double)gPtrAcqCtrl->scan.galvo_pixel_delay * 16.0 + 1.0) * ((double)(gPtrAcqCtrl->gblCtrl.hor_pix_num - 1)) / (double)SYS_CLOCK_FREQ;


			double rate = ADC_MAX_SAMPLE_RATE / (gPtrAcqCtrl->streamProcessing.downsample_rate + 1);
			double dwellTime = lineSweepTime / gPtrAcqCtrl->gblCtrl.hor_pix_num;

			double samplesPerPixel = dwellTime * rate;

			double lineSample = lineTime * rate;

			double samplesIdleSamples = samples_idle * rate;

			double multiplier = 1;

			//if (gPtrAcqCtrl->streamProcessing.enableMovingAverageFilter)
			//{
			//	multiplier = imaging_config.imageCtrl.movingAverageMultiplier;
			//}
			//UINT16 sample_of_start = (UINT16)round((samplesIdleSamples / 2.0 / lineTime * (double)(USHRT_MAX + 1))); //Shift to the real start of the waveformBuffer, after one samples_idle to match the waveformBuffer timing.
			UINT16 sample_of_start = (UINT16)round(multiplier * (samples_idle / 2.0 / lineTime * (double)USHRT_MAX)); //Shift to the real start of the waveformBuffer, after one samples_idle to match the waveformBuffer timing.
			if (imaging_config.imageCtrl.scanMode == SCAN_MODES::BIDIRECTION_SCAN || imaging_config.galvoGalvoCtrl.fastOneWayImaging)
			{
				sample_of_start += 2 * _MAfilterAlignmentOffset / 32;
			}
			//UINT16 sample_of_start = (USHRT_MAX - sample_of_interval * gPtrAcqCtrl->gblCtrl.hor_pix_num) / 2 ; 
			//double starRecordTime = turnAroundTime / 2.0;
			for (int i = 0; i < (int)gPtrAcqCtrl->gblCtrl.hor_pix_num; ++i)
			{
				//160MSPS / 6.25ns
				// Calculate the closest pixel sample position. Need to round at this level for better sample accuracy. Otherwise we have a cumulative error.
			//	UINT16 pixelSamplePosition = (UINT16)(round((double)i * samplesPerPixel / lineSample * (double)(USHRT_MAX + 1)));
				UINT16 pixelSamplePosition = (UINT16)(multiplier * round((double)i * intervalTime / lineTime * (double)USHRT_MAX));

				UINT16 val = sample_of_start + pixelSamplePosition;
				//UINT16 val = static_cast<UINT16>(floor(0.5 + (starRecordTime  + (double)(i + 0.5) * (((double)gPtrAcqCtrl->scan.galvo_pixel_dwell + ((double)gPtrAcqCtrl->scan.galvo_pixel_delay * 16.0 + 1.0))/ SYS_CLOCK_FREQ))/lineTime * USHRT_MAX)); 
				memcpy(&(scanLUT.lut[i]), &val, sizeof(USHORT));
			}
		}
	}
	else if (imaging_config.imageCtrl.system_mode == SYSTEM_MODE::INTERNAL_RESONANT_GALVO)
	{
		for (ULONG i = 0; i < (int)gPtrAcqCtrl->gblCtrl.hor_pix_num; ++i)
		{
			//TODO: figure out what the right equation to calculate the sample should be
			double phi = (-M_PI / gPtrAcqCtrl->gblCtrl.hor_pix_num * i) + (M_PI / 2 * (1.0 - 1.0 / gPtrAcqCtrl->gblCtrl.hor_pix_num));
			UINT16 val = static_cast<UINT16>(0.5 + (acos(1.0 - (2.0 * i + 1.0) / gPtrAcqCtrl->gblCtrl.hor_pix_num) / M_PI) * USHRT_MAX);	//round
			memcpy(&(scanLUT.lut[i]), &val, sizeof(USHORT));
		}
	}

	scanLUT.ch = 4;
	THORDAQ_STATUS tdStatus = API_ADCsampleImagizerLUT(PS2MM_ADCSAMPLE_LUT(&scanLUT.lut[0]));
	return tdStatus == STATUS_SUCCESSFUL ? TRUE : FALSE;
}

LONG CThordaq::SetDACSettingsForImaging(IMAGING_CONFIGURATION_STRUCT	imaging_config, std::map<UINT, DAC_CRTL_STRUCT> dacCtrl, bool setDacSettingsForUnusedChannels)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	ULONG64 max_flyback_samples = 80;

	for (int i = 0; i < WAVETABLE_CHANNEL_COUNT; i++)
	{
		DAC_CRTL_STRUCT dac_setting = dacCtrl[i];
		if (dac_setting.output_port == 10)
		{
			dac_setting.output_port = 11;
		}
		else if (dac_setting.output_port == 11)
		{
			dac_setting.output_port = 10;
		}


		max_flyback_samples = min(MAX_FLYBACK_DAC_SAMPLES, max(max_flyback_samples, dac_setting.flyback_samples));

		if (setDacSettingsForUnusedChannels || dac_setting.enablePort)
		{
			if (dac_setting.enablePort)
			{
				//disable continuous mode for enabled channels or unused channel
				if (DACEnableDisableContinuousModeOnChannel(i, false) != THORDAQ_STATUS::STATUS_SUCCESSFUL)
				{
					status = STATUS_PARAMETER_SETTINGS_ERROR;
					break;
				}

				if (DACEnableDisablePerChannelRunStopOnChannel(i, false) != THORDAQ_STATUS::STATUS_SUCCESSFUL)
				{
					status = STATUS_PARAMETER_SETTINGS_ERROR;
					break;
				}
				
			}

			SetDACPerChannelSettingsForImaging(i, dac_setting);

			_isDACChannelEnabledForImaging[i] = dac_setting.enablePort;

		}
	}

	USHORT bankSwitchingFrameCount = imaging_config.imageCtrl.frameNumPerTransfer - 1;

	status = FPGAregisterWRITE("DAC_Bank_Switch_EN", imaging_config.imageCtrl.TwoBankDACDMAPlayback);

	status = FPGAregisterWRITE("DAC_SLOW_RATE", imaging_config.imageCtrl.dacSlowMoveToOffset);

	status = FPGAregisterWRITE("DAC_Bank_Sel_Frame_CNT", bankSwitchingFrameCount);

	status = FPGAregisterWRITE("DACWaveGen_EOF_DelaySampleCnt", max_flyback_samples); // 0x2B8

	if (setDacSettingsForUnusedChannels)
	{
		UINT8 DmaChannelCount = WAVETABLE_CHANNEL_COUNT - 1;
		status = FPGAregisterWRITE("DAC_DMA_Chans_Active", DmaChannelCount);
	}

	return TRUE;
}

LONG CThordaq::SetDACPerChannelSettingsForImaging(UINT channel, DAC_CRTL_STRUCT dacCtrl)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	const USHORT offset_mid = dacCtrl.offset_val > 0 ? 0x7fff : 0x8000;
	USHORT offset = static_cast<USHORT>(round(dacCtrl.offset_val / GALVO_RESOLUTION) + offset_mid); // round offset to nearest position
	USHORT updateRate = static_cast<USHORT>(round((double)SYS_CLOCK_FREQ / dacCtrl.update_rate - 1.0)); // update rate should be floored. Or maybe it shouldn't?

	string dacOffset = "DAC_Offset_Chan" + std::to_string(channel);
	string dacUpdateRate = "DAC_UpdateRate_Chan" + std::to_string(channel);
	string dacDMAPlaybackEn = "DAC_DMA_Playback_En" + std::to_string(channel);
	string dacSyncHSync = "DAC_Sync_hsync" + std::to_string(channel);
	string dacFilterEnable = "DAC_Filter_En" + std::to_string(channel);
	string dacFilterInhibit = "DACWave_Filter_Inhibit" + std::to_string(channel);
	string dacEOFFreezeEnable = "DACWavegenEOFFreezeEnable_chan" + std::to_string(channel);

	//don't change the offset if channel is not being used
	if (dacCtrl.enablePort)
	{
		status = FPGAregisterWRITE(dacOffset.c_str(), offset);
	}
	status = FPGAregisterWRITE(dacUpdateRate.c_str(), updateRate);
	status = FPGAregisterWRITE(dacDMAPlaybackEn.c_str(), dacCtrl.enablePort);
	status = FPGAregisterWRITE(dacSyncHSync.c_str(), dacCtrl.hSync);
	status = FPGAregisterWRITE(dacFilterEnable.c_str(), dacCtrl.enableFilter);
	status = FPGAregisterWRITE(dacFilterInhibit.c_str(), dacCtrl.filterInhibit);
	status = FPGAregisterWRITE(dacEOFFreezeEnable.c_str(), dacCtrl.enableEOFFreeze);

	switch (channel)
	{
	case 12:
		status = FPGAregisterWRITE("DAC_Digital_Out_Park0", static_cast<USHORT>(dacCtrl.park_val));
		status = FPGAregisterWRITE("DACWave_Filter_Inhibit12", 1); //set the inhibit filter bit high for the Digital lines
		break;
	case 13:
		status = FPGAregisterWRITE("DAC_Digital_Out_Park1", static_cast<USHORT>(dacCtrl.park_val));
		status = FPGAregisterWRITE("DACWave_Filter_Inhibit13", 1); //set the inhibit filter bit high for the Digital lines
		break;
	}

	return TRUE;
}

LONG CThordaq::SetFIRFilterSettings3P(double& sample_rate, double pixel_frequency, IMAGING_CONFIGURATION_STRUCT	imaging_config)
{
	for (int p = 0; p < FIR_FILTER_COUNT; ++p)
	{
		for (int c = 0; c < MAX_CHANNEL_COUNT; ++c)
		{
			for (int f = 0; f < FIR_FILTER_TAP_COUNT; ++f)
			{
				gPtrAcqCtrl->streamProcessing.fir_coefficient[p][c][f] = static_cast<USHORT>(round(8192 * imaging_config.streamingCtrl.fir_filter[p][c][f]));
			}
		}
	}
	return TRUE;
}

LONG CThordaq::SetFIRFilterSettings(FIRFilterMode filter_mode, double& sampleRate, double pixel_frequency, IMAGING_CONFIGURATION_STRUCT	imaging_config)
{
	//This number is always a power of 2 because of the way sample rate was reduced
	UINT16 downSampleRate = static_cast<UINT16>(ADC_MAX_SAMPLE_RATE / sampleRate);

	//The first FIR is only 16 taps long, can't set a longer first filter
	if (downSampleRate > FIR_FILTER_TAP_COUNT)
	{
		downSampleRate = FIR_FILTER_TAP_COUNT;
	}

	int index_before = (FIR_FILTER_TAP_COUNT / 2) - (downSampleRate / 2);
	int index_after = index_before + downSampleRate;


	//set the first FIR based on the downsample rate
	if (filter_mode == FIRFilterMode::INTERNAL_CLOCK_MODE) {
		for (int c = 0; c < MAX_CHANNEL_COUNT; ++c)
		{
			for (int f = 0; f < FIR_FILTER_TAP_COUNT; ++f)
			{
				if (f >= index_before && f < index_after)
				{
					gPtrAcqCtrl->streamProcessing.fir_coefficient[FIRST_FIR_FILTER][c][f] = 8192 / downSampleRate;
				}
				else
				{
					gPtrAcqCtrl->streamProcessing.fir_coefficient[FIRST_FIR_FILTER][c][f] = 0;
				}
			}
		}
	}
	else if (filter_mode == FIRFilterMode::EXTERNAL_CLOCK_MODE) {
		//if we're in external clock mode, the first FIR should only set every other FIR tap to the length of the downsampler
		//this is done using the modulus in the if statment below and doubling the FIR value
		for (int c = 0; c < MAX_CHANNEL_COUNT; ++c)
		{
			for (int f = 0; f < FIR_FILTER_TAP_COUNT; ++f)
			{
				if (f >= index_before && f < index_after && (f % 2) == 0)  //only the even ones
				{
					gPtrAcqCtrl->streamProcessing.fir_coefficient[FIRST_FIR_FILTER][c][f] = 2 * 8192 / downSampleRate;
				}
				else
				{
					gPtrAcqCtrl->streamProcessing.fir_coefficient[FIRST_FIR_FILTER][c][f] = 0;
				}
			}
		}
	}

	//The second FIR adjusts box car from the sample rate coming out of the downsampler to match the pixel_frequency
	UINT16 boxcarFIR2 = static_cast<UINT16>(ceil(sampleRate / pixel_frequency));
	//The second FIR is only 16 taps long
	if (boxcarFIR2 > FIR_FILTER_TAP_COUNT)
	{
		boxcarFIR2 = FIR_FILTER_TAP_COUNT;
	}

	index_before = (FIR_FILTER_TAP_COUNT / 2) - (boxcarFIR2 / 2);
	index_after = index_before + boxcarFIR2;
	for (int c = 0; c < MAX_CHANNEL_COUNT; ++c)
	{
		for (int f = 0; f < FIR_FILTER_TAP_COUNT; ++f)
		{
			if (f >= index_before && f < index_after)
			{
				gPtrAcqCtrl->streamProcessing.fir_coefficient[SECOND_FIR_FILTER][c][f] = 8192 / boxcarFIR2;
			}
			else
			{
				gPtrAcqCtrl->streamProcessing.fir_coefficient[SECOND_FIR_FILTER][c][f] = 0;
			}
		}
	}
	return TRUE;
}


THORDAQ_STATUS CThordaq::SetThreePhotonSampleOffset(UINT channel, UINT8 threePhotonSampleOffset)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	switch (channel)
	{
	case 0:	status = FPGAregisterWRITE("ADCStream_3PSampleOffset0", threePhotonSampleOffset); break;
	case 1:	status = FPGAregisterWRITE("ADCStream_3PSampleOffset1", threePhotonSampleOffset); break;
	case 2:	status = FPGAregisterWRITE("ADCStream_3PSampleOffset2", threePhotonSampleOffset); break;
	case 3:	status = FPGAregisterWRITE("ADCStream_3PSampleOffset3", threePhotonSampleOffset); break;
	}

	return status;
}

THORDAQ_STATUS CThordaq::SetAllADCChannelsGain(ULONG clock_source, ULONG32 adcGain[], bool forceGainUpdate)
{
	INT32 error = 0;
	for (UINT8 i = 0; i < MAX_CHANNEL_COUNT; i += 2)
	{
		if (forceGainUpdate && clock_source == CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE)
		{
			UCHAR gain = static_cast<UCHAR>(adcGain[i] | (adcGain[i + 1]) << 4);
			ThordaqErrChk(L"SetADCGain", SetADCGain(i, gain));
		}
		else if (forceGainUpdate)
		{
			ThordaqErrChk(L"SetADCGain", SetADCGain(i, ADC_GAIN::ADC_OFF));
		}
	}
	return 	STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CThordaq::SetClockSourceAndFrequency(ULONG clock_source, UINT8 set_flag)
{

	STAT_STRUCT StatusInfo;
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;

	if (clock_source == CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE)
	{
		// external laser ref
		ZeroMemory(&(gPtrAcqCtrl->adcInterface.before), sizeof(USHORT));
		gPtrAcqCtrl->adcInterface.before.jesdCore1 = 0x1;
		gPtrAcqCtrl->adcInterface.before.jesdCore2 = 0x1;
		gPtrAcqCtrl->adcInterface.before.jesdSysRefSync = 0x1;
		gPtrAcqCtrl->adcInterface.before.jesdTestEn = 0x0;
		gPtrAcqCtrl->adcInterface.before.gpio0IntRefSel = 0x0;
		gPtrAcqCtrl->adcInterface.before.gpio1IntRefEn = 0x0;
		gPtrAcqCtrl->adcInterface.before.gpio2LFpgaRefEn = 0x0;
		gPtrAcqCtrl->adcInterface.before.gpio3Filt = 0x0;
		gPtrAcqCtrl->adcInterface.before.gpio4Filt = 0x1;

		ZeroMemory(&(gPtrAcqCtrl->adcInterface.after), sizeof(USHORT));
		gPtrAcqCtrl->adcInterface.after.jesdCore1 = 0x0;
		gPtrAcqCtrl->adcInterface.after.jesdCore2 = 0x0;
		gPtrAcqCtrl->adcInterface.after.jesdSysRefSync = 0x1;
		gPtrAcqCtrl->adcInterface.after.jesdTestEn = 0x0;
		gPtrAcqCtrl->adcInterface.after.gpio0IntRefSel = 0x0;
		gPtrAcqCtrl->adcInterface.after.gpio1IntRefEn = 0x0;
		gPtrAcqCtrl->adcInterface.after.gpio2LFpgaRefEn = 0x0;
		gPtrAcqCtrl->adcInterface.after.gpio3Filt = 0x0;
		gPtrAcqCtrl->adcInterface.after.gpio4Filt = 0x1;

	}
	else
	{
		//internal clock source 80MHz
		ZeroMemory(&(gPtrAcqCtrl->adcInterface.before), sizeof(USHORT));
		gPtrAcqCtrl->adcInterface.before.jesdCore1 = 0x1;
		gPtrAcqCtrl->adcInterface.before.jesdCore2 = 0x1;
		gPtrAcqCtrl->adcInterface.before.jesdSysRefSync = 0x0;
		gPtrAcqCtrl->adcInterface.before.jesdTestEn = 0x0;
		gPtrAcqCtrl->adcInterface.before.gpio0IntRefSel = 0x1;
		gPtrAcqCtrl->adcInterface.before.gpio1IntRefEn = 0x0;
		gPtrAcqCtrl->adcInterface.before.gpio2LFpgaRefEn = 0x1;
		gPtrAcqCtrl->adcInterface.before.gpio3Filt = 0x0;
		gPtrAcqCtrl->adcInterface.before.gpio4Filt = 0x1;

		ZeroMemory(&(gPtrAcqCtrl->adcInterface.after), sizeof(USHORT));
		gPtrAcqCtrl->adcInterface.after.jesdCore1 = 0x0;
		gPtrAcqCtrl->adcInterface.after.jesdCore2 = 0x0;
		gPtrAcqCtrl->adcInterface.after.jesdSysRefSync = 0x0;
		gPtrAcqCtrl->adcInterface.after.jesdTestEn = 0x0;
		gPtrAcqCtrl->adcInterface.after.gpio0IntRefSel = 0x1;
		gPtrAcqCtrl->adcInterface.after.gpio1IntRefEn = 0x0;
		gPtrAcqCtrl->adcInterface.after.gpio2LFpgaRefEn = 0x1;
		gPtrAcqCtrl->adcInterface.after.gpio3Filt = 0x0;
		gPtrAcqCtrl->adcInterface.after.gpio4Filt = 0x1;
	}

	if (set_flag)
	{
		BYTE* buffer = GetNewBufferOfSize(sizeof(USHORT)); //0x00;

		//first set Jesdcore1 and 2 to zero and everything else to what their new values
		memcpy(buffer, &gPtrAcqCtrl->adcInterface.after, sizeof(USHORT));
		ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(WRITE_TO_CARD, 3, 0x200, buffer, 0, 2, &StatusInfo))
			if (status != STATUS_SUCCESSFUL)
			{
				SAFE_DELETE_ARRAY(buffer);
				return STATUS_READWRITE_REGISTER_ERROR;
			}

		//sleep to allow the changes to happen
		Sleep(25);

		memcpy(buffer, &gPtrAcqCtrl->adcInterface.before, sizeof(USHORT));
		ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(WRITE_TO_CARD, 3, 0x200, buffer, 0, 2, &StatusInfo))
			if (status != STATUS_SUCCESSFUL)
			{
				SAFE_DELETE_ARRAY(buffer);
				return STATUS_READWRITE_REGISTER_ERROR;
			}

		//sleep to allow the changes to happen
		Sleep(25);

		memcpy(buffer, &gPtrAcqCtrl->adcInterface.after, sizeof(USHORT));
		ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(WRITE_TO_CARD, 3, 0x200, buffer, 0, 2, &StatusInfo))
			if (status != STATUS_SUCCESSFUL)
			{
				SAFE_DELETE_ARRAY(buffer);
				return STATUS_READWRITE_REGISTER_ERROR;
			}
		SAFE_DELETE_ARRAY(buffer);
		Sleep(10);
	}

	return 	STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CThordaq::SetADCInterfaceSettings(IMAGING_CONFIGURATION_STRUCT imaging_config)
{
	//add 1% to the clock rate to have some margin when calculating the RDivideFactor
	double clockRate = imaging_config.imageCtrl.clockRate * 1.01 / 1000000;
	THORDAQ_STATUS status;
	BYTE rdivideFactor = 0;

	if (clockRate > RDivideLaserFreqLimit)
	{
		if ((clockRate / floor(clockRate)) <= RDivideLaserFreqLimit)
		{
			rdivideFactor = (BYTE)((BYTE)floor(clockRate) - 1);
		}
		else
		{
			rdivideFactor = (BYTE)(floor(clockRate));
		}
	}

	status = FPGAregisterWRITE("ADCInterface3PMarkersRDivide", rdivideFactor); // 0x230

	if (status != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	return 	STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CThordaq::SetDCOffsetPreFIR(short preDcOffset, USHORT channel)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	if (channel >= MAX_CHANNEL_COUNT)
	{
		return STATUS_PARAMETER_SETTINGS_ERROR;
	}
	//	ULONGLONG valueSize = sizeof(short);
	//	ULONGLONG dc_offset_pre_fir_addr = 0x1c0 + 0x18;
	//	ULONGLONG dc_offset_channel_addr = dc_offset_pre_fir_addr + (channel * sizeof(short));

		// replace legacy FPGA write with new shadow register API
	switch (channel)
	{
	case 0:
		status = FPGAregisterWRITE("ADCStreamDCoffsetChan0", preDcOffset); // 0x1D8
		break;
	case 1:
		status = FPGAregisterWRITE("ADCStreamDCoffsetChan1", preDcOffset); // 0x1DA
		break;
	case 2:
		status = FPGAregisterWRITE("ADCStreamDCoffsetChan2", preDcOffset); // 0x1DC
		break;
	case 3:
		status = FPGAregisterWRITE("ADCStreamDCoffsetChan3", preDcOffset); // 0x1DE
		break;

	}

	//	STAT_STRUCT StatusInfo;
	//	BYTE* buffer = GetNewBufferOfSize(valueSize);
	//	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	//	INT32 error = 0;
	//	memcpy(buffer, &preDcOffset, sizeof(short));
	//	ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(WRITE_TO_CARD, 3, dc_offset_channel_addr, buffer, 0, valueSize, &StatusInfo));
	//	if (status != STATUS_SUCCESSFUL)
	//	{
	//		SAFE_DELETE_ARRAY(buffer);
	//		return status;
	//	}
	//	SAFE_DELETE_ARRAY(buffer);

	return status;
}

THORDAQ_STATUS CThordaq::SetGRClockRate(ULONG32 adc_sample_rate, ULONG32 expectedFrequency)
{
	STAT_STRUCT StatusInfo;
	BYTE* buffer = GetNewBufferOfSize(sizeof(UCHAR));
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;

	UCHAR    pll_fltr_ctrl = 0x42;// only G/R
	ULONG32  pll_cntr_freq = (ULONG32)ceil(((double)MAXULONG32 / (adc_sample_rate)*expectedFrequency) - 0.5); //0x000346DC; only G/R 			

	memcpy(buffer, &pll_fltr_ctrl, sizeof(UCHAR));
	ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(WRITE_TO_CARD, 3, 0x148, buffer, 0, 1, &StatusInfo))
		if (status != STATUS_SUCCESSFUL)
		{
			SAFE_DELETE_ARRAY(buffer);
			return STATUS_READWRITE_REGISTER_ERROR;
		}

	memcpy(buffer, &pll_cntr_freq, sizeof(ULONG32));
	ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(WRITE_TO_CARD, 3, 0x158, buffer, 0, 4, &StatusInfo))
		if (status != STATUS_SUCCESSFUL)
		{
			SAFE_DELETE_ARRAY(buffer);
			return STATUS_READWRITE_REGISTER_ERROR;
		}
	Sleep(5);
	SAFE_DELETE_ARRAY(buffer);

	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CThordaq::GetLineTriggerFrequency(UINT32 sample_rate, double& frequency, UINT32 expectedFrequency)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;
	BYTE* buffer = GetNewBufferOfSize(sizeof(USHORT));
	USHORT val = 0x0000;
	memcpy(buffer, &val, sizeof(USHORT));
	STAT_STRUCT StatusInfo;
	ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(READ_FROM_CARD, 3, 0x140, buffer, 0, 2, &StatusInfo));
	memcpy(&val, buffer, sizeof(USHORT));
	if (status == STATUS_SUCCESSFUL)
	{
		if (val != 0)
		{
			frequency = static_cast<double>(sample_rate) / static_cast<double>(val);
		}
		else
		{
			frequency = expectedFrequency;
		}
	}
	else
	{
		status = STATUS_READWRITE_REGISTER_ERROR;;
	}
	SAFE_DELETE_ARRAY(buffer);
	return 	status;
}


THORDAQ_STATUS CThordaq::GetTotalFrameCount(UINT32& frame_count)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;

	BYTE* buffer = GetNewBufferOfSize(sizeof(ULONG32));
	memcpy(buffer, &frame_count, sizeof(ULONG32));
	STAT_STRUCT StatusInfo;
	ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(READ_FROM_CARD, 3, 0x142, buffer, 0, sizeof(ULONG32), &StatusInfo));
	memcpy(&frame_count, buffer, sizeof(ULONG32));
	if (status != STATUS_SUCCESSFUL)
	{
		status = STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return status;
}

//TODO 
THORDAQ_STATUS CThordaq::SetThreePhotonModeEnable(bool enableThreePhoton)
{
	//return FPGAregisterWRITE("SC_3P_EN", enableThreePhoton);
	BYTE* buffer_write = GetNewBufferOfSize(sizeof(USHORT));  //0x00;
	STAT_STRUCT StatusInfo;
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;

	if (enableThreePhoton)
	{
		USHORT enable_threePhoton_cmd = 0x0008; //50ms count
		USHORT select_threePhoton_cmd = 0x000B;

		memcpy(buffer_write, &enable_threePhoton_cmd, sizeof(USHORT));
		ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(WRITE_TO_CARD, 3, 0x180, buffer_write, 0, 2, &StatusInfo))
			if (status != STATUS_SUCCESSFUL)
			{
				SAFE_DELETE_ARRAY(buffer_write);
				return STATUS_READWRITE_REGISTER_ERROR;
			}
	}
	else
	{
		USHORT select_non3p_cmd = 0x0003;
		memcpy(buffer_write, &select_non3p_cmd, sizeof(USHORT));
		ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(WRITE_TO_CARD, 3, 0x180, buffer_write, 0, 2, &StatusInfo))
			if (status != STATUS_SUCCESSFUL)
			{
				SAFE_DELETE_ARRAY(buffer_write);
				return STATUS_READWRITE_REGISTER_ERROR;
			}
		//sleep to allow the changes to happen
	}

	SAFE_DELETE_ARRAY(buffer_write);

	return STATUS_SUCCESSFUL;
}

//TODO: see how we can update the speed at which we are doing detecting the freq
//TODO: this function enables three photon. This should not happen here, there should be a way to disable 3 photon as well.
THORDAQ_STATUS CThordaq::MeasureExternClockRate(ULONG32& clock_rate, ULONG32& clock_ref, ULONG32 mode)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;
	STAT_STRUCT StatusInfo;
	BYTE* buffer_write = GetNewBufferOfSize(sizeof(ULONG32));  //0x00;
	BYTE* buffer_read = GetNewBufferOfSize(sizeof(ULONG32));

	if (mode == 1) // threePhoton Mode
	{
		//TODO make sure this call is needed. It might be needed to check the rate
		USHORT enable_threePhoton_cmd = 0x0009; //50ms count
		memcpy(buffer_write, &enable_threePhoton_cmd, sizeof(USHORT));
		ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(WRITE_TO_CARD, 3, 0x180, buffer_write, 0, 2, &StatusInfo))
			if (status != STATUS_SUCCESSFUL)
			{
				SAFE_DELETE_ARRAY(buffer_write);
				SAFE_DELETE_ARRAY(buffer_read);
				return STATUS_READWRITE_REGISTER_ERROR;
			}
	}

	ULONG32 set_time_period = (ULONG32)ceil((100000.0 / 0.005) - 0.5);//100ms 
	memcpy(buffer_write, &set_time_period, sizeof(ULONG32));
	ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(WRITE_TO_CARD, 3, 0x1A0, buffer_write, 0, 4, &StatusInfo))
		if (status != STATUS_SUCCESSFUL)
		{
			SAFE_DELETE_ARRAY(buffer_write);
			SAFE_DELETE_ARRAY(buffer_read);
			return STATUS_READWRITE_REGISTER_ERROR;
		}

	Sleep(200); // Should be double of set_time_period

	if (mode == 0)
	{
		USHORT select_non3p_cmd = 0x0003;
		memcpy(buffer_write, &select_non3p_cmd, sizeof(USHORT));
		ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(WRITE_TO_CARD, 3, 0x180, buffer_write, 0, 2, &StatusInfo))
			if (status != STATUS_SUCCESSFUL)
			{
				SAFE_DELETE_ARRAY(buffer_write);
				SAFE_DELETE_ARRAY(buffer_read);
				return STATUS_READWRITE_REGISTER_ERROR;
			}
		//sleep to allow the changes to happen
		Sleep(5);
	}

	ULONG32 count = 0;
	memcpy(buffer_read, &count, sizeof(ULONG32));
	ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(READ_FROM_CARD, 3, 0x181, buffer_read, 0, 4, &StatusInfo))
		if (status != STATUS_SUCCESSFUL)
		{
			SAFE_DELETE_ARRAY(buffer_write);
			SAFE_DELETE_ARRAY(buffer_read);
			return STATUS_READWRITE_REGISTER_ERROR;
		}
	memcpy(&count, buffer_read, sizeof(ULONG32));

	clock_rate = (ULONG32)((double)count / 0.1);

	if (mode == 1) // threePhoton Mode
	{
		//TODO: make sure this call is needed. It might be needed to check the rate.
		USHORT select_threePhoton_cmd = 0x000B;
		memcpy(buffer_write, &select_threePhoton_cmd, sizeof(USHORT));
		ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(WRITE_TO_CARD, 3, 0x180, buffer_write, 0, 2, &StatusInfo))
			if (status != STATUS_SUCCESSFUL)
			{
				SAFE_DELETE_ARRAY(buffer_write);
				SAFE_DELETE_ARRAY(buffer_read);
				return STATUS_READWRITE_REGISTER_ERROR;
			}
		//sleep to allow the changes to happen
		Sleep(200);

		ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(READ_FROM_CARD, 3, 0x181, buffer_read, 0, 4, &StatusInfo))
			if (status != STATUS_SUCCESSFUL)
			{
				SAFE_DELETE_ARRAY(buffer_write);
				SAFE_DELETE_ARRAY(buffer_read);
				return STATUS_READWRITE_REGISTER_ERROR;
			}
		memcpy(&count, buffer_read, sizeof(ULONG32));
		clock_ref = (ULONG32)((double)count / 0.1);

		if (clock_ref == 0 || clock_rate == 0)
		{
			SAFE_DELETE_ARRAY(buffer_write);
			SAFE_DELETE_ARRAY(buffer_read);
			return STATUS_READWRITE_REGISTER_ERROR;
		}
	}

	SAFE_DELETE_ARRAY(buffer_write);
	SAFE_DELETE_ARRAY(buffer_read);
	if (clock_rate == 0)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}
	return status;
}

THORDAQ_STATUS GetDACSamplesPerLine(UINT32& samples, double& dac_rate, double line_time)
{
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CThordaq::GetExternClockStatus(ULONG32& isClockedSynced)
{
	STAT_STRUCT StatusInfo;
	BYTE* buffer_read = GetNewBufferOfSize(sizeof(BYTE));
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;
	ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(READ_FROM_CARD, 3, 0x180, buffer_read, 0, 1, &StatusInfo))
		if (status != STATUS_SUCCESSFUL)
		{
			SAFE_DELETE_ARRAY(buffer_read);
			return STATUS_READWRITE_REGISTER_ERROR;
		}

	isClockedSynced = (*buffer_read & 0x10) ? 1 : 0;
	SAFE_DELETE_ARRAY(buffer_read);
	return STATUS_SUCCESSFUL;
}


THORDAQ_STATUS CThordaq::SetADCGain(UINT8 channel, UCHAR gain)
{
	//sanity check
	if (channel < 0 || channel >= MAX_CHANNEL_COUNT || gain < ADC_GAIN::ADC_OFF || gain >(ADC_GAIN::db29 | (ADC_GAIN::db29 << 4)))
	{
		return  STATUS_READWRITE_REGISTER_ERROR;
	}
	STAT_STRUCT StatusInfo;
	BYTE* buffer_write = (BYTE*)(&gain); // retrieve the pointer
	ULONGLONG register_offset = 0x208;
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;
	if (channel < 2) // 1st 2nd channel
	{
		register_offset = 0x208;
	}
	else if (channel < 4) // 3rd 4th channel
	{
		register_offset = 0x210;
	}
	else //5th 6th channel
	{
		register_offset = 0x218;
	}
	ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(WRITE_TO_CARD, 3, register_offset, buffer_write, 0, 1, &StatusInfo))
		if (status != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
	gPtrAcqCtrl->adcInterface.gain[channel] = (UCHAR)gain;
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CThordaq::SlowSmoothMoveToAndFromParkEnable(bool enable)
{
	THORDAQ_STATUS status = FPGAregisterWRITE("DAC_SLOW_RATE", enable);

	return status;
}


THORDAQ_STATUS CThordaq::SetDDSClockEnable(bool enable)
{
	THORDAQ_STATUS status = FPGAregisterWRITE("DDS_CLK_3P_EN", enable);

	return status;
}


THORDAQ_STATUS CThordaq::SetDDSClockPhase(int channel, double phase)
{
	BYTE phaseOffset = static_cast<BYTE>(round(256.0 * phase / 360.0)) - 1;

	THORDAQ_STATUS status = STATUS_INCOMPLETE;

	switch (channel)
	{
	case 0: status = FPGAregisterWRITE("DDS_3P_RVB_Phase0", phaseOffset); break;
	case 1: status = FPGAregisterWRITE("DDS_3P_RVB_Phase1", phaseOffset); break;
	}

	return status;
}


// Replace Carl's whole enchilada "ImageAcquisitionConfig()" kernel IOCTL call,
// replacing his FPGA register writes with new API shadow register writes
// NOTE!  The NWL kernel driver makes some FPGA writes to the DMA core control, which is
// inherited by the ThorDAQ DLL design.  Intention is to leave these unchanged, and move
// all other FPGA register read/write in the C++ ThorDAQ DLL
// This function follows IOCTL kernel function "NTSTATUS ImageAcquisitionConfig()"
// NOTE!  Indentically named STRUCT types have different names of fields
// between user and kernel STRUCT definitions, e.g. 
// GLOBAL_IMG_GEN_CTRL_STRUCT   User              Kernel
//                              img_scan_mode     imgSyncCtrl
// SCAN_SUBSYS_STRUCT           sync_ctrl         syncCtrl
//                              frm_cnt           frameCnt
//                              pll_sync_offset   syncOffset  
//
THORDAQ_STATUS CThordaq::APIimageAcqConfig(DATA_ACQ_CTRL_STRUCT* PtrAcqCtrl)  // PtrAcqCtrl is "the whole enchilada"
{
	//TODO: Add missing registers
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	// stop all FPGA DMA (was done in NWL API/kernel)
	status = GlobalSCANstart(false);  // i.e. includes "GIGCR0_STOP_RUN" bit
	// clear S2mm Host Interrupts
	status = FPGAregisterWRITE("S2MMDMA_ControlReg1", 0x50);
	status = FPGAregisterWRITE("S2MMDMA_ControlReg2", 0x50);
	status = FPGAregisterWRITE("S2MMDMA_ControlReg3", 0x50);
	status = FPGAregisterWRITE("S2MMDMA_ControlReg4", 0x50);

	// Scanning Control
	status = FPGAregisterWRITE("GlobalImageSyncControlReg", PtrAcqCtrl->gblCtrl.img_scan_mode); // BAR3 0x008
	status = FPGAregisterWRITE("ScanningSyncControlReg", PtrAcqCtrl->scan.sync_ctrl);  // 0x140
	status = FPGAregisterWRITE("ScanningFrameCount", PtrAcqCtrl->scan.frm_cnt);            // 0x142
	status = FPGAregisterWRITE("ADPLL_SyncDelay", PtrAcqCtrl->scan.pll_sync_offset);          // 0x150

	status = FPGAregisterWRITE("ScanningIntraFrameDelay", PtrAcqCtrl->scan.galvo_intra_frame_delay);  // 0x178
	status = FPGAregisterWRITE("ScanningPreSOFDelay", PtrAcqCtrl->scan.galvo_pre_SOF_delay);	// 0x17B 
	status = FPGAregisterWRITE("ScanningGalvoPixelDelay", PtrAcqCtrl->scan.galvo_pixel_delay);        // 0x168
	status = FPGAregisterWRITE("ScanningGalvoPixelDwell", PtrAcqCtrl->scan.galvo_pixel_dwell);        // 0x160
	status = FPGAregisterWRITE("ScanningIntraLineDelay", PtrAcqCtrl->scan.galvo_intra_line_delay);     // 0x170

	// ADC Stream Processing
	// copy Carl's FIR logic from IOCTL
	int c, p, f;
	for (p = 0; p < 2; p++)
	{
		for (c = 0; c < 4; c++)
		{
			UINT64 regValue = (PtrAcqCtrl->streamProcessing.stream_ctrl & 0x07) | ((p * 4 + c) << 3);
			status = FPGAregisterWRITE("ADCStreamControlReg", regValue); // 0x1C0
			for (f = 0; f < 16; f++)
			{
				status = FPGAregisterWRITE("ADCStreamFIRcoeffReg", PtrAcqCtrl->streamProcessing.fir_coefficient[p][c][f]); // 0x1E8
			}
		}
	}
	status = FPGAregisterWRITE("ADCStreamControl2Reg", PtrAcqCtrl->streamProcessing.stream_ctrl2); // 0x1C1

	status = FPGAregisterWRITE("ADCStream_MAFilterLength", PtrAcqCtrl->streamProcessing.movingAverageFilterLength); // 0x1C2
	status = FPGAregisterWRITE("ADCStream_MA_Filter_Sel", PtrAcqCtrl->streamProcessing.enableMovingAverageFilter); // 0x1C3

	status = FPGAregisterWRITE("ADCStreamPulseInterleaveOffsetReg", PtrAcqCtrl->streamProcessing.pulse_interleave_offset);  // 0x1F0
	status = FPGAregisterWRITE("ADCStreamScanningPeriodReg", PtrAcqCtrl->streamProcessing.scan_period);  // 0x1C8
	status = FPGAregisterWRITE("ADCStreamDownsampleReg", PtrAcqCtrl->streamProcessing.downsample_rate);  // 0x1D0
	status = FPGAregisterWRITE("ADCStreamDownsampleReg2", PtrAcqCtrl->streamProcessing.downsample_rate2);  // 0x1D0

	status = FPGAregisterWRITE("ADCStream_3PSampleOffset0", PtrAcqCtrl->streamProcessing.threePhoton_sample_offset0); // 0x1F8
	status = FPGAregisterWRITE("ADCStream_3PSampleOffset1", PtrAcqCtrl->streamProcessing.threePhoton_sample_offset1); // 0x1F8
	status = FPGAregisterWRITE("ADCStream_3PSampleOffset2", PtrAcqCtrl->streamProcessing.threePhoton_sample_offset2); // 0x1F8
	status = FPGAregisterWRITE("ADCStream_3PSampleOffset3", PtrAcqCtrl->streamProcessing.threePhoton_sample_offset3); // 0x1F8

	status = FPGAregisterWRITE("ADCStream3PReverbMPCyclesReg", PtrAcqCtrl->streamProcessing.threePhoton_reverb_MP_cycles); // 0x1F9
	status = FPGAregisterWRITE("ADCStreamReverbDownsampleReg", PtrAcqCtrl->streamProcessing.threePhoton_reverb_downsample_rate); // 0x1F9

	// Sampling Clock
	status = FPGAregisterWRITE("SamplingClockControlReg", PtrAcqCtrl->samplingClock.controlRegister0); // 0x180
	status = FPGAregisterWRITE("SamplingClockPhaseOffset", PtrAcqCtrl->samplingClock.phase_offset); // 0x188
	status = FPGAregisterWRITE("SamplingClockPhaseStep", PtrAcqCtrl->samplingClock.phase_step); // 0x190
	status = FPGAregisterWRITE("SamplingClockPhaseLimit", PtrAcqCtrl->samplingClock.phase_limit); // 0x198

	// Image config 
	status = FPGAregisterWRITE("GlobalImageHSIZE", PtrAcqCtrl->gblCtrl.hor_pix_num - 1); // 0x010

	USHORT acqHSize = PtrAcqCtrl->gblCtrl.hor_pix_num * static_cast<UINT8>(PtrAcqCtrl->gblCtrl.numPlanes) - 1;
	status = FPGAregisterWRITE("GlobalImageAcqHSIZE", acqHSize); // 0x012
	status = FPGAregisterWRITE("GlobalImageVSIZE", PtrAcqCtrl->gblCtrl.vrt_pix_num - 1); // 0x018

	UINT64 currentVal = 0x0;

	//get the shadow register so we don't replace the current values when we are rearming the DAC IRQ
	status = FPGAregisterRead(_DACWaveGen3PSyncControlRegIndex, -1, &currentVal);
	_rearmDAC1 = 0x20000000 | currentVal; //setting bit 29 to 1
	_rearmDAC0 = (~(0x20000000)) & currentVal; //setting bit 29 to 0 
	// Image S2MM DMA configuration? ---  In NWL model there is no DMA setup IOCTL matching legacy (Carl's) custom method, 
	// no "DataBufferStartAddress" in pDevExt
	// Enable channels...
	S2MM_CONFIG s2mmConfig;

	s2mmConfig.ChannelMask = 0x0f; // all four channels
	s2mmConfig.DDR3imageStartingAddress = 0x0; // ThorDAQ's DDR3 mem baseAddress as seen by NWL DMA
	s2mmConfig.HSize = PtrAcqCtrl->gblCtrl.hor_pix_num * PtrAcqCtrl->gblCtrl.numPlanes;
	s2mmConfig.VSize = PtrAcqCtrl->gblCtrl.vrt_pix_num;
	s2mmConfig.NumberOfDescriptorsPerBank = PtrAcqCtrl->gblCtrl.frm_per_txn;

	status = S2MMconfig(&s2mmConfig);  // configure the S2MM image DMA "Stream" to "Mapped [DDR3] Memory"

	//get the shadow register so we don't replace the current values when we are rearming the imaging IRQ
	for (int i = 0; i < MAX_CHANNEL_COUNT; ++i)
	{
		status = FPGAregisterRead(_S2MMDMAControlRegIndex[i], -1, &currentVal);
		_reArmS2MMDMACommand[i] = currentVal | 0x40;
	}

	return status;
}

THORDAQ_STATUS CThordaq::ImagingTrackLevelTriggerAndRearmS2MM()
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	while (!_abortReadImage && true == _imagingStartStopStatus && true == _imagingLevelTriggerActive)
	{
		if (_imagingLevelTriggerWentLow)
		{
			_imagingLevelTriggerWentLow = false;

			// Image S2MM DMA configuration? ---  In NWL model there is no DMA setup IOCTL matching legacy (Carl's) custom method, 
			// no "DataBufferStartAddress" in pDevExt
			// Enable channels...
			S2MM_CONFIG s2mmConfig;

			s2mmConfig.ChannelMask = 0x0f; // all four channels
			s2mmConfig.DDR3imageStartingAddress = 0x0; // ThorDAQ's DDR3 mem baseAddress as seen by NWL DMA
			s2mmConfig.HSize = gPtrAcqCtrl->gblCtrl.hor_pix_num * gPtrAcqCtrl->gblCtrl.numPlanes;
			s2mmConfig.VSize = gPtrAcqCtrl->gblCtrl.vrt_pix_num;
			s2mmConfig.NumberOfDescriptorsPerBank = gPtrAcqCtrl->gblCtrl.frm_per_txn;

			status = S2MMconfig(&s2mmConfig);  // configure the S2MM image DMA "Stream" to "Mapped [DDR3] Memory"
		}
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	};

	if (_abortReadImage)
	{
		status = STATUS_ACQUISITION_ABORTED;
	}

	return status;
}

/*THORDAQ_STATUS CThordaq::SetDownsampleRate(bool enableDownsampleChange, USHORT downSampleRate)
{
	_enableDownsamplingRateChange = enableDownsampleChange;
	_threePhotonDownsamplingRate = downSampleRate;
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CThorDAQ::GetDownSampleRate(USHORT downSampleRate)
{

}*/