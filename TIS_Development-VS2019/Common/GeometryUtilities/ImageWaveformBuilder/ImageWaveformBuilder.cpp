// ImageWaveformBuilder.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ImageWaveformBuilder.h"
#include "WaveformMemory.h"

#define THORDAQ_DIG_ACTIVE_ENVELOPE 0x101;
#define THORDAQ_DIG_CYCLE_COMPLETE 0x202;
#define THORDAQ_DIG_CYCLE_ENVELOPE 0x404;
#define THORDAQ_DIG_EPOCH_ENVELOPE 0x1010;
#define THORDAQ_DIG_ITERATION_ENVELOPE 0x2020;
#define THORDAQ_DIG_LOW 0x0;
#define THORDAQ_DIG_PATTERN_COMPLETE 0x4040;
#define THORDAQ_DIG_PATTERN_ENVELOPE 0x8080;
#define THORDAQ_DIG_POCKELS_DIGI 0x808;

ImageWaveformBuilder::ImageWaveformBuilder(int type)
{
	_daqType = type;
	switch(_daqType)
	{
	case (int)BuilderType::ALAZAR:
		_daqSampleSize = 32;
		break;
	default:
		_daqSampleSize = 32;
		break;
	}
	InitializeParams();
	ResetCounter();
	_waveformFileName = L"";
}

///	***************************************** <summary> static members & global variables </summary>	********************************************** ///

#ifdef LOGGING_ENABLED
std::auto_ptr<LogDll> logDll(new LogDll(L"..\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

bool ImageWaveformBuilder::_instanceFlag = false;
std::unique_ptr<ImageWaveformBuilder> ImageWaveformBuilder::_single;
unsigned long ImageWaveformBuilder::_countPerCallback[SignalType::SIGNALTYPE_LAST] = {0}; 
uint64_t ImageWaveformBuilder::_countTotal[MAX_MULTI_AREA_SCAN_COUNT][SignalType::SIGNALTYPE_LAST][3];
uint64_t ImageWaveformBuilder::_countIndex[MAX_MULTI_AREA_SCAN_COUNT][(int)(SignalType::SIGNALTYPE_LAST)];
WaveformGenParams ImageWaveformBuilder::_wParams;
GGalvoWaveformParams ImageWaveformBuilder::_gParams[MAX_MULTI_AREA_SCAN_COUNT];
GGalvoWaveformParams ImageWaveformBuilder::_gWaveXY[MAX_MULTI_AREA_SCAN_COUNT];
ThorDAQGGWaveformParams ImageWaveformBuilder::_gThorDAQParams[MAX_MULTI_AREA_SCAN_COUNT];
ThorDAQGGWaveformParams ImageWaveformBuilder::_gThorDAQWaveXY[MAX_MULTI_AREA_SCAN_COUNT];
BlockRingBuffer* ImageWaveformBuilder::_bRingBuffer[(int)(SignalType::SIGNALTYPE_LAST)] = {NULL};
long ImageWaveformBuilder::_scanAreaId = 0;
wchar_t message[_MAX_PATH] = {NULL};
DWORD startTime = GetTickCount();

void ImageWaveformBuilder::BufferAvailableCallbackFunc(long sType, long bufSpaceNum)
{
	unsigned long totalSizeToFill = bufSpaceNum * _countPerCallback[sType];	//total size to be copied
	unsigned long currentIdx = 0;											//index within totalSizeToFill
	unsigned long countToCopy[3] = {0};										//count to be copied per if/else section
	unsigned long bufSpaceIdx = 0;											//current index of copied buffer space number
	unsigned char pockelsCount = GetPockelsCount();
	double* dSrc = NULL, *dTgt = NULL;
	UCHAR* cSrc = NULL;

	//do not continue if user did not reset global index:
	if (0 == (_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1] + _countTotal[_scanAreaId][sType][2]))
		return;

	//return if no line or pockels:
	switch ((SignalType)sType)
	{
	case SignalType::ANALOG_POCKEL:
		if(0 == pockelsCount)
			return;
		break;
	case SignalType::DIGITAL_LINES:
		if(0 == _wParams.digLineSelect)
			return;
		break;
	}

	if((_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1] + _countTotal[_scanAreaId][sType][2]) <= _countIndex[_scanAreaId][sType])
	{
		if((_wParams.scanAreaCount - 1) <= _scanAreaId)
		{
			return;
		}
		else
		{
			_scanAreaId++;
		}
	}

	if(FALSE == GetMutex(_gWaveXY[_scanAreaId].bufferHandle))
		return;
	if(FALSE == GetMutex(_gParams[_scanAreaId].bufferHandle))
	{
		ReleaseMutex(_gWaveXY[_scanAreaId].bufferHandle);
		return;
	}
	do
	{

		//determine current index location among the whole waveform,
		//Travel to start section:
		if ((0 < _countTotal[_scanAreaId][sType][0]) && (_countTotal[_scanAreaId][sType][0] > _countIndex[_scanAreaId][sType]))		
		{
			countToCopy[sType] = std::min(static_cast<unsigned long>(_countTotal[_scanAreaId][sType][0] - _countIndex[_scanAreaId][sType]), static_cast<unsigned long>(totalSizeToFill - currentIdx));

			//only can write up to 1 block at a time
			countToCopy[sType] = std::min(_countPerCallback[sType], countToCopy[sType]);

			//offset from beginning within 
			long offset = std::max(static_cast<long>(0), static_cast<long>(_countIndex[_scanAreaId][sType] % _gWaveXY[_scanAreaId].unitSize[sType]));

			switch ((SignalType)sType)
			{
			case ANALOG_XY:
				dSrc = _gWaveXY[_scanAreaId].GalvoWaveformXY + 2 * offset;
				_bRingBuffer[sType]->WriteUnits((UCHAR*)dSrc, countToCopy[sType] * 2);
				break;
			case ANALOG_POCKEL:
				//no waveform should be copied, Pockels only driven by line triggers
				break;
			case DIGITAL_LINES:
				cSrc = _gWaveXY[_scanAreaId].DigBufWaveform + _gWaveXY[_scanAreaId].digitalLineCnt * offset;
				_bRingBuffer[sType]->WriteUnits(cSrc, countToCopy[sType] * _gWaveXY[_scanAreaId].digitalLineCnt);
				break;
			}

			_countIndex[_scanAreaId][sType] += countToCopy[sType];
			currentIdx += countToCopy[sType];
		}
		//multi-frame section: unitSize * frame #; _gParams: single frame waveform 
		else if ((0 < (_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1])) && 
			((_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1]) > _countIndex[_scanAreaId][sType]))
		{
			//unit size to copy
			unsigned long unitCopySize = std::min(static_cast<unsigned long>(_gParams[_scanAreaId].unitSize[sType]), _countPerCallback[sType]);

			//offset from beginning of one frame, only applies if countPerCallback is smaller than a frame
			unsigned long offset = std::max(static_cast<long>(0), static_cast<long>((_countIndex[_scanAreaId][sType] - _countTotal[_scanAreaId][sType][0]) % _gParams[_scanAreaId].unitSize[sType]));

			//remain to copy in one frame
			unsigned long remain = static_cast<unsigned long>(_gParams[_scanAreaId].unitSize[sType] - offset);

			//remain to copy in callback
			unsigned long remainInCallback = static_cast<unsigned long>(_countPerCallback[sType] - (currentIdx % _countPerCallback[sType]));

			countToCopy[sType] = std::max(static_cast<unsigned long>(0), std::min(std::min(std::min(remain, unitCopySize), remainInCallback), static_cast<unsigned long>(totalSizeToFill - currentIdx)));

			switch ((SignalType)sType)
			{
			case ANALOG_XY:
				dSrc = _gParams[_scanAreaId].GalvoWaveformXY + 2 * offset;
				_bRingBuffer[sType]->WriteUnits((UCHAR*)dSrc, countToCopy[sType] * 2);
				break;
			case ANALOG_POCKEL:
				//pockels waveform of one line
				dSrc = _gParams[_scanAreaId].GalvoWaveformPockel + pockelsCount * offset;
				_bRingBuffer[sType]->WriteUnits((UCHAR*)dSrc, countToCopy[sType] * pockelsCount);
				break;
			case DIGITAL_LINES:
				cSrc = _gParams[_scanAreaId].DigBufWaveform + _gParams[_scanAreaId].digitalLineCnt * offset;
				_bRingBuffer[sType]->WriteUnits(cSrc, countToCopy[sType] * _gParams[_scanAreaId].digitalLineCnt);
				break;
			}

			_countIndex[_scanAreaId][sType] += countToCopy[sType];
			currentIdx += countToCopy[sType];
		}
		else
		{
			//next scan area:
			if ((_wParams.scanAreaCount - 1) > _scanAreaId)
			{
				_scanAreaId++;
			}
			//end of all scan area, patch for callback section, repeat last since it should only be hit once:
			else if ((0 < (_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1] + _countTotal[_scanAreaId][sType][2])) && 
				((_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1] + _countTotal[_scanAreaId][sType][2]) > _countIndex[_scanAreaId][sType]))
			{
				countToCopy[sType] = std::min(static_cast<unsigned long>(_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1] + _countTotal[_scanAreaId][sType][2] - _countIndex[_scanAreaId][sType]), 
					static_cast<unsigned long>(totalSizeToFill - currentIdx));

				//only can write up to 1 block at a time
				countToCopy[sType] = std::min(_countPerCallback[sType], countToCopy[sType]);

				unsigned long long lastIdx = std::max((unsigned long long)0, _gParams[_scanAreaId].unitSize[sType] - 1);

				switch ((SignalType)sType)
				{
				case ANALOG_XY:
					dSrc = (double*)malloc(countToCopy[sType] * 2 * sizeof(double));
					dTgt = dSrc;
					for (unsigned long i = currentIdx; i < (currentIdx + countToCopy[sType]); i++)
					{
						*dTgt = std::max(MIN_AO_VOLTAGE, std::min(_gParams[_scanAreaId].GalvoWaveformXY[2*lastIdx],MAX_AO_VOLTAGE));
						*(dTgt+1) = std::max(MIN_AO_VOLTAGE, std::min(_gParams[_scanAreaId].GalvoWaveformXY[2*lastIdx + 1],MAX_AO_VOLTAGE));
						dTgt += 2;
					}
					_bRingBuffer[sType]->WriteUnits((UCHAR*)dSrc, countToCopy[sType] * 2);
					SAFE_DELETE_MEMORY(dSrc);
					break;
				case ANALOG_POCKEL:
					dSrc = (double*)malloc(countToCopy[sType] * pockelsCount * sizeof(double));
					dTgt = dSrc;
					for (unsigned long i = 0; i < countToCopy[sType]; i++)
					{
						for (int j = 0; j < pockelsCount; j++)
						{
							*dTgt = std::max(MIN_AO_VOLTAGE, std::min(_gParams[_scanAreaId].GalvoWaveformPockel[pockelsCount*lastIdx + j],MAX_AO_VOLTAGE));
							dTgt++;
						}
					}
					_bRingBuffer[sType]->WriteUnits((UCHAR*)dSrc, countToCopy[sType] * pockelsCount);
					SAFE_DELETE_MEMORY(dSrc);
					break;
				case DIGITAL_LINES:
					cSrc = (UCHAR*)malloc(countToCopy[sType] * _gParams[_scanAreaId].digitalLineCnt * sizeof(unsigned char));
					memset(cSrc, 0x0, countToCopy[sType] * _gParams[_scanAreaId].digitalLineCnt * sizeof(unsigned char));
					_bRingBuffer[sType]->WriteUnits(cSrc, countToCopy[sType] * _gParams[_scanAreaId].digitalLineCnt);
					SAFE_DELETE_MEMORY(cSrc);
					break;
				}

				_countIndex[_scanAreaId][sType] += countToCopy[sType];
				currentIdx += countToCopy[sType];
			}		
		}
	}while(totalSizeToFill > currentIdx);

	ReleaseMutex(_gParams[_scanAreaId].bufferHandle);
	ReleaseMutex(_gWaveXY[_scanAreaId].bufferHandle);
	return;
}

