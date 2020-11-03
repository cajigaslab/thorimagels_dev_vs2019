#pragma once

class PowerControlXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	 
	 static const char * const CONNECTION;
	 enum {NUM_CONNECTION_ATTRIBUTES = 3};
	 static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];

	 static const char * const PowerLevel;
	 enum {NUM_POWERLEVEL_ATTRIBUTES = 1};
	 static const char * const POWERLEVEL_ATTR[NUM_POWERLEVEL_ATTRIBUTES];
	 
	PowerControlXML();
	~PowerControlXML();

	long GetConnection(long &portID,long &baudRate, long &address);
	long GetPowerLevel(long &value);
	long SetPowerLevel(long value);

	long OpenConfigFile();
	long SaveConfigFile();
};