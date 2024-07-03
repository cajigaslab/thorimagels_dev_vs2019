#include "acq.h"

ImgAcqPty::ImgAcqPty()
{
	this->pixelX = DEFAULT_PIXEL_X;
	this->pixelY = DEFAULT_PIXEL_Y;
	this->fieldSize = DEFAULT_FIELD_SIZE_X;
	this->offsetX = 0; 
	this->offsetY = 0; 
	this->channel = MIN_CHANNEL;
	memset(this->inputRange,DEFAULT_INPUTRANGE,MAX_CHANNEL_COUNT*sizeof(long));
	this->alignmentForField = DEFAULT_ALIGNMENT;
	this->clockSource = 0; //internal is 1, external is 2, set it to 0 so it gets set when loading settings
	this->clockRateInternal = DEFAULT_INTERNALCLOCKRATE; 
	this->clockRateExternal = DEFAULT_EXTCLOCKRATE; 
	this->maxSampleRate = DEFAULT_INTERNALCLOCKRATE;
	this->scanMode = DEFAULT_SCANMODE;  
	this->averageMode = NO_AVERAGE; 
	this->averageNum= 2;
	this->triggerMode = DEFAULT_TRIGGER_MODE; 
	this->numFrame = 1; 
	this->yAmplitudeScaler = DEFAULT_Y_AMPLITUDE_SCALER;
	this->areaMode = ICamera::SQUARE;
	this->flybackCycle = DEFAULT_FLYBACK_CYCLE;
	this->minimizeFlybackCycles = true;
	this->dataMapMode = ICamera::POLARITY_MIXED;
	this->dwellTime = DEFAULT_DWELL_TIME; 
	this->rasterAngle = DEFAULT_RASTERANGLE;
	this->galvoForwardLineDuty = DEFAULT_FORWARD_LINE_DUTY; 
	this->progressCounter = 0;
	this->galvoEnable = TRUE;
	this->yChannelEnable = TRUE;
	this->verticalScanDirection = 1;
	this->scanAreaAngle = DEFAULT_SCANAREA_ANGLE;
	this->dmaBufferCount = DEFAULT_DMA_BUFFER_NUM;
	this->rawSaveEnabledChannelOnly = FALSE; 
	this->horizontalFlip = FALSE;
	memset(this->ADCGain,0,4*sizeof(ULONG32));
	memset(this->channelPolarity,0,MAX_CHANNEL_COUNT*sizeof(long));
	memset(this->FIRFilters,1,FIR_FILTER_COUNT*MAX_CHANNEL_COUNT*FIR_FILTER_TAP_COUNT*sizeof(double));
	this->PhaseMode = 0;
	this->PhaseLimit = 0;
	this->fineOffsetX = 0.0;
	this->fineOffsetY = 0.0;
	this->fineFieldSizeScaleX = DEFALUT_FINE_FIELD_SCALE;
	this->fineFieldSizeScaleY = DEFALUT_FINE_FIELD_SCALE;
	this->pockelPty = PockelPty();
	this->laserCoherentSamplingEnable = FALSE;
	this->laserCoherentSamplingPhase = 0;
	this->realTimeDataAverage = FALSE;
	this->threePhotonModeEnable = FALSE;
	memset(this->threePhotonModeAlignmentPhase, 0, MAX_CHANNEL_COUNT * sizeof(long));
	this->turnAroundTimeUS = DEFAULT_TURN_AROUND_TIME_US;
	this->numberOfPlanes = MIN_NUMBER_OF_PLANES;
	this->FIR1ManualControlenable = FALSE;
	this->powerRampEnable = FALSE;
	this->powerRampNumFrames = MIN_NUMBER_OF_POWER_RAMP_FRAMES;
	this->powerRampNumFlybackFrames = DEFAULT_NUMBER_OF_POWER_RAMP_FLYBACK_FRAMES;
	this->powerRampPercentValues.clear();
	this->powerRampMode = 0;
	this->fastOneWayEnable = FALSE;
	this->acquireDuringTurnAround = FALSE;
	this->sampleOffsetStartLUT3PTI = 0;
	this->enableDownsamplingRateChange = FALSE;
	this->threePhotonDownsamplingRate = DEFAULT_DOWNSAMPLING_RATE;
	this->selectedImagingGG = 0;
	this->selectedStimGG = 0;
	this->movingAverageFilterEnable = FALSE;
	this->lineAveragingEnable = FALSE;
	this->lineAveragingNumber = DEFAULT_LINE_AVERAGING_NUMBE;
}