void ImageWaveformBuilder::ConnectBufferCallback(SignalType sType, BlockRingBuffer* brBuf)
{
	//start from 0 at initial
	_scanAreaId = 0;

	_bRingBuffer[sType] = brBuf;
	if (NULL != _bRingBuffer[sType])
		_bRingBuffer[sType]->SpaceAvailableCallback = &(BufferAvailableCallbackFunc);
}

// singleton
ImageWaveformBuilder* ImageWaveformBuilder::getInstance(long type)
{
	if(!_instanceFlag)
	{
		try
		{
			_single.reset(new ImageWaveformBuilder(type));
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

void ImageWaveformBuilder::SetWaveformGenParams(void* params)
{
	WaveformGenParams* param = (WaveformGenParams*)params;
	_wParams = *param;
}

void ImageWaveformBuilder::ResetGGalvoWaveformParams()
{
	for (int i = 0; i < MAX_MULTI_AREA_SCAN_COUNT; i++)
	{
		long unitSize[SignalType::SIGNALTYPE_LAST] = { 0 };
		ResetGGalvoWaveformParam(&_gParams[i], unitSize, 0, 0);
		ResetGGalvoWaveformParam(&_gWaveXY[i], unitSize, 0, 0);
	}
}

void ImageWaveformBuilder::ResetThorDAQGGalvoWaveformParams()
{
	for (int i = 0; i < MAX_MULTI_AREA_SCAN_COUNT; i++)
	{
		long unitSize[SignalType::SIGNALTYPE_LAST] = { 0 };
		ResetThorDAQGGWaveformParam(&_gThorDAQParams[i], unitSize, 0, 0);
		ResetThorDAQGGWaveformParam(&_gThorDAQWaveXY[i], unitSize, 0, 0);
	}
}




uint64_t ImageWaveformBuilder::GetCounter(SignalType sType)
{
	return _countIndex[_scanAreaId][sType];
}

// retrieve galvo waveform params for custom imaging
long ImageWaveformBuilder::GetGGalvoWaveformParams(void* params)
{
	GGalvoWaveformParams* param = (GGalvoWaveformParams*)params;
	*param = _gParams[_scanAreaId];
	return TRUE;
}

///	***************************************** <summary> File Mapping Functions </summary>	********************************************** ///

// retrieve galvo waveform params from memory map file initlaized by RebuildWaveformFromFile with travel to start
long ImageWaveformBuilder::GetGGalvoWaveformParams(SignalType sType, void* params, long preCaptureStatus, uint64_t& indexNow)
{
	if(NULL != _gParams[_scanAreaId].bufferHandle)
	{
		if(WAIT_OBJECT_0 != WaitForSingleObject(_gParams[_scanAreaId].bufferHandle, 3*EVENT_WAIT_TIME))					//timeout: 15 seconds
			return FALSE;
	}
	long unitSize[SignalType::SIGNALTYPE_LAST] = { _countPerCallback[sType], _countPerCallback[sType], _countPerCallback[sType], 0 };
	if(FALSE == ResetGGalvoWaveformParam(&_gParams[_scanAreaId], unitSize, _gWaveXY[_scanAreaId].pockelsCount, _gWaveXY[_scanAreaId].digitalLineCnt))	//count to be copied before return is _countPerCallback
	{
		ReleaseMutex(_gParams[_scanAreaId].bufferHandle);
		return FALSE;
	}
	unsigned long currentIdx = 0;										//index within _countPerCallback
	unsigned long countToCopy[SignalType::SIGNALTYPE_LAST] = {0};		//count to be copied per if/else section

	//update reset params:
	_gParams[_scanAreaId].ClockRate = _gWaveXY[_scanAreaId].ClockRate;
	_gParams[_scanAreaId].stepVolt = _gWaveXY[_scanAreaId].stepVolt;
	_gParams[_scanAreaId].Scanmode = ScanMode::BLEACH_SCAN;
	_gParams[_scanAreaId].PreCapStatus = preCaptureStatus;
	_gParams[_scanAreaId].pockelsCount = _gWaveXY[_scanAreaId].pockelsCount;

	//do not continue if user did not reset global index:
	if((_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1] + _countTotal[_scanAreaId][sType][2]) <= _countIndex[_scanAreaId][sType])
	{
		ReleaseMutex(_gParams[_scanAreaId].bufferHandle);
		return FALSE;
	}

	do
	{
		//determine current index location among the whole waveform,
		//Travel to start section:
		if ((0 < _countTotal[_scanAreaId][sType][0]) && (_countTotal[_scanAreaId][sType][0] > _countIndex[_scanAreaId][sType]))		
		{
			countToCopy[sType] = std::min(static_cast<unsigned long>(_countTotal[_scanAreaId][sType][0] - _countIndex[_scanAreaId][sType]), static_cast<unsigned long>(_countPerCallback[sType] - currentIdx));

			int lineIdx = -1;
			switch (sType)
			{
			case ANALOG_XY:
				//travel to start:
				SAFE_MEMCPY((void*)(_gParams[_scanAreaId].GalvoWaveformXY + (2*currentIdx)), (2*countToCopy[sType]*WaveformMemory::GetDataTypeSizeInBytes<double>()), (void*)(_gWaveXY[_scanAreaId].GalvoWaveformXY + (2*currentIdx)));
				break;
			case ANALOG_POCKEL:
				for (int pid = 0; pid < _gParams[_scanAreaId].pockelsCount; pid++)
				{
					SAFE_MEMCPY((void*)(_gParams[_scanAreaId].GalvoWaveformPockel + (pid*_countPerCallback[sType]) + currentIdx), (countToCopy[sType]*WaveformMemory::GetDataTypeSizeInBytes<double>()), (void*)(_gWaveXY[_scanAreaId].GalvoWaveformPockel + (pid*_countTotal[_scanAreaId][sType][0]) + currentIdx));
				}
				break;
			case DIGITAL_LINES:
				//digital lines:
				for (int i = BLEACHSCAN_DIGITAL_LINENAME::DUMMY; i < BLEACHSCAN_DIGITAL_LINENAME::DIGITAL_LINENAME_LAST; i++)
				{
					//check if user selected:
					if(0 < (_digitalLineSelection & (0x1 << i)))
					{
						lineIdx++;
					}
					else
					{
						continue;
					}

					if ((BLEACHSCAN_DIGITAL_LINENAME::ACTIVE_ENVELOPE == i)	|| (BLEACHSCAN_DIGITAL_LINENAME::CYCLE_COMPLEMENTARY == i))		//ActiveEnvelope, cycleComplementary set to be high
					{
						std::memset((void*)(_gParams[_scanAreaId].DigBufWaveform + (lineIdx*_countPerCallback[sType]) + currentIdx), 0x1, (countToCopy[sType]*WaveformMemory::GetDataTypeSizeInBytes<unsigned char>()));
					}
					else if(BLEACHSCAN_DIGITAL_LINENAME::CYCLE_COMPLETE == i)			//CycleComplete set to be low
					{						
						std::memset((void*)(_gParams[_scanAreaId].DigBufWaveform + (lineIdx*_countPerCallback[sType]) + currentIdx), 0x0, (countToCopy[sType]*WaveformMemory::GetDataTypeSizeInBytes<unsigned char>()));
					}
					else if(((BLEACHSCAN_DIGITAL_LINENAME::DUMMY == i)) && (0 == _countIndex[_scanAreaId][sType])) //at Dummy and beginning of waveform
					{
						//first of Dummy is 1 at the very first:
						std::memset(_gParams[_scanAreaId].DigBufWaveform, 0x0, (countToCopy[sType]*WaveformMemory::GetDataTypeSizeInBytes<unsigned char>()));
						std::memset(_gParams[_scanAreaId].DigBufWaveform, 0x1, (1*WaveformMemory::GetDataTypeSizeInBytes<unsigned char>()));
					}
					else
					{
						std::memset((void*)(_gParams[_scanAreaId].DigBufWaveform + (lineIdx*_countPerCallback[sType]) + currentIdx), 0x0, (countToCopy[sType]*WaveformMemory::GetDataTypeSizeInBytes<unsigned char>()));
					}
				}			
				break;
			default:
				break;
			}

			_countIndex[_scanAreaId][sType] += countToCopy[sType];
			currentIdx += countToCopy[sType];
		}
		//file load section:
		else if ((0 < (_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1])) && ((_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1]) > _countIndex[_scanAreaId][sType]))
		{
			countToCopy[sType] = std::min(static_cast<unsigned long>(_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1] - _countIndex[_scanAreaId][sType]), static_cast<unsigned long>(_countPerCallback[sType] - currentIdx));

			//offset from beginning of file
			long fileStartOffset = std::max((long)0, static_cast<long>(_countIndex[_scanAreaId][sType] - _countTotal[_scanAreaId][sType][0]));	

			char* ptr = NULL;
			int lineIdx = -1;
			int fileLineIdx = 0;	//no dummy in file
			switch (sType)
			{
			case ANALOG_XY:
				//load analogXY:
				ptr = WaveformMemory::getInstance()->GetMemMapPtr(SignalType::ANALOG_XY, 2*fileStartOffset, 2*countToCopy[sType]);
				SAFE_MEMCPY((void*)(_gParams[_scanAreaId].GalvoWaveformXY + (2*currentIdx)), (2*countToCopy[sType]*WaveformMemory::GetDataTypeSizeInBytes<double>()), (void*)(ptr));
				WaveformMemory::getInstance()->UnlockMemMapPtr();
				break;
			case ANALOG_POCKEL:
				//analog Pockel, digial lines are build-all & load-selected, 
				//but pockels lines are build-selected-no-gap to save memory space
				for (int pid = 0; pid < _gParams[_scanAreaId].pockelsCount; pid++)
				{
					ptr = WaveformMemory::getInstance()->GetMemMapPtr(SignalType::ANALOG_POCKEL, (pid * _countTotal[_scanAreaId][sType][1]) + fileStartOffset, countToCopy[sType]);
					SAFE_MEMCPY((void*)(_gParams[_scanAreaId].GalvoWaveformPockel + (pid * _countPerCallback[sType]) + currentIdx), (countToCopy[sType]*WaveformMemory::GetDataTypeSizeInBytes<double>()), (void*)(ptr));
					WaveformMemory::getInstance()->UnlockMemMapPtr();
				}
				break;
			case DIGITAL_LINES:
				//digital lines: dummy, pockels digital, complete, cycle, iteration, pattern, patternComplete, active
				//dummy should be 1 at the very first one:
				if(0 < (_digitalLineSelection & (0x1 << BLEACHSCAN_DIGITAL_LINENAME::DUMMY)))
				{
					lineIdx++;
					memset((void*)(_gParams[_scanAreaId].DigBufWaveform + currentIdx), 0x0, (countToCopy[sType]*WaveformMemory::GetDataTypeSizeInBytes<unsigned char>()));
					if(0 == _countIndex[_scanAreaId][sType])
					{
						memset((void*)(_gParams[_scanAreaId].DigBufWaveform + currentIdx), 0x1, 1*WaveformMemory::GetDataTypeSizeInBytes<unsigned char>());
					}
				}
				if(0 < (_digitalLineSelection & (0x1 << BLEACHSCAN_DIGITAL_LINENAME::POCKEL_DIG)))
				{
					ptr = WaveformMemory::getInstance()->GetMemMapPtr(SignalType::DIGITAL_LINES, (fileLineIdx * _countTotal[_scanAreaId][sType][1]) + fileStartOffset, countToCopy[sType]);
					lineIdx++;
					SAFE_MEMCPY((void*)(_gParams[_scanAreaId].DigBufWaveform + (lineIdx*_countPerCallback[sType]) + currentIdx), (countToCopy[sType]*WaveformMemory::GetDataTypeSizeInBytes<unsigned char>()), (void*)(ptr));
					WaveformMemory::getInstance()->UnlockMemMapPtr();
				}
				fileLineIdx++;
				//check complete, cycle:
				switch (preCaptureStatus)
				{
				case (long)PreCaptureStatus::PRECAPTURE_WAVEFORM_MID_CYCLE:			//in cycles, low start means allow cycle envelope to have gap
					if(0 < (_digitalLineSelection & (0x1 << BLEACHSCAN_DIGITAL_LINENAME::ACTIVE_ENVELOPE)))
					{
						lineIdx++;
						memset((void*)(_gParams[_scanAreaId].DigBufWaveform + (lineIdx*_countPerCallback[sType]) + currentIdx), 0x1, countToCopy[sType]*WaveformMemory::GetDataTypeSizeInBytes<unsigned char>());		//ActiveEnvelope always high if not last
					}
					fileLineIdx++;
					if(0 < (_digitalLineSelection & (0x1 << BLEACHSCAN_DIGITAL_LINENAME::CYCLE_COMPLETE)))
					{
						lineIdx++;
						memset((void*)(_gParams[_scanAreaId].DigBufWaveform + (lineIdx*_countPerCallback[sType]) + currentIdx), 0x0, countToCopy[sType]*WaveformMemory::GetDataTypeSizeInBytes<unsigned char>());		//CycleComplete always low if not last
					}
					fileLineIdx++;
					break;
				case (long)PreCaptureStatus::PRECAPTURE_WAVEFORM_LAST_CYCLE:			//last cycle, low start means allow cycle envelope to have gap
					for (int digiID = (int)BLEACHSCAN_DIGITAL_LINENAME::ACTIVE_ENVELOPE; digiID <= (int)BLEACHSCAN_DIGITAL_LINENAME::CYCLE_COMPLETE; digiID++)
					{
						if(0 < (_digitalLineSelection & (0x1 << digiID)))
						{
							ptr = WaveformMemory::getInstance()->GetMemMapPtr(SignalType::DIGITAL_LINES, (fileLineIdx * _countTotal[_scanAreaId][sType][1]) + fileStartOffset, countToCopy[sType]);
							lineIdx++;
							SAFE_MEMCPY((void*)(_gParams[_scanAreaId].DigBufWaveform + (lineIdx*_countPerCallback[sType]) + currentIdx), (countToCopy[sType]*WaveformMemory::GetDataTypeSizeInBytes<unsigned char>()), (void*)(ptr));
							WaveformMemory::getInstance()->UnlockMemMapPtr();
						}
						fileLineIdx++;
					}
					break;
				}

				//load iteration, pattern, patternComplete, epoch, cycleInverse:
				for (int digiID = (int)BLEACHSCAN_DIGITAL_LINENAME::CYCLE_ENVELOPE; digiID < (int)BLEACHSCAN_DIGITAL_LINENAME::DIGITAL_LINENAME_LAST; digiID++)
				{
					if(0 < (_digitalLineSelection & (0x1 << digiID)))
					{
						ptr = WaveformMemory::getInstance()->GetMemMapPtr(SignalType::DIGITAL_LINES, (fileLineIdx * _countTotal[_scanAreaId][sType][1]) + fileStartOffset, countToCopy[sType]);
						lineIdx++;
						SAFE_MEMCPY((void*)(_gParams[_scanAreaId].DigBufWaveform + (lineIdx*_countPerCallback[sType]) + currentIdx), (countToCopy[sType]*WaveformMemory::GetDataTypeSizeInBytes<unsigned char>()), (void*)(ptr));
						WaveformMemory::getInstance()->UnlockMemMapPtr();
					}
					fileLineIdx++;
				}
				break;
			default:
				break;
			}

			_countIndex[_scanAreaId][sType] += countToCopy[sType];
			currentIdx += countToCopy[sType];
		}
		//patch for callback section:
		else if ((0 < (_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1] + _countTotal[_scanAreaId][sType][2])) && ((_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1] + _countTotal[_scanAreaId][sType][2]) > _countIndex[_scanAreaId][sType]))
		{
			countToCopy[sType] = std::min(static_cast<unsigned long>(_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1] + _countTotal[_scanAreaId][sType][2] - _countIndex[_scanAreaId][sType]), static_cast<unsigned long>(_countPerCallback[sType] - currentIdx));
			int lastIdx = (1 < currentIdx) ? (currentIdx-1) : currentIdx;

			int lineIdx = -1;
			switch (sType)
			{
			case ANALOG_XY:
				//patch for callback, repeat last since it should only be hit once:
				for (unsigned long i = currentIdx; i < (currentIdx + countToCopy[sType]); i++)
				{
					_gParams[_scanAreaId].GalvoWaveformXY[2*i] = std::max(MIN_AO_VOLTAGE, std::min(_gParams[_scanAreaId].GalvoWaveformXY[2*lastIdx],MAX_AO_VOLTAGE));
					_gParams[_scanAreaId].GalvoWaveformXY[2*i+1] = std::max(MIN_AO_VOLTAGE, std::min(_gParams[_scanAreaId].GalvoWaveformXY[2*lastIdx+1],MAX_AO_VOLTAGE));
				}
				break;
			case ANALOG_POCKEL:
				//patch for callback, repeat last since it should only be hit once:
				for (int pid = 0; pid < _gParams[_scanAreaId].pockelsCount; pid++)
				{
					std::fill(_gParams[_scanAreaId].GalvoWaveformPockel + (pid*_countPerCallback[sType]) + currentIdx, _gParams[_scanAreaId].GalvoWaveformPockel + (pid*_countPerCallback[sType]) + currentIdx + countToCopy[sType], std::max(MIN_AO_VOLTAGE, std::min(*(_gParams[_scanAreaId].GalvoWaveformPockel + (pid*_countPerCallback[sType]) + lastIdx),MAX_AO_VOLTAGE)));
				}
				break;
			case DIGITAL_LINES:
				for (int j = BLEACHSCAN_DIGITAL_LINENAME::DUMMY; j < BLEACHSCAN_DIGITAL_LINENAME::DIGITAL_LINENAME_LAST; j++)
				{
					if(0 < (_digitalLineSelection & (0x1 << j)))
					{
						lineIdx++;
					}
					else
					{
						continue;
					}
					memset((void*)(_gParams[_scanAreaId].DigBufWaveform + (lineIdx*_countPerCallback[sType]) + currentIdx),*(_gParams[_scanAreaId].DigBufWaveform + (lineIdx*_countPerCallback[sType]) + lastIdx), (countToCopy[sType]*WaveformMemory::GetDataTypeSizeInBytes<unsigned char>()));
				}			
				break;
			default:
				break;
			}

			_countIndex[_scanAreaId][sType] += countToCopy[sType];
			currentIdx += countToCopy[sType];
		}
	}while(_countPerCallback[sType] > currentIdx);

	//done:
	indexNow = _countIndex[_scanAreaId][sType];
	ReleaseMutex(_gParams[_scanAreaId].bufferHandle);

	GGalvoWaveformParams* param = (GGalvoWaveformParams*)params;
	*param = _gParams[_scanAreaId];
	LogPerformance(L"ImageWaveformBuilder active load callback MSec");
	return TRUE;
}

