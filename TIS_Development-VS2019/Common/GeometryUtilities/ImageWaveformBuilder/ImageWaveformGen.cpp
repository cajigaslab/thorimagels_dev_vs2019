// WaveformBuilder.cpp : Defines the waveform building functions for the DLL application.
//

#include "stdafx.h"
#include "ImageWaveformBuilder.h"

///	***************************************** <summary> Build Line			</summary>	********************************************** ///

//srcDstVxy is expected to be: [FromVx, FromVy, ToVx, ToVy]; only build analog XY and Pockels
long ImageWaveformBuilder::BuildTravelToStart(double powerIdle, double * srcDstVxy, uint64_t& outCount)
{
	outCount = 0;
	double deltaX_volt = _gWaveXY[_scanAreaId].stepVolt; //[Volt/step] MAX_FIELD_SIZE * FIELD2VOLTS * SLOW_GALVO_RATE / _gWaveXY.ClockRate
	double deltaY_volt = deltaX_volt;		//same step size alone x and y

	//already arrived:
	if ((abs(*(srcDstVxy) - *(srcDstVxy+2)) < deltaX_volt) && (abs(*(srcDstVxy+1) - *(srcDstVxy+3)) < deltaY_volt))
	{
		return FALSE;
	}
	if(0 >= _gWaveXY[_scanAreaId].stepVolt)
		return FALSE;

	//Travel alone diagonal line with modulation ON/OFF and maximum speed:
	double DistanceVx = abs(*(srcDstVxy+2) - *(srcDstVxy));
	double DistanceVy = abs(*(srcDstVxy+3) - *(srcDstVxy+1));
	double xDirection = ((*(srcDstVxy+2) - *(srcDstVxy)) > 0) ? 1 : INV_DIRECTION;      //DistanceVx = 0 will be xDirection = 0.
	double yDirection = ((*(srcDstVxy+3) - *(srcDstVxy+1)) > 0) ? 1 : INV_DIRECTION;    //DistanceVy = 0 will be yDirection = 0.
	int Nd = std::max((int)floor(DistanceVx / deltaX_volt), (int)floor(DistanceVy / deltaY_volt));

	//Travel residue distance:
	double dV_sub_X = *(srcDstVxy+2) -(*(srcDstVxy) + (Nd-1) * xDirection * (float)std::min(deltaX_volt, DistanceVx / Nd));
	double dV_sub_Y = *(srcDstVxy+3) -(*(srcDstVxy+1) + (Nd-1) * yDirection * (float)std::min(deltaY_volt, DistanceVy / Nd));
	outCount = (dV_sub_X != 0 || dV_sub_Y != 0) ? (Nd + 1) : Nd;

	if (Nd > 0)
	{
		size_t unitSize = outCount * sizeof(double);
		_gWaveXY[_scanAreaId].GalvoWaveformXY = (double*)realloc(_gWaveXY[_scanAreaId].GalvoWaveformXY, 2 * unitSize);
		if(NULL == _gWaveXY[_scanAreaId].GalvoWaveformXY)
		{
			return FALSE;
		}
		_gWaveXY[_scanAreaId].analogXYSize = static_cast<long>(2 * unitSize);
		_gWaveXY[_scanAreaId].GalvoWaveformPockel = (double*)realloc(_gWaveXY[_scanAreaId].GalvoWaveformPockel, unitSize);
		if(NULL == _gWaveXY[_scanAreaId].GalvoWaveformPockel)
		{
			return FALSE;
		}
		_gWaveXY[_scanAreaId].analogPockelSize = static_cast<long>(unitSize);

		for (int id = 0; id < Nd; id++)
		{	
			_gWaveXY[_scanAreaId].GalvoWaveformXY[2*id] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (*(srcDstVxy) + id * xDirection * (float)std::min(deltaX_volt, DistanceVx / Nd))));
			_gWaveXY[_scanAreaId].GalvoWaveformXY[2*id+1] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (*(srcDstVxy+1) + id * yDirection * (float)std::min(deltaY_volt, DistanceVy / Nd))));
			_gWaveXY[_scanAreaId].GalvoWaveformPockel[id] = powerIdle;
		}
	}
	if (dV_sub_X != 0 || dV_sub_Y != 0)
	{
		_gWaveXY[_scanAreaId].GalvoWaveformXY[2*Nd] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (_gWaveXY[_scanAreaId].GalvoWaveformXY[2*(Nd-1)] + dV_sub_X)));
		_gWaveXY[_scanAreaId].GalvoWaveformXY[2*Nd+1] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (_gWaveXY[_scanAreaId].GalvoWaveformXY[2*(Nd-1)+1] + dV_sub_Y)));
		_gWaveXY[_scanAreaId].GalvoWaveformPockel[Nd] = powerIdle;
	}

	return TRUE;
}

//srcDstVxy is expected to be: [FromVx, FromVy]; build analog XY and multiple Pockels to the start of waveform body
//[Note]: must successfully invoke BuildGGFrameWaveformXY before this function
long ImageWaveformBuilder::BuildTravelToStartImagePos(long sAreaId, double stepVolts, double * srcDstVxy)
{
	if(0 >= stepVolts)
		return FALSE;

	//return if already arrived:
	if ((abs(*(srcDstVxy) - *(srcDstVxy+2)) < stepVolts) && (abs(*(srcDstVxy+1) - *(srcDstVxy+3)) < stepVolts))
		return FALSE;

	//locate destination position:
	if(TRUE == GetMutex(_gParams[sAreaId].bufferHandle))
	{
		SAFE_MEMCPY((void*)(srcDstVxy+2), (2*sizeof(double)), (void*)(_gParams[sAreaId].GalvoWaveformXY));
		ReleaseMutex(_gParams[sAreaId].bufferHandle);
	}

	double deltaX_volt = _gWaveXY[sAreaId].stepVolt = stepVolts;
	double deltaY_volt = deltaX_volt;		//same step size alone x and y

	//travel alone diagonal line with modulation ON/OFF and maximum speed:
	double DistanceVx = abs(*(srcDstVxy+2) - *(srcDstVxy));
	double DistanceVy = abs(*(srcDstVxy+3) - *(srcDstVxy+1));
	double xDirection = ((*(srcDstVxy+2) - *(srcDstVxy)) > 0) ? 1 : INV_DIRECTION;      //DistanceVx = 0 will be xDirection = 0.
	double yDirection = ((*(srcDstVxy+3) - *(srcDstVxy+1)) > 0) ? 1 : INV_DIRECTION;    //DistanceVy = 0 will be yDirection = 0.
	int Nd = std::max((int)floor(DistanceVx / deltaX_volt), (int)floor(DistanceVy / deltaY_volt));

	//travel residue distance:
	double dV_sub_X = *(srcDstVxy+2) -(*(srcDstVxy) + (Nd-1) * xDirection * (float)std::min(deltaX_volt, DistanceVx / Nd));
	double dV_sub_Y = *(srcDstVxy+3) -(*(srcDstVxy+1) + (Nd-1) * yDirection * (float)std::min(deltaY_volt, DistanceVy / Nd));
	long outCount = (dV_sub_X != 0 || dV_sub_Y != 0) ? (Nd + 1) : Nd;
	long pockelsCount = GetPockelsCount();
	long unitSize[SignalType::SIGNALTYPE_LAST] = {outCount, outCount, outCount}; 
	ReleaseMutex(_gWaveXY[sAreaId].bufferHandle);

	if(FALSE == ResetGGalvoWaveformParam(&_gWaveXY[sAreaId], unitSize, pockelsCount, _wParams.digLineSelect))
		return FALSE;

	if(TRUE == GetMutex(_gWaveXY[sAreaId].bufferHandle))
	{
		//build pockels with idle:
		if(0 < pockelsCount)
		{
			pockelsCount = 0;
			for(long k = 0; k< MAX_GG_POCKELS_CELL_COUNT;k++)
			{
				if(TRUE == _wParams.pockelsLineEnable[k])
				{
					for (int i = 0; i < _gWaveXY[sAreaId].unitSize[SignalType::ANALOG_POCKEL]; i++)
					{
						_gWaveXY[sAreaId].GalvoWaveformPockel[i + pockelsCount * _gWaveXY[sAreaId].unitSize[SignalType::ANALOG_POCKEL]] = _wParams.pockelsIdlePower[k];
					}
					pockelsCount++;
				}
			}
		}

		//build digital lines:
		memset(_gWaveXY[sAreaId].DigBufWaveform, 0x0, _gWaveXY[sAreaId].digitalSize * sizeof(unsigned char));

		//build XY travel:
		if (Nd > 0)
		{
			for (int id = 0; id < Nd; id++)
			{	
				_gWaveXY[sAreaId].GalvoWaveformXY[2*id] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (*(srcDstVxy) + id * xDirection * (float)std::min(deltaX_volt, DistanceVx / Nd))));
				_gWaveXY[sAreaId].GalvoWaveformXY[2*id+1] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (*(srcDstVxy+1) + id * yDirection * (float)std::min(deltaY_volt, DistanceVy / Nd))));
			}
		}
		if (dV_sub_X != 0 || dV_sub_Y != 0)
		{
			_gWaveXY[sAreaId].GalvoWaveformXY[2*Nd] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (_gWaveXY[sAreaId].GalvoWaveformXY[2*(Nd-1)] + dV_sub_X)));
			_gWaveXY[sAreaId].GalvoWaveformXY[2*Nd+1] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (_gWaveXY[sAreaId].GalvoWaveformXY[2*(Nd-1)+1] + dV_sub_Y)));
		}

		ReleaseMutex(_gWaveXY[sAreaId].bufferHandle);
	}
	return TRUE;
}

