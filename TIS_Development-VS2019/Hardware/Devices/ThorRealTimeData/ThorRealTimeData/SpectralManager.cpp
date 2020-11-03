#pragma once

#include "stdafx.h"
#include "strsafe.h"

std::auto_ptr<IPPSDll> ippsDll(new IPPSDll(L".\\.\\ipps-7.0.dll"));

///	***************************************** <summary> SpectralAnalyzer - generate frequency-domain channels based on time-domain data </summary>	********************************************** ///

void SpectralAnalyzer::AddTimeLine(std::string tName)
{
	std::map<std::string, CircularBuffer*>::iterator it = _timeDataMap.find(tName);
	//unique line name 
	if (it != _timeDataMap.end())
		return;

	//input buffer
	CircularBuffer* cBuf = new CircularBuffer(tName);
	cBuf->AllocMem(sizeof(double) * _timeLength);
	__hook(&CircularBuffer::BufferSizeReachedEvent, cBuf, &SpectralAnalyzer::BufferFullEventHandler);
	_timeDataMap.insert(std::pair<std::string, CircularBuffer*>(tName, cBuf));

	_specVec += ChannelCenter::getInstance()->YieldVariables(tName);

	//output buffer
	Ipp64fc* pBuf = NULL;
	pBuf = ippsDll->ippsMalloc_64fc(_freqLength);
	ippsDll->ippsZero_64fc(pBuf, _freqLength); 
	_outDFTMap.insert(std::pair<std::string, Ipp64fc*>(tName, pBuf));
}

long SpectralAnalyzer::InitialDefault()
{
	_freqLength = _timeLength / NYQUIST_RATIO;
	IppStatus status = ippsDll->ippsDFTInitAlloc_C_64fc(&_spec, _timeLength, IPP_FFT_DIV_BY_SQRTN, ippAlgHintAccurate);
	_inDFTRe = ippsDll->ippsMalloc_64f(_timeLength);
	_inDFTIm = ippsDll->ippsMalloc_64f(_timeLength);
	_inDFT = ippsDll->ippsMalloc_64fc(_timeLength);
	_outDFT = ippsDll->ippsMalloc_64fc(_timeLength);

	if ((0 != status) || (NULL == _inDFTRe) || (NULL == _inDFTIm) || (NULL == _inDFT) || (NULL == _outDFT)) 
		return FALSE;

	return TRUE;
}

void SpectralAnalyzer::Mag(std::string name, double* pTgt) 
{
	ippsDll->ippsMagnitude_64fc(_outDFTMap[name], pTgt, _freqLength);
}

void SpectralAnalyzer::Mag(double* pSrcRe, double* pSrcIm, double* pTgt, long length)
{
	ippsDll->ippsMagnitude_64f(pSrcRe, pSrcIm, pTgt, length);
}

void SpectralAnalyzer::Real(std::string name, double* pTgt) 
{
	ippsDll->ippsReal_64fc(_outDFTMap[name], pTgt, _freqLength);
}

void SpectralAnalyzer::Imag(std::string name, double* pTgt) 
{
	ippsDll->ippsImag_64fc(_outDFTMap[name], pTgt, _freqLength);
}

long SpectralAnalyzer::WriteSource(CompoundData* cpData)
{
	if (0 == _timeDataMap.size())
		return FALSE;

	std::map<std::string, CircularBuffer*>::iterator it = _timeDataMap.begin();
	double* pDInValue = NULL, *pDst = NULL;

	//return if buffer is already full before lock, 
	//check first only since all were written at the same time.
	if(it->second->IsFull())
		return FALSE;

	//lock before access buffers
	for (it = _timeDataMap.begin(); it != _timeDataMap.end(); it++) 
	{
		it->second->Lock();
	}

	size_t length = cpData->GetgcLengthValue();

	//process time line data
	for (std::size_t i = 0; i < _specVec.size(); i++)
	{
		switch ((SignalType)(_specVec[i].sType))
		{
		case SignalType::ANALOG_IN:
			if(NULL != cpData->GetStrucData()->aiDataPtr)
			{
				_timeDataMap[_specVec[i].name]->Write((char*)(cpData->GetStrucData()->aiDataPtr + _specVec[i].offset * length), sizeof(double) * length);
			}
			break;
		case SignalType::DIGITAL_IN:
			if(NULL != cpData->GetStrucData()->diDataPtr)
			{
				pDInValue = (double*)realloc(pDInValue, sizeof(double) * length);
				pDst = pDInValue;
				for (size_t j = 0; j < length; j++)
				{
					double tmp = *(cpData->GetStrucData()->diDataPtr + _specVec[i].offset * length + j);
					*pDst = tmp;
					pDst++;
				}
				_timeDataMap[_specVec[i].name]->Write((char*)pDInValue, sizeof(double) * length);
			}
			break;
		case SignalType::VIRTUAL:
			if(NULL != cpData->GetStrucData()->viDataPtr)
			{
				_timeDataMap[_specVec[i].name]->Write((char*)(cpData->GetStrucData()->viDataPtr + _specVec[i].offset * length), sizeof(double) * length);
			}
			break;
		}
	}

	SAFE_DELETE_MEMORY(pDInValue);

	//release access buffers
	for (it = _timeDataMap.begin(); it != _timeDataMap.end(); it++) 
	{
		it->second->ReleaseLock();
	}

	return 0;
}

