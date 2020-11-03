#pragma once

class ZPiezoXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:

	static const char * const CONVERSION;
	enum {NUM_CONVERSION_ATTRIBUTES = 6};
	static const char * const CONVERSION_ATTR[NUM_CONVERSION_ATTRIBUTES];

	static const char * const DMA;
	enum {NUM_DMA_ATTRIBUTES = 2};
	static const char * const DMA_ATTR[NUM_DMA_ATTRIBUTES];

	static const char * const IO;
	enum {NUM_IO_ATTRIBUTES = 4};
	static const char * const IO_ATTR[NUM_IO_ATTRIBUTES];

	ZPiezoXML();
	~ZPiezoXML();

	long GetConversion(double &volt2mm, double &mmoffset, double &mm_min, double &mm_max, long &stairCaseDelayPercentage, double &pockelsRefThreshold);
	long GetDMA(long &activeLoadMS, long &preLoadCount);
	long GetIO(string &piezoLine, string &analogLine, string &triggerLine, string &pockelsReferenceAnalogLine);
	long OpenConfigFile();
	long SaveConfigFile();
};