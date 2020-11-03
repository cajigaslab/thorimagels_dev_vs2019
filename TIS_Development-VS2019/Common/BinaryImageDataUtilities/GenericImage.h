#pragma once

#include <vector>
#include <sstream>
#include <string.h>
#include <algorithm>
#include "ImageMemoryModel.h"
#include "ChannelManipulator.h"
#include "ImageIterator.h"
#include "InterlacedChannelImageMemoryModel.h"
#include <iostream>


/// <summary>
/// Template based image class used for 3 dimensional, multi-channel, mosaic'd images, with a user specified type for pixel values </summary>
/// <remarks>
/// The GenericImage class contains images many access and manipulation functions, but does not store the image
/// data internally. Instead it needs to be provided a pointer to a buffer containing the image data. The data can be
/// arranged in multiple formats, specified by the memModelType constructor argument. Currently only 
/// formats following a CONTIGUOUS_CHANNEL_MEM_MAP configuration are supported. </remarks>
template <typename T>
class GenericImage
{

public:
	
	/// <summary> MemoryType enum describing the arrangement of memory in an image </summary>
	enum MemoryType
	{
		CONTIGUOUS_CHANNEL, // In order, x y c m z
		INTERLACED_CHANNEL,  // In order, x c y m z
		CONTIGUOUS_CHANNEL_DFLIM_HISTO //dflim Image buffer In order, x y c m z
	};


	//Image setup functions
	GenericImage(int width, int height, int numZSlices, int numChannels, int numM, MemoryType memModelType, const std::vector<int>& enabledChannels = std::vector<int>(), T* buf = NULL);
	virtual ~GenericImage();
	GenericImage(const GenericImage<T>& copyFrom);
	GenericImage<T>& operator = (const GenericImage<T>& assignFrom);
	virtual void setMemoryBuffer(T* buffer);


	//Image copy functions
	long long getSizeInBytes();
	int getSizeInPixels();


	//Image copy functions
	void copyFrom(const GenericImage<T>& fromImage); 
	void copyFromUsingBuffer(const GenericImage<T>& fromImage); 
	void copyChannelFrom(const GenericImage<T>& fromImage, int fromChannel, int toChannel);
	void copyChannelFrom(const GenericImage<T>& fromImage, int fromChannel, int fromZ, int toChannel, int toZ);


	//Image manipulation functions
	void setVal(int x, int y, int z, int channel, int m, T val);
	void averageWith(std::vector<GenericImage<T>*> otherImages);
	void averageWith(std::vector<GenericImage<T>*> otherImages, GenericImage& averagedImage);
	void sumWith(std::vector<GenericImage<T>*> otherImages);
	void sumWith(std::vector<GenericImage<T>*> otherImages, GenericImage& averagedImage);
	void fillWith(T val);
	void deleteChannels(std::vector<int>& channelsToDelete);


	//Image Information Getter Functions
	T getVal(int x, int y, int z, int channel, int m) const;
	int getWidth() const;
	int getHeight() const;
	int getNumChannels() const;
	int getNumEnabledChannels() const;
	int getNumM() const;
	int getNumZSlices() const;
	bool isChannelEnabled(int channel) const;
	bool sameDimensions(const GenericImage<T>& imageToCompare) const;


	//Low Level Image Data Access Function
	T* getDirectPointerToData(int x, int y, int z, int channel, int m);
	typedef ImageIterator<T> iterator;
	typedef const ImageIterator<T> const_iterator;
	iterator begin();
	iterator end();
	iterator channelBegin(int channel, int m, int z);
	iterator channelEnd(int channel, int m, int z);


	//Operator Overloads
	bool operator ==(const GenericImage<T>& other);
	bool operator !=(const GenericImage<T>& other);

	void printImage();
	void printImage(T min, T max);
    char getValChar(T val, T min, T max);

protected:

	//Image Dimensions
	int width;
	int height;
	int numChannels;
	int numM;
	int numZSlices;


	//Image Information
	bool hasDisabledChannels() const;


	//Image Memory Specifications
	MemoryType memoryModelType;
	std::shared_ptr<ImageMemoryModel<T>> memoryModel;
	T* memoryBuffer; 
	std::vector<int> enabledChannels;


	//Memory Safety Checks
	bool valuesWithinBounds(int x, int y, int z, int channel, int m) const;
	std::string getOutOfBoundsErrorMessage(int x, int y, int z, int channel, int m) const;


	//Optimization
	bool optimizedCopyChannelFrom(const GenericImage<T>& fromImage, int fromChannel, int fromZ, int toChannel, int toZ);
	void optimizedCopyChannelFrom_TCON_FCON(const GenericImage<T>& fromImage, int fromChannel, int fromZ, int toChannel, int toZ);
	void optimizedCopyChannelFrom_TCON_FINT(const GenericImage<T>& fromImage, int fromChannel, int fromZ, int toChannel, int toZ);
	void optimizedCopyChannelFrom_TINT_FCON(const GenericImage<T>& fromImage, int fromChannel, int fromZ, int toChannel, int toZ);
	void optimizedCopyChannelFrom_TINT_FINT(const GenericImage<T>& fromImage, int fromChannel, int fromZ, int toChannel, int toZ);
	void regularCopyChannelFrom(const GenericImage<T>& fromImage, int fromChannel, int fromZ, int toChannel, int toZ);

	void regularAverageWith(std::vector<GenericImage<T>*> otherImages, GenericImage<T>& averagedImage);
	void optimizedAverageWith(std::vector<GenericImage<T>*> otherImages, GenericImage<T>& averagedImage);
	void regularSumWith(std::vector<GenericImage<T>*> otherImages, GenericImage<T>& averagedImage);
	void optimizedSumWith(std::vector<GenericImage<T>*> otherImages, GenericImage<T>& averagedImage);

