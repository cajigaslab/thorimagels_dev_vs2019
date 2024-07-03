/*++

Copyright (c) Thorlabs, Inc.  All rights reserved.

Module Name: thordaqDAC.cpp


Abstract:

	Defines the API functions for the thordaq driver, DAC related only on this file.

Environment:

	kernel mode only.

Style:
	Google C++ coding style.
--*/

#include "stdafx.h"
#pragma warning(disable:4201) //I just wanna use freedom nonstandard "nameless struct/union"
#include "thordaq.h"
#include "thordaqguid.h"


UINT64 CalculateMinSamples(double dacRate, double minSeconds)
{
	//TODO: define some ranges for the DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD. It appears the value needs to be longer for faster update rates, but can be much shorter for slower update rates.
	UINT64 minSamples = static_cast<UINT64>(ceil(dacRate * minSeconds));
	//thordaq requires that the number of samples is always even
	if (minSamples % DAC_DESC_SAMPLE_COUNT_RESOLUTION != 0)
	{
		++minSamples;
	}
	return minSamples;
}

UINT64 CalculateMinSamplesStart(double dacRate)
{
	double minSeconds = 0;

	if (DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD0 <= dacRate)
	{
		minSeconds = DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD0;
	}
	else if (DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD1 < dacRate)
	{
		minSeconds = DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD1;
	}
	else if (DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD2 < dacRate)
	{
		minSeconds = DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD2;
	}
	else if (DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD3 < dacRate)
	{
		minSeconds = DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD3;
	}
	else if (DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD4 < dacRate)
	{
		minSeconds = DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD4;
	}
	else if (DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD5 < dacRate)
	{
		minSeconds = DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD5;
	}
	else if (DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD6 < dacRate)
	{
		minSeconds = DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD6;
	}
	else if (DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD7 < dacRate)
	{
		minSeconds = DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD7;
	}
	else
	{
		minSeconds = DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD8;
	}

	UINT64 minSamples = static_cast<UINT64>(ceil(dacRate * minSeconds));
	//thordaq requires that the number of samples is always even
	if (minSamples % DAC_DESC_SAMPLE_COUNT_RESOLUTION != 0)
	{
		++minSamples;
	}
	return minSamples;
}

UINT64 CalculateMinSamplesEnd(double dacRate)
{
	double minSeconds = 0;
	if (DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD1 < dacRate)
	{
		minSeconds = DAC_MIN_SECONDS_LAST_DESC_DYNAMIC_LOAD1;
	}
	else if (DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD2 < dacRate)
	{
		minSeconds = DAC_MIN_SECONDS_LAST_DESC_DYNAMIC_LOAD2;
	}
	else if (DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD3 < dacRate)
	{
		minSeconds = DAC_MIN_SECONDS_LAST_DESC_DYNAMIC_LOAD3;
	}
	else if (DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD4 < dacRate)
	{
		minSeconds = DAC_MIN_SECONDS_LAST_DESC_DYNAMIC_LOAD4;
	}
	else if (DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD5 < dacRate)
	{
		minSeconds = DAC_MIN_SECONDS_LAST_DESC_DYNAMIC_LOAD5;
	}
	else if (DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD6 < dacRate)
	{
		minSeconds = DAC_MIN_SECONDS_LAST_DESC_DYNAMIC_LOAD6;
	}
	else if (DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD7 < dacRate)
	{
		minSeconds = DAC_MIN_SECONDS_LAST_DESC_DYNAMIC_LOAD7;
	}
	else
	{
		minSeconds = DAC_MIN_SECONDS_LAST_DESC_DYNAMIC_LOAD8;
	}

	UINT64 minSamples = static_cast<UINT64>(ceil(dacRate * minSeconds));
	//thordaq requires that the number of samples is always even
	if (minSamples % DAC_DESC_SAMPLE_COUNT_RESOLUTION != 0)
	{
		++minSamples;
	}
	return minSamples;
}

THORDAQ_STATUS CThordaq::SetDIOChannelSelection(vector<string> DIOSelection)
{

	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;
	for (int i = 0; i < DIOSelection.size(); ++i)
	{
		status = APISetDIOConfig(*this, (CHAR*)DIOSelection[i].c_str(), (UINT32)DIOSelection[i].size());
	}

	return status;
}

THORDAQ_STATUS CThordaq::SetScanActiveLineInvert(bool invertScanActiveLine)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	FPGAregisterWRITE("Capture_Active_Invert", invertScanActiveLine);
	_captureActiveLinveInvert = invertScanActiveLine;
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CThordaq::SetDACParkValue(ULONG32 outputChannel, double parkValue)
{
	//only analog channels
	if (outputChannel > 11 || parkValue < -10.0 || parkValue > 10.0)
	{
		return STATUS_PARAMETER_SETTINGS_ERROR;
	}

	//keep track of parked positions
	_dacParkingPositions[outputChannel] = parkValue;

	USHORT park_mid = parkValue > 0 ? 0x7fff : 0x8000;
	USHORT dacParkValue = static_cast<USHORT>(std::floor(parkValue / GALVO_RESOLUTION + 0.5) + park_mid);

	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	_dacParkingPositionsFPGA[outputChannel] = dacParkValue;

	switch (outputChannel)
	{
	case 0: status = FPGAregisterWRITE("DAC_Park_Chan0", dacParkValue); break;
	case 1: status = FPGAregisterWRITE("DAC_Park_Chan1", dacParkValue); break;
	case 2: status = FPGAregisterWRITE("DAC_Park_Chan2", dacParkValue); break;
	case 3: status = FPGAregisterWRITE("DAC_Park_Chan3", dacParkValue); break;
	case 4: status = FPGAregisterWRITE("DAC_Park_Chan4", dacParkValue); break;
	case 5: status = FPGAregisterWRITE("DAC_Park_Chan5", dacParkValue); break;
	case 6: status = FPGAregisterWRITE("DAC_Park_Chan6", dacParkValue); break;
	case 7: status = FPGAregisterWRITE("DAC_Park_Chan7", dacParkValue); break;
	case 8: status = FPGAregisterWRITE("DAC_Park_Chan8", dacParkValue); break;
	case 9: status = FPGAregisterWRITE("DAC_Park_Chan9", dacParkValue); break;
	case 10: status = FPGAregisterWRITE("DAC_Park_Chan10", dacParkValue); break;
	case 11: status = FPGAregisterWRITE("DAC_Park_Chan11", dacParkValue); break;
	}

	switch (outputChannel)
	{
	case 0: status = FPGAregisterWRITE("DAC_Offset_Chan0", dacParkValue); break;
	case 1: status = FPGAregisterWRITE("DAC_Offset_Chan1", dacParkValue); break;
	case 2: status = FPGAregisterWRITE("DAC_Offset_Chan2", dacParkValue); break;
	case 3: status = FPGAregisterWRITE("DAC_Offset_Chan3", dacParkValue); break;
	case 4: status = FPGAregisterWRITE("DAC_Offset_Chan4", dacParkValue); break;
	case 5: status = FPGAregisterWRITE("DAC_Offset_Chan5", dacParkValue); break;
	case 6: status = FPGAregisterWRITE("DAC_Offset_Chan6", dacParkValue); break;
	case 7: status = FPGAregisterWRITE("DAC_Offset_Chan7", dacParkValue); break;
	case 8: status = FPGAregisterWRITE("DAC_Offset_Chan8", dacParkValue); break;
	case 9: status = FPGAregisterWRITE("DAC_Offset_Chan9", dacParkValue); break;
	case 10: status = FPGAregisterWRITE("DAC_Offset_Chan10", dacParkValue); break;
	case 11: status = FPGAregisterWRITE("DAC_Offset_Chan11", dacParkValue); break;
	}

	return status;
}

THORDAQ_STATUS CThordaq::SetDACOffsetValue(ULONG32 outputChannel, double parkValue)
{
	//only analog channels
	if (outputChannel > 11 || parkValue < -10.0 || parkValue > 10.0)
	{
		return STATUS_PARAMETER_SETTINGS_ERROR;
	}

	if (_isDACChannelEnabledForImaging[outputChannel])
	{
		return THORDAQ_STATUS::STATUS_SUCCESSFUL;
	}

	//keep track of parked positions
	_dacParkingPositions[outputChannel] = parkValue;

	USHORT park_mid = parkValue > 0 ? 0x7fff : 0x8000;
	USHORT dacOffsetValue = static_cast<USHORT>(std::floor(parkValue / GALVO_RESOLUTION + 0.5) + park_mid);

	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	switch (outputChannel)
	{
	case 0: status = FPGAregisterWRITE("DAC_Offset_Chan0", dacOffsetValue); break;
	case 1: status = FPGAregisterWRITE("DAC_Offset_Chan1", dacOffsetValue); break;
	case 2: status = FPGAregisterWRITE("DAC_Offset_Chan2", dacOffsetValue); break;
	case 3: status = FPGAregisterWRITE("DAC_Offset_Chan3", dacOffsetValue); break;
	case 4: status = FPGAregisterWRITE("DAC_Offset_Chan4", dacOffsetValue); break;
	case 5: status = FPGAregisterWRITE("DAC_Offset_Chan5", dacOffsetValue); break;
	case 6: status = FPGAregisterWRITE("DAC_Offset_Chan6", dacOffsetValue); break;
	case 7: status = FPGAregisterWRITE("DAC_Offset_Chan7", dacOffsetValue); break;
	case 8: status = FPGAregisterWRITE("DAC_Offset_Chan8", dacOffsetValue); break;
	case 9: status = FPGAregisterWRITE("DAC_Offset_Chan9", dacOffsetValue); break;
	case 10: status = FPGAregisterWRITE("DAC_Offset_Chan10", dacOffsetValue); break;
	case 11: status = FPGAregisterWRITE("DAC_Offset_Chan11", dacOffsetValue); break;
	}

	return status;
}

THORDAQ_STATUS CThordaq::DACSetParkValueForChannels(std::map<UINT, USHORT> dacParkPosition)
{
	//keep track of parked positions
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	for (UINT i = 0; i < DAC_ANALOG_CHANNEL_COUNT; ++i)
	{
		if (dacParkPosition.find(i) != dacParkPosition.end())
		{
			_dacParkingPositionsFPGA[i] = dacParkPosition[i];

			switch (i)
			{
			case 0: status = FPGAregisterWRITE("DAC_Park_Chan0", dacParkPosition[i]); break;
			case 1: status = FPGAregisterWRITE("DAC_Park_Chan1", dacParkPosition[i]); break;
			case 2: status = FPGAregisterWRITE("DAC_Park_Chan2", dacParkPosition[i]); break;
			case 3: status = FPGAregisterWRITE("DAC_Park_Chan3", dacParkPosition[i]); break;
			case 4: status = FPGAregisterWRITE("DAC_Park_Chan4", dacParkPosition[i]); break;
			case 5: status = FPGAregisterWRITE("DAC_Park_Chan5", dacParkPosition[i]); break;
			case 6: status = FPGAregisterWRITE("DAC_Park_Chan6", dacParkPosition[i]); break;
			case 7: status = FPGAregisterWRITE("DAC_Park_Chan7", dacParkPosition[i]); break;
			case 8: status = FPGAregisterWRITE("DAC_Park_Chan8", dacParkPosition[i]); break;
			case 9: status = FPGAregisterWRITE("DAC_Park_Chan9", dacParkPosition[i]); break;
			case 10: status = FPGAregisterWRITE("DAC_Park_Chan10", dacParkPosition[i]); break;
			case 11: status = FPGAregisterWRITE("DAC_Park_Chan11", dacParkPosition[i]); break;
			}
		}
	}

	return status;
}

THORDAQ_STATUS CThordaq::DACSetWaveformConfigurationForStaticLoading(DAC_FREERUN_WAVEFORM_CONFIG dacConfig)
{
	if (dacConfig.dacCtrl.size() <= 0)
	{
		return STATUS_PARAMETER_SETTINGS_ERROR;
	}

	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	_dacApproachingNSamplesCallbackPtrs.clear();
	_dacCycleDoneCallbackPtrs.clear();
	_dacApproachinLoadedWaveformEndCallbackPtrs.clear();
	_dacDescTableTracker.clear();
	_dacDescTableSamplesTracker.clear();
	_dacCtrl.dacCtrl.clear();
	_dacCtrl.dacCtrlPart2.clear();
	_dacWaveformCurrentCycleCount = 0;
	_experimentType = ThorDAQExperimentType::DACFreeRunModeStaticLoad;
	_dacCtrl = dacConfig;

	UINT maxDescChannel = 0;
	UINT64 maxDescCount = 0;

	//into memory
	std::map<UINT, DAC_CRTL_STRUCT> dacCtrl;
	std::map<UINT, DAC_WAVE_DESC_STRUCT> dacWaveDescs;

	std::map<UINT8, bool> enabledChannels;

	//put DAC settings in necessary struct to load dac related registers (DAC_CRTL_STRUCT)
	//put waveform in necessary struct to create and load the DAC DMA descriptor table (DAC_WAVE_DESC_STRUCT)
	for (UINT8 i = 0; i < WAVETABLE_CHANNEL_COUNT; i++)
	{
		if (dacConfig.dacCtrl.find(i) != dacConfig.dacCtrl.end())
		{
			DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT* gctrol1 = &dacConfig.dacCtrl[i];
			if (gctrol1->update_rate > DAC_MAX_UPDATERATE || gctrol1->update_rate < DAC_MIN_UPDATERATE)
			{
				status = STATUS_DAC_UPDATE_RATE_OUT_OF_RANGE;
				break;
			}

			enabledChannels[i] = true;
			//set DAC settings

			DAC_CRTL_STRUCT dacCtrlStruct = DAC_CRTL_STRUCT();
			dacCtrlStruct.park_val = 0;
			dacCtrlStruct.update_rate = gctrol1->update_rate;
			dacCtrlStruct.output_port = gctrol1->output_port;
			dacCtrlStruct.enablePort = true;
			dacCtrlStruct.offset_val = gctrol1->offset;
			dacCtrlStruct.park_val = gctrol1->park;
			dacCtrlStruct.triggerSettings = gctrol1->triggerSettings;
			dacCtrlStruct.hSync = false;
			dacCtrlStruct.filterInhibit = gctrol1->filterInhibit;
			dacCtrl.insert({ i, dacCtrlStruct });

			DAC_WAVE_DESC_STRUCT waveDesc = DAC_WAVE_DESC_STRUCT();

			//depending on the length of the waveform we want to build the waveform a little bit differently
			//so we use two different functions to do so
			UINT64 descCountP1 = 0;
			UINT64 descCountP2 = 0;
			if (dacConfig.generalSettings.numberOfCycles > 1)
			{
				UINT64 numberOfCyclesP1 = dacConfig.generalSettings.numberOfCycles - 1;
				waveDesc.dacWaveformGroups = DACCreateDACWaveformGroupsStaticLoadSetup(gctrol1, numberOfCyclesP1, descCountP1);


				DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT* gctrol2 = &dacConfig.dacCtrlPart2[i];

				auto tempGroup = DACCreateDACWaveformGroupsLastSectionStaticLoadSetup(gctrol2, descCountP2);


				for (int k = 0; k < tempGroup.size(); ++k)
				{
					waveDesc.dacWaveformGroups.push_back(tempGroup[k]);
				}
			}
			else
			{
				DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT* gctrol2 = &dacConfig.dacCtrlPart2[i];

				waveDesc.dacWaveformGroups = DACCreateDACWaveformGroupsLastSectionStaticLoadSetup(gctrol2, descCountP2);
			}

			UINT64 descCount = descCountP1 + descCountP2;

			//extract the max number of descriptors
			if (maxDescCount < descCount)
			{
				maxDescCount = descCount;
				maxDescChannel = i;
			}

			waveDesc.finishPlaybackInLastGroup = true;

			dacWaveDescs.insert({ i, waveDesc });
		}
	}

	if (status == STATUS_SUCCESSFUL)
	{

		status = DACSetContinuousModeGeneralSettings(dacWaveDescs, dacCtrl, false, enabledChannels, &dacConfig.generalSettings);

		//calculate the interrupts per cycle
		_dacStaticLoadNumberOfDescPerInterrupt = maxDescCount > DAC_MAX_DESC_BEFORE_INTERRUPT ? DAC_STATIC_DESC_PER_INTERRUPT : maxDescCount;

		_dacStaticLoadInterruptsPerCycle = maxDescCount <= DAC_MAX_DESC_BEFORE_INTERRUPT ? 1 : static_cast<UINT64>(ceil((double)maxDescCount / (double)_dacStaticLoadNumberOfDescPerInterrupt));

		DACSetWaveGenInterruptsOnChannel(maxDescChannel, (UINT8)_dacStaticLoadNumberOfDescPerInterrupt);
	}

	//memory cleanup
	for (auto& waveformCtrl : dacWaveDescs)
	{
		for (auto& waveGroup : waveformCtrl.second.dacWaveformGroups)
		{
			SAFE_DELETE_ARRAY(waveGroup.waveformBuffer);
		}
		waveformCtrl.second.dacWaveformGroups.clear();
	}
	_dacContinuousModeEnabledChannels = enabledChannels;
	return status;
}