///	***************************************** <summary> File Mapping Functions </summary>	********************************************** ///

// retrieve galvo waveform params from memory map file initlaized by RebuildWaveformFromFile with travel to start
long ImageWaveformBuilder::GetThorDAQGGWaveformParams(SignalType sType, void* params, long preCaptureStatus, uint64_t& indexNow)
{
	if (NULL != _gThorDAQParams[_scanAreaId].bufferHandle)
	{
		if (WAIT_OBJECT_0 != WaitForSingleObject(_gThorDAQParams[_scanAreaId].bufferHandle, 3 * EVENT_WAIT_TIME))					//timeout: 15 seconds
			return FALSE;
	}
	long unitSize[3] = { static_cast<long>(_countPerCallback[sType]), static_cast<long>(_countPerCallback[sType]), static_cast<long>(_countPerCallback[sType]) };
	if (FALSE == ResetThorDAQGGWaveformParam(&_gThorDAQParams[_scanAreaId], unitSize, 1, _gThorDAQWaveXY[_scanAreaId].digitalLineCnt))	//count to be copied before return is _countPerCallback
	{
		ReleaseMutex(_gThorDAQParams[_scanAreaId].bufferHandle);
		return FALSE;
	}
	unsigned long currentIdx = 0;										//index within _countPerCallback
	unsigned long countToCopy[SignalType::SIGNALTYPE_LAST] = { 0 };		//count to be copied per if/else section

	//update reset params:
	_gThorDAQParams[_scanAreaId].ClockRate = _gThorDAQWaveXY[_scanAreaId].ClockRate;
	_gThorDAQParams[_scanAreaId].stepVolt = _gThorDAQWaveXY[_scanAreaId].stepVolt;
	_gThorDAQParams[_scanAreaId].Scanmode = ScanMode::BLEACH_SCAN;
	_gThorDAQParams[_scanAreaId].PreCapStatus = preCaptureStatus;
	_gThorDAQParams[_scanAreaId].pockelsCount = _gThorDAQWaveXY[_scanAreaId].pockelsCount;

	//do not continue if user did not reset global index:
	if ((_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1] + _countTotal[_scanAreaId][sType][2]) <= _countIndex[_scanAreaId][sType])
	{
		ReleaseMutex(_gThorDAQParams[_scanAreaId].bufferHandle);
		return FALSE;
	}


	do
	{
		//determine current index location among the whole waveform,
		//Travel to start section:
		if ((0 < _countTotal[_scanAreaId][sType][0]) && (_countTotal[_scanAreaId][sType][0] > _countIndex[_scanAreaId][sType]))
		{
			countToCopy[sType] = std::min(static_cast<unsigned long>(_countTotal[_scanAreaId][sType][0] - _countIndex[_scanAreaId][sType]), static_cast<unsigned long>(_countPerCallback[sType] - currentIdx));

			int lineIdx = -1;
			switch (sType)
			{
				case ANALOG_XY:
					//travel to start:
					SAFE_MEMCPY((void*)(_gThorDAQParams[_scanAreaId].GalvoWaveformX + currentIdx), (countToCopy[sType] * sizeof(unsigned short)), (void*)(_gThorDAQWaveXY[_scanAreaId].GalvoWaveformX + currentIdx));
					SAFE_MEMCPY((void*)(_gThorDAQParams[_scanAreaId].GalvoWaveformY + currentIdx), (countToCopy[sType] * sizeof(unsigned short)), (void*)(_gThorDAQWaveXY[_scanAreaId].GalvoWaveformY + currentIdx));
					break;
				case ANALOG_POCKEL:
					SAFE_MEMCPY((void*)(_gThorDAQParams[_scanAreaId].GalvoWaveformPockel + currentIdx), (countToCopy[sType] * sizeof(unsigned short)), (void*)(_gThorDAQWaveXY[_scanAreaId].GalvoWaveformPockel + currentIdx));
					break;
				case DIGITAL_LINES:
					//digital lines:

					for (unsigned int i = 0; i < countToCopy[sType]; ++i)
					{
						_gThorDAQParams[_scanAreaId].DigBufWaveform[currentIdx + i] = (unsigned short)THORDAQ_DIG_ACTIVE_ENVELOPE;
					}
					break;
				default:
					break;
			}

			_countIndex[_scanAreaId][sType] += countToCopy[sType];
			currentIdx += countToCopy[sType];
		}
		//file load section:
		else if ((0 < (_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1])) && ((_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1]) > _countIndex[_scanAreaId][sType]))
		{
			countToCopy[sType] = std::min(static_cast<unsigned long>(_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1] - _countIndex[_scanAreaId][sType]), static_cast<unsigned long>(_countPerCallback[sType] - currentIdx));

			//offset from beginning of file
			long fileStartOffset = std::max((long)0, static_cast<long>(_countIndex[_scanAreaId][sType] - _countTotal[_scanAreaId][sType][0]));

			char* ptr = NULL;
			//int lineIdx = -1;
		//	int fileLineIdx = 0;	//no dummy in file
			switch (sType)
			{
				case ANALOG_XY:
					//load analogGX:
					ptr = WaveformMemory::getInstance()->GetMemMapPtrThorDAQ(SignalTypeThorDAQ::TDQANALOG_X, fileStartOffset, countToCopy[sType]);
					SAFE_MEMCPY((void*)(_gThorDAQParams[_scanAreaId].GalvoWaveformX + currentIdx), (countToCopy[sType] * sizeof(unsigned short)), (void*)(ptr));
					WaveformMemory::getInstance()->UnlockMemMapPtr();

					//load analogGY:
					ptr = WaveformMemory::getInstance()->GetMemMapPtrThorDAQ(SignalTypeThorDAQ::TDQANALOG_Y, fileStartOffset, countToCopy[sType]);
					SAFE_MEMCPY((void*)(_gThorDAQParams[_scanAreaId].GalvoWaveformY + currentIdx), (countToCopy[sType] * sizeof(unsigned short)), (void*)(ptr));
					WaveformMemory::getInstance()->UnlockMemMapPtr();
					break;
				case ANALOG_POCKEL:
					//analog Pockel:
					ptr = WaveformMemory::getInstance()->GetMemMapPtrThorDAQ(SignalTypeThorDAQ::TDQANALOG_POCKEL, fileStartOffset, countToCopy[sType]);
					SAFE_MEMCPY((void*)(_gThorDAQParams[_scanAreaId].GalvoWaveformPockel + currentIdx), (countToCopy[sType] * sizeof(unsigned short)), (void*)(ptr));
					WaveformMemory::getInstance()->UnlockMemMapPtr();
					break;
				case DIGITAL_LINES:
					ptr = WaveformMemory::getInstance()->GetMemMapPtrThorDAQ(SignalTypeThorDAQ::TDQDIGITAL_LINES, fileStartOffset, countToCopy[sType]);
					SAFE_MEMCPY((void*)(_gThorDAQParams[_scanAreaId].DigBufWaveform + currentIdx), (countToCopy[sType] * sizeof(unsigned short)), (void*)(ptr));
					WaveformMemory::getInstance()->UnlockMemMapPtr();

					//check complete, cycle:
					switch (preCaptureStatus)
					{
						case (long)PreCaptureStatus::PRECAPTURE_WAVEFORM_MID_CYCLE:			//in cycles, low start means allow cycle envelope to have gap
							if (0 < (_digitalLineSelection & (0x1 << BLEACHSCAN_DIGITAL_LINENAME::ACTIVE_ENVELOPE)))
							{
								//ActiveEnvelope always high if not last
								for (unsigned int i = 0; i < countToCopy[sType]; ++i)
								{
									_gThorDAQParams[_scanAreaId].DigBufWaveform[currentIdx + i] |= THORDAQ_DIG_ACTIVE_ENVELOPE;
								}
							}
							if (0 < (_digitalLineSelection & (0x1 << BLEACHSCAN_DIGITAL_LINENAME::CYCLE_COMPLETE)))
							{
								for (unsigned int i = 0; i < countToCopy[sType]; ++i)
								{
									_gThorDAQParams[_scanAreaId].DigBufWaveform[currentIdx + i] &= ~THORDAQ_DIG_CYCLE_COMPLETE;
								}
							}
							break;
						case (long)PreCaptureStatus::PRECAPTURE_WAVEFORM_LAST_CYCLE:			//last cycle, low start means allow cycle envelope to have gap
							//don't change the waveform if at the last cycle
							break;
					}
					break;
				default:
					break;
			}

			_countIndex[_scanAreaId][sType] += countToCopy[sType];
			currentIdx += countToCopy[sType];
		}
		//patch for callback section:
		else if ((0 < (_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1] + _countTotal[_scanAreaId][sType][2])) && ((_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1] + _countTotal[_scanAreaId][sType][2]) > _countIndex[_scanAreaId][sType]))
		{
			countToCopy[sType] = std::min(static_cast<unsigned long>(_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1] + _countTotal[_scanAreaId][sType][2] - _countIndex[_scanAreaId][sType]), static_cast<unsigned long>(_countPerCallback[sType] - currentIdx));
			int lastIdx = (1 < currentIdx) ? (currentIdx - 1) : currentIdx;

			int lineIdx = -1;
			switch (sType)
			{
				case ANALOG_XY:
					//patch for callback, repeat last since it should only be hit once:
					for (unsigned long i = currentIdx; i < (currentIdx + countToCopy[sType]); ++i)
					{
						_gThorDAQParams[_scanAreaId].GalvoWaveformX[i] = _gThorDAQParams[_scanAreaId].GalvoWaveformX[lastIdx];
						_gThorDAQParams[_scanAreaId].GalvoWaveformY[i] = _gThorDAQParams[_scanAreaId].GalvoWaveformY[lastIdx];
					}
					break;
				case ANALOG_POCKEL:
					//patch for callback, repeat last since it should only be hit once:
					for (unsigned long i = currentIdx; i < (currentIdx + countToCopy[sType]); ++i)
					{
						_gThorDAQParams[_scanAreaId].GalvoWaveformPockel[i] = _gThorDAQParams[_scanAreaId].GalvoWaveformPockel[lastIdx];
					}
					break;
				case DIGITAL_LINES:
					//patch for callback, repeat last since it should only be hit once:
					for (unsigned long i = currentIdx; i < (currentIdx + countToCopy[sType]); ++i)
					{
						_gThorDAQParams[_scanAreaId].DigBufWaveform[i] = _gThorDAQParams[_scanAreaId].DigBufWaveform[lastIdx];
					}
					break;
				default:
					break;
			}

			_countIndex[_scanAreaId][sType] += countToCopy[sType];
			currentIdx += countToCopy[sType];
		}
	} while (_countPerCallback[sType] > currentIdx);

	//done:
	indexNow = _countIndex[_scanAreaId][sType];
	ReleaseMutex(_gThorDAQParams[_scanAreaId].bufferHandle);

	ThorDAQGGWaveformParams* param = (ThorDAQGGWaveformParams*)params;
	*param = _gThorDAQParams[_scanAreaId];
	LogPerformance(L"ImageWaveformBuilder active load callback MSec");
	return TRUE;
}

