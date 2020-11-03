// ThorDiskIO.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ThorDiskIO.h"
#include <fstream>
#include <iostream>
#include "..\..\Common\BinaryImageDataUtilities\RawFile.h"
#include "..\..\Common\BinaryImageDataUtilities\ChannelManipulator.h"

DWORD dwThorDiskIOThreadId = NULL;
HANDLE hThorDiskIOThread = NULL;
DWORD dwThreadId = NULL;
HANDLE hThread = NULL;
HANDLE hStatusEvent;
BOOL stopRun = FALSE;
BOOL activeRun = FALSE;
const int MAX_CHANNELS = 16;
static char * pChan[MAX_CHANNELS];
//long maxCameraWidth = 1344;
//long maxCameraHeight = 1024;
//auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
wchar_t message[256];
auto_ptr<TiffLibDll> tiffDll(new TiffLibDll(L".\\libtiff3.dll"));


DllExport_ThorDiskIO ReadImageInfo(wchar_t * selectedFileName, long &width, long &height, long &colorChannels)
{
	TIFF* image;
	wchar_t * path = selectedFileName;

	// Open the TIFF image
	if((image = tiffDll->TIFFOpenW(path, "r")) == NULL)
	{
		fprintf(stderr, "Could not open incoming image\n");
		return FALSE;
	}

	int w,h,c;

	w = 0;
	h = 0;
	c = 0;

	// getting the image parameters
	tiffDll->TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &w);
	tiffDll->TIFFGetField(image, TIFFTAG_IMAGELENGTH, &h);
	tiffDll->TIFFGetField(image, TIFFTAG_SAMPLESPERPIXEL, &c);  

	width = w;
	height = h;
	colorChannels = c;

	int orientation=0;
	tiffDll->TIFFGetField(image, TIFFTAG_ORIENTATION,&orientation);

	// Close the TIFF image
	tiffDll->TIFFClose(image);

	return TRUE;
}

DllExport_ThorDiskIO ReadImageInfo2(wchar_t * selectedFileName, long &width, long &height, long &colorChannels, long& tiles, long& tileWidth, long& tileHeight)
{
	TIFF* image;
	wchar_t * path = selectedFileName;

	// Open the TIFF image
	if((image = tiffDll->TIFFOpenW(path, "r")) == NULL)
	{
		fprintf(stderr, "Could not open incoming image\n");
		return FALSE;
	}

	int w=0, h=0, c=0;
	int tw=0, th=0;

	// getting the image parameters
	tiffDll->TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &w);
	tiffDll->TIFFGetField(image, TIFFTAG_IMAGELENGTH, &h);
	tiffDll->TIFFGetField(image, TIFFTAG_SAMPLESPERPIXEL, &c);  
	tiles = tiffDll->TIFFNumberOfTiles(image);
	tiffDll->TIFFGetField(image, TIFFTAG_TILEWIDTH, &tw);
	tiffDll->TIFFGetField(image, TIFFTAG_TILELENGTH, &th);

	width = w;
	height = h;
	colorChannels = c;
	tileWidth = tw;
	tileHeight = th;

	int orientation=0;
	tiffDll->TIFFGetField(image, TIFFTAG_ORIENTATION,&orientation);

	// Close the TIFF image
	tiffDll->TIFFClose(image);

	return TRUE;
}


DllExport_ThorDiskIO ReadImage(char *selectedFileName, char* &outputBuffer)
{
	TIFF* image;
	tsize_t stripSize;
	unsigned long imageOffset, result;
	int stripMax, stripCount;
	unsigned long bufferSize;
	wchar_t * path = (wchar_t*)selectedFileName;
	bool status;

	// Open the TIFF image
	if((image = tiffDll->TIFFOpenW(path, "r")) == NULL){
		//		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"Could not open incoming image");

	}

	// Read in the possibly multiple strips
	stripSize = tiffDll->TIFFStripSize(image);
	stripMax = tiffDll->TIFFNumberOfStrips (image);
	imageOffset = 0;

	bufferSize = tiffDll->TIFFNumberOfStrips (image) * stripSize;

	for (stripCount = 0; stripCount < stripMax; stripCount++)
	{
		if((result = tiffDll->TIFFReadEncodedStrip (image, stripCount, outputBuffer + imageOffset, stripSize)) == -1)
		{
			//logDll->TLTraceEvent(VERBOSE_EVENT,1,L"Read error on input strip number");
		}
		imageOffset += result;
	}

	// Close the TIFF image
	tiffDll->TIFFClose(image);

	if(outputBuffer > 0)
	{
		//logDll->TLTraceEvent(VERBOSE_EVENT,1,L"inside output buffer: TRUE");
		status = TRUE;
	}
	else
	{
		//logDll->TLTraceEvent(VERBOSE_EVENT,1,L"inside output buffer: FALSE");
		status = FALSE;
	}	
	return status;  
}

