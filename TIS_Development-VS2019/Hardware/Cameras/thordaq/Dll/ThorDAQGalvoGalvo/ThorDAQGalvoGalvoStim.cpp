#include "stdafx.h"
#include "ThorDAQGalvoGalvo.h"
#include "thordaqGalvoGalvoSetupXML.h"

long CThorDAQGalvoGalvo::_precaptureStatus = PreCaptureStatus::PRECAPTURE_BLEACHER_IDLE;
long CThorDAQGalvoGalvo::_bleachStatus = StatusType::STATUS_BUSY;

uint64_t CThorDAQGalvoGalvo::_currentIndex[(int)(SignalType::SIGNALTYPE_LAST)] = { 0 };
uint64_t CThorDAQGalvoGalvo::_totalLength[SignalType::SIGNALTYPE_LAST] = { 0 };
long CThorDAQGalvoGalvo::_dLengthPerDACCallback[SignalType::SIGNALTYPE_LAST] = { Constants::ACTIVE_LOAD_UNIT_SIZE };
std::wstring CThorDAQGalvoGalvo::_stimWaveformPath = L"";
ThorDAQGGWaveformParams CThorDAQGalvoGalvo::_gWaveformParams = ThorDAQGGWaveformParams();
DAC_FREERUN_WAVEFORM_CONFIG CThorDAQGalvoGalvo::_daqStimCfg = DAC_FREERUN_WAVEFORM_CONFIG();
size_t CThorDAQGalvoGalvo::_dacWaveSamples = 0;
int CThorDAQGalvoGalvo::_digiBleachSelect = 0;
UINT64 CThorDAQGalvoGalvo::_stimCompletedCycles = 0;
UINT64 CThorDAQGalvoGalvo::_stimPreLoadedCycles = 0;
bool CThorDAQGalvoGalvo::_dacWavepformPlaybackComplete = false;
bool CThorDAQGalvoGalvo::_dacWavepformPlaybackStarted = false;
long CThorDAQGalvoGalvo::_stimActiveLoadCount = 1;
std::map<UINT, USHORT> CThorDAQGalvoGalvo::_stimParkPositions;
std::atomic<bool> CThorDAQGalvoGalvo::_dacConfiguringWaveforms = false;
std::atomic<bool> CThorDAQGalvoGalvo::_dacLoadedLastWaveformSection = false;
std::atomic<bool> CThorDAQGalvoGalvo::_dacPrepareRetrigger = false;
std::atomic<bool> CThorDAQGalvoGalvo::_dacWaveformPreloaded = false;
std::atomic<bool> CThorDAQGalvoGalvo::_dacPreloadingWaveforms = false;
std::atomic<bool> CThorDAQGalvoGalvo::_dacRunning = false;
std::map<UINT, DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT> CThorDAQGalvoGalvo::_dacCtrlDynamicLoad;


long CThorDAQGalvoGalvo::SetStimDACTriggerOptions(THORDAQ_TRIGGER_MODES startTriggerMode, DAC_FREERUN_WAVEFORM_CONFIG& daqStimCfg)
{
	THORDAQ_TRIGGER_SETTINGS triggerSettings = THORDAQ_TRIGGER_SETTINGS();

	//TODO ThorDAQ 2.0: still need to work on internal digital trigger and level trigger modes
	THORDAQ_HW_TRIGGER_MODES triggerMode1 = (_digitalIOSelection[DI_StimHardwareTrigger1] >= 0) ? THORDAQ_HW_TRIGGER_MODES::THORDAQ_HARDWARE_EDGE_TRIGGER_MODE : THORDAQ_HW_TRIGGER_MODES::THORDAQ_NO_TRRIGER;
	THORDAQ_HW_TRIGGER_MODES triggerMode2 = (_digitalIOSelection[DI_StimHardwareTrigger2] >= 0) ? THORDAQ_HW_TRIGGER_MODES::THORDAQ_HARDWARE_EDGE_TRIGGER_MODE : THORDAQ_HW_TRIGGER_MODES::THORDAQ_NO_TRRIGER;
	//TODO ThorDAQ 2.0: the inter digital trigger works like a level trigger. For this to work with Stim, we would need to park the complete line high until imaging is complete (when N frames are acquired).
	triggerSettings.hwTrigger1Selection = THORDAQ_TRIGGER_INPUTS::THORDAQ_HW_TRIGGER_3;
	triggerSettings.hwTrigger2Selection = THORDAQ_TRIGGER_INPUTS::THORDAQ_HW_TRIGGER_4;
	triggerSettings.hwTrigger1Mode = triggerMode1;
	triggerSettings.hwTrigger2Mode = triggerMode2;
	triggerSettings.enableInternalDigitalTrigger = false;
	triggerSettings.logicOperand = THORDAQ_OR;

	switch (startTriggerMode)
	{
	case THORDAQ_TRIGGER_MODES::THORDAQ_SOFTWARE_START:
		triggerSettings.triggerMode = THORDAQ_TRIGGER_MODES::THORDAQ_SOFTWARE_START;

		break;
	case THORDAQ_TRIGGER_MODES::THORDAQ_HARDWARE_TRIGGER:
		triggerSettings.triggerMode = THORDAQ_TRIGGER_MODES::THORDAQ_HARDWARE_TRIGGER;
		break;
	}

	for (auto& waveformCtrl : daqStimCfg.dacCtrl)
	{
		waveformCtrl.second.triggerSettings = triggerSettings;
	}

	for (auto& waveformCtrl : daqStimCfg.dacCtrlPart2)
	{
		waveformCtrl.second.triggerSettings = triggerSettings;
	}

	return TRUE;
}

