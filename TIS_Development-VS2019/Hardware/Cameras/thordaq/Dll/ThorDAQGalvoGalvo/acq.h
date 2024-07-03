#ifndef CONST_H
#include "const.h"
#endif

#ifndef ACQ_H
#define ACQ_H
#include "stdafx.h"
#include "thordaqcmd.h"

class PockelPty
{
	public:
		double pockelsPowerLevel[MAX_POCKELS_CELL_COUNT];///<level of pockels power		
		double pockelsPowerLevel2[MAX_POCKELS_CELL_COUNT];///<level of pockels power	
		double pockelsLineBlankingPercentage[MAX_POCKELS_CELL_COUNT];///<percentage of line to keep pockels at power
		double pockelsMinVoltage[MAX_POCKELS_CELL_COUNT];///<array of minimums for the pockels minimum output 
		double pockelsMaxVoltage[MAX_POCKELS_CELL_COUNT];///<array of maximums for the pockels maximum output 
		long   pockelsMaskEnable[MAX_POCKELS_CELL_COUNT];///<use pockels mask(s)
		long   pockelsMaskInvert[MAX_POCKELS_CELL_COUNT];///<invert pockels mask(s)
		long   pockelOutputChannel[MAX_POCKELS_CELL_COUNT];///<output channel for the pockels>
		double pockelsDelayUS[MAX_POCKELS_CELL_COUNT];///<line delay for pockels>
		PockelPty();
		bool IsEqual(PockelPty &iPockelPty);
		PockelPty& operator = (const PockelPty &iPockelPty);
};


class ImgAcqPty
{
public:
	long   fieldSize; ///<A parameter scales to scan field size, the actual scan sized at the object plane may vary from device to device
	long   pixelX; ///<Image pixel number in the x direction
	long   pixelY; ///<Image pixel number in the y direction
	long   offsetX; ///<Off set of scan field size in the x direction
	long   offsetY; ///<Off set of scan field size in the y direction
	long   channel; ///<Bitwise selection of channels.
	long   averageMode; ///< average mode, see enumeration of AverageMode;
	long   averageNum;///< number of frame, lines to average
	long   alignmentForField; ///<Forward Backward scan alignment
	long   inputRange[MAX_CHANNEL_COUNT]; ///<The digitizer input channel measurement range, see the enumeration of "inputrange"
	long   clockSource; ///<0 to use internal sample clock source, 1 to use external clock source
	long   clockRateInternal; ///< internal sample clock rate, the enumeration of "ClockRate"
	long   clockRateExternal; ///< external sample clock rate, this the actual sample rate
	long   maxSampleRate; ///< current  ADC sample rate
	long   scanMode; ///< 2 way, 1 way forward or 1 way backward scan mode, see enumeration of ScanMode
	long   triggerMode; ///<trigger source of each frame
	unsigned long long  numFrame; ///<number of frame to acquire for a experiment
	double yAmplitudeScaler;///<pixel aspect ratio scaler
	long   areaMode;///<scan pattern mode square,rect,line
	long   flybackCycle;///<number of line cycles to wait when galvo is flying back to start of the frame
	bool   minimizeFlybackCycles; ///<flag to minimize the number of flyback cycles
	long   dataMapMode;///<mapping of raw digital values into output buffer
	double dwellTime; ///<number of lines per second for the galvo-galvo raster scan
	double rasterAngle; ///<angle of scan rotation in unit of arc degree, x'=x cos(a)-y sin(a);  y'=x sin(a)+ y cos (a)
	double galvoForwardLineDuty; ///<duty cycle in percent for the forward and backward lines,  = FwdLineSplLength/(LineSplLength)
	long   progressCounter;///<stores the completed number of frames. Purpose is to force the SetupAcquisition to execute all hardware settings
	long   galvoEnable;///<enable or disable y galvo movement
	long   yChannelEnable;///<enable or disable the communication with y channel, NOT implemented yet.
	double verticalScanDirection;///<direction of the y galvo scan 1:increasing scan  -1: decreasing scan 
	double fineOffsetX;///<fine adjustment of offset X
	double fineOffsetY;///<fine adjustment of offset Y
	double fineFieldSizeScaleX;///<fine adjustment of field size X
	double fineFieldSizeScaleY;///<fine adjustment of field size Y
	double scanAreaAngle;///<angle for roi
	long   dmaBufferCount;///<number of buffers for circular buffer that grabs data from the card and the circular buffer that sends it to CopyAcquisition
	long   rawSaveEnabledChannelOnly; ///<copy only the enabled channels continuously into the buffer
	long   horizontalFlip;///<flip the image in the X direction
	ULONG32   ADCGain[MAX_CHANNEL_COUNT];
	USHORT PhaseMode;
	USHORT PhaseLimit;
	PockelPty pockelPty;
	long laserCoherentSamplingEnable;
	long laserCoherentSamplingPhase;
	long realTimeDataAverage;
	long threePhotonModeEnable;
	long threePhotonModeAlignmentPhase[MAX_CHANNEL_COUNT]; // do not check if this one changed, it will be updated live
	double FIRFilters[FIR_FILTER_COUNT][MAX_CHANNEL_COUNT][FIR_FILTER_TAP_COUNT];
	long channelPolarity[MAX_CHANNEL_COUNT];
	long turnAroundTimeUS;
	long numberOfPlanes;
	long FIR1ManualControlenable;
	long powerRampEnable;
	long powerRampNumFrames;
	long powerRampNumFlybackFrames;
	long powerRampMode;
	long fastOneWayEnable;
	long acquireDuringTurnAround;
	long sampleOffsetStartLUT3PTI;
	long enableDownsamplingRateChange;
	long threePhotonDownsamplingRate;
	long selectedImagingGG;
	long selectedStimGG;
	long lineAveragingEnable;
	long lineAveragingNumber;
	std::vector<double> powerRampPercentValues;
	long movingAverageFilterEnable;
	double movingAverageFilterMultiplier;
	ImgAcqPty();
	bool KeyPropertiesAreEqual(ImgAcqPty &imagAcqPty);
	ImgAcqPty& operator = (const ImgAcqPty &imagAcqPty);

};

#endif