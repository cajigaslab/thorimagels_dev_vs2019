#include "stdafx.h"
#include "Strsafe.h"
#include "ThorGGNI.h"

///	***************************************** <summary> Single Tasks - Tasks will be terminated after done </summary>	********************************************** ///

long ThorLSMCam::FindPockelsMinMax(long index)
{
	int32 retVal = 0;
	int32 error = 0;

	if(!_pockelsEnable[index])
	{
		return retVal;
	}

	try
	{
		TerminateTask(_taskHandleAOPockels);

		DAQmxErrChk(L"DAQmxCreateTask",DAQmxCreateTask("", &_taskHandleAOPockels));
		DAQmxErrChk(L"DAQmxCreateAOVoltageChan",DAQmxCreateAOVoltageChan(_taskHandleAOPockels, _pockelsLine[index].c_str(), "", -10.0, 10.0, DAQmx_Val_Volts, NULL));

		DAQmxErrChk(L"DAQmxCreateTask",DAQmxCreateTask("", &_taskHandleAIPockels[index]));
		DAQmxErrChk(L"DAQmxCreateAIVoltageChan",DAQmxCreateAIVoltageChan(_taskHandleAIPockels[index], _pockelsPowerInputLine[index].c_str(), "",DAQmx_Val_Cfg_Default, 0,10.0,DAQmx_Val_Volts, NULL));

		const float64 VOLTAGE_START = _pockelsScanVoltageStart[index];
		float64 pockelsPos = VOLTAGE_START;
		const float64 VOLTAGE_RANGE = _pockelsScanVoltageStop[index] - _pockelsScanVoltageStart[index];

		for(long i=0; i<POCKELS_VOLTAGE_STEPS; i++)
		{
			const float64* tmpWfm = &pockelsPos;

			//In DAQ 9.8, task will never be done by Writing Scalar:
			DAQmxErrChk(L"DAQmxWriteAnalogF64",DAQmxWriteAnalogF64(_taskHandleAOPockels, 1, TRUE, 10.0, DAQmx_Val_GroupByChannel, tmpWfm, NULL, NULL));
			DAQmxErrChk(L"DAQmxWaitUntilTaskDone", DAQmxWaitUntilTaskDone(_taskHandleAOPockels, MAX_TASK_WAIT_TIME));

			Sleep(30);

			int32 numRead;
			DAQmxErrChk(L"DAQmxReadAnalogF64",DAQmxReadAnalogF64(_taskHandleAIPockels[index], 1, 10.0, DAQmx_Val_GroupByChannel, &_pockelsReadArray[index][i],1,&numRead,NULL));
			DAQmxErrChk(L"DAQmxWaitUntilTaskDone", DAQmxWaitUntilTaskDone(_taskHandleAIPockels[index], MAX_TASK_WAIT_TIME));

			Sleep(1);

			pockelsPos += VOLTAGE_RANGE/POCKELS_VOLTAGE_STEPS;
		}

		//move back to the start position after the scan
		pockelsPos = VOLTAGE_START;
		const float64* resetWfm = &pockelsPos;
		DAQmxErrChk(L"DAQmxWriteAnalogF64",DAQmxWriteAnalogF64(_taskHandleAOPockels, 1, TRUE, 10.0, DAQmx_Val_GroupByChannel, resetWfm, NULL, NULL));
		DAQmxErrChk(L"DAQmxWaitUntilTaskDone", DAQmxWaitUntilTaskDone(_taskHandleAOPockels, MAX_TASK_WAIT_TIME));

		float64 * pSmoothBuffer = new float64[POCKELS_VOLTAGE_STEPS];

		memcpy(pSmoothBuffer,&_pockelsReadArray[index],POCKELS_VOLTAGE_STEPS * sizeof(float64));

		//smooth the data and ignore the ends
		const long KERNEL_SIZE = 5;
		const long KERNEL_SKIP = 2;

		for(long n=0; n<5; n++)
		{
			for(long i=KERNEL_SKIP; i<POCKELS_VOLTAGE_STEPS-KERNEL_SKIP; i++)
			{
				float64 sum = 0;

				////////////////////////////////////////////////////////////
				//Average Filter
				//for(long j=-1*(KERNEL_SIZE>>1); j<=(KERNEL_SIZE>>1); j++)
				//{
				//	sum += pSmoothBuffer[i+j];
				//}
				//pSmoothBuffer[i] = sum/KERNEL_SIZE;

				//float64 results[KERNEL_SIZE];

				////////////////////////////////////////////////////////////
				//Median Filter
				//for(long k=0,j=-1*(KERNEL_SIZE>>1); j<=(KERNEL_SIZE>>1); j++,k++)
				//{
				//	results[k] = pSmoothBuffer[i+j];
				//}
				//qsort(results,KERNEL_SIZE,sizeof (float64),cmpfunc);

				//pSmoothBuffer[i] = results[(KERNEL_SIZE>>2) + 1];
				////////////////////////////////////////////////////////////

				////////////////////////////////////////////////////////////
				//Gaussian Filter
				float64 kernel[KERNEL_SIZE] = {.06,.24,.4,.24,.06};

				for(long k=0,j=-1*(KERNEL_SIZE>>1); j<=(KERNEL_SIZE>>1); j++,k++)
				{
					sum += kernel[k] * pSmoothBuffer[i+j];
				}

				pSmoothBuffer[i] = sum;
			}
		}

		double arrayMinVal = 10.0;
		double arrayMaxVal = -10.0;

		//locat the min and max for the dataset
		for(long i=KERNEL_SKIP+1; i<POCKELS_VOLTAGE_STEPS-KERNEL_SKIP-1; i++)
		{
			if(pSmoothBuffer[i] < arrayMinVal)
			{
				arrayMinVal = pSmoothBuffer[i];
			}

			if(pSmoothBuffer[i] > arrayMaxVal)
			{
				arrayMaxVal = pSmoothBuffer[i];
			}
		}

		/************* Method I ********************************/

		//Algorithm to find the min and max values to control the pockels
		//Steps/Description:
		//1. Find the mid value between the max and min overall registered values
		//2. Find the location in the array corresponding to this value
		//3. Find the min location and value for the min point by comparing 
		//the slope until it is smaller than the threshold set in the settings file
		//4. Find the max location and value for the min point by comparing 
		//the slope until it is smaller than the threshold set in the settings file

		long midLoc = -1;
		double midVal = 0.0;

		const float64 PEAK_THRESHOLD_VOLTS = (arrayMaxVal - arrayMinVal)*.001;
		const float64 DIFFERENCE_THRESHOLD_VOLTS = (arrayMaxVal - arrayMinVal)*.30;
		const float64 MAX_MIDVALUE_DIFFERENCE = (arrayMaxVal - arrayMinVal) * 0.01;
		const double MID_VALUE = (arrayMaxVal + arrayMinVal) / 2;

		//find the midPoint location
		for(long i=KERNEL_SKIP+2; i<POCKELS_VOLTAGE_STEPS-KERNEL_SKIP-2; i++)
		{			
			if (MAX_MIDVALUE_DIFFERENCE >  MID_VALUE - pSmoothBuffer[i] )
			{
				midLoc = i;
				midVal = _pockelsReadArray[index][i];
				break;
			}
		}

		long minLoc = -1;
		long maxLoc = -1;
		double minVal = 0.0;
		double maxVal = 0.0;

		const double SLOPE_THRESHOLD = _pockelsVoltageSlopeThreshold;

		if (midLoc > 0)
		{
			//find the minVal and minLoc location
			for (int i = midLoc; i >= KERNEL_SKIP+2; i--)
			{
				double x1 = i - 1;
				double x2 = i;

				double y1 = pSmoothBuffer[i - 1];
				double y2 = pSmoothBuffer[i];

				//calculate the slope
				double m = SLOPE_THRESHOLD;
				if 	(x2 != x1)
				{
					m = (y2 - y1) / (x2- x1);
				}
				if (SLOPE_THRESHOLD > abs(m))
				{
					minLoc = i;
					minVal = _pockelsReadArray[index][i];
					break;
				}
			}

			//find the maxVal and maxLoc location
			for (int i = midLoc; i < POCKELS_VOLTAGE_STEPS-KERNEL_SKIP-2; i++)
			{
				double x1 = i - 1;
				double x2 = i;

				double y1 = pSmoothBuffer[i - 1];
				double y2 = pSmoothBuffer[i];

				//calculate the slope
				double m = SLOPE_THRESHOLD;
				if 	(x2 != x1)
				{
					m = (y2 - y1) / (x2- x1);
				}

				if (SLOPE_THRESHOLD > abs(m))
				{
					maxLoc = i;
					maxVal = _pockelsReadArray[index][i];
					break;
				}
			}
		}
		/*********** End of Method I ***************/


		/************* Method II ********************************/
		//long minLoc = -1;
		//long maxLoc = -1;
		//double minVal = 0;
		//double maxVal = 0;

		//const float64 PEAK_THRESHOLD_VOLTS = (arrayMaxVal - arrayMinVal)*.001;
		//const float64 DIFFERENCE_THRESHOLD_VOLTS = (arrayMaxVal - arrayMinVal)*.30;

		//for(long i=KERNEL_SKIP+2; i<POCKELS_VOLTAGE_STEPS-KERNEL_SKIP-2; i++)
		//{
		//	//find the minimum first
		//	if(-1 == minLoc)
		//	{
		//		//transition point indicating a minimum
		//		if((pSmoothBuffer[i-2] - PEAK_THRESHOLD_VOLTS > pSmoothBuffer[i]) && (pSmoothBuffer[i] < pSmoothBuffer[i+2] - PEAK_THRESHOLD_VOLTS))
		//		{
		//			minLoc = i;
		//			minVal = _pockelsReadArray[index][i];

		//		}
		//	}
		//	else
		//	{
		//		//transition point indicating a maximum
		//		if((pSmoothBuffer[i-2]  < pSmoothBuffer[i]) && (pSmoothBuffer[i] > pSmoothBuffer[i+2])&& ((pSmoothBuffer[i] - minVal) > DIFFERENCE_THRESHOLD_VOLTS))
		//		{
		//			maxLoc = i;
		//			maxVal = _pockelsReadArray[index][i];
		//			break;
		//		}
		//	}
		//}
		/*********** End of Method II ***************/

		//*NOTE* To display error information un comment the code below
		//if(minLoc == -1)
		//{
		//	MessageBox(NULL,L"no min found",NULL,NULL);
		//}
		//
		//if(maxLoc == -1)
		//{
		//	MessageBox(NULL,L"no max found",NULL,NULL);
		//}

		//if(midLoc == -1)
		//{
		//	wchar_t msg[_MAX_PATH]
		//	StringCbPrintfW(msg,_MAX_PATH,L"no Mid Found midLoc: %d midVal: %f", midLoc, midVal);
		//	MessageBox(NULL,msg,NULL,NULL);
		//}

		//if((maxVal-minVal) <= DIFFERENCE_THRESHOLD_VOLTS)
		//{
		//  wchar_t msg[_MAX_PATH]
		//	StringCbPrintfW(msg,_MAX_PATH,L"diff threshold failed minVal: %f maxVal: %f", minVal, maxVal);
		//	MessageBox(NULL,msg,NULL,NULL);
		//}

		if((minLoc != -1) && (maxLoc != -1) && ((maxVal - minVal) > DIFFERENCE_THRESHOLD_VOLTS))
		{
			_pockelsMinVoltage[index] = VOLTAGE_START + minLoc * VOLTAGE_RANGE / POCKELS_VOLTAGE_STEPS;
			_pockelsMaxVoltage[index] = VOLTAGE_START + maxLoc * VOLTAGE_RANGE / POCKELS_VOLTAGE_STEPS;
		}
		else
		{
			retVal = -1;
		}

		//remove the line below to show the raw data in the plot
		memcpy(&_pockelsReadArray[index],pSmoothBuffer,POCKELS_VOLTAGE_STEPS * sizeof(float64));

		delete pSmoothBuffer;

		TerminateTask(_taskHandleAOPockels);

	}
	catch(...)
	{
		memset(&_pockelsReadArray[index],0,POCKELS_VOLTAGE_STEPS * sizeof(float64));

		retVal = -1;
	}
	return retVal;
}

