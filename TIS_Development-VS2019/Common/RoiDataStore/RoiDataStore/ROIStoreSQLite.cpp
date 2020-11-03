#include "stdafx.h"
#include "ROIStoreSQLite.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <time.h>

using namespace std;
#define BUFFER_SIZE 256
#define MAX_DATE 30
#define MAX_STATNAME_LEN 32

ROIStoreSQLite::ROIStoreSQLite(char* pathAndName)
{
	_insertCount = 0;
	_readCount = 0;
	_numStats = 0;
	_stats = NULL;
	_statsName = NULL;	
	CreateDataBase(pathAndName);
}

ROIStoreSQLite::~ROIStoreSQLite()
{
	long rc = sqlite3_close(_database);
	if (rc)
	{
		printf("sqlite3_bind_text Failed! %s\n", sqlite3_errmsg(_database));
	}
	remove(_databasePathAndName);

	if(NULL != _statsName)
	{
		for(int i=0; i<_numStats; i++)
		{
			CoTaskMemFree(_statsName[i]);
		}

		CoTaskMemFree(_statsName);
		_statsName = NULL;
	}
	delete[] _databasePathAndName;
}

void ROIStoreSQLite::CreateDataBase(char* pathAndName)
{
	_databasePathAndName = new char[_MAX_PATH];
	strcpy_s(_databasePathAndName, _MAX_PATH, pathAndName);
	char timeStamp[MAX_DATE];
	struct tm tim;
	time_t now;
	now = time(NULL);
	
	//get the time in 64bit format
	_time64(&now);
	
	//translate the time to local time
	localtime_s(&tim, &now);

	//format it and put it in a string
	strftime(timeStamp,30,"%b%d%Y%H%M%S",&tim);

	//add the string to the database file name
	strcat_s(_databasePathAndName, _MAX_PATH, timeStamp);

	//add the termination to the file
	char* fileTermination = ".db";
	strcat_s(_databasePathAndName, _MAX_PATH, fileTermination);

	//open/Create the database
	long rc = sqlite3_open(_databasePathAndName, &_database);

	/* Drop ROIStats TABLE SQL statement */
	char* sql = "DROP TABLE ROIStats";

	char *error;
	/* Execute SQL statement */	
	rc = sqlite3_exec(_database, sql, NULL, 0, &error);	

	/* Create ROIStats Table SQL statement */
	sql = "CREATE TABLE ROIStats (TimeIndex INTEGER, StatName TEXT, Stat REAL, NumStats INTEGER);";
	
	/* Execute SQL statement */
	rc = sqlite3_exec(_database, sql, NULL, NULL, &error);
	if (rc)
	{
		printf("Create Index Failed! %s\n", error);
		sqlite3_free(error);
	}

	/* Execute SQL statement */
	rc = sqlite3_exec(_database, sql, NULL, NULL, &error);
	if (rc)
	{
		printf("Create Index Failed! %s\n", error);
		sqlite3_free(error);
	}

	//Options to speed up the inserts to the database
	rc = sqlite3_exec(_database, "PRAGMA synchronous = OFF", NULL, NULL, &error);
    if (rc)
	{
		printf("Create Index Failed! %s\n", error);
		sqlite3_free(error);
	}
	rc = sqlite3_exec(_database, "PRAGMA journal_mode = MEMORY", NULL, NULL, &error);
	if (rc)
	{
		printf("Create Index Failed! %s\n", error);
		sqlite3_free(error);
	}

	/* Create Index Statement SQL statement */
	char* sSql = "CREATE  INDEX 'Table_Index' ON 'ROIStats' ('TimeIndex', 'StatName', 'Stat', 'NumStats')";
	
	/* Execute SQL statement */
	rc = sqlite3_exec(_database, sSql, NULL, NULL, &error);
	if (rc)
	{
		printf("Create Index Failed! %s\n", error);
		sqlite3_free(error);
	}
}


