#pragma once

typedef struct _ROI_ELEMENT
{
	double* Mean;
	double* StdDev;
	long*   Max;
	long*   Min;
	long	NumROI;
	long	ChanIndex;
	long*   Frame;
	long*   Z;
	long*   time;
} ROI_ELEMENT, *PROI_ELEMENT;

#define ROI_ELEMENT_MAX_NUM	0xFFFF
#define CHANNEL_NUM 0x4