	//Swap
	void swap(GenericImage<T>& fromImage);



private:

	//Image memory functions
	std::shared_ptr<ImageMemoryModel<T> > getImageMemoryModel(MemoryType memModelType);



};


//==============================================
//        Begin Template Definition
//==============================================

#include "ContiguousChannelImageMemoryModel.h"
#include "InternallyStoredImage.h"


///<summary> Constructs a 5 dimensional image class. The dimensions are width, height, depth, channel, and M </summary>
///<param name="width"> The width in pixels for this image </param>
///<param name="height"> The width in pixels for this image </param>
///<param name="numZSlices"> The depth in pixels of this image </param>
///<param name="numChannels"> The number of channels for this image </param>
///<param name="numM"> the length of the 'm' dimension in pixels for this image </param>
///<param name="enabledChannels"> Optional vector containing the enabled channels in this image. If a value is given, the total channel parameters will remain the same, but only the enabled channels will be stored in memory </param>
template <typename T> GenericImage<T>::GenericImage(int width, int height, int numZSlices, int numChannels, int numM, MemoryType memModelType, const std::vector<int>& enabledChannels, T* buf)

{

	//Basic parameters
	this->width = width;
	this->height = height;
	this->numChannels = numChannels;
	this->numM = numM;
	this->numZSlices = numZSlices;


	//Make sure enabled channels vector is sorted and unique
	this->enabledChannels = enabledChannels;
	std::sort(this->enabledChannels.begin(), this->enabledChannels.end());
	auto value = std::unique(this->enabledChannels.begin(), this->enabledChannels.end());


	//Get memory model
	memoryModel = getImageMemoryModel(memModelType);
	this->memoryModelType = memModelType;

	if (NULL != buf)
		this->setMemoryBuffer(buf);
}


template <typename T> GenericImage<T>::~GenericImage()
{
}
 

/// <summary> Gets the value at the specified pixel </summary>
/// <param name="x"> Width coordinate of pixel </param>
/// <param name="y"> Height coordinate of pixel </param>
/// <param name="z"> Depth coordinate of pixel</param>
/// <param name="channel"> Channel of pixel</param>
/// <param name="m"> M of pixel</param>
/// <returns> Value at the specified pixel </returns>
template <typename T> T GenericImage<T>::getVal(int x, int y, int z, int channel, int m) const
{

	if(!isChannelEnabled(channel))
		return T();
	if(hasDisabledChannels())
		channel = ChannelManipulator<T>::getSequentialMemoryChannel(channel,enabledChannels);


	if(valuesWithinBounds(x,y,z,channel,m))
	{
		T val = memoryModel->getVal(x,y,z,channel,m); 
		return val;
	}
	else
	{
		throw std::out_of_range(getOutOfBoundsErrorMessage(x,y,z,channel,m));
	}

}


/// <summary> Sets the value at the specified pixel </summary>
/// <param name="x"> Width coordinate of pixel </param>
/// <param name="y"> Height coordinate of pixel </param>
/// <param name="z"> Depth coordinate of pixel</param>
/// <param name="channel"> Channel of pixel</param>
/// <param name="m"> M of pixel</param>
/// <param name="val"> The new value of the pixel </param>
template <typename T> void GenericImage<T>::setVal(int x, int y, int z, int channel, int m, T val)
{
	if(!isChannelEnabled(channel))
		return;
	if(hasDisabledChannels())
		channel = ChannelManipulator<T>::getSequentialMemoryChannel(channel,enabledChannels);

	if(valuesWithinBounds(x,y,z,channel,m))
	{
		memoryModel->setVal(x,y,z,channel,m,val);
	}
	else
	{
		throw std::out_of_range(getOutOfBoundsErrorMessage(x,y,z,channel,m));
	}
}


/// <summary> Gets the pointer to the location in memory of the value corresponding to the selected coordinates </summary>
/// <param name="x"> Width coordinate to be accessed </param>
/// <param name="y"> Height coordinate to be accessed </param>
/// <param name="z"> Depth coordinate to be accessed </param>
/// <param name="channel"> Channel to be accessed </param>
/// <param name="m"> M to be accessed </param>
/// <returns> Pointer to the data at the specified coordinates </returns>
template <typename T> T* GenericImage<T>::getDirectPointerToData(int x, int y, int z, int channel, int m)
{
	if(hasDisabledChannels())
		channel = ChannelManipulator<T>::getSequentialMemoryChannel(channel,enabledChannels);

	return memoryModel->getPointerForCoordinates(x,y,z,channel,m);
}

/// <summary> Returns a smart pointer to a MemoryModel object that can be used to interpret the data stored in the buffer </summary>
/// <param name="memModelType"> Enumeration defining the binary format the data is stored in </param>
template <typename T> std::shared_ptr<ImageMemoryModel<T> > GenericImage<T>::getImageMemoryModel(MemoryType memModelType)
{
	switch(memModelType)
	{

	case CONTIGUOUS_CHANNEL:
		return std::auto_ptr<ImageMemoryModel<T> >(new ContiguousChannelImageMemoryModel<T>(*this));
		break;

	case INTERLACED_CHANNEL:
		return std::auto_ptr<ImageMemoryModel<T> >(new InterlacedChannelImageMemoryModel<T>(*this));
		break;

	default:
		return std::auto_ptr<ImageMemoryModel<T> >(new ContiguousChannelImageMemoryModel<T>(*this));
		break;

	}

}


/// <summary> Returns the size that this image takes up in memory </summary>
/// <returns> The size in bytes </returns>
template <typename T> long long GenericImage<T>::getSizeInBytes()
{
	long long sizeInBytes = memoryModel->getSizeInBytes();	
	return sizeInBytes;
}


