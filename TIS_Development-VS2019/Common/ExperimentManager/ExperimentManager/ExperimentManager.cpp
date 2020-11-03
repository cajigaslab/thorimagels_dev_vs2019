// ExperimentManger.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ExperimentXML.h"
#include "ExperimentManager.h"

auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
wchar_t message[MSG_LENGTH];

ExperimentManager::ExperimentManager()
{    

}

ExperimentManager::~ExperimentManager()
{
	instanceFlag = false;

}


bool ExperimentManager::instanceFlag = false;

auto_ptr<ExperimentManager> ExperimentManager::_single(new ExperimentManager());

//IExperiment* ExperimentManager::activeExperiment = NULL;

auto_ptr<ExperimentXML> ExperimentManager::activeExperiment(new ExperimentXML());

CritSect ExperimentManager::critSect;

ExperimentManager* ExperimentManager::getInstance()
{
	Lock lock(critSect);

	if(! instanceFlag)
	{
		try
		{
			_single.reset(new ExperimentManager());
		}
		catch(...)
		{
			//critically low on resources
			//do not proceed with application
			throw;
		}
		instanceFlag = true;
	}
	return _single.get();
}

IExperiment * ExperimentManager::CreateExperiment(wstring path)
{
	Lock lock(critSect);

	StringCbPrintfW(message,MSG_LENGTH,L"ExperimentManager CreateExperiment %S",path.c_str());
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	activeExperiment->CreateConfigFile(WStringToString(path));

	return activeExperiment.get();
}

long ExperimentManager::DeleteExperiment(string path)
{	
	Lock lock(critSect);

	return TRUE;

}

IExperiment * ExperimentManager::GetExperiment(wstring path)
{
	Lock lock(critSect);

	StringCbPrintfW(message,MSG_LENGTH,L"ExperimentManager GetExperiment %s",path.c_str());
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	activeExperiment->OpenConfigFile(WStringToString(path));

	return activeExperiment.get();	
}

long ExperimentManager::SetActiveExperiment(wstring path)
{
	Lock lock(critSect);

	StringCbPrintfW(message,MSG_LENGTH,L"ExperimentManager SetActiveExperiment %s",path.c_str());
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	activeExperiment->OpenConfigFile(WStringToString(path));

	StringCbPrintfW(message,MSG_LENGTH,L"ExperimentManager SetActiveExperiment completed");
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	return TRUE;
}

IExperiment * ExperimentManager::GetActiveExperiment()
{
	StringCbPrintfW(message,MSG_LENGTH,L"ExperimentManager GetActiveExperiment");
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	return activeExperiment.get();
}

wstring ExperimentManager::GetActiveExperimentPath()
{
	wstring ws;

	ws = StringToWString(activeExperiment->GetPath());
	return ws;
}

wstring ExperimentManager::GetActiveExperimentPathAndName()
{
	wstring ws;
	activeExperiment->GetPathAndName(ws);
	return ws;
}

long  ExperimentManager::GetTotalSampleRowsAndColumns(IExperiment::SampleType type, long &totalRows,long &totalCols)
{
	switch(type)
	{		
	case IExperiment::WELLTYPE_6:
		{
			totalCols = 3;
			totalRows = 2;
		}
		break;
	case IExperiment::WELLTYPE_24:
		{
			totalCols = 6;
			totalRows = 4;
		}
		break;
	case IExperiment::WELLTYPE_96:
		{
			totalCols = 12;
			totalRows = 8;
		}
		break;
	case IExperiment::WELLTYPE_384:
		{
			totalCols = 24;
			totalRows = 16;
		}
		break;
	case IExperiment::WELLTYPE_1536:
		{
			totalCols = 48;
			totalRows = 32;
		}
		break;
	case IExperiment::WELLTYPE_SLIDE:
		{
			totalCols = 1;
			totalRows = 1;
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}
