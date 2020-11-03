#pragma once

class ThorShutterDig4XML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	 
	 static const char * const IO;
	 enum {NUM_IO_ATTRIBUTES = 2};
	 static const char * const IO_ATTR[NUM_IO_ATTRIBUTES];

	ThorShutterDig4XML();
	~ThorShutterDig4XML();

	long GetIO(string &piezoLine, long &shutterDelayMS);
	long OpenConfigFile();
	long SaveConfigFile();

};