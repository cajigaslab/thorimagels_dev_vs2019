#include "stdafx.h"
#include "WaveformMemory.h"


///***	static members	***///
bool WaveformMemory::_instanceFlag = false;
std::unique_ptr<WaveformMemory> WaveformMemory::_single;

WaveformMemory::WaveformMemory()
{
	InitializeCriticalSection(&_hFileCriticalSection);

	//initialize the memory mapped file output to the temp directory for the current user
	DWORD result = ::GetTempPath(0, L"");

	if(result != 0)
	{
		std::vector<TCHAR> tempPath(result + 1);
		result = ::GetTempPath(static_cast<DWORD>(tempPath.size()), &tempPath[0]);
		std::wstring str(tempPath.begin(), tempPath.begin() + static_cast<std::size_t>(result));

		_memMapPath = str;
	}
	hFileMapArray = hFileArray = NULL;
	_dwlHeaderSize = _dwlAnalogXYSize = _dwlAnalogPoSize = _dwlDigitalLSize = NULL;

	_doubleByteSize = GetDataTypeSizeInBytes<double>();
	_ushortByteSize = GetDataTypeSizeInBytes<USHORT>();
	_uint64ByteSize = GetDataTypeSizeInBytes<uint64_t>();
	_byteByteSize = GetDataTypeSizeInBytes<BYTE>();
}

WaveformMemory* WaveformMemory::getInstance()
{
	if(!_instanceFlag)
	{
		try
		{
			_single.reset(new WaveformMemory());
			_instanceFlag = true;
		}
		catch(...)
		{
			throw;
		}
	}
	return _single.get();
}

// create memory map file at folder path
long WaveformMemory::AllocateMem(GGalvoWaveformParams gWParams, const wchar_t* memMapPath)
{
	DWORD dwError;

	if((0 >= gWParams.ClockRate) || (0 >= gWParams.analogXYSize) || (0 >= gWParams.analogPockelSize) 
		|| (0 >= gWParams.digitalSize) || (0 >= gWParams.stepVolt))
	{
		return FALSE;
	}
	try
	{
		wchar_t uniqueFileName[_MAX_PATH];
		std::wstring mPath(memMapPath);

		//if a path is passed in use it. otherwise default to standard temp path
		if(mPath.size() > 0)
		{	_memMapPath = mPath.c_str();	}

		if(_tempFileName.size()>0)
		{	StringCbPrintfW(uniqueFileName,_MAX_PATH,L"%s%s",_memMapPath.c_str(),_tempFileName.c_str());	}
		else
		{	::GetTempFileName(_memMapPath.c_str(),L"tis",0,uniqueFileName);		}

		CloseMem();
		hFileArray = new HANDLE[1];
		hFileMapArray = new HANDLE[1];
		//Create file
		hFileArray[0] = CreateFile(uniqueFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH|FILE_FLAG_NO_BUFFERING|FILE_FLAG_OVERLAPPED ,NULL);	

		if(NULL == hFileArray[0])
		{	
			dwError = GetLastError();
			return FALSE;
		}
		
		_dwlHeaderSize = (4 * _uint64ByteSize) + _doubleByteSize;
		_dwlAnalogXYSize = _doubleByteSize * gWParams.analogXYSize;
		_dwlAnalogPoSize = _doubleByteSize * gWParams.analogPockelSize;
		_dwlDigitalLSize = _byteByteSize * gWParams.digitalSize;


		DWORDLONG sizeLong = _dwlHeaderSize + _dwlAnalogXYSize + _dwlAnalogPoSize + _dwlDigitalLSize;
		DWORDLONG dwllowOrderSize = sizeLong & 0xFFFFFFFF;					//0xFFFFFFFFul	
		DWORDLONG dwlhighOrderSize = (sizeLong & 0xFFFFFFFF00000000)>>32;	//0xFFFFFFFFul	
		DWORD lowOrderSize = static_cast<DWORD>(dwllowOrderSize);
		DWORD highOrderSize = static_cast<DWORD>(dwlhighOrderSize);
		//Open file mapping called WaveformMapping
		hFileMapArray[0] = CreateFileMapping(hFileArray[0], NULL, PAGE_READWRITE, highOrderSize, lowOrderSize,L"WaveformMapping");


		if(NULL == hFileMapArray[0])
		{	
			dwError = GetLastError();

			switch(dwError)
			{
			case ERROR_DISK_FULL:
				break;
			default:
				break;
			}
			return FALSE;
		}

		SYSTEM_INFO si;
		GetSystemInfo(&si);
		_granularityOfMapView = si.dwAllocationGranularity;

		//fill header: clockRate, xySize, pockelSize, digLineSize, deltaVolt
		char *p = (char*)MapViewOfFile(hFileMapArray[0], FILE_MAP_ALL_ACCESS, 0x0, 0x0, _dwlHeaderSize);
		SAFE_MEMCPY(p, _uint64ByteSize, &(gWParams.ClockRate));
		SAFE_MEMCPY(p + _uint64ByteSize, _uint64ByteSize, &(gWParams.analogXYSize));
		SAFE_MEMCPY(p + 2 * _uint64ByteSize, _uint64ByteSize, &(gWParams.analogPockelSize));
		SAFE_MEMCPY(p + 3 * _uint64ByteSize, _uint64ByteSize, &(gWParams.digitalSize));
		SAFE_MEMCPY(p + 4 * _uint64ByteSize, _doubleByteSize, &(gWParams.stepVolt));

		if(FALSE == UnmapViewOfFile(p))
		{
			DWORD err = GetLastError();
		}
	}
	catch(...)
	{
		//system low on resources. unable to fully allocate waveform buffer
		return FALSE;
	}
	return TRUE;
}


