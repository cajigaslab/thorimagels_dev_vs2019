#include "DAQBoard.h"
#include "DevParamDef.h"
#include "Logger.h"
#include "..\..\..\Common\StringCPP.h"

using namespace std;

DAQBoard::DAQBoard()
{
	_isRunning = false;
}

DAQBoard::~DAQBoard()
{
}

std::unique_ptr<DAQBoard> DAQBoard::_pInstance;
std::once_flag DAQBoard::_onceFlag;

DAQBoard* DAQBoard::GetInstance()
{
	std::call_once(_onceFlag,
		[] {
			_pInstance.reset(new DAQBoard);
	});
	return _pInstance.get();
}

long DAQBoard::InitDAQBoard()
{
	ClearTasks();
	return TRUE;
}

long DAQBoard::SetSampleClock(double sampleClock, long samplesPerLineTrigger)
{
	_sampleClock = sampleClock;
	_samplesPerLineTrigger = samplesPerLineTrigger;
	return TRUE;
}

long DAQBoard::RigisterTasks(const char* name, const char* channel, CHANNEL_TYPE type, void* pBuffer, unsigned int channelCount)
{
	long ret = FALSE;
	TaskHandle taskHandle = NULL;// CreateTask(channel, type, pBuffer, channelCount);
	//if (taskHandle != 0) as the tasks need to recreate each time.
	{
		auto handle = CreateMutex(NULL, false, NULL);
		NITask* nITask = new NITask(taskHandle, pBuffer, channelCount, handle, channel, type, name);
		_runningTasks.push_back(nITask);
		ret = TRUE;
	}
	return ret;
}

TaskHandle DAQBoard::CreateTask(const char* name, const char* channel, CHANNEL_TYPE type, void* pBuffer, unsigned int channelCount)
{
	int32 retVal = 0;
	int  error = 0;
	TaskHandle taskHandle = 0;
	try
	{
		switch (type)
		{
		case AO:
		case DO:
			if (channel == NULL || pBuffer == NULL || channelCount == 0) return 0;
			if (type == AO)
			{
				DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask(name, &taskHandle));
				DAQmxErrChk(L"DAQmxCreateAOVoltageChan",retVal = DAQmxCreateAOVoltageChan(taskHandle, channel, "", VOLT_MIN, VOLT_MAX, DAQmx_Val_Volts, NULL));
			}
			else if (type == DO)
			{
				DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask(name, &taskHandle));
				DAQmxErrChk(L"DAQmxCreateDOChan",retVal = DAQmxCreateDOChan(taskHandle, channel, "", DAQmx_Val_ChanPerLine));
			}
			DAQmxErrChk(L"DAQmxSetWriteRegenMode",retVal = DAQmxSetWriteRegenMode(taskHandle, DAQmx_Val_DoNotAllowRegen));
			DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(taskHandle, "", _sampleClock, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, _samplesPerLineTrigger));

			DAQmxErrChk(L"DAQmxCfgDigEdgeStartTrig",retVal = DAQmxCfgDigEdgeStartTrig(taskHandle, DEV_GY_TRIG_STR, DAQmx_Val_Rising));
			DAQmxErrChk(L"DAQmxSetStartTrigRetriggerable",retVal = DAQmxSetStartTrigRetriggerable(taskHandle, 1));

			DAQmxErrChk(L"DAQmxCfgOutputBuffer",retVal = DAQmxCfgOutputBuffer(taskHandle, NI_BUFFER_LENGTH * BUFFER_COUNT_IN_NI_CARD));
			DAQmxErrChk(L"DAQmxRegisterEveryNSamplesEvent",retVal = DAQmxRegisterEveryNSamplesEvent(taskHandle, DAQmx_Val_Transferred_From_Buffer, NI_BUFFER_LENGTH, 0, EveryNCallback, NULL));
			break;
		case AI:
			break;
		case DI:
			break;
		case CO:
			break;
		case CI:	//HW trigger in, user pass in event handle
			ResetEvent(pBuffer);
			DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &taskHandle));
			DAQmxErrChk(L"DAQmxCreateCICountEdgesChan",retVal = DAQmxCreateCICountEdgesChan (taskHandle, "/Dev1/ctr3", "", DAQmx_Val_Rising, 0, DAQmx_Val_CountUp ));
			DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming (taskHandle, channel, 1000, DAQmx_Val_Rising, DAQmx_Val_HWTimedSinglePoint, 0));
			DAQmxErrChk(L"DAQmxRegisterSignalEvent",retVal = DAQmxRegisterSignalEvent(taskHandle, DAQmx_Val_SampleClock , 0, DAQBoard::HWTriggerCallback, pBuffer));
			break;
		default:
			break;
		}
		return taskHandle;
	}
	catch(...)
	{
		if (DAQmxFailed(error))
			DAQmxGetExtendedErrorInfo(_errBuff, ERROR_STR_LEN);
		Logger::getInstance().LogMessage((wchar_t*)StringToWString(_errBuff).c_str());
		TerminateTask (taskHandle);
	}
	return 0;
}

