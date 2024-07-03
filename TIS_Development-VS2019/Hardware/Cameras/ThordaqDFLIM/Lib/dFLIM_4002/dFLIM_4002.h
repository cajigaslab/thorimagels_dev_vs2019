#pragma once
#include "stdafx.h"
#include "dFLIMcmd.h" // thordaqcmd.h
#include "dFLIMres.h" // thordaqres.h
#include "memorypool.h"
#include "thordaqconst.h"

#include <atomic>

#define MAXIMUM_NUMBER_OF_BOARDS 4 //
#define DFLIM_PROCESSING_STRUCTS_NUM 4

//
// Copied Macros from ntddk.h. Used to using the kernel-mode
// style of linked list.
//


inline double round(double num) {
    return (num > 0.0) ? floor(num + 0.5) : ceil(num - 0.5);
}

#define FPGAregisterWRITE(a,b)  (FPGAregisterWrite(a,(int)strlen(a),b))
#define FPGAregisterREAD(a,b)  (FPGAregisterRead(a,(int)strlen(a),b))

class DMADescriptor
{
public:
	ULONG64 next_node;
	ULONG64 length;
	ULONG64 buf_addr;
};

// DZ SDK //////////////
// Define structure for AXI-DMA Descriptiors (SWUG "Internal AXI-DMA Descriptors")
typedef struct _AXI_DMA_Desc
{
	//ULONG32 Rsvd         : 6;  // 0x00
	ULONG32 NextDescPtr : 32; // 26-bit field
	ULONG32 RsvdA : 32; // 0x04
	ULONG32 DDR3Address : 32; // 0x08
	ULONG32 RsvdB : 32; // 0x0C
	ULONG32 RsvdC : 24; // 0x10
	ULONG32 AwCACHE : 4;
	ULONG32 AwUSER : 4;
	ULONG32 Stride : 16; // 0x14 
	ULONG32 RsvdD : 3;
	ULONG32 VSIZE : 13;
	ULONG32 HSIZE : 16; // 0x18
	ULONG32 RsvdE : 16;

	ULONG32 TDest : 5;  // 0x1C   (Multi-Channel)
	ULONG32 RsvdF : 3;
	ULONG32 TID : 5;
	ULONG32 RsvdG : 3;
	ULONG32 TUser : 4;
	ULONG32 RsvdH : 6;
	ULONG32 RxEOP : 1;
	ULONG32 RxSOP : 1;
	ULONG32 IE : 1;
	ULONG32 SE : 1;
	ULONG32 DE : 1;
	ULONG32 Cmp : 1;
} AXI_DMA_DESCRIPTOR, * PAXI_DMA_DESCRIPTOR;



// SHADOW REGISTERS
// CRITICAL!!  These enumerations (i.e. the highest count) must match the objects created in the .cpp file
// a common mistake is to add or delete a register in the .cpp file and not properly update this enumeration list;
// symptom is exception-failure when trying to match index with a valid or invalid Shadow Register NAME
enum FPGARegisterIndex {
	TD_FPGA_DLL_VER = 0,
	NWL_DDR3_DMAcore,			// x4000 (BAR0)
	
	S2MM_DMA_Desc_Chan0,        // 0x0 (BAR1)
	S2MM_DMA_Desc_Chan1,        // 0x10000
	S2MM_DMA_Desc_Chan2,		// 0x20000
	S2MM_DMA_Desc_Chan3,		// 0x30000
	
	S2MMDMA_ControlReg1,         // 0x0 (BAR2)
	S2MMDMA_StatusReg1,          // 0x0  ReadOnly 
	S2MMDMA_ChainHead_Chan0,    // 0x2  WriteOnly
	S2MMDMA_ChainTail_Chan0,     // 0x4
	S2MMDMA_IRQthreshld_Chan0,   // 0x6

	S2MMDMA_ControlReg2,         // 0x40 (BAR2)
	S2MMDMA_StatusReg2,          // 0x40  ReadOnly 
	S2MMDMA_ChainHead_Chan1,    // 0x42  WriteOnly
	S2MMDMA_ChainTail_Chan1,     // 0x44
	S2MMDMA_IRQthreshld_Chan1,   // 0x46

	S2MMDMA_ControlReg3,         // 0x80 (BAR2)
	S2MMDMA_StatusReg3,          // 0x80  ReadOnly 
	S2MMDMA_ChainHead_Chan2,    // 0x82  WriteOnly
	S2MMDMA_ChainTail_Chan2,     // 0x84
	S2MMDMA_IRQthreshld_Chan2,   // 0x86

	S2MMDMA_ControlReg4,         // 0xC0 (BAR2)
	S2MMDMA_StatusReg4,          // 0xC0  ReadOnly 
	S2MMDMA_ChainHead_Chan3,    // 0xC2  WriteOnly
	S2MMDMA_ChainTail_Chan3,     // 0xC4
	S2MMDMA_IRQthreshld_Chan3,   // 0xC6

	GlobalControlReg,           // 0x0 (BAR3)
	GlobalFPGARevReg,           // 0x0  READ only
	FlashStatusReg,				 // 0x4	READ only (1 byte used in programming FLASH)
	GlobalReadData,				 // 0x6	READ only

	GlobalImageSyncControlReg,  // 0x8
	GlobalImageHSIZE,           // 0x10
//	GlobalImageAcqHSIZE,        // 0x12 - NOT in dFLIM
	GlobalImageVSIZE,           // 0x18  
	GlobalDBB1muxReg,           // 0x20 "GPIOConfig"
	FPGAConfigFlashFIFO,		// 0x38 FPGA flash programming (write-only)

