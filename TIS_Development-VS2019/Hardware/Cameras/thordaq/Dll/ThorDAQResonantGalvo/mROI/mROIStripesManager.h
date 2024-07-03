#pragma once
#include "mROIExperimentLoader.h"
#include <mutex>
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

class mROIStripesManager
{
public:
	static mROIStripesManager* GetInstance();
	~mROIStripesManager();

	long GenerateStripList(Scan* scanInCache, vector<StripInfo*>& strinpList);
private:
	static unique_ptr<mROIStripesManager> _pInstance;
	static std::once_flag _onceFlag;

	mROIStripesManager();

};
