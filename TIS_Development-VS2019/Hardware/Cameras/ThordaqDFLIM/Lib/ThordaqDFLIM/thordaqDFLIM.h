#include "stdafx.h"
#include "thordaqcmd.h"
#include "thordaqres.h"
#include "memorypool.h"
#include "thordaqconst.h"
#include <atomic>

#define MAXIMUM_NUMBER_OF_BOARDS 4 //

//
// Copied Macros from ntddk.h. Used to using the kernel-mode
// style of linked list.
//


inline double round(double num) {
    return (num > 0.0) ? floor(num + 0.5) : ceil(num - 0.5);
}

class DMADescriptor
{
public:
	ULONG64 next_node;
	ULONG64 length;
	ULONG64 buf_addr;
};

// This class is exported from the thordaq.dll
class ThordaqDFLIM {
public:
	// Constructor
	ThordaqDFLIM(UINT boardNum);
	// Destructor
	~ThordaqDFLIM();

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

	THORDAQ_STATUS ReSync();

	THORDAQ_STATUS SetCoarseShift(ULONG32 shift, int channel);

	THORDAQ_STATUS SetFineShift(LONG32 shift, int channel);

	THORDAQ_STATUS GetClockFrequency(int clockIndex, double& frequency);

	THORDAQ_STATUS SetDFLIMSyncingSettings(ULONG32 syncDelay, ULONG32 resyncDelay, bool forceResyncEverLine);

	THORDAQ_STATUS SetDLIMFrontEndSettings();
	
private:
	HANDLE								gHdlDevice;
	HDEVINFO							gDeviceInfo;				 // The handle to a device information set that contains all installed devices that matched the supplied parameters
	UINT								gBoardIndex;				 // The index number to record board number
	PSP_DEVICE_INTERFACE_DETAIL_DATA	gDeviceInterfaceDetailData;  // The structure to receice information about device specified interface
	DMA_INFO_STRUCT						gDmaInfo;					 // The structure to retrieve the board dma configuration
	DATA_ACQ_CTRL_STRUCT*               gPtrAcqCtrl;                   // The Global Pointer to Thordaq acquisition configuration struct
	ULONG64								_dacDescpListIndex;
	atomic<bool>						abortPacketRead;

	//Set up FPGA DMA Packet Mode
	THORDAQ_STATUS SetPacketMode(
		int		dma_engine_offset,		// DMA Engine number offset to use
		bool stream_to_mem_dma_enable,
		ULONG *	packet_max_size,		// Length of largest packet (FIFO Mode)
		int		packet_alloc_mode,			// Sets mode, FIFO or Addressable Packet mode
		int		num_of_descriptors	// Number of DMA Descriptors to allocate (Addressable mode)	
	);

	THORDAQ_STATUS SetImagingSettings();
	THORDAQ_STATUS StartImaging();
	THORDAQ_STATUS SetI2C_0x2C0(ULONG bytes);
	THORDAQ_STATUS SetI2C_0x2C4(ULONG bytes);
	THORDAQ_STATUS SetI2C_0x2C8(BYTE byte);
	THORDAQ_STATUS SetI2C_0x2C8_232();

	THORDAQ_STATUS ThordaqDFLIM::ResetBackEndAcq();  //gy

	THORDAQ_STATUS ReadI2C_0x2C0(ULONG length);

	THORDAQ_STATUS SetNONI2C_0x1C0(ULONG bytes);
	THORDAQ_STATUS SetNONI2C_0x1C4(ULONG bytes);
	THORDAQ_STATUS SetNONI2C_0x1C8(BYTE byte);

	THORDAQ_STATUS ReadNONI2C_0x1C0(ULONG length);

	//Get DMA Engine Capabilitie
	THORDAQ_STATUS GetDMAEngineCap(
		ULONG			dma_engine_offset,	// DMA Engine number to use
		PDMA_CAP_STRUCT dma_capability		// Returned DMA Engine Capabilitie
	);

	//Sends two PACKET_BUF_DEALLOC_IOCTL calls to the driver to teardown the recieve buffer and teardown the descriptors for sending packets
	THORDAQ_STATUS ReleasePacketBuffers(
		int				dma_engine_offset // DMA Engine number to use
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
	
	LONG SetGlobalSettings(IMAGING_CONFIGURATION_STRUCT	imaging_config);

	LONG SetScanSettings(IMAGING_CONFIGURATION_STRUCT	imaging_config);

	LONG SetCoherentSampleingSettings(IMAGING_CONFIGURATION_STRUCT	imaging_config);
	
	LONG SetStreamProcessingSettings(IMAGING_CONFIGURATION_STRUCT	imaging_config);

	LONG SetDACSettings(
		IMAGING_CONFIGURATION_STRUCT	imaging_config
	);

	

	LONG SetWaveformPlayback(
		DAC_CRTL_STRUCT dac_setting,
		int              channel,
		UINT8 set_flag = TRUE
		);


	LONG LoadDACDescriptors(DAC_CRTL_STRUCT* dac_settings);

	void SetDAC_DMADescriptors(DMADescriptor& dmaDescp, ULONG64 index, ULONG64* dmaDescpTable, bool& status);

	LONG ExportScript(DATA_ACQ_CTRL_STRUCT*               gPtrAcqCtrl, SCAN_LUT scanLUT);

	void LogMessage(wchar_t* logMsg, long eventLevel);
};

