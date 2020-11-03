#include "stdafx.h"
#include "ROIStorePassthrough.h"

ROIStorePassthrough::ROIStorePassthrough(void)
{
	_isNewdata = FALSE;
	_numStats = 0;
}


ROIStorePassthrough::~ROIStorePassthrough(void)
{
	//delete existing
	if(NULL != _statsName)
	{
		for(int i=0; i<_numStats; i++)
		{
			CoTaskMemFree(_statsName[i]);
		}

		CoTaskMemFree(_statsName);
		_statsName = NULL;
	}
}

void ROIStorePassthrough::SaveROIData(char** statsName, double* stats, long nStats, long isLast)
{
	//delete existing
	if((NULL != _statsName)&&(_numStats != nStats) )
	{
		for(int i=0; i<_numStats; i++)
		{
			CoTaskMemFree(_statsName[i]);
		}

		CoTaskMemFree(_statsName);
		_statsName = NULL;
	}
	//if there are stats
	if(0 < nStats)
	{
		const long MAX_STATNAME_LEN = 32;
	
		//allocate if needed
		if(NULL == _statsName)
		{
			_statsName = (char**)CoTaskMemAlloc(sizeof(char *) * nStats);

			memset(_statsName,0,sizeof(char *) * nStats);
		
			for(int i=0; i<nStats; i++)
			{
				_statsName[i] =  (char*)CoTaskMemAlloc(MAX_STATNAME_LEN);
			}
		}

		for(int i=0; i<nStats; i++)
		{
			strcpy_s(_statsName[i],MAX_STATNAME_LEN,statsName[i]);
		}
	}
	_stats = stats;
	_numStats = nStats;
	_isNewdata = TRUE; //flag used to check if there has been a new set of data
}



void ROIStorePassthrough::ReadROIData(char*** statsName, double** stats, long &len, long &isLast, long &isNewdata)
{
	if(FALSE == _isNewdata) //if there is no new data return;
	{
		isNewdata = FALSE;
		isLast = TRUE;
		return;
	}
	
	*statsName = _statsName;
	*stats = _stats;
	len = _numStats;

	isLast = TRUE;
	isNewdata = TRUE; //value going back to method caller
	_isNewdata = FALSE; //flag used to check if there has been a new set of data
}

