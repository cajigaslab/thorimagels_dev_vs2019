#pragma once

struct ImageInfo;

class ClassicTiffReader
{
private:
	ClassicTiffReader();
	static bool _instanceFlag;
	static ClassicTiffReader* _single;

	static long LoadImageCount(const wchar_t* file_name, int* tiff_handle, unsigned int* image_count);
	static long LoadImageInfo(int tiff_handle, unsigned int frame_number, ImageInfo* image_info);

public:
	~ClassicTiffReader();
	static ClassicTiffReader* getInstance();

	static int GetImageCount(const wchar_t* file_name, int* tiff_handle, unsigned int* image_count);

	static int GetImageInformation(int tiff_handle, unsigned int frame_number, ImageInfo* image_info);

	static int GetImageBuffer(int tiff_handle, unsigned int frame_number, void* image_data);

	static int CloseImage(int tiff_handle);
};