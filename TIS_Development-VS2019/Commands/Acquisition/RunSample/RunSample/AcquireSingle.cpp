#include "stdafx.h"
#include "..\..\..\..\Commands\General\SelectHardware\SelectHardware\SelectHardware.h"
#include "RunSample.h"
#include "AcquireSingle.h"
#include "ImageCorrection.h"
#include "..\..\..\..\Common\PublicFuncs.h"
#include <ctime>

extern auto_ptr<CommandDll> shwDll;
extern auto_ptr<TiffLibDll> tiffDll;
extern vector<ScanRegion> activeScanAreas;
extern long viewMode;

long _lsmChannel = 0;

AcquireSingle::AcquireSingle(IExperiment *pExperiment, wstring path)
{
	_pExp = pExperiment;
	_counter = 0;
	_zFrame = 1;
	_tFrame = 1;
	_path = path;

	_sp.doOME = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"OMETIFFTag",L"value", FALSE);
	_sp.doCompression = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"TIFFCompressionEnable",L"value", FALSE);
	_sp.doJPEG = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"JPEGEnable",L"value", FALSE);	
}

UINT StatusThreadProc( LPVOID pParam )
{
	long status = ICamera::STATUS_BUSY;

	ICamera * pCamera = (ICamera*)pParam;

	while(status == ICamera::STATUS_BUSY)
	{
		if(FALSE == pCamera->StatusAcquisition(status))
		{
			break;
		}
	}

	SetEvent( AcquireSingle::hEvent);

	return 0;
}

UINT StatusZThreadProc2( LPVOID pParam )
{
	long status = IDevice::STATUS_BUSY;

	IDevice * pDevice = (IDevice*)pParam;

	while(status == IDevice::STATUS_BUSY)
	{
		if(FALSE == pDevice->StatusPosition(status))
		{
			break;
		}
	}

	SetEvent( AcquireSingle::hEventZ);

	return 0;
}


HANDLE AcquireSingle::hEvent = NULL;
HANDLE AcquireSingle::hEventZ = NULL;
BOOL AcquireSingle::_evenOdd = FALSE;
double AcquireSingle::_lastGoodFocusPosition = 0.0;

long AcquireSingle::CallCaptureComplete(long captureComplete)
{
	return TRUE;
}

long AcquireSingle::CallCaptureImage(long index)
{
	CaptureImage(index);
	return TRUE;
}

long AcquireSingle::CallSaveImage(long index, BOOL isImageUpdate)
{
	SaveImage(index, isImageUpdate);
	return TRUE;
}

long AcquireSingle::CallCaptureSubImage(long index)
{
	CaptureSubImage(index);
	return TRUE;
}

long AcquireSingle::CallSaveSubImage(long index)
{
	SaveSubImage(index);
	return TRUE;
}

long AcquireSingle::CallSequenceStepCurrent(long index)
{
	SequenceStepCurrent(index);
	return TRUE;
}

long AcquireSingle::PreCaptureEventCheck(long &status)
{
	return TRUE;
}

long AcquireSingle::StopCaptureEventCheck(long &status)
{
	StopCapture(status);
	return TRUE;

}

long AcquireSingle::CallSaveZImage(long index, double power0, double power1, double power2, double power3,double power4, double power5)
{
	SaveZImage(index, power0, power1, power2, power3, power4, power5);
	return TRUE;
}

long AcquireSingle::CallSaveTImage(long index)
{
	SaveTImage(index);
	return TRUE;
}

long AcquireSingle::CallStartProgressBar(long index, long resetTotalCount)
{
	StartProgressBar(index, resetTotalCount);
	return TRUE;
}

long AcquireSingle::CallInformMessage(wchar_t* message)
{
	InformMessage(message);
	return TRUE;
}

long AcquireSingle::CallNotifySavedFileIPC(wchar_t* message)
{
	NotifySavedFileIPC(message);
	return TRUE;
}

long AcquireSingle::CallAutoFocusStatus(long status, long bestScore, double bestZPos, double nextZPos, long currRepeat)
{
	AutoFocusStatus(status, bestScore, bestZPos, nextZPos, currRepeat);
	return TRUE;
}

int Call_TiffVSetField(TIFF* out, uint32 ttag_t, ...)
{
	int retv;
	va_list marker;

	va_start( marker, ttag_t );  

	retv = tiffDll->TIFFVSetField(out, ttag_t, marker ); //actual setting of the colormap into the TIFF images

	va_end( marker );             

	return retv;
}

string CreateOMEMetadata(int width, int height,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string * acquiredDateTime, double deltaT, 
						 string * omeTiffData, PhysicalSize physicalSize)
{
	string tagData = "<?xml version=\"1.0\"?><OME xmlns=\"http://www.openmicroscopy.org/Schemas/OME/2010-06\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.openmicroscopy.org/Schemas/OME/2010-06 http://www.openmicroscopy.org/Schemas/OME/2010-06/ome.xsd\">";

	tagData += "<Image ID=\"Image:0\" AcquiredDate=";

	std::ostringstream ssDT;
	if(acquiredDateTime)
	{
		ssDT<<"\"" << *acquiredDateTime <<"\"";
	}
	else 
	{
		ssDT<<"\"\"";
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

	if(NULL == omeTiffData)
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

//string CreateOMEMetadata(int width, int height,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string * acquiredDateTime, double deltaT, string * omeTiffData)
//        {
//
//
//            string tagData = "<?xml version=\"1.0\"?><OME xmlns=\"http://www.openmicroscopy.org/Schemas/OME/2010-06\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.openmicroscopy.org/Schemas/OME/2010-06 http://www.openmicroscopy.org/Schemas/OME/2010-06/ome.xsd\">";
//
//			tagData += "<Image ID=\"Image:0\" AcquiredDate=";
//			
//			std::ostringstream ssDT;
//			if(acquiredDateTime)
//			{
//				ssDT<<"\"" << *acquiredDateTime <<"\"";
//			}else {
//				ssDT<<"\"\"";
//			}
//			tagData += ssDT.str();
//
//			tagData += "><Pixels";
//            tagData += " DimensionOrder=";
//            tagData += "\"XYCZT\"";
//            tagData += " ID=\"Pixels:0\"";
//           // tagData += " PhysicalSizeX=\"1.0\"";
//           // tagData += " PhysicalSizeY=\"1.0\"";
//           // tagData += " PhysicalSizeZ=\"1.0\"";			
//            tagData += " SizeC=";
//			std::ostringstream ssC;
//			ssC << "\"" << nc << "\"";
//            tagData += ssC.str();
//            tagData += " SizeT=";
//			std::ostringstream ssT;
//			ssT << "\"" << nt << "\"";
//            tagData += ssT.str();
//            tagData += " SizeX=";
//			std::ostringstream ssX;
//			ssX << "\"" << width << "\"";
//            tagData += ssX.str();
//            tagData += " SizeY=";
//			std::ostringstream ssY;
//			ssY << "\"" << height << "\"";
//            tagData += ssY.str();
//            tagData += " SizeZ=";
//			std::ostringstream ssZ;
//			ssZ << "\"" << nz << "\"";
//            tagData += ssZ.str();
//            tagData += " TimeIncrement=";
//			std::ostringstream ssTi;
//			ssTi << "\"" << timeIncrement << "\"";
//            tagData += ssTi.str();
//            tagData += " Type=";
//            tagData += "\"uint16\"";
//            tagData += ">";
//            tagData += "<Channel ID=";
//            tagData += "\"Channel:0:0\"";
//            tagData += " SamplesPerPixel=\"1\"><LightPath/></Channel>";
//            tagData += "<BinData BigEndian=\"false\" Length = \"0\" xmlns=\"http://www.openmicroscopy.org/Schemas/BinaryFile/2010-06\"/>";
//			tagData += "<Plane TheZ=";
//			std::ostringstream ssTheZ;
//			ssTheZ << "\"" << z << "\" ";
//			tagData += ssTheZ.str();
//			tagData += "TheT=";
//			std::ostringstream ssTheT;
//			ssTheT << "\"" << t << "\" ";
//			tagData += ssTheT.str();
//			tagData += "TheC=";
//			std::ostringstream ssTheC;
//			ssTheC << "\"" << c << "\" ";
//			tagData += ssTheC.str();
//			tagData += "DeltaT=";
//			std::ostringstream ssDeltaT;
//			ssDeltaT.setf(3);
//			ssDeltaT << "\"" << deltaT << "\"/>";
//			tagData += ssDeltaT.str();
//
//
//			if(NULL == omeTiffData)
//			{
//				std::ostringstream ssFirst;
//				ssFirst << "<TiffData FirstC=\"" << c << "\"" << " FirstT=\"" << t << "\"" << " FirstZ=\"" << z << "\"" << " IFD=\"0\" PlaneCount=\"1\">";
//				tagData += ssFirst.str();
//				tagData += "</TiffData>";
//			}
//			else
//			{
//				tagData += *omeTiffData;
//			}
//            tagData += "</Pixels>";
//            tagData += "</Image>";
//			tagData += "<StructuredAnnotations xmlns=\"http://www.openmicroscopy.org/Schemas/SA/2010-06\"/>";
//            tagData += "</OME>";
//
//            return tagData;
//}

long SaveTiledTiff(wchar_t *filePathAndName, char *pMemoryBuffer, long bufferSizeBytes, long imageWidth, long imageHeight, long tileWidth, long tileHeight, long totalChannels, long channelIndex, double umPerPixel, string imageDescription, bool compress = true)
{
	TIFF *out= tiffDll->TIFFOpenW(filePathAndName, "w"); 

	//XML-READ
	int sampleperpixel = 1;    // or 3 if there is no alpha channel, you should get a understanding of alpha in class soon. 

	//The length of a tile in a tiled image has to be a multiple of 16
	//When imaging lines, these can have as little as 1 or two pixels in height
	//to get around this  we pad with zeros the remainder of the tile
	//the logic below works for any pixel height for a single tile that is not a multiple of 16
	long MIN_TILE_LENGTH = 16;
	long newTileHeight = MIN_TILE_LENGTH;	
	if (tileHeight % MIN_TILE_LENGTH == 0)
	{
		newTileHeight = tileHeight;
	}
	else
	{		
		long remainder = (tileHeight) % MIN_TILE_LENGTH;
		long residualPixels = (0 == remainder) ? 0 : (MIN_TILE_LENGTH - remainder);
		newTileHeight = tileHeight + residualPixels;
	}

	long newImageHeight = 2 * newTileHeight;

	tiffDll->TIFFSetField(out, TIFFTAG_IMAGEWIDTH, imageWidth);  // set the width of the image
	tiffDll->TIFFSetField(out, TIFFTAG_IMAGELENGTH, newImageHeight);    // set the height of the image
	tiffDll->TIFFSetField(out, TIFFTAG_TILEWIDTH, tileWidth);
	tiffDll->TIFFSetField(out, TIFFTAG_TILELENGTH, newTileHeight);
	tiffDll->TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);   // set number of channels per pixel
	//XML-READ
	tiffDll->TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 16);    // set the size of the channels
	tiffDll->TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.

	//  Some other essential fields to set that you do not have to understand for now.
	if(compress)
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

	float pixelsPerInch = static_cast<float>(UM_PER_INCH_CONVERSION/umPerPixel);	
	Call_TiffVSetField(out, TIFFTAG_XRESOLUTION, pixelsPerInch);
	Call_TiffVSetField(out, TIFFTAG_YRESOLUTION, pixelsPerInch);

	Call_TiffVSetField(out, TIFFTAG_IMAGEDESCRIPTION, imageDescription.c_str());

	// We set the strip size of the file to be size of one row of pixels
	unsigned int stripSize = tiffDll->TIFFDefaultStripSize(out, imageWidth*sampleperpixel);
	tiffDll->TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, stripSize);

	long tileBytes =  tiffDll->TIFFTileSize(out);

	//if the tiles are padded with zeros, tilesBytes and originalTileBytes are not equal
	long originalTileBytes = tileWidth * tileHeight * 2;

	// allocate buffer for the tile to be written to image
	unsigned char * buf = (unsigned char *) tiffDll->_TIFFmalloc(tileBytes);

	long x, y, tileIndex = 0;

	// number of tiles contained in the buffer data
	int chs = (totalChannels == 1)? 1 : 4;
	long tiles = bufferSizeBytes / (originalTileBytes * chs);

	for (y = 0; y < newImageHeight; y += newTileHeight)
	{
		for (x = 0; x < imageWidth; x += tileWidth)
		{
			long offset = (chs == 1) ? (tileIndex * chs) * originalTileBytes : (channelIndex + tileIndex * chs) * originalTileBytes;

			if(tileIndex < tiles) 
			{
				memset(buf, 0, tileBytes);
				memcpy(buf, pMemoryBuffer + offset, originalTileBytes);
			}
			else 
			{
				// write out blank tile for the last tile
				memset(buf, 0, tileBytes);
			}

			tiffDll->TIFFWriteTile(out, buf, x, y, 0, 0); 

			tileIndex += 1;
		}
	}

	if (buf)
		tiffDll->_TIFFfree(buf);

	//Finally we close the output file, and destroy the buffer 
	tiffDll->TIFFClose(out); 

	return TRUE;
}