/// <summary> Returns the total number of pixels in this image, accounting for width, height, depth, channels, and mosaics </summary>
/// <returns> The total number of pixels in this image </returns>
template <typename T> int GenericImage<T>::getSizeInPixels()
{
	return memoryModel->getSizeInPixels();
}


/// <summary> Sets the pointer for the start of the memory buffer used by this image </summary>
/// <param name="buffer"> A pointer to the start of the memory buffer containing the data for this image </param>
template <typename T> void GenericImage<T>::setMemoryBuffer(T* buffer)
{
	memoryModel->setBuffer(buffer);
}


/// <returns> The width of this image in pixels </returns>
template <typename T> int GenericImage<T>::getWidth() const
{
	return width;
}


/// <returns> The height of this image in pixels </returns>
template <typename T> int GenericImage<T>::getHeight() const
	{
	return height;
}


/// <returns> The number of channels, both enabled and disabled, in this image </returns>
template <typename T> int GenericImage<T>::getNumChannels() const
{
	return numChannels;
}


/// <returns> The number of enabled channels in this image </returns>
template <typename T> int GenericImage<T>::getNumEnabledChannels() const
{
	if(hasDisabledChannels())
		return static_cast<int>(enabledChannels.size());
	else
		return getNumChannels();
}


/// <returns> The number of mosaics in this image  </returns>
template <typename T> int GenericImage<T>::getNumM() const
{
	return numM;
}


/// <returns> The number of z slices taken </returns>
template <typename T> int GenericImage<T>::getNumZSlices() const
{
	return numZSlices;
}

/// <summary> Checks to see if the requested pixel is within the bounds of this image </summary>
/// <param name="x"> Width coordinate trying to be accessed </param>
/// <param name="y"> Height coordinate trying to be accessed </param>
/// <param name="z"> Depth coordinate trying to be accessed </param>
/// <param name="channel"> Channel trying to be accessed </param>
/// <param name="m"> M trying to be accessed </param>
/// <returns> True if the coordinates are within the image, False otherwise </returns>
template <typename T> bool GenericImage<T>::valuesWithinBounds(int x, int y, int z, int channel, int m) const
{

	if(x < 0 || x > getWidth()-1)
	{
		return false;
	}
	else if(y < 0 || y > getHeight()-1)
	{
		return false;
	}
	else if(z < 0 || z > getNumZSlices()-1)
	{
		return false;
	}
	else if(channel < 0 || channel > getNumChannels()-1)
	{
		return false;
	}
	else if(m < 0 || m > getNumM()-1)
	{
		return false;
	}

	return true;

}

/// <summary> Gets an string describing the specifics of an out of bounds access error </summary>
/// <param name="x"> Width coordinate trying to be accessed </param>
/// <param name="y"> Height coordinate trying to be accessed </param>
/// <param name="z"> Depth coordinate trying to be accessed </param>
/// <param name="channel"> Channel trying to be accessed </param>
/// <param name="m"> M trying to be accessed </param>
/// <returns> String describing the reasons for an out of bounds condition </returns>
template <typename T> std::string GenericImage<T>::getOutOfBoundsErrorMessage(int x, int y, int z, int channel, int m) const
{

	std::stringstream errorMessage;
	errorMessage << "Requested Pixel Outside Of Image";

	if(x < 0)
		errorMessage << " ... x < 0";
	if(x > getWidth()-1)
		errorMessage << " ... x > width:" << getWidth()-1;
	if(y < 0 )
		errorMessage << " ... y < 0";
	if(y > getHeight()-1)
		errorMessage << " ... y > height:" << getHeight()-1;
	if(z < 0)
		errorMessage << " ... z < 0";
	if(z > getNumZSlices()-1)
		errorMessage << " ... z > maxZ:" << getNumZSlices()-1;
	if(channel < 0)
		errorMessage << " ... c < 0";
	if(channel > getNumChannels()-1)
		errorMessage << " ... z > numChannels:" << getNumChannels()-1;
	if(m < 0)
		errorMessage << " ... m < 0";
	if(m > getNumM()-1)
		errorMessage << " ... z > maxM:" << getNumM()-1;

	return errorMessage.str();


}


/// <summary> Copies all pixels from one image into this image, using a intermediary buffer. Can be used to safely copy a source image that has an overlapping address space with the destination
///           , eliminating the risk of copying data that has already been overwritten in the copy </summary>
/// <param name="fromImage"> The source image for the data to be copied into this image </param>
template <typename T> void GenericImage<T>::copyFromUsingBuffer(const GenericImage<T>& fromImage)
{

	InternallyStoredImage<T> buffer(getWidth(), getHeight(), getNumZSlices(), getNumChannels(), getNumM(), memoryModelType, enabledChannels);

	buffer.copyFrom(fromImage);

	copyFrom(buffer);

}

/// <summary> Fills all pixels in this image with a value </summary>
/// <param name="fromImage"> The value to fill this image with </param>
template <typename T> void GenericImage<T>::fillWith(T val)
{
	std::fill(begin(), end(), val);
}


/// <summary> Copies all pixels from one image into this image </summary>
/// <param name="fromImage"> The source image for the data to be copied into this image </param>
template <typename T> void GenericImage<T>::copyFrom(const GenericImage<T>& fromImage)
{

	for(int channel=0; channel<getNumChannels(); channel++)
	{
		copyChannelFrom(fromImage,channel,channel);
	}

}