// create memory map file at folder path
long WaveformMemory::AllocateMemThorDAQ(ThorDAQGGWaveformParams gWParams, const wchar_t* memMapPath)
{
	DWORD dwError;

	if((0 >= gWParams.ClockRate) || (0 >= gWParams.analogXYSize) || (0 >= gWParams.analogPockelSize) || 
		(0 >= gWParams.digitalSize) || (0 >= gWParams.stepVolt))
	{
		return FALSE;
	}
	try
	{
		wchar_t uniqueFileName[_MAX_PATH];
		std::wstring mPath(memMapPath);

		//if a path is passed in use it. otherwise default to standard temp path
		if(mPath.size() > 0)
		{	_memMapPath = mPath.c_str();	}

		if(_tempFileName.size()>0)
		{	StringCbPrintfW(uniqueFileName,_MAX_PATH,L"%s%s",_memMapPath.c_str(),_tempFileName.c_str());	}
		else
		{	::GetTempFileName(_memMapPath.c_str(),L"tis",0,uniqueFileName);		}

		CloseMem();
		hFileArray = new HANDLE[1];
		hFileMapArray = new HANDLE[1];
		//Create file
		hFileArray[0] = CreateFile(uniqueFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH|FILE_FLAG_NO_BUFFERING|FILE_FLAG_OVERLAPPED ,NULL);	

		if(NULL == hFileArray[0])
		{	
			dwError = GetLastError();
			return FALSE;
		}
		
		_dwlHeaderSize = (4 * _uint64ByteSize) + _doubleByteSize;
		_dwlAnalogXYSize = _ushortByteSize * gWParams.analogXYSize;
		_dwlAnalogPoSize = _ushortByteSize * gWParams.analogPockelSize;
		_dwlDigitalLSize = _byteByteSize * gWParams.digitalSize;


		DWORDLONG sizeLong = _dwlHeaderSize + _dwlAnalogXYSize + _dwlAnalogPoSize + _dwlDigitalLSize;
		DWORDLONG dwllowOrderSize = sizeLong & 0xFFFFFFFF;					//0xFFFFFFFFul	
		DWORDLONG dwlhighOrderSize = (sizeLong & 0xFFFFFFFF00000000)>>32;	//0xFFFFFFFFul	
		DWORD lowOrderSize = static_cast<DWORD>(dwllowOrderSize);
		DWORD highOrderSize = static_cast<DWORD>(dwlhighOrderSize);
		//Open file mapping called WaveformMapping
		hFileMapArray[0] = CreateFileMapping(hFileArray[0], NULL, PAGE_READWRITE, highOrderSize, lowOrderSize,L"WaveformMapping");


		if(NULL == hFileMapArray[0])
		{	
			dwError = GetLastError();

			switch(dwError)
			{
			case ERROR_DISK_FULL:
				break;
			default:
				break;
			}
			return FALSE;
		}

		SYSTEM_INFO si;
		GetSystemInfo(&si);
		_granularityOfMapView = si.dwAllocationGranularity;

		//fill header: clockRate, xySize, pockelSize, digLineSize, deltaVolt
		char *p = (char*)MapViewOfFile(hFileMapArray[0], FILE_MAP_ALL_ACCESS, 0x0, 0x0, _dwlHeaderSize);
		SAFE_MEMCPY(p, _uint64ByteSize, &(gWParams.ClockRate));
		SAFE_MEMCPY(p + _uint64ByteSize, _uint64ByteSize, &(gWParams.analogXYSize));
		SAFE_MEMCPY(p + 2 * _uint64ByteSize, _uint64ByteSize, &(gWParams.analogPockelSize));
		SAFE_MEMCPY(p + 3 * _uint64ByteSize, _uint64ByteSize, &(gWParams.digitalSize));
		SAFE_MEMCPY(p + 4 * _uint64ByteSize, _doubleByteSize, &(gWParams.stepVolt));

		if(FALSE == UnmapViewOfFile(p))
		{
			DWORD err = GetLastError();
		}
	}
	catch(...)
	{
		//system low on resources. unable to fully allocate waveform buffer
		return FALSE;
	}
	return TRUE;
}

