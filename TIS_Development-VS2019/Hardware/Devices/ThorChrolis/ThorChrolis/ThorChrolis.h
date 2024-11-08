#ifndef __ThorChrolis_H__
#define __ThorChrolis_H__

#include "stdafx.h"

#define MSG_SIZE	256

class  CThorChrolis : IDevice
{
public:
	// Get the Singleton Instance
	static CThorChrolis* getInstance(void);

	~CThorChrolis(void);

	// Inherited from IDevice
	long FindDevices(long & deviceCount) override;
	long SelectDevice(const long device) override;
	long TeardownDevice(void) override;

	long GetLastErrorMsg(wchar_t * msg, long size) override;
	long GetParamInfo(const long paramID, long & paramType,
		long & paramAvailable, long & paramReadOnly,
		double & paramMin, double & paramMax,
		double & paramDefault) override;

	long SetParam(const long paramID, const double param) override;
	long GetParam(const long paramID, double & param) override;
	long SetParamString(const long paramID, wchar_t * str) override;
	long GetParamString(const long paramID, wchar_t * str, long size) override;
	long SetParamBuffer(const long paramID, char * buffer, long size) override;
	long GetParamBuffer(const long paramID, char * buffer, long size) override;

	long PreflightPosition(void) override;
	long SetupPosition(void) override;
	long StartPosition(void) override;
	long StatusPosition(long & status) override;
	long PostflightPosition(void) override;

	long ReadPosition(DeviceType deviceType, double & pos) override;

private:
	static bool _instanceFlag;
	static std::auto_ptr<CThorChrolis> _singleton;
	static HANDLE _hGetAdaptationStatus;	
	static int _ledIndex;
	bool _externalMode1 = FALSE;
	bool _externalMode2 = FALSE;
	bool _externalMode3 = FALSE;
	bool _externalMode4 = FALSE;
	bool _externalMode5 = FALSE;
	bool _externalMode6 = FALSE;


	wchar_t _errMsg[MSG_SIZE];                   // error message written to the log file
	long _runInAdaptationMode;
	long _ledsEnabled;

	bool _led1State;
	bool _led2State;
	bool _led3State;
	bool _led4State;
	bool _led5State;
	bool _led6State;
	/*
	short _led1Power;
	short _led2Power;
	short _led3Power;
	short _led4Power;
	short _led5Power;
	short _led6Power;*/

	double _led1Temp; 
	double _led2Temp; 
	double _led3Temp;
	double _led4Temp;
	double _led5Temp;
	double _led6Temp;

	long _led1NominalWavelength;
	long _led2NominalWavelength;
	long _led3NominalWavelength;
	long _led4NominalWavelength;
	long _led5NominalWavelength;
	long _led6NominalWavelength;

	CThorChrolis(void);

	CritSect _critSect;

	long GetLEDSerialNumber(const long paramID, char *  pStr);
	long GetLEDControlName(const long paramID, char *  pStr);
	long GetLEDHeadsColorName(const long paramID, char *  pStr);

	long GetLEDPeakWaveLength(const long paramID, double & param);
	long GetLEDLightColor(const long paramID, double & param);
	long UpdateLEDTemperatures();

	long GetLEDPowerState(const long paramID, double & param);
	long SetLEDPowerState(const double state);

	long GetLEDPower(const long paramID, double & param);
	long SetLEDPower(const long paramID, const double param);

	long GetLinearValue(const long paramID, double & param);
	long SetLinearValue(const double param);

	long RunAdaptationMode(long ledIndex);

	long SetExternal(long ledIndex, bool param);

	HANDLE GetAdaptationStatus(DWORD &threadID);	///<Start thread that will read the status of the adaptation
	static void GetAdaptionProgress(void *instance); ///<Request the progress value of the adaption process

	static void LogMessage(wchar_t *message, long eventLevel);
};

#endif __ThorChrolis_H__
