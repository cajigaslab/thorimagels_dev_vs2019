// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ImageWaveformBuilder.h"
#include "WaveformMemory.h"
#include "WaveformBuilderFactory.h"
#include "WaveformSaver.h"

#ifdef IMGWFBUILDER_EXPORTS
#define DllExport_IMGWFBUILDER extern "C" __declspec( dllexport )
#else
#define DllExport_IMGWFBUILDER __declspec(dllimport)
#endif


BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

long _boardID = 0;

DllExport_IMGWFBUILDER void SetWaveformBuilderBoardID(long id)
{
	_boardID = id;
}

DllExport_IMGWFBUILDER unsigned long GetForwardLines()
{
	return ImageWaveformBuilder::getInstance(_boardID)->GetForwardLines();
}

DllExport_IMGWFBUILDER unsigned long GetOverallLines()
{
	return ImageWaveformBuilder::getInstance(_boardID)->GetOverallLines();
}

DllExport_IMGWFBUILDER long GetFrameDataLength()
{
	return ImageWaveformBuilder::getInstance(_boardID)->GetFrameDataLength();
}

DllExport_IMGWFBUILDER long GetLineDataLength()
{
	return ImageWaveformBuilder::getInstance(_boardID)->GetLineDataLength();
}

DllExport_IMGWFBUILDER long GetSamplesPadding()
{
	return ImageWaveformBuilder::getInstance(_boardID)->GetSamplesPadding();
}

DllExport_IMGWFBUILDER double GetFrameTime()
{
	return ImageWaveformBuilder::getInstance(_boardID)->GetFrameTime();
}

DllExport_IMGWFBUILDER long GetPockelsSamplesEffective()
{
	return ImageWaveformBuilder::getInstance(_boardID)->GetPockelsSamplesEffective();
}

DllExport_IMGWFBUILDER long GetGGalvoWaveformParams(void* params)
{
	return ImageWaveformBuilder::getInstance(_boardID)->GetGGalvoWaveformParams(params);
}

DllExport_IMGWFBUILDER long GetGGalvoWaveformParamsWithStatus(SignalType sType, void* params, long preCaptureStatus, uint64_t& indexNow)
{
	return ImageWaveformBuilder::getInstance(_boardID)->GetGGalvoWaveformParams(sType, params, preCaptureStatus, indexNow);
}

DllExport_IMGWFBUILDER long GetThorDAQGGWaveformParamsAndBufferWithStatus(SignalType sType, void* params, long preCaptureStatus, uint64_t& indexNow)
{
	return ImageWaveformBuilder::getInstance(_boardID)->GetThorDAQGGWaveformParams(sType, params, preCaptureStatus, indexNow);
}

DllExport_IMGWFBUILDER long GetThorDAQGGWaveformParams(const wchar_t* waveformFileName, void* params)
{
	return ImageWaveformBuilder::getInstance(_boardID)->GetThorDAQGGWaveformParams(waveformFileName, params);
}


DllExport_IMGWFBUILDER long GetGGalvoWaveformStartLoc(const wchar_t* waveformFileName, double* startXY, long& clockRate)
{
	return ImageWaveformBuilder::getInstance(_boardID)->GetGGalvoWaveformStartLoc(waveformFileName, startXY, clockRate);
}

DllExport_IMGWFBUILDER long GetThorDAQGGWaveformStartLoc(const wchar_t* waveformFileName, unsigned short* startXY, long& clockRate)
{
	return ImageWaveformBuilder::getInstance(_boardID)->GetThorDAQGGWaveformStartLoc(waveformFileName, startXY, clockRate);
}

DllExport_IMGWFBUILDER double* GetTravelWaveform(double stepSize, long outputInterleave, double* posFromXYToXY, long& count)
{
	return ImageWaveformBuilder::getInstance(_boardID)->GetTravelWaveform(stepSize, outputInterleave, posFromXYToXY, count);
}

DllExport_IMGWFBUILDER void SetWaveformGenParams(void* params)
{
	ImageWaveformBuilder::getInstance(_boardID)->SetWaveformGenParams(params);
}

DllExport_IMGWFBUILDER void ResetGGalvoWaveformParams()
{
	ImageWaveformBuilder::getInstance(_boardID)->ResetGGalvoWaveformParams();
}

DllExport_IMGWFBUILDER void ResetThorDAQGGalvoWaveformParams()
{
	ImageWaveformBuilder::getInstance(_boardID)->ResetGGalvoWaveformParams();
}

DllExport_IMGWFBUILDER long VerifyPolyLine(const long* Ptx, const long* Pty, long PtCnt, long fieldSize, double field2Volts, double fieldScaleFineX, double fieldScaleFineY, long PixelY, long &PixelX)
{
	long ptxVal = 0, ptyVal = 0;
	std::vector<long> ptxSet;
	std::vector<long> ptySet;
	for(long i=0; i < PtCnt; i++)
	{
		ptxVal = Ptx[i];
		ptxSet.push_back(ptxVal);
		ptyVal = Pty[i];
		ptySet.push_back(ptyVal);
	}
	long ret = ImageWaveformBuilder::getInstance(_boardID)->VerifyPolyLine(ptxSet, ptySet, fieldSize, field2Volts, fieldScaleFineX,fieldScaleFineY, PixelY, PixelX);
	ptxSet.clear();
	ptySet.clear();
	return ret;
}

DllExport_IMGWFBUILDER long BuildPolyLine()
{
	return ImageWaveformBuilder::getInstance(_boardID)->BuildPolyLine();
}

DllExport_IMGWFBUILDER long BuildSpiral(long count)
{
	return ImageWaveformBuilder::getInstance(_boardID)->BuildSpiral(count);
}