// get waveform to travel between two (interleaved X,Y) positions with provided unit step size
double* ImageWaveformBuilder::GetTravelWaveform(double stepSize, long outputInterleave, double* posFromXYToXY, long& count)
{
	long ret = FALSE;
	SAFE_DELETE_MEMORY(_outputXY);
	uint64_t val = 0;

	if(GetMutex(_gWaveXY[_scanAreaId].bufferHandle))
	{
		_gWaveXY[_scanAreaId].stepVolt = stepSize;
		ret = BuildTravelToStart(0.0, posFromXYToXY, val);
		if(TRUE == ret)
		{
			_outputXY = (double*)realloc(_outputXY, _gWaveXY[_scanAreaId].analogXYSize);
			if (outputInterleave)
			{
				SAFE_MEMCPY(_outputXY, _gWaveXY[_scanAreaId].analogXYSize, _gWaveXY[_scanAreaId].GalvoWaveformXY);
			}
			else
			{
				for (uint64_t i = 0; i < val; i++)
				{
					_outputXY[i] = _gWaveXY[_scanAreaId].GalvoWaveformXY[2*i];
					_outputXY[val + i] = _gWaveXY[_scanAreaId].GalvoWaveformXY[2*i + 1];
				}
			}
		}

		ReleaseMutex(_gWaveXY[_scanAreaId].bufferHandle);
	}
	count = static_cast<long>(val);
	return _outputXY;
}

///	***************************************** <summary> Build Polyline		</summary>	********************************************** ///

double ImageWaveformBuilder::AngleInTwoVectors(double Vx1, double Vy1,double Vx2, double Vy2)
{
	double thetaRad = abs(abs(atan2(Vy2,Vx2)) - abs(atan2(Vy1,Vx1)));
	return (180*thetaRad/PI);
}

