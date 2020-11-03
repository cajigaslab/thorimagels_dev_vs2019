// TestConsoleCpp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <memory>
#include "..\ThorImageProcess\ImageProcessDll.h"
#include "..\..\..\..\Tools\Intel IPP\intel64\include\IPPlib.h"
#include <iostream>
#include <fstream>
#include <chrono>

using namespace std;

std::auto_ptr<ImageProcessDll> imgProDll(new ImageProcessDll(L"ThorImageProcess.dll"));

//long ImageProcess(USHORT* srcImagePtr, USHORT width, USHORT height, int&roiCount, USHORT* contourImagePtr)
//{
//	long ret = FALSE;
//	USHORT th_low = 0;
//	USHORT th_high = 0;
//	//Get minimal Label Index of Auto Tracking ROI
//	long minLabel = 1;
//	//Binarize
//	if (FALSE == imgProDll->BinarizeImage(srcImagePtr, width, height, BI_OTSU, 15))
//	{
//		return FALSE;
//	}
//	//Morphology
//	if (TRUE)
//	{
//		ret = imgProDll->MorphImage(srcImagePtr, width, height, MORPH_OPEN);
//		ret = imgProDll->MorphImage(srcImagePtr, width, height, MORPH_CLOSE);
//		if (ret == FALSE)
//		{
//			return FALSE;
//		}
//	}
//	//Label ROI
//	if (FALSE == imgProDll->LableMarkers(srcImagePtr, width, height, minLabel, 0, roiCount))
//	{
//		return FALSE;
//	}
//	//Filter
//	long *area = new long[roiCount];
//	memset(area,0,roiCount*sizeof(long));
//	if (true)
//	{
//		ret = imgProDll->FilterImage(srcImagePtr,width,height, roiCount,FILTER_TYPE(FILTER_AREA+FILTER_GREATER), 50, minLabel, area);
//		if (ret == FALSE)
//		{
//			delete[] area;
//			return FALSE;
//		}
//	}
//	delete[] area;
//	//Contour Tracking
//	if (FALSE == imgProDll->FindContourImg(srcImagePtr,width,height,0,contourImagePtr))
//	{
//		return FALSE;
//	}
//	return TRUE;
//}

int _tmain(int argc, _TCHAR* argv[])
{
	//ifstream srcImg("Image_0001_0001.raw", ifstream::binary);
	ifstream srcImg("Image.raw", ifstream::binary);
	ofstream dstImg("bImage.raw", ifstream::binary);
	if(srcImg)
	{
		srcImg.seekg(0, srcImg.end);
		ULONG size = 512 * 512;
		UCHAR* img = (UCHAR*)malloc(512 * 512 * 2);
		srcImg.seekg(0, srcImg.beg);
		srcImg.read((char*)img, 512 * 512 * 2);
		USHORT th;
		USHORT thHigh;
		UCHAR* bImg = (UCHAR*)malloc(512 * 512 * 2);
		long result = 0;
		int roiCount = 0;
		auto start_time = chrono::high_resolution_clock::now();
		//for (int i = 0; i < 1000; i++)
		//{
		//	size = (ULONG)srcImg.tellg();
		//	img = (UCHAR*)malloc(size);
		//	srcImg.seekg(0, srcImg.beg);
		//    srcImg.read((char*)img, size);
		USHORT* imag= (USHORT*) img;
		result = imgProDll->GetThreshold_Otsu_16u((USHORT*)img, 512 * 512, th);
		result = imgProDll->InRange_InPlace_16u((USHORT*)img, 512, 512, th - 1);
		//result = imgProDll->MorphImage((USHORT*)img, 64, 64, MORPH_OPEN);
		//result = imgProDll->MorphImage((USHORT*)img, 64, 64,MORPH_CLOSE);
		USHORT* cnturs = (USHORT*)malloc(512 * 512 * sizeof(USHORT));
		ZeroMemory(cnturs, 512 * 512 * sizeof(USHORT));
		USHORT roiCnts = 500;
		USHORT minRoiArea = 50;
		long t1 = GetTickCount();
		result = imgProDll->LableImage((USHORT*)img, cnturs, 512, 512,  roiCnts, minRoiArea, 2);
		long t2 = GetTickCount();
		result = imgProDll->LableMarkers((USHORT*)img, 512, 512, 1, 0, roiCount);

		result = imgProDll->FindContourImg((USHORT*)img, 64, 64, 0, (USHORT*)bImg);
		long *area = new long[roiCount];
		memset(area, 0, roiCount * sizeof(long));
		result = imgProDll->GetArea((USHORT*)img, 512, 512,roiCount,area,1);
		//result = imgProDll->FilterImage((USHORT*)img,512,512,roiCount,FILTER_TYPE(FILTER_AREA+FILTER_GREATER),50,1, area);
		//result = imgProDll->FindContourImg((USHORT*)img, 512, 512, 0, (USHORT*)bImg);
		long t3 = GetTickCount();
		long tx1 = t2 - t1;
		long tx2 = t3 - t2;
		for (int i = 0; i < roiCount; i++)
		{
			printf("%d \n", area[i]);
		}
		delete[] area;
		MOMENT_TYPE momentType = M_0_0;
		double *momentValue = new double[roiCount];
		result = imgProDll->Moments((USHORT*)img, 512, 512, 1, momentType, momentValue );
		for (int j = 0; j < roiCount; j++)
		{
			printf("%d \n", momentValue[j]);
		}
		//ImageProcess((USHORT*)img, 512, 512, roiCount, (USHORT*)bImg);
		//}
		auto end_time = chrono::high_resolution_clock::now();
		std::cout <<"Time difference is "<< chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count() <<" milliseconds"<<endl;
		printf("The num is %d \n", roiCount);
		//printf("OTSU thershold is %d \n", th);
		dstImg.write((char*)bImg, size);
		srcImg.close();
		dstImg.close();
	}
	return 0;
}

