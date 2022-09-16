#pragma once

class RawToTIFFConverter
{
private:
	RawToTIFFConverter();
	static bool _instanceFlag;
	static RawToTIFFConverter* _single;

public:
	~RawToTIFFConverter();
	static RawToTIFFConverter* getInstance();

	static int ConvertRawToTIFF(const wchar_t* rawFileName, const wchar_t* tiffFolderName, int cCount, int tCount, int zCount, double intervalSec, const wchar_t* channelNameArray, long width, long height, double umPerPixel, double zStepSizeUM);
};
