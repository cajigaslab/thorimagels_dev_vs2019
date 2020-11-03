#pragma once
class ThorLSKGRXML
{

private:
	static const char * const CONNECTION;
	enum {NUM_CONNECTION_ATTRIBUTES = 2};
	static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];
	
	static const char * const CONFIGURATION;	
	enum {NUM_CONFIGURATION_ATTRIBUTES = 1};
	static const char * const CONFIGURATION_ATTR[NUM_CONFIGURATION_ATTRIBUTES];

	long OpenConfigFile();
	long SaveConfigFile();
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	
public:
	ThorLSKGRXML(void);
	~ThorLSKGRXML(void);
	long GetConnection(long &portID,long &baudRate);
	long GetConfiguration(long &rsInitMode);	// resonance scanner initial mode: 1 on or 0 off
};

