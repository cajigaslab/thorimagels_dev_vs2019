#pragma once
#include "stdafx.h"
#include "ICircularBuffer.h"
//**************************************************************************//
//*** class to create a circular memory feature of frames, allow us		 ***//
//*** to manage buffer between digitizer card and application;  		 ***//
//*** FrameBufferCapacity is to show total frames allocated, and    	 ***//
//*** FrameBufferSize is occupied frame memory. It will not allow to     ***//
//*** write any more once memory is fully occupied, and only read frame  ***//
//*** will clear part of memory for next writing.						 ***//
//**************************************************************************//

class FrameCirBuffer : public ICircularBuffer
{
private:
	size_t _nxtReadFrmIndex;
	size_t _nxtWriteFrmIndex;
	size_t _dmaBufferCount; // total frames allocated = total channels * DMA buffer number
	long _unitFrameInBytes;   // single frame size in Bytes
	int _channelCount;             // the number of channel enabled
	//UCHAR* _data;             // frame buffer data
	vector<vector<ProcessedFrame*>> _circularBuffer;
public:
	//Construct circular buffer with capacity of frames including channel number:
	FrameCirBuffer(int pixelX, int pixelY, int channel, int pixelUnit, size_t dmaBuffersCount);
	~FrameCirBuffer();
	//Return total capacity in frame number:
	size_t FrameBufferCapacity() const { return _dmaBufferCount; }
	//Get current size in frame number:
	size_t FrameBufferSize();
	//Write number of frames into buffer, return number of frames written:
	size_t WriteFrames(const vector<vector<ProcessedFrame*>> pFrmData, size_t NumFrames);
	//Read number of frames out of buffer(input have to include channel number),
	//return number of frames read:
	long GetNextFrame(vector<ProcessedFrame*> pbuffer);

	void Reset();
};