long ThorLSMCam::MoveGalvoToStart(void)
{
	int32 retVal = 0;

	if(NULL == _pGalvoStartPos)
	{
		//try to locate first position in file:
		_pGalvoStartPos = (double*)realloc ((void*)_pGalvoStartPos, 2 * sizeof(double));
		if(FALSE == ImageWaveformBuilder->GetGGalvoWaveformStartLoc(_waveformPathName.c_str(), _pGalvoStartPos, _clockRateNI))
			return 0;
	}

	//setup both locations from current to parking the galvo
	float64 posTo[2];
	posTo[0] = _pGalvoStartPos[0];
	posTo[1] = _pGalvoStartPos[1];
	MoveGalvoToPosition(posTo, 0);
	return retVal;
}

long ThorLSMCam::MoveGalvoToParkPosition(long parkAtParking)
{
	int32 retVal = 0;

	if((0 == parkAtParking) && (TRUE == _galvoParkAtStart))
	{
		//park galvo at start position
		MoveGalvoToStart();
		return retVal;
	}

	//setup both locations from current to parking the galvo
	float64 posTo[2] = {0};
	switch (parkAtParking)
	{
	case 2:		//part at exit position
		posTo[0] = _verticalScanDirection * _analogXYmode[0][1] * GALVO_PARK_POSITION;;
		posTo[1] = -1.0 * _verticalScanDirection * _analogXYmode[0][1] * GALVO_PARK_POSITION;
		break;
	case 1:		//park at parking position and honor selected mode
		posTo[0] = _verticalScanDirection * _analogXYmode[0][0] * GALVO_PARK_POSITION;;
		posTo[1] = -1.0 * _verticalScanDirection * _analogXYmode[0][1] * GALVO_PARK_POSITION;
		break;
	case 0:
	default:
		posTo[0] = _verticalScanDirection * GALVO_PARK_POSITION;;
		posTo[1] = -1.0 * _verticalScanDirection * GALVO_PARK_POSITION;
		break;
	}
	MoveGalvoToPosition(posTo, 0);
	return retVal;
}

long ThorLSMCam::MoveGalvoToPosition(double* posXY, int pathID)
{
	int32 retVal = 0, error = 0, samplesRead = 0;
	const double CLK_RATE = DEFAULT_PIXEL_X * Constants::MS_TO_SEC;
	try
	{
		//clear any previous clock and analog output tasks
		TerminateTask(_taskHandleAO1);
		TerminateTask(_taskHandleCO0);

		//read the current location of the galvo
		float64 currentLoc[2] = {0};
		if (!analogReaderNI.get()->getInstance()->ReadLine(_galvoLinesInput, 2, currentLoc))
			return FALSE;

		//setup both locations from current to parking the galvo,
		//consider galvo analog line swap case: X:ao1, Y:ao0, reading position YX instead of XY,
		//also set lines based on pathID to support dual path OTM:
		float64 path[4];
		currentLoc[0] = (0 != _analogFeedbackRatio[0][0]) ? currentLoc[0] / _analogFeedbackRatio[0][0] : currentLoc[0];	// consider feedback ratio from controller
		currentLoc[1] = (0 != _analogFeedbackRatio[0][1]) ? currentLoc[1] / _analogFeedbackRatio[0][1] : currentLoc[1];
		switch (pathID)
		{
		case 1:
			//don't continue if second path is not configured
			if ((0 == _analogChannels[2].length()) || (0 == _analogChannels[3].length()))
				return retVal;

			_galvoLinesOutput = _analogChannels[2] + "," + _analogChannels[3];
			path[0] = currentLoc[0];	//From position X
			path[1] = currentLoc[1];	//From position Y
			break;
		case 0:
		default:
			_galvoLinesOutput = ((0 < _analogChannels[0].length()) && (0 < _analogChannels[1].length())) ? (_analogChannels[0] + "," + _analogChannels[1]) : "/Dev2/ao0:1";
			path[0] = (1 == _analogXYmode[0][1]) ? currentLoc[0] : currentLoc[1];	//From position X
			path[1] = (1 == _analogXYmode[0][1]) ? currentLoc[1] : currentLoc[0];	//From position Y
			break;
		}
		path[2] = posXY[0];				//To position X
		path[3] = posXY[1];				//To position Y

		//use linear maximum step size:
		double stepSizeX = _fieldSizeMax * _field2Theta * _minGalvoFreqHz[pathID] / (double)Constants::DEFAULT_GALVO_HZ / DEFAULT_PIXEL_X;

		//don't continue if already arrived:
		if((stepSizeX > abs(path[2]-path[0])) && (stepSizeX > abs(path[3]-path[1])))
			return retVal;

		int32 count;
		float64* waveXY = ImageWaveformBuilder->GetTravelWaveform(stepSizeX, DAQmx_Val_GroupByScanNumber, path, count);
		if(NULL != waveXY)
		{
			DAQmxErrChk(L"DAQmxCreateTask",DAQmxCreateTask("", &_taskHandleCO0));
			DAQmxErrChk(L"DAQmxCreateCOPulseChanFreq",retVal = DAQmxCreateCOPulseChanFreq(_taskHandleCO0, _controllerOutputLine0.c_str(), "", DAQmx_Val_Hz, DAQmx_Val_Low, 0.0, CLK_RATE, 0.5));
			DAQmxErrChk(L"DAQmxCfgImplicitTiming ",retVal = DAQmxCfgImplicitTiming (_taskHandleCO0, DAQmx_Val_ContSamps, AO_CLOCK_LENGTH));

			//create the analog channels for X & Y galvo out on ao0 and ao1
			DAQmxErrChk(L"DAQmxCreateTask",DAQmxCreateTask("", &_taskHandleAO1));
			DAQmxErrChk(L"DAQmxCreateAOVoltageChan",DAQmxCreateAOVoltageChan(_taskHandleAO1,_galvoLinesOutput.c_str(), "", MIN_AO_VOLTAGE, MAX_AO_VOLTAGE, DAQmx_Val_Volts, NULL));
			DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(_taskHandleAO1, _controllerInternalOutput0.c_str(), CLK_RATE, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, count));
			DAQmxErrChk(L"DAQmxCfgOutputBuffer",retVal = DAQmxCfgOutputBuffer(_taskHandleAO1, count));
			DAQmxErrChk(L"DAQmxWriteAnalogF64",retVal = DAQmxWriteAnalogF64(_taskHandleAO1, count, false, -1, DAQmx_Val_GroupByScanNumber, waveXY, NULL, NULL));
			DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleAO1));
			DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleCO0));
			DAQmxErrChk(L"DAQmxWaitUntilTaskDone",retVal = DAQmxWaitUntilTaskDone(_taskHandleAO1, MAX_TASK_WAIT_TIME));
		}
	}
	catch(...)
	{
		DAQmxFailed(error);
		StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorGGNI MoveGalvoToPosition failed, error: (%d)", retVal);
		LogMessage(_errMsg,ERROR_EVENT);
	}

	//force tasks to be recreated:
	TerminateTask(_taskHandleAO1);
	TerminateTask(_taskHandleCO0);

	return retVal;
}

