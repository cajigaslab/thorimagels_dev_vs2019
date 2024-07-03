#include "GalvoXWaveForm.h"

GalvoXWaveForm::GalvoXWaveForm()
{
	_sineArr = NULL;
	_galvoX2Scale = 1.0;
	_galvoX2Shift = 0.0;
}

long GalvoXWaveForm::SetParameters(double f2TGx1, unsigned int samplesPerLine, double maxFieldWidth, double zeroPointVoltGX1,
								   double galvoX2Scale, double galvoX2Shift, unsigned int extend_lines_start, unsigned int extend_lines_end)
{
	if (CheckParameters(f2TGx1, maxFieldWidth, samplesPerLine, extend_lines_start, extend_lines_end, zeroPointVoltGX1) != TRUE)
		return FALSE;
	_f2TGx1 = f2TGx1;
	_samplesPerLine = samplesPerLine;
	_extendLinesStart = extend_lines_start;
	_extendLinesEnd = extend_lines_end;
	_zeroPointVoltGX1 = zeroPointVoltGX1;
	_galvoX2Scale = galvoX2Scale;
	_galvoX2Shift = galvoX2Shift;
	return TRUE;
}

long GalvoXWaveForm::SetFlyBackParams(double maxVelocity, double maxOvershoot, double pointsPerSecond)
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

long GalvoXWaveForm::CheckParameters(double f2TGx1, double maxFieldWidth, unsigned int samplesPerLine,
									 unsigned int extend_lines_start, unsigned int extend_lines_end, double zeroPointVoltGX1)
{
	if (maxFieldWidth < 0)
		return FALSE;
	const double maxF2TGx1 = (MAX_VOLT_GX1 - MIN_VOLT_GX1) / maxFieldWidth;
	if (f2TGx1 < -maxF2TGx1 || f2TGx1 > maxF2TGx1)
		return FALSE;
	double f2TGx2 = f2TGx1*_galvoX2Scale;
	const double maxF2TGx2 = (MAX_VOLT_GX2 - MIN_VOLT_GX2) / maxFieldWidth;
	if (f2TGx2 < -maxF2TGx2 || f2TGx2 > maxF2TGx2)
		return FALSE;
	if (samplesPerLine < MIN_POKELSPOINT_COUNT || samplesPerLine > MAX_POKELSPOINT_COUNT)
		return FALSE;
	return TRUE;
}

long GalvoXWaveForm::SetCurrentPosition(double currentX1Pos, double currentX2Pos)
{
	_startX1Pos = max(MIN_VOLT_GX1, min(MAX_VOLT_GX1, currentX1Pos));
	_startX2Pos = max(MIN_VOLT_GX1, min(MAX_VOLT_GX1, currentX2Pos));
	return TRUE;
}

void GalvoXWaveForm::CreateCurrentPositionWaveformX1(BufferPtr data, long length)
{
	CreateConstantWaveform(data, length, _startX1Pos);
}

void GalvoXWaveForm::CreateCurrentPositionWaveformX2(BufferPtr data, long length)
{
	CreateConstantWaveform(data, length, _startX2Pos);
}

