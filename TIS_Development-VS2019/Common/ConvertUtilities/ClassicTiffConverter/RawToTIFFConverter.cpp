#include <memory>
#include <sstream>
#include "..\..\..\Tools\tiff-3.8.2\libtiff\tiffio.h"
#include "..\..\..\Tools\tiff-3.8.2\include\TiffLib.h"
#include "..\..\ResourceManager\ResourceManager\ResourceManager.h"
#include "common.h"
#include "RawToTIFFConverter.h"
#include "ClassicTiffConverter.h"

bool RawToTIFFConverter::_instanceFlag = false;
RawToTIFFConverter* RawToTIFFConverter::_single;

using namespace std;

TiffLibDll* tiffDll;

RawToTIFFConverter::RawToTIFFConverter()
{
	tiffDll = new TiffLibDll(L"libtiff3.dll");
}

RawToTIFFConverter::~RawToTIFFConverter()
{
	delete tiffDll;
	_instanceFlag = false;
}

RawToTIFFConverter* RawToTIFFConverter::getInstance()
{
	if (!_instanceFlag)
	{
		_single = new RawToTIFFConverter();
		_instanceFlag = true;
	}
	return _single;
}

int Call_TiffVSetField(TIFF* out, uint32 ttag_t, ...)
{
	int retv;
	va_list marker;

	va_start(marker, ttag_t);

	retv = tiffDll->TIFFVSetField(out, ttag_t, marker); //actual setting of the colormap into the TIFF images

	va_end(marker);

	return retv;
}

string uUIDSetup(vector<string>* wavelengthNames, long timePoints, long zstageSteps)
{
	string strOME;

	for (long t = 0; t < timePoints; t++)
	{
		for (long z = 0; z < zstageSteps; z++)
		{
			for (long c = 0; c < wavelengthNames->size(); c++)
			{
				string wavelengthName = wavelengthNames->at(c);

				GUID* pguid = 0x00;

				pguid = new GUID;

				CoCreateGuid(pguid);

				OLECHAR* bstrGuid;
				StringFromCLSID(*pguid, &bstrGuid);

				wstring ws(bstrGuid);

				//remove the curly braces at the end and start of the guid
				ws.erase(ws.size() - 1, 1);
				ws.erase(0, 1);

				string strGuid = ConvertWStringToString(ws);

				ostringstream ss;
				ss << "<TiffData" << " FirstT=\"" << t << "\"" << " FirstZ=\"" << z << "\"" << " FirstC=\"" << c << "\">";

				ostringstream strFilePathAndName;
				strFilePathAndName << wavelengthName << "_001_001_" << format("%03d", (z+1)) << "_" << format("%03d", (t+1)) << ".tif";

				ss << "<UUID FileName=\"" << strFilePathAndName.str().c_str() << "\">" << "urn:uuid:" << strGuid.c_str() << "</UUID>" << "</TiffData>";
				strOME += ss.str();
				// ensure memory is freed
				::CoTaskMemFree(bstrGuid);

				delete pguid;
			}
		}
	}

	return strOME;
}