long ThorLSMCam::MoveGalvoToCenter(void)
{
	const double CENTER_GALVO_VOLTAGE = 0.0;
	float64 centerArray [2] = {CENTER_GALVO_VOLTAGE, CENTER_GALVO_VOLTAGE };
	if(_centerWithOffsets)
	{
		centerArray[0] = CENTER_GALVO_VOLTAGE + _offsetX * _field2Theta * _theta2Volts + _fineOffset[0] + _highResOffset[0];
		centerArray[1] = CENTER_GALVO_VOLTAGE + _verticalScanDirection * _offsetY * _field2Theta * _theta2Volts + _fineOffset[1] + _highResOffset[1];
		MoveGalvoToPosition(centerArray, 0);

		centerArray[0] = CENTER_GALVO_VOLTAGE + _fineOffset2[0] + _highResOffset2[0];
		centerArray[1] = CENTER_GALVO_VOLTAGE + _fineOffset2[1] + _highResOffset2[1];
		MoveGalvoToPosition(centerArray, 1);
	}
	else
	{
		MoveGalvoToPosition(centerArray, 0);
		MoveGalvoToPosition(centerArray, 1);
	}
	return 0;
}

long ThorLSMCam::MovePockelsToParkPosition(void)
{
	long ret = TRUE;
	int32 error = 0;
	try
	{
		if(_pockelsSelect)
		{
			TerminateTask(_taskHandleAOPockels);

			DAQmxErrChk(L"DAQmxCreateTask",DAQmxCreateTask("", &_taskHandleAOPockels));

			string channelString;

			if(_pockelsEnable[0])
			{
				channelString = _pockelsLine[0].c_str();
				if(_pockelsEnable[1])
				{
					channelString += ",";
					channelString += _pockelsLine[1].c_str();
					if(_pockelsEnable[2])
					{
						channelString += ",";
						channelString += _pockelsLine[2].c_str();
						if(_pockelsEnable[3])
						{
							channelString += ",";
							channelString += _pockelsLine[3].c_str();
						}
					}
				}
			}

			DAQmxErrChk(L"DAQmxCreateAOVoltageChan",DAQmxCreateAOVoltageChan(_taskHandleAOPockels, channelString.c_str(), "", -10.0, 10.0, DAQmx_Val_Volts, NULL));

			float64 parkPosArray[4];

			float64 pockelsSetVal0 = _pockelsMinVoltage[0] + _pockelsPowerLevel[0] *(_pockelsMaxVoltage[0] - _pockelsMinVoltage[0]);
			parkPosArray[0] = (TRUE == _pockelsParkAtMinimum) ? _pockelsMinVoltage[0] : pockelsSetVal0;

			float64 pockelsSetVal1 = _pockelsMinVoltage[1] + _pockelsPowerLevel[1] *(_pockelsMaxVoltage[1] - _pockelsMinVoltage[1]);
			parkPosArray[1] = (TRUE == _pockelsParkAtMinimum) ? _pockelsMinVoltage[1] : pockelsSetVal1;

			float64 pockelsSetVal2 = _pockelsMinVoltage[2] + _pockelsPowerLevel[2] *(_pockelsMaxVoltage[2] - _pockelsMinVoltage[2]);
			parkPosArray[2] = (TRUE == _pockelsParkAtMinimum) ? _pockelsMinVoltage[2] : pockelsSetVal2;

			float64 pockelsSetVal3 = _pockelsMinVoltage[3] + _pockelsPowerLevel[3] *(_pockelsMaxVoltage[3] - _pockelsMinVoltage[3]);
			parkPosArray[3] = (TRUE == _pockelsParkAtMinimum) ? _pockelsMinVoltage[3] : pockelsSetVal3;

			DAQmxErrChk(L"DAQmxWriteAnalogF64",DAQmxWriteAnalogF64(_taskHandleAOPockels, 1, true, 10.0, DAQmx_Val_GroupByScanNumber, parkPosArray, NULL, NULL));
			DAQmxErrChk(L"DAQmxWaitUntilTaskDone", DAQmxWaitUntilTaskDone(_taskHandleAOPockels, MAX_TASK_WAIT_TIME));

			TerminateTask(_taskHandleAOPockels);
		}
	}
	catch(...)
	{
		DAQmxFailed(error);
		ret = FALSE;
	}
	return ret;
}

long ThorLSMCam::MovePockelsToPowerLevel(long index)
{
	int32 error = 0;
	try
	{
		if(_pockelsEnable[index])
		{
			TerminateTask(_taskHandleAOPockels);

			DAQmxErrChk(L"DAQmxCreateTask",DAQmxCreateTask("", &_taskHandleAOPockels));

			DAQmxErrChk(L"DAQmxCreateAOVoltageChan",DAQmxCreateAOVoltageChan(_taskHandleAOPockels, _pockelsLine[index].c_str(), "", MIN_AO_VOLTAGE, MAX_AO_VOLTAGE, DAQmx_Val_Volts, NULL));

			float64 minArray [1] = {_pockelsMinVoltage[index] + (_pockelsMaxVoltage[index] - _pockelsMinVoltage[index])*_pockelsPowerLevel[index]};
			DAQmxErrChk(L"DAQmxWriteAnalogF64",DAQmxWriteAnalogF64(_taskHandleAOPockels, 1, true, MAX_TASK_WAIT_TIME, DAQmx_Val_GroupByScanNumber, minArray, NULL, NULL));

			DAQmxErrChk(L"DAQmxWaitUntilTaskDone", DAQmxWaitUntilTaskDone(_taskHandleAOPockels, MAX_TASK_WAIT_TIME));

			TerminateTask(_taskHandleAOPockels);
		}
	}
	catch (...)
	{
		DAQmxFailed(error);
		return FALSE;
	}
	return TRUE;
}

long ThorLSMCam::SetCaptureActiveOutput(long startOrStop)
{	
	int32 retVal = 0;
	int32 error = 0;
	int32 written = 0;
	uInt8 out = static_cast<uInt8>(startOrStop);

	if(_captureActiveOutput.size() > 0 
		&& _captureActiveOutput.find("Dev")!=std::string::npos 
		&& (_captureActiveOutput.find("PFI")!=std::string::npos || _captureActiveOutput.find("port")!=std::string::npos))
	{
		//Lock the mutex at the beginning of the function to keep other digital tasks off being created
		WaitForSingleObject(_hSpecialDigitalLineReadyHandle, Constants::TIMEOUT_MS);
		TerminateTask(_taskHandleDO3);
		try
		{
			DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleDO3));
			DAQmxErrChk(L"DAQmxCreateDOChan",retVal = DAQmxCreateDOChan(_taskHandleDO3, _captureActiveOutput.c_str(), "", DAQmx_Val_ChanPerLine));

			DAQmxErrChk (L"DAQmxWriteDigitalLines",retVal = DAQmxWriteDigitalLines(_taskHandleDO3,1,TRUE,0,DAQmx_Val_GroupByChannel,&out,&written,NULL));
			DAQmxErrChk (L"DAQmxWaitUntilTaskDone",retVal = DAQmxWaitUntilTaskDone(_taskHandleDO3,MAX_TASK_WAIT_TIME));
		}
		catch(...)
		{
			StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI error set frame buffer ready output = %d", retVal);
			LogMessage(message,VERBOSE_EVENT);	
		}
		//release mutex before returning to allow other digital tasks being created
		TerminateTask(_taskHandleDO3);
		ReleaseMutex(_hSpecialDigitalLineReadyHandle);
	}	
	return (retVal == 0) ? TRUE : FALSE;
}

