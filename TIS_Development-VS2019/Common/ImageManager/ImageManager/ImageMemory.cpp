#include "stdafx.h"
#include "ImageMemory.h"

ImageMemory::ImageMemory()
{
	hFileArray = NULL;
	hFileMapArray = NULL;
	ptr = NULL;
	ptrContiguous = NULL;

	//initialize the memory mapped file output to the temp directory for the current user
	DWORD result = ::GetTempPath(0, L"");

	if(result != 0)
	{
		std::vector<TCHAR> tempPath(result + 1);
		result = ::GetTempPath(static_cast<DWORD>(tempPath.size()), &tempPath[0]);
		wstring str(tempPath.begin(), tempPath.begin() + static_cast<std::size_t>(result));

		_memMapPath = str;
	}
}

ImageMemory::~ImageMemory()
{
	if(ptr != NULL)
	{
		switch(memoryType)
		{

		case DETACHED_CHANNEL:
			{
				//c - m - z - t

				long ic;
				long im;
				long iz;
				long it;

				long chan = 0;
				for(it=0; it<nT; it++)
				{
					for(iz=0; iz<nZ; iz++)
					{
						for(im=0; im<nM; im++)
						{
							for(ic=0; ic<nC; ic++)
							{
								delete[] ptr[chan];							
								chan++;
							}
						}
					}
				}

				delete[] ptr;
				ptr = NULL;
			}
			break;

		case CONTIGUOUS_CHANNEL:
			{
				delete[] ptrContiguous;
				ptrContiguous = NULL;
				delete[] ptr;
				ptr = NULL;
			}
			break;
			//Memory mapped logic
		case CONTIGUOUS_CHANNEL_MEM_MAP:
		case CONTIGUOUS_CHANNEL_MEM_MAP_NO_FILE_DELETE:
			{		
				DWORD err;
				if (NULL != hFileMapArray)
				{
					if(FALSE == CloseHandle(hFileMapArray[0]))
					{
						err = GetLastError();
					}
					delete[] hFileMapArray;
					hFileMapArray = NULL;
				}
				if (NULL != hFileArray)
				{
					if(FALSE == CloseHandle(hFileArray[0]))
					{
						err = GetLastError();
					}
					delete[] hFileArray;
					hFileArray = NULL;
				}
				if(CONTIGUOUS_CHANNEL_MEM_MAP == memoryType)
				{
					if(FALSE == DeleteFile(fileNames[0].c_str()))
					{
						err = GetLastError();
					}
				}

				fileNames.clear();
				delete[] ptr;
			}
			break;
			//case DETACHED_CHANNEL_MEM_MAP:
			//	{
			//		//c - m - z - t
			//		long ic;
			//		long im;
			//		long iz;
			//		long it;

			//		long chan = 0;
			//		for(it=0; it<nT; it++)
			//		{
			//			for(iz=0; iz<nZ; iz++)
			//			{
			//				for(im=0; im<nM; im++)
			//				{
			//					for(ic=0; ic<nC; ic++)
			//					{
			//						CloseHandle(hFileMapArray[chan]);
			//						UnmapViewOfFile(ptr[chan]);
			//						CloseHandle(hFileArray[chan]);
			//						DeleteFile(fileNames[chan].c_str());
			//						chan++;
			//					}
			//				}
			//			}
			//		}

			//		fileNames.clear();
			//		delete ptr;
			//	}
			//	break;
		}
	}
}

