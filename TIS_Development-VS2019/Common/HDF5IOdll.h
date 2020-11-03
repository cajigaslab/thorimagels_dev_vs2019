#pragma once
#include <string>
#include <vector>
#include "./PDLL/pdll.h"

typedef enum H5DataTypeEnum
{
	INITIAL_DATATYPE,
	DATA_DOUBLE,
	DATA_UINT32,
	DATA_UINT64,
	DATA_FLOAT,
	DATA_UCHAR
}H5DataType;

typedef enum H5FileTypeEnum
{
	OVERWRITE,
	READONLY,
	READWRITE
}H5FileType;

//dll wrapper class:
class HDF5ioDLL : public PDLL
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(HDF5ioDLL)
#pragma warning(pop)

	DECLARE_FUNCTION0(long, CloseFileIO)
	DECLARE_FUNCTION3(long, CheckGroupDataset, const char*, const char*, unsigned __int64 &)
	DECLARE_FUNCTION3(long, CreateFileIO, const wchar_t *, long, unsigned __int64)
	DECLARE_FUNCTION4(long, CreateGroupDatasets, const char*, const char**,long, long)
	DECLARE_FUNCTION0(long, DestroyFileIO)
	DECLARE_FUNCTION6(long, ExtendData, const char*, const char*, void*, long, bool, unsigned long)
	DECLARE_FUNCTION5(long, GetGroupDatasetNames, const char*, char ***, long *, char ***, long *)
	DECLARE_FUNCTION2(long, GetPathandFilename, wchar_t *, long)
	DECLARE_FUNCTION2(long, OpenFileIO, const wchar_t *, long)
	DECLARE_FUNCTION6(long, ReadData, const char*, const char*, void*, long, unsigned __int64, unsigned __int64)
	DECLARE_FUNCTION1(long, SetPathandFilename, const wchar_t *)
	DECLARE_FUNCTION6(long, WriteData, const char*, const char*, void*, long, unsigned __int64, unsigned long)

};