#pragma once

enum DataType
{
	DATATYPE_MIN = 0,
	INT_8BIT = 0,
	INT_16BIT,
	FLOAT_32BIT,
	FLOAT_64BIT,
	INT_32BIT,
	INT_64BIT,
	DATATYPE_MAX
};

enum MemoryType
{
	MEMORYTYPE_MIN = 0,
	DETACHED_CHANNEL = 0,
	CONTIGUOUS_CHANNEL,
	CONTIGUOUS_CHANNEL_MEM_MAP,
	CONTIGUOUS_CHANNEL_MEM_MAP_NO_FILE_DELETE,
	MEMORYTYPE_MAX
	/* TODO
	COMBINED_COLOR,
	COMBINED_M,
	COMBINED_Z,
	COMBINED_T,
	*/
};

enum
{
	XMIN = 1,
	XMAX = INT_MAX,
	YMIN = 1,
	YMAX = 65536,
	CMIN = 1,
	CMAX = 65536,
	MMIN = 1,
	MMAX = 65536,
	ZMIN = 1,
	ZMAX = INT_MAX,
	TMIN = 1,
	TMAX = INT_MAX,
};

class ImageMemory
{
public:
	ImageMemory();
	~ImageMemory();



	long AllocateMem(MemoryType memType, DataType dType, long x, long y, long c,long z,long m,long t, long bufferType, wstring memMapPath);
	char *GetMemPtr(long c, long m, long z, long t);
	char *GetMemPtr(long c, long m, long z, long, UINT64 offset);
	void UnlockMemPtr(long c, long m, long z, long t);
	long GetDataTypeSizeInBytes(DataType d,long &size);
	MemoryType GetMemoryType();
	long setTempFileName(wstring tFileName);
	wstring GetImageMemoryPath();
private:
	char ** ptr;
	char * ptrContiguous;

	HANDLE *hFileArray;
	HANDLE *hFileMapArray;

	vector<wstring> fileNames;
	wstring _memMapPath;
	wstring _tempFileName;
	LONGLONG offsetC;
	LONGLONG offsetM;
	LONGLONG offsetZ;
	LONGLONG offsetT;

	long nX;
	long nY;
	long nC;
	long nM;
	long nZ;
	long nT;

	long granularityOfMapView;

	DataType dataType;
	MemoryType memoryType;
	long imageBufferType;
};