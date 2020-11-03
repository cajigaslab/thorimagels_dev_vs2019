// LiveImageData.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "..\..\..\..\Tools\tiff-3.9.4\libtiff\tiff.h"
#include "..\..\..\..\Tools\tiff-3.9.4\libtiff\tiffio.h"
#include <iostream>


char* pMemoryBuffer = NULL;
using namespace std;

extern "C" __declspec(dllexport) long Test()
{
	long a;
	 a = 1;
	 a++;
	 return a;
}

extern "C" __declspec(dllexport) char* LoadTIFF(char *fileName)
{	
	long ret = TRUE;
	TIFF* image;
	tsize_t stripSize;
	unsigned long imageOffset, result;
	int stripMax, stripCount;
	unsigned long bufferSize;
	//wchar_t * path = (wchar_t*)fileName;
	//char* outputBuffer;

	int w,h,b;

	// Open the TIFF image
	if((image = TIFFOpen(fileName, "r")) == NULL)
	{
		cout<<"error opening .tiff image"<<endl;
		ret = FALSE;
	}

	// Read in the possibly multiple strips
	TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &h);
	TIFFGetField(image, TIFFTAG_BITSPERSAMPLE, &b);

	if (NULL != pMemoryBuffer)
	{
		free(pMemoryBuffer);
		pMemoryBuffer = NULL;
	}

	pMemoryBuffer = (char*)malloc(sizeof(char)*w*h*2);

	stripSize = TIFFStripSize(image);
	stripMax = TIFFNumberOfStrips (image);
	imageOffset = 0;

	bufferSize = TIFFNumberOfStrips (image) * stripSize;

	for (stripCount = 0; stripCount < stripMax; stripCount++)
	{
		if((result = TIFFReadEncodedStrip (image, stripCount, pMemoryBuffer + imageOffset, stripSize)) == -1)
		{
			cout<<"error reading .tiff image"<<endl;
			break;
		}

		imageOffset += result;
	}

	// Close the TIFF image
	TIFFClose(image);

	return pMemoryBuffer;  
}