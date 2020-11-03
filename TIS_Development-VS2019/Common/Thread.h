#pragma once

//------------------------------------
//  Reliable Software, (c) 1996-2002
//------------------------------------

class CritSect
{
	friend class Lock;
public:
	CritSect () { ::InitializeCriticalSection (&_critSection); }
	~CritSect () { ::DeleteCriticalSection (&_critSection); }
private:
	void Acquire () 
	{ 
		::EnterCriticalSection (&_critSection);
	}
	void Release () 
	{ 
		::LeaveCriticalSection (&_critSection);
	}

	CRITICAL_SECTION _critSection;
};

class Lock 
{
public:
	// Acquire the state of the semaphore
	Lock (CritSect & mutex ) : _critSect(mutex) 
	{
		_critSect.Acquire();
	}
	// Release the state of the semaphore
	~Lock ()
	{
		_critSect.Release();
	}
private:
	CritSect & _critSect;
};
