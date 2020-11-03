#pragma once
#include ".\PDLL\pdll.h"

//***********************************************************************//
//*** Abstract class for signal IO Device Control,					  ***//
//*** including input/output of Analog/Digital/counter channel types, ***//
//*** allow setting of sampleRate, sampleMode (finite/Contineous),	  ***//
//*** and Hardware trigger  ...etc.								      ***//
//***********************************************************************//

typedef signed long (__cdecl *EveryNSamplesEventCallbackPtr)(void* taskHandle, signed long everyNsamplesEventType, unsigned long nSamples, void *callbackData);
typedef signed long (__cdecl *DoneEventCallbackPtr)(void* taskHandle, signed long status, void *callbackData);
typedef signed long (__cdecl *SignalEventCallbackPtr)(void* taskHandle, signed long signalID, void *callbackData);

class IDAQ
{
public:
	enum DAQType
	{
		DAQ_FIRST_TYPE,
		NI,
		DIGITIZER
	};

	enum SignalType
	{	
		SIGNAL_FIRST_TYPE,
		ANALOG_IN,
		ANALOG_OUT,
		DIGITAL_IN,
		DIGITAL_LINE_OUT,
		DIGITAL_PORT_OUT,
		COUNTER_IN,
		COUNTER_OUT,
		PFI_IN,
		PFI_OUT
	};
	
	enum TriggerType
	{
		TRIGGER_FIRST_TYPE,
		TRIGGER_DIG,
		TRIGGER_ANLG
	};

	enum DAQParams
	{
		DAQ_FIRST_PARAM = 0,

		DAQ_NAME = 1,
		DAQ_TYPE,
		DAQ_ANALOG_CNT,
		DAQ_DIGITAL_CNT,
		DAQ_COUNTER_CNT,
		DAQ_TOTAL_TASKCNT,

		DAQ_LAST_PARAM
	};

	enum TaskParams
	{
		TASK_FIRST_PARAM = 0,
				
		TASK_NAME,
		TASK_HANDLE,
		TASK_LINE,
		TASK_SIGNAL_TYPE,
		TASK_USE_DEFAULT,
		TASK_DATA_PTR,
		TASK_DATA_LENGTH,
		TASK_SAMPLE_LINE,
		TASK_TERM_LINE,
		TASK_SAMPLE_RATE,
		TASK_SAMPLE_MODE,
		TASK_SAMPLE_LENGTH,
		TASK_MIN_RANGE,
		TASK_MAX_RANGE,
		TASK_WAITTIME,
		TASK_AUTOSTART,
		TASK_DATALAYOUT,
		TASK_DATAUNIT,
		TASK_IDLESTATE,
		TASK_DELAY,
		TASK_DUTYCYCLE,
		TASK_TERM_CONFIG,
		TASK_SAMPLE_EXE_CNT,

		TASK_TRIGGER_ENABLE,
		TASK_TRIGGER_LINE,
		TASK_TRIGGER_TYPE,
		TASK_TRIGGER_EDGE,
		TASK_TRIGGER_LEVEL,
		TASK_RETRIGGER,
		TASK_ARM_LINE,
		TASK_ARMSTART_TYPE,
		TASK_ARMSTART_EDGE,
		TASK_EVENT_DONE_CALLBACK,
		TASK_EVENT_NSAMPLES_CALLBACK,
		TASK_EVENT_SIGNAL_CALLBACK,
		TASK_EVENT_DONE_CALLBACK_DATA,
		TASK_EVENT_NSAMPLES_CALLBACK_CNT,
		TASK_EVENT_NSAMPLES_CALLBACK_TYPE,
		TASK_EVENT_NSAMPLES_CALLBACK_DATA,
		TASK_EVENT_SIGNAL_CALLBACK_DATA,
		TASK_EVENT_SIGNAL_CALLBACK_ID,
	
		TASK_LAST_PARAM
	};

	enum AttrParams
	{
		FIRST_ATTR_PARAM,
		
		UNIT_VOLT,
		UNIT_HZ,

		TERM_CONFIG_RSE,
		TERM_NO_INVERT,
		TERM_INVERT,
		TERM_INI_DELAY,
		TERM_ENABLE_INI_DELAY_RETRIGGER,

		GROUP_BY_CHANNEL,
		GROUP_BY_SCANNUMBER,
		
		//sample mode:
		SAMPLE_CONTINUOUS,
		SAMPLE_FINITE,
		SAMPLE_HWSINGLEPOINT,

		//signal:
		EDGE_RISING,
		EDGE_FALLING,
		STATE_HIGH,
		STATE_LOW,
		COUNT_UP,
		COUNT_DOWN,

		//event:
		EVENT_SIGNALID_SAMPLECLK,

