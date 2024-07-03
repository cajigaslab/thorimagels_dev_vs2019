/*++

Copyright (c) Thorlab.Inc.  All rights reserved.

Module Name: ThordaqDFLIM.c


Abstract:

    Defines the API functions for the thordaq driver.

Environment:

    kernel mode only.

Style:
	Google C++ coding style.
Note:
	"Wisely and slow; they stumble that run fast". - (William Shakespeare  Romeo and Juliet Act II, Scene III).
--*/

#include "stdafx.h"
#pragma warning(disable:4201) //I just wanna use freedom nonstandard "nameless struct/union"
#include "dFLIM_4002.h"
#include "XilinxI2C.h"

//#include "thordaqguid.h"

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

/**********************************************************************************************//**
 * @fn	ThordaqDFLIM::ThordaqDFLIM ( UINT boardNum )
 *
 * @brief	Default constructor.
 *
 * @author	Cge
 * @date	3/17/2017
 * DZimmerman 24-Aug-23, mods for common thordaq API
 * 
 * @param	boardNum	Index of thordaq board.
 **************************************************************************************************/
CdFLIM_4002::CdFLIM_4002()
{
	gDeviceInfo = NULL;
	gDeviceInterfaceDetailData = NULL;
	gDmaInfo.PacketRecvEngineCount = 0;
	gDmaInfo.PacketSendEngineCount = 0;
	gDmaInfo.AddressablePacketMode = false; // was "IsAddressablePacket"
	gPtrAcqCtrl = new DATA_ACQ_CTRL_STRUCT(); //Zero initialize a dynamically allocated thordaq acquisition struct.
	memset(gPtrAcqCtrl, 0, sizeof(DATA_ACQ_CTRL_STRUCT));
	return;
}
CdFLIM_4002::CdFLIM_4002 ( UINT boardNum, HANDLE handle )
{
	gBoardIndex = boardNum;
    gDeviceInfo = NULL;
    gHdlDevice = handle; // from dllmain.cpp discovery/opening of kernel device
    gDeviceInterfaceDetailData = NULL;
	gDmaInfo.PacketRecvEngineCount = 0;
    gDmaInfo.PacketSendEngineCount = 0;
    gDmaInfo.AddressablePacketMode = false; // was "IsAddressablePacket"
	gPtrAcqCtrl = new DATA_ACQ_CTRL_STRUCT(); //Zero initialize a dynamically allocated thordaq acquisition struct.
	memset(gPtrAcqCtrl, 0, sizeof(DATA_ACQ_CTRL_STRUCT));
	_NWL_Common_DMA_Register = 0;

	// DZ Xilinx I2C -- create MUTEX to protect I2C master hardware
	WCHAR brdIndx[2];
	_itow_s(boardNum, brdIndx, 16); // radix Base16 (0 - F)
	_wcMname[7] = brdIndx[0];
	_I2CmutexName = _wcMname;
	// Since this is single hardware object, prevent simulatenous access from
	// (potential) multiple threads 
	// 
	_hXI2Cmutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		_I2CmutexName);    // name "XI2CBrdN", where N is 0-F
	/////////////////////

	// DZ SDK -- define the hardware registers and bit fields for this board
	// If both WriteOnly and ReadOnly are false, register is Read/Write
	char RegFldVerBuf[48] = "dFLIM V1.0.0.0 \0";
	char* RegMapBldVer = RegFldVerBuf; // __DATE__;
	const char* RegFldDateBuf = __DATE__;
	const char* RegFldTimeBuf = __TIME__;
	strncat_s(RegMapBldVer, 48, RegFldDateBuf, _TRUNCATE);
	strncat_s(RegMapBldVer, 48, " ", _TRUNCATE);
	strncat_s(RegMapBldVer, 48, RegFldTimeBuf, _TRUNCATE);
	_FPGAregister[TD_FPGA_DLL_VER] = new FPGA_HardwareReg(RegMapBldVer, 1, BAR1, 0xFFFFFFFF, 0, WRITEONLY);

	// START SHADOW REGISTERS //
	/////////// BAR 0 (NorthWest Logic core) /////////////////////
	_FPGAregister[NWL_DDR3_DMAcore] = new FPGA_HardwareReg("NWL_DDR3_DMAcore", 4, BAR0, 0x4000, 0, READWRITE);
	_FPGAregister[NWL_DDR3_DMAcore]->BitField[NWL_GlobalDMAIntEnable] = new RegisterBitField("NWL_GlobalDMAIntEnable", 0, 0);
	_FPGAregister[NWL_DDR3_DMAcore]->BitField[NWL_DMAIntActive] = new RegisterBitField("NWL_DMAIntActive", 1, 1);
	_FPGAregister[NWL_DDR3_DMAcore]->BitField[NWL_DMAIntPending] = new RegisterBitField("NWL_DMAIntPending", 2, 2);
	_FPGAregister[NWL_DDR3_DMAcore]->BitField[NWL_DMAIntMode] = new RegisterBitField("NWL_DMAIntMode", 3, 3);
	_FPGAregister[NWL_DDR3_DMAcore]->BitField[NWL_UserIntEnable] = new RegisterBitField("NWL_UserIntEnable", 4, 4);
	_FPGAregister[NWL_DDR3_DMAcore]->BitField[NWL_UserIntActive] = new RegisterBitField("NWL_UserIntActive", 5, 5);
	_FPGAregister[NWL_DDR3_DMAcore]->BitField[NWL_MSIXMode] = new RegisterBitField("NWL_MSIXMode", 6, 6);

	/////////// BAR 1 /////////////////////
	// Read/Write access to ThorDAQ's BAR1 S2MM DMA descriptors, but the Logicore AXI DMA engine is abstracted and unreadable
	_FPGAregister[S2MM_DMA_Desc_Chan0] = new FPGA_HardwareReg("S2MM_DMA_Desc_Chan0", 4, BAR1, 0x0, 0, READWRITE);
	_FPGAregister[S2MM_DMA_Desc_Chan1] = new FPGA_HardwareReg("S2MM_DMA_Desc_Chan1", 4, BAR1, 0x10000, 0, READWRITE);
	_FPGAregister[S2MM_DMA_Desc_Chan2] = new FPGA_HardwareReg("S2MM_DMA_Desc_Chan2", 4, BAR1, 0x20000, 0, READWRITE);
	_FPGAregister[S2MM_DMA_Desc_Chan3] = new FPGA_HardwareReg("S2MM_DMA_Desc_Chan3", 4, BAR1, 0x30000, 0, READWRITE);

	/////////  BAR 2 ////////////////////
	_FPGAregister[S2MMDMA_ControlReg1] = new FPGA_HardwareReg("S2MMDMA_ControlReg1", 1, BAR2, 0x0, 0, WRITEONLY);
	_FPGAregister[S2MMDMA_ControlReg1]->BitField[S2MM_CONFIG_Chan0] = new RegisterBitField("S2MM_CONFIG_Chan0", 0, 0);
	_FPGAregister[S2MMDMA_ControlReg1]->BitField[S2MM_RUN_STOP_Chan0] = new RegisterBitField("S2MM_RUN_STOP_Chan0", 1, 1);
	_FPGAregister[S2MMDMA_ControlReg1]->BitField[S2MM_SG_CYCLIC_BD_Chan0] = new RegisterBitField("S2MM_SG_CYCLIC_BD_Chan0", 4, 4);
	_FPGAregister[S2MMDMA_ControlReg1]->BitField[S2MM_IRQ_REARM_Chan0] = new RegisterBitField("S2MM_IRQ_REARM_Chan0", 6, 6);
	_FPGAregister[S2MMDMA_ControlReg1]->BitField[S2MM_SB_BRAM_BANK_SEL_Chan0] = new RegisterBitField("S2MM_SB_BRAM_BANK_SEL_Chan0", 7, 7);
	_FPGAregister[S2MMDMA_StatusReg1] = new FPGA_HardwareReg("S2MMDMA_StatusReg1", 1, BAR2, 0x0, 0, READONLY);
	_FPGAregister[S2MMDMA_StatusReg1]->BitField[S2MM_DMAcontrollerRev_Chan0] = new RegisterBitField("S2MM_DMAcontrollerRev_Chan0", 0, 3);
	_FPGAregister[S2MMDMA_StatusReg1]->BitField[S2MM_DESC_CHAIN_IRQ_Chan0] = new RegisterBitField("S2MM_DESC_CHAIN_IRQ_Chan0", 4, 4);
	_FPGAregister[S2MMDMA_StatusReg1]->BitField[S2MM_CHANNUMindex_Chan0] = new RegisterBitField("S2MM_CHANNUMindex_Chan0", 6, 7);
	_FPGAregister[S2MMDMA_ChainHead_Chan0] = new FPGA_HardwareReg("S2MMDMA_ChainHead_Chan0", 2, BAR2, 0x2, 0, WRITEONLY);
	_FPGAregister[S2MMDMA_ChainTail_Chan0] = new FPGA_HardwareReg("S2MMDMA_ChainTail_Chan0", 2, BAR2, 0x4, 0, WRITEONLY);
	_FPGAregister[S2MMDMA_IRQthreshld_Chan0] = new FPGA_HardwareReg("S2MMDMA_IRQthreshld_Chan0", 2, BAR2, 0x6, 0, WRITEONLY);


	_FPGAregister[S2MMDMA_ControlReg2] = new FPGA_HardwareReg("S2MMDMA_ControlReg2", 1, BAR2, 0x40, 0, WRITEONLY);
	_FPGAregister[S2MMDMA_ControlReg2]->BitField[S2MM_CONFIG_Chan1] = new RegisterBitField("S2MM_CONFIG_Chan1", 0, 0);
	_FPGAregister[S2MMDMA_ControlReg2]->BitField[S2MM_RUN_STOP_Chan1] = new RegisterBitField("S2MM_RUN_STOP_Chan1", 1, 1);
	_FPGAregister[S2MMDMA_ControlReg2]->BitField[S2MM_SG_CYCLIC_BD_Chan1] = new RegisterBitField("S2MM_SG_CYCLIC_BD_Chan1", 4, 4);
	_FPGAregister[S2MMDMA_ControlReg2]->BitField[S2MM_IRQ_REARM_Chan1] = new RegisterBitField("S2MM_IRQ_REARM_Chan1", 6, 6);
	_FPGAregister[S2MMDMA_ControlReg2]->BitField[S2MM_SB_BRAM_BANK_SEL_Chan1] = new RegisterBitField("S2MM_SB_BRAM_BANK_SEL_Chan1", 7, 7);
	_FPGAregister[S2MMDMA_StatusReg2] = new FPGA_HardwareReg("S2MMDMA_StatusReg2", 1, BAR2, 0x40, 0, READONLY);
	_FPGAregister[S2MMDMA_StatusReg2]->BitField[S2MM_DMAcontrollerRev_Chan1] = new RegisterBitField("S2MM_DMAcontrollerRev_Chan1", 0, 3);
	_FPGAregister[S2MMDMA_StatusReg2]->BitField[S2MM_DESC_CHAIN_IRQ_Chan1] = new RegisterBitField("S2MM_DESC_CHAIN_IRQ_Chan1", 4, 4);
	_FPGAregister[S2MMDMA_StatusReg2]->BitField[S2MM_CHANNUMindex_Chan1] = new RegisterBitField("S2MM_CHANNUMindex_Chan1", 6, 7);
	_FPGAregister[S2MMDMA_ChainHead_Chan1] = new FPGA_HardwareReg("S2MMDMA_ChainHead_Chan1", 2, BAR2, 0x42, 0, WRITEONLY);
	_FPGAregister[S2MMDMA_ChainTail_Chan1] = new FPGA_HardwareReg("S2MMDMA_ChainTail_Chan1", 2, BAR2, 0x44, 0, WRITEONLY);
	_FPGAregister[S2MMDMA_IRQthreshld_Chan1] = new FPGA_HardwareReg("S2MMDMA_IRQthreshld_Chan1", 2, BAR2, 0x46, 0, WRITEONLY);

	_FPGAregister[S2MMDMA_ControlReg3] = new FPGA_HardwareReg("S2MMDMA_ControlReg3", 1, BAR2, 0x80, 0, WRITEONLY);
	_FPGAregister[S2MMDMA_ControlReg3]->BitField[S2MM_CONFIG_Chan2] = new RegisterBitField("S2MM_CONFIG_Chan2", 0, 0);
	_FPGAregister[S2MMDMA_ControlReg3]->BitField[S2MM_RUN_STOP_Chan2] = new RegisterBitField("S2MM_RUN_STOP_Chan2", 1, 1);
	_FPGAregister[S2MMDMA_ControlReg3]->BitField[S2MM_SG_CYCLIC_BD_Chan2] = new RegisterBitField("S2MM_SG_CYCLIC_BD_Chan2", 4, 4);
	_FPGAregister[S2MMDMA_ControlReg3]->BitField[S2MM_IRQ_REARM_Chan2] = new RegisterBitField("S2MM_IRQ_REARM_Chan2", 6, 6);
	_FPGAregister[S2MMDMA_ControlReg3]->BitField[S2MM_SB_BRAM_BANK_SEL_Chan2] = new RegisterBitField("S2MM_SB_BRAM_BANK_SEL_Chan2", 7, 7);
	_FPGAregister[S2MMDMA_StatusReg3] = new FPGA_HardwareReg("S2MMDMA_StatusReg3", 1, BAR2, 0x80, 0, READONLY);
	_FPGAregister[S2MMDMA_StatusReg3]->BitField[S2MM_DMAcontrollerRev_Chan2] = new RegisterBitField("S2MM_DMAcontrollerRev_Chan2", 0, 3);
	_FPGAregister[S2MMDMA_StatusReg3]->BitField[S2MM_DESC_CHAIN_IRQ_Chan2] = new RegisterBitField("S2MM_DESC_CHAIN_IRQ_Chan2", 4, 4);
	_FPGAregister[S2MMDMA_StatusReg3]->BitField[S2MM_CHANNUMindex_Chan2] = new RegisterBitField("S2MM_CHANNUMindex_Chan2", 6, 7);
	_FPGAregister[S2MMDMA_ChainHead_Chan2] = new FPGA_HardwareReg("S2MMDMA_ChainHead_Chan2", 2, BAR2, 0x82, 0, WRITEONLY);
	_FPGAregister[S2MMDMA_ChainTail_Chan2] = new FPGA_HardwareReg("S2MMDMA_ChainTail_Chan2", 2, BAR2, 0x84, 0, WRITEONLY);
	_FPGAregister[S2MMDMA_IRQthreshld_Chan2] = new FPGA_HardwareReg("S2MMDMA_IRQthreshld_Chan2", 2, BAR2, 0x86, 0, WRITEONLY);

	_FPGAregister[S2MMDMA_ControlReg4] = new FPGA_HardwareReg("S2MMDMA_ControlReg4", 1, BAR2, 0xC0, 0, WRITEONLY);
	_FPGAregister[S2MMDMA_ControlReg4]->BitField[S2MM_CONFIG_Chan3] = new RegisterBitField("S2MM_CONFIG_Chan3", 0, 0);
	_FPGAregister[S2MMDMA_ControlReg4]->BitField[S2MM_RUN_STOP_Chan3] = new RegisterBitField("S2MM_RUN_STOP_Chan3", 1, 1);
	_FPGAregister[S2MMDMA_ControlReg4]->BitField[S2MM_SG_CYCLIC_BD_Chan3] = new RegisterBitField("S2MM_SG_CYCLIC_BD_Chan3", 4, 4);
	_FPGAregister[S2MMDMA_ControlReg4]->BitField[S2MM_IRQ_REARM_Chan3] = new RegisterBitField("S2MM_IRQ_REARM_Chan3", 6, 6);
	_FPGAregister[S2MMDMA_ControlReg4]->BitField[S2MM_SB_BRAM_BANK_SEL_Chan3] = new RegisterBitField("S2MM_SB_BRAM_BANK_SEL_Chan3", 7, 7);
	_FPGAregister[S2MMDMA_StatusReg4] = new FPGA_HardwareReg("S2MMDMA_StatusReg4", 1, BAR2, 0xC0, 0, READONLY);
	_FPGAregister[S2MMDMA_StatusReg4]->BitField[S2MM_DMAcontrollerRev_Chan3] = new RegisterBitField("S2MM_DMAcontrollerRev_Chan3", 0, 3);
	_FPGAregister[S2MMDMA_StatusReg4]->BitField[S2MM_DESC_CHAIN_IRQ_Chan3] = new RegisterBitField("S2MM_DESC_CHAIN_IRQ_Chan3", 4, 4);
	_FPGAregister[S2MMDMA_StatusReg4]->BitField[S2MM_CHANNUMindex_Chan3] = new RegisterBitField("S2MM_CHANNUMindex_Chan3", 6, 7);
	_FPGAregister[S2MMDMA_ChainHead_Chan3] = new FPGA_HardwareReg("S2MMDMA_ChainHead_Chan3", 2, BAR2, 0xC2, 0, WRITEONLY);
	_FPGAregister[S2MMDMA_ChainTail_Chan3] = new FPGA_HardwareReg("S2MMDMA_ChainTail_Chan3", 2, BAR2, 0xC4, 0, WRITEONLY);
	_FPGAregister[S2MMDMA_IRQthreshld_Chan3] = new FPGA_HardwareReg("S2MMDMA_IRQthreshld_Chan3", 2, BAR2, 0xC6, 0, WRITEONLY);

	/////////  BAR 3 ////////////////////
	_FPGAregister[GlobalControlReg] = new FPGA_HardwareReg("GlobalControlReg", 4, BAR3, 0x0, 0, WRITEONLY);  // StopRun_FpgaRev
	_FPGAregister[GlobalControlReg]->BitField[GIGCR0_STOP_RUN] = new RegisterBitField("GIGCR0_STOP_RUN", 0, 0);  // Arm_ASSERT for ImageAcqTrigger (PT format)
	_FPGAregister[GlobalControlReg]->BitField[GIGCR0_LED1] = new RegisterBitField("GIGCR0_LED1", 1, 1);  // reversed from ThorDAQ FPGA
	_FPGAregister[GlobalControlReg]->BitField[GIGCR0_LED2] = new RegisterBitField("GIGCR0_LED2", 2, 2);
	_FPGAregister[GlobalControlReg]->BitField[ImageAcqTrig_HWSW_SEL] = new RegisterBitField("ImageAcqTrig_HWSW_SEL", 3, 3);
	_FPGAregister[GlobalControlReg]->BitField[BPI_FLASH_MSB24] = new RegisterBitField("BPI_FLASH_MSB24", 6, 6);
	_FPGAregister[GlobalControlReg]->BitField[BPI_FLASH_MSB25] = new RegisterBitField("BPI_FLASH_MSB25", 7, 7);


	_FPGAregister[GlobalFPGARevReg] = new FPGA_HardwareReg("GlobalFPGARevReg", 4, BAR3, 0x0, 0, READONLY);

	_FPGAregister[FlashStatusReg] = new FPGA_HardwareReg("FlashStatusReg", 2, BAR3, 0x4, 0, READONLY); // 1st byte is used, 8-15 "status" not referenced
	_FPGAregister[GlobalReadData] = new FPGA_HardwareReg("GlobalReadData", 1, BAR3, 0x6, 0, READONLY);

	_FPGAregister[GlobalImageSyncControlReg] = new FPGA_HardwareReg("GlobalImageSyncControlReg", 1, BAR3, 0x8, 0, WRITEONLY);  // ImgSyncCtrl
	_FPGAregister[GlobalImageSyncControlReg]->BitField[BIDIR_SCAN_MODE] = new RegisterBitField("BIDIR_SCAN_MODE", 1, 1);
	_FPGAregister[GlobalImageSyncControlReg]->BitField[SCAN_DIR] = new RegisterBitField("SCAN_DIR", 2, 2);
	// (dFLIM lacks advanced triggering)

	_FPGAregister[GlobalImageHSIZE] = new FPGA_HardwareReg("GlobalImageHSIZE", 2, BAR3, 0x10, 0, WRITEONLY);
	_FPGAregister[GlobalImageVSIZE] = new FPGA_HardwareReg("GlobalImageVSIZE", 2, BAR3, 0x18, 0, WRITEONLY);

	_FPGAregister[GlobalDBB1muxReg] = new FPGA_HardwareReg("GlobalDBB1muxReg", 8, BAR3, 0x20, 0, WRITEONLY);
	_FPGAregister[GlobalDBB1muxReg]->BitField[DBB1_IOCIRCUIT_8] = new RegisterBitField("DBB1_IOCIRCUIT_8", 0, 7);
	_FPGAregister[GlobalDBB1muxReg]->BitField[DBB1_IOCIRCUIT_7] = new RegisterBitField("DBB1_IOCIRCUIT_7", 8, 15);
	_FPGAregister[GlobalDBB1muxReg]->BitField[DBB1_IOCIRCUIT_4] = new RegisterBitField("DBB1_IOCIRCUIT_4", 16, 23);
	_FPGAregister[GlobalDBB1muxReg]->BitField[DBB1_IOCIRCUIT_3] = new RegisterBitField("DBB1_IOCIRCUIT_3", 24, 31);
	_FPGAregister[GlobalDBB1muxReg]->BitField[DBB1_IOCIRCUIT_6] = new RegisterBitField("DBB1_IOCIRCUIT_6", 32, 39);
	_FPGAregister[GlobalDBB1muxReg]->BitField[DBB1_IOCIRCUIT_5] = new RegisterBitField("DBB1_IOCIRCUIT_5", 40, 47);
	_FPGAregister[GlobalDBB1muxReg]->BitField[DBB1_IOCIRCUIT_2] = new RegisterBitField("DBB1_IOCIRCUIT_2", 48, 55);
	_FPGAregister[GlobalDBB1muxReg]->BitField[DBB1_IOCIRCUIT_1] = new RegisterBitField("DBB1_IOCIRCUIT_1", 56, 63);

	_FPGAregister[FPGAConfigFlashFIFO] = new FPGA_HardwareReg("FPGAConfigFlashFIFO", 4, BAR3, 0x38, 0, WRITEONLY);

	// PACKET_CHANNEL_GEN_CTRL_STRUCT ( 4 struct instances we allocated in Legacy kernel driver, but not accessed)
	_FPGAregister[BeatsPerFrame_Ch0] = new FPGA_HardwareReg("BeatsPerFrame_Ch0", 32, BAR3, 0x40, 0, WRITEONLY);
	_FPGAregister[IntraBeatDelay_HSize_Ch0] = new FPGA_HardwareReg("IntraBeatDelay_HSize_Ch0", 32, BAR3, 0x44, 0, WRITEONLY);
	_FPGAregister[IntraBeatFrameDelay_Ch0] = new FPGA_HardwareReg("IntraBeatFrameDelay_Ch0", 32, BAR3, 0x48, 0, WRITEONLY);

	_FPGAregister[BeatsPerFrame_Ch1] = new FPGA_HardwareReg("BeatsPerFrame_Ch1", 32, BAR3, 0x80, 0, WRITEONLY);
	_FPGAregister[IntraBeatDelay_HSize_Ch1] = new FPGA_HardwareReg("IntraBeatDelay_HSize_Ch1", 32, BAR3, 0x84, 0, WRITEONLY);
	_FPGAregister[IntraBeatFrameDelay_Ch1] = new FPGA_HardwareReg("IntraBeatFrameDelay_Ch1", 32, BAR3, 0x88, 0, WRITEONLY);
	_FPGAregister[BeatsPerFrame_Ch2] = new FPGA_HardwareReg("BeatsPerFrame_Ch2", 32, BAR3, 0xC0, 0, WRITEONLY);
	_FPGAregister[IntraBeatDelay_HSize_Ch2] = new FPGA_HardwareReg("IntraBeatDelay_HSize_Ch2", 32, BAR3, 0xC4, 0, WRITEONLY);
	_FPGAregister[IntraBeatFrameDelay_Ch2] = new FPGA_HardwareReg("IntraBeatFrameDelay_Ch2", 32, BAR3, 0xC8, 0, WRITEONLY);

	_FPGAregister[BeatsPerFrame_Ch3] = new FPGA_HardwareReg("BeatsPerFrame_Ch3", 32, BAR3, 0x100, 0, WRITEONLY);
	_FPGAregister[IntraBeatDelay_HSize_Ch3] = new FPGA_HardwareReg("IntraBeatDelay_HSize_Ch3", 32, BAR3, 0x104, 0, WRITEONLY);
	_FPGAregister[IntraBeatFrameDelay_Ch3] = new FPGA_HardwareReg("IntraBeatFrameDelay_Ch3", 32, BAR3, 0x108, 0, WRITEONLY);

