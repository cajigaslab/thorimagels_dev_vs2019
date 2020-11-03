#pragma once

#include "ROIDataStore.h"
#include "stdafx.h"

class ROIStoreSQLite : public IROIDataStore
{
public:
	ROIStoreSQLite(char* pathAndName);
	~ROIStoreSQLite();
	
	void SaveROIData(char** statsName, double* stats, long nStats, long isLast);
	void ReadROIData(char*** statsName, double** stats, long &len, long &isLast, long &isNewdata);
private:
	void CreateDataBase(char* pathAndName);
	sqlite3* _database;
	long _insertCount;
	char* _databasePathAndName;
	char** _statsName;
	double* _stats;
	long _numStats;
	long _readCount;	
};