long ThorLSMCam::SetFrameBufferReadyOutput()
{	
	int32 retVal = 0;
	int32 error = 0;
	int32 written = 0;
	uInt8* out = new uInt8[2];
	out[0] = 1; out[1] = 0;
	uInt8 outHigh = 1;
	uInt8 outLow = 0;

	if(_frameBufferReadyOutput.size() > 0 
		&& _frameBufferReadyOutput.find("Dev")!=std::string::npos 
		&& (_frameBufferReadyOutput.find("PFI")!=std::string::npos || _frameBufferReadyOutput.find("port")!=std::string::npos))
	{
		//Lock the mutex at the beginning of the function to keep other digital tasks off being created
		WaitForSingleObject(_hSpecialDigitalLineReadyHandle, Constants::TIMEOUT_MS);
		TerminateTask(_taskHandleDO3);
		try
		{
			DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleDO3));
			DAQmxErrChk(L"DAQmxCreateDOChan",retVal = DAQmxCreateDOChan(_taskHandleDO3, _frameBufferReadyOutput.c_str(), "", DAQmx_Val_ChanPerLine));
			DAQmxErrChk (L"DAQmxWriteDigitalLines",retVal = DAQmxWriteDigitalLines(_taskHandleDO3,1,TRUE,0,DAQmx_Val_GroupByChannel,&outHigh,&written,NULL));
			DAQmxErrChk (L"DAQmxWaitUntilTaskDone",retVal = DAQmxWaitUntilTaskDone(_taskHandleDO3,MAX_TASK_WAIT_TIME));
			DAQmxErrChk (L"DAQmxWriteDigitalLines",retVal = DAQmxWriteDigitalLines(_taskHandleDO3,1,TRUE,0,DAQmx_Val_GroupByChannel,&outLow,&written,NULL));
			DAQmxErrChk (L"DAQmxWaitUntilTaskDone",retVal = DAQmxWaitUntilTaskDone(_taskHandleDO3,MAX_TASK_WAIT_TIME));
		}
		catch(...)
		{
			StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI error set frame buffer ready output = %d", retVal);
			LogMessage(message,VERBOSE_EVENT);	
		}
		//release mutex before returning to allow other digital tasks being created
		TerminateTask(_taskHandleDO3);
		ReleaseMutex(_hSpecialDigitalLineReadyHandle);
	}
	return (retVal == 0) ? TRUE : FALSE;
}

void ThorLSMCam::ThorCloseNITasks(void)
{
	///The order in which the tasks are stopped and cleared is extremely important
	///using the wrong order may affect the next acquisition
	//the first task to be stopped and cleared should be _taskHandleAI0
	//***		analog data input task		***//
	if (_taskHandleAI0 != 0)
	{
		DAQmxStopTask(_taskHandleAI0);
		DAQmxClearTask(_taskHandleAI0);
		_taskHandleAI0 = NULL;
	}

	//***		waveform tasks		***//
	if (_taskHandleDI1 != 0)
	{
		DAQmxStopTask(_taskHandleDI1);
		DAQmxClearTask(_taskHandleDI1);
		_taskHandleDI1 = NULL;
	}
	//***		non-waveform tasks		***//
	if (_taskHandleCO0 != 0)
	{
		DAQmxStopTask(_taskHandleCO0);
		DAQmxClearTask(_taskHandleCO0);
		_taskHandleCO0 = NULL;
	}

	if (_taskHandleCO1 != 0)
	{
		DAQmxStopTask(_taskHandleCO1);
		DAQmxClearTask(_taskHandleCO1);
		_taskHandleCO1 = NULL;
	}	

	if (_taskHandleCO2 != 0)
	{
		DAQmxStopTask(_taskHandleCO2);
		DAQmxClearTask(_taskHandleCO2);
		_taskHandleCO2 = NULL;
	}

	if (_taskHandleAO1 != 0)
	{
		DAQmxStopTask(_taskHandleAO1);
		DAQmxClearTask(_taskHandleAO1);
		_taskHandleAO1 = NULL;
	}

	if (_taskHandleDO1 != 0)
	{
		DAQmxStopTask(_taskHandleDO1);
		DAQmxClearTask(_taskHandleDO1);
		_taskHandleDO1 = NULL;
	}

	if (_taskHandleAOPockels != 0)
	{
		DAQmxStopTask(_taskHandleAOPockels);
		DAQmxClearTask(_taskHandleAOPockels);		
		_taskHandleAOPockels = NULL;
	}

	TerminateTask(_taskHandleDO3);

	StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorGGNI closed all NI tasks.");
	LogMessage(_errMsg,VERBOSE_EVENT);
}

long ThorLSMCam::ThorVCMDigitalShutterPosition(int pos)
{
	int32 retVal=0;
	int32 error = 0;

	if(_shutterLine.size() <= 0)
	{
		return retVal;
	}

	retVal = DAQmxStopTask(_taskHandleDO2);
	retVal = DAQmxClearTask(_taskHandleDO2);
	DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleDO2));
	DAQmxErrChk(L"DAQmxCreateDOChan",retVal = DAQmxCreateDOChan(_taskHandleDO2, _shutterLine.c_str(), "", DAQmx_Val_ChanForAllLines));
	uInt32 triggerLevel[1];
	if(1 == pos)
	{
		triggerLevel[0] = 0x1;
	}
	else
	{
		triggerLevel[0] = 0x0;
	}
	DAQmxErrChk(L"DAQmxWriteDigitalU32",retVal = DAQmxWriteDigitalU32(_taskHandleDO2, 1, 1, 0, DAQmx_Val_GroupByChannel, &triggerLevel[0], NULL, NULL));
	DAQmxErrChk(L"DAQmxWaitUntilTaskDone", DAQmxWaitUntilTaskDone(_taskHandleDO2, MAX_TASK_WAIT_TIME));
	DAQmxStopTask(_taskHandleDO2);
	DAQmxClearTask(_taskHandleDO2);
	return retVal;
}

///	***************************************** <summary> Setup Tasks - Tasks will not be terminated after started </summary>	********************************************** ///

/// <summary> Callback function to be invoked for clock master image load mode on digital lines </summary>
int32 CVICALLBACK ThorLSMCam::EveryNClockMasterLineCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	//terminate if necessary:
	if(WaitForSingleObject(ThorLSMCam::_hStopAcquisition, 0) == WAIT_OBJECT_0)
	{
		ThorCloseNITasks();
		return 0;
	}

	return TryWriteClockMasterLineWaveform();
}

/// <summary> Callback function to be invoked for clock master image load mode on galvo XY </summary>
int32 CVICALLBACK ThorLSMCam::EveryNClockMasterGalvoCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	//terminate if necessary:
	if(WaitForSingleObject(ThorLSMCam::_hStopAcquisition, 0) == WAIT_OBJECT_0)
	{
		ThorCloseNITasks();
		return 0;
	}

	return TryWriteClockMasterGalvoWaveform();
}

/// <summary> Callback function to be invoked for clock master image load mode on pockels </summary>
int32 CVICALLBACK ThorLSMCam::EveryNClockMasterPockelCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	//terminate if necessary:
	if(WaitForSingleObject(ThorLSMCam::_hStopAcquisition, 0) == WAIT_OBJECT_0)
	{
		ThorCloseNITasks();
		return 0;
	}

	return TryWriteClockMasterPockelWaveform();
}