long ImageWaveformBuilder::BuildPolyLine()
{
	long ret = TRUE;
	if(0 == _lineSegs.size())
		return FALSE;

	//forward scan:
	double* wFwdGalvoX = (double*) malloc(_galvoFwdSamplesPerLine * sizeof(double));
	double* wFwdGalvoY = (double*) malloc(_galvoFwdSamplesPerLine * sizeof(double));
	unsigned char* wFwdLineTrig = (unsigned char*) malloc(_galvoFwdSamplesPerLine * sizeof(unsigned char));

	double padding = _deltaX_Volt * (2.0 * _galvoSamplesPadding / PI);
	double paddingStepSize = PI / _galvoSamplesPadding / 2.0;
	Cartesian2D startPt = Cartesian2D(std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( + padding * _lineSegs[0].GetUnitVector(-1).First() + _lineSegs[0].GetStartPoint().First()))), 
		std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( + padding * _lineSegs[0].GetUnitVector(-1).Second() + _lineSegs[0].GetStartPoint().Second()))));
	Cartesian2D curPt = Cartesian2D();

	//calculate the first line waveform, the rest are just replica
	//left-right:
	unsigned long k = 0;
	for(unsigned long i = 0; i < 1; i++)
	{
		//accelerate:
		for (long j=0; j < _galvoSamplesPadding; j++)
		{
			wFwdGalvoX[k]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( + padding * _lineSegs[0].GetUnitVector(-1).First() * cos ( paddingStepSize * static_cast<double>(j)) + _lineSegs[0].GetStartPoint().First())));
			wFwdGalvoY[k]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( + padding * _lineSegs[0].GetUnitVector(-1).Second() * cos ( paddingStepSize * static_cast<double>(j)) + _lineSegs[0].GetStartPoint().Second())));
			wFwdLineTrig[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
		}
		//linear:
		unsigned long p = 0;
		for (long j=0; j < static_cast<long>(_lineSegs.size()); j++)
		{
			long lineSegStepCnt = static_cast<long>(ceil(_lineSegs[j].GetLineLength()/_deltaX_Volt));
			long lineSegStepCntFloor = static_cast<long>(floor(_lineSegs[j].GetLineLength()/_deltaX_Volt));
			for(long m=0; m < lineSegStepCntFloor; m++)
			{
				wFwdGalvoX[k]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( _lineSegs[j].GetUnitVector(1).First()*(_deltaX_Volt * static_cast<double>(m)) + _lineSegs[j].GetStartPoint().First())));
				wFwdGalvoY[k]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( _lineSegs[j].GetUnitVector(1).Second()*(_deltaX_Volt * static_cast<double>(m)) + _lineSegs[j].GetStartPoint().Second())));
				wFwdLineTrig[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				p++;
			}
			if(lineSegStepCntFloor != lineSegStepCnt)
			{
				wFwdGalvoX[k]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( _lineSegs[j].GetEndPoint().First())));
				wFwdGalvoY[k]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( _lineSegs[j].GetEndPoint().Second())));
				wFwdLineTrig[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				p++;
			}
		}
		//padding for sampling:
		for(long j=0; j < (_galvoSamplesEffective-static_cast<long>(p)); j++)
		{
			wFwdGalvoX[k]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( _lineSegs[_lineSegs.size()-1].GetUnitVector(1).First()*(_deltaX_Volt * static_cast<double>(j)) + _lineSegs[_lineSegs.size()-1].GetEndPoint().First())));
			wFwdGalvoY[k]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( _lineSegs[_lineSegs.size()-1].GetUnitVector(1).Second()*(_deltaX_Volt * static_cast<double>(j)) + _lineSegs[_lineSegs.size()-1].GetEndPoint().Second())));
			wFwdLineTrig[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
		}
		curPt = Cartesian2D(wFwdGalvoX[k-1],wFwdGalvoY[k-1]);
		//decelerate:
		for (long j=0; j < _galvoSamplesPadding; j++)
		{
			wFwdGalvoX[k]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( - padding * _lineSegs[_lineSegs.size()-1].GetUnitVector(1).First() * cos ( PI/2.0 + paddingStepSize * static_cast<double>(j)) + curPt.First())));
			wFwdGalvoY[k]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( - padding * _lineSegs[_lineSegs.size()-1].GetUnitVector(1).Second() * cos ( PI/2.0 + paddingStepSize * static_cast<double>(j)) + curPt.Second())));
			wFwdLineTrig[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
		}
	}

	///flipscan:
	if (1 != _wParams.verticalScanDirection)
	{
		for(long i = 0; i < _galvoFwdSamplesPerLine; i++)
		{
			double tempY = wFwdGalvoY[i];
			wFwdGalvoY[i]=tempY*_wParams.verticalScanDirection;
		}
	}

	///If the scan area is different from 0 do a matrix rotation to the left-right waveform
	///before the right-left or offset is added
	if (0 != _wParams.scanAreaAngle)
	{
		for(long i = 0; i < _galvoFwdSamplesPerLine; i++)
		{
			double tempX = wFwdGalvoX[i];
			double tempY = wFwdGalvoY[i];
			wFwdGalvoX[i]=tempX*cos(_wParams.scanAreaAngle) - tempY*sin(_wParams.scanAreaAngle);
			wFwdGalvoY[i]=tempX*sin(_wParams.verticalScanDirection*_wParams.scanAreaAngle) + tempY*cos(_wParams.verticalScanDirection*_wParams.scanAreaAngle);
		}
	}

	//right-left:
	double* wBwdGalvoX = (double*) malloc(_galvoBwdSamplesPerLine * sizeof(double));
	double* wBwdGalvoY = (double*) malloc(_galvoBwdSamplesPerLine * sizeof(double));

	//always one way scan since retrace may not follow trace
	for(long i = 0; i < _galvoBwdSamplesPerLine; i++)
	{
		wBwdGalvoX[i] = wFwdGalvoX[_galvoBwdSamplesPerLine-1-i];
		wBwdGalvoY[i] = wFwdGalvoY[_galvoBwdSamplesPerLine-1-i];
	}

	//build digital lines, backward scan are all DIGITAL_LINE_LOW.
	memset(_gParams[_scanAreaId].DigBufWaveform, DIGITAL_LINE_LOW, _gParams[_scanAreaId].digitalSize * sizeof(unsigned char));

	k = 0;
	for (unsigned long i = 0; i < _forwardLines; i++)
	{
		//trace:
		for(unsigned long j=0;j < static_cast<unsigned long>(_galvoFwdSamplesPerLine);j++)
		{
			//left-to-right: (line-frame-pockelDig)
			_gParams[_scanAreaId].DigBufWaveform[k++] = wFwdLineTrig[j];
			_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
			if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
				_gParams[_scanAreaId].DigBufWaveform[k++] = wFwdLineTrig[j];				
		}
		//retrace:
		for(unsigned long j=0;j < static_cast<unsigned long>(_galvoBwdSamplesPerLine);j++)
		{
			//right-to-left: (line-frame-pockelDig)
			_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
			_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
			if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
		}
	}	//backward scan are all DIGITAL_LINE_LOW.

	//copy galvo buffers:
	k = 0;
	for (unsigned long i = 0; i < _forwardLines; i++)
	{
		//forward scan:
		for(unsigned long j=0;j<static_cast<unsigned long>(_galvoFwdSamplesPerLine);j++)
		{
			//left-to-right:
			_gParams[_scanAreaId].GalvoWaveformXY[k++] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (_galvoXOffset + wFwdGalvoX[j]* -1.0)));
			_gParams[_scanAreaId].GalvoWaveformXY[k++] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (_galvoYOffset + wFwdGalvoY[j])));
		}
		for(unsigned long j=0;j<static_cast<unsigned long>(_galvoBwdSamplesPerLine);j++)
		{
			//right-to-left:
			_gParams[_scanAreaId].GalvoWaveformXY[k++] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (_galvoXOffset + wBwdGalvoX[j]* -1.0)));
			_gParams[_scanAreaId].GalvoWaveformXY[k++] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (_galvoYOffset + wBwdGalvoY[j])));
		}
	}

	//backward scan:
	for(long j=0;j<_lineFlybackLength;j++)
	{
		_gParams[_scanAreaId].GalvoWaveformXY[k++] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (_galvoXOffset + wFwdGalvoX[0]* -1.0)));
		_gParams[_scanAreaId].GalvoWaveformXY[k++] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (_galvoYOffset + wFwdGalvoY[0])));
	}

	//copy line buffers:

	//release mem:
	SAFE_DELETE_MEMORY(wFwdGalvoX);
	SAFE_DELETE_MEMORY(wFwdGalvoY);
	SAFE_DELETE_MEMORY(wFwdLineTrig);

	SAFE_DELETE_MEMORY(wBwdGalvoX);
	SAFE_DELETE_MEMORY(wBwdGalvoY);
	return ret;
}

long ImageWaveformBuilder::CalculatePolyLinePixelX()
{
	long linearStepCnt = 0;
	for(int i=0;i<static_cast<int>(_lineSegs.size());i++)
	{
		linearStepCnt += static_cast<long>(ceil(_lineSegs[i].GetLineLength()/_deltaX_Volt));
	}

	long remainder = (linearStepCnt+1) % _daqSampleSize;

	//Only have  residual steps when the remainder is not zero
	//when the remainder is not zero it means that the roi will not fit in the current pixel density
	long residualSteps = (0 == remainder) ? 0 : (_daqSampleSize - remainder);

	return (linearStepCnt + 1 + residualSteps);
}

long ImageWaveformBuilder::VerifyPolyLine(std::vector<long> Ptx, std::vector<long> Pty, long fieldSize, double field2Volts, double fieldScaleFineX, double fieldScaleFineY, long PixelY, long &PixelX)
{
	long ret = TRUE;
	int theta = THRESHOLD_ANGLE;
	size_t PtNum = Ptx.size();
	_lineSegs.clear();

	//check point coordinate pairs:
	if(Ptx.size() != Pty.size())
	{
		return FALSE;
	}
	for(long i=2; i<static_cast<long>(PtNum); i++)
	{
		double Vx2 = Ptx[i]-Ptx[i-1];
		double Vy2 = Pty[i]-Pty[i-1];
		double Vx1 = Ptx[i-1]-Ptx[i-2];
		double Vy1 = Pty[i-1]-Pty[i-2];

		//return false for acute angle:
		if(theta < AngleInTwoVectors(Vx1,Vy1,Vx2,Vy2))
		{
			//return FALSE;
		}
	}

	//initialize params:
	double fieldX_volt = fieldSize * field2Volts * fieldScaleFineX;
	double fieldY_volt = fieldSize * field2Volts * fieldScaleFineY * PixelY / PixelX;
	_deltaX_Volt = fieldX_volt/PixelX;	
	std::vector<double> volt_X = ConvertPixelToVolt(Ptx,-(fieldX_volt/2),_deltaX_Volt);
	std::vector<double> volt_Y = ConvertPixelToVolt(Pty,-(fieldY_volt/2),_deltaX_Volt);
	for(int i=1;i<volt_Y.size();i++)
	{
		Cartesian2D firstPt = Cartesian2D(volt_X[i-1],volt_Y[i-1]);
		Cartesian2D secondPt = Cartesian2D(volt_X[i],volt_Y[i]);
		LineSegment tmpLineSeg = LineSegment(firstPt,secondPt);
		_lineSegs.push_back(tmpLineSeg);
	}

	PixelX = CalculatePolyLinePixelX();

	return ret;
}

std::vector<double> ImageWaveformBuilder::ConvertPixelToVolt(std::vector<long> vertices,double offsetVolt,double deltaX_volt)
{
	std::vector<double> voltVec;

	for(int i=0;i<static_cast<int>(vertices.size());i++)
	{
		voltVec.push_back(deltaX_volt*vertices[i] + offsetVolt);
	}
	return voltVec;
}

