#pragma once

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the IMAGESTORELIBRARY_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// IMAGESTORELIBRARY_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef IMAGESTORELIBRARY_EXPORTS
#define IMAGESTORELIBRARY_API extern "C" __declspec(dllexport)
#else
#define IMAGESTORELIBRARY_API extern "C" __declspec(dllimport)
#endif
#include <stdint.h>
#include ".\include\ipltypes.h"

/*
* tag definitions.
*/
#define		IMAGESTORETAG_COMPRESSION			259	/* data compression technique */
#define	    IMAGESTORE_COMPRESSION_NONE			1	/* dump mode */
#define	    IMAGESTORE_COMPRESSION_LZW			5	/* Lempel-Ziv  & Welch */

/*
*	frame info
*/
struct frame_info
{
	uint32_t scan_id;
	uint16_t region_id;
	uint16_t channel_id;
	uint32_t z_id;
	uint32_t time_id;
	uint32_t s_id;
};

struct frame_info_ext
{
	frame_info_ext(frame_info frameInfo)
	{
		frame = frameInfo;
		scaleLevel = 0;
	}

	frame_info frame;
	uint16_t scaleLevel; // frame size to 2^-1 * scaleLevel;
};


/*
* image type definitions
*/
enum ImageType {
	ImageType_GRAY = 0,
	ImageType_ARGB = 1,
};

/*
* open mode definitions
*/
enum OpenMode {
	CREATE_MODE = 1,
	READ_WRITE_MODE = 2,
	READ_ONLY_MODE = 3,
};

/*
* result info definitions
*/
enum ResultInfo
{
	SUCCESS = 1,
	HANDLE_IS_NOT_EXIST = -1,
	FAILED_FILE_NAME = -2,
	PLATE_HAS_SET = -3,
	BUFFER_SIZE_IS_NOT_CORRECT = -4,
	PLATE_INFO_IS_NOT_EXIT = -5,
	INPUT_PTR_IS_NULL = -6,
	SCAN_ID_IS_NOT_EXIST = -7,
	FRAMEINFO_IS_NOT_CORRECT = -8,
	TILE_ROW_IS_NOT_CORRECT = -9,
	TILE_COLUMN_IS_NOT_CORRECT = -10,
	SCALE_IS_OUTOF_RANGE = -11,
	ROI_IS_OUTOF_RANGE = -12,
	IMAGE_IS_NOT_FULL_FILLED = -13,
	ADDITIONAL_DATA_IS_NOT_EXIST = -14,
	FILE_HAS_BEEN_OPENED = -15,
	UNKNOWN_ERROR = -100
};

/// <summary>
///  open or create imaging file.
/// </summary>
/// <param name="file_name">TIFF file name to be opened.</param>
/// <param name="mode"></param>
/// <returns> 1: success; -2: failed file name; -15: file has been opened; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_open_file(char* file_name, OpenMode mode, bool is_create_pyramidal_data = false);

/// <summary>
///  close imaging file.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <returns> 1: success; -1: handle is not exist; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_close_file(int handle);

/// <summary>
///  clean whole data.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <returns> 1: success; -1: handle is not exist; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_clean_data(int handle);

/// <summary>
///  set plates information.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="plates_info">plates information.</param>
/// <param name="size">size of plates information.</param>
/// <returns> 1: success; -1: handle is not exist; -3: plate has been set; -6: input ptr is null; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_set_plates_info(int handle, void * plates_info, uint32_t size);

/// <summary>
///  get plates information xml string size.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="size">plates information size.</param>
/// <returns> 1: success; -1: handle is not exist; -5: plate info is not exist; -6: input ptr is null; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_get_plates_info_size(int handle, uint32_t * size);

/// <summary>
///  get plates information xml string.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="plate_info">plates information xml string.</param>
/// <returns> 1: success; -1: handle is not exist; -4: buffer size is not correct; -5: plate info is not exist; -6: input ptr is null; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_get_plates_info(int handle, void * plate_info, uint32_t size);

/// <summary>
///  add new scan information.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="scan_info">scan information.</param>
/// <param name="size">size of scan information.</param>
/// <returns> 1: success; -1: handle is not exist; -6: input ptr is null; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_add_scan_info(int handle, void * scan_info, uint32_t size);

/// <summary>
///  get scans information xml string size.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="size">size of scans information.</param>
/// <returns> 1: success; -1: handle is not exist; -6: input ptr is null; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_get_scans_info_size(int handle, uint32_t * size);

/// <summary>
///  get scans information xml string.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="scan_infos">scans information.</param>
/// <param name="size">size of scans information.</param>
/// <returns> 1: success; -1: handle is not exist; -4: buffer size is not correct; -6: input ptr is null; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_get_scans_info(int handle, void * scan_infos, uint32_t size);