long ImageMemory::AllocateMem(MemoryType memType, DataType dType, long x, long y, long c,long z,long m,long t,long bufferType,wstring memMapPath)
{
	if(
		(x < XMIN)||
		(x > XMAX)||
		(y < YMIN)||
		(y > YMAX)||
		(c < CMIN)||
		(c > CMAX)||
		(m < MMIN)||
		(m > MMAX)||
		(z < ZMIN)||
		(z > ZMAX)||
		(t < TMIN)||
		(t > TMAX)||
		(memType < MEMORYTYPE_MIN)||
		(memType >= MEMORYTYPE_MAX)||
		(dType < DATATYPE_MIN)||
		(dType >= DATATYPE_MAX)
		)
	{
		return FALSE; 
	}


	switch(memType)
	{
	case DETACHED_CHANNEL:
		{
			//c - m - z - t
			try
			{
				long byteSize;
				if(FALSE == GetDataTypeSizeInBytes(dType,byteSize) || (NULL != ptr))
				{
					return FALSE;
				}

				long numChannels = c*z*m*t;

				ptr = new char*[numChannels];

				nX = x;
				nY = y;
				nC = c;
				nM = m;
				nZ = z;
				nT = t;

				memoryType = memType;
				dataType = dType;
				imageBufferType = bufferType;

				long ic;
				long im;
				long iz;
				long it;

				long frameSize = (x*y)*byteSize;

				offsetC = frameSize;
				offsetM = frameSize*static_cast<LONGLONG>(c);
				offsetZ = frameSize*static_cast<LONGLONG>(c*m);
				offsetT = frameSize*static_cast<LONGLONG>(c*m*z);

				long chan = 0;
				for(it=0; it<nT; it++)
				{
					for(iz=0; iz<nZ; iz++)
					{
						for(im=0; im<nM; im++)
						{
							for(ic=0; ic<nC; ic++)
							{
								ptr[chan] = new char[frameSize];								
								chan++;
							}
						}
					}
				}
			}
			catch(...)
			{
				//system low on resources. unable to fully allocate image buffer
				return FALSE;
			}
		}
		break;

	case CONTIGUOUS_CHANNEL:
		{
			//c - m - z - t
			try
			{
				long byteSize;
				if(FALSE == GetDataTypeSizeInBytes(dType,byteSize) || (NULL != ptr) || (NULL != ptrContiguous))
				{
					return FALSE;
				}

				long numChannels = c*z*m*t;

				ptr = new char*[numChannels];

				nX = x;
				nY = y;
				nC = c;
				nM = m;
				nZ = z;
				nT = t;

				memoryType = memType;
				dataType = dType;
				imageBufferType = bufferType;

				long ic;
				long im;
				long iz;
				long it;

				long frameSize = (x*y)*byteSize;

				offsetC = frameSize;
				offsetM = frameSize*static_cast<LONGLONG>(c);
				offsetZ = frameSize*static_cast<LONGLONG>(c*m);
				offsetT = frameSize*static_cast<LONGLONG>(c*m*z);

				ptrContiguous = new char[frameSize*nC*nM*nZ*nT];

				char * p = ptrContiguous;

				long chan = 0;

				for(it=0; it<nT; it++)
				{
					for(iz=0; iz<nZ; iz++)
					{
						for(im=0; im<nM; im++)
						{
							for(ic=0; ic<nC; ic++)
							{
								ptr[chan] = p;
								p += frameSize;								
								chan++;
							}
						}
					}
				}
			}
			catch(...)
			{
				//system low on resources. unable to fully allocate image buffer
				return FALSE;
			}
		}
		break;
	case CONTIGUOUS_CHANNEL_MEM_MAP:
	case CONTIGUOUS_CHANNEL_MEM_MAP_NO_FILE_DELETE:
		{
			//c - m - z - t
			try
			{
				long byteSize;
				if(FALSE == GetDataTypeSizeInBytes(dType,byteSize) || (NULL != ptr))
				{
					return FALSE;
				}

				long numChannels = c*z*m*t;

				ptr = new char*[numChannels];

				hFileArray = new HANDLE[1];
				hFileMapArray = new HANDLE[1];

				nX = x;
				nY = y;
				nC = c;
				nM = m;
				nZ = z;
				nT = t;

				memoryType = memType;
				dataType = dType;
				imageBufferType = bufferType;

				long frameSize = (x*y)*byteSize;

				offsetC = frameSize;
				offsetM = frameSize*static_cast<LONGLONG>(c);
				offsetZ = frameSize*static_cast<LONGLONG>(c*m);
				offsetT = frameSize*static_cast<LONGLONG>(c*m*z);

				TCHAR *guidStr = 0x00;

				wchar_t uniqueFileName[_MAX_PATH];

				//if a path is passed in use it. otherwise default to standard temp path
				if(memMapPath.size() > 0)
				{	_memMapPath = memMapPath;	}

				if(_tempFileName.size()>0)
				{	StringCbPrintfW(uniqueFileName,_MAX_PATH,L"%s%s",_memMapPath.c_str(),_tempFileName.c_str());	}
				else
				{	::GetTempFileName(_memMapPath.c_str(),L"tis",0,uniqueFileName);		}

				//Open file
				hFileArray[0] = CreateFile(uniqueFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH|FILE_FLAG_NO_BUFFERING ,NULL);	

				fileNames.push_back(wstring(uniqueFileName));


				DWORDLONG dwlX = x;
				DWORDLONG dwlY = y;
				DWORDLONG dwlByteSize = byteSize;
				DWORDLONG dwlC = nC;
				DWORDLONG dwlM = nM;
				DWORDLONG dwlZ = nZ;
				DWORDLONG dwlT = nT;


				DWORDLONG sizeLong =  frameSize*dwlC*dwlM*dwlZ*dwlT;
				DWORDLONG dwllowOrderSize = sizeLong & 0xFFFFFFFF;					//0xFFFFFFFFul	
				DWORDLONG dwlhighOrderSize = (sizeLong & 0xFFFFFFFF00000000)>>32;	//0xFFFFFFFFul	
				DWORD lowOrderSize = static_cast<DWORD>(dwllowOrderSize);
				DWORD highOrderSize = static_cast<DWORD>(dwlhighOrderSize);
				//Open file mapping called MySharedMapping
				hFileMapArray[0] = CreateFileMapping(hFileArray[0], NULL, PAGE_READWRITE, highOrderSize, lowOrderSize,_tempFileName.c_str());

				DWORD dwError;

				if(NULL == hFileMapArray[0])
				{	
					dwError = GetLastError();

					switch(dwError)
					{
					case ERROR_DISK_FULL:
						{
						}
						break;
					default:
						{
						}
					}
					CloseHandle(hFileMapArray[0]);
					delete[] hFileMapArray;
					hFileMapArray = NULL;
					CloseHandle(hFileArray[0]);
					delete[] hFileArray;
					hFileArray = NULL;
					return FALSE;
				}

				SYSTEM_INFO si;
				GetSystemInfo(&si);

				granularityOfMapView = si.dwAllocationGranularity;
			}
			catch(...)
			{
				//system low on resources. unable to fully allocate image buffer
				return FALSE;
			}
		}
		break;		
		//	//Memory map logic
		//case DETACHED_CHANNEL_MEM_MAP:
		//	{
		//		//c - m - z - t
		//		try
		//		{
		//			long byteSize;
		//			if(FALSE == GetDataTypeSizeInBytes(dType,byteSize))
		//			{
		//				return FALSE;
		//			}

		//			long numChannels = c*z*m*t;

		//			ptr = new char*[numChannels];

		//			hFileArray = new HANDLE[numChannels];
		//			hFileMapArray = new HANDLE[numChannels];

		//			nX = x;
		//			nY = y;
		//			nC = c;
		//			nM = m;
		//			nZ = z;
		//			nT = t;

		//			memoryType = memType;
		//			dataType = dType;

		//			long ic;
		//			long im;
		//			long iz;
		//			long it;

		//			offsetC = (x*y)*byteSize;
		//			offsetM = (x*y)*byteSize*c;
		//			offsetZ = (x*y)*byteSize*c*m;
		//			offsetT = (x*y)*byteSize*c*m*z;

		//			TCHAR *guidStr = 0x00;

		//			long chan = 0;
		//			for(it=0; it<nT; it++)
		//			{
		//				for(iz=0; iz<nZ; iz++)
		//				{
		//					for(im=0; im<nM; im++)
		//					{
		//						for(ic=0; ic<nC; ic++)
		//						{
		//							 DWORD result = ::GetTempPath(0, L"");

		//							 if(result != 0)
		//							 {
		//								 std::vector<TCHAR> tempPath(result + 1);
		//								 result = ::GetTempPath(static_cast<DWORD>(tempPath.size()), &tempPath[0]);
		//								 wstring str(tempPath.begin(), tempPath.begin() + static_cast<std::size_t>(result));

		//								 wchar_t uniqueFileName[_MAX_PATH];

		//								 ::GetTempFileName(str.c_str(),L"tis",0,uniqueFileName);

		//								 //Open file
		//								 hFileArray[chan] = CreateFile(uniqueFileName, GENERIC_READ | GENERIC_WRITE, 0,NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);

		//								 fileNames.push_back(wstring(uniqueFileName));

		//								 DWORD lowOrderSize = x*y*byteSize;
		//								 //Open file mapping called MySharedMapping
		//								 hFileMapArray[chan] = CreateFileMapping(hFileArray[chan], NULL, PAGE_READWRITE, 0, lowOrderSize,L"MySharedMapping");
		//								 //Get pointer to memory representing file
		//								 ptr[chan] = (char*)MapViewOfFile(hFileMapArray[chan], FILE_MAP_WRITE, 0, 0, 0);
		//							 }

		//							chan++;
		//						}
		//					}
		//				}
		//			}
		//		}
		//		catch(...)
		//		{
		//			//system low on resources. unable to fully allocate image buffer
		//			return FALSE;
		//		}
		//	}
		//	break;
		//Memory map logic

	default:
		{
			return FALSE;
		}		
	}

	return TRUE;

}

