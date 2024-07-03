#pragma once
#include "thordaqcmd.h"
#include <queue>
enum class AO : ULONG
{
	GR_Y = 0,
	GR_P0,
	GR_P1,
	GR_P2,
	GR_P3,
	GG0_X,
	GG0_Y,
	GG0_P0,
	GG0_P1,
	GG0_P2,
	GG0_P3,
	GG1_X,
	GG1_Y,
	GG1_P0,
	GG1_P1,
	GG1_P2,
	GG1_P3,
	Z,
	REMOTE_FOCUS,
	LAST_AO
};

enum class DIOSettingsType : ULONG
{
	GR = 0,
	GG,
	STIM
};


class ThorDAQIOXML
{
public:

	ThorDAQIOXML();
	~ThorDAQIOXML();
	long GetDIOLinesConfiguration(std::map<UINT8, long>& digitalIOSelection);
	long GetAOLinesConfiguration(std::map<AO, long>& aoSelection);
	long GetDIOSettings(long& captureActiveInvert);
	long ConfigDigitalLines(std::map<UINT8, long> digitalIOSelection, THORDAQ_BOB_TYPE bobType, DIOSettingsType settingsType, std::vector<string>& dioConfig);

private:
	std::string _currentPathAndFile;
	ticpp::Document* _xmlObj;
	static const char* const DIOCONFIG;
	static const char* const DIOCONFIG_ATTR[DIO_LINES::LAST_LINE];

	static const char* const AOCONFIG;
	static const char* const AOCONFIG_ATTR[(int)AO::LAST_AO];

	static const char* const DIOSettings;
	enum { NUM_DIO_SETTINGS_ATTRIBUTES = 1 };
	static const char* const DIOSETTINGS_ATTR[NUM_DIO_SETTINGS_ATTRIBUTES];

	UINT8 GetDigLineSelection(UINT8 lineIndex, long dbb1Selection, THORDAQ_BOB_TYPE bobType);
	long ConvertDBBSelectionToDIO(long lineIndex, THORDAQ_BOB_TYPE bobType);
};

