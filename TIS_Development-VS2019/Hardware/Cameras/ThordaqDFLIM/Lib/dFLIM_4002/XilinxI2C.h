#pragma once
#include "dFLIM_4002.h"

/*
Base_Addr = 0x2C0          BAR 3
I2C:  AXI_Lite_Bridge_Register: BAR 3, 0x2C0 , Write Only
--  Bit 31 – 0 : Xilinx I2C IP AXI - Lite Data bus bits
--  Bit 63 – 32 : Xilinx I2C IP AXI - Lite Address bus bits

I2C : AXI - Lite_Bridge_Control Register : BAR 3, 0x2C8, Write Only
--  Bit 0 : AXI - Lite Write
--         A 0-> 1 Transition causes address and data in AXI_Lite_Bridge_Register to be written to the
Xilinx I2C IP core.
--Bit 1 : AXI - Lite Read
--        A 0-> 1 Transition causes the address in AXI_Lite_Bridge_Register to be read and
stored in the AXI_Lite_Bridge_Status register from the Xilinx I2C IP core.
--Bit 2 : I2C Reset
--         Direct connect from bit to external I2C reset.
--Bit 3 : I2C Mode Switch [Requires FPGA Ver D and above]
--       0-> Xilinx I2C Master Enable 
--       1-> Legacy Master Enable (deprecated)

I2C : AXI_Lite_Status_Register: BAR 3, 0x2C0, Read Only
--  Bit 31 - 0 : Data being read back from Xilinx I2C IP core
*/

/**************************************************************************************************
 * @class	XilinxI2C
 *
 * @brief	This class implements new Xilinx "AXI IIC Bus Interface v2.0, LogiCORE IP Product Guide,
 *          PG090, Oct. 5, 2016, through AXI Bridge interface by BRadtke, Dec. 2020
 *
 * @author	DZimmerman
 * @date	1/3/2024 (based on Dec-2020 XI2Ccontroller design)
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


enum I2C_XilinxCmdREG
{
	I2C_AXIwrite,             // bit 0
	I2C_AXIread,
	TI9548AChipReset_,        // bit 2 (asserted LOW)
	I2C_GYdFLIMmodeEn       // bit 3 (asserted HI) 
}; 
class XilinxI2C
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
	XilinxI2C(CdFLIM_4002& TdBrd, bool Speed400kHz)    // CONSTRUCTOR: pass reference to ThorDAQ board hardware
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
		WriteI2CAIXBridge(TdBrd, XilinxI2C::Control_Register, XilinxI2C::TX_FIFO_Rst | XilinxI2C::EN);
		WriteI2CAIXBridge(TdBrd, XilinxI2C::Control_Register, XilinxI2C::EN); // ENable (other bits cleared)

		// boost TBUF time for Atmel MCU slave
		WriteI2CAIXBridge(TdBrd, XilinxI2C::TBUF_Register, 50000);   // e.g. 50,000 ticks * 5ns/tick = 250 usec

		// 400 kHz settings (all I2C register unit 5ns "ticks)
		if (Speed400kHz) {

			UINT64 TSUSTA = 200;
			WriteI2CAIXBridge(TdBrd, XilinxI2C::TSUSTA_Register, TSUSTA);

			UINT64 THDSTA = 200;  // e.g. 200(ticks) * 5(ns/tick) = 1.0 usecs
			WriteI2CAIXBridge(TdBrd, XilinxI2C::THDSTA_Register, THDSTA);

			UINT64 TSUDAT = 58; // (58 * 5ns/tick = 290 ns) Xilinx includes "Tf", fall time
			WriteI2CAIXBridge(TdBrd, XilinxI2C::TSUDAT_Register, TSUDAT);
			UINT64 THDDAT = 120; // (60 * 5ns/tick = 300)
			WriteI2CAIXBridge(TdBrd, XilinxI2C::THDDAT_Register, THDDAT);

			UINT64 TBUF = 280; // 
			WriteI2CAIXBridge(TdBrd, XilinxI2C::TBUF_Register, TBUF);

			UINT64 TLOW = 850; // see Xlinix calculation, RegValue = ((200,000,000 / 2 / Fscl) - 7 - SCL_InertialDelay (243 for 400 kHz)
			WriteI2CAIXBridge(TdBrd, XilinxI2C::TLOW_Register, TLOW);
			UINT64 THIGH = 425; // 
			WriteI2CAIXBridge(TdBrd, XilinxI2C::THIGH_Register, THIGH);

		}

	}
	// Functions to read and write I2C Core Registers through ThorDAQ's FPGA bridge...
	// I2C registers are 32 bits (address and data) - FPGA bridge registers are 64 bits
	void WriteI2CAIXBridge(CdFLIM_4002& TdBrd, UINT64 CoreRegAddress, UINT64 Data)  // WriteI2C Register through AIX bridge 
	{
		// first setup up the Address/Data in Bridge Register
		UINT64 BridgeAddrDataValue = (CoreRegAddress << (UINT64)32);
		BridgeAddrDataValue |= Data;

		TdBrd.FPGAregisterWrite(_I2C_AXIBridge, BridgeAddrDataValue);
		TdBrd.FPGAregisterWrite(_I2C_AXIControl, _TI9548A_RESET | 0x1);  // WRITE command (OR in the 9548 RESET de-assert)
		TdBrd.FPGAregisterWrite(_I2C_AXIControl, _TI9548A_RESET);  // (must Toggle)
	}
	void ReadI2CAIXBridge(CdFLIM_4002& TdBrd, UINT64 CoreRegAddress, UINT64* Data)  // WriteI2C Register through AIX bridge
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
	void ResetTI9548MUX(CdFLIM_4002& TdBrd)
	{
		TdBrd.FPGAregisterWrite(_I2C_AXIControl, 0); // reset is asserted low
		TdBrd.FPGAregisterWrite(_I2C_AXIControl, _TI9548A_RESET); // bit 2 - set to release from RESET
	}

	~XilinxI2C()
	{
		// Release the MUTEX for next use...
		if (_hCopyOfXI2Cmutex != NULL)
			ReleaseMutex(_hCopyOfXI2Cmutex);		// handle to mutex (from CThordaq board class)
	}
};

