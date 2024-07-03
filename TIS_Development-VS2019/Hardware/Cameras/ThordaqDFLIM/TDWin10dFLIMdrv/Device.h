/*++

Module Name:

    device.h

Abstract:

    This file contains the device definitions.

Environment:

    Kernel-mode Driver Framework

--*/


#pragma pack(push,1)

//#ifdef WIN32	// Windows Version
#pragma warning(disable:4201)  // nameless struct/union warning
//#endif 	// Windows Version

//#ifndef WIN32   // Linux version ---------------------------------------------
//
//#ifndef PACKED
//#    define PACKED                      __attribute__((packed))
//#endif /* PACKED */
//
//#else // Windows
//#ifndef PACKED
//#    define PACKED 
//#endif /* PACKED */

// DMA Defines
#define MAX_NUM_DMA_ENGINES		64
//#endif // Windows vs. Linux

#define DMA_BAR					0

#define	MSIX_MESSAGE_ID			0x11
#define	TABLE_BIR_MASK			0x07
#define	MSI_MESSAGE_ID			0x05

// DMA Descriptor Defines
//
// DMA descriptors must be on a 32 byte boundary
#define	DMA_NUM_DESCR			131200//32800 //65600 // //32768 //16384
//#define DMA_DESCR_MAX_LEN       (256 * 1024 * 1024)
#define DMA_DESCR_ALIGN_LEN     32
#define DMA_DESCR_ALIGN_REQUIREMENT	(FILE_32_BYTE_ALIGNMENT)
// Maximum number of bytes for number of descriptors (-1 for alignment)
#define DMA_MAX_TRANSFER_LENGTH ((DMA_NUM_DESCR-2) * PAGE_SIZE)
//#define DMA_MAX_TRANSACTION_LENGTH (256 * 1024 * 1024)

// The watchdog interval.  Any request pending for this number of seconds is
// cancelled.  This is a one second granularity timer, so the timeout must be
// at least one second more than the minimum allowed.
#define CARD_WATCHDOG_INTERVAL   5

//#ifndef _WIN32	// Linux Version
//
//#define DMA_DESCR_ALIGN_VIRT_ADDR(x) ((PDMA_DESCRIPTOR_STRUCT)((((unsigned long)(x)) + (DMA_DESCR_ALIGN_LEN - 1)) & ~(DMA_DESCR_ALIGN_LEN - 1)))
//#define DMA_DESCR_ALIGN_PHYS_ADDR(x) (((x) + (DMA_DESCR_ALIGN_LEN - 1)) & ~(DMA_DESCR_ALIGN_LEN - 1))
//
///*!
//*******************************************************************************
//*/
//typedef struct _DMA_TRANSACTION_STRUCT
//{
//	struct list_head        list;
//
//	size_t                  length;
//	uint64_t		       	CardOffset;
//	uint64_t		       	UserInfo;		// UserStatus or UserControl
//	int                     offset;
//	int                     numPages;
//	struct page **          pages;
//	int                     direction;
//	int						status; 
//	uint32_t				PacketStatus;
//	uint32_t				BytesTransfered;
//
//	uint16_t				DMAAvailable;
//	uint16_t                DMAComplete;
//
//	wait_queue_head_t       waiting;
//	wait_queue_head_t       completed;
//
//	struct scatterlist *    pSgList;
//	int                     SgNumElements;
//} DMA_TRANSACTION_STRUCT, *PDMA_TRANSACTION_STRUCT;
//
//// Interrupt status defines
//#define IRQ_DMA_COMPLETE(x) 	(uint64_t) (0x00000001 << (x))
//
//#endif // Linux Version


/*!
*******************************************************************************
** Board Register set.
*  DMA Common Control Structure
*/
typedef struct _DMA_COMMON_CONTROL_STRUCT
{
	ULONG32		ControlStatus;
	ULONG32		DMABackEndCoreVersion;
	ULONG32		PCIExpressCoreVersion;
	ULONG32		UserVersion;
} DMA_COMMON_CONTROL_STRUCT, *PDMA_COMMON_CONTROL_STRUCT;

