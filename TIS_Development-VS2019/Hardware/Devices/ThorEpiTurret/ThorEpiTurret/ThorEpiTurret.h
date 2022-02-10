#pragma once


#define MOBO_ID		0x50
#define HOST_ID		 0x01
#define CMD_LENGTH	6
#define BUF_LENGTH 100
typedef enum
{
	GET_SERIAL_NUM,
	GET_HARDWARE_INFO,
	GET_CURRENT_STAT,
	TOTAL_CMD_TYPES
}EPI_TURRET_CMD_TYPE;


class ThorEpiTurret : public IDevice
{
public:
	static ThorEpiTurret* getInstance();
	~ThorEpiTurret();

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
	long ReadPosition(DeviceType deviceType,double &pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t *msg, long size);

private:
	ThorEpiTurret();
	long GetFirmwareVersionAndSerialNumber(std::wstring &version,std::wstring &serialNum);
	enum
	{
		EpiTur_1 = 0,
		DEVICE_NUM = 1,
	};

	static bool _instanceFlag;        ///singleton created flag
	static auto_ptr<ThorEpiTurret> _single;        ///pointer to internal Device object

	const long WAIT_TIME_BETWEEN_SEND_COMMANDS;
	wchar_t _errMsg[MSG_SIZE];

	CSerial _serialPort[DEVICE_NUM];
	BOOL _deviceDetected[DEVICE_NUM + 1];
	long _timeOutTime;
	string _settingsSerialNumber[DEVICE_NUM];

	static CritSect critSec;

	map<long, ParamInfo *> _tableParams;

	long BuildParamTable();
	long ExecuteCmd(ParamInfo *pParamInfo);
	long ExecuteCmd(long paramID, std::vector<unsigned char> cmd, double &readBackValue);
	void LogMessage(wchar_t *message);
	double Round(double number, int decimals);

	long GetDeviceSerialNumber(long deviceIndx, string &deviceSerialNumber);
	long VerifySerialNumbers(long portID[]);
	bool BlockUntilDataArrives(CSerial & serialPort, int numberOfBytes, long maxTimeInMillis, long pollingFrequencyInMillis);
	long ClearSerialBuffer(CSerial & serialPort);
	std::wstring _firmwareVersion;
	std::wstring _serialNumber;
	long _setToBootloaderMode;
};