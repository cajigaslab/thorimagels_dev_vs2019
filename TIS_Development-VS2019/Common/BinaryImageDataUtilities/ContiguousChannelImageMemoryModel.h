#pragma once
#include "imagememorymodel.h"
#include "ImageIterator.h"
#include "ContiguousMemoryIterator.h"


/// <summary>
/// An image model using a contiguous channel interpretation.
/// <remarks>
/// The ContiguousChannelImageMemoryModel stores pixel information contiguously in memory. Pixels are stored first widthwise, then heightwise.
/// After the width and height of the image is transversed, the channel is incremented. After all channels at a given depth and mosaic are transversed, the mosaic is
/// incremented. Finally, after all mosaics at a given depth are transversed, the depth is incremented. </remarks>
template<typename T> class ContiguousChannelImageMemoryModel : public ImageMemoryModel<T>
{

public:

	//Setup
	ContiguousChannelImageMemoryModel(GenericImage<T>& parent);
	~ContiguousChannelImageMemoryModel();
	virtual void updateParameters(GenericImage<T>& image);
	virtual void calculateOffsets();

	//Image information 
	virtual long long getSizeInBytes();

	//Channel operations 
	virtual void deleteChannels(std::vector<int>& channelsToDelete);

	//Iterator operations
	typedef ImageIterator<T> iterator;
	typedef const ImageIterator<T> const_iterator;
	virtual iterator begin();
	virtual iterator end();
	virtual iterator channelBegin(int channel, int m, int z);
	virtual iterator channelEnd(int channel, int m, int z);


private:

	//Memory offset storage
	long long offsetPerX;
	long long offsetPerYUnit;
	long long offsetPerPlane;
	long long offsetPerChannel;
	long long offsetPerM;
	long long offsetPerZSlice;

	//Direct memory access
	virtual T* getPointerForCoordinates(int x, int y, int plane, int channel, int m, int z);

	//Helper methods for iteration
	bool nextChannel(int& c, int& m, int& z);
	bool nextSourceChannel(int& c, int& m, int& z, std::vector<int>& channelsToDelete);


};
#include "GenericImage.h"


/// <summary> Constructs an ImageMemoryModel with the same image parameters as the input GenericImage </summary>
/// <param name="parent"> GenericImage to copy image parameters from </param>
template <typename T> ContiguousChannelImageMemoryModel<T>::ContiguousChannelImageMemoryModel(GenericImage<T>& parent):
	ImageMemoryModel(parent)
{
	calculateOffsets();
}


template <typename T> ContiguousChannelImageMemoryModel<T>::~ContiguousChannelImageMemoryModel(void)
{
}


/// <summary> Updates offset variables used in calculating pixel addresses </summary>
template <typename T> void ContiguousChannelImageMemoryModel<T>::calculateOffsets()
{

	//Calculate Byte Offsets Per Each Image Dimension
	offsetPerX = 1;
	offsetPerYUnit = width;
	offsetPerPlane = offsetPerYUnit * height;
	offsetPerChannel = offsetPerPlane * numPlanes;
	offsetPerM = offsetPerChannel * channels;
	offsetPerZSlice = offsetPerM * numM;

}


/// <summary> Updates this objects image parameters to reflect the input image </summary>
/// <param name="parent"> GenericImage to copy image parameters from </param>
template <typename T> void ContiguousChannelImageMemoryModel<T>::updateParameters(GenericImage<T>& image)
{
	ImageMemoryModel<T>::updateParameters(image);
	calculateOffsets();
}



/// <summary> Gets a pointer to a the memory location of the requested pixel </summary>
/// <param name="x"> The x coordinate of the requested pixel </param>
/// <param name="y"> The y coordinate of the requested pixel </param>
/// <param name="plane"> The plane of the requested pixel </param>
/// <param name="z"> The z coordinate of the requested pixel </param>
/// <param name="channel"> The channel of the requested pixel </param>
/// <param name="m"> The mosaic number of the requested pixel </param>
/// <returns> Pointer to the pixel value at the specified coordinate </returns>
template <typename T> T* ContiguousChannelImageMemoryModel<T>::getPointerForCoordinates(int x, int y, int plane, int channel, int m, int z)
{


	//Multiply Byte Offsets By Current Position and Combine
	long long thisOffset = x*offsetPerX + y*offsetPerYUnit + plane*offsetPerPlane + channel*offsetPerChannel + m*offsetPerM + z*offsetPerZSlice;
	T* bufPtr = buffer+thisOffset;
	return bufPtr;


}


/// <summary> The total size of memory that this model interpretation covers based on image parameters. Does not speak to actual memory allocated, as that is 
/// outside the preview of ImageMemoryModel classes </summary>
/// <returns> The total amount of data in bytes that this object models </returns>
template <typename T> long long ContiguousChannelImageMemoryModel<T>::getSizeInBytes()
{

	long long pixels = width * height * numPlanes * channels * numM * zSlices; 
	pixels *= bytesPerPixel();
	return pixels;

}


/// <summary> Used to iterating over every pixel, returns a pointer to the beginning of the model </summary>
/// <returns> A pointer to the beginning of the model </returns>
template <typename T> ImageIterator<T> ContiguousChannelImageMemoryModel<T>::begin()
{
	std::shared_ptr<ContiguousMemoryIterator<T> > implementationIterator(new ContiguousMemoryIterator<T>(buffer));
	return ImageIterator<T>(implementationIterator);
}


