#pragma once

#include "stdafx.h"
#include "ThorSerialCom.h"
#include "ParamInfo.h"
#include "..\..\..\..\Common\thread.h"
#include <list>
#include <map>
#include <vector>
#include <algorithm>


class ThorECU : IDevice
{
private:
    ThorECU();
public:

	enum
	{
		PMT_TYPES_NUM = 2,
		PMT_NUM = 4
	};

	enum
	{
		PMT_GAIN_MIN = 0,
		PMT_GAIN_MAX = 100,
		PMT_GAIN_DEFAULT = 0,

		SCANNER_ZOOM_MIN = 5,
		SCANNER_ZOOM_MAX = 255,
		SCANNER_ZOOM_DEFAULT = 120
	};

	enum
	{
		PMT1GAIN = 0x01,
		PMT2GAIN = 0x02,
		PMT3GAIN = 0x03,
		PMT4GAIN = 0x04,
		PMT1ENABLE = 0x10,
		PMT2ENABLE = 0x12,
		PMT3ENABLE = 0x13,
		PMT4ENABLE = 0x14,
		PMTSTATUS = 0x20,
		SCANNERENABLE = 0x21
	};

	static ThorECU* getInstance();
    ~ThorECU();

	long FindDevices(long &DeviceCount);
	long SelectDevice(const long Device);
	long TeardownDevice();
	long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double &param);
	long SetParamString(const long paramID, wchar_t * str);
	long GetParamString(const long paramID, wchar_t * str, long size);
	long SetParamBuffer(const long paramID, char * buffer, long size);
	long GetParamBuffer(const long paramID, char * buffer, long size);
	long PreflightPosition();
	long SetupPosition();
	long StartPosition();
	long StatusPosition(long &status);
	long ReadPosition(DeviceType deviceType, double &pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t *msg, long size);

private:
	long ReadECUPort(long timeout);
	long ParseReadBufferStatus(int inquiryID);
	long GainPercent2Byte(int PMTID, int percentageGain);
	long GainByte2Percent(int PMTID, int byteGain);
	long QueryPMTStatus(ParamInfo * pParamInfo);
	long ReadHelpCommandSet();
	long ParseHelpResponse();
	long UpdateParameterMapEntry(long paramID, long paramValue);
	long CoarseAlignDataLoadFile();
	long FineAlignDataLoadFile();

	long CreateParamTable();
	long DestroyParamTable();
	long ExecuteCmdStr(ParamInfo *pParamInfo);
	long DisablePMTFor(const long paramID);
	void LogMessage(wchar_t *message);


private:
	std::list<ParamInfo *> _tableParams;
	std::map<const long, long> _parameterMap;	//store the relavant value displayed in UI
	std::map<const long, long> _gainEnableMap; //store the PMT_Enable paramID associated with PMT_Gain paramID	
	
	long _pmt1GainMax_R; //real pmt 1 gain max value
	long _pmt2GainMax_R; //real pmt 2 gain max value
	long _pmt3GainMax_R; //real pmt 3 gain max value
	long _pmt4GainMax_R; //real pmt 4 gain max value
	long _scannerZoomMax_R; //real zoom max value 1023 for 4 channel new ECUs
	long _scanAlignMax_R; //real scan alignment max value 255
	long _rsInitMode;	// resonance scanner mode: on  (for ever) or off

	bool _r_pmt1Enable;
	bool _r_pmt2Enable;
	bool _r_pmt3Enable;
	bool _r_pmt4Enable;
	int  _r_pmt1Gain;
	int  _r_pmt2Gain;
	int  _r_pmt3Gain;
	int  _r_pmt4Gain;
	bool _r_crsEnable;
	int  _r_crsZoom;
	bool _r_pmtError;

    static bool _instanceFlag;
    static auto_ptr<ThorECU> _single;

	CThorSerialCom _serialPort;

	wchar_t _errMsg[MSG_SIZE];

	//CSerial _serialPort;

	bool _deviceDetected;

	TCHAR _dataBuffer[READ_BUFFER_SIZE];
	TCHAR _readBuffer[READ_BUFFER_SIZE];

	static CritSect _critSect;

	const int NUM_TWOWAY_ZONES;

	long* _twoWayZones;///<digital pot zones for correcting two way alignment
	long* _twoWayZonesFine;
	long _pmtType[PMT_NUM]; //PMT type for each of the 4 PMTs

	static const long _pmtGainCountsMax[PMT_TYPES_NUM]; //One max gain counts per PMT type
	static const double _pmtGainPower[PMT_TYPES_NUM]; //One GainPower per PMT type
	static const double _pmtGainMaxVolts[PMT_TYPES_NUM]; //One max gain Volts per PMT type
	static const double _pmtGainMinVolts[PMT_TYPES_NUM]; //One min gain Volts per PMT type
};