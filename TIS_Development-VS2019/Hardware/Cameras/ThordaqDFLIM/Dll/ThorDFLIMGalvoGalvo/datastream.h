#ifndef __DATASTREAM_H__
#define __DATASTREAM_H__

#include "buffer.h"
#include "thordaqcmd.h"
#include <vector>

#define PHOTON_VAL_MAX 240
#define PHOTON_VAL_MIN 0
#define PHOTON_VAL_NEWPIXEL 255
#define PHOTON_VAL_NEWLINE  254

typedef ULONG64 Descriptor_size;

enum PacketType : LONG32
{
	PIXEL_DATA = 1,
	PHOTON_DATA = 2,
	DIAGNOSTIC_DATA = 3,
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
	LONG64 numOfPkt, dataAddr;
	LineDescriptor() : numOfPkt(0), dataAddr(0){}
	LineDescriptor( LONG64 numOfPkt, LONG64 dataAddr) : numOfPkt(numOfPkt), dataAddr(dataAddr){}
};

class DataStream
{
private:
	long getPacketDescriptorInfo(PacketDescriptor*& lineDescriptor, ULONG64 value);
	long processPixelData(const UCHAR* data, const int dataOffset, const int dataSize, FlimBuffer*& buffer, int& pixelIndex, int numPixel, bool flipThisLine, int &linePixelCount);
	long processPhotonData(const UCHAR* data, const int dataOffset, const int dataSize, FlimBuffer*& buffer, int numPhoton, bool flipThisLine);
	long processHistogramData(const UCHAR* data, const int dataOffset, const int dataSize, FlimBuffer*& buffer, int numHist);
	long processFullFrameHistogramData(FlimBuffer*& buffer);
	long processDiagnosticData(const UCHAR* data, const int dataOffset, const int dataSize, FlimBuffer*& buffer, int& pixelIndex, int numPixel);
	ACQUISITION_MODE _aquisitionMode;
	long _scanmode;
	vector<UCHAR> backward_arrival_time_vector;
public:
	DataStream();

	~DataStream(void);
	long deSerialize(UCHAR* buffer, ULONG32 size, ULONG32 numberOfLines, ULONG32 hSize, FlimBuffer*& acqFlimBuffer);
	void SetAcquisitionMode(ACQUISITION_MODE aquisitionMode);
	void SetScanMode(long mode);
};

#endif // __DATASTREAM_H__
