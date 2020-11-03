#pragma once

class CoherentChameleonXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:

	static const char * const CONNECTION;
	enum {NUM_CONNECTION_ATTRIBUTES = 4};
	static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];

	CoherentChameleonXML();
	~CoherentChameleonXML();

	long GetConnection(long &portID, long &secondLaserLine, long &laserMin, long &laserMax);

	long OpenConfigFile(string path);

};