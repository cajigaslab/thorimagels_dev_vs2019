#pragma once

class PinholeStepperXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	 
	 static const char * const CONNECTION;
	 enum {NUM_CONNECTION_ATTRIBUTES = 3};
	 static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];

	 static const char * const PINHOLELOC;
	 enum {NUM_PINHOLELOC_ATTRIBUTES = 1};
	 static const char * const PINHOLELOC_ATTR[NUM_PINHOLELOC_ATTRIBUTES];
	 
	PinholeStepperXML();
	~PinholeStepperXML();

	long GetConnection(long &portID,long &baudRate, long &address);
	long GetPinholeLocation(long id, long &value);
	long SetPinholeLocation(long id, long value);

	long OpenConfigFile();
	long SaveConfigFile();

};