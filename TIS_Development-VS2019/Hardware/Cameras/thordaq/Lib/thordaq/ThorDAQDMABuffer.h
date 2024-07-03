#pragma once
#include "stdafx.h"
class ThorDAQDMAbuffer
{
private:
	HANDLE _hProcess;
	LPVOID _lpAddress;
	SIZE_T _dsSize;
	DWORD  _dwAllocationType;
	DWORD  _protect, _lastErr;
public:
	ThorDAQDMAbuffer()
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

	ThorDAQDMAbuffer(SIZE_T MaxSizeInBytes) // constructor
	{
		_hProcess = NULL;
		_lastErr = 0;
		_dsSize = MaxSizeInBytes; 	// maximum possible (ADC) S2MM DMA size
		_dwAllocationType = MEM_COMMIT;
		_protect = PAGE_READWRITE;
		_lpAddress = VirtualAlloc(NULL, _dsSize, _dwAllocationType, _protect); // let O/S assign virt. address
		if (_lpAddress == nullptr)
			_lastErr = GetLastError();
	}
	// segment the max size buffer into MAX_CHANNELs according to current pixel Width x Height
	// "frame" size
	UCHAR* GetChannelStartAddress(int channel, int pixelWidth, int pixelHeight)
	{
		UCHAR* startPtr = PUINT8(_lpAddress) + (channel * ((pixelWidth * pixelHeight) * 2));
		return startPtr;
	}
	// can have multiple "frames" per channel (per bank), on 1k boundary (legacy code requirement)
	UCHAR* GetChannelStartAddress(int channel, size_t AllocSize)
	{
		if (_lpAddress == nullptr)
			return nullptr;
		UCHAR* startPtr = PUINT8(_lpAddress) + (channel * AllocSize);
		return startPtr;
	}

	~ThorDAQDMAbuffer()
	{
		if (_lpAddress != nullptr)
		{
			BOOL bStatus = VirtualFree(_lpAddress, _dsSize, MEM_RELEASE);
		}
	}
};