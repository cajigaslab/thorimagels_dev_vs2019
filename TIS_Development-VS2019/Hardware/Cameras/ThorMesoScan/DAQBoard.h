#pragma once
#include <stdio.h>
#include "Types.h"
#include "..\..\..\Tools\National Instruments\NIDAQmx\include\BoardInfoNI.h"
#include <vector>
#define ERROR_STR_LEN (2048)

enum CHANNEL_TYPE
{
	AO,
	AI,
	DO,
	DI,
	CO,
	CI
};

struct NITask
{
	TaskHandle taskHandle;
	void* pBuffer;
	unsigned int channelCount;
	HANDLE hWritingMutex;
	char channel[100];
	CHANNEL_TYPE channelType;
	char name[100];
	NITask(TaskHandle tHandle, void* pIBuffer, unsigned int cCount, HANDLE handle, const char* channelStr, CHANNEL_TYPE type, const char* nameStr)
	{
		taskHandle = tHandle; pBuffer = pIBuffer; channelCount = cCount; hWritingMutex = handle; channelType = type;
		strcpy_s(channel, channelStr);
		strcpy_s(name, nameStr);
	}
};
class DAQBoard
{
public:
	static DAQBoard* GetInstance();
	~DAQBoard();
	long InitDAQBoard();

	// line scan
	long SetSampleClock(double sampleClock, long samplesPerLineTrigger);
	long RigisterTasks(const char* name, const char* channel, CHANNEL_TYPE type, void* pBuffer, unsigned int channelCount);
	long PreStartTasks();
	long StarTasks();
	long ClearTasks();
	// single control
	long InvokeTask(const char* channel, CHANNEL_TYPE type, void* pBuffer, uint32_t size, double rate);
	long AysncInvokeTask(const char* channel, CHANNEL_TYPE type, void* pBuffer, uint32_t size);

	long StopAllTasks();
	long InitRuningTasks(CHANNEL_TYPE type);
	long MeasureFrequency(const char* channel, const char* data, float64 *value);
	long ReadVoltages(const char* channel, float64* voltages, unsigned int channelCount);
	bool IsRunning() { return _isRunning; }

	// dynamic waveform call back 
	static int32 EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
	static int32 CVICALLBACK HWTriggerCallback(TaskHandle taskHandle, int32 signalID, void *callbackData);

	//***	additional members & functions	***//

	std::unique_ptr<BoardInfoNI> _boardInfoNI;

	long CheckTask(const char* channel);

private:
	DAQBoard();
	long WriteWaveform(NITask* task, long length);

	TaskHandle CreateTask(const char* name, const char* channel, CHANNEL_TYPE type, void* pBuffer, unsigned int channelCount);
	static std::unique_ptr<DAQBoard> _pInstance;
	static std::once_flag _onceFlag;
	double _sampleClock;
	long _samplesPerLineTrigger;

	char _errBuff[ERROR_STR_LEN];
	vector<NITask*> _runningTasks;
	bool _isRunning;
};
