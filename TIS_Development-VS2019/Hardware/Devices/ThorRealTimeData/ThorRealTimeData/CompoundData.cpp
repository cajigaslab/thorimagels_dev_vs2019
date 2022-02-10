#include "stdafx.h"
#include "CompoundData.h"

///create an object of combined global counter, analog-, digital-, counter-, virtual-input
///with individual sizes: 
CompoundData::CompoundData(size_t gcsize, size_t aisize, size_t disize, size_t cisize, size_t visize)
{
	strucData = new CompoundDataStruct();
	_localGCtrSize = gcsize;
	strucData->aiLength = aisize;
	strucData->diLength = disize;
	strucData->ciLength = cisize;
	strucData->viLength = visize;
	strucData->gcLength = 0;
	CompoundData::allocBufferMem();
	_saving = FALSE;
	_createTime = 0;
}

///create an object with interleaved data sets:
CompoundData::CompoundData(CompoundData* cd,int interleave)
{
	int idx=0, i=0, j=0;
	size_t newGCtrSize = 0;	
	_saving = cd->_saving;
	_createTime = cd->_createTime;
	strucData = new CompoundDataStruct();
	strucData->gcLength = 0;
	int nAI = static_cast<int>(cd->strucData->aiLength/cd->_localGCtrSize);
	int nDI = static_cast<int>(cd->strucData->diLength/cd->_localGCtrSize);
	int nCI = static_cast<int>(cd->strucData->ciLength/cd->_localGCtrSize);
	int nVI = static_cast<int>(cd->strucData->viLength/cd->_localGCtrSize);
	int nGC = static_cast<int>(cd->strucData->gcLength/cd->_localGCtrSize);

	//determine how long new data sets should be:
	newGCtrSize=(interleave > 0) ? static_cast<size_t>(ceil(double(cd->_localGCtrSize)/interleave)) : cd->_localGCtrSize;

	gCtr = ((_localGCtrSize = newGCtrSize) > 0) ? new unsigned long[_localGCtrSize] : NULL;
	strucData->aiDataPtr = ((strucData->aiLength = newGCtrSize * nAI) > 0) ? new double[strucData->aiLength] : NULL;
	strucData->diDataPtr = ((strucData->diLength = newGCtrSize * nDI) > 0) ? new unsigned char[strucData->diLength] : NULL;
	strucData->ciDataPtr = ((strucData->ciLength = newGCtrSize * nCI) > 0) ? new unsigned long[strucData->ciLength] : NULL;
	strucData->viDataPtr = ((strucData->viLength = newGCtrSize * nVI) > 0) ? new double[strucData->viLength] : NULL;
	strucData->gCtr64Ptr = ((strucData->gcLength = newGCtrSize * nGC) > 0) ? new unsigned __int64[strucData->gcLength] : NULL;

	if(interleave > 0)
	{
		//interleave for all channels:
		idx = 0;
		for(i=0;i<cd->_localGCtrSize;i+=interleave)
		{
			if(idx<newGCtrSize)
			{
				//32x global counter:
				gCtr[idx] = cd->gCtr[i];
				//ai channels:
				for(j=0;j<nAI;j++)
				{
					strucData->aiDataPtr[idx+j*newGCtrSize] = cd->strucData->aiDataPtr[i+j*cd->_localGCtrSize];
				}				
				//di channels:
				for(j=0;j<nDI;j++)
				{
					strucData->diDataPtr[idx+j*newGCtrSize] = cd->strucData->diDataPtr[i+j*cd->_localGCtrSize];
				}
				//ci channels:
				for(j=0;j<nCI;j++)
				{
					strucData->ciDataPtr[idx+j*newGCtrSize] = cd->strucData->ciDataPtr[i+j*cd->_localGCtrSize];
				}
				//vi channels:
				for(j=0;j<nVI;j++)
				{
					strucData->viDataPtr[idx+j*newGCtrSize] = cd->strucData->viDataPtr[i+j*cd->_localGCtrSize];
				}
				//64x global counter:
				if((strucData->gcLength>0) && (strucData->gCtr64Ptr != NULL))
				{
					strucData->gCtr64Ptr[idx] = cd->strucData->gCtr64Ptr[i];
				}
			}
			idx++;
		}

	}
	else
	{
		//make a identical copy:
		if(_localGCtrSize > 0)
		{
			memcpy(gCtr,cd->gCtr,cd->_localGCtrSize*sizeof(unsigned long));
		}
		if(strucData->aiLength > 0)
		{			
			for(j=0;j<nAI;j++)
			{
				memcpy(strucData->aiDataPtr+j*_localGCtrSize,cd->strucData->aiDataPtr+j*(cd->strucData->aiLength/nAI),_localGCtrSize*sizeof(double));
			}
		}
		if(strucData->diLength > 0)
		{
			for(j=0;j<nDI;j++)
			{
				memcpy(strucData->diDataPtr+j*_localGCtrSize,cd->strucData->diDataPtr+j*(cd->strucData->diLength/nDI),_localGCtrSize*sizeof(unsigned char));
			}
		}
		if(strucData->ciLength > 0)
		{
			for(j=0;j<nCI;j++)
			{	
				memcpy(strucData->ciDataPtr+j*_localGCtrSize,cd->strucData->ciDataPtr+j*(cd->strucData->ciLength/nCI),_localGCtrSize*sizeof(unsigned long));
			}
		}
		if(strucData->viLength > 0)
		{
			for(j=0;j<nVI;j++)
			{	
				memcpy(strucData->viDataPtr+j*_localGCtrSize,cd->strucData->viDataPtr+j*(cd->strucData->viLength/nVI),_localGCtrSize*sizeof(double));
			}
		}
		if((strucData->gcLength > 0) && (cd->strucData->gCtr64Ptr != NULL))
		{			 
			memcpy(strucData->gCtr64Ptr,cd->strucData->gCtr64Ptr,strucData->gcLength*sizeof(unsigned __int64));
		}
	}
}

