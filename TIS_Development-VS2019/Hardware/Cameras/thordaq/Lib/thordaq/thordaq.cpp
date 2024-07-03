/*++

Copyright (c) Thorlabs, Inc.  All rights reserved.

Module Name: thordaq.cpp


Abstract:

	Defines the API functions for the thordaq driver.

Environment:

	user mode only.

Style:
	Google C++ coding style.
Note:
	"Wisely and slow; they stumble that run fast". - (William Shakespeare  Romeo and Juliet Act II, Scene III).
--*/

#include "stdafx.h"
#pragma warning(disable:4201) //I just wanna use freedom nonstandard "nameless struct/union"
#include "thordaq.h"
#include "thordaqguid.h"


wchar_t CThordaq::thordaqLogMessage[MSG_SIZE];

// allocate Xilinx I2C controller through AIX bridge
static XI2Ccontroller* XI2C;

// allocate for known (legacy Master) I2C devices (see latest SWUG "I2C Network" diagram)
//static I2CDevice* MainBrdEEPROM;
static I2C_BBox_ChipPCA9554* DBB1_I2C;
static I2C_BBox_ChipPCA9554* ABB1_I2C;
static I2C_BBox_ChipPCA9554* ABB2_I2C;
static I2C_BBox_ChipPCA9554* ABB3_I2C;
static BreakoutBoxLEDs DBB1;
static BreakoutBoxLEDs ABB1;
static BreakoutBoxLEDs ABB2;
static BreakoutBoxLEDs ABB3;
static BreakoutBoxMAX127AI BB_MAX127;



// kernel driver fixing NWL DMA S/G 1 MB len boundary bug was 6.1.0
#define MIN_DRIVER_VERSION_MAJOR 6
#define MIN_DRIVER_VERSION_MINOR 1
#define MIN_DRIVER_VERSION_SUBMINOR 0

/**********************************************************************************************//**
 * @fn	CThordaq::CThordaq ( UINT boardNum )
 *
 * @brief	Default constructor.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param	boardNum	Index of thordaq board.
 **************************************************************************************************/
