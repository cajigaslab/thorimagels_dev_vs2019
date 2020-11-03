#pragma once

#include "FileMappedImage.h"
#include "ChannelManipulator.h"

template <typename T>
class RawFile
{
public:

	//Constructors
	RawFile(std::wstring filePath, int imageWidth, int imageHeight, int imageDepth, int maxImageChannels, int imageM, bool containsDisabledChannels, std::vector<int> enabledChannels, typename GenericImage<T>::MemoryType memModelType);
	RawFile(std::wstring filePath, int imageSizeBytes, int maxImageChannels, int imageM, bool containsDisabledChannels, std::vector<int> enabledChannels, typename GenericImage<T>::MemoryType memModelType);
	RawFile(const RawFile<T>& copyFrom);
	RawFile<T>& operator= (const RawFile<T>& assignFrom);
	~RawFile();

	//File information
	long long getFileSize();
	bool isValid();
	void releaseFile();

	//Getting individual images
	long long getSingleImageSize();
	int getNumImages();
	FileMappedImage<T> getImageAtIndex(int index);


	//Whole series manipulation
	void convertToEnabledOnly();
	void shortenSeriesTo(int seriesSize);
	void shortenSeriesToTotalZSlices(int numberOfZSlices);
	void shortenSeriesToSize(UINT64 sizeInBytes);
	void deleteChannels(std::vector<int>& channelsToDelete);
	void averageImages(int imagesPerAverage);
	void SumImages(int imagesPerAverage);
	void fillAll(T val);


	//Image Properties
	int getImageWidth() const;
	int getImageHeight() const;
	int getImageZSlices() const;
	int getImageNumChannels() const;
	int getImageNumM() const;


	//Raw file structure
	typename GenericImage<T>::MemoryType getMemoryLayout() const;
	bool containsEnabledChannelsOnly();
	std::vector<int> getEnabledChannels();

private:

	//File members
	HANDLE file;
	std::wstring filePath;


	//File properties
	bool containsOnlyEnabledChannels;
	std::vector<int> enabledChannels;


	//Image properties
	int imageWidth;
	int imageHeight;
	int imageSizeBytes;
	int imageZSlices;
	int imageNumChannels;
	int imageNumM;
	typename GenericImage<T>::MemoryType memModelType;


	//Helper operations
	void updateImageParameters(int newWidth, int newHeight, int newNumZSlices, int newNumChannels, int newNumM, bool containsDisabledChannels);
	FileMappedImage<T> getImageAtIndex(int index, long long perImageOffset);


	//File operations
	void openFile(std::wstring filePath);
	void changeFileSize(long long newFileSizeBytes);



};



/// <summary> Constructs a raw file object that allows for the manipulation of a raw file </summary>
/// <param="filePath"> File path to raw file </param>
/// <param name="imageWidth"> Width in pixels of images in raw file </param>
/// <param name="imageHeight"> Height in pixels of images in raw file  </param>
/// <param name="imageDepth"> Number of z slices taken in the image </param>
/// <param name="maxImageChannels"> Total number of channels that the images represent, including disabled and enabled channels </param>
/// <param name="imageM"> Mosaic size </param>
/// <param name="containsDisabledChannels"> True if the raw file has blank data placeholders for the disabled channels </param>
/// <param name="enabledChannels"> Int vector containing all enabled channels </param>
/// <param name="memModelType"> Memory configuration of raw file </param>
template <typename T> RawFile<T>::RawFile(std::wstring filePath, int imageWidth, int imageHeight, int imageDepth, int maxImageChannels, int imageM, bool containsDisabledChannels, std::vector<int> enabledChannels, typename GenericImage<T>::MemoryType memModelType)
{


	openFile(filePath);

	this->filePath = filePath;
	this->imageWidth = imageWidth;
	this->imageHeight = imageHeight;
	this->imageZSlices = imageDepth;
	this->imageNumChannels = maxImageChannels;
	this->imageNumM = imageM;
	this->memModelType = memModelType;
	this->containsOnlyEnabledChannels = !containsDisabledChannels;
	this->enabledChannels = enabledChannels;

}