THORDAQ_STATUS CThordaq::DACSetWaveformConfigurationForDynamicLoading(DAC_FREERUN_WAVEFORM_CONFIG dacConfig)
{
	if (dacConfig.dacCtrl.size() <= 0)
	{
		return STATUS_PARAMETER_SETTINGS_ERROR;
	}

	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	//clear this wavefrom data before we use it
	for (auto& waveformCtrl : _dacPresetDynamicLoadWaveDescs)
	{
		for (auto& waveGroup : waveformCtrl.second.dacWaveformGroups)
		{
			SAFE_DELETE_ARRAY(waveGroup.waveformBuffer);
		}
		waveformCtrl.second.dacWaveformGroups.clear();
	}
	_dacPresetDynamicLoadWaveDescs.clear();

	_dacApproachingNSamplesCallbackPtrs.clear();
	_dacCycleDoneCallbackPtrs.clear();
	_dacApproachinLoadedWaveformEndCallbackPtrs.clear();
	_dacDescTableTracker.clear();
	_dacDescTableSamplesTracker.clear();
	_dacCtrl.dacCtrl.clear();
	_dacCtrl.dacCtrlPart2.clear();
	_dacWaveformCurrentCycleCount = 0;
	_experimentType = ThorDAQExperimentType::DACFreeRunModeDynamicLoad;
	_dacCtrl = dacConfig;

	UINT maxDescChannel = 0;
	UINT64 maxDescCount = 0;

	//into memory
	std::map<UINT, DAC_CRTL_STRUCT> dacCtrl;
	std::map<UINT, DAC_WAVE_DESC_STRUCT> dacWaveDescs;

	std::map<UINT8, bool> enabledChannels;
	size_t extraSamples = (_dacCtrl.generalSettings.numberOfSamplesPerCycle) % DAC_DESC_SAMPLE_COUNT_RESOLUTION;
	extraSamples = extraSamples > 0 ? DAC_DESC_SAMPLE_COUNT_RESOLUTION - extraSamples : 0;
	_dacCtrl.generalSettings.numberOfSamplesPerCycle += extraSamples;
	_dacDynamicLoadLastDescSampleCount = 0;

	//put DAC settings in necessary struct to load dac related registers (DAC_CRTL_STRUCT)
	//put waveform in necessary struct to create and load the DAC DMA descriptor table (DAC_WAVE_DESC_STRUCT)
	for (UINT8 i = 0; i < WAVETABLE_CHANNEL_COUNT; i++)
	{
		if (dacConfig.dacCtrl.find(i) != dacConfig.dacCtrl.end())
		{
			DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT* gctrol = &dacConfig.dacCtrl[i];
			if (gctrol->update_rate > DAC_MAX_UPDATERATE || gctrol->update_rate < DAC_MIN_UPDATERATE)
			{
				status = STATUS_DAC_UPDATE_RATE_OUT_OF_RANGE;
				break;
			}

			enabledChannels[i] = true;
			//set DAC settings

			DAC_CRTL_STRUCT dacCtrlStruct = DAC_CRTL_STRUCT();
			dacCtrlStruct.park_val = 0;
			dacCtrlStruct.update_rate = gctrol->update_rate;
			dacCtrlStruct.output_port = gctrol->output_port;
			dacCtrlStruct.enablePort = true;
			dacCtrlStruct.offset_val = gctrol->offset;
			dacCtrlStruct.park_val = gctrol->park;
			dacCtrlStruct.triggerSettings = gctrol->triggerSettings;
			dacCtrlStruct.hSync = false;
			dacCtrlStruct.filterInhibit = gctrol->filterInhibit;
			dacCtrl.insert({ i, dacCtrlStruct });

			DAC_WAVE_DESC_STRUCT waveDesc = DAC_WAVE_DESC_STRUCT();

			//depending on the length of the waveform we want to build the waveform a little bit differently
			//so we use two different functions to do so
			UINT64 descCount = 0;

			UINT64 minSamples = CalculateMinSamplesStart(gctrol->update_rate);
			UINT64 minSamplesLastDesc = CalculateMinSamplesEnd(gctrol->update_rate);
			if (gctrol->waveform_buffer_size / sizeof(USHORT) >= DAC_TRANSMIT_BUFFER_MAX / sizeof(USHORT) + minSamples)
			{
				std::vector<CThordaq::DAC_WAVE_GROUP_STRUCT> dacWaveformGroups;

				DACCreateDACWaveformGroupsLongWaveformForDynamicLoadSetup(gctrol, descCount, dacWaveformGroups);

				waveDesc.dacWaveformGroups = dacWaveformGroups;
			}
			else
			{
				if ((gctrol->waveform_buffer_size / sizeof(USHORT)) >= minSamplesLastDesc + minSamples)
				{
					std::vector<CThordaq::DAC_WAVE_GROUP_STRUCT> dacWaveformGroups;

					DACCreateDACWaveformGroupsShortWaveformForDynamicLoadSetup(gctrol, descCount, dacWaveformGroups);

					waveDesc.dacWaveformGroups = dacWaveformGroups;
				}
				else
				{
					status = STATUS_DAC_TOO_FEW_SAMPLES;
					break;
				}
			}

			//extract the max number of descriptors
			if (maxDescCount < descCount)
			{
				maxDescCount = descCount;
				maxDescChannel = i;
			}

			if (0 == _dacDynamicLoadLastDescSampleCount)
			{
				UINT64 minSamples = minSamplesLastDesc;
				//thordaq requires that the number of samples is always even
				if (minSamples % DAC_DESC_SAMPLE_COUNT_RESOLUTION != 0)
				{
					++minSamples;
				}
				_dacDynamicLoadLastDescSampleCount = waveDesc.dacWaveformGroups[waveDesc.dacWaveformGroups.size() - 1].waveform_buffer_size >> 1;
			}

			waveDesc.finishPlaybackInLastGroup = dacConfig.generalSettings.numberOfCycles == 1 && dacConfig.generalSettings.numberOfSamplesPerCycle == gctrol->waveform_buffer_size / 2;

			dacWaveDescs.insert({ i, waveDesc });
		}
	}

	if (status == STATUS_SUCCESSFUL)
	{
		status = DACSetContinuousModeGeneralSettings(dacWaveDescs, dacCtrl, true, enabledChannels, &dacConfig.generalSettings);
	}


	_dacPresetDynamicLoadWaveDescs = dacWaveDescs;

	_dacProcessingPresetWaveformForDynamicLoading = false;
	_dacDynamicLoadingPresetWaveform = false;
	_dacPresetWaveformForDynamicLoadingReady = false;

	_dacContinuousModeEnabledChannels = enabledChannels;

	return status;
}

THORDAQ_STATUS  CThordaq::DACPresetNextWaveformSection(std::map<UINT, DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT> dacCtrlSettings, bool isLastPartOfWaveform)
{
	if (_dacProcessingPresetWaveformForDynamicLoading)
	{
		return THORDAQ_STATUS::STATUS_WRITE_BUFFER_ERROR;
	}
	_dacProcessingPresetWaveformForDynamicLoading = true;


	while (_dacDynamicLoadingPresetWaveform)
	{
		if ((_dacAbortContinousMode || !_dacContinuousModeStartStopStatus))
		{
			return THORDAQ_STATUS::STATUS_ACQUISITION_ABORTED;
		}
	}

	_dacPresetWaveformForDynamicLoadingReady = false;

	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;

	//TODO: check for waveform length, make sure it is not longer than the original, shorter than the original waveforms are ok

	//put DAC settings in necessary struct to load dac related registers (DAC_CRTL_STRUCT)
	//put waveform in necessary struct to create and load the DAC DMA descriptor table (DAC_WAVE_DESC_STRUCT)

	for (UINT8 i = 0; i < WAVETABLE_CHANNEL_COUNT; i++)
	{
		if (dacCtrlSettings.find(i) != dacCtrlSettings.end())
		{
			//set DAC settings
			DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT* gctrol = &dacCtrlSettings[i];

			DAC_WAVE_DESC_STRUCT waveDesc;

			if (_dacPresetDynamicLoadWaveDescs.find(i) != _dacPresetDynamicLoadWaveDescs.end())
			{
				waveDesc = _dacPresetDynamicLoadWaveDescs[i];
			}
			else
			{
				waveDesc = DAC_WAVE_DESC_STRUCT();
			}
			//depending on the length of the waveform we want to build the waveform a little bit differently
			//so we use two different functions to do so
			UINT64 descCount = 0;
			UINT64 minSamples = CalculateMinSamplesStart(gctrol->update_rate);
			UINT64 minSamplesLastDesc = CalculateMinSamplesEnd(gctrol->update_rate);
			if ((gctrol->waveform_buffer_size >> 1) >= (DAC_TRANSMIT_BUFFER_MAX >> 1) + minSamples)
			{
				DACCreateDACWaveformGroupsLongWaveformForDynamicLoadSetup(gctrol, descCount, waveDesc.dacWaveformGroups);
			}
			else
			{
				if ((gctrol->waveform_buffer_size >> 1) >= minSamplesLastDesc + minSamples)
				{
					DACCreateDACWaveformGroupsShortWaveformForDynamicLoadSetup(gctrol, descCount, waveDesc.dacWaveformGroups);
				}
				else
				{
					status = STATUS_DAC_TOO_FEW_SAMPLES;
					break;
				}
			}

			waveDesc.finishPlaybackInLastGroup = isLastPartOfWaveform;

			_dacPresetDynamicLoadWaveDescs[i] = waveDesc;
		}
	}

	_dacProcessingPresetWaveformForDynamicLoading = false;
	_dacPresetWaveformForDynamicLoadingReady = true;
	return status;
}

/// <summary>
/// dynamically load dac waveforms while another waveform is already playing, this function will smoothly connect to the next waveform
/// </summary>
/// <param name="dacCtrlSettings"></param>
/// <param name="isLastPartOfWaveform"></param>
/// <returns></returns>
THORDAQ_STATUS CThordaq::DACDynamicLoadPresetWaveform(bool isLastPartOfWaveform)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	//into memory
	std::map<UINT, DAC_WAVE_DESC_STRUCT>& dacWaveDescs = _dacPresetDynamicLoadWaveDescs;

	if (_dacAbortContinousMode)
	{
		return THORDAQ_STATUS::STATUS_ACQUISITION_ABORTED;
	}

	_dacDescriptorMutex.lock();

	//TODO: check for waveform length, make sure it is not longer than the original, shorter than the original waveforms are ok

	//put DAC settings in necessary struct to load dac related registers (DAC_CRTL_STRUCT)
	//put waveform in necessary struct to create and load the DAC DMA descriptor table (DAC_WAVE_DESC_STRUCT)

	while (_dacProcessingPresetWaveformForDynamicLoading)
	{
		if ((_dacAbortContinousMode || !_dacContinuousModeStartStopStatus))
		{
			return THORDAQ_STATUS::STATUS_ACQUISITION_ABORTED;
		}
	}

	//only continue if a new waveform is ready and we are not already loading a new waveform
	if (!_dacPresetWaveformForDynamicLoadingReady)
	{
		_dacDescriptorMutex.unlock();
		return THORDAQ_STATUS::STATUS_WRITE_BUFFER_ERROR;
	}

	if (status == STATUS_SUCCESSFUL)
	{
		//we assume each group needs only one desc
		for (UINT8 k = 0; k < WAVETABLE_CHANNEL_COUNT; k++)
		{
			if (dacWaveDescs.find(k) != dacWaveDescs.end())
			{
				auto& waveGroups = dacWaveDescs[k].dacWaveformGroups;

				auto& samplesTracker = _dacDescTableSamplesTracker[(UINT)k];
				auto& descTableTracker = _dacDescTableTracker[(UINT8)k];

				for (int j = 0; j < waveGroups.size() - 2; ++j)
				{
					if (samplesTracker.size() <= j)
					{
						break;
					}
					status = WriteDDR3((UCHAR*)waveGroups[j].waveformBuffer, samplesTracker[j].address, static_cast<UINT32>(waveGroups[j].waveform_buffer_size));

					auto& dmaDescp = waveGroups[j];

					if (dmaDescp.waveform_buffer_size != samplesTracker[j].size)
					{
						waveGroups[j].waveform_buffer_start_address = samplesTracker[j].address;

						ULONG32 BTT = (ULONG32)(dmaDescp.waveform_buffer_size) >> 2; // divide by 4, same as bit shifting to the right 2 bits

						//TODO: currently looping doesn't work with dynamic loading
						ULONG64 dacDesc = samplesTracker[j].nxtDexc << DAC_NXT_DESC_OFFSET | (UINT64)BTT << DAC_TOTAL_BYTES_TO_SEND_OFFSET | (UINT64)dmaDescp.loopEnable << DAC_LOOP_OFFSET | waveGroups[j].waveform_buffer_start_address;
						const UINT64 DAC_DESC_TABLE_ADDRESS = 0x50000;
						UINT32 addressOffset = (UINT32)(sizeof(UINT64) * samplesTracker[j].descIndex);
						STAT_STRUCT DoMemStatus;
						status = DoMem(WRITE_TO_CARD, BAR3, (PUINT8)(&dacDesc), 0, DAC_DESC_TABLE_ADDRESS + addressOffset, sizeof(ULONG64), &DoMemStatus);
					}
				}
			}
		}

		//wait until the second to last descriptor is done to overwrite it
		for (int i = 0; i < WAVETABLE_CHANNEL_COUNT; ++i)
		{
			if (_dacApproachinLoadedWaveformEndCallbackPtrs.find(i) != _dacApproachinLoadedWaveformEndCallbackPtrs.end())
			{
				auto& samplesTrackerVector = _dacDescTableSamplesTracker[i];
				size_t channelDescCount = samplesTrackerVector.size();

				//TODO: this is not the right method, should instead keep count of where it is when callback call
				while (_DACWaveGenInterruptCounter[i] <= (_DACWaveGenInterruptCountWhenApproachingCallbackCalled[i]))
				{
					if (_dacAbortContinousMode || !_dacContinuousModeStartStopStatus)
					{
						_dacDescriptorMutex.unlock();
						return THORDAQ_STATUS::STATUS_ACQUISITION_ABORTED;
					}
				}
			}
		}

		for (UINT8 k = 0; k < WAVETABLE_CHANNEL_COUNT; k++)
		{
			if (dacWaveDescs.find(k) != dacWaveDescs.end())
			{
				auto& waveGroups = dacWaveDescs[k].dacWaveformGroups;

				auto& samplesTracker = _dacDescTableSamplesTracker[(UINT)k];
				auto& descTableTracker = _dacDescTableTracker[(UINT8)k];

				size_t j = waveGroups.size() - 2;

				if (samplesTracker.size() <= j)
				{
					continue;
				}

				status = WriteDDR3((UCHAR*)waveGroups[j].waveformBuffer, samplesTracker[j].address, static_cast<UINT32>(waveGroups[j].waveform_buffer_size));
				auto& dmaDescp = waveGroups[j];

				waveGroups[j].waveform_buffer_start_address = samplesTracker[j].address;

				ULONG32 BTT = (ULONG32)(dmaDescp.waveform_buffer_size) >> 2; // divide by 4, same as bit shifting to the right 2 bits

				//TODO: currently looping doesn't work with dynamic loading
				ULONG64 dacDesc = samplesTracker[j].nxtDexc << DAC_NXT_DESC_OFFSET | (UINT64)BTT << DAC_TOTAL_BYTES_TO_SEND_OFFSET | (UINT64)dmaDescp.loopEnable << DAC_LOOP_OFFSET | waveGroups[j].waveform_buffer_start_address;
				const UINT64 DAC_DESC_TABLE_ADDRESS = 0x50000;
				UINT32 addressOffset = (UINT32)(sizeof(UINT64) * samplesTracker[j].descIndex);
				STAT_STRUCT DoMemStatus;
				status = DoMem(WRITE_TO_CARD, BAR3, (PUINT8)(&dacDesc), 0, DAC_DESC_TABLE_ADDRESS + addressOffset, sizeof(ULONG64), &DoMemStatus);

			}
		}

		//if is the last part of the waveformget the next available address to set the last waveform portion
		UINT64 lastDDR3Address = 0;
		if (isLastPartOfWaveform)
		{
			for (UINT8 k = 0; k < WAVETABLE_CHANNEL_COUNT; k++)
			{
				if (dacWaveDescs.find(k) != dacWaveDescs.end())
				{
					auto& waveGroups = dacWaveDescs[k].dacWaveformGroups;

					auto& samplesTracker = _dacDescTableSamplesTracker[(UINT)k];
					auto& descTableTracker = _dacDescTableTracker[(UINT8)k];

					for (int j = 0; j < waveGroups.size(); ++j)
					{
						if (samplesTracker.size() <= j)
						{
							break;
						}
						if (lastDDR3Address < samplesTracker[j].address + waveGroups[j].waveform_buffer_size)
						{
							lastDDR3Address = samplesTracker[j].address + waveGroups[j].waveform_buffer_size;
						}
					}
				}
			}
		}

		//if its the last part of the waveform then create the memory needed
		USHORT* lastWavePortion = NULL;
		UINT64 lastLength = _dacDynamicLoadLastDescSampleCount;
		if (isLastPartOfWaveform)
		{
			lastWavePortion = new USHORT[lastLength];
		}

		//wait until the last descriptor is done to overwrite it
		for (int i = 0; i < WAVETABLE_CHANNEL_COUNT; ++i)
		{
			if (_dacApproachinLoadedWaveformEndCallbackPtrs.find(i) != _dacApproachinLoadedWaveformEndCallbackPtrs.end())
			{
				auto& samplesTrackerVector = _dacDescTableSamplesTracker[i];
				size_t channelDescCount = samplesTrackerVector.size();

				//TODO: this is not the right method, should instead keep count of where it is when callback call
				while (_DACWaveGenInterruptCounter[i] <= (_DACWaveGenInterruptCountWhenApproachingCallbackCalled[i] + 1))
				{
					if (_dacAbortContinousMode || !_dacContinuousModeStartStopStatus)
					{
						_dacDescriptorMutex.unlock();
						return THORDAQ_STATUS::STATUS_ACQUISITION_ABORTED;
					}
				}
			}
		}

		for (UINT8 k = 0; k < WAVETABLE_CHANNEL_COUNT; k++)
		{
			if (dacWaveDescs.find(k) != dacWaveDescs.end())
			{
				auto& waveGroups = dacWaveDescs[k].dacWaveformGroups;

				auto& samplesTracker = _dacDescTableSamplesTracker[(UINT)k];
				auto& descTableTracker = _dacDescTableTracker[(UINT8)k];

				size_t j = waveGroups.size() - 1;

				if (samplesTracker.size() <= j)
				{
					continue;
				}

				status = WriteDDR3((UCHAR*)waveGroups[j].waveformBuffer, samplesTracker[j].address, static_cast<UINT32>(waveGroups[j].waveform_buffer_size));
				auto& dmaDescp = waveGroups[j];
				if (isLastPartOfWaveform)
				{
					waveGroups[j].waveform_buffer_start_address = samplesTracker[j].address;

					ULONG32 BTT = (ULONG32)(dmaDescp.waveform_buffer_size) >> 2; // divide by 4, same as bit shifting to the right 2 bits

					//TODO: currently looping doesn't work with dynamic loading
					UINT64 nxt = samplesTracker[j].descIndex + 1;
					ULONG64 dacDesc = nxt << DAC_NXT_DESC_OFFSET | (UINT64)BTT << DAC_TOTAL_BYTES_TO_SEND_OFFSET | (UINT64)dmaDescp.loopEnable << DAC_LOOP_OFFSET | waveGroups[j].waveform_buffer_start_address;
					const UINT64 DAC_DESC_TABLE_ADDRESS = 0x50000;
					UINT32 descAddressOffset = (UINT32)(sizeof(UINT64) * samplesTracker[j].descIndex);
					STAT_STRUCT DoMemStatus;
					status = DoMem(WRITE_TO_CARD, BAR3, (PUINT8)(&dacDesc), 0, DAC_DESC_TABLE_ADDRESS + descAddressOffset, sizeof(ULONG64), &DoMemStatus);

					//set the last descriptor, which is only two samples repeating the last value of the waveform, to repeat it self by pointint to itself as the next desc
					UINT64 lastIndex = (waveGroups[j].waveform_buffer_size >> 1) - 1;

					UINT64 lastSize = lastLength * sizeof(USHORT);
					std::memset(lastWavePortion, waveGroups[j].waveformBuffer[lastIndex], lastSize);

					UINT64 lastAddress = samplesTracker[j].address + samplesTracker[j].size - DAC_DESC_SAMPLE_COUNT_RESOLUTION * sizeof(USHORT); //TODO: is it ok to assume the last 2 samples are the same?

					status = WriteDDR3((UCHAR*)lastWavePortion, lastDDR3Address, static_cast<UINT32>(lastSize));
					BTT = (ULONG32)(lastSize >> 2);
					dacDesc = nxt << DAC_NXT_DESC_OFFSET | (UINT64)BTT << DAC_TOTAL_BYTES_TO_SEND_OFFSET | (UINT64)dmaDescp.loopEnable << DAC_LOOP_OFFSET | lastDDR3Address;
					descAddressOffset = (UINT32)(sizeof(UINT64) * (nxt));

					status = DoMem(WRITE_TO_CARD, BAR3, (PUINT8)(&dacDesc), 0, DAC_DESC_TABLE_ADDRESS + descAddressOffset, sizeof(ULONG64), &DoMemStatus);

					lastDDR3Address += lastSize;

					////add lastLength * 2 more samples to count before its over to ensure waveform ended
					////and this will also ensure the waveform is repeating the last samples when stopped
					////TODO: ensure this is necessary, just one count of lastLength might be enough
					_dacSampleCountToTriggerCycleCompleteEvent[k] += lastLength * 2;

					DAC_DESC_TABLE_SAMPLES_TRACKER samplesTracker = DAC_DESC_TABLE_SAMPLES_TRACKER();
					samplesTracker.channelIndex = (UINT8)k;
					samplesTracker.descIndex = nxt;
					samplesTracker.nxtDexc = nxt;
					samplesTracker.size = lastSize;
					samplesTracker.address = lastDDR3Address;
					_dacDescTableSamplesTracker[k].push_back(samplesTracker);
					_dacDescTableSamplesTracker[k].push_back(samplesTracker);
					++_dacDescriptorsPerChannel[k];
					++_dacDescriptorsPerChannel[k];
				}
				else if (dmaDescp.waveform_buffer_size != samplesTracker[j].size)
				{
					waveGroups[j].waveform_buffer_start_address = samplesTracker[j].address;

					ULONG32 BTT = (ULONG32)(dmaDescp.waveform_buffer_size) >> 2; // divide by 4, same as bit shifting to the right 2 bits

					//TODO: currently looping doesn't work with dynamic loading
					ULONG64 dacDesc = samplesTracker[j].nxtDexc << DAC_NXT_DESC_OFFSET | (UINT64)BTT << DAC_TOTAL_BYTES_TO_SEND_OFFSET | (UINT64)dmaDescp.loopEnable << DAC_LOOP_OFFSET | waveGroups[j].waveform_buffer_start_address;
					const UINT64 DAC_DESC_TABLE_ADDRESS = 0x50000;
					UINT32 addressOffset = (UINT32)(sizeof(UINT64) * samplesTracker[j].descIndex);
					STAT_STRUCT DoMemStatus;
					status = DoMem(WRITE_TO_CARD, BAR3, (PUINT8)(&dacDesc), 0, DAC_DESC_TABLE_ADDRESS + addressOffset, sizeof(ULONG64), &DoMemStatus);
				}
			}
		}
		SAFE_DELETE_ARRAY(lastWavePortion);
	}
	for (UINT8 k = 0; k < WAVETABLE_CHANNEL_COUNT; k++)
	{
		if (_dacApproachinLoadedWaveformEndCallbackPtrs.find(k) != _dacApproachinLoadedWaveformEndCallbackPtrs.end())
		{
			//TODO: what if it's a long waveform? then we could be more than 2 descriptors apparat without falling behind
			//need to account for that possibility
			if ((_DACWaveGenInterruptCountWhenApproachingCallbackCalled[k] + 2) < _DACWaveGenInterruptCounter[k])
			{
				DACStopWaveforms();
				_dacDescriptorMutex.unlock();
				return THORDAQ_STATUS::STATUS_WRITE_BUFFER_ERROR;
			}
		}
	}
	_dacPresetWaveformForDynamicLoadingReady = false;

	_dacDescriptorMutex.unlock();
	return status;
}


