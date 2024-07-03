// -------------------------------------------------------------------------
// 
// PRODUCT:			ThorDAQ Driver
// MODULE NAME:		ThorDAQres.h
// 
// MODULE DESCRIPTION: 
// 
// Contains the Driver IOCtl defines, typedefs, structures and function prototypes.
// 
// $Revision:  $
//
// ------------------------- CONFIDENTIAL ----------------------------------
// 
//              Copyright (c) 2016 by ThorLabs Imaging Research Group, LLC.   
//                       All rights reserved. 
// 
// -------------------------------------------------------------------------

//
// This header defines all the User defined IOCTL codes and data structures for 
// the PCI Driver.  The data structures sent to/from the driver are also defined.
// (Any user defined types that are shared by the driver and app must be defined in this
// function or by the system)
//
// Define control codes for DMADriver
//
// IOCTL  Description             	Data to Driver       	Data from Driver
// -----  -----------             	-----------------     	---------------------
//  800   Get Board Config        	None                  	BOARD_CONFIG_STRUCT
//  802   Memory Read             	DO_MEM_STRUCT      		data
//  803   Memory Write            	DO_MEM_STRUCT      		data
//  806   Get DMA Engine Cap      	EngineNum (ULONG)  		DMA_CAP_STRUCT
//  807   Acquisition Config Ctrl   IMG_ACQ_CONF_IOCTL	    none
//  808   Get DMA Performance     	EngineNum (ULONG)  		DMA_STAT_STRUCT
//
//   	Common Packet Mode APIs
//  820   Packet DMA Buf Alloc    	BUF_ALLOC_STRUCT   		RET_BUF_ALLOC_STRUCT
//  821   Packet DMA Buf Release  	BUF_DEALLOC_STRUCT   	None
// 
// 		FIFO Packet Mode APIs
//  822   Packet Receive			PACKET_RECEIVE_STRUCT  	PACKET_RET_RECEIVE
//  824   Packet Send				PACKET_SEND_STRUCT		data
//  826   Packet Receives			PACKET_RECEIVES_STRUCT 	PACKET_RECEIVES_STRUCT
//
//		Addressable Packet Mode APIs
//  830   Packet Read				PACKET_READ_STRUCT  	data
//  831   Packet Write				PACKET_WRITE_STRUCT		data

#ifndef __THORDAQRES__h__
#define __THORDAQRES__h__
#include <windows.h>   // for using Windows data types and functions
#include "Setupapi.h"  // for using SetupDi functions

#define MAX_BARS                        6       // TYPE-0 configuration header values.
#define USED_MAX_BARS                   3
#define USED_MAX_CARD_OFFSET            0x20000
#define PCI_STD_CONFIG_HEADER_SIZE      64      // 64 Bytes is the standard header size
#define PCI_DEVICE_SPECIFIC_SIZE		192    // Device specific bytes [64..256].

// Board Config Struc defines
#define CARD_IRQ_MSI_ENABLED                0x0008
#define CARD_IRQ_MSIX_ENABLED               0x0040
#define CARD_MAX_PAYLOAD_SIZE_MASK          0x0700
#define CARD_MAX_READ_REQUEST_SIZE_MASK     0x7000

// BarCfg defines
#define BAR_CFG_BAR_SIZE_OFFSET     	    (24)
#define BAR_CFG_BAR_PRESENT				    0x0001
#define BAR_CFG_BAR_TYPE_MEMORY			    0x0000
#define BAR_CFG_BAR_TYPE_IO		 		    0x0002
#define BAR_CFG_MEMORY_PREFETCHABLE		    0x0004
#define BAR_CFG_MEMORY_64BIT_CAPABLE	    0x0008

// ExpRomCfg defines
#define EXP_ROM_BAR_SIZE_OFFSET	            (24)
#define EXP_ROM_PRESENT			            0x0001
#define EXP_ROM_ENABLED			            0x0002

// read_write_flag defines
#define WRITE_TO_CARD                       0x0             // S2C
#define READ_FROM_CARD                      0x1             // C2S