DllExport_ThorDiskIO ReadColorImage(char *redFileName, char *greenFileName, char *blueFileName, char* &outputBuffer, long cameraWidth, long cameraHeight)
{
	TIFF* image;
	tsize_t stripSize;
	unsigned long imageOffset, result;
	int stripMax, stripCount;
	unsigned long bufferSize;
	bool status;

	wchar_t * rPath = (wchar_t*)redFileName;
	wchar_t * gPath = (wchar_t*)greenFileName;
	wchar_t * bPath = (wchar_t*)blueFileName;

	long width = cameraWidth;
	long height = cameraHeight;

	long i;

	pChan[0] = outputBuffer;
	for(i=1; i<MAX_CHANNELS; i++)
	{
		pChan[i] = pChan[i-1] + width * height * 2;
	}

	//create a new color buffer where the data will be captured and merged
	char * colorBuffer = new char[width * height * 2 * 3];

	memset(colorBuffer,0,width * height * 2 * 3);

	char * rBuf = colorBuffer;
	char * gBuf = rBuf + width * height * 2;
	char * bBuf = gBuf + width * height * 2;

	//build red channel image buffer
	if(rPath != NULL)
	{	
		// Open the TIFF image
		if((image = tiffDll->TIFFOpenW(rPath, "r")) == NULL){
			//logDll->TLTraceEvent(VERBOSE_EVENT,1,L"Could not open incoming image");	
		}

		// Read in the possibly multiple strips
		stripSize = tiffDll->TIFFStripSize(image);
		stripMax = tiffDll->TIFFNumberOfStrips (image);
		imageOffset = 0;

		bufferSize = tiffDll->TIFFNumberOfStrips (image) * stripSize;

		for (stripCount = 0; stripCount < stripMax; stripCount++)
		{
			if((result = tiffDll->TIFFReadEncodedStrip (image, stripCount, rBuf + imageOffset, stripSize)) == -1)
			{
				//logDll->TLTraceEvent(VERBOSE_EVENT,1,L"Read error on input strip number");
			}
			imageOffset += result;
		}

		// Close the TIFF image
		tiffDll->TIFFClose(image);
	}

	//build green channel image buffer
	if(gPath != NULL)
	{	
		// Open the TIFF image
		if((image = tiffDll->TIFFOpenW(gPath, "r")) == NULL){
			//logDll->TLTraceEvent(VERBOSE_EVENT,1,L"Could not open incoming image");	
		}

		// Read in the possibly multiple strips
		stripSize = tiffDll->TIFFStripSize(image);
		stripMax = tiffDll->TIFFNumberOfStrips (image);
		imageOffset = 0;

		bufferSize = tiffDll->TIFFNumberOfStrips (image) * stripSize;

		for (stripCount = 0; stripCount < stripMax; stripCount++)
		{
			if((result = tiffDll->TIFFReadEncodedStrip (image, stripCount, gBuf + imageOffset, stripSize)) == -1)
			{
				//logDll->TLTraceEvent(VERBOSE_EVENT,1,L"Read error on input strip number");
			}
			imageOffset += result;
		}

		// Close the TIFF image
		tiffDll->TIFFClose(image);
	}

	//build blue channel image buffer
	if(bPath != NULL)
	{	
		// Open the TIFF image
		if((image = tiffDll->TIFFOpenW(bPath, "r")) == NULL){
			//logDll->TLTraceEvent(VERBOSE_EVENT,1,L"Could not open incoming image");	
		}

		// Read in the possibly multiple strips
		stripSize = tiffDll->TIFFStripSize(image);
		stripMax = tiffDll->TIFFNumberOfStrips (image);
		imageOffset = 0;

		bufferSize = tiffDll->TIFFNumberOfStrips (image) * stripSize;

		for (stripCount = 0; stripCount < stripMax; stripCount++)
		{
			if((result = tiffDll->TIFFReadEncodedStrip (image, stripCount, bBuf + imageOffset, stripSize)) == -1)
			{
				//logDll->TLTraceEvent(VERBOSE_EVENT,1,L"Read error on input strip number");
			}
			imageOffset += result;
		}

		// Close the TIFF image
		tiffDll->TIFFClose(image);
	}

	memcpy(pChan[0],rBuf,width * height * 2);
	memcpy(pChan[1],gBuf,width * height * 2);
	memcpy(pChan[2],bBuf,width * height * 2);

	delete[] colorBuffer;

	if(outputBuffer > 0)
	{
		//logDll->TLTraceEvent(VERBOSE_EVENT,1,L"inside output buffer: TRUE");
		status = TRUE;
	}
	else
	{
		//logDll->TLTraceEvent(VERBOSE_EVENT,1,L"inside output buffer: FALSE");
		status = FALSE;
	}	
	return status;  
}

