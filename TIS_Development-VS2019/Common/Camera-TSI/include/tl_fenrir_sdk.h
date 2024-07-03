//#ifndef __THORLABS_SCIENTIFIC_IMAGING_TIFF_LIBRARY_H__
//#define __THORLABS_SCIENTIFIC_IMAGING_TIFF_LIBRARY_H__
#pragma once
#define F_EXPORT __declspec(dllexport)



//------------------------------------------------------------------------------
// <sys/types.h> or <stdint.h>
//------------------------------------------------------------------------------
#ifdef __ENABLE_STDINT__
typedef char               int8_t;
typedef unsigned char     uint8_t;
typedef short             int16_t;
typedef unsigned short   uint16_t;
typedef int               int32_t;
typedef unsigned int     uint32_t;
typedef __int64           int64_t;
typedef unsigned __int64 uint64_t;
typedef float              fp32_t;
typedef double             fp64_t;
#endif

#pragma pack(push, 1)
typedef struct 
{
   unsigned int numerator;
   unsigned int denominator;
} TIFF_RATIONAL;
#pragma pack(pop)

//------------------------------------------------------------------------------
// TL_FENRIR_ERROR
//------------------------------------------------------------------------------
enum TL_FENRIR_ERROR
{
    TL_FENRIR_ERROR_NO_ERROR = 0,
    TL_FENRIR_ERROR_BAD_FSTREAM = 1,
    TL_FENRIR_ERROR_NULL_HANDLE = 2,
    TL_FENRIR_ERROR_UNKNOWN_IMAGE_TYPE = 3,
    TL_FENRIR_ERROR_FILE_SIZE_LIMIT_REACHED = 4,
    TL_FENRIR_ERROR_STRIP_OFFSETS_INT_TAG_MISSING = 5,
    TL_FENRIR_ERROR_MEM_ALLOC_ERROR = 6,
    TL_FENRIR_ERROR_COULD_NOT_OPEN_FILE = 7,
    TL_FENRIR_ERROR_INVALID_HANDLE = 8,
    TL_FENRIR_ERROR_INVALID_ARGUMENT = 9,
    TL_FENRIR_ERROR_ALREADY_INITIALIZED = 10,
    TL_FENRIR_ERROR_UNABLE_TO_LOAD_LIBRARY = 11,
    TL_FENRIR_ERROR_FUNCTION_NOT_FOUND = 12,
    TL_FENRIR_ERROR_PIXEL_BUFFER_ELEMENT_SIZE_TOO_SMALL = 13,
    TL_FENRIR_ERROR_PIXEL_BUFFER_ELEMENT_SIZE_UNSUPPORTED = 14
};


//------------------------------------------------------------------------------
// TL_FENRIR_TAG_DATA_TYPE
//------------------------------------------------------------------------------
enum TL_FENRIR_TIFF_TAG_DATA_TYPE
{
   TL_FENRIR_TIFF_TAG_DATA_TYPE_BYTE       =  1,
   TL_FENRIR_TIFF_TAG_DATA_TYPE_ASCII      =  2,
   TL_FENRIR_TIFF_TAG_DATA_TYPE_SHORT      =  3,
   TL_FENRIR_TIFF_TAG_DATA_TYPE_LONG       =  4,
   TL_FENRIR_TIFF_TAG_DATA_TYPE_RATIONAL   =  5,
   TL_FENRIR_TIFF_TAG_DATA_TYPE_SBYTE      =  6,
   TL_FENRIR_TIFF_TAG_DATA_TYPE_UNDEFINED  =  7,
   TL_FENRIR_TIFF_TAG_DATA_TYPE_SSHORT     =  8,
   TL_FENRIR_TIFF_TAG_DATA_TYPE_SLONG      =  9,
   TL_FENRIR_TIFF_TAG_DATA_TYPE_SRATIONAL  = 10,
   TL_FENRIR_TIFF_TAG_DATA_TYPE_FLOAT      = 11,
   TL_FENRIR_TIFF_TAG_DATA_TYPE_DOUBLE     = 12,
   TL_FENRIR_TIFF_TAG_DATA_TYPE_MAX        = 13
};