//
// DMA Common ControlStatus defs
#define CARD_IRQ_ENABLE             (1 << 0)
#define CARD_IRQ_ACTIVE             (1 << 1)
#define CARD_IRQ_PENDING            (1 << 2)
#define CARD_IRQ_MSI                (1 << 3)
#define CARD_USER_INTERRUPT_MODE	(1 << 4)
#define CARD_USER_INTERRUPT_ACTIVE	(1 << 5)
#define CARD_IRQ_MSIX_MODE		    (1 << 6)
#define CARD_MAX_PAYLOAD_SIZE_MASK		0x0700
#define CARD_MAX_READ_REQUEST_SIZE_MASK	0x7000
#define CARD_S2C_INTERRUPT_STATUS_MASK	0x00FF0000
#define CARD_C2S_INTERRUPT_STATUS_MASK	0xFF000000

#define CARD_MAX_C2S_SIZE(x)        (1UL << ((((x) >> 8) & 0xFF) + 7))
#define CARD_MAX_S2C_SIZE(x)        (1UL << ((((x) >> 12) & 0xFF) + 7))

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
//
//  Packet Mdoe DMA Descriptor (C2S) Read
typedef struct _PACKET_DESCRIPTOR_C2S_STRUCT
{
	// Hardware specific entries - Do not change or reorder
	ULONG32		StatusFlags_BytesCompleted;		// C2S StatusFlags and Bytes count completed       
	ULONG64		UserStatus;                     // C2S User Status - Passed from Users design
	ULONG32		CardAddress;                    // C2S Card Address (offset) Not used in FIFO Mode 
	ULONG32		ControlFlags_ByteCount;         // C2S Control Flags and Bytes Count to transfer   
	ULONG64 	SystemAddressPhys;              // C2S System Physical Buffer Address to transfer  
	ULONG32		NextDescriptorPhys;             // C2S Physical Address to Next DMA Descriptor     
	// Software specific entries
//#ifdef _WIN32  /* Windows */
	PSCATTER_GATHER_LIST pScatterGatherList;	// Pointer to the Scatter List for this set of descriptors
	PVOID *				SystemAddressVirt;		// User address for the SystemAddress
	WDFDMATRANSACTION 		DmaTransaction;		// Contains the DMA Transaction associated to this descriptor
//#else /* Linux */
//	PDMA_TRANSACTION_STRUCT	pDmaTrans;			// Pointer to the DMA Transaction for this buffer
//	//	struct scatterlist * pScatterGatherList;	// Pointer to the Scatter List for this set of descriptors
//	UCHAR *				SystemAddressVirt;		// User address for the SystemAddress
//#endif /* Windows vs Linux */
} PACKET_DESCRIPTOR_C2S_STRUCT, *PPACKET_DESCRIPTOR_C2S_STRUCT;


#define PACKET_DESC_S2C_CTRL_IRQ_ON_COMPLETE	0x01000000
#define PACKET_DESC_S2C_CTRL_IRQ_ON_ERROR		0x02000000
#define PACKET_DESC_S2C_CTRL_FIFO_OVERRIDE		0x04000000
#define PACKET_DESC_S2C_CTRL_END_OF_PACKET		0x40000000
#define PACKET_DESC_S2C_CTRL_START_OF_PACKET	0x80000000

