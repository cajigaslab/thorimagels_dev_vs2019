#include "FrameTriggerWaveform.h"

long FrameTriggerWaveform::SetParameters(uint32_t samplesPerLine)
{
	_samplesPerLine = samplesPerLine;
	return TRUE;
}

long FrameTriggerWaveform::CreateStripeFrameTriggerWaveform(UInt8BufferPtr pWaveform, StripInfo * pCurrentStripe)
{
	if (pWaveform == NULL || pCurrentStripe == NULL)
		return FALSE;
	auto lowPowerPoints = pCurrentStripe->SkipSignal*_samplesPerLine;
	CreateConstantWaveform(pWaveform, lowPowerPoints, 0);
	pWaveform += lowPowerPoints;
	CreateConstantWaveform(pWaveform, pCurrentStripe->IncludeSignal*_samplesPerLine, 1);
	return TRUE;
}

void FrameTriggerWaveform::CreateConstantWaveform(UInt8BufferPtr data, long length, uint8_t value)
{
	for (int cycle = 0; cycle < length; cycle++)
	{
		data[cycle] = value;
	}
}
