#include "stdafx.h"
#include "VoiceCoilWaveForm.h"

long VoiceCoilWaveForm::SetParameters(unsigned int samplesPerLine, double fieldWidth,
									  double fieldHeight, double voicecoilZToVolts, unsigned int extendLinesStart, unsigned int extendLinesEnd,
									  double zeroPointVolt, double maxPointVolt, unsigned int pointsPerLine)
{
	if (CheckParameters(samplesPerLine, fieldWidth, fieldHeight, extendLinesEnd, extendLinesStart, pointsPerLine) != TRUE)
		return FALSE;
	_samplesPerLine = samplesPerLine;
	_halfFieldWidth = fieldWidth / 2;
	_halfFieldHeight = fieldHeight / 2;
	_voicecoilZToVolts = voicecoilZToVolts;
	_zeroPointVolt = zeroPointVolt;
	_maxPointVolt = maxPointVolt;
	_extendLinesEnd = extendLinesEnd;
	_extendLinesStart = extendLinesStart;
	_stepsPerLine = pointsPerLine;
	return TRUE;
}

long VoiceCoilWaveForm::SetFlyBackParams(double maxVelocity, double maxOvershoot, double pointsPerSecond)
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

long VoiceCoilWaveForm::SetCurveParameters(double paramA, double paramB, double centerShiftX, double centerShiftY)
{
	if (paramA > VC_PARAM_MAX || paramA < 0 || paramB > VC_PARAM_MAX || paramB < 0)
		return FALSE;
	_paramA = paramA;
	_paramB = paramB;
	_centerShiftX = centerShiftX;
	_centerShiftY = centerShiftY;
	return TRUE;
}

long VoiceCoilWaveForm::SetCurrentPosition(double currentVPos)
{
	_startVPos = max(MIN_VOLT_VC, min(MAX_VOLT_VC, currentVPos));
	return TRUE;
}

void VoiceCoilWaveForm::CreateCurrentPositionWaveform(BufferPtr data, long length)
{
	CreateConstantWaveform(data, length, _startVPos);
}

long VoiceCoilWaveForm::CheckParameters(unsigned int samplesPerLine, double fieldWidth,	double fieldHeight,
										unsigned int extendLinesEnd, unsigned int extendLinesStart, unsigned int pointsPerLine)
{
	if (fieldWidth < 0 || fieldHeight < 0)
		return FALSE;
	if (samplesPerLine < MIN_POKELSPOINT_COUNT || samplesPerLine > MAX_POKELSPOINT_COUNT)
		return FALSE;
	if (pointsPerLine < 1) return FALSE;
	return TRUE;
}

double VoiceCoilWaveForm::GetCurveValue(double x, double y, double z)
{
	double xOffset = (x - _halfFieldWidth - _centerShiftX) / 1000;
	double yOffset = (y - _halfFieldHeight - _centerShiftY) / 1000;
	double posZ = _paramA*xOffset*xOffset + _paramB*yOffset*yOffset;
	return (posZ + z)*_voicecoilZToVolts + _zeroPointVolt;
	//return _voicecoilZToVolts*(z - maxValue + paramA*xOffset*xOffset + paramB*yOffset*yOffset) + _zeroPointVolt;
}

inline double VoiceCoilWaveForm::GetCurveYDiff(double y, double paramB)
{
	double yOffset = (y - _halfFieldHeight - _centerShiftY) / 1000;
	return _voicecoilZToVolts * 2 * paramB*yOffset / 1000;
}

