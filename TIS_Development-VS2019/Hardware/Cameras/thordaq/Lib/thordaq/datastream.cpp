#include "stdafx.h"
#include "datastream.h"

DataStream::DataStream(void)
{
}


DataStream::~DataStream(void)
{
}

/**********************************************************************************************//**
 * @fn	long DataStream::getLineDescriptorInfo(LineDescriptor*& lineDescriptor, ULONG64 value)
 *
 * @brief	Gets line descriptor information.
 *          Bits 31 downto 0:  	Offset of line data from frame start, in octets
 *		    Bits 47 downto 32:  Offset of packet descriptors from line start, in octets
 *		    Bits 59 downto 48:  Number of packet descriptors (req: 8-9 bits)
 *          Bits 63 downto 60:  reserved
 *
 * @author	Cge
 * @date	6/29/2018
 *
 * @param [in,out]	lineDescriptor	[in,out] If non-null, information describing the line.
 * @param 		  	value		  	The value.
 *
 * @return	The line descriptor information.
 **************************************************************************************************/

long DataStream::getLineDescriptorInfo(LineDescriptor*& lineDescriptor, ULONG64 value)
{
	if (lineDescriptor == nullptr)
	{
		return FALSE;
	}
	lineDescriptor->dataAddr = (value & 0xFFFF) * sizeof(ULONG64);
	lineDescriptor->pktAddr = lineDescriptor->dataAddr + ((value >> 32) & 0xFFFF) * sizeof(ULONG64);
	lineDescriptor->numOfPkt = value >> 48 & 0xFF;
	return TRUE;
}

/**********************************************************************************************//**
 * @fn	long DataStream::getPacketDescriptorInfo(PacketDescriptor*& packetDescriptor, ULONG64 value)
 *
 * @brief	Gets packet descriptor information.
 *			Bits 15 downto 0:  	number of octets in packet
 *			Bits 19 downto 16:  Data type (1=pixel, 2=photon, 3=diagnostic, 4=histogram)
 *			Bits 63 downto 20:  reserved
 *
 * @author	Cge
 * @date	6/29/2018
 *
 * @param [in,out]	packetDescriptor	[in,out] If non-null, information describing the packet.
 * @param 		  	value				The value.
 *
 * @return	The packet descriptor information.
 **************************************************************************************************/

long DataStream::getPacketDescriptorInfo(PacketDescriptor*& packetDescriptor, ULONG64 value)
{
	if (packetDescriptor == nullptr)
	{
		return FALSE;
	}
	packetDescriptor->len = value & 0xFFFF;
	packetDescriptor->type = static_cast<PacketType> ((value >> 16) & 0xFF);
	return TRUE;
}

/**********************************************************************************************//**
 * @fn	long DataStream::processPixelData(const UCHAR* data, FlimBuffer*& buffer, int& pixelIndex, int numPixel)
 *
 * @brief	Process the pixel data.
 *			Bits 23 downto 0: 		unsigned sum over photons of arrival times (req: 17 bits)
 *			Bits 39 downto 24:  	unsigned number of single photons (req: 9 bits)
 *			Bits 55 downto 40:  	unsigned number of total photons in pixel (req: 12 bits)
 *          Bits 63 downto 56:  	reserved
 * @author	Cge
 * @date	6/29/2018
 *
 * @param 		  	data	  	Pointer to the data bufeer.
 * @param [in,out]	buffer	  	[in,out] If non-null, the Flim acquisition buffer.
 * @param [in,out]	pixelIndex	Zero-based index of the pixel.
 * @param 		  	numPixel  	Number of pixels.
 *
 * @return	A long.
 **************************************************************************************************/

long DataStream::processPixelData(const UCHAR* data, FlimBuffer*& buffer, int& pixelIndex, int numPixel)
{
	if (buffer == NULL || buffer->photon_num_buffer_ == nullptr || buffer->single_photon_buffer_ == nullptr || buffer->arrival_time_sum_buffer_ == nullptr)
	{
		return FALSE;
	}
	ULONG64* t = (ULONG64*)data;
	for (int i = 0; i < numPixel; i++)
	{
		buffer->arrival_time_sum_buffer_[pixelIndex] = static_cast<ULONG32>((*(t+i) & 0xffffffff));
		buffer->photon_num_buffer_[pixelIndex] = static_cast<USHORT>((*(t+i) >> 48 & 0xffff));
		buffer->single_photon_buffer_[pixelIndex] = static_cast<USHORT>((*(t+i) >> 32 & 0xffff));
		++pixelIndex;
	}
	return TRUE;
}

/**********************************************************************************************//**
 * @fn	long DataStream::processPhotonData(const UCHAR* data, FlimBuffer*& buffer, int numPhoton)
 *
 * @brief	Process the photon data.
 *			Low-order bytes are historically first.  
 *			Unsigned values 0-240 are arrival times, in bin units (see below).
 *			End-of-pixel mark (255).
 *			Octets are padded (if needed) at the end of a line with pad value (254).

 * @author	Cge
 * @date	6/29/2018
 *
 * @param 		  	data	 	Pointer to the data bufeer.
 * @param [in,out]	buffer   	[in,out] If non-null, the buffer.
 * @param 		  	numPhoton	Number of photons.
 *
 * @return	A long.
 **************************************************************************************************/