CThordaq::CThordaq(UINT boardNum, HANDLE kernelDevHandle)
{
	gBoardIndex = boardNum;
	gPCIeBoardIndex = boardNum; // public variable
	gDeviceInfo = NULL;
	gHdlDevice = kernelDevHandle; // from dllmain.cpp discovery of hardware (where constructor called)
	//	gDeviceInterfaceDetailData = NULL;
	gDmaInfo.PacketRecvEngineCount = 0;
	gDmaInfo.PacketSendEngineCount = 0;
	gDmaInfo.AddressablePacketMode = false;
	gPtrAcqCtrl = new DATA_ACQ_CTRL_STRUCT(); //Zero initialize a dynamically allocated thordaq acquisition struct.
	memset(gPtrAcqCtrl, 0, sizeof(DATA_ACQ_CTRL_STRUCT));
	filterFactory = new FilterFactory(FilterType::FIR);
	filter = filterFactory->getFilterFactory();
	filter->initializeFilterTable();
	_auxDigitalOutToggleVal = 0;

	// storage for kernel driver's PCI/card hardware data at time of object instantiation 
	memset((void*)&Board_CONFIG_data, 0, sizeof(BOARD_INFO_STRUCT));

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

	// DZ SDK -- initialize the 3U BOB DIO configuration with default (support legacy BBoxes)
	// the 3U BOB's CPLD power-on default is all ports as INPUTSs, sourced from their own Index
	// the ADC (high speed) DIO power-on default (via TD FPGA) is all ports INPUTs
	// see TD_DIO_MUXedSLAVE_PORTS in "SharedEnums.cs" for MUX codes
	int iDIOindex;
	BOB_DIOconfiguration = new char[TD_BOBDIODef::NumBOB_DIOs * CharPerBOB_DIO];
	// Set Legacy default:  DBB1 DIO5, DIO6 (3U BOB D4, D5) are Inputs
	snprintf(&CThordaq::BOB_DIOconfiguration[0 * TD_BOBDIODef::CharPerBOB_DIO], TD_BOBDIODef::CharPerBOB_DIO, "D00O00M%-2.2dA-1", DO_2_WaveMUX);
	BOB_DIOSettings[0].MUX = DO_2_WaveMUX;
	BOB_DIOSettings[0].bOutputDir = TRUE;
	BOB_DIOSettings[0].OUTvalue = 0;

	snprintf(&CThordaq::BOB_DIOconfiguration[1 * TD_BOBDIODef::CharPerBOB_DIO], TD_BOBDIODef::CharPerBOB_DIO, "D01O01M%-2.2dA-1", DO_3_WaveMUX);
	BOB_DIOSettings[1].MUX = DO_3_WaveMUX;
	BOB_DIOSettings[1].bOutputDir = TRUE;
	BOB_DIOSettings[1].OUTvalue = 0;
	snprintf(&CThordaq::BOB_DIOconfiguration[2 * TD_BOBDIODef::CharPerBOB_DIO], TD_BOBDIODef::CharPerBOB_DIO, "D02O02M%-2.2dA-1", DO_Pixel_Clock);
	BOB_DIOSettings[2].MUX = DO_Pixel_Clock;
	BOB_DIOSettings[2].bOutputDir = TRUE;
	BOB_DIOSettings[2].OUTvalue = 0;
	snprintf(&CThordaq::BOB_DIOconfiguration[3 * TD_BOBDIODef::CharPerBOB_DIO], TD_BOBDIODef::CharPerBOB_DIO, "D03O03M%-2.2dA-1", DO_0_WaveMUX);
	BOB_DIOSettings[3].MUX = DO_0_WaveMUX;
	BOB_DIOSettings[3].bOutputDir = TRUE;
	BOB_DIOSettings[3].OUTvalue = 0;

	snprintf(&CThordaq::BOB_DIOconfiguration[4 * TD_BOBDIODef::CharPerBOB_DIO], TD_BOBDIODef::CharPerBOB_DIO, "D04I04M%-2.2dA-1", TD_DIO_MUXedSLAVE_PORTS::DI_ResonantScanner_Line_Feedback);
	BOB_DIOSettings[4].MUX = TD_DIO_MUXedSLAVE_PORTS::DI_ResonantScanner_Line_Feedback;
	BOB_DIOSettings[4].bOutputDir = FALSE;
	BOB_DIOSettings[4].OUTvalue = 0;
	snprintf(&CThordaq::BOB_DIOconfiguration[5 * TD_BOBDIODef::CharPerBOB_DIO], TD_BOBDIODef::CharPerBOB_DIO, "D05I05M%-2.2dA-1", DI_External_Frame_Retrigger);
	BOB_DIOSettings[5].MUX = DI_External_Frame_Retrigger;
	BOB_DIOSettings[5].bOutputDir = FALSE;
	BOB_DIOSettings[5].OUTvalue = 0;
	snprintf(&CThordaq::BOB_DIOconfiguration[6 * TD_BOBDIODef::CharPerBOB_DIO], TD_BOBDIODef::CharPerBOB_DIO, "D06O06M%-2.2dA-1", DO_ScanLine_Direction);
	BOB_DIOSettings[6].MUX = DO_ScanLine_Direction;
	BOB_DIOSettings[6].bOutputDir = TRUE;
	BOB_DIOSettings[6].OUTvalue = 0;
	snprintf(&CThordaq::BOB_DIOconfiguration[7 * TD_BOBDIODef::CharPerBOB_DIO], TD_BOBDIODef::CharPerBOB_DIO, "D07O07M%-2.2dA-1", DO_1_WaveMUX);
	BOB_DIOSettings[7].MUX = DO_1_WaveMUX;
	BOB_DIOSettings[7].bOutputDir = TRUE;
	BOB_DIOSettings[7].OUTvalue = 0;

	// set other defaults in "decoded" DLL struct...
	for (iDIOindex = 8; iDIOindex < TD_BOBDIODef::NumBOB_DIOs; iDIOindex++)
	{
		BOB_DIOSettings[iDIOindex].BNClabel = iDIOindex;
		BOB_DIOSettings[iDIOindex].FPGAioIndex = iDIOindex;
		BOB_DIOSettings[iDIOindex].MUX = BOB3U_GPIO; // i.e., set by I2C command to CPLD
		BOB_DIOSettings[iDIOindex].AuxMUX = -1;
		BOB_DIOSettings[iDIOindex].OUTvalue = 0;
		BOB_DIOSettings[iDIOindex].bOutputDir = FALSE;
	}

	// Set 3U BOB Dn port defaults (above first 8 high speed DIO)																															  // set defaults for 3U Panel CPLD configured DIO
	for (iDIOindex = 8; iDIOindex < TD_BOBDIODef::NumBOB_DIOs; iDIOindex++)
	{
		snprintf(&CThordaq::BOB_DIOconfiguration[iDIOindex * CharPerBOB_DIO], TD_BOBDIODef::CharPerBOB_DIO, "D%-2.2d%c%-2.2dM%-2.2dA-1", iDIOindex, (BOB_DIOSettings[iDIOindex].bOutputDir ? 'O' : 'I'), iDIOindex, BOB3U_GPIO); // i.e. BOB3U_GPIO 0x30
	}

	for (int i = 0; i < TD_BOBAIDef::NumBOB_AIs; i++)
	{
		BOB_AISettings[i].BNClabel = i; // (debug sanity)
		BOB_AISettings[i].Polarity = 'B';
		BOB_AISettings[i].VoltageRange = 10.0; // e.g. see "Maxim Integrated" MAX127/MAX128 Data Sheet 19-4773; Rev1; 12/12, pg. 10 
	}


	/////////////////////

	// DZ SDK -- define the hardware registers and bit fields for this board
	// If both WriteOnly and ReadOnly are false, register is Read/Write
	char RegFldVerBuf[48] = "V1.7.0.1 \0";
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
	_FPGAregister[GlobalControlReg] = new FPGA_HardwareReg("GlobalControlReg", 4, BAR3, 0x0, 0, WRITEONLY);
	_FPGAregister[GlobalControlReg]->BitField[GIGCR0_STOP_RUN] = new RegisterBitField("GIGCR0_STOP_RUN", 0, 0);  // Arm_ASSERT for ImageAcqTrigger (PT format)
	_FPGAregister[GlobalControlReg]->BitField[GIGCR0_LED1] = new RegisterBitField("GIGCR0_LED1", 1, 1);
	_FPGAregister[GlobalControlReg]->BitField[GIGCR0_LED2] = new RegisterBitField("GIGCR0_LED2", 2, 2);
	// (HW_TRIG_MODE deprecated in F/W 20220901 and later (TILS 5.x)
	_FPGAregister[GlobalControlReg]->BitField[BPI_FLASH_MSB24] = new RegisterBitField("BPI_FLASH_MSB24", 6, 6);
	_FPGAregister[GlobalControlReg]->BitField[BPI_FLASH_MSB25] = new RegisterBitField("BPI_FLASH_MSB25", 7, 7);
	_FPGAregister[GlobalControlReg]->BitField[ImageAcqPT_HW_In1_SEL] = new RegisterBitField("ImageAcqPT_HW_In1_SEL", 8, 9);
	_FPGAregister[GlobalControlReg]->BitField[ImageAcqPT_HW_In2_SEL] = new RegisterBitField("ImageAcqPT_HW_In2_SEL", 10, 11);
	_FPGAregister[GlobalControlReg]->BitField[ImageAcqPT_HWSW_SEL] = new RegisterBitField("ImageAcqPT_HWSW_SEL", 12, 12);
	_FPGAregister[GlobalControlReg]->BitField[ImageAcqPT_InputCfgIndx] = new RegisterBitField("ImageAcqPT_InputCfgIndx", 16, 20);
	_FPGAregister[GlobalControlReg]->BitField[ImageAcqPT_OutCfgFn] = new RegisterBitField("ImageAcqPT_OutCfgFn", 21, 22);
	_FPGAregister[GlobalControlReg]->BitField[ImageAcqPT_CfgWriteSTROBE] = new RegisterBitField("ImageAcqPT_CfgWriteSTROBE", 23, 23);
	_FPGAregister[GlobalControlReg]->BitField[ImageAcqPT_DO_WaveformIN_SEL] = new RegisterBitField("ImageAcqPT_DO_WaveformIN_SEL", 24, 27);

	_FPGAregister[GlobalFPGARevReg] = new FPGA_HardwareReg("GlobalFPGARevReg", 4, BAR3, 0x0, 0, READONLY);

	_FPGAregister[GlobalReadData] = new FPGA_HardwareReg("GlobalReadData", 1, BAR3, 0x6, 0, READONLY);
	_FPGAregister[GlobalReadData]->BitField[hw_trig_readback] = new RegisterBitField("hw_trig_readback", 0, 0);
	_FPGAregister[GlobalReadData]->BitField[hw_trig_irq] = new RegisterBitField("hw_trig_irq", 1, 1);
	_FPGAregister[GlobalReadData]->BitField[SCRATCH_BITS_READ] = new RegisterBitField("SCRATCH_BITS_READ", 2, 7);

	_FPGAregister[GlobalScratch] = new FPGA_HardwareReg("GlobalScratch", 1, BAR3, 0x28, 0, WRITEONLY);
	_FPGAregister[GlobalScratch]->BitField[SCRATCH_BITS_WRITE] = new RegisterBitField("SCRATCH_BITS_WRITE", 0, 5);

	_FPGAregister[GlobalGPIAuxReg] = new FPGA_HardwareReg("GlobalGPIAuxReg", 1, BAR3, 0x7, 0, READONLY);
	_FPGAregister[GlobalGPIAuxReg]->BitField[Global_Aux_GPI_0] = new RegisterBitField("Global_Aux_GPI_0", 0, 0);
	_FPGAregister[GlobalGPIAuxReg]->BitField[Global_Aux_GPI_1] = new RegisterBitField("Global_Aux_GPI_1", 1, 1);
	_FPGAregister[GlobalGPIAuxReg]->BitField[Global_Aux_GPI_2] = new RegisterBitField("Global_Aux_GPI_2", 2, 2);
	_FPGAregister[GlobalGPIAuxReg]->BitField[Global_Aux_GPI_3] = new RegisterBitField("Global_Aux_GPI_3", 3, 3);
	_FPGAregister[GlobalGPIAuxReg]->BitField[Global_Aux_GPI_4] = new RegisterBitField("Global_Aux_GPI_4", 4, 4);

	_FPGAregister[GlobalImageSyncControlReg] = new FPGA_HardwareReg("GlobalImageSyncControlReg", 1, BAR3, 0x8, 0, WRITEONLY);
	_FPGAregister[GlobalImageSyncControlReg]->BitField[BIDIR_SCAN_MODE] = new RegisterBitField("BIDIR_SCAN_MODE", 1, 1);
	_FPGAregister[GlobalImageSyncControlReg]->BitField[SCAN_DIR] = new RegisterBitField("SCAN_DIR", 2, 2);

	_FPGAregister[GlobalImageHSIZE] = new FPGA_HardwareReg("GlobalImageHSIZE", 2, BAR3, 0x10, 0, WRITEONLY);
	_FPGAregister[GlobalImageAcqHSIZE] = new FPGA_HardwareReg("GlobalImageAcqHSIZE", 2, BAR3, 0x12, 0, WRITEONLY);
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

	_FPGAregister[GlobalGPOAuxReg] = new FPGA_HardwareReg("GlobalGPOAuxReg", 2, BAR3, 0x30, 0, WRITEONLY); // 
	_FPGAregister[GlobalGPOAuxReg]->BitField[Global_Aux_GPO_0] = new RegisterBitField("Global_Aux_GPO_0", 0, 0);
	_FPGAregister[GlobalGPOAuxReg]->BitField[Global_Aux_GPO_1] = new RegisterBitField("Global_Aux_GPO_1", 1, 1);
	_FPGAregister[GlobalGPOAuxReg]->BitField[Global_Aux_GPO_2] = new RegisterBitField("Global_Aux_GPO_2", 2, 2);
	_FPGAregister[GlobalGPOAuxReg]->BitField[Global_Aux_GPO_3] = new RegisterBitField("Global_Aux_GPO_3", 3, 3);
	_FPGAregister[GlobalGPOAuxReg]->BitField[Global_Aux_GPO_4] = new RegisterBitField("Global_Aux_GPO_4", 4, 4);

	_FPGAregister[GlobalAuxGPIODirReg] = new FPGA_HardwareReg("GlobalGPIODirReg", 2, BAR3, 0x32, 0, WRITEONLY); // 
	_FPGAregister[GlobalAuxGPIODirReg]->BitField[Aux_GPIO_DIR_0] = new RegisterBitField("Aux_GPIO_DIR_0", 0, 0);
	_FPGAregister[GlobalAuxGPIODirReg]->BitField[Aux_GPIO_DIR_1] = new RegisterBitField("Aux_GPIO_DIR_1", 1, 1);
	_FPGAregister[GlobalAuxGPIODirReg]->BitField[Aux_GPIO_DIR_2] = new RegisterBitField("Aux_GPIO_DIR_2", 2, 2);
	_FPGAregister[GlobalAuxGPIODirReg]->BitField[Aux_GPIO_DIR_3] = new RegisterBitField("Aux_GPIO_DIR_3", 3, 3);
	_FPGAregister[GlobalAuxGPIODirReg]->BitField[Aux_GPIO_DIR_4] = new RegisterBitField("Aux_GPIO_DIR_4", 4, 4);

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
	_FPGAregister[ScanningFrameCount] = new FPGA_HardwareReg("ScanningFrameCount", 4, BAR3, 0x142, 0, WRITEONLY);

	_FPGAregister[ADPLL_ControlReg] = new FPGA_HardwareReg("ADPLL_ControlReg", 1, BAR3, 0x148, 0, WRITEONLY);
	_FPGAregister[ADPLL_SyncDelay] = new FPGA_HardwareReg("ADPLL_SyncDelay", 2, BAR3, 0x150, 0, WRITEONLY);
	_FPGAregister[ADPLL_PhaseOffset] = new FPGA_HardwareReg("ADPLL_PhaseOffset", 2, BAR3, 0x152, 0, WRITEONLY);
	_FPGAregister[ADPLL_DCO_CenterFreq] = new FPGA_HardwareReg("ADPLL_DCO_CenterFreq", 4, BAR3, 0x158, 0, WRITEONLY);

	_FPGAregister[ScanningGalvoPixelDwell] = new FPGA_HardwareReg("ScanningGalvoPixelDwell", 3, BAR3, 0x160, 0, WRITEONLY);
	_FPGAregister[ScanningGalvoPixelDelay] = new FPGA_HardwareReg("ScanningGalvoPixelDelay", 3, BAR3, 0x168, 0, WRITEONLY);
	_FPGAregister[ScanningIntraLineDelay] = new FPGA_HardwareReg("ScanningIntraLineDelay", 3, BAR3, 0x170, 0, WRITEONLY);
	_FPGAregister[ScanningIntraFrameDelay] = new FPGA_HardwareReg("ScanningIntraFrameDelay", 3, BAR3, 0x178, 0, WRITEONLY);
	_FPGAregister[ScanningPreSOFDelay] = new FPGA_HardwareReg("ScanningPreSOFDelay", 3, BAR3, 0x17B, 0, WRITEONLY);

	// Sampling Clock Gen
	_FPGAregister[SamplingClockControlReg] = new FPGA_HardwareReg("SamplingClockControlReg", 1, BAR3, 0x180, 0, WRITEONLY);
	_FPGAregister[SamplingClockControlReg]->BitField[SC_PHASE_INCREMENT_MODE] = new RegisterBitField("SC_PHASE_INCREMENT_MODE", 0, 0); // Writeonly 8-bits
	_FPGAregister[SamplingClockControlReg]->BitField[SC_FREQ_MEAS_SEL] = new RegisterBitField("SC_FREQ_MEAS_SEL", 1, 1); // 
	_FPGAregister[SamplingClockControlReg]->BitField[DDS_CLK_3P_EN] = new RegisterBitField("DDS_CLK_3P_EN", 2, 2); // 
	_FPGAregister[SamplingClockControlReg]->BitField[SC_3P_EN] = new RegisterBitField("SC_3P_EN", 3, 3); // 
	_FPGAregister[SamplingClockControlReg]->BitField[SC_CLK_CNT_CLEAR] = new RegisterBitField("SC_CLK_CNT_CLEAR", 4, 4); // 
	_FPGAregister[SamplingClockControlReg]->BitField[SC_SPI_TXRX_EN] = new RegisterBitField("SC_SPI_TXRX_EN", 5, 5); // 

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
	_FPGAregister[SamplingClockFreqMeasurePeriod] = new FPGA_HardwareReg("SamplingClockFreqMeasurePeriod", 4, BAR3, 0x1A0, 0, WRITEONLY);  // 32-bit
	_FPGAregister[SamplingClockCLKRX_TXBYTE] = new FPGA_HardwareReg("SamplingClockCLKRX_TXBYTE", 1, BAR3, 0x1B0, 0, WRITEONLY);  // 8-bit

	_FPGAregister[DDS_3P_RVB_Phase0] = new FPGA_HardwareReg("DDS_3P_RVB_Phase0", 1, BAR3, 0X1B8, 0, WRITEONLY);  // 8-bit
	_FPGAregister[DDS_3P_RVB_Phase1] = new FPGA_HardwareReg("DDS_3P_RVB_Phase1", 1, BAR3, 0X1B9, 0, WRITEONLY);  // 8-bit

	// ADC Stream
	_FPGAregister[ADCStreamControlReg] = new FPGA_HardwareReg("ADCStreamControlReg", 1, BAR3, 0x1C0, 0, WRITEONLY); // WRITEonly
	_FPGAregister[ADCStreamControlReg]->BitField[ADCStream_SCAN_PER_SRCE] = new RegisterBitField("ADCStream_SCAN_PER_SRCE", 0, 0); // 
	_FPGAregister[ADCStreamControlReg]->BitField[ADCStream_DCoffsetPreFIR] = new RegisterBitField("ADCStream_DCoffsetPreFIR", 1, 1); // 
	_FPGAregister[ADCStreamControlReg]->BitField[ADCStream_DCoffsetPostFIR] = new RegisterBitField("ADCStream_DCoffsetPostFIR", 2, 2); // 
	_FPGAregister[ADCStreamControlReg]->BitField[ADCStream_FIRcoeffReload] = new RegisterBitField("ADCStream_FIRcoeffReload", 3, 7); // 

	_FPGAregister[ADCStreamControl2Reg] = new FPGA_HardwareReg("ADCStreamControl2Reg", 1, BAR3, 0x1C1, 0, WRITEONLY); // WRITEonly
	_FPGAregister[ADCStreamControl2Reg]->BitField[ADCStream_PLSE_INTERLV_EN] = new RegisterBitField("ADCStream_PLSE_INTERLV_EN", 0, 5); //
	_FPGAregister[ADCStreamControl2Reg]->BitField[ADCStream_REVERB_MP_EN] = new RegisterBitField("ADCStream_REVERB_MP_EN", 6, 6); // 
	_FPGAregister[ADCStreamControl2Reg]->BitField[ADCStream_LASER3P_MARKER_EN] = new RegisterBitField("ADCStream_LASER3P_MARKER_EN", 7, 7); //

	_FPGAregister[ADCStreamControl3Reg] = new FPGA_HardwareReg("ADCStreamControl3Reg", 2, BAR3, 0x1C2, 0, WRITEONLY); // WRITEonly
	_FPGAregister[ADCStreamControl3Reg]->BitField[ADCStream_MAFilterLength] = new RegisterBitField("ADCStream_MAFilterLength", 0, 2); //
	_FPGAregister[ADCStreamControl3Reg]->BitField[ADCStream_MA_Filter_Sel] = new RegisterBitField("ADCStream_MA_Filter_Sel", 8, 8); // 

	_FPGAregister[ADCStreamScanningPeriodReg] = new FPGA_HardwareReg("ADCStreamScanningPeriodReg", 2, BAR3, 0x1C8, 0, WRITEONLY); // WRITEonly
	_FPGAregister[ADCStreamDownsampleReg] = new FPGA_HardwareReg("ADCStreamDownsampleReg", 2, BAR3, 0x1D0, 0, WRITEONLY); //
	_FPGAregister[ADCStreamDownsampleReg2] = new FPGA_HardwareReg("ADCStreamDownsampleReg2", 2, BAR3, 0x1D2, 0, WRITEONLY); // 
	_FPGAregister[ADCStreamReverbDownsampleReg] = new FPGA_HardwareReg("ADCStreamReverbDownsampleReg", 1, BAR3, 0x1D6, 0, WRITEONLY); // only 4 bits

	_FPGAregister[ADCStreamDCoffsetChan0] = new FPGA_HardwareReg("ADCStreamDCoffsetChan0", 2, BAR3, 0x1D8, 0, WRITEONLY);
	_FPGAregister[ADCStreamDCoffsetChan1] = new FPGA_HardwareReg("ADCStreamDCoffsetChan1", 2, BAR3, 0x1DA, 0, WRITEONLY);
	_FPGAregister[ADCStreamDCoffsetChan2] = new FPGA_HardwareReg("ADCStreamDCoffsetChan2", 2, BAR3, 0x1DC, 0, WRITEONLY);
	_FPGAregister[ADCStreamDCoffsetChan3] = new FPGA_HardwareReg("ADCStreamDCoffsetChan3", 2, BAR3, 0x1DE, 0, WRITEONLY);
	_FPGAregister[ADCStreamDCoffsetChan4] = new FPGA_HardwareReg("ADCStreamDCoffsetChan4", 2, BAR3, 0x1E0, 0, WRITEONLY);
	_FPGAregister[ADCStreamDCoffsetChan5] = new FPGA_HardwareReg("ADCStreamDCoffsetChan5", 2, BAR3, 0x1E2, 0, WRITEONLY);
	_FPGAregister[ADCStreamDCoffsetChan6] = new FPGA_HardwareReg("ADCStreamDCoffsetChan6", 2, BAR3, 0x1E4, 0, WRITEONLY);
	_FPGAregister[ADCStreamDCoffsetChan7] = new FPGA_HardwareReg("ADCStreamDCoffsetChan7", 2, BAR3, 0x1E6, 0, WRITEONLY);

	_FPGAregister[ADCStreamFIRcoeffReg] = new FPGA_HardwareReg("ADCStreamFIRcoeffReg", 2, BAR3, 0x1E8, 0, WRITEONLY);
	_FPGAregister[ADCStreamPulseInterleaveOffsetReg] = new FPGA_HardwareReg("ADCStreamPulseInterleaveOffsetReg", 2, BAR3, 0x1F0, 0, WRITEONLY);

	_FPGAregister[ADCStream3PReverbMPCyclesReg] = new FPGA_HardwareReg("ADCStream3PReverbMPCyclesReg", 1, BAR3, 0x1F9, 0, WRITEONLY);

	_FPGAregister[ADCStream3PSampleOffsetReg] = new FPGA_HardwareReg("ADCStream3PSampleOffsetReg", 4, BAR3, 0x1FC, 0, WRITEONLY);
	_FPGAregister[ADCStream3PSampleOffsetReg]->BitField[ADCStream_3PSampleOffset0] = new RegisterBitField("ADCStream_3PSampleOffset0", 0, 7); //
	_FPGAregister[ADCStream3PSampleOffsetReg]->BitField[ADCStream_3PSampleOffset1] = new RegisterBitField("ADCStream_3PSampleOffset1", 8, 15); // 
	_FPGAregister[ADCStream3PSampleOffsetReg]->BitField[ADCStream_3PSampleOffset2] = new RegisterBitField("ADCStream_3PSampleOffset2", 16, 23); //
	_FPGAregister[ADCStream3PSampleOffsetReg]->BitField[ADCStream_3PSampleOffset3] = new RegisterBitField("ADCStream_3PSampleOffset3", 24, 31); // 

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
	_FPGAregister[ADCInterface3PMarkersRDivide] = new FPGA_HardwareReg("ADCInterface3PMarkersRDivide", 1, BAR3, 0x230, 0, WRITEONLY);

	_FPGAregister[ADCFMCStatusReg] = new FPGA_HardwareReg("ADCFMCStatusReg", 2, BAR3, 0x200, 0, READONLY); // 0x200 READonly
	_FPGAregister[ADCFMCStatusReg]->BitField[ThorDAQ_ADC_VERSION] = new RegisterBitField("ThorDAQ_ADC_VERSION", 0, 3); // 
	_FPGAregister[ADCFMCStatusReg]->BitField[mailbox_tnsfr_cmplt] = new RegisterBitField("mailbox_tnsfr_cmplt", 4, 4); // 
	_FPGAregister[ADCFMCStatusReg]->BitField[jesd204b_rx_sync_chan0_1] = new RegisterBitField("jesd204b_rx_sync_chan0_1", 5, 5); // 
	_FPGAregister[ADCFMCStatusReg]->BitField[jesd204b_rx_sync_chan2_3] = new RegisterBitField("jesd204b_rx_sync_chan2_3", 6, 6); // 
	_FPGAregister[ADCFMCStatusReg]->BitField[jesd_rx_lost_cnt_chan0_1] = new RegisterBitField("jesd_rx_lost_cnt_chan0_1", 8, 10); // 
	_FPGAregister[ADCFMCStatusReg]->BitField[jesd_rx_lost_cnt_chan2_3] = new RegisterBitField("jesd_rx_lost_cnt_chan2_3", 11, 13); // 

	_FPGAregister[DACWaveGenControlReg] = new FPGA_HardwareReg("DACWaveGenControlReg", 8, BAR3, 0x240, 0, WRITEONLY); // 64 bits   SWUG: ThorDAQ Galvo Waveform Generation Subsystem Register(s):     
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_DMA_Chans_Active] = new RegisterBitField("DAC_DMA_Chans_Active", 0, 3); // 4 bits
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_DMA_Playback_En0] = new RegisterBitField("DAC_DMA_Playback_En0", 4, 4);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_DMA_Playback_En1] = new RegisterBitField("DAC_DMA_Playback_En1", 5, 5);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_DMA_Playback_En2] = new RegisterBitField("DAC_DMA_Playback_En2", 6, 6);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_DMA_Playback_En3] = new RegisterBitField("DAC_DMA_Playback_En3", 7, 7);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_DMA_Playback_En4] = new RegisterBitField("DAC_DMA_Playback_En4", 8, 8);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_DMA_Playback_En5] = new RegisterBitField("DAC_DMA_Playback_En5", 9, 9);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_DMA_Playback_En6] = new RegisterBitField("DAC_DMA_Playback_En6", 10, 10);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_DMA_Playback_En7] = new RegisterBitField("DAC_DMA_Playback_En7", 11, 11);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_DMA_Playback_En8] = new RegisterBitField("DAC_DMA_Playback_En8", 12, 12);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_DMA_Playback_En9] = new RegisterBitField("DAC_DMA_Playback_En9", 13, 13);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_DMA_Playback_En10] = new RegisterBitField("DAC_DMA_Playback_En10", 14, 14);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_DMA_Playback_En11] = new RegisterBitField("DAC_DMA_Playback_En11", 15, 15);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_DMA_Playback_En12] = new RegisterBitField("DAC_DMA_Playback_En12", 16, 16);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_DMA_Playback_En13] = new RegisterBitField("DAC_DMA_Playback_En13", 17, 17);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Filter_En0] = new RegisterBitField("DAC_Filter_En0", 18, 18);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Filter_En1] = new RegisterBitField("DAC_Filter_En1", 19, 19);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Filter_En2] = new RegisterBitField("DAC_Filter_En2", 20, 20);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Filter_En3] = new RegisterBitField("DAC_Filter_En3", 21, 21);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Filter_En4] = new RegisterBitField("DAC_Filter_En4", 22, 22);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Filter_En5] = new RegisterBitField("DAC_Filter_En5", 23, 23);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Filter_En6] = new RegisterBitField("DAC_Filter_En6", 24, 24);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Filter_En7] = new RegisterBitField("DAC_Filter_En7", 25, 25);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Filter_En8] = new RegisterBitField("DAC_Filter_En8", 26, 26);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Filter_En9] = new RegisterBitField("DAC_Filter_En9", 27, 27);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Filter_En10] = new RegisterBitField("DAC_Filter_En10", 28, 28);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Filter_En11] = new RegisterBitField("DAC_Filter_En11", 29, 29);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Filter_En12] = new RegisterBitField("DAC_Filter_En12", 30, 30);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Filter_En13] = new RegisterBitField("DAC_Filter_En13", 31, 31);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Sync_hsync0] = new RegisterBitField("DAC_Sync_hsync0", 32, 32);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Sync_hsync1] = new RegisterBitField("DAC_Sync_hsync1", 33, 33);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Sync_hsync2] = new RegisterBitField("DAC_Sync_hsync2", 34, 34);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Sync_hsync3] = new RegisterBitField("DAC_Sync_hsync3", 35, 35);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Sync_hsync4] = new RegisterBitField("DAC_Sync_hsync4", 36, 36);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Sync_hsync5] = new RegisterBitField("DAC_Sync_hsync5", 37, 37);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Sync_hsync6] = new RegisterBitField("DAC_Sync_hsync6", 38, 38);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Sync_hsync7] = new RegisterBitField("DAC_Sync_hsync7", 39, 39);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Sync_hsync8] = new RegisterBitField("DAC_Sync_hsync8", 40, 40);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Sync_hsync9] = new RegisterBitField("DAC_Sync_hsync9", 41, 41);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Sync_hsync10] = new RegisterBitField("DAC_Sync_hsync10", 42, 42);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Sync_hsync11] = new RegisterBitField("DAC_Sync_hsync11", 43, 43);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Sync_hsync12] = new RegisterBitField("DAC_Sync_hsync12", 44, 44);
	_FPGAregister[DACWaveGenControlReg]->BitField[DAC_Sync_hsync13] = new RegisterBitField("DAC_Sync_hsync13", 45, 45);

	_FPGAregister[DAC_UpdateRate_Chan0] = new FPGA_HardwareReg("DAC_UpdateRate_Chan0", 2, BAR3, 0x248, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan1] = new FPGA_HardwareReg("DAC_UpdateRate_Chan1", 2, BAR3, 0x24A, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan2] = new FPGA_HardwareReg("DAC_UpdateRate_Chan2", 2, BAR3, 0x24C, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan3] = new FPGA_HardwareReg("DAC_UpdateRate_Chan3", 2, BAR3, 0x24E, 0, WRITEONLY);

	_FPGAregister[DAC_UpdateRate_Chan4] = new FPGA_HardwareReg("DAC_UpdateRate_Chan4", 2, BAR3, 0x250, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan5] = new FPGA_HardwareReg("DAC_UpdateRate_Chan5", 2, BAR3, 0x252, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan6] = new FPGA_HardwareReg("DAC_UpdateRate_Chan6", 2, BAR3, 0x254, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan7] = new FPGA_HardwareReg("DAC_UpdateRate_Chan7", 2, BAR3, 0x256, 0, WRITEONLY);

	_FPGAregister[DAC_UpdateRate_Chan8] = new FPGA_HardwareReg("DAC_UpdateRate_Chan8", 2, BAR3, 0x258, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan9] = new FPGA_HardwareReg("DAC_UpdateRate_Chan9", 2, BAR3, 0x25A, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan10] = new FPGA_HardwareReg("DAC_UpdateRate_Chan10", 2, BAR3, 0x25C, 0, WRITEONLY);
	_FPGAregister[DAC_UpdateRate_Chan11] = new FPGA_HardwareReg("DAC_UpdateRate_Chan11", 2, BAR3, 0x25E, 0, WRITEONLY);

	_FPGAregister[DACWaveGen3PSyncControlReg] = new FPGA_HardwareReg("DACWaveGen3PSyncControlReg", 8, BAR3, 0x260, 0, WRITEONLY);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Bank_Sel_Frame_CNT] = new RegisterBitField("DAC_Bank_Sel_Frame_CNT", 16, 25);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_SLOW_RATE] = new RegisterBitField("DAC_SLOW_RATE", 28, 28);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Waveplay_IRQ_rearm] = new RegisterBitField("DAC_Waveplay_IRQ_rearm", 29, 29);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Waveplay_Start_Of_Waveform_Delay_Enable] = new RegisterBitField("DAC_Waveplay_Start_Of_Waveform_Delay_Enable", 30, 30);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Bank_Switch_EN] = new RegisterBitField("DAC_Bank_Switch_EN", 31, 31);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Waveplay_Cont_Playback_En0] = new RegisterBitField("DAC_Waveplay_Cont_Playback_En0", 32, 32);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Waveplay_Cont_Playback_En1] = new RegisterBitField("DAC_Waveplay_Cont_Playback_En1", 33, 33);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Waveplay_Cont_Playback_En2] = new RegisterBitField("DAC_Waveplay_Cont_Playback_En2", 34, 34);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Waveplay_Cont_Playback_En3] = new RegisterBitField("DAC_Waveplay_Cont_Playback_En3", 35, 35);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Waveplay_Cont_Playback_En4] = new RegisterBitField("DAC_Waveplay_Cont_Playback_En4", 36, 36);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Waveplay_Cont_Playback_En5] = new RegisterBitField("DAC_Waveplay_Cont_Playback_En5", 37, 37);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Waveplay_Cont_Playback_En6] = new RegisterBitField("DAC_Waveplay_Cont_Playback_En6", 38, 38);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Waveplay_Cont_Playback_En7] = new RegisterBitField("DAC_Waveplay_Cont_Playback_En7", 39, 39);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Waveplay_Cont_Playback_En8] = new RegisterBitField("DAC_Waveplay_Cont_Playback_En8", 40, 40);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Waveplay_Cont_Playback_En9] = new RegisterBitField("DAC_Waveplay_Cont_Playback_En9", 41, 41);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Waveplay_Cont_Playback_En10] = new RegisterBitField("DAC_Waveplay_Cont_Playback_En10", 42, 42);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Waveplay_Cont_Playback_En11] = new RegisterBitField("DAC_Waveplay_Cont_Playback_En11", 43, 43);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Waveplay_Cont_Playback_En12] = new RegisterBitField("DAC_Waveplay_Cont_Playback_En12", 44, 44);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[DAC_Waveplay_Cont_Playback_En13] = new RegisterBitField("DAC_Waveplay_Cont_Playback_En13", 45, 45);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[PT_WaveformControlTrig_SEL0] = new RegisterBitField("PT_WaveformControlTrig_SEL0", 46, 46); // if 0, controlled by ImageAcqTrigger (Legacy trig)
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[PT_WaveformControlTrig_SEL1] = new RegisterBitField("PT_WaveformControlTrig_SEL1", 47, 47);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[PT_WaveformControlTrig_SEL2] = new RegisterBitField("PT_WaveformControlTrig_SEL2", 48, 48);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[PT_WaveformControlTrig_SEL3] = new RegisterBitField("PT_WaveformControlTrig_SEL3", 49, 49);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[PT_WaveformControlTrig_SEL4] = new RegisterBitField("PT_WaveformControlTrig_SEL4", 50, 50);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[PT_WaveformControlTrig_SEL5] = new RegisterBitField("PT_WaveformControlTrig_SEL5", 51, 51);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[PT_WaveformControlTrig_SEL6] = new RegisterBitField("PT_WaveformControlTrig_SEL6", 52, 52);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[PT_WaveformControlTrig_SEL7] = new RegisterBitField("PT_WaveformControlTrig_SEL7", 53, 53);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[PT_WaveformControlTrig_SEL8] = new RegisterBitField("PT_WaveformControlTrig_SEL8", 54, 54);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[PT_WaveformControlTrig_SEL9] = new RegisterBitField("PT_WaveformControlTrig_SEL9", 55, 55);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[PT_WaveformControlTrig_SEL10] = new RegisterBitField("PT_WaveformControlTrig_SEL10", 56, 56);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[PT_WaveformControlTrig_SEL11] = new RegisterBitField("PT_WaveformControlTrig_SEL11", 57, 57);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[PT_WaveformControlTrig_SEL12] = new RegisterBitField("PT_WaveformControlTrig_SEL12", 58, 58);
	_FPGAregister[DACWaveGen3PSyncControlReg]->BitField[PT_WaveformControlTrig_SEL13] = new RegisterBitField("PT_WaveformControlTrig_SEL13", 59, 59);

	_FPGAregister[DAC_Park_Chan0] = new FPGA_HardwareReg("DAC_Park_Chan0", 2, BAR3, 0x268, 0, WRITEONLY);
	_FPGAregister[DAC_Park_Chan1] = new FPGA_HardwareReg("DAC_Park_Chan1", 2, BAR3, 0x26A, 0, WRITEONLY);
	_FPGAregister[DAC_Park_Chan2] = new FPGA_HardwareReg("DAC_Park_Chan2", 2, BAR3, 0x26C, 0, WRITEONLY);
	_FPGAregister[DAC_Park_Chan3] = new FPGA_HardwareReg("DAC_Park_Chan3", 2, BAR3, 0x26E, 0, WRITEONLY);

	_FPGAregister[DAC_Park_Chan4] = new FPGA_HardwareReg("DAC_Park_Chan4", 2, BAR3, 0x270, 0, WRITEONLY);
	_FPGAregister[DAC_Park_Chan5] = new FPGA_HardwareReg("DAC_Park_Chan5", 2, BAR3, 0x272, 0, WRITEONLY);
	_FPGAregister[DAC_Park_Chan6] = new FPGA_HardwareReg("DAC_Park_Chan6", 2, BAR3, 0x274, 0, WRITEONLY);
	_FPGAregister[DAC_Park_Chan7] = new FPGA_HardwareReg("DAC_Park_Chan7", 2, BAR3, 0x276, 0, WRITEONLY);

	_FPGAregister[DAC_Park_Chan8] = new FPGA_HardwareReg("DAC_Park_Chan8", 2, BAR3, 0x278, 0, WRITEONLY);
	_FPGAregister[DAC_Park_Chan9] = new FPGA_HardwareReg("DAC_Park_Chan9", 2, BAR3, 0x27A, 0, WRITEONLY);
	_FPGAregister[DAC_Park_Chan10] = new FPGA_HardwareReg("DAC_Park_Chan10", 2, BAR3, 0x27C, 0, WRITEONLY);
	_FPGAregister[DAC_Park_Chan11] = new FPGA_HardwareReg("DAC_Park_Chan11", 2, BAR3, 0x27E, 0, WRITEONLY);

	_FPGAregister[DACWaveGenISR] = new FPGA_HardwareReg("DACWaveGenISR", 8, BAR3, 0x280, 0, READONLY);
	_FPGAregister[DACWaveGenISR]->BitField[DACWaveGenIntStatus_Chan0] = new RegisterBitField("DACWaveGenIntStatus_Chan0", 48, 48);
	_FPGAregister[DACWaveGenISR]->BitField[DACWaveGenIntStatus_Chan1] = new RegisterBitField("DACWaveGenIntStatus_Chan1", 49, 49);
	_FPGAregister[DACWaveGenISR]->BitField[DACWaveGenIntStatus_Chan2] = new RegisterBitField("DACWaveGenIntStatus_Chan2", 50, 50);
	_FPGAregister[DACWaveGenISR]->BitField[DACWaveGenIntStatus_Chan3] = new RegisterBitField("DACWaveGenIntStatus_Chan3", 51, 51);
	_FPGAregister[DACWaveGenISR]->BitField[DACWaveGenIntStatus_Chan4] = new RegisterBitField("DACWaveGenIntStatus_Chan4", 52, 52);
	_FPGAregister[DACWaveGenISR]->BitField[DACWaveGenIntStatus_Chan5] = new RegisterBitField("DACWaveGenIntStatus_Chan5", 53, 53);
	_FPGAregister[DACWaveGenISR]->BitField[DACWaveGenIntStatus_Chan6] = new RegisterBitField("DACWaveGenIntStatus_Chan6", 54, 54);
	_FPGAregister[DACWaveGenISR]->BitField[DACWaveGenIntStatus_Chan7] = new RegisterBitField("DACWaveGenIntStatus_Chan7", 55, 55);
	_FPGAregister[DACWaveGenISR]->BitField[DACWaveGenIntStatus_Chan8] = new RegisterBitField("DACWaveGenIntStatus_Chan8", 56, 56);
	_FPGAregister[DACWaveGenISR]->BitField[DACWaveGenIntStatus_Chan9] = new RegisterBitField("DACWaveGenIntStatus_Chan9", 57, 57);
	_FPGAregister[DACWaveGenISR]->BitField[DACWaveGenIntStatus_Chan10] = new RegisterBitField("DACWaveGenIntStatus_Chan10", 58, 58);
	_FPGAregister[DACWaveGenISR]->BitField[DACWaveGenIntStatus_Chan11] = new RegisterBitField("DACWaveGenIntStatus_Chan11", 59, 59);
	_FPGAregister[DACWaveGenISR]->BitField[DACWaveGenIntStatus_Chan12] = new RegisterBitField("DACWaveGenIntStatus_Chan12", 60, 60);
	_FPGAregister[DACWaveGenISR]->BitField[DACWaveGenIntStatus_Chan13] = new RegisterBitField("DACWaveGenIntStatus_Chan13", 61, 61);


	_FPGAregister[DAC_Offset_Chan0] = new FPGA_HardwareReg("DAC_Offset_Chan0", 2, BAR3, 0x280, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan1] = new FPGA_HardwareReg("DAC_Offset_Chan1", 2, BAR3, 0x282, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan2] = new FPGA_HardwareReg("DAC_Offset_Chan2", 2, BAR3, 0x284, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan3] = new FPGA_HardwareReg("DAC_Offset_Chan3", 2, BAR3, 0x286, 0, WRITEONLY);

	_FPGAregister[DAC_Offset_Chan4] = new FPGA_HardwareReg("DAC_Offset_Chan4", 2, BAR3, 0x288, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan5] = new FPGA_HardwareReg("DAC_Offset_Chan5", 2, BAR3, 0x28A, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan6] = new FPGA_HardwareReg("DAC_Offset_Chan6", 2, BAR3, 0x28C, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan7] = new FPGA_HardwareReg("DAC_Offset_Chan7", 2, BAR3, 0x28E, 0, WRITEONLY);

	_FPGAregister[DAC_Offset_Chan8] = new FPGA_HardwareReg("DAC_Offset_Chan8", 2, BAR3, 0x290, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan9] = new FPGA_HardwareReg("DAC_Offset_Chan9", 2, BAR3, 0x292, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan10] = new FPGA_HardwareReg("DAC_Offset_Chan10", 2, BAR3, 0x294, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan11] = new FPGA_HardwareReg("DAC_Offset_Chan11", 2, BAR3, 0x296, 0, WRITEONLY);

	// 16-bit registers for the analog channel mapping
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


	//// 16-bit registers for to select or deselect external dig triggering on a per dac channel basis
	_FPGAregister[DACWaveGenSelExtDigTrigReg] = new FPGA_HardwareReg("DACWaveGenSelExtDigTrigReg", 2, BAR3, 0x29E, 0, WRITEONLY);
	_FPGAregister[DACWaveGenSelExtDigTrigReg]->BitField[WaveformControlPT_HWSW_SEL_chan0] = new RegisterBitField("WaveformControlPT_HWSW_SEL_chan0", 0, 0);
	_FPGAregister[DACWaveGenSelExtDigTrigReg]->BitField[WaveformControlPT_HWSW_SEL_chan1] = new RegisterBitField("WaveformControlPT_HWSW_SEL_chan1", 1, 1);
	_FPGAregister[DACWaveGenSelExtDigTrigReg]->BitField[WaveformControlPT_HWSW_SEL_chan2] = new RegisterBitField("WaveformControlPT_HWSW_SEL_chan2", 2, 2);
	_FPGAregister[DACWaveGenSelExtDigTrigReg]->BitField[WaveformControlPT_HWSW_SEL_chan3] = new RegisterBitField("WaveformControlPT_HWSW_SEL_chan3", 3, 3);
	_FPGAregister[DACWaveGenSelExtDigTrigReg]->BitField[WaveformControlPT_HWSW_SEL_chan4] = new RegisterBitField("WaveformControlPT_HWSW_SEL_chan4", 4, 4);
	_FPGAregister[DACWaveGenSelExtDigTrigReg]->BitField[WaveformControlPT_HWSW_SEL_chan5] = new RegisterBitField("WaveformControlPT_HWSW_SEL_chan5", 5, 5);
	_FPGAregister[DACWaveGenSelExtDigTrigReg]->BitField[WaveformControlPT_HWSW_SEL_chan6] = new RegisterBitField("WaveformControlPT_HWSW_SEL_chan6", 6, 6);
	_FPGAregister[DACWaveGenSelExtDigTrigReg]->BitField[WaveformControlPT_HWSW_SEL_chan7] = new RegisterBitField("WaveformControlPT_HWSW_SEL_chan7", 7, 7);
	_FPGAregister[DACWaveGenSelExtDigTrigReg]->BitField[WaveformControlPT_HWSW_SEL_chan8] = new RegisterBitField("WaveformControlPT_HWSW_SEL_chan8", 8, 8);
	_FPGAregister[DACWaveGenSelExtDigTrigReg]->BitField[WaveformControlPT_HWSW_SEL_chan9] = new RegisterBitField("WaveformControlPT_HWSW_SEL_chan9", 9, 9);
	_FPGAregister[DACWaveGenSelExtDigTrigReg]->BitField[WaveformControlPT_HWSW_SEL_chan10] = new RegisterBitField("WaveformControlPT_HWSW_SEL_chan10", 10, 10);
	_FPGAregister[DACWaveGenSelExtDigTrigReg]->BitField[WaveformControlPT_HWSW_SEL_chan11] = new RegisterBitField("WaveformControlPT_HWSW_SEL_chan11", 11, 11);
	_FPGAregister[DACWaveGenSelExtDigTrigReg]->BitField[WaveformControlPT_HWSW_SEL_chan12] = new RegisterBitField("WaveformControlPT_HWSW_SEL_chan12", 12, 12);
	_FPGAregister[DACWaveGenSelExtDigTrigReg]->BitField[WaveformControlPT_HWSW_SEL_chan13] = new RegisterBitField("WaveformControlPT_HWSW_SEL_chan13", 13, 13);


	_FPGAregister[DAC_UpdateRate_Chan12] = new FPGA_HardwareReg("DAC_UpdateRate_Chan12", 2, BAR3, 0x2A0, 0, WRITEONLY);  // 0x240 + 60h, DAC "digital" chans 12,13 (0-11 are "analog")
	_FPGAregister[DAC_UpdateRate_Chan13] = new FPGA_HardwareReg("DAC_UpdateRate_Chan13", 2, BAR3, 0x2A2, 0, WRITEONLY);  // Chan12 is Waveforms D0-D7, Char13 D8-D15

	_FPGAregister[DACWaveDACFilterInhibit] = new FPGA_HardwareReg("DACWaveDACFilterInhibit", 2, BAR3, 0x2A4, 0, WRITEONLY); // 14-bit REG
	_FPGAregister[DACWaveDACFilterInhibit]->BitField[DACWave_Filter_Inhibit0] = new RegisterBitField("DACWave_Filter_Inhibit0", 0, 0);
	_FPGAregister[DACWaveDACFilterInhibit]->BitField[DACWave_Filter_Inhibit1] = new RegisterBitField("DACWave_Filter_Inhibit1", 1, 1);
	_FPGAregister[DACWaveDACFilterInhibit]->BitField[DACWave_Filter_Inhibit2] = new RegisterBitField("DACWave_Filter_Inhibit2", 2, 2);
	_FPGAregister[DACWaveDACFilterInhibit]->BitField[DACWave_Filter_Inhibit3] = new RegisterBitField("DACWave_Filter_Inhibit3", 3, 3);
	_FPGAregister[DACWaveDACFilterInhibit]->BitField[DACWave_Filter_Inhibit4] = new RegisterBitField("DACWave_Filter_Inhibit4", 4, 4);
	_FPGAregister[DACWaveDACFilterInhibit]->BitField[DACWave_Filter_Inhibit5] = new RegisterBitField("DACWave_Filter_Inhibit5", 5, 5);
	_FPGAregister[DACWaveDACFilterInhibit]->BitField[DACWave_Filter_Inhibit6] = new RegisterBitField("DACWave_Filter_Inhibit6", 6, 6);
	_FPGAregister[DACWaveDACFilterInhibit]->BitField[DACWave_Filter_Inhibit7] = new RegisterBitField("DACWave_Filter_Inhibit7", 7, 7);
	_FPGAregister[DACWaveDACFilterInhibit]->BitField[DACWave_Filter_Inhibit8] = new RegisterBitField("DACWave_Filter_Inhibit8", 8, 8);
	_FPGAregister[DACWaveDACFilterInhibit]->BitField[DACWave_Filter_Inhibit9] = new RegisterBitField("DACWave_Filter_Inhibit9", 9, 9);
	_FPGAregister[DACWaveDACFilterInhibit]->BitField[DACWave_Filter_Inhibit10] = new RegisterBitField("DACWave_Filter_Inhibit10", 10, 10);
	_FPGAregister[DACWaveDACFilterInhibit]->BitField[DACWave_Filter_Inhibit11] = new RegisterBitField("DACWave_Filter_Inhibit11", 11, 11);
	_FPGAregister[DACWaveDACFilterInhibit]->BitField[DACWave_Filter_Inhibit12] = new RegisterBitField("DACWave_Filter_Inhibit12", 12, 12);
	_FPGAregister[DACWaveDACFilterInhibit]->BitField[DACWave_Filter_Inhibit13] = new RegisterBitField("DACWave_Filter_Inhibit13", 13, 13);

	_FPGAregister[DACWavePerChannelRunStop] = new FPGA_HardwareReg("DACWavePerChannelRunStop", 2, BAR3, 0x2A6, 0, WRITEONLY); // 14-bit REG
	_FPGAregister[DACWavePerChannelRunStop]->BitField[WaveformControlPT_SW_ArmASSERT_chan0] = new RegisterBitField("WaveformControlPT_SW_ArmASSERT_chan0", 0, 0);
	_FPGAregister[DACWavePerChannelRunStop]->BitField[WaveformControlPT_SW_ArmASSERT_chan1] = new RegisterBitField("WaveformControlPT_SW_ArmASSERT_chan1", 1, 1);
	_FPGAregister[DACWavePerChannelRunStop]->BitField[WaveformControlPT_SW_ArmASSERT_chan2] = new RegisterBitField("WaveformControlPT_SW_ArmASSERT_chan2", 2, 2);
	_FPGAregister[DACWavePerChannelRunStop]->BitField[WaveformControlPT_SW_ArmASSERT_chan3] = new RegisterBitField("WaveformControlPT_SW_ArmASSERT_chan3", 3, 3);
	_FPGAregister[DACWavePerChannelRunStop]->BitField[WaveformControlPT_SW_ArmASSERT_chan4] = new RegisterBitField("WaveformControlPT_SW_ArmASSERT_chan4", 4, 4);
	_FPGAregister[DACWavePerChannelRunStop]->BitField[WaveformControlPT_SW_ArmASSERT_chan5] = new RegisterBitField("WaveformControlPT_SW_ArmASSERT_chan5", 5, 5);
	_FPGAregister[DACWavePerChannelRunStop]->BitField[WaveformControlPT_SW_ArmASSERT_chan6] = new RegisterBitField("WaveformControlPT_SW_ArmASSERT_chan6", 6, 6);
	_FPGAregister[DACWavePerChannelRunStop]->BitField[WaveformControlPT_SW_ArmASSERT_chan7] = new RegisterBitField("WaveformControlPT_SW_ArmASSERT_chan7", 7, 7);
	_FPGAregister[DACWavePerChannelRunStop]->BitField[WaveformControlPT_SW_ArmASSERT_chan8] = new RegisterBitField("WaveformControlPT_SW_ArmASSERT_chan8", 8, 8);
	_FPGAregister[DACWavePerChannelRunStop]->BitField[WaveformControlPT_SW_ArmASSERT_chan9] = new RegisterBitField("WaveformControlPT_SW_ArmASSERT_chan9", 9, 9);
	_FPGAregister[DACWavePerChannelRunStop]->BitField[WaveformControlPT_SW_ArmASSERT_chan10] = new RegisterBitField("WaveformControlPT_SW_ArmASSERT_chan10", 10, 10);
	_FPGAregister[DACWavePerChannelRunStop]->BitField[WaveformControlPT_SW_ArmASSERT_chan11] = new RegisterBitField("WaveformControlPT_SW_ArmASSERT_chan11", 11, 11);
	_FPGAregister[DACWavePerChannelRunStop]->BitField[WaveformControlPT_SW_ArmASSERT_chan12] = new RegisterBitField("WaveformControlPT_SW_ArmASSERT_chan12", 12, 12);
	_FPGAregister[DACWavePerChannelRunStop]->BitField[WaveformControlPT_SW_ArmASSERT_chan13] = new RegisterBitField("WaveformControlPT_SW_ArmASSERT_chan13", 13, 13);

	_FPGAregister[DACWaveGenDescCntBeforeInt0] = new FPGA_HardwareReg("DACWaveGenDescCntBeforeInt0", 1, BAR3, 0x2A8, 0, WRITEONLY);
	_FPGAregister[DACWaveGenDescCntBeforeInt1] = new FPGA_HardwareReg("DACWaveGenDescCntBeforeInt1", 1, BAR3, 0x2A9, 0, WRITEONLY);
	_FPGAregister[DACWaveGenDescCntBeforeInt2] = new FPGA_HardwareReg("DACWaveGenDescCntBeforeInt2", 1, BAR3, 0x2AA, 0, WRITEONLY);
	_FPGAregister[DACWaveGenDescCntBeforeInt3] = new FPGA_HardwareReg("DACWaveGenDescCntBeforeInt3", 1, BAR3, 0x2AB, 0, WRITEONLY);
	_FPGAregister[DACWaveGenDescCntBeforeInt4] = new FPGA_HardwareReg("DACWaveGenDescCntBeforeInt4", 1, BAR3, 0x2AC, 0, WRITEONLY);
	_FPGAregister[DACWaveGenDescCntBeforeInt5] = new FPGA_HardwareReg("DACWaveGenDescCntBeforeInt5", 1, BAR3, 0x2AD, 0, WRITEONLY);
	_FPGAregister[DACWaveGenDescCntBeforeInt6] = new FPGA_HardwareReg("DACWaveGenDescCntBeforeInt6", 1, BAR3, 0x2AE, 0, WRITEONLY);
	_FPGAregister[DACWaveGenDescCntBeforeInt7] = new FPGA_HardwareReg("DACWaveGenDescCntBeforeInt7", 1, BAR3, 0x2AF, 0, WRITEONLY);
	_FPGAregister[DACWaveGenDescCntBeforeInt8] = new FPGA_HardwareReg("DACWaveGenDescCntBeforeInt8", 1, BAR3, 0x2B0, 0, WRITEONLY);
	_FPGAregister[DACWaveGenDescCntBeforeInt9] = new FPGA_HardwareReg("DACWaveGenDescCntBeforeInt9", 1, BAR3, 0x2B1, 0, WRITEONLY);
	_FPGAregister[DACWaveGenDescCntBeforeInt10] = new FPGA_HardwareReg("DACWaveGenDescCntBeforeInt10", 1, BAR3, 0x2B2, 0, WRITEONLY);
	_FPGAregister[DACWaveGenDescCntBeforeInt11] = new FPGA_HardwareReg("DACWaveGenDescCntBeforeInt11", 1, BAR3, 0x2B3, 0, WRITEONLY);
	_FPGAregister[DACWaveGenDescCntBeforeInt12] = new FPGA_HardwareReg("DACWaveGenDescCntBeforeInt12", 1, BAR3, 0x2B4, 0, WRITEONLY);
	_FPGAregister[DACWaveGenDescCntBeforeInt13] = new FPGA_HardwareReg("DACWaveGenDescCntBeforeInt13", 1, BAR3, 0x2B5, 0, WRITEONLY);

	_FPGAregister[DACWaveGen_EOF_DelaySampleCnt] = new FPGA_HardwareReg("DACWaveGen_EOF_DelaySampleCnt", 2, BAR3, 0x2B6, 0, WRITEONLY);

	_FPGAregister[DAC_Park_Chan12] = new FPGA_HardwareReg("DAC_Park_Chan12", 2, BAR3, 0x2B8, 0, WRITEONLY); // DAC waveform Indexes 12 & 13
	_FPGAregister[DAC_Park_Chan13] = new FPGA_HardwareReg("DAC_Park_Chan13", 2, BAR3, 0x2BA, 0, WRITEONLY); // (digital MUXed waveforms)

	_FPGAregister[DAC_Offset_Chan12] = new FPGA_HardwareReg("DAC_Offset_Chan12", 2, BAR3, 0x2BC, 0, WRITEONLY);
	_FPGAregister[DAC_Offset_Chan13] = new FPGA_HardwareReg("DAC_Offset_Chan13", 2, BAR3, 0x2BE, 0, WRITEONLY);

	_FPGAregister[DACWaveGenPerChannelHWTrigControl] = new FPGA_HardwareReg("DACWaveGenPerChannelHWTrigControl", 3, BAR3, 0x2C0, 0, WRITEONLY);
	_FPGAregister[DACWaveGenPerChannelHWTrigControl]->BitField[WaveformControlPT_InputCfgIndx] = new RegisterBitField("WaveformControlPT_InputCfgIndx", 0, 4);
	_FPGAregister[DACWaveGenPerChannelHWTrigControl]->BitField[WaveformControlPT_OutCfgFn] = new RegisterBitField("WaveformControlPT_OutCfgFn", 8, 9);
	_FPGAregister[DACWaveGenPerChannelHWTrigControl]->BitField[WaveformControlPT_TruthTableCfg_SEL] = new RegisterBitField("WaveformControlPT_TruthTableCfg_SEL", 16, 19);
	_FPGAregister[DACWaveGenPerChannelHWTrigControl]->BitField[WaveformControlPT_CfgWriteSTROBE] = new RegisterBitField("WaveformControlPT_CfgWriteSTROBE", 23, 23);

	_FPGAregister[DACWaveGenPerChannelHWTrigSel1] = new FPGA_HardwareReg("DACWaveGenPerChannelHWTrigSel1", 4, BAR3, 0x2C8, 0, WRITEONLY);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel1]->BitField[WaveformControlPT_HW_In1_SEL_chan0] = new RegisterBitField("WaveformControlPT_HW_In1_SEL_chan0", 0, 1);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel1]->BitField[WaveformControlPT_HW_In1_SEL_chan1] = new RegisterBitField("WaveformControlPT_HW_In1_SEL_chan1", 2, 3);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel1]->BitField[WaveformControlPT_HW_In1_SEL_chan2] = new RegisterBitField("WaveformControlPT_HW_In1_SEL_chan2", 4, 5);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel1]->BitField[WaveformControlPT_HW_In1_SEL_chan3] = new RegisterBitField("WaveformControlPT_HW_In1_SEL_chan3", 6, 7);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel1]->BitField[WaveformControlPT_HW_In1_SEL_chan4] = new RegisterBitField("WaveformControlPT_HW_In1_SEL_chan4", 8, 9);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel1]->BitField[WaveformControlPT_HW_In1_SEL_chan5] = new RegisterBitField("WaveformControlPT_HW_In1_SEL_chan5", 10, 11);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel1]->BitField[WaveformControlPT_HW_In1_SEL_chan6] = new RegisterBitField("WaveformControlPT_HW_In1_SEL_chan6", 12, 13);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel1]->BitField[WaveformControlPT_HW_In1_SEL_chan7] = new RegisterBitField("WaveformControlPT_HW_In1_SEL_chan7", 14, 15);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel1]->BitField[WaveformControlPT_HW_In1_SEL_chan8] = new RegisterBitField("WaveformControlPT_HW_In1_SEL_chan8", 16, 17);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel1]->BitField[WaveformControlPT_HW_In1_SEL_chan9] = new RegisterBitField("WaveformControlPT_HW_In1_SEL_chan9", 18, 19);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel1]->BitField[WaveformControlPT_HW_In1_SEL_chan10] = new RegisterBitField("WaveformControlPT_HW_In1_SEL_chan10", 20, 21);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel1]->BitField[WaveformControlPT_HW_In1_SEL_chan11] = new RegisterBitField("WaveformControlPT_HW_In1_SEL_chan11", 22, 23);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel1]->BitField[WaveformControlPT_HW_In1_SEL_chan12] = new RegisterBitField("WaveformControlPT_HW_In1_SEL_chan12", 24, 25);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel1]->BitField[WaveformControlPT_HW_In1_SEL_chan13] = new RegisterBitField("WaveformControlPT_HW_In1_SEL_chan13", 26, 27);

	_FPGAregister[DACWaveGenPerChannelHWTrigSel2] = new FPGA_HardwareReg("DACWaveGenPerChannelHWTrigSel2", 4, BAR3, 0x2CC, 0, WRITEONLY);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel2]->BitField[WaveformControlPT_HW_In2_SEL_chan0] = new RegisterBitField("WaveformControlPT_HW_In2_SEL_chan0", 0, 1);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel2]->BitField[WaveformControlPT_HW_In2_SEL_chan1] = new RegisterBitField("WaveformControlPT_HW_In2_SEL_chan1", 2, 3);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel2]->BitField[WaveformControlPT_HW_In2_SEL_chan2] = new RegisterBitField("WaveformControlPT_HW_In2_SEL_chan2", 4, 5);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel2]->BitField[WaveformControlPT_HW_In2_SEL_chan3] = new RegisterBitField("WaveformControlPT_HW_In2_SEL_chan3", 6, 7);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel2]->BitField[WaveformControlPT_HW_In2_SEL_chan4] = new RegisterBitField("WaveformControlPT_HW_In2_SEL_chan4", 8, 9);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel2]->BitField[WaveformControlPT_HW_In2_SEL_chan5] = new RegisterBitField("WaveformControlPT_HW_In2_SEL_chan5", 10, 11);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel2]->BitField[WaveformControlPT_HW_In2_SEL_chan6] = new RegisterBitField("WaveformControlPT_HW_In2_SEL_chan6", 12, 13);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel2]->BitField[WaveformControlPT_HW_In2_SEL_chan7] = new RegisterBitField("WaveformControlPT_HW_In2_SEL_chan7", 14, 15);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel2]->BitField[WaveformControlPT_HW_In2_SEL_chan8] = new RegisterBitField("WaveformControlPT_HW_In2_SEL_chan8", 16, 17);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel2]->BitField[WaveformControlPT_HW_In2_SEL_chan9] = new RegisterBitField("WaveformControlPT_HW_In2_SEL_chan9", 18, 19);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel2]->BitField[WaveformControlPT_HW_In2_SEL_chan10] = new RegisterBitField("WaveformControlPT_HW_In2_SEL_chan10", 20, 21);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel2]->BitField[WaveformControlPT_HW_In2_SEL_chan11] = new RegisterBitField("WaveformControlPT_HW_In2_SEL_chan11", 22, 23);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel2]->BitField[WaveformControlPT_HW_In2_SEL_chan12] = new RegisterBitField("WaveformControlPT_HW_In2_SEL_chan12", 24, 25);
	_FPGAregister[DACWaveGenPerChannelHWTrigSel2]->BitField[WaveformControlPT_HW_In2_SEL_chan13] = new RegisterBitField("WaveformControlPT_HW_In2_SEL_chan13", 26, 27);

	_FPGAregister[DACWaveGenPerChannelDigitalTrigSel] = new FPGA_HardwareReg("DACWaveGenPerChannelDigitalTrigSel", 7, BAR3, 0x2D0, 0, WRITEONLY);
	_FPGAregister[DACWaveGenPerChannelDigitalTrigSel]->BitField[WaveformControlPT_WaveformIN_SEL_chan0] = new RegisterBitField("WaveformControlPT_WaveformIN_SEL_chan0", 0, 3);
	_FPGAregister[DACWaveGenPerChannelDigitalTrigSel]->BitField[WaveformControlPT_WaveformIN_SEL_chan1] = new RegisterBitField("WaveformControlPT_WaveformIN_SEL_chan1", 4, 7);
	_FPGAregister[DACWaveGenPerChannelDigitalTrigSel]->BitField[WaveformControlPT_WaveformIN_SEL_chan2] = new RegisterBitField("WaveformControlPT_WaveformIN_SEL_chan2", 8, 11);
	_FPGAregister[DACWaveGenPerChannelDigitalTrigSel]->BitField[WaveformControlPT_WaveformIN_SEL_chan3] = new RegisterBitField("WaveformControlPT_WaveformIN_SEL_chan3", 12, 15);
	_FPGAregister[DACWaveGenPerChannelDigitalTrigSel]->BitField[WaveformControlPT_WaveformIN_SEL_chan4] = new RegisterBitField("WaveformControlPT_WaveformIN_SEL_chan4", 16, 19);
	_FPGAregister[DACWaveGenPerChannelDigitalTrigSel]->BitField[WaveformControlPT_WaveformIN_SEL_chan5] = new RegisterBitField("WaveformControlPT_WaveformIN_SEL_chan5", 20, 23);
	_FPGAregister[DACWaveGenPerChannelDigitalTrigSel]->BitField[WaveformControlPT_WaveformIN_SEL_chan6] = new RegisterBitField("WaveformControlPT_WaveformIN_SEL_chan6", 24, 27);
	_FPGAregister[DACWaveGenPerChannelDigitalTrigSel]->BitField[WaveformControlPT_WaveformIN_SEL_chan7] = new RegisterBitField("WaveformControlPT_WaveformIN_SEL_chan7", 28, 31);
	_FPGAregister[DACWaveGenPerChannelDigitalTrigSel]->BitField[WaveformControlPT_WaveformIN_SEL_chan8] = new RegisterBitField("WaveformControlPT_WaveformIN_SEL_chan8", 32, 35);
	_FPGAregister[DACWaveGenPerChannelDigitalTrigSel]->BitField[WaveformControlPT_WaveformIN_SEL_chan9] = new RegisterBitField("WaveformControlPT_WaveformIN_SEL_chan9", 36, 39);
	_FPGAregister[DACWaveGenPerChannelDigitalTrigSel]->BitField[WaveformControlPT_WaveformIN_SEL_chan10] = new RegisterBitField("WaveformControlPT_WaveformIN_SEL_chan10", 40, 43);
	_FPGAregister[DACWaveGenPerChannelDigitalTrigSel]->BitField[WaveformControlPT_WaveformIN_SEL_chan11] = new RegisterBitField("WaveformControlPT_WaveformIN_SEL_chan11", 44, 47);
	_FPGAregister[DACWaveGenPerChannelDigitalTrigSel]->BitField[WaveformControlPT_WaveformIN_SEL_chan12] = new RegisterBitField("WaveformControlPT_WaveformIN_SEL_chan12", 48, 51);
	_FPGAregister[DACWaveGenPerChannelDigitalTrigSel]->BitField[WaveformControlPT_WaveformIN_SEL_chan13] = new RegisterBitField("WaveformControlPT_WaveformIN_SEL_chan13", 52, 55);

	_FPGAregister[DACWaveGenPerChannelEOFFreeze] = new FPGA_HardwareReg("DACWaveGenPerChannelEOFFreeze", 2, BAR3, 0x2D8, 0, WRITEONLY);
	_FPGAregister[DACWaveGenPerChannelEOFFreeze]->BitField[DACWavegenEOFFreezeEnable_chan0] = new RegisterBitField("DACWavegenEOFFreezeEnable_chan0", 0, 0);
	_FPGAregister[DACWaveGenPerChannelEOFFreeze]->BitField[DACWavegenEOFFreezeEnable_chan1] = new RegisterBitField("DACWavegenEOFFreezeEnable_chan1", 1, 1);
	_FPGAregister[DACWaveGenPerChannelEOFFreeze]->BitField[DACWavegenEOFFreezeEnable_chan2] = new RegisterBitField("DACWavegenEOFFreezeEnable_chan2", 2, 2);
	_FPGAregister[DACWaveGenPerChannelEOFFreeze]->BitField[DACWavegenEOFFreezeEnable_chan3] = new RegisterBitField("DACWavegenEOFFreezeEnable_chan3", 3, 3);
	_FPGAregister[DACWaveGenPerChannelEOFFreeze]->BitField[DACWavegenEOFFreezeEnable_chan4] = new RegisterBitField("DACWavegenEOFFreezeEnable_chan4", 4, 4);
	_FPGAregister[DACWaveGenPerChannelEOFFreeze]->BitField[DACWavegenEOFFreezeEnable_chan5] = new RegisterBitField("DACWavegenEOFFreezeEnable_chan5", 5, 5);
	_FPGAregister[DACWaveGenPerChannelEOFFreeze]->BitField[DACWavegenEOFFreezeEnable_chan6] = new RegisterBitField("DACWavegenEOFFreezeEnable_chan6", 6, 6);
	_FPGAregister[DACWaveGenPerChannelEOFFreeze]->BitField[DACWavegenEOFFreezeEnable_chan7] = new RegisterBitField("DACWavegenEOFFreezeEnable_chan7", 7, 7);
	_FPGAregister[DACWaveGenPerChannelEOFFreeze]->BitField[DACWavegenEOFFreezeEnable_chan8] = new RegisterBitField("DACWavegenEOFFreezeEnable_chan8", 8, 8);
	_FPGAregister[DACWaveGenPerChannelEOFFreeze]->BitField[DACWavegenEOFFreezeEnable_chan9] = new RegisterBitField("DACWavegenEOFFreezeEnable_chan9", 9, 9);
	_FPGAregister[DACWaveGenPerChannelEOFFreeze]->BitField[DACWavegenEOFFreezeEnable_chan10] = new RegisterBitField("DACWavegenEOFFreezeEnable_chan10", 10, 10);
	_FPGAregister[DACWaveGenPerChannelEOFFreeze]->BitField[DACWavegenEOFFreezeEnable_chan11] = new RegisterBitField("DACWavegenEOFFreezeEnable_chan11", 11, 11);
	_FPGAregister[DACWaveGenPerChannelEOFFreeze]->BitField[DACWavegenEOFFreezeEnable_chan12] = new RegisterBitField("DACWavegenEOFFreezeEnable_chan12", 12, 12);
	_FPGAregister[DACWaveGenPerChannelEOFFreeze]->BitField[DACWavegenEOFFreezeEnable_chan13] = new RegisterBitField("DACWavegenEOFFreezeEnable_chan13", 13, 13);

	// Xilinx CORE I2C interface (PG090, v2.0)
	_FPGAregister[I2C_AXIBridge] = new FPGA_HardwareReg("I2C_AXIBridge", 8, BAR3, 0x300, 0, WRITEONLY);
	_FPGAregister[I2C_AXIStatus] = new FPGA_HardwareReg("I2C_AXIStatus", 4, BAR3, 0x300, 0, READONLY);
	_FPGAregister[I2C_AXIControl] = new FPGA_HardwareReg("I2C_AXIControl", 8, BAR3, 0x308, 0, WRITEONLY);
	_FPGAregister[I2C_AXIControl]->BitField[I2C_AXIwrite] = new RegisterBitField("I2C_AXIwrite", 0, 0);
	_FPGAregister[I2C_AXIControl]->BitField[I2C_AXIread] = new RegisterBitField("I2C_AXIread", 1, 1);
	_FPGAregister[I2C_AXIControl]->BitField[TI9548AChipReset_] = new RegisterBitField("TI9548AChipReset_", 2, 2);

	// TBD - allow table/array?
	_FPGAregister[DACWavePlayDMAdescriptorChan0] = new FPGA_HardwareReg("DACWavePlayDMAdescriptorChan0", 8, BAR3, 0x50000, 0, WRITEONLY);
	_FPGAregister[DACWavePlayDMAdescriptorChan1] = new FPGA_HardwareReg("DACWavePlayDMAdescriptorChan1", 8, BAR3, 0x50008, 0, WRITEONLY);
	_FPGAregister[DACWavePlayDMAdescriptorChan2] = new FPGA_HardwareReg("DACWavePlayDMAdescriptorChan2", 8, BAR3, 0x50010, 0, WRITEONLY);
	_FPGAregister[DACWavePlayDMAdescriptorChan3] = new FPGA_HardwareReg("DACWavePlayDMAdescriptorChan3", 8, BAR3, 0x50018, 0, WRITEONLY);
	_FPGAregister[DACWavePlayDMAdescriptorChan4] = new FPGA_HardwareReg("DACWavePlayDMAdescriptorChan4", 8, BAR3, 0x50020, 0, WRITEONLY);
	_FPGAregister[DACWavePlayDMAdescriptorChan5] = new FPGA_HardwareReg("DACWavePlayDMAdescriptorChan5", 8, BAR3, 0x50028, 0, WRITEONLY);
	_FPGAregister[DACWavePlayDMAdescriptorChan6] = new FPGA_HardwareReg("DACWavePlayDMAdescriptorChan6", 8, BAR3, 0x50030, 0, WRITEONLY);
	_FPGAregister[DACWavePlayDMAdescriptorChan7] = new FPGA_HardwareReg("DACWavePlayDMAdescriptorChan7", 8, BAR3, 0x50038, 0, WRITEONLY);
	_FPGAregister[DACWavePlayDMAdescriptorChan8] = new FPGA_HardwareReg("DACWavePlayDMAdescriptorChan8", 8, BAR3, 0x50040, 0, WRITEONLY);
	_FPGAregister[DACWavePlayDMAdescriptorChan9] = new FPGA_HardwareReg("DACWavePlayDMAdescriptorChan9", 8, BAR3, 0x50048, 0, WRITEONLY);
	_FPGAregister[DACWavePlayDMAdescriptorChan10] = new FPGA_HardwareReg("DACWavePlayDMAdescriptorChan10", 8, BAR3, 0x50050, 0, WRITEONLY);
	_FPGAregister[DACWavePlayDMAdescriptorChan11] = new FPGA_HardwareReg("DACWavePlayDMAdescriptorChan11", 8, BAR3, 0x50058, 0, WRITEONLY); // last of the 12 ABBx channels
	_FPGAregister[DACWavePlayDMAdescriptorChan12] = new FPGA_HardwareReg("DACWavePlayDMAdescriptorChan12", 8, BAR3, 0x50060, 0, WRITEONLY); // DO waveform (Chan 12, MUX D0-7)
	_FPGAregister[DACWavePlayDMAdescriptorChan13] = new FPGA_HardwareReg("DACWavePlayDMAdescriptorChan13", 8, BAR3, 0x50068, 0, WRITEONLY); // DO waveform (Chan 13, MUX D8-15)
	// END SHADOW REGISTERS //

	// Instantiate the ProgrammableTrigger
	CThordaq::ImageAcqTrigger = new ProgrammableTrigger(*this, -1);


	const LPCWSTR MUTEXname = { L"nwlDMAmutex" };		// MUTEX 'name' for the NWL DMA descriptor buffer hardware resource
	_nwlDMAdescriptorsMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		MUTEXname);

	GetDMAcapability(); // query the NWL interface


	// Legacy I2C LED (3U BBox uses different controller chip)
	DBB1_I2C = new I2C_BBox_ChipPCA9554("DBB1LEDs", 0x02, 0xFF, 0x23);
	ABB1_I2C = new I2C_BBox_ChipPCA9554("ABB1LEDs", 0x08, 0x01, 0x23);
	ABB2_I2C = new I2C_BBox_ChipPCA9554("ABB2LEDs", 0x08, 0x02, 0x23);
	ABB3_I2C = new I2C_BBox_ChipPCA9554("ABB3LEDs", 0x08, 0x08, 0x23);


	_NWL_Common_DMA_Register = 0;
	_usrIrqWaitStruct.boardNum = gBoardIndex;      // routine args:  boardIndex, DMA Bank indicator, int. timeout, etc.
	_usrIrqWaitStruct.DMA_Bank = 0;
	_usrIrqWaitStruct.NWL_Common_DMA_Register_Block = &_NWL_Common_DMA_Register;
	_dacWaveformCurrentCycleCount = 0;
	_dacStaticLoadNumberOfDescPerInterrupt = 0;
	_dacStaticLoadInterruptsPerCycle = 0;
	_dacDynamicLoadLastDescSampleCount = 0;
	_imagingStartStopStatus = false;
	_irqThread = NULL;
	_dacDescriptorCountTrackingThread = NULL;
	_imagingDACBankSwitchingTrackingThread = NULL;
	_dacDMADescTable = new DAC_DESCP_TABLE();
	_dacDMAMultibankDescTable = new DAC_MULTIBANK_DESCP_TABLE();
	_dacCtrl = DAC_FREERUN_WAVEFORM_CONFIG();

	for (UINT8 i = 0; i < WAVETABLE_CHANNEL_COUNT; i++)
	{
		_DACWaveGenInterruptCounter[i] = 0;
		_DACWaveGenInterruptCountWhenApproachingCallbackCalled[i] = 0;
		_dacDescriptorsPerChannel[i] = 0;
		_dacDescCountToTriggerApproachingNSamplesEvent[i] = 0;
		_dacDescCountToTriggerApproachinLoadedWaveformEndEvent[i] = 0;
		_dacSampleCountToTriggerCycleCompleteEvent[i] = 0;
	}

	_abortReadImage = false;
	_dacAbortContinousMode = false;
	_dacProcessingPresetWaveformForDynamicLoading = false;
	_dacDynamicLoadingPresetWaveform = false;


	_dacSecondBankWaveformDDR3StartAddress = 0;
	_dacBankSwitchingReadyForNextWaveformCallbackSettingsList = std::vector<DACBankSwitchingReadyForNextWaveformCallbackSettings>();

	_DACPerChannelRuntBitSelection = 0;
	_rearmDAC1 = 0;
	_rearmDAC0 = 0;
	_captureActiveLinveInvert = false;

	bool bstatus = SearchFPGAregFieldIndices("DACWaveGen3PSyncControlReg", (int)strlen("DACWaveGen3PSyncControlReg"), &_DACWaveGen3PSyncControlRegIndex, &_nullBitField);
	_nullBitField = -1;
	bstatus = SearchFPGAregFieldIndices("DACWaveGenISR", (int)strlen("DACWaveGenISR"), &_DACWaveGenISRIndex, &_nullBitField);

	bstatus = SearchFPGAregFieldIndices("S2MMDMA_ControlReg1", (int)strlen("S2MMDMA_ControlReg1"), &_S2MMDMAControlRegIndex[0], &_nullBitField);
	bstatus = SearchFPGAregFieldIndices("S2MMDMA_ControlReg2", (int)strlen("S2MMDMA_ControlReg2"), &_S2MMDMAControlRegIndex[1], &_nullBitField);
	bstatus = SearchFPGAregFieldIndices("S2MMDMA_ControlReg3", (int)strlen("S2MMDMA_ControlReg3"), &_S2MMDMAControlRegIndex[2], &_nullBitField);
	bstatus = SearchFPGAregFieldIndices("S2MMDMA_ControlReg4", (int)strlen("S2MMDMA_ControlReg4"), &_S2MMDMAControlRegIndex[3], &_nullBitField);
	bstatus = SearchFPGAregFieldIndices("S2MMDMA_StatusReg1", (int)strlen("S2MMDMA_StatusReg1"), &_S2MMDMAStatusRegIndex[0], &_nullBitField);
	bstatus = SearchFPGAregFieldIndices("S2MMDMA_StatusReg2", (int)strlen("S2MMDMA_StatusReg2"), &_S2MMDMAStatusRegIndex[1], &_nullBitField);
	bstatus = SearchFPGAregFieldIndices("S2MMDMA_StatusReg3", (int)strlen("S2MMDMA_StatusReg3"), &_S2MMDMAStatusRegIndex[2], &_nullBitField);
	bstatus = SearchFPGAregFieldIndices("S2MMDMA_StatusReg4", (int)strlen("S2MMDMA_StatusReg4"), &_S2MMDMAStatusRegIndex[3], &_nullBitField);

	_dacContinuousModeStartStopStatus = false;
	_imagingEventPrepared = false;
	_dacContinuousModeEventPrepared = false;
	_dacDescpListIndex = WAVETABLE_CHANNEL_COUNT - 1;
	_dacParkingPositions = std::map<UINT, double>();
	_enableDownsamplingRateChange = 0;
	for (UINT i = 0; i < WAVETABLE_CHANNEL_COUNT; i++)
	{
		_dacParkingPositions[i] = 0;
	}

	for (UINT i = 0; i < DAC_ANALOG_CHANNEL_COUNT; i++)
	{
		_dacParkingPositionsFPGA[i] = 0x8000; //0 volts equivalent
	}

	_isPreviewImaging = false;

	_stoppingDAC = false;
	_dacChannelsInitParkedAtZero = false;
	_imagingLevelTriggerWentLow = false;
	_imagingLevelTriggerActive = false;
	_MAfilterAlignmentOffset = 0;
	//clear the descriptors
	memset(_dacDMADescTable->descps, 0, sizeof(ULONG64) * DAC_DESCP_LENGTH);
	memset(_dacDescriptorsPerChannel, 0, sizeof(_dacDescriptorsPerChannel));
	memset(_isDACChannelEnabledForImaging, 0, sizeof(bool) * WAVETABLE_CHANNEL_COUNT);
	memset(_dacBankSwitchingPerChannelAddressOffset, 0, sizeof(ULONG64) * WAVETABLE_CHANNEL_COUNT);
	unique_ptr<ThorDAQSettingsXML> pSetup(new ThorDAQSettingsXML());
	INT64 lastConnectionTime = 0;

	wchar_t errMsg[MSG_SIZE];
	try
	{
		if (FALSE == pSetup->GetDACLastParkingPositions(_dacParkingPositionsFPGA))
		{

			StringCbPrintfW(errMsg, MSG_SIZE, L"GetLastHWConnectionTime from ThorDAQSettings.XML failed");
			LogMessage(errMsg, ERROR_EVENT);
		}

	}
	catch (...)
	{
		// we don't mind missing XML fields, but if the entire .XML file is MISSING, we should fail
		StringCbPrintfW(errMsg, MSG_SIZE, L"ThorDAQSettings.XML file not found - fatal error");
		LogMessage(errMsg, ERROR_EVENT);
	}

	try
	{
		if (FALSE == pSetup->GetLastHWConnectionTime(lastConnectionTime))
		{

			StringCbPrintfW(errMsg, MSG_SIZE, L"GetLastHWConnectionTime from ThorDAQSettings.XML failed");
			LogMessage(errMsg, ERROR_EVENT);
		}

	}
	catch (...)
	{
		// we don't mind missing XML fields, but if the entire .XML file is MISSING, we should fail
		StringCbPrintfW(errMsg, MSG_SIZE, L"ThorDAQSettings.XML file not found - fatal error");
		LogMessage(errMsg, ERROR_EVENT);
	}

	_lastShutdownTypeSinceFPGAConnection = GetShutDownTypeSinceLastConnection(lastConnectionTime);


	return;
} // END OF CONSTRUCTOR

