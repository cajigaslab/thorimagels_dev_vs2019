#pragma once

class ElectroPhysXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:

	static const char * const IO;
	enum {NUM_IO_ATTRIBUTES = 2};
	static const char * const IO_ATTR[NUM_IO_ATTRIBUTES];


	static const char * const DIG_OUTPUT;
	enum {NUM_DIG_OUTPUT_ATTRIBUTES = 8};
	static const char * const DIG_OUTPUT_ATTR[NUM_DIG_OUTPUT_ATTRIBUTES];

	static const char * const TRIGGER;
	enum {NUM_TRIGGER_ATTRIBUTES = 6};
	static const char * const TRIGGER_ATTR[NUM_TRIGGER_ATTRIBUTES];

	static const char * const FREQPROBE;
	enum {NUM_FREQPROBE_ATTRIBUTES = 4};
	static const char * const FREQPROBE_ATTR[NUM_FREQPROBE_ATTRIBUTES];

	ElectroPhysXML();
	~ElectroPhysXML();

	long GetIO(std::string &devName, std::string &digitalPort);	
	long GetDigOutput(std::string &port1, std::string &port2, std::string &port3, std::string &port4, std::string &port5, std::string &port6, std::string &port7, std::string &port8);
	long GetFrequencyProbe(double &probeIntervalSec, long &averageCount, std::string &counterLine, std::string &measureLine);
	long GetTriggerConfig(long &activeLoadCount, std::vector<std::string> *triggerConfig);
	long OpenConfigFile();
	long SaveConfigFile();

};