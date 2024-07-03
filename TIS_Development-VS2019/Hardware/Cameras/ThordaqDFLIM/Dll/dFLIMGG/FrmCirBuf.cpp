#include "stdafx.h"
#include "FrmCirBuf.h"

std::mutex FrameCirBuffer::_bufAccessMutex;


FrameCirBuffer::FrameCirBuffer(int pixelX, int pixelY, int channel, size_t numFramesCapacity)
	:_startFrmIndex(0)
	,_endFrmIndex(0)
	,_sizeofFrames(0)
	,_channel(channel)
	,_numFramesCapacity(numFramesCapacity)
{
	//size_t frameTotalSizeInBytes = static_cast<size_t>(_unitFrameInBytes * _capacityofFrames * channel);
	_data = new FlimBuffer[numFramesCapacity]; // Allocate the data buffer

	for (int i = 0; i < _numFramesCapacity; ++i)
	{
		_data[i].setupBuffer(pixelX, pixelY);
	}
}

FrameCirBuffer::~FrameCirBuffer()
{
	SAFE_DELETE_ARRAY(_data);
}

FlimBuffer* FrameCirBuffer::GetEndPtr()
{
	FlimBuffer * ret = NULL;

	_bufAccessMutex.lock();
	ret = &_data[_endFrmIndex];
	_bufAccessMutex.unlock();

	return ret;
}

UINT FrameCirBuffer::FrameBufferSize(size_t* frmSize)
{
	UINT retVal = 0;
	//if(TryEnterCriticalSection(&_bufAccess))
	//{
	_bufAccessMutex.lock();
	//if(_bufAccessMutex.try_lock())
	//{
	*frmSize = _sizeofFrames;
	retVal = 1;
	//	LeaveCriticalSection(&_bufAccess);
	//}
	_bufAccessMutex.unlock();
	//}

	return retVal;	
}

/**********************************************************************************************//**
 * @fn	size_t FrameCirBuffer::ReadFrames(UCHAR *pbuffer, size_t NumFrames)
 *
 * @brief	Read number of frames out of buffer(input have to include channel number).
 *
 * @param	pbuffer			Identifier for the frame buffer.
 * @param 	numFrames	  	Number of the frame to read.
 *
 * @return	The number of frames written.
 **************************************************************************************************/
size_t FrameCirBuffer::ReadFrames(FlimBuffer *pbuffer, size_t NumFrames)
{
	if(0 != (NumFrames % _channel) || (int)NumFrames < _channel)
	{
		return 0; //numFrames must be multiple of channel count
	} 
	if (_sizeofFrames < NumFrames)
	{
		return 0;
	}
	size_t retVal = 0;
	//lock the buffer, disable the write
	_bufAccessMutex.lock();

	size_t frames_to_read = min(NumFrames, _sizeofFrames);
	int offset = static_cast<int>(_startFrmIndex);

	// Read in a single step

	for (int i = 0; i < frames_to_read; ++i)
	{
		memcpy(pbuffer[i].arrival_time_sum_buffer, _data[offset].arrival_time_sum_buffer, _data[offset].arrivalTimeSumBufferSizeBytes);
		memcpy(pbuffer[i].single_photon_buffer, _data[offset].single_photon_buffer, _data[offset].singlePhotonBufferSizeBytes);
		memcpy(pbuffer[i].photon_num_buffer, _data[offset].photon_num_buffer, _data[offset].photonNumBufferSizeBytes);			
		memcpy(pbuffer[i].histogram_buffer, _data[offset].histogram_buffer, _data[offset].histogramBufferSizeBytes);
		pbuffer[i].arrival_time_vector = _data[offset].arrival_time_vector;

		offset++;
		if (offset >= _numFramesCapacity)
		{
			offset = 0;
		}		
	}

	_startFrmIndex += frames_to_read;

	if (_startFrmIndex >= _numFramesCapacity)
	{
		_startFrmIndex = 0;
	}

	_sizeofFrames -= frames_to_read;		
	retVal = frames_to_read;

	_bufAccessMutex.unlock();

	return retVal;	
}

/**********************************************************************************************//**
 * @fn	size_t FrameCirBuffer::WriteFrames(const UCHAR *pbuffer, size_t numFrames)
 *
 * @brief	Write number of frames into buffer.
 *
 * @param	pbuffer			Identifier for the frame buffer.
 * @param 	numFrames	  	Number of the frame to write.
 *
 * @return	The number of frames written.
 **************************************************************************************************/
size_t FrameCirBuffer::WriteFrames(const FlimBuffer *pbuffer, size_t numFrames)
{
	// check 
	if (numFrames <= 0) return 0;
	// lock the buffer, disable read
	_bufAccessMutex.lock();

	size_t frames_to_write = min(numFrames, (_numFramesCapacity - _sizeofFrames)); //if remaining buffer is smaller than the buffer to write, save the size of remaing data buffer 
	size_t offset = static_cast<size_t>(_endFrmIndex); // saving buffer start address

	for (int i = 0; i < frames_to_write; ++i)
	{
		memcpy(_data[offset].arrival_time_sum_buffer, pbuffer[i].arrival_time_sum_buffer, pbuffer[i].arrivalTimeSumBufferSizeBytes);
		memcpy(_data[offset].single_photon_buffer, pbuffer[i].single_photon_buffer, pbuffer[i].singlePhotonBufferSizeBytes);
		memcpy(_data[offset].photon_num_buffer, pbuffer[i].photon_num_buffer, pbuffer[i].photonNumBufferSizeBytes);			
		memcpy(_data[offset].histogram_buffer, pbuffer[i].histogram_buffer, pbuffer[i].histogramBufferSizeBytes);
		_data[offset].arrival_time_vector = pbuffer[i].arrival_time_vector;
		offset++;

		if (offset >= _numFramesCapacity)
		{
			offset = 0;
		}
	}

	_endFrmIndex += frames_to_write;
	if (_endFrmIndex >= _numFramesCapacity) // Circular buffer, if start address equals the last address, start from beginning, overwrite the first data
	{
		_endFrmIndex = 0;
	}

	_sizeofFrames += frames_to_write;
	//Ulock Buffer mutex 
	_bufAccessMutex.unlock();
	return frames_to_write;
}