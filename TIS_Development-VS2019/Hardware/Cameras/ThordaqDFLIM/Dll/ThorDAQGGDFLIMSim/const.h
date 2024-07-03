
//========= Copyright Thorlabs Imaging Research Group, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef CONST_H
#define CONST_H
#include "stdafx.h"

#define TEST TRUE

#define TIMEOUT_MS 1500

#define FLYBACK_CYCLE_SCALE 4

#define MAX_CHANNEL_COUNT 4

#define MAX_DMA_BUFFER_NUM 2048
#define DEFAULT_DMA_BUFFER_NUM 1

#define MAX_POCKELS_CELL_COUNT 4

#define POCKELS_VOLTAGE_STEPS 100

#define LINE_FRAMETRIGGER_LOW_TIMEPOINTS 15

//
//Clock Source
//
#define INTERNAL_CLOCK			0x00000001UL
#define EXTERNAL_CLOCK			0x00000002UL

// This is the min# and max# of resolution of image
#define MAX_PIXEL_X		4096
#define MIN_PIXEL_X		32
#define MAX_PIXEL_Y		4096
#define MIN_PIXEL_Y		1 //Allow Line Scan, which vertical resolution is 1
// This is the default,
#define DEFAULT_PIXEL_X          1024
#define DEFAULT_PIXEL_Y          1024
// This is the min# and max# of scan area fieldsize
#define MAX_FIELD_SIZE_X         255
#define MIN_FIELD_SIZE_X         5
#define DEFAULT_FIELD_SIZE_X     180

#define MIN_ALIGNMENT            0//-5000
#define MAX_ALIGNMENT            65535//5000
#define DEFAULT_ALIGNMENT        0

#define MIN_AVERAGENUM           2
#define MAX_AVERAGENUM           1024
#define DEFAULT_AVERAGENUM       8

#define MIN_SCANMODE             0
#define MAX_SCANMODE		     4
#define DEFAULT_SCANMODE         0

#define MIN_TRIGGER_MODE         0
#define MAX_TRIGGER_MODE         5
#define DEFAULT_TRIGGER_MODE     2

#define MAX_GALVO_READ_TIMEOUT   10

#define MIN_CHANNEL              1
#define MAX_CHANNEL              0xF

#define MIN_TRIGGER_TIMEOUT      1 
#define DEFAULT_TRIGGER_TIMEOUT  30
#define MAX_TRIGGER_TIMEOUT      2147483

#define MIN_ENABLE_FRAME_TRIGGER 0
#define MAX_ENABLE_FRAME_TRIGGER 1
 
#define MIN_AREAMODE             0
#define MAX_AREAMODE             5

#define MIN_Y_AMPLITUDE_SCALER       0
#define MAX_Y_AMPLITUDE_SCALER       1000
#define DEFAULT_Y_AMPLITUDE_SCALER   100

#define MIN_INPUTRANGE        0
#define MAX_INPUTRANGE        0
#define DEFAULT_INPUTRANGE    0


#define MIN_FLYBACK_CYCLE            0
#define MAX_FLYBACK_CYCLE            1000000
#define MAX_FLYBACK_TIME             5 //Max flyback time 5s
#define DEFAULT_FLYBACK_CYCLE        1

#define MIN_GALVO_ENABLE             0
#define MAX_GALVO_ENABLE             1
#define DEFAULT_GALVO_ENABLE         1

#define MAX_RASTERANGLE              180
#define MIN_RASTERANGLE              -180
#define DEFAULT_RASTERANGLE          0

#define MAX_FORWARD_LINE_DUTY        1
#define MIN_FORWARD_LINE_DUTY        0.5
#define DEFAULT_FORWARD_LINE_DUTY    0.5

#define MIN_DWELL_TIME               8
#define MAX_DWELL_TIME               20
#define DEFAULT_DWELL_TIME           8
#define DWELL_TIME_STEP              0.2

#define MIN_GALVO_VOLTAGE            -10.0
#define MAX_GALVO_VOLTAGE            10.0

#define GALVO_PARK_POSITION          10

#define GALVO_PADDING_SAMPLE         64//PAD ONE BOTH END OF FAST AXIS WAVEFORM TO REDUCE NOISE, LEAVE TRIGGER REARMING TIME, AND ENSURE LINEAR PART IN THE MIDDLE

#define GALVO_MOVE_PATH_LENGTH       128

#define GALVO_MOVE_PATH_SLEEP        2

#define GALVO_LINE_PADDING_PERCENT   0.25

#define GALVO_RETRACE_TIME          200.005 // 203.845//This is half of Galvo Turn around time, total time is 400 us 
#define MIN_TURN_AROUND_TIME_US		50 // 150 us for one full turn around
#define MAX_TURN_AROUND_TIME_US 	1000 // 1000 us for one full turn around
#define DEFAULT_TURN_AROUND_TIME_US	400 // 400 us for one full turn around

#define MAX_GALVO_OPTICAL_ANGLE      20

#define MIN_Y_CHANNEL_ENABLE         0
#define MAX_Y_CHANNEL_ENABLE         1
#define DEFAULT_Y_CHANNEL_ENABLE     1

#define MAX_POCKELS_LINE_BLANKING_PERCENTAGE  49

#define MAX_SCANAREA_ANGLE           M_PI
#define MIN_SCANAREA_ANGLE           -M_PI
#define DEFAULT_SCANAREA_ANGLE       0


#define MIN_INTERNALCLOCKRATE 625000    //625K 
#define	MAX_INTERNALCLOCKRATE 160000000 //160M
#define	DEFAULT_INTERNALCLOCKRATE 160000000 //160M

#define MIN_EXTCLOCKRATE 10000 //10KSPS
#define MAX_EXTCLOCKRATE 125000000 //125MSPS
#define DEFAULT_EXTCLOCKRATE 1000000 //1MS/s

#define MIN_3PCLOCKRATE 0
#define MAX_3PCLOCKRATE 4000000 //4MSPS

#define POCKELS_CALIBRATION_FAILED  -1

/// ThorDAQ Global Params

#define DEFAULT_CARD_NUMBER          0 //First Borad Connected

#define MIN_FINE_FIELD_SCALE     0.8
#define DEFALUT_FINE_FIELD_SCALE     1
#define MAX_FINE_FIELD_SCALE     1.2

//Average Mode, Frame Average/ Line Average/ Pixel Average
enum AverageMode
{
	NO_AVERAGE = 0,
	FRM_CUMULATIVE_MOVING,
	FRM_SIMPLE_MOVING,
	LINE_AVERAGE,
	LINE_INTEGRATION,
	PIXEL_AVERAGE,
	PIXEL_INTEGRATION,

};

// Shutter action
enum
{
	CLOSE_SHUTTER = 0,
	OPEN_SHUTTER = 1
};

// Channel Polarity
enum
{
	POL_NEG = 0,
	POL_POS = 1
};

#endif