bool ImgAcqPty::KeyPropertiesAreEqual(ImgAcqPty &imagAcqPty)
{
	return(!((imagAcqPty.fieldSize != this->fieldSize) ||
		(imagAcqPty.pixelX != this->pixelX) ||
		(imagAcqPty.pixelY != this->pixelY) ||
		(imagAcqPty.offsetX != this->offsetX) ||
		(imagAcqPty.offsetY != this->offsetY) ||
		(imagAcqPty.channel != this->channel) ||
		(imagAcqPty.averageMode != this->averageMode) ||
		(imagAcqPty.averageNum != this->averageNum) ||
		(imagAcqPty.scanMode != this->scanMode) ||
		(imagAcqPty.alignmentForField != this->alignmentForField) ||
		(memcmp(&imagAcqPty.inputRange, &this->inputRange, sizeof(this->inputRange)) != 0) ||
		(imagAcqPty.clockRateInternal != this->clockRateInternal) ||
		(imagAcqPty.maxSampleRate != this->maxSampleRate) ||
		(imagAcqPty.yAmplitudeScaler != this->yAmplitudeScaler) ||
		(imagAcqPty.areaMode != this->areaMode) ||
		(imagAcqPty.flybackCycle != this->flybackCycle) ||
		(imagAcqPty.minimizeFlybackCycles != this->minimizeFlybackCycles) ||
		(imagAcqPty.dataMapMode != this->dataMapMode) ||
		(imagAcqPty.rasterAngle != this->rasterAngle) ||
		(imagAcqPty.dwellTime != this->dwellTime) ||
		(imagAcqPty.galvoForwardLineDuty != this->galvoForwardLineDuty) ||
		(imagAcqPty.triggerMode != this->triggerMode) ||
		(imagAcqPty.progressCounter != this->progressCounter) ||
		(imagAcqPty.galvoEnable != this->galvoEnable) ||
		(imagAcqPty.yChannelEnable != this->yChannelEnable) ||
		(memcmp(&imagAcqPty.channelPolarity, &this->channelPolarity, sizeof(this->channelPolarity)) != 0) ||
		(memcmp(&imagAcqPty.FIRFilters, &this->FIRFilters, sizeof(this->FIRFilters)) != 0) ||
		(memcmp(&imagAcqPty.ADCGain, &this->ADCGain, sizeof(this->ADCGain)) != 0) ||
		(imagAcqPty.verticalScanDirection != this->verticalScanDirection) ||
		(imagAcqPty.horizontalFlip != this->horizontalFlip) ||
		(imagAcqPty.fineOffsetX != this->fineOffsetX) ||
		(imagAcqPty.fineOffsetY != this->fineOffsetY) ||
		(imagAcqPty.fineFieldSizeScaleX != this->fineFieldSizeScaleX) ||
		(imagAcqPty.fineFieldSizeScaleY != this->fineFieldSizeScaleY) ||
		(imagAcqPty.dmaBufferCount != this->dmaBufferCount) ||
		(imagAcqPty.numFrame != this->numFrame) ||
		(imagAcqPty.scanAreaAngle != this->scanAreaAngle) ||
		(imagAcqPty.rawSaveEnabledChannelOnly != this->rawSaveEnabledChannelOnly) ||
		(imagAcqPty.PhaseMode != this->PhaseMode) ||
		(imagAcqPty.PhaseLimit != this->PhaseLimit) ||
		(!imagAcqPty.pockelPty.IsEqual(this->pockelPty)) ||
		(imagAcqPty.realTimeDataAverage != this->realTimeDataAverage) ||
		(imagAcqPty.laserCoherentSamplingEnable != this->laserCoherentSamplingEnable) ||
		(imagAcqPty.laserCoherentSamplingPhase != this->laserCoherentSamplingPhase) ||
		(imagAcqPty.threePhotonModeEnable != this->threePhotonModeEnable) ||
		(imagAcqPty.turnAroundTimeUS != this->turnAroundTimeUS) ||
		(imagAcqPty.numberOfPlanes != this->numberOfPlanes) ||
		(imagAcqPty.FIR1ManualControlenable != this->FIR1ManualControlenable) ||
		(imagAcqPty.powerRampEnable != this->powerRampEnable) ||
		(imagAcqPty.powerRampNumFrames != this->powerRampNumFrames) ||
		(imagAcqPty.powerRampNumFlybackFrames != this->powerRampNumFlybackFrames) ||
		(imagAcqPty.powerRampPercentValues != this->powerRampPercentValues) ||
		(imagAcqPty.powerRampMode != this->powerRampMode) ||
		(imagAcqPty.fastOneWayEnable != this->fastOneWayEnable) ||
		(imagAcqPty.acquireDuringTurnAround != this->acquireDuringTurnAround) ||
		(imagAcqPty.sampleOffsetStartLUT3PTI != this->sampleOffsetStartLUT3PTI) ||
		(imagAcqPty.enableDownsamplingRateChange != this->enableDownsamplingRateChange) ||
		(imagAcqPty.threePhotonDownsamplingRate != this->threePhotonDownsamplingRate) ||
		(memcmp(&imagAcqPty.threePhotonModeAlignmentPhase, &this->threePhotonModeAlignmentPhase, sizeof(this->threePhotonModeAlignmentPhase)) != 0) ||
		(imagAcqPty.selectedImagingGG != this->selectedImagingGG) ||
		(imagAcqPty.selectedStimGG != this->selectedStimGG) || 
		(imagAcqPty.movingAverageFilterEnable != this->movingAverageFilterEnable) || 
		(imagAcqPty.movingAverageFilterMultiplier != this->movingAverageFilterMultiplier) ||
		(imagAcqPty.lineAveragingEnable != this->lineAveragingEnable) ||
		(imagAcqPty.lineAveragingNumber != this->lineAveragingNumber)
		));
}

