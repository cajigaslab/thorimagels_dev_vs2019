// TaskMaterFuncs.cpp : Defines Bleach scan functions for the DLL application.
//

#include "stdafx.h"
#include "ThorGGNI.h"
#include "Strsafe.h"


//***	Waveform static members:	***//
long ThorLSMCam::_finishedCycleCnt = 0;
long ThorLSMCam::_triggeredCycleCnt = 0;
long ThorLSMCam::_cycleDoneLength = 0;
long ThorLSMCam::_precaptureStatus = PreCaptureStatus::PRECAPTURE_BLEACHER_IDLE;
long ThorLSMCam::_activeLoadCount = 1;
long ThorLSMCam::_dLengthPerAOCallback[SignalType::SIGNALTYPE_LAST] = {Constants::ACTIVE_LOAD_UNIT_SIZE};
uint64_t ThorLSMCam::_totalLength[SignalType::SIGNALTYPE_LAST] = {0};
string ThorLSMCam::_pockelDigOut;
string ThorLSMCam::_waveformCompleteOut;
string ThorLSMCam::_bleachCycleOut;
string ThorLSMCam::_bleachIterationOut;
string ThorLSMCam::_bleachPatternOut;
string ThorLSMCam::_bleachPatternCompleteOut;
string ThorLSMCam::_bleachActiveOut;
string ThorLSMCam::_bleachEpochOut;
string ThorLSMCam::_bleachCycleInverse;
string ThorLSMCam::_bleachShutterLine;
long ThorLSMCam::_bleachShutterIdle[2] = {0, 0};
long ThorLSMCam::_waveformTaskStatus = ICamera::STATUS_BUSY;
int ThorLSMCam::_digiBleachSelect = 0x0;
uint64_t ThorLSMCam::_currentIndex[(int)(SignalType::SIGNALTYPE_LAST)] = {0};

///	***************************************** <summary> Task-Master related functions	</summary>	**************************************************************** ///

/// <summary> Callback function to be invoked when frame trigger in </summary>
int32 CVICALLBACK ThorLSMCam::HWTriggerCallback(TaskHandle taskHandle, int32 signalID, void *callbackData)
{
	switch (_imgPtyDll.triggerMode)
	{
	case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
		ThorLSMCam::_triggeredCycleCnt++;
		if(1 == ThorLSMCam::_triggeredCycleCnt)
		{
			SetEvent(ThorLSMCam::_hHardwareTriggerInEvent);
		}
		break;
	default:
		SetEvent(ThorLSMCam::_hHardwareTriggerInEvent);
		//stop DI task:
		if(_taskHandleDI1)
		{
			DAQmxStopTask(_taskHandleDI1);
			DAQmxClearTask(_taskHandleDI1);
		}
		break;
	}
	return 0;
}

/// <summary> Callback function to be invoked in Waveform mode other than HW trigger each </summary>
int32 CVICALLBACK ThorLSMCam::CycleDoneCallback(TaskHandle taskHandle, int32 signalID, void *callbackData)
{
	int32 retVal = 0, error = 0;
	long updateCase = 0;
	ThorLSMCam::_finishedCycleCnt++;

	//terminate if necessary:
	if((WaitForSingleObject(ThorLSMCam::_hStopAcquisition, 0) == WAIT_OBJECT_0) || (ThorLSMCam::_finishedCycleCnt ==_imgPtyDll.numFrame))
	{
		//pockels task may be finished earlier:
		//if((_pockelsEnable[0]) && _taskHandleAOPockels)
		//{
		//	DAQmxWaitUntilTaskDone(_taskHandleAOPockels,Constants::TIMEOUT_MS);
		//}

		//Done last cycle:		
		return WaveformModeFinished();
	}

	//continue in SW mode after triggered for HW trigger first mode:
	if(ICamera::HW_MULTI_FRAME_TRIGGER_FIRST == _imgPtyDll.triggerMode)
	{
		_imgPtyDll.triggerMode = ICamera::SW_MULTI_FRAME;
		updateCase = 1;
	}

	//prepare last cycle:
	if ((_imgPtyDll.numFrame - 1) == ThorLSMCam::_finishedCycleCnt)
	{
		_precaptureStatus = PreCaptureStatus::PRECAPTURE_WAVEFORM_LAST_CYCLE;
		//update when active or complete line is configured:
		if((0 < (_digiBleachSelect & (0x1 << BLEACHSCAN_DIGITAL_LINENAME::CYCLE_COMPLETE)))
			|| (0 < (_digiBleachSelect & (0x1 << BLEACHSCAN_DIGITAL_LINENAME::ACTIVE_ENVELOPE))))
			updateCase = (1 != updateCase) ? 2 : 1;
	}

	switch (updateCase)
	{
	case 1:	//reset all tasks
		SyncCustomWaveformOnOff(false);
		//re-start waveform:
		TryBuildTaskMaster();
		SetupTaskMasterGalvo();
		TryWriteTaskMasterGalvoWaveform(TRUE);

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
int32 CVICALLBACK ThorLSMCam::EveryNDigitalOutCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32 retVal = 0, error = 0;
	//return if not triggered, and only handle trigger each case:
	if((_imgPtyDll.triggerMode != ICamera::HW_MULTI_FRAME_TRIGGER_EACH) || 
		(0 == ThorLSMCam::_triggeredCycleCnt) ||
		(ThorLSMCam::_waveformTaskStatus != ICamera::STATUS_BUSY) ||
		(ThorLSMCam::_triggeredCycleCnt == ThorLSMCam::_finishedCycleCnt))
		return retVal;

	//increment of finished cycle count:
	if(ThorLSMCam::_triggeredCycleCnt > ThorLSMCam::_finishedCycleCnt)
	{
		ThorLSMCam::_finishedCycleCnt++;
	}

	//terminate if necessary:
	if((WaitForSingleObject(ThorLSMCam::_hStopAcquisition, 0) == WAIT_OBJECT_0) || (ThorLSMCam::_finishedCycleCnt ==_imgPtyDll.numFrame))
	{
		//callback is invoked earlier than the task finishes,
		//could be further triggered if sleep too long:
		Sleep(12);
		//Done last cycle:		
		return WaveformModeFinished();
	}

	//prepare for last cycle:
	if (ThorLSMCam::_finishedCycleCnt == (_imgPtyDll.numFrame - 1))
	{
		////prepare complete line:	
		//SetupCompleteWaveformTrigger(TRUE);
	}

	return retVal;
}

/// <summary> Callback function to be invoked for waveform active load mode </summary>
int32 CVICALLBACK ThorLSMCam::EveryNGalvoOutCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	//terminate if necessary:
	if((WaitForSingleObject(ThorLSMCam::_hStopAcquisition, 0) == WAIT_OBJECT_0) || (ThorLSMCam::_finishedCycleCnt ==_imgPtyDll.numFrame))
	{
		//Done active load:
		return WaveformModeFinished();
	}

	//digital lines and galvo share the same clock:
	TryWriteTaskMasterLineWaveform(TRUE);
	return TryWriteTaskMasterGalvoWaveform(TRUE);
}

