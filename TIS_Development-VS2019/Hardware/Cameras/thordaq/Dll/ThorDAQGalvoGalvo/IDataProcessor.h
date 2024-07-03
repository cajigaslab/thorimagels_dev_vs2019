#ifndef IDATA_PROCESSING_H
#define IDATA_PROCESSING_H
#pragma once
class IDataProcessor
{
public:
	virtual vector<UCHAR*> ProcessBuffer(UCHAR** pFrmData, ULONG framsePerTransfer) = 0;
	virtual ~IDataProcessor()
	{
		// Compulsory virtual destructor definition
	}

};

#endif