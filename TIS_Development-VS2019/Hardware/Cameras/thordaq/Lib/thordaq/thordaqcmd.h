#ifndef ___THORDAQCMD_H___
#define ___THORDAQCMD_H___
#include "thordaqres.h"
//#include "..\..\..\..\..\Common\ThorSharedTypes\ThorSharedTypes\SharedEnums.cs"
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

#define THORDAQ_FIRMWARE_VERSION_RELEASE_MIN 0x20230414
#define THORDAQ_FIRMWARE_VERSION_RELEASE_MAJOR_MIN 0x2
#define THORDAQ_FIRMWARE_VERSION_RELEASE_MAJOR_BIT_SELECT 0xF0000000
#define THORDAQ_FIRMWARE_VERSION_RELEASE_MINOR_MIN 0x0
#define THORDAQ_FIRMWARE_VERSION_RELEASE_MINOR_BIT_SELECT 0x0F000000
#define THORDAQ_FIRMWARE_VERSION_RELEASE_DATE_MIN 0x230414
#define THORDAQ_FIRMWARE_VERSION_RELEASE_DATE_BIT_SELECT 0x00FFFFFF

#define THORDAQ_FIRMWARE_VERSION_DEBUG_MIN 0x50
#define THORDAQ_FIRMWARE_VERSION_DEBUG_MAX 0x100

#define MAX_FRAME_NUM                           0x7FFFFFFFUL
#define US_TO_S									1000000.0 //microseconds to seconds conversion

//
// THordaq Board System-level parameters 
//
#define SYS_CLOCK_FREQ						200000000				//System Clock
#define DAC_RESOLUTION                      0.00030517578125		//20.0/65536.0     (16bits register)
#define GALVO_RESOLUTION                    DAC_RESOLUTION		//20.0/65536.0     (16bits register)
#define GALVO_RESOLUTION_MAX                20000.0                    //20 full range 1ms 
#define DAC_RESOLUTION					    0.00030517578125		//20.0/65536.0     (16bits register)
#define DAC_MIN_VOLTS					    -10.0
#define DAC_MAX_VOLTS					    10.0
#define VOLT_TO_THORDAQ_VAL					65535.0 / 20.0
#define DAC_MAX_UPDATERATE                  1000000.0
#define DAC_DISABLED_CHANNEL_UPDATERATE_CONTINUOUS_MODE    4000.0
#define DAC_DISABLED_CHANNEL_UPDATERATE		4000.0 //45000.0		//ceil(SYS_CLOCK_FREQ/(USHRT_MAX + 1)) floor(SYS_CLOCK_FREQ/(0x7ffc - 1))//
#define DAC_MIN_UPDATERATE                  3052.0		//ceil(SYS_CLOCK_FREQ/(USHRT_MAX + 1))

#define CrsFrequencyHighPrecision				 7931.0
#define MAX_CHANNEL_COUNT                        4
#define WAVETABLE_CHANNEL_COUNT                        14 //includes 2 digital output channels, despite the dac name
#define DAC_ANALOG_CHANNEL_COUNT                 12
#define DAC_TRANSMIT_BUFFER_MAX                  262140 //65528 * 4
#define DAC_MAX_BUFFER_CHANNEL                   268435456//65536 * 4096
#define DAC_FIFO_DEPTH                           4096 // bytes
#define DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD0	 0.0004// minimum time for any descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_SECONDS_LAST_DESC_DYNAMIC_LOAD0	 0.0004// minimum time for the last descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD1	 0.0013// minimum time for any descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_SECONDS_LAST_DESC_DYNAMIC_LOAD1	 0.0013 // minimum time for the last descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD2	 0.002// minimum time for any descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_SECONDS_LAST_DESC_DYNAMIC_LOAD2	 0.002 // minimum time for the last descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD3	 0.004// minimum time for any descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_SECONDS_LAST_DESC_DYNAMIC_LOAD3	 0.004 // minimum time for the last descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD4	 0.006// minimum time for any descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_SECONDS_LAST_DESC_DYNAMIC_LOAD4	 0.006 // minimum time for the last descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD5	 0.010// minimum time for any descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_SECONDS_LAST_DESC_DYNAMIC_LOAD5	 0.010 // minimum time for the last descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD6	 0.012// minimum time for any descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_SECONDS_LAST_DESC_DYNAMIC_LOAD6	 0.012 // minimum time for the last descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD7	 0.016// minimum time for any descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_SECONDS_LAST_DESC_DYNAMIC_LOAD7	 0.016 // minimum time for the last descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_SECONDS_PER_DESC_DYNAMIC_LOAD8	 0.020// minimum time for any descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_SECONDS_LAST_DESC_DYNAMIC_LOAD8	 0.020 // minimum time for the last descriptor of the loaded waveform, calculated multiplying samples * udpate rate
#define DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD0   	 700000 // update rate used to decide which time per desc to use
#define DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD1   	 500000 // update rate used to decide which time per desc to use
#define DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD2   	 200000 // update rate used to decide which time per desc to use
#define DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD3   	 100000 // update rate used to decide which time per desc to use
#define DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD4   	 75000 // update rate used to decide which time per desc to use
#define DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD5   	 50000 // update rate used to decide which time per desc to use
#define DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD6   	 35000 // update rate used to decide which time per desc to use
#define DAC_MIN_UPDATE_RATE_DYNAMIC_LOAD7   	 30000 // update rate used to decide which time per desc to use