/**********************************************************************************************//**
 * @fn	CThordaq::~CThordaq()
 *
 * @brief	Destructor.
 *
 * @author	Cge
 * @date	3/17/2017
 **************************************************************************************************/

CThordaq::~CThordaq()
{
	// free Programmable Triggers from heap
/*
	if(ImageAcqTrigger != nullptr)
		delete ImageAcqTrigger;
	for (int d = 0; d < 14; d++)
	{
		if(WafeformControlTrigger[d] != nullptr)
			delete WafeformControlTrigger[d];
	}
	*/
	bExitBlinkLEDTask = true;
	Sleep(550); // time for thread exit
	delete BOB_DIOconfiguration; // free the BOB config array

	// free up the I2C device allocations
	if (_hXI2Cmutex != NULL)
	{
		ReleaseMutex(_hXI2Cmutex); // make certain it's released
		CloseHandle(_hXI2Cmutex);
		_hXI2Cmutex = NULL;
	}

	//	delete MainBrdEEPROM;
	delete DBB1_I2C;
	delete ABB1_I2C;
	delete ABB2_I2C;
	delete ABB3_I2C;

	//***reset all global available
	for (int r = 0; r < TD_LAST_FPGA_REGISTER; r++)
	{
		for (int f = 0; f < MAX_REGISTER_FIELDS; f++)
		{
			if (_FPGAregister[r]->BitField[f] == NULL) break;
			SAFE_DELETE_PTR(_FPGAregister[r]->BitField[f]);
		}
		SAFE_DELETE_PTR(_FPGAregister[r]);
	}
	gBoardIndex = -1; //no board is connected 
	// Reset to no DMA Engines found
	gDmaInfo.PacketRecvEngineCount = 0;
	gDmaInfo.PacketSendEngineCount = 0;
	gDmaInfo.AddressablePacketMode = false;
	//*** Free all allocated thordaq struct
	SAFE_DELETE_PTR(gPtrAcqCtrl);
	if (gDeviceInfo != NULL)
	{
		SetupDiDestroyDeviceInfoList(gDeviceInfo); // Free the Info handle
		gDeviceInfo = NULL;
	}

	// lastly, close handle to kernel driver
	CloseHandle(gHdlDevice);

	return;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS CThordaq::APIBindToBoard()
 *
 * @brief	ThorDAQAPI Bind/Connect to the board.
 *
 * @author	DZimmerman
 * @date	6/1/2020
 *
 * @return	STATUS_SUCCESSFUL if Thordaq binds, error otherwise
 **************************************************************************************************/

THORDAQ_STATUS CThordaq::GetDMAcapability()
{
	THORDAQ_STATUS status;
	INT32 error;
	// Get Board Configuration. 
	// Initiate onBoard parameters.

	// 
	ThordaqErrChk(L"CopyKernelBoardCfg", status = CopyKernelBoardCfg()); // puts kernel data in our private struct
	if (status != STATUS_SUCCESSFUL)
	{
		return STATUS_GET_BOARD_CONFIG_ERROR;
	}

	if (Board_CONFIG_data.DriverVersionMajor < MIN_DRIVER_VERSION_MAJOR)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, _MAX_PATH, L"ThorDAQ driver version Should be greater or equal to %d.%d.%d", MIN_DRIVER_VERSION_MAJOR, MIN_DRIVER_VERSION_MINOR, MIN_DRIVER_VERSION_SUBMINOR);
		MessageBox(NULL, errMsg, L"Out of date driver", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
		return STATUS_GET_BOARD_CONFIG_ERROR;
	}

	if (Board_CONFIG_data.DriverVersionMajor == MIN_DRIVER_VERSION_MAJOR)
	{
		if (Board_CONFIG_data.DriverVersionMinor < MIN_DRIVER_VERSION_MINOR)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, _MAX_PATH, L"ThorDAQ driver version Should be greater or equal to %d.%d.%d", MIN_DRIVER_VERSION_MAJOR, MIN_DRIVER_VERSION_MINOR, MIN_DRIVER_VERSION_SUBMINOR);
			MessageBox(NULL, errMsg, L"Out of date driver", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
			return STATUS_GET_BOARD_CONFIG_ERROR;
		}
	}


	if (Board_CONFIG_data.DriverVersionMajor == MIN_DRIVER_VERSION_MAJOR && Board_CONFIG_data.DriverVersionMinor < MIN_DRIVER_VERSION_MINOR)
	{
		if (Board_CONFIG_data.DriverVersionSubMinor < MIN_DRIVER_VERSION_SUBMINOR)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, _MAX_PATH, L"ThorDAQ driver version Should be greater or eaqual to %d.%d.%d", MIN_DRIVER_VERSION_MAJOR, MIN_DRIVER_VERSION_MINOR, MIN_DRIVER_VERSION_SUBMINOR);
			MessageBox(NULL, errMsg, L"Out of date driver", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
			return STATUS_GET_BOARD_CONFIG_ERROR;
		}
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
	for (char i = 0; i < MAX_NUM_DMA_ENGINES; i += 32)  // there are only 2 engines - READ at index 0, WRITE at index 32
	{
		gDmaInfo.PacketRecvEngine[i] = -1;
		gDmaInfo.PacketSendEngine[i] = -1;
		ThordaqErrChk(L"GetDMAEngineCap", GetDMAEngineCap(i, &DMA_cap));

		if ((DMA_cap.dma_capabilityabilities & DMA_CAP_ENGINE_PRESENT) == DMA_CAP_ENGINE_PRESENT)
		{
			if ((DMA_cap.dma_capabilityabilities & DMA_CAP_ENGINE_TYPE_MASK) & DMA_CAP_PACKET_DMA)
			{
				if ((DMA_cap.dma_capabilityabilities & DMA_CAP_ENGINE_TYPE_MASK) & DMA_CAP_ADDRESSABLE_PACKET_DMA)
				{
					gDmaInfo.AddressablePacketMode = true;
				}
				if ((DMA_cap.dma_capabilityabilities & DMA_CAP_DIRECTION_MASK) == DMA_CAP_SYSTEM_TO_CARD)
				{
					gDmaInfo.PacketSendEngine[gDmaInfo.PacketSendEngineCount++] = i;
				}
				else if ((DMA_cap.dma_capabilityabilities & DMA_CAP_DIRECTION_MASK) == DMA_CAP_CARD_TO_SYSTEM)
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

	// make sure the GlobalDMA is enabled
	// no read/write to DDR3, under any circumstances, is possible without the
	// NWL IRQ enabled -- if possible, verify what is resetting this bit on startup
	status = FPGAregisterWRITE("NWL_GlobalDMAIntEnable", 1);
	UINT64 regValue = 0;
	status = FPGAregisterREAD("NWL_DDR3_DMAcore", &regValue);

	////reset all write registers
	//for (int r = 0; r < TD_LAST_FPGA_REGISTER; r++)
	//{
	//	FPGAregisterWRITE(_FPGAregister[r]->_RegName, 0x0);
	//}

	if (!IsDACMappingConfigured())
	{
		switch (_lastShutdownTypeSinceFPGAConnection)
		{
		case ShutDownType::RESTART:
			//park the galvo when starting
			DACSetParkValueForChannels(_dacParkingPositionsFPGA);
			break;
		case ShutDownType::NOSHUTDOWN:
			break;
		case ShutDownType::POWER_OFF:
		case ShutDownType::UNKNOWN:
			for (UINT i = 0; i < DAC_ANALOG_CHANNEL_COUNT; ++i)
			{
				SetDACParkValue(i, 0.0);
			}
			break;
		default:
			break;
		}
		_lastShutdownTypeSinceFPGAConnection = ShutDownType::NOSHUTDOWN;
	}

	SetDefaultBOBMapping(false);

	wchar_t logMsg[MSG_SIZE];
	try
	{

		unique_ptr<ThorDAQSettingsXML> pSetup(new ThorDAQSettingsXML());
		INT64 lastConnectionTime = 0;
		auto now = std::chrono::system_clock::now();
		auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
		auto value = now_ms.time_since_epoch();
		INT64 now_duration = value.count();
		//keep track of the last time we connected to thordaq
		if (FALSE == pSetup->SetLastHWConnectionTime(now_duration))
		{
			StringCbPrintfW(logMsg, MSG_SIZE, L"SetLastHWConnectionTime from ThorDAQSettings.XML failed");
			LogMessage(logMsg, ERROR_EVENT);
		}

	}
	catch (...)
	{
		// we don't mind missing XML fields, but if the entire .XML file is MISSING, we should fail
		StringCbPrintfW(logMsg, MSG_SIZE, L"ThorDAQSettings.XML file not found - fatal error");
		LogMessage(logMsg, ERROR_EVENT);
	}


	StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::GetDMAcapability(), DevHandle = 0x%p, NWL_DDR3_DMAcore 0x%x", gHdlDevice, (int)regValue);
	CThordaq::LogMessage(logMsg, VERBOSE_EVENT);
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CThordaq::SetDefaultBOBMapping(bool initialParkAllAtZero)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	if (true == initialParkAllAtZero && false == _dacChannelsInitParkedAtZero)
	{
		for (UINT i = 0; i < DAC_ANALOG_CHANNEL_COUNT; i++)
		{
			SetDACParkValue(i, 0.0);
		}

		_dacChannelsInitParkedAtZero = true;
	}
	DiscoverBOBtype(*this);
	UINT64 ABBXmux;
	FPGAregisterREAD("GlobalABBXmuxReg", &ABBXmux);
	if ('P' == BOB_HardwareType)
	{
		if (ABBXmux == 0) // "illegal" power-up value
		{
			status = FPGAregisterWRITE("GlobalABBXmuxReg", CThordaq::Panel3UDACdefaultChannelMap);
			status = SetDefaultDIOConfig();
		}
	}
	else
	{
		if (ABBXmux == 0) // "illegal" power-up value
		{
			status = FPGAregisterWRITE("GlobalABBXmuxReg", CThordaq::LegacyDACdefaultChannelMap);
		}
	}
	status = FPGAregisterWRITE("SCRATCH_BITS_WRITE", 0x01);
	return status;
}

bool CThordaq::IsDACMappingConfigured()
{
	UINT64 uiValue;
	THORDAQ_STATUS status = FPGAregisterREAD("SCRATCH_BITS_READ", &uiValue);

	if (status != THORDAQ_STATUS::STATUS_SUCCESSFUL)
	{
		return false;
	}

	return uiValue != 0x0;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS CThordaq:: DisconnectFromBoard()
 *
 * @brief	Disconnect to the device.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @return	STATUS_SUCCESSFUL if Thordaq disconnected. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS CThordaq::DisconnectFromBoard()
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;
	wchar_t logMsg[MSG_SIZE];
	StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::DisconnectFromBoard(), DevHandle = 0x%p", gHdlDevice);
	CThordaq::LogMessage(logMsg, VERBOSE_EVENT);
	//Free Device handle
//	if (gHdlDevice != INVALID_HANDLE_VALUE)
//	{
		// in revised design to ONLY bind board object to correct Prod ID (e.g.0x4001)
		// the handle to kernel driver remains until the CThordaq object is destroyed
		// because the handle is assigned at object construction
	//		CloseHandle(gHdlDevice);
//		gHdlDevice = INVALID_HANDLE_VALUE;
//	}
	// Free up device detail ONLY in destructor
//	if (gDeviceInterfaceDetailData != NULL)
//	{
//		free(gDeviceInterfaceDetailData);
//		gDeviceInterfaceDetailData = NULL;
//	}

	unique_ptr<ThorDAQSettingsXML> pSetup(new ThorDAQSettingsXML());
	INT64 lastConnectionTime = 0;

	wchar_t errMsg[MSG_SIZE];
	try
	{
		if (FALSE == pSetup->SetDACLastParkingPositions(_dacParkingPositionsFPGA))
		{

			StringCbPrintfW(errMsg, MSG_SIZE, L"GetLastHWConnectionTime from ThorDAQSettings.XML failed");
			LogMessage(errMsg, ERROR_EVENT);
		}

	}
	catch (...)
	{
		// we don't mind missing XML fields, but if the entire .XML file is MISSING, we should fail
		StringCbPrintfW(errMsg, MSG_SIZE, L"ThorDAQSettings.XML file not found - fatal error");
		LogMessage(errMsg, ERROR_EVENT);
	}

	return STATUS_SUCCESSFUL;
}


/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS CThordaq::CopyKernelBoardCfg ( BOARD_INFO_STRUCT* board_info )
 * @brief	Get kernel driver data for this board and store in object
 *
 * @author	DZimmerman
 * @date	16-Aug-23

 **************************************************************************************************/
THORDAQ_STATUS CThordaq::CopyKernelBoardCfg()
{
	THORDAQ_STATUS			status = STATUS_SUCCESSFUL;
	DWORD					bytes_returned = 0;
	DWORD					last_error_status = 0;
	//	BOARD_CONFIG_STRUCT     board_config;
	OVERLAPPED overlapped;  // OVERLAPPED structure for the operation

	//Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// Send GET_BOARD_CONFIG_IOCTL
	if (!DeviceIoControl(gHdlDevice, GET_BOARD_CONFIG_IOCTL, NULL, 0, (LPVOID)&Board_CONFIG_data, sizeof(BOARD_CONFIG_STRUCT), &bytes_returned, &overlapped))
	{
		last_error_status = GetLastError();

		if (last_error_status != ERROR_IO_PENDING)
		{
#if _DEBUG
			printf("GetBoardCfg IOCTL call failed. Error = %d\n", GetLastError());
#endif // _DEBUG            
			status = STATUS_GET_BOARD_CONFIG_ERROR;
		}
		else
		{
			// Wait here (forever) for the Overlapped I/O to complete
			if (!GetOverlappedResult(gHdlDevice, &overlapped, &bytes_returned, TRUE))
			{
				last_error_status = GetLastError();
#if _DEBUG
				printf("GetBoardCfg IOCTL call failed. Error = %d\n", last_error_status);
#endif // _DEBUG
				status = STATUS_GET_BOARD_CONFIG_ERROR;
			}
		}
	}

	// check returned structure size
	if ((bytes_returned != sizeof(BOARD_CONFIG_STRUCT)) && (status == STATUS_SUCCESSFUL))
	{
		// ioctl failed
#if _DEBUG
		printf("GetBoardCfg IOCTL returned invalid size (%d)\n", bytes_returned);
#endif // _DEBUG
		status = STATUS_GET_BOARD_CONFIG_ERROR;
	}

	if (overlapped.hEvent != 0)
		CloseHandle(overlapped.hEvent);
	return status;
}



/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS CThordaq:: GetBoardCfg ( BOARD_INFO_STRUCT* board_info )
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

THORDAQ_STATUS CThordaq::GetBoardCfg(
	BOARD_INFO_STRUCT* board_info  // Returned structure
)
{
	THORDAQ_STATUS			status = STATUS_SUCCESSFUL;
	DWORD					bytes_returned = 0;
	DWORD					last_error_status = 0;
	BOARD_CONFIG_STRUCT     board_config;
	OVERLAPPED overlapped;  // OVERLAPPED structure for the operation

	// Initiate Firmware and Driver version number
	board_info->DriverVersionBuildNumber = 0;
	board_info->DriverVersionMajor = 0;
	board_info->DriverVersionMinor = 0;
	board_info->DriverVersionSubMinor = 0;
	board_info->UserVersion = 0;

	//Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	DWORD ioctlCode = GET_BOARD_CONFIG_IOCTL;
	// Send GET_BOARD_CONFIG_IOCTL
	if (!DeviceIoControl(gHdlDevice, ioctlCode, NULL, 0, (LPVOID)&board_config, sizeof(BOARD_CONFIG_STRUCT), &bytes_returned, &overlapped))
	{
		last_error_status = GetLastError();

		if (last_error_status != ERROR_IO_PENDING)
		{
#if _DEBUG
			printf("GetBoardCfg IOCTL call failed. Error = %d\n", GetLastError());
#endif // _DEBUG            
			status = STATUS_GET_BOARD_CONFIG_ERROR;
		}
		else
		{
			// Wait here (forever) for the Overlapped I/O to complete
			if (!GetOverlappedResult(gHdlDevice, &overlapped, &bytes_returned, TRUE))
			{
				last_error_status = GetLastError();
#if _DEBUG
				printf("GetBoardCfg IOCTL call failed. Error = %d\n", last_error_status);
#endif // _DEBUG
				status = STATUS_GET_BOARD_CONFIG_ERROR;
			}
		}
	}

	// check returned structure size
	if ((bytes_returned != sizeof(BOARD_CONFIG_STRUCT)) && (status == STATUS_SUCCESSFUL))
	{
		// ioctl failed
#if _DEBUG
		printf("GetBoardCfg IOCTL returned invalid size (%d)\n", bytes_returned);
#endif // _DEBUG
		status = STATUS_GET_BOARD_CONFIG_ERROR;
	}
	else
	{
		board_info->DriverVersionBuildNumber = board_config.DriverVersionBuildNumber;
		board_info->DriverVersionMajor = board_config.DriverVersionMajor;
		board_info->DriverVersionMinor = board_config.DriverVersionMinor;
		board_info->DriverVersionSubMinor = board_config.DriverVersionSubMinor;
		board_info->UserVersion = board_config.UserVersion;
		board_info->PCIVendorDeviceID = board_config.PciConfig.DeviceId;
	}
	if (overlapped.hEvent != 0)
		CloseHandle(overlapped.hEvent);
	return status;
}

/// <summary>
/// This function is used to get the low frquency board info
/// It was placed here to have it in a general place where it can be consumed by
/// All SDKs consuming the ThorDAQ API
/// </summary>
/// <param name="board_info"></param>
/// <returns></returns>
THORDAQ_STATUS CThordaq::GetLowFreqBoardCfg(
	LOW_FREQ_TRIG_BOARD_INFO_STRUCT* board_info  // Returned structure
)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	UINT32 MAX_I2C_SLAVE_WRITE_Len = (6 + 16);
	UINT8* MGMSG_HW_REQ_INFO = new  UINT8[MAX_I2C_SLAVE_WRITE_Len]{ 0x05, 0x00, 0x00, 0x00, 0x67, 0x1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	const UINT MasterMUXChan = 0x1;
	const UINT SlaveMUXChan = 0xff;
	const UINT TargetSlaveAddr = 0x67; // CPLD address on DAC board
	const int PageSize = 16;
	bool bI2C_SlaveRead = true;
	const int TRIGcardFW_VER_LEN = 18;
	const int I2CbusHz = 100;
	UINT I2CtransferLen = 18;
	UINT8* FWverDataBytes = new UINT8[TRIGcardFW_VER_LEN]();

	const UINT TDMasterI2CMUXAddr = 0x71;
	const int TDSlaveI2CMUXAddr = 0x70;

	status = APIXI2CReadWrite(*this, bI2C_SlaveRead,
		TDMasterI2CMUXAddr, MasterMUXChan,
		TDSlaveI2CMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, PageSize,
		MGMSG_HW_REQ_INFO, &MAX_I2C_SLAVE_WRITE_Len, // "OpCodeLen" IN: Opcode byte count (in buffer)
		FWverDataBytes, &I2CtransferLen);

	if (status == THORDAQ_STATUS::STATUS_SUCCESSFUL)
	{
		const int sizeOfLTFAppName = 8;

		//We receive an app name from the ltf board firmware through the i2c call.
		//There are at least 2 versions for this firmware (3P and dFLIM)
		for (int i = 0; i < sizeOfLTFAppName; ++i)
		{
			board_info->AppFW += (wchar_t)FWverDataBytes[i + 4];
		}

		board_info->CPLDUsercodeMajor = FWverDataBytes[0];
		board_info->CPLDUsercodeSubMajor = FWverDataBytes[1];
		board_info->CPLDUsercodeMinor = FWverDataBytes[2];
		board_info->CPLDUsercodeSubMinor = FWverDataBytes[3];
		board_info->FWVerMajor = FWverDataBytes[16];
		board_info->FWVerMinor = FWverDataBytes[15];
		board_info->FWVerSubMinor = FWverDataBytes[14];
		board_info->boardConnected = true;
	}
	else
	{
		board_info->boardConnected = false;
	}

	return status;
}

//////// NWL restore SDK functions

inline THORDAQ_STATUS CThordaq::DoMem(
	UINT32          Rd_Wr_n,        // 1==Read, 0==Write
	UINT32          BarNum,         // Base Address Register (BAR) to access
	PUINT8          Buffer,         // Data buffer
	UINT64          Offset,         // Offset in data buffer to start transfer
	UINT64          CardOffset,     // Offset in BAR to start transfer
	UINT64          Length,         // Byte length of transfer
	PSTAT_STRUCT    Status          // Completion Status
)
{
	DO_MEM_STRUCT   doMemStruct;
	OVERLAPPED      os;         // OVERLAPPED structure for the operation
	DWORD           bytesReturned = 0;
	DWORD           ioctlCode;
	DWORD           LastErrorStatus = 0;
	UINT32          status = STATUS_SUCCESSFUL;

	os.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// fill in the doMem Structure
	doMemStruct.BarNum = BarNum;
	doMemStruct.Offset = Offset;
	doMemStruct.CardOffset = CardOffset;
	doMemStruct.Length = Length;

	// determine the ioctl code
	if (Rd_Wr_n == READ_FROM_CARD)
	{
		ioctlCode = DO_MEM_READ_ACCESS_IOCTL;
	}
	else
	{
		ioctlCode = DO_MEM_WRITE_ACCESS_IOCTL;
	}

	// Send DoMem IOCTL
	if (!DeviceIoControl(gHdlDevice, ioctlCode,
		(LPVOID)&doMemStruct, sizeof(DO_MEM_STRUCT),
		(LPVOID)Buffer, (DWORD)Length,
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
				printf("DoMem IOCTL call failed. Error = %d\n", LastErrorStatus);
#endif // _DEBUG
				status = LastErrorStatus;
			}
		}
		else
		{
			// ioctl failed
			Status->CompletedByteCount = 0;
#if _DEBUG
			printf("DoMem IOCTL call failed. Error = %d\n", LastErrorStatus);
#endif // _DEBUG
			status = LastErrorStatus;
		}
	}
	// save the returned size
	Status->CompletedByteCount = bytesReturned;
	// check returned structure size
//  printf("DoMem bytesReturned=%d, Length=%d\n", bytesReturned, Length);

	if ((bytesReturned != Length) &&
		(status == STATUS_SUCCESSFUL))
	{
		// ioctl failed
#if _DEBUG
		printf("DoMem IOCTL returned invalid size (%d), expected length %llu\n", bytesReturned, Length);
#endif // _DEBUG
		status = STATUS_INCOMPLETE;
	}
	if (os.hEvent != 0)
		CloseHandle(os.hEvent);
	return (THORDAQ_STATUS)status;
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
THORDAQ_STATUS CThordaq::SetupPacket(
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

			// Send Setup Packet Mode Addressable IOCTL
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
THORDAQ_STATUS CThordaq::PacketWriteEx
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

THORDAQ_STATUS CThordaq::PacketReadEx(
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
					printf("Packet Read Overlapped failed. Error = %d\n", LastErrorStatus);
#endif // _DEBUG
					status = LastErrorStatus;
				}
			}
			else
			{
				// ioctl failed
#if _DEBUG
				printf("Packet Read failed, Error = %d\n", LastErrorStatus);
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
				printf("Packet Read failed. Return structure size is mismatched (Ret=%d)\n", bytesReturned);
#endif // _DEBUG
				status = STATUS_INCOMPLETE;
			}
		}
	} // if (EngineOffset ...
	else
	{
#if _DEBUG
		printf("DLL: Packet Read failed. No Packet Read Engine\n");
#endif // _DEBUG
		status = STATUS_INVALID_MODE;
	}
	CloseHandle(os.hEvent);
	return (THORDAQ_STATUS)status;
}