long VoiceCoilWaveForm::CreateStripeVoiceCoilWaveform(BufferPtr pWaveform, StripInfo* pCurrentStripe)
{
	if (pWaveform == NULL || pCurrentStripe == NULL)
		return FALSE;
	long blankLength = pCurrentStripe->SkipSignal*_samplesPerLine;
	bool isTwoWay = pCurrentStripe->ScanMode == TWO_WAY_SCAN;
	int yLinesCount = pCurrentStripe->YSize;
	if (isTwoWay && pCurrentStripe->YSize % 2 == 1)
	{
		yLinesCount++;
	}

	CreateVoiceCoilFlyBackWaveformByStripe(pWaveform, blankLength, pCurrentStripe);
	pWaveform += blankLength;
	int points = isTwoWay ? _samplesPerLine / 2 : _samplesPerLine;
	double xStepSize = pCurrentStripe->XPhysicalSize / _stepsPerLine;
	double yPixelSize = pCurrentStripe->YPhysicalSize / pCurrentStripe->YSize / _stepsPerLine;
	long length = points / _stepsPerLine;
	long lastStepLength = points - _stepsPerLine*length + length;
	double x = pCurrentStripe->XPos + xStepSize / 2;
	double z = pCurrentStripe->ZPos;
	double currentY, nextY, value, nextValue;
	currentY = nextY = pCurrentStripe->YPos;
	for (int line = 0; line < yLinesCount; line++)
	{
		currentY = nextY;
		nextY = currentY + yPixelSize;
		value = GetCurveValue(x, currentY, z);
		nextValue = GetCurveValue(x, nextY, z);
		auto startPos = pWaveform + line*points;
		CreateLinearWaveform(startPos, points, value, nextValue);
	}
	//double x = pCurrentStripe->XPos;
	//double y = pCurrentStripe->YPos;
	//double z = pCurrentStripe->ZPos;
	//double nextX, nextY, nextZ, nextValue;
	//for (int line = 0; line < yLinesCount; line++)
	//{
	//	double startY = y + (line + 1)*_stepsPerLine*yPixelSize;
	//	double value = GetCurveValue(x, startY, z);
	//	auto startPos = pWaveform + line * points;
	//	for (unsigned int step = 1; step <= _stepsPerLine; step++)
	//	{
	//		nextX = x + xStepSize * step;
	//		nextY = y + ((line + 1)*_stepsPerLine + step)*yPixelSize;
	//		nextZ = z;
	//		nextValue = GetCurveValue(nextX, nextY, nextZ);
	//		long len = step == _stepsPerLine ? lastStepLength : length;
	//		CreateLinearWaveform(startPos + length * (step - 1), len, value, nextValue);
	//		value = nextValue;
	//	}
	//}
	return TRUE;
}
long VoiceCoilWaveForm::GetSkipLines(StripInfo* pCurrentStripe, SkipLines* pSkipLines)
{
	StripInfo* pLastStripe = pCurrentStripe->preStrip;
	bool isTwoWay = pCurrentStripe->ScanMode == TWO_WAY_SCAN;
	double startValue;
	double deltaLastStripe;
	long extendLinesEnd = _extendLinesEnd;
	if (pLastStripe != NULL)
	{
		int lastYLinesCount = pLastStripe->YSize;
		if (isTwoWay && pLastStripe->YSize % 2 == 1)
		{
			lastYLinesCount++;
		}
		double yEnd = pLastStripe->YPos + pLastStripe->YPhysicalSize / pLastStripe->YSize * lastYLinesCount;
		startValue = GetCurveValue(pLastStripe->XPos + pLastStripe->XPhysicalSize / 2, yEnd, pLastStripe->ZPos);
		deltaLastStripe = pLastStripe->YPhysicalSize / pLastStripe->YSize / (_samplesPerLine * (isTwoWay ? 0.5 : 1)) * GetCurveYDiff(yEnd, _paramB);
	}
	else
	{
		startValue = _startVPos;
		deltaLastStripe = 0;
		extendLinesEnd = 0;
	}
	double endValue = GetCurveValue(pCurrentStripe->XPos + pCurrentStripe->XPhysicalSize / 2, pCurrentStripe->YPos, pCurrentStripe->ZPos);
	double deltaNextStripe = pCurrentStripe->YPhysicalSize / pCurrentStripe->YSize / (_samplesPerLine * (isTwoWay ? 0.5 : 1)) * GetCurveYDiff(pCurrentStripe->YPos, _paramB);

	long extendLength1 = extendLinesEnd*_samplesPerLine;
	long extendLength2 = _extendLinesStart*_samplesPerLine;
	double point1Value = startValue + extendLength1*deltaLastStripe;
	double point2Value = endValue - extendLength2*deltaNextStripe;
	SkipLines lines;
	CalculateFlyBackLines(_paramAmp, _paramFreq, _samplesPerLine, point1Value, point2Value, deltaLastStripe, deltaNextStripe, &lines);
	*pSkipLines = lines + _extendLinesStart + extendLinesEnd;

	return TRUE;
}

