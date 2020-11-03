// ImageManger.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Image.h"
#include "ImageManager.h"

ImageManager::ImageManager()
{    

}

ImageManager::~ImageManager()
{
	instanceFlag = false;

	for(map<long,Image*>::const_iterator it = imageMap.begin(); it != imageMap.end(); ++it)
	{
		delete it->second;
	}

	imageMap.clear();
}

void ImageManager::cleanup(void)
{
	Lock lock(critSect);

	if(single != NULL)
	{
		delete single;
	}
}

bool ImageManager::instanceFlag = false;

long ImageManager::idCounter = 1;

ImageManager* ImageManager::single = NULL;

CritSect ImageManager::critSect;

ImageManager* ImageManager::getInstance()
{
	Lock lock(critSect);

	if(! instanceFlag)
	{
		try
		{
			single = new ImageManager();
		}
		catch(...)
		{
			//critically low on resources
			//do not proceed with application
			throw;
		}
		instanceFlag = true;
		atexit(cleanup);
		return single;
	}
	else
	{
		return single;
	}
}

long ImageManager::CreateImage(long &imageID,Dimensions d)
{
	Lock lock(critSect);

	Image *image;
	try
	{
		image = new Image(d,_memMapPath);
	}
	catch(...)
	{
		//bad memory allocation
		imageID = 0;
		return FALSE;
	}

	imageMap.insert(std::pair<long,Image*>(idCounter,image));
	imageID = idCounter;
	idCounter++;

	return TRUE;
}

long ImageManager::CreateImage(long &imageID,Dimensions d,wstring tFName)
{
	Lock lock(critSect);
	wchar_t tempFileName[_MAX_FNAME];			
	Image *image;
	try
	{
		imageID = idCounter;
		long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
		std::wstringstream imgNameFormat;
		imgNameFormat << L"%s_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tmp";
		StringCbPrintfW(tempFileName,_MAX_FNAME,imgNameFormat.str().c_str(),tFName.c_str(),imageID);
		image = new Image(d,_memMapPath,wstring(tempFileName));	
	}
	catch(...)
	{
		//bad memory allocation
		imageID = 0;
		return FALSE;
	}

	imageMap.insert(std::pair<long,Image*>(idCounter,image));
	//imageMap.push_back(image);
	idCounter++;

	return TRUE;
}

long ImageManager::CreateImage(long &imageID, Dimensions d, wstring tFName, long index, long subWell)
{
	Lock lock(critSect);
	wchar_t tempFileName[_MAX_FNAME];			
	Image *image;
	try
	{
		imageID = idCounter;
		long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
		std::wstringstream imgNameFormat;
		imgNameFormat << L"%s_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.raw";
		StringCbPrintfW(tempFileName,_MAX_FNAME,imgNameFormat.str().c_str(),tFName.c_str(),index,subWell);
		image = new Image(d,_memMapPath,wstring(tempFileName));	
	}
	catch(...)
	{
		//bad memory allocation
		imageID = 0;
		return FALSE;
	}

	imageMap.insert(std::pair<long,Image*>(idCounter,image));
	//imageMap.push_back(image);
	idCounter++;

	return TRUE;
}

long ImageManager::DestroyImage(long imageID)
{	
	Lock lock(critSect);

	Image *image;
	image = imageMap[imageID];

	if(NULL == image)
	{
		return FALSE;
	}

	delete image;
	imageMap.erase(imageID);

	return TRUE;

}

long ImageManager::OpImage(long imageID, IOp *op)
{
	Lock lock(critSect);

	Image *image;
	image = imageMap[imageID];

	if(NULL == image)
	{
		return FALSE;
	}

	return op->Execute(image);
}

long ImageManager::GetImage(long imageID, IImage *image)
{
	Lock lock(critSect);

	image = imageMap[imageID];

	if(NULL == image)
	{
		return FALSE;
	}
	return TRUE;
}

char *ImageManager::GetImagePtr(long imageID, long c,long m, long z, long t)
{
	Lock lock(critSect);

	return imageMap[imageID]->GetDataPtr(c,m,z,t);
}

char *ImageManager::GetImagePtr(long imageID, long c,long m, long z, long t, UINT64 offset)
{
	Lock lock(critSect);

	return imageMap[imageID]->GetDataPtr(c,m,z,t, offset);
}

void ImageManager::UnlockImagePtr(long imageID, long c,long m, long z, long t)
{
	Lock lock(critSect);

	imageMap[imageID]->UnlockDataPtr(c,m,z,t);
}

void ImageManager::SetMemMapPath(const WCHAR *path)
{
	_memMapPath = path;
}

wstring ImageManager::GetImagePath(long imageID)
{
	Lock lock(critSect);

	return imageMap[imageID]->GetImagePath();
}