long ThorLSMCam::SetupClockMasterClock(void)
{
	int32 retVal = 0;
	int32 error = 0;	

	//determine clock settings
	long clockTypeContinuous = FALSE;

	switch (_imgPtyDll.triggerMode)
	{
	case ICamera::SW_FREE_RUN_MODE:
	case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
		clockTypeContinuous = TRUE;
		break;
	default:
		break;
	}

	//_galvoDataLength and frameCount are both of type long
	//making it impossible to overflow on a uInt64 when multiplying		
	uInt64 numSamples = _totalLength[SignalType::ANALOG_XY];

	//stop and clear counter 1 first since it is linked to counter 0
	retVal = DAQmxStopTask(_taskHandleCO1);
	retVal = DAQmxClearTask(_taskHandleCO1);
	_taskHandleCO1 = NULL;
	//
	//1st Counter output pulses are used as the clock for the AO waveform generation
	//
	retVal = DAQmxStopTask(_taskHandleCO0);
	retVal = DAQmxClearTask(_taskHandleCO0);
	_taskHandleCO0 = NULL;
	try
	{
		DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleCO0));
		DAQmxErrChk(L"DAQmxCreateCOPulseChanFreq",retVal = DAQmxCreateCOPulseChanFreq(_taskHandleCO0, _controllerOutputLine0.c_str(), "", DAQmx_Val_Hz, DAQmx_Val_Low, 0.0, _clockRateNI, 0.5));
		//select between continuous capture and finite
		if(clockTypeContinuous == TRUE)
		{
			DAQmxErrChk(L"DAQmxCfgImplicitTiming ",retVal = DAQmxCfgImplicitTiming (_taskHandleCO0, DAQmx_Val_ContSamps, AO_CLOCK_LENGTH));
		}
		else
		{
			//if numSamples is greater than MAXINT32 then we change the mode to continuous
			//by doing this an overflow inside of the NI functions that follow is avoided
			//if numSamples iss less than or equal to MAXINT32 then set the finite number of samples
			if (MAXINT32 < numSamples)
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"CO0 numSamples = _totalLength[SignalType::DIGITAL_LINES] * frameCount is greater than MAXINT32. Will change from finite to continuous");
				LogMessage(_errMsg,INFORMATION_EVENT);
				DAQmxErrChk(L"DAQmxCfgImplicitTiming ",retVal = DAQmxCfgImplicitTiming (_taskHandleCO0, DAQmx_Val_ContSamps, AO_CLOCK_LENGTH));
			}
			else
			{
				DAQmxErrChk(L"DAQmxCfgImplicitTiming ",retVal = DAQmxCfgImplicitTiming (_taskHandleCO0, DAQmx_Val_FiniteSamps, numSamples));
			}
		}

		DAQmxErrChk(L"DAQmxConnectTerms",retVal = DAQmxConnectTerms(_controllerInternalOutput0.c_str(), _startTriggerLine.c_str(), DAQmx_Val_DoNotInvertPolarity)); 

		//start with a software trigger not hardware for imaging
		DAQmxErrChk(L"DAQmxDisableStartTrig",retVal = DAQmxDisableStartTrig(_taskHandleCO0));

		//
		//The 2nd Counter output pulse will be used as the digital output clock, for the frame and line trigger
		//The 2nd Counter output pulse start is triggered by the 1st Counter output; Not used in waveform mode
		DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleCO1));
		DAQmxErrChk(L"DAQmxCreateCOPulseChanFreq",retVal = DAQmxCreateCOPulseChanFreq(_taskHandleCO1, _controllerOutputLine1.c_str(), "", DAQmx_Val_Hz, DAQmx_Val_Low, 0.0, _clockRateNI, 0.5));
		//select between continuous capture and finite
		if(clockTypeContinuous == TRUE)
		{
			DAQmxErrChk(L"DAQmxCfgImplicitTiming ",retVal = DAQmxCfgImplicitTiming (_taskHandleCO1, DAQmx_Val_ContSamps, AO_CLOCK_LENGTH));
		}
		else
		{
			//_galvoDataLength and frameCount are both of type long
			//making it impossible to overflow on a uInt64 when multiplying		
			//if numSamples is greater than MAXINT32 then we change the mode to continuous
			//by doing this an overflow inside of the NI functions that follow is avoided
			//if numSamples iss less than or equal to MAXINT32 then set the finite number of samples
			if (MAXINT32 < numSamples)
			{
				StringCbPrintfW(_errMsg,_MAX_PATH,L"CO1 numSamples = _totalLength[SignalType::DIGITAL_LINES] * frameCount is greater than MAXINT32. Will change from finite to continuous");
				LogMessage(_errMsg,INFORMATION_EVENT);
				DAQmxErrChk(L"DAQmxCfgImplicitTiming ",retVal = DAQmxCfgImplicitTiming (_taskHandleCO1, DAQmx_Val_ContSamps, AO_CLOCK_LENGTH));
			}
			else
			{
				DAQmxErrChk(L"DAQmxCfgImplicitTiming ",retVal = DAQmxCfgImplicitTiming (_taskHandleCO1, DAQmx_Val_FiniteSamps, numSamples));
			}	
		}
		DAQmxErrChk(L"DAQmxConfigureStartTrig",retVal = DAQmxCfgDigEdgeStartTrig(_taskHandleCO1, _controllerInternalOutput0.c_str(),DAQmx_Val_Rising)); //start triggered by CO0;
		DAQmxErrChk (L"DAQmxSetArmStartTrigType",retVal = DAQmxSetArmStartTrigType(_taskHandleCO1,DAQmx_Val_DigEdge));
		DAQmxErrChk (L"DAQmxSetDigEdgeArmStartTrigSrc",retVal = DAQmxSetDigEdgeArmStartTrigSrc(_taskHandleCO1,_controllerInternalOutput0.c_str()));
		DAQmxErrChk (L"DAQmxSetDigEdgeArmStartTrigEdge",retVal = DAQmxSetDigEdgeArmStartTrigEdge(_taskHandleCO1,DAQmx_Val_Rising));	

		//convert to seconds, don't allow negative delay
		float64 pulseDelay =  (_pockelsPhaseDelayUS >= 0) ? _pockelsPhaseDelayUS / Constants::US_TO_SEC : 0;

		DAQmxErrChk(L"DAQmxSetChanAtrribute",retVal = DAQmxSetChanAttribute(_taskHandleCO1, "",DAQmx_CO_Pulse_Freq_InitialDelay, pulseDelay,NULL));

		DAQmxErrChk(L"DAQmxExportSignal",retVal = DAQmxExportSignal(_taskHandleCO0,DAQmx_Val_CounterOutputEvent,_clockExportLine.c_str()));

		DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleCO1)); //in fact doesn't start until CO0 starts.

		//prepare counter clock for pockels AO:
		if(TRUE == _pockelsEnable[0])
		{
			DAQmxStopTask(_taskHandleCO2);
			DAQmxClearTask(_taskHandleCO2);
			_taskHandleCO2 = NULL;

			DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleCO2));
			DAQmxErrChk(L"DAQmxCreateCOPulseChanFreq",retVal = DAQmxCreateCOPulseChanFreq(_taskHandleCO2, _controllerOutputLine2.c_str(), "", DAQmx_Val_Hz, DAQmx_Val_Low, 0.0, _clockRatePockels, 0.5));
			DAQmxErrChk(L"DAQmxCfgImplicitTiming ",retVal = DAQmxCfgImplicitTiming (_taskHandleCO2, DAQmx_Val_FiniteSamps, ImageWaveformBuilder->GetPockelsSamplesEffective()));
			DAQmxErrChk(L"DAQmxConfigureStartTrig",retVal = DAQmxCfgDigEdgeStartTrig(_taskHandleCO2,_pockelsTriggerIn.c_str(),DAQmx_Val_Rising)); //start triggered by line
			DAQmxErrChk(L"DAQmxSetArmStartTrigType",retVal = DAQmxSetArmStartTrigType(_taskHandleCO2,DAQmx_Val_DigEdge));
			DAQmxErrChk(L"DAQmxSetDigEdgeArmStartTrigSrc",retVal = DAQmxSetDigEdgeArmStartTrigSrc(_taskHandleCO2,_pockelsTriggerIn.c_str()));
			DAQmxErrChk(L"DAQmxSetDigEdgeArmStartTrigEdge",retVal = DAQmxSetDigEdgeArmStartTrigEdge(_taskHandleCO2,DAQmx_Val_Rising));
			DAQmxErrChk(L"DAQmxSetStartTrigRetriggerable",retVal = DAQmxSetStartTrigRetriggerable(_taskHandleCO2,true));
			DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleCO2)); //doesn't start until line trigger starts.
		}
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorGGNI SetupClockMasterClock failed. (%d)", error);
		LogMessage(_errMsg,ERROR_EVENT);
	}
	return retVal;
}

