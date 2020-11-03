// ThorImageProcess.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "ThorImageProcess.h"
#include <objbase.h>
#include "..\..\..\..\common\thread.h"

#define IPP_MAX_16U 65535

using namespace std;

CritSect critSect;

auto_ptr<IPPCVDll> ippcvDll(new IPPCVDll(L".\\ippcvu8-7.0.dll"));
auto_ptr<IPPCOREDll> ippcoreDll(new IPPCOREDll(L".\\ippcore-7.0.dll"));
auto_ptr<IPPIDll> ippiDll(new IPPIDll(L".\\ippiu8-7.0.dll"));

bool ImageProcess::_instanceFlag = false;
std::auto_ptr<ImageProcess> ImageProcess::_single(NULL);

ImageProcess::ImageProcess()
{
	ippiDll->ippiMomentInitAlloc_64f(&_moment, ippAlgHintFast);
}

ImageProcess::~ImageProcess()
{
	_instanceFlag = false;
	ippiDll->ippiMomentFree_64f(_moment);
}

ImageProcess* ImageProcess::getInstance()
{
	if(! _instanceFlag)
	{
		_single.reset(new ImageProcess());
		_instanceFlag = true;
	}
	return _single.get();
}

LONG ImageProcess::GetThreshold_Otsu_16u(const USHORT* srcImg, ULONG size, USHORT& threshold)
{
	const int Max_8u = 256;

	long val = FALSE;
	UCHAR* upperImg = (UCHAR*)malloc(size * sizeof(UCHAR));
	ULONG* upperHist = (ULONG*)malloc(Max_8u * sizeof(ULONG));
	ZeroMemory(upperHist, Max_8u * sizeof(ULONG));

	for(int i = 0; i < static_cast<int>(size); i++)
	{
		upperImg[i] = (UCHAR)(srcImg[i]>>8);
		upperHist[upperImg[i]]++;
	}

	IppiSize upperImgRect = {static_cast<int>(size), 1};
	Ipp8u upperThreshold = 1;

	IppStatus ok = ippiDll->ippiComputeThreshold_Otsu_8u_C1R(upperImg, size, upperImgRect, &upperThreshold);
	if(ok >= 0)
	{
		UCHAR* lowerImg = (UCHAR*)malloc(upperHist[upperThreshold] * sizeof(UCHAR));
		for(int i = 0, j = 0; i < static_cast<int>(size); i++)
		{
			if(upperImg[i] == upperThreshold)
			{
				lowerImg[j] = (UCHAR)srcImg[i];
				j++;
			}
		}

		IppiSize lowerImgRect = {static_cast<int>(upperHist[upperThreshold]), 1};
		Ipp8u lowerThreshold = 0;

		ok = ippiDll->ippiComputeThreshold_Otsu_8u_C1R(lowerImg, upperHist[upperThreshold], lowerImgRect, &lowerThreshold);
		if(ok >= 0)
		{
			threshold = (USHORT)((upperThreshold<<8) + lowerThreshold);
			val = TRUE; 
		}
		free(lowerImg);
	}
	free(upperImg);
	free(upperHist);
	return val;
}

LONG ImageProcess::InRange_inPlace_16u(USHORT* imag, USHORT width, USHORT height, USHORT threshold)
{
	long val = FALSE;
	if(imag != NULL)
	{
		for(int i = 0; i < width * height; i++)
		{
			if(*(imag + i) < threshold)
				*(imag + i) = 0;
		}		
		val = TRUE; 
	}
	return val;
}

LONG ImageProcess::InRange_notInPlace_16u(const USHORT* srcImag, USHORT* dstImag, USHORT width, USHORT height, USHORT threshold)
{
	long val = FALSE;
	if(srcImag != NULL && dstImag != NULL)
	{
		IppiSize roi = {width, height};
		IppStatus ok = ippiDll->ippiThreshold_LTValGTVal_16u_C1R(srcImag, width * sizeof(USHORT), dstImag, width, roi, threshold, 0, threshold, IPP_MAX_16U);
		if(ok >= 0)
		{
			val = TRUE; 
		}
	}
	return val;
}