#define DAC_MIN_SAMPLES_END_DESC                 400// TODO: this number should be tested to cover the full range of dac rep rates, perhaps it should be milliseconds instead of number of samples
#define DAC_MIN_SAMPLES_DUMMY_WAVEFORM           2//  ushort samples
#define DAC_DESC_SAMPLE_COUNT_RESOLUTION         2 // ushort samples
#define DAC_DESC_STATIC_LOAD_MIN_SAMPLE_PER_DESC 20 // ushort samples TODO: need to see if lower is ok

#define DAC_DESC_STATIC_LOAD_MIN_SAMPLE_FOR_LAST_DESC 40 // ushort samples
#define DAC_DESC_STATIC_LOAD_MIN_SAMPLES		 DAC_DESC_STATIC_LOAD_MIN_SAMPLE_PER_DESC + DAC_DESC_STATIC_LOAD_MIN_SAMPLE_FOR_LAST_DESC // ushort samples TODO: need to see if lower is ok
#define DAC_NXT_DESC_OFFSET						 51
#define DAC_TOTAL_BYTES_TO_SEND_OFFSET			 35
#define DAC_LOOP_OFFSET							 34
#define DACLOOP_TOTAL_LOOP_COUNT_OFFSET			 32
#define DACLOOP_LOOP_START_DESC_OFFSET			 51
#define DAC_MAX_DESCRIPTORS_PER_INTERRUPT		 255
#define DAC_MIN_DESC_TIME						 1000 //us

#define DAC_DESC_MAX_LOOP_COUNT					 16384 // ushort samples
#define DAC_MAX_DESC_BEFORE_INTERRUPT			 255
#define DAC_STATIC_DESC_PER_INTERRUPT			 30
#define DAC_STATIC_EXTRA_DESC_TO_WAIT			 120

#define DAC_BANK_SWITCH_FRAME_COUNT_OFFSET		 16
#define DAC_WAVEPLAY_IRQ_REARM_OFFSET			 29
#define DAC_COUNTINUOUS_PLAYBACK_ENABLE_OFFSET	 30
#define DAC_BANK_SWITCH_ENABLE_OFFSET			 31

#define RDivideLaserFreqLimit	1.25		//Hz
#define BOB_3U_DIO_AVAILABLE_BNCS 32
// Filter const
#define FIR_FILTER_COUNT			2
#define FIR_FILTER_TAP_COUNT		16
#define MIN_TAP_INDEX 0
#define MAX_TAP_INDEX 15
#define MIN_FIR_TAP_COEFFICIENT -2
#define MAX_FIR_TAP_COEFFICIENT 1.9998779296875
#define ADC_MAX_SAMPLE_RATE         160000000.0
#define FIFO_DELAY_SAMPLES 4 
#define FIRST_FIR_FILTER	0
#define SECOND_FIR_FILTER	1


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
//#define DI_ResonantScanner_Line_Feedback              0x00
//#define DI_External_Line_Trigger                      0x01                
//#define DI_External_Pixel_Clock                       0x02
//#define DO_ScanLine_Direction						  0x03
//#define DO_Horizontal_Line_Trigger_Pulse			  0x04
//#define DO_Pixel_Integration_Interval				  0x05
//#define DO_Internal_SOF_Trigger					      0x06  //pulse at start of frame (10us)
//#define DI_External_Frame_Retrigger                   0x07
//#define DI_External_SOF_Trigger                       0x08
//#define DO_Pixel_Clock                                0x09
//#define DO_0_WaveMUX								  0x0A	//DO_0
//#define DO_1_WaveMUX		                          0x0B	//DO_1
//#define DO_2_WaveMUX					              0x0C	//DO_2
//#define DO_3_WaveMUX								  0x0D	//DO_3
//#define DO_4_WaveMUX								  0x0E
//#define DO_5_WaveMUX                                  0x0F
//#define DO_6_WaveMUX                                  0x10
//#define DO_7_WaveMUX                                  0x11
//#define DO_8_WaveMUX                                  0x12
//#define DO_9_WaveMUX                                  0x13
//#define DO_10_WaveMUX                                 0x14
//#define DO_11_WaveMUX                                 0x15
//#define DO_12_WaveMUX                                 0x16
//#define DO_13_WaveMUX                                 0x17
//#define DO_14_WaveMUX                                 0x18
//#define DO_15_WaveMUX                                 0x19
//#define DO_Capture_Active							  0x1A
// unlike MUXes above which are exclusively read and written by FPGA, AUX_GPIOs are under software control

