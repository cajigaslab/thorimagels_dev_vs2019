// EPhysWaveformBuilder.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "EPhysWaveformBuilder.h"

///	***************************************** <summary>				EPhysWaveformBuilder Constructor			</summary>	********************************************** ///

EPhysWaveformBuilder::EPhysWaveformBuilder()
{
	ResetCounter();
}

///	***************************************** <summary> EPhysWaveformBuilder static members & global variables	</summary>	********************************************** ///

bool EPhysWaveformBuilder::_instanceFlag = false;
std::unique_ptr<EPhysWaveformBuilder> EPhysWaveformBuilder::_single;
uint64_t EPhysWaveformBuilder::_countTotal[BUF_REGION_COUNT] = {0};
uint64_t EPhysWaveformBuilder::_countIndex = 0;
BlockRingBuffer* EPhysWaveformBuilder::_bRingBuffer = {NULL};
CircularBuffer* EPhysWaveformBuilder::_ephysBuffer[BUF_REGION_COUNT] = {NULL};
HANDLE EPhysWaveformBuilder::_hInitialized = CreateEvent(NULL, true, true, NULL);	//2nd parameter "true" so it needs manual "Reset" after "Set (signal)" event

///	***************************************** <summary>				EPhysWaveformBuilder Functions				</summary>	********************************************** ///

// singleton
EPhysWaveformBuilder* EPhysWaveformBuilder::getInstance()
{
	if(!_instanceFlag)
	{
		try
		{
			_single.reset(new EPhysWaveformBuilder());
			_instanceFlag = true;
			return _single.get();
		}
		catch(...)
		{
			throw;
		}
	}
	return _single.get();
}