/// <summary>
/// dynamically load dac waveforms while another waveform is already playing, this function will smoothly connect to the next waveform
/// </summary>
/// <param name="dacCtrlSettings"></param>
/// <param name="isLastPartOfWaveform"></param>
/// <returns></returns>
THORDAQ_STATUS CThordaq::DACDynamicLoadWaveform(std::map<UINT, DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT> dacCtrlSettings, bool isLastPartOfWaveform)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	//into memory
	std::map<UINT, DAC_WAVE_DESC_STRUCT> dacWaveDescs;

	if (_dacAbortContinousMode || !_dacContinuousModeStartStopStatus)
	{
		return THORDAQ_STATUS::STATUS_ACQUISITION_ABORTED;
	}
	//TODO: check for waveform length, make sure it is not longer than the original, shorter than the original waveforms are ok

	//put DAC settings in necessary struct to load dac related registers (DAC_CRTL_STRUCT)
	//put waveform in necessary struct to create and load the DAC DMA descriptor table (DAC_WAVE_DESC_STRUCT)
	_dacDescriptorMutex.lock();
	for (UINT8 i = 0; i < WAVETABLE_CHANNEL_COUNT; i++)
	{
		if (dacCtrlSettings.find(i) != dacCtrlSettings.end())
		{
			//set DAC settings
			DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT* gctrol = &dacCtrlSettings[i];

			DAC_WAVE_DESC_STRUCT waveDesc = DAC_WAVE_DESC_STRUCT();

			//depending on the length of the waveform we want to build the waveform a little bit differently
			//so we use two different functions to do so
			UINT64 descCount = 0;
			UINT64 minSamples = CalculateMinSamplesStart(gctrol->update_rate);
			UINT64 minSamplesLastDesc = CalculateMinSamplesEnd(gctrol->update_rate);
			if (gctrol->waveform_buffer_size / sizeof(USHORT) >= DAC_TRANSMIT_BUFFER_MAX / sizeof(USHORT) + minSamples)
			{
				std::vector<CThordaq::DAC_WAVE_GROUP_STRUCT> dacWaveformGroups;

				DACCreateDACWaveformGroupsLongWaveformForDynamicLoadSetup(gctrol, descCount, dacWaveformGroups);

				waveDesc.dacWaveformGroups = dacWaveformGroups;
			}
			else
			{
				if ((gctrol->waveform_buffer_size / sizeof(USHORT)) >= minSamplesLastDesc + minSamples)
				{
					std::vector<CThordaq::DAC_WAVE_GROUP_STRUCT> dacWaveformGroups;

					DACCreateDACWaveformGroupsShortWaveformForDynamicLoadSetup(gctrol, descCount, dacWaveformGroups);

					waveDesc.dacWaveformGroups = dacWaveformGroups;
				}
				else
				{
					status = STATUS_DAC_TOO_FEW_SAMPLES;
					break;
				}
			}

			waveDesc.finishPlaybackInLastGroup = isLastPartOfWaveform;

			dacWaveDescs.insert({ i, waveDesc });
		}
	}

	if (status == STATUS_SUCCESSFUL)
	{
		//we assume each group needs only one desc
		for (UINT8 k = 0; k < WAVETABLE_CHANNEL_COUNT; k++)
		{
			if (dacWaveDescs.find(k) != dacWaveDescs.end())
			{
				auto& waveGroups = dacWaveDescs[k].dacWaveformGroups;

				auto& samplesTracker = _dacDescTableSamplesTracker[(UINT)k];
				auto& descTableTracker = _dacDescTableTracker[(UINT8)k];

				for (int j = 0; j < waveGroups.size() - 1; ++j)
				{
					status = WriteDDR3((UCHAR*)waveGroups[j].waveformBuffer, samplesTracker[j].address, static_cast<UINT32>(waveGroups[j].waveform_buffer_size));

					auto& dmaDescp = waveGroups[j];

					if (dmaDescp.waveform_buffer_size != samplesTracker[j].size)
					{
						waveGroups[j].waveform_buffer_start_address = samplesTracker[j].address;

						ULONG32 BTT = (ULONG32)(dmaDescp.waveform_buffer_size) >> 2; // divide by 4, same as bit shifting to the right 2 bits

						//TODO: currently looping doesn't work with dynamic loading
						ULONG64 dacDesc = samplesTracker[j].nxtDexc << DAC_NXT_DESC_OFFSET | (UINT64)BTT << DAC_TOTAL_BYTES_TO_SEND_OFFSET | (UINT64)dmaDescp.loopEnable << DAC_LOOP_OFFSET | waveGroups[j].waveform_buffer_start_address;
						const UINT64 DAC_DESC_TABLE_ADDRESS = 0x50000;
						UINT32 addressOffset = (UINT32)(sizeof(UINT64) * samplesTracker[j].descIndex);
						STAT_STRUCT DoMemStatus;
						status = DoMem(WRITE_TO_CARD, BAR3, (PUINT8)(&dacDesc), 0, DAC_DESC_TABLE_ADDRESS + addressOffset, sizeof(ULONG64), &DoMemStatus);
					}
				}
			}
		}

		//if is the last part of the waveformget the next available address to set the last waveform portion
		UINT64 lastDDR3Address = 0;
		if (isLastPartOfWaveform)
		{
			for (UINT8 k = 0; k < WAVETABLE_CHANNEL_COUNT; k++)
			{
				if (dacWaveDescs.find(k) != dacWaveDescs.end())
				{
					auto& waveGroups = dacWaveDescs[k].dacWaveformGroups;

					auto& samplesTracker = _dacDescTableSamplesTracker[(UINT)k];
					auto& descTableTracker = _dacDescTableTracker[(UINT8)k];

					for (int j = 0; j < waveGroups.size(); ++j)
					{

						if (lastDDR3Address < samplesTracker[j].address + waveGroups[j].waveform_buffer_size)
						{
							lastDDR3Address = samplesTracker[j].address + waveGroups[j].waveform_buffer_size;
						}
					}
				}
			}
		}

		//if its the last part of the waveform then create the memory needed
		USHORT* lastWavePortion = NULL;
		UINT64 lastLength = DAC_MIN_SAMPLES_END_DESC;
		if (isLastPartOfWaveform)
		{
			lastWavePortion = new USHORT[lastLength];
		}

		//wait until the last descriptor is done to overwrite it
		for (int i = 0; i < WAVETABLE_CHANNEL_COUNT; ++i)
		{
			if (_dacApproachinLoadedWaveformEndCallbackPtrs.find(i) != _dacApproachinLoadedWaveformEndCallbackPtrs.end())
			{
				auto& samplesTrackerVector = _dacDescTableSamplesTracker[i];
				size_t channelDescCount = samplesTrackerVector.size();

				//TODO: this is not the right method, should instead keep count of where it is when callback call
				while (_DACWaveGenInterruptCounter[i] <= (_DACWaveGenInterruptCountWhenApproachingCallbackCalled[i]))
				{
					if ((_dacAbortContinousMode || !_dacContinuousModeStartStopStatus))
					{

						for (auto& waveformCtrl : dacWaveDescs)
						{
							for (auto& waveGroup : waveformCtrl.second.dacWaveformGroups)
							{
								SAFE_DELETE_ARRAY(waveGroup.waveformBuffer);
							}
							waveformCtrl.second.dacWaveformGroups.clear();
						}

						return THORDAQ_STATUS::STATUS_ACQUISITION_ABORTED;
					}
				}
			}
		}

		for (UINT8 k = 0; k < WAVETABLE_CHANNEL_COUNT; k++)
		{
			if (dacWaveDescs.find(k) != dacWaveDescs.end())
			{
				auto& waveGroups = dacWaveDescs[k].dacWaveformGroups;

				auto& samplesTracker = _dacDescTableSamplesTracker[(UINT)k];
				auto& descTableTracker = _dacDescTableTracker[(UINT8)k];

				size_t j = waveGroups.size() - 1;

				status = WriteDDR3((UCHAR*)waveGroups[j].waveformBuffer, samplesTracker[j].address, static_cast<UINT32>(waveGroups[j].waveform_buffer_size));
				auto& dmaDescp = waveGroups[j];
				if (isLastPartOfWaveform)
				{
					waveGroups[j].waveform_buffer_start_address = samplesTracker[j].address;

					ULONG32 BTT = (ULONG32)(dmaDescp.waveform_buffer_size) >> 2; // divide by 4, same as bit shifting to the right 2 bits

					//TODO: currently looping doesn't work with dynamic loading
					UINT64 nxt = samplesTracker[j].descIndex + 1;
					ULONG64 dacDesc = nxt << DAC_NXT_DESC_OFFSET | (UINT64)BTT << DAC_TOTAL_BYTES_TO_SEND_OFFSET | (UINT64)dmaDescp.loopEnable << DAC_LOOP_OFFSET | waveGroups[j].waveform_buffer_start_address;
					const UINT64 DAC_DESC_TABLE_ADDRESS = 0x50000;
					UINT32 descAddressOffset = (UINT32)(sizeof(UINT64) * samplesTracker[j].descIndex);
					STAT_STRUCT DoMemStatus;
					status = DoMem(WRITE_TO_CARD, BAR3, (PUINT8)(&dacDesc), 0, DAC_DESC_TABLE_ADDRESS + descAddressOffset, sizeof(ULONG64), &DoMemStatus);

					//set the last descriptor, which is only two samples repeating the last value of the waveform, to repeat it self by pointint to itself as the next desc

					UINT64 lastIndex = waveGroups[j].waveform_buffer_size / sizeof(USHORT) - 1;
					UINT64 lastSize = lastLength * sizeof(USHORT);
					std::memset(lastWavePortion, waveGroups[j].waveformBuffer[lastIndex], lastSize);

					//for (int i = 0; i < DAC_DESC_SAMPLE_COUNT_RESOLUTION; ++i)
					//{
					//	lastWavePortion[i] = waveGroups[j].waveformBuffer[waveGroups[j].waveform_buffer_size / sizeof(USHORT) - 1]; //copy the last value
					//}
					UINT64 lastAddress = samplesTracker[j].address + samplesTracker[j].size - DAC_DESC_SAMPLE_COUNT_RESOLUTION * sizeof(USHORT); //TODO: is it ok to assume the last 2 samples are the same?

					status = WriteDDR3((UCHAR*)lastWavePortion, lastDDR3Address, static_cast<UINT32>(lastSize));
					BTT = (ULONG32)(lastSize >> 2);
					dacDesc = nxt << DAC_NXT_DESC_OFFSET | (UINT64)BTT << DAC_TOTAL_BYTES_TO_SEND_OFFSET | (UINT64)dmaDescp.loopEnable << DAC_LOOP_OFFSET | lastDDR3Address;
					descAddressOffset = (UINT32)(sizeof(UINT64) * (nxt));

					status = DoMem(WRITE_TO_CARD, BAR3, (PUINT8)(&dacDesc), 0, DAC_DESC_TABLE_ADDRESS + descAddressOffset, sizeof(ULONG64), &DoMemStatus);

					lastDDR3Address += lastSize;

					//add lastLength * 2 more samples to count before its over to ensure waveform ended
					//and this will also ensure the waveform is repeating the last samples when stopped
					_dacSampleCountToTriggerCycleCompleteEvent[k] += lastLength * 2;

					DAC_DESC_TABLE_SAMPLES_TRACKER samplesTracker = DAC_DESC_TABLE_SAMPLES_TRACKER();
					samplesTracker.channelIndex = (UINT8)k;
					samplesTracker.descIndex = nxt;
					samplesTracker.nxtDexc = nxt;
					samplesTracker.size = lastSize;
					samplesTracker.address = lastDDR3Address;
					_dacDescTableSamplesTracker[k].push_back(samplesTracker);
					_dacDescTableSamplesTracker[k].push_back(samplesTracker);
				}
				else if (dmaDescp.waveform_buffer_size != samplesTracker[j].size)
				{
					waveGroups[j].waveform_buffer_start_address = samplesTracker[j].address;

					ULONG32 BTT = (ULONG32)(dmaDescp.waveform_buffer_size) >> 2; // divide by 4, same as bit shifting to the right 2 bits

					//TODO: currently looping doesn't work with dynamic loading
					ULONG64 dacDesc = samplesTracker[j].nxtDexc << DAC_NXT_DESC_OFFSET | (UINT64)BTT << DAC_TOTAL_BYTES_TO_SEND_OFFSET | (UINT64)dmaDescp.loopEnable << DAC_LOOP_OFFSET | waveGroups[j].waveform_buffer_start_address;
					const UINT64 DAC_DESC_TABLE_ADDRESS = 0x50000;
					UINT32 addressOffset = (UINT32)(sizeof(UINT64) * samplesTracker[j].descIndex);
					STAT_STRUCT DoMemStatus;
					status = DoMem(WRITE_TO_CARD, BAR3, (PUINT8)(&dacDesc), 0, DAC_DESC_TABLE_ADDRESS + addressOffset, sizeof(ULONG64), &DoMemStatus);
				}
			}
		}
		SAFE_DELETE_ARRAY(lastWavePortion);
	}

	for (auto& waveformCtrl : dacWaveDescs)
	{
		for (auto& waveGroup : waveformCtrl.second.dacWaveformGroups)
		{
			SAFE_DELETE_ARRAY(waveGroup.waveformBuffer);
		}
		waveformCtrl.second.dacWaveformGroups.clear();
	}
	_dacDescriptorMutex.unlock();
	return status;
}