DllExport_IMGWFBUILDER long BuildImageWaveform(double* startXY, long* countPerCallback, uint64_t* total, wstring outPath)
{
	return ImageWaveformBuilder::getInstance(_boardID)->BuildImageWaveform(startXY, countPerCallback, total, outPath);
}

DllExport_IMGWFBUILDER long BuildImageWaveformFromStart(long rebuild, double stepSize, double* currentVxy, long* countPerCallback, uint64_t* total)
{
	return ImageWaveformBuilder::getInstance(_boardID)->BuildImageWaveformFromStart(rebuild, stepSize, currentVxy, countPerCallback, total);
}

DllExport_IMGWFBUILDER void BufferAvailableCallbackFunc(long sType, long bufSpaceNum)
{
	return ImageWaveformBuilder::getInstance(_boardID)->BufferAvailableCallbackFunc(sType, bufSpaceNum);
}

DllExport_IMGWFBUILDER void ConnectBufferCallback(SignalType sType, BlockRingBuffer* brBuf)
{
	return ImageWaveformBuilder::getInstance(_boardID)->ConnectBufferCallback(sType, brBuf);
}

DllExport_IMGWFBUILDER uint64_t RebuildWaveformFromFile(const wchar_t* waveformFileName, double * currentVxy, int digLineSelection, long* countPerCallback)
{
	return ImageWaveformBuilder::getInstance(_boardID)->RebuildWaveformFromFile(waveformFileName, currentVxy, digLineSelection, countPerCallback);
}

DllExport_IMGWFBUILDER uint64_t RebuildThorDAQWaveformFromFile(const wchar_t* waveformFileName, unsigned short* currentVxy, int digLineSelection, long* countPerCallback)
{
	return ImageWaveformBuilder::getInstance(_boardID)->RebuildThorDAQWaveformFromFile(waveformFileName, currentVxy, digLineSelection, countPerCallback);
}

DllExport_IMGWFBUILDER void CloseWaveformFile()
{
	return ImageWaveformBuilder::getInstance(_boardID)->CloseWaveformFile();
}

DllExport_IMGWFBUILDER void ResetCounter()
{
	return ImageWaveformBuilder::getInstance(_boardID)->ResetCounter();
}

DllExport_IMGWFBUILDER uint64_t GetCounter(SignalType sType)
{
	return ImageWaveformBuilder::getInstance(_boardID)->GetCounter(sType);
}

// *** WaveformMemory Class	*** ///

DllExport_IMGWFBUILDER long AllocateMem(GGalvoWaveformParams gWParams, const wchar_t* memMapPath, long lpSecurityAttributes)
{
	return WaveformMemory::getInstance()->AllocateMem(gWParams, memMapPath, lpSecurityAttributes);
}

DllExport_IMGWFBUILDER long OpenMem(GGalvoWaveformParams& gWParams, const wchar_t* memMapPathName)
{
	return WaveformMemory::getInstance()->OpenMem(gWParams, memMapPathName);
}

DllExport_IMGWFBUILDER void CloseMem()
{
	WaveformMemory::getInstance()->CloseMem();
}

DllExport_IMGWFBUILDER char * GetMemMapPtr(SignalType stype, uint64_t offset, uint64_t size)
{
	return WaveformMemory::getInstance()->GetMemMapPtr(stype, offset, size);
}

DllExport_IMGWFBUILDER void UnlockMemMapPtr()
{
	WaveformMemory::getInstance()->UnlockMemMapPtr();
}

DllExport_IMGWFBUILDER long SetTempFileName(const wchar_t* tFileName)
{
	return WaveformMemory::getInstance()->SetTempFileName(tFileName);
}

DllExport_IMGWFBUILDER long SaveWaveformDataStruct(const wchar_t* tPathName, GGalvoWaveformParams waveformParams)
{
	return WaveformMemory::getInstance()->SaveWaveformDataStruct(tPathName, waveformParams);
}

DllExport_IMGWFBUILDER long SaveThorDAQWaveformDataStruct(const wchar_t* tPathName, ThorDAQGGWaveformParams waveformParams)
{
	return WaveformMemory::getInstance()->SaveThorDAQWaveformDataStruct(tPathName, waveformParams);
}

// ******************************************************** ///
// *************** WaveformBuilder Class ****************** ///
// Prepare for future extension of waveform builder types.  ///
// ******************************************************** ///

BuilderType type = BuilderType::EPHYS_DO;

DllExport_IMGWFBUILDER void SetBuilderType (BuilderType bType)
{
	type = bType;
}

DllExport_IMGWFBUILDER void ConnectCallback (void* buf)
{
	return WaveformBuilderFactory::GetBuilderInstance(type)->ConnectCallback(buf);
}

DllExport_IMGWFBUILDER void SetWaveformParams (void* param)
{
	return WaveformBuilderFactory::GetBuilderInstance(type)->SetWaveformParams(param);
}

DllExport_IMGWFBUILDER long TryBuildWaveform (uint64_t& totalLength)
{
	return WaveformBuilderFactory::GetBuilderInstance(type)->TryBuildWaveform(totalLength);
}

DllExport_IMGWFBUILDER HANDLE GetSignalHandle ()
{
	return WaveformBuilderFactory::GetBuilderInstance(type)->GetSignalHandle();
}

// ******************************************************** ///
// **************** WaveformSaver Class ******************* ///
// ******************************************************** ///

DllExport_IMGWFBUILDER long SaveData(wstring outPath, SignalType stype, void* gparam, unsigned long long length)
{
	return WaveformSaver::getInstance()->SaveData(outPath, stype, gparam, length);
}