/// <summary> Callback function to be invoked for waveform active load mode </summary>
int32 CVICALLBACK ThorLSMCam::EveryNPockelOutCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	//terminate if necessary:
	if((WaitForSingleObject(ThorLSMCam::_hStopAcquisition, 0) == WAIT_OBJECT_0) || (ThorLSMCam::_finishedCycleCnt ==_imgPtyDll.numFrame))
	{
		//Done active load:
		return WaveformModeFinished();
	}

	//get and execute next piece of waveform:
	return TryWriteTaskMasterPockelWaveform(TRUE);
}

/// <summary> build digital output waveform based on supplied memory [Dropped, check line selection only]</summary>
long ThorLSMCam::BuildTaskMasterDigital(void)
{
	///Copy the arrays for digital output as waveform cycle triggers.
	///Use P0.6 as Trigger out, followed by pockels Digital output, 
	///complete digital output, cycle envelope, iteration envelope, pattern triggers, ... etc

	//trigger line buffer:
	_digitalTriggerLines = "/" + _devID + "/port0/line6";
	_digiBleachSelect = 0x1;

	//no memory copy in active loading, only set bitwise line selections:
	if(_pockelDigOut.size() > 0)
	{
		_digitalTriggerLines += "," + _pockelDigOut;
		_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::POCKEL_DIG);
	}
	if(_bleachActiveOut.size() > 0)
	{
		_digitalTriggerLines += "," + _bleachActiveOut;
		_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::ACTIVE_ENVELOPE);
	}
	if(_waveformCompleteOut.size() > 0)
	{
		_digitalTriggerLines += "," + _waveformCompleteOut;
		_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::CYCLE_COMPLETE);
	}
	if(_bleachCycleOut.size() > 0)
	{	
		_digitalTriggerLines += "," + _bleachCycleOut;
		_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::CYCLE_ENVELOPE);
	}
	if(_bleachIterationOut.size() > 0)
	{
		_digitalTriggerLines += "," + _bleachIterationOut;
		_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::ITERATION_ENVELOPE);
	}
	if(_bleachPatternOut.size() > 0)
	{
		_digitalTriggerLines += "," + _bleachPatternOut;
		_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::PATTERN_TRIGGER);
	}
	if(_bleachPatternCompleteOut.size() > 0)
	{
		_digitalTriggerLines += "," + _bleachPatternCompleteOut;
		_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::PATTERN_COMPLETE);
	}
	if(_bleachEpochOut.size() > 0)
	{
		_digitalTriggerLines += "," + _bleachEpochOut;
		_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::EPOCH_ENVELOPE);
	}
	if(_bleachCycleInverse.size() > 0)
	{
		_digitalTriggerLines += "," + _bleachCycleInverse;
		_digiBleachSelect |= (0x1 << (int)BLEACHSCAN_DIGITAL_LINENAME::CYCLE_COMPLEMENTARY);
	}
	return TRUE;
}

/// <summary> force update image properties for task master setup</summary>
void ThorLSMCam::ForceUpdateProperties(void)
{
	_imgPtyDll = _ImgPty;
}