THORDAQ_STATUS CThordaq::DACGetMinSamples(double dacUpdateRate, UINT64& minSamples)
{
	if (dacUpdateRate > DAC_MAX_UPDATERATE || dacUpdateRate < DAC_MIN_UPDATERATE)
	{
		return STATUS_DAC_UPDATE_RATE_OUT_OF_RANGE;
	}

	//TODO: define some ranges for the DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD. It appears the value needs to be longer for faster update rates, but can be much shorter for slower update rates.
	minSamples = 2 * CalculateMinSamplesStart(dacUpdateRate) + 2 * CalculateMinSamplesEnd(dacUpdateRate);

	return STATUS_SUCCESSFUL;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS CThordaq::StartDACWaveforms()
 *
 * @brief	Start acquisition.
 *
 * @author	BGB
 * @date	10/2020
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/
THORDAQ_STATUS CThordaq::DACStartWaveforms()
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;
	if (gPtrAcqCtrl != nullptr)
	{
		_dacAbortContinousMode = false;
		_dacContinuousModeStartStopStatus = true;

		// reset counters
		_dacWaveformCurrentCycleCount = 0;
		std::memset(_DACWaveGenInterruptCounter, 0, sizeof(_DACWaveGenInterruptCounter));
		std::memset(_DACWaveGenInterruptCountWhenApproachingCallbackCalled, 0, sizeof(_DACWaveGenInterruptCountWhenApproachingCallbackCalled));
		std::memset(_dacDescCountToTriggerApproachingNSamplesEvent, 0, sizeof(_dacDescCountToTriggerApproachingNSamplesEvent));
		std::memset(_dacDescCountToTriggerApproachinLoadedWaveformEndEvent, 0, sizeof(_dacDescCountToTriggerApproachinLoadedWaveformEndEvent));
		std::memset(_dacSampleCountToTriggerCycleCompleteEvent, 0, sizeof(_dacSampleCountToTriggerCycleCompleteEvent));

		switch (_experimentType)
		{
		case ThorDAQExperimentType::Imaging: return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;
		case ThorDAQExperimentType::DACFreeRunModeDynamicLoad:
			for (UINT8 i = 0; i < WAVETABLE_CHANNEL_COUNT; i++)
			{
				if (_dacApproachingNSamplesCallbackPtrs.find(i) != _dacApproachingNSamplesCallbackPtrs.end())
				{
					size_t descNum = _dacDescTableSamplesTracker[i].size();
					_dacDescCountToTriggerApproachingNSamplesEvent[i] = descNum > 0 ? descNum - 1 : 0;
				}
				if (_dacApproachinLoadedWaveformEndCallbackPtrs.find(i) != _dacApproachinLoadedWaveformEndCallbackPtrs.end())
				{
					size_t descNum = _dacDescTableSamplesTracker[i].size();
					_dacDescCountToTriggerApproachinLoadedWaveformEndEvent[i] = descNum > 0 ? descNum - 2 : 0;
				}

				if (_dacCycleDoneCallbackPtrs.find(i) != _dacCycleDoneCallbackPtrs.end())
				{
					_dacSampleCountToTriggerCycleCompleteEvent[i] = _dacCtrl.generalSettings.numberOfSamplesPerCycle;
				}
			}

			SAFE_DELETE_PTR(_dacDescriptorCountTrackingThread);
			_dacDescriptorCountTrackingThread = new std::thread([this] { this->DACTrackCompletedDescriptorsDynamicLoadingWaveform(); });
			_dacDescriptorCountTrackingThread->detach();
			SetThreadPriority(_dacDescriptorCountTrackingThread->native_handle(), THREAD_PRIORITY_HIGHEST);
			break;
		case ThorDAQExperimentType::DACFreeRunModeStaticLoad:
			SAFE_DELETE_PTR(_dacDescriptorCountTrackingThread);
			_dacDescriptorCountTrackingThread = new std::thread([this] { this->DACTrackCompletedDescriptorsStaticWaveform(); });
			_dacDescriptorCountTrackingThread->detach();
			SetThreadPriority(_dacDescriptorCountTrackingThread->native_handle(), THREAD_PRIORITY_HIGHEST);

			break;
		default:
			break;
		}

		std::memset(_DACWaveGenInterruptCounter, 0, sizeof(_DACWaveGenInterruptCounter)); // reset counters

		UINT64 currentVal = 0x0;
		status = FPGAregisterRead(_DACWaveGen3PSyncControlRegIndex, -1, &currentVal);  // clears FPGA's ISR, but not our Shadow bit fields
		_rearmDAC1 = 0x20000000 | currentVal; //setting bit 29 to 1
		_rearmDAC0 = (~(0x20000000)) & currentVal; //setting bit 29 to 0 
		if (!_imagingStartStopStatus)
		{
			_usrIrqWaitStruct.boardNum = gBoardIndex;      // routine args:  boardIndex, DMA Bank indicator, int. timeout, etc.
			_usrIrqWaitStruct.DMA_Bank = 0;
			_usrIrqWaitStruct.NWL_Common_DMA_Register_Block = &_NWL_Common_DMA_Register;
			SAFE_DELETE_PTR(_irqThread); // prior thread should have terminated
			_irqThread = new std::thread([this] { this->APItdUserIntTask(&_usrIrqWaitStruct); });
			_irqThread->detach();  // makes WinOS responsible for releasing thread resources on termination.
			SetThreadPriority(_irqThread->native_handle(), THREAD_PRIORITY_HIGHEST);
			//give time to the irq thread to start before setting the run bit
			Sleep(1);

			// [APItdUserIntTask() thread terminates as normal exit after APItdCancelUserIntTask() is called]


			FPGAregisterWRITE("NWL_UserIntEnable", 0x1); // ENable FPGA hardare interrupts

			FPGAregisterWRITE("GIGCR0_LED2", 0x1);      //  Acq. LED ON	
		}
		else
		{
			//if is preview imaging then we can set the global run stop to low and then high so the dac descriptors start from 0
			if (_isPreviewImaging)
			{
				GlobalSCANstart(false);
				Sleep(1);
				//set imaging settings again and then run
				APIimageAcqConfig(gPtrAcqCtrl);
				Sleep(5);
				GlobalSCANstart(true);
				Sleep(50);
			}
			//we need to sleep to let the descriptor checking thread start
			Sleep(1);
		}

		FPGAregisterWRITE("DACWavePerChannelRunStop", _DACPerChannelRuntBitSelection);  // start everything	

		gPtrAcqCtrl->gblCtrl.run_stop_mode = PACKET_GEN_RUN; // legacy code concern	

		if (status != THORDAQ_STATUS::STATUS_SUCCESSFUL)
		{
			_dacContinuousModeStartStopStatus = false;
		}
	}
	return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS CThordaq::StopDACWaveforms()
 *
 * @brief	Stop acquisition.
 *
 * @author	BGB
 * @date	10/2020
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS CThordaq::DACStopWaveforms()
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	if (_stoppingDAC)
	{
		return status;
	}
	_stoppingDAC = true;
	if (!_imagingStartStopStatus)
	{
		FPGAregisterWRITE("NWL_UserIntEnable", 0x0); // Disable FPGA hardare interrupts 
		FPGAregisterWRITE("GIGCR0_LED2", 0x0);      // Acq. LED OFF
	}

	wchar_t logMsg[MSG_SIZE];

	if (!_imagingStartStopStatus)
	{
		status = APItdCancelUserIntTask();  // we don't expect IOCTL to fail, but...
		if (status != STATUS_SUCCESSFUL)
		{
			StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::GlobalSCANstart(false), APItdCancelUserIntTask() failed, status 0x%x", status);
			CThordaq::LogMessage(logMsg, VERBOSE_EVENT);
		}
	}


	FPGAregisterWRITE("DACWavePerChannelRunStop", 0x0000);  // stop everything

	StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::DACStopWaveforms() called");

	_dacContinuousModeStartStopStatus = false;
	_dacContinuousModeEventPrepared = false; 

	_dacAbortContinousMode = true;

	_stoppingDAC = false;
	return status;
}

THORDAQ_STATUS CThordaq::DACRegisterApproachingNSamplesEvent(UINT8 dacChannel, UINT32 nSamples, UINT32 options, ThorDAQDACApproachingNSamplesCallbackPtr callbackFunction, void* callbackData)
{
	if (dacChannel >= WAVETABLE_CHANNEL_COUNT || dacChannel < 0 || callbackFunction == NULL)
	{
		return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;
	}
	DACApproachingNSamplesCallbackSettings callbackSettings = DACApproachingNSamplesCallbackSettings();
	callbackSettings.callbackFunction = callbackFunction;
	callbackSettings.numberOfSamples = nSamples;
	callbackSettings.options = options;
	callbackSettings.callbackData = callbackData;

	_dacApproachingNSamplesCallbackPtrs[dacChannel] = callbackSettings;

	// turn on the dac counter for the channel so we can count sample
	DACSetWaveGenInterruptsOnChannel(dacChannel, 1);

	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	return status;
}

THORDAQ_STATUS CThordaq::DACRegisterApproachingLoadedWaveformEndEvent(UINT8 dacChannel, UINT32 options, ThorDAQDACApproachingLoadedWaveformEndCallbackPtr callbackFunction, void* callbackData)
{
	if (dacChannel >= WAVETABLE_CHANNEL_COUNT || dacChannel < 0 || callbackFunction == NULL)
	{
		return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;
	}

	//need to build variable with record of descriptors per cycle for each channel in callback
	//also need one for NSamples and one to keep good track on the descriptor table
	DACApproachingLoadedWaveformEndSettings callbackSettings = DACApproachingLoadedWaveformEndSettings();
	callbackSettings.callbackFunction = callbackFunction;
	callbackSettings.options = options;
	callbackSettings.callbackData = callbackData;

	_dacApproachinLoadedWaveformEndCallbackPtrs[dacChannel] = callbackSettings;

	// turn on the dac counter for the channel so we can count sample
	DACSetWaveGenInterruptsOnChannel(dacChannel, 1);

	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	return status;
}

THORDAQ_STATUS CThordaq::DACRegisterCycleDoneEvent(UINT8 dacChannel, UINT32 options, ThorDAQDACCycleDoneCallbackPtr callbackFunction, void* callbackData)
{
	if (dacChannel >= WAVETABLE_CHANNEL_COUNT || dacChannel < 0 || callbackFunction == NULL)
	{
		return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;
	}

	//need to build variable with record of descriptors per cycle for each channel in callback
	//also need one for NSamples and one to keep good track on the descriptor table
	DACCycleDoneCallbackSettings callbackSettings = DACCycleDoneCallbackSettings();
	callbackSettings.callbackFunction = callbackFunction;
	callbackSettings.options = options;
	callbackSettings.callbackData = callbackData;

	_dacCycleDoneCallbackPtrs[dacChannel] = callbackSettings;

	// turn on the dac counter for the channel so we can count sample
	DACSetWaveGenInterruptsOnChannel(dacChannel, 1);

	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	return status;
}

THORDAQ_STATUS CThordaq::DACRegisterWaveformPlaybackCompleteEvent(UINT32 options, ThorDAQDACWaveformPlaybackCompleteCallbackPtr callbackFunction, void* callbackData)
{
	//need to build variable with record of descriptors per cycle for each channel in callback
	//also need one for NSamples and one to keep good track on the descriptor table
	_dacWaveformPlaybackCompletePtr.callbackFunction = callbackFunction;
	_dacWaveformPlaybackCompletePtr.options = options;
	_dacWaveformPlaybackCompletePtr.callbackData = callbackData;

	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	return status;
}

THORDAQ_STATUS CThordaq::DACRegisterWaveformPlaybackStartedEvent(UINT32 options, ThorDAQDACWaveformPlaybackStartedCallbackPtr callbackFunction, void* callbackData)
{
	//need to build variable with record of descriptors per cycle for each channel in callback
	//also need one for NSamples and one to keep good track on the descriptor table
	_dacWaveformPlaybackStartedCallbackPtr.callbackFunction = callbackFunction;
	_dacWaveformPlaybackStartedCallbackPtr.options = options;
	_dacWaveformPlaybackStartedCallbackPtr.callbackData = callbackData;
	_dacWaveformPlaybackStartedCallbackPtr.hasBeenCalled = false;
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	return status;
}

/// <summary>
/// Create a vector of waveform groups given the settings and waveform in the DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gctrol variable
/// it ensures there are at least 2 descriptors and sets the looping
/// </summary>
/// <param name="gctrol">original settings and waveform</param>
/// <param name="totalDescriptors">the number of descriptors in the groups is set here</param>
/// <returns>vector with groups DAC_WAVE_GROUP_STRUCT created</returns>
std::vector<CThordaq::DAC_WAVE_GROUP_STRUCT> CThordaq::DACCreateDACWaveformGroupsStaticLoadSetup(DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT* gctrol, UINT64 loopCount, UINT64& totalDescriptors)
{
	USHORT* waveform = gctrol->waveformBuffer;
	ULONG64 waveformSize = gctrol->waveform_buffer_size;
	ULONG64 waveformLength = gctrol->waveform_buffer_size >> 1;
	bool deleteWaveform = false;
	if (gctrol->waveform_buffer_size >> 1 < DAC_DESC_STATIC_LOAD_MIN_SAMPLES)
	{
		waveform = new USHORT[DAC_DESC_STATIC_LOAD_MIN_SAMPLES];
		waveformSize = DAC_DESC_STATIC_LOAD_MIN_SAMPLES * sizeof(USHORT);
		waveformLength = DAC_DESC_STATIC_LOAD_MIN_SAMPLES;
		deleteWaveform = true;
		std::copy(gctrol->waveformBuffer, gctrol->waveformBuffer + waveformLength, waveform);

		for (ULONG64 i = gctrol->waveform_buffer_size >> 1; i < DAC_DESC_STATIC_LOAD_MIN_SAMPLES; ++i)
		{
			waveform[i] = gctrol->waveformBuffer[(gctrol->waveform_buffer_size >> 1) - 1];
		}
	}

	std::vector<CThordaq::DAC_WAVE_GROUP_STRUCT> dacWaveformGroups;
	USHORT offset = gctrol->offset;
	size_t sampleOffset = 0;
	//We need at least 2 descriptors for looping to work
	UINT64 descCount = 0;
	bool hasInitWaveform = false;
	if (waveformSize <= DAC_TRANSMIT_BUFFER_MAX + DAC_DESC_STATIC_LOAD_MIN_SAMPLE_PER_DESC)
	{
		DAC_WAVE_GROUP_STRUCT wavestructinit = DAC_WAVE_GROUP_STRUCT();
		wavestructinit.waveformBuffer = new USHORT[DAC_DESC_STATIC_LOAD_MIN_SAMPLE_PER_DESC];
		wavestructinit.waveform_buffer_size = DAC_DESC_STATIC_LOAD_MIN_SAMPLE_PER_DESC * sizeof(USHORT);

		if (gctrol->offsetTheWaveforms)
		{
			for (UINT64 i = 0; i < DAC_DESC_STATIC_LOAD_MIN_SAMPLE_PER_DESC; ++i)
			{
				wavestructinit.waveformBuffer[i] = waveform[i] - offset;
			}
		}
		else
		{
			std::copy(waveform, waveform + DAC_DESC_STATIC_LOAD_MIN_SAMPLE_PER_DESC, wavestructinit.waveformBuffer);
		}

		wavestructinit.needsMemoryCleanup = true;
		wavestructinit.loopCount = 0;
		wavestructinit.loopEnable = false;
		wavestructinit.isLoopStart = loopCount > 1;
		wavestructinit.isLoopEnd = false;
		dacWaveformGroups.push_back(wavestructinit);

		sampleOffset = DAC_DESC_STATIC_LOAD_MIN_SAMPLE_PER_DESC;
		++descCount;
		hasInitWaveform = true;
	}

	// we can only set up to 16384 loops in the loop count
	//so for every extra loop we have to add one or more descriptors,
	//here we don't separate each group by descriptor, instead we group the whole
	//waveform and let the function that defines the descriptor table to separate the waveform into
	//each descriptor, considering the max length per descriptor
	//depending on the size of the waveform to be looped
	INT64 remaningLoops = static_cast<INT64>(loopCount);
	do
	{
		UINT64 endWaveformLength = waveformLength - sampleOffset;
		//we need to make sure we have the right number of samples to fill the minimum needed per register
		size_t extraSamples = (endWaveformLength) % DAC_DESC_SAMPLE_COUNT_RESOLUTION;
		extraSamples = extraSamples > 0 ? DAC_DESC_SAMPLE_COUNT_RESOLUTION - extraSamples : 0;

		size_t newEndWaveformLength = endWaveformLength + extraSamples;

		DAC_WAVE_GROUP_STRUCT wavestructEnd = DAC_WAVE_GROUP_STRUCT();

		size_t initOffset = sampleOffset;

		wavestructEnd.waveform_buffer_size = newEndWaveformLength * sizeof(USHORT);

		wavestructEnd.waveformBuffer = new USHORT[newEndWaveformLength];

		if (gctrol->offsetTheWaveforms)
		{
			for (UINT64 i = 0; i < endWaveformLength; ++i)
			{
				wavestructEnd.waveformBuffer[i] = waveform[i + initOffset] - offset;
			}
		}
		else
		{
			std::copy(waveform + initOffset, waveform + initOffset + endWaveformLength, wavestructEnd.waveformBuffer);
		}

		std::memset(wavestructEnd.waveformBuffer + newEndWaveformLength - extraSamples, wavestructEnd.waveformBuffer[newEndWaveformLength - extraSamples - 1], sizeof(USHORT) * extraSamples);

		USHORT loops = remaningLoops < DAC_DESC_MAX_LOOP_COUNT ? static_cast<USHORT>(remaningLoops) : DAC_DESC_MAX_LOOP_COUNT;

		//set memory cleanup and looping settings
		wavestructEnd.needsMemoryCleanup = true;
		wavestructEnd.loopCount = loops > 1 ? (USHORT)loops : 0;
		wavestructEnd.loopEnable = loops > 1;
		wavestructEnd.isLoopStart = hasInitWaveform && loops > 1 ? false : true;
		wavestructEnd.isLoopEnd = true;
		dacWaveformGroups.push_back(wavestructEnd);
		hasInitWaveform = false; // only the first loop group needs an init waveform

		//the last descriptor doesn't count
		descCount += (UINT64)ceil((double)wavestructEnd.waveform_buffer_size / (double)DAC_TRANSMIT_BUFFER_MAX);
		if (wavestructEnd.loopEnable)
		{
			++descCount; //add one more for the loop descriptor
		}
		remaningLoops -= loops;
	} while (remaningLoops > 0);

	if (deleteWaveform)
	{
		SAFE_DELETE_ARRAY(waveform);
	}
	totalDescriptors = (descCount) * (loopCount);

	return dacWaveformGroups;
}

/// <summary>
/// Create a vector of waveform groups given the settings and waveform in the DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gctrol variable
/// it sets at least 2 descriptors, the last part of the waveform and one decriptor that repeats the last value
/// </summary>
/// <param name="gctrol">original settings and waveform</param>
/// <param name="totalDescriptors">the number of descriptors in the groups is set here</param>
/// <returns>vector with groups DAC_WAVE_GROUP_STRUCT created</returns>
std::vector<CThordaq::DAC_WAVE_GROUP_STRUCT> CThordaq::DACCreateDACWaveformGroupsLastSectionStaticLoadSetup(DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT* gctrol, UINT64& totalDescriptors)
{
	USHORT* waveform = gctrol->waveformBuffer;
	ULONG64 waveformSize = gctrol->waveform_buffer_size;
	ULONG64 waveformLength = gctrol->waveform_buffer_size >> 1;
	bool deleteWaveform = false;
	if (gctrol->waveform_buffer_size >> 1 < DAC_DESC_STATIC_LOAD_MIN_SAMPLES)
	{
		waveform = new USHORT[DAC_DESC_STATIC_LOAD_MIN_SAMPLES];
		waveformSize = DAC_DESC_STATIC_LOAD_MIN_SAMPLES * sizeof(USHORT);
		waveformLength = DAC_DESC_STATIC_LOAD_MIN_SAMPLES;
		std::copy(gctrol->waveformBuffer, gctrol->waveformBuffer + waveformLength, waveform);
		deleteWaveform = true;
		for (ULONG64 i = gctrol->waveform_buffer_size >> 1; i < DAC_DESC_STATIC_LOAD_MIN_SAMPLES; ++i)
		{
			waveform[i] = gctrol->waveformBuffer[(gctrol->waveform_buffer_size >> 1) - 1];
		}
	}

	std::vector<CThordaq::DAC_WAVE_GROUP_STRUCT> dacWaveformGroups;


	//we need to make sure we have the right number of samples to fill the minimum needed per register
	//int extraSamples = (initWaveformLength) % DAC_DESC_SAMPLE_COUNT_RESOLUTION;
	//extraSamples = extraSamples > 0 ? DAC_DESC_SAMPLE_COUNT_RESOLUTION - extraSamples : 0;

	DAC_WAVE_GROUP_STRUCT wavestructinit = DAC_WAVE_GROUP_STRUCT();


	//we need to make sure we have the right number of samples to fill the minimum needed per register
	size_t extraSamples = (waveformLength) % DAC_DESC_SAMPLE_COUNT_RESOLUTION;
	extraSamples = extraSamples > 0 ? DAC_DESC_SAMPLE_COUNT_RESOLUTION - extraSamples : 0;

	size_t newInitWaveformLength = waveformLength + extraSamples;

	wavestructinit.waveformBuffer = new USHORT[newInitWaveformLength];
	wavestructinit.waveform_buffer_size = newInitWaveformLength * sizeof(USHORT);
	//we need to offset the data so that the first point starts at the offset

	if (gctrol->offsetTheWaveforms)
	{
		USHORT offset = gctrol->offset;
		for (UINT64 i = 0; i < waveformLength; ++i)
		{
			wavestructinit.waveformBuffer[i] = waveform[i] - offset;
		}
	}
	else
	{
		std::copy(gctrol->waveformBuffer, gctrol->waveformBuffer + waveformLength, wavestructinit.waveformBuffer);
	}

	std::memset(wavestructinit.waveformBuffer + newInitWaveformLength - extraSamples, wavestructinit.waveformBuffer[newInitWaveformLength - extraSamples - 1], sizeof(USHORT) * extraSamples);

	//set memory cleanup and looping settings
	wavestructinit.needsMemoryCleanup = true;
	wavestructinit.loopCount = 0;
	wavestructinit.loopEnable = false;
	wavestructinit.isLoopStart = false;
	wavestructinit.isLoopEnd = false;

	dacWaveformGroups.push_back(wavestructinit);

	DAC_WAVE_GROUP_STRUCT wavestructEnd = DAC_WAVE_GROUP_STRUCT();

	wavestructEnd.waveform_buffer_size = DAC_DESC_STATIC_LOAD_MIN_SAMPLE_FOR_LAST_DESC * sizeof(USHORT);

	wavestructEnd.waveformBuffer = new USHORT[DAC_DESC_STATIC_LOAD_MIN_SAMPLE_FOR_LAST_DESC];

	for (UINT64 i = 0; i < DAC_DESC_STATIC_LOAD_MIN_SAMPLE_FOR_LAST_DESC; ++i)
	{
		wavestructEnd.waveformBuffer[i] = wavestructinit.waveformBuffer[newInitWaveformLength - 1];
	}

	std::memset(wavestructEnd.waveformBuffer, wavestructinit.waveformBuffer[newInitWaveformLength - 1], sizeof(USHORT) * DAC_DESC_STATIC_LOAD_MIN_SAMPLE_FOR_LAST_DESC);

	//set memory cleanup and looping settings
	wavestructEnd.needsMemoryCleanup = true;
	wavestructEnd.loopCount = 0;
	wavestructEnd.loopEnable = false;
	wavestructEnd.isLoopStart = false;
	wavestructEnd.isLoopEnd = false;

	dacWaveformGroups.push_back(wavestructEnd);
	if (deleteWaveform)
	{
		SAFE_DELETE_ARRAY(waveform);
	}
	totalDescriptors = (UINT64)ceil((double)wavestructinit.waveform_buffer_size / (double)DAC_TRANSMIT_BUFFER_MAX) + DAC_STATIC_EXTRA_DESC_TO_WAIT;
	return dacWaveformGroups;
}


