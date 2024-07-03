#include "stdafx.h"
#include "SimulatorImage.h"
#include "TorboJpeg/turbojpeg.h"
#include "..\..\..\Tools\ImageProcessLibrary.1.0.3\build\native\ipltypes.h"
#include "..\..\..\Tools\ImageProcessLibrary.1.0.3\build\native\ImageProcessLibrary.h"
unsigned char* jpeg_reader(const char* filepath, int* width, int* height)
{
	FILE* file = nullptr;
	if (fopen_s(&file, filepath, "rb") != 0) return nullptr;
	else
	{
		fseek(file, 0, SEEK_END);
		long file_len = ftell(file);
		fseek(file, 0, SEEK_SET);
		unsigned char* file_data = new unsigned char[file_len + 1];
		memset(file_data, 0, file_len + 1);
		fread(file_data, sizeof(unsigned char), file_len, file);
		fclose(file);
		tjhandle handler = tjInitDecompress();
		if (handler == nullptr)
		{
			delete[] file_data;
			return nullptr;
		}
		else
		{
			int samp, color_space;
			tjDecompressHeader3(handler, file_data, file_len, width, height, &samp, &color_space);
			unsigned char* gray_buffer = new unsigned char[*width*(*height)];
			tjDecompress2(handler, file_data, file_len, gray_buffer, *width, 0, *height, TJPF_GRAY, TJFLAG_ACCURATEDCT);
			tjDestroy(handler);
			delete[] file_data;
			return gray_buffer;
		}
	}
}

unsigned short* jpeg_reader(const char * filepath, int bits, int* dst_width, int* dst_height)
{

	unsigned char* gray_buffer = jpeg_reader(filepath, dst_width, dst_height);
	int width = *dst_width;
	int height = *dst_height;
	if (gray_buffer == nullptr) return nullptr;
	int index = 0;
	unsigned short* dst_buffer = (unsigned short*)calloc(width*height, sizeof(unsigned short));
	int max_value = (0x01 << bits) - 1;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			index = i * width + j;
			dst_buffer[index] = gray_buffer[index] * max_value / MAXBYTE;
		}
	}
	delete[] gray_buffer;
	return dst_buffer;
}

SimulatorImage::SimulatorImage(const char* path, double fieldWidth, double fieldHeight)
{
	if (path != NULL)
	{
		_buffer = jpeg_reader(path, 14, &_width, &_height);
	}
	_maxScale = max(fieldWidth / _width, fieldHeight / _height);
}


SimulatorImage::~SimulatorImage()
{
	if (_buffer != NULL)
	{
		delete _buffer;
		_buffer = NULL;
	}
}

void SimulatorImage::GetImageBuffer(StripInfo* stripInfo, unsigned short* buffer, int stripWidth)
{
	IplRect srcRect;
	srcRect.x = (unsigned int)((stripInfo->XPos) / _maxScale);
	srcRect.y = (unsigned int)((stripInfo->YPos) / _maxScale);
	srcRect.width = (unsigned int)((stripInfo->XPhysicalSize) / _maxScale);
	srcRect.height = (unsigned int)((stripInfo->YPhysicalSize) / _maxScale);

	IplRect destRect;
	destRect.x = 0;
	destRect.y = 0;
	destRect.width = stripInfo->FrameROI.width;
	destRect.height = stripInfo->FrameROI.height;
	fnImageProcess_resize_16u_C1R(_buffer, srcRect, _width, buffer, destRect, stripWidth);
	return ;
}

