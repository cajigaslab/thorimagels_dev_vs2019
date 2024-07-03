#include <windows.h>
#include "DAQBoard.h"
#include "CameraConfig.h"
#include "ThorMesoScan.h"

#define VOLT_MIN -2.0
#define VOLT_MAX 2.0
#define SAMPLE_COUNT 100
#define MAX_TASK_WAIT_TIME 10.0

using namespace std;

int GetKernelSmooth(float64* value, float64* maxVoltage, float64* minVoltage,float64* minPercent)
{
	float64 * pSmoothBuffer = new float64[SAMPLE_COUNT];
	memcpy_s(pSmoothBuffer, SAMPLE_COUNT * sizeof(float64), value, SAMPLE_COUNT * sizeof(float64));

	const long KERNEL_SIZE = 5;
	const long KERNEL_SKIP = 2;
	for (long n = 0; n < 5; n++)
	{
		for (long i = KERNEL_SKIP; i < SAMPLE_COUNT - KERNEL_SKIP; i++)
		{
			float64 sum = 0;
			float64 kernel[KERNEL_SIZE] = { .06,.24,.4,.24,.06 };
			for (long k = 0, j = -1 * (KERNEL_SIZE >> 1); j <= (KERNEL_SIZE >> 1); j++, k++)
			{
				sum += kernel[k] * pSmoothBuffer[i + j];
			}
			pSmoothBuffer[i] = sum;
		}
	}
	double arrayMinVal = 10.0;
	double arrayMaxVal = -10.0;

	//locat the min and max for the dataset
	for (long i = KERNEL_SKIP + 1; i < SAMPLE_COUNT - KERNEL_SKIP - 1; i++)
	{
		if (pSmoothBuffer[i] < arrayMinVal)
		{
			arrayMinVal = pSmoothBuffer[i];
		}

		if (pSmoothBuffer[i] > arrayMaxVal)
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

	const float64 DIFFERENCE_THRESHOLD_VOLTS = (arrayMaxVal - arrayMinVal)*0.3;
	const float64 MAX_MIDVALUE_DIFFERENCE = (arrayMaxVal - arrayMinVal) * 0.01;
	const double MID_VALUE = (arrayMaxVal + arrayMinVal) / 2;

	//find the midPoint location
	for (long i = KERNEL_SKIP + 2; i < SAMPLE_COUNT - KERNEL_SKIP - 2; i++)
	{
		if (MAX_MIDVALUE_DIFFERENCE > MID_VALUE - pSmoothBuffer[i])
		{
			midLoc = i;
			midVal = value[i];
			break;
		}
	}

	long minLoc = -1;
	long maxLoc = -1;
	double minVal = 0.0;
	double maxVal = 0.0;

	const double SLOPE_THRESHOLD[4] = { 0.01,0.02,0.03 };


	if (midLoc > 0)
	{
		for each (auto temp_slope in SLOPE_THRESHOLD)
		{
			//find the minVal and minLoc location
			for (int i = midLoc; i >= KERNEL_SKIP + 2; i--)
			{
				double x1 = i - 1;
				double x2 = i;

				double y1 = pSmoothBuffer[i - 1];
				double y2 = pSmoothBuffer[i];

				//calculate the slope
				double m = temp_slope;
				if (x2 != x1)
				{
					m = (y2 - y1) / (x2 - x1);
				}
				if (temp_slope > abs(m))
				{
					minLoc = i;
					minVal = value[i];
					break;
				}
			}
			if (minLoc != -1)
				break;
		}

		for each (auto temp_slope in SLOPE_THRESHOLD)
		{
			//find the maxVal and maxLoc location
			for (int i = midLoc; i < SAMPLE_COUNT - KERNEL_SKIP - 2; i++)
			{
				double x1 = i - 1;
				double x2 = i;

				double y1 = pSmoothBuffer[i - 1];
				double y2 = pSmoothBuffer[i];

				//calculate the slope
				double m = temp_slope;
				if (x2 != x1)
				{
					m = (y2 - y1) / (x2 - x1);
				}

				if (temp_slope > abs(m))
				{
					maxLoc = i;
					maxVal = value[i];
					break;
				}
			}
			if (maxLoc != -1)
				break;
		}
	}
	if ((minLoc != -1) && (maxLoc != -1) && maxLoc- minLoc>=10 && ((maxVal - minVal) > DIFFERENCE_THRESHOLD_VOLTS))
	{
		*minVoltage = VOLT_MIN + minLoc * (VOLT_MAX - VOLT_MIN) / SAMPLE_COUNT;
		*maxVoltage = VOLT_MIN + maxLoc * (VOLT_MAX - VOLT_MIN) / SAMPLE_COUNT;
		*minPercent = minVal / maxVal*100;
		delete pSmoothBuffer;
		return TRUE;
	}
	else
	{
		delete pSmoothBuffer;
		if (minLoc == -1)
		{
			if (maxLoc == -1)
				return ICamera::CameraStates::POCKELS_CALIBRATION_FIND_MAX_MIN_FAILED;
			else
				return ICamera::CameraStates::POCKELS_CALIBRATION_FIND_MIN_FAILED;
		}
		else if(maxLoc == -1)
			return ICamera::CameraStates::POCKELS_CALIBRATION_FIND_MAX_FAILED;
		return ICamera::CameraStates::POCKELS_CALIBRATION_VALUE_SMALL_CHANGE;
	}
}

int FindPockelsMinMax(DAQBoard* _hDAQController, CameraConfig* _pCameraConfig, bool& runningFlag)
{
	const float64 VoltageRange = VOLT_MAX - VOLT_MIN;
	const float64 Step = VoltageRange / SAMPLE_COUNT;
	float64 readValue[SAMPLE_COUNT];
	for (int i = 0; i < SAMPLE_COUNT; i++)
	{
		float64 value = VOLT_MIN + i * Step;
		if(!_hDAQController->InvokeTask(_pCameraConfig->POCKEL_AO_CHANNEL, AO, &value, 1, 1))
			return ICamera::CameraStates::POCKELS_CALIBRATION_SET_VALUE_FAILED;
		Sleep(30);
		if (!_hDAQController->ReadVoltages(_pCameraConfig->PDA_CHANNEL, &readValue[i], 1))
			return ICamera::CameraStates::POCKELS_CALIBRATION_GET_VALUE_FAILED;
		if (readValue[i] > 10.0)return ICamera::CameraStates::POCKELS_CALIBRATION_FAILED_UPPER_BOUND;
		if (readValue[i] < 0.0)return ICamera::CameraStates::POCKELS_CALIBRATION_FAILED_LOWER_BOUND;
		if (!runningFlag)
		{
			_hDAQController->InvokeTask(_pCameraConfig->POCKEL_AO_CHANNEL, AO, &_pCameraConfig->PockelInMin, 1, 1);
			return ICamera::CameraStates::POCKELS_CALIBRATION_STOP;
		}
	}

	float64 maxVoltage = 0;
	float64 minVoltage = 0;
	float64 minPercent = 0;
	int states = GetKernelSmooth(readValue, &maxVoltage, &minVoltage,&minPercent);
	if(states == (int)ICamera::CameraStates::Success)
	{
		_pCameraConfig->PockelInMax = maxVoltage;
		_pCameraConfig->PockelInMin = minVoltage;
		_pCameraConfig->PockelMinPercent = minPercent;
	}
	_hDAQController->InvokeTask(_pCameraConfig->POCKEL_AO_CHANNEL, AO, &_pCameraConfig->PockelInMin, 1, 1);
	return states;
}

int StartPockelsCalibration(DAQBoard* _hDAQController, CameraConfig* _pCameraConfig,bool& runningFlag )
{
	return FindPockelsMinMax(_hDAQController, _pCameraConfig, runningFlag);
}