/// the data read is in the following format
/// [ChanA: Tile1, Tile 2, Tile3, Tile 4][ChanB: Tile 1, Tile 2, Tile3, Tile 4]
DllExport_ThorDiskIO ReadChannelTiledImages(char **fileNames, long numChannels, char* &outputBuffer)
{
	int i;

	for(i=0; i<numChannels; i++)
	{
		if(fileNames[i] == NULL)
		{
			continue;
		}

		wchar_t *path = (wchar_t*)fileNames[i];

		// Open the TIFF image
		TIFF* image = tiffDll->TIFFOpenW(path, "r");

		if(image) {
			int imageWidth = 0, imageHeight = 0;
			int tileWidth = 0, tileHeight = 0;
			long x, y;
			int tileIndex = 0;

			long tileBytes = tiffDll->TIFFTileSize(image);
			long numOfTiles = tiffDll->TIFFNumberOfTiles(image);

			tiffDll->TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &imageWidth);
			tiffDll->TIFFGetField(image, TIFFTAG_IMAGELENGTH, &imageHeight);
			tiffDll->TIFFGetField(image, TIFFTAG_TILEWIDTH, &tileWidth);
			tiffDll->TIFFGetField(image, TIFFTAG_TILELENGTH, &tileHeight);

			char * buf = outputBuffer + i * imageWidth * imageHeight * 2;
			memset(buf, 0, imageWidth * imageHeight * 2);

			for (y = 0; y < imageHeight; y += tileHeight) {
				for (x = 0; x < imageWidth; x += tileWidth) {
					tiffDll->TIFFReadTile(image, buf + tileIndex * tileBytes, x, y, 0, 0);
					tileIndex ++;
				}
			}
		}

		// Close the TIFF image
		tiffDll->TIFFClose(image);
	}
	return TRUE;
}

DllExport_ThorDiskIO ReadChannelImages(char **fileNames, long numChannels, char* &outputBuffer, long cameraWidth, long cameraHeight)
{
	TIFF* image;
	tsize_t stripSize;
	unsigned long imageOffset, result = 0;
	int stripMax, stripCount;
	unsigned long bufferSize;
	bool status;

	long width = cameraWidth;
	long height = cameraHeight;

	long i;

	pChan[0] = outputBuffer;
	for(i=1; i<numChannels; i++)
	{
		pChan[i] = pChan[i-1] + width * height * 2;
		memset(pChan[i],0,width * height * 2);
	}


	for(i=0; i<numChannels; i++)
	{
		if(fileNames[i] == NULL)
		{
			//buf += width*height*2;
			continue;
		}

		char * buf = pChan[i];

		wchar_t *path = (wchar_t*)fileNames[i];
		// Open the TIFF image
		if((image = tiffDll->TIFFOpenW(path, "r")) == NULL)
		{
		}

		// Read in the possibly multiple strips
		stripSize = tiffDll->TIFFStripSize(image);
		stripMax = tiffDll->TIFFNumberOfStrips (image);
		imageOffset = 0;

		bufferSize = tiffDll->TIFFNumberOfStrips (image) * stripSize;

		for (stripCount = 0; stripCount < stripMax; stripCount++)
		{
			result = tiffDll->TIFFReadEncodedStrip (image, stripCount, buf + imageOffset, stripSize);
			if(result == -1)
			{
				//logDll->TLTraceEvent(VERBOSE_EVENT,1,L"Read error on input strip number");
			}
			imageOffset += result;
		}

		// Close the TIFF image
		tiffDll->TIFFClose(image);
	}

	if(outputBuffer > 0)
	{
		//logDll->TLTraceEvent(VERBOSE_EVENT,1,L"inside output buffer: TRUE");
		status = TRUE;
	}
	else
	{
		//logDll->TLTraceEvent(VERBOSE_EVENT,1,L"inside output buffer: FALSE");
		status = FALSE;
	}	
	return status;  
}

