#include "stdafx.h"
#include "ThorSerialCom.h"
#include "Strsafe.h"

#ifndef GUID_DEVINTERFACE_COMPORT 
DEFINE_GUID(GUID_DEVINTERFACE_COMPORT , 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, \
			0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73);
#endif

wchar_t message[MSG_SIZE];

CThorSerialCom::CThorSerialCom()
{
	hSerialPort = NULL;

	BaudRate = CBR_115200;
	HandShake = 0;
	ByteSize = 8;
	Parity = NOPARITY;
	StopBits = ONESTOPBIT;

	IsOpen = FALSE;
	WriteBufferSize = 4096;
	ReadTimeout = 1000;
	WriteTimeout = 1000;

	bPanelFuncSet = FALSE;

	QueryPerformanceFrequency(&litmp);
	dfFreq = (double)litmp.QuadPart;
}

CThorSerialCom::~CThorSerialCom()
{
}

BOOL CThorSerialCom::Open(LPWSTR PortName)
{
	int i, j, StrLength;
	DWORD ErrorCode;

	TCHAR SerialPort[32];

#ifdef _UNICODE
	if (wcsncmp(PortName, _T("\\\\.\\"), 4) == 0)
	{
		j = 0;
	}
	else
	{
		wcscpy_s(SerialPort, _T("\\\\.\\"));
		j = 4;
	}
	StrLength = static_cast<int>(wcslen(PortName));
#else
	if (strncmp(PortName, "\\\\.\\", 4) == 0)
	{
		j = 0;
	}
	else
	{
		strcpy_s(SerialPort, "\\\\.\\");
		j = 4;
	}
	StrLength = strlen(PortName);
#endif

	for (i = 0; i < StrLength; i++)
		SerialPort[i+j] = PortName[i];

	SerialPort[i+j] = NULL;

	hSerialPort = CreateFile(SerialPort,						// Specify port device
							 GENERIC_READ | GENERIC_WRITE,	// Specify mode that open device
							 0,								// must be opened with exclusive-access
							 NULL,							// no security attributes
							 OPEN_EXISTING,					// must use OPEN_EXISTING
							 0,								// not overlapped I/O
							 NULL);

	if (hSerialPort == INVALID_HANDLE_VALUE)
	{
		IsOpen = FALSE;
		ErrorCode = GetLastError();
		if (ErrorCode == 5)
			bAccessDenied = TRUE;
	}
	else
	{
		// Fill in DCB: 57,600 bps, 8 data bits, no parity, 1 stop bit, no flow control.
		SerialPortConfig.DCBlength = 28;
		SerialPortConfig.BaudRate = BaudRate;
		SerialPortConfig.fBinary = 1;
		SerialPortConfig.fParity = 0;
		SerialPortConfig.fOutxCtsFlow = 0;
		SerialPortConfig.fOutxDsrFlow = 0;
		SerialPortConfig.fDtrControl = 1;
		SerialPortConfig.fDsrSensitivity = 0;
		SerialPortConfig.fTXContinueOnXoff = 0;
		SerialPortConfig.fOutX = 0;
		SerialPortConfig.fInX = 0;
		SerialPortConfig.fErrorChar = 0;
		SerialPortConfig.fNull = 0;
		SerialPortConfig.fRtsControl = HandShake;
		SerialPortConfig.fAbortOnError = 0;
		SerialPortConfig.fDummy2 = 0;
		SerialPortConfig.XonLim = 2048;
		SerialPortConfig.XoffLim = 512;
		SerialPortConfig.ByteSize = ByteSize;
		SerialPortConfig.Parity = Parity;
		SerialPortConfig.StopBits = StopBits;
		SerialPortConfig.XonChar = 17;
		SerialPortConfig.XoffChar = 19;
		SerialPortConfig.ErrorChar = 0;
		SerialPortConfig.EofChar = 0;
		SerialPortConfig.EvtChar = 0;
		SerialPortConfig.wReserved1 = 0;

		if (SetCommState(hSerialPort, &SerialPortConfig) != NULL)
		{
			commTimeout.ReadIntervalTimeout = ReadTimeout;
			commTimeout.ReadTotalTimeoutConstant = ReadTimeout;
			commTimeout.ReadTotalTimeoutMultiplier = 0;
			commTimeout.WriteTotalTimeoutConstant = WriteTimeout;
			commTimeout.WriteTotalTimeoutMultiplier = 0;

			GetCommState(hSerialPort, &SerialPortConfig);

			if (SetCommTimeouts(hSerialPort, &commTimeout))
			{
				IsOpen = TRUE;
			}
			else
				IsOpen = FALSE;
		}
		else
			IsOpen = FALSE;
	}
	return IsOpen;
}

