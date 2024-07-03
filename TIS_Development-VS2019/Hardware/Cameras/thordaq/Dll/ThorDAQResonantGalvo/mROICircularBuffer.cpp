#include "stdafx.h"
#include "mROICircularBuffer.h"

mROICircularBuffer::mROICircularBuffer(vector<StripInfo*> imageAreas, int channelCount, int pixelUnit, size_t dmaBufferCount, bool ismROI)
	:_nxtReadFrmIndex(0)
	, _nxtWriteFrmIndex(0)
	, _channelCount(channelCount)
	, _dmaBufferCount(dmaBufferCount)
{
	for (int i = 0; i < static_cast<int>(dmaBufferCount); ++i)
	{
		UINT currentScanAreaID = 0;
		UINT areaWidth = 0;
		UINT areaHeight = 0;
		bool areaChanged = true;
		vector<ProcessedFrame*> imageAreasData;
		int top = 0;
		int left = 0;
		bool leftSet = false;
		for (int i = 0; i < imageAreas.size(); ++i)
		{
			currentScanAreaID = imageAreas[i]->ScanAreaID;
		
			UINT fullImageWidth = 0;
			UINT fullImageHeight = 0;

			fullImageWidth = (UINT)floor(imageAreas[i]->FullFOVPhysicalSizeUM / imageAreas[i]->XPixelSize);
			fullImageWidth = (fullImageWidth % 2) != 0 ? fullImageWidth + 1 : fullImageWidth;

			fullImageHeight = (UINT)floor(imageAreas[i]->FullFOVPhysicalSizeUM / imageAreas[i]->XPixelSize);
			fullImageHeight = (fullImageHeight % 2) != 0 ? fullImageHeight + 1 : fullImageHeight;

			if ((imageAreas.size() > i + 1 && currentScanAreaID != imageAreas[i + 1]->ScanAreaID) ||
				(imageAreas.size() == i + 1))
			{

				areaWidth += imageAreas[i]->XSize;
				areaHeight = imageAreas[i]->YSize;				
			
				if (ismROI)
				{								
					top = (int)floor((double)fullImageHeight / 2 + imageAreas[i]->YPos / imageAreas[i]->YPixelSize);

					if (0 == left && !leftSet)
					{
						left = (int)floor((double)fullImageWidth / 2 + (imageAreas[i]->XPos - imageAreas[i]->XPhysicalSize / 2) / imageAreas[i]->XPixelSize);
						leftSet = true;
					}
				}
				else
				{
					fullImageWidth = areaWidth;
					fullImageHeight = areaHeight;
				}

				if (top < 0)
				{
					top = 0;
				}

				if (left < 0)
				{
					left = 0;
				}

				ProcessedFrame* pf = new ProcessedFrame(areaWidth, areaHeight, _channelCount, currentScanAreaID, fullImageWidth, fullImageHeight, top, left);
				imageAreasData.push_back(pf);
				areaWidth = 0;
				areaHeight = 0;
				top = 0;
				left = 0;
				leftSet = false;
			}
			else
			{
				areaWidth += imageAreas[i]->XSize;

				if (0 == left)
				{
					left = (int)floor((double)fullImageWidth / 2 + (imageAreas[i]->XPos - imageAreas[i]->XPhysicalSize / 2) / imageAreas[i]->XPixelSize);
					leftSet = true;
				}
			}
		}
		_circularBuffer.push_back(imageAreasData);
	}
}

mROICircularBuffer::~mROICircularBuffer()
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

/**********************************************************************************************//**
 * @fn	size_t mROICircularBuffer::ReadFrames(UCHAR *pbuffer, size_t NumFrames)
 *
 * @brief	Read number of frames out of buffer(input have to include channel number).
 *
 * @param	pbuffer			Identifier for the frame buffer.
 * @param 	numFrames	  	Number of the frame to read.
 *
 * @return	The number of frames written.
 **************************************************************************************************/
long mROICircularBuffer::GetNextFrame(vector<ProcessedFrame*> pbuffer)
{
	long retVal = 0;

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

			memcpy(pbuffer[i]->Data, _circularBuffer.at(_nxtReadFrmIndex)[i]->Data, _circularBuffer.at(_nxtReadFrmIndex)[i]->GetDataSize());

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
 * @fn	size_t mROICircularBuffer::Reset()
 *
 * @brief	reset the next read frame index and next right index.
 *			this will help in not having to rebuild the circular buffer when settings are the same
 **************************************************************************************************/
void mROICircularBuffer::Reset()
{
	_nxtReadFrmIndex = 0;
	_nxtWriteFrmIndex = 0;
}

/**********************************************************************************************//**
 * @fn	size_t mROICircularBuffer::WriteFrames(const UCHAR *pbuffer, size_t numFrames)
 *
 * @brief	Write number of frames into buffer.
 *
 * @param	pbuffer			Identifier for the frame buffer.
 * @param 	numFrames	  	Number of the frame to write.
 *
 * @return	The number of frames written.
 **************************************************************************************************/
size_t mROICircularBuffer::WriteFrames(const vector<vector<ProcessedFrame*>> pFrmData, size_t numFrames)
{
	// check 
	if (numFrames <= 0) return 0;

	//size_t frames_to_write = std::min(numFrames, (_dmaBuffers - _sizeofFrames)); //if remaining buffer is smaller than the buffer to write, save the size of remaing data buffer 
	size_t bytes_to_write = 0;  //total number of frames to be write * single frame buffer 
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

				memcpy(_circularBuffer.at(_nxtWriteFrmIndex)[j]->Data, s[j]->Data, s[j]->GetDataSize());
				bytes_to_write += s[j]->GetDataSize();

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