/// <summary> Constructs a raw file object that allows for the manipulation of a raw file </summary>
/// <param="filePath"> File path to raw file </param>
/// <param name="imageWidth"> Width in pixels of images in raw file </param>
/// <param name="imageHeight"> Height in pixels of images in raw file  </param>
/// <param name="imageDepth"> Number of z slices taken in the image </param>
/// <param name="maxImageChannels"> Total number of channels that the images represent, including disabled and enabled channels </param>
/// <param name="imageM"> Mosaic size </param>
/// <param name="containsDisabledChannels"> True if the raw file has blank data placeholders for the disabled channels </param>
/// <param name="enabledChannels"> Int vector containing all enabled channels </param>
/// <param name="memModelType"> Memory configuration of raw file </param>
template <typename T> RawFile<T>::RawFile(std::wstring filePath, int imageSizeBytes, int maxImageChannels, int imageM, bool containsDisabledChannels, std::vector<int> enabledChannels, typename GenericImage<T>::MemoryType memModelType)
{
	openFile(filePath);

	this->filePath = filePath;	
	this->imageWidth = imageWidth;
	this->imageHeight = imageHeight;
	this->imageZSlices = imageDepth;
	this->imageNumChannels = maxImageChannels;
	this->imageNumM = imageM;
	this->memModelType = memModelType;
	this->containsOnlyEnabledChannels = !containsDisabledChannels;
	this->enabledChannels = enabledChannels;
	this->imageSizeBytes = imageSizeBytes;
}

/// <summary> Create a new raw file as a copy of another </summary>
/// <param name="copyFrom"> The raw file to copy </param>
template <typename T> RawFile<T>::RawFile(const RawFile<T>& copyFrom)
{

	filePath = copyFrom.filePath;
	imageWidth = copyFrom.imageWidth;
	imageHeight = copyFrom.imageHeight;
	imageZSlices = copyFrom.imageZSlices;
	imageNumChannels = copyFrom.imageNumChannels;
	imageNumM = copyFrom.imageNumM;
	memModelType = copyFrom.memModelType;
	containsOnlyEnabledChannels = copyFrom.containsOnlyEnabledChannels;
	enabledChannels = copyFrom.enabledChannels;
	imageSizeBytes = copyFrom.imageSizeBytes;

	openFile(filePath);

}

/// <summary> Assign a different raw file to this raw file </summary>
/// <param name="assignFrom"> The raw file to be assigned </param>
template <typename T> RawFile<T>& RawFile<T>::operator=(const RawFile<T>& assignFrom)
{
	if(this != &assignFrom)
	{

		this->~RawFile();
		RawFile<T> copy(assignFrom);

		filePath = copy.filePath;
		imageWidth = copy.imageWidth;
		imageHeight = copy.imageHeight;
		imageZSlices = copy.imageZSlices;
		imageNumChannels = copy.imageNumChannels;
		imageNumM = copy.imageNumM;
		memModelType = copy.memModelType;;
		containsOnlyEnabledChannels = assignFrom.containsOnlyEnabledChannels;
		enabledChannels = assignFrom.enabledChannels;


		openFile(filePath);

	}
	else
	{
	}

	return *this;

}

/// <summary> Destructor releases file resources </summary>
template <typename T> RawFile<T>::~RawFile()
{
	if (NULL !=file) 
	{
		CloseHandle(file); 
		file = NULL;
	}

}

/// <summary> Returns the size of a single image </summary>
/// <returns> The size of an image in bytes </returns>
template <typename T> long long RawFile<T>::getSingleImageSize()
{
	if(containsEnabledChannelsOnly())
	{
		GenericImage<T> imageToGetSize(getImageWidth(),getImageHeight(),getImageZSlices(),getImageNumChannels(),getImageNumM(),memModelType, enabledChannels);
		return imageToGetSize.getSizeInBytes();
	}
	else
	{
		GenericImage<T> imageToGetSize(getImageWidth(),getImageHeight(),getImageZSlices(),getImageNumChannels(),getImageNumM(),memModelType);
		return imageToGetSize.getSizeInBytes();
	}
}

/// <summary> releases file resources </summary>
template <typename T> void RawFile<T>::releaseFile()
{
	if (NULL !=file) 
	{
		CloseHandle(file); 
		file = NULL;
	}

}

