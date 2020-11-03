#pragma once
#include "ROIDataStore.h"
#include "stdafx.h"
class ROIStorePassthrough : public IROIDataStore
{
public:
	ROIStorePassthrough();
	~ROIStorePassthrough();
	
	void SaveROIData(char** statsName, double* stats, long nStats, long isLast);
	void ReadROIData(char*** statsName, double** stats, long &len, long &isLast, long &isNewdata);
protected:
	
private:
	char** _statsName;
	double* _stats;
	long _numStats;
	long _isNewdata;
};