#include "msxml6.h"
#include "Strsafe.h"
#include "APT.h"
#include "MCM301.h"

class MCM301XML
{
private:
	static void LogMessage(wchar_t* logMsg, long eventLevel);
	unsigned long CountSetBits(int value); ///< Function that counts number of enabled bits in a number
	bool IsSlotIdValid(UCHAR);

public:

	MCM301XML();
	~MCM301XML();

	long ReadSettingsFile(Mcm301Params* params, ScopeType& scopeType, string& portId, int& baudRate, long& setSlots, unsigned long& numberOfSetSlots);
	long SaveSlotNameToSettingsFile(Mcm301Params* params, long& settingsFileChanged);
	long SaveSerialNoToSettingsFile(Mcm301Params* mcm301Params);
	long VerifySlotCards(Mcm301Params* params, ScopeType& scopeType);
};