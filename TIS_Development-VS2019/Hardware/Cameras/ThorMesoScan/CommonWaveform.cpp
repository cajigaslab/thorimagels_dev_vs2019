#include "CommonWaveform.h"

bool TryCalculateFlybackLineParams(double sign, double paramA, double paramF, double startValue, double endValue, double startDelta, double endDelta, FlybackParams* params)
{
	double F2PI = M_2PI * paramF;
	params->sign = sign;
	double delta = -sign * paramA*F2PI;
	{
		params->theta1 = static_cast<int>(acos(min(1.0, max(-1.0, sign*startDelta / (paramA*F2PI)) / F2PI)));
		params->theta2 = static_cast<int>(acos(min(1.0, max(-1.0, sign*endDelta / (paramA*F2PI)) / F2PI)));

		params->offset1 = -sign * paramA*sin((double)(params->theta1)*M_2PI * paramF) + startValue;
		params->offset2 = -sign * paramA*sin(-(double)(params->theta2)*M_2PI * paramF) + endValue;

		if ((params->offset2 - params->offset1 > 0 && sign == 1) || (params->offset2 - params->offset1 < 0 && sign == -1))
		{
			params->needLinear = false;
			if (params->offset2 - params->offset1 > 2 * paramA || params->offset2 - params->offset1 < -2 * paramA)
			{
				return false;
			}
			params->tangentPoint = static_cast<int>((M_PI - asin(min(1.0, max(-1.0, sign*(params->offset2 - params->offset1) / paramA / 2)))) / F2PI + 0.5);
			int refTangentPoint = static_cast<int>(asin(min(1.0, max(-1.0, sign*(params->offset2 - params->offset1) / paramA / 2))) / F2PI + 0.5);
			if (params->tangentPoint < params->theta1 || params->tangentPoint < params->theta2)
			{
				return false;
			}

			params->points = static_cast<long>(params->tangentPoint * 2 - params->theta1 - params->theta2);
			if (refTangentPoint < params->theta1 || refTangentPoint < params->theta2)
			{
				params->refPoints = 0;
			}
			else
			{
				params->refPoints = static_cast<long>(refTangentPoint * 2 - params->theta1 - params->theta2);
			}
		}
		else
		{
			params->needLinear = true;
			params->tangentPoint = static_cast <int>(1 / paramF / 2);
			params->points = static_cast<long>((params->offset2 - params->offset1) / delta + (params->tangentPoint * 2 - params->theta1 - params->theta2));
			params->refPoints = 0;
		}
	}
	return true;
}

SkipLines MergeMaxLines(SkipLines skipLines1, SkipLines skipLines2)
{
	SkipLines newSkipLines;
	int maxVal1 = max(skipLines1.firstRangeStart, skipLines2.firstRangeStart);
	if (skipLines1.isSecondRangeAvailable || skipLines2.isSecondRangeAvailable)
	{
		int minVal1End;
		int maxVal2;
		if (skipLines1.isSecondRangeAvailable && skipLines2.isSecondRangeAvailable)
		{
			minVal1End = min(skipLines1.firstRangeEnd, skipLines2.firstRangeEnd);
			if (minVal1End < maxVal1)
			{
				minVal1End = max(skipLines1.firstRangeEnd, skipLines2.firstRangeEnd);
				maxVal1 = min(skipLines1.secondRangeStart, skipLines2.secondRangeStart);
			}
			maxVal2 = max(skipLines1.secondRangeStart, skipLines2.secondRangeStart);
		}
		else if (skipLines1.isSecondRangeAvailable)
		{
			minVal1End = skipLines1.firstRangeEnd;
			maxVal2 = max(skipLines1.secondRangeStart, skipLines2.firstRangeStart);
		}
		else
		{
			minVal1End = skipLines2.firstRangeEnd;
			maxVal2 = max(skipLines2.secondRangeStart, skipLines1.firstRangeStart);
		}

		if (minVal1End < maxVal1)
		{
			newSkipLines.isSecondRangeAvailable = false;
			newSkipLines.firstRangeStart = maxVal2;
			newSkipLines.firstRangeEnd = 0;
			newSkipLines.secondRangeStart = 0;
		}
		else
		{
			newSkipLines.isSecondRangeAvailable = true;
			newSkipLines.firstRangeStart = maxVal1;
			newSkipLines.firstRangeEnd = minVal1End;
			newSkipLines.secondRangeStart = maxVal2;
		}
	}
	else
	{
		newSkipLines.isSecondRangeAvailable = false;
		newSkipLines.firstRangeStart = maxVal1;
		newSkipLines.firstRangeEnd = 0;
		newSkipLines.secondRangeStart = 0;
	}
	return newSkipLines;
}


