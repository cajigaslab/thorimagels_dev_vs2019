// NISpecific.cpp : Defines most private NI generic functions.

#include "stdafx.h"
#include "ThorStim.h"
#include "ThorStimSetupXML.h"

wchar_t _errMsg[_MAX_PATH];	//used for message box or other not-for-print messages

///	***************************************** <summary> Task-Master related functions	</summary>	**************************************************************** ///

/// <summary> Callback function to be invoked when frame trigger in </summary>
int32 CVICALLBACK ThorStim::HWTriggerCallback(TaskHandle taskHandle, int32 signalID, void *callbackData)
{
	switch (_triggerMode)
	{
	case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
		ThorStim::_triggeredCycleCnt++;
		if(1 == ThorStim::_triggeredCycleCnt)
		{
			SetEvent(ThorStim::_hHardwareTriggerInEvent);
		}
		break;
	default:
		SetEvent(ThorStim::_hHardwareTriggerInEvent);
		TerminateTask(_taskHandleDI);
		break;
	}
	return 0;
}

/// <summary> Callback function to be invoked in Waveform mode other than HW trigger each </summary>
int32 CVICALLBACK ThorStim::CycleDoneCallback(TaskHandle taskHandle, int32 signalID, void *callbackData)
{
	int32 retVal = 0, error = 0;
	long updateCase = 0;
	ThorStim::_finishedCycleCnt++;

	//terminate if necessary:
	if((WaitForSingleObject(ThorStim::_hStopAcquisition, 0) == WAIT_OBJECT_0) || (ThorStim::_finishedCycleCnt == _frameCount))
	{
		//Done last cycle:		
		return WaveformModeFinished();
	}

	//continue in SW mode after triggered for HW trigger first mode:
	if(ICamera::HW_MULTI_FRAME_TRIGGER_FIRST == _triggerMode)
	{
		_triggerMode = ICamera::SW_MULTI_FRAME;
		updateCase = 1;
	}

	//prepare last cycle:
	if ((_frameCount - 1) == ThorStim::_finishedCycleCnt)
	{
		_precaptureStatus = PreCaptureStatus::PRECAPTURE_WAVEFORM_LAST_CYCLE;
		//update when active or complete line is configured:
		if((0 < (_digiLineSelect & (0x1 << BLEACHSCAN_DIGITAL_LINENAME::CYCLE_COMPLETE)))
			|| (0 < (_digiLineSelect & (0x1 << BLEACHSCAN_DIGITAL_LINENAME::ACTIVE_ENVELOPE))))
			updateCase = (1 != updateCase) ? 2 : 1;
	}

	switch (updateCase)
	{
	case 1:	//reset all tasks
		SyncCustomWaveformOnOff(false);
		//re-start waveform:
		TryBuildTaskMaster();

		SetupTaskMasterPockel();
		TryWriteTaskMasterPockelWaveform(TRUE);

		SetupTaskMasterDigital();
		TryWriteTaskMasterLineWaveform(TRUE);
		break;
	case 2:	//reset dig task only
		SyncCustomWaveformOnOff(false);
		//re-start waveform:
		TryBuildTaskMaster();
		SetupTaskMasterDigital();
		TryWriteTaskMasterLineWaveform(TRUE);
		break;
	case 0:
	default:
		break;
	}
	//restart next:
	return SyncCustomWaveformOnOff(true);
}

/// <summary> Callback function to be invoked for Waveform HW trigger each mode </summary>
int32 CVICALLBACK ThorStim::EveryNDigitalOutCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32 retVal = DAQmxSuccess;
	const int EARLY_INVOKE_MS = 12;

	//return if not triggered, and only handle trigger each case:
	if((_triggerMode != ICamera::HW_MULTI_FRAME_TRIGGER_EACH) || 
		(0 == ThorStim::_triggeredCycleCnt) ||
		(ThorStim::_waveformTaskStatus != ICamera::STATUS_BUSY) ||
		(ThorStim::_triggeredCycleCnt == ThorStim::_finishedCycleCnt))
		return retVal;

	//increment of finished cycle count:
	if(ThorStim::_triggeredCycleCnt > ThorStim::_finishedCycleCnt)
	{
		ThorStim::_finishedCycleCnt++;
	}

	//terminate if necessary:
	if((WaitForSingleObject(ThorStim::_hStopAcquisition, 0) == WAIT_OBJECT_0) || (ThorStim::_finishedCycleCnt ==_frameCount))
	{
		//callback is invoked earlier than the task finishes,
		//could be further triggered if sleep too long:
		Sleep(EARLY_INVOKE_MS);
		//Done last cycle:		
		return WaveformModeFinished();
	}

	return retVal;
}