long ImageWaveformBuilder::GetThorDAQGGWaveformParams(const wchar_t* waveformFileName, void* params)
{
	uint64_t ret = 0;
	_scanAreaId = 0;	//use 1st area id, to be extended in the future

	//wait for mutex:
	if (FALSE == GetMutex(_gThorDAQWaveXY[_scanAreaId].bufferHandle))
		return FALSE;

	//open mem map:
	if (FALSE == WaveformMemory::getInstance()->OpenMemThorDAQ(_gThorDAQWaveXY[_scanAreaId], waveformFileName))
	{
		ReleaseMutex(_gThorDAQWaveXY[_scanAreaId].bufferHandle);
		return FALSE;
	}

	_waveformFileName = waveformFileName;

	ThorDAQGGWaveformParams* param = (ThorDAQGGWaveformParams*)params;
	*param = _gThorDAQWaveXY[_scanAreaId];
	ReleaseMutex(_gThorDAQWaveXY[_scanAreaId].bufferHandle);
	return TRUE;
}

// build waveform including travel to start position from currentVxy
long ImageWaveformBuilder::GetGGalvoWaveformStartLoc(const wchar_t* waveformFileName, double* startXY, long& clockRate)
{
	//wait for mutex:
	if(NULL != _gWaveXY[_scanAreaId].bufferHandle)
	{
		if(WAIT_OBJECT_0 != WaitForSingleObject(_gWaveXY[_scanAreaId].bufferHandle, EVENT_WAIT_TIME))
			return FALSE;
	}

	//open mem map:
	if (FALSE == WaveformMemory::getInstance()->OpenMem(_gWaveXY[_scanAreaId], waveformFileName))
	{
		ReleaseMutex(_gWaveXY[_scanAreaId].bufferHandle);
		return FALSE;
	}

	//get clock rate:
	clockRate = static_cast<long>(_gWaveXY[_scanAreaId].ClockRate);

	//find first XY location, skip if no current location:
	if(startXY)
	{
		char* ptr = WaveformMemory::getInstance()->GetMemMapPtr(SignalType::ANALOG_XY, 0, 2);
		SAFE_MEMCPY((void*)(startXY), (2*sizeof(double)), (void*)(ptr));
		WaveformMemory::getInstance()->UnlockMemMapPtr();
	}
	ReleaseMutex(_gWaveXY[_scanAreaId].bufferHandle);
	return TRUE;
}