/// <summary> Returns the number of images in a file based on the file size and the size of a single image </summary>
/// <returns> The number of images in this raw file </returns>
template <typename T> int RawFile<T>::getNumImages()
{
	long long singleImageSize = getSingleImageSize();
	long long seriesSize = getFileSize();
	int numImages = static_cast<int>(seriesSize / singleImageSize);

	return numImages;
}


/// <summary> Returns the given image in the series </summary>
/// <param name="index"> The number of the image to return in the series </param>
/// <param name="perImageOffset"> The offset in bytes from image to image, usually the size in bytes of one image </param>
/// <exception cref="std::out_of_range"> Thrown when index is out of range </exception>
/// <returns> A file mapped image that contains the image requested </returns>
template <typename T> FileMappedImage<T> RawFile<T>::getImageAtIndex(int index, long long perImageOffset)
{

	//Index out of bounds
	if(index >= getNumImages())
	{
		std::stringstream errorMessage;
		errorMessage << "Attempting to access image " << index << " in raw file of size " << getNumImages()-1;
		throw std::out_of_range(errorMessage.str());
	}


	long long byteOffsetToStart = perImageOffset * index;

	if(containsEnabledChannelsOnly())
	{
		FileMappedImage<T> imageInSeries(filePath,byteOffsetToStart,getImageWidth(),getImageHeight(),getImageZSlices(),getImageNumChannels(),getImageNumM(),memModelType, enabledChannels);
		return imageInSeries;
	}
	else
	{
		FileMappedImage<T> imageInSeries(filePath,byteOffsetToStart,getImageWidth(),getImageHeight(),getImageZSlices(),getImageNumChannels(),getImageNumM(),memModelType);
		return imageInSeries;
	}

}


/// <summary> Returns the image at the requested index in the raw file </summary>
/// <param name="index"> The number image to be returned </param>
/// <returns> File mapped image which contains the requested image </returns>
template <typename T> FileMappedImage<T> RawFile<T>::getImageAtIndex(int index)
{

	return getImageAtIndex(index, getSingleImageSize());

}


/// <summary> Removes images from the end of the file, shortening the file and permanently destroying the images at the end </summary>
/// <param name="seriesSize"> The number of images to keep </param>
template <typename T> void RawFile<T>::shortenSeriesTo(int seriesSize)
{
	
	long long newSizeInBytes = getSingleImageSize() * seriesSize;
	changeFileSize(newSizeInBytes);
}

/// <summary> Shortens the series by removing zSlices from the file. Removes a number of images ie. volumes that are needed to hold
/// the input number of 2D images ie. zSlices, rounded down to the nearest complete volume </summary>
/// <param name="numberOfZSlices"> The number of zSlices to keep </param>
template <typename T> void RawFile<T>::shortenSeriesToTotalZSlices(int numberOfZSlices)
{

	int volumesToSave = numberOfZSlices / getImageZSlices();
	shortenSeriesTo(volumesToSave);

}

/// <summary> Removes images from the end of the file, shortening the file and permanently destroying the images at the end </summary>
/// <param name="seriesSize"> The number of images to keep </param>
template <typename T> void RawFile<T>::shortenSeriesToSize(UINT64 sizeInBytes)
{	
	changeFileSize(sizeInBytes);
}

/// <summary> Removes channels from every image in the raw file </summary>
/// <param name="channelsToDelete"> Vector containing the channel numbers that will be deleted </param>
template <typename T> void RawFile<T>::deleteChannels(std::vector<int>& channelsToDelete)
{

	//=== Delete Channel In Each Image ===
	for(int f=0; f<getNumImages(); f++)
	{
		FileMappedImage<T> image = getImageAtIndex(f);
		image.deleteChannels(channelsToDelete);
	}
}

