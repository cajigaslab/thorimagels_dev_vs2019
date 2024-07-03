#ifndef __FLIMBUFFER_H__
#define __FLIMBUFFER_H__

class FlimBuffer
{
private:
	
public:
	
	
	FlimBuffer(void);
	~FlimBuffer(void);

	long setupBuffer(int numLine, int numPixel);
	long deleteBuffer();

	USHORT* photon_num_buffer_; //number of photons   intensity buffer
	USHORT* single_photon_buffer_; //number of single photons in pixel
	ULONG32* arrival_time_sum_buffer_; //sum over photons of arrival times in pixel
	USHORT* histogram_buffer_; // histogram buffer
	vector<UCHAR> arrival_time_buffer_; //arrival time of single photon
};

#endif // __FLIMBUFFER_H__

