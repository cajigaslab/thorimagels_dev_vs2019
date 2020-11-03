#pragma once


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


#define DllExport extern "C" void __declspec(dllexport)


DllExport TLTraceEvent(long eventType,long id,LPWSTR str);



