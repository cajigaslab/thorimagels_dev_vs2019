#pragma once

#include <string>
#include "./PDLL/pdll.h"
#include "BlockRingBuffer.h"

#define BUF_REGION_COUNT	3

enum BuilderType
{
	ALAZAR,
	GR,
	EPHYS_DO,
	EPHYS_AO
};

static int UnitSizeInBytes(SignalType input)
{
	int unitSizeInByte = 0;
	switch (input)
	{
	case SignalType::ANALOG_POCKEL:
	case SignalType::ANALOG_XY:
		unitSizeInByte = sizeof(double);
		break;
	case SignalType::DIGITAL_LINES:
		unitSizeInByte = sizeof(unsigned char);
		break;
	default:
		break;
	}
	return unitSizeInByte;
}

/// <summary>
/// abstract class for waveform generation, may refactor ImageWaveformBuilder in the future.
/// </summary>
class IWaveformBuilder
{
public:
	virtual void ConnectCallback(void* brBuf) = 0; ///<register to buffer available callback
	virtual void SetWaveformParams(void* params) = 0; ///<set data stucture for later waveform build
	virtual long TryBuildWaveform(uint64_t& totalLength) = 0; ///<build waveform body
	virtual HANDLE GetSignalHandle() = 0; ///<signal to check waveform being pushed to buffer or not

};

class ImageWaveformBuilderDLL : public PDLL
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(push)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(ImageWaveformBuilderDLL)
#pragma warning(pop)
#pragma warning(pop)	
	DECLARE_FUNCTION1(void, SetWaveformBuilderBoardID, long)
	DECLARE_FUNCTION0(unsigned long, GetForwardLines)
	DECLARE_FUNCTION0(unsigned long, GetOverallLines)
	DECLARE_FUNCTION0(long, GetFrameDataLength)
	DECLARE_FUNCTION0(long, GetLineDataLength)
	DECLARE_FUNCTION0(long, GetSamplesPadding)
	DECLARE_FUNCTION0(double, GetFrameTime)
	DECLARE_FUNCTION0(long, GetPockelsSamplesEffective)
	DECLARE_FUNCTION1(long, GetGGalvoWaveformParams, void*)
	DECLARE_FUNCTION4(long, GetGGalvoWaveformParamsWithStatus, SignalType, void*, long, uint64_t&)
	DECLARE_FUNCTION2(long, GetThorDAQGGWaveformParams, const wchar_t*, void*)
	DECLARE_FUNCTION4(long, GetThorDAQGGWaveformParamsAndBufferWithStatus, SignalType, void*, long, uint64_t&)
	DECLARE_FUNCTION3(long, GetGGalvoWaveformStartLoc, const wchar_t*, double*, long&)
	DECLARE_FUNCTION4(double*, GetTravelWaveform, double, long, double*, long&)
	DECLARE_FUNCTION1(void, SetWaveformGenParams, void*)
	DECLARE_FUNCTION0(void, ResetGGalvoWaveformParams)
	DECLARE_FUNCTION0(void, ResetThorDAQGGalvoWaveformParams)
	DECLARE_FUNCTION8(long, VerifyPolyLine, const long*, const long*, long, double, double, double, long , long&)
	DECLARE_FUNCTION0(long, BuildPolyLine)
	DECLARE_FUNCTION1(long, BuildSpiral, long)
	DECLARE_FUNCTION1(uint64_t, GetCounter, SignalType)
	DECLARE_FUNCTION0(void, ResetCounter)
	DECLARE_FUNCTION4(long, BuildImageWaveform, double*, long*, uint64_t*, wstring)
	DECLARE_FUNCTION5(long, BuildImageWaveformFromStart, long, double, double*, long*, uint64_t*)
	DECLARE_FUNCTION2(void, BufferAvailableCallbackFunc, long, long)
	DECLARE_FUNCTION2(void, ConnectBufferCallback, SignalType, BlockRingBuffer*)
	DECLARE_FUNCTION4(uint64_t, RebuildWaveformFromFile, const wchar_t*, double*, int, long*)
	DECLARE_FUNCTION4(uint64_t, RebuildThorDAQWaveformFromFile, const wchar_t*, unsigned short*, int, long*)
	DECLARE_FUNCTION0(void, CloseWaveformFile)

};

class WaveformMemoryDLL : public PDLL
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(WaveformMemoryDLL)
#pragma warning(pop)

	DECLARE_FUNCTION3(long, AllocateMem, GGalvoWaveformParams, const wchar_t*, long)
	DECLARE_FUNCTION2(long, OpenMem, GGalvoWaveformParams& , const wchar_t*)
	DECLARE_FUNCTION0(void, CloseMem)
	DECLARE_FUNCTION3(char*, GetMemMapPtr, SignalType, uint64_t, uint64_t)
	DECLARE_FUNCTION0(void, UnlockMemMapPtr)
	DECLARE_FUNCTION1(long, SetTempFileName, const wchar_t*)
	DECLARE_FUNCTION2(long, SaveWaveformDataStruct, const wchar_t*, GGalvoWaveformParams)
	DECLARE_FUNCTION2(long, SaveThorDAQWaveformDataStruct, const wchar_t*, ThorDAQGGWaveformParams)
	
};

class WaveformBuilderDLL : public PDLL, IWaveformBuilder
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(WaveformBuilderDLL)
#pragma warning(pop)

	DECLARE_FUNCTION1(void, SetBuilderType, BuilderType)
	DECLARE_FUNCTION1(void, ConnectCallback, void*)
	DECLARE_FUNCTION1(void, SetWaveformParams, void*)
	DECLARE_FUNCTION1(long, TryBuildWaveform, uint64_t&)
	DECLARE_FUNCTION0(HANDLE, GetSignalHandle)
};

/// <summary>
/// class to save waveform, use bleach waveforom data structure to maintain raw file header consistency.
/// </summary>
class IWaveformSaver
{
public:
	virtual long SaveData(wstring outPath, SignalType stype, void* gparam, unsigned long long length) = 0; ///<save data to file based on settings
};

class WaveformSaverDLL : public PDLL, IWaveformSaver
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(WaveformSaverDLL)
#pragma warning(pop)

	DECLARE_FUNCTION4(long, SaveData, wstring, SignalType, void*, unsigned long long)
};
