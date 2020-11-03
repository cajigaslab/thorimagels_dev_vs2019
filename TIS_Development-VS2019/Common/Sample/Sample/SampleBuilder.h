#pragma once

#include "Sample.h"

class SampleBuilder
{
public:
	SampleBuilder();

	Sample *CreatePlateMosaicSample(double sampleOffsetX, double sampleOffsetY, double sampleOffsetZ, vector<IExperiment::SubImage>& subImages);
	Sample *CreatePlateSample(double sampleOffsetX, double sampleOffsetY, long wellRows, long wellCols, double wellOffsetX, double wellOffsetY);
	Sample *CreateSample(double sampleOffsetX, double sampleOffsetY, PositionsIterator positions);
	Sample *CreateSampleFromFile(FILE *file);
};