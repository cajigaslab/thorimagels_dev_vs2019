#pragma once

#include "ImageStoreLibrary.h"
#include "xml_handler.h"
class TiffData
{
public:
	TiffData();
	~TiffData();
	long Init(char* file_name, OpenMode openMode, bool is_create_pyramidal_data);
	long SaveTileData(void * image_data, uint32_t stride, frame_info frame, unsigned int tile_row, unsigned int tile_column);
	long LoadRawData(frame_info frame, IplRect src_rect, void* buffer);
	long LoadScaledData(frame_info frame, IplSize dst_size, IplRect src_rect, void* buffer);
	long LoadScaledData(frame_info frame, uint16_t scaleLevel, uint16_t row, uint16_t column, void* buffer);

	long GeneratePyramidalData(uint8_t scan_id);
	long GeneratePyramidalData(frame_info frame, uint8_t* image, uint32_t image_width, uint32_t image_height);
	long SaveAdditionalData(void* additional_data, uint32_t size, char* name);
	long GetAdditionalDataSize(char* name, uint32_t* size);
	long GetAdditionalData(char* name, void* additional_data, uint32_t size);
	long DeleteAdditionalData(char* name);
	long CleanData();
	long SetPlateInfo(void* plate_info, uint32_t size);
	long GetPlateInfoSize(uint32_t* size);
	long GetPlateInfo(void* plate_info, uint32_t size);
	long AddScanInfo(void* scan_info, uint32_t size);
	long GetScanInfosSize(uint32_t* size);
	long GetScanInfos(void* scan_infos, uint32_t size);
	long RemoveScan(uint32_t scan_id);
	long SetField(uint32_t tag, uint32_t v);
	char tiff_file_extension[STRING_BUFFER_SIZE];
	char tiff_file_name[STRING_BUFFER_SIZE];
	char tiff_file_dir[STRING_BUFFER_SIZE];
	char tiff_file_full_name[STRING_BUFFER_SIZE];
	char tiff_tmp_file[STRING_BUFFER_SIZE];
	char zip_file_full_name[STRING_BUFFER_SIZE];
	uint32_t tag_compression;
	long rewriteOMEHeader();

	TIFF * tiff;
	map<uint8_t, Scan*> scans;
	map<uint16_t, Plate*> plates;
	map<uint8_t, Scan*>::iterator removeScan(map<uint8_t, Scan*>::iterator scanit);
	bool _is_file_using;
	bool _is_create_pyramidal_data;
};
