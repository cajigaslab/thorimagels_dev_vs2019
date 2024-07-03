#define _USE_MATH_DEFINES
#define POWER_GLOBAL	255
#define POWER_ZERO		254
#define SampleStart_Extend 2
#define SampleEnd_Extend 3

#include "stdafx.h"
#include "PokelsCellWaveForm.h"
#include "..\..\..\Common\PublicFuncs.h"
#include "Logger.h"
#include <omp.h>
#include <algorithm>
#include <Mmsystem.h>
#include <memory>
#include <math.h>

PokelsCellWaveForm::PokelsCellWaveForm()
{
	_lineWaveform = NULL;
	_currentScanId = -1;
}

PokelsCellWaveForm::~PokelsCellWaveForm()
{
	ResetAllPowerMask();
	if (_lineWaveform != NULL)
		std::free(_lineWaveform);
}

long PokelsCellWaveForm::SetParameters(int samplesPerLine, double dutyCycle, double resonanceWidth, double pockelInMax,double pockelInMin, double pockelMinPercent, vector<pair<uint16_t, double>>* powerBoxs,int twowayOffset)
{
	if (CheckParameters(samplesPerLine, dutyCycle, resonanceWidth) != TRUE)
		return FALSE;
	if (_samplesPerLine != 0 && (_samplesPerLine != samplesPerLine || _dutyCycle != dutyCycle || _resonanceWidth != resonanceWidth|| _twowayOffset != twowayOffset))
		ResetAllPowerMask();
	_dutyCycle = dutyCycle;
	_samplesPerLine = samplesPerLine;
	_samplesPerLine_half = _samplesPerLine / 2;
	_resonanceWidth = resonanceWidth;
	_twowayOffset = twowayOffset;
	_sampleStart = ConvertPosition2Sample((1 - _dutyCycle) / 2, _samplesPerLine);
	_sampleEnd = ConvertPosition2Sample((1 - _dutyCycle) / 2 + _dutyCycle, _samplesPerLine);
	_pockelInMax = pockelInMax;
	_pockelInMin = pockelInMin;
	_pockelMinPercent = pockelMinPercent;
	if (_sampleStart - SampleStart_Extend > 0)
		_sampleStart -= SampleStart_Extend;
	else
		_sampleStart = 0;
	if (_sampleEnd + SampleEnd_Extend < _samplesPerLine_half)
		_sampleEnd += SampleEnd_Extend;
	else
		_sampleEnd = _samplesPerLine_half;
	_lineWaveform = (uint8_t*)realloc(_lineWaveform, _samplesPerLine_half * sizeof(uint8_t));
	memset(_lineWaveform, POWER_ZERO, _samplesPerLine_half * sizeof(uint8_t));
	for (int i = 0; i < _sampleStart; i++)
	{
		_lineWaveform[i] = POWER_ZERO;
	}
	for (int i = _sampleStart; i < _sampleEnd; i++)
	{
		_lineWaveform[i] = POWER_GLOBAL;
	}
	for (int i = _sampleEnd; i < _samplesPerLine_half; i++)
	{
		_lineWaveform[i] = POWER_ZERO;
	}
	for (int i = 0; i < POWER_GLOBAL; i++)
	{
		_powerMap[i] = _pockelInMin;
	}
	for (vector<pair<uint16_t, double>>::iterator iter = (*powerBoxs).begin(); iter != (*powerBoxs).end(); iter++)
	{
		_powerMap[(*iter).first] = (*iter).second;
	}
	(*powerBoxs).clear();
	return TRUE;
}
long PokelsCellWaveForm::CheckParameters(int samplesPerLine, double dutyCycle, double resonanceWidth)
{
	if (samplesPerLine < MIN_POKELSPOINT_COUNT || samplesPerLine > MAX_POKELSPOINT_COUNT)
	{
		Logger::getInstance().LogMessage(L"Samples per line out of range.");
		return FALSE;
	}
	if (dutyCycle < 0 || dutyCycle>1)
	{
		Logger::getInstance().LogMessage(L"Duty cycle out of range.");
		return FALSE;
	}
	if (resonanceWidth <= 0)
	{
		Logger::getInstance().LogMessage(L"Resonance width out of range.");
		return FALSE;
	}
	return TRUE;
}
long PokelsCellWaveForm::GetSkipLines(StripInfo* current_strip, SkipLines* pSkipLines)
{
	SkipLines skipLines;
	skipLines.isSecondRangeAvailable = false;
	skipLines.firstRangeStart = skipLines.firstRangeEnd = skipLines.secondRangeStart = 0;
	*pSkipLines = skipLines;
	return TRUE;
}

