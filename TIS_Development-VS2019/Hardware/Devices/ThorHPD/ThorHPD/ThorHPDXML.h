#pragma once

class ThorHPDXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	
	long OpenConfigFile();
	long SaveConfigFile();

	enum {NUM_SETTINGS_ATTRIBUTES = 4};
	static const char * const CONNECTION_ATTR[NUM_SETTINGS_ATTRIBUTES];

	static const char * const GAIN;
	enum {NUM_GAIN_ATTRIBUTES = 4};
	static const char * const GAIN_ATTR[NUM_GAIN_ATTRIBUTES];
public:	 
	ThorHPDXML();
	~ThorHPDXML();
	long GetDeviceConnectionInfo(const string signature, long &portID, long &baudRate, string &serialNumber, long &detectorType);
	long GetDetectorDefaultGain(long &gain1, long &gain2, long &gain3, long &gain4);
	long SetDetectorDefaultGain(long gain1, long gain2, long gain3, long gain4);
};