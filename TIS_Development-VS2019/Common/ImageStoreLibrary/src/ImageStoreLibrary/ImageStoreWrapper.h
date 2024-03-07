#pragma once

#include <memory>
#include <map>
#include "storeconf.h"
#include "xml_handler.h"
#include "ImageStoreLibrary.h"
#include "..\..\..\Experiment.h"

#define LOGGING_ENABLED

//singleton wrapper class for ImageStoreLibrary, 
//provide entry point and deal with plates/scans. 
class ImageStoreWrapper
{
private:
	ImageStoreWrapper();

	static bool _instanceFlag;
	static std::unique_ptr<ImageStoreWrapper> _single;

	IExperiment* _exp;
	std::map<uint16_t, Plate*> _plates;	//plate info for all wells, or well samples
	std::map<uint32_t, Scan*> _scans;	//scan info for all scan sessions
	Scan* _activeScan;					//current scan for saving tile
	region* _activeRegion;				//current region for saving tile
	int _fileHandle;					//big tiff handle
	int _imageID;						//global image ID, start from 1 if able to add scan
	long _cameraType;					//camera type, either camera or LSM
	long _lsmType;						//lsm type, can be GR, GG, RGG, ...
	long _captureFile;					//file mode to be acquired, only save in FILE_BIG_TIFF(2)

	//get time frame counts and interval in second, frame count == -1 will bypass saving
	long GetTimeInfo(double &timeIntervalSec);	

public:
	~ImageStoreWrapper();
	static ImageStoreWrapper* getInstance();

	//add scan to tif, given z positions after tilt
	long AddScan(double zStartPosUM, double zStopPosUM, double zStepSizeUM, long frameCount);

	//close ome tiff
	long ClearImageStore();

	//get ome tiff info, zMaxCount: Maximum z size of all scans, timeCount: total time count from all scans
	long GetImageStoreInfo(long regionID, long &regionCount, long &width, long &height, long &channelCount, long &zMaxCount, long &timeCount, long &specCount);

	//load ome tiff
	long LoadImageStore(char* fileWithPath);

	//read one channel image buffer, default single well or tile (well sample) for streaming, timeID is expected to span across multiple scans
	//channelCount: either 1 (channelID) or 4 (all), chnnelID: 0:ChA, 1:ChB, 2:ChC, 3:ChD. all index 0-based from user
	long ReadChannelData(char* buf, long channelCount, long width, long height, long zSliceID, long timeID, long specID, long regionID = 0);

	//initialize ome tif, last image must be closed before this function
	long SetupImageStore(wchar_t * path, void* exp, long doCompression = FALSE);

	//set active scan and region id
	long SetScan(long scanID);
	long SetRegion(long regionID);

	//save buffer to ome tif at particular location
	long SaveData(void* buf, uint16_t channelID, uint32_t z, uint32_t t, uint32_t s = 1);

	//adjust scan frame count, especially when experiment stopped manually
	long AdjustScanTCount(int count);
};