#include "stdafx.h"
#pragma once

#include "..\..\..\..\..\Common\ThorSharedTypes\ThorSharedTypes\SharedEnums.cs"
#include "thordaqcmd.h"
#include "thordaqres.h"
#include "thordaqconst.h"
#include "ThorDAQSettingsXML.h"
#include "filter.h"
#include <atomic>

#define MAXIMUM_NUMBER_OF_BOARDS 4 //

// Convert function name macro to wide string for ThordaqErrChk
#define WIDE2(x) L##x
#define WIDE1(x) WIDE2(x)
#define WFUNCTION WIDE1(__FUNCTION__)

#define MIN_MEM_BUFFER_SIZE 8

#define LOGGING_ENABLED
#ifdef LOGGING_ENABLED
#include "..\..\..\..\..\Common\Log.h"
static std::auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#else
enum EventType
{
	// Summary:
	//     Fatal error or application crash.
	CRITICAL_EVENT = 1,
	//
	// Summary:
	//     Recoverable error.
	ERROR_EVENT = 2,
	//
	// Summary:
	//     Noncritical problem.
	WARNING_EVENT = 4,
	//
	// Summary:
	//     Informational message.
	INFORMATION_EVENT = 8,
	//
	// Summary:
	//     Debugging trace.
	VERBOSE_EVENT = 16,

};
#endif

#define ThordaqErrChk(fnName,fnCall) if ((error=(fnCall)) > STATUS_SUCCESSFUL){ StringCbPrintfW(thordaqLogMessage,_MAX_PATH,WFUNCTION L": %s failed. Error code %d",fnName, error); LogMessage(thordaqLogMessage,ERROR_EVENT); }

#define FPGAregisterWRITE(a,b)  (FPGAregisterWrite(a,(int)strlen(a),b))
#define FPGAregisterREAD(a,b)  (FPGAregisterRead(a,(int)strlen(a),b))
//
// Copied Macros from ntddk.h. Used to using the kernel-mode
// style of linked list.
//

enum class ShutDownType
{
	POWER_OFF = 0,
	RESTART = 1,
	UNKNOWN = 2,
	NOSHUTDOWN = 3
};

struct DMADescriptor
{
	ULONG64 nextDesc;
	ULONG64 loopStartDesc;
	ULONG64 length;
	ULONG64 buf_addr;
	USHORT loopCount;
	bool isLoopStart;
	bool isLoopEnd;
	bool loopEnable;
};

struct DMADescriptors
{
	std::vector<DMADescriptor> desc;
	bool finishPlaybackInLastGroup;
};

// DZ SDK /////////////////////////////////////////

//TODO: need to update to the new definition that includes the LP bit and 13bits for NextDesc
// Define bit fields for the DAC waveformBuffer descriptor per SWUG "ThorDAQ Waveplay DMA Descriptors"
typedef struct _DAC_WAVEPLAY_DESCRIPTOR
{
	UINT64 DDR3StartAddr : 34;  // bits  0 - 33
	UINT64 LooP : 1;            // bit 34
	UINT64 TotalBytes : 16;     // bits 35 - 50  (Total bytes to xmit / 4)
	UINT64 NextDesc : 13;       // bits 51 - 63
} DAC_WAVEPLAY_DESCRIPTOR, * PDAC_WAVEPLAY_DESCRIPTOR;

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

// Note that these REGISTER indices are inteneded to be arbitrary
// It is the NAME of the RECORD and FIELD that defines FPGA hardware location

// PROGRAMMING NOTE!  Don't allow short field name to 
// EXACTLY MATCH beginning of longer field name
#define MAX_REGISTER_FIELDS 64 // max number of bitwise fields in any register
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
	GlobalReadData,				 // 0x6	READ only
	GlobalGPIAuxReg,            // 0x7  READ only (1 byte) - see GlobalGPIODirReg
	GlobalImageSyncControlReg,  // 0x8
	GlobalImageHSIZE,           // 0x10
	GlobalImageAcqHSIZE,        // 0x12
	GlobalImageVSIZE,           // 0x18
	GlobalDBB1muxReg,           // 0x20
	GlobalScratch,				// 0x28
	GlobalGPOAuxReg,			// 0x30 Sets output value of Aux_GPIO_x when configured as OUTPUT (read @BAR3 0x07
	GlobalAuxGPIODirReg,           // 0x32 Sets Input/Ouput direction of Aux_GPIO_x

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
	ScanningPreSOFDelay,        // 0x17B

	SamplingClockControlReg,    // 0x180 (WriteOnly)
	SamplingClockStatusReg,     // 0x180 (ReadOnly 1 byte)
	SamplingClock_LASER_CLK_CNT,// 0x181 (RO, 4 bytes)
	SamplingClock_CLKRX_RXBYTE, // 0x185 (RO, 1 byte)
	SamplingClockPhaseOffset,   // 0x188
	SamplingClockPhaseStep,     // 0x190 
	SamplingClockPhaseLimit,    // 0x198
	SamplingClockFreqMeasurePeriod, // 0x1A0
	SamplingClockCLKRX_TXBYTE,      // 0x1B0


	DDS_3P_RVB_Phase0,			// 0X1B8
	DDS_3P_RVB_Phase1,			// 0X1B9

	ADCStreamControlReg,           // 0x1C0 (8-bits)
	ADCStreamControl2Reg,		   // 0x1C1 (8-bits)
	ADCStreamControl3Reg,		   // 0x1C2 (16-bits)
	ADCStreamScanningPeriodReg,    // 0x1C8
	ADCStreamDownsampleReg,        // 0x1D0
	ADCStreamDownsampleReg2,        // 0x1D2
	ADCStreamReverbDownsampleReg,  // 0x1D6 (only 4-bits)

	//ADCStreamDCoffsetChan0_3Reg,   // 0x1D8
	ADCStreamDCoffsetChan0,        // 0x1D8
	ADCStreamDCoffsetChan1,        // 0x1DA
	ADCStreamDCoffsetChan2,        // 0x1DC
	ADCStreamDCoffsetChan3,        // 0x1DE
	//	ADCStreamDCoffsetChan4_7Reg,   // 0x1E0
	ADCStreamDCoffsetChan4,        // 0x1E0
	ADCStreamDCoffsetChan5,        // 0x1E0
	ADCStreamDCoffsetChan6,        // 0x1E0
	ADCStreamDCoffsetChan7,        // 0x1E0

	ADCStreamFIRcoeffReg,          // 0x1E8
	ADCStreamPulseInterleaveOffsetReg, // 0x1F0
	ADCStream3PSampleOffsetReg,        // 0x1F8
	ADCStream3PReverbMPCyclesReg,      // 0x1F9

	ADCFMCInterfaceControlReg,  // 0x200 (WriteOnly)
	ADCFMCStatusReg,			// 0x200 (ReadOnly)
	ADCInterfaceDAT12,          // 0x208 WO (Digital ATtenuator)
	ADCInterfaceDAT34,          // 0x210 WO
	ADCInterfaceDAT56,          // 0x218 WO
	ADCInterface3PMarkersRDivide, //0x230 

	DACWaveGenControlReg,       // 0x240

	DAC_UpdateRate_Chan0,       // 0x248
	DAC_UpdateRate_Chan1,       // 0x24a
	DAC_UpdateRate_Chan2,       // 0x24c
	DAC_UpdateRate_Chan3,     // 0x24e
	DAC_UpdateRate_Chan4,       // 0x250
	DAC_UpdateRate_Chan5,       // 0x252
	DAC_UpdateRate_Chan6,       // 0x254
	DAC_UpdateRate_Chan7,       // 0x256
	DAC_UpdateRate_Chan8,       // 0x258
	DAC_UpdateRate_Chan9,       // 0x25a
	DAC_UpdateRate_Chan10,      // 0x25c
	DAC_UpdateRate_Chan11,       // 0x25e

	DACWaveGen3PSyncControlReg, // 0x260 (includes BANK switch post Nov. 2020)

	DAC_Park_Chan0,             // 
	DAC_Park_Chan1,             // 
	DAC_Park_Chan2,             // 
	DAC_Park_Chan3,
	DAC_Park_Chan4,             // 
	DAC_Park_Chan5,             // 
	DAC_Park_Chan6,             // 
	DAC_Park_Chan7,
	DAC_Park_Chan8,             // 
	DAC_Park_Chan9,             // 
	DAC_Park_Chan10,            // 
	DAC_Park_Chan11,

	DACWaveGenISR,              // 0x280 (8-bytes READONLY)

	DAC_Offset_Chan0,
	DAC_Offset_Chan1,
	DAC_Offset_Chan2,
	DAC_Offset_Chan3,
	DAC_Offset_Chan4,
	DAC_Offset_Chan5,
	DAC_Offset_Chan6,
	DAC_Offset_Chan7,
	DAC_Offset_Chan8,
	DAC_Offset_Chan9,
	DAC_Offset_Chan10,
	DAC_Offset_Chan11,

	GlobalABBXmuxReg,       // 0x298 (the ABBx mapping)

	DACWaveGenSelExtDigTrigReg, // 0x29E (Galvo_WaveGen:  Sel_Ext_Dig_Trig_Register:)

	DAC_UpdateRate_Chan12,
	DAC_UpdateRate_Chan13,

	DACWaveDACFilterInhibit,    // 0x2A4

	DACWavePerChannelRunStop, // 0x2A6

	DACWaveGenDescCntBeforeInt0, //0x2A8
	DACWaveGenDescCntBeforeInt1, //0x2A9
	DACWaveGenDescCntBeforeInt2, //0x2AA
	DACWaveGenDescCntBeforeInt3, //0x2AB
	DACWaveGenDescCntBeforeInt4, //0x2AC
	DACWaveGenDescCntBeforeInt5, //0x2AD
	DACWaveGenDescCntBeforeInt6, //0x2AE
	DACWaveGenDescCntBeforeInt7, //0x2AF
	DACWaveGenDescCntBeforeInt8, //0x2B0
	DACWaveGenDescCntBeforeInt9, //0x2B1
	DACWaveGenDescCntBeforeInt10,//0x2B2
	DACWaveGenDescCntBeforeInt11,//0x2B3
	DACWaveGenDescCntBeforeInt12,//0x2B4
	DACWaveGenDescCntBeforeInt13,//0x2B5

	DACWaveGen_EOF_DelaySampleCnt, // 0x2B6

	DAC_Park_Chan12, //0x2B8
	DAC_Park_Chan13, //0x2BA

	DAC_Offset_Chan12, //0x2BC
	DAC_Offset_Chan13, //0x2BE

	DACWaveGenPerChannelHWTrigControl,
	DACWaveGenPerChannelHWTrigSel1,
	DACWaveGenPerChannelHWTrigSel2,
	DACWaveGenPerChannelDigitalTrigSel,
	DACWaveGenPerChannelEOFFreeze,

	// Xilinx I2C Master core
	I2C_AXIBridge,         // 0x300 8 byte (write-only)
	I2C_AXIStatus,         // 0x300 4 byte (read-only)
	I2C_AXIControl,  // 0x308 8 byte (write-only)

	// 0x10000 - x017FFF, the 4k Wave Playback Table
	// define the 14 pre-defined channel registers
	// the following 14 channel defines MUST BE CONTIGUOUS
	DACWavePlayDMAdescriptorChan0,		// 0x50000
	DACWavePlayDMAdescriptorChan1,		// 0x50008
	DACWavePlayDMAdescriptorChan2,
	DACWavePlayDMAdescriptorChan3,
	DACWavePlayDMAdescriptorChan4,
	DACWavePlayDMAdescriptorChan5,
	DACWavePlayDMAdescriptorChan6,
	DACWavePlayDMAdescriptorChan7,
	DACWavePlayDMAdescriptorChan8,
	DACWavePlayDMAdescriptorChan9,
	DACWavePlayDMAdescriptorChan10,
	DACWavePlayDMAdescriptorChan11, // last of the 12 breakout box AO channels
	DACWavePlayDMAdescriptorChan12, // the "Digital I/O" wave channel
	DACWavePlayDMAdescriptorChan13,
	// the previous 13+1 channel defines MUST BE CONTIGUOUS
	TD_LAST_FPGA_REGISTER
};

enum GlobalABBXmuxREG  // see FPGA SWUG "DAC Channel Map[x]" Reg, B3 0x298
{
	DAC_DMA00 = 0,
	DAC_DMA01,
	DAC_DMA02,
	DAC_DMA03,
	DAC_DMA04,
	DAC_DMA05,
	DAC_DMA06,
	DAC_DMA07,
	DAC_DMA08,
	DAC_DMA09,
	DAC_DMA10,
	DAC_DMA11
};

enum GlobalAuxGPIODirREG           // 0x32 Sets Input/Ouput direction of Aux_GPIO_x
{
	Aux_GPIO_DIR_0 = 0,
	Aux_GPIO_DIR_1 = 1,
	Aux_GPIO_DIR_2 = 2,
	Aux_GPIO_DIR_3 = 3,
	Aux_GPIO_DIR_4 = 4,
};

