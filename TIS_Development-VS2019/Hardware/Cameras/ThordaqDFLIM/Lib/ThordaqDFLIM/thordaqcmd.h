#ifndef ___THORDAQCMD_H___
#define ___THORDAQCMD_H___

#ifdef __cplusplus
	extern "C" 
	{
#endif

#ifndef TRUE
 #define TRUE            (1L)
#endif
#ifndef FALSE
 #define FALSE           (0L)
#endif
#ifndef NULL
 #define NULL            (0L)
#endif

//*****************************************************************************
//
//	Constants Used With The Command Set Listed Above
//
//*****************************************************************************

#define MAX_FRAME_NUM                           0x7FFFFFFFUL
#define US_TO_S									1000000.0 //microseconds to seconds conversion

//
// THordaq Board System-level parameters 
//
#define SYS_CLOCK_FREQ						200000000				//System Clock
#define GALVO_RESOLUTION                    0.00030517578125		//20.0/65536.0     (16bits register)
#define GALVO_RESOLUTION_MAX                20000.0                    //20 full range 1ms 

#define DAC_MAX_UPDATERATE                  1000000.0
#define DAC_MIN_UPDATERATE                  SYS_CLOCK_FREQ/USHRT_MAX

#define CrsFrequencyHighPrecision				 7931.0
#define MAX_CHANNEL_COUNT                        4
#define DAC_CHANNEL_COUNT                        13
#define DAC_TRANSMIT_BUFFER_MAX                  65528
#define DAC_MAX_BUFFER_CHANNEL                   268435456//65536 * 4096
#define DAC_FIFO_DEPTH                           4096 // bytes

// Filter const
#define FITLER_MAX_TAP              16
#define ADC_MAX_SAMPLE_RATE         160000000.0
#define FIFO_DELAY_SAMPLES 4 

//
// Channel Selection
//
#define CHANNEL_ALL				0x000000FFUL
#define CHANNEL_A				0x00000001UL
#define CHANNEL_B				0x00000002UL
#define CHANNEL_C				0x00000004UL
#define CHANNEL_D				0x00000008UL
#define CHANNEL_E				0x00000010UL
#define CHANNEL_F				0x00000020UL
#define CHANNEL_G				0x00000040UL
#define CHANNEL_H				0x00000080UL

//
// GPIO Selection
//
#define DI_ResonantScanner_Line_Clock                 0x00
#define DI_External_Line_Clock                        0x01                
#define DI_External_Pixel_Clock                       0x02
#define DO_Line_Clock                                 0x03
#define DO_Line_trigger                               0x04
#define DO_PixelInterval_Clock                        0x05
#define DO_Frame_Trigger                              0x06
#define DI_Hardware_Trigger                           0x07
#define DI_Frame_Trigger                              0x08
#define DO_Pixel_Clock                                0x09
#define DO_Frame_Trigger_Out                          0x0A
#define DO_1                                          0x0B
#define DO_2                                          0x0C
#define DO_3                                          0x0D
#define DO_4                                          0x0E
#define DO_5                                          0x0F
#define DO_6                                          0x10
#define DO_7                                          0x11

enum ADC_CHANNEL : UCHAR
{
	CH0 = DO_Line_trigger,
	CH1 = DO_Line_Clock,
	CH2 = DO_Frame_Trigger_Out,
	CH3 = DO_Pixel_Clock,
	CH4 = DI_Hardware_Trigger,
	CH5 = DI_ResonantScanner_Line_Clock,
	CH6 = DI_Frame_Trigger,
	CH7 = DI_External_Line_Clock,
};

enum ADC_GAIN : ULONG
{
	noGain = 0,
	db1 = 8,
	db5,
	db9,
	db13,
	db17,
	db21,
	db25,
	db29
};

enum ACQUISITION_MODE : ULONG32
{
	DFLIM = 0,
	Diagnostics = 1
};

enum DAC_CHANNEL : ULONG
{
	GalvoResonantY = 0,
	GalvoGalvoX = 1,
	GalvoGalvoY = 2,
	Pockel1 = 3,
	Pockel2 = 4,
	Pockel3 = 5,
	Pockel4 = 6,
	b1 = 7,
	b2 = 8,
	b3 = 9,
	b4 = 10,
	b5 = 11,
	DO0 = 12
};
//
// Error Code
//
/* API Return Code Values */
typedef enum _THORDAQ_STATUS
{
    // Return status defines
    // The operation completed successfully.
    STATUS_SUCCESSFUL		                        =   	    0L,
	// Borad Software in initiated unsuccessfully.
	STATUS_INITIATION_INCOMPLETE                    =           1L,
    // No Hardware is installed.
    STATUS_DEVICE_NOT_EXISTS                        =           2L,
    // Board number does not exist
    STATUS_INVALID_BOARDNUM	                        =           3L, 
	// Retrieve Board device information error 
	STATUS_GET_BOARD_CONFIG_ERROR                   =           4L,
	// ReadWrite registers unsuccessfully
	STATUS_READWRITE_REGISTER_ERROR                 =           5L,
	// Set up DMA Packet Mode unsuccessfully
	STATUS_SETUP_PACKET_MODE_INCOMPLETE             =           6L,	
	// Release DMA Packet Mode unsuccessfully
	STATUS_RELEASE_PACKET_MODE_INCOMPLETE           =           7L,
	// Write buffer data to on-board DRAM unsuccessfully
	STATUS_WRITE_BUFFER_ERROR                       =           8L,
	// Read buffer data from on-board DRAM unsuccessfully
	STATUS_READ_BUFFER_ERROR                        =           9L,
	// Read buffer data timeout error
	STATUS_READ_BUFFER_TIMEOUT_ERROR                =           10L,
	// Set up Acquisition settings unsuccessfully
	STATUS_PARAMETER_SETTINGS_ERROR                 =           11L,    
	// Overflow 
	STATUS_OVERFLOW	                                =           12L,	
}THORDAQ_STATUS;

enum SCAN_MODE : UCHAR
{
	UNIDIRECTION_SCAN = 0x00,
	BIDIRECTION_SCAN = 0x01
};

enum SCAN_DIRECTION : UCHAR
{
	FORWARD_SCAN = 0x00,
	REVERSE_SCAN = 0x01
};

enum SYSTEM_MODE : USHORT
{
	INTERNAL_RESONANT_GALVO = 0x0000,
	INTERNAL_GALVO_GALVO
};

enum CLOCK_SOURCE : ULONG32
{
	INTERNAL_80MHZ_REF = 0x00000001,
	EXTERNAL_CLOCK_SOURCE = 0x00000002,
};

enum TRIGGER_MODE: UCHAR
{
	SOFTWARE_RUN_MODE = 0x01,
	HARDWARE_TRIGGER_MODE =  0x00
};

enum channel_MDOE : USHORT
{
	channel_A = 0x0001,
	channel_B = 0x0002,
	channel_C = 0x0004,
	channel_D = 0x0008,
	channel_E = 0x0010,
	channel_F = 0x0020,
};

enum WAVEFORM_TYPE : UCHAR
{
	STATIC_OUTPUT = 0x00,
	SELF_DEFINED = 0x01,
	GALVO_RESONANT = 0x02,
	GALVO_GALVO_X = 0x03,
	GALVO_GALVO_Y = 0x04
};


/// port index

typedef struct _GLOBAL_IMAGE_CTRL_STRUCT
{
	SYSTEM_MODE	    system_mode;
	ULONG32         clockRate; // clock rate depends on clock source is external or internal. In External mode clock, rate pairs with external laser clock. Internal Mode, rate is always be 160MSPS 
	CLOCK_SOURCE    clock_source;
	USHORT			channel;
	USHORT			imgHSize;
	USHORT			imgVSize;
	ULONG32			dataHSize;
	USHORT			linesPerStripe;
	SCAN_MODE		scanMode;
	SCAN_DIRECTION  scanDir;
	USHORT			alignmentOffset;
	ULONG32			frameCnt;
	ULONG32			frameNumPerSec;
	ULONG32			frameNumPerTransfer;
	ULONG32         defaultMode;
	TRIGGER_MODE    triggerMode;
	ULONG32         threePhotonMode;
	ULONG32         threePhotonPhaseAlignment;
	ULONG32			acquisitionMode; //DFLIM mode = 0, Diagnostics mode = 1
}GLOBAL_IMAGE_CTRL_STRUCT, *PGLOBAL_IMAGE_CTRL_STRUCT;

typedef struct _STREAM_PROCESSING_CTRL_STRUCT
{
	double scan_period;
	ULONG32 fir_filter_enabled;
	ULONG32 channel_multiplexing_enabled;
}STREAM_PROCESSING_CTRL_STRUCT, *PSTREAM_PROCESSING_CTRL_STRUCT;

typedef struct _COHERENT_SAMPLING_CTRL_STRUCT
{
	USHORT phaseIncrementMode;
	USHORT phaseOffset;
	USHORT phaseStep;
	USHORT phaseLimit;
}COHERENT_SAMPLING_CTRL_STRUCT, *PCOHERENT_SAMPLING_CTRL_STRUCT;

typedef struct _RESONANT_GALVO_CTRL_STRUCT
{
	double flybackTime;
	double vGalvoAmpVal;
	double vGalvoParkVal;
	double vGalvoOffset;
}RESONANT_GALVO_CTRL_STRUCT, *PRESONANT_GALVO_CTRL_STRUCT;

typedef struct _GALVO_GALVO_CTRL_STRUCT
{
	double dwellTime; //unit: s
	double pixelDelayCnt;
	double turnaroundTime;  //unit: s
	double flybackTime;
	double flybackCycle;
	double lineTime;
}GALVO_GALVO_CTRL_STRUCT, *PGALVO_GALVO_CTRL_STRUCT;

typedef struct _DAC_CRTL_STRUCT
{
	ULONG64 output_port;
	double offset_val;
	double park_val;
	double update_rate;
	double amplitude;
	ULONG64 waveform_buffer_start_address;
	ULONG64 waveform_buffer_length;
	ULONG64 flyback_samples;
}DAC_CRTL_STRUCT, *PDAC_CRTL_STRUCT;

typedef struct _IMAGING_CONFIGURATION_STRUCT
{
	GLOBAL_IMAGE_CTRL_STRUCT imageCtrl;
	STREAM_PROCESSING_CTRL_STRUCT streamingCtrl;
	COHERENT_SAMPLING_CTRL_STRUCT coherentSamplingCtrl;
	GALVO_GALVO_CTRL_STRUCT galvoGalvoCtrl;
	RESONANT_GALVO_CTRL_STRUCT resonantGalvoCtrl;
	DAC_CRTL_STRUCT dacCtrl[DAC_CHANNEL_COUNT];
}IMAGING_CONFIGURATION_STRUCT, *PIMAGING_CONFIGURATION_STRUCT;


typedef struct _IMAGING_BUFFER_STRUCT
{
	ULONG   channel;        // Channel Index
	UCHAR*  buffer;         // Data buffer
	ULONGLONG offset;       // Offset in data buffer to start transfer
    ULONGLONG length;       // Byte length of transfer
}IMAGING_BUFFER_STRUCT, *PIMAGING_BUFFER_STRUCT;

#ifdef __cplusplus
	}
#endif

#endif // ___THORDAQCMD_H___