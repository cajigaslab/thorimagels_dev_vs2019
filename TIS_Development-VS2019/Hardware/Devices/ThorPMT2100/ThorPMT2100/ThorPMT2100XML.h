#pragma once

class ThorPMT2100XML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	
	long OpenConfigFile();
	enum {NUM_SETTINGS_ATTRIBUTES = 1};
	static const char * const CONNECTION_ATTR[NUM_SETTINGS_ATTRIBUTES];
public:	 
	ThorPMT2100XML();
	~ThorPMT2100XML();
	long GetDeviceAddress(const string signature, string &serialNumber);
};