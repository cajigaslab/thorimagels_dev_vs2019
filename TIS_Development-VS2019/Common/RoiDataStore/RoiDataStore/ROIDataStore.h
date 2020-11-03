
#pragma once

#include "stdafx.h"
#include "IROIDataStore.h"
#include "ROIElement.h"


typedef void (_cdecl *pushROIDataCallBack)(char** statsName, double* stats, long &nStats, long &bIsLast);
typedef void (_cdecl *pullROIDataCallBack)(char** statsName, double* stats, long &nStats, long &bIsLast);

class ROIDataStore 
{
public:
	~ROIDataStore();
	static void CreateROIDataStore(long type, char* pathAndName);
	static ROIDataStore* GetInstance();
	static void StatsFuncPtr(char** statsName, double* stats, long nStats, long isLast);
	void LoadROIData(char** statsName, double* stats, long nStats, long bIsLast);
	void PostROIData();
	long InitCallBack(pushROIDataCallBack pushFuncPtr);
	void RequestROIData();	
	void ReadROIData(char** nameChain, double* values, long* len);
protected:
	ROIDataStore();
	std::vector<PROI_ELEMENT> ROIData;
private:
	static bool _instanceFlag;
	static std::auto_ptr<ROIDataStore> _single;
	static std::auto_ptr<IROIDataStore> _cDataStore;
	pushROIDataCallBack _pushFuncPtr;	
	long _frame;
	long _isReading;
};