///	***************************************** <summary> Build Rectangle		</summary>		********************************************** ///

// _wParams must be pre-defined before building of rectangle image waveform, interleaved XY
long ImageWaveformBuilder::BuildGGFrameWaveformXY(void)
{
	long ret = TRUE;

	///*********************************************************************************//
	///************************		Build XY Galvo Waveform		************************//
	///*********************************************************************************//

	/// Calculate the waveform of raster X
	/// X is the fast axis, doing the sawtooth motion through the whole frame.
	double galvoXFwdStep = _fieldX_volt/(double) (_galvoSamplesEffective);
	double galvoXBwdStep = _fieldX_volt/(double) (_galvoSamplesEffective);

	/***** Method I - Sinusoidal Padding  *************/
	// Calculate the padded waveform (sinusoidal) amplitude to both ends of the linear part
	double vPAD= galvoXFwdStep * 2 * _galvoSamplesPadding / PI;
	double padStep = PI / _galvoSamplesPadding / 2.0;

	//calculate the first line waveform, the rest are just replica
	for (unsigned long i = 0, k = 0; i < 1; i++)
	{
		for (long j=0; j < _galvoSamplesPadding; j++)
			_pGalvoWaveformX[k++]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( - vPAD * cos ( padStep * (double)j) - _fieldX_volt / 2)));
		for (long j=0; j < _galvoSamplesEffective; j++)
			_pGalvoWaveformX[k++]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( galvoXFwdStep * (double)j - _fieldX_volt / 2)));
		for (long j=0; j < _galvoSamplesPadding *2; j++)
			_pGalvoWaveformX[k++]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( - vPAD * cos ( PI/2 + padStep * (double)j) + _fieldX_volt / 2)));
		for (long j=0; j < _galvoSamplesEffective ; j++)
			_pGalvoWaveformX[k++]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( _fieldX_volt / 2 - galvoXBwdStep * (double)j )));
		for (long j=0; j < _galvoSamplesPadding; j++)
			_pGalvoWaveformX[k++]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( - vPAD * cos ( 1.5 * PI + padStep * (double)j) - _fieldX_volt / 2)));
	}
	/*********** End of Method I ***************/

	//If galvoEnable is false (meaning line scan) there is no need for a backscan
	//only a need to keep the frame trigger down for a few time points (LINE_FRAMETRIGGER_TIMEPOINTS)
	if(FALSE == _wParams.galvoEnable)
	{
		///replicate the forward scan and keep
		///the X stationary on the backward scan
		long k = 0;
		for (unsigned long i = 0; i < _forwardLines; i++)
		{
			for (long j = 0; j < _galvoSamplesPerLine; j++)
			{
				_pGalvoWaveformX[k++] = _pGalvoWaveformX[j];
			}			
		}
		for (long j = 0; j < _lineFlybackLength; j++)
		{
			if (k >= _galvoDataLength)
			{
				break;
			}
			_pGalvoWaveformX[k++] = _pGalvoWaveformX[0];
		}
	}
	else
	{
		for (unsigned long i = 0, k = 0; i < _overallLines; i++)
		{
			if (_forwardLines > i)
			{
				for (long j = 0; j < _galvoSamplesPerLine; j++)
				{
					_pGalvoWaveformX[k++] = _pGalvoWaveformX[j];
				}
			}
			else
			{
				for (long j = 0; j < _galvoSamplesPerLine; j++)
				{
					_pGalvoWaveformX[k++] = _pGalvoWaveformX[0];
				}
			}
		}
	}	

	/// Calculate the waveform of Y, Y is the slow axis.  Keeps moving at a constant pace to accomplish a sawtooth xy motion
	double galvoYFwdStep = (0.0 != (double) _forwardLines) ? _fieldY_volt/(double) _forwardLines : (double)0.0;
	double galvoYBwdStep = (0.0 != (double)(_backwardLines*_galvoSamplesPerLine)) ? _fieldY_volt/(double) (_backwardLines*_galvoSamplesPerLine) : (double)0.0;

	double vPadY= galvoYFwdStep / 4.0;


	//If galvoEnable is false (meaning line scan) there is no need for a backscan
	//only a need to keep the frame trigger down for a few time points (_lineFlybackLength)
	if(FALSE == _wParams.galvoEnable)
	{
		// Keep Y stationary for retracing as tracing, _forwardLines == 1 && _fieldY_volt == 0
		for (long i = 0, k = 0; i < _galvoDataLength; i++)
		{
			_pGalvoWaveformY[k++] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (double)0.0));
		}
	}
	else
	{
		// Method I sinuoid for Y waveform
		for (unsigned long i = 0, k = 0; i < _forwardLines; i++)
		{
			for (long j = 0; j < _galvoSamplesPadding; j++)
				_pGalvoWaveformY[k++] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE,  _wParams.verticalScanDirection * (galvoYFwdStep * (double)i + vPadY * (sin((double)j * padStep)) - _fieldY_volt / 2.0) ));
			for (long j = 0; j < _galvoSamplesEffective; j++)
				_pGalvoWaveformY[k++] = _pGalvoWaveformY[k - 1];
			for (long j = 0; j < _galvoSamplesPadding * 2; j++)
				_pGalvoWaveformY[k++] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE,  _wParams.verticalScanDirection * (galvoYFwdStep * (double)i + vPadY * (2 + (sin((double)j * padStep - PI * 0.5)))  - _fieldY_volt / 2.0) ));
			for (long j = 0; j < _galvoSamplesEffective ; j++)
				_pGalvoWaveformY[k++] = _pGalvoWaveformY[k - 1];
			for (long j = 0; j < _galvoSamplesPadding; j++)
				_pGalvoWaveformY[k++] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE,  _wParams.verticalScanDirection * (galvoYFwdStep * (double)i + vPadY * (3 + sin((double)j * padStep)) - _fieldY_volt / 2.0) ));
		}
		// Backward Y scan
		for (unsigned long i = 0, k = _galvoDataForward; i < _backwardLines * _galvoSamplesPerLine; i++)
		{
			_pGalvoWaveformY[k++] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE,  _wParams.verticalScanDirection * (_fieldY_volt / 2.0 - galvoYBwdStep * (double)i)));
		}
	}	

	return ret;
}