#define PACKET_DESC_S2C_STAT_COMPLETE			0x01000000
#define PACKET_DESC_S2C_STAT_SHORT				0x02000000
#define PACKET_DESC_S2C_STAT_ERROR				0x10000000
//
//  Packet Mdoe DMA Descriptor (S2C) Write
typedef struct _PACKET_DESCRIPTOR_S2C_STRUCT
{
	// Hardware specific entries - Do not change or reorder
	ULONG32		StatusFlags_BytesCompleted;		// S2C StatusFlags and Bytes count completed
	ULONG64		UserControl;					// S2C User Control - passed to Users design
	ULONG32		CardAddress;					// S2C Card Address (offset) Not used in FIFO Mode
	ULONG32		ControlFlags_ByteCount;			// S2C Control Flags and Bytes Count to transfer
	ULONG64 	SystemAddressPhys;				// S2C System Physical Buffer Address to transfer
	ULONG32		NextDescriptorPhys;				// S2C Physical Address to Next DMA Descriptor
	// Software specific entries
//#ifdef _WIN32
	WDFDMATRANSACTION 		DmaTransaction;		// Contains the DMA Transaction associated to this descriptor
//#else  /* Linux */
//	PDMA_TRANSACTION_STRUCT pDmaTransaction;	// Pointer the DMA Transaction struct
//#endif /* Windows vs. Linux */
} PACKET_DESCRIPTOR_S2C_STRUCT, *PPACKET_DESCRIPTOR_S2C_STRUCT;

#define	DESC_FLAGS_SIG							0x6db6AA00
#define	DESC_FLAGS_SIG_MASK						0xFFFFFF00
#define	DESC_FLAGS_MASK							0x000000FF
#define	DESC_FLAGS_HW_OWNED						0x00000001
#define	DESC_FLAGS_SW_OWNED						0x00000002
#define	DESC_FLAGS_SW_FREED						0x00000004
#define	DESC_FLAGS_ADDRESSABLE_MODE				0x00000008

typedef struct _PACKET_DMA_DESCRIPTOR_STRUCT
{
	union
	{
		PACKET_DESCRIPTOR_S2C_STRUCT	S2C;	// HW System To Card Descriptor components
		PACKET_DESCRIPTOR_C2S_STRUCT	C2S;	// HW Card To System Descriptor components
	}; // Direction;								// Direction specific Hardware descriptor
	ULONG32				DescriptorNumber;		// "Token" for this descriptor
	ULONG32				DescFlags;				// Flags for this decriptor
	PVOID *				pNextDescriptorVirt;	// Driver usable pointer to the next descriptor
//#ifdef _WIN32 /* Windows */
	PHYSICAL_ADDRESS	pDescPhys;				// Physical address for this descriptor
//#else /* Linux */
//	ULONG32				pDescPhys;				// Physical address for this descriptor
//#endif /* Windows vs. Linux */
} PACKET_DMA_DESCRIPTOR_STRUCT, *PPACKET_DMA_DESCRIPTOR_STRUCT;

//
// DMA Engine Control Structure
typedef struct _DMA_ENGINE_STRUCT
{
	ULONG32		Capabilities;  					// Common Capabilities
	volatile ULONG32 ControlStatus;				// Common Control and Status
	union
	{
		struct
		{
			ULONG64		Descriptor;				// Block Mode Descriptor pointer
			ULONG32		HardwareTime;			// Block Mode Hardware timer
			ULONG32		ChainCompleteByteCount;	// Block Mode Completed Byte Count
		} Block;
		struct
		{
			ULONG32		NextDescriptorPtr;		// Packet Mode Next Descriptor Pointer (Physical)
			ULONG32		SoftwareDescriptorPtr;	// Packet Mode Software Owned Descriptor pointer (Physical)
			ULONG32		CompletedDescriptorPtr;	// Packet Mode Completed Packet Descriptor Pointer (Physical)
			ULONG32		DMAActiveTime;			// Amount of time the DMA was active / second (4ns resolution)
			ULONG32		DMAWaitTime;			// Amount of time the DMA was inactive / second (4ns resolution)
			ULONG32		DMACompletedByteCount;	// Amount DMA byte transfers / second (4 byte resolution)
			ULONG32 	InterruptControl;		// New Firmware Interrupt Control register
		}Packet;
	};
	UCHAR		Reserved[0x100-(9*sizeof(ULONG32))];	// Reserved size
} DMA_ENGINE_STRUCT, *PDMA_ENGINE_STRUCT;

//#ifdef _X86_
//#define	RESERVED_SIZE	64					// Assume a 32 bit machine
//#else
#define	RESERVED_SIZE	128					// 64 bit pointers increase the size beyond 64.
//#endif