// ThorDAQ Scanning Subsystem Registers
	_FPGAregister[ScanningSyncControlReg] = new FPGA_HardwareReg("ScanningSyncControlReg", 2, BAR3, 0x140, 0, WRITEONLY);
	_FPGAregister[ScanningSyncControlReg]->BitField[SRCE_SOF_MODE] = new RegisterBitField("SRCE_SOF_MODE", 0, 0);
	_FPGAregister[ScanningSyncControlReg]->BitField[FRM_GEN_MODE] = new RegisterBitField("FRM_GEN_MODE", 1, 1);
	_FPGAregister[ScanningSyncControlReg]->BitField[FRM_RETRIGGER] = new RegisterBitField("FRM_RETRIGGER", 2, 2);
	_FPGAregister[ScanningSyncControlReg]->BitField[ExternalPixelClock] = new RegisterBitField("ExternalPixelClock", 3, 3);
	_FPGAregister[ScanningSyncControlReg]->BitField[HorizontalLineSyncGapCtrl] = new RegisterBitField("HorizontalLineSyncGapCtrl", 4, 4);
	_FPGAregister[ScanningSyncControlReg]->BitField[TIME_SRCE_SELECT] = new RegisterBitField("TIME_SRCE_SELECT", 5, 7);
	_FPGAregister[ScanningSyncControlReg]->BitField[Capture_Active_Invert] = new RegisterBitField("Capture_Active_Invert", 8, 8);

	_FPGAregister[ScanningResonantPeriodReadbackReg] = new FPGA_HardwareReg("ScanningResonantPeriodReadbackReg", 2, BAR3, 0x140, 0, READONLY);
	_FPGAregister[ScanningFrameReadbackReg] = new FPGA_HardwareReg("ScanningFrameReadbackReg", 4, BAR3, 0x142, 0, READONLY);
	_FPGAregister[ScanningFrameCount] = new FPGA_HardwareReg("ScanningFrameCount", 2, BAR3, 0x142, 0, WRITEONLY);

	_FPGAregister[ADPLL_ControlReg] = new FPGA_HardwareReg("ADPLL_ControlReg", 1, BAR3, 0x148, 0, WRITEONLY);
	_FPGAregister[ADPLL_SyncDelay] = new FPGA_HardwareReg("ADPLL_SyncDelay", 2, BAR3, 0x150, 0, WRITEONLY);
	_FPGAregister[ADPLL_PhaseOffset] = new FPGA_HardwareReg("ADPLL_PhaseOffset", 2, BAR3, 0x152, 0, WRITEONLY);
	_FPGAregister[ADPLL_DCO_CenterFreq] = new FPGA_HardwareReg("ADPLL_DCO_CenterFreq", 4, BAR3, 0x158, 0, WRITEONLY);

	_FPGAregister[ScanningGalvoPixelDwell] = new FPGA_HardwareReg("ScanningGalvoPixelDwell", 2, BAR3, 0x160, 0, WRITEONLY);
	_FPGAregister[ScanningGalvoPixelDelay] = new FPGA_HardwareReg("ScanningGalvoPixelDelay", 2, BAR3, 0x168, 0, WRITEONLY);
	_FPGAregister[ScanningIntraLineDelay] = new FPGA_HardwareReg("ScanningIntraLineDelay", 2, BAR3, 0x170, 0, WRITEONLY);
	_FPGAregister[ScanningIntraFrameDelay] = new FPGA_HardwareReg("ScanningIntraFrameDelay", 4, BAR3, 0x178, 0, WRITEONLY);

	// Sampling Clock Gen
	_FPGAregister[SamplingClockControlReg] = new FPGA_HardwareReg("SamplingClockControlReg", 1, BAR3, 0x180, 0, WRITEONLY);
	_FPGAregister[SamplingClockControlReg]->BitField[SC_PHASE_INCREMENT_MODE] = new RegisterBitField("SC_PHASE_INCREMENT_MODE", 0, 0); // Writeonly 8-bits
	_FPGAregister[SamplingClockControlReg]->BitField[SC_FREQ_MEAS_SEL] = new RegisterBitField("SC_FREQ_MEAS_SEL", 1, 1); // 
	_FPGAregister[SamplingClockControlReg]->BitField[DDS_CLK_3P_EN] = new RegisterBitField("DDS_CLK_3P_EN", 2, 2); // 
	_FPGAregister[SamplingClockControlReg]->BitField[SC_3P_EN] = new RegisterBitField("SC_3P_EN", 3, 3); // 
	_FPGAregister[SamplingClockControlReg]->BitField[SC_CLK_CNT_CLEAR] = new RegisterBitField("SC_CLK_CNT_CLEAR", 4, 4); // 
	_FPGAregister[SamplingClockControlReg]->BitField[SC_SPI_TXRX_EN] = new RegisterBitField("SC_SPI_TXRX_EN", 5, 5); // 

	// ThorDAQ Sampling Clock Generation Subsystem Registers
	_FPGAregister[SamplingClockStatusReg] = new FPGA_HardwareReg("SamplingClockStatusReg", 1, BAR3, 0x180, 0, READONLY); // RO - 8 bits
	_FPGAregister[SamplingClockStatusReg]->BitField[SC_CLK_GEN_REV] = new RegisterBitField("SC_CLK_GEN_REV", 0, 2); // 
	_FPGAregister[SamplingClockStatusReg]->BitField[SC_SPI_TXRX_READY] = new RegisterBitField("SC_SPI_TXRX_READY", 3, 3); // 
	_FPGAregister[SamplingClockStatusReg]->BitField[SC_CLKRX_PLL_LOCK] = new RegisterBitField("SC_CLKRX_PLL_LOCK", 4, 4); // 
	_FPGAregister[SamplingClockStatusReg]->BitField[SC_LOCK_LOST_CNT] = new RegisterBitField("SC_LOCK_LOST_CNT", 5, 7); // 
	//	  _FPGAregister[SamplingClockStatusReg]->BitField[SC_LASER_CLK_CNT] = new RegisterBitField("SC_LASER_CLK_CNT", 8, 39); // 
	//	  _FPGAregister[SamplingClockStatusReg]->BitField[SC_CLKRX_RXBYTE] = new RegisterBitField("SC_CLKRX_RXBYTE", 40, 47); // 

	_FPGAregister[SamplingClock_LASER_CLK_CNT] = new FPGA_HardwareReg("SamplingClock_LASER_CLK_CNT", 4, BAR3, 0x181, 0, READONLY); // RO - 4 bytes (bits 8-39)
	_FPGAregister[SamplingClock_CLKRX_RXBYTE] = new FPGA_HardwareReg("SamplingClock_CLKRX_RXBYTE", 1, BAR3, 0x185, 0, READONLY);   // RO - 1 byte (bits 40-47)
	//  SamplingClock_CLKRX_RXBYTE, // 0x185 (RO, 1 byte)

	_FPGAregister[SamplingClockPhaseOffset] = new FPGA_HardwareReg("SamplingClockPhaseOffset", 2, BAR3, 0x188, 0, WRITEONLY); // 9-bits
	_FPGAregister[SamplingClockPhaseStep] = new FPGA_HardwareReg("SamplingClockPhaseStep", 1, BAR3, 0x190, 0, WRITEONLY);  // 8-bit
	_FPGAregister[SamplingClockPhaseLimit] = new FPGA_HardwareReg("SamplingClockPhaseLimit", 2, BAR3, 0x198, 0, WRITEONLY);  // 9-bit

	// dFLIM Front End register interface (bit fields TBD)
	// ThorDAQ DFlim FrontEnd Subsystem Registers (Gary Yellen interface - does not use I2C lines controlled by AIX Master @ 0x2C0)
	_FPGAregister[dFLIMfrontEndCtrl] = new FPGA_HardwareReg("dFLIMfrontEndCtrl", 8, BAR3, 0x1C0, 0, WRITEONLY); // WRITEonly
	_FPGAregister[dFLIMfrontEndStrobe] = new FPGA_HardwareReg("dFLIMfrontEndStrobe", 1, BAR3, 0x1C8, 0, WRITEONLY); // strobe indicates valid 1C0 data
	_FPGAregister[dFLIMfrontEndStatus] = new FPGA_HardwareReg("dFLIMfrontEndStatus", 8, BAR3, 0x1C0, 0, READONLY);

	_FPGAregister[ADCStreamDownsampleReg] = new FPGA_HardwareReg("ADCStreamDownsampleReg", 4, BAR3, 0x1D0, 0, WRITEONLY); //

	_FPGAregister[DC_OFFSET_LOW] = new FPGA_HardwareReg("DC_OFFSET_LOW", 8, BAR3, 0x1D8, 0, WRITEONLY); //  differs from ThorDAQ (ADCStreamDCoffsetChan0, 2 bytes)
	_FPGAregister[DC_OFFSET_HIGH] = new FPGA_HardwareReg("DC_OFFSET_HIGH", 8, BAR3, 0x1E0, 0, WRITEONLY); 

	/*
	_FPGAregister[ADCStreamFIRcoeffReg] = new FPGA_HardwareReg("ADCStreamFIRcoeffReg", 2, BAR3, 0x1E8, 0, WRITEONLY);

	_FPGAregister[ADCStreamPulseInterleaveOffsetReg] = new FPGA_HardwareReg("ADCStreamPulseInterleaveOffsetReg", 4, BAR3, 0x1F0, 0, WRITEONLY); // ThorDAQ is 2 bytes
	_FPGAregister[ADCThreePhotonSampleOffset] = new FPGA_HardwareReg("ADCThreePhotonSampleOffset", 4, BAR3, 0x1F8, 0, WRITEONLY); // ThorDAQ differs

	// ADC Interface (not yet verified in dFLIM hardware)
	_FPGAregister[ADCFMCInterfaceControlReg] = new FPGA_HardwareReg("ADCFMCInterfaceControlReg", 2, BAR3, 0x200, 0, WRITEONLY); // WRITEonly
	_FPGAregister[ADCFMCInterfaceControlReg]->BitField[JESD204B_config_core1] = new RegisterBitField("JESD204B_config_core1", 0, 0); // 
	_FPGAregister[ADCFMCInterfaceControlReg]->BitField[JESD204B_config_core2] = new RegisterBitField("JESD204B_config_core2", 1, 1); // 
	_FPGAregister[ADCFMCInterfaceControlReg]->BitField[SPI_Peripheral_Override] = new RegisterBitField("SPI_Peripheral_Override", 2, 2); // 
	_FPGAregister[ADCFMCInterfaceControlReg]->BitField[JESD204B_Clear_rxsync_loss] = new RegisterBitField("JESD204B_Clear_rxsync_loss", 4, 4); // 
	_FPGAregister[ADCFMCInterfaceControlReg]->BitField[JESD204B_Sysref_sync_to_laser] = new RegisterBitField("JESD204B_Sysref_sync_to_laser", 6, 6); // 
	_FPGAregister[ADCFMCInterfaceControlReg]->BitField[JESD204B_Test_Mode_Enable] = new RegisterBitField("JESD204B_Test_Mode_Enable", 7, 7); // 
	_FPGAregister[ADCFMCInterfaceControlReg]->BitField[ADC_GPIO0_INT_REF_SEL] = new RegisterBitField("ADC_GPIO0_INT_REF_SEL", 11, 11); // 
	_FPGAregister[ADCFMCInterfaceControlReg]->BitField[ADC_GPIO1_INT_REF_EN] = new RegisterBitField("ADC_GPIO1_INT_REF_EN", 12, 12); // 
	_FPGAregister[ADCFMCInterfaceControlReg]->BitField[ADC_GPIO2_L_FPGA_REF_EN] = new RegisterBitField("ADC_GPIO2_L_FPGA_REF_EN", 13, 13); // 
	_FPGAregister[ADCFMCInterfaceControlReg]->BitField[ADC_GPIO3_FiltAB_MODE] = new RegisterBitField("ADC_GPIO3_FiltAB_MODE", 14, 15); // 

	// ADC Digital ATtenuator settings, including AFE (Analog Front End)
	_FPGAregister[ADCInterfaceDAT12] = new FPGA_HardwareReg("ADCInterfaceDAT12", 1, BAR3, 0x208, 0, WRITEONLY); // WRITEonly
	_FPGAregister[ADCInterfaceDAT12]->BitField[ADCGainChan0] = new RegisterBitField("ADCGainChan0", 0, 2); // 3 bit "gain"
	_FPGAregister[ADCInterfaceDAT12]->BitField[ADCAFEenChan0] = new RegisterBitField("ADCAFEenChan0", 3, 3); // Analog Front End (AFE) enable
	_FPGAregister[ADCInterfaceDAT12]->BitField[ADCGainChan1] = new RegisterBitField("ADCGainChan1", 4, 6); // 3 bit "gain"
	_FPGAregister[ADCInterfaceDAT12]->BitField[ADCAFEenChan1] = new RegisterBitField("ADCAFEenChan1", 7, 7); // Analog Front End (AFE) enable
	_FPGAregister[ADCInterfaceDAT34] = new FPGA_HardwareReg("ADCInterfaceDAT34", 1, BAR3, 0x210, 0, WRITEONLY); // WRITEonly
	_FPGAregister[ADCInterfaceDAT34]->BitField[ADCGainChan2] = new RegisterBitField("ADCGainChan2", 0, 2); // 3 bit "gain"
	_FPGAregister[ADCInterfaceDAT34]->BitField[ADCAFEenChan2] = new RegisterBitField("ADCAFEenChan2", 3, 3); // Analog Front End (AFE) enable
	_FPGAregister[ADCInterfaceDAT34]->BitField[ADCGainChan3] = new RegisterBitField("ADCGainChan3", 4, 6); // 3 bit "gain"
	_FPGAregister[ADCInterfaceDAT34]->BitField[ADCAFEenChan3] = new RegisterBitField("ADCAFEenChan3", 7, 7); // Analog Front End (AFE) enable
	_FPGAregister[ADCInterfaceDAT56] = new FPGA_HardwareReg("ADCInterfaceDAT56", 1, BAR3, 0x218, 0, WRITEONLY); // WRITEonly
	_FPGAregister[ADCInterfaceDAT56]->BitField[ADCGainChan4] = new RegisterBitField("ADCGainChan4", 0, 2); // 3 bit "gain"
	_FPGAregister[ADCInterfaceDAT56]->BitField[ADCAFEenChan4] = new RegisterBitField("ADCAFEenChan4", 3, 3); // Analog Front End (AFE) enable
	_FPGAregister[ADCInterfaceDAT56]->BitField[ADCGainChan5] = new RegisterBitField("ADCGainChan5", 4, 6); // 3 bit "gain"
	_FPGAregister[ADCInterfaceDAT56]->BitField[ADCAFEenChan5] = new RegisterBitField("ADCAFEenChan5", 7, 7); // Analog Front End (AFE) enable
//	_FPGAregister[ADCInterface3PMarkersRDivide] = new FPGA_HardwareReg("ADCInterface3PMarkersRDivide", 1, BAR3, 0x230, 0, WRITEONLY);

	_FPGAregister[ADCFMCStatusReg] = new FPGA_HardwareReg("ADCFMCStatusReg", 2, BAR3, 0x200, 0, READONLY); // 0x200 READonly
	_FPGAregister[ADCFMCStatusReg]->BitField[ThorDAQ_ADC_VERSION] = new RegisterBitField("ThorDAQ_ADC_VERSION", 0, 3); // 
	_FPGAregister[ADCFMCStatusReg]->BitField[mailbox_tnsfr_cmplt] = new RegisterBitField("mailbox_tnsfr_cmplt", 4, 4); // 
	_FPGAregister[ADCFMCStatusReg]->BitField[jesd204b_rx_sync_chan0_1] = new RegisterBitField("jesd204b_rx_sync_chan0_1", 5, 5); // 
	_FPGAregister[ADCFMCStatusReg]->BitField[jesd204b_rx_sync_chan2_3] = new RegisterBitField("jesd204b_rx_sync_chan2_3", 6, 6); // 
	_FPGAregister[ADCFMCStatusReg]->BitField[jesd_rx_lost_cnt_chan0_1] = new RegisterBitField("jesd_rx_lost_cnt_chan0_1", 8, 10); // 
	_FPGAregister[ADCFMCStatusReg]->BitField[jesd_rx_lost_cnt_chan2_3] = new RegisterBitField("jesd_rx_lost_cnt_chan2_3", 11, 13); // 
*/
// GALVO Waveform Control (differs from ThorDAQ)
	_FPGAregister[GALVOWaveGenControlReg] = new FPGA_HardwareReg("GALVOWaveGenControlReg", 8, BAR3, 0x240, 0, WRITEONLY); 
	_FPGAregister[DAC_UpdateRate_Chan0] = new FPGA_HardwareReg("DAC_UpdateRate_Chan0", 2, BAR3, 0x248, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan1] = new FPGA_HardwareReg("DAC_UpdateRate_Chan1", 2, BAR3, 0x24A, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan2] = new FPGA_HardwareReg("DAC_UpdateRate_Chan2", 2, BAR3, 0x24C, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan3] = new FPGA_HardwareReg("DAC_UpdateRate_Chan3", 2, BAR3, 0x24E, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan4] = new FPGA_HardwareReg("DAC_UpdateRate_Chan4", 2, BAR3, 0x250, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan5] = new FPGA_HardwareReg("DAC_UpdateRate_Chan5", 2, BAR3, 0x252, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan6] = new FPGA_HardwareReg("DAC_UpdateRate_Chan6", 2, BAR3, 0x254, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan7] = new FPGA_HardwareReg("DAC_UpdateRate_Chan7", 2, BAR3, 0x256, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan8] = new FPGA_HardwareReg("DAC_UpdateRate_Chan8", 2, BAR3, 0x258, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan9] = new FPGA_HardwareReg("DAC_UpdateRate_Chan9", 2, BAR3, 0x25a, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan10] = new FPGA_HardwareReg("DAC_UpdateRate_Chan10", 2, BAR3, 0x25c, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan11] = new FPGA_HardwareReg("DAC_UpdateRate_Chan11", 2, BAR3, 0x25e, 0, WRITEONLY);
	// questionable if 0x260 - 0x266 are actually utilized
	_FPGAregister[DAC_Amplitude] = new FPGA_HardwareReg("DAC_Amplitude", 2, BAR3, 0x260, 0, WRITEONLY);
	_FPGAregister[DAC_StepSize_Chan0] = new FPGA_HardwareReg("DAC_StepSize_Chan0", 2, BAR3, 0x262, 0, WRITEONLY);
	_FPGAregister[DAC_StepSize_Chan1] = new FPGA_HardwareReg("DAC_StepSize_Chan1", 2, BAR3, 0x264, 0, WRITEONLY);
	_FPGAregister[DAC_StepSize_Chan2] = new FPGA_HardwareReg("DAC_StepSize_Chan2", 2, BAR3, 0x266, 0, WRITEONLY);
// Park voltages position galvos when SCAN not active
	_FPGAregister[DAC_ParkValue_Chan0] = new FPGA_HardwareReg("DAC_ParkValue_Chan0", 2, BAR3, 0x268, 0, WRITEONLY);
	_FPGAregister[DAC_ParkValue_Chan1] = new FPGA_HardwareReg("DAC_ParkValue_Chan1", 2, BAR3, 0x26A, 0, WRITEONLY);
	_FPGAregister[DAC_ParkValue_Chan2] = new FPGA_HardwareReg("DAC_ParkValue_Chan2", 2, BAR3, 0x26C, 0, WRITEONLY);
	_FPGAregister[DAC_ParkValue_Chan3] = new FPGA_HardwareReg("DAC_ParkValue_Chan3", 2, BAR3, 0x26E, 0, WRITEONLY);
	_FPGAregister[DAC_ParkValue_Chan4] = new FPGA_HardwareReg("DAC_ParkValue_Chan4", 2, BAR3, 0x270, 0, WRITEONLY);
	_FPGAregister[DAC_ParkValue_Chan5] = new FPGA_HardwareReg("DAC_ParkValue_Chan5", 2, BAR3, 0x272, 0, WRITEONLY);
	_FPGAregister[DAC_ParkValue_Chan6] = new FPGA_HardwareReg("DAC_ParkValue_Chan6", 2, BAR3, 0x274, 0, WRITEONLY);
	_FPGAregister[DAC_ParkValue_Chan7] = new FPGA_HardwareReg("DAC_ParkValue_Chan7", 2, BAR3, 0x276, 0, WRITEONLY);
	_FPGAregister[DAC_ParkValue_Chan8] = new FPGA_HardwareReg("DAC_ParkValue_Chan8", 2, BAR3, 0x278, 0, WRITEONLY);
	_FPGAregister[DAC_ParkValue_Chan9] = new FPGA_HardwareReg("DAC_ParkValue_Chan9", 2, BAR3, 0x27a, 0, WRITEONLY);
	_FPGAregister[DAC_ParkValue_Chan10] = new FPGA_HardwareReg("DAC_ParkValue_Chan10", 2, BAR3, 0x27c, 0, WRITEONLY);
	_FPGAregister[DAC_ParkValue_Chan11] = new FPGA_HardwareReg("DAC_ParkValue_Chan11", 2, BAR3, 0x27e, 0, WRITEONLY);
	// (NOTE - we are not defining channels not used)
	
// GALVO Waveform Control REG STRUCT 2 (dFLIM)
	
	_FPGAregister[DAC_Offset_Chan0] = new FPGA_HardwareReg("DAC_Offset_Chan0", 2, BAR3, 0x280, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan1] = new FPGA_HardwareReg("DAC_Offset_Chan1", 2, BAR3, 0x282, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan2] = new FPGA_HardwareReg("DAC_Offset_Chan2", 2, BAR3, 0x284, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan3] = new FPGA_HardwareReg("DAC_Offset_Chan3", 2, BAR3, 0x286, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan4] = new FPGA_HardwareReg("DAC_Offset_Chan4", 2, BAR3, 0x288, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan5] = new FPGA_HardwareReg("DAC_Offset_Chan5", 2, BAR3, 0x28a, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan6] = new FPGA_HardwareReg("DAC_Offset_Chan6", 2, BAR3, 0x28c, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan7] = new FPGA_HardwareReg("DAC_Offset_Chan7", 2, BAR3, 0x28e, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan8] = new FPGA_HardwareReg("DAC_Offset_Chan8", 2, BAR3, 0x290, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan9] = new FPGA_HardwareReg("DAC_Offset_Chan9", 2, BAR3, 0x292, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan10] = new FPGA_HardwareReg("DAC_Offset_Chan10", 2, BAR3, 0x294, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan11] = new FPGA_HardwareReg("DAC_Offset_Chan11", 2, BAR3, 0x296, 0, WRITEONLY);

	_FPGAregister[GlobalABBXmuxReg] = new FPGA_HardwareReg("GlobalABBXmuxReg", 6, BAR3, 0x298, 0, WRITEONLY);
	_FPGAregister[GlobalABBXmuxReg]->BitField[DAC_DMA00] = new RegisterBitField("DAC_DMA00", 0, 3); // 12 4-bit nibbles
	_FPGAregister[GlobalABBXmuxReg]->BitField[DAC_DMA01] = new RegisterBitField("DAC_DMA01", 4, 7);
	_FPGAregister[GlobalABBXmuxReg]->BitField[DAC_DMA02] = new RegisterBitField("DAC_DMA02", 8, 11);
	_FPGAregister[GlobalABBXmuxReg]->BitField[DAC_DMA03] = new RegisterBitField("DAC_DMA03", 12, 15);
	_FPGAregister[GlobalABBXmuxReg]->BitField[DAC_DMA04] = new RegisterBitField("DAC_DMA04", 16, 19);
	_FPGAregister[GlobalABBXmuxReg]->BitField[DAC_DMA05] = new RegisterBitField("DAC_DMA05", 20, 23);
	_FPGAregister[GlobalABBXmuxReg]->BitField[DAC_DMA06] = new RegisterBitField("DAC_DMA06", 24, 27);
	_FPGAregister[GlobalABBXmuxReg]->BitField[DAC_DMA07] = new RegisterBitField("DAC_DMA07", 28, 31);
	_FPGAregister[GlobalABBXmuxReg]->BitField[DAC_DMA08] = new RegisterBitField("DAC_DMA08", 32, 35);
	_FPGAregister[GlobalABBXmuxReg]->BitField[DAC_DMA09] = new RegisterBitField("DAC_DMA09", 36, 39);
	_FPGAregister[GlobalABBXmuxReg]->BitField[DAC_DMA10] = new RegisterBitField("DAC_DMA10", 40, 43);
	_FPGAregister[GlobalABBXmuxReg]->BitField[DAC_DMA11] = new RegisterBitField("DAC_DMA11", 44, 47); // (ends @ bit47)

	
	_FPGAregister[DOUpdateRate] = new FPGA_HardwareReg("DOUpdateRate", 8, BAR3, 0x2A0, 0, WRITEONLY); // these are ThorDAQ "digital waveforms" (chan12) which dFLIM cannot use
	_FPGAregister[DOParkValue] = new FPGA_HardwareReg("DOParkValue", 8, BAR3, 0x2A8, 0, WRITEONLY);
	_FPGAregister[DOOffset] = new FPGA_HardwareReg("DOOffset", 8, BAR3, 0x2B0, 0, WRITEONLY);
	_FPGAregister[DACAmFilterWindow] = new FPGA_HardwareReg("DACAmFilterWindow", 2, BAR3, 0x2B8, 0, WRITEONLY); // SWUG  Eof_dly_sample

	// The original dFLIM I2C master had Legacy code by Zhun, a clumsy interface which did NOT
	// support all the I2C protocol features we require (such as read/write for Lattice CPLD and SAMD21 MCU),
	// PLUS an additional interface answering at 0x1C0 (both interfaces were used to configure FMC126, but had
	// no support for other I2C devices). 
	// Early dFLIM FPGA firmware versions ONLY support Zhun code - later firmware versions (Jan-2024) only support AIX
	// The SCL,SDA hardware lines may only be controlled by one interface at a time
	// Xilinx AIX I2C Master (SHARED with Gary's LEGACY interface, AIX code mutually exclusive of legcay Zhun code)  
	// See docs: Xilinx CORE I2C interface (PG090, v2.0)
	_FPGAregister[I2C_AXIBridge] = new FPGA_HardwareReg("I2C_AXIBridge", 8, BAR3, 0x2C0, 0, WRITEONLY);
	_FPGAregister[I2C_AXIStatus] = new FPGA_HardwareReg("I2C_AXIStatus", 4, BAR3, 0x2C0, 0, READONLY);
	_FPGAregister[I2C_AXIControl] = new FPGA_HardwareReg("I2C_AXIControl", 8, BAR3, 0x2C8, 0, WRITEONLY);
	_FPGAregister[I2C_AXIControl]->BitField[I2C_AXIwrite] = new RegisterBitField("I2C_AXIwrite", 0, 0);
	_FPGAregister[I2C_AXIControl]->BitField[I2C_AXIread] = new RegisterBitField("I2C_AXIread", 1, 1);
	_FPGAregister[I2C_AXIControl]->BitField[TI9548AChipReset_] = new RegisterBitField("TI9548AChipReset_", 2, 2);
	// is this bit actually used by Gary's interface?  It was NOT being set in his command byte, but TI9548AChipReset_ was asserted (low) by his interface.
	_FPGAregister[I2C_AXIControl]->BitField[I2C_GYdFLIMmodeEn] = new RegisterBitField("I2C_GYdFLIMmodeEn", 3, 3);

	// dFLIM processing subsys
	_FPGAregister[GPOReg0_Ch0] = new FPGA_HardwareReg("GPOReg0_Ch0", 8, BAR3, 0x300, 0, WRITEONLY);
	_FPGAregister[GPOReg1_Ch0] = new FPGA_HardwareReg("GPOReg1_Ch0", 8, BAR3, 0x308, 0, WRITEONLY);
	_FPGAregister[GPOReg2_Ch0] = new FPGA_HardwareReg("GPOReg2_Ch0", 4, BAR3, 0x310, 0, WRITEONLY);
	_FPGAregister[GPOReg3_Ch0] = new FPGA_HardwareReg("GPOReg3_Ch0", 8, BAR3, 0x318, 0, WRITEONLY);
	_FPGAregister[GPOReg4_Ch0] = new FPGA_HardwareReg("GPOReg4_Ch0", 4, BAR3, 0x320, 0, WRITEONLY);
	_FPGAregister[GPOReg5_1_Ch0] = new FPGA_HardwareReg("GPOReg5_1_Ch0", 4, BAR3, 0x328, 0, WRITEONLY);
	_FPGAregister[GPOReg5_2_Ch0] = new FPGA_HardwareReg("GPOReg5_2_Ch0", 4, BAR3, 0x32C, 0, WRITEONLY);
	_FPGAregister[GPOReg6_Ch0] = new FPGA_HardwareReg("GPOReg6_Ch0", 8, BAR3, 0x330, 0, WRITEONLY);
	_FPGAregister[GPOReg7_Ch0] = new FPGA_HardwareReg("GPOReg7_Ch0", 8, BAR3, 0x338, 0, WRITEONLY);

	_FPGAregister[GPOReg0_Ch1] = new FPGA_HardwareReg("GPOReg0_Ch1", 8, BAR3, 0x340, 0, WRITEONLY);
	_FPGAregister[GPOReg1_Ch1] = new FPGA_HardwareReg("GPOReg1_Ch1", 8, BAR3, 0x348, 0, WRITEONLY);
	_FPGAregister[GPOReg2_Ch1] = new FPGA_HardwareReg("GPOReg2_Ch1", 4, BAR3, 0x350, 0, WRITEONLY);
	_FPGAregister[GPOReg3_Ch1] = new FPGA_HardwareReg("GPOReg3_Ch1", 8, BAR3, 0x358, 0, WRITEONLY);
	_FPGAregister[GPOReg4_Ch1] = new FPGA_HardwareReg("GPOReg4_Ch1", 4, BAR3, 0x360, 0, WRITEONLY);
	_FPGAregister[GPOReg5_1_Ch1] = new FPGA_HardwareReg("GPOReg5_1_Ch1", 4, BAR3, 0x368, 0, WRITEONLY);
	_FPGAregister[GPOReg5_2_Ch1] = new FPGA_HardwareReg("GPOReg5_2_Ch1", 4, BAR3, 0x36C, 0, WRITEONLY);
	_FPGAregister[GPOReg6_Ch1] = new FPGA_HardwareReg("GPOReg6_Ch1", 8, BAR3, 0x370, 0, WRITEONLY);
	_FPGAregister[GPOReg7_Ch1] = new FPGA_HardwareReg("GPOReg7_Ch1", 8, BAR3, 0x378, 0, WRITEONLY);

	_FPGAregister[GPOReg0_Ch2] = new FPGA_HardwareReg("GPOReg0_Ch2", 8, BAR3, 0x380, 0, WRITEONLY);
	_FPGAregister[GPOReg1_Ch2] = new FPGA_HardwareReg("GPOReg1_Ch2", 8, BAR3, 0x388, 0, WRITEONLY);
	_FPGAregister[GPOReg2_Ch2] = new FPGA_HardwareReg("GPOReg2_Ch2", 4, BAR3, 0x390, 0, WRITEONLY);
	_FPGAregister[GPOReg3_Ch2] = new FPGA_HardwareReg("GPOReg3_Ch2", 8, BAR3, 0x398, 0, WRITEONLY);
	_FPGAregister[GPOReg4_Ch2] = new FPGA_HardwareReg("GPOReg4_Ch2", 4, BAR3, 0x3A0, 0, WRITEONLY);
	_FPGAregister[GPOReg5_1_Ch2] = new FPGA_HardwareReg("GPOReg5_1_Ch2", 4, BAR3, 0x3A8, 0, WRITEONLY);
	_FPGAregister[GPOReg5_2_Ch2] = new FPGA_HardwareReg("GPOReg5_2_Ch2", 4, BAR3, 0x3AC, 0, WRITEONLY);
	_FPGAregister[GPOReg6_Ch2] = new FPGA_HardwareReg("GPOReg6_Ch2", 8, BAR3, 0x3B0, 0, WRITEONLY);
	_FPGAregister[GPOReg7_Ch2] = new FPGA_HardwareReg("GPOReg7_Ch2", 8, BAR3, 0x3B8, 0, WRITEONLY);

	_FPGAregister[GPOReg0_Ch3] = new FPGA_HardwareReg("GPOReg0_Ch3", 8, BAR3, 0x3C0, 0, WRITEONLY);
	_FPGAregister[GPOReg1_Ch3] = new FPGA_HardwareReg("GPOReg1_Ch3", 8, BAR3, 0x3C8, 0, WRITEONLY);
	_FPGAregister[GPOReg2_Ch3] = new FPGA_HardwareReg("GPOReg2_Ch3", 4, BAR3, 0x3D0, 0, WRITEONLY);
	_FPGAregister[GPOReg3_Ch3] = new FPGA_HardwareReg("GPOReg3_Ch3", 8, BAR3, 0x3D8, 0, WRITEONLY);
	_FPGAregister[GPOReg4_Ch3] = new FPGA_HardwareReg("GPOReg4_Ch3", 4, BAR3, 0x3E0, 0, WRITEONLY);
	_FPGAregister[GPOReg5_1_Ch3] = new FPGA_HardwareReg("GPOReg5_1_Ch3", 4, BAR3, 0x3E8, 0, WRITEONLY);
	_FPGAregister[GPOReg5_2_Ch3] = new FPGA_HardwareReg("GPOReg5_2_Ch3", 4, BAR3, 0x3EC, 0, WRITEONLY);
	_FPGAregister[GPOReg6_Ch3] = new FPGA_HardwareReg("GPOReg6_Ch3", 8, BAR3, 0x3F0, 0, WRITEONLY);
	_FPGAregister[GPOReg7_Ch3] = new FPGA_HardwareReg("GPOReg7_Ch3", 8, BAR3, 0x3F8, 0, WRITEONLY);

	// for real-time sensitive shadow regs, get indexes in advance to avoid string search at scan-time
	BOOL bstatus;
	// Searching the bit field name gives you the register it belongs to
	bstatus = SearchFPGAregFieldIndices("S2MM_IRQ_REARM_Chan0", (int)strlen("S2MM_IRQ_REARM_Chan0"), &_S2MMDMA_IRQ_REARM_RegIndex[0], &_nullBitField);
	bstatus = SearchFPGAregFieldIndices("S2MM_IRQ_REARM_Chan1", (int)strlen("S2MM_IRQ_REARM_Chan1"), &_S2MMDMA_IRQ_REARM_RegIndex[1], &_nullBitField);
	bstatus = SearchFPGAregFieldIndices("S2MM_IRQ_REARM_Chan2", (int)strlen("S2MM_IRQ_REARM_Chan2"), &_S2MMDMA_IRQ_REARM_RegIndex[2], &_nullBitField);
	bstatus = SearchFPGAregFieldIndices("S2MM_IRQ_REARM_Chan3", (int)strlen("S2MM_IRQ_REARM_Chan3"), &_S2MMDMA_IRQ_REARM_RegIndex[3], &_nullBitField);
	bstatus = SearchFPGAregFieldIndices("S2MMDMA_StatusReg1", (int)strlen("S2MMDMA_StatusReg1"), &_S2MMDMAStatusRegIndex[0], &_nullBitField);
	bstatus = SearchFPGAregFieldIndices("S2MMDMA_StatusReg2", (int)strlen("S2MMDMA_StatusReg2"), &_S2MMDMAStatusRegIndex[1], &_nullBitField);
	bstatus = SearchFPGAregFieldIndices("S2MMDMA_StatusReg3", (int)strlen("S2MMDMA_StatusReg3"), &_S2MMDMAStatusRegIndex[2], &_nullBitField);
	bstatus = SearchFPGAregFieldIndices("S2MMDMA_StatusReg4", (int)strlen("S2MMDMA_StatusReg4"), &_S2MMDMAStatusRegIndex[3], &_nullBitField);

	GetDMAcapability();

	srand(0x36A9); // seed random number generator
    return;
} // end of CdFLIM_4002 CONSTRUCTOR

/**********************************************************************************************//**
 * @fn	ThordaqDFLIM::~ThordaqDFLIM()
 *
 * @brief	Destructor.
 *
 * @author	Cge
 * @date	3/17/2017
 **************************************************************************************************/

