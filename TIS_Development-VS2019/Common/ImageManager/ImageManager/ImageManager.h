#pragma once

#include <map>
#include <vector>
#include "..\..\thread.h"

#if defined(IMAGE_MANAGER)
#define DllExport __declspec(dllexport)
#else
//    definitions used when using DLL
#define DllExport __declspec(dllimport)
#endif

class DllExport ImageManager
{
public:
	~ImageManager();
    static ImageManager* getInstance();
	long CreateImage(long &imageID, Dimensions d);
	long CreateImage(long &imageID, Dimensions d, wstring tFName);
	long CreateImage(long &imageID, Dimensions d, wstring tFName, long index, long subWell);
	long CreateImages(vector<long> imageIDs, vector<Dimensions> dimentions, wstring tFName, long index, long subWell);
	long DestroyImage(long imageID);
	long DestroyImages(vector<long> imageIDs);
	long OpImage(long imageID,IOp *op); 
	long GetImage(long imageID, IImage *image);
	char *GetImagePtr(long imageID, long c,long m, long z, long t);
	char *GetImagePtr(long imageID, long c,long m, long z, long t, UINT64 offset);
	void UnlockImagePtr(long imageID, long c,long m, long z, long t);
	void SetMemMapPath(const WCHAR *path);
	wstring GetImagePath(long imageID);

private:
    ImageManager();
	
//including an STL type in a dll exported class
//generates a warning. Ignoring due to visibility
//of the member
#pragma warning(push)
#pragma warning(disable:4251)

    static bool instanceFlag;
    static ImageManager *single;
	static CritSect critSect;
	static long idCounter;
	static void cleanup(void);

	wstring _memMapPath;

	std::map<long,Image*> imageMap;

#pragma warning(pop)
};