	BeatsPerFrame_Ch0,			// 0x40
	IntraBeatDelay_HSize_Ch0,
	IntraBeatFrameDelay_Ch0,

	BeatsPerFrame_Ch1,			// 0x80
	IntraBeatDelay_HSize_Ch1,
	IntraBeatFrameDelay_Ch1,

	BeatsPerFrame_Ch2,			// 0xC0
	IntraBeatDelay_HSize_Ch2,
	IntraBeatFrameDelay_Ch2,

	BeatsPerFrame_Ch3,			// 0x100
	IntraBeatDelay_HSize_Ch3,
	IntraBeatFrameDelay_Ch3,

	ScanningSyncControlReg,     // 0x140
	ScanningResonantPeriodReadbackReg,     // 0x140 (READ only)
	ScanningFrameReadbackReg,   // 0x142 (READ only)
	ScanningFrameCount,                    // 0x142 Write only
	ADPLL_ControlReg,           // 0x148
	ADPLL_SyncDelay,            // 0x150
	ADPLL_PhaseOffset,         // 0x152
	ADPLL_DCO_CenterFreq,       // 0x158
	ScanningGalvoPixelDwell,    // 0x160
	ScanningGalvoPixelDelay,    // 0x168
	ScanningIntraLineDelay,     // 0x170    Intra_Line_Delay "turnaround"
	ScanningIntraFrameDelay,    // 0x178	Intra_Frame_Delay "flyback?"

	SamplingClockControlReg,    // 0x180 (WriteOnly)
	SamplingClockStatusReg,     // 0x180 (ReadOnly 1 byte)
	SamplingClock_LASER_CLK_CNT,// 0x181 (RO, 4 bytes)
	SamplingClock_CLKRX_RXBYTE, // 0x185 (RO, 1 byte)
	SamplingClockPhaseOffset,   // 0x188
	SamplingClockPhaseStep,     // 0x190 
	SamplingClockPhaseLimit,    // 0x198

	dFLIMfrontEndCtrl,        // 0x1C0 write  NOTE: the Legacy kernel driver had conflicting (old ThorDAQ) definitions for 1C0-1CF
	dFLIMfrontEndStrobe,      // 0x1C8 write
	dFLIMfrontEndStatus,        // 0x1C0 read
	ADCStreamDownsampleReg,     // 0x1D0   (Are these valid for dFLIM ??)
	DC_OFFSET_LOW,				// 0x1D8
	DC_OFFSET_HIGH,				// 0x1E0

/*	ADCStreamFIRcoeffReg,       // 0x1E8

	ADCStreamPulseInterleaveOffsetReg, // 0x1F0
	ADCThreePhotonSampleOffset, // 0x1F8

	// ADC interface control
	ADCFMCInterfaceControlReg,  // 0x200  
	ADCInterfaceDAT12,
	ADCInterfaceDAT34,
	ADCInterfaceDAT56,
	ADCFMCStatusReg,
*/	
	// DAC Waveform Control ("GALVO" in dFLIM)
	GALVOWaveGenControlReg,		// 0x240
	DAC_UpdateRate_Chan0,    // 0x248
	DAC_UpdateRate_Chan1,    // 0x24a
	DAC_UpdateRate_Chan2,    // 0x24c
	DAC_UpdateRate_Chan3,    // 0x24e
	DAC_UpdateRate_Chan4,    // 0x250
	DAC_UpdateRate_Chan5,    // 0x252
	DAC_UpdateRate_Chan6,    // 0x254
	DAC_UpdateRate_Chan7,    // 0x256
	DAC_UpdateRate_Chan8,    // 0x258
	DAC_UpdateRate_Chan9,    // 0x25a
	DAC_UpdateRate_Chan10,   // 0x25c
	DAC_UpdateRate_Chan11,   // 0x25e

	DAC_Amplitude,              // 0x260
	DAC_StepSize_Chan0,       // 0x262
	DAC_StepSize_Chan1,       // 0x264
	DAC_StepSize_Chan2,      // 0x266

	DAC_ParkValue_Chan0,     // 0x268
	DAC_ParkValue_Chan1,     // 0x26A
	DAC_ParkValue_Chan2,     // 0x26C
	DAC_ParkValue_Chan3,     // 0x26E
	DAC_ParkValue_Chan4,     // 0x270
	DAC_ParkValue_Chan5,     // 0x272
	DAC_ParkValue_Chan6,     // 0x274
	DAC_ParkValue_Chan7,     // 0x276
	DAC_ParkValue_Chan8,     // 0x278
	DAC_ParkValue_Chan9,     // 0x27a
	DAC_ParkValue_Chan10,    // 0x27c
	DAC_ParkValue_Chan11,    // 0x27e

	DAC_Offset_Chan0,			// 0x280
	DAC_Offset_Chan1,			// 0x282
	DAC_Offset_Chan2,			// 0x284
	DAC_Offset_Chan3,			// 0x286
	DAC_Offset_Chan4,			// 0x288
	DAC_Offset_Chan5,			// 0x28a
	DAC_Offset_Chan6,			// 0x28c
	DAC_Offset_Chan7,			// 0x28e
	DAC_Offset_Chan8,			// 0x290
	DAC_Offset_Chan9,			// 0x292
	DAC_Offset_Chan10,			// 0x294
	DAC_Offset_Chan11,			// 0x296