// close memory map file
void WaveformMemory::CloseMem()
{
	DWORD err;
	EnterCriticalSection(&_hFileCriticalSection);

	if(NULL != hFileMapArray)
	{
		if(NULL != hFileMapArray[0])
		{
			if(FALSE == CloseHandle(hFileMapArray[0]))
			{
				err = GetLastError();
			}
			delete[] hFileMapArray;
			hFileMapArray = NULL;
		}
	}
	if(NULL != hFileArray)
	{
		if(NULL != hFileArray[0])
		{
			if(FALSE == CloseHandle(hFileArray[0]))
			{
				err = GetLastError();
			}
			delete[] hFileArray;
			hFileArray = NULL;
		}
	}

	LeaveCriticalSection(&_hFileCriticalSection);
}

// open existing memory map file
long WaveformMemory::OpenMem(GGalvoWaveformParams& gWParams, const wchar_t* memMapPathName)
{
	std::wstring mPathName(memMapPathName);

	if(0 >= mPathName.size())
		return FALSE;	

	DWORD dwError;

	try
	{
		CloseMem();

		hFileArray = new HANDLE[1];
		hFileMapArray = new HANDLE[1];

		hFileArray[0] = CreateFile(mPathName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED ,NULL);	
		if(NULL == hFileArray[0])
		{	
			dwError = GetLastError();
			return FALSE;
		}

		_dwlHeaderSize = (4 * _uint64ByteSize) + _doubleByteSize;

		DWORDLONG dwllowOrderSize = _dwlHeaderSize & 0xFFFFFFFF;					
		DWORDLONG dwlhighOrderSize = (_dwlHeaderSize & 0xFFFFFFFF00000000)>>32;	
		DWORD lowOrderSize = static_cast<DWORD>(dwllowOrderSize);
		DWORD highOrderSize = static_cast<DWORD>(dwlhighOrderSize);
		//Open file mapping called WaveformRead
		hFileMapArray[0] = CreateFileMapping(hFileArray[0], NULL, PAGE_READONLY, highOrderSize, lowOrderSize, L"WaveformRead");


		if(NULL == hFileMapArray[0] || (INVALID_HANDLE_VALUE == hFileMapArray[0]))
		{	
			throw;
		}

		SYSTEM_INFO si;
		GetSystemInfo(&si);
		_granularityOfMapView = si.dwAllocationGranularity;

		//fill header:
		char *p = (char*)MapViewOfFile(hFileMapArray[0], FILE_MAP_READ, 0x0, 0x0, _dwlHeaderSize);
		SAFE_MEMCPY((void*)(&(gWParams.ClockRate)), _uint64ByteSize, p);
		SAFE_MEMCPY((void*)(&(gWParams.analogXYSize)), _uint64ByteSize, (p + _uint64ByteSize));
		SAFE_MEMCPY((void*)(&(gWParams.analogPockelSize)), _uint64ByteSize, (p + 2 * _uint64ByteSize));
		SAFE_MEMCPY((void*)(&(gWParams.digitalSize)), _uint64ByteSize, (p + 3 * _uint64ByteSize));
		SAFE_MEMCPY((void*)(&(gWParams.stepVolt)), _doubleByteSize, (p + 4 * _uint64ByteSize));

		_dwlAnalogXYSize = _doubleByteSize * gWParams.analogXYSize;
		_dwlAnalogPoSize = _doubleByteSize * gWParams.analogPockelSize;
		_dwlDigitalLSize = _byteByteSize * gWParams.digitalSize;

		if(FALSE == UnmapViewOfFile(p))
		{
			dwError = GetLastError();
		}

		CloseMem();

		//redo create map:
		hFileArray = new HANDLE[1];
		hFileMapArray = new HANDLE[1];

		hFileArray[0] = CreateFile(mPathName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED ,NULL);	

		DWORDLONG sizeLong = _dwlHeaderSize + _dwlAnalogXYSize + _dwlAnalogPoSize + _dwlDigitalLSize;
		dwllowOrderSize = sizeLong & 0xFFFFFFFF;					
		dwlhighOrderSize = (sizeLong & 0xFFFFFFFF00000000)>>32;	
		lowOrderSize = static_cast<DWORD>(dwllowOrderSize);
		highOrderSize = static_cast<DWORD>(dwlhighOrderSize);
		//Open file mapping called WaveformRead
		hFileMapArray[0] = CreateFileMapping(hFileArray[0], NULL, PAGE_READWRITE, highOrderSize, lowOrderSize, L"WaveformRead");

		if(NULL == hFileMapArray[0] || (INVALID_HANDLE_VALUE == hFileMapArray[0]))
		{	
			throw;
		}

	}
	catch(...)
	{
		dwError = GetLastError();

		switch(dwError)
		{
		case ERROR_OPEN_FAILED:
			break;
		default:
			break;
		}
		return FALSE;
	}
	return TRUE;
}

