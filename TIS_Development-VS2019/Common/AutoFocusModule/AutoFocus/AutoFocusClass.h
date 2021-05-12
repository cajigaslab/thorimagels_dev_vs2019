#pragma once

#include "..\..\..\Common\thread.h"


class IAutoFocus
{
public:
    static IAutoFocus* getInstance();
	long SetupAF(long afType, long repeat, double afFocusOffset, double expTimeMS, double stepSizeUM, double startPosMM, double stopPosMM, long binning, double finePercentage, long enableGUIUpdate);
	long RunAF(long index, long afType, BOOL& bFound);//Synchronous execution of autofocus
	long AFExecuteNextIteration(long afType);
	long GetAFStatusAndImage(char* imageBuffer, long& imageAvailable, long& afRunning, long& frameNumber, long& currentRepeat, long& status, long& zSteps, long& currentZIndex);
	long GetAFRunning();
	long StopAF();
	long GetAFStatus(long& currentStatus, long& bestContrastScore, double& bestZPosition, double& nextZPosition, long& currentRepeatIndex);

	static HANDLE hEventZ;

	static long SetAFStartZPosition(double afStartPos, BOOL bWait, BOOL afFound);

	enum
	{
		AF_HARDWARE,
		AF_HARDWARE_IMAGE,
		AF_IMAGE,
		AF_NONE
	};

protected:
	IAutoFocus();
	~IAutoFocus();

private:
	virtual long WillAFExecuteNextIteration();
	virtual long Execute(long index, IDevice* pAutoFocus, BOOL& bFound);//Synchronous execution of autofocus
	
    static bool instanceFlag;
    static IAutoFocus*single;
	static CritSect critSect;
	static void cleanup(void);	
	static double _lastGoodFocusPosition;

	static double _adaptiveOffset;
	static long _selectedAutoFocusType;

	static long AutoFocusAndRetry(long index, IDevice* pAutoFocusDevice, IAutoFocus* autofocusType, BOOL& afFound);

	static long _autoFocusRunning;
};