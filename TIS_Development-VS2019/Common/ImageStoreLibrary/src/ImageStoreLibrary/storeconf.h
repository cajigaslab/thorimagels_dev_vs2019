#pragma once
#include "ImageStoreLibrary.h"
#include "../../../Experiment.h"
#include <vector>
#include <map>
using namespace std;

#define STRING_BUFFER_SIZE 256
#define MinPyramidalLength 256
#define NUM_PYRAMIDAL 8

#define TIFF_GET_FIELD_OR_FAIL(tiff,tag,value)			\
	if(!TIFFGetField(tiff,tag,&value)){				\
		return UNKNOWN_ERROR;							\
	}

#define TIFF_SET_FIELD_OR_FAIL(tiff,tag,value)			\
	do{													\
		if (!TIFFSetField(tiff, tag, value)) {			\
			return UNKNOWN_ERROR;						\
		}												\
	}while(0)
static const double pyramidal_scale_list[NUM_PYRAMIDAL] = {1.0, 0.5,0.25,0.125,0.0625,0.03125,0.015625,0.0078125 };

static uint16_t thumbnailTileSize = MinPyramidalLength;

static const string PixelTypeString[]=
{
	"int8",
	"int16",
	"int32",
	"uint8",
	"uint16",
	"uint32",
	"float",
	"double",
	"complex",
	"double_complex",
	"bit"
};
static const uint32_t RegisterCODEC[] =
{
	IMAGESTORE_COMPRESSION_NONE,
	IMAGESTORE_COMPRESSION_LZW
};

inline bool operator < (const struct frame_info &_A, const struct frame_info &_B) {
	if (_A.scan_id < _B.scan_id) return true;
	else if (_A.scan_id > _B.scan_id) return false;
	if (_A.channel_id < _B.channel_id) return true;
	else if (_A.channel_id > _B.channel_id) return false;
	if (_A.region_id < _B.region_id) return true;
	else if (_A.region_id > _B.region_id) return false;
	if (_A.s_id < _B.s_id) return true;
	else if (_A.s_id > _B.s_id) return false;
	if (_A.time_id < _B.time_id) return true;
	else if (_A.time_id > _B.time_id) return false;
	if (_A.z_id < _B.z_id) return true;
	else if (_A.z_id > _B.z_id) return false;
	return false;
}

inline bool operator < (const struct frame_info_ext &_eA, const struct frame_info_ext &_eB) {
	if (_eA.scaleLevel < _eB.scaleLevel)return true;
	else if (_eA.scaleLevel > _eB.scaleLevel) return false;
	return _eA.frame < _eB.frame;
}

enum PixelType {
	//PixelType_INT8 = 0,
	PixelType_INT16 = 1,
	//PixelType_INT32 = 2,
	PixelType_UINT8 = 3,
	PixelType_UINT16 = 4,
	//PixelType_UINT32 = 5,
	//PixelType_FLOAT = 6,
	//PixelType_DOUBLE = 7,
	//PixelType_COMPLEX = 8,
	//PixelType_DOUBLE_COMPLEX = 9,
	//PixelType_BIT = 10
};
