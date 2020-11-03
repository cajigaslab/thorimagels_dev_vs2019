#pragma once

class ThorObjectiveChangerXML
{

private:
	static const char * const CONNECTION;
	enum {NUM_CONNECTION_ATTRIBUTES = 4};
	static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];
	
	long OpenConfigFile();
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	
	
public:
	long SaveConfigFile();
	ThorObjectiveChangerXML(void);
	~ThorObjectiveChangerXML(void);
	long GetConnection(long &portID, long &baudRate, std::wstring &serialNumber, long &homed);
	long UpdateHomedFlagToSettings(long &homed);
};

