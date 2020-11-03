#pragma once

#include "stdafx.h"
#include "ThorSerialCom.h"

class ThorPMT2 : IDevice
{
private:
    ThorPMT2();
public:

	enum
	{
		PMT_GAIN_MIN = 0,
		PMT_GAIN_MAX = 100,
		PMT_GAIN_DEFAULT = 50,
	};

	enum
	{
		PMT3GAIN = 0x01,
		PMT4GAIN = 0x02,
		PMT3ENABLE = 0x10,
		PMT4ENABLE = 0x12,
		PMTSTATUS = 0x20,
		SCANNERENABLE = 0x21
	};

	static ThorPMT2* getInstance();
    ~ThorPMT2();

	long FindDevices(long &DeviceCount);
	long SelectDevice(const long Device);
	long TeardownDevice();
	long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double &param);
	long SetParamBuffer(const long paramID, char * buffer, long size);
	long GetParamBuffer(const long paramID, char * buffer, long size);
	long SetParamString(const long paramID, wchar_t * str);
	long GetParamString(const long paramID, wchar_t * str, long size);
	long PreflightPosition();
	long SetupPosition();
	long StartPosition();
	long StatusPosition(long &status);
	long ReadPosition(DeviceType deviceType, double &pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t *msg, long size);

private:
long ThorPMT2::SetPMTEnable(int PMTID, bool enable);
long ThorPMT2::SetPMTGain(int PMTID, int gain);
long ThorPMT2::GetPMTEnable(int PMTID, bool &enable);
long ThorPMT2::GetPMTGain(int PMTID, int &gain);
long ThorPMT2::SetCRSEnable(int enable);
long ThorPMT2::GetCRSEnable(int &enable);
long ThorPMT2::GetControllerID(char* ctrlID);
long ThorPMT2::ReadPMTPort();
long ThorPMT2::ParseReadBuffer(int inquiryID);
long ThorPMT2::GainPercent2Byte(int percentageGain);
long ThorPMT2::GainByte2Percent(int byteGain);
long ThorPMT2::QueryStatus(void);

private:

	long _pmt3Enable;
	long _pmt4Enable;
	long _pmt3Gain;
	long _pmt4Gain;
	
	long _pmt3Enable_C;
	long _pmt4Enable_C;
	long _pmt3Gain_C;
	long _pmt4Gain_C;

	bool _r_pmt3Enable;
	bool _r_pmt4Enable;
	int  _r_pmt3Gain;
	int  _r_pmt4Gain;
	bool _r_crsEnable;
	bool _r_pmtError;

    static bool _instanceFlag;
    static auto_ptr<ThorPMT2> _single;

	CThorSerialCom _serialPort;

	CritSect _critSect;

	wchar_t _errMsg[MSG_SIZE];

	//CSerial _serialPort;

	bool _deviceDetected;

	TCHAR _dataBuffer[256];
	TCHAR _readBuffer[256];
};