/// <summary> Callback function to be invoked for waveform active load mode </summary>
int32 CVICALLBACK ThorStim::EveryNPockelOutCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	//terminate if necessary:
	if((WaitForSingleObject(ThorStim::_hStopAcquisition, 0) == WAIT_OBJECT_0) || (ThorStim::_finishedCycleCnt == _frameCount))
	{
		//Done active load:
		return WaveformModeFinished();
	}

	//get and execute next piece of waveform,
	//digital lines and galvo share the same clock:
	TryWriteTaskMasterLineWaveform(TRUE);
	return TryWriteTaskMasterPockelWaveform(TRUE);
}

/// <summary> setup clock task for stimulation </summary>
long ThorStim::SetupTaskMasterClock(void)
{
	long retVal = FALSE;
	BoardInfo* bInfo = boardInfoNI.get()->getInstance()->GetBoardInfo(GetDevIDName(_counter));
	char cID = _counter.at(_counter.length()-1);
	retVal = TryTaskMasterClockwCounter(atoi(&cID));

	if (!retVal && NULL != bInfo)
	{
		for (char i = 0; i < bInfo->counterCount; i++)
		{
			if (TryTaskMasterClockwCounter(i))
			{
				_counter = "/" + bInfo->devName + "/ctr" + i;
				retVal = TRUE;
				break;
			}
		}
	}
	return retVal;
}

/// <summary> setup Pockel task for stimulation </summary>
long ThorStim::SetupTaskMasterPockel(void)
{
	int32 retVal = DAQmxSuccess, error = 0;

	if (!_numPockelsLines)
		return FALSE;

	TerminateTask(_taskHandleAOPockels);

	try
	{
		//support USB board control on Pockels cell
		int32 dataXferType = DAQmx_Val_DMA;
		BoardInfo* bInfo = boardInfoNI.get()->getInstance()->GetBoardInfo(GetDevIDName(_pockelsLineStr));
		if(NULL != bInfo)
		{
			dataXferType = (BoardStyle::USB == bInfo->boardStyle) ? DAQmx_Val_USBbulk : DAQmx_Val_DMA;
		}

		std::string controllerInternalOutput = "/" + GetDevIDName(_counter) + "/Ctr" + _counter.at(_counter.length()-1) + "InternalOutput";

		DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleAOPockels));

		DAQmxErrChk(L"DAQmxCreateAOVoltageChan",retVal = DAQmxCreateAOVoltageChan(_taskHandleAOPockels, _pockelsLineStr.c_str() ,"", MIN_AO_VOLTAGE,MAX_AO_VOLTAGE, DAQmx_Val_Volts, NULL));

		if(FALSE == ImageWaveformBuilder->GetGGalvoWaveformParamsWithStatus(SignalType::ANALOG_POCKEL,&_gGalvoWaveParams, _precaptureStatus, _currentIndex[SignalType::ANALOG_POCKEL]))
		{
			StringCbPrintfW(message,_MAX_PATH,L"ImageWaveformBuilder unable to retrieve galvo waveform.");
			LogMessage(message,ERROR_EVENT);
		}
		if(_dLengthPerAOCallback[SignalType::ANALOG_POCKEL] < _totalLength[SignalType::ANALOG_POCKEL])
		{
			DAQmxErrChk (L"DAQmxSetWriteAttribute",retVal = DAQmxSetWriteAttribute (_taskHandleAOPockels, DAQmx_Write_RegenMode, DAQmx_Val_DoNotAllowRegen));
			DAQmxErrChk (L"DAQmxRegisterEveryNSamplesEvent",retVal = DAQmxRegisterEveryNSamplesEvent(_taskHandleAOPockels, DAQmx_Val_Transferred_From_Buffer, static_cast<int32>(_dLengthPerAOCallback[SignalType::ANALOG_POCKEL]), 0, EveryNPockelOutCallback, NULL));
		}			
		DAQmxErrChk(L"DAQmxRegisterDoneEvent",retVal = DAQmxRegisterDoneEvent(_taskHandleAOPockels,0,ThorStim::CycleDoneCallback,NULL));

		DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(_taskHandleAOPockels, controllerInternalOutput.c_str(), static_cast<float64>(_gGalvoWaveParams.ClockRate), DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, _totalLength[SignalType::ANALOG_POCKEL]));

		DAQmxErrChk (L"DAQmxSetAODataXferMech", retVal = DAQmxSetAODataXferMech(_taskHandleAOPockels,"",dataXferType));

		DAQmxErrChk (L"DAQmxCfgOutputBuffer",retVal = DAQmxCfgOutputBuffer(_taskHandleAOPockels,static_cast<uInt32>(min(_totalLength[SignalType::ANALOG_POCKEL], _dLengthPerAOCallback[SignalType::ANALOG_POCKEL] * _activeLoadCount))));

		//AO cannot be armStarted:
		SetFrameInTriggerableTask(_taskHandleAOPockels, FALSE);
		DAQmxErrChk(L"DAQmxWriteAnalogF64",retVal = DAQmxWriteAnalogF64(_taskHandleAOPockels, static_cast<int32>(_dLengthPerAOCallback[SignalType::ANALOG_POCKEL]), false, -1, DAQmx_Val_GroupByChannel, _gGalvoWaveParams.GalvoWaveformPockel, NULL, NULL));
		DAQmxErrChk(L"DAQmxTaskControl",retVal = DAQmxTaskControl(_taskHandleAOPockels,DAQmx_Val_Task_Reserve));
		DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleAOPockels));
	}
	catch(...)
	{
#ifdef _DEBUG
		DAQmxGetExtendedErrorInfo(logMsg,_MAX_PATH);
		LogMessage((wchar_t*)StringToWString(std::string(logMsg)).c_str(), ERROR_EVENT);
#endif
		DAQmxFailed(error);
		StringCbPrintfW(message,_MAX_PATH,L"%hs failed: (%d)",__FUNCTION__, error);
		LogMessage(message,ERROR_EVENT);
		TerminateTask(_taskHandleAOPockels);
		return FALSE;
	}
	return DAQmxSuccess == retVal ? TRUE : FALSE;
}

