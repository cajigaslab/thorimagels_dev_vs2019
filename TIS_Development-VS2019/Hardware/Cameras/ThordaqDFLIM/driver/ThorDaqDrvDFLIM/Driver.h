/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#define INITGUID

#include <ntddk.h>
#include <wdf.h>
#include <wdmguid.h>

#include "public.h"
#include "device.h"
#include "IoCtl.h"
#include "Version.h"
//#define TRACE_ENABLED			0

//#define IRQL_CALLBACK			1

#define MAX_NUMBER_OF_BARS 		6

#define DMA_TYPE_PACKET_SEND	0x02
#define DMA_TYPE_PACKET_RECV	0x04
#define DMA_TYPE_PACKET_WRITE	0x02
#define DMA_TYPE_PACKET_READ	0x04

// MSI-X Capability Defines
#define	MSG_CTRL_MSIX_ENABLE		0x8000
#define	MSG_CTRL_FUNC_MASK_VECTORS	0x4000
#define MSG_CTRL_TABLE_SIZE_MASK	0x03FF

#define MSIX_TABLE_OFFSET_MASK		0xFFFFFFF8
#define MSIX_TABLE_BIR_MASK			0x00000007

#define PBA_OFFSET_MASK				0xFFFFFFF8
#define PBA_BIR_MASK				0x00000007

//half second MSI
#define ACQ_SINGLE_CHANNEL_BUF_CAP  0x04000000
//#define _NELEM(arr)                 (sizeof(arr) / sizeof(arr[0]))
//#ifdef CONFIG_X86_64				
//#define _OFFSETOF(t,m)              ((ULONGLONG) &((t *)0)->m)
//#else
//#define _OFFSETOF(t,m)              (&((t *)0)->m)
//#endif

typedef struct _DMA_XFER  {
	WDFREQUEST		Request;
	SIZE_T			bytesTransferred;
	ULONG64 		CardAddress;
	ULONG64 		UserControl;
	PMDL			pMdl;
	ULONG32			Mode;
	ULONG32			PacketStatus;
} DMA_XFER, *PDMA_XFER;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DMA_XFER, DMAXferContext)


/* The device extension for a DMA Engine */

typedef struct _DMA_ENGINE_DEVICE_EXTENSION  {
	UCHAR					DmaEngine;
	UCHAR					DmaType;			// Block or Packet
	WDF_DMA_DIRECTION		DmaDirection;
	ULONG32					NumberOfDescriptors;
	LONG					NumberOfUsedDescriptors;
	ULONG32					DMAEngineMSIVector;
	size_t					MaximumTransferLength;

	WDFCOMMONBUFFER			DescCommonBuffer;
	WDFQUEUE 				TransactionQueue;
	WDFDMAENABLER			DmaEnabler;
	WDFDMAENABLER			DmaEnabler32BitOnly;
	WDFDMATRANSACTION		DmaTransaction;
	WDFREQUEST				DmaRequest;
	WDFDPC					CompletionDpc;

	PDMA_DESCRIPTOR_STRUCT	pDescriptorBase;
	PHYSICAL_ADDRESS		pDescriptorBasePhysical;	// Shared for both Block and Packet Modes
	PDMA_ENGINE_STRUCT		pDmaEng;			// Shared for both Block and Packet Modes

	WDFSPINLOCK				HeadSpinLock;		// For protecting the Head of the queue
	WDFSPINLOCK				TailSpinLock;		// For protecting the Tail of the queue
	PDMA_DESCRIPTOR_STRUCT	pNextDescVirt;		// Pointer to the front of the queue
	PHYSICAL_ADDRESS		pNextDescPhys;		// Used to Init the Rx Descriptors
	PDMA_DESCRIPTOR_STRUCT	pTailDescVirt;		// Pointer to the back of the queue

	PDMA_ADAPTER			pReadDmaAdapter;

    UCHAR					TimeoutCount;
	UCHAR					bAddressablePacketMode;
	BOOLEAN					bDescriptorAllocSuccess;
	ULONG32					PacketMode;

	BOOLEAN					bFreeRun;
	USHORT					DMAEngineStatus;

	// Performance counters
	ULONG64					BytesInLastSecond;
	ULONG64	 		   		BytesInCurrentSecond;
	ULONG64					HardwareTimeInLastSecond;
	ULONG64					HardwareTimeInCurrentSecond;
	ULONG64					DMAInactiveTime;
	ULONG64					IntsInLastSecond;
	ULONG64					DPCsInLastSecond;

	PVOID              		UserVa;     		// Mapped VA for the process
	PMDL               		PMdl;               // MDL used to map memory
} DMA_ENGINE_DEVICE_EXTENSION, *PDMA_ENGINE_DEVICE_EXTENSION;