/// <summary> setup clock task for Bleach Scan </summary>
long ThorLSMCam::SetupTaskMasterClock(void)
{
	int32 retVal = 0;
	int32 error = 0;	

	uInt64 numSamples = _totalLength[SignalType::ANALOG_XY];

	//stop and clear counter 1 first since it is linked to counter 0
	retVal = DAQmxStopTask(_taskHandleCO1);
	retVal = DAQmxClearTask(_taskHandleCO1);
	_taskHandleCO1 = NULL;
	//
	//1st Counter output pulses are used as the clock for the AO waveform generation
	//
	retVal = DAQmxStopTask(_taskHandleCO0);
	retVal = DAQmxClearTask(_taskHandleCO0);
	_taskHandleCO0 = NULL;

	DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleCO0));
	DAQmxErrChk(L"DAQmxCreateCOPulseChanFreq",retVal = DAQmxCreateCOPulseChanFreq(_taskHandleCO0, _controllerOutputLine0.c_str(), "", DAQmx_Val_Hz, DAQmx_Val_Low, 0.0, _clockRateNI, 0.5));

	DAQmxErrChk(L"DAQmxConnectTerms",retVal = DAQmxConnectTerms(_controllerInternalOutput0.c_str(), _startTriggerLine.c_str(), DAQmx_Val_DoNotInvertPolarity)); 

	//use CO0 only in waveform mode, since not necessary to sync with Alazar:
	//use continuous clock to pick up first trigger, but failed at KHz input to keep DO & AO aligned:
	DAQmxErrChk(L"DAQmxCfgImplicitTiming ",retVal = DAQmxCfgImplicitTiming (_taskHandleCO0, DAQmx_Val_ContSamps, numSamples));
	//use below with finite & armed clock to keep DO & AO aligned at KHz input, but will miss the first trigger:
	//SetFrameInTriggerableTask(_taskHandleCO0, TRUE);

	// _taskHandleCO1 is available for others in Bleach Scan mode...

	//prepare counter clock for pockels AO:
	if(TRUE == _pockelsEnable[0])
	{
		DAQmxStopTask(_taskHandleCO2);
		DAQmxClearTask(_taskHandleCO2);
		_taskHandleCO2 = NULL;
		DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleCO2));
		DAQmxErrChk(L"DAQmxCreateCOPulseChanFreq",retVal = DAQmxCreateCOPulseChanFreq(_taskHandleCO2, _controllerOutputLine2.c_str(), "", DAQmx_Val_Hz, DAQmx_Val_Low, 0.0, _clockRateNI, 0.5));
		DAQmxErrChk(L"DAQmxCfgImplicitTiming ",retVal = DAQmxCfgImplicitTiming (_taskHandleCO2, DAQmx_Val_ContSamps, _totalLength[SignalType::ANALOG_POCKEL]));
	}
	return retVal;
}

/// <summary> setup Galvo task for Bleach Scan </summary>
long ThorLSMCam::SetupTaskMasterGalvo(void)
{
	int32 retVal = 0, error = 0;

	if (_taskHandleAO1)
	{
		retVal = DAQmxStopTask(_taskHandleAO1);
		retVal = DAQmxClearTask(_taskHandleAO1);
	}

	if(_pockelsEnableIntegrated)
	{
		_galvoAndPockelsLinesOutput = ((0 < _analogChannels[0].length()) && (0 < _analogChannels[1].length()) && (0 < _analogChannels[2].length())) ? (_analogChannels[0] + "," + _analogChannels[1] + "," + _analogChannels[2]) : "/Dev2/ao0:2";
	}
	else
	{
		_galvoAndPockelsLinesOutput = ((0 < _analogChannels[0].length()) && (0 < _analogChannels[1].length())) ? (_analogChannels[0] + "," + _analogChannels[1]) : "/Dev2/ao0:1";
	}
	try
	{
		DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleAO1));
		DAQmxErrChk(L"DAQmxCreateAOVoltageChan",retVal = DAQmxCreateAOVoltageChan(_taskHandleAO1, _galvoAndPockelsLinesOutput.c_str(), "", MIN_AO_VOLTAGE, MAX_AO_VOLTAGE, DAQmx_Val_Volts, NULL));
		//retrieve waveforms:
		if(FALSE == ImageWaveformBuilder->GetGGalvoWaveformParamsWithStatus(SignalType::ANALOG_XY,&_gGalvoWaveParams, _precaptureStatus, ThorLSMCam::_currentIndex[SignalType::ANALOG_XY]))
		{
			StringCbPrintfW(_errMsg,_MAX_PATH,L"ImageWaveformBuilder unable to retrieve galvo waveform.");
			LogMessage(_errMsg,ERROR_EVENT);
		}
		if(_dLengthPerAOCallback[SignalType::ANALOG_XY] < _totalLength[SignalType::ANALOG_XY])
		{
			DAQmxErrChk (L"DAQmxSetWriteAttribute",retVal = DAQmxSetWriteAttribute (_taskHandleAO1, DAQmx_Write_RegenMode, DAQmx_Val_DoNotAllowRegen));
			DAQmxErrChk (L"DAQmxRegisterEveryNSamplesEvent",retVal = DAQmxRegisterEveryNSamplesEvent(_taskHandleAO1, DAQmx_Val_Transferred_From_Buffer, static_cast<uInt32>(_dLengthPerAOCallback[SignalType::ANALOG_XY]), 0, EveryNGalvoOutCallback, NULL));
		}
		DAQmxErrChk(L"DAQmxRegisterDoneEvent",retVal = DAQmxRegisterDoneEvent(_taskHandleAO1,0,ThorLSMCam::CycleDoneCallback,NULL));
		DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(_taskHandleAO1, _controllerInternalOutput0.c_str(), static_cast<float64>(_gGalvoWaveParams.ClockRate), DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, _totalLength[SignalType::ANALOG_XY]));
		DAQmxErrChk (L"DAQmxCfgOutputBuffer",retVal = DAQmxCfgOutputBuffer(_taskHandleAO1,static_cast<uInt32>(_totalLength[SignalType::ANALOG_XY])));
		//AO cannot be armStarted:
		SetFrameInTriggerableTask(_taskHandleAO1, FALSE);
		DAQmxErrChk(L"DAQmxWriteAnalogF64",retVal = DAQmxWriteAnalogF64(_taskHandleAO1, static_cast<int32>(_dLengthPerAOCallback[SignalType::ANALOG_XY]), false, -1, DAQmx_Val_GroupByScanNumber, _gGalvoWaveParams.GalvoWaveformXY, NULL, NULL));
		DAQmxErrChk(L"DAQmxTaskControl",retVal = DAQmxTaskControl(_taskHandleAO1,DAQmx_Val_Task_Reserve));
		DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleAO1));
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorGGNI SetupTaskMasterGalvo failed, error: (%d)", retVal);
		LogMessage(_errMsg,ERROR_EVENT);
	}
	return retVal;
}

