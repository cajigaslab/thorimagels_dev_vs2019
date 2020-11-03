// ROIDataStoreTestApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include "stdlib.h"
#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <memory>
#include <vector>
#include <sstream>
#include <omp.h>
#include "..\ROIDataStore\ROIDataStore.h"
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	long nROIs = 10;
	long nChannels = 4;
	long nROIStats = 4;
	long storageType = 2;
	long inserts = 2000;
	cout << " Enter number of ROIs: ";
	cin >> nROIs;
	cout << "\n Enter number of Channels: ";
	cin >> nChannels;
	cout << "\n Enter the number of inserts: ";
	cin >> inserts;

	cout << "\n Enter Type of storage (0=PassThrough, 1=SQLite, 2= HDF5): ";
	cin >> storageType;
	cout << "\n";

	auto_ptr<ROIDataStoreDll> roiDataStoreDll(new ROIDataStoreDll(L"ROIDataStore.dll"));
	long nStats = 0;
	string* statsNames = new string[nROIs * nChannels * nROIStats];
	char** columnNames = new char*[nROIs * nChannels * nROIStats];
	double* stats = new double[nROIs * nChannels * nROIStats];
	//columnNames[0] = "Frame";
	char* pathAndName = "D:\\Temp\\ROIData2";
    roiDataStoreDll->CreateROIDataStore(storageType, pathAndName);
	
	for (long i = 0; i < nChannels; i++)
	{
		for(long j = 0; j < nROIs; j++)
		{
			for(long k = 0; k < nROIStats; k++)
			{
				statsNames[nStats] ="Chan";
				statsNames[nStats].append(std::to_string(i));
				statsNames[nStats].append("ROI");
				statsNames[nStats].append(std::to_string(j));
				statsNames[nStats].append("Stat");
				statsNames[nStats].append(std::to_string(k));
				columnNames[nStats] = new char[statsNames[nStats].length() + 1];
				strcpy_s(columnNames[nStats],statsNames[nStats].length() + 1 , statsNames[nStats].c_str());
				stats[nStats] = nStats;
				nStats++;
			}
		}
	}
	   // roiDataStoreDll->GetInstance()->LoadROIData(columnNames,stats,nStats);
	double dtime = omp_get_wtime();
	long bIsLast = FALSE;

	for (int l = 0; l < inserts; l++)
	{
		if(l == (inserts - 1)) bIsLast = 1;
		roiDataStoreDll->LoadROIData(columnNames,stats,nStats, bIsLast);
	}
	char** columnNames2 = new char*[nROIs * nChannels * nROIStats];
	double* statsArray= new double[1];
	long* len = new long[2];
	dtime = omp_get_wtime() - dtime;

	fprintf(stdout, std::to_string(dtime).c_str());
	cout << "\n";
	system("pause");
	dtime = omp_get_wtime();
	//roiDataStoreDll->ReadROIData(columnNames2, statsArray, len);
	//dtime = omp_get_wtime() - dtime;
	//fprintf(stdout, std::to_string(dtime).c_str());
	cout << "\n";

	system("pause");
	return 0;
}