char *ImageMemory::GetMemPtr(long c, long m, long z, long t)
{
	long loc;

	switch(memoryType)
	{
	case DETACHED_CHANNEL:
		{
			loc = (t) + (z) + (m) + (c);
			return ptr[loc];
		}

	case CONTIGUOUS_CHANNEL:
		{
			loc = t*nZ*nM*nC + z*nM*nC + m*nC + c;
			//loc = (t) + (z) + (m) + (c);
			return ptr[loc];
		}
	case CONTIGUOUS_CHANNEL_MEM_MAP:
	case CONTIGUOUS_CHANNEL_MEM_MAP_NO_FILE_DELETE:
		{			
			long byteSize;
			if(FALSE == GetDataTypeSizeInBytes(dataType,byteSize))
			{
			}

			loc = (t) + (z) + (m) + (c);


			long chan = 0;

			DWORDLONG dwlByteSize = byteSize;
			DWORDLONG dwlC = c;
			DWORDLONG dwlM = m;
			DWORDLONG dwlZ = z;
			DWORDLONG dwlT = t;

			DWORDLONG dwlOffsetC =  offsetC;
			DWORDLONG dwlOffsetM =  offsetM;
			DWORDLONG dwlOffsetZ =  offsetZ;
			DWORDLONG dwlOffsetT =  offsetT;

			DWORDLONG mapOffset =  dwlOffsetT*dwlT + dwlOffsetZ*dwlZ + dwlOffsetM*dwlM + dwlOffsetC*dwlC;

			DWORDLONG val = mapOffset/granularityOfMapView;

			DWORDLONG allowedOffset = val * granularityOfMapView;

			DWORDLONG dwllowOrderOffset = allowedOffset & 0xFFFFFFFF;					//0xFFFFFFFFul
			DWORDLONG dwlhighOrderOffset = (allowedOffset & 0xFFFFFFFF00000000)>>32;	//0xFFFFFFFFul
			DWORD lowOffset = static_cast<DWORD>(dwllowOrderOffset);
			DWORD highOffset = static_cast<DWORD>(dwlhighOrderOffset);

			DWORDLONG memOffset = (mapOffset - allowedOffset);

			size_t dwBytesToMap = dwlOffsetC*static_cast<DWORDLONG>(nC) + memOffset;
			//Get pointer to memory representing file
			char *p = (char*)MapViewOfFile(hFileMapArray[0], FILE_MAP_ALL_ACCESS, highOffset, lowOffset, dwBytesToMap);

			if(p == NULL)
			{
				DWORD err = GetLastError();
				throw;
			}

			ptr[loc] = p;	

			char* retPtr = p + memOffset;

			return retPtr;
		}
	default:
		return NULL;
	}

	return NULL;
}