long DAQBoard::InitRuningTasks(CHANNEL_TYPE type)
{
	for (vector<NITask*>::iterator it = _runningTasks.begin(); it != _runningTasks.end(); it++)
	{
		auto task = *it;
		if (type == task->channelType)
		{
			TaskHandle taskHandle = CreateTask(task->name, task->channel, task->channelType, task->pBuffer, task->channelCount);
			if (taskHandle != NULL)
			{
				task->taskHandle = taskHandle;
			}
			else
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

long DAQBoard::PreStartTasks()
{
	for (size_t i = 0; i < _runningTasks.size(); i++)
	{
		if (FALSE == WriteWaveform(_runningTasks.at(i), NI_BUFFER_LENGTH*BUFFER_COUNT_IN_NI_CARD))
		{
			StringCbPrintfW(message,_MAX_PATH,L"DAQBoard PreStartTasks failed (%ls)", StringToWString(_runningTasks.at(i)->name).c_str());
			Logger::getInstance().LogMessage(message,ERROR_EVENT);
			return FALSE;
		}
	}
	return TRUE;
}

long DAQBoard::StarTasks()
{
	int32 retVal = 0;
	int  error = 0;
	if (_runningTasks.size() == 0) return FALSE;
	try
	{
		for (vector<NITask*>::iterator it = _runningTasks.begin(); it != _runningTasks.end(); it++)
		{
			NITask* task = *it;
			if(task == NULL) return FALSE;
			if(NULL != task->taskHandle) 
			{
				DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(task->taskHandle));
			}
		}
		_isRunning = true;
		return TRUE;
	}
	catch(...)
	{
#ifdef LOGGING_ENABLED 
		if (DAQmxFailed(error))
			DAQmxGetExtendedErrorInfo(_errBuff, ERROR_STR_LEN);
		Logger::getInstance().LogMessage((wchar_t*)StringToWString(_errBuff).c_str());
#endif
		StopAllTasks();
		return FALSE;
	}
}

long DAQBoard::StopAllTasks()
{
	for (vector<NITask*>::iterator it = _runningTasks.begin(); it != _runningTasks.end(); it++)
	{
		if (*it == NULL) continue;
		DAQmxStopTask((*it)->taskHandle);
		DAQmxClearTask((*it)->taskHandle);
		(*it)->taskHandle = NULL;
	}
	_isRunning = false;
	return TRUE;
}

long DAQBoard::ClearTasks()
{
	if (_isRunning)
	{
		StopAllTasks();
	}
	if (0 < _runningTasks.size())
	{
		for (size_t i = 0; i < _runningTasks.size(); i++)
		{
			if (NULL != _runningTasks.at(i))
			{
				delete _runningTasks.at(i);
				_runningTasks.at(i) = NULL;
			}
		}
		_runningTasks.clear();
	}
	return TRUE;
}

long DAQBoard::InvokeTask(const char * channel, CHANNEL_TYPE type, void * pBuffer, uint32_t size, double rate)
{
	int32 retVal = 0;
	int  error = 0;
	TaskHandle taskHandle = 0;
	if (channel == NULL || pBuffer == NULL || size == 0) return FALSE;
	if (FALSE == CheckTask(channel)) return FALSE;
	try
	{
		switch ((CHANNEL_TYPE)type)
		{
		case AO:	//set pockels power
			DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &taskHandle));
			DAQmxErrChk(L"DAQmxCreateAOVoltageChan",retVal = DAQmxCreateAOVoltageChan(taskHandle, channel, "", VOLT_MIN, VOLT_MAX, DAQmx_Val_Volts, NULL));
			// directly set the voltage if only 1
			if (size == 1)
			{
				DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(taskHandle));
				DAQmxErrChk(L"DAQmxWriteAnalogF64",retVal = DAQmxWriteAnalogF64(taskHandle, size, 1, 10.0, DAQmx_Val_GroupByChannel, (float64*)pBuffer, NULL, NULL));
			}
			else
			{
				DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(taskHandle, "", rate, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, size));
				DAQmxErrChk(L"DAQmxCfgOutputBuffer",retVal = DAQmxCfgOutputBuffer(taskHandle, size));
				DAQmxErrChk(L"DAQmxWriteAnalogF64",retVal = DAQmxWriteAnalogF64(taskHandle, size, 0, 10.0, DAQmx_Val_GroupByChannel, (float64*)pBuffer, NULL, NULL));
				DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(taskHandle));
				DAQmxErrChk(L"DAQmxWaitUntilTaskDone",retVal = DAQmxWaitUntilTaskDone(taskHandle, MAX_TASK_WAIT_TIME));
			}
			break;
		case AI:
			break;
		case DO:	//set frame trigger out low
			DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &taskHandle));
			DAQmxErrChk(L"DAQmxCreateDOChan",retVal = DAQmxCreateDOChan(taskHandle, channel, "", DAQmx_Val_ChanForAllLines));
			DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(taskHandle));
			DAQmxErrChk(L"DAQmxWriteDigitalLines",retVal = DAQmxWriteDigitalLines(taskHandle, size, 1, 0, DAQmx_Val_GroupByChannel, (uInt8*)pBuffer, NULL, NULL));
			break;
		default:
			break;
		}
		TerminateTask(taskHandle);
		return TRUE;
	}
	catch(...)
	{
#ifdef LOGGING_ENABLED 
		if (DAQmxFailed(error))
			DAQmxGetExtendedErrorInfo(_errBuff, ERROR_STR_LEN);
		Logger::getInstance().LogMessage((wchar_t*)StringToWString(_errBuff).c_str());
#endif
		TerminateTask(taskHandle); 
		return FALSE;
	}
}