/*!
*******************************************************************************
** Descriptor engine register set.
*/

#   define      DMA_CTRL_IRQ_ENABLE                    (1 << 0)
#   define      DMA_CTRL_IRQ_ACTIVE                    (1 << 1)
#   define      DMA_CTRL_CHAIN_START                   (1 << 8)
#   define      DMA_CTRL_CHAIN_STOP                    (1 << 9)
#   define      DMA_STAT_CHAIN_RUNNING                 (1 << 10)
#   define      DMA_STAT_CHAIN_COMPLETE                (1 << 11)
#   define      DMA_STAT_CHAIN_SHORT_ERR               (1 << 12)
#   define      DMA_STAT_CHAIN_SHORT_SW                (1 << 13)
#   define      DMA_STAT_CHAIN_SHORT_HW                (1 << 14)

//#ifdef WIN32	// Windows Version
#pragma warning(disable:4200)  // Zero sized array
//#endif      	// Windows Version

//
// DMA_DESCRIPTOR_MAX_SIZE is used only for a size reference when allocating space
//  for the descriptor pool
typedef struct _DMA_DESCRIPTOR_STRUCT
{
	PACKET_DMA_DESCRIPTOR_STRUCT	Packet;
	UCHAR	reserved[RESERVED_SIZE - sizeof(PACKET_DMA_DESCRIPTOR_STRUCT)];
} DMA_DESCRIPTOR_STRUCT, *PDMA_DESCRIPTOR_STRUCT;

//#ifdef WIN32	// Windows Version
#pragma warning(default:4200)  // Zero sized array
//#endif      	// Windows Version

#define	PACKET_GENERATOR_ENGINE_OFFSET	0x0000a000

  //Packet Generator Structure - Information for the Packet Generator
typedef struct _PACKET_GENENRATOR_STRUCT
{
	ULONG32		Control;			// Packet Generator Control DWORD
	ULONG32		NumPackets;			// Count of packet to generate, 0 = infinite
	ULONG32		DataSeed;			// Data Seed pattern
	ULONG32		UserCtrlStatSeed;	// Seed for the User Control/Status fields
	ULONG32		reserved[4];		// reserved
	ULONG32 	PacketLength[4];	// Packet Length array
	UCHAR		Reserved[0x100-(12*sizeof(ULONG32))];	// Reserved size
} PACKET_GENENRATOR_STRUCT, *PPACKET_GENENRATOR_STRUCT;

//
// Register Structure located at BAR0
typedef struct _BAR0_REGISTER_MAP_STRUCT
{
	DMA_ENGINE_STRUCT		dmaEngine[MAX_NUM_DMA_ENGINES];	// Pointer to the base of the DMA Engines in BAR0 space
	DMA_COMMON_CONTROL_STRUCT	commonControl;		// Pointer to the common control struct in BAR0 space
	UCHAR					Reserved[PACKET_GENERATOR_ENGINE_OFFSET - 
		((sizeof(DMA_ENGINE_STRUCT) * MAX_NUM_DMA_ENGINES) + 
		sizeof(DMA_COMMON_CONTROL_STRUCT))];	// Reserved size
	PACKET_GENENRATOR_STRUCT	packetGen[MAX_NUM_DMA_ENGINES];
} BAR0_REGISTER_MAP_STRUCT, *PBAR0_REGISTER_MAP_STRUCT;

//************************ BAR 1 ********************************
#define INNER_MEM_SIZE_PER_CHANNEL 0x8000000
#define S2MM_DESCRS_PER_BLK		2
#define MAX_S2MM_BLKS_PER_CHANNEL	1024
#define S2MM_CHANNEL_PER_BOARD	 6
#define DAC_DESCRS_PER_BOARD 4096
#define DFLIM_PROCESSING_STRUCTS_NUM 4