typedef struct _S2MM_ENGINE_EXTENSION  {
	UCHAR			IsDataReady;
	WDFSPINLOCK		InterReadLock;
} S2MM_ENGINE_EXTENSION, *PS2MM_ENGINE_EXTENSION;

typedef struct _S2MM_LAYER_EXTENSION  {
	S2MM_ENGINE_EXTENSION	pS2mm[S2MM_CHANNEL_PER_BOARD];
	ULONG   AcqBufOffset;
	USHORT  OffsetsIndexHead;
	USHORT  OffsetsIndexTail;
	USHORT  IndexCap;
	UCHAR  BankHead;
	UCHAR BankTail;
	UCHAR  IsFull;
	HANDLE					BramOverflowEventHandle;
	PKEVENT					BramOverflowEvent;					
	WDFDPC					CompletionDpc;
	//UCHAR					RequestServingDMAIndex[4];
} S2MM_LAYER_EXTENSION, *PS2MM_LAYER_EXTENSION;

/* The device extension for the device object */

typedef struct _DEVICE_EXTENSION  {
	WDFDEVICE				Device;
	// IOCTL Queue
	WDFQUEUE				IoctlQueue;
	// Interrupt Object
	WDFINTERRUPT			Interrupt[MAX_NUM_DMA_ENGINES];

	// Board Configuration Structure
	BOARD_CONFIG_STRUCT		BoardConfig;

	// PCI Resources
	BUS_INTERFACE_STANDARD	BusInterface;		// PCI Bus interface
	UCHAR					NumberOfBARS;
	BOOLEAN					Use64BitAddresses;
    BOOLEAN					MSISupported;
	BOOLEAN					MSIXSupported;
	UCHAR					BarType[MAX_NUMBER_OF_BARS];
	PHYSICAL_ADDRESS		BarPhysicalAddress[MAX_NUMBER_OF_BARS];
	ULONG64					BarLength[MAX_NUMBER_OF_BARS];
	PVOID					BarVirtualAddress[MAX_NUMBER_OF_BARS];

	// DMA Resources
	size_t							MaximumDmaTransferLength;
	PBAR0_REGISTER_MAP_STRUCT		pDmaRegisters;
	PDMA_ENGINE_DEVICE_EXTENSION 	pDmaEngineDevExt[MAX_NUM_DMA_ENGINES];
	PDMA_ENGINE_DEVICE_EXTENSION 	pDmaExtMSIVector[MAX_NUM_DMA_ENGINES];

	PS2MM_LAYER_EXTENSION			pS2mmLayerExt;
	PBAR1_DESCRIPTOR_MAP_STRUCT		pBar1Descriptors;
	PBAR2_CONTROL_MAP_STRUCT		pBar2Controls;
	PBAR3_MAP_STRUCT				pBar3Map;
	ULONG32							VSize;
	ULONG32							HSize;	
	ULONG32							dataHSize;
	ULONG32							linesPerStripe;
	ULONG32							FrameSize;
	USHORT							FrameRate;
	USHORT							FramesPerTransfer;
	USHORT							Channel;
	ULONG32                         DataBufferStartAddress;//acq buffer offset
	ULONG32                         DataChannelLength; // single channel acq buffer length
	ULONG32                         DataInterruptLength;//bank 0/1 length
	BOOLEAN                         ChannelDescriptor[S2MM_CHANNEL_PER_BOARD];

	PDEVICE_OBJECT			PhysicalDeviceObject;
	PDEVICE_OBJECT			FunctionalDeviceObject;

	ULONG32					NumIRQVectors;
	ULONG32					NumberDMAEngines;
	ULONG32					MSINumberVectors;
	// Watchdog timer Resources
	WDFTIMER				WatchdogTimer;

	ULONG32					InterruptMode;
	ULONG32					mode;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_EXTENSION, ThorDaqDrvGetDeviceContext)

