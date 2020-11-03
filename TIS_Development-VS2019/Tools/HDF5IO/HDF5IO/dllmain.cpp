#include "stdafx.h"
#include "HDF5dll.h"

// Local Prototypes
//void InitializeDll ();
//void CleanupDll ();

// Main DLL Entry Point
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	UNREFERENCED_PARAMETER(hModule);
	UNREFERENCED_PARAMETER(lpReserved);

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//InitializeDll();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		//CleanupDll ();
		break;
	}
	return TRUE;
}


// This is called when the DLL is created.
// It initializes any global variables.
//void InitializeDll ()
//{
//}

// This is called when the DLL is cleaned up.
// This detaches any ongoing connections and cleans up any global variables.
//void CleanupDll ()
//{
//}

DllExport_HDF5IO long CloseFileIO()
{
	return HDF5IO::getInstance()->CloseFileIO();
}

DllExport_HDF5IO long CheckGroupDataset(const char* groupnm,const char* datasetnm,unsigned __int64 &size)
{
	return HDF5IO::getInstance()->CheckGroupDataset(groupnm,datasetnm,size);
}

DllExport_HDF5IO long CreateFileIO(const wchar_t * pathAndFilename, long ftype, unsigned __int64 bufsize)
{
	std::wstring ws = pathAndFilename;
	return HDF5IO::getInstance()->CreateFileIO(ws,ftype,bufsize);
}

DllExport_HDF5IO long CreateGroupDatasets(const char* groupnm, const char** datasetnm, long dsetnum, long dtype)
{
	std::string str = groupnm;
	std::string dstr;
	std::vector<std::string> dsetname;
	for(int i=0; i<dsetnum; i++)
	{
		dstr = datasetnm[i];
		dsetname.push_back(dstr);
	}
	long ret = HDF5IO::getInstance()->CreateGroupDatasets(str,&dsetname,dtype);
	dsetname.clear();
	return ret;
}

DllExport_HDF5IO long DestroyFileIO()
{
	return HDF5IO::getInstance()->DestroyFileIO();
}

DllExport_HDF5IO long ExtendData(const char* groupnm,const char* datasetnm,void* buf,long datatype, bool extend,unsigned long writesize)
{
	return HDF5IO::getInstance()->ExtendData(groupnm,datasetnm,buf,datatype,extend,writesize);
}

DllExport_HDF5IO long GetGroupDatasetNames(const char* path, char *** gpNames, long * gpNum, char *** dsNames, long * dsNum)
{
	HDF5IO::getInstance()->GetGroupDatasets(path, gpNames, gpNum, dsNames, dsNum);
	return TRUE;
}

DllExport_HDF5IO long GetPathandFilename(wchar_t * path, long size)
{
	std::wstring ws;
	HDF5IO::getInstance()->GetPathandFilename(ws);
	wcscpy_s(path, size, ws.c_str());
	return TRUE;
}

DllExport_HDF5IO long OpenFileIO(const wchar_t * pathAndFilename, long openType)
{
	std::wstring ws = pathAndFilename;
	return HDF5IO::getInstance()->OpenFileIO(ws,openType);
}

DllExport_HDF5IO long ReadData(const char* groupnm, const char* datasetnm,void* buf,long datatype, unsigned __int64 start,unsigned __int64 readsize)
{
	return HDF5IO::getInstance()->ReadData(groupnm,datasetnm,buf,datatype,start,readsize);
}

DllExport_HDF5IO long SetPathandFilename(const wchar_t * pathAndFilename)
{
	std::wstring ws = pathAndFilename;
	return HDF5IO::getInstance()->SetPathandFilename(ws);
}

DllExport_HDF5IO long WriteData(const char* groupnm, const char* datasetnm,void* buf,long datatype,unsigned __int64 startLoc,unsigned long extsize)
{
	return HDF5IO::getInstance()->WriteData(groupnm,datasetnm,buf,datatype,startLoc,extsize);
}

DllExport_HDF5IO long GetDataSetName(const char* groupnm, char** datasetnm, unsigned __int64 * nmLen)
{
	return HDF5IO::getInstance()->GetDataSetName(groupnm, datasetnm, nmLen);
}