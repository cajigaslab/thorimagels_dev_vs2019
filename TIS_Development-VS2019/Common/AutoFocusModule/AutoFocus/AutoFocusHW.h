#pragma once

#include "AutoFocusClass.h"

class AutoFocusHW : public IAutoFocus
{
public:
	AutoFocusHW();

	long Execute(long index, IDevice* pAutoFocus, BOOL& afFound) override;//Synchrnous execution of autofocus

	void SetupParameters(long repeat,double focusOffset);

	void SetStopFlag(long stopValue);

	long WillAFExecuteNextIteration();

	static HANDLE hEventAutoFocus;

private:
	long _repeat;
	double _focusOffset;
	long _counter;

	static atomic_long _stopFlag;
};