/* The device extension for the device object */
typedef struct _DPC_CTX  {
	union
	{
		PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;
		PS2MM_LAYER_EXTENSION			pS2mmExt;
	};
	
} DPC_CTX, *PDPC_CTX;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DPC_CTX, DPCContext)

/* The queue extension to point to the DMA Engine */
typedef struct _QUEUE_CTX  {
	PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt;
} QUEUE_CTX, *PQUEUE_CTX;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_CTX, QueueContext)

// InterruptStatus defines
#define IRQ_DMA_COMPLETE(x) ((ULONGLONG) 0x0001 << (x))

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;

EVT_WDF_DRIVER_DEVICE_ADD ThorDaqDrvEvtDeviceAdd;
EVT_WDF_DEVICE_CONTEXT_CLEANUP ThorDaqDrvEvtDriverContextCleanup;
EVT_WDF_DEVICE_PREPARE_HARDWARE ThorDaqDrvEvtDevicePrepareHardware ;
EVT_WDF_DEVICE_RELEASE_HARDWARE ThorDaqDrvEvtDeviceReleaseHardware;
EVT_WDF_DEVICE_D0_ENTRY ThorDaqDrvEvtDeviceD0Entry;
EVT_WDF_DEVICE_D0_EXIT ThorDaqDrvEvtDeviceD0Exit;
EVT_WDF_DEVICE_SELF_MANAGED_IO_RESTART ThorDaqDrvEvtDeviceSelfManagedIoRestart;
EVT_WDF_DEVICE_SELF_MANAGED_IO_CLEANUP ThorDaqDrvEvtDeviceSelfManagedIoCleanup ;

//#ifdef IRQL_CALLBACK
EVT_WDF_IO_IN_CALLER_CONTEXT ThorDaqDrvIoInCallerContext;

typedef struct _REQUEST_CONTEXT {
//	WDFMEMORY		PacketReadBuffer;
	PACKET_READ_STRUCT	PacketReadStruct;
	PMDL			pMdl;
} REQUEST_CONTEXT, *PREQUEST_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(REQUEST_CONTEXT, RequestContext)

//#endif // IRQL_CALLBACK

// Init.c Prototypes
EVT_WDF_IO_QUEUE_IO_READ ThorDaqDrvEvtIoRead;
EVT_WDF_IO_QUEUE_IO_WRITE ThorDaqDrvEvtIoWrite;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL ThorDaqDrvEvtIoDeviceControl;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL ThorDaqDrvEvtIoReadWriteDeviceControl;

NTSTATUS ThorDaqDrvInitializeDeviceExtension(PDEVICE_EXTENSION pDevExt);

NTSTATUS ThorDaqDrvPrepareHardware (
		IN PDEVICE_EXTENSION		pDevExt,
		IN WDFCMRESLIST				ResourcesTranslated);


// BoardConfigHandling.c Prototypes

NTSTATUS GetBoardConfigDeviceControl (
		IN WDFDEVICE	device,
		IN WDFREQUEST  	Request,
		IN size_t *	    pInfoSize);