/// <summary> generate digital output task after digital waveform being built </summary>
long ThorStim::SetupTaskMasterDigital(void)
{
	int32 retVal, error = 0;	

	TerminateTask(_taskHandleDO);

	if(_digiLineSelect)
	{
		try
		{
			//support USB board control on Pockels cell
			int32 dataXferType = DAQmx_Val_DMA;
			BoardInfo* bInfo = boardInfoNI.get()->getInstance()->GetBoardInfo(GetDevIDName(_digiLineStr));
			if(NULL != bInfo)
			{
				dataXferType = (BoardStyle::USB == bInfo->boardStyle) ? DAQmx_Val_USBbulk : DAQmx_Val_DMA;
			}

			std::string controllerInternalOutput = "/" + GetDevIDName(_counter) + "/Ctr" + _counter.at(_counter.length()-1) + "InternalOutput";

			DAQmxErrChk (L"DAQmxCreateTask",retVal = DAQmxCreateTask("",&_taskHandleDO));
			DAQmxErrChk (L"DAQmxCreateDOChan",retVal = DAQmxCreateDOChan(_taskHandleDO,_digiLineStr.c_str(),"",DAQmx_Val_ChanPerLine));
			if(FALSE == ImageWaveformBuilder->GetGGalvoWaveformParamsWithStatus(SignalType::DIGITAL_LINES,&_gGalvoWaveParams, _precaptureStatus, _currentIndex[SignalType::DIGITAL_LINES]))
			{
				StringCbPrintfW(message,_MAX_PATH,L"ImageWaveformBuilder unable to retrieve galvo waveform.");
				LogMessage(message,ERROR_EVENT);
			}
			if(_dLengthPerAOCallback[SignalType::DIGITAL_LINES] < _totalLength[SignalType::DIGITAL_LINES])
			{
				DAQmxErrChk (L"DAQmxSetWriteAttribute",retVal = DAQmxSetWriteAttribute (_taskHandleDO, DAQmx_Write_RegenMode, DAQmx_Val_DoNotAllowRegen));
			}
			DAQmxErrChk (L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(_taskHandleDO,controllerInternalOutput.c_str(),static_cast<float64>(_gGalvoWaveParams.ClockRate),DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,_totalLength[SignalType::DIGITAL_LINES]));
			DAQmxErrChk (L"DAQmxCfgOutputBuffer",retVal = DAQmxCfgOutputBuffer(_taskHandleDO,static_cast<uInt32>(min(_totalLength[SignalType::DIGITAL_LINES], _dLengthPerAOCallback[SignalType::DIGITAL_LINES] * _activeLoadCount))));	
			//DO cannot be armStarted:
			SetFrameInTriggerableTask(_taskHandleDO, FALSE);

			DAQmxErrChk (L"DAQmxWriteDigitalLines",retVal = DAQmxWriteDigitalLines(_taskHandleDO, static_cast<int32>(_dLengthPerAOCallback[SignalType::DIGITAL_LINES]),false,-1,DAQmx_Val_GroupByChannel,_gGalvoWaveParams.DigBufWaveform,NULL,NULL));
			DAQmxErrChk (L"DAQmxTaskControl",retVal = DAQmxTaskControl(_taskHandleDO,DAQmx_Val_Task_Reserve));
			DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleDO));
		}
		catch(...)
		{
			StringCbPrintfW(message,_MAX_PATH, L"ThorStim NI Exception at SetupTaskMasterDigital: %d", retVal);
			LogMessage(message,ERROR_EVENT);
		}
	}
	return retVal;
}

