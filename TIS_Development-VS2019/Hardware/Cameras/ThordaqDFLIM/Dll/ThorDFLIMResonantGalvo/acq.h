#ifndef CONST_H
#include "const.h"
#endif

#ifndef ACQ_H
#define ACQ_H
#include "stdafx.h"

class PockelPty
{
	public:
		double pockelsPowerLevel[MAX_POCKELS_CELL_COUNT];///<level of pockels power		
		double pockelsLineBlankingPercentage[MAX_POCKELS_CELL_COUNT];///<percentage of line to keep pockels at power
		double pockelsMinVoltage[MAX_POCKELS_CELL_COUNT];///<array of minimums for the pockels minimum output 
		double pockelsMaxVoltage[MAX_POCKELS_CELL_COUNT];///<array of maximums for the pockels maximum output 
		long   pockelsMaskEnable[MAX_POCKELS_CELL_COUNT];///<use pockels mask(s)
		long   pockelsMaskInvert[MAX_POCKELS_CELL_COUNT];///<invert pockels mask(s)

		PockelPty()
		{
			memset(this->pockelsPowerLevel,0,MAX_POCKELS_CELL_COUNT*sizeof(double));	
			memset(this->pockelsLineBlankingPercentage,0,MAX_POCKELS_CELL_COUNT*sizeof(double));	
			memset(this->pockelsMinVoltage,0,MAX_POCKELS_CELL_COUNT*sizeof(double));	
			memset(this->pockelsMaxVoltage,0,MAX_POCKELS_CELL_COUNT*sizeof(double));
			memset(this->pockelsMaskEnable,0,MAX_POCKELS_CELL_COUNT*sizeof(long));	
			memset(this->pockelsMaskInvert,0,MAX_POCKELS_CELL_COUNT*sizeof(long));
		}

		bool IsEqual(PockelPty &iPockelPty)
		{
			return(!(
			(memcmp(&iPockelPty.pockelsLineBlankingPercentage,&this->pockelsLineBlankingPercentage,sizeof(this->pockelsLineBlankingPercentage))!=0)||
			(memcmp(&iPockelPty.pockelsPowerLevel,&this->pockelsPowerLevel,sizeof(this->pockelsPowerLevel))!=0)||
			(memcmp(&iPockelPty.pockelsMinVoltage,&this->pockelsMinVoltage,sizeof(this->pockelsMinVoltage))!=0)||
			(memcmp(&iPockelPty.pockelsMaxVoltage,&this->pockelsMaxVoltage,sizeof(this->pockelsMaxVoltage))!=0)||
			(memcmp(&iPockelPty.pockelsMaskEnable,&this->pockelsMaskEnable,sizeof(this->pockelsMaskEnable))!=0)||
			(memcmp(&iPockelPty.pockelsMaskInvert,&this->pockelsMaskInvert,sizeof(this->pockelsMaskInvert))!=0)
			));
		}

		PockelPty& operator = (const PockelPty &iPockelPty)
		{
			memcpy(this->pockelsPowerLevel,iPockelPty.pockelsPowerLevel,MAX_POCKELS_CELL_COUNT*sizeof(double));	
			memcpy(this->pockelsLineBlankingPercentage,iPockelPty.pockelsLineBlankingPercentage,MAX_POCKELS_CELL_COUNT*sizeof(double));	
			memcpy(this->pockelsMinVoltage,iPockelPty.pockelsMinVoltage,MAX_POCKELS_CELL_COUNT*sizeof(double));	
			memcpy(this->pockelsMaxVoltage,iPockelPty.pockelsMaxVoltage,MAX_POCKELS_CELL_COUNT*sizeof(double));	
			memcpy(this->pockelsMaskEnable,iPockelPty.pockelsMaskEnable,MAX_POCKELS_CELL_COUNT*sizeof(long));	
			memcpy(this->pockelsMaskInvert,iPockelPty.pockelsMaskInvert,MAX_POCKELS_CELL_COUNT*sizeof(long));	
			return *this;
		}
};


