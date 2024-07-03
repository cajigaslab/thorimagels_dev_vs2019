#pragma once
#include "Types.h"
#include "WaveformManagerBase.h"

#include "DAQController.h"

#include "GalvoXWaveForm.h"
#include "GalvoYWaveForm.h"
#include "PokelsCellWaveForm.h"
#include "VoiceCoilWaveform.h"
#include "FrameTriggerWaveform.h"

#define USE_WAVEFORM_GENERATOR
#define WAVEFORM_MAX_BUFFER_LENGTH	(3600000)

enum WaveformChannels
{
	GALVO_X1 = 0,
	GALVO_X2 = 1,
	GALVO_Y = 2,
	VOICECOIL = 3,
	POCKELSCELL = 4,
	FRAMETRIGGER = 5,
	LAST_WAVEFORM_CHANNEL
};

class MesoWaveformManager :WaveformManagerBase
{
public:
	static MesoWaveformManager* GetInstance();
	~MesoWaveformManager();

	long CreateMoveToPositionWaveform(WaveformChannels channel, double* data, long length, double oldPositionValue, double newPositionValue);
private:
	static unique_ptr<MesoWaveformManager> _pInstance;
	static std::once_flag _onceFlag;

	GalvoXWaveForm _wfGalvoX;
	GalvoYWaveForm _wfGalvoY;
	PokelsCellWaveForm _wfPokels;
	VoiceCoilWaveForm _wfVoice;
	FrameTriggerWaveform _wfFrameTrigger;


	long _offsetX1;
	long _offsetX2;
	long _offsetY;
	long _offsetP;
	long _offsetV;
	long _offsetFrameTrigger;
	long _maxOffset;

	long _maxDelayForEntryLines;

	MesoWaveformManager();
	long SetupCircularBuffer(long id = -1);
	long SetParams(WaveformParams* waveformParams, long bufferSize);
	long ResetScanParams(Scan* scan, bool onlyResetUnusedMask);
	long ResetAllParams();
	long SetDelays(unsigned long X1Delay, unsigned long X2Delay, unsigned long YDelay, unsigned long VDelay, unsigned long PDelay);
	long SetCurrentPosition(double x1Pos, double x2Pos, double yPos, double vPos);
	long IsBufferReady(StripInfo* stripe);
	long GenerateStripWaveform(StripInfo* stripInfo);
	long GenerateStripSkipInfo(StripInfo* stripInfo, bool isNeedGeneratePowerMask = true);


	long GenerateStripWaveformX1(StripInfo* stripInfo);
	long GenerateStripWaveformX2(StripInfo* stripInfo);
	long GenerateStripWaveformY(StripInfo* stripInfo);
	long GenerateStripWaveformVoceCoil(StripInfo* stripInfo);
	long GenerateStripWaveformPockel(StripInfo* stripInfo);
	long GenerateStripWaveformFrameTrigger(StripInfo* stripInfo);
	long GenerateStripList(Scan* scanInCache, StripInfo ** first, StripInfo ** end, unsigned int TCount, DMABufferInfo* dmaInfo);
	double GetStripTime(StripInfo* strip);
	long GetTotalScanTime(Scan scans[], uint8_t scanSize, CameraConfig* cameraConfig);

	//***	additional members & functions	***//
	double _totalTime;

	double GetTotalScanTime() {return _totalTime; }
	void TeardownCircularBuffers();
	long DefaultWaveformParams(CameraConfig* camConfig);

};