/// <summary>
/// Create a vector of waveform groups given the settings and waveform in the DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gctrol variable
/// each group in the returned vector needs only one dac descp and is within DAC_TRANSMIT_BUFFER_MAX in size
/// </summary>
/// <param name="gctrol">original settings and waveform</param>
/// <param name="totalDescriptors">the number of descriptors in the groups is set here</param>
/// <returns>vector with groups DAC_WAVE_GROUP_STRUCT created</returns>
inline THORDAQ_STATUS CThordaq::DACCreateDACWaveformGroupsLongWaveformForDynamicLoadSetup(DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT* gctrol, UINT64& totalDescriptors, std::vector<DAC_WAVE_GROUP_STRUCT>& dacWaveformGroups)
{
	USHORT offset = gctrol->offset;
	INT64 LASTSIZE = DAC_TRANSMIT_BUFFER_MAX / 2;
	INT64 waveformSize = gctrol->waveform_buffer_size - LASTSIZE;
	size_t sampleOffset = 0;

	UINT64 minSamples = CalculateMinSamplesStart(gctrol->update_rate);
	int i = 0;
	while (waveformSize > 0)
	{
		size_t nextSize = 0;
		if (waveformSize >= LASTSIZE * 2)
		{
			nextSize = LASTSIZE;
		}
		else if (waveformSize >= (INT64)(LASTSIZE + minSamples))
		{
			nextSize = LASTSIZE;
		}
		else if (waveformSize >= LASTSIZE)
		{
			nextSize = minSamples;
		}
		else
		{
			nextSize = waveformSize;
		}

		//we need to make sure we have the right number of samples to fill the minimum needed per register
		int extraSamples = (nextSize / sizeof(USHORT)) % DAC_DESC_SAMPLE_COUNT_RESOLUTION;
		extraSamples = extraSamples > 0 ? DAC_DESC_SAMPLE_COUNT_RESOLUTION - extraSamples : 0;
		size_t newInitWaveformLength = nextSize / sizeof(USHORT) + extraSamples;

		size_t size = newInitWaveformLength * sizeof(USHORT);
		DAC_WAVE_GROUP_STRUCT wavestructinit = DAC_WAVE_GROUP_STRUCT();

		if (dacWaveformGroups.size() > i)
		{
			wavestructinit = dacWaveformGroups[i];

			if (wavestructinit.waveformBuffer == NULL || wavestructinit.waveform_buffer_size != size)
			{
				SAFE_DELETE_ARRAY(wavestructinit.waveformBuffer);
				wavestructinit.waveformBuffer = new USHORT[newInitWaveformLength];
				wavestructinit.waveform_buffer_size = size;
			}
		}
		else
		{
			wavestructinit.waveformBuffer = new USHORT[newInitWaveformLength];
			wavestructinit.waveform_buffer_size = size;
			dacWaveformGroups.push_back(wavestructinit);
		}

		wavestructinit.needsMemoryCleanup = true;
		wavestructinit.loopCount = 0;
		wavestructinit.loopEnable = false;
		wavestructinit.isLoopStart = false;
		wavestructinit.isLoopEnd = false;

		//we need to offset the data so that the first point starts at the offset
		if (gctrol->offsetTheWaveforms)
		{
			for (UINT64 i = 0; i < newInitWaveformLength; ++i)
			{
				wavestructinit.waveformBuffer[i] = gctrol->waveformBuffer[sampleOffset + i] - offset;
			}
		}
		else
		{
			std::copy(gctrol->waveformBuffer + sampleOffset, gctrol->waveformBuffer + sampleOffset + newInitWaveformLength, wavestructinit.waveformBuffer);
		}
		sampleOffset += newInitWaveformLength;

		dacWaveformGroups[i] = wavestructinit;
		++i;

		waveformSize -= newInitWaveformLength * sizeof(USHORT);
	}

	DAC_WAVE_GROUP_STRUCT wavestructEnd = DAC_WAVE_GROUP_STRUCT();

	size_t endWaveformSize = gctrol->waveform_buffer_size - sampleOffset * sizeof(USHORT);// DAC_TRANSMIT_BUFFER_MAX;
	size_t endWaveformLength = endWaveformSize / sizeof(USHORT);

	//we need to make sure we have the right number of samples to fill the minimum needed per register
	size_t extraSamples = (endWaveformLength) % DAC_DESC_SAMPLE_COUNT_RESOLUTION;
	extraSamples = extraSamples > 0 ? DAC_DESC_SAMPLE_COUNT_RESOLUTION - extraSamples : 0;

	size_t newEndWaveformLength = endWaveformLength + extraSamples;

	size_t initOffset = sampleOffset;

	size_t endSize = newEndWaveformLength * sizeof(USHORT);
	if (dacWaveformGroups.size() > i)
	{
		wavestructEnd = dacWaveformGroups[i];

		if (wavestructEnd.waveformBuffer == NULL || wavestructEnd.waveform_buffer_size != endSize)
		{
			SAFE_DELETE_ARRAY(wavestructEnd.waveformBuffer);
			wavestructEnd.waveformBuffer = new USHORT[newEndWaveformLength];
			wavestructEnd.waveform_buffer_size = endSize;
		}
	}
	else
	{
		wavestructEnd.waveformBuffer = new USHORT[newEndWaveformLength];
		wavestructEnd.waveform_buffer_size = endSize;
		dacWaveformGroups.push_back(wavestructEnd);
	}

	if (gctrol->offsetTheWaveforms)
	{
		for (UINT64 i = 0; i < endWaveformLength; ++i)
		{
			wavestructEnd.waveformBuffer[i] = gctrol->waveformBuffer[i + initOffset] - offset;
		}
	}
	else
	{
		std::copy(gctrol->waveformBuffer + initOffset, gctrol->waveformBuffer + initOffset + endWaveformLength, wavestructEnd.waveformBuffer);
	}

	std::memset(wavestructEnd.waveformBuffer + newEndWaveformLength - extraSamples, wavestructEnd.waveformBuffer[newEndWaveformLength - extraSamples - 1], sizeof(USHORT) * extraSamples);

	//set memory cleanup and looping settings
	wavestructEnd.needsMemoryCleanup = true;
	wavestructEnd.loopCount = 0;
	wavestructEnd.loopEnable = false;
	wavestructEnd.isLoopStart = false;
	wavestructEnd.isLoopEnd = false;

	dacWaveformGroups[i] = wavestructEnd;

	totalDescriptors = dacWaveformGroups.size();

	return THORDAQ_STATUS::STATUS_SUCCESSFUL;
}

/// <summary>
/// Create a vector of waveform groups given the settings and waveform in the DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gctrol variable
/// each group in the returned vector needs only one dac descp and is within DAC_TRANSMIT_BUFFER_MAX in size
/// </summary>
/// <param name="gctrol">original settings and waveform</param>
/// <param name="totalDescriptors">the number of descriptors in the groups is set here</param>
/// <returns>vector with groups DAC_WAVE_GROUP_STRUCT created</returns>
inline THORDAQ_STATUS CThordaq::DACCreateDACWaveformGroupsShortWaveformForDynamicLoadSetup(DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT* gctrol, UINT64& totalDescriptors, std::vector<DAC_WAVE_GROUP_STRUCT>& dacWaveformGroups)
{
	UINT64 minSamples = CalculateMinSamplesEnd(gctrol->update_rate);
	size_t totalWaveformLength = gctrol->waveform_buffer_size / sizeof(USHORT);

	size_t initWaveformLength = totalWaveformLength / 4;
	USHORT offset = gctrol->offset;
	size_t dataOffset = 0;
	const int groups = 3;
	int i = 0;
	for (int k = 0; k < groups; ++k)
	{
		//we need to make sure we have the right number of samples to fill the minimum needed per register
		//int extraSamples = (initWaveformLength) % DAC_DESC_SAMPLE_COUNT_RESOLUTION;
		//extraSamples = extraSamples > 0 ? DAC_DESC_SAMPLE_COUNT_RESOLUTION - extraSamples : 0;

		size_t size = initWaveformLength * sizeof(USHORT);
		DAC_WAVE_GROUP_STRUCT wavestructinit;

		if (dacWaveformGroups.size() > i)
		{
			wavestructinit = dacWaveformGroups[i];

			if (wavestructinit.waveformBuffer == NULL || wavestructinit.waveform_buffer_size != size)
			{
				SAFE_DELETE_ARRAY(wavestructinit.waveformBuffer);
				wavestructinit.waveformBuffer = new USHORT[initWaveformLength];
				wavestructinit.waveform_buffer_size = size;
			}
		}
		else
		{
			wavestructinit.waveformBuffer = new USHORT[initWaveformLength];
			wavestructinit.waveform_buffer_size = size;
			dacWaveformGroups.push_back(wavestructinit);
		}

		//we need to offset the data so that the first point starts at the offset

		if (gctrol->offsetTheWaveforms)
		{
			for (UINT64 i = 0; i < initWaveformLength; ++i)
			{
				wavestructinit.waveformBuffer[i] = gctrol->waveformBuffer[i + dataOffset] - offset;
			}
		}
		else
		{
			std::copy(gctrol->waveformBuffer + dataOffset, gctrol->waveformBuffer + dataOffset + initWaveformLength, wavestructinit.waveformBuffer);
		}

		dataOffset += initWaveformLength;
		//set memory cleanup and looping settings
		wavestructinit.needsMemoryCleanup = true;
		wavestructinit.loopCount = 0;
		wavestructinit.loopEnable = false;
		wavestructinit.isLoopStart = false;
		wavestructinit.isLoopEnd = false;
		dacWaveformGroups[i] = wavestructinit;
		++i;
	}


	DAC_WAVE_GROUP_STRUCT wavestructEnd;
	size_t waveform3Size = gctrol->waveform_buffer_size - dataOffset * sizeof(USHORT);
	size_t waveform3Length = waveform3Size / sizeof(USHORT);

	//we need to make sure we have the right number of samples to fill the minimum needed per register
	size_t extraSamples = (waveform3Length) % DAC_DESC_SAMPLE_COUNT_RESOLUTION;
	extraSamples = extraSamples > 0 ? DAC_DESC_SAMPLE_COUNT_RESOLUTION - extraSamples : 0;

	size_t newWaveform3Length = waveform3Length + extraSamples;

	size_t endSize = newWaveform3Length * sizeof(USHORT);

	if (dacWaveformGroups.size() > i)
	{
		wavestructEnd = dacWaveformGroups[i];

		if (wavestructEnd.waveformBuffer == NULL || wavestructEnd.waveform_buffer_size != endSize)
		{
			SAFE_DELETE_ARRAY(wavestructEnd.waveformBuffer);
			wavestructEnd.waveformBuffer = new USHORT[newWaveform3Length];
			wavestructEnd.waveform_buffer_size = endSize;
		}
	}
	else
	{
		wavestructEnd.waveformBuffer = new USHORT[newWaveform3Length];
		wavestructEnd.waveform_buffer_size = endSize;
		dacWaveformGroups.push_back(wavestructEnd);
	}


	if (gctrol->offsetTheWaveforms)
	{
		for (UINT64 i = 0; i < waveform3Length; ++i)
		{
			wavestructEnd.waveformBuffer[i] = gctrol->waveformBuffer[i + dataOffset] - offset;
		}
	}
	else
	{
		std::copy(gctrol->waveformBuffer + dataOffset, gctrol->waveformBuffer + dataOffset + waveform3Length, wavestructEnd.waveformBuffer);
	}
	std::memset(wavestructEnd.waveformBuffer + newWaveform3Length - extraSamples, wavestructEnd.waveformBuffer[newWaveform3Length - extraSamples - 1], sizeof(USHORT) * extraSamples);

	//set memory cleanup and looping settings
	wavestructEnd.needsMemoryCleanup = true;
	wavestructEnd.loopCount = 0;
	wavestructEnd.loopEnable = false;
	wavestructEnd.isLoopStart = false;
	wavestructEnd.isLoopEnd = false;

	dacWaveformGroups[i] = wavestructEnd;

	totalDescriptors = dacWaveformGroups.size();
	return THORDAQ_STATUS::STATUS_SUCCESSFUL;
}

/// <summary>
/// Load the DAC waveformBuffer descriptors into the board.
/// Currently using th two bank system, but both banks are the same
/// TODO: switch between 2 bank system and 1 bank system dynamically, depending on waveformBuffer length.
/// Single bank has double the size of each bank in the two bank system. Two bank system allows dynamic
/// data loading.
/// </summary>
/// <param name="dac_settings"></param>
/// <returns></returns>
LONG CThordaq::DACLoadWaveformAndDescriptors(std::map<UINT, DAC_WAVE_DESC_STRUCT> dac_settings, ThorDAQExperimentType experimentType, bool preDynamicLoad)
{
	THORDAQ_STATUS status;
	STAT_STRUCT DoMemStatus;
	std::map<UINT, DAC_WAVE_DESC_STRUCT> waveformSettings = dac_settings;
	bool isFreeRunMode = ((int)ThorDAQExperimentType::DACFreeRunMode & (int)experimentType) == 1;

	const ULONG64 DESC_OFFSET = isFreeRunMode ? DAC_DESCP_LENGTH / (ULONG64)16 : 0;
	UINT64 DDR3Offset = isFreeRunMode ? DAC_TRANSMIT_BUFFER_MAX * DESC_OFFSET : 0;
	UINT64 bufferOffset = DDR3Offset;

	const UINT64 baseAddress = 0x50000000;
	_dacDescriptorMutex.lock();
	if (isFreeRunMode)
	{
		_dacWaveformFreeRunModeChannels.clear();
	}
	else
	{
		_dacWaveformImagingChannels.clear();
	}

	for (UINT i = 0; i < WAVETABLE_CHANNEL_COUNT; i++)
	{
		ULONG64 size = 0;

		if (dac_settings.find(i) != dac_settings.end())
		{
			for (auto& waveGRoup : waveformSettings[i].dacWaveformGroups)
			{
				if (NULL == waveGRoup.waveformBuffer)
				{
					waveGRoup.waveform_buffer_size = 0;
				}
				else
				{
					ULONG64 startAddress = baseAddress + bufferOffset;

					const ULONG64 MAX_WAVEFORM_COPY_SIZE = 16 * 1024 * 1024; //16MB

					//There seems to be corruption of the waveform when copying larger waveforms.
					//Experimentaly proved that limiting the copy size to 16MBs there is no longer a corruption of the waveform					
					if (waveGRoup.waveform_buffer_size > MAX_WAVEFORM_COPY_SIZE)
					{
						ULONG64 sizeLeft = waveGRoup.waveform_buffer_size;
						ULONG64 waveformOffset = 0;
						while (sizeLeft > MAX_WAVEFORM_COPY_SIZE)
						{
							status = WriteDDR3(((UCHAR*)waveGRoup.waveformBuffer) + waveformOffset, startAddress + waveformOffset, static_cast<UINT32>(MAX_WAVEFORM_COPY_SIZE));
							sizeLeft -= MAX_WAVEFORM_COPY_SIZE;
							waveformOffset += MAX_WAVEFORM_COPY_SIZE;
						}
						if (sizeLeft > 0)
						{
							status = WriteDDR3(((UCHAR*)waveGRoup.waveformBuffer) + waveformOffset, startAddress + waveformOffset, static_cast<UINT32>(sizeLeft));
						}
					}
					else
					{
						status = WriteDDR3((UCHAR*)waveGRoup.waveformBuffer, startAddress, static_cast<UINT32>(waveGRoup.waveform_buffer_size));
					}

					waveGRoup.waveform_buffer_start_address = startAddress;
					bufferOffset += waveGRoup.waveform_buffer_size;

					if (isFreeRunMode)
					{
						_dacWaveformFreeRunModeChannels[i] = true;

						//the channels need to be mutually exclusive
						if (_dacWaveformImagingChannels.find(i) != _dacWaveformImagingChannels.end())
						{
							_dacWaveformImagingChannels.erase(i);
						}
					}
					else
					{
						_dacWaveformImagingChannels[i] = true;

						//the channels need to be mutually exclusive
						if (_dacWaveformFreeRunModeChannels.find(i) != _dacWaveformFreeRunModeChannels.end())
						{
							_dacWaveformFreeRunModeChannels.erase(i);
						}
					}
				}
			}
		}
	}

	DMADescriptors* despTable = new DMADescriptors[WAVETABLE_CHANNEL_COUNT]();

	if (isFreeRunMode)
	{
		std::memset(_dacDescriptorsPerChannel, 0, sizeof(_dacDescriptorsPerChannel));

		_dacDescTableTracker.clear();

		_dacDescTableSamplesTracker.clear();
		_dacDescpListIndex = DESC_OFFSET;
	}
	else
	{
		_dacDescpListIndex = WAVETABLE_CHANNEL_COUNT - 1;
	}

	for (UINT8 j = 0; j < WAVETABLE_CHANNEL_COUNT; j++)
	{
		DAC_WAVE_DESC_STRUCT dac_setting = DAC_WAVE_DESC_STRUCT();
		if (waveformSettings.find(j) != waveformSettings.end())
		{
			dac_setting = waveformSettings[j];
			if (dac_setting.dacWaveformGroups.size() == 0)
			{
				//continue;
			}
			UINT64 k = 0;
			for (auto& waveGRoup : dac_setting.dacWaveformGroups)
			{
				DMADescriptor desc = DMADescriptor();
				desc.buf_addr = waveGRoup.waveform_buffer_start_address;
				desc.length = waveGRoup.waveform_buffer_size;
				desc.nextDesc = k + j;

				desc.loopEnable = waveGRoup.loopEnable;
				desc.loopCount = waveGRoup.loopCount > 0 ? waveGRoup.loopCount - 1 : 0;
				desc.loopStartDesc = k + j;
				desc.isLoopEnd = waveGRoup.isLoopEnd;
				desc.isLoopStart = waveGRoup.isLoopStart;
				despTable[j].desc.push_back(desc);
			}
		}
		if ((_dacWaveformImagingChannels.find(j) == _dacWaveformImagingChannels.end() && isFreeRunMode) || (_dacWaveformFreeRunModeChannels.find(j) == _dacWaveformFreeRunModeChannels.end() && !isFreeRunMode))
		{
			despTable[j].finishPlaybackInLastGroup = dac_setting.finishPlaybackInLastGroup;
			if (FALSE == DACSetDMADescriptors(despTable[j], j, _dacDMADescTable->descps, DAC_DESCP_LENGTH, preDynamicLoad))
			{
				_dacDescriptorMutex.unlock();
				SAFE_DELETE_ARRAY(despTable);
				return FALSE;
			}
		}
	}
	const UINT64 DAC_DESC_TABLE_ADDRESS = 0x50000;
	status = DoMem(WRITE_TO_CARD, BAR3, (PUINT8)(&(_dacDMADescTable->descps)), 0, DAC_DESC_TABLE_ADDRESS, (_dacDescpListIndex + 1) * sizeof(ULONG64), &DoMemStatus);

	SAFE_DELETE_ARRAY(despTable);
	_dacDescriptorMutex.unlock();
	return status == THORDAQ_STATUS::STATUS_SUCCESSFUL;
}

