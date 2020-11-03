#pragma once

class ThorConfocalSimulatorXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	 
	 static const char * const CONNECTION;
	 enum {NUM_CONNECTION_ATTRIBUTES = 3};
	 static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];

	ThorConfocalSimulatorXML();
	~ThorConfocalSimulatorXML();

	long GetConnection(string& path, long& rotationPositionlong, long& imageUpdateIntervalMS);
	long GetTilesDimension(string& path, long& row, long& col, long& zSteps);
	long OpenConfigFile(string path);

};