string CreateOMEMetadata(int width, int height, int nc, int nt, int nz, double timeIncrement, int c, int t, int z, 
	string* acquiredDateTime, double deltaT, string* omeTiffData, PhysicalSize physicalSize)
{
	string tagData = "<?xml version=\"1.0\"?><OME xmlns=\"http://www.openmicroscopy.org/Schemas/OME/2010-06\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.openmicroscopy.org/Schemas/OME/2010-06 http://www.openmicroscopy.org/Schemas/OME/2010-06/ome.xsd\">";

	tagData += "<Image ID=\"Image:0\" AcquiredDate=";

	std::ostringstream ssDT;
	if (acquiredDateTime)
	{
		ssDT << "\"" << *acquiredDateTime << "\"";
	}
	else
	{
		ssDT << "\"\"";
	}
	tagData += ssDT.str();

	tagData += "><Pixels";
	tagData += " DimensionOrder=";
	tagData += "\"XYCZT\"";
	tagData += " ID=\"Pixels:0\"";

	tagData += " PhysicalSizeX=";	// physical size (resolution) in um
	std::ostringstream ssPhysicalSizeX;
	ssPhysicalSizeX << "\"" << physicalSize.x << "\"";
	tagData += ssPhysicalSizeX.str();
	tagData += " PhysicalSizeY=";
	std::ostringstream ssPhysicalSizeY;
	ssPhysicalSizeY << "\"" << physicalSize.y << "\"";
	tagData += ssPhysicalSizeY.str();
	tagData += " PhysicalSizeZ=";
	std::ostringstream ssPhysicalSizeZ;
	ssPhysicalSizeZ << "\"" << physicalSize.z << "\"";
	tagData += ssPhysicalSizeZ.str();

	tagData += " SizeC=";
	std::ostringstream ssC;
	ssC << "\"" << nc << "\"";
	tagData += ssC.str();
	tagData += " SizeT=";
	std::ostringstream ssT;
	ssT << "\"" << nt << "\"";
	tagData += ssT.str();
	tagData += " SizeX=";
	std::ostringstream ssX;
	ssX << "\"" << width << "\"";
	tagData += ssX.str();
	tagData += " SizeY=";
	std::ostringstream ssY;
	ssY << "\"" << height << "\"";
	tagData += ssY.str();
	tagData += " SizeZ=";
	std::ostringstream ssZ;
	ssZ << "\"" << nz << "\"";
	tagData += ssZ.str();
	tagData += " TimeIncrement=";
	std::ostringstream ssTi;
	ssTi << "\"" << timeIncrement << "\"";
	tagData += ssTi.str();
	tagData += " Type=";
	tagData += "\"uint16\"";
	tagData += ">";
	tagData += "<Channel ID=";
	tagData += "\"Channel:0:0\"";
	tagData += " SamplesPerPixel=\"1\"><LightPath/></Channel>";
	tagData += "<BinData BigEndian=\"false\" Length = \"0\" xmlns=\"http://www.openmicroscopy.org/Schemas/BinaryFile/2010-06\"/>";
	tagData += "<Plane DeltaT=";
	std::ostringstream ssDeltaT;
	ssDeltaT.setf(3);
	ssDeltaT << "\"" << deltaT << "\" ";
	tagData += ssDeltaT.str();

	tagData += "TheC=";
	std::ostringstream ssTheC;
	ssTheC << "\"" << c << "\" ";
	tagData += ssTheC.str();
	tagData += "TheZ=";
	std::ostringstream ssTheZ;
	ssTheZ << "\"" << z << "\" ";
	tagData += ssTheZ.str();
	tagData += "TheT=";
	std::ostringstream ssTheT;
	ssTheT << "\"" << t << "\"/>";
	tagData += ssTheT.str();

	if (NULL == omeTiffData)
	{
		std::ostringstream ssFirst;
		ssFirst << "<TiffData FirstC=\"" << c << "\"" << " FirstT=\"" << t << "\"" << " FirstZ=\"" << z << "\"" << " IFD=\"0\" PlaneCount=\"1\">";
		tagData += ssFirst.str();
		tagData += "</TiffData>";
	}
	else
	{
		tagData += *omeTiffData;
	}
	tagData += "</Pixels>";
	tagData += "</Image>";
	tagData += "<StructuredAnnotations xmlns=\"http://www.openmicroscopy.org/Schemas/SA/2010-06\"/>";
	tagData += "</OME>";

	return tagData;
}

string AddTimestamp()
{
	const int MAX_TIME_LENGTH = 30;
	char acquiredDateTime[MAX_TIME_LENGTH];

	SYSTEMTIME sysTime;

	GetSystemTime(&sysTime);

	StringCbPrintfA(acquiredDateTime, MAX_TIME_LENGTH, "%4d-%02d-%02dT%02d:%02d:%02d.%03d-%02d:00", sysTime.wYear, sysTime.wMonth, sysTime.wDay,
		sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds, 0);

	return string(acquiredDateTime);
}

