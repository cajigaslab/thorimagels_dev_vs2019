// GenericFuncs.cpp : Defines most private or generic functions.

#include "stdafx.h"
#include "ThorStim.h"
#include "ThorStimSetupXML.h"

TaskHandle ThorStim::_taskHandleAOPockels = NULL;
TaskHandle ThorStim::_taskHandleDO = NULL;
TaskHandle ThorStim::_taskHandleDI = NULL;
TaskHandle ThorStim::_taskHandleCO0 = NULL;

long ThorStim::_finishedCycleCnt = 0;
long ThorStim::_triggeredCycleCnt = 0;
long ThorStim::_cycleDoneLength = 0;
long ThorStim::_precaptureStatus = PreCaptureStatus::PRECAPTURE_BLEACHER_IDLE;
long ThorStim::_dLengthPerAOCallback[SignalType::SIGNALTYPE_LAST] = {Constants::ACTIVE_LOAD_UNIT_SIZE};
uint64_t ThorStim::_totalLength[SignalType::SIGNALTYPE_LAST] = {0};
long ThorStim::_waveformTaskStatus = ICamera::STATUS_BUSY;
uint64_t ThorStim::_currentIndex[(int)(SignalType::SIGNALTYPE_LAST)] = {0};
std::wstring ThorStim::_waveformPathName = L"";
GGalvoWaveformParams ThorStim::_gGalvoWaveParams = GGalvoWaveformParams();

long ThorStim::CloseNITasks()
{
	TerminateTask(_taskHandleAOPockels);
	TerminateTask(_taskHandleDO);
	TerminateTask(_taskHandleCO0);
	TerminateTask(_taskHandleDI);
	return TRUE;
}

long ThorStim::MovePockelsToParkPosition(void)
{
	long ret = TRUE;
	float64* parkPosArray = NULL;
	if (_numPockelsLines)
	{
		string channelString = "";
		float64* parkPosArray = new float64[_numPockelsLines];
		for (int i = 0, j = 0; i < MAX_GG_POCKELS_CELL_COUNT; i++)
		{
			if(0 < _pockelsLine[i].length())
			{
				parkPosArray[j] = _pockelsMinVoltage[i];
				j++;
			}
		}
		TerminateTask(_taskHandleAOPockels);
		ret = SetVoltageToAnalogLine (_taskHandleAOPockels, _pockelsLineStr, parkPosArray);
	}
	SAFE_DELETE_ARRAY(parkPosArray);
	return ret;
}

long ThorStim::MovePockelsToPowerLevel(long index)
{
	long ret = TRUE;
	if(0 < _pockelsLine[index].length())
	{
		TerminateTask(_taskHandleAOPockels);
		float64 mArray[1] = {_pockelsMinVoltage[index] + (_pockelsMaxVoltage[index] - _pockelsMinVoltage[index])*_pockelsPowerLevel[index]};
		ret = SetVoltageToAnalogLine (_taskHandleAOPockels, _pockelsLine[index], mArray);
	}
	return ret;
}

long ThorStim::SetupProtocol()
{
	//determine digital line, set bitwise line selections
	//skip the first dummy line trigger for GalvoGalvo
	_digiLineSelect = 0x0;
	for (int i = BLEACHSCAN_DIGITAL_LINENAME::POCKEL_DIG; i < BLEACHSCAN_DIGITAL_LINENAME::DIGITAL_LINENAME_LAST; i++)
	{
		if (0 < _digiLines[i-1].length())
			_digiLineSelect |= (0x1 << i);
	}

	switch ((WaveformDriverType)_driverType)
	{
	case WaveformDriverType::WaveformDriver_NI:
		{
			MovePockelsToParkPosition();

			//BleachScan is a Task-Master mode which only use event of _hStopAcquisition and _hThreadStopped;
			//Two type of tasks: analog (pockels) and digital will be controlled by sampleRateHz.
			//user need to define: triggermode, frameCount.
			TryBuildTaskMaster();

			//could be failed when sharing Pockels with G/R at Dev1
			if (!SetupTaskMasterClock())
				return FALSE;
			if (!SetupTaskMasterPockel())
				return FALSE;

			//setup digital waveforms into DAQ; Setup Triggers: triggered cycle count be handled in CycleDone callback;
			//open bleach shutter at setup, and set cycle complementary (cycleInverse) high,
			//before set digital task due to shared NI task
			//TogglePulseToDigitalLine(_taskHandleDO, _bleachShutterLine, 1, TogglePulseMode::ToggleHigh, _bleachShutterIdle[0]);
			TogglePulseToDigitalLine(_taskHandleDO, _digiLines[ThorStimXML::NUM_WAVEFORM_ATTRIBUTES-1], 1, TogglePulseMode::ToggleHigh);
			SetupTaskMasterDigital();

			TryWriteTaskMasterPockelWaveform(FALSE);
			TryWriteTaskMasterLineWaveform(FALSE);
			SetupFrameTriggerInput();
		}
		break;
	case WaveformDriverType::WaveformDriver_ThorDAQ:
	default:
		return FALSE;
	}
	ResetEvent(ThorStim::_hStopAcquisition); 
	return TRUE;
}