enum I2C_XilinxCmdREG
{
	I2C_AXIwrite,
	I2C_AXIread,
	TI9548AChipReset_         // asserted LOW
};
enum I2C_StatusREG
{
	I2C_Status_NACK,
	I2C_Status_ACK,
	I2C_Status_MSGbusy,
	I2C_Status_HWtimeout,
	I2C_Status_HWerror,
	I2C_Status_Ready,
	I2C_Status_BusyTrans,
	I2C_Status_BusyTransFIFO,
};
enum I2C_ControlREG
{
	I2C_FIFO_rst,
	I2C_FIFO_in_strobe,
	I2C_FIFO_out_strobe,
	I2C_Emergency_Brake,
	I2C_Master_Start // "command_valid"
};
enum I2C_SlaveDevAddressREG
{
	I2C_DevAddress,
	I2C_ReadWrite
};
enum DACWaveGenISRREG
{
	DACWaveGenIntStatus_Chan0 = 0,
	DACWaveGenIntStatus_Chan1,
	DACWaveGenIntStatus_Chan2,
	DACWaveGenIntStatus_Chan3,
	DACWaveGenIntStatus_Chan4,
	DACWaveGenIntStatus_Chan5,
	DACWaveGenIntStatus_Chan6,
	DACWaveGenIntStatus_Chan7,
	DACWaveGenIntStatus_Chan8,
	DACWaveGenIntStatus_Chan9,
	DACWaveGenIntStatus_Chan10,
	DACWaveGenIntStatus_Chan11,
	DACWaveGenIntStatus_Chan12,
	DACWaveGenIntStatus_Chan13
};
enum DACWaveDACFilterInhibitREG
{
	DACWave_Filter_Inhibit0 = 0,
	DACWave_Filter_Inhibit1,
	DACWave_Filter_Inhibit2,
	DACWave_Filter_Inhibit3,
	DACWave_Filter_Inhibit4,
	DACWave_Filter_Inhibit5,
	DACWave_Filter_Inhibit6,
	DACWave_Filter_Inhibit7,
	DACWave_Filter_Inhibit8,
	DACWave_Filter_Inhibit9,
	DACWave_Filter_Inhibit10,
	DACWave_Filter_Inhibit11,
	DACWave_Filter_Inhibit12,
	DACWave_Filter_Inhibit13
};

enum DACWaveGen3PSyncControlREG
{
	DAC_Bank_Sel_Frame_CNT,
	DAC_SLOW_RATE,
	DAC_Waveplay_IRQ_rearm,
	DAC_Waveplay_Start_Of_Waveform_Delay_Enable,
	DAC_Bank_Switch_EN,
	DAC_Waveplay_Cont_Playback_En0,
	DAC_Waveplay_Cont_Playback_En1,
	DAC_Waveplay_Cont_Playback_En2,
	DAC_Waveplay_Cont_Playback_En3,
	DAC_Waveplay_Cont_Playback_En4,
	DAC_Waveplay_Cont_Playback_En5,
	DAC_Waveplay_Cont_Playback_En6,
	DAC_Waveplay_Cont_Playback_En7,
	DAC_Waveplay_Cont_Playback_En8,
	DAC_Waveplay_Cont_Playback_En9,
	DAC_Waveplay_Cont_Playback_En10,
	DAC_Waveplay_Cont_Playback_En11,
	DAC_Waveplay_Cont_Playback_En12,
	DAC_Waveplay_Cont_Playback_En13,
	PT_WaveformControlTrig_SEL0,
	PT_WaveformControlTrig_SEL1,
	PT_WaveformControlTrig_SEL2,
	PT_WaveformControlTrig_SEL3,
	PT_WaveformControlTrig_SEL4,
	PT_WaveformControlTrig_SEL5,
	PT_WaveformControlTrig_SEL6,
	PT_WaveformControlTrig_SEL7,
	PT_WaveformControlTrig_SEL8,
	PT_WaveformControlTrig_SEL9,
	PT_WaveformControlTrig_SEL10,
	PT_WaveformControlTrig_SEL11,
	PT_WaveformControlTrig_SEL12,
	PT_WaveformControlTrig_SEL13
};

enum DACWavePerChannelRunStopREG
{
	WaveformControlPT_SW_ArmASSERT_chan0 = 0,
	WaveformControlPT_SW_ArmASSERT_chan1,
	WaveformControlPT_SW_ArmASSERT_chan2,
	WaveformControlPT_SW_ArmASSERT_chan3,
	WaveformControlPT_SW_ArmASSERT_chan4,
	WaveformControlPT_SW_ArmASSERT_chan5,
	WaveformControlPT_SW_ArmASSERT_chan6,
	WaveformControlPT_SW_ArmASSERT_chan7,
	WaveformControlPT_SW_ArmASSERT_chan8,
	WaveformControlPT_SW_ArmASSERT_chan9,
	WaveformControlPT_SW_ArmASSERT_chan10,
	WaveformControlPT_SW_ArmASSERT_chan11,
	WaveformControlPT_SW_ArmASSERT_chan12,
	WaveformControlPT_SW_ArmASSERT_chan13
};

enum DACWaveGenPerChannelHWTrigControlReg
{
	WaveformControlPT_InputCfgIndx = 0,
	WaveformControlPT_OutCfgFn,
	WaveformControlPT_TruthTableCfg_SEL,
	WaveformControlPT_CfgWriteSTROBE
};

enum DACWaveGenPerChannelHWTrigSel1Reg
{
	WaveformControlPT_HW_In1_SEL_chan0,
	WaveformControlPT_HW_In1_SEL_chan1,
	WaveformControlPT_HW_In1_SEL_chan2,
	WaveformControlPT_HW_In1_SEL_chan3,
	WaveformControlPT_HW_In1_SEL_chan4,
	WaveformControlPT_HW_In1_SEL_chan5,
	WaveformControlPT_HW_In1_SEL_chan6,
	WaveformControlPT_HW_In1_SEL_chan7,
	WaveformControlPT_HW_In1_SEL_chan8,
	WaveformControlPT_HW_In1_SEL_chan9,
	WaveformControlPT_HW_In1_SEL_chan10,
	WaveformControlPT_HW_In1_SEL_chan11,
	WaveformControlPT_HW_In1_SEL_chan12,
	WaveformControlPT_HW_In1_SEL_chan13,
};

enum DACWaveGenPerChannelHWTrigSel2Reg
{
	WaveformControlPT_HW_In2_SEL_chan0,
	WaveformControlPT_HW_In2_SEL_chan1,
	WaveformControlPT_HW_In2_SEL_chan2,
	WaveformControlPT_HW_In2_SEL_chan3,
	WaveformControlPT_HW_In2_SEL_chan4,
	WaveformControlPT_HW_In2_SEL_chan5,
	WaveformControlPT_HW_In2_SEL_chan6,
	WaveformControlPT_HW_In2_SEL_chan7,
	WaveformControlPT_HW_In2_SEL_chan8,
	WaveformControlPT_HW_In2_SEL_chan9,
	WaveformControlPT_HW_In2_SEL_chan10,
	WaveformControlPT_HW_In2_SEL_chan11,
	WaveformControlPT_HW_In2_SEL_chan12,
	WaveformControlPT_HW_In2_SEL_chan13,
};

enum DACWaveGenPerChannelDigitalTrigSel
{
	WaveformControlPT_WaveformIN_SEL_chan0,
	WaveformControlPT_WaveformIN_SEL_chan1,
	WaveformControlPT_WaveformIN_SEL_chan2,
	WaveformControlPT_WaveformIN_SEL_chan3,
	WaveformControlPT_WaveformIN_SEL_chan4,
	WaveformControlPT_WaveformIN_SEL_chan5,
	WaveformControlPT_WaveformIN_SEL_chan6,
	WaveformControlPT_WaveformIN_SEL_chan7,
	WaveformControlPT_WaveformIN_SEL_chan8,
	WaveformControlPT_WaveformIN_SEL_chan9,
	WaveformControlPT_WaveformIN_SEL_chan10,
	WaveformControlPT_WaveformIN_SEL_chan11,
	WaveformControlPT_WaveformIN_SEL_chan12,
	WaveformControlPT_WaveformIN_SEL_chan13
};

enum DACWaveGenPerChannelEOFFreeze
{
	DACWavegenEOFFreezeEnable_chan0,
	DACWavegenEOFFreezeEnable_chan1,
	DACWavegenEOFFreezeEnable_chan2,
	DACWavegenEOFFreezeEnable_chan3,
	DACWavegenEOFFreezeEnable_chan4,
	DACWavegenEOFFreezeEnable_chan5,
	DACWavegenEOFFreezeEnable_chan6,
	DACWavegenEOFFreezeEnable_chan7,
	DACWavegenEOFFreezeEnable_chan8,
	DACWavegenEOFFreezeEnable_chan9,
	DACWavegenEOFFreezeEnable_chan10,
	DACWavegenEOFFreezeEnable_chan11,
	DACWavegenEOFFreezeEnable_chan12,
	DACWavegenEOFFreezeEnable_chan13,
	DACWavegenEOFFreezeEnable_chan14,
};