/************************************************************************************************
* @fn	long CThorDAQGalvoGalvo::ConfigDACWaveforms(ImgAcqPty* pImgAcqPty)
*
* @brief	Configure thordaq settings for Stimulus, especially the waveforms
* @param [in,out]	pImgAcqPty	  	Identifier of settings Struct.
* @return	A long.
**************************************************************************************************/
long CThorDAQGalvoGalvo::ConfigDACWaveforms(ImgAcqPty* pImgAcqPty)
{
	if (_dacConfiguringWaveforms)
	{
		return FALSE;
	}
	_dacConfiguringWaveforms = true;
	_dacPreloadingWaveforms = true;
	for (int i = 0; i < SignalType::SIGNALTYPE_LAST; i++)
	{
		_currentIndex[i] = _totalLength[i] = 0;
	}
	ImageWaveformBuilder->CloseWaveformFile();
	//reset params to force rebuild waveform:
	ImageWaveformBuilder->ResetThorDAQGGalvoWaveformParams();

	UINT64 activeLoadCount = static_cast<UINT64>(_stimActiveLoadCount); //TODO:should there be a setting and some logic to load as much as possible taking into account the memory available in the FPGA (register space)?, and also the smallest possible

	if (FALSE == ImageWaveformBuilder->GetThorDAQGGWaveformParams(_stimWaveformPath.c_str(), &_gWaveformParams, true))
	{
		_dacConfiguringWaveforms = false;
		_dacPreloadingWaveforms = false;
		return FALSE;
	}

	//check pre-capture status:
	if (1 < pImgAcqPty->numFrame)
	{
		_precaptureStatus = PreCaptureStatus::PRECAPTURE_WAVEFORM_MID_CYCLE;
	}

	_dacTriggerMode = THORDAQ_TRIGGER_MODES::THORDAQ_SOFTWARE_START;

	ULONG32 cycles = static_cast<ULONG32>(pImgAcqPty->numFrame);

	switch (pImgAcqPty->triggerMode)
	{
		case ICamera::SW_FREE_RUN_MODE:
			cycles = static_cast<ULONG32>(pImgAcqPty->numFrame);
			_dacTriggerMode = THORDAQ_TRIGGER_MODES::THORDAQ_SOFTWARE_START;
			break;
		case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
			cycles = 1;
			_dacTriggerMode = THORDAQ_TRIGGER_MODES::THORDAQ_HARDWARE_TRIGGER;
			break;
		case ICamera::SW_SINGLE_FRAME:
			cycles = 1;
			_dacTriggerMode = THORDAQ_TRIGGER_MODES::THORDAQ_SOFTWARE_START;
			break;
		case ICamera::HW_SINGLE_FRAME:
			cycles = 1;
			_dacTriggerMode = THORDAQ_TRIGGER_MODES::THORDAQ_HARDWARE_TRIGGER;
			break;
		case ICamera::SW_MULTI_FRAME:
			cycles = (MAX_FRAME_NUM <= pImgAcqPty->numFrame) ? (MAX_FRAME_NUM) : static_cast<ULONG32>(pImgAcqPty->numFrame);
			_dacTriggerMode = THORDAQ_TRIGGER_MODES::THORDAQ_SOFTWARE_START;
			break;
		case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
			cycles = (MAX_FRAME_NUM <= pImgAcqPty->numFrame) ? (MAX_FRAME_NUM) : static_cast<ULONG32>(pImgAcqPty->numFrame);
			_dacTriggerMode = THORDAQ_TRIGGER_MODES::THORDAQ_HARDWARE_TRIGGER;
			break;
	}

	_daqStimCfg.generalSettings = DAC_FREERUN_WAVEFORM_GENERAL_SETTINGS();

	_daqStimCfg.generalSettings.digitalLinesConfig = _thorDAQStimDigitalLinesConfig;

	INT32 error = 0, retVal = 0;
	UINT64 minSamples = 0;
	double dac_rate = (double)_gWaveformParams.ClockRate;

	ThordaqErrChk(L"ThorDAQAPIDACGetMinSamples", retVal = ThorDAQAPIDACGetMinSamples(_DAQDeviceIndex, dac_rate, minSamples));

	if (THORDAQ_STATUS::STATUS_SUCCESSFUL != retVal)
	{
		_dacConfiguringWaveforms = false;
		_dacPreloadingWaveforms = false;
		return FALSE;
	}

	if (minSamples < _gWaveformParams.analogXYSize && activeLoadCount * Constants::ACTIVE_LOAD_UNIT_SIZE >= _gWaveformParams.analogXYSize)
	{
		for (int i = 0; i < static_cast<long>(SignalType::SIGNALTYPE_LAST); i++)
		{
			_dLengthPerDACCallback[i] = (long)_gWaveformParams.analogXYSize;
		}
	}
	else
	{
		if (minSamples > ((UINT64)activeLoadCount * Constants::ACTIVE_LOAD_UNIT_SIZE))
		{
			for (int i = 0; i < static_cast<long>(SignalType::SIGNALTYPE_LAST); i++)
			{
				_dLengthPerDACCallback[i] = (long)minSamples;
			}
		}
		else
		{
			for (int i = 0; i < static_cast<long>(SignalType::SIGNALTYPE_LAST); i++)
			{
				_dLengthPerDACCallback[i] = (long)activeLoadCount * Constants::ACTIVE_LOAD_UNIT_SIZE;
			}
		}

	}

	bool oneCyclePerCallback = _dLengthPerDACCallback[SignalType::ANALOG_XY] >= _gWaveformParams.analogXYSize;

	//if the size is greated than the callback size then we are able to dynamically load the data
	//otherwise load all the waveform before running
	if (_gWaveformParams.analogXYSize >= _dLengthPerDACCallback[SignalType::ANALOG_XY] && (oneCyclePerCallback && cycles > 1 || !oneCyclePerCallback))
	{
		_isDynamicWaveformLoadingStim = true;
		uint64_t totalDataCount = ImageWaveformBuilder->RebuildThorDAQWaveformFromFile(_stimWaveformPath.c_str(), NULL, _digiBleachSelect, _dLengthPerDACCallback);
		if (0 == totalDataCount)
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"ImageWaveformBuilder unable to build waveform.");
			LogMessage(_errMsg, ERROR_EVENT);
			_dacConfiguringWaveforms = false;
			_dacPreloadingWaveforms = false;
			return FALSE;
		}

		if (FALSE == ImageWaveformBuilder->GetThorDAQGGWaveformParamsAndBufferWithStatus(SignalType::ANALOG_XY, &_gWaveformParams, _precaptureStatus, _currentIndex[SignalType::ANALOG_XY], true))
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"ImageWaveformBuilder unable to retrieve galvo waveform.");
			LogMessage(_errMsg, ERROR_EVENT);
			_dacConfiguringWaveforms = false;
			_dacPreloadingWaveforms = false;
			return FALSE;
		}
		size_t analogXYLength = _gWaveformParams.analogXYSize;
		//--------------Analog X galvo waveformBuffer--------------
		DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gXCtrl = DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT();
		gXCtrl.update_rate = dac_rate;
		gXCtrl.output_port = _stimActiveAOSelection[GG_AO::GG_X];
		gXCtrl.waveform_buffer_size = (analogXYLength) * sizeof(USHORT);
		gXCtrl.waveformBuffer = _gWaveformParams.GalvoWaveformX;
		gXCtrl.offset = gXCtrl.waveformBuffer[0] == 0 ? _gWaveformParams.GalvoWaveformXOffset : gXCtrl.waveformBuffer[0];
		gXCtrl.offsetTheWaveforms = gXCtrl.waveformBuffer[0] != 0;
		gXCtrl.park = gXCtrl.offset;
		gXCtrl.filterInhibit = false;

		//--------------Analog Y galvo waveformBuffer--------------
		DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gYCtrl = DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT();
		gYCtrl.update_rate = dac_rate;
		gYCtrl.output_port = _stimActiveAOSelection[GG_AO::GG_Y];
		gYCtrl.waveform_buffer_size = (analogXYLength) * sizeof(USHORT);
		gYCtrl.waveformBuffer = _gWaveformParams.GalvoWaveformY;
		gYCtrl.offset = gYCtrl.waveformBuffer[0] == 0 ? _gWaveformParams.GalvoWaveformYOffset : gYCtrl.waveformBuffer[0];
		gYCtrl.offsetTheWaveforms = gYCtrl.waveformBuffer[0] != 0;
		gYCtrl.park = gYCtrl.offset;
		gYCtrl.filterInhibit = false;

		_stimParkPositions[_stimActiveAOSelection[GG_AO::GG_X]] = gXCtrl.park;
		_stimParkPositions[_stimActiveAOSelection[GG_AO::GG_Y]] = gYCtrl.park;

		//when in HW trigger mode we want to be ready as soon as possible to start stimulation
		//for that reason we want to move to the offset position as we are preparing
		//annd we wont allow thordaq any timing to move there when the trigger is raised
		//since it will be already at the right position
		if (THORDAQ_TRIGGER_MODES::THORDAQ_HARDWARE_TRIGGER == _dacTriggerMode)
		{
			const USHORT park_mid = 0x7fff;
			double parkVal = (_stimParkPositions[_stimActiveAOSelection[GG_AO::GG_X]] - park_mid) * GALVO_RESOLUTION;
			ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, _stimActiveAOSelection[GG_AO::GG_X], parkVal));
			parkVal = (_stimParkPositions[_stimActiveAOSelection[GG_AO::GG_Y]] - park_mid) * GALVO_RESOLUTION;
			ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, _stimActiveAOSelection[GG_AO::GG_Y], parkVal));
			_daqStimCfg.generalSettings.allowTimeToMoveToOffset = false;
		}
		else
		{
			_daqStimCfg.generalSettings.allowTimeToMoveToOffset = true;
		}

		if (FALSE == ImageWaveformBuilder->GetThorDAQGGWaveformParamsAndBufferWithStatus(SignalType::ANALOG_POCKEL, &_gWaveformParams, _precaptureStatus, _currentIndex[SignalType::ANALOG_POCKEL], false))
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"ImageWaveformBuilder unable to retrieve pockels analog waveform.");
			LogMessage(_errMsg, ERROR_EVENT);
			_dacConfiguringWaveforms = false;
			_dacPreloadingWaveforms = false;
			ImageWaveformBuilder->CloseWaveformFile();
			return FALSE;
		}

		//retrieve waveform:
		if (FALSE == ImageWaveformBuilder->GetThorDAQGGWaveformParamsAndBufferWithStatus(SignalType::DIGITAL_LINES, &_gWaveformParams, _precaptureStatus, _currentIndex[SignalType::DIGITAL_LINES], false))
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"ImageWaveformBuilder unable to retrieve digital lines waveform.");
			LogMessage(_errMsg, ERROR_EVENT);
			_dacConfiguringWaveforms = false;
			_dacPreloadingWaveforms = false;
			ImageWaveformBuilder->CloseWaveformFile();
			return FALSE;
		}

		_totalLength[SignalType::ANALOG_XY] = _totalLength[SignalType::ANALOG_POCKEL] = _totalLength[SignalType::DIGITAL_LINES] = totalDataCount;

		_dacWaveSamples = analogXYLength;

		DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gP1Ctrl = DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT();
		gP1Ctrl.update_rate = dac_rate;
		gP1Ctrl.waveform_buffer_size = (_gWaveformParams.analogPockelSize) * sizeof(USHORT);
		gP1Ctrl.waveformBuffer = _gWaveformParams.GalvoWaveformPockel;
		gP1Ctrl.offset = gP1Ctrl.waveformBuffer[0] == 0 ? _gWaveformParams.GalvoWaveformPoceklsOffset : gP1Ctrl.waveformBuffer[0];
		gP1Ctrl.offsetTheWaveforms = gP1Ctrl.waveformBuffer[0] != 0;
		gP1Ctrl.park = gP1Ctrl.offset;
		gP1Ctrl.filterInhibit = true;

		DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gDigiCtrl = DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT();
		gDigiCtrl.update_rate = dac_rate;
		gDigiCtrl.output_port = WAVETABLE_CHANNEL::DIG_D0to7;
		gDigiCtrl.waveform_buffer_size = (_gWaveformParams.digitalSize) * sizeof(USHORT);
		gDigiCtrl.waveformBuffer = _gWaveformParams.DigBufWaveform;
		gDigiCtrl.offset = 0;
		gDigiCtrl.park = 0;
		gDigiCtrl.filterInhibit = true;

		_daqStimCfg.generalSettings.numberOfCycles = cycles;
		_daqStimCfg.generalSettings.numberOfSamplesPerCycle = _totalLength[SignalType::ANALOG_XY];

		_daqStimCfg.dacCtrl[_stimActiveAOSelection[GG_AO::GG_X]] = gXCtrl;
		_daqStimCfg.dacCtrl[_stimActiveAOSelection[GG_AO::GG_Y]] = gYCtrl;
		_daqStimCfg.dacCtrl[WAVETABLE_CHANNEL::DIG_D0to7] = gDigiCtrl;

		for (int pockelsIndex = 0; pockelsIndex < MAX_POCKELS_CELL_COUNT; pockelsIndex++)
		{
			bool pockelsEnable = _stimActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + pockelsIndex)] >= 0 ? _pockelsEnable[pockelsIndex] : FALSE;

			if (pockelsEnable == TRUE)
			{
				long pockelOutputChannel = _stimActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + pockelsIndex)];
				gP1Ctrl.output_port = pockelOutputChannel;
				_daqStimCfg.dacCtrl[pockelOutputChannel] = gP1Ctrl;
				break;
			}
		}

		_stimParkPositions[_stimActiveAOSelection[GG_AO::Pockels0]] = gP1Ctrl.park;

		SetStimDACTriggerOptions(_dacTriggerMode, _daqStimCfg);

		ThordaqErrChk(L"ThorDAQAPIDACSetWaveformConfigurationForDynamicLoad", retVal = ThorDAQAPIDACSetWaveformConfigurationForDynamicLoad(_DAQDeviceIndex, _daqStimCfg));

		if (retVal != THORDAQ_STATUS::STATUS_SUCCESSFUL)
		{
			_dacConfiguringWaveforms = false;
			_dacPreloadingWaveforms = false;
			ImageWaveformBuilder->CloseWaveformFile();
			return FALSE;
		}

		ThordaqErrChk(L"ThorDAQAPIDACRegisterApproachingLoadedWaveformEndEvent", retVal = ThorDAQAPIDACRegisterApproachingLoadedWaveformEndEvent(_DAQDeviceIndex, (UINT8)_stimActiveAOSelection[GG_AO::GG_X], 0, DACApproachingLoadedWaveformEndCallback, NULL));
		ThordaqErrChk(L"ThorDAQAPIDACRegisterCycleDoneEvent", retVal = ThorDAQAPIDACRegisterCycleDoneEvent(_DAQDeviceIndex, (UINT8)_stimActiveAOSelection[GG_AO::GG_X], 0, DACycleDoneCallback, NULL));
		_dacCtrlDynamicLoad = _daqStimCfg.dacCtrl;
		//done here with dynamic load section, set flags accordingly
		_dacLoadedLastWaveformSection = false;
	}
	else
	{
		_isDynamicWaveformLoadingStim = false;
		for (int i = 0; i < static_cast<long>(SignalType::SIGNALTYPE_LAST); i++)
		{
			_dLengthPerDACCallback[i] = (long)_gWaveformParams.analogXYSize;
		}
		uint64_t totalDataCount = ImageWaveformBuilder->RebuildThorDAQWaveformFromFile(_stimWaveformPath.c_str(), NULL, _digiBleachSelect, _dLengthPerDACCallback);
		if (0 == totalDataCount)
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"ImageWaveformBuilder unable to build waveform.");
			LogMessage(_errMsg, ERROR_EVENT);
			_dacConfiguringWaveforms = false;
			_dacPreloadingWaveforms = false;
			ImageWaveformBuilder->CloseWaveformFile();
			return FALSE;
		}

		if (FALSE == ImageWaveformBuilder->GetThorDAQGGWaveformParamsAndBufferWithStatus(SignalType::ANALOG_XY, &_gWaveformParams, _precaptureStatus, _currentIndex[SignalType::ANALOG_XY], true))
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"ImageWaveformBuilder unable to retrieve galvo waveform.");
			LogMessage(_errMsg, ERROR_EVENT);
			_dacConfiguringWaveforms = false;
			_dacPreloadingWaveforms = false;
			ImageWaveformBuilder->CloseWaveformFile();
			return FALSE;
		}

		if (FALSE == ImageWaveformBuilder->GetThorDAQGGWaveformParamsAndBufferWithStatus(SignalType::ANALOG_POCKEL, &_gWaveformParams, _precaptureStatus, _currentIndex[SignalType::ANALOG_POCKEL], false))
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"ImageWaveformBuilder unable to retrieve pockels analog waveform.");
			LogMessage(_errMsg, ERROR_EVENT);
			_dacConfiguringWaveforms = false;
			_dacPreloadingWaveforms = false;
			ImageWaveformBuilder->CloseWaveformFile();
			return FALSE;
		}

		size_t analogXYLength = _gWaveformParams.analogXYSize;

		//--------------Analog X galvo waveformBuffer--------------
		DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gXCtrl1 = DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT();
		gXCtrl1.update_rate = dac_rate;
		gXCtrl1.output_port = _stimActiveAOSelection[GG_AO::GG_X];
		gXCtrl1.waveform_buffer_size = (analogXYLength) * sizeof(USHORT);
		gXCtrl1.waveformBuffer = _gWaveformParams.GalvoWaveformX;
		gXCtrl1.offset = gXCtrl1.waveformBuffer[0] == 0 ? _gWaveformParams.GalvoWaveformXOffset : gXCtrl1.waveformBuffer[0];
		gXCtrl1.offsetTheWaveforms = gXCtrl1.waveformBuffer[0] != 0;
		gXCtrl1.park = gXCtrl1.offset;
		gXCtrl1.filterInhibit = false;

		//--------------Analog Y galvo waveformBuffer--------------
		DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gYCtrl1 = DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT();
		gYCtrl1.update_rate = dac_rate;
		gYCtrl1.output_port = _stimActiveAOSelection[GG_AO::GG_Y];
		gYCtrl1.waveform_buffer_size = (analogXYLength) * sizeof(USHORT);
		gYCtrl1.waveformBuffer = _gWaveformParams.GalvoWaveformY;
		gYCtrl1.offset = gYCtrl1.waveformBuffer[0] == 0 ? _gWaveformParams.GalvoWaveformYOffset : gYCtrl1.waveformBuffer[0];
		gYCtrl1.offsetTheWaveforms = gYCtrl1.waveformBuffer[0] != 0;
		gYCtrl1.park = gYCtrl1.offset;
		gYCtrl1.filterInhibit = false;

		DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gP1Ctrl1 = DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT();
		gP1Ctrl1.update_rate = dac_rate;
		gP1Ctrl1.waveform_buffer_size = (_gWaveformParams.analogPockelSize) * sizeof(USHORT);
		gP1Ctrl1.waveformBuffer = _gWaveformParams.GalvoWaveformPockel;
		gP1Ctrl1.offset = gP1Ctrl1.waveformBuffer[0] == 0 ? _gWaveformParams.GalvoWaveformPoceklsOffset : gP1Ctrl1.waveformBuffer[0];
		gP1Ctrl1.offsetTheWaveforms = gP1Ctrl1.waveformBuffer[0] != 0;
		gP1Ctrl1.park = gP1Ctrl1.offset;
		gP1Ctrl1.filterInhibit = true;

		_stimParkPositions[_stimActiveAOSelection[GG_AO::GG_X]] = gXCtrl1.park;
		_stimParkPositions[_stimActiveAOSelection[GG_AO::GG_Y]] = gYCtrl1.park;

		//when in HW trigger mode we want to be ready as soon as possible to start stimulation
		//for that reason we want to move to the offset position as we are preparing
		//annd we wont allow thordaq any timing to move there when the trigger is raised
		//since it will be already at the right position
		if (THORDAQ_TRIGGER_MODES::THORDAQ_HARDWARE_TRIGGER == _dacTriggerMode)
		{
			const USHORT park_mid = 0x7fff;
			double parkVal = (_stimParkPositions[_stimActiveAOSelection[GG_AO::GG_X]] - park_mid) * GALVO_RESOLUTION;
			ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, _stimActiveAOSelection[GG_AO::GG_X], parkVal));
			parkVal = (_stimParkPositions[_stimActiveAOSelection[GG_AO::GG_Y]] - park_mid) * GALVO_RESOLUTION;
			ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, _stimActiveAOSelection[GG_AO::GG_Y], parkVal));
			_daqStimCfg.generalSettings.allowTimeToMoveToOffset = false;
		}
		else
		{
			_daqStimCfg.generalSettings.allowTimeToMoveToOffset = true;
		}

		_dacWaveSamples = analogXYLength;

		if (FALSE == ImageWaveformBuilder->GetThorDAQGGWaveformParamsAndBufferWithStatus(SignalType::DIGITAL_LINES, &_gWaveformParams, _precaptureStatus, _currentIndex[SignalType::DIGITAL_LINES], false))
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"ImageWaveformBuilder unable to retrieve digital lines waveform.");
			LogMessage(_errMsg, ERROR_EVENT);
			_dacConfiguringWaveforms = false;
			_dacPreloadingWaveforms = false;
			ImageWaveformBuilder->CloseWaveformFile();
			return FALSE;
		}

		USHORT* digiLinesBuffer = new USHORT[_gWaveformParams.digitalSize]();
		memcpy(digiLinesBuffer, _gWaveformParams.DigBufWaveform, (_gWaveformParams.digitalSize) * sizeof(USHORT));

		_totalLength[SignalType::ANALOG_XY] = _totalLength[SignalType::ANALOG_POCKEL] = _totalLength[SignalType::DIGITAL_LINES] = totalDataCount;

		_stimParkPositions[_stimActiveAOSelection[GG_AO::Pockels0]] = gP1Ctrl1.park;

		DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gDigiCtrl1 = DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT();
		gDigiCtrl1.update_rate = dac_rate;
		gDigiCtrl1.output_port = WAVETABLE_CHANNEL::DIG_D0to7;
		gDigiCtrl1.waveform_buffer_size = (_gWaveformParams.digitalSize) * sizeof(USHORT);
		gDigiCtrl1.waveformBuffer = digiLinesBuffer;
		gDigiCtrl1.offset = 0;
		gDigiCtrl1.park = 0;
		gDigiCtrl1.filterInhibit = true;

		_daqStimCfg.generalSettings.numberOfCycles = cycles;
		_daqStimCfg.generalSettings.numberOfSamplesPerCycle = _totalLength[SignalType::ANALOG_XY];

		totalDataCount = ImageWaveformBuilder->RebuildThorDAQWaveformFromFile(_stimWaveformPath.c_str(), NULL, _digiBleachSelect, _dLengthPerDACCallback);

		if ((_stimCompletedCycles < (pImgAcqPty->numFrame - 1)) || (ICamera::HW_MULTI_FRAME_TRIGGER_EACH != pImgAcqPty->triggerMode))
		{
			_precaptureStatus = PreCaptureStatus::PRECAPTURE_WAVEFORM_LAST_CYCLE;
		}

		if (FALSE == ImageWaveformBuilder->GetThorDAQGGWaveformParamsAndBufferWithStatus(SignalType::ANALOG_XY, &_gWaveformParams, _precaptureStatus, _currentIndex[SignalType::ANALOG_XY], false))
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"ImageWaveformBuilder unable to retrieve galvo waveform.");
			LogMessage(_errMsg, ERROR_EVENT);
			_dacConfiguringWaveforms = false;
			_dacPreloadingWaveforms = false;
			SAFE_DELETE_ARRAY(digiLinesBuffer);
			ImageWaveformBuilder->CloseWaveformFile();
			return FALSE;
		}

		if (FALSE == ImageWaveformBuilder->GetThorDAQGGWaveformParamsAndBufferWithStatus(SignalType::ANALOG_POCKEL, &_gWaveformParams, _precaptureStatus, _currentIndex[SignalType::ANALOG_POCKEL], false))
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"ImageWaveformBuilder unable to retrieve pockels analog waveform.");
			LogMessage(_errMsg, ERROR_EVENT);
			_dacConfiguringWaveforms = false;
			_dacPreloadingWaveforms = false;
			SAFE_DELETE_ARRAY(digiLinesBuffer);
			ImageWaveformBuilder->CloseWaveformFile();
			return FALSE;
		}

		//retrieve waveform:
		if (FALSE == ImageWaveformBuilder->GetThorDAQGGWaveformParamsAndBufferWithStatus(SignalType::DIGITAL_LINES, &_gWaveformParams, _precaptureStatus, _currentIndex[SignalType::DIGITAL_LINES], false))
		{
			StringCbPrintfW(_errMsg, _MAX_PATH, L"ImageWaveformBuilder unable to retrieve digital lines waveform.");
			LogMessage(_errMsg, ERROR_EVENT);
			_dacConfiguringWaveforms = false;
			_dacPreloadingWaveforms = false;
			SAFE_DELETE_ARRAY(digiLinesBuffer);
			ImageWaveformBuilder->CloseWaveformFile();
			return FALSE;
		}

		//--------------Analog X galvo waveformBuffer--------------
		DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gXCtrl2 = DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT();
		gXCtrl2.update_rate = dac_rate;
		gXCtrl2.output_port = _stimActiveAOSelection[GG_AO::GG_X];
		gXCtrl2.waveform_buffer_size = (analogXYLength) * sizeof(USHORT);
		gXCtrl2.waveformBuffer = _gWaveformParams.GalvoWaveformX;
		gXCtrl2.offset = gXCtrl2.waveformBuffer[0] == 0 ? _gWaveformParams.GalvoWaveformXOffset : gXCtrl2.waveformBuffer[0];
		gXCtrl2.offsetTheWaveforms = gXCtrl2.waveformBuffer[0] != 0;
		gXCtrl2.park = gXCtrl2.offset;
		gXCtrl2.filterInhibit = false;

		//--------------Analog Y galvo waveformBuffer--------------
		DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gYCtrl2 = DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT();
		gYCtrl2.update_rate = dac_rate;
		gYCtrl2.output_port = _stimActiveAOSelection[GG_AO::GG_Y];
		gYCtrl2.waveform_buffer_size = (analogXYLength) * sizeof(USHORT);
		gYCtrl2.waveformBuffer = _gWaveformParams.GalvoWaveformY;
		gYCtrl2.offset = gYCtrl2.waveformBuffer[0] == 0 ? _gWaveformParams.GalvoWaveformYOffset : gYCtrl2.waveformBuffer[0];
		gYCtrl2.offsetTheWaveforms = gYCtrl2.waveformBuffer[0] != 0;
		gYCtrl2.park = gYCtrl2.offset;
		gYCtrl2.filterInhibit = false;

		_dacWaveSamples = analogXYLength;

		DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gP1Ctrl2 = DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT();
		gP1Ctrl2.update_rate = dac_rate;
		gP1Ctrl2.waveform_buffer_size = (_gWaveformParams.analogPockelSize) * sizeof(USHORT);
		gP1Ctrl2.waveformBuffer = _gWaveformParams.GalvoWaveformPockel;
		gP1Ctrl2.offset = gP1Ctrl2.waveformBuffer[0] == 0 ? _gWaveformParams.GalvoWaveformPoceklsOffset : gP1Ctrl2.waveformBuffer[0];
		gP1Ctrl2.offsetTheWaveforms = gP1Ctrl2.waveformBuffer[0] != 0;
		gP1Ctrl2.park = gP1Ctrl2.offset;
		gP1Ctrl2.filterInhibit = true;

		DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gDigiCtrl2 = DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT();
		gDigiCtrl2.update_rate = dac_rate;
		gDigiCtrl2.output_port = WAVETABLE_CHANNEL::DIG_D0to7;
		gDigiCtrl2.waveform_buffer_size = (_gWaveformParams.digitalSize) * sizeof(USHORT);
		gDigiCtrl2.waveformBuffer = _gWaveformParams.DigBufWaveform;
		gDigiCtrl2.offset = 0;
		gDigiCtrl2.park = 0;
		gDigiCtrl2.filterInhibit = true;

		_daqStimCfg.generalSettings.numberOfCycles = cycles;
		_daqStimCfg.generalSettings.numberOfSamplesPerCycle = _totalLength[SignalType::ANALOG_XY];

		_daqStimCfg.dacCtrl[_stimActiveAOSelection[GG_AO::GG_X]] = gXCtrl2;
		_daqStimCfg.dacCtrl[_stimActiveAOSelection[GG_AO::GG_Y]] = gYCtrl2;
		_daqStimCfg.dacCtrl[WAVETABLE_CHANNEL::DIG_D0to7] = gDigiCtrl1;


		for (int pockelsIndex = 0; pockelsIndex < MAX_POCKELS_CELL_COUNT; pockelsIndex++)
		{
			bool pockelsEnable = _stimActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + pockelsIndex)] >= 0 ? _pockelsEnable[pockelsIndex] : FALSE;
			if (pockelsEnable == TRUE)
			{
				long pockelOutputChannel = _stimActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + pockelsIndex)];
				gP1Ctrl2.output_port = pockelOutputChannel;
				_daqStimCfg.dacCtrl[pockelOutputChannel] = gP1Ctrl2;
				break;
			}
		}

		_daqStimCfg.dacCtrlPart2[_stimActiveAOSelection[GG_AO::GG_X]] = gXCtrl2;
		_daqStimCfg.dacCtrlPart2[_stimActiveAOSelection[GG_AO::GG_Y]] = gYCtrl2;
		_daqStimCfg.dacCtrlPart2[WAVETABLE_CHANNEL::DIG_D0to7] = gDigiCtrl2;

		for (int pockelsIndex = 0; pockelsIndex < MAX_POCKELS_CELL_COUNT; pockelsIndex++)
		{
			bool pockelsEnable = _stimActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + pockelsIndex)] >= 0 ? _pockelsEnable[pockelsIndex] : FALSE;
			if (pockelsEnable == TRUE)
			{
				long pockelOutputChannel = _stimActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + pockelsIndex)];
				gP1Ctrl2.output_port = pockelOutputChannel;
				_daqStimCfg.dacCtrlPart2[pockelOutputChannel] = gP1Ctrl2;
				break;
			}
		}

		SetStimDACTriggerOptions(_dacTriggerMode, _daqStimCfg);

		ThordaqErrChk(L"ThorDAQAPISetConfigurationWithFullwWaveform", retVal = ThorDAQAPISetConfigurationWithFullwWaveform(_DAQDeviceIndex, _daqStimCfg));

		if (retVal != THORDAQ_STATUS::STATUS_SUCCESSFUL)
		{
			_dacConfiguringWaveforms = false;
			_dacPreloadingWaveforms = false;
			SAFE_DELETE_ARRAY(digiLinesBuffer);
			ImageWaveformBuilder->CloseWaveformFile();
			return FALSE;
		}
		SAFE_DELETE_ARRAY(digiLinesBuffer);
		ThordaqErrChk(L"ThorDAQAPIDACRegisterWaveformPlaybackCompleteEvent", retVal = ThorDAQAPIDACRegisterWaveformPlaybackCompleteEvent(_DAQDeviceIndex, 0, DACWavefomPlaybackCompleteCallback, NULL));
		ImageWaveformBuilder->CloseWaveformFile();
	}

	//done here, set the flags accordingly
	_dacConfiguringWaveforms = false;
	_dacPreloadingWaveforms = false;

	_stimPreLoadedCycles = 0;

	//if the current index (preloaded waveform) is the same as the total length of the waveform, we have already preloaded one cycle
	if (_currentIndex[SignalType::ANALOG_XY] == _totalLength[SignalType::ANALOG_XY])
	{
		++_stimPreLoadedCycles;
	}

	return TRUE;
}