void CThorSerialCom::Close()
{
	if (hSerialPort)
	{
		if(CloseHandle(hSerialPort) == NULL)
		{
			IsOpen = TRUE;
		}
		else
		{
			IsOpen = FALSE;
			hSerialPort = NULL;
		}
	}
}

void CThorSerialCom::SetComTimeout()
{
	if (hSerialPort)
	{
		if (ReadTimeout > 0)
		{
			commTimeout.ReadIntervalTimeout = ReadTimeout;
			commTimeout.ReadTotalTimeoutConstant = ReadTimeout;
		}

		if (WriteTimeout > 0)
			commTimeout.WriteTotalTimeoutConstant = WriteTimeout;

		commTimeout.ReadTotalTimeoutMultiplier = 0;
		commTimeout.WriteTotalTimeoutMultiplier = 0;

		SetCommTimeouts(hSerialPort, &commTimeout);
	}
}

void CThorSerialCom::GetNumPorts(int *NumPorts)
{
	// Create a device information set that will be the container for 
	// the device interfaces.
	TCHAR strErr[256];
	GUID *guidDev = (GUID*) &GUID_DEVINTERFACE_COMPORT; //GUID_CLASS_COMPORT;

	HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
	SP_DEVICE_INTERFACE_DETAIL_DATA *pDetData = NULL;

	*NumPorts = 0;

	try 
	{
		hDevInfo = SetupDiGetClassDevs(guidDev,
									   NULL,
									   NULL,
									   DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

		if(hDevInfo == INVALID_HANDLE_VALUE) 
		{
#ifdef _UNICODE
			wcscpy_s(strErr, _T("SetupDiGetClassDevs failed."));
#else
			strcpy_s(strErr, "SetupDiGetClassDevs failed.");
#endif
			throw strErr;
		}
		
		// Enumerate the serial ports
		BOOL bOk = TRUE;

		SP_DEVICE_INTERFACE_DATA ifcData;

		ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		
		for (int i = 0; bOk; i++)
		{
			bOk = SetupDiEnumDeviceInterfaces(hDevInfo,
											  NULL,
											  guidDev,
											  i,
											  &ifcData);
			if (bOk)
			{
				*NumPorts = *NumPorts + 1;
			}
			else
			{
				DWORD err = GetLastError();
				if (err != ERROR_NO_MORE_ITEMS)
				{
#ifdef _UNICODE
					wcscpy_s(strErr, _T("SetupDiEnumDeviceInterfaces failed."));
#else
					strcpy_s(strErr, "SetupDiEnumDeviceInterfaces failed.");
#endif
					throw strErr;
				}
			}
		}
	}
	catch (LPTSTR strCatchErr)
	{
		StringCbPrintfW(message,MSG_SIZE,L"%S",strCatchErr);
	}

	if (hDevInfo != INVALID_HANDLE_VALUE)
		SetupDiDestroyDeviceInfoList(hDevInfo);
}

void CThorSerialCom::GetPortInfo(int PortIndex, LPTSTR PortName)
{
	TCHAR strErr[256];
	// Create a device information set that will be the container for 
	// the device interfaces.
	GUID *guidDev = (GUID*) &GUID_DEVINTERFACE_COMPORT; //GUID_CLASS_COMPORT;

	HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
	SP_DEVICE_INTERFACE_DETAIL_DATA *pDetData = NULL;

	try 
	{
		hDevInfo = SetupDiGetClassDevs(guidDev,
									   NULL,
									   NULL,
									   DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

		if(hDevInfo == INVALID_HANDLE_VALUE) 
		{
#ifdef _UNICODE
			wcscpy_s(strErr, _T("SetupDiGetClassDevs failed."));
#else
			strcpy_s(strErr, "SetupDiGetClassDevs failed.");
#endif
			throw strErr;
		}

		// Enumerate the serial ports
		BOOL bOk = TRUE;
		SP_DEVICE_INTERFACE_DATA ifcData;
		DWORD dwDetDataSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + 256;
		pDetData = (SP_DEVICE_INTERFACE_DETAIL_DATA*) new char[dwDetDataSize];
		ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		pDetData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		bOk = SetupDiEnumDeviceInterfaces(hDevInfo,
										  NULL,
										  guidDev,
										  PortIndex,
										  &ifcData);
		if (bOk)
		{
			// Got a device. Get the details.
			SP_DEVINFO_DATA devdata = {sizeof(SP_DEVINFO_DATA)};
			bOk = SetupDiGetDeviceInterfaceDetail(hDevInfo,
												  &ifcData,
												  pDetData,
												  dwDetDataSize,
												  NULL,
												  &devdata);
			if (bOk)
			{
				TCHAR fname[256];
				BOOL bGetName = FALSE;
				int i, j;
				BOOL bSuccess = SetupDiGetDeviceRegistryProperty(hDevInfo,
																 &devdata,
																 SPDRP_FRIENDLYNAME,
																 NULL,
																 (PBYTE)fname,
																 sizeof(fname),
																 NULL);

				if (bSuccess)
				{
					j = 0;
					for (i = 0; i < sizeof(fname); i++)
					{
						if (fname[i] == ' ' && fname[i+1] == '(')
							bGetName = TRUE;

						if (fname[i+2] == ')' && bGetName)
						{
							bGetName = FALSE;
							i = sizeof(fname);
							PortName[j] = NULL;
						}

						if (bGetName)
						{
							PortName[j] = fname[i+2];
							j++;
						}
					}
				}
				else
				{
#ifdef _UNICODE
					wcscpy_s(strErr, _T("SetupDiGetDeviceInterfaceDetail failed."));
#else
					strcpy_s(strErr, "SetupDiGetDeviceInterfaceDetail failed.");
#endif
					PortName = NULL;
					throw strErr;
				}
			}
		}
	}
	catch (LPTSTR strCatchErr)
	{
		StringCbPrintfW(message,MSG_SIZE,L"%S",strCatchErr);
	}

	if (pDetData != NULL)
		delete [] (char*)pDetData;

	if (hDevInfo != INVALID_HANDLE_VALUE)
		SetupDiDestroyDeviceInfoList(hDevInfo);
}

BOOL CThorSerialCom::Write(LPTSTR TxData)
{
	DWORD BytesWritten;
	char TxBuffer[2048];
	int i;
	int TxDataLength;

	if (hSerialPort)
	{
		for (i = 0; i < 2048; i++)
		{
			if (TxData[i] != NULL)
				TxBuffer[i] = static_cast<char>(TxData[i]);
			else
			{
				TxDataLength = i;
				i = 2048;
			}
		}
		
		TxBuffer[TxDataLength] = '\r';
		TxBuffer[TxDataLength+1] = 0;

		WriteFile(hSerialPort,				// handle to file to write to
				  TxBuffer,					// pointer to data to write to file
				  TxDataLength+1,			// number of bytes to write
				  &BytesWritten,			// pointer to number of bytes written
				  NULL);
	
		//char* pCurrent = TxBuffer;
		//int counter = 0;
		//while(*pCurrent)
		//{
		//	WriteFile(hSerialPort,				// handle to file to write to
		//		  pCurrent++,					// pointer to data to write to file
		//		  1,			// number of bytes to write
		//		  &BytesWritten,			// pointer to number of bytes written
		//		  NULL);
		//	counter++;
		//	Sleep(1);
		//}


		if (BytesWritten == TxDataLength+1)
			return TRUE;
	}

	return FALSE;
}

int CThorSerialCom::Read(LPTSTR RxData, int Timeout)
{
	DWORD BytesRead;
	int i;
	BOOL bGetPromptChar = FALSE;
	BOOL bTimeout = FALSE;
	char RxBuffer[2];

	bLessThanPrompt = FALSE;
	
	if (hSerialPort)
	{
		i = 0;

		QueryPerformanceCounter(&litmp);
		QPart1 = litmp.QuadPart;

		while (!bGetPromptChar && !bTimeout)
		{
			ReadFile(hSerialPort,
					 RxBuffer,
					 1,
					 &BytesRead,
					 NULL);
			
			if (BytesRead > 0)
			{
				if (RxBuffer[0] == '\r')
				{
					RxData[i] = '\r';
					i++;
					//RxData[i] = '\n';
					//i++;

					bGetPromptChar = FALSE;
					bLessThanPrompt = FALSE;
				}
				else
				{
					RxData[i] = RxBuffer[0];
					i++;

					if (RxBuffer[0] == '>' || RxBuffer[0] == '<')
					{
						if (RxBuffer[0] == '<')
							bLessThanPrompt = TRUE;

						ReadFile(hSerialPort,
								 RxBuffer,
								 1,
								 &BytesRead,
								 NULL);

						if (BytesRead == 0)
						{
							bGetPromptChar = TRUE;
						}
						else
						{
							RxData[i] = RxBuffer[0];
							i++;

							if (RxBuffer[0] == ' ')
							{
								RxData[i] = NULL;
								i++;
								bGetPromptChar = TRUE;
							}
						}
					}
					else
					{
						bGetPromptChar = FALSE;
					}
				}
			}

			QueryPerformanceCounter(&litmp);
			QPart2 = litmp.QuadPart;
			dfMinus = (double)(QPart2 - QPart1);
			dfTim = dfMinus/dfFreq*1000;

			if (dfTim > Timeout)
			{
				bTimeout = TRUE;
				RxData[0] = NULL;
				i = -1;
			}
		}

		if (bLessThanPrompt && bPanelFuncSet)
			PanelChangeFunc();
	}

	return i;
}

int CThorSerialCom::ReadQuery(LPTSTR RxData, int Timeout)
{
	DWORD BytesRead;
	int i;
	BOOL bGetPromptChar = FALSE;
	BOOL bTimeout = FALSE;
	char RxBuffer[2];

	bLessThanPrompt = FALSE;
	
	if (hSerialPort)
	{
		i = 0;

		QueryPerformanceCounter(&litmp);
		QPart1 = litmp.QuadPart;

		while (!bGetPromptChar && !bTimeout)
		{
			ReadFile(hSerialPort,
					 RxBuffer,
					 1,
					 &BytesRead,
					 NULL);
			
			if (BytesRead > 0)
			{
				if (RxBuffer[0] == '\r')
				{
					RxData[i] = '\r';
					i++;
					RxData[i] = '\n';
					i++;

					bGetPromptChar = FALSE;
					bLessThanPrompt = FALSE;
				}
				else
				{
					RxData[i] = RxBuffer[0];
					i++;

					if (RxBuffer[0] == '>' || RxBuffer[0] == '<')
					{
						if (RxBuffer[0] == '<')
							bLessThanPrompt = TRUE;

						ReadFile(hSerialPort,
								 RxBuffer,
								 1,
								 &BytesRead,
								 NULL);

						if (BytesRead == 0)
						{
							bGetPromptChar = TRUE;
						}
						else
						{
							RxData[i] = RxBuffer[0];
							i++;

							if (RxBuffer[0] == ' ')
							{
								RxData[i] = NULL;
								i++;
								bGetPromptChar = TRUE;
							}
						}
					}
					else
					{
						bGetPromptChar = FALSE;
					}
				}
			}

			QueryPerformanceCounter(&litmp);
			QPart2 = litmp.QuadPart;
			dfMinus = (double)(QPart2 - QPart1);
			dfTim = dfMinus/dfFreq*1000;

			if (dfTim > Timeout)
			{
				bTimeout = TRUE;
				RxData[0] = NULL;
				i = -1;
			}
		}

		if (bLessThanPrompt && bPanelFuncSet)
			PanelChangeFunc();
	}

	return i;
}


void CThorSerialCom::Flush()
{
	if (hSerialPort)
		PurgeComm(hSerialPort, PURGE_TXCLEAR |PURGE_RXCLEAR);
}

LPTSTR CThorSerialCom::parseSetting(LPTSTR commandToSend, LPTSTR settingToRead)
{
	int i;
	int BytesRead;
	int Offset;
	int StrLength;
	TCHAR RxData[2048];

    try
    {
		if (!Write(commandToSend))
			return NULL;

        BytesRead = Read(RxData, 2000);
		if (BytesRead > 0)
		{
			for (i = 0; i < 2048; i++)
			{
				if (settingToRead[i] == NULL)
				{
					StrLength = i;
					i = 2048;
				}
			}

			Offset = 0;

			for (i = 0; i < 2048 - StrLength; i++)
			{
#ifdef _UNICODE
				Offset = wcsncmp(RxData+i, settingToRead, StrLength);
#else
				Offset = strncmp(RxData+i, settingToRead, StrLength);
#endif
				if (Offset == 0)
				{
					Offset = i;
					i = 2048;
				}
			}

			if (Offset == 0)
				return NULL;
			else
			{
				Offset = Offset + StrLength + 3;
				for (i = Offset; i < 2048; i++)
				{
					if (RxData[i] != ' ' &&
						RxData[i] != NULL &&
						RxData[i] != '\r')
						ValueStr[i-Offset] = RxData[i];
					else
					{
						ValueStr[i-Offset] = NULL;
						i = 2048;
					}
				}
			}
		}
		else
			return NULL;
	}
	catch (char *Str)
	{
		StringCbPrintfW(message,MSG_SIZE,L"%S",Str);
		return NULL;
	}

	return ValueStr;
}
/*
CHidNotification::CHidNotification(HWND hWnd)
{
	int i;
	HINSTANCE hInst;
	WNDCLASSEX WndClsEx;

	hInst = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);

	TCHAR ClsName[32];
	TCHAR WndName[32];

#ifdef _UNICODE
	wcscpy_s(ClsName, _T("ThorUSBDevice"));
	wcscpy_s(WndName, _T("ThorUSBDeviceNotify"));
#else
	strcpy_s(ClsName, "ThorUSBDevice");
	strcpy_s(WndName, "ThorUSBDeviceNotify");
#endif

	WndClsEx.cbSize        = sizeof(WNDCLASSEX);
	WndClsEx.style         = CS_HREDRAW | CS_VREDRAW;
	WndClsEx.lpfnWndProc   = (WNDPROC)StaticWndProcedure;
	WndClsEx.cbClsExtra    = 0;
	WndClsEx.cbWndExtra    = 0;
	WndClsEx.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	WndClsEx.hCursor       = LoadCursor(NULL, IDC_ARROW);
	WndClsEx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClsEx.lpszMenuName  = NULL;
	WndClsEx.lpszClassName = ClsName;
	WndClsEx.hInstance     = hInst;
	WndClsEx.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&WndClsEx);

    hDlg = CreateWindow(ClsName,
						WndName,
						WS_OVERLAPPEDWINDOW,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						NULL,
						NULL,
						hInst,
						this);

	::ShowWindow(hDlg, SW_HIDE);

	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
	ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
	NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	
	for(i = 0; i < sizeof(GUID_DEVINTERFACE_LIST)/sizeof(GUID); i++)
	{
		NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_LIST[i];
		hDevNotify = RegisterDeviceNotification(hDlg, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
	}

	bMCLSPreviousStatus = FALSE;
	bMCLSCurrentStatus = FALSE;
	bPCU2APreviousStatus = FALSE;
	bPCU2ACurrentStatus = FALSE;
	bVCMCCUPreviousStatus = FALSE;
	bVCMCCUCurrentStatus = FALSE;
	MCLSStatus = 0;
	PCU2AStatus = 0;
	VCMCCUStatus = 0;
	MCLSPortName[0] = NULL;
	PCU2APortName[0] = NULL;
	VCMCCUPortName[0] = NULL;

	GetPortsStatus();
}

CHidNotification::~CHidNotification()
{
	UnregisterDeviceNotification(hDevNotify);
	DestroyWindow(hDlg);
}

void CHidNotification::GetPortsStatus()
{
	int i;
	int Bytes;
	int CurrentDeviceNum;
	TCHAR PortsName[128];
	TCHAR TxCommand[2048];
	TCHAR RxData[2048];

	CThorSerialCom *SerialCom;

	SerialCom = new CThorSerialCom;

	bMCLSPreviousStatus = bMCLSCurrentStatus;
	bPCU2APreviousStatus = bPCU2ACurrentStatus;
	bVCMCCUPreviousStatus = bVCMCCUCurrentStatus;

	bMCLSCurrentStatus = FALSE;
	bPCU2ACurrentStatus = FALSE;
	bVCMCCUCurrentStatus = FALSE;

	SerialCom->GetNumPorts(&CurrentDeviceNum);

#ifdef _UNICODE
	wcscpy_s(TxCommand, _T("id?"));
#else
	strcpy_s(TxCommand, "id?");
#endif

	for (i = 0; i < CurrentDeviceNum; i++)
	{
		SerialCom->bAccessDenied = FALSE;
		SerialCom->GetPortInfo(i, PortsName);
		SerialCom->Open(PortsName);

		if (SerialCom->IsOpen)
		{
			if (SerialCom->Write(TxCommand))
			{
				Bytes = SerialCom->Read(RxData, 100);
				if (Bytes > 0)
				{
#ifdef _UNICODE
					if (wcsncmp(RxData, _T("Command error"), 13) == 0)
					{
						if (SerialCom->Write(TxCommand))
							Bytes = SerialCom->Read(RxData, 100);
					}

					if (wcsncmp(RxData + 14, _T("MCLS"), 4) == 0)
					{
						bMCLSCurrentStatus = TRUE;
						wcscpy_s(MCLSPortName, PortsName);
					}
					else if (wcsncmp(RxData + 14, _T("PCU2A"), 5) == 0)
					{
						bPCU2ACurrentStatus = TRUE;
						wcscpy_s(PCU2APortName, PortsName);
					}
					else if (wcsncmp(RxData + 14, _T("VCMCCU"), 6) == 0)
					{
						bVCMCCUCurrentStatus = TRUE;
						wcscpy_s(VCMCCUPortName, PortsName);
					}
#else
					if (strncmp(RxData, "Command error", 13) == 0)
					{
						if (SerialCom->Write(TxCommand))
							Bytes = SerialCom->Read(RxData, 100);
					}

					if (strncmp(RxData + 14, "MCLS", 4) == 0)
					{
						bMCLSCurrentStatus = TRUE;
						strcpy_s(MCLSPortName, PortsName);
					}
					else if (strncmp(RxData + 14, "PCU2A", 5) == 0)
					{
						bPCU2ACurrentStatus = TRUE;
						strcpy_s(PCU2APortName, PortsName);
					}
					else if (strncmp(RxData + 14, "VCMCCU", 6) == 0)
					{
						bVCMCCUCurrentStatus = TRUE;
						strcpy_s(VCMCCUPortName, PortsName);
					}
#endif
				}
			}
		}
		else
		{
			if (SerialCom->bAccessDenied)
			{
#ifdef _UNICODE
				if (wcscmp(PortsName, MCLSPortName) == 0)
				{
					bMCLSCurrentStatus = bMCLSPreviousStatus;
				}
				else if (wcscmp(PortsName, PCU2APortName) == 0)
				{
					bPCU2ACurrentStatus = bPCU2APreviousStatus;
				}
				else if (wcscmp(PortsName, VCMCCUPortName) == 0)
				{
					bVCMCCUCurrentStatus = bVCMCCUPreviousStatus;
				}
#else
				if (strcmp(PortsName, MCLSPortName) == 0)
				{
					bMCLSCurrentStatus = bMCLSPortName;
				}
				else if (strcmp(PortsName, PCU2APortName) == 0)
				{
					bPCU2ACurrentStatus = bPCU2APreviousStatus;
				}
				else if (strcmp(PortsName, VCMCCUPortName) == 0)
				{
					bVCMCCUCurrentStatus = bVCMCCUPreviousStatus;
				}
#endif
			}
		}
		SerialCom->Close();
	}
	delete SerialCom;
}

void CHidNotification::CheckDeviceStatus()
{
	MCLSStatus = bMCLSCurrentStatus - bMCLSPreviousStatus;
	PCU2AStatus = bPCU2ACurrentStatus - bPCU2APreviousStatus;
	VCMCCUStatus = bVCMCCUCurrentStatus - bVCMCCUPreviousStatus;

	if (MCLSStatus != 0 ||
		PCU2AStatus != 0 ||
		VCMCCUStatus != 0)
		bDeviceChanged = TRUE;
	else
		bDeviceChanged = FALSE;

#ifdef _UNICODE
		if (MCLSStatus == 0)
			ID_MCLS[0] = NULL;
		else
			wcscpy_s(ID_MCLS, _T("MCLS"));

		if (PCU2AStatus == 0)
			ID_PCU2A[0] = NULL;
		else
			wcscpy_s(ID_PCU2A, _T("PCU2A"));

		if (VCMCCUStatus == 0)
			ID_VCMCCU[0] = NULL;
		else
			wcscpy_s(ID_VCMCCU, _T("VCMCCU"));
#else
		if (MCLSStatus == 0)
			ID_MCLS[0] = NULL;
		else
			strcpy_s(ID_MCLS, "MCLS");

		if (PCU2AStatus == 0)
			ID_PCU2A[0] = NULL;
		else
			strcpy_s(ID_PCU2A, "PCU2A");

		if (VCMCCUStatus == 0)
			ID_VCMCCU[0] = NULL;
		else
			strcpy_s(ID_VCMCCU, "VCMCCU");
#endif
}

LRESULT CALLBACK CHidNotification::StaticWndProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CHidNotification* pParent;

	// Get pointer to window
	if(uMsg == WM_CREATE)
	{
		pParent = (CHidNotification *)((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)pParent);
	}
	else
	{
		pParent = (CHidNotification *)GetWindowLongPtr(hWnd, GWL_USERDATA);
		if(!pParent)
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
   }

   return pParent->WndProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CHidNotification::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    // If the user wants to close the application
    case WM_DESTROY:
        // then close it
        PostQuitMessage(WM_QUIT);
        break;

	case WM_DEVICECHANGE: 
		{ 
			if (wParam == DBT_DEVICEARRIVAL)
			{
				PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
				PDEV_BROADCAST_DEVICEINTERFACE pDevInf;
				switch( pHdr->dbch_devicetype )
				{
				case DBT_DEVTYP_DEVICEINTERFACE:
					pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)pHdr;
					GetPortsStatus();
					CheckDeviceStatus();
					if (bDeviceChanged)
						PortChangeFunc(ID_MCLS, MCLSStatus,
									   ID_PCU2A, PCU2AStatus,
									   ID_VCMCCU, VCMCCUStatus);
					break;
				}
			}
			else if (wParam == DBT_DEVICEREMOVECOMPLETE)
			{
				PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
				PDEV_BROADCAST_DEVICEINTERFACE pDevInf;
				switch( pHdr->dbch_devicetype )
				{
				case DBT_DEVTYP_DEVICEINTERFACE:
					pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)pHdr;
					GetPortsStatus();
					CheckDeviceStatus();
					if (bDeviceChanged)
						PortChangeFunc(ID_MCLS, MCLSStatus,
									   ID_PCU2A, PCU2AStatus,
									   ID_VCMCCU, VCMCCUStatus);
					break;
				}
			}
		} 

    default:
        // Process the left-over messages
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    // If something was not done, let it go
    return 0;
}
*/