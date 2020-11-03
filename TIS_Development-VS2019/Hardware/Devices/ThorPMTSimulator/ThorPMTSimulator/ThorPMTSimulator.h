#pragma once

#include "stdafx.h"
#include "ThorSerialCom.h"

class ThorPMTSimulator : IDevice
{
private:
    ThorPMTSimulator();
public:

	enum
	{
		PMT_GAIN_MIN = 0,
		PMT_GAIN_MAX = 100,
		PMT_GAIN_DEFAULT = 50,
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

	static ThorPMTSimulator* getInstance();
    ~ThorPMTSimulator();

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
long ThorPMTSimulator::SetPMTEnable(int PMTID, bool enable);
long ThorPMTSimulator::SetPMTGain(int PMTID, int gain);
long ThorPMTSimulator::GetPMTEnable(int PMTID, bool &enable);
long ThorPMTSimulator::GetPMTGain(int PMTID, int &gain);
long ThorPMTSimulator::SetCRSEnable(bool enable);
long ThorPMTSimulator::GetCRSEnable(bool &enable);
long ThorPMTSimulator::GetControllerID(char* ctrlID);
long ThorPMTSimulator::ReadPMTPort();
long ThorPMTSimulator::ParseReadBuffer(int inquiryID);
long ThorPMTSimulator::GainPercent2Byte(int percentageGain);
long ThorPMTSimulator::GainByte2Percent(int byteGain);
long ThorPMTSimulator::QueryStatus(void);

private:

	long _pmt1Enable;
	long _pmt2Enable;
	long _pmt3Enable;
	long _pmt4Enable;
	long _pmt1Gain;
	long _pmt2Gain;
	long _pmt3Gain;
	long _pmt4Gain;
	long _crsEnable;
	
	long _pmt1Enable_C;
	long _pmt2Enable_C;
	long _pmt3Enable_C;
	long _pmt4Enable_C;
	long _pmt1Gain_C;
	long _pmt2Gain_C;
	long _pmt3Gain_C;
	long _pmt4Gain_C;
	long _crsEnable_C;

	//bool _r_pmt1Enable;
	//bool _r_pmt2Enable;
	//bool _r_pmt3Enable;
	//bool _r_pmt4Enable;
	//int  _r_pmt1Gain;
	//int  _r_pmt2Gain;
	//int  _r_pmt3Gain;
	//int  _r_pmt4Gain;
	//bool _r_crsEnable;

    static bool _instanceFlag;
    static auto_ptr<ThorPMTSimulator> _single;

	CThorSerialCom _serialPort;


	//CSerial _serialPort;
	int _deviceCount;
	bool _deviceDetected;

	TCHAR _dataBuffer[256];
	TCHAR _readBuffer[256];
};