#pragma once

#ifdef IMAGEPROCESSLIBRARY_EXPORTS
#define IMAGEPROCESSLIBRARY_API extern "C" __declspec(dllexport)
#else
#define IMAGEPROCESSLIBRARY_API extern "C" __declspec(dllimport)
#endif
#include "IplTypes.h"

//memory
IMAGEPROCESSLIBRARY_API int fnImageProcess_reset(void* srcBuf, unsigned long long size);

//resize
IMAGEPROCESSLIBRARY_API int fnImageProcess_resize_16u_C1R(unsigned short* pSrc, IplRect srcRect, unsigned int srcWidth, unsigned short* pDst, IplRect dstRect, unsigned int dstWidth);
IMAGEPROCESSLIBRARY_API int fnImageProcess_resize_16s_C1R(short* pSrc, IplRect srcRect, unsigned int srcWidth, short* pDst, IplRect dstRect, unsigned int dstWidth);
IMAGEPROCESSLIBRARY_API int fnImageProcess_resize_8u_C1R(unsigned char* pSrc, IplRect srcRect, unsigned int srcWidth, unsigned char* pDst, IplRect dstRect, unsigned int dstWidth);
IMAGEPROCESSLIBRARY_API int fnImageProcess_shift_buffer(void* srcBuf, int srcBits, void* dstBuf, int dstBits, unsigned long long size);

//projection and slice
IMAGEPROCESSLIBRARY_API int fnImageProcess_get_slice_size(IplRect3D srcRect3D, double* dstAxes, IplRect* dstRect);

IMAGEPROCESSLIBRARY_API int fnImageProcess_projection_max_16u(unsigned short* pSrc, IplRect srcRect, unsigned int srcZ, unsigned int sizeZ, unsigned short* pDst, double* dstAxes, IplRect dstRect);
IMAGEPROCESSLIBRARY_API int fnImageProcess_projection_max_16s(short* pSrc, IplRect srcRect, unsigned int srcZ, unsigned int sizeZ, short* pDst, double* dstAxes, IplRect dstRect);
IMAGEPROCESSLIBRARY_API int fnImageProcess_projection_max_8u(unsigned char* pSrc, IplRect srcRect, unsigned int srcZ, unsigned int sizeZ, unsigned char* pDst, double* dstAxes, IplRect dstRect);

IMAGEPROCESSLIBRARY_API int fnImageProcess_projection_min_16u(unsigned short* pSrc, IplRect srcRect, unsigned int srcZ, unsigned int sizeZ, unsigned short* pDst, double* dstAxes, IplRect dstRect);
IMAGEPROCESSLIBRARY_API int fnImageProcess_projection_min_16s(short* pSrc, IplRect srcRect, unsigned int srcZ, unsigned int sizeZ, short* pDst, double* dstAxes, IplRect dstRect);
IMAGEPROCESSLIBRARY_API int fnImageProcess_projection_min_8u(unsigned char* pSrc, IplRect srcRect, unsigned int srcZ, unsigned int sizeZ, unsigned char* pDst, double* dstAxes, IplRect dstRect);

IMAGEPROCESSLIBRARY_API int fnImageProcess_projection_mean_16u(unsigned short* pSrc, IplRect srcRect, unsigned int srcZ, unsigned int sizeZ, unsigned short* pDst, double* dstAxes, IplRect dstRect);
IMAGEPROCESSLIBRARY_API int fnImageProcess_projection_mean_16s(short* pSrc, IplRect srcRect, unsigned int srcZ, unsigned int sizeZ, short* pDst, double* dstAxes, IplRect dstRect);
IMAGEPROCESSLIBRARY_API int fnImageProcess_projection_mean_8u(unsigned char* pSrc, IplRect srcRect, unsigned int srcZ, unsigned int sizeZ, unsigned char* pDst, double* dstAxes, IplRect dstRect);

IMAGEPROCESSLIBRARY_API int fnImageProcess_section_slice_16u(unsigned short* pSrc, IplRect srcRect, unsigned int srcZ, unsigned short* pDst, double* dstAxes, IplRect dstRect);
IMAGEPROCESSLIBRARY_API int fnImageProcess_section_slice_8u(unsigned char* pSrc, IplRect srcRect, unsigned int srcZ, unsigned char* pDst, double* dstAxes, IplRect dstRect);

IMAGEPROCESSLIBRARY_API int fnImageProcess_get_histogram_8u(unsigned char* pSrc, unsigned int srcSize, unsigned int* pHist, unsigned int histSize);
IMAGEPROCESSLIBRARY_API int fnImageProcess_get_histogram_16u(unsigned short* pSrc, unsigned int srcSize, unsigned int* pHist, unsigned int histSize);

IMAGEPROCESSLIBRARY_API int fnImageProcess_max_16s(short* buffer, int width, int height, int channels, short* pMaxValue);
IMAGEPROCESSLIBRARY_API int fnImageProcess_max_16u(unsigned short* buffer, int width, int height, int channels, unsigned short* pMaxValue);

IMAGEPROCESSLIBRARY_API int fnImageProcess_min_16s(short* buffer, int width, int height, int channels, short* pMaxValue);
IMAGEPROCESSLIBRARY_API int fnImageProcess_min_16u(unsigned short* buffer, int width, int height, int channels, unsigned short* pMaxValue);

//brightness contrast
IMAGEPROCESSLIBRARY_API int fnImageProcess_set_brightness_contrast_16u(unsigned short * pSrc, int width, int height, int channels, int bits, double alpha, int beta, unsigned short * pDst);
IMAGEPROCESSLIBRARY_API int fnImageProcess_set_brightness_contrast_8u(unsigned char * pSrc, int width, int height, int channels, int bits, double alpha, int beta, unsigned char * pDst);
IMAGEPROCESSLIBRARY_API int fnImageProcess_to_computer_Color_BC_16u(unsigned short* src, int srcBits, unsigned char* colortable, double alpha, int beta, unsigned short* dst, int dstBits, int size);
IMAGEPROCESSLIBRARY_API int fnImageProcess_to_computer_Color_BC_8u(unsigned char* src, int srcBits, unsigned char* colortable, double alpha, int beta, unsigned short* dst, int dstBits, int size);