// build waveform including travel to start position from currentVxy
long ImageWaveformBuilder::GetThorDAQGGWaveformStartLoc(const wchar_t* waveformFileName, unsigned short* startXY, long& clockRate)
{
	//wait for mutex:
	if(NULL != _gThorDAQWaveXY[_scanAreaId].bufferHandle)
	{
		if(WAIT_OBJECT_0 != WaitForSingleObject(_gThorDAQWaveXY[_scanAreaId].bufferHandle, EVENT_WAIT_TIME))
			return FALSE;
	}

	//open mem map:
	if (FALSE == WaveformMemory::getInstance()->OpenMemThorDAQ(_gThorDAQWaveXY[_scanAreaId], waveformFileName))
	{
		ReleaseMutex(_gThorDAQWaveXY[_scanAreaId].bufferHandle);
		return FALSE;
	}

	//get clock rate:
	clockRate = static_cast<long>(_gThorDAQWaveXY[_scanAreaId].ClockRate);

	//find first XY location, skip if no current location:
	if(startXY)
	{
		char* ptr = WaveformMemory::getInstance()->GetMemMapPtrThorDAQ(SignalTypeThorDAQ::TDQANALOG_X, 0, 1);
		SAFE_MEMCPY((void*)(startXY), (sizeof(unsigned short)), (void*)(ptr));
		WaveformMemory::getInstance()->UnlockMemMapPtr();

		ptr = WaveformMemory::getInstance()->GetMemMapPtrThorDAQ(SignalTypeThorDAQ::TDQANALOG_Y, 0, 1);
		SAFE_MEMCPY((void*)(startXY + 1), (sizeof(unsigned short)), (void*)(ptr));
		WaveformMemory::getInstance()->UnlockMemMapPtr();
	}
	ReleaseMutex(_gThorDAQWaveXY[_scanAreaId].bufferHandle);
	return TRUE;
}

