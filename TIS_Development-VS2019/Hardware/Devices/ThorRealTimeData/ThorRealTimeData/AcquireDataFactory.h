#pragma once

#include "AcquireData.h"
#include "RealTimeDataXML.h"

class AcquireDataFactory
{
private:
	static bool _instanceFlag;
	static bool _acqInstanceFlag;

	static std::unique_ptr<AcquireDataFactory> _single;
	static std::unique_ptr<IAcquireRealTimeData> _singleAcq;
	static std::vector<std::string> _allowNIBoardNames;

	AcquireDataFactory();

public:
	~AcquireDataFactory();
	static AcquireDataFactory* getInstance();
	static IAcquireRealTimeData* getAcquireInstance();
};