// GP?AuxSignals segregated because they are named bit fields
enum GPIAuxSignals  // INPUT bit fields (0x7)
{
	Global_Aux_GPI_0 = 0,
	Global_Aux_GPI_1,
	Global_Aux_GPI_2,
	Global_Aux_GPI_3,
	Global_Aux_GPI_4,
};
enum GPOAuxSignals // OUTPUT bit fields (0x30)
{
	Global_Aux_GPO_0 = 0,
	Global_Aux_GPO_1,
	Global_Aux_GPO_2,
	Global_Aux_GPO_3,
	Global_Aux_GPO_4,
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

enum ADCStreamControl3Reg
{
	ADCStream_MAFilterLength,        // bits 0-2
	ADCStream_MA_Filter_Sel,		 // bit 9
};

enum ADCStreamControl3PSampleOffset
{
	ADCStream_3PSampleOffset0,        // bits 0-7
	ADCStream_3PSampleOffset1,		  // bit 8-15
	ADCStream_3PSampleOffset2,		  // bit 16-23
	ADCStream_3PSampleOffset3		  // bit 24-31
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
	BPI_FLASH_MSB24,
	BPI_FLASH_MSB25,
	ImageAcqPT_HW_In1_SEL,
	ImageAcqPT_HW_In2_SEL,
	ImageAcqPT_HWSW_SEL,
	ImageAcqPT_InputCfgIndx,
	ImageAcqPT_OutCfgFn,
	ImageAcqPT_CfgWriteSTROBE,
	ImageAcqPT_DO_WaveformIN_SEL
};
enum GlobalReadDataREG
{
	hw_trig_readback,
	hw_trig_irq,
	SCRATCH_BITS_READ,
};

enum GlobalScratchREG
{
	SCRATCH_BITS_WRITE
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
enum DACWaveGenControlREG  // 0x240
{
	DAC_DMA_Chans_Active = 0,
	DAC_DMA_Playback_En0,
	DAC_DMA_Playback_En1,
	DAC_DMA_Playback_En2,
	DAC_DMA_Playback_En3,
	DAC_DMA_Playback_En4,
	DAC_DMA_Playback_En5,
	DAC_DMA_Playback_En6,
	DAC_DMA_Playback_En7,
	DAC_DMA_Playback_En8,
	DAC_DMA_Playback_En9,
	DAC_DMA_Playback_En10,
	DAC_DMA_Playback_En11,
	DAC_DMA_Playback_En12,
	DAC_DMA_Playback_En13,
	DAC_Filter_En0,
	DAC_Filter_En1,
	DAC_Filter_En2,
	DAC_Filter_En3,
	DAC_Filter_En4,
	DAC_Filter_En5,
	DAC_Filter_En6,
	DAC_Filter_En7,
	DAC_Filter_En8,
	DAC_Filter_En9,
	DAC_Filter_En10,
	DAC_Filter_En11,
	DAC_Filter_En12,
	DAC_Filter_En13,
	DAC_Sync_hsync0,
	DAC_Sync_hsync1,
	DAC_Sync_hsync2,
	DAC_Sync_hsync3,
	DAC_Sync_hsync4,
	DAC_Sync_hsync5,
	DAC_Sync_hsync6,
	DAC_Sync_hsync7,
	DAC_Sync_hsync8,
	DAC_Sync_hsync9,
	DAC_Sync_hsync10,
	DAC_Sync_hsync11,
	DAC_Sync_hsync12,
	DAC_Sync_hsync13,
	DACWaveGen_SOF_DelaySysClkCnt,  // [0x246] - 2-byte field
};

enum DACWaveGenSelExtDigTrigREG
{
	WaveformControlPT_HWSW_SEL_chan0 = 0,
	WaveformControlPT_HWSW_SEL_chan1,
	WaveformControlPT_HWSW_SEL_chan2,
	WaveformControlPT_HWSW_SEL_chan3,
	WaveformControlPT_HWSW_SEL_chan4,
	WaveformControlPT_HWSW_SEL_chan5,
	WaveformControlPT_HWSW_SEL_chan6,
	WaveformControlPT_HWSW_SEL_chan7,
	WaveformControlPT_HWSW_SEL_chan8,
	WaveformControlPT_HWSW_SEL_chan9,
	WaveformControlPT_HWSW_SEL_chan10,
	WaveformControlPT_HWSW_SEL_chan11,
	WaveformControlPT_HWSW_SEL_chan12,
	WaveformControlPT_HWSW_SEL_chan13,
};

enum class ThorDAQExperimentType
{
	Imaging = 0x00,
	DACFreeRunMode = 0x1,
	DACFreeRunModeDynamicLoad = 0x03,
	DACFreeRunModeStaticLoad = 0x05
};

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

// Breakout Box Analog Channels, Legacy - DBB1 and ABBx AND newer 3U IO Panel
// "e" to enum, to distinguish from legacy code
enum BBoxADchannels
{
	// the legacy "Slow" ADC read by DAC mez. card (same for ABBx and 3U IOPanel)
	eAI0 = 0,
	eAI1,
	eAI2,
	eI3,
	eI4,
	eI5,
	// These "slow" AI channels only exist on 3U IO Panel (read by MAX127 on I2C)
	eAI6,
	eAI7,
	eAI8,
	eAI9,
	eAI10,
	eAI11,
	eAI12,
	eAI13,
	// These 1 MSPS DAC set AO channels same on ABBx AND 3U IO Panel
	eAO0 = 14,
	eAO1,
	eAO2,
	eAO3,
	eAO4,
	eAO5,
	eAO6,
	eAO7,
	eAO8,
	eAO9,
	eAO10,
	eAO11,
};


typedef struct _BOB_DIO_Settings // Digitals
{
	UINT64 BNClabel; // 0-based BNC index (e.g. "D0" for 3U panel is "DIO1" for legacy DBB1)
	UINT64 FPGAioIndex; // for Option to copy Output source from different BNCconnection
	UINT64 MUX;  // CPLD (I2C) or FPGA control source - any number below 48 (0x30) is FPGA's MUX
	INT64 AuxMUX; // 0-based index necessary for FPGA controlled GPIO AuxMUX (-1 if unused)
	//BOOL bValue; // last read or written value, 0 or 1;
	UINT64 INvalue;  // binary value valid when DIO configured as INPUT
	UINT64 OUTvalue; // binary value valid when DIO configured as OUTPUT
	BOOL bOutputDir; // TRUE if DIRection is Out
} BOB_DIO_settings;
typedef struct _BOB_AO_Settings // Analogs
{
	UINT64 BNClabel; // 0-based BNC index (e.g. "D0" for 3U panel is "DIO1" for legacy DBB1)
	CHAR Polarity;   // 'U'nipolar or 'B'ipolar
	double VoltageRange; // e.g. 5 or 10 volts
	double dVolts;  // last read or written Voltage value
	INT32 i32Value; // last read or written ADC/DAC COUNTS value
} BOB_AO_settings;
typedef struct _BOB_AI_Settings // Analogs
{
	UINT64 BNClabel; // 0-based BNC index (e.g. "D0" for 3U panel is "DIO1" for legacy DBB1)
	CHAR Polarity;   // 'U'nipolar or 'B'ipolar
	double VoltageRange; // e.g. 5 or 10 volts
	double dVolts;  // last read or written Voltage value
	INT32 i32Value; // last read or written ADC/DAC COUNTS value
} BOB_AI_settings;


// This class is exported from the thordaq.dll
class CThordaq
{
public:
	// Constructor
	CThordaq(UINT boardNum, HANDLE kernelDevHandle);

	// Destructor
	~CThordaq();

private:
	BOARD_CONFIG_STRUCT Board_CONFIG_data; // store a copy of kernel driver board data when object is instantiated
public:
	BOOL _bProgressiveScan;
	UINT gPCIeBoardIndex; // for public reference (read) only
	WCHAR _wcMname[10] = L"XI2CBrd ";
	LPCWSTR _I2CmutexName;
	HANDLE _hXI2Cmutex;

	// Advanced Programmable Triggers
//	ProgrammableTrigger* ImageAcqTrigger;
	//	ProgrammableTrigger* WaveformControlTrigger[14];

	// The Legacy BreakOutBox (BOB) is "LBOB", then 3U Panel BOB "3BOB"
	CHAR BOB_HardwareType = 'X'; // undefined (default) - use USB to detect BOB connected (if any)
	CHAR* BOB_DIOconfiguration; // NumBOB_DIOs occurences of CharPerBOB_DIO, in ascending order, to/from API ;
	BOB_DIO_settings BOB_DIOSettings[TD_BOBDIODef::NumBOB_DIOs];
	BOB_AO_settings BOB_AOSettings[TD_BOBAODef::NumBOB_AOs];
	BOB_AI_settings BOB_AISettings[TD_BOBAIDef::NumBOB_AIs];

	THORDAQ_STATUS GetDMAcapability(   // API version
	);

	//Disconnect to the device.
	THORDAQ_STATUS DisconnectFromBoard(
	);

	// NWL-API based FPGA register read/write
	THORDAQ_STATUS WriteReadRegister(UINT read_write_flag, UINT register_bar_num, ULONGLONG register_card_offset, BYTE buffer[sizeof(UINT64)], ULONGLONG offset, ULONGLONG length, PSTAT_STRUCT completed_status);

	THORDAQ_STATUS CopyKernelBoardCfg();

	THORDAQ_STATUS GetBoardCfg(
		BOARD_INFO_STRUCT* board_info  // Returned structure
	);

	THORDAQ_STATUS GetLowFreqBoardCfg(
		LOW_FREQ_TRIG_BOARD_INFO_STRUCT* board_info  // Returned structure
	);

	// DZ NWL replacement - read all channels
	THORDAQ_STATUS CThordaq::APIReadFrames(
		UINT64* buffer_length,
		void* sySHostBuffer,
		double timeout_ms,
		ULONG transferFrames,
		BOOL isLastTransfer,
		BOOL& isCompleteImage);

	// Abort reading 
	THORDAQ_STATUS AbortPacketRead(
	);


	THORDAQ_STATUS WriteDDR3(
		PUINT8 HostSourceAddress,
		UINT64 DDR3DestinationAddress,
		UINT32 ByteLength);

	THORDAQ_STATUS ReadDDR3(
		UINT64 DDR3SourceAddress,
		PUINT8 HostDestinationAddress,
		PUINT32 ByteLength);

	// NWL driver replacement for Carl's function
	THORDAQ_STATUS PacketWriteBuffer(
		ULONG64 register_card_offset,           // Start address to read in the card
		ULONG   Length,            // Size of the Packet Write to Data Buffer Count by Byte (FIFO Mode)
		UCHAR* Buffer,            // Data buffer (Packet Mode)
		ULONG   Timeout            // Generate Timeout error when timeout
	);


	// Replaces original SetImagingConfiguration for ThorImageLS compatibility
	THORDAQ_STATUS CThordaq::APIimageAcqConfig(DATA_ACQ_CTRL_STRUCT* imaging_config);

	//Set up data imaging configuration
	THORDAQ_STATUS SetImagingConfiguration(
		IMAGING_CONFIGURATION_STRUCT imaging_config // Image configuration stuct
	);

	//Set up data imaging configuration
	THORDAQ_STATUS SetImagingWaveforms(
		std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT> dacCtrl,
		std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT> dacCtrl2 // use this one for 2bank acquisitions to set the second bank
	);

	/******************************************
	* ThorDAQ DAC Control Functions
	******************************************/
	THORDAQ_STATUS SetDACParkValue(ULONG32 outputChannel, double outputValue);
	THORDAQ_STATUS SetDACOffsetValue(ULONG32 outputChannel, double outputValue);
	THORDAQ_STATUS ToggleAuxDigitalOutputs(USHORT auxChannelIndex, USHORT value);
	THORDAQ_STATUS SetThreePhotonSampleOffset(UINT offset, UINT8 threePhtonSampleOffset);

	/******************************************
	* ThorDAQ Control Functions
	******************************************/
	THORDAQ_STATUS SetADCGain(UINT8 channel, UCHAR gain);
	THORDAQ_STATUS SetAllADCChannelsGain(ULONG clock_source, ULONG32 adcGain[], bool forceGainUpdate);

	/******************************************
	* ThorDAQ IO Status Read Functions
	******************************************/
	THORDAQ_STATUS GetLineTriggerFrequency(UINT32 sample_rate, double& frequency, UINT32 expectedFrequency);
	THORDAQ_STATUS GetTotalFrameCount(UINT32& frame_count);

	// Capture API Functions
	THORDAQ_STATUS GlobalSCANstart(bool Start);
	//Start acquisition
	THORDAQ_STATUS StartAcquisition();

	//Stop acquisition
	THORDAQ_STATUS StopAcquisition();

	THORDAQ_STATUS CThordaq::APIBreakOutBoxLED(UINT boardNum, INT32 ConnectorEnum, INT32 Range, INT32 BiPolar, UINT32* ADCcounts);

	THORDAQ_STATUS SetClockSourceAndFrequency(ULONG clock_source, UINT8 set_flag = TRUE);

	THORDAQ_STATUS SetGRClockRate(ULONG32 adc_sample_rate, ULONG32 expectedFrequency);

	THORDAQ_STATUS MeasureExternClockRate(ULONG32& clock_rate, ULONG32& clock_ref, ULONG32 mode);

	THORDAQ_STATUS SetThreePhotonModeEnable(bool enableThreePhton);

	THORDAQ_STATUS GetExternClockStatus(ULONG32& isClockedSynced);

	THORDAQ_STATUS SetDCOffsetPreFIR(short preDcOffset, USHORT channel);


	THORDAQ_STATUS DACReArmWaveplayIRQ();

	THORDAQ_STATUS DACStartWaveforms();

	THORDAQ_STATUS DACStopWaveforms();

	THORDAQ_STATUS DACDynamicLoadWaveform(std::map<UINT, DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT> dacCtrlSettings, bool isLastPartOfWaveform);

	THORDAQ_STATUS DACDynamicLoadPresetWaveform(bool isLastPartOfWaveform);

	THORDAQ_STATUS DACPresetNextWaveformSection(std::map<UINT, DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT> dacCtrlSettings, bool isLastPartOfWaveform);

	THORDAQ_STATUS DACGetMinSamples(double dacUpdateRate, UINT64& minSamples);

	THORDAQ_STATUS DACSetWaveformConfigurationForStaticLoading(DAC_FREERUN_WAVEFORM_CONFIG dacConfig);

	THORDAQ_STATUS DACSetWaveformConfigurationForDynamicLoading(DAC_FREERUN_WAVEFORM_CONFIG dacConfig);

	THORDAQ_STATUS DACRegisterApproachingNSamplesEvent(UINT8 dacChannel, UINT32 nSamples, UINT32 options, ThorDAQDACApproachingNSamplesCallbackPtr callbackFunction, void* callbackData);

	THORDAQ_STATUS DACRegisterWaveformPlaybackCompleteEvent(UINT32 options, ThorDAQDACWaveformPlaybackCompleteCallbackPtr callbackFunction, void* callbackData);

	THORDAQ_STATUS DACRegisterCycleDoneEvent(UINT8 dacChannel, UINT32 options, ThorDAQDACCycleDoneCallbackPtr callbackFunction, void* callbackData);

	THORDAQ_STATUS DACRegisterApproachingLoadedWaveformEndEvent(UINT8 dacChannel, UINT32 options, ThorDAQDACApproachingLoadedWaveformEndCallbackPtr callbackFunction, void* callbackData);

	THORDAQ_STATUS DACRegisterWaveformPlaybackStartedEvent(UINT32 options, ThorDAQDACWaveformPlaybackStartedCallbackPtr callbackFunction, void* callbackData);

	THORDAQ_STATUS DACBankSwitchingLoadNextWaveform(std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>  dacCtrlSettings);

	THORDAQ_STATUS DACBankSwitchingRegisterReadyForNextImageWaveformsEvent(UINT32 options, ThorDAQDACBankSwitchingReadyForNextWaveformCallbackPtr callbackFunction, void* callbackData);

	THORDAQ_STATUS DACBankSwitchingClearRegisteredReadyForNextImageWaveformsEventHandlers();

	THORDAQ_STATUS SetDIOChannelSelection(std::vector<string> DIOSelection);

	THORDAQ_STATUS SetScanActiveLineInvert(bool invertCaptureActiveLine);

