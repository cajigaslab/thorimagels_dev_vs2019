#pragma once

#include "stdafx.h"
#include "MonitorInfo.h"

#ifdef __cplusplus
extern "C"
{
#endif

	class CWinDVI
	{
	private:
		static bool _instanceFlag;
		static std::auto_ptr<CWinDVI> _single;
		static wchar_t _errMsg[MSG_SIZE];
		static BYTE* _bmpBuffers[MAX_BUFFER_CNT]; ///<bitmap buffers for dynamic display
		static long _bmpBufID; ///<index of bitmap buffer to display, zero-based, default: -1

		CSysDisplays _cDisplay;
		HANDLE hWinThread;

	public:
		static int winWidth;
		static int winHeight;
		static CMonitorInfoEx cMInfo;

		static CWinDVI* getInstance();
		~CWinDVI();

		long ChooseDVI(const wchar_t* id);
		long CreateDVIWindow(int w, int h);
		long EditBMP(int id, unsigned char* bmpBuf, BITMAPINFO bmpInfo);
		void ClearBMPs();
		long DisplayBMP(int id);
		void DestroyDVIWindow();
		long GetStatus(long& status);///<status will follow IDevice status: (0) BUSY, (1) READY, (2) ERROR

		long GetLastErrorMsg(wchar_t* msg, long size);
		static void LogMessage(wchar_t* message, long eventLevel);

	private:
		CWinDVI();
		static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
		static ATOM MyRegisterClass(HINSTANCE hInstance);
		static void PaintWindow();

	};

#ifdef __cplusplus
}
#endif
