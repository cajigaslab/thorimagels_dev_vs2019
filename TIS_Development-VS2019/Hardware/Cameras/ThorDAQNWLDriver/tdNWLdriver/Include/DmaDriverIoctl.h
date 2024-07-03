// -------------------------------------------------------------------------
// 
// PRODUCT:			DMA Driver
// MODULE NAME:		DmaDriverIOCtl.h
// 
// MODULE DESCRIPTION: 
// 
// Contains the Driver IOCtl defines, typedefs, structures and function prototypes.
// 
// $Revision:  $
//
// ------------------------- CONFIDENTIAL ----------------------------------
// 
//              Copyright (c) 2016 by Northwest Logic, Inc.    
//                       All rights reserved. 
// 
// Trade Secret of Northwest Logic, Inc.  Do not disclose. 
// 
// Use of this source code in any form or means is permitted only 
// with a valid, written license agreement with Northwest Logic, Inc. 
// 
// Licensee shall keep all information contained herein confidential  
// and shall protect same in whole or in part from disclosure and  
// dissemination to all third parties. 
// 
// 
//                        Northwest Logic, Inc. 
//                  1100 NW Compton Drive, Suite 100 
//                      Beaverton, OR 97006, USA 
//   
//                        Ph:  +1 503 533 5800 
//                        Fax: +1 503 533 5900 
//                      E-Mail: info@nwlogic.com 
//                           www.nwlogic.com 
// 
// -------------------------------------------------------------------------

//!
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
//  806   Get DMA Engine Cap      	EngineNum (UINT32) 		DMA_CAP_STRUCT
//  808   Get DMA Performance     	EngineNum (UINT32) 		DMA_STAT_STRUCT
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

#ifndef __DMADriverioctl__h_
#define __DMADriverioctl__h_

#define MAX_BARS                        6       // TYPE-0 configuration header values.
#define PCI_STD_CONFIG_HEADER_SIZE      64      // 64 Bytes is the standard header size


#ifndef __WINNT__   // Linux version ---------------------------------------------

#ifndef PACKED
#    define PACKED                      __attribute__((packed))
#endif /* PACKED */

#define PROCFS_FB_NAME             "dmadriver"
#define PROCFS_FB_PATH             "/proc/DMAD/" PROCFS_FB_NAME

#define PROCFS_PATH             	"/proc/DMAD"

#define NWLogic_VendorID            0x19AA
#define NWLogic_PciExp_E000         0xE000
#define NWLogic_PciExp_E001         0xE001
#define NWLogic_PciExp_E002         0xE002
#define NWLogic_PciExp_E003         0xE003
#define NWLogic_PciExp_E004         0xE004

/*!
** Number of descriptors to preallocate for each DMA engine. This
** will limit the maximum amount that can be transfered. Theoretically
** the maximum will be this number times the maximum amount of a 
** single DMA descriptor transfer.
*/
#ifdef ARM
#define NUM_DESCRIPTORS_PER_ENGINE			128
#else
#define NUM_DESCRIPTORS_PER_ENGINE          8192
#endif // ARM | x86_64 Descriptor Count
// General IOCTL
#define GET_BOARD_CONFIG_IOCTL_BASE         0x800
#define DO_MEM_READ_ACCESS_IOCTL_BASE       0x802
#define DO_MEM_WRITE_ACCESS_IOCTL_BASE      0x803
#define GET_DMA_ENGINE_CAP_IOCTL_BASE       0x806
// Packet Gen / Chk control ioctl removed in version 4.9.x.x
#define GET_PERF_IOCTL_BASE                 0x808
// Added as of version 4.6.x.x
#define	WRITE_PCI_CONFIG_IOCTL				0x809
#define READ_PCI_CONFIG_IOCTL				0x80A
// Added in version 4.9.x.x
#define RESET_DMA_ENGINE_IOCTL_BASE			0x858

