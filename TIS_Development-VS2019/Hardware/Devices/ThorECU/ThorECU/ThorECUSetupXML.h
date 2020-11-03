#pragma once

class ThorECUXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	 
	 static const char * const CONNECTION;	
	 enum {NUM_CONNECTION_ATTRIBUTES = 1};
	 static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];

	 static const char * const CONFIGURATION;	
	 enum {NUM_CONFIGURATION_ATTRIBUTES = 1};
	 static const char * const CONFIGURATION_ATTR[NUM_CONFIGURATION_ATTRIBUTES];

	ThorECUXML();
	~ThorECUXML();

	long GetConnection(long &portID);
	long GetConfiguration(long &rsInitMode);	// resonance sscanner initial mode: 1 on or 0 off

	long OpenConfigFile(string path);

};