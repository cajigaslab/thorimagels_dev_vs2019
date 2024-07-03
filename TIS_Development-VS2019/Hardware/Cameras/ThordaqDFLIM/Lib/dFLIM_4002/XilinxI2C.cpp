#include "stdafx.h"
#include "XilinxI2C.h"


 

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
bool AIXI2C_pollCoreSTATUSregisterForState(CdFLIM_4002& TdBrd, XilinxI2C* XI2C, ULONG Timeout, UINT64 BitMask, bool bState) // bit state we poll for
{
	UINT64 StatusReg;
	volatile ULONG ulTimeout = Timeout; // poll loop timeout
	do
	{
		XI2C->ReadI2CAIXBridge(TdBrd, XilinxI2C::Status_Register, &StatusReg);
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
	CdFLIM_4002& TdBrd, // This CdFLIM_4002 object by reference	
	XilinxI2C* XI2C,
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
	bool bBusNotBusy = AIXI2C_pollCoreSTATUSregisterForState(TdBrd, XI2C, ulTimeout, XilinxI2C::BB, false);
	if (bBusNotBusy == false)
	{
		return STATUS_I2C_TIMEOUT_ERR;
	}

	XI2C->WriteI2CAIXBridge(TdBrd, XilinxI2C::TX_FIFO_Reg, AXIbridgeValue); // this (S)tarts I2C bus
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
		XI2C->WriteI2CAIXBridge(TdBrd, XilinxI2C::TX_FIFO_Reg, AXIbridgeValue);

		// if the I2C Slave Address and byte(s) were flushed to hardware, the implication 
		// is device exists and data is successfully sent - this condition is reflected by "empty" status
		// POLL for empty status is only high performance option - if we WaitForSingleObject()
		// it will incur millisecs delay when we ask for nanoseconds
		volatile ULONG ulTimeout = 0x400; // poll loop timeout - NOTE this varies widely on WRITE to particular hardware device
		bool bTX_FIFOEmpty;
		bTX_FIFOEmpty = AIXI2C_pollCoreSTATUSregisterForState(TdBrd, XI2C, ulTimeout, XilinxI2C::TX_FIFO_Empty, true);
		// WHY IS THIS TX_FIFO_Empty bit not SET when byte fails in dFLIM??????
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
			XI2C->WriteI2CAIXBridge(TdBrd, XilinxI2C::TX_FIFO_Reg, AXIbridgeValue);
		}
		// write the expected RECEIVE byte count from Slave
		AXIbridgeValue = 0x200 | (*ReceiveBufferLen); // byte count we expect from slave, with automatic Master NAK when count satisfied
		XI2C->WriteI2CAIXBridge(TdBrd, XilinxI2C::TX_FIFO_Reg, AXIbridgeValue);

		// RECEIVE bytes from slave....
		// For each byte, poll for RX_FIFO_Empty Status Register bit being CLEAR (it is set after SOFTR)
		// with timeout
		UINT32 incomingByteIndx;
		ulTimeout = 0x1500;
		bool ByteReady;
		for (incomingByteIndx = 0; incomingByteIndx < *ReceiveBufferLen; incomingByteIndx++)
		{
			ByteReady = AIXI2C_pollCoreSTATUSregisterForState(TdBrd, XI2C, ulTimeout, XilinxI2C::RX_FIFO_Empty, false);
			if (ByteReady)
			{
				// Get byte from RX_FIFO
				XI2C->ReadI2CAIXBridge(TdBrd, XilinxI2C::RX_FIFO_Reg, &AXIbridgeValue);
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
THORDAQ_STATUS CdFLIM_4002::APIXI2CReadWrite(
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
)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	volatile ULONG ulTimeout = 0x01000; // poll loop timeout
	bool bBusNotBusy;

	XilinxI2C* XI2C;  // the Xilinx I2C controller through AIX bridge
	bool b400Hz;
	if (I2CbusHz == 400) {
		b400Hz = true;
	}
	else {
		b400Hz = false;

	}

	FPGAregisterWRITE("GIGCR0_LED1", 1); // turn ON at entry, OFF at exit (detect hang at BAR3, 0x300 AXI bridge access)

	//	bool b400Hz = (I2CbusHz == 400) ? true : false;  // only two speeds for now - LFT/MCU needs 100 kHz, other slaves 400 kHz

	XI2C = new XilinxI2C(TdBrd, b400Hz);  // constructor may reset device.... ?
	//	UINT64 StatusReg, ControlReg ;
	UINT32 I2CbytesThisTransmit = 0;
	UINT32 ExpectedTotalTransferLen = *OpCodeLen + *BiDirectionalDataLen; // PageWrites modify - could transfer less (or 0) on error
	UINT32 I2CtotalBytesWritten = 0; // accumulator for OpCodeBuff/TxBuff bytes written
	bool bAddStopBit = false;
	bool bI2CslaveAddrReadDirection = false;
	UINT8 SetupOpCodeBuf[4]; // for local use!

	// TEST - what are default settings by Xilinx core?  
	// Use registers to change speed from 100 kHz (def.) to 400 kHz
	//  UINT64 THDSTA, TSUSTA, TSUDAT, TBUF, THIGH, TLOW, THDDAT;
	/*
	XI2C->ReadI2CAIXBridge(TdBrd, XilinxI2C::THDSTA_Register, &THDSTA);  // units: 5ns "ticks" count
	XI2C->ReadI2CAIXBridge(TdBrd, XilinxI2C::TSUSTA_Register, &TSUSTA);
	XI2C->ReadI2CAIXBridge(TdBrd, XilinxI2C::TSUDAT_Register, &TSUDAT);
	XI2C->ReadI2CAIXBridge(TdBrd, XilinxI2C::TBUF_Register, &TBUF);
	XI2C->ReadI2CAIXBridge(TdBrd, XilinxI2C::THIGH_Register, &THIGH);
	XI2C->ReadI2CAIXBridge(TdBrd, XilinxI2C::TLOW_Register, &TLOW);
	XI2C->ReadI2CAIXBridge(TdBrd, XilinxI2C::THDDAT_Register, &THDDAT);
	*/

	// for "Dynamic" operation...
	// set max RX_FIFO depth 
	XI2C->WriteI2CAIXBridge(TdBrd, XilinxI2C::RX_FIFO_PIRQ_Register, 0x8); // MAX RX FIFO depth is 0xF (16)

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

	ulTimeout = 0x01000; // poll loop timeout
	bBusNotBusy = AIXI2C_pollCoreSTATUSregisterForState(TdBrd, XI2C, ulTimeout, XilinxI2C::BB, false);

I2C_Completed:
	XI2C->WriteI2CAIXBridge(TdBrd, XilinxI2C::Control_Register, 0); // DISable 
	delete(XI2C);
	*BiDirectionalDataLen = I2CtotalBytesWritten;
	FPGAregisterWRITE("GIGCR0_LED1", 0); // turn ON at entry, OFF at exit (detect hang at BAR3, 0x300 AXI bridge access)
	return status;
}