LONG CThordaq::DACSetDMADescriptors(DMADescriptors& dmaDescpGroup, ULONG64 chanIndex, ULONG64* dmaDescpTable, ULONG64 descpTableLength, bool preDynamicLoad)
{
	if (WAVETABLE_CHANNEL_COUNT <= chanIndex)
	{
		return FALSE;
	}

	ULONG64 decpIndex = chanIndex;
	ULONG64 loopFirstIndex = chanIndex;
	ULONG64 loopStart = 0;
	DAC_DESC_TABLE_SETUP_TRACKER dacChandescTableTracker = DAC_DESC_TABLE_SETUP_TRACKER();

	if (dmaDescpGroup.desc.size() == 0 || dmaDescpGroup.desc[0].length == 0)
	{
		ULONG32 BTT = (ULONG32)(DAC_TRANSMIT_BUFFER_MAX) >> 2; // divide by 4, same as bit shifting to the right 2 bits
		dmaDescpTable[decpIndex] = decpIndex << DAC_NXT_DESC_OFFSET | (UINT64)BTT << DAC_TOTAL_BYTES_TO_SEND_OFFSET | (UINT64)0 << DAC_LOOP_OFFSET | 0x50000000;
		return TRUE;
	}
	//_dacWaveformDescriptorSetForChannels[(UINT)chanIndex] = true;

	dacChandescTableTracker.totalSamples = dmaDescpGroup.desc[0].length / sizeof(USHORT);
	dacChandescTableTracker.firstDescIndex = chanIndex;
	dacChandescTableTracker.secondDescIndex = chanIndex;//_dacDescpListIndex + 1;
	std::vector<DAC_DESC_TABLE_SAMPLES_TRACKER> samplesTrackerVector;
	for (size_t i = 0; i < dmaDescpGroup.desc.size(); ++i)
	{
		auto& dmaDescp = dmaDescpGroup.desc[i];

		if (dmaDescp.isLoopStart)
		{
			loopStart = decpIndex;
		}
		while (dmaDescp.length > 0)
		{
			++_dacDescriptorsPerChannel[chanIndex];
			//set the second desc index to the second descriptor index (the first index after all the WAVETABLE_CHANNEL_COUNT have already set the first one)
			if (dacChandescTableTracker.secondDescIndex == chanIndex && decpIndex >= WAVETABLE_CHANNEL_COUNT)
			{
				dacChandescTableTracker.secondDescIndex = decpIndex;
			}
			if (dmaDescp.length <= DAC_TRANSMIT_BUFFER_MAX)
			{
				ULONG32 BTT = (ULONG32)(dmaDescp.length) >> 2; // divide by 4, same as bit shifting to the right 2 bits
				if ((i + 1) < dmaDescpGroup.desc.size())
				{
					if (dmaDescp.isLoopEnd && dmaDescp.loopEnable && dmaDescp.loopCount > 0)
					{
						dmaDescp.nextDesc = _dacDescpListIndex + 2;

						dmaDescpTable[decpIndex] = dmaDescp.nextDesc << DAC_NXT_DESC_OFFSET | (UINT64)BTT << DAC_TOTAL_BYTES_TO_SEND_OFFSET | (UINT64)dmaDescp.loopEnable << DAC_LOOP_OFFSET | dmaDescp.buf_addr;

						DAC_DESC_TABLE_SAMPLES_TRACKER samplesTracker = DAC_DESC_TABLE_SAMPLES_TRACKER();
						samplesTracker.channelIndex = (UINT8)chanIndex;
						samplesTracker.descIndex = decpIndex;
						samplesTracker.nxtDexc = dmaDescp.nextDesc;
						samplesTracker.size = dmaDescp.length;
						samplesTracker.address = dmaDescp.buf_addr;
						samplesTrackerVector.push_back(samplesTracker);

						//TODO: loop start desc should be set to the first desc of the loop group
						dmaDescp.loopStartDesc = loopStart;
						++_dacDescpListIndex;
						decpIndex = _dacDescpListIndex;

						dmaDescpTable[decpIndex] = dmaDescp.loopStartDesc << DACLOOP_LOOP_START_DESC_OFFSET | (UINT64)dmaDescp.loopCount << DACLOOP_TOTAL_LOOP_COUNT_OFFSET;
						++_dacDescriptorsPerChannel[chanIndex];

						++_dacDescpListIndex;
						decpIndex = _dacDescpListIndex;
						//++_dacDescpListIndex;
					}
					else
					{
						++_dacDescpListIndex;
						dmaDescp.nextDesc = _dacDescpListIndex;

						dmaDescpTable[decpIndex] = dmaDescp.nextDesc << DAC_NXT_DESC_OFFSET | (UINT64)BTT << DAC_TOTAL_BYTES_TO_SEND_OFFSET | (UINT64)false << DAC_LOOP_OFFSET | dmaDescp.buf_addr;

						DAC_DESC_TABLE_SAMPLES_TRACKER samplesTracker = DAC_DESC_TABLE_SAMPLES_TRACKER();
						samplesTracker.channelIndex = (UINT8)chanIndex;
						samplesTracker.descIndex = decpIndex;
						samplesTracker.nxtDexc = dmaDescp.nextDesc;
						samplesTracker.size = dmaDescp.length;
						samplesTracker.address = dmaDescp.buf_addr;
						samplesTrackerVector.push_back(samplesTracker);

						decpIndex = _dacDescpListIndex;
					}
					break;
				}
				else
				{
					if (dmaDescpGroup.finishPlaybackInLastGroup)
					{
						dmaDescp.nextDesc = decpIndex;
					}
					else
					{
						dmaDescp.nextDesc = chanIndex;
					}

					if (dmaDescp.isLoopEnd && dmaDescp.loopEnable && dmaDescp.loopCount > 0)
					{
						dmaDescpTable[decpIndex] = dmaDescp.nextDesc << DAC_NXT_DESC_OFFSET | (UINT64)BTT << DAC_TOTAL_BYTES_TO_SEND_OFFSET | (UINT64)dmaDescp.loopEnable << DAC_LOOP_OFFSET | dmaDescp.buf_addr;

						DAC_DESC_TABLE_SAMPLES_TRACKER samplesTracker = DAC_DESC_TABLE_SAMPLES_TRACKER();
						samplesTracker.channelIndex = (UINT8)chanIndex;
						samplesTracker.descIndex = decpIndex;
						samplesTracker.nxtDexc = dmaDescp.nextDesc;
						samplesTracker.size = dmaDescp.length;
						samplesTracker.address = dmaDescp.buf_addr;
						samplesTrackerVector.push_back(samplesTracker);


						dmaDescp.loopStartDesc = loopStart;
						++_dacDescpListIndex;
						decpIndex = _dacDescpListIndex;

						dmaDescpTable[decpIndex] = dmaDescp.loopStartDesc << DACLOOP_LOOP_START_DESC_OFFSET | (UINT64)dmaDescp.loopCount << DACLOOP_TOTAL_LOOP_COUNT_OFFSET;
						++_dacDescriptorsPerChannel[chanIndex];
					}
					else
					{
						dmaDescpTable[decpIndex] = dmaDescp.nextDesc << DAC_NXT_DESC_OFFSET | (UINT64)BTT << DAC_TOTAL_BYTES_TO_SEND_OFFSET | (UINT64)dmaDescp.loopEnable << DAC_LOOP_OFFSET | dmaDescp.buf_addr;

						DAC_DESC_TABLE_SAMPLES_TRACKER samplesTracker = DAC_DESC_TABLE_SAMPLES_TRACKER();
						samplesTracker.channelIndex = (UINT8)chanIndex;
						samplesTracker.descIndex = decpIndex;
						samplesTracker.nxtDexc = dmaDescp.nextDesc;
						samplesTracker.size = dmaDescp.length;
						samplesTracker.address = dmaDescp.buf_addr;
						samplesTrackerVector.push_back(samplesTracker);
					}

					//if dynamic load dac motion type then live space in between channels for the last descriptor that will be added at the end,
					//calling it self so the line stays there
					if (preDynamicLoad)
					{
						++_dacDescpListIndex;
					}

					//Need space for one last buffer with 2 samples for the last descriptor that will keep calling itself as the nxt descriptor
					dacChandescTableTracker.lastDescIndex = decpIndex;

					_dacDescTableTracker[(UINT8)chanIndex] = dacChandescTableTracker;

					_dacDescTableSamplesTracker[(UINT)chanIndex] = samplesTrackerVector;

					return TRUE;
				}
			}
			else
			{
				if (_dacDescpListIndex > descpTableLength - 2) // reach the end of the descriptor table
				{
					return FALSE;
				}

				++_dacDescpListIndex;
				// Because waveformBuffer data includes 1024 words at the end to leave the enough space to flush the FIFO. So must leave 1024 words space at the end of last descriptor. 
				// If it doesn't, put some data (choose 2048 samples here) to the next descriptor.
				if ((dmaDescp.length - DAC_TRANSMIT_BUFFER_MAX) <= (DAC_FIFO_DEPTH / 2))
				{
					ULONG32 BTT = (ULONG32)(DAC_TRANSMIT_BUFFER_MAX - DAC_FIFO_DEPTH) >> 2; // divide by 4, same as bit shifting to the right 2 bits

					dmaDescpTable[decpIndex] = _dacDescpListIndex << DAC_NXT_DESC_OFFSET | (ULONG64)BTT << DAC_TOTAL_BYTES_TO_SEND_OFFSET | dmaDescp.buf_addr;

					dmaDescp.length = dmaDescp.length - (ULONG64)(DAC_TRANSMIT_BUFFER_MAX - DAC_FIFO_DEPTH);
					dmaDescp.buf_addr = dmaDescp.buf_addr + (ULONG64)(DAC_TRANSMIT_BUFFER_MAX - DAC_FIFO_DEPTH);

					DAC_DESC_TABLE_SAMPLES_TRACKER samplesTracker = DAC_DESC_TABLE_SAMPLES_TRACKER();
					samplesTracker.channelIndex = (UINT8)chanIndex;
					samplesTracker.descIndex = decpIndex;
					samplesTracker.nxtDexc = _dacDescpListIndex;
					samplesTracker.size = (DAC_TRANSMIT_BUFFER_MAX - DAC_FIFO_DEPTH);
					samplesTracker.address = dmaDescp.buf_addr;
					samplesTrackerVector.push_back(samplesTracker);
				}
				else
				{
					ULONG32 BTT = (ULONG32)(DAC_TRANSMIT_BUFFER_MAX) >> 2; // divide by 4, same as bit shifting to the right 2 bits

					dmaDescpTable[decpIndex] = _dacDescpListIndex << DAC_NXT_DESC_OFFSET | (ULONG64)BTT << DAC_TOTAL_BYTES_TO_SEND_OFFSET | dmaDescp.buf_addr;

					dmaDescp.length = dmaDescp.length - (ULONG64)DAC_TRANSMIT_BUFFER_MAX;
					dmaDescp.buf_addr = dmaDescp.buf_addr + (ULONG64)DAC_TRANSMIT_BUFFER_MAX;

					DAC_DESC_TABLE_SAMPLES_TRACKER samplesTracker = DAC_DESC_TABLE_SAMPLES_TRACKER();
					samplesTracker.channelIndex = (UINT8)chanIndex;
					samplesTracker.descIndex = decpIndex;
					samplesTracker.nxtDexc = _dacDescpListIndex;
					samplesTracker.size = DAC_TRANSMIT_BUFFER_MAX;
					samplesTracker.address = dmaDescp.buf_addr;
					samplesTrackerVector.push_back(samplesTracker);
				}

				decpIndex = _dacDescpListIndex;
			}
		}
	}
	return TRUE;
}

THORDAQ_STATUS CThordaq::DACSetChannelsSettings(std::map<UINT, DAC_CRTL_STRUCT> dacCtrl)
{
	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;

	bool setDacSettingsForUnusedChannels = !_imagingEventPrepared && !_imagingStartStopStatus;


	for (UINT i = 0; i < WAVETABLE_CHANNEL_COUNT; i++)
	{
		if (dacCtrl.find(i) == dacCtrl.end())
		{
			DAC_CRTL_STRUCT dacCtrlStruct = DAC_CRTL_STRUCT();
			dacCtrlStruct.park_val = 0;
			dacCtrlStruct.update_rate = DAC_DISABLED_CHANNEL_UPDATERATE_CONTINUOUS_MODE;
			dacCtrlStruct.output_port = i;
			dacCtrlStruct.enablePort = false;

			//only analog channels
			if (i < DAC_ANALOG_CHANNEL_COUNT)
			{
				// Set the offset to the corresponding parking positions for waveformBuffer playback
				const USHORT offset_mid = _dacParkingPositions[i] > 0 ? 0x7fff : 0x8000;
				dacCtrlStruct.offset_val = static_cast<USHORT>(floor(_dacParkingPositions[i] / GALVO_RESOLUTION) + offset_mid);
			}
			else
			{
				dacCtrlStruct.offset_val = _dacParkingPositions[i];
			}

			dacCtrl.insert({ i, dacCtrlStruct });
		}
	}

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

		if (setDacSettingsForUnusedChannels || dacCtrl[i].enablePort)
		{
			USHORT offset = static_cast<USHORT>(dac_setting.offset_val);
			USHORT updateRate = static_cast<USHORT>(round((double)SYS_CLOCK_FREQ / dac_setting.update_rate - 1.0));
			switch (i)
			{
			case 0:
				status = FPGAregisterWRITE("DAC_Offset_Chan0", offset);
				status = FPGAregisterWRITE("DAC_UpdateRate_Chan0", updateRate);
				status = FPGAregisterWRITE("DAC_DMA_Playback_En0", dacCtrl[i].enablePort);
				status = FPGAregisterWRITE("DACWave_Filter_Inhibit0", dacCtrl[i].filterInhibit);
				status = FPGAregisterWRITE("DAC_Sync_hsync0", dacCtrl[i].hSync);
				break;
			case 1:
				status = FPGAregisterWRITE("DAC_Offset_Chan1", offset);
				status = FPGAregisterWRITE("DAC_UpdateRate_Chan1", updateRate);
				status = FPGAregisterWRITE("DAC_DMA_Playback_En1", dacCtrl[i].enablePort);
				status = FPGAregisterWRITE("DACWave_Filter_Inhibit1", dacCtrl[i].filterInhibit);
				status = FPGAregisterWRITE("DAC_Sync_hsync1", dacCtrl[i].hSync);
				break;
			case 2:
				status = FPGAregisterWRITE("DAC_Offset_Chan2", offset);
				status = FPGAregisterWRITE("DAC_UpdateRate_Chan2", updateRate);
				status = FPGAregisterWRITE("DAC_DMA_Playback_En2", dacCtrl[i].enablePort);
				status = FPGAregisterWRITE("DACWave_Filter_Inhibit2", dacCtrl[i].filterInhibit);
				status = FPGAregisterWRITE("DAC_Sync_hsync2", dacCtrl[i].hSync);
				break;
			case 3:
				status = FPGAregisterWRITE("DAC_Offset_Chan3", offset);
				status = FPGAregisterWRITE("DAC_UpdateRate_Chan3", updateRate);
				status = FPGAregisterWRITE("DAC_DMA_Playback_En3", dacCtrl[i].enablePort);
				status = FPGAregisterWRITE("DACWave_Filter_Inhibit3", dacCtrl[i].filterInhibit);
				status = FPGAregisterWRITE("DAC_Sync_hsync3", dacCtrl[i].hSync);
				break;
			case 4:
				status = FPGAregisterWRITE("DAC_Offset_Chan4", offset);
				status = FPGAregisterWRITE("DAC_UpdateRate_Chan4", updateRate);
				status = FPGAregisterWRITE("DAC_DMA_Playback_En4", dacCtrl[i].enablePort);
				status = FPGAregisterWRITE("DACWave_Filter_Inhibit4", dacCtrl[i].filterInhibit);
				status = FPGAregisterWRITE("DAC_Sync_hsync4", dacCtrl[i].hSync);
				break;
			case 5:
				status = FPGAregisterWRITE("DAC_Offset_Chan5", offset);
				status = FPGAregisterWRITE("DAC_UpdateRate_Chan5", updateRate);
				status = FPGAregisterWRITE("DAC_DMA_Playback_En5", dacCtrl[i].enablePort);
				status = FPGAregisterWRITE("DACWave_Filter_Inhibit5", dacCtrl[i].filterInhibit);
				status = FPGAregisterWRITE("DAC_Sync_hsync5", dacCtrl[i].hSync);
				break;
			case 6:
				status = FPGAregisterWRITE("DAC_Offset_Chan6", offset);
				status = FPGAregisterWRITE("DAC_UpdateRate_Chan6", updateRate);
				status = FPGAregisterWRITE("DAC_DMA_Playback_En6", dacCtrl[i].enablePort);
				status = FPGAregisterWRITE("DACWave_Filter_Inhibit6", dacCtrl[i].filterInhibit);
				status = FPGAregisterWRITE("DAC_Sync_hsync6", dacCtrl[i].hSync);
				break;
			case 7:
				status = FPGAregisterWRITE("DAC_Offset_Chan7", offset);
				status = FPGAregisterWRITE("DAC_UpdateRate_Chan7", updateRate);
				status = FPGAregisterWRITE("DAC_DMA_Playback_En7", dacCtrl[i].enablePort);
				status = FPGAregisterWRITE("DACWave_Filter_Inhibit7", dacCtrl[i].filterInhibit);
				status = FPGAregisterWRITE("DAC_Sync_hsync7", dacCtrl[i].hSync);
				break;
			case 8:
				status = FPGAregisterWRITE("DAC_Offset_Chan8", offset);
				status = FPGAregisterWRITE("DAC_UpdateRate_Chan8", updateRate);
				status = FPGAregisterWRITE("DAC_DMA_Playback_En8", dacCtrl[i].enablePort);
				status = FPGAregisterWRITE("DACWave_Filter_Inhibit8", dacCtrl[i].filterInhibit);
				status = FPGAregisterWRITE("DAC_Sync_hsync8", dacCtrl[i].hSync);
				break;
			case 9:
				status = FPGAregisterWRITE("DAC_Offset_Chan9", offset);
				status = FPGAregisterWRITE("DAC_UpdateRate_Chan9", updateRate);
				status = FPGAregisterWRITE("DAC_DMA_Playback_En9", dacCtrl[i].enablePort);
				status = FPGAregisterWRITE("DACWave_Filter_Inhibit9", dacCtrl[i].filterInhibit);
				status = FPGAregisterWRITE("DAC_Sync_hsync9", dacCtrl[i].hSync);
				break;
			case 10:
				status = FPGAregisterWRITE("DAC_Offset_Chan10", offset);
				status = FPGAregisterWRITE("DAC_UpdateRate_Chan10", updateRate);
				status = FPGAregisterWRITE("DAC_DMA_Playback_En10", dacCtrl[i].enablePort);
				status = FPGAregisterWRITE("DACWave_Filter_Inhibit10", dacCtrl[i].filterInhibit);
				status = FPGAregisterWRITE("DAC_Sync_hsync10", dacCtrl[i].hSync);
				break;
			case 11:
				status = FPGAregisterWRITE("DAC_Offset_Chan11", offset);
				status = FPGAregisterWRITE("DAC_UpdateRate_Chan11", updateRate);
				status = FPGAregisterWRITE("DAC_DMA_Playback_En11", dacCtrl[i].enablePort);
				status = FPGAregisterWRITE("DACWave_Filter_Inhibit11", dacCtrl[i].filterInhibit);
				status = FPGAregisterWRITE("DAC_Sync_hsync11", dacCtrl[i].hSync);
				break;
			case 12:
				status = FPGAregisterWRITE("DAC_Offset_Chan12", offset);
				status = FPGAregisterWRITE("DAC_UpdateRate_Chan12", updateRate);
				status = FPGAregisterWRITE("DAC_Digital_Out_Park0", static_cast<USHORT>(dacCtrl[i].park_val));
				status = FPGAregisterWRITE("DAC_DMA_Playback_En12", dacCtrl[i].enablePort);
				status = FPGAregisterWRITE("DACWave_Filter_Inhibit12", 1); //set the inhibit filter bit high for the Digital lines
				status = FPGAregisterWRITE("DAC_Sync_hsync12", dacCtrl[i].hSync);
				break;
			case 13:
				status = FPGAregisterWRITE("DAC_Offset_Chan13", offset);
				status = FPGAregisterWRITE("DAC_UpdateRate_Chan13", updateRate);
				status = FPGAregisterWRITE("DAC_Digital_Out_Park1", static_cast<USHORT>(dacCtrl[i].park_val));
				status = FPGAregisterWRITE("DAC_DMA_Playback_En13", dacCtrl[i].enablePort);
				status = FPGAregisterWRITE("DACWave_Filter_Inhibit13", 1); //set the inhibit filter bit high for the Digital lines
				status = FPGAregisterWRITE("DAC_Sync_hsync13", dacCtrl[i].hSync);
				break;
			}
		}
	}

	if (setDacSettingsForUnusedChannels)
	{
		UINT8 DmaChannelCount = WAVETABLE_CHANNEL_COUNT - 1;

		status = FPGAregisterWRITE("DAC_DMA_Chans_Active", DmaChannelCount);

		status = FPGAregisterWRITE("DACWaveGen_EOF_DelaySampleCnt", 0); // 0x2B8
	}

	return status;
}

