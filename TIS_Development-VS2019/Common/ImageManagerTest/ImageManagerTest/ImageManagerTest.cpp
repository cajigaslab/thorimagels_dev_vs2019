
#include <vector>
#include <string>

using namespace std;

#include "..\..\..\Tools\WinUnit\Include\WinUnit.h"
#include "..\..\ImageManager\ImageManager\Image.h"
#include "..\..\ImageManager\ImageManager\ImageManager.h"

#include <stdlib.h>
#include <time.h>

#include "tiffio.h"
#include "TiffLib.h"



FIXTURE(ImageManagerTestFixture);


SETUP(ImageManagerTestFixture)
{
	
	
}

TEARDOWN(ImageManagerTestFixture)
{
}


//Create Images
BEGIN_TESTF(CreateImage,ImageManagerTestFixture)
{
	long id;
	Dimensions d;


	d.x = 1024;
	d.y = 1024;
	
	const long ITERATIONS = 2;

	for(long y=512; y<2048; y+=512)
	{
		for(long x=512; x<2048; x+=512)
		{
			for(long t=0; t<ITERATIONS; t++)
			{
				for(long z=0; z<ITERATIONS; z++)
				{
					for(long m=0; m<ITERATIONS; m++)
					{
						for(long c=0; c<ITERATIONS; c++)
						{
							d.x = x;
							d.y = y;
							d.c = c+1;
							d.m = m+1;
							d.z = z+1;
							d.t = t+1;
							d.mType = DETACHED_CHANNEL;

							WIN_TRACE("X=%d\n",d.x);
							WIN_TRACE("Y=%d\n",d.y);
							WIN_TRACE("M=%d\n",d.m);
							WIN_TRACE("C=%d\n",d.c);
							WIN_TRACE("Z=%d\n",d.z);
							WIN_TRACE("T=%d\n",d.t);

							d.dType = INT_16BIT;

							WIN_ASSERT_TRUE(ImageManager::getInstance()->CreateImage(id, d)==TRUE);
							WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(id)==TRUE);

							d.dType = INT_8BIT;

							WIN_ASSERT_TRUE(ImageManager::getInstance()->CreateImage(id, d)==TRUE);
							WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(id)==TRUE);

							d.dType = FLOAT_32BIT;

							WIN_ASSERT_TRUE(ImageManager::getInstance()->CreateImage(id, d)==TRUE);
							WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(id)==TRUE);

							d.dType = FLOAT_64BIT;

							WIN_ASSERT_TRUE(ImageManager::getInstance()->CreateImage(id, d)==TRUE);
							WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(id)==TRUE);

						}
					}
				}
			}
		}
	}

	//PASSING LIMIT TESTS
	const long TESTCASES = 14;
	for(long i=0; i<TESTCASES; i++)
	{
		//defaults
		d.x = 8;
		d.y = 8;
		d.c = 1;
		d.m = 1;
		d.z = 1;
		d.t = 1;
		d.mType = DETACHED_CHANNEL;

		//failure cases
		switch(i)
		{
		case 0:d.x = XMIN;break;
		case 1:d.x = XMAX;break;
		case 2:d.y = YMIN;break;
		case 3:d.y = YMAX;break;
		case 4:d.c = CMIN;break;
		case 5:d.c = CMAX;break;
		case 6:d.m = MMIN;break;
		case 7:d.m = MMAX;break;
		case 8:d.z = ZMIN;break;
		case 9:d.z = ZMAX;break;
		case 10:d.t = TMIN;break;
		case 11:d.t = TMAX;break;
		case 12:d.mType = static_cast<MemoryType>(MEMORYTYPE_MIN);break;
		case 13:d.mType = static_cast<MemoryType>(MEMORYTYPE_MAX-1);break;
		}

							WIN_TRACE("X=%d\n",d.x);
							WIN_TRACE("Y=%d\n",d.y);
							WIN_TRACE("M=%d\n",d.m);
							WIN_TRACE("C=%d\n",d.c);
							WIN_TRACE("Z=%d\n",d.z);
							WIN_TRACE("T=%d\n",d.t);

		d.dType = INT_16BIT;

		WIN_ASSERT_TRUE(ImageManager::getInstance()->CreateImage(id, d)==TRUE);
		WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(id)==TRUE);

		d.dType = INT_8BIT;

		WIN_ASSERT_TRUE(ImageManager::getInstance()->CreateImage(id, d)==TRUE);
		WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(id)==TRUE);

		d.dType = FLOAT_32BIT;

		WIN_ASSERT_TRUE(ImageManager::getInstance()->CreateImage(id, d)==TRUE);
		WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(id)==TRUE);

		d.dType = FLOAT_64BIT;

		WIN_ASSERT_TRUE(ImageManager::getInstance()->CreateImage(id, d)==TRUE);
		WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(id)==TRUE);

	}

	//FAILING LIMIT TESTS
	for(long i=0; i<TESTCASES; i++)
	{
		//defaults
		d.x = 8;
		d.y = 8;
		d.c = 1;
		d.m = 1;
		d.z = 1;
		d.t = 1;
		d.mType = DETACHED_CHANNEL;

		//failure cases
		switch(i)
		{
		case 0:d.x = XMIN-1;break;
		case 1:d.x = XMAX+1;break;
		case 2:d.y = YMIN-1;break;
		case 3:d.y = YMAX+1;break;
		case 4:d.c = CMIN-1;break;
		case 5:d.c = CMAX+1;break;
		case 6:d.m = MMIN-1;break;
		case 7:d.m = MMAX+1;break;
		case 8:d.z = ZMIN-1;break;
//		case 9:d.z = ZMAX+1;break;
		case 10:d.t = TMIN-1;break;
//		case 11:d.t = TMAX+1;break;
		case 12:d.mType = static_cast<MemoryType>(MEMORYTYPE_MIN-1);break;
		case 13:d.mType = static_cast<MemoryType>(MEMORYTYPE_MAX);break;
		}

		d.dType = INT_16BIT;

		WIN_ASSERT_TRUE(ImageManager::getInstance()->CreateImage(id, d)==FALSE);
		WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(id)==FALSE);

		d.dType = INT_8BIT;

		WIN_ASSERT_TRUE(ImageManager::getInstance()->CreateImage(id, d)==FALSE);
		WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(id)==FALSE);

		d.dType = FLOAT_32BIT;

		WIN_ASSERT_TRUE(ImageManager::getInstance()->CreateImage(id, d)==FALSE);
		WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(id)==FALSE);

		d.dType = FLOAT_64BIT;

		WIN_ASSERT_TRUE(ImageManager::getInstance()->CreateImage(id, d)==FALSE);
		WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(id)==FALSE);

	}

	//fail destroying images that do not exist
		WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(0)==FALSE);
		WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(1)==FALSE);
		WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(10)==FALSE);
		WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(100)==FALSE);

}
END_TESTF

