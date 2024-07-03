#pragma once

#include <vector>
#include "ImageIterator.h"

template <typename ImageT>
class GenericImage;


/// <summary>
/// Template based abstract class used for memory interpretation for 3 dimensional, multi-channel, mosaic'd images, with a user specified type for pixel values </summary>
/// <remarks>
/// The ImageMemoryModel class interprets memory at an adress based on input image parameters, it's template type, and a pointer to memory. The interpreted memory can be accessed by a set of pure virtual methods that
/// are implemented in child classes, providing for different interpretation strategies. An ImageMemoryModel object does not concern itself with the validity of the memory, so care should be taken that it's memory buffer is
/// the correct size. </remarks>
template <typename T>
class ImageMemoryModel
{
public:

	//Object Creation
	ImageMemoryModel(GenericImage<T>& parent);
	virtual ~ImageMemoryModel(void);
	virtual void updateParameters(GenericImage<T>& image);

	//Direct Pixel Access
	virtual T getVal(int x, int y, int p, int z, int channel, int m);
	virtual void setVal(int x, int y, int p, int z, int channel, int m, T val);

	//Memory Management
	virtual long long getSizeInBytes() = 0;
	virtual int getSizeInPixels();
	virtual void setBuffer(T* buffer);
	int bytesPerPixel();

	//Whole image manupulation
	virtual void deleteChannels(std::vector<int>& channelsToDelete)=0;

	//Iterator access
	typedef ImageIterator<T> iterator;
	typedef const ImageIterator<T> const_iterator;
	virtual iterator begin()=0;
	virtual iterator end()=0;
	virtual iterator channelBegin(int channel, int m, int z)=0;
	virtual iterator channelEnd(int channel, int m, int z)=0;

	//Direct Memory Access
	virtual T* getPointerForCoordinates(int x, int y, int p, int channel, int m, int z)=0;
	virtual T* getMemoryBufferStart();


protected:

	//Image Params
	int width, height, numPlanes, channels, zSlices, numM;
	T* buffer;

private:



};

#include "GenericImage.h"

/// <summary> Constructs an ImageMemoryModel with the same image parameters as the input GenericImage </summary>
/// <param name="parent"> GenericImage to copy image parameters from </param>
template <typename T> ImageMemoryModel<T>::ImageMemoryModel(GenericImage<T>& parent)
{
	updateParameters(parent);
}

/// <summary> Updates this objects image parameters to reflect the input image </summary>
/// <param name="parent"> GenericImage to copy image parameters from </param>
template <typename T> void ImageMemoryModel<T>::updateParameters(GenericImage<T>& image)
{
	width = image.getWidth();
	height = image.getHeight();
	numPlanes = image.getNumPlanes();
	zSlices = image.getNumZSlices();
	channels = image.getNumEnabledChannels();
	numM = image.getNumM();
}


template <typename T> ImageMemoryModel<T>::~ImageMemoryModel(void)
{
}

/// <summary> Returns the total size in pixels that this model has, based on it's image parameters </summary>
/// <returns> The total number of pixels reflecting width, height, number of channels, depth, and mosaics </returns>
template <typename T> int ImageMemoryModel<T>::getSizeInPixels()
{
	return height*width*numPlanes*channels*zSlices*numM;
}


/// <summary> Gets the pixel value at the specified coordinate </summary>
/// <param name="x"> The x coordinate of the requested pixel </param>
/// <param name="y"> The y coordinate of the requested pixel </param>
/// <param name="p"> The plane of the requested pixel </param>
/// <param name="z"> The z coordinate of the requested pixel </param>
/// <param name="channel"> The channel of the requested pixel </param>
/// <param name="m"> The mosaic number of the requested pixel </param>
/// <returns> The pixel value at the specified coordinate </returns>
template <typename T> T ImageMemoryModel<T>::getVal(int x, int y, int p, int z, int channel, int m) 
{
	return *getPointerForCoordinates(x,y,p,channel,m,z);
}


/// <summary> Sets the pixel value at the specified coordinate </summary>
/// <param name="x"> The x coordinate of the pixel to set </param>
/// <param name="y"> The y coordinate of the pixel to set </param>
/// <param name="p"> The plane of the pixel to set </param>
/// <param name="z"> The z coordinate of the pixel to set </param>
/// <param name="channel"> The channel of the pixel to set </param>
/// <param name="m"> The mosaic number of the pixel to set </param>
template <typename T> void ImageMemoryModel<T>::setVal(int x, int y, int p, int z, int channel, int m, T val)
{
		*getPointerForCoordinates(x,y,p,channel,m,z) = val;
}


/// <summary> Sets the start of the memory buffer to be interpreted by this model. There should be enough memory allocated for this model
/// based on its size that the model will never acess memory beyond the buffer. This model has no control over the alocation of memory </summary>
/// <param name="buffer"> Pointer to the start of the image memory to be interpreted </param>
template <typename T> void ImageMemoryModel<T>::setBuffer(T* buffer)
{
	this->buffer = buffer;
}


/// <summary> Returns the number of bytes taken to store the value of a single pixel </summary>
/// <returns> The size in bytes of a pixel value, usually sizeof(template type) </returns>
template <typename T> int ImageMemoryModel<T>::bytesPerPixel()
{
	return sizeof(T);
}

/// <summary> Get the start of the internal memory buffer used by this memory model </summary>
/// <returns> Pointer to start of memory Buffer </returns>
template <typename T> T* ImageMemoryModel<T>::getMemoryBufferStart()
{
	return buffer;
}