ImgAcqPty& ImgAcqPty::operator = (const ImgAcqPty& imagAcqPty)
{
	this->pixelX = imagAcqPty.pixelX;
	this->pixelY = imagAcqPty.pixelY;
	this->fieldSize = imagAcqPty.fieldSize;
	this->offsetX = imagAcqPty.offsetX;
	this->offsetY = imagAcqPty.offsetY;
	this->channel = imagAcqPty.channel;
	this->alignmentForField = imagAcqPty.alignmentForField;
	this->clockSource = imagAcqPty.clockSource;
	this->clockRateInternal = imagAcqPty.clockRateInternal;
	this->clockRateExternal = imagAcqPty.clockRateExternal;
	this->maxSampleRate = imagAcqPty.maxSampleRate;
	this->scanMode = imagAcqPty.scanMode;
	this->averageMode = imagAcqPty.averageMode;
	this->averageNum = imagAcqPty.averageNum;
	this->triggerMode = imagAcqPty.triggerMode;
	this->numFrame = imagAcqPty.numFrame;
	this->yAmplitudeScaler = imagAcqPty.yAmplitudeScaler;
	this->areaMode = imagAcqPty.areaMode;
	this->flybackCycle = imagAcqPty.flybackCycle;
	this->minimizeFlybackCycles = imagAcqPty.minimizeFlybackCycles;
	this->dataMapMode = imagAcqPty.dataMapMode;
	this->dwellTime = imagAcqPty.dwellTime;
	this->rasterAngle = imagAcqPty.rasterAngle;
	this->galvoForwardLineDuty = imagAcqPty.galvoForwardLineDuty;
	this->progressCounter = imagAcqPty.progressCounter;
	this->galvoEnable = imagAcqPty.galvoEnable;
	this->yChannelEnable = imagAcqPty.yChannelEnable;
	this->verticalScanDirection = imagAcqPty.verticalScanDirection;
	this->fineOffsetX = imagAcqPty.fineOffsetX;
	this->fineOffsetY = imagAcqPty.fineOffsetY;
	this->fineFieldSizeScaleX = imagAcqPty.fineFieldSizeScaleX;
	this->fineFieldSizeScaleY = imagAcqPty.fineFieldSizeScaleY;
	this->scanAreaAngle = imagAcqPty.scanAreaAngle;
	this->dmaBufferCount = imagAcqPty.dmaBufferCount;
	this->rawSaveEnabledChannelOnly = imagAcqPty.rawSaveEnabledChannelOnly;
	this->horizontalFlip = imagAcqPty.horizontalFlip;
	this->PhaseMode = imagAcqPty.PhaseMode;
	this->PhaseLimit = imagAcqPty.PhaseLimit;
	this->pockelPty = imagAcqPty.pockelPty;
	this->laserCoherentSamplingEnable = imagAcqPty.laserCoherentSamplingEnable;
	this->laserCoherentSamplingPhase = imagAcqPty.laserCoherentSamplingPhase;
	this->threePhotonModeEnable = imagAcqPty.threePhotonModeEnable;
	memcpy(this->threePhotonModeAlignmentPhase, imagAcqPty.threePhotonModeAlignmentPhase, MAX_CHANNEL_COUNT * sizeof(long));
	this->turnAroundTimeUS = imagAcqPty.turnAroundTimeUS;
	this->numberOfPlanes = imagAcqPty.numberOfPlanes;
	this->FIR1ManualControlenable = imagAcqPty.FIR1ManualControlenable;
	memcpy(this->inputRange, imagAcqPty.inputRange, MAX_CHANNEL_COUNT * sizeof(long));
	memcpy(this->channelPolarity, imagAcqPty.channelPolarity, MAX_CHANNEL_COUNT * sizeof(long));
	memcpy(this->FIRFilters, imagAcqPty.FIRFilters, FIR_FILTER_COUNT * MAX_CHANNEL_COUNT * FIR_FILTER_TAP_COUNT * sizeof(double));
	memcpy(this->ADCGain, imagAcqPty.ADCGain, MAX_CHANNEL_COUNT * sizeof(ULONG32));
	this->powerRampEnable = imagAcqPty.powerRampEnable;
	this->powerRampNumFrames = imagAcqPty.powerRampNumFrames;
	this->powerRampNumFlybackFrames = imagAcqPty.powerRampNumFlybackFrames;
	this->powerRampPercentValues = imagAcqPty.powerRampPercentValues;
	this->powerRampMode = imagAcqPty.powerRampMode;
	this->fastOneWayEnable = imagAcqPty.fastOneWayEnable;
	this->acquireDuringTurnAround = imagAcqPty.acquireDuringTurnAround;
	this->selectedImagingGG = imagAcqPty.selectedImagingGG;
	this->selectedStimGG = imagAcqPty.selectedStimGG;
	this->sampleOffsetStartLUT3PTI = imagAcqPty.sampleOffsetStartLUT3PTI;
	this->movingAverageFilterEnable = imagAcqPty.movingAverageFilterEnable;
	this->movingAverageFilterMultiplier = imagAcqPty.movingAverageFilterMultiplier;
	this->enableDownsamplingRateChange = imagAcqPty.enableDownsamplingRateChange;
	this->threePhotonDownsamplingRate = imagAcqPty.threePhotonDownsamplingRate;
	this->lineAveragingEnable = imagAcqPty.lineAveragingEnable;
	this->lineAveragingNumber = imagAcqPty.lineAveragingNumber;
	return *this;
}


