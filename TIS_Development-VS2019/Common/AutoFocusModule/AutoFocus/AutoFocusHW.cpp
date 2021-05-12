#include "stdafx.h"
#include "..\..\..\Commands\General\SelectHardware\SelectHardware\SelectHardware.h"
#include "AutoFocusHW.h"


extern auto_ptr<CommandDll> shwDll;
extern auto_ptr<TiffLibDll> tiffDll;

atomic_long AutoFocusHW::_stopFlag = FALSE;

UINT StatusAutoFocusHWThreadProc( LPVOID pParam )
{
	long status = ICamera::STATUS_BUSY;

	IDevice * pDevice = (IDevice*)pParam;

	while(status == IDevice::STATUS_BUSY)
	{
		if(FALSE == pDevice->StatusPosition(status))
		{
			break;
		}
	}

	SetEvent( AutoFocusHW::hEventAutoFocus);

	return 0;
}

AutoFocusHW::AutoFocusHW()
{
	_counter = 0;
}


HANDLE AutoFocusHW::hEventAutoFocus = NULL;

long AutoFocusHW::WillAFExecuteNextIteration()
{
	if ((_counter % _repeat) != 0)
	{	
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

long AutoFocusHW::Execute(long index, IDevice * pAutoFocus, BOOL &afFound)
{	
	//the first image will always execute autofocus with (counter == 0) 
	if ((_counter % _repeat) != 0)
	{	
		_counter++;
		return TRUE;
	}
	_counter++;

	pAutoFocus->SetParam(IDevice::PARAM_AUTOFOCUS_OFFSET,_focusOffset);

	//enable autofocus
	pAutoFocus->SetParam(IDevice::PARAM_AUTOFOCUS_POS, 1.0);

	pAutoFocus->PreflightPosition();

	pAutoFocus->SetupPosition ();

	pAutoFocus->StartPosition();

	hEventAutoFocus = CreateEvent(0, FALSE, FALSE, 0);

	DWORD dwThread;

	HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusAutoFocusHWThreadProc, pAutoFocus, 0, &dwThread );

	const long MAX_AF_WAIT_TIME = 10000;

	DWORD dwWait = WaitForSingleObject( hEventAutoFocus, MAX_AF_WAIT_TIME);

	if(dwWait != WAIT_OBJECT_0)
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"AutoFocus Hardware Execute AutoFocus failed");
		//return FALSE;
	}		

	CloseHandle(hThread);
	CloseHandle(hEventAutoFocus);

	pAutoFocus->PostflightPosition();	

	double param=0;

	pAutoFocus->GetParam(IDevice::PARAM_AUTOFOCUS_FOUND,param);
	
	afFound = static_cast<int>(param);

	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"AutoFocusHW Execute");

	return TRUE;
}

void AutoFocusHW::SetupParameters(long repeat,double focusOffset)
{
	_repeat = repeat;
	_focusOffset = focusOffset;
}

void AutoFocusHW::SetStopFlag(long stopValue)
{
	_stopFlag = stopValue;
}