/************************************************************************************************
* @fn	long CThorDAQGalvoGalvo::ConfigStimSettings(ImgAcqPty* pImgAcqPty)
*
* @brief	Configure thordaq settings for Stimulation
* @param [in,out]	pImgAcqPty	  	Identifier of settings Struct.
* @return	A long.
**************************************************************************************************/
long CThorDAQGalvoGalvo::ConfigStimSettings(ImgAcqPty* pImgAcqPty)
{
	_digiBleachSelect = 0x1;
	//no memory copy in active loading, only set bitwise line selections:

	_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::POCKEL_DIG);

	_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::ACTIVE_ENVELOPE);

	_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::CYCLE_COMPLETE);

	_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::CYCLE_ENVELOPE);

	_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::ITERATION_ENVELOPE);

	_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::PATTERN_TRIGGER);

	_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::PATTERN_COMPLETE);

	_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::EPOCH_ENVELOPE);

	_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::CYCLE_COMPLEMENTARY);

	_daqStimCfg.dacCtrl.clear();

	_daqStimCfg.dacCtrlPart2.clear();

	//initialize the struct
	_daqStimCfg = DAC_FREERUN_WAVEFORM_CONFIG();
	_stimCompletedCycles = 0;
	//reset params:
	_precaptureStatus = PreCaptureStatus::PRECAPTURE_DONE;
	_stimParkPositions.clear();

	//force trigger first if single frame in trigger each:
	//TODO: see if this logic makes sense for this GG
	if ((1 == pImgAcqPty->numFrame) && (ICamera::HW_MULTI_FRAME_TRIGGER_EACH == pImgAcqPty->triggerMode))
	{
		pImgAcqPty->triggerMode = ICamera::HW_MULTI_FRAME_TRIGGER_FIRST;
	}	

	_dacWavepformPlaybackComplete = false;
	_dacWavepformPlaybackStarted = false;
	_dacPrepareRetrigger = false;
	_bleachStatus = StatusType::STATUS_BUSY;
	_dacConfiguringWaveforms = false;

	ConfigDACWaveforms(pImgAcqPty);

	INT32 error = 0, retVal = 0;

	ThordaqErrChk(L"ThorDAQAPIDACRegisterWaveformPlaybackStartedEvent", retVal = ThorDAQAPIDACRegisterWaveformPlaybackStartedEvent(_DAQDeviceIndex, 0, DACWavefomPlaybackStartedCallback, NULL));

	return TRUE;
}

