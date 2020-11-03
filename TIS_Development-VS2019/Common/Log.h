
#pragma once
//Log.h
#include ".\PDLL\pdll.h"
//dll wrapper class using the virtual class


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

class ILog
{
public:
	virtual void TLTraceEvent(long eventType,long id, LPWSTR str) = 0;
};

class LogDll : public PDLL, public ILog
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(LogDll)
#pragma warning(pop)

	DECLARE_FUNCTION3(void, TLTraceEvent, long, long, LPWSTR)
};

