// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ThorImageProcess.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


DllExportImageProcess GetThreshold_Otsu_16u(const USHORT * srcImg, ULONG size, USHORT &threshold)
{
	return ImageProcess::getInstance()->GetThreshold_Otsu_16u(srcImg, size, threshold);
}

DllExportImageProcess InRange_InPlace_16u(USHORT* imag, USHORT width, USHORT height, USHORT threshold)
{
	return ImageProcess::getInstance()->InRange_inPlace_16u(imag, width, height, threshold);
}

DllExportImageProcess InRange_NotInPlace_16u(const USHORT* srcImag, USHORT* dstImag, USHORT width, USHORT height, USHORT threshold)
{
	return ImageProcess::getInstance()->InRange_notInPlace_16u(srcImag, dstImag, width, height, threshold);
}

//DllExportImageProcess PickRange_NotInPlace_16u(const USHORT* srcImag, USHORT* dstImag, USHORT width, USHORT height, USHORT threshold)
//{
//	return ImageProcess::getInstance()->PickRange_notInPlace_16u(srcImag, dstImag, width, height, threshold);
//}
//
//DllExportImageProcess ImgSub_InPlace_16u(const USHORT* srcImag, USHORT* dstImag, USHORT width, USHORT height)
//{
//	return ImageProcess::getInstance()->ImgSub_InPlace_16u(srcImag, dstImag, width, height);
//}

DllExportImageProcess Dilate3x3_16u(USHORT* imag, USHORT width, USHORT height)
{
	return ImageProcess::getInstance()->Dilate3x3_16u(imag, width, height);
}

DllExportImageProcess Erode3x3_16u(USHORT* imag, USHORT width, USHORT height)
{
	return ImageProcess::getInstance()->Erode3x3_16u(imag, width, height);
}

DllExportImageProcess Moments_00_01_10(USHORT* imag, USHORT width, USHORT height, double& m00, double& m01, double& m10)
{
	return ImageProcess::getInstance()->Moments_00_01_10(imag, width, height, m00, m01, m10);
}

DllExportImageProcess LableMarkers(USHORT* imag, USHORT width, USHORT height,int minLabel, USHORT norm, int & roiCounts)
{
	return ImageProcess::getInstance()->LableMarkers(imag, width, height, minLabel, norm, roiCounts);
}

DllExportImageProcess LableImage(USHORT* img, USHORT* contourImg, USHORT width, USHORT height, USHORT& roiCounts, USHORT minRoiArea, USHORT minLabel)
{
	return ImageProcess::getInstance()->LableImage(img, contourImg,  width, height, roiCounts, minRoiArea, minLabel);
}

DllExportImageProcess FindContourPoints(USHORT* srcImg,  USHORT width, USHORT height, USHORT norm, int roiCounts, ULONG** roiPointsArray, USHORT** roiSizeArray)
{
	return ImageProcess::getInstance()->FindContourPoints(srcImg, width, height, norm, roiCounts, roiPointsArray, roiSizeArray);
}

DllExportImageProcess FreeContourPointssBuf(ULONG** roiPointsArray, USHORT** roiSizeArray)
{
	return ImageProcess::getInstance()->FreeContourPointsBuf(roiPointsArray, roiSizeArray);
}

DllExportImageProcess FindContourImg(USHORT* srcLabelImage, USHORT width, USHORT height, USHORT norm, USHORT* dstContourImage)
{
	return ImageProcess::getInstance()->FindContourImg(srcLabelImage, width, height, norm, dstContourImage);
}

DllExportImageProcess MorphImage(USHORT* imag,USHORT width, USHORT height, MORPH_TYPE morphType)
{
	return ImageProcess::getInstance()->MorphImage(imag, width, height, morphType);
}

DllExportImageProcess BinarizeImage(USHORT* srcImage, USHORT width, USHORT height, BI_TYPE binarizeType, int minSnr)
{
	return ImageProcess::getInstance()->BinarizeImage( srcImage,  width,  height, binarizeType, minSnr);
}

DllExportImageProcess Moments(USHORT* imag, USHORT width, USHORT height, MOMENT_TYPE momentType, double* momentValue)
{
	return ImageProcess::getInstance()->Moments(imag, width, height, momentType, momentValue);
}

DllExportImageProcess FilterImage(USHORT* srcImage, USHORT width, USHORT height, int& roicount, FILTER_TYPE filterType, long threshold,int minLabel, long* area)
{
	return ImageProcess::getInstance()->FilterImage(srcImage, width, height, roicount, filterType, threshold,minLabel, area);
}

DllExportImageProcess GetArea(USHORT* imag, USHORT width, USHORT height,int roiCount, long* area, int minLabel)
{
	return ImageProcess::getInstance()->GetArea(imag, width, height, roiCount,area,minLabel);
}