char *ImageMemory::GetMemPtr(long c, long m, long z, long t, UINT64 offset)
{
	long loc;

	switch(memoryType)
	{
	case DETACHED_CHANNEL:
	case CONTIGUOUS_CHANNEL:
		{
			//Need to update the allocate logic to allow to get a pointer
			//at any offset when in CONTIGUOUS_CHANNEL or DETACHED_CHANNEL
			throw;
		}
		break;
	case CONTIGUOUS_CHANNEL_MEM_MAP:
	case CONTIGUOUS_CHANNEL_MEM_MAP_NO_FILE_DELETE:
		{			
			long byteSize;
			if(FALSE == GetDataTypeSizeInBytes(dataType,byteSize))
			{
			}

			loc = (t) + (z) + (m) + (c);

			long chan = 0;

			DWORDLONG dwlByteSize = byteSize;
			DWORDLONG dwlC = c;
			DWORDLONG dwlM = m;
			DWORDLONG dwlZ = z;
			DWORDLONG dwlT = t;

			DWORDLONG dwlOffsetC =  offsetC;
			DWORDLONG dwlOffsetM =  offsetM;
			DWORDLONG dwlOffsetZ =  offsetZ;
			DWORDLONG dwlOffsetT =  offsetT;

			DWORDLONG mapOffset =  offset;

			DWORDLONG val = mapOffset/granularityOfMapView;

			DWORDLONG allowedOffset = val * granularityOfMapView;

			DWORDLONG dwllowOrderOffset = allowedOffset & 0xFFFFFFFF;					//0xFFFFFFFFul
			DWORDLONG dwlhighOrderOffset = (allowedOffset & 0xFFFFFFFF00000000)>>32;	//0xFFFFFFFFul
			DWORD lowOffset = static_cast<DWORD>(dwllowOrderOffset);
			DWORD highOffset = static_cast<DWORD>(dwlhighOrderOffset);

			DWORDLONG memOffset = (mapOffset - allowedOffset);

			size_t dwBytesToMap = dwlOffsetC*static_cast<DWORDLONG>(nC) + memOffset;
			//Get pointer to memory representing file
			char *p = (char*)MapViewOfFile(hFileMapArray[0], FILE_MAP_ALL_ACCESS, highOffset, lowOffset, dwBytesToMap);

			if(p == NULL)
			{
				DWORD err = GetLastError();
				throw;
			}

			ptr[loc] = p;	

			char* retPtr = p + memOffset;

			return retPtr;
		}
		//case DETACHED_CHANNEL_MEM_MAP:
		//	{
		//		loc = (t) + (z) + (m) + (c);
		//		return ptr[loc];
		//	}
	default:
		return NULL;
	}

	return NULL;
}