#define DO_Buffer_Ready								TD_DIO_MUXedSLAVE_PORTS::Aux_GPIO_0  //d27, Aux_GPIO MUX 0, I/O direction configured separately
//#define Aux_GPIO_0									  0x1B  // (Copy for general API)
//#define Aux_GPIO_1									  0x1C  // these appear in Config XML as "DO_Aux_GPIOn" or "DI_ ", indicating DIR to config
//#define Aux_GPIO_2									  0x1D
//#define Aux_GPIO_3									  0x1E
//#define Aux_GPIO_4									  0x1F  //d31
//#define DI_HW_Trigger_1								  0x20
//#define DI_HW_Trigger_2								  0x21
//#define DI_HW_Trigger_3								  0x22
//#define DI_HW_Trigger_4								  0x23
//#define BOB3U_GPIO									  0x30  // 48d "slow" general purpose GPIO D16-D31 on 3U Panel, DIR configured and I/O value through CPLD via I2C

//TODO: Bring this definitions below out of Thordaq, into a common folder where the ThorDAQIOXML files are

#define DO_StimActiveEnvelope_Out                     TD_DIO_MUXedSLAVE_PORTS::DO_0_WaveMUX // can't do frame trigger at the same time as stim active envelope
#define DO_StimCycleComplete_Out				      TD_DIO_MUXedSLAVE_PORTS::DO_1_WaveMUX // can't do line trigger at the same time as stim cycle complete
#define DO_StimCycleEnvelope_Out                      TD_DIO_MUXedSLAVE_PORTS::DO_2_WaveMUX
#define DO_StimPockelsDig_Out		 				  TD_DIO_MUXedSLAVE_PORTS::DO_3_WaveMUX
#define DO_StimEpochEnvelope_Out					  TD_DIO_MUXedSLAVE_PORTS::DO_4_WaveMUX
#define DO_StimIterationEnvelope_Out	              TD_DIO_MUXedSLAVE_PORTS::DO_5_WaveMUX
#define DO_StimPatternComplete_Out                    TD_DIO_MUXedSLAVE_PORTS::DO_6_WaveMUX
#define DO_StimPatternEnvelope_Out                    TD_DIO_MUXedSLAVE_PORTS::DO_7_WaveMUX

#define DO_Frame_Trigger_Out						  TD_DIO_MUXedSLAVE_PORTS::DO_8_WaveMUX
#define DO_Line_Trigger_Out		 					  TD_DIO_MUXedSLAVE_PORTS::DO_9_WaveMUX
#define DO_Pockels_Imaging_Dig_Out					  TD_DIO_MUXedSLAVE_PORTS::DO_11_WaveMUX


	// ThorDAQ Digital Output Lines Params
#define DO0_HIGH					0x0101;
#define DO1_HIGH					0x0202;
#define DO2_HIGH					0x0404;
#define DO3_HIGH					0x0808;
#define DO4_HIGH					0x1010;
#define DO5_HIGH					0x2020;
#define DO6_HIGH					0x4040;
#define DO7_HIGH					0x8080;