CdFLIM_4002::~CdFLIM_4002()
{
	//***reset all global available
    gBoardIndex = -1; //no board is connected 
    // Reset to no DMA Engines found
    gDmaInfo.PacketRecvEngineCount = 0;
    gDmaInfo.PacketSendEngineCount = 0;
    gDmaInfo.AddressablePacketMode = false;
	//*** Free all allocated thordaq struct
	SAFE_DELETE_PTR(gPtrAcqCtrl); 
    if ( gDeviceInfo != NULL )
    {
//        SetupDiDestroyDeviceInfoList ( gDeviceInfo ); // Free the Info handle
        gDeviceInfo = NULL;
    }
    if ( gDeviceInterfaceDetailData != NULL )
    {
        free ( gDeviceInterfaceDetailData ); // Free interface detail handle
        gDeviceInterfaceDetailData = NULL;
    }
    return;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM::GetDMACapability
 *
 * @brief	Interrogate the NWL DMA interface
 *
 * @author	DZimmerman
 * @date	12/09/2023
 *
 * @return	STATUS_SUCCESSFUL if DMA data captured
 **************************************************************************************************/

THORDAQ_STATUS CdFLIM_4002::GetDMAcapability()
{

	// Get Board DMA Configuration. 
	// Initiate onBoard parameters.
	BOARD_INFO_STRUCT		 board_cfg_info;
	if (GetBoardCfg ( &board_cfg_info ) != THORDAQ_STATUS::STATUS_SUCCESSFUL)
	{
		return STATUS_GET_BOARD_CONFIG_ERROR;
	}
	//// set flag for other function calls
    //this->AttachedToDriver = true;
    // Assume no DMA Engines found
	DMA_CAP_STRUCT DMA_cap; // dma capacity
    gDmaInfo.PacketRecvEngineCount = 0;
    gDmaInfo.PacketSendEngineCount = 0;
    gDmaInfo.AddressablePacketMode = false;
    // Get DMA Engine cap to extract the engine numbers for the packet and block mode engines
    // Base Configuration has 1 Card to System DMA Engine and 1 System to Card DMA Engine
	// Multi-Engine Configuration has 1-4 Card to System DMA Engines and 1-4 System to Card DMA Engines
	for (char i = 0; i < MAX_NUM_DMA_ENGINES; i+=32 ) // there are ONLY 2 engines, at index 0 and 32
    {
        gDmaInfo.PacketRecvEngine[i] = -1;
        gDmaInfo.PacketSendEngine[i] = -1;
        GetDMAEngineCap ( i, &DMA_cap );

        if ( ( DMA_cap.dma_capabilityabilities & DMA_CAP_ENGINE_PRESENT ) == DMA_CAP_ENGINE_PRESENT )
        {
            if ( ( DMA_cap.dma_capabilityabilities & DMA_CAP_ENGINE_TYPE_MASK ) & DMA_CAP_PACKET_DMA )
            {
                if ( ( DMA_cap.dma_capabilityabilities & DMA_CAP_ENGINE_TYPE_MASK ) & DMA_CAP_ADDRESSABLE_PACKET_DMA )
                {
                    gDmaInfo.AddressablePacketMode = true;
                }
                if ( ( DMA_cap.dma_capabilityabilities & DMA_CAP_DIRECTION_MASK ) == DMA_CAP_SYSTEM_TO_CARD )
                {
                    gDmaInfo.PacketSendEngine[gDmaInfo.PacketSendEngineCount++] = i;
                }
				else if ( ( DMA_cap.dma_capabilityabilities & DMA_CAP_DIRECTION_MASK ) == DMA_CAP_CARD_TO_SYSTEM )
                {
                     gDmaInfo.PacketRecvEngine[gDmaInfo.PacketRecvEngineCount++] = i;
                }
				else
                {
#if _DEBUG
                    printf("of invalid type\n");
#endif
				}
            }
        }
    }

	SetDACChannelMapping();
	SetADCChannelMapping();
	//SetDLIMFrontEndSettings();

	return STATUS_SUCCESSFUL;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM:: DisconnectFromBoard()
 *
 * @brief	Disconnect to the device.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @return	STATUS_SUCCESSFUL if Thordaq disconnected. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS CdFLIM_4002:: DisconnectFromBoard()
{
	//Free Device handle
    if ( gHdlDevice != INVALID_HANDLE_VALUE )
    {
        for (int i = 0; i < gDmaInfo.PacketRecvEngineCount; i++ )
        {
			if( ReleasePacketBuffers ( i ) != STATUS_SUCCESSFUL)
			{
				return STATUS_RELEASE_PACKET_MODE_INCOMPLETE;
			}
        }
        CloseHandle ( gHdlDevice );
        gHdlDevice = INVALID_HANDLE_VALUE;
    }
    // Free up device detail
    if ( gDeviceInterfaceDetailData != NULL )
    {
        free ( gDeviceInterfaceDetailData );
        gDeviceInterfaceDetailData = NULL;
    }
    return STATUS_SUCCESSFUL;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM:: GetBoardCfg ( BOARD_INFO_STRUCT* board_info )
 *
 * @brief	Get the configuration of the device.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param [in,out]	board_info	Firmware Version number.
 *
 * @return	STATUS_SUCCESSFUL if Thordaq return. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS CdFLIM_4002:: GetBoardCfg (
    BOARD_INFO_STRUCT*	    board_info  // Returned structure
)
{
    THORDAQ_STATUS			status            = STATUS_SUCCESSFUL;
    DWORD					bytes_returned    = 0;
    DWORD					last_error_status = 0;
    BOARD_CONFIG_STRUCT     board_config;
	OVERLAPPED overlapped;  // OVERLAPPED structure for the operation

	// Initiate Firmware and Driver version number
	board_info->DriverVersionBuildNumber = 0;
	board_info->DriverVersionMajor		 = 0;
	board_info->DriverVersionMinor       = 0;
	board_info->DriverVersionSubMinor    = 0;
	board_info->UserVersion              = 0;

	//Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );
	DWORD ioctlCode = GET_BOARD_CONFIG_IOCTL;
    // Send GET_BOARD_CONFIG_IOCTL
    if ( !DeviceIoControl ( gHdlDevice, ioctlCode, NULL, 0, ( LPVOID ) &board_config, sizeof ( BOARD_CONFIG_STRUCT ), &bytes_returned, &overlapped ) )
    {
        last_error_status = GetLastError();

        if ( last_error_status != ERROR_IO_PENDING )
        {
#if _DEBUG
            printf ( "GetBoardCfg IOCTL call failed. Error = %d\n", GetLastError() );
#endif // _DEBUG            
			status = STATUS_GET_BOARD_CONFIG_ERROR;
        } else
        {
            // Wait here (forever) for the Overlapped I/O to complete
            if ( !GetOverlappedResult ( gHdlDevice, &overlapped, &bytes_returned, TRUE ) )
            {
                last_error_status = GetLastError();
#if _DEBUG
                printf ( "GetBoardCfg IOCTL call failed. Error = %d\n", last_error_status );
#endif // _DEBUG
                status = STATUS_GET_BOARD_CONFIG_ERROR;
            }
        }
    }

    // check returned structure size
    if ( ( bytes_returned != sizeof ( BOARD_CONFIG_STRUCT ) ) && ( status == STATUS_SUCCESSFUL ) )
    {
        // ioctl failed
#if _DEBUG
        printf ( "GetBoardCfg IOCTL returned invalid size (%d)\n", bytes_returned );
#endif // _DEBUG
        status = STATUS_GET_BOARD_CONFIG_ERROR;
    } else
    {
		board_info->DriverVersionBuildNumber = board_config.DriverVersionBuildNumber;
		board_info->DriverVersionMajor		 = board_config.DriverVersionMajor;
		board_info->DriverVersionMinor       = board_config.DriverVersionMinor;
		board_info->DriverVersionSubMinor    = board_config.DriverVersionSubMinor;
		board_info->UserVersion              = board_config.UserVersion;
		board_info->PCIVendorDeviceID		 = board_config.PciConfig.DeviceId;
	}
	if(overlapped.hEvent != 0 )
		CloseHandle ( overlapped.hEvent );
    return status;
}


/////// restore NWL API, and define DZ SDK based on that API /////////////////
THORDAQ_STATUS CdFLIM_4002::DoMem(
	UINT32          Rd_Wr_n,        // 1==Read, 0==Write
	UINT32          BarNum,         // Base Address Register (BAR) to access
	PUINT8          Buffer,         // Data buffer
	UINT64          Offset,         // Offset in data buffer to start transfer
	UINT64          CardOffset,     // Offset in BAR to start transfer
	UINT64          Length,         // Byte length of transfer
	PSTAT_STRUCT    Status          // Completion Status
)
{
	THORDAQ_STATUS status = WriteReadRegister(Rd_Wr_n, BarNum, CardOffset, Buffer, Offset, Length, Status);
	return status;
}

/*! ReleasePacketBuffers
 *
 * \brief Sends two PACKET_BUF_DEALLOC_IOCTL calls to the driver to teardown the recieve buffer
 *  and teardown the descriptors for sending packets
 * \param EngineOffset - DMA Engine number offset to use
 * \return Completion status.
 */
THORDAQ_STATUS CdFLIM_4002::ReleasePacketBuffers(
	INT32                   EngineOffset
)
{
	BUF_DEALLOC_STRUCT      BufDeAlloc;
	OVERLAPPED              os;                 // OVERLAPPED structure for the operation
	DWORD                   bytesReturned = 0;
	DWORD                   LastErrorStatus = 0;
	THORDAQ_STATUS          status = STATUS_INVALID_MODE;

	os.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (os.hEvent == NULL)
	{
		return (THORDAQ_STATUS)GetLastError();
	}

	//    if (EngineOffset < gDmaInfo.PacketRecvEngineCount)
	if (EngineOffset == 0 || EngineOffset == 32)// gDmaInfo.PacketRecvEngineCount)
	{
		// Set the DMA Engine we want to de-allocate
		BufDeAlloc.EngineNum = EngineOffset;// gDmaInfo.PacketRecvEngine[EngineOffset];
		// Set the allocation mode to what we used above
		BufDeAlloc.Reserved = 0;
		// Return the Buffer Address we recieved from the Allocate call
		BufDeAlloc.RxBufferAddress = _pRxPacketBufferHandle; // always set to NULL in NWL original code

		// Send Packet Mode Release
		if (!DeviceIoControl(gHdlDevice, PACKET_BUF_RELEASE_IOCTL,
			&BufDeAlloc, sizeof(BUF_DEALLOC_STRUCT),
			NULL, 0,
			&bytesReturned, &os))
		{
			LastErrorStatus = GetLastError();
			if (LastErrorStatus == ERROR_IO_PENDING)
			{
				// Wait here (forever) for the Overlapped I/O to complete
				if (!GetOverlappedResult(gHdlDevice, &os, &bytesReturned, TRUE))
				{
					LastErrorStatus = GetLastError();
					status = (THORDAQ_STATUS)LastErrorStatus;
				}
				else
				{
					_pRxPacketBufferHandle = NULL;
					status = STATUS_SUCCESSFUL;
				}
			}
			else
			{
				// ioctl failed
#if _DEBUG
				printf("Packet Rx buffer DeAllocate failed. Error = %d\n", LastErrorStatus);
#endif // _DEBUG
				status = (THORDAQ_STATUS)LastErrorStatus;
			}
		}
		else
		{
			_pRxPacketBufferHandle = NULL;
			status = STATUS_SUCCESSFUL;
		}
	}
	CloseHandle(os.hEvent);
	BOOL bStatus = ReleaseMutex(_nwlDMAdescriptorsMutex);

	return status;
}


/*! SetupPacket
 *
 * \brief Sends PACKET_BUF_ALLOC_IOCTL call to the driver to setup the recieve buffer
 *  and intialize the descriptors for Reading or Receiving packets
 * \param EngineOffset
 * \param Buffer
 * \param BufferSize
 * \param MaxPacketSize
 * \param PacketModeSetting
 * \param NumberDescriptors
 * \return Completion status.
 */
THORDAQ_STATUS CdFLIM_4002::SetupPacket(
	INT32   EngineOffset,       // Engine NUMBER: 0 for WRITE, 32 for READ
	PUINT8  Buffer,
	PUINT32 BufferSize,
	PUINT32 MaxPacketSize,
	INT32   PacketModeSetting,
	INT32   NumberDescriptors
)
{
	BUF_ALLOC_STRUCT        BufAlloc;
	OVERLAPPED              os;         // OVERLAPPED structure for the operation
	DWORD                   bytesReturned = 0;
	DWORD                   LastErrorStatus = 0;
	THORDAQ_STATUS          status = STATUS_SUCCESSFUL;

	os.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (os.hEvent == NULL)
	{
		return (THORDAQ_STATUS)GetLastError();
	}
	// DMA Engine INDEX to use (always 0, ThorDAQ has 1 read engine, 1 write)
//	if (EngineOffset < gDmaInfo.PacketRecvEngineCount)
	if (EngineOffset == 0 || EngineOffset == 32)  // write OR read?
	{
		// Set the DMA Engine we want to allocate for
		BufAlloc.EngineNum = EngineOffset; // gDmaInfo.PacketRecvEngine[EngineOffset]; // (read or write, always 0)

		if ((PacketModeSetting == PACKET_MODE_FIFO) ||
			(PacketModeSetting == PACKET_MODE_STREAMING))
		{
			// This is an application allocated the buffer
			BufAlloc.AllocationMode = PacketModeSetting;
			// Allocate the size of...
			BufAlloc.Length = *BufferSize;
			// Allocate the number of decriptors based on the Maximum Packet Size we can handle
			BufAlloc.MaxPacketSize = *MaxPacketSize;

			// Send FIFO Packet Mode Setup IOCTL  (NOT for ThorDAQ)
			if (!DeviceIoControl(gHdlDevice, PACKET_BUF_ALLOC_IOCTL,
				&BufAlloc, sizeof(BUF_ALLOC_STRUCT),
				(LPVOID)Buffer, (DWORD)*BufferSize,
				&bytesReturned, &os))
			{
				LastErrorStatus = GetLastError();
				if (LastErrorStatus == ERROR_IO_PENDING)
				{
					// Wait here (forever) for the Overlapped I/O to complete
					if (!GetOverlappedResult(gHdlDevice, &os, &bytesReturned, TRUE))
					{
						LastErrorStatus = GetLastError();
#if _DEBUG
						printf("FIFO Packet Mode setup overlap failed. Error = %d\n", LastErrorStatus);
#endif // _DEBUG
						status = (THORDAQ_STATUS)LastErrorStatus;
					}
				}
				else
				{
					// ioctl failed
#if _DEBUG
					printf("FIFO Packet Mode setup failed. Error = %d\n", LastErrorStatus);
#endif // _DEBUG
					status = (THORDAQ_STATUS)LastErrorStatus;
				}
			}
		}
		else  if (PacketModeSetting == PACKET_MODE_ADDRESSABLE) // ThorDAQ preferred mode
		{
			// acquire MUTEX -- release in ReleasePacketBuffers()
			DWORD MutexWaitResult = WaitForSingleObject(
				_nwlDMAdescriptorsMutex,    // handle to mutex
				INFINITE);  // no time-out interval

			// Setup in Addressable Packet mode
			BufAlloc.AllocationMode = PacketModeSetting;
			BufAlloc.Length = 0;
			BufAlloc.MaxPacketSize = 0;
			BufAlloc.NumberDescriptors = NumberDescriptors;

			// Send Setup Packet Mode Addressable IOCTL (into Kernel driver)
			if (!DeviceIoControl(gHdlDevice, PACKET_BUF_ALLOC_IOCTL,
				&BufAlloc, sizeof(BUF_ALLOC_STRUCT),
				NULL, 0,     // no buffer Mapping - done later by ReadEx or WriteEx
				&bytesReturned, &os))
			{
				LastErrorStatus = GetLastError();
				if (LastErrorStatus == ERROR_IO_PENDING)
				{
					// Wait here (forever) for the Overlapped I/O to complete
					if (!GetOverlappedResult(gHdlDevice, &os, &bytesReturned, TRUE))
					{
						LastErrorStatus = GetLastError();
#if _DEBUG
						printf("Addressable Packet Mode setup overlap failed. Error = %d\n", LastErrorStatus);
#endif // _DEBUG
						status = (THORDAQ_STATUS)LastErrorStatus;
					}
				}
				else
				{
					// ioctl failed
#if _DEBUG
					printf("Addressable Packet mode setup failed. Error = %d\n", LastErrorStatus);
#endif // _DEBUG
					status = (THORDAQ_STATUS)LastErrorStatus;
				}
			}
		}
		else
		{
			status = STATUS_INVALID_MODE;
		}
	}
	else
	{
#if _DEBUG
		printf("No Packet Mode DMA Engines found\n");
#endif // _DEBUG
		status = STATUS_INVALID_MODE;
	}

	CloseHandle(os.hEvent);
	return status;
}

/*! PacketWriteEx
 *
 * \brief Send a PACKET_WRITE_IOCTL call to the driver
 * \param EngineOffset - DMA Engine number offset to use
 * \param UserControl - User Control to set in the first DMA Descriptor
 * \param CardOffset
 * \param Mode - Control Mode Flags
 * \param Buffer
 * \param Length
 * \return Completion status.
 */
THORDAQ_STATUS CdFLIM_4002::PacketWriteEx
(
	INT32                   EngineOffset,
	UINT64                  UserControl,
	UINT64                  CardOffset,
	UINT32                  Mode,
	PUINT8                  Buffer,
	UINT32                  Length
)
{
	PACKET_WRITE_STRUCT     sPacketWrite;
	OVERLAPPED              os;         // OVERLAPPED structure for the operation
	DWORD                   bytesReturned = 0;
	DWORD                   LastErrorStatus = 0;
	UINT32                  status = STATUS_SUCCESSFUL;

	os.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (os.hEvent == NULL)
	{
		return (THORDAQ_STATUS)GetLastError();
	}

	if (EngineOffset < gDmaInfo.PacketSendEngineCount)
	{
		// Select a Packet Send DMA Engine
		sPacketWrite.EngineNum = gDmaInfo.PacketSendEngine[EngineOffset];
		sPacketWrite.UserControl = UserControl;
		sPacketWrite.ModeFlags = Mode;
		sPacketWrite.CardOffset = CardOffset;
		sPacketWrite.Length = Length;

		if (!DeviceIoControl(gHdlDevice, PACKET_WRITE_IOCTL,
			&sPacketWrite, sizeof(PACKET_WRITE_STRUCT),
			(LPVOID)Buffer, (DWORD)Length,
			&bytesReturned, &os))
		{
			LastErrorStatus = (THORDAQ_STATUS)GetLastError();
			if (LastErrorStatus == ERROR_IO_PENDING)
			{
				// Wait here (forever) for the Overlapped I/O to complete
				if (!GetOverlappedResult(gHdlDevice, &os, &bytesReturned, TRUE)) // hangs here when IRQs not working - happens routinely when kernel driver reloaded (need reboot)
				{
					LastErrorStatus = GetLastError();
#if _DEBUG
					printf("Packet Write Overlapped failed. Error = %d\n", LastErrorStatus);
#endif // _DEBUG
					status = LastErrorStatus;
				}
			}
			else
			{
				// ioctl failed
#if _DEBUG
				printf("Packet Write failed, Error = %d\n", LastErrorStatus);
#endif // _DEBUG
				status = LastErrorStatus;
			}
		}  // if (!DeviceIoControl...
		// Make sure we returned something useful
		if (status == STATUS_SUCCESSFUL)
		{
			if (bytesReturned != Length)
			{
#if _DEBUG
				printf("Packet Write failed. Return size does not equal request (Ret=%d)\n", bytesReturned);
#endif // _DEBUG
				status = STATUS_INCOMPLETE;
			}
		}
	}
	else
	{
#if _DEBUG
		printf("DLL: Packet Write failed. No Packet Send Engine\n");
#endif // _DEBUG
		status = STATUS_INVALID_MODE;
	}
	CloseHandle(os.hEvent);
	return (THORDAQ_STATUS)status;
}

/*! PacketRead
 *
 * \brief Send a PACKET_READ_IOCTL call to the driver
 * \param EngineOffset - DMA Engine number offset to use
 * \param UserStatus - User Status to set in the first DMA Descriptor
 * \param CardOffset
 * \param Mode - Control Mode Flags
 * \param Buffer
 * \param Length
 * \return Completion status.
 */

THORDAQ_STATUS CdFLIM_4002::PacketReadEx(
	INT32                   EngineOffset,       //
	PUINT64                 UserStatus,         //
	UINT64                  CardOffset,
	UINT32                  Mode,               //
	PUINT8                  Buffer,
	PUINT32                 Length
)
{
	PACKET_READ_STRUCT      sPacketRead;
	PACKET_RET_READ_STRUCT  sRetPacketRead;
	OVERLAPPED              os;         // OVERLAPPED structure for the operation
	DWORD                   bytesReturned = 0;
	DWORD                   LastErrorStatus = 0;
	UINT32                  status = STATUS_SUCCESSFUL;

	os.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (os.hEvent == NULL)
	{
		return (THORDAQ_STATUS)GetLastError();
	}
	if (EngineOffset == 32)// gDmaInfo.PacketRecvEngineCount)  [only one read engine]
	{
		// Select a Packet Read DMA Engine
		sPacketRead.EngineNum = EngineOffset;// gDmaInfo.PacketRecvEngine[EngineOffset];
		sPacketRead.CardOffset = CardOffset;   // ThorDAQ DDR3
		sPacketRead.ModeFlags = Mode;         // PACKET_MODE_ADDRESSABLE
		sPacketRead.BufferAddress = (UINT64)Buffer; // System(Host) physical memory to MAP
		sPacketRead.Length = *Length;

		if (!DeviceIoControl(gHdlDevice, PACKET_READ_IOCTL,
			&sPacketRead, sizeof(PACKET_READ_STRUCT),
			&sRetPacketRead, sizeof(PACKET_RET_READ_STRUCT),
			&bytesReturned, &os))
		{
			LastErrorStatus = GetLastError();
			if (LastErrorStatus == ERROR_IO_PENDING)
			{
				// Wait here (forever) for the Overlapped I/O to complete
				if (!GetOverlappedResult(gHdlDevice, &os, &bytesReturned, TRUE))
				{
					LastErrorStatus = GetLastError();
#if _DEBUG
					printf("Packet Read Overlapped failed. Error = %d\n", LastErrorStatus); DebugBreak();
#endif // _DEBUG
					status = LastErrorStatus;
				}
			}
			else
			{
				// ioctl failed
#if _DEBUG
				printf("Packet Read failed, Error = %d\n", LastErrorStatus); DebugBreak();
#endif // _DEBUG
				status = LastErrorStatus;
			}
		} // if (!DeviceIoControl ...
		*UserStatus = 0;
		*Length = 0;
		// Make sure we returned something useful
		if (status == STATUS_SUCCESSFUL)
		{
			if (bytesReturned == sizeof(PACKET_RET_READ_STRUCT))
			{
				*UserStatus = sRetPacketRead.UserStatus;
				*Length = sRetPacketRead.Length;
			}
			else
			{
#if _DEBUG
				printf("Packet Read failed. Return structure size is mismatched (Ret=%d)\n", bytesReturned);  DebugBreak();
#endif // _DEBUG
				status = STATUS_INCOMPLETE;
			}
		}
	} // if (EngineOffset ...
	else
	{
#if _DEBUG
		printf("DLL: Packet Read failed. No Packet Read Engine\n");  DebugBreak();
#endif // _DEBUG
		status = STATUS_INVALID_MODE;
	}
	CloseHandle(os.hEvent);
	return (THORDAQ_STATUS)status;
}

// SHADOW REGISTER functions in support of API calls
/////  DZ SDK functions

#pragma optimize( "", off )
inline UINT64 FPGA_HardwareReg::Read(int Field)
{
	BYTE bField = Field & 63; // only 64 bit fields possible
	// if Field is "-1", we read entire register; otherwise the Field
	if (Field == -1)
	{
		if (_size <= 8)
		{
			return _VALUE; // return shadow copy
		}
		else // handle register set/table
		{
			// TBD - requires changing argument to pointer to array of UINT64
		}
	}
	// get mask out and return the field
	UINT64 mask = 0xffffffffffffffff >> (static_cast<UINT64>(64) - BitField[bField]->_bitFldLen);
	UINT64 shiftedValue = _VALUE >> BitField[Field]->_bitNumStart;
#define	WCHAR_MSG_SIZE 128
	//	wchar_t wcsbuf[WCHAR_MSG_SIZE];
	//	swprintf(wcsbuf, WCHAR_MSG_SIZE, L"returning 0x%x", shiftedValue & mask);
	//	OutputDebugString(wcsbuf);
	return(shiftedValue & mask);
}
#pragma optimize( "", on )


inline void FPGA_HardwareReg::Write(int Field, UINT64 NewValue)
{
	if (_RdWrDir == READONLY)
		return;

	UINT64 ShadowValue = _VALUE; // current Register contents
	// After processing, Write the register per byte len definition
	// in case we're writing entire register (Field is -1), value is aligned by defintion
	if (Field != -1) // only the bit field
	{
		UINT64 mask = 0xFFFFffffFFFFffff;
		UINT64 msb_mask = 0xffffFFFFffffFFFF;
		// shift value to correct position in bit field
		NewValue <<= BitField[Field]->_bitNumStart;
		// create mask for the defined bit field (ignore bits outside field)
		mask <<= BitField[Field]->_bitNumStart;
		msb_mask >>= 63 - BitField[Field]->_bitNumEnd;
		mask &= msb_mask;
		NewValue &= mask;        // limit New value to field definition
		mask ^= 0xFFFFffffFFFFffff;
		ShadowValue &= mask;           // clear previous bit field
		ShadowValue |= NewValue; // set new value
		_VALUE = ShadowValue;
	}
	else // the whole register
	{
		_VALUE = NewValue; //	replace ENTIRE register
	}
	// FINALLY call CThordaq function to write register len according to definition
	// 
	return;
}

// These functions are necessary because we can't access "DoMem" inside FPGA_HardwareReg class
inline UINT64 CdFLIM_4002::FPGAregisterReadDoMem(int Record)
{
	THORDAQ_STATUS status;
	STAT_STRUCT DoMemStatus;

	status = DoMem(READ_FROM_CARD,
		_FPGAregister[Record]->_BAR,
		(PUINT8)&_FPGAregister[Record]->_VALUE, 0,
		_FPGAregister[Record]->_BARoffset,
		_FPGAregister[Record]->_size, // # of bytes in register
		&DoMemStatus);
	return _FPGAregister[Record]->_VALUE;
}

inline void CdFLIM_4002::FPGAregisterWriteDoMem(int Record)
{
	THORDAQ_STATUS status;
	STAT_STRUCT DoMemStatus;

	status = DoMem(!(READ_FROM_CARD),
		_FPGAregister[Record]->_BAR,
		(PUINT8)&_FPGAregister[Record]->_VALUE, 0,
		_FPGAregister[Record]->_BARoffset,
		_FPGAregister[Record]->_size, // # of bytes in register
		&DoMemStatus);
}

// utility function to find Register/Field based on Name
// to avoid partial field name match (e.g., "DBB1_IOCIRCUIT" matching "DBB1_IOCIRCUIT_8"),
// verify passed 
bool CdFLIM_4002::SearchFPGAregFieldIndices(const char* name, int nameSize, int* Record, int* Field)
{
	int r, f;
	for (r = 0; r < TD_LAST_FPGA_REGISTER; r++)
	{
		if (!strncmp(name, _FPGAregister[r]->_RegName, nameSize)) // matching name?
		{
			if (nameSize == strlen(_FPGAregister[r]->_RegName))  // exact char length of name?
			{
				*Record = r;
				*Field = -1; // register, not field
				return true; // found it
			}
		}
		// check any field names of this register
		for (f = 0; f < 64; f++) // at most, 64 bit fields in register
		{
			if (_FPGAregister[r]->BitField[f] == NULL) // non-existent?
				break;
			if (!strncmp(name, _FPGAregister[r]->BitField[f]->_FieldName, nameSize))
			{
				if (nameSize == strlen(_FPGAregister[r]->BitField[f]->_FieldName))  // exact char length of field?
				{
					*Record = r;
					*Field = f;
					return true; // found it at indices *Record, *Field
				}
			}
		}
	}
	*Record = -1; // failed, invalid
	*Field = -1;
	return false;  // not found
}

// returns STATUS_SUCCESSFUL if RegIndex/FldIndex exists
// otherwise STATUS_INVALID_PARAMETER
THORDAQ_STATUS CdFLIM_4002::FPGAregisterQuery(
	int RegIndex,           // 0-based list of WO, RO, and R/W registers
	int FldIndex,           // 0-based enumerated field in register
	char* pName,			// returned name of register/field (if found)
	int pNameSize,
	PREG_FIELD_DESC RegFldDesc // register/field description
)
{
	// check for validity of Register/Field
	if (RegIndex < 0 || RegIndex >= TD_LAST_FPGA_REGISTER)
		return (THORDAQ_STATUS)STATUS_INVALID_PARAMETER;

	// return values for Register or Field...
	RegFldDesc->BAR = _FPGAregister[RegIndex]->_BAR;
	RegFldDesc->BARoffset = _FPGAregister[RegIndex]->_BARoffset;
	RegFldDesc->RegisterByteSize = _FPGAregister[RegIndex]->_size;
	RegFldDesc->bReadOnly = _FPGAregister[RegIndex]->_RdWrDir == READONLY ? TRUE : FALSE;
	RegFldDesc->bWriteOnly = _FPGAregister[RegIndex]->_RdWrDir == WRITEONLY ? TRUE : FALSE;
	// just the REGSITER?
	if (FldIndex < 0) // requesting register
	{
		strncpy_s(pName, pNameSize, &_FPGAregister[RegIndex]->_RegName[0], sizeof(_FPGAregister[RegIndex]->_RegName));
	}
	else
	{
		if (_FPGAregister[RegIndex]->BitField[FldIndex] == NULL) // non-existent?
			return (THORDAQ_STATUS)STATUS_INVALID_PARAMETER;

		// field exists...
		strncpy_s(pName, pNameSize, &_FPGAregister[RegIndex]->BitField[FldIndex]->_FieldName[0],
			sizeof(_FPGAregister[RegIndex]->BitField[FldIndex]->_FieldName));

		RegFldDesc->BitFieldOffset = _FPGAregister[RegIndex]->BitField[FldIndex]->_bitNumStart;
		RegFldDesc->BitFieldLen = _FPGAregister[RegIndex]->BitField[FldIndex]->_bitFldLen;
	}
	return STATUS_SUCCESSFUL;
}


// This function insures that all WRITE only registers have readable copy of last written value
// writes "shadow" copy to our internal structure, then writes to FPGA hardware
void CdFLIM_4002::FPGAregisterShadowFPGAwrite(int Record, int Field, UINT64 Value)
{
	_FPGAregister[Record]->Write(Field, Value);  // the "shadow" copy
	FPGAregisterWriteDoMem(Record);              // actual write to FPGA hardware
}

THORDAQ_STATUS CdFLIM_4002::FPGAregisterWrite(
	const char* pName,		// name of register/field (if found)
	int nameSize,
	UINT64  Value           // register/field value to write (from 1 to 64 bits)
)
{
	int Rcd = 0;
	int Field = -1;
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	// find the register/field indices
	bool bFound = SearchFPGAregFieldIndices(pName, nameSize, &Rcd, &Field);
	if (bFound != true)
		return STATUS_PARAMETER_SETTINGS_ERROR; // name not found

	FPGAregisterShadowFPGAwrite(Rcd, Field, Value);
	return status;
}

// overload FPGAregisterWrite with Record/Field for performance sensitive invocation
inline THORDAQ_STATUS CdFLIM_4002::FPGAregisterWrite(
	int Record, 			// zero based register array index
	UINT64 Value           // register/field value - either from FPGA hardware or shadow copy
)
{
	// ALWAYS write to the hardware (unless READONLY register)
	if (_FPGAregister[Record]->_RdWrDir == READONLY) // can't write to this - invalid "Record"
	{
		return STATUS_PARAMETER_SETTINGS_ERROR;
	}
	_FPGAregister[Record]->_VALUE = Value;
	FPGAregisterWriteDoMem(Record); // writes _VALUE to FPGA hardware

	return STATUS_SUCCESSFUL;
}



/*inline*/ THORDAQ_STATUS CdFLIM_4002::FPGAregisterRead(
	int Record, 			// zero based register array index
	int Field,				// zero based field index within register (or -1 for no field)
	UINT64* Value           // register/field value - either from FPGA hardware or shadow copy
)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	if (_FPGAregister[Record]->_RdWrDir != WRITEONLY) // read from HARDWARE register, not shadow copy
	{
		_FPGAregister[Record]->_VALUE = FPGAregisterReadDoMem(Record);
	}

	*Value = _FPGAregister[Record]->Read(Field);  // read our shadow copy (which may have come from FPGA hardware)
	return status;
}


THORDAQ_STATUS CdFLIM_4002::FPGAregisterRead(
	const char* pName,			// name of register/field (if found)
	int nameSize,
	UINT64* Value           // "shadow" DLL register copy value (REG or FIELD)
)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	int Record, Field;
	// find the register/field indices
	bool bFound = SearchFPGAregFieldIndices(pName, nameSize, &Record, &Field);
	if (bFound != true)
		return STATUS_PARAMETER_SETTINGS_ERROR; // name not found

	if (_FPGAregister[Record]->_RdWrDir != WRITEONLY) // read from HARDWARE register, not shadow copy
	{
		_FPGAregister[Record]->_VALUE = FPGAregisterReadDoMem(Record);
	}


	*Value = _FPGAregister[Record]->Read(Field);
	return status;
}


// ref: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/rand?view=msvc-170
BYTE RangedRand(int range_min, int range_max)
{
	// Generate random numbers in the interval [range_min, range_max], inclusive.
	int val = rand();
	return((BYTE)val);
}
#define MEMTEST_BLOCK_SIZE 0x2000000  // 32MB
#define MEMTEST_ERROR_STRING_LEN 200
THORDAQ_STATUS CdFLIM_4002::APIMemTest(char* RetString) // return SUCCESS, or if ERROR a string describing problem
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	bool bAddrBit30Operative = true;
	// start with a check of address bits to make sure Bit30 of DDR address range is functional
	// (this was broken from the ThorDAQ beginning until Jan 2024)
	BYTE* HostPtr = (BYTE*)malloc(MEMTEST_BLOCK_SIZE);
	BYTE* ReadBackHostPtr = (BYTE*)malloc(MEMTEST_BLOCK_SIZE);
	UINT32 ReadBackByteLen = MEMTEST_BLOCK_SIZE;
	int BytePosition = 0;
	if (HostPtr == NULL || ReadBackHostPtr == NULL)
	{
		sprintf_s(RetString, MEMTEST_ERROR_STRING_LEN, "malloc of %d bytes failed ", MEMTEST_BLOCK_SIZE);
		return THORDAQ_STATUS::STATUS_WRITE_BUFFER_ERROR;
	}
	// DDR address bit30 test - write block that spans the address bit30 start (0x4000.0000)
	// if the bit is inoperative, the memory actually gets written to 0x0000.00000, because
	// memory above 3fff.FFFF does not exit
	for (int i = 0; i < MEMTEST_BLOCK_SIZE; i++)
	{
		if( HostPtr != nullptr)  *(HostPtr+i) = RangedRand(0, 255);
	}
	// now write this range of random bytes to Bit30 boundary, and read back
	status = WriteDDR3(HostPtr, 0x40000000, MEMTEST_BLOCK_SIZE);
	// did the Win32 functions themselves work correctly?
	if (status == STATUS_SUCCESSFUL)
	{
		// now read back the bytes
		status = ReadDDR3(0x40000000, ReadBackHostPtr, &ReadBackByteLen);
		if(status == STATUS_SUCCESSFUL && ReadBackByteLen == MEMTEST_BLOCK_SIZE)
		{
			
			// compare the bytes at their EXPECTED addresses
			for (BytePosition = 0; BytePosition < MEMTEST_BLOCK_SIZE; BytePosition++)
			{
				if (HostPtr[BytePosition] != ReadBackHostPtr[BytePosition])
				{
					// byte error!!
					break;
				}
			} // loop will progress to BytePosition == MEMTEST_BLOCK_SIZE on success
			if (BytePosition == MEMTEST_BLOCK_SIZE)
			{
				// finally, read the starting address block (starts at DDR3 0x0) to make
				// sure we didn't copy the random bytes from 0x4000.0000 and above there as well
				// since these are random bytes ALL the bytes have to match before we
				// declare the memory at high address actually goes to low address
				ReadBackByteLen = MEMTEST_BLOCK_SIZE;

				status = ReadDDR3(0x0, ReadBackHostPtr, &ReadBackByteLen);
				for (BytePosition = 0; BytePosition < MEMTEST_BLOCK_SIZE; BytePosition++)
				{
					if (HostPtr[BytePosition] != ReadBackHostPtr[BytePosition])
					{
						break;
					}
				} // loop will progress to MEMTEST_BLOCK_SIZE if all the random bytes match
				if (BytePosition == MEMTEST_BLOCK_SIZE) 
				{
					bAddrBit30Operative = false;
				}
			}
		}
	}
	if(status != STATUS_SUCCESSFUL && bAddrBit30Operative == true)
	{
		sprintf_s(RetString, MEMTEST_ERROR_STRING_LEN, "Write or Read DDR3 API failed, status 0x%x: WroteLen %d, ReadLen %d, BytePos %d(%x), Wrote 0x%x, Read 0x%x", 
			status, MEMTEST_BLOCK_SIZE, ReadBackByteLen, BytePosition, BytePosition, HostPtr[BytePosition], ReadBackHostPtr[BytePosition]);
	}
	else if(bAddrBit30Operative == false)
	{
		sprintf_s(RetString, MEMTEST_ERROR_STRING_LEN, "Address @Bit30 fails: random data written to 0x4000.0000 written to 0000.0000: WroteLen %d", MEMTEST_BLOCK_SIZE);
		status = THORDAQ_STATUS::STATUS_READ_BUFFER_ERROR;
	}

	free(HostPtr);
	free(ReadBackHostPtr);
	return status;
}
THORDAQ_STATUS CdFLIM_4002::ReadDDR3(UINT64 DDR3SourceAddress, PUINT8 HostDestinationAddress, PUINT32 ByteLength)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	UINT64 UserControl = 0;
	PUINT8 NotUsedFIFObuf = NULL;
	PUINT32 NotUsedBufferSize = 0;
	PUINT32 NotUsedFIFOMaxPacketSize = 0;
	INT32 NumDMAdescriptors = 8192;
	PUINT8 WinOSVirtAddress = HostDestinationAddress;
	INT32 BytesRemaining = *ByteLength;