class ImgAcqPty
{
public:
	long   pixelX; ///<Image pixel number in the x direction
	long   pixelY; ///<Image pixel number in the y direction
	long   fieldSize; ///<A parameter scales to scan field size, the actual scan sized at the object plane may vary from device to device
	long   offsetX; ///<Off set of scan field size in the x direction
	long   offsetY; ///<Off set of scan field size in the y direction
	long   channel; ///<Bitwise selection of channels.
	long   alignmentForField; ///<Forward Backward scan alignment
	long   inputRangeChannel1; ///<The digitizer input channel measurement range, see the enumeration of "inputrange"
	long   inputRangeChannel2;
	long   inputRangeChannel3;
	long   inputRangeChannel4;
	long   clockSource; ///<0 to use internal sample clock source, 1 to use external clock source
	long   clockRateInternal; ///< internal sample clock rate, the enumeration of "ClockRate"
	long   clockRateExternal; ///< external sample clock rate, this the actual sample rate
	long   scanMode; ///< 2 way, 1 way forward or 1 way backward scan mode, see enumeration of ScanMode
	long   averageMode; ///< average mode, see enumeration of AverageMode;
	long   averageNum;///< number of frame, lines to average
	long   triggerMode; ///<trigger source of each frame
	unsigned long long  numFrame; ///<number of frame to acquire for a experiment
	double yAmplitudeScaler;///<pixel aspect ratio scaler
	long   areaMode;///<scan pattern mode square,rect,line
	long   flybackCycle;///<number of line cycles to wait when galvo is flying back to start of the frame
	long   dataMapMode;///<mapping of raw digital values into output buffer
	double dwellTime; ///<number of lines per second for the galvo-galvo raster scan
	long   progressCounter;///<stores the completed number of frames. Purpose is to force the SetupAcquisition to execute all hardware settings
	long   galvoEnable;///<enable or disable y galvo movement
	long   yChannelEnable;///<enable or disable the communication with y channel, NOT implemented yet.
	double verticalScanDirection;///<direction of the y galvo scan 1:increasing scan  -1: decreasing scan 
	double fineOffsetX;///<fine adjustment of offset X
	double fineOffsetY;///<fine adjustment of offset Y
	double fineFieldSizeScaleX;///<fine adjustment of field size X
	double fineFieldSizeScaleY;///<fine adjustment of field size Y
	double scanAreaAngle;///<angle for roi
	long   rawSaveEnabledChannelOnly; ///<copy only the enabled channels continuously into the buffer
	long   horizontalFlip;///<flip the image in the X direction
	double DCOffset[2];
	long resetFlybackEnable;
	PockelPty pockelPty;
	long realTimeDataAverage;
	long captureWithoutLineTrigger;
	long twoWayZones[NUM_TWOWAY_ZONES];///<digital pot zones for correcting two way alignment
	long twoWayZonesFine[NUM_TWOWAY_ZONES];///<digitizer pixel zones for correcting the two way alignment
	long laserCoherentSamplingEnable;
	long laserCoherentSamplingPhase;
	long channelPolarity[MAX_CHANNEL_COUNT];///<store the polarity to use for each channel
	long frameTriggerEnableWithHWTrig;
	long threePhotonModeEnable;
	ImgAcqPty()
	{
		this->pixelX = DEFAULT_PIXEL_X;
		this->pixelY = DEFAULT_PIXEL_Y;
		this->fieldSize = DEFAULT_FIELD_SIZE_X;
		this->offsetX = 0; 
		this->offsetY = 0; 
		this->channel = MIN_CHANNEL; 
		this->alignmentForField = DEFAULT_ALIGNMENT; 
		this->inputRangeChannel1 = DEFAULT_INPUTRANGE; 
		this->inputRangeChannel2 = DEFAULT_INPUTRANGE;
		this->inputRangeChannel3 = DEFAULT_INPUTRANGE;
		this->inputRangeChannel4 = DEFAULT_INPUTRANGE;
		this->clockSource = 1; //using internal  
		this->clockRateInternal = DEFAULT_INTERNALCLOCKRATE; 
		this->clockRateExternal = DEFAULT_EXTCLOCKRATE; 
		this->scanMode = DEFAULT_SCANMODE;  
		this->averageMode = NO_AVERAGE; 
		this->averageNum= 2;
		this->triggerMode = DEFAULT_TRIGGER_MODE; 
		this->numFrame = 1; 
		this->yAmplitudeScaler = DEFAULT_Y_AMPLITUDE_SCALER;
		this->areaMode = ICamera::SQUARE;
		this->flybackCycle = DEFAULT_FLYBACK_CYCLE;
		this->dataMapMode = ICamera::POLARITY_MIXED;
		this->dwellTime = DEFAULT_DWELL_TIME; 
		this->progressCounter = 0;
		this->galvoEnable = TRUE;
		this->yChannelEnable = TRUE;
		this->verticalScanDirection = 1;
		this->scanAreaAngle = DEFAULT_SCANAREA_ANGLE;
		this->rawSaveEnabledChannelOnly = FALSE; 
		this->horizontalFlip = FALSE;
		memset(this->DCOffset,0,2*sizeof(double));
		this->pockelPty = PockelPty();
		this->resetFlybackEnable = FALSE;
		this->realTimeDataAverage = FALSE;
		this->captureWithoutLineTrigger = FALSE;
		memset(this->twoWayZones,0,NUM_TWOWAY_ZONES*sizeof(long));
		memset(this->twoWayZonesFine,0,NUM_TWOWAY_ZONES*sizeof(long));
		this->laserCoherentSamplingEnable = FALSE;
		this->laserCoherentSamplingPhase = FALSE;
		memset(this->channelPolarity,0,MAX_CHANNEL_COUNT*sizeof(long));
		this->frameTriggerEnableWithHWTrig = TRUE;
		this->threePhotonModeEnable = FALSE;
	}