void ROIStoreSQLite::SaveROIData(char** statsName, double* stats, long nStats, long isLast)
{
	
	char *error;
	long rc = sqlite3_exec(_database, "BEGIN TRANSACTION", NULL, NULL, &error);
	char buffer[BUFFER_SIZE] = "INSERT INTO ROIStats VALUES (@TI, @SN, @ST, @NS)\0";
	sqlite3_stmt* stmt;
	rc = sqlite3_prepare_v2(_database, buffer, BUFFER_SIZE, &stmt, NULL);
	if (rc)
	{
		printf("sqlite3_prepare_v2 Failed! %s\n", sqlite3_errmsg(_database));
	}
	//if there are stats
	if(0 < nStats)
	{
		for (int i=0; i < nStats; i++)
		{
			rc = sqlite3_bind_int(stmt, 1, _insertCount);
			if (rc)
			{
				printf("sqlite3_bind_int Failed! %s\n", sqlite3_errmsg(_database));
			}
			rc = sqlite3_bind_text(stmt, 2,statsName[i], -1, SQLITE_STATIC);
		
			if (rc)
			{
				printf("sqlite3_bind_text Failed! %s\n", sqlite3_errmsg(_database));
			}
		
			rc = sqlite3_bind_double(stmt, 3, stats[i]);
			if (rc)
			{
				printf("sqlite3_bind_double Failed! %s\n", sqlite3_errmsg(_database));
			}

			rc = sqlite3_bind_int(stmt, 4, nStats);
			if (rc)
			{
				printf("sqlite3_bind_double Failed! %s\n", sqlite3_errmsg(_database));
			}
		
			int retVal = sqlite3_step(stmt);
			if (retVal != SQLITE_DONE)
			{
				printf("sqlite3_step Failed! %s\n", sqlite3_errmsg(_database));
			}
 
			sqlite3_reset(stmt);
		}
	}
	else
	{
		// if there are no stats fill one row to keep track
		rc = sqlite3_bind_int(stmt, 1, _insertCount);
		if (rc)
		{
			printf("sqlite3_bind_int Failed! %s\n", sqlite3_errmsg(_database));
		}
		char noData[MAX_STATNAME_LEN] = "noData";
		rc = sqlite3_bind_text(stmt, 2, noData, -1, SQLITE_STATIC);
		if (rc)
		{
			printf("sqlite3_bind_text Failed! %s\n", sqlite3_errmsg(_database));
		}
		
		rc = sqlite3_bind_double(stmt, 3, 0);
		if (rc)
		{
			printf("sqlite3_bind_double Failed! %s\n", sqlite3_errmsg(_database));
		}

		rc = sqlite3_bind_int(stmt, 4, 0);
		if (rc)
		{
			printf("sqlite3_bind_double Failed! %s\n", sqlite3_errmsg(_database));
		}
		
		int retVal = sqlite3_step(stmt);
		if (retVal != SQLITE_DONE)
		{
			printf("sqlite3_step Failed! %s\n", sqlite3_errmsg(_database));
		}
 
		sqlite3_reset(stmt);
	}

	rc = sqlite3_exec(_database, "COMMIT TRANSACTION", NULL, NULL, &error);
	if (rc)
	{
		printf("Commit Failed! %s\n", error);
		sqlite3_free(error);
	}
	sqlite3_finalize(stmt);

	_insertCount++;
}

