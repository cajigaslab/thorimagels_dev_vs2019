#pragma once
#include <codecvt>
#include <Windows.h>
#include "APT.h"
#include "..\..\..\..\Common\ThorSharedTypesCPP.h"
#include "..\..\..\..\Common\thread.h"

#define LOGGING_ENABLED
#define BUFFER_LENGTH	255
#define STATIC_CARD_SLOT_MASK    0x8000

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
	ST_Stepper_type,                // 2 
	High_Current_Stepper_Card,      // 3 42-0093 MCM6000 HC Stepper Module
	Servo_type,                     // 4 42-0098 MCM Servo Module
	Shutter_type,                   // 5 42-0098 MCM Servo Module
	OTM_Dac_type,                   // 6
	OTM_RS232_type,                 // 7
	High_Current_Stepper_Card_HD,   // 8 42-0107 MCM6000 HC Stepper Module Micro DB15
	Slider_IO_type,                 // 9
	Shutter_4_type,                 // 10 42-0108 MCM6000 4 Shutter Card PCB
	Piezo_Elliptec_type,            // 11
	ST_Invert_Stepper_BISS_type,    // 12 42-0113 MCM Stepper ABS BISS encoder Module
	ST_Invert_Stepper_SSI_type,     // 13 42-0113 MCM Stepper ABS BISS encoder Module
	Piezo_Type,                     // 14 42-0123 Piezo Slot Card PCB
	MCM_Stepper_Internal_BISS_L6470,// 15 41-0128 MCM Stepper Internal Slot Card
	MCM_Stepper_Internal_SSI_L6470, // 16 41-0128 MCM Stepper Internal Slot Card
	MCM_Stepper_L6470_MicroDB15,    // 17 41-0129 MCM Stepper LC Micro DB15
	Shutter_4_type_REV6,     	    // 18 42-0108 MCM6000 4 Shutter Card PCB REV 6
	MAX_CARD_TYPE,
	END_CARD_TYPES = 0xffff    // make this enum 16bit
};

enum Piezo_modes
{
	PZ_STOP_MODE = 0,
	PZ_ANALOG_INPUT_MODE = 1,
	SET_DAC_VOLTS = 2,
	PZ_CONTROLER_MODE = 3,
	PZ_PID_MODE = 4,
	PZ_HYSTERESIS_MODE = 5,
};

class MCM6000
{
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
	long HomeCondenser();
	long Zero();
	long ZeroX();
	long ZeroY();
	long ZeroZ();
	long ZeroR();
	long ZeroCondenser();
	long Stop();
	long StopX();
	long StopY();
	long StopZ();
	long StopR();
	long StopCondenser();
	long MoveXTo(double);
	long MoveYTo(double);
	long MoveZTo(double);
	long MoveRTo(double);
	long MoveCondenserTo(double);
	long MoveXBy(double);
	long MoveYBy(double);
	long MoveZBy(double);
	long MoveRBy(double);
	long MoveCondenserBy(double);
	long MoveLpTo(int);
	long MoveEtTo(int);
	long MoveNDDTo(int);
	long MoveGGLightpath(long);
	long MoveGRLightpath(long);
	long MoveCAMLightpath(long);
	long MoveToStoredPos(int, UCHAR);
	long XJogCW();
	long YJogCW();
	long ZJogCW();
	long RJogCW();
	long CondenserJogCW();
	long XJogCCW();
	long YJogCCW();
	long ZJogCCW();
	long RJogCCW();
	long CondenserJogCCW();
	long GetXPos(double&);
	long GetYPos(double&);
	long GetZPos(double&);
	long GetRPos(double&);
	long GetCondenserPos(double&);
	long GetLpPos(long&);
	long GetEtPos(long&);
	long GetZElevatorPos(double&);
	long GetNDDPos(long&);
	long Close();
	bool IsConnected();
	long FindAllDevs(long& devCount);
	long SelectAndConnect(const long& dev);
	bool IsXmoving();
	bool IsYmoving();
	bool IsZmoving();
	bool IsRmoving();
	bool IsCondenserMoving();
	bool IsZEmoving();
	bool IsLighPathMoving();
	bool IsEpiTurretMoving();
	bool IsNDDmoving();
	long StatusPosition(long& status);

	static Mcm6kParams* _mcm6kParams;
	ScopeType _scopeType;

	inline double round(double num) {
		return (num > 0.0) ? floor(num + 0.5) : ceil(num - 0.5);
	}

private:

	static APT* _apt;
	long SerialPortDisconnect();
	long RequestParams(USHORT, UCHAR, UCHAR);
	long ConfigPid(UCHAR, BYTE*, bool);
	long ConfigPidKickout(UCHAR, BYTE*, bool);
	void SerialPort_DataReceived();
	long InitializeParams();
	long GetHardwareInfo();
	long RequestStageParameters();
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
	static long _settingsFileChanged;
	static CritSect _critSect;

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
	long PiezoSetMode(long mode);
	long PiezoRequestMode(long& mode);
	long RecursiveReadData(char* completeBuf, long initialCounter, long extDataSize); //:TODO: Can be used in the future to read status faster

	HANDLE GetStatusThread(DWORD& threadID);	///<Start thread that periodically requests status from all boards
	static void GetStatusAllBoards(void* instance); ///<Request the status (current position, is moving), from all set up boards

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
};

