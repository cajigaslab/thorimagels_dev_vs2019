#include "stdafx.h"
#include "ThordaqSimulator.h"

//THORDAQ_STATUS CdFLIM_4002:: ConnectToBoard()
//{
//
//}

//ifstream infile;
  //infile.open("hello.dat", ios::binary | ios::in);

/*
ThordaqSimulator::ThordaqSimulator()
{
	

}*/

ThordaqSimulator::ThordaqSimulator(UINT boardNum, HANDLE PCIDevHandle)
{
	ifstream infile;

	infile.open("fullFrameSim.bin", ios::binary | ios::in);

	_loadedFrame = new char[8192 * 256];

	_frameHSize = 8192;
	_frameImageWidth = 256;
	_frameImageHeight = 256;

	infile.read(_loadedFrame, 8192 * 256);
	
}
	

//Connect to the device.
THORDAQ_STATUS ThordaqSimulator::ConnectToBoard(
)
{
	return STATUS_SUCCESSFUL;
}

//Disconnect to the device.
THORDAQ_STATUS ThordaqSimulator::DisconnectFromBoard(
)
{
	return STATUS_SUCCESSFUL;
}

//Set/Get the register value of the device.
THORDAQ_STATUS ThordaqSimulator::WriteReadRegister ( UINT read_write_flag, UINT register_bar_num, ULONGLONG register_card_offset, BYTE* buffer, ULONGLONG offset,  ULONGLONG length,  PSTAT_STRUCT completed_status)
{
	return STATUS_SUCCESSFUL;
}


THORDAQ_STATUS ThordaqSimulator::GetBoardCfg(
	BOARD_INFO_STRUCT*	    board_info  // Returned structure
)
{
	return STATUS_SUCCESSFUL;
}

// Read data from particular channel address predefined in the FPGA 
THORDAQ_STATUS ThordaqSimulator::PacketReadChannel(
	ULONG   Channel,           // Channel Index to be read from
	ULONG*  buffer_length,            // Size of the Packet Recieve Data Buffer requested (FIFO Mode) 
	void*   Buffer,            // Data buffer (Packet Mode) 
	double   Timeout_ms            // Generate Timeout error when timeout
)
{
	ULONG size = _frameHSize * _frameImageVSize;
	if (*buffer_length != size)
	{
		memcpy(Buffer, Buffer, size);
	
		return STATUS_SUCCESSFUL;
	}
	else
	{
		return STATUS_READ_BUFFER_ERROR;
	}
}

// Abort reading 
THORDAQ_STATUS ThordaqSimulator::AbortPacketRead(
)
{
	return STATUS_SUCCESSFUL;
}

//Read data from particular data start address
THORDAQ_STATUS ThordaqSimulator::PacketReadBuffer(
	ULONG64  Address,           // Start address to read in the card
	ULONG*   Length,            // Size of the Packet Recieve Data Buffer requested (FIFO Mode)
	void*    Buffer,            // Data buffer (Packet Mode)
	ULONG    Timeout            // Generate Timeout error when timeout
)
{
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqSimulator::PacketWriteBuffer(
	ULONG64 register_card_offset,           // Start address to read in the card
	ULONG   Length,            // Size of the Packet Write to Data Buffer Count by Byte (FIFO Mode)
	UCHAR*  Buffer,            // Data buffer (Packet Mode)
	ULONG   Timeout            // Generate Timeout error when timeout
)
{
	return STATUS_SUCCESSFUL;
}

//Set up data imaging configuration
THORDAQ_STATUS ThordaqSimulator::SetImagingConfiguration(
	IMAGING_CONFIGURATION_STRUCT	imaging_config // Image configuration stuct
	)
{
	return STATUS_SUCCESSFUL;
}

/******************************************
* ThorDAQ DAC Control Functions
******************************************/
THORDAQ_STATUS ThordaqSimulator::SetDACParkValue(
	ULONG32 outputChannel, 
	double outputValue
)
{
	return STATUS_SUCCESSFUL;
}

LONG ThordaqSimulator::SetDACChannelMapping()
{
	return STATUS_SUCCESSFUL;
}

/******************************************
* ThorDAQ Control Functions
******************************************/
THORDAQ_STATUS ThordaqSimulator::SetADCChannelMapping()
{
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqSimulator::SetADCGain(UINT8 channel, ULONG gain)
{
	return STATUS_SUCCESSFUL;
}

/******************************************
* ThorDAQ Trouble Shooting Functions
******************************************/
THORDAQ_STATUS ThordaqSimulator::DoMemoryTest()
{
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqSimulator::GetFirmwareVersion(UINT8& major, UINT8& minor, UINT8& revision)
{
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqSimulator::GetDriverVersion(UINT8& major, UINT8& minor, UINT8& revision)
{
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqSimulator::GetCPLDVersion(UINT8& major, UINT8& minor, UINT8& revision)
{
	return STATUS_SUCCESSFUL;
}

/******************************************
* ThorDAQ IO Status Read Functions
******************************************/
THORDAQ_STATUS ThordaqSimulator::GetLineTriggerFrequency(UINT32 sample_rate,double& frequency)
{
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqSimulator::GetTotalFrameCount(UINT32& frame_count)
{
	return STATUS_SUCCESSFUL;
}
	
THORDAQ_STATUS ThordaqSimulator::GetDACSamplesPerLine(UINT32& samples,double& dac_rate, double line_time)
{
	return STATUS_SUCCESSFUL;
}

// Individual Parameter API Functions
THORDAQ_STATUS ThordaqSimulator::SetParameter( UINT32 parameter,long value)
{
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqSimulator::GetParameter( UINT32 parameter,long& retValue)
{
	return STATUS_SUCCESSFUL;
}

/******************************************
* ThorDAQ DMA Functions
******************************************/
THORDAQ_STATUS  ThordaqSimulator::WaitForBufferReady(double time_ms)
{
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS  ThordaqSimulator::ReadBuffer(UINT8 channel, UINT32*  buffer_length, void* buffer, double timeout_ms)
{
	return STATUS_SUCCESSFUL;
}
	
// Capture API Functions
//Start acquisition
THORDAQ_STATUS ThordaqSimulator::StartAcquisition()
{
	return STATUS_SUCCESSFUL;
}

//Stop acquisition
THORDAQ_STATUS ThordaqSimulator::StopAcquisition()
{
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqSimulator::SetADCSettings(ULONG clock_source, ULONG32 adc_sample_rate, UINT8 set_flag)
{
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqSimulator::MeasureExternClockRate(ULONG32& clock_rate, ULONG32& clock_ref, ULONG32 mode)
{
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqSimulator::GetExternClockStatus(ULONG32& isClockedSynced)
{
	return STATUS_SUCCESSFUL;
}

//Set up FPGA DMA Packet Mode to addressable
THORDAQ_STATUS ThordaqSimulator::SetPacketModeAddressable(bool stream_to_mem_dma_enable)
{
	return STATUS_SUCCESSFUL;
}												