#pragma once

#include "GenericImage.h"

/// <summary>
/// Template based image class used for 3 dimensional, multi-channel, mosaic'd images, with a user specified type for pixel values. Allocates it's own memory buffer on creation. </summary>
/// <remarks>
/// The InternallyStoredImage class extends GenericImage to actually store the image data, as opposed to using an external buffer. On creation it allocates a array to store the image in
/// based on the size needed by the input image parameters. On destruction it destroys the allocated array. 
/// </remarks>
template <typename T>
class InternallyStoredImage : public GenericImage<T>
{
public:

	//Setup
	InternallyStoredImage(int width, int height, int numPlanes, int numZSlices, int numChannels, int numM, MemoryType memModelType, const std::vector<int>& enabledChannels = std::vector<int>());
	virtual ~InternallyStoredImage(void);
	InternallyStoredImage(const InternallyStoredImage<T>& copyFrom);
	InternallyStoredImage<T>& operator = (const InternallyStoredImage<T>& assignFrom);

protected:

	void swap(InternallyStoredImage<T>& fromImage);


private:

	//Pointer to buffer
	T* buffer;

};

///<summary> Constructs a 6 dimensional image class with an internal buffer to store the pixel values. The dimensions are width, height, planes, depth, channel, and M </summary>
///<param name="width"> The width in pixels for this image </param>
///<param name="height"> The width in pixels for this image </param>
///<param name="numPlanes"> The number of planes for this image </param>
///<param name="numZSlices"> The depth in pixels of this image </param>
///<param name="numChannels"> The number of channels for this image </param>
///<param name="numM"> the length of the 'm' dimension in pixels for this image </param>
///<param name="enabledChannels"> Optional vector containing the enabled channels in this image. If a value is given, the total channel parameters will remain the same, but only the enabled channels will be stored in memory </param>
template <typename T> InternallyStoredImage<T>::InternallyStoredImage(int width, int height, int numPlanes, int numZSlices, int numChannels, int numM, MemoryType memModelType, const std::vector<int>& enabledChannels):
	GenericImage(width,height,numPlanes,numZSlices,numChannels,numM,memModelType, enabledChannels)
{

	buffer = new T[getSizeInPixels()];
	setMemoryBuffer(buffer);

}


///<summary> Deleted allocated buffer </summary>
template <typename T> InternallyStoredImage<T>::~InternallyStoredImage(void)
{
	delete buffer;
}


/// <summary> Copy constructor, creates duplicate buffer </summary>
template <typename T> InternallyStoredImage<T>::InternallyStoredImage(const InternallyStoredImage<T>& copyFrom) : GenericImage<T>(copyFrom)
{

	//=== Create New Internal Buffer ===
	buffer = new T[getSizeInPixels()];
	setMemoryBuffer(buffer);
	this->copyFrom(copyFrom);
}


/// <summary> Assignemnt operator, creates duplicate buffer </summary>
template <typename T> InternallyStoredImage<T>& InternallyStoredImage<T>::operator = (const InternallyStoredImage<T>& assignFrom)
{

	//Dont assign to oneself
	if(this != &assignFrom)
	{

		InternallyStoredImage<T> copy(assignFrom);
		swap(copy);

	}

	return *this;
}


/// <summary> Swap internal structures from one image to another </summary>
/// <param name="withImage"> The image to swap with </param>
template <typename T> void InternallyStoredImage<T>::swap(InternallyStoredImage<T>& withImage)
{

	GenericImage<T>::swap(withImage);
	std::swap(buffer, withImage.buffer);

}