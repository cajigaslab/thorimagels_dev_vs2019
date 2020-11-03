#pragma once
#include <Windows.h>
#include <memory>
#include "..\..\..\..\Tools\Intel IPP\intel64\include\IPPlib.h"

enum MORPH_TYPE
{
	MORPH_ERODE,
	MORPH_DILATE,
	MORPH_OPEN,
	MORPH_CLOSE
};
enum BI_TYPE
{
	BI_FIXED,
	BI_OTSU,
    BI_IN_RANGE = 10,
	BI_OUT_RANGE,
	BI_EQUAL,
	BI_GREATER,
	BI_LESS,
	BI_NOT_EQUAL
};

enum FILTER_TYPE
{
	FILTER_GREATER,
	FILTER_LESS,
	FILTER_EQUAL,
	FILTER_NOT_EQUAL,
	FILTER_AREA = 100,

};

enum MOMENT_TYPE
{
	M_0_0,
	M_0_1,
	M_1_0,
	M_1_1,
	M_2_0,
	M_0_2,
	M_2_1,
	M_1_2,
	M_3_0,
	M_0_3,
};


class ImageProcess
{
private:
	static bool _instanceFlag;
	static std::auto_ptr<ImageProcess> _single;
	IppiMomentState_64f* _moment;
	ImageProcess();
public:
	static ImageProcess* getInstance();
	LONG GetThreshold_Otsu_16u(const USHORT* srcImag, ULONG size, USHORT& threshold);
	LONG InRange_inPlace_16u(USHORT* imag, USHORT width, USHORT height, USHORT threshold);
	LONG InRange_notInPlace_16u(const USHORT* srcImag, USHORT* dstImag, USHORT width, USHORT height, USHORT threshold);
	LONG PickRange_notInPlace_16u(const USHORT* srcImag, USHORT* dstImag, USHORT width, USHORT height, USHORT threshold);
	LONG ImgSub_InPlace_16u(const USHORT* srcImag, USHORT* dstImag, USHORT width, USHORT height);
	LONG Dilate3x3_16u(USHORT* imag, USHORT width, USHORT height);
	LONG Erode3x3_16u(USHORT* imag, USHORT width, USHORT height);	
	LONG Moments_00_01_10(USHORT* imag, USHORT width, USHORT height, double& m00, double& m01, double& m10);
	LONG LableMarkers(USHORT* imag, USHORT width, USHORT height,int minLabel, USHORT norm, int& roiCounts);
	LONG LableImage(USHORT* img, USHORT* contourImg, USHORT width, USHORT height, USHORT& roiCounts, USHORT minRoiArea, USHORT minLabel);
	LONG LocalSearch(USHORT* img, USHORT* labelImg, USHORT* contureImg, USHORT width, USHORT height, int label, int* indexBuf, int& areaSize, int& contureSize);
	LONG FindContourPoints(USHORT* srcImg,  USHORT width, USHORT height, USHORT norm, int roiCounts, ULONG** roiPointsArray, USHORT** roiSizeArray);
	LONG FreeContourPointsBuf(ULONG** roiPointsArray, USHORT** roiSizeArray);
	LONG FindContourImg(USHORT* srcLabelImage, USHORT width, USHORT height, USHORT norm, USHORT* dstContourImage);
	LONG MorphImage(USHORT* imag,USHORT width, USHORT height, MORPH_TYPE morphType);
	LONG BinarizeImage(USHORT* srcImage, USHORT width, USHORT height,BI_TYPE binarizeType, int minSnr);
	LONG FilterImage(USHORT* srcImage, USHORT width, USHORT height, int& roicount, FILTER_TYPE filterType, long threshold,int minLabel, long* area);
	LONG Moments(USHORT* imag, USHORT width, USHORT height, MOMENT_TYPE momentType, double* momentValue);
	LONG GetArea(USHORT* imag, USHORT width, USHORT height,int roiCount, long* area, int minLabel);
	~ImageProcess();
};