long DAQBoard::AysncInvokeTask(const char * channel, CHANNEL_TYPE type, void * pBuffer, uint32_t size)
{
	return 0;
}

int32 CVICALLBACK DAQBoard::EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int ret = TRUE;
	NITask* nITask = NULL;
	for (vector<NITask*>::iterator it = _pInstance->_runningTasks.begin(); it != _pInstance->_runningTasks.end(); it++)
	{
		if ((*it)->taskHandle == taskHandle)
		{
			nITask = *it;
		}
	}
	if (nITask != NULL)
	{
		WaitForSingleObject(nITask->hWritingMutex, 50);
		ret = _pInstance->WriteWaveform(nITask, NI_BUFFER_LENGTH);
		ReleaseMutex(nITask->hWritingMutex);
	}
	return ret;
}

int32 CVICALLBACK DAQBoard::HWTriggerCallback(TaskHandle taskHandle, int32 signalID, void *callbackData)
{
	SetEvent(callbackData);
	DAQmxStopTask(taskHandle);
	return 0;
}

long DAQBoard::WriteWaveform(NITask* task, long bufferlength)
{
	if (task == NULL) return FALSE;
	long ret = FALSE;
	int  error = 0;
	int32 written = 0, retVal = 0;

	uint32_t sizeOfDataType = 4;

	switch ((CHANNEL_TYPE)task->channelType)
	{
	case AO:
		sizeOfDataType = sizeof(double);
		break;
	case DO:
		sizeOfDataType = sizeof(uint8_t);
		break;
	case CI:
	case CO:
	default:
		return TRUE;
	}

	void* pWaveformBuffer;
	long length = task->channelCount * bufferlength * sizeOfDataType;
	long lengthOut = ((IBuffer*)(task->pBuffer))->DataSize(length);
	if (lengthOut == 0)
	{
		return FALSE;
	}
	else if (lengthOut < length)
	{
		bufferlength = lengthOut / task->channelCount / sizeOfDataType;
	}
	ret = ((IBuffer*)(task->pBuffer))->GetReadPointer((void**)&pWaveformBuffer, lengthOut);
	if (ret == FALSE) return ret;
	try
	{
		if (task->channelType == AO)
		{
			DAQmxErrChk(L"DAQmxWriteAnalogF64",retVal = DAQmxWriteAnalogF64(task->taskHandle, bufferlength, 0, WRITE_TIMEOUT, DAQmx_Val_GroupByScanNumber, (double*)pWaveformBuffer, &written, NULL));
		}
		else if (task->channelType == DO)
		{
			DAQmxErrChk(L"DAQmxWriteDigitalLines",retVal = DAQmxWriteDigitalLines(task->taskHandle, bufferlength, 0, WRITE_TIMEOUT, DAQmx_Val_GroupByChannel, (uint8_t*)pWaveformBuffer, &written, NULL));
		}

		((IBuffer*)(task->pBuffer))->ReadCompleted(lengthOut);

		return TRUE;;
	}
	catch(...)
	{
#ifdef LOGGING_ENABLED 
		if (DAQmxFailed(error))
			DAQmxGetExtendedErrorInfo(_errBuff, ERROR_STR_LEN);
		Logger::getInstance().LogMessage((wchar_t*)StringToWString(_errBuff).c_str());
#endif
		StopAllTasks();
		return FALSE;
	}
}