DllExport_ThorDiskIO ReadChannelRawImages(char **fileNames, long numChannels, char* &outputBuffer, long cameraWidth, long cameraHeight)
{
	TIFF* image;
	tsize_t stripSize;
	unsigned long imageOffset, result = 0;
	int stripMax, stripCount;
	unsigned long bufferSize;
	bool status;

	long width = cameraWidth;
	long height = cameraHeight;

	long i;

	pChan[0] = outputBuffer;
	for(i=1; i<numChannels; i++)
	{
		pChan[i] = pChan[i-1] + width * height * 2;
		memset(pChan[i],0,width * height * 2);
	}


	for(i=0; i<numChannels; i++)
	{
		if(fileNames[i] == NULL)
		{
			//buf += width*height*2;
			continue;
		}

		char * buf = pChan[i];

		wchar_t *path = (wchar_t*)fileNames[i];
		// Open the TIFF image
		if((image = tiffDll->TIFFOpenW(path, "r")) == NULL)
		{
		}

		// Read in the possibly multiple strips
		stripSize = tiffDll->TIFFStripSize(image);
		stripMax = tiffDll->TIFFNumberOfStrips (image);
		imageOffset = 0;

		bufferSize = tiffDll->TIFFNumberOfStrips (image) * stripSize;

		for (stripCount = 0; stripCount < stripMax; stripCount++)
		{
			result = tiffDll->TIFFReadRawStrip (image, stripCount, buf + imageOffset, stripSize);
			if(result == -1)
			{
				//logDll->TLTraceEvent(VERBOSE_EVENT,1,L"Read error on input strip number");
			}
			imageOffset += result;
		}

		// Close the TIFF image
		tiffDll->TIFFClose(image);
	}

	if(outputBuffer > 0)
	{
		//logDll->TLTraceEvent(VERBOSE_EVENT,1,L"inside output buffer: TRUE");
		status = TRUE;
	}
	else
	{
		//logDll->TLTraceEvent(VERBOSE_EVENT,1,L"inside output buffer: FALSE");
		status = FALSE;
	}	
	return status;  
}

