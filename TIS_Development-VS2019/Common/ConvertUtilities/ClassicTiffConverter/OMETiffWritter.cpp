#include "ome_tiff_library.h"
#include <stdexcept>
#include <condition_variable>
#include <chrono>
#include <cassert>
#include <fstream> 
#include <fcntl.h>
#include <stdio.h>
#include "common.h"
#include "OMETiffWritter.h"
#include "ClassicTiffConverter.h"
using namespace std;
using namespace ome;

bool OMETiffWritter::_instanceFlag = false;
OMETiffWritter* OMETiffWritter::_single;

#define TILEWIDTH 512
#define TILEHEIGHT 512

OMETiffWritter::OMETiffWritter()
{
}

OMETiffWritter::~OMETiffWritter()
{
	_instanceFlag = false;
}

OMETiffWritter* OMETiffWritter::getInstance()
{
	if (!_instanceFlag)
	{
		_single = new OMETiffWritter();
		_instanceFlag = true;
	}
	return _single;
}


long OMETiffWritter::Create(char* file_name)
{
	size_t len = strlen(file_name) + 1;
	size_t converted = 0;
	wchar_t* WStr = (wchar_t*)malloc(len * sizeof(wchar_t));
	mbstowcs_s(&converted, WStr, len, file_name, _TRUNCATE);

	long ret = ome_open_file(WStr,  OpenMode::CREATE_MODE);
	return ret;
}