/// <summary> Used to rearrange and fix the memory of the raw file after a major change to the images within. For example,
/// after a channel is deleted the memory is non contiguous, calling this rearranges the images to make the raw file contiguous and updates the file length
/// to reflect the new data </summary>
/// <param name="newWidth"> The width of images in the raw file after updating </param>
/// <param name="newHeight"> The height of images in the raw file after updating</param>
/// <param name="newNumZSlices"> The number of z slices of images in the raw file after updating </param>
/// <param name="newNumChannels"> The number of channels of images in the raw file after updating </param>
/// <param name="newNumM"> The number of M of images in the raw file after updating </param>
/// <param name="containsDisabledChannels"> Whether the raw file after updating contains disabled channels </param>
template <typename T> void RawFile<T>::updateImageParameters(int newWidth, int newHeight, int newNumZSlices, int newNumChannels, int newNumM, bool containsDisabledChannels)
{

	//=== Calculate sizes using old parameters ===
	long long oldImageSize = getSingleImageSize();
	long long numImages = getNumImages();

	//=== Update parameters ===
	imageWidth = newWidth;
	imageHeight = newHeight;
	imageZSlices = newNumZSlices;
	imageNumChannels = newNumChannels;
	imageNumM = newNumM;
	containsOnlyEnabledChannels = !containsDisabledChannels;

	//=== Calculate sizes using new parameters ===
	long long newImageSize = getSingleImageSize();
	long long newFileSize = numImages * newImageSize;

	for(int i=0; i<numImages; i++)
	{
		getImageAtIndex(i).copyFromUsingBuffer(getImageAtIndex(i,oldImageSize));
	}

	changeFileSize(newFileSize);


}

/// <summary> Fills every pixel in all images in the raw file with the input value </summary>
/// <param name="val"> The value to fill all images with </param>
template <typename T> void RawFile<T>::fillAll(T val)
{
	for(int i=0; i<getNumImages();  i++)
	{
		getImageAtIndex(i).fillWith(val);
	}
}

/// <summary> Average images in the raw file, shortening the file in the process. Only valid for sequential images, ie. with 3 images per average (1,2,3) are averaged and stored in 1,
/// (4,5,6) are averaged and stored in 2, (7,8,9) are averaged and stored in 3. </summary>
/// <param name="imagesPerAverage"> The number of images averaged together </param>
template <typename T> void RawFile<T>::averageImages(int imagesPerAverage)
{

	if(imagesPerAverage < 2)
		return;
	
	//Average Every ImagePerAverage Image
	int count = 0;
	for(int f=imagesPerAverage-1; f<getNumImages(); f+=imagesPerAverage)
	{
		//Get other images and store them
		std::vector<FileMappedImage<T>> imagesToAverage;
		for(int i=f-imagesPerAverage+1; i<f; i++)
		{
			imagesToAverage.push_back(getImageAtIndex(i));
		}

		//Get pointers to the stored images
		std::vector<GenericImage<T>*> imagePointersToAverage;
		for(unsigned int i=0; i<imagesToAverage.size(); i++)
		{
			imagePointersToAverage.push_back(&imagesToAverage[i]);
		}

		//Store average in the beginning of the file
		GenericImage<T>& destinationImage = getImageAtIndex(count);
		getImageAtIndex(f).averageWith(imagePointersToAverage, destinationImage);

		count++;
	}
	

	shortenSeriesTo(getNumImages()/imagesPerAverage);
}

/// <summary> Sum images in the raw file, shortening the file in the process. Only valid for sequential images, ie. with 3 images per sum (1,2,3) are summed and stored in 1,
/// (4,5,6) are summed and stored in 2, (7,8,9) are sumemd and stored in 3. </summary>
/// <param name="imagesPerSum"> The number of images averaged together </param>
template <typename T> void RawFile<T>::SumImages(int imagesPerSum)
{

	if(imagesPerSum < 2)
		return;
	
	//Average Every ImagePerAverage Image
	int count = 0;
	for(int f=imagesPerSum-1; f<getNumImages(); f+=imagesPerSum)
	{
		//Get other images and store them
		std::vector<FileMappedImage<T>> imagesToAverage;
		for(int i=f-imagesPerSum+1; i<f; i++)
		{
			imagesToAverage.push_back(getImageAtIndex(i));
		}

		//Get pointers to the stored images
		std::vector<GenericImage<T>*> imagePointersToAverage;
		for(unsigned int i=0; i<imagesToAverage.size(); i++)
		{
			imagePointersToAverage.push_back(&imagesToAverage[i]);
		}

		//Store average in the beginning of the file
		GenericImage<T>& destinationImage = getImageAtIndex(count);
		getImageAtIndex(f).sumWith(imagePointersToAverage, destinationImage);

		count++;
	}
	

	shortenSeriesTo(getNumImages()/imagesPerSum);
}