void ImageMemory::UnlockMemPtr(long c, long m, long z, long t)
{

	long loc;

	loc = (t) + (z) + (m) + (c);
	switch(memoryType)
	{
	case CONTIGUOUS_CHANNEL_MEM_MAP:
	case CONTIGUOUS_CHANNEL_MEM_MAP_NO_FILE_DELETE:
		{			
			long byteSize;

			if(FALSE == GetDataTypeSizeInBytes(dataType,byteSize))
			{
			}

			//FlushViewOfFile(ptr[loc-i],nX*nY*byteSize);
			if(FALSE == UnmapViewOfFile(ptr[loc]))
			{
				DWORD err = GetLastError();
			}

		}
		break;
	}
}

long ImageMemory::GetDataTypeSizeInBytes(DataType d,long &size)
{
	switch(d)
	{
	case INT_8BIT:
		size = sizeof(char);
		break;
	case INT_16BIT:
		size = sizeof(unsigned short)/sizeof(char);
		break;
	case FLOAT_32BIT:
		size = sizeof(float)/sizeof(char);
		break;
	case FLOAT_64BIT:
		size = sizeof(double)/sizeof(char);
		break;
	case INT_32BIT:
		size = sizeof(UINT32)/sizeof(char);
		break;
	case INT_64BIT:
		size = sizeof(UINT64)/sizeof(char);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

MemoryType ImageMemory::GetMemoryType()
{
	return memoryType;
}

long ImageMemory::setTempFileName(wstring tFileName)
{		
	_tempFileName = tFileName;
	return TRUE;
}

wstring ImageMemory::GetImageMemoryPath()
{
	wstring fileName = L"";
	if (!fileNames.empty())
	{
		fileName = fileNames.front();
	}

	return fileName;
}