long ThorLSMCam::SetupClockMasterDigital(void)
{
	int32 retVal = 0, error = 0;
	long bufCount = _wBuffer[SignalType::DIGITAL_LINES].get()->GetBlockCount();

	if(_taskHandleDO1)
	{
		retVal = DAQmxStopTask(_taskHandleDO1);
		retVal = DAQmxClearTask(_taskHandleDO1);
	}
	try
	{
		DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleDO1));

		DAQmxErrChk(L"DAQmxCreateDOChan",retVal = DAQmxCreateDOChan(_taskHandleDO1, _frameTriggerLineInOut.c_str(), "", DAQmx_Val_ChanPerLine));

		if(_dLengthPerAOCallback[SignalType::DIGITAL_LINES] < _totalLength[SignalType::DIGITAL_LINES])
		{
			DAQmxErrChk (L"DAQmxSetWriteAttribute",retVal = DAQmxSetWriteAttribute (_taskHandleDO1, DAQmx_Write_RegenMode, DAQmx_Val_DoNotAllowRegen));
			DAQmxErrChk (L"DAQmxRegisterEveryNSamplesEvent",retVal = DAQmxRegisterEveryNSamplesEvent(_taskHandleDO1, DAQmx_Val_Transferred_From_Buffer, static_cast<uInt32>(_dLengthPerAOCallback[SignalType::DIGITAL_LINES]), 0, EveryNClockMasterLineCallback, NULL));

			DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(_taskHandleDO1, _controllerInternalOutput1.c_str(), _clockRateNI, DAQmx_Val_Rising, DAQmx_Val_ContSamps, _dLengthPerAOCallback[SignalType::DIGITAL_LINES]));

			DAQmxErrChk (L"DAQmxCfgOutputBuffer",retVal = DAQmxCfgOutputBuffer(_taskHandleDO1,static_cast<uInt32>(bufCount * _dLengthPerAOCallback[SignalType::DIGITAL_LINES])));

			DAQmxErrChk(L"DAQmxSetChanAttribute",retVal = DAQmxSetChanAttribute(_taskHandleDO1, "", DAQmx_DO_DataXferReqCond, DAQmx_Val_OnBrdMemNotFull));

			//fill up buffer
			_frameTrigger = (uInt8*)realloc((void*)_frameTrigger, _wBuffer[SignalType::DIGITAL_LINES]->GetBlockSizeInByte() * bufCount);
			if(NULL == _frameTrigger)
				return (-1);
			UCHAR* pFrm = _frameTrigger;
			for (int i = 0; i < bufCount; i++)
			{
				if(TRUE == _wBuffer[SignalType::DIGITAL_LINES]->ReadBlocks((UCHAR*)pFrm))
				{
					pFrm += _wBuffer[SignalType::DIGITAL_LINES]->GetBlockSizeInByte();
				}
				else
				{
					return (-1);
				}
			}
			DAQmxErrChk (L"DAQmxWriteDigitalLines",retVal = DAQmxWriteDigitalLines(_taskHandleDO1,static_cast<int32>(_dLengthPerAOCallback[SignalType::DIGITAL_LINES] * bufCount),false,-1,DAQmx_Val_GroupByScanNumber,_frameTrigger,NULL,NULL));
		}
		else
		{
			DAQmxErrChk (L"DAQmxSetWriteAttribute",retVal = DAQmxSetWriteAttribute (_taskHandleDO1, DAQmx_Write_RegenMode, DAQmx_Val_AllowRegen));

			DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(_taskHandleDO1, _controllerInternalOutput1.c_str(), _clockRateNI, DAQmx_Val_Rising, DAQmx_Val_ContSamps, _dLengthPerAOCallback[SignalType::DIGITAL_LINES]));

			DAQmxErrChk (L"DAQmxCfgOutputBuffer",retVal = DAQmxCfgOutputBuffer(_taskHandleDO1,static_cast<uInt32>(_dLengthPerAOCallback[SignalType::DIGITAL_LINES])));

			//fill up buffer
			_frameTrigger = (uInt8*)realloc((void*)_frameTrigger, _wBuffer[SignalType::DIGITAL_LINES]->GetBlockSizeInByte());
			if(NULL == _frameTrigger)
				return (-1);
			if(FALSE == _wBuffer[SignalType::DIGITAL_LINES]->ReadBlocks((UCHAR*)_frameTrigger))
				return (-1);
			DAQmxErrChk (L"DAQmxWriteDigitalLines",retVal = DAQmxWriteDigitalLines(_taskHandleDO1,static_cast<int32>(_dLengthPerAOCallback[SignalType::DIGITAL_LINES]),false,-1,DAQmx_Val_GroupByScanNumber,_frameTrigger,NULL,NULL));
		}
		DAQmxErrChk(L"DAQmxTaskControl",retVal = DAQmxTaskControl(_taskHandleDO1,DAQmx_Val_Task_Reserve));
		DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleDO1));

		//prepare buffer for later write
		_frameTrigger = (uInt8*)realloc((void*)_frameTrigger, _wBuffer[SignalType::DIGITAL_LINES]->GetBlockSizeInByte());
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorGGNI SetupClockMasterDigital failed, error: (%d)", retVal);
		LogMessage(_errMsg,ERROR_EVENT);
	}
	return retVal;
}

long ThorLSMCam::SetupClockMasterGalvo(void)
{
	int32 retVal = 0, error = 0;
	long bufCount = _wBuffer[SignalType::ANALOG_XY].get()->GetBlockCount();

	if (_taskHandleAO1)
	{
		retVal = DAQmxStopTask(_taskHandleAO1);
		retVal = DAQmxClearTask(_taskHandleAO1);
	}

	if(_pockelsEnableIntegrated)
	{
		_galvoAndPockelsLinesOutput = ((0 < _analogChannels[0].length()) && (0 < _analogChannels[1].length()) && (0 < _analogChannels[2].length())) ? (_analogChannels[0] + "," + _analogChannels[1] + "," + _analogChannels[2]) : "/Dev2/ao0:2";
	}
	else
	{
		_galvoAndPockelsLinesOutput = ((0 < _analogChannels[0].length()) && (0 < _analogChannels[1].length())) ? (_analogChannels[0] + "," + _analogChannels[1]) : "/Dev2/ao0:1";
	}
	try
	{

		DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleAO1));

		DAQmxErrChk(L"DAQmxCreateAOVoltageChan",retVal = DAQmxCreateAOVoltageChan(_taskHandleAO1, _galvoAndPockelsLinesOutput.c_str(), "", MIN_AO_VOLTAGE, MAX_AO_VOLTAGE, DAQmx_Val_Volts, NULL));

		//allow regen if not active load
		if(_dLengthPerAOCallback[SignalType::ANALOG_XY] < _totalLength[SignalType::ANALOG_XY])
		{
			DAQmxErrChk (L"DAQmxSetWriteAttribute",retVal = DAQmxSetWriteAttribute (_taskHandleAO1, DAQmx_Write_RegenMode, DAQmx_Val_DoNotAllowRegen));
			DAQmxErrChk (L"DAQmxRegisterEveryNSamplesEvent",retVal = DAQmxRegisterEveryNSamplesEvent(_taskHandleAO1, DAQmx_Val_Transferred_From_Buffer, static_cast<uInt32>(_dLengthPerAOCallback[SignalType::ANALOG_XY]), 0, EveryNClockMasterGalvoCallback, NULL));

			DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(_taskHandleAO1, _controllerInternalOutput0.c_str(), _clockRateNI, DAQmx_Val_Rising, DAQmx_Val_ContSamps, _dLengthPerAOCallback[SignalType::ANALOG_XY]));

			DAQmxErrChk (L"DAQmxCfgOutputBuffer",retVal = DAQmxCfgOutputBuffer(_taskHandleAO1,static_cast<uInt32>(bufCount * _dLengthPerAOCallback[SignalType::ANALOG_XY])));

			DAQmxErrChk(L"DAQmxSetChanAttribute",retVal = DAQmxSetChanAttribute(_taskHandleAO1, "", DAQmx_AO_DataXferReqCond, DAQmx_Val_OnBrdMemNotFull));

			//fill up buffer
			_pGalvoWaveformXYP = (float64*)realloc((void*)_pGalvoWaveformXYP, _wBuffer[SignalType::ANALOG_XY]->GetBlockSizeInByte() * bufCount);
			if(NULL == _pGalvoWaveformXYP)
				return (-1);
			UCHAR* pGalvo = (UCHAR*)_pGalvoWaveformXYP;
			for (int i = 0; i < bufCount; i++)
			{
				if(TRUE == _wBuffer[SignalType::ANALOG_XY]->ReadBlocks(pGalvo))
				{
					pGalvo += _wBuffer[SignalType::ANALOG_XY]->GetBlockSizeInByte();
				}
				else
				{
					return (-1);
				}
			}
			DAQmxErrChk(L"DAQmxWriteAnalogF64",retVal = DAQmxWriteAnalogF64(_taskHandleAO1,static_cast<int32>(_dLengthPerAOCallback[SignalType::ANALOG_XY] * bufCount), false, -1, DAQmx_Val_GroupByScanNumber, _pGalvoWaveformXYP, NULL, NULL));
		}
		else
		{
			DAQmxErrChk (L"DAQmxSetWriteAttribute",retVal = DAQmxSetWriteAttribute (_taskHandleAO1, DAQmx_Write_RegenMode, DAQmx_Val_AllowRegen));

			DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(_taskHandleAO1, _controllerInternalOutput0.c_str(), _clockRateNI, DAQmx_Val_Rising, DAQmx_Val_ContSamps, _dLengthPerAOCallback[SignalType::ANALOG_XY]));

			DAQmxErrChk (L"DAQmxCfgOutputBuffer",retVal = DAQmxCfgOutputBuffer(_taskHandleAO1,static_cast<uInt32>(_dLengthPerAOCallback[SignalType::ANALOG_XY])));

			//fill up buffer
			_pGalvoWaveformXYP = (float64*)realloc((void*)_pGalvoWaveformXYP, _wBuffer[SignalType::ANALOG_XY]->GetBlockSizeInByte());
			if(NULL == _pGalvoWaveformXYP)
				return (-1);
			if(FALSE == _wBuffer[SignalType::ANALOG_XY]->ReadBlocks((UCHAR*)_pGalvoWaveformXYP))
				return (-1);
			DAQmxErrChk(L"DAQmxWriteAnalogF64",retVal = DAQmxWriteAnalogF64(_taskHandleAO1,static_cast<int32>(_dLengthPerAOCallback[SignalType::ANALOG_XY]), false, -1, DAQmx_Val_GroupByScanNumber, _pGalvoWaveformXYP, NULL, NULL));
		}
		DAQmxErrChk(L"DAQmxTaskControl",retVal = DAQmxTaskControl(_taskHandleAO1,DAQmx_Val_Task_Reserve));
		DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleAO1));

		//prepare buffer for later write
		_pGalvoWaveformXYP = (float64*)realloc((void*)_pGalvoWaveformXYP, _wBuffer[SignalType::ANALOG_XY]->GetBlockSizeInByte());
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,_MAX_PATH,L"ThorGGNI SetupClockMasterGalvo failed, error: (%d)", retVal);
		LogMessage(_errMsg,ERROR_EVENT);
	}
	return retVal;
}

