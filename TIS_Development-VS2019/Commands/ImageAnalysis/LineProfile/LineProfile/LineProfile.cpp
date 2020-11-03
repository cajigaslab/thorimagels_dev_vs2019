
#include "stdafx.h"
#include "LineProfile.h"
#include <math.h>



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
//--------------------------------------------------------------------------------
//!GetLineProfile
//--------------------------------------------------------------------------------
/*!
\param:
\return:	True on successful operation
\brief:		Extract line profile
*/


extern "C"
{__declspec(dllexport) long  LineProfile(unsigned char* imageBuffer,
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
	long nCount = 0, nTotalPoints = 0, nBuffIndex = 0, nWidthParam = lineWidth / 2, nIndex = 0;

	if ((NULL == resultBuffer) || ((lineWidth != 1) && (0 == nWidthParam)))
	{
		return 0;
	}

	for (nIndex = nWidthParam; nIndex > 0; nIndex--)
	{
		nCount = RetrievesValues(imageBuffer, imageWidth, imageHeight, bitdepth, point1X, point1Y - nIndex, point2X, point2Y - nIndex, resultBuffer, nTotalPoints);
		nTotalPoints += nCount;
	}

	nCount = RetrievesValues(imageBuffer, imageWidth, imageHeight, bitdepth, point1X, point1Y, point2X, point2Y, resultBuffer, nTotalPoints);
	nTotalPoints += nCount;

	for (nIndex = 0; nIndex < nWidthParam; nIndex++)
	{
		nCount = RetrievesValues(imageBuffer, imageWidth, imageHeight, bitdepth, point1X, point1Y + nIndex, point2X, point2Y + nIndex, resultBuffer, nTotalPoints);
		nTotalPoints += nCount;
	}

	return nTotalPoints;
}
}