//------------------------------------------------------------------------------
// TL_FENRIR_TIFF_TAG_IDS
//------------------------------------------------------------------------------
enum TL_FENRIR_TIFF_TAG_IDS
{
   //---------------------------------------------------------------------------
   // Standard TIFF tags 
   //---------------------------------------------------------------------------
   TL_FENRIR_TIFF_TAG_NEW_SUBFILE_TYPE_INT     = 254,
   TL_FENRIR_TIFF_TAG_SUBFILE_TYPE_INT         = 255,
   TL_FENRIR_TIFF_TAG_IMAGE_WIDTH_INT          = 256, 
   TL_FENRIR_TIFF_TAG_IMAGE_HEIGHT_INT         = 257,   // Referred to as 'LENGTH' in tiff spec.
   TL_FENRIR_TIFF_TAG_BITS_PER_SAMPLE_INT      = 258,
   TL_FENRIR_TIFF_TAG_COMPRESSION_INT          = 259,
   TL_FENRIR_TIFF_TAG_PHOTOMETRIC_INTERP_INT   = 262,   // See TL_FENRIR_PHOTOMETRIC_INTERP_VALUES for values.
   TL_FENRIR_TIFF_TAG_STRIP_OFFSETS_INT        = 273,
   TL_FENRIR_TIFF_TAG_SAMPLES_PER_PIXEL_INT    = 277,
   TL_FENRIR_TIFF_TAG_ROWS_PER_STRIP_INT       = 278,
   TL_FENRIR_TIFF_TAG_STRIP_BYTE_COUNTS_INT    = 279, 
   TL_FENRIR_TIFF_TAG_X_RESOLUTION             = 282,
   TL_FENRIR_TIFF_TAG_Y_RESOLUTION             = 283,
   TL_FENRIR_TIFF_TAG_RESOLUTION_UNIT          = 296,

   //---------------------------------------------------------------------------
   // TSI specific tags.
   //---------------------------------------------------------------------------
   TL_FENRIR_TIFF_TAG_TSI_START                     = 32768,
   TL_FENRIR_TIFF_TAG_TSI_BITS_PER_PIXEL_INT   = TL_FENRIR_TIFF_TAG_TSI_START +  0, // Same as TL_FENRIR_TIFF_TAG_DYNAMIC_RANGE_INT
   TL_FENRIR_TIFF_TAG_TSI_DYNAMIC_RANGE_INT    = TL_FENRIR_TIFF_TAG_TSI_START +  0, // Same as TL_FENRIR_TIFF_TAG_BITS_PER_PIXEL_INT
   TL_FENRIR_TIFF_TAG_TSI_IMAGE_VERSION_INT    = TL_FENRIR_TIFF_TAG_TSI_START +  1,
   TL_FENRIR_TIFF_TAG_TSI_BIN_X_INT            = TL_FENRIR_TIFF_TAG_TSI_START +  2,
   TL_FENRIR_TIFF_TAG_TSI_BIN_Y_INT            = TL_FENRIR_TIFF_TAG_TSI_START +  3, 
   TL_FENRIR_TIFF_TAG_TSI_ROI_ORIGIN_X_INT     = TL_FENRIR_TIFF_TAG_TSI_START +  4, 
   TL_FENRIR_TIFF_TAG_TSI_ROI_ORIGIN_Y_INT     = TL_FENRIR_TIFF_TAG_TSI_START +  5, 
   TL_FENRIR_TIFF_TAG_TSI_ROI_PIXELS_X_INT     = TL_FENRIR_TIFF_TAG_TSI_START +  6, 
   TL_FENRIR_TIFF_TAG_TSI_ROI_PIXELS_Y_INT     = TL_FENRIR_TIFF_TAG_TSI_START +  7, 
   TL_FENRIR_TIFF_TAG_TSI_EXPOSURE_TIME_US_INT = TL_FENRIR_TIFF_TAG_TSI_START +  8,
   TL_FENRIR_TIFF_TAG_TSI_PIXEL_CLOCK_HZ_INT   = TL_FENRIR_TIFF_TAG_TSI_START +  9, 
   TL_FENRIR_TIFF_TAG_TSI_NUM_TAPS_INT         = TL_FENRIR_TIFF_TAG_TSI_START + 10,
   TL_FENRIR_TIFF_TAG_TSI_FRAME_NUMBER_INT     = TL_FENRIR_TIFF_TAG_TSI_START + 11,

