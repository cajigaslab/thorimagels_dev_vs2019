#pragma once
#define _USE_MATH_DEFINES

#include "WaveformModel.h"
#include "DevParamDef.h"

struct SkipPoints
{
	bool isSecondRangeAvailable;
	int firstRangeStart;
	int firstRangeEnd;
	int secondRangeStart;
};

struct FlybackParams
{
	bool needLinear;
	int points;
	int refPoints;
	double sign;
	int theta1;
	int theta2;
	int tangentPoint;
	double offset1;
	double offset2;
};

struct SkipLines
{
	bool isSecondRangeAvailable;
	int firstRangeStart;
	int firstRangeEnd;
	int secondRangeStart;

	inline SkipLines operator +(const int offset) const
	{
		SkipLines skipLines;
		skipLines.isSecondRangeAvailable = isSecondRangeAvailable;
		skipLines.firstRangeStart = firstRangeStart + offset;
		skipLines.firstRangeEnd = firstRangeEnd + offset;
		skipLines.secondRangeStart = secondRangeStart + offset;
		return skipLines;
	}
};

const double M_2PI = 2.0*M_PI;

SkipLines MergeMaxLines(SkipLines skipLines1, SkipLines skipLine2);
long CalculateFlyBackLines(double paramA, double paramF, int pokelsPoints, double startValue, double endValue, double startDelta, double endDelta, SkipLines* pSkipLines);
void CreateConstantWaveformInt8(UInt8BufferPtr data, long length, uint8_t value);
double* CreateSineCurve(double paramA, double paramF);
long CalculateFlyBackPoints(double paramA, double paramF, double startValue, double endValue, double startDelta, double endDelta, SkipPoints* pSkipPoints);
bool IsSameFrame(ChanBufferInfo* f1, ChanBufferInfo* f2);

template <class T>
void CreateLinearWaveform(T pData, long length, double startValue, double endValue);

template <class T>
long CreateCommonFlyBackWaveform(double paramA, double paramF, T data, long length, double startValue, double endValue, double startDelta, double endDelta, double* sineArr);

template <class T>
void CreateConstantWaveform(T pData, long length, double value);
