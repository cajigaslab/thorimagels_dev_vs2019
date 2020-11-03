// HDF5io.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "hdf5io.h"

HDF5IO::HDF5IO()
{	
	offset[0]=0;
	offset[1]=0;
	_pathAndFilename = ( L"" );
}

///static members:
const hsize_t maxdims[2] = {H5S_UNLIMITED, H5S_UNLIMITED};

bool HDF5IO::instanceFlag = false;
std::unique_ptr<HDF5IO> HDF5IO::single;
hsize_t HDF5IO::_bufsize = 0;
hsize_t HDF5IO::dims[RANK]={0,1};
hsize_t HDF5IO::cmprdims[RANK]={0,1};
hsize_t HDF5IO::dimsNow[RANK]={0,1};
hsize_t HDF5IO::offset[RANK]={0,0};
std::unique_ptr<H5::H5File> HDF5IO::_file;
std::wstring HDF5IO::_pathAndFilename;

///singleton:
HDF5IO* HDF5IO::getInstance()
{
	if(!instanceFlag)
	{
		try
		{
			single.reset(new HDF5IO());
			instanceFlag = true;
			return single.get();
		}
		catch(...)
		{
			throw;
		}
		return single.get();
	}
	else
	{
		return single.get();
	}
}

///find out the current length of data:
long HDF5IO::CheckGroupDataset(const char* groupnm, const char* datasetnm,unsigned __int64 &size)
{
	long ret = TRUE;
	std::string tmpName = groupnm;
	strcpy_s<_MAX_FNAME>(tmpStr,datasetnm);
	if(tmpStr[0] != '/')
	{	
		tmpName += '/';	
	}
	tmpName += tmpStr;

	try
	{
		if((_file.get() != nullptr) && (_file.get()->getId() > 0))
		{

			H5::DataSet _dataset = _file.get()->openDataSet(tmpName.c_str());
			H5::DataSpace _filespace = _dataset.getSpace();
			_filespace.getSimpleExtentDims(dimsNow,NULL);
			size = static_cast<unsigned __int64>(dimsNow[0]);

			_dataset.close();
			_dataset.~DataSet();
			_filespace.close();
			_filespace.~DataSpace();
		}
		else
		{
			ret = FALSE;
		}
	}
	catch(H5::FileIException not_found_error)
	{
		ret = FALSE;
	}

	return ret;
}

///close HDF5 file:
long HDF5IO::CloseFileIO()
{
	try
	{
		if((_file.get() != nullptr) && (_file.get()->getId()>0))
		{
			_file.get()->close();
		}
		return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}
}

std::string HDF5IO::ConvertWStringToString(std::wstring ws)
{

	size_t origsize = wcslen(ws.c_str()) + 1;
	const size_t newsize = _MAX_PATH;
	size_t convertedChars = 0;
	char nstring[newsize];
	wcstombs_s(&convertedChars, nstring, origsize, ws.c_str(), _TRUNCATE);

	std::string str(nstring);

	return str;
}

///create by TRUNC will overwrite if existing, use OpenFileIO to re-open file instead:
long HDF5IO::CreateFileIO(std::wstring pathAndFilename, long ftype, unsigned __int64 bufsize)
{
	long ret = TRUE;

	std::string _filename = ConvertWStringToString(pathAndFilename);
	//_filename += ".h5";

	try
	{
		_bufsize = bufsize;
		dims[0]=_bufsize;
		dims[1]=1;

		if(bufsize>1)
		{
			cmprdims[0]=_bufsize/2;
			cmprdims[1]=1;
		}
		else
		{
			cmprdims[0]=1;
			cmprdims[1]=1;
		}

		switch(ftype)
		{
		case 0:	//create
			_file.reset(new H5::H5File(_filename.c_str(), H5F_ACC_TRUNC));
			break;
		case 1:	//read
			_file.reset(new H5::H5File(_filename.c_str(), H5F_ACC_RDONLY));
			break;
		case 2:	//write
			_file .reset(new H5::H5File(_filename.c_str(), H5F_ACC_RDWR));
			break;
		default:
			break;
		}	
		if(_file.get()->getId() <= 0)
		{	
			return FALSE;	
		}
	}
	catch(H5::FileIException not_found_error)
	{
		ret = FALSE;
	}
	return ret;
}