void ROIStoreSQLite::ReadROIData(char*** statsName, double** stats, long &len, long &isLast, long &isNewdata)
{
	//Only continue if there is new data
	if (_readCount == _insertCount)
	{
		isNewdata = FALSE;
		isLast = TRUE;
		return;
	}
		
	isNewdata = TRUE;

	int k = 0;	
	char  buffer[BUFFER_SIZE];
	long rc = 0;
	long retVal = 0;
	sqlite3_stmt* stmt;

	/////***** Retrieve number of Stats in this time point *****/////
	k  = sprintf_s( buffer, BUFFER_SIZE,     "SELECT NumStats FROM ROIStats WHERE TimeIndex =%d", _readCount);
	rc = sqlite3_prepare_v2(_database, buffer, BUFFER_SIZE, &stmt, NULL);
	if (rc)
	{
		const char * y =  sqlite3_errmsg(_database);
		printf("Reading Failed! %d\n",10);
	}
	int j = 0;

	while(1)
	{
		// fetch a row's status
		retVal = sqlite3_step(stmt);

		if(SQLITE_ROW == retVal)
		{
			len  = (long)sqlite3_column_int(stmt, 0);

			//Return if there is no data for this time point
			if (1 > len)
			{
				_readCount++;
				sqlite3_finalize(stmt);
				return;
			}

			//delete existing
			if((NULL != _statsName || NULL !=_stats)&&(_numStats != len) )
			{		
			
				for(int i=0; i<_numStats; i++)
				{
					CoTaskMemFree(_statsName[i]);
				}
				delete[] _stats;
				CoTaskMemFree(_statsName);

				_stats = NULL;
				_statsName = NULL;
			}

			//keep track of the number of stats to initialize the arrat of char strings
			_numStats = len;
	
			//allocate if needed
			if(NULL == _statsName || NULL == _stats)
			{
				_statsName = (char**)CoTaskMemAlloc(sizeof(char *) * _numStats);
				_stats = new double[_numStats];

				memset(_statsName,0,sizeof(char *) * _numStats);
		
				for(int i=0; i<_numStats; i++)
				{
					_statsName[i] =  (char*)CoTaskMemAlloc(MAX_STATNAME_LEN);
				}
			}
			break;			
		}
		else if(retVal == SQLITE_DONE)
		{
			break;
		}
		else
		{
			sqlite3_finalize(stmt);
			printf("Some error encountered\n");
			break;
		}			
	}	
	sqlite3_finalize(stmt);

	/////***** Retrieve stats names in this time point *****/////
	k  = sprintf_s( buffer, BUFFER_SIZE,     "SELECT StatName FROM ROIStats WHERE TimeIndex =%d", _readCount);
	rc = sqlite3_prepare_v2(_database, buffer, BUFFER_SIZE, &stmt, NULL);
	if (rc)
	{
		const char * y =  sqlite3_errmsg(_database);
		printf("%s", "Reading Failed! %d\n");
	}
	j = 0;
	while(1)
	{
		// fetch a row's status
		retVal = sqlite3_step(stmt);
		if(SQLITE_ROW == retVal)
		{	
			const char* temp = temp = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
			strcpy_s(_statsName[j],MAX_STATNAME_LEN, temp);
			j++;			
		}
		else if(retVal == SQLITE_DONE)
		{
			break;
		}
		else
		{
			sqlite3_finalize(stmt);
			printf("%s", "Some error encountered\n");
			break;
		}			
	}	
	sqlite3_finalize(stmt);

	/////***** Retrieve stats in this time point *****/////
	k  = sprintf_s( buffer, BUFFER_SIZE,     "SELECT Stat FROM ROIStats WHERE TimeIndex =%d", _readCount);
	rc = sqlite3_prepare_v2(_database, buffer, BUFFER_SIZE, &stmt, NULL);
	if (rc)
	{
		const char * y =  sqlite3_errmsg(_database);
		printf("%s", "Reading Failed! %d\n");
	}

	j = 0;
	while(1)
	{
		// fetch a row's status
		retVal = sqlite3_step(stmt);

		if(SQLITE_ROW == retVal)
		{
			_stats[j] = sqlite3_column_double(stmt, 0);
			j++;		
		}
		else if(retVal == SQLITE_DONE)
		{
			break;
		}
		else
		{
			sqlite3_finalize(stmt);
			printf("Some error encountered\n");
			break;
		}			
	}
	sqlite3_finalize(stmt);
	_readCount ++;

	*stats = _stats;
	*statsName = _statsName;
	
	//Compare the number of read stats time points versus the number of insterted stats time points
	if (_readCount == _insertCount)
	{
		isLast = TRUE;
	}
	else
	{
		isLast = FALSE;
	}
}