void SpectralAnalyzer::BufferFullEventHandler()
{
	std::map<std::string, CircularBuffer*>::iterator it;
	double* tmpBuf = NULL;

	//wait until all circular buffers are ready to process
	for (it = _timeDataMap.begin(); it != _timeDataMap.end(); it++) 
	{
		if(FALSE == it->second->IsFull())
			return;
	}
	//return if user request stop
	if(ChannelCenter::getInstance()->_stopLoading)
	{
		ChannelCenter::getInstance()->_isLoading = FALSE;
		return;
	}

	//initialize buffer at beginning and configure params
	long averageNum = 1;
	long blockNum = 1;
	if(_useCenterSetting)	//at data review
	{
		if (0 < ChannelCenter::getInstance()->_specParam.freqAvgMode)
			averageNum = (1 <= ChannelCenter::getInstance()->_specParam.freqAvgNum) ? ChannelCenter::getInstance()->_specParam.freqAvgNum : 1;

		blockNum = (1 <= ChannelCenter::getInstance()->_specParam.blockNum) ? ChannelCenter::getInstance()->_specParam.blockNum : 1;
		if(1 < blockNum)
		{
			tmpBuf = (double*)realloc(tmpBuf, sizeof(double)*blockNum);
			memset(tmpBuf, 0, sizeof(double)*blockNum);
		}
	}
	long timeLengthPerAverage = static_cast<long>(_timeDataMap.begin()->second->Size() / sizeof(unsigned __int64)) / averageNum / blockNum;

	if(timeLengthPerAverage != _timeLength)
	{
		ReleaseDefault();
		_timeLength = timeLengthPerAverage;
		InitialDefault();
	}

	////lock before access buffers
	for (it = _timeDataMap.begin(); it != _timeDataMap.end(); it++) 
	{
		it->second->Lock();
	}

	//hold access to output buffers
	_saLock.Enter();
	auto t1 = Clock::now();

	//DFT on all available time-domain data
	for (it = _timeDataMap.begin(); it != _timeDataMap.end(); it++) 
	{
		for (int iAvg = 0; iAvg < averageNum; iAvg++)
		{
			//load time data considering block average
			if (1 < blockNum)
			{
				for (int i = 0; i < _timeLength; i++)
				{
					it->second->Read((char*)tmpBuf, sizeof(double) * blockNum, false);
					double val = 0;
					for (int k = 0; k < blockNum; k++)
					{
						val += tmpBuf[k]/blockNum;
					}
					_inDFTRe[i] = val;
				}
			}
			else
			{
				it->second->Read((char*)_inDFTRe, sizeof(double) * _timeLength, false);
			}

			ippsDll->ippsZero_64f(_inDFTIm, _timeLength); 
			ippsDll->ippsRealToCplx_64f(_inDFTRe, _inDFTIm, _inDFT, _timeLength);

			if (0 != ippsDll->ippsDFTFwd_CToC_64fc(_inDFT, _outDFT, _spec, NULL))
			{
				StringCbPrintfW(message,MSG_LENGTH,L"SpectralAnalyzer Error: failed to do DFT on %s.", it->first);
				LogMessage(message, ERROR_EVENT);
				SAFE_DELETE_MEMORY(tmpBuf);
				for (it = _timeDataMap.begin(); it != _timeDataMap.end(); it++) 
				{
					it->second->Clear();
					it->second->ReleaseLock();
				}
				_saLock.Leave();
				ChannelCenter::getInstance()->_isLoading = FALSE;
				return;
			}

			//map half (positive frequency) to output after average:
			if(1 < averageNum)
			{
				double* pSrc = (double*)_outDFT;
				double* pTgt = (double*)_outDFTMap[it->first];
				for (int fid = 0; fid < _timeLength; fid++)	//average on both real & imag parts, therefore using _timeLength
				{
					*pTgt += *pSrc / averageNum;
					pTgt++;
					pSrc++;
				}
			}
			else
			{
				SAFE_MEMCPY(_outDFTMap[it->first], sizeof(Ipp64fc) * _freqLength, _outDFT);
			}
			//break if user request stop
			if(ChannelCenter::getInstance()->_stopLoading)
				break;
		}
		if(ChannelCenter::getInstance()->_stopLoading)
			break;
	}
	auto t2 = Clock::now();

	//clear and release buffers
	SAFE_DELETE_MEMORY(tmpBuf);
	for (it = _timeDataMap.begin(); it != _timeDataMap.end(); it++) 
	{
		it->second->Clear();
		it->second->ReleaseLock();
	}
	//release access of output buffers
	_saLock.Leave();

	//notice the spectral analysis result is available
	if(ChannelCenter::getInstance()->_stopLoading)
	{
		ChannelCenter::getInstance()->_isLoading = FALSE;
	}
	else
	{
		DFTAvailable();
	}
#if _DEBUG
	milliseconds ms = duration_cast<milliseconds>(t2-t1);
	StringCbPrintfW(message,MSG_LENGTH,L"SpectralAnalyzer DFT %d lines for: %u ms", static_cast<int>(_timeDataMap.size()), static_cast<unsigned long>(ms.count()));
	LogMessage(message, VERBOSE_EVENT);
#endif
}