/************************************************************************************************
* @fn	UINT CThorDAQGalvoGalvo::StimProcess(LPVOID instance)
*
* @brief	Start  Stim Thread.
* @param 	instance	  	Stim Thread instance.
* @return	A uint.
**************************************************************************************************/
UINT CThorDAQGalvoGalvo::StimProcess(LPVOID instance)
{
	_bleachStatus = StatusType::STATUS_BUSY;
	THORDAQ_STATUS	status = STATUS_SUCCESSFUL;
	UINT64			numberOfcycles = _imgAcqPty_Pre.numFrame;
	double          regular_timeout = 10000 * _dacWaveSamples / _daqStimCfg.dacCtrl[CThorDAQGalvoGalvo::GetInstance()->_stimActiveAOSelection[GG_AO::GG_X]].update_rate;
	double			hardware_trigger_timeout = (double)_triggerWaitTimeout * MS_TO_SEC;
	BOOL			is_hardware_captured = FALSE;
	BOOL			hardware_timeout_enable = (THORDAQ_TRIGGER_MODES::THORDAQ_HARDWARE_TRIGGER == CThorDAQGalvoGalvo::GetInstance()->_dacTriggerMode) ? TRUE : FALSE;
	double			timeout = (hardware_timeout_enable && !is_hardware_captured) ? hardware_trigger_timeout : regular_timeout;
	UINT64 cycleCount = 0;
	int32 error = 0, retVal = 0;
	bool errorLoading = false;
	CThorDAQGalvoGalvo::GetInstance()->_stopStimulating = false;
	CThorDAQGalvoGalvo::GetInstance()->_currentlyStimulating = true;
	CThorDAQGalvoGalvo::GetInstance()->LogMessage(L"thordaqGalvoGalvo::StimProcess thread starting", VERBOSE_EVENT);

	// START with "global scan" Setting...
	ThordaqErrChk(L"ThorDAQAPIStartDACWaveforms", retVal = ThorDAQAPIStartDACWaveforms(_DAQDeviceIndex));

	_dacRunning = true;
	if (CThorDAQGalvoGalvo::GetInstance()->_isDynamicWaveformLoadingStim)
	{
		DACPreloadNextWaveformSection();
	}
	// start the loopEnable which exits on user application (stop acquisition) command
	do
	{
		if (hardware_timeout_enable && _dacWavepformPlaybackStarted)
		{
			is_hardware_captured = TRUE;
			hardware_timeout_enable = FALSE;

			// Notify WaitForHardwareTrigger the board did receive a hardware trigger 
			SetEvent(_hHardwareTriggerInEvent);
		}
		if (_dacPrepareRetrigger)
		{
			_dacPrepareRetrigger = false;
			if (DACPrepareForRetrigger() == FALSE)
			{
				errorLoading = true;
				break;
			}
		}
		//std::this_thread::sleep_for(std::chrono::microseconds(50));
	} while ((_stimCompletedCycles < (numberOfcycles)) && (WaitForSingleObject(_hStopAcquisition, 0) != WAIT_OBJECT_0) && false == _dacWavepformPlaybackComplete);

	ThorDAQAPIStopDACWaveforms(_DAQDeviceIndex);
	if (hardware_timeout_enable && _dacWavepformPlaybackStarted)
	{
		is_hardware_captured = TRUE;
		hardware_timeout_enable = FALSE;

		// Notify WaitForHardwareTrigger the board did receive a hardware trigger 
		SetEvent(_hHardwareTriggerInEvent);
	}
	_dacRunning = false;
	CThorDAQGalvoGalvo::GetInstance()->_currentlyStimulating = false;
	CThorDAQGalvoGalvo::GetInstance()->_stopStimulating = true;
	if (errorLoading)
	{
		CThorDAQGalvoGalvo::GetInstance()->LogMessage(L"thordaqGalvoGalvo::StimProcess Error loading waveform", ERROR_EVENT);
		_bleachStatus = StatusType::STATUS_ERROR;
	}
	else
	{
		_bleachStatus = StatusType::STATUS_READY;
	}
	CThorDAQGalvoGalvo::GetInstance()->LogMessage(L"thordaqGalvoGalvo::StimProcess thread exiting", VERBOSE_EVENT);
	ImageWaveformBuilder->CloseWaveformFile();
	SetEvent(_hThreadStopped);

	return TRUE;
}