/*! ReleasePacketBuffers
 *
 * \brief Sends two PACKET_BUF_DEALLOC_IOCTL calls to the driver to teardown the recieve buffer
 *  and teardown the descriptors for sending packets
 * \param EngineOffset - DMA Engine number offset to use
 * \return Completion status.
 */
THORDAQ_STATUS CThordaq::ReleasePacketBuffers(
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
		BufDeAlloc.RxBufferAddress = pRxPacketBufferHandle; // always set to NULL in NWL original code

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
					pRxPacketBufferHandle = NULL;
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
			pRxPacketBufferHandle = NULL;
			status = STATUS_SUCCESSFUL;
		}
	}
	CloseHandle(os.hEvent);
	BOOL bStatus = ReleaseMutex(_nwlDMAdescriptorsMutex);

	return status;
}

THORDAQ_STATUS ReadWriteI2CDevice(
	CThordaq& TdBrd, // This CThordaq object by reference	
	I2C_BBox_ChipPCA9554& I2Cdev,
	BOOL Read, // TRUE if reading, else writing
	UINT32 SlaveCommandByte,   // what particular command (I2C slave specific)
	PUINT8  Buffer,
	UINT32* BufByteLen)  // IN: size of buffer, OUT: bytes read or written
{
	THORDAQ_STATUS status;

	status = I2Cdev.SetupI2CMux(TdBrd, TRUE); // always do master
	if (status != STATUS_SUCCESSFUL)
	{
		// certain I2C devices (like BBox LED) will fail on a sequential scan
		// of I2C DeviceAddress - make one additional attempt before failing

		status = I2Cdev.SetupI2CMux(TdBrd, TRUE); // always do master
		if (status != STATUS_SUCCESSFUL)
			return STATUS_I2C_INVALIDMUX;
	}
	if (I2Cdev.HasSlaveMUX() == TRUE)
	{
		status = I2Cdev.SetupI2CMux(TdBrd, FALSE); // the slave
		if (status != STATUS_SUCCESSFUL)
			return STATUS_I2C_INVALIDMUX;
	}
	// MUX(es) are setup - now operate on the I2C device
	status = I2Cdev.ExecuteI2CreadWrite(TdBrd, I2Cdev, Read, SlaveCommandByte, Buffer, BufByteLen);

	return status;
}


// Logic for Xlinx I2C master combined-write function
// 
/* Base_Addr = 0x300          BAR 3

I2C:  AXI_Lite_Bridge_Register: BAR 3, Base_Addr + 0x00, Write Only

--   Bit 31 – 0 : Xilinx I2C IP AXI - Lite Data bus bits

--   Bit 63 – 32 : Xilinx I2C IP AXI - Lite Address bus bits

I2C:  AXI - Lite_Bridge_Control Register : BAR 3, Base_Addr + 0x08, Write Only

--   Bit 0 : AXI - Lite Write
--         A 0-> 1 Transition causes address and data in AXI_Lite_Bridge_Register to be written to the Xilinx I2C IP core.
--   Bit 1: AXI - Lite Read
--         A 0-> 1 Transition causes the address in AXI_Lite_Bridge_Register to be read and stored in the AXI_Lite_Bridge_Status register from the Xilinx I2C IP core.
--   Bit 2: TI9548A MUX Reset (asserted low)
			Direct connect from bit to external I2C MUX chip reset, 1 -> 0 -> 1 completes reset


I2C:  AXI_Lite_Status_Register: BAR 3, Base_Addr + 0x00, Read Only

--   Bit 31 - 0 : Data being read back from Xilinx I2C IP core
*/

// <LOCAL (private) function...>
// Continuously read Xilinx AIX core for status bit "bState"
// Return:  TRUE	desired bState recevied before timeout
//          FALSE	timed out
bool AIXI2C_pollCoreSTATUSregisterForState(CThordaq& TdBrd, ULONG Timeout, UINT64 BitMask, bool bState) // bit state we poll for
{
	UINT64 StatusReg;
	volatile ULONG ulTimeout = Timeout; // poll loop timeout
	do
	{
		XI2C->ReadI2CAIXBridge(TdBrd, XI2Ccontroller::Status_Register, &StatusReg);
	} while (((bool)(StatusReg & BitMask) != bState) && (--ulTimeout != 0));
	if (ulTimeout == 0)
	{
		return false;
	}
	return true;
}
// <LOCAL (private) function...>

// <LOCAL (private) function...>
// We need a "Address-WriteOpcodes" function because of the MUXed I2C bus
// Every target slave device lies behind one or two MUXes, meaning we must
// set up correct MUX chain, then access device.  Each time a MUX is setup,
// Another ReStart-Address must be performed, and when the
// final (TargetSlaveDevice) is reached, we may need ReStart to change
// direction from WRITE to READ (for the same Target address)
// This programming sequence requires DYNAMIC CONTROLLER LOGIC FLOW!
// Among other things, Dynamic automatically handles "Start" vs. "ReStart"
const unsigned int I2CByteMicroSecs = 110; // nominal time for hardware transmission of single I2C byte (@ 100 Hz)
THORDAQ_STATUS Write_I2C_BusAddressPlusBytes(
	CThordaq& TdBrd, // This CThordaq object by reference	
	XI2Ccontroller* XI2C,
	UINT32 BusAddress, // i.e. 7-bit address (must be shifted)
	bool bI2C_ReadDirection,
	PUINT8 TransmitByteBuffer,
	UINT32* TransmitByteLen,   // can be 0 bytes when we ONLY change Read/Write direction
	bool bReadNOTWrite,  // sets Bit#0 in address byte
	bool bRequireStop,    // necessary because Opcode(s) might be end of I2C message
	PUINT8 ReceiveByteBuffer,
	UINT32* ReceiveBufferLen   // if reading, IN: numberBytesExpected  OUT: numberBytesReceived
)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	UINT64 AXIbridgeValue;
	UINT64 ReadNOTWrite = 0; // vast majority of cases -- we have something to WRITE before READ, or we just WRITE
	UINT32 indx;
	UINT32 totalBytesTransferred = 0; // Opcode bytes + Data bytes IN or OUT (don't include Address byte)
	BusAddress = (BusAddress << 1) & 0xFE; // shift and make space for Read/Write bit
	if (bI2C_ReadDirection == true && *TransmitByteLen == 0) // READ with NO preceding write (OpCode) bytes!
		ReadNOTWrite = 1; // force bit0


	AXIbridgeValue = (0x100 | ReadNOTWrite) | BusAddress; // bit 0 of ADDR is "0" for WRITE, bit #8 START for dynamic FPGA logic

	// this can be a "Start" or "Restart" - make sure I2C BUS isn't still busy with previous byte
	ULONG ulTimeout = 0x02000; // poll loop timeout
	bool bBusNotBusy = AIXI2C_pollCoreSTATUSregisterForState(TdBrd, ulTimeout, XI2Ccontroller::BB, false);
	if (bBusNotBusy == false)
	{
		return STATUS_I2C_TIMEOUT_ERR;
	}

	XI2C->WriteI2CAIXBridge(TdBrd, XI2Ccontroller::TX_FIFO_Reg, AXIbridgeValue); // this (S)tarts I2C bus
	// NOTE!!  We do NOT have "NAK" status bit, per se
	// if the BusAddress does not exist, or if it exists and issues NAK, 
	// the Xilinx I2C master reponds with "Stop" bit; otherwise, it retains control
	// of I2C bus and stretches SCL waiting for next byte (or other core command)
	// We must write the next byte to find out!
	// If there was a NAK, when the next byte is written, it will NOT be transmitted,
	// because the message was already terminated by I2C master core logic...
	// 
	// write OpCode bytes into TX_FIFO as fast as possible, only pausing if TX_FIFO is full (TBD)
	// FIFO depth 16 bytes
	for (indx = 0; indx < *TransmitByteLen; indx++)
	{
		AXIbridgeValue = TransmitByteBuffer[indx];
		// In some cases, the last Opcode is the END of the I2C message
		if (((indx + 1) == *TransmitByteLen) && (bRequireStop == true)) // end of "OpCode" len yet?
		{
			AXIbridgeValue |= 0x200; // add the "STOP" for Dynamic logic ONLY IF we don't intend "ReStart" to change DIR to READ
		}
		XI2C->WriteI2CAIXBridge(TdBrd, XI2Ccontroller::TX_FIFO_Reg, AXIbridgeValue);

		// if the I2C Slave Address and byte(s) were flushed to hardware, the implication 
		// is device exists and data is successfully sent - this condition is reflected by "empty" status
		// POLL for empty status is only high performance option - if we WaitForSingleObject()
		// it will incur millisecs delay when we ask for nanoseconds
		volatile ULONG ulTimeout = 0x0400; // poll loop timeout - NOTE this varies widely on WRITE to particular hardware device
		bool bTX_FIFOEmpty;
		bTX_FIFOEmpty = AIXI2C_pollCoreSTATUSregisterForState(TdBrd, ulTimeout, XI2Ccontroller::TX_FIFO_Empty, true);

		if (bTX_FIFOEmpty) // byte pushed onto wire
		{
			totalBytesTransferred++;
		}
		else  // timeout - always happens with NAK
		{
			if (totalBytesTransferred == 0)
			{
				status = STATUS_I2C_INVALIDDEV;
			}
			else
			{
				status = STATUS_I2C_TIMEOUT_ERR;
			}
			// interpret this scenario -- no bytes written, NAK from NoExistingDevice or FailedDevice, as fatal
		}
	}
	// are we Reading?  Don't continue if previous start/address byte failed
	if (bReadNOTWrite == true && (status == STATUS_SUCCESSFUL))
	{

		if (*TransmitByteLen > 0) // did we WRITE OpCodes? 
		{
			// we need a ReStart: resend Address with Bit#0 set to READ (1)
			// wait for OpCode bytes
			AXIbridgeValue = 0x101 | BusAddress; // bit 0 of ADDR is "0" for WRITE, bit #8 START for dynamic FPGA logic
			XI2C->WriteI2CAIXBridge(TdBrd, XI2Ccontroller::TX_FIFO_Reg, AXIbridgeValue);
		}
		// write the expected RECEIVE byte count from Slave
		AXIbridgeValue = 0x200 | (*ReceiveBufferLen); // byte count we expect from slave, with automatic Master NAK when count satisfied
		XI2C->WriteI2CAIXBridge(TdBrd, XI2Ccontroller::TX_FIFO_Reg, AXIbridgeValue);

		// RECEIVE bytes from slave....
		// For each byte, poll for RX_FIFO_Empty Status Register bit being CLEAR (it is set after SOFTR)
		// with timeout
		UINT32 incomingByteIndx;
		ulTimeout = 0x1500;
		bool ByteReady;
		for (incomingByteIndx = 0; incomingByteIndx < *ReceiveBufferLen; incomingByteIndx++)
		{
			ByteReady = AIXI2C_pollCoreSTATUSregisterForState(TdBrd, ulTimeout, XI2Ccontroller::RX_FIFO_Empty, false);
			if (ByteReady)
			{
				// Get byte from RX_FIFO
				XI2C->ReadI2CAIXBridge(TdBrd, XI2Ccontroller::RX_FIFO_Reg, &AXIbridgeValue);
				// 
				ReceiveByteBuffer[incomingByteIndx] = (UINT8)AXIbridgeValue;
				// 
				// Read Byte from the RX_FIFO
				totalBytesTransferred++;
			}
			else  // error condition - give up
			{
				status = STATUS_I2C_TIMEOUT_ERR;
				break;
			}
		}
	}
	*TransmitByteLen = totalBytesTransferred;
	return status;
}

// This Xilinx I2C Master is protected with hardware MUTEX!
// Returned total bytes written includes opcode bytes (if any)
// NAK from slave at any time terminates, meaning less than expected
// bytes may be written
THORDAQ_STATUS CThordaq::APIXI2CReadWrite(
	CThordaq& TdBrd, // This CThordaq object by reference	
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
)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	bool b400Hz;
	if (I2CbusHz == 400) {
		b400Hz = true;
	}
	else {
		b400Hz = false;

	}

	FPGAregisterWRITE("GIGCR0_LED1", 1); // turn ON at entry, OFF at exit (detect hang at BAR3, 0x300 AXI bridge access)

	//	bool b400Hz = (I2CbusHz == 400) ? true : false;  // only two speeds for now - LFT/MCU needs 100 kHz, other slaves 400 kHz

	XI2C = new XI2Ccontroller(TdBrd, b400Hz);  // constructor may reset device.... ?
	//	UINT64 StatusReg, ControlReg ;
	/* test*/ UINT64 THDSTA, TSUSTA, TSUDAT, TBUF, THIGH, TLOW, THDDAT;
	UINT32 I2CbytesThisTransmit = 0;
	UINT32 ExpectedTotalTransferLen = *OpCodeLen + *BiDirectionalDataLen; // PageWrites modify - could transfer less (or 0) on error
	UINT32 I2CtotalBytesWritten = 0; // accumulator for OpCodeBuff/TxBuff bytes written
	bool bAddStopBit = false;
	bool bI2CslaveAddrReadDirection = false;
	UINT8 SetupOpCodeBuf[4]; // for local use!

	// TEST - what are default settings by Xilinx core?  
	// Use registers to change speed from 100 kHz (def.) to 400 kHz
	XI2C->ReadI2CAIXBridge(TdBrd, XI2Ccontroller::THDSTA_Register, &THDSTA);  // units: 5ns "ticks" count
	XI2C->ReadI2CAIXBridge(TdBrd, XI2Ccontroller::TSUSTA_Register, &TSUSTA);
	XI2C->ReadI2CAIXBridge(TdBrd, XI2Ccontroller::TSUDAT_Register, &TSUDAT);
	XI2C->ReadI2CAIXBridge(TdBrd, XI2Ccontroller::TBUF_Register, &TBUF);
	XI2C->ReadI2CAIXBridge(TdBrd, XI2Ccontroller::THIGH_Register, &THIGH);
	XI2C->ReadI2CAIXBridge(TdBrd, XI2Ccontroller::TLOW_Register, &TLOW);
	XI2C->ReadI2CAIXBridge(TdBrd, XI2Ccontroller::THDDAT_Register, &THDDAT);


	// for "Dynamic" operation...
	// set max RX_FIFO depth 
	XI2C->WriteI2CAIXBridge(TdBrd, XI2Ccontroller::RX_FIFO_PIRQ_Register, 0x8); // MAX RX FIFO depth is 0xF (16)

	// Use "Dynamic Controller Logic Flow", Xilinx AXI IIC Bus Interface v2.0, PG090, pg. 36 of 61
	// Load the TxFIFO (max depth 16) with address and "OpCode" data if any
	// The Read/Write bit that is sent with an address must be the
	// LSB of the address written into the TX_FIFO.
	// NOTE: MSMS (master slave mode select) in CR appears to be automatically set by writing TX_FIFO with "START" set

	// 1. Always send MasterMUXaddress MasterMUXChan
	I2CbytesThisTransmit = 1; // address byte NOT included
	SetupOpCodeBuf[0] = (UINT8)MasterMUXChan;
	bAddStopBit = true;  // MUX does not switch channel until stoP bit received!
	status = Write_I2C_BusAddressPlusBytes(TdBrd, XI2C, MasterMUXAddr, bI2C_Read, SetupOpCodeBuf, &I2CbytesThisTransmit, bI2CslaveAddrReadDirection, bAddStopBit, BiDirectionalDataBuffer, BiDirectionalDataLen);
	if ((status == STATUS_SUCCESSFUL) && (I2CbytesThisTransmit == 1))
	{
		// 2. If SlaveMUX defined, send it next 
		if (SlaveMUXChan != 0xFF)
		{
			bAddStopBit = true;
			bI2CslaveAddrReadDirection = false;
			I2CbytesThisTransmit = 1; // address byte NOT included
			SetupOpCodeBuf[0] = (UINT8)SlaveMUXChan;
			status = Write_I2C_BusAddressPlusBytes(TdBrd, XI2C, SlaveMUXAddr, bI2C_Read, SetupOpCodeBuf, &I2CbytesThisTransmit, bI2CslaveAddrReadDirection, bAddStopBit, BiDirectionalDataBuffer, BiDirectionalDataLen);
			if ((status != STATUS_SUCCESSFUL) || (I2CbytesThisTransmit != 1))
			{
				goto I2C_Completed;
			}
		}
		//  MUXes are set up!  Ready to access the targeted slave device

		// 3. Are we WRITING TargetDevice?  

		if (!bI2C_Read)    //                WRITING to Target I2C Slave
		{
			// If needed, break writes into "PageSize" segments if needed
#define MAX_I2_CDEV_PAGESIZE 256
#define MAX_I2C_OPCODE_LEN 8
			UINT8 TransmitByteBuffer[MAX_I2C_OPCODE_LEN + MAX_I2_CDEV_PAGESIZE]; // local write buffer to merge "opcode" address with following data -
			// may be limited by PAGE size (defined if != 0)

// first check for special case of NO DATA, only opcodes (e.g. CPLD commands)
			if (*BiDirectionalDataLen == 0) // send ALL opcodes (PageSize not applicable)
			{
				for (int n = 0; n < (int)*OpCodeLen; n++)             // start with SlaveDevIndex for write
				{
					TransmitByteBuffer[n] = OpCodeBuffer[n];
				}
				I2CbytesThisTransmit = *OpCodeLen;
				status = Write_I2C_BusAddressPlusBytes(TdBrd, XI2C, DevAddress, bI2C_Read, TransmitByteBuffer, &I2CbytesThisTransmit, bI2CslaveAddrReadDirection, bAddStopBit, BiDirectionalDataBuffer, BiDirectionalDataLen);
				I2CtotalBytesWritten += I2CbytesThisTransmit;
			}
			else
			{
				// PageSize can be as small as single byte, or as large as "MAX_I2_CDEV_PAGESIZE"
				// Range check...
				INT32 RangeCheckedPageSize;
				RangeCheckedPageSize = (PageSize == 0) ? 1 : PageSize;
				RangeCheckedPageSize = (PageSize > MAX_I2_CDEV_PAGESIZE) ? MAX_I2_CDEV_PAGESIZE : PageSize;

				int ByteCountThisPage, TotalDATAbytesToWrite;
				TotalDATAbytesToWrite = *BiDirectionalDataLen;
				// we need a stoP bit after each "Page" write
				if (RangeCheckedPageSize == 1)
				{
					ByteCountThisPage = 1; //  I2C stoP bit after each byte!
				}
				else // initialize first page write
				{
					ByteCountThisPage = (RangeCheckedPageSize <= (INT32)*BiDirectionalDataLen) ? RangeCheckedPageSize
						: TotalDATAbytesToWrite % RangeCheckedPageSize; // ByteCountThisPage is less than or equal to PageSize!
				}
				// finally -- IF we have multiple pages, slow devices (like EEPROMs) 
				// need delay between page writes
				int PageIndex = 0;
				bool bPageWriteDelayNeeded = (*BiDirectionalDataLen > (UINT32)RangeCheckedPageSize) ? true : false;
				do
				{
					// send a "page" size of data each I2C WRITE command!
					bAddStopBit = true; // stop bit required to WRITE DATA to device
					bI2CslaveAddrReadDirection = false;
					I2CbytesThisTransmit = *OpCodeLen + ByteCountThisPage; // indxAddr[for now single byte] + data
					// combine Addr+Data
					int n, m = 0;
					for (n = 0; n < (int)*OpCodeLen; n++, m++)             // start with SlaveDevIndex or OpCode(s) for write
					{
						TransmitByteBuffer[m] = OpCodeBuffer[n];
					}
					for (n = 0; n < ByteCountThisPage; n++, m++)  // add on the DATA
					{
						TransmitByteBuffer[m] = BiDirectionalDataBuffer[n + (PageIndex * RangeCheckedPageSize)]; // add offset into caller's data buffer
					}
					status = Write_I2C_BusAddressPlusBytes(TdBrd, XI2C, DevAddress, bI2C_Read, TransmitByteBuffer, &I2CbytesThisTransmit, bI2CslaveAddrReadDirection, bAddStopBit, BiDirectionalDataBuffer, BiDirectionalDataLen);
					I2CtotalBytesWritten += I2CbytesThisTransmit;
					if ((status != STATUS_SUCCESSFUL))
					{
						goto I2C_Completed;
					}
					// decrement total byte count for next iteration
					TotalDATAbytesToWrite -= ByteCountThisPage;
					OpCodeBuffer[0] += RangeCheckedPageSize;  // must address on Page boundary
					PageIndex++;  // we wrote a "page" - not necessarily a complete page but bytes into a paged-addressed-boundary
					// next page ByteCount
					ByteCountThisPage = (RangeCheckedPageSize <= TotalDATAbytesToWrite) ? RangeCheckedPageSize
						: TotalDATAbytesToWrite % RangeCheckedPageSize; // ByteCountThisPage is less than or equal to PageSize!

					// The issue here -- we may have done a WRITE to slow device 
					// (e.g. M24C08-W EEPROM which takes up to 5ms to complete hardware WRITE).
					// If we IMMEDIATELY attempt the next page WRITE, we can receive a NAK if previous write not done.
					// 
					if (bPageWriteDelayNeeded)
						Sleep(5); // WORST CASE time (M24C08 EEPROM chip) - we can optimize this (TDB)
				} while (TotalDATAbytesToWrite > 0);
			}
		}
		else  //                READING from TargetSlave - we need "RESTART" and to load expected target Read BYTE count
		{
			// 3-B  send OpCode(x) -- this means READ direction for I2C slave
			bAddStopBit = false; // Xilinx Dynamic logic will send NAK after last EXPECTED byte is received
			bI2CslaveAddrReadDirection = true; // we need the call to return Slave data bytes...
			I2CbytesThisTransmit = *OpCodeLen; // address byte NOT included
			status = Write_I2C_BusAddressPlusBytes(TdBrd, XI2C, DevAddress, bI2C_Read, OpCodeBuffer, &I2CbytesThisTransmit, bI2CslaveAddrReadDirection, bAddStopBit, BiDirectionalDataBuffer, BiDirectionalDataLen);
			I2CtotalBytesWritten += I2CbytesThisTransmit;
			if ((status != STATUS_SUCCESSFUL) || (I2CbytesThisTransmit < *OpCodeLen))
			{
				goto I2C_Completed;
			}
		}
	}

	// since our code is FAR faster than slow I2C hardware, poll the SR "BB" (Bus Busy)
	// bit for completion

	volatile ULONG ulTimeout = 0x01000; // poll loop timeout
	bool bBusNotBusy = AIXI2C_pollCoreSTATUSregisterForState(TdBrd, ulTimeout, XI2Ccontroller::BB, false);