long GalvoXWaveForm::CreateStripeGalvoX1Waveform(BufferPtr pWaveform, StripInfo* pCurrentStripe)
{
	if (pWaveform == NULL || pCurrentStripe == NULL)
		return FALSE;
	long moveLength = pCurrentStripe->SkipSignal*_samplesPerLine;
	StripInfo* pLastStripe = pCurrentStripe->preStrip;
	if ((pLastStripe != NULL) && (!pCurrentStripe->IsStart))
	{
		CreateGalvoX1MoveWaveform(pWaveform, moveLength, false, pLastStripe->XPosResonMid, pCurrentStripe->XPosResonMid);
	}
	else
	{
		CreateGalvoX1MoveWaveform(pWaveform, moveLength, true, 0, pCurrentStripe->XPosResonMid);
	}
	pWaveform += moveLength;
	CreateConstantWaveform(pWaveform, pCurrentStripe->IncludeSignal*_samplesPerLine, GalvoX1FieldToVolts(pCurrentStripe->XPosResonMid));
	return TRUE;
}
long GalvoXWaveForm::CreateStripeGalvoX2Waveform(BufferPtr pWaveform, StripInfo * pCurrentStripe)
{
	if (pWaveform == NULL || pCurrentStripe == NULL)
		return FALSE;
	long moveLength = pCurrentStripe->SkipSignal*_samplesPerLine;
	StripInfo* pLastStripe = pCurrentStripe->preStrip;
	if ((pLastStripe != NULL) && (!pCurrentStripe->IsStart))
	{
		CreateGalvoX2MoveWaveform(pWaveform, moveLength, false, pLastStripe->XPosResonMid, pCurrentStripe->XPosResonMid);
	}
	else
	{
		CreateGalvoX2MoveWaveform(pWaveform, moveLength, true, 0, pCurrentStripe->XPosResonMid);
	}
	pWaveform += moveLength;
	CreateConstantWaveform(pWaveform, pCurrentStripe->IncludeSignal*_samplesPerLine, GalvoX2FieldToVolts(pCurrentStripe->XPosResonMid));
	return TRUE;
}
long GalvoXWaveForm::GetSkipLines(StripInfo* current_strip, SkipLines* pSkipLines)
{
	double startX1Volt;
	double startX2Volt;
	if (current_strip->preStrip == NULL)
	{
		startX1Volt = _startX1Pos;
		startX2Volt = _startX2Pos;
	}
	else
	{
		startX1Volt = GalvoX1FieldToVolts(current_strip->preStrip->XPosResonMid);
		startX2Volt = GalvoX2FieldToVolts(current_strip->preStrip->XPosResonMid);
	}
	SkipLines linesX1 = { false, 0, 0, 0 }, linesX2 = { false, 0, 0, 0 };
	CalculateFlyBackLines(_paramAmp, _paramFreq, _samplesPerLine, startX1Volt, GalvoX1FieldToVolts(current_strip->XPosResonMid), 0, 0, &linesX1);
	CalculateFlyBackLines(_paramAmp, _paramFreq, _samplesPerLine, startX2Volt, GalvoX2FieldToVolts(current_strip->XPosResonMid), 0, 0, &linesX2);
	*pSkipLines = MergeMaxLines(linesX1, linesX2) + _extendLinesEnd + _extendLinesStart;
	return TRUE;
}
GalvoXWaveForm::~GalvoXWaveForm()
{
	if (_sineArr != NULL)
		free(_sineArr);
}
void GalvoXWaveForm::CreateGalvoX1MoveWaveform(BufferPtr data, long length, bool isStartStripe, double start, double end)
{
	double startValue = isStartStripe ? _startX1Pos : GalvoX1FieldToVolts(start);
	double endValue = GalvoX1FieldToVolts(end);
	CreateFlyBackWaveform(data, length, startValue, endValue);
}
void GalvoXWaveForm::CreateGalvoX2MoveWaveform(BufferPtr data, long length, bool isStartStripe, double start, double end)
{
	double startValue = isStartStripe ? _startX2Pos : GalvoX2FieldToVolts(start);
	double endValue = GalvoX2FieldToVolts(end);
	CreateFlyBackWaveform(data, length, startValue, endValue);
}

void GalvoXWaveForm::CreateFlyBackWaveform(BufferPtr data, long length, double startValue, double endValue)
{
	BufferPtr pWaveform = data;
	long flyBackLength = length - _samplesPerLine*_extendLinesEnd - _samplesPerLine*_extendLinesStart;
	CreateConstantWaveform(pWaveform, _samplesPerLine*_extendLinesEnd, startValue);
	pWaveform += _samplesPerLine*_extendLinesEnd;
	CreateCommonFlyBackWaveform(_paramAmp, _paramFreq, pWaveform, flyBackLength, startValue, endValue, 0, 0, _sineArr);
	pWaveform += flyBackLength;
	CreateConstantWaveform(pWaveform, _samplesPerLine*_extendLinesStart, endValue);
}

void GalvoXWaveForm::MoveToPosition(double* data, long length, double oldPositionValue, double newPositionValue)
{
	CreateCommonFlyBackWaveform<double*>(_paramAmp, _paramFreq, data, length, oldPositionValue, newPositionValue, 0, 0, _sineArr);
}