   //DELETE ME
   //TIFF_TAG_TSI_NUM_IMAGES           = 32770
};

//------------------------------------------------------------------------------
// TL_FENRIR_TIFF_PIXEL_TYPE
//------------------------------------------------------------------------------
enum TL_FENRIR_TIFF_PIXEL_TYPE
{
   TL_FENRIR_TIFF_GREYSCALE8      = 0x10000001,
   TL_FENRIR_TIFF_GREYSCALE12     = 0x10000002,
   TL_FENRIR_TIFF_GREYSCALE14     = 0x10000004,
   TL_FENRIR_TIFF_GREYSCALE16     = 0x10000008,
   TL_FENRIR_TIFF_GREYSCALE32     = 0x10000010,
   TL_FENRIR_TIFF_GREYSCALE64     = 0x10000020,
   TL_FENRIR_TIFF_RGB8            = 0x30000001,
   TL_FENRIR_TIFF_RGB16           = 0x30000002,
   TL_FENRIR_TIFF_RGB32           = 0x30000004,
   TL_FENRIR_TIFF_BGR8            = 0x40000001,
   TL_FENRIR_TIFF_BGR12           = 0x40000002,
   TL_FENRIR_TIFF_BGR14           = 0x40000003,
   TL_FENRIR_TIFF_BGR16           = 0x40000004,
   //TL_FENRIR_TIFF_MULTISPECTRAL8  = 0x80000001,
   //TL_FENRIR_TIFF_MULTISPECTRAL16 = 0x80000002,
};


//------------------------------------------------------------------------------
// TL_FENRIR_PHOTOMETRIC_INTERP_VALUES
//------------------------------------------------------------------------------
enum TL_FENRIR_PHOTOMETRIC_INTERP_VALUES
{
   TL_FENRIR_PHOTOMETRIC_MINISWHITE    = 0,       // 0 is white.
   TL_FENRIR_PHOTOMETRIC_MINISBLACK    = 1,       // 0 is black.
   TL_FENRIR_PHOTOMETRIC_RGB           = 2,       // Full color RGB.
   TL_FENRIR_PHOTOMETRIC_PALETTE       = 3,       // The pixels are indicies into a RGB lut.
   TL_FENRIR_PHOTOMETRIC_MASK          = 4,
   TL_FENRIR_PHOTOMETRIC_SEPARATED     = 5,
   TL_FENRIR_PHOTOMETRIC_YCBCR         = 6,
   TL_FENRIR_PHOTOMETRIC_CIELAB        = 8,
   TL_FENRIR_PHOTOMETRIC_ICCLAB        = 9,
   TL_FENRIR_PHOTOMETRIC_ITULAB        = 10,
   TL_FENRIR_PHOTOMETRIC_LOGL          = 32844,
   TL_FENRIR_PHOTOMETRIC_LOGLUV        = 32845,
   TL_FENRIR_PHOTOMETRIC_MULTISPECTRAL = 32768,    // TSI defined value for multispectral (channel) images.
};

//------------------------------------------------------------------------------
// TL_FENRIR_PIXEL_BUFFER_ELEMENT_SIZE
//------------------------------------------------------------------------------
enum TL_FENRIR_PIXEL_BUFFER_ELEMENT_SIZE
{
    TL_FENRIR_PIXEL_BUFFER_ELEMENT_SIZE_8BIT = 1,
    TL_FENRIR_PIXEL_BUFFER_ELEMENT_SIZE_16BIT = 2,
    TL_FENRIR_PIXEL_BUFFER_ELEMENT_SIZE_32BIT = 4,
    TL_FENRIR_PIXEL_BUFFER_ELEMENT_SIZE_64BIT = 8,
};