		LAST_ATTR_PARAM
	};
	//virtual functions:
	//iDAQ Device:
	virtual long EnterDAQ() = 0; ///<get exclusive control of iDAQ device
	virtual long LeaveDAQ() = 0; ///<leave exclusive control of iDAQ device
	virtual long TryEnterDAQ() = 0; ///<try to get exclusive control of iDAQ device
	virtual long FindDAQs(long &daqCount) = 0;///<returns the number of iDAQ devices
	virtual long SelectDAQ(const char* DAQName) = 0;///<index of the iDAQ device to attach to
	virtual long TeardownDAQ() = 0;///<release iDAQ device and its resources
	virtual long GetParam(const long dparamID, double &param) = 0;
	virtual long GetParamString(const long dparamID, char* str, long size) = 0;
	//iDAQ Task:
	virtual long CreateTask(const char* taskName) = 0;///<Create a task of task name
	virtual long GetTask(const char* taskName) = 0;///<Find the task with task name
	virtual long GetTaskParam(const long tparamID, double &param) = 0;///<Get value of the task param
	virtual long SetTaskParam(const long tparamID, const double param) = 0;///<Set value of the task param
	virtual	long GetTaskParamBuffer(const long tparamID, char* buffer, long size) = 0;///<Get buffer of the task param
	virtual	long SetTaskParamBuffer(const long tparamID, char* buffer, long size) = 0;///<Set buffer of the task param
	virtual long GetTaskParamString(const long tparamID, char* str, long size) = 0;///<Get string of the task param
	virtual long SetTaskParamString(const long tparamID, const char* str) = 0;///<Set string of the task param
	virtual long SetTaskChanAttribute(double chanAttrValue) = 0;///<Set custom param not on the param list
	virtual long ArmTask() = 0;///<Arming the task by arm line if exist
	virtual long RegisterTask() = 0;///<Register task with callback function
	virtual long TriggerTask() = 0;///<Triggering the task by trigger line if exist
	virtual long SampleTask() = 0;///<Set buffered task with sample clock
	virtual long ReserveTask(long reserve) = 0;///<Reserve the task
	virtual long SetupTask() = 0;///<Setup the task
	virtual long StartTask() = 0;///<Start the task
	virtual long StopTask() = 0;///<Stop the task
	virtual long ClearTask() = 0;///<Clear the task
	virtual long ClearAllTask() = 0;///<Clear all tasks
	//none task-related functions:
	virtual long ConnectTerminals(long connect, const char* LineName1, const char* LineName2) = 0;///<(dis)connect two lines
};

class iDAQFactory
{
public:
	virtual IDAQ* GetDAQInstance(long type, const char *name) = 0;
};

class DAQDll : public PDLL, public IDAQ, public iDAQFactory
{
	//call the macro and pass your class name
	DECLARE_CLASS(DAQDll)
	//use DECLARE_FUNCTION4 since this function has 4 parameters
	
	DECLARE_FUNCTION2(IDAQ*, GetDAQInstance, long, const char *)
	
	DECLARE_FUNCTION0(long, EnterDAQ)	
	DECLARE_FUNCTION0(long, LeaveDAQ)	
	DECLARE_FUNCTION0(long, TryEnterDAQ)	
	DECLARE_FUNCTION1(long, FindDAQs, long &)
	DECLARE_FUNCTION1(long, SelectDAQ, const char *)	
	DECLARE_FUNCTION0(long, TeardownDAQ)
	DECLARE_FUNCTION2(long, GetParam, const long, double &)
	DECLARE_FUNCTION3(long, GetParamString, const long, char *, long)
	DECLARE_FUNCTION1(long, CreateTask, const char *)
	DECLARE_FUNCTION1(long, GetTask, const char *)
	DECLARE_FUNCTION2(long, GetTaskParam, const long, double &)
	DECLARE_FUNCTION2(long, SetTaskParam, const long, const double)
	DECLARE_FUNCTION3(long, GetTaskParamBuffer, const long, char *, long)
	DECLARE_FUNCTION3(long, SetTaskParamBuffer, const long, char *, long)
	DECLARE_FUNCTION3(long, GetTaskParamString, const long, char *, long)
	DECLARE_FUNCTION2(long, SetTaskParamString, const long, const char *)
	DECLARE_FUNCTION1(long, SetTaskChanAttribute, double)
	DECLARE_FUNCTION0(long, ArmTask)
	DECLARE_FUNCTION0(long, RegisterTask)
	DECLARE_FUNCTION0(long, TriggerTask)
	DECLARE_FUNCTION0(long, SampleTask)
	DECLARE_FUNCTION1(long, ReserveTask, long)
	DECLARE_FUNCTION0(long, SetupTask)
	DECLARE_FUNCTION0(long, StartTask)
	DECLARE_FUNCTION0(long, StopTask)
	DECLARE_FUNCTION0(long, ClearTask)
	DECLARE_FUNCTION0(long, ClearAllTask)
	DECLARE_FUNCTION3(long, ConnectTerminals, long, const char *, const char *)

};