// Return DMA status defines
#define STATUS_DMA_SUCCESSFUL				0x00
#define STATUS_DMA_INCOMPLETE				0x01
#define STATUS_DMA_INVALID_ENGINE			0x02
#define STATUS_DMA_INVALID_ENGINE_DIRECTION	0x04
#define STATUS_DMA_BUFFER_MAP_ERROR			0x08
#define STATUS_DMA_ENGINE_TIMEOUT			0x10
#define STATUS_DMA_ABORT_DESC_ALIGN_ERROR	0x20
#define STATUS_DMA_ABORT_BY_DRIVER			0x40
#define STATUS_DMA_ABORT_BY_HARDWARE		0x80

#ifndef MAX_NUM_DMA_ENGINES
#define MAX_NUM_DMA_ENGINES		            64
#endif // MAX_DMA_ENGINES

// DMA Engine Capability Defines
#define DMA_CAP_ENGINE_NOT_PRESENT			0x00000000
#define DMA_CAP_ENGINE_PRESENT				0x00000001
#define DMA_CAP_DIRECTION_MASK              0x00000006
#define DMA_CAP_SYSTEM_TO_CARD				0x00000000
#define DMA_CAP_CARD_TO_SYSTEM				0x00000002
#define DMA_CAP_BIDIRECTIONAL				0x00000006
#define DMA_CAP_ENGINE_TYPE_MASK            0x00000030
#define DMA_CAP_BLOCK_DMA                   0x00000000
#define DMA_CAP_PACKET_DMA                  0x00000010
#define DMA_CAP_FIFO_PACKET_DMA             0x00000010		// New definition of "Packet" mode
#define DMA_CAP_ADDRESSABLE_PACKET_DMA      0x00000020
#define DMA_CAP_ENGINE_NUMBER_MASK          0x0000FF00
#define	DMA_CAP_CARD_ADDRESS_SIZE_MASK		0x007F0000
#define DMA_CAP_MAX_DESCRIPTOR_BYTE_MASK	0x3F000000
#define DMA_CAP_STATS_SCALING_FACTOR_MASK  	0xC0000000

#define DMA_CAP_ENGINE_NUMBER_SHIFT_SIZE    8
#define DMA_CAP_ENGINE_NUMBER(x)            (((x) & DMA_CAP_ENGINE_NUMBER_MASK) >> DMA_CAP_ENGINE_NUMBER_SHIFT_SIZE)
#define DMA_CAP_ENGINE_DIRECTION(X)		    ((X)& DMA_CAP_DIRECTION_MASK)
#define DMA_CAP_CARD_ADDR_SIZE_SHIFT		16
#define DMA_CAP_CARD_ADDR_SIZE_MASK			(0xff)
#define DMA_CAP_CARD_ADDR_SIZE(x)			(1ULL << (((x) >> DMA_CAP_CARD_ADDR_SIZE_SHIFT) & DMA_CAP_CARD_ADDR_SIZE_MASK))
#define DMA_CAP_STATS_SCALING_FACTOR_0  	0x00000000
#define DMA_CAP_STATS_SCALING_FACTOR_2  	0x40000000
#define DMA_CAP_STATS_SCALING_FACTOR_4  	0x80000000
#define DMA_CAP_STATS_SCALING_FACTOR_8  	0xC0000000

#define READ_WRITE_MODE_FLAG_FIFO			0x00000001
#define READ_WRITE_MODE_FLAG_ADDRESSED		0x00000000
//*************************************************************
// Packet Mode defines and Structures

#define	PACKET_MODE_FIFO					0x00
#define	PACKET_MODE_ADDRESSABLE				0x02
#define	PACKET_MODE_STREAMING			    0x40
/* DMA_MODE_NOT_SET - DMA Mode not set, uses default DMA Engine behavior */
#define	DMA_MODE_NOT_SET					0xFFFFFFFF
// Engine Status definitions
#define	DMA_OVERRUN_ERROR					0x8000
#define	PACKET_ERROR_MALFORMED				0x4000
// Thorlab packet generator StopRun DWORD
#define PACKET_GEN_STOP						0x00
#define PACKET_GEN_RUN						0x01

