// WinDVI.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "WinDVI.h"
#include "Strsafe.h"
#include "..\..\Device.h"		//for status enum

#ifdef LOGGING_ENABLED
std::auto_ptr<LogDll> logDll(new LogDll(L".\\ThorLoggingUnmanaged.dll"));
#endif

#define THREAD_CHECKTIME	500		//[ms]
#define DEFAULT_SIZE		512

// Static members:
bool CWinDVI::_instanceFlag = false;
std::auto_ptr<CWinDVI> CWinDVI::_single(new CWinDVI());
wchar_t CWinDVI::_errMsg[MSG_SIZE];
BYTE* CWinDVI::_bmpBuffers[MAX_BUFFER_CNT] = {NULL};
long CWinDVI::_bmpBufID = -1;
int CWinDVI::winWidth = DEFAULT_SIZE;
int CWinDVI::winHeight = DEFAULT_SIZE;
CMonitorInfoEx CWinDVI::cMInfo;

// Global Variables:
TCHAR				szWindowClass[MAX_PATH] = TEXT("WinDVI");
HINSTANCE			hInst = NULL;
HWND				hwnd = NULL;
HBITMAP				hBitmap = NULL;
BITMAPINFO			bmi;
HANDLE				hStatusWinDVI = CreateEvent(0, TRUE, FALSE, 0);		//manual reset
CRITICAL_SECTION	mCriticalSection;									//for _bmpBuffers
long				winDVIStatus = IDevice::STATUS_BUSY;

// Thread to create a window
UINT WindowCreateProc( LPVOID pParam )
{
	long ret = TRUE;
	MSG	messages;

	hwnd = CreateWindow(szWindowClass, 0, WS_POPUP,	CWinDVI::cMInfo.rcMonitor.left, CWinDVI::cMInfo.rcMonitor.top, CWinDVI::winWidth, CWinDVI::winHeight, NULL, NULL, hInst, NULL);
	SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
    SetMenu(hwnd, NULL);
	::SetWindowPos(hwnd,		// handle to window
                HWND_TOPMOST,	// placement-order handle
                0,				// horizontal position
                0,				// vertical position
                0,				// width
                0,				// height
                SWP_NOMOVE|SWP_NOSIZE); // window-positioning options

	if (!hwnd)
    {
		CWinDVI::winWidth = CWinDVI::winHeight = DEFAULT_SIZE;
		return FALSE;
    }

	ret = ShowWindow(hwnd, SW_SHOWDEFAULT);
	ret = EnableWindow(hwnd, FALSE);

	/* Run the message loop in the same thread of window. It will run until GetMessage() returns 0 */
	while (0 < GetMessage (&messages, hwnd, 0, WM_USER))
	{
		/* Translate virtual-key messages into character messages */
		TranslateMessage(&messages);
		/* Send message to WindowProcedure */
		DispatchMessage(&messages);
	}
	return ret;
}

CWinDVI::CWinDVI()
{
	_cDisplay.Update();
	int count = _cDisplay.Count();	
	if(count <= 1)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"CWinDVI: No second monitor is available");
		LogMessage(_errMsg,VERBOSE_EVENT);
	}
	// assume using last monitor:
	cMInfo = (0 < count) ? _cDisplay.Get(count-1) : _cDisplay.Get(0); 

	hWinThread = NULL;

	InitializeCriticalSectionAndSpinCount(&mCriticalSection, 0x00000400);
}

CWinDVI::~CWinDVI()
{
	_instanceFlag = false;
	ClearBMPs();
	DestroyDVIWindow();
	
	if(hWinThread)
	{
		CloseHandle(hWinThread);
		hWinThread = NULL;
	}
}

CWinDVI* CWinDVI::getInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new CWinDVI());
		_instanceFlag = true;
		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

// Processes messages for the window.
// WM_COMMAND- process the application menu
// WM_PAINT	- Paint the main window
// WM_DESTROY- post a quit message and return
LRESULT CALLBACK CWinDVI::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT		ps;
	HDC				hdc;
	char*			ppvBits;
    BITMAP          bitmap;
    HDC             hdcMem;
    HGDIOBJ         oldBitmap;

	switch (message)
	{
	case WM_PAINT:
		//paint bmp based on ID:
		EnterCriticalSection(&mCriticalSection);

		if(NULL !=  _bmpBuffers[_bmpBufID])
		{
			hdc = BeginPaint(hWnd, &ps);

			hBitmap = CreateDIBSection(hdc,&bmi, DIB_RGB_COLORS, (void**)&ppvBits, NULL, 0);
			SetDIBits(NULL, hBitmap, 0, bmi.bmiHeader.biHeight, _bmpBuffers[_bmpBufID], &bmi, DIB_RGB_COLORS);
		
			hdcMem = CreateCompatibleDC(hdc);
			oldBitmap = SelectObject(hdcMem, hBitmap);

			GetObject(hBitmap, sizeof(bitmap), &bitmap);
			BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

			SelectObject(hdcMem, oldBitmap);
			DeleteDC(hdcMem);

			EndPaint(hWnd, &ps);
		}

		LeaveCriticalSection(&mCriticalSection);
		SetEvent(hStatusWinDVI);
		break;
	case WM_DESTROY:
		DeleteObject(hBitmap);
		PostQuitMessage(0);
		SetEvent(hStatusWinDVI);
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Registers the window class
ATOM CWinDVI::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = {0}; // make sure all the members are zero-ed out to start

	wcex.cbSize = sizeof(wcex);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= CWinDVI::WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(hInstance, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

// Method to create thread for a new window if not exist
long CWinDVI::CreateDVIWindow(int w, int h)
{
	long ret = TRUE;
	if ((0 == w) || (0 == h))
		return FALSE;
	if ((hwnd) && (winWidth == w) && (winHeight == h))
		return TRUE;

	//re-create when different size:
	if((hwnd) && ((winWidth != w) || (winHeight != h)))
	{
		DestroyDVIWindow();
	}

	hInst = GetModuleHandle(0);
	MyRegisterClass(hInst);		//FALSE when re-register

	winWidth = (cMInfo.Width() > w) ? w : cMInfo.Width();
	winHeight = (cMInfo.Height() > h) ? h : cMInfo.Height();
		
	if(hWinThread)
	{
		CloseHandle(hWinThread);
		hWinThread = NULL;
	}
	DWORD dwThread;
	hWinThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) WindowCreateProc, NULL, 0, &dwThread );
    return ret;
}