//Thorlabs S2MM packet dma descriptor structure
typedef struct _S2MM_DMA_DESCRIPTOR_STRUCT
{
	ULONG32		NxtDescPtr;		// first 6 bits are reserved, the rest 26 bit is used as pointer
	ULONG32		PtrRsvd;		// not use
	ULONG32		BuffAddr;		// 32-bit address of on_board_memory (SDRAM)
	ULONG32		AddrRsvd;		// not use
	ULONG32		Usr_Cache;		// the first 3 bytes are reserved, the last 4-bit for User and the rest 4-bit for Cache.
	ULONG32		VSize_Stride;	// the first 2 bytes for Stride and last 13 bits for VSize
	ULONG32		HSize;			// the first 2 bytes for HSize, and the rest reserved
	ULONG32		DescFlags;		// Flags for this decriptor

} S2MM_DMA_DESCRIPTOR_STRUCT, *PS2MM_DMA_DESCRIPTOR_STRUCT;

typedef struct _S2MM_DESC_BLK_STRUCT
{
	S2MM_DMA_DESCRIPTOR_STRUCT desc[S2MM_DESCRS_PER_BLK];
} S2MM_DESC_BLK_STRUCT, *PS2MM_DESC_BLK_STRUCT;

typedef struct _S2MM_DESC_CHANNEL_STRUCT
{
	S2MM_DESC_BLK_STRUCT descBlk[MAX_S2MM_BLKS_PER_CHANNEL];
} S2MM_DESC_CHANNEL_STRUCT, *PS2MM_DESC_CHANNEL_STRUCT;

// Register Structure located at BAR1
typedef struct _BAR1_DESCRIPTOR_MAP_STRUCT
{
	S2MM_DESC_CHANNEL_STRUCT descCh[S2MM_CHANNEL_PER_BOARD];
} BAR1_DESCRIPTOR_MAP_STRUCT, *PBAR1_DESCRIPTOR_MAP_STRUCT;

//************************ BAR 2 ********************************
#define BAR2_BYTES_PER_CHANNEL	64

//Thorlabs S2MM packet dma control and status structure
typedef struct _S2MM_CTRL_STAT_STRUCT
{
	USHORT		SR0_CR0;			// first 8 bits for control, and the next 8 bits for status
	USHORT		ChainStartAddr;		// Address of BRam, pointing to the head of chain.
	USHORT		ChainTailAddr;		// Address of BRam, pointing to the tail of chain.
	USHORT		ChainIrqThreshold;	// for MSI of chain switch.
} S2MM_CTRL_STAT_STRUCT, *PS2MM_CTRL_STAT_STRUCT;

typedef struct _S2MM_CTRL_CHANNEL_STRUCT
{
	S2MM_CTRL_STAT_STRUCT	ctrl;
	UCHAR					rsvd[BAR2_BYTES_PER_CHANNEL - sizeof(S2MM_CTRL_STAT_STRUCT)];
} S2MM_CTRL_CHANNEL_STRUCT, *PS2MM_CTRL_CHANNEL_STRUCT;

// Register Structure located at BAR2
typedef struct _BAR2_CONTROL_MAP_STRUCT
{
	S2MM_CTRL_CHANNEL_STRUCT ctrlCh[S2MM_CHANNEL_PER_BOARD];
} BAR2_CONTROL_MAP_STRUCT, *PBAR2_CONTROL_MAP_STRUCT;

//************************ BAR 3 ********************************
#define BAR3_BYTES_PER_SECTION	64
// Thorlab packet generator structure
typedef struct _PACKET_GLOBAL_GEN_CTRL_STRUCT
{
	struct _LAYOUT_GBL_GEN
	{
		ULONG		StopRun_FpgaRev;			// (was UCHAR) Write only bit0 for stop(0) and run(1), the next 4 bits are for debug 
	//	UCHAR		r1;
	//	USHORT		FpgaProgStatus;     // was "FpgaProgStatus"
		ULONG		testDiag;  // was UCHAR[4]  r2
		UCHAR		ImgSyncCtrl;  // 0x08
		UCHAR		r3[7];
		USHORT		ImgHSize;     // 0x10
		UCHAR		r4[6];
		USHORT		ImgVSize;     // 0x18
		UCHAR		r5[6];
		ULONG64     GPIOConfig;  // 0x20
	} Layout;
	UCHAR		rsvd[BAR3_BYTES_PER_SECTION - sizeof(struct _LAYOUT_GBL_GEN)];
} PACKET_GLOBAL_GEN_CTRL_STRUCT, *PPACKET_GLOBAL_GEN_CTRL_STRUCT;