//------------------------------------------------------------------------------
// Typedefs for export.
//------------------------------------------------------------------------------
typedef int   (*TL_FENRIR_TIFF_OPEN                  )(const char *path, const char *mode, void **tiff_handle);
typedef int   (*TL_FENRIR_TIFF_CLOSE                 )(void *handle           );
typedef int   (*TL_FENRIR_TIFF_GET_NUM_IMAGES        )(void *handle, int *num_images         );
typedef int   (*TL_FENRIR_TIFF_SET_IMAGE_INDEX       )(void *handle, int index);
typedef int   (*TL_FENRIR_TIFF_GET_PIXEL_BUFFER      )(void *handle, void **pixel_buffer);
typedef int   (*TL_FENRIR_TIFF_GET_NUM_TAGS          )(void *handle, int *num_tags);
typedef int   (*TL_FENRIR_TIFF_GET_TAG_INFO          )(void *handle, int index, int *tag_id, int *tag_type, int *size_in_bytes, int *num_elements);
typedef int   (*TL_FENRIR_TIFF_GET_TAG_ID            )(void *handle, int index, int *tag_id);
typedef int   (*TL_FENRIR_TIFF_GET_TAG_TYPE          )(void *handle, int tag_id, int *tag_type);
typedef int   (*TL_FENRIR_TIFF_GET_TAG_SIZE_IN_BYTES )(void *handle, int tag_id, int *tag_size);
typedef int   (*TL_FENRIR_TIFF_GET_TAG_NUM_ELEMENTS  )(void *handle, int tag_id, int *tag_num_elements);
typedef int   (*TL_FENRIR_TIFF_GET_TAG_INT           )(void *handle, int tag_id, int index, int *tag_int);
typedef int   (*TL_FENRIR_TIFF_GET_TAG_STR           )(void *handle, int tag_id, int index, char *tag, int tag_size);
typedef int   (*TL_FENRIR_TIFF_GET_TAG_FLOAT         )(void *handle, int tag_id, int index, float *tag);
typedef int   (*TL_FENRIR_TIFF_ADD_IMAGE             )(void *handle, int image_type, int width, int height, void *pixel_buffer, int pixel_buffer_element_size);
typedef int   (*TL_FENRIR_TIFF_SET_TAG               )(void *handle, int tag_id, int tag_type, int size_in_bytes, int num_elements, void *tag_value);

#ifndef thorlabs_tsi_fenrir_sdk_EXPORTS

#ifdef __cplusplus
extern "C"
{
#endif
//------------------------------------------------------------------------------
// Exported functions 
//------------------------------------------------------------------------------
extern TL_FENRIR_TIFF_OPEN tl_fenrir_tiff_open;
extern TL_FENRIR_TIFF_CLOSE tl_fenrir_tiff_close;
extern TL_FENRIR_TIFF_GET_NUM_IMAGES tl_fenrir_tiff_get_num_images;
extern TL_FENRIR_TIFF_SET_IMAGE_INDEX tl_fenrir_tiff_set_image_index;
extern TL_FENRIR_TIFF_GET_PIXEL_BUFFER tl_fenrir_tiff_get_pixel_buffer;
extern TL_FENRIR_TIFF_GET_NUM_TAGS tl_fenrir_tiff_get_num_tags;
extern TL_FENRIR_TIFF_GET_TAG_INFO tl_fenrir_tiff_get_tag_info;
extern TL_FENRIR_TIFF_GET_TAG_ID tl_fenrir_tiff_get_tag_id;
extern TL_FENRIR_TIFF_GET_TAG_TYPE tl_fenrir_tiff_get_tag_type;
extern TL_FENRIR_TIFF_GET_TAG_SIZE_IN_BYTES tl_fenrir_tiff_get_tag_size_in_bytes;
extern TL_FENRIR_TIFF_GET_TAG_NUM_ELEMENTS tl_fenrir_tiff_get_tag_num_elements;
extern TL_FENRIR_TIFF_GET_TAG_INT tl_fenrir_tiff_get_tag_int;
extern TL_FENRIR_TIFF_GET_TAG_STR tl_fenrir_tiff_get_tag_str;
extern TL_FENRIR_TIFF_GET_TAG_FLOAT tl_fenrir_tiff_get_tag_float;
extern TL_FENRIR_TIFF_ADD_IMAGE tl_fenrir_tiff_add_image;
extern TL_FENRIR_TIFF_SET_TAG tl_fenrir_tiff_set_tag;
#ifdef __cplusplus
}
#endif