// open existing memory map file
long WaveformMemory::OpenMemThorDAQ(ThorDAQGGWaveformParams& gWParams, const wchar_t* memMapPathName)
{
	std::wstring mPathName(memMapPathName);

	if(0 >= mPathName.size())
		return FALSE;	

	DWORD dwError;

	try
	{
		CloseMem();

		hFileArray = new HANDLE[1];
		hFileMapArray = new HANDLE[1];

		hFileArray[0] = CreateFile(mPathName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED ,NULL);	
		if(NULL == hFileArray[0])
		{	
			dwError = GetLastError();
			return FALSE;
		}

		_dwlHeaderSize = (4 * _uint64ByteSize) + _doubleByteSize;

		DWORDLONG dwllowOrderSize = _dwlHeaderSize & 0xFFFFFFFF;					
		DWORDLONG dwlhighOrderSize = (_dwlHeaderSize & 0xFFFFFFFF00000000)>>32;	
		DWORD lowOrderSize = static_cast<DWORD>(dwllowOrderSize);
		DWORD highOrderSize = static_cast<DWORD>(dwlhighOrderSize);
		//Open file mapping called WaveformRead
		hFileMapArray[0] = CreateFileMapping(hFileArray[0], NULL, PAGE_READONLY, highOrderSize, lowOrderSize, L"WaveformRead");


		if(NULL == hFileMapArray[0] || (INVALID_HANDLE_VALUE == hFileMapArray[0]))
		{	
			throw;
		}

		SYSTEM_INFO si;
		GetSystemInfo(&si);
		_granularityOfMapView = si.dwAllocationGranularity;

		//fill header:
		char *p = (char*)MapViewOfFile(hFileMapArray[0], FILE_MAP_READ, 0x0, 0x0, _dwlHeaderSize);
		SAFE_MEMCPY((void*)(&(gWParams.ClockRate)), _uint64ByteSize, p);
		SAFE_MEMCPY((void*)(&(gWParams.analogXYSize)), _uint64ByteSize, (p + _uint64ByteSize));
		SAFE_MEMCPY((void*)(&(gWParams.analogPockelSize)), _uint64ByteSize, (p + 2 * _uint64ByteSize));
		SAFE_MEMCPY((void*)(&(gWParams.digitalSize)), _uint64ByteSize, (p + 3 * _uint64ByteSize));
		SAFE_MEMCPY((void*)(&(gWParams.stepVolt)), _doubleByteSize, (p + 4 * _uint64ByteSize));

		_dwlAnalogXYSize = _ushortByteSize * gWParams.analogXYSize;
		_dwlAnalogPoSize = _ushortByteSize * gWParams.analogPockelSize;
		_dwlDigitalLSize = _byteByteSize * gWParams.digitalSize;

		if(FALSE == UnmapViewOfFile(p))
		{
			dwError = GetLastError();
		}

		CloseMem();

		//redo create map:
		hFileArray = new HANDLE[1];
		hFileMapArray = new HANDLE[1];

		hFileArray[0] = CreateFile(mPathName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED ,NULL);	

		DWORDLONG sizeLong = _dwlHeaderSize + _dwlAnalogXYSize + _dwlAnalogPoSize + _dwlDigitalLSize;
		dwllowOrderSize = sizeLong & 0xFFFFFFFF;					
		dwlhighOrderSize = (sizeLong & 0xFFFFFFFF00000000)>>32;	
		lowOrderSize = static_cast<DWORD>(dwllowOrderSize);
		highOrderSize = static_cast<DWORD>(dwlhighOrderSize);
		//Open file mapping called WaveformRead
		hFileMapArray[0] = CreateFileMapping(hFileArray[0], NULL, PAGE_READWRITE, highOrderSize, lowOrderSize, L"WaveformRead");

		if(NULL == hFileMapArray[0] || (INVALID_HANDLE_VALUE == hFileMapArray[0]))
		{	
			throw;
		}

	}
	catch(...)
	{
		dwError = GetLastError();

		switch(dwError)
		{
		case ERROR_OPEN_FAILED:
			break;
		default:
			break;
		}
		return FALSE;
	}
	return TRUE;
}