/// <summary> Copies a channel at the specified depths from an image to this image </summary>
/// <param name="fromImage"> The source image which contains the channel to be copied from </param>
/// <param name="fromChannel"> The source channel in the 'fromImage' to be copied </param>
/// <param name="fromZ"> The specific depth of the 'fromImage' channel to be copied </param>
/// <param name="toChannel"> The channel to be copied into in this image </param>
/// <param name="toZ"> The depth of the channel to be copied into in this image </param>
template <typename T> void GenericImage<T>::copyChannelFrom(const GenericImage<T>& fromImage, int fromChannel, int fromZ, int toChannel, int toZ)
{
	
	// No need to copy if the channel is disabled in either image, as the data will be garbage or overwritten
	if(!isChannelEnabled(toChannel) || !fromImage.isChannelEnabled(fromChannel))
		return;


	if(!optimizedCopyChannelFrom(fromImage,fromChannel,fromZ,toChannel,toZ))
		regularCopyChannelFrom(fromImage,fromChannel,fromZ,	toChannel,toZ);

}


/// <summary> Copies a channel from an image to this image </summary>
/// <param name="fromImage"> The source image which contains the channel to be copied from </param>
/// <param name="fromChannel"> The source channel in the 'fromImage' to be copied </param>
/// <param name="toChannel"> The channel to be copied into in this image </param>
template <typename T> void GenericImage<T>::copyChannelFrom(const GenericImage<T>& fromImage, int fromChannel, int toChannel)
{
	for(int m=0; m<numM; m++)
	{
		for(int z=0; z<numZSlices; z++)
		{
			copyChannelFrom(fromImage, fromChannel, z, toChannel, z);
		}
	}

}


/// <summary> Averages this image with the input images, storing in this image's memory </summary>
/// <param name="otherImages"> A std::vector of pointers to other GenericImages that will be averaged with this one. These images must be the
///                            the same size as this image. </param>
template <typename T> void GenericImage<T>::averageWith(std::vector<GenericImage<T>*> otherImages)
{

	averageWith(otherImages, *this);

}

/// <summary> Averages this image with the input images, storing in the input image's memory </summary>
/// <param name="otherImages"> A std::vector of pointers to other GenericImages that will be averaged with this one. These images must be the
///                            the same size as this image. </param>
/// <param name="averagedImage"> Reference to generic image to store the result of the average in. </param>
template <typename T> void GenericImage<T>::averageWith(std::vector<GenericImage<T>*> otherImages, GenericImage<T>& averagedImage)
{

	//Check if each image has the same memory type
	for(GenericImage<T>* otherImage : otherImages)
	{
		if(averagedImage.memoryModelType != otherImage->memoryModelType)
		{
			regularAverageWith(otherImages, averagedImage);
			return;
		}
	}

	//All images have the same type, use optimized function 
	optimizedAverageWith(otherImages, averagedImage);

}

/// <summary> Averages this image with the input images, storing in the input image's memory. This function uses iterators
///           and is able to average images with different memory types
/// </summary>
/// <param name="otherImages"> A std::vector of pointers to other GenericImages that will be averaged with this one. These images must be the
///                            the same size as this image. </param>
/// <param name="averagedImage"> Reference to generic image to store the result of the average in. </param>
template <typename T> void GenericImage<T>::regularAverageWith(std::vector<GenericImage<T>*> otherImages, GenericImage<T>& averagedImage)
{

	//Get image iterators to iterate over
	std::vector<GenericImage<T>::iterator > imageIterators = std::vector<GenericImage<T>::iterator >();
	for(GenericImage* otherImage : otherImages)
	{
		auto it = otherImage->begin();
		imageIterators.push_back(it);
	}
	imageIterators.push_back(begin());


	//Iterate over each image
	auto end = averagedImage.end();
	for(auto toIt = averagedImage.begin(); toIt != end; ++toIt)
	{
		__int64 sum = 0;
		for(auto it=imageIterators.begin(); it!=imageIterators.end(); ++it)
		{
			sum += static_cast<T>(**it);
			++(*it);
		}
		*toIt = static_cast<T>(sum/(imageIterators.size()));
	}

}


/// <summary> Averages this image with the input images, storing in the input image's memory. Uses a direct memory approach that is
///           only compatible if all images share the same memory type
/// </summary>
/// <param name="otherImages"> A std::vector of pointers to other GenericImages that will be averaged with this one. These images must be the
///                            the same size as this image and all have the same memory type </param>
/// <param name="averagedImage"> Reference to generic image to store the result of the average in. </param>
template <typename T> void GenericImage<T>::optimizedAverageWith(std::vector<GenericImage<T>*> otherImages, GenericImage<T>& averagedImage)
{
	//Get pointers to destination buffer
	T* destinationBuffer = &(*averagedImage.begin());
	T* thisBuffer= &(*begin());

	//Get pointers to each image's buffer
	std::vector<T*> buffers(otherImages.size());
	std::transform(otherImages.begin(), otherImages.end(), buffers.begin(), [](GenericImage<T>* im) -> T* { return &(*im->begin()); } );
	buffers.push_back(thisBuffer);

	int numPixels = getSizeInPixels();

	//Sum each pixel in each buffer
	for(int p=0; p<numPixels; p++)
	{
		__int64 sum = 0;
		for(T* buf : buffers)
		{
			sum += buf[p];
		}
		destinationBuffer[p] = static_cast<T>(sum / buffers.size());
	}
}

/// <summary> Sums this image with the input images, storing in this image's memory </summary>
/// <param name="otherImages"> A std::vector of pointers to other GenericImages that will be summed with this one. These images must be the
///                            the same size as this image. </param>
template <typename T> void GenericImage<T>::sumWith(std::vector<GenericImage<T>*> otherImages)
{

	SumWith(otherImages, *this);

}

