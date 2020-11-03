// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "SampleDll.h"
#include "SampleDllConcrete.h"
#include "SampleBuilder.h"

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

auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
auto_ptr<Sample> activeSample;
void (*sampleFunctionPointer)(double &x, double &y) = NULL;
wchar_t message[256];

#define DllExport_Sample extern "C" long __declspec(dllexport)

////exported directly to allow C# calls
//DllExport_Sample CreatePlateMosaicSample(long startRow, long startCol, long totalRows, long totalCols, double sampleOffsetX, double sampleOffsetY, long wellRows, long wellCols, double wellOffsetX, double wellOffsetY, long subRows, long subCols, double subOffsetX, double subOffsetY, double transOffsetX, double transOffsetY)
//{	
//	return SampleDllConcrete::getInstance()->CreatePlateMosaicSample(startRow, startCol, totalRows, totalCols, sampleOffsetX,  sampleOffsetY,  wellRows,  wellCols,  wellOffsetX,  wellOffsetY, subRows, subCols, subOffsetX, subOffsetY, transOffsetX, transOffsetY);
//}
//exported directly to allow C# calls
DllExport_Sample CreatePlateMosaicSample(double sampleOffsetX, double sampleOffsetY, double sampleOffsetZ, vector<IExperiment::SubImage>& subImages)
{	
	return SampleDllConcrete::getInstance()->CreatePlateMosaicSample(sampleOffsetX, sampleOffsetY, sampleOffsetZ, subImages);
}
//exported directly to allow C# calls
DllExport_Sample CreatePlateSample(double sampleOffsetX, double sampleOffsetY, long wellRows, long wellCols, double wellOffsetX, double wellOffsetY)
{	
	return SampleDllConcrete::getInstance()->CreatePlateSample( sampleOffsetX,  sampleOffsetY,  wellRows,  wellCols,  wellOffsetX,  wellOffsetY);
}


//exported directly to allow C# calls
DllExport_Sample GoToWellSiteAndOffset(long row, long col, long subRow, long subColumn,double sampleOffsetX, double sampleOffsetY, double wellOffsetX, double wellOffsetY, double transOffsetX, double transOffsetY, double subOffsetX, double subOffsetY, sampleFPPrototype dm)
{	
	return SampleDllConcrete::getInstance()->GoToWellSiteAndOffset( row,  col, subRow, subColumn, sampleOffsetX, sampleOffsetY, wellOffsetX, wellOffsetY, transOffsetX, transOffsetY, subOffsetX, subOffsetY, dm);
}

DllExport_Sample GoToAllWellSites(IDevice* xyStage, IAcquire* acquire, IExperiment* pExp)
{
	return SampleDllConcrete::getInstance()->GoToAllWellSites(xyStage, acquire, pExp);
}


SampleDllConcrete::SampleDllConcrete()
{
	//private constructor
}

bool SampleDllConcrete::instanceFlag = false;

bool SampleDllConcrete::setupFlag = false;

auto_ptr<SampleDllConcrete> SampleDllConcrete::_single(new SampleDllConcrete());

SampleDllConcrete* SampleDllConcrete::getInstance()
{
	if(! instanceFlag)
	{
		wsprintf(message,L"Creating SampleDllConcrete Singleton");
		logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

		_single.reset(new SampleDllConcrete());
		instanceFlag = true;
		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

//long SampleDllConcrete::CreatePlateMosaicSample(long startRow, long startCol, long totalRows, long totalCols, double sampleOffsetX, double sampleOffsetY, long wellRows, long wellCols, double wellOffsetX, double wellOffsetY, 
//												long subRows, long subCols, double subOffsetX, double subOffsetY, double transOffsetX, double transOffsetY)
//{
//	SampleBuilder sb;
//	
//	auto_ptr<Sample> p1(sb.CreatePlateMosaicSample(startRow, startCol, totalRows, totalCols, sampleOffsetX, sampleOffsetY, wellRows, wellCols, wellOffsetX, wellOffsetY, 
//						subRows, subCols, subOffsetX, subOffsetY, transOffsetX, transOffsetY));
//
//	activeSample = p1;
//
//	return TRUE;
//}

long SampleDllConcrete::CreatePlateMosaicSample(double sampleOffsetX, double sampleOffsetY, double sampleOffsetZ, vector<IExperiment::SubImage>& subImages)
{
	SampleBuilder sb;
	
	auto_ptr<Sample> p1(sb.CreatePlateMosaicSample(sampleOffsetX, sampleOffsetY, sampleOffsetZ, subImages));

	activeSample = p1;

	return TRUE;
}


long SampleDllConcrete::CreatePlateSample(double sampleOffsetX, double sampleOffsetY, long wellRows, long wellCols, double wellOffsetX, double wellOffsetY)
{
	SampleBuilder sb;
	
	auto_ptr<Sample> p1(sb.CreatePlateSample(sampleOffsetX, sampleOffsetY, wellRows, wellCols, wellOffsetX, wellOffsetY));

	activeSample = p1;

	return TRUE;
}

long SampleDllConcrete::GoToWellSiteAndOffset(long row, long col, long subRow, long subColumn, double sampleOffsetX, double sampleOffsetY, double wellOffsetX, double wellOffsetY, double transOffsetX, double transOffsetY, double subOffsetX, double subOffsetY, sampleFPPrototype dm)
{
	if(activeSample.get() == NULL)
	{
		return FALSE;
	}

	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"Sample GoToWellSiteAndOffset");

	activeSample->GoToWellSiteAndOffset(row, col, subRow, subColumn, sampleOffsetX, sampleOffsetY, wellOffsetX, wellOffsetY, transOffsetX, transOffsetY, subOffsetX, subOffsetY ,dm);

	return TRUE;
}

long SampleDllConcrete::GoToAllWellSites(IDevice* xyStage, IAcquire* acquire, IExperiment* pExp)
{
	if(activeSample.get() == NULL)
	{
		return FALSE;
	}

	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"Sample GoToAllWellSites");

	activeSample->GoToAllWellSites(xyStage,acquire,pExp);

	return TRUE;
}