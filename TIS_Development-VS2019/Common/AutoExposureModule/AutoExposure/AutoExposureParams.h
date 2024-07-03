#pragma once
class AutoExposureParams
{

public:

	int BitDepth;

	int ImageWidth;

	int ImageHeight;

	int RoiWidth_pixels;

	int RoiHeight_pixels;

	double ExposureTime_ms;

	double Gain_dB;

	double ExposureTimeMin_ms;

	double ExposureTimeMax_ms;

	double GainMin_dB;

	double GainMax_dB;

	bool IsGainAdjustable;

	double TargetPercent;

};

