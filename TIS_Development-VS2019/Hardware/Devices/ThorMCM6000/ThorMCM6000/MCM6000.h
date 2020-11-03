#pragma once
#include "APT.h"
#include <Windows.h>
#include "..\..\..\..\Common\ThorSharedTypesCPP.h"

#define LOGGING_ENABLED

#ifdef LOGGING_ENABLED
#include "..\..\..\..\Common\Log.h"
static auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

enum InvertedScopeLightPathPos
{
	LEFT = 0,
	MID,
	RIGHT
};

enum BoardTypes
{
	NO_BOARD = 0x8000,
	BOARD_NOT_PROGRAMMED = 0x8001,
	MCM6000_REV_002,
	MCM6000_REV_003,
	OTM_BOARD_REV_001,
	HEXAPOD_BOARD_REV_001,
	MCM6000_REV_004
};

enum CardTypes
{
	NO_CARD_IN_SLOT = 0,
	CARD_IN_SLOT_NOT_PROGRAMMED = 1,
	ST_Stepper_type,
	High_Current_Stepper_Card,
	Servo_type,
	Shutter_type,
	OTM_Dac_type,
	OTM_RS232_type,
	High_Current_Stepper_Card_HD,
	Slider_IO_type,
	Shutter_4_type,
	Piezo_Elliptec_type,
	ST_Invert_Stepper_BISS_type,
	ST_Invert_Stepper_SSI_type
};

class MCM6000
{
private:

	static APT* _apt;
	long SerialPortDisconnect();
	long RequestParams(USHORT, UCHAR, UCHAR);
	long ConfigPid(UCHAR, byte*, bool);
	long ConfigPidKickout(UCHAR, byte*, bool);
	void SerialPort_DataReceived();
	long ReadSettingsFile();
	long InitializeParams();
	long GetHardwareInfo();
	long RequestStageParameters();
	bool IsSlotIdValid(UCHAR);
	static void LogMessage(wchar_t* logMsg, long eventLevel);

	string _serialNumber;
	vector<string> _snList;
	int _baudRate;
	int _ftdiBaudRate;
	string _portId;
	string _ftdiPortId;
	UCHAR _firmwareRev[3]; // minor, interim, major
	UCHAR _cpldRev;
	USHORT _cardType[TOTAL_CARD_SLOTS];
	USHORT _board_type;
	unsigned long _numberOfSetSlots; ///< Number of slots configured in ThorMCM6000Settings.xml
	long _ftdiModeEnabled;

	static HANDLE _hGetStatusThread;
	static bool _requestStageParameters; ///< Boolean flag, set to true if the GetStatusThread is reading stage parameters
	static int _threadDeviceHandler; ///< static int for the device handler, used for differnt threads
	static long _setSlots; ///< Has bits enabled for every slot that was set up in ThorMCM6000Settings.xml
	static unsigned long _responseWaitTime; ///< Global used to set the wait time for any request commands. It is different between serial and FTDI
	static long _stopStatusThread;
	static long _statusThreadStopped;

	int _deviceHandler;
	long _foundUsbSer;
	long _foundFTDI;
	long Home(unsigned char);
	long Zero(unsigned char);
	long Stop(unsigned char);
	long MoveTo(unsigned char, double);
	long MoveBy(unsigned char, double);
	long Jog(unsigned char, unsigned char);
	long SaveJogSize(UCHAR, double);
	long MoveMirror(unsigned char slotId, long state, long mirrorChannel);
	long RecursiveReadData(char* completeBuf, long initialCounter, long extDataSize); //:TODO: Can be used in the future to read status faster
	unsigned long CountSetBits(int value); ///< Function that counts number of enabled bits in a number

	HANDLE GetStatusThread(DWORD& threadID);	///<Start thread that periodically requests status from all boards
	static void GetStatusAllBoards(void* instance); ///<Request the status (current position, is moving), from all set up boards

	CRITICAL_SECTION CriticalSection;
	wchar_t* firmwareVersion;
	wchar_t* serialNumber;
	int firmwareVersionLength;
	int serialNumberLength;
	long UpdateDeviceInfo();
	vector<string> SerialNumbers(string VID, string PID);
	vector<string> SerialNumbersFTDI(string VID, string PID);
	string FindCOMPort(string VID, string PID, string serialNum);
	string FindCOMPortFTDI(string VID, string PID, string serialNum);

	friend APT;
public:
	MCM6000();
	~MCM6000();

	long FirmwareVersion(wchar_t*, int);
	long SerialNumber(wchar_t*, int);
	long Home();
	long HomeX();
	long HomeY();
	long HomeZ();
	long HomeR();
	long Zero();
	long ZeroX();
	long ZeroY();
	long ZeroZ();
	long ZeroR();
	long Stop();
	long StopX();
	long StopY();
	long StopZ();
	long StopR();
	long MoveXTo(double);
	long MoveYTo(double);
	long MoveZTo(double);
	long MoveRTo(double);
	long MoveXBy(double);
	long MoveYBy(double);
	long MoveZBy(double);
	long MoveRBy(double);
	long MoveLpTo(int);
	long MoveEtTo(int);
	long MoveGGLightpath(long);
	long MoveGRLightpath(long);
	long MoveCAMLightpath(long);
	long MoveToStoredPos(int, UCHAR);
	long XJogCW();
	long YJogCW();
	long ZJogCW();
	long RJogCW();
	long XJogCCW();
	long YJogCCW();
	long ZJogCCW();
	long RJogCCW();
	long GetXPos(double&);
	long GetYPos(double&);
	long GetZPos(double&);
	long GetRPos(double&);
	long GetLpPos(long&);
	long GetEtPos(long&);
	long GetZElevatorPos(double&);
	long GetXStatus(long&);
	long GetYStatus(long&);
	long GetZStatus(long&);
	long GetRStatus(long&);
	long Close();
	bool IsConnected();
	long FindAllDevs(long& devCount);
	long SelectAndConnect(const long& dev);
	bool IsXmoving();
	bool IsYmoving();
	bool IsZmoving();
	bool IsRmoving();
	long StatusPosition(long& status);

	static Mcm6kParams* _mcm6kParams;
	ScopeType _scopeType;

	inline double round(double num) {
		return (num > 0.0) ? floor(num + 0.5) : ceil(num - 0.5);
	}
};

