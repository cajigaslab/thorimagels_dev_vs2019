#pragma once
#include "genericimage.h"
#include "SmartFileMapping.h"


/// <summary>
/// Template based image class used for 3 dimensional, multi-channel, mosaic'd images, with a user specified type for pixel values. It's memory buffer is mapped
/// to a specific part of a file using a File Map. </summary>
/// <remarks>
/// The FileMappedImage class extends GenericImage to handle all of the details of opening and reading a file, allowing a file to easily be used as the image buffer to read from and write to. 
/// It accomplishes this using a SmartFileMapping object, effectively allowing the image to be copied and assigned while still referencing the same file.
/// </remarks>
template <typename T>
class FileMappedImage : public GenericImage<T>
{
public:
	
	//Constructor
	FileMappedImage(std::wstring filepath, long long offsetInFile, int width, int height, int numZSlices, int numChannels, int numM, MemoryType memModelType, const std::vector<int>& enabledChannels = std::vector<int>());

	//File Information
	bool isValid(); 

private:

	//File object
	SmartFileMapping<T> fileMap;

};


///<summary> Constructs a 5 dimensional image class containing data from the input file. The dimensions are width, height, depth, channel, and M </summary>
/// <param name="filepath"> String path to the file to be read from and written to </param>
/// <param name="offsetInFile"> Offset from the front of the file to begin reading information from </param>
///<param name="width"> The width in pixels for this image </param>
///<param name="height"> The width in pixels for this image </param>
///<param name="numZSlices"> The depth in pixels of this image </param>
///<param name="numChannels"> The number of channels for this image </param>
///<param name="numM"> the length of the 'm' dimension in pixels for this image </param>
///<param name="enabledChannels"> Optional vector containing the enabled channels in this image. If a value is given, the total channel parameters will remain the same, but only the enabled channels will be stored in memory </param>
template <typename T> FileMappedImage<T>::FileMappedImage(std::wstring filepath, long long offsetInFile, int width, int height, int numZSlices, int numChannels, int numM, MemoryType memModelType, const std::vector<int>& enabledChannels) :
	GenericImage(width,height,numZSlices,numChannels,numM,memModelType, enabledChannels),
	fileMap(filepath, offsetInFile, GenericImage::getSizeInBytes())
{
	setMemoryBuffer((T*)fileMap.getDataPointer());
}


/// <summary> Returns if the file was opened properly and this object can read and write to it </summary>
/// <returns> True if the file has been opened successfully, false otherwise </returns>
template <typename T> bool FileMappedImage<T>::isValid()
{
	return fileMap.isValid();
}