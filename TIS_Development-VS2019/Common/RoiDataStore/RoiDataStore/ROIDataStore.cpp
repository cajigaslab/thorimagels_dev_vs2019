// ROIDataStore.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ROIDataStore.h"
#include "ROIStoreHDF5.h"
#include "ROIStoreSQLite.h"
#include "ROIStorePassthrough.h"
#include "StatsManagerDll.h"

const long MSG_SIZE = 256;
const long MAX_CHAN = 4;
wchar_t message[MSG_SIZE];
std::auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));

bool ROIDataStore::_instanceFlag = false;
std::auto_ptr<ROIDataStore> ROIDataStore::_single(NULL);
std::auto_ptr<IROIDataStore> ROIDataStore::_cDataStore(NULL);

enum
{
	DSTYPE_PASSTHROUGH = 0,
	DSTYPE_SQLITE = 1,
	DSTYPE_HDF5   = 2,	
};

ROIDataStore::ROIDataStore()
{
	_pushFuncPtr = NULL;
	_isReading = FALSE;
}

ROIDataStore::~ROIDataStore()
{
	_instanceFlag = false;
}

void ROIDataStore::CreateROIDataStore(long type = DSTYPE_PASSTHROUGH, char* pathAndName = "")
{
	_cDataStore.reset(NULL);
	switch(type)
	{
	case DSTYPE_PASSTHROUGH:
		_cDataStore.reset(new ROIStorePassthrough());
		break;
	case DSTYPE_SQLITE:
		_cDataStore.reset(new ROIStoreSQLite(pathAndName));
		break;
	case DSTYPE_HDF5:
		_cDataStore.reset(new ROIStoreHDF5(pathAndName));
		break;
	default:
		break;
	}
	
	StringCbPrintfW(message,MSG_SIZE,L"Creating ROIDataStore Singleton");
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	return;
}

ROIDataStore* ROIDataStore::GetInstance()
{
	if(! _instanceFlag)
	{
		try
		{
			_single.reset(new ROIDataStore());
			
			wsprintf(message,L"ROIDataStore Created");
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		catch(...)
		{
			//critically low on resources
			//do not proceed with application
			throw;
		}
		_instanceFlag = true;
		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

void ROIDataStore::StatsFuncPtr(char** statsName, double* stats, long nStats, long bIsLast)
{
	_single->LoadROIData(statsName,stats, nStats, bIsLast);
}

long ROIDataStore::InitCallBack(pushROIDataCallBack pushFuncPtr)
{
	_pushFuncPtr = pushFuncPtr;

	if(_pushFuncPtr != NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ROIDataStore InitCallBack");
	}
	return TRUE;
}

void ROIDataStore::LoadROIData(char** statsName, double* stats, long nStats, long bIsLast)
{
	if(NULL == _cDataStore.get())
	{
		return;
	}
	_cDataStore->SaveROIData(statsName, stats, nStats, bIsLast);	
}

void ROIDataStore::RequestROIData()
{
	//only allow one read request at time
	if (TRUE == _isReading)
	{
		return;
	}
	_isReading = TRUE;

	if(NULL == _cDataStore.get())
	{
		_isReading = FALSE;
		return;
	}
	long isLast = FALSE;
	long isNewData = FALSE;
	double* stats =NULL;
	char** statsNames = NULL;
	long length = 0;
	while(FALSE == isLast)
	{
		_cDataStore->ReadROIData(&statsNames, &stats, length, isLast, isNewData);
		if(FALSE == isNewData)
		{
			_isReading = FALSE;
			return;
		}
		(*_pushFuncPtr)(statsNames, stats, length, isLast); // callback function to the view
	}

	//Set flag to FALSE to allow the next read request
	_isReading = FALSE;
}

void ROIDataStore::PostROIData()
{
}