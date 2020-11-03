#pragma once

#include "ROIDataStore.h"
#include <map>
#include <set>
#include "..\..\..\Common\HDF5IOdll.h"

class ROIStoreHDF5 : public IROIDataStore
{
public:
	ROIStoreHDF5(char* pathAndName);
	~ROIStoreHDF5();
	void SaveROIData(char **, double *, long, long);
	void ReadROIData(char*** nameChain, double** values, long &len, long &isLast, long &isNewData);
private:
	std::map<std::string, long> _dimsMap;
	std::map<std::string, long> _varsMap;
	std::wstring _fileName;
	char** _statsName;
	double* _stats;
	long _numStats;
	long _isNewdata;
};
