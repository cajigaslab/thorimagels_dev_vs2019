// Partial inspired by the work of Andreas Hartl

#pragma once 

#include <windows.h>
#include <stdio.h>
#include <typeinfo>

//convert string to wstring for BMP filename, can be used anywhere
static std::wstring ConvertStringToWString(std::string s)
{
	size_t origsize = strlen(s.c_str()) + 1;
	const size_t newsize = _MAX_PATH;
	size_t  convertedChars = 0;
	wchar_t wcstring[newsize];
	mbstowcs_s(&convertedChars, wcstring, origsize, s.c_str(), _TRUNCATE);

	std::wstring ws(wcstring);
	return ws;
}

//load bmp to memory, remember to delete[] memory after use
static BYTE* LoadBMP (BITMAPINFOHEADER* bmpinfo, LPCTSTR bmpfile)
{
	BITMAPFILEHEADER bmpheader;
	DWORD bytesread;

	HANDLE file = CreateFile(bmpfile , GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (NULL == file)
		return NULL;

	if (false == ReadFile(file, &bmpheader, sizeof(BITMAPFILEHEADER), &bytesread, NULL))
	{
		CloseHandle ( file );
		return NULL;
	}
	if (false == ReadFile(file, bmpinfo, sizeof(BITMAPINFOHEADER), &bytesread, NULL))
	{
		CloseHandle ( file );
		return NULL;
	}
	//check bitmap without compression, allow 8 bits
	if (( bmpheader.bfType != 'MB' ) || ( bmpinfo->biCompression != BI_RGB ))	//( bmpinfo.biBitCount != 24 )
	{
		CloseHandle ( file );
		return NULL;
	}

	long size = bmpheader.bfSize - bmpheader.bfOffBits;		//bfOffBits: bit offset for header to beginning of data
	BYTE* Buffer = new BYTE[size];

	SetFilePointer(file, bmpheader.bfOffBits, NULL, FILE_BEGIN);
	if (false == ReadFile(file, Buffer, size, &bytesread, NULL))
	{
		delete[] Buffer;
		CloseHandle(file);
		return NULL;
	}
	CloseHandle(file);
	return Buffer;
}

typedef struct tagRGBTriplet
{
	BYTE red;
	BYTE green;
	BYTE blue;
} RGBTriplet;

//convert from BGR (BMP format) to RGB, delete[] memory after use
static BYTE* ConvertBGRToRGBBuffer(BYTE* Buffer, BITMAPINFOHEADER bmpinfo, long* newsize)
{
	int channelCnt = static_cast<int>(bmpinfo.biBitCount/CHAR_BIT);
	if ((NULL == Buffer) || (bmpinfo.biWidth <= 0) || (bmpinfo.biHeight <= 0))
		return NULL;

	int padding = 0;
	int scanlinebytes = bmpinfo.biWidth*channelCnt;
	while ((scanlinebytes+padding)%4 != 0)
	{
		padding++;
	}
	int psw = scanlinebytes + padding;

	*newsize = bmpinfo.biHeight * psw;
	BYTE* newbuf = new BYTE[*newsize];

	long bufpos = 0;   
	long newpos = 0;
	for (int y = 0; y < bmpinfo.biHeight; y++)
	{
		for (int x = 0; x < channelCnt*bmpinfo.biWidth; x+=channelCnt)
		{
			newpos = y*channelCnt*bmpinfo.biWidth + x;     
			bufpos = (bmpinfo.biHeight-y-1)*psw + x;
			// swap R and B
			for (int c = 0; c < channelCnt; c++)
			{
				newbuf[newpos+c] = Buffer[bufpos+(channelCnt-1)-c];       
			}
		}
	}
	return newbuf;
}

//convert from RGB to BGR (BMP format), delete[] memory after use
static BYTE* ConvertRGBToBGRBuffer(BYTE* Buffer, BITMAPINFOHEADER bmpinfo, long* newsize)
{
	int channelCnt = static_cast<int>(bmpinfo.biBitCount/CHAR_BIT);
	if ((NULL == Buffer) || (bmpinfo.biWidth <= 0) || (bmpinfo.biHeight <= 0))
		return NULL;

	int padding = 0;
	int scanlinebytes = bmpinfo.biWidth*channelCnt;
	while ((scanlinebytes+padding)%4 != 0) 
	{
		padding++;
	}
	int psw = scanlinebytes + padding;

	*newsize = bmpinfo.biHeight * psw;
	BYTE* newbuf = new BYTE[*newsize];

	std::memset(newbuf, 0, *newsize);

	long bufpos = 0;   
	long newpos = 0;
	for (int y = 0; y<bmpinfo.biHeight; y++)
	{
		for (int x = 0; x<(channelCnt*bmpinfo.biWidth); x+=channelCnt)
		{
			bufpos = y*channelCnt*bmpinfo.biWidth + x;		// position in original buffer
			newpos = (bmpinfo.biHeight-y-1)*psw + x;		// position in padded buffer
			// swap R and B  
			for (int c = 0; c < channelCnt; c++)
			{
				newbuf[newpos+c] = Buffer[bufpos+(channelCnt-1)-c];  
			}
		}
	}
	return newbuf;
}

//save memory to bmp
template<typename T>
static BOOL SaveBMP(T* Buffer, int width, int height, long paddedsize, LPCTSTR bmpfile)
{
	BOOL ret = TRUE;
	const int BMP_BYTE_NUM = 4;
	const int PIXEL_LEVEL = 256;
	int channel = paddedsize/width/height;

	RGBQUAD palette[PIXEL_LEVEL];
	for(int i = 0; i < PIXEL_LEVEL; ++i)
	{
		palette[i].rgbBlue = (BYTE)i;
		palette[i].rgbGreen = (BYTE)i;
		palette[i].rgbRed = (BYTE)i;
	}

	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER info;
	std::memset(&bmfh, 0, sizeof (BITMAPFILEHEADER));
	std::memset(&info, 0, sizeof (BITMAPINFOHEADER));


	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biWidth = width;
	info.biHeight = height;
	info.biPlanes = 1;	
	info.biBitCount = CHAR_BIT*channel;
	info.biCompression = BI_RGB;	
	info.biSizeImage = paddedsize;
	info.biXPelsPerMeter = 0;  
	info.biYPelsPerMeter = 0;     
	info.biClrUsed = 0;	
	info.biClrImportant = 0; 

	bmfh.bfType = 'B'+('M' << 8);	// 0x4d42 = 'BM' ;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * PIXEL_LEVEL;
	bmfh.bfSize = bmfh.bfOffBits + 	info.biSizeImage;

	HANDLE file = CreateFile (bmpfile , GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (NULL == file)
	{
		CloseHandle(file);
		return FALSE;
	}

	unsigned long bwritten = 0;
	if (false == WriteFile(file, &bmfh, sizeof(BITMAPFILEHEADER), &bwritten, NULL))
	{	
		CloseHandle(file);
		return FALSE;
	}

	if (false == WriteFile(file, &info, sizeof(BITMAPINFOHEADER), &bwritten, NULL))
	{	
		CloseHandle(file);
		return FALSE;
	}
	if (false == WriteFile(file, &palette[0], sizeof(RGBQUAD) * PIXEL_LEVEL, &bwritten, NULL))
	{	
		CloseHandle(file);
		return FALSE;
	}

	BYTE* localBuf;
	if(typeid(float) == typeid(T))
	{
		const int MAX_FLOAT_VAL = 65535;
		const int MAX_BYTE_VAL = 255;

		//linear scale:
		localBuf = new BYTE[paddedsize];
		T* pSrc = Buffer;
		for (int i = 0; i < paddedsize; i++)
		{
			BYTE val = (MAX_BYTE_VAL < (*pSrc)) ? static_cast<BYTE>(MAX_BYTE_VAL*static_cast<double>((*pSrc)/MAX_FLOAT_VAL)) : (*pSrc);
			localBuf[i] = val;
			pSrc++;
		}

		//write the RGB Data:
		if(width % BMP_BYTE_NUM == 0)
		{
			WriteFile(file, localBuf, info.biSizeImage, &bwritten, NULL );
		}
		else
		{
			char* empty = new char[ BMP_BYTE_NUM - width % BMP_BYTE_NUM];
			for(int i = 0; i < height; ++i)
			{
				WriteFile(file, &localBuf[i * width], width, &bwritten, NULL );
				WriteFile(file, empty,  BMP_BYTE_NUM - width % BMP_BYTE_NUM, &bwritten, NULL );
			}
		}

		delete[] localBuf;
	}
	else if(typeid(BYTE) == typeid(T))
	{
		//write the RGB Data
		if(width%BMP_BYTE_NUM == 0)
		{
			WriteFile(file, Buffer, info.biSizeImage, &bwritten, NULL );
		}
		else
		{
			char* empty = new char[ BMP_BYTE_NUM - width % BMP_BYTE_NUM];
			for(int i = 0; i < height; ++i)
			{
				WriteFile(file, &Buffer[i * width], width, &bwritten, NULL );
				WriteFile(file, empty,  BMP_BYTE_NUM - width % BMP_BYTE_NUM, &bwritten, NULL );
			}
		}
	}

	CloseHandle(file);
	return ret;
}