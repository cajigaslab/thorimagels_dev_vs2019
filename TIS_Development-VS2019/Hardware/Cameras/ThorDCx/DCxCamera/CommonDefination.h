#ifndef _COMMON_DEFINATION_H_
#define _COMMON_DEFINATION_H_

#include "uc480class.h"

#define SERIAL_NUMBER_MAX_LENGHT   16

#define MIN_ANGLE				0
#define MAX_ANGLE				360

#define BINNING_X_MIN			1
#define BINNING_X_MAX			16
#define BINNING_Y_MIN			1
#define BINNING_Y_MAX			16

#define GAMA_MIN				1
#define GAMA_MAX				2.2
#define GAMA_DEFAULT			1

typedef struct _DCX_ROI 
{
	INT Left;
	INT Top;
	INT Bottom;
	INT Right;
} DCX_ROI, *PDCX_ROI;

typedef struct _DCX_EXPOSURE
{
	DOUBLE Min;
	DOUBLE Max;
	DOUBLE Interval;
	DOUBLE Current;
} DCX_EXPOSURE, *PDCX_EXPOSURE;

typedef struct _DCX_PIXEL_CLOCK
{
	INT pixelClock;
	INT min;
	INT max;
} DCX_PIXEL_CLOCK, *PDCX_PIXEL_CLOCK;

typedef struct _DCX_FRAME_TIME
{
	DOUBLE min;
	DOUBLE max;
	DOUBLE interval;
} DCX_FRAME_TIME, *PDCX_FRAME_TIME;

typedef struct _DCX_BINNING
{
	BOOL isBinning;	//true: Binning supported, false: SubSampling supported
	INT xCurrent;
	INT yCurrent;
	INT supportBinning;
} DCX_BINNING, *PDCX_BINNING;

typedef struct _ImgPty
{
	DCX_EXPOSURE exposure;
	DCX_ROI	roi;		
	DCX_BINNING binning;
	DCX_PIXEL_CLOCK pixelClock;
	DCX_FRAME_TIME frameTime;

	SENSORSCALERINFO scalerInfo;
	SENSORINFO sensorInfo;
	IS_RANGE_S32 blackRange;

	DOUBLE scaler;
	DOUBLE gain;
	DOUBLE gamma;
	INT triggerMode; ///<trigger source of each frame
	INT triggerSource;
	INT blackLevel;
	INT tapsIndex;
	INT tapBalanceEnable;
	INT numImagesToBuffer;
	INT readOutSpeedIndex; // 0=20MHz, 1=40MHz
	INT channel; ///<Bitwise selection of channels.		
	INT averageMode; ///< average mode, see enumeration of AverageMode;
	INT averageNum;///< number of frame, lines to average		

	INT dmaBufferCount;///<number of buffers for DMA
	INT verticalFlip;///<flip the image in the Y direction
	INT horizontalFlip;///<flip the image in the X direction
	INT imageAngle; ///<0,90,180,270

	BOOL hotPixelCorrection;

	DOUBLE multiFrame; 
	DOUBLE frameRate; ///<number of frame to acquire for a experiment
	INT colorMode;	//color mode
	INT bitsPerPixel; // Used to store the camera's bitsPerPixel 
}ImgPty, *pImgPty;

#endif