#include "classic_tiff_library.h"
#include <stdexcept>
#include <condition_variable>
#include <chrono>
#include <cassert>
#include <fstream> 
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "ClassicTiffReader.h"
#include "ClassicTiffConverter.h"

bool ClassicTiffReader::_instanceFlag = false;
ClassicTiffReader* ClassicTiffReader::_single;

using namespace std;
using namespace tiff;

ClassicTiffReader::ClassicTiffReader()
{
}

ClassicTiffReader::~ClassicTiffReader()
{
	_instanceFlag = false;
}

ClassicTiffReader* ClassicTiffReader::getInstance()
{
	if (!_instanceFlag)
	{
		_single = new ClassicTiffReader();
		_instanceFlag = true;
	}
	return _single;
}


int ClassicTiffReader::GetImageCount(const wchar_t* file_name, int* tiff_handle, unsigned int* image_count)
{
	long status = LoadImageCount(file_name, tiff_handle, image_count);
	return status;
}

int ClassicTiffReader::GetImageInformation(int tiff_handle, unsigned int frame_number, ImageInfo* image_info)
{
	long status = LoadImageInfo(tiff_handle, frame_number, image_info);
	return status;
}

int ClassicTiffReader::GetImageBuffer(int tiff_handle, unsigned int frame_number, void* image_data)
{
	long status = load_image_data(tiff_handle, frame_number, image_data, 0);
	return status;
}

int ClassicTiffReader::CloseImage(int tiff_handle)
{
	close_tiff(tiff_handle);
	return STATUS_OK;
}

long ClassicTiffReader::LoadImageInfo(int tiff_handle, unsigned int frame_number, ImageInfo* image_info)
{
	SingleImageInfo info;
	long status = get_image_info(tiff_handle, frame_number, &info);
	if (status != STATUS_OK)
	{
		return status;
	}

	image_info->width = info.width;
	image_info->height = info.height;
	image_info->m_validBits = info.valid_bits;
	image_info->pixel_type = (int)info.pixel_type;
	image_info->image_type = (int)info.image_type;
	image_info->compression_mode = (int)info.compress_mode;

	const int bytes[] = { 1, 1, 2, 2, 4 };
	image_info->line_bytes = image_info->width * bytes[image_info->pixel_type] * (image_info->image_type == 1 ? 3 : 1);
	
	return STATUS_OK;
}

long ClassicTiffReader::LoadImageCount(const wchar_t* file_name, int* tiff_handle, unsigned int* image_count)
{
	int handle = open_tiff(file_name, tiff::OpenMode::READ_ONLY_MODE);
	if (handle < 0)
	{
		return handle;
	}

	unsigned int count;
	long status = get_image_count(handle, &count);
	if (status != STATUS_OK || count < 1)
	{
		return status;
	}

	*tiff_handle = handle;
	*image_count = count;

	return STATUS_OK;
}