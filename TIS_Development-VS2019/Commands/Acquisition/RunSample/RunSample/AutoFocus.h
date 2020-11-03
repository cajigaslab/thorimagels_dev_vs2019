#pragma once

class IAutoFocus
{
public:

	enum
	{
		AF_HARDWARE,
		AF_HARDWARE_IMAGE,
		AF_IMAGE,
		AF_NONE
	};

	virtual long Execute(long index, IDevice * pAutoFocus, BOOL &bFound) = 0;//Synchrnous execution of autofocus
	virtual long WillAFExecuteNextIteration()=0;
};