/// <summary> Sums this image with the input images, storing in the input image's memory </summary>
/// <param name="otherImages"> A std::vector of pointers to other GenericImages that will be summed with this one. These images must be the
///                            the same size as this image. </param>
/// <param name="summedImage"> Reference to generic image to store the result of the sum in. </param>
template <typename T> void GenericImage<T>::sumWith(std::vector<GenericImage<T>*> otherImages, GenericImage<T>& summedImage)
{

	//Check if each image has the same memory type
	for(GenericImage<T>* otherImage : otherImages)
	{
		if(summedImage.memoryModelType != otherImage->memoryModelType)
		{
			regularSumWith(otherImages, summedImage);
			return;
		}
	}

	//All images have the same type, use optimized function 
	optimizedSumWith(otherImages, summedImage);
}

/// <summary> Sums this image with the input images, storing in the input image's memory. This function uses iterators
///           and is able to sum images with different memory types
/// </summary>
/// <param name="otherImages"> A std::vector of pointers to other GenericImages that will be sum with this one. These images must be the
///                            the same size as this image. </param>
/// <param name="summedImage"> Reference to generic image to store the result of the sum in. </param>
template <typename T> void GenericImage<T>::regularSumWith(std::vector<GenericImage<T>*> otherImages, GenericImage<T>& summedImage)
{

	//Get image iterators to iterate over
	std::vector<GenericImage<T>::iterator > imageIterators = std::vector<GenericImage<T>::iterator >();
	for(GenericImage* otherImage : otherImages)
	{
		auto it = otherImage->begin();
		imageIterators.push_back(it);
	}
	imageIterators.push_back(begin());


	//Iterate over each image
	auto end = summedImage.end();
	for(auto toIt = summedImage.begin(); toIt != end; ++toIt)
	{
		__int64 sum = 0;
		for(auto it=imageIterators.begin(); it!=imageIterators.end(); ++it)
		{
			sum += static_cast<T>(**it);
			++(*it);
		}
		*toIt = static_cast<T>(sum);
	}

}


/// <summary> Sum this image with the input images, storing in the input image's memory. Uses a direct memory approach that is
///           only compatible if all images share the same memory type
/// </summary>
/// <param name="otherImages"> A std::vector of pointers to other GenericImages that will be summeded with this one. These images must be the
///                            the same size as this image and all have the same memory type </param>
/// <param name="summedImage"> Reference to generic image to store the result of the sum in. </param>
template <typename T> void GenericImage<T>::optimizedSumWith(std::vector<GenericImage<T>*> otherImages, GenericImage<T>& summedImage)
{
	//Get pointers to destination buffer
	T* destinationBuffer = &(*summedImage.begin());
	T* thisBuffer= &(*begin());

	//Get pointers to each image's buffer
	std::vector<T*> buffers(otherImages.size());
	std::transform(otherImages.begin(), otherImages.end(), buffers.begin(), [](GenericImage<T>* im) -> T* { return &(*im->begin()); } );
	buffers.push_back(thisBuffer);

	int	numSumPixels = getSizeInPixels();

	//Sum each pixel in each buffer
	for(int p=0; p<numSumPixels; p++)
	{
		__int64 sum = 0;
		for(T* buf : buffers)
		{
			sum += buf[p];
		}
		destinationBuffer[p] = static_cast<T>(sum);
	}
}


///<summary> Deletes specified channels from the image, moving the remaining channels to the front of the image </summary>
///<param name="channelsToDelete"> A std::vector containing the number of the channels to delete </param>
template <typename T> void GenericImage<T>::deleteChannels(std::vector<int>& channelsToDelete)
{

	//Allocate arrays
	std::sort(channelsToDelete.begin(), channelsToDelete.end());
	std::vector<int> editedChannelsToDelete = channelsToDelete;


	//Disabled channels mean memory configuration will not have as many channels as this image
	if(hasDisabledChannels())
	{

		//Get list of only enabled channels to delete, adjusted for missing channels
		editedChannelsToDelete= std::vector<int>(channelsToDelete.size());
		auto lastElement = std::set_intersection(channelsToDelete.begin(), channelsToDelete.end(), enabledChannels.begin(), enabledChannels.end(),editedChannelsToDelete.begin());
		editedChannelsToDelete.resize(lastElement-editedChannelsToDelete.begin());  
		editedChannelsToDelete = ChannelManipulator<T>::getSequentialMemoryChannels(enabledChannels, editedChannelsToDelete);


		//Adjust Enabled Channels
		ChannelManipulator<T>::removeChannels(enabledChannels, channelsToDelete);
		if(enabledChannels.size() == getNumChannels() - channelsToDelete.size())
			enabledChannels = std::vector<int>();
	}


	//Delete Channels
	memoryModel->deleteChannels(editedChannelsToDelete);
	numChannels-=static_cast<int>(channelsToDelete.size());
	memoryModel->updateParameters(*this);



}


/// <summary> Used to iterating over every pixel, returns a pointer to the beginning of the image </summary>
/// <returns> A pointer to the beginning of the image </returns>
template <typename T> ImageIterator<T> GenericImage<T>::begin()
{
	return memoryModel->begin();
}


/// <summary> Used to iterate over every pixel, returns a pointer to the end of the image </summary>
/// <returns> A pointer to the end of the image </returns>
template <typename T> ImageIterator<T> GenericImage<T>::end()
{
	return memoryModel->end();
}


/// <summary> Used to iterate over every pixel in a channel, returns a pointer to the beginning of the channel. Iterations are only guaranteed valid within one channel </summary>
/// <param name="channel"> The channel to iterate over </param>
/// <param name="m"> The m of the channel  </param>
/// <param name="z"> The depth of the channel </param>
/// <returns> A pointer to the beginning of the channel </returns>
template <typename T> ImageIterator<T> GenericImage<T>::channelBegin(int channel, int m, int z)
{
	if(hasDisabledChannels())
	{
		channel = ChannelManipulator<T>::getSequentialMemoryChannel(channel,enabledChannels);
	}

	return memoryModel->channelBegin(channel, m, z);
}