	GlobalABBXmuxReg,			// 0x298	
	DOUpdateRate,				// 0x2A0
	DOParkValue,				// 0x2A8
	DOOffset,					// 0x2B0
	DACAmFilterWindow,			// 0x2B8

	// Xilinx AIX I2C (legacy, only single byte read) - mutually exclusive of legacy Zhun interface
	I2C_AXIBridge,					// 0x2C0
	I2C_AXIStatus,					// 0x2C8 read-only
	I2C_AXIControl,					// 0X2C8 write-only


	// DFLIM Processing subsystem
	GPOReg0_Ch0,					// 0x300 
	GPOReg1_Ch0,					// 0x308 
	GPOReg2_Ch0,					// 0x310
	GPOReg3_Ch0,					// 0x318
	GPOReg4_Ch0,					// 0x320
	GPOReg5_1_Ch0,					// 0x328
	GPOReg5_2_Ch0,					// 0x32C
	GPOReg6_Ch0,					// 0x330
	GPOReg7_Ch0,					// 0x338

	GPOReg0_Ch1,					// 0x340 
	GPOReg1_Ch1,					// 0x348 
	GPOReg2_Ch1,					// 0x350
	GPOReg3_Ch1,					// 0x358
	GPOReg4_Ch1,					// 0x360
	GPOReg5_1_Ch1,					// 0x368
	GPOReg5_2_Ch1,					// 0x36C
	GPOReg6_Ch1,					// 0x370
	GPOReg7_Ch1,					// 0x378

	GPOReg0_Ch2,					// 0x380 
	GPOReg1_Ch2,					// 0x388 
	GPOReg2_Ch2,					// 0x390
	GPOReg3_Ch2,					// 0x398
	GPOReg4_Ch2,					// 0x3A0
	GPOReg5_1_Ch2,					// 0x3A8
	GPOReg5_2_Ch2,					// 0x3BC
	GPOReg6_Ch2,					// 0x3B0
	GPOReg7_Ch2,					// 0x3B8

	GPOReg0_Ch3,					// 0x3C0 
	GPOReg1_Ch3,					// 0x3C8 
	GPOReg2_Ch3,					// 0x3D0
	GPOReg3_Ch3,					// 0x3D8
	GPOReg4_Ch3,					// 0x3E0
	GPOReg5_1_Ch3,					// 0x3E8
	GPOReg5_2_Ch3,					// 0x3EC
	GPOReg6_Ch3,					// 0x3F0
	GPOReg7_Ch3,					// 0x3F8


	TD_LAST_FPGA_REGISTER
};


enum SamplingClockStatusREG
{
	SC_CLK_GEN_REV,
	SC_SPI_TXRX_READY,
	SC_CLKRX_PLL_LOCK,
	SC_LOCK_LOST_CNT,
	SC_LASER_CLK_CNT,
	SC_CLKRX_RXBYTE
};
enum SamplingClockControlREG
{
	SC_PHASE_INCREMENT_MODE,
	SC_FREQ_MEAS_SEL,
	DDS_CLK_3P_EN,
	SC_3P_EN,
	SC_CLK_CNT_CLEAR,
	SC_SPI_TXRX_EN
};
enum ScanningSyncControlREG
{
	SRCE_SOF_MODE = 0,
	FRM_GEN_MODE,
	FRM_RETRIGGER,
	ExternalPixelClock,
	HorizontalLineSyncGapCtrl,
	TIME_SRCE_SELECT,
	Capture_Active_Invert
};
enum ADCFMCStatusREG
{
	ThorDAQ_ADC_VERSION,
	mailbox_tnsfr_cmplt,
	jesd204b_rx_sync_chan0_1,
	jesd204b_rx_sync_chan2_3,
	jesd_rx_lost_cnt_chan0_1,
	jesd_rx_lost_cnt_chan2_3,
};
enum ADCInterfaceControlREG
{
	JESD204B_config_core1,
	JESD204B_config_core2,
	SPI_Peripheral_Override,
	JESD204B_Clear_rxsync_loss,
	JESD204B_Sysref_sync_to_laser,
	JESD204B_Test_Mode_Enable,
	ADC_GPIO0_INT_REF_SEL,
	ADC_GPIO1_INT_REF_EN,
	ADC_GPIO2_L_FPGA_REF_EN,
	ADC_GPIO3_FiltAB_MODE,
};
enum ADCInterfaceDAT12
{
	ADCGainChan0,
	ADCAFEenChan0,
	ADCGainChan1,
	ADCAFEenChan1
};
enum ADCInterfaceDAT34
{
	ADCGainChan2,
	ADCAFEenChan2,
	ADCGainChan3,
	ADCAFEenChan3
};
enum ADCInterfaceDAT56
{
	ADCGainChan4,
	ADCAFEenChan4,
	ADCGainChan5,
	ADCAFEenChan5
};

enum ADCStreamControlREG
{
	ADCStream_SCAN_PER_SRCE, // SCAN PERiod SouRCE
	ADCStream_DCoffsetPreFIR,
	ADCStream_DCoffsetPostFIR,
	ADCStream_FIRcoeffReload,
};
enum ADCStreamControl2Reg
{
	ADCStream_PLSE_INTERLV_EN,        // bits 0-5
	ADCStream_REVERB_MP_EN,			  // bit 6
	ADCStream_LASER3P_MARKER_EN       // bit 7 
};

