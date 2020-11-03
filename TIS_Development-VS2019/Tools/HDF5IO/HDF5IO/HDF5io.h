#pragma once

//#include "stdafx.h"

#include "hdf5.h"	/*..\..\..\..\Tools\HDF5\HDF5IO\HDF5IO\includes\*/
#include "H5Cpp.h"  

#ifndef RANK
#define RANK 2
#endif

#if defined(HDF5_IO)
#define DllExport __declspec(dllexport)
#else
//    definitions used when using DLL
#define DllExport __declspec(dllimport)
#endif

class HDF5IO
{
private:
	HDF5IO();

	static bool instanceFlag;								///singleton created flag
	static std::unique_ptr<HDF5IO> single;					///pointer to internal object

	static hsize_t _bufsize;
	static hsize_t dims[RANK];  
	static hsize_t cmprdims[RANK];
	static hsize_t dimsNow[RANK];
	static hsize_t offset[RANK];

	static std::unique_ptr<H5::H5File> _file;				///pointer to h5 file
	char tmpStr[_MAX_FNAME];
	static std::wstring _pathAndFilename;

public:

	static HDF5IO* getInstance();

	long SetPathandFilename(std::wstring pathAndFilename);
	long GetPathandFilename(std::wstring &pathandFilename);
	long CreateFileIO(std::wstring pathAndFilename, long ftype, unsigned __int64 bufsize);
	long CreateGroupDatasets(std::string groupnm, std::vector<std::string>* datasetnm, long dtype);
	long OpenFileIO(std::wstring pathAndFilename, long openType);
	long CheckGroupDataset(const char* groupnm,const char* datasetnm,unsigned __int64 &size);
	long GetGroupDatasets(std::string path, char *** gpNames, long * gpNum, char *** dsNames, long * dsNum);
	long ExtendData(const char* groupnm,const char* datasetnm,void* buf,long datatype, bool extend,unsigned long writesize);
	long WriteData(const char* groupnm, const char* datasetnm,void* buf,long datatype,unsigned __int64 startLoc,unsigned long extsize);
	long ReadData(const char* groupnm, const char* datasetnm,void* buf,long datatype, unsigned __int64 start,unsigned __int64 readsize);
	long GetDataSetName(const char* groupnm, char** datasetnm, unsigned __int64 * nmLen);
	long CloseFileIO();
	long DestroyFileIO();

	~HDF5IO()
	{
		instanceFlag = false;
	}

private:
	std::string ConvertWStringToString(std::wstring ws);
};
