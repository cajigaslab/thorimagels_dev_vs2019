#pragma once


class SampleDllConcrete : public ISampleDll
{
public:

	static SampleDllConcrete* getInstance();

	long CreatePlateMosaicSample(double sampleOffsetX, double sampleOffsetY, double sampleOffsetZ, vector<IExperiment::SubImage>& subImages);
	long CreatePlateSample(double sampleOffsetX, double sampleOffsetY, long wellRows, long wellCols, double wellOffsetX, double wellOffsetY);
	long GoToWellSiteAndOffset(long row, long col, long subRow, long subColumn, double sampleOffsetX, double sampleOffsetY, double wellOffsetX, double wellOffsetY, double transOffsetX, double transOffsetY, double subOffsetX, double subOffsetY, sampleFPPrototype dm);
	long GoToAllWellSites(IDevice* xyStage, IAcquire* acquire, IExperiment* pExp);

private:

	SampleDllConcrete();
	static bool instanceFlag;
	static bool setupFlag;
	static auto_ptr<SampleDllConcrete> _single;
};