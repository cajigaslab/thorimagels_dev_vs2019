#pragma once

class ThorPLSZXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	static const char * const CONNECTION;
	enum {NUM_CONNECTION_ATTRIBUTES = 3};
	static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];

	static const char * const ZRANGECONFIG;
	enum {NUM_ZRANGECONFIG_ATTRIBUTES = 5};
	static const char * const ZRANGECONFIG_ATTR[NUM_ZRANGECONFIG_ATTRIBUTES];

	static const char * const SLEEPAFTERMOVE;
	enum {NUM_SLEEPAFTERMOVE_ATTRIBUTES = 1};
	static const char * const SLEEPAFTERMOVE_ATTR[NUM_SLEEPAFTERMOVE_ATTRIBUTES];

	ThorPLSZXML();
	~ThorPLSZXML();

	long GetConnection(long &portID,long &baudRate, long &address);
	long GetZRangeConfig(double &minMM, double &maxMM, double &threshold, bool &invert, bool &home);
	long GetSleepAfterMove(long &time_milliseconds);

	long OpenConfigFile();
	long SaveConfigFile();
};