long OMETiffWritter::ConfigOMEHeader(long handle, SampleInfo sample, int regionPixelX, int regionPixelY, float regionW, float regionH, int zCount, int tCount,
	int regionPositionPixelX, int regionPositionPixelY, int bitsPerPixel, float regionPixelSizeUM, double zStepSizeUM, double intervalSec, int channelNumber, const char* channels)
{
	// Add plates
	Plates plates;
	Plate* plate = plates.add_plate();
	plate->id = 1;
	plate->set_name(sample.name);
	plate->width = sample.width; // mm
	plate->height = sample.height;
	plate->physicalsize_unit_x = DistanceUnit::DISTANCE_MILLIMETER;
	plate->physicalsize_unit_y = DistanceUnit::DISTANCE_MILLIMETER;
	if (strncmp("Slide", sample.type, 5) == 0) { // Slide
		plate->row_size = 1;
		plate->column_size = 1;
	}
	else {
		plate->row_size = sample.rowSize;
		plate->column_size = sample.columnSize;
	}

	// Add wells
	if (strncmp("Slide", sample.type, 5) == 0) { // Slide
		Well* well = plate->add_well();
		well->id = 1;
		well->well_shape = Shape::SHAPE_RECTANGLE;
		well->width = regionPixelX * regionPixelSizeUM / 1000; // mm
		well->height = regionPixelY * regionPixelSizeUM / 1000;

		well->position_x = sample.topLeftCenterOffsetX;
		well->position_y = sample.topLeftCenterOffsetY;
		well->row_index = 0;
		well->column_index = 0;
	}
	else
	{
		for (int i = 0; i < plate->row_size; i++)
			for (int j = 0; j < plate->column_size; j++) {
				Well* well = plate->add_well();
				well->id = j + 1 + i * plate->column_size;
				well->well_shape = (Shape)sample.wellShape;
				if (well->well_shape == Shape::SHAPE_ELLIPSE) {
					well->width = sample.diameter;
					well->height = sample.diameter;
				}
				else if (well->well_shape == Shape::SHAPE_RECTANGLE) {
					well->width = sample.wellWidth;
					well->height = sample.wellHeight;
				}
				well->position_x = sample.topLeftCenterOffsetX - well->width / 2 + j * sample.centerToCenterX;
				well->position_y = sample.topLeftCenterOffsetY - well->height / 2 + i * sample.centerToCenterY;
				well->row_index = i;
				well->column_index = j;
			}
	}
	long status = ome_set_plates(handle, &plates);

	if (status != STATUS_OK)
		return status;

	// Add scan, only 1 scan needed
	Scan scan;
	scan.id = 1;
	scan.plate_id = 1;
	scan.pixel_physical_size_x = (float)regionPixelSizeUM;
	scan.pixel_physical_size_y = (float)regionPixelSizeUM;
	scan.pixel_physical_size_z = (float)zStepSizeUM;
	scan.pixel_physical_uint_x = DistanceUnit::DISTANCE_MICROMETER;
	scan.pixel_physical_uint_y = DistanceUnit::DISTANCE_MICROMETER;
	scan.pixel_physical_uint_z = DistanceUnit::DISTANCE_MICROMETER;
	scan.time_increment = (unsigned int)intervalSec;
	scan.time_increment_unit = TimeUnit::TIME_SECOND;
	scan.set_dimension_order("XYCZT");
	scan.tile_pixel_size_width = regionPixelX; // TILEWIDTH; 
	scan.tile_pixel_size_height = regionPixelY;// ILEHEIGHT;

	scan.pixel_type = PixelType::PIXEL_UINT16; // to do: need to judge by bitsPerPixel
	scan.significant_bits = bitsPerPixel;

	// Add channels
	string channelStr = channels;
	string delim = ",";
	vector<string>* channelVector = new vector<string>[channelNumber];
	Split(channelStr, delim, channelVector);

	for (int i = 0; i < channelNumber; i++) {
		Channel* channelA = scan.add_channel();
		channelA->id = i;
		channelA->set_name(channelVector->at(i).c_str());
	}

	// Add well samples
	if (strncmp("Slide", sample.type, 5) == 0) { // Slide
		ScanRegion* region = scan.add_region();
		region->id = 0;
		region->well_id = 1;
		region->size_pixel_x = regionPixelX;
		region->size_pixel_y = regionPixelY;
		region->size_pixel_z = zCount;
		region->size_time = tCount;
		region->size_stream = 1;

		WellSample* ws = &region->well_sample;
		Well* well = plate->get_well(region->well_id);
		ws->position_x = well->position_x * 1000;
		ws->position_y = well->position_y * 1000;
		ws->position_z = 0;
		ws->physicalsize_unit_x = DistanceUnit::DISTANCE_MICROMETER; //um
		ws->physicalsize_unit_y = DistanceUnit::DISTANCE_MICROMETER;
		ws->physicalsize_unit_z = DistanceUnit::DISTANCE_MICROMETER;
	}
	else
	{
		float wsOffsetX = (regionPixelX / 2 + regionPositionPixelX) * regionPixelSizeUM;
		float wsOffsetY = (regionPixelY / 2 + regionPositionPixelY) * regionPixelSizeUM;
		for (int i = 0; i < plate->row_size; i++)
			for (int j = 0; j < plate->column_size; j++) {
				ScanRegion* region = scan.add_region();
				region->id = j + i * plate->column_size;
				region->well_id = j + 1 + i * plate->column_size;
				region->size_pixel_x = regionPixelX;
				region->size_pixel_y = regionPixelY;
				region->size_pixel_z = zCount;
				region->size_time = tCount;
				region->size_stream = 1;

				WellSample* ws = &region->well_sample;
				Well* well = plate->get_well(region->well_id);
				ws->position_x = (well->position_x + well->width / 2) * 1000 - wsOffsetX; //pixel , corresponding to well phy position
				ws->position_y = (well->position_y + well->height / 2) * 1000 - wsOffsetY;
				ws->position_z = 0;
				ws->physicalsize_unit_x = DistanceUnit::DISTANCE_MICROMETER; // um
				ws->physicalsize_unit_y = DistanceUnit::DISTANCE_MICROMETER;
				ws->physicalsize_unit_z = DistanceUnit::DISTANCE_MICROMETER;
			}
	}
	return ome_add_scan(handle, &scan);
}

long OMETiffWritter::SaveOMEData(long handle, int channelID, unsigned short zIndex, unsigned short tIndex, unsigned short* data)
{
	Plates* plates = new Plates();
	long status = ome_get_plates(handle, plates);
	if (status != STATUS_OK)
		return status;

	Plate* plate = plates->get_plate(1);
	if (plate == NULL)
		return -1;

	unsigned short regionID;
	for (int i = 0; i < plate->row_size; i++)
		for (int j = 0; j < plate->column_size; j++) {
			regionID = j + i * plate->column_size;
			ome::FrameInfo frameInfo = { 1, regionID, (unsigned char)channelID, zIndex, tIndex, 0 };
			status = ome_save_tile_data(handle, data, frameInfo, 0, 0);
		}

	return status;
}

long OMETiffWritter::SaveAdditionalData(long handle, const char* data, int size, const char* name)
{
	return ome_save_additional_data(handle, (void*)data, (unsigned int)size, (char*)name);
}

long OMETiffWritter::CloseFile(long handle)
{
	return ome_close_file(handle);
}