typedef struct _PACKET_CHANNEL_GEN_CTRL_STRUCT
{
	struct _LAYOUT_CH_GEN
	{
		ULONG32		BeatsPerFrame;			// Write only, valid range 0~(2^26-1)
											// BeatsPerFrame=(HSize*VSize)/4
		ULONG32		IntraBeatDelay_HSize;	// both valid range 0 ~ (2^16-1)
		ULONG32		IntraFrameDelay;
	} Layout;

	UCHAR		rsvd[BAR3_BYTES_PER_SECTION - sizeof(struct _LAYOUT_CH_GEN)];

} PACKET_CHANNEL_GEN_CTRL_STRUCT, *PPACKET_CHANNEL_GEN_CTRL_STRUCT;

typedef struct _SCANNING_SUBSYS_REG_STRUCT
{
	struct  _LAYOUT_SCAN
	{
		USHORT	SyncCtrl_ResScanPeriod;	// offset 0x140
		USHORT	FrameCnt;               // 0x142
		UCHAR	r1[4];
		UCHAR	AdpllCtrl;              // 0x148  (aka "pll_sync_offset" or "alignmentOffset")
		UCHAR	r2[7];
		USHORT	SyncOffset;				// 0x150
		USHORT  AdpllPhaseOffset;
		UCHAR	r3[4];
		ULONG32	CenterFreq;
		UCHAR	r4[4];
		ULONG32 GalvoPixelDwell;		// offset 0x20
		UCHAR	r5[4];
		ULONG32 GalvoPixelDelay;
		UCHAR   r6[4];
		ULONG32 IntraLineDelay;			// offset 0x30
		UCHAR	r7[4];
		ULONG32 IntraFrameDelay;
	} Layout;
	UCHAR rsvd[BAR3_BYTES_PER_SECTION - sizeof(struct _LAYOUT_SCAN)];
} SCANNING_SUBSYS_REG_STRUCT, *PSCANNING_SUBSYS_REG_STRUCT;

typedef struct _SAMPLING_CLOCK_GEN_SUBSYS_REG_STRUCT
{
	struct _LAYOUT_CLOCK
	{	
		UCHAR	CtrlReg;			// offset 0x180
		UCHAR	r1[7];
		USHORT	PhaseOffset;		// 0x188
		UCHAR   r2[6];
		UCHAR	PhaseStep;			// 0x190
		UCHAR	r3[7];
		USHORT	PhaseLimit;
	} Layout;

	UCHAR	rsvd[BAR3_BYTES_PER_SECTION - sizeof(struct _LAYOUT_CLOCK)];
} SAMPLING_CLOCK_GEN_SUBSYS_REG_STRUCT, *PSAMPLING_CLOCK_GEN_SUBSYS_REG_STRUCT;

typedef struct _STREAM_PROCESSING_SUBSYS_REG_STRUCT
{
	struct _LAYOUT_STREAM  // NOTE!  Registers 0x1C0-1CF conflict with dFLIM Front End Settings interface
	{	
		UCHAR	CtrlReg;				// offset 0x1C0
		UCHAR   PulseInterleave;
		UCHAR	r1[6];					
		USHORT	ScanPeriod;             // 0x1C8
		UCHAR	r2[6];
		ULONG32	DownsampleRate;			// offset 0x10
		UCHAR	r3[4];
		ULONG64	DC_OFFSET_LOW;			// offset 0x18
		ULONG64 DC_OFFSET_HIGH;         // offset 0x20
		USHORT	FirCoefficient;			// offset 0x28
		UCHAR	r4[6];
		ULONG32 PulseInterleaveOffset;  // offset 0x30
		UCHAR	r5[4];
		ULONG32 ThreePhotonSampleOffset;// offset 0x38
	} Layout;

	UCHAR	rsvd[BAR3_BYTES_PER_SECTION - sizeof(struct _LAYOUT_STREAM)];
} STREAM_PROCESSING_SUBSYS_REG_STRUCT, *PSTREAM_PROCESSING_SUBSYS_REG_STRUCT;