void SpectralAnalyzer::ReleaseData()
{
	//unhook events then release time data
	for (std::map<std::string, CircularBuffer*>::iterator it = _timeDataMap.begin(); it != _timeDataMap.end(); it++) 
	{
		__unhook(&CircularBuffer::BufferSizeReachedEvent, it->second, &SpectralAnalyzer::BufferFullEventHandler);
		it->second->ReleaseMem();
		it->second = NULL;
	}
	_timeDataMap.clear();
	_specVec.clear();

	//release output buffers
	std::map<std::string, Ipp64fc*>::iterator it;
	for (it = _outDFTMap.begin(); it != _outDFTMap.end(); it++) 
	{
		ippsDll->ippsFree(it->second);
	}
	_outDFTMap.clear();
};

void SpectralAnalyzer::ReleaseDefault()
{
	if(NULL != _spec)
	{
		ippsDll->ippsDFTFree_C_64fc(_spec);
		_spec = NULL;
	}
	if(NULL != _inDFTRe)
	{
		ippsDll->ippsFree(_inDFTRe);
		_inDFTRe = NULL;
	}
	if(NULL != _inDFTIm)
	{
		ippsDll->ippsFree(_inDFTIm);
		_inDFTIm = NULL;
	}
	if(NULL != _inDFT)
	{
		ippsDll->ippsFree(_inDFT);
		_inDFT = NULL;
	}
	_freqLength = 0;
};

void SpectralAnalyzer::ReleaseAll()
{ 
	ReleaseData(); 
	ReleaseDefault();
}

