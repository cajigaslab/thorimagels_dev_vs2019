#pragma once
#include "storeconf.h"
#include <map>
#include <mutex>
#include "region.h"

using namespace std;

class TiffData;


struct interect_info
{
	uint16_t tile_index;
	uint32_t tile_start;
	uint32_t tile_length;
	uint32_t image_start;
};
class Channel
{
public:
	uint16_t ChannelID;		//0-based id, no gap allowed, 0:1:channelCount
	uint16_t ChannelRefID;	//reference index, used for LSM channels, 0:A, 1:B, 2:C, 3:D
	char Name[STRING_BUFFER_SIZE];
private:

};

typedef struct
{
	ttile_t tileNum;
	int ifd;
} TileKey;





class Scan
{
public:
	Scan(TiffData* parent = NULL);
	~Scan();

	uint8_t ScanID;
	uint16_t PlateID;
	double PhysicalSizeX;
	double PhysicalSizeY;
	double PhysicalSizeZ;
	ResUnit PhysicalSizeXUnit;
	ResUnit PhysicalSizeYUnit;
	ResUnit PhysicalSizeZUnit;
	uint16_t TimeIncrement;
	char TimeIncrementUnit[STRING_BUFFER_SIZE];
	char DimensionOrder[STRING_BUFFER_SIZE];
	uint16_t TileWidth;
	uint16_t TileHeight;
	PixelType Type;
	uint16_t SignificantBits;
	map<uint16_t, region*> Regions;
	map<uint16_t, Channel*> Channels;
	TiffData* parent_tiff;


private:
	char file_name[STRING_BUFFER_SIZE];
	
};
