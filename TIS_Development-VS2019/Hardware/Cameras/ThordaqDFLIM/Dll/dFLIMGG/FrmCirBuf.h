#pragma once
#include "stdafx.h"
#include "buffer.h"

//**************************************************************************//
//*** class to create a circular memory feature of frames, allow us		 ***//
//*** to manage buffer between digitizer card and application;  		 ***//
//*** FrameBufferCapacity is to show total frames allocated, and    	 ***//
//*** FrameBufferSize is occupied frame memory. It will not allow to     ***//
//*** write any more once memory is fully occupied, and only read frame  ***//
//*** will clear part of memory for next writing.						 ***//
//**************************************************************************//

class FrameCirBuffer
{
private:
	static std::mutex _bufAccessMutex;
	size_t _startFrmIndex;
	size_t _endFrmIndex;
	size_t _sizeofFrames;     // the number of unread frames saved in the buffer
	size_t _numFramesCapacity; // total frames allocated = total channels * DMA buffer number
	//long _unitFrameInBytes;   // single frame size in Bytes
	int _channel;             // the number of channel enabled
	FlimBuffer* _data;             // frame buffer data

public:
	//Construct circular buffer with capacity of frames including channel number:
	FrameCirBuffer(int pixelX, int pixelY, int channel, size_t _capacityofFrames);
	~FrameCirBuffer();
	//Return the last data address:
	FlimBuffer* GetEndPtr();
	//Return total capacity in frame number:
	size_t FrameBufferCapacity() const { return _numFramesCapacity; }
	//Get current size in frame number:
	UINT FrameBufferSize(size_t* frmSize);
	//Write number of frames into buffer, return number of frames written:
	size_t WriteFrames(const FlimBuffer *pbuffer, size_t NumFrames);
	//Read number of frames out of buffer(input have to include channel number),
	//return number of frames read:
	size_t ReadFrames(FlimBuffer *pbuffer, size_t NumFrames);

};
