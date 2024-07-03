// -------------------------------------------------------------------------
// 
// PRODUCT:			DMA Driver
// MODULE NAME:		DmaDriverHw.h
// 
// MODULE DESCRIPTION: 
// 
// Contains the Hardware specific defines, typedefs, structures.
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

#ifndef _DMA_DRIVER_HW_H_
#define _DMA_DRIVER_HW_H_

#pragma pack(push,1)

/*! \note
 * The following pragma will disable warnings for:
 * nameless struct/union warning
 */
#ifdef __WINNT__
#pragma warning(disable:4201)  
#endif 	// end pragma in the Windows driver.

#ifndef __WINNT__
 #ifndef PACKED
 #define PACKED		__attribute__((packed))
 #endif	// PACKED
#else	// Windows
 #ifndef PACKED
 #define PACKED 
#endif	// end PACKED conditional compile.

// ---------------------------------------------------------------------
// DMA Defines
// ---------------------------------------------------------------------
#define MAX_NUM_DMA_ENGINES		64
#endif	// Windows vs. Linux

#define DMA_REG_BAR				0

#define	MSIX_MESSAGE_ID			0x11
#define	TABLE_BIR_MASK			0x07
#define	MSI_MESSAGE_ID			0x05

// ---------------------------------------------------------------------
// DMA Descriptor Defines
// ---------------------------------------------------------------------

/*! \note
 * Below are the descriptor counts for the Linux driver.
 * to determine the maximum possible transfer size, as configured,
 * multiply the number of descriptors (the quantity) by page size.
 * By default:	the x86_64 driver allocates 8,192 x 4096-byte pages.
 * 				the ARM driver allocates 128 x 4096-byte pages.
 */ 

//! DMA descriptors must be on a 32 byte boundary
#ifdef  ARM
#define DMA_NUM_DESCR			128
#else
#define	DMA_NUM_DESCR			16384			
#endif  // ARM | x86_64			
#define DMA_DESCR_ALIGN_LEN     32
#define DMA_DESCR_ALIGN_REQUIREMENT	(FILE_32_BYTE_ALIGNMENT)
//! Maximum number of bytes for number of descriptors (-1 for alignment)
#define DMA_MAX_TRANSFER_LENGTH ((DMA_NUM_DESCR-2) * PAGE_SIZE)
#define	MINIMUM_NUMBER_DESCRIPTORS	32

/*! \note
 * The watchdog interval.  Any request pending for this number of seconds is
 * cancelled.  This is a one second granularity timer, so the timeout must be
 * at least one second more than the minimum allowed.
 */
#define CARD_WATCHDOG_INTERVAL   5

#ifndef __WINNT__	// Linux Version

#define DMA_DESCR_ALIGN_VIRT_ADDR(x) ((PDMA_DESCRIPTOR_STRUCT)((((unsigned long)(x)) + (DMA_DESCR_ALIGN_LEN - 1)) & ~(DMA_DESCR_ALIGN_LEN - 1)))
#define DMA_DESCR_ALIGN_PHYS_ADDR(x) (((x) + (DMA_DESCR_ALIGN_LEN - 1)) & ~(DMA_DESCR_ALIGN_LEN - 1))

#define DMA_TRANS_SIGNATURE                0x6DB6aa55

/*!
 \struct DMA_TRANSACTION_STRUCT
  Linux Conditional Compile
*/
typedef struct _DMA_TRANSACTION_STRUCT
{
    struct list_head        list;
	INT32					signature;
    size_t                  length;
	UINT64		       		CardOffset;
	UINT64		       		UserInfo;		// UserStatus or UserControl
    INT32                   offset;
    INT32                   numPages;
    struct page **          pages;
    INT32                   direction;
	INT32					status; 
	UINT32					PacketStatus;
	UINT32					BytesTransfered;

	UINT16					DMAAvailable;
    UINT16					DMAComplete;

	wait_queue_head_t       waiting;
	wait_queue_head_t       completed;

	struct scatterlist *	pSgList;
    INT32                   SgNumElements;
} DMA_TRANSACTION_STRUCT, *PDMA_TRANSACTION_STRUCT;