// Packet DMA IOCTLs
#define PACKET_BUF_ALLOC_IOCTL_BASE 	    0x820
#define PACKET_BUF_RELEASE_IOCTL_BASE       0x821

// FIFO Packet Mode IOCTLs
#define PACKET_RECEIVE_IOCTL_BASE		    0x822
#define PACKET_SEND_IOCTL_BASE			    0x824
#define PACKET_RECEIVES_IOCTL_BASE			0x826

// Addressable Packet Mode IOCTLs
#define PACKET_READ_IOCTL_BASE		    	0x830
#define PACKET_WRITE_IOCTL_BASE			    0x831

// User Interrupt IOCTLs
#define	USER_IRQ_WAIT_IOCTL				    0x841
#define USER_IRQ_CANCEL_IOCTL			    0x842

#define STATUS_INVALID_PARAMETER			1
#define STATUS_INVALID_DEVICE_STATE			2
#define STATUS_INSUFFICIENT_RESOURCES		3

#else   // Window version  ------------------------------------------------------------

// General IOCTL
#define GET_BOARD_CONFIG_IOCTL          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED,   FILE_ANY_ACCESS)
#define DO_MEM_READ_ACCESS_IOCTL        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define DO_MEM_WRITE_ACCESS_IOCTL       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_IN_DIRECT,  FILE_ANY_ACCESS)
#define GET_DMA_ENGINE_CAP_IOCTL        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED,   FILE_ANY_ACCESS)
// Packet Gen / Chk control ioctl removed in version 4.9.x.x
#define GET_PERF_IOCTL     				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED,   FILE_ANY_ACCESS)
#define RESET_DMA_ENGINE_IOCTL			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x858, METHOD_BUFFERED,   FILE_ANY_ACCESS)
// Added as of version 4.6.x.x
#define WRITE_PCI_CONFIG_IOCTL			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x809, METHOD_BUFFERED,   FILE_ANY_ACCESS)
#define READ_PCI_CONFIG_IOCTL			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80A, METHOD_BUFFERED,   FILE_ANY_ACCESS)

// Packet DMA IOCTLs
#define PACKET_BUF_ALLOC_IOCTL 			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x820, METHOD_IN_DIRECT, 	FILE_ANY_ACCESS)
#define PACKET_BUF_RELEASE_IOCTL 		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x821, METHOD_IN_DIRECT,  FILE_ANY_ACCESS)

// FIFO Packet Mode IOCTLs
#define PACKET_RECEIVE_IOCTL  			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x822, METHOD_OUT_DIRECT,	FILE_ANY_ACCESS)
#define PACKET_SEND_IOCTL 				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x824, METHOD_IN_DIRECT,	FILE_ANY_ACCESS)
#define PACKET_RECEIVES_IOCTL  			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x826, METHOD_OUT_DIRECT,	FILE_ANY_ACCESS)

// Addressable Packet Mode IOCTLs
#define PACKET_READ_IOCTL  				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x830, METHOD_OUT_DIRECT,	FILE_ANY_ACCESS)
#define PACKET_WRITE_IOCTL 				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x831, METHOD_IN_DIRECT,	FILE_ANY_ACCESS)

// User Interrupt IOCTLs
#define	USER_IRQ_WAIT_IOCTL				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x841, METHOD_BUFFERED,	FILE_ANY_ACCESS)
#define USER_IRQ_CANCEL_IOCTL			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x842, METHOD_OUT_DIRECT,	FILE_ANY_ACCESS)

//#pragma pack(push,1)
#pragma pack(1)

#ifndef PACKED
#define PACKED                      
#endif /* PACKED */

#endif  // WINDOWS version ----------------------------------------------------------

#define PCI_DEVICE_SPECIFIC_SIZE 192    // Device specific bytes [64..256].