vector<double> PokelsCellWaveForm::GetNode(double x, ROI* roi)
{
	vector<double> vector_y;
	switch (roi->Type)
	{
	case ROI_Rectangle:
		{
			vector_y.push_back(roi->Bound.y);
			vector_y.push_back(roi->Bound.y + roi->Bound.height);
			break;
		}
	case ROI_Ellipse:
		{
			double A = roi->Bound.width / 2;
			double B = roi->Bound.height / 2;
			double Ox = roi->Bound.x + A;
			double Oy = roi->Bound.y + B;
			double temp_y = sqrt(pow(B, 2) - pow(B / A * (x - Ox), 2));
			vector_y.push_back(Oy - temp_y);
			vector_y.push_back(Oy + temp_y);
			break;
		}
	case ROI_PolyLine:
	case ROI_Polygon:
		{
			for (int i = 0, l = (int)roi->Points.size(), j = l - 1; i < l; j = i, i++)
			{
				Point_64f* p1 = roi->Points[i];
				Point_64f* p2 = roi->Points[j];
				if (p1->X == p2->X)continue;
				else if (p1->X == x)
				{
					vector_y.push_back(p1->Y);
					continue;
				}
				else if (p2->X == x)
				{
					vector_y.push_back(p2->Y);
					continue;
				}

				if (p1->X > x && p2->X < x || p1->X < x && p2->X > x)
				{
					double temp_y = p1->Y + (x - p1->X)*(p2->Y - p1->Y) / (p2->X - p1->X);
					vector_y.push_back(temp_y);
				}
			}
			if (vector_y.size() > 0)
				sort(vector_y.begin(), vector_y.end());
			break;
		}
	}
	return vector_y;
}

inline int PokelsCellWaveForm::ConvertPosition2Sample(double position, int samplesPerLine)
{
	int i = (int)round((M_PI - acos(min(1.0, max(-1.0, 2 * position - 1)))) / (2 * M_PI) * samplesPerLine);
	if (i<0 || i>samplesPerLine)
		throw i;
	return i;
}

inline double PokelsCellWaveForm::ConvertSample2Position(int sample, int samplesPerLine)
{
	return 0.5*cos((double)sample / (double)samplesPerLine * 2 * M_PI + M_PI) + 0.5;
}

long PokelsCellWaveForm::GenerateStripWaveform(BufferPtr pData, StripInfo* pStrip)
{
	if (pStrip->PowerMask == NULL)
	{
		Logger::getInstance().LogMessage(L"Error of power mask.");
		return FALSE;
	}

	uint32_t powerMaskSize = _samplesPerLine * pStrip->IncludeSignal;
	for (uint32_t i = 0; i < pStrip->SkipSignal * _samplesPerLine; i++)
	{
		pData[i] = 0;
	}
	pData += pStrip->SkipSignal * _samplesPerLine;
	uint8_t* data = pStrip->PowerMask->data;
	double para = _pockelInMin;
	if (pStrip->PockelsVolt > _pockelMinPercent)
	{
		double tempVolt = (pStrip->PockelsVolt*pStrip->Power - _pockelMinPercent) / (1 - _pockelMinPercent/100);
		para = asin(pow(tempVolt / 100, 1.0 / 2.2)) / (M_PI*0.5)*(_pockelInMax-_pockelInMin)+ _pockelInMin;
		if (para > _pockelInMax) para = _pockelInMax;
		else if (para < _pockelInMin) para = _pockelInMin;
	}
	_powerMap[POWER_GLOBAL] = para;
	for (uint32_t i = 0; i < powerMaskSize; i++)
	{
		pData[i] = max(VOLT_MIN, min(VOLT_MAX, _powerMap[data[i]]));
	}
	return TRUE;
}