int SaveTIFFWithoutOME(const wchar_t* filePathAndName, char* pMemoryBuffer, long width, long height, double umPerPixel, long doCompression)
{
	TIFF* out = tiffDll->TIFFOpenW(filePathAndName, "w");

	if (!out)
		return -1;

	//XML-READ
	int sampleperpixel = 1;    // or 3 if there is no alpha channel, you should get a understanding of alpha in class soon. 

	tiffDll->TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width);  // set the width of the image
	tiffDll->TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);    // set the height of the image
	tiffDll->TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);   // set number of channels per pixel
	//XML-READ
	tiffDll->TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 16);    // set the size of the channels
	tiffDll->TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.

	//  Some other essential fields to set that you do not have to understand for now.
	if (TRUE == doCompression)
	{
		tiffDll->TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	}
	else
	{
		tiffDll->TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	}

	tiffDll->TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

	tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

	//units are pixels per inch
	const int RESOLUTION_UNIT = 2;
	tiffDll->TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, RESOLUTION_UNIT);

	const double UM_PER_INCH_CONVERSION = 25399.99999997256800000002962656;

	float pixelsPerInch = static_cast<float>(UM_PER_INCH_CONVERSION / umPerPixel);
	Call_TiffVSetField(out, TIFFTAG_XRESOLUTION, pixelsPerInch);
	Call_TiffVSetField(out, TIFFTAG_YRESOLUTION, pixelsPerInch);

	// Color Palette is not working with the OME TIFF. /*TODO*/ determine the issue and reinsert at a later date
	//	tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
	//	Call_TiffVSetField(out, TIFFTAG_COLORMAP, rlut, glut, blut);

	tsize_t linebytes = sampleperpixel * width * 2;     // length in memory of one row of pixel in the image. 

	unsigned char* buf = NULL;        // buffer used to store the row of pixel information for writing to file

	// Allocating memory to store the pixels of current row
	if (tiffDll->TIFFScanlineSize(out) == linebytes)
		buf = (unsigned char*)tiffDll->_TIFFmalloc(linebytes);
	else
		buf = (unsigned char*)tiffDll->_TIFFmalloc(tiffDll->TIFFScanlineSize(out));

	// We set the strip size of the file to be size of one row of pixels
	tiffDll->TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, tiffDll->TIFFDefaultStripSize(out, width * sampleperpixel));


	//Now writing image to the file one strip at a time
	for (uint32 row = 0; row < static_cast<uint32>(height); row++)
	{
		memcpy(buf, &pMemoryBuffer[row * linebytes], linebytes);    // check the index here, and figure out why not using h*linebytes
		if (tiffDll->TIFFWriteScanline(out, buf, row, 0) < 0)
			break;
	}

	if (buf)
		tiffDll->_TIFFfree(buf);

	//Finally we close the output file, and destroy the buffer 
	tiffDll->TIFFClose(out);

	return 0;
}

long SaveTIFF(const wchar_t* filePathAndName, char* pMemoryBuffer, long width, long height, double umPerPixel,
	 int nc, int nt, int nz, double timeIncrement, int c, int t, int z, string* acquiredDateTime, double deltaT,
	string* omeTiffData, PhysicalSize physicalSize, long doCompression)
{
	TIFF* out = tiffDll->TIFFOpenW(filePathAndName, "w");

	if (!out)
		return -1;

	//XML-READ
	int sampleperpixel = 1;    // or 3 if there is no alpha channel, you should get a understanding of alpha in class soon. 

	tiffDll->TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width);  // set the width of the image
	tiffDll->TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);    // set the height of the image
	tiffDll->TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);   // set number of channels per pixel
	//XML-READ
	tiffDll->TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 16);    // set the size of the channels
	tiffDll->TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.

	//  Some other essential fields to set that you do not have to understand for now.
	if (TRUE == doCompression)
	{
		tiffDll->TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	}
	else
	{
		tiffDll->TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	}
	tiffDll->TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

	tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

	//units are pixels per inch
	const int RESOLUTION_UNIT = 2;
	tiffDll->TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, RESOLUTION_UNIT);

	const double UM_PER_INCH_CONVERSION = 25399.99999997256800000002962656;

	float pixelsPerInch = static_cast<float>(UM_PER_INCH_CONVERSION / umPerPixel);
	Call_TiffVSetField(out, TIFFTAG_XRESOLUTION, pixelsPerInch);
	Call_TiffVSetField(out, TIFFTAG_YRESOLUTION, pixelsPerInch);

	// Color Palette is not working with the OME TIFF. /*TODO*/ determine the issue and reinsert at a later date
	//	tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
	//	Call_TiffVSetField(out, TIFFTAG_COLORMAP, rlut, glut, blut);

	// Feng committed below 2 lines for later change
	string str = CreateOMEMetadata(width, height, nc, nt, nz, timeIncrement, c, t, z, acquiredDateTime, deltaT, omeTiffData, physicalSize);

	Call_TiffVSetField(out, TIFFTAG_IMAGEDESCRIPTION, str.c_str());

	tsize_t linebytes = sampleperpixel * width * 2;     // length in memory of one row of pixel in the image. 

	unsigned char* buf = NULL;        // buffer used to store the row of pixel information for writing to file

	// Allocating memory to store the pixels of current row
	if (tiffDll->TIFFScanlineSize(out) == linebytes)
		buf = (unsigned char*)tiffDll->_TIFFmalloc(linebytes);
	else
		buf = (unsigned char*)tiffDll->_TIFFmalloc(tiffDll->TIFFScanlineSize(out));

	// We set the strip size of the file to be size of one row of pixels
	tiffDll->TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, tiffDll->TIFFDefaultStripSize(out, width * sampleperpixel));


	//Now writing image to the file one strip at a time
	for (uint32 row = 0; row < static_cast<uint32>(height); row++)
	{
		memcpy(buf, &pMemoryBuffer[row * linebytes], linebytes);    // check the index here, and figure out why not using h*linebytes
		if (tiffDll->TIFFWriteScanline(out, buf, row, 0) < 0)
			break;
	}

	if (buf)
		tiffDll->_TIFFfree(buf);

	//Finally we close the output file, and destroy the buffer 
	tiffDll->TIFFClose(out);

	return 0;
}

