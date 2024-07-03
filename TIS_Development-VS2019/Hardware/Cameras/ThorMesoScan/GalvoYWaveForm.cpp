#include "stdafx.h"
#include "GalvoYWaveForm.h"
#include <math.h>

long GalvoYWaveForm::SetParameters(double f2TGy, unsigned int samplesPerLine, double maxFieldHeight, double zeroPointVoltGY, unsigned int extendLinesStart, unsigned int extendLinesEnd)
{
	if (CheckParameters(f2TGy, samplesPerLine, maxFieldHeight, zeroPointVoltGY) != TRUE)
		return FALSE;
	_f2TGy = f2TGy;
	_samplesPerLine = samplesPerLine;
	_zeroPointVoltGY = zeroPointVoltGY;

	_extendLinesEnd = extendLinesEnd;
	_extendLinesStart = extendLinesStart;

	return TRUE;
}

long GalvoYWaveForm::SetFlyBackParams(double maxVelocity, double maxOvershoot, double pointsPerSecond)
{
	if (maxVelocity <= 0.0 || maxVelocity >= FLYBACK_MAX_VELOCITY ||
		maxOvershoot <= 0.0 || maxOvershoot >= FLYBACK_MAX_OVERSHOOT || pointsPerSecond <= 0)
		return FALSE;
	double lineDeltaY = maxVelocity / pointsPerSecond;
	_paramAmp = maxOvershoot;
	_paramFreq = lineDeltaY / _paramAmp / M_2PI;
	if (_sineArr != NULL)
	{
		free(_sineArr);
		_sineArr = NULL;
	}
	_sineArr = CreateSineCurve(_paramAmp, _paramFreq);
	return (NULL != _sineArr) ? TRUE : FALSE;
}

long GalvoYWaveForm::SetCurrentPosition(double currentYPos)
{
	_startYPos = max(MIN_VOLT_GY, min(MAX_VOLT_GY, currentYPos));
	return TRUE;
}

void GalvoYWaveForm::CreateCurrentPositionWaveform(BufferPtr data, long length)
{
	CreateConstantWaveform(data, length, _startYPos);
}

long GalvoYWaveForm::CheckParameters(double f2TGy, unsigned int samplesPerLine, double maxFieldHeight, double zeroPointVoltGY)
{
	if (maxFieldHeight < 0)
		return FALSE;
	const double maxF2TGy = (MAX_VOLT_GY - MIN_VOLT_GY) / maxFieldHeight;
	if (f2TGy < -maxF2TGy || f2TGy > maxF2TGy)
		return FALSE;
	if (zeroPointVoltGY < MIN_VOLT_GY || zeroPointVoltGY > MAX_VOLT_GY)
		return FALSE;
	if (samplesPerLine < MIN_POKELSPOINT_COUNT || samplesPerLine > MAX_POKELSPOINT_COUNT)
		return FALSE;
	return TRUE;
}

