#pragma once

#include <string>
#include <windows.h>

/// <summary>
/// Object oriented file mapping object that uses one underlying File Map and reference counting to properly close the map when all instances have been destroyed. </summary>
/// <remarks>
/// The SmartFileMapping class wraps and abstracts the underlying windows File Map functionality, allowing convienent creation of a file map for pointer manupulation of 
/// hard drive files. When copies are made the same file map is shared between them, and when the last copy is destroyed the file map is closed.
/// </remarks>
template <typename T> class SmartFileMapping
{
public:


	//Object Creation Functions
	SmartFileMapping(std::wstring filePath, long long offsetToStart, long long mapLength);
	SmartFileMapping(const SmartFileMapping<T>& copyFrom);
	SmartFileMapping<T>& operator = (const SmartFileMapping<T>& assignFrom);
	~SmartFileMapping(void);


	//Get Functions
	char* getDataPointer();
	long long  getDataLength();


	//Status Functions
	bool isValid();


private:


	//Object Creation Functions
	void createFileMap(std::wstring filePath, long long dataStartOffset, long long dataLength);


	//Smart Reference Count
	int* activeCount;


	//File Map Members
	std::wstring filePath;    //File path as a string
	HANDLE file;         //File to open
	HANDLE mapFile;      //Handle of Map within File
	char* mapPointer;    //Start of map
	char* dataPointer;   //Start of data in map
	long long  dataLength;    //Size of overall map

};




/// <summary> Creates a SmartFileMapping object</summary>
/// <param name="filePath"> The file to map to </param>
/// <param name="dataStartOffset"> An offset in bytes from the start of the object to begin the map </param>
/// <param name="dataLength"> The size in bytes of the data to be accessed from the map  </param>
/// <returns> </returns>
template <typename T> SmartFileMapping<T>::SmartFileMapping(std::wstring filePath, long long dataStartOffset, long long dataLength)
{

	activeCount = new int(1);

	createFileMap(filePath, dataStartOffset, dataLength);
	this->dataLength = dataLength;
	this->filePath = filePath;


}


/// <summary> Destroys the SmartFileMapping and closes the underlying file map if no other SmartFileMapping objects are using it</summary>
template <typename T> SmartFileMapping<T>::~SmartFileMapping(void)
{

	if(*activeCount <= 1)
	{
		delete activeCount;
		UnmapViewOfFile(mapPointer);
		CloseHandle(mapFile);
		CloseHandle(file);
	}
	else
	{
		--*activeCount;
	}

}


/// <summary> Copy constructor </summary>
/// <param name="copyFrom"> Create a new SmartFileMapping object from the current one, sharing the same underlying file map</param>
template <typename T> SmartFileMapping<T>::SmartFileMapping(const SmartFileMapping<T>& copyFrom)
{

	//Copy File Pointers
	file = copyFrom.file;
	mapFile = copyFrom.mapFile;
	mapPointer = copyFrom.mapPointer;
	dataPointer = copyFrom.dataPointer;
	dataLength = copyFrom.dataLength;
	filePath = copyFrom.filePath;
	
	//Reference Count
	activeCount = copyFrom.activeCount;
	++*activeCount;

}


/// <summary> Assignent operator </summary>
/// <param name="assignFrom"> Assign to this object from the input object</param>
template <typename T> SmartFileMapping<T>& SmartFileMapping<T>::operator=(const SmartFileMapping<T>& assignFrom) 
{

	//Dont assign to oneself
	if(this != &assignFrom)
	{
		//Already using same file map
		if(this->activeCount != assignFrom.activeCount)
		{

			this->~SmartFileMapping();
			SmartFileMapping copy(assignFrom);

			//Copy File Pointers
			file = copy.file;
			mapFile = copy.mapFile;
			mapPointer = copy.mapPointer;
			dataPointer = copy.dataPointer;
			dataLength = copy.dataLength;
			filePath = copy.filePath;

			//Reference Count
			activeCount = copy.activeCount;
			++*activeCount;

		}
	}

	return *this;
}


///<summary> Returns a pointer to the begining of the mapped region. This pointer is valid as long as a smart file mapped object with
///          this information exists. Care should be taken to not write beyond the length of the file mapping. </summary>
///<return> char* to the begining of the file map. See cautions in description about validity of this pointer</return>
template <typename T> char* SmartFileMapping<T>::getDataPointer()
{
	return dataPointer;
}


///<summary> Returns the total length in bytes of the requested file mapping. The actual file mapping may be longer due to OS constraints </summary>
///<return> Requested size of this file map in bytes </return>
template <typename T> long long  SmartFileMapping<T>::getDataLength()
{
	return dataLength;
}


/// <summary> Creates a OS level file map object </summary>
/// <param name="filePath"> The file to create a map of </param>
/// <param name="dataStartOffset"> An offset in bytes from the start of the object to begin the map </param>
/// <param name="dataLength"> The size in bytes of the data to be accessed from the map  </param>
template <typename T> void SmartFileMapping<T>::createFileMap(std::wstring wfilePath, long long dataStartOffset, long long dataLength)
{


	//=== Get System Info ===
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	long long granularityOfMapView = si.dwAllocationGranularity;

	//=== Calculate Map Parameters ===
	LARGE_INTEGER mapStartOffsetLI, fileSizeLI;
	long long mapStartOffset = (dataStartOffset / granularityOfMapView) * granularityOfMapView;
	long long mapLength = (dataStartOffset % granularityOfMapView) + dataLength;
	long long mapEndOffset = mapLength + mapStartOffset;
	long long dataStartOffsetFromMap = (dataStartOffset - mapStartOffset);
	fileSizeLI.QuadPart = mapEndOffset;
	mapStartOffsetLI.QuadPart = mapStartOffset;

	//=== Open File and Map ===
	file = CreateFile(wfilePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL/*FILE_FLAG_WRITE_THROUGH|FILE_FLAG_NO_BUFFERING*/,NULL);	//current file
	mapFile = CreateFileMapping(file, NULL, PAGE_READWRITE, fileSizeLI.HighPart, fileSizeLI.LowPart, NULL);
	mapPointer = (char*)MapViewOfFile(mapFile, FILE_MAP_ALL_ACCESS, mapStartOffsetLI.HighPart, mapStartOffsetLI.LowPart, static_cast<SIZE_T>(mapLength));

	//=== Offset Map ===
	dataPointer = mapPointer + dataStartOffsetFromMap;


}


/// <summary> Returns if the underlying file map was mapped successfully </summary>
/// <returns> True if the map is valid and can be used properly, false otherwise </returns>
template <typename T> bool SmartFileMapping<T>::isValid()
{
	if(file == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	else if(mapFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	return true;
}