// Interrupt status defines
#define IRQ_DMA_COMPLETE(x) 	(UINT64) (0x00000001 << (x))

#endif // Linux Version

/*! Board Register set.
 * \struct DMA_COMMON_CONTROL_STRUCT
 * \note DMA Common Control Structure
 */
typedef struct _DMA_COMMON_CONTROL_STRUCT
{
	UINT32	ControlStatus;
	UINT32	DMABackEndCoreVersion;
	UINT32	PCIExpressCoreVersion;
	UINT32	UserVersion;
	UINT8	Reserved[0x4000 - (4 * sizeof(UINT32))];	// Reserved size
	UINT32 	Reserved1[16];								// Reserved size
	UINT32  UserIntControl;
	UINT32 	Reserved2[3];								// Reserved size
	UINT32  UserIntStatus;
} PACKED DMA_COMMON_CONTROL_STRUCT, *PDMA_COMMON_CONTROL_STRUCT;

// DMA Common ControlStatus defs
#define CARD_IRQ_ENABLE					(1 << 0)
#define CARD_IRQ_ACTIVE					(1 << 1)
#define CARD_IRQ_PENDING				(1 << 2)
#define CARD_IRQ_MSI					(1 << 3)
#define CARD_USER_INTERRUPT_MODE		(1 << 4)
#define CARD_USER_INTERRUPT_ACTIVE		(1 << 5)
#define CARD_IRQ_MSIX_MODE				(1 << 6)
#define CARD_MAX_PAYLOAD_SIZE_MASK		0x0700
#define CARD_MAX_READ_REQUEST_SIZE_MASK	0x7000
#define CARD_S2C_INTERRUPT_STATUS_MASK	0x00FF0000
#define CARD_C2S_INTERRUPT_STATUS_MASK	0xFF000000

#define CARD_MAX_C2S_SIZE(x)        (1UL << ((((x) >> 8) & 0xFF)+7))
#define CARD_MAX_S2C_SIZE(x)        (1UL << ((((x) >> 12) & 0xFF)+7))

#define COMMON_DMA_CTRL_IRQ_ENABLE         		0x00000001
#define COMMON_DMA_CTRL_IRQ_ACTIVE         		0x00000002

#define PACKET_DMA_CTRL_DESC_COMPLETE			0x00000004
#define PACKET_DMA_CTRL_DESC_ALIGN_ERROR		0x00000008
#define PACKET_DMA_CTRL_DESC_FETCH_ERROR		0x00000010
#define PACKET_DMA_CTRL_DESC_SW_ABORT_ERROR		0x00000020
#define PACKET_DMA_CTRL_DESC_CHAIN_END			0x00000080
#define PACKET_DMA_CTRL_DMA_ENABLE				0x00000100
#define PACKET_DMA_CTRL_DMA_RUNNING				0x00000400
#define PACKET_DMA_CTRL_DMA_WAITING				0x00000800
#define PACKET_DMA_CTRL_DMA_WAITING_PERSIST 	0x00001000
#define PACKET_DMA_CTRL_DMA_RESET_REQUEST	 	0x00004000
#define PACKET_DMA_CTRL_DMA_RESET			 	0x00008000

#define	PACKET_DMA_INT_CTRL_IRQ_COMPLETE		0x00000000
#define PACKET_DMA_INT_CTRL_INT_EOP				0x00000002

#define PACKET_DESC_STATUS_MASK					0xFF000000
#define PACKET_DESC_COMPLETE_BYTE_COUNT_MASK	0x000FFFFF
#define PACKET_DESC_CONTROL_MASK				0xFF000000
#define PACKET_DESC_CARD_ADDRESS32_35_MASK		0x00F00000
#define PACKET_DESC_BYTE_COUNT_MASK				0x000FFFFF

#define PACKET_DESC_C2S_CTRL_IRQ_ON_COMPLETE	0x01000000
#define PACKET_DESC_C2S_CTRL_IRQ_ON_ERROR		0x02000000
#define PACKET_DESC_C2S_CTRL_START_OF_PACKET	0x80000000		// Addressable Packet Mode on on C2S
#define PACKET_DESC_C2S_CTRL_END_OF_PACKET		0x40000000		// Addressable Packet Mode on on C2S