PowerMask* PokelsCellWaveForm::FindPowerMask(StripInfo* pStrip)
{
	if (pStrip->ROIPower.size() == 0)
	{
		for (int i = 0; i < static_cast<int>((*_defaultPowerMaskVector).size()); i++)
		{
			if ((*_defaultPowerMaskVector).at(i)->LineCount != pStrip->IncludeSignal || (*_defaultPowerMaskVector).at(i)->XPhysicalSize != pStrip->XPhysicalSize || (*_defaultPowerMaskVector).at(i)->YSize != pStrip->YSize)
				continue;
			return (*_defaultPowerMaskVector).at(i);
		}
	}
	else
	{
		for (vector<PowerMask*>::iterator iter = (*_ROIPowerMaskVector).begin(); iter != (*_ROIPowerMaskVector).end(); ++iter)
		{
			if ((*iter)->ROIs.size() != pStrip->ROIPower.size() || (*iter)->XPos != pStrip->XPos || (*iter)->LineCount != pStrip->IncludeSignal
				|| (*iter)->YSize != pStrip->YSize || (*iter)->XPhysicalSize != pStrip->XPhysicalSize)
				continue;
			bool isSamePowerbox = false;
			for (vector<ROI*>::iterator iterS = (*iter)->ROIs.begin(); iterS != (*iter)->ROIs.end(); ++iterS)
			{
				uint32_t idS = (*iterS)->ROIID;
				for (vector<pair<ROI*, double>>::iterator iterP = pStrip->ROIPower.begin(); iterP != pStrip->ROIPower.end(); ++iterP)
				{
					uint32_t idP = (*iterP).first->ROIID;
					if (idS == idP)
					{
						if ((*iterS)->Points.size() != (*iterP).first->Points.size())
						{
							isSamePowerbox = false;
							break;
						}
						for (int i = 0; i < static_cast<int>((*iterS)->Points.size()); i++)
						{
							if ((*iterP).first->Points[i]->X == (*iterS)->Points[i]->X && (*iterP).first->Points[i]->Y == (*iterS)->Points[i]->Y)
								isSamePowerbox = true;
							else
							{
								isSamePowerbox = false;
								break;
							}
						}
					}
					else
					{
						isSamePowerbox = false;
						continue;
					}
					if (isSamePowerbox == true)
						break;
				}
				if (!isSamePowerbox)
					break;
			}
			if (!isSamePowerbox)
				continue;
			return (*iter);
		}
	}
	return NULL;
}

long PokelsCellWaveForm::GetPowerMask(StripInfo* pStrip)
{
	if (pStrip->ChanBufInfo[0].ScanID != _currentScanId)
		SelectPowerMask(pStrip->ChanBufInfo[0].ScanID);

	PowerMask* powerMask = FindPowerMask(pStrip);
	if (powerMask == NULL)
	{
		powerMask = GeneratePowerMask(pStrip);
		if (pStrip->ROIPower.size() == 0)
			(*_defaultPowerMaskVector).push_back(powerMask);
		else
			(*_ROIPowerMaskVector).push_back(powerMask);
	}

	pStrip->PowerMask = powerMask;
	pStrip->PowerMask->isUsed = true;
	return TRUE;
}

long PokelsCellWaveForm::SelectPowerMask(uint16_t id)
{
	vector<PowerMask*>* defaultPowerMaskVector = NULL;
	vector<PowerMask*>* ROIPowerMaskVector = NULL;
	for (vector<pair<uint16_t, vector<PowerMask*>>>::iterator iter = _defaultPowerMaskVectors.begin(); iter != _defaultPowerMaskVectors.end(); iter++)
	{
		if ((*iter).first == id)
			defaultPowerMaskVector = &(*iter).second;
	}
	for (vector<pair<uint16_t, vector<PowerMask*>>>::iterator iter = _ROIPowerMaskVectors.begin(); iter != _ROIPowerMaskVectors.end(); iter++)
	{
		if ((*iter).first == id)
			ROIPowerMaskVector = &(*iter).second;
	}

	if (defaultPowerMaskVector != NULL)
	{
		_defaultPowerMaskVector = defaultPowerMaskVector;
		_ROIPowerMaskVector = ROIPowerMaskVector;
	}
	else
	{
		_defaultPowerMaskVectors.push_back(pair<uint16_t, vector<PowerMask*>>(id, vector<PowerMask*>()));
		_ROIPowerMaskVectors.push_back(pair<uint16_t, vector<PowerMask*>>(id, vector<PowerMask*>()));
		return SelectPowerMask(id);
	}

	_currentScanId = id;
	return TRUE;
}