typedef struct _PCI_CONFIG_HEADER
{
    UINT16	VendorId;           // VendorID[15:0] 0-1
	UINT16	DeviceId;           // DeviceID[15: 2-3
    UINT16	Command;            // Command[15:0] 4-5
    UINT16	Status;             // Status[15:0] 6-7
    UINT8   RevisionId;         // RevisionId[7:0] 8
    UINT8	Interface;          // Interface[7:0] 9
    UINT8	SubClass;           // SubClass[7:0] 0xA
    UINT8	BaseClass;          // BaseClass[7:0] 0xB
    UINT8	CacheLineSize;		// 0xC
    UINT8	LatencyTimer;		// 0xD
    UINT8	HeaderType;			// 0xE
    UINT8	BIST;				// 0xF
    UINT32	BarCfg[MAX_BARS];   // BAR[5:0] Configuration	0x10, 0x14, 0x18, 0x1C, 0x20, 0x24
    UINT32	CardBusCISPtr;		// 0x28
    UINT16	SubsystemVendorId;  // SubsystemVendorId[15:0] 0x2C
    UINT16	SubsystemId;        // SubsystemId[15:0] 0x2E
    UINT32	ExpRomCfg;          // Expansion ROM Configuration 0x30
    UINT8	CapabilitiesPtr;	// 0x34
    UINT8	Reserved1[3];       // 0x35
    UINT32	Reserved2;          // 0x38
    UINT8	InterruptLine;      // 0x3C
    UINT8	InterruptPin;       // 0x3D
    UINT8	MinimumGrant;       // 0x3E
    UINT8	MaximumLatency;     // 0x3F
    UINT8	DeviceSpecific[PCI_DEVICE_SPECIFIC_SIZE];
} PACKED PCI_CONFIG_HEADER, *PPCI_CONFIG_HEADER;

typedef struct _PCI_DEVICE_SPECIFIC_HEADER
{
    UINT8	PCIeCapabilityID;
    UINT8	PCIeNextCapabilityPtr;
    UINT16	PCIeCapability;
    UINT32	PCIeDeviceCap;      // PCIe Capability: Device Capabilities
    UINT16	PCIeDeviceControl;  // PCIe Capability: Device Control
    UINT16	PCIeDeviceStatus;   // PCIe Capability: Device Status
    UINT32	PCIeLinkCap;        // PCIe Capability: Link Capabilities
    UINT16	PCIeLinkControl;    // PCIe Capability: Link Control
    UINT16	PCIeLinkStatus;     // PCIe Capability: Link Status
} PACKED PCI_DEVICE_SPECIFIC_HEADER, *PPCI_DEVICE_SPECIFIC_HEADER;

/* The PCI Config space capabilites for MSI */
typedef struct _PCI_MSI_CAPABILITY  {
    UINT8	PCIeCapabilityID;
    UINT8	PCIeNextCapabilityPtr;
	UINT16	MessageControl;
} PACKED PCI_MSI_CAPABILITY, *PPCI_MSI_CAPABILITY;

/* The PCI Config space capabilites for MSIX */
typedef struct _PCI_MSIX_CAPABILITY  {
    UINT8	PCIeCapabilityID;
    UINT8	PCIeNextCapabilityPtr;
	UINT16	MessageControl;
	UINT32	TableOffsetBIR;
	UINT32	PBAOffsetBIR;
} PACKED PCI_MSIX_CAPABILITY, *PPCI_MSIX_CAPABILITY;

/*!
 * \struct BOARD_CONFIG_STRUCT
 * \brief  Board Configuration Definitions
 */