I2C_Completed:
	XI2C->WriteI2CAIXBridge(TdBrd, XI2Ccontroller::Control_Register, 0); // DISable 
	delete(XI2C);
	*BiDirectionalDataLen = I2CtotalBytesWritten;
	FPGAregisterWRITE("GIGCR0_LED1", 0); // turn ON at entry, OFF at exit (detect hang at BAR3, 0x300 AXI bridge access)
	return status;
}


////////////////////////////////////////////
THORDAQ_STATUS CThordaq::APIReadI2CDevice(
	CThordaq& TdBrd, // This CThordaq object by reference	
	UINT32 MasterChan,
	INT32 SlaveChan,
	UINT32 DevAddress,
	UINT32 SlaveCommandByte,
	PUINT8 ReadBuffer,
	UINT32* ByteLen  // IN: ReadBuffer Len, OUT: byte length read
)
{
	THORDAQ_STATUS status;
	I2C_BBox_ChipPCA9554* MappedI2Cdev = new I2C_BBox_ChipPCA9554("NetworkDev", MasterChan, SlaveChan, DevAddress);

	status = ReadWriteI2CDevice(TdBrd, *MappedI2Cdev, TRUE, SlaveCommandByte, ReadBuffer, ByteLen);
	// all done
	delete MappedI2Cdev;
	return status;
}
THORDAQ_STATUS CThordaq::APIWriteI2CDevice(
	CThordaq& TdBrd, // This CThordaq object by reference	
	UINT32 MasterChan,
	INT32 SlaveChan,
	UINT32 DevAddress,
	UINT32 SlaveCommandByte,
	PUINT8 WriteBuffer,
	UINT32* ByteLen  // IN: ReadBuffer Len, OUT: byte length read
)
{
	THORDAQ_STATUS status;
	I2C_BBox_ChipPCA9554* MappedI2Cdev = new I2C_BBox_ChipPCA9554("NetworkDev", MasterChan, SlaveChan, DevAddress);

	status = ReadWriteI2CDevice(TdBrd, *MappedI2Cdev, FALSE, SlaveCommandByte, WriteBuffer, ByteLen);
	// all done
	delete MappedI2Cdev;
	return status;
}

const BYTE LED1 = 0x02;
const BYTE LED2 = 0x01;
const BYTE LED3 = 0x08;
const BYTE LED4 = 0x04;
const BYTE LED5 = 0x20; // note: ABBx boxes skip 5 and 6 (only 6 LEDs)
const BYTE LED6 = 0x10;
const BYTE LED7 = 0x80;
const BYTE LED8 = 0x40;

// for every LED on BreakOut Boxes, map the correct BBox, correct I2CDev within
// that box, and set the hardware LED mask (position) for PCA9554 register control
// 
BreakoutBoxLEDs* GetBboxLEDmask(int LEDenum, int* Mask)
{
	switch (LEDenum)
	{
		// DBB1:
	case lDIO1:
		*Mask = 0x02;
		goto SetDBB1bbox;
	case lDIO2:
		*Mask = 0x01;
		goto SetDBB1bbox;
	case lDIO3:
		*Mask = 0x08;
		goto SetDBB1bbox;
	case lDIO4:
		*Mask = 0x04;
		goto SetDBB1bbox;
	case lDIO5:
		*Mask = 0x20;
		goto SetDBB1bbox;
	case lDIO6:
		*Mask = 0x10;
		goto SetDBB1bbox;
	case lDIO7:
		*Mask = 0x80;
		goto SetDBB1bbox;
	case lDIO8:
		*Mask = 0x40;
	SetDBB1bbox:
		return &DBB1;
		// ABB1:
	case lAO1:
		*Mask = 0x02;
		goto SetABB1box;
	case lAO2:
		*Mask = 0x01;
		goto SetABB1box;
	case lAO3:
		*Mask = 0x08;
		goto SetABB1box;
	case lAO4:
		*Mask = 0x04;
		goto SetABB1box;
	case lAI1:
		*Mask = 0x80;
		goto SetABB1box;
	case lAI2:
		*Mask = 0x40;
	SetABB1box:
		return &ABB1;

		// ABB2:
	case lAO5:
		*Mask = 0x02;
		goto SetABB2box;
	case lAO6:
		*Mask = 0x01;
		goto SetABB2box;
	case lAO7:
		*Mask = 0x08;
		goto SetABB2box;
	case lAO8:
		*Mask = 0x04;
		goto SetABB2box;
	case lAI3:
		*Mask = 0x80;
		goto SetABB2box;
	case lAI4:
		*Mask = 0x40;
	SetABB2box:
		return &ABB2;
		// ABB3:
	case lAO9:
		*Mask = 0x02;
		goto SetABB3box;
	case lAO10:
		*Mask = 0x01;
		goto SetABB3box;
	case lAO11:
		*Mask = 0x08;
		goto SetABB3box;
	case lAO12:
		*Mask = 0x04;
		goto SetABB3box;
	case lAI5:
		*Mask = 0x80;
		goto SetABB3box;
	case lAI6:
		*Mask = 0x40;
	SetABB3box:
		return &ABB3;

	default:
		*Mask = -1; // ERROR - undefined enum
		return nullptr;
	}
}


THORDAQ_STATUS CThordaq::APIGetAI(
	CThordaq& TdBrd,                // Board to target
	INT32 BNCindex,					// enumerator for Label (like "AI2") on BBox for FPGA reg (legacy) or "AI6" (or higher) MAX127 channel
	BOOL bVolts,					// if FALSE, return raw ADC counts instead of voltage
	double* Value			        // raw ADC counts or converted Volts from MAX127 chip (or FPGA reg)
)
{
	UINT32 ADCcounts;
	// FOR NOW, we don't have ability to read LEGACY BBox "slow" Analog Inputs.
	// These legacy AI's, AI1 - AI6 on ABB1 to ABB3, pass through to new 3U IO Panel
	// Consequently, this function is reading the 8 new slow AI channels via
	// the I2C from MAX127 chip

	THORDAQ_STATUS status = (THORDAQ_STATUS)STATUS_INVALID_PARAMETER;
	if (bVolts)
		status = BB_MAX127.Read(TdBrd, BNCindex, Value);
	else
	{
		status = BB_MAX127.Read(TdBrd, BNCindex, &ADCcounts);
		if (status == STATUS_SUCCESSFUL)
		{
			*Value = 1.0 * (double)ADCcounts;
		}
	}
	return status;
}

// For Legacy BOB, config is all in DLL; For 3U Panel BOB, need additional I2C comm with CPLD
// Thordaq's "BOB_DIOconfiguration" is defined as current config
// Input ConfigArray:   DnnXmmMcc, where (all 0-based, decimal)
//	nn - channel number 
//	X  - direction 'I' or 'O'
//  mm - CpySrc, if different from nn and Dir is 'O', the Dnn source of the output (i.e. copied from another output)
//  cc - MUX channel of signal origin, a unique FPGA-defined code (from FPGA's SWUG) or code 48, CPLD configured via I2C
static const char* AuxGPIODir[5] = {
"Aux_GPIO_DIR_0",
"Aux_GPIO_DIR_1",
"Aux_GPIO_DIR_2",
"Aux_GPIO_DIR_3",
"Aux_GPIO_DIR_4"
};
static const char* AuxGPO[5] = {
"Global_Aux_GPO_0",
"Global_Aux_GPO_1",
"Global_Aux_GPO_2",
"Global_Aux_GPO_3",
"Global_Aux_GPO_4",
};
static const char* AuxGPI[5] = {
"Global_Aux_GPI_0",
"Global_Aux_GPI_1",
"Global_Aux_GPI_2",
"Global_Aux_GPI_3",
"Global_Aux_GPI_4",
};
static const char* GlobalDBB1MUXreg[8] = {
"DBB1_IOCIRCUIT_1",     // i.e. BNC label "D0" (3U BOB) or "DIO1" (Legacy DBB1 BOB)
"DBB1_IOCIRCUIT_2",
"DBB1_IOCIRCUIT_3",
"DBB1_IOCIRCUIT_4",
"DBB1_IOCIRCUIT_5",
"DBB1_IOCIRCUIT_6",
"DBB1_IOCIRCUIT_7",
"DBB1_IOCIRCUIT_8",
};


THORDAQ_STATUS  CThordaq::SetBOB_DO(CThordaq& TdBrd, CHAR* ConfigArray, int DOrecordSize, int NumDOs)
{
	THORDAQ_STATUS status = CThordaq::GetOrSetBOB_DIO(TdBrd, ConfigArray, DOrecordSize, NumDOs, TRUE);

	return status;
}
THORDAQ_STATUS  CThordaq::GetBOB_DIO(CThordaq& TdBrd, CHAR* ConfigArray, int DOrecordSize, int NumDOs)
{
	THORDAQ_STATUS status = CThordaq::GetOrSetBOB_DIO(TdBrd, ConfigArray, DOrecordSize, NumDOs, FALSE);

	return status;
}

// we process each "CallersDIOrecord" separately - from 1 to 32 records possible, actual count is "NumDOs"
// each CallersDIOrecord" must have a valid BNC label (0-based index, D0 - D31)
// ALSO use this call to verify the connected BOB (Legacy DBB1 or 3U Panel)
// Because we want the commands executed as QUICKLY AS POSSIBLE in succession, because
// there may be state information gained from fast-as-possible reading, we bundle a
// group of DIOs together in the API.  Nominally, it takes 1 ms real-time per DIO to complete
// reading - PROVIDED there is no Win10 system interruption which can cause 100s of millsec (or more)
// delay between getting or setting DIOs
THORDAQ_STATUS  CThordaq::GetOrSetBOB_DIO(CThordaq& TdBrd, CHAR* ConfigArray, int DOrecordSize, int NumDOs, BOOL bSet)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	CHAR cNum[3];
	CHAR cDIOsetOrClearBuff[TD_BOBDIODef::NumBOB_DIOs][CharPerBOB_DIO];
	int iBNCsToSet[TD_BOBDIODef::NumBOB_DIOs]; // in theory Caller could set/clear all 32 DOs at once
	memset(iBNCsToSet, 0, sizeof(int) * TD_BOBDIODef::NumBOB_DIOs);
	memset(cDIOsetOrClearBuff, 0, TD_BOBDIODef::NumBOB_DIOs * CharPerBOB_DIO);

	// set defaults to invalid numbers
	int BNClabel = 0xFF;
	int iSetOrclearValue = 0xFF;
	// process the passed data to discover all DOs to be set by this call (i.e., from 1 to 32)
	if (DOrecordSize < CharPerBOB_DIO) // caller's buff can be bigger, but not smaller (differing size breaks NumDOs > 1)
		return STATUS_PARAMETER_SETTINGS_ERROR;

	if (NumDOs > TD_BOBDIODef::NumBOB_DIOs || NumDOs < 0) // sanity check arg
		return STATUS_PARAMETER_SETTINGS_ERROR;

	// copy the contiguous array of NumDOs from caller...
	memcpy_s(cDIOsetOrClearBuff, (rsize_t)DOrecordSize * NumDOs, ConfigArray, (rsize_t)DOrecordSize * NumDOs);


	// analyze command for up to NumBOB_DIOs set/clear DIOs
	int CallersDIOrecord = NumDOs - 1;
	while (CallersDIOrecord >= 0) // for all NumDOs, verify config and get caller's Value
	{
		// e.g. "D01X01   "  Set D1
		// e.g. "D03X00   "  Clear D3
		cNum[0] = cDIOsetOrClearBuff[CallersDIOrecord][1]; // get BNC label from caller
		cNum[1] = cDIOsetOrClearBuff[CallersDIOrecord][2];
		cNum[2] = 0;
		BNClabel = atoi(cNum); // the "D" zero-based channel index
		if (BNClabel < 0 || BNClabel > TD_BOBDIODef::NumBOB_DIOs) // BNC # out of range?
			return STATUS_PARAMETER_SETTINGS_ERROR;

		iBNCsToSet[CallersDIOrecord] = BNClabel;  // fill array with Dn index to process

		if (bSet && BOB_DIOSettings[BNClabel].bOutputDir == TRUE) // WRITING value to OUTPUT?
		{
			// we already have the CONFIG - all that's needed is caller's VALUE to write
			cNum[0] = cDIOsetOrClearBuff[CallersDIOrecord][4]; // get VALUE caller
			cNum[1] = cDIOsetOrClearBuff[CallersDIOrecord][5];
			int iValue = atoi(cNum);
			// In original 1:1 mapping, BNClabel = FPGAindex, but when 
			// MUXING (i.e. BNClabel != FPGAindex) we must SET or CLEAR using FPGAindex (not BNClabel)
			// IN THE HARDWARE.  The setting below is SHADOW REGISTER ONLY
			BOB_DIOSettings[BNClabel].OUTvalue = (UINT64)iValue; // the "Shadow" copy, to be written to hardware
		}
		// reflect current config back to caller (replace X)
		if (BOB_DIOSettings[BNClabel].bOutputDir == TRUE)
		{
			cDIOsetOrClearBuff[CallersDIOrecord][3] = 'O';
		}
		else
		{
			cDIOsetOrClearBuff[CallersDIOrecord][3] = 'I';
		}
		cDIOsetOrClearBuff[CallersDIOrecord][CharPerBOB_DIO - 1] = 0; // NULL terminate
		// next record from Caller...
		--CallersDIOrecord;
	}

	// NOW - execute the DO write or read ...
	CallersDIOrecord = 0;
	int BNClabelThisIteration;
	char cBuff;
	BOOL bFPGAcontrolledDIO;


	while (CallersDIOrecord < NumDOs) // for all NumDOs, verify config and get caller's Value
	{
		bFPGAcontrolledDIO = false; // default
		BNClabelThisIteration = iBNCsToSet[CallersDIOrecord];
		// use the FPGA index as the source or destination of the BNC's digital signal!
		if ((BOB_DIOSettings[BNClabelThisIteration].bOutputDir == TRUE))
		{   // we are WRITING
			// The "source" or driver of output is the FPGA, either a "fast" wired index 0-7,
			// or a slow I2C command index 8-31
			if (BOB_DIOSettings[BNClabelThisIteration].FPGAioIndex < 8) // a "fast" FPGA index? 
			{
				INT64 iAuxSrc;

				// write value to FPGA hardware IF it's an AUX GPIO output we control
				// (we cannot control many FPGA outputs purely under FPGA firmware control)
				switch (BOB_DIOSettings[BNClabelThisIteration].MUX)
				{
				case DO_Buffer_Ready:  // ports NOT dependent on FPGA GlobalScan state
				case Aux_GPIO_1:  // which may be configured and set any time 
				case Aux_GPIO_2:
				case Aux_GPIO_3:
				case Aux_GPIO_4:
					// (can we compute the 0-based "index" of the Aux_GPIO_n 
					// assuming they are incremental) ???
					iAuxSrc = BOB_DIOSettings[BNClabelThisIteration].AuxMUX;
					if (iAuxSrc < 0 || iAuxSrc > 4) // sanity check result
						return STATUS_PARAMETER_SETTINGS_ERROR; // invalid Aux_GPIO_ index

					// WRITING value
					// We have zero-based FPGA index - verify it's an OUTPUT !
					if (BOB_DIOSettings[BNClabelThisIteration].bOutputDir != TRUE) // NOT an output?!
						return STATUS_PARAMETER_SETTINGS_ERROR;
					// write "shadow" VALUE we received from API
					status = FPGAregisterWrite(AuxGPO[iAuxSrc], (int)strlen("Global_Aux_GPX_n"), BOB_DIOSettings[BNClabelThisIteration].OUTvalue); // SETs value (via FPGA)
					if (status != STATUS_SUCCESSFUL)
						return STATUS_PARAMETER_SETTINGS_ERROR; // something FAILED, abandon process
					break;
				default:
					// for MOST of the FPGA controlled MUXes (for Input or Output) we cannot access
					// the values through API software - so user will see "XX" for these I/O
					// values when we attempt to read them via API.
					bFPGAcontrolledDIO = true; // cannot write
					break;
				}
			}
			else // an I2C command-controlled index
			{
				// send the I2C command to BOB CPLD
				status = ReadWriteBOB_CPLD_DIO(TdBrd, BNClabelThisIteration);
				if (status != STATUS_SUCCESSFUL)
					return STATUS_PARAMETER_SETTINGS_ERROR; // something FAILED, abandon process
			}
		}
		else // We are READING
		{
			// The "source" or driver of output is the FPGA, either a "fast" wired index 0-7,
			// or a slow I2C command index 8-31
			if (BOB_DIOSettings[BNClabelThisIteration].FPGAioIndex < 8) // a "fast" FPGA index? 
			{
				INT64 iAuxSrc;
				// write value to FPGA hardware IF it's an AUX GPIO output we control
				// (we cannot control many FPGA outputs purely under FPGA firmware control)
				switch (BOB_DIOSettings[BNClabelThisIteration].MUX)
				{
				case DO_Buffer_Ready:  // ports NOT dependent on FPGA GlobalScan state
				case Aux_GPIO_1:  // which may be configured and set any time 
				case Aux_GPIO_2:
				case Aux_GPIO_3:
				case Aux_GPIO_4:
					// (can we compute the 0-based "index" of the Aux_GPIO_n 
					// assuming they are incremental) ???
					iAuxSrc = BOB_DIOSettings[BNClabelThisIteration].AuxMUX;
					if (iAuxSrc < 0 || iAuxSrc > 4) // sanity check result
						return STATUS_PARAMETER_SETTINGS_ERROR; // invalid Aux_GPIO_ index

					UINT64 uiValue;
					// READING value
					// We have zero-based FPGA index - verify DIR !
					if (BOB_DIOSettings[BNClabelThisIteration].bOutputDir == TRUE) // NOT an input?!
						return STATUS_PARAMETER_SETTINGS_ERROR;
					// write "shadow" VALUE we received from API
					status = FPGAregisterRead(AuxGPI[iAuxSrc], (int)strlen("Global_Aux_GPX_n"), &uiValue); // GE value (via FPGA)
					BOB_DIOSettings[BNClabelThisIteration].INvalue = uiValue;
					if (status != STATUS_SUCCESSFUL)
						return STATUS_PARAMETER_SETTINGS_ERROR; // something FAILED, abandon process
					break;
				default:
					// for MOST of the FPGA controlled MUXes (for Input or Output) we cannot access
					// the values through API software - so user will see "XX" for these I/O
					// values when we attempt to read them via API.
					// We can always read AUX GPIOs from FPGA and all I2C controlled I/O
					bFPGAcontrolledDIO = true; // cannot read
					break;
				}
			}
			else // Read "slow" I2C index
			{
				// send the I2C command to BOB CPLD
				status = ReadWriteBOB_CPLD_DIO(TdBrd, (int)BOB_DIOSettings[BNClabelThisIteration].FPGAioIndex);
				if (status != STATUS_SUCCESSFUL)
					return STATUS_PARAMETER_SETTINGS_ERROR; // something FAILED, abandon process
			}
		}

		// return DIO value to caller 
		if (bFPGAcontrolledDIO == TRUE) // we cannot read or write the DIO
		{

			cDIOsetOrClearBuff[CallersDIOrecord][5] = 'x';
			cDIOsetOrClearBuff[CallersDIOrecord][4] = 'x';
		}
		else  // what did we read or write?
		{
			if (BOB_DIOSettings[BNClabelThisIteration].bOutputDir == TRUE) // we WROTE
			{
				cBuff = (BOB_DIOSettings[BNClabelThisIteration].OUTvalue == 0) ? '0' : '1'; // last OUTPUT we wrote
				cDIOsetOrClearBuff[CallersDIOrecord][5] = cBuff; // value in caller's array
				cDIOsetOrClearBuff[CallersDIOrecord][4] = '0';   // value in caller's array 
			}
			else // we READ
			{
				cBuff = (BOB_DIOSettings[BNClabelThisIteration].INvalue == 0) ? '0' : '1'; // last INPUT we read
				cDIOsetOrClearBuff[CallersDIOrecord][5] = cBuff; // value in caller's array
				cDIOsetOrClearBuff[CallersDIOrecord][4] = '0';   // value in caller's array
			}
		}
		CallersDIOrecord++; // next DIO arg...
	}; // end while DIOs to process


	if (bSet == FALSE) // READing?
	{
		// copy the our buffer back to Caller...
		memcpy_s(ConfigArray, (rsize_t)DOrecordSize * NumDOs, cDIOsetOrClearBuff, (rsize_t)DOrecordSize * NumDOs);
	}
	return status;
}

THORDAQ_STATUS  CThordaq::DiscoverBOBtype(CThordaq& TdBrd)
{
	THORDAQ_STATUS I2Cstatus;
	// (0x71 and 0x70 are Master and Slave are hardcoded MUXes on ThorDAQ)
	// (0x3C hardcoded address of 3U Panel IS31FL LED controller, 0x23 is 9554 LED controller [Legacy])
	// if BOB_HardwareType == 'X', we have not discovered type or none is connected
	UINT8 I2CbyteBuffer[64];
	UINT8 I2CopcodeBuffer[8];
	UINT32 I2CbufferLen = 0;
	UINT32 I2CopcodeLen = 0;
	UINT32 LEDi2cAddress = 0x3C; // def. to 3U Panel IS31FL LED Controller
	UINT32 SlaveCmdByte;
	if (BOB_HardwareType == 'X')
	{
		I2Cstatus = STATUS_DEVICE_NOT_EXISTS;
		// we must discover BOB attached (if any) - we can detect by which I2C device answers
		// start with IS31FL LED chip
		I2CbufferLen = 2;
		I2CbyteBuffer[0] = 0; 	I2CbyteBuffer[1] = 1; // Shutdown Reg 0x0, set to "Normal" operation
		// WRITE the device to verify existence
		I2Cstatus = APIXI2CReadWrite(TdBrd, FALSE, 0x71, 0x2, 0x70, 0xFF, LEDi2cAddress, 400, 36, I2CopcodeBuffer, &I2CopcodeLen, I2CbyteBuffer, &I2CbufferLen);

		if (I2Cstatus == STATUS_SUCCESSFUL)  // 3U Panel found?
		{
			BOB_HardwareType = 'P';  // done!
		}
		else // try the Legacy DBB1 (chip PCA9554)
		{
			LEDi2cAddress = 0x23;
			SlaveCmdByte = 0x2; // polarity inversion register (leave at default 0)
			I2CbufferLen = 1;
			I2C_BBox_ChipPCA9554* MappedI2Cdev = new I2C_BBox_ChipPCA9554("LEDcontrol", 0x8, 0x1, LEDi2cAddress); // (ABB1 used alone by dFLIM)
			// READ device to verify existence
			I2Cstatus = ReadWriteI2CDevice(TdBrd, *MappedI2Cdev, TRUE, SlaveCmdByte, I2CbyteBuffer, &I2CbufferLen);
			delete MappedI2Cdev;  // done
			if (I2Cstatus == STATUS_SUCCESSFUL)  // Legacy DBB1 found?
			{
				BOB_HardwareType = 'L';  // done!
			}
		}

		// Did we find BOB?
		if (I2Cstatus != STATUS_SUCCESSFUL)
		{
			//// caller will log error, if desired, because this error may be hit many hundreds of times
			////  for single cable disconnect
			//// 
			//// additional logic to decipher mixed up HDMI cabling??
			//return STATUS_DEVICE_NOT_EXISTS;

			//TODO: We should find a way to test for the Prelude BOB, and identify it as such. 
			//for now we are calling legacy any unrecognized device, to be able to use the prelude.
			//once we find out how to  check for prelude we should put back the logic for STATUS_DEVICE_NOT_EXISTS if it's not
			//a 3U BOB, Legacy, or Prelude BOB
			BOB_HardwareType = 'L';  //Prelude 
		}
	}
	return STATUS_SUCCESSFUL;
}


static const char* GlobalABBxMUXreg[12] = {
"DAC_DMA00",     // see FPGA SWUG
"DAC_DMA01",
"DAC_DMA02",
"DAC_DMA03",
"DAC_DMA04",
"DAC_DMA05",
"DAC_DMA06",
"DAC_DMA07",
"DAC_DMA08",
"DAC_DMA09",
"DAC_DMA10",
"DAC_DMA11",
};

// private function to reconfigure FPGA DAC MUX
// CRITICAL PRESUMPTION is the current 48bit MUX complies with
// MUTUALLY EXCLUSIVE rule, that single 4bit MUX value only appears in one position
// in FPGA's GlobalABBXmuxReg!!
// This means a new MUX position must be switched with old one.
THORDAQ_STATUS CThordaq::SwapABBxMUX(
	CThordaq& TdBrd,                // Board to target
	int BNClabel,                   // BNC connector lable on BOB
	int newBNClabel)                // i.e., FPGA's Wavetable DAC channel (4bit Nibble Index 0-11)
{
	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;
	UINT64 currentDMAch, newDMAch;

	if (BNClabel < 0 || BNClabel > 11)
		status = THORDAQ_STATUS::STATUS_I2C_INVALIDMUX;
	if (newBNClabel < 0 || newBNClabel > 11)
		status = THORDAQ_STATUS::STATUS_I2C_INVALIDMUX;
	if (status != THORDAQ_STATUS::STATUS_SUCCESSFUL)
	{
		wchar_t logMsg[MSG_SIZE]; // log BOB config errors
		StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::ThorDAQAPISetAIOConfig: SwapABBxMUX() %d to %d failed, invalid BNC", BNClabel, newBNClabel);
		CThordaq::LogMessage(logMsg, ERROR_EVENT);
	}

	// FPGA SWUG Emphasizes that MUX assignments are MUTUALLY EXCLUSIVE!  
	// consequently we must identify current MUX assignment for what is being set,
	// and swap channels with new assignment
		// e.g., MUXindexToSwap (BNClablel) is 0xA -- find it's current position.
		// If different from current config, swap it.
	if (BNClabel == newBNClabel)
		return THORDAQ_STATUS::STATUS_SUCCESSFUL; // already done

	// you can ONLY reset the MUX when GlobalScan is '0'
	UINT64 globalScan;
	status = FPGAregisterREAD("GIGCR0_STOP_RUN", &globalScan); // (reads shadow bit or WRITE-only REG)
	if (globalScan == 1) // scanning is enabled
	{
		return THORDAQ_STATUS::STATUS_INVALID_MODE; // illegal to change when SCAN is active
	}

	// because settings are EXCLUSIVE (duplicate GlobalABBXmuxReg nibbles = chaos!!)
	// we MUST SWAP whatever is currently in BNClabel with new config
	status = FPGAregisterRead(GlobalABBxMUXreg[BNClabel], (int)strlen("DAC_DMAnn"), &currentDMAch);
	status = FPGAregisterRead(GlobalABBxMUXreg[newBNClabel], (int)strlen("DAC_DMAnn"), &newDMAch);
	status = FPGAregisterWrite(GlobalABBxMUXreg[BNClabel], (int)strlen("DAC_DMAnn"), newDMAch);
	status = FPGAregisterWrite(GlobalABBxMUXreg[newBNClabel], (int)strlen("DAC_DMAnn"), currentDMAch);

	return status;
}


// FIRST call to this function sets GlobalABBXmuxReg DEFAULTS based on BOB type
// e.g. "AnnXDVmmMcc", where	"n" is 0-31 (BNC label "nn", e.g. "A12" or "A03"),
//							"X" is Input/Output direction "I" or "O", 
//							"D" is "U"nipolar or "B"ipolar, 
//							"V" is Volts "mm" range e.g. "10"
//							"M" is (Output only) MUX'd DAC Waveform DMAcc [channel (00-11)] location for BNC label in FPGA's GlobalABBXmuxReg (all BOBs)
THORDAQ_STATUS CThordaq::APISetAIOConfig(
	CThordaq& TdBrd,                // Board to target
	CHAR* config,
	UINT32 configSize  // byte size of caller's config buffer
)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	THORDAQ_STATUS I2Cstatus;
	wchar_t logMsg[MSG_SIZE]; // log BOB config errors
	CHAR cNum[3];
	CHAR cCfgArrayBuff[TD_BOBAIDef::CharPerBOB_AI];
	bool bFirstConfig = (CThordaq::BOB_HardwareType == 'X') ? true : false;

	I2Cstatus = DiscoverBOBtype(TdBrd);
	if (I2Cstatus != STATUS_SUCCESSFUL)
	{
		// FATAL error - no BOB detected
		StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::ThorDAQAPISetAIOConfig: Cannot locate BreakOut Box hardware via I2C, HDMI cable problem?");
		CThordaq::LogMessage(logMsg, ERROR_EVENT);

		// fatal error!  no connection to BOB
		return STATUS_DEVICE_NOT_EXISTS;
	}

	if (configSize < TD_BOBAIDef::CharPerBOB_AI) // caller's buff can be bigger, but not smaller
		return STATUS_PARAMETER_SETTINGS_ERROR;

	// get a buffer copy from caller (typ. C#)
	memcpy_s(cCfgArrayBuff, configSize, config, configSize);
	//"AnnXDVmm"
	cNum[0] = cCfgArrayBuff[1];
	cNum[1] = cCfgArrayBuff[2];
	cNum[2] = 0;
	int BNClabel = atoi(cNum);
	// limit check arg
	if (BNClabel < 0 || BNClabel >= TD_BOBAIDef::NumBOB_AIs)
	{
		return STATUS_PARAMETER_SETTINGS_ERROR;
	}
	// now decode OUTPUT command...
	//       01234567890
	// e.g. "AnnXDVmmMcc", where	"n" is 0-31 (BNC label "nn", e.g. "A12" or "A03"),
	//							"X" is Input/Output direction "I" or "O", 
	//							"D" is "U"nipolar or "B"ipolar, 
	//							"V" is Volts "mm" range e.g. "10"
	//							"M" is (Output only) MUX'd DAC Waveform DMAcc [channel (00-11)] location for BNC label in FPGA's GlobalABBXmuxReg (all BOBs)
	cNum[0] = cCfgArrayBuff[9];
	cNum[1] = cCfgArrayBuff[10];
	int DMCch = atoi(cNum);

	if (CThordaq::BOB_HardwareType == 'P') // 3U Panel BOB?
	{
		// if this is the first call, set 3U Panel FPGA MUX for high-speed DAC waveform BNC locations (AO0-AO11)
		if (bFirstConfig)  // if we got this far, we know the BOB hardware type
		{
			status = FPGAregisterWRITE("GlobalABBXmuxReg", CThordaq::Panel3UDACdefaultChannelMap);
		}
		if ((cCfgArrayBuff[3] == 'I' || cCfgArrayBuff[3] == 'i') && BNClabel >= 6) // configurable AI?
		{  // we are configuring INPUT
			BOB_AISettings[BNClabel].BNClabel = BNClabel; // (debug sanity)
			// 01234567
			//"AnnXDVmm", "D"irection is Unipolar or Bipolar
			if (cCfgArrayBuff[4] == 'u' || cCfgArrayBuff[4] == 'U')
			{ // unipolar
				BOB_AISettings[BNClabel].Polarity = 'U';
			}
			else
			{
				BOB_AISettings[BNClabel].Polarity = 'B';
			}
			// now decode volts range
			double VoltageRange = 10.0; // default
			try
			{
				cNum[0] = cCfgArrayBuff[6];
				cNum[1] = cCfgArrayBuff[7];
				VoltageRange = atof(cNum);
				BOB_AISettings[BNClabel].VoltageRange = VoltageRange;
			}
			catch (int)
			{
				// forced to use default...
				StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::ThorDAQAPISetAIOConfig: Failed to configure BNC %d", BNClabel);
				CThordaq::LogMessage(logMsg, ERROR_EVENT);


			}
		}
		else // 3U panel BOB OUTPUT
		{
			// now decode OUTPUT command...
			status = SwapABBxMUX(TdBrd, BNClabel, DMCch);
		}
	}
	else  // LEGACY ABBx BOB hardware (set whatever we can)
	{
		if (bFirstConfig)  // if we got this far, we know the BOB hardware type, and it needs a default setup
		{
			status = FPGAregisterWRITE("GlobalABBXmuxReg", CThordaq::LegacyDACdefaultChannelMap); // see default "dacChannelMap" var.
		}
		// now try to set what caller requires (more limited config on Legacy BOB)
		if ((cCfgArrayBuff[3] == 'O' || cCfgArrayBuff[3] == 'o') && (BNClabel >= 0 && BNClabel < 11)) // configurable AO?
		{
			status = SwapABBxMUX(TdBrd, BNClabel, DMCch);
		}
		// currently nothing to set for Legacy AI (ABBx AI1 - AI6 have never worked)
	}
	return status;
}