//	UINT64 OffsetBoundary = 0; // bump 64MB at a time (when needed)
	INT32 BytesThisRead;
	const INT32 MAXTRANSFERLEN = 0x04000000;
	
	// NWA DMA will NOT read (or write) more than 0x400.0000 (64MB) of data in a single call
	// break up large read commands
	// NOTE! Original NWL kernel is BROKEN for read/write over 64MB - symptom, if you write 0x400.0100 bytes,
	// the Win10 DMA framework runs two transfers, one of 64MB, then second of 512 bytes, but
	// NWL returns "512" for "BytesRead"; moreover, the READ wraps around to beginning of virt.buffer and DDR3 beginning
	// That's why we ignore the return values, and break reads in 64MB segments/remainders

	THORDAQ_STATUS PktSetupStatus;
	PktSetupStatus = SetupPacket(32, NotUsedFIFObuf, NotUsedBufferSize, NotUsedFIFOMaxPacketSize, PACKET_MODE_ADDRESSABLE, 8192);
	if (PktSetupStatus != STATUS_SUCCESSFUL)
		DebugBreak();
	*ByteLength = 0; // reset return value
	do
	{
		BytesThisRead = (BytesRemaining > MAXTRANSFERLEN) ? MAXTRANSFERLEN : BytesRemaining;
		UINT32 ScratchByteCount = BytesThisRead; // returned value can be WRONG

		status = PacketReadEx(32, &UserControl, DDR3SourceAddress, 0, WinOSVirtAddress, (PUINT32)&ScratchByteCount);
		if (status != STATUS_SUCCESSFUL)
			DebugBreak();

		*ByteLength += BytesThisRead; // return to caller our read progress (assume "success" status means all bytes read)
		DDR3SourceAddress += MAXTRANSFERLEN;
		WinOSVirtAddress += MAXTRANSFERLEN;
		BytesRemaining -= BytesThisRead;

	} while (status == STATUS_SUCCESSFUL && BytesRemaining > 0);
	THORDAQ_STATUS RelStatus = ReleasePacketBuffers(32); // releases Descriptor resources
	if (RelStatus != STATUS_SUCCESSFUL)
		DebugBreak();

	return status;
}
// ThorDAQ's DDR3 memory can ONLY be accessed by NWL DMA engine
// ThorDAQ FPGA implements only two of the possible 64 DMA engines:
//   READ engine at index (a.k.a. offset) 0
//   WRITE engine at index 32
// There are numerous methods of DMA - streaming, FIFO, block, addressable packet, etc.
// ThorDAQ mostly uses high performance PACKET_MODE_ADDRESSABLE (or "Addressable Packet")
//
// According to Northwest Logic "DMA Driver, Windows & Linux User Guide", ver 2.31 (2013),
// page 36, the "general Addressable Packet read/write flow" with latest API calls is:
// SetupPacketMode()    [SetupPacket()]
// PacketReadEx() (Card to System / "C2S") or PacketWriteEx() ("S2C")
// ShutdownPacketMode() [ReleasePacketBuffers()]
//
// These functions provide general DDR3 read/write functionality, wrapping the 
// descriptor setup and release for the transaction.
// Note in addressable mode, the SetupPacketMode() does not use
// a host virtual buffer baseAddress argument -- this means the MDL (kernel
// memory descriptor list) is mapped in the Read/Write call, which means
// failing to call ReleasePacketBuffers() after the Read/Write can lead to --
// not necessarily coincidental in time -- a kernel panic (BSOD).
THORDAQ_STATUS CdFLIM_4002::WriteDDR3(PUINT8 HostSourceAddress, UINT64 DDR3DestinationAddress, UINT32 ByteLength)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	UINT64 UserControl = 0;
	PUINT8 NotUsedFIFObuf = NULL;
	PUINT32 NotUsedBufferSize = 0;
	PUINT32 NotUsedFIFOMaxPacketSize = 0;
	INT32 NumDMAdescriptors = 8192;

	//	status = SetupPacket(0, NotUsedFIFObuf, NotUsedBufferSize, NotUsedFIFOMaxPacketSize, PACKET_MODE_ADDRESSABLE, NumDMAdescriptors);
	status = PacketWriteEx(0, UserControl, DDR3DestinationAddress, 0, HostSourceAddress, ByteLength);
	//	THORDAQ_STATUS RelStatus = ReleasePacketBuffers(0); // releases Descriptor resources

	return status;
}








/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM:: WriteReadRegister ( UINT read_write_flag, UINT register_bar_num, ULONGLONG register_card_offset, BYTE* buffer, ULONGLONG offset, ULONGLONG length, PSTAT_STRUCT completed_status)
 *
 * @brief	Set/Get the register value of the device.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param 		  	read_write_flag			Idnetifier for data transfer direction 1==Read,
 * 											0==Write.
 * @param 		  	register_bar_num		Index of Bar configuration of FPGA.
 * @param 		  	register_card_offset	Register address offset to the head of bar.
 * @param [in,out]	buffer					Buffer to write/read.
 * @param 		  	offset					Offset in data buffer to start transfer.
 * @param 		  	length					Byte length of transfer buffer.
 * @param 		  	completed_status		The completed status.
 *
 * @return	STATUS_SUCCESSFUL if Thordaq return. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS CdFLIM_4002::WriteReadRegister ( UINT read_write_flag, UINT register_bar_num, ULONGLONG register_card_offset, BYTE* buffer, ULONGLONG offset,  ULONGLONG length,  PSTAT_STRUCT completed_status)
{
    THORDAQ_STATUS status = STATUS_SUCCESSFUL;
    DO_MEM_STRUCT			write_read_struct;
	OVERLAPPED overlapped;  // OVERLAPPED structure for the operation
    DWORD					bytes_returned = 0;
    DWORD					io_ctl_code;
    DWORD					last_error_status = 0;
	//Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

	// bounds checking
	if ((read_write_flag != READ_FROM_CARD) && (read_write_flag != WRITE_TO_CARD)
		||(register_bar_num > USED_MAX_BARS)
		||(register_card_offset > USED_MAX_CARD_OFFSET))
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

    // fill in the doMem Structure
    write_read_struct.BARnum = register_bar_num;
    write_read_struct.VirtBufAddr		       = offset;
    write_read_struct.BARbyteOffset = register_card_offset;
    write_read_struct.Length               = length;
    // determine the ioctl code
    io_ctl_code = ( read_write_flag == READ_FROM_CARD ) ? DO_MEM_READ_ACCESS_IOCTL : DO_MEM_WRITE_ACCESS_IOCTL;
    // Send write_read_struct to driver io control
    if ( !DeviceIoControl ( gHdlDevice, io_ctl_code, ( LPVOID ) &write_read_struct, sizeof ( DO_MEM_STRUCT ), ( LPVOID ) buffer, ( DWORD ) length, &bytes_returned, &overlapped ) )
    {
        last_error_status = GetLastError();
        if ( last_error_status != ERROR_IO_PENDING )
        {
            completed_status->CompletedByteCount = 0;
#if _DEBUG
            printf ( "DoMem IOCTL call failed. Error = %d\n", last_error_status );
#endif          
			status = STATUS_READWRITE_REGISTER_ERROR;
        } else
        {
            // Wait here (forever) for the Overlapped I/O to complete
            if ( !GetOverlappedResult ( gHdlDevice, &overlapped, &bytes_returned, TRUE ) )
            {
                last_error_status = GetLastError();
#if _DEBUG
                printf ( "DoMem IOCTL call failed. Error = %d\n", last_error_status );
#endif // _DEBUG
                status = STATUS_READWRITE_REGISTER_ERROR;
            }
        }
    }
    // save the returned size
    completed_status->CompletedByteCount = bytes_returned;
    // check returned structure size
    if ( ( bytes_returned != length ) && ( status == STATUS_SUCCESSFUL ) ) // Got the wrong data
    {
        // ioctl failed
#if _DEBUG
        printf ( "DoMem IOCTL returned invalid size (%d), expected length %lld\n", bytes_returned, length );
#endif // _DEBUG
        status = STATUS_READWRITE_REGISTER_ERROR;
    }
	if (overlapped.hEvent != NULL)
		CloseHandle ( overlapped.hEvent );
    return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM:: SetPacketMode (int EngineNum,bool stream_to_mem_dma_enable, ULONG* , int packet_alloc_mode, int NumberDescriptors)
 *
 * @brief	Sets packet mode.
 *
 * @author	Cge
 * @date	3/20/2017
 *
 * @param 		  	EngineNum			The dma engine offset.
 * @param 		  	stream_to_mem_dma_enable	True if stream to memory dma enable.
 * @param [in,out]	parameter3					If non-null, the third parameter.
 * @param 		  	packet_alloc_mode			The packet allocate mode.
 * @param 		  	NumberDescriptors			Number of descriptors.
 *
 * @return	A ThordaqDFLIM::
 **************************************************************************************************/

THORDAQ_STATUS CdFLIM_4002:: SetPacketMode (int EngineNum,bool stream_to_mem_dma_enable, ULONG* , int packet_alloc_mode, int NumberDescriptors)
{
    THORDAQ_STATUS status = STATUS_SUCCESSFUL;
    BUF_ALLOC_STRUCT		buf_alloc;
	OVERLAPPED overlapped;  // OVERLAPPED structure for the operation
    DWORD					bytes_returned = 0;
    DWORD					last_error_status = 0;
    //Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

	// bounds checking
    if ( EngineNum >= gDmaInfo.PacketRecvEngineCount )
    {
		return STATUS_SETUP_PACKET_MODE_INCOMPLETE;
	}

//	buf_alloc.IsS2mmDmaEnabled = stream_to_mem_dma_enable == true? TRUE: FALSE;
	
//	stream_to_mem_dma_enable == FALSE; // REDESIGN - this field not in NWL

	// Set the DMA Engine we want to allocate for
    buf_alloc.EngineNum = gDmaInfo.PacketRecvEngine[EngineNum];
    // Setup in Addressable Packet mode
    buf_alloc.AllocationMode = packet_alloc_mode;
    buf_alloc.Length = 0;
    buf_alloc.MaxPacketSize = 0;
    buf_alloc.NumberDescriptors = NumberDescriptors;

    // Send Setup Packet Mode Addressable IOCTL
    if ( !DeviceIoControl ( gHdlDevice, PACKET_BUF_ALLOC_IOCTL, &buf_alloc, sizeof ( BUF_ALLOC_STRUCT ), NULL, 0, &bytes_returned, &overlapped ) )
    {
        last_error_status = GetLastError();

        if ( last_error_status == ERROR_IO_PENDING )
        {
            // Wait here (forever) for the Overlapped I/O to complete
            if ( !GetOverlappedResult ( gHdlDevice, &overlapped, &bytes_returned, TRUE ) )
            {
                last_error_status = GetLastError();
#if _DEBUG
                printf ( "Addressable Packet Mode setup overlap failed. Error = %d\n", last_error_status );
#endif
				status = STATUS_SETUP_PACKET_MODE_INCOMPLETE;
            }
        }

        else
        {
#if _DEBUG
            printf ( "Addressable Packet mode setup failed. Error = %d\n", last_error_status );
#endif          
			status = STATUS_SETUP_PACKET_MODE_INCOMPLETE;
        }
    }
	if (overlapped.hEvent != NULL)
		CloseHandle ( overlapped.hEvent );

	//status = SetImagingSettings();

    return status;
}



/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM:: GetDMAEngineCap ( ULONG EngineNum, PDMA_CAP_STRUCT dma_capability)
 *
 * @brief	Get DMA Engine Capabilitie.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param	EngineNum	DMA Engine number offset to use.
 * @param	dma_capability   	Returned DMA Engine Capabilitie.
 *
 * @return	STATUS_SUCCESSFUL if Thordaq return. Others if problem happened.
 **************************************************************************************************/
THORDAQ_STATUS CdFLIM_4002:: GetDMAEngineCap ( ULONG EngineNum, PDMA_CAP_STRUCT dma_capability)
{
    THORDAQ_STATUS status = STATUS_SUCCESSFUL;
    OVERLAPPED				overlapped;			// OVERLAPPED structure for the operation
    DWORD					bytes_returned = 0;
    DWORD					last_error_status = 0;
	//Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );
    // Send BLOCK_DIRECT_GET_PERF_IOCTL IOCTL
    if ( DeviceIoControl ( gHdlDevice, GET_DMA_ENGINE_CAP_IOCTL, ( LPVOID ) &EngineNum, sizeof ( ULONG ), ( LPVOID ) dma_capability, sizeof ( DMA_CAP_STRUCT ), &bytes_returned, &overlapped ) == 0 )
    {
        last_error_status = GetLastError();

        if ( last_error_status != ERROR_IO_PENDING )
        {
#if _DEBUG
            printf ( "Getdma_capability IOCTL call failed. Error = %d\n", last_error_status );
#endif
			status = STATUS_GET_BOARD_CONFIG_ERROR;
        } else
        {
            // Wait here (forever) for the Overlapped I/O to complete
            if ( !GetOverlappedResult ( gHdlDevice, &overlapped, &bytes_returned, TRUE ) )
            {
                last_error_status = GetLastError();
#if _DEBUG
                printf ( "Getdma_capability IOCTL call failed. Error = %d\n", last_error_status );
#endif // _DEBUG
                status = STATUS_GET_BOARD_CONFIG_ERROR;
            }
        }
    }
    // check returned structure size
    if ( ( bytes_returned != sizeof ( DMA_CAP_STRUCT ) ) &&
            ( status == STATUS_SUCCESSFUL ) )
    {
        // ioctl failed
#if _DEBUG
        printf ( "Getdma_capability IOCTL returned invalid size (%d)\n", bytes_returned );
#endif // _DEBUG
        status = STATUS_GET_BOARD_CONFIG_ERROR;
    }
	if (overlapped.hEvent != NULL)
		CloseHandle ( overlapped.hEvent );
    return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM::ReleasePacketBuffers ( int EngineNum )
 *
 * @brief	Sends two PACKET_BUF_DEALLOC_IOCTL calls to the driver to teardown the recieve buffer
 * 			and teardown the descriptors for sending packets.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param	EngineNum	DMA Engine number offset to use.
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/
/*
THORDAQ_STATUS CdFLIM_4002::ReleasePacketBuffers ( int EngineNum )
{
    BUF_DEALLOC_STRUCT		buffer_release;
    OVERLAPPED				overlapped;			// OVERLAPPED structure for the operation
    DWORD					bytes_returned = 0;
    DWORD					last_error_status = 0;
    THORDAQ_STATUS			status = STATUS_RELEASE_PACKET_MODE_INCOMPLETE;
	ULONG64					_pRxPacketBufferHandle = 0;      // The Address of DMA Packet Mode Allocated Buffer

    //Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

    if ( EngineNum < gDmaInfo.PacketRecvEngineCount )
    {
        // Set the DMA Engine we want to de-allocate
        buffer_release.EngineNum = gDmaInfo.PacketRecvEngine[EngineNum];
        // Set the allocation mode to what we used above
        buffer_release.Reserved = 0;
        // Return the Buffer Address we recieved from the Allocate call
        buffer_release.RxBufferAddress = _pRxPacketBufferHandle;

        // Send Packet Mode Release
        if ( !DeviceIoControl ( gHdlDevice, PACKET_BUF_RELEASE_IOCTL, &buffer_release, sizeof ( BUF_DEALLOC_STRUCT ), NULL, 0, &bytes_returned,	&overlapped ) )
        {
            last_error_status = GetLastError();

            if ( last_error_status == ERROR_IO_PENDING )
            {
                // Wait here (forever) for the Overlapped I/O to complete
                if ( !GetOverlappedResult ( gHdlDevice, &overlapped, &bytes_returned, TRUE ) )
                {
                    last_error_status = GetLastError();
#if _DEBUG
                    printf ( "GetOverlappedResult Packet Rx Buffer Deallocate failed. Error = %d\n", last_error_status );
#endif          
					status = STATUS_RELEASE_PACKET_MODE_INCOMPLETE;
                }else
                {
                    _pRxPacketBufferHandle = NULL;
                    status = STATUS_SUCCESSFUL;
                }
            } else
            {
                // ioctl failed
#if _DEBUG     
				printf ( "DeviceIoControl Packet Rx buffer DeAllocate failed. Error = %d\n", last_error_status );
#endif          
				status = STATUS_RELEASE_PACKET_MODE_INCOMPLETE;
            }
        } else
        {
            _pRxPacketBufferHandle = NULL;
            status = STATUS_SUCCESSFUL;
        }
    }
    CloseHandle ( overlapped.hEvent );
    return status;
}
*/

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM::PacketRead( ULONG Channel, ULONG* buffer_length, void* Buffer, ULONG Timeout)
 *
 * @brief	Read data from particular channel address predefined in the FPGA.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param 		  	Channel			Channel Index to be read from.
 * @param [in,out]	buffer_length	Size of the Packet Recieve Data Buffer requested (FIFO Mode)
 * @param [in,out]	Buffer			Data buffer (Packet Mode)
 * @param 		  	Timeout			Generate Timeout error when timeout.
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS CdFLIM_4002::PacketReadChannel( ULONG channel, ULONG*  buffer_length, void* buffer, double Timeout_ms)
{
    THORDAQ_STATUS status = STATUS_SUCCESSFUL;
    PACKET_READ_STRUCT		packet_read_config;
    PACKET_RET_READ_STRUCT	packet_read_config_ret;
    DWORD					bytes_returned = 0;
    DWORD					last_error_status = 0;
    int                     EngineNum = 0;
	ULONG32                 bufferSize = 0;
	OVERLAPPED				overlapped;			// OVERLAPPED structure for the operation
	ULONG32                 Timeout =  0xffffffff;
	//sanity check 
	if ( channel >= MAX_CHANNEL_COUNT
		|| Timeout_ms < 0 
		|| gDmaInfo.PacketRecvEngineCount <= 0)
	{
		return STATUS_READ_BUFFER_ERROR;
	}

	//Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

/// REDESIGN ///////////////////
    packet_read_config.EngineNum	  =  gDmaInfo.PacketRecvEngine[EngineNum]; //use the receive engine
    packet_read_config.ModeFlags	  = (ULONG32) C2S_DIRECTION;
    packet_read_config.CardOffset    = (ULONG64)0;
//    packet_read_config.IsMemCheck    = (UCHAR)0;
	packet_read_config.Length = *buffer_length;
//    packet_read_config.Channel       = static_cast<UCHAR> ( channel );
    packet_read_config.BufferAddress = ( ULONG64 ) ( buffer );
	bufferSize				  =	(ULONG32) *buffer_length;
	/// REDESIGN ///////////////////

	packet_read_config_ret.Length = 0;
	packet_read_config_ret.UserStatus = 0x1111111111111111;
	abortPacketRead = false;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	long long time_diff = 0;
	do
	{
		//packet_read_config.Length = bufferSize;
		if ( !DeviceIoControl ( gHdlDevice, PACKET_CHANNEL_READ_IOCTL, &packet_read_config, sizeof ( PACKET_READ_STRUCT ), &packet_read_config_ret, sizeof ( PACKET_RET_READ_STRUCT ), &bytes_returned, &overlapped ) )
		{
			last_error_status = GetLastError();

			while ( (( last_error_status == ERROR_IO_PENDING ) || ( last_error_status == ERROR_IO_INCOMPLETE )) && abortPacketRead == false)
			{
				// Wait here (forever) for the Overlapped I/O to complete
				if ( !GetOverlappedResult ( gHdlDevice, &overlapped, &bytes_returned, FALSE ) )
				{
					last_error_status = GetLastError();

					if ( ( last_error_status == ERROR_IO_PENDING ) || ( last_error_status == ERROR_IO_INCOMPLETE ) )
					{
						if ( !--Timeout )
						{
#if _DEBUG
							printf ( "Packet Read Overlapped Timed Out. Error = %d\n", last_error_status );
#endif
							status = STATUS_READ_BUFFER_ERROR;
							CloseHandle(overlapped.hEvent);
							return status;
						}
					}
					else
					{
#if _DEBUG
						printf ( "Packet Read Overlapped failed. Error = %d\n", last_error_status );
#endif
						break;
					}
				} else
				{
					break;
				}
			}  // while ...
		} // if (!DeviceIoControl ...
		std::chrono::steady_clock::time_point end= std::chrono::steady_clock::now();
		time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
		if (((double)time_diff > Timeout_ms) && (packet_read_config_ret.UserStatus != 0))
		{
			status = STATUS_READ_BUFFER_TIMEOUT_ERROR;
			break;
		}
	} while (packet_read_config_ret.UserStatus != 0 && abortPacketRead == false);
 
    if ( status == STATUS_SUCCESSFUL && bytes_returned != sizeof ( PACKET_RET_READ_STRUCT ) )
    {
		status = STATUS_READ_BUFFER_ERROR;
    }
	if (overlapped.hEvent != NULL)
		CloseHandle(overlapped.hEvent);
    return status;
}

//////////////////////////////////////////////////////////////////////////////////////
// DZimmerman 9-Jul-2020
// UserIntTask() -- Task (thread) to asynchronously handle the ThorDAQ Interrupt (NWL User_Interrupt)
//
// The (undocumented API) NWL logic design for user interrupts has a blocking call UserIRQWait(),
// which completes when the NWL UserInterrupt DPC is called - with DPC queued by ISR
// on User_Interrupt in NWL common status reg.
// The design requires this UserInterrupt - the S2MM DMA DDR3 bank switch, which
// also typically serves as EOF (End of frame) indication - to switch DDR3 bank
// locations for READing DDR3 image data.  We have ONE application virtual memory
// buffer ( _pBuffer[4]) with room for image(s) for all Descriptors (frames) 
// for 4 ADC channels, The DDR3 source baseAddress switches based on which Bank is being read.
// 
// This FPGA interrupt handling task exits gracefully on call to CancelUserIntTask()
// 
THORDAQ_STATUS CdFLIM_4002::APItdCancelUserIntTask()
{
	THORDAQ_STATUS status;

	status = UserIRQCancel();
	return status;
}

inline THORDAQ_STATUS CdFLIM_4002::APItdUserIntTask(USER_IRQ_WAIT_STRUCT* usrIrqWaitStruct)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	wchar_t logMsg[MSG_SIZE];
	StringCbPrintfW(logMsg, MSG_SIZE, L"CdFLIM APItdUserIntTask() enter, sizeof(USER_IRQ_WAIT_STRUCT)=%d", (int)sizeof(USER_IRQ_WAIT_STRUCT));
	CdFLIM_4002::LogMessage(logMsg, VERBOSE_EVENT);
	UINT64 IrqCnt0=0, IrqCnt1=0, IrqCnt2=0, IrqCnt3=0;
	// wait a super long time - effectively forever, but NOT forever so we can call
	// UserIRQCancel() to complete the blocking UserIRQWait() call and exit thread
	usrIrqWaitStruct->dwTimeoutMilliSec = 0x7FFFFFFF;

	do {
		// UserIRQWait() waits forever for INTERRUPT (success status) or TIMEOUT error caused by UserIRQCancel()
		// when acquisition stops
		status = UserIRQWait(usrIrqWaitStruct,
			&IrqCnt0, &IrqCnt1, &IrqCnt2, &IrqCnt3);  // synchronous - blocking; SUCCESSFUL status means IRQ received
		if (status != STATUS_SUCCESSFUL)
		{
			break;   // UserIRQCancel() causes a "not" success status & normal thread exit
		}
	} while (status == STATUS_SUCCESSFUL);  // "SUCCESS" defined as getting an IRQ

	//StringCbPrintfW(logMsg, MSG_SIZE, L"CdFLIM APItdUserIntTask() exit, status 0x%x", status);
	//CThordaq::LogMessage(logMsg, VERBOSE_EVENT);

	return status;
}

static USER_IRQ_WAIT_STRUCT UsrIrqWaitStruct;
THORDAQ_STATUS CdFLIM_4002::APIWaitForUserIRQ(UINT64* CountOfChan0_INTs, UINT64* CountOfChan1_INTs, UINT64* CountOfChan2_INTs, UINT64* CountOfChan3_INTs)
{
	THORDAQ_STATUS status = (THORDAQ_STATUS)STATUS_INVALID_PARAMETER;


	*CountOfChan0_INTs = 1;
	*CountOfChan1_INTs = 2;
	*CountOfChan2_INTs = 3;
	*CountOfChan3_INTs = 4;
	UsrIrqWaitStruct.dwTimeoutMilliSec = 0x7FFFFFFF; // we essentially block forever waiting for FPGA interrupt or IOCTL Cancel call

	do {
		// UserIRQWait() waits forever for INTERRUPT (success status) or TIMEOUT error caused by UserIRQCancel()
		// when acquisition stops
		status = UserIRQWait(&UsrIrqWaitStruct, CountOfChan0_INTs, CountOfChan1_INTs, CountOfChan2_INTs, CountOfChan3_INTs);  // synchronous - blocking; SUCCESSFUL status means IRQ received
		if (status != STATUS_SUCCESSFUL)
		{
			break;   // UserIRQCancel() causes a "not" success status & normal thread exit
		}
	} while (status == STATUS_SUCCESSFUL);  // "SUCCESS" defined as getting an IRQ

	return status;
}
THORDAQ_STATUS CdFLIM_4002::APICancelWaitForUserIRQ()
{
	THORDAQ_STATUS status = (THORDAQ_STATUS)STATUS_INVALID_PARAMETER;

	status = UserIRQCancel();

	return status;
}



/////////////
// DZimmerman, 6-Jul-2020
// 
// Use single PCIe MSI interrupt vector to read all 4 ADC channels from ThorDAQ
// Original design made four separate IOCTL calls to read the 4 channels from DDR3 memory
// In the simplest layout for reading all channels in a bank, we only need starting DDR3
// baseAddress which depends on frame size (width x height x 2) * MAX_CHANNEL_COUNT
// e.g. for 512 x 512 resolution, each DDR3 frame is 0x8.0000 
// so bank0 starts at 0x0 and bank1 starts at (0x8.0000 * 4) == 0x20.0000 (2 MB)

// The starting S2MM DMA DDR3 Bank depends is always 0 (S2MM_SB_BRAM_BANK_SEL_CHANx to 0)
// ThorImageLS has options "Start Capture" and "Live"
// For "Live" (SW_FREE_RUN_MODE) do a "progressive" scan, sync with IRQ to continuously read the 
// DDR3 Bank which is currently being written by FPGA S2MM DMA engine
// If not Live, we are capturing to disk and we MUST synchronize with IRQ to do single read
// of Bank which is NOT being updated by S2MM DMA.
// For "Start Capture" it is IMPERATIVE that FPGA interrupt is running so that we get only complete
// image frames consistent with expected frame rate.
static UINT64 iterationsPerBank; // testing
THORDAQ_STATUS CdFLIM_4002::APIReadFrames(PUINT32 buffer_length, int chan, void* SySHostBuffer[4], double Timeout_ms, ULONG transferFrames, BOOL isLastTransfer, BOOL& isPartialData)
{

	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	UINT64 DDR3startAddr; // add frame & channel offset , then determine & add ping-pong BANK
	INT  msToWaitForBankSwitch = (INT)Timeout_ms;
	_bProgressiveScan = FALSE;  // (TEST with ths OFF for now) set by ThorDAQAPI call
	
	if (_usrIrqWaitStruct.DMA_Bank == 0)
	{
		iterationsPerBank = 0; // debug counter

		_s2mmBankLastRead = 1; // progressive scan - immediately read live bank; capture - wait for 1st bank switch (IRQ)
	}

	if (_bProgressiveScan == TRUE) // set by new ThorDAQAPI call
	{
		BOOL currentBank = BOOL(_usrIrqWaitStruct.DMA_Bank & 0x00000001);

		//if we received an interrupt we can copy a fully acquired frame image once before moving to progressive images again
		// for dFLIM, compute fixed address within current ping-pong bank

		if (currentBank != _s2mmBankLastRead && _usrIrqWaitStruct.DMA_Bank != 0)
		{
			isPartialData = FALSE;
		}
		else
		{
			//			DDR3startAddr = currentBank * (*buffer_length);
			isPartialData = TRUE;
		}
		// READ as fast as possible...
		// setup and teardown the user application image buffer (MDL page lock) on every transfer
		for (int readChan = 0; readChan < 4; readChan++) {
			// ARE WE DOING Progressive scan?
			// 
//			DDR3startAddr = (UINT64)readChan * DDR3_SINGLEIMAGE_CHANNEL_CAP + ((UINT64)readFromBank * DDR3_IMAGE_2ndPINGPONGBUFFstart);
//			status = ReadDDR3(DDR3startAddr, (PUINT8)SySHostBuffer[readChan], (PUINT32)buffer_length);
//			if (status != STATUS_SUCCESSFUL)
//				DebugBreak();
		}
		_s2mmBankLastRead = currentBank;
	}
	else  // wait for FPGA hardware IRQ to establish timing base for acquired images
	{
		if(1)// (transferFrames == gPtrAcqCtrl->gblCtrl.frm_per_txn)
		{
			// READ at rate of the Bank interrupt... do NOT return from this routine
			// until Bank switch detected
			while ((_s2mmBankLastRead != BOOL(_usrIrqWaitStruct.DMA_Bank & 0x00000001)) && (--msToWaitForBankSwitch > 0) && !abortPacketRead)
			{
				Sleep(1); // 1 millisec delay
			};
			// check time-out condition
			if (msToWaitForBankSwitch <= 0)
			{
				// TIMEOUT ERROR
				status = THORDAQ_STATUS::STATUS_READ_BUFFER_TIMEOUT_ERROR;
				return status;
			}

			if (abortPacketRead)
			{
				status = STATUS_ACQUISITION_ABORTED;
				return status;
			}

			// ready to read completed bank
			BOOL  currentBank = BOOL(_usrIrqWaitStruct.DMA_Bank & 0x00000001);

			//data is being written to the currentBank now, we must read from the opposite bank
			BOOL readFromBank = !currentBank;

			//DDR3startAddr = readFromBank * (*buffer_length); // (ThorDAQ)
			// setup and teardown the user application image buffer (MDL page lock) on every transfer
			for (int readChan = 0; readChan < 4; readChan++) {
				DDR3startAddr = (UINT64)readChan * DDR3_SINGLEIMAGE_CHANNEL_CAP + ((UINT64)readFromBank * DDR3_IMAGE_2ndPINGPONGBUFFstart);
				status = ReadDDR3(DDR3startAddr, (PUINT8)SySHostBuffer[readChan], (PUINT32)buffer_length);
				if (status != STATUS_SUCCESSFUL)
					DebugBreak();
			}
			_s2mmBankLastRead = readFromBank;
			isPartialData = FALSE;
		}
		else if (transferFrames < gPtrAcqCtrl->gblCtrl.frm_per_txn && TRUE == isLastTransfer)
		{
			DWORD sleepTime = (DWORD)gPtrAcqCtrl->gblCtrl.frm_per_sec * gPtrAcqCtrl->gblCtrl.frm_per_txn;
			Sleep(sleepTime);
			//data is being written to the currentBank now, we must read from the opposite bank
			BOOL readFromBank = !_s2mmBankLastRead;

			// ASSUMES 1 FRAME PER CHAN!
			for (int readChan = 0; readChan < 4; readChan++) {
				DDR3startAddr = (UINT64)readChan * DDR3_SINGLEIMAGE_CHANNEL_CAP + ((UINT64)readFromBank * DDR3_IMAGE_2ndPINGPONGBUFFstart);
				status = ReadDDR3(DDR3startAddr, (PUINT8)SySHostBuffer[readChan], (PUINT32)buffer_length);
				if (status != STATUS_SUCCESSFUL)
					DebugBreak();
			}
			isPartialData = FALSE;
		}
		else
		{
			return THORDAQ_STATUS::STATUS_READ_BUFFER_ERROR;
		}
	}
	// NOTE!  if we timed out too fast, beware of FPGA/NWA ability to do "late" DMA to this
	// buffer we are about to unlock and cause BSOD if we failed to clean up the timed-out DMA config

	iterationsPerBank++; // test var.
	return status;
}


/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM::AbortPacketRead()
 *
 * @brief	Abort reading.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS CdFLIM_4002::AbortPacketRead()
{
	abortPacketRead = true;
	THORDAQ_STATUS status = GlobalSCANstart(false);  // stop acquisition and UserInt thread
    return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM::PacketReadEx( ULONG64 Address, ULONG* Length, void* Buffer, ULONG Timeout )
 *
 * @brief	Read data from particular data start address.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param 		  	Address	Start address to be read from.
 * @param [in,out]	Length 	Size of the Packet Recieve Data Buffer requested (FIFO Mode)
 * @param [in,out]	Buffer 	Data buffer (Packet Mode)
 * @param 		  	Timeout	Generate Timeout error when timeout.
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS CdFLIM_4002::PacketReadBuffer( ULONG64 Address, ULONG* Length, void* Buffer, ULONG Timeout )
{
	
    THORDAQ_STATUS status = STATUS_SUCCESSFUL;
    PACKET_READ_STRUCT		packet_read_config;
    PACKET_RET_READ_STRUCT	packet_read_config_ret;
    OVERLAPPED				overlapped;			// OVERLAPPED structure for the operation
    DWORD					bytes_returned = 0;
    DWORD					last_error_status = 0;
    int                     EngineNum = 0;
    //Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

    packet_read_config.EngineNum = gDmaInfo.PacketRecvEngine[EngineNum];
    packet_read_config.ModeFlags = C2S_DIRECTION;
    packet_read_config.Length = *Length;
    packet_read_config.CardOffset = Address;
	// REDESIGN
	//packet_read_config.IsMemCheck = 0;
//    packet_read_config.Channel = static_cast<UCHAR> ( 0 );
	// REDESIGN
	packet_read_config.BufferAddress = ( ULONG64 ) ( Buffer );
	abortPacketRead = false;
	if ( !DeviceIoControl ( gHdlDevice, PACKET_BUF_READ_IOCTL, &packet_read_config, sizeof ( PACKET_READ_STRUCT ), &packet_read_config_ret, sizeof ( PACKET_RET_READ_STRUCT ), &bytes_returned, &overlapped ) )
	{
		last_error_status = GetLastError();

		while ( (( last_error_status == ERROR_IO_PENDING ) || ( last_error_status == ERROR_IO_INCOMPLETE )) && abortPacketRead == false)
		{
			// Wait here (forever) for the Overlapped I/O to complete
			if ( !GetOverlappedResult ( gHdlDevice, &overlapped, &bytes_returned, FALSE ) )
			{
				last_error_status = GetLastError();

				if ( ( last_error_status == ERROR_IO_PENDING ) || ( last_error_status == ERROR_IO_INCOMPLETE ) )
				{
					if ( !--Timeout )
					{
#if _DEBUG
						printf ( "Packet Read Overlapped Timed Out. Error = %d\n", last_error_status );
#endif
						status = STATUS_READ_BUFFER_ERROR;
						return status;
					}
				}
				else
				{
#if _DEBUG
					printf ( "Packet Read Overlapped failed. Error = %d\n", last_error_status );
#endif
					break;
				}
			} else
			{
				break;
			}
		}  // while ...
	} // if (!DeviceIoControl ...
 
    if (status != STATUS_SUCCESSFUL || (status == STATUS_SUCCESSFUL && bytes_returned != sizeof ( PACKET_RET_READ_STRUCT )))
    {
		status = STATUS_READ_BUFFER_ERROR;
    }
	if(overlapped.hEvent != NULL)
		CloseHandle(overlapped.hEvent);
    return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM::PacketWriteBuffer( ULONG64 register_card_offset, ULONG Length, UCHAR* Buffer, ULONG Timeout )
 *
 * @brief	Packet write ex.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param 		  	register_card_offset	The card offset.
 * @param 		  	Length	  	The length.
 * @param [in,out]	Buffer	  	If non-null, the buffer.
 * @param 		  	Timeout   	The timeout.
 *
 * @return	A THORDAQ_STATUS.
 **************************************************************************************************/

THORDAQ_STATUS CdFLIM_4002::PacketWriteBuffer(
		ULONG64  register_card_offset,           // Start address to read in the card
		ULONG    buf_len,            // Size of the Packet Write to Data Buffer Count by Byte (FIFO Mode)
		UCHAR*   buf_ptr,            // Data buffer (Packet Mode)
		ULONG    timeout            // Generate Timeout error when timeout
		)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	PACKET_WRITE_STRUCT packet_write_config_ret;
	OVERLAPPED				overlapped;			// OVERLAPPED structure for the operation
    DWORD					bytes_returned = 0;
    DWORD					last_error_status = 0;
    int                     EngineNum = 0;

    //Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );
	//This sets up yje C2S DMA Descriptors and enables the DMA.
	status = SetPacketMode(EngineNum,false, NULL, PACKET_MODE_ADDRESSABLE, 8192);
	if (status != STATUS_SUCCESSFUL)
	{
		return status;
	}
	//
	packet_write_config_ret.CardOffset = register_card_offset;
	packet_write_config_ret.EngineNum = gDmaInfo.PacketSendEngine[EngineNum];
	packet_write_config_ret.Length = buf_len;
	packet_write_config_ret.ModeFlags = 0;
	packet_write_config_ret.UserControl = 0;

	if (!DeviceIoControl(gHdlDevice, PACKET_WRITE_IOCTL, &packet_write_config_ret, sizeof(PACKET_WRITE_STRUCT), (LPVOID) buf_ptr, (DWORD) buf_len, &bytes_returned, &overlapped))
	{
			last_error_status = GetLastError();
			while ((last_error_status == ERROR_IO_PENDING) || 
				(last_error_status == ERROR_IO_INCOMPLETE))
			{
				// Wait here (forever) for the Overlapped I/O to complete
				if (!GetOverlappedResult(gHdlDevice, &overlapped, &bytes_returned, FALSE))
				{
					last_error_status = GetLastError();
					if ((last_error_status == ERROR_IO_PENDING) || (last_error_status == ERROR_IO_INCOMPLETE))
					{
						if (!--timeout)
						{
							status = STATUS_WRITE_BUFFER_ERROR;
							break;
						}
					}
					else
					{
						status = STATUS_WRITE_BUFFER_ERROR;
						break;
					}
				}
				else
				{
					break;
				}
			}
	}  // if (!DeviceIoControl...
	// Make sure we returned something useful
	if (status != STATUS_SUCCESSFUL || (status == STATUS_SUCCESSFUL && bytes_returned != buf_len))
	{
		status = STATUS_WRITE_BUFFER_ERROR;
	}
	if (overlapped.hEvent != NULL)
		CloseHandle(overlapped.hEvent);
    return status;
}


/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS CThordaq::GlobalSCANcontrol()
 *
 * @brief	Combine actions necessary to start or stop scan - especially,
 *          controlling the FPGA interrupt handling task
 *
 * @author	DZimmerman
 * @date	12/11/2020
 *
 * @return	STATUS_SUCCESSFUL, or Thordaq error
 **************************************************************************************************/

THORDAQ_STATUS CdFLIM_4002::GlobalSCANstart(bool Start) // if TRUE, start GlobalSCAN, otherwise STOP
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	wchar_t logMsg[MSG_SIZE];
	UINT64 GIGCR0_STOP_RUNBitValue = 0xFFFF;
	if (gPtrAcqCtrl != nullptr)
	{
		if (Start == true)  // START GlobalSCAN
		{
			abortPacketRead = false; // set "true" by API call for user's exit demand
			// use the SHADOW REGISTER bit to see if we ALREADY set GlobalSCAN as caller requests...
			// This prevents creating multiple threads for IRQ handling, so we can
			// invoke Global start separately or combined with Image DMA and Waveform DMA
			FPGAregisterREAD("GIGCR0_STOP_RUN", &GIGCR0_STOP_RUNBitValue);
			if (GIGCR0_STOP_RUNBitValue == 1)
			{
				return STATUS_SUCCESSFUL;
			}

			status = APIimageAcqConfig(gPtrAcqCtrl);   // setup the "whole enchilada" Legacy kernel was doing; S2MM_CONFIG, etc.

			_usrIrqWaitStruct.boardNum = gBoardIndex;      // routine args:  boardIndex, DMA Bank indicator, int. timeout, etc.
			_usrIrqWaitStruct.DMA_Bank = 0;
			_usrIrqWaitStruct.NWL_Common_DMA_Register_Block = &_NWL_Common_DMA_Register;
			SAFE_DELETE_PTR(_irqThread); // prior thread should have terminated
			_irqThread = new std::thread([this] { this->APItdUserIntTask(&_usrIrqWaitStruct); });
			_irqThread->detach();  // makes WinOS responsible for releasing thread resources on termination.
			SetThreadPriority(_irqThread->native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);
			//give time to the irq thread to start before setting the run bit
			Sleep(1);

			FPGAregisterWRITE("NWL_UserIntEnable", 0x1); // ENable FPGA hardare interrupts
			FPGAregisterWRITE("GIGCR0_LED2", 0x1);      //  Acq. LED ON	
			FPGAregisterWRITE("GIGCR0_STOP_RUN", 0x1);  // start everything			
			gPtrAcqCtrl->gblCtrl.run_stop_mode = PACKET_GEN_RUN; // legacy code concern				
		}
		else  // STOP the GlobalSCAN
		{
			UINT64 NWLreg;
			gPtrAcqCtrl->gblCtrl.run_stop_mode = PACKET_GEN_STOP; // legacy code
			status = FPGAregisterREAD("NWL_DDR3_DMAcore", &NWLreg); // debug/status info
//			status = FPGAregisterWRITE("NWL_DDR3_DMAcore", NWLreg | 0x3F); // ACK all IRQs
//			FPGAregisterWRITE("NWL_UserIntEnable", 0x0); // Disable FPGA hardare interrupts 
			FPGAregisterWRITE("GIGCR0_STOP_RUN", 0x0);   // All FPGA scan related processes disabled 

			// Causes NWL UserInterrupt handling thread to exit (user INTs are disabled)
			StringCbPrintfW(logMsg, MSG_SIZE, L"CdFLIM::GlobalSCANstart() Stop");
			CdFLIM_4002::LogMessage(logMsg, VERBOSE_EVENT);
			status = APItdCancelUserIntTask();  // we don't expect IOCTL to fail, but...
			if (status != STATUS_SUCCESSFUL)
			{
				StringCbPrintfW(logMsg, MSG_SIZE, L"CdFLIM::GlobalSCANstart(false), APItdCancelUserIntTask() failed, status 0x%x", status);
				CdFLIM_4002::LogMessage(logMsg, VERBOSE_EVENT);
			}
			FPGAregisterWRITE("GIGCR0_LED2", 0x0);      // Acq. LED OFF

		}
	}
	return status;
}

// DZimmerman 2-Dec-2019
// Configure the FPGA's S2MM DMA hardware for streaming the ADC samples to 
// DDR3 memory in the form of "images", one image per DMA descriptor.  
// Separately, the NWL DMA from DDR3 to Host memory is also setup
// Write FPGA registers required to setup ThorDAQ's ADC S2MM Descriptor Chain
// These writes should be done when GIGCR0_STOP_RUN bit is 0 AND S2MMDMA_CONFIG and S2MMDMA_RUN_STOP
// are in config settings
// 
	// registers required to configure DMA...
static const char* ConfigRegName[4] = { "S2MM_CONFIG_Chan0", "S2MM_CONFIG_Chan1","S2MM_CONFIG_Chan2", "S2MM_CONFIG_Chan3" };
static const char* RunStopRegName[4] = { "S2MM_RUN_STOP_Chan0", "S2MM_RUN_STOP_Chan1", "S2MM_RUN_STOP_Chan2", "S2MM_RUN_STOP_Chan3" };
static const char* CyclicRegName[4] = { "S2MM_SG_CYCLIC_BD_Chan0", "S2MM_SG_CYCLIC_BD_Chan1", "S2MM_SG_CYCLIC_BD_Chan2", "S2MM_SG_CYCLIC_BD_Chan3" };
static const char* S2MM_DMA_IRQreArmRegName[4] = { "S2MM_IRQ_REARM_Chan0", "S2MM_IRQ_REARM_Chan1", "S2MM_IRQ_REARM_Chan2", "S2MM_IRQ_REARM_Chan3" };
static const char* BankSelRegName[4] = { "S2MM_SB_BRAM_BANK_SEL_Chan0", "S2MM_SB_BRAM_BANK_SEL_Chan1", "S2MM_SB_BRAM_BANK_SEL_Chan2", "S2MM_SB_BRAM_BANK_SEL_Chan3" };
static const char* ChainHeadRegName[4] = { "S2MMDMA_ChainHead_Chan0", "S2MMDMA_ChainHead_Chan1", "S2MMDMA_ChainHead_Chan2", "S2MMDMA_ChainHead_Chan3" };
static const char* ChainTailRegName[4] = { "S2MMDMA_ChainTail_Chan0", "S2MMDMA_ChainTail_Chan1", "S2MMDMA_ChainTail_Chan2", "S2MMDMA_ChainTail_Chan3" };
static const char* IRQthreshldRegName[4] = { "S2MMDMA_IRQthreshld_Chan0", "S2MMDMA_IRQthreshld_Chan1", "S2MMDMA_IRQthreshld_Chan2", "S2MMDMA_IRQthreshld_Chan3" };

// SWUG pg. 9, Max Descriptors in a chain
// array for S2MMconfig below
#define MAX_DESCRIPTORS_IN_S2MM_CHAIN 1024
AXI_DMA_DESCRIPTOR DMA_Desc[MAX_DESCRIPTORS_IN_S2MM_CHAIN][MAX_CHANNEL_COUNT]; // all channels

THORDAQ_STATUS CdFLIM_4002::S2MMconfig(
	PS2MM_CONFIG pS2MMconfig)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	int desc, chan, bank, BARchanOffset;


	// range check
	if ((pS2MMconfig->NumberOfDescriptorsPerBank > MAX_DESCRIPTORS_IN_S2MM_CHAIN)
		||
		pS2MMconfig->NumberOfDescriptorsPerBank < 1) return STATUS_PARAMETER_SETTINGS_ERROR;

	// create the linked list 
	// for each bank
	const UINT8 BYTES_PER_PIXEL = 2;
	UINT32 DDRaddr = pS2MMconfig->DDR3imageStartingAddress;
	UINT32 Stride_BytesPerLine = pS2MMconfig->dataHSize; // product of pixel width * constants, and 8 beats factor
	UINT32 ImageSizeInBytes = pS2MMconfig ->dataHSize * pS2MMconfig->VSize;  // total bytes/line * lines is raw image size (per single channel)

	// dFLIM produces huge data frames for long Pixel Dwells at high resolution
	// it is possible to exceed the 16-bit limit of the "Stride" bytes per line; legacy
	// kernel handles this by dividing Stride by 2  AND multiplying VSize by 2, which
	// changes "aspect ration" (W x H) but keeps the DMA total frame size the same
	// we repeat this method here:
	UINT32 StrideLimitedTo16bits = Stride_BytesPerLine, VSizeAdjustedToStrideLimit = pS2MMconfig->VSize;
	int StrideFitFactor = 2;
	while (StrideLimitedTo16bits > 0xFFFF)
	{
		StrideLimitedTo16bits = Stride_BytesPerLine / StrideFitFactor;
		VSizeAdjustedToStrideLimit = pS2MMconfig->VSize * StrideFitFactor;
		StrideFitFactor *= 2;
	};

	// From SWUG, BAR1 offsets per channel = 0x10000, and additional 0x20 offset for 2nd BANK
	// (channel 0)
	// BARchanOffset -- BAR1 offset to BRAM which holds descriptor list for the specific channel
	// Legacy dFLIM used hard coded boundaries for each channel; replicate that S2MM config here
	//
	// 0x0000.0000  Start of 1st ping-pong buffer, 1st chan (Image memory)
	// 0x0E00.0000   "     "   "     "       "     2nd chan
	// 0x1C00.0000                                 3rd
	// 0x2A00.0000                                 4th
	// 0x3800.0000  Start of 2nd ping-pong buffer, 1st chan
	// 0x4600.0000                                 2nd chan
	// 0x5400.0000
	// 0x6200.0000
	// 0x7000.0000  Start of DAC Waveforms (Analog output channels)
	// 0x8000.0000  END of physical DDR3 memory

	UINT32 ChainHead, ChainTail; // record these for FPGA register write
	for (bank = 0, DDRaddr = 0; bank < S2MM_BANKS; bank++ )
	{
		memset((void*)&DMA_Desc, 0, sizeof(AXI_DMA_DESCRIPTOR) * MAX_DESCRIPTORS_IN_S2MM_CHAIN * 2); // make sure to clear!
		for (chan = 0, BARchanOffset = 0; chan < MAX_CHANNEL_COUNT; chan++, 
			BARchanOffset += 0x10000, 
			DDRaddr = (chan * DDR3_SINGLEIMAGE_CHANNEL_CAP) + (bank* DDR3_IMAGE_2ndPINGPONGBUFFstart))
		{
			// is the channel enabled by mask?
			if ((pS2MMconfig->ChannelMask >> chan & 0x1) != 1) continue;
			// each descriptor defines the same, complete frame image
			// both "banks" have Next descriptor links @ 0, 0x40, 0x80, 0xC0, etc.  -- the difference
			// is descriptors for bank 0 are at BAR1 offset 0x0, 40, 80..., while bank 1 are at 0x20, 60, A0... 

			ChainHead = ChainTail = BARchanOffset + (bank * 0x20); // record for FPGA register setting
			// (LOGIC only checked for 1 frame per channel!)
			for (desc = 0; desc < pS2MMconfig->NumberOfDescriptorsPerBank; desc++, DDRaddr += ImageSizeInBytes) // ImageSizeInBytes is DMA raw image frame length
			{
				DMA_Desc[desc][chan].DDR3Address = DDRaddr; // starting DDR3 addr (for this descriptor/image)
				DMA_Desc[desc][chan].VSIZE = VSizeAdjustedToStrideLimit; // total "lines" and bytes per line (adjusted when necessary)
				DMA_Desc[desc][chan].HSIZE = StrideLimitedTo16bits;
				DMA_Desc[desc][chan].Stride = StrideLimitedTo16bits;  // horizontal line BYTE count
				DMA_Desc[desc][chan].AwCACHE = 0x3; // Xilinx AXI DMA PG021, pg 52, says "default value should be 0011"
				ChainTail += 0x40; // (always could be last Descriptor)
				DMA_Desc[desc][chan].NextDescPtr = ChainTail;
				// check for last Descriptor - loop back to start
				if (desc == (pS2MMconfig->NumberOfDescriptorsPerBank - 1)) // end of chain?
				{
					DMA_Desc[desc][chan].NextDescPtr = ChainHead; // loop the "next" back to start
				}
			} // next descriptor

			// Descriptor linked-list done: Configure DMA and WRITE the descriptor array to FPGA BAR1
			// for this channel in this bank
			status = CdFLIM_4002::WriteS2MMdescChain(chan, bank, ChainHead, 0x510, DMA_Desc, pS2MMconfig);
			// Ping-Pong buffer start Bank
			// NOTE: always tell FPGA to start at Bank0.  The DDR3 starting baseAddress (Bank) for read depends on whether 
			// we want "Live" progressive scan (continuously read DDR3 being written by S2MM DMA) or
			// "Capture" of the S2MM DMA that is complete (not being written by S2MM DMA)
			status = FPGAregisterWrite(BankSelRegName[chan], (int)strlen(BankSelRegName[chan]), 0); // bank to start from
			// Enable S2MM DMA engine -- first  _RUN_STOP to 1, followed by _CONFIG_DMA to 1.
			status = FPGAregisterWrite(RunStopRegName[chan], (int)strlen(RunStopRegName[chan]), 1);
			status = FPGAregisterWrite(ConfigRegName[chan], (int)strlen(ConfigRegName[chan]), 1);
			// in Legacy dFLIM kernel driver the "re-arm" bit was NOT set for Initial IRQ
			status = FPGAregisterWrite(S2MM_DMA_IRQreArmRegName[chan], (int)strlen(S2MM_DMA_IRQreArmRegName[chan]), 1);

		} // next channel

	} // next bank

	// Enable NWL User interrupts...  required for ThorDAQ's S2MM linked-list DMA IRQ generation
	// Use USER_IRQ_WAIT_IOCTL to set the User Int Enable to make sure we're ready to process it
	//	status = FPGAregisterWRITE("NWL_UserIntEnable", 1);
	// Enable NWL Global interrupts - required for host computer DMA to/from DDR3 mem
	status = FPGAregisterWRITE("NWL_GlobalDMAIntEnable", 1);


	pS2MMconfig->DDR3endingAddress = DDRaddr - 1;
	return status;
}
THORDAQ_STATUS CdFLIM_4002::WriteS2MMdescChain(int ADCchannel, int BRAMbank, int ChainHead,
	int ChainTail, AXI_DMA_DESCRIPTOR DMA_Desc[][MAX_CHANNEL_COUNT], PS2MM_CONFIG pS2MMconfig)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	int DescIndx;
	UINT64 BarOffset;
	STAT_STRUCT DoMemStatus;
	memset(&DoMemStatus, 0, sizeof(DoMemStatus));

	// (presumes GIGCR0_STOP_RUN is stopped)
	// First STOP DMA
	status = FPGAregisterWrite(RunStopRegName[ADCchannel], (int)strlen(RunStopRegName[ADCchannel]), 0);
	// Toggle CONFIG
	status = FPGAregisterWrite(ConfigRegName[ADCchannel], (int)strlen(ConfigRegName[ADCchannel]), 1);  // 1 -> 0 transition causes RESET
	status = FPGAregisterWrite(ConfigRegName[ADCchannel], (int)strlen(ConfigRegName[ADCchannel]), 0);
	// Enable "CYCLIC" Mode (re-using same DDR3 mem) - Xilinx PG021, Table 2-15 pg. 24, "Cyclic BD Enable"
	status = FPGAregisterWrite(CyclicRegName[ADCchannel], (int)strlen(CyclicRegName[ADCchannel]), 1);
	status = FPGAregisterWrite(S2MM_DMA_IRQreArmRegName[ADCchannel], (int)strlen(S2MM_DMA_IRQreArmRegName[ADCchannel]), 1);

	// write the descriptor chain (array) for BRAMbank passed argument
	for (DescIndx = 0; DescIndx < pS2MMconfig->NumberOfDescriptorsPerBank; DescIndx++)
	{
		BarOffset = ((UINT64)ADCchannel * 0x10000) + ((UINT64)DescIndx * 0x40) + ((UINT64)BRAMbank * 0x20);
		status = DoMem(!(READ_FROM_CARD), BAR1, (PUINT8)&DMA_Desc[DescIndx][ADCchannel], 0,
			BarOffset, 0x20, &DoMemStatus);
	}
	// By design use BANK0 for the Descriptor Chain HEAD (start) and TAIL addresses
	if (BRAMbank == 0)
	{
		status = FPGAregisterWrite(ChainHeadRegName[ADCchannel], (int)strlen(ChainHeadRegName[ADCchannel]), ChainHead);
		status = FPGAregisterWrite(ChainTailRegName[ADCchannel], (int)strlen(ChainTailRegName[ADCchannel]), ChainTail); // invalid addr (not multiple of 0x40)
		// Write IRQ threshold - which must equal length of descriptor list (SWUG pg. 13)
		status = FPGAregisterWrite(IRQthreshldRegName[ADCchannel], (int)strlen(IRQthreshldRegName[ADCchannel]), pS2MMconfig->NumberOfDescriptorsPerBank);
	}
	return status;
}

