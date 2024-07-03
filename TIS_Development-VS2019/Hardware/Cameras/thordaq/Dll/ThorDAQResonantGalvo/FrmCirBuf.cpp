#include "stdafx.h"
#include "FrmCirBuf.h"

FrameCirBuffer::FrameCirBuffer(int pixelX, int pixelY, int channelCount, int pixelUnit, size_t dmaBufferCount)
	:_nxtReadFrmIndex(0)
	, _nxtWriteFrmIndex(0)
	, _channelCount(channelCount)
	, _dmaBufferCount(dmaBufferCount)
{
	_unitFrameInBytes = pixelX * pixelY * pixelUnit * channelCount;

	for (int i = 0; i < static_cast<int>(dmaBufferCount); ++i)
	{
		ProcessedFrame* pf = new ProcessedFrame(pixelX, pixelY, (UINT)channelCount, 0, pixelX, pixelY, 0, 0);

		vector<ProcessedFrame*> imageAreasData;
		imageAreasData.push_back(pf);

		_circularBuffer.push_back(imageAreasData);
	}
}

FrameCirBuffer::~FrameCirBuffer()
{
	if (_circularBuffer.size() > 0)
	{
		for (int i = 0; i < _circularBuffer.size(); i++)
		{
			for (auto& buffer : _circularBuffer.at(i))
			{
				delete buffer;
			}
			_circularBuffer.at(i).clear();
		}
		_circularBuffer.clear();
	}
}


size_t FrameCirBuffer::FrameBufferSize()
{
	return _unitFrameInBytes;
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
long FrameCirBuffer::GetNextFrame(vector<ProcessedFrame*> pbuffer)
{	
	long retVal = 0;
	size_t bytes_to_read = static_cast<size_t>(_unitFrameInBytes);
	size_t offset = static_cast<size_t>(_unitFrameInBytes * _nxtReadFrmIndex);

	if (pbuffer.size() != _circularBuffer.at(_nxtReadFrmIndex).size())
	{
		return FALSE;
	}
	// Read in a single step
	if (1 <= _dmaBufferCount - _nxtReadFrmIndex)
	{
		for (int i = 0; i < pbuffer.size(); ++i)
		{
			//lock the buffer, disable the write
			_circularBuffer.at(_nxtReadFrmIndex)[i]->Lock();

			memcpy(pbuffer[i]->Data, _circularBuffer.at(_nxtReadFrmIndex)[i]->Data, bytes_to_read);

			_circularBuffer.at(_nxtReadFrmIndex)[i]->Unlock();
		}
		++_nxtReadFrmIndex;
		if (_nxtReadFrmIndex == _dmaBufferCount)
		{
			_nxtReadFrmIndex = 0;
		}
		retVal = TRUE;
	}
	else
	{
		retVal = FALSE;
	}

	return retVal;
}

/**********************************************************************************************//**
 * @fn	size_t FrameCirBuffer::Reset()
 *
 * @brief	reset the next read frame index and next right index.
 *			this will help in not having to rebuild the circular buffer when settings are the same
 **************************************************************************************************/
void FrameCirBuffer::Reset()
{
	_nxtReadFrmIndex = 0;
	_nxtWriteFrmIndex = 0;
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
size_t FrameCirBuffer::WriteFrames(const vector<vector<ProcessedFrame*>> pFrmData, size_t numFrames)
{
	// check 
	if (numFrames <= 0) return 0;

	//size_t frames_to_write = std::min(numFrames, (_dmaBuffers - _sizeofFrames)); //if remaining buffer is smaller than the buffer to write, save the size of remaing data buffer 
	size_t bytes_to_write = static_cast<size_t>(_unitFrameInBytes);  //total number of frames to be write * single frame buffer 
	_nxtWriteFrmIndex; // saving buffer start address

	if (1 <= _dmaBufferCount - _nxtWriteFrmIndex) // normal saving 
	{
		//use numFrames instead of pFrmData.size() because sometimes numFrames could be less the pFrmData.size()
		for (int i = 0; i < numFrames; ++i)
		{

			auto s = pFrmData.at(i);

			if (s.size() != _circularBuffer.at(_nxtWriteFrmIndex).size())
			{
				return 0;
			}

			for (int j = 0; j < s.size(); ++j)
			{
				_circularBuffer.at(_nxtWriteFrmIndex)[j]->Lock();
				memcpy(_circularBuffer.at(_nxtWriteFrmIndex)[j]->Data, s[j]->Data, bytes_to_write);
				_circularBuffer.at(_nxtWriteFrmIndex)[j]->Unlock();
			}
			++_nxtWriteFrmIndex;
			if (_nxtWriteFrmIndex == _dmaBufferCount) // Circular buffer, if start address equals the last address, start from beginning, overwrite the first data
			{
				_nxtWriteFrmIndex = 0;
			}
		}
	}

	return bytes_to_write;
}