string WStringToStringAS(wstring ws)
{	
	size_t origsize = wcslen(ws.c_str()) + 1;
	const size_t newsize = _MAX_PATH;
	size_t convertedChars = 0;
	char nstring[newsize];
	wcstombs_s(&convertedChars, nstring, origsize, ws.c_str(), _TRUNCATE);

	string str(nstring);

	StringCbPrintfW(message,MSG_LENGTH,L"ExperimentManager ConvertWStringToString: %S",nstring);
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	return str;
}

long ParseApplicationSettingsXMLAS(const char* pcFilename)	// get OMETiffTag Enable
{
	long ret = FALSE;

	// load the ApplicationSettings.xml 
	ticpp::Document doc(pcFilename);
	doc.LoadFile();

	// parse through all children
	ticpp::Iterator<ticpp::Element> child;
	for(child = child.begin(doc.FirstChildElement()); child != child.end(); child++)
	{
		std::string strName;
		std::string strValue;
		child->GetValue(&strName);

		if ("OMETIFFTag" == strName)
		{
			// now parse through all the attributes of this fruit
			ticpp::Iterator< ticpp::Attribute > attribute;
			for(attribute = attribute.begin(child.Get()); attribute != attribute.end(); attribute++)
			{
				attribute->GetName(&strName);
				attribute->GetValue(&strValue);
				if ("1" == strValue)
				{
					ret = TRUE;
				}				
			}

			return ret;
		}
	}

	return ret;
}

long GetOMETIFFTagEnableFlagAS()
{
	wchar_t fileName[MAX_PATH];
	wstring tempPath = ResourceManager::getInstance()->GetApplicationSettingsPath();
	tempPath += wstring(L"ApplicationSettings.xml");
	StringCbPrintfW(fileName,_MAX_PATH, tempPath.c_str());

	long ret = ParseApplicationSettingsXMLAS(WStringToStringAS(fileName).c_str());

	return ret;	
}


//long SaveTIFFWithoutOME(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut,double umPerPixel,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string * acquiredDateTime,double deltaT, string * omeTiffData)
long SaveTIFFWithoutOME(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut,double umPerPixel,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string * acquiredDateTime,double deltaT,long doCompression)

{
	TIFF *out= tiffDll->TIFFOpenW(filePathAndName, "w"); 

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
	}else
	{
		tiffDll->TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);	
	}

	tiffDll->TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

	tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK); 

	//units are pixels per inch
	const int RESOLUTION_UNIT = 2;
	tiffDll->TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, RESOLUTION_UNIT); 

	const double UM_PER_INCH_CONVERSION = 25399.99999997256800000002962656;

	float pixelsPerInch = static_cast<float>(UM_PER_INCH_CONVERSION/umPerPixel);	
	Call_TiffVSetField(out, TIFFTAG_XRESOLUTION, pixelsPerInch);
	Call_TiffVSetField(out, TIFFTAG_YRESOLUTION, pixelsPerInch);

	// Color Palette is not working with the OME TIFF. /*TODO*/ determine the issue and reinsert at a later date
	//	tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
	//	Call_TiffVSetField(out, TIFFTAG_COLORMAP, rlut, glut, blut);

	tsize_t linebytes = sampleperpixel * width * 2;     // length in memory of one row of pixel in the image. 

	unsigned char *buf = NULL;        // buffer used to store the row of pixel information for writing to file

	// Allocating memory to store the pixels of current row
	if (tiffDll->TIFFScanlineSize(out)==linebytes)
		buf =(unsigned char *)tiffDll->_TIFFmalloc(linebytes);
	else
		buf = (unsigned char *)tiffDll->_TIFFmalloc(tiffDll->TIFFScanlineSize(out));

	// We set the strip size of the file to be size of one row of pixels
	tiffDll->TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, tiffDll->TIFFDefaultStripSize(out, width*sampleperpixel));


	//Now writing image to the file one strip at a time
	for (uint32 row = 0; row < static_cast<uint32>(height); row++)
	{
		memcpy(buf, &pMemoryBuffer[row*linebytes], linebytes);    // check the index here, and figure out why not using h*linebytes
		if (tiffDll->TIFFWriteScanline(out, buf, row, 0) < 0)
			break;
	}

	if (buf)
		tiffDll->_TIFFfree(buf);

	//Finally we close the output file, and destroy the buffer 
	tiffDll->TIFFClose(out); 

	return TRUE;
}

long SaveTIFF(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut,
			  double umPerPixel,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string * acquiredDateTime,double deltaT, 
			  string * omeTiffData, PhysicalSize physicalSize,long doCompression)
{
	TIFF *out= tiffDll->TIFFOpenW(filePathAndName, "w"); 

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
	}else
	{
		tiffDll->TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);	
	}
	tiffDll->TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

	tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK); 

	//units are pixels per inch
	const int RESOLUTION_UNIT = 2;
	tiffDll->TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, RESOLUTION_UNIT); 

	const double UM_PER_INCH_CONVERSION = 25399.99999997256800000002962656;

	float pixelsPerInch = static_cast<float>(UM_PER_INCH_CONVERSION/umPerPixel);	
	Call_TiffVSetField(out, TIFFTAG_XRESOLUTION, pixelsPerInch);
	Call_TiffVSetField(out, TIFFTAG_YRESOLUTION, pixelsPerInch);

	// Color Palette is not working with the OME TIFF. /*TODO*/ determine the issue and reinsert at a later date
	//	tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
	//	Call_TiffVSetField(out, TIFFTAG_COLORMAP, rlut, glut, blut);

	string str = CreateOMEMetadata(width, height,nc, nt, nz, timeIncrement, c, t, z,acquiredDateTime, deltaT, omeTiffData,physicalSize);

	Call_TiffVSetField(out, TIFFTAG_IMAGEDESCRIPTION, str.c_str());

	tsize_t linebytes = sampleperpixel * width * 2;     // length in memory of one row of pixel in the image. 

	unsigned char *buf = NULL;        // buffer used to store the row of pixel information for writing to file

	// Allocating memory to store the pixels of current row
	if (tiffDll->TIFFScanlineSize(out)==linebytes)
		buf =(unsigned char *)tiffDll->_TIFFmalloc(linebytes);
	else
		buf = (unsigned char *)tiffDll->_TIFFmalloc(tiffDll->TIFFScanlineSize(out));

	// We set the strip size of the file to be size of one row of pixels
	tiffDll->TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, tiffDll->TIFFDefaultStripSize(out, width*sampleperpixel));


	//Now writing image to the file one strip at a time
	for (uint32 row = 0; row < static_cast<uint32>(height); row++)
	{
		memcpy(buf, &pMemoryBuffer[row*linebytes], linebytes);    // check the index here, and figure out why not using h*linebytes
		if (tiffDll->TIFFWriteScanline(out, buf, row, 0) < 0)
			break;
	}

	if (buf)
		tiffDll->_TIFFfree(buf);

	//Finally we close the output file, and destroy the buffer 
	tiffDll->TIFFClose(out); 

	return TRUE;
}
//
//long SaveTIFF(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,
//			  unsigned short * blut,double umPerPixel,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string * acquiredDateTime,double deltaT, 
//			  string * omeTiffData)
//{
//	TIFF *out= tiffDll->TIFFOpenW(filePathAndName, "w"); 
//
//	//XML-READ
//	int sampleperpixel = 1;    // or 3 if there is no alpha channel, you should get a understanding of alpha in class soon. 
//
//	tiffDll->TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width);  // set the width of the image
//	tiffDll->TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);    // set the height of the image
//	tiffDll->TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);   // set number of channels per pixel
//	//XML-READ
//	tiffDll->TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 16);    // set the size of the channels
//	tiffDll->TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
//
//	//  Some other essential fields to set that you do not have to understand for now.
//	tiffDll->TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
//	tiffDll->TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
//	
//	tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK); 
//
//	//units are pixels per inch
//	const int RESOLUTION_UNIT = 2;
//	tiffDll->TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, RESOLUTION_UNIT); 
//
//	const double UM_PER_INCH_CONVERSION = 25399.99999997256800000002962656;
//
//	float pixelsPerInch = static_cast<float>(UM_PER_INCH_CONVERSION/umPerPixel);	
//	Call_TiffVSetField(out, TIFFTAG_XRESOLUTION, pixelsPerInch);
//	Call_TiffVSetField(out, TIFFTAG_YRESOLUTION, pixelsPerInch);
//
//	// Color Palette is not working with the OME TIFF. /*TODO*/ determine the issue and reinsert at a later date
////	tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
////	Call_TiffVSetField(out, TIFFTAG_COLORMAP, rlut, glut, blut);
//	//long GetOMETIFFTagEnableFlag()
//	
//	if (GetOMETIFFTagEnableFlagAS())
//	{
//		string str = CreateOMEMetadata(width, height,nc, nt, nz, timeIncrement, c, t, z,acquiredDateTime, deltaT, omeTiffData);
//		Call_TiffVSetField(out, TIFFTAG_IMAGEDESCRIPTION, str.c_str());
//	}	
//
//	tsize_t linebytes = sampleperpixel * width * 2;     // length in memory of one row of pixel in the image. 
//
//	unsigned char *buf = NULL;        // buffer used to store the row of pixel information for writing to file
//
//	// Allocating memory to store the pixels of current row
//	if (tiffDll->TIFFScanlineSize(out)==linebytes)
//		buf =(unsigned char *)tiffDll->_TIFFmalloc(linebytes);
//	else
//		buf = (unsigned char *)tiffDll->_TIFFmalloc(tiffDll->TIFFScanlineSize(out));
//
//	// We set the strip size of the file to be size of one row of pixels
//	tiffDll->TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, tiffDll->TIFFDefaultStripSize(out, width*sampleperpixel));
//
//
//	//Now writing image to the file one strip at a time
//	for (uint32 row = 0; row < static_cast<uint32>(height); row++)
//	{
//		memcpy(buf, &pMemoryBuffer[row*linebytes], linebytes);    // check the index here, and figure out why not using h*linebytes
//		if (tiffDll->TIFFWriteScanline(out, buf, row, 0) < 0)
//			break;
//	}
//
//	if (buf)
//		tiffDll->_TIFFfree(buf);
//
//	//Finally we close the output file, and destroy the buffer 
//	tiffDll->TIFFClose(out); 
//
//	return TRUE;
//}