FreqCompoundData* SpectralAnalyzer::YieldFreqCompoundData()
{
	if(0 == _outDFTMap.size())
		return NULL;

	FreqCompoundData* fcData = new FreqCompoundData(_freqLength, _specVec.size()*_freqLength);
	std::map<std::string, Ipp64fc*>::iterator it;

	//set freq range
	ChannelCenter::getInstance()->_freqHz.clear();
	ChannelCenter::getInstance()->_freqHz.reserve(_freqLength);
	double df = (ChannelCenter::getInstance()->_isLive) ? (ChannelCenter::getInstance()->_mode.sampleRate / NYQUIST_RATIO / _freqLength) : (ChannelCenter::getInstance()->_mode.sampleRate / NYQUIST_RATIO / _freqLength / ChannelCenter::getInstance()->_specParam.blockNum);
	double fmin = (ChannelCenter::getInstance()->_otmParam.fitFreqMin < ChannelCenter::getInstance()->_otmParam.fitFreqMax) ? ChannelCenter::getInstance()->_otmParam.fitFreqMin : ChannelCenter::getInstance()->_otmParam.fitFreqMax;
	double fmax = (ChannelCenter::getInstance()->_otmParam.fitFreqMin < ChannelCenter::getInstance()->_otmParam.fitFreqMax) ? ChannelCenter::getInstance()->_otmParam.fitFreqMax : ChannelCenter::getInstance()->_otmParam.fitFreqMin;
	double beta2fmin = (ChannelCenter::getInstance()->_otmParam.beta2FreqMin < ChannelCenter::getInstance()->_otmParam.beta2FreqMax) ? ChannelCenter::getInstance()->_otmParam.beta2FreqMin : ChannelCenter::getInstance()->_otmParam.beta2FreqMax;
	double beta2fmax = (ChannelCenter::getInstance()->_otmParam.beta2FreqMin < ChannelCenter::getInstance()->_otmParam.beta2FreqMax) ? ChannelCenter::getInstance()->_otmParam.beta2FreqMax : ChannelCenter::getInstance()->_otmParam.beta2FreqMin;

	bool found[4] = {false, false, false, false};
	for (long i = 0; i < _freqLength; i++)
	{
		double fVal = (i + 1) * df;
		*(fcData->GetStrucData()->freqData + i) = fVal;
		ChannelCenter::getInstance()->_freqHz.emplace_back(fVal);

		//Range index [start,end), length = ene-start+1; (include start)
		if((fVal >= fmin) && !found[0])
		{
			ChannelCenter::getInstance()->_freqRangeIdx[0] = i;
			found[0] = true;
		}
		if((fVal >= fmax) && !found[1])
		{
			ChannelCenter::getInstance()->_freqRangeIdx[1] = i;
			found[1] = true;
		}
		if((fVal >= beta2fmin) && !found[2])
		{
			ChannelCenter::getInstance()->_freqRangeIdx[2] = i;
			found[2] = true;
		}
		if((fVal >= beta2fmax) && !found[3])
		{
			ChannelCenter::getInstance()->_freqRangeIdx[3] = i;
			found[3] = true;
		}
	}
	//return if curve fitting range not found 
	if(!found[0] || !found[1])
	{
		delete fcData;
		return NULL;
	}
	//skip beta2 fitting if not found
	if(!found[2] || !found[3])
	{
		ChannelCenter::getInstance()->_freqRangeIdx[2] = ChannelCenter::getInstance()->_freqRangeIdx[3] = ChannelCenter::getInstance()->_freqRangeIdx[0];
	}
	//set freq fit range for plot if fitting, consider freq. block
	//retain freqHz and freqRangeIdx as if freq. block == 1
	if(!ChannelCenter::getInstance()->_isLive && ChannelCenter::getInstance()->_otmParam.isCurveFit)
	{
		long freqFitLength = ChannelCenter::getInstance()->_freqRangeIdx[1] - ChannelCenter::getInstance()->_freqRangeIdx[0] + 1;
		freqFitLength = (0 < ChannelCenter::getInstance()->_otmParam.freqBlock) ? (freqFitLength / ChannelCenter::getInstance()->_otmParam.freqBlock) : freqFitLength;
		if(0 < freqFitLength)
		{
			fcData->SetFreqFitData(freqFitLength);
			double* pTgt = fcData->GetStrucData()->freqFitData;
			long fblockId = 1;			//freq. block index
			double fblockSumVal = 0.0;	//sum freq value: sum(f1,...,fN)
			long offset = (ChannelCenter::getInstance()->_freqRangeIdx[1] + 1 - ChannelCenter::getInstance()->_freqRangeIdx[0]) % ChannelCenter::getInstance()->_otmParam.freqBlock;
			for (long i = ChannelCenter::getInstance()->_freqRangeIdx[0] + offset; i <= ChannelCenter::getInstance()->_freqRangeIdx[1]; i++, fblockId++)
			{
				fblockSumVal += ChannelCenter::getInstance()->_freqHz.at(i);
				if(fblockId >= ChannelCenter::getInstance()->_otmParam.freqBlock)
				{
					//average freq value: sum(f1,...,fN)/N
					*(pTgt) = (0 < ChannelCenter::getInstance()->_otmParam.freqBlock) ? (fblockSumVal / ChannelCenter::getInstance()->_otmParam.freqBlock) : fblockSumVal;
					pTgt++;
					//reset freq. block index & sum value for next block
					fblockId = 0;
					fblockSumVal = 0.0;
				}
			}
		}
	}

	//hold access to output buffers
	_saLock.Enter();

	//populate spectral data following spectral vector order to align with display plot
	for (it = _outDFTMap.begin(); it != _outDFTMap.end(); it++) 
	{
		auto iv = std::find(_specVec.begin(), _specVec.end(), VirtualVariable(std::string(it->first)));
		if (_specVec.end() != iv)
		{
			int64_t id = std::distance(_specVec.begin(), iv);
			Real(it->first, (fcData->GetStrucData()->specDataRe + id * _freqLength));
			Imag(it->first, (fcData->GetStrucData()->specDataIm + id * _freqLength));
		}
	}

	//release access to output buffers
	_saLock.Leave();

	return fcData;
}

