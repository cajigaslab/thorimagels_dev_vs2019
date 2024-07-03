#include "stdafx.h"
#include "MultiPlaneDataProcessor.h"

MultiPlaneDataProcessor::MultiPlaneDataProcessor(BYTE channelEnable, ULONG rawPixelX, ULONG rawPixelY, ULONG linesToAverage, ULONG maxTransferNumberOfFrames, ULONG numberOfPlanes, USHORT* datamap[MAX_CHANNEL_COUNT], ULONG scanMode, ULONG blankLines)
{
	_channelCount = 0;
	for (int c = 0; c < MAX_CHANNEL_COUNT; ++c)
	{
		if ((channelEnable & (0x01 << c)) != 0x00)
		{
			++_channelCount;
		}
	}


	_channelEnable = channelEnable;
	_maxTransferNumberOfFrames = maxTransferNumberOfFrames;
	_numberOfPlanes = numberOfPlanes;
	_rawFrameLength = rawPixelX * rawPixelY;
	_processedFrameLength = _rawFrameLength / linesToAverage;
	_rawPixelX = rawPixelX;
	_rawPixelY = rawPixelY;
	_processedPixelX = rawPixelX;
	_processedPixelY = rawPixelY / linesToAverage;
	_linesToAverage = linesToAverage;
	_scanMode = scanMode;
	_rawFrameBufferSize = sizeof(USHORT) * _rawFrameLength * _channelCount * _numberOfPlanes;
	_processedFrameBufferSize = sizeof(USHORT) * _processedFrameLength * _channelCount * _numberOfPlanes;
	_datamap = datamap;
	_blankLines = blankLines;
	SetupProcessedBuffer();

}

MultiPlaneDataProcessor::~MultiPlaneDataProcessor()
{
	DeleteProcessedBuffer();
}

long MultiPlaneDataProcessor::DeleteProcessedBuffer()
{
	if (_pProcessedData.size() > 0)
	{
		for (int i = 0; i < _pProcessedData.size(); i++)
		{
			SAFE_DELETE_ARRAY(_pProcessedData.at(i))
		}
		_pProcessedData.clear();
	}
	return TRUE;
}

long MultiPlaneDataProcessor::SetupProcessedBuffer()
{
	for (int i = 0; i < static_cast<int>(_maxTransferNumberOfFrames); ++i)
	{
		UCHAR* ptr = new UCHAR[_processedFrameBufferSize];
		_pProcessedData.push_back(ptr);
		if (_pProcessedData.at(i) == NULL)
		{
			printf("DataProcessor buffer malloc failed at channel %d, Size = %zd\n", i, _processedFrameBufferSize);
			return FALSE;
		}
	}
	return TRUE;
}

vector<UCHAR*> MultiPlaneDataProcessor::ProcessBuffer(UCHAR** pFrmData, ULONG numberOfFrames)
{
	if (pFrmData == NULL)
	{
		return _pProcessedData;
	}
	short* pDataProcessed2[MAX_CHANNEL_COUNT];
	
	//raw data comes like this:
	//chanel1pixel1plane1,chanel1pixel1plane2...chanel1pixel1planeN, chanel1pixel2plane1, chanel1pixel2plane2...chanel1pixelNplaneN
	//chanel2pixel1plane1,chanel2pixel2plane2...chanel2pixel2planeN, chanel2pixel2plane1, chanel2pixel2plane2...chanel2pixelNplaneN
	//...
	//...
	//...
	//chanelNpixel1plane1,chanelNpixel2plane2...chanelNpixel2planeN, chanelNpixel2plane1, chanelNpixel2plane2...chanelNpixelNplaneN
	for (size_t f = 0; f < numberOfFrames; ++f)
	{
		short* pDataProcessed = (short*)_pProcessedData.at(f);

		size_t channel = 0;
		long isReverseScan = FALSE;
		size_t planeIndex = 0;
		for (int c = 0; c < MAX_CHANNEL_COUNT; ++c)
		{
			pDataProcessed2[c] = ((short*)_pProcessedData.at(f) + _processedFrameLength*c);
			if ((_channelEnable & (0x01 << c)) != 0x00)
			{
				if (pFrmData[c] == NULL)
				{
					++channel;
					continue;
				}
				short* pDataRaw = ((short*)pFrmData[c]) + _rawFrameLength * f * _numberOfPlanes;

				for (size_t y = 0; y < _processedPixelY; ++y)
				{
					for (size_t x = 0; x < _processedPixelX; ++x)
					{
						size_t processedPixelIndex = y * _processedPixelX + x;
						isReverseScan = (0 == (static_cast<ULONG>(processedPixelIndex / _processedPixelX) % 2)) ? FALSE : TRUE;

						for (size_t plane = 0; plane < _numberOfPlanes; ++plane)
						{
							//In 2-way scanning, the reverse scan will reverse the order of the planes
							//so a forward scan goes Pixel 1, plane 1 (X1P1), followed by Pixel 1, plane 2 (X1P2) and on a reverse it goes X1P2, X1P1  
							//only the planes get reversed not the pixels
							if (isReverseScan && 0 == _scanMode) // 0 is TWO_WAY_SCAN_MODE
							{
								planeIndex = _numberOfPlanes - plane - 1;
							}
							else
							{
								planeIndex = plane;
							}
						
							size_t procesedIndex = planeIndex * _channelCount * _processedFrameLength + channel * _processedFrameLength + processedPixelIndex;
							double sum = 0;

							for (size_t l = 0; l < _linesToAverage; ++l)
							{
								size_t rawIndex = (_rawPixelX * y * _linesToAverage + _rawPixelX * l + x) * _numberOfPlanes + plane;

								if (0 <= *(pDataRaw + rawIndex))
								{
									sum += _datamap[c][*(pDataRaw + rawIndex)];
								}
							}
							pDataProcessed[procesedIndex] = (short)round(sum / _linesToAverage);
						}
					}
				}
				++channel;
			}
		}
	}

	return _pProcessedData;
}