// get memory map with offset and size
char *WaveformMemory::GetMemMapPtr(SignalType stype, uint64_t offset, uint64_t size)
{
	DWORDLONG mapOffset = _dwlHeaderSize;
	DWORDLONG mapSize = size;
	switch (stype)
	{
	case SignalType::ANALOG_XY:
		mapOffset += offset * _doubleByteSize;
		mapSize *= _doubleByteSize;
		break;
	case SignalType::ANALOG_POCKEL:
		mapOffset += _dwlAnalogXYSize + offset * _doubleByteSize;
		mapSize *= _doubleByteSize;
		break;
	case SignalType::DIGITAL_LINES:
		mapOffset += _dwlAnalogXYSize + _dwlAnalogPoSize + offset * _byteByteSize;
		mapSize *= _byteByteSize;
		break;
	case SignalType::SIGNALTYPE_LAST:
	default:
		return NULL;
		break;
	}

	DWORDLONG val = mapOffset/_granularityOfMapView;

	DWORDLONG allowedOffset = val * _granularityOfMapView;

	DWORDLONG dwllowOrderOffset = allowedOffset & 0xFFFFFFFF;					//0xFFFFFFFFul
	DWORDLONG dwlhighOrderOffset = (allowedOffset & 0xFFFFFFFF00000000)>>32;	//0xFFFFFFFFul
	DWORD lowOffset = static_cast<DWORD>(dwllowOrderOffset);
	DWORD highOffset = static_cast<DWORD>(dwlhighOrderOffset);

	long memOffset = static_cast<long>(mapOffset - allowedOffset);

	if(NULL == hFileMapArray)
		return NULL;

	//Get pointer to memory representing file
	ptrMap = (char*)MapViewOfFile(hFileMapArray[0], FILE_MAP_ALL_ACCESS, highOffset, lowOffset, mapSize + memOffset);
	if(ptrMap == NULL)
	{
		DWORD err = GetLastError();
		throw;
	}

	char*retPtr = ptrMap + memOffset;
	return retPtr;
}

