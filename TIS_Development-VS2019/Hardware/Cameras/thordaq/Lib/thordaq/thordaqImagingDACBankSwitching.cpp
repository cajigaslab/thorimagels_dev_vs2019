#include "stdafx.h"
#pragma warning(disable:4201) //I just wanna use freedom nonstandard "nameless struct/union"
#include "thordaq.h"
#include "thordaqguid.h"


/// <summary>
/// Register for the ReadyForNextWaveform event, everytime this function is called, it will be added to the list of registered event handler
/// </summary>
/// <param name="options"></param>
/// <param name="callbackFunction"></param>
/// <param name="callbackData"></param>
/// <returns></returns>
THORDAQ_STATUS CThordaq::DACBankSwitchingRegisterReadyForNextImageWaveformsEvent(UINT32 options, ThorDAQDACBankSwitchingReadyForNextWaveformCallbackPtr callbackFunction, void* callbackData)
{

	DACBankSwitchingReadyForNextWaveformCallbackSettings callbackStruct = DACBankSwitchingReadyForNextWaveformCallbackSettings();
	callbackStruct.callbackFunction = callbackFunction;
	callbackStruct.options = options;
	callbackStruct.callbackData = callbackData;

	_dacBankSwitchingReadyForNextWaveformCallbackSettingsList.push_back(callbackStruct);

	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	return status;
}

/// <summary>
/// Clear registered event handlers
/// </summary>
/// <returns></returns>
THORDAQ_STATUS CThordaq::DACBankSwitchingClearRegisteredReadyForNextImageWaveformsEventHandlers()
{
	_dacBankSwitchingReadyForNextWaveformCallbackSettingsList.clear();

	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	return status;
}

/// <summary>
/// load the waveform that goes in the next bank, while it is not being used
/// </summary>
/// <param name="dacCtrlSettings"></param>
/// <returns></returns>
THORDAQ_STATUS CThordaq::DACBankSwitchingLoadNextWaveform(std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT> dacCtrlSettings)
{
	int currentBankIndx = int(_usrIrqWaitStruct.DMA_Bank & 0x00000001);
	int bankIndx = 1 == currentBankIndx ? 0 : 1;
	if (bankIndx >= DAC_DESC_BANKS)
	{
		return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;
	}

	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;

	std::map<UINT, DAC_WAVE_DESC_STRUCT> dac_settings;
	for (UINT i = 0; i < WAVETABLE_CHANNEL_COUNT; i++)
	{
		if (dacCtrlSettings.find(i) != dacCtrlSettings.end())
		{
			DAC_WAVE_GROUP_STRUCT wavestruct = DAC_WAVE_GROUP_STRUCT();
			wavestruct.waveformBuffer = dacCtrlSettings[i].waveformBuffer;// +FIFO_DELAY_SAMPLES;
			wavestruct.waveform_buffer_size = dacCtrlSettings[i].waveform_buffer_size;// -FIFO_DELAY_SAMPLES * sizeof(USHORT);
			wavestruct.isLoopEnd = true;
			wavestruct.isLoopStart = false;
			wavestruct.loopEnable = false;
			wavestruct.needsMemoryCleanup = false;

			DAC_WAVE_DESC_STRUCT waveDesc = DAC_WAVE_DESC_STRUCT();
			waveDesc.finishPlaybackInLastGroup = false;
			waveDesc.dacWaveformGroups.push_back(wavestruct);

			dac_settings.insert({ i, waveDesc });
		}
	}

	std::map<UINT, DAC_WAVE_DESC_STRUCT> waveformSettings = dac_settings;

	UINT64 bufferOffset = 0;
	UINT64 baseAddress = 0 == bankIndx ? 0x50000000 : _dacSecondBankWaveformDDR3StartAddress;
	
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
					ULONG64 startAddress = baseAddress + _dacBankSwitchingPerChannelAddressOffset[i];

					status = WriteDDR3((UCHAR*)waveGRoup.waveformBuffer, startAddress, static_cast<UINT32>(waveGRoup.waveform_buffer_size));

					waveGRoup.waveform_buffer_start_address = startAddress;
					bufferOffset += waveGRoup.waveform_buffer_size;
				}
			}
		}
	}

	return status;
}