	THORDAQ_STATUS SetDefaultBOBMapping(bool initialParkAllAtZero);

	THORDAQ_STATUS SlowSmoothMoveToAndFromParkEnable(bool enable);

	THORDAQ_STATUS SetDDSClockEnable(bool enable);

	THORDAQ_STATUS SetDDSClockPhase(int channel, double phase);

	THORDAQ_STATUS GetBOBType(THORDAQ_BOB_TYPE& bobType);
private:
	typedef struct _DAC_WAVE_GROUP_STRUCT
	{
		ULONG64 waveform_buffer_start_address;
		ULONG64 waveform_buffer_size;
		USHORT* waveformBuffer;
		bool isLoopStart;
		bool isLoopEnd;
		bool loopEnable;
		USHORT loopCount;
		ULONG64 loopStartGroupIndex;
		bool needsMemoryCleanup;
	}DAC_WAVE_GROUP_STRUCT, * PDAC_WAVE_GROUP_STRUCT;

	typedef struct _DAC_CRTL_STRUCT // need to rename so it's only for imaging
	{
		ULONG64 output_port;
		double offset_val;
		double park_val;
		double update_rate;
		ULONG64 flyback_samples;
		bool enablePort;
		bool enableFilter;
		bool filterInhibit;
		THORDAQ_TRIGGER_SETTINGS triggerSettings;
		bool hSync;
		bool enableEOFFreeze;

	}DAC_CRTL_STRUCT, * PDAC_CRTL_STRUCT;

	typedef struct _DAC_WAVE_DESC_STRUCT
	{
		bool finishPlaybackInLastGroup;
		std::vector<DAC_WAVE_GROUP_STRUCT> dacWaveformGroups;

	}DAC_WAVE_DESC_STRUCT, * PDAC_WAVE_DESC_STRUCT;

	//The assumption is that the first the waveform is continuous between the first and the last
	typedef struct _DAC_DESC_TABLE_SETUP_TRACKER
	{
		UINT64 firstDescIndex;
		UINT64 secondDescIndex;
		UINT64 lastDescIndex;
		UINT64 maxNumberOfDescriptors;
		UINT64 totalSamples;
	}DAC_DESC_TABLE_SETUP_TRACKER, * PDAC_DESC_TABLE_SETUP_TRACKER;

	typedef struct _DAC_DESC_TABLE_SAMPLES_TRACKER
	{
		UINT64 descIndex;
		UINT64 nxtDexc;
		UINT8 channelIndex;
		UINT64 size;
		UINT64 address;
	}DAC_DESC_TABLE_SAMPLES_TRACKER, * PDAC_DESC_TABLE_SAMPLES_TRACKER;

	typedef struct _DACCycleDoneCallbackSettings
	{
		ThorDAQDACCycleDoneCallbackPtr callbackFunction;
		UINT32 options;
		void* callbackData;
	}DACCycleDoneCallbackSettings, * PDACCycleDoneCallbackSettings;

	typedef struct _DACWaveformPlaybackCompleteSettings
	{
		ThorDAQDACWaveformPlaybackCompleteCallbackPtr callbackFunction;
		UINT32 options;
		void* callbackData;
	}DACWaveformPlaybackCompleteSettings, * PDACWaveformPlaybackCompleteSettings;

	typedef struct _DACApproachingNSamplesCallbackSettings
	{

		ThorDAQDACApproachingNSamplesCallbackPtr callbackFunction;
		UINT32 numberOfSamples;
		UINT32 options;
		void* callbackData;
	}DACApproachingNSamplesCallbackSettings, * PDACApproachingNSamplesCallbackSettings;

	typedef struct _DACApproachingLoadedWaveformEndSettings
	{
		ThorDAQDACApproachingLoadedWaveformEndCallbackPtr callbackFunction;
		UINT32 options;
		void* callbackData;
	}DACApproachingLoadedWaveformEndSettings, * PDACApproachingLoadedWaveformEndSettings;

	typedef struct _DACWaveformPlaybackStartedSettings
	{
		ThorDAQDACWaveformPlaybackStartedCallbackPtr callbackFunction;
		UINT32 options;
		bool hasBeenCalled = false;
		void* callbackData;
	}DACWaveformPlaybackStartedSettings, * PDACWaveformPlaybackStartedSettings;


	typedef struct _DACBankSwitchingReadyForNextWaveformCallbackSettings
	{
		ThorDAQDACBankSwitchingReadyForNextWaveformCallbackPtr callbackFunction;
		UINT32 options;
		void* callbackData;
	}DACBankSwitchingReadyForNextWaveformCallbackSettings, * PDACBankSwitchingReadyForNextWaveformCallbackSettings;

private:
	HANDLE								gHdlDevice;
	HDEVINFO							gDeviceInfo;				 // The handle to a device information set that contains all installed devices that matched the supplied parameters
	UINT								gBoardIndex;				 // The index number to record board number
	//PSP_DEVICE_INTERFACE_DETAIL_DATA	gDeviceInterfaceDetailData;  // The structure to receice information about device specified interface
	UINT64			                    pRxPacketBufferHandle;       // restore NWL
	DMA_INFO_STRUCT						gDmaInfo;					 // The structure to retrieve the board dma configuration
	DATA_ACQ_CTRL_STRUCT* gPtrAcqCtrl;                 // The Global Pointer to Thordaq acquisition configuration struct
	ULONG64								_dacDescpListIndex;
	USHORT								_auxDigitalOutToggleVal;	 // Global 16-bit variable used to save the toggle value set for all aux digital channels (current: 5 channels use 5 first bits)
	int _S2MMDMAStatusRegIndex[MAX_CHANNEL_COUNT];
	int _S2MMDMAControlRegIndex[MAX_CHANNEL_COUNT];
	UINT64 _reArmS2MMDMACommand[MAX_CHANNEL_COUNT];
	FilterFactory* filterFactory;
	Filter* filter;
	atomic<bool>						_abortReadImage;
	const UINT64 LegacyDACdefaultChannelMap = 0x0000BA9876543210;	// ABBx settings consistent with Legacy TILS system use (i.e. "SetParkValue -c" channels not contiguous)

	const UINT64 Panel3UDACdefaultChannelMap = 0x0000AB3276549810;	// ABBx settings for Rev3 3U Panel hardware (puts all AOn in continguous Wavetable DMA_CHn order per 3U PCB layout)
	UINT64								_DAC_DDR3_startAddress;       // beginning of DDR3 physical memory for DAC waveforms (DDR3 ADC /images in low mem @ 0)
	DAC_WAVEPLAY_DESCRIPTOR             _DACwavePlayDescriptorTable[DAC_DESCP_LENGTH];
	UINT32                              _DACwaveformDescriptorsPerChannel;  // if FIXED waveformBuffer size mode, else 0
	UINT32								_DACwavePlayDescriptorTableFreeIndex = 14; // hardware defined start at 14 (12 analog channels + 2 Digital out channel)
	UINT64								_DAC_DDR3startAddress;
	FPGA_HardwareReg* _FPGAregister[TD_LAST_FPGA_REGISTER + 1];
	HANDLE								_nwlDMAdescriptorsMutex;
	UINT32								_NWL_Common_DMA_Register;	  // snapshot of Register from Interrupt (DPC) handler (currently NOT working)
	USER_IRQ_WAIT_STRUCT				_usrIrqWaitStruct;            // contains DMA Bank indicator, int. timeout, etc.
	BOOL								_s2mmBankLastRead;			  // used to keep track of the last read s2mm bank
	UINT64								_dacWaveformCurrentCycleCount;// used to keep track of the current cycle count, when running dac waveforms without imaging timing
	UINT64								_DACWaveGenInterruptCountWhenApproachingCallbackCalled[WAVETABLE_CHANNEL_COUNT]; // counts number of recv'd interrupts since GlobalSTART
	UINT64								_DACWaveGenInterruptCounter[WAVETABLE_CHANNEL_COUNT]; // counts number of recv'd interrupts since GlobalSTART
	UINT64								_dacStaticLoadNumberOfDescPerInterrupt;
	UINT64								_dacStaticLoadInterruptsPerCycle;
	std::thread* _irqThread;					  // thread to check for interrupts
	ThorDAQExperimentType				_experimentType;
	atomic<bool>						_imagingStartStopStatus;
	atomic<bool>						_dacContinuousModeStartStopStatus;
	atomic<bool>						_dacAbortContinousMode;
	DAC_FREERUN_WAVEFORM_CONFIG			_dacCtrl;
	DAC_DESCP_TABLE* _dacDMADescTable;
	std::mutex _dacDescriptorMutex;
	DAC_MULTIBANK_DESCP_TABLE* _dacDMAMultibankDescTable;
	UINT64														_dacDescCountToTriggerApproachingNSamplesEvent[WAVETABLE_CHANNEL_COUNT];
	UINT64														_dacDescCountToTriggerApproachinLoadedWaveformEndEvent[WAVETABLE_CHANNEL_COUNT];
	UINT64														_dacSampleCountToTriggerCycleCompleteEvent[WAVETABLE_CHANNEL_COUNT];
	UINT64														_dacDescriptorsPerChannel[WAVETABLE_CHANNEL_COUNT];
	std::map<UINT, DACCycleDoneCallbackSettings>				_dacCycleDoneCallbackPtrs;
	std::map<UINT, DACApproachingNSamplesCallbackSettings>		_dacApproachingNSamplesCallbackPtrs;
	std::map<UINT, DACApproachingLoadedWaveformEndSettings>		_dacApproachinLoadedWaveformEndCallbackPtrs;
	DACWaveformPlaybackCompleteSettings							_dacWaveformPlaybackCompletePtr;
	DACWaveformPlaybackStartedSettings							_dacWaveformPlaybackStartedCallbackPtr;
	std::map<UINT, DAC_WAVE_DESC_STRUCT>						_dacPresetDynamicLoadWaveDescs;
	atomic<bool>												_dacProcessingPresetWaveformForDynamicLoading;
	atomic<bool>												_dacPresetWaveformForDynamicLoadingReady;
	atomic<bool>												_dacDynamicLoadingPresetWaveform;
	size_t														_dacDynamicLoadLastDescSampleCount;
	std::map<UINT, DAC_DESC_TABLE_SETUP_TRACKER>				_dacDescTableTracker;
	std::map<UINT, std::vector<DAC_DESC_TABLE_SAMPLES_TRACKER>>	_dacDescTableSamplesTracker;
	std::thread* _dacDescriptorCountTrackingThread;					  // thread to keep track of the descriptor count
	std::map<UINT, bool> _dacWaveformImagingChannels;
	std::map<UINT, bool> _dacWaveformFreeRunModeChannels;
	std::map<UINT, double> _dacParkingPositions;
	std::map<UINT, USHORT> _dacParkingPositionsFPGA;
	std::map<UINT8, bool> _dacContinuousModeEnabledChannels;
	UINT64 _rearmDAC1;
	UINT64 _rearmDAC0;
	ShutDownType _lastShutdownTypeSinceFPGAConnection;
	bool _dacChannelsInitParkedAtZero;
	bool _isPreviewImaging;
	bool _stoppingDAC;
	UINT64													_dacSecondBankWaveformDDR3StartAddress;
	UINT64 _dacBankSwitchingPerChannelAddressOffset[WAVETABLE_CHANNEL_COUNT];
	std::vector<DACBankSwitchingReadyForNextWaveformCallbackSettings>	_dacBankSwitchingReadyForNextWaveformCallbackSettingsList;
	std::thread* _imagingDACBankSwitchingTrackingThread;					  // thread to keep track of the bank switching
	std::thread* _imagingTrackLevelTriggerAndRearmS2MMThread;					  // thread to keep track of the bank switching
	atomic<bool> _imagingLevelTriggerWentLow;
	bool _imagingLevelTriggerActive;
	UINT16 _DACPerChannelRuntBitSelection;
	bool _imagingEventPrepared;
	bool _dacContinuousModeEventPrepared;
	int _DACWaveGen3PSyncControlRegIndex = -1;
	int _DACWaveGenISRIndex = -1;
	int _nullBitField = -1;
	bool _isDACChannelEnabledForImaging[WAVETABLE_CHANNEL_COUNT];
	bool _captureActiveLinveInvert;
	int _enableDownsamplingRateChange;
	int _threePhotonDownsamplingRate;
	static wchar_t	thordaqLogMessage[MSG_SIZE];
	INT32 _MAfilterAlignmentOffset;
	std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT> imagingDACCtrl;
	std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT> imagingDACCtrl2; // use this one for 2bank acquisitions to set the second bank at the beggining

private: void CThordaq::UpdateDACWavePlayDMFPGAShadowRegister(UINT32 DACchannel, PDAC_WAVEPLAY_DESCRIPTOR desc);
public: void CThordaq::FPGAregisterWriteDoMem(int Record);
public: UINT64 CThordaq::FPGAregisterReadDoMem(int Record);
public: UINT8 CThordaq::BOBLEDstateArray[BBoxLEDenum::AI13 + 1] = {}; // 3U BOB, 32 DIO, 6 diagnostic, 14 AIs, 12 AOs (64), stateArray written by blink thread
public: UINT8 CThordaq::BOBLEDcmdArray[BBoxLEDenum::AI13 + 1] = {}; // Command array written by API, read by blink thread