typedef struct _BOARD_CONFIG_STRUCT
{
    UINT8   NumDmaWriteEngines; // Number of S2C DMA Engines
    UINT8   FirstDmaWriteEngine;// Number of 1st S2C DMA Engine
    UINT8   NumDmaReadEngines;  // Number of C2S DMA Engines
    UINT8   FirstDmaReadEngine; // Number of 1st C2S DMA Engine
    UINT8   DMARegistersBAR;    // BAR Number where DMA Registers reside
    UINT8   reserved1;			// Reserved
	UINT8	DriverVersionMajor;	// Driver Major Revision number
	UINT8	DriverVersionMinor;	// Driver Minor Revision number
	UINT8	DriverVersionSubMinor;	// Driver Sub Minor Revision number 
	UINT8	DriverVersionBuildNumber;	// Driver Build number
	UINT32	CardStatusInfo;			// BAR 0 + 0x4000 DMA_Common_Control_and_Status
	UINT32	DMABackEndCoreVersion;	// DMA Back End Core version number
	UINT32	PCIExpressCoreVersion;	// PCI Express Core version number
	UINT32	UserVersion;		// User Version Number (optional)
	// The following structure is placed at the end due to packing issues in Linux
	// Do Not Move or the Get Board Cfg command will fail.
    struct _PCI_CONFIG_HEADER   PciConfig;
} PACKED BOARD_CONFIG_STRUCT, *PBOARD_CONFIG_STRUCT;

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

// Rd_Wr_n defines
#define WRITE_TO_CARD                       0x0             // S2C
#define READ_FROM_CARD                      0x1             // C2S

// Return status defines
#define STATUS_SUCCESSFUL		    	    0x00
#define STATUS_INCOMPLETE			        0x01
#define STATUS_INVALID_BARNUM		        0x02
#define STATUS_INVALID_CARDOFFSET	        0x04
#define STATUS_OVERFLOW				        0x08
#define STATUS_INVALID_BOARDNUM		        0x10			// Board number does not exist
#define STATUS_INVALID_MODE					0x20			// Mode not supported by hardware
#define STATUS_BAD_PARAMETER				0x40			// Passed an invalid parameter

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
#define DMA_CAP_BLOCK_DMA                   0x00000000		// No Longer Supported.
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
#define DMA_CAP_CARD_ADDR_SIZE(x)			\
	(1ULL << (((x) >> DMA_CAP_CARD_ADDR_SIZE_SHIFT) & DMA_CAP_CARD_ADDR_SIZE_MASK))
#define DMA_CAP_STATS_SCALING_FACTOR_0  	0x00000000
#define DMA_CAP_STATS_SCALING_FACTOR_2  	0x40000000
#define DMA_CAP_STATS_SCALING_FACTOR_4  	0x80000000
#define DMA_CAP_STATS_SCALING_FACTOR_8  	0xC0000000

#define READ_WRITE_MODE_FLAG_FIFO			0x00000001
#define READ_WRITE_MODE_FLAG_ADDRESSED		0x00000000

/*!
 * \struct DMA_STAT_STRUCT
 * \brief DMA Status Structure - Status Information from a DMA IOCTL transaction
 */
typedef struct _DMA_STAT_STRUCT
{
#ifndef __WINNT__
	UINT32  EngineNum;          // DMA Engine number to use
#endif  // Linux Only
	UINT64  CompletedByteCount; // Number of bytes transferred
	UINT64  DriverTime;         // Number of nanoseconds for driver
	UINT64  HardwareTime;       // Number of nanoseconds for hardware
	UINT64  IntsPerSecond;      // Number of interrupts per second
	UINT64  DPCsPerSecond;      // Number of DPCs/Tasklets per second
} DMA_STAT_STRUCT, *PDMA_STAT_STRUCT;

// DO_MEM_STRUCT
// Do Memory Structure - Information for performing a Memory Transfer
typedef struct _DO_MEM_STRUCT
{
	UINT32	BarNum;             // Base Addres Register (BAR) to access
	UINT64	Offset;             // Byte starting offset in application buffer
	UINT64	CardOffset;         // Byte starting offset in BAR
	UINT64	Length;             // Transaction length in bytes
#ifndef __WINNT__
	UINT8 * Buffer;             // Buffer Pointer
#endif  // Linux Only
} DO_MEM_STRUCT, *PDO_MEM_STRUCT;

