#pragma once
#include "dFLIM_4002.h"
#include "thordaqcmd.h"
#include "thordaqres.h"
#include "memorypool.h"
#include "thordaqconst.h"
#include "filter.h"

class ThordaqSimulator : public CdFLIM_4002
{
	public:
	// Constructor
		ThordaqSimulator();
	ThordaqSimulator(UINT boardNum, HANDLE PCIDevHandle);
	

	//Connect to the device.
	THORDAQ_STATUS ConnectToBoard(
	);

	//Disconnect to the device.
	THORDAQ_STATUS DisconnectFromBoard(
	);

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
	THORDAQ_STATUS SetImagingConfiguration(
		IMAGING_CONFIGURATION_STRUCT	imaging_config // Image configuration stuct
	 );

	/******************************************
	* ThorDAQ DAC Control Functions
	******************************************/
	THORDAQ_STATUS SetDACParkValue(
		ULONG32 outputChannel, 
		double outputValue
	);

	LONG SetDACChannelMapping();
	/******************************************
	* ThorDAQ Control Functions
	******************************************/
	THORDAQ_STATUS SetADCChannelMapping();
	THORDAQ_STATUS SetADCGain(UINT8 channel, ULONG gain);

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

	THORDAQ_STATUS SetADCSettings(ULONG clock_source, ULONG32 adc_sample_rate = ADC_MAX_SAMPLE_RATE, UINT8 set_flag = TRUE);

	THORDAQ_STATUS MeasureExternClockRate(ULONG32& clock_rate, ULONG32& clock_ref, ULONG32 mode);

	THORDAQ_STATUS GetExternClockStatus(ULONG32& isClockedSynced);

	//Set up FPGA DMA Packet Mode to addressable
	THORDAQ_STATUS SetPacketModeAddressable(bool stream_to_mem_dma_enable);

	private:
		char* _loadedFrame;
		ULONG _frameHSize;
		ULONG _frameImageWidth;
		ULONG _frameImageHeight;
		ULONG _frameImageVSize;
};

