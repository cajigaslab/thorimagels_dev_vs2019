#include "stdafx.h"
#include "LiveImageData.h"
//#include "HardwareSetupXML.h"
#include "math.h"
#include "limits.h"

extern char * pMemoryBuffer;

int imgWidth  = 1024;
int imgHeight = 1024;

//------------------------------------ rectangle ROI-----------------------------------------------------//
void ImageROIRectStats(unsigned short * pBuffer, long width, long height, long chan,
					   long roiTop,
					   long roiLeft,
					   long roiWidth,
					   long roiHeight,
					   double &meanVal,
					   long &minVal,
					   long &maxVal,
					   double &stdDevVal)
{

	//if(chan > LiveImageData::getInstance()->_copiedChannels)
	//{ 
	//	return;
	//}

	int offset = roiLeft + roiTop * width + chan*width*height;

	double sum = 0;
	long count = 0;
	minVal = USHRT_MAX;
	maxVal = 0;
	meanVal = 0;
	unsigned short *pData = pBuffer + offset;

	// mean (arithmatic)
	for (int y = 0; y < roiHeight; y++)	
	{
		pData = pBuffer + offset;
		for (int x = 0; x < roiWidth; x++)
		{
			unsigned short val = *pData;
			minVal = min(val, minVal);
			maxVal = max(val, maxVal);
			sum += val;
			count++;
			pData++;
		}
		offset += width;
	}

	meanVal = sum / count;

	sum = 0;
	offset = roiLeft + roiTop * width + chan*width*height;

	// std dev
	for (int y = 0; y < roiHeight; y++)	
	{
		pData = pBuffer + offset;
		for (int x = 0; x < roiWidth; x++)
		{
			double val  = *pData - meanVal;
			sum += val*val;
		}
		offset += width;
	}

	stdDevVal = sqrt(sum / count);
}

DllExportLiveImage GetImageROIRectStats(   long roiTop,
								 long roiLeft,
								 long roiWidth,
								 long roiHeight,
								 long channel,
								 double &meanVal,
								 long &minVal,
								 long &maxVal,
								 double &stdDevVal)

{

	if(NULL == pMemoryBuffer)
	{
		return FALSE;
	}

		ImageROIRectStats((unsigned short*)pMemoryBuffer,
		/*LiveImageData::getInstance()->_copiedWidth,
		LiveImageData::getInstance()->_copiedHeight,*/
		imgWidth,
		imgHeight,
		channel,
		roiTop,
		roiLeft,
		roiWidth,
		roiHeight,	
		meanVal,
		minVal,
		maxVal,
		stdDevVal);

	return TRUE;
}

//------------------------------------ line ROI-----------------------------------------------------//
long RetrievesValues(unsigned char* imageBuffer, 
				  long imageWidth, 
				  long imageHeight,
				  long bitdepth,
				  long point1X, 
				  long point1Y, 
				  long point2X, 
				  long point2Y, 				   
				  unsigned char* resultBuffer,long nBuffIndex)
{
	if(point1X < 0)
	{
		point1X = 0;
	}

	if(point1Y < 0)
	{
		point1Y = 0;
	}

	if(point2X < 0)
	{
		point2X = 0;
	}

	if(point2Y < 0)
	{
		point2Y = 0;
	}
	
	long nWidth  =  imageWidth;
	long nHeight =  imageHeight;	

	double x=0, y=0, xDiff=0.0, yDiff=0.0;

	long x1=0,y1=0,x2=0,y2=0,sx=0,sy=0,nCnt=0;	

	long nCurrPix = 0, err=0, e2=0;

	int nColorWidth = 1;

	unsigned short* pSrcBuffer = NULL;
	unsigned short* pDstBuffer = NULL;

	switch(bitdepth)
	{
	case 16:
		{
			pSrcBuffer = (unsigned short*)imageBuffer;
			pDstBuffer = (unsigned short*)resultBuffer;
		}
		break;

	case 24:
		{
			nColorWidth = 3;
		}
		break;
	}	

	x1 = point1X ; y1 = point1Y ;
	x2 = point2X ; y2 = point2Y ;

	xDiff = abs(x2-x1);
	yDiff = abs(y2-y1);
	err	  = static_cast<long>(xDiff - yDiff); 

	long nLength = static_cast<long>(sqrt((xDiff*xDiff)+(yDiff*yDiff)));	

	if (x1 < x2)
		sx = 1;
	else
		sx = -1;

	if (y1 < y2)
		sy = 1;
	else
		sy = -1;

	while(1)
	{
		nCurrPix = (y1 *  nWidth * nColorWidth) + x1;			
		
		if(nCnt < nLength)
		{
			switch(bitdepth)
			{
			case 8:
				{
					resultBuffer[nBuffIndex+nCnt] = imageBuffer[nCurrPix];		
					nCnt++;
				}
				break;

			case 16:
				{
					pDstBuffer[nBuffIndex+nCnt] = pSrcBuffer[nCurrPix];					
					nCnt++;
				}
				break;

			case 24:
				{
					resultBuffer[nBuffIndex+nCnt]   = imageBuffer[nCurrPix];
					resultBuffer[nBuffIndex+nCnt+1] = imageBuffer[nCurrPix+1];
					resultBuffer[nBuffIndex+nCnt+2] = imageBuffer[nCurrPix+2];

					nCnt = nCnt+3;
				}
				break;
			}
		}
		
		if((x1 == x2) && (y1 == y2))
			break;

		e2 = 2 * err;

		if(e2 > -yDiff )
		{
			err = static_cast<long>(err - yDiff);
			x1  = x1 + sx; 
		}

		if(e2 < xDiff)
		{
			err = static_cast<long>(err + xDiff);
			y1  = y1  + sy;
		}
	}

	return nCnt;

}