inline long CThorDAQGalvoGalvo::DACPrepareForRetrigger()
{
	int32 error = 0, retVal = 0;
	const USHORT park_mid = 0x7fff;

	//TODO: remove below code when trigger each is working
	//ThordaqErrChk(L"ThorDAQAPIAbortRead", retVal = ThorDAQAPIAbortRead(_DAQDeviceIndex));

	ThorDAQAPIStopDACWaveforms(_DAQDeviceIndex);

	if (FALSE == CThorDAQGalvoGalvo::GetInstance()->ConfigDACWaveforms(&_imgAcqPty_Pre))
	{
		return FALSE;
	}

	ThordaqErrChk(L"ThorDAQAPIStartDACWaveforms", retVal = ThorDAQAPIStartDACWaveforms(_DAQDeviceIndex));

	if (CThorDAQGalvoGalvo::GetInstance()->_isDynamicWaveformLoadingStim)
	{
		DACPreloadNextWaveformSection();
	}
	return (THORDAQ_STATUS::STATUS_SUCCESSFUL == retVal);
}

inline long CThorDAQGalvoGalvo::DACPreloadNextWaveformSection()
{
	//if we are already loading a the waveform return
	//this shouldn't happen, but check here just in case
	if (_dacPreloadingWaveforms)
	{
		return TRUE;
	}

	_dacPreloadingWaveforms = true;

	//if we are in the last section of the last cycle then set the precapture status to PRECAPTURE_WAVEFORM_LAST_CYCLE
	//if the current index is equal to the total length and there are two callbacks per cycle then we need to set this one cycle before
	if ((_stimPreLoadedCycles == _imgAcqPty_Pre.numFrame - 1) && (
		(_currentIndex[SignalType::ANALOG_XY] + _dLengthPerDACCallback[SignalType::ANALOG_XY] == _totalLength[SignalType::ANALOG_XY]) ||
		(_dLengthPerDACCallback[SignalType::ANALOG_XY] == _totalLength[SignalType::ANALOG_XY])||
		(_currentIndex[SignalType::ANALOG_XY] == _totalLength[SignalType::ANALOG_XY]) && (_totalLength[SignalType::ANALOG_XY] / _dLengthPerDACCallback[SignalType::ANALOG_XY] == 2))
		)
	{
		_precaptureStatus = PreCaptureStatus::PRECAPTURE_WAVEFORM_LAST_CYCLE;
	}

	// when the waveform rebuild then go back to the beginning
	if (_currentIndex[SignalType::ANALOG_XY] == _totalLength[SignalType::ANALOG_XY])
	{
		uint64_t totalDataCount = ImageWaveformBuilder->RebuildThorDAQWaveformFromFile(_stimWaveformPath.c_str(), NULL, _digiBleachSelect, _dLengthPerDACCallback);

		_totalLength[SignalType::ANALOG_XY] = _totalLength[SignalType::ANALOG_POCKEL] = _totalLength[SignalType::DIGITAL_LINES] = totalDataCount;
		_currentIndex[SignalType::ANALOG_XY] = _currentIndex[SignalType::ANALOG_POCKEL] = _currentIndex[SignalType::DIGITAL_LINES] = 0;
	}

	//retrieve galvo X and Y analog waveforms
	if (FALSE == ImageWaveformBuilder->GetThorDAQGGWaveformParamsAndBufferWithStatus(SignalType::ANALOG_XY, &_gWaveformParams, _precaptureStatus, _currentIndex[SignalType::ANALOG_XY], false))
	{
		StringCbPrintfW(CThorDAQGalvoGalvo::_errMsg, _MAX_PATH, L"ImageWaveformBuilder unable to retrieve galvo X and Y waveforms.");
		LogMessage(_errMsg, ERROR_EVENT);
		_dacPreloadingWaveforms = false;
		SetEvent(_hStopAcquisition);
		return FALSE;
	}

	//retrieve pockels analog waveform
	if (FALSE == ImageWaveformBuilder->GetThorDAQGGWaveformParamsAndBufferWithStatus(SignalType::ANALOG_POCKEL, &_gWaveformParams, _precaptureStatus, _currentIndex[SignalType::ANALOG_POCKEL], false))
	{
		StringCbPrintfW(CThorDAQGalvoGalvo::_errMsg, _MAX_PATH, L"ImageWaveformBuilder unable to retrieve pockels waveform.");
		LogMessage(_errMsg, ERROR_EVENT);
		_dacPreloadingWaveforms = false;
		SetEvent(_hStopAcquisition);
		return FALSE;
	}

	////retrieve digital waveform
	if (FALSE == ImageWaveformBuilder->GetThorDAQGGWaveformParamsAndBufferWithStatus(SignalType::DIGITAL_LINES, &_gWaveformParams, _precaptureStatus, _currentIndex[SignalType::DIGITAL_LINES], false))
	{
		StringCbPrintfW(CThorDAQGalvoGalvo::_errMsg, _MAX_PATH, L"ImageWaveformBuilder unable to retrieve digital waveform.");
		LogMessage(_errMsg, ERROR_EVENT);
		_dacPreloadingWaveforms = false;
		SetEvent(_hStopAcquisition);
		return FALSE;
	}

	size_t analogXYLength = _gWaveformParams.analogXYSize;
	double dac_rate = (double)_gWaveformParams.ClockRate;

	//--------------Analog X galvo waveformBuffer--------------
	DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gXCtrl = DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT();
	gXCtrl.update_rate = dac_rate;
	gXCtrl.output_port = CThorDAQGalvoGalvo::GetInstance()->_stimActiveAOSelection[GG_AO::GG_X];
	gXCtrl.waveform_buffer_size = (analogXYLength) * sizeof(USHORT);
	gXCtrl.waveformBuffer = _gWaveformParams.GalvoWaveformX;
	gXCtrl.offset = _daqStimCfg.dacCtrl[CThorDAQGalvoGalvo::GetInstance()->_stimActiveAOSelection[GG_AO::GG_X]].offset;;
	gXCtrl.offsetTheWaveforms = _daqStimCfg.dacCtrl[CThorDAQGalvoGalvo::GetInstance()->_stimActiveAOSelection[GG_AO::GG_X]].offsetTheWaveforms;
	//--------------Analog Y galvo waveformBuffer--------------
	DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gYCtrl = DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT();
	gYCtrl.update_rate = dac_rate;
	gYCtrl.output_port = CThorDAQGalvoGalvo::GetInstance()->_stimActiveAOSelection[GG_AO::GG_Y];
	gYCtrl.waveform_buffer_size = (analogXYLength) * sizeof(USHORT);
	gYCtrl.waveformBuffer = _gWaveformParams.GalvoWaveformY;
	gYCtrl.offset = _daqStimCfg.dacCtrl[CThorDAQGalvoGalvo::GetInstance()->_stimActiveAOSelection[GG_AO::GG_Y]].offset;
	gYCtrl.offsetTheWaveforms = _daqStimCfg.dacCtrl[CThorDAQGalvoGalvo::GetInstance()->_stimActiveAOSelection[GG_AO::GG_Y]].offsetTheWaveforms;
	_dacWaveSamples = analogXYLength;

	//--------------Analog Pockels galvo waveformBuffer--------------
	DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gP1Ctrl = DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT();
	gP1Ctrl.update_rate = dac_rate;
	gP1Ctrl.waveform_buffer_size = (_gWaveformParams.analogPockelSize) * sizeof(USHORT);
	gP1Ctrl.waveformBuffer = _gWaveformParams.GalvoWaveformPockel;

	//--------------Digital Lines waveformBuffer--------------
	DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT gDigiCtrl = DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT();
	gDigiCtrl.update_rate = dac_rate;
	gDigiCtrl.output_port = WAVETABLE_CHANNEL::DIG_D0to7;
	gDigiCtrl.waveform_buffer_size = (_gWaveformParams.digitalSize) * sizeof(USHORT);
	gDigiCtrl.waveformBuffer = _gWaveformParams.DigBufWaveform;
	gDigiCtrl.offset = 0;

	_dacCtrlDynamicLoad[CThorDAQGalvoGalvo::GetInstance()->_stimActiveAOSelection[GG_AO::GG_X]] = gXCtrl;
	_dacCtrlDynamicLoad[CThorDAQGalvoGalvo::GetInstance()->_stimActiveAOSelection[GG_AO::GG_Y]] = gYCtrl;
	_dacCtrlDynamicLoad[WAVETABLE_CHANNEL::DIG_D0to7] = gDigiCtrl;

	for (int pockelsIndex = 0; pockelsIndex < MAX_POCKELS_CELL_COUNT; pockelsIndex++)
	{
		bool pockelsEnable = CThorDAQGalvoGalvo::GetInstance()->_stimActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + pockelsIndex)] >= 0 ? CThorDAQGalvoGalvo::GetInstance()->_pockelsEnable[pockelsIndex] : FALSE;
		if (pockelsEnable == TRUE)
		{
			long pockelOutputChannel = CThorDAQGalvoGalvo::GetInstance()->_stimActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + pockelsIndex)];
			gP1Ctrl.output_port = pockelOutputChannel;
			gP1Ctrl.offset = _daqStimCfg.dacCtrl[pockelOutputChannel].offset;
			gP1Ctrl.offsetTheWaveforms = _daqStimCfg.dacCtrl[pockelOutputChannel].offsetTheWaveforms;
			_dacCtrlDynamicLoad[pockelOutputChannel] = gP1Ctrl;
			break;
		}
	}

	if (!CThorDAQGalvoGalvo::GetInstance()->_stopStimulating)
	{

		bool isLastGroup = (_currentIndex[SignalType::ANALOG_XY] == _totalLength[SignalType::ANALOG_XY]);
		INT32 error = 0, retVal = 0;
		if ((PreCaptureStatus::PRECAPTURE_WAVEFORM_LAST_CYCLE == _precaptureStatus && isLastGroup) || (isLastGroup && TriggerMode::HW_MULTI_FRAME_TRIGGER_EACH == _imgAcqPty_Pre.triggerMode))
		{
			ThordaqErrChk(L"ThorDAQAPIDACPresetNextWaveformSection", retVal = ThorDAQAPIDACPresetNextWaveformSection(_DAQDeviceIndex, _dacCtrlDynamicLoad, true));
		}
		else
		{
			ThordaqErrChk(L"ThorDAQAPIDACPresetNextWaveformSection", retVal = ThorDAQAPIDACPresetNextWaveformSection(_DAQDeviceIndex, _dacCtrlDynamicLoad, false));
		}

		if (retVal != THORDAQ_STATUS::STATUS_SUCCESSFUL && retVal != THORDAQ_STATUS::STATUS_ACQUISITION_ABORTED)
		{
			SetEvent(_hStopAcquisition);

			StringCbPrintfW(_errMsg, _MAX_PATH, L"Unable to load waveform. Increase the activeLoadCount in the ThorDAQGalvoGalvoSettings.xml and try again.");
			MessageBox(NULL, _errMsg, L"Trigger Error", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
			//Log
			StringCbPrintfW(_errMsg, _MAX_PATH, L"activeLoad count too short");
			LogMessage(_errMsg, ERROR_EVENT);

			return FALSE;
		}
	}

	if (_currentIndex[SignalType::ANALOG_XY] == _totalLength[SignalType::ANALOG_XY])
	{
		++_stimPreLoadedCycles;
	}
	//waveform is preloaded, set flags accordingly
	_dacPreloadingWaveforms = false;
	_dacWaveformPreloaded = true;
	return TRUE;
}