// _wParams must be pre-defined before building of interleave rectangle scan waveform
long ImageWaveformBuilder::BuildGGInterleaveScanXY(void)
{
	long ret = TRUE;

	/***** Method I - Sinusoidal Padding  *************/
	/// Calculate the waveform of raster X
	/// X is the fast axis, doing the sawtooth motion through the whole frame.
	double galvoXFwdStep = _fieldX_volt/(double) (_galvoSamplesEffective);
	double galvoXBwdStep = _fieldX_volt/(double) (_galvoSamplesEffective);
	/// Calculate the waveform of Y, Y is the slow axis.  Keeps moving at a constant pace to accomplish a sawtooth xy motion
	double galvoYFwdStep = _fieldY_volt/(double) _forwardLines;
	double galvoYFwdStepOffset = 0;

	// Calculate the padded waveform (sinusoidal) amplitude to both ends of the linear part
	double vPadX = galvoXFwdStep * 2 * _galvoSamplesPadding / PI;
	double xPadStep = PI / _galvoSamplesPadding / 2.0;
	double vPadY = galvoYFwdStep / 4.0;

	//calculate the first 2 line waveform, the rest are just replica
	unsigned long i = 0; //line index
	unsigned long kx = 0, ky = 0; //data index
	unsigned long f = 0; //frame index
	for (; f < 2; f++)
	{
		//Method I sinuoid for X waveform for 1 line
		for (i = 0; i < 1; i++)
		{
			for (long j=0; j < _galvoSamplesPadding; j++)
				_pGalvoWaveformX[kx++]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( - vPadX * cos ( xPadStep * (double)j) - _fieldX_volt / 2)));
			for (long j=0; j < _galvoSamplesEffective; j++)
				_pGalvoWaveformX[kx++]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( galvoXFwdStep * (double)j - _fieldX_volt / 2)));
			for (long j=0; j < _galvoSamplesPadding * 2; j++)
				_pGalvoWaveformX[kx++]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( - vPadX * cos ( PI/2 + xPadStep * (double)j) + _fieldX_volt / 2)));
			for (long j=0; j < _galvoSamplesEffective ; j++)
				_pGalvoWaveformX[kx++]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( _fieldX_volt / 2 - galvoXBwdStep * (double)j )));
			for (long j=0; j < _galvoSamplesPadding; j++)
				_pGalvoWaveformX[kx++]=std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, ( - vPadX * cos ( 1.5 * PI + xPadStep * (double)j) - _fieldX_volt / 2)));
		}
		//replicate first line for the rest
		for (; i < _forwardLines; i++)
		{
			for (long j = 0; j < _galvoSamplesPerLine; j++)
			{
				_pGalvoWaveformX[kx++] = _pGalvoWaveformX[j];
			}
		}
		//backward line scan X
		for (unsigned long j = 0; j < _backwardLines * _galvoSamplesPerLine; j++)
		{
			_pGalvoWaveformX[kx++] = _pGalvoWaveformX[0];
		}

		// Method I sinuoid for Y waveform
		for (i = 0; i < _forwardLines; i++)
		{
			for (long j = 0; j < _galvoSamplesPadding; j++)
				_pGalvoWaveformY[ky++] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE,  _wParams.verticalScanDirection * (galvoYFwdStepOffset + galvoYFwdStep * (double)i + vPadY * (sin((double)j * xPadStep)) - _fieldY_volt / 2.0) ));
			for (long j = 0; j < _galvoSamplesEffective; j++)
				_pGalvoWaveformY[ky++] = _pGalvoWaveformY[ky - 1];
			for (long j = 0; j < _galvoSamplesPadding * 2; j++)
				_pGalvoWaveformY[ky++] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE,  _wParams.verticalScanDirection * (galvoYFwdStepOffset + galvoYFwdStep * (double)i + vPadY * (2 + (sin((double)j * xPadStep - PI * 0.5)))  - _fieldY_volt / 2.0) ));
			for (long j = 0; j < _galvoSamplesEffective ; j++)
				_pGalvoWaveformY[ky++] = _pGalvoWaveformY[ky - 1];
			for (long j = 0; j < _galvoSamplesPadding; j++)
				_pGalvoWaveformY[ky++] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE,  _wParams.verticalScanDirection * (galvoYFwdStepOffset + galvoYFwdStep * (double)i + vPadY * (3 + sin((double)j * xPadStep)) - _fieldY_volt / 2.0) ));
		}
		//backward line scan
		//alternating start offset based on even or odd frames
		galvoYFwdStepOffset = (0 == (f % 2)) ? galvoYFwdStep / 4 : 0;
		double galvoYBwdRange = (0 == (f % 2)) ? (_fieldY_volt - galvoYFwdStep / 4) : (_fieldY_volt + galvoYFwdStep / 4);
		double galvoYBwdStep = galvoYBwdRange/(double) (_backwardLines*_galvoSamplesPerLine);

		for (unsigned long j = 0; j < _backwardLines * _galvoSamplesPerLine; j++)
		{
			_pGalvoWaveformY[ky++] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE,  _wParams.verticalScanDirection * (_fieldY_volt / 2.0 - galvoYBwdStep * (double)j)));
		}
		//only build one frame if requested
		if(1 >= _wParams.numFrame)
			return ret;
	}
	return ret;
}

// build interleaved line, frame, pockelsDig triggers
long ImageWaveformBuilder::BuildGGFrameDigiLines()
{
	long ret = TRUE;

	///Calculate the arrays for digital output as line triggers and frame triggers.
	///[GalvoGalvo] Use P0.6 as Line Trigger out, P0.5 as Sample Trigger out, and P0.7 and Frm Trigger out

	//keep the digitial line high for data capture forward of the frame
	//dealy the triggering signal to compensate for the reaction time of the galvo

	//pockels blanking:
	long blankSampleCount = static_cast<long>(_wParams.pockelsLineBlankingPercentage[0] * _galvoSamplesEffective);
	long lineBufSize = sizeof(unsigned char) * _wParams.digLineSelect * _galvoSamplesPerLine;

	if (_wParams.scanMode == TWO_WAY_SCAN) //two_way scan trigger out (line, frame)
	{
		//calculate the first forward line, the rest are just replica
		for (unsigned long i = 0, k = 0; i < 1; i++)
		{
			//first padding
			for (long j=0; j < _galvoSamplesPadding; j++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);			//line buffer
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);			//frame buffer
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);		//Pockels Dig buffer
			}
			//forward of the line, up to blank
			for (long j=0; j < blankSampleCount; j++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
			}

			//forward of the line
			for (long j=0; j < _galvoSamplesEffective - blankSampleCount*2; j++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
			}

			//forward of the line, another side of blank
			for (long j=0; j < blankSampleCount; j++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
			}

			//turn around padding
			for (long j=0; j < _galvoSamplesPadding * 2; j++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
			}
			//backward of the line, up to blank
			for (long j=0; j < blankSampleCount; j++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
			}
			//backward of the line
			for (long j=0; j < _galvoSamplesEffective - blankSampleCount*2; j++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
			}
			//backward of the line, another side of blank
			for (long j=0; j < blankSampleCount; j++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
			}
			//end padding
			for (long j=0; j < _galvoSamplesPadding; j++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
			}
		}
		//replicate the first line for the rest of forward lines
		for (unsigned long i = 1; i < _forwardLines; i++)
		{
			SAFE_MEMCPY(_gParams[_scanAreaId].DigBufWaveform + i * lineBufSize, lineBufSize, _gParams[_scanAreaId].DigBufWaveform);
		}

		//If galvoEnable is false (meaning line scan) there is no need for a backscan
		//only a need to keep the frame trigger down for a few time points (_lineFlybackLength)
		if(FALSE == _wParams.galvoEnable)
		{
			for (long i = 0, k = _wParams.digLineSelect*_galvoDataForward; i < _lineFlybackLength; i++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
			}
		}
		else
		{
			///backward of the frame
			for (unsigned long i = 0, k = _wParams.digLineSelect*_galvoDataForward; i < _backwardLines; i++)
			{
				//first padding
				for (long j=0; j < _galvoSamplesPadding; j++)
				{
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
						_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				}

				//forward of the line
				for (long j=0; j < _galvoFwdSamplesPerLine - _galvoSamplesPadding*2; j++)
				{
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
						_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				}

				//turn around padding
				for (long j=0; j < _galvoSamplesPadding *2; j++)
				{
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
						_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				}
				//backward of the line
				for (long j=0; j < _galvoBwdSamplesPerLine - _galvoSamplesPadding*2; j++)
				{
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
						_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				}

				//end padding
				for (long j=0; j < _galvoSamplesPadding; j++)
				{
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
						_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				}
			}
		}			
	}
	else  //one way scan trigger out (line, frame trigger)
	{
		//calculate the first forward line, the rest are just replica
		for (unsigned long i = 0, k = 0; i < 1; i++)
		{
			//first padding
			for (long j=0; j < _galvoSamplesPadding; j++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
			}
			//forward of the line, up to blank
			for (long j=0; j < blankSampleCount; j++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
			}
			//forward of the line
			for (long j=0; j < _galvoSamplesEffective - blankSampleCount*2; j++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
			}
			//forward of the line, another side of blank
			for (long j=0; j < blankSampleCount; j++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
			}
			//turn around padding
			for (long j=0; j < _galvoSamplesPadding *2; j++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
			}
			//backward of the line
			for (long j=0; j < _galvoBwdSamplesPerLine - _galvoSamplesPadding*2; j++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
			}

			//end padding
			for (long j=0; j < _galvoSamplesPadding; j++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_HIGH);
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
			}
		}
		//replicate the first line for the rest of forward lines
		for (unsigned long i = 1; i < _forwardLines; i++)
		{
			SAFE_MEMCPY(_gParams[_scanAreaId].DigBufWaveform + i * lineBufSize, lineBufSize, _gParams[_scanAreaId].DigBufWaveform);
		}

		//If galvoEnable is false (meaning line scan) there is no need for a backscan
		//only a need to keep the frame trigger down for a few time points (_lineFlybackLength)
		if(FALSE == _wParams.galvoEnable)
		{
			for (long i = 0, k = _wParams.digLineSelect*_galvoDataForward; i < _lineFlybackLength; i++)
			{
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
			}
		}
		else
		{
			///backward of the frame
			for (unsigned long i = 0, k = _wParams.digLineSelect*_galvoDataForward; i < _backwardLines; i++)
			{

				//first padding
				for (long j=0; j < _galvoSamplesPadding; j++)
				{
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
						_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				}

				//forward of the line
				for (long j=0; j < _galvoFwdSamplesPerLine - _galvoSamplesPadding*2; j++)
				{
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
						_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				}

				//turn around padding
				for (long j=0; j < _galvoSamplesPadding *2; j++)
				{
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
						_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				}
				//backward of the line
				for (long j=0; j < _galvoBwdSamplesPerLine - _galvoSamplesPadding*2; j++)
				{
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
						_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				}
				//end padding
				for (long j=0; j < _galvoSamplesPadding; j++)
				{
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
					if(Constants::MAX_IMG_DIG_LINE_COUNT == _wParams.digLineSelect)
						_gParams[_scanAreaId].DigBufWaveform[k++] = static_cast<unsigned char>(DIGITAL_LINE_LOW);
				}
			}
		}
	}
	return ret;
}