///create an object with selected data sets
///based on first DI channel:
CompoundData::CompoundData(CompoundData* cd,StimulusSaveStruct* ssaveMode)
{
	int idx, i=0, j=0;
	size_t newGCtrSize = 0;	
	strucData = new CompoundDataStruct();
	_saving = cd->_saving;
	_createTime = cd->_createTime;
	int nAI = static_cast<int>(cd->strucData->aiLength/cd->_localGCtrSize);
	int nDI = static_cast<int>(cd->strucData->diLength/cd->_localGCtrSize);
	int nCI = static_cast<int>(cd->strucData->ciLength/cd->_localGCtrSize);
	int nVI = static_cast<int>(cd->strucData->viLength/cd->_localGCtrSize);
	int nGC = static_cast<int>(cd->strucData->gcLength/cd->_localGCtrSize);

	if(ssaveMode->enable)
	{
		//find out how long new data sets should be:
		switch(ssaveMode->signalType)
		{
		case 0:		//AI
			for(i=0;i<cd->_localGCtrSize;i++)
			{
				if(cd->strucData->aiDataPtr[i+(ssaveMode->stimChannelID*cd->_localGCtrSize)] > ssaveMode->threshold)
				{
					newGCtrSize++;
				}
			}
			gCtr = ((_localGCtrSize = newGCtrSize) > 0) ? new unsigned long[_localGCtrSize] : NULL;
			strucData->ciDataPtr = ((strucData->ciLength = newGCtrSize * nCI) > 0) ? new unsigned long[strucData->ciLength] : NULL;
			strucData->gCtr64Ptr = ((strucData->gcLength = newGCtrSize * nGC) > 0) ? new unsigned __int64[strucData->gcLength] : NULL;
			strucData->aiDataPtr = ((strucData->aiLength = newGCtrSize * nAI) > 0) ? new double[strucData->aiLength] : NULL;
			strucData->diDataPtr = ((strucData->diLength = newGCtrSize * nDI) > 0) ? new unsigned char[strucData->diLength] : NULL;
			strucData->viDataPtr = ((strucData->viLength = newGCtrSize * nVI) > 0) ? new double[strucData->viLength] : NULL;

			//copy based on the stimulus channel:
			idx = 0;
			for(i=0;i<cd->_localGCtrSize;i++)
			{
				if(cd->strucData->aiDataPtr[i+(ssaveMode->stimChannelID*cd->_localGCtrSize)] > ssaveMode->threshold)
				{
					if(idx<newGCtrSize)
					{
						//32x global counter:
						gCtr[idx] = cd->gCtr[i];
						//ai channels:
						for(j=0;j<nAI;j++)
						{
							strucData->aiDataPtr[idx+j*newGCtrSize] = cd->strucData->aiDataPtr[i+j*cd->_localGCtrSize];
						}				
						//di channels:
						for(j=0;j<nDI;j++)
						{
							strucData->diDataPtr[idx+j*newGCtrSize] = cd->strucData->diDataPtr[i+j*cd->_localGCtrSize];
						}
						//ci channels:
						for(j=0;j<nCI;j++)
						{
							strucData->ciDataPtr[idx+j*newGCtrSize] = cd->strucData->ciDataPtr[i+j*cd->_localGCtrSize];
						}
						//vi channels:
						for(j=0;j<nVI;j++)
						{
							strucData->viDataPtr[idx+j*newGCtrSize] = cd->strucData->viDataPtr[i+j*cd->_localGCtrSize];
						}
						//64x global counter:
						if((strucData->gcLength>0) && (strucData->gCtr64Ptr != NULL))
						{
							strucData->gCtr64Ptr[idx] = cd->strucData->gCtr64Ptr[i];
						}
					}
					idx++;
				}
			}
			break;
		case 1:		//DI
			for(i=0;i<cd->_localGCtrSize;i++)
			{
				if(cd->strucData->diDataPtr[i+(ssaveMode->stimChannelID*cd->_localGCtrSize)] > 0)
				{
					newGCtrSize++;
				}
			}
			gCtr = ((_localGCtrSize = newGCtrSize) > 0) ? new unsigned long[_localGCtrSize] : NULL;
			strucData->ciDataPtr = ((strucData->ciLength = newGCtrSize * nCI) > 0) ? new unsigned long[strucData->ciLength] : NULL;
			strucData->gCtr64Ptr = ((strucData->gcLength = newGCtrSize * nGC) > 0) ? new unsigned __int64[strucData->gcLength] : NULL;
			strucData->diDataPtr = ((strucData->diLength = newGCtrSize * nDI) > 0) ? new unsigned char[strucData->diLength] : NULL;
			strucData->aiDataPtr = ((strucData->aiLength = newGCtrSize * nAI) > 0) ? new double[strucData->aiLength] : NULL;
			strucData->viDataPtr = ((strucData->viLength = newGCtrSize * nVI) > 0) ? new double[strucData->viLength] : NULL;

			//copy based on the stimulus channel:
			idx = 0;
			for(i=0;i<cd->_localGCtrSize;i++)
			{
				if(cd->strucData->diDataPtr[i+(ssaveMode->stimChannelID*cd->_localGCtrSize)] > 0)
				{
					if(idx<newGCtrSize)
					{
						//32x global counter:
						gCtr[idx] = cd->gCtr[i];
						//ai channels:
						for(j=0;j<nAI;j++)
						{
							strucData->aiDataPtr[idx+j*newGCtrSize] = cd->strucData->aiDataPtr[i+j*cd->_localGCtrSize];
						}				
						//di channels:
						for(j=0;j<nDI;j++)
						{
							strucData->diDataPtr[idx+j*newGCtrSize] = cd->strucData->diDataPtr[i+j*cd->_localGCtrSize];
						}
						//ci channels:
						for(j=0;j<nCI;j++)
						{
							strucData->ciDataPtr[idx+j*newGCtrSize] = cd->strucData->ciDataPtr[i+j*cd->_localGCtrSize];
						}
						//vi channels:
						for(j=0;j<nVI;j++)
						{
							strucData->viDataPtr[idx+j*newGCtrSize] = cd->strucData->viDataPtr[i+j*cd->_localGCtrSize];
						}
						//64x global counter:
						if((strucData->gcLength>0) && (strucData->gCtr64Ptr != NULL))
						{
							strucData->gCtr64Ptr[idx] = cd->strucData->gCtr64Ptr[i];
						}
					}
					idx++;
				}
			}
			break;
		}
	}
	else
	{
		//GCtrSize should be primary length:
		gCtr = ((_localGCtrSize = cd->_localGCtrSize) > 0) ?  new unsigned long[_localGCtrSize] : NULL;
		strucData->aiDataPtr = ((strucData->aiLength = _localGCtrSize * nAI) > 0) ? new double[strucData->aiLength] : NULL;
		strucData->diDataPtr = ((strucData->diLength = _localGCtrSize * nDI) > 0) ? new unsigned char[strucData->diLength] : NULL;
		strucData->ciDataPtr = ((strucData->ciLength = _localGCtrSize * nCI) > 0) ? new unsigned long[strucData->ciLength] : NULL;
		strucData->viDataPtr = ((strucData->viLength = _localGCtrSize * nVI) > 0) ? new double[strucData->viLength] : NULL;
		strucData->gCtr64Ptr = ((strucData->gcLength = cd->strucData->gcLength) > 0) ? new unsigned __int64[strucData->gcLength] : NULL;

		if(_localGCtrSize > 0)
		{
			memcpy(gCtr,cd->gCtr,cd->_localGCtrSize*sizeof(unsigned long));
		}
		if(strucData->aiLength > 0)
		{			
			for(j=0;j<nAI;j++)
			{
				memcpy(strucData->aiDataPtr+j*_localGCtrSize,cd->strucData->aiDataPtr+j*(cd->strucData->aiLength/nAI),_localGCtrSize*sizeof(double));
			}
		}
		if(strucData->diLength > 0)
		{
			for(j=0;j<nDI;j++)
			{
				memcpy(strucData->diDataPtr+j*_localGCtrSize,cd->strucData->diDataPtr+j*(cd->strucData->diLength/nDI),_localGCtrSize*sizeof(unsigned char));
			}
		}
		if(strucData->ciLength > 0)
		{
			for(j=0;j<nCI;j++)
			{	
				memcpy(strucData->ciDataPtr+j*_localGCtrSize,cd->strucData->ciDataPtr+j*(cd->strucData->ciLength/nCI),_localGCtrSize*sizeof(unsigned long));
			}
		}
		if(strucData->viLength > 0)
		{
			for(j=0;j<nVI;j++)
			{	
				memcpy(strucData->viDataPtr+j*_localGCtrSize,cd->strucData->viDataPtr+j*(cd->strucData->viLength/nVI),_localGCtrSize*sizeof(double));
			}
		}
		if((strucData->gcLength > 0) && (cd->strucData->gCtr64Ptr != NULL))
		{			

			memcpy(strucData->gCtr64Ptr,cd->strucData->gCtr64Ptr,strucData->gcLength*sizeof(unsigned __int64));
		}
	}
}

