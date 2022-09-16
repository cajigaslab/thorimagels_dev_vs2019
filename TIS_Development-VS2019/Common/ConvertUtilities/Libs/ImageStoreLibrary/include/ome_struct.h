#pragma once
#ifdef OME_TIFF_LIBRARY_EXPORTS
#define OME_TIFF_LIBRARY_CLASS __declspec(dllexport)
#else
#define OME_TIFF_LIBRARY_CLASS __declspec(dllimport)
#endif

namespace ome
{
	/* Save pyramidal progress callback */
	typedef void(*ProgressCallback)(int);

	//These are the bit map of image_file_type
#define RAW_DATA 0x01 //"TRUE" means raw data exists
#define PYRAMIDAL_JPEG 0x02 //"TRUE" means 8bit pyramidal exists

	enum class DistanceUnit {
		DISTANCE_UNDEFINED = -1,
		DISTANCE_KILOMETER = 1,
		DISTANCE_METER = 2,
		DISTANCE_MILLIMETER = 3,
		DISTANCE_MICROMETER = 4,
		DISTANCE_NANOMETER = 5,
		DISTANCE_PICOMETER = 6,
	};

	enum class Shape {
		SHAPE_UNDEFINED = -1,
		SHAPE_RECTANGLE = 1,
		SHAPE_ELLIPSE = 2,
	};

	enum class TimeUnit {
		TIME_UNDEFINED = -1,
		TIME_SECOND = 1,
		TIME_MILLISECOND = 2,
		TIME_MICROSECOND = 3,
		TIME_NANOSECOND = 4,
		TIME_PICOSECOND = 5,
		TIME_FEMTOSECOND = 6,
	};

	enum class TiffTagDataType {
		TIFF_NOTYPE = 0,      /* placeholder */
		TIFF_BYTE = 1,        /* 8-bit unsigned integer */
		TIFF_ASCII = 2,       /* 8-bit bytes w/ last byte null */
		TIFF_SHORT = 3,       /* 16-bit unsigned integer */
		TIFF_LONG = 4,        /* 32-bit unsigned integer */
		TIFF_RATIONAL = 5,    /* 64-bit unsigned fraction */
		TIFF_SBYTE = 6,       /* !8-bit signed integer */
		TIFF_UNDEFINED = 7,   /* !8-bit untyped data */
		TIFF_SSHORT = 8,      /* !16-bit signed integer */
		TIFF_SLONG = 9,       /* !32-bit signed integer */
		TIFF_SRATIONAL = 10,  /* !64-bit signed fraction */
		TIFF_FLOAT = 11,      /* !32-bit IEEE floating point */
		TIFF_DOUBLE = 12,     /* !64-bit IEEE floating point */
		TIFF_IFD = 13,        /* %32-bit unsigned integer (offset) */
		TIFF_LONG8 = 16,      /* BigTIFF 64-bit unsigned integer */
		TIFF_SLONG8 = 17,     /* BigTIFF 64-bit signed integer */
		TIFF_IFD8 = 18        /* BigTIFF 64-bit unsigned integer (offset) */
	};

	enum class PixelType {
		PIXEL_UNDEFINED = -1,
		PIXEL_INT8 = 0,
		PIXEL_UINT8 = 1,
		PIXEL_UINT16 = 2,
		PIXEL_INT16 = 3,
		PIXEL_FLOAT32 = 4,
	};

	enum class ImageType {
		IMAGE_GRAY = 0,
		IMAGE_RGB = 1
	};

	enum class OpenMode {
		CREATE_MODE = 1,
		READ_WRITE_MODE = 2,
		READ_ONLY_MODE = 3,
	};

	enum ErrorCode {
		STATUS_OK = 0,

		TIFF_ERR_USELESS_HDL = -1,
		TIFF_ERR_READ_DATA_FROM_FILE_FAILED = -2,
		TIFF_ERR_OPEN_FILE_PARAMETER_ERROR = -3,
		TIFF_ERR_OPEN_FILE_FAILED = -4,
		TIFF_ERR_CLOSE_FILE_FAILED = -5,
		TIFF_ERR_NO_TIFF_FORMAT = -6,
		TIFF_ERR_BLOCK_SIZE_IN_IFD_IS_EMPTY = -7,
		TIFF_ERR_NEED_IFD_NOT_FOUND = -8,
		TIFF_ERR_BLOCK_OUT_OF_RANGE = -9,
		TIFF_ERR_NO_IFD_FOUND = -10,
		TIFF_ERR_FIRST_IFD_STRUCT_NOT_COMPLETE = -11,
		TIFF_ERR_BLOCK_OFFSET_OUT_OF_RANGE = -12,
		TIFF_ERR_TAG_NOT_FOUND = -13,
		TIFF_ERR_BAD_PARAMETER_VALUE = -14,
		TIFF_ERR_IFD_ALREADY_EXIST = -15,
		TIFF_ERR_IFD_OUT_OF_DIRECTORY = -16,
		TIFF_ERR_IFD_FULL = -17,
		TIFF_ERR_UNSUPPORTED_BITS_PER_SAMPLE = -18,
		TIFF_ERR_PREVIOUS_IFD_NOT_CLOSED = -19,
		TIFF_ERR_TAG_TYPE_INCORRECT = -20,
		TIFF_ERR_TAG_SIZE_INCORRECT = -21,
		TIFF_ERR_SEEK_FILE_POINTER_FAILED = -22,
		TIFF_ERR_ALLOC_MEMORY_FAILED = -23,
		TIFF_ERR_WRONG_OPEN_MODE = -24,
		TIFF_ERR_APPEND_TAG_NOT_ALLOWED = -25,
		TIFF_ERR_DUPLICATE_WRITE_NOT_ALLOWED = -26,