///	***************************************** <summary> SpectralManager - process spectral compound data for file saving and display </summary>	********************************************** ///

SpectralManager::SpectralManager() 
{
	_saveFile = FALSE;
	_specAnalyzer = NULL;
}

SpectralManager::~SpectralManager() 
{
	Release();
}

//instance flag must initialize after constructor
bool SpectralManager::_instanceFlag = false;
std::unique_ptr<SpectralManager> SpectralManager::_single = NULL;

SpectralManager* SpectralManager::getInstance()
{ 
	if(! _instanceFlag)
	{ 
		_single.reset(new SpectralManager()); 
		_instanceFlag = true;
	} 
	return _single.get(); 
}

void SpectralManager::UpdateSpectralAnalyzer(long timeLength)
{
	Release();

	if(0 == ChannelCenter::getInstance()->_specChannel.size())
		return;

	_specAnalyzer = (0 == timeLength) ? new SpectralAnalyzer() : new SpectralAnalyzer(timeLength);

	__hook(&SpectralAnalyzer::DFTAvailable, _specAnalyzer, &SpectralManager::DFTAvailableEventHandler);

	for (size_t i = 0; i < ChannelCenter::getInstance()->_specChannel.size(); i++)
	{
		if(0 == ChannelCenter::getInstance()->_specChannel[i].type.compare("/FI"))
		{
			_specAnalyzer->AddTimeLine(ChannelCenter::getInstance()->_specChannel[i].lineId);
		}
	}

	UpdateSpectralVirtualProcessor();
}

void SpectralManager::UpdateSpectralVirtualProcessor()
{
	_virtualFreqChanManager->getInstance()->UpdateProcessor();
}

void SpectralManager::DFTAvailableEventHandler()
{
	//get spectral with real & imag:
	FreqCompoundData* fData = _specAnalyzer->YieldFreqCompoundData();
	if(NULL == fData)
	{
		//invoke GUI for error notice
		(*functionPointer)(NULL);
		ChannelCenter::getInstance()->_isLoading = FALSE;
		return;
	}

	if(!ChannelCenter::getInstance()->_stopLoading)
	{
		//process virtual channels, including special expressions:
		int nVI = static_cast<int>(ChannelCenter::getInstance()->_enabledVF.size());
		fData->SetVirtualFreqData(nVI * fData->GetfreqSizeValue());
		_virtualFreqChanManager->getInstance()->Execute(fData);

		//output power spectrum at real part:
		SpectralAnalyzer::Mag(fData->GetStrucData()->specDataRe, fData->GetStrucData()->specDataIm, fData->GetStrucData()->specDataRe, static_cast<long>(fData->GetStrucData()->specDataLength));

		//update OTM settings while fitting:
		ChannelCenter::getInstance()->UpdateOTM();
	}

	//notice display buffer:
	if(!ChannelCenter::getInstance()->_stopLoading)
		(*functionPointer)(fData->GetStrucData());

	//save to file:
	if((!ChannelCenter::getInstance()->_stopLoading) && (_saveFile))
		ChannelCenter::getInstance()->SaveSpectral(fData->GetStrucData());

	//done:
	delete fData;
	fData = NULL;
	ChannelCenter::getInstance()->_isLoading = FALSE;
}

void SpectralManager::Release()
{
	if(NULL != _specAnalyzer)
	{
		_specAnalyzer->ReleaseAll();
		__unhook(&SpectralAnalyzer::DFTAvailable, _specAnalyzer, &SpectralManager::DFTAvailableEventHandler);
		delete _specAnalyzer;
		_specAnalyzer = NULL;
	}
}

long SpectralManager::WriteSource(CompoundData* cpData)
{
	if(_specAnalyzer)
	{
		return _specAnalyzer->WriteSource(cpData);
	}
	return FALSE;
}
