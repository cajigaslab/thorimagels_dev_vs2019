#include "storeconf.h"

class WellSample
{
public:
	uint16_t PlateID;
	uint16_t WellID;
	uint16_t WellSampleID;
	uint8_t ScanID;
	uint16_t ImageID;

	map<uint16_t, SampleRegion*> SampleRegions;
	WellSample():PlateID(1),WellID(1),WellSampleID(1),ScanID(1),ImageID(1){}
	~WellSample()
	{
		map<uint16_t, SampleRegion*>::iterator sampleRegionit = SampleRegions.begin();
		while (sampleRegionit != SampleRegions.end())
		{
			delete sampleRegionit->second;
			sampleRegionit->second = NULL;
			SampleRegions.erase(sampleRegionit++);
		}
	}
};
class Well
{
public:
	uint16_t PlateID;
	uint16_t WellID;
	double PositionX;
	double PositionY;
	double Width;
	double Height;
	int Row;
	int Column;
	char Shape[STRING_BUFFER_SIZE];
	map<uint16_t, WellSample*> WellSamples;
	Well()
	{

	}
	~Well()
	{
		map<uint16_t, WellSample*>::iterator wellSampleit = WellSamples.begin();
		while (wellSampleit != WellSamples.end())
		{
			delete wellSampleit->second;
			wellSampleit->second = NULL;
			WellSamples.erase(wellSampleit++);
		}
	}
};
class Plate
{
public:
	uint16_t PlateID;
	char Name[STRING_BUFFER_SIZE];
	double Width;
	double Height;
	ResUnit PhysicalSizeXUnit;
	ResUnit PhysicalSizeYUnit;
	int Rows;
	int Columns;
	map<uint16_t, Well*> Wells;
	Plate()
	{

	}
	~Plate()
	{
		map<uint16_t, Well*>::iterator wellit = Wells.begin();
		while (wellit != Wells.end())
		{
			delete wellit->second;
			wellit->second = NULL;
			Wells.erase(wellit++);
		}
	}
};
