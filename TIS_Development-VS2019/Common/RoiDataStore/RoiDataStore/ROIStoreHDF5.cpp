#include "stdafx.h"
#include "ROIStoreHDF5.h"

const long MAX_PATH_AND_NAME_LEN = 256;
std::unique_ptr<HDF5ioDLL> h5io(new HDF5ioDLL(L".\\Modules_Native\\HDF5IO.dll"));


ROIStoreHDF5::ROIStoreHDF5(char* pathAndName)
{
	wchar_t fn[MAX_PATH_AND_NAME_LEN];	
	size_t c;
	mbstowcs_s(&c, fn, pathAndName, strlen(pathAndName) + 1);
	h5io->SetPathandFilename(fn);
	h5io->GetPathandFilename(fn,MAX_PATH_AND_NAME_LEN);
	_fileName = fn;
	_isNewdata = FALSE;
	_numStats = 0;
	if(_fileName.size() < 0)
	{
		return;
	}
	if(h5io->CreateFileIO(fn, H5FileType::OVERWRITE, 0))
	{
		h5io->CloseFileIO();
	}
}

ROIStoreHDF5::~ROIStoreHDF5()
{
	h5io.get()->DestroyFileIO();
	//delete existing
	if(NULL != _statsName)
	{
		for(int i=0; i<_numStats; i++)
		{
			CoTaskMemFree(_statsName[i]);
		}

		CoTaskMemFree(_statsName);
		_statsName = NULL;
	}
}

void ROIStoreHDF5::SaveROIData(char** statsName, double* stats, long nStats, long isLast)
{
	//if there are stats
	if (0 < nStats)
	{
		//***Store in H5 File***//
		if(h5io->OpenFileIO(_fileName.c_str(), H5FileType::READWRITE))
		{
			for(int i = 0; i < nStats; i++)
			{				
				std::string gpTemp = '/' + std::string(*(statsName + i));
				unsigned __int64 tSize;
				if(!h5io->CheckGroupDataset(gpTemp.c_str(), gpTemp.c_str(), tSize))
				{
					const char * dsTemp = gpTemp.c_str();
					h5io->CreateGroupDatasets(gpTemp.c_str(), &dsTemp, 1, H5DataType::DATA_DOUBLE);
				}
				h5io->ExtendData(gpTemp.c_str(), gpTemp.c_str(), (stats + i), H5DataType::DATA_DOUBLE, TRUE, 1);
			}
			/*if(bIsLast) {*/h5io->CloseFileIO();/*}*/			
		}
	}

	//***Keep a local copy***//
	//delete existing
	if((NULL != _statsName)&&(_numStats != nStats) )
	{
		for(int i=0; i<_numStats; i++)
		{
			CoTaskMemFree(_statsName[i]);
		}

		CoTaskMemFree(_statsName);
		_statsName = NULL;
	}
	//if there are stats
	if(0 < nStats)
	{
		const long MAX_STATNAME_LEN = 32;
	
		//allocate if needed
		if(NULL == _statsName)
		{
			_statsName = (char**)CoTaskMemAlloc(sizeof(char *) * nStats);

			memset(_statsName,0,sizeof(char *) * nStats);
		
			for(int i=0; i<nStats; i++)
			{
				_statsName[i] =  (char*)CoTaskMemAlloc(MAX_STATNAME_LEN);
			}
		}

		for(int i=0; i<nStats; i++)
		{
			strcpy_s(_statsName[i],MAX_STATNAME_LEN,statsName[i]);
		}		
	}
	_stats = stats;
	_numStats = nStats;
	_isNewdata = TRUE; //flag used to check if there has been a new set of data
}

void ROIStoreHDF5::ReadROIData(char*** statsName, double** stats, long &len, long &isLast, long &isNewdata)
{
	if(FALSE == _isNewdata) //if there is no new data return;
	{
		isNewdata = FALSE;
		isLast = TRUE;
		return;
	}
	
	*statsName = _statsName;
	*stats = _stats;
	len = _numStats;

	isLast = TRUE;
	isNewdata = TRUE; //value going back to method caller
	_isNewdata = FALSE; //set to FALSE flag used to check if there has been a new set of data
	
}
