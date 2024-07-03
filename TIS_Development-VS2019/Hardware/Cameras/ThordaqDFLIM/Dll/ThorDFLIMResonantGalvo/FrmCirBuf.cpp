#include "stdafx.h"
#include "FrmCirBuf.h"

std::mutex FrameCirBuffer::_bufAccessMutex;


FrameCirBuffer::FrameCirBuffer(int pixelX, int pixelY, int channel, int pixelUnit, size_t _capacityofFrames)
	:_startFrmIndex(0)
	,_endFrmIndex(0)
	,_sizeofFrames(0)
	,_channel(channel)
	,_capacityofFrames(_capacityofFrames)
{
	_unitFrameInBytes = pixelX * pixelY * pixelUnit;
	size_t frameTotalSizeInBytes = static_cast<size_t>(_unitFrameInBytes * _capacityofFrames);
	_data = new UCHAR[frameTotalSizeInBytes]; // Allocate the data buffer
}

FrameCirBuffer::~FrameCirBuffer()
{
	SAFE_DELETE_PTR(_data);
}

UCHAR* FrameCirBuffer::GetEndPtr()
{
	UCHAR * ret = NULL;

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
size_t FrameCirBuffer::ReadFrames(UCHAR *pbuffer, size_t NumFrames)
{
	if(0 != (NumFrames % _channel) || (int)NumFrames < _channel) return 0; //numFrames must be multiple of channel count
	if (_sizeofFrames < NumFrames) return 0;
	size_t retVal = 0;
	//lock the buffer, disable the write
	_bufAccessMutex.lock();

	size_t frames_to_read = std::min(NumFrames, _sizeofFrames);
	size_t bytes_to_read = static_cast<size_t>(_unitFrameInBytes * frames_to_read);
	size_t offset = static_cast<size_t>(_unitFrameInBytes*_startFrmIndex);

	// Read in a single step
	if (frames_to_read <= _capacityofFrames - _startFrmIndex)
	{

		memcpy(pbuffer, _data + offset, bytes_to_read);
		_startFrmIndex += frames_to_read;
		if (_startFrmIndex == _capacityofFrames)
		{
			_startFrmIndex = 0;
		}
	}
	// Read in two steps
	else
	{
		OutputDebugStringW(L"here two step.");
		size_t frameSize1 = _capacityofFrames - _startFrmIndex;
		bytes_to_read = static_cast<size_t>(_unitFrameInBytes*frameSize1);
		offset = static_cast<size_t>(_unitFrameInBytes*_startFrmIndex);
		memcpy(pbuffer, _data + offset, bytes_to_read);

		size_t frameSize2 = frames_to_read - frameSize1;
		bytes_to_read = static_cast<size_t>(_unitFrameInBytes*frameSize2);
		offset = static_cast<size_t>(_unitFrameInBytes*frameSize1);
		memcpy(pbuffer + offset, _data, bytes_to_read);
		_startFrmIndex = frameSize2;
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
size_t FrameCirBuffer::WriteFrames(const UCHAR *pbuffer, size_t numFrames)
{
	// check 
	if (numFrames <= 0) return 0;
	// lock the buffer, disable read
	_bufAccessMutex.lock();

	size_t frames_to_write = std::min(numFrames, (_capacityofFrames - _sizeofFrames)); //if remaining buffer is smaller than the buffer to write, save the size of remaing data buffer 
	size_t bytes_to_write = static_cast<size_t>(_unitFrameInBytes * frames_to_write);
	size_t offset = static_cast<size_t>(_unitFrameInBytes * _endFrmIndex); // saving buffer start address

	// Write in a single step
	if (frames_to_write <= _capacityofFrames - _endFrmIndex) // normal saving 
	{
		memcpy(_data + offset, pbuffer, bytes_to_write);
		_endFrmIndex += frames_to_write;
		if (_endFrmIndex == _capacityofFrames) // Circular buffer, if start address equals the last address, start from beginning
		{
			_endFrmIndex = 0;
		}
	}
	// Write in two steps
	else
	{
		size_t frameSize1 = _capacityofFrames - _endFrmIndex; //if remaining buffer is smaller than the buffer to write, save the size of remaing data buffer 
		bytes_to_write = static_cast<size_t>(_unitFrameInBytes*frameSize1);
		offset = static_cast<size_t>(_unitFrameInBytes*_endFrmIndex);
		memcpy(_data + offset, pbuffer, bytes_to_write);

		size_t frameSize2 = frames_to_write - frameSize1; // writing remaining data from api
		bytes_to_write = static_cast<size_t>(_unitFrameInBytes*frameSize2);
		offset = static_cast<size_t>(_unitFrameInBytes*frameSize1);
		memcpy(_data, pbuffer + offset, bytes_to_write); // write  to the beginning of data buffer
		_endFrmIndex = frameSize2;
	}
	_sizeofFrames += frames_to_write;
	//Ulock Buffer mutex 
	_bufAccessMutex.unlock();
	return frames_to_write;
}