long DAQBoard::MeasureFrequency(const char* channel, const char* data, float64 *value)
{
	int32 retVal = 0;
	int         error = 0;
	TaskHandle  taskHandle = 0;
	char        errBuff[2048] = { '\0' };
	try
	{
		/*********************************************/
		// DAQmx Configure Code
		/*********************************************/
		DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &taskHandle));
		DAQmxErrChk(L"DAQmxCreateCIFreqChan",retVal = DAQmxCreateCIFreqChan(taskHandle, channel, "", 1.192093, 10000000.000000, DAQmx_Val_Hz, DAQmx_Val_Rising, DAQmx_Val_LowFreq1Ctr, 0.001, 4, ""));
		DAQmxErrChk(L"DAQmxSetCIFreqTerm",retVal = DAQmxSetCIFreqTerm(taskHandle, channel, data));

		/*********************************************/
		// DAQmx Start Code
		/*********************************************/
		DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(taskHandle));

		/*********************************************/
		// DAQmx Read Code
		/*********************************************/
		DAQmxErrChk(L"DAQmxReadCounterScalarF64",retVal = DAQmxReadCounterScalarF64(taskHandle, 10.0, value, 0));

		TerminateTask (taskHandle);		
		return TRUE;
	}
	catch(...)
	{
#ifdef LOGGING_ENABLED 
		if (DAQmxFailed(error))
			DAQmxGetExtendedErrorInfo(errBuff, 2048);
		Logger::getInstance().LogMessage((wchar_t*)StringToWString(_errBuff).c_str());
#endif
		TerminateTask (taskHandle);
		return FALSE;
	}
}

long DAQBoard::ReadVoltages(const char* channel, float64* voltages, unsigned int channelCount)
{
	int32 retVal = 0;
	int  error = 0;
	TaskHandle taskHandle = 0;
	try
	{
		DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &taskHandle));
		DAQmxErrChk(L"DAQmxCreateAIVoltageChan",retVal = DAQmxCreateAIVoltageChan(taskHandle, channel, "", DAQmx_Val_Cfg_Default, VOLT_MIN, VOLT_MAX, DAQmx_Val_Volts, NULL));	//DAQmx_Val_RSE 

		int32 samplesRead = 0;
		DAQmxErrChk(L"DAQmxReadAnalogF64",retVal = DAQmxReadAnalogF64(taskHandle, 1, READ_TIMEOUT, DAQmx_Val_GroupByChannel, voltages, channelCount, &samplesRead, NULL));
		if (taskHandle != 0) 
		{
			DAQmxStopTask(taskHandle);
			DAQmxClearTask(taskHandle);
		}
		return TRUE;
	}
	catch(...)
	{
#ifdef LOGGING_ENABLED 
		if (DAQmxFailed(error))
			DAQmxGetExtendedErrorInfo(_errBuff, ERROR_STR_LEN);
		Logger::getInstance().LogMessage((wchar_t*)StringToWString(_errBuff).c_str());
#endif
		return FALSE;
	}
}

//**********************************	Additional Functions	**********************************//

long DAQBoard::CheckTask(const char* channel)
{
	BoardInfo* bInfo = _boardInfoNI.get()->getInstance()->GetBoardInfo(GetDevIDName(channel));
	if (NULL != bInfo)
	{
		if (std::string::npos != bInfo->devType.find("6363"))
		{
			return TRUE;
		}
		else
		{
			StringCbPrintfW(message,_MAX_PATH,L"Hardware configuration to control (%ls) is not valid, only accept NI PCIe-6363.", StringToWString(channel).c_str());
			Logger::getInstance().LogMessage(message, VERBOSE_EVENT);
		}
	}
	return FALSE;
}
