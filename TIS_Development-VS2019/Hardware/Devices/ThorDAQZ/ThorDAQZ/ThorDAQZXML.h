#pragma once
class ThorDAQZXML
{
private:
	ticpp::Document* _xmlObj;
	string _currentPathAndFile;

	static const char* const CONVERSION;
	enum { NUM_CONVERSION_ATTRIBUTES = 4 };
	static const char* const CONVERSION_ATTR[NUM_CONVERSION_ATTRIBUTES];

	static const char* const DMA;
	enum { NUM_DMA_ATTRIBUTES = 2 };
	static const char* const DMA_ATTR[NUM_DMA_ATTRIBUTES];

	static const char* const SETTLING_CONFIG;
	enum { NUM_SETTLING_CONFIG_ATTRIBUTES = 2 };
	static const char* const SETTLING_CONFIG_ATTR[NUM_SETTLING_CONFIG_ATTRIBUTES];
public:
	ThorDAQZXML();
	~ThorDAQZXML();

	long GetConversion(double& volt2mm, double& mmoffset, double& mm_min, double& mm_max);
	long GetDMA(long& activeLoadMS, long& preLoadCount);
	long GetsettlingConfig(double& settlingTimeTypicalMove, double& typicalMove_mm);
};