LONG ImageProcess::PickRange_notInPlace_16u(const USHORT* srcImag, USHORT* dstImag, USHORT width, USHORT height, USHORT threshold)
{
	long val = FALSE;
	if(srcImag != NULL && dstImag != NULL)
	{
		IppiSize roi = {width, height};
		IppStatus ok = ippiDll->ippiThreshold_LTValGTVal_16u_C1R(srcImag, width * sizeof(USHORT), dstImag, width, roi, threshold, 0, threshold, 0);
		if(ok >= 0)
		{
			val = TRUE; 
		}
	}
	return val;
}

LONG ImageProcess::ImgSub_InPlace_16u(const USHORT* srcImag, USHORT* srcDstImag, USHORT width, USHORT height)
{
	long val = FALSE;
	if(srcImag != NULL && srcDstImag != NULL)
	{
		IppiSize roi = {width, height};
		IppStatus ok = ippiDll->ippiSub_16u_C1IRSfs(srcImag, width * sizeof(USHORT), srcDstImag,  width * sizeof(USHORT), roi, 0);
		if(ok >= 0)
		{
			val = TRUE; 
		}
	}
	return val;
}

LONG ImageProcess::Dilate3x3_16u(USHORT* imag, USHORT width, USHORT height)
{
	long val = FALSE;
	if (width<2 && height<2)
	{
		return FALSE;
	}
	if(imag != NULL)
	{
		IppiSize roi = {width-2, height-2};
		IppStatus ok = ippiDll->ippiDilate3x3_16u_C1IR((imag+width+1), width*sizeof(USHORT), roi);
		if(ok >= 0)
		{
			val = TRUE; 
		}
	}
	return val;
}

LONG ImageProcess::Erode3x3_16u(USHORT* imag, USHORT width, USHORT height)
{
	long val = FALSE;
	if (width<2 && height<2)
	{
		return FALSE;
	}
	if(imag != NULL)
	{
		IppiSize roi = {width-2, height-2};
		IppStatus ok = ippiDll->ippiErode3x3_16u_C1IR((imag+width+1), width*sizeof(USHORT), roi);
		if(ok >= 0)
		{
			val = TRUE; 
		}
	}
	return val;
}

LONG ImageProcess::Moments_00_01_10(USHORT* imag, USHORT width, USHORT height, double& m00, double& m01, double& m10)
{
	long val = FALSE;
	/*if(imag != NULL)
	{
	IppiSize roi = {width, height};
	IppiMomentState_64f* moment = NULL;
	IppStatus ok = ippiDll->ippiMoments64f_16u_C1R(imag, width, roi, moment);
	if(ok >= 0)
	{
	IppiPoint point = { 0, 0 };
	ippiGetSpatialMoment_64f(moment, 0, 0, 0, point, &m00);			
	ippiGetSpatialMoment_64f(moment, 0, 1, 0, point, &m01);
	ippiGetSpatialMoment_64f(moment, 1, 0, 0, point, &m10);
	val = TRUE; 
	}
	}*/
	return val;
}

LONG ImageProcess::Moments(USHORT* imag, USHORT width, USHORT height, MOMENT_TYPE momentType, double* momentValue )
{
	long val = FALSE;
	if(imag != NULL && _moment != NULL)
	{
		IppiSize roi = {width, height};

		IppStatus ok = ippiDll->ippiMoments64f_16u_C1R(imag, width * sizeof(USHORT), roi, _moment);
		if (ok >= 0)
		{
			IppiPoint point = {0, 0};

			switch (momentType)
			{
			case M_0_0:
				ippiDll->ippiGetSpatialMoment_64f(_moment, 0, 0, 0, point, momentValue);
				val = TRUE;
				break;
			case M_0_1:
				break;
			case M_1_0:
				break;
			case M_1_1:
				break;
			case M_2_0:
				break;
			case M_0_2:
				break;
			case M_2_1:
				break;
			case M_1_2:
				break;
			case M_3_0:
				break;
			case M_0_3:
				break;
			default:
				break;
			}//end switch
		}

	}

	return val;
}
//boundRect left right top bottom
LONG ImageProcess::GetArea(USHORT* imag, USHORT width, USHORT height, int roiCount, long* area, int minLabel)
{
	if(imag == NULL)
	{
		return FALSE;
	}

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if(*(imag + i * width + j) < minLabel) 
				continue;
			area[*(imag + i * width + j) - minLabel]++;
		}
	}
	return TRUE;

}

