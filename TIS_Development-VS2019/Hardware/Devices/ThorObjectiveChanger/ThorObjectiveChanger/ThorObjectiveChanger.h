#pragma once

#include "stdafx.h"

using namespace std;

#define SET_JOG_SIZE_OFFSET					10
#define SET_POS_1_OFFSET					10
#define SET_POS_2_OFFSET					14
#define SET_FIRMWARE_UPDATE_OFFSET			14
#define GET_BRD_INFO_BUF_SIZE				90
#define GET_MTR_STAT_BUF_SIZE				20
#define GET_POS_INFO_BUF_SIZE				22
#define DEFAULT_POS							-1

enum class MOVER_LOCATION
{
	HOME,
	POS1,
	POS2,
	HOMING,
	MOVING_POS1,
	MOVING_POS2,
	DISCONNECTED
};

typedef enum 
{
	GET_BRD_INFO,
	GET_MTR_STAT,
	GET_POS_INFO,
	SET_JOG_SIZE,
	SET_POS_1_2,	
	GOTO_POS_1,
	GOTO_POS_2,
	GOTO_HOME,
	JOG_FORWARD,
	JOG_BACKWARD,
	JOG_STOP,
	FIRMWARE_UPDATE,
	CMD_TYPE_SIZE
} MOVER_CMD_TYPE;

struct MTR_STAT
{
	UCHAR	upperLimit	: 1,
			lowerLimit	: 1,
						: 1,
			homed		: 1,
			homing		: 1,
						: 1,
			moving_cw	: 1, 
						: 1,
			moving_ccw	: 1,
						: 1,
			collision	: 1;
};

struct CMD_BUF_LOC
{
	char* cmdTstring;
	ULONG offset;
	ULONG len;
};

class ThorObjectiveChanger : IDevice
{
private:
	ThorObjectiveChanger();
public:

	static ThorObjectiveChanger* getInstance();
	~ThorObjectiveChanger();

	long FindDevices(long &DeviceCount);
	long SelectDevice(const long Device);
	long TeardownDevice();
	long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double &param);
	long PreflightPosition();
	long SetupPosition();
	long StartPosition();
	long StatusPosition(long &status);
	long ReadPosition(DeviceType deviceType, double &pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t *msg, long size);
	long SetParamString(const long paramID, wchar_t * str);
	long GetParamString(const long paramID, wchar_t * str, long size);
	long SetParamBuffer(const long paramID, char * buffer, long size);
	long GetParamBuffer(const long paramID, char * buffer, long size);

	long SetValue(MOVER_CMD_TYPE type, ULONG val1 = 0, ULONG val2 = 0);
	long GetValue(MOVER_CMD_TYPE type);
	long GetSerialNumberString(const long paramID, wchar_t * str, long size);

private:
	long RefreshChangerCurrentPos();
	void LogMessage(wchar_t *message);
	long ParseReadback(MOVER_CMD_TYPE type, UCHAR* rb);
	wstring FindCOMPort(string VID, string PID, string serialNum);
	list<string> SerialNumbers(string VID, string PID);

	vector<SERIAL_PORT_STATUS> _devices;
	int  _selectedDevIndex;
	ULONG _currentPos1, _currentPos2;
	static bool _instanceFlag;
	static auto_ptr<ThorObjectiveChanger> _single;
	wchar_t _errMsg[MSG_SIZE];
	MOVER_LOCATION _moverLoc;
	BOOL _moverLoc_B;
	bool _deviceDetected;
	long _homed;
	list<string> _serialNums;
	CSerial _serialPort;

	const UCHAR			HOST_ID;
	UCHAR				MOTHERBOARD_ID;
	char*				_cmdTBuff;
	ULONG				_cmdTemplateBuffSize;
	const double		UmDriverCountConv;
	const double		UmEncoderCountConv;
	CMD_BUF_LOC			_cmdTemplates[CMD_TYPE_SIZE];

	string				_brdID;
	string				_brdFirmwareRev;
	string				_brdSerialNum;

	double				_umDriverCount;
	double				_umEncoderCount;
	long				_rawDriverCount;
	long				_rawEncoderCount;
	long				_moverPos;
	long				_status;
	MTR_STAT			_mtrStat;

	long				_pos1;
	long				_pos2;

};