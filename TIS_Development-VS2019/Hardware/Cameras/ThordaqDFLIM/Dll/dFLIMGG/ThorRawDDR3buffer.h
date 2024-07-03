#pragma once
#include "stdafx.h"
class ThorRawDDR3buffer
{
private:
	HANDLE _hProcess;
	LPVOID _lpAddress;
	SIZE_T _dsSize;
	DWORD  _dwAllocationType;
	DWORD  _protect, _lastErr;
public:
	ThorRawDDR3buffer()
	{
		_hProcess = NULL;
		_lastErr = 0;
		_dsSize = 0; 	// maximum possible (ADC) S2MM DMA size
		_dwAllocationType = MEM_COMMIT;
		_protect = PAGE_READWRITE;
		_lpAddress = VirtualAlloc(NULL, _dsSize, _dwAllocationType, _protect); // let O/S assign virt. address
		if (_lpAddress == nullptr)
			_lastErr = GetLastError();
	}

	ThorRawDDR3buffer(SIZE_T MaxSizeInBytes) // constructor
	{
		_hProcess = NULL;
		_lastErr = 0;
		_dsSize = MaxSizeInBytes; 	// maximum possible raw DDR3 buff size
		_dwAllocationType = MEM_COMMIT;
		_protect = PAGE_READWRITE;
		_lpAddress = VirtualAlloc(NULL, _dsSize, _dwAllocationType, _protect); // let O/S assign virt. address
		if (_lpAddress == nullptr)
			_lastErr = GetLastError();
	}

	// can have multiple "frames" per channel (per bank), on 1k boundary (legacy code requirement)
	UCHAR* GetChannelStartAddress(int channel, size_t AllocSize)
	{
		if (_lpAddress == nullptr)
			return nullptr;
		UCHAR* startPtr = PUINT8(_lpAddress) + (channel * AllocSize);
		return startPtr;
	}

	~ThorRawDDR3buffer()
	{
		if (_lpAddress != nullptr)
		{
			BOOL bStatus = VirtualFree(_lpAddress, 0, MEM_RELEASE); // dwSize must be 0 to release ALL mem
		}
	}
};