//calculate approximate flyback line length and return lines to be skipped
long CalculateFlyBackLines(double paramA, double paramF, int samplesPerLine, double startValue, double endValue, double startDelta, double endDelta, SkipLines* pSkipLines)
{
	SkipPoints skipPoints;
	CalculateFlyBackPoints(paramA, paramF, startValue, endValue, startDelta, endDelta, &skipPoints);
	pSkipLines->firstRangeStart = skipPoints.firstRangeStart % samplesPerLine == 0 ? skipPoints.firstRangeStart / samplesPerLine : skipPoints.firstRangeStart / samplesPerLine + 1;
	if (skipPoints.isSecondRangeAvailable)
	{
		if (skipPoints.firstRangeEnd / samplesPerLine < pSkipLines->firstRangeStart)
		{
			pSkipLines->isSecondRangeAvailable = false;
			pSkipLines->firstRangeEnd = 0;
			pSkipLines->secondRangeStart = 0;
		}
		else
		{
			pSkipLines->isSecondRangeAvailable = true;
			pSkipLines->firstRangeEnd = skipPoints.firstRangeEnd / samplesPerLine;
			pSkipLines->secondRangeStart = skipPoints.secondRangeStart % samplesPerLine == 0 ? skipPoints.secondRangeStart / samplesPerLine : skipPoints.secondRangeStart / samplesPerLine + 1;
		}
	}
	else
	{
		pSkipLines->isSecondRangeAvailable = false;
	}
	return TRUE;
}

void CreateConstantWaveformInt8(UInt8BufferPtr pData, long length, uint8_t value)
{
	int cycle;
	for (cycle = 0; cycle < length; cycle++)
	{
		*pData = value;
		++pData;
	}
}

double * CreateSineCurve(double paramA, double paramF)
{
	const double M_2PI = 2.0*M_PI;
	int length = static_cast<int>(1.0 / paramF + 1);
	double* sineArr = (double *)malloc(length * sizeof(double));
	if (NULL == sineArr)
		return NULL;

	for (int i = 0; i < length; i++)
	{
		sineArr[i] = paramA * sin((double)i*M_2PI * paramF);
	}
	return sineArr;
}

long CalculateFlyBackPoints(double paramA, double paramF, double startValue, double endValue, double startDelta, double endDelta, SkipPoints* pSkipPoints)
{
	double F2PI = M_2PI * paramF;
	if (startDelta > paramA*F2PI || startDelta < -paramA * F2PI || endDelta > paramA*F2PI || endDelta < -paramA * F2PI)
	{
		return false;
	}

	FlybackParams paramSign1 = { false,0,0,0,0,0,0 };
	FlybackParams paramSign2 = { false,0,0,0,0,0,0 };

	bool sign1Succeeded = TryCalculateFlybackLineParams(-1, paramA, paramF, startValue, endValue, startDelta, endDelta, &paramSign1);
	bool sign2Succeeded = TryCalculateFlybackLineParams(1, paramA, paramF, startValue, endValue, startDelta, endDelta, &paramSign2);

	FlybackParams *firstParamByPoints, *secondParamByPoints;
	if (sign1Succeeded && sign2Succeeded)
	{
		pSkipPoints->isSecondRangeAvailable = true;
		if (paramSign1.points <= paramSign2.points)
		{
			firstParamByPoints = &paramSign1;
			secondParamByPoints = &paramSign2;
		}
		else
		{
			firstParamByPoints = &paramSign2;
			secondParamByPoints = &paramSign1;
		}
	}
	else if (sign1Succeeded)
	{
		pSkipPoints->isSecondRangeAvailable = false;
		firstParamByPoints = &paramSign1;
		secondParamByPoints = &paramSign2;
	}
	else if (sign2Succeeded)
	{
		firstParamByPoints = &paramSign2;
		secondParamByPoints = &paramSign1;
		pSkipPoints->isSecondRangeAvailable = false;
	}
	else
	{
		return false;
	}
	pSkipPoints->firstRangeStart = firstParamByPoints->points;
	pSkipPoints->firstRangeEnd = secondParamByPoints->refPoints;
	pSkipPoints->secondRangeStart = secondParamByPoints->points;
	return TRUE;
}

bool IsSameFrame(ChanBufferInfo* f1, ChanBufferInfo* f2)
{
	return (f1->ScanID == f2->ScanID) && (f1->ScanAreaID == f2->ScanAreaID) && (f1->StreamID == f2->StreamID) &&
		(f1->TimeID == f2->TimeID) && (f1->ZID == f2->ZID);
}