// build waveform including travel to start position from currentVxy
uint64_t ImageWaveformBuilder::RebuildWaveformFromFile(const wchar_t* waveformFileName, double * currentVxy, int digLineSelection, long* countPerCallback)
{
	uint64_t ret = 0;	
	_scanAreaId = 0;	//use 1st area id, to be extended in the future

	//wait for mutex:
	if(FALSE == GetMutex(_gWaveXY[_scanAreaId].bufferHandle))
		return FALSE;

	//open mem map:
	if (FALSE == WaveformMemory::getInstance()->OpenMem(_gWaveXY[_scanAreaId], waveformFileName))
	{
		ReleaseMutex(_gWaveXY[_scanAreaId].bufferHandle);
		return FALSE;
	}

	//set file unit size:
	for (int i = 0; i < static_cast<int>(SignalType::SIGNALTYPE_LAST); i++)
	{
		_countTotal[_scanAreaId][i][0] = _countTotal[_scanAreaId][i][2] = 0;												//[0] travel to start, [2] zero pedding end
		_countTotal[_scanAreaId][i][1] = _gWaveXY[_scanAreaId].analogPockelSize / _gWaveXY[_scanAreaId].pockelsCount;		//[1] unit size of waveform body
	}
	_waveformFileName = waveformFileName;

	//find first XY location, skip if no current location:
	if(currentVxy)
	{
		double positionVxy[4] = {*(currentVxy), *(currentVxy + 1), 0 , 0}; 
		char* ptr = WaveformMemory::getInstance()->GetMemMapPtr(SignalType::ANALOG_XY, 0, 2);
		SAFE_MEMCPY((void*)(positionVxy+2), (2*sizeof(double)), (void*)(ptr));
		WaveformMemory::getInstance()->UnlockMemMapPtr();

		//find power idle:
		double* powerIdle = new double[_gWaveXY[_scanAreaId].pockelsCount];
		for (int i = 0; i < _gWaveXY[_scanAreaId].pockelsCount; i++)
		{
			ptr = WaveformMemory::getInstance()->GetMemMapPtr(SignalType::ANALOG_POCKEL, i + _countTotal[_scanAreaId][i][1], 1);
			SAFE_MEMCPY((void*)(&powerIdle[i]), (sizeof(double)), (void*)(ptr));
			WaveformMemory::getInstance()->UnlockMemMapPtr();
		}

		//get count of travel to first point in file
		BuildTravelToStart(powerIdle, positionVxy, _countTotal[_scanAreaId][SignalType::ANALOG_XY][0]);
		SAFE_DELETE_ARRAY(powerIdle);

		//same length for all signal types
		_countTotal[_scanAreaId][SignalType::DIGITAL_LINES][0] = _countTotal[_scanAreaId][SignalType::ANALOG_POCKEL][0] = _countTotal[_scanAreaId][SignalType::ANALOG_XY][0];
	}

	//set counters: all signal types should be same length but XY could be empty
	for (int i = 0; i < static_cast<int>(SignalType::SIGNALTYPE_LAST); i++)
	{
		ret = GetTotalCount(countPerCallback[i], (SignalType)i, _countTotal[_scanAreaId][SignalType::ANALOG_XY][0], _countTotal[_scanAreaId][SignalType::ANALOG_XY][1]);
		if(FALSE == ret && SignalType::ANALOG_XY != i)
		{
			ReleaseMutex(_gWaveXY[_scanAreaId].bufferHandle);
			return FALSE;
		}
	}

	//digital line selection:
	_digitalLineSelection = digLineSelection;
	_gWaveXY[_scanAreaId].digitalLineCnt = 0;
	for (int i = 0; i < BLEACHSCAN_DIGITAL_LINENAME::DIGITAL_LINENAME_LAST; i++)
	{
		_gWaveXY[_scanAreaId].digitalLineCnt += (0 < (_digitalLineSelection & (0x1 << i))) ? 1 : 0;
	}

	//done:
	ResetCounter();
	ReleaseMutex(_gWaveXY[_scanAreaId].bufferHandle);	
	LogPerformance(L"ImageWaveformBuilder Reset time");
	return ret;
}