enum NWL_DDR3_DMAcoreREG
{
	NWL_GlobalDMAIntEnable,  // bit 0
	NWL_DMAIntActive,        //     1
	NWL_DMAIntPending,       //     2
	NWL_DMAIntMode,          //     3
	NWL_UserIntEnable,       //     4 
	NWL_UserIntActive,       //     5
	NWL_MSIXMode             //     6
};
enum S2MMDMA_ControlREG1
{
	S2MM_CONFIG_Chan0,
	S2MM_RUN_STOP_Chan0,
	S2MM_SG_CYCLIC_BD_Chan0,
	S2MM_IRQ_REARM_Chan0, // clears S2MM_DESC_CHAIN_IRQ
	S2MM_SB_BRAM_BANK_SEL_Chan0,
};
enum S2MMDMA_ControlREG2
{
	S2MM_CONFIG_Chan1,
	S2MM_RUN_STOP_Chan1,
	S2MM_SG_CYCLIC_BD_Chan1,
	S2MM_IRQ_REARM_Chan1, // clears S2MM_DESC_CHAIN_IRQ
	S2MM_SB_BRAM_BANK_SEL_Chan1,
};
enum S2MMDMA_ControlREG3
{
	S2MM_CONFIG_Chan2,
	S2MM_RUN_STOP_Chan2,
	S2MM_SG_CYCLIC_BD_Chan2,
	S2MM_IRQ_REARM_Chan2, // clears S2MM_DESC_CHAIN_IRQ
	S2MM_SB_BRAM_BANK_SEL_Chan2,
};
enum S2MMDMA_ControlREG4
{
	S2MM_CONFIG_Chan3,
	S2MM_RUN_STOP_Chan3,
	S2MM_SG_CYCLIC_BD_Chan3,
	S2MM_IRQ_REARM_Chan3, // clears S2MM_DESC_CHAIN_IRQ
	S2MM_SB_BRAM_BANK_SEL_Chan3,
};

enum S2MMDMA_StatusREG1
{
	S2MM_DMAcontrollerRev_Chan0,
	S2MM_DESC_CHAIN_IRQ_Chan0, // Interrupt generated
	S2MM_CHANNUMindex_Chan0, // ?confirm/readback of 0-based channel
};
enum S2MMDMA_StatusREG2
{
	S2MM_DMAcontrollerRev_Chan1,
	S2MM_DESC_CHAIN_IRQ_Chan1, // Interrupt generated
	S2MM_CHANNUMindex_Chan1, // ?confirm/readback of 0-based channel
};
enum S2MMDMA_StatusREG3
{
	S2MM_DMAcontrollerRev_Chan2,
	S2MM_DESC_CHAIN_IRQ_Chan2, // Interrupt generated
	S2MM_CHANNUMindex_Chan2, // ?confirm/readback of 0-based channel
};
enum S2MMDMA_StatusREG4
{
	S2MM_DMAcontrollerRev_Chan3,
	S2MM_DESC_CHAIN_IRQ_Chan3, // Interrupt generated
	S2MM_CHANNUMindex_Chan3, // ?confirm/readback of 0-based channel
};

enum GlobalImageSyncControlREG
{
	BIDIR_SCAN_MODE,
	SCAN_DIR,
	HW_TRIG_MODE,
	WAVEFORM_PLAYBACK_HW_TRIG_MODE,
};
enum GlobalControlREG
{
	GIGCR0_STOP_RUN,
	GIGCR0_LED1,
	GIGCR0_LED2,
	ImageAcqTrig_HWSW_SEL,
	BPI_FLASH_MSB24,
	BPI_FLASH_MSB25,
};

// DIO (DBB1) REGISTER MUX (Legacy 1-based index)
// These are PCB hardware CIRCUIT indexes 
enum DBB1_DIO_MUX_REG // previously DBB1_DIOn_MUX
{
	DBB1_IOCIRCUIT_1 = 7,  // i.e., in 1:1 mapping, index 7 in FPGA reg connects to BNC D0
	DBB1_IOCIRCUIT_2 = 6,
	DBB1_IOCIRCUIT_3 = 3,
	DBB1_IOCIRCUIT_4 = 2,
	DBB1_IOCIRCUIT_5 = 5,
	DBB1_IOCIRCUIT_6 = 4,  // i.e., in 1:1 mapping, index 4 in FPGA reg connects BNC D5
	DBB1_IOCIRCUIT_7 = 1,
	DBB1_IOCIRCUIT_8 = 0
};
enum GlobalABBXmuxREG  // see FPGA SWUG "DAC Channel Map[x]" Reg, 0x298; we MUX the 0-based index to the BOB's BNC
{
	DAC_DMA00 = 3, // Legacy BBox ABB1
	DAC_DMA01 = 2,
	DAC_DMA02 = 1,
	DAC_DMA03 = 0,
	DAC_DMA04 = 4, // Chan 4-11 do not respond on ABB2 and ABB3
	DAC_DMA05,
	DAC_DMA06,
	DAC_DMA07,
	DAC_DMA08,
	DAC_DMA09,
	DAC_DMA10,
	DAC_DMA11
};

// END SHADOW REGISTERS

// FPGA write only registers must be tracked as to last written values
// define common fields like BAR and offset, then define the bit-fields
#define BAR0 0
#define BAR1 1
#define BAR2 2
#define BAR3 3
#define REGISTER_NAME_LEN 48
#define FIELD_NAME_LEN 48

#define NWLwriteDMAengineOffset 0
#define NWLreadDMAengineOffset 32
#define MAX_REGISTER_FIELDS 64 // max number of bitwise fields in any register

