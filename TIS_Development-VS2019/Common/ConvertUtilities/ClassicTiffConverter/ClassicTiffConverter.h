#pragma once

#ifdef TIFFCONVERTER_EXPORTS
#define DLLAPI __declspec(dllexport)
#else
#define DLLAPI __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif
	struct ImageInfo
	{
		unsigned int width;
		unsigned int height;
		unsigned int line_bytes;
		int pixel_type;
		unsigned short m_validBits;
		int image_type;
		int compression_mode;
	};

	struct SampleInfo
	{
		char* name;
		float width;
		float height;
		char* type;
		unsigned short rowSize;
		unsigned short columnSize;
		float centerToCenterX;
		float centerToCenterY;
		float topLeftCenterOffsetX;
		float topLeftCenterOffsetY;
		int wellShape;
		float diameter; // for SHAPE_ELLIPSE
		float wellWidth; // for SHAPE_RECTANGLE
		float wellHeight; // for SHAPE_RECTANGLE
	};

	struct PhysicalSize
	{
		double x, y, z;	// phiscal size, usually in micron, of a single voxle in three dimension
	};

	DLLAPI int TC_GetImageCount(const wchar_t* file_name, int* tiff_handle, unsigned int* image_count);
	DLLAPI int TC_GetImageInfo(int tiff_handle, unsigned int image_number, ImageInfo* info);
	DLLAPI int TC_GetImageData(int tiff_handle, unsigned int image_number, void* image_data);
	DLLAPI int TC_CloseImage(int tiff_handle);

	DLLAPI int TC_LoadRawDataFile(const wchar_t* file_name, int* image_handle);
	DLLAPI int TC_GetRawData(int image_handle, int offset, int size, void* image_data);
	DLLAPI int TC_CloseRawImage(int image_handle);

	DLLAPI long TC_CreateOMETiff(char* file_name);
	DLLAPI long TC_ConfigOMEHeader(long handle, SampleInfo sample, int regionPixelX, int regionPixelY, float regionW, float regionH, unsigned short zCount, unsigned int tCount,
		int regionPositionPixelX, int regionPositionPixelY, int bitsPerPixel, float regionPixelSizeUM, double zStepSizeUM, double intervalSec, int channelNumber, const char* channels);
	DLLAPI long TC_SaveOMEData(long handle, int channelID, int zIndex, int tIndex, unsigned short* data);
	DLLAPI long TC_SaveAdditionalData(long handle, const char* data, int size);
	DLLAPI long TC_CloseOMETiff(long handle);

	DLLAPI int TC_ConvertRawToTIFF(const wchar_t* rawFileName, const wchar_t* tiffFolderName, int cCount, int tCount, int zCount, double intervalSec, const wchar_t* channelNameArray, long width, long height, double umPerPixel, double zStepSizeUM);

#ifdef __cplusplus
}
#endif