#include "stdafx.h"
#include "dataprocessor.h"


DataProcessor::DataProcessor(BYTE channelEnable, ULONG rawPixelX, ULONG rawPixelY, ULONG linesToAverage, ULONG maxTransferNumberOfFrames, USHORT* datamap[MAX_CHANNEL_COUNT])
{
	_channelCount = 0;
	for (int c = 0; c < MAX_CHANNEL_COUNT; ++c)
	{
		if ((channelEnable & (0x01 << c)) != 0x00)
		{
			++_channelCount;
		}
	}

	_rawFrameLength = rawPixelX * rawPixelY;
	_processedFrameLength = _rawFrameLength / linesToAverage;
	_channelEnable = channelEnable;
	_maxTransferNumberOfFrames = maxTransferNumberOfFrames;
	_rawFrameBufferSize = sizeof(USHORT) * _rawFrameLength * _channelCount;
	_processedFrameBufferSize = sizeof(USHORT) * _processedFrameLength * _channelCount;
	_datamap = datamap;
	_linesToAverage = linesToAverage;
	_rawPixelX = rawPixelX;
	_rawPixelY = rawPixelY;
	_processedPixelX = rawPixelX;
	_processedPixelY = rawPixelY / linesToAverage;
	SetupProcessedBuffer();
}

 DataProcessor::~ DataProcessor()
{
	 DeleteProcessedBuffer();
}

 long DataProcessor::DeleteProcessedBuffer()
 {
	 if (_pFrameData.size() > 0)
	 {
		 for (int i = 0; i < _pFrameData.size(); i++)
		 {
			 SAFE_DELETE_ARRAY(_pFrameData.at(i))
		 }
		 _pFrameData.clear();
	 }
	 return TRUE;
 }

 long DataProcessor::SetupProcessedBuffer()
 {	
	 for (int i = 0; i < static_cast<int>(_maxTransferNumberOfFrames); ++i)
	 {
		 UCHAR* ptr = new UCHAR[_processedFrameBufferSize];
		 _pFrameData.push_back(ptr);
		 if (_pFrameData.at(i) == NULL)
		 {
			 printf("DataProcessor buffer malloc failed at channel %d, Size = %zd\n", i, _processedFrameBufferSize);
			 return FALSE;
		 }
	 }
	 return TRUE;
 }


 vector<UCHAR*> DataProcessor::ProcessBuffer(UCHAR** pFrmDataVector, ULONG numberOfFrames)
 {
	 if (NULL == pFrmDataVector)
	 {
		 return _pFrameData;
	 }
	 for (ULONG f = 0; f < numberOfFrames; ++f)
	 {
		 short* pDataProcessed = (short*)_pFrameData.at(f);
		 int j = 0;
		 for (int c = 0; c < MAX_CHANNEL_COUNT; ++c)
		 {
			 if ((_channelEnable & (0x01 << c)) != 0x00)
			 {
				 if (NULL == pFrmDataVector[c])
				 {
					 ++j;
					 continue;
				 }
				 short* pDataRaw = ((short*)pFrmDataVector[c]) + (size_t)_rawFrameLength * f;

				 for (ULONG y = 0; y < _processedPixelY; ++y)
				 {
					 for (ULONG x = 0; x < _processedPixelX; ++x)
					 {
						 size_t index = j * _processedFrameLength + y * _processedPixelX + x;
						 double sum = 0;
						 for (ULONG l = 0; l < _linesToAverage; ++l)
						 {
							 int rawIndex = _rawPixelX * y * _linesToAverage + _rawPixelX * l + x;

							 if (0 <= *(pDataRaw + rawIndex))
							 {
								 sum += _datamap[c][*(pDataRaw + rawIndex)];
							 }
						 }
						 *(pDataProcessed + index) = (short)round(sum / _linesToAverage);
					 }
				 }
				 ++j;
			 }
		 }
	 }
	 return _pFrameData;
 }