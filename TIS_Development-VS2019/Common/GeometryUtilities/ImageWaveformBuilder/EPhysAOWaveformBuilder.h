#pragma once

#include "..\..\ImageWaveformBuilderDll.h"

#ifdef LOGGING_ENABLED
extern std::auto_ptr<LogDll> logDll;
#endif
extern wchar_t message[_MAX_PATH];

/// Other than Square or Rectangle, ImageWaveformBuilder will handle other shapes' imaging waveform:
/// Polyline, ...
class EPhysAOWaveformBuilder : IWaveformBuilder
{
private:

	static bool _instanceFlag;
	static std::unique_ptr<EPhysAOWaveformBuilder> _single;
	static uint64_t _countTotal[BUF_REGION_COUNT]; ///<[0]:offset idle region, [1]:waveform body, [2]: zero padding
	static uint64_t _countIndex; ///<index to reflect current location in waveform
	static long _repeatWaveform; ///<to determine if waveform should be repeated
	EPhysTriggerStruct _eStruct;///<data structure for building waveform
	static BlockRingBuffer* _bRingBuffer; ///<block ring buffers for active loading
	static CircularBuffer* _ephysBuffer[BUF_REGION_COUNT]; ///<[0]:offset idle region, [1]:body of one repeat waveform, [2]: zero padding

	static HANDLE _hInitialized; ///<Signals for waveform being pushed to buffer

	EPhysAOWaveformBuilder();

	void ResetWaveform();

public:
	~EPhysAOWaveformBuilder() { ResetWaveform(); _instanceFlag = false; }
	static EPhysAOWaveformBuilder* getInstance();

	static void BufferAvailableCallbackFunc(long sType, long bufSpaceNum);

	virtual void ConnectCallback(void* buf); ///<register to buffer available callback
	virtual void SetWaveformParams(void* params); ///<set ephys data stucture
	virtual long TryBuildWaveform(uint64_t& totalLength);///<rebuild waveform based on given ephys trigger data structures, return FALSE if failed
	virtual HANDLE GetSignalHandle() { return _hInitialized; }; ///<allow to check waveform being pushed to buffer or not
	void ResetCounter() { _countIndex = 0; } ///<reset counter index for potential re-execute of the same waveform

};
