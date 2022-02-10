// EPhysAOWaveformBuilder.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "EPhysAOWaveformBuilder.h"

///	***************************************** <summary>				EPhysAOWaveformBuilder Constructor			</summary>	********************************************** ///

EPhysAOWaveformBuilder::EPhysAOWaveformBuilder()
{
	ResetCounter();
}

///	***************************************** <summary> EPhysAOWaveformBuilder static members & global variables	</summary>	********************************************** ///

bool EPhysAOWaveformBuilder::_instanceFlag = false;
std::unique_ptr<EPhysAOWaveformBuilder> EPhysAOWaveformBuilder::_single;
uint64_t EPhysAOWaveformBuilder::_countTotal[BUF_REGION_COUNT] = { 0 };
uint64_t EPhysAOWaveformBuilder::_countIndex = 0;
long EPhysAOWaveformBuilder::_repeatWaveform = 0;
BlockRingBuffer* EPhysAOWaveformBuilder::_bRingBuffer = { NULL };
CircularBuffer* EPhysAOWaveformBuilder::_ephysBuffer[BUF_REGION_COUNT] = { NULL };
HANDLE EPhysAOWaveformBuilder::_hInitialized = CreateEvent(NULL, true, true, NULL);	//2nd parameter "true" so it needs manual "Reset" after "Set (signal)" event

///	***************************************** <summary>				EPhysAOWaveformBuilder Functions				</summary>	********************************************** ///

// singleton
EPhysAOWaveformBuilder* EPhysAOWaveformBuilder::getInstance()
{
	if (!_instanceFlag)
	{
		try
		{
			_single.reset(new EPhysAOWaveformBuilder());
			_instanceFlag = true;
			return _single.get();
		}
		catch (...)
		{
			throw;
		}
	}
	return _single.get();
}

void EPhysAOWaveformBuilder::BufferAvailableCallbackFunc(long sType, long bufSpaceNum)
{
	if ((NULL == _bRingBuffer) || (NULL == _ephysBuffer[1]) || (NULL == _ephysBuffer[2]))
		goto SET_EVENT;

	//only one signal type in this builder
	if (_bRingBuffer->GetSignalType() != sType)
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
	unsigned long oneRepeatSize = static_cast<unsigned long>(_ephysBuffer[1]->Size() / UnitSizeInBytes((SignalType)_bRingBuffer->GetSignalType()));
	double* dSrc = NULL, * dTgt = NULL;
	UCHAR* cSrc = NULL;

	do
	{
		//determine current index location among the whole waveform,
		//idle section:
		if ((0 < _countTotal[0]) && (_countTotal[0] > _countIndex))
		{
			countToCopy = std::min(static_cast<unsigned long>(_countTotal[0] - _countIndex), static_cast<unsigned long>(totalSizeToFill - currentIdx));
			_ephysBuffer[0]->Lock();
			cSrc = _ephysBuffer[0]->Copy(countToCopy * sizeof(double));
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
			cSrc = _ephysBuffer[1]->Copy(countToCopy * sizeof(double));
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
			cSrc = _ephysBuffer[2]->Copy(countToCopy * sizeof(double));
			_bRingBuffer->WriteUnits(cSrc, countToCopy);
			SAFE_DELETE_MEMORY(cSrc);
			_ephysBuffer[2]->ReleaseLock();
		}

		_countIndex += countToCopy;
		currentIdx += countToCopy;

		//avoid overflow in continuous mode, and only repeat waveform body
		if (1 == _repeatWaveform && (_countTotal[0] + _countTotal[1]) <= _countIndex)
			_countIndex = _countTotal[0];

	} while (totalSizeToFill > currentIdx);

SET_EVENT:
	SetEvent(_hInitialized);
}

void EPhysAOWaveformBuilder::ConnectCallback(void* buf)
{
	_bRingBuffer = (BlockRingBuffer*)buf;
	_bRingBuffer->SpaceAvailableCallback = &(BufferAvailableCallbackFunc);
}

void EPhysAOWaveformBuilder::SetWaveformParams(void* params)
{
	EPhysTriggerStruct* iStruct = (EPhysTriggerStruct*)params;
	SAFE_MEMCPY(&_eStruct, sizeof(EPhysTriggerStruct), (void*)iStruct);
}