long  LineProfile(unsigned char* imageBuffer, 
				  long imageWidth, 
				  long imageHeight, 
				  long bitdepth,
				  long point1X, 
				  long point1Y, 
				  long point2X, 
				  long point2Y, 
				  long lineWidth, 
				  unsigned char* resultBuffer)
{	
	long nCount = 0,nTotalPoints = 0,nBuffIndex=0, nWidthParam=lineWidth/2,nIndex=0;

	if((NULL == resultBuffer) || ((lineWidth != 1)&&(0 == nWidthParam)))
	{
		return 0;
	}

	for(nIndex=nWidthParam; nIndex > 0;  nIndex--)
	{
		nCount = RetrievesValues (imageBuffer,imageWidth,imageHeight,bitdepth,point1X,point1Y-nIndex,point2X,point2Y-nIndex,resultBuffer,nTotalPoints);		
		nTotalPoints += nCount;
	}
		
	nCount = RetrievesValues (imageBuffer,imageWidth,imageHeight,bitdepth,point1X,point1Y,point2X,point2Y,resultBuffer,nTotalPoints);		
	nTotalPoints += nCount;

	for(nIndex=0; nIndex < nWidthParam;  nIndex++)
	{
		nCount = RetrievesValues (imageBuffer,imageWidth,imageHeight,bitdepth,point1X,point1Y+nIndex,point2X,point2Y+nIndex,resultBuffer,nTotalPoints);		
		nTotalPoints += nCount;
	}

	return nTotalPoints;
}

DllExportLiveImage GetLineProfile(int imageWidth,
								  int imageHeight,
								  int bitdepth,
								  int channelIndex,
								  int point1X,						// start point
								  int point1Y,
								  int point2X,						// end point
								  int point2Y,
								  int lineWidth,
								  unsigned char* resultBuffer)								          
{

	if(NULL == pMemoryBuffer)
	{
		return FALSE;
	}

	//if(channelIndex > LiveImageData::getInstance()->_copiedChannels)
	//{
	//	return FALSE;
	//}

	int offset = channelIndex*imageWidth*imageHeight*(bitdepth/8);

	return LineProfile( (unsigned char*)(pMemoryBuffer + offset),	
						imageWidth,
						imageHeight,
						bitdepth,
						point1X,
						point1Y,
						point2X,
						point2Y,
						lineWidth,
						resultBuffer);
}

//------------------------------------ polygon ROI-----------------------------------------------------//

	//				    imgWidth
	//	 ___________________________________
	//	|							        |
	//  |roi(minX, minY) ______				|
	//	|				|      |			|
	//	|				| ROI  |roiHeight	|
	//	|				|      |			| imgHeight
	//	|				|______|			|
	//	|				roiWidth			|	
	//	|___________________________________|

