#pragma once

class MLSStageXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	 
	 static const char * const CONNECTION;
	 enum {NUM_CONNECTION_ATTRIBUTES = 3};
	 static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];
	 
	 static const char * const XAXISCONFIG;
	 enum {NUM_XAXISCONFIG_ATTRIBUTES = 1};
	 static const char * const XAXISCONFIG_ATTR[NUM_XAXISCONFIG_ATTRIBUTES];

	 static const char * const YAXISCONFIG;
	 enum {NUM_YAXISCONFIG_ATTRIBUTES = 1};
	 static const char * const YAXISCONFIG_ATTR[NUM_YAXISCONFIG_ATTRIBUTES];

	MLSStageXML();
	~MLSStageXML();

	long GetConnection(long &portID,long &baudRate, long &address);
	long GetXAxisConfig (bool &invert);
	long GetYAxisConfig (bool &invert);

	long OpenConfigFile();
	long SaveConfigFile();

};