long ThorLSMCam::SetupClockMasterPockel(void)
{
	int32 retVal = 0, error = 0;
	long bufCount = _wBuffer[SignalType::ANALOG_POCKEL].get()->GetBlockCount();

	//support USB board control on Pockels cell
	int32 dataXferType = DAQmx_Val_DMA;
	BoardInfo* bInfo = _boardInfoNI.get()->getInstance()->GetBoardInfo(GetDevIDName(_controllerInternalOutput2));
	if(NULL != bInfo)
	{
		dataXferType = (BoardStyle::USB == bInfo->boardStyle) ? DAQmx_Val_USBbulk : DAQmx_Val_DMA;
	}

	if(_pockelsSelect)
	{
		TerminateTask(_taskHandleAOPockels);

		string channelString;
		channelString = (_pockelsEnable[0]) ? _pockelsLine[0].c_str() : "";

		if(_pockelsEnable[1])
		{
			if(0 < channelString.size())
				channelString += ",";
			channelString += _pockelsLine[1].c_str();
			if(_pockelsEnable[2])
			{
				if(0 < channelString.size())
					channelString += ",";
				channelString += _pockelsLine[2].c_str();
				if(_pockelsEnable[3])
				{
					if(0 < channelString.size())
						channelString += ",";
					channelString += _pockelsLine[3].c_str();
				}
			}
		}
		try
		{
			DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleAOPockels));

			DAQmxErrChk(L"DAQmxCreateAOVoltageChan",retVal = DAQmxCreateAOVoltageChan(_taskHandleAOPockels, channelString.c_str() ,"", MIN_AO_VOLTAGE,MAX_AO_VOLTAGE, DAQmx_Val_Volts, NULL));				

			if(_dLengthPerAOCallback[SignalType::ANALOG_POCKEL] < _totalLength[SignalType::ANALOG_POCKEL])
			{
				DAQmxErrChk (L"DAQmxSetWriteAttribute",retVal = DAQmxSetWriteAttribute (_taskHandleAOPockels, DAQmx_Write_RegenMode, DAQmx_Val_DoNotAllowRegen));

				DAQmxErrChk (L"DAQmxRegisterEveryNSamplesEvent",retVal = DAQmxRegisterEveryNSamplesEvent(_taskHandleAOPockels, DAQmx_Val_Transferred_From_Buffer, static_cast<uInt32>(_dLengthPerAOCallback[SignalType::ANALOG_POCKEL]), 0, EveryNClockMasterPockelCallback, NULL));

				DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(_taskHandleAOPockels, _controllerInternalOutput2.c_str(), _clockRatePockels, DAQmx_Val_Rising, DAQmx_Val_ContSamps, _dLengthPerAOCallback[SignalType::ANALOG_POCKEL]));

				DAQmxErrChk (L"DAQmxSetAODataXferMech", retVal = DAQmxSetAODataXferMech(_taskHandleAOPockels,"",dataXferType));

				DAQmxErrChk (L"DAQmxCfgOutputBuffer",retVal = DAQmxCfgOutputBuffer(_taskHandleAOPockels,static_cast<uInt32>(bufCount * _dLengthPerAOCallback[SignalType::ANALOG_POCKEL])));

				DAQmxErrChk(L"DAQmxSetChanAttribute",retVal = DAQmxSetChanAttribute(_taskHandleAOPockels, "", DAQmx_AO_DataXferReqCond, DAQmx_Val_OnBrdMemNotFull));

				//fill up buffer
				_pPockelsWaveform = (float64*)realloc((void*)_pPockelsWaveform, _wBuffer[SignalType::ANALOG_POCKEL]->GetBlockSizeInByte() * bufCount);
				if(NULL == _pPockelsWaveform)
					return (-1);
				UCHAR* pPockel = (UCHAR*)_pPockelsWaveform;
				for (int i = 0; i < bufCount; i++)
				{
					if(TRUE == _wBuffer[SignalType::ANALOG_POCKEL]->ReadBlocks(pPockel))
					{
						pPockel += _wBuffer[SignalType::ANALOG_POCKEL]->GetBlockSizeInByte();
					}
					else
					{
						return (-1);
					}
				}
				retVal = DAQmxWriteAnalogF64(_taskHandleAOPockels,static_cast<int32>(_dLengthPerAOCallback[SignalType::ANALOG_POCKEL] * bufCount), false, -1, DAQmx_Val_GroupByScanNumber, _pPockelsWaveform, NULL, NULL);
			}
			else
			{
				DAQmxErrChk (L"DAQmxSetWriteAttribute",retVal = DAQmxSetWriteAttribute (_taskHandleAOPockels, DAQmx_Write_RegenMode, DAQmx_Val_AllowRegen));

				DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(_taskHandleAOPockels, _controllerInternalOutput2.c_str(), _clockRatePockels, DAQmx_Val_Rising, DAQmx_Val_ContSamps, _dLengthPerAOCallback[SignalType::ANALOG_POCKEL]));

				DAQmxErrChk (L"DAQmxCfgOutputBuffer",retVal = DAQmxCfgOutputBuffer(_taskHandleAOPockels,static_cast<uInt32>(_dLengthPerAOCallback[SignalType::ANALOG_POCKEL])));

				//fill up buffer
				_pPockelsWaveform = (float64*)realloc((void*)_pPockelsWaveform, _wBuffer[SignalType::ANALOG_POCKEL]->GetBlockSizeInByte());
				if(NULL == _pPockelsWaveform)
					return (-1);
				if(FALSE == _wBuffer[SignalType::ANALOG_POCKEL]->ReadBlocks((UCHAR*)_pPockelsWaveform))
					return (-1);
				retVal = DAQmxWriteAnalogF64(_taskHandleAOPockels,static_cast<int32>(_dLengthPerAOCallback[SignalType::ANALOG_POCKEL]), false, -1, DAQmx_Val_GroupByScanNumber, _pPockelsWaveform, NULL, NULL);
			}

			if(0 == retVal && TRUE == _pockelsEnable[0] && TRUE == _imgPtyDll.useReferenceForPockelsOutput && TRUE == _pockelsReferenceRequirementsMet)
			{
				//Set the pockels1 reference to be external
				DAQmxErrChk(L"DAQmxSetAODACRefSrc",retVal = DAQmxSetAODACRefSrc(_taskHandleAOPockels, _pockelsLine[0].c_str(), DAQmx_Val_External));
				//Even if the pockels1 reference is set to external, the value needs to be set
				DAQmxErrChk(L"DAQmxSetAODACRefVal",retVal = DAQmxSetAODACRefVal(_taskHandleAOPockels, _pockelsLine[0].c_str(), MAX_AO_VOLTAGE));
				//Set the input line for the pockels reference. It needs to be an APFI line
				DAQmxErrChk(L"DAQmxSetAODACRefExtSrc",retVal = DAQmxSetAODACRefExtSrc(_taskHandleAOPockels, _pockelsLine[0].c_str(), _pockelsReferenceLine.c_str()));
			}

			if(0 == retVal)
			{
				DAQmxErrChk(L"DAQmxTaskControl",retVal = DAQmxTaskControl(_taskHandleAOPockels,DAQmx_Val_Task_Reserve));
				DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleAOPockels));
			}
			StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI SetupClockMasterPockel DAQmxStartTask AO2 Return = %d", retVal);
			LogMessage(message,VERBOSE_EVENT);

			//The pockels waveform starts after the second line trigger
			//To get around this, setup the pockels waveform, move the line trigger to 1 and immediately back to 0
			//to simulate a trigger
			//With this, the pockels will trigger the next time it sees a line trigger
			std::string triggerLine = "/" + _devID + "/port0/line6";
			TogglePulseToDigitalLine(_taskHandleDO1, triggerLine.c_str(), 1, TogglePulseMode::Pulse);
		}
		catch(...)
		{
			StringCbPrintfW(_errMsg,_MAX_PATH, L"ThorGGNI SetupClockMasterPockel failed, error: (%d)", retVal);
			LogMessage(_errMsg,ERROR_EVENT);
		}
	}
	return retVal;
}

