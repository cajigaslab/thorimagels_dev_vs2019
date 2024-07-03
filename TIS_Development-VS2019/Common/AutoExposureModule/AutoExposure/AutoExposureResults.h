#pragma once
class AutoExposureResults
{
public:

	AutoExposureResults()
		: TargetExposureTime_ms(0),
		TargetGain_dB(0),
		NumSaturatedPixels(0)
	{}

	AutoExposureResults(double targetExposureTime_ms, double targetGain_dB, int numSaturatedPixels)
		: TargetExposureTime_ms(targetExposureTime_ms),
		TargetGain_dB(targetGain_dB),
		NumSaturatedPixels(numSaturatedPixels)
	{}

	double TargetExposureTime_ms;

	double TargetGain_dB;

	int NumSaturatedPixels;

};