PockelPty::PockelPty()
{
	memset(this->pockelsPowerLevel, 0, MAX_POCKELS_CELL_COUNT * sizeof(double));
	memset(this->pockelsLineBlankingPercentage, 0, MAX_POCKELS_CELL_COUNT * sizeof(double));
	memset(this->pockelsMinVoltage, 0, MAX_POCKELS_CELL_COUNT * sizeof(double));
	memset(this->pockelsMaxVoltage, 0, MAX_POCKELS_CELL_COUNT * sizeof(double));
	memset(this->pockelsMaskEnable, 0, MAX_POCKELS_CELL_COUNT * sizeof(long));
	memset(this->pockelsMaskInvert, 0, MAX_POCKELS_CELL_COUNT * sizeof(long));
	memset(this->pockelOutputChannel, 0, MAX_POCKELS_CELL_COUNT * sizeof(long));
	memset(this->pockelsDelayUS, 0, MAX_POCKELS_CELL_COUNT * sizeof(double));
}

bool PockelPty::IsEqual(PockelPty& iPockelPty)
{
	return(!(
		(memcmp(&iPockelPty.pockelsLineBlankingPercentage, &this->pockelsLineBlankingPercentage, sizeof(this->pockelsLineBlankingPercentage)) != 0) ||
		(memcmp(&iPockelPty.pockelsPowerLevel, &this->pockelsPowerLevel, sizeof(this->pockelsPowerLevel)) != 0) ||
		(memcmp(&iPockelPty.pockelsMinVoltage, &this->pockelsMinVoltage, sizeof(this->pockelsMinVoltage)) != 0) ||
		(memcmp(&iPockelPty.pockelsMaxVoltage, &this->pockelsMaxVoltage, sizeof(this->pockelsMaxVoltage)) != 0) ||
		(memcmp(&iPockelPty.pockelsMaskEnable, &this->pockelsMaskEnable, sizeof(this->pockelsMaskEnable)) != 0) ||
		(memcmp(&iPockelPty.pockelsMaskInvert, &this->pockelsMaskInvert, sizeof(this->pockelsMaskInvert)) != 0) ||
		(memcmp(&iPockelPty.pockelOutputChannel, &this->pockelOutputChannel, sizeof(this->pockelOutputChannel)) != 0) ||
		(memcmp(&iPockelPty.pockelsDelayUS, &this->pockelsDelayUS, sizeof(this->pockelsDelayUS)) != 0)
		));
}

