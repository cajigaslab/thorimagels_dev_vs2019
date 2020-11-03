#pragma once

class ZStepperXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	 
	 static const char * const CONNECTION;
	 enum {NUM_CONNECTION_ATTRIBUTES = 3};
	 static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];

	 static const char * const STEPCONFIG;
	 enum {NUM_STEPCONFIG_ATTRIBUTES = 4};
	 static const char * const STEPCONFIG_ATTR[NUM_STEPCONFIG_ATTRIBUTES];
	 
	 static const char * const RANGECONFIG;
	 enum {NUM_RANGECONFIG_ATTRIBUTES = 4};
	 static const char * const RANGECONFIG_ATTR[NUM_RANGECONFIG_ATTRIBUTES];

	ZStepperXML();
	~ZStepperXML();

	long GetConnection(long &portID,long &baudRate, long &address);
	long GetStepConfig(long &microStep,double &prescaler, double &stepToEncoder, double &mmPerRot);
	long GetRangeConfig(double &minMM, double &maxMM, double &threshold, long &invert);

	long OpenConfigFile(string path);

};