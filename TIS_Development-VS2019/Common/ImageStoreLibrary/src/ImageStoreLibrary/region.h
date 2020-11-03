#pragma once
#include <cstdint>
#include "image_container.h"
class Scan;
class region
{
public:
	region();
	~region();

	uint8_t ScanID;
	uint16_t RegionID;
	uint16_t PlateID;
	uint16_t WellID;
	uint16_t WellSampleID;

	uint32_t SizeX;
	uint32_t SizeY;
	uint32_t SizeZ;
	uint32_t SizeT;
	uint32_t SizeS;
	uint16_t MaxScaleLevel;

	vector<IplSize> ThumbImageSize;

	long GeneratePyramidalData(frame_info frame);
	long GeneratePyramidalData();
	long SaveTileData(void * image_data, uint32_t stride, frame_info frame, unsigned int tile_row, unsigned int tile_column);
	long LoadRawRectData(frame_info frame, IplRect src_rect, void* buffer);
	long LoadPyramidalRectData(frame_info frame, IplSize dst_size, IplRect src_rect, void* buffer);
	long LoadPyramidalRectData(frame_info frame, uint16_t scaleLevel, uint16_t row, uint16_t column, void* buffer);

	///<summary>
	/// Initialize ThumbImageSize and default MaxScaleLevel.
	///</summary>
	void Initialize();
	Scan* _parentScan;
	long Remove();
	map<uint16_t, image_container*> RawContainers;
	map<uint16_t, image_container*> ThumbnailContainers;
	static bool GetPyramidalLength(uint32_t actualLength, double pyramidalScale, uint32_t* pyramidalLength, uint32_t* subPyamidalLength, uint32_t* subPyamidalCount);
	image_container* findImageContainer(frame_info_ext frame, bool isThumbnail);
	long CreateContainers(bool is_create_pyramidal_data);
private:
	image_container * addContainer(bool isThumbnail);
};