#define PACKET_DESC_C2S_STAT_START_OF_PACKET	0x80000000
#define PACKET_DESC_C2S_STAT_END_OF_PACKET		0x40000000
#define PACKET_DESC_C2S_STAT_ERROR				0x10000000
#define PACKET_DESC_C2S_STAT_USER_STAT_HI_0		0x08000000
#define PACKET_DESC_C2S_STAT_USER_STAT_LO_0		0x04000000
#define PACKET_DESC_C2S_STAT_SHORT				0x02000000
#define PACKET_DESC_C2S_STAT_COMPLETE			0x01000000

/*! Packet Mode DMA Descriptor Card-to-System (Read).
* \struct PACKET_DESCRIPTOR_C2S_STRUCT
*/
typedef struct _PACKET_DESCRIPTOR_C2S_STRUCT
{
	// Hardware specific entries - Do not change or reorder
	UINT32		StatusFlags_BytesCompleted;		// C2S StatusFlags and Bytes count completed       
	UINT64		UserStatus;                     // C2S User Status - Passed from Users design
	UINT32		CardAddress;                    // C2S Card Address (offset) Not used in FIFO Mode 
	UINT32		ControlFlags_ByteCount;         // C2S Control Flags and Bytes Count to transfer   
	UINT64		SystemAddressPhys;              // C2S System Physical Buffer Address to transfer  
	UINT32		NextDescriptorPhys;             // C2S Physical Address to Next DMA Descriptor     
} PACKED PACKET_DESCRIPTOR_C2S_STRUCT, *PPACKET_DESCRIPTOR_C2S_STRUCT;

#define PACKET_DESC_S2C_CTRL_IRQ_ON_COMPLETE	0x01000000
#define PACKET_DESC_S2C_CTRL_IRQ_ON_ERROR		0x02000000
#define PACKET_DESC_S2C_CTRL_FIFO_OVERRIDE		0x04000000
#define PACKET_DESC_S2C_CTRL_END_OF_PACKET		0x40000000
#define PACKET_DESC_S2C_CTRL_START_OF_PACKET	0x80000000

#define PACKET_DESC_S2C_STAT_COMPLETE			0x01000000
#define PACKET_DESC_S2C_STAT_SHORT				0x02000000
#define PACKET_DESC_S2C_STAT_ERROR				0x10000000

/*! Packet Mode DMA Descriptor System-to-Card (Write).
* \struct PACKET_DESCRIPTOR_S2C_STRUCT
*/
typedef struct _PACKET_DESCRIPTOR_S2C_STRUCT
{
	// Hardware specific entries - Do not change or reorder
	UINT32		StatusFlags_BytesCompleted;		// S2C StatusFlags and Bytes count completed
	UINT64		UserControl;					// S2C User Control - passed to Users design
	UINT32		CardAddress;					// S2C Card Address (offset) Not used in FIFO Mode
	UINT32		ControlFlags_ByteCount;			// S2C Control Flags and Bytes Count to transfer
	UINT64		SystemAddressPhys;				// S2C System Physical Buffer Address to transfer
	UINT32		NextDescriptorPhys;				// S2C Physical Address to Next DMA Descriptor
} PACKED PACKET_DESCRIPTOR_S2C_STRUCT, *PPACKET_DESCRIPTOR_S2C_STRUCT;

#define	DESC_FLAGS_SIG							0x6db6AA00
#define	DESC_FLAGS_SIG_MASK						0xFFFFFF00
#define	DESC_FLAGS_MASK							0x000000FF
#define	DESC_FLAGS_HW_OWNED						0x00000001
#define	DESC_FLAGS_SW_OWNED						0x00000002
#define	DESC_FLAGS_SW_FREED						0x00000004
#define	DESC_FLAGS_ADDRESSABLE_MODE				0x00000008

