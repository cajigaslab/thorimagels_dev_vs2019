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


class ThorMCM6000_Condenser : public IDevice
{
public:
	static ThorMCM6000_Condenser* getInstance();
	~ThorMCM6000_Condenser();

	long FindDevices(long& DeviceCount);
	long SelectDevice(const long Device);
	long TeardownDevice();
	long GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double& param);
	long SetParamString(const long paramID, wchar_t* str);
	long GetParamString(const long paramID, wchar_t* str, long size);
	long SetParamBuffer(const long paramID, char* buffer, long size);
	long GetParamBuffer(const long paramID, char* buffer, long size);
	long PreflightPosition();
	long SetupPosition();
	long StartPosition();
	long StatusPosition(long& status);
	long ReadPosition(DeviceType deviceType, double& pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t* msg, long size);

private:
	ThorMCM6000_Condenser();

	static bool _instanceFlag;        ///singleton created flag
	static auto_ptr<ThorMCM6000_Condenser> _single;        ///pointer to internal Device object

	wchar_t _errMsg[MSG_SIZE];

	static CritSect critSec;
	static long _selectedDevice;
	static long _deviceFound;

	void LogMessage(wchar_t* message, long eventType);
	double Round(double number, int decimals);
};