CompoundData::~CompoundData()
{
	CompoundData::dellocBufferMem();	
}

///allocate memory based on length:
long CompoundData::allocBufferMem()
{
	gCtr = (_localGCtrSize > 0) ? new unsigned long[_localGCtrSize] : NULL;
	strucData->aiDataPtr =(strucData->aiLength > 0) ? new double[strucData->aiLength] : NULL;
	strucData->diDataPtr = (strucData->diLength > 0) ? new unsigned char[strucData->diLength] : NULL;
	strucData->ciDataPtr = (strucData->ciLength > 0) ? new unsigned long[strucData->ciLength] : NULL;
	strucData->viDataPtr =(strucData->viLength > 0) ? new double[strucData->viLength] : NULL;
	strucData->gCtr64Ptr = (strucData->gcLength > 0) ? new unsigned __int64[strucData->gcLength] : NULL; 

	return TRUE;
}

///copy data from input CompoundData, 
///data length needs to be the same on both side:
long CompoundData::CopyCompoundData(CompoundData* cd)
{
	long ret = TRUE;
	_saving = cd->_saving;
	_createTime = cd->_createTime;
	if((strucData->aiLength == cd->strucData->aiLength) && (strucData->diLength == cd->strucData->diLength) && (strucData->ciLength == cd->strucData->ciLength) && (strucData->viLength == cd->strucData->viLength))
	{
		memcpy(strucData->aiDataPtr,cd->strucData->aiDataPtr,cd->strucData->aiLength*sizeof(double));
		memcpy(strucData->diDataPtr,cd->strucData->diDataPtr,cd->strucData->diLength*sizeof(unsigned char));	
		memcpy(strucData->ciDataPtr,cd->strucData->ciDataPtr,cd->strucData->ciLength*sizeof(unsigned long));
		memcpy(strucData->viDataPtr,cd->strucData->viDataPtr,cd->strucData->viLength*sizeof(double));
	}
	else
	{ 
		ret = FALSE;  
	}

	if((cd->_localGCtrSize > 0) && (_localGCtrSize == cd->_localGCtrSize))
	{
		memcpy(gCtr,cd->gCtr,cd->_localGCtrSize*sizeof(unsigned long));

		if(cd->strucData->gCtr64Ptr != NULL)
		{
			if(strucData->gCtr64Ptr == NULL)
			{
				strucData->gcLength = cd->strucData->gcLength;
				strucData->gCtr64Ptr = new unsigned __int64[strucData->gcLength];
			}
			memcpy(strucData->gCtr64Ptr,cd->strucData->gCtr64Ptr,cd->strucData->gcLength*sizeof(unsigned __int64));
		}
	}
	else
	{  
		ret = FALSE;
	}

	return ret;
}