/// <summary> Used to iterate over every pixel in a channel, returns a pointer to the end of the channel. Iterations are only guaranteed valid within one channel </summary>
/// <param name="channel"> The channel to iterate over </param>
/// <param name="m"> The m of the channel  </param>
/// <param name="z"> The depth of the channel </param>
/// <returns> A pointer to the end of the channel </returns>
template <typename T> ImageIterator<T> GenericImage<T>::channelEnd(int channel, int m, int z)
{
	if(hasDisabledChannels())
	{
		channel = ChannelManipulator<T>::getSequentialMemoryChannel(channel,enabledChannels);
	}

	return memoryModel->channelEnd(channel, m, z);
}

/// <summary> Get if this channel is enabled in this image </summary>
/// <param name="channel"> The channel of interest </param>
/// <returns> Returns if this channel is enabled(true), or disabled(false) </returns>
template <typename T> bool GenericImage<T>::isChannelEnabled(int channel) const
{

	//All channels enabled
	if(!hasDisabledChannels())
	{
		return (channel < getNumChannels());
	}

	//Some channels disabled
	else
	{
		return ChannelManipulator<short>::isChannelEnabled(channel,enabledChannels);
	}

}


/// <summary> Returns if this image has one or more disabled channels </summary>
/// <returns> Has one or more disabled channels (true), or all channels enabled (false) </returns>
template <typename T> bool GenericImage<T>::hasDisabledChannels() const
{
	return enabledChannels.size() > 0;
}


/// <summary> Attempts to copy channel from one image to another, using an optimized approach tailored for 
///           copying between the underlying memory structures. If no optimized routine is available, a standard
///           copy using iterators is applied. In testing, the optimized copy was roughly 30 times faster, depending
///           on the image size 
/// </summary>
/// <param name="fromImage"> The source image which contains the channel to be copied from </param>
/// <param name="fromChannel"> The source channel in the 'fromImage' to be copied </param>
/// <param name="fromZ"> The specific depth of the 'fromImage' channel to be copied </param>
/// <param name="toChannel"> The channel to be copied into in this image </param>
/// <param name="toZ"> The depth of the channel to be copied into in this image </param>
/// <returns> True if an optimized copy was able to be performed. False means no copy took place </returns>
template <typename T> bool GenericImage<T>::optimizedCopyChannelFrom(const GenericImage<T>& fromImage, int fromChannel, int fromZ, int toChannel, int toZ)
{
	
	switch(memoryModelType)
	{
	case CONTIGUOUS_CHANNEL:
		switch(fromImage.memoryModelType)
		{
		case CONTIGUOUS_CHANNEL:
			{
				optimizedCopyChannelFrom_TCON_FCON(fromImage,fromChannel,fromZ,toChannel,toZ);	
			}
			return true;
		case INTERLACED_CHANNEL:
			{
				optimizedCopyChannelFrom_TCON_FINT(fromImage,fromChannel,fromZ,toChannel,toZ);	
			}
			return true;
		default:
			return false;
		}

	case INTERLACED_CHANNEL:
		switch(fromImage.memoryModelType)
		{
		case CONTIGUOUS_CHANNEL:
			{
				optimizedCopyChannelFrom_TINT_FCON(fromImage,fromChannel,fromZ,toChannel,toZ);	
			}
			return true;
		case INTERLACED_CHANNEL:
			{
				optimizedCopyChannelFrom_TINT_FINT(fromImage,fromChannel,fromZ,toChannel,toZ);	
			}
			return true;
		default:
			return false;
		}
	default:
		return false;
	}

}

/// <summary> Copies a channel at the specified depths from an image to this image, using a iterator based
///           based approach that is compatible with any underlying image memory structure
/// </summary>
/// <param name="fromImage"> The source image which contains the channel to be copied from </param>
/// <param name="fromChannel"> The source channel in the 'fromImage' to be copied </param>
/// <param name="fromZ"> The specific depth of the 'fromImage' channel to be copied </param>
/// <param name="toChannel"> The channel to be copied into in this image </param>
/// <param name="toZ"> The depth of the channel to be copied into in this image </param>
template <typename T> void GenericImage<T>::regularCopyChannelFrom(const GenericImage<T>& fromImage, int fromChannel, int fromZ, int toChannel, int toZ)
{
	
	throw "GenericImage::regularCopyChannelFrom disabled due to warning when using std::copy. The functionality in the commented out line works, but until it is needed it will be commented to prevent the warning.";

	// Copy all channels
	for(int m=0; m<numM; m++)
	{
		//std::copy(const_cast<GenericImage<T>&>(fromImage).channelBegin(fromChannel,m,fromZ), const_cast<GenericImage<T>&>(fromImage).channelEnd(fromChannel,m,fromZ), channelBegin(toChannel,m,toZ));
	}

}

/// <summary> Copies a channel at the specified depths from an image to this image, assuming a CONTIGUOUS_CHANNEL
///           memory model in this image, and a CONTIGUOUS_CHANNEL memory model in the image copied from 
/// </summary>
/// <param name="fromImage"> The source image which contains the channel to be copied from </param>
/// <param name="fromChannel"> The source channel in the 'fromImage' to be copied </param>
/// <param name="fromZ"> The specific depth of the 'fromImage' channel to be copied </param>
/// <param name="toChannel"> The channel to be copied into in this image </param>
/// <param name="toZ"> The depth of the channel to be copied into in this image </param>
template <typename T> void GenericImage<T>::optimizedCopyChannelFrom_TCON_FCON(const GenericImage<T>& fromImage, int fromChannel, int fromZ, int toChannel, int toZ)
{
	
	// Copy all channels
	for(int m=0; m<numM; m++)
	{
			auto fromBegin = const_cast<GenericImage<T>&>(fromImage).channelBegin(fromChannel,m,fromZ);
			auto toBegin = channelBegin(toChannel,m,toZ);
			int length = width * height;
			memcpy_s(&(*toBegin), length*sizeof(T), &(*fromBegin), length*sizeof(T));
	}

}

