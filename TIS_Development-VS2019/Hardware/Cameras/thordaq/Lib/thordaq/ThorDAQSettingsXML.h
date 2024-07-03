#pragma once
#include "thordaqcmd.h"
class ThorDAQSettingsXML
{
public:
	ThorDAQSettingsXML();
	~ThorDAQSettingsXML();
	long GetDACLastParkingPositions(std::map<UINT, USHORT>& dacParkingPositions);
	long SetDACLastParkingPositions(std::map<UINT, USHORT> dacParkingPositions);
	long GetLastHWConnectionTime(INT64& time);
	long SetLastHWConnectionTime(INT64 time);


private:
	ticpp::Document* _xmlObj;
	static const char* const DAC_PARKING_POSITIONS;

	static const char* const LASTHWCONNECTIONTIME;
	enum { NUM_LASTHWCONNECTIONTIME_ATTRIBUTES = 1 };
	static const char* const LASTHWCONNECTIONTIME_ATTR[NUM_LASTHWCONNECTIONTIME_ATTRIBUTES];
};