void EPhysWaveformBuilder::BufferAvailableCallbackFunc(long sType, long bufSpaceNum)
{
	if((NULL == _bRingBuffer) || (NULL == _ephysBuffer[0]) || (NULL == _ephysBuffer[1]) || (NULL == _ephysBuffer[2]))
		goto SET_EVENT;

	//only one signal type in this builder
	if(_bRingBuffer->GetSignalType() != sType)
		goto SET_EVENT;

	//do not continue if waveform was not built, or already reached the end
	if ((MAXULONG32 > _countTotal[2]) && ((0 == (_countTotal[0] + _countTotal[1] + _countTotal[2])) || ((_countTotal[0] + _countTotal[1] + _countTotal[2]) <= _countIndex)))
		goto SET_EVENT;

	unsigned long totalSizeToFill = bufSpaceNum * _bRingBuffer->GetBlockSize();		//total size to be copied

	//return if nothing to fill
	if (0 >= totalSizeToFill)
		goto SET_EVENT;

	//local variables
	unsigned long currentIdx = 0;													//index within totalSizeToFill
	unsigned long countToCopy = 0;													//count to be copied per if/else section
	unsigned long bufSpaceIdx = 0;													//current index of copied buffer space number
	unsigned long oneRepeatSize = static_cast<unsigned long>(_ephysBuffer[1]->Size()/UnitSizeInBytes((SignalType)_bRingBuffer->GetSignalType()));
	double* dSrc = NULL, *dTgt = NULL;
	UCHAR* cSrc = NULL;

	do
	{
		//determine current index location among the whole waveform,
		//idle section:
		if ((0 < _countTotal[0]) && (_countTotal[0] > _countIndex))		
		{
			countToCopy = std::min(static_cast<unsigned long>(_countTotal[0] - _countIndex), static_cast<unsigned long>(totalSizeToFill - currentIdx));
			_ephysBuffer[0]->Lock();
			cSrc = _ephysBuffer[0]->Copy(countToCopy * sizeof(unsigned char));
			_bRingBuffer->WriteUnits(cSrc, countToCopy);
			SAFE_DELETE_MEMORY(cSrc);
			_ephysBuffer[0]->ReleaseLock();

		}
		//waveform body including all repeats or continuous upto MAXULONGLONG:
		else if ((0 < (_countTotal[0] + _countTotal[1])) && ((_countTotal[0] + _countTotal[1]) > _countIndex))
		{
			//unit size to copy
			unsigned long unitCopySize = std::min(static_cast<long>(oneRepeatSize), _bRingBuffer->GetBlockSize());

			//offset from beginning of one repeat, only applies if countPerCallback is smaller than one repeat
			unsigned long offset = std::max(static_cast<long>(0), static_cast<long>((_countIndex - _countTotal[0]) % oneRepeatSize));

			//remain to copy in one repeat
			unsigned long remain = static_cast<unsigned long>(oneRepeatSize - offset);

			//remain to copy in callback
			unsigned long remainInCallback = static_cast<unsigned long>(_bRingBuffer->GetBlockSize() - (currentIdx % _bRingBuffer->GetBlockSize()));

			countToCopy = std::min(static_cast<unsigned long>(_countTotal[1]), std::max(static_cast<unsigned long>(0), std::min(std::min(std::min(remain, unitCopySize), remainInCallback), static_cast<unsigned long>(totalSizeToFill - currentIdx))));
			_ephysBuffer[1]->Lock();
			cSrc = _ephysBuffer[1]->Copy(countToCopy * sizeof(unsigned char));
			_bRingBuffer->WriteUnits(cSrc, countToCopy);
			SAFE_DELETE_MEMORY(cSrc);
			_ephysBuffer[1]->ReleaseLock();

		}
		//zero padding at the end
		else if ((0 < (_countTotal[0] + _countTotal[1] + _countTotal[2])) && ((_countTotal[0] + _countTotal[1] + _countTotal[2]) > _countIndex))
		{
			//remain to copy
			unsigned long remain = static_cast<unsigned long>(_countTotal[0] + _countTotal[1] + _countTotal[2] - _countIndex);

			countToCopy = std::min(static_cast<unsigned long>(_countTotal[2]), std::max(static_cast<unsigned long>(0), std::min(static_cast<unsigned long>(_bRingBuffer->GetBlockSize()), std::min(remain, static_cast<unsigned long>(totalSizeToFill - currentIdx)))));
			_ephysBuffer[2]->Lock();
			cSrc = _ephysBuffer[2]->Copy(countToCopy * sizeof(unsigned char));
			_bRingBuffer->WriteUnits(cSrc, countToCopy);
			SAFE_DELETE_MEMORY(cSrc);
			_ephysBuffer[2]->ReleaseLock();
		}

		_countIndex += countToCopy;
		currentIdx += countToCopy;

		//avoid overflow in continuous mode, and only repeat waveform body
		if((MAXULONG32 == _countTotal[1]) && ((_countTotal[0] + _countTotal[1]) <= _countIndex))
			_countIndex = _countTotal[0];

	}while(totalSizeToFill > currentIdx);

SET_EVENT:
	SetEvent(_hInitialized);
}

void EPhysWaveformBuilder::ConnectCallback(void* buf)
{
	BlockRingBuffer* brBuf = (BlockRingBuffer*)buf;
	_bRingBuffer = brBuf;
	_bRingBuffer->SpaceAvailableCallback = &(BufferAvailableCallbackFunc);
}

void EPhysWaveformBuilder::SetWaveformParams(void* params)
{
	EPhysTriggerStruct* iStruct = (EPhysTriggerStruct*)params;
	_eStruct.startEdge = iStruct->startEdge;
	_eStruct.repeats = iStruct->repeats;
	_eStruct.framePerZSlice = iStruct->framePerZSlice;
	std::copy(std::begin(iStruct->stepEdge), std::end(iStruct->stepEdge), std::begin(_eStruct.stepEdge));
}