typedef struct _DMA_DESCRIPTOR_STRUCT
{
	union
	{
		PACKET_DESCRIPTOR_S2C_STRUCT	S2C;	// HW System To Card Descriptor components
		PACKET_DESCRIPTOR_C2S_STRUCT	C2S;	// HW Card To System Descriptor components
	}; // Direction;							// Direction specific Hardware descriptor
} PACKED DMA_DESCRIPTOR_STRUCT, *PDMA_DESCRIPTOR_STRUCT;

/*! DMA Engine
*  \struct DMA_ENGINE_STRUCT
*/
typedef struct _DMA_ENGINE_STRUCT
{
	UINT32			Capabilities;				// Common Capabilities
	volatile UINT32	ControlStatus;				// Common Control and Status
	UINT32			NextDescriptorPtr;			// Packet Mode Next Descriptor Pointer (Physical)
	UINT32			SoftwareDescriptorPtr;		// Packet Mode Software Owned Descriptor pointer (Physical)
	UINT32			CompletedDescriptorPtr;		// Packet Mode Completed Packet Descriptor Pointer (Physical)
	UINT32			DMAActiveTime;				// Amount of time the DMA was active / second (4ns resolution)
	UINT32			DMAWaitTime;				// Amount of time the DMA was inactive / second (4ns resolution)
	UINT32			DMACompletedByteCount;		// Amount DMA byte transfers / second (4 byte resolution)
	UINT32 			InterruptControl;			// New Firmware Interrupt Control register
	UINT8			Reserved[0x100 - (9 * sizeof(UINT32))];	// Reserved size
} PACKED DMA_ENGINE_STRUCT, *PDMA_ENGINE_STRUCT;

//----------------------------------------------------------------------
// Descriptor Engine Register Set.
// ---------------------------------------------------------------------
#   define      DMA_CTRL_IRQ_ENABLE                    (1 << 0)
#   define      DMA_CTRL_IRQ_ACTIVE                    (1 << 1)
#   define      DMA_CTRL_CHAIN_START                   (1 << 8)
#   define      DMA_CTRL_CHAIN_STOP                    (1 << 9)
#   define      DMA_STAT_CHAIN_RUNNING                 (1 << 10)
#   define      DMA_STAT_CHAIN_COMPLETE                (1 << 11)
#   define      DMA_STAT_CHAIN_SHORT_ERR               (1 << 12)
#   define      DMA_STAT_CHAIN_SHORT_SW                (1 << 13)
#   define      DMA_STAT_CHAIN_SHORT_HW                (1 << 14)

/*! \note The following pragma removes the warning condition for
 *  a Zero sized array.
 */
#ifdef __WINNT__	// pragma Conditional Compile
 #pragma warning(disable:4200)  
#endif		      	// Windows Driver Only.

// ---------------------------------------------------------------------
//  Driver DMA Descriptor Ancillary data
// ---------------------------------------------------------------------
typedef struct _DRIVER_DESC_STRUCT
{
	struct _DRIVER_DESC_STRUCT * pNextDesc;		// Driver usable pointer to the next descriptor
	UINT32				DescriptorNumber;		// "Token" for this descriptor
	UINT32				DescFlags;				// Flags for this decriptor
	PDMA_DESCRIPTOR_STRUCT	pHWDesc;			// Pointer to the associated HW DMA Descriptor
#ifdef __WINNT__  	// Conditional Compile
	PHYSICAL_ADDRESS	pHWDescPhys;			// Physical address for the HW descriptor
	PSCATTER_GATHER_LIST pScatterGatherList;	// Pointer to the Scatter List for this set of descriptors
	PVOID *				SystemAddressVirt;		// User address for the SystemAddress
	WDFDMATRANSACTION 	DmaTransaction;		    // Contains the DMA Transaction associated to this descriptor
#else 				// Linux Environment
	UINT32				pHWDescPhys;			// Physical address for this descriptor
//	struct scatterlist * pScatterGatherList;	// Pointer to the Scatter List for this set of descriptors
	PUINT8				SystemAddressVirt;		// User address for the SystemAddress
	PDMA_TRANSACTION_STRUCT	pDmaTrans;			// Pointer to the DMA Transaction for this buffer
#endif // Windows | Linux Conditional Compile.
} DRIVER_DESC_STRUCT, *PDRIVER_DESC_STRUCT;

