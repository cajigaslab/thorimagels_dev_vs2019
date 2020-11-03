#pragma once

#include "stdafx.h"
#include "..\..\..\..\Common\thread.h"
#include "BeamsProfile.h"
#include "PiezoInertiaActuators.h"

class ThorBeamStabilizer : IDevice
{
private:
	ThorBeamStabilizer();
public:

	static ThorBeamStabilizer* getInstance();
	~ThorBeamStabilizer();

	long FindDevices(long &DeviceCount);
	long SelectDevice(const long Device);
	long TeardownDevice();
	long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double &param);
	long SetParamBuffer(const long paramID, char * buffer, long size);
	long GetParamBuffer(const long paramID, char * buffer, long size);
	long SetParamString(const long paramID, wchar_t * str);
	long GetParamString(const long paramID, wchar_t * str, long size);
	long PreflightPosition();
	long SetupPosition();
	long StartPosition();
	long StatusPosition(long &status);
	long ReadPosition(DeviceType deviceType, double &pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t *msg, long size);

	static HANDLE _hThread;

	//**This are public to be able to access them on a separate thread **//
	BeamsProfile* _beamsProfile;
	PiezoInertiaActuators* _piezoActuators;
	bool _connectionStablished;
	bool _stabilizeBeam;
	bool _stabilizeBeam_C;
	bool _stabilizeBeam_B;
	bool _factoryPosBeam;
	bool _factoryPosBeam_C;
	bool _factoryPosBeam_B;
	std::array<long, PiezoInertiaActuators::PIEZO_ACTUATORS_NUM> _piezoActuatorStart;
	std::array<long, PiezoInertiaActuators::PIEZO_ACTUATORS_NUM> _piezoOrientation;
	long _deadBand;
	std::array<double, BeamsProfile::BEAM_PROFILE_NUM> _pTerm;
	double _maxExposureTime;
	double _minExposureTime;
	long _alignTimeoutSec;
	std::atomic<bool> _stabilizingThreadActive;
	bool CheckPiezoLimits(std::array<long, PiezoInertiaActuators::PIEZO_ACTUATORS_NUM> piezoPos);
	//******************************************************************//


private:
	static bool _instanceFlag;
	static std::unique_ptr<ThorBeamStabilizer> _single;	
	void LogMessage(wchar_t* message);



	wchar_t _errMsg[MSG_SIZE];   
};