long ThorLSMCam::SetupFrameTriggerInput(void)
{
	int32 retVal = 0, error = 0;

	switch (_imgPtyDll.triggerMode)
	{
	case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
	case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
	case ICamera::HW_SINGLE_FRAME: ///Setup frame triggered by external hardware
		ResetEvent(_hHardwareTriggerInEvent);

		if(_taskHandleDI1)
		{
			DAQmxStopTask(_taskHandleDI1);
			DAQmxClearTask(_taskHandleDI1);
		}
		try
		{
			DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleDI1));

			DAQmxErrChk(L"DAQmxCreateCICountEdgesChan",retVal = DAQmxCreateCICountEdgesChan (_taskHandleDI1, _controllerOutputLine3.c_str(), "", DAQmx_Val_Rising, 0, DAQmx_Val_CountUp ));

			DAQmxErrChk (L"DAQmxSetCICountEdgesTerm",retVal = DAQmxSetCICountEdgesTerm(_taskHandleDI1,_controllerOutputLine3.c_str(),_frameTriggerLineIn.c_str()));

			DAQmxErrChk(L"DAQmxCfgSampClkTiming",retVal = DAQmxCfgSampClkTiming(_taskHandleDI1,_frameTriggerLineIn.c_str(),1000,DAQmx_Val_Rising,DAQmx_Val_HWTimedSinglePoint, 0));

			DAQmxErrChk(L"DAQmxRegisterSignalEvent",retVal = DAQmxRegisterSignalEvent(_taskHandleDI1, DAQmx_Val_SampleClock , 0, ThorLSMCam::HWTriggerCallback, NULL));	

			DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleDI1));

			////DO NOT use below DI method to count HW trigger,
			////since it will cause DO task to be 2x freq at waveform mode:
			//DAQmxErrChk(L"DAQmxCreateTask",retVal = DAQmxCreateTask("", &_taskHandleDI1));
			//DAQmxErrChk(L"DAQmxCreateDIChan",retVal = DAQmxCreateDIChan(_taskHandleDI1, _frameTriggerLineIn.c_str(), "", DAQmx_Val_ChanPerLine));
			//DAQmxErrChk (L"DAQmxCfgChangeDetectionTiming",retVal = DAQmxCfgChangeDetectionTiming(_taskHandleDI1,_frameTriggerLineIn.c_str(),"", DAQmx_Val_ContSamps, 1000));
			//DAQmxErrChk(L"DAQmxSetBufInputBufSize",retVal = DAQmxSetBufInputBufSize(_taskHandleDI1,0));	//For DI to work on PFI line
			//DAQmxErrChk(L"DAQmxRegisterSignalEvent",retVal = DAQmxRegisterSignalEvent(_taskHandleDI1, DAQmx_Val_ChangeDetectionEvent , 0, ThorLSMCam::HWTriggerCallback, NULL));	
			//DAQmxErrChk(L"DAQmxStartTask",retVal = DAQmxStartTask(_taskHandleDI1));
		}
		catch(...)
		{
			StringCbPrintfW(message,_MAX_PATH, L"ThorGGNI error set frame trigger input = %d", retVal);
			LogMessage(message,VERBOSE_EVENT);
		}
		break;
	}
	return retVal;
}

/// <summary> write galvo waveform directly from waveform builder in active load only </summary>
long ThorLSMCam::TryWriteClockMasterGalvoWaveform(void)
{
	int32 retVal = 0;

	if(_taskHandleAO1)
	{
		//no more buffer to read:
		if(0 == _wBuffer[SignalType::ANALOG_XY]->GetReadableBlockCounts())
			return (-1);

		if(TRUE == _wBuffer[SignalType::ANALOG_XY]->ReadBlocks((UCHAR*)_pGalvoWaveformXYP))
		{
			retVal = DAQmxWriteAnalogF64(_taskHandleAO1, static_cast<int32>(_dLengthPerAOCallback[SignalType::ANALOG_XY]), false, -1, DAQmx_Val_GroupByScanNumber, _pGalvoWaveformXYP, NULL, NULL);
			if(0 != retVal)
				goto TERM_TASK;

			//invoke update of buffer:
			_wBuffer[SignalType::ANALOG_XY]->CheckWritableBlockCounts(TRUE);
		}
		else
			return (-1);
	}
	return retVal;

TERM_TASK:
	SetEvent(_hStopAcquisition);
	SetEvent(_hStatusError);
	ThorCloseNITasks();
	StringCbPrintfW(_errMsg,_MAX_PATH,L"Unable to write image waveform.\n Please consider to increase image load time if this issue persists.\n");
	MessageBox(NULL,_errMsg,L"Image Load Error", MB_OK | MB_SETFOREGROUND | MB_ICONERROR);
	return retVal;
}

/// <summary> write pockels waveform directly from waveform builder in active load only </summary>
long ThorLSMCam::TryWriteClockMasterPockelWaveform(void)
{
	int32 retVal = 0;

	//write pockels:
	if((_pockelsSelect) && (_taskHandleAOPockels))
	{
		//no more buffer to read:
		if(0 == _wBuffer[SignalType::ANALOG_POCKEL]->GetReadableBlockCounts())
			return (-1);

		if(TRUE == _wBuffer[SignalType::ANALOG_POCKEL]->ReadBlocks((UCHAR*)_pPockelsWaveform))
		{
			retVal = DAQmxWriteAnalogF64(_taskHandleAOPockels, static_cast<int32>(_dLengthPerAOCallback[SignalType::ANALOG_POCKEL]), false, -1, DAQmx_Val_GroupByScanNumber, _pPockelsWaveform, NULL, NULL);
			if(0 != retVal)
				goto TERM_TASK;

			//invoke update of buffer:
			_wBuffer[SignalType::ANALOG_POCKEL]->CheckWritableBlockCounts(TRUE);
		}
		else
			return (-1);
	}
	return retVal;

TERM_TASK:
	SetEvent(_hStopAcquisition);
	SetEvent(_hStatusError);
	ThorCloseNITasks();
	return retVal;
}

/// <summary> write line waveforms directly from waveform builder in active load only </summary>
long ThorLSMCam::TryWriteClockMasterLineWaveform(void)
{
	int32 retVal = 0;

	if(_taskHandleDO1)
	{
		//no more buffer to read:
		if(0 == _wBuffer[SignalType::DIGITAL_LINES]->GetReadableBlockCounts())
			return (-1);

		if(TRUE == _wBuffer[SignalType::DIGITAL_LINES]->ReadBlocks((UCHAR*)_frameTrigger))
		{
			DAQmxResetWriteNextWriteIsLast(_taskHandleDO1);

			retVal = DAQmxWriteDigitalLines(_taskHandleDO1,static_cast<int32>(_dLengthPerAOCallback[SignalType::DIGITAL_LINES]), false, -1, DAQmx_Val_GroupByScanNumber, _frameTrigger, NULL, NULL);
			if(0 != retVal)
				goto TERM_TASK;

			//invoke update of buffer:
			_wBuffer[SignalType::DIGITAL_LINES]->CheckWritableBlockCounts(TRUE);
		}
		else
			return (-1);
	}
	return retVal;

TERM_TASK:
	SetEvent(_hStopAcquisition);
	SetEvent(_hStatusError);
	ThorCloseNITasks();
	return retVal;
}