long SaveTIFF8(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut,
			   double umPerPixel,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string * acquiredDateTime,double deltaT, string * omeTiffData,
			   long bitShiftToEight,PhysicalSize physicalSize)
{
	TIFF *out= tiffDll->TIFFOpenW(filePathAndName, "w"); 

	//XML-READ
	int sampleperpixel = 1;    // or 3 if there is no alpha channel, you should get a understanding of alpha in class soon. 

	tiffDll->TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width);  // set the width of the image
	tiffDll->TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);    // set the height of the image
	tiffDll->TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);   // set number of channels per pixel
	//XML-READ
	tiffDll->TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);    // set the size of the channels
	tiffDll->TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.

	//  Some other essential fields to set that you do not have to understand for now.
	tiffDll->TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	tiffDll->TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

	tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK); 

	//units are pixels per inch
	const int RESOLUTION_UNIT = 2;
	tiffDll->TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, RESOLUTION_UNIT); 

	const double UM_PER_INCH_CONVERSION = 25399.99999997256800000002962656;

	float pixelsPerInch = static_cast<float>(UM_PER_INCH_CONVERSION/umPerPixel);	
	Call_TiffVSetField(out, TIFFTAG_XRESOLUTION, pixelsPerInch);
	Call_TiffVSetField(out, TIFFTAG_YRESOLUTION, pixelsPerInch);

	// Color Palette is not working with the OME TIFF. /*TODO*/ determine the issue and reinsert at a later date
	//	tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
	//	Call_TiffVSetField(out, TIFFTAG_COLORMAP, rlut, glut, blut);


	string str = CreateOMEMetadata(width, height,nc, nt, nz, timeIncrement, c, t, z,acquiredDateTime, deltaT, omeTiffData, physicalSize);
	Call_TiffVSetField(out, TIFFTAG_IMAGEDESCRIPTION, str.c_str());


	tsize_t linebytes = sampleperpixel * width;     // length in memory of one row of pixel in the image. 

	unsigned char *buf = NULL;        // buffer used to store the row of pixel information for writing to file

	// Allocating memory to store the pixels of current row
	if (tiffDll->TIFFScanlineSize(out)==linebytes)
		buf =(unsigned char *)tiffDll->_TIFFmalloc(linebytes);
	else
		buf = (unsigned char *)tiffDll->_TIFFmalloc(tiffDll->TIFFScanlineSize(out));

	// We set the strip size of the file to be size of one row of pixels
	tiffDll->TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, tiffDll->TIFFDefaultStripSize(out, width*sampleperpixel));


	//Now writing image to the file one strip at a time
	for (uint32 row = 0; row < static_cast<uint32>(height); row++)
	{
		unsigned char *pBuf = buf;
		unsigned short *pMem = (unsigned short*)&pMemoryBuffer[row*linebytes*2];
		for(uint32 col =0; col <static_cast<uint32>(linebytes); col++)
		{
			*pBuf = (*pMem)>>bitShiftToEight;    // check the index here, and figure out why not using h*linebytes
			pBuf++;
			pMem++;
		}
		if (tiffDll->TIFFWriteScanline(out, buf, row, 0) < 0)
			break;
	}

	if (buf)
		tiffDll->_TIFFfree(buf);

	//Finally we close the output file, and destroy the buffer 
	tiffDll->TIFFClose(out); 

	return TRUE;
}

long SaveTIFF8WithoutOME(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut,
						 double umPerPixel,int nc, int nt, int nz, double timeIncrement, int c, int t, int z,string * acquiredDateTime,double deltaT, 
						 string * omeTiffData, long bitShiftToEight)
{
	TIFF *out= tiffDll->TIFFOpenW(filePathAndName, "w"); 

	//XML-READ
	int sampleperpixel = 1;    // or 3 if there is no alpha channel, you should get a understanding of alpha in class soon. 

	tiffDll->TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width);  // set the width of the image
	tiffDll->TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);    // set the height of the image
	tiffDll->TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);   // set number of channels per pixel
	//XML-READ
	tiffDll->TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);    // set the size of the channels
	tiffDll->TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.

	//  Some other essential fields to set that you do not have to understand for now.
	tiffDll->TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	tiffDll->TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

	tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK); 

	//units are pixels per inch
	const int RESOLUTION_UNIT = 2;
	tiffDll->TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, RESOLUTION_UNIT); 

	const double UM_PER_INCH_CONVERSION = 25399.99999997256800000002962656;

	float pixelsPerInch = static_cast<float>(UM_PER_INCH_CONVERSION/umPerPixel);	
	Call_TiffVSetField(out, TIFFTAG_XRESOLUTION, pixelsPerInch);
	Call_TiffVSetField(out, TIFFTAG_YRESOLUTION, pixelsPerInch);

	// Color Palette is not working with the OME TIFF. /*TODO*/ determine the issue and reinsert at a later date
	//	tiffDll->TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
	//	Call_TiffVSetField(out, TIFFTAG_COLORMAP, rlut, glut, blut);

	tsize_t linebytes = sampleperpixel * width;     // length in memory of one row of pixel in the image. 

	unsigned char *buf = NULL;        // buffer used to store the row of pixel information for writing to file

	// Allocating memory to store the pixels of current row
	if (tiffDll->TIFFScanlineSize(out)==linebytes)
		buf =(unsigned char *)tiffDll->_TIFFmalloc(linebytes);
	else
		buf = (unsigned char *)tiffDll->_TIFFmalloc(tiffDll->TIFFScanlineSize(out));

	// We set the strip size of the file to be size of one row of pixels
	tiffDll->TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, tiffDll->TIFFDefaultStripSize(out, width*sampleperpixel));


	//Now writing image to the file one strip at a time
	for (uint32 row = 0; row < static_cast<uint32>(height); row++)
	{
		unsigned char *pBuf = buf;
		unsigned short *pMem = (unsigned short*)&pMemoryBuffer[row*linebytes*2];
		for(uint32 col =0; col <static_cast<uint32>(linebytes); col++)
		{
			*pBuf = (*pMem)>>bitShiftToEight;    // check the index here, and figure out why not using h*linebytes
			pBuf++;
			pMem++;
		}
		if (tiffDll->TIFFWriteScanline(out, buf, row, 0) < 0)
			break;
	}

	if (buf)
		tiffDll->_TIFFfree(buf);

	//Finally we close the output file, and destroy the buffer 
	tiffDll->TIFFClose(out); 

	return TRUE;
}

long SaveJPEG(wchar_t *filePathAndName, char * pMemoryBuffer, long width, long height, unsigned short * rlut, unsigned short * glut,unsigned short * blut,long bitDepth)
{
	int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
	int color_space = JCS_RGB;		/* red/green/blue */

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	/* this is a pointer to one row of image data */
	JSAMPARRAY buffer;

	wstring ws(filePathAndName);

	size_t origsize = wcslen(ws.c_str()) + 1;
	const size_t newsize = _MAX_PATH;
	size_t convertedChars = 0;
	char nstring[newsize];
	wcstombs_s(&convertedChars, nstring, origsize, ws.c_str(), _TRUNCATE);

	string str(nstring);

	FILE *outfile;

	if(0 != fopen_s(&outfile,str.c_str(), "wb"))
	{
		StringCbPrintfW(message,MSG_LENGTH,L"AcquireSingle Execute Error opening output jpeg %s",filePathAndName);
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		return FALSE;
	}

	cinfo.err = jpeg_std_error( &jerr );

	jpeg_CreateCompress(&cinfo,JPEG_LIB_VERSION, (size_t) sizeof(struct jpeg_compress_struct));

	jpeg_stdio_dest(&cinfo, outfile);

	// Setting the parameters of the output file here 
	cinfo.image_width = width;	
	cinfo.image_height = height;
	cinfo.input_components = bytes_per_pixel;
	cinfo.in_color_space = (J_COLOR_SPACE)color_space;

	// default compression parameters, we shouldn't be worried about these 
	jpeg_set_defaults( &cinfo );		

	jpeg_start_compress( &cinfo, TRUE );

	unsigned char * rowBuffer = new unsigned char[cinfo.image_width*cinfo.input_components];
	unsigned short *p1;
	unsigned char *p2;

	long shiftValue = 0;

	// SaveJPEG is called from Execute, if a new camera with a different bitDepth is added, this switch statement should change
	// to handle a new case.
	switch(bitDepth)
	{
	case 12:shiftValue=4;break;
	case 14:shiftValue=2;break;
	case 16:shiftValue=0;break;
	}
	while( cinfo.next_scanline < cinfo.image_height )
	{
		p1 = (unsigned short*)pMemoryBuffer;
		p1 += cinfo.next_scanline * cinfo.image_width;
		p2 = rowBuffer;

		for(unsigned int i = 0; i<cinfo.image_width; i++)
		{
			//lookup table is scalled to the full 65536 unsigned short range. Scale the raw data (shfitValue) to fill out the lookup table
			const long COLOR_MAP_SIZE =65536;
			long lookUpVal = min(COLOR_MAP_SIZE-1,(*p1)<<shiftValue);

			*p2 = rlut[lookUpVal]>>8;								
			p2++;
			*p2 = glut[lookUpVal]>>8;								
			p2++;
			*p2 = blut[lookUpVal]>>8;								
			p2++;
			p1++;
		}

		buffer = (JSAMPARRAY)&rowBuffer;
		jpeg_write_scanlines( &cinfo, buffer, 1 );
	}

	delete[] rowBuffer;

	// similar to read file, clean up after we're done compressing 
	jpeg_finish_compress( &cinfo );
	jpeg_destroy_compress( &cinfo );

	fclose( outfile );

	return TRUE;
}

void GetColorInfo(HardwareSetupXML *pHardware,string wavelengthName, long &red, long &green, long &blue,long &bp, long &wp)
{
	string color;
	double ex, em, dic;
	long fluor;
	pHardware->GetWavelength(wavelengthName, ex, em, dic, fluor, color, bp, wp);

	//splitting the color into R G B values
	string strOrig = color.erase(0,3);
	string strresult = strOrig;
	string strR = strresult.erase(2,4);
	strresult = strOrig;
	string strTemp = strresult.erase(0,2);
	string strG = strTemp.erase(2,2);
	strresult = strOrig;
	string strB = strresult.erase(0,4);

	istringstream isR(strR);
	istringstream isG(strG);
	istringstream isB(strB);

	long num;
	isR >> hex >> num;
	red = num;

	isG >> hex >> num;
	green = num;

	isB >> hex >>num;
	blue = num;	
}

