#pragma once
class CMaiTaiLaserSetupXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	CMaiTaiLaserSetupXML(void);
	~CMaiTaiLaserSetupXML(void);


	static const char * const CONNECTION;
	 enum {NUM_CONNECTION_ATTRIBUTES = 2};
	 static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];

	long GetConnection(long &portID,long &baudRate);
	
	long OpenConfigFile();
	long SaveConfigFile();
};