	bool IsEqual(ImgAcqPty &imagAcqPty)
	{
		return(!((imagAcqPty.fieldSize != this->fieldSize) ||
		(imagAcqPty.pixelX != this->pixelX) ||
		(imagAcqPty.pixelY != this->pixelY) ||
		(imagAcqPty.offsetX != this->offsetX) ||
		(imagAcqPty.offsetY != this->offsetY) ||
		(imagAcqPty.channel != this->channel) ||
		(imagAcqPty.clockSource !=  this->clockSource)||
		(imagAcqPty.averageMode != this->averageMode) ||
		(imagAcqPty.averageNum != this->averageNum) ||
		(imagAcqPty.scanMode != this->scanMode) ||
		(imagAcqPty.alignmentForField != this->alignmentForField) ||
		(imagAcqPty.clockRateInternal != this->clockRateInternal) ||
		(imagAcqPty.yAmplitudeScaler != this->yAmplitudeScaler)||
		(imagAcqPty.areaMode != this->areaMode)||
		(imagAcqPty.flybackCycle != this->flybackCycle)||
		(imagAcqPty.dataMapMode != this->dataMapMode)||
		(imagAcqPty.dwellTime != this->dwellTime)||
		(imagAcqPty.triggerMode != this->triggerMode) ||
		(imagAcqPty.progressCounter != this->progressCounter) ||
		(imagAcqPty.galvoEnable != this->galvoEnable) ||
		(imagAcqPty.yChannelEnable != this->yChannelEnable)||
		(memcmp(&imagAcqPty.DCOffset,&this->DCOffset,sizeof(this->DCOffset))!=0)||
		(imagAcqPty.verticalScanDirection != this->verticalScanDirection)||
		(imagAcqPty.horizontalFlip != this->horizontalFlip)||
		(imagAcqPty.numFrame != this->numFrame)||
		(imagAcqPty.scanAreaAngle != this->scanAreaAngle)||
		(imagAcqPty.rawSaveEnabledChannelOnly!= this->rawSaveEnabledChannelOnly)||
		(!imagAcqPty.pockelPty.IsEqual(this->pockelPty))||
		(imagAcqPty.resetFlybackEnable != this->resetFlybackEnable)||
		(imagAcqPty.realTimeDataAverage != this->realTimeDataAverage)||
		(imagAcqPty.captureWithoutLineTrigger != this->captureWithoutLineTrigger)||
		(memcmp(&imagAcqPty.twoWayZones,&this->twoWayZones,sizeof(this->twoWayZones))!=0)||
		(memcmp(&imagAcqPty.twoWayZonesFine,&this->twoWayZonesFine,sizeof(this->twoWayZonesFine))!=0)||
		(memcmp(&imagAcqPty.channelPolarity,&this->channelPolarity,sizeof(this->channelPolarity))!=0)||
		(imagAcqPty.laserCoherentSamplingEnable != this->laserCoherentSamplingEnable)||
		(imagAcqPty.frameTriggerEnableWithHWTrig != this->frameTriggerEnableWithHWTrig)||
		(imagAcqPty.laserCoherentSamplingPhase != this->laserCoherentSamplingPhase)||
		(imagAcqPty.threePhotonModeEnable != this->threePhotonModeEnable)
		));
	}

