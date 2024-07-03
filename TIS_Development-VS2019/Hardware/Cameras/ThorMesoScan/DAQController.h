#pragma once
#include "stdafx.h"
#include "Types.h"
#define ERROR_STR_LEN (2048)

class DAQController
{
public:
	~DAQController();
	static DAQController * GetInstance()
	{
		if(_pInstance == NULL)
		{
			_pInstance = new DAQController();
		}
		return _pInstance;
	}
	// dynamic waveform call back 
	static int32 EveryNCallback0(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
	static int32 EveryNCallback1(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
	// waveform finished call back
	//static int32 DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);

	long FindDAQController();
	long InitDAQController();
	bool IsAvaliable();
	long SetSampleClock(double sampleClock);
	long SetCaptureActiveOutput(long startOrStop);
	long SetFrameBufferReadyOutput();
	long SetFrameTriggerOutLow();
	long SetupClockMasterClock();
	long SetupClockMasterDigital();
	long SetupClockMasterGalvo();
	long StopAllSignals();
	long ResetDAQ();
	long SetWaveformBuffer(int index, IBuffer* waveformBuffer);
	long SetResonance(bool isOpen);
	long MeasureFrequency(float64 *value);
	long ReadVoltage(float64* voltGX1, float64* voltGX2, float64* voltGY);

private:
	DAQController();
	long StartOutput();
	long WriteWaveformCh0(long length, long count);
	long WriteWaveformCh1(long length, long count);
	long ConfigTask(double rate);
	long StopAndClear(bool forceStop = true);
	static void Output(const wchar_t* szFormatm, ...);

private:
	static DAQController* _pInstance;

	IBuffer* _waveformBufferGYPV;
	IBuffer* _waveformBufferGX12;
	bool _hasDAQController;

	HANDLE _hWritingBuffer[2];

	TaskHandle _taskHandleAO0;  // galvoX1, galvoX2
	TaskHandle _taskHandleAO1;  // galvoY, voice coil, pockel

	long _bufferLength;
	int _pokelsPointCount;
	bool _isActive;
	bool _isContinueOutput;

	//int _endCount;

	char _errBuff[ERROR_STR_LEN];
};