typedef struct _ADC_INTERFACE_CTRL_STRUCT
{
	struct _LAYOUT_ADC
	{
		USHORT		CtrlReg;			// Write only bit0 for stop(0) and run(1), the next 4 bits are for debug 
		UCHAR		r1[6];
		UCHAR		Gain12;
		UCHAR		r2[7];
		UCHAR		Gain34;
	}Layout;
	UCHAR		rsvd[BAR3_BYTES_PER_SECTION - sizeof(struct _LAYOUT_ADC)];
} ADC_INTERFACE_CTRL_STRUCT, *PADC_INTERFACE_CTRL_STRUCT;

typedef struct _GALVO_WAVEFORM_GEN_SUBSYSTEM_REG_STRUCT1
{
	struct _LAYOUT_GALVO1 //0x240
	{
		ULONG64 CtrlReg;              // offset 0x240     include the Ctrl and UpdateTimeOffset    
		ULONG64 DACUpdateRate[3];     // offset 0x248
		USHORT  DACAmplitude;         // offset 0x260
		USHORT  DACStepSize[3];       // offset 0x262
		ULONG64 DACParkValue[3];      // offset 0x268
	} Layout;

}GALVO_WAVEFORM_GEN_SUBSYSTEM_REG_STRUCT1, *PGALVO_WAVEFORM_GEN_SUBSYSTEM_REG_STRUCT1;

typedef struct _GALVO_WAVEFORM_GEN_SUBSYSTEM_REG_STRUCT2
{
	struct _LAYOUT_GALVO2 //0x280
	{
		ULONG64 DACOffset[3];            // offset 0x00
		ULONG64 DACChannelMap;         // offset 0x18
		ULONG64 DOUpdateRate;          // offset 0x20
		ULONG64 DOParkValue;           // offset 0x20
		ULONG64 DOOffset;              // offset 0x20
		ULONG64 DACAmFilterWindow;
	} Layout;
}GALVO_WAVEFORM_GEN_SUBSYSTEM_REG_STRUCT2, *PGALVO_WAVEFORM_GEN_SUBSYSTEM_REG_STRUCT2;

typedef struct _I2C_SUBSYS_REG_STRUCT
{
	struct _LAYOUT_I2C //0x2C0
	{
		ULONG64 CmdReg;             // offset 0x00
		ULONG32 CtrlReg;            // offset 0x08
		UCHAR	r1[4];
		ULONG64 DataReg1;           // offset 0x10
		ULONG64 DataReg2;           // offset 0x18
	} Layout;
	UCHAR rsvd[BAR3_BYTES_PER_SECTION - sizeof(struct _LAYOUT_I2C)]; //0x2C0
}I2C_SUBSYS_REG_STRUCT,*PI2C_SUBSYS_REG_STRUCT;

//typedef struct _DFLIM_FRONTEND_SUBSYS_REG_STRUCT
//{
//	struct _LAYOUT_DFLIM_FRONTEND //0x1c0
//	{
//		ULONG64 GP0;             // offset 0x00
//		ULONG32 CtrlReg;            // offset 0x08
//		ULONG64 DataReg1;           // offset 0x10
//		ULONG64 DataReg2;           // offset 0x18
//	} Layout;
//	UCHAR rsvd[BAR3_BYTES_PER_SECTION - sizeof(struct _LAYOUT_DFLIM_FRONTEND)]; //0x2C0
//}DFLIM_FRONTEND_SUBSYS_REG_STRUCT,*PDFLIM_FRONTEND_SUBSYS_REG_STRUCT;
//
typedef struct _DFLIM_PROCESSING_SUBSYS_REG_STRUCT
{
	struct _LAYOUT_DFLIM_PROCESSING //0x300
	{
		ULONG64 GP0Reg0;             // offset 0x00
		ULONG64 GP0Reg1;             // offset 0x08
		ULONG32 GP0Reg2;             // offset 0x10
		UCHAR	r1[4];
		ULONG64 GP0Reg3;            // offset 0x18
		ULONG32 GP0Reg4;            // offset 0x20
		UCHAR	r2[4];
		ULONG32 GP0Reg5_1;            // offset 0x28
		ULONG32 GP0Reg5_2;            // offset 0x2C
		ULONG64 GP0Reg6;           // offset 0x30
		ULONG64 GP0Reg7;           // offset 0x38
	} Layout;
	//UCHAR rsvd[BAR3_BYTES_PER_SECTION - sizeof(struct _LAYOUT_DFLIM_PROCESSING)]; //0x2C0
}DFLIM_PROCESSING_SUBSYS_REG_STRUCT,*PDFLIM_PROCESSING_SUBSYS_REG_STRUCT;

