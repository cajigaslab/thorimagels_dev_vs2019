#include "windows.h"
#include "Dbt.h"
#include "objbase.h"
#include "initguid.h"
#include "Setupapi.h"


#ifdef _UNICODE
#define _T(x)      L ## x
#else /* _UNICODE */
#define _T(x)      x
#endif /* _UNICODE */

#define OTM_ID 1
#define PCU2A_ID 2
#define VCMCCU_ID 4
#define DEVICE_NOCHANGE 0
#define DEVICE_REMOVAL 1
#define DEVICE_ARRIVAL 2
#define MAX_DEVICE_NUM 10
#define DEVICE_ID_LENGTH 16

const byte HOST_ID = 0x01;
//parameters
const byte NUM_OF_SLOTS = 7;

//byte *card_type = new byte[NUM_OF_SLOTS];

        

       
        const byte SLOT1 = 0;
        const byte SLOT2 = 1;
        const byte SLOT3 = 2;
        const byte SLOT4 = 3;
        const byte SLOT5 = 4;
        const byte SLOT6 = 5;
        const byte SLOT7 = 6;

        const byte NO_CARD_IN_SLOT = 0;
        const byte MCM001_Stepper = 1;
        const byte MCM002_Servo = 2;
        const byte MCM007_Shutter = 3;


        // static int[] type = new int[NUM_OF_SLOTS];
        static double* quad_pos = new double[NUM_OF_SLOTS];

        
        const byte NO_LIMIT = 0x01;
        const byte MAKES_ON_CONTACT = 0x02;
        const byte BREAKS_ON_CONTACT = 0x03;
        const byte MAKES_HOMING_ONLY = 0x04;
        const byte BREAKS_HOMING_ONLY = 0x05;
        const byte REVERSE_LIMITS = 0x80;

        const byte HOME_TO_INDEX = 1;
        const byte HOME_TO_LIMIT = 2;

        const byte HOME_CW = 1;
        const byte HOME_CCW = 2;
        const byte HOME_CW_FIRST = 3;
        const byte HOME_CW_TILL_STALL = 5;
        const byte HOME_CCW_TILL_STALL = 6;

        const int MGMSG_HW_REQ_INFO = 0x0005;
        const int MGMSG_HW_GET_INFO = 0x0006;
        const int MGMSG_MOD_SET_CHANENABLESTATE = 0x0210;
        const int MGMSG_MOD_REQ_CHANENABLESTATE = 0x0211;
        const int MGMSG_MOD_GET_CHANENABLESTATE = 0x0212;
        const int MGMSG_MOT_SET_POSCOUNTER = 0x0410;
        const int MGMSG_MOT_REQ_POSCOUNTER = 0x0411;
        const int MGMSG_MOT_GET_POSCOUNTER = 0x0412;
        const int MGMSG_MOT_SET_ENCCOUNTER = 0x0409;
        const int MGMSG_MOT_REQ_ENCCOUNTER = 0x040A;
        const int MGMSG_MOT_GET_ENCCOUNTER = 0x040B;
        const int MGMSG_MOT_SET_POWERPARAMS = 0x0426;
        const int MGMSG_MOT_REQ_POWERPARAMS = 0x0427;
        const int MGMSG_MOT_GET_POWERPARAMS = 0x0428;
        const int MGMSG_MOT_SET_HOMEPARAMS = 0x0440;
        const int MGMSG_MOT_REQ_HOMEPARAMS = 0x0441;
        const int MGMSG_MOT_GET_HOMEPARAMS = 0x0442;
        const int MGMSG_MOT_SET_PMDJOYSTICKPARAMS = 0x04E6;
        const int MGMSG_MOT_REQ_PMDJOYSTICKPARAMS = 0x04E7;
        const int MGMSG_MOT_GET_PMDJOYSTICKPARAMS = 0x04E8;
        const int MGMSG_MOT_SET_GENMOVEPARAMS = 0x043A;
        const int MGMSG_MOT_REQ_GENMOVEPARAMS = 0x043B;
        const int MGMSG_MOT_GET_GENMOVEPARAMS = 0x043C;
        const int MGMSG_MOT_MOVE_HOME = 0x0443;
        const int MGMSG_MOT_MOVE_RELATIVE = 0x0448;
        const int MGMSG_MOT_MOVE_ABSOLUTE = 0x0453;
        const int MGMSG_MOT_MOVE_JOG = 0x046A;
        const int MGMSG_MOT_MOVE_VELOCITY = 0x0457;
        const int MGMSG_MOT_MOVE_STOP = 0x0465;
        const int MGMSG_MOT_REQ_STATUSUPDATE = 0x0480;
        const int MGMSG_MOT_GET_STATUSUPDATE = 0x0481;
        const int MGMSG_MOT_SET_EEPROMPARAMS = 0x04B9;
        const int MGMSG_MOT_REQ_EEPROMPARAMS = 0x04BA;
        const int MGMSG_MOT_GET_EEPROMPARAMS = 0x04BB;
        const int MGMSG_MOT_SET_SOL_STATE = 0x04CB;
        const int MGMSG_MOT_REQ_SOL_STATE = 0x04CC;
        const int MGMSG_MOT_GET_SOL_STATE = 0x04CD;


        const int MGMSG_MOT_SET_PMDSTAGEAXISPARAMS = 0x04F0;
        const int MGMSG_MOT_REQ_PMDSTAGEAXISPARAMS = 0x04F1;
        const int MGMSG_MOT_GET_PMDSTAGEAXISPARAMS = 0x04F2;
        const int MGMSG_LA_SET_MAGNIFICATION = 0x0840;
        const int MGMSG_LA_REQ_MAGNIFICATION = 0x0841;
        const int MGMSG_LA_GET_MAGNIFICATION = 0x0842;
        const int MGMSG_MOT_GET_JOGPARAMS = 0x0418;

        const int MGMSG_MOT_SET_LIMSWITCHPARAMS = 0x0423;
        const int MGMSG_MOT_REQ_LIMSWITCHPARAMS = 0x0424;
        const int MGMSG_MOT_GET_LIMSWITCHPARAMS = 0x0425;

        // new
        const int MGMSG_LATTICE_JED_UPDATE = 0x0855;

        const int MGMSG_SLOT_SET_TYPE = 0x0860;
        const int MGMSG_SLOT_REQ_TYPE = 0x0861;
        const int MGMSG_SLOT_GET_TYPE = 0x0862;

        // DBG 
        const int MGMSG_DBG_PRINT = 0x0856;