LONG ImageProcess::LableImage(USHORT* img, USHORT* contourImg, USHORT width, USHORT height, USHORT& roiCounts, USHORT minRoiArea, USHORT minLabel)
{
	long ret = TRUE;
	int size = width * height;
	USHORT* labelImg = (USHORT*)malloc(size * sizeof(USHORT));
	ZeroMemory(labelImg, size * sizeof(USHORT));

	USHORT label = minLabel;
	int maxRoiNum = roiCounts;
	int* indexBuff = (int*)malloc(width * height * sizeof(int));
	roiCounts = 0;
	for(int i = 0; i < size; i++)
	{
		if(*(img + i))
		{
			*(indexBuff) = i;
			*(labelImg + i) = label;
			int contureSize = 0;
			int areaSize = 1;
			LocalSearch(img, labelImg, contourImg, width, height, label, indexBuff, areaSize, contureSize);
			if(areaSize > minRoiArea)
			{
				label++;
				roiCounts++;
				if(roiCounts == maxRoiNum) 
					break;
			}
			else
			{
				for(int j = 0; j < areaSize; j++)
				{
					*(labelImg + *(indexBuff + j)) = 0;
				}
				for(int j = 0; j < contureSize; j++)
				{
					*(contourImg + *(indexBuff + size - 1 -j )) = 0;
				}
			}
		}
	}
	memcpy(img, labelImg, size * sizeof(USHORT));
	free(labelImg);
	free(indexBuff);
	return ret;
}

LONG ImageProcess::LocalSearch(USHORT* img, USHORT* labelImg, USHORT* contureImg, USHORT width, USHORT height, int label, int* indexBuf, int& areaSize, int& contureSize)
{
	long ret = TRUE;
	const POINT Deltas[8]={{1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}};	
	int size = width * height;
	int areaStackDepth = 0;

	while(areaStackDepth != areaSize)
	{		
		int p = *(indexBuf + areaStackDepth);
		areaStackDepth++;
		if(*(img + p))
		{
			*(img + p) = 0;
			for(int k = 0; k < 8; k++)
			{
				if(((p % width) + Deltas[k].x) < 0 || ((p % width) + Deltas[k].x) > (width - 1) || 
					((int)(p / width) + Deltas[k].y) < 0 || ((int)(p / width) + Deltas[k].y) > (height - 1)) 
					continue;
				int pN = ((int)(p / width) + Deltas[k].y) * width + (p % width) + Deltas[k].x;
				if(*(img + pN))
				{
					if(!(*(labelImg + pN)))
					{
						*(indexBuf + areaSize) = pN;
						*(labelImg + pN) = label;						
						areaSize++;
					}
				}
				else
				{
					if(!(*(labelImg + pN)) && !(*(contureImg + pN)))
					{
						*(indexBuf + size - 1 - contureSize) = pN;
						*(contureImg + pN) = label;
						contureSize++;
					}
				}
			}
		}
	}

	return ret;
}

LONG ImageProcess::LableMarkers(USHORT* imag, USHORT width, USHORT height,int minLabel, USHORT norm, int& roiCounts)
{
	const int Max_Labels = 30000;
	long val = FALSE;

	minLabel = max(1, minLabel);

	IppiSize size = {width, height};//
	int bufferSize;
	IppStatus ok = ippcvDll->ippiLabelMarkersGetBufferSize_16u_C1R(size, &bufferSize);//Computes buffer size for the marker labeling.
	if (ok >= 0)
	{
		Ipp8u *pBuffer = (Ipp8u*) malloc(bufferSize);
		ok = ippcvDll->ippiLabelMarkers_16u_C1IR(imag, width*sizeof(USHORT), size, minLabel, Max_Labels, (IppiNorm)norm, &roiCounts, pBuffer);
		free(pBuffer);
		if (ok >= 0)
		{
			val = TRUE;
		}
	}
	return val;
}