// The dFLIM GPO registers were all set in the legacy Kernel driver; copy their settings
// through shadow regs
static const char* GPOReg2ShadowReg[4] = { "GPOReg2_Ch0", "GPOReg2_Ch1","GPOReg2_Ch2", "GPOReg2_Ch3" };
static const char* GPOReg3ShadowReg[4] = { "GPOReg3_Ch0", "GPOReg3_Ch1","GPOReg3_Ch2", "GPOReg3_Ch3" };
static const char* GPOReg4ShadowReg[4] = { "GPOReg4_Ch0", "GPOReg4_Ch1","GPOReg4_Ch2", "GPOReg4_Ch3" };
static const char* GPOReg5_1ShadowReg[4] = { "GPOReg5_1_Ch0", "GPOReg5_1_Ch1","GPOReg5_1_Ch2", "GPOReg5_1_Ch3" };
static const char* GPOReg5_2ShadowReg[4] = { "GPOReg5_2_Ch0", "GPOReg5_2_Ch1","GPOReg5_2_Ch2", "GPOReg5_2_Ch3" };
static const char* GPOReg6ShadowReg[4] = { "GPOReg6_Ch0", "GPOReg6_Ch1","GPOReg6_Ch2", "GPOReg6_Ch3" };
static const char* GPOReg7ShadowReg[4] = { "GPOReg7_Ch0", "GPOReg7_Ch1","GPOReg7_Ch2", "GPOReg7_Ch3" };


THORDAQ_STATUS CdFLIM_4002::SetdFLIM_GPOregs(int Chan, ULONG32 GPOReg2, ULONG64 GPOReg3, ULONG32 GPOReg4,
	ULONG32 GPOReg5_1, ULONG32 GPOReg5_2, ULONG64 GPOReg6, ULONG64 GPOReg7)
{
	THORDAQ_STATUS RetStatus;
	UINT32 status;

	status = FPGAregisterWrite(GPOReg2ShadowReg[Chan], (int)strlen(GPOReg2ShadowReg[Chan]), (UINT64)GPOReg2);
	status |= FPGAregisterWrite(GPOReg3ShadowReg[Chan], (int)strlen(GPOReg3ShadowReg[Chan]), GPOReg3);
	status |= FPGAregisterWrite(GPOReg4ShadowReg[Chan], (int)strlen(GPOReg4ShadowReg[Chan]), (UINT64)GPOReg4);
	status |= FPGAregisterWrite(GPOReg5_1ShadowReg[Chan], (int)strlen(GPOReg5_1ShadowReg[Chan]), (UINT64)GPOReg5_1);
	status |= FPGAregisterWrite(GPOReg5_2ShadowReg[Chan], (int)strlen(GPOReg5_2ShadowReg[Chan]), (UINT64)GPOReg5_2);
	status |= FPGAregisterWrite(GPOReg6ShadowReg[Chan], (int)strlen(GPOReg6ShadowReg[Chan]), GPOReg6);
	status |= FPGAregisterWrite(GPOReg7ShadowReg[Chan], (int)strlen(GPOReg7ShadowReg[Chan]), GPOReg7);

	RetStatus = (THORDAQ_STATUS)status;
	return RetStatus;
}


