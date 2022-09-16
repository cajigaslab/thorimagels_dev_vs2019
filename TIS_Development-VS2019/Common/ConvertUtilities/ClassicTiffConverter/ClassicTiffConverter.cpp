#include "ClassicTiffConverter.h"
#include "ClassicTiffReader.h"
#include "OMETiffWritter.h"
#include "RawDataReader.h"
#include "RawToTIFFConverter.h"

// Tiff
int TC_GetImageCount(const wchar_t* file_name, int* tiff_handle, unsigned int* image_count)
{
	return ClassicTiffReader::GetImageCount(file_name, tiff_handle, image_count);
}

int TC_GetImageInfo(int tiff_handle, unsigned int image_number, ImageInfo* info)
{
	return ClassicTiffReader::GetImageInformation(tiff_handle, image_number, info);
}

int TC_GetImageData(int tiff_handle, unsigned int image_number, void* image_data)
{
	return ClassicTiffReader::GetImageBuffer(tiff_handle, image_number, image_data);
}

int TC_CloseImage(int image_handle)
{
	return ClassicTiffReader::CloseImage(image_handle);
}

// Raw
int TC_LoadRawDataFile(const wchar_t* file_name, int* image_handle)
{
	return RawDataReader::LoadRawDataFile(file_name, image_handle);
}

int TC_GetRawData(int image_handle, int offset, int size, void* image_data)
{
	return RawDataReader::GetRawData(image_handle, offset, size, image_data);
}

int TC_CloseRawImage(int image_handle)
{
	return RawDataReader::CloseImage(image_handle);
}

// OME tiff
long TC_CreateOMETiff(char* file_name)
{
	return OMETiffWritter::Create(file_name);
}

long TC_ConfigOMEHeader(long handle, SampleInfo sample, int regionPixelX, int regionPixelY, float regionW, float regionH, unsigned short zCount, unsigned short tCount,
	int regionPositionPixelX, int regionPositionPixelY, int bitsPerPixel, float regionPixelSizeUM, double zStepSizeUM, double intervalSec, int channelNumber, const char* channels)
{
	return OMETiffWritter::ConfigOMEHeader(handle, sample, regionPixelX, regionPixelY, regionW, regionH, zCount, tCount,
		regionPositionPixelX, regionPositionPixelY, bitsPerPixel, regionPixelSizeUM, zStepSizeUM, intervalSec, channelNumber, channels);
}

long TC_SaveOMEData(long handle, int channelID, int zIndex, int tIndex, unsigned short* data)
{
	return OMETiffWritter::SaveOMEData(handle, channelID, zIndex, tIndex, data);
}

long TC_SaveAdditionalData(long handle, const char* data, int size)
{
	const char* name = "Experiment";
	return OMETiffWritter::SaveAdditionalData(handle, data, size, name);
}

long TC_CloseOMETiff(long handle)
{
	return OMETiffWritter::CloseFile(handle);
}

int TC_ConvertRawToTIFF(const wchar_t* rawFileName, const wchar_t* tiffFolderName, int cCount, int tCount, int zCount, double intervalSec, const wchar_t* channelNameArray, long width, long height, double umPerPixel, double zStepSizeUM)
{
	return RawToTIFFConverter::ConvertRawToTIFF(rawFileName, tiffFolderName, cCount, tCount, zCount, intervalSec, channelNameArray, width, height, umPerPixel, zStepSizeUM);
}