///Global Counter: handle overflow and offset with input value
long CompoundData::SetupGlobalCounter(unsigned long &overflowGCtrCnt)
{
	long ret = FALSE;

	//single global counter:
	if(_localGCtrSize > 0)
	{
		if(strucData->gCtr64Ptr == NULL)
		{
			strucData->gcLength = _localGCtrSize;
			strucData->gCtr64Ptr = new unsigned __int64[strucData->gcLength];
		}
		for(int j=0;j<strucData->gcLength;j++)
		{
			if((j>0) && (gCtr[j]<gCtr[j-1]))
			{
				overflowGCtrCnt++;
				ret = TRUE;
			}
			strucData->gCtr64Ptr[j] = static_cast<unsigned __int64>(overflowGCtrCnt)<<32;
			strucData->gCtr64Ptr[j] += static_cast<unsigned __int64>(gCtr[j]);
			strucData->gCtr64Ptr[j] -= strucData->gCtr64Ptr[0];
		}
	}

	return ret;
}
long CompoundData::SetupGlobalCounter(unsigned __int64 initGCtr, unsigned __int64 offsetCnt, unsigned long &overflowGCtrCnt)
{
	long ret = FALSE;

	//single global counter:
	if(_localGCtrSize > 0)
	{
		if(strucData->gCtr64Ptr == NULL)
		{
			strucData->gcLength = _localGCtrSize;
			strucData->gCtr64Ptr = new unsigned __int64[strucData->gcLength];
		}
		for(int j=0;j<strucData->gcLength;j++)
		{
			if((j>0) && (gCtr[j]<gCtr[j-1]))
			{
				overflowGCtrCnt++;
				ret = TRUE;
			}
			strucData->gCtr64Ptr[j] = static_cast<unsigned __int64>(overflowGCtrCnt)<<32;
			strucData->gCtr64Ptr[j] += static_cast<unsigned __int64>(gCtr[j]);
			strucData->gCtr64Ptr[j] -= initGCtr;
			strucData->gCtr64Ptr[j] += offsetCnt;			
		}
	}

	return ret;
}