NTSTATUS GetDmaEngineCapabilities (
		IN WDFDEVICE	device,
		IN WDFREQUEST  	Request,
		IN size_t *	    pInfoSize);

NTSTATUS GetDmaPerfNumbers (
		IN WDFDEVICE	device,
		IN WDFREQUEST  	Request,
		IN size_t *	    pInfoSize);

VOID	SetScanLUT (
		IN PDEVICE_EXTENSION	device,
		IN WDFREQUEST  			Request);

VOID	SetDACDesc (
		IN PDEVICE_EXTENSION	device,
		IN WDFREQUEST  			Request);

VOID	OpenOverflowEvent (
		IN PDEVICE_EXTENSION	device,
		IN WDFREQUEST  			Request);

VOID	MessageExchange (
		IN PDEVICE_EXTENSION	device,
		IN WDFREQUEST  			Request);

VOID	ImgAcqConf (
		IN PDEVICE_EXTENSION	device,
		IN WDFREQUEST  			Request);

// Init.c Prototypes

NTSTATUS ThorDaqDrvBoardDmaInit (PDEVICE_EXTENSION pDevExt);

NTSTATUS ThorDaqDrvS2mmDmaInit (PDEVICE_EXTENSION pDevExt);

NTSTATUS ThorDaqDrvBoardConfigInit(PDEVICE_EXTENSION pDevExt);

// IrqHandling.c Prototypes

NTSTATUS ThorDaqDrvInterruptCreate (
		IN PDEVICE_EXTENSION 	pDevExt);

EVT_WDF_INTERRUPT_ISR ThorDaqDrvInterruptIsr;
EVT_WDF_INTERRUPT_ISR ThorDaqDrvInterruptMSIIsr;
EVT_WDF_INTERRUPT_DPC ThorDaqDrvInterruptDpc;
EVT_WDF_INTERRUPT_ENABLE ThorDaqDrvInterruptEnable;
EVT_WDF_INTERRUPT_DISABLE ThorDaqDrvInterruptDisable;

// IoctlHandling.c Prototypes

int GetDMAEngineContext(
	PDEVICE_EXTENSION	    pDevExt,
	ULONG32					EngineNum,
	PDMA_ENGINE_DEVICE_EXTENSION * ppDmaExt);

WDFREQUEST FindRequestByRequest(
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt,
	IN WDFREQUEST			MatchReq);

// PacketIoctl.c Prototypes

NTSTATUS PacketBufferAllocate(
		IN PDEVICE_EXTENSION	pDevExt,
		IN WDFREQUEST	Request,
		IN size_t		OutputBufferLength,
		IN size_t		InputBufferLength);

NTSTATUS PacketBufferRelease(
		IN PDEVICE_EXTENSION	pDevExt,
		IN WDFREQUEST	Request,
		IN size_t		OutputBufferLength,
		IN size_t		InputBufferLength);

NTSTATUS PacketFlushRequests(
		IN PDEVICE_EXTENSION	pDevExt,
		IN WDFREQUEST	Request,
		IN size_t		InputBufferLength);

NTSTATUS ImageAcquisitionConfig(
		IN PDEVICE_EXTENSION	pDevExt,
		IN WDFREQUEST	Request,
		IN size_t		InputBufferLength);

// PacketInit.c Prototypes

NTSTATUS AllocateRxBuffer(
		IN PDEVICE_EXTENSION 	pDevExt,
		IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt);

NTSTATUS InitializeRxDescriptors(
		IN PDEVICE_EXTENSION 	pDevExt,
		IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt,
		IN ULONG32		AllocSize, 
		IN ULONG32		MaxPacketSize);

NTSTATUS InitializeS2mmPacketDescriptors(
	IN PDEVICE_EXTENSION 	pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt);


NTSTATUS InitializeAddressablePacketDescriptors(
	IN PDEVICE_EXTENSION 	pDevExt,
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt);

NTSTATUS InitializeTxDescriptors(
		IN PDEVICE_EXTENSION 	pDevExt,
		IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt);

