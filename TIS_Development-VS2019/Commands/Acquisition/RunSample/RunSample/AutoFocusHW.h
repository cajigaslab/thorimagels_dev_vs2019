#pragma once

#include "AutoFocus.h"

class AutoFocusHW : public IAutoFocus
{
public:
	AutoFocusHW();

	virtual long Execute(long index, IDevice *pAutoFocus,BOOL &afFound);//Synchrnous execution of autofocus

	void SetupParameters(long repeat,double focusOffset);

	long WillAFExecuteNextIteration();

	static HANDLE hEventAutoFocus;

private:
	long _repeat;
	double _focusOffset;
	long _counter;
};
