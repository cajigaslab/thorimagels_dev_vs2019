#pragma once

class ThorMCLSXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	 
	 static const char * const CONNECTION;
	 enum {NUM_CONNECTION_ATTRIBUTES = 1};
	 static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];

	ThorMCLSXML();
	~ThorMCLSXML();

	long GetConnection(long &portID);

	long OpenConfigFile(string path);

};