/// <summary> setup frame trigger input callback for trigger counting </summary>
long ThorStim::SetupFrameTriggerInput(void)
{
	long retVal = FALSE;
	const int DEFAULT_COUNTER_COUNT = 4;

	switch (_triggerMode)
	{
	case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
	case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
	case ICamera::HW_SINGLE_FRAME: ///Setup frame triggered by external hardware
		ResetEvent(_hHardwareTriggerInEvent);

		BoardInfo* bInfo = boardInfoNI.get()->getInstance()->GetBoardInfo(GetDevIDName(_counter));
		//use next available counter of counter device
		retVal = TryFrameTriggerInputwCounter((_counter.at(_counter.length()-1) + 1) % (NULL != bInfo ? bInfo->counterCount : DEFAULT_COUNTER_COUNT));

		if (!retVal && NULL != bInfo)
		{
			for (char i = 0; i < bInfo->counterCount; i++)
			{
				if (TryFrameTriggerInputwCounter(i))
				{
					retVal = TRUE;
					break;
				}
			}
		}
		break;
	}
	return retVal;
}

/// <summary> try setup clock task based on counter ID [0,1,2,3] </summary>
long ThorStim::TryTaskMasterClockwCounter(int counterID)
{
	int32 retVal = DAQmxSuccess, error = 0;
	if (0 > counterID || 3 < counterID) 
		return FALSE;

	TerminateTask(_taskHandleCO0);
	try
	{
		std::string counterLine = "/" + GetDevIDName(_counter) + "/ctr" + std::to_string(counterID);

		//
		//Counter output pulses are used as the clock for the AO/DO waveform generation
		//
		DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleCO0));
		DAQmxErrChk(L"DAQmxCreateCOPulseChanFreq",retVal = DAQmxCreateCOPulseChanFreq(_taskHandleCO0, counterLine.c_str(), "", DAQmx_Val_Hz, DAQmx_Val_Low, 0.0, _sampleRateHz, 0.5));

		uInt64 numSamples = _totalLength[SignalType::ANALOG_POCKEL];
		//use continuous clock to pick up first trigger, but failed at KHz input to keep DO & AO aligned:
		DAQmxErrChk(L"DAQmxCfgImplicitTiming ",retVal = DAQmxCfgImplicitTiming (_taskHandleCO0, DAQmx_Val_ContSamps, numSamples));
		//use below with finite & armed clock to keep DO & AO aligned at KHz input, but will miss the first trigger:
		//SetFrameInTriggerableTask(_taskHandleCO0, TRUE);
	}
	catch(...)
	{
#ifdef _DEBUG
		DAQmxGetExtendedErrorInfo(logMsg,_MAX_PATH);
		LogMessage((wchar_t*)StringToWString(std::string(logMsg)).c_str(), ERROR_EVENT);
#endif
		DAQmxFailed(error);
		StringCbPrintfW(message,_MAX_PATH,L"%hs failed: (%d)",__FUNCTION__, error);
		LogMessage(message,VERBOSE_EVENT);
		TerminateTask(_taskHandleCO0);
		return FALSE;
	}
	return DAQmxSuccess == retVal ? TRUE : FALSE;
}

