#pragma once

class RemoteFocusNIXML
{
private:
	ticpp::Document* _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];

public:
	static const char* const DMA;
	enum { NUM_DMA_ATTRIBUTES = 2 };
	static const char* const DMA_ATTR[NUM_DMA_ATTRIBUTES];

	static const char* const IO;
	enum { NUM_IO_ATTRIBUTES = 3 };
	static const char* const IO_ATTR[NUM_IO_ATTRIBUTES];

	RemoteFocusNIXML();
	~RemoteFocusNIXML();

	long GetDMA(long& activeLoadMS, long& preLoadCount);
	long GetIO(string& devCard, string& analogLine, string& triggerLine);
	long OpenConfigFile();
	long SaveConfigFile();
	long ReadPositionVoltages(long& numberOfPlanes, vector<double>* posVoltages);
};