/// <summary> Copies a channel at the specified depths from an image to this image, assuming a CONTIGUOUS_CHANNEL
///           memory model in this image, and a INTERLACED_CHANNEL memory model in the image copied from 
/// </summary>
/// <param name="fromImage"> The source image which contains the channel to be copied from </param>
/// <param name="fromChannel"> The source channel in the 'fromImage' to be copied </param>
/// <param name="fromZ"> The specific depth of the 'fromImage' channel to be copied </param>
/// <param name="toChannel"> The channel to be copied into in this image </param>
/// <param name="toZ"> The depth of the channel to be copied into in this image </param>>
template <typename T> void GenericImage<T>::optimizedCopyChannelFrom_TCON_FINT(const GenericImage<T>& fromImage, int fromChannel, int fromZ, int toChannel, int toZ)
{

	// Copy all channels
	for(int m=0; m<numM; m++)
	{
		auto fromBegin = const_cast<GenericImage<T>&>(fromImage).channelBegin(fromChannel,m,fromZ);
		auto toBegin = channelBegin(toChannel,m,toZ);
		T* destinationBuffer = &(*toBegin);
		T* sourceBuffer = &(*fromBegin);

		for (long y = 0; y < height; y++) {
			memcpy_s((void*) (destinationBuffer), width * sizeof(T), (void*) (sourceBuffer), width * sizeof(T));
			destinationBuffer += width;
			sourceBuffer += fromImage.getWidth()*fromImage.getNumEnabledChannels();
		}
	}

}


/// <summary> Copies a channel at the specified depths from an image to this image, assuming a INTERLACED_CHANNEL
///           memory model in this image, and a CONTIGUOUS_CHANNEL memory model in the image copied from 
/// </summary>
/// <param name="fromImage"> The source image which contains the channel to be copied from </param>
/// <param name="fromChannel"> The source channel in the 'fromImage' to be copied </param>
/// <param name="fromZ"> The specific depth of the 'fromImage' channel to be copied </param>
/// <param name="toChannel"> The channel to be copied into in this image </param>
/// <param name="toZ"> The depth of the channel to be copied into in this image </param>
template <typename T> void GenericImage<T>::optimizedCopyChannelFrom_TINT_FCON(const GenericImage<T>& fromImage, int fromChannel, int fromZ, int toChannel, int toZ)
{
	
	// Copy all channels
	for(int m=0; m<numM; m++)
	{
		auto fromBegin = const_cast<GenericImage<T>&>(fromImage).channelBegin(fromChannel,m,fromZ);
		auto toBegin = channelBegin(toChannel,m,toZ);
		T* destinationBuffer = &(*toBegin);
		T* sourceBuffer = &(*fromBegin);

		for (long y = 0; y < height; y++) {
			memcpy_s((void*) (destinationBuffer), width * sizeof(T), (void*) (sourceBuffer), width * sizeof(T));
			destinationBuffer+= width*getNumEnabledChannels();
			sourceBuffer += width;
		}
	}

}

/// <summary> Copies a channel at the specified depths from an image to this image, assuming a INTERLACED_CHANNEL
///           memory model in this image, and a INTERLACED_CHANNEL memory model in the image copied from 
/// </summary>
/// <param name="fromImage"> The source image which contains the channel to be copied from </param>
/// <param name="fromChannel"> The source channel in the 'fromImage' to be copied </param>
/// <param name="fromZ"> The specific depth of the 'fromImage' channel to be copied </param>
/// <param name="toChannel"> The channel to be copied into in this image </param>
/// <param name="toZ"> The depth of the channel to be copied into in this image </param>>
template <typename T> void GenericImage<T>::optimizedCopyChannelFrom_TINT_FINT(const GenericImage<T>& fromImage, int fromChannel, int fromZ, int toChannel, int toZ)
{

	// Copy all channels
	for(int m=0; m<numM; m++)
	{
		auto fromBegin = const_cast<GenericImage<T>&>(fromImage).channelBegin(fromChannel,m,fromZ);
		auto toBegin = channelBegin(toChannel,m,toZ);
		T* destinationBuffer = &(*toBegin);
		T* sourceBuffer = &(*fromBegin);

		for (long y = 0; y < height; y++) {
			memcpy_s((void*) (destinationBuffer), width * sizeof(T), (void*) (sourceBuffer), width * sizeof(T));
			destinationBuffer+= width*getNumEnabledChannels();
			sourceBuffer += fromImage.getWidth()*fromImage.getNumEnabledChannels();
		}
	}

}


/// <summary> Is this image equal to the other, in image dimensions, enabled channels, and data </summary>
/// <returns> True if all of the above criteria are the same </returns>
template <typename T> bool GenericImage<T>::operator==(const GenericImage<T>& otherImage)
{
	if(!sameDimensions(otherImage))
	{
		return false;
	}
	else
	{
		auto it2=const_cast<GenericImage<T>&>(otherImage).begin();
		for(auto it=begin(); it!=end(); ++it, ++it2)
		{
			if(*it!=*it2)
				return false;
		}
		return true;
		//return std::equal(begin(), end(), const_cast<GenericImage<T>&>(otherImage).begin());
	}

}