///create group and dataset nodes:
long HDF5IO::CreateGroupDatasets(std::string groupnm, std::vector<std::string>* datasetnm, long dtype)
{
	long ret = TRUE;

	try
	{
		if((_file.get() != nullptr) && (_file.get()->getId() > 0))
		{
			//create Group:
			H5::Group _group;
			if(groupnm.length() > 0)
			{
				_group =  _file.get()->createGroup(groupnm.c_str(),0);
			}
			//create Dataset:
			H5::DataSpace _dataspace( RANK, dims, maxdims);
			H5::DSetCreatPropList ds_creatplist(H5P_DATASET_CREATE);
			ds_creatplist.setChunk( RANK, cmprdims );					// then modify it for compression in the future
			//ds_creatplist.setDeflate( 6 );	

			for (int i=0;i<datasetnm->size();i++)
			{			
				std::string tmpstr = groupnm + datasetnm->at(i);
				H5::DataSet _dataset;

				switch (dtype)
				{	
				case 1:
					_dataset = _file.get()->createDataSet(tmpstr.c_str(), H5::PredType::NATIVE_DOUBLE,_dataspace,ds_creatplist);
					break;
				case 2:
					_dataset = _file.get()->createDataSet(tmpstr.c_str(), H5::PredType::NATIVE_UINT32,_dataspace,ds_creatplist);
					break;
				case 3:
					_dataset = _file.get()->createDataSet(tmpstr.c_str(), H5::PredType::NATIVE_UINT64,_dataspace,ds_creatplist);
					break;
				case 4:
					_dataset = _file.get()->createDataSet(tmpstr.c_str(), H5::PredType::NATIVE_FLOAT,_dataspace,ds_creatplist);
					break;
				case 5:
					_dataset = _file.get()->createDataSet(tmpstr.c_str(), H5::PredType::NATIVE_UCHAR,_dataspace,ds_creatplist);
					break;
				default:
					ret = FALSE;
					break;
				}
				_dataset.close();
				_dataset.~DataSet();

			}
			_dataspace.close();
			_dataspace.~DataSpace();
			ds_creatplist.close();
			ds_creatplist.~DSetCreatPropList();
			_group.close();
			_group.~Group();

		}
		else
		{
			ret = FALSE;
		}
	}
	catch(H5::FileIException not_found_error)
	{
		ret = FALSE;
	}
	catch(H5::DataSetIException err)
	{
		err.printError();
		ret = FALSE;
	}		
	return ret;
}

///reset singleton:
long HDF5IO::DestroyFileIO()
{
	if(_file.get() != nullptr)
	{
		_file.get()->H5::H5File::~H5File();	
		_file.release();
	}	
	instanceFlag = false;			
	return TRUE;
}

///extend data from the last:
long HDF5IO::ExtendData(const char* groupnm,const char* datasetnm,void* buf,long datatype, bool extend,unsigned long writesize)
{
	long ret = TRUE;
	hsize_t extdims[RANK] ={writesize,1};
	offset[0] = 0;
	std::string tmpName = groupnm;
	strcpy_s<_MAX_FNAME>(tmpStr,datasetnm);
	if(tmpStr[0] != '/')
	{	
		tmpName += '/';	
	}
	tmpName += tmpStr;

	try
	{
		if((_file.get() != nullptr) && (_file.get()->getId() > 0))
		{
			H5::DataSet _dataset = _file.get()->openDataSet(tmpName.c_str());
			//get current dataset size:
			H5::DataSpace _filespace = _dataset.getSpace();
			_filespace.getSimpleExtentDims(dimsNow,NULL);

			//prepare offset and extend size:
			if(extend)
			{
				offset[0]=dimsNow[0];
			}
			dimsNow[0] = offset[0] + extdims[0];
			_dataset.extend(dimsNow);				

			//get extended dataset size:
			_filespace = _dataset.getSpace();
			_filespace.getSimpleExtentDims(dimsNow,NULL);

			//setup data and memory space:
			_filespace.selectHyperslab(H5S_SELECT_SET,extdims,offset,NULL,NULL);
			H5::DataSpace _dataspace = H5::DataSpace(RANK,extdims,NULL);

			//write to file based on data type:
			switch (datatype)
			{
			case 1:
				_dataset.write(buf,H5::PredType::NATIVE_DOUBLE,_dataspace,_filespace,H5P_DEFAULT);//static_cast<double*>
				break;
			case 2:
				_dataset.write(buf,H5::PredType::NATIVE_UINT32,_dataspace,_filespace,H5P_DEFAULT);//static_cast<unsigned long*>
				break;
			case 3:				
				_dataset.write(buf,H5::PredType::NATIVE_UINT64,_dataspace,_filespace,H5P_DEFAULT);//static_cast<unsigned long long*>
				break;
			case 4:				
				_dataset.write(buf,H5::PredType::NATIVE_FLOAT,_dataspace,_filespace,H5P_DEFAULT);//static_cast<float*>
				break;
			case 5:				
				_dataset.write(buf,H5::PredType::NATIVE_UCHAR,_dataspace,_filespace,H5P_DEFAULT);
				break;
			default:
				ret = FALSE;
				break;
			}
			_dataset.close();
			_dataset.~DataSet();
			_dataspace.close();
			_dataspace.~DataSpace();
			_filespace.close();
			_filespace.~DataSpace();
		}
		else
		{
			ret = FALSE;
		}
	}
	catch(H5::DataSpaceIException not_access_error)
	{		
		ret = FALSE;
	}
	catch(H5::DataSetIException err)
	{
		err.printError();
		ret = FALSE;
	}
	return ret;
}

