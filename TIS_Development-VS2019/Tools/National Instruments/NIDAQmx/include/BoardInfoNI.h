#pragma once
#include <memory>
#include <regex>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include "Strsafe.h"
#include "NIDAQmx.h"

#define DAQmxErrChk(fnName,fnCall) if ((error=(fnCall)) < 0){StringCbPrintfW(message,_MAX_PATH,L"DAQMX failed %s Error code %d ",fnName, error);LogMessage(message, ERROR_EVENT); throw "fnCall";}//else{StringCbPrintfW(message,_MAX_PATH,L"DAQMX %s return code %d ",fnName, error); LogMessage(message,VERBOSE_EVENT);}
#define DAQmxFailed(error)              ((error)<0)

#define MAX_TASK_WAIT_TIME		10.0

typedef std::chrono::high_resolution_clock Clock;
using	std::chrono::milliseconds;
using	std::chrono::duration_cast;

//#define LOGGING_ENABLED

#ifdef LOGGING_ENABLED
#include "..\..\..\..\Common\Log.h"
static std::auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#else

enum EventType
{
	// Summary:
	//     Fatal error or application crash.
	CRITICAL_EVENT = 1,
	//
	// Summary:
	//     Recoverable error.
	ERROR_EVENT = 2,
	//
	// Summary:
	//     Noncritical problem.
	WARNING_EVENT = 4,
	//
	// Summary:
	//     Informational message.
	INFORMATION_EVENT = 8,
	//
	// Summary:
	//     Debugging trace.
	VERBOSE_EVENT = 16,

};
#endif

extern wchar_t message[_MAX_PATH];

enum BoardStyle
{
	PCI,
	USB
};

enum TogglePulseMode
{
	Pulse = 0,
	ToggleHigh,
	ToggleLow
};

enum LineTypeNI
{
	FIRST_SIGNALTYPE,
	ANALOG_IN = 0,
	DIGITAL_IN = 1,
	COUNTER_IN = 2,
	LAST_SIGNAL_TYPE
};

struct BoardInfo
{
	std::string	devName;
	std::string	devType;
	BoardStyle	boardStyle;
	long		rtsiConfigure;
};

static std::string GetDevIDName(std::string input)
{
	std::string outString = "";
	std::string strSeperator = "/";
	std::regex baseRegex((std::string::npos == input.rfind(strSeperator, 0) ? "" : strSeperator) + "(.*?)" + strSeperator);	//check if start with "/"
	std::smatch baseMatch;
	if(std::regex_search(input, baseMatch, baseRegex) && (2 == baseMatch.size()))
	{
		outString = baseMatch[1].str();
	}
	return outString;
}

static std::string GetNIDeviceAttribute(std::string devName, int32 attribute)
{
	int buffersize;
	char* input = NULL;
	std::string str="";
	try
	{
		buffersize = DAQmxGetDeviceAttribute(devName.c_str(),attribute,NULL);
		if(buffersize > 0)	//-200604 DAQmxErrorNULLPtr
		{
			input = (char*)malloc(buffersize);
			DAQmxGetDeviceAttribute(devName.c_str(),attribute,input,buffersize);
			str = input;
		}
	}
	catch(...)
	{
	}
	if(input)
	{
		free(input);
		input = NULL;
	}
	return str;
}

static void LogMessage(wchar_t *message, long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, message);
#endif
}

static void TerminateTask(TaskHandle& handle)
{
	if(NULL != handle)
	{
		DAQmxStopTask(handle);
		DAQmxClearTask(handle);
		handle = NULL;
	}
}

