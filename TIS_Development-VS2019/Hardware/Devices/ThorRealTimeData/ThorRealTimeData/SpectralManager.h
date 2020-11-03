#pragma once

#include <stdio.h>
#include <map>
#include <string>
#include "..\..\..\..\Common\CircularBuffer.h"
#include "PublicType.h"

//**************************************************************************//
//*** class to generate spectral channels based on time domain data.   	 ***//
//**************************************************************************//

[event_receiver(native), event_source(native)]
class SpectralAnalyzer
{
private:
	long				_useCenterSetting;
	long				_timeLength;	//number of time samples 
	long				_freqLength;	//number of frequency samples, will be half of time sample length (half bandwidth)
	BufferLock			_saLock;
	IppsDFTSpec_C_64fc*	_spec;
	Ipp64f*				_inDFTRe;		//hold time domain data for DFT, Real part
	Ipp64f*				_inDFTIm;		//hold time domain data for DFT, Imag part
	Ipp64fc*			_inDFT;		
	Ipp64fc*			_outDFT;
	std::map<std::string, CircularBuffer*> _timeDataMap;	//time domain data resorvoir, convert all to double data type

	std::vector<VirtualVariable> _specVec;					//vector to hold offset values in acquired compound data

	std::map<std::string, Ipp64fc*>	_outDFTMap;				//hold spectral result, Real and Imag part of half bandwidth

	//initialize default memory
	long InitialDefault();

	//get DFT buffer Magnitude
	void Mag(std::string name, double* pTgt);

	//get DFT buffer Real part
	void Real(std::string name, double* pTgt);

	//get DFT buffer Imag part
	void Imag(std::string name, double* pTgt);

	//release output memory
	void ReleaseData();

	//release local memory
	void ReleaseDefault();

public:
	__event void DFTAvailable();

	//invoked at capturing data
	SpectralAnalyzer()
		: _spec(NULL)
		, _inDFTRe(NULL)
		, _inDFTIm(NULL)
		, _inDFT(NULL)
	{
		_timeLength = static_cast<long>(ChannelCenter::getInstance()->_mode.sampleRate * ChannelCenter::getInstance()->_specParam.liveSampleSec / NYQUIST_RATIO);
		_useCenterSetting = FALSE;
		InitialDefault();
	};

	//invoked at reviewing data
	SpectralAnalyzer(long timeLength)
		: _spec(NULL)
		, _inDFTRe(NULL)
		, _inDFTIm(NULL)
		, _inDFT(NULL)
	{
		_useCenterSetting = TRUE;
		_timeLength = timeLength;
		InitialDefault();
	};

	~SpectralAnalyzer()
	{
		ReleaseDefault();
	};

	//add time data with physical line name
	void AddTimeLine(std::string tName);

	void Enter() { _saLock.Enter(); }

	BOOL TryEnter() { return _saLock.TryEnter(); }

	void Leave() { _saLock.Leave(); }

	//get DFT buffer Magnitude, general available
	static void Mag(double* pSrcRe, double* pSrcIm, double* pTgt, long length);

	//generate frequency compound data based on current output map
	FreqCompoundData* YieldFreqCompoundData();

	//write time domain buffer in circular mode, will do DFT when full
	long WriteSource(CompoundData* cpData);

	//buffer event handler
	void BufferFullEventHandler();

	//release all memory
	void ReleaseAll();
};

//**************************************************************************//
//*** class to manage spectral channels, including virtual channels.   	 ***//
//**************************************************************************//

[event_receiver(native)]
class SpectralManager
{
private:
	static bool _instanceFlag;
	static std::unique_ptr<SpectralManager> _single;

	BOOL _saveFile;
	std::vector<VirtualChannelProcessor> _processors;
	std::unique_ptr<VirtualFreqChannelManager> _virtualFreqChanManager;
	SpectralAnalyzer* _specAnalyzer;

	SpectralManager();

	//handle spectral virtual channels by processor
	void UpdateSpectralVirtualProcessor();

public:
	static SpectralManager* getInstance();

	~SpectralManager();

	//assign single spectral analyzer for all DFT lines
	void UpdateSpectralAnalyzer(long dataLength = 0);

	//callback function when DFT is finished
	void DFTAvailableEventHandler();

	//write time domain data to circular buffer for later DFT
	long WriteSource(CompoundData* cpData);

	void SetSaveFile(BOOL saveFile) { _saveFile = saveFile; }

	void Release();
};