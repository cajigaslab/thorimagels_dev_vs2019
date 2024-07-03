#ifndef IDATA_PROCESSING_H
#define IDATA_PROCESSING_H
#pragma once


class ProcessedFrame
{
public:
	ProcessedFrame(UINT width, UINT height, UINT channels, UINT scanAreaID, UINT fullImageWidth, UINT fullImageHeight, UINT top, UINT left) :
		Width(_x), Height(_y), ScanAreaID(_s), ChannelCount(_c) , FullImageWidth(_fullImageWidth), FullImageHeight(_fullImageHeight), Top(_top), Left(_left)
	{
		_x = width;
		_y = height;
		_s = scanAreaID;
		_s = scanAreaID;
		_c = channels;

		_fullImageWidth = fullImageWidth;
		_fullImageHeight = fullImageHeight;
		_top = top;
		_left = left;

		Data = new USHORT[width * height * channels];
		StripesInArea = 0;
	}

	//bool CopyData(USHORT* data, UINT width, UINT height, UINT scanAreaID)
	//{
	//	if (Data == nullptr || data == nullptr || width != width || height != height)
	//	{
	//		return false;
	//	}	
	//	
	//	memcpy(data, Data, width* height*sizeof(USHORT));

	//	return true;
	//}

	void Lock()
	{
		_bufAccessMutex.lock();
	}

	void Unlock()
	{
		_bufAccessMutex.unlock();
	}
	
	~ProcessedFrame()
	{
		delete[] Data;
	}

	size_t GetDataSize()
	{
		return _x * _y * _c* sizeof(USHORT);
	}

	size_t GetDataLengthPerChannel()
	{
		return _x * _y;
	}

	size_t GetDataLength()
	{
		return _x * _y * _c;
	}

	const UINT& Width;
	const UINT& Height;
	const UINT& ScanAreaID;
	const UINT& ChannelCount;
	const UINT& FullImageWidth;
	const UINT& FullImageHeight;
	const UINT& Top;
	const UINT& Left;
	UINT StripesInArea;
	USHORT* Data;	

private:
	UINT _x;
	UINT _y;
	UINT _s;
	UINT _c;
	UINT _fullImageWidth;
	UINT _fullImageHeight;
	UINT _top;
	UINT _left;
	std::mutex _bufAccessMutex;
};


class IDataProcessor
{
public:
	virtual vector<vector<ProcessedFrame*>> ProcessBuffer(UCHAR** pFrmData, UINT numberOfFrames) = 0;
	virtual ~IDataProcessor()
	{
		// Compulsory virtual destructor definition
	}
};

#endif