// DEFINE the shadow register classes/methods
class RegisterBitField
{
public:
	char _FieldName[REGISTER_NAME_LEN]; // name of REG bit field per SWUG
	UINT32 _bitNumStart; // field start bit (0-63)
	UINT32 _bitNumEnd;      // bit length of field (1-64)
	UINT32 _bitFldLen;

public:
	RegisterBitField(const char* name, UINT32 bitNumStart, UINT32 bitNumEnd)
	{
		strcpy_s(_FieldName, name);
		_bitNumStart = bitNumStart;
		_bitNumEnd = bitNumEnd;
		_bitFldLen = bitNumEnd - bitNumStart + 1;
	}
};
class FPGA_HardwareReg
{
#define WRITEONLY 0
#define READONLY 1
#define READWRITE 2
#define WriteOnly true
#define ReadOnly true
	// register is assumed Read/Write unless one of these flags is set:
public:
	INT32 _RdWrDir; // 
	//bool _bWriteOnly; // if true, WRITE-ONLY register
	//bool _bReadOnly;   // hardware read, no write
	char _RegName[REGISTER_NAME_LEN]; // name of register per SWUG
	UINT32 _size; // byte length of register (1-8)
	UINT32 _BAR;
	UINT32 _BARoffset;
	UINT64 _ResetValue;
	UINT64 _VALUE;  // last WRITTEN (or reset) value (contains all fields)

public:
	RegisterBitField* BitField[MAX_REGISTER_FIELDS]; // size according to FPGA design

public:  // constructor
	FPGA_HardwareReg(const char* name, UINT32 size, UINT32 BAR, UINT32 BARoffset, UINT64 ResetValue, INT32 RdWrDir)
	{
		_RdWrDir = RdWrDir;
		strcpy_s(_RegName, name);
		_size = size;
		_BAR = BAR;
		_BARoffset = BARoffset;
		_VALUE = _ResetValue = ResetValue;
		for (int f = 0; f < MAX_REGISTER_FIELDS; f++) // dynamically allocated
			BitField[f] = NULL;
	}

	UINT64 Read(int Field);
	void Write(int Field, UINT64 NewRecFldValue);
};



// This class is exported from the thordaq.dll
class CdFLIM_4002 {

private:
	FPGA_HardwareReg* _FPGAregister[TD_LAST_FPGA_REGISTER + 1];
	UINT64			                    _pRxPacketBufferHandle;       // restore NWL API variable

public:
	// Constructors
	CdFLIM_4002();
	CdFLIM_4002(UINT boardNum, HANDLE handle);
	// Destructor
	~CdFLIM_4002();

	WCHAR _wcMname[10] = L"XI2CBrd ";
	LPCWSTR _I2CmutexName;
	HANDLE _hXI2Cmutex; // Xilinx I2C MUTEX for master in case of thread contention

	//Connect to the device.
	THORDAQ_STATUS GetDMAcapability(
	);

	//Disconnect to the device.
	THORDAQ_STATUS DisconnectFromBoard(
	);


	/////// restore NWL API, and define DZ SDK based on that API /////////////////
	THORDAQ_STATUS DoMem(
	UINT32          Rd_Wr_n,        // 1==Read, 0==Write
	UINT32          BarNum,         // Base Address Register (BAR) to access
	PUINT8          Buffer,         // Data buffer
	UINT64          Offset,         // Offset in data buffer to start transfer
	UINT64          CardOffset,     // Offset in BAR to start transfer
	UINT64          Length,         // Byte length of transfer
	PSTAT_STRUCT    Status          // Completion Status
);

public: THORDAQ_STATUS SetupPacket(
	INT32   EngineOffset,       // DMA Engine number offset to use
	PUINT8  Buffer,
	PUINT32 BufferSize,
	PUINT32 MaxPacketSize,
	INT32   PacketModeSetting,
	INT32   NumberDescriptors

);
public: THORDAQ_STATUS PacketReadEx(
	INT32                   EngineOffset,       //
	PUINT64                 UserStatus,         //
	UINT64                  CardOffset,
	UINT32                  Mode,               //
	PUINT8                  Buffer,
	PUINT32                 Length
);
public: THORDAQ_STATUS PacketWriteEx(
	INT32   EngineOffset,           // DMA Engine number offset to use
	UINT64  UserControl,
	UINT64  CardOffset,
	UINT32  Mode,                   // Control Mode Flags
	PUINT8  Buffer,
	UINT32  Length
);

	THORDAQ_STATUS WriteDDR3(
		PUINT8 HostSourceAddress,
		UINT64 DDR3DestinationAddress,
		UINT32 ByteLength);

