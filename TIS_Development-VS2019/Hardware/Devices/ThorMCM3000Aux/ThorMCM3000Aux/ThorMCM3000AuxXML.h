#pragma once

class ThorMCM3000AuxXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	static const char * const CONNECTION;
	enum {NUM_CONNECTION_ATTRIBUTES = 3};
	static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];

	static const char * const XRANGECONFIG;
	enum {NUM_XRANGECONFIG_ATTRIBUTES = 4};
	static const char * const XRANGECONFIG_ATTR[NUM_XRANGECONFIG_ATTRIBUTES];

	static const char * const YRANGECONFIG;
	enum {NUM_YRANGECONFIG_ATTRIBUTES = 4};
	static const char * const YRANGECONFIG_ATTR[NUM_YRANGECONFIG_ATTRIBUTES];

	static const char * const ZRANGECONFIG;
	enum {NUM_ZRANGECONFIG_ATTRIBUTES = 4};
	static const char * const ZRANGECONFIG_ATTR[NUM_ZRANGECONFIG_ATTRIBUTES];

	static const char * const STAGEAXISCONFIG;
	enum {NUM_STAGEAXISCONFIG_ATTRIBUTES = 3};
	static const char * const STAGEAXISCONFIG_ATTR[NUM_STAGEAXISCONFIG_ATTRIBUTES];

	static const char * const SLEEPAFTERMOVE;
	enum {NUM_SLEEPAFTERMOVE_ATTRIBUTES = 1};
	static const char * const SLEEPAFTERMOVE_ATTR[NUM_SLEEPAFTERMOVE_ATTRIBUTES];

	ThorMCM3000AuxXML();
	~ThorMCM3000AuxXML();

	long GetConnection(long &portID,long &baudRate, long &address);
	long GetXRangeConfig(double &minMM, double &maxMM, double &threshold, bool &invert);
	long GetYRangeConfig(double &minMM, double &maxMM, double &threshold, bool &invert);
	long GetZRangeConfig(double &minMM, double &maxMM, double &threshold, bool &invert);
	long GetXYZStageAxes(long &axis0,long &axis1, long &axis2);
	long GetSleepAfterMove(long &time_milliseconds);

	long OpenConfigFile();
	long SaveConfigFile();
};