long EPhysWaveformBuilder::TryBuildWaveform(uint64_t& totalLength)
{
	ResetWaveform();

	size_t bufLength[BUF_REGION_COUNT];
	size_t bufSize[BUF_REGION_COUNT];
	unsigned char* eWaveform[BUF_REGION_COUNT] = {NULL};

	//determine total length of 1 repeat, including frame per slice
	int lastNonZeroID = -1;		//0-based
	bufLength[1] = (0 ==_eStruct.stepEdge[0]) ? _eStruct.framePerZSlice : 0;
	for (int i = 0; i < _MAX_PATH; i++)
	{
		if (0 > _eStruct.stepEdge[i])
			break;
		else
		{
			lastNonZeroID = i;
			bufLength[1] += (_eStruct.stepEdge[i] + 1) * _eStruct.framePerZSlice;
		}
	}

	if(-1 == lastNonZeroID)
	{
		totalLength = 0;
		return FALSE;
	}

	//build initial idle region
	bufLength[0] = _eStruct.startEdge * _eStruct.framePerZSlice;
	bufSize[0] = sizeof(unsigned char) * bufLength[0];
	if (0 < bufSize[0])
	{
		eWaveform[0] = (unsigned char*)malloc(bufSize[0]);
		memset(eWaveform[0], 0x0, bufSize[0]);
		for (int i = 0; i < _eStruct.framePerZSlice; i++)
		{
			eWaveform[0][bufLength[0]-1-i] = 0x1;		//startEdge 1-based
		}
	}

	//build 1 repeat of digital waveform based on given steps
	bufSize[1] = sizeof(unsigned char) * bufLength[1];
	if (0 < bufSize[1])
	{
		eWaveform[1] = (unsigned char*)malloc(bufSize[1]);
		memset(eWaveform[1], 0x0, bufSize[1]);
		unsigned char* pDst = eWaveform[1];
		for (int i = 0; i <= lastNonZeroID; i++)
		{
			pDst += _eStruct.stepEdge[i] * _eStruct.framePerZSlice;
			for (int j = 0; j < _eStruct.framePerZSlice; j++)
			{
				*pDst = 0x1;
				pDst++;
			}
		}
	}
	//zero padding at the end with single data point to bring down the line
	bufLength[2] = 1;
	bufSize[2] = sizeof(unsigned char) * bufLength[2];
	if (0 < bufSize[2])
	{
		eWaveform[2] = (unsigned char*)malloc(bufSize[2]);
		memset(eWaveform[2], 0x0, bufSize[2]);
	}
	//setup buffer for two regions, idle followed by one repeat
	for (int i = 0; i < BUF_REGION_COUNT; i++)
	{
		if (0 < bufSize[i])
		{
			std::string name = "EPhysDO" + std::to_string(i);
			_ephysBuffer[i] = new CircularBuffer(name);
			_ephysBuffer[i]->AllocMem(bufSize[i]);
			_ephysBuffer[i]->Lock();
			_ephysBuffer[i]->Write((char*)eWaveform[i], bufSize[i]);
			_ephysBuffer[i]->ReleaseLock();
			SAFE_DELETE_MEMORY(eWaveform[i]);
		}
	}


	//set total counts for callback
	_countTotal[0] = bufLength[0];
	_countTotal[1] = (0 < _eStruct.repeats) ? (_eStruct.repeats * bufLength[1]) : MAXULONG32;
	_countTotal[2] = bufLength[2];
	totalLength = (MAXULONG32 == _countTotal[1]) ? MAXULONG32 : (_countTotal[0] + _countTotal[1] + _countTotal[2]);
	ResetCounter();

	ResetEvent(_hInitialized);
	return TRUE;
}

void EPhysWaveformBuilder::ResetWaveform()
{ 
	for (int i = 0; i < BUF_REGION_COUNT; i++)
	{
		if (NULL != _ephysBuffer[i]) 
		{
			_ephysBuffer[i]->ReleaseMem();
			delete _ephysBuffer[i];
			_ephysBuffer[i] = NULL;
		} 
	}
};