long PokelsCellWaveForm::ResetAllPowerMask()
{
	for (vector<pair<uint16_t, vector<PowerMask*>>>::iterator iter = _defaultPowerMaskVectors.begin(); iter != _defaultPowerMaskVectors.end(); iter++)
	{
		for (vector<PowerMask*>::iterator iterP = (*iter).second.begin(); iterP != (*iter).second.end();)
		{
			delete *iterP;
			iterP = (*iter).second.erase(iterP);
		}
		(*iter).second.clear();
	}
	for (vector<pair<uint16_t, vector<PowerMask*>>>::iterator iter = _ROIPowerMaskVectors.begin(); iter != _ROIPowerMaskVectors.end(); iter++)
	{
		for (vector<PowerMask*>::iterator iterP = (*iter).second.begin(); iterP != (*iter).second.end();)
		{
			delete *iterP;
			iterP = (*iter).second.erase(iterP);
		}
		(*iter).second.clear();
	}
	return TRUE;
}

long PokelsCellWaveForm::ResetPowerMask(int id, bool onlyResetUnusedMask)
{
	if (id < 0)
		return TRUE;
	for (vector<pair<uint16_t, vector<PowerMask*>>>::iterator iter = _defaultPowerMaskVectors.begin(); iter != _defaultPowerMaskVectors.end(); iter++)
	{
		if ((*iter).first == id)
		{
			vector<PowerMask*>* defaultPowerMaskVector = &(*iter).second;
			for (vector<PowerMask*>::iterator iterP = (*defaultPowerMaskVector).begin(); iterP != (*defaultPowerMaskVector).end(); )
			{
				if (!onlyResetUnusedMask || !(*iterP)->isUsed)
				{
					delete *iterP;
					iterP = (*defaultPowerMaskVector).erase(iterP);
				}
				else
				{
					(*iterP)->isUsed = false;
					iterP++;
				}

			}
		}
	}
	for (vector<pair<uint16_t, vector<PowerMask*>>>::iterator iter = _ROIPowerMaskVectors.begin(); iter != _ROIPowerMaskVectors.end(); iter++)
	{
		if ((*iter).first == id)
		{
			vector<PowerMask*>* ROIPowerMaskVector = &(*iter).second;
			for (vector<PowerMask*>::iterator iterP = (*ROIPowerMaskVector).begin(); iterP != (*ROIPowerMaskVector).end();)
			{
				if (!onlyResetUnusedMask || !(*iterP)->isUsed)
				{
					delete *iterP;
					iterP = (*ROIPowerMaskVector).erase(iterP);
				}
				else
				{
					(*iterP)->isUsed = false;
					iterP++;
				}
			}
		}
	}
	return TRUE;
}

