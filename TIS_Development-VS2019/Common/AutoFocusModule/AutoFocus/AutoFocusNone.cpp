#include "stdafx.h"
#include "..\..\..\Commands\General\SelectHardware\SelectHardware\SelectHardware.h"
#include "AutoFocusNone.h"


extern auto_ptr<CommandDll> shwDll;
extern auto_ptr<TiffLibDll> tiffDll;

AutoFocusNone::AutoFocusNone()
{
	_counter = 0;
}


HANDLE AutoFocusNone::hEventAutoFocus = NULL;


long AutoFocusNone::WillAFExecuteNextIteration()
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

long AutoFocusNone::Execute(long index, IDevice * pAutoFocus, BOOL &afFound)
{
	//the first image will always execute autofocus with (counter == 0) 
	if ((_counter % _repeat) != 0)
	{	
		_counter++;
		return TRUE;
	}
	_counter++;

	logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AutoFocusNone Execute");
	
	return TRUE;
}


void AutoFocusNone::SetupParameters(long repeat)
{
	_repeat = repeat;
}