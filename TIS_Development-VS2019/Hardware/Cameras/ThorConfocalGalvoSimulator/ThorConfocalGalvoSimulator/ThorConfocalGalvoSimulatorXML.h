#pragma once

class ThorConfocalGalvoSimulatorXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	 
	 static const char * const CONNECTION;
	 enum {NUM_CONNECTION_ATTRIBUTES = 2};
	 static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];

	ThorConfocalGalvoSimulatorXML();
	~ThorConfocalGalvoSimulatorXML();

	long GetConnection(string& path, long& imageUpdateIntervalMS);
	long GetTilesDimension(string& path, long& row, long& col, long& zSteps);
	long OpenConfigFile(string path);

};