#define SCAN_LUT_MAX_LEN					4096
#define DAC_DESCP_MAX_LEN                   4096

#define	S2C_DIRECTION		TRUE
#define C2S_DIRECTION		FALSE

// DAQ DRAM MAPPING
#define ACQ_SINGLE_CHANNEL_BUF_CAP  0x04000000
#define ACQ_MSI_BUF_OFFSET          ACQ_SINGLE_CHANNEL_BUF_CAP * 6 
#define DAC_BUF_OFFSET              ACQ_MSI_BUF_OFFSET *2

#define FRONT_END_SETUP_SLEEP1 2
#define FRONT_END_SETUP_SLEEP2 10
#define FRONT_END_SETUP_SLEEP3 60

#pragma pack(1) 



// STAT_STRUCT
//
// Status Structure - Status Information from an IOCTL transaction
typedef struct _STAT_STRUCT
{
	ULONGLONG   CompletedByteCount; // Number of bytes transfered
} STAT_STRUCT, *PSTAT_STRUCT;

typedef struct _BOARD_INFO_STRUCT
{
	UCHAR		DriverVersionMajor;	// Driver Major Revision number
	UCHAR		DriverVersionMinor;	// Driver Minor Revision number
	UCHAR		DriverVersionSubMinor; // Driver Sub Minor Revision number 
	UCHAR		DriverVersionBuildNumber; // Driver Build number
	ULONG32		UserVersion;		// User Version Number (optional)
}BOARD_INFO_STRUCT;

// DMA_INFO_STRUCT
//
// DMA Info Structure - Contains information on the type and 
//	quantity of DMA Engines
typedef struct _DMA_INFO_STRUCT
{
	char		PacketSendEngineCount;
	char		PacketRecvEngineCount;
	char		PacketSendEngine[MAX_NUM_DMA_ENGINES];
	char		PacketRecvEngine[MAX_NUM_DMA_ENGINES];
	char		IsAddressablePacket;
} DMA_INFO_STRUCT, *PDMA_INFO_STRUCT;

typedef struct _SCAN_LUT
{
	USHORT		ch;
	USHORT		lut[SCAN_LUT_MAX_LEN];
} SCAN_LUT, *PSCAN_LUT;

typedef struct _DAC_DESCP_TABLE
{
	ULONG64		descp[DAC_DESCP_MAX_LEN];
} DAC_DESCP_TABLE, *PDAC_DESCP_TABLE;

typedef struct _PCI_CONFIG_HEADER
{
	USHORT		VendorId;           // VendorID[15:0] 0-1
	USHORT		DeviceId;           // DeviceID[15: 2-3
	USHORT		Command;            // Command[15:0] 4-5
	USHORT		Status;             // Status[15:0] 6-7
	UCHAR		RevisionId;         // RevisionId[7:0] 8
	UCHAR		Interface;          // Interface[7:0] 9
	UCHAR		SubClass;           // SubClass[7:0] 0xA
	UCHAR		BaseClass;          // BaseClass[7:0] 0xB
	UCHAR		CacheLineSize;		// 0xC
	UCHAR		LatencyTimer;		// 0xD
	UCHAR		HeaderType;			// 0xE
	UCHAR		BIST;				// 0xF
	ULONG32		BarCfg[MAX_BARS];   // BAR[5:0] Configuration	0x10, 0x14, 0x18, 0x1C, 0x20, 0x24
	ULONG32		CardBusCISPtr;		// 0x28
	USHORT		SubsystemVendorId;  // SubsystemVendorId[15:0] 0x2C
	USHORT		SubsystemId;        // SubsystemId[15:0] 0x2E
	ULONG32		ExpRomCfg;          // Expansion ROM Configuration 0x30
	UCHAR		CapabilitiesPtr;	// 0x34
	UCHAR		Reserved1[3];       // 0x35
	ULONG32		Reserved2;          // 0x38
	UCHAR		InterruptLine;      // 0x3C
	UCHAR		InterruptPin;       // 0x3D
	UCHAR		MinimumGrant;       // 0x3E
	UCHAR		MaximumLatency;     // 0x3F
	UCHAR		DeviceSpecific[PCI_DEVICE_SPECIFIC_SIZE];
} PCI_CONFIG_HEADER, *PPCI_CONFIG_HEADER;

