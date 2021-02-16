#pragma once
#include "..\..\ImageWaveformBuilderDll.h"
#include "..\..\ThorSharedTypesCPP.h"

class WaveformMemory
{
public:
	~WaveformMemory(){ CloseMem(); _instanceFlag = false; }
	//singleton:
	static WaveformMemory* getInstance();

	long AllocateMem(GGalvoWaveformParams gWParams, const wchar_t* memMapPath, long lpSecurityAttributes);
	long AllocateMemThorDAQ(ThorDAQGGWaveformParams gWParams, const wchar_t* memMapPath);
	long OpenMem(GGalvoWaveformParams& gWParams, const wchar_t* memMapPathName);
	long OpenMemThorDAQ(ThorDAQGGWaveformParams& gWParams, const wchar_t* memMapPathName);
	void CloseMem();
	char *GetMemMapPtr(SignalType stype, uint64_t offset, uint64_t size);
	char *GetMemMapPtrThorDAQ(SignalType stype, uint64_t offset, uint64_t size);
	void UnlockMemMapPtr();
	long SetTempFileName(const wchar_t* tFileName);
	long SaveWaveformDataStruct(const wchar_t* tPathName, GGalvoWaveformParams waveformParams);
	long SaveThorDAQWaveformDataStruct(const wchar_t* tPathName, ThorDAQGGWaveformParams waveformParams);

	template<typename T>
	static long GetDataTypeSizeInBytes(){ return sizeof(T)/sizeof(char); }

private:
	static bool _instanceFlag;
	static std::unique_ptr<WaveformMemory> _single;
	CRITICAL_SECTION _hFileCriticalSection; 

	DWORDLONG _dwlHeaderSize;
	DWORDLONG _dwlAnalogXYSize;
	DWORDLONG _dwlAnalogPoSize;
	DWORDLONG _dwlAnalogPo1Size;
	DWORDLONG _dwlAnalogPo2Size;
	DWORDLONG _dwlAnalogPo3Size;
	DWORDLONG _dwlDigitalLSize;
	DWORDLONG _dwlAnalogPZSize;
	long	  _granularityOfMapView;
	long	  _doubleByteSize;
	long	  _ushortByteSize;
	long	  _uint64ByteSize;
	long	  _byteByteSize;

	HANDLE *hFileArray;
	HANDLE *hFileMapArray;
	char   *ptrMap;

	std::wstring _memMapPath;
	std::wstring _tempFileName;

	WaveformMemory();

};