THORDAQ_STATUS  CThordaq::GetBOB_DIOConfig(CThordaq& TdBrd, CHAR* ConfigArray, int ConfigArraySize)
{
	// What BOB (BreakOut Box) are we connected to ?
	// Query the ADC (DDB1 cable) to discover and set BOBtype
	THORDAQ_STATUS status = DiscoverBOBtype(TdBrd);
	if (status != THORDAQ_STATUS::STATUS_SUCCESSFUL)
		return status;

	if (ConfigArraySize < sizeof(&TdBrd.BOB_DIOconfiguration)) // caller's buff can be bigger, but not smaller
		return THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;
	// copy the current config to caller

	memcpy_s(&ConfigArray[0], ConfigArraySize, &TdBrd.BOB_DIOconfiguration[0], ConfigArraySize);
	return THORDAQ_STATUS::STATUS_SUCCESSFUL;
}

THORDAQ_STATUS  CThordaq::SetBOB_DIOConfig(CThordaq& TdBrd, CHAR* ConfigArray, int ConfigArraySize)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	THORDAQ_STATUS I2Cstatus;
	wchar_t logMsg[MSG_SIZE]; // log BOB config errors
	CHAR cNum[3];
	UINT64 FPGA_AUX_DIO_Dir;
	CHAR cCfgArrayBuff[CharPerBOB_DIO];
	BOOL bHighSpeedChannel = false;
	I2Cstatus = DiscoverBOBtype(TdBrd);
	if (I2Cstatus != STATUS_SUCCESSFUL)
	{
		// FATAL error - no BOB detected
		StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::ThorDAQAPISetDIOconfig: Cannot locate DIO BreakOut Box hardware via I2C, HDMI cable problem?");
		CThordaq::LogMessage(logMsg, VERBOSE_EVENT);

		// fatal error!  no connection to BOB
		return STATUS_DEVICE_NOT_EXISTS;
	}

	if (ConfigArraySize < CharPerBOB_DIO) // caller's buff can be bigger, but not smaller
		return STATUS_PARAMETER_SETTINGS_ERROR;
	// get a buffer copy from caller (typ. C#)
	memcpy_s(cCfgArrayBuff, ConfigArraySize, &ConfigArray[0], ConfigArraySize);

	try
	{
		// interpret the char string sent to us for config task
		// Ex: ConfigArray = "D01O01M13A-1" BNC port 1 (BNC "D1" or "DIO2"), Output, source is FPGAindex 1, MUX control FPGA "13" (oDigital_Waveform_3)
		// Ex: ConfigArray = "D12O02M29A02" BNC port 12, Output, FPGAindex 2, MUX control FPGA 29 (Aux_GPIO_2), AUX index 02
		// Ex: ConfigArray = "D15O01M48A-1" BNC port 15, Output, CPLD-MUX Value from FPGA index1
		// Ex: ConfigArray = "D09I04M00A-1" BNC port 9, Input, FPGA (destination) index 4, MUX control FPGA "00" (iResonantScannerFeedback)
		int BNClabel;  // i.e., value 0 means BNC port connector at "D0" on 3U BOB or "DIO1" on DBB1 BOB
		int iFPGAioIndex = -1, iMUX;
		int iAuxSource = -1;
		BOOL bDIRisOutput = TRUE;
		// order MSB, LSB ASCII endianess correctly
		// 012345678901
		// D04I04M00A-1
		cNum[0] = cCfgArrayBuff[1];
		cNum[1] = cCfgArrayBuff[2];
		cNum[2] = 0;
		BNClabel = atoi(cNum);  // CPLD side

		if (cCfgArrayBuff[3] == 'I' || cCfgArrayBuff[3] == 'i')
		{
			FPGA_AUX_DIO_Dir = 1;  // FPGA SWUG defines AUX GPIO DIR as "1" for Input - in CPLD, "1" is Output
			bDIRisOutput = FALSE;
		}
		else
		{
			FPGA_AUX_DIO_Dir = 0;
		}
		cNum[0] = cCfgArrayBuff[4];
		cNum[1] = cCfgArrayBuff[5];
		iFPGAioIndex = atoi(cNum);		// FPGA side: 0-7 are the high speed signals 
		// now get the MUX
		cNum[0] = cCfgArrayBuff[7];
		cNum[1] = cCfgArrayBuff[8];
		iMUX = atoi(cNum);
		// analyze the AUX (only for FPGA's AUX_GPIOx)
		cNum[0] = cCfgArrayBuff[10];
		cNum[1] = cCfgArrayBuff[11];
		iAuxSource = atoi(cNum);

		// first test validity of INDEXes - only DIOs 0-15 can be MUXed:  Any DIO 16 or higher, BNC must equal FPGAindex (1:1 mapping)
		if (BOB_HardwareType == 'P')
		{ // 2U BBox?
			if ((BNClabel > 15 && (BNClabel != iFPGAioIndex)) || BNClabel > 31 || iFPGAioIndex > 31)
			{
				StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::ThorDAQAPISetDIOconfig: Attempt to configure DIO BNC D%d / FPGA index %d invalid", BNClabel, iFPGAioIndex);
				return STATUS_PARAMETER_SETTINGS_ERROR;
			}
		}
		// In Legacy DIO we have only 8 DIOs and the BNCindex must always equal FPGAindex
		if (BOB_HardwareType == 'L') // Legacy BBox?
		{
			if (BNClabel > 7 || iFPGAioIndex > 7 || (BNClabel != iFPGAioIndex))
			{
				StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::ThorDAQAPISetDIOconfig: Attempt to configure DIO BNC D%d / FPGA index %d invalid", BNClabel, iFPGAioIndex);
				return STATUS_PARAMETER_SETTINGS_ERROR;
			}
		}

		// verify that FPGA Digital MUX direction is consistent - flag errors.
		// PERMITTING ELECTRICAL I/O DIRECTION CONFLICTS can damage hardware
		bool bDIRmismatch = false;
		if (bDIRisOutput) // OUTPUT
		{
			if (iFPGAioIndex <= 7)  // FPGA index MUX dir MUST match BNC DIR 
			{
				// catch error instances of using "input" only FPGA MUX for output config
				switch (iMUX)
				{
				case TD_DIO_MUXedSLAVE_PORTS::DI_ResonantScanner_Line_Feedback: // 0x00
				case TD_DIO_MUXedSLAVE_PORTS::DI_External_Line_Trigger: // 0x01
				case TD_DIO_MUXedSLAVE_PORTS::DI_External_Pixel_Clock:  // 0x02
				case TD_DIO_MUXedSLAVE_PORTS::DI_External_Frame_Retrigger: // 0x07
				case TD_DIO_MUXedSLAVE_PORTS::DI_External_SOF_Trigger:  // 0x08
					bDIRmismatch = true;
				default:
					bDIRmismatch = false;
				}

				if (bDIRmismatch)
				{
					StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::ThorDAQAPISetDIOconfig: Attempt to configure DIO BNC D%d / FPGA index %d (MUX 0x%x) I/O Direction mismatch", BNClabel, iFPGAioIndex, iMUX);
					return STATUS_PARAMETER_SETTINGS_ERROR;
				}

				// Is the FPGA MUX an AUX GPIO requiring additional config? (AUX GPIO DIR must be separately set)
				if (iMUX >= 27 && iMUX <= 31)
				{
					// additional config...
					if (iAuxSource < 0 || iAuxSource > 4) // sanity check result
					{
						StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::ThorDAQAPISetDIOconfig: Attempt to configure DIO BNC D%d, FPGA MUX %d, AUX GPIO index %d invalid", BNClabel, iMUX, iAuxSource);
						return STATUS_PARAMETER_SETTINGS_ERROR; // invalid Aux_GPIO_ index
					}
					// Set In/Out Direction
					status = FPGAregisterWrite(AuxGPIODir[iAuxSource], (int)strlen("Aux_GPIO_DIR_x"), FPGA_AUX_DIO_Dir); // DIR '1' for Input, '0' for Output
				}
				// Set the Digital MUX in the FPGA's config register...
				status = FPGAregisterWrite(GlobalDBB1MUXreg[iFPGAioIndex], (int)strlen("DBB1_IOCIRCUIT_n"), iMUX); // sets FPGA MUX connection to BNC
			}
			// Finally, for 3U BOB, ALWAYS send I2C config command; for Legacy, never send			
			if (BOB_HardwareType == 'P')
			{
				// we need to configure the final CPLD-controlled BNC port via I2C command, both DIRection and MUXing signal from FPGA side to CPLD side
				// if it's HSC (High Speed Channel) the iCopiedSource needs to be 0-7, corresponding
				// to the signal on DBB1 cable
				status = APISetCPLD_DIOConfig(TdBrd, BNClabel, iFPGAioIndex, bHighSpeedChannel, (int)FPGA_AUX_DIO_Dir);  // '1' is Input, '0' Output (FPGA AUX define)
				if (status != THORDAQ_STATUS::STATUS_SUCCESSFUL)
				{	// typically a comm error
					StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::ThorDAQAPISetDIOconfig: Attempt to configure DIO BNC D%d, got CPLD I2C comm failure", BNClabel);
					CThordaq::LogMessage(logMsg, VERBOSE_EVENT);
					return status;
				}
			}
		}
		else // INPUT
		{
			if (iFPGAioIndex <= 7)  // FPGA index MUST match DIR 
			{
				// catch error instances of using "output" only FPGA MUX for input config
				switch (iMUX)
				{
				case DO_0_WaveMUX: // 0x0A
				case DO_1_WaveMUX:
				case DO_2_WaveMUX:
				case DO_3_WaveMUX:
				case DO_4_WaveMUX:
				case DO_5_WaveMUX:
				case DO_6_WaveMUX:
				case DO_7_WaveMUX:
				case DO_8_WaveMUX:
				case DO_9_WaveMUX:
				case DO_10_WaveMUX:
				case DO_11_WaveMUX:
				case DO_12_WaveMUX:
				case DO_13_WaveMUX:
				case DO_14_WaveMUX:
				case DO_15_WaveMUX:
				case DO_Capture_Active:

				case DO_ScanLine_Direction: // x0x3
				case DO_Horizontal_Line_Trigger_Pulse:
				case DO_Pixel_Integration_Interval:
				case DO_Internal_SOF_Trigger: // 0x06
				case DO_Pixel_Clock:  // 0x09
					bDIRmismatch = true;
				default:
					bDIRmismatch = false;
				}
				// catch error instances of using "input" only FPGA MUX for output config
				if (bDIRmismatch)
				{
					StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::ThorDAQAPISetDIOconfig: Attempt to configure DIO BNC D%d / FPGA index %d (MUX 0x%x) I/O Direction mismatch", BNClabel, iFPGAioIndex, iMUX);
					return STATUS_PARAMETER_SETTINGS_ERROR;
				}
				// Is the FPGA MUX an AUX GPIO requiring additional config? (AUX GPIO DIR must be separately set)
				if (iMUX >= 27 && iMUX <= 31)
				{
					// additional config...
					if (iAuxSource < 0 || iAuxSource > 4) // sanity check result
					{
						StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::ThorDAQAPISetDIOconfig: Attempt to configure DIO BNC D%d, FPGA MUX %d, AUX GPIO index %d invalid", BNClabel, iMUX, iAuxSource);
						return STATUS_PARAMETER_SETTINGS_ERROR; // invalid Aux_GPIO_ index
					}
					// Set In/Out Direction
					status = FPGAregisterWrite(AuxGPIODir[iAuxSource], (int)strlen("Aux_GPIO_DIR_x"), FPGA_AUX_DIO_Dir); // DIR '1' for Input, '0' for Output
				}
				// Set the Digital MUX in the FPGA's config register...
				status = FPGAregisterWrite(GlobalDBB1MUXreg[iFPGAioIndex], (int)strlen("DBB1_IOCIRCUIT_n"), iMUX); // sets FPGA MUX connection to BNC

			} // END case FPGA index
				// Finally, for 3U BOB, ALWAYS send I2C config command; for Legacy, never send			
			if (BOB_HardwareType == 'P')
			{
				// we need to configure the final CPLD-controlled BNC port via I2C command, both DIRection and MUXing signal from FPGA side to CPLD side
				// if it's HSC (High Speed Channel) the iCopiedSource needs to be 0-7, corresponding
				// to the signal on DBB1 cable
				status = APISetCPLD_DIOConfig(TdBrd, BNClabel, iFPGAioIndex, bHighSpeedChannel, (int)FPGA_AUX_DIO_Dir);  // '1' is Input, '0' Output (FPGA AUX define)
				if (status != THORDAQ_STATUS::STATUS_SUCCESSFUL)
				{	// typically a comm error
					StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::ThorDAQAPISetDIOconfig: Attempt to configure DIO BNC D%d, got CPLD I2C comm failure", BNClabel);
					CThordaq::LogMessage(logMsg, VERBOSE_EVENT);
					return status;
				}
			}

		}

		// IF SUCCESSFUL, update current config record
		memcpy_s(&TdBrd.BOB_DIOconfiguration[BNClabel * CharPerBOB_DIO], CharPerBOB_DIO - 1, ConfigArray, (rsize_t)ConfigArraySize - 1);
		// AND, update current Config/Value struct...
		TdBrd.BOB_DIOSettings[BNClabel].BNClabel = BNClabel;
		TdBrd.BOB_DIOSettings[BNClabel].FPGAioIndex = iFPGAioIndex;
		TdBrd.BOB_DIOSettings[BNClabel].MUX = iMUX;
		TdBrd.BOB_DIOSettings[BNClabel].AuxMUX = iAuxSource; // for AUX GPIOs
		TdBrd.BOB_DIOSettings[BNClabel].bOutputDir = bDIRisOutput;
		// return status to caller
		return status;
	}
	catch (int e) // what error?
	{
		switch (e)
		{
		case 10:
			status = STATUS_INCOMPLETE;
			break;
		default:
			status = STATUS_PARAMETER_SETTINGS_ERROR;
			break;
		}
	}
	return status;
}



THORDAQ_STATUS CThordaq::APISetDO( // SET the value of one or more DOs
	CThordaq& TdBrd,                // Board to target
	CHAR* config,            // Collection of fields per Config functions - AUX field holds value
	UINT32 RecordSize,       // Record size (of each record comprising Config-field definition)
	UINT32 NumDOs            // 1 or more DO records in "config"
)
{
	THORDAQ_STATUS status = (THORDAQ_STATUS)STATUS_INVALID_PARAMETER;

	status = (THORDAQ_STATUS)CThordaq::SetBOB_DO(TdBrd, config, RecordSize, NumDOs);

	return status;
}
THORDAQ_STATUS CThordaq::APIGetDIO( // SET the value of one or more DOs
	CThordaq& TdBrd,                // Board to target
	CHAR* config,            // Collection of fields per Config functions - AUX field holds value
	UINT32 RecordSize,       // field size 
	UINT32 NumDOs            // 1 or more DO records in "config"
)
{
	THORDAQ_STATUS status = (THORDAQ_STATUS)STATUS_INVALID_PARAMETER;

	status = (THORDAQ_STATUS)CThordaq::GetBOB_DIO(TdBrd, config, RecordSize, NumDOs);

	return status;
}

// private function to control ALL the LEDs on the 3U BOB
THORDAQ_STATUS CThordaq::LEDControlAll_IS31FL_LEDs_on_BOB(CThordaq& TdBrd, UINT8 ControlByte)
{
	// first set the PCI board backplane LEDs

	FPGAregisterWRITE("GIGCR0_LED1", (ControlByte & 0x01));
	FPGAregisterWRITE("GIGCR0_LED2", (ControlByte & 0x01));

	// Send I2C command sequence to IS31FL controller - there are two controllers,
	// one on DBB1 HDMI cable (ADC mezz. card, no slave MUX), the other on ABB1 HDMI cable (slave MUX 0x1) 
	UINT8 I2CbyteBuffer[64];
	UINT8 I2CopcodeBuffer[8];
	UINT32 I2CbufferLen = 0;
	UINT32 I2CopcodeLen = 0;
	UINT32 LEDi2cAddress = 0x3C; // 3U Panel IS31FL LED Controller I2C slave addr
	UINT32 I2CmasterMUX = 2;
	UINT32 I2CslaveMUX = 0xFF;

	THORDAQ_STATUS retStatus = THORDAQ_STATUS::STATUS_SUCCESSFUL;
	THORDAQ_STATUS I2Cstatus = THORDAQ_STATUS::STATUS_SUCCESSFUL;
	int iChips = 2; // i.e. the two IS31FL controllers
	while (iChips-- > 0)
	{
		// insure IS31FL chip in "normal" operation
		I2CbufferLen = 2;
		I2CbyteBuffer[0] = 0; 	I2CbyteBuffer[1] = 1; // Shutdown Reg 0x0, set to "Normal" operation
		I2Cstatus = APIXI2CReadWrite(TdBrd, FALSE, 0x71, I2CmasterMUX, 0x70, I2CslaveMUX, LEDi2cAddress, 400, 36, I2CopcodeBuffer, &I2CopcodeLen, I2CbyteBuffer, &I2CbufferLen);
		if (I2Cstatus != STATUS_SUCCESSFUL)
		{
			retStatus = I2Cstatus;
			// first controller failed, try second controller
			// (next chip)
			I2CmasterMUX = 0x8;
			I2CslaveMUX = 0x1;

			continue;
		}
		// chip ready for command... (we should expect additional I2C writes to succeed)
		// we must prevent overcurrent in software!
		// set PWM (voltage level), e.g. 0x3F
		UINT8 RegisterAddressByte, i;
		RegisterAddressByte = 1; // the "PWM" Register
		// first BYTE after SlaveAddr is 'command' RegisterAddress
		// the chip will "auto-increment" this value, allowing us to start at beginning register
		// and write values for all 36 channels in one I2C WRITE stream
		I2CbyteBuffer[0] = RegisterAddressByte;
		for (i = 1; i <= 36; i++)
		{
			I2CbyteBuffer[i] = 0x3F; // Set PWM 
		}
		I2CbufferLen = 37;  // the command Register, plus 36 PWM values
		I2Cstatus = APIXI2CReadWrite(TdBrd, FALSE, 0x71, I2CmasterMUX, 0x70, I2CslaveMUX, LEDi2cAddress, 400, 37, I2CopcodeBuffer, &I2CopcodeLen, I2CbyteBuffer, &I2CbufferLen);
		if (I2Cstatus != STATUS_SUCCESSFUL)
		{
			retStatus = I2Cstatus;
		}

		I2CbufferLen = 2;

		// Set Current (Intensity) and ON/OFF status 
		// "Channel" is 1-based in IS31FL
		RegisterAddressByte = 0x26; // the "LED control" Register
		I2CbyteBuffer[0] = RegisterAddressByte;
		for (i = 1; i <= 36; i++)
		{
			I2CbyteBuffer[i] = ControlByte; // Set Control (current bits2:1 + on/off bit0)
		}
		I2CbufferLen = 37;  // the command Register, plus 36 PWM values
		I2Cstatus = APIXI2CReadWrite(TdBrd, FALSE, 0x71, I2CmasterMUX, 0x70, I2CslaveMUX, LEDi2cAddress, 400, 37, I2CopcodeBuffer, &I2CopcodeLen, I2CbyteBuffer, &I2CbufferLen);

		// (next chip)
		I2CmasterMUX = 0x8;
		I2CslaveMUX = 0x1;
	};


	// finally, latch in everything just configured...
	I2CbyteBuffer[0] = 0x25; 	I2CbyteBuffer[1] = 0; // 
	I2CbufferLen = 2;
	I2Cstatus = APIXI2CReadWrite(TdBrd, FALSE, 0x71, I2CmasterMUX, 0x70, I2CslaveMUX, LEDi2cAddress, 400, 37, I2CopcodeBuffer, &I2CopcodeLen, I2CbyteBuffer, &I2CbufferLen);
	if (I2Cstatus != STATUS_SUCCESSFUL)
	{
		retStatus = I2Cstatus;
	}
	// (the other chip)
	I2CmasterMUX = 0x2;
	I2CslaveMUX = 0xFF;
	I2CbufferLen = 2;
	I2Cstatus = APIXI2CReadWrite(TdBrd, FALSE, 0x71, I2CmasterMUX, 0x70, I2CslaveMUX, LEDi2cAddress, 400, 37, I2CopcodeBuffer, &I2CopcodeLen, I2CbyteBuffer, &I2CbufferLen);
	if (I2Cstatus != STATUS_SUCCESSFUL)
	{
		retStatus = I2Cstatus;
	}

	return retStatus;
}

// runs twice per second to check LED "state" array
// 

void CThordaq::BlinkLEDTask()
{
	bool bBlinkThisCycle = true;
	int i;
	// traverse array to read values set by API
	BOBLEDstateArray[BBoxLEDenum::DC4] = 2; // TEST

	BOBLEDstateArray[BBoxLEDenum::DC6] = 1; // TEST

	while (!bExitBlinkLEDTask) {
		for (i = 0; i <= BBoxLEDenum::AI13; i++)
		{
			// for each LED
			if (BOBLEDstateArray[i] == 2) // blinking?
			{

			}
			else // static ON or OFF
			{

			}

			//		LEDControlIS31FL(DriverList[gPCIeBoardIndex], &BOBLEDstateArray[i], BOBLEDstateArray[i]);
		}
		Sleep(500);
		bBlinkThisCycle = (bBlinkThisCycle == true) ? 0 : 1; // toggle
	}
}

// see www.issi.com, "IS31FL3236 36 CHANNELS LED DRIVER" Data Sheet, Fev. F, 09/29/2016, pgs. 8-9 for I2C command set
// State is 0 for off, 1 for on, 2 for blink
THORDAQ_STATUS CThordaq::LEDControlIS31FL(CThordaq& TdBrd, int BBoxLEDenums, UCHAR State)// int Control[], int ChannelCount)
{
	int Control[2] = { 0,0 };
	Control[0] = (State == 1) ? 1 : 0; // ON/off
	Control[0] += 2; // IMAX/2 amps, ON/Off
	// Send I2C command sequence to IS31FL controller - there are two controllers,
	// one on DBB1 HDMI cable (ADC mezz. card, no slave MUX), the other on ABB1 HDMI cable (slave MUX 0x1) 
	UINT8 I2CbyteBuffer[64];
	UINT8 I2CopcodeBuffer[8];
	UINT32 I2CbufferLen = 0;
	UINT32 I2CopcodeLen = 0;
	UINT32 LEDi2cAddress = 0x3C; // def. to 3U Panel IS31FL LED Controller
	UINT32 I2CmasterMUX;
	UINT32 I2CslaveMUX;
	THORDAQ_STATUS I2Cstatus;
	int IS31FLchannel = 0xFF, UChip = 1; // we must translate the "enum" to a channel to access registers on correct chip
	// (for now, ONE arg per call)

	// the Channel[] is array of enums which must translate to chip "Channel" controlling LED on BOB
	// e.g. "CONN" for Connected
	switch (BBoxLEDenums)
	{
		// IS31FL chip U2 (on DBB1 HDMI cable)
	case BBoxLEDenum::D0:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD0;
		UChip = 2;
		break;
	case BBoxLEDenum::D1:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD1;
		UChip = 2;
		break;
	case BBoxLEDenum::D2:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD2;
		UChip = 2;
		break;
	case BBoxLEDenum::D3:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD3;
		UChip = 2;
		break;
	case BBoxLEDenum::D4:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD4;
		UChip = 2;
		break;
	case BBoxLEDenum::D5:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD5;
		UChip = 2;
		break;
	case BBoxLEDenum::D6:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD6;
		UChip = 2;
		break;
	case BBoxLEDenum::D7:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD7;
		UChip = 2;
		break;
	case BBoxLEDenum::D8:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD8;
		UChip = 2;
		break;
	case BBoxLEDenum::D9:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD9;
		UChip = 2;
		break;
	case BBoxLEDenum::D10:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD10;
		UChip = 2;
		break;
	case BBoxLEDenum::D11:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD11;
		UChip = 2;
		break;
	case BBoxLEDenum::D12:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD12;
		UChip = 2;
		break;
	case BBoxLEDenum::D13:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD13;
		UChip = 2;
		break;
	case BBoxLEDenum::D14:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD14;
		UChip = 2;
		break;
	case BBoxLEDenum::D15:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD15;
		UChip = 2;
		break;
	case BBoxLEDenum::D16:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD16;
		UChip = 2;
		break;
	case BBoxLEDenum::D17:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD17;
		UChip = 2;
		break;
	case BBoxLEDenum::D18:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD18;
		UChip = 2;
		break;
	case BBoxLEDenum::D19:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD19;
		UChip = 2;
		break;
	case BBoxLEDenum::D20:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD20;
		UChip = 2;
		break;
	case BBoxLEDenum::D21:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD21;
		UChip = 2;
		break;
	case BBoxLEDenum::D22:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD22;
		UChip = 2;
		break;
	case BBoxLEDenum::D23:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD23;
		UChip = 2;
		break;
	case BBoxLEDenum::D24:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD24;
		UChip = 2;
		break;
	case BBoxLEDenum::D25:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD25;
		UChip = 2;
		break;
	case BBoxLEDenum::D26:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD26;
		UChip = 2;
		break;
	case BBoxLEDenum::D27:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD27;
		UChip = 2;
		break;
	case BBoxLEDenum::D28:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD28;
		UChip = 2;
		break;
	case BBoxLEDenum::D29:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD29;
		UChip = 2;
		break;
	case BBoxLEDenum::D30:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD30;
		UChip = 2;
		break;
	case BBoxLEDenum::D31:
		IS31FLchannel = TD_3UchipU2BOB_LEDs::cD31;
		UChip = 2;
		break;

		// CASES for default IS31FL chip U1 (on ABB1 HDMI cable)
	case BBoxLEDenum::DC1:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cDC1;
		break;
	case BBoxLEDenum::DC2:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cDC2;
		break;
	case BBoxLEDenum::DC3:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cDC3;
		break;
	case BBoxLEDenum::DC4:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cDC4;
		break;
	case BBoxLEDenum::DC5:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cDC5;
		break;
	case BBoxLEDenum::DC6:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cDC6;
		break;
	case BBoxLEDenum::AI0:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAI0;
		break;
	case BBoxLEDenum::AI1:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAI1;
		break;
	case BBoxLEDenum::AI2:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAI2;
		break;
	case BBoxLEDenum::AI3:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAI3;
		break;
	case BBoxLEDenum::AI4:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAI4;
		break;
	case BBoxLEDenum::AI5:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAI5;
		break;
	case BBoxLEDenum::AI6:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAI6;
		break;
	case BBoxLEDenum::AI7:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAI7;
		break;
	case BBoxLEDenum::AI8:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAI8;
		break;
	case BBoxLEDenum::AI9:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAI9;
		break;
	case BBoxLEDenum::AI10:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAI10;
		break;
	case BBoxLEDenum::AI11:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAI11;
		break;
	case BBoxLEDenum::AI12:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAI12;
		break;
	case BBoxLEDenum::AI13:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAI13;
		break;
	case BBoxLEDenum::AO0:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAO0;
		break;
	case BBoxLEDenum::AO1:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAO1;
		break;
	case BBoxLEDenum::AO2:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAO2;
		break;
	case BBoxLEDenum::AO3:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAO3;
		break;
	case BBoxLEDenum::AO4:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAO4;
		break;
	case BBoxLEDenum::AO5:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAO5;
		break;
	case BBoxLEDenum::AO6:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAO6;
		break;
	case BBoxLEDenum::AO7:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAO7;
		break;
	case BBoxLEDenum::AO8:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAO8;
		break;
	case BBoxLEDenum::AO9:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAO9;
		break;
	case BBoxLEDenum::AO10:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAO10;
		break;
	case BBoxLEDenum::AO11:
		IS31FLchannel = TD_3UchipU1BOB_LEDs::cAO11;
		break;
	case 0xFF: // ALL option
		return LEDControlAll_IS31FL_LEDs_on_BOB(TdBrd, (BYTE)Control[0]);

		break;

	default:
		return STATUS_DEVICE_NOT_EXISTS; // LED code not found
	}
	if (UChip == 2) // i.e. U2 on PCB
	{
		I2CmasterMUX = 0x2;
		I2CslaveMUX = 0xFF;
	}
	else  // U1 on PCB
	{
		I2CmasterMUX = 0x8;
		I2CslaveMUX = 0x1;
	}
	// insure IS31FL chip in "normal" operation
	I2CbufferLen = 2;
	I2CbyteBuffer[0] = 0; 	I2CbyteBuffer[1] = 1; // Shutdown Reg 0x0, set to "Normal" operation
	I2Cstatus = APIXI2CReadWrite(TdBrd, FALSE, 0x71, I2CmasterMUX, 0x70, I2CslaveMUX, LEDi2cAddress, 400, 36, I2CopcodeBuffer, &I2CopcodeLen, I2CbyteBuffer, &I2CbufferLen);
	if (I2Cstatus != STATUS_SUCCESSFUL)
		return STATUS_DEVICE_NOT_EXISTS;
	// chip ready for command... (we should expect additional I2C writes to succeed)
	// we must prevent overcurrent in software!
	// set PWM (voltage level), e.g. 0x3F
	I2CbufferLen = 2;
	I2CbyteBuffer[0] = IS31FLchannel; 	I2CbyteBuffer[1] = 0x3F; // Set PWM 
	I2Cstatus = APIXI2CReadWrite(TdBrd, FALSE, 0x71, I2CmasterMUX, 0x70, I2CslaveMUX, LEDi2cAddress, 400, 36, I2CopcodeBuffer, &I2CopcodeLen, I2CbyteBuffer, &I2CbufferLen);
	I2CbufferLen = 2;
	// Set Current (Intensity) and ON/OFF status 
	// "Channel" is 1-based in IS31FL
	I2CbyteBuffer[0] = (BYTE)(IS31FLchannel + 0x25); // offset to LED Control Register 0x26 (first chan is '1')
	I2CbyteBuffer[1] = (BYTE)Control[0];
	I2Cstatus = APIXI2CReadWrite(TdBrd, FALSE, 0x71, I2CmasterMUX, 0x70, I2CslaveMUX, LEDi2cAddress, 400, 36, I2CopcodeBuffer, &I2CopcodeLen, I2CbyteBuffer, &I2CbufferLen);

	// finally, latch in everything just configured...
	I2CbufferLen = 2;
	I2CbyteBuffer[0] = 0x25; 	I2CbyteBuffer[1] = 0; // 
	I2Cstatus = APIXI2CReadWrite(TdBrd, FALSE, 0x71, I2CmasterMUX, 0x70, I2CslaveMUX, LEDi2cAddress, 400, 36, I2CopcodeBuffer, &I2CopcodeLen, I2CbyteBuffer, &I2CbufferLen);

	return I2Cstatus;
}

// Change on/off to LED "State" so we can blink
// 0 = always off, 1 = on, 2 = blink