struct opdata 
{
	std::vector<std::string> groupnames;
	std::vector<std::string> datasetnames;
};
herr_t file_info (hid_t loc_id, const char *name, void *operator_data)
{
	herr_t          status, return_val = 0;
	H5G_stat_t      statbuf;
	struct opdata   *od = (struct opdata *) operator_data;

	//Get type of the object:
	status = H5Gget_objinfo (loc_id, name, 0, &statbuf);
	switch (statbuf.type) 
	{
	case H5G_GROUP:
		od->groupnames.push_back(name);
		break;
	case H5G_DATASET:
		od->datasetnames.push_back(name);
		break;
	case H5G_TYPE:
		break;
	}

	return return_val;
}

///find out group or dataset names at provided path:
long HDF5IO::GetGroupDatasets(std::string path, char *** gpNames, long * gpNum, char *** dsNames, long * dsNum)
{
	long ret = TRUE;

	try
	{
		if((_file.get() != nullptr) && (_file.get()->getId() > 0))
		{		
			opdata odata;
			herr_t idx = H5Giterate(_file.get()->getId(),(const char*)path.c_str(),NULL,file_info, (void *)&odata);
			long strLen;
			*gpNum = (long)odata.groupnames.size();
			*gpNames = (char **)CoTaskMemAlloc(sizeof(char *) * (*gpNum));

			for(int i = 0; i < *gpNum; i++)
			{
				strLen = (long)strlen(odata.groupnames[i].c_str());
				*(*gpNames + i) = (char *)CoTaskMemAlloc(sizeof(char) * (strLen + 1));
				memcpy(*(*gpNames + i), odata.groupnames[i].c_str(), strLen);
				(*(*gpNames + i))[strLen] = '\0';
			}

			////////////////////////////////
			*dsNum = (long)odata.datasetnames.size();
			*dsNames = (char **)CoTaskMemAlloc(sizeof(char *) * (*dsNum));

			for(int i = 0; i < *dsNum; i++)
			{
				strLen = (long)strlen(odata.datasetnames[i].c_str());
				*(*dsNames + i) = (char *)CoTaskMemAlloc(sizeof(char) * (strLen + 1));
				memcpy(*(*dsNames + i), odata.datasetnames[i].c_str(), strLen);
				(*(*dsNames + i))[strLen] = '\0';
			}
		}
		else
		{
			ret = FALSE;
		}
	}
	catch(H5::FileIException not_found_error)
	{
		ret = FALSE;
	}

	return ret;
}

///retrieve file path and name:
long HDF5IO::GetPathandFilename(std::wstring &pathandFilename)
{
	pathandFilename = _pathAndFilename;
	return TRUE;
}

///access created file for read and write:
long HDF5IO::OpenFileIO(std::wstring pathAndFilename, long openType)
{	
	long ret = TRUE;
	std::string _filename = ConvertWStringToString(pathAndFilename);
	//_filename += ".h5";
	try
	{
		switch(openType)
		{
		case 1:	//read
			_file.reset(new H5::H5File(_filename.c_str(), H5F_ACC_RDONLY));
			break;
		case 2:	//write
			_file.reset(new H5::H5File(_filename.c_str(), H5F_ACC_RDWR));
			break;
		default:
			ret = FALSE;
			break;
		}
		if(_file.get()->getId() <= 0)
		{   
			ret = FALSE;  
		}	
	}
	catch(H5::FileIException not_found_error)
	{
		ret = FALSE;
	}

	return ret;	
}