/// <summary> send a pulse or toggle designated digital line </summary>
static long TogglePulseToDigitalLine(TaskHandle handle, std::string lineName, long lineCount, TogglePulseMode tpMode, long idleMS = 0)
{
	int32 retVal = 0, error = 0, written = 0;
	uInt8* outHigh = NULL;
	uInt8* outLow = NULL;

	if(0 < lineName.size() && 0 < lineCount)
	{
		if(handle)
		{
			retVal = DAQmxStopTask(handle);
			retVal = DAQmxClearTask(handle);
			handle = NULL;
		}
		outHigh = new uInt8[lineCount];
		outLow = new uInt8[lineCount];
		for (int i = 0; i < lineCount; i++)
		{
			outHigh[i] = 1;
			outLow[i] = 0;
		}
		try
		{
			DAQmxErrChk (L"DAQmxCreateTask",retVal = DAQmxCreateTask("",&handle));
			DAQmxErrChk (L"DAQmxCreateDOChan",retVal = DAQmxCreateDOChan(handle, lineName.c_str(),"",DAQmx_Val_ChanPerLine));
			switch (tpMode)
			{
			case Pulse:
				DAQmxErrChk (L"DAQmxWriteDigitalLines",retVal = DAQmxWriteDigitalLines(handle,1,TRUE,0,DAQmx_Val_GroupByChannel,outHigh,&written,NULL));
				DAQmxErrChk (L"DAQmxWaitUntilTaskDone",retVal = DAQmxWaitUntilTaskDone(handle,MAX_TASK_WAIT_TIME));
				DAQmxErrChk (L"DAQmxWriteDigitalLines",retVal = DAQmxWriteDigitalLines(handle,1,TRUE,0,DAQmx_Val_GroupByChannel,outLow,&written,NULL));
				DAQmxErrChk (L"DAQmxWaitUntilTaskDone",retVal = DAQmxWaitUntilTaskDone(handle,MAX_TASK_WAIT_TIME));
				break;
			case ToggleHigh:
				DAQmxErrChk (L"DAQmxWriteDigitalLines",retVal = DAQmxWriteDigitalLines(handle,1,TRUE,0,DAQmx_Val_GroupByChannel,outHigh,&written,NULL));
				DAQmxErrChk (L"DAQmxWaitUntilTaskDone",retVal = DAQmxWaitUntilTaskDone(handle,MAX_TASK_WAIT_TIME));
				break;
			case ToggleLow:
				DAQmxErrChk (L"DAQmxWriteDigitalLines",retVal = DAQmxWriteDigitalLines(handle,1,TRUE,0,DAQmx_Val_GroupByChannel,outLow,&written,NULL));
				DAQmxErrChk (L"DAQmxWaitUntilTaskDone",retVal = DAQmxWaitUntilTaskDone(handle,MAX_TASK_WAIT_TIME));
				break;
			default:
				break;
			}
			DAQmxStopTask(handle);
			DAQmxClearTask(handle);
			handle = NULL;

			//idle time
			if(0 < idleMS)
				std::this_thread::sleep_for (std::chrono::milliseconds(idleMS)); 

		}
		catch(...)
		{
			DAQmxFailed(error);
			StringCbPrintfW(message,_MAX_PATH,L"%hs failed: (%d)",__FUNCTION__, error);
			LogMessage(message,ERROR_EVENT);
		}
		if (outHigh != NULL) { delete[] outHigh; outHigh = NULL; }
		if (outLow != NULL) { delete[] outLow; outLow = NULL; }
	}
	return retVal;
}

class BoardInfoNI
{
private:///<Private members
	static bool _instanceFlag;
	static std::unique_ptr<BoardInfoNI> _single;
	std::vector<BoardInfo> _boardVec;

	BoardInfoNI(){};

public:
	~BoardInfoNI(){};
	static BoardInfoNI* getInstance();

	void GetAllBoardsInfo();
	BoardInfo* GetBoardInfo(std::string devName);
	long VerifyLineNI(BoardInfo* bInfo, LineTypeNI signalType, std::string lineName);
};

class AnalogReaderNI
{
private:///<Private members
	static bool _instanceFlag;
	static std::unique_ptr<AnalogReaderNI> _single;
	std::unique_ptr<BoardInfoNI> boardsInfo;

	std::vector<std::string> _lineNames;
	std::vector<TaskHandle> _taskHandles;
	std::mutex _lock;

	AnalogReaderNI(){};

public:
	~AnalogReaderNI(){};
	static AnalogReaderNI* getInstance();

	long AddLine(std::string lineName);
	void RemoveLine(std::string lineName);
	long ReadLine(std::string lineName, int count, double* data);
};
