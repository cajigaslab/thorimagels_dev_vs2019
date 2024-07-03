#pragma once
#include "stdafx.h"
#include "ICircularBuffer.h"
#include "mROI/mROIExperimentLoader.h"
class mROICircularBuffer : public ICircularBuffer
{
private:
	size_t _nxtReadFrmIndex;
	size_t _nxtWriteFrmIndex;
	size_t _dmaBufferCount; // total frames allocated = total channels * DMA buffer number
	int _channelCount;             // the number of channel enabled
	//UCHAR* _data;             // frame buffer data
	vector<vector<ProcessedFrame*>> _circularBuffer;
public:
	//Construct circular buffer with capacity of frames including channel number:
	mROICircularBuffer(vector<StripInfo*> imageAreas, int channelCount, int pixelUnit, size_t dmaBufferCount, bool ismROI);
	~mROICircularBuffer();

	//Write number of frames into buffer, return number of frames written:
	size_t WriteFrames(const vector<vector<ProcessedFrame*>> pFrmData, size_t NumFrames);
	//Read number of frames out of buffer(input have to include channel number),
	//return number of frames read:
	long GetNextFrame(vector<ProcessedFrame*> pbuffer);

	void Reset();
};