/// <summary> Is this image not equal to the other, in image dimensions, enabled channels, or data </summary>
/// <returns> True if any of the above criteria are different </returns>
template <typename T> bool GenericImage<T>::operator!=(const GenericImage<T>& otherImage)
{
	return !(*this==otherImage);
}


/// <summary> Copy constructor, duplicates underlying memory model, but still points to same external buffer </summary>
template <typename T> GenericImage<T>::GenericImage(const GenericImage<T>& copyFrom)
{


	//Basic parameters
	width = copyFrom.width;
	height = copyFrom.height;
	numChannels = copyFrom.numChannels;
	numM = copyFrom.numM;
	numZSlices = copyFrom.numZSlices;


	//Make sure enabled channels vector is sorted and unique
	enabledChannels = copyFrom.enabledChannels;


	//Get memory model
	memoryModelType = copyFrom.memoryModelType;
	memoryModel = getImageMemoryModel(memoryModelType);

	setMemoryBuffer(copyFrom.memoryModel->getMemoryBufferStart());

}


/// <summary> Assignment operator </summary>
template <typename T> GenericImage<T>& GenericImage<T>::operator = (const GenericImage<T>& assignFrom)
{

	//Dont assign to oneself
	if(this != &assignFrom)
	{

		GenericImage<T> copy(assignFrom);
		swap(copy);

	}

	return *this;
}

/// <summary> Swap the internals of this image with another </summary>
/// <param name="withImage"> The image to swap with </param>
template <typename T> void GenericImage<T>::swap(GenericImage<T>& withImage)
{

	std::swap(width, withImage.width);
	std::swap(height, withImage.height);
	std::swap(numChannels, withImage.numChannels);
	std::swap(numM, withImage.numM);
	std::swap(numZSlices, withImage.numZSlices);
	std::swap(enabledChannels, withImage.enabledChannels);
	std::swap(memoryModelType, withImage.memoryModelType);
	std::swap(memoryModel, withImage.memoryModel);

}


/// <summary> Test if this image has the same dimensions (width,height,channels,depth,M,enabledChannels)
///           as the input image 
///</summary>
/// <param name="imageToCompare"> The image to compare to </param>
/// <returns> True if this image has the same dimensions as the input </returns>
template <typename T> bool GenericImage<T>::sameDimensions(const GenericImage<T>& imageToCompare) const
{
	bool different = 
		imageToCompare.getWidth() != getWidth() ||
		imageToCompare.getHeight() != getHeight() ||
		imageToCompare.getNumChannels() != getNumChannels() ||
		imageToCompare.getNumZSlices() != getNumZSlices() ||
		imageToCompare.getNumM() != getNumM() ||
		imageToCompare.getNumEnabledChannels() != getNumEnabledChannels();

	return !different;

}


/// <summary> Returns a char representation of this value, using the min and max as a scale </summary>
/// <param name="val"> The value to convert </param> 
/// <param name="min"> The min value for this percent range </param>
/// <param name="max"> The max value for this percent range </param>
template <typename T> char GenericImage<T>::getValChar(T val, T min, T max)
{

	//char chars[] = {' ','-','=','+','%','@','#','°'};
	char chars[] = {'-','=','+','%','@','#','°'};
	//char chars[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
	const double STEPS = sizeof(chars)/sizeof(chars[0]);
	double stepBreaks = (static_cast<double>(max)-static_cast<double>(min))/STEPS*1.01;


	double curBreak = stepBreaks;
	for(int i=0; i<STEPS; i++)
	{
		if(val-min < curBreak)
			return chars[i];
		curBreak+=stepBreaks;
	}

	return ' ';

}

/// <summary> Prints the image to the standard output stream. There is a limit of 70 pixels in width
///           of height currently built in to automatically disable it when it generally becomes unreadable
/// </summary>
template <typename T> void GenericImage<T>::printImage()
{

	auto minMaxValue = std::minmax_element(begin(), end());
	T minValue = *(minMaxValue.first);
	T maxValue = *(minMaxValue.second);
	if(maxValue - minValue < 1)
		minValue--;

	printImage(minValue,maxValue);

}

/// <summary> Prints the image to the standard output stream. There is a limit of 70 pixels in width
///           of height currently built in to automatically disable it when it generally becomes unreadable
/// </summary>
/// <param name="min"> The minimum value in this image, used to scale the output to maximize visible differences </param>
/// <param name="min"> The maximum value in this image, used to scale the output to maximize visible differences </param>
template <typename T> void GenericImage<T>::printImage(T min, T max)
{

	if(getWidth() > 70 || getHeight() > 70)
		return;

	std::stringstream image;

	image << "=========================================================================\n";
	image << "D:"<< width << "x" <<  height << " Channels:" << getNumChannels() << " Depth:" << getNumZSlices() <<  " M:" << getNumM() << " ";
	if(enabledChannels.size() > 0)
	{
		image << "Disabled Channels:";
		bool skippedOnce = false;
		for(int channel : ChannelManipulator<T>::getDisabledChannels(getNumChannels(),enabledChannels))
		{
			if(skippedOnce)
			{
				image <<",";
				skippedOnce = true;
			}
			image << channel;
		}
	}
	else
	{
		image <<"Disabled Channels:none";
	}
	image << "\n";
	image << "=========================================================================\n";
	for(int m=0; m<getNumM(); m++) {
		for(int z=0; z<getNumZSlices(); z++) {
			for(int y=0; y<getHeight(); y++) {
				for(int c=0; c<getNumChannels(); c++) {
					for(int x=0; x<getWidth(); x++) {
						T val = getVal(x,y,z,c,m);
						image << getValChar(val, min, max);
					}
					image << "   ";
				}
				image << "\n";
			}
			image << "\n";
		}
	}
	image << "=========================================================================\n";

	std::cout << image.str() << std::endl;

}
