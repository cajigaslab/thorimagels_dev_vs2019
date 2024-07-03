#pragma once
#define _USE_MATH_DEFINES

#include "WaveformModel.h"
#include "CommonWaveform.h"

class GalvoYWaveForm
{
public:
	GalvoYWaveForm();
	~GalvoYWaveForm();
	long SetParameters(double f2TGy, unsigned int samplesPerLine, double fieldHeight, double zeroPointVoltGY,
		unsigned int extendLinesStart, unsigned int extendLinesEnd);
	long SetFlyBackParams(double maxVelocity, double maxOvershoot, double pointsPerSecond);
	long SetCurrentPosition(double currentYPos);
    void CreateCurrentPositionWaveform(BufferPtr data, long length);
	long CreateStripeGalvoYWaveform(BufferPtr data, StripInfo* pCurrentStripe);
	long GetSkipLines(StripInfo* current_strip, SkipLines* pSkipLines);
	void MoveToPosition(double* data, long length, double oldPositionValue, double newPositionValue);
protected:
//********** General parameters **********
	unsigned int _samplesPerLine;
	//********** Start position **********
	double _startYPos;	//start position

	double GalvoYFieldToVolts(double size) { return size * _f2TGy + _zeroPointVoltGY; }
private:
//********** General parameters **********
	double _f2TGy;	//field to voltage
	double _zeroPointVoltGY;
//********** Flyback line **********
	//sine curve parameters of flyback line
	double _paramAmp;	//amplitude of sine curve
	double _paramFreq;	//frequency of sine curve
	double* _sineArr;
	unsigned int _extendLinesEnd;	//extend the Galvo Y waveform linearly before scanning stripe
	unsigned int _extendLinesStart;	//extend the Galvo Y waveform linearly after scanning stripe

	long CheckParameters(double f2TGy, unsigned int samplesPerLine, double fieldHeight, double zeroPointVoltGY);
	double GalvoYFieldToVoltsOffset(double size) { return size * _f2TGy; }
	inline void CreateFlyBackWaveform(BufferPtr data, long length, double preValue, double nextValue, double preDelta, double nextDelta);
};