/// <returns> The image width in pixels </returns>
template <typename T> int RawFile<T>::getImageWidth() const
{
	return imageWidth;
}

/// <returns> The image height in pixels </returns>
template <typename T> int RawFile<T>::getImageHeight() const
{
	return imageHeight;
}

/// <returns> The number of z slices in the image </returns>
template <typename T> int RawFile<T>::getImageZSlices() const
{
	return imageZSlices;
}

/// <returns> The number of channels, both enabled and disabled in the image </returns>
template <typename T> int RawFile<T>::getImageNumChannels() const
{
	return imageNumChannels;
}

/// <returns> The number of mosaics in the image </returns>
template <typename T> int RawFile<T>::getImageNumM() const
{
	return imageNumM;
}


/// <summary> Opens the file at the requested path </summary>
/// <param name="filePath"> The path of the raw file to open </param>
template <typename T> void RawFile<T>::openFile(std::wstring filePath)
{
	//=== Get System Info ===
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	long long granularityOfMapView = si.dwAllocationGranularity;

	//=== Open File and Map ===
	file = CreateFile(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL/*FILE_FLAG_WRITE_THROUGH|FILE_FLAG_NO_BUFFERING*/,NULL);	//current file

}


/// <summary> Get the size of the raw file </summary>
/// <returns> The size of the raw file in bytes </returns>
template <typename T> long long RawFile<T>::getFileSize()
{
	if(!isValid())
		return -1;

	LARGE_INTEGER fileSizeLI;
	LPDWORD highPart = (LPDWORD)&(fileSizeLI.HighPart);
	fileSizeLI.LowPart = GetFileSize(file, highPart);
	return fileSizeLI.QuadPart;

}

/// <summary> Change the size of the raw file on disk, truncating any data stored after the new size</summary>
/// <param name="newFileSizeBytes"> The new file size in bytes </param>
template <typename T> void RawFile<T>::changeFileSize(long long newFileSizeBytes)
{

	LARGE_INTEGER newSize;
	newSize.QuadPart = newFileSizeBytes;
	
	if (SetFilePointerEx(file, newSize, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER )
	{	
		SetEndOfFile(file);
	}	

}


/// <summary> Was the file on disk able to be successfully opened </summary>
/// <returns> True if the file is correctly opened </returns>
template <typename T> bool RawFile<T>::isValid()
{
	if(file == INVALID_HANDLE_VALUE)
		return false;
	return true;
}

/// <summary> Whether this raw file only contains enabled channels, or if the file still has random data for disabled channels </summary>
/// <returns> True if the raw file contains only enabled channels </returns>
template <typename T> bool RawFile<T>::containsEnabledChannelsOnly()
{
	return containsOnlyEnabledChannels;
}

/// <summary> Returns a vector with integers representing all enabled channels </summary>
/// <returns> Vector filled with channel indexes for all enabled channels. Channel indexes start at 0 </returns>
template <typename T> std::vector<int> RawFile<T>::getEnabledChannels()
{
	return enabledChannels;
}


/// <summary> Returns a vector with integers representing all enabled channels </summary>
/// <returns> Vector filled with channel indexes for all enabled channels. Channel indexes start at 0 </returns>
template <typename T> typename GenericImage<T>::MemoryType RawFile<T>::getMemoryLayout() const
{
	return memModelType;
}

/// <summary> If this file contains both enabled and disabled channels, removes disabled channels from the file and adjusts the size </summary>
template <typename T> void RawFile<T>::convertToEnabledOnly()
{

	if(containsEnabledChannelsOnly())
		return;

	//Delete disabled channels
	deleteChannels(ChannelManipulator<T>::getDisabledChannels(getImageNumChannels(),enabledChannels));
	updateImageParameters(getImageWidth(), getImageHeight(), getImageZSlices(), getImageNumChannels(), getImageNumM(), false);
}
