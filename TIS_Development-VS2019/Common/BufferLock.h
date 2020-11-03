#pragma once

class BufferLock
{
private:
	CRITICAL_SECTION _cSection;

public:
	BufferLock()
	{
		::InitializeCriticalSection(&_cSection);
	}

	~BufferLock()
	{
		::DeleteCriticalSection(&_cSection);
	}

	void Enter()
	{
		::EnterCriticalSection(&_cSection);
	}

	BOOL TryEnter()
	{
		return ::TryEnterCriticalSection(&_cSection);
	}

	void Leave()
	{
		::LeaveCriticalSection(&_cSection);
	}

};