template <class T>
void CreateLinearWaveform(T pData, long length, double startValue, double endValue)
{
	int cycle;
	double step = (endValue - startValue) / length;
	double value = startValue;
	for (cycle = 0; cycle < length; cycle++)
	{
		*pData = value;
		++pData;
		value += step;
	}
}

template <class T>
long CreateCommonFlyBackWaveform(double paramA, double paramF, T data, long length, double startValue, double endValue, double startDelta, double endDelta, double* sineArr)
{
	if (length == 0)
	{
		return TRUE;
	}

	FlybackParams paramSign1 = { false,0,0,0,0,0,0 };
	FlybackParams paramSign2 = { false,0,0,0,0,0,0 };

	bool sign1Succeeded = TryCalculateFlybackLineParams(-1, paramA, paramF, startValue, endValue, startDelta, endDelta, &paramSign1);
	bool sign2Succeeded = TryCalculateFlybackLineParams(1, paramA, paramF, startValue, endValue, startDelta, endDelta, &paramSign2);

	FlybackParams* pSelectedParamSet = NULL;
	FlybackParams* pInitialParamSet = NULL;

	if (sign1Succeeded && length == paramSign1.points)
	{
		pSelectedParamSet = &paramSign1;
	}
	else if (sign2Succeeded && length == paramSign2.points)
	{
		pSelectedParamSet = &paramSign2;
	}
	else if (sign1Succeeded && sign2Succeeded)
	{
		pInitialParamSet = paramSign1.points <= paramSign2.points ? &paramSign1 : &paramSign2;
	}
	else if (sign1Succeeded)
	{
		pInitialParamSet = &paramSign1;
	}
	else if (sign2Succeeded)
	{
		pInitialParamSet = &paramSign2;
	}

	if (pSelectedParamSet != NULL)
	{
		if (!pSelectedParamSet->needLinear)
		{
			int length1 = pSelectedParamSet->tangentPoint - pSelectedParamSet->theta1 + 1;
			T pData = data;
			for (int i = pSelectedParamSet->theta1; i <= pSelectedParamSet->tangentPoint; i++)
			{
				*pData = max(VOLT_MIN, min(VOLT_MAX, pSelectedParamSet->sign * sineArr[i] + pSelectedParamSet->offset1));
				++pData;
			}

			int length2 = pSelectedParamSet->tangentPoint - pSelectedParamSet->theta2;
			pData = data + length - 1;
			for (int i = pSelectedParamSet->theta2 + 1; i <= pSelectedParamSet->tangentPoint; i++)
			{
				*pData = max(VOLT_MIN, min(VOLT_MAX, -pSelectedParamSet->sign * sineArr[i] + pSelectedParamSet->offset2));
				--pData;
			}
		}
		else
		{
			int length1 = pSelectedParamSet->tangentPoint - pSelectedParamSet->theta1 + 1;
			T pData = data;
			for (int i = pSelectedParamSet->theta1; i <= pSelectedParamSet->tangentPoint; i++)
			{
				*pData = max(VOLT_MIN, min(VOLT_MAX, pSelectedParamSet->sign * sineArr[i] + pSelectedParamSet->offset1));
				++pData;
			}

			int length2 = pSelectedParamSet->tangentPoint - pSelectedParamSet->theta2;
			pData = data + length - 1;
			for (int i = pSelectedParamSet->theta2 + 1; i <= pSelectedParamSet->tangentPoint; i++)
			{
				*pData = max(VOLT_MIN, min(VOLT_MAX, -pSelectedParamSet->sign * sineArr[i] + pSelectedParamSet->offset2));
				--pData;
			}
			if (0 <= length - length1 - length2 + 1 && 0 <= length - length2)
				CreateLinearWaveform(data + length1 - 1, length - length1 - length2 + 1, data[length1 - 1], data[length - length2]);
		}
	}
	else
	{
		//the flyback line is composed of 3 curves: 2 sine curves at the start and the end, and 1 straight line between them
		double sign1 = pInitialParamSet->sign;
		double sign2 = pInitialParamSet->sign;
		double F2PI = M_2PI * paramF;

		//validate for angles
		int theta1 = static_cast<int>(acos(min(1.0, max(-1.0, sign1*startDelta / (paramA*F2PI)))) / F2PI);	//theta of the start point of first sine curve
		int theta2 = static_cast<int>(acos(min(1.0, max(-1.0, sign2*endDelta / (paramA*F2PI)))) / F2PI);	//theta of the end point of second sine curve

		double offset1 = -sign1 * sineArr[theta1] + startValue;	//max y value of flyback line
		double offset2 = sign2 * sineArr[theta2] + endValue;	//min y value of flyback line
		int estimatedLineStartX, estimatedLineEndX;
		double estimatedLineStartY, estimatedLineEndY;
		int mid = static_cast<int>(1 / paramF / 4);

		if (theta1 > mid || theta2 > mid)
		{
			if (theta1 >= theta2)
			{
				estimatedLineStartX = 0;
				estimatedLineStartY = startValue;
				estimatedLineEndX = theta1 - theta2;
				estimatedLineEndY = offset2 - sign2 * sineArr[theta1];
			}
			else
			{
				estimatedLineStartX = theta2 - theta1;
				estimatedLineStartY = offset1 + sign1 * sineArr[theta2];
				estimatedLineEndX = 0;
				estimatedLineEndY = endValue;
			}
		}
		else
		{
			estimatedLineStartX = mid - theta1;
			estimatedLineStartY = offset1 + sign1 * paramA;
			estimatedLineEndX = mid - theta2;
			estimatedLineEndY = offset2 - sign2 * paramA;
		}
		//calculate the line slope and angle
		double delta = (estimatedLineEndY - estimatedLineStartY) / (length - (estimatedLineStartX + estimatedLineEndX));
		int thetaLine = static_cast<int>(acos(min(1.0, max(-1.0, sign1*delta / (paramA*F2PI)))) / F2PI);
		int thetaLine1 = thetaLine;
		int thetaLine2 = thetaLine;
		double sign1Update = sign1;
		double sign2Update = sign2;
		int theta1Update = theta1;
		int theta2Update = theta2;
		double offset1Update = offset1;
		double offset2Update = offset2;
		if (thetaLine < theta1)
		{
			sign1Update = -sign1;
			theta1Update = static_cast<int>(acos(min(1.0, max(-1.0, sign1Update*startDelta / (paramA*F2PI)))) / F2PI);
			offset1Update = -sign1Update * sineArr[theta1Update] + startValue;
			delta = (offset2 - offset1Update - sign1Update * paramA - sign2 * paramA) / (length - (mid * 2 - theta1Update - theta2));
			thetaLine1 = static_cast<int>(acos(min(1.0, max(-1.0, sign1Update*delta / (paramA*F2PI)))) / F2PI);
		}
		if (thetaLine < theta2)
		{
			sign2Update = -sign2;
			theta2Update = static_cast<int>(acos(min(1.0, max(-1.0, sign2Update*endDelta / (paramA*F2PI)))) / F2PI);
			offset2Update = sign2Update * sineArr[theta2Update] + endValue;
			delta = (offset2Update - offset1 - sign1 * paramA - sign2Update * paramA) / (length - (mid * 2 - theta1 - theta2Update));
			thetaLine2 = static_cast<int>(acos(min(1.0, max(-1.0, sign2Update*delta / (paramA*F2PI)))) / F2PI);
		}
		sign1 = sign1Update;
		sign2 = sign2Update;
		theta1 = theta1Update;
		theta2 = theta2Update;
		offset1 = offset1Update;
		offset2 = offset2Update;

		//interpolation
		int length1 = thetaLine1 - theta1 + 1;
		T pData = data;
		for (int i = theta1; i <= thetaLine1; i++)
		{
			*pData = max(VOLT_MIN, min(VOLT_MAX, sign1 * sineArr[i] + offset1));
			++pData;
		}

		int length2 = thetaLine2 - theta2;
		if (length2 <= 0)
		{
			CreateLinearWaveform(data + length1 - 1, length - length1 - length2 + 1, data[length1 - 1], endValue);
			return TRUE;
		}
		pData = data + length - 1;
		for (int i = theta2 + 1; i <= thetaLine2; i++)
		{
			*pData = max(VOLT_MIN, min(VOLT_MAX, -sign2 * sineArr[i] + offset2));
			--pData;
		}
		if (0 <= length - length1 - length2 + 1 && 0 <= length - length2)
			CreateLinearWaveform(data + length1 - 1, length - length1 - length2 + 1, data[length1 - 1], data[length - length2]);
	}
	return TRUE;
}

template <class T>
void CreateConstantWaveform(T pData, long length, double value)
{
	int cycle;
	for (cycle = 0; cycle < length; cycle++)
	{
		*pData = max(VOLT_MIN, min(VOLT_MAX, value));
		++pData;
	}
}

void TemplateFunctionDefinition()
{
	void *p;
	p = &CreateConstantWaveform<BufferPtr>;
	p = &CreateCommonFlyBackWaveform<BufferPtr>;
	p = &CreateLinearWaveform<BufferPtr>;

	p = &CreateConstantWaveform<double*>;
	p = &CreateCommonFlyBackWaveform<double*>;
	p = &CreateLinearWaveform<double*>;
}