DllExport_ThorDiskIO ReadChannelImageRawSliceToChannel(char* outputBuffer, int channelInOutputBuffer, char* fileName, int width, int height, int zDepth, int channels, int loadChannel, int zSlice, int time, int enabledChannelsBitmask, bool containsDisabledChannels)
{

	//=== Convert bitmask to vector ===
	std::vector<int> enabledChannels = ChannelManipulator<unsigned short>::getEnabledChannels(enabledChannelsBitmask);


	//=== Convert Filename to Wide String ===
	std::string fileNameString(fileName);
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &fileNameString[0], (int)fileNameString.size(), NULL, 0);
	std::wstring fileNameWideString(size_needed, 0 );
	MultiByteToWideChar(CP_UTF8, 0, &fileNameString[0], (int)fileNameString.size(), &fileNameWideString[0], size_needed);


	//=== Open Files and Buffers as Image Objects ===
	RawFile<unsigned short> rawFileStream(fileNameWideString,width,height,zDepth,channels,1,containsDisabledChannels, enabledChannels, GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
	GenericImage<unsigned short>& sourceImage = rawFileStream.getImageAtIndex(time);
	GenericImage<unsigned short> destinationImage(width,height,1,channelInOutputBuffer+1,1,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
	destinationImage.setMemoryBuffer((unsigned short*)outputBuffer);


	//=== Copy Into Buffer ===
	destinationImage.copyChannelFrom(sourceImage,loadChannel,zSlice,channelInOutputBuffer,0);
	return TRUE;


}



DllExport_ThorDiskIO ReadChannelImageRawSlice(char* outputBuffer, char* fileName, int width, int height, int zDepth, int channels, int loadChannel, int zSlice, int time, int enabledChannelsBitmask, bool containsDisabledChannels)
{

	//=== Convert bitmask to vector ===
	std::vector<int> enabledChannels = ChannelManipulator<unsigned short>::getEnabledChannels(enabledChannelsBitmask);


	//=== Convert Filename to Wide String ===
	std::string fileNameString(fileName);
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &fileNameString[0], (int)fileNameString.size(), NULL, 0);
	std::wstring fileNameWideString(size_needed, 0 );
	MultiByteToWideChar(CP_UTF8, 0, &fileNameString[0], (int)fileNameString.size(), &fileNameWideString[0], size_needed);


	//=== Open Files and Buffers as Image Objects ===
	RawFile<unsigned short> rawFileStream(fileNameWideString,width,height,zDepth,channels,1,containsDisabledChannels, enabledChannels, GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
	GenericImage<unsigned short>& sourceImage = rawFileStream.getImageAtIndex(time);
	GenericImage<unsigned short> destinationImage(width,height,1,channels,1,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
	destinationImage.setMemoryBuffer((unsigned short*)outputBuffer);


	//=== Copy Into Buffer ===
	destinationImage.copyChannelFrom(sourceImage,loadChannel,zSlice,loadChannel,0);
	return TRUE;


}

DllExport_ThorDiskIO ReadImagesRaw(wchar_t* fileName, int64_t size, char* &outputBuffer, int64_t offset)
{
	std::ifstream inFile (fileName, ios::binary | ios::in);
	
	// Open the raw image file
	if(inFile.is_open())
	{
		inFile.seekg(offset, ios::beg);
		inFile.read(outputBuffer, size);
		inFile.close();

		return TRUE;
	}

	return FALSE;
}


DllExport_ThorDiskIO ReadChannelImagesRaw(char* &outputBuffer, long BufChs, char* fileName, long FileChs, long ChToRead, long frmSize, long blkIndex)
{
	ULONG64 offset;
	if(BufChs == 4)
	{			
		for(int i = 0; i < 4; i++)
		{
			pChan[i] = outputBuffer + i * frmSize;
		}
	}
	else if(BufChs == 1)
	{			
		pChan[ChToRead] = outputBuffer;
	}
	else
	{
		return FALSE;
	}

	std::ifstream inFile (fileName, ios::in|ios::binary);
	// Open the raw image file
	if(inFile.is_open())
	{
		if(FileChs == 4)
		{
			if(ChToRead == 4)
			{
				offset = (((ULONG64)blkIndex) * 4 * ((ULONG64)frmSize));
				inFile.seekg(offset, ios::beg);
				inFile.read(pChan[0], frmSize * 4);
			}
			else
			{				
				offset = (((ULONG64)blkIndex) * 4 + ChToRead) * ((ULONG64)frmSize);
				inFile.seekg(offset, ios::beg);
				inFile.read(pChan[ChToRead], frmSize);
			}
		}
		else if(FileChs == 1)
		{
			offset = (((ULONG64)blkIndex) * ((ULONG64)frmSize));
			inFile.seekg(offset, ios::beg);
			inFile.read(pChan[ChToRead], frmSize);
		}
		else
		{
			int k = 0;
			for(int i = 0; i < 4; i++)
			{
				if((((byte)ChToRead) & (0x1<<i)) != 0)
				{
					offset = (((ULONG64)blkIndex) * FileChs + k) * ((ULONG64)frmSize);
					k++;
					inFile.seekg(offset, ios::beg);
					inFile.read(pChan[i], frmSize);
				}
			}
		}

		// Close the raw image file
		inFile.close();	
	}

	return TRUE;
}