		ERR_FILE_PATH_ERROR = -101,
		ERR_HANDLE_NOT_EXIST = -102,
		ERR_PLATES_REPEAT_SET = -103,
		ERR_BUFFER_SIZE_ERROR = -104,
		ERR_BUFFER_IS_NULL = -105,
		ERR_PLATES_NOT_EXIST = -106,
		ERR_SCANS_NOT_EXIST = -107,
		ERR_FRAME_NOT_EXIST = -108,
		ERR_DATA_NOT_EXIST = -109,
		ERR_MODIFY_NOT_ALLOW = -110,
		ERR_CREATE_CONTAINER_FAILED = -111,
		ERR_CONTAINER_INVALID = -112,
		ERR_ZIP = -113,
		ERR_CREATE_OME_XML_FAILED = -114,
		ERR_READ_OME_XML_FAILED = -115,
		ERR_XML_TO_PLATES_FAILED = -116,
		ERR_XML_TO_SCAN_FAILED = -117,
		ERR_PLATES_TO_XML_FAILED = -118,
		ERR_SCANS_TO_XML_FAILED = -119,
		ERR_PAREMETER_IS_NULL = -120,
		ERR_NO_CONTAINER_EXIST = -121,
		ERR_GET_IMAGEINFO_FAILED = -122,
		ERR_GET_BLOCK_DATA_FAILED = -123,
		ERR_CREATE_IFD_FAILED = -124,
		ERR_DECOMPRESS_JPEG_FAILED = -125,
		ERR_SET_TAG_FAILED = -127,
		ERR_SAVE_BLOCK_FAILED = -128,
		ERR_CLOSE_FILE_FAILED = -129,
		ERR_SCAN_ID_EXIST = -130,
		ERR_GET_OMEXML_FAILED = -131,
		ERR_ROW_OUT_OF_RANGE = -132,
		ERR_COLUMN_OUT_OF_RANGE = -133,
		ERR_GET_TAG_FAILED = -134,
		ERR_SCALELEVEL_NOT_EXIST = -135,
		ERR_THUMBNAIL_NOT_EXIST = -136,
		ERR_STRIDE_NOT_CORRECT = -137,
		ERR_DATA_TYPE_NOTSUPPORT = -138,
		ERR_COMPRESS_TYPE_NOTSUPPORT = -139,
		ERR_TAG_TYPE_NOTSUPPORT = -140,
		ERR_GET_MIN_VALUE_FAILED = -141,
		ERR_GET_MAX_VALUE_FAILED = -142,
		ERR_DECOMPRESS_LZW_FAILED = -144,
		ERR_DECOMPRESS_LZ4_FAILED = -145,
		ERR_RAWCONTAINER_IS_NOT_EXIST = -146,
		ERR_DECOMPRESS_ZLIB_FAILED = -147,
		ERR_TAG_CONDITION_NOT_MET = -150,
		ERR_P2D_SHIFT_FAILED = -151,
		ERR_P2D_COPY_FAILED = -152,
		ERR_P2D_RESIZE_FAILED = -153,
	};

	enum class CompressionMode {
		COMPRESSIONMODE_NONE = 0,
		COMPRESSIONMODE_LZ4 = 1,
		COMPRESSIONMODE_LZW = 2,
		COMPRESSIONMODE_JPEG = 3,
		COMPRESSIONMODE_ZIP = 4,
	};

	//User can define any custom tag id between (CustomTag_First, CustomTag_Last), CustomTag_First and CustomTag_Last are not valid tag id.
	enum class CustomTag
	{
		CustomTag_First = 10600,
		CustomTag_Last = 10699,
	};

	struct OmeRect {
		int x;
		int y;
		int width;
		int height;
	};

	struct OmeSize {
		int width;
		int height;
	};

	class OME_TIFF_LIBRARY_CLASS WellSample
	{
	public:
		float position_x;
		float position_y;
		float position_z;
		DistanceUnit physicalsize_unit_x;
		DistanceUnit physicalsize_unit_y;
		DistanceUnit physicalsize_unit_z;

		WellSample();
		WellSample& operator=(WellSample& wellSample);
	};

	class OME_TIFF_LIBRARY_CLASS Well
	{
	public:
		unsigned short id;
		float position_x;
		float position_y;
		float width;
		float height;
		unsigned short row_index;
		unsigned short column_index;
		Shape well_shape;

		Well();
		Well& operator =(Well& well);
	};

