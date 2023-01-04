#include "stdafx.h"
#include "buffer.h"
#include <new>

FlimBuffer::FlimBuffer(void):
	photon_num_buffer(NULL),
	histogram_buffer(NULL),
	single_photon_buffer(NULL),
	arrival_time_sum_buffer(NULL),
	photonNumBufferSizeBytes(0),
	singlePhotonBufferSizeBytes(0),
	arrivalTimeSumBufferSizeBytes(0),
	histogramBufferSizeBytes(0)
{	
}

FlimBuffer::~FlimBuffer(void)
{
	deleteBuffer();
}

long FlimBuffer::setupBuffer(int numPixel, int numLine)
{
	if (numLine == 0 || numPixel == 0)
	{
		return FALSE;
	}

	arrivalTimeSumBufferLength = numLine * numPixel;
	singlePhotonBufferLength = numLine * numPixel;
	photonNumBufferLength = numLine * numPixel;

	arrivalTimeSumBufferSizeBytes =  arrivalTimeSumBufferLength * sizeof(ULONG32);
	singlePhotonBufferSizeBytes = singlePhotonBufferLength * sizeof(USHORT);
	photonNumBufferSizeBytes = photonNumBufferLength * sizeof(USHORT);
	
	histogramRawBufferLength = 256 * numLine;
	histogramBufferLength = 256;	

	histogramRawBufferSizeBytes = histogramRawBufferLength * sizeof(USHORT);
	histogramBufferSizeBytes = histogramBufferLength * sizeof(ULONG32);

	arrival_time_sum_buffer = new ULONG32[arrivalTimeSumBufferLength]();
	single_photon_buffer = new USHORT[singlePhotonBufferLength]();
	photon_num_buffer = new USHORT[photonNumBufferLength]();
	histogram_raw_buffer = new USHORT[histogramRawBufferLength]();	
	histogram_buffer = new ULONG32[histogramBufferLength]();

	histogram_buffer[252] = static_cast<ULONG32>(numPixel);
	histogram_buffer[253] = static_cast<ULONG32>(numLine);

	pixelsPerLine = numPixel;
	NumberOflines = numLine;

	arrival_time_vector.clear();

	return TRUE;
}

long FlimBuffer::deleteBuffer()
{
	SAFE_DELETE_ARRAY(photon_num_buffer);	
	SAFE_DELETE_ARRAY(single_photon_buffer);
	SAFE_DELETE_ARRAY(arrival_time_sum_buffer);
	SAFE_DELETE_ARRAY(histogram_buffer);
	SAFE_DELETE_ARRAY(histogram_raw_buffer);

	arrival_time_vector.clear();

	return TRUE;
}