void ImageROIPolyStats( unsigned short * pMemoryBuffer,					// image buffer		
						unsigned char * pMaskBuffer,					// mask buffer (image size)
					    long imgWidth,									// image width
						long imgHeight,									// image height   
						long channelIndex,								// current channel			
						long roiMinX,									// ROI left
						long roiMinY,									// ROI right
						long roiWidth,									// ROI width
						long roiHeight,									// ROI top
						double &meanVal,								// mean (arithmatic) of pixel intensity for ROI
						long &minVal,									// minimum pixel intensity in ROI
						long &maxVal,									// maximum pixel intensity in ROI
						double &stdDevVal)								// standard deviation of pixel intensity for ROI
{
	//if(channelIndex > LiveImageData::getInstance()->_copiedChannels)
	//{
	//	return;
	//}

	int imgPtOffset  = roiMinY*imgWidth + roiMinX + channelIndex*imgWidth*imgHeight;
	int maskPtOffset = roiMinY*imgWidth + roiMinX;

	double sum = 0;
	long count = 0;
	minVal  = USHRT_MAX;
	maxVal  = 0;
	meanVal = 0;

	unsigned short *pData = pMemoryBuffer + imgPtOffset;
	unsigned char  *pMask = pMaskBuffer   + maskPtOffset;

	for (int y = 0; y < roiHeight; y++)	// mean (arithmatic)
	{
		pData = pMemoryBuffer + imgPtOffset;
		pMask = pMaskBuffer   + maskPtOffset;
		for (int x = 0; x < roiWidth; x++)
		{			
			if (1 == *pMask)
			{
				unsigned short Val  = *pData;
				
				minVal = min(Val, minVal);
				maxVal = max(Val, maxVal);
				sum += Val;	
				count++;
			}
			
			pData++;
			pMask++;
		}

		imgPtOffset  += imgWidth;
		maskPtOffset += imgWidth;
	}

	meanVal = sum / count;

	sum = 0;	
	imgPtOffset  = roiMinY*imgWidth + roiMinX + channelIndex*imgWidth*imgHeight;
	maskPtOffset = roiMinY*imgWidth + roiMinX;

	for (int y = 0; y < roiHeight; y++)	// std dev
	{
		pData = pMemoryBuffer + imgPtOffset;
		pMask = pMaskBuffer   + maskPtOffset;

		for (int x = 0; x < roiWidth; x++)
		{
			if (1 == *pMask)
			{
				double val  = *pData - meanVal;
				sum += val*val;
			}

			pData++;
			pMask++;
		}

		imgPtOffset  += imgWidth;
		maskPtOffset += imgWidth;
	}

	stdDevVal = sqrt(sum / count);
}

DllExportLiveImage GetImageROIPolyStats(unsigned char * pMaskBuffer,	// mask buffer
										long channelIndex,				// current channel
										long roiMinX,					// ROI left
										long roiMinY,					// ROI right
										long roiWidth,					// ROI width
										long roiHeight,					// ROI height
										double &meanVal,				// ROI mean
										long &minVal,					// ROI min
										long &maxVal,					// ROI max
										double &stdDevVal)				// ROI standard deviation

{
	if(NULL == pMemoryBuffer)
	{
		return FALSE;
	}

	if (NULL == pMaskBuffer )
	{
		return FALSE;
	}	

	//if(channelIndex > LiveImageData::getInstance()->_copiedChannels)
	//{
	//	return FALSE;
	//}
	
	//long imgWidth  = LiveImageData::getInstance()->_copiedWidth;		// image width
	//long imgHeight = LiveImageData::getInstance()->_copiedHeight;		// image height

	int imgPtOffset  = roiMinY*imgWidth + roiMinX + channelIndex*imgWidth*imgHeight;
	int maskPtOffset = roiMinY*imgWidth + roiMinX;

	ImageROIPolyStats( (unsigned short*) pMemoryBuffer,					// image buffer		
						pMaskBuffer,									// mask buffer (image size)
					    imgWidth,										// image width
						imgHeight,										// image height   
						channelIndex,									// current channel			
						roiMinX,										// ROI left
						roiMinY,										// ROI right
						roiWidth,										// ROI width
						roiHeight,										// ROI height
						meanVal,										// mean (arithmatic) of pixel intensity for ROI
						minVal,											// minimum pixel intensity in ROI
						maxVal,											// maximum pixel intensity in ROI
						stdDevVal);										// standard deviation of pixel intensity for ROI

	return TRUE;
}