typedef struct _PCI_DEVICE_SPECIFIC_HEADER
{
	UCHAR		PCIeCapabilityID;
	UCHAR		PCIeNextCapabilityPtr;
	USHORT		PCIeCapability;
	ULONG32		PCIeDeviceCap;      // PCIe Capability: Device Capabilities
	USHORT		PCIeDeviceControl;  // PCIe Capability: Device Control
	USHORT		PCIeDeviceStatus;   // PCIe Capability: Device Status
	ULONG32		PCIeLinkCap;        // PCIe Capability: Link Capabilities
	USHORT		PCIeLinkControl;    // PCIe Capability: Link Control
	USHORT		PCIeLinkStatus;     // PCIe Capability: Link Status
} PCI_DEVICE_SPECIFIC_HEADER, *PPCI_DEVICE_SPECIFIC_HEADER;

typedef struct _PCI_MSI_CAPABILITY  {
	UCHAR		PCIeCapabilityID;
	UCHAR		PCIeNextCapabilityPtr;
	USHORT		MessageControl;
} PCI_MSI_CAPABILITY, *PPCI_MSI_CAPABILITY;

typedef struct _PCI_MSIX_CAPABILITY  {
	UCHAR		PCIeCapabilityID;
	UCHAR		PCIeNextCapabilityPtr;
	USHORT		MessageControl;
	ULONG32		TableOffsetBIR;
	ULONG32		PBAOffsetBIR;
} PCI_MSIX_CAPABILITY, *PPCI_MSIX_CAPABILITY;

typedef struct _BOARD_CONFIG_STRUCT
{
	UCHAR		NumDmaWriteEngines; // Number of S2C DMA Engines
	UCHAR		FirstDmaWriteEngine;// Number of 1st S2C DMA Engine
	UCHAR		NumDmaReadEngines;  // Number of C2S DMA Engines
	UCHAR		FirstDmaReadEngine; // Number of 1st C2S DMA Engine
	UCHAR		NumDmaRWEngines;    // Number of bidirectional DMA Engines
	UCHAR		FirstDmaRWEngine;   // Number of 1st bidirectional DMA Engine
	UCHAR		DriverVersionMajor;	// Driver Major Revision number
	UCHAR		DriverVersionMinor;	// Driver Minor Revision number
	UCHAR		DriverVersionSubMinor; // Driver Sub Minor Revision number 
	UCHAR		DriverVersionBuildNumber; // Driver Build number

	ULONG32		CardStatusInfo;		// BAR 0 + 0x4000 DMA_Common_Control_and_Status
	ULONG32		DMABackEndCoreVersion; // DMA Back End Core version number
	ULONG32		PCIExpressCoreVersion; // PCI Express Core version number
	ULONG32		UserVersion;		// User Version Number (optional)
	// The following structure is placed at the end due to packing issues in Linux
	// Do Not Move or the Get Board Cfg command will fail.
	struct _PCI_CONFIG_HEADER   PciConfig;
} BOARD_CONFIG_STRUCT, *PBOARD_CONFIG_STRUCT;

typedef struct _DMA_STAT_STRUCT
{
	ULONGLONG   CompletedByteCount; // Number of bytes transferred
	ULONGLONG   DriverTime;         // Number of nanoseconds for driver
	ULONGLONG   HardwareTime;       // Number of nanoseconds for hardware
	ULONGLONG   IntsPerSecond;      // Number of interrupts per second
	ULONGLONG   DPCsPerSecond;      // Number of DPCs/Tasklets per second
} DMA_STAT_STRUCT, *PDMA_STAT_STRUCT;

