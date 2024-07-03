
#pragma once

#define ThorErrChk(fnCall) if (200 > (error=(fnCall)) > 100) throw "fnCall";
#define ThorFnFailed(error)             ( 200 > (error) > 100 )

#define Z_DEFAULT					0.0

/// <summary>
/// Class ThorZPiezo.
/// </summary>
class ThorZPiezo : IDevice
{
private:
	ThorZPiezo();
public:

	static ThorZPiezo* getInstance();
	~ThorZPiezo();

	long FindDevices(long &DeviceCount);
	long SelectDevice(const long Device);
	long TeardownDevice();
	long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double &param);	
	long PreflightPosition();
	long SetupPosition();
	long StartPosition();
	long StatusPosition(long &status);
	long ReadPosition(DeviceType deviceType, double &pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t * errMsg, long size);	
	long SetParamString(const long paramID, wchar_t * str);
	long GetParamString(const long paramID, wchar_t * str, long size);
	long SetParamBuffer(const long paramID, char * buffer, long size);
	long GetParamBuffer(const long paramID, char * buffer, long size);

private:
	static int32 CVICALLBACK EveryNDataCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);

	static void ThorZPiezo::CloseNITasks(double waitTime = 1.0);
	double* ThorZPiezo::BuildPockelsReferenceWaveform();
	double* ThorZPiezo::BuildSingleWaveform();
	double* ThorZPiezo::BuildStaircaseWaveform();
	long ThorZPiezo::BuildWaveforms();
	long ThorZPiezo::LoadWaveformsAndArmDAQ();
	long ThorZPiezo::SetAO0(double voltage);

private:

	double _zPos;
	double _zPos_C;
	double _zPos_min;
	double _zPos_max;
	double _volts2mm;
	double _offsetmm;
	double _pockelsRefThreshold;
	double* _zPockelsPowerBuffer;
	double* _zPositionBuffer;
	double _flybackTimeAdjustMS;
	double _volumeTimeAdjustMS;
	double _stepTimeAdjustMS;
	double _zStepTime;
	double _zIntraStepTime;
	double _pockelsMin;
	long _zPositionBufferSize;
	long _zPockelsPowerBufferSize;
	long _outputPockelsReference;
	long _outputPockelsResponseType;
	long _referenceWaveformEnable;
	long _activeLoadMS;
	long _preLoadCount;
	long _piezoVolumePoints;
	long _pockelsRefVolumePoints;
	long _piezoStepPoints;
	long _piezoRefStepPoints;
	long _piezoIntraStepPoints;
	long _piezoFlybackPoints;
	long _piezoSampleRate;
	long _staircaseDelayPercentage;
	static long _totalPoints;
	static long _callbackPoints;
	static long _index;
	static long _outputLineCount;

	string _devName;
	string _analogLine;
	string _triggerLine;
	string _pockelsReferenceAnalogLine;
	wstring _waveformOutPath;

	static bool _instanceFlag;
	static auto_ptr<ThorZPiezo> _single;
	static wchar_t _errMsg[MAX_PATH];

	long _z_analog_mode;
	long _z_analog_mode_C;

	double _z_fast_start_pos;
	double _z_fast_start_pos_C;

	double _z_fast_stop_pos;
	double _z_fast_stop_pos_C;

	double _z_fast_volume_time;
	double _z_fast_volume_time_C;

	double _z_fast_volume_time_min;
	double _z_fast_volume_time_max;

	double _z_fast_flyback_time;
	double _z_fast_flyback_time_C;

	double _z_fast_flyback_time_min;
	double _z_fast_flyback_time_max;

	double _z_fast_flyback_timeAdjustMS_min;
	double _z_fast_flyback_timeAdjustMS_max;

	double _z_fast_volumeOrStep_timeAdjustMS_min;
	double _z_fast_volumeOrStep_timeAdjustMS_max;

	static float64* _pWaveform;

	static TaskHandle _taskHandleAO0;

	BOOL _deviceDetected;
	long _numDevices;

};