// Replace Carl's whole enchilada "SetImagingConfiguration()" kernel IMG_ACQ_CONF_IOCTL call,
// replacing his FPGA register writes with new API shadow register writes
// NOTE!  The NWL kernel driver makes some FPGA writes to the DMA core control, which is
// inherited by the ThorDAQ DLL design.  Intention is to leave these unchanged, and move
// all other FPGA register read/write in the C++ ThorDAQ DLL
// This function follows IOCTL kernel function "NTSTATUS ImageAcquisitionConfig()"
// NOTE!  Indentically named STRUCT types have different names of fields
// between user and kernel STRUCT definitions, e.g. 
// GLOBAL_IMG_GEN_CTRL_STRUCT   User              Kernel
//                              img_scan_mode     imgSyncCtrl
// SCAN_SUBSYS_STRUCT           sync_ctrl         syncCtrl
//                              frm_cnt           frameCnt
//                              pll_sync_offset   syncOffset  
//
THORDAQ_STATUS CdFLIM_4002::APIimageAcqConfig(DATA_ACQ_CTRL_STRUCT* PtrAcqCtrl)  // PtrAcqCtrl is "the whole enchilada"
{
	//TODO: Add missing registers
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	// stop all FPGA DMA (was done in NWL API/kernel)
	status = GlobalSCANstart(false);  // i.e. includes "GIGCR0_STOP_RUN" bit
	status = ResetBackEndAcq();  // added by gy

	UINT64 NWLreg;
	status = FPGAregisterREAD("NWL_DDR3_DMAcore", &NWLreg);
	status = FPGAregisterWRITE("NWL_DDR3_DMAcore", NWLreg | 0x3F);
	
	// clear S2mm Host Interrupts
	status = FPGAregisterWRITE("S2MMDMA_ControlReg1", 0x50);
	status = FPGAregisterWRITE("S2MMDMA_ControlReg2", 0x50);
	status = FPGAregisterWRITE("S2MMDMA_ControlReg3", 0x50);
	status = FPGAregisterWRITE("S2MMDMA_ControlReg4", 0x50);
	// clear Global Ints

	// Image config 
	status = FPGAregisterWRITE("GlobalImageSyncControlReg", PtrAcqCtrl->gblCtrl.img_scan_mode); // BAR3 0x008 (0x2 = SCAN_DIR forward)
	status = FPGAregisterWRITE("GlobalImageHSIZE", PtrAcqCtrl->gblCtrl.hor_pix_num - 1); // 0x010
	status = FPGAregisterWRITE("GlobalImageVSIZE", PtrAcqCtrl->gblCtrl.vrt_pix_num - 1); // 0x018

//	USHORT acqHSize = PtrAcqCtrl->gblCtrl.hor_pix_num * static_cast<UINT8>(PtrAcqCtrl->gblCtrl.numPlanes) - 1;
//	status = FPGAregisterWRITE("GlobalImageAcqHSIZE", acqHSize); // 0x012

	// Scanning Control
	status = FPGAregisterWRITE("ScanningSyncControlReg", PtrAcqCtrl->scan.sync_ctrl);  // 0x140
	status = FPGAregisterWRITE("ScanningFrameCount", PtrAcqCtrl->scan.frm_cnt);            // 0x142
	status = FPGAregisterWRITE("ADPLL_SyncDelay", PtrAcqCtrl->scan.pll_sync_offset);          // 0x150


	status = FPGAregisterWRITE("ScanningIntraFrameDelay", PtrAcqCtrl->scan.galvo_intra_frame_delay);  // 0x178
//	status = FPGAregisterWRITE("ScanningPreSOFDelay", PtrAcqCtrl->scan.galvo_pre_SOF_delay);	// 0x17B (does this exist in dFLIM?
	status = FPGAregisterWRITE("ScanningGalvoPixelDelay", PtrAcqCtrl->scan.galvo_pixel_delay);        // 0x168
	status = FPGAregisterWRITE("ScanningGalvoPixelDwell", PtrAcqCtrl->scan.galvo_pixel_dwell);        // 0x160
	status = FPGAregisterWRITE("ScanningIntraLineDelay", PtrAcqCtrl->scan.galvo_intra_line_delay);     // 0x170

	status = FPGAregisterWRITE("SamplingClockControlReg", (UINT64)PtrAcqCtrl->samplingClock.CtrlReg); // 0x180
	status = FPGAregisterWRITE("SamplingClockPhaseOffset", (UINT64)PtrAcqCtrl->samplingClock.phase_offset);// 0x188
	status = FPGAregisterWRITE("SamplingClockPhaseStep", (UINT64)PtrAcqCtrl->samplingClock.phase_step);// 0x190
	status = FPGAregisterWRITE("SamplingClockPhaseLimit", (UINT64)PtrAcqCtrl->samplingClock.phase_limit);// 0x198

		// use Shadow Register writes to FPGA
	status = FPGAregisterWRITE("GALVOWaveGenControlReg", PtrAcqCtrl->galvoCtrl.ctrlReg); // 0x240
	status = FPGAregisterWRITE("DACAmFilterWindow", PtrAcqCtrl->galvoCtrl.dacAmFilterWindow); // 0x2B8, Eof_dly_sample in SWUG

	status = FPGAregisterWRITE("DAC_StepSize_Chan0", 1); // 0x262
	status = FPGAregisterWRITE("DAC_StepSize_Chan1", 1);
	status = FPGAregisterWRITE("DAC_StepSize_Chan2", 1);
	for (int index = 0; index < 3; index++) {
//		status = FPGAregisterWrite(DACupdateRates[index], (int)strlen("DAC_UpdateRate_ChanN_NN"), PtrAcqCtrl->galvoCtrl.dacUpdateRate[index]); //0x262, 264, 266
		UINT64 offset16bitDACcntsValue = PtrAcqCtrl->galvoCtrl.dacOffset[0] >> (index * 16);
//		status = FPGAregisterWrite(DACoffset[index], (int)strlen("DAC_Offset_ChanN"), offset16bitDACcntsValue);  // 0x280, 288, 290
	}
	status = FPGAregisterWRITE("DOUpdateRate", (UINT64)PtrAcqCtrl->galvoCtrl.doUpdateRate);
	
	// set the FPGA MUXes for Breakout Boxes (hardcoded in dFLIM)
	status = SetDACChannelMapping();  // Legacy
	status = SetADCChannelMapping();
	status = FPGAregisterWRITE("GlobalABBXmuxReg", (UINT64)0x0000BA9876543210);   // 0x298
	status = FPGAregisterWRITE("GlobalDBB1muxReg", (ULONG64)0x01080007090a0304);  // 0x020


//	status = FPGAregisterWRITE("GlobalDBB1muxReg", gPtrAcqCtrl->gblCtrl.gpio_cfg); // no DBB1 yet in dFLIM

	// Write the dFLIM specific GPO register set (total of 4 channels)
	for (int chan = 0; chan < 4; chan++)
	{
		UINT32 GPOreg2, GPOreg4, GPOreg5_1, GPOreg5_2;
		UINT64 GPOreg3, GPOreg6, GPOreg7;
		
		GPOreg2 = (PtrAcqCtrl->gblCtrl.dataHSize / 8) | (PtrAcqCtrl->gblCtrl.linesPerStripe << 16);
		GPOreg3 = 0;
		GPOreg4 = 0;
		GPOreg5_1 = 0x000d0016;  // hardcode copy from legacy kernel driver
		GPOreg5_2 = 0x0088003c;
		GPOreg7 = 0x00000080;

		if (PtrAcqCtrl->gblCtrl.acquisitionMode == 0) // dFLIM hardware acq or Diags "mode"
		{
			GPOreg6 = (PtrAcqCtrl->gblCtrl.acquisitionMode == 0) ? 0 : 1; // "dFLIM mode" = 0, diagnostics = 1
		}
		else // diagnostic mode
		{
			GPOreg6 = 0x43 | (((PtrAcqCtrl->gblCtrl.dataHSize / 8) - 10) << 16);
		}
		status = SetdFLIM_GPOregs(chan, GPOreg2, GPOreg3, GPOreg4, GPOreg5_1, GPOreg5_2, GPOreg6, GPOreg7);
	}

	S2MM_CONFIG s2mmConfig;

	s2mmConfig.ChannelMask = PtrAcqCtrl->gblCtrl.ImagingChan; // which channels does the caller want enabled?

	s2mmConfig.DDR3imageStartingAddress = 0x0; // ThorDAQ's DDR3 mem baseAddress as seen by NWL DMA
	s2mmConfig.HSize = PtrAcqCtrl->gblCtrl.hor_pix_num * PtrAcqCtrl->gblCtrl.numPlanes;
	s2mmConfig.dataHSize = PtrAcqCtrl->gblCtrl.dataHSize; // specific to dFLIM
	s2mmConfig.VSize = PtrAcqCtrl->gblCtrl.vrt_pix_num;
	s2mmConfig.NumberOfDescriptorsPerBank = PtrAcqCtrl->gblCtrl.frm_per_txn;

	status = S2MMconfig(&s2mmConfig);  // configure the S2MM image DMA "Stream" to "Mapped [DDR3] Memory"

	return status;
}


/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM::SetImagingConfiguration( IMAGING_CONFIGURATION_STRUCT imaging_config)
 *
 * @brief	Set up data imaging configuration.
 *
 * @author	Cge
 * @date	3/17/2017
 * 
 * DZimmerman, 18-Sep-2023, replace all kernel driver register writes with Shadow Reg writes
 *
 * @param	imaging_config	Image configuration stuct.
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS CdFLIM_4002::SetImagingConfiguration( dFLIM_IMAGING_CONFIGURATION_STRUCT	imaging_config)
{
	
	DWORD						bytes_returned = 0;
	DWORD						last_error_status = 0;
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	// These are mostly the legacy kernel copies that were passed in huge IOCTL that doesn't exist in NWL kernel
	memset(&gPtrAcqCtrl->gblCtrl, 0, sizeof(gPtrAcqCtrl->gblCtrl));
	memset(&gPtrAcqCtrl->scan, 0, sizeof(gPtrAcqCtrl->scan));
	memset(&gPtrAcqCtrl->samplingClock, 0, sizeof(gPtrAcqCtrl->samplingClock));
	memset(&gPtrAcqCtrl->streamProcessing, 0, sizeof(gPtrAcqCtrl->streamProcessing));
	memset(&gPtrAcqCtrl->adcInterface, 0, sizeof(gPtrAcqCtrl->adcInterface));
	memset(&gPtrAcqCtrl->galvoCtrl, 0, sizeof(gPtrAcqCtrl->galvoCtrl));
	memset(&gPtrAcqCtrl->i2cCtrl, 0, sizeof(gPtrAcqCtrl->i2cCtrl));

// The following routines (e.g. SetGlobalSettings) copies values from upper level DLL (i.e. TILS)
// into gPtrAcqCtrl
	if(SetGlobalSettings(imaging_config) == FALSE)
	{
		return STATUS_PARAMETER_SETTINGS_ERROR;
	}

	if (imaging_config.imageCtrl.defaultMode == 0)
	{
		//Setup Scan Subsystem Configurations
		if(SetScanSettings(imaging_config) == FALSE)
		{
			return STATUS_PARAMETER_SETTINGS_ERROR;
		}

		//Setup Analogy out settings
		if (SetDACSettings(imaging_config) == FALSE)
		{
			return STATUS_PARAMETER_SETTINGS_ERROR;
		}

		//Setup Coherent Sampling Configurations
		if(SetCoherentSampleingSettings(imaging_config) == FALSE)
		{
			return STATUS_PARAMETER_SETTINGS_ERROR;
		}
		
		//Setup Stream Processing
		if (SetStreamProcessingSettings(imaging_config) == FALSE)
		{
			return STATUS_PARAMETER_SETTINGS_ERROR;
		}

		// Load the DMA Descriptors
		if (LoadDACDescriptors(imaging_config.dacCtrl) != STATUS_SUCCESSFUL)
		{
			return STATUS_PARAMETER_SETTINGS_ERROR;
		}
	}else
	{
		memset(&gPtrAcqCtrl->galvoCtrl, 0, sizeof(gPtrAcqCtrl->galvoCtrl));
	}
	// Take the entire collection of TILS params copied into gPtrAcqCtrl for ShadowReg write
	status = APIimageAcqConfig(gPtrAcqCtrl);   // setup S2MM_CONFIG, etc.

/*
	//Initiate overlapped structure
	OVERLAPPED					overlapped;			// OVERLAPPED structure for the operation
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );
	if (!DeviceIoControl(gHdlDevice, IMG_ACQ_CONF_IOCTL, (LPVOID)gPtrAcqCtrl, sizeof(DATA_ACQ_CTRL_STRUCT), NULL, 0, &bytes_returned, &overlapped))
	{
		last_error_status = GetLastError();
		if (last_error_status == ERROR_IO_PENDING)
		{
			// Wait here (forever) for the Overlapped I/O to complete
			if (!GetOverlappedResult(gHdlDevice, &overlapped, &bytes_returned, TRUE))
			{
				last_error_status = GetLastError();
#if _DEBUG		
				printf("Packet Gen/Chk Control failed. Error = %d\n", last_error_status);
#endif // _DEBUG
				status = STATUS_PARAMETER_SETTINGS_ERROR;
			}
		}
		else
		{
			// ioctl failed	
#if _DEBUG	
			printf("Packet Generator Control failed. Error = %d\n", last_error_status);
#endif		
			status = STATUS_PARAMETER_SETTINGS_ERROR;
		}
	}
	if (overlapped.hEvent != NULL)
		CloseHandle(overlapped.hEvent);
*/

    return status;

	//return SetImagingSettings();
}

LONG CdFLIM_4002::SetGlobalSettings(dFLIM_IMAGING_CONFIGURATION_STRUCT	imaging_config)
{
	//set up const settings
	gPtrAcqCtrl->gblCtrl.dma_engine_index   = gDmaInfo.PacketRecvEngine[0];         // set DMA engine
	gPtrAcqCtrl->gblCtrl.run_stop_mode		= PACKET_GEN_RUN;                       // enable acquisition bit
	gPtrAcqCtrl->gblCtrl.acq_buf_addr		= (ULONG32)0x00000000;
	gPtrAcqCtrl->gblCtrl.acq_buf_chn_offset = (ULONG32)DDR3_SINGLEIMAGE_CHANNEL_CAP;
	// REDESIGN
	//	gPtrAcqCtrl->gblCtrl.acquisitionMode	= imaging_config.imageCtrl.acquisitionMode; //0 = DFLIM mode, 1 = diagnostic mode, 2 = Counter mode

	// set uo the channels are enabled
	if ((imaging_config.imageCtrl.channel - MIN_CHANNEL) <=  RANGE_CHANNEL) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.ImagingChan = imaging_config.imageCtrl.channel;  
	}else
	{
		return FALSE;
	}
	// set up frame count to be acquired
	if (imaging_config.imageCtrl.frameCnt  <=  MAX_FRAME_NUM) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.frame_number = imaging_config.imageCtrl.frameCnt;  
	}else
	{
		return FALSE;
	}
	// set up horizontal pixel density
	if ((imaging_config.imageCtrl.imgHSize - MIN_PIXEL_X) <=  RANGE_PIXEL_X) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.hor_pix_num = imaging_config.imageCtrl.imgHSize;  
	}else
	{
		return FALSE;
	}
	if (imaging_config.imageCtrl.dataHSize <= MAX_DATA_HSIZE) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.dataHSize = imaging_config.imageCtrl.dataHSize;  
	}else
	{
		return FALSE;
	}
	// set up vertical pixel density
	if ((imaging_config.imageCtrl.imgVSize - MIN_PIXEL_Y) <=  RANGE_PIXEL_Y) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.vrt_pix_num = imaging_config.imageCtrl.imgVSize;  
	}else
	{
		return FALSE;
	}
	if ((imaging_config.imageCtrl.linesPerStripe - MIN_PIXEL_Y) <=  RANGE_PIXEL_Y) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.linesPerStripe = imaging_config.imageCtrl.linesPerStripe;  
	}else
	{
		return FALSE;
	}
	//set up frame rate
	if ((imaging_config.imageCtrl.frameNumPerSec - MIN_FRAME_RATE) <=  RANGE_FRAME_RATE) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.frm_per_sec		= imaging_config.imageCtrl.frameNumPerSec;
		gPtrAcqCtrl->gblCtrl.frm_per_txn		= imaging_config.imageCtrl.frameNumPerTransfer;
	}else
	{
		return FALSE;
	}
	//set up debug mode
	if (imaging_config.imageCtrl.defaultMode - MIN_DEBUG_MODE <=  RANGE_DEBUG_MODE) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.dbg_test_mode		= imaging_config.imageCtrl.defaultMode; // Disable Debug Mode (only used with GUI)
	}else
	{
		return FALSE;
	}
	if (imaging_config.imageCtrl.defaultMode == MIN_DEBUG_MODE)
	{
		//Set up Scan direction, Bi-Scan mode, hardware trigger mode
		if (imaging_config.imageCtrl.scanMode < 2   &&  imaging_config.imageCtrl.scanDir < 2 && imaging_config.imageCtrl.triggerMode < 2) //bounds check
		{
			{
				gPtrAcqCtrl->gblCtrl.img_scan_mode = imaging_config.imageCtrl.scanMode << 1 | (imaging_config.imageCtrl.scanDir << 2) | ((imaging_config.imageCtrl.triggerMode ^ 0x01) << 4);
			}
		}else
		{
			return FALSE;
		}

		//set gpio
		// REDESIGN
/*		gPtrAcqCtrl->gblCtrl.gpio_cfg = ((static_cast<ULONG64>(CH0)))
			| ((static_cast<ULONG64>(CH1)) << 8)
			| ((static_cast<ULONG64>(CH2)) << 16)
			| ((static_cast<ULONG64>(CH3)) << 24)
			| ((static_cast<ULONG64>(CH4)) << 32)
			| ((static_cast<ULONG64>(CH5)) << 40)
			| ((static_cast<ULONG64>(CH6)) << 48)
			| ((static_cast<ULONG64>(CH7)) << 56); */
		// REDESIGN
	}
	
	return TRUE;
}

LONG CdFLIM_4002::SetScanSettings(dFLIM_IMAGING_CONFIGURATION_STRUCT	imaging_config)
{
	//Set up GG/GR scan mode, frame count
	if (imaging_config.imageCtrl.system_mode < 2)
	{
		gPtrAcqCtrl->scan.sync_ctrl = (gPtrAcqCtrl->scan.sync_ctrl & 0x1D) | (imaging_config.imageCtrl.system_mode << 5);
		if (imaging_config.imageCtrl.frameCnt != (ULONG32)MAX_FRAME_NUM) // Continuously scan
		{
			gPtrAcqCtrl->scan.sync_ctrl = gPtrAcqCtrl->scan.sync_ctrl | 0x02;
			gPtrAcqCtrl->scan.frm_cnt	= imaging_config.imageCtrl.frameCnt;
		}
		//aditional DFLIM settings
		gPtrAcqCtrl->scan.sync_ctrl |= (0xbc << 8);
	}
	else
	{
		return FALSE;
	}
	
	/**** Write to log Frm_Cnt & FrameCnt for trouble-shooting 
	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg,MSG_SIZE,L"CdFLIM \nFrm_Cnt: %d \nFrameCnt: %d", gPtrAcqCtrl->scan.frm_cnt, imaging_config.imageCtrl.frameCnt);
	CdFLIM_4002::LogMessage(errMsg,VERBOSE_EVENT);
	****/

	//gPtrAcqCtrl->scan.pll_fltr_ctrl   = 0x42; //Default Thordaq ADPLL Loop Filter settings
	//gPtrAcqCtrl->scan.pll_cntr_freq   = MAXULONG32 /imaging_config.imageCtrl.sample_rate  * 8000; //8khz
	
	gPtrAcqCtrl->scan.pll_sync_offset = max(1, imaging_config.imageCtrl.alignmentOffset);

	//Setup Galvo Settings
	if (imaging_config.imageCtrl.system_mode == SYSTEM_MODE::INTERNAL_GALVO_GALVO)
	{
		gPtrAcqCtrl->scan.galvo_pixel_delay = static_cast<ULONG32>(round(imaging_config.galvoGalvoCtrl.pixelDelayCnt));
		gPtrAcqCtrl->scan.galvo_pixel_dwell = static_cast<ULONG32>(round(imaging_config.galvoGalvoCtrl.dwellTime * (double)SYS_CLOCK_FREQ));
		gPtrAcqCtrl->scan.galvo_intra_line_delay  = static_cast<ULONG32>(round((imaging_config.galvoGalvoCtrl.turnaroundTime * (double)SYS_CLOCK_FREQ / 2.0  - 1.0) / 16.0));
		gPtrAcqCtrl->scan.galvo_intra_frame_delay = static_cast<ULONG32>(round(imaging_config.galvoGalvoCtrl.flybackTime * (double)SYS_CLOCK_FREQ  - 2.0) / 16.0);// Flyback_time resgiter = (flyback time  * acq_clk_freq - 2) / 16
	}
	else
	{
		gPtrAcqCtrl->scan.galvo_intra_frame_delay =static_cast<ULONG32>(imaging_config.resonantGalvoCtrl.flybackTime * imaging_config.streamingCtrl.scan_period);
	}

	return TRUE;
}

LONG CdFLIM_4002::SetCoherentSampleingSettings(dFLIM_IMAGING_CONFIGURATION_STRUCT imaging_config)
{
	//Dont consider GG Scan with Conherent Sampling
	if(imaging_config.coherentSamplingCtrl.phaseIncrementMode > (USHORT)(0))
	{
		// "samplingClock.phase_ctrl" is .controlRegister0 in common Thordaq
		gPtrAcqCtrl->samplingClock.CtrlReg = static_cast<UCHAR>  (imaging_config.coherentSamplingCtrl.phaseIncrementMode - 1);
		gPtrAcqCtrl->samplingClock.CtrlReg |= (imaging_config.imageCtrl.threePhotonMode == TRUE)? 0x08 : 0x00;
		gPtrAcqCtrl->samplingClock.phase_offset = static_cast<USHORT> (imaging_config.coherentSamplingCtrl.phaseOffset);
		gPtrAcqCtrl->samplingClock.phase_step   = static_cast<UCHAR>  (imaging_config.coherentSamplingCtrl.phaseStep);
		gPtrAcqCtrl->samplingClock.phase_limit  = static_cast<USHORT> (imaging_config.coherentSamplingCtrl.phaseLimit);
			
		gPtrAcqCtrl->streamProcessing.pulse_interleave_offset = 0;
		gPtrAcqCtrl->streamProcessing.pulse_interleave = 0;
		if (imaging_config.streamingCtrl.channel_multiplexing_enabled == TRUE)		{

			gPtrAcqCtrl->streamProcessing.pulse_interleave |= 0x0f;
			gPtrAcqCtrl->streamProcessing.pulse_interleave_offset = 0x11 ;
		}
		gPtrAcqCtrl->streamProcessing.pulse_interleave |= (imaging_config.imageCtrl.threePhotonMode == TRUE)? 0x80 : 0x00;

	}
	return TRUE;
}

LONG CdFLIM_4002::SetStreamProcessingSettings(dFLIM_IMAGING_CONFIGURATION_STRUCT	imaging_config)
{
	double pixel_frequency = 0;
	double SampleRate = imaging_config.imageCtrl.clockRate;
	double lineTime = 0;

	if (imaging_config.imageCtrl.system_mode == SYSTEM_MODE::INTERNAL_GALVO_GALVO) // GG Settings
	{
		gPtrAcqCtrl->streamProcessing.stream_ctrl = 0x02; // Enable DC Offset Correction pre-FIR filter
		pixel_frequency = 1.0 / imaging_config.galvoGalvoCtrl.dwellTime;
		double lineSweepTime  = ((double)gPtrAcqCtrl->scan.galvo_pixel_dwell * (double)gPtrAcqCtrl->gblCtrl.hor_pix_num) + ((double)gPtrAcqCtrl->scan.galvo_pixel_delay * 16.0 + 1.0) * ((double)(gPtrAcqCtrl->gblCtrl.hor_pix_num - 1)); // (Dwell Time + Pixel Delay Time) * PixelX - Pixel Delay Time
		double turnAroundTime = 2.0 * ((double)gPtrAcqCtrl->scan.galvo_intra_line_delay * 16.0 + 1.0);
		lineTime = (lineSweepTime + turnAroundTime) / SYS_CLOCK_FREQ;
		/*wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg,MSG_SIZE,L"CdFLIM SetStreamProcessingSettings \n LineTime: %f", lineTime);
		LogMessage(errMsg,ERROR_EVENT);	*/		
	}
	else // GR Settings
	{
		gPtrAcqCtrl->streamProcessing.stream_ctrl  = 0x03; // enable Scan Period Source and DC Offset Correction pre-FIR filter
		lineTime = 1.0 / 2.0 / imaging_config.streamingCtrl.scan_period;
		pixel_frequency = 1.0 / (lineTime / static_cast<double>(imaging_config.imageCtrl.imgHSize));	
	}

	if (imaging_config.imageCtrl.threePhotonMode) //this part can be integrated into main logic. This block is for test purpose.
	{
		gPtrAcqCtrl->streamProcessing.scan_period = static_cast<USHORT>(round((double)imaging_config.imageCtrl.clockRate * lineTime)); //static_cast<USHORT>( imaging_config.imageCtrl.clockRate * lineTime );
		ULONG32 downSampleRate =  static_cast<ULONG32>(floor(ADC_MAX_SAMPLE_RATE / (double)imaging_config.imageCtrl.clockRate));
		if ((downSampleRate % 2) != 0)
		{
			--downSampleRate;
		}
		gPtrAcqCtrl->streamProcessing.downsample_rate = downSampleRate;

		gPtrAcqCtrl->streamProcessing.threePhoton_sample_offset = imaging_config.imageCtrl.threePhotonPhaseAlignment;
		return TRUE;
	}

	while (lineTime * SampleRate > USHRT_MAX)//16Bits
	{
		SampleRate = SampleRate / 2.0;
	}
	

	gPtrAcqCtrl->streamProcessing.downsample_rate = static_cast<ULONG32>(((ADC_MAX_SAMPLE_RATE / SampleRate) - 1) /** 16843009*/); //* (1+2^8+2^16+2^24);
	gPtrAcqCtrl->streamProcessing.scan_period     = static_cast<USHORT>(ceil(lineTime * SampleRate));          //Round the value and set in the galvo galvo system
	return TRUE;
}

LONG CdFLIM_4002::SetWaveformPlayback( DAC_CRTL_STRUCT dac_setting, int channel, UINT8 set_flag )
{
	ULONG64 GalvoBitsMask = 0x000000000000ffff;
	USHORT park_mid = dac_setting.park_val > 0? 0x7fff: 0x8000;
	USHORT offset_mid = dac_setting.offset_val > 0? 0x7fff: 0x8000;
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;

	int bits_to_shift = ((channel  % 4) * 16);
	int index = static_cast<int>(floor(channel / 4));

	gPtrAcqCtrl->galvoCtrl.dacUpdateRate[index] = (gPtrAcqCtrl->galvoCtrl.dacUpdateRate[index] & ~(GalvoBitsMask << bits_to_shift)) |((ULONG64)round(SYS_CLOCK_FREQ/ dac_setting.update_rate - 1) << bits_to_shift);
	gPtrAcqCtrl->galvoCtrl.dacOffset[index] = (gPtrAcqCtrl->galvoCtrl.dacOffset[index] & ~(GalvoBitsMask << bits_to_shift)) |(static_cast<ULONG64>((dac_setting.offset_val / GALVO_RESOLUTION) + offset_mid) << bits_to_shift);
	gPtrAcqCtrl->galvoCtrl.dacParkValue[index] = (gPtrAcqCtrl->galvoCtrl.dacParkValue[index] & ~(GalvoBitsMask << bits_to_shift)) |(static_cast<ULONG64>((dac_setting.park_val / GALVO_RESOLUTION) + park_mid) << bits_to_shift);

	if(1)// (set_flag)  In legacy kernel, all of these registers were written to FPGA; to test/fix all channels, revert to original writes, THEN convert to shadow regs
	{
		ULONGLONG update_rate_addr = 0x248 + (UINT64)index * 8; 
		ULONGLONG park_addr = 0x268 + (UINT64)index * 8;
		ULONGLONG offset_addr = 0x280 + (UINT64)index * 8;

		BYTE* buffer = new BYTE[8];
		STAT_STRUCT StatusInfo;
		
//		status = FPGAregisterWrite(DACupdateRates[index], (int)strlen("DAC_UpdateRate_ChanN"), gPtrAcqCtrl->galvoCtrl.dacUpdateRate[index]);
		memcpy(buffer,&gPtrAcqCtrl->galvoCtrl.dacUpdateRate[index],sizeof(ULONG64));
		WriteReadRegister(WRITE_TO_CARD,3,update_rate_addr,buffer,0,8,&StatusInfo);
		if (status != STATUS_SUCCESSFUL)
		{
			SAFE_DELETE_ARRAY(buffer);
			return FALSE;
		}

		memcpy(buffer,&gPtrAcqCtrl->galvoCtrl.dacOffset[index],sizeof(ULONG64));
		WriteReadRegister(WRITE_TO_CARD,3,offset_addr,buffer,0,8,&StatusInfo);
		if (status != STATUS_SUCCESSFUL)
		{
			SAFE_DELETE_ARRAY(buffer);
			return FALSE;
		}

		//TODO: Parking should only be set at startup and when it changes, probably should be the same for update rate
		//and offset. Figure out what is the best way forward.
		//memcpy(buffer,&gPtrAcqCtrl->galvoCtrl.dacParkValue[index],sizeof(ULONG64));
		//ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(WRITE_TO_CARD,3,park_addr,buffer,0,8,&StatusInfo))
		//if (status != STATUS_SUCCESSFUL)
		//{
		//	SAFE_DELETE_ARRAY(buffer);
		//	return FALSE;
		//}

		SAFE_DELETE_ARRAY(buffer);
	}
	
	return TRUE;
}


// The Legacy kernel wrote most of the DAC settings to FPGA registers
LONG CdFLIM_4002::SetDACSettings(dFLIM_IMAGING_CONFIGURATION_STRUCT	imaging_config)
{
	ULONG64 DMAChannelCountBitsMask = 0x000000000000000F;
	ULONG64 DMAChannelBitsMask		= 0x0000000000000010;
	ULONG64 LineSyncBitsMask		= 0x0000000040000000;

	ULONG64 DmaChannelCount  = 0x0000000000000000;
	ULONG64 max_flyback_samples = 80;
	gPtrAcqCtrl->galvoCtrl.dacChannelMap = 0;
	gPtrAcqCtrl->galvoCtrl.ctrlReg = 0;
	for (int i = 0; i < DAC_CHANNEL_COUNT; i++)
	{
			DAC_CRTL_STRUCT dac_setting = imaging_config.dacCtrl[i];
			if (dac_setting.output_port== 10)
			{
				dac_setting.output_port = 11;
			}
			else if (dac_setting.output_port == 11)
			{
				dac_setting.output_port = 10;
			}
			gPtrAcqCtrl->galvoCtrl.dacChannelMap += (dac_setting.output_port << (i * 4));
			
			gPtrAcqCtrl->galvoCtrl.ctrlReg = gPtrAcqCtrl->galvoCtrl.ctrlReg & (~DMAChannelCountBitsMask) | ((DmaChannelCount));
			gPtrAcqCtrl->galvoCtrl.ctrlReg = gPtrAcqCtrl->galvoCtrl.ctrlReg | (DMAChannelBitsMask << i);
			//if (imaging_config.imageCtrl.system_mode == SYSTEM_MODE::INTERNAL_RESONANT_GALVO || (imaging_config.imageCtrl.system_mode == SYSTEM_MODE::INTERNAL_GALVO_GALVO && imaging_config.imageCtrl.threePhotonMode))
			//{
				gPtrAcqCtrl->galvoCtrl.ctrlReg = gPtrAcqCtrl->galvoCtrl.ctrlReg | (LineSyncBitsMask << i);
			//}
			DmaChannelCount++;
			if (DAC_CHANNEL_COUNT - 1 > i)
			{
				if ((SYSTEM_MODE::INTERNAL_GALVO_GALVO == imaging_config.imageCtrl.system_mode && (int)GalvoResonantY == i) ||
					(SYSTEM_MODE::INTERNAL_RESONANT_GALVO == imaging_config.imageCtrl.system_mode && (int)GalvoGalvoX == i))
				{ 
					// If it is GG imaging, don't move the GR Y Galvo to offset 0, keep it in park instead. For GR imaging, keep GG X Galvo at park position (10V)
					dac_setting.offset_val = (double)GALVO_PARK_POSITION;
				}
				else if (SYSTEM_MODE::INTERNAL_RESONANT_GALVO == imaging_config.imageCtrl.system_mode && (int)GalvoGalvoY == i)
				{
					// If it is GR imaging, keep the GG Y galvo in park position (-10V)
					dac_setting.offset_val = -1.0 * (double)GALVO_PARK_POSITION;
				}
				SetWaveformPlayback(dac_setting,i, false);
			}
			max_flyback_samples = max(max_flyback_samples,dac_setting.flyback_samples);
	}	

	//gPtrAcqCtrl->galvoCtrl.dacChannelMap += (DAC_CHANNEL::DO0 << (12 * 4));
	// NOTE: doOffset is an array of 2 elements in Thordaq common, so use element [0] only in dFLIM
	gPtrAcqCtrl->galvoCtrl.doOffset = 0; // doOffset is an array of 2 elements in Thordaq common, so use element[0] only in dFLIM
	gPtrAcqCtrl->galvoCtrl.doParkValue = 0;
	gPtrAcqCtrl->galvoCtrl.doUpdateRate = 0;//static_cast<ULONG64>(round(SYS_CLOCK_FREQ/ imaging_config.dacCtrl[DAC_CHANNEL_COUNT - 1].update_rate - 1));
	int bits_to_shift = ((0  % 4) * 16);
	int bits_to_shift2 = ((1  % 4) * 16);
	int bits_to_shift3 = ((2  % 4) * 16);
	int bits_to_shift4 = ((3  % 4) * 16);
	ULONG64 GalvoBitsMask = 0x000000000000ffff;
	gPtrAcqCtrl->galvoCtrl.doUpdateRate = (gPtrAcqCtrl->galvoCtrl.doUpdateRate & ~(GalvoBitsMask << bits_to_shift)) | (USHORT)((ULONG64)round(SYS_CLOCK_FREQ/ imaging_config.dacCtrl[DAC_CHANNEL_COUNT - 1].update_rate - 1) << bits_to_shift);
	gPtrAcqCtrl->galvoCtrl.doUpdateRate = (gPtrAcqCtrl->galvoCtrl.doUpdateRate & ~(GalvoBitsMask << bits_to_shift2)) | (USHORT)((ULONG64)round(SYS_CLOCK_FREQ/ imaging_config.dacCtrl[DAC_CHANNEL_COUNT - 1].update_rate - 1) << bits_to_shift2);
	gPtrAcqCtrl->galvoCtrl.doUpdateRate = (gPtrAcqCtrl->galvoCtrl.doUpdateRate & ~(GalvoBitsMask << bits_to_shift3)) | (USHORT)((ULONG64)round(SYS_CLOCK_FREQ/ imaging_config.dacCtrl[DAC_CHANNEL_COUNT - 1].update_rate - 1) << bits_to_shift3);
	gPtrAcqCtrl->galvoCtrl.doUpdateRate = (gPtrAcqCtrl->galvoCtrl.doUpdateRate & ~(GalvoBitsMask << bits_to_shift4)) | (USHORT)((ULONG64)round(SYS_CLOCK_FREQ/ imaging_config.dacCtrl[DAC_CHANNEL_COUNT - 1].update_rate - 1) << bits_to_shift4);
	
	gPtrAcqCtrl->galvoCtrl.dacStepSize[0] = 1;
	gPtrAcqCtrl->galvoCtrl.dacStepSize[1] = 1;
	gPtrAcqCtrl->galvoCtrl.dacStepSize[2] = 1;


	gPtrAcqCtrl->galvoCtrl.dacAmFilterWindow = max_flyback_samples;

	return TRUE;
}


/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM::StartAcquisition()
 *
 * @brief	Start acquisition.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS CdFLIM_4002::StartAcquisition()
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	//TODO: check if lines below are necessary or can be removed
	/////////////////////////////////////////////////////////////////////////////
	//// line 42
	/////////////////////////////////////////////////////////////////////////////
	//SetI2C_0x2C0(0x01990000);
	//SetI2C_0x2C4(0x000529);
	//SetI2C_0x2C8_232();

	//Sleep(250);

//	ResetBackEndAcq();  // added by gy

	if (gPtrAcqCtrl != nullptr)
	{
		status = GlobalSCANstart(true);
	}