// build waveform based on SetWaveformGenParams, receive start location and total length.
// [Note]: must SetWaveformGenParams before this function
long ImageWaveformBuilder::BuildImageWaveform(double* startXY, long* countPerCallback, uint64_t* total)
{
	long ret = FALSE;
	long pockelsCount = GetPockelsCount();

	//setup global parameters based on area mode
	if (FALSE == SetupParamsForArea())
		return ret;

	//prepare Galvo X,Y driving waveform
	_pGalvoWaveformX = (double *) realloc(_pGalvoWaveformX, _galvoDataLength * sizeof(double));
	if(NULL == _pGalvoWaveformX)
		return ret;
	_pGalvoWaveformY = (double *) realloc(_pGalvoWaveformY, _galvoDataLength * sizeof(double));
	if(NULL == _pGalvoWaveformY)
		return ret;

	if(GetMutex(_gParams[_scanAreaId].bufferHandle))
	{
		switch ((ICamera::LSMAreaMode)_wParams.areaMode)
		{
		case ICamera::LSMAreaMode::POLYLINE:
			if((FALSE == BuildPolyLine()) || (FALSE == BuildGGFrameDigiLines()) || (FALSE == BuildGGLinePockels()))
				goto BUILD_WAVEFORM_ERROR;
			break;
		case ICamera::LSMAreaMode::SQUARE:
		case ICamera::LSMAreaMode::RECTANGLE:
		case ICamera::LSMAreaMode::LINE:
			//interleave scan not applied to line scan in SetupParamsForArea
			if((_wParams.interleaveScan) && (_wParams.galvoEnable))
			{
				if(FALSE == BuildGGInterleaveScanXY())
					goto BUILD_WAVEFORM_ERROR;
			}
			else
			{
				if(FALSE == BuildGGFrameWaveformXY())
					goto BUILD_WAVEFORM_ERROR;
			}
			if((FALSE == ProcGGFrameWaveformXY()) || 
				(FALSE == BuildGGFrameDigiLines()) || 
				(FALSE == BuildGGLinePockels()))
				goto BUILD_WAVEFORM_ERROR;
			break;
		}

		//set start position
		SAFE_MEMCPY(startXY, 2 * sizeof(double) ,_gParams[_scanAreaId].GalvoWaveformXY);
		ResetCounter();

		//[total]: (0)initial travel, (1)count per frame * frames, (2)end patch:
		for (int i = 0; i < static_cast<int>(SignalType::SIGNALTYPE_LAST); i++)
		{
			double unitSizeRatio = ((_wParams.interleaveScan) && (1 < _wParams.numFrame)) ? 0.5 : 1.0;
			switch ((SignalType)i)
			{
			case SignalType::ANALOG_XY:
				total[i] = GetTotalCount(countPerCallback[i], (SignalType)i, 0, static_cast<uint64_t>(_gParams[_scanAreaId].unitSize[i] * unitSizeRatio * _wParams.numFrame));
				break;
			case SignalType::ANALOG_POCKEL:
				//Pockels unitSize is per line, total needs to consider lines x frames
				//no travel section since signal controlled by line trigger, 
				//idle section should be at the very end after all scan areas
				total[i] = (0 < pockelsCount) ? GetTotalCount(countPerCallback[i], (SignalType)i, 0, _gParams[_scanAreaId].unitSize[i] * _forwardLines * _lineFactor * _wParams.numFrame) : 0;
				break;
			case SignalType::DIGITAL_LINES:
				total[i] = GetTotalCount(countPerCallback[i], (SignalType)i, 0, _gParams[_scanAreaId].unitSize[i] * _wParams.numFrame);
				break;
			}
		}

		SAFE_DELETE_MEMORY(_pGalvoWaveformX);
		SAFE_DELETE_MEMORY(_pGalvoWaveformY);
		ReleaseMutex(_gParams[_scanAreaId].bufferHandle);
		return TRUE;
	}
	return ret;

BUILD_WAVEFORM_ERROR:
	SAFE_DELETE_MEMORY(_pGalvoWaveformX);
	SAFE_DELETE_MEMORY(_pGalvoWaveformY);
	ReleaseMutex(_gParams[_scanAreaId].bufferHandle);
	return FALSE;
}

// build waveform including travel to start position from currentVxy. 
// [Note]: must SetWaveformGenParams before this function
long ImageWaveformBuilder::BuildImageWaveformFromStart(long rebuild, double stepVolts, double* currentVxy, long* countPerCallback, uint64_t* total)
{
	//return if no current location:
	if(NULL == currentVxy)
		return FALSE;

	//try re-build waveform:
	if(rebuild)
	{
		double startXY[2];
		if(FALSE == BuildImageWaveform(startXY, countPerCallback, total))
			return FALSE;
	}

	//build travel to first XY location:
	if (FALSE == BuildTravelToStartImagePos(0, stepVolts, currentVxy))
		return FALSE;

	//set counters:
	if(FALSE == GetMutex(_gWaveXY[_scanAreaId].bufferHandle))
		return FALSE;

	if(FALSE == GetMutex(_gParams[_scanAreaId].bufferHandle))
	{
		ReleaseMutex(_gWaveXY[_scanAreaId].bufferHandle);	
		return FALSE;
	}

	//done:
	ResetCounter();
	LogPerformance(L"ImageWaveformBuilder Reset time");

	//[total]: (0)initial travel, (1)count per frame * frames, (2)end patch:
	for (int i = 0; i < static_cast<int>(SignalType::SIGNALTYPE_LAST); i++)
	{
		total[i] = GetTotalCount(countPerCallback[i], (SignalType)i,_gWaveXY[_scanAreaId].unitSize[i], _gParams[_scanAreaId].unitSize[i] * _wParams.numFrame);
	}

	ReleaseMutex(_gWaveXY[_scanAreaId].bufferHandle);	
	ReleaseMutex(_gParams[_scanAreaId].bufferHandle);
	return TRUE;
}