long GalvoYWaveForm::CreateStripeGalvoYWaveform(BufferPtr pWaveform, StripInfo* pCurrentStripe)
{
	if (pWaveform == NULL || pCurrentStripe == NULL)
		return FALSE;
	bool isTwoWay = pCurrentStripe->ScanMode == TWO_WAY_SCAN;
	double startVolt = GalvoYFieldToVolts(pCurrentStripe->YPos);
	double endVolt = GalvoYFieldToVolts(pCurrentStripe->YPos + pCurrentStripe->YPhysicalSize);
	long xCount = isTwoWay ? _samplesPerLine / 2 : _samplesPerLine;
	double deltaVolt = (endVolt - startVolt) / pCurrentStripe->YSize / xCount;
	long blanklength = pCurrentStripe->SkipSignal * _samplesPerLine;
	double yEndVolt;
	StripInfo* pLastStripe = pCurrentStripe->preStrip;
	double deltaP;


	if ((pLastStripe != NULL) && (!pCurrentStripe->IsStart))
	{
		int lastYLinesCount = pLastStripe->YSize;
		if (isTwoWay && pLastStripe->YSize % 2 == 1)
		{
			lastYLinesCount++;
		}
		double lastPos = pLastStripe->YPos + pLastStripe->YPhysicalSize / pLastStripe->YSize * lastYLinesCount;
		yEndVolt = GalvoYFieldToVolts(lastPos);
		deltaP = deltaVolt;
	}
	else
	{
		yEndVolt = _startYPos;
		deltaP = 0;
	}
	double deltaN = deltaVolt;
	CreateFlyBackWaveform(pWaveform, blanklength, yEndVolt, GalvoYFieldToVolts(pCurrentStripe->YPos), deltaP, deltaN);
	//CreateCommonFlyBackWaveform(_paramAmp, _paramFreq, pWaveform, blanklength, yEndVolt, GalvoYFieldToVolts(pCurrentStripe->YPos), deltaP, deltaN, _sineArr);
	pWaveform += blanklength;
	double value = startVolt;
	int yLinesCount = pCurrentStripe->YSize;
	if (isTwoWay && pCurrentStripe->YSize % 2 == 1)
	{
		yLinesCount++;
	}
	for (unsigned int idx = 0; idx < static_cast<unsigned int>(yLinesCount * xCount); idx++)
	{
		*pWaveform = max(VOLT_MIN, min(VOLT_MAX, value));
		++pWaveform;
		value += deltaVolt;
	}
	return TRUE;
}
long GalvoYWaveForm::GetSkipLines(StripInfo* current_strip, SkipLines* pSkipLines)
{
	double speedPreByVolt;
	double posPreByVolt;
	bool isTwoWay = current_strip->ScanMode == TWO_WAY_SCAN;
	if (current_strip->preStrip != NULL)
	{
		speedPreByVolt = GalvoYFieldToVoltsOffset(current_strip->preStrip->YPhysicalSize / current_strip->preStrip->YSize / (_samplesPerLine * (isTwoWay ? 0.5 : 1)));
		posPreByVolt = GalvoYFieldToVolts(current_strip->preStrip->YPos + current_strip->preStrip->YPhysicalSize);
	}
	else
	{
		speedPreByVolt = 0;
		posPreByVolt = _startYPos;
	}
	double speedNextByVolt = GalvoYFieldToVoltsOffset(current_strip->YPhysicalSize / current_strip->YSize / (_samplesPerLine * (isTwoWay ? 0.5 : 1)));
	double posNextByVolt = GalvoYFieldToVolts(current_strip->YPos);

	long extendLength1 = _extendLinesEnd*_samplesPerLine;
	long extendLength2 = _extendLinesStart*_samplesPerLine;
	double point1Value = posPreByVolt + extendLength1*speedPreByVolt;
	double point2Value = posNextByVolt - extendLength2*speedNextByVolt;
	SkipLines lines;
	CalculateFlyBackLines(_paramAmp, _paramFreq, _samplesPerLine, point1Value, point2Value, speedPreByVolt, speedNextByVolt, &lines);
	*pSkipLines = lines + _extendLinesEnd + _extendLinesStart;
	return TRUE;
}

void GalvoYWaveForm::CreateFlyBackWaveform(BufferPtr data, long length, double preValue, double nextValue, double preDelta, double nextDelta)
{
	//extend the waveform at the start and end to stablize voice coil
	long extendLength1 = _extendLinesEnd*_samplesPerLine;
	long extendLength2 = _extendLinesStart*_samplesPerLine;
	double point1Value = preValue + extendLength1*preDelta;
	double point2Value = nextValue - extendLength2*nextDelta;
	CreateLinearWaveform(data, extendLength1, preValue, point1Value);
	CreateCommonFlyBackWaveform(_paramAmp, _paramFreq, data + extendLength1, length - extendLength1 - extendLength2, point1Value, point2Value, preDelta, nextDelta, _sineArr);
	CreateLinearWaveform(data + (length - extendLength2), extendLength2, point2Value, nextValue);
}

void GalvoYWaveForm::MoveToPosition(double* data, long length, double oldPositionValue, double newPositionValue)
{
	CreateCommonFlyBackWaveform<double*>(_paramAmp, _paramFreq, data, length, oldPositionValue, newPositionValue, 0, 0, _sineArr);
}

GalvoYWaveForm::GalvoYWaveForm()
{
	_sineArr = NULL;
}

GalvoYWaveForm::~GalvoYWaveForm()
{
	if (_sineArr != NULL)
	{
		free(_sineArr);
		_sineArr = NULL;
	}
}