void CWinDVI::DestroyDVIWindow()
{
	if(hwnd)
	{
		//set flag and event before callback:
		winDVIStatus = IDevice::STATUS_BUSY;
		ResetEvent(hStatusWinDVI);
		
		//send request to close window:
		SendMessage(hwnd, WM_CLOSE, 0, 0);

		if(WAIT_OBJECT_0 != WaitForSingleObject(hStatusWinDVI, THREAD_CHECKTIME))
		{
			logDll->TLTraceEvent(ERROR_EVENT,1,L"WinDVI DestroyDVIWindow WaitForSingleObject hStatusWinDVI failed.\n");
			LogMessage(_errMsg, ERROR_EVENT);
		}

		//done:
		winDVIStatus = IDevice::STATUS_READY;
		hwnd = NULL;		
		hInst = NULL;
	}
}

// populate bitmap buffers up to MAX_BUFFER_CNT count
long CWinDVI::EditBMP(int id, unsigned char* bmpBuf, BITMAPINFO bmpInfo)
{
	long ret = TRUE;
	if((NULL == bmpBuf) || (0 > id) || (MAX_BUFFER_CNT <= id))
		return FALSE;

	EnterCriticalSection(&mCriticalSection);

	_bmpBuffers[id] = (BYTE*) realloc((void*)_bmpBuffers[id], bmpInfo.bmiHeader.biSizeImage);
	if(NULL == _bmpBuffers[id])
		return FALSE;
	if(0 == memcpy_s(_bmpBuffers[id],bmpInfo.bmiHeader.biSizeImage,bmpBuf,bmpInfo.bmiHeader.biSizeImage))
	{
		_bmpBufID = id;
		bmi = bmpInfo;
	}

	LeaveCriticalSection(&mCriticalSection);
	return ret;
}

void CWinDVI::ClearBMPs()
{
	for (int i = 0; i < MAX_BUFFER_CNT; i++)
	{
		if(NULL != _bmpBuffers[i])
		{
			free(_bmpBuffers[i]);
			_bmpBuffers[i] = NULL;
		}
	}
	_bmpBufID = -1;
}

//Display bitmap with defined index on the window
long CWinDVI::DisplayBMP(int id)
{
	if((hwnd) && (NULL != _bmpBuffers[id]))
	{		
		_bmpBufID = id;

		::SetActiveWindow(hwnd);
		RECT rect;
		GetClientRect(hwnd, &rect);
		InvalidateRect(hwnd, &rect, FALSE);

		//set flag and event before callback:
		winDVIStatus = IDevice::STATUS_BUSY;
		ResetEvent(hStatusWinDVI);

		if(0 != UpdateWindow(hwnd))		//callback for WM_PAINT
		{
			if(WAIT_OBJECT_0 != WaitForSingleObject(hStatusWinDVI, THREAD_CHECKTIME))
			{
				logDll->TLTraceEvent(ERROR_EVENT,1,L"WinDVI DisplayBMP WaitForSingleObject hStatusWinDVI failed.\n");
				LogMessage(_errMsg, ERROR_EVENT);
			}
		}
		else
		{
			logDll->TLTraceEvent(ERROR_EVENT,1,L"WinDVI UpdateWindow failed.\n");
			LogMessage(_errMsg, ERROR_EVENT);
		}
		
		//set flag with done:
		winDVIStatus = IDevice::STATUS_READY;
		return TRUE;
	}
	else
	{
		winDVIStatus = IDevice::STATUS_ERROR;
	}
	return FALSE;
}

long CWinDVI::GetStatus(long &status)
{
	status = winDVIStatus;
	return TRUE;
}

long CWinDVI::GetLastErrorMsg(wchar_t * msg, long size)
{
	wcsncpy_s(msg,size,_errMsg,MSG_SIZE);

	//reset the error message
	_errMsg[0] = 0;
	return TRUE;
}

void CWinDVI::LogMessage(wchar_t *logMsg,long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}