// Process Galvo XY frame waveform, rotate and interleave
long ImageWaveformBuilder::ProcGGFrameWaveformXY(void)
{
	long ret = TRUE;

	///If the scan area is different from 0 do a matrix rotation to the whole waveform
	///before the offset is added
	if (0 != _wParams.scanAreaAngle)
	{
		double angle = (1 == _wParams.verticalScanDirection) ? _wParams.scanAreaAngle : -_wParams.scanAreaAngle;
		for (unsigned long i = 0; i < static_cast<unsigned long>(_galvoDataLength); i++)
		{
			double tempX = _pGalvoWaveformX[i];
			double tempY = _pGalvoWaveformY[i];
			_pGalvoWaveformX[i] = tempX*cos(angle) - tempY*sin(angle);
			_pGalvoWaveformY[i] = tempX*sin(angle) + tempY*cos(angle);
		}
	}

	///Calculate the scan offset (X Y displacement)
	for (long i = 0; i < _galvoDataLength; i++)
	{
		/**** Temporary	Change for roration test****/ // comment the following list out for rotation test
		//_pGalvoWaveformX[i] = max(MIN_AO_VOLTAGE, min(MAX_AO_VOLTAGE, (_galvoXOffset + pGalvoWaveformX[i] * (-1.0) * _imgPtyDll.verticalScanDirection)));
		/**** End of Temporary Change for rotation test ****/
		_pGalvoWaveformX[i] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (_galvoXOffset + _pGalvoWaveformX[i] * (-1.0))));
		_pGalvoWaveformY[i] = std::max(MIN_AO_VOLTAGE, std::min(MAX_AO_VOLTAGE, (_galvoYOffset + _pGalvoWaveformY[i])));
	}

	double programmedXVoltageMin = MAX_AO_VOLTAGE;
	double programmedXVoltageMax = MIN_AO_VOLTAGE;
	double programmedYVoltageMin = MAX_AO_VOLTAGE;
	double programmedYVoltageMax = MIN_AO_VOLTAGE;

	///interleave XY waveform
	long offset = 2;

	for (long i = 0; i < _galvoDataLength; i++)
	{
		_gParams[_scanAreaId].GalvoWaveformXY[offset * i] = _pGalvoWaveformX[i];
		_gParams[_scanAreaId].GalvoWaveformXY[offset * i + 1] = _pGalvoWaveformY[i];

		programmedXVoltageMin = std::min(programmedXVoltageMin, _pGalvoWaveformX[i]);
		programmedXVoltageMax = std::max(programmedXVoltageMax, _pGalvoWaveformX[i]);
		programmedYVoltageMin = std::min(programmedYVoltageMin, _pGalvoWaveformY[i]);
		programmedYVoltageMax = std::max(programmedYVoltageMax, _pGalvoWaveformY[i]);
	}

	StringCbPrintfW(message, _MAX_PATH, L"Voltage ranges X min(%d.%d) max(%d.%d) Y min(%d.%d) max(%d.%d)", 
		static_cast<long>(programmedXVoltageMin), 
		static_cast<long>(abs(1000 * (programmedXVoltageMin - static_cast<long>(programmedXVoltageMin)))),
		static_cast<long>(programmedXVoltageMax),
		static_cast<long>(abs(1000 * (programmedXVoltageMax - static_cast<long>(programmedXVoltageMax)))),
		static_cast<long>(programmedYVoltageMin),
		static_cast<long>(abs(1000 * (programmedYVoltageMin - static_cast<long>(programmedYVoltageMin)))),
		static_cast<long>(programmedYVoltageMax),
		static_cast<long>(abs(1000 * (programmedYVoltageMax - static_cast<long>(programmedYVoltageMax)))));
	LogMessage(message, VERBOSE_EVENT);

	return ret;
}

///	***************************************** <summary> Build Other Types	</summary>		********************************************** ///

long ImageWaveformBuilder::BuildSpiral(long count)
{
	const double	T_Period = 0.1;	//[sec]
	const	int		Envelope = 5;
	//double			ClockRate = _wParams.PixelX * MS_TO_S; //[Hz]
	const double	Amplitude = 5;	//[V]

	uint64_t ClockRate = _gParams[0].ClockRate = _gWaveXY[0].ClockRate;
	_gParams[0].digitalLineCnt = _gWaveXY[0].digitalLineCnt;

	_gParams[0].analogXYSize = static_cast<unsigned long>(2 * count);
	_gParams[0].GalvoWaveformXY = (double*) realloc(_gParams[0].GalvoWaveformXY, _gParams[0].analogXYSize*sizeof(double));
	if(NULL == _gParams[0].GalvoWaveformXY)
	{
		return FALSE;
	}
	_gParams[0].digitalSize = static_cast<unsigned long>(_gWaveXY[0].digitalLineCnt * count);			//dummy signal ...
	_gParams[0].DigBufWaveform = (unsigned char*) realloc(_gParams[0].DigBufWaveform, _gParams[0].digitalSize*sizeof(unsigned char));
	if(NULL == _gParams[0].DigBufWaveform)
	{
		return FALSE;
	}
	std::memset((void*)_gParams[0].DigBufWaveform, 0x0, _gParams[0].digitalSize*sizeof(unsigned char));
	_gParams[0].analogPockelSize = static_cast<long>(count);
	_gParams[0].GalvoWaveformPockel = (double*) realloc(_gParams[0].GalvoWaveformPockel, _gParams[0].analogPockelSize * sizeof(double));	//can be multiple pockels ...
	if(NULL == _gParams[0].GalvoWaveformPockel)
	{
		return FALSE;
	}
	std::memset((void*)_gParams[0].GalvoWaveformPockel, 0x0, _gParams[0].analogPockelSize*sizeof(double));

	for (uint64_t i = 0; i < static_cast<uint64_t>(count); i++)
	{
		_gParams[0].GalvoWaveformXY[2*i] = Amplitude*abs(sin(2*PI*(_countIndex[0][0]+i)/Envelope/T_Period/ClockRate))*cos(2*PI*(_countIndex[0][0]+i)/T_Period/ClockRate);
		_gParams[0].GalvoWaveformXY[2*i + 1] = Amplitude*abs(sin(2*PI*(_countIndex[0][0]+i)/Envelope/T_Period/ClockRate))*sin(2*PI*(_countIndex[0][0]+i)/T_Period/ClockRate);
	}

	_countIndex[0][0] += count;
	LogPerformance(L"ImageWaveformBuilder active BuildSpiral MSec");
	return TRUE;
}

///	***************************************** <summary> Area General Functions </summary>	********************************************** ///