// build waveform including travel to start position from currentVxy
uint64_t ImageWaveformBuilder::RebuildThorDAQWaveformFromFile(const wchar_t* waveformFileName, unsigned short * currentVxy, int digLineSelection, long* countPerCallback)
{
	uint64_t ret = 0;	
	_scanAreaId = 0;	//use 1st area id, to be extended in the future

	//wait for mutex:
	if(FALSE == GetMutex(_gThorDAQWaveXY[_scanAreaId].bufferHandle))
		return FALSE;

	//open mem map:
	if (FALSE == WaveformMemory::getInstance()->OpenMemThorDAQ(_gThorDAQWaveXY[_scanAreaId], waveformFileName))
	{
		ReleaseMutex(_gThorDAQWaveXY[_scanAreaId].bufferHandle);
		return FALSE;
	}

	//set file unit size:
	for (int i = 0; i < static_cast<int>(SignalType::SIGNALTYPE_LAST); i++)
	{
		_countTotal[_scanAreaId][i][0] = _countTotal[_scanAreaId][i][2] = 0;
		_countTotal[_scanAreaId][i][1] = _gThorDAQWaveXY[_scanAreaId].analogPockelSize;		//unit size is the same as pockel size in file loading bleach scan mode
	}
	_waveformFileName = waveformFileName;

	//set counters: all signal types should be same length
	for (int i = 0; i < static_cast<int>(SignalType::SIGNALTYPE_LAST); i++)
	{
		ret = GetTotalCount(countPerCallback[i], (SignalType)i, _countTotal[_scanAreaId][SignalType::ANALOG_XY][0], _countTotal[_scanAreaId][SignalType::ANALOG_XY][1]);
		if(FALSE == ret && SignalType::ANALOG_XY != i)
		{
			ReleaseMutex(_gThorDAQWaveXY[_scanAreaId].bufferHandle);
			return FALSE;
		}
	}

	//digital line selection:
	_digitalLineSelection = digLineSelection;
	_gThorDAQWaveXY[_scanAreaId].digitalLineCnt = 0;
	for (int i = 0; i < BLEACHSCAN_DIGITAL_LINENAME::DIGITAL_LINENAME_LAST; i++)
	{
		_gThorDAQWaveXY[_scanAreaId].digitalLineCnt += (0 < (_digitalLineSelection & (0x1 << i))) ? 1 : 0;
	}

	//done:
	ResetCounter();
	ReleaseMutex(_gThorDAQWaveXY[_scanAreaId].bufferHandle);	
	LogPerformance(L"ImageWaveformBuilder Reset time");
	return ret;
}

// close waveform memory map file
void ImageWaveformBuilder::CloseWaveformFile()
{	
	WaveformMemory::getInstance()->CloseMem();
}


///	***************************************** <summary> General Private Functions </summary>	********************************************** ///

unsigned char ImageWaveformBuilder::GetPockelsCount()
{
	unsigned char pockelsCnt = 0;
	for(int poc=0; poc<MAX_GG_POCKELS_CELL_COUNT; poc++)
	{
		if(TRUE == _wParams.pockelsLineEnable[poc])
			pockelsCnt++;
	}
	return pockelsCnt;
}

uint64_t ImageWaveformBuilder::GetTotalCount(long& countPerCallback, SignalType sType, uint64_t stage1, uint64_t stage2)
{
	_countTotal[_scanAreaId][sType][0] = stage1;
	_countTotal[_scanAreaId][sType][1] = stage2;
	//check if valid
	if(0 == _countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1])
	{
		_countTotal[_scanAreaId][sType][2] = 0;
		return 0;
	}

	//determine callback length by galvo, no update if overflow
	_countPerCallback[sType] = (0 > static_cast<long>(_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1])) ? countPerCallback : 
		std::min(static_cast<long>(_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1]), countPerCallback);

	//return if invalid
	if(0 == _countPerCallback[sType])
		return 0;

	//update user value if smaller
	countPerCallback = std::min(countPerCallback, static_cast<long>(_countPerCallback[sType]));

	//calculate idle step length if not multiple of callback length
	_countTotal[_scanAreaId][sType][2] = (0 == ((_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1]) % _countPerCallback[sType])) ? 
		0 :	(_countPerCallback[sType] - ((_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1]) % _countPerCallback[sType]));

	//return maximum limited value
	return (ULLONG_MAX < (_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1] + _countTotal[_scanAreaId][sType][2])) ? ULLONG_MAX : 
		(_countTotal[_scanAreaId][sType][0] + _countTotal[_scanAreaId][sType][1] + _countTotal[_scanAreaId][sType][2]);
}

void ImageWaveformBuilder::InitializeParams()
{
	_lineFlybackLength = 0;
	_frameTime = 0;

	//scan area index, prepare for multi-scan area
	_scanAreaId = 0;

	//initialize _wParams:
	_wParams.areaMode = 0;
	_wParams.fieldScaleFineX = _wParams.fieldScaleFineY = 0;
	_wParams.fieldSize = 0;
	_wParams.PixelX = _wParams.PixelY = 0;
	_wParams.offsetX = _wParams.offsetY = 0;
	_wParams.fineOffset[0] = _wParams.fineOffset[1] = 0;
	_wParams.numFrame = 0;
	_wParams.scaleYScan = 0;
	_wParams.flybackCycles = 0;
	_wParams.minLowPoints = 0;
	_wParams.clockRatePockels = 0;
	_wParams.digLineSelect = 0;
	_wParams.pockelsTurnAroundBlank = 0;
	_wParams.scanMode = ScanMode::TWO_WAY_SCAN;
	_wParams.scanAreaAngle = 0;
	_wParams.scanAreaIndex = 0;
	_wParams.galvoEnable = 0;
	_wParams.useReferenceForPockelsOutput = 0;
	_wParams.pockelsReferenceRequirementsMet = 0;
	_wParams.verticalScanDirection = 1;
	_wParams.yAmplitudeScaler = 0;
	_wParams.field2Volts = 0;
	_wParams.dwellTime = 0;

	for(long i=0; i<MAX_GG_POCKELS_CELL_COUNT; i++)
	{
		_wParams.pockelsLineEnable[i] = 0;
		_wParams.pockelsPower[i] = 0;
		_wParams.pockelsIdlePower[i] = 0;
		_wParams.pockelsMaxPower[i] = 0;
		_wParams.pockelsLineBlankingPercentage[i] = 0;
	}
	_lineSegs.clear();

	//initialize GGalvoParams:
	for (int i = 0; i < MAX_MULTI_AREA_SCAN_COUNT; i++)
	{
		_gParams[i].GalvoWaveformPockel = NULL;
		_gParams[i].GalvoWaveformXY = NULL;
		_gParams[i].DigBufWaveform = NULL;
		_gParams[i].bufferHandle = NULL;

		_gWaveXY[i].GalvoWaveformPockel = NULL;
		_gWaveXY[i].GalvoWaveformXY = NULL;
		_gWaveXY[i].DigBufWaveform = NULL;
		_gWaveXY[i].bufferHandle = NULL;
	}

	_pGalvoWaveformX = NULL;
	_pGalvoWaveformY = NULL;
}