THORDAQ_STATUS CThordaq::APIBreakOutBoxLED(
	CThordaq& TdBrd, // This CThordaq object by reference
	INT32 LEDenum,   // -1 to control ALL the LEDs
	UCHAR  State                    // 0 for off, 1 for on, 2 for blink
)
{
	BreakoutBoxLEDs* BBox = nullptr;
	THORDAQ_STATUS status = (THORDAQ_STATUS)STATUS_INVALID_PARAMETER;
	int LEDmask = 0;

	// are we connected to BreakOut Box?
	if (BOB_HardwareType == 'X')  // not yet discovered?
	{
		THORDAQ_STATUS I2Cstatus = DiscoverBOBtype(TdBrd);
		if (I2Cstatus != STATUS_SUCCESSFUL)
		{
			// fatal error!  no connection to BOB
			// do not log error
			return STATUS_DEVICE_NOT_EXISTS;
		}
	}
	if (BOB_HardwareType == 'P')  // 3U Panel BOB?
	{
		int BBoxEnum = LEDenum;
		//		int Control[1] = { 0x2 };  // Bits 2:1 for "Imax" current division (0 is max, 3 is min)
		//		int ChannelCount = 1;    // 
		//		if (State==1) Control[0] += 1; // chip logic for ON (reverse from Legacy yBBx LED logic)
				// check for 0xFF LEDenum indicating ALL LEDs should be controlled
		if (LEDenum == 0xff)
		{
			for (int L = 0; L <= BBoxLEDenum::AI13; L++)
				BOBLEDcmdArray[L] = State;

		}
		else
		{
			BOBLEDcmdArray[BBoxEnum] = State;
		}
		status = STATUS_SUCCESSFUL;
		// all Control LED control from dllmain thread...
		//		status = LEDControlIS31FL(TdBrd, BBoxEnums, State);// Control, ChannelCount);

	}
	else  // Legacy BOB hardware? (DBB1 or ABBx)
	{
		// have I2C devices for BBox been initialized?
		if (DBB1.GetI2CdevPtr() == nullptr)
		{
			DBB1.SetI2CdevPtr(DBB1_I2C);
			ABB1.SetI2CdevPtr(ABB1_I2C);
			ABB2.SetI2CdevPtr(ABB2_I2C);
			ABB3.SetI2CdevPtr(ABB3_I2C);
		}
		if (LEDenum == 0xFF)
		{
			status = DBB1.Control(TdBrd, 0xFF, (BOOL)State);
			status = ABB1.Control(TdBrd, 0xFF, (BOOL)State);
			ABB2.Control(TdBrd, 0xFF, (BOOL)State); // in Legacy BOB config, ABB2 and ABB3 are not required
			ABB3.Control(TdBrd, 0xFF, (BOOL)State);
		}
		else // single LED
		{
			// we must TRANSLATE API enum (e.g. D0 or AI7)
			// to Legacy equivalents...
			// 
			int translatedEnum = 0xff;
			switch (LEDenum)
			{
			case BBoxLEDenum::D0:
				translatedEnum = TD_LegacyBOB_LEDs::lDIO1;
				break;
			case BBoxLEDenum::D1:
				translatedEnum = TD_LegacyBOB_LEDs::lDIO2;
				break;
			case BBoxLEDenum::D2:
				translatedEnum = TD_LegacyBOB_LEDs::lDIO3;
				break;
			case BBoxLEDenum::D3:
				translatedEnum = TD_LegacyBOB_LEDs::lDIO4;
				break;
			case BBoxLEDenum::D4:
				translatedEnum = TD_LegacyBOB_LEDs::lDIO5;
				break;
			case BBoxLEDenum::D5:
				translatedEnum = TD_LegacyBOB_LEDs::lDIO6;
				break;
			case BBoxLEDenum::D6:
				translatedEnum = TD_LegacyBOB_LEDs::lDIO7;
				break;
			case BBoxLEDenum::D7:
				translatedEnum = TD_LegacyBOB_LEDs::lDIO8;
				break;

			case BBoxLEDenum::AI0:
				translatedEnum = TD_LegacyBOB_LEDs::lAI1;
				break;
			case BBoxLEDenum::AI1:
				translatedEnum = TD_LegacyBOB_LEDs::lAI2;
				break;
			case BBoxLEDenum::AI2:
				translatedEnum = TD_LegacyBOB_LEDs::lAI3;
				break;
			case BBoxLEDenum::AI3:
				translatedEnum = TD_LegacyBOB_LEDs::lAI4;
				break;
			case BBoxLEDenum::AI4:
				translatedEnum = TD_LegacyBOB_LEDs::lAI5;
				break;
			case BBoxLEDenum::AI5:
				translatedEnum = TD_LegacyBOB_LEDs::lAI6;
				break;

			case BBoxLEDenum::AO0:
				translatedEnum = TD_LegacyBOB_LEDs::lAO1;
				break;
			case BBoxLEDenum::AO1:
				translatedEnum = TD_LegacyBOB_LEDs::lAO2;
				break;
			case BBoxLEDenum::AO2:
				translatedEnum = TD_LegacyBOB_LEDs::lAO3;
				break;
			case BBoxLEDenum::AO3:
				translatedEnum = TD_LegacyBOB_LEDs::lAO4;
				break;
			case BBoxLEDenum::AO4:
				translatedEnum = TD_LegacyBOB_LEDs::lAO5;
				break;
			case BBoxLEDenum::AO5:
				translatedEnum = TD_LegacyBOB_LEDs::lAO6;
				break;
			case BBoxLEDenum::AO6:
				translatedEnum = TD_LegacyBOB_LEDs::lAO7;
				break;
			case BBoxLEDenum::AO7:
				translatedEnum = TD_LegacyBOB_LEDs::lAO8;
				break;
			case BBoxLEDenum::AO8:
				translatedEnum = TD_LegacyBOB_LEDs::lAO9;
				break;
			case BBoxLEDenum::AO9:
				translatedEnum = TD_LegacyBOB_LEDs::lAO10;
				break;
			case BBoxLEDenum::AO10:
				translatedEnum = TD_LegacyBOB_LEDs::lAO11;
				break;
			case BBoxLEDenum::AO11:
				translatedEnum = TD_LegacyBOB_LEDs::lAO12;
				break;
			default:  // return ERROR if we don't identify the LED
				return STATUS_DEVICE_NOT_EXISTS;
			}
			// which BreakOut Box is being referenced?  Determine by LED enum
			BBox = GetBboxLEDmask(translatedEnum, &LEDmask);
			if (BBox == nullptr)
				return (THORDAQ_STATUS)STATUS_INVALID_PARAMETER;
			// we have the BBox and I2CDev...
			status = BBox->Control(TdBrd, LEDmask, (BOOL)State);
		}
	}
	return status;
}


THORDAQ_STATUS CThordaq::APIProgressiveScan(BOOL bProgressiveScan)
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

inline THORDAQ_STATUS CThordaq::UserIRQWait(USER_IRQ_WAIT_STRUCT* pUsrIrqWaitStruct, UINT64* IrqCnt0, UINT64* IrqCnt1, UINT64* IrqCnt2, UINT64* IrqCnt3)
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

	// For performance, Do NOT read the 13 separate bit fields
	// 1. Read entire DACWaveGenISR register
	// 2. Re-Arm Wave IRQs
	// 3. Increment separate counters according to ISR
	value = FPGAregisterReadDoMem(_DACWaveGenISRIndex);
	// do we have a potential "race" condition here?  Once we take snapshot read, is there
	// a chance the FPGA will set another "sticky" bit 5 nanosecs after we read,
	// which means the follow "rearm" will clear that fresh bit before we read it?
	//rearmStatus = FPGAregisterWRITE("DAC_Waveplay_IRQ_rearm", 0);  // set to 0 after 1 as per SWUG // this could be set right before rearming instead

	// now update Waveform IRQ counters...
	//value >>= 48; // DAC Waveform Chan0 ISR bit at bit location 48
	IrqCounters[0] = IrqCnt0;
	IrqCounters[1] = IrqCnt1;
	IrqCounters[2] = IrqCnt2;
	IrqCounters[3] = IrqCnt3;

	USHORT isr = value >> 48;

	if (isr)
	{
		rearmStatus = DACReArmWaveplayIRQ(); // clears FPGA's ISR, but not our Shadow bit fields
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

	// Now that we have IRQs from both S2MM image DMA and Waveplay DMA, check both sets of ISR bits

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
			rearmStatus = FPGAregisterWrite(_S2MMDMAControlRegIndex[Chan], _reArmS2MMDMACommand[Chan]); // clear the IRQ
		}
	}

	if (os.hEvent != 0)
	{
		CloseHandle(os.hEvent);
	}
	return status;
}
/*! UserIRQCancel
 *
 * \brief Sends USER_IRQ_CANCEL_IOCTL kernel call to kill the UserInt timer
 * \return IOCTL status
 * \note This call causes the FPGA interrupt processing thread to exit
 */
THORDAQ_STATUS CThordaq::UserIRQCancel(VOID)
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

//////// END NWL restore SDK functions

