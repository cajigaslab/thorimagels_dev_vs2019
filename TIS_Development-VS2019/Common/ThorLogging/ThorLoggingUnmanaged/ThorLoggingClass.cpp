// ThorLoggingClass.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <codecvt>
#include "ThorLoggingUnmanaged.h"
#include "ThorLoggingClass.h"
#include "log.h"



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

std::string ConvertWStringToString(std::wstring ws)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
	return converterX.to_bytes(ws);
}

ThorLoggingClass* ThorLoggingClass::getInstance()
{
	Lock lock(critSect);

	if(! instanceFlag)
	{
		
		try
		{
			single = new ThorLoggingClass();
			FILE *pFile = NULL;
			pFile = _fsopen("ThorLog.log", "a",_SH_DENYWR);
			Output2FILE::Stream() = pFile;

			std::wstring ws = std::wstring(L"ThorLogging.xml");

			ticpp::Document *pDoc = new ticpp::Document(ConvertWStringToString(ws));
			pDoc->LoadFile();

			ticpp::Element *configObj = pDoc->FirstChildElement(false);

			if ( configObj == NULL )
			{
			}
			else
			{
				ticpp::Element *rootObj = configObj->FirstChildElement(false);
				ticpp::Element *priorityObj = rootObj->FirstChildElement(false);

				std::string str;		
				str = priorityObj->GetAttribute("value");	

				FILELog::ReportingLevel() = FILELog::FromString(str);
			}	

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
	int result = EXIT_SUCCESS;

	try
	{
		std::string strLocal = ConvertWStringToString(std::wstring(str));
		switch(eventType)
		{
		case  CRITICAL_EVENT:
			FILE_LOG(logERROR) << strLocal;
			break;
		case ERROR_EVENT:
			FILE_LOG(logERROR) << strLocal;
			break;
		case WARNING_EVENT:
			FILE_LOG(logWARNING) << strLocal;
			break;
		case INFORMATION_EVENT:
			FILE_LOG(logINFO) << strLocal;
			break;
		case VERBOSE_EVENT:
			FILE_LOG(logDEBUG) << strLocal;
			break;
		}
	}
	catch(std::exception&)
	{
		result = EXIT_FAILURE;
	}
}