// Build interleaved Pockels Waveform of one line unit size
long ImageWaveformBuilder::BuildGGLinePockels()
{
	long ret = TRUE;
	long beginningOfForwardLine[MAX_GG_POCKELS_CELL_COUNT], endOfForwardLine[MAX_GG_POCKELS_CELL_COUNT];
	double pockelsOnVoltage[MAX_GG_POCKELS_CELL_COUNT];

	if(_wParams.pockelsLineEnable[0])
	{
		for(long k=0; k< MAX_GG_POCKELS_CELL_COUNT; k++)
		{
			//the reference output is only used for pockels1, for everything else use the normal settings
			pockelsOnVoltage[k] = (k > 0 || FALSE == _wParams.useReferenceForPockelsOutput || FALSE == _wParams.pockelsReferenceRequirementsMet) ?
				_wParams.pockelsPower[k] : _wParams.pockelsMaxPower[k];

			//no apply pockels blanking on polyline
			if (ICamera::LSMAreaMode::POLYLINE == (ICamera::LSMAreaMode)_wParams.areaMode)
			{
				beginningOfForwardLine[k] = 0; 
				endOfForwardLine[k] = _pockelsSamplesEffective;
			}
			else
			{
				beginningOfForwardLine[k] = static_cast<long>(_wParams.pockelsLineBlankingPercentage[k] * _pockelsSamplesEffective);
				endOfForwardLine[k] = _pockelsSamplesEffective - static_cast<long>(_wParams.pockelsLineBlankingPercentage[k] * _pockelsSamplesEffective);
			}
		}
		long j = 0;
		for(long i = 0; i<_pockelsSamplesEffective; i++)
		{
			for(long k=0; k< MAX_GG_POCKELS_CELL_COUNT; k++)
			{
				if(_wParams.pockelsLineEnable[k])
				{
					if((i < beginningOfForwardLine[k]) || ((0 == i) && (_wParams.pockelsTurnAroundBlank)))
					{
						//force the first/last element to be the minimum so that
						//the pockels is attenuated between lines if zero is entered
						//for blanking percentage
						_gParams[_scanAreaId].GalvoWaveformPockel[j++] = _wParams.pockelsIdlePower[k];
					}
					else if((i >endOfForwardLine[k]) || (((_pockelsSamplesEffective - 1) == i) && (_wParams.pockelsTurnAroundBlank)))
					{
						_gParams[_scanAreaId].GalvoWaveformPockel[j++] = _wParams.pockelsIdlePower[k];
					}
					else
					{
						_gParams[_scanAreaId].GalvoWaveformPockel[j++] = pockelsOnVoltage[k];
					}
				}
			}
		}
	}
	return ret;
}

long ImageWaveformBuilder::SetupParamsForArea()
{
	long ret = TRUE;
	double theta, offset_Xtheta, offset_Ytheta;
	long pockelsCnt = GetPockelsCount();

	//clock the galvos according to the entered dwell time (non-waveform scan mode)
	long clockRateExternal = static_cast<long>(Constants::MHZ/_wParams.dwellTime);

	switch ((ICamera::LSMAreaMode)_wParams.areaMode)
	{
	case ICamera::LSMAreaMode::POLYLINE:
		//using initialized stepSize(volt),
		_galvoSamplesEffective = CalculatePolyLinePixelX();
		//pockels waveform needs to be shorter in time than the line trigger digital waveform
		//this is to allow time to re-arm before the new trigger comes in
		//For this reason the FRAMETRIGGER_WAITPOINTS is not included in the size of the pockels array
		_pockelsSamplesEffective = static_cast<long>(_wParams.clockRatePockels * _wParams.dwellTime * _galvoSamplesEffective / Constants::US_TO_SEC);

		_galvoXOffset = static_cast<double>(_wParams.offsetX * _wParams.field2Volts + _wParams.fineOffset[0]);
		_galvoYOffset = static_cast<double>(_wParams.verticalScanDirection * _wParams.offsetY * _wParams.field2Volts + _wParams.fineOffset[1]);

		_forwardLines = _wParams.PixelY;
		_backwardLines = _wParams.flybackCycles;
		_overallLines = _forwardLines + _backwardLines;
		break;
	case ICamera::LSMAreaMode::SQUARE:
	case ICamera::LSMAreaMode::RECTANGLE:
	case ICamera::LSMAreaMode::LINE:
		//line scan does not support interleave scan
		if ((ICamera::LSMAreaMode::LINE == (ICamera::LSMAreaMode)_wParams.areaMode)	|| (2 >= _wParams.PixelY))
			_wParams.interleaveScan = 0;

		//Set Galvo Related Line number per frame
		_lineFactor = (ScanMode::TWO_WAY_SCAN == (ScanMode)_wParams.scanMode) ? Constants::GALVO_DATA_POINT_MULTIPLIER : 1;
		_forwardLines = _wParams.PixelY / _lineFactor;
		if ((_wParams.interleaveScan) && (1 < _forwardLines))
			_forwardLines /= 2;

		if (TRUE == _wParams.galvoEnable)
		{
			_backwardLines = (0 < _wParams.flybackCycles) ? _wParams.flybackCycles : 1;
		}
		else
		{
			_backwardLines = _wParams.flybackCycles;
		}

		_overallLines = _forwardLines + _backwardLines;

		//detemine the X& Y FOV in unit of volt, full swing of waveform,
		//based on field size and the image pixel aspect ratio
		// voltage required is happend to be the mechanical angle of the mirror 
		theta = (double) _wParams.fieldSize * _wParams.field2Volts;

		_fieldX_volt =  theta * _wParams.fieldScaleFineX; //theta /2.0; 
		_fieldY_volt = theta * (double) _wParams.PixelY / (double)_wParams.PixelX ; 
		_fieldY_volt = (_wParams.yAmplitudeScaler/100.0) * _fieldY_volt * _wParams.fieldScaleFineY;

		if(FALSE == _wParams.galvoEnable)
		{
			//force all of the Y values to be equal
			_fieldY_volt = 0.0;
		}

		offset_Xtheta = (double) _wParams.offsetX * _wParams.field2Volts;
		offset_Ytheta = (double) _wParams.verticalScanDirection * _wParams.offsetY * _wParams.field2Volts;//to keep the direction of the galvo/resonance and galvo/galvo equivalent. change the sign for the y offset

		_galvoXOffset = offset_Xtheta + _wParams.fineOffset[0];
		_galvoYOffset = offset_Ytheta + _wParams.fineOffset[1];

		_galvoSamplesEffective = static_cast<long>(clockRateExternal *_wParams.dwellTime * _wParams.PixelX / Constants::US_TO_SEC);
		_pockelsSamplesEffective = static_cast<long>(_wParams.clockRatePockels *_wParams.dwellTime * _wParams.PixelX / Constants::US_TO_SEC);
		break;
	default:
		return FALSE;
	}

	//for all area modes:
	_galvoSamplesPadding = static_cast<long> (_wParams.galvoRetraceTime/_wParams.dwellTime);  //used fixed amount of time for x galvo retrace

	_galvoFwdSamplesPerLine = _galvoSamplesEffective + (_galvoSamplesPadding * 2);
	_galvoBwdSamplesPerLine = _galvoFwdSamplesPerLine;
	_galvoSamplesPerLine = _galvoFwdSamplesPerLine + _galvoBwdSamplesPerLine;

	_galvoDataForward = _forwardLines * _galvoSamplesPerLine;
	_galvoDataBack = _backwardLines * _galvoSamplesPerLine;

	//If galvoEnable is false (meaning line scan) there is no need for a backscan
	//only a need to keep the frame trigger down for a few time points (LINE_FRAMETRIGGER_TIMEPOINTS)
	if(FALSE == _wParams.galvoEnable)
	{
		_lineFlybackLength = (0 < _backwardLines) ? _backwardLines * _galvoSamplesPerLine : _wParams.minLowPoints;
		_galvoDataLength = _galvoDataForward + _lineFlybackLength;
	}
	else
	{
		_galvoDataLength = _galvoDataForward + _galvoDataBack;
	}
	_frameTime = (0 < clockRateExternal) ? static_cast<double>(_galvoDataLength) / clockRateExternal : 0;
	_frameDataLength = _galvoDataLength;

	//total waveform will be 2 frames after 1st	in interleave
	if ((_wParams.interleaveScan) && (1 < _wParams.numFrame))
	{
		_galvoDataLength = 2 * (_galvoDataForward + _galvoDataBack);	
	}

	//prepare params for later build
	long unitSize[3] = {_galvoDataLength, _pockelsSamplesEffective, _frameDataLength};
	if(FALSE == ResetGGalvoWaveformParam(&_gParams[_scanAreaId], unitSize, pockelsCnt, _wParams.digLineSelect))
	{
		StringCbPrintfW(message,_MAX_PATH, L"ResetGGalvoWaveformParam failed.");
		LogMessage(message,VERBOSE_EVENT);
		return FALSE;
	}
	return ret;
}
