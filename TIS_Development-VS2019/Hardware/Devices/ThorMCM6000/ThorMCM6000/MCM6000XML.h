#include "msxml6.h"
#include "Strsafe.h"
#include "APT.h"
#include "MCM6000.h"

class MCM6000XML
{
private:
	static void LogMessage(wchar_t* logMsg, long eventLevel);
	unsigned long CountSetBits(int value); ///< Function that counts number of enabled bits in a number
	bool IsSlotIdValid(UCHAR);

public:

	MCM6000XML();
	~MCM6000XML();

	long ReadSettingsFile(Mcm6kParams* params, ScopeType& scopeType, string& portId, int& baudRate, string& ftdiPortId, int& ftdiBaudRate, long& ftdiModeEnabled, long& setSlots, unsigned long& numberOfSetSlots);
	long SaveSlotNameToSettingsFile(Mcm6kParams* params, long& settingsFileChanged);
	long VerifySlotCards(Mcm6kParams* params, ScopeType& scopeType);
};