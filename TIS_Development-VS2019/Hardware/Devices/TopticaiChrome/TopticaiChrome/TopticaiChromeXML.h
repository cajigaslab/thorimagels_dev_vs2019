#pragma once

class TopticaiChromeXML
{
private:
	ticpp::Document* _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];
	long OpenConfigFile();
	long SaveConfigFile();

	static const char* const CONNECTION;
	enum { NUM_SETTINGS_ATTRIBUTES = 2 };
	static const char* const CONNECTION_ATTR[NUM_SETTINGS_ATTRIBUTES];
	wchar_t _errMsg[MSG_SIZE];

public:
	TopticaiChromeXML();
	~TopticaiChromeXML();
	long GetDeviceConnectionInfo(long& portID, long& baudRate);
};