typedef struct _DO_MEM_STRUCT
{
	ULONG		register_bar_num;             // Base Addres Register (BAR) to access
	ULONGLONG	Offset;             // Byte starting offset in application buffer
	ULONGLONG	register_card_offset;         // Byte starting offset in BAR
	ULONGLONG	Length;             // Transaction length in bytes
} DO_MEM_STRUCT, *PDO_MEM_STRUCT;

//typedef struct _DMA_CAP_STRUCT
//{
//	UCHAR	IsPresent	: 1;	// 1 == Engine Present; 0 == Engine not present.
//	UCHAR	Direction	: 2;	// 00 == Write to Card; 01 == Read from Card; 10 == Reserved; 11 == Bidirectional.
//	UCHAR	Rev1		: 1;	// Reserved.
//	UCHAR	Mod			: 2;	// 00 == Block; 01 == FIFO Packet; 10 == Reserved; 11 == addressable packet.
//	UCHAR	Rev2		: 2;	// Reserved.
//	UCHAR	EngNum;				// Unique identifying number for this DMA Engine
//	UCHAR	CardAddrWidth;		// Size in bytes of the Card Address space that this
//								// DMA Engine is connected to == 2^ImplCardAddresWidth.
//								// ImplCardAddresWidth == 0 indicates that the DMA Engine
//								// is connected to a Stream/FIFO and does not implement
//								// addressable Card Memory
//	UCHAR	Rev;				// Reserved
//} DMA_CAP_STRUCT, *PDMA_CAP_STRUCT;

typedef struct _DMA_CAP_STRUCT
{
	ULONG	    dma_capabilityabilities;    // DMA Capabilities
} DMA_CAP_STRUCT, *PDMA_CAP_STRUCT;


typedef struct _RW_PCI_CONFIG_STRUCT
{
	ULONG		Offset;             // Byte starting offset in application buffer
	ULONG		Length;             // Transaction length in bytes
} RW_PCI_CONFIG_STRUCT, *PRW_PCI_CONFIG_STRUCT;

typedef struct _BUF_ALLOC_STRUCT
{
	ULONG32     IsS2mmDmaEnabled;
	ULONG32     EngineNum;  // DMA Engine number to use
	ULONG32	    AllocationMode;     // Allocation type Flags
	ULONG32		MaxPacketSize;  	// Maximum Packet Size (Must be dividable into PoolSize) (FIFO mode)
	ULONG32		Length;				// Length of Application buffer or size to Allocate (FIFO mode)
	ULONG32		NumberDescriptors;	// Number of C2S Descriptors to allocate (Addressable)
} BUF_ALLOC_STRUCT, *PBUF_ALLOC_STRUCT;

typedef struct _RET_BUF_ALLOC_STRUCT
{
	ULONG64		RxBufferAddress;  	// Returned buffer address.
	ULONG32 	Length;        		// Length of Allocated buffer
	ULONG32		MaxPacketSize;  	// Maximum Packet Size
	ULONG32		NumberDescriptors;	// Number of C2S Descriptors to allocate (Addressable)
} RET_BUF_ALLOC_STRUCT, *PRET_BUF_ALLOC_STRUCT;

typedef struct _BUF_DEALLOC_STRUCT
{
	ULONG32     EngineNum;  // DMA Engine number to use
	ULONG32	    Reserved;			// Reserved was Allocation type
	ULONG64		RxBufferAddress;  	// Buffer address to Release
} BUF_DEALLOC_STRUCT, *PBUF_DEALLOC_STRUCT;

typedef struct _PACKET_RECEIVE_STRUCT
{
	ULONG32     EngineNum;          // DMA Engine number to use
	ULONG32	    RxReleaseToken;     // Recieve Token of buffer to release (if any)
} PACKET_RECEIVE_STRUCT, *PPACKET_RECEIVE_STRUCT;

