#pragma once
#define _USE_MATH_DEFINES

#include "WaveformModel.h"
#include "CommonWaveform.h"

class VoiceCoilWaveForm
{
public:
	VoiceCoilWaveForm();
	~VoiceCoilWaveForm();
	long SetParameters(unsigned int samplesPerLine, double fieldWidth, double fieldHeight, double voicecoilZToVolts,
		unsigned int extendLinesStart, unsigned int extendLinesEnd, double zeroPointVolt, double maxPointVolt, unsigned int pointsPerLine);
	long SetFlyBackParams(double maxVelocity, double maxOvershoot, double pointsPerSecond);
	long SetCurveParameters(double paramA, double paramB, double centerShiftX, double centerShiftY);
	long SetCurrentPosition(double currentVPos);
	void CreateCurrentPositionWaveform(BufferPtr data, long length);
	long CreateStripeVoiceCoilWaveform(BufferPtr data, StripInfo* pCurrentStripe);
	long GetSkipLines(StripInfo* pCurrentStripe, SkipLines* pSkipLines);
	void MoveToPosition(double* data, long length, double oldPositionValue, double newPositionValue);
protected:
//********** Voice coil curve parameters **********
// Z = A*(X-shiftX)^2 + B*(Y-shiftY)^2
	double _paramA;
	double _paramB;
//********** General parameters **********
	unsigned int _samplesPerLine;
	double _halfFieldWidth;
	double _halfFieldHeight;
	unsigned int _stepsPerLine;
//********** Start position **********
	double _startVPos;

	inline double GetCurveValue(double x, double y, double z);
	void CreateVoiceCoilFlyBackWaveformByStripe(BufferPtr data, long length, StripInfo* pCurrentStripe);
private:
//********** General parameters **********
	double _physicalStripeWidth;
	double _voicecoilZToVolts;	//field to voltage
	double _zeroPointVolt;
	double _maxPointVolt;
	double _centerShiftX;
	double _centerShiftY;
//********** Flyback line **********
	double _paramAmp;	//amplitude of sine curve
	double _paramFreq;	//frequency of sine curve
	double* _sineArr;
	unsigned int _extendLinesEnd;	//extend the Voice coil waveform linearly before scanning stripe
	unsigned int _extendLinesStart;	//extend the Voice coil waveform linearly after scanning stripe
	//unsigned int _voiceCoilSkipLines;	//set skip lines to a fixed value

	long CheckParameters(unsigned int samplesPerLine, double fieldWidth, double fieldHeight, unsigned int extendLinesEnd,
		unsigned int extendLinesStart, unsigned int pointsPerLine);
	inline double GetCurveYDiff(double y, double paramB);
	inline void CreateFlyBackWaveform(BufferPtr data, long length, long extendLinesEnd, long extendLinesStart, double preValue, double nextValue, double preDelta, double nextDelta);
};