// DMA_CAP_STRUCT
// DMA Capabilities Structure
typedef struct _DMA_CAP_STRUCT
{
#ifndef __WINNT__
	UINT32  EngineNum;          // DMA Engine number to use
#endif  // Linux Only
	UINT32  DmaCapabilities;    // DMA Capabilities
} DMA_CAP_STRUCT, *PDMA_CAP_STRUCT;

// RW_PCI_CONFIG_STRUCT
// PCI Config Read/Write Structure - Information for reading or writing CI onfiguration space
typedef struct _RW_PCI_CONFIG_STRUCT
{
	UINT32  Offset;             // Byte starting offset in application buffer
	UINT32	Length;             // Transaction length in bytes
#ifndef __WINNT__
	PUINT8  Buffer;             // Buffer Pointer
#endif  // Linux Only
} RW_PCI_CONFIG_STRUCT, *PRW_PCI_CONFIG_STRUCT;

//*************************************************************
// Packet Mode defines and Structures

#define		PACKET_MODE_FIFO					0x00
#define		PACKET_MODE_ADDRESSABLE				0x02
#define		PACKET_MODE_STREAMING			    0x40

/* DMA_MODE_NOT_SET - DMA Mode not set, uses default DMA Engine behavior */
#define		DMA_MODE_NOT_SET					0xFFFFFFFF

// Minimum Buffer pool size is 65536 (16 * PAGE_SIZE (4096 bytes))
//    This equates to 16 DMA Descriptors allocated
#define		MIN_BUFFER_POOL_SIZE				65536

/*!
 * \struct BUF_ALLOC_STRUCT
 * \brief Buffer Allocate Structure - Information for performing the Buffer Allocation task
 */
typedef struct _BUF_ALLOC_STRUCT
{
	UINT32  EngineNum;          // DMA Engine number to use
	UINT32  AllocationMode;     // Allocation type Flags
	UINT32	MaxPacketSize;  	// Maximum Packet Size (Must be dividable into PoolSize) (FIFO mode)
	UINT32	Length;				// Length of Application buffer or size to Allocate (FIFO mode)
	UINT32	NumberDescriptors;	// Number of C2S Descriptors to allocate (Addressable)
#ifndef __WINNT__ // Linux variant
    PVOID  BufferAddress;      // Buffer Address for data transfer
#endif // Linux -
} BUF_ALLOC_STRUCT, *PBUF_ALLOC_STRUCT;

#ifdef __WINNT__ // Windows Only

/*! 
 * \struct RET_BUF_ALLOC_STRUCT
 * \brief Return Buffer Allocate Structure 
 *  Information returned by the Rx Buffer Allocation task
 */
typedef struct _RET_BUF_ALLOC_STRUCT
{
	UINT32	Length;        		// Length of Allocated buffer
	UINT32	MaxPacketSize;  	// Maximum Packet Size
	UINT32	NumberDescriptors;	// Number of C2S Descriptors to allocate (Addressable)
} RET_BUF_ALLOC_STRUCT, *PRET_BUF_ALLOC_STRUCT;
#endif // Only used on Windows version

/*!
 * \struct RX_BUF_DEALLOC_STRUCT
 * \brief Rx Buffer DeAllocate Structure 
 *  Information for performing the Rx Buffer Release task
 */
typedef struct _BUF_DEALLOC_STRUCT
{
	UINT32  EngineNum;          // DMA Engine number to use
	UINT32  Reserved;			// Reserved was Allocation type
	UINT64	RxBufferAddress;  	// Buffer address to Release
} BUF_DEALLOC_STRUCT, *PBUF_DEALLOC_STRUCT;

#ifdef __WINNT__    /* Windows Version of Packet Structures */
/*!
 * \struct PACKET_RECEIVE_STRUCT
 * \brief Packet Receive Structure 
 *  Information for the PacketReceive function
 */