long DataStream::processPhotonData(const UCHAR* data, FlimBuffer*& buffer, int numPhoton)
{
	if (buffer == NULL)
	{
		return FALSE;
	}
	
	UCHAR* t = (UCHAR*)data;
	int num = sizeof(ULONG64) / sizeof(UCHAR);

	for (int i = 0; i < numPhoton; i++)
	{
		for (int j = 0; j < num; j++)
		{
			UCHAR photonVal = *(t + i * num + j);

			if (photonVal <= PHOTON_VAL_MAX && photonVal>= PHOTON_VAL_MIN)
			{
				buffer->arrival_time_buffer_.push_back(photonVal);
			}
		}
	}
	
	return TRUE;
}

/**********************************************************************************************//**
 * @fn	long DataStream::processHistogramData(const UCHAR* data, FlimBuffer*& buffer, int& histIndex, int numHist)
 *
 * @brief	Process the histogram data. object comes directly from the 64-octet (512 byte) histogram packets.
 * 			 These arrive for each line as 256 bins of 16 bit (unsigned) integers, but they should be summed over all lines of the frame into 32-bit integers.
 * 			 Bins 0-240 contain the number of photons that arrived at the specified arrival time
 *			 Bins 241-253 are empty.
 *			 Bins 254 and 255 contain clock rate information that can be translated into nsPerBin for the entire frame 
 *
 * @author	Cge
 * @date	6/29/2018
 *
 * @param 		  	data	 	Pointer to the data bufeer.
 * @param [in,out]	buffer   	[in,out] If non-null, the buffer.
 * @param [in,out]	histIndex	Zero-based index of the hist.
 * @param 		  	numHist  	Number of hists.
 *
 * @return	A long.
 **************************************************************************************************/

long DataStream::processHistogramData(const UCHAR* data, FlimBuffer*& buffer, int& histIndex, int numHist)
{
	if (buffer->histogram_buffer_ == nullptr)
	{
		return FALSE;
	}
	USHORT* t = (USHORT*)data;
	int num = sizeof(ULONG64) / sizeof(USHORT);
	for (int i = 0; i < numHist; i++)
	{
		for (int j = 0; j < num; j++)
		{
			buffer->histogram_buffer_[histIndex++] = *(t + numHist * i + j);
		}
	}
	return TRUE;
}

/**********************************************************************************************//**
 * @fn	long DataStream::deSerialize(UCHAR* buffer, ULONG32 size, FlimBuffer* acqFlimBuffer)
 *
 * @brief	DeSerialize this object to the given stream.
 *
 * @author	Cge
 * @date	6/29/2018
 *
 * @param [in,out]	buffer		 	pointer to the buffer.
 * @param 		  	size		 	The size of buffer unit: 8bits.
 * @param [in,out]	acqFlimBuffer	pointer to the FLiM buffer.
 *
 * @return	A long.
 **************************************************************************************************/

long DataStream::deSerialize(UCHAR* buffer, ULONG32 size, FlimBuffer* acqFlimBuffer)
{
	if (acqFlimBuffer == nullptr || buffer == nullptr)
	{
		return FALSE;
	}
	// read last 16bits to get line information
	int numOfLines = (*(buffer + size - 7) << 8) | *(buffer + size - 8);
	ULONG64 descriptorSize =  static_cast<ULONG64>(sizeof(Descriptor_size));
	// get lineDescriptor address
	ULONG64 lineInfoAddr = size - descriptorSize * (numOfLines + 1);
	ULONG64 value = 0;
	LineDescriptor* lineDescriptor = new LineDescriptor();
	PacketDescriptor* packetDescriptor = new PacketDescriptor();
	int pixelIndex = 0;
	int histogramIndex = 0;

	for (int i = 0; i < numOfLines; i++)
	{
		// retrieve line descriptor information
		memcpy(&value, buffer + lineInfoAddr + i * descriptorSize, sizeof(ULONG64));
		if (getLineDescriptorInfo(lineDescriptor,value) == FALSE)
		{
			return FALSE;
		}	
		for (int j = 0; j < lineDescriptor->numOfPkt; j++)
		{
			// retrieve packet descriptor information
			memcpy(&value, buffer + lineDescriptor->pktAddr + j * descriptorSize, sizeof(ULONG64));
			if (getPacketDescriptorInfo(packetDescriptor,value) == FALSE)
			{
				return FALSE;
			}
			switch (packetDescriptor->type)	
			{
			case PacketType::PIXEL_DATA: processPixelData(buffer + lineDescriptor->dataAddr, acqFlimBuffer, pixelIndex, (int)packetDescriptor->len); break;
			case PacketType::PHOTON_DATA: processPhotonData(buffer + lineDescriptor->dataAddr, acqFlimBuffer, (int)packetDescriptor->len); break;
			case PacketType::HISTOGRAM_DATA: processHistogramData(buffer + lineDescriptor->dataAddr, acqFlimBuffer, histogramIndex, (int)packetDescriptor->len);break;
			default:
				break;
			}
			lineDescriptor->dataAddr += packetDescriptor->len * descriptorSize;
		}
	}
	SAFE_DELETE_PTR(lineDescriptor);
	SAFE_DELETE_PTR(packetDescriptor);
	return TRUE;
}