void GetLookUpTables(unsigned short * rlut, unsigned short * glut, unsigned short *blut,long red, long green, long blue, long bp, long wp, long bitdepth)
{
	const long COLOR_MAP_SIZE = 65536;
	const double COLOR_MAP_SCALE_FACTOR = 256.0;

	double bp_s = bp * COLOR_MAP_SCALE_FACTOR;
	double wp_s = wp * COLOR_MAP_SCALE_FACTOR;

	double val = 1.0;

	if(16 == bitdepth)
	{
		val = 256.0;
	}
	else if(8 == bitdepth)
	{
		val = 1.0;
	}

	double red_s = red / val;
	double green_s = green / val;
	double blue_s = blue / val;

	for (long i = 0; i < COLOR_MAP_SIZE;i++)
	{
		double a = (COLOR_MAP_SIZE-1)/(wp_s - bp_s);
		double b = 0 - (a * bp_s);

		double dvalR = (a * i * (red_s)) + bp_s;
		dvalR = max(dvalR, bp_s);
		dvalR = min(dvalR, wp_s);

		double dvalG = (a * i * (green_s)) + bp_s;
		dvalG = max(dvalG, bp_s);
		dvalG = min(dvalG, wp_s);

		double dvalB = (a * i * (blue_s)) + bp_s;
		dvalB = max(dvalB, bp_s);
		dvalB = min(dvalB, wp_s);

		rlut[i]= static_cast<unsigned short>(dvalR);
		glut[i]= static_cast<unsigned short>(dvalG);
		blut[i]= static_cast<unsigned short>(dvalB);
	}
}

long AcquireSingle::Execute(long index, long subWell, long zFrame, long tFrame)
{
	_zFrame = zFrame;
	_tFrame = tFrame;
	return Execute(index,subWell);
}

string AcquireSingle::uUIDSetup(auto_ptr<HardwareSetupXML> &pHardware, long timePoints, long zstageSteps, long index, long subWell)
{
	wchar_t filePathAndName[_MAX_PATH];
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];

	string strOME;

	//build wavelengthname list for quick query later
	vector<string> wavelengthNames;

	//Get the Capture Sequence Settings.
	//If it is a sequential capture, with more than one step the wavelenth
	//information for the steps in the sequence needs to be retrieved to
	//have the corrent information in the OME.
	long captureSequenceEnable = FALSE;
	_pExp->GetCaptureSequence(captureSequenceEnable);
	vector<IExperiment::SequenceStep> captureSequence;
	_pExp->GetSequenceSteps(captureSequence);
	if (TRUE == captureSequenceEnable && captureSequence.size() > 1)
	{
		for (long i=0; i < captureSequence.size(); i++)
		{
			for (long j=0; j < captureSequence[i].Wavelength.size(); j++)
			{
				wavelengthNames.push_back(captureSequence[i].Wavelength[j].name);
			}
		}
		//Because the wavelengths might be out of order in the sequence
		//Sort them, so the channels are in alphabetical order
		sort(wavelengthNames.begin(), wavelengthNames.end());
	}
	else
	{
		long bufferChannels = _pExp->GetNumberOfWavelengths();
		for(long c=0; c<bufferChannels; c++)
		{
			string wavelengthName;
			double exposureTimeMS;
			_pExp->GetWavelength(c, wavelengthName,exposureTimeMS);
			wavelengthNames.push_back(wavelengthName);
		}
	}

	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS,L"ImageNameFormat",L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	std::wstringstream imgNameFormat;
	imgNameFormat << L"%S_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%" 
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tif";

	for(long t=0; t<timePoints; t++)
	{
		for(long z=0; z<zstageSteps; z++)
		{
			for(long c=0; c<wavelengthNames.size(); c++)
			{	
				string wavelengthName = wavelengthNames.at(c);

				GUID *pguid = 0x00;

				pguid = new GUID;

				CoCreateGuid(pguid);

				OLECHAR* bstrGuid;
				StringFromCLSID(*pguid, &bstrGuid);

				wstring ws(bstrGuid);

				//remove the curly braces at the end and start of the guid
				ws.erase(ws.size()-1,1);
				ws.erase(0,1);

				string strGuid = ConvertWStringToString(ws);

				ostringstream ss;
				ss << "<TiffData" << " FirstT=\"" << t << "\"" << " FirstZ=\"" << z << "\"" << " FirstC=\"" << c << "\">" ;

				_wsplitpath_s(_path.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);

				StringCbPrintfW(filePathAndName,_MAX_PATH,imgNameFormat.str().c_str(),wavelengthName.c_str(),index,subWell,z+1,t+1);

				wstring wsPath(filePathAndName);
				string strFilePathAndName =  ConvertWStringToString(wsPath);
				ss << "<UUID FileName=\"" << strFilePathAndName.c_str() << "\">" << "urn:uuid:" << strGuid.c_str()  << "</UUID>" << "</TiffData>"; 
				strOME += ss.str();
				// ensure memory is freed
				::CoTaskMemFree(bstrGuid);	

				delete pguid; 
			}
		}
	}

	return strOME;
}

long SetupDimensions(ICamera *pCamera,IExperiment *pExperiment,double fieldSizeCalibration, double magnification, Dimensions &d, long &avgFrames, long &bufferChannels, long &avgMode, double &umPerPixel, long &numPlanes)
{	
	long width,height;

	double typeVal;

	pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE,typeVal);

	long cameraType = static_cast<long>(typeVal);
	avgMode = ICamera::AVG_MODE_NONE;

	switch(cameraType)
	{
	case ICamera::CCD:
		{
			string camName;
			long camImageWidth;
			long camImageHeight;
			double camPixelSize;
			double camExposureTimeMS;		
			long gain, blackLevel, lightMode;				
			long left,top,right,bottom;
			long binningX, binningY;
			long tapsIndex, tapsBalance;
			long readoutSpeedIndex;
			long camAverageMode, camAverageNum;
			long camVericalFlip, camHorizontalFlip, imageAngle;

			//getting the values from the experiment setup XML files
			pExperiment->GetCamera(camName,camImageWidth,camImageHeight,camPixelSize,camExposureTimeMS,gain,blackLevel,lightMode,left,top,right,bottom,binningX,binningY,tapsIndex,tapsBalance,readoutSpeedIndex,camAverageMode,camAverageNum,camVericalFlip,camHorizontalFlip,imageAngle);
			width = camImageWidth;
			height = camImageHeight;
			_lsmChannel = bufferChannels = 1;	//StatsManager computeStats is requesting bitwise _lsmChannel

			d.c = bufferChannels;
			d.dType = INT_16BIT;
			d.m = 1;
			d.mType = DETACHED_CHANNEL;
			d.t = 1;
			d.x = width;
			d.y = height;
			d.z = 1;
			d.imageBufferType = 0;
			avgMode = camAverageMode;
			//if the average mode is enabled and set to cumulative. assign the cumulative average frame count.
			if(ICamera::AVG_MODE_CUMULATIVE == avgMode)
			{
				pCamera->SetParam(ICamera::PARAM_LSM_AVERAGEMODE,ICamera::AVG_MODE_NONE);
				avgFrames = camAverageNum;
			}
			numPlanes = 1;
		}
		break;		
	case ICamera::CCD_MOSAIC:
		{
			string camName;
			long camImageWidth;
			long camImageHeight;
			double camPixelSize;
			double camExposureTimeMS;		
			long gain, blackLevel, lightMode;				
			long left,top,right,bottom;
			long binningX, binningY;
			long tapsIndex, tapsBalance;
			long readoutSpeedIndex;
			long camAverageMode, camAverageNum;
			long camVericalFlip, camHorizontalFlip, imageAngle;

			//getting the values from the experiment setup XML files
			pExperiment->GetCamera(camName,camImageWidth,camImageHeight,camPixelSize,camExposureTimeMS,gain,blackLevel,lightMode,left,top,right,bottom,binningX,binningY,tapsIndex,tapsBalance,readoutSpeedIndex,camAverageMode,camAverageNum,camVericalFlip,camHorizontalFlip,imageAngle);
			width = static_cast<long>((right - left)/binningX);
			height = static_cast<long>((bottom - top)/binningY);
			bufferChannels = 3;
			_lsmChannel = 1;	//StatsManager computeStats is requesting bitwise _lsmChannel

			d.c = bufferChannels;
			d.dType = INT_16BIT;
			d.m = 1;
			d.mType = CONTIGUOUS_CHANNEL;
			d.t = 1;
			d.x = width;
			d.y = height;
			d.z = 1;
			d.imageBufferType = 0;
			numPlanes = 1;
		}
		break;
	case ICamera::LSM:
		{
			long areaMode;
			double areaAngle;
			long scanMode;
			long interleave;
			long pixelX;
			long pixelY;
			long lsmChannel;
			long fieldSize;
			long offsetX; 
			long offsetY;
			long averageMode;
			long averageNum;
			long clockSource;
			long inputRange1;
			long inputRange2;
			long twoWayAlignment;
			long extClockRate;
			double dwellTime;
			long flybackCycles;
			long inputRange3;
			long inputRange4;
			long minimizeFlybackCycles;
			long polarity[4];
			long verticalFlip;
			long horizontalFlip;
			double crsFrequencyHz = 0;
			long timeBasedLineScan = FALSE;
			long timeBasedLineScanMS = 0;
			long threePhotonEnable = FALSE;
			long numberOfPlanes = 1;
			pExperiment->GetLSM(areaMode,areaAngle,scanMode,interleave,pixelX,pixelY,lsmChannel,fieldSize,offsetX,offsetY,averageMode,averageNum,clockSource,inputRange1,inputRange2,twoWayAlignment,extClockRate,dwellTime,flybackCycles,inputRange3,inputRange4,minimizeFlybackCycles, polarity[0],polarity[1],polarity[2],polarity[3], verticalFlip, horizontalFlip, crsFrequencyHz, timeBasedLineScan, timeBasedLineScanMS, threePhotonEnable, numberOfPlanes);
			
			pCamera->SetParam(ICamera::PARAM_LSM_VERTICAL_SCAN_DIRECTION, verticalFlip);
			pCamera->SetParam(ICamera::PARAM_LSM_HORIZONTAL_FLIP, horizontalFlip);
			pCamera->SetParam(ICamera::PARAM_LSM_DWELL_TIME, dwellTime);
			pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_X, pixelX);
			pCamera->SetParam(ICamera::PARAM_LSM_PIXEL_Y, pixelY);
			pCamera->SetParam(ICamera::PARAM_LSM_FIELD_SIZE, fieldSize);
			pCamera->SetParam(ICamera::PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY, FALSE);

			//if its a timebased line scan then we want to get the pixel Y from the camera instead of assuming that it is what we set it as
			if (pCamera->SetParam(ICamera::PARAM_LSM_TIME_BASED_LINE_SCAN, timeBasedLineScan) && TRUE == timeBasedLineScan)
			{
				pCamera->SetParam(ICamera::PARAM_LSM_TB_LINE_SCAN_TIME_MS, timeBasedLineScanMS);
				double lsmHeight;
				pCamera->GetParam(ICamera::PARAM_LSM_PIXEL_Y, lsmHeight);
				height = pixelY = static_cast<long>(lsmHeight);
			}
			else
			{
				height = pixelY;
			}

			_lsmChannel = lsmChannel;

			width = static_cast<long>(pixelX);

			double temp;
			pCamera->GetParam(ICamera::PARAM_LSM_CHANNEL,temp);
			long channel = static_cast<long>(temp);


			switch(channel)
			{
			case 0x1:bufferChannels = 1;break;
			case 0x2:bufferChannels = 1;break;
			case 0x4:bufferChannels = 1;break;
			case 0x8:bufferChannels = 1;break;
			default:
				{
					long paramType;
					long paramAvailable;
					long paramReadOnly;
					double paramMin;
					double paramMax;
					double paramDefault;

					pCamera->GetParamInfo(ICamera::PARAM_LSM_CHANNEL,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault);
					switch(static_cast<long>(paramMax))
					{
					case 0x3:bufferChannels = 3;break;
					case 0xF:bufferChannels = 4;break;
					default:bufferChannels = 3;
					}
				}
				break;
			}

			numPlanes = numberOfPlanes >= 1 && TRUE == threePhotonEnable ? numberOfPlanes : 1;

			d.c = bufferChannels;
			d.dType = INT_16BIT;
			d.m = 1;
			d.mType = CONTIGUOUS_CHANNEL;
			d.t = 1;
			d.x = width;
			d.y = height * numPlanes;
			d.z = 1;
			d.imageBufferType = 0;
			avgMode = averageMode;
			//if the average mode is enabled and set to cumulative. assign the cumulative average frame count.
			if(ICamera::AVG_MODE_CUMULATIVE == avgMode)
			{
				pCamera->SetParam(ICamera::PARAM_LSM_AVERAGEMODE,ICamera::AVG_MODE_NONE);
				avgFrames = averageNum;
			}

			umPerPixel = (fieldSize * fieldSizeCalibration)/(pixelX * magnification);

		}
		break;
	}
	return TRUE;
}