void VoiceCoilWaveForm::CreateVoiceCoilFlyBackWaveformByStripe(BufferPtr data, long length, StripInfo* pCurrentStripe)
{
	StripInfo* pLastStripe = pCurrentStripe->preStrip;
	bool isTwoWay = pCurrentStripe->ScanMode == TWO_WAY_SCAN;
	double startValue;
	double deltaLastStripe;
	long extendLinesEnd = _extendLinesEnd;
	if ((pLastStripe != NULL) && (!pCurrentStripe->IsStart))
	{
		int lastYLinesCount = pLastStripe->YSize;
		if (isTwoWay && pLastStripe->YSize % 2 == 1)
		{
			lastYLinesCount++;
		}
		double yEnd = pLastStripe->YPos + pLastStripe->YPhysicalSize / pLastStripe->YSize * lastYLinesCount;
		startValue = GetCurveValue(pLastStripe->XPos + pLastStripe->XPhysicalSize / 2, yEnd, pLastStripe->ZPos);
		deltaLastStripe = pLastStripe->YPhysicalSize / pLastStripe->YSize / (_samplesPerLine * (isTwoWay ? 0.5 : 1)) * GetCurveYDiff(yEnd, _paramB);
	}
	else
	{
		startValue = _startVPos;
		deltaLastStripe = 0;
		extendLinesEnd = 0;
	}
	double endValue = GetCurveValue(pCurrentStripe->XPos + pCurrentStripe->XPhysicalSize / 2, pCurrentStripe->YPos, pCurrentStripe->ZPos);
	double deltaNextStripe = pCurrentStripe->YPhysicalSize / pCurrentStripe->YSize / (_samplesPerLine * (isTwoWay ? 0.5 : 1)) * GetCurveYDiff(pCurrentStripe->YPos, _paramB);
	CreateFlyBackWaveform(data, length, extendLinesEnd, _extendLinesStart, startValue, endValue, deltaLastStripe, deltaNextStripe);
}

void VoiceCoilWaveForm::CreateFlyBackWaveform(BufferPtr data, long length, long extendLinesEnd, long extendLinesStart, double preValue, double nextValue, double preDelta, double nextDelta)
{
	//extend the waveform at the start and end to stablize voice coil
	long extendLength1 = extendLinesEnd*_samplesPerLine;
	long extendLength2 = extendLinesStart*_samplesPerLine;
	double point1Value = preValue + extendLength1*preDelta;
	double point2Value = nextValue - extendLength2*nextDelta;
	CreateLinearWaveform(data, extendLength1, preValue, point1Value);
	CreateCommonFlyBackWaveform(_paramAmp, _paramFreq, data + extendLength1, length - extendLength1 - extendLength2, point1Value, point2Value, preDelta, nextDelta, _sineArr);
	CreateLinearWaveform(data + (length - extendLength2), extendLength2, point2Value, nextValue);
}

void VoiceCoilWaveForm::MoveToPosition(double* data, long length, double oldPositionValue, double newPositionValue)
{
	CreateCommonFlyBackWaveform<double*>(_paramAmp, _paramFreq, data, length, oldPositionValue, newPositionValue, 0, 0, _sineArr);
}

VoiceCoilWaveForm::VoiceCoilWaveForm()
{
	_sineArr = NULL;
}

VoiceCoilWaveForm::~VoiceCoilWaveForm()
{
	if (_sineArr != NULL)
	{
		free(_sineArr);
		_sineArr = NULL;
	}
}
