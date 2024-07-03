#pragma once
#include <map>
#include <list>
#include "..\..\..\..\..\Common\camera.h"
#include <windows.h>
#include "..\..\..\..\..\Common\ThorSharedTypesCPP.h"
#include "Types.h"

#define TIME_UNIT_BY_SECOND (0.001)
#define GALVO_MIN_VOL -9.99
#define GALVO_MAX_VOL 9.99
#define POCKEL_MIN_VOL -9.99
#define POCKEL_MAX_VOL 9.99

template <typename T>
struct ParameterValue
{
	T MinValue;
	T* ActualValue;
	T MaxValue;
	ParameterValue() {};
	ParameterValue(T minValue, T maxValue, T* value):MinValue(minValue), MaxValue(maxValue), ActualValue(value) {}
};

template <typename T>
struct PointMap
{
	T XValue;
	T YValue;
	PointMap() {};
	PointMap(T xValue, T yValue) :XValue(xValue), YValue(yValue) {}
};

class CameraConfig
{
public: 

	//********** General waveform parameters **********
	double ResonanceFrequency;	//approximate frequency of resonance
	unsigned int SamplesPerLine;	//sample points in each line

	//********** Galvo X parameters **********
	double F2VGx1;	//field to voltage for X1 channel
	double F2VGx2;	//field to voltage for X2 channel
	double GX1FeedbackRatio;	//ratio of Galvo X1 feedback signal to input signal
	double GX2FeedbackRatio;	//ratio of Galvo X2 feedback signal to input signal
	double GX1FeedbackOffset;	//offset of Galvo X1 feedback signal to input signal
	double GX2FeedbackOffset;	//offset of Galvo X2 feedback signal to input signal
	double DelayTimeGx1;	//DelayTime of X1 channel by millisecond
	double DelayTimeGx2;	//DelayTime of X2 channel by millisecond
	// the input of galvo X2 and X1 must satisfy:
	// X2=-GForX2*X1 + HForX2
	double GForX2;
	double HForX2;
	//range of actual position
	double MinPosX;
	double MaxPosX;
	//range of control voltage
	double MinPosXVoltage;
	double MaxPosXVoltage;
	//params for flyBack line
	double MaxVelocityX;	//maximum gradient of sine wave and the line
	double MaxOvershootX;	//amplitude of sine wave part
	double GXExtendTimeStart;	//extend the constant voltage waveform of galvo X before scanning stripe
	double GXExtendTimeEnd;		//extend the constant voltage waveform of galvo X after scanning stripe

	//********** Galvo Y parameters **********
	double F2VGy;	//field to voltage for Y channel
	double GYFeedbackRatio;	//ratio of Galvo Y feedback signal to input signal
	double GYFeedbackOffset;	//offset of Galvo Y feedback signal to input signal
	double DelayTimeGy;	//DelayTime of Y channel by sample points
	//range of actual position
	double MinPosY;
	double MaxPosY;
	//range of control voltage
	double MinPosYVoltage;
	double MaxPosYVoltage;
	//params for flyBack line
	double GYExtendTimeStart;	//extend the Galvo Y waveform linearly before scanning stripe
	double GYExtendTimeEnd;		//extend the Galvo Y waveform linearly after scanning stripe
	double MaxVelocityY;	//maximum gradient of sine wave and the line
	double MaxOvershootY;	//amplitude of sine wave part

	//********** Pokels cell parameters **********
	double DelayTimePC;	//DelayTime of Pokels cell channel by sample points
	//range of actual position
	double PockelDutyCycle;	//duty cycle of pokels cell square wave
	double PockelAllowMin;
	double PockelAllowMax;
	double PockelMinPercent;
	double PockelInMax;
	double PockelInMin;

	//********** Voice coil parameters **********
	double F2VZ;	//field to voltage for Voice coil channel
	double VCFeedbackRatio;	//ratio of Voice coil feedback signal to input signal
	double VCFeedbackOffset;	//offset of Voice coil feedback signal to input signal
	double DelayTimeVC;	//DelayTime of Voice coil channel by sample points
	//range of actual position
	double MinPosZ;
	double MaxPosZ;
	//range of control voltage
	double MinPosZVoltage;
	double MaxPosZVoltage;
	//params for flyBack line
	double VCExtendTimeStart;	//extend the Voice coil waveform linearly before scanning stripe
	double VCExtendTimeEnd;		//extend the Voice coil waveform linearly after scanning stripe
	unsigned int VCSkipLines;	//set skip lines to a fixed value
	double MaxOvershootVC;
	double MaxVelocityVC;
	// for the FOV center shift;
	double CenterShiftX;
	double CenterShiftY;

	double CurveParamA;
	double CurveParamB;

	// sample points per line
	unsigned int VCPointsPerLine;

	//********** additional parameters **********
	static const int MIN_FIELDSIZE = 5;
	static const int DEFAULT_FIELDSIZE = 160;
	static const int MAX_FIELDSIZE = 255;

	unsigned int GalvoEnable;								//line scan mode when galvoEnable = 0
	unsigned int ScanMode;									//scan mode
	unsigned int AreaMode;									//area mode
	unsigned int FieldSize;									//field size
	unsigned int AverageMode;								//average mode
	unsigned int AverageNum;								//average number
	unsigned int OneXFieldSize;								//field size at magnification 1x
	unsigned int FrameCount;								//total frame count to acquire
	double FieldSizeCalibration;							//field size calibration based on stripe dimension
	double FieldOffset[2];									//field offset X[0] and Y[1]
private:
	std::map<ICamera::Params, ParameterValue<double>> _doubleparameters;
	std::map<ICamera::Params, ParameterValue<unsigned int>> _uintParameters;

