#ifdef HDF5DLL_EXPORTS
#define DllExport_HDF5IO extern "C" __declspec( dllexport )
#else
#define DllExport_HDF5IO __declspec(dllimport)
#endif


DllExport_HDF5IO long CloseFileIO();
DllExport_HDF5IO long CheckGroupDataset(const char* groupnm,const char* datasetnm,unsigned __int64 &size);
DllExport_HDF5IO long CreateFileIO(const wchar_t * pathAndFilename, long ftype, unsigned __int64 bufsize);
DllExport_HDF5IO long CreateGroupDatasets(const char* groupnm, const char** datasetnm,long dsetnum, long dtype);
DllExport_HDF5IO long DestroyFileIO();
DllExport_HDF5IO long ExtendData(const char* groupnm,const char* datasetnm,void* buf,long datatype, bool extend,unsigned long writesize);
DllExport_HDF5IO long GetPathandFilename(wchar_t * path, long size);
DllExport_HDF5IO long GetGroupDatasetNames(const char* path, char *** gpNames, long * gpNum, char *** dsNames, long * dsNum);
DllExport_HDF5IO long OpenFileIO(const wchar_t * pathAndFilename, long openType);
DllExport_HDF5IO long ReadData(const char* groupnm, const char* datasetnm,void* buf,long datatype, unsigned __int64 start,unsigned __int64 readsize);
DllExport_HDF5IO long SetPathandFilename(const wchar_t * pathAndFilename);
DllExport_HDF5IO long WriteData(const char* groupnm, const char* datasetnm,void* buf,long datatype,unsigned __int64 startLoc,unsigned long extsize);
DllExport_HDF5IO long GetDataSetName(const char* groupnm, char** datasetnm, unsigned __int64 * nmLen);