long ThorStim::PostflightProtocol()
{
	switch ((WaveformDriverType)_driverType)
	{
	case WaveformDriverType::WaveformDriver_NI:
		CloseNITasks();

		MovePockelsToParkPosition();

		//set digital lines to expected state if stopped by user:
		TogglePulseToDigitalLine(_taskHandleDO, _digiLineStr, _numDigiLines, TogglePulseMode::ToggleLow);
		break;
	case WaveformDriverType::WaveformDriver_ThorDAQ:
	default:
		break;
	}
	return TRUE;
}

/// <summary> replace local parameters based on active loaded waveform files </summary>
long ThorStim::TryBuildTaskMaster(void)
{
	//calculate callback count, minimum 3000:
	const int MIN_CALLBACK_CNT = 3000;
	double callbackTime = static_cast<double>(_activeLoadMS) / Constants::MS_TO_SEC;
	for (int i = 0; i < static_cast<long>(SignalType::SIGNALTYPE_LAST); i++)
	{
		_dLengthPerAOCallback[i] = max(MIN_CALLBACK_CNT, static_cast<long>(floor(_sampleRateHz * callbackTime / Constants::ACTIVE_LOAD_UNIT_SIZE) * Constants::ACTIVE_LOAD_UNIT_SIZE));	//round to 100's samples callback
	}

	//returned total count should be the maximum available, 
	//assuming GalvoXY was not saved
	uint64_t totalDataCount = ImageWaveformBuilder->RebuildWaveformFromFile(_waveformPathName.c_str(), NULL, _digiLineSelect, _dLengthPerAOCallback);
	if(0 == totalDataCount)
	{
		StringCbPrintfW(message,_MAX_PATH,L"ImageWaveformBuilder unable to build waveform.");
		LogMessage(message,ERROR_EVENT);
		return FALSE;
	}

	//return if no change of data, so that no need
	//to restart task at cycle done callback:
	if(totalDataCount == _totalLength[SignalType::ANALOG_POCKEL])
	{
		return FALSE;
	}

	//reset data length including initial travel and final callback patch,
	//using values from waveform file:
	_totalLength[SignalType::ANALOG_POCKEL] = _totalLength[SignalType::DIGITAL_LINES] = totalDataCount;
	return TRUE;
}

/// <summary> signal done of waveform mode </summary>
long ThorStim::WaveformModeFinished(void)
{
	if(ICamera::STATUS_BUSY == ThorStim::_waveformTaskStatus)
	{
		switch ((WaveformDriverType)_driverType)
		{
		case WaveformDriverType::WaveformDriver_NI:
			SyncCustomWaveformOnOff(false);
			break;
		case WaveformDriverType::WaveformDriver_ThorDAQ:
		default:
			break;
		}
		ImageWaveformBuilder->CloseWaveformFile();
		SetEvent(_hThreadStopped);
		//TogglePulseToDigitalLine(_taskHandleDO, _bleachShutterLine, 1, TogglePulseMode::ToggleLow, _bleachShutterIdle[1]);	//close bleach shutter
		ThorStim::_waveformTaskStatus = ICamera::STATUS_READY;
	}
	return 0;
}