	THORDAQ_STATUS ReadDDR3(
		UINT64 DDR3SourceAddress,
		PUINT8 HostDestinationAddress,
		PUINT32 ByteLength);


public: void FPGAregisterShadowFPGAwrite(int Record, int Field, UINT64 value);
public: THORDAQ_STATUS FPGAregisterWrite(
	const char* pName,			// name of register/field (if found)
	int nameSize,
	UINT64  Value           // register/field value to write (from 1 to 64 bits)
); 
	  // for performance, provide register write by index (no Bitfield!)
public: THORDAQ_STATUS FPGAregisterWrite(
	int Record, 			// zero based register array index
	UINT64 Value           // register/field value - either from FPGA hardware or shadow copy
);
public: THORDAQ_STATUS FPGAregisterRead(
	int Record, 			// zero based register array index
	int Field,				// zero based field index within register (or -1 for no field)
	UINT64* Value           // register/field value - either from FPGA hardware or shadow copy
);
public: THORDAQ_STATUS FPGAregisterRead(
	const char* pName,			// name of register/field (if found)
	int nameSize,
	UINT64* Value           // "shadow" DLL register copy value (REG or FIELD)
);
public: void FPGAregisterWriteDoMem(int Record);
public: UINT64 FPGAregisterReadDoMem(int Record);
	  bool SearchFPGAregFieldIndices(
		  const char* name,
		  int nameSize,
		  int* pRecord,
		  int* pField);

THORDAQ_STATUS FPGAregisterQuery(
		int RegIndex,           // 0-based list of WO, RO, and R/W registers
		int FldIndex,           // 0-based enumerated field in register
		char* pName,			// returned name of register/field (if found)
		int pNameSize,
		PREG_FIELD_DESC RegFldDesc); // register/field description

	//Set/Get the register value of the device.
	THORDAQ_STATUS WriteReadRegister ( UINT read_write_flag, UINT register_bar_num, ULONGLONG register_card_offset, BYTE* buffer, ULONGLONG offset,  ULONGLONG length,  PSTAT_STRUCT completed_status);

	
	THORDAQ_STATUS GetBoardCfg(
		BOARD_INFO_STRUCT*	    board_info  // Returned structure
	);

	// Read data from particular channel address predefined in the FPGA 
    THORDAQ_STATUS PacketReadChannel(
		ULONG   Channel,           // Channel Index to be read from
		ULONG*  buffer_length,            // Size of the Packet Recieve Data Buffer requested (FIFO Mode) 
		void*   Buffer,            // Data buffer (Packet Mode) 
		double   Timeout_ms            // Generate Timeout error when timeout
	);

	// Abort reading 
    THORDAQ_STATUS AbortPacketRead(
	);
	
	// DZ NWL replacement - read all channels
	THORDAQ_STATUS APIMemTest(char*); // return SUCCESS, or if ERROR a string describing problem

	THORDAQ_STATUS APIReadFrames(
		PUINT32 buffer_length,
		int chan,
		void* sySHostBuffer[4],
		double timeout_ms,
		ULONG transferFrames,
		BOOL isLastTransfer,
		BOOL& isCompleteImage);


	//Read data from particular data start address
	THORDAQ_STATUS PacketReadBuffer(
		ULONG64  Address,           // Start address to read in the card
		ULONG*   Length,            // Size of the Packet Recieve Data Buffer requested (FIFO Mode)
		void*    Buffer,            // Data buffer (Packet Mode)
		ULONG    Timeout            // Generate Timeout error when timeout
	);

	THORDAQ_STATUS PacketWriteBuffer(
		ULONG64 register_card_offset,           // Start address to read in the card
		ULONG   Length,            // Size of the Packet Write to Data Buffer Count by Byte (FIFO Mode)
		UCHAR*  Buffer,            // Data buffer (Packet Mode)
		ULONG   Timeout            // Generate Timeout error when timeout
	);

	//Set up data imaging configuration

//	THORDAQ_STATUS dFLIMSetTestUtilConfig( CL_GUI_GLOBAL_TEST_STRUCT config);

	THORDAQ_STATUS SetImagingConfiguration(
		dFLIM_IMAGING_CONFIGURATION_STRUCT	imaging_config // Image configuration stuct
	 );

	// read and write production ID and Serial Number (dedicated fields in ThorDAQ system EEPROMs)
	THORDAQ_STATUS APIReadProdSN(
		CdFLIM_4002& TdBrd,
		UINT32 fileOp,                 // if 0, no file operation, 1 record update to file
		char eepromID[4],  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.
		char ProdID[10],   // TIS Production system fields
		char ProdSN[6]); 

	THORDAQ_STATUS APIWriteProdSN(
		CdFLIM_4002& TdBrd,
		UINT32 fileOp,                 // if 0, no file operation, 1 record update to file
		char eepromID[4],  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.
		char ProdID[10],   // TIS Production system fields
		char ProdSN[6]);

	THORDAQ_STATUS APIReadEEPROM(
		CdFLIM_4002& TdBrd,
		UINT32 fileOp,                 // if 0, no file operation
		char eepromID[4],  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.
		char eepromDATA[64]); // hardware definition)

	THORDAQ_STATUS APIWriteEEPROM(
		CdFLIM_4002& TdBrd,
		UINT32 fileOp,                 // if 0, no file operation
		char eepromID[4],  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.
		char eepromDATA[64]); // hardware definition)

	THORDAQ_STATUS APIGetEEPROMlabel(
		CdFLIM_4002& TdBrd,
		UINT32 eeprom_ID,
		char eepromID[4]);  // the identifier of device, e.g. TD3, ADC, FMC, DAC, DCL, DB1, etc.

	/******************************************
	* ThorDAQ DAC Control Functions
	******************************************/
	THORDAQ_STATUS SetDACParkValue(
		ULONG32 outputChannel, 
		double outputValue
	);

	THORDAQ_STATUS SetDACChannelMapping();
	/******************************************
	* ThorDAQ Control Functions
	******************************************/
	THORDAQ_STATUS SetADCChannelMapping();