	  /////// restore NWL API, and define DZ SDK based on that API /////////////////
public: THORDAQ_STATUS CThordaq::DoMem(
	UINT32          Rd_Wr_n,        // 1==Read, 0==Write
	UINT32          BarNum,         // Base Address Register (BAR) to access
	PUINT8          Buffer,         // Data buffer
	UINT64          Offset,         // Offset in data buffer to start transfer
	UINT64          CardOffset,     // Offset in BAR to start transfer
	UINT64          Length,         // Byte length of transfer
	PSTAT_STRUCT    Status          // Completion Status
);

public: THORDAQ_STATUS CThordaq::SetupPacket(
	INT32   EngineOffset,       // DMA Engine number offset to use
	PUINT8  Buffer,
	PUINT32 BufferSize,
	PUINT32 MaxPacketSize,
	INT32   PacketModeSetting,
	INT32   NumberDescriptors

);
public: THORDAQ_STATUS CThordaq::PacketReadEx(
	INT32                   EngineOffset,       //
	PUINT64                 UserStatus,         //
	UINT64                  CardOffset,
	UINT32                  Mode,               //
	PUINT8                  Buffer,
	PUINT32                 Length
);
public: THORDAQ_STATUS CThordaq::PacketWriteEx(
	INT32   EngineOffset,           // DMA Engine number offset to use
	UINT64  UserControl,
	UINT64  CardOffset,
	UINT32  Mode,                   // Control Mode Flags
	PUINT8  Buffer,
	UINT32  Length
);
	  //Get DMA Engine Capabilitie
	  THORDAQ_STATUS GetDMAEngineCap(
		  ULONG			dma_engine_offset,	// DMA Engine number to use
		  PDMA_CAP_STRUCT dma_capability		// Returned DMA Engine Capabilitie
	  );

	  THORDAQ_STATUS UserIRQWait(USER_IRQ_WAIT_STRUCT* pUserIrqWaitStruct, UINT64* IrqCnt0, UINT64* IrqCnt1, UINT64* IrqCnt2, UINT64* IrqCnt3);

	  THORDAQ_STATUS UserIRQCancel(VOID);

	  THORDAQ_STATUS APIProgressiveScan(BOOL bProgressiveScan);

	  THORDAQ_STATUS ReleasePacketBuffers(
		  INT32				dma_engine_offset // DMA Engine number to use
	  );


	  LONG SetGlobalSettings(IMAGING_CONFIGURATION_STRUCT	imaging_config);

	  LONG SetScanSettings(IMAGING_CONFIGURATION_STRUCT	imaging_config);

	  LONG SetCoherentSampleingSettings(IMAGING_CONFIGURATION_STRUCT	imaging_config);

	  LONG SetStreamProcessingSettings(IMAGING_CONFIGURATION_STRUCT	imaging_config);

	  LONG SetDACSettingsForImaging(IMAGING_CONFIGURATION_STRUCT	imaging_config, std::map<UINT, DAC_CRTL_STRUCT> dacCtrl, bool setDacSettingsForUnusedChannels);

	  LONG SetDACPerChannelSettingsForImaging(UINT channel, DAC_CRTL_STRUCT dacCtrl);

	  LONG SetImagingTriggerOptions(IMAGING_CONFIGURATION_STRUCT imaging_config);

	  //Set up Acquisition Look Up Table
	  LONG SetLUTSettings(
		  SCAN_LUT& scanLUT,
		  IMAGING_CONFIGURATION_STRUCT	imaging_config // Image configuration stuct
	  );

	  //Set up Acquisition FIR Filters
	  LONG SetFIRFilterSettings3P(double& sample_rate, double pixel_frequency, IMAGING_CONFIGURATION_STRUCT	imaging_config);

	  LONG SetFIRFilterSettings(FIRFilterMode filter_mode, double& sample_rate, double pixel_frequency, IMAGING_CONFIGURATION_STRUCT imaging_config);
	  // DS SDK

	  bool CThordaq::SearchFPGAregFieldIndices(
		  const char* name,
		  int nameSize,
		  int* pRecord,
		  int* pField);
	  IMAGING_CONFIGURATION_STRUCT _imagingConfiguration;
private:
	THORDAQ_STATUS SetADCInterfaceSettings(IMAGING_CONFIGURATION_STRUCT imaging_config);

	void LogMessage(wchar_t* logMsg, long eventLevel);

	LONG ExportScript(DATA_ACQ_CTRL_STRUCT* gPtrAcqCtrl, SCAN_LUT scanLUT);

	BYTE* GetNewBufferOfSize(size_t bufferSize);

	ShutDownType GetShutDownTypeSinceLastConnection(INT64 lastConnectionTime); // Find GG start location
	void TriggerLogicResponse(THORDAQ_TRIGGER_SETTINGS triggerSettings, BYTE lutAddress, bool& do0, bool& do1);
	bool IsDACMappingConfigured();
	THORDAQ_STATUS DACSetParkValueForChannels(std::map<UINT, USHORT> dacParkPosition);
	LONG DACLoadWaveformAndDescriptors(std::map<UINT, DAC_WAVE_DESC_STRUCT> dac_settings, ThorDAQExperimentType experimentType, bool preDynamicLoad = false);
	LONG DACSetDMADescriptors(DMADescriptors& dmaDescpGroup, ULONG64 chanIndex, ULONG64* dmaDescpTable, ULONG64 descpTableLength, bool preDynamicLoad);
	THORDAQ_STATUS DACResetWaveGenInterrupts();
	THORDAQ_STATUS DACSetWaveGenInterruptsOnChannel(UINT8 dacChannel, UINT8 descCntBeforeInterrupt);
	THORDAQ_STATUS DACSetChannelsSettings(std::map<UINT, DAC_CRTL_STRUCT> dacCtrl);
	THORDAQ_STATUS DACTrackCompletedDescriptorsDynamicLoadingWaveform();
	THORDAQ_STATUS DACTrackCompletedDescriptorsStaticWaveform();
	THORDAQ_STATUS DACResetContinuousModeOnAllChannels();
	THORDAQ_STATUS DACEnableDisableContinuousModeOnChannel(UINT8 dacChannel, bool enable);
	THORDAQ_STATUS DACEnableDisablePerChannelRunStopOnChannel(UINT8 dacChannel, bool enable);
	THORDAQ_STATUS DACSetTriggerSettingsOnChannel(UINT8 dacChannel, THORDAQ_TRIGGER_SETTINGS triggerSettings);
	THORDAQ_STATUS DACSetContinuousModeTriggerLUTOnChannel(UINT8 lutAddress, UINT8 dacChannel, bool d0, bool d1);
	THORDAQ_STATUS DACCreateDACWaveformGroupsLongWaveformForDynamicLoadSetup(DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT* originalConfig, UINT64& totalDescriptors, std::vector<DAC_WAVE_GROUP_STRUCT>& wavegroups);
	THORDAQ_STATUS DACCreateDACWaveformGroupsShortWaveformForDynamicLoadSetup(DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT* originalConfig, UINT64& totalDescriptors, std::vector<DAC_WAVE_GROUP_STRUCT>& wavegroups);
	std::vector<DAC_WAVE_GROUP_STRUCT> DACCreateDACWaveformGroupsStaticLoadSetup(DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT* originalConfig, UINT64 loopCount, UINT64& totalDescriptors);
	std::vector<DAC_WAVE_GROUP_STRUCT> DACCreateDACWaveformGroupsLastSectionStaticLoadSetup(DAC_WAVEFORM_PLAYBACK_CRTL_STRUCT* originalConfig, UINT64& totalDescriptors);

	LONG DACBankSwitchingLoadWaveformAndDescriptors(std::map<UINT, DAC_WAVE_DESC_STRUCT> dac_settings, UINT8 bankIndx, bool preDynamicLoad = false);
	THORDAQ_STATUS ImagingDACBanckSwitchingTrackCurrentBank();
	THORDAQ_STATUS DACSetContinuousModeGeneralSettings(std::map<UINT, DAC_WAVE_DESC_STRUCT> waveform, std::map<UINT, DAC_CRTL_STRUCT> dacCtrl, bool dynamicWaveform, std::map<UINT8, bool> dacEnabledChannels, DAC_FREERUN_WAVEFORM_GENERAL_SETTINGS* generalSettings);
	THORDAQ_STATUS ImagingTrackLevelTriggerAndRearmS2MM();

	THORDAQ_STATUS SetDefaultDIOConfig();
	// This function returns zero based index of API defined FPGA indexes based on BARindex and BARoffset
	// Returns SUCCESS status and valid FPGAregisterIndex if found
public: THORDAQ_STATUS CThordaq::FPGAregisterQuery(int BARindex, unsigned int BARoffset, UINT ReadWriteDir, int* FPGAregisterIndex);
	  THORDAQ_STATUS APIWaitForUserIRQ(UINT64* CountOfChan0_INTs, UINT64* CountOfChan1_INTs, UINT64* CountOfChan2_INTs, UINT64* CountOfChan3_INTs);
	  THORDAQ_STATUS APICancelWaitForUserIRQ();

public: THORDAQ_STATUS CThordaq::FPGAregisterQuery(
	int RegIndex,           // the 0-based index of all defined registers - starts with 0 psuedo REG, which is VERSION info
	int FldIndex,           // 0-based index of defined bit field within register (if any)
	char* pName,			// returned name of register/field (if found)
	int pNameSize,
	PREG_FIELD_DESC RegFldDesc // FPGA register/field detailed hardware description (BAR, offset, etc.)
);

public: void CThordaq::FPGAregisterShadowFPGAwrite(int Record, int Field, UINT64 value);
public: THORDAQ_STATUS CThordaq::FPGAregisterWrite(
	const char* pName,			// name of register/field (if found)
	int nameSize,
	UINT64  Value           // register/field value to write (from 1 to 64 bits)
);

	  // for performance, provide register write by index (no Bitfield!)
public: THORDAQ_STATUS CThordaq::FPGAregisterWrite(
	int Record, 			// zero based register array index
	UINT64 Value           // register/field value - either from FPGA hardware or shadow copy
);

public: THORDAQ_STATUS CThordaq::FPGAregisterRead(
	int Record, 			// zero based register array index
	int Field,				// zero based field index within register (or -1 for no field)
	UINT64* Value           // register/field value - either from FPGA hardware or shadow copy
);

public: THORDAQ_STATUS CThordaq::FPGAregisterRead(
	const char* pName,			// name of register/field (if found)
	int nameSize,
	UINT64* Value           // "shadow" DLL register copy value (REG or FIELD)
);


public: THORDAQ_STATUS CThordaq::DACwaveInit(
	UINT64 DDR3startAddr,          // location in physical DDR3 mem to start (ADC images in low memory)      
	UINT32 CPLDfrequency           // speed of 16-bit sample read (from DDR3 mem)
);

public: THORDAQ_STATUS CThordaq::DACwaveLoad(
	UINT32 DACchannel,             // DAC hardware channel 
	PUINT8 DACsampleBuffer,        // array of 16-bit waveformBuffer samples to load
	UINT32 BufferSize,             // Total size (bytes) of sample buffer
	PUINT64 DDR3startAddr     // if successful, the DDR3 start addr where waveformBuffer is loaded
);
public: THORDAQ_STATUS CThordaq::S2MMconfig(
	PS2MM_CONFIG pS2MMconfig);

public: THORDAQ_STATUS CThordaq::APIProgrammableTrigger(
	CThordaq& TdBrd, // This CThordaq object by reference	
	signed char chan,
	bool ArmASSERT
);

public:	THORDAQ_STATUS  CThordaq::APIXI2CReadWrite(
	CThordaq& TdBrd, // This CThordaq object by reference	
	bool bRead,
	UINT32 MasterMUXAddr,
	UINT32 MasterMUXChan,
	UINT32 SlaveMUXAddr,
	UINT32 SlaveMUXChan,
	UINT32 DevAddress,
	UINT32 I2CbusHz,    // frequency of bus, i.e. 100 or 400 Hz (or ...)
	INT32 PageSize,
	PUINT8 OpCodeBuffer,
	UINT32* OpCodeLen,
	PUINT8 DataBuffer,
	UINT32* DataLen // IN: expect returned bytes Rx'd from slave  OUT: actual bytes (can be 0), Opcodes+Data
);

public: THORDAQ_STATUS CThordaq::APIReadI2CDevice(
	CThordaq& TdBrd,
	UINT32 MasterChan,
	INT32 SlaveChan,
	UINT32 DevAddress,
	UINT32 SlaveCommandByte,
	PUINT8 ReadBuffer,
	UINT32* ByteLen  // IN: ReadBuffer Len, OUT: byte length read
);
public: THORDAQ_STATUS CThordaq::APIWriteI2CDevice(
	CThordaq& TdBrd,
	UINT32 MasterChan,
	INT32 SlaveChan,
	UINT32 DevAddress,
	UINT32 SlaveCommandByte,
	PUINT8 WriteBuffer,
	UINT32* ByteLen  // IN: ReadBuffer Len, OUT: byte length read
);