// width and height are counted by pixels
LONG ImageProcess::FindContourPoints(USHORT* srcImg,  USHORT width, USHORT height, USHORT norm, int roiCounts, ULONG** roiPointsArray, USHORT** roiSizeArray)
{
	long val = FALSE;
	const IppiPoint CodeDeltas[8]={{1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}};

	if (norm != ippiNormInf && norm != ippiNormL1)
	{
		//print Error message
	}
	else
	{
		(*roiSizeArray) = (USHORT*)CoTaskMemAlloc(roiCounts * sizeof(USHORT));
		ZeroMemory(*roiSizeArray, roiCounts * sizeof(USHORT));
		ULONG** roiPoints = (ULONG**)CoTaskMemAlloc(roiCounts * sizeof(ULONG*));
		for(int i = 0; i < roiCounts; i++)
		{
			roiPoints[i] = (ULONG*)CoTaskMemAlloc(2 * (width + height - 2) * sizeof(ULONG));
		}

		Ipp16u* dstImg = (Ipp16u*)CoTaskMemAlloc(width * height * sizeof(USHORT));
		Ipp16u* tmpBuf = (Ipp16u*)CoTaskMemAlloc((width + 2) * (height + 2) * sizeof(USHORT)); 
		ZeroMemory(tmpBuf, (width + 2) * (height + 2) * sizeof(USHORT));

		IppiSize tmpROI = {width + 2, height + 2};		
		IppiSize size = {width, height};
		ippiDll->ippiCopyConstBorder_16u_C1R(srcImg, width * 2, size, tmpBuf, tmpROI.width * 2, tmpROI, 1, 1, 0); //add the white border 0.1ms

		Ipp16u value = 0;
		for (int i = 1; i < height + 1; i++)
		{
			int  offset = i * tmpROI.width;
			for (int j = 1; j < width + 1; j++)
			{
				value = tmpBuf[offset + j] & IPP_MAX_16U;
				if( value != 0){ 
					for (int k = 0; k < 8; k += (1 + norm))
					{
						int x = j + CodeDeltas[k].x;
						int y = i + CodeDeltas[k].y;
						if (tmpBuf[y * tmpROI.width + x] != value)//If the points who have value are on the boundary of image, it's boundary pixel
						{
							dstImg[(i - 1) * width + j - 1] = value;
							break;
						}						
					} 
				}

				USHORT roiIndex = dstImg[(i - 1) * width + j - 1];
				if(roiIndex != 0)
				{
					(roiPoints[roiIndex - 1])[(*roiSizeArray)[roiIndex - 1]] = (ULONG)(((j - 1)<<16) + (i - 1));
					roiSizeArray[roiIndex]++;
				}
			}
		}

		int totalPoints = 0;
		for(int k = 0; k < roiCounts; k++)
		{
			totalPoints += (*roiSizeArray)[k];
		}

		(*roiPointsArray) = (ULONG*)CoTaskMemAlloc(totalPoints * sizeof(ULONG));
		int offset = 0;
		for(int k = 1; k < roiCounts; k++)
		{		
			memcpy(roiPointsArray + offset, roiPoints[k], (*roiSizeArray)[k]);
			offset += (*roiSizeArray)[k];
		}

		for(int i = 0; i < roiCounts; i++)
		{
			CoTaskMemFree(roiPoints[i]);
		}

		CoTaskMemFree(roiPoints);
		CoTaskMemFree(dstImg);
		CoTaskMemFree(tmpBuf);

		val = TRUE;
	}
	return val;
}

LONG ImageProcess::FreeContourPointsBuf(ULONG** roiPointsArray, USHORT** roiSizeArray)
{
	long val = TRUE;

	CoTaskMemFree(*roiPointsArray);
	CoTaskMemFree(*roiSizeArray);

	return val;
}


LONG ImageProcess::FindContourImg(USHORT* srcLabelImage, USHORT width, USHORT height, USHORT norm, USHORT* dstContourImage)
{
	long val = TRUE;
	const IppiPoint CodeDeltas[8]={{1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}};

	if ((IppiNorm)norm != ippiNormInf && (IppiNorm)norm != ippiNormL1)
	{
		//print Error message
	}
	else
	{
		//Ipp16u* tmpBuf = (Ipp16u*)CoTaskMemAlloc((width + 2) * (height + 2) * sizeof(USHORT)); 
		//ZeroMemory(tmpBuf, (width + 2) * (height + 2) * sizeof(USHORT));
		Ipp16u* tmpBuf = new Ipp16u[(width+2)*(height+2)];
		IppiSize destROI = {width+2, height+2};	
		IppiSize size = {width, height};
		ippiDll->ippiCopyConstBorder_16u_C1R(srcLabelImage, width * 2,size,tmpBuf,(destROI.width)*2,destROI,1,1,0); //add the white border 0.1ms
		Ipp16u value = 0;
		for (int i = 1; i < height; i++)
		{
			int  offset = i*destROI.width;
			for (int j = 1; j < width; j++)
			{
				value = tmpBuf[offset+j] & IPP_MAX_16U;
				if( value != 0){ 
					for (int k = 0; k < 8; k += (1 + (IppiNorm)norm))
					{
						int x = j + CodeDeltas[k].x;
						int y = i + CodeDeltas[k].y;
						if( tmpBuf[y * destROI.width + x] != value){
							dstContourImage[(i - 1) * width + (j - 1)] = value;
							break;
						}
					} 
				}
			}//end width loop
		}//end height loop
		delete[] tmpBuf;
	}
	return val;
}