//	status = SetPacketMode(0,true, NULL, PACKET_MODE_ADDRESSABLE, 8192);
	return status;
}

THORDAQ_STATUS CdFLIM_4002::SetPacketModeAddressable(bool enableStreamToDMA)
{
	THORDAQ_STATUS status;
	status = SetPacketMode(0, enableStreamToDMA, NULL, PACKET_MODE_ADDRESSABLE, 8192);
	return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM::StopAcquisition()
 *
 * @brief	Stop acquisition.
 *
 * @author	Cge
 * @date	3/17/2017
 * @date    9/11/2023  DZimmerman, IMG_ACQ_CONF_IOCTL never existed in API-NWL driver
 * (Stop Acq in user software, not in the kernel)
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS CdFLIM_4002::StopAcquisition()
{
	THORDAQ_STATUS status;

	status = GlobalSCANstart(false); // handles USER INT wait task termination & other

	return status;
}

// DAC descriptor for LoadDACDescriptors() function below
dFLIM_DAC_DESCP_TABLE dmaDescpTable;
LONG CdFLIM_4002::LoadDACDescriptors(DAC_CRTL_STRUCT* dac_settings)
{
	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;
	STAT_STRUCT StatusInfo;
	BYTE* buffer = new BYTE[8];
	// Sort the _dacDescpList first

	memset(dmaDescpTable.descp, 0, sizeof(ULONG64) * dFLIM_DAC_DESCP_MAX_LEN); // initiate the table which is used to load into the Device
	DMADescriptor despTable[DAC_CHANNEL_COUNT];
	memset(despTable, 0, sizeof(DMADescriptor) * DAC_CHANNEL_COUNT);

	_dacDescpListIndex = DAC_CHANNEL_COUNT - 1;
	bool is_set_flag = true;
	for (ULONG64 i = 0; i < DAC_CHANNEL_COUNT; i++)
	{
		DAC_CRTL_STRUCT dac_setting = *(dac_settings+i);
		if (dac_setting.waveform_buffer_length == 0)
		{
			continue;
		}
		despTable[i].buf_addr  = dac_setting.waveform_buffer_start_address;
		despTable[i].length    = dac_setting.waveform_buffer_length;
		despTable[i].next_node = i;
		SetDMADescriptors(despTable[i], i, dmaDescpTable.descp, is_set_flag); // iterative function: dmaDescpTable is output
		if (!is_set_flag)
		{
			return FALSE; 
		}
	}
	// now write the FPGA WAVETABLE entries starting at BAR3, 0x10000
	ULONGLONG BAR3addr;
	for (ULONGLONG ULONGindex = 0; ULONGindex < dFLIM_DAC_DESCP_MAX_LEN; ULONGindex++) // a 32k byte write, 8 bytes 4096 times
	{
		BAR3addr = 0x10000 + ULONGindex*8; // pointer arithmetic
		// we need to break the 64 bits into 4 byte (max size of call) halves
		UINT64 LongValue = dmaDescpTable.descp[ULONGindex];
		memcpy(buffer, &LongValue, 8); // not using stack variable for PCI register value
		status = WriteReadRegister(WRITE_TO_CARD, 3, BAR3addr, buffer, 0, 8, &StatusInfo); // MAX write size is 4 bytes
	}
	SAFE_DELETE_ARRAY(buffer);

	/******   DAC_DESC_SETUP_IOCTL does not exist in NWL-API version
	OVERLAPPED					os;			// OVERLAPPED structure for the operation
	DWORD						bytes_returned = 0;
	DWORD						last_error_status = 0;
	os.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	// Send Write PCI Configuration IOCTL
	if (!DeviceIoControl( gHdlDevice, DAC_DESC_SETUP_IOCTL, (LPVOID) &dmaDescpTable, sizeof(DAC_DESCP_TABLE), NULL, 0, &bytes_returned, &os))
	{
		last_error_status = GetLastError();
		if (last_error_status != ERROR_IO_PENDING)
		{			
			//Status->CompletedByteCount = 0;	
			printf("ScanLUTSetup IOCTL call failed. Error = %d\n", last_error_status);
			return FALSE;
		}
		// Wait here (forever) for the Overlapped I/O to complete
		if (!GetOverlappedResult(gHdlDevice, &os, &bytes_returned, TRUE))
		{
			last_error_status = GetLastError();
#if _DEBUG		
			printf("Packet Gen/Chk Control failed. Error = %d\n", last_error_status);
#endif // _DEBUG
			return FALSE;
		}
	}
	if (os.hEvent != NULL)
		CloseHandle(os.hEvent);
		*/
	return status;
}

// output of this iterative algorithm is dmaDescpTable -
// that table is written to BAR3 0x10000 (Wave Descriptor Table)
void CdFLIM_4002::SetDMADescriptors(DMADescriptor& dmaDescp, ULONG64 index, ULONG64* dmaDescpTable, bool& status)
{
	if (dmaDescp.length <= DAC_TRANSMIT_BUFFER_MAX)
	{
		*(dmaDescpTable+index) = dmaDescp.next_node << 52 | dmaDescp.length << 36 | dmaDescp.buf_addr;
	}else
	{
		if (_dacDescpListIndex > dFLIM_DAC_DESCP_MAX_LEN - 2) // reach the end of the discriptor table
		{
			status = false;
			return;
		}
		// Because waveform data includes 1024 words at the end to leave the enough space to flush the FIFO. So must leave 1024 words space at the end of last discriptor. 
		// If it doesn't, put some data (choose 2048 samples here) to the next discriptor.
		if ((dmaDescp.length - DAC_TRANSMIT_BUFFER_MAX) <= (DAC_FIFO_DEPTH / 2))
		{
			*(dmaDescpTable+index) = (_dacDescpListIndex + 1) << 52 | (ULONG64)(DAC_TRANSMIT_BUFFER_MAX - DAC_FIFO_DEPTH) << 36 | dmaDescp.buf_addr;
			dmaDescp.length = dmaDescp.length - (ULONG64)(DAC_TRANSMIT_BUFFER_MAX - DAC_FIFO_DEPTH);
			dmaDescp.buf_addr = dmaDescp.buf_addr + (ULONG64)(DAC_TRANSMIT_BUFFER_MAX - DAC_FIFO_DEPTH);
		}else
		{
			*(dmaDescpTable+index) = (_dacDescpListIndex + 1) << 52 | (ULONG64)DAC_TRANSMIT_BUFFER_MAX << 36 | dmaDescp.buf_addr;
			dmaDescp.length = dmaDescp.length - (ULONG64)DAC_TRANSMIT_BUFFER_MAX;
			dmaDescp.buf_addr = dmaDescp.buf_addr + (ULONG64)DAC_TRANSMIT_BUFFER_MAX;
		}
		SetDMADescriptors(dmaDescp,++_dacDescpListIndex,dmaDescpTable,status);
	}
}


// ThorDAQ's API function to set Park voltage on BOB's AO channel's BNC connector
// MODIFY THE Analog MUX so that zero-based channels match the BNC label
// get and set the DAC channel voltages by channel and DC voltage
// ADC counts   Voltage
// 0x0    - 0x7FFF  -10.0 to 0
// 0x8000 - 0xFFFF  +0.0 to +10.0
//static const char* ParkVoltageRegName[DAC_ANALOG_CHANNEL_COUNT] = {  // index[12],[13] DigitalWaveform channels can't be used on dFLIM

THORDAQ_STATUS CdFLIM_4002::APIsetDACvoltage( UINT32 chan, double Voltage)
{
	THORDAQ_STATUS status;
	ULONG64 dacChannelMap = 0x000089AB45670123;  // Analog MUX so that zero - based channels match the BNC label
	status = FPGAregisterWRITE("GlobalABBXmuxReg", dacChannelMap);

	USHORT DACunitsMidpoint = Voltage > 0 ? 0x7FFF : 0x8000; // note difference between -0.0 and +0.0
// about 0.000305176 Volts per DAC count
	USHORT DACunits_toSetVDC = static_cast<USHORT>(floor(Voltage / (20.0 / 65536) + 0.5) + DACunitsMidpoint);
#define cFldName 32
	char cFieldName[cFldName];  // holds shadow reg name, e.g. DAC_ParkValue_Chan3

	// legacy code zeros the update rate Register for Park condition
	sprintf_s(cFieldName, cFldName, "DAC_Offset_Chan%d", chan);  // sprintf adds the null
	status = FPGAregisterWrite(cFieldName, (int)strlen(cFieldName), 0);


	sprintf_s(cFieldName, cFldName, "DAC_ParkValue_Chan%d", chan);  // sprintf adds the null
 	status = FPGAregisterWrite(cFieldName, (int)strlen(cFieldName), DACunits_toSetVDC);
	return status;
}



// This function is call by upper Level DLL by TILS when the "FindCamera" / "SelectCamera" sequence commences
THORDAQ_STATUS CdFLIM_4002::SetDACParkValue(ULONG32 outputChannel, double parkValue)
{

	if (outputChannel > 11 || parkValue < -10.0 || parkValue > 10.0 )
	{
		return STATUS_PARAMETER_SETTINGS_ERROR;
	}

	USHORT park_mid = parkValue > 0? 0x7fff: 0x8000;
	int index = static_cast<int>(floor(outputChannel / 4));
	USHORT dacUpdateRate = static_cast<USHORT>(std::floor(SYS_CLOCK_FREQ/ DAC_MIN_UPDATERATE + 0.5));
	USHORT dacParkValue = static_cast<USHORT>(std::floor(parkValue / GALVO_RESOLUTION + 0.5) + park_mid);

	ULONGLONG update_rate_addr = 0x248 + (ULONGLONG)outputChannel * 2; 
	ULONGLONG park_addr = 0x268  + (ULONGLONG)outputChannel * 2;

	STAT_STRUCT StatusInfo;
	BYTE* buffer = new BYTE[2];		
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;
	memcpy(buffer,&dacUpdateRate,sizeof(USHORT));
	UINT64 RegValue = dacUpdateRate;

	status = WriteReadRegister(WRITE_TO_CARD,3,update_rate_addr,buffer,0,2,&StatusInfo);
//	if (outputChannel < 4)
//		status = FPGAregisterWrite(DACUpdateRate[0], (int)(strlen(DACUpdateRate[0])), RegValue);

	if (status!= STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		return status;
	}

	memcpy(buffer,&dacParkValue,sizeof(USHORT));
	status = WriteReadRegister(WRITE_TO_CARD,3,park_addr,buffer,0,2,&StatusInfo);
	if (status != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		return status;
	}

	SAFE_DELETE_ARRAY(buffer);

	return STATUS_SUCCESSFUL;
}


THORDAQ_STATUS CdFLIM_4002::SetDACChannelMapping()
{
	ULONG64 dacChannelMap = 0x0000BA9876543210;
	STAT_STRUCT StatusInfo;
	THORDAQ_STATUS status;

	gPtrAcqCtrl->galvoCtrl.dacChannelMap = dacChannelMap;

	// write the Shadow Reg
	status = FPGAregisterWrite("GlobalABBXmuxReg", (int)strlen("GlobalABBXmuxReg"), dacChannelMap);

	if (WriteReadRegister(WRITE_TO_CARD, 3, 0x298, (BYTE*)&dacChannelMap , 0, 8, &StatusInfo) == STATUS_SUCCESSFUL) return STATUS_SUCCESSFUL;

	return THORDAQ_STATUS::STATUS_INITIATION_INCOMPLETE;
}

THORDAQ_STATUS CdFLIM_4002::SetADCChannelMapping()
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;



	// REDESIGN
// make GPIO match dFLIM static assignment at startup
	gPtrAcqCtrl->gblCtrl.gpio_cfg = (ULONG64)0x01080007090a0304; 
	// REDESIGN

	BYTE* GPIOBuffer = new BYTE[8];
	STAT_STRUCT StatusInfo;
	memcpy(GPIOBuffer,&gPtrAcqCtrl->gblCtrl.gpio_cfg,sizeof(ULONG64));
	status = WriteReadRegister(WRITE_TO_CARD, 3, 0x020, GPIOBuffer, 0 , 8, &StatusInfo);
	if (status != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY( GPIOBuffer);
		return STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY( GPIOBuffer);
	
	return status;
}


THORDAQ_STATUS CdFLIM_4002::GetLineTriggerFrequency(UINT32 sample_rate, double& frequency)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	BYTE* buffer = new BYTE[2];
	USHORT val = 0x0000;
	memcpy(buffer,&val,sizeof(USHORT));
	STAT_STRUCT StatusInfo;
	status = WriteReadRegister(READ_FROM_CARD,3,0x140,buffer,0,2,&StatusInfo);
	memcpy(&val,buffer,sizeof(USHORT));
	if (status == STATUS_SUCCESSFUL)
	{
		if (val != 0 && val < 0x5120 && val > 0x4D2B)
		{
			frequency = static_cast<double>(sample_rate) /  static_cast<double>(val);
		}else
		{
			frequency = CrsFrequencyHighPrecision;
		}
		
	}else
	{
		status = STATUS_READWRITE_REGISTER_ERROR;;	
	}
	SAFE_DELETE_ARRAY(buffer);
	return 	status;
}


THORDAQ_STATUS CdFLIM_4002::GetTotalFrameCount(UINT32& frame_count)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	BYTE* buffer = new BYTE[2];
	memcpy(buffer,&frame_count,sizeof(USHORT));
	STAT_STRUCT StatusInfo;
	status = WriteReadRegister(READ_FROM_CARD,3,0x142,buffer,0,2,&StatusInfo);
	memcpy(&frame_count,buffer,sizeof(USHORT));
	if (status != STATUS_SUCCESSFUL)
	{
		status = STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return status;
}


THORDAQ_STATUS CdFLIM_4002::APIdFLIMGetClockFrequency(int clockIndex, double& frequency)
{
	ULONG clock = (ULONG)clockIndex;
	frequency = 0;
	double averagedValue = 0;
	const long AVGTIMES = 5;
	for (int i = 0; i < AVGTIMES; ++i)
	{	
		SetNONI2C_0x1C0(clock);
		SetNONI2C_0x1C4(0x10000600);
		SetNONI2C_0x1C8(0x01);
		SetNONI2C_0x1C8(0x00);

		Sleep(5);

		SetNONI2C_0x1C0(0x00000000);
		SetNONI2C_0x1C4(0x20000601);
		SetNONI2C_0x1C8(0x01);
		SetNONI2C_0x1C8(0x00);

		THORDAQ_STATUS status = STATUS_SUCCESSFUL;
		const long readLength = 8;
		ULONG64 value;
		STAT_STRUCT StatusInfo;
		status = WriteReadRegister(READ_FROM_CARD,3,0x1c0,(BYTE*)(&value),0,readLength,&StatusInfo);
		if (status != STATUS_SUCCESSFUL)
		{
			frequency = 0;
			status = STATUS_READWRITE_REGISTER_ERROR;
		}

		averagedValue+=(value & 0xFFFF);
	}

	averagedValue/=AVGTIMES;

	frequency = 200*(averagedValue + 1) / 8192;

	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CdFLIM_4002::SetCoarseShift(ULONG32 shift, int channel)
{
	STAT_STRUCT StatusInfo;
	USHORT shortShift = static_cast<USHORT>(shift);


	switch (channel)
	{
	case 0:
		if (WriteReadRegister(WRITE_TO_CARD,3,0x318,(BYTE*)(&shortShift),0,2,&StatusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
		break;
	case 1:
		if (WriteReadRegister(WRITE_TO_CARD,3,0x358,(BYTE*)(&shortShift),0,2,&StatusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
		break;
	case 2:
		if (WriteReadRegister(WRITE_TO_CARD,3,0x398,(BYTE*)(&shortShift),0,2,&StatusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
		break;
	case 3:
		if (WriteReadRegister(WRITE_TO_CARD,3,0x3D8,(BYTE*)(&shortShift),0,2,&StatusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
		break;
	}

	APIdFLIMReSync();

	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CdFLIM_4002::SetFineShift(LONG32 shift, int channel)
{
	STAT_STRUCT StatusInfo;

	SHORT shortShift = static_cast<SHORT>(shift) * 4;

	switch (channel)
	{
	case 0:
		if (WriteReadRegister(WRITE_TO_CARD,3,0x320,(BYTE*)(&shortShift),0,2,&StatusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
		break;
	case 1:
		if (WriteReadRegister(WRITE_TO_CARD,3,0x360,(BYTE*)(&shortShift),0,2,&StatusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
		break;
	case 2:
		if (WriteReadRegister(WRITE_TO_CARD,3,0x3A0,(BYTE*)(&shortShift),0,2,&StatusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
		break;
	case 3:
		if (WriteReadRegister(WRITE_TO_CARD,3,0x3E0,(BYTE*)(&shortShift),0,2,&StatusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
		break;
	}

	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CdFLIM_4002::SetDFLIMSyncingSettings(ULONG32 syncDelay, ULONG32 resyncDelay, bool forceResyncEverLine)
{
	USHORT shortShift = static_cast<USHORT>(syncDelay);

	ULONG value = (USHORT)syncDelay | ((USHORT)resyncDelay << 16) | (forceResyncEverLine << 31);

	SetNONI2C_0x1C0(value); 
	SetNONI2C_0x1C4(0x10000011);
	SetNONI2C_0x1C8(0x01);
	SetNONI2C_0x1C8(0x00);

	APIdFLIMReSync();

	return STATUS_SUCCESSFUL;
}

// BackEnd reset of all 4 channels
THORDAQ_STATUS CdFLIM_4002::ResetBackEndAcq()
{
	THORDAQ_STATUS status;
//	STAT_STRUCT StatusInfo;

	LONG backEndReset = 1;
	LONG backEndNorm  = 0;

	status = FPGAregisterWRITE("GPOReg0_Ch0", 1); // assert RESET
	status = FPGAregisterWRITE("GPOReg0_Ch1", 1);
	status = FPGAregisterWRITE("GPOReg0_Ch2", 1);
	status = FPGAregisterWRITE("GPOReg0_Ch3", 1);
	// toggle
	FPGAregisterWRITE("GPOReg0_Ch0", 0); // de-assert RESET
	FPGAregisterWRITE("GPOReg0_Ch1", 0);
	FPGAregisterWRITE("GPOReg0_Ch2", 0);
	FPGAregisterWRITE("GPOReg0_Ch3", 0);

	/*
	if (WriteReadRegister(WRITE_TO_CARD,3,0x300,(BYTE*)(&backEndReset),0,4,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (WriteReadRegister(WRITE_TO_CARD,3,0x340,(BYTE*)(&backEndReset),0,4,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (WriteReadRegister(WRITE_TO_CARD,3,0x380,(BYTE*)(&backEndReset),0,4,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (WriteReadRegister(WRITE_TO_CARD,3,0x3C0,(BYTE*)(&backEndReset),0,4,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (WriteReadRegister(WRITE_TO_CARD,3,0x300,(BYTE*)(&backEndNorm),0,4,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (WriteReadRegister(WRITE_TO_CARD,3,0x340,(BYTE*)(&backEndNorm),0,4,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (WriteReadRegister(WRITE_TO_CARD,3,0x380,(BYTE*)(&backEndNorm),0,4,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (WriteReadRegister(WRITE_TO_CARD,3,0x3C0,(BYTE*)(&backEndNorm),0,4,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}
*/
	return STATUS_SUCCESSFUL;
}


THORDAQ_STATUS CdFLIM_4002::APIdFLIMReSync()
{
	THORDAQ_STATUS status;
	bool bI2Cread = false;  // default is WRITE
	const UINT32 MasterMUXaddr = 0x71, SlaveMUXaddr = 0x70; // fixed by hardware
	UINT32 MasterMUXchan = 2, SlaveMUXchan = 0xFF, SlaveAddress;  // set by caller
	UINT32 TransferBufferLen = 0; // read or write command
	UINT32 OpcodeByteLen = 0; // optional - depends on I2C device protocol
	INT32 PageReadWriteLen = 16; // required-default for EEPROMs
	UINT32 I2CbusHz = 100; // run dFLIM slower than ThorDAQ
	UINT8 OpCodeBuffer[16];
	UINT32 OpCodeLen = 0; // length of optional OpCodes
	UINT8 BiDirDataBuf[128];
	UINT32 BiDirDataLen;
	// recalibrates the VCO (in case the laser freq is very different or was absent before)
	//   and then re-syncs the digitizer

	/////////////////////////////////////////////////////////////////////////////
	//// line 42
	/////////////////////////////////////////////////////////////////////////////
	//if (SetI2C_0x2C0(0x01990000) != STATUS_SUCCESSFUL)
	//{
	//	return STATUS_READWRITE_REGISTER_ERROR;
	//}

	//if (SetI2C_0x2C4(0x000529) != STATUS_SUCCESSFUL)
	//{
	//	return STATUS_READWRITE_REGISTER_ERROR;
	//}

	//if (SetI2C_0x2C8_232() != STATUS_SUCCESSFUL)
	//{
	//	return STATUS_READWRITE_REGISTER_ERROR;
	//}

	//SetdFLIMFrontEndSettings();

	if (1) 
	{
	//	// this section recalibrates the VCO.  But it seems to have created more trouble
	//	// perhaps because the wait after recalibration is not quite long enough

	//	///////////////////////////////////////////////////////////////////////////
	//	// line 41 Set SPI mode
	//	///////////////////////////////////////////////////////////////////////////
	//	SetI2C_0x2C0(0xf0ffff0b);
	//	SetI2C_0x2C4(0x000329);
	//	SetI2C_0x2C8_232();

	//	Sleep(FRONT_END_SETUP_SLEEP1);

		//////////////////////////////////////////////////////////////////////////
		// line 81    Clock chip: 	0x018	0x26	set VCO cal now 0
		///////////////////////////////////////////////////////////////////////////
		SlaveAddress = 0x29;    // I2C-SPI bridge
		OpCodeLen = 4;
		OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
		OpCodeBuffer[1] = 0x0;
		OpCodeBuffer[2] = 0x18;
		OpCodeBuffer[3] = 0x26;
		BiDirDataLen = 0;
		status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
			&OpCodeBuffer[0], &OpCodeLen,
			&BiDirDataBuf[0], &BiDirDataLen);
//		SetI2C_0x2C0(0x01261800);
//		SetI2C_0x2C4(0x000529);
//		SetI2C_0x2C8_232();

		Sleep(FRONT_END_SETUP_SLEEP1);
	
		//////////////////////////////////////////////////////////////////////////
		// line 89    Clock chip: 	0x232	0x01	update all registers
		///////////////////////////////////////////////////////////////////////////
//		SetI2C_0x2C0(0x01013202);
//		SetI2C_0x2C4(0x000529);
//		SetI2C_0x2C8_232();

		Sleep(FRONT_END_SETUP_SLEEP2);

		//////////////////////////////////////////////////////////////////////////
		// line 81    Clock chip: 	0x018	0x27	set VCO cal now 1
		///////////////////////////////////////////////////////////////////////////
		OpCodeLen = 4;
		OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
		OpCodeBuffer[1] = 0x0;
		OpCodeBuffer[2] = 0x18;
		OpCodeBuffer[3] = 0x27;
		BiDirDataLen = 0;
		status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
			&OpCodeBuffer[0], &OpCodeLen,
			&BiDirDataBuf[0], &BiDirDataLen);
//		SetI2C_0x2C0(0x01271800);
//		SetI2C_0x2C4(0x000529);
//		SetI2C_0x2C8_232();

		Sleep(FRONT_END_SETUP_SLEEP3);

		//////////////////////////////////////////////////////////////////////////
		// line 89    Clock chip: 	update all reg
		///////////////////////////////////////////////////////////////////////////
		OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
		OpCodeBuffer[1] = 0x2;
		OpCodeBuffer[2] = 0x31;
		OpCodeBuffer[3] = 0x01;
		BiDirDataLen = 0;
		status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
			&OpCodeBuffer[0], &OpCodeLen,
			&BiDirDataBuf[0], &BiDirDataLen);
//		SetI2C_0x2C0(0x01013202);
//		SetI2C_0x2C4(0x000529);
//		SetI2C_0x2C8_232();

		Sleep(30);
	};

	//////////////////////////////////////////////////////////////////////////
	// ReSync Command
	///////////////////////////////////////////////////////////////////////////
	SetNONI2C_0x1C0(0x00000080);
	SetNONI2C_0x1C4(0x10000010);
	SetNONI2C_0x1C8(0x01);
	SetNONI2C_0x1C8(0x00);

	Sleep(FRONT_END_SETUP_SLEEP2);

	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS GetDACSamplesPerLine(UINT32& samples,double& dac_rate, double line_time)
{
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CdFLIM_4002::GetExternClockStatus(ULONG32& isClockedSynced)
{
	STAT_STRUCT StatusInfo;
	BYTE* buffer_read = new BYTE[1];
	if (WriteReadRegister(READ_FROM_CARD,3,0x180,buffer_read,0,1,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer_read);
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	isClockedSynced = (*buffer_read & 0x10) ? 1 : 0;
	SAFE_DELETE_ARRAY(buffer_read);
	return STATUS_SUCCESSFUL;
}


/* NOT REFERENCED
THORDAQ_STATUS CdFLIM_4002::StartImaging()
{
	STAT_STRUCT StatusInfo;
	// run dma
	BYTE runDMAByte=  0x13;
	if (WriteReadRegister(WRITE_TO_CARD,2,0x000,(BYTE*)(&runDMAByte),0,1,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}
	//To Set GIGCR0_STOP_RUN (RUN)
	BYTE startByte = 0x01;
	if (WriteReadRegister(WRITE_TO_CARD,3,0x00,(BYTE*)(&startByte),0,1,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	return STATUS_SUCCESSFUL;
}
*/

/////////////////////////////////////////////////////////////////////////////////
//  NOTE: Critical configuration of FMC126 is done via the I2C-SPI bridge device:
// 	    Ref:  "SC18IS602B I2C - bus to SPI bridge, Prod. Data Sheet, Rev. 7 — 21 October 2019" NXP Semiconductor
// 	   CONFIG BYTE DEFINITION
// 	   Bit  7		6		5		4		3		2		1		0
// 	       X       X      ORDER     X       M1      M0		F1		F0 
//	F1 : F0 SPI clock rate
//		00 - 1843 kHz
//		01 - 461 kHz
//		10 - 115 kHz
//		11 - 58 kHz
// 	M1 : M0
// 	    00 - SPICLK LOW when idle; data clocked in on leading edge
// 	    01 - SPICLK LOW when idle; data clocked in on trailing edge
//		10 - SPICLK HI when idle; data clocked in on trailing edge
// 	    11 - SPICLK HI when idle; data clocked in on leading edge
// 	ORDER
// 	    0 - MSB of data word transmitted first
// 	    1 - LSB "
//
//  The 3 I2Cslave devices on the "ADC mezz card" (i.e. Master MUX chan 0x02, when FMC126 card resides)
//  0x51: 24LC02B EEPROM
//  0x29: SC18IS602b I2C to SPI Bridge
//  0x4B: ADT7411 Digital Temp Sensor and A/D
//
//  On SPI side of the bridge are 3 slaves:
//  0x01 - AD9517 PLL clock controller
//  0x02 - ADC
//  0x08 - CPLD


THORDAQ_STATUS CdFLIM_4002::SetdFLIMFrontEndSettings()
{
	THORDAQ_STATUS status;
	bool bI2Cread = false;  // default is WRITE
	const UINT32 MasterMUXaddr = 0x71, SlaveMUXaddr = 0x70; // fixed by hardware
	UINT32 MasterMUXchan, SlaveMUXchan, SlaveAddress;  // set by caller
	UINT32 TransferBufferLen = 0; // read or write command
	UINT32 OpcodeByteLen = 0; // optional - depends on I2C device protocol
	INT32 PageReadWriteLen = 16; // required-default for EEPROMs
	UINT32 I2CbusHz = 100; // run dFLIM slower than ThorDAQ
	UINT8 OpCodeBuffer[16]; 
	UINT32 OpCodeLen = 0; // length of optional OpCodes
	UINT8 BiDirDataBuf[128];
	UINT32 BiDirDataLen;

	int mode = 0;
	if (mode == 2)
	{
//		WriteMem 3 0x1C0 1 0x00
//Delay 250
//
/////////////////////////////////////////////////////////////////////////////
//// set max_cnt (#of beats)
/////////////////////////////////////////////////////////////////////////////
//WriteMem 3 0x1C8 4 0x00001824
//WriteMem 3 0x1Cc 4 0x00000000
/*		STAT_STRUCT statusInfo;
		BYTE byte = 0x00;
		if (WriteReadRegister(WRITE_TO_CARD, 3, 0x1C0, (BYTE*)(&byte), 0 , 1, &statusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}

		ULONG mxBeatsCount = 0x00001824;
		if (WriteReadRegister(WRITE_TO_CARD, 3, 0x1C8, (BYTE*)(&mxBeatsCount), 0 , 4, &statusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}

		ULONG mxBeatsCount2 = 0x00001824;
		if (WriteReadRegister(WRITE_TO_CARD, 3, 0x1Cc, (BYTE*)(&mxBeatsCount2), 0 , 4, &statusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}*/
	}
	else 
	{
		MasterMUXchan = 2, SlaveMUXchan = 0xff;
		SlaveAddress = 0x29; // FMC126's CPLD
		// set up SPI device on bridge
		OpCodeLen = 1;
		OpCodeBuffer[0] = 0x8;  // CPLD device select
		// set up command to CPLD
		BiDirDataBuf[0] = 0;     // CPLD's register
		BiDirDataBuf[1] = 0x2B;  // register data CLKSRC Internal Clock, External Reference | SYNCSRC Carrier (through SYNC_FROM_FPGA) | Clock Tree SPI reset
		BiDirDataLen = 2;
		// set SPI  bus speed to 461 KHz
		// set up command to SC18IS602B
		SlaveAddress = 0x29;  // I2C-SPI bridge slave
		OpCodeLen = 2;
		OpCodeBuffer[0] = 0xF0;  // bridge function code, SPI operating mode & freq.
		OpCodeBuffer[1] = 0x09;  // bridge commmand (461 kHz)
		BiDirDataLen = 0;
		status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
			&OpCodeBuffer[0], &OpCodeLen,
			&BiDirDataBuf[0], &BiDirDataLen);

		// set up ThorDAQ_I2C_Mode and perform a !RESET to PCA9548A (I2C MUX master)
//	SetI2C_0x2C8(0x00);
//	SetI2C_0x2C8(0x02);

	///////////////////////////////////////////////////////////////////////////
	// Access channel 1 PCA9548A  (All I2C MUXing is done in API command
	///////////////////////////////////////////////////////////////////////////
//	SetI2C_0x2C0(0xffffff02);
//	SetI2C_0x2C4(0x000271);
//	SetI2C_0x2C8_232();

//	Sleep(FRONT_END_SETUP_SLEEP1);

	// FNC126's CPLD settings (slave addr 0x29 is the I2C-SPI bridge, CPLD select is 0x8)
	// Ref: "Abaco FMC12x User Manual, UM009, Appendix D. CPLD Register Map, pg. 31" 
	///////////////////////////////////////////////////////////////////////////
	// line 33    FMC12x CPLD: 	0x00	0x2B	sets reset bit for SPIclocktree
	///////////////////////////////////////////////////////////////////////////
		// NOTE:  The 
		status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
			&OpCodeBuffer[0], &OpCodeLen,
			&BiDirDataBuf[0], &BiDirDataLen);   
	
	//SetI2C_0x2C0(0x08ff2b00);
	//SetI2C_0x2C4(0x000429);
	//SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 34    FMC12x CPLD: 	0x00	0x0B	clear reset
	///////////////////////////////////////////////////////////////////////////
	// set up command to CPLD
	// set up SPI device on bridge
	OpCodeLen = 1;
	OpCodeBuffer[0] = 0x8;  // CPLD device select
	BiDirDataBuf[0] = 0;     // CPLD's register
	BiDirDataBuf[1] = 0x0B;  // clear SPI reset
	BiDirDataLen = 2;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);

//	SetI2C_0x2C0(0x08ff0b00);
//	SetI2C_0x2C4(0x000429);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);


	///////////////////////////////////////////////////////////////////////////
	// line 35    FMC12x CPLD: 	0x00	0x4B	sets reset bit for SPIadc
	///////////////////////////////////////////////////////////////////////////
		// set up command to CPLD
	// set up SPI device on bridge
	OpCodeLen = 1;
	OpCodeBuffer[0] = 0x8;  // CPLD device select
	BiDirDataBuf[0] = 0;     // CPLD's register
	BiDirDataBuf[1] = 0x4B;  // RESET A/D Device SPI SPI  | SYNCSRC Carrier (through SYNC_FROM_FPGA) | register data CLKSRC Internal Clock, External Reference 
	BiDirDataLen = 2;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
	
//	SetI2C_0x2C0(0x08ff4b00);
//	SetI2C_0x2C4(0x000429);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 36    FMC12x CPLD: 	0x00	0x0B	clear reset and set clock routing: internal with ext ref, sync from host
	///////////////////////////////////////////////////////////////////////////
		// set up command to CPLD
		// set up SPI device on bridge
	OpCodeLen = 1;
	OpCodeBuffer[0] = 0x8;  // CPLD device select
	BiDirDataBuf[0] = 0;     // CPLD's register
	BiDirDataBuf[1] = 0x0B;  // CLEAR A/D Device SPI SPI reset | SYNCSRC Carrier (through SYNC_FROM_FPGA) | register data CLKSRC Internal Clock, External Reference 
	BiDirDataLen = 2;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);

//	SetI2C_0x2C0(0x08ff0b00);
//	SetI2C_0x2C4(0x000429);
//	SetI2C_0x2C8_232();
	
	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 37    FMC12x CPLD: 	0x01	0x00	all fans enabled, all HDMI (frontio) signals are inputs
	///////////////////////////////////////////////////////////////////////////
		// set up command to CPLD
		// set up SPI device on bridge
	OpCodeLen = 1;
	OpCodeBuffer[0] = 0x8;  // CPLD device select
	BiDirDataBuf[0] = 1;     // CPLD's register
	BiDirDataBuf[1] = 0x00;  // FAN1 & FAN0 ON | All Front IO transcievers DIR is INPUT 
	BiDirDataLen = 2;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);

//	SetI2C_0x2C0(0x08ff0001);
//	SetI2C_0x2C4(0x000429);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 38    FMC12x CPLD: 	0x02	0x00	LED off
	// FMC12x User Manual r1.18 pg. 32,CPLD_REG2 WRITE lower 5 bit config for 
	// PGOOD LED source:
	// 00000 - undefined
	// xxxx1 - AD9517 REFMON output 
	// xxx10 - AD9517 LD (lock detect) output
	// xx100 - AD9517 STATUS output
	// x1000 - ADT7411 VM INT# output (inverted)
	// 10000 - CPLD logic INT
	///////////////////////////////////////////////////////////////////////////
	// set up command to CPLD
	OpCodeLen = 1;
	OpCodeBuffer[0] = 0x8;  // CPLD device select
	BiDirDataBuf[0] = 2;     // CPLD's register
	BiDirDataBuf[1] = 0x10;  // see logic table above 
	BiDirDataLen = 2;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);

//	SetI2C_0x2C0(0x08ff1002);
//	SetI2C_0x2C4(0x000429);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);


	/////////////////////////////////////////////////////////////
	//  7	6	5	4		3		2		1		0
	// reserved    IRQ     VM    STATUS	   LD    REFMON
	// 
	// set up command to CPLD
	// READ the status bits in REG2 - because we are going through I2C <-> SPI bridge, "write" the read command, then "read"
/*	bI2Cread = true;  // attempt to get CPLD to respond by writing to bridge buffer
	OpCodeLen = 2;    // see Bridge read example
	OpCodeBuffer[0] = 0x8;  // CPLD device select
	OpCodeBuffer[1] = 0x2;  // CPLD's register
	BiDirDataBuf[0] = 0xAA; // initialize for CPLD data overwrite
	BiDirDataLen = 3;  // we must issue separate command to read BRIDGE's buffer
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
*/

	///////////////////////////////////////////////////////////////////////////
	// line 41 Set I2C-SPI bridge (slave 0x29) mode
	///////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//  Ref:  Analog Devices 12-Output Clock Generator with Integrated 2.8 GHz VCO, Data Sheet Rev F.
	// NOTE: Pg. 52 of 80 shows MSB first is DEFAULT for AD9517 chip, so select that in I2C-SPI Bridge
	// 
	// 

	// set up command to SC18IS602B
	SlaveAddress = 0x29;  // I2C-SPI bridge slave
	OpCodeLen = 2;
	OpCodeBuffer[0] = 0xF0;  // bridge register
	OpCodeBuffer[1] = 0x0A;  // MSB fist (def.) | Mode SPICLK HIGH on idle | SPI speed 115 kHz 
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0xf0ffff0b);
//	SetI2C_0x2C4(0x000329);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 42	Clock chip:		0x00	0x99	Set AD9517 in 4-wire mode (SDO enabled)
	///////////////////////////////////////////////////////////////////////////
	bI2Cread = false; // start series of write-configs to AD9517	
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0x0;  
	OpCodeBuffer[3] = 0x99;  
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);

//	SetI2C_0x2C0(0x01990000);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 43   Clock chip:	0x10	0x7C	CP 4.8mA, normal op
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0x10;
	OpCodeBuffer[3] = 0x7c;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
	
//	SetI2C_0x2C0(0x017c1000);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 44    Clock chip: 	0x11	0x01	R lo
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0x11;
	OpCodeBuffer[3] = 0x01;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01011100);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 45    Clock chip: 	0x12	0x00	R hi
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0x12;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01001200);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 46    Clock chip: 	0x13	0x02	A
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0x13;
	OpCodeBuffer[3] = 0x02;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01021300);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 47    Clock chip: 	0x14	0x0F	B lo
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0x14;
	OpCodeBuffer[3] = 0x0F;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x010f1400);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 48    Clock chip: 	0x15	0x00	B hi
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0x15;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01001500);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 49    Clock chip: 	0x16	0x06	Prescaler dual modulo 32
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0x16;
	OpCodeBuffer[3] = 0x06;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01061600);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 50    Clock chip: 	0x17	0xB4	Status = DLD
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0x17;
	OpCodeBuffer[3] = 0xB4;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01b41700);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 51    Clock chip: 	0x19	0x00	No SYNC pin reset of dividers
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0x19;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01001900);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 52    Clock chip: 	0x1A	0x00	LD = DLD
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0x1A;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01001a00);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 53    Clock chip: 	0x1B	0x00	REFMON = GND
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0x1B;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01001b00);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 54    Clock chip: 	0x1C	0x87	Diff ref input
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0x1c;
	OpCodeBuffer[3] = 0x87;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01871c00);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 55    Clock chip: 	0x1D	0x00	PLL control refs off
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0x1D;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01001d00);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(205);

	//////////////////////////////////////////////////////////////////////////
	// line 56    Clock chip: 	0x0F0	0x02	out0, safe power down
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0xF0;
	OpCodeBuffer[3] = 0x02;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x0102f000);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 57    Clock chip: 	0x0F1	0x0C	out1, lvpecl 960mW
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0xF1;
	OpCodeBuffer[3] = 0x0C;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x010cf100);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 58    Clock chip: 	0x0F4	0x02	out2, safe power down
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0xF4;
	OpCodeBuffer[3] = 0x02;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x0102f400);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 59    Clock chip: 	0x0F5	0x0C	out3, adc, lvpecl 960mW
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0xF5;
	OpCodeBuffer[3] = 0x0C;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x010cf500);
