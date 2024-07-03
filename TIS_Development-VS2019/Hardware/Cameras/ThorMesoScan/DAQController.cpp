#include "DAQController.h"
#include "stdafx.h"
//#include "MesoScanWaveform.h"
#include "DevParamDef.h"

//#define DEBUG_TIME
//#define DEBUG_LOG
#ifdef DEBUG_LOG
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

#ifdef DEBUG_TIME
#include "MyTimer.h"
#define TIME_START \
	MyTimer mt; \
	mt.Start();
#define TIME_END(name) \
	mt.End(); \
	printf(#name##" time = %ld\n", mt.costTime);
#else
#define TIME_START
#define TIME_END()
#endif

//#define DEBUG_NO_TRIG

#define BUFFER_COUNT_IN_NI_CARD (8)

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else
DAQController * DAQController::_pInstance = NULL;
DAQController::DAQController():_hasDAQController(false)
{
	_hWritingBuffer[0] = CreateMutex(NULL, false, NULL);
	_hWritingBuffer[1] = CreateMutex(NULL, false, NULL);

	_taskHandleAO0 = NULL;  // galvoX1, galvoX2
	_taskHandleAO1 = NULL;  // galvoY, voice coil, pockel

	_bufferLength = NI_BUFFER_LENGTH;
	_pokelsPointCount = SAMPLES_PER_LINE;
	_isActive = false;
	_isContinueOutput = false;
	_pInstance = this;
}

DAQController::~DAQController()
{
	//if (_pMesoScanWaveform != NULL)
	//{
	//	free(_pMesoScanWaveform);
	//	_pMesoScanWaveform = NULL;
	//}
}

long DAQController::FindDAQController()
{
	_hasDAQController = true;
	return TRUE;
}

long DAQController::InitDAQController()
{
	if (!_hasDAQController)
		return FALSE;
	return TRUE;
}

bool DAQController::IsAvaliable()
{
	return _hasDAQController;
}
long DAQController::SetWaveformBuffer(int index, IBuffer* waveformBuffer)
{
	switch (index)
	{
	case 0:
		_waveformBufferGX12 = waveformBuffer;
		break;
	case 1:
		_waveformBufferGYPV = waveformBuffer;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

long DAQController::SetSampleClock(double sampleClock)
{
	if (!_hasDAQController)
		return FALSE;
	if (_isActive)
	{
		StopAndClear();
		_isActive = false;
	}
	if (ConfigTask(sampleClock))
	{
		_isActive = true;
		_isContinueOutput = true;
		return true;
	}
	return false;
}

long DAQController::SetCaptureActiveOutput(long startOrStop)
{
	if (!_hasDAQController)
		return FALSE;
	if(startOrStop == 1)
	{
		long ret = FALSE;
		if(_isContinueOutput)
		{
			bool isContinueOutput1 = WriteWaveformCh0(_bufferLength, BUFFER_COUNT_IN_NI_CARD);
			bool isContinueOutput2 = WriteWaveformCh1(_bufferLength, BUFFER_COUNT_IN_NI_CARD);
			_isContinueOutput = isContinueOutput1 && isContinueOutput2;
			ret = StartOutput();
		}
		return ret;
	}
	else
	{
		return TRUE;
	}
}
long DAQController::WriteWaveformCh0(long length, long count)
{
	LOG("Enter WriteWaveform\n");
	TIME_START
	int         error = 0;

	int32   	written;
	bool isContinue = true;
	for(int bufferIdx = 0; bufferIdx < count && isContinue; bufferIdx++)
	{
		printf("read buffer\n");
		double* pWaveformBufferGalvoX;

		long length = _bufferLength*CHANNELS_IN_BUFFER1 * sizeof(double);
		long lengthOut = _waveformBufferGX12->DataSize(length);

		long ret = _waveformBufferGX12->GetReadPointer((void**)&pWaveformBufferGalvoX, lengthOut);
		TIME_END(GetGalvoX1X2WaveForm)
		
			if (lengthOut != length)
			{
				Sleep(100);
			}
				DAQmxErrChk(DAQmxWriteAnalogF64(_taskHandleAO0, lengthOut / CHANNELS_IN_BUFFER1 / sizeof(double), 0, WRITE_TIMEOUT, DAQmx_Val_GroupByScanNumber, pWaveformBufferGalvoX, &written, NULL));
		_waveformBufferGX12->ReadCompleted(lengthOut);
		if (lengthOut < length)
		{
			printf("buffer stop\n");
			return FALSE;
		}
	}
Error:
	if (DAQmxFailed(error))
		DAQmxGetExtendedErrorInfo(_errBuff, ERROR_STR_LEN);
	if (DAQmxFailed(error))
		LOG("DAQmx Error: %s\n", _errBuff);
	LOG("Waveform generated\n");
	TIME_END(WriteWaveform)
	return isContinue ? TRUE : FALSE;
}
long DAQController::WriteWaveformCh1(long length, long count)
{
	LOG("Enter WriteWaveform\n");
	TIME_START
		int         error = 0;
	int32   	written;
	long lengthOut = 0;
	bool isContinue = true;
	for (int bufferIdx = 0; bufferIdx < count && isContinue; bufferIdx++)
	{
		printf("read buffer\n");
		double* pWaveformBuffer;

		long length = _bufferLength*CHANNELS_IN_BUFFER2 * sizeof(double);
		long lengthOut = _waveformBufferGYPV->DataSize(length);
		if (lengthOut == 0)
		{
			printf("buffer stop\n");
			return FALSE;
		}

		long ret = _waveformBufferGYPV->GetReadPointer((void**)&pWaveformBuffer, lengthOut);
		TIME_END(GetGalvoVoiceCoilWaveForm)
			if (lengthOut != length)
			{
				Sleep(100);
			}
				DAQmxErrChk(DAQmxWriteAnalogF64(_taskHandleAO1, lengthOut / CHANNELS_IN_BUFFER2 / sizeof(double), 0, WRITE_TIMEOUT, DAQmx_Val_GroupByScanNumber, pWaveformBuffer, &written, NULL));
		_waveformBufferGYPV->ReadCompleted(lengthOut);
		if (lengthOut < length)
		{
			printf("buffer stop\n");
			return FALSE;
		}
	}
Error:
	if (DAQmxFailed(error))
		DAQmxGetExtendedErrorInfo(_errBuff, ERROR_STR_LEN);
	if (DAQmxFailed(error))
		LOG("DAQmx Error: %s\n", _errBuff);
	LOG("Waveform generated\n");
	TIME_END(WriteWaveform)
		return isContinue ? TRUE : FALSE;
}

long DAQController::ConfigTask(double sampleClock)
{
	int         error = 0;

	DAQmxErrChk(DAQmxCreateTask("", &_taskHandleAO0));
	DAQmxErrChk(DAQmxCreateAOVoltageChan(_taskHandleAO0, DEV_GX_OUT_STR, "", VOLT_MIN, VOLT_MAX, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxSetWriteRegenMode(_taskHandleAO0, DAQmx_Val_DoNotAllowRegen));
	//DAQmxErrChk(DAQmxCfgSampClkTiming(_taskHandleAO0, "", sampleClock, DAQmx_Val_Rising, DAQmx_Val_ContSamps, _bufferLength));
	DAQmxErrChk(DAQmxCfgSampClkTiming(_taskHandleAO0, "", sampleClock, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, _pokelsPointCount));

	DAQmxErrChk(DAQmxCreateTask("", &_taskHandleAO1));
	DAQmxErrChk(DAQmxCreateAOVoltageChan(_taskHandleAO1, DEV_GY_P_V_OUT_STR, "", VOLT_MIN, VOLT_MAX, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxSetWriteRegenMode(_taskHandleAO1, DAQmx_Val_DoNotAllowRegen));
	//DAQmxErrChk(DAQmxCfgSampClkTiming(_taskHandleAO1, "", sampleClock, DAQmx_Val_Rising, DAQmx_Val_ContSamps, _bufferLength));
	DAQmxErrChk(DAQmxCfgSampClkTiming(_taskHandleAO1, "", sampleClock, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, _pokelsPointCount));
#ifndef DEBUG_NO_TRIG
	DAQmxErrChk(DAQmxCfgDigEdgeStartTrig(_taskHandleAO0, DEV_GY_TRIG_STR, DAQmx_Val_Rising));
	DAQmxErrChk(DAQmxCfgDigEdgeStartTrig(_taskHandleAO1, DEV_GY_TRIG_STR, DAQmx_Val_Rising));
	DAQmxErrChk(DAQmxSetStartTrigRetriggerable(_taskHandleAO0, 1));
	DAQmxErrChk(DAQmxSetStartTrigRetriggerable(_taskHandleAO1, 1));
#endif
	DAQmxErrChk(DAQmxCfgOutputBuffer(_taskHandleAO0, _bufferLength * BUFFER_COUNT_IN_NI_CARD));
	//DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(_taskHandleAO0, DAQmx_Val_Transferred_From_Buffer, _bufferLength, 0, EveryNCallback, NULL));

	DAQmxErrChk(DAQmxCfgOutputBuffer(_taskHandleAO1, _bufferLength * BUFFER_COUNT_IN_NI_CARD));
	DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(_taskHandleAO0, DAQmx_Val_Transferred_From_Buffer, _bufferLength, 0, EveryNCallback0, NULL));
	DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(_taskHandleAO1, DAQmx_Val_Transferred_From_Buffer, _bufferLength, 0, EveryNCallback1, NULL));
	return TRUE;
Error:
	if (DAQmxFailed(error))
		DAQmxGetExtendedErrorInfo(_errBuff, ERROR_STR_LEN);
	if (DAQmxFailed(error))
		LOG("DAQmx Error: %s\n", _errBuff);
	LOG("ConfigTask end\n");
	return FALSE;
}

long DAQController::StartOutput()
{
	int         error = 0;
	DAQmxErrChk(DAQmxStartTask(_taskHandleAO0));
	DAQmxErrChk(DAQmxStartTask(_taskHandleAO1));
	//_endCount = 0;
	return TRUE;
Error:
	if (DAQmxFailed(error))
		DAQmxGetExtendedErrorInfo(_errBuff, ERROR_STR_LEN);
	if (_taskHandleAO0 != 0) {
		DAQmxStopTask(_taskHandleAO0);
		DAQmxClearTask(_taskHandleAO0);
	}
	if (_taskHandleAO1 != 0) {
		DAQmxStopTask(_taskHandleAO1);
		DAQmxClearTask(_taskHandleAO1);
	}
	if (DAQmxFailed(error))
		LOG("DAQmx Error: %s\n", _errBuff);
	return FALSE;
}
long DAQController::StopAndClear(bool forceStop)
{
	int         errorTask1 = 0;
	int         errorTask2 = 0;
	long ret = TRUE;
	if (forceStop)
	{
		errorTask1 = DAQmxStopTask(_taskHandleAO0);
		errorTask2 = DAQmxStopTask(_taskHandleAO1);
	}
	else
	{
		//bool32 isTaskDone;
		//DAQmxIsTaskDone(_taskHandleAO0, &isTaskDone);
		//if (!isTaskDone)
		//	errorTask1 = DAQmxStopTask(_taskHandleAO0);
		//DAQmxIsTaskDone(_taskHandleAO1, &isTaskDone);
		//if (!isTaskDone)
		//	errorTask2 = DAQmxStopTask(_taskHandleAO1);
		DAQmxWaitUntilTaskDone(_taskHandleAO0, WAIT_TIME);
		DAQmxWaitUntilTaskDone(_taskHandleAO1, WAIT_TIME);
	}
Error:
	if ((DAQmxFailed(errorTask1) && errorTask1 != DAQmxErrorGenStoppedToPreventRegenOfOldSamples)
		|| (DAQmxFailed(errorTask2) && errorTask2 != DAQmxErrorGenStoppedToPreventRegenOfOldSamples))
	{
		DAQmxGetExtendedErrorInfo(_errBuff, ERROR_STR_LEN);
		LOG("DAQmx Error: %s\n", _errBuff);
		ret = FALSE;
	}
	DAQmxClearTask(_taskHandleAO0);
	DAQmxClearTask(_taskHandleAO1);
	_taskHandleAO0 = NULL;
	_taskHandleAO1 = NULL;
	return ret;
}
long DAQController::SetFrameBufferReadyOutput()
{
	if (!_hasDAQController)
		return FALSE;
	return TRUE;
}

long DAQController::StopAllSignals()
{
	if (!_hasDAQController)
		return FALSE;
	LOG("StopAllSignals\n");
	LOG("WaitForSingleObject\n");
	WaitForMultipleObjects(2, _hWritingBuffer, FALSE, 50);
	LOG("WaitForSingleObject end\n");
	bool ret = true;
	if(_isActive)
	{
		if(!StopAndClear())
			ret = false;
		_isActive = false;
	}
	LOG("StopAllSignals end\n");
	ReleaseMutex(_hWritingBuffer[0]);
	ReleaseMutex(_hWritingBuffer[1]);
	return ret;
}

long DAQController::SetFrameTriggerOutLow()
{
	if (!_hasDAQController)
		return FALSE;
	return TRUE;
}

long DAQController::SetupClockMasterClock()
{
	if (!_hasDAQController)
		return FALSE;
	long ret = TRUE;
	return ret;
}

long DAQController::SetupClockMasterDigital()
{
	if (!_hasDAQController)
		return FALSE;
	return TRUE;
}

long DAQController::SetupClockMasterGalvo()
{
	if (!_hasDAQController)
		return FALSE;
	return TRUE;
}

long DAQController::ResetDAQ()
{
	if (!_hasDAQController)
		return FALSE;
	if (_isActive)
	{
		long ret = StopAllSignals();
		if (!ret)
			return FALSE;
	}
	_taskHandleAO0 = NULL;
	_taskHandleAO1 = NULL;
}

int32 CVICALLBACK DAQController::EveryNCallback0(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	LOG("EveryNCallback: %d\n", nSamples);
	WaitForSingleObject(_pInstance->_hWritingBuffer[0], 50);
	int32 written;
	if(_pInstance->_isActive)
	{
		if(_pInstance->_isContinueOutput)
		{
			_pInstance->_isContinueOutput = _pInstance->WriteWaveformCh0(_pInstance->_bufferLength, 1);
			LOG("Write end\n");
		}
	}

	ReleaseMutex(_pInstance->_hWritingBuffer[0]);
	LOG("ReleaseMutex\n");
	return 0;
}
int32 CVICALLBACK DAQController::EveryNCallback1(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	LOG("EveryNCallback: %d\n", nSamples);
	WaitForSingleObject(_pInstance->_hWritingBuffer[1], 50);
	int32 written;
	if (_pInstance->_isActive)
	{
		if (_pInstance->_isContinueOutput)
		{
			_pInstance->_isContinueOutput = _pInstance->WriteWaveformCh1(_pInstance->_bufferLength, 1);
			LOG("Write end\n");
		}
	}

	ReleaseMutex(_pInstance->_hWritingBuffer[1]);
	LOG("ReleaseMutex\n");
	return 0;
}
long DAQController::SetResonance(bool isOpen)
{
	float64 data = 0.0000;
	if (isOpen)
	{
		data = 1.65;
	}
	long ret = TRUE;
	int  error = 0;
	TaskHandle taskHandle = 0;
	DAQmxErrChk(DAQmxCreateTask("", &taskHandle));
	DAQmxErrChk(DAQmxCreateAOVoltageChan(taskHandle, DEV_RES, "", VOLT_MIN, VOLT_MAX, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxStartTask(taskHandle));

	DAQmxErrChk(DAQmxWriteAnalogF64(taskHandle, 1, 1, 10.0, DAQmx_Val_GroupByChannel, &data, NULL, NULL));
	if (taskHandle != 0) {
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	return TRUE;
Error:
	if (DAQmxFailed(error))
		DAQmxGetExtendedErrorInfo(_errBuff, ERROR_STR_LEN);
	if (taskHandle != 0) {
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	if (DAQmxFailed(error))
		LOG("DAQmx Error: %s\n", _errBuff);
	return FALSE;

}
long DAQController::MeasureFrequency(float64 *value)
{
	int         error = 0;
	TaskHandle  taskHandle = 0;
	char        errBuff[2048] = { '\0' };

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk(DAQmxCreateTask("", &taskHandle));
	DAQmxErrChk(DAQmxCreateCIFreqChan(taskHandle, DEV_CTR, "", 1.192093, 10000000.000000, DAQmx_Val_Hz, DAQmx_Val_Rising, DAQmx_Val_LowFreq1Ctr, 0.001, 4, ""));
	DAQmxErrChk(DAQmxSetCIFreqTerm(taskHandle, DEV_CTR, DEV_READ_FREQ));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk(DAQmxStartTask(taskHandle));

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk(DAQmxReadCounterScalarF64(taskHandle, 10.0, value, 0));

	if (taskHandle != 0) {
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	return TRUE;

Error:
	if (DAQmxFailed(error))
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
	if (taskHandle != 0) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	if (DAQmxFailed(error))
		LOG("DAQmx Error: %s\n", _errBuff);
	return FALSE;
}

long DAQController::ReadVoltage(float64* voltGX1, float64* voltGX2, float64* voltGY)
{
	int  error = 0;
	TaskHandle taskHandle = 0;
	double voltage[3];

	DAQmxErrChk(DAQmxCreateTask("", &taskHandle));
	DAQmxErrChk(DAQmxCreateAIVoltageChan(taskHandle, DEV_READ_POS, "", DAQmx_Val_RSE, VOLT_MIN, VOLT_MAX, DAQmx_Val_Volts, NULL));

	int32 samplesRead;
	DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, 1, READ_TIMEOUT, DAQmx_Val_GroupByScanNumber, voltage, 3, &samplesRead, NULL));

	if (taskHandle != 0) {
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}

	*voltGY = voltage[0];
	*voltGX1 = voltage[1];
	*voltGX2 = voltage[2];

	return TRUE;
Error:
	if (DAQmxFailed(error))
		DAQmxGetExtendedErrorInfo(_errBuff, ERROR_STR_LEN);
	if (DAQmxFailed(error))
		LOG("DAQmx Error: %s\n", _errBuff);
	return FALSE;
}

void DAQController::Output(const wchar_t* szFormat, ...)
{
	wchar_t szBuff[1024];
	va_list arg;
	va_start(arg, szFormat);
	_vsnwprintf_s(szBuff, sizeof(szBuff), szFormat, arg);
	va_end(arg);

	OutputDebugString(szBuff);
}