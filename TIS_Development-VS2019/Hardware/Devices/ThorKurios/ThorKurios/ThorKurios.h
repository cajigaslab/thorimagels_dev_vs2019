#pragma once

#include "stdafx.h"
#include "Kurios_handler.h"

class ThorKurios : IDevice
{
private:
	ThorKurios();
public:

	static ThorKurios* getInstance();
	~ThorKurios();

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
	long GetLastErrorMsg(wchar_t *msg, long size);
	long SetParamString(const long paramID, wchar_t * str);
	long GetParamString(const long paramID, wchar_t * str, long size);
	long SetParamBuffer(const long paramID, char * buffer, long size);
	long GetParamBuffer(const long paramID, char * buffer, long size);


private:

	long _wavelengthMin;
	long _wavelengthMax;
	long _bandwidthModeMin;
	long _bandwidthModeMax;
	long _controlModeMin;
	long _controlModeMax;
	long _temperatureStatusMin;
	long _temperatureStatusMax;
	long _triggerOutSignalModeMin;
	long _triggerOutSignalModeMax;
	long _forceTriggerMin;
	long _forceTriggerMax;
	long _triggerOutTimeModeMin;
	long _triggerOutTimeModeMax;
	long _switchDelayMin;
	long _switchDelayMax;
	long _deleteSequenceMin;
	long _deleteSequenceMax;

	long _wavelength;
	long _bandwidthMode;
	long _controlMode;
	long _triggerOutSignalMode;
	long _forceTrigger;
	long _triggerOutTimeMode;
	long _switchDelay;
	long _deleteSequence;

	long _cmdWL;
	long _cmdOM;
	long _cmdBW;
	long _cmdSS;
	long _cmdIS;
	long _cmdDS;
	long _cmdFS;
	long _cmdTO;
	long _cmdET;
	long _cmdVA;
	long _cmdTT;

	bool _isInit;
	char * _setSequence;
	char * _insertSequence;
	vector<char*> _devices;
	kurios_handler * _kurios;
	static bool _instanceFlag;
	static auto_ptr<ThorKurios> _single;
	wchar_t _errMsg[MSG_SIZE];

};