inline THORDAQ_STATUS CThordaq::DACTrackCompletedDescriptorsDynamicLoadingWaveform()
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	UINT64 currentInterruptCount[WAVETABLE_CHANNEL_COUNT];
	std::memset(currentInterruptCount, 0, sizeof(currentInterruptCount));

	UINT64 currrentInterruptCountInGroup[WAVETABLE_CHANNEL_COUNT];
	std::memset(currrentInterruptCountInGroup, 0, sizeof(currrentInterruptCountInGroup));

	UINT64 currrentSampleCountInGroup[WAVETABLE_CHANNEL_COUNT];
	std::memset(currrentSampleCountInGroup, 0, sizeof(currrentSampleCountInGroup));

	UINT8 i = 0;

	while (!_dacAbortContinousMode && true == _dacContinuousModeStartStopStatus)
	{
		for (i = 0; i < WAVETABLE_CHANNEL_COUNT; ++i)
		{
			if ((_dacAbortContinousMode || !_dacContinuousModeStartStopStatus))
			{
				break;
			}
			//TODO: add a % of the number of samples, per the number of interrupts, 
			//but need to keep track if samples decrement in another  cycle (likely the lastone)
			if (_dacDescTableSamplesTracker.find(i) != _dacDescTableSamplesTracker.end() && _dacDescTableSamplesTracker[i].size() > 0)
			{
				while (currentInterruptCount[i] < _DACWaveGenInterruptCounter[i])
				{
					++currentInterruptCount[i];

					++currrentInterruptCountInGroup[i];

					auto samplesTrackerVector = _dacDescTableSamplesTracker[i];

					if (_dacDescCountToTriggerApproachingNSamplesEvent > 0 && _dacDescCountToTriggerApproachingNSamplesEvent[i] == currrentInterruptCountInGroup[i])
					{
						if (_dacDescTableSamplesTracker.find(i) != _dacDescTableSamplesTracker.end() && _dacApproachingNSamplesCallbackPtrs.find(i) != _dacApproachingNSamplesCallbackPtrs.end())
						{
							_dacApproachingNSamplesCallbackPtrs[i].callbackFunction(i, _dacApproachingNSamplesCallbackPtrs[i].numberOfSamples, THORDAQ_STATUS::STATUS_SUCCESSFUL, NULL);
						}
					}
					if ((_dacAbortContinousMode || !_dacContinuousModeStartStopStatus))
					{
						break;
					}
					if (_dacDescCountToTriggerApproachinLoadedWaveformEndEvent[i] > 0 && _dacDescCountToTriggerApproachinLoadedWaveformEndEvent[i] == currrentInterruptCountInGroup[i])
					{
						if (_dacDescTableSamplesTracker.find(i) != _dacDescTableSamplesTracker.end() && _dacApproachinLoadedWaveformEndCallbackPtrs.find(i) != _dacApproachinLoadedWaveformEndCallbackPtrs.end())
						{
							_DACWaveGenInterruptCountWhenApproachingCallbackCalled[i] = currentInterruptCount[i];
							_dacApproachinLoadedWaveformEndCallbackPtrs[i].callbackFunction(i, THORDAQ_STATUS::STATUS_SUCCESSFUL, NULL);
						}
					}
					if ((_dacAbortContinousMode || !_dacContinuousModeStartStopStatus))
					{
						break;
					}

					if (samplesTrackerVector.size() > (currrentInterruptCountInGroup[i] - 1))
					{
						currrentSampleCountInGroup[i] += samplesTrackerVector[currrentInterruptCountInGroup[i] - 1].size >> 1;
						if (_dacSampleCountToTriggerCycleCompleteEvent[i] > 0 && _dacSampleCountToTriggerCycleCompleteEvent[i] <= currrentSampleCountInGroup[i])
						{
							if (_dacDescTableSamplesTracker.find(i) != _dacDescTableSamplesTracker.end() && _dacCycleDoneCallbackPtrs.find(i) != _dacCycleDoneCallbackPtrs.end())
							{
								_dacCycleDoneCallbackPtrs[i].callbackFunction(i, THORDAQ_STATUS::STATUS_SUCCESSFUL, NULL);
							}
							currrentSampleCountInGroup[i] = 0;
						}
					}
					else
					{
						break;
						int x = 0;
					}
					if ((_dacAbortContinousMode || !_dacContinuousModeStartStopStatus))
					{
						break;
					}
					//reset when at the total number of descriptors
					if (_dacDescriptorsPerChannel[i] == currrentInterruptCountInGroup[i])
					{
						currrentInterruptCountInGroup[i] = 0;
					}

					if (_dacWaveformPlaybackStartedCallbackPtr.callbackFunction != NULL && false == _dacWaveformPlaybackStartedCallbackPtr.hasBeenCalled)
					{
						_dacWaveformPlaybackStartedCallbackPtr.hasBeenCalled = true;

						//TODO: when doing HW trigger we should check weather the trigger has gone high instead
						_dacWaveformPlaybackStartedCallbackPtr.callbackFunction(THORDAQ_STATUS::STATUS_SUCCESSFUL, NULL);
					}
					if ((_dacAbortContinousMode || !_dacContinuousModeStartStopStatus))
					{
						break;
					}
					//currentInterruptCount[i] += samplesTrackerVector[currentIndex].size;

					//TODO: when starting the acquisition calculate at which point (desc index, or number of size) would be appropriate to call the callback function

					//TODO: thinking I should have 2 separate threads waiting to call the callback functions without interrupting this thread

					//TODO: reset the sample count when we complete the last sample in the cycle (after nSamples are completed)

					size_t totalCurrentSamples = 0;
				}

				//currentInterruptCount[i] = _DACWaveGenInterruptCounter[i];

			}
		}

		std::this_thread::sleep_for(std::chrono::microseconds(1)); //TODO: remove if not needed
	};

	_dacWaveformCurrentCycleCount = currentInterruptCount[i];

	if (_dacAbortContinousMode)
	{
		status = STATUS_ACQUISITION_ABORTED;
	}

	return status;
}

THORDAQ_STATUS CThordaq::DACTrackCompletedDescriptorsStaticWaveform()
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	UINT8 i = 0;

	while (!_dacAbortContinousMode && true == _dacContinuousModeStartStopStatus)
	{
		for (i = 0; i < WAVETABLE_CHANNEL_COUNT; ++i)
		{
			if (_DACWaveGenInterruptCounter[i] != 0)
			{
				if (_dacWaveformPlaybackStartedCallbackPtr.callbackFunction != NULL && false == _dacWaveformPlaybackStartedCallbackPtr.hasBeenCalled)
				{
					_dacWaveformPlaybackStartedCallbackPtr.hasBeenCalled = true;

					//TODO: when doing HW trigger we should check weather the trigger has gone high instead
					_dacWaveformPlaybackStartedCallbackPtr.callbackFunction(THORDAQ_STATUS::STATUS_SUCCESSFUL, NULL);
				}
			}

			if (_dacStaticLoadInterruptsPerCycle <= _DACWaveGenInterruptCounter[i])
			{
				if (_dacWaveformPlaybackCompletePtr.callbackFunction != NULL)
				{
					//TODO: should make this oenw ork for dynamic waveform loading too (we know the number of cycles and samples per cycle, so we can keep track)
					_dacWaveformPlaybackCompletePtr.callbackFunction(STATUS_SUCCESSFUL, NULL);
				}
				return status;
			}
		}
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	};

	if (_dacAbortContinousMode)
	{
		status = STATUS_ACQUISITION_ABORTED;
	}

	return status;
}

THORDAQ_STATUS CThordaq::DACResetWaveGenInterrupts()
{
	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;
	//set the desired intterupt counts to  0 for all channels
	status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt0", 0);
	status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt1", 0);
	status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt2", 0);
	status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt3", 0);
	status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt4", 0);
	status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt5", 0);
	status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt6", 0);
	status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt7", 0);
	status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt8", 0);
	status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt9", 0);
	status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt10", 0);
	status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt11", 0);
	status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt12", 0);
	status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt13", 0);
	return status;
}


THORDAQ_STATUS CThordaq::DACSetWaveGenInterruptsOnChannel(UINT8 dacChannel, UINT8 descCntBeforeInterrupt)
{
	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;

	//can only count two or more descriptors, or set to 0 to not count on that channel
	if (descCntBeforeInterrupt > DAC_MAX_DESCRIPTORS_PER_INTERRUPT)
	{
		return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;
	}

	if (dacChannel >= WAVETABLE_CHANNEL_COUNT)
	{
		return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;
	}

	//set the number of descpriptors per interrupt for the desired channel dacChannel
	switch (dacChannel)
	{
	case 0: status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt0", descCntBeforeInterrupt); break;
	case 1: status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt1", descCntBeforeInterrupt); break;
	case 2: status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt2", descCntBeforeInterrupt); break;
	case 3: status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt3", descCntBeforeInterrupt); break;
	case 4: status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt4", descCntBeforeInterrupt); break;
	case 5: status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt5", descCntBeforeInterrupt); break;
	case 6: status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt6", descCntBeforeInterrupt); break;
	case 7: status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt7", descCntBeforeInterrupt); break;
	case 8: status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt8", descCntBeforeInterrupt); break;
	case 9: status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt9", descCntBeforeInterrupt); break;
	case 10: status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt10", descCntBeforeInterrupt); break;
	case 11: status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt11", descCntBeforeInterrupt); break;
	case 12: status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt12", descCntBeforeInterrupt); break;
	case 13: status = FPGAregisterWRITE("DACWaveGenDescCntBeforeInt13", descCntBeforeInterrupt); break;
	}

	return status;
}

inline THORDAQ_STATUS CThordaq::DACReArmWaveplayIRQ()
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	status = FPGAregisterWrite(_DACWaveGen3PSyncControlRegIndex, _rearmDAC1);  // clears FPGA's ISR, but not our Shadow bit fields

	status = FPGAregisterWrite(_DACWaveGen3PSyncControlRegIndex, _rearmDAC0);  // set to 0 after 1 as per SWUG

	return status;
}

THORDAQ_STATUS  CThordaq::DACResetContinuousModeOnAllChannels()
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En0", 0x0);
	status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En1", 0x0);
	status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En2", 0x0);
	status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En3", 0x0);
	status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En4", 0x0);
	status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En5", 0x0);
	status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En6", 0x0);
	status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En7", 0x0);
	status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En8", 0x0);
	status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En9", 0x0);
	status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En10", 0x0);
	status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En11", 0x0);
	status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En12", 0x0);
	status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En13", 0x0);

	return status;
}

THORDAQ_STATUS CThordaq::DACSetContinuousModeTriggerLUTOnChannel(UINT8 lutAddress, UINT8 dacChannel, bool do0, bool do1)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	BYTE do0do1 = (BYTE)do0 | (BYTE)do1 << 1;
	status = FPGAregisterWRITE("WaveformControlPT_InputCfgIndx", lutAddress);
	status = FPGAregisterWRITE("WaveformControlPT_OutCfgFn", do0do1);
	status = FPGAregisterWRITE("WaveformControlPT_TruthTableCfg_SEL", dacChannel);
	status = FPGAregisterWRITE("WaveformControlPT_CfgWriteSTROBE", 0x0);
	status = FPGAregisterWRITE("WaveformControlPT_CfgWriteSTROBE", 0x1);

	return status;
}