typedef struct _TEMP_OCCUPIED
{
	UCHAR r[112 * BAR3_BYTES_PER_SECTION];

} TEMP_OCCUPIED, *PTEMP_OCCUPIED;

typedef struct _IMG_LUT_STRUCT
{
	UCHAR t[0x2000];
} IMG_LUT_STRUCT, *PIMG_LUT_STRUCT;

typedef struct _TEMP_OCCUPIED1
{
	UCHAR r[0x2000];
} TEMP_OCCUPIED1, *PTEMP_OCCUPIED1;

typedef struct _DAC_DESCRIPTOR_STRUCT
{
	ULONG64 Descriptor;
}DAC_DESCRIPTOR_STRUCT, *PDAC_DESCRIPTOR_STRUCT;

// Register Structure located at BAR3
typedef struct _BAR3_MAP_STRUCT
{
	PACKET_GLOBAL_GEN_CTRL_STRUCT			globalGenCtrl;  // 0x0
	PACKET_CHANNEL_GEN_CTRL_STRUCT			channelGenCtrl[4];  // used??
	SCANNING_SUBSYS_REG_STRUCT				scanningRegs; //0x140
	SAMPLING_CLOCK_GEN_SUBSYS_REG_STRUCT	samplingClockGenRegs;
	STREAM_PROCESSING_SUBSYS_REG_STRUCT		streamProcessingRegs;
	ADC_INTERFACE_CTRL_STRUCT				adcInterfaceCtrl;
	GALVO_WAVEFORM_GEN_SUBSYSTEM_REG_STRUCT1 galvoWaveformGenRegs1;
	GALVO_WAVEFORM_GEN_SUBSYSTEM_REG_STRUCT2 galvoWaveformGenRegs2;
	I2C_SUBSYS_REG_STRUCT                   i2cRegs;
	DFLIM_PROCESSING_SUBSYS_REG_STRUCT		dflimProcessingRegs[DFLIM_PROCESSING_STRUCTS_NUM];
	TEMP_OCCUPIED							tmpOccupied;
	IMG_LUT_STRUCT							imgLutTable[S2MM_CHANNEL_PER_BOARD]; // was 6 now 4
//	IMG_LUT_STRUCT                          unusedLutTable[2];
	TEMP_OCCUPIED1                          tempOccupied1;
	DAC_DESCRIPTOR_STRUCT                   dacDescriptor[DAC_DESCRS_PER_BOARD];
} BAR3_MAP_STRUCT, *PBAR3_MAP_STRUCT;


//************************ S2mm Layer ********************************
#define MAX_INTERRUPTS_BUFFERRED	1024

#pragma pack(pop)


//
// The device context performs the same job as
// a WDM device extension in the driver frameworks
//
typedef struct _DEVICE_CONTEXT
{
    ULONG PrivateDeviceData;  // just a placeholder

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

//
// This macro will generate an inline function called DeviceGetContext
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

//
// Function to initialize the device and its callbacks
//
//NTSTATUS
//ThorDaqDrvCreateDevice(
//    _Inout_ PWDFDEVICE_INIT DeviceInit
//    );

