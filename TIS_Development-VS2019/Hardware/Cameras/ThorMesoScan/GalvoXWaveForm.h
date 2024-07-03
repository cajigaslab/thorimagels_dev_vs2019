#pragma once
#define _USE_MATH_DEFINES

#include "WaveformModel.h"
#include "CommonWaveform.h"

class GalvoXWaveForm
{
public:
	GalvoXWaveForm();
	~GalvoXWaveForm();
	long SetParameters(double f2TGx1, unsigned int samplesPerLine, double fieldWidth, double zeroPointVoltGX1,
		double galvoX2Scale, double galvoX2Shift, unsigned int extend_lines_start, unsigned int extend_lines_end);
	long SetFlyBackParams(double maxVelocity, double maxOvershoot, double pointsPerSecond);
	long SetCurrentPosition(double currentX1Pos, double currentX2Pos);
    void CreateCurrentPositionWaveformX1(BufferPtr data, long length);
    void CreateCurrentPositionWaveformX2(BufferPtr data, long length);
	long CreateStripeGalvoX1Waveform(BufferPtr pWaveform, StripInfo* pCurrentStripe);
	long CreateStripeGalvoX2Waveform(BufferPtr pWaveform, StripInfo* pCurrentStripe);
	long GetSkipLines(StripInfo* current_strip, SkipLines* pSkipLines);
	void MoveToPosition(double* data, long length, double oldPositionValue, double newPositionValue);
protected:
	//********** General parameters **********
	unsigned int _samplesPerLine;
	//********** Start position **********
	double _startX1Pos;	//start position for X1
	double _startX2Pos;	//start position for X2

	inline double GalvoX1FieldToVolts(double size) 
	{ 
		return (size * _f2TGx1 + _zeroPointVoltGX1) * _galvoX2Scale + _galvoX2Shift;
	}
	inline double GalvoX2FieldToVolts(double size) 
	{ 
		return size * _f2TGx1 + _zeroPointVoltGX1;
	}

private:
	//********** General parameters **********
	double _f2TGx1;	//field to voltage
	double _zeroPointVoltGX1;	//zero point
	// X2=-galvoX2Scale*X1 + galvoX2Shift
	double _galvoX2Scale;
	double _galvoX2Shift;
	//********** Flyback line **********
	unsigned int _extendLinesStart;	//extend the constant voltage waveform of galvo X before scanning stripe
	unsigned int _extendLinesEnd;	//extend the constant voltage waveform of galvo X after scanning stripe
	//sine curve parameters of flyback line
	double _paramAmp;	//amplitude of sine curve
	double _paramFreq;	//frequency of sine curve
	double* _sineArr;

	long CheckParameters(double f2TGx1, double fieldWidth, unsigned int samplesPerLine,
		unsigned int extend_lines_start, unsigned int extend_lines_end, double zeroPointVoltGX1);
	void CreateGalvoX1MoveWaveform(BufferPtr data, long length, bool isStartStripe, double start, double end);
	void CreateGalvoX2MoveWaveform(BufferPtr data, long length, bool isStartStripe, double start, double end);
	void CreateFlyBackWaveform(BufferPtr data, long length, double startValue, double endValue);
};