// get memory map with offset and size
char *WaveformMemory::GetMemMapPtrThorDAQ(SignalType stype, uint64_t offset, uint64_t size)
{
	DWORDLONG mapOffset = _dwlHeaderSize;
	DWORDLONG mapSize = size;
	switch (stype)
	{
	case SignalType::ANALOG_XY:
		mapOffset += offset * _ushortByteSize;
		mapSize *= _ushortByteSize;
		break;
	case SignalType::ANALOG_POCKEL:
		mapOffset += _dwlAnalogXYSize + offset * _ushortByteSize;
		mapSize *= _ushortByteSize;
		break;
	case SignalType::DIGITAL_LINES:
		mapOffset += _dwlAnalogXYSize + _dwlAnalogPoSize + offset * _byteByteSize;
		mapSize *= _byteByteSize;
		break;
	case SignalType::SIGNALTYPE_LAST:
	default:
		return NULL;
		break;
	}

	DWORDLONG val = mapOffset/_granularityOfMapView;

	DWORDLONG allowedOffset = val * _granularityOfMapView;

	DWORDLONG dwllowOrderOffset = allowedOffset & 0xFFFFFFFF;					//0xFFFFFFFFul
	DWORDLONG dwlhighOrderOffset = (allowedOffset & 0xFFFFFFFF00000000)>>32;	//0xFFFFFFFFul
	DWORD lowOffset = static_cast<DWORD>(dwllowOrderOffset);
	DWORD highOffset = static_cast<DWORD>(dwlhighOrderOffset);

	long memOffset = static_cast<long>(mapOffset - allowedOffset);

	if(NULL == hFileMapArray)
		return NULL;

	//Get pointer to memory representing file
	ptrMap = (char*)MapViewOfFile(hFileMapArray[0], FILE_MAP_ALL_ACCESS, highOffset, lowOffset, mapSize + memOffset);
	if(ptrMap == NULL)
	{
		DWORD err = GetLastError();
		throw;
	}

	char*retPtr = ptrMap + memOffset;
	return retPtr;
}


// release memory map
void WaveformMemory::UnlockMemMapPtr()
{
	if(FALSE == UnmapViewOfFile(ptrMap))
	{
		DWORD err = GetLastError();
	}
}

// set memory map file name
long WaveformMemory::SetTempFileName(const wchar_t* tFileName)
{		
	_tempFileName = tFileName;
	return TRUE;
}