	std::list<PointMap<double>> _resonantAmplitudeToVoltage;
	std::list<PointMap<double>> _resonantTwoWayAlignment;

public:
	CameraConfig();
	CameraConfig(CameraConfig* cfg);
	void LoadFromXMLFile();
	long SaveConfigFile();
	void InitParameterRange();
	long LoadExperimentXML();

	long SetParameter(ICamera::Params parameter, unsigned int value);
	long SetParameter(ICamera::Params parameter, double value);
	long GetParameterRange(ICamera::Params parameter, unsigned int &min, unsigned int &max, unsigned int &value);
	long GetParameterRange(ICamera::Params parameter, double &min, double &max, double &value);
	long GetParameter(ICamera::Params parameter, unsigned int &value);
	long GetParameter(ICamera::Params parameter, double &value);
	long GetResonantVoltage(double amplitude, double &voltage);
	long SetReosnantVoltage(double amplitude, double voltage);
	long GetTwoWayAlignmentPoint(double amplitude, double &point);
	long SetTwoWayAlignmentPoint(double amplitude, double point);
	bool IsUIntValue(ICamera::Params parameter);

	void CalculateFieldToVoltage() {
		F2VGx1 = (MaxPosXVoltage - MinPosXVoltage) / (MaxPosX - MinPosX);
		F2VGx2 = GForX2 * F2VGx1;
		F2VGy = (MaxPosYVoltage - MinPosYVoltage) / (MaxPosY - MinPosY);
		F2VZ = (MaxPosZVoltage - MinPosZVoltage) / (MaxPosZ - MinPosZ);
	};
	double GetMaxDelayTime() {
		double delayTime = 0;
		delayTime = max(delayTime, DelayTimeGy);
		delayTime = max(delayTime, DelayTimeGx1);
		delayTime = max(delayTime, DelayTimeGx2);
		delayTime = max(delayTime, DelayTimeVC);
		delayTime = max(delayTime, DelayTimePC);
		return delayTime * TIME_UNIT_BY_SECOND;
	};

	double GetYCenterVoltage()
	{
		return (MaxPosYVoltage + MinPosYVoltage) / 2;
	}

	double GetX2CenterVoltage()
	{
		return (MaxPosXVoltage + MinPosXVoltage) / 2;
	}

	double GetX1CenterVoltage()
	{
		return (MaxPosXVoltage + MinPosXVoltage) / 2 * GForX2 + HForX2;
	}

	double GetZCenterVoltage()
	{
		return (MaxPosZVoltage + MinPosZVoltage) / 2;
	}

	double GetPockelPowerVoltage(double percentage);

	//get park pos
	double GetYParkPosVoltage(GALVO_PARK_TYPE type = GALVO_PARK_TYPE::PARK_NEAR_START)
	{
		return (MinPosYVoltage > 0 && PARK_NEAR_START == type) ? GALVO_MAX_VOL : GALVO_MIN_VOL;
	}

	double GetX2ParkPosVoltage(GALVO_PARK_TYPE type = GALVO_PARK_TYPE::PARK_NEAR_START)
	{
		return (MinPosXVoltage > 0 && PARK_NEAR_START == type) ? GALVO_MAX_VOL : GALVO_MIN_VOL;
	}

	double GetX1ParkPosVoltage(GALVO_PARK_TYPE type = GALVO_PARK_TYPE::PARK_NEAR_START)
	{
		return GetX2ParkPosVoltage(type) * GForX2 + HForX2;
	}

	double GetZParkVoltage()
	{
		return GetZCenterVoltage();
	}

	//get real voltage
	double CheckRange(double voltage)
	{
		if (voltage > GALVO_MAX_VOL) return  GALVO_MAX_VOL;
		if (voltage < GALVO_MIN_VOL) return GALVO_MIN_VOL;
		return voltage;
	}
	double GetYRealVoltage(double voltage)
	{
		double realVoltage = (voltage - GYFeedbackOffset) / GYFeedbackRatio;
		return CheckRange(realVoltage);
	}

	double GetX2RealVoltage(double voltage)
	{
		double realVoltage = (voltage - GX2FeedbackOffset) / GX2FeedbackRatio;
		return CheckRange(realVoltage);
	}

	double GetX1RealVoltage(double voltage)
	{
		double realVoltage = (voltage - GX1FeedbackOffset) / GX1FeedbackRatio;
		return CheckRange(realVoltage);
	}

	double GetZRealVoltage(double voltage)
	{
		double realVoltage = (voltage - VCFeedbackOffset) / VCFeedbackRatio;
		return CheckRange(realVoltage);
	}

	template <class T> 
	T GetParameter(ICamera::Params parameter, T* defaultValue = NULL)
	{
		T val = (NULL != defaultValue) ? *defaultValue : 0;
		if (typeid(unsigned int) == typeid(T))
		{
			unsigned int iVal = 0;
			if (!GetParameter((ICamera::Params)parameter, iVal) && (NULL != defaultValue))
				iVal = static_cast<unsigned int>(*defaultValue);
			val = iVal;
		}
		else if (typeid(double) == typeid(T))
		{
			double dVal = 0.0;
			if (!GetParameter((ICamera::Params)parameter, dVal) && (NULL != defaultValue))
				dVal = *defaultValue;
			val = static_cast<T>(dVal);
		}
		return val;
	};

	~CameraConfig();
};

static unsigned long long GetAvailableMemorySize()
{
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);
	return status.ullAvailPhys;
}