/// <summary> setup Pockel task for Bleach Scan </summary>
long ThorLSMCam::SetupTaskMasterPockel(void)
{
	int32 retVal = 0, error = 0;

	//support USB board control on Pockels cell
	int32 dataXferType = DAQmx_Val_DMA;
	BoardInfo* bInfo = _boardInfoNI.get()->getInstance()->GetBoardInfo(GetDevIDName(_controllerInternalOutput2));
	if(NULL != bInfo)
	{
		dataXferType = (BoardStyle::USB == bInfo->boardStyle) ? DAQmx_Val_USBbulk : DAQmx_Val_DMA;
	}

	if(_pockelsEnable[0])
	{
		TerminateTask(_taskHandleAOPockels);

		DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleAOPockels));

		string channelString = _pockelsLine[0].c_str();

		DAQmxErrChk(L"DAQmxCreateAOVoltageChan",retVal = DAQmxCreateAOVoltageChan(_taskHandleAOPockels, channelString.c_str() ,"", MIN_AO_VOLTAGE,MAX_AO_VOLTAGE, DAQmx_Val_Volts, NULL));

		if(FALSE == ImageWaveformBuilder->GetGGalvoWaveformParamsWithStatus(SignalType::ANALOG_POCKEL,&_gGalvoWaveParams, _precaptureStatus, ThorLSMCam::_currentIndex[SignalType::ANALOG_POCKEL]))
		{
			StringCbPrintfW(_errMsg,_MAX_PATH,L"ImageWaveformBuilder unable to retrieve galvo waveform.");
			LogMessage(_errMsg,ERROR_EVENT);
		}
		if(_dLengthPerAOCallback[SignalType::ANALOG_POCKEL] < _totalLength[SignalType::ANALOG_POCKEL])
		{
			DAQmxErrChk (L"DAQmxSetWriteAttribute",retVal = DAQmxSetWriteAttribute (_taskHandleAOPockels, DAQmx_Write_RegenMode, DAQmx_Val_DoNotAllowRegen));
			DAQmxErrChk (L"DAQmxRegisterEveryNSamplesEvent",retVal = DAQmxRegisterEveryNSamplesEvent(_taskHandleAOPockels, DAQmx_Val_Transferred_From_Buffer, static_cast<int32>(_dLengthPerAOCallback[SignalType::ANALOG_POCKEL]), 0, EveryNPockelOutCallback, NULL));
		}			
		DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(_taskHandleAOPockels, _controllerInternalOutput2.c_str(), static_cast<float64>(_gGalvoWaveParams.ClockRate), DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, _totalLength[SignalType::ANALOG_POCKEL]));

		DAQmxErrChk (L"DAQmxSetAODataXferMech", retVal = DAQmxSetAODataXferMech(_taskHandleAOPockels,"",dataXferType));

		DAQmxErrChk (L"DAQmxCfgOutputBuffer",retVal = DAQmxCfgOutputBuffer(_taskHandleAOPockels,static_cast<uInt32>(_totalLength[SignalType::ANALOG_POCKEL])));

		retVal = DAQmxCfgDigEdgeStartTrig(_taskHandleAOPockels,_pockelsTriggerIn.c_str() ,DAQmx_Val_Rising);

		retVal = DAQmxWriteAnalogF64(_taskHandleAOPockels, static_cast<int32>(_dLengthPerAOCallback[SignalType::ANALOG_POCKEL]), false, -1, DAQmx_Val_GroupByChannel, _gGalvoWaveParams.GalvoWaveformPockel, NULL, NULL);

		if(0 == retVal && TRUE == _pockelsEnable[0] && TRUE == _imgPtyDll.useReferenceForPockelsOutput && TRUE == _pockelsReferenceRequirementsMet)
		{
			//Set the pockels1 reference to be external
			DAQmxErrChk(L"DAQmxSetAODACRefSrc",retVal = DAQmxSetAODACRefSrc(_taskHandleAO1, _pockelsLine[0].c_str(), DAQmx_Val_External));
			const double MAX_POCKELS_VOLTAGE = 10.0;
			//Even if the pockels1 reference is set to external, the value needs to be set
			DAQmxErrChk(L"DAQmxSetAODACRefVal",retVal = DAQmxSetAODACRefVal(_taskHandleAO1, _pockelsLine[0].c_str(), MAX_POCKELS_VOLTAGE));
			//Set the input line for the pockels reference. It needs to be an APFI line
			DAQmxErrChk(L"DAQmxSetAODACRefExtSrc",retVal = DAQmxSetAODACRefExtSrc(_taskHandleAO1, _pockelsLine[0].c_str(), _pockelsReferenceLine.c_str()));
		}

		if(0 == retVal)
		{
			DAQmxErrChk(L"DAQmxTaskControl",retVal = DAQmxTaskControl(_taskHandleAOPockels,DAQmx_Val_Task_Reserve));
			DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleAOPockels));
		}
		StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI SetupTaskMasterPockel DAQmxStartTask AO2 Return = %d", retVal);
		LogMessage(message,VERBOSE_EVENT);
	}

	return retVal;
}