	  /*public: THORDAQ_STATUS CThordaq::APIFlashI2CSlave(
		  CThordaq& TdBrd,
		  UINT32 MasterChan,
		  INT32 SlaveChan,
		  UINT32 DevAddress,
		  UINT32 SlaveCommandByte,
		  PUINT8 WriteBuffer,
		  UINT32* ByteLen  // IN: ReadBuffer Len, OUT: byte length read
	  );*/

public: THORDAQ_STATUS CThordaq::APIGetAUXGPIOvalue(
	CThordaq& TdBrd,
	UINT32 MUXindex,
	UINT32* BitValue
);

public:  THORDAQ_STATUS CThordaq::APIGetDDR3status( // returns current DDR3 mem card info
	CThordaq& TdBrd,
	CHAR* StatusString,
	UINT32 StringChars
);

public:  THORDAQ_STATUS CThordaq::APIGetBOBstatus( // returns current Breakout Box status
	CThordaq& TdBrd,          // Board to target
	CHAR* StatusString,
	UINT32 StringChars
);

public: THORDAQ_STATUS APIGetAI(
	CThordaq& TdBrd,
	INT32 BNCindex,
	BOOL bVolts,
	double* Value
);

	  //private: THORDAQ_STATUS CThordaq::Read_MAX127_AI(
	  //	CThordaq& TdBrd,                // Board to target
	  //	int BNClabel                    // the BNC connection identified on BOB
	  //	);
private: THORDAQ_STATUS CThordaq::GetDIOvalueForFPGAindex(
	CThordaq& TdBrd,
	int iFPGAindex,
	int* BNCindex,
	uint8_t* uiValue);

private:  THORDAQ_STATUS CThordaq::ReadWriteBOB_CPLD_DIO(
	CThordaq& TdBrd,                // Board to target
	int BNClabel    // BNC index on 3U BOB
);

private: THORDAQ_STATUS CThordaq::SwapABBxMUX(
	CThordaq& TdBrd,                // Board to target
	int BNClabel,
	int DMAchIndex
);

public: THORDAQ_STATUS CThordaq::APISetAIOConfig(
	CThordaq& TdBrd,                // Board to target
	CHAR* config,
	UINT32 configSize
);
public: THORDAQ_STATUS CThordaq::APIGetAIOConfig(
	CThordaq& TdBrd,                // Board to target
	CHAR* config,
	UINT32 configSize
);
public: THORDAQ_STATUS CThordaq::APIGetDIOConfig(
	CThordaq& TdBrd,                // Board to target
	CHAR* config,
	UINT32 configSize
);
public: THORDAQ_STATUS CThordaq::APISetDIOConfig(
	CThordaq& TdBrd,                // Board to target
	CHAR* config,
	UINT32 configSize
);
public: THORDAQ_STATUS CThordaq::APISetCPLD_DIOConfig(
	CThordaq& TdBrd,                // Board to target
	int BNClabel,
	int CopiedSourcelabel,
	int FPGA_HSC, // 1 if High Speed source DBB1
	int Input     // 1 if Input, 0 if Output (FPGA AUX define)
);

public: THORDAQ_STATUS CThordaq::APISetDO( // SET the value of one or more DOs
	CThordaq& TdBrd,
	CHAR* config,            // Collection of fields per Config functions - AUX field holds value
	UINT32 configSize,       // field size 
	UINT32 NumDOs            // 1 or more DO records in "config"
);
public: THORDAQ_STATUS CThordaq::APIGetDIO( // SET the value of one or more DOs
	CThordaq& TdBrd,
	CHAR* config,            // Collection of fields per Config functions - AUX field holds value
	UINT32 configSize,       // field size 
	UINT32 NumDOs            // 1 or more DO records in "config"
);

private: THORDAQ_STATUS  CThordaq::GetOrSetBOB_AIO(CThordaq& TdBrd, INT32* AIOArray, double* dValues, BOOL bVolts, INT32 AIOCount);
private: THORDAQ_STATUS  CThordaq::GetOrSetBOB_DIO(CThordaq& TdBrd, CHAR* ConfigArray, int ConfigArraySize, int NumDOs, BOOL bSet);
private: THORDAQ_STATUS CThordaq::DiscoverBOBtype(CThordaq& TdBrd);
private: THORDAQ_STATUS CThordaq::LEDControlAll_IS31FL_LEDs_on_BOB(CThordaq& TdBrd, UINT8 ControlByte);
public: THORDAQ_STATUS CThordaq::LEDControlIS31FL(CThordaq& TdBrd, int BBoxLEDenums, UCHAR State);
public: void CThordaq::BlinkLEDTask();
public:  atomic<bool> bExitBlinkLEDTask = false;
private: THORDAQ_STATUS  CThordaq::SetBOB_DO(CThordaq& TdBrd, CHAR* ConfigArray, int ConfigArraySize, int NumDOs);
private: THORDAQ_STATUS  CThordaq::GetBOB_DIO(CThordaq& TdBrd, CHAR* ConfigArray, int ConfigArraySize, int NumDOs);

public: THORDAQ_STATUS  CThordaq::SetBOB_DIOConfig(CThordaq& TdBrd, CHAR* ConfigArray, int ConfigArraySize);
public: THORDAQ_STATUS  CThordaq::GetBOB_DIOConfig(CThordaq& TdBrd, CHAR* ConfigArray, int ConfigArraySize);

public: THORDAQ_STATUS CThordaq::APIBreakOutBoxLED(
	CThordaq& TdBrd,
	INT32 LEDenum,
	UCHAR state);

public: THORDAQ_STATUS CThordaq::API_ADCsampleImagizerLUT(
	PS2MM_ADCSAMPLE_LUT pS2MMsampleLUT);

public: THORDAQ_STATUS CThordaq::APIsetDACvoltage(
	UINT32 channel, double Voltage);

public: THORDAQ_STATUS CThordaq::APItdUserIntTask(USER_IRQ_WAIT_STRUCT* usrIrqWaitStruct);
public: THORDAQ_STATUS CThordaq::APItdCancelUserIntTask();

public: THORDAQ_STATUS CThordaq::WriteS2MMdescChain(
	int ADCchannel, int StartingBRAMbank,
	int ChainHead, int ChainTail,
	AXI_DMA_DESCRIPTOR DMA_Desc[][MAX_CHANNEL_COUNT],
	PS2MM_CONFIG pS2MMconfig);

	  class ProgrammableTrigger
	  {
#define PT_SHADOW_REG_STRLEN 32
	  private:
		  char _PT_HW_In1_SEL[PT_SHADOW_REG_STRLEN];
		  char _PT_HW_In2_SEL[PT_SHADOW_REG_STRLEN];
		  char _PT_SW_ArmASSERT[PT_SHADOW_REG_STRLEN];
		  char _PT_HWSW_SEL[PT_SHADOW_REG_STRLEN];
		  char _PT_InputCfgIndx[PT_SHADOW_REG_STRLEN];  // The "truth table" 5 bit lookup table entry "address"
		  char _PT_OutCfgFn[PT_SHADOW_REG_STRLEN];      // The "truth table" 2 bit output function  "data" for the table entry
		  char _PT_CfgWriteSTROBE[PT_SHADOW_REG_STRLEN];   // causes InputCfgIndx+OutCfgFn to be written to 
		  char _PT_DO_WaveformIN_SEL[PT_SHADOW_REG_STRLEN]; // Selects one of the 16 possible DO Waveforms as INput to PT

		  char _PT_ImageAcqTrig_SEL[PT_SHADOW_REG_STRLEN]; // undefined for ImageAcqTrigger, selects ImageAcq or WaveformControl PT
		  char _PT_TruthTableCfg_SEL[PT_SHADOW_REG_STRLEN];  // only for WaveformControl PT - selects TruthTable by channel

	  public:
		  ProgrammableTrigger(CThordaq& TbBrd, signed char chan); // constructor
		  // AppConfig is the enum for pre-defined Programmable Triggers

		  THORDAQ_STATUS ProgrammableTriggerConfig(CThordaq& TbBrd, signed char chan, int AppConfig); 	// chan -1 for ImageAcqTrigger, 0-13 for WaveformControlTrigger
		  THORDAQ_STATUS ProgrammableTriggerTruthTableSetup(CThordaq& TbBrd, signed char chan, int AppConfig);
		  void ProgrammableTriggerArmASSERT(CThordaq& TbBrd, signed char chan, bool Assert); // Software Arm/TRIGGER FPGA register write

	  public:
		  ~ProgrammableTrigger();  // destructor
	  };


