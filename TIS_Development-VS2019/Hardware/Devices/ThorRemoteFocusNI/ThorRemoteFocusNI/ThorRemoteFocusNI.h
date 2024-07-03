
#pragma once

#define ThorErrChk(fnCall) if (200 > (error=(fnCall)) > 100) throw "fnCall";
#define ThorFnFailed(error)             ( 200 > (error) > 100 )

#define MIN_PLANES					1

/// <summary>
/// Class ThorRemoteFocusNI.
/// </summary>
class ThorRemoteFocusNI : IDevice
{
private:
	ThorRemoteFocusNI();
public:

	static ThorRemoteFocusNI* getInstance();
	~ThorRemoteFocusNI();

	long FindDevices(long& DeviceCount);
	long SelectDevice(const long Device);
	long TeardownDevice();
	long GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double& param);
	long PreflightPosition();
	long SetupPosition();
	long StartPosition();
	long StatusPosition(long& status);
	long ReadPosition(DeviceType deviceType, double& pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t* errMsg, long size);
	long SetParamString(const long paramID, wchar_t* str);
	long GetParamString(const long paramID, wchar_t* str, long size);
	long SetParamBuffer(const long paramID, char* buffer, long size);
	long GetParamBuffer(const long paramID, char* buffer, long size);

private:
	static int32 CVICALLBACK EveryNDataCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void* callbackData);

	static void ThorRemoteFocusNI::CloseNITasks(double waitTime = 1.0);
	double* ThorRemoteFocusNI::BuildPockelsReferenceWaveform();
	double* ThorRemoteFocusNI::BuildStaircaseWaveform();
	long ThorRemoteFocusNI::BuildWaveforms();
	long ThorRemoteFocusNI::LoadWaveformsAndArmDAQ();
	long ThorRemoteFocusNI::SetAO(double voltage);

private:

	double _zPos;
	double _zPos_C;
	double* _zPockelsPowerBuffer;
	double* _zPositionBuffer;
	double _flybackTimeAdjustMS;
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
	long _volumePoints;
	long _stepPoints;
	long _intraStepPoints;
	long _flybackPoints;
	long _sampleRate;

	static long _totalPoints;
	static long _callbackPoints;
	static long _index;
	static long _outputLineCount;

	string _devName;
	string _analogLine;
	string _triggerLine;
	long _numberOfPlanes;
	wstring _waveformOutPath;

	static bool _instanceFlag;
	static auto_ptr<ThorRemoteFocusNI> _single;
	static wchar_t _errMsg[MAX_PATH];

	long _analog_mode;
	long _analog_mode_C;

	double _start_pos;
	double _start_pos_C;

	double _stop_pos;
	double _stop_pos_C;

	double _volume_time;
	double _volume_time_C;

	double _volume_time_min;
	double _volume_time_max;

	double _flyback_time;
	double _flyback_time_C;

	double _flyback_time_min;
	double _flyback_time_max;

	double _flyback_timeAdjustMS_min;
	double _flyback_timeAdjustMS_max;

	double _step_timeAdjustMS_min;
	double _step_timeAdjustMS_max;

	vector<double> _positionsVoltageValues;

	static float64* _pWaveform;

	static TaskHandle _taskHandleAO0;

	BOOL _deviceDetected;
	long _numDevices;

};