long AcquireSingle::Execute(long index, long subWell)
{
	double magnification;
	string objName;
	_pExp->GetMagnification(magnification, objName);
	//Get filter parameters from hardware setup.xml

	auto_ptr<HardwareSetupXML> pHardware(new HardwareSetupXML());

	long position = 0;
	double numAperture;
	double afStartPos = 0;
	double afFocusOffset = 0;
	double afAdaptiveOffset = 0;
	long beamExpPos = 0;
	long beamExpWavelength = 0;
	long beamExpPos2 = 0;
	long beamExpWavelength2 = 0;
	long turretPosition = 0;
	long zAxisToEscape = 0;
	double zAxisEscapeDistance = 0;
	double fineAutoFocusPercentage = 0.15;

	pHardware->GetMagInfoFromName(objName, magnification, position, numAperture, afStartPos, afFocusOffset, afAdaptiveOffset, beamExpPos, beamExpWavelength, beamExpPos2, beamExpWavelength2, turretPosition, zAxisToEscape, zAxisEscapeDistance, fineAutoFocusPercentage);

	_adaptiveOffset = afAdaptiveOffset;

	ICamera* pCamera = NULL;

	pCamera = GetCamera(SelectedHardware::SELECTED_CAMERA1);

	if (NULL == pCamera)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"RunSample Execute could not create camera");
		return FALSE;
	}

	double fieldSizeCalibration = 100.0;
	pCamera->GetParam(ICamera::PARAM_LSM_FIELD_SIZE_CALIBRATION, fieldSizeCalibration);

	Dimensions baseDimensions;
	long avgFrames = 1;
	long bufferChannels = 1;
	long avgMode = ICamera::AVG_MODE_NONE;
	double umPerPixel = 1.0;
	long numberOfPlanes = 1;
	SetupDimensions(pCamera, _pExp, fieldSizeCalibration, magnification, baseDimensions, avgFrames, bufferChannels, avgMode, umPerPixel, numberOfPlanes);
	//Set temp path to streaming temp path:
	wstring tempPath; double previewRate;
	pHardware->GetStreaming(tempPath, previewRate);
	if (NULL != tempPath.c_str())
	{
		ImageManager::getInstance()->SetMemMapPath((tempPath + L"\\").c_str());
	}

	char* pMemoryBuffer = NULL;
	char* pMemoryBufferDFLIMHisto = NULL;
	char* pMemoryBufferDFLIMSinglePhoton = NULL;
	char* pMemoryBufferDFLIMArrivalTimeSum = NULL;
	char* pMemoryBufferDFLIMPhotons = NULL;

	long tempImageID;

	map<long, long> imageIDs;
	map<long, Dimensions> buffersDimensions;
	double dflimType = 0;
	long imageMethod = (pCamera->GetParam(ICamera::PARAM_DFLIM_FRAME_TYPE, dflimType)) ? static_cast<long>(dflimType) : 0;

	Dimensions dIntensity = baseDimensions;
	dIntensity.imageBufferType = BufferType::INTENSITY;

	if (ImageManager::getInstance()->CreateImage(tempImageID, dIntensity, L"intensity") == FALSE)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"RunSample Execute could not create intensity  memory buffer");
		return FALSE;
	}
	imageIDs.insert(std::pair<long, long>(BufferType::INTENSITY, tempImageID));
	buffersDimensions.insert(std::pair<long, Dimensions>(BufferType::INTENSITY, dIntensity));

	//if DFLIM
	if (imageMethod == 1)
	{
		const long DFLIM_HISTOGRAM_BINS = 256;
		Dimensions ddflimHisto = baseDimensions;
		ddflimHisto.imageBufferType = BufferType::DFLIM_HISTOGRAM;
		ddflimHisto.x = DFLIM_HISTOGRAM_BINS;
		ddflimHisto.y = 1;
		ddflimHisto.dType = DataType::INT_32BIT;
		if (ImageManager::getInstance()->CreateImage(tempImageID, ddflimHisto, L"dFLIMShisto") == FALSE)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"RunSample Execute could not create dflim histogram  memory buffer");
			return FALSE;
		}
		imageIDs.insert(std::pair<long, long>(BufferType::DFLIM_HISTOGRAM, tempImageID));
		buffersDimensions.insert(std::pair<long, Dimensions>(BufferType::DFLIM_HISTOGRAM, ddflimHisto));

		Dimensions ddflimSinglePhoton = baseDimensions;
		ddflimSinglePhoton.imageBufferType = BufferType::DFLIM_IMAGE_SINGLE_PHOTON;
		ddflimSinglePhoton.dType = DataType::INT_16BIT;
		if (ImageManager::getInstance()->CreateImage(tempImageID, ddflimSinglePhoton, L"dFLIMSinglePhoton") == FALSE)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"RunSample Execute could not create dFLIMSinglePhoton  memory buffer");
			return FALSE;
		}
		imageIDs.insert(std::pair<long, long>(BufferType::DFLIM_IMAGE_SINGLE_PHOTON, tempImageID));
		buffersDimensions.insert(std::pair<long, Dimensions>(BufferType::DFLIM_IMAGE_SINGLE_PHOTON, ddflimSinglePhoton));

		Dimensions ddflimArrivalTimeSum = baseDimensions;
		ddflimArrivalTimeSum.imageBufferType = BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM;
		ddflimArrivalTimeSum.dType = DataType::INT_32BIT;
		if (ImageManager::getInstance()->CreateImage(tempImageID, ddflimArrivalTimeSum, L"dFLIMArrivalTimeSum") == FALSE)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"RunSample Execute could not create dFLIMArrivalTimeSum  memory buffer");
			return FALSE;
		}
		imageIDs.insert(std::pair<long, long>(BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM, tempImageID));
		buffersDimensions.insert(std::pair<long, Dimensions>(BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM, ddflimArrivalTimeSum));

		double dwelltime = 0;
		pCamera->GetParam(ICamera::PARAM_LSM_DWELL_TIME, dwelltime);

		long maxPhotonsSize = static_cast<long>(round(baseDimensions.x * baseDimensions.y * (1 + 30 * dwelltime)));
		Dimensions ddflimPhotons = baseDimensions;
		ddflimPhotons.x = maxPhotonsSize;
		ddflimPhotons.y = 1;
		ddflimPhotons.imageBufferType = BufferType::DFLIM_PHOTONS;
		ddflimPhotons.dType = DataType::INT_8BIT;
		if (ImageManager::getInstance()->CreateImage(tempImageID, ddflimPhotons, L"dFLIMphotons") == FALSE)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"RunSample Execute could not create dFLIMphotons  memory buffer");
			return FALSE;
		}
		imageIDs.insert(std::pair<long, long>(BufferType::DFLIM_PHOTONS, tempImageID));
		buffersDimensions.insert(std::pair<long, Dimensions>(BufferType::DFLIM_PHOTONS, ddflimPhotons));

	}

	pMemoryBuffer = ImageManager::getInstance()->GetImagePtr(imageIDs[BufferType::INTENSITY], 0, 0, 0, 0);
	if (imageMethod == 1) //if dflim capture
	{
		Dimensions dHisto = buffersDimensions[BufferType::DFLIM_HISTOGRAM];
		Dimensions dSinglePhoton = buffersDimensions[BufferType::DFLIM_IMAGE_SINGLE_PHOTON];
		Dimensions dArrivalTimeSum = buffersDimensions[BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM];
		Dimensions dPhotons = buffersDimensions[BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM];

		pMemoryBufferDFLIMHisto = ImageManager::getInstance()->GetImagePtr(imageIDs[BufferType::DFLIM_HISTOGRAM], 0, 0, 0, 0);
		memset(pMemoryBufferDFLIMHisto, 0, dHisto.x * dHisto.y * dHisto.c * sizeof(uint32));

		pMemoryBufferDFLIMSinglePhoton = ImageManager::getInstance()->GetImagePtr(imageIDs[BufferType::DFLIM_IMAGE_SINGLE_PHOTON], 0, 0, 0, 0);
		memset(pMemoryBufferDFLIMSinglePhoton, 0, dSinglePhoton.x * dSinglePhoton.y * dHisto.c * sizeof(unsigned short));

		pMemoryBufferDFLIMArrivalTimeSum = ImageManager::getInstance()->GetImagePtr(imageIDs[BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM], 0, 0, 0, 0);
		memset(pMemoryBufferDFLIMArrivalTimeSum, 0, dArrivalTimeSum.x * dArrivalTimeSum.y * dHisto.c * sizeof(uint32));

		pMemoryBufferDFLIMPhotons = ImageManager::getInstance()->GetImagePtr(imageIDs[BufferType::DFLIM_PHOTONS], 0, 0, 0, 0);
		memset(pMemoryBufferDFLIMPhotons, 0, dPhotons.x * dPhotons.y * dHisto.c * sizeof(char));
	}

	const int MAX_TIME_LENGTH = 30;
	long wavelengthIndex = 0, zstageSteps, timePoints, triggerModeTimelapse, zEnable;
	string wavelengthName, zstageName;
	double exposureTimeMS, zstageStepSize, zStartPos, intervalSec;
	long zStreamFrames, zStreamMode;
	_pExp->GetWavelength(wavelengthIndex, wavelengthName, exposureTimeMS);
	_pExp->GetZStage(zstageName, zEnable, zstageSteps, zstageStepSize, zStartPos, zStreamFrames, zStreamMode);
	_pExp->GetTimelapse(timePoints, intervalSec, triggerModeTimelapse);
	if (0 == zEnable) zstageSteps = 1;	//If Z is not enable, set the step number to 0 for FIJI to read it
	string strOME;
	//build the ome image guid list on the first acquisition only
	if ((_zFrame == 1) && (_tFrame == 1))
	{
		strOME = uUIDSetup(pHardware, timePoints, zstageSteps, index, subWell);
	}

	Dimensions dAvg;

	dAvg.c = baseDimensions.c;
	dAvg.dType = baseDimensions.dType;
	dAvg.m = baseDimensions.m;
	dAvg.mType = CONTIGUOUS_CHANNEL_MEM_MAP;
	dAvg.t = avgFrames;
	dAvg.x = baseDimensions.x;
	dAvg.y = baseDimensions.y;
	dAvg.z = baseDimensions.z;


	long tempAvgImageID;

	map<long, long> avgImageIDs;
	map<long, Dimensions> avgBuffersDimensions;

	Dimensions dIntensityAvg = dAvg;
	dIntensityAvg.imageBufferType = BufferType::INTENSITY;
	if (ImageManager::getInstance()->CreateImage(tempAvgImageID, dIntensityAvg, L"intensity") == FALSE)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"RunSample Execute could not create intensity  memory buffer");
		return FALSE;
	}
	avgImageIDs.insert(std::pair<long, long>(BufferType::INTENSITY, tempAvgImageID));
	avgBuffersDimensions.insert(std::pair<long, Dimensions>(BufferType::INTENSITY, dIntensityAvg));

	//if DFLIM
	if (imageMethod == 1)
	{
		const long DFLIM_HISTOGRAM_BINS = 256;
		Dimensions ddflimHisto = dAvg;
		ddflimHisto.imageBufferType = BufferType::DFLIM_HISTOGRAM;
		ddflimHisto.x = DFLIM_HISTOGRAM_BINS;
		ddflimHisto.y = 1;
		ddflimHisto.dType = DataType::INT_32BIT;
		if (ImageManager::getInstance()->CreateImage(tempAvgImageID, ddflimHisto, L"dFLIMShisto") == FALSE)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"RunSample Execute could not create dflim histogram  memory buffer");
			return FALSE;
		}
		avgImageIDs.insert(std::pair<long, long>(BufferType::DFLIM_HISTOGRAM, tempAvgImageID));
		avgBuffersDimensions.insert(std::pair<long, Dimensions>(BufferType::DFLIM_HISTOGRAM, ddflimHisto));

		Dimensions ddflimSinglePhoton = dAvg;
		ddflimSinglePhoton.imageBufferType = BufferType::DFLIM_IMAGE_SINGLE_PHOTON;
		ddflimSinglePhoton.dType = DataType::INT_16BIT;
		if (ImageManager::getInstance()->CreateImage(tempAvgImageID, ddflimSinglePhoton, L"dFLIMSinglePhoton") == FALSE)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"RunSample Execute could not create dFLIMSinglePhoton  memory buffer");
			return FALSE;
		}
		avgImageIDs.insert(std::pair<long, long>(BufferType::DFLIM_IMAGE_SINGLE_PHOTON, tempAvgImageID));
		avgBuffersDimensions.insert(std::pair<long, Dimensions>(BufferType::DFLIM_IMAGE_SINGLE_PHOTON, ddflimSinglePhoton));

		Dimensions ddflimArrivalTimeSum = dAvg;
		ddflimArrivalTimeSum.imageBufferType = BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM;
		ddflimArrivalTimeSum.dType = DataType::INT_32BIT;
		if (ImageManager::getInstance()->CreateImage(tempAvgImageID, ddflimArrivalTimeSum, L"dFLIMArrivalTimeSum") == FALSE)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"RunSample Execute could not create dFLIMArrivalTimeSum  memory buffer");
			return FALSE;
		}
		avgImageIDs.insert(std::pair<long, long>(BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM, tempAvgImageID));
		avgBuffersDimensions.insert(std::pair<long, Dimensions>(BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM, ddflimArrivalTimeSum));

		double dwelltime = 0;
		pCamera->GetParam(ICamera::PARAM_LSM_DWELL_TIME, dwelltime);

		long maxPhotonsSize = static_cast<long>(round(baseDimensions.x * baseDimensions.y * (1 + 30 * dwelltime)));

		Dimensions ddflimPhotons = dAvg;
		ddflimPhotons.x = maxPhotonsSize;
		ddflimPhotons.y = 1;
		ddflimPhotons.imageBufferType = BufferType::DFLIM_PHOTONS;
		ddflimPhotons.dType = DataType::INT_8BIT;
		if (ImageManager::getInstance()->CreateImage(tempAvgImageID, ddflimPhotons, L"dFLIMphotons") == FALSE)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"RunSample Execute could not create dFLIMphotons  memory buffer");
			return FALSE;
		}
		avgImageIDs.insert(std::pair<long, long>(BufferType::DFLIM_PHOTONS, tempAvgImageID));
		avgBuffersDimensions.insert(std::pair<long, Dimensions>(BufferType::DFLIM_PHOTONS, ddflimPhotons));
	}



	long maxFrameWaitTime = 30000;
	double typeVal;

	pCamera->GetParam(ICamera::PARAM_CAMERA_TYPE, typeVal);

	long cameraType = static_cast<long>(typeVal);
	avgMode = ICamera::AVG_MODE_NONE;

	long currentTriggerMode;
	double dVal;
	pCamera->GetParam(ICamera::PARAM_TRIGGER_MODE, dVal);

	currentTriggerMode = static_cast<long>(dVal);

	pCamera->SetParam(ICamera::PARAM_LSM_FORCE_SETTINGS_UPDATE, 1);
	pCamera->SetParam(ICamera::PARAM_MULTI_FRAME_COUNT, avgFrames);

	switch (cameraType)
	{
	case ICamera::CCD:
	{
		switch (currentTriggerMode)
		{
		case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
		case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
		case ICamera::HW_SINGLE_FRAME:
		{
			maxFrameWaitTime = INFINITE;
		}
		break;
		default:
		{
			const long ADDITIONAL_TRANSFER_TIME_MS = 500;
			maxFrameWaitTime = static_cast<long>(exposureTimeMS + ADDITIONAL_TRANSFER_TIME_MS);
			pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::SW_MULTI_FRAME);
		}
		break;
		}
	}
	break;
	case ICamera::LSM:
	{
		switch (currentTriggerMode)
		{
		case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
		case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
		case ICamera::HW_SINGLE_FRAME:
		{
			pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::HW_MULTI_FRAME_TRIGGER_FIRST);
		}
		break;
		default:
		{
			pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::SW_MULTI_FRAME);
		}
		break;
		}
	}
	break;
	}

	double deltaT = 0;
	string acquiredDateTime;
	FrameInfo frameInfo = { 0, 0, 0, 0 };

	pCamera->SetupAcquisition(pMemoryBuffer);

	long camStarted = pCamera->StartAcquisition(pMemoryBuffer);
	//HW timed out, or failed to start:
	if (FALSE == camStarted)
	{
		pCamera->PostflightAcquisition(NULL);

		std::map<long, long>::iterator it = imageIDs.begin();
		while (it != imageIDs.end())
		{
			long imageID = it->second;
			ImageManager::getInstance()->UnlockImagePtr(imageID, 0, 0, 0, 0);
			ImageManager::getInstance()->DestroyImage(imageID);
			it++;
		}

		it = avgImageIDs.begin();
		while (it != avgImageIDs.end())
		{
			long imageID = it->second;
			ImageManager::getInstance()->DestroyImage(imageID);
			it++;
		}

		return FALSE;
	}

	unsigned short* pAvg;
	uint32* pDFLIMHistoAvg;
	unsigned short* pDFLIMSinglePhotonAvg;
	uint32* pDFLIMArrivalTimeSumAvg;
	char* pDFLIMPhotonsAvg;

	pAvg = (unsigned short*)ImageManager::getInstance()->GetImagePtr(avgImageIDs[BufferType::INTENSITY], 0, 0, 0, 0);
	if (imageMethod == 1)
	{
		pDFLIMHistoAvg = (uint32*)ImageManager::getInstance()->GetImagePtr(avgImageIDs[BufferType::DFLIM_HISTOGRAM], 0, 0, 0, 0);
		pDFLIMSinglePhotonAvg = (unsigned short*)ImageManager::getInstance()->GetImagePtr(avgImageIDs[BufferType::DFLIM_IMAGE_SINGLE_PHOTON], 0, 0, 0, 0);
		pDFLIMArrivalTimeSumAvg = (uint32*)ImageManager::getInstance()->GetImagePtr(avgImageIDs[BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM], 0, 0, 0, 0);
		pDFLIMPhotonsAvg = ImageManager::getInstance()->GetImagePtr(avgImageIDs[BufferType::DFLIM_PHOTONS], 0, 0, 0, 0);
	}

	UINT64 dflimPhotonListOffset = 0;

	for (long i = 0; i < avgFrames; i++)
	{
		//hEvent = CreateEvent(0, FALSE, FALSE, 0);

		if (i == 0)
		{
			// TO DO: 
			// and (index == 1) condition when it is supported to capture multiple sample locations in a later release
			if ((subWell == 1) && (_zFrame == 1) && (_tFrame == 1))
			{
				// set current as the start time of the experiment
				_acquireSaveInfo->getInstance()->SetExperimentStartCount();
			}
			else
			{
				deltaT = _acquireSaveInfo->getInstance()->AddTimingInfo();
			}
			acquiredDateTime = _acquireSaveInfo->getInstance()->AddTimestamp();

		}

		//DWORD dwThreadId;
		//hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, pCamera, 0, &dwThreadId );
		//		
		//long stopStatus = 0;

		//dwWait = WAIT_FAILED;

		//while(dwWait != WAIT_OBJECT_0)
		//{
		//	const long CHECK_STATUS_WAIT_TIME_MS = 10;
		//	//check to see if the image is ready
		//	dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, CHECK_STATUS_WAIT_TIME_MS );

		//	if(dwWait != WAIT_OBJECT_0)
		//	{
		//		StopCaptureEventCheck(stopStatus);
		//		//user has asked to stop the capture
		//		if(1 == stopStatus)
		//		break;
		//	}
		//}

		//CloseHandle(hThread);
		//CloseHandle(hEvent);

		long stopStatus = 0;
		long status = ICamera::STATUS_BUSY;

		while ((ICamera::STATUS_BUSY == status) || (ICamera::STATUS_PARTIAL == status))
		{
			if (FALSE == pCamera->StatusAcquisition(status))
			{
				break;
			}

			//if the capture is still busy check the stop status
			if (ICamera::STATUS_BUSY == status)
			{
				StopCaptureEventCheck(stopStatus);

				//user has asked to stop the capture
				if (1 == stopStatus)
					break;
			}
		}


		if (1 == stopStatus)
		{
			std::map<long, long>::iterator it = avgImageIDs.begin();
			while (it != avgImageIDs.end())
			{
				long imageID = it->second; // Accessing VALUE from element pointed by it.
				ImageManager::getInstance()->UnlockImagePtr(imageID, 0, 0, 0, i);
				ImageManager::getInstance()->DestroyImage(imageID);
				it++;// Increment the Iterator to point to next entry
			}

			return FALSE;
		}


		frameInfo.bufferType = BufferType::INTENSITY;
		pCamera->CopyAcquisition((char*)pAvg, &frameInfo);
		//if DFLIM
		if (imageMethod == 1)
		{
			frameInfo.bufferType = BufferType::DFLIM_HISTOGRAM;
			pCamera->CopyAcquisition((char*)pDFLIMHistoAvg, &frameInfo);
			frameInfo.bufferType = BufferType::DFLIM_IMAGE_SINGLE_PHOTON;
			pCamera->CopyAcquisition((char*)pDFLIMSinglePhotonAvg, &frameInfo);
			frameInfo.bufferType = BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM;
			pCamera->CopyAcquisition((char*)pDFLIMArrivalTimeSumAvg, &frameInfo);
			frameInfo.bufferType = BufferType::DFLIM_PHOTONS;
			pCamera->CopyAcquisition((char*)pDFLIMPhotonsAvg, &frameInfo);
			dflimPhotonListOffset += frameInfo.copySize;
		}

		std::map<long, long>::iterator it = avgImageIDs.begin();
		while (it != avgImageIDs.end())
		{
			long imageID = it->second; // Accessing VALUE from element pointed by it.
			ImageManager::getInstance()->UnlockImagePtr(imageID, 0, 0, 0, i);
			it++;// Increment the Iterator to point to next entry
		}

		if (i < (avgFrames - 1))
		{
			pAvg = (unsigned short*)ImageManager::getInstance()->GetImagePtr(avgImageIDs[BufferType::INTENSITY], 0, 0, 0, i + 1);
			//if DFLIM
			if (imageMethod == 1)
			{
				pDFLIMHistoAvg = (uint32*)ImageManager::getInstance()->GetImagePtr(avgImageIDs[BufferType::DFLIM_HISTOGRAM], 0, 0, 0, i + 1);
				pDFLIMSinglePhotonAvg = (unsigned short*)ImageManager::getInstance()->GetImagePtr(avgImageIDs[BufferType::DFLIM_IMAGE_SINGLE_PHOTON], 0, 0, 0, i + 1);
				pDFLIMArrivalTimeSumAvg = (uint32*)ImageManager::getInstance()->GetImagePtr(avgImageIDs[BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM], 0, 0, 0, i + 1);
				pDFLIMPhotonsAvg = ImageManager::getInstance()->GetImagePtr(avgImageIDs[BufferType::DFLIM_PHOTONS], 0, 0, 0, i + 1, dflimPhotonListOffset);
			}
		}
	}

	//The camera needs to be stopped before it can be started again
	if (ICamera::CCD == cameraType)
	{
		pCamera->PostflightAcquisition(NULL);
	}
	clock_t start = std::clock();
	//reset the trigger mode in the event it was changed for averaging
	pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, currentTriggerMode);


	Dimensions dSum = buffersDimensions[BufferType::INTENSITY];

	long* pSumBuffer = new long[dSum.x * dSum.y * dSum.c];

	memset(pSumBuffer, 0, sizeof(long) * dSum.x * dSum.y * dSum.c);

	for (long i = 0; i < avgFrames; i++)
	{
		long* pSum = pSumBuffer;

		unsigned short* pAvgBuffer = (unsigned short*)ImageManager::getInstance()->GetImagePtr(avgImageIDs[BufferType::INTENSITY], 0, 0, 0, i);

		for (long k = 0; k < dSum.x * dSum.y * dSum.c; k++)
		{
			*pSum += *pAvgBuffer;
			pSum++;
			pAvgBuffer++;
		}

		//if DFLIM
		if (imageMethod == 1)
		{
			uint32* pDFLIMHistoSum = (uint32*)pMemoryBufferDFLIMHisto;
			unsigned short* pDFLIMSinglePhotonSum = (unsigned short*)pMemoryBufferDFLIMSinglePhoton;
			uint32* pArrivalTimeSumSum = (uint32*)pMemoryBufferDFLIMArrivalTimeSum;

			uint32* pAvgDFLIMHistoBuffer = (uint32*)ImageManager::getInstance()->GetImagePtr(avgImageIDs[BufferType::DFLIM_HISTOGRAM], 0, 0, 0, i);
			unsigned short* pAvgDFLIMSinglePhotonBuffer = (unsigned short*)ImageManager::getInstance()->GetImagePtr(avgImageIDs[BufferType::DFLIM_IMAGE_SINGLE_PHOTON], 0, 0, 0, i);
			uint32* pAvgDFLIMArrivalTimeSumBuffer = (uint32*)ImageManager::getInstance()->GetImagePtr(avgImageIDs[BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM], 0, 0, 0, i);

			Dimensions dHistoSum = buffersDimensions[BufferType::DFLIM_HISTOGRAM];
			Dimensions dSinglePhotonSum = buffersDimensions[BufferType::DFLIM_IMAGE_SINGLE_PHOTON];
			Dimensions dArrivalTimeSumSum = buffersDimensions[BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM];

			for (long k = 0; k < dHistoSum.x * dHistoSum.y * dHistoSum.c; k++)
			{
				*pDFLIMHistoSum += *pAvgDFLIMHistoBuffer;
				pDFLIMHistoSum++;
				pAvgDFLIMHistoBuffer++;
			}

			for (long k = 0; k < dSinglePhotonSum.x * dSinglePhotonSum.y * dSinglePhotonSum.c; k++)
			{
				*pDFLIMSinglePhotonSum += *pAvgDFLIMSinglePhotonBuffer;
				pDFLIMSinglePhotonSum++;
				pAvgDFLIMSinglePhotonBuffer++;
			}

			for (long k = 0; k < dArrivalTimeSumSum.x * dArrivalTimeSumSum.y * dArrivalTimeSumSum.c; k++)
			{
				*pArrivalTimeSumSum += *pAvgDFLIMArrivalTimeSumBuffer;
				pArrivalTimeSumSum++;
				pAvgDFLIMArrivalTimeSumBuffer++;
			}
		}

		std::map<long, long>::iterator it = avgImageIDs.begin();
		while (it != avgImageIDs.end())
		{
			long imageID = it->second; // Accessing VALUE from element pointed by it.
			ImageManager::getInstance()->UnlockImagePtr(imageID, 0, 0, 0, i);
			it++;// Increment the Iterator to point to next entry
		}
	}

	unsigned short* pBuf = (unsigned short*)pMemoryBuffer;
	long* pSum = pSumBuffer;

	for (long k = 0; k < dSum.x * dSum.y * dSum.c; k++)
	{
		*pBuf = static_cast<unsigned short>((*pSum) / (double)avgFrames);
		pBuf++;
		pSum++;
	}

	delete[] pSumBuffer;

	//restore the average mode since it was switched to AVG_MODE_NONE
	pCamera->SetParam(ICamera::PARAM_LSM_AVERAGEMODE, avgMode);

	//re establish the existing triggermode
	pCamera->SetParam(ICamera::PARAM_TRIGGER_MODE, currentTriggerMode);

	ImageCorrections(_pExp, pMemoryBuffer, dSum.x, dSum.y, bufferChannels);

	//save the deltaT to timing.txt
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	wchar_t timingFile[_MAX_PATH];

	_wsplitpath_s(_path.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

	if ((_zFrame == zstageSteps) && (_tFrame == timePoints))
	{
		StringCbPrintfW(timingFile, _MAX_PATH, L"%s%stiming.txt", drive, dir);

		string timeFile = ConvertWStringToString(timingFile);
		//string timeFile = expPath + "\\timing.txt";
		_acquireSaveInfo->getInstance()->SaveTimingToFile(timeFile);
		_acquireSaveInfo->getInstance()->ClearTimingInfo();
		_acquireSaveInfo->getInstance()->ClearTimestamps();
	}

	long red;
	long green;
	long blue;
	long bp;
	long wp;

	GetColorInfo(pHardware.get(), wavelengthName, red, green, blue, bp, wp);

	const int COLOR_MAP_SIZE = 65536;
	unsigned short rlut[COLOR_MAP_SIZE];
	unsigned short glut[COLOR_MAP_SIZE];
	unsigned short blut[COLOR_MAP_SIZE];

	const int COLOR_MAP_BIT_DEPTH_TIFF = 8;

	GetLookUpTables(rlut, glut, blut, red, green, blue, bp, wp, COLOR_MAP_BIT_DEPTH_TIFF);

	wchar_t filePathAndName[_MAX_PATH];

	string redChan, greenChan, blueChan, cyanChan, magentaChan, yellowChan, grayChan;
	pHardware->GetColorChannels(redChan, greenChan, blueChan, cyanChan, magentaChan, yellowChan, grayChan);

	long bufferOffsetIndex = 0;

	PhysicalSize physicalSize;	// unit: um
	double res = floor(umPerPixel * 1000 + 0.5) / 1000;	// keep 2 figures after decimal point; res will be x.xx micron	
	physicalSize.x = res;
	physicalSize.y = res;
	physicalSize.z = zstageStepSize;

	long totalExperimentWavelengths = 0;
	if (TRUE == _sp.doOME)
	{
		long captureSequenceEnable = FALSE;
		_pExp->GetCaptureSequence(captureSequenceEnable);

		//Get the Capture Sequence Settings.
		//If it is a sequential capture, with more than one step the wavelenth
		//information for the steps in the sequence needs to be retrieved to
		//have the corrent information in the OME.
		vector<IExperiment::SequenceStep> captureSequence;
		_pExp->GetSequenceSteps(captureSequence);
		if (TRUE == captureSequenceEnable && captureSequence.size() > 1)
		{

			for (long i = 0; i < captureSequence.size(); i++)
			{
				totalExperimentWavelengths += static_cast<long>(captureSequence[i].Wavelength.size());
			}
		}
		else
		{
			totalExperimentWavelengths = _pExp->GetNumberOfWavelengths();
		}
	}
	long imgIndxDigiCnts = ResourceManager::getInstance()->GetSettingsParamLong((int)SettingsFileType::APPLICATION_SETTINGS, L"ImageNameFormat", L"indexDigitCounts", (int)Constants::DEFAULT_FILE_FORMAT_DIGITS);
	switch (imageMethod)
	{
	case 1: //dflim capture
	{
		Dimensions dIntensity = buffersDimensions[BufferType::INTENSITY];
		Dimensions dHisto = buffersDimensions[BufferType::DFLIM_HISTOGRAM];
		Dimensions dSinglePhoton = buffersDimensions[BufferType::DFLIM_IMAGE_SINGLE_PHOTON];
		Dimensions dArrivalTimeSum = buffersDimensions[BufferType::DFLIM_IMAGE_ARRIVAL_TIME_SUM];
		Dimensions dPhotonList = buffersDimensions[BufferType::DFLIM_PHOTONS];
		long numberOfchannels = _pExp->GetNumberOfWavelengths();
		long dflimImageSize = dHisto.x * dHisto.y * numberOfchannels * sizeof(uint32) +
			dIntensity.x * dIntensity.y * numberOfchannels * sizeof(unsigned short) +
			dSinglePhoton.x * dSinglePhoton.y * numberOfchannels * sizeof(unsigned short) +
			dArrivalTimeSum.x * dArrivalTimeSum.y * numberOfchannels * sizeof(uint32);

		char* dflimBuffer = new char[dflimImageSize];
		long copiedDFLIMSize = 0;
		for (long i = 0; i < bufferChannels; i++)
		{
			if (i < _pExp->GetNumberOfWavelengths())
			{
				_pExp->GetWavelength(i, wavelengthName, exposureTimeMS);

				//A single buffer is used for one channel capture so no offset is needed for that case
				if (bufferChannels > 1)
				{
					pHardware->GetWavelengthIndex(wavelengthName, bufferOffsetIndex);
				}

				memcpy(dflimBuffer + copiedDFLIMSize, pMemoryBufferDFLIMHisto + dHisto.x * dHisto.y * bufferOffsetIndex * sizeof(uint32), dHisto.x * dHisto.y * sizeof(uint32));
				copiedDFLIMSize += dHisto.x * dHisto.y * sizeof(uint32);
			}
		}

		for (long i = 0; i < bufferChannels; i++)
		{
			if (i < _pExp->GetNumberOfWavelengths())
			{
				_pExp->GetWavelength(i, wavelengthName, exposureTimeMS);

				//A single buffer is used for one channel capture so no offset is needed for that case
				if (bufferChannels > 1)
				{
					pHardware->GetWavelengthIndex(wavelengthName, bufferOffsetIndex);
				}
				memcpy(dflimBuffer + copiedDFLIMSize, pMemoryBuffer + dIntensity.x * dIntensity.y * bufferOffsetIndex * sizeof(unsigned short), dIntensity.x * dIntensity.y * sizeof(unsigned short));
				copiedDFLIMSize += dIntensity.x * dIntensity.y * sizeof(unsigned short);
			}
		}

		for (long i = 0; i < bufferChannels; i++)
		{
			if (i < _pExp->GetNumberOfWavelengths())
			{
				_pExp->GetWavelength(i, wavelengthName, exposureTimeMS);

				//A single buffer is used for one channel capture so no offset is needed for that case
				if (bufferChannels > 1)
				{
					pHardware->GetWavelengthIndex(wavelengthName, bufferOffsetIndex);
				}
				memcpy(dflimBuffer + copiedDFLIMSize, pMemoryBufferDFLIMSinglePhoton + dSinglePhoton.x * dSinglePhoton.y * bufferOffsetIndex * sizeof(unsigned short), dSinglePhoton.x * dSinglePhoton.y * sizeof(unsigned short));
				copiedDFLIMSize += dSinglePhoton.x * dSinglePhoton.y * sizeof(unsigned short);
			}
		}

		for (long i = 0; i < bufferChannels; i++)
		{
			if (i < _pExp->GetNumberOfWavelengths())
			{
				_pExp->GetWavelength(i, wavelengthName, exposureTimeMS);

				//A single buffer is used for one channel capture so no offset is needed for that case
				if (bufferChannels > 1)
				{
					pHardware->GetWavelengthIndex(wavelengthName, bufferOffsetIndex);
				}
				memcpy(dflimBuffer + copiedDFLIMSize, pMemoryBufferDFLIMArrivalTimeSum + dArrivalTimeSum.x * dArrivalTimeSum.y * bufferOffsetIndex * sizeof(uint32), dArrivalTimeSum.x * dArrivalTimeSum.y * sizeof(uint32));
				copiedDFLIMSize += dArrivalTimeSum.x * dArrivalTimeSum.y * sizeof(uint32);
			}
		}

		FrameInfo statsFrameInfo;
		statsFrameInfo.bufferType = BufferType::DFLIM_IMAGE;
		statsFrameInfo.imageWidth = dIntensity.x;
		statsFrameInfo.imageHeight = dIntensity.y;
		StatsManager::getInstance()->ComputeStats((unsigned short*)dflimBuffer,
			statsFrameInfo,
			_lsmChannel, FALSE, TRUE, TRUE);

		std::wstringstream dFLIMImgNameFormat;
		dFLIMImgNameFormat << L"%s%s\\Image_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
			<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
			<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
			<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.dFLIM";
		std::wstringstream photonsImgNameFormat;
		photonsImgNameFormat << L"%s%s\\Image_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
			<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
			<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
			<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.photons";

		std::wstring rawdFLIMPhotonsName = ImageManager::getInstance()->GetImagePath(avgImageIDs[BufferType::DFLIM_PHOTONS]);

		StringCbPrintfW(filePathAndName, _MAX_PATH, dFLIMImgNameFormat.str().c_str(), drive, dir, index, subWell, _zFrame, _tFrame);

		wchar_t filePathAndNamePhotons[_MAX_PATH];
		StringCbPrintfW(filePathAndNamePhotons, _MAX_PATH, photonsImgNameFormat.str().c_str(), drive, dir, index, subWell, _zFrame, _tFrame);

		ofstream dflimFile(filePathAndName, ios_base::binary);
		dflimFile.write(dflimBuffer, dflimImageSize);
		dflimFile.close();
		delete[] dflimBuffer;
		CallNotifySavedFileIPC(filePathAndName);

		long maxPhotonListSizePerFrame = dPhotonList.c * dPhotonList.x * dPhotonList.y;
		ofstream photonsFile(filePathAndNamePhotons, ios_base::binary);
		if (dflimPhotonListOffset > maxPhotonListSizePerFrame)
		{
			int n = static_cast<int>(floor((double)dflimPhotonListOffset / (double)maxPhotonListSizePerFrame));
			char* photonsBuffer;
			for (int i = 0; i < n; ++i)
			{
				photonsBuffer = ImageManager::getInstance()->GetImagePtr(avgImageIDs[BufferType::DFLIM_PHOTONS], 0, 0, 0, i, i * maxPhotonListSizePerFrame);
				photonsFile.write((char*)photonsBuffer, maxPhotonListSizePerFrame);
				ImageManager::getInstance()->UnlockImagePtr(avgImageIDs[BufferType::DFLIM_PHOTONS], 0, 0, 0, i);
				CallNotifySavedFileIPC(filePathAndNamePhotons);
			}

			int rem = dflimPhotonListOffset % maxPhotonListSizePerFrame;
			if (rem != 0)
			{
				photonsBuffer = ImageManager::getInstance()->GetImagePtr(avgImageIDs[BufferType::DFLIM_PHOTONS], 0, 0, 0, n, n * maxPhotonListSizePerFrame);
				photonsFile.write((char*)photonsBuffer, rem);
				ImageManager::getInstance()->UnlockImagePtr(avgImageIDs[BufferType::DFLIM_PHOTONS], 0, 0, 0, n);
				CallNotifySavedFileIPC(filePathAndNamePhotons);
			}
		}
		else
		{
			char* photonsBuffer = ImageManager::getInstance()->GetImagePtr(avgImageIDs[BufferType::DFLIM_PHOTONS], 0, 0, 0, 0);
			photonsFile.write((char*)photonsBuffer, dflimPhotonListOffset);
			ImageManager::getInstance()->UnlockImagePtr(avgImageIDs[BufferType::DFLIM_PHOTONS], 0, 0, 0, 0);
			CallNotifySavedFileIPC(filePathAndNamePhotons);
		}
		photonsFile.close();

		logDll->TLTraceEvent(INFORMATION_EVENT, 1, filePathAndName);
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, filePathAndNamePhotons);
	}
	break;
	default:
	{
		FrameInfo statsFrameInfo;
		statsFrameInfo.bufferType = BufferType::INTENSITY;
		statsFrameInfo.imageWidth = dSum.x;
		statsFrameInfo.imageHeight = dSum.y;
		StatsManager::getInstance()->ComputeStats((unsigned short*)pMemoryBuffer,
			statsFrameInfo,
			_lsmChannel, FALSE, TRUE, FALSE);
	}
	break;
	}
	long bitDepth = 14;
	switch (cameraType)
	{
	case ICamera::CCD:
	case ICamera::CCD_MOSAIC:
	{
		double bitsPerPixel = 12;
		pCamera->GetParam(ICamera::PARAM_BITS_PER_PIXEL, bitsPerPixel);
		bitDepth = static_cast<long>(bitsPerPixel);
	}
	break;
	case ICamera::LSM:
	{
		bitDepth = 14; //%TODO%  Retrieve the bitdepth from the camera
	}
	break;
	}

	Dimensions d = dSum;

	if (imageMethod == 1)
	{
		std::wstringstream rawImgNameFormat;
		rawImgNameFormat << L"%s%s\\Image_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
			<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
			<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
			<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.raw";

		StringCbPrintfW(filePathAndName, _MAX_PATH, rawImgNameFormat.str().c_str(), drive, dir, index, subWell, _zFrame, _tFrame);

		ofstream rawFile(filePathAndName, ios_base::binary);
		long numberOfchannels = _pExp->GetNumberOfWavelengths();

		long rawFrameSize = dIntensity.x * dIntensity.y * sizeof(unsigned short);
		for (long i = 0; i < bufferChannels; i++)
		{
			if (i < _pExp->GetNumberOfWavelengths())
			{
				_pExp->GetWavelength(i, wavelengthName, exposureTimeMS);
				//A single buffer is used for one channel capture so no offset is needed for that case
				if (bufferChannels > 1)
				{
					pHardware->GetWavelengthIndex(wavelengthName, bufferOffsetIndex);
				}

				rawFile.write(pMemoryBuffer + ((size_t)bufferOffsetIndex * d.x * d.y * sizeof(unsigned short)), rawFrameSize);
				CallNotifySavedFileIPC(filePathAndName);
			}
		}

		rawFile.close();
	}

	std::wstringstream imgNameFormat;
	imgNameFormat << L"%s%s%S_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.tif";
	std::wstringstream jpgNameFormat;
	jpgNameFormat << L"%s%sjpeg\\%S_%" << std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d_%"
		<< std::setw(2) << std::setfill(L'0') << imgIndxDigiCnts << L"d.jpg";

	for (long i = 0; i < bufferChannels; i++)
	{
		if (i < _pExp->GetNumberOfWavelengths())
		{
			_pExp->GetWavelength(i, wavelengthName, exposureTimeMS);

			//A single buffer is used for one channel capture so no offset is needed for that case
			if (bufferChannels > 1)
			{
				pHardware->GetWavelengthIndex(wavelengthName, bufferOffsetIndex);
			}

			GetColorInfo(pHardware.get(), wavelengthName, red, green, blue, bp, wp);
			GetLookUpTables(rlut, glut, blut, red, green, blue, bp, wp, COLOR_MAP_BIT_DEPTH_TIFF);

			StringCbPrintfW(filePathAndName, _MAX_PATH, imgNameFormat.str().c_str(), drive, dir, wavelengthName.c_str(), index, subWell, _zFrame, _tFrame);

			logDll->TLTraceEvent(INFORMATION_EVENT, 1, filePathAndName);

			string acquiredDate = acquiredDateTime;

			if (TRUE == _sp.doOME)
			{
				SaveTIFF(filePathAndName, pMemoryBuffer + (bufferOffsetIndex * d.x * d.y * 2), d.x, d.y, rlut, glut, blut,
					umPerPixel, totalExperimentWavelengths, timePoints, zstageSteps, intervalSec, i, _tFrame - 1, _zFrame - 1, &acquiredDate, deltaT,
					&strOME, physicalSize, _sp.doCompression);
			}
			else
			{
				SaveTIFFWithoutOME(filePathAndName, pMemoryBuffer + (bufferOffsetIndex * d.x * d.y * 2), d.x, d.y, rlut, glut, blut, umPerPixel, _pExp->GetNumberOfWavelengths(), timePoints, zstageSteps, intervalSec, i, _tFrame - 1, _zFrame - 1, &acquiredDate, deltaT, _sp.doCompression);
			}
			CallNotifySavedFileIPC(filePathAndName);

			if (_sp.doJPEG)
			{
				const int COLOR_MAP_BIT_DEPTH_JPEG = 16;

				GetLookUpTables(rlut, glut, blut, red, green, blue, bp, wp, COLOR_MAP_BIT_DEPTH_JPEG);

				StringCbPrintfW(filePathAndName, _MAX_PATH, jpgNameFormat.str().c_str(), drive, dir, wavelengthName.c_str(), index, subWell, _zFrame, _tFrame);

				SaveJPEG(filePathAndName, pMemoryBuffer + (bufferOffsetIndex * d.x * d.y * 2), d.x, d.y, rlut, glut, blut, bitDepth);
				CallNotifySavedFileIPC(filePathAndName);
			}
		}
	}


	std::map<long, long>::iterator it = avgImageIDs.begin();
	while (it != avgImageIDs.end())
	{
		long imageID = it->second;
		ImageManager::getInstance()->DestroyImage(imageID);
		it++;
	}

	it = imageIDs.begin();
	while (it != imageIDs.end())
	{
		long imageID = it->second;
		ImageManager::getInstance()->UnlockImagePtr(imageID, 0, 0, 0, 0);
		ImageManager::getInstance()->DestroyImage(imageID);
		it++;
	}


	double val;
	pCamera->GetParam(ICamera::PARAM_LSM_TYPE, val);
	ICamera::LSMType lsmType = (ICamera::LSMType)static_cast<long>(val);

	//When the camera is a Galvo-Galvo it needs some time to end the NI task
	//30ms has been determined to be the optimal time through testing
	if (ICamera::LSMType::GALVO_GALVO == lsmType)
	{
		double duration = (clock() - start) / (double)CLOCKS_PER_SEC * 1000; //ms
		long MIN_DURATION = 30; //ms
		double durationDiff = MIN_DURATION - duration;
		if (0 < durationDiff)
		{
			Sleep(static_cast<long>(durationDiff));
		}
		else
		{
			Sleep(1);
		}
	}
	return TRUE;
}