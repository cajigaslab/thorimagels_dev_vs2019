#pragma once

class ThorBScopeXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	 
	 static const char * const CONNECTION;
	 enum {NUM_CONNECTION_ATTRIBUTES = 3};
	 static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];
	 
	 static const char * const RRANGECONFIG;
	 enum {NUM_RRANGECONFIG_ATTRIBUTES = 2};
	 static const char * const RRANGECONFIG_ATTR[NUM_RRANGECONFIG_ATTRIBUTES];

	 static const char * const XRANGECONFIG;
	 enum {NUM_XRANGECONFIG_ATTRIBUTES = 4};
	 static const char * const XRANGECONFIG_ATTR[NUM_XRANGECONFIG_ATTRIBUTES];

	 static const char * const YRANGECONFIG;
	 enum {NUM_YRANGECONFIG_ATTRIBUTES = 4};
	 static const char * const YRANGECONFIG_ATTR[NUM_YRANGECONFIG_ATTRIBUTES];

	 static const char * const ZRANGECONFIG;
	 enum {NUM_ZRANGECONFIG_ATTRIBUTES = 4};
	 static const char * const ZRANGECONFIG_ATTR[NUM_ZRANGECONFIG_ATTRIBUTES];
	 	 
	ThorBScopeXML();
	~ThorBScopeXML();

	long GetConnection(long &portID,long &baudRate, long &address);
	long GetRRangeConfig(double &minMM, double &maxMM);
	long GetXRangeConfig(double &minMM, double &maxMM, double &threshold, bool &invert);
	long GetYRangeConfig(double &minMM, double &maxMM, double &threshold, bool &invert);
	long GetZRangeConfig(double &minMM, double &maxMM, double &threshold, bool &invert);	

	long OpenConfigFile();
	long SaveConfigFile();
};