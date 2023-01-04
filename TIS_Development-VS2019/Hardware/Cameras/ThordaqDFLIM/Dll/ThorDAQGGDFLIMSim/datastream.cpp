#include "stdafx.h"
#include "datastream.h"
#include <ctime>

DataStream::DataStream()
{	
	_aquisitionMode = ACQUISITION_MODE::DFLIM;
}

void DataStream::SetAcquisitionMode(ACQUISITION_MODE aquisitionMode)
{
	_aquisitionMode = aquisitionMode;
	_aquisitionMode = ACQUISITION_MODE::DFLIM;
	srand((unsigned) time(0));
}

DataStream::~DataStream()
{
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
 * @update  BGB
 * @date    2/13/2019
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
	packetDescriptor->type = static_cast<PacketType> ((value >> 16) & 0xF);
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
 * @update  BGB
 * @date    2/13/2019
 * @param 		  	data	  	Pointer to the data bufeer.
 * @param [in,out]	buffer	  	[in,out] If non-null, the Flim acquisition buffer.
 * @param [in,out]	pixelIndex	Zero-based index of the pixel.
 * @param 		  	numPixel  	Number of pixels.
 *
 * @return	A long.
 **************************************************************************************************/

long DataStream::processPixelData(const UCHAR* data, const int dataOffset, const int dataSize, FlimBuffer*& buffer, int& pixelIndex, int numPixel)  // TODO: need to add reverseThisLine parameter
{
	if (buffer == NULL || buffer->photon_num_buffer == nullptr || buffer->single_photon_buffer == nullptr || buffer->arrival_time_sum_buffer == nullptr)
	{
		return FALSE;
	}
	ULONG32 signDecide =  static_cast<ULONG32>((rand() % 2));
	LONG32 randomSign = 0 == signDecide ? 1 : -1;
	ULONG randomNumber = 0;//randomSign*static_cast<ULONG32>((rand() % 55));
	ULONG64* t = (ULONG64*)data;
	for (int i = 0; i < numPixel; ++i)  // TODO: if (reverseThisLine) we process the pixels in reverse order (( just make the loop variable j, then: int i = !reverseThisLine ? j : (numPixel-1)-j;  ))
	{
		if (dataOffset + i * sizeof(ULONG64) >= dataSize)
		{
			break;
		}
		if (buffer->arrivalTimeSumBufferSizeBytes <= (pixelIndex)*sizeof(ULONG32))
		{
			return FALSE;
		}
		//ULONG randomNumber = randomSign*static_cast<ULONG32>((rand() % 55));
		buffer->arrival_time_sum_buffer[pixelIndex] = static_cast<ULONG32>((*(t+i) & 0xffffffff) + randomNumber);
		buffer->single_photon_buffer[pixelIndex] = static_cast<USHORT>((*(t+i) >> 32 & 0x1ff) + randomNumber);
		buffer->photon_num_buffer[pixelIndex] = static_cast<USHORT>((*(t+i) >> 48 & 0xfff) + randomNumber);		
		++pixelIndex;
	}
	return TRUE;
}


long DataStream::processDiagnosticData(const UCHAR* data, const int dataOffset, const int dataSize, FlimBuffer*& buffer, int& pixelIndex, int numPixel)
{
	if (buffer == NULL || buffer->photon_num_buffer == nullptr || buffer->single_photon_buffer == nullptr || buffer->arrival_time_sum_buffer == nullptr)
	{
		return FALSE;
	}

	UCHAR* t = (UCHAR*)data;
	for (int i = 0; i < numPixel; ++i)
	{
		if (dataOffset + i * sizeof(UCHAR) >= dataSize)
		{
			break;
		}
		if (buffer->photonNumBufferSizeBytes <= (pixelIndex)*sizeof(USHORT))
		{
			return FALSE;
		}
		buffer->photon_num_buffer[pixelIndex] = static_cast<USHORT>((*(t+i)));		
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

long DataStream::processPhotonData(const UCHAR* data, const int dataOffset, const int dataSize, FlimBuffer*& buffer, int numPhoton) // TODO: need to add reverseThisLine parameter
{
	if (buffer == NULL)
	{
		return FALSE;
	}
	
	UCHAR* t = (UCHAR*)data;
	int num = sizeof(ULONG64) / sizeof(UCHAR);

	for (int i = 0; i < numPhoton; ++i)  // TODO: if (reverseThisLine) we process the photons in reverse order (reversing the order of the beats as well as the bytes within a beat)
	{
		for (int j = 0; j < num; ++j)
		{
			if (dataOffset + i * num + j >= dataSize)
			{
				return TRUE;
			}
			UCHAR photonVal = *(t + i * num + j);

			if (photonVal == PHOTON_VAL_NEWPIXEL || photonVal <= PHOTON_VAL_MAX && photonVal>= PHOTON_VAL_MIN) // gy modified to copy NEWPIXEL marks (but not NEWLINE marks)
			{
				buffer->arrival_time_vector.push_back(photonVal);
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

long DataStream::processHistogramData(const UCHAR* data, const int dataOffset, const int dataSize, FlimBuffer*& buffer, int numHist)
{
	if (buffer->histogram_buffer == nullptr)
	{
		return FALSE;
	}
	USHORT* t = (USHORT*)data;
	int num = sizeof(ULONG64) / sizeof(USHORT);

	if (dataOffset + numHist * sizeof(ULONG64) >= dataSize)
	{
		return FALSE;
	}
	
	ULONG32 signDecide =  static_cast<ULONG32>((rand() % 2));
	LONG32 randomSign = 1;
	LONG32 randomNumber = 0;//static_cast<ULONG32>((rand() % 200)) * randomSign;
	for (int i = 0; i < numHist; ++i)
	{		
		for (int j = 0; j < num; ++j)
		{
			if ((num * i + j) >= 241)
			{
				buffer->histogram_buffer[num * i + j]+=static_cast<USHORT>((*(t + num * i + j)));
			}
			else
			{
				buffer->histogram_buffer[num * i + j]+=static_cast<USHORT>((*(t + num * i + j)) + randomNumber);
			}
		}
	}
	
	return TRUE;
}

/**********************************************************************************************//**
 * @fn	long DataStream::processFullFrameHistogramData(FlimBuffer*& buffer)
 *
 * @brief	Process the histogram data. object comes directly from the 64-octet (512 byte) histogram packets.
 * 			 These arrive for each line as 256 bins of 16 bit (unsigned) integers, but they should be summed over all lines of the frame into 32-bit integers.
 * 			 Bins 0-240 contain the number of photons that arrived at the specified arrival time
 *			 Bins 241-253 are empty.
 *			 Bins 254 and 255 contain clock rate information that can be translated into nsPerBin for the entire frame 
 *
 * @author	BGB
 * @date	5/6/2019

 * @param [in,out]	buffer   	[in,out] If non-null, the buffer.
 *
 * @return	A long.
 **************************************************************************************************/

long DataStream::processFullFrameHistogramData(FlimBuffer*& buffer)
{
	const int BINS = 256;
	for (int i = 0; i < buffer->histogramBufferLength; ++i)
	{
		//data between 241 and 253 should not be summed up
		if (i >= 241 && i <= 253)
		{
			continue;
		}
		for (int j = 0; j < buffer->NumberOflines; ++j)
		{
			buffer->histogram_buffer[i]+= buffer->histogram_raw_buffer[j * BINS + i];
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
 * @update  BGB
 * @date    2/10/2019
 * @param [in,out]	buffer		 	pointer to the buffer.
 * @param 		  	size		 	The size of buffer unit: 8bits.
 * @param [in,out]	acqFlimBuffer	pointer to the FLiM buffer.
 *
 * @return	A long.
 **************************************************************************************************/

long DataStream::deSerialize(UCHAR* buffer, ULONG32 size, ULONG32 numberOfLines, ULONG32 hSize, FlimBuffer*& acqFlimBuffer)  // gy: we will want to add a lineReversal parameter to indicate the need for alternate line reversal
{

	if (acqFlimBuffer == nullptr || buffer == nullptr)
	{
		return FALSE;
	}
	memset(acqFlimBuffer->arrival_time_sum_buffer, 0,acqFlimBuffer->arrivalTimeSumBufferSizeBytes);
	memset(acqFlimBuffer->photon_num_buffer, 0,acqFlimBuffer->photonNumBufferSizeBytes);
	memset(acqFlimBuffer->single_photon_buffer, 0,acqFlimBuffer->singlePhotonBufferSizeBytes);
	memset(acqFlimBuffer->histogram_raw_buffer, 0,acqFlimBuffer->histogramRawBufferSizeBytes);
	memset(acqFlimBuffer->histogram_buffer, 0,acqFlimBuffer->histogramBufferSizeBytes);
	
	acqFlimBuffer->arrival_time_vector.clear();
	switch (_aquisitionMode)
	{
	case ACQUISITION_MODE::DFLIM:
	case ACQUISITION_MODE::Diagnostics:
		{
			// read last 16bits to get line information
			ULONG64 descriptorSize =  static_cast<ULONG64>(sizeof(Descriptor_size));
			LineDescriptor* lineDescriptor = new LineDescriptor();
			PacketDescriptor* packetDescriptor = new PacketDescriptor();
			int pixelIndex = 0;
			int diagnosticPixelIndex = 0;
			int histogramIndex = 0;
			bool photonsWritten = FALSE;

			ULONG64 value = 0;

			for (ULONG32 i = 0; i < numberOfLines; ++i)
			{
				////***Use for correct raw image format****////
				//lineDescriptor->numOfPkt = (buffer[(i + 1) * hSize - 6] & 0xFF) | ((buffer[(i + 1) * hSize - 5] & 0x0F)  << 8);
				//lineDescriptor->dataAddr = hSize*i;
				//ULONG64 packetOffset = (i + 1)*hSize - descriptorSize*lineDescriptor->numOfPkt - 8;
				////***End Use for correct raw image format****////

				////***Use for example raw image format (wrong format)****////
				lineDescriptor->numOfPkt = (buffer[(i + 1) * hSize - 7 - 1] & 0xFF) | ((buffer[(i + 1) * hSize - 6 - 1] & 0x0F)  << 8);
				lineDescriptor->dataAddr = hSize*i;
				ULONG64 packetOffset = (i + 1)*hSize - descriptorSize*lineDescriptor->numOfPkt - 8;
				////***End Use for example raw image format (wrong format)****////
				
				photonsWritten = FALSE; // reset at start of each line
				
				// TODO: bool reverseThisLine = lineReversal && i%2==1;  // even-numbered lines (0,2...) are read normally, but odd-numbered lines are reversed
				// TODO: if (reverseThisLine) we need to assemble a list of positions of all the packets, but then read the packets in reverse order

				for (int j = 0; j < lineDescriptor->numOfPkt; ++j)  
				{
					// retrieve packet descriptor information
					memcpy(&value, buffer + packetOffset, sizeof(ULONG64));
					if (getPacketDescriptorInfo(packetDescriptor,value) == FALSE)
					{
						return FALSE;
					}

					switch (packetDescriptor->type)	
					{
					case PacketType::PIXEL_DATA: 
						{
							if (lineDescriptor->dataAddr + (packetDescriptor->len  + pixelIndex) * sizeof(ULONG64) >= size)
							{
								//break;
							}
							processPixelData(buffer + lineDescriptor->dataAddr, (int)lineDescriptor->dataAddr, size, acqFlimBuffer, pixelIndex, (int)packetDescriptor->len);  // TODO: if (reverseThisLine) we process the pixels in reverse order
						}
						break;
					case PacketType::PHOTON_DATA: 
						{
							if (lineDescriptor->dataAddr + packetDescriptor->len * sizeof(ULONG64) >= size)
							{
								//break;
							}
							processPhotonData(buffer + lineDescriptor->dataAddr, (int)lineDescriptor->dataAddr, size, acqFlimBuffer, (int)packetDescriptor->len);  // TODO: if (reverseThisLine) we process the photons in reverse order
							photonsWritten = TRUE;
						}
						 break;
					case PacketType::HISTOGRAM_DATA:
						{
							if (lineDescriptor->dataAddr + packetDescriptor->len * sizeof(ULONG64) + histogramIndex * sizeof(USHORT) >= size)
							{
								//break;
							}
							processHistogramData(buffer + lineDescriptor->dataAddr, (int)lineDescriptor->dataAddr, size, acqFlimBuffer, (int)packetDescriptor->len);
						}
						break;
					case PacketType::DIAGNOSTIC_DATA:
						{
							if (lineDescriptor->dataAddr + packetDescriptor->len * sizeof(ULONG64) + histogramIndex * sizeof(USHORT) >= size)
							{
								//break;
							}
							processDiagnosticData(buffer + lineDescriptor->dataAddr, (int)lineDescriptor->dataAddr, size, acqFlimBuffer, diagnosticPixelIndex, (int)packetDescriptor->len);
						}
						break;
					default: 
						{
							int x = 0;
							break;
						}
					}
					lineDescriptor->dataAddr += packetDescriptor->len * descriptorSize;
					packetOffset += descriptorSize;
				}
				if (photonsWritten)
				{
					acqFlimBuffer->arrival_time_vector.push_back(PHOTON_VAL_NEWLINE); // this mark is inserted at end of line (regardless of whether we read it forward or backward)
				}
			}

			SAFE_DELETE_PTR(lineDescriptor);
			SAFE_DELETE_PTR(packetDescriptor);

			//processFullFrameHistogramData(acqFlimBuffer);
		}
		break;
	}
	return TRUE;
}