template<class T>
class MyOp : public IOp
{
public:
	MyOp();
	~MyOp();

	virtual long Execute(IImage *image);

	T data;
};

template<class T>
 MyOp<T>::MyOp()
{
}


template<class T>
MyOp<T>::~MyOp()
{
}

template<class T>
long MyOp<T>::Execute(IImage *image)
{
	Dimensions d;

	image->GetImageDimensions(d);

	long x;
	long y;
	long c;
	long m;
	long z;
	long t;

	long chan=0;
	for(t=0;t<d.t;t++)
	{
		for(z=0;z<d.z;z++)
		{
			for(m=0;m<d.m;m++)
			{	
				for(c=0;c<d.c;c++)
				{
					T*p = (T*)image->GetDataPtr(c,m,z,t);

					for(y=0; y<d.y; y++)
					{
						for(x=0; x<d.x; x++)
						{
							if(chan%1)
							{
								*p = x;
							}
							else
							{
								*p = d.x - x - 1;
							}
							p++;
							chan++;
						}
					}
				}
			}
		}
	}
return TRUE;

}

//Operate on Images
BEGIN_TESTF(ImageOperate,ImageManagerTestFixture)
{
	long id;
	Dimensions d;
	d.x = 1024;
	d.y = 1024;
	d.c = 1;
	d.m = 1;
	d.z = 1;
	d.t = 1;
	d.mType = DETACHED_CHANNEL;
	d.dType = INT_16BIT;

	WIN_ASSERT_TRUE(ImageManager::getInstance()->CreateImage(id, d)==TRUE);

	MyOp<unsigned short> op;

	ImageManager::getInstance()->OpImage(id,&op);

	TiffLibDll myTiff(_T(".\\libtiff3.dll"));

	TIFF *out= myTiff.TIFFOpenW(_T("C:\\Temp\\new.tif"), "w"); 

	int sampleperpixel = 1;    // or 3 if there is no alpha channel, you should get a understanding of alpha in class soon. 

	unsigned int width = d.x;
	unsigned int height = d.y;

	myTiff.TIFFSetField (out, TIFFTAG_IMAGEWIDTH, width);  // set the width of the image
	myTiff.TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);    // set the height of the image
	myTiff.TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);   // set number of channels per pixel
	myTiff.TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 16);    // set the size of the channels
	myTiff.TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.

	//   Some other essential fields to set that you do not have to understand for now.
	myTiff.TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	myTiff.TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB); 

	tsize_t linebytes = sampleperpixel * width * 2;     // length in memory of one row of pixel in the image. 

	unsigned char *buf = NULL;        // buffer used to store the row of pixel information for writing to file

	Image* image=NULL;

	char * pMemoryBuffer= ImageManager::getInstance()->GetImagePtr(id,0,0,0,0);

	//    Allocating memory to store the pixels of current row
	if (myTiff.TIFFScanlineSize(out)==linebytes)
		buf =(unsigned char *)myTiff._TIFFmalloc(linebytes);
	else
		buf = (unsigned char *)myTiff._TIFFmalloc(myTiff.TIFFScanlineSize(out));

	// We set the strip size of the file to be size of one row of pixels
	myTiff.TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, myTiff.TIFFDefaultStripSize(out, width*sampleperpixel));

	//Now writing image to the file one strip at a time
	for (uint32 row = 0; row < height; row++)
	{
		memcpy(buf, &pMemoryBuffer[(height-row-1)*linebytes], linebytes);    // check the index here, and figure out why not using h*linebytes
		if (myTiff.TIFFWriteScanline(out, buf, row, 0) < 0)
			break;
	}

	//Finally we close the output file, and destroy the buffer 
	myTiff.TIFFClose(out); 

	if (buf)
		myTiff._TIFFfree(buf);

	WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(id)==TRUE);
}
END_TESTF

