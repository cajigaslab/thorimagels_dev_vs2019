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

#define MCLS_ID 1
#define PCU2A_ID 2
#define VCMCCU_ID 4
#define DEVICE_NOCHANGE 0
#define DEVICE_REMOVAL 1
#define DEVICE_ARRIVAL 2
#define MAX_DEVICE_NUM 10
#define DEVICE_ID_LENGTH 16

static const GUID GUID_DEVINTERFACE_LIST[] = 
{
	// GUID_DEVINTERFACE_COM_DEVICE
	{ 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73} ,
};

typedef void (*PortNotifyFunc)(LPTSTR MCLSid, BOOL IsMCLSArrival,
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

private:
	HANDLE hSerialPort;

	LARGE_INTEGER litmp;
	LONGLONG QPart1;
	LONGLONG QPart2;
	double dfMinus;
	double dfFreq;
	double dfTim;	
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
	TCHAR ID_MCLS[DEVICE_ID_LENGTH];
	TCHAR ID_PCU2A[DEVICE_ID_LENGTH];
	TCHAR ID_VCMCCU[DEVICE_ID_LENGTH];
	TCHAR MCLSPortName[DEVICE_ID_LENGTH];
	TCHAR PCU2APortName[DEVICE_ID_LENGTH];
	TCHAR VCMCCUPortName[DEVICE_ID_LENGTH];
	BOOL bMCLSPreviousStatus;
	BOOL bMCLSCurrentStatus;
	BOOL bPCU2APreviousStatus;
	BOOL bPCU2ACurrentStatus;
	BOOL bVCMCCUPreviousStatus;
	BOOL bVCMCCUCurrentStatus;
	int MCLSStatus;
	int PCU2AStatus;
	int VCMCCUStatus;
	BOOL bDeviceChanged;	
};