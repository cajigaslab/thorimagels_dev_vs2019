#include "ImageStoreWrapper.h"
#include "strsafe.h"
#include "..\..\..\Log.h"
#include "..\..\..\ThorSharedTypesCPP.h"
#include "..\..\..\PublicFuncs.h"
#include "..\..\..\StringCPP.h"
#include "..\..\..\BinaryImageDataUtilities\GenericImage.h"
#include "..\..\..\Camera.h"

///***		static members		***///

#ifdef LOGGING_ENABLED
std::auto_ptr<LogDll> logDll(new LogDll(L".\\ThorLoggingUnmanaged.dll"));
#endif

wchar_t message[_MAX_PATH];

//error message log
static void LogMessage(wchar_t *logMsg, long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

IExperiment* _exp;
std::map<uint16_t, Plate*> _plates;	//plate info for all wells, or well samples
std::map<uint8_t, Scan*> _scans;	//scan info for all scan sessions
Scan* _activeScan;					//current scan for saving tile
region* _activeRegion;				//current region for saving tile

ImageStoreWrapper::ImageStoreWrapper()
{
	_exp = NULL;
	_fileHandle = _cameraType = _imageID = 0;
	_captureFile = CaptureFile::FILE_TIFF;
	_activeScan = NULL;
	_activeRegion = NULL;
}

ImageStoreWrapper::~ImageStoreWrapper()
{
	_instanceFlag = false;
}

ImageStoreWrapper* ImageStoreWrapper::getInstance()
{
	if(! _instanceFlag)
	{
		_single.reset(new ImageStoreWrapper());
		_instanceFlag = true;
	}
	return _single.get();
}

bool ImageStoreWrapper::_instanceFlag = false;
std::unique_ptr<ImageStoreWrapper> ImageStoreWrapper::_single = NULL;

///***		Private Functions	***///

long ImageStoreWrapper::GetTimeInfo(double &timeIntervalSec)
{
	_captureFile = CaptureFile::FILE_TIFF;
	timeIntervalSec = 0;
	if(NULL == _exp)
		return FALSE;

	//determine time frames, don't continue if no time frame to be acquired
	long timeFrames = 0, captureMode = 0, triggerMode = 0;
	long enable=0, rawData=0, displayImage=0, storageMode=0, zFastEnable=0, zFastMode=0, flybackFrames=0, flybackLines=0, previewIndex=0, stimulusTriggering=0, dmaFrames=0, stimulusMaxFrames=0, useReferenceVoltageForPockels=0;
	double flybackTimeAdjustMS=0, volumeTimeAdjustMS=0, stepTimeAdjustMS=0;
	long displayCumulativeAveragePreview = FALSE;
	_exp->GetCaptureMode(captureMode);
	switch (CaptureModes(captureMode))
	{
	case T_AND_Z:
		_exp->GetTimelapse(timeFrames,timeIntervalSec,triggerMode);
		break;
	case STREAMING:
	case HYPERSPECTRAL:
		_exp->GetStreaming(enable, timeFrames, rawData, triggerMode, displayImage, storageMode, zFastEnable, zFastMode, flybackFrames, flybackLines, flybackTimeAdjustMS, volumeTimeAdjustMS, stepTimeAdjustMS, previewIndex, stimulusTriggering, dmaFrames, stimulusMaxFrames, useReferenceVoltageForPockels, displayCumulativeAveragePreview);
		_captureFile = CaptureFile(rawData);
		break;
	default:
		break;
	}
	return TRUE;
}

///***		Public Functions	***///

long ImageStoreWrapper::AddScan(double zStartPosUM, double zStopPosUM, double zStepSizeUM, long frameCount)
{
	try
	{
		//determine enabled channels, consider CaptureSequence
		vector<int> enabledChannel;
		long captureSequenceEnable = 0;
		if(_exp->GetCaptureSequence(captureSequenceEnable) && 1 == captureSequenceEnable)
		{
			vector<IExperiment::SequenceStep> captureSequence;
			_exp->GetSequenceSteps(captureSequence);
			for (int i = 0; i < static_cast<int>(captureSequence.size()); i++)
			{
				vector<int> tVec = ChannelManipulator<unsigned short>::getEnabledChannels(captureSequence[i].LSMChannel);
				enabledChannel.insert(std::end(enabledChannel), std::begin(tVec), std::end(tVec));
			}
		}
		if(0 >= enabledChannel.size())
		{
			//CaptureSequence enabled but no channel selections, continue as regular
			enabledChannel = ChannelManipulator<unsigned short>::getEnabledChannels(_exp->GetChannelEnableSet());
		}
		//return if not saving in big tiff or no channel enabled
		double timeIntervalSec = 0;
		GetTimeInfo(timeIntervalSec);
		if((CaptureFile::FILE_BIG_TIFF != _captureFile) || (0 >= enabledChannel.size()))
			return FALSE;

		//image ID only increment if add scan is successful
		//set channels
		string chName = "";
		double exposureTimeMS = 0;
		long captureMode = 0;
		_exp->GetCaptureMode(captureMode);	
		long viewMode = MesoScanTypes::Meso; 
		vector<ScanRegion> activeScanAreas;
		_exp->GetScanRegions(viewMode, activeScanAreas);
		//keep dummy if non template
		if (0 >= activeScanAreas.size())
		{
			viewMode = MesoScanTypes::Meso;
			ScanRegion sregion = {0};
			activeScanAreas.push_back(sregion);
		}

		//add scan to file
		Scan* scanInfo = new Scan();
		for (int i = 0; i < static_cast<int>(enabledChannel.size()); i++)
		{
			//no gap allowed in scan info channels, keep channel ID based on number of channels, channelID: 0-based
			//and use ref ID for actual enabled channel ID
			Channel* chInfo = new Channel();
			chInfo->ChannelID = static_cast<uint16_t>(i);
			chInfo->ChannelRefID = static_cast<uint16_t>(enabledChannel[i]);
			_exp->GetWavelength(i, chName, exposureTimeMS);
			strcpy_s(chInfo->Name, chName.c_str());
			scanInfo->Channels[chInfo->ChannelID] = chInfo;
		}
		//meso scan area size
		long scanAreaPixelX = 0, scanAreaPixelY = 0;
		double tileWidth = 0, tileHeight = 0;
		double pixelSizeUM = 1;
		_exp->GetImageArea(_cameraType, _lsmType, scanAreaPixelX, scanAreaPixelY, pixelSizeUM);
		_exp->GetScanAttribute(viewMode, "TileWidth", tileWidth);
		_exp->GetScanAttribute(viewMode, "TileHeight", tileHeight);

		scanInfo->PlateID = _plates[1]->PlateID;	//single plate only, plateID: 1-based

		//update well samples, and set scan regions
		for (const auto& wellit : _plates[1]->Wells)
		{			
			for (const auto& wellSampleit : wellit.second->WellSamples)
			{
				wellSampleit.second->ImageID = wellSampleit.second->ScanID = _imageID + 1;

				for (const auto& sampleRegionit : wellSampleit.second->SampleRegions)
				{
					sampleRegionit.second->PositionZ = zStartPosUM;
					sampleRegionit.second->SizeZ = abs(zStartPosUM - zStopPosUM);

					//set scan region
					region* rgnInfo = new region();
					rgnInfo->PlateID = wellSampleit.second->PlateID;
					uint16_t rID = rgnInfo->RegionID = sampleRegionit.second->RegionID; // (0 < wellSampleit.second->WellSampleID) ? wellSampleit.second->WellSampleID-1 : 0;	//regionID: 0-based
					rgnInfo->ScanID = wellSampleit.second->ScanID;
					rgnInfo->WellID = wellSampleit.second->WellID;
					rgnInfo->WellSampleID = sampleRegionit.second->WellSampleID; // wellSampleit.second->WellSampleID;

					auto scanArea = std::find_if(activeScanAreas.begin(), activeScanAreas.end(), [&rID](const ScanRegion& obj) { return obj.RegionID == rID;});
					if (scanArea == activeScanAreas.end()) throw new exception("ScanRegion");
					switch ((MesoScanTypes)viewMode)
					{
					case MesoScanTypes::Micro:
						rgnInfo->SizeX = scanArea->SizeX;
						rgnInfo->SizeY = scanArea->SizeY;
						break;
					case MesoScanTypes::Meso:
					default:
						rgnInfo->SizeX = (0 < scanAreaPixelX) ? scanAreaPixelX : 1;
						rgnInfo->SizeY = (0 < scanAreaPixelY) ? scanAreaPixelY : 1;
						break;
					}
					uint32 sizeZ = static_cast<uint32>(abs(zStartPosUM - zStopPosUM) / zStepSizeUM);
					rgnInfo->SizeZ = (0 < sizeZ) ? (sizeZ+1) : 1;															//zID: 1-based
					rgnInfo->SizeT = ((IExperiment::HYPERSPECTRAL != captureMode) && (0 < frameCount)) ? frameCount : 1;	//tID: 1-based
					rgnInfo->SizeS = ((IExperiment::HYPERSPECTRAL == captureMode) && (0 < frameCount)) ? frameCount : 1;	//sID: 1-based
					scanInfo->Regions[rgnInfo->RegionID] = _activeRegion = rgnInfo;
				}
			}
		}

		scanInfo->ScanID = _imageID + 1;	//scanID: 1-based
		scanInfo->SignificantBits = 14;
		scanInfo->TileWidth = static_cast<uint16_t>((0 < tileWidth) ? tileWidth : scanAreaPixelX);
		scanInfo->TileHeight = static_cast<uint16_t>((0 < tileHeight) ? tileHeight : scanAreaPixelY);
		scanInfo->TimeIncrement = static_cast<uint16_t>(timeIntervalSec);
		strcpy_s(scanInfo->TimeIncrementUnit, "s");
		scanInfo->Type = PixelType::PixelType_UINT16;
		strcpy_s(scanInfo->DimensionOrder, "XYCZT");
		scanInfo->PhysicalSizeX = scanInfo->PhysicalSizeY = pixelSizeUM;
		scanInfo->PhysicalSizeXUnit = scanInfo->PhysicalSizeYUnit = ResUnit::Micron;
		scanInfo->PhysicalSizeZ = (0 < zStepSizeUM) ? zStepSizeUM : 1;
		scanInfo->PhysicalSizeZUnit = ResUnit::Micron;
		GenerateScanXML(_fileHandle, _plates, scanInfo);
		_scans[scanInfo->ScanID] = _activeScan = scanInfo;
		_imageID++;

	}
	catch(...)
	{
		StringCbPrintfW(message,_MAX_PATH,L"ImageStoreWrapper unable to add scan: %d\n", _imageID+1);
		LogMessage(message,ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long ImageStoreWrapper::ClearImageStore()
{
	//all will be cleared by plate or scan at close
	if(0 < _fileHandle)
	{
		fnAISS_close_file(_fileHandle);
		_fileHandle = -1;
	}

	_activeScan = NULL;
	_activeRegion = NULL;
	_plates.clear();
	_scans.clear();
	return TRUE;
}

long ImageStoreWrapper::GetImageStoreInfo(long regionID, long &regionCount, long &width, long &height, long &channelCount, long &zMaxCount, long &timeCount, long &specCount)
{
	long ret = TRUE;
	regionCount = width = height = channelCount = zMaxCount = timeCount = specCount = 0;

	//return if no scan is available from load
	if(0 >= _scans.size())
		return FALSE;

	try
	{
		//timeCount: all time counts across all scans,
		//others are maximum values of scans
		for (const auto& scanIt : _scans)
		{
			Scan* curScan = scanIt.second;
			channelCount = (channelCount < static_cast<long>(curScan->Channels.size())) ? static_cast<long>(curScan->Channels.size()) : channelCount;
			regionCount = (regionCount < static_cast<long>(curScan->Regions.size())) ? static_cast<long>(curScan->Regions.size()) : regionCount;

			if(curScan->Regions.find(static_cast<uint16_t>(regionID)) != curScan->Regions.end())
			{
				width = (width < static_cast<long>(curScan->Regions[static_cast<uint16_t>(regionID)]->SizeX)) ? static_cast<long>(curScan->Regions[static_cast<uint16_t>(regionID)]->SizeX) : width;
				height = (height < static_cast<long>(curScan->Regions[static_cast<uint16_t>(regionID)]->SizeY)) ? static_cast<long>(curScan->Regions[static_cast<uint16_t>(regionID)]->SizeY) : height;
				zMaxCount = (zMaxCount < static_cast<long>(curScan->Regions[static_cast<uint16_t>(regionID)]->SizeZ)) ? static_cast<long>(curScan->Regions[static_cast<uint16_t>(regionID)]->SizeZ) : zMaxCount;
				timeCount += static_cast<long>(curScan->Regions[static_cast<uint16_t>(regionID)]->SizeT);
				specCount = (specCount < static_cast<long>(curScan->Regions[static_cast<uint16_t>(regionID)]->SizeS)) ? static_cast<long>(curScan->Regions[static_cast<uint16_t>(regionID)]->SizeS) : specCount;
				break;
			}
		}
	}
	catch(exception& e)
	{
		LogMessage((wchar_t*)StringToWString(string(e.what())).c_str(),ERROR_EVENT);
		ret = FALSE;
	}
	return ret;
}

long ImageStoreWrapper::LoadImageStore(char* fileWithPath)
{
	long ret = TRUE;
	char *plate_xml = NULL, *scans_xml = NULL;

	try
	{
		//reset file
		ClearImageStore();

		std::string filePath(fileWithPath);
		_fileHandle = fnAISS_open_file((char*)filePath.c_str(), READ_ONLY_MODE, false);
		if (0 >= _fileHandle)
		{
			StringCbPrintfW(message,_MAX_PATH,L"ImageStoreWrapper unable to create file: %s\n", filePath.c_str());
			throw exception();
		}

		//get plates information size
		uint32_t platesSize, scansSize;
		if ((SUCCESS != fnAISS_get_plates_info_size(_fileHandle, &platesSize)) || (SUCCESS != fnAISS_get_scans_info_size(_fileHandle, &scansSize)))
		{
			StringCbPrintfW(message,_MAX_PATH,L"ImageStoreWrapper unable to get info size: %s\n", filePath.c_str());
			throw exception();
		}

		//get plates information and reslove to structure
		plate_xml = (char*)malloc(platesSize);
		if ((SUCCESS != fnAISS_get_plates_info(_fileHandle, plate_xml, platesSize)) || (FALSE == ResolvePlatesXML(plate_xml, &_plates)))
		{
			StringCbPrintfW(message,_MAX_PATH,L"ImageStoreWrapper unable to get plates info: %s\n", filePath.c_str());
			throw exception();
		}

		//get scans information and reslove to structure
		scans_xml = (char*)malloc(scansSize);
		if ((SUCCESS != fnAISS_get_scans_info(_fileHandle, scans_xml, scansSize)) || (FALSE == ResolveScansXML(scans_xml, &_plates, &_scans)))
		{
			StringCbPrintfW(message,_MAX_PATH,L"ImageStoreWrapper unable to get scans info: %s\n", filePath.c_str());
			throw exception();
		}

	}
	catch(...)
	{
		ClearImageStore();
		LogMessage(message,ERROR_EVENT);
		ret = FALSE;
	}
	SAFE_DELETE_MEMORY(plate_xml);
	SAFE_DELETE_MEMORY(scans_xml);
	return ret;
}

long ImageStoreWrapper::ReadChannelData(char* buf, long channelCount, long width, long height, long zSliceID, long timeID, long specID, long regionID)
{
	long ret = TRUE;
	_activeScan = NULL;
	long tempT = 0;

	//return if no scan is available from load
	if(0 >= _scans.size())
		return FALSE;

	try
	{
		//timeID is expected to span across multiple scans, all index 0-based from user
		//use it to determine the target scan
		for (const auto& scanIt : _scans)
		{
			if(timeID <= tempT + static_cast<long>(scanIt.second->Regions[static_cast<uint16_t>(regionID)]->SizeT) - 1)
			{
				_activeScan = scanIt.second;
				timeID -= tempT;
				break;
			}
			else
			{
				tempT += static_cast<long>(scanIt.second->Regions[static_cast<uint16_t>(regionID)]->SizeT);
			}
		}

		if(NULL != _activeScan)
		{
			_activeRegion = NULL;
			for (const auto& regionit : _activeScan->Regions)
			{
				if(regionID == static_cast<long>(regionit.second->RegionID))
				{
					_activeRegion = regionit.second;
					break;
				}
			}

			//copy buffer, will provide 4 channels or 1 channel
			char* tgt = buf;
			long imageSize = width * height * static_cast<long>(ceil((double)_activeScan->SignificantBits / Constants::BITS_PER_BYTE));
			IplRect src_rect = {0, 0, static_cast<unsigned int>(width), static_cast<unsigned int>(height)};
			if(NULL != _activeRegion)
			{
				if(1 == channelCount)
				{
					frame_info frame = { _activeScan->ScanID, _activeRegion->RegionID, 0, static_cast<uint16_t>((zSliceID+1)), static_cast<uint16_t>(timeID+1), static_cast<uint16_t>(specID+1) };
					if(SUCCESS != fnAISS_get_raw_data(_fileHandle, frame, src_rect, tgt))
					{
						StringCbPrintfW(message,_MAX_PATH,L"ImageStoreWrapper unable to read %d z, %d t\n", zSliceID, timeID);
						LogMessage(message,ERROR_EVENT);
					}
				}
				else
				{
					for (long i = 0; i < channelCount; i++)
					{
						//locate channelID among available channels, 0-based, from A to D
						for (auto& ch : _activeScan->Channels)
						{
							if(i == static_cast<long>(ch.second->ChannelRefID))
							{
								frame_info frame = { _activeScan->ScanID, _activeRegion->RegionID, static_cast<uint16_t>(ch.second->ChannelID), static_cast<uint16_t>((zSliceID+1)), static_cast<uint16_t>(timeID+1), 1 };
								if(SUCCESS != fnAISS_get_raw_data(_fileHandle, frame, src_rect, tgt))
								{
									StringCbPrintfW(message,_MAX_PATH,L"ImageStoreWrapper unable to read %d channel, %d z, %d t\n", i, zSliceID, timeID);
									LogMessage(message,ERROR_EVENT);
								}
								break;
							}
						} 
						//offset for one channel since multiple channel buffer provided, 
						//and it is expected to be filled in channel order
						tgt += imageSize;
					}
				}
			}
		}
	}
	catch(exception& e)
	{
		LogMessage((wchar_t*)StringToWString(string(e.what())).c_str(),ERROR_EVENT);
		ret = FALSE;
	}
	return ret;
}

long ImageStoreWrapper::SetupImageStore(wchar_t * path, void* exp, long doCompression) 
{
	try
	{
		_exp = (IExperiment*)exp;
		if(NULL == _exp)
			return FALSE;

		string str = "";
		_cameraType = ICamera::CameraType::LAST_CAMERA_TYPE;
		_lsmType = ICamera::LSMType::LSMTYPE_LAST;
		_exp->GetModality(_cameraType, str, _lsmType);

		//reset file
		ClearImageStore();

		//return if not saving in big tiff
		double timeIntervalSec = 0;
		GetTimeInfo(timeIntervalSec);
		if(CaptureFile::FILE_BIG_TIFF != _captureFile)
			return FALSE;

		string name = "Slide";
		double width = 1, height = 1;
		long row = 1, column = 1;
		double diameter = 1, centerToCenterX = 1, centerToCenterY = 1, topLeftCenterOffsetX = 0, topLeftCenterOffsetY = 0, initialStageLocationX = 0, initialStageLocationY = 0; 
		string WellShape = "Rectangle";
		double WellWidth = 1, WellHeight = 1;
		long scanAreaPixelX = 1, scanAreaPixelY = 1;
		double pixelSizeUM = 1;
		vector<IExperiment::SubImage> SubImages; 
		long viewMode = MesoScanTypes::Meso; 
		vector<SampleRegion> activeScanAreas;

		std::string filePath = WStringToString(path) + ".tif";
		_fileHandle = fnAISS_open_file((char*)filePath.c_str(), CREATE_MODE, false);
		if (0 >= _fileHandle)
		{
			StringCbPrintfW(message,_MAX_PATH,L"ImageStoreWrapper unable to create file: %s\n", filePath.c_str());
			LogMessage(message,ERROR_EVENT);
			return FALSE;
		}
		//save image data in compression mode
		uint32 compression = (TRUE == doCompression) ? IMAGESTORE_COMPRESSION_LZW : IMAGESTORE_COMPRESSION_NONE;
		if (SUCCESS != fnAISS_set_field(_fileHandle, IMAGESTORETAG_COMPRESSION, compression))
		{
			StringCbPrintfW(message,_MAX_PATH,L"ImageStoreWrapper unable to set compression of file: %s\n", filePath.c_str());
			LogMessage(message,ERROR_EVENT);
			return FALSE;
		}

		_exp->GetSampleInfo(name, width, height, row, column, diameter, centerToCenterX, centerToCenterY, topLeftCenterOffsetX, topLeftCenterOffsetY, WellShape, WellWidth, WellHeight, initialStageLocationX, initialStageLocationY);
		_exp->GetSubImages(SubImages,_cameraType,_lsmType);
		_exp->GetImageArea(_cameraType, _lsmType, scanAreaPixelX, scanAreaPixelY, pixelSizeUM);
		_exp->GetSampleRegions(viewMode, activeScanAreas);
		//keep dummy for non template
		if (0 >= activeScanAreas.size())
		{
			viewMode = MesoScanTypes::Meso;
			SampleRegion sregion = {0};
			activeScanAreas.push_back(sregion);
		}
		//reset then build plates,
		//reset scans for user to add scan later
		_plates.clear();
		_scans.clear();
		_imageID = 0;

		//for now, one tile is one scanRegion, must have one tile available 
		//wellSample [size in mm] or ScanRegion [size in Pixel] == SubImage(or tile),
		//default imageID = 1 and imageID seems to be linked with scanID
		if(0 >= SubImages.size())
		{
			IExperiment::SubImage sImage;
			sImage.isEnable = true;
			sImage.overlapX = sImage.overlapY = 0;
			sImage.scanAreaWidth = round(scanAreaPixelX * pixelSizeUM)/Constants::UM_TO_MM;		//[mm]
			sImage.scanAreaHeight = round(scanAreaPixelY * pixelSizeUM)/Constants::UM_TO_MM;
			sImage.subRows = sImage.subColumns = 1;
			sImage.transOffsetXMM = initialStageLocationX;
			sImage.transOffsetYMM = initialStageLocationY;
			sImage.transOffsetZMM = 0;
			sImage.wellID = 1;
			SubImages.push_back(sImage);

			//force well as 1 if no sub image configured
			row = column = 1;
		}
		//no concept of plates, only wells
		//set single plate
		Plate* plateInfo = new Plate();
		plateInfo->PlateID = 1;
		plateInfo->Rows = plateInfo->Columns = 1;
		strcpy_s(plateInfo->Name, name.c_str());
		plateInfo->Width = width;
		plateInfo->Height = height;
		plateInfo->PhysicalSizeXUnit = plateInfo->PhysicalSizeYUnit = ResUnit::Millimetre;

		//must have one well available
		std::map<uint16_t, Well*> Wells;
		if((0 >= row) && (0 >= column))
		{
			row = column = 1;
		}
		for (int m = 0; m < row; m++)
		{
			for (int n = 0; n < column; n++)
			{
				int wID = m*column + n + 1;

				Well* wellInfo = new Well();
				wellInfo->WellID = wID;
				wellInfo->PlateID = 1;
				wellInfo->PositionX = topLeftCenterOffsetX + (n * centerToCenterX);
				wellInfo->PositionY = topLeftCenterOffsetY + (m * centerToCenterY);
				wellInfo->Width = WellWidth;
				wellInfo->Height = WellHeight;
				wellInfo->Row = m + 1;
				wellInfo->Column = n + 1;
				strcpy_s(wellInfo->Shape, WellShape.c_str());

				for (int i = 0; i < static_cast<int>(SubImages.size()); i++)
				{
					if(!SubImages[i].isEnable)
						continue;

					//set well sample id in zig-zag order, align with tile scan
					for (int k = 0; k < SubImages[i].subColumns; k++)
					{
						for (int j = 0; j < SubImages[i].subRows; j++)
						{
							int wSampleID = (0 == (j % 2)) ? (j*SubImages[i].subColumns + k + 1) : ((j+1)*SubImages[i].subColumns - k);	//well sample ID 1-based

							WellSample* wSampleInfo = new WellSample();
							wSampleInfo->PlateID = 1;
							wSampleInfo->WellID = SubImages[i].wellID;
							wSampleInfo->WellSampleID = wSampleID;
							wSampleInfo->ImageID = 1;	//*** need update at add scan ***//
							wSampleInfo->ScanID = 1;	//*** need update at add scan ***//

							for (int aid = 0; aid < activeScanAreas.size(); aid++)
							{
								SampleRegion* sampleRegion = new SampleRegion();
								sampleRegion->WellSampleID = wSampleInfo->WellSampleID;
								sampleRegion->RegionID = activeScanAreas[aid].RegionID;	//0-based, meso area ID:0, micro area ID:1,2,...
								switch ((MesoScanTypes)viewMode)
								{
								case MesoScanTypes::Micro:
									sampleRegion->SizeX = activeScanAreas[aid].SizeX;
									sampleRegion->SizeY = activeScanAreas[aid].SizeY;
									sampleRegion->PositionX = activeScanAreas[aid].PositionX;
									sampleRegion->PositionY = activeScanAreas[aid].PositionY;
									break;
								case MesoScanTypes::Meso:
								default:
									sampleRegion->SizeX = SubImages[i].scanAreaWidth;	//keep physical size in mm since no multiple tiles in one well sample
									sampleRegion->SizeY = SubImages[i].scanAreaHeight;
									sampleRegion->PositionX = SubImages[i].transOffsetXMM - (k - (k-1)*(SubImages[i].overlapX/Constants::HUNDRED_PERCENT)) * SubImages[i].scanAreaWidth/2;
									sampleRegion->PositionY = SubImages[i].transOffsetYMM + (j - (j-1)*(SubImages[i].overlapY/Constants::HUNDRED_PERCENT)) * SubImages[i].scanAreaHeight/2;
									break;
								}
								sampleRegion->SizeZ = 0;		//*** need update at add scan ***//
								sampleRegion->PositionZ = 0;	//*** need update at add scan ***//
								sampleRegion->PhysicalSizeXUnit = sampleRegion->PhysicalSizeYUnit = sampleRegion->PhysicalSizeZUnit = ResUnit::Millimetre;

								if (wSampleInfo->SampleRegions.find(sampleRegion->RegionID) == wSampleInfo->SampleRegions.end())
									wSampleInfo->SampleRegions.insert(make_pair(sampleRegion->RegionID, sampleRegion));
							}

							wellInfo->WellSamples[wSampleInfo->WellSampleID] = wSampleInfo;
						}
					}
				}
				plateInfo->Wells[wellInfo->WellID] = wellInfo;
			}
		}
		_plates[plateInfo->PlateID] = plateInfo;

		//add plate to file
		return GeneratePlatesXML(_fileHandle, _plates);
	}
	catch(...)
	{
		StringCbPrintfW(message,_MAX_PATH,L"ImageStoreWrapper unable to setup Image Store: %s\n", path);
		LogMessage(message,ERROR_EVENT);
		return FALSE;
	}
}

long ImageStoreWrapper::SetScan(long scanID)
{
	_activeScan = NULL;

	if(CaptureFile::FILE_BIG_TIFF != _captureFile)
		return FALSE;

	for (const auto& scanIt : _scans)
	{
		if(scanID == static_cast<long>(scanIt.second->ScanID))
		{
			_activeScan = scanIt.second;
			return TRUE;
		}
	}
	return FALSE;
}

long ImageStoreWrapper::SetRegion(long regionID)
{
	_activeRegion = NULL;

	if((CaptureFile::FILE_BIG_TIFF != _captureFile) ||(NULL == _activeScan))
		return FALSE;

	for (const auto& regionit : _activeScan->Regions)
	{
		if(regionID == static_cast<long>(regionit.second->RegionID))
		{
			_activeRegion = regionit.second;
			return TRUE;
		}
	}
	return FALSE;
}

long ImageStoreWrapper::SaveData(void* buf, uint16_t channelID, uint16_t z, uint16_t t, uint16_t s)
{
	if ((NULL == _activeScan) || (NULL == _activeRegion))
		return FALSE;

	frame_info frame = { _activeScan->ScanID, _activeRegion->RegionID, channelID, z, t, s };

	for (uint32_t y = 0; y < _activeRegion->SizeY; y += _activeScan->TileHeight)
	{
		for (uint32_t x = 0; x < _activeRegion->SizeX; x += _activeScan->TileWidth)
		{
			//save tile data
			//the stride is the width of a single row of pixels (a scan line)
			if (SUCCESS != fnAISS_save_tile_data(_fileHandle, buf, _activeScan->TileWidth, frame, y / _activeScan->TileHeight, x / _activeScan->TileWidth))
			{
				StringCbPrintfW(message,_MAX_PATH,L"ImageStoreWrapper unable to save at z %d t %d\n", z, t);
				LogMessage(message,ERROR_EVENT);
				return FALSE;
			}
			////fnAISS_generate_pyramidal_data can be called at the end of scan, take a while to finish; Keep raw for now,
			////since no large tiled frames to be rendered.
			////generate frame pyramid,  uint8_t* image buffer for scaled jpeg, has to be offseted from raw buffer
			//fnAISS_generate_pyramidal_data_frame(_fileHandle, frame, buf, _activeScan->TileWidth, _activeScan->TileHeight);
		}
	}
	return TRUE;
}