typedef struct _PACKET_RECEIVE_STRUCT
{
	UINT32  EngineNum;          // DMA Engine number to use
	UINT32  RxReleaseToken;     // Recieve Token of buffer to release (if any)
    BOOLEAN Block;              // Blocking or Non-Blocking call
} PACKET_RECEIVE_STRUCT, *PPACKET_RECEIVE_STRUCT;

/*! 
 * \struct PACKET_RET_RECEIVE_STRUCT
 * \brief Packet Return Receive Structure 
 *  Information from the PacketReceive function
 */
typedef struct _PACKET_RET_RECEIVE_STRUCT
{
	UINT32	RxToken;  			// Token for this recieve.
	UINT64	UserStatus;			// Contents of UserStatus from the EOP Descriptor
	UINT64	Address;			// Address of data buffer for the receive
	UINT32	Length;        		// Length of packet
} PACKET_RET_RECEIVE_STRUCT, *PPACKET_RET_RECEIVE_STRUCT;
#else   /* Linux version of Packet Recieve structures */

//  Packet Receive Structure - Information for the PacketReceive function
typedef struct _PACKET_RECEIVE_STRUCT
{
	UINT32  EngineNum;          // DMA Engine number to use
	UINT32	RxToken;            // Recieve Token of buffer to release (if any)
	UINT64	UserStatus;			// Contents of UserStatus from the EOP Descriptor
	UINT64	Address;			// Address of data buffer for the receive
	UINT32 	Length;        		// Length of packet
    BOOLEAN Block;              // Blocking or Non-Blocking call
} PACKET_RECEIVE_STRUCT, *PPACKET_RECEIVE_STRUCT;

#endif  /* Windows vs. Linux Packet Recieve structure(s) */

/*! 
 * \struct PACKET_SEND_STRUCT
 * \brief Packet Send Structure 
 *  Information for the PacketSend function
 */
typedef struct _PACKET_SEND_STRUCT
{
	UINT32  EngineNum;          // DMA Engine number to use
	UINT64	CardOffset;         // Byte starting offset in DMA Card Memory
	UINT64	UserControl;		// Contents to write to UserControl field of SOP Descriptor
	UINT32 	Length;        		// Length of packet
#ifndef __WINNT__ // Linux variant
    void *      BufferAddress;      // Buffer Address for data transfer
#endif // Linux -
} PACKET_SEND_STRUCT, *PPACKET_SEND_STRUCT;

#ifdef __WINNT__    /* Windows Version of Packet Structures */

/*!
 * \struct PACKET_READ_STRUCT
 * \brief Packet Read Structure 
 *  Information for the PacketRead function
 */
typedef struct _PACKET_READ_STRUCT
{
	UINT32  EngineNum;          // DMA Engine number to use
	UINT64	CardOffset;         // Byte starting offset in DMA Card Memory
	UINT32	ModeFlags;			// Mode Flags for PacketReadEx
	UINT32 	Length;        		// Length of packet
	UINT64  BufferAddress;      // Buffer Address for data transfer
} PACKET_READ_STRUCT, *PPACKET_READ_STRUCT;

/*!
 * \struct PACKET_RET_READ_STRUCT
 * \brief Packet Returned Read Structure 
 *  Information from the PacketRead function
 */
typedef struct _PACKET_RET_READ_STRUCT
{
	UINT64	UserStatus;			// Contents of UserStatus from the EOP Descriptor or Contents to write to UserControl field of SOP Descriptor
	UINT32 	Length;        		// Length of packet
} PACKET_RET_READ_STRUCT, *PPACKET_RET_READ_STRUCT;

/*! 
 * \struct PACKET_WRITE_STRUCT
 * \brief Packet Write Structure 
 *  Information for the PacketWrite function
 */
