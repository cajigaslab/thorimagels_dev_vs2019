#include "stdafx.h"
#include "FreqCompoundData.h"

FreqCompoundData::FreqCompoundData(size_t freqSize, size_t fisize, size_t vfsize, size_t freqFitSize)
{
	strucData = new FreqCompDataStruct();
	strucData->freqLength = freqSize;
	strucData->specDataLength = fisize;
	strucData->vspecDataLength = vfsize;
	strucData->freqFitLength = freqFitSize;
	allocBufferMem();
	_createTime = 0;
}

FreqCompoundData::~FreqCompoundData()
{
	FreqCompoundData::dellocBufferMem();	
}

long FreqCompoundData::allocBufferMem()
{
	strucData->freqData = (strucData->freqLength > 0) ? new double[strucData->freqLength]() : NULL;
	strucData->specDataRe = (strucData->specDataLength > 0) ? new double[strucData->specDataLength]() : NULL;
	strucData->specDataIm = (strucData->specDataLength > 0) ? new double[strucData->specDataLength]() : NULL;
	strucData->vSpecData = (strucData->vspecDataLength > 0) ? new double[strucData->vspecDataLength]() : NULL;
	strucData->freqFitData =(strucData->freqFitLength > 0) ? new double[strucData->freqFitLength]() : NULL;

	return TRUE;
}

long FreqCompoundData::SetFreqFitData(size_t freqFitsize)
{
	SAFE_DELETE_ARRAY(strucData->freqFitData);

	strucData->freqFitLength = freqFitsize;
	strucData->freqFitData = (strucData->freqFitLength > 0) ? new double[strucData->freqFitLength]() : NULL;
	return TRUE;
}

long FreqCompoundData::SetVirtualFreqData(size_t vfsize)
{
	SAFE_DELETE_ARRAY(strucData->vSpecData);

	strucData->vspecDataLength = vfsize;
	strucData->vSpecData = (strucData->vspecDataLength > 0) ? new double[strucData->vspecDataLength]() : NULL;
	return TRUE;
}

long FreqCompoundData::CopyFreqCompoundData(FreqCompoundData* cd)
{
	long ret = TRUE;
	_createTime = cd->_createTime;
	if((strucData->freqLength == cd->strucData->freqLength) && (strucData->specDataLength == cd->strucData->specDataLength) 
		&& (strucData->vspecDataLength == cd->strucData->vspecDataLength) && (strucData->freqFitLength == cd->strucData->freqFitLength))
	{
		SAFE_MEMCPY(strucData->freqData, cd->strucData->freqLength*sizeof(double), cd->strucData->freqData);
		SAFE_MEMCPY(strucData->specDataRe, cd->strucData->specDataLength*sizeof(double), cd->strucData->specDataRe);	
		SAFE_MEMCPY(strucData->specDataIm, cd->strucData->specDataLength*sizeof(double), cd->strucData->specDataIm);	
		SAFE_MEMCPY(strucData->vSpecData, cd->strucData->vspecDataLength*sizeof(double), cd->strucData->vSpecData);
		SAFE_MEMCPY(strucData->freqFitData, cd->strucData->freqFitLength*sizeof(double), cd->strucData->freqFitData);
	}
	else
	{ 
		ret = FALSE;  
	}

	return ret;
}

void FreqCompoundData::dellocBufferMem()
{
	SAFE_DELETE_ARRAY(strucData->freqData);
	SAFE_DELETE_ARRAY(strucData->specDataRe);
	SAFE_DELETE_ARRAY(strucData->specDataIm);
	SAFE_DELETE_ARRAY(strucData->vSpecData);
	SAFE_DELETE_ARRAY(strucData->freqFitData);
	SAFE_DELETE_ARRAY(strucData);
}

time_t FreqCompoundData::GetCreateTime()
{
	return _createTime;
}

void FreqCompoundData::SetCreateTime()
{
	time(&_createTime);
}

size_t FreqCompoundData::GetfreqSizeValue()
{ 
	return strucData->freqLength;
}

size_t FreqCompoundData::GetfreqFitSizeValue()
{ 
	return strucData->freqFitLength;
}

FreqCompDataStruct* FreqCompoundData::GetStrucData()
{
	return strucData;
}