long WaveformMemory::SaveWaveformDataStruct(const wchar_t* tPathName, GGalvoWaveformParams waveformParams)
{
	//get path:
	std::wstring mPathName(tPathName);
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	wchar_t rawPath[_MAX_PATH];
	_wsplitpath_s(mPathName.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
	StringCbPrintfW(rawPath,_MAX_PATH,L"%s%s",fname,L".raw");
	SetTempFileName(rawPath);
	StringCbPrintfW(rawPath,_MAX_PATH,L"%s%s",drive,dir);

	//save to file:
	if (TRUE == AllocateMem(waveformParams, rawPath))
	{
		//analogXY, interleave XY:
		double* tmp = (double*)malloc(waveformParams.analogXYSize * _doubleByteSize);
		double* pDst = tmp;
		for (unsigned long long i = 0; i < waveformParams.analogPockelSize; i++)
		{
			*pDst = *(waveformParams.GalvoWaveformXY + i);
			pDst++;
			*pDst = *(waveformParams.GalvoWaveformXY + waveformParams.analogPockelSize + i);
			pDst++;
		}	
		char* ptr = GetMemMapPtr(SignalType::ANALOG_XY, 0, waveformParams.analogXYSize);
		SAFE_MEMCPY(ptr,waveformParams.analogXYSize*_doubleByteSize,tmp);
		UnlockMemMapPtr();
		free(tmp);

		//analogPockel:
		ptr = GetMemMapPtr(SignalType::ANALOG_POCKEL, 0, waveformParams.analogPockelSize);
		SAFE_MEMCPY(ptr,waveformParams.analogPockelSize*_doubleByteSize,waveformParams.GalvoWaveformPockel);
		UnlockMemMapPtr();

		//digital lines:
		ptr = GetMemMapPtr(SignalType::DIGITAL_LINES, 0, waveformParams.digitalSize);
		SAFE_MEMCPY(ptr,waveformParams.digitalSize*_byteByteSize,waveformParams.DigBufWaveform);
		UnlockMemMapPtr();

		CloseMem();
	}
	return TRUE;
}

long WaveformMemory::SaveThorDAQWaveformDataStruct(const wchar_t* tPathName, ThorDAQGGWaveformParams waveformParams)
{
	//get path:
	std::wstring mPathName(tPathName);
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	wchar_t rawPath[_MAX_PATH];
	_wsplitpath_s(mPathName.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
	StringCbPrintfW(rawPath,_MAX_PATH,L"%s%s",fname,L".raw");
	SetTempFileName(rawPath);
	StringCbPrintfW(rawPath,_MAX_PATH,L"%s%s",drive,dir);

	//save to file:
	if (TRUE == AllocateMemThorDAQ(waveformParams, rawPath))
	{
		//analogXY, interleave XY:
		double* tmp = (double*)malloc(waveformParams.analogXYSize * _doubleByteSize);
		double* pDst = tmp;
		for (unsigned long long i = 0; i < waveformParams.analogPockelSize; i++)
		{
			*pDst = *(waveformParams.GalvoWaveformXY + i);
			pDst++;
			*pDst = *(waveformParams.GalvoWaveformXY + waveformParams.analogPockelSize + i);
			pDst++;
		}	
		char* ptr = GetMemMapPtrThorDAQ(SignalType::ANALOG_XY, 0, waveformParams.analogXYSize);
		SAFE_MEMCPY(ptr,waveformParams.analogXYSize*_ushortByteSize,tmp);
		UnlockMemMapPtr();
		free(tmp);

		//analogPockel:
		ptr = GetMemMapPtrThorDAQ(SignalType::ANALOG_POCKEL, 0, waveformParams.analogPockelSize);
		SAFE_MEMCPY(ptr,waveformParams.analogPockelSize*_ushortByteSize,waveformParams.GalvoWaveformPockel);
		UnlockMemMapPtr();

		//digital lines:
		ptr = GetMemMapPtrThorDAQ(SignalType::DIGITAL_LINES, 0, waveformParams.digitalSize);
		SAFE_MEMCPY(ptr,waveformParams.digitalSize*_byteByteSize,waveformParams.DigBufWaveform);
		UnlockMemMapPtr();

		CloseMem();
	}
	return TRUE;
}


template<typename T>
long WaveformMemory::GetDataTypeSizeInBytes()
{
	return sizeof(T)/sizeof(char);
}