///read data from specified start location for size:
long HDF5IO::ReadData(const char* groupnm,const char* datasetnm,void* buf,long datatype, unsigned __int64 startLoc,unsigned __int64 readsize)
{
	long ret = TRUE;
	hsize_t tmpoffset[RANK]={startLoc,0};
	hsize_t readdims[RANK]={readsize,1};
	std::string tmpName = groupnm;
	strcpy_s<_MAX_FNAME>(tmpStr,datasetnm);
	if(tmpStr[0] != '/')
	{	
		tmpName += '/';	
	}
	tmpName += tmpStr;

	try
	{
		if((_file.get() != nullptr) && (_file.get()->getId() > 0))
		{
			H5::DataSet _dataset = _file.get()->openDataSet(tmpName.c_str());
			H5::DataSpace _filespace = _dataset.getSpace();
			_filespace.getSimpleExtentDims(dimsNow,NULL);
			_filespace.selectHyperslab(H5S_SELECT_SET,readdims,tmpoffset,NULL,NULL);
			H5::DataSpace _dataspace = H5::DataSpace(RANK,readdims,NULL);

			switch(datatype)
			{
			case 1:
				_dataset.read(buf,H5::PredType::NATIVE_DOUBLE,_dataspace,_filespace,H5P_DEFAULT);	//static_cast<double*>
				break;
			case 2:
				_dataset.read(buf,H5::PredType::NATIVE_UINT32,_dataspace,_filespace,H5P_DEFAULT);	//static_cast<unsigned long*>
				break;
			case 3:
				_dataset.read(buf,H5::PredType::NATIVE_UINT64,_dataspace,_filespace,H5P_DEFAULT);	//static_cast<unsigned long long*>
				break;
			case 4:
				_dataset.read(buf,H5::PredType::NATIVE_FLOAT,_dataspace,_filespace,H5P_DEFAULT);	//static_cast<float*>
				break;
			case 5:
				_dataset.read(buf,H5::PredType::NATIVE_UCHAR,_dataspace,_filespace,H5P_DEFAULT);	
				break;
			default:
				ret = FALSE;
				break;
			}
			_dataset.close();
			_dataset.~DataSet();
			_dataspace.close();
			_dataspace.~DataSpace();
			_filespace.close();
			_filespace.~DataSpace();
		}
		else
		{
			ret = FALSE;
		}
	}
	catch(H5::DataSpaceIException not_access_error)
	{
		ret = FALSE;
	}
	catch(H5::DataSetIException err)
	{
		err.printError();
		ret = FALSE;
	}
	return ret;
}

///set file path and name:
long HDF5IO::SetPathandFilename(std::wstring pathAndFilename)
{
	_pathAndFilename = pathAndFilename;
	return TRUE;
}

long HDF5IO::GetDataSetName(const char* groupnm, char** datasetnm, unsigned __int64 * nmLen)
{
	return TRUE;
}

///write or overwrite data from specified start location for size:
long HDF5IO::WriteData(const char* groupnm,const char* datasetnm,void* buf,long datatype, unsigned __int64 startLoc,unsigned long writesize)
{
	long ret = TRUE;
	hsize_t extdims[RANK] ={writesize,1};
	offset[0]=startLoc;
	std::string tmpName = groupnm;
	strcpy_s<_MAX_FNAME>(tmpStr,datasetnm);
	if(tmpStr[0] != '/')
	{	
		tmpName += '/';
	}
	tmpName += tmpStr;

	try
	{
		if((_file.get() != nullptr) && (_file.get()->getId() > 0))
		{
			H5::DataSet _dataset = _file.get()->openDataSet(tmpName.c_str());
			//get current dataset size:
			H5::DataSpace _filespace = _dataset.getSpace();
			_filespace.getSimpleExtentDims(dimsNow,NULL);

			//prepare offset and extend size:			
			dimsNow[0] = offset[0] + extdims[0];
			_dataset.extend(dimsNow);				

			//get extended dataset size:
			_filespace = _dataset.getSpace();
			_filespace.getSimpleExtentDims(dimsNow,NULL);

			//setup data and memory space:
			_filespace.selectHyperslab(H5S_SELECT_SET,extdims,offset,NULL,NULL);
			H5::DataSpace _dataspace = H5::DataSpace(RANK,extdims,NULL);

			//write to file based on data type:
			switch (datatype)
			{
			case 1:
				_dataset.write(buf,H5::PredType::NATIVE_DOUBLE,_dataspace,_filespace,H5P_DEFAULT);
				break;
			case 2:
				_dataset.write(buf,H5::PredType::NATIVE_UINT32,_dataspace,_filespace,H5P_DEFAULT);
				break;
			case 3:				
				_dataset.write(buf,H5::PredType::NATIVE_UINT64,_dataspace,_filespace,H5P_DEFAULT);
				break;
			case 4:				
				_dataset.write(buf,H5::PredType::NATIVE_FLOAT,_dataspace,_filespace,H5P_DEFAULT);
				break;
			case 5:				
				_dataset.write(buf,H5::PredType::NATIVE_UCHAR,_dataspace,_filespace,H5P_DEFAULT);
				break;
			default:
				ret = FALSE;
				break;
			}
			_dataset.close();
			_dataset.~DataSet();
			_dataspace.close();
			_dataspace.~DataSpace();
			_filespace.close();
			_filespace.~DataSpace();
		}
		else
		{
			ret = FALSE;
		}
	}
	catch(H5::DataSpaceIException not_access_error)
	{
		ret = FALSE;
	}
	catch(H5::DataSetIException err)
	{
		err.printError();
		ret = FALSE;
	}
	return ret;
}