	class OME_TIFF_LIBRARY_CLASS Plate
	{
	public:
		unsigned char id;
		float width;
		float height;
		DistanceUnit physicalsize_unit_x;
		DistanceUnit physicalsize_unit_y;
		unsigned short row_size;
		unsigned short column_size;

		Plate();
		~Plate();
		Well* add_well();
		Well** get_wells();
		Well* get_well(unsigned short id);
		unsigned short get_wells_size();
		void clear_wells();
		void set_name(const char* name);
		const char* get_name();
		Plate& operator=(Plate& plate);
	private:
		char* _name;
		void* _wells_array;
	};

	class OME_TIFF_LIBRARY_CLASS Plates
	{
	public:
		Plates();
		~Plates();
		Plate* add_plate();
		Plate** get_plates();
		Plate* get_plate(unsigned char id);
		unsigned char get_plates_size();
		void clear_plates();
		Plates& operator=(Plates& plates);
	private:
		void* _plates_array;
	};

	class OME_TIFF_LIBRARY_CLASS Channel
	{
	public:
		unsigned char id;

		Channel();
		~Channel();
		void set_name(const char* name);
		const char* name();
		Channel& operator=(Channel& channel);
	private:
		char* _name;
	};

	class OME_TIFF_LIBRARY_CLASS ScanRegion
	{
	public:
		unsigned short id;
		unsigned short well_id;
		unsigned int size_pixel_x;
		unsigned int size_pixel_y;
		unsigned short size_pixel_z;
		unsigned short size_stream;
		unsigned short size_time;
		WellSample well_sample;

		ScanRegion();
		~ScanRegion();
		ScanRegion& operator=(ScanRegion& region);
		void add_scale_level(unsigned char level);
		unsigned char** get_scale_levels();
		unsigned char get_scale_levels_size();
		void clear_scale_levels();
	private:
		void* _scale_levels_array;
	};

	class OME_TIFF_LIBRARY_CLASS Scan
	{
	public:
		unsigned char id;
		unsigned char plate_id;
		float pixel_physical_size_x;            // A pixel's width
		float pixel_physical_size_y;            // A pixel's height
		float pixel_physical_size_z;            // Z step lenth
		DistanceUnit pixel_physical_uint_x;     // Unit of "pixel_physical_size_x"
		DistanceUnit pixel_physical_uint_y;     // Unit of "pixel_physical_size_y"
		DistanceUnit pixel_physical_uint_z;     // Unit of "pixel_physical_size_z"
		unsigned int time_increment;            // Time series' increment
		TimeUnit time_increment_unit;           // Unit of "time_increment"
		unsigned int tile_pixel_size_width;     // Raw data's horizontal pixel count in a tile
		unsigned int tile_pixel_size_height;    // Raw data's vertical pixel count in a tile
		PixelType pixel_type;                   // A pixel's type
		unsigned char significant_bits;         // Actual bits the image used
		unsigned int thumbnail_tile_pixel_size; // Read only: Scale data and 8bit JPEG scale data's tile width and height
		unsigned int histogram_size;            // Read only: Saved 3D histogram's size

		Scan();
		~Scan();
		Channel* add_channel();
		Channel** get_channels();
		Channel* get_channel(unsigned char id);
		unsigned char get_channels_size();
		void clear_channels();
		ScanRegion* add_region();
		ScanRegion* get_region(unsigned short id);
		ScanRegion** get_regions();
		unsigned short get_regions_size();
		void clear_regions();
		void set_dimension_order(const char* dimension_order);

		const char* dimension_order();
		Scan& operator=(Scan& scan);
	private:
		char* _dimension_order;
		void* _channels_array;
		void* _regions_array;
	};

	class OME_TIFF_LIBRARY_CLASS Scans
	{
	public:
		Scans();
		~Scans();
		Scan** get_scans();
		Scan* get_scan(unsigned char id);
		unsigned char get_scans_size();
		void clear_scans();
		Scan* add_scan();
		Scans& operator=(Scans& scans);
	private:
		void* _scans_array;
	};

	struct FrameInfo
	{
		unsigned char scan_id;    // Better start from 0. Start from other number is OK. Unique in one OME-TIFF file
		unsigned short region_id; // Better start from 0. Start from other number is OK
		unsigned char channel_id; // Better start from 0. Start from other number is OK
		unsigned short z_id;      // must start from 0
		unsigned short time_id;   // must start from 0
		unsigned short s_id;      // must start from 0. Will be discarded.Use time_id instead

		bool operator< (const FrameInfo& f)const
		{
			if (scan_id < f.scan_id) return true;
			else if (scan_id > f.scan_id) return false;
			if (channel_id < f.channel_id) return true;
			else if (channel_id > f.channel_id) return false;
			if (region_id < f.region_id) return true;
			else if (region_id > f.region_id) return false;
			if (s_id < f.s_id) return true;
			else if (s_id > f.s_id) return false;
			if (time_id < f.time_id) return true;
			else if (time_id > f.time_id) return false;
			if (z_id < f.z_id) return true;
			else if (z_id > f.z_id) return false;
			return false;
		}
	};
}