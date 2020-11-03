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

	ZStepperXML();
	~ZStepperXML();

	long GetConnection(long &portID,long &baudRate, long &address);

	long OpenConfigFile(string path);

};