/// <summary>
///  remove scan information and data.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="scan_id">scan id to be removed.</param>
/// <returns> 1: success; -1: handle is not exist; -7: scan id is not exist; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_remove_scan(int handle, uint32_t scan_id);

/// <summary>
///  Change t count value of scan information.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="t_count">t count value.</param>
/// <returns> 1: success; -1: handle is not exist; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_adjust_scan_tcount(int handle, uint32_t t_count);

/// <summary>
///  save captured tile image of special scan region.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="image_data">buffer of image data.</param>
/// <param name="stride">the width of a single row of pixels.</param>
/// <param name="frame">frame information.</param>
/// <param name="tile_row">row of tile.</param>
/// <param name="tile_column">column of tile.</param>
/// <returns> 1: success; -1: handle is not exist; -6: input ptr is null; -8: frame info is not correct; -9: tile row is not correct; -10: tile column is not correct; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_save_tile_data(int handle, void* image_data, uint32_t stride, frame_info frame, uint32_t tile_row, uint32_t tile_column);

/// <summary>
///  get scan region roi image raw data.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="frame">frame information.</param>
/// <param name="src_rect">rectangle of source image.</param>
/// <param name="image_data">buffer of image data.</param>
/// <returns> 1: success; -1: handle is not exist; -6: input ptr is null; -8: frame info is not correct; -12: source rectangle is out of range; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_get_raw_data(int handle, frame_info frame, IplRect src_rect, void * image_data);

/// <summary>
///  get scaled scan region roi image data, the pixel type of image data is always unsigned char.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="frame">frame information.</param>
/// <param name="dst_size">size of destination image.</param>
/// <param name="src_rect">rectangle of source image.</param>
/// <param name="image_data">buffer of image data..</param>
/// <returns> 1: success; -1: handle is not exist; -6: input ptr is null; -8: frame info is not correct; -11: scale is out of range; -12: source rectangle is out of range; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_get_scaled_data(int handle, frame_info frame, IplSize dst_size, IplRect src_rect, void * image_data);


IMAGESTORELIBRARY_API long fnAISS_get_scaled_tileData(int handle, frame_info frame, uint16_t scaleLevel, uint16_t row, uint16_t column, void * image_data);

/// <summary>
///  auto generate the pyramidal(low level) image data, that according to scan region image size.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="scan_id">scan id to generate.</param>
/// <returns> 1: success; -1: handle is not exist; -7: scan id is not exist; -13: image is not full filled; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_generate_pyramidal_data(int handle, uint8_t scan_id);

/// <summary>
///  auto generate the pyramidal(low level) image data, that according to scan region image size.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="frame">frame information.</param>
/// <returns> 1: success; -1: handle is not exist; -7: scan id is not exist; -13: image is not full filled; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_generate_pyramidal_data_frame(int handle, frame_info frame, uint8_t* image, uint32_t image_width, uint32_t image_height);

/// <summary>
///  save additional data with name.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="additional_data">buffer of additional data.</param>
/// <param name="size">size of additional data.</param>
/// <param name="name">name of additional data.</param>
/// <returns> 1: success; -1: handle is not exist; -6: input ptr is null; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_save_additional_data(int handle, void *additional_data, uint32_t size, char* name);

/// <summary>
///  get additional data size.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="size">size of additional data.</param>
/// <param name="name">name of additional data.</param>
/// <returns> 1: success; -1: handle is not exist; -6: input ptr is null; -14: additional data is not exist; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_get_additional_data_size(int handle, char* name, uint32_t * size);

/// <summary>
///  get additional data.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="name">name of additional data.</param>
/// <param name="additional_data">buffer of additional data.</param>
/// <param name="size">size of additional data.</param>
/// <returns> 1: success; -1: handle is not exist; -4: buffer size is not correct; -6: input ptr is null; -14: additional data is not exist; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_get_additional_data(int handle, char* name, void *additional_data, uint32_t size);

/// <summary>
///  delete additional data.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="name">name of additional data.</param>
/// <returns> 1: success; -1: handle is not exist; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_delete_additional_data(int handle, char* name);

/// <summary>
///  set field to imaging file.
/// </summary>
/// <param name="handle">handle of imaging file.</param>
/// <param name="tag">field tag.</param>
/// <param name="v">field value.</param>
/// <returns> 1: success; -1: handle is not exist; -100: unknown error. </returns>
IMAGESTORELIBRARY_API long fnAISS_set_field(int handle, uint32_t tag, uint32_t v);