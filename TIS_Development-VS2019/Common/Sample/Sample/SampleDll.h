#pragma once

#include "..\..\PDLL\pdll.h"
#include "Sample.h"


class ISampleDll
{
public: 
	
	virtual long CreatePlateMosaicSample(double sampleOffsetX, double sampleOffsetY, double sampleOffsetZ, vector<IExperiment::SubImage>& subImages)=0;
	virtual long CreatePlateSample(double sampleOffsetX, double sampleOffsetY, long wellRows, long wellCols, double wellOffsetX, double wellOffsetY)=0;
	virtual long GoToWellSiteAndOffset(long row, long col, long subRow, long subColumn, double sampleOffsetX, double sampleOffsetY, double wellOffsetX, double wellOffsetY, double transOffsetX, double transOffsetY, double subOffsetX, double subOffsetY, sampleFPPrototype dm)=0;
	virtual long GoToAllWellSites(IDevice*, IAcquire*, IExperiment*)=0;
};

class SampleDll : public PDLL, public ISampleDll
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(SampleDll)
#pragma warning(pop)
	
	DECLARE_FUNCTION4(long, CreatePlateMosaicSample, double, double, double, vector<IExperiment::SubImage>&)
	DECLARE_FUNCTION6(long, CreatePlateSample, double, double, long, long, double, double)
	DECLARE_FUNCTION13(long, GoToWellSiteAndOffset,long, long, long, long, double, double, double, double, double, double, double, double, sampleFPPrototype)
	DECLARE_FUNCTION3(long, GoToAllWellSites, IDevice*, IAcquire*, IExperiment*)
};