	  ProgrammableTrigger* ImageAcqTrigger;

};  // END of CThordaq
//#include "TDAdvancedTriggering.h"  // references CThordaq, so include after CThordaq class def


/**************************************************************************************************
 * @class	XI2Ccontroller
 *
 * @brief	This class implements new Xilinx "AXI IIC Bus Interface v2.0, LogiCORE IP Product Guide,
 *          PG090, Oct. 5, 2016, through AXI Bridge interface by BRadtke, Dec. 2020
 *
 * @author	DZimmerman
 * @date	12/22/2020
 * This class handles the "AIX Bridge" FPGA interface to the I2C Master Register Space (PG090, pg. 11)
 * There are 3 registers:
 * I2C_AXIBridge  - The upper 32 bits is Address (for both read/write), lower 32 bits DATA (for write)
 * I2C_AXIStatus  - The DATA read back from bridge
 * I2C_AXIControl - bits 0,1 tell FPGA if we are reading or writing - asserted on 0->1 value change
 * Xilinx I2C AXI IIC Core Register Map:
01Ch GIE Global Interrupt Enable Register
020h ISR Interrupt Status Register
028h IER Interrupt Enable Register
040h SOFTR Soft Reset Register
100h CR Control Register
104h SR Status Register
108h TX_FIFO Transmit FIFO Register
10Ch RX_FIFO Receive FIFO Register
110h ADR Slave Address Register
114h TX_FIFO_OCY Transmit FIFO Occupancy Register
118h RX_FIFO_OCY Receive FIFO Occupancy Register
11Ch TEN_ADR Slave Ten Bit Address Register
120h RX_FIFO_PIRQ Receive FIFO Programmable Depth Interrupt Register
124h GPO General Purpose Output Register
128h TSUSTA Timing Parameter Register
12Ch TSUSTO Timing Parameter Register
130h THDSTA Timing Parameter Register
134h TSUDAT Timing Parameter Register
138h TBUF Timing Parameter Register
13Ch THIGH Timing Parameter Register
140h TLOW Timing Parameter Register
144h THDDAT Timing Parameter Register
*
**************************************************************************************************/
class XI2Ccontroller
{
public:
	enum I2Cdir
	{
		WRITE = 0,
		READ = 1
	};
	enum I2CSR
	{
		ABGC = 0x1, // Addressed By General Call (broadcast)
		AAS = 0x2, // Addressed as Slave
		BB = 0x4,   // I2C Bus Busy
		SRW = 0x8,  // Slave Read/Write
		TX_FIFO_Full = 0x10, // 
		RX_FIFO_Full = 0x20, // 
		RX_FIFO_Empty = 0x40,
		TX_FIFO_Empty = 0x80
	};
	enum IER
	{
		ArbLost = 0,
		TxErr,
		TxFIFOEmpty,
		RxFIFOFull,
		I2CBusNotBusy,
		AddressedAsSlave,
		NotAddressedAsSlave,
		TxFIFOhalfEmpty
	};	enum I2CcoreRegs
	{
		Global_Interrupt_Enable_Register = 0x1C,
		Interrupt_Status_Register = 0x20,
		Interrupt_Enable_Register = 0x28,
		Soft_Reset_Register = 0x40,
		Control_Register = 0x100,
		Status_Register = 0x104,
		TX_FIFO_Reg = 0x108,
		RX_FIFO_Reg = 0x10C,
		Slave_Address_Register = 0x110,
		TX_FIFO_Occupancy_Register = 0x114,
		RX_FIFO_Occupancy_Register = 0x118,
		Ten_Bit_Address_Register = 0x11C,
		RX_FIFO_PIRQ_Register = 0x120,
		GPO_Register = 0x124,
		TSUSTA_Register = 0x128,
		TSUSTO_Register = 0x12C,
		THDSTA_Register = 0x130,
		TSUDAT_Register = 0x134,
		TBUF_Register = 0x138,
		THIGH_Register = 0x13C,
		TLOW_Register = 0x140,
		THDDAT_Register = 0x144
	};
	enum CR_Action
	{
		GC_EN = 0x40,
		RSTA = 0x20, // repeated start
		TXAK = 0x10, // 0 ACK,  1 NACK
		TX = 0x08,
		MSMS = 0x04, // I2C (S)tart bit assserted, transmission starts
		TX_FIFO_Rst = 0x02, // Transmit FIFO reset
		EN = 0x01        // enable the I2C core (0 holds in reset)
	};
private:
	UINT _gBoardPCIeIndex = 0; // copy from ThorDAQ class
	HANDLE _hCopyOfXI2Cmutex;
	UINT32 _masterMUX = -1; // last I2C MUX setup - saves time of resetting MUX
	UINT32 _slaveMUX = -1;  // when not necessary
	int _I2C_AXIBridge = -1, _nullBitField = -1; // (BitField not supported)
	int _I2C_AXIStatus = -1;
	int _I2C_AXIControl = -1;
	const UINT64 _TI9548A_RESET = 0x4; // bit 2

public:
	XI2Ccontroller(CThordaq& TdBrd, bool Speed400kHz)    // CONSTRUCTOR: pass reference to ThorDAQ board hardware
	{
		// CONSTRUCTOR: pass reference to ThorDAQ board hardware
		// Grab the XI2C Hardware MUTEX!
		if (TdBrd._hXI2Cmutex != NULL)
		{
			_hCopyOfXI2Cmutex = TdBrd._hXI2Cmutex;
			DWORD dwWaitResult = WaitForSingleObject(
				_hCopyOfXI2Cmutex,		// handle to mutex (from CThordaq board class)
				INFINITE);				// no time-out interval		
		}
		else // debug - should never get here
		{
			DebugBreak();
		}
		// to gain performance, lookup Shadow Register, then reference by index
		// register/bit field indexes of the I2C/AXI bridge registers
		bool bStatus = TdBrd.SearchFPGAregFieldIndices("I2C_AXIBridge", (int)strlen("I2C_AXIBridge"), &_I2C_AXIBridge, &_nullBitField);
		bStatus = TdBrd.SearchFPGAregFieldIndices("I2C_AXIStatus", (int)strlen("I2C_AXIStatus"), &_I2C_AXIStatus, &_nullBitField);
		bStatus = TdBrd.SearchFPGAregFieldIndices("I2C_AXIControl", (int)strlen("I2C_AXIControl"), &_I2C_AXIControl, &_nullBitField);

		// Our expectation is to config/enable the I2C core once at startup

		ResetTI9548MUX(TdBrd); // reset the 9548 master I2C MUX
		WriteI2CAIXBridge(TdBrd, Control_Register, 0); // Reset the core
		WriteI2CAIXBridge(TdBrd, Control_Register, CR_Action::EN); // enable the core
		WriteI2CAIXBridge(TdBrd, Soft_Reset_Register, 0xA); // soft reset
		// make sure TX FIFO is reset; previous I2C errors can leave remnants
		WriteI2CAIXBridge(TdBrd, XI2Ccontroller::Control_Register, XI2Ccontroller::TX_FIFO_Rst | XI2Ccontroller::EN);
		WriteI2CAIXBridge(TdBrd, XI2Ccontroller::Control_Register, XI2Ccontroller::EN); // ENable (other bits cleared)

		// boost TBUF time for Atmel MCU slave
		WriteI2CAIXBridge(TdBrd, XI2Ccontroller::TBUF_Register, 50000);   // e.g. 50,000 ticks * 5ns/tick = 250 usec

		// 400 kHz settings (all I2C register unit 5ns "ticks)
		if (Speed400kHz) {

			UINT64 TSUSTA = 200;
			WriteI2CAIXBridge(TdBrd, XI2Ccontroller::TSUSTA_Register, TSUSTA);

			UINT64 THDSTA = 200;  // e.g. 200(ticks) * 5(ns/tick) = 1.0 usecs
			WriteI2CAIXBridge(TdBrd, XI2Ccontroller::THDSTA_Register, THDSTA);

			UINT64 TSUDAT = 58; // (58 * 5ns/tick = 290 ns) Xilinx includes "Tf", fall time
			WriteI2CAIXBridge(TdBrd, XI2Ccontroller::TSUDAT_Register, TSUDAT);
			UINT64 THDDAT = 120; // (60 * 5ns/tick = 300)
			WriteI2CAIXBridge(TdBrd, XI2Ccontroller::THDDAT_Register, THDDAT);

			UINT64 TBUF = 280; // 
			WriteI2CAIXBridge(TdBrd, XI2Ccontroller::TBUF_Register, TBUF);

			UINT64 TLOW = 850; // see Xlinix calculation, RegValue = ((200,000,000 / 2 / Fscl) - 7 - SCL_InertialDelay (243 for 400 kHz)
			WriteI2CAIXBridge(TdBrd, XI2Ccontroller::TLOW_Register, TLOW);
			UINT64 THIGH = 425; // 
			WriteI2CAIXBridge(TdBrd, XI2Ccontroller::THIGH_Register, THIGH);

		}

	}
	// Functions to read and write I2C Core Registers through ThorDAQ's FPGA bridge...
	// I2C registers are 32 bits (address and data) - FPGA bridge registers are 64 bits
	void WriteI2CAIXBridge(CThordaq& TdBrd, UINT64 CoreRegAddress, UINT64 Data)  // WriteI2C Register through AIX bridge 
	{
		// first setup up the Address/Data in Bridge Register
		UINT64 BridgeAddrDataValue = (CoreRegAddress << (UINT64)32);
		BridgeAddrDataValue |= Data;

		TdBrd.FPGAregisterWrite(_I2C_AXIBridge, BridgeAddrDataValue);
		TdBrd.FPGAregisterWrite(_I2C_AXIControl, _TI9548A_RESET | 0x1);  // WRITE command (OR in the 9548 RESET de-assert)
		TdBrd.FPGAregisterWrite(_I2C_AXIControl, _TI9548A_RESET);  // (must Toggle)
	}
	void ReadI2CAIXBridge(CThordaq& TdBrd, UINT64 CoreRegAddress, UINT64* Data)  // WriteI2C Register through AIX bridge
	{
		// first setup up the Address/Data in Bridge Register
		UINT64 BridgeAddr = (CoreRegAddress << (UINT64)32); // lower 32-bits "DATA" is don't care
		TdBrd.FPGAregisterWrite(_I2C_AXIBridge, BridgeAddr);
		TdBrd.FPGAregisterWrite(_I2C_AXIControl, _TI9548A_RESET | 0x2);  // READ command (OR in the 9548 RESET de-assert)
		TdBrd.FPGAregisterWrite(_I2C_AXIControl, _TI9548A_RESET);  // (must Toggle)
		TdBrd.FPGAregisterRead(_I2C_AXIStatus, _nullBitField, Data);
	}
	// see "TCA9548A Low-Voltage 8-Channel I2C Switch with Reset", pg. 1
	// On pg. 8, RESET pulse duration, 6 to 500 ns, dependent on I2C protocol state
	// Our expected use: once every start of I2C master message
	void ResetTI9548MUX(CThordaq& TdBrd)
	{
		TdBrd.FPGAregisterWrite(_I2C_AXIControl, 0); // reset is asserted low
		TdBrd.FPGAregisterWrite(_I2C_AXIControl, _TI9548A_RESET); // bit 2 - set to release from RESET
	}

	~XI2Ccontroller()
	{
		// Release the MUTEX for next use...
		if (_hCopyOfXI2Cmutex != NULL)
			ReleaseMutex(_hCopyOfXI2Cmutex);		// handle to mutex (from CThordaq board class)
	}
};



//TODO: move this and other classes to separate files and just leave the thordaq class in this file
class I2C_BBox_ChipPCA9554
{
public:
	// Read/Write Thordaq I2C network device
#define I2CsetupStringLEN 16  // e.g. 0x
	char _Name[I2CsetupStringLEN]; // e.g. "DAC_EEPROM"
	UINT32 _MasterI2Cchannel;
	INT32 _SlaveI2Cchannel;     // -1 means no slave I2C
	UINT32 _DevAddress;
	UINT32 _I2CBusHz;           // expect 400 Hz
public:
	I2C_BBox_ChipPCA9554(const char* Name, UINT32 MasterChan, INT32 SlaveChan, UINT32 DevAddr)
	{
		strcpy_s(_Name, Name);
		_MasterI2Cchannel = MasterChan;
		_SlaveI2Cchannel = SlaveChan;
		_DevAddress = DevAddr;
		_I2CBusHz = 400;
	}
	UINT32 DeviceAddress()
	{
		return _DevAddress;
	}
	BOOL HasSlaveMUX()
	{
		if (_SlaveI2Cchannel > 0)
			return true;
		return false;
	}
	// Process to execute a read/write to PCA9554 8-bit I2C device
	// 1. Reset Xilix I2C core
	// 2. Set MUX channel Master/Slave  
	// 3. Shadow Register of current contents
	// 4. Write new setting

	// TI 9548 Master and 9546 Slave I2C MUX setup function
	// I2C multiplexed channel devices must be setup in series process -
	// ALWAYS set up the master, then if needed the slave
#define I2CbufLen 16
	THORDAQ_STATUS SetupI2CMux(CThordaq& TdBrd, BOOL Master)
	{
		THORDAQ_STATUS status;
		// hardware Master MUX deviceAddress is 0x71, slave is 0x70		
		bool bI2C_Read = false;  // WRITE for setting up PCA9554 8-bit chip
		int PageSize = 1;
		UINT8 OpCodes[3];
		UINT8 DataBuf[1];
		OpCodes[0] = 0x3; // Configuration Register Command
		OpCodes[1] = 0x0; // Change from input '1' to output '0', all pins
		UINT32 OpCodeLen = 2;
		UINT32 DataLen = 0;   // send all as "OpCodes"

		status = TdBrd.APIXI2CReadWrite(TdBrd, bI2C_Read, 0x71, _MasterI2Cchannel, 0x70, _SlaveI2Cchannel, _DevAddress, _I2CBusHz, PageSize, OpCodes, &OpCodeLen, DataBuf, &DataLen);

		return status;
	}

