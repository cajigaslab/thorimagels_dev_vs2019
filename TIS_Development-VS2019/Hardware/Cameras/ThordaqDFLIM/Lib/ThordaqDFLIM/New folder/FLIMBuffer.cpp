#include "stdafx.h"
#include "buffer.h"

FlimBuffer::FlimBuffer(void)
{
	this->photon_num_buffer_ = nullptr;
	this->histogram_buffer_ = nullptr;
	this->single_photon_buffer_ = nullptr;
	this->arrival_time_sum_buffer_ = nullptr;
	this->arrival_time_buffer_.clear();
}

FlimBuffer::~FlimBuffer(void)
{
	
}

long FlimBuffer::setupBuffer(int numLine, int numPixel)
{
	if (numLine == 0 || numPixel == 0)
	{
		return FALSE;
	}
	photon_num_buffer_ = new USHORT[numLine * numPixel];
	single_photon_buffer_ = new USHORT[numLine * numPixel];
	histogram_buffer_ = new USHORT[numLine * 256];
	arrival_time_sum_buffer_ = new ULONG32[numLine * numPixel];
	
	return TRUE;
}

long FlimBuffer::deleteBuffer()
{
	SAFE_DELETE_ARRAY(photon_num_buffer_);
	SAFE_DELETE_ARRAY(histogram_buffer_);
	SAFE_DELETE_ARRAY(single_photon_buffer_);
	SAFE_DELETE_ARRAY(arrival_time_sum_buffer_);

	return TRUE;
}