long EPhysAOWaveformBuilder::TryBuildWaveform(uint64_t& totalLength)
{
	ResetWaveform();

	size_t bufSize[BUF_REGION_COUNT] = { 0 };
	unsigned char* eWaveform[BUF_REGION_COUNT] = { NULL };
	double* pSrc = NULL;

	size_t durationLength = static_cast<size_t>(_eStruct.durationMS * _eStruct.clockRateHz / Constants::MS_TO_SEC);
	size_t idleLength = static_cast<size_t>(_eStruct.idleMS * _eStruct.clockRateHz / Constants::MS_TO_SEC);
	size_t periodLength = durationLength + idleLength;

	//find last valid power index, 
	//build body of waveform if more power settings in continuous mode (0 == _eStruct.iterations)
	long powerID = 0;
	for (int i = 0; i < _MAX_PATH; i++)
	{
		if (0 <= _eStruct.powerPercent[i])
			powerID = i;
		else
			break;
	}
	_repeatWaveform = (0 >= _eStruct.iterations) ? 1 : 0;
	long iterationCount = (0 >= _eStruct.iterations) ? std::max((long)1, powerID + 1) : _eStruct.iterations;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// build the waveform, clock rate dependent - the length of the waveform depends on the clockrate to make up fixed high and low times.//
	// build [0]: initial idle region,																									  //
	// build [1]: # repeats of duration + idle time,																					  //
	// build [2]: zero-padding with single data point to bring down the line															  //
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	_countTotal[0] = static_cast<uint64_t>(_eStruct.startIdleMS * _eStruct.clockRateHz / Constants::MS_TO_SEC);
	_countTotal[1] = periodLength * iterationCount;
	_countTotal[2] = 1;

	//return if no body to build
	if (0 >= _countTotal[1])
		return FALSE;

	for (int i = 0; i < BUF_REGION_COUNT; i++)
		bufSize[i] = sizeof(double) * _countTotal[i];

	//build initial idle
	if (0 < bufSize[0])
	{
		eWaveform[0] = (unsigned char*)malloc(bufSize[0]);
		pSrc = (double*)eWaveform[0];
		for (int i = 0; i < _countTotal[0]; i++)
		{
			*pSrc = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, _eStruct.voltageRange[0]));
			pSrc++;
		}
	}

	//build # repeats of (duration + idle) time
	if (0 < bufSize[1])
	{
		eWaveform[1] = (unsigned char*)malloc(bufSize[1]);
		pSrc = (double*)eWaveform[1];
		powerID = 0;
		for (long i = 0; i < iterationCount; i++)
		{
			powerID = (i < EPHYS_ARRAY_SIZE && 0 <= _eStruct.powerPercent[i]) ? i : powerID;
			double intermediateValue = 0;
			switch (_eStruct.responseType)
			{
			case SINE_RESPONSE:
				intermediateValue = acos(1 - static_cast<double>(Constants::AREA_UNDER_CURVE) * _eStruct.powerPercent[powerID] / (double)Constants::HUNDRED_PERCENT) / PI;
				break;
			case LINEAR_RESPONSE:
			default:
				intermediateValue = _eStruct.powerPercent[powerID] / (double)Constants::HUNDRED_PERCENT;
				break;
			}

			double powerVal = intermediateValue * (_eStruct.voltageRange[1] - _eStruct.voltageRange[0]) + _eStruct.voltageRange[0];
			for (long j = 0; j < periodLength; j++)
			{
				*pSrc = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (j < durationLength) ? powerVal : _eStruct.voltageRange[0]));
				pSrc++;
			}
		}
	}

	//padding at the end with single data point to bring down the line
	if (0 < bufSize[2])
	{
		eWaveform[2] = (unsigned char*)malloc(bufSize[2]);
		pSrc = (double*)eWaveform[2];
		*pSrc = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, _eStruct.voltageRange[0]));
	}

	//setup buffer for all regions, idle followed by body and padding
	for (int i = 0; i < BUF_REGION_COUNT; i++)
	{
		if (0 < bufSize[i])
		{
			std::string name = "EPhysAO" + std::to_string(i);
			_ephysBuffer[i] = new CircularBuffer(name);
			_ephysBuffer[i]->AllocMem(bufSize[i]);
			_ephysBuffer[i]->Lock();
			_ephysBuffer[i]->Write((char*)eWaveform[i], bufSize[i]);
			_ephysBuffer[i]->ReleaseLock();
			SAFE_DELETE_MEMORY(eWaveform[i]);
		}
	}

	//set total counts for callback
	totalLength = _countTotal[0] + _countTotal[1] + _countTotal[2];
	ResetCounter();

	ResetEvent(_hInitialized);
	return TRUE;
}

void EPhysAOWaveformBuilder::ResetWaveform()
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
