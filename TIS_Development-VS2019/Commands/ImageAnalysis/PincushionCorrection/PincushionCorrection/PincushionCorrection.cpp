// PincushionCorrection.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "math.h"
#include "PincushionCorrection.h"

#define	DBL_EPSILON 1e-10
#define	DOUBLE_EQ(x,v) (((v - DBL_EPSILON) < x) && (x <( v + DBL_EPSILON)))

double currentK1=0;
double currentK2=0;
double currentK3=0;
double currentK4=0;
Point * pLookUp = NULL;
char *pImageResultBuffer = NULL;
long lookUpWidth = 0;
long lookUpHeight = 0;

void BuildLookupTable(long width, long height, double k1, double k2, double k3, double k4)
{	
	if(pLookUp)
	{
		delete pLookUp;
	}

	if(pImageResultBuffer)
	{
		delete pImageResultBuffer;
	}

	//create look up table for the correction resutls
	pLookUp = new Point[width * height];

	//create buffer to store the results of the correction
	pImageResultBuffer = new char[width*height*2];

	double R;
	double Rp;

	long xp;
	long yp;

	Point *pTemp = pLookUp;

	const double PI_CONST = 3.1415926535897932384626433832795;

	for(long y=0; y<height; y++)
	{
		for(long x=0; x<width; x++)
		{
			double xNorm = (x-(width/2))/(double)(width/2);
			double yNorm = (y-(height/2))/(double)(height/2);

			R = sqrt(pow(xNorm,2) + pow(yNorm,2));
			Rp = k1*pow(R,4) + k2*pow(R,3)+ k3*pow(R,2)+ k4*R;  

			xp = 0;
			yp = 0;

			double xd;
			double yd;

			double angle;

			if(x == (width/2))
			{
				if(y < (height/2))
				{
					angle = -1*PI_CONST/2;
				}
				else
				{
					angle = PI_CONST/2;
				}
			}
			else
			{
				angle = atan((double)((y-(height/2))/(double)(x-(width/2))));
			}

			if(x< (width/2))
			{
				xd = min(max(0,(width/2) - Rp * cos(angle) *(width/2)),width);
				yd = min(max(0,(height/2) - Rp * sin(angle)*(height/2)),height);
			}
			else
			{
				xd = min(max(0,(width/2) + Rp * cos(angle)*(width/2)),width);
				yd = min(max(0,(height/2) + Rp * sin(angle)*(height/2)),height);
			}


			xp  = static_cast<long>(floor(xd + .5));
			yp = static_cast<long>(floor(yd + .5));

			pTemp[x + y * width].x = xp;
			pTemp[x + y * width].y = yp;
		}
	}

	lookUpWidth = width;
	lookUpHeight = height;
	currentK1 = k1;
	currentK2 = k2;
	currentK3 = k3;
	currentK4 = k4;
}



DllExport_Pin PincushionCorrection(char * pImageBuffer, long width, long height, long dataType, double k1, double k2, double k3, double k4)
{
	if(		(lookUpWidth != width)||
		(lookUpHeight != height)||
		(FALSE == DOUBLE_EQ(currentK1,k1))||
		(FALSE == DOUBLE_EQ(currentK2,k2))||
		(FALSE == DOUBLE_EQ(currentK3,k3))||
		(FALSE == DOUBLE_EQ(currentK4,k4))||
		(pLookUp == NULL)||
		(pImageResultBuffer == NULL))
	{
		BuildLookupTable(width, height, k1, k2, k3, k4);
	}

	Point pt;
	unsigned short *pSource = (unsigned short*)pImageBuffer;

	unsigned short *pStart = (unsigned short *)pImageResultBuffer;
	unsigned short *p = pStart;

	for(long y=0; y<height; y++)
	{
		for(long x=0; x<width; x++)
		{
			pt = pLookUp[x + y*width];
			*p = pSource[pt.x + pt.y*width];
			p++;
		}
	}

	memcpy(pImageBuffer,pImageResultBuffer,width*height*2);

	return TRUE;
}