	/******************************************
	* ThorDAQ Trouble Shooting Functions
	******************************************/
	THORDAQ_STATUS DoMemoryTest();
	THORDAQ_STATUS GetFirmwareVersion(UINT8& major, UINT8& minor, UINT8& revision);
    THORDAQ_STATUS GetDriverVersion(UINT8& major, UINT8& minor, UINT8& revision);
    THORDAQ_STATUS GetCPLDVersion(UINT8& major, UINT8& minor, UINT8& revision);

	/******************************************
	* ThorDAQ IO Status Read Functions
	******************************************/
	THORDAQ_STATUS GetLineTriggerFrequency(UINT32 sample_rate,double& frequency);
	THORDAQ_STATUS GetTotalFrameCount(UINT32& frame_count);
	THORDAQ_STATUS GetDACSamplesPerLine(UINT32& samples,double& dac_rate, double line_time);
	// Individual Parameter API Functions
	THORDAQ_STATUS SetParameter( UINT32 parameter,long value);
	THORDAQ_STATUS GetParameter( UINT32 parameter,long& retValue);

	/******************************************
	* ThorDAQ DMA Functions
	******************************************/
	THORDAQ_STATUS  WaitForBufferReady(double time_ms);
	THORDAQ_STATUS  ReadBuffer(UINT8 channel, UINT32*  buffer_length, void* buffer, double timeout_ms);
	
	// Capture API Functions
	//Start acquisition
	THORDAQ_STATUS StartAcquisition();

	//Stop acquisition
	THORDAQ_STATUS StopAcquisition();

	THORDAQ_STATUS GetExternClockStatus(ULONG32& isClockedSynced);

	//Set up FPGA DMA Packet Mode to addressable
	THORDAQ_STATUS SetPacketModeAddressable(bool stream_to_mem_dma_enable);

	THORDAQ_STATUS APIdFLIMReSync();

	THORDAQ_STATUS SetCoarseShift(ULONG32 shift, int channel);

	THORDAQ_STATUS SetFineShift(LONG32 shift, int channel);

	THORDAQ_STATUS APIdFLIMGetClockFrequency(int clockIndex, double& frequency);

	THORDAQ_STATUS SetDFLIMSyncingSettings(ULONG32 syncDelay, ULONG32 resyncDelay, bool forceResyncEverLine);

	THORDAQ_STATUS SetdFLIMFrontEndSettings();

	THORDAQ_STATUS GlobalSCANstart(bool Start);
	THORDAQ_STATUS S2MMconfig(PS2MM_CONFIG pS2MMconfig);
	THORDAQ_STATUS SetdFLIM_GPOregs(int Chan, ULONG32 GPOReg0, ULONG64 GPOReg3, ULONG32 GPOReg4,
		ULONG32 GPOReg5_1, ULONG32 GPOReg5_2, ULONG64 GPOReg6, ULONG64 GPOReg7);
public: THORDAQ_STATUS APItdUserIntTask(USER_IRQ_WAIT_STRUCT* usrIrqWaitStruct);
public: THORDAQ_STATUS APItdCancelUserIntTask();
	THORDAQ_STATUS APIWaitForUserIRQ(UINT64* CountOfChan0_INTs, UINT64* CountOfChan1_INTs, UINT64* CountOfChan2_INTs, UINT64* CountOfChan3_INTs);
	THORDAQ_STATUS APICancelWaitForUserIRQ();

public: THORDAQ_STATUS WriteS2MMdescChain(
	int ADCchannel, int StartingBRAMbank,
	int ChainHead, int ChainTail,
	AXI_DMA_DESCRIPTOR DMA_Desc[][MAX_CHANNEL_COUNT],
	PS2MM_CONFIG pS2MMconfig);

	THORDAQ_STATUS APIimageAcqConfig(DATA_ACQ_CTRL_STRUCT* imaging_config);
	THORDAQ_STATUS UserIRQWait(USER_IRQ_WAIT_STRUCT* pUserIrqWaitStruct, UINT64* IrqCnt0, UINT64* IrqCnt1, UINT64* IrqCnt2, UINT64* IrqCnt3);
	THORDAQ_STATUS UserIRQCancel(VOID);
	THORDAQ_STATUS APIProgressiveScan(BOOL bProgressiveScan);
	THORDAQ_STATUS APIsetDACvoltage( UINT32 chan, double Volts);

	// 8-Jan-2024, D. Zimmerman: add the Xilinx I2C Master
//
	THORDAQ_STATUS APIXI2CReadWrite(
		CdFLIM_4002& TdBrd, // This CdFLIM_4002 object by reference	
		bool bI2C_Read,            // when true, READ bytes from I2C slave
		UINT32 MasterMUXAddr,
		UINT32 MasterMUXChan,
		UINT32 SlaveMUXAddr,
		UINT32 SlaveMUXChan,
		UINT32 DevAddress,
		UINT32 I2CbusHz,    // frequency of bus, i.e. 100 or 400 Hz (or ...)
		INT32 PageSize,     // I2C slave specific, especially for page WRITEs (higher performance)
		PUINT8 OpCodeBuffer,
		UINT32* OpCodeLen,
		PUINT8 BiDirectionalDataBuffer,  // bi-directional data buffer -- DATA bytes to write to slave, or read from slave
		UINT32* BiDirectionalDataLen // IN: expect returned bytes Rx'd from slave  OUT: actual bytes (can be 0), Opcodes+Data
	);
	// end of Xilinx I2C Master
private:
	HANDLE								gHdlDevice;
	HDEVINFO							gDeviceInfo;				 // The handle to a device information set that contains all installed devices that matched the supplied parameters
	UINT								gBoardIndex;				 // The index number to record board number
	PSP_DEVICE_INTERFACE_DETAIL_DATA	gDeviceInterfaceDetailData;  // The structure to receice information about device specified interface
	DMA_INFO_STRUCT						gDmaInfo;					 // The structure to retrieve the board dma configuration
	DATA_ACQ_CTRL_STRUCT*               gPtrAcqCtrl;                   // The Global Pointer to Thordaq acquisition configuration struct
	ULONG64								_dacDescpListIndex;
	atomic<bool>						abortPacketRead;
	HANDLE								_nwlDMAdescriptorsMutex;

