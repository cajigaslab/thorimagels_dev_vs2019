#pragma once
#include "WaveformModel.h"
#include "CommonWaveform.h"
#include "DevParamDef.h"

class PokelsCellWaveForm
{
public:
	PokelsCellWaveForm();
	~PokelsCellWaveForm();
	long SetParameters(int samplesPerLine, double dutyCycle, double resonanceWidth, double pockelInMax, double pockelInMin,double pockelMinPercent, vector<pair<uint16_t, double>>* powerBoxs, int twowayOffset=2);
	long GetSkipLines(StripInfo* current_strip, SkipLines* pSkipLines);
	long GenerateStripWaveform(BufferPtr pData, StripInfo* pStrip);
	PowerMask* GeneratePowerMask(StripInfo* pStrip);
	void MoveToPosition(double* data, long length, double newPositionValue);
	long GetPowerMask(StripInfo* pStrip);
	PowerMask* FindPowerMask(StripInfo* pStrip);
	long ResetPowerMask(int id, bool onlyResetUnusedMask);
	long ResetAllPowerMask();
private:
	int _samplesPerLine;
	int _samplesPerLine_half;
	int _twowayOffset;
	double _dutyCycle;
	double _resonanceWidth;
	double _pockelInMax;
	double _pockelInMin;
	double _pockelMinPercent;
	int _sampleStart;
	int _sampleEnd;
	uint8_t* _lineWaveform;
	int _currentScanId;
	double _powerMap[256];
	int ConvertPosition2Sample(double position,int samplesPerLine);
	double ConvertSample2Position(int sample, int samplesPerLine);
	long CheckParameters(int samplesPerLine, double dutyCycle, double resonanceWidth);
	vector<double> GetNode(double x, ROI* roi);
	vector<pair<uint16_t, vector<PowerMask*>>> _defaultPowerMaskVectors;
	vector<pair<uint16_t, vector<PowerMask*>>> _ROIPowerMaskVectors;
	vector<PowerMask*>* _defaultPowerMaskVector;
	vector<PowerMask*>* _ROIPowerMaskVector;
	long SelectPowerMask(uint16_t id);
};