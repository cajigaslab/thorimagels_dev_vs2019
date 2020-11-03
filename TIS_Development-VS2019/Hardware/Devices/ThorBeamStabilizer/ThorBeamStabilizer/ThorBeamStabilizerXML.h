#pragma once
class ThorBeamStabilizerXML
{
public:
	ThorBeamStabilizerXML();
	~ThorBeamStabilizerXML();
	long GetPiezoActuatorsConnection(long &portID,long &baudRate);
	long GetPiezoActuatorPositions(long &piezo1,long &piezo2, long &piezo3, long &piezo4);
	long SetPiezoActuatorPositions(long piezo1,long piezo2, long piezo3, long piezo4);
	long GetFactoryPiezoActuatorPositions(long &piezo1,long &piezo2, long &piezo3, long &piezo4, long &piezoStepLimit);
	long GetBeamProfilerSerialNumber(const std::string signature, std::string &serialNumber);
	long GetControlSettings(long &piezo1Orientation, long &piezo2Orientation, long &piezo3Orientation, long &piezo4Orientation, long &deadband, double &p1Term, double &p2Term, double &maxExposureTime, double &minExposureTime, double &clipLevel, long &alignTimeOutSec);
	long SaveConfigFile();

private:
	long OpenConfigFile();

	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

	static const char* const CONNECTION;
	enum {NUM_CONNECTION_ATTRIBUTES = 2};
	static const char* const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];

	static const char* const ACTUATORS_POSITION;
	enum {NUM_ACTUATORS_POSITION_ATTRIBUTES = 4};
	static const char* const ACTUATORS_POSITION_ATTR[NUM_ACTUATORS_POSITION_ATTRIBUTES];

	static const char* const FACTORY_ACTUATORS_POSITION;
	enum {NUM_FACTORY_ACTUATORS_POSITION_ATTRIBUTES = 5};
	static const char* const FACTORY_ACTUATORS_POSITION_ATTR[NUM_FACTORY_ACTUATORS_POSITION_ATTRIBUTES];

	enum {NUM_BEAM_PROFILE_SETTINGS_ATTRIBUTES = 1};
	static const char* const BEAM_PROFILER_ATTR[NUM_BEAM_PROFILE_SETTINGS_ATTRIBUTES];

	static const char* const CONTROL_SETTINGS;
	enum {NUM_CONTROL_SETTINGS_ATTRIBUTES = 11};
	static const char* const CONTROL_SETTINGS_ATTR[NUM_CONTROL_SETTINGS_ATTRIBUTES];
};