//reset GGalvoWaveformParam to null or with certain unit size
long ImageWaveformBuilder::ResetGGalvoWaveformParam(GGalvoWaveformParams * params, long* unitSize, long numPockels, long digitalLineCnt)
{
	params->ClockRate = params->CycleNum = params->Triggermode = 0;
	params->PreCapStatus = PreCaptureStatus::PRECAPTURE_BLEACHER_IDLE;
	for (int i = 0; i < static_cast<int>(SignalType::SIGNALTYPE_LAST); i++)
	{
		params->unitSize[i] = unitSize[i];
	}
	params->digitalLineCnt = digitalLineCnt;

	//wait for mutex:
	if(TRUE == GetMutex(params->bufferHandle))
	{
		//analog lines:
		params->analogXYSize = std::max(2 * params->unitSize[SignalType::ANALOG_XY], (unsigned long long)0);	//x,y
		if(0 < params->analogXYSize)
		{
			params->GalvoWaveformXY = (double*)realloc(params->GalvoWaveformXY, params->analogXYSize * sizeof(double));
			if (NULL == params->GalvoWaveformXY)
			{
				ReleaseMutex(params->bufferHandle);
				return FALSE;
			}
		}
		else
		{
			if(NULL != params->GalvoWaveformXY)
			{
				free(params->GalvoWaveformXY);
				params->GalvoWaveformXY = NULL;
			}
		}

		//pockels lines:
		params->analogPockelSize = std::max(numPockels * params->unitSize[SignalType::ANALOG_POCKEL], (unsigned long long)0);
		if(0 < params->analogPockelSize)
		{
			params->GalvoWaveformPockel = (double*)realloc(params->GalvoWaveformPockel, params->analogPockelSize * sizeof(double));
			if (NULL == params->GalvoWaveformPockel)
			{
				ReleaseMutex(params->bufferHandle);
				return FALSE;
			}
		}
		else
		{
			if(NULL != params->GalvoWaveformPockel)
			{
				free(params->GalvoWaveformPockel);
				params->GalvoWaveformPockel = NULL;
			}
		}
		//dig lines:
		params->digitalSize = std::max(digitalLineCnt * params->unitSize[SignalType::DIGITAL_LINES], (unsigned long long)0);		//[BleachScan]: pockels digital with complete, cycle, iteration, pattern, patternComplete lines
		if(0 < params->digitalSize)
		{
			params->DigBufWaveform = (unsigned char*)realloc(params->DigBufWaveform, params->digitalSize * sizeof(unsigned char));
			if (NULL == params->DigBufWaveform)
			{
				ReleaseMutex(params->bufferHandle);
				return FALSE;
			}
		}
		else
		{
			if(NULL != params->DigBufWaveform)
			{
				free(params->DigBufWaveform);
				params->DigBufWaveform = NULL;
			}
		}
		//z piezo:
		params->analogZSize = std::max(params->unitSize[SignalType::ANALOG_Z], (unsigned long long)0);	//z
		if (0 < params->analogZSize)
		{
			params->PiezoWaveformZ = (double*)realloc(params->PiezoWaveformZ, params->analogZSize * sizeof(double));
			if (NULL == params->PiezoWaveformZ)
			{
				ReleaseMutex(params->bufferHandle);
				return FALSE;
			}
		}
		else
		{
			if (NULL != params->PiezoWaveformZ)
			{
				free(params->PiezoWaveformZ);
				params->PiezoWaveformZ = NULL;
			}
		}

		//single mutex:
		if ((0 == params->analogXYSize) && (0 == params->analogPockelSize) && (0 == params->digitalSize)) // && (0 == params->analogZSize)) - not include Z yet
		{
			if(NULL != params->bufferHandle)
			{
				CloseHandle(params->bufferHandle);
				params->bufferHandle = NULL;
			}
		}
		else
		{
			if(NULL == params->bufferHandle)
			{
				params->bufferHandle = CreateMutex(NULL, false, NULL);
			}
		}

		ReleaseMutex(params->bufferHandle);
	}
	return TRUE;
}

//reset GGalvoWaveformParam to null or with certain unit size
long ImageWaveformBuilder::ResetThorDAQGGWaveformParam(ThorDAQGGWaveformParams * params, long* unitSize, long numPockels, long digitalLineCnt)
{
	params->ClockRate = params->CycleNum = params->Triggermode = 0;
	params->PreCapStatus = PreCaptureStatus::PRECAPTURE_BLEACHER_IDLE;
	for (int i = 0; i < static_cast<int>(SignalType::SIGNALTYPE_LAST); i++)
	{
		params->unitSize[i] = unitSize[i];

	}
	params->digitalLineCnt = digitalLineCnt;

	//wait for mutex:
	if(TRUE == GetMutex(params->bufferHandle))
	{
		//analog lines:
		params->analogXYSize = std::max(params->unitSize[SignalType::ANALOG_XY], (unsigned long long)0);	//x,y
		if(0 < params->analogXYSize)
		{
			params->GalvoWaveformX = (unsigned short*)realloc(params->GalvoWaveformX, params->analogXYSize * sizeof(unsigned short));
			if (NULL == params->GalvoWaveformX)
			{
				ReleaseMutex(params->bufferHandle);
				return FALSE;
			}

			params->GalvoWaveformY = (unsigned short*)realloc(params->GalvoWaveformY, params->analogXYSize * sizeof(unsigned short));
			if (NULL == params->GalvoWaveformY)
			{
				ReleaseMutex(params->bufferHandle);
				return FALSE;
			}		
		}
		else
		{
			if(NULL != params->GalvoWaveformX)
			{
				free(params->GalvoWaveformX);
				params->GalvoWaveformX = NULL;
			}

			if (NULL != params->GalvoWaveformY)
			{
				free(params->GalvoWaveformY);
				params->GalvoWaveformY = NULL;
			}
		}

		//pockels lines:
		params->analogPockelSize = std::max(numPockels * params->unitSize[SignalType::ANALOG_POCKEL], (unsigned long long)0);
		if(0 < params->analogPockelSize)
		{
			params->GalvoWaveformPockel = (unsigned short*)realloc(params->GalvoWaveformPockel, params->analogPockelSize * sizeof(unsigned short));
			if (NULL == params->GalvoWaveformPockel)
			{
				ReleaseMutex(params->bufferHandle);
				return FALSE;
			}
		}
		else
		{
			if(NULL != params->GalvoWaveformPockel)
			{
				free(params->GalvoWaveformPockel);
				params->GalvoWaveformPockel = NULL;
			}
		}
		//dig lines:
		params->digitalSize = std::max(params->unitSize[SignalType::DIGITAL_LINES], (unsigned long long)0);		//[BleachScan]: pockels digital with complete, cycle, iteration, pattern, patternComplete lines
		if(0 < params->digitalSize)
		{
			params->DigBufWaveform = (unsigned short*)realloc(params->DigBufWaveform, params->digitalSize * sizeof(unsigned short));
			if (NULL == params->DigBufWaveform)
			{
				ReleaseMutex(params->bufferHandle);
				return FALSE;
			}
		}
		else
		{
			if(NULL != params->DigBufWaveform)
			{
				free(params->DigBufWaveform);
				params->DigBufWaveform = NULL;
			}
		}

		//single mutex:
		if((0 == params->analogXYSize) && (0 == params->analogPockelSize) && (0 == params->digitalSize))
		{
			if(NULL != params->bufferHandle)
			{
				CloseHandle(params->bufferHandle);
				params->bufferHandle = NULL;
			}
		}
		else
		{
			if(NULL == params->bufferHandle)
			{
				params->bufferHandle = CreateMutex(NULL, false, NULL);
			}
		}

		ReleaseMutex(params->bufferHandle);
	}
	return TRUE;
}