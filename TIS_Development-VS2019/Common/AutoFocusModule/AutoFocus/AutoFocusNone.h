#pragma once

#include "AutoFocusClass.h"

class AutoFocusNone : public IAutoFocus
{
public:
	AutoFocusNone();

	long Execute(long index, IDevice *pAutoFocus,BOOL &afFound) override;//Synchrnous execution of autofocus

	void SetupParameters(long repeat);
	long WillAFExecuteNextIteration();

	static HANDLE hEventAutoFocus;

private:
	long _repeat;
	long _counter;
};