int RawToTIFFConverter::ConvertRawToTIFF(const wchar_t* rawFileName, const wchar_t* tiffFolderName, int cCount, int tCount, int zCount, double intervalSec,
	const wchar_t* channelNameArray, long width, long height, double umPerPixel, double zStepSizeUM)
{
	int imageHandle = 0;

	// Load raw data file
	if (TC_LoadRawDataFile(rawFileName, &imageHandle) != 0)
		return FALSE;

	string resultFolder = WCharToString(tiffFolderName);

	// Get all channel names
	string channelString= WCharToString(channelNameArray);
	string delim = ",";
	vector<string>* channelArray = new vector<string>[cCount];
	Split(channelString, delim, channelArray);

	int imageSize = width * height * 2; //1 pixel=2 bytes
	char* data = new char[imageSize];

	// For OME meta data
	string acquiredDateTime = AddTimestamp();
	string strOME;
	PhysicalSize physicalSize;	// unit: um
	double res = floor(umPerPixel * 1000 + 0.5) / 1000;	// keep 2 figures after decimal point; res will be x.xx micron	
	physicalSize.x = res;
	physicalSize.y = res;
	physicalSize.z = zStepSizeUM;

	// Get OME nd Compression flags from application settings xml
	wchar_t* omeTIFFTag = CharToWchar("OMETIFFTag");
	wchar_t* compressionEnable = CharToWchar("TIFFCompressionEnable");
	wchar_t* value = CharToWchar("value");
	long doOME = ResourceManager::getInstance()->GetSettingsParamLong(1, omeTIFFTag, value, FALSE);
	long doCompression = ResourceManager::getInstance()->GetSettingsParamLong(1, compressionEnable, value, FALSE);

	// init tiff lib
	if (!tiffDll)
		tiffDll = new TiffLibDll(L"libtiff3.dll");

	// Read data and save tiff images
	for (int j = 1; j <= tCount; j++)
	{
		for (int i = 1; i <= zCount; i++)
		{
			if ((i == 1) && (j == 1))
			{
				strOME = uUIDSetup(channelArray, tCount, zCount);
			}

			for (int k = 0; k < cCount; k++)
			{
				// Get raw data
				int offset = ((j - 1) * cCount * zCount + (i - 1) * cCount + k) * imageSize;
				int result = TC_GetRawData(imageHandle, offset, imageSize, data);
				if (result != 0)
					continue;

				// tiff file name
				ostringstream filename;
				filename << resultFolder.c_str() << "\\" << channelArray->at(k) << "_001_001_" << format("%03d", i) << "_" << format("%03d", j) << ".tif";
				const wchar_t* tiffFileName = CharToWchar(filename.str().c_str());

				if (doOME)
					SaveTIFF(tiffFileName, data, width, height, umPerPixel, cCount, tCount, zCount, intervalSec, k, j, i, &acquiredDateTime, 0, &strOME, physicalSize, doCompression);
				else
					SaveTIFFWithoutOME(tiffFileName, data, width, height, umPerPixel, doCompression);
			}
		}
	}
	delete[] data;
	delete[] channelArray;
	TC_CloseRawImage(imageHandle);

	return 0;
}