///release memory for destructor:
long CompoundData::dellocBufferMem()
{
	if(strucData->diDataPtr)
	{
		delete[] strucData->diDataPtr;
		strucData->diDataPtr = NULL;
	}
	if(strucData->aiDataPtr)
	{
		delete[] strucData->aiDataPtr;
		strucData->aiDataPtr = NULL;
	}
	if(strucData->ciDataPtr)
	{
		delete[] strucData->ciDataPtr;
		strucData->ciDataPtr = NULL;
	}
	if(strucData->viDataPtr)
	{
		delete[] strucData->viDataPtr;
		strucData->viDataPtr = NULL;
	}
	if(gCtr)
	{
		delete[] gCtr;
		gCtr = NULL;
	}
	if(strucData->gCtr64Ptr)
	{
		delete[] strucData->gCtr64Ptr;
		strucData->gCtr64Ptr = NULL;
	}
	if(strucData)
	{
		delete strucData;
		strucData = NULL;
	}
	return TRUE;
}

///get member functions:
size_t CompoundData::GetaiLengthValue()
{
	return strucData->aiLength;
}

size_t CompoundData::GetdiLengthValue()
{
	return strucData->diLength;
}

size_t CompoundData::GetciLengthValue()
{
	return strucData->ciLength;
}

size_t CompoundData::GetviLengthValue()
{
	return strucData->viLength;
}

time_t CompoundData::GetCreateTime()
{
	return _createTime;
}

size_t CompoundData::GetgcLengthValue()
{
	return _localGCtrSize;
}

size_t CompoundData::GetgcLengthComValue()
{
	return strucData->gcLength;
}

unsigned long* CompoundData::GetgCtr()
{
	return gCtr;
}

long CompoundData::GetSaving()
{
	return _saving;
}

CompDataStruct* CompoundData::GetStrucData()
{
	return strucData;
}

///For display buffer only, get the pointer of data structure:
long CompoundData::GetStrucData(CompDataStruct* ptr)
{
	long ret = TRUE;

	if(strucData)
	{
		if(strucData->gcLength == 0)
		{
			ret = FALSE;
		}
		*ptr = *strucData;		
	}

	return ret;
}

long CompoundData::SetCreateTime(time_t ctime)
{
	_createTime = ctime;
	return TRUE;
}

long CompoundData::SetSaving(long toSave)
{
	_saving = toSave;
	return TRUE;
}
