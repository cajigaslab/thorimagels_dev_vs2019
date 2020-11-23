#pragma once

class ThorStimXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:

	static wchar_t _libName[_MAX_PATH];

	static const char * const CONFIGURES;
	enum {NUM_CONFIGURES_ATTRIBUTES = 5};
	static const char * const CONFIGURES_ATTR[NUM_CONFIGURES_ATTRIBUTES];

	static const char * const MODULATION;
	enum {NUM_MODULATION_ATTRIBUTES = 14};
	static const char * const MODULATION_ATTR[NUM_MODULATION_ATTRIBUTES];

	static const char * const WAVEFORM;
	enum {NUM_WAVEFORM_ATTRIBUTES = 9};
	static const char * const WAVEFORM_ATTR[NUM_WAVEFORM_ATTRIBUTES];

	ThorStimXML();
	~ThorStimXML();

	long GetConfigures(long &driverType, long &activeLoadMS, long &activeLoadCount, long &sampleRateKHz, double &riseTimeUS);

	long GetModulations(string &counter, string &triggerIn, 
		string &line1, double &lineMinVoltage1, double &lineMaxVoltage1,
		string &line2, double &lineMinVoltage2, double &lineMaxVoltage2,
		string &line3, double &lineMinVoltage3, double &lineMaxVoltage3,
		string &line4, double &lineMinVoltage4, double &lineMaxVoltage4);

	long SetModulations(double lineMinVoltage1, double lineMaxVoltage1,double lineMinVoltage2, double lineMaxVoltage2,
		double lineMinVoltage3, double lineMaxVoltage3,double lineMinVoltage4, double lineMaxVoltage4);

	long GetWaveform(std::vector<std::string> *digiLines);

	long OpenConfigFile();
	long SaveConfigFile();
};