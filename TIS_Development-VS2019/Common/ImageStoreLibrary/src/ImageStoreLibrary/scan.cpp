#include "scan.h"
#include "TiffData.h"


Scan::Scan(TiffData* parent)
{
	parent_tiff = parent;
	ScanID = 0;
	PlateID = 0;
	PhysicalSizeX = 0;
	PhysicalSizeY = 0;
	PhysicalSizeZ = 0;
	PhysicalSizeXUnit = ResUnit::None;
	PhysicalSizeYUnit = ResUnit::None;
	PhysicalSizeZUnit = ResUnit::None;
	TimeIncrement = 0;
	TileWidth = 0;
	TileHeight = 0;
	Type = PixelType::PixelType_UINT16;
	SignificantBits = 0;
}

Scan::~Scan()
{
	map<uint16_t, region*>::iterator regionit = Regions.begin();
	while (regionit != Regions.end())
	{

		delete regionit->second;
		regionit->second = NULL;
		Regions.erase(regionit++);
	}

}