LONG ImageProcess::MorphImage(USHORT* imag,USHORT width, USHORT height, MORPH_TYPE morphType)
{
	long val = FALSE;
	if(imag != NULL)
	{
		switch (morphType)
		{
		case MORPH_ERODE:
			val = Erode3x3_16u(imag, width, height);
			break;
		case MORPH_DILATE:
			val = Dilate3x3_16u(imag, width, height);
			break;
		case MORPH_OPEN:
			val = Erode3x3_16u(imag, width, height);
			val = Dilate3x3_16u(imag, width, height);
			break;
		case MORPH_CLOSE:
			val = Dilate3x3_16u(imag, width, height);
			val = Erode3x3_16u(imag, width, height);
			break;
		default://print error
			break;
		}
	}
	return val;
}

LONG ImageProcess::BinarizeImage(USHORT* srcImage, USHORT width, USHORT height, BI_TYPE binarizeType, int minSnr)
{
	long val = FALSE;
	USHORT threshold = 0;
	double sumSi = 0;
	double sumBg = 0;
	double sq_sumBg = 0;
	int bgArea = 0;
	int siArea = 0;
	double averSi = 0;
	double averBg = 0;
	double dev = 0;
	double snr;
	if(srcImage != NULL)
	{
		switch (binarizeType)		
		{
		case BI_OTSU:
			val = GetThreshold_Otsu_16u((USHORT*)srcImage, width*height, threshold);

			for(int i = 0; i < width * height; i++)
			{
				if(*(srcImage + i) < threshold)
				{
					sumBg += *(srcImage + i); 
					sq_sumBg += static_cast<double>((*(srcImage + i))) * (*(srcImage + i));
					bgArea++;
				}
				else
				{
					sumSi += *(srcImage + i); 
					siArea++;
				}
			}
			averSi = sumSi / siArea;
			averBg = sumBg / bgArea;
			dev = sq_sumBg / bgArea - averBg * averBg;
			dev = sqrt(dev);
			snr = 10 * log10(averSi / dev);
			if(snr < minSnr) break;

			val = InRange_inPlace_16u((USHORT*)srcImage, width, height, threshold);
			break;
		case BI_FIXED + BI_GREATER:
			break;
		default:
			break;
		}
	}
	return val;
}

LONG ImageProcess::FilterImage(USHORT* srcImage, USHORT width, USHORT height, int& roicount, FILTER_TYPE filterType, long threshold,int minLabel, long* area)
{
	long val = FALSE;

	USHORT *pixelLUT = new USHORT[roicount];
	minLabel = max(1, minLabel);
	USHORT pixelValue = minLabel;
	if (srcImage != NULL)
	{			
		double mVal = 0;
		switch (filterType)
		{
		case FILTER_AREA + FILTER_GREATER:
			for (int i = 0; i < roicount; i++)
			{
				if (area[i] < threshold)
				{
					pixelLUT[i] = 0;
				}
				else
				{
					pixelLUT[i] = pixelValue;
					pixelValue++;
				}

			}
			roicount = pixelValue - minLabel;
			for (int i = 0; i < width * height; i++)
			{
				if (*(srcImage + i))
				{
					*(srcImage + i) = pixelLUT[*(srcImage + i) - minLabel];
				}
			}
			val = TRUE;
			break;
		case FILTER_AREA + FILTER_LESS:
			break;
		case FILTER_AREA + FILTER_EQUAL:
			break;
		case FILTER_AREA + FILTER_NOT_EQUAL:
			break;
		default:
			break;
		}
	}
	delete[] pixelLUT;
	return val;
}
