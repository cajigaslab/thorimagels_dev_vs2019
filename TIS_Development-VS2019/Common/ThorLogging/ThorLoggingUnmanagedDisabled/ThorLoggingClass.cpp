// ThorLoggingClass.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "stdio.h"
#include "ThorLoggingUnmanagedDisabled.h"
#include "ThorLoggingClass.h"

#ifndef DISABLE_LOG4CXX

#pragma comment(lib,"apr")
#pragma comment(lib,"aprutil")
#pragma comment(lib,"xml")
#pragma comment(lib,"log4cxx")

#include <log4cxx\logger.h> 
#include <log4cxx\xml\domconfigurator.h>
#include <windows.h>

using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;	


static LoggerPtr loggerToFile(Logger::getLogger( L"ThorLogger" ));

#endif

ThorLoggingClass::ThorLoggingClass()
{     

}

ThorLoggingClass::~ThorLoggingClass()
{
}

bool ThorLoggingClass::instanceFlag = false;

ThorLoggingClass* ThorLoggingClass::single = NULL;

CritSect ThorLoggingClass::critSect;

void ThorLoggingClass::cleanup(void)
{
	Lock lock(critSect);

	if(single != NULL)
	{
		delete single;
	}
}

ThorLoggingClass* ThorLoggingClass::getInstance()
{
	Lock lock(critSect);

	if(! instanceFlag)
	{
		
		try
		{
			single = new ThorLoggingClass();
#ifndef DISABLE_LOG4CXX
			DOMConfigurator::configure(L".\\ThorLogging.xml");
#endif
					}
		catch(...)
		{
			//critically low on resources
			//do not proceed with application
			throw;
		}
		instanceFlag = true;
		atexit(cleanup);
		return single;
	}
	else
	{
		return single;
	}
}

void ThorLoggingClass::TraceEvent(EventType eventType,int id,LPWSTR str)
{
#ifndef DISABLE_LOG4CXX
	int result = EXIT_SUCCESS;

	try
	{
		switch(eventType)
		{
		case  CRITICAL_EVENT:
			LOG4CXX_FATAL(loggerToFile, str);
			break;
		case ERROR_EVENT:
			LOG4CXX_ERROR(loggerToFile, str);
			break;
		case WARNING_EVENT:
			LOG4CXX_WARN(loggerToFile, str);
			break;
		case INFORMATION_EVENT:
			LOG4CXX_INFO(loggerToFile, str);
			break;
		case VERBOSE_EVENT:
			LOG4CXX_DEBUG(loggerToFile, str);
			break;
		}
	}
	catch(std::exception&)
	{
		result = EXIT_FAILURE;
	}
#else

	bool LogItem = false;
	bool NeedsNewline = false;

	switch(eventType)
	{
		case  CRITICAL_EVENT:
		case ERROR_EVENT:
			LogItem = true;
			break;

		case WARNING_EVENT:		
		case INFORMATION_EVENT:
		case VERBOSE_EVENT:		
		default:		
			break;

	}

	NeedsNewline = (NULL == wcsrchr(str, '\n'));

	OutputDebugString (str);
	if (NeedsNewline) {
		OutputDebugString (L"\n");
	}

	if (LogItem) {

		FILE		*LogFile		= NULL;
		errno_t		LogFileErrno	= 0;

		DWORD EnvResult = GetEnvironmentVariable (L"THORIMAGELS_LOG", NULL, 0);
		DWORD dwResult  = GetLastError ();

		if ((EnvResult > 0) && (ERROR_SUCCESS == dwResult)) {

			LogFileErrno = fopen_s (&LogFile, "ThorImageLS.log", "a+t");

			if ((0 == LogFileErrno) && (NULL != LogFile)) {

				if (NeedsNewline) {
					fprintf (LogFile, "%S\n", str); 
				} else {
					fprintf (LogFile, "%S", str); 
				}

				fclose (LogFile);

			} else {
				OutputDebugString (L"Error opening ThorTSI.log");
			}

		}

	}

#endif
}