#endif


////------------------------------------------------------------------------------
//// Exported functions 
////------------------------------------------------------------------------------
//F_EXPORT void *tl_fenrir_tiff_open                 (char *path, char *mode);  // "r": read, "w": write
//F_EXPORT int   tl_fenrir_tiff_close                (void *handle);
//F_EXPORT int   tl_fenrir_tiff_get_num_images       (void *handle);
//F_EXPORT int   tl_fenrir_tiff_set_image_index      (void *handle, int index);
//F_EXPORT void *tl_fenrir_tiff_get_pixel_buffer     (void *handle);
//F_EXPORT int   tl_fenrir_tiff_get_num_tags         (void *handle);
//F_EXPORT int   tl_fenrir_tiff_get_tag_info         (void *handle, int index, int *tag_id, int *tag_type, int *size_in_bytes, int *num_elements);
//F_EXPORT int   tl_fenrir_tiff_get_tag_id           (void *handle, int index);
//F_EXPORT int   tl_fenrir_tiff_get_tag_type         (void *handle, int tag_id);
//F_EXPORT int   tl_fenrir_tiff_get_tag_size_in_bytes(void *handle, int tag_id);
//F_EXPORT int   tl_fenrir_tiff_get_tag_num_elements (void *handle, int tag_id);
//F_EXPORT int   tl_fenrir_tiff_get_tag_int          (void *handle, int tag_id, int index);
//F_EXPORT char *tl_fenrir_tiff_get_tag_str          (void *handle, int tag_id, int index);
//F_EXPORT float tl_fenrir_tiff_get_tag_float        (void *handle, int tag_id, int index);
//F_EXPORT int tl_fenrir_tiff_add_image(void* handle, int image_type, int w, int h, int stride_in_bytes, void* pixel_buffer);
//F_EXPORT int tl_fenrir_tiff_set_tag(void* handle, int tag_id, int tag_type, int size_in_bytes, int num_elements, void* tag_value);
//
//
//
////------------------------------------------------------------------------------
//// tl_fenrir_tiff_add_image - @width            : Width of image in pixels.
////                      @height           : Height of image in pixels.
////                      @stride_in_bytes  : Stride of image in bytes. The memory width of one row.
////                      @pixel_buffer     : The pixel buffer for the image.
////------------------------------------------------------------------------------
//F_EXPORT int tl_fenrir_tiff_add_image(void *handle, int image_type, int w, int h, int stride_in_bytes, void *pixel_buffer);
//
////------------------------------------------------------------------------------
//// tl_fenrir_tiff_set_tag     - @tag_id          : TIFF_TAG_XXXX id. 
////                        @tag_type        : TSI_TIFF_TAG_DATA_TYPE
////                        @tag_num_elements: Value from 1 to N, used to specify an array of elements.
////                        @tag_data        : If 0, then reserve space for tag, otherwise
////                                           address of tag data.
////------------------------------------------------------------------------------
//F_EXPORT int tl_fenrir_tiff_set_tag (void *handle, int tag_id, int tag_type, int size_in_bytes, int num_elements, void *tag_value);
//




// GetPathFreeSpace

// SetPathSizeLimit


//#endif
