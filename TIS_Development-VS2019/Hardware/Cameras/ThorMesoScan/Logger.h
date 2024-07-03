#pragma once
#include <memory>

class Logger
{
public:
	static Logger& getInstance()
	{
		static Logger instance; // Guaranteed to be destroyed. Instantiated on first use.
		return instance;
	};
	//error message log
	static void LogMessage(wchar_t *logMsg, long eventLevel = 2)
	{
#ifdef LOGGING_ENABLED
		logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
	};
	~Logger(){};
private:
	Logger(){};
};