typedef struct _PACKET_WRITE_STRUCT
{
	UINT32  EngineNum;          // DMA Engine number to use
	UINT64	CardOffset;         // Byte starting offset in DMA Card Memory
	UINT32	ModeFlags;			// Mode Flags for PacketWriteEx
	UINT64	UserControl;		// Contents to write to UserControl field of SOP Descriptor
	UINT32 	Length;        		// Length of packet
} PACKET_WRITE_STRUCT, *PPACKET_WRITE_STRUCT;

#else // Linux version

// PACKET_READ_WRITE_STRUCT
//
//  Packet Read and Write Structure - Information for the PacketRead and PacketWrite function
typedef struct _PACKET_READ_WRITE_STRUCT
{
	UINT32  EngineNum;          // DMA Engine number to use
	UINT64	CardOffset;         // Byte starting offset in DMA Card Memory
	UINT32	ModeFlags;			// Mode Flags for PacketReadEx/WriteEx
	UINT64	UserInfo;			// Contents of UserStatus from the EOP Descriptor or Contents to write to UserControl field of SOP Descriptor
	UINT32 	Length;        		// Length of packet
    PVOID  BufferAddress;      // Buffer Address for data transfer
} PACKET_READ_WRITE_STRUCT, *PPACKET_READ_WRITE_STRUCT;

#endif  // Windows vs. Linux Packet Read structure(s)

// PACKET_ENTRY_STRUCT
//
//  Packet Entry Structure - Per packet structure to be included into
//		The PACKET_RECVS_STRUCT
typedef struct _PACKET_ENTRY_STRUCT
{
	UINT64	Address;			// Address of data buffer for the receive
	UINT32 	Length;        		// Length of packet
	UINT32 	Status;        		// Packet Status
	UINT64	UserStatus;			// Contents of UserStatus from the EOP Descriptor
} PACKET_ENTRY_STRUCT, *PPACKET_ENTRY_STRUCT;

// Engine Status definitions
#define	DMA_OVERRUN_ERROR		0x8000
#define	PACKET_ERROR_MALFORMED	0x4000

// PACKET_RECVS_STRUCT
//
//  Packet Recieves Structure - Superstructure that contains multiple packet 
//	recieve indications
typedef struct _PACKET_RECVS_STRUCT
{
	UINT16	EngineNum;          // DMA Engine number to use
	UINT16	AvailNumEntries;	// Number of packet entries available
	UINT16 	RetNumEntries;		// Returned Number of packet entries
	UINT16 	EngineStatus;		// DMA Engine status
	PACKET_ENTRY_STRUCT	Packets[1];	// Packet Entries
} PACKET_RECVS_STRUCT, *PPACKET_RECVS_STRUCT;

// USER_IRQ_WAIT_STRUCT - Information passed in the DeviceIOControl (ioctl)
//		that waits for the interrupt or times out after duration defined by <dwTimeoutMilliSec>.
// 
typedef struct _USER_IRQ_WAIT_STRUCT
{
	UINT32 boardNum;            // index of ThorDAQ board
	UINT32	dwTimeoutMilliSec;	// Timeout in ms, 0 = no time out.
	PUINT32 DMA_Bank;           // IN: current bank,  OUT: next bank (0 or 1)
	PUINT32 NWL_Common_DMA_Register_Block; 
} USER_IRQ_WAIT_STRUCT, *PUSER_IRQ_WAIT_STRUCT;

// RESET_DMA_STRUCT
//
//  Reset DMA Engine ControlStructure
//  *********************THIS IS USED FOR TESTING ONLY**********************
typedef struct _RESET_DMA_STRUCT
{
	char	EngineNum;          // DMA Engine number to reset
} RESET_DMA_STRUCT, *PRESET_DMA_STRUCT;

#ifdef __WINNT__   // Windows version
#pragma pack()
//#pragma pack(pop)
#endif // Windows version

#endif /* __DMADriverioctl__h_ */