void CTHORDAQCALLBACK CThorDAQGalvoGalvo::DACApproachingNSamplesCallback(UINT8 dacChannel, UINT32 numberSample, THORDAQ_STATUS status, void* callbackData)
{
	int x = 0;
}

inline void CTHORDAQCALLBACK CThorDAQGalvoGalvo::DACycleDoneCallback(UINT8 dacChannel, THORDAQ_STATUS status, void* callbackData)
{
	++_stimCompletedCycles;
	if (_stimCompletedCycles < _imgAcqPty_Pre.numFrame && (TriggerMode::HW_MULTI_FRAME_TRIGGER_EACH == _imgAcqPty_Pre.triggerMode))
	{
		_dacPrepareRetrigger = true;		
	}
}

void CTHORDAQCALLBACK CThorDAQGalvoGalvo::DACWavefomPlaybackCompleteCallback(THORDAQ_STATUS status, void* callbackData)
{	
	if (TriggerMode::HW_MULTI_FRAME_TRIGGER_EACH == _imgAcqPty_Pre.triggerMode)
	{
		//only when trigger mode is HW_MULTI_FRAME_TRIGGER_EACH do we get a callback here for each completed cycle
		++_stimCompletedCycles;
		if (_stimCompletedCycles < _imgAcqPty_Pre.numFrame)
		{
			_dacPrepareRetrigger = true;
		}
		else
		{
			_dacWavepformPlaybackComplete = true;
		}
	}
	else
	{
		_dacWavepformPlaybackComplete = true;
	}
}

