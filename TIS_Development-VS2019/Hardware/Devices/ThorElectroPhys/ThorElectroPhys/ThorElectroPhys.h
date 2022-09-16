
#pragma once
#include "..\..\..\..\Common\BlockRingBuffer.h"

#define MAX_DIG_PORT_OUTPUT		8
#define DEFAULT_DO_SAMPLE_RATE	20000	//[Hz]

enum EPhysTriggerLines
{
	FIRST_TRIGGER_CONFIG = 0,
	DIGITAL_COUNTER = 0,
	DIGITAL_OUTPUT = 1,
	ANALOG_COUNTER = 2,
	ANALOG_OUTPUT = 3,
	EDGE_OUTPUT = 4,
	BUFFER_OUTPUT = 5,
	CUSTOM_INPUT = 6,
	LAST_TRIGGER_CONFIG
};

enum EPhysModeConfig
{
	NONE_TRIG = 0,
	MANUAL_TRIG = 1,
	EDGE_MONITOR = 2,
	CUSTOM_CONFIG = 3,
	LAST_MODE_CONFIG
};

enum AnalogTaskType
{
	FINITE_LIMITED = 0,
	CONTINUOUS_LIMITED = 1,
	FINITE_UNLIMITED = 2,
	CONTINUOUS_UNLIMITED = 3,
	LAST_ANALOG_TASK_TYPE
};

class ThorElectroPhys : IDevice
{
private:
	ThorElectroPhys();
public:

	static ThorElectroPhys* getInstance();
	~ThorElectroPhys();

	long FindDevices(long& DeviceCount);
	long SelectDevice(const long Device);
	long TeardownDevice();
	long GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double& param);
	long SetParamString(const long paramID, wchar_t* str);
	long GetParamString(const long paramID, wchar_t* str, long size);
	long SetParamBuffer(const long paramID, char* buffer, long size);
	long GetParamBuffer(const long paramID, char* buffer, long size);
	long PreflightPosition();
	long SetupPosition();
	long StartPosition();
	long StatusPosition(long& status);
	long ReadPosition(DeviceType deviceType, double& pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t* errMsg, long size);

private:

	int CloseNITasks();
	void ResetParams();
	static std::string GetTriggerInputLine();
	long SetDigitalTriggerTask();
	static long SetAnalogTriggerTask();
	long SetEdgeMonitorTask();
	long SetTriggerTasks();
	long StartTriggerTasks();
	static void CloseMeasureTasks();
	static void CloseTriggerTasks(long bringDownLines = TRUE);
	static long FillupAvailableBuffer(int id); ///<[0:AO,1:DO] element N callback
	static void GotoAnalog(double value);
	static int32 CVICALLBACK TriggerCOCallback(TaskHandle taskHandle, int32 signalID, void* callbackData);
	static int32 CVICALLBACK EveryNTriggerCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void* callbackData);
	static int32 CVICALLBACK AnalogCycleDoneCallback(TaskHandle taskHandle, int32 status, void* callbackData);
	static UINT ThorElectroPhys::FreqMeasureAsync(void);
private:

	std::string _devName;
	std::string _digitalPort;
	std::string _digitalPortOutput[MAX_DIG_PORT_OUTPUT];
	static std::vector<std::string> _triggerConfig;
	bool _digitalPortOutputAvailable[MAX_DIG_PORT_OUTPUT];
	long _digitalPortState[MAX_DIG_PORT_OUTPUT];

	TaskHandle _taskHandleDI0;
	TaskHandle _taskHandleDO[MAX_DIG_PORT_OUTPUT];
	static TaskHandle _taskTriggerHandle[2]; ///<[0:DO, 1:AO], both do active loading
	static TaskHandle _taskTriggerCO[2]; ///<[0:DO, 1:AO]

	//*********************************************************//
	//***	frequency probe to measure signal frequency		***//
	//*********************************************************//

	std::string _freqCounterLine; ///<counter line for frequency probe, can be at different device if RTSI is configured.
	std::string _freqMeasureLine; ///<frequency probe line
	static TaskHandle _taskTriggerCI;
	static double* _freqMeasure; ///<array of frequencies per probe interval time
	static unsigned int _freqBufSize; ///<buffer size to read from frequency measure task
	static std::unique_ptr<CircularBuffer> _freqCirBuf; ///<averaged measurements of frequency
	static HANDLE _freqThreadStopped; ///<event to control timing on frequency measurement thread
	static HANDLE _freqThread; ///<thread to measure frequency
	double _freqIntervalSec; ///<time interval to probe frequency
	double _freqAveraged; ///<averaged frequency measurement

	//*****************************************************************//
	//***	signal generator to generate aligned digital pulses		***//
	//*****************************************************************//

	BOOL _deviceDetected;
	long _numDevices;
	long _parkAnalogLineAtLastVoltage; ///<Flag to decide wheather to leave the analog output low or high after cyles are done
	double _gotoValues[2]; ///<set fixed values on digital (and) analog channel(s), [0]:digital, [1]:analog

	static bool _instanceFlag;
	static std::auto_ptr<ThorElectroPhys> _single;
	static std::unique_ptr<BlockRingBuffer> _bRingBuf[2];
	static uInt8* _localBuffer;
	static long _localBufferSizeInBytes;
	static long _ringBufferSize; ///<# of blocks in ring buffer, coming from "activeLoadCount" settings
	static long _ringBufferUnit; ///<# of units in one block of ring buffer, converted from "activeUnitMS" settings
	static CRITICAL_SECTION _analogCritSection; ///<critical section control for analog task
	static bool _analogCritSectionInitialized;
	static long _analogRepeatCount; ///<# of repeat count of analog task is done
	static long _analogRestarted; ///<flag of analog task is restarted at CycleDone event
	static std::string _analogCounterInternalOutput; ///<clock source for analog output task, "InternalOutput" of AnalogCounter
	static unsigned long long _targetCount; ///<total trigger counts of finite counter output task, used to determine when to stop edge monitor mode
	static unsigned long long _outputCount; ///<output index of finite counter output task, index of triggering edges
	static AnalogTaskType _analogTaskType; ///<mark analog task is finite [0], continuous but limit [1], or open-end continuous [2]
};