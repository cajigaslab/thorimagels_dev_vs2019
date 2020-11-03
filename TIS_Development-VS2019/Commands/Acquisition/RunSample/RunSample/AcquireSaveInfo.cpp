#include "stdafx.h"
#include "AcquireSaveInfo.h"

AcquireSaveInfo::AcquireSaveInfo()
{
}

AcquireSaveInfo::~AcquireSaveInfo()
{
	_instanceFlag = false;
}

double AcquireSaveInfo::_expStartCount = 0;

bool AcquireSaveInfo:: _instanceFlag = false;

auto_ptr<AcquireSaveInfo> AcquireSaveInfo::_single(new AcquireSaveInfo());

AcquireSaveInfo* AcquireSaveInfo::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new AcquireSaveInfo());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

void AcquireSaveInfo::SetExperimentStartCount()
{
	//add 0 as start time to list
	if(!listDeltaT_h.empty())
	{
		listDeltaT_h.clear();
	}
	listDeltaT_h.push_back(0.0);

	LARGE_INTEGER now_h = {0, 0};
	QueryPerformanceCounter(&now_h);

	_expStartCount = static_cast<double>(now_h.QuadPart);


}

double AcquireSaveInfo::GetExperimentStartCount()
{
	return _expStartCount;
}

long AcquireSaveInfo::SaveTimingToFile(string filename)
{
	//const char *textfile = filename.c_str();
	//std::ofstream timelog;
	//timelog.open(textfile, ios::out | ios::app);

	//for (list<double>::iterator it = listDeltaT_h.begin(); it != listDeltaT_h.end(); it++)
	//{
	//	std::ostringstream sstream;
	//	sstream << *it;
	//	timelog << sstream.str() << "\n";
	//}
	//timelog.close();

	return TRUE;
}


double AcquireSaveInfo::AddTimingInfo()
{

	// get the time counter since the start of the experiment
	LARGE_INTEGER now_h = {0, 0};
	QueryPerformanceCounter(&now_h);
		
	double deltaCounts_h = (now_h.QuadPart - GetExperimentStartCount());
	LARGE_INTEGER timerFreq = {0, 0};
	QueryPerformanceFrequency(&timerFreq);
	double deltaT_h = 0;
	if(timerFreq.QuadPart != 0)
	{
		deltaT_h= static_cast<double>(deltaCounts_h / timerFreq.QuadPart);
	}
	else
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"RunSample AcquireSaveInfo DivideByZero Exception occoured");
		return FALSE;
	}
	listDeltaT_h.push_back(deltaT_h);

	return deltaT_h;
}

double AcquireSaveInfo::RemoveTimingInfo()
{
	double ret = 0;
	if(!listDeltaT_h.empty())
	{
		ret = listDeltaT_h.front();
		listDeltaT_h.pop_front();
	}
		
	return ret;
}

bool AcquireSaveInfo::ClearTimingInfo()
{
	if(listDeltaT_h.empty())
		return false;

	listDeltaT_h.clear();
	return true;
}

string AcquireSaveInfo::AddTimestamp()
{
	const int MAX_TIME_LENGTH = 30;
	char acquiredDateTime[MAX_TIME_LENGTH];

	SYSTEMTIME sysTime;
	long timeZoneBias=0;

	GetSystemTime(&sysTime);
//#if (WINVER >= 0x0600)
//				DYNAMIC_TIME_ZONE_INFORMATION timeZoneInfo;
//				GetDynamicTimeZoneInformation(&timeZoneInfo); //get the timezone, systemtime, etc
//				timeZoneBias = timeZoneInfo.Bias / 60; // standard time, Daylight Time is not supported for now
//#endif

	if(timeZoneBias >= 0)
	{
		StringCbPrintfA(acquiredDateTime,MAX_TIME_LENGTH,"%4d-%02d-%02dT%02d:%02d:%02d.%03d-%02d:00",sysTime.wYear,sysTime.wMonth,sysTime.wDay,
			sysTime.wHour,sysTime.wMinute,sysTime.wSecond,sysTime.wMilliseconds, timeZoneBias);
	}
	else
	{
		StringCbPrintfA(acquiredDateTime,MAX_TIME_LENGTH,"%4d-%02d-%02dT%02d:%02d:%02d.%03d+%02d:00",sysTime.wYear,sysTime.wMonth,sysTime.wDay,
			sysTime.wHour,sysTime.wMinute,sysTime.wSecond,sysTime.wMilliseconds, timeZoneBias);
	}

	timestamps.push_back(acquiredDateTime);

	return string(acquiredDateTime);
}

string AcquireSaveInfo::RemoveTimestamp()
{
	string ret = "";
	if(!timestamps.empty())
	{
		ret = timestamps.front();
		timestamps.pop_front();
	}
		
	return ret;
}

bool AcquireSaveInfo::ClearTimestamps()
{
	if(timestamps.empty())
	{
		return false;
	}

	timestamps.clear();
	return true;
}




void AcquireSaveInfo::RemoveTimestampAt(long i)
{
	if(!timestamps.empty())
	{
		std::list<string>::iterator it;

		it = timestamps.begin();
		advance(it,i);

		timestamps.erase(it);
	}
}

void AcquireSaveInfo::RemoveTimingInfoAt(long i)
{	
	if(!listDeltaT_h.empty())
	{
		std::list<double>::iterator it;

		it = listDeltaT_h.begin();
		advance(it,i);

		listDeltaT_h.erase(it);
	}
}