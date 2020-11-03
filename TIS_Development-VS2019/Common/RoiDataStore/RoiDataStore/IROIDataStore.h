
#ifndef IROIDATASTORE_H
#define IROIDATASTORE_H

class IROIDataStore
{
public:
	IROIDataStore(){}
	virtual ~ IROIDataStore(){}
	// Interfaces for load/input data
	
	// Interfaces for output data to Database or files
	virtual void SaveROIData(char** statsName, double* stats, long nStats, long isLast)=0;
	virtual void ReadROIData(char*** nameChain, double** values, long &len, long &isLast, long &isNewData) = 0;

};

#endif