/// <summary> generate digital output task after digital waveform being built </summary>
long ThorLSMCam::SetupTaskMasterDigital(void)
{
	int32 retVal, error = 0;	

	retVal = DAQmxStopTask(_taskHandleDO1);
	retVal = DAQmxClearTask(_taskHandleDO1);

	if(_digitalTriggerLines.size() > 0)
	{
		try
		{
			DAQmxErrChk (L"DAQmxCreateTask",retVal = DAQmxCreateTask("",&_taskHandleDO1));
			DAQmxErrChk (L"DAQmxCreateDOChan",retVal = DAQmxCreateDOChan(_taskHandleDO1,_digitalTriggerLines.c_str(),"",DAQmx_Val_ChanPerLine));
			if(FALSE == ImageWaveformBuilder->GetGGalvoWaveformParamsWithStatus(SignalType::DIGITAL_LINES,&_gGalvoWaveParams, _precaptureStatus, ThorLSMCam::_currentIndex[SignalType::DIGITAL_LINES]))
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"ImageWaveformBuilder unable to retrieve galvo waveform.");
				LogMessage(_errMsg,ERROR_EVENT);
			}
			if(_dLengthPerAOCallback[SignalType::DIGITAL_LINES] < _totalLength[SignalType::DIGITAL_LINES])
			{
				DAQmxErrChk (L"DAQmxSetWriteAttribute",retVal = DAQmxSetWriteAttribute (_taskHandleDO1, DAQmx_Write_RegenMode, DAQmx_Val_DoNotAllowRegen));
			}
			DAQmxErrChk (L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(_taskHandleDO1,_controllerInternalOutput0.c_str(),static_cast<float64>(_gGalvoWaveParams.ClockRate),DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,_totalLength[SignalType::DIGITAL_LINES]));
			DAQmxErrChk (L"DAQmxCfgOutputBuffer",retVal = DAQmxCfgOutputBuffer(_taskHandleDO1,static_cast<uInt32>(_totalLength[SignalType::DIGITAL_LINES])));	
			//DO cannot be armStarted:
			SetFrameInTriggerableTask(_taskHandleDO1, FALSE);
			////EveryN callback works with digital output only, not analog output, used to determine cycles 
			////[dropped] for trigger each will be handled by cycle done as well in active loading
			//if(ICamera::HW_MULTI_FRAME_TRIGGER_EACH == _imgPtyDll.triggerMode)
			//{
			//	DAQmxErrChk (L"DAQmxRegisterEveryNSamplesEvent",retVal = DAQmxRegisterEveryNSamplesEvent(_taskHandleDO1, DAQmx_Val_Transferred_From_Buffer, static_cast<uInt32>(_totalLength[SignalType::DIGITAL_LINES]), 0, EveryNDigitalOutCallback, NULL));
			//}
			DAQmxErrChk (L"DAQmxWriteDigitalLines",retVal = DAQmxWriteDigitalLines(_taskHandleDO1, static_cast<int32>(_dLengthPerAOCallback[SignalType::DIGITAL_LINES]),FALSE,0,DAQmx_Val_GroupByChannel,_gGalvoWaveParams.DigBufWaveform,NULL,NULL));
			DAQmxErrChk (L"DAQmxTaskControl",retVal = DAQmxTaskControl(_taskHandleDO1,DAQmx_Val_Task_Reserve));
			DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleDO1));
		}
		catch(...)
		{
			StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI NI Exception at SetupTaskMasterDigital: %d", retVal);
			LogMessage(message,ERROR_EVENT);
		}
	}
	return retVal;
}

