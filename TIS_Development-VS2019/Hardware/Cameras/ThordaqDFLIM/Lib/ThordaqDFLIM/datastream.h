#ifndef __DATASTREAM_H__
#define __DATASTREAM_H__

#include "buffer.h"

#define PHOTON_VAL_MAX 240
#define PHOTON_VAL_MIN 0

typedef ULONG64 Descriptor_size;

enum PacketType : LONG32
{
	PIXEL_DATA = 1,
	PHOTON_DATA = 2,
	HISTOGRAM_DATA = 4
};

struct PacketDescriptor
{
	PacketType type;
	ULONG64 len;
	PacketDescriptor() : type(PIXEL_DATA), len(0){}
	PacketDescriptor( PacketType type, ULONG64 len) : type(type), len(len){}
};

struct LineDescriptor
{
	LONG64 numOfPkt, pktAddr, dataAddr;
	LineDescriptor() : numOfPkt(0), pktAddr(0), dataAddr(0){}
	LineDescriptor( LONG64 numOfPkt, LONG64 pktAddr, LONG64 dataAddr) : numOfPkt(numOfPkt), pktAddr(pktAddr), dataAddr(dataAddr){}
};

class DataStream
{
private:
	long getLineDescriptorInfo(LineDescriptor*& lineDescriptor, ULONG64 value);
	long getPacketDescriptorInfo(PacketDescriptor*& lineDescriptor, ULONG64 value);
	long processPixelData(const UCHAR* data, FlimBuffer*& buffer, int& pixelIndex, int numPixel);
	long processPhotonData(const UCHAR* data, FlimBuffer*& buffer, int numPhoton);
	long processHistogramData(const UCHAR* data, FlimBuffer*& buffer, int& histIndex, int numHist);
public:
	DataStream(void);
	~DataStream(void);
	long deSerialize(UCHAR* buffer, ULONG32 size, FlimBuffer* acqFlimBuffer);
};

#endif // __DATASTREAM_H__
