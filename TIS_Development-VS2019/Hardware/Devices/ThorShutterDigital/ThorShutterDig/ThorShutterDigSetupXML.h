#pragma once

class ThorShutterDigXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	 
	 static const char * const IO;
	 enum {NUM_IO_ATTRIBUTES = 2};
	 static const char * const IO_ATTR[NUM_IO_ATTRIBUTES];

	ThorShutterDigXML();
	~ThorShutterDigXML();

	long GetIO(string &piezoLine, long &shutterDelayMS);
	long OpenConfigFile();
	long SaveConfigFile();

};