#pragma once
#include "WaveformModel.h"

class FrameTriggerWaveform
{
public:
	long SetParameters(uint32_t samplesPerLine);
	long CreateStripeFrameTriggerWaveform(UInt8BufferPtr pWaveform, StripInfo* pCurrentStripe);
private:
	void CreateConstantWaveform(UInt8BufferPtr data, long length, uint8_t value);

	uint32_t _samplesPerLine;
};

