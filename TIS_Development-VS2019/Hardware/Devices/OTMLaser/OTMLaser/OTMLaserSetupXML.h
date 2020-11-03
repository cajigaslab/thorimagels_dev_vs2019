#pragma once

class OTMLaserXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	 
	 static const char * const CONNECTION;
	 enum {NUM_CONNECTION_ATTRIBUTES = 1};
	 static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];

	 static const char * const LASERCONFIG;
	 enum {NUM_LASERCONFIG_ATTRIBUTES = 1};
	 static const char * const LASERCONFIG_ATTR[NUM_LASERCONFIG_ATTRIBUTES];

	OTMLaserXML();
	~OTMLaserXML();

	long GetConnection(long &portID);
	long GetLaserConfig(long &IsRS232);

	long OpenConfigFile();

};