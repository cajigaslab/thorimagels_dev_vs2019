#include ".\include\turbojpeg.h"  
#include <fstream>

long jpeg_decompress(unsigned char* buf, unsigned char* encoded_buf, unsigned long encoded_size)
{
	int ret = -1;
	tjhandle handle = tjInitDecompress();
	if (handle == nullptr)
		return -1;
	int width, height, subsample, colorspace;
	if (tjDecompressHeader3(handle, encoded_buf, encoded_size, &width, &height, &subsample, &colorspace) == 0)
	{
		ret = tjDecompress2(handle, encoded_buf, encoded_size, buf, width, 0, height, TJPF_GRAY, TJFLAG_ACCURATEDCT);
	}
	tjDestroy(handle);
	return ret;
}
long jpeg_compress(unsigned char* buf, unsigned char** encoded_buf, unsigned long* encoded_size, unsigned long width, unsigned long height)
{
	int ret = -1;
	tjhandle handle = tjInitCompress();
	if (handle == nullptr)
		return -1;
	ret = tjCompress2(handle, buf, width, width, height, TJPF_GRAY, encoded_buf, encoded_size, TJSAMP_GRAY, 50, TJFLAG_ACCURATEDCT);
	tjDestroy(handle);
	return ret;
}
long jpeg_write_file(void* buf, char* file_name, unsigned long size)
{
	std::ofstream fout(file_name, std::ios::binary);
	fout.write((char *)buf, size);
	fout.close();
	return 0;
}