THORDAQ_STATUS CThordaq::DACSetTriggerSettingsOnChannel(UINT8 dacChannel, THORDAQ_TRIGGER_SETTINGS triggerSettings)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	bool doTrigger = !(THORDAQ_TRIGGER_MODES::THORDAQ_SOFTWARE_START == triggerSettings.triggerMode ||
		(THORDAQ_HW_TRIGGER_MODES::THORDAQ_NO_TRRIGER == triggerSettings.hwTrigger1Mode &&
			THORDAQ_HW_TRIGGER_MODES::THORDAQ_NO_TRRIGER == triggerSettings.hwTrigger2Mode &&
			false == triggerSettings.enableInternalDigitalTrigger));

	switch (dacChannel)
	{
	case 0:
		status = FPGAregisterWRITE("WaveformControlPT_HWSW_SEL_chan0", doTrigger);

		if (doTrigger)
		{
			status = FPGAregisterWRITE("WaveformControlPT_HW_In1_SEL_chan0", triggerSettings.hwTrigger1Selection);
			status = FPGAregisterWRITE("WaveformControlPT_HW_In2_SEL_chan0", triggerSettings.hwTrigger2Selection);
			status = FPGAregisterWRITE("WaveformControlPT_WaveformIN_SEL_chan0", triggerSettings.internalDigitalTrigger);
		}
		break;
	case 1:
		status = FPGAregisterWRITE("WaveformControlPT_HWSW_SEL_chan1", doTrigger);

		if (doTrigger)
		{
			status = FPGAregisterWRITE("WaveformControlPT_HW_In1_SEL_chan1", triggerSettings.hwTrigger1Selection);
			status = FPGAregisterWRITE("WaveformControlPT_HW_In2_SEL_chan1", triggerSettings.hwTrigger2Selection);
			status = FPGAregisterWRITE("WaveformControlPT_WaveformIN_SEL_chan1", triggerSettings.internalDigitalTrigger);
		}
		break;
	case 2:
		status = FPGAregisterWRITE("WaveformControlPT_HWSW_SEL_chan2", doTrigger);

		if (doTrigger)
		{
			status = FPGAregisterWRITE("WaveformControlPT_HW_In1_SEL_chan2", triggerSettings.hwTrigger1Selection);
			status = FPGAregisterWRITE("WaveformControlPT_HW_In2_SEL_chan2", triggerSettings.hwTrigger2Selection);
			status = FPGAregisterWRITE("WaveformControlPT_WaveformIN_SEL_chan2", triggerSettings.internalDigitalTrigger);
		}
		break;
	case 3:
		status = FPGAregisterWRITE("WaveformControlPT_HWSW_SEL_chan3", doTrigger);

		if (doTrigger)
		{
			status = FPGAregisterWRITE("WaveformControlPT_HW_In1_SEL_chan3", triggerSettings.hwTrigger1Selection);
			status = FPGAregisterWRITE("WaveformControlPT_HW_In2_SEL_chan3", triggerSettings.hwTrigger2Selection);
			status = FPGAregisterWRITE("WaveformControlPT_WaveformIN_SEL_chan3", triggerSettings.internalDigitalTrigger);
		}
		break;
	case 4:
		status = FPGAregisterWRITE("WaveformControlPT_HWSW_SEL_chan4", doTrigger);

		if (doTrigger)
		{
			status = FPGAregisterWRITE("WaveformControlPT_HW_In1_SEL_chan4", triggerSettings.hwTrigger1Selection);
			status = FPGAregisterWRITE("WaveformControlPT_HW_In2_SEL_chan4", triggerSettings.hwTrigger2Selection);
			status = FPGAregisterWRITE("WaveformControlPT_WaveformIN_SEL_chan4", triggerSettings.internalDigitalTrigger);
		}
		break;
	case 5:
		status = FPGAregisterWRITE("WaveformControlPT_HWSW_SEL_chan5", doTrigger);

		if (doTrigger)
		{
			status = FPGAregisterWRITE("WaveformControlPT_HW_In1_SEL_chan5", triggerSettings.hwTrigger1Selection);
			status = FPGAregisterWRITE("WaveformControlPT_HW_In2_SEL_chan5", triggerSettings.hwTrigger2Selection);
			status = FPGAregisterWRITE("WaveformControlPT_WaveformIN_SEL_chan5", triggerSettings.internalDigitalTrigger);
		}
		break;
	case 6:
		status = FPGAregisterWRITE("WaveformControlPT_HWSW_SEL_chan6", doTrigger);

		if (doTrigger)
		{
			status = FPGAregisterWRITE("WaveformControlPT_HW_In1_SEL_chan6", triggerSettings.hwTrigger1Selection);
			status = FPGAregisterWRITE("WaveformControlPT_HW_In2_SEL_chan6", triggerSettings.hwTrigger2Selection);
			status = FPGAregisterWRITE("WaveformControlPT_WaveformIN_SEL_chan6", triggerSettings.internalDigitalTrigger);
		}
		break;
	case 7:
		status = FPGAregisterWRITE("WaveformControlPT_HWSW_SEL_chan7", doTrigger);

		if (doTrigger)
		{
			status = FPGAregisterWRITE("WaveformControlPT_HW_In1_SEL_chan7", triggerSettings.hwTrigger1Selection);
			status = FPGAregisterWRITE("WaveformControlPT_HW_In2_SEL_chan7", triggerSettings.hwTrigger2Selection);
			status = FPGAregisterWRITE("WaveformControlPT_WaveformIN_SEL_chan7", triggerSettings.internalDigitalTrigger);
		}
		break;
	case 8:
		status = FPGAregisterWRITE("WaveformControlPT_HWSW_SEL_chan8", doTrigger);

		if (doTrigger)
		{
			status = FPGAregisterWRITE("WaveformControlPT_HW_In1_SEL_chan8", triggerSettings.hwTrigger1Selection);
			status = FPGAregisterWRITE("WaveformControlPT_HW_In2_SEL_chan8", triggerSettings.hwTrigger2Selection);
			status = FPGAregisterWRITE("WaveformControlPT_WaveformIN_SEL_chan8", triggerSettings.internalDigitalTrigger);
		}
		break;
	case 9:
		status = FPGAregisterWRITE("WaveformControlPT_HWSW_SEL_chan9", doTrigger);

		if (doTrigger)
		{
			status = FPGAregisterWRITE("WaveformControlPT_HW_In1_SEL_chan9", triggerSettings.hwTrigger1Selection);
			status = FPGAregisterWRITE("WaveformControlPT_HW_In2_SEL_chan9", triggerSettings.hwTrigger2Selection);
			status = FPGAregisterWRITE("WaveformControlPT_WaveformIN_SEL_chan9", triggerSettings.internalDigitalTrigger);
		}
		break;
	case 10:
		status = FPGAregisterWRITE("WaveformControlPT_HWSW_SEL_chan10", doTrigger);

		if (doTrigger)
		{
			status = FPGAregisterWRITE("WaveformControlPT_HW_In1_SEL_chan10", triggerSettings.hwTrigger1Selection);
			status = FPGAregisterWRITE("WaveformControlPT_HW_In2_SEL_chan10", triggerSettings.hwTrigger2Selection);
			status = FPGAregisterWRITE("WaveformControlPT_WaveformIN_SEL_chan10", triggerSettings.internalDigitalTrigger);
		}
		break;
	case 11:
		status = FPGAregisterWRITE("WaveformControlPT_HWSW_SEL_chan11", doTrigger);

		if (doTrigger)
		{
			status = FPGAregisterWRITE("WaveformControlPT_HW_In1_SEL_chan11", triggerSettings.hwTrigger1Selection);
			status = FPGAregisterWRITE("WaveformControlPT_HW_In2_SEL_chan11", triggerSettings.hwTrigger2Selection);
			status = FPGAregisterWRITE("WaveformControlPT_WaveformIN_SEL_chan11", triggerSettings.internalDigitalTrigger);
		}
		break;
	case 12:
		status = FPGAregisterWRITE("WaveformControlPT_HWSW_SEL_chan12", doTrigger);

		if (doTrigger)
		{
			status = FPGAregisterWRITE("WaveformControlPT_HW_In1_SEL_chan12", triggerSettings.hwTrigger1Selection);
			status = FPGAregisterWRITE("WaveformControlPT_HW_In2_SEL_chan12", triggerSettings.hwTrigger2Selection);
			status = FPGAregisterWRITE("WaveformControlPT_WaveformIN_SEL_chan12", triggerSettings.internalDigitalTrigger);
		}
		break;
	case 13:
		status = FPGAregisterWRITE("WaveformControlPT_HWSW_SEL_chan13", doTrigger);

		if (doTrigger)
		{
			status = FPGAregisterWRITE("WaveformControlPT_HW_In1_SEL_chan13", triggerSettings.hwTrigger1Selection);
			status = FPGAregisterWRITE("WaveformControlPT_HW_In2_SEL_chan13", triggerSettings.hwTrigger2Selection);
			status = FPGAregisterWRITE("WaveformControlPT_WaveformIN_SEL_chan13", triggerSettings.internalDigitalTrigger);
		}
		break;
	}

	if (doTrigger)
	{
		for (BYTE lutAddress = 0; lutAddress < 32; ++lutAddress)
		{
			bool do0 = false;
			bool do1 = false;

			TriggerLogicResponse(triggerSettings, lutAddress, do0, do1);
			DACSetContinuousModeTriggerLUTOnChannel(lutAddress, dacChannel, do0, do1);
		}
		status = FPGAregisterWRITE("WaveformControlPT_CfgWriteSTROBE", 0x0);
	}

	return status;
}

THORDAQ_STATUS CThordaq::DACEnableDisablePerChannelRunStopOnChannel(UINT8 dacChannel, bool enable)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	switch (dacChannel)
	{
	case 0:
		status = FPGAregisterWRITE("PT_WaveformControlTrig_SEL0", enable);
		break;
	case 1:
		status = FPGAregisterWRITE("PT_WaveformControlTrig_SEL1", enable);
		break;
	case 2:
		status = FPGAregisterWRITE("PT_WaveformControlTrig_SEL2", enable);
		break;
	case 3:
		status = FPGAregisterWRITE("PT_WaveformControlTrig_SEL3", enable);
		break;
	case 4:
		status = FPGAregisterWRITE("PT_WaveformControlTrig_SEL4", enable);
		break;
	case 5:
		status = FPGAregisterWRITE("PT_WaveformControlTrig_SEL5", enable);
		break;
	case 6:
		status = FPGAregisterWRITE("PT_WaveformControlTrig_SEL6", enable);
		break;
	case 7:
		status = FPGAregisterWRITE("PT_WaveformControlTrig_SEL7", enable);
		break;
	case 8:
		status = FPGAregisterWRITE("PT_WaveformControlTrig_SEL8", enable);
		break;
	case 9:
		status = FPGAregisterWRITE("PT_WaveformControlTrig_SEL9", enable);
		break;
	case 10:
		status = FPGAregisterWRITE("PT_WaveformControlTrig_SEL10", enable);
		break;
	case 11:
		status = FPGAregisterWRITE("PT_WaveformControlTrig_SEL11", enable);
		break;
	case 12:
		status = FPGAregisterWRITE("PT_WaveformControlTrig_SEL12", enable);
		break;
	case 13:
		status = FPGAregisterWRITE("PT_WaveformControlTrig_SEL13", enable);
		break;
	}

	return status;
}

THORDAQ_STATUS CThordaq::DACEnableDisableContinuousModeOnChannel(UINT8 dacChannel, bool enable)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	switch (dacChannel)
	{
	case 0:
		status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En0", enable);
		break;
	case 1:
		status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En1", enable);
		break;
	case 2:
		status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En2", enable);
		break;
	case 3:
		status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En3", enable);
		break;
	case 4:
		status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En4", enable);
		break;
	case 5:
		status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En5", enable);
		break;
	case 6:
		status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En6", enable);
		break;
	case 7:
		status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En7", enable);
		break;
	case 8:
		status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En8", enable);
		break;
	case 9:
		status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En9", enable);
		break;
	case 10:
		status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En10", enable);
		break;
	case 11:
		status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En11", enable);
		break;
	case 12:
		status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En12", enable);
		break;
	case 13:
		status = FPGAregisterWRITE("DAC_Waveplay_Cont_Playback_En13", enable);
		break;
	}

	//if we are enabling Continuous mode on a DAC channel, we must turn off hsync on that channel
	//per SWUG
	// For Z control we want to sync to imaging so it should be a per channel setup using hsync variable
	//if (enable)
	//{
	//	switch (dacChannel)
	//	{
	//		case 0: status = FPGAregisterWRITE("DAC_Sync_hsync0", 0x0); break;
	//		case 1: status = FPGAregisterWRITE("DAC_Sync_hsync1", 0x0); break;
	//		case 2: status = FPGAregisterWRITE("DAC_Sync_hsync2", 0x0); break;
	//		case 3: status = FPGAregisterWRITE("DAC_Sync_hsync3", 0x0); break;
	//		case 4: status = FPGAregisterWRITE("DAC_Sync_hsync4", 0x0); break;
	//		case 5: status = FPGAregisterWRITE("DAC_Sync_hsync5", 0x0); break;
	//		case 6: status = FPGAregisterWRITE("DAC_Sync_hsync6", 0x0); break;
	//		case 7: status = FPGAregisterWRITE("DAC_Sync_hsync7", 0x0); break;
	//		case 8: status = FPGAregisterWRITE("DAC_Sync_hsync8", 0x0); break;
	//		case 9: status = FPGAregisterWRITE("DAC_Sync_hsync9", 0x0); break;
	//		case 10: status = FPGAregisterWRITE("DAC_Sync_hsync10", 0x0); break;
	//		case 11: status = FPGAregisterWRITE("DAC_Sync_hsync11", 0x0); break;
	//		case 12: status = FPGAregisterWRITE("DAC_Sync_hsync12", 0x0); break;
	//		case 13: status = FPGAregisterWRITE("DAC_Sync_hsync13", 0x0); break;
	//	}
	//}

	return status;
}


THORDAQ_STATUS CThordaq::DACSetContinuousModeGeneralSettings(std::map<UINT, DAC_WAVE_DESC_STRUCT> dacWaveDescs, std::map<UINT, DAC_CRTL_STRUCT> dacCtrl, bool dynamicWaveform, std::map<UINT8, bool> enabledChannels, DAC_FREERUN_WAVEFORM_GENERAL_SETTINGS* generalSettings)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	UINT64 currentVal = 0x0;
	status = FPGAregisterRead(_DACWaveGen3PSyncControlRegIndex, -1, &currentVal);  // clears FPGA's ISR, but not our Shadow bit fields
	_rearmDAC1 = 0x20000000 | currentVal; //setting bit 29 to 1
	_rearmDAC0 = (~(0x20000000)) & currentVal; //setting bit 29 to 0 

	//rearm the waveplay IRQ to receive interrupts
	if (DACReArmWaveplayIRQ() != THORDAQ_STATUS::STATUS_SUCCESSFUL)
	{
		status = STATUS_PARAMETER_SETTINGS_ERROR;
	}

	_DACPerChannelRuntBitSelection = 0x0;

	for (UINT8 i = 0; i < WAVETABLE_CHANNEL_COUNT; i++)
	{
		if (enabledChannels.find(i) != enabledChannels.end())
		{
			if (DACEnableDisableContinuousModeOnChannel(i, true) != THORDAQ_STATUS::STATUS_SUCCESSFUL)
			{
				status = STATUS_PARAMETER_SETTINGS_ERROR;
				break;
			}

			//TODO: this needs to be an option
			if (DACEnableDisablePerChannelRunStopOnChannel(i, true) != THORDAQ_STATUS::STATUS_SUCCESSFUL)
			{
				status = STATUS_PARAMETER_SETTINGS_ERROR;
				break;
			}

			DACSetTriggerSettingsOnChannel(i, dacCtrl[i].triggerSettings);
			_DACPerChannelRuntBitSelection |= 0x1 << i;
		}
		else
		{
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
	}

	status = DACSetChannelsSettings(dacCtrl);

	//// Load the DMA Descriptors
	if (DACLoadWaveformAndDescriptors(dacWaveDescs, ThorDAQExperimentType::DACFreeRunMode, dynamicWaveform) == FALSE)
	{
		status = STATUS_PARAMETER_SETTINGS_ERROR;
	}

	if (!_imagingEventPrepared && !_imagingStartStopStatus)
	{
		SetDIOChannelSelection(generalSettings->digitalLinesConfig);
	}

	status = FPGAregisterWRITE("DAC_Waveplay_Start_Of_Waveform_Delay_Enable", generalSettings->allowTimeToMoveToOffset);
	_dacContinuousModeEventPrepared = true;
	return status;
}

//Prioritized TODOs
//TODO: -find a way to delete memory created in this project for the waveform, so each dll takes care of it's memory -- Done
//		-maybe add another descriptor up front to make it smooth -- Done
//		-this is only necessary with one channel, other channels can have 3descp -- Done
//		-Keep a variable that has the number of descriptors for the waveform. Perhaps we should just always have 2 descriptors per interrupt -- Done
//		-Simplify how the waveform is passed from the GG and GR dlls to the thordaq level, old logic was better, but can be improved. But also do it in a way that can be expanded. -- Done
//		-interrupts in longer waveforms get messed up. seems to be working as off of 20201130 -- Done
//		-Need to test tornado fill and raster, make sure they don't trip the galvos -- Done
//		-Need to make it work for very fast short waveforms (apparently missing interrupts), could it be becaue I'm only adding descriptors to the longest channel? -- Done, new item Added to address remaining issues
//		-The offset should not be set outside, at least not for the current settings. -- Done, now the offset is set outside but in int16 value, to the start value or whatever we want it to be3
//		-need to set priority on std::threads to highest. -- Done
//		-Need to add the callback ability, should be able to set the number of samples before a callback, and devide up the waveform appropiately. -- Done
//		-Need to keep a variable with the number of desc to count total for the inner cycle (the first group of data loaded) and -1 and for the whole real cycle. -- Done.
//		-think about using the triggers in one channel to count outer decriptors in dynamic load waveform section -1, another one to count descriptors in dynamic load waveform section, and a third one to count descriptors per cycle... -- Not needed.
//				even if channels are not set, set them, do this if unable to keep track of single desc counts because they are coming too quickly for the PC -- Not needed.
//		-Calculate a minimum time per sample loaded needed, the smaller the better -- Done
//		-should probably start with the first few values in the first descp, then we can always point to the first descp when looping, and there is no delay for the real waveform to start. -- Done
//		-Short waveforms don't get interrupts, there must be a minimum number of samples/rep rate that works//. -- Fixed
//		-Count cycles in higher level. -- Done for longer dynamically loaded waveforms, for statically loaded waveforms cycles are assumed to be to short to be counted outside
//		-Get looping and just general preloaded full waveform working again for short waveforms < 10ms... no need to active load these. -- Done
//		-Add the digital channels, test them, the whole waveform extraction logic for digital channels has to be reworked. -- Done
//		-Add a way to configure the digital channels. -- Done
//		-talk to Bill about digital lines quickly going up and down after runStop bit set to 0. -- Done, Bill is looking into it
//		-it would be nice if we could divorce some DAC channels from the image timing. -- FW part is Done, need to do SW
//		-check for corner cases when dividing up the waveform. -- Done
//		-when there is a crash make sure thorimage kills everything at startup. -- Done
//		-configure DAC_UpdateRate_Chan8, etc, into bitfields. -- Done
//		-make it so the settings file DIOs are match the silk screen. -- Done
//		-add active loading count to settings file. -- Done
//		-Preload the next part of the waveform to be able to load the next waveform section faster... After loading the new section then again we should preload the next. -- Done
//		-Prepass the waveform to thordaq.dll so it processes it before it's needed (separate it into each separate group and shift each value by the offset). -- Done
//		-Use DoMem instead of shadow registers to rearm the dac desc user IRQ. Used FPGAregisterWrite with a predetermined index to save time. -- Done
//		-create a way to just start the same acquisition again, with maybe just changing the number of frames to quickly switch from GG stim to GR imaging. -- Done
//		-Might be better to do a channel imaging, and another channel STIM for digital. -- Done
//		-try to see if it might be easier to park at offset position and set the filter window to zero and the intraframe delay to zero so it starts right away. -- Done
//		-Can see the bouncing digital lins with very unique dac waveform settings on continuous mode.992x992 image, square is about a qtr of image (top left quadrant) pixel density = 49, dwelltime 10000us, est duration 381.9ms, boundary trace, total time per cycle 401.50.  -- Fixed in FW
// 
// 
//		-Check if the HW trigger came in to raise the stated event when in HW trigger state
//		-Create a new XML to read the IO settings to be shared by all cameras
//		-Need a method to know exactly which group I'm on.. currently not able to track if we are on new group when ending: currrentSampleCountInGroup[i] += samplesTrackerVector[currrentInterruptCountInGroup[i] - 1].size >> 1;
//		-use readRegister to check the status of the glogbal runstop bit instead of a flag


//TODO: <-when everything is working well and coding is complete or near complete do the following:
//		-Cleanup code, add more functions, encapsulate what's possible in classes... should use Don's I2C class as example: THORDAQ_STATUS SetupI2CMux(CThordaq& TdBrd, BOOL Master)s
//		-extensively test removing the FIFO_DELAY_SAMPLES from the GG and GR logic, so waveforms can be built faster
//		-SQA needs to test different DAC update rates, make sure they work well
//		-should probably set the idle position for the cycle complete digital line to 1... should I?
//		-Check the AI voltage to be able to get rid of the NI functions all together
//		-Decide if we should keep function DACDynamicLoadWaveform() now that we have DACDynamicLoadPresetWaveform()... it might be good to keep DACDynamicLoadWaveform for other users, it's easier to use
			//and have to worry less about timing

//		-if the waveform is too short even for static loading, then add the last sample many times at the end until it's long enough

//register to get count of lines read


//TODO:
//		<-Fix trigger each stim, with simultaneous imaging, by the last descriptor aways being the first descriptor pointint to itself, that way it will always be the first descriptor to be read
//			regardless of imaging being stopped or not before restarting stim
//		<-Figure out a way so that the DAC lines can be set up from different dlls when being used in continuous mode