//parameter ends
      //byte MOTHER_ID = 0x11;

//int bytesToSend[6]= { 0x55, 0x08, 0x00, 0x00, 0x11, HOST_ID };

static const GUID GUID_DEVINTERFACE_LIST[] = 
{
	// GUID_DEVINTERFACE_COM_DEVICE
	{ 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73} ,
};

typedef void (*PortNotifyFunc)(LPTSTR OTMid, BOOL IsOTMArrival,
							   LPTSTR PCU2Aid, BOOL IsPCU2AArrival,
							   LPTSTR VCMCCUid, BOOL IsVCMCCUArrival);

typedef void (*PanelNotifyFunc)(void);

class CThorSerialCom
{
public:
	CThorSerialCom();
	~CThorSerialCom();

	DCB SerialPortConfig;
	COMMTIMEOUTS commTimeout;

	bool IsOpen;
	int WriteBufferSize;
	int ReadTimeout;
	int WriteTimeout;
	int _readFileSuccess;
	
	unsigned long BaudRate;
	unsigned long HandShake;
	unsigned char ByteSize;
	unsigned char Parity;
	unsigned char StopBits;

	TCHAR ValueStr[128];
	BOOL bPanelFuncSet;
	BOOL bLessThanPrompt;
	BOOL bAccessDenied;
	
	void Close();
	BOOL Open(LPWSTR PortName);
	void SetComTimeout();
	BOOL Write(LPTSTR TxData);
	int Read(LPTSTR RxData, int Timeout);
	void Flush();
	void GetNumPorts(int *NumPorts);
	void GetPortInfo(int PortIndex, LPTSTR PortName);
	LPTSTR parseSetting(LPTSTR commandToSend, LPTSTR settingToRead);

	PanelNotifyFunc PanelChangeFunc;
	BOOL WriteCommByteArray(const unsigned char * byteArray, int size );

private:
	HANDLE _hSerialPort;

	LARGE_INTEGER _litmp;
	LONGLONG _QPart1;
	LONGLONG _QPart2;
	double _dfMinus;
	double _dfFreq;
	double _dfTim;	
	void usbWrite(byte *bytesToSend, int length);

	//BOOL WriteCommByteArray(const unsigned char * byteArray, int size );

	
	OVERLAPPED m_OverlappedRead, m_OverlappedWrite;
	BOOL m_bOpened;

};

class CHidNotification
{
public:
	CHidNotification(HWND hWnd);
	~CHidNotification();

	void GetPortsStatus();	
	void CheckDeviceStatus();
	
	static LRESULT CALLBACK StaticWndProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	PortNotifyFunc PortChangeFunc;	

private:
	HDEVNOTIFY hDevNotify;
	HWND hDlg;
	TCHAR ID_OTM[DEVICE_ID_LENGTH];
	TCHAR ID_PCU2A[DEVICE_ID_LENGTH];
	TCHAR ID_VCMCCU[DEVICE_ID_LENGTH];
	TCHAR OTMPortName[DEVICE_ID_LENGTH];
	TCHAR PCU2APortName[DEVICE_ID_LENGTH];
	TCHAR VCMCCUPortName[DEVICE_ID_LENGTH];
	BOOL bOTMPreviousStatus;
	BOOL bOTMCurrentStatus;
	BOOL bPCU2APreviousStatus;
	BOOL bPCU2ACurrentStatus;
	BOOL bVCMCCUPreviousStatus;
	BOOL bVCMCCUCurrentStatus;
	int OTMStatus;
	int PCU2AStatus;
	int VCMCCUStatus;
	BOOL bDeviceChanged;	

	

};