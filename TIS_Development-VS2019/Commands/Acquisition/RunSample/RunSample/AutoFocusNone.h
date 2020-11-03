#pragma once

#include "AutoFocus.h"

class AutoFocusNone : public IAutoFocus
{
public:
	AutoFocusNone();

	virtual long Execute(long index, IDevice *pAutoFocus,BOOL &afFound);//Synchrnous execution of autofocus

	void SetupParameters(long repeat);
	long WillAFExecuteNextIteration();

	static HANDLE hEventAutoFocus;

private:
	long _repeat;
	long _counter;
};