/// <summary> generate 0.5ms pulse at complete PFI line, [dropped]:have to use buffered digital line </summary>
//long ThorLSMCam::SetupCompleteWaveformTrigger(long startNow)
//{
//	int32 retVal = 0, error = 0;
//
//	//trigger by frame trigger input in trigger each mode, 
//	//otherwise by counter clock:
//	std::string startTriggerLine = ((ICamera::HW_MULTI_FRAME_TRIGGER_EACH == _imgPtyDll.triggerMode) 
//		|| ((1 == _imgPtyDll.numFrame) && (ICamera::HW_MULTI_FRAME_TRIGGER_FIRST == _imgPtyDll.triggerMode)))
//		? _frameTriggerLineIn : _controllerInternalOutput0;
//	//use CO1 (available in waveform mode):
//	if(_taskHandleCO1)
//	{
//		retVal = DAQmxStopTask(_taskHandleCO1);
//		retVal = DAQmxClearTask(_taskHandleCO1);
//		_taskHandleCO1 = NULL;
//	}
//	//force to be a PFI line:
//	if((_waveformCompleteOut.size() > 0) && 
//		((_waveformCompleteOut.find("PFI")!= string::npos) || (_waveformCompleteOut.find("port1") != string::npos)))
//	{
//		try
//		{
//			float64 delayTime = (0 == _cycleDoneLength)? static_cast<float64>(_galvoDataLength - 1)/static_cast<float64>(_clockRateNI) : static_cast<float64>(_cycleDoneLength)/static_cast<float64>(_clockRateNI);
//
//			DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleCO1));
//			//use CO1 (available in waveform mode) output one 0.5ms pulse:
//			DAQmxErrChk(L"DAQmxCreateCOPulseChanFreq",retVal = DAQmxCreateCOPulseChanFreq(_taskHandleCO1, _controllerOutputLine1.c_str(), "", DAQmx_Val_Hz, DAQmx_Val_Low, delayTime, 1000, 0.5));
//			DAQmxErrChk(L"DAQmxCfgImplicitTiming",retVal = DAQmxCfgImplicitTiming (_taskHandleCO1, DAQmx_Val_FiniteSamps, 1));
//			DAQmxErrChk(L"DAQmxSetCOPulseTerm",retVal = DAQmxSetCOPulseTerm(_taskHandleCO1,"",_waveformCompleteOut.c_str()));
//			//arm start with ctr0 clock for waveform AO & DO:
//			DAQmxErrChk(L"DAQmxCfgDigEdgeStartTrig",retVal = DAQmxCfgDigEdgeStartTrig(_taskHandleCO1, startTriggerLine.c_str(), DAQmx_Val_Rising));
//			DAQmxErrChk(L"DAQmxSetArmStartTrigType",retVal = DAQmxSetArmStartTrigType(_taskHandleCO1, DAQmx_Val_DigEdge));
//			DAQmxErrChk(L"DAQmxSetDigEdgeArmStartTrigSrc",retVal = DAQmxSetDigEdgeArmStartTrigSrc(_taskHandleCO1, _controllerInternalOutput0.c_str()));
//			DAQmxErrChk(L"DAQmxSetDigEdgeArmStartTrigEdge",retVal = DAQmxSetDigEdgeArmStartTrigEdge(_taskHandleCO1, DAQmx_Val_Rising));
//			DAQmxErrChk(L"DAQmxTaskControl",retVal = DAQmxTaskControl(_taskHandleCO1,DAQmx_Val_Task_Reserve));
//			if(TRUE == startNow)	//if not, let it start with other tasks for trigger accuracy.
//			{
//				DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleCO1));
//			}
//		}
//		catch(...)
//		{
//			StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI NI Exception at SetupCompleteWaveformTrigger: %d", retVal);
//			LogMessage(message,ERROR_EVENT);
//		}
//	}
//	return retVal;
//}

/// <summary> restart or stop waveform tasks </summary>
long ThorLSMCam::SyncCustomWaveformOnOff(bool32 start)
{
	int32 retVal = 0, error = 0;
	try 
	{
		if(_taskHandleCO0)
		{
			//stop primary counter first since all other tasks are dependent:
			retVal = DAQmxStopTask(_taskHandleCO0);
			if(_taskHandleCO2)
			{
				retVal = DAQmxStopTask(_taskHandleCO2);
				if(start)
					DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleCO2));
			}
			if(_taskHandleAOPockels)	//not start until DO1 dummy line
			{
				retVal = DAQmxStopTask(_taskHandleAOPockels);
				if(start)
					DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleAOPockels));
			}
			if(_taskHandleDO1)
			{
				retVal = DAQmxStopTask(_taskHandleDO1);
				if(start)
					DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleDO1));
			}
			if(_taskHandleAO1)
			{
				retVal = DAQmxStopTask(_taskHandleAO1);
				if(start)
					DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleAO1));
			}
			if(_taskHandleCO1)
			{
				retVal = DAQmxStopTask(_taskHandleCO1);
				if(start)
					DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleCO1));
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
		StringCbPrintfW(_errMsg,_MAX_PATH, L"ThorGGNI NI Exception at SyncCustomWaveformOnOff: %d", retVal);
		LogMessage(_errMsg,ERROR_EVENT);
	}
	return retVal;
}