typedef struct _PACKET_RET_RECEIVE_STRUCT
{
	ULONG32		RxToken;  			// Token for this recieve.
	ULONG64		UserStatus;			// Contents of UserStatus from the EOP Descriptor
	ULONG64		Address;			// Address of data buffer for the receive
	ULONG32 	Length;        		// Length of packet
} PACKET_RET_RECEIVE_STRUCT, *PPACKET_RET_RECEIVE_STRUCT;

typedef struct _PACKET_SEND_STRUCT
{
	ULONG32     EngineNum;          // DMA Engine number to use
	ULONG64		register_card_offset;         // Byte starting offset in DMA Card Memory
	ULONG64		UserControl;		// Contents to write to UserControl field of SOP Descriptor
	ULONG32 	Length;        		// Length of packet
} PACKET_SEND_STRUCT, *PPACKET_SEND_STRUCT;

typedef struct _PACKET_READ_STRUCT
{
	ULONG32     EngineNum;          // DMA Engine number to use
	UCHAR		Channel;    // not in NWL
	UCHAR		IsMemCheck; // not in NWL
	ULONG64		register_card_offset;         // Byte starting offset in DMA Card Memory
	ULONG32		ModeFlags;			// Mode Flags for PacketReadEx
	ULONG32 	Length;        		// Length of packet
	ULONG64     BufferAddress;      // Buffer Address for data transfer
} PACKET_READ_STRUCT, *PPACKET_READ_STRUCT;

typedef struct _PACKET_RET_READ_STRUCT
{
	ULONG64		UserStatus;			// Contents of UserStatus from the EOP Descriptor or Contents to write to UserControl field of SOP Descriptor
	ULONG32 	Length;        		// Length of packet
} PACKET_RET_READ_STRUCT, *PPACKET_RET_READ_STRUCT;

typedef struct _PACKET_WRITE_STRUCT
{
	ULONG32     EngineNum;          // DMA Engine number to use
	ULONG64		register_card_offset;         // Byte starting offset in DMA Card Memory
	ULONG32		ModeFlags;			// Mode Flags for PacketWriteEx
	ULONG64		UserControl;		// Contents to write to UserControl field of SOP Descriptor
	ULONG32 	Length;        		// Length of packet
} PACKET_WRITE_STRUCT, *PPACKET_WRITE_STRUCT;

typedef struct _PACKET_ENTRY_STRUCT
{
	ULONG64		Address;			// Address of data buffer for the receive
	ULONG32 	Length;        		// Length of packet
	ULONG32 	Status;        		// Packet Status
	ULONG64		UserStatus;			// Contents of UserStatus from the EOP Descriptor
} PACKET_ENTRY_STRUCT, *PPACKET_ENTRY_STRUCT;

typedef struct _PACKET_RECVS_STRUCT
{
	USHORT		EngineNum;          // DMA Engine number to use
	USHORT	 	AvailNumEntries;	// Number of packet entries available
	USHORT 		RetNumEntries;		// Returned Number of packet entries
	USHORT 		EngineStatus;		// DMA Engine status
	PACKET_ENTRY_STRUCT	Packets[1];	// Packet Entries
} PACKET_RECVS_STRUCT, *PPACKET_RECVS_STRUCT;

typedef struct _GLOBAL_IMG_GEN_CTRL_STRUCT
{
	ULONG32     dma_engine_index;    // DMA Engine index to use
	UCHAR		run_stop_mode;	// Packet Generator Control DWORD
	UCHAR		img_scan_mode;   
	USHORT		channel;    // configuration of channel setting. Activiate channel by setting bits. e.g. 11111111B = 8 channels are enabled.   
	ULONG32		frame_number;	// Number of packet to generate, 0 = infinite
	ULONG32 	vrt_pix_num;	// vertical pixel density( valid range 0 to 2^26-1 )
	ULONG32		hor_pix_num;    // horizontal pixel density( valid range 0 to 2^26-1 )
	ULONG32		dataHSize;		// number of bytes per line
	ULONG32		linesPerStripe; // lines per stripe
	ULONG64     gpio_cfg;
	ULONG32 	frm_per_sec;	// frames generated by second. ( valid range 0 to 2^16-1 )
	ULONG32		frm_per_txn;    // frames per transaction
	ULONG32		acq_buf_addr;   // address of acquisition buffer
	ULONG32     acq_buf_chn_offset;
	ULONG32     dbg_test_mode;		//1: Debug Mode is activated
	ULONG32		acquisitionMode; //0 DFLIM, 1 Diagnostics
} GLOBAL_IMG_GEN_CTRL_STRUCT, *PGLOBAL_IMG_GEN_CTRL_STRUCT;