#define NUM_THREADS 10

namespace
{
	HANDLE hEvent[NUM_THREADS];
	long val;
}

struct Info
{
	long id;
	HANDLE hEventThread;
};

UINT Thread1Proc( LPVOID pParam )
{
	long i;

	long id;
	Info info;

	info = *(Info*)(pParam);

	Dimensions d;
	d.x = 1024;
	d.y = 1024;
	d.c = 1;
	d.m = 1;
	d.z = 1;
	d.t = 1;
	d.mType = DETACHED_CHANNEL;
	d.dType = INT_16BIT;
	for(i=0; i<10; i++)
	{
		WIN_ASSERT_TRUE(ImageManager::getInstance()->CreateImage(id, d)==TRUE);
		WIN_TRACE("Thread %d create %d\n",::GetCurrentThreadId(),id);

		MyOp<unsigned short> op;

		ImageManager::getInstance()->OpImage(info.id,&op);

		Image* image=NULL;

		char * pMemoryBuffer= ImageManager::getInstance()->GetImagePtr(info.id,0,0,0,0);

		memset(pMemoryBuffer,0,1024);

		WIN_ASSERT_TRUE(ImageManager::getInstance()->DestroyImage(id)==TRUE);
		WIN_TRACE("Thread %d destroy %d\n",::GetCurrentThreadId(),id);
	}

	SetEvent( info.hEventThread );
	return 0;
}

//Access same image from multiple threads
BEGIN_TESTF(ImageThreading,ImageManagerTestFixture)
{
	long id;
	Dimensions d;
	d.x = 1024;
	d.y = 1024;
	d.c = 1;
	d.m = 1;
	d.z = 1;
	d.t = 1;
	d.mType = DETACHED_CHANNEL;
	d.dType = INT_16BIT;

	WIN_ASSERT_TRUE(ImageManager::getInstance()->CreateImage(id, d)==TRUE);


	long i;

	for(i=0; i<NUM_THREADS; i++)
	{
	hEvent[i] = CreateEvent(0, FALSE, FALSE, 0);
	}

	DWORD dwThreadId[NUM_THREADS];

	Info info[NUM_THREADS];
	
	for(i=0; i<NUM_THREADS; i++)
	{
	info[i].id = id;
	info[i].hEventThread = hEvent[i];
	}

	HANDLE hThread[NUM_THREADS];
	
	for(i=0; i<NUM_THREADS; i++)
	{
	hThread[i] = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) Thread1Proc, (LPVOID)&info[i], 0, &dwThreadId[i] );
	}

	DWORD dwWait = WaitForMultipleObjects( NUM_THREADS, hEvent, TRUE, INFINITE );

	WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

	for(i=0; i<NUM_THREADS; i++)
	{
	CloseHandle(hThread[i]);
	}

}
END_TESTF