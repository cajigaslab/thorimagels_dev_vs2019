#pragma once
#include "stdafx.h"
#include "AcquireData.h"
#include "RealTimeDataXML.h"

#define DAQmxErrChk(fnName,fnCall) if ((error=(fnCall)) < 0){StringCbPrintfW(message,MSG_LENGTH,L"DAQMX failed %s Error code %d ",fnName, error); LogMessage(message,ERROR_EVENT);RunTimeException ex = RunTimeException(std::wstring(fnName),error); throw ex;}

typedef std::vector<TaskHandle>  taskHandles;

//**************************************************************************************//
//*** class to implement interface of IAcquireRealTimeData for NI PCIe-6363/21 card. ***//
//*** All channels will be synchronized on primarySampleTask, 						 ***//
//*** Start(), Stop(): start or stop acquisition, will save to file in SaveThread	 ***//
//*** if file name is defined, otherwise, will only update display buffer for GUI.	 ***//
//*** Pause(), Restart(): stop or start all tasks except global counter. 			 ***//
//*** GetAcquiring(), GetDone(): get to know the status of acquisition.			 	 ***//
//**************************************************************************************//

class AcquireNIData : public IAcquireRealTimeData
{	
private:	//members:
	static taskHandles CItaskHandles;
	static TaskHandle AItaskHandle;
	static TaskHandle DItaskHandle;
	static TaskHandle DOtaskHandle;
	static TaskHandle AOtaskHandle;
	static TaskHandle primarySampleTask;
	static TaskHandle asyncHWTrigTask;
	static char sampleChannel[_MAX_FNAME];	
	static std::string triggerChannel;
	static long _isSaving;
	static bool _retrig;
	static bool _analogTrig;
	static bool _pauseTrig;
	static bool _invertEnabled;
	static std::vector<std::string> _invLines;
	static std::string _primaryChanType;
	static std::string _pmtShutter;
	uInt64 _samplesPerChannel;

	std::vector<std::string> _ailineName;
	std::vector<std::string> _dilineName;

public:		//members
	static std::vector<std::string> counterNames;
	static int32 sampleMode;
	static volatile size_t _saveThreadCnt;
	static volatile size_t _saveThreadFinishedCnt;
	static volatile size_t _totalNumThreads;
	static BOOL _inSaveThread;
	static std::list<HANDLE> _hSaveThreads;
	static HANDLE _hProcessSaveThread;
	static HANDLE _hAsyncThread;
	static HANDLE _hProcessAsyncThread;
	static uInt64 _samplesPerCallback;
	static uInt64 _displayPerCallback;
	static CompoundData* _displayCompData;
	static BOOL _isAcquiring;
	static BOOL _isAsyncAcquiring;

	static unsigned long gCtrOverflowCnt;
	static StimulusSaveStruct* stimulusParams;
	static long interleave;
	static long _nAverage;
	static long _nAverageCnt;

	static time_t triggerTime;
	static HANDLE _hHWTriggerAsyncEvent;	///<event to HW trigger Async
	static HANDLE _hStopAsyncEvent;			///<event to stop Async
	static HANDLE _hAsyncEventFinished;		///<event to signal Async finished
	static HANDLE _hAsyncCallbackInvoked;   ///<event to signal Async callback is invoked
	static unsigned long lastCountValue; // to check increment when writing frame timing file
	static std::wstring ciLogFile; // file name for writing frame timing
	static unsigned int ciLogSuffix; // suffix of file name
	static uInt64 elapsedTimeUS; // for recording frame timing

public:		//functions:
	AcquireNIData();
	//AcquireNIData(BoardInfo* boardinfo,std::vector<Channels>* channel,Mode* mode);
	~AcquireNIData();
	//int32 GetTerminalNameWithDevPrefix(TaskHandle taskHandle, const char terminalName[], char triggerName[]);
	static int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
	static int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);
	static int32 CVICALLBACK EveryNBleachCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
	static int32 CVICALLBACK HWTriggerAsyncCallback(TaskHandle taskHandle, int32 signalID, void *callbackData);
	static long GetErrorMessage();
	static std::string GetNIDeviceAttribute(std::string devName, int32 attribute);
	static long SetDataTransferMech(TaskHandle taskhandle, SignalType sType);
	static long StopTaskArray(taskHandles &handles, long destroy);
	static long StopTask(TaskHandle &handle, long destroy);
	//static long CopyStructData(void* ptr);
	static void pmtShutterTask(uInt8 value);
	static long writeDigitalOutputTask(std::string lineName, std::string sampleLine, uInt32 val, int length, uInt8* array);
	static UINT createWriteDigitalOutputThread();
	static UINT writeDigitalOutputProc();
	static long ResetBleachHWTrigger();
	static long InvertLines(bool connect);
	static long VerifyNIBoard(BoardInfo* board);
	static long VerifyNILine(Channels* channel, int customType, std::string targetName);
	static long StaticStop();
	static long EndTasks();
	static std::string CalcTimeString(uInt64 deltaTimeUS);

	//virtual functions' implementation:
	virtual long Enter();
	virtual long Exit();
	virtual long SetupChannels();
	virtual long SetupFileIO();
	virtual long Start();
	virtual long Pause();
	virtual long Restart();
	virtual long Stop();
	virtual long GetAcquiring();
	virtual long GetAsyncAcquiring();
	virtual long GetSaving();
	virtual long SetSaving(long toSave);
	virtual long InitCallbacks(SpectralUpdateCallback su, DataUpdateCallback du);
	virtual long CopyStructData(void* ptr);
	virtual long Status();
	virtual long LoadXML();
	virtual long StartAsync();
	virtual long StopAsync();
	virtual std::wstring GetLastError();

private:	//functions:
	void DeleteCriticalSections();
	void InitializeCriticalSections();
	long StartTaskArray(taskHandles handles);	
	long StartTask(TaskHandle handle);
	long SetupHDF5File();
	long SetupLineNames();
	long SetupGlobalChannels();
	long SetupLocalChannels();
	long SetupCtrContineousMode();
	long SetupCtrFiniteMode();
	long SetupArmStartTrigger(TaskHandle taskhandle,const char* lineName);
	long SetupStartTrigger(TaskHandle taskhandle,bool retrig);
	long SetupPauseTrigger(TaskHandle taskhandle,bool pauseTrig);
	long SetupCfgBufferForContTasks();
	long SetSamClkBehavior(TaskHandle taskhandle);
	long ExecuteTasks();
	long Execute();
	void ResetParams();
	void ResetTimingParams();
	void ResetAsyncParams();
	AsyncParams* AllocAsyncParams();
};