/////  DZ SDK functions
#pragma optimize( "", off )
inline UINT64 FPGA_HardwareReg::Read(int Field)
{
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
	UINT64 mask = 0xffffffffffffffff >> (static_cast<UINT64>(64) - BitField[Field]->_bitFldLen);
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
inline UINT64 CThordaq::FPGAregisterReadDoMem(int Record)
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

inline void CThordaq::FPGAregisterWriteDoMem(int Record)
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
bool CThordaq::SearchFPGAregFieldIndices(const char* name, int nameSize, int* Record, int* Field)
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

// This function insures that all WRITE only registers have readable copy of last written value
// writes "shadow" copy to our internal structure, then writes to FPGA hardware
void CThordaq::FPGAregisterShadowFPGAwrite(int Record, int Field, UINT64 Value)
{
	_FPGAregister[Record]->Write(Field, Value);  // the "shadow" copy
	FPGAregisterWriteDoMem(Record);              // actual write to FPGA hardware
}

THORDAQ_STATUS CThordaq::FPGAregisterWrite(
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
inline THORDAQ_STATUS CThordaq::FPGAregisterWrite(
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



inline THORDAQ_STATUS CThordaq::FPGAregisterRead(
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

THORDAQ_STATUS CThordaq::FPGAregisterRead(
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

// This function returns zero based index of API defined FPGA indexes based on BARindex and BARoffset
// Returns SUCCESS status and valid FPGAregisterIndex if found
THORDAQ_STATUS CThordaq::FPGAregisterQuery(int BARindex, unsigned int BARoffset, UINT ReadWriteDir, int* FPGAregisterIndex)
{
	int RegIndex = 1;  // 0-based list of WO, RO, and R/W registers (0 is psuedo REG for VERsion)
	for (; RegIndex < TD_LAST_FPGA_REGISTER; RegIndex++)
	{
		if ((_FPGAregister[RegIndex]->_BAR == BARindex) && _FPGAregister[RegIndex]->_BARoffset == BARoffset)
		{
			// we have some registers with same BAR/offset with different READONLY and WRITEONLY definitions
			// 
			if ((_FPGAregister[RegIndex]->_RdWrDir == READWRITE) ||  // for READWRITE, one BAR/offset Register
				_FPGAregister[RegIndex]->_RdWrDir == ReadWriteDir) {
				*FPGAregisterIndex = RegIndex; // found -- success
				return STATUS_SUCCESSFUL;
			}
		}
	}
	return (THORDAQ_STATUS)STATUS_INVALID_PARAMETER;
}

// returns STATUS_SUCCESSFUL if RegIndex/FldIndex exists
// otherwise STATUS_INVALID_PARAMETER
THORDAQ_STATUS CThordaq::FPGAregisterQuery(
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


// Manufacture Test: Set up Waveform Loading for CPLD playback per ThorDAQ FPGFA SWUG
// section "ThorDAQ Waveplay DMA Descriptors"
// REVISED for Oct. 2020 (moves from 0x10000 to 0x50000
// BAR3, Base Addr 0x5.0000 - 0x5.7FFF
//  The 64-bit descriptor (DDR3WafeformDesc) field definition:
//  63 12bits 51 50  (16bits)  35 34 33          (34bits)            0
// |------------|----------------|--|---------------------------------|
// | nextIndex  |   NumBytes     |LP|   DDR3start_address             |
// |------------|----------------|--|---------------------------------|
//
// Starting at BAR3 0x50000, in SINGLE bank config, we have 8192 of these DDR3WafeformDesc descriptors
// from Index 0 to 8191
// The first 12 indexes (0-11) correspond to 12 DAC channels, which have 
// outputs to the 3 analog Breakout Boxes ABB1, ABB2, ABB3 (4 channels per box).
// If the "waveformBuffer" is less than 262,140 bytes, then it's completely described
// by a single descriptor and "nextIndex" simply points back to itself.
// If waveformBuffer is longer, then "nextIndex" value is between 12
// and 4095, and the final descript "nextIndex" points back to its origin.
//
// CONTRAINTS:
// 1. DAC hardware requires 16-bit samples, so NumBytes must be even number
//    to meet hardware constraint
// 2. NumBytes must be evenly divisible by 4 - if there is a remainder,
//    copy the last 16-bit (2-byte) sample to pad remaining len.
// 3. Highest possible DDR3 physical addres in 34 bits: 0x3.FFFF.FFFF
//
// Example showing waveformBuffer of 129 Kbytes (old ~64k max size)                     DDR3 hardware addr
// Index
//			|------------|----------------|------------------------------------|    |----->  0x05000000
//	0	,---|      13    |   65528        |   0x05000000                       |----|		
//		|	|------------|----------------|------------------------------------|           
//		|	|------------|----------------|------------------------------------|             
//	1	|	|      1     |   NumBytes     |   DDR3start_address Chan1          |
//		|	|------------|----------------|------------------------------------|
//		|	|------------|----------------|------------------------------------|
//	2	|	|      2     |   NumBytes     |   DDR3start_address Chan2          |
//		|	|------------|----------------|------------------------------------|
//		|	|------------|----------------|------------------------------------|
//		|	|     ..     |   NumBytes     |   DDR3start_address   ...          |
//		|	|------------|----------------|------------------------------------|
//		|	|------------|----------------|------------------------------------|
//	11	|	|     11     |   NumBytes     |   DDR3start_address Chan 11        |
//		|	|------------|----------------|------------------------------------|
//		|	|------------|----------------|------------------------------------|	 
//	12	|	|     12     |   NumBytes     |   DDR3start_address Chan 12        |
//		|	|------------|----------------|------------------------------------|
//		|	|------------|----------------|------------------------------------|
//	13	`->	|     14     |   65528        |   0x05010000                       |---------->  0x05010000
//		,---|------------|----------------|------------------------------------|
//		|	|------------|----------------|------------------------------------|
//	14	`->	|     0      |   1040         |   0x05020000  (final - back to 0)  |---------->  0x05020000
//			|------------|----------------|------------------------------------|
//			|------------|----------------|------------------------------------|
//			|    ...     |   NumBytes     |   (available)                      |
//			|------------|----------------|------------------------------------|
//			|------------|----------------|------------------------------------|
//			|    8191    |   NumBytes     |   (available)                      |
//			|------------|----------------|------------------------------------|


//TODO: MaxDescTotalBytes and WavePlayTableBARoffset need to be updated
#define WavePlayDMADescriptorBAR 3
#define WavePlaySingleBANKdescriptorCnt 8192
#define WavePlayTableBARoffset 0x50000
// starting baseAddress for DAC waveforms loaded into DDR3
#define MaxDescTotalBytes (262140) // 16 bit field, FPGA requires multiple of 4 (larger size, Oct. 2020 FPGA rev, was 65536-8, mult. of 8)
THORDAQ_STATUS CThordaq::DACwaveInit(
	UINT64 DDR3startAddr,          // location in physical DDR3 mem to start (ADC images in low memory)      
	UINT32 WaveformByteLen         // length in bytes of waveformBuffer (all waveforms should be equal), or '0' if unknown
)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	_DAC_DDR3_startAddress = DDR3startAddr;
	// clear table
	memset(_DACwavePlayDescriptorTable, 0, sizeof(_DACwavePlayDescriptorTable));
	_DACwavePlayDescriptorTableFreeIndex = WAVETABLE_CHANNEL_COUNT; // hardware defined - 12 analog + 2 digital to MUX

	UINT32 channel;
	_DACwaveformDescriptorsPerChannel = WaveformByteLen / MaxDescTotalBytes; // use to partition Waveplay table into channels
	if (WaveformByteLen % MaxDescTotalBytes > 0)
		_DACwaveformDescriptorsPerChannel++;

	// see discussion above -- in FIXED WAVEFORM size mode,
	// the size of waveformBuffer determines how many channels we
	// can simultaneously use

	UINT32 MaxWaveformLen_bytes = _DACwaveformDescriptorsPerChannel * MaxDescTotalBytes;

	// FIXED WAVEFORM SIZE mode?
	// intialize the first 12 channels in BAR table with start baseAddress
	if (WaveformByteLen > 0) // intialize with fixed channel partition sizes?
	{
		for (channel = 0; channel < _DACwavePlayDescriptorTableFreeIndex; channel++)
		{
			_DACwavePlayDescriptorTable[channel].DDR3StartAddr = _DAC_DDR3_startAddress + ((UINT64)channel * (UINT64)MaxWaveformLen_bytes);
		};
	}
	// write INIT table to FPGA...
	STAT_STRUCT DoMemStatus;
	memset(&DoMemStatus, 0, sizeof(DoMemStatus));
	status = DoMem(!(READ_FROM_CARD), WavePlayDMADescriptorBAR, (PUINT8)_DACwavePlayDescriptorTable, 0, WavePlayTableBARoffset, WavePlaySingleBANKdescriptorCnt * 8, &DoMemStatus);
	// copy to shadow register                                                                                                                                                                                                                                     	status = DoMem( !(READ_FROM_CARD), WavePlayDMADescriptorBAR, (PUINT8)_DACwavePlayDescriptorTable, 0, 0x10000, 4096, &DoMemStatus);
	for (channel = 0; channel < _DACwavePlayDescriptorTableFreeIndex; channel++)
		UpdateDACWavePlayDMFPGAShadowRegister(channel, &_DACwavePlayDescriptorTable[channel]);

	return status;
}

void CThordaq::UpdateDACWavePlayDMFPGAShadowRegister(UINT32 DACchannel, PDAC_WAVEPLAY_DESCRIPTOR desc)
{
	UINT64 newShadowValue = desc->DDR3StartAddr | (desc->LooP << 34) | (desc->TotalBytes << 35) | (desc->NextDesc << 51);
	UINT32 RegisterIndex = DACWavePlayDMAdescriptorChan0 + DACchannel;
	_FPGAregister[RegisterIndex]->_VALUE = newShadowValue; // Register itself already written
}

// Update: Oct 2020 SWUG, Reverb alterations
// For now, CL-GUI API isn't supporting REVERB test (no method/process yet)
// The S2MM DMA depends on correct setup of the FPGA's opaque ADC sample line buffer
// The line buffer has fixed programmed length in CONFOCAL/Galvo-Galvo mode where
// sample clock is fixed 6.25 ns
// In laser-sync (2P/3P) mode, line buffer length must be computed.
// The LUT is single-dimensional array of 16-bit values
// max length horizontal image line (e.g. 4096 pixels, 4k)
// Originally LUT was 4k * 2 bytesPerIndex = 8k bytes
// Now, 
// And there is one LUT for each ADC channel (4 used, 2 reserved)
//  SWUG:  "Look Up Table", Pg. 37
//  BAR3
// REVERB/MP
// 0x1.0000 - 0x1.FFFF   ADC Chan0 Imagizer LUT 
// 0x2.0000 - 0x2.FFFF   ADC Chan1 Imagizer LUT 
// 0x3.0000 - 0x3.FFFF   ADC Chan2 Imagizer LUT 
// 0x4.0000 - 0x4.FFFF   ADC Chan3 Imagizer LUT 
//
// <OBSOLETE>
// 0x2000 - 0x3FFF   ADC Chan0 Imagizer LUT 
// 0x4000 - 0x5FFF   ADC Chan1
// 0x6000 - 0x7FFF   ADC Chan2
// 0x8000 - 0x9FFF   ADC Chan3
// 0xA000 - 0xBFFF   ADC Chan4 (reserved)
// 0xC000 - 0xDFFF   ADC Chan5 (reserved)
THORDAQ_STATUS CThordaq::API_ADCsampleImagizerLUT(
	PS2MM_ADCSAMPLE_LUT pS2MMsampleLUT)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	STAT_STRUCT DoMemStatus;
	memset(&DoMemStatus, 0, sizeof(DoMemStatus));

	// copy the array in case it's a .NET buffer - reference pointers from .NET can fail in kernel
	const ULONG cardOffset = 0x10000;
	for (int i = 0; i < MAX_CHANNEL_COUNT; i++)
	{
		status = DoMem(!(READ_FROM_CARD), BAR3, (PUINT8)pS2MMsampleLUT->ADCsampleLUTchan0, 0, cardOffset + SCAN_LUT_MAX_LEN * sizeof(USHORT) * i, SCAN_LUT_MAX_LEN * sizeof(USHORT), &DoMemStatus);
	}
	return status;
}

// get and set the DAC channel voltages by channel and DC voltage
// ADC counts   Voltage
// 0x0    - 0x7FFF  -10.0 to 0
// 0x8000 - 0xFFFF  +0.0 to +10.0

static const char* ParkVoltageRegName[WAVETABLE_CHANNEL_COUNT] = {  // index[12],[13] DigitalWaveform channels
	"DAC_Park_Chan0", "DAC_Park_Chan1","DAC_Park_Chan2","DAC_Park_Chan3","DAC_Park_Chan4", "DAC_Park_Chan5","DAC_Park_Chan6","DAC_Park_Chan7",
	"DAC_Park_Chan8", "DAC_Park_Chan9","DAC_Park_Chan10","DAC_Park_Chan11","DAC_Park_Chan12","DAC_Park_Chan13" };

THORDAQ_STATUS CThordaq::APIsetDACvoltage(UINT32 channel, double Voltage)
{
	USHORT DACunitsMidpoint = Voltage > 0 ? 0x7FFF : 0x8000; // note difference between -0.0 and +0.0
	// about 0.000305176 Volts per DAC count
	USHORT DACunits_toSetVDC = static_cast<USHORT>(floor(Voltage / (20.0 / 65536) + 0.5) + DACunitsMidpoint);

	return FPGAregisterWrite(ParkVoltageRegName[channel], (int)strlen(ParkVoltageRegName[channel]), DACunits_toSetVDC);
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


THORDAQ_STATUS CThordaq::S2MMconfig(
	PS2MM_CONFIG pS2MMconfig)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	int desc, chan, bank, BARchanOffset;
	// SWUG pg. 9, Max Descriptors in a chain
#define MAX_DESCRIPTORS_IN_S2MM_CHAIN 1024
	AXI_DMA_DESCRIPTOR DMA_Desc[MAX_DESCRIPTORS_IN_S2MM_CHAIN][MAX_CHANNEL_COUNT]; // all channels

	// range check
	if ((pS2MMconfig->NumberOfDescriptorsPerBank > MAX_DESCRIPTORS_IN_S2MM_CHAIN)
		||
		pS2MMconfig->NumberOfDescriptorsPerBank < 1) return STATUS_PARAMETER_SETTINGS_ERROR;

	// create the linked list 
	// for each bank
	const UINT8 BYTES_PER_PIXEL = 2;
	UINT32 DDRaddr = pS2MMconfig->DDR3imageStartingAddress;
	UINT32 Stride = pS2MMconfig->HSize * BYTES_PER_PIXEL;
	UINT32 ImageSizeInBytes = Stride * pS2MMconfig->VSize;

	// From SWUG, BAR1 offsets per channel = 0x10000, and additional 0x20 offset for 2nd BANK
	// (channel 0)
	// BARchanOffset -- BAR1 offset to BRAM which holds descriptor list for the specific channel
	UINT32 ChainHead, ChainTail; // record these for FPGA register write
	for (bank = 0; bank < S2MM_BANKS; bank++)
	{
		memset((void*)&DMA_Desc, 0, sizeof(AXI_DMA_DESCRIPTOR) * MAX_DESCRIPTORS_IN_S2MM_CHAIN * 2); // make sure to clear!
		for (chan = 0, BARchanOffset = 0; chan < MAX_CHANNEL_COUNT; chan++, BARchanOffset += 0x10000)
		{
			// is the channel enabled by mask?
			if ((pS2MMconfig->ChannelMask >> chan & 0x1) != 1) continue;
			// each descriptor defines the same, complete frame image
			// both "banks" have Next descriptor links @ 0, 0x40, 0x80, 0xC0, etc.  -- the difference
			// is descriptors for bank 0 are at BAR1 offset 0x0, 40, 80..., while bank 1 are at 0x20, 60, A0... 

			ChainHead = ChainTail = BARchanOffset + (bank * 0x20); // record for FPGA register setting
			for (desc = 0; desc < pS2MMconfig->NumberOfDescriptorsPerBank; desc++, DDRaddr += ImageSizeInBytes) // 1 frame per Descriptor
			{
				DMA_Desc[desc][chan].DDR3Address = DDRaddr; // starting DDR3 addr (for this descriptor/image)
				DMA_Desc[desc][chan].VSIZE = pS2MMconfig->VSize; // "lines"
				DMA_Desc[desc][chan].HSIZE = Stride;             // horizontal "pixels" in BYTES
				DMA_Desc[desc][chan].Stride = Stride;
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
			status = CThordaq::WriteS2MMdescChain(chan, bank, ChainHead, 0x510, DMA_Desc, pS2MMconfig);
			// Ping-Pong buffer start Bank
			// NOTE: always tell FPGA to start at Bank0.  The DDR3 starting baseAddress (Bank) for read depends on whether 
			// we want "Live" progressive scan (continuously read DDR3 being written by S2MM DMA) or
			// "Capture" of the S2MM DMA that is complete (not being written by S2MM DMA)
			status = FPGAregisterWrite(BankSelRegName[chan], (int)strlen(BankSelRegName[chan]), 0); // bank to start from
			// Enable S2MM DMA engine -- first  _RUN_STOP to 1, followed by _CONFIG_DMA to 1.
			status = FPGAregisterWrite(RunStopRegName[chan], (int)strlen(RunStopRegName[chan]), 1);
			status = FPGAregisterWrite(ConfigRegName[chan], (int)strlen(ConfigRegName[chan]), 1);
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

THORDAQ_STATUS CThordaq::WriteS2MMdescChain(int ADCchannel, int BRAMbank, int ChainHead,
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
inline THORDAQ_STATUS CThordaq::WriteDDR3(PUINT8 HostSourceAddress, UINT64 DDR3DestinationAddress, UINT32 ByteLength)
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
inline THORDAQ_STATUS CThordaq::ReadDDR3(UINT64 DDR3SourceAddress, PUINT8 HostDestinationAddress, PUINT32 ByteLength)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	UINT64 UserControl = 0;
	PUINT8 NotUsedFIFObuf = NULL;
	PUINT32 NotUsedBufferSize = 0;
	PUINT32 NotUsedFIFOMaxPacketSize = 0;
	INT32 NumDMAdescriptors = 8192;

	THORDAQ_STATUS PktSetupStatus = SetupPacket(32, NotUsedFIFObuf, NotUsedBufferSize, NotUsedFIFOMaxPacketSize, PACKET_MODE_ADDRESSABLE, 8192);
	if (PktSetupStatus != STATUS_SUCCESSFUL)
		DebugBreak();

	status = PacketReadEx(32, &UserControl, DDR3SourceAddress, 0, HostDestinationAddress, ByteLength);
	if (status != STATUS_SUCCESSFUL)
		DebugBreak();

	THORDAQ_STATUS RelStatus = ReleasePacketBuffers(32); // releases Descriptor resources
	if (RelStatus != STATUS_SUCCESSFUL)
		DebugBreak();


	return status;
}



// "DACwaveLoad" segments the total byte length of "DACsampleBuffer" into a linked-list of addressable 
// waveformBuffer playback table descriptors for the passed DACchannel, then loads the waveformBuffer itself into 
// DDR3 memory. Since there are 4096 descriptors, each one addressing (nominally) 64k of memory, only
// ~256MB of TOTAL waveformBuffer data (for all channels) can be described.
// If/when the supply of 4096 descriptors is exhausted or the waveformBuffer file length exceeds the limit,
// STATUS_PARAMETER_SETTINGS_ERROR is returned
THORDAQ_STATUS CThordaq::DACwaveLoad(
	UINT32 DACchannel,             // DAC hardware channel 
	PUINT8 DACsampleBuffer,        // array of 16-bit waveformBuffer samples to load
	UINT32 BufferSize,             // Total size (bytes) of sample buffer
	PUINT64 DDR3startAddr     // if successful, the DDR3 start addr where waveformBuffer is loaded
)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	//	UINT32 DescriptorsPerChannel;
	UINT32 DescriptorsNeededForThisChannel; // how many descriptor-links needed?
	// check to see if descriptor table has been correctly initialized for this wavelen
	STAT_STRUCT DoMemStatus;
	memset(&DoMemStatus, 0, sizeof(DoMemStatus));

	DescriptorsNeededForThisChannel = BufferSize / MaxDescTotalBytes;
	if (BufferSize % MaxDescTotalBytes > 0)
		DescriptorsNeededForThisChannel++;

	*DDR3startAddr = 0; // assume invalid waveformBuffer error / cannot load

	UINT32 MaxWaveformLen_Loadable_bytes;
	if (_DACwaveformDescriptorsPerChannel > 0)
	{
		MaxWaveformLen_Loadable_bytes = _DACwaveformDescriptorsPerChannel * MaxDescTotalBytes;
	}
	else
		MaxWaveformLen_Loadable_bytes = (4095 - _DACwavePlayDescriptorTableFreeIndex) * MaxDescTotalBytes;

	// first, determine whether 
	// range checks
	if (BufferSize > MaxWaveformLen_Loadable_bytes || (_DACwavePlayDescriptorTableFreeIndex >= 4095))
	{

		return STATUS_PARAMETER_SETTINGS_ERROR;
	}
	INT32 RemainingWaveLength = (INT32)BufferSize;
	//	UINT32 TableFreeIndex = 12 + (_DACwaveformDescriptorsPerChannel * DACchannel); // FPGA says free indexes start at 12
	// Each descriptor can hold 65536 samples - if needed break total BufferSize into 'TotalBytes' segments
	//		 DDR3StartAddr : 36; // bits  0 - 35
	//		 TotalBytes    : 16; // bits 36 - 51
	//		 NextDesc      : 12; // bits 52 - 63
		// The FPGA pre-defines first channel descriptor...
		// Because size is fixed, we know exactly where to start additional descriptors
	int ThisSegmentLen = MaxDescTotalBytes;  // init assumes we'll need more than 1 descriptor..
	// handle the FIRST segment separately because indexes are dedicated...
	if (RemainingWaveLength / MaxDescTotalBytes > 0)
	{
		ThisSegmentLen = MaxDescTotalBytes;
	}
	else  // first and last segment... 
	{
		ThisSegmentLen = RemainingWaveLength;
	}

	_DACwavePlayDescriptorTable[DACchannel].DDR3StartAddr = _DAC_DDR3_startAddress;
	_DACwavePlayDescriptorTable[DACchannel].LooP = 0; // no looping for this config
	_DACwavePlayDescriptorTable[DACchannel].NextDesc = DACchannel; // per FPGA, the FIRST channel pre-defined
	_DACwavePlayDescriptorTable[DACchannel].TotalBytes = ThisSegmentLen / 4;
	if (ThisSegmentLen % 4 != 0)
	{
		DebugBreak();
	}

	UINT64 SegmentDDR3startAddr = _DAC_DDR3_startAddress;
	UINT32 LastTableIndex = DACchannel;
	SegmentDDR3startAddr += ThisSegmentLen;

	RemainingWaveLength -= ThisSegmentLen;
	// additional segments if needed...
	while (RemainingWaveLength > 0)
	{
		if (RemainingWaveLength / MaxDescTotalBytes > 0)
		{
			ThisSegmentLen = MaxDescTotalBytes;
		}
		else  // last segment... 
		{
			ThisSegmentLen = RemainingWaveLength;
		}
		// CPLD hardware fetches 8 bytes at a time, so if length is not multiple of 8,
		// copy the last 2-byte sample to fill out 8-byte segment (TBD)

		// now set descriptor fields
		_DACwavePlayDescriptorTable[LastTableIndex].NextDesc = _DACwavePlayDescriptorTableFreeIndex; // mark last index with current
		//		status = DoMem( !(READ_FROM_CARD), WavePlayDMADescriptorBAR, (PUINT8)_DACwavePlayDescriptorTable, 0, (0x10000 + LastTableIndex), 8, &DoMemStatus);

		_DACwavePlayDescriptorTable[_DACwavePlayDescriptorTableFreeIndex].TotalBytes = ThisSegmentLen / 4;
		_DACwavePlayDescriptorTable[_DACwavePlayDescriptorTableFreeIndex].DDR3StartAddr = SegmentDDR3startAddr;
		_DACwavePlayDescriptorTable[_DACwavePlayDescriptorTableFreeIndex].NextDesc = DACchannel; // assume each segment is LAST, which points back to first
		//		status = DoMem( !(READ_FROM_CARD), WavePlayDMADescriptorBAR, (PUINT8)_DACwavePlayDescriptorTable, 0, (0x10000 + _DACwavePlayDescriptorTableFreeIndex), 8, &DoMemStatus);


		LastTableIndex = _DACwavePlayDescriptorTableFreeIndex;
		_DACwavePlayDescriptorTableFreeIndex++;  // contiguous free descriptors (until we exhaust pre-define supply)
		if (_DACwavePlayDescriptorTableFreeIndex > 4095) // FATAL error: exhausted table links
		{
			_DACwavePlayDescriptorTableFreeIndex = 4095; // do not wrap around
			return STATUS_PARAMETER_SETTINGS_ERROR;
		}
		SegmentDDR3startAddr += ThisSegmentLen;
		RemainingWaveLength -= ThisSegmentLen;
	};

	*DDR3startAddr = _DACwavePlayDescriptorTable[DACchannel].DDR3StartAddr;

	// SUCCESS!  The waveformBuffer can be described in the table for CPLD access - now write
	// it to DDR3

	status = WriteDDR3(DACsampleBuffer, *DDR3startAddr, BufferSize);

	// now write the completed table for this DACchannel to FPGA BAR  
	status = DoMem(!(READ_FROM_CARD), WavePlayDMADescriptorBAR, (PUINT8)_DACwavePlayDescriptorTable, 0, WavePlayTableBARoffset, 4096 * 8, &DoMemStatus);
	// copy to shadow register                                                                                                                                                                                                                                     	status = DoMem( !(READ_FROM_CARD), WavePlayDMADescriptorBAR, (PUINT8)_DACwavePlayDescriptorTable, 0, 0x10000, 4096, &DoMemStatus);
	UpdateDACWavePlayDMFPGAShadowRegister(DACchannel, &_DACwavePlayDescriptorTable[DACchannel]);
	// update next free DDR3 location
	_DAC_DDR3_startAddress += BufferSize;
	// must end on 8-byte boundary
	UINT64 roundto8bytes = _DAC_DDR3_startAddress % 8;
	if (roundto8bytes > 0)
		_DAC_DDR3_startAddress += (8 - roundto8bytes);


	return status;
}

// Replaces original (Carl) driver Register read/write with ThorDAQ API shadow register version
THORDAQ_STATUS CThordaq::WriteReadRegister(UINT read_write_flag, UINT register_bar_num, ULONGLONG register_card_offset, BYTE buffer[sizeof(UINT64)], ULONGLONG offset, ULONGLONG length, PSTAT_STRUCT completed_status)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	int FPGAregisterIndex = 0;

	// "Register" should never exceed 64-bits (8 bytes)
	if (length > sizeof(UINT64))
	{
		return STATUS_PARAMETER_SETTINGS_ERROR;
	}

	status = CThordaq::FPGAregisterQuery(register_bar_num, static_cast<UINT32>(register_card_offset), read_write_flag, &FPGAregisterIndex);
	if (status != STATUS_SUCCESSFUL) // check for undefined/unsupported registers
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (read_write_flag == WRITE_TO_CARD)
	{
		//convert to a unsigned 64bit integer, only using the bytes up to "length"
		UINT64 val = 0;
		const int BITS_PER_BYTE = 8;
		for (int i = 0, j = 0; i < length; ++i, j += BITS_PER_BYTE)
		{
			val |= (UINT64)buffer[i] << j;
		}
		FPGAregisterShadowFPGAwrite(FPGAregisterIndex, -1, val); // ensures shadow copy plus FPGA hardware
	}
	else
	{
		FPGAregisterRead(FPGAregisterIndex, -1, (UINT64*)buffer);
	}

	return status;
}
/////  End DZ SDK functions

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS CThordaq:: GetDMAEngineCap ( ULONG dma_engine_offset, PDMA_CAP_STRUCT dma_capability)
 *
 * @brief	Get DMA Engine Capabilitie.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param	dma_engine_offset	DMA Engine number offset to use.
 * @param	dma_capability   	Returned DMA Engine Capabilitie.
 *
 * @return	STATUS_SUCCESSFUL if Thordaq return. Others if problem happened.
 **************************************************************************************************/
THORDAQ_STATUS CThordaq::GetDMAEngineCap(ULONG dma_engine_offset, PDMA_CAP_STRUCT dma_capability)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	OVERLAPPED				overlapped;			// OVERLAPPED structure for the operation
	DWORD					bytes_returned = 0;
	DWORD					last_error_status = 0;
	//Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	// Send BLOCK_DIRECT_GET_PERF_IOCTL IOCTL
	if (DeviceIoControl(gHdlDevice, GET_DMA_ENGINE_CAP_IOCTL, (LPVOID)&dma_engine_offset, sizeof(ULONG), (LPVOID)dma_capability, sizeof(DMA_CAP_STRUCT), &bytes_returned, &overlapped) == 0)
	{
		last_error_status = GetLastError();

		if (last_error_status != ERROR_IO_PENDING)
		{
#if _DEBUG
			printf("Getdma_capability IOCTL call failed. Error = %d\n", last_error_status);
#endif
			status = STATUS_GET_BOARD_CONFIG_ERROR;
		}
		else
		{
			// Wait here (forever) for the Overlapped I/O to complete
			if (!GetOverlappedResult(gHdlDevice, &overlapped, &bytes_returned, TRUE))
			{
				last_error_status = GetLastError();
#if _DEBUG
				printf("Getdma_capability IOCTL call failed. Error = %d\n", last_error_status);
#endif // _DEBUG
				status = STATUS_GET_BOARD_CONFIG_ERROR;
			}
		}
	}
	// check returned structure size
	if ((bytes_returned != sizeof(DMA_CAP_STRUCT)) &&
		(status == STATUS_SUCCESSFUL))
	{
		// ioctl failed
#if _DEBUG
		printf("Getdma_capability IOCTL returned invalid size (%d)\n", bytes_returned);
#endif // _DEBUG
		status = STATUS_GET_BOARD_CONFIG_ERROR;
	}
	if (overlapped.hEvent != 0)
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
THORDAQ_STATUS CThordaq::APItdCancelUserIntTask()
{
	THORDAQ_STATUS status;

	status = UserIRQCancel();
	return status;
}

inline THORDAQ_STATUS CThordaq::APItdUserIntTask(USER_IRQ_WAIT_STRUCT* usrIrqWaitStruct)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	wchar_t logMsg[MSG_SIZE];
	StringCbPrintfW(logMsg, MSG_SIZE, L"ThorDAQ APItdUserIntTask() enter, sizeof(USER_IRQ_WAIT_STRUCT)=%d", (int)sizeof(USER_IRQ_WAIT_STRUCT));
	CThordaq::LogMessage(logMsg, VERBOSE_EVENT);
	UINT64 IrqCnt0, IrqCnt1, IrqCnt2, IrqCnt3;
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

	//StringCbPrintfW(logMsg, MSG_SIZE, L"ThorDAQ APItdUserIntTask() exit, status 0x%x", status);
	//CThordaq::LogMessage(logMsg, VERBOSE_EVENT);

	return status;
}

//TODO: move static variables to the class definition
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
THORDAQ_STATUS CThordaq::APIReadFrames(UINT64* buffer_length, void* SySHostBuffer, double Timeout_ms, ULONG transferFrames, BOOL isLastTransfer, BOOL& isPartialData)
{
	/*
  New kernel design aims to move entire BANK of image data - all channels, all frames.
  The word and variable "frame" has at least different meanings in legacy code

  DDR3 ADC (Image) memory organization
  EXAMPLE for 512 x 512, with NumDescriptors = 2 (frameNumPerTransfer,
	a.k.a. frm_per_txn for S2MM config)

Chan0 0x00,0000		BANK0
	  0x08,0000
Chan1 0x10,0000
	  0x18,0000
Chan2 0x20,0000
	  0x28,0000
Chan3 0x30,0000
	  0x38,0000
Chan0 0x40,0000		BANK1
	  0x48,0000
Chan1 0x50,0000
	  0x58,0000
Chan2 0x60,0000
	  0x78,0000
Chan3 0x70,0000
	  0x78,0000

*/

	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	UINT64 DDR3startAddr = 0x0;
	INT  msToWaitForBankSwitch = (INT)Timeout_ms;
	_abortReadImage = false;

	if (_usrIrqWaitStruct.DMA_Bank == 0)
	{
		iterationsPerBank = 0; // debug counter

		_s2mmBankLastRead = 1; // progressive scan - immediately read live bank. // capture - wait for 1st bank switch (IRQ)
	}

	if (_bProgressiveScan == TRUE) // set by new ThorDAQAPI call
	{
		BOOL currentBank = BOOL(_usrIrqWaitStruct.DMA_Bank & 0x00000001);

		//if we received an interrupt we can copy a fully acquired frame image once before moving to progressive images again
		if (currentBank != _s2mmBankLastRead && _usrIrqWaitStruct.DMA_Bank != 0)
		{
			DDR3startAddr = _s2mmBankLastRead * (*buffer_length);
			isPartialData = FALSE;
		}
		else
		{
			DDR3startAddr = currentBank * (*buffer_length);

			isPartialData = TRUE;
		}
		// READ as fast as possible...
		// setup and teardown the user application image buffer (MDL page lock) on every transfer
		status = ReadDDR3(DDR3startAddr, (PUINT8)SySHostBuffer, (PUINT32)buffer_length);
		if (status != STATUS_SUCCESSFUL)
			DebugBreak();
		_s2mmBankLastRead = currentBank;
	}
	else  // wait for FPGA hardware IRQ to establish timing base for acquired images
	{
		if (transferFrames == gPtrAcqCtrl->gblCtrl.frm_per_txn)
		{
			// READ at rate of the Bank interrupt... do NOT return from this routine
			// until Bank switch detected
			while ((_s2mmBankLastRead != BOOL(_usrIrqWaitStruct.DMA_Bank & 0x00000001)) && (--msToWaitForBankSwitch > 0) && !_abortReadImage)
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

			if (_abortReadImage)
			{
				status = STATUS_ACQUISITION_ABORTED;
				return status;
			}

			// ready to read completed bank
			BOOL  currentBank = BOOL(_usrIrqWaitStruct.DMA_Bank & 0x00000001);

			//data is being written to the currentBank now, we must read from the opposite bank
			BOOL readFromBank = !currentBank;
			// setup and teardown the user application image buffer (MDL page lock) on every transfer
			DDR3startAddr = readFromBank * (*buffer_length);
			status = ReadDDR3(DDR3startAddr, (PUINT8)SySHostBuffer, (PUINT32)buffer_length);
			if (status != STATUS_SUCCESSFUL)	DebugBreak();
			_s2mmBankLastRead = readFromBank;
			isPartialData = FALSE;
		}
		else if (transferFrames < gPtrAcqCtrl->gblCtrl.frm_per_txn && TRUE == isLastTransfer)
		{
			DWORD sleepTime = (DWORD)gPtrAcqCtrl->gblCtrl.frm_per_sec * gPtrAcqCtrl->gblCtrl.frm_per_txn;
			Sleep(sleepTime);
			//data is being written to the currentBank now, we must read from the opposite bank
			BOOL readFromBank = !_s2mmBankLastRead;
			DDR3startAddr = readFromBank * (*buffer_length);
			status = ReadDDR3(DDR3startAddr, (PUINT8)SySHostBuffer, (PUINT32)buffer_length);
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
 * @fn	THORDAQ_STATUS CThordaq::AbortPacketRead()
 *
 * @brief	Abort reading.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS CThordaq::AbortPacketRead()
{
	_abortReadImage = true;
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	return status;
}


/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS CThordaq::PacketWriteBuffer( ULONG64 register_card_offset, ULONG Length, UCHAR* Buffer, ULONG Timeout )
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

THORDAQ_STATUS CThordaq::PacketWriteBuffer(
	ULONG64 DDR3_address,         // Start baseAddress in FPGA controlled DDR3 (destination)
	ULONG   Length,               // Size of the Packet Write to Data Buffer Count by Byte (FIFO Mode)
	UCHAR* Buffer,                // Host memory buffer (source)
	ULONG   Timeout               // Generate Timeout error when timeout
)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	status = WriteDDR3(Buffer, DDR3_address, Length);


	return status;
}


void CThordaq::LogMessage(wchar_t* logMsg, long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
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

THORDAQ_STATUS CThordaq::GlobalSCANstart(bool Start) // if TRUE, start GlobalSCAN, otherwise STOP
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	wchar_t logMsg[MSG_SIZE];
	UINT64 GIGCR0_STOP_RUNBitValue = 0xFFFF;
	if (gPtrAcqCtrl != nullptr)
	{
		if (Start == true)  // START GlobalSCAN
		{
			// use the SHADOW REGISTER bit to see if we ALREADY set GlobalSCAN as caller requests...
			// This prevents creating multiple threads for IRQ handling, so we can
			// invoke Global start separately or combined with Image DMA and Waveform DMA
			FPGAregisterREAD("GIGCR0_STOP_RUN", &GIGCR0_STOP_RUNBitValue);
			if (GIGCR0_STOP_RUNBitValue == 1)
			{
				return STATUS_SUCCESSFUL;
			}
			if (!_dacContinuousModeStartStopStatus)
			{
				_usrIrqWaitStruct.boardNum = gBoardIndex;      // routine args:  boardIndex, DMA Bank indicator, int. timeout, etc.
				_usrIrqWaitStruct.DMA_Bank = 0;
				_usrIrqWaitStruct.NWL_Common_DMA_Register_Block = &_NWL_Common_DMA_Register;
				SAFE_DELETE_PTR(_irqThread); // prior thread should have terminated
				_irqThread = new std::thread([this] { this->APItdUserIntTask(&_usrIrqWaitStruct); });
				_irqThread->detach();  // makes WinOS responsible for releasing thread resources on termination.
				SetThreadPriority(_irqThread->native_handle(), THREAD_PRIORITY_HIGHEST);
				//give time to the irq thread to start before setting the run bit
				Sleep(1);

				FPGAregisterWRITE("NWL_UserIntEnable", 0x1); // ENable FPGA hardare interrupts
				FPGAregisterWRITE("GIGCR0_LED2", 0x1);      //  Acq. LED ON	
			}
			FPGAregisterWRITE("GIGCR0_STOP_RUN", 0x1);  // start everything			
			gPtrAcqCtrl->gblCtrl.run_stop_mode = PACKET_GEN_RUN; // legacy code concern				
		}
		else  // STOP the GlobalSCAN
		{
			gPtrAcqCtrl->gblCtrl.run_stop_mode = PACKET_GEN_STOP;
			if (!_dacContinuousModeStartStopStatus)
			{
				FPGAregisterWRITE("NWL_UserIntEnable", 0x0); // Disable FPGA hardare interrupts 
				FPGAregisterWRITE("GIGCR0_LED2", 0x0);      // Acq. LED OFF
			}
			FPGAregisterWRITE("GIGCR0_STOP_RUN", 0x0);   // All FPGA scan related processes disabled 

			// Causes NWL UserInterrupt handling thread to exit (user INTs are disabled)
			StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::GlobalSCANstart() Start = %d", Start);
			CThordaq::LogMessage(logMsg, VERBOSE_EVENT);
			if (!_dacContinuousModeStartStopStatus)
			{
				status = APItdCancelUserIntTask();  // we don't expect IOCTL to fail, but...
			}
			if (status != STATUS_SUCCESSFUL)
			{
				StringCbPrintfW(logMsg, MSG_SIZE, L"CThordaq::GlobalSCANstart(false), APItdCancelUserIntTask() failed, status 0x%x", status);
				CThordaq::LogMessage(logMsg, VERBOSE_EVENT);
			}
		}
	}
	return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS CThordaq::StartAcquisition()
 *
 * @brief	Stop acquisition.
 *
 * @author	Cge
 * @date	3/17/2017
 * @date    12/11/2020 - DZimmerman, couple GlobalSCAN process
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS CThordaq::StartAcquisition()
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	if (gPtrAcqCtrl != nullptr)
	{
		_abortReadImage = false;
		_imagingStartStopStatus = true;

		_usrIrqWaitStruct.DMA_Bank = 0;

		if (gPtrAcqCtrl->galvoCtrl.twoBankSystemEnable)
		{
			SAFE_DELETE_PTR(_imagingDACBankSwitchingTrackingThread);
			_imagingDACBankSwitchingTrackingThread = new std::thread([this] { this->ImagingDACBanckSwitchingTrackCurrentBank(); });
			_imagingDACBankSwitchingTrackingThread->detach();
		}

		if (_imagingLevelTriggerActive)
		{
			SAFE_DELETE_PTR(_imagingTrackLevelTriggerAndRearmS2MMThread);
			_imagingTrackLevelTriggerAndRearmS2MMThread = new std::thread([this] { this->ImagingTrackLevelTriggerAndRearmS2MM(); });
			_imagingTrackLevelTriggerAndRearmS2MMThread->detach();
		}
		status = GlobalSCANstart(true); // start scan

		if (status != THORDAQ_STATUS::STATUS_SUCCESSFUL)
		{
			_imagingStartStopStatus = false;
		}

	}
	return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS CThordaq::StopAcquisition()
 *
 * @brief	Stop acquisition.
 *
 * @author	Cge
 * @date	3/17/2017
 * @date    12/11/2020 - DZimmerman, couple GlobalSCAN process
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS CThordaq::StopAcquisition()
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	if (gPtrAcqCtrl != nullptr)
	{
		status = GlobalSCANstart(false); // stop scan
	}
	memset(_isDACChannelEnabledForImaging, 0, sizeof(bool) * WAVETABLE_CHANNEL_COUNT);
	_imagingStartStopStatus = false;
	_abortReadImage = true;
	_imagingEventPrepared = false;
	return status;
}



//TODO: update to use Don's new logic
//Toggle the Aux_digital_output ports. The value they toggle to will depend on value high = 1 and low = 0
//auxChannelIndex is 0 based, goes from 0 - 4. Register 0x30 (GlobalGPOAuxReg) is a 16-bit register that toggles aux digital lines 0-4
THORDAQ_STATUS CThordaq::ToggleAuxDigitalOutputs(USHORT auxChannelIndex, USHORT value)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;

	if (auxChannelIndex >= MAX_AUX_DIG_OUT_CHANNEL)
	{
		return STATUS_PARAMETER_SETTINGS_ERROR;
	}

	if (1 == value)
	{
		_auxDigitalOutToggleVal |= (1 << auxChannelIndex);
	}

	if (0 == value)
	{
		_auxDigitalOutToggleVal &= ~(1 << auxChannelIndex);
	}

	UINT64 uiValue = (UINT64)_auxDigitalOutToggleVal;
	FPGAregisterWRITE("GlobalGPOAuxReg", uiValue);
	ThordaqErrChk(L"FPGAregisterWRITE", status = FPGAregisterWRITE("GlobalGPOAuxReg", uiValue)); // i.e. 0x30, Ashraf register
	if (status != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	return status;
}

void CThordaq::TriggerLogicResponse(THORDAQ_TRIGGER_SETTINGS triggerSettings, BYTE lutAddress, bool& do0, bool& do1)
{
	BYTE addressHWTrigger1 = 0;
	bool ignoreHWTrigger1 = false;
	switch (triggerSettings.hwTrigger1Mode)
	{
	case THORDAQ_HW_TRIGGER_MODES::THORDAQ_NO_TRRIGER:
		ignoreHWTrigger1 = true;
		addressHWTrigger1 = 0x0;
		break;
	case THORDAQ_HW_TRIGGER_MODES::THORDAQ_HARDWARE_EDGE_TRIGGER_MODE:
		addressHWTrigger1 = 0x2;
		break;
	case THORDAQ_HW_TRIGGER_MODES::THORDAQ_HARDWARE_LEVEL_TRIGGER_MODE:
		addressHWTrigger1 = 0x1;
		break;
	default:
		break;
	}

	BYTE addressHWTrigger2 = 0;
	bool ignoreHWTrigger2 = false;
	switch (triggerSettings.hwTrigger2Mode)
	{
	case THORDAQ_HW_TRIGGER_MODES::THORDAQ_NO_TRRIGER:
		ignoreHWTrigger2 = true;
		addressHWTrigger2 = 0x0;
		break;
	case THORDAQ_HW_TRIGGER_MODES::THORDAQ_HARDWARE_EDGE_TRIGGER_MODE:
		addressHWTrigger2 = 0x8;
		break;
	case THORDAQ_HW_TRIGGER_MODES::THORDAQ_HARDWARE_LEVEL_TRIGGER_MODE:
		addressHWTrigger2 = 0x4;
		break;
	default:
		break;
	}

	BYTE digitalGPOWaveAddress = 0;	
	bool ignoreDigitalGPO = false;

	if (!triggerSettings.enableInternalDigitalTrigger)
	{
		ignoreDigitalGPO = true;
		digitalGPOWaveAddress = 0x0;
	}
	else
	{
		ignoreHWTrigger2 = false;
		digitalGPOWaveAddress = 0x10;
	}

	do0 = false;
	switch (triggerSettings.logicOperand)
	{
	case THORDAQ_TRIGGER_LOGIC_OPERANDS::THORDAQ_AND:
		do0 = (ignoreHWTrigger1 || ((addressHWTrigger1 & lutAddress) != 0)) && (ignoreHWTrigger2 || ((addressHWTrigger2 & lutAddress) != 0)) && (ignoreDigitalGPO || ((digitalGPOWaveAddress & lutAddress) != 0));
		break;
	case THORDAQ_TRIGGER_LOGIC_OPERANDS::THORDAQ_OR:
		do0 = ((addressHWTrigger1 & lutAddress) != 0) || ((addressHWTrigger2 & lutAddress) != 0) || ((digitalGPOWaveAddress & lutAddress) != 0);
		break;
	default:
		break;
	}
	do1 = false;
}


template <class T>
string to_string(T t, ios_base& (*f)(ios_base&))
{
	ostringstream oss;
	oss << f << t;
	return oss.str();
}


LONG CThordaq::ExportScript(DATA_ACQ_CTRL_STRUCT* gPtrAcqCtrl, SCAN_LUT scanLUT)
{
	ofstream myfile;
	myfile.open("script.txt");

	//myfile << "writemem 3 0x00 1 0x00\n";
	//myfile << "writemem 3 0x08 1 0x" << to_string<USHORT>(gPtrAcqCtrl->gblCtrl.img_scan_mode, hex) << endl;
	//myfile << "writemem 3 0x10 1 0x" << to_string<USHORT>(gPtrAcqCtrl->gblCtrl.hor_pix_num - 1, hex) << endl;
	//myfile << "writemem 3 0x18 1 0x" << to_string<USHORT>(gPtrAcqCtrl->gblCtrl.vrt_pix_num - 1, hex) << endl;
	//myfile << "writemem 3 0x20 8 0x" << to_string<ULONG64>(gPtrAcqCtrl->gblCtrl.gpio_cfg, hex) << endl;
	//myfile << "writemem 3 0x140 1 0x" << to_string<USHORT>(gPtrAcqCtrl->scan.sync_ctrl, hex) << endl;
	//myfile << "writemem 3 0x142 2 0x" << to_string<USHORT>(gPtrAcqCtrl->scan.frm_cnt, hex) << endl;
	//myfile << "writemem 3 0x148 1 0x42\n";
	//myfile << "writemem 3 0x150 2 0x" << to_string<USHORT>(gPtrAcqCtrl->scan.pll_sync_offset, hex) << endl;
	//myfile << "writemem 3 0x160 4 0x" << to_string<ULONG32>(gPtrAcqCtrl->scan.galvo_pixel_dwell, hex) << endl;
	//myfile << "writemem 3 0x168 4 0x" << to_string<ULONG32>(gPtrAcqCtrl->scan.galvo_pixel_delay, hex) << endl;
	//myfile << "writemem 3 0x170 4 0x" << to_string<ULONG32>(gPtrAcqCtrl->scan.galvo_intra_line_delay, hex) << endl;
	//myfile << "writemem 3 0x178 4 0x" << to_string<ULONG32>(gPtrAcqCtrl->scan.galvo_intra_frame_delay, hex) << endl;

	//myfile << "writemem 3 0x180 1 0x" << to_string<USHORT>(gPtrAcqCtrl->samplingClock.controlRegister0, hex) << endl;
	//myfile << "writemem 3 0x188 2 0x" << to_string<USHORT>(gPtrAcqCtrl->samplingClock.phase_offset, hex) << endl;
	//myfile << "writemem 3 0x190 1 0x" << to_string<USHORT>(gPtrAcqCtrl->samplingClock.phase_step, hex) << endl;
	//myfile << "writemem 3 0x198 2 0x" << to_string<USHORT>(gPtrAcqCtrl->samplingClock.phase_limit, hex) << endl;

	//myfile << "writemem 3 0x1c0 1 0x" << to_string<USHORT>(gPtrAcqCtrl->streamProcessing.stream_ctrl, hex) << endl;
	//myfile << "writemem 3 0x1c2 1 0x" << to_string<USHORT>(gPtrAcqCtrl->streamProcessing.stream_ctrl2, hex) << endl;
	//myfile << "writemem 3 0x1c8 2 0x" << to_string<USHORT>(gPtrAcqCtrl->streamProcessing.scan_period, hex) << endl;
	//myfile << "writemem 3 0x1d0 4 0x" << to_string<ULONG32>(gPtrAcqCtrl->streamProcessing.downsample_rate, hex) << endl;

	//myfile << "writemem 3 0x200 2 0x8043\n";
	//myfile << "writemem 3 0x200 2 0x8040\n";


	for (int j = 0; j < 4; j++)
	{
		myfile << "\n\\\\LUT\n\n";
		USHORT startAddress = 0x2000 + j * 0x2000;
		for (ULONG32 i = 0; i < gPtrAcqCtrl->gblCtrl.hor_pix_num; i++)
		{
			myfile << "writemem 3 0x" << to_string<USHORT>(startAddress + i * 2, hex) << " 2 0x" << to_string<USHORT>(scanLUT.lut[i], hex) << endl;
		}
	}

	myfile.close();
	return TRUE;
}

BYTE* CThordaq::GetNewBufferOfSize(size_t bufferSize)
{
	if (MIN_MEM_BUFFER_SIZE > bufferSize)
	{
		return new BYTE[MIN_MEM_BUFFER_SIZE]();
	}

	return new BYTE[bufferSize];
}

wstring getStringFromEvent(EVT_HANDLE publisherMetaData, EVT_HANDLE event, EVT_FORMAT_MESSAGE_FLAGS formatId)
{
	LPWSTR buffer = NULL;
	DWORD bufferSize = 0;
	DWORD bufferUsed = 0;
	DWORD error = 0;

	wstring result = L"";

	if (!EvtFormatMessage(publisherMetaData, event, 0, 0, NULL, formatId, bufferSize, buffer, &bufferUsed))
	{
		error = GetLastError();
		if (ERROR_INSUFFICIENT_BUFFER == error)
		{
			if ((EvtFormatMessageKeyword == formatId))
				buffer[bufferSize - 1] = L'\0';
			else
				bufferSize = bufferUsed;

			buffer = (LPWSTR)malloc(bufferSize * sizeof(WCHAR));

			if (buffer)
			{
				EvtFormatMessage(publisherMetaData, event, 0, 0, NULL, formatId, bufferSize, buffer, &bufferUsed);

				// Add the second null terminator character.
				if ((EvtFormatMessageKeyword == formatId))
					buffer[bufferUsed - 1] = L'\0';
			}
		}
	}

	// Cleanup
	if (buffer)
	{
		result = std::wstring(buffer);
		free(buffer);
	}

	return result;
}


ShutDownType CThordaq::GetShutDownTypeSinceLastConnection(INT64 lastConnectionTime)
{
	ShutDownType shutDownType = 0 < lastConnectionTime ? ShutDownType::NOSHUTDOWN : ShutDownType::POWER_OFF;

	if (lastConnectionTime > 0)
	{
		STRSAFE_LPWSTR query = \
			L"<QueryList>" \
			L"  <Query Id='0' Path='System'>" \
			L"  <Select Path='System'>*[System[Provider[@Name='User32'] and (EventID=1074) and TimeCreated[timediff(@SystemTime) &lt;= %lld]]]</Select>"\
			L"  </Query>" \
			L"</QueryList>";

		wchar_t q[2000] = { NULL };
		auto now = std::chrono::system_clock::now();
		auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
		auto value = now_ms.time_since_epoch();
		INT64 now_duration = value.count();
		INT64 timeDifference = now_duration - lastConnectionTime;
		StringCbPrintfW(q, 2000, query, timeDifference);

		LPCWSTR publisher = L"Microsoft-Windows-TerminalServices-LocalSessionManager";
		EVT_HANDLE eventResults = nullptr;
		eventResults = EvtQuery(nullptr, // local computer
			nullptr,
			q,
			EvtQueryChannelPath | EvtQueryTolerateQueryErrors);

		EVT_HANDLE publisherMetaData = EvtOpenPublisherMetadata(NULL, publisher, NULL, 0, 0);

		if (eventResults)
		{
			DWORD nextSize = 100; // Any number but same as the buffer size
			EVT_HANDLE nextEvents[100];
			DWORD returnedNext;

			int offset = 0;
			bool done = false;

			while (EvtSeek(eventResults, offset, nullptr, 0, EvtSeekRelativeToLast) && EvtNext(eventResults, nextSize, nextEvents, INFINITE, 0, &returnedNext) && !done)
			{
				for (DWORD i = 0; i < returnedNext; i++)
				{
					wstring message = getStringFromEvent(publisherMetaData, nextEvents[i], EvtFormatMessageXml);
					if (message != L"")
					{

						ticpp::Document* xmlObj = new ticpp::Document();

						size_t origsize = wcslen(message.c_str()) + 1;
						const size_t newsize = _MAX_PATH + 2000;
						size_t convertedChars = 0;
						char nstring[newsize];
						wcstombs_s(&convertedChars, nstring, origsize, message.c_str(), _TRUNCATE);

						string msg(nstring);
						xmlObj->Parse(msg);

						ticpp::Element* configObj = xmlObj->FirstChildElement(false);

						ticpp::Node* child = configObj->FirstChild("EventData");

						ticpp::Iterator< ticpp::Element > child2(child->FirstChildElement(), "Data");
						for (child2 = child2.begin(child); child2 != child2.end(); child2++)
						{
							string str;
							child2->GetAttribute("Name", &str);

							if (str == "param5")
							{
								string shutDownTypeStr = child2->GetText();
								shutDownType = shutDownTypeStr == "restart" ? ShutDownType::RESTART : ShutDownType::POWER_OFF;
								if (shutDownType == ShutDownType::POWER_OFF)
								{
									done = true;
									break;
								}
							}
						}
					}
					else
					{
						done = true;
						break;
					}
				}
				if (done == true)
				{
					break;
				}
				++offset;
			}

			EvtClose(eventResults);
		}

	}

	return shutDownType;
}

THORDAQ_STATUS CThordaq::SetDefaultDIOConfig()
{
	// configure minimum Global Registers needed to run
	// route the Digital (DBB1) I/O signals
	// MATCH ThorImageLS with DIO1 12   digit_output_2 (software)               IDevice.TD_DIO_MUXedSLAVE_PORTS.Digital_Output_2
	//                        DIO2 13   digit_output_3 (software)               TD_DIO_MUXedSLAVE_PORTS .Digital_Output_3
	//                        DIO3 9    pixel clock pulse output                TD_DIO_MUXedSLAVE_PORTS .Pixel_clock_pulse_output
	//                        DIO4 10   digit_output_0 (software)               TD_DIO_MUXedSLAVE_PORTS .Digital_Output_0
	//                        DIO5 0    resonant scanner line trigger input     TD_DIO_MUXedSLAVE_PORTS .Resonant_scanner_line_trigger_input
	//                        DIO6 7    external hardware frame trigger input   TD_DIO_MUXedSLAVE_PORTS .Hardware_trigger_input
	//                        DIO7 3    scan direction                          TD_DIO_MUXedSLAVE_PORTS .Start_of_frame_output [replace "3" for ManufTest]
	//                        DIO8 11   digit_output_1 (software)               IDevice.TD_DIO_MUXedSLAVE_PORTS .Digital_Output_1

	// read the current BOB config table and setup BOB hardware according to Legacy or 3U

	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;
	vector<string> strDIOlist = vector<string>();

	char* UnManagedConfigBuffer = new char[TD_BOBDIODef::NumBOB_DIOs * TD_BOBDIODef::CharPerBOB_DIO];

	char* DIOconfig = new char[TD_BOBDIODef::CharPerBOB_DIO];

	status = APIGetDIOConfig(*this, UnManagedConfigBuffer, ((int)TD_BOBDIODef::NumBOB_DIOs * (int)TD_BOBDIODef::CharPerBOB_DIO)); // DIOconfig is DLL char (e.g. [32][6] array) - send our size

	if (status == THORDAQ_STATUS::STATUS_SUCCESSFUL)
	{
		int i, j;
		for (i = 0; i < (int)TD_BOBDIODef::NumBOB_DIOs; i++)
		{
			for (j = 0; j < (int)TD_BOBDIODef::CharPerBOB_DIO; j++)
			{
				DIOconfig[j] = UnManagedConfigBuffer[i * TD_BOBDIODef::CharPerBOB_DIO + j];
			}
			// convert the array into strings for easier display
			strDIOlist.push_back(string(DIOconfig));
		}
	}

	// we now have the entire CURRENT DIO config definition - write the entire configuration back,
	// one DIO at a time

	BYTE* bytes;
	int iDIOsConfigured = 0;
	int k;
	for (k = 15 /*(int)TD_BOBDIODef.NumBOB_DIOs-1*/; k >= 0; k--) // for each of the 32 3U DIOs (or 8 for Legacy DIOs)
	{
		bytes = new BYTE[strDIOlist[k].length()];
		std::memcpy(bytes, strDIOlist[k].data(), strDIOlist[k].length());

		for (int Idx = 0; Idx < strDIOlist[k].length(); ++Idx)
		{
			UnManagedConfigBuffer[Idx] = strDIOlist[k][Idx];
		}
		status = APISetDIOConfig(*this, UnManagedConfigBuffer, (int)TD_BOBDIODef::CharPerBOB_DIO); // DIOconfig is DLL char (e.g. [32][] array) - send our size
		if (status == THORDAQ_STATUS::STATUS_SUCCESSFUL)
		{
			++iDIOsConfigured;
		}
		Sleep(1);
	}
	status = (iDIOsConfigured >= 8) ? THORDAQ_STATUS::STATUS_SUCCESSFUL : THORDAQ_STATUS::STATUS_I2C_INVALIDMUX;
	return status;
}

THORDAQ_STATUS CThordaq::GetBOBType(THORDAQ_BOB_TYPE& bobType)
{
	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;
	DiscoverBOBtype(*this);

	if ('P' == BOB_HardwareType)
	{
		bobType = THORDAQ_BOB_TYPE::TDQ_3U_BOB;
	}
	else
	{
		bobType = THORDAQ_BOB_TYPE::TDQ_LEGACY_BOB;
	}

	return status;
}
/*THORDAQ_STATUS CThordaq::SetDownsampleRate(bool enableDownsampleChange, USHORT downSampleRate)
{
	_enableDownsamplingRateChange = enableDownsampleChange;
	_threePhotonDownsamplingRate = downSampleRate;
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS CThorDAQ::GetDownSampleRate(USHORT downSampleRate)
{

}*/