void CTHORDAQCALLBACK CThorDAQGalvoGalvo::DACWavefomPlaybackStartedCallback(THORDAQ_STATUS status, void* callbackData)
{
	_dacWavepformPlaybackStarted = true;
}

inline void CTHORDAQCALLBACK CThorDAQGalvoGalvo::DACApproachingLoadedWaveformEndCallback(UINT8 dacChannel, THORDAQ_STATUS status, void* callbackData)
{
	bool loadAgain = (_totalLength[SignalType::ANALOG_XY] == _dLengthPerDACCallback[SignalType::ANALOG_XY] && _stimCompletedCycles < _imgAcqPty_Pre.numFrame - 1) || _totalLength[SignalType::ANALOG_XY] > _dLengthPerDACCallback[SignalType::ANALOG_XY];

	//only execute if the number of cycles are not complete and the last section hasn't been loaded
	if (_stimCompletedCycles < _imgAcqPty_Pre.numFrame && !_dacConfiguringWaveforms && !_dacLoadedLastWaveformSection && loadAgain)
	{		
		//sync loading
		while (_dacPreloadingWaveforms ||  !_dacWaveformPreloaded)
		{
			if (!_dacRunning || CThorDAQGalvoGalvo::GetInstance()->_stopStimulating)
			{
				return;
			}
			//wait while the waveforms are preloading
		}

		INT32 error = 0, retVal = 0;
		bool isLastGroup = (_currentIndex[SignalType::ANALOG_XY] == _totalLength[SignalType::ANALOG_XY]);

		if ((PreCaptureStatus::PRECAPTURE_WAVEFORM_LAST_CYCLE == _precaptureStatus && isLastGroup) || (isLastGroup && TriggerMode::HW_MULTI_FRAME_TRIGGER_EACH == _imgAcqPty_Pre.triggerMode))
		{
			ThordaqErrChk(L"ThorDAQAPIDACDynamicLoadPresetWaveform", retVal = ThorDAQAPIDACDynamicLoadPresetWaveform(_DAQDeviceIndex, true));
			_dacLoadedLastWaveformSection = true;
			_dacWaveformPreloaded = false;
		}
		else
		{
			ThordaqErrChk(L"ThorDAQAPIDACDynamicLoadPresetWaveform", retVal = ThorDAQAPIDACDynamicLoadPresetWaveform(_DAQDeviceIndex, false));
			_dacWaveformPreloaded = false;
			DACPreloadNextWaveformSection();
		}		
		if (retVal != THORDAQ_STATUS::STATUS_SUCCESSFUL && retVal != THORDAQ_STATUS::STATUS_ACQUISITION_ABORTED)
		{
			SetEvent(_hStopAcquisition);

			StringCbPrintfW(_errMsg, _MAX_PATH, L"Unable to dynamically load waveform. Please increase the activeLoadCount number in the ThorDAQGalvoGalvoSettings.xml file and try again");
			MessageBox(NULL, _errMsg, L"Trigger Error", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
			//Log
			StringCbPrintfW(_errMsg, _MAX_PATH, L"activeLoad count too short");
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}
}