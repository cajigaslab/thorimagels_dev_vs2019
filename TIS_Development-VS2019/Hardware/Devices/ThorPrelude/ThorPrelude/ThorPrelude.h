#pragma once
#include "Serial.h"
#include "ParamInfo.h"
#include "stdafx.h"

class ThorPrelude : IDevice
{
#define HOST_ID				0x01
#define MOTHERBOARD_ID		0xd0		// This is the ID 0x50 or'd with 0x80 
#define NUM_TWOWAY_ZONES	251
#define MIN_AMPLITUDE		0
#define MAX_AMPLITUDE		65536
#define CAMERA_LED_MIN		0
#define CAMERA_LED_MAX		4095
#define LED_GUI_MAX			100
#define AMPLITUDE_STEP_SIZE	256			// Scale the Max amplitude to 256 to coincide with the regular GR values
#define PHASE_SCALE			18000		// Scale for the phase value, since it is a huge number, we should scale it down 
#define MIN_PHASE			0
#define MAX_PHASE			2147483647	//In reality the Max is 2 ^ 32. We only need to use half of the rotation (180 degrees) for the alignment. Setting it past 180 degrees flips the scan angle. Could be useful for flipping horizontally

public:
	~ThorPrelude();

	static ThorPrelude* getInstance();
	long FindDevices(long& DeviceCount);
	long SelectDevice(const long Device);
	long TeardownDevice();
	long GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double& param);
	long SetParamBuffer(const long paramID, char* buffer, long size);
	long GetParamBuffer(const long paramID, char* buffer, long size);
	long SetParamString(const long paramID, wchar_t* str);
	long GetParamString(const long paramID, wchar_t* str, long size);
	long PreflightPosition();
	long SetupPosition();
	long StartPosition();
	long StatusPosition(long& status);
	long ReadPosition(DeviceType deviceType, double& pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t* msg, long size);

private:
	ThorPrelude();
	long ExecuteCmd(ParamInfo* pParamInfo);
	long ExecuteCmd(std::vector<unsigned char> cmd, double& readBackValue);
	std::vector<unsigned char> GetBytes(int value);
	long CoarseAlignDataLoadFile();
	long GetFirmwareVersionAndSerialNumber(std::wstring& version, std::wstring& serialNum);
	long CreateParamTable();
	long DestroyParamTable();
	static void LogMessage(wchar_t* logMsg, long eventLevel);
	static bool _instanceFlag;
	static std::shared_ptr <ThorPrelude> _single;
	std::wstring _firmwareVersion;
	std::wstring _serialNumber;
	bool _deviceDetected;
	CSerial _serialPort;
	long* _twoWayZones;///<digital pot zones for correcting two way alignment
	std::map<long, ParamInfo*> _tableParams;
	wchar_t _errMsg[MSG_SIZE];
	CritSect _critSect;
	vector<string> _serialNums;

	const string THORLABS_VID = "1313";
	const string THORLABS_PRELUDE_PID = "2015";

	const USHORT MGMSG_STCK_RES_REQ_PHASE = 0x4373;
	const USHORT MGMSG_STCK_RES_GET_PHASE = 0x4374;
	const USHORT MGMSG_STCK_RES_SET_PHASE = 0x4375;
	const USHORT MGMSG_STCK_RES_REQ_AMPLITUDE = 0x4376;
	const USHORT MGMSG_STCK_RES_GET_AMPLITUDE = 0x4377;
	const USHORT MGMSG_STCK_RES_SET_AMPLITUDE = 0x4378;
	const USHORT MGMSG_STCK_SYS_REQ_SERIAL_NUMBER = 0x4310;
	const USHORT MGMSG_STCK_SYS_GET_SERIAL_NUMBER = 0x4311;
	const USHORT MGMSG_STCK_SYS_SET_FW_UPDATE = 0x430E;
	const USHORT MGMSG_STCK_SYS_REQ_CM7_VERSION = 0x4303;
	const USHORT MGMSG_STCK_SYS_GET_CM7_VERSION = 0x4304;
	const USHORT MGMSG_STCK_SYS_REQ_CM4_VERSION = 0x4306;
	const USHORT MGMSG_STCK_SYS_GET_CM4_VERSION = 0x4307;
	const USHORT MGMSG_STCK_SYS_REQ_CAMERA_LED_CONTROL = 0x4333;
	const USHORT MGMSG_STCK_SYS_GET_CAMERA_LED_CONTROL = 0x4334;
	const USHORT MGMSG_STCK_SYS_SET_CAMERA_LED_CONTROL = 0x4335;

	enum
	{
		SCANNER_ZOOM_MIN = 5,
		SCANNER_ZOOM_MAX = 255,
		SCANNER_ZOOM_DEFAULT = 120
	};
	enum OutputState
	{
		LED_OUTPUT_OFF = 0,
		LED_OUTPUT_ON = 1
	};
};

