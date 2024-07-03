#pragma once
#include "thordaqcmd.h"
#include "thordaqapi.h"

#include "..\..\..\Cameras\thordaq\Dll\ThorDAQIOXML.h" //TODO: need to move this file to a more common place just as we need to move the thordaq dll project to a more common place

#define WIDE2(x) L##x
#define WIDE1(x) WIDE2(x)
#define WFUNCTION WIDE1(__FUNCTION__)

#define ThordaqErrChk(fnName,fnCall) if ((error=(fnCall)) > STATUS_SUCCESSFUL){ StringCbPrintfW(thordaqLogMessage,MSG_SIZE,WFUNCTION L": %s failed. Error code %d",fnName, error); LogMessage(thordaqLogMessage,ERROR_EVENT); }
#define DEFAULT_CARD_NUMBER          0 //First Borad Connected

#define Z_DEFAULT 0.0

class ThorDAQZ : IDevice
{

private:
	ThorDAQZ();
public:

	static ThorDAQZ* getInstance();
	~ThorDAQZ();

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
	long BuildAndGetFastZWaveform(const double updateRate, const long samplesFrame, const long samplesFlyback, const double frameRate, ThorDAQZWaveformParams* waveformParams);
private:
	static void LogMessage(wchar_t* message, long eventLevel);
	double* BuildSingleWaveform();
	double* BuildStaircaseWaveform();
private:

	double _zPos;
	double _zPos_C;
	double _zPos_min;
	double _zPos_max;
	double _zVolts2mm;
	double _zOffsetmm;
	long _DAQDeviceIndex; //<The Index of connected ThorDAQ Device
	static bool _instanceFlag;
	static unique_ptr<ThorDAQZ> _single;
	static wchar_t thordaqLogMessage[MSG_SIZE];
	wchar_t _errMsg[MSG_SIZE]; // error message written to the log file

	long _zPositionBufferSize;
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
	double* _zPockelsPowerBuffer;
	double* _zPositionBuffer;

	double _flybackTimeAdjustMS;
	double _volumeTimeAdjustMS;
	double _stepTimeAdjustMS;
	double _zStepTime;
	double _zIntraStepTime;
	long _volumePoints;
	long _stepPoints;
	long _intraStepPoints;
	long _flybackPoints;
	long _staircaseDelayPercentage;
	BOOL _deviceDetected;
	long _numDevices;
	wstring _waveformOutPath;
	BOARD_INFO_STRUCT _boardInfo;
	std::map<AO, long> _thordaqAOSelection;

	static long _totalPoints;

	static USHORT* _pThorDAQZWaveform;

	bool _useWaveformMode;
};