/*! \note
 * The Zero-sized array pragma only applies to DRIVER_DESC_STRUCT.
 * Here, the driver is setting the warning back to default.
 */
#ifdef __WINNT__	// pragma Conditional Compile
#pragma warning(default:4200)  
#endif      		// Windows Driver Only

#ifndef __WINNT__   // Linux version ---------------------------------------------

static const UINT32  BAR_TYPE_MASK       = 0x1;
static const UINT32  BAR_TYPE_MEM        = 0x0;
static const UINT32  BAR_TYPE_IO         = 0x1;
static const UINT32  BAR_MEM_ADDR_MASK   = 0x6;
static const UINT32  BAR_MEM_ADDR_32     = 0x0;
static const UINT32  BAR_MEM_ADDR_1M     = 0x2;
static const UINT32  BAR_MEM_ADDR_64     = 0x4;
static const UINT32  BAR_MEM_CACHE_MASK  = 0x8;
static const UINT32  BAR_MEM_CACHABLE    = 0x8;

#define BAR_MEM_MASK        (~0x0F)
#define BAR_IO_MASK         (~0x03)
#define IS_IO_BAR(x)            (((x) & BAR_TYPE_MASK)      == BAR_TYPE_IO)
#define IS_MEMORY_BAR(x)        (((x) & BAR_TYPE_MASK)      == BAR_TYPE_MEM)
#define IS_MEMORY_32BIT(x)      (((x) & BAR_MEM_ADDR_MASK)  == BAR_MEM_ADDR_32)
#define IS_MEMORY_64BIT(x)      (((x) & BAR_MEM_ADDR_MASK)  == BAR_MEM_ADDR_64)
#define IS_MEMORY_BELOW1M(x)    (((x) & BAR_MEM_ADDR_MASK)  == BAR_MEM_ADDR_1M)
#define IS_MEMORY_CACHABLE(x)   (((x) & BAR_MEM_CACHE_MASK) == BAR_MEM_CACHABLE)

/* 
 * Bar Types
 */
#define REGISTER_MEM_PCI_BAR_TYPE  0
#define RAM_MEM_PCI_BAR_TYPE       1
#define REGISTER_IO_PCI_BAR_TYPE   2
#define DISABLED_PCI_BAR_TYPE      3

#define TRANSFER_SIZE_8_BIT        0x01    // 8 Bit transfer
#define TRANSFER_SIZE_16_BIT       0x02    // 16 Bit transfer
#define TRANSFER_SIZE_32_BIT       0x04    // 32 Bit transfer
#define TRANSFER_SIZE_64_BIT       0x08    // 64 Bit transfer (not currently supported)
#define TRANSFER_SIZE_32_BIT_DMA   0x10    // 32 Bit transfer
#define TRANSFER_SIZE_64_BIT_DMA   0x20    // 64 Bit transfer

#endif  // Linux version ---------------------------------------------

//! Register Structure located at BAR0
typedef struct _BAR0_REGISTER_MAP_STRUCT
{
	DMA_ENGINE_STRUCT			dmaEngine[MAX_NUM_DMA_ENGINES];	// Pointer to the base of the DMA Engines in BAR0 space
	DMA_COMMON_CONTROL_STRUCT	commonControl;					// Pointer to the common control struct in BAR0 space
} PACKED BAR0_REGISTER_MAP_STRUCT, *PBAR0_REGISTER_MAP_STRUCT;

typedef struct _THORDAQ_ADCS2MM_DMA_MAP_STRUCT
{
	BYTE S2MMDMA_ControlStatus; // if read, Status, if written, Control: 4 channels @ 0x0, 0x40, 0x80, 0xC0

} PACKED THORDAQ_ADCS2MM_DMA_MAP_STRUCT, *PTHORDAQ_ADCS2MM_DMA_MAP_STRUCT;



#pragma pack(pop)

#endif // _DMA_DRIVER_HW_H_