/// <summary> try setup frame trigger input task based on counter ID [0,1,2,3] </summary>
long ThorStim::TryFrameTriggerInputwCounter(char counterID)
{
	int32 retVal = DAQmxSuccess, error = 0;
	TerminateTask(_taskHandleDI);
	try
	{
		std::string triggerInputCtrLine = "/" + GetDevIDName(_counter) + "/ctr" + std::to_string(counterID);

		DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleDI));

		DAQmxErrChk(L"DAQmxCreateCICountEdgesChan",retVal = DAQmxCreateCICountEdgesChan (_taskHandleDI, triggerInputCtrLine.c_str(), "", DAQmx_Val_Rising, 0, DAQmx_Val_CountUp));

		DAQmxErrChk (L"DAQmxSetCICountEdgesTerm",retVal = DAQmxSetCICountEdgesTerm(_taskHandleDI,triggerInputCtrLine.c_str(),_triggerIn.c_str()));

		DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(_taskHandleDI,_triggerIn.c_str(),1000,DAQmx_Val_Rising,DAQmx_Val_HWTimedSinglePoint, 0));

		DAQmxErrChk(L"DAQmxRegisterSignalEvent",retVal = DAQmxRegisterSignalEvent(_taskHandleDI, DAQmx_Val_SampleClock , 0, ThorStim::HWTriggerCallback, NULL));	

		DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleDI));
	}
	catch(...)
	{
#ifdef _DEBUG
		DAQmxGetExtendedErrorInfo(logMsg,_MAX_PATH);
		LogMessage((wchar_t*)StringToWString(std::string(logMsg)).c_str(), ERROR_EVENT);
#endif
		DAQmxFailed(error);
		StringCbPrintfW(message,_MAX_PATH,L"%hs failed: (%d)",__FUNCTION__, error);
		LogMessage(message,VERBOSE_EVENT);
		TerminateTask(_taskHandleDI);
		return FALSE;
	}
	return DAQmxSuccess == retVal ? TRUE : FALSE;
}

/// <summary> restart or stop waveform tasks </summary>
long ThorStim::SyncCustomWaveformOnOff(bool32 start)
{
	int32 retVal = DAQmxSuccess, error = 0;
	try 
	{
		if(_taskHandleCO0)
		{
			//stop primary counter first since all other tasks are dependent:
			DAQmxStopTask(_taskHandleCO0);
			if(_taskHandleAOPockels)	//not start until DO1 dummy line
			{
				DAQmxStopTask(_taskHandleAOPockels);
				if(start)
					DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleAOPockels));
			}
			if(_taskHandleDO)
			{
				DAQmxStopTask(_taskHandleDO);
				if(start)
					DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleDO));
			}
			//start primary counter to start all tasks:
			if(start)
				DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleCO0));
		}
	}
	catch(...)
	{
		WaveformModeFinished();
		_precaptureStatus = PreCaptureStatus::PRECAPTURE_BLEACHER_ERROR;
		StringCbPrintfW(message,_MAX_PATH, L"ThorStim NI Exception at SyncCustomWaveformOnOff: %d", retVal);
		LogMessage(message,ERROR_EVENT);
		return FALSE;
	}
	return DAQmxSuccess == retVal ? TRUE : FALSE;
}

/// <summary> set waveform task triggerable by frame trigger in line </summary>
long ThorStim::SetFrameInTriggerableTask(TaskHandle taskHandle, long armStart)
{
	int32 retVal = DAQmxSuccess, error = 0;
	switch ((ICamera::TriggerMode)_triggerMode)
	{
	case ICamera::TriggerMode::HW_MULTI_FRAME_TRIGGER_EACH:
	case ICamera::TriggerMode::HW_MULTI_FRAME_TRIGGER_FIRST:
	case ICamera::TriggerMode::HW_SINGLE_FRAME:
		retVal = DAQmxCfgDigEdgeStartTrig(taskHandle,_triggerIn.c_str() ,DAQmx_Val_Rising);
		if(armStart)
		{
			retVal = DAQmxSetArmStartTrigType(taskHandle, DAQmx_Val_DigEdge);
			retVal = DAQmxSetDigEdgeArmStartTrigSrc(taskHandle, _triggerIn.c_str());
			retVal = DAQmxSetDigEdgeArmStartTrigEdge(taskHandle, DAQmx_Val_Rising);
		}
		break;
	}
	return DAQmxSuccess == retVal ? TRUE : FALSE;
}