	ImgAcqPty& operator = (const ImgAcqPty &imagAcqPty)
	{
		this->pixelX = imagAcqPty.pixelX;
		this->pixelY = imagAcqPty.pixelY;
		this->fieldSize = imagAcqPty.fieldSize;
		this->offsetX = imagAcqPty.offsetX; 
		this->offsetY = imagAcqPty.offsetY; 
		this->channel = imagAcqPty.channel; 
		this->alignmentForField = imagAcqPty.alignmentForField; 
		this->inputRangeChannel1 = imagAcqPty.inputRangeChannel1; 
		this->inputRangeChannel2 = imagAcqPty.inputRangeChannel2;
		this->inputRangeChannel3 = imagAcqPty.inputRangeChannel3;
		this->inputRangeChannel4 = imagAcqPty.inputRangeChannel4;
		this->clockSource = imagAcqPty.clockSource; 
		this->clockRateInternal = imagAcqPty.clockRateInternal; 
		this->clockRateExternal = imagAcqPty.clockRateExternal; 
		this->scanMode = imagAcqPty.scanMode;  
		this->averageMode = imagAcqPty.averageMode; 
		this->averageNum= imagAcqPty.averageNum;
		this->triggerMode = imagAcqPty.triggerMode; 
		this->numFrame = imagAcqPty.numFrame; 
		this->yAmplitudeScaler = imagAcqPty.yAmplitudeScaler;
		this->areaMode = imagAcqPty.areaMode;
		this->flybackCycle = imagAcqPty.flybackCycle;
		this->dataMapMode = imagAcqPty.dataMapMode;
		this->dwellTime = imagAcqPty.dwellTime; 
		this->progressCounter = imagAcqPty.progressCounter;
		this->galvoEnable = imagAcqPty.galvoEnable;
		this->yChannelEnable = imagAcqPty.yChannelEnable;
		memcpy(this->DCOffset,imagAcqPty.DCOffset,2*sizeof(double));	
		this->verticalScanDirection = imagAcqPty.verticalScanDirection;
		this->scanAreaAngle = imagAcqPty.scanAreaAngle;
		this->rawSaveEnabledChannelOnly = imagAcqPty.rawSaveEnabledChannelOnly; 
		this->horizontalFlip = imagAcqPty.horizontalFlip;	
		this->pockelPty = imagAcqPty.pockelPty;
		this->resetFlybackEnable = imagAcqPty.resetFlybackEnable;
		this->realTimeDataAverage = imagAcqPty.realTimeDataAverage;
		this->captureWithoutLineTrigger = imagAcqPty.captureWithoutLineTrigger;
		memcpy(this->twoWayZones,imagAcqPty.twoWayZones,NUM_TWOWAY_ZONES*sizeof(long));
		memcpy(this->twoWayZonesFine,imagAcqPty.twoWayZonesFine,NUM_TWOWAY_ZONES*sizeof(long));
		memcpy(this->channelPolarity,imagAcqPty.channelPolarity,MAX_CHANNEL_COUNT*sizeof(long));
		this->laserCoherentSamplingEnable = imagAcqPty.laserCoherentSamplingEnable;
		this->laserCoherentSamplingPhase = imagAcqPty.laserCoherentSamplingPhase;
		this->frameTriggerEnableWithHWTrig = imagAcqPty.frameTriggerEnableWithHWTrig;
		this->threePhotonModeEnable = imagAcqPty.threePhotonModeEnable;
		return *this;
	}
};

#endif