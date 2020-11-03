#pragma once
#include <cstdint>
#include <map>
#include "ImageStoreLibrary.h"
#include ".\include\tiffconf.h"
#include ".\include\tiffio.h"
#include ".\include\tiffvers.h"
#include <rpc.h>
#include "storeconf.h"
#include <mutex>


#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
class region;
struct interect_info;

class image_container
{
private:
	uint32_t _capacity;
	
	TIFF* tiff;
	region* _parentRegion;
	string file_name;

	uint16_t _imageWidth;
	uint16_t _imageHeight;

	uint16_t _tileWidth;
	uint16_t _tileHeight;
	PixelType _pxType;
	uint16_t SignificantBits;
	std::mutex _readmt;

	bool _isThumbnail;
	OpenMode _mode;

	PixelType Type;

	long OpenFile(OpenMode openMode);
	long CloseFile();
	static void TIFFErrorProcExt(thandle_t hdl, const char* pModule, const char* pFormat, va_list pArg);


public:
	uint32_t _id;
	image_container(uint32_t id, bool isThumbnail,region* parentRegion,OpenMode mode, const char* fileName=nullptr);
	~image_container();

	//Set up the IFD for original 16 bit images or Create the thumbnail
	long SetupFramesIFD(std::vector<frame_info_ext> frames);

	//Sve the origin image data by tile.
	long SaveTileData(void * image_data, uint32_t stride, frame_info_ext frame, unsigned int tile_row, unsigned int tile_column);

	interect_info* calculateInterectInfo(uint32 imageLength, uint32 tileLength, uint32 roiStart, uint32 roiLength, double scale, uint16* tileCount, uint32* dstImageLength);

	long Remove();
	long LoadRawRectData(frame_info frame, IplRect src_rect, void* buffer);
	long loadPyramidalRectData(frame_info_ext frame, IplSize dst_size, IplRect src_rect, void* buffer);
	long loadScaledRawRectData(frame_info frame, IplSize dst_size, IplRect src_rect, void* buffer);
	long LoadTileData(frame_info_ext frame, uint16_t row, uint16_t column, void* buffer);

	long LoadPyramidalRectData(frame_info_ext frame, uint16_t row, uint16_t column, void * buffer);

	bool ContainsFrame(frame_info_ext frame);

	bool CanCreateIFD();

	std::map<frame_info_ext, uint16_t> _frameIfds;

	string GetFileName();
};

