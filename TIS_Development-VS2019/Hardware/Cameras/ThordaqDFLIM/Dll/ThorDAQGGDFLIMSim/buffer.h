#ifndef __BUFFER_H__
#define __BUFFER_H__

class FlimBuffer
{
private:
	
public:
	FlimBuffer(void);
	~FlimBuffer(void);

	long setupBuffer(int numPixel, int numLine);
	long deleteBuffer();

	size_t photonNumBufferSizeBytes;
	size_t singlePhotonBufferSizeBytes;
	size_t arrivalTimeSumBufferSizeBytes;
	size_t histogramBufferSizeBytes;
	size_t histogramRawBufferSizeBytes;

	long photonNumBufferLength;
	long singlePhotonBufferLength;
	long arrivalTimeSumBufferLength;
	long histogramBufferLength;
	long histogramRawBufferLength;

	bool photonNumBufferCopied;
	bool singlePhotonBufferCopied;
	bool arrivalTimeSumBufferCopied;
	bool histogramBufferCopied;
	bool photonListCopied;

	long pixelsPerLine;
	long NumberOflines;

	USHORT* photon_num_buffer; //number of photons intensity buffer
	USHORT* single_photon_buffer; //number of single photons in pixel
	ULONG32* arrival_time_sum_buffer; //sum over photons of arrival times in pixel
	USHORT* histogram_raw_buffer; //histogram buffer
	ULONG32* histogram_buffer; //histogram buffer
	vector<UCHAR> arrival_time_vector; //arrival time of single photon
};

#endif // __FLIMBUFFER_H__