#define THORDAQ_DO_LOW								0x0;
#define FRAME_TRIGGER_OUT_HIGH						DO0_HIGH;
#define LINE_TRIGGER_OUT_HIGH						DO1_HIGH;
#define THORDAQ_DO_STIM_ACTIVE_ENVELOPE_HIGH		DO0_HIGH;
#define THORDAQ_DO_STIM_COMPLETE_HIGH				DO1_HIGH;
#define THORDAQ_DO_STIM_CYCLE_ENVELOPE_HIGH	 		DO2_HIGH;
#define THORDAQ_DO_POCKELS_DIG_HIGH					DO3_HIGH; //THORDAQ_DO_POCKELS_DIG_HIGH
#define THORDAQ_DO_STIM_EPOCH_ENVELOPE_HIGH			DO4_HIGH;
#define THORDAQ_DO_STIM_ITERATION_ENVELOPE_HIGH		DO5_HIGH;
#define THORDAQ_DO_STIM_PATTERN_COMPLETE_HIGH		DO6_HIGH;
#define THORDAQ_DO_STIM_PATTERN_ENVELOPE_HIGH		DO7_HIGH;

#define DIO_CHANNEL_COUNT			8

	enum DIO_CHANNEL : UCHAR
	{
		//CH0 = DO_0,
		//CH1 = DO_0,
		//CH2 = DO_0,
		//CH3 = DO_0,
		//CH4 = DO_0,
		//CH5 = DO_0,
		//CH6 = DO_0,
		//CH7 = DO_0
	//		CH0 = TD_DIO_MUXedSLAVE_PORTS::DO_Line_Trigger_Out,
	//		CH1 = TD_DIO_MUXedSLAVE_PORTS::DO_ScanLine_Direction,
	//	CH2 = DO_Frame_Trigger_Out,
	//	CH3 = DO_Buffer_Ready,
	//		CH4 = TD_DIO_MUXedSLAVE_PORTS::DI_HW_Trigger_1,
		//	CH5 = TD_DIO_MUXedSLAVE_PORTS::DI_ResonantScanner_Line_Feedback,
	//		CH6 = DO_Pockels_Imaging_Dig_Out,
	//		CH7 = TD_DIO_MUXedSLAVE_PORTS::DO_Capture_Active,

			//CH0 = DO_0,
			//CH1 = DO_1,
			//CH2 = DO_2,
			//CH3 = DO_3,
			//CH4 = DO_4,
			//CH5 = DO_5,
			//CH6 = DO_6,
			//CH7 = DO_7
	};

	// DIO_LINES are the application specific defines for digital signals that must have FPGA GPIO MUX defines;
	// these FPGA defines are in ThorSharedTypes "SharedEnums.cs" TD_DIO_MUXedSLAVE_PORTS, and C++ code 
	// can copy them to other defines as desired.
	enum DIO_LINES : UINT8
	{
		FIRST_LINE = 0,
		DI_ImagingHardwareTrigger1 = 0,
		DI_ImagingHardwareTrigger2,
		DI_ImagingInternalDigitalTrigger,
		DI_StimHardwareTrigger1,
		DI_StimHardwareTrigger2,
		DI_StimInternalDigitalTrigger,
		DI_ResonantScannerLineClock,
		DO_GRFrameTrigger,
		DO_GRPockelsDigitalLine,
		DO_GGFrameTrigger,
		DO_GGLineTrigger,
		DO_GGPockelsDigitalLine,
		DO_CaptureActive,
		DO_BufferReady,
		DO_StimPockelsDigitalLine,
		DO_StimActiveEnvelope,
		DO_StimComplete,
		DO_StimCycleEnvelope,
		DO_StimIterationEnvelope,
		DO_StimEpochEnvelope,
		DO_StimPatterComplete,
		DO_StimPatternEnvelope,
		DO_InternalLinetrigger,
		DO_InternalFrameTrigger,
		DO_InternalPixelClock,
		DO_InternalLineClock,
		LAST_LINE
	};


	enum ADC_GAIN : ULONG
	{
		ADC_OFF = 0,
		db1 = 8,
		db5,
		db9,
		db13,
		db17,
		db21,
		db25,
		db29,
		ADC_GAIN_LAST
	};

	// using shadow register defines, we map both Legacy BOB and 3U Panel BOB so that DAC BNC index 0-11 directly corresponds to WAVETABLE channel 0-11
	enum WAVETABLE_CHANNEL : ULONG  // was "DAC_CHANNEL::DAC_DO0" for instance; now conceptually coded as "source::destination"
	{
		DAC_AO0 = 0,  // Destination is hardware TI 8812 16-bit DAC chip
		DAC_AO1 = 1,
		DAC_AO2 = 2,
		DAC_AO3 = 3,
		DAC_AO4 = 4,
		DAC_AO5 = 5,
		DAC_AO6 = 6,
		DAC_AO7 = 7,
		DAC_AO8 = 8,
		DAC_AO9 = 9,
		DAC_AO10 = 10, // Destination is hardware TI 7822 12-bit DAC chip
		DAC_AO11 = 11,
		DIG_D0to7 = 12,  // destination is FPGA's DIGital MUX which leads to a final DBB1 BNC Dn terminal
		DIG_D8to15 = 13,
	};

	enum MovingAverageFilterLength : BYTE
	{
		MA_16 = 0x0,
		MA_32 = 0x1,
		MA_64 = 0x2,
		MA_128 = 0x3,
		MA_256 = 0x4,
		MA_512 = 0x5,
		MA_1024 = 0x6,
		MA_2048 = 0x7
	};




	//
	// Error Code
	//
	/* API Return Code Values */
	typedef enum _THORDAQ_STATUS
	{
		// Return status defines
		// The operation completed successfully.
		STATUS_SUCCESSFUL = 0L,
		// Borad Software in initiated unsuccessfully.
		STATUS_INITIATION_INCOMPLETE = 1L,
		// No Hardware is installed.
		STATUS_DEVICE_NOT_EXISTS = 2L,
		// Board number does not exist
		STATUS_INVALID_BOARDNUM = 3L,
		// Retrieve Board device information error 
		STATUS_GET_BOARD_CONFIG_ERROR = 4L,
		// ReadWrite registers unsuccessfully
		STATUS_READWRITE_REGISTER_ERROR = 5L,
		// Set up DMA Packet Mode unsuccessfully
		STATUS_SETUP_PACKET_MODE_INCOMPLETE = 6L,
		// Release DMA Packet Mode unsuccessfully
		STATUS_RELEASE_PACKET_MODE_INCOMPLETE = 7L,
		// Write buffer data to on-board DRAM unsuccessfully
		STATUS_WRITE_BUFFER_ERROR = 8L,
		// Read buffer data from on-board DRAM unsuccessfully
		STATUS_READ_BUFFER_ERROR = 9L,
		// Read buffer data timeout error
		STATUS_READ_BUFFER_TIMEOUT_ERROR = 10L,
		// Set up Acquisition settings unsuccessfully
		STATUS_PARAMETER_SETTINGS_ERROR = 11L,
		// Overflow 
		STATUS_OVERFLOW = 12L,
		// NWL code
		STATUS_INVALID_MODE = 13L,
		// NWL code
		STATUS_INCOMPLETE = 14L,
		STATUS_I2C_INVALIDMUX = 15L,
		STATUS_I2C_INVALIDDEV = 16L,
		STATUS_I2C_TIMEOUT_ERR = 17L,
		//acquistion aborted
		STATUS_ACQUISITION_ABORTED = 18L,

		//DAC waveform
		STATUS_DAC_TOO_FEW_SAMPLES = 19L,

		STATUS_DAC_UPDATE_RATE_OUT_OF_RANGE = 20L,
		STATUS_FUNCTION_NOT_SUPPORTED = 21L
	}THORDAQ_STATUS;

	enum SCAN_MODES : UCHAR
	{
		UNIDIRECTION_SCAN = 0x00,
		BIDIRECTION_SCAN = 0x01
	};

	enum SCAN_DIRECTION : UCHAR
	{
		FORWARD_SC = 0x00, //TODO: change back to regunar name ending in SCAN
		REVERSE_SC = 0x01
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

	enum THORDAQ_TRIGGER_MODES : UCHAR
	{
		THORDAQ_SOFTWARE_START = 0x00,
		THORDAQ_HARDWARE_TRIGGER
	};

	enum THORDAQ_HW_TRIGGER_MODES : UCHAR
	{
		THORDAQ_NO_TRRIGER,
		THORDAQ_HARDWARE_EDGE_TRIGGER_MODE,
		THORDAQ_HARDWARE_LEVEL_TRIGGER_MODE,
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

	enum FIRFilterMode : UCHAR
	{
		INTERNAL_CLOCK_MODE = 0x00,
		EXTERNAL_CLOCK_MODE = 0x01,
		THREE_PHOTON_MODE = 0x04,
	};

	enum THORDAQ_TRIGGER_LOGIC_OPERANDS : UCHAR
	{
		THORDAQ_OR,
		THORDAQ_AND,
		//THORDAQ_NOR,
		//THORDAQ_XOR,
		//THORDAQ_XNOR,
		//THORDAQ_NAND,
	};

	enum THORDAQ_TRIGGER_INPUTS : UCHAR
	{
		THORDAQ_HW_TRIGGER_1 = 0x0,
		THORDAQ_HW_TRIGGER_2 = 0x1,
		THORDAQ_HW_TRIGGER_3 = 0x2,
		THORDAQ_HW_TRIGGER_4 = 0x3
	};

	enum THORDAQ_BOB_TYPE : UCHAR
	{
		TDQ_LEGACY_BOB,
		TDQ_3U_BOB
	};

	/// port index

	typedef struct _GLOBAL_IMAGE_CTRL_STRUCT
	{
		SYSTEM_MODE	    system_mode;
		ULONG32         clockRate; // clock rate depends on clock source is external or internal. In External mode clock, rate pairs with external laser clock. Internal Mode, rate is always be 160MSPS 
		ULONG32         clockReference;
		CLOCK_SOURCE    clock_source;
		USHORT			channel;
		USHORT			imgHSize;
		USHORT			imgVSize;
		SCAN_MODES		scanMode;
		SCAN_DIRECTION  scanDir;
		USHORT			alignmentOffset;
		ULONG32			frameCnt;
		ULONG32			frameNumPerSec;
		ULONG32			frameNumPerTransfer;
		double			frameRate;
		ULONG32         defaultMode;
		ULONG32         threePhotonMode;
		ULONG32         threePhotonPhaseAlignment[MAX_CHANNEL_COUNT];
		ULONG32			ADCGain[MAX_CHANNEL_COUNT];
		UINT8			numPlanes;
		bool			TwoBankDACDMAPlayback;
		bool			dacSlowMoveToOffset;
		vector<string>			digitalLinesConfig;
		bool			isPreviewImaging;
		bool			ddsEnable;
		ULONG32         enableDownsampleRateChange;
		ULONG32         downSampleRate;
		bool movingAverageFilterEnable;
		double movingAverageMultiplier;
	}GLOBAL_IMAGE_CTRL_STRUCT, * PGLOBAL_IMAGE_CTRL_STRUCT;

	typedef struct _STREAM_PROCESSING_CTRL_STRUCT
	{
		double scan_period;
		ULONG32 fir_filter_enabled;
		double fir_filter[FIR_FILTER_COUNT][MAX_CHANNEL_COUNT][FIR_FILTER_TAP_COUNT];
		ULONG32 channel_multiplexing_enabled;
	}STREAM_PROCESSING_CTRL_STRUCT, * PSTREAM_PROCESSING_CTRL_STRUCT;

	typedef struct _THORDAQ_TRIGGER_SETTINGS
	{
		THORDAQ_TRIGGER_MODES		triggerMode;
		THORDAQ_TRIGGER_INPUTS		hwTrigger1Selection;
		THORDAQ_TRIGGER_INPUTS		hwTrigger2Selection;
		THORDAQ_HW_TRIGGER_MODES	hwTrigger1Mode;
		THORDAQ_HW_TRIGGER_MODES	hwTrigger2Mode;
		BYTE						internalDigitalTrigger; 
		bool						enableInternalDigitalTrigger;
		bool						trigger2RetriggerEnable;		
		THORDAQ_TRIGGER_LOGIC_OPERANDS logicOperand;
	}THORDAQ_TRIGGER_SETTINGS, * PTHORDAQ_TRIGGER_SETTINGS;

	typedef struct _COHERENT_SAMPLING_CTRL_STRUCT
	{
		USHORT phaseIncrementMode;
		USHORT phaseOffset;
		USHORT phaseStep;
		USHORT phaseLimit;
	}COHERENT_SAMPLING_CTRL_STRUCT, * PCOHERENT_SAMPLING_CTRL_STRUCT;

	typedef struct _RESONANT_GALVO_CTRL_STRUCT
	{
		double flybackTime;
		double preSOFTime;
	}RESONANT_GALVO_CTRL_STRUCT, * PRESONANT_GALVO_CTRL_STRUCT;

	typedef struct _GALVO_GALVO_CTRL_STRUCT
	{
		double dwellTime; //unit: s
		double pixelDelayCnt;
		double turnaroundTime;  //unit: s
		double pureTurnAroundTime;  //unit: s
		double flybackTime;
		double preSOFTime;
		double flybackCycle;
		double lineTime;
		bool fastOneWayImaging;
		long sampleOffsetStartLUT3PTI;
	}GALVO_GALVO_CTRL_STRUCT, * PGALVO_GALVO_CTRL_STRUCT;

	typedef struct _DAC_WAVEFORM_CRTL_STRUCT
	{
		ULONG64 output_port;
		double offset_val; //-10v to 10v
		double park_val; //-10v to 10v
		double update_rate; //max is 1MSPS
		ULONG64 flyback_samples;
		ULONG64 waveform_buffer_size;
		USHORT* waveformBuffer;
		bool enableWaveformFilter;
		bool filterInhibit;
		bool hSync;
		bool enableEOFFreeze;
	}DAC_WAVEFORM_CRTL_STRUCT, * PDAC_WAVEFORM_CRTL_STRUCT;

	typedef struct _DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT
	{
		ULONG64 output_port;
		UINT16  offset; //this offset is an unsgined 16bit represantation of the -10 - 10v range. -10v = 0, 10v = 65535
		UINT16  park; //this park is an unsgined 16bit represantation of the -10 - 10v range. -10v = 0, 10v = 65535
		bool filterInhibit;
		double update_rate;
		ULONG64 waveform_buffer_size;
		USHORT* waveformBuffer;
		THORDAQ_TRIGGER_SETTINGS  triggerSettings;
		bool enablePort;
		bool offsetTheWaveforms;
	}DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT, * PDAC_WAVEFORM_PLAYBACK_CRTL_STRUCT;

	typedef struct _IMAGING_CONFIGURATION_STRUCT
	{
		GLOBAL_IMAGE_CTRL_STRUCT imageCtrl;
		STREAM_PROCESSING_CTRL_STRUCT streamingCtrl;
		COHERENT_SAMPLING_CTRL_STRUCT coherentSamplingCtrl;
		GALVO_GALVO_CTRL_STRUCT galvoGalvoCtrl;
		RESONANT_GALVO_CTRL_STRUCT resonantGalvoCtrl;
		std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT> dacCtrl;
		std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT> dacCtrl2; // use this one for 2bank acquisitions to set the second bank at the beggining
		THORDAQ_TRIGGER_SETTINGS  triggerSettings;
	}IMAGING_CONFIGURATION_STRUCT, * PIMAGING_CONFIGURATION_STRUCT;

	// dFLIM_4002 legacy defines
	enum TRIGGER_MODE : UCHAR
	{
		SOFTWARE_RUN_MODE = 0x01,
		HARDWARE_TRIGGER_MODE = 0x00
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

	enum ACQUISITION_MODE : ULONG32
	{
		DFLIM = 0,
		Diagnostics = 1
	};
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
	}DAC_CRTL_STRUCT, * PDAC_CRTL_STRUCT;
#define DAC_CHANNEL_COUNT                        13

	typedef struct _dFLIM_GLOBAL_IMAGE_CTRL_STRUCT
	{
		SYSTEM_MODE	    system_mode;
		ULONG32         clockRate; // clock rate depends on clock source is external or internal. In External mode clock, rate pairs with external laser clock. Internal Mode, rate is always be 160MSPS 
		CLOCK_SOURCE    clock_source;
		USHORT			channel;
		USHORT			imgHSize;
		USHORT			imgVSize;
		ULONG32			dataHSize;
		USHORT			linesPerStripe;
		SCAN_MODES		scanMode;
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
	} dFLIM_GLOBAL_IMAGE_CTRL_STRUCT, * dFLIM_PGLOBAL_IMAGE_CTRL_STRUCT;
	typedef struct _dFLIM_IMAGING_CONFIGURATION_STRUCT
	{
		dFLIM_GLOBAL_IMAGE_CTRL_STRUCT imageCtrl;
		STREAM_PROCESSING_CTRL_STRUCT streamingCtrl;
		COHERENT_SAMPLING_CTRL_STRUCT coherentSamplingCtrl;
		GALVO_GALVO_CTRL_STRUCT galvoGalvoCtrl;
		RESONANT_GALVO_CTRL_STRUCT resonantGalvoCtrl;
		DAC_CRTL_STRUCT dacCtrl[DAC_CHANNEL_COUNT];
	} dFLIM_IMAGING_CONFIGURATION_STRUCT, * PdFLIM_IMAGING_CONFIGURATION_STRUCT;
	// END dFLIM_4002 legacy define


	typedef struct _DAC_FREERUN_WAVEFORM_GENERAL_SETTINGS
	{
		ULONG32 numberOfCycles;
		ULONG64 numberOfSamplesPerCycle; //TODO: this should probably be set on a perchannel basis
		bool allowTimeToMoveToOffset;
		vector<string> digitalLinesConfig;
	}DAC_FREERUN_WAVEFORM_GENERAL_SETTINGS, * PDAC_FREERUN_WAVEFORM_GENERAL_SETTINGS;

	typedef struct _DAC_FREERUN_WAVEFORM_CONFIG
	{
		std::map<UINT, DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT> dacCtrl;
		std::map<UINT, DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT> dacCtrlPart2; ///<use for static load only (when the whole waveform is preloaded) this is the last part of the waveform
		DAC_FREERUN_WAVEFORM_GENERAL_SETTINGS generalSettings;
	}DAC_FREERUN_WAVEFORM_CONFIG, * PDAC_FREERUN_WAVEFORM_CONFIG;

	typedef struct _IMAGING_BUFFER_STRUCT
	{
		ULONG   channel;        // Channel Index
		UCHAR* buffer;         // Data buffer
		ULONGLONG offset;       // Offset in data buffer to start transfer
		ULONGLONG length;       // Byte length of transfer
	}IMAGING_BUFFER_STRUCT, * PIMAGING_BUFFER_STRUCT;


	// DZ SDK //////////////////////////////

	typedef struct _RegFieldDesc
	{
		INT32	BAR;
		INT32	BARoffset;
		INT32   RegisterByteSize;   // byte length (1-8)
		INT32	BitFieldOffset; // -1 if REGISTER reference
		INT32   BitFieldLen;    // only for BitField
		// read/write registers have WO/RO both false
		// we can have the same Register address (e.g. 0x142)
		// as WO and RO
		// Simplify .NET array passing with same basic types
		INT32    bWriteOnly;     // "read" give shadow register
		INT32    bReadOnly;      // read from hardware
	} REG_FIELD_DESC, * PREG_FIELD_DESC;

	typedef struct _S2MM_Config
	{
		INT32 ChannelMask;  // e.g. 0x6 for channels 1&2, 0xF for chan 0-3
		INT32 HSize;
		INT32 VSize;
		INT32 NumberOfDescriptorsPerBank; // a.k.a. # of frames (per bank)
		INT32 DDR3imageStartingAddress; // physical address in DDR3 for separate DMA to host
		INT32 DDR3endingAddress;   // returned by S2MM DMA setup
	} S2MM_CONFIG, * PS2MM_CONFIG;

	typedef struct _S2MM_ADCSAMPLE_LUT
	{
		UINT16 ADCsampleLUTchan0[SCAN_LUT_MAX_LEN];
		UINT16 ADCsampleLUTchan1[SCAN_LUT_MAX_LEN];
		UINT16 ADCsampleLUTchan2[SCAN_LUT_MAX_LEN];
		UINT16 ADCsampleLUTchan3[SCAN_LUT_MAX_LEN];
	} S2MM_ADCSAMPLE_LUT, * PS2MM_ADCSAMPLE_LUT;
	////////////////////////////////////////

#define CTHORDAQCDECL			 __cdecl
#define CTHORDAQCALLBACK        CTHORDAQCDECL
	typedef void(CTHORDAQCALLBACK* ThorDAQDACCycleDoneCallbackPtr)(UINT8 dacChannel, THORDAQ_STATUS status, void* callbackData);
	typedef void(CTHORDAQCALLBACK* ThorDAQDACWaveformPlaybackCompleteCallbackPtr)(THORDAQ_STATUS status, void* callbackData);
	typedef void(CTHORDAQCALLBACK* ThorDAQDACApproachingNSamplesCallbackPtr)(UINT8 dacChannel, UINT32 numberSample, THORDAQ_STATUS status, void* callbackData);
	typedef void(CTHORDAQCALLBACK* ThorDAQDACApproachingLoadedWaveformEndCallbackPtr)(UINT8 dacChannel, THORDAQ_STATUS status, void* callbackData);
	typedef void(CTHORDAQCALLBACK* ThorDAQDACWaveformPlaybackStartedCallbackPtr)(THORDAQ_STATUS status, void* callbackData);
	typedef void(CTHORDAQCALLBACK* ThorDAQDACBankSwitchingReadyForNextWaveformCallbackPtr)(THORDAQ_STATUS status, void* callbackData);
#ifdef __cplusplus
}
#endif

#endif // ___THORDAQCMD_H___