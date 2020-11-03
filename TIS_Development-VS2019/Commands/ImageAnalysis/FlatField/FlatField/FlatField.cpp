// FlatField.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "math.h"
#include "FlatField.h"

DllExport_FlatField BackgroundCorrection(unsigned short *pBufferData, unsigned short *pBufferCorrection, long w, long h, long c)
{
	unsigned short *pD = pBufferData;
	unsigned short *pB = pBufferCorrection;;

	for(long y=0; y<h; y++)
	{
		for(long x=0; x<w; x++)
		{
			*pD = max(0,*pD - *pB);
			pD++;
			pB++;
		}
	}
	return TRUE;	
}

DllExport_FlatField FlatFieldCorrection(unsigned short *pBufferData, unsigned short *pBufferCorrection, double flatFieldMeanValue, long w, long h, long c)
{
	unsigned short *pD = pBufferData;
	unsigned short *pB = pBufferCorrection;;

	for(long y=0; y<h; y++)
	{
		for(long x=0; x<w; x++)
		{
			if(*pB > 0)
			{
				*pD = static_cast<unsigned short>(flatFieldMeanValue * (*pD)/(double)(*pB));
			}
			else
			{
				*pD = 0;
			}

			pD++;
			pB++;
		}
	}

	return TRUE;
}

DllExport_FlatField FlatFieldAndBackgroundCorrection(unsigned short * pBufferData,unsigned short * pBufferCorrectionBackground,unsigned short * pBufferCorrectionFlatField, double flatFieldMeanValue, long w, long h, long c)
{
	unsigned short *pD = pBufferData;
	unsigned short *pB = pBufferCorrectionBackground;
	unsigned short *pF = pBufferCorrectionFlatField;

	for(long y=0; y<h; y++)
	{
		for(long x=0; x<w; x++)
		{
			if(*pF > 0)
			{
				*pD = static_cast<unsigned short>(flatFieldMeanValue * max(0,*pD-*pB)/(double)(*pF));
			}
			else
			{
				*pD = 0;
			}

			pD++;
			pB++;
			pF++;
		}
	}
	return TRUE;
}