/// <summary>
/// Load the DAC waveformBuffer descriptors into the board, for the selected bank index
//  Two bank system allows dynamic
/// data loading.
/// </summary>
/// <param name="dac_settings"></param>
/// <returns></returns>
LONG CThordaq::DACBankSwitchingLoadWaveformAndDescriptors(std::map<UINT, DAC_WAVE_DESC_STRUCT> dac_settings, UINT8 bankIndx, bool preDynamicLoad)
{
	if (bankIndx >= DAC_DESC_BANKS)
	{
		return FALSE;
	}

	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;
	STAT_STRUCT DoMemStatus;
	std::map<UINT, DAC_WAVE_DESC_STRUCT> waveformSettings = dac_settings;
	_dacDescriptorMutex.lock();
	UINT64 bufferOffset = 0;
	UINT64 baseAddress = 0 == bankIndx ? 0x50000000 : _dacSecondBankWaveformDDR3StartAddress;
	//TODO: see if code below is repeated and simplify
	for (UINT i = 0; i < WAVETABLE_CHANNEL_COUNT; i++)
	{
		ULONG64 size = 0;

		if (dac_settings.find(i) != dac_settings.end())
		{
			for (auto& waveGRoup : waveformSettings[i].dacWaveformGroups)
			{
				_dacBankSwitchingPerChannelAddressOffset[i] = bufferOffset;
				if (NULL == waveGRoup.waveformBuffer)
				{
					waveGRoup.waveform_buffer_size = 0;
				}
				else
				{
					ULONG64 startAddress = baseAddress + bufferOffset;

					status = WriteDDR3((UCHAR*)waveGRoup.waveformBuffer, startAddress, static_cast<UINT32>(waveGRoup.waveform_buffer_size));

					waveGRoup.waveform_buffer_start_address = startAddress;
					bufferOffset += waveGRoup.waveform_buffer_size;
				}				
			}
		}
	}

	if (0 == bankIndx)
	{
		_dacSecondBankWaveformDDR3StartAddress = bufferOffset + baseAddress;
	}

	const ULONG64 DESCP_LENGTH = DAC_DESCP_LENGTH / (ULONG64)DAC_DESCP_BANKS;

	// clear the _dacDescpList first
	memset(_dacDMAMultibankDescTable->descpBanks[bankIndx], 0, sizeof(size_t) * DESCP_LENGTH);


	DMADescriptors* despTable = new DMADescriptors[WAVETABLE_CHANNEL_COUNT]();

	memset(_dacDescriptorsPerChannel, 0, sizeof(_dacDescriptorsPerChannel));

	_dacDescpListIndex = WAVETABLE_CHANNEL_COUNT - 1;

	for (UINT j = 0; j < WAVETABLE_CHANNEL_COUNT; ++j)
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
		despTable[j].finishPlaybackInLastGroup = dac_setting.finishPlaybackInLastGroup;
		if (FALSE == DACSetDMADescriptors(despTable[j], j, _dacDMAMultibankDescTable->descpBanks[bankIndx], DESCP_LENGTH, preDynamicLoad))
		{
			SAFE_DELETE_ARRAY(despTable);
			_dacDescriptorMutex.unlock();
			return FALSE;
		}
	}

	const UINT64 DAC_DESC_TABLE_BASE_ADDRESS = 0x50000;
	UINT64 descTableForSelectedBankAddress = DAC_DESC_TABLE_BASE_ADDRESS + bankIndx * DESCP_LENGTH * sizeof(ULONG64);
	//Only copy the descriptors used: _dacDescpListIndex + 1
	status = DoMem(WRITE_TO_CARD, BAR3, (PUINT8)(&(_dacDMAMultibankDescTable->descpBanks[bankIndx])), 0, descTableForSelectedBankAddress, (_dacDescpListIndex + 1) * sizeof(ULONG64), &DoMemStatus);

	SAFE_DELETE_ARRAY(despTable);
	_dacDescriptorMutex.unlock();
	return (THORDAQ_STATUS::STATUS_SUCCESSFUL == status);
}

THORDAQ_STATUS CThordaq::ImagingDACBanckSwitchingTrackCurrentBank()
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	BOOL currentBank = 0;

	while (!_abortReadImage && true == _imagingStartStopStatus)
	{
		if (BOOL(_usrIrqWaitStruct.DMA_Bank & 0x00000001) != currentBank)
		{
			currentBank = BOOL(_usrIrqWaitStruct.DMA_Bank & 0x00000001);

			for (int i = 0; i < _dacBankSwitchingReadyForNextWaveformCallbackSettingsList.size(); ++i)
			{
				if (_dacBankSwitchingReadyForNextWaveformCallbackSettingsList[i].callbackFunction != NULL)
				{
					_dacBankSwitchingReadyForNextWaveformCallbackSettingsList[i].callbackFunction(THORDAQ_STATUS::STATUS_SUCCESSFUL, NULL);
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}

	if (_abortReadImage)
	{
		status = STATUS_ACQUISITION_ABORTED;
	}

	return status;
}

//TODO:
//		-Use bank switching to have the correct power ramp when using fast Z. -- Done
//		-need to start thread. -- Done