/// <summary> Used to iterate over every pixel, returns a pointer to the end of the model </summary>
/// <returns> A pointer to the end of the model </returns>
template <typename T> ImageIterator<T> ContiguousChannelImageMemoryModel<T>::end()
{
	std::shared_ptr<ContiguousMemoryIterator<T> > implementationIterator(new ContiguousMemoryIterator<T>(buffer+getSizeInPixels()));
	return ImageIterator<T>(implementationIterator);
}



/// <summary> Used to iterate over every pixel in a channel, returns a pointer to the beginning of the channel. </summary>
/// <param name="channel"> The channel to iterate over </param>
/// <param name="m"> The m of the channel  </param>
/// <param name="z"> The depth of the channel </param>
/// <returns> A pointer to the beginning of the channel </returns
template <typename T> ImageIterator<T> ContiguousChannelImageMemoryModel<T>::channelBegin(int channel, int m, int z)
{
	std::shared_ptr<ContiguousMemoryIterator<T> > implementationIterator(new ContiguousMemoryIterator<T>(buffer + offsetPerChannel*channel + offsetPerM*m + offsetPerZSlice*z));
	return ImageIterator<T>(implementationIterator);
}


/// <summary> Used to iterate over every pixel in a channel, returns a pointer to the end of the channel. </summary>
/// <param name="channel"> The channel to iterate over </param>
/// <param name="m"> The m of the channel  </param>
/// <param name="z"> The depth of the channel </param>
/// <returns> A pointer to the end of the channel </returns>
template <typename T> ImageIterator<T> ContiguousChannelImageMemoryModel<T>::channelEnd(int channel, int m, int z)
{
	std::shared_ptr<ContiguousMemoryIterator<T> > implementationIterator(new ContiguousMemoryIterator<T>(buffer + offsetPerChannel*(channel+1) + offsetPerM*m + offsetPerZSlice*z));
	return ImageIterator<T>(implementationIterator);
}


/// <summary> Deletes the input channels, rearranging memory so that the remaining channels are contiguous in memory </summary>
/// <param name="channelsToDelete"> vector of channel numbers to delete as ints </param>
template <typename T> void ContiguousChannelImageMemoryModel<T>::deleteChannels(std::vector<int>& channelsToDelete)
{

	int cFrom,mFrom,zFrom;
	int cTo,mTo,zTo;
	mFrom=zFrom=mTo=zTo=0;
	cFrom=cTo=-1;

	while(nextChannel(cTo,mTo,zTo) && nextSourceChannel(cFrom,mFrom,zFrom,channelsToDelete))
	{
		T fromVal = *channelBegin(cFrom,mFrom,zFrom);
		T toVal = *channelBegin(cTo,mTo,zTo);
		for(auto fromIt = channelBegin(cFrom,mFrom,zFrom), toIt = channelBegin(cTo,mTo,zTo); toIt != channelEnd(cTo,mTo,zTo); ++fromIt, ++toIt)
		{
			T toItV = *toIt;
			T fromItV = *fromIt;
			*toIt = *fromIt;
		}
	}


}


/// <summary> Helper method for transversing channels in order across depths and mosaics. For example, once the final channel has been reached at a given depth,
/// the channel number is changed to 0, and the depth increases by one. </summary>
/// <param name="c"> Reference of the current channel. Incremented to the next available channel </param>
/// <param name="m"> Reference of the current mosaic. Incremented to the next available channel </param>
/// <param name="z"> Reference of the current depth Incremented to the next available channel </param>
/// <returns> False if there are no more channels, true otherwise </returns>
template <typename T> bool ContiguousChannelImageMemoryModel<T>::nextChannel(int& c, int& m, int& z)
{
	int ctemp=c;
	int mtemp=m;
	int ztemp=z;

	ctemp++;
	if(ctemp>=channels)
	{
		ctemp=0;
		mtemp++;
	}
	if(mtemp>=numM)
	{
		mtemp=0;
		ztemp++;
	}
	if(ztemp>=zSlices)
		return false;

	c=ctemp;
	m=mtemp;
	z=ztemp;

	return true;

}


/// <summary> Helper method for transversing channels in order across depths and mosaics. This function skips channels that are contained in the 
///  channelsToDeleteArgument. For example, once the final channel has been reached at a given depth, the channel number is changed to 0, and the depth increases by one.
/// if the new channel 0 is in the vector argument, the channel is incremented again to 1. </summary>
/// <param name="c"> Reference of the current channel. Incremented to the next available channel </param>
/// <param name="m"> Reference of the current mosaic. Incremented to the next available channel </param>
/// <param name="z"> Reference of the current depth Incremented to the next available channel </param>
/// <param name="channelsToDelete"> Vector containing the channels to delete. </param>
/// <returns> False if there are no more channels, true otherwise </returns>
template <typename T> bool ContiguousChannelImageMemoryModel<T>::nextSourceChannel(int& c, int& m, int& z, std::vector<int>& channelsToDelete)
{
	int ctemp=c;
	int mtemp=m;
	int ztemp=z;

	//=== Move To Next Channel ===
	if(!nextChannel(ctemp,mtemp,ztemp))
		return false;


	//=== Keep Moving If Channel is in Channels To Delete ===
	while(std::find(channelsToDelete.begin(), channelsToDelete.end(), ctemp) != channelsToDelete.end())
	{
		if(!nextChannel(ctemp,mtemp,ztemp))
			return false;
	}

	c=ctemp;
	m=mtemp;
	z=ztemp;

	return true;

}