PowerMask* PokelsCellWaveForm::GeneratePowerMask(StripInfo* pStrip)
{
	PowerMask* powerMask = new PowerMask();
	powerMask->XPos = pStrip->XPos;
	powerMask->YSize = pStrip->YSize;
	powerMask->LineCount = pStrip->IncludeSignal;
	powerMask->XPhysicalSize = pStrip->XPhysicalSize;

	bool isTwoWay = pStrip->ScanMode == TWO_WAY_SCAN;
	if (pStrip->PowerMask != NULL)
		free(pStrip->PowerMask);
	powerMask->data = (uint8_t*)malloc(_samplesPerLine * pStrip->IncludeSignal * sizeof(uint8_t));
	uint8_t* pMask = powerMask->data;
	uint8_t* lineWaveform = (uint8_t*)malloc(_samplesPerLine_half * sizeof(uint8_t));
	int sampleEnd = _sampleEnd;
	int sampleStart = _sampleStart;
	memcpy_s(lineWaveform, _samplesPerLine_half * sizeof(uint8_t), _lineWaveform, _samplesPerLine_half * sizeof(uint8_t));
	if (abs(pStrip->XPhysicalSize - _resonanceWidth * _dutyCycle) > (pStrip->XPhysicalSize / pStrip->XSize))
	{
		//sampleEnd = ConvertPosition2Sample((1 - _dutyCycle) / 2 + pStrip->XPhysicalSize / _resonanceWidth, _samplesPerLine);
		sampleEnd = ConvertPosition2Sample((pStrip->XPhysicalSize / _resonanceWidth+1)/2, _samplesPerLine);
		if (sampleEnd + SampleEnd_Extend < _samplesPerLine_half)
			sampleEnd += SampleEnd_Extend;
		else
			sampleEnd = _samplesPerLine_half;
		for (int i = sampleEnd; i < _sampleEnd; i++)
		{
			lineWaveform[i] = POWER_ZERO;
		}

		sampleStart = ConvertPosition2Sample((1.0-pStrip->XPhysicalSize / _resonanceWidth ) / 2, _samplesPerLine);
		if (sampleStart - SampleStart_Extend > 0)
			sampleStart -= SampleStart_Extend;
		else
			sampleStart = 0;
		for (int i = _sampleStart; i < sampleStart; i++)
		{
			lineWaveform[i] = POWER_ZERO;
		}
	}

	if (isTwoWay)
	{
		uint8_t* lineWaveformReverse = (uint8_t*)malloc(_samplesPerLine_half * sizeof(uint8_t));
		memset(lineWaveformReverse, POWER_ZERO, _samplesPerLine_half * sizeof(uint8_t));
		int start = _twowayOffset <= 0 ? 0 : _twowayOffset;
		int end = _twowayOffset >= 0 ? _samplesPerLine_half : _samplesPerLine_half-abs(_twowayOffset);
		for (int i = start; i < end; i++)
		{
			lineWaveformReverse[i] = lineWaveform[_samplesPerLine_half - i - 1 + _twowayOffset];
		}

		for (uint32_t y = 0; y < pStrip->YSize; y += 2, pMask += _samplesPerLine)
		{
			memcpy_s(pMask, _samplesPerLine_half * sizeof(uint8_t), lineWaveform, _samplesPerLine_half * sizeof(uint8_t));
			memcpy_s(pMask + _samplesPerLine_half, _samplesPerLine_half * sizeof(uint8_t), lineWaveformReverse, _samplesPerLine_half * sizeof(uint8_t));
		}
		if (pStrip->YSize % 2 == 1)
			memset(pMask - _samplesPerLine_half , POWER_ZERO, _samplesPerLine_half);
		free(lineWaveformReverse);
	}
	else
	{
		for (uint32_t y = 0; y < pStrip->YSize; y++, pMask += _samplesPerLine)
		{
			memcpy_s(pMask, _samplesPerLine_half * sizeof(uint8_t), lineWaveform, _samplesPerLine_half * sizeof(uint8_t));
			memset(pMask + _samplesPerLine_half, POWER_ZERO, _samplesPerLine_half);
		}
	}

	if (pStrip->ROIPower.size() == 0)
	{
		free(lineWaveform);
		return powerMask;
	}

	for (vector<pair<ROI*, double>>::iterator iter = pStrip->ROIPower.begin(); iter != pStrip->ROIPower.end(); ++iter)
	{
		ROI* newROI = new ROI(*(*iter).first);
		newROI->Bound.width = ceil(newROI->Bound.width);
		newROI->Bound.x = floor(newROI->Bound.x);
		powerMask->ROIs.push_back(newROI);
	}

#pragma omp parallel for
	for (int x = sampleStart; x < sampleEnd; x++)
	{
		uint8_t* pMask = powerMask->data;
		double physicalX = ConvertSample2Position(x, _samplesPerLine)*_resonanceWidth + (pStrip->XPosResonMid - _resonanceWidth / 2);
		if (physicalX < 0)physicalX = 0;
		for (vector<ROI*>::iterator iter = powerMask->ROIs.begin(); iter != powerMask->ROIs.end(); ++iter)
		{
			ROI* roi = *iter;
			if (physicalX > pStrip->XPhysicalSize + pStrip->XPos)
				physicalX = pStrip->XPhysicalSize + pStrip->XPos;
			if (physicalX < roi->Bound.x || physicalX > roi->Bound.x + roi->Bound.width)
				continue;
			vector<double> ROINode = GetNode(physicalX, roi);
			vector<int> Node_y;
			for each (double node in ROINode)
			{
				int yPosition = (int)round((node - pStrip->YPos) * pStrip->YSize / pStrip->YPhysicalSize);
				Node_y.push_back(min(yPosition,(int)powerMask->YSize));
			}

			if (isTwoWay)
			{
				for (int nodeIndex = 0; nodeIndex < static_cast<int>(Node_y.size()); nodeIndex += 2)
				{
					uint32_t index = Node_y[nodeIndex] * _samplesPerLine_half;
					const uint32_t increase_odd = _samplesPerLine -2*x;
					const uint32_t increase_even = 2 * x;
					if (Node_y[nodeIndex] % 2 == 1 && Node_y[nodeIndex + 1] > Node_y[nodeIndex])
					{
						index += _samplesPerLine_half - x;
						pMask[index + _twowayOffset] = roi->ROIID;
						if (x == sampleStart + SampleStart_Extend)
						{
							for (int k = 1; k <= SampleEnd_Extend+1; ++k)
							{
								pMask[index + _twowayOffset + k] = roi->ROIID;
							}
						}
						else if (x == sampleEnd - 1 - SampleEnd_Extend)
						{
							for (int k = 1; k <= SampleStart_Extend+1; ++k)
							{
								pMask[index + _twowayOffset - k] = roi->ROIID;
							}
						}
						index += increase_even;
						Node_y[nodeIndex]++;

					}
					else
					{
						index += x;
					}

					if (Node_y[nodeIndex + 1] % 2 == 0 && Node_y[nodeIndex + 1] > Node_y[nodeIndex])
					{
						Node_y[nodeIndex + 1]--;
					}

					uint32_t temp_index = index;
					for (int i = Node_y[nodeIndex]; i < Node_y[nodeIndex + 1]; i += 2, temp_index += increase_even)
					{
						pMask[temp_index] = roi->ROIID;
						temp_index += increase_odd;
						pMask[temp_index +_twowayOffset] = roi->ROIID;
					}
					temp_index = index;
					if (x == sampleStart + SampleStart_Extend)
					{
						for (int i = Node_y[nodeIndex]; i < Node_y[nodeIndex + 1]; i += 2, temp_index += _samplesPerLine)
						{
							for (int k = 1; k <= SampleStart_Extend+1; ++k)
							{
								pMask[temp_index - k] = roi->ROIID;
								pMask[temp_index + k + _twowayOffset + increase_odd] = roi->ROIID;
							}
						}
					}
					else if (x == sampleEnd - 1 - SampleEnd_Extend)
					{
						for (int i = Node_y[nodeIndex]; i < Node_y[nodeIndex + 1]; i += 2, temp_index += _samplesPerLine)
						{
							for (int k = 1; k <= SampleEnd_Extend+1; ++k)
							{
								pMask[temp_index + k] = roi->ROIID;
								pMask[temp_index - k + _twowayOffset + increase_odd] = roi->ROIID;
							}
						}
					}
				}
			}
			else
			{
				for (int nodeIndex = 0; nodeIndex < static_cast<int>(Node_y.size()); nodeIndex += 2)
				{
					uint32_t index = Node_y[nodeIndex] * _samplesPerLine + x;
					uint32_t index_end = Node_y[nodeIndex + 1] * _samplesPerLine + x;
					for (uint32_t i = index; i < index_end; i += _samplesPerLine)
					{
						pMask[i] = roi->ROIID;
					}

					if (x == _sampleStart+ SampleStart_Extend)
					{
						for (uint32_t i =index; i < index_end; i += _samplesPerLine)
						{
							for(int k=1;k<= SampleStart_Extend;++k)
								pMask[i-k] = roi->ROIID;
						}
					}
					else if (x == sampleEnd-1 - SampleEnd_Extend)
					{
						for (uint32_t i = index; i < index_end; i += _samplesPerLine)
						{
							for (int k = 1; k <= SampleEnd_Extend; ++k)
								pMask[i + k] = roi->ROIID;
						}
					}
				}
			}
		}
	}
	std::free(lineWaveform);
	return powerMask;
}

void PokelsCellWaveForm::MoveToPosition(double* data, long length, double newPositionValue)
{
	CreateConstantWaveform<double*>(data, length, newPositionValue);
}