typedef struct _CH_INTERNAL_IMG_GEN_CTRL_STRUCT
{
	ULONG32	beatsPerFrame;
	USHORT	hor_pix_num;
	USHORT	intraBeatDelay;
	ULONG32	intraFrameDelay;

} CH_INTERNAL_IMG_GEN_CTRL_STRUCT, *PCH_INTERNAL_IMG_GEN_CTRL_STRUCT;

typedef struct _SCAN_SUBSYS_STRUCT
{
	USHORT	sync_ctrl;
	UCHAR	gpio_func;
	USHORT	frm_cnt;
	//UCHAR	pll_fltr_ctrl;
	USHORT	pll_sync_offset;
	//ULONG32	pll_cntr_freq;
	ULONG32	galvo_pixel_dwell;
	ULONG32 galvo_pixel_delay;
	ULONG32 galvo_intra_line_delay;
	ULONG32	galvo_intra_frame_delay;

} SCAN_SUBSYS_STRUCT, *PSCAN_SUBSYS_STRUCT;

typedef struct _SAMPLING_CLOCK_SUBSYS_STRUCT
{
	UCHAR	phase_ctrl;
	USHORT	phase_offset;
	UCHAR	phase_step;
	USHORT	phase_limit;

} SAMPLING_CLOCK_SUBSYS_STRUCT, *PSAMPLING_CLOCK_SUBSYS_STRUCT;

typedef struct _STREAM_PROCESSING_SUBSYS_STRUCT
{
	UCHAR	stream_ctrl;
	UCHAR   pulse_interleave;
	USHORT	scan_period;
	ULONG32 downsample_rate;
	USHORT  dc_offset[8];
	USHORT  fir_coefficient[2][4][16];
	ULONG32 pulse_interleave_offset;
	ULONG32 threePhoton_sample_offset;
} STREAM_PROCESSING_SUBSYS_STRUCT, *PSTREAM_PROCESSING_SUBSYS_STRUCT;

typedef struct _ADC_INTER_LAYOUT
	{
		USHORT	jesdCore1		:1, 
			jesdCore2		:1, 
							:4,
		    jesdSysRefSync  :1,
			jesdTestEn		:1, 
							:3, 
			gpio0IntRefSel	:1, 
			gpio1IntRefEn	:1, 
			gpio2LFpgaRefEn	:1, 
			gpio3Filt		:1, 
			gpio4Filt		:1;
	} ADC_INTER_LAYOUT, *PADC_INTER_LAYOUT;

typedef struct _ADC_INTERFACE_SUBSYS_STRUCT
{
	ADC_INTER_LAYOUT before;
	ADC_INTER_LAYOUT after;
	UCHAR gain[4];
} ADC_INTERFACE_SUBSYS_STRUCT, *PADC_INTERFACE_SUBSYS_STRUCT;

//ThorDAQ Galvo Waveform Generation Subsystem Register(s)
typedef struct _GALVO_WAVEFORM_GEN_SUBSYS_STRUCT
{
	ULONG64 ctrlReg;              
	ULONG64 dacUpdateRate[3];       
	USHORT  dacAmplitude;         
	USHORT  dacStepSize[3];         
	ULONG64 dacParkValue[3];
	ULONG64 dacOffset[3];          
	ULONG64 dacChannelMap;  
	ULONG64 doUpdateRate;
	ULONG64 doParkValue;
	ULONG64 doOffset;
	ULONG64 dacAmFilterWindow;
}GALVO_WAVEFORM_GEN_SUBSYS_STRUCT, *PGALVO_WAVEFORM_GEN_SUBSYS_STRUCT;