PockelPty& PockelPty::operator = (const PockelPty& iPockelPty)
{
	memcpy(this->pockelsPowerLevel, iPockelPty.pockelsPowerLevel, MAX_POCKELS_CELL_COUNT * sizeof(double));
	memcpy(this->pockelsLineBlankingPercentage, iPockelPty.pockelsLineBlankingPercentage, MAX_POCKELS_CELL_COUNT * sizeof(double));
	memcpy(this->pockelsMinVoltage, iPockelPty.pockelsMinVoltage, MAX_POCKELS_CELL_COUNT * sizeof(double));
	memcpy(this->pockelsMaxVoltage, iPockelPty.pockelsMaxVoltage, MAX_POCKELS_CELL_COUNT * sizeof(double));
	memcpy(this->pockelsMaskEnable, iPockelPty.pockelsMaskEnable, MAX_POCKELS_CELL_COUNT * sizeof(long));
	memcpy(this->pockelsMaskInvert, iPockelPty.pockelsMaskInvert, MAX_POCKELS_CELL_COUNT * sizeof(long));
	memcpy(this->pockelOutputChannel, iPockelPty.pockelOutputChannel, MAX_POCKELS_CELL_COUNT * sizeof(long));
	memcpy(this->pockelsDelayUS, iPockelPty.pockelsDelayUS, MAX_POCKELS_CELL_COUNT * sizeof(double));
	return *this;
}