	BOOL _bProgressiveScan;
	int _DACWaveGen3PSyncControlRegIndex = -1; // used in dFLIM?
	int _DACWaveGenISRIndex = -1;
//	UINT64								_DACWaveGenInterruptCounter[WAVETABLE_CHANNEL_COUNT]; // counts number of recv'd interrupts since GlobalSTART (ThorDAQ only)
	int _nullBitField = -1;											// dummy field for index search
	int _S2MMDMAStatusRegIndex[MAX_CHANNEL_COUNT];
	int _S2MMDMA_IRQ_REARM_RegIndex[MAX_CHANNEL_COUNT];
	UINT64 _reArmS2MMDMACommand[MAX_CHANNEL_COUNT];
	USER_IRQ_WAIT_STRUCT				_usrIrqWaitStruct;            // contains DMA Bank indicator, int. timeout, etc.
	BOOL								_s2mmBankLastRead;			  // used to keep track of the last read s2mm bank

	UINT32								_NWL_Common_DMA_Register;	  // snapshot of Register from Interrupt (DPC) handler (currently NOT working)
	std::thread* _irqThread;					  // thread to check for interrupts

	//Set up FPGA DMA Packet Mode
	THORDAQ_STATUS SetPacketMode(
		int		EngineNum,		// DMA Engine number offset to use
		bool stream_to_mem_dma_enable,
		ULONG *	MaxPacketSize,		// Length of largest packet (FIFO Mode)
		int		packet_alloc_mode,			// Sets mode, FIFO or Addressable Packet mode
		int		NumberDescriptors	// Number of DMA Descriptors to allocate (Addressable mode)	
	);


// 
//	THORDAQ_STATUS SetImagingSettings();
	THORDAQ_STATUS StartImaging();
	THORDAQ_STATUS SetI2C_0x2C0(ULONG bytes);
	THORDAQ_STATUS SetI2C_0x2C4(ULONG bytes);
	THORDAQ_STATUS SetI2C_0x2C8(BYTE byte);
	THORDAQ_STATUS SetI2C_0x2C8_232();

	THORDAQ_STATUS ResetBackEndAcq();  //gy

	THORDAQ_STATUS ReadI2C_0x2C0(ULONG length);

	THORDAQ_STATUS SetNONI2C_0x1C0(ULONG bytes);
	THORDAQ_STATUS SetNONI2C_0x1C4(ULONG bytes);
	THORDAQ_STATUS SetNONI2C_0x1C8(BYTE byte);

	THORDAQ_STATUS ReadNONI2C_0x1C0(ULONG length);

	//Get DMA Engine Capabilitie
	THORDAQ_STATUS GetDMAEngineCap(
		ULONG			EngineNum,	// DMA Engine number to use
		PDMA_CAP_STRUCT dma_capability		// Returned DMA Engine Capabilitie
	);

	//Sends two PACKET_BUF_DEALLOC_IOCTL calls to the driver to teardown the recieve buffer and teardown the descriptors for sending packets
	THORDAQ_STATUS ReleasePacketBuffers(
		int				EngineNum // DMA Engine number to use
	);

	//Set up galvo resonant scanner Galvo configuration
	LONG SetGRGalvoSettings(
		DAC_CRTL_STRUCT dac_setting //Galvo configuration stuct
	);

	//Set up galvo galvo system galvo configuration
	LONG SetGGGalvoSettings(
		GALVO_GALVO_CTRL_STRUCT galvo_galvo_ctrl,
		WAVEFORM_TYPE    waveformType,
		 DAC_CRTL_STRUCT dac_setting //Galvo configuration stuct
	);
	
	LONG SetGlobalSettings(dFLIM_IMAGING_CONFIGURATION_STRUCT	imaging_config);

	LONG SetScanSettings(dFLIM_IMAGING_CONFIGURATION_STRUCT	imaging_config);

	LONG SetCoherentSampleingSettings(dFLIM_IMAGING_CONFIGURATION_STRUCT	imaging_config);
	
	LONG SetStreamProcessingSettings(dFLIM_IMAGING_CONFIGURATION_STRUCT	imaging_config);

	LONG SetDACSettings(
		dFLIM_IMAGING_CONFIGURATION_STRUCT	imaging_config
	);

	

	LONG SetWaveformPlayback(
		DAC_CRTL_STRUCT dac_setting,
		int              channel,
		UINT8 set_flag = TRUE
		);


	LONG LoadDACDescriptors(DAC_CRTL_STRUCT* dac_settings);

	void SetDMADescriptors(DMADescriptor& dmaDescp, ULONG64 index, ULONG64* dmaDescpTable, bool& status);

//	LONG ExportScript(DATA_ACQ_CTRL_STRUCT*               gPtrAcqCtrl, SCAN_LUT scanLUT);

	void LogMessage(wchar_t* logMsg, long eventLevel);
};