/// <summary> write pockels waveform directly from waveform builder in active load only </summary>
long ThorStim::TryWriteTaskMasterPockelWaveform(long checkStop)
{
	int32 error = 0;

	//write pockels:
	if(_numPockelsLines && _taskHandleAOPockels)
	{
		if (checkStop && (WaitForSingleObject(ThorStim::_hStopAcquisition, 0) == WAIT_OBJECT_0))
		{
			return FALSE;
		}

		//retrieve waveform:
		if(FALSE == ImageWaveformBuilder->GetGGalvoWaveformParamsWithStatus(SignalType::ANALOG_POCKEL,&_gGalvoWaveParams, _precaptureStatus, ThorStim::_currentIndex[SignalType::ANALOG_POCKEL]))
		{
			return FALSE;
		}

		//write waveforms:
		if(_totalLength[SignalType::ANALOG_POCKEL] <= ThorStim::_currentIndex[SignalType::ANALOG_POCKEL])
		{
			DAQmxSetWriteNextWriteIsLast(_taskHandleAOPockels, DAQmx_Write_NextWriteIsLast);
		}
		else
		{
			DAQmxResetWriteNextWriteIsLast(_taskHandleAOPockels);
		}

		error = DAQmxWriteAnalogF64(_taskHandleAOPockels, static_cast<int32>(_gGalvoWaveParams.analogPockelSize / _gGalvoWaveParams.pockelsCount), false, -1, DAQmx_Val_GroupByChannel, _gGalvoWaveParams.GalvoWaveformPockel, NULL, NULL);
		if(0 != error)
		{
			SetEvent(_hStopAcquisition);
			WaveformModeFinished();
			if(_totalLength[SignalType::ANALOG_POCKEL] > ThorStim::_currentIndex[SignalType::ANALOG_POCKEL])
			{
				_precaptureStatus = PreCaptureStatus::PRECAPTURE_BLEACHER_ERROR;
				//StringCbPrintfW(_errMsg,_MAX_PATH,L"Unable to write pockels waveform due to error: %d\n Count = %d of %d" , error, static_cast<int>(ThorStim::_currentIndex[SignalType::ANALOG_POCKEL]), static_cast<int>(_totalLength[SignalType::ANALOG_POCKEL]));
				StringCbPrintfW(_errMsg,_MAX_PATH,L"Unable to write pockels waveform.\n");
				MessageBox(NULL,_errMsg,L"Waveform Load Error", MB_OK | MB_SETFOREGROUND | MB_ICONERROR);
			}
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

/// <summary> write line waveforms directly from waveform builder in active load only </summary>
long ThorStim::TryWriteTaskMasterLineWaveform(long checkStop)
{
	int32 error = 0;

	if(_taskHandleDO)
	{
		if (checkStop && (WaitForSingleObject(ThorStim::_hStopAcquisition, 0) == WAIT_OBJECT_0))
		{
			return FALSE;
		}

		//retrieve waveform:
		if(FALSE == ImageWaveformBuilder->GetGGalvoWaveformParamsWithStatus(SignalType::DIGITAL_LINES,&_gGalvoWaveParams, _precaptureStatus, ThorStim::_currentIndex[SignalType::DIGITAL_LINES]))
		{
			return FALSE;
		}

		//write waveforms:
		if(_totalLength[SignalType::DIGITAL_LINES] <= ThorStim::_currentIndex[SignalType::DIGITAL_LINES])
		{
			DAQmxSetWriteNextWriteIsLast(_taskHandleDO, DAQmx_Write_NextWriteIsLast);
		}
		else
		{
			DAQmxResetWriteNextWriteIsLast(_taskHandleDO);
		}

		error = DAQmxWriteDigitalLines(_taskHandleDO,static_cast<int32>(_gGalvoWaveParams.digitalSize/_gGalvoWaveParams.digitalLineCnt), false, -1,DAQmx_Val_GroupByChannel,_gGalvoWaveParams.DigBufWaveform,NULL,NULL);
		if(0 != error)
		{
			SetEvent(_hStopAcquisition);
			WaveformModeFinished();
			if(_totalLength[SignalType::DIGITAL_LINES] > ThorStim::_currentIndex[SignalType::DIGITAL_LINES])
			{
				_precaptureStatus = PreCaptureStatus::PRECAPTURE_BLEACHER_ERROR;
				//StringCbPrintfW(_errMsg,_MAX_PATH,L"Unable to write line waveform due to error: %d\n Count = %d of %d" , error, static_cast<int>(ThorStim::_currentIndex[SignalType::DIGITAL_LINES]), static_cast<int>(_totalLength[SignalType::DIGITAL_LINES]));
				StringCbPrintfW(_errMsg,_MAX_PATH,L"Unable to write line waveform.\n");
				MessageBox(NULL,_errMsg,L"Waveform Load Error", MB_OK | MB_SETFOREGROUND | MB_ICONERROR);
			}
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}