NTSTATUS InitializeDriverRxDescriptors(
		IN PDEVICE_EXTENSION 	pDevExt,
		IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt,
		IN SIZE_T 				AllocSize, 
		IN ULONG32				MaxPacketSize);

void FreeRxDescriptors(
		IN PDEVICE_EXTENSION 	pDevExt,
		IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt);

void FreeDriverRxDescriptors(
		IN PDEVICE_EXTENSION 	pDevExt,
		IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt);

void ShutdownDMAEngine(
		IN PDEVICE_EXTENSION, 
		IN PDMA_ENGINE_DEVICE_EXTENSION);


// PacketDMA.c Prototypes

EVT_WDF_DPC  PacketS2CDpc;

EVT_WDF_DPC  PacketC2SDpc;

VOID PacketS2CDpc (
		IN WDFDPC Dpc);

VOID PacketC2SDpc (
		IN WDFDPC Dpc);

EVT_WDF_DPC  S2mmInterruptDpc;

VOID S2mmInterruptDpc (
		IN WDFDPC Dpc);



NTSTATUS PacketStartSend (
		IN WDFREQUEST			Request,
		IN PDEVICE_EXTENSION	pDevExt,
		IN PPACKET_SEND_STRUCT	pSendPacket);

NTSTATUS PacketStartWrite (
		IN WDFREQUEST			Request,
		IN PDEVICE_EXTENSION	pDevExt,
		IN PPACKET_WRITE_STRUCT	pWritePacket);

//NTSTATUS ReadRequestSubmit (
//	IN WDFREQUEST			Request,
//	IN PDEVICE_EXTENSION	pDevExt,
//	IN PPACKET_READ_STRUCT	pReadPacket
//	);

NTSTATUS PacketStartRead (
		IN WDFREQUEST			Request,
		IN PDEVICE_EXTENSION	pDevExt,
		IN PPACKET_READ_STRUCT	pReadPacket);

NTSTATUS PacketProcessCompletedReceives(
		IN PDEVICE_EXTENSION	pDevExt,
		IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt);

NTSTATUS PacketProcessReturnedDescriptors(
		IN PDEVICE_EXTENSION	pDevExt,
		IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt,
		IN ULONG32				ReturnToken);

NTSTATUS PacketReadComplete(
		IN PDEVICE_EXTENSION	pDevExt,
		IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt);

EVT_WDF_IO_QUEUE_IO_CANCELED_ON_QUEUE PacketReadRequestCancel;

VOID PacketReadRequestCancel(
		IN WDFQUEUE		Queue,
		IN WDFREQUEST	Request);

NTSTATUS PacketProcessCompletedFreeRunDescriptors(
	IN PDMA_ENGINE_DEVICE_EXTENSION	pDmaExt,
	PPACKET_RECVS_STRUCT			pPacketRecvs);

// WatchdogTimerHandling.c Prototypes
NTSTATUS ThorDaqDrvWatchdogTimerInit (
		IN WDFDEVICE Device
		);

EVT_WDF_TIMER ThorDaqDrvWatchdogTimerCall;

VOID ThorDaqDrvWatchdogTimerStart (
		IN WDFDEVICE Device
		);

VOID ThorDaqDrvWatchdogTimerStop (
		IN WDFDEVICE Device
		);

VOID ThorDaqDrvWatchdogTimerDelete (
		IN WDFDEVICE Device
		);

// ReadWriteHandling.c Prototypes

VOID ReadWriteMemAccess	(
		IN PDEVICE_EXTENSION	pDevExt,
		IN WDFREQUEST			Request,
		IN WDF_DMA_DIRECTION	Rd_Wr_n
		);

VOID ReadWritePCIConfig(
		IN PDEVICE_EXTENSION 	pDevExt,
		IN WDFREQUEST		Request,
		WDF_DMA_DIRECTION	Rd_Wr_n);