/// <summary> set waveform task triggerable by frame trigger in line </summary>
long ThorLSMCam::SetFrameInTriggerableTask(TaskHandle taskHandle, long armStart)
{
	int32 retVal = 0, error = 0;
	switch ((ICamera::TriggerMode)_imgPtyDll.triggerMode)
	{
	case ICamera::TriggerMode::HW_MULTI_FRAME_TRIGGER_EACH:
	case ICamera::TriggerMode::HW_MULTI_FRAME_TRIGGER_FIRST:
	case ICamera::TriggerMode::HW_SINGLE_FRAME:
		retVal = DAQmxCfgDigEdgeStartTrig(taskHandle,_frameTriggerLineIn.c_str() ,DAQmx_Val_Rising);
		if(armStart)
		{
			retVal = DAQmxSetArmStartTrigType(taskHandle, DAQmx_Val_DigEdge);
			retVal = DAQmxSetDigEdgeArmStartTrigSrc(taskHandle, _frameTriggerLineIn.c_str());
			retVal = DAQmxSetDigEdgeArmStartTrigEdge(taskHandle, DAQmx_Val_Rising);
		}
		////No retriggable in active loading:
		//if(_imgPtyDll.triggerMode == ICamera::HW_MULTI_FRAME_TRIGGER_EACH)
		//{
		//	retVal = DAQmxSetStartTrigRetriggerable(taskHandle,true);
		//}
		break;
	}
	return retVal;
}

/// <summary> replace local parameters based on active loaded waveform files </summary>
long ThorLSMCam::TryBuildTaskMaster(void)
{
	long retVal = TRUE;
	for (int i = 0; i < static_cast<long>(SignalType::SIGNALTYPE_LAST); i++)
	{
		_dLengthPerAOCallback[i] = _activeLoadCount * Constants::ACTIVE_LOAD_UNIT_SIZE;
	}
	uint64_t totalDataCount = ImageWaveformBuilder->RebuildWaveformFromFile(_waveformPathName.c_str(), NULL, _digiBleachSelect, _dLengthPerAOCallback);
	if(0 == totalDataCount)
	{
		StringCbPrintfW(_errMsg,_MAX_PATH,L"ImageWaveformBuilder unable to build waveform.");
		LogMessage(_errMsg,ERROR_EVENT);
		return FALSE;
	}

	//return if no change of data, so that no need
	//to restart task at cycle done callback:
	if(totalDataCount == _totalLength[SignalType::ANALOG_XY])
	{
		return FALSE;
	}

	//reset data length including initial travel and final callback patch,
	//using values from waveform file:
	_totalLength[SignalType::ANALOG_XY] = _totalLength[SignalType::ANALOG_POCKEL] = _totalLength[SignalType::DIGITAL_LINES] = totalDataCount;
	return retVal;
}

/// <summary> write galvo waveform directly from waveform builder in active load only </summary>
long ThorLSMCam::TryWriteTaskMasterGalvoWaveform(long checkStop)
{
	int32 error = 0;

	if(_taskHandleAO1)
	{
		if (checkStop && (WaitForSingleObject(ThorLSMCam::_hStopAcquisition, 0) == WAIT_OBJECT_0))
		{
			return FALSE;
		}

		//retrieve waveform:
		if(FALSE == ImageWaveformBuilder->GetGGalvoWaveformParamsWithStatus(SignalType::ANALOG_XY,&_gGalvoWaveParams, _precaptureStatus, ThorLSMCam::_currentIndex[SignalType::ANALOG_XY]))
		{
			StringCbPrintfW(message, _MAX_PATH, L"ThorGGNI:%hs@%u: failed to get buffer for Galvo.", __FILE__, __LINE__);
			LogMessage(message, ERROR_EVENT);
			return FALSE;
		}

		//write waveforms:
		if(_totalLength[SignalType::ANALOG_XY] <= ThorLSMCam::_currentIndex[SignalType::ANALOG_XY])
		{
			DAQmxSetWriteNextWriteIsLast(_taskHandleAO1, DAQmx_Write_NextWriteIsLast);
		}
		else
		{
			DAQmxResetWriteNextWriteIsLast(_taskHandleAO1);
		}

		error = DAQmxWriteAnalogF64(_taskHandleAO1, static_cast<int32>(_gGalvoWaveParams.analogXYSize/2), false, -1, DAQmx_Val_GroupByScanNumber, _gGalvoWaveParams.GalvoWaveformXY, NULL, NULL);
		if(0 != error)
		{
			SetEvent(_hStopAcquisition);
			WaveformModeFinished();
			if(_totalLength[SignalType::ANALOG_XY] > ThorLSMCam::_currentIndex[SignalType::ANALOG_XY])
			{
				_precaptureStatus = PreCaptureStatus::PRECAPTURE_BLEACHER_ERROR;
				//StringCbPrintfW(_errMsg,_MAX_PATH,L"Unable to writegalvo waveform due to error: %d" , error);
				StringCbPrintfW(_errMsg,_MAX_PATH,L"Unable to write galvo waveform.\n");
				MessageBox(NULL,_errMsg,L"Waveform Load Error", MB_OK | MB_SETFOREGROUND | MB_ICONERROR);
			}
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

/// <summary> write pockels waveform directly from waveform builder in active load only </summary>
long ThorLSMCam::TryWriteTaskMasterPockelWaveform(long checkStop)
{
	int32 error = 0;

	//write pockels:
	if((_pockelsEnable[0]) && (_taskHandleAOPockels))
	{
		if (checkStop && (WaitForSingleObject(ThorLSMCam::_hStopAcquisition, 0) == WAIT_OBJECT_0))
		{
			return FALSE;
		}

		//retrieve waveform:
		if(FALSE == ImageWaveformBuilder->GetGGalvoWaveformParamsWithStatus(SignalType::ANALOG_POCKEL,&_gGalvoWaveParams, _precaptureStatus, ThorLSMCam::_currentIndex[SignalType::ANALOG_POCKEL]))
		{
			StringCbPrintfW(message, _MAX_PATH, L"ThorGGNI:%hs@%u: failed to get buffer for Pockels.", __FILE__, __LINE__);
			LogMessage(message, ERROR_EVENT);
			return FALSE;
		}

		//write waveforms:
		if(_totalLength[SignalType::ANALOG_POCKEL] <= ThorLSMCam::_currentIndex[SignalType::ANALOG_POCKEL])
		{
			DAQmxSetWriteNextWriteIsLast(_taskHandleAOPockels, DAQmx_Write_NextWriteIsLast);
		}
		else
		{
			DAQmxResetWriteNextWriteIsLast(_taskHandleAOPockels);
		}

		error = DAQmxWriteAnalogF64(_taskHandleAOPockels, static_cast<int32>(_gGalvoWaveParams.analogPockelSize), false, -1, DAQmx_Val_GroupByChannel, _gGalvoWaveParams.GalvoWaveformPockel, NULL, NULL);
		if(0 != error)
		{
			SetEvent(_hStopAcquisition);
			WaveformModeFinished();
			if(_totalLength[SignalType::ANALOG_POCKEL] > ThorLSMCam::_currentIndex[SignalType::ANALOG_POCKEL])
			{
				_precaptureStatus = PreCaptureStatus::PRECAPTURE_BLEACHER_ERROR;
				//StringCbPrintfW(_errMsg,_MAX_PATH,L"Unable to write pockels waveform due to error: %d\n Count = %d of %d" , error, static_cast<int>(ThorLSMCam::_currentIndex[SignalType::ANALOG_POCKEL]), static_cast<int>(_totalLength[SignalType::ANALOG_POCKEL]));
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
long ThorLSMCam::TryWriteTaskMasterLineWaveform(long checkStop)
{
	int32 error = 0;

	if(_taskHandleDO1)
	{
		if (checkStop && (WaitForSingleObject(ThorLSMCam::_hStopAcquisition, 0) == WAIT_OBJECT_0))
		{
			return FALSE;
		}

		//retrieve waveform:
		if(FALSE == ImageWaveformBuilder->GetGGalvoWaveformParamsWithStatus(SignalType::DIGITAL_LINES,&_gGalvoWaveParams, _precaptureStatus, ThorLSMCam::_currentIndex[SignalType::DIGITAL_LINES]))
		{
			StringCbPrintfW(message, _MAX_PATH, L"ThorGGNI:%hs@%u: failed to get buffer for Digital lines.", __FILE__, __LINE__);
			LogMessage(message, ERROR_EVENT);
			return FALSE;
		}

		//write waveforms:
		if(_totalLength[SignalType::DIGITAL_LINES] <= ThorLSMCam::_currentIndex[SignalType::DIGITAL_LINES])
		{
			DAQmxSetWriteNextWriteIsLast(_taskHandleDO1, DAQmx_Write_NextWriteIsLast);
		}
		else
		{
			DAQmxResetWriteNextWriteIsLast(_taskHandleDO1);
		}

		error = DAQmxWriteDigitalLines(_taskHandleDO1,static_cast<int32>(_gGalvoWaveParams.digitalSize/_gGalvoWaveParams.digitalLineCnt), false, -1,DAQmx_Val_GroupByChannel,_gGalvoWaveParams.DigBufWaveform,NULL,NULL);
		if(0 != error)
		{
			SetEvent(_hStopAcquisition);
			WaveformModeFinished();
			if(_totalLength[SignalType::DIGITAL_LINES] > ThorLSMCam::_currentIndex[SignalType::DIGITAL_LINES])
			{
				_precaptureStatus = PreCaptureStatus::PRECAPTURE_BLEACHER_ERROR;
				//StringCbPrintfW(_errMsg,_MAX_PATH,L"Unable to write line waveform due to error: %d\n Count = %d of %d" , error, static_cast<int>(ThorLSMCam::_currentIndex[SignalType::DIGITAL_LINES]), static_cast<int>(_totalLength[SignalType::DIGITAL_LINES]));
				StringCbPrintfW(_errMsg,_MAX_PATH,L"Unable to write line waveform.\n");
				MessageBox(NULL,_errMsg,L"Waveform Load Error", MB_OK | MB_SETFOREGROUND | MB_ICONERROR);
			}
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

/// <summary> signal done of waveform mode </summary>
long ThorLSMCam::WaveformModeFinished(void)
{
	if(ICamera::STATUS_BUSY == ThorLSMCam::_waveformTaskStatus)
	{
		SyncCustomWaveformOnOff(false);
		ImageWaveformBuilder->CloseWaveformFile();
		SetEvent(_hThreadStopped);
		TogglePulseToDigitalLine(_taskHandleDO1, _bleachShutterLine, 1, TogglePulseMode::ToggleLow, _bleachShutterIdle[1]);	//close bleach shutter
		ThorLSMCam::_waveformTaskStatus = ICamera::STATUS_READY;
	}
	return 0;
}
