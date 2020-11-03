#include "stdafx.h"
#include "AcquireDataFactory.h"
#include "AcquireNIData.h"
#include "AcquireSim.h"

//private constructor:
AcquireDataFactory::AcquireDataFactory()
{
	ChannelCenter::getInstance()->LoadXML(FALSE);

	//build allowed board names:
	_allowNIBoardNames.push_back("NI6363");
	_allowNIBoardNames.push_back("NI6363-USB");
	_allowNIBoardNames.push_back("NI6361");
	_allowNIBoardNames.push_back("NI6321");
	_allowNIBoardNames.push_back("NI6341");
}

AcquireDataFactory::~AcquireDataFactory()
{
	_instanceFlag = false;
	_acqInstanceFlag = false;

	if(_allowNIBoardNames.size()>0)
	{
		_allowNIBoardNames.clear();
	}
}

//singleton:
AcquireDataFactory* AcquireDataFactory::getInstance()
{
	if(! _instanceFlag)
	{
		_single.reset(new AcquireDataFactory());
		_instanceFlag = true;
	}
	return _single.get();
}

///*** static members		***///
bool AcquireDataFactory::_instanceFlag = false;
bool AcquireDataFactory::_acqInstanceFlag = false;
std::unique_ptr<AcquireDataFactory> AcquireDataFactory::_single;
std::unique_ptr<IAcquireRealTimeData> AcquireDataFactory::_singleAcq;
std::vector<std::string> AcquireDataFactory::_allowNIBoardNames;

///*** end static members	***///

//singleton of IAcquireRealTimeData:
IAcquireRealTimeData* AcquireDataFactory::getAcquireInstance()
{
	if(!_acqInstanceFlag)
	{		
		long type = 0;
		for(int i=0;i<_allowNIBoardNames.size();i++)
		{
			if(0 == ChannelCenter::getInstance()->_board.name.compare(_allowNIBoardNames.at(i)))
			{
				type = 1;
			}
		}
		if(0 == ChannelCenter::getInstance()->_board.name.compare("Simulator"))
		{
			type = 2;
		}
		switch(type)
		{
		case 1:
			{
				_singleAcq.reset(new AcquireNIData());
				_acqInstanceFlag = true;
			}
			break;
		case 2:
			{
				_singleAcq.reset(new AcquireSim());
				_acqInstanceFlag = true;
			}
			break;
		}
	}
	return _singleAcq.get();
}