//  SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 60    Clock chip: 	0x140	0x01	out4, sync, pd
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x40;
	OpCodeBuffer[3] = 0x01;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01014001);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 61    Clock chip: 	0x141	0x01	out5, pd
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x41;
	OpCodeBuffer[3] = 0x01;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01014101);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 62    Clock chip: 	0x142	0x00	out6, lvds 1.75mA
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x42;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01004201);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 63    Clock chip: 	0x143	0x01	out7, pd
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x43;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01014301);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 64    Clock chip: 	0x190	0xBC	div0, clk out, /50 (50MHz) :  clk out (external clock output) no longer used 
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x90;
	OpCodeBuffer[3] = 0xBC;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01bc9001);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();
	
	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 65    Clock chip: 	0x191	0x80	div0, clk out, bypass
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x91;
	OpCodeBuffer[3] = 0x80;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01809101);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 66    Clock chip: 	0x192	0x02	div0, clk out, clk to output
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x92;
	OpCodeBuffer[3] = 0x02;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01029201);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 67    Clock chip: 	0x196	0x22	div1, adc, /2
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x96;
	OpCodeBuffer[3] = 0x22;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01229601);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 68    Clock chip: 	0x197	0x80	div1, adc, divider bypassed
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x97;
	OpCodeBuffer[3] = 0x80;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01809701);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 69    Clock chip: 	0x198	0x02	div1, adc, clk to output
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x98;
	OpCodeBuffer[3] = 0x02;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01029801);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 70    Clock chip: 	0x199	0x00	div2.1, /2
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x99;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01009901);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 71    Clock chip: 	0x19A	0x00	phase
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x9A;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01009a01);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 72    Clock chip: 	0x19B	0x00	div2.2, /2
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x9B;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01009b01);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 73    Clock chip: 	0x19C	0x00	div2.1 on, div2.2 on
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x9C;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01009c01);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 74    Clock chip: 	0x19D	0x00	div2 dcc on
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x9D;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01009d01);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 75    Clock chip: 	0x19E	0x00	div3.1, /2
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x9E;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01009e01);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 76    Clock chip: 	0x19F	0x00	phase
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x9F;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01009f01);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 77    Clock chip: 	0x1A0	0x00	div3.2, /2
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0xA0;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x0100a001);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 78    Clock chip: 	0x1A1	0x00	div3.1 on, div3.2 on
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0xA1;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x0100a101);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 79    Clock chip: 	0x1A2	0x00	div3 dcc on
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0xA2;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x0100a201);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 80    Clock chip: 	0x1E0	0x00	vco div /2
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0xE0;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x0100e001);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 81    Clock chip: 	0x018	0x26	set VCO cal now 0
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0x18;
	OpCodeBuffer[3] = 0x26;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01261800);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 82    Clock chip: 	0x1E1	0x02	use internal vco and vco divider
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0xE1;
	OpCodeBuffer[3] = 0x02;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x0102e101);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 83    Clock chip: 	0x19E	0x00	div3.1 /2
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x9E;
	OpCodeBuffer[3] = 0x02;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01009e01);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 84    Clock chip: 	0x19F	0x00	phase
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0x9F;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01009f01);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 85    Clock chip: 	0x1A0	0x00	div3.2 /2
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0xA0;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x0100a001);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 86    Clock chip: 	0x1A1	0x20	div3.1 on, div3.2 BYPASS (gy for Drift; was 0x00 for both dividers on)
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0xA1;
	OpCodeBuffer[3] = 0x20;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x0120a101);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 87    Clock chip: 	0x1A2	0x00	div3 dcc on
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x1;
	OpCodeBuffer[2] = 0xA2;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x0100a201);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 88    Clock chip: 	0x230	0x00	no pwd, no sync
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x2;
	OpCodeBuffer[2] = 0x30;
	OpCodeBuffer[3] = 0x00;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01003002);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 89    Clock chip: 	0x232	0x01	update all registers
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x2;
	OpCodeBuffer[2] = 0x32;
	OpCodeBuffer[3] = 0x01;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01013202);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP2);

	//////////////////////////////////////////////////////////////////////////
	// line 81    Clock chip: 	0x018	0x27	set VCO cal now 1
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x0;
	OpCodeBuffer[2] = 0x18;
	OpCodeBuffer[3] = 0x27;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01271800);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 89    Clock chip: 	update all reg
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 4;
	OpCodeBuffer[0] = 0x1;  // function-code: AD9517 device select
	OpCodeBuffer[1] = 0x2;
	OpCodeBuffer[2] = 0x32;
	OpCodeBuffer[3] = 0x01;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x01013202);
//	SetI2C_0x2C4(0x000529);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP2);



	//////////////////////////////////////////////////////////////////////////
	// line 93 Set SPI mode
	///////////////////////////////////////////////////////////////////////////
	// set up command to SC18IS602B
	SlaveAddress = 0x29;  // I2C-SPI bridge slave
	OpCodeLen = 2;
	OpCodeBuffer[0] = 0xF0;  // bridge register
	OpCodeBuffer[1] = 0x03;  // MSB fist (def.) | Mode SPICLK LOW on idle | SPI speed 58 kHz 
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);	
//	SetI2C_0x2C0(0xf0ffff03);
//	SetI2C_0x2C4(0x000329);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 94 non-I2C:  set the IDELAY for the Trigger
	///////////////////////////////////////////////////////////////////////////
	//SetNONI2C_0x1C0(0x00000000);  // sets IDELAY and SyncDelay - see register docs for FrontEnd register 0x0000011
	//SetNONI2C_0x1C4(0x10000011);
	//SetNONI2C_0x1C8(0x01);
	//SetNONI2C_0x1C8(0x00);


	//Sleep(FRONT_END_SETUP_SLEEP1);

	////////////////////////////////////////////////////////////////////////////
	//// line 95	EV10 chip:	0x05	0x01	Set flashing pattern for test.   GY: Need to check these addresses?
	/////////////////////////////////////////////////////////////////////////////
	//SetI2C_0x2C0(0x02010085);
	//SetI2C_0x2C4(0x000529);
	//SetI2C_0x2C8_232();

	//Sleep(FRONT_END_SETUP_SLEEP1);

	////////////////////////////////////////////////////////////////////////////
	//// line 96	EV10 chip:	0x01	0x0100	Set test mode on.   GY: Need to check these addresses/values ?
	/////////////////////////////////////////////////////////////////////////////
	//SetI2C_0x2C0(0x02401081);
	//SetI2C_0x2C4(0x000529);
	//SetI2C_0x2C8_232();


	///////////////////////////////////////////////////////////////////////////
	// line 97 non-I2C:	FrontEnd register 0x0000010: reset the clock buffer and IDELAYs
	///////////////////////////////////////////////////////////////////////////
	SetNONI2C_0x1C0(0x00000003);
	SetNONI2C_0x1C4(0x10000010);
	SetNONI2C_0x1C8(0x01);
	SetNONI2C_0x1C8(0x00);

	Sleep(FRONT_END_SETUP_SLEEP2);

	/////////////////////////////////////////////////////////////////////////////
	//// line 98 non-I2C:	FrontEnd register 0x0000010: request firmware automatic phasing of inputs
	/////////////////////////////////////////////////////////////////////////////
	//SetNONI2C_0x1C0(0x00000008);
	//SetNONI2C_0x1C4(0x10000010);
	//SetNONI2C_0x1C8(0x01);
	//SetNONI2C_0x1C8(0x00);

	//Sleep(FRONT_END_SETUP_SLEEP1);

	////////////////////////////////////////////////////////////////////////////
	//// line 99	EV10 chip:	0x01	0x0000	Set test mode off.   GY: Need to check these addresses?
	/////////////////////////////////////////////////////////////////////////////
	//SetI2C_0x2C0(0x02400081);
	//SetI2C_0x2C4(0x000529);
	//SetI2C_0x2C8_232();

	//Sleep(FRONT_END_SETUP_SLEEP2);

	Sleep(FRONT_END_SETUP_SLEEP3);

	///////////////////////////////////////////////////////////////////////////
	// line 100 non-I2C:	FrontEnd register 0x0000010: request firmware to generate ADC SYNC
	///////////////////////////////////////////////////////////////////////////
	SetNONI2C_0x1C0(0x00000080);
	SetNONI2C_0x1C4(0x10000010);
	SetNONI2C_0x1C8(0x01);
	SetNONI2C_0x1C8(0x00);

	Sleep(FRONT_END_SETUP_SLEEP2);

	//////////////////////////////////////////////////////////////////////////
	// line 104  Temperature monitor setup ????? GY ?????
	///////////////////////////////////////////////////////////////////////////
	// set up command to ADT7411 (slave 0x4B) 
	SlaveAddress = 0x4B;  // slave
	OpCodeLen = 2;
	OpCodeBuffer[0] = 0x18;  
	OpCodeBuffer[1] = 0x2D;   
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x18ffff2d);
//	SetI2C_0x2C4(0x00034b);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 105  Temperature monitor setup ????? GY ?????
	///////////////////////////////////////////////////////////////////////////
	OpCodeLen = 2;
	OpCodeBuffer[0] = 0x1A;
	OpCodeBuffer[1] = 0x10;
	BiDirDataLen = 0;
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
//	SetI2C_0x2C0(0x1affff10);
//	SetI2C_0x2C4(0x00034b);
//	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 106  Temperature monitor read ????? GY ?????
	///////////////////////////////////////////////////////////////////////////
/*	bI2Cread = true;
	SlaveAddress = 0x4B;  // slave
	OpCodeLen = 1;  
	OpCodeBuffer[0] = 0x07; // read starting at 0x07
	BiDirDataLen = 1;  // read 1 byte
	status = APIXI2CReadWrite(*this, bI2Cread, MasterMUXaddr, MasterMUXchan, SlaveMUXaddr, SlaveMUXchan, SlaveAddress, I2CbusHz, PageReadWriteLen,
		&OpCodeBuffer[0], &OpCodeLen,
		&BiDirDataBuf[0], &BiDirDataLen);
	SetI2C_0x2C0(0xffffff07);
	SetI2C_0x2C4(0x0142cb);
	SetI2C_0x2C8_232();
*/

	Sleep(FRONT_END_SETUP_SLEEP1);

//	ReadI2C_0x2C0(1);

	//////////////////////////////////////////////////////////////////////////
	// line 107  Temperature monitor read ????? GY ?????
	///////////////////////////////////////////////////////////////////////////
//	SetI2C_0x2C0(0xffffff08);
//	SetI2C_0x2C4(0x0142cb);
//	SetI2C_0x2C8_232();


//	Sleep(FRONT_END_SETUP_SLEEP1);

//	ReadI2C_0x2C0(1);

	///////////////////////////////////////////////////////////////////////////
	// verify   Checking phasing of inputs ????? GY ????? not used
	///////////////////////////////////////////////////////////////////////////
	SetNONI2C_0x1C0(0x00000000);
	SetNONI2C_0x1C4(0x20000010);
	SetNONI2C_0x1C8(0x01);
	SetNONI2C_0x1C8(0x00);

	Sleep(FRONT_END_SETUP_SLEEP1);

	ReadNONI2C_0x1C0(8);

	Sleep(FRONT_END_SETUP_SLEEP1);

	// later this (and coarseShifts, fineShifts, etc.) should be loaded from a config file and set separately for each channel

	///////////////////////////////////////////////////////////////////////////
	// Delay val for SYNC signal (IDELAY giving output SYNC delay)
	///////////////////////////////////////////////////////////////////////////
	SetNONI2C_0x1C0(0x00000003); // value is here
	SetNONI2C_0x1C4(0x10000011);
	SetNONI2C_0x1C8(0x01);
	SetNONI2C_0x1C8(0x00);

	//SetCoarseShift(3); 
	//SetFineShift(-24);

	Sleep(FRONT_END_SETUP_SLEEP1);

	
	}
	return STATUS_SUCCESSFUL;
}


THORDAQ_STATUS CdFLIM_4002::SetNONI2C_0x1C0(ULONG bytes)
{
	STAT_STRUCT statusInfo;
	BYTE * buffer = new BYTE[4];  //0x00;
	memcpy(buffer,&bytes,sizeof(UCHAR)*4);
	if (WriteReadRegister(WRITE_TO_CARD, 3, 0x1C0, buffer, 0 , 4, &statusInfo) != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		return STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CdFLIM_4002::SetNONI2C_0x1C4(ULONG bytes)
{
	STAT_STRUCT statusInfo;
	BYTE * buffer = new BYTE[4];  //0x00;
	memcpy(buffer,&bytes,sizeof(UCHAR)*4);
	if (WriteReadRegister(WRITE_TO_CARD, 3, 0x1C4, buffer, 0 , 4, &statusInfo) != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		return STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CdFLIM_4002::SetNONI2C_0x1C8(BYTE byte)
{
	STAT_STRUCT statusInfo;
	BYTE * buffer = new BYTE[1];  //0x00;
	memcpy(buffer,&byte,sizeof(UCHAR));
	if (WriteReadRegister(WRITE_TO_CARD, 3, 0x1C8, buffer, 0 , 1, &statusInfo) != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		return STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CdFLIM_4002::SetI2C_0x2C0(ULONG bytes)
{
	STAT_STRUCT statusInfo;
	BYTE * buffer = new BYTE[4];  //0x00;
	memcpy(buffer,&bytes,sizeof(UCHAR)*4);
	if (WriteReadRegister(WRITE_TO_CARD, 3, 0x2C0, buffer, 0 , 4, &statusInfo) != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		return STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CdFLIM_4002::SetI2C_0x2C4(ULONG bytes)
{
	STAT_STRUCT statusInfo;
	BYTE * buffer = new BYTE[3];  //0x00;
	memcpy(buffer,&bytes,sizeof(UCHAR)*3);
	if (WriteReadRegister(WRITE_TO_CARD, 3, 0x2C4, buffer, 0 , 3, &statusInfo) != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		return STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CdFLIM_4002::SetI2C_0x2C8(BYTE byte)
{
	STAT_STRUCT statusInfo;
	BYTE * buffer = new BYTE[1];  //0x00;
	memcpy(buffer,&byte,sizeof(UCHAR));
	if (WriteReadRegister(WRITE_TO_CARD, 3, 0x2C8, buffer, 0 , 1, &statusInfo) != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		return STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return STATUS_SUCCESSFUL;
}


THORDAQ_STATUS CdFLIM_4002::SetI2C_0x2C8_232()
{
	if (SetI2C_0x2C8(0x02) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (SetI2C_0x2C8(0x03) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (SetI2C_0x2C8(0x02) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CdFLIM_4002::ReadI2C_0x2C0(ULONG length)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	BYTE* buffer = new BYTE[length];
	STAT_STRUCT StatusInfo;
	status = WriteReadRegister(READ_FROM_CARD,3,0x2c0,buffer,0,length,&StatusInfo);
	if (status != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		status = STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return status;
}

THORDAQ_STATUS CdFLIM_4002::ReadNONI2C_0x1C0(ULONG length)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	BYTE* buffer = new BYTE[length];
	STAT_STRUCT StatusInfo;
	status = WriteReadRegister(READ_FROM_CARD,3,0x1c0,buffer,0,length,&StatusInfo);
	if (status != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		status = STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return status;
}

void CdFLIM_4002::LogMessage(wchar_t* logMsg, long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

//THORDAQ_STATUS status = STATUS_SUCCESSFUL;
//	BYTE* buffer = new BYTE[2];
//	memcpy(buffer,&frame_count,sizeof(USHORT));
//	STAT_STRUCT StatusInfo;
//	status = WriteReadRegister(READ_FROM_CARD,3,0x142,buffer,0,2,&StatusInfo);
//	memcpy(&frame_count,buffer,sizeof(USHORT));
//	if (status != STATUS_SUCCESSFUL)
//	{
//		status = STATUS_READWRITE_REGISTER_ERROR;
//	}
//	SAFE_DELETE_ARRAY(buffer);
//	return status;

template <class T>
string to_string(T t, ios_base & (*f)(ios_base&))
{
  ostringstream oss;
  oss << f << t;
  return oss.str();
}


THORDAQ_STATUS CdFLIM_4002::APIProgressiveScan(BOOL bProgressiveScan)
{
	THORDAQ_STATUS          status = STATUS_SUCCESSFUL;
	_bProgressiveScan = bProgressiveScan;
	return status;
}

//-------------------------------------------------------------------------
// User Interrupts
//-------------------------------------------------------------------------
/*! UserIRQWait  [CDmaDriverDll::UserIRQWait()]
 *
 * \brief Waits for the given IRQMask bits interrupt or times out waiting.
   SUCCESS means an interrupt was received
   STATUS_IO_TIMEOUT means the IrqWaitCancel() call was made, which is the way
   we exit the task which runs forever looking for ThorDAQ IRQs
 *  [The dwIRQMask is given as the interrupt(s) to wait for. ??]
 *  [The dwIRQStatus is the returned value of the User Interrupt Control Register. ??]
 *  the dwTimeoutMilliSec is the time out value in ms, 0 = no timeout.
 * \param dwTimeoutMilliSec
 * \note [This call uses the BarNum and Offset used in the UserIRQEnable function
 * \return status. ???]
 */
static UINT64 IRQwaitIterations = 0; // TEST counter
inline THORDAQ_STATUS CdFLIM_4002::UserIRQWait(USER_IRQ_WAIT_STRUCT* pUsrIrqWaitStruct, UINT64* IrqCnt0, UINT64* IrqCnt1, UINT64* IrqCnt2, UINT64* IrqCnt3)
{
	OVERLAPPED              os;         // OVERLAPPED structure for the operation
	DWORD                   bytesReturned = 0;
	DWORD                   LastErrorStatus = 0;
	THORDAQ_STATUS          status = STATUS_SUCCESSFUL;
	BOOL                    bStatus;
	UINT64* IrqCounters[4];

	os.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// USER IRQ IOCTL

	bStatus = DeviceIoControl(gHdlDevice,
		USER_IRQ_WAIT_IOCTL,               // Buffered I/O -- kernel receives data, returns data
		pUsrIrqWaitStruct,                 // "input" to kernel driver
		sizeof(USER_IRQ_WAIT_STRUCT),
		pUsrIrqWaitStruct,                 // "output" from kernel driver
		sizeof(USER_IRQ_WAIT_STRUCT),
		&bytesReturned,
		//		(LPOVERLAPPED)NULL); // blocking
		&os);      // non-blocking

	if (!bStatus)
	{
		LastErrorStatus = GetLastError();
		if (LastErrorStatus == ERROR_IO_PENDING)
		{
			// Wait here (forever) for the Overlapped I/O to complete
			if (!GetOverlappedResult(gHdlDevice, &os, &bytesReturned, TRUE))
			{
				//printf("UserIRQWait OverlappedResult call failed.\n");
				status = (THORDAQ_STATUS)GetLastError();
			}
		}
		else
		{
			// ioctl failed
#if _DEBUG
			printf("UserIRQWait IOCTL call failed. Error = 0x%x\n", LastErrorStatus);
#endif // _DEBUG
			status = (THORDAQ_STATUS)LastErrorStatus;
		}
	}
	int Chan;
	UINT S2MM_DMAactiveISRmask = 0;
	THORDAQ_STATUS rearmStatus;
	UINT64 value;

	IrqCounters[0] = IrqCnt0; // S2MM FPGA interrupt counters per chan
	IrqCounters[1] = IrqCnt1;
	IrqCounters[2] = IrqCnt2;
	IrqCounters[3] = IrqCnt3;

/*
	// For performance, Do NOT read the 13 separate bit fields
	// 1. Read entire DACWaveGenISR register
	// 2. Re-Arm Wave IRQs
	// 3. Increment separate counters according to ISR
NWL_Common_DMA_Register_Block//	value = FPGAregisterReadDoMem(_DACWaveGenISRIndex);  // (invalid for dFLIM)
	// do we have a potential "race" condition here?  Once we take snapshot read, is there
	// a chance the FPGA will set another "sticky" bit 5 nanosecs after we read,
	// which means the follow "rearm" will clear that fresh bit before we read it?
	//rearmStatus = FPGAregisterWRITE("DAC_Waveplay_IRQ_rearm", 0);  // set to 0 after 1 as per SWUG // this could be set right before rearming instead

	// now update Waveform IRQ counters...
	//value >>= 48; // DAC Waveform Chan0 ISR bit at bit location 48


	USHORT isr = value >> 48;

	if (isr)
	{
//		rearmStatus = DACReArmWaveplayIRQ(); // used in dFLIM? clears FPGA's ISR, but not our Shadow bit fields
		for (Chan = 0; Chan < WAVETABLE_CHANNEL_COUNT; ++Chan)
		{
			if ((isr & (0x0001 << Chan)) != 0x0000)
			{
				++_DACWaveGenInterruptCounter[Chan];
				break; //  lets assume we are only tracking one channel
			}
		}
	}

	//	Sleep(1);  // for debug
*/

	// what are the states of "Interrupt Generated" (bit4) S2MM DMA status reg?
	for (Chan = 0; Chan < MAX_CHANNEL_COUNT; Chan++)
	{
		value = 0x10 & FPGAregisterReadDoMem(_S2MMDMAStatusRegIndex[Chan]);
		if (value)
		{
			S2MM_DMAactiveISRmask |= (1 << Chan);
			++(*IrqCounters[Chan]);
		}
	}
	// if ANY of the image DMA INTs set, increment bank counter because a 
	// image DDR3 memory bank switch occurred
	if (S2MM_DMAactiveISRmask)
	{
		// Interrupt happened!  Increment the FPGA DMA bank counter (LS Bit is the bank)
		++(pUsrIrqWaitStruct->DMA_Bank);
	}
	else
	{
		if (os.hEvent != 0)
		{
			CloseHandle(os.hEvent);
		}
		return status;
	}
	//TODO: Remove after testing level trigger functionality with the new location to check the trigger
	//else if(0 == isr)
	//{
	//	FPGAregisterRead("hw_trig_irq", (int)strlen(S2MMirqStatus[Chan]), &value);

	//	if (0x1 == value)
	//	{
	//		_imagingLevelTriggerWentLow = true;
	//	}
	//}

	// re-arm the S2MM DMA Interrupts 
	// (See FPGA SWUG, "Re-Arm Generation of Host System Interrupt)
	// (we observe that re-arming only first chan re-arms all channels - verify)
	for (Chan = 0; Chan < MAX_CHANNEL_COUNT; Chan++)
	{
		if (S2MM_DMAactiveISRmask & (1 << Chan))
		{
			rearmStatus = FPGAregisterWrite(_S2MMDMA_IRQ_REARM_RegIndex[Chan], 0x53); // clear the IRQ (bit 6), other bits remain set
		}
	}

	if (os.hEvent != 0)
	{
		CloseHandle(os.hEvent);
	}
	IRQwaitIterations++;
	return status;
}
/*! UserIRQCancel
 *
 * \brief Sends USER_IRQ_CANCEL_IOCTL kernel call to kill the UserInt timer
 * \return IOCTL status
 * \note This call causes the FPGA interrupt processing thread to exit
 */
THORDAQ_STATUS CdFLIM_4002::UserIRQCancel(VOID)
{
	OVERLAPPED              os, osDMAreset;         // OVERLAPPED structure for IOCTL operation
	DWORD                   bytesReturned = 0;
	DWORD                   LastErrorStatus = 0;
	THORDAQ_STATUS                  status = STATUS_SUCCESSFUL;
	//	DMA_ENGINE_RESET	EngineReset;

	os.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	osDMAreset.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);


	// Send User IRQ Cancel message
	if (!DeviceIoControl(gHdlDevice, USER_IRQ_CANCEL_IOCTL,
		NULL, 0,
		NULL, 0,
		&bytesReturned, &os))
	{
		LastErrorStatus = GetLastError();
		if (LastErrorStatus == ERROR_IO_PENDING)
		{
			// Wait here (forever) for the Overlapped I/O to complete
			if (!GetOverlappedResult(gHdlDevice, &os, &bytesReturned, TRUE))
			{
				// DBG : printf("UserIRQCancel OverlappedResult call failed.\n");
				status = (THORDAQ_STATUS)GetLastError();
			}
		}
		else
		{
			// ioctl failed
			// DBG : printf("UserIRQWait IOCTL call failed. Error = 0x%x\n", LastErrorStatus);
			status = (THORDAQ_STATUS)LastErrorStatus;
		}
	}
	if (os.hEvent != 0)
		CloseHandle(os.hEvent);
	/*
		EngineReset.boardNum = 0;
		EngineReset.engineIndex = 32;
		// Send DMA engine reset
		if (!DeviceIoControl(gHdlDevice, RESET_DMA_ENGINE_IOCTL,
			&EngineReset, sizeof(DMA_ENGINE_RESET),
			NULL, 0,
			&bytesReturned, &osDMAreset))
		{
			LastErrorStatus = GetLastError();
			if (LastErrorStatus == ERROR_IO_PENDING)
			{
				// Wait here (forever) for the Overlapped I/O to complete
				if (!GetOverlappedResult(gHdlDevice, &osDMAreset, &bytesReturned, TRUE))
				{
					// DBG : printf("UserIRQCancel OverlappedResult call failed.\n");
					status = (THORDAQ_STATUS)GetLastError();
				}
			}
			else
			{
				// ioctl failed
				// DBG : printf("UserIRQWait IOCTL call failed. Error = 0x%x\n", LastErrorStatus);
				status = (THORDAQ_STATUS)LastErrorStatus;
			}
		}
		if (osDMAreset.hEvent != 0)
			CloseHandle(osDMAreset.hEvent);
	*/

	return status;
}
// End User Interrupt support