	THORDAQ_STATUS ExecuteI2CreadWrite(CThordaq& TdBrd, I2C_BBox_ChipPCA9554& I2Cdev, BOOL Read, UINT32 SlaveCommandByte, PUINT8 DATAbuffer, PUINT32 byteLen)
	{
		THORDAQ_STATUS status = STATUS_PARAMETER_SETTINGS_ERROR;
		PUINT8 bufferPosition = DATAbuffer;
		UINT32 BytePosition = 0;
		UINT64 DataAndStatus = 0;
		INT32 timeoutMS = 50;
		UINT8 OpCodes[3];
		OpCodes[0] = SlaveCommandByte;
		OpCodes[1] = DATAbuffer[0]; // in case it's a WRITE
		UINT32 OpCodeLen = 2; // format is always "CommandByte" "DataByte"

		if (Read == TRUE)
		{
			// DATAbuffer receives DATA
			*byteLen = 1; // number of READ bytes expected
		}
		else // WRITE
		{
			*byteLen = 0; // no "DATA", only "OpCodes"
		}
		status = TdBrd.APIXI2CReadWrite(TdBrd, Read, 0x71, _MasterI2Cchannel, 0x70, _SlaveI2Cchannel, _DevAddress, _I2CBusHz, 1, OpCodes, &OpCodeLen, DATAbuffer, byteLen);
		return status;
	}

};  // -END- class I2CDevice


////////////////////////////////////////////////////////////////////////////////////////////
//
// 3U IOPanel BreakBox Slow Analog Inputs - I2C control of MAX127 chip for 8 channels
// See "Maxim Integrated" MAX127/MAX128 Datasheet, 19-4773; Rev 1; 12/12, esp. pg. 10
// 
// There must be 2 bytes for I2C bus write command:  
//  ~~~~~~~~~~~~~~~~~~~~~    ~~~~~~~~~~~~~~~   
// |  i2c SlaveDevAddr   |  | Control
//  ~~~~~~~~~~~~~~~~~~~~~    ~~~~~~~~~~~~~~~  
//  Bit
//  7  6  5  4  3  2  1  0
//  1* c  c  c  r  b  0  x
// ccc is the channel, 0 - 7
// r is range (typically 1 for 10 volts, 0 for 5 volts
// b is bipolar (typically 1), or 0 for unipolar (0-5 or 0-10 volts depending on "r")
// *NOTE-- failure to set this bit to one hangs I2C bus requiring power recycle
// We run at I2C bus freq. of 400 kHz; the ADC sampling/conversion completes
// faster than the I2C can turn around to read result
// DATA:
// Data is returned in two bytes, the MSB, followed by LSB -- lower nibble
// of LSB is ignored.
class BreakoutBoxMAX127AI
{
private: THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;
	   const UINT32 _MasterMuxAddr = 0x71;
	   const UINT32 _SlaveMuxAddr = 0x70;
	   const UINT32 _MasterI2Cchannel = 0x8;
	   const UINT32 _SlaveI2Cchannel = 0x8;
	   const UINT32 _DevAddress = 0x28; // MAX127 slave addr. (hardware based)
	   const UINT32 _I2CBusHz = 400; // Hz
	   UINT32 _OpCodeLen = 0;
	   bool _bI2Cread = 0;

public: BreakoutBoxMAX127AI() // def. constructor
{
}
public: THORDAQ_STATUS Read(CThordaq& TdBrd, INT32 BNCindx, double* VoltageADCcounts)
{
	UINT32 ADCcounts;
	THORDAQ_STATUS status = Read(TdBrd, BNCindx, &ADCcounts);
	if (status != STATUS_SUCCESSFUL)
		return status;
	INT32 Max127ChanIndex = BNCindx - 6;
	if (Max127ChanIndex < 0 || Max127ChanIndex > 7)
		return (THORDAQ_STATUS)STATUS_INVALID_PARAMETER;
	// convert ADC counts to Voltage
	// "Maxim Integrated", MAX127 / MAX128 Data Sheet, 19-4773 Rev 1 12/12, pg. 10
	if (TdBrd.BOB_AISettings[BNCindx].Polarity == 'B')
	{
		double FullScaleVolts = 2 * TdBrd.BOB_AISettings[BNCindx].VoltageRange; // 20.0
		double VoltsPerBit = FullScaleVolts / 4096;
		*VoltageADCcounts = (ADCcounts * VoltsPerBit);
		if (ADCcounts >= 0x800) // bit 11 is SIGN bit  (2048 - 4095 are negative voltages)
		{
			*VoltageADCcounts -= FullScaleVolts;
		}
	}
	else // unipolar
	{
		double VoltsPerBit = TdBrd.BOB_AISettings[BNCindx].VoltageRange / 4096;
		*VoltageADCcounts = (ADCcounts * VoltsPerBit);
	}
	return STATUS_SUCCESSFUL;
}
	  // read the MAX127 ADC channel - always in 12-bits ADC Counts
public: THORDAQ_STATUS Read(CThordaq& TdBrd, INT32 BNCindex, UINT32* ADCcounts)
{
	UINT8 OpCodes[8];		// standard AIX I2C buffer sizes...
	UINT8 DATAbuffer[64];
	UINT32 byteLen[8];      // (always use 64-bit boudaries)
	INT32 MAX127ChipChanIndex = BNCindex - 6;
	if (MAX127ChipChanIndex < 0 || MAX127ChipChanIndex > 7)
		return (THORDAQ_STATUS)STATUS_INVALID_PARAMETER;

	int range = (TdBrd.BOB_AISettings[BNCindex].VoltageRange == 10.0) ? 1 : 0;
	int biPolar = (TdBrd.BOB_AISettings[BNCindex].Polarity == 'B') ? 1 : 0;
	_OpCodeLen = 0; // write single byte, then STOP bit
	byteLen[0] = 1; // SINGLE command write byte count
	DATAbuffer[0] = 0x80 | (MAX127ChipChanIndex << 4);
	DATAbuffer[0] |= range << 3 | (biPolar << 2);

	_bI2Cread = FALSE; // write "OpCode" command byte first
	status = TdBrd.APIXI2CReadWrite(TdBrd, _bI2Cread, _MasterMuxAddr, _MasterI2Cchannel, _SlaveMuxAddr, _SlaveI2Cchannel, _DevAddress, _I2CBusHz, 2, OpCodes, &_OpCodeLen, DATAbuffer, byteLen);
	if (status != STATUS_SUCCESSFUL)
		return status;  // BBox disconnected or (Legacy BBox?)
	// READ back the channel ADC counts (quickly - if you delay, i.e. debug breakpoint, you may get single byte & timeout error)
	_OpCodeLen = 0; // no OpCode needed for read
	byteLen[0] = 2; // expect 2 bytes for the 12-bit ADC count
	_bI2Cread = TRUE; // immediately read ADC result
	status = TdBrd.APIXI2CReadWrite(TdBrd, _bI2Cread, _MasterMuxAddr, _MasterI2Cchannel, _SlaveMuxAddr, _SlaveI2Cchannel, _DevAddress, _I2CBusHz, 2, OpCodes, &_OpCodeLen, DATAbuffer, byteLen);
	if (status != STATUS_SUCCESSFUL)
		return status;  // BBox disconnected or (Legacy BBox?)

	if (DATAbuffer[1] == 0xFF && DATAbuffer[0] == 0xFF) // invalid value! e.g. when MAX127 READ is not preceeded by WRITE control byte
	{
		return STATUS_PARAMETER_SETTINGS_ERROR;
	}
	*ADCcounts = (DATAbuffer[1] >> 4); // see MAX127 Datasheet Fig. 10 pg. 13 (format)
	*ADCcounts |= DATAbuffer[0] << 4;
	return status;
}
};



////////////////////////////////////////////////////////////////////////////////////////////
//
// 3U IOPanel BreakBox LEDs - I2C control of IS31FL3236 chip for 8 (or 6) LEDs
//
// There must be at least 3 bytes for I2C bus write command:  
//  ~~~~~~~~~~~~~~~~~~~~~    ~~~~~~~~~~~~~~~   ~~~~~~~~~~~~
// |  i2c SlaveDevAddr   |  | Command/Index | | WriteData  |
//  ~~~~~~~~~~~~~~~~~~~~~    ~~~~~~~~~~~~~~~   ~~~~~~~~~~~~
// NOTE:  Failure to terminate I2C command to this chip with I2C stoP bit Hangs I2C
// bus requiring a power cycle!
// For IS31FL3236 LED controller, the Constructor must ENABLE device with
// WRITE: SlaveAddr--0x0--0x1  (write Reg 0x0, value 0x1)
// The Destructor should reset device with
// WRITE: SlaveAddr--0x4F--0x0
// 
// Limit the current to the device to NOT exceed the 350 mA HDMI cable limit
// WRITE: 
// Parameterized I2C device command (i.e., not assuming sequential addresses from 0)
// The PCA 9554 defaults to "INPUT" and must be configured for OUTPUT at startup
// e.g. to initialize PCA9554 LED controller, 0x353, 0x03 (Command-Config), 0x00 (config as OUTPUT)
//      to set LED pattern,                   0x353, 0x01 (Output port Reg), 0xAA (turn on bottom LEDs)
// See "TI PCA 9554 Remote 8-Bit I2C and SMBus I/O Epander With Int. Output and Config Registers", 
// Rev. June 2014, pg 18.
// The LEDs wiring results in reverse logic polarity when writing to 9554 output register (O7-O0)
// 0 -- LED on
// 1 -- LED off
// ("Polarity inversion" in 9554 is only for inputs)
// ALSO, while the label says (for instance) DIO1 on top, and DIO2 on bottom, the data register
// bits are inverted (the least significant bit controls bottom LED, not the top per label)
// LED
// * DIO1 (bit 1)   DIO3 (bit 3) ...
// * DIO2 (bit 0)   DIO4 (bit 2) 
/// //////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////
//
// BreakBox LEDs - I2C control of PCA 9554 chip for 8 (or 6) LEDs
//
// There must be at least 3 bytes for I2C bus write command:  
//  ~~~~~~~~~~~~~~~~~~~~~    ~~~~~~~~~~~~~~~   ~~~~~~~~~~~~
// |  i2c SlaveDevAddr   |  | Command/Index | | WriteData  |
//  ~~~~~~~~~~~~~~~~~~~~~    ~~~~~~~~~~~~~~~   ~~~~~~~~~~~~
// For EEPROM, Command/Index is always the byte Index for read/write location
// For PCA9554 LED controller, Command code is either "configure" device, or select OutputRegister
// Parameterized I2C device command (i.e., not assuming sequential addresses from 0)
// The PCA 9554 defaults to "INPUT" and must be configured for OUTPUT at startup
// e.g. to initialize PCA9554 LED controller, 0x353, 0x03 (Command-Config), 0x00 (config as OUTPUT)
//      to set LED pattern,                   0x353, 0x01 (Output port Reg), 0xAA (turn on bottom LEDs)
// See "TI PCA 9554 Remote 8-Bit I2C and SMBus I/O Epander With Int. Output and Config Registers", 
// Rev. June 2014, pg 18.
// The LEDs wiring results in reverse logic polarity when writing to 9554 output register (O7-O0)
// 0 -- LED on
// 1 -- LED off
// ("Polarity inversion" in 9554 is only for inputs)
// ALSO, while the label says (for instance) DIO1 on top, and DIO2 on bottom, the data register
// bits are inverted (the least significant bit controls bottom LED, not the top per label)
// LED
// * DIO1 (bit 1)   DIO3 (bit 3) ...
// * DIO2 (bit 0)   DIO4 (bit 2) 
/// //////////////////////////////////////////////////////
/// </remarks>

class BreakoutBoxLEDs
{
private:
	BOOL _bIsConfigured;     // PCA9554 device requires CONFIG to make GPIO into Outputs
	BYTE _ucShadowRegister;  // last register state (avoids delay of I2C read)
	I2C_BBox_ChipPCA9554* _I2CdevPCA9554;

	bool Configure(CThordaq& TdBrd, I2C_BBox_ChipPCA9554& I2Cdev) // on power-up, GPIOs are configure for INPUT - switch to OUTPUT
	{
		THORDAQ_STATUS status = (THORDAQ_STATUS)0;
		_ucShadowRegister = 0xFF;  // for now, ALL LEDs OFF at startup (we can read
		// hardware Master MUX deviceAddress is 0x71, slave is 0x70		
		bool bI2C_Read = false;  // WRITE for setting up PCA9554 8-bit chip
		int PageSize = 1;
		UINT8 OpCodes[3];
		UINT8 DataBuf[1];
		OpCodes[0] = 0x3; // Configuration Register Command
		OpCodes[1] = 0x0; // Change from input '1' to output '0', all pins
		UINT32 OpCodeLen = 2;
		UINT32 DataLen = 0;   // send all as "OpCodes"
		UINT32 I2CBusHz = 400; // default speed 
		status = TdBrd.APIXI2CReadWrite(TdBrd, bI2C_Read, 0x71, I2Cdev._MasterI2Cchannel, 0x70, I2Cdev._SlaveI2Cchannel, I2Cdev._DevAddress, I2CBusHz, PageSize, OpCodes, &OpCodeLen, DataBuf, &DataLen);
		if (status == STATUS_SUCCESSFUL)
		{
			_bIsConfigured = true;
			return true;
		}
		return false;
	}

	BOOL IsConfigured()
	{
		return _bIsConfigured;
	}
public:  // constructor
	BreakoutBoxLEDs()
	{
		_bIsConfigured = false;
		_ucShadowRegister = 0xFF;
		_I2CdevPCA9554 = nullptr;
	}
	void SetI2CdevPtr(I2C_BBox_ChipPCA9554* I2Cdev)
	{
		_I2CdevPCA9554 = I2Cdev;
	}
	I2C_BBox_ChipPCA9554* GetI2CdevPtr()
	{
		return _I2CdevPCA9554;
	}
	THORDAQ_STATUS Control(CThordaq& TdBrd, BYTE LEDidentifierMask, bool On)
	{
		if (!IsConfigured())
		{
			// sanity check that I2C device has been set...
			if (_I2CdevPCA9554 == nullptr)
			{
				return STATUS_INCOMPLETE; // init failure
			}
			if (Configure(TdBrd, *_I2CdevPCA9554) != TRUE) // I2C failure when BBox disconnected
				return STATUS_I2C_INVALIDDEV;
		}
		THORDAQ_STATUS status;
		UINT32 DATAbufLen = 1;
		UINT8 I2Ccmd = 0x1; // PCA9554 Output Port

		// (use shadow register to preserve state of non-altered LEDs)
//		if (LEDidentifierMask == 0xFF)
		if (On)
		{
			if (LEDidentifierMask == 0xFF) // ALL LEDs for this BBox?
				_ucShadowRegister = 0; // ALL on
			else
				_ucShadowRegister &= ~LEDidentifierMask; // 0 is "ON"
		}
		else
		{
			if (LEDidentifierMask == 0xFF) // ALL LEDs for this BBox?
				_ucShadowRegister = 0xFF; // ALL off
			else
				_ucShadowRegister |= LEDidentifierMask; // 1 is "OFF"
		}
		status = _I2CdevPCA9554->ExecuteI2CreadWrite(TdBrd, *_I2CdevPCA9554, FALSE, (UINT32)I2Ccmd, &_ucShadowRegister, &DATAbufLen);
		return status;
	}
};