typedef struct _I2C_SUBSYS_STRUCT
{
	ULONG64 cmdReg;             
	ULONG32 ctrlReg;            
	ULONG64 dataReg1;           
	ULONG64 dataReg2;          
}I2C_SUBSYS_STRUCT, *PI2C_SUBSYS_STRUCT;

typedef struct _DATA_ACQ_CTRL_STRUCT
{
	GLOBAL_IMG_GEN_CTRL_STRUCT			gblCtrl;
	CH_INTERNAL_IMG_GEN_CTRL_STRUCT		chCtrl[4];
	SCAN_SUBSYS_STRUCT					scan;
	SAMPLING_CLOCK_SUBSYS_STRUCT		samplingClock;
	STREAM_PROCESSING_SUBSYS_STRUCT		streamProcessing;
	ADC_INTERFACE_SUBSYS_STRUCT			adcInterface;
	GALVO_WAVEFORM_GEN_SUBSYS_STRUCT    galvoCtrl;
	I2C_SUBSYS_STRUCT                   i2cCtrl;
} DATA_ACQ_CTRL_STRUCT, *PDATA_ACQ_CTRL_STRUCT;

#pragma pack()

// General IOCTL
#define GET_BOARD_CONFIG_IOCTL          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED,   FILE_ANY_ACCESS)
#define DO_MEM_READ_ACCESS_IOCTL        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define DO_MEM_WRITE_ACCESS_IOCTL       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_IN_DIRECT,  FILE_ANY_ACCESS)
#define GET_DMA_ENGINE_CAP_IOCTL        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED,   FILE_ANY_ACCESS)
#define IMG_ACQ_CONF_IOCTL				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED,   FILE_ANY_ACCESS)
#define GET_PERF_IOCTL     				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED,   FILE_ANY_ACCESS)
// Added as of version 4.6.x.x
#define WRITE_PCI_CONFIG_IOCTL			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x809, METHOD_BUFFERED,   FILE_ANY_ACCESS)
#define READ_PCI_CONFIG_IOCTL			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80A, METHOD_BUFFERED,   FILE_ANY_ACCESS)
#define SCAN_LUT_SETUP_IOCTL			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80B, METHOD_BUFFERED,   FILE_ANY_ACCESS)
#define DAC_DESC_SETUP_IOCTL			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80C, METHOD_BUFFERED,   FILE_ANY_ACCESS)

// New interface for FPGA download and Status readback, Bar3 configuration
#define OPEN_OVERFLOW_EVENT_IOCTL		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x810, METHOD_BUFFERED,   FILE_ANY_ACCESS)
#define MESSAGE_EXCHANGE_IOCTL			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x811, METHOD_BUFFERED,   FILE_ANY_ACCESS)

// Packet DMA IOCTLs
#define PACKET_BUF_ALLOC_IOCTL 			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x820, METHOD_IN_DIRECT, 	FILE_ANY_ACCESS)
#define PACKET_BUF_RELEASE_IOCTL 		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x821, METHOD_IN_DIRECT,  FILE_ANY_ACCESS)

// FIFO Packet Mode IOCTLs
#define PACKET_RECEIVE_IOCTL  			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x822, METHOD_OUT_DIRECT,	FILE_ANY_ACCESS)
#define PACKET_SEND_IOCTL 				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x824, METHOD_IN_DIRECT,	FILE_ANY_ACCESS)
#define PACKET_RECEIVES_IOCTL  			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x826, METHOD_OUT_DIRECT,	FILE_ANY_ACCESS)

// Addressable Packet Mode IOCTLs
#define PACKET_CHANNEL_READ_IOCTL  				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x830, METHOD_OUT_DIRECT,	FILE_ANY_ACCESS)
#define PACKET_WRITE_IOCTL 				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x831, METHOD_IN_DIRECT,	FILE_ANY_ACCESS)
#define PACKET_BUF_READ_IOCTL  			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x832, METHOD_OUT_DIRECT,	FILE_ANY_ACCESS)

#endif