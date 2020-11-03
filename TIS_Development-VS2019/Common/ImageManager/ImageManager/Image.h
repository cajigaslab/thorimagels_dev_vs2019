#pragma once

#include "ImageMemory.h"
#include <memory>

using namespace std;

#if defined(IMAGE_MANAGER)
#define DllExport __declspec(dllexport)
#else
//    definitions used when using DLL
#define DllExport __declspec(dllimport)
#endif

struct Dimensions
{
	long x;
	long y;
	long c;
	long m;
	long z;
	long t;
	DataType dType;
	MemoryType mType;
	long imageBufferType;
};


class DllExport IImage
{
public:
	virtual long GetImageDimensions(Dimensions&) = 0;
	virtual char * GetDataPtr(long c,long m,long z,long t) = 0;
};

class IOp
{
public:
	virtual long Execute(IImage *) = 0;
};

class DllExport Image : public IImage
{
public:
	Image();
	Image(Dimensions,wstring memMapPath);
	Image(Dimensions,wstring memMapPath,wstring tFileName);
	~Image();
	Image(Image&);

	virtual long GetImageDimensions(Dimensions&);
	virtual char * GetDataPtr(long c,long m,long z,long t);
	virtual char * GetDataPtr(long c,long m,long z,long t, UINT64 offset);
	virtual void UnlockDataPtr(long c,long m,long z,long t);
	virtual wstring GetImagePath();
private:
	long x;
	long y;
	long c;
	long m;
	long z;
	long t;
	DataType dataType;
	long bufferType;

//including an STL type in a dll exported class
//generates a warning. Ignoring due to visibility
//of the member
#pragma warning(push)
#pragma warning(disable:4251)
	const auto_ptr<ImageMemory> pMem;
#pragma warning(pop)
};

class DummyOp : public IOp
{
public:
	DummyOp();
	~DummyOp();

	virtual long Execute(IImage *image);
};