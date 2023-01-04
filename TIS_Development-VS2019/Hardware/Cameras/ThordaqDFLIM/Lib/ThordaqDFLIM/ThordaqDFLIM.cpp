/*++

Copyright (c) Thorlab.Inc.  All rights reserved.

Module Name: ThordaqDFLIM.c


Abstract:

    Defines the API functions for the thordaq driver.

Environment:

    kernel mode only.

Style:
	Google C++ coding style.
Note:
	"Wisely and slow; they stumble that run fast". - (William Shakespeare  Romeo and Juliet Act II, Scene III).
--*/

#include "stdafx.h"
#pragma warning(disable:4201) //I just wanna use freedom nonstandard "nameless struct/union"
#include "ThordaqDFLIM.h"
#include "thordaqguid.h"

#define LOGGING_ENABLED
#ifdef LOGGING_ENABLED
#include "..\..\..\..\..\Common\Log.h"
static std::auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#else
enum EventType
{
	// Summary:
	//     Fatal error or application crash.
	CRITICAL_EVENT = 1,
	//
	// Summary:
	//     Recoverable error.
	ERROR_EVENT = 2,
	//
	// Summary:
	//     Noncritical problem.
	WARNING_EVENT = 4,
	//
	// Summary:
	//     Informational message.
	INFORMATION_EVENT = 8,
	//
	// Summary:
	//     Debugging trace.
	VERBOSE_EVENT = 16,

};
#endif

/**********************************************************************************************//**
 * @fn	ThordaqDFLIM::ThordaqDFLIM ( UINT boardNum )
 *
 * @brief	Default constructor.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param	boardNum	Index of thordaq board.
 **************************************************************************************************/
ThordaqDFLIM::ThordaqDFLIM ( UINT boardNum )
{
	gBoardIndex = boardNum;
    gDeviceInfo = NULL;
    gHdlDevice = INVALID_HANDLE_VALUE;
    gDeviceInterfaceDetailData = NULL;
	gDmaInfo.PacketRecvEngineCount = 0;
    gDmaInfo.PacketSendEngineCount = 0;
    gDmaInfo.IsAddressablePacket = false;
	gPtrAcqCtrl = new DATA_ACQ_CTRL_STRUCT(); //Zero initialize a dynamically allocated thordaq acquisition struct.
	memset(gPtrAcqCtrl, 0, sizeof(DATA_ACQ_CTRL_STRUCT));
    return;
}

/**********************************************************************************************//**
 * @fn	ThordaqDFLIM::~ThordaqDFLIM()
 *
 * @brief	Destructor.
 *
 * @author	Cge
 * @date	3/17/2017
 **************************************************************************************************/

ThordaqDFLIM::~ThordaqDFLIM()
{
	//***reset all global available
    gBoardIndex = -1; //no board is connected 
    // Reset to no DMA Engines found
    gDmaInfo.PacketRecvEngineCount = 0;
    gDmaInfo.PacketSendEngineCount = 0;
    gDmaInfo.IsAddressablePacket = false;
	//*** Free all allocated thordaq struct
	SAFE_DELETE_PTR(gPtrAcqCtrl); 
    if ( gDeviceInfo != NULL )
    {
        SetupDiDestroyDeviceInfoList ( gDeviceInfo ); // Free the Info handle
        gDeviceInfo = NULL;
    }
    if ( gDeviceInterfaceDetailData != NULL )
    {
        free ( gDeviceInterfaceDetailData ); // Free interface detail handle
        gDeviceInterfaceDetailData = NULL;
    }
    return;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM:: ConnectToBoard()
 *
 * @brief	Connect to the device.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @return	STATUS_SUCCESSFUL if Thordaq connected. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS ThordaqDFLIM:: ConnectToBoard()
{
    SP_DEVICE_INTERFACE_DATA device_interface_data;
    UINT					 device_count = 0;
    DWORD					 device_interface_data_required_size = 0;
    BOOL					 bool_status = TRUE;
 
    // Get the device info
    if ( gDeviceInfo == NULL )
    {
        gDeviceInfo = SetupDiGetClassDevs (	
			(LPGUID)&GUID_DEVINTERFACE_ThorDaqDrv, // Pointor to the GUID of Thordaq
			NULL,                                  // Define no enumerator (global)
			NULL,                                  // Define no
			DIGCF_DEVICEINTERFACE |                // Function class devices.
			DIGCF_PRESENT );                       // Only Devices present
    }
    if ( gDeviceInfo == INVALID_HANDLE_VALUE )
    {
        return STATUS_DEVICE_NOT_EXISTS;
    }
    //
    // Enumerate devices of a specific interface class
    //
    device_interface_data.cbSize = sizeof ( SP_DEVICE_INTERFACE_DATA );

	while ( SetupDiEnumDeviceInterfaces ( gDeviceInfo, NULL, &GUID_DEVINTERFACE_ThorDaqDrv, device_count, &device_interface_data ) )
	{
		++device_count;
	}

    // last one failed, find out why
    if ( GetLastError() != ERROR_NO_MORE_ITEMS )
    {
        /*printf("SetupDiEnumDeviceInterfaces returned FALSE, index= %d, error = %d\n, Should be ERROR_NO_MORE_ITEMS (%d)\n",
        count-1,
        GetLastError(),
        ERROR_NO_MORE_ITEMS);*/
    }

    // Check to see if there are any boards present
    if ( device_count == 0 )
    {
        return STATUS_DEVICE_NOT_EXISTS;
    }
	// Check to see if there are enough number of boards present
    if ( device_count <= gBoardIndex )
    {
        return STATUS_INVALID_BOARDNUM;
    }

    // Get information for device
    bool_status = SetupDiEnumDeviceInterfaces ( gDeviceInfo, 
		NULL, // No care about specific PDOs
		(LPGUID)&GUID_DEVINTERFACE_ThorDaqDrv, 
		gBoardIndex, //
		&device_interface_data );

    if ( bool_status == false )
    {
        return STATUS_INVALID_BOARDNUM;
    }

    // Get the Device Interface Detailed Data
    // This is done in multiple parts:
    // 1.  query for the required size of the data structure (fix part + variable part)
    // 2.  malloc the returned size
    // 3.  Set the cbSize of the data structure to be the fixed size (required by interface)
    // 4.  query for the actual data
    bool_status = SetupDiGetDeviceInterfaceDetail ( 
		gDeviceInfo, 
		&device_interface_data, 
		NULL, // probing so no output buffer yet
		0, // probing so output buffer length of zero
		&device_interface_data_required_size, 
		NULL );

    // this should fail (returning false) and setting error to ERROR_INSUFFICIENT_BUFFER
    if ( ( bool_status == TRUE ) || ( GetLastError() != ERROR_INSUFFICIENT_BUFFER ) )
    {
        return STATUS_INVALID_BOARDNUM;
    }

    // allocate the correct size
    gDeviceInterfaceDetailData = ( PSP_DEVICE_INTERFACE_DETAIL_DATA ) malloc ( device_interface_data_required_size );

    if ( gDeviceInterfaceDetailData == NULL )
    {
        return STATUS_INVALID_BOARDNUM;
    }

    // set the size to the fixed data size (not the full size)
    gDeviceInterfaceDetailData->cbSize = sizeof ( SP_DEVICE_INTERFACE_DETAIL_DATA );
    // get the data. Do not need DeviceInfoData at this time
    bool_status = SetupDiGetDeviceInterfaceDetail ( gDeviceInfo, &device_interface_data, gDeviceInterfaceDetailData, device_interface_data_required_size, NULL, NULL );

    if ( bool_status == false )
    {
        return STATUS_INVALID_BOARDNUM;
    }

    // Now connect to the card
    gHdlDevice = CreateFile (	gDeviceInterfaceDetailData->DevicePath,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
                            INVALID_HANDLE_VALUE );

    if ( gHdlDevice == INVALID_HANDLE_VALUE )
    {
        return STATUS_INVALID_BOARDNUM;
    }

	// Get Board Configuration. 
	// Initiate onBoard parameters.
	BOARD_INFO_STRUCT		 board_cfg_info;
	if (GetBoardCfg ( &board_cfg_info ) != STATUS_SUCCESSFUL)
	{
		return STATUS_GET_BOARD_CONFIG_ERROR;
	}
	//// set flag for other function calls
    //this->AttachedToDriver = true;
    // Assume no DMA Engines found
	DMA_CAP_STRUCT DMA_cap; // dma capacity
    gDmaInfo.PacketRecvEngineCount = 0;
    gDmaInfo.PacketSendEngineCount = 0;
    gDmaInfo.IsAddressablePacket = false;
    // Get DMA Engine cap to extract the engine numbers for the packet and block mode engines
    // Base Configuration has 1 Card to System DMA Engine and 1 System to Card DMA Engine
	// Multi-Engine Configuration has 1-4 Card to System DMA Engines and 1-4 System to Card DMA Engines
	for (char i = 0; i < MAX_NUM_DMA_ENGINES; i++ )
    {
        gDmaInfo.PacketRecvEngine[i] = -1;
        gDmaInfo.PacketSendEngine[i] = -1;
        GetDMAEngineCap ( i, &DMA_cap );

        if ( ( DMA_cap.dma_capabilityabilities & DMA_CAP_ENGINE_PRESENT ) == DMA_CAP_ENGINE_PRESENT )
        {
            if ( ( DMA_cap.dma_capabilityabilities & DMA_CAP_ENGINE_TYPE_MASK ) & DMA_CAP_PACKET_DMA )
            {
                if ( ( DMA_cap.dma_capabilityabilities & DMA_CAP_ENGINE_TYPE_MASK ) & DMA_CAP_ADDRESSABLE_PACKET_DMA )
                {
                    gDmaInfo.IsAddressablePacket = true;
                }
                if ( ( DMA_cap.dma_capabilityabilities & DMA_CAP_DIRECTION_MASK ) == DMA_CAP_SYSTEM_TO_CARD )
                {
                    gDmaInfo.PacketSendEngine[gDmaInfo.PacketSendEngineCount++] = i;
                }
				else if ( ( DMA_cap.dma_capabilityabilities & DMA_CAP_DIRECTION_MASK ) == DMA_CAP_CARD_TO_SYSTEM )
                {
                     gDmaInfo.PacketRecvEngine[gDmaInfo.PacketRecvEngineCount++] = i;
                }
				else
                {
#if _DEBUG
                    printf("of invalid type\n");
#endif
				}
            }
        }
    }

	SetDACChannelMapping();
	SetADCChannelMapping();
	//SetDLIMFrontEndSettings();

	return STATUS_SUCCESSFUL;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM:: DisconnectFromBoard()
 *
 * @brief	Disconnect to the device.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @return	STATUS_SUCCESSFUL if Thordaq disconnected. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS ThordaqDFLIM:: DisconnectFromBoard()
{
	//Free Device handle
    if ( gHdlDevice != INVALID_HANDLE_VALUE )
    {
        for (int i = 0; i < gDmaInfo.PacketRecvEngineCount; i++ )
        {
			if( ReleasePacketBuffers ( i ) != STATUS_SUCCESSFUL)
			{
				return STATUS_RELEASE_PACKET_MODE_INCOMPLETE;
			}
        }
        CloseHandle ( gHdlDevice );
        gHdlDevice = INVALID_HANDLE_VALUE;
    }
    // Free up device detail
    if ( gDeviceInterfaceDetailData != NULL )
    {
        free ( gDeviceInterfaceDetailData );
        gDeviceInterfaceDetailData = NULL;
    }
    return STATUS_SUCCESSFUL;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM:: GetBoardCfg ( BOARD_INFO_STRUCT* board_info )
 *
 * @brief	Get the configuration of the device.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param [in,out]	board_info	Firmware Version number.
 *
 * @return	STATUS_SUCCESSFUL if Thordaq return. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS ThordaqDFLIM:: GetBoardCfg (
    BOARD_INFO_STRUCT*	    board_info  // Returned structure
)
{
    THORDAQ_STATUS			status            = STATUS_SUCCESSFUL;
    DWORD					bytes_returned    = 0;
    DWORD					last_error_status = 0;
    BOARD_CONFIG_STRUCT     board_config;
	OVERLAPPED overlapped;  // OVERLAPPED structure for the operation

	// Initiate Firmware and Driver version number
	board_info->DriverVersionBuildNumber = 0;
	board_info->DriverVersionMajor		 = 0;
	board_info->DriverVersionMinor       = 0;
	board_info->DriverVersionSubMinor    = 0;
	board_info->UserVersion              = 0;

	//Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

    // Send GET_BOARD_CONFIG_IOCTL
    if ( !DeviceIoControl ( gHdlDevice, GET_BOARD_CONFIG_IOCTL, NULL, 0, ( LPVOID ) &board_config, sizeof ( BOARD_CONFIG_STRUCT ), &bytes_returned, &overlapped ) )
    {
        last_error_status = GetLastError();

        if ( last_error_status != ERROR_IO_PENDING )
        {
#if _DEBUG
            printf ( "GetBoardCfg IOCTL call failed. Error = %d\n", GetLastError() );
#endif // _DEBUG            
			status = STATUS_GET_BOARD_CONFIG_ERROR;
        } else
        {
            // Wait here (forever) for the Overlapped I/O to complete
            if ( !GetOverlappedResult ( gHdlDevice, &overlapped, &bytes_returned, TRUE ) )
            {
                last_error_status = GetLastError();
#if _DEBUG
                printf ( "GetBoardCfg IOCTL call failed. Error = %d\n", last_error_status );
#endif // _DEBUG
                status = STATUS_GET_BOARD_CONFIG_ERROR;
            }
        }
    }

    // check returned structure size
    if ( ( bytes_returned != sizeof ( BOARD_CONFIG_STRUCT ) ) && ( status == STATUS_SUCCESSFUL ) )
    {
        // ioctl failed
#if _DEBUG
        printf ( "GetBoardCfg IOCTL returned invalid size (%d)\n", bytes_returned );
#endif // _DEBUG
        status = STATUS_GET_BOARD_CONFIG_ERROR;
    } else
    {
		board_info->DriverVersionBuildNumber = board_config.DriverVersionBuildNumber;
		board_info->DriverVersionMajor		 = board_config.DriverVersionMajor;
		board_info->DriverVersionMinor       = board_config.DriverVersionMinor;
		board_info->DriverVersionSubMinor    = board_config.DriverVersionSubMinor;
		board_info->UserVersion              = board_config.UserVersion;
    }
	CloseHandle ( overlapped.hEvent );
    return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM:: WriteReadRegister ( UINT read_write_flag, UINT register_bar_num, ULONGLONG register_card_offset, BYTE* buffer, ULONGLONG offset, ULONGLONG length, PSTAT_STRUCT completed_status)
 *
 * @brief	Set/Get the register value of the device.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param 		  	read_write_flag			Idnetifier for data transfer direction 1==Read,
 * 											0==Write.
 * @param 		  	register_bar_num		Index of Bar configuration of FPGA.
 * @param 		  	register_card_offset	Register address offset to the head of bar.
 * @param [in,out]	buffer					Buffer to write/read.
 * @param 		  	offset					Offset in data buffer to start transfer.
 * @param 		  	length					Byte length of transfer buffer.
 * @param 		  	completed_status		The completed status.
 *
 * @return	STATUS_SUCCESSFUL if Thordaq return. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS ThordaqDFLIM:: WriteReadRegister ( UINT read_write_flag, UINT register_bar_num, ULONGLONG register_card_offset, BYTE* buffer, ULONGLONG offset,  ULONGLONG length,  PSTAT_STRUCT completed_status)
{
    THORDAQ_STATUS status = STATUS_SUCCESSFUL;
    DO_MEM_STRUCT			write_read_struct;
	OVERLAPPED overlapped;  // OVERLAPPED structure for the operation
    DWORD					bytes_returned = 0;
    DWORD					io_ctl_code;
    DWORD					last_error_status = 0;
	//Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

	// bounds checking
	if ((read_write_flag != READ_FROM_CARD) && (read_write_flag != WRITE_TO_CARD)
		||(register_bar_num > USED_MAX_BARS)
		||(register_card_offset > USED_MAX_CARD_OFFSET))
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

    // fill in the doMem Structure
    write_read_struct.register_bar_num     = register_bar_num;
    write_read_struct.Offset		       = offset;
    write_read_struct.register_card_offset = register_card_offset;
    write_read_struct.Length               = length;
    // determine the ioctl code
    io_ctl_code = ( read_write_flag == READ_FROM_CARD ) ? DO_MEM_READ_ACCESS_IOCTL : DO_MEM_WRITE_ACCESS_IOCTL;
    // Send write_read_struct to driver io control
    if ( !DeviceIoControl ( gHdlDevice, io_ctl_code, ( LPVOID ) &write_read_struct, sizeof ( DO_MEM_STRUCT ), ( LPVOID ) buffer, ( DWORD ) length, &bytes_returned, &overlapped ) )
    {
        last_error_status = GetLastError();
        if ( last_error_status != ERROR_IO_PENDING )
        {
            completed_status->CompletedByteCount = 0;
#if _DEBUG
            printf ( "DoMem IOCTL call failed. Error = %d\n", last_error_status );
#endif          
			status = STATUS_READWRITE_REGISTER_ERROR;
        } else
        {
            // Wait here (forever) for the Overlapped I/O to complete
            if ( !GetOverlappedResult ( gHdlDevice, &overlapped, &bytes_returned, TRUE ) )
            {
                last_error_status = GetLastError();
#if _DEBUG
                printf ( "DoMem IOCTL call failed. Error = %d\n", last_error_status );
#endif // _DEBUG
                status = STATUS_READWRITE_REGISTER_ERROR;
            }
        }
    }
    // save the returned size
    completed_status->CompletedByteCount = bytes_returned;
    // check returned structure size
    if ( ( bytes_returned != length ) && ( status == STATUS_SUCCESSFUL ) ) // Got the wrong data
    {
        // ioctl failed
#if _DEBUG
        printf ( "DoMem IOCTL returned invalid size (%d), expected length %lld\n", bytes_returned, length );
#endif // _DEBUG
        status = STATUS_READWRITE_REGISTER_ERROR;
    }
    CloseHandle ( overlapped.hEvent );
    return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM:: SetPacketMode (int dma_engine_offset,bool stream_to_mem_dma_enable, ULONG* , int packet_alloc_mode, int num_of_descriptors)
 *
 * @brief	Sets packet mode.
 *
 * @author	Cge
 * @date	3/20/2017
 *
 * @param 		  	dma_engine_offset			The dma engine offset.
 * @param 		  	stream_to_mem_dma_enable	True if stream to memory dma enable.
 * @param [in,out]	parameter3					If non-null, the third parameter.
 * @param 		  	packet_alloc_mode			The packet allocate mode.
 * @param 		  	num_of_descriptors			Number of descriptors.
 *
 * @return	A ThordaqDFLIM::
 **************************************************************************************************/

THORDAQ_STATUS ThordaqDFLIM:: SetPacketMode (int dma_engine_offset,bool stream_to_mem_dma_enable, ULONG* , int packet_alloc_mode, int num_of_descriptors)
{
    THORDAQ_STATUS status = STATUS_SUCCESSFUL;
    BUF_ALLOC_STRUCT		buf_alloc;
	OVERLAPPED overlapped;  // OVERLAPPED structure for the operation
    DWORD					bytes_returned = 0;
    DWORD					last_error_status = 0;
    //Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

	// bounds checking
    if ( dma_engine_offset >= gDmaInfo.PacketRecvEngineCount )
    {
		return STATUS_SETUP_PACKET_MODE_INCOMPLETE;
	}

	buf_alloc.IsS2mmDmaEnabled = stream_to_mem_dma_enable == true? TRUE: FALSE;
	
    // Set the DMA Engine we want to allocate for
    buf_alloc.dma_engine_offset = gDmaInfo.PacketRecvEngine[dma_engine_offset];
    // Setup in Addressable Packet mode
    buf_alloc.AllocationMode = packet_alloc_mode;
    buf_alloc.Length = 0;
    buf_alloc.packet_max_size = 0;
    buf_alloc.num_of_descriptors = num_of_descriptors;

    // Send Setup Packet Mode Addressable IOCTL
    if ( !DeviceIoControl ( gHdlDevice, PACKET_BUF_ALLOC_IOCTL, &buf_alloc, sizeof ( BUF_ALLOC_STRUCT ), NULL, 0, &bytes_returned, &overlapped ) )
    {
        last_error_status = GetLastError();

        if ( last_error_status == ERROR_IO_PENDING )
        {
            // Wait here (forever) for the Overlapped I/O to complete
            if ( !GetOverlappedResult ( gHdlDevice, &overlapped, &bytes_returned, TRUE ) )
            {
                last_error_status = GetLastError();
#if _DEBUG
                printf ( "Addressable Packet Mode setup overlap failed. Error = %d\n", last_error_status );
#endif
				status = STATUS_SETUP_PACKET_MODE_INCOMPLETE;
            }
        }

        else
        {
#if _DEBUG
            printf ( "Addressable Packet mode setup failed. Error = %d\n", last_error_status );
#endif          
			status = STATUS_SETUP_PACKET_MODE_INCOMPLETE;
        }
    }
    CloseHandle ( overlapped.hEvent );

	//status = SetImagingSettings();

    return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM:: GetDMAEngineCap ( ULONG dma_engine_offset, PDMA_CAP_STRUCT dma_capability)
 *
 * @brief	Get DMA Engine Capabilitie.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param	dma_engine_offset	DMA Engine number offset to use.
 * @param	dma_capability   	Returned DMA Engine Capabilitie.
 *
 * @return	STATUS_SUCCESSFUL if Thordaq return. Others if problem happened.
 **************************************************************************************************/
THORDAQ_STATUS ThordaqDFLIM:: GetDMAEngineCap ( ULONG dma_engine_offset, PDMA_CAP_STRUCT dma_capability)
{
    THORDAQ_STATUS status = STATUS_SUCCESSFUL;
    OVERLAPPED				overlapped;			// OVERLAPPED structure for the operation
    DWORD					bytes_returned = 0;
    DWORD					last_error_status = 0;
	//Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );
    // Send BLOCK_DIRECT_GET_PERF_IOCTL IOCTL
    if ( DeviceIoControl ( gHdlDevice, GET_DMA_ENGINE_CAP_IOCTL, ( LPVOID ) &dma_engine_offset, sizeof ( ULONG ), ( LPVOID ) dma_capability, sizeof ( DMA_CAP_STRUCT ), &bytes_returned, &overlapped ) == 0 )
    {
        last_error_status = GetLastError();

        if ( last_error_status != ERROR_IO_PENDING )
        {
#if _DEBUG
            printf ( "Getdma_capability IOCTL call failed. Error = %d\n", last_error_status );
#endif
			status = STATUS_GET_BOARD_CONFIG_ERROR;
        } else
        {
            // Wait here (forever) for the Overlapped I/O to complete
            if ( !GetOverlappedResult ( gHdlDevice, &overlapped, &bytes_returned, TRUE ) )
            {
                last_error_status = GetLastError();
#if _DEBUG
                printf ( "Getdma_capability IOCTL call failed. Error = %d\n", last_error_status );
#endif // _DEBUG
                status = STATUS_GET_BOARD_CONFIG_ERROR;
            }
        }
    }
    // check returned structure size
    if ( ( bytes_returned != sizeof ( DMA_CAP_STRUCT ) ) &&
            ( status == STATUS_SUCCESSFUL ) )
    {
        // ioctl failed
#if _DEBUG
        printf ( "Getdma_capability IOCTL returned invalid size (%d)\n", bytes_returned );
#endif // _DEBUG
        status = STATUS_GET_BOARD_CONFIG_ERROR;
    }
    CloseHandle ( overlapped.hEvent );
    return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM::ReleasePacketBuffers ( int dma_engine_offset )
 *
 * @brief	Sends two PACKET_BUF_DEALLOC_IOCTL calls to the driver to teardown the recieve buffer
 * 			and teardown the descriptors for sending packets.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param	dma_engine_offset	DMA Engine number offset to use.
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS ThordaqDFLIM::ReleasePacketBuffers ( int dma_engine_offset )
{
    BUF_DEALLOC_STRUCT		buffer_release;
    OVERLAPPED				overlapped;			// OVERLAPPED structure for the operation
    DWORD					bytes_returned = 0;
    DWORD					last_error_status = 0;
    THORDAQ_STATUS			status = STATUS_RELEASE_PACKET_MODE_INCOMPLETE;
	ULONG64					_pRxPacketBufferHandle = 0;      // The Address of DMA Packet Mode Allocated Buffer

    //Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

    if ( dma_engine_offset < gDmaInfo.PacketRecvEngineCount )
    {
        // Set the DMA Engine we want to de-allocate
        buffer_release.dma_engine_offset = gDmaInfo.PacketRecvEngine[dma_engine_offset];
        // Set the allocation mode to what we used above
        buffer_release.Reserved = 0;
        // Return the Buffer Address we recieved from the Allocate call
        buffer_release.RxBufferAddress = _pRxPacketBufferHandle;

        // Send Packet Mode Release
        if ( !DeviceIoControl ( gHdlDevice, PACKET_BUF_RELEASE_IOCTL, &buffer_release, sizeof ( BUF_DEALLOC_STRUCT ), NULL, 0, &bytes_returned,	&overlapped ) )
        {
            last_error_status = GetLastError();

            if ( last_error_status == ERROR_IO_PENDING )
            {
                // Wait here (forever) for the Overlapped I/O to complete
                if ( !GetOverlappedResult ( gHdlDevice, &overlapped, &bytes_returned, TRUE ) )
                {
                    last_error_status = GetLastError();
#if _DEBUG
                    printf ( "GetOverlappedResult Packet Rx Buffer Deallocate failed. Error = %d\n", last_error_status );
#endif          
					status = STATUS_RELEASE_PACKET_MODE_INCOMPLETE;
                }else
                {
                    _pRxPacketBufferHandle = NULL;
                    status = STATUS_SUCCESSFUL;
                }
            } else
            {
                // ioctl failed
#if _DEBUG     
				printf ( "DeviceIoControl Packet Rx buffer DeAllocate failed. Error = %d\n", last_error_status );
#endif          
				status = STATUS_RELEASE_PACKET_MODE_INCOMPLETE;
            }
        } else
        {
            _pRxPacketBufferHandle = NULL;
            status = STATUS_SUCCESSFUL;
        }
    }
    CloseHandle ( overlapped.hEvent );
    return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM::PacketRead( ULONG Channel, ULONG* buffer_length, void* Buffer, ULONG Timeout)
 *
 * @brief	Read data from particular channel address predefined in the FPGA.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param 		  	Channel			Channel Index to be read from.
 * @param [in,out]	buffer_length	Size of the Packet Recieve Data Buffer requested (FIFO Mode)
 * @param [in,out]	Buffer			Data buffer (Packet Mode)
 * @param 		  	Timeout			Generate Timeout error when timeout.
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS ThordaqDFLIM::PacketReadChannel( ULONG channel, ULONG*  buffer_length, void* buffer, double Timeout_ms)
{
    THORDAQ_STATUS status = STATUS_SUCCESSFUL;
    PACKET_READ_STRUCT		packet_read_config;
    PACKET_RET_READ_STRUCT	packet_read_config_ret;
    DWORD					bytes_returned = 0;
    DWORD					last_error_status = 0;
    int                     dma_engine_offset = 0;
	ULONG32                 bufferSize = 0;
	OVERLAPPED				overlapped;			// OVERLAPPED structure for the operation
	ULONG32                 Timeout =  0xffffffff;
	//sanity check 
	if ( channel >= MAX_CHANNEL_COUNT
		|| Timeout_ms < 0 
		|| gDmaInfo.PacketRecvEngineCount <= 0)
	{
		return STATUS_READ_BUFFER_ERROR;
	}

	//Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

    packet_read_config.dma_engine_offset	  =  gDmaInfo.PacketRecvEngine[dma_engine_offset]; //use the receive engine
    packet_read_config.ModeFlags	  = (ULONG32) C2S_DIRECTION;
    packet_read_config.register_card_offset    = (ULONG64)0;
    packet_read_config.IsMemCheck    = (UCHAR)0;
	packet_read_config.Length = *buffer_length;
    packet_read_config.Channel       = static_cast<UCHAR> ( channel );
    packet_read_config.BufferAddress = ( ULONG64 ) ( buffer );
	bufferSize				  =	(ULONG32) *buffer_length;

	packet_read_config_ret.Length = 0;
	packet_read_config_ret.UserStatus = 0x1111111111111111;
	abortPacketRead = false;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	long long time_diff = 0;
	do
	{
		//packet_read_config.Length = bufferSize;
		if ( !DeviceIoControl ( gHdlDevice, PACKET_CHANNEL_READ_IOCTL, &packet_read_config, sizeof ( PACKET_READ_STRUCT ), &packet_read_config_ret, sizeof ( PACKET_RET_READ_STRUCT ), &bytes_returned, &overlapped ) )
		{
			last_error_status = GetLastError();

			while ( (( last_error_status == ERROR_IO_PENDING ) || ( last_error_status == ERROR_IO_INCOMPLETE )) && abortPacketRead == false)
			{
				// Wait here (forever) for the Overlapped I/O to complete
				if ( !GetOverlappedResult ( gHdlDevice, &overlapped, &bytes_returned, FALSE ) )
				{
					last_error_status = GetLastError();

					if ( ( last_error_status == ERROR_IO_PENDING ) || ( last_error_status == ERROR_IO_INCOMPLETE ) )
					{
						if ( !--Timeout )
						{
#if _DEBUG
							printf ( "Packet Read Overlapped Timed Out. Error = %d\n", last_error_status );
#endif
							status = STATUS_READ_BUFFER_ERROR;
							CloseHandle(overlapped.hEvent);
							return status;
						}
					}
					else
					{
#if _DEBUG
						printf ( "Packet Read Overlapped failed. Error = %d\n", last_error_status );
#endif
						break;
					}
				} else
				{
					break;
				}
			}  // while ...
		} // if (!DeviceIoControl ...
		std::chrono::steady_clock::time_point end= std::chrono::steady_clock::now();
		time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
		if (((double)time_diff > Timeout_ms) && (packet_read_config_ret.UserStatus != 0))
		{
			status = STATUS_READ_BUFFER_TIMEOUT_ERROR;
			break;
		}
	} while (packet_read_config_ret.UserStatus != 0 && abortPacketRead == false);
 
    if ( status == STATUS_SUCCESSFUL && bytes_returned != sizeof ( PACKET_RET_READ_STRUCT ) )
    {
		status = STATUS_READ_BUFFER_ERROR;
    }
	CloseHandle(overlapped.hEvent);
    return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM::AbortPacketRead()
 *
 * @brief	Abort reading.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS ThordaqDFLIM::AbortPacketRead()
{
	abortPacketRead = true;
    THORDAQ_STATUS status = STATUS_SUCCESSFUL;
    return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM::PacketReadEx( ULONG64 Address, ULONG* Length, void* Buffer, ULONG Timeout )
 *
 * @brief	Read data from particular data start address.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param 		  	Address	Start address to be read from.
 * @param [in,out]	Length 	Size of the Packet Recieve Data Buffer requested (FIFO Mode)
 * @param [in,out]	Buffer 	Data buffer (Packet Mode)
 * @param 		  	Timeout	Generate Timeout error when timeout.
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS ThordaqDFLIM::PacketReadBuffer( ULONG64 Address, ULONG* Length, void* Buffer, ULONG Timeout )
{
	
    THORDAQ_STATUS status = STATUS_SUCCESSFUL;
    PACKET_READ_STRUCT		packet_read_config;
    PACKET_RET_READ_STRUCT	packet_read_config_ret;
    OVERLAPPED				overlapped;			// OVERLAPPED structure for the operation
    DWORD					bytes_returned = 0;
    DWORD					last_error_status = 0;
    int                     dma_engine_offset = 0;
    //Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

    packet_read_config.dma_engine_offset = gDmaInfo.PacketRecvEngine[dma_engine_offset];
    packet_read_config.ModeFlags = C2S_DIRECTION;
    packet_read_config.Length = *Length;
    packet_read_config.register_card_offset = Address;
    packet_read_config.IsMemCheck = 0;
    packet_read_config.Channel = static_cast<UCHAR> ( 0 );
    packet_read_config.BufferAddress = ( ULONG64 ) ( Buffer );
	abortPacketRead = false;
	if ( !DeviceIoControl ( gHdlDevice, PACKET_BUF_READ_IOCTL, &packet_read_config, sizeof ( PACKET_READ_STRUCT ), &packet_read_config_ret, sizeof ( PACKET_RET_READ_STRUCT ), &bytes_returned, &overlapped ) )
	{
		last_error_status = GetLastError();

		while ( (( last_error_status == ERROR_IO_PENDING ) || ( last_error_status == ERROR_IO_INCOMPLETE )) && abortPacketRead == false)
		{
			// Wait here (forever) for the Overlapped I/O to complete
			if ( !GetOverlappedResult ( gHdlDevice, &overlapped, &bytes_returned, FALSE ) )
			{
				last_error_status = GetLastError();

				if ( ( last_error_status == ERROR_IO_PENDING ) || ( last_error_status == ERROR_IO_INCOMPLETE ) )
				{
					if ( !--Timeout )
					{
#if _DEBUG
						printf ( "Packet Read Overlapped Timed Out. Error = %d\n", last_error_status );
#endif
						status = STATUS_READ_BUFFER_ERROR;
						return status;
					}
				}
				else
				{
#if _DEBUG
					printf ( "Packet Read Overlapped failed. Error = %d\n", last_error_status );
#endif
					break;
				}
			} else
			{
				break;
			}
		}  // while ...
	} // if (!DeviceIoControl ...
 
    if (status != STATUS_SUCCESSFUL || (status == STATUS_SUCCESSFUL && bytes_returned != sizeof ( PACKET_RET_READ_STRUCT )))
    {
		status = STATUS_READ_BUFFER_ERROR;
    }
	CloseHandle(overlapped.hEvent);
    return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM::PacketWriteBuffer( ULONG64 register_card_offset, ULONG Length, UCHAR* Buffer, ULONG Timeout )
 *
 * @brief	Packet write ex.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param 		  	register_card_offset	The card offset.
 * @param 		  	Length	  	The length.
 * @param [in,out]	Buffer	  	If non-null, the buffer.
 * @param 		  	Timeout   	The timeout.
 *
 * @return	A THORDAQ_STATUS.
 **************************************************************************************************/

THORDAQ_STATUS ThordaqDFLIM::PacketWriteBuffer(
		ULONG64  register_card_offset,           // Start address to read in the card
		ULONG    buf_len,            // Size of the Packet Write to Data Buffer Count by Byte (FIFO Mode)
		UCHAR*   buf_ptr,            // Data buffer (Packet Mode)
		ULONG    timeout            // Generate Timeout error when timeout
		)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	PACKET_WRITE_STRUCT packet_write_config_ret;
	OVERLAPPED				overlapped;			// OVERLAPPED structure for the operation
    DWORD					bytes_returned = 0;
    DWORD					last_error_status = 0;
    int                     dma_engine_offset = 0;

    //Initiate overlapped structure
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );
	//This sets up yje C2S DMA Descriptors and enables the DMA.
	status = SetPacketMode(dma_engine_offset,false, NULL, PACKET_MODE_ADDRESSABLE, 8192);
	if (status != STATUS_SUCCESSFUL)
	{
		return status;
	}
	//
	packet_write_config_ret.register_card_offset = register_card_offset;
	packet_write_config_ret.dma_engine_offset = gDmaInfo.PacketSendEngine[dma_engine_offset];
	packet_write_config_ret.Length = buf_len;
	packet_write_config_ret.ModeFlags = 0;
	packet_write_config_ret.UserControl = 0;

	if (!DeviceIoControl(gHdlDevice, PACKET_WRITE_IOCTL, &packet_write_config_ret, sizeof(PACKET_WRITE_STRUCT), (LPVOID) buf_ptr, (DWORD) buf_len, &bytes_returned, &overlapped))
	{
			last_error_status = GetLastError();
			while ((last_error_status == ERROR_IO_PENDING) || 
				(last_error_status == ERROR_IO_INCOMPLETE))
			{
				// Wait here (forever) for the Overlapped I/O to complete
				if (!GetOverlappedResult(gHdlDevice, &overlapped, &bytes_returned, FALSE))
				{
					last_error_status = GetLastError();
					if ((last_error_status == ERROR_IO_PENDING) || (last_error_status == ERROR_IO_INCOMPLETE))
					{
						if (!--timeout)
						{
							status = STATUS_WRITE_BUFFER_ERROR;
							break;
						}
					}
					else
					{
						status = STATUS_WRITE_BUFFER_ERROR;
						break;
					}
				}
				else
				{
					break;
				}
			}
	}  // if (!DeviceIoControl...
	// Make sure we returned something useful
	if (status != STATUS_SUCCESSFUL || (status == STATUS_SUCCESSFUL && bytes_returned != buf_len))
	{
		status = STATUS_WRITE_BUFFER_ERROR;
	}
	CloseHandle(overlapped.hEvent);
    return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM::SetImagingConfiguration( IMAGING_CONFIGURATION_STRUCT imaging_config)
 *
 * @brief	Set up data imaging configuration.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @param	imaging_config	Image configuration stuct.
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS ThordaqDFLIM::SetImagingConfiguration( IMAGING_CONFIGURATION_STRUCT	imaging_config)
{
	
	DWORD						bytes_returned = 0;
	DWORD						last_error_status = 0;
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	memset(&gPtrAcqCtrl->gblCtrl, 0, sizeof(gPtrAcqCtrl->gblCtrl));
	memset(&gPtrAcqCtrl->scan, 0, sizeof(gPtrAcqCtrl->scan));
	memset(&gPtrAcqCtrl->samplingClock, 0, sizeof(gPtrAcqCtrl->samplingClock));
	memset(&gPtrAcqCtrl->streamProcessing, 0, sizeof(gPtrAcqCtrl->streamProcessing));
	memset(&gPtrAcqCtrl->adcInterface, 0, sizeof(gPtrAcqCtrl->adcInterface));
	memset(&gPtrAcqCtrl->galvoCtrl, 0, sizeof(gPtrAcqCtrl->galvoCtrl));
	memset(&gPtrAcqCtrl->i2cCtrl, 0, sizeof(gPtrAcqCtrl->i2cCtrl));
	if(SetGlobalSettings(imaging_config) == FALSE)
	{
		return STATUS_PARAMETER_SETTINGS_ERROR;
	}

	if (imaging_config.imageCtrl.defaultMode == 0)
	{
		//Setup Scan Subsystem Configurations
		if(SetScanSettings(imaging_config) == FALSE)
		{
			return STATUS_PARAMETER_SETTINGS_ERROR;
		}

		//Setup Analogy out settings
		if (SetDACSettings(imaging_config) == FALSE)
		{
			return STATUS_PARAMETER_SETTINGS_ERROR;
		}

		//Setup Coherent Sampling Configurations
		if(SetCoherentSampleingSettings(imaging_config) == FALSE)
		{
			return STATUS_PARAMETER_SETTINGS_ERROR;
		}
		
		//Setup Stream Processing
		if (SetStreamProcessingSettings(imaging_config) == FALSE)
		{
			return STATUS_PARAMETER_SETTINGS_ERROR;
		}

		// Load the DMA Descriptors
		if (LoadDACDescriptors(imaging_config.dacCtrl) == FALSE)
		{
			return STATUS_PARAMETER_SETTINGS_ERROR;
		}
	}else
	{
		memset(&gPtrAcqCtrl->galvoCtrl, 0, sizeof(gPtrAcqCtrl->galvoCtrl));
	}
	
	//Initiate overlapped structure
	OVERLAPPED					overlapped;			// OVERLAPPED structure for the operation
	memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );
	if (!DeviceIoControl(gHdlDevice, IMG_ACQ_CONF_IOCTL, (LPVOID)gPtrAcqCtrl, sizeof(DATA_ACQ_CTRL_STRUCT), NULL, 0, &bytes_returned, &overlapped))
	{
		last_error_status = GetLastError();
		if (last_error_status == ERROR_IO_PENDING)
		{
			// Wait here (forever) for the Overlapped I/O to complete
			if (!GetOverlappedResult(gHdlDevice, &overlapped, &bytes_returned, TRUE))
			{
				last_error_status = GetLastError();
#if _DEBUG		
				printf("Packet Gen/Chk Control failed. Error = %d\n", last_error_status);
#endif // _DEBUG
				status = STATUS_PARAMETER_SETTINGS_ERROR;
			}
		}
		else
		{
			// ioctl failed	
#if _DEBUG	
			printf("Packet Generator Control failed. Error = %d\n", last_error_status);
#endif		
			status = STATUS_PARAMETER_SETTINGS_ERROR;
		}
	}
	CloseHandle(overlapped.hEvent);
    return status;

	//return SetImagingSettings();
}

LONG ThordaqDFLIM::SetGlobalSettings(IMAGING_CONFIGURATION_STRUCT	imaging_config)
{
	//set up const settings
	gPtrAcqCtrl->gblCtrl.dma_engine_index   = gDmaInfo.PacketRecvEngine[0];         // set DMA engine
	gPtrAcqCtrl->gblCtrl.run_stop_mode		= PACKET_GEN_RUN;                       // enable acquisition bit
	gPtrAcqCtrl->gblCtrl.acq_buf_addr		= (ULONG32)0x00000000;
	gPtrAcqCtrl->gblCtrl.acq_buf_chn_offset = (ULONG32)ACQ_SINGLE_CHANNEL_BUF_CAP;
	gPtrAcqCtrl->gblCtrl.acquisitionMode	= imaging_config.imageCtrl.acquisitionMode; //0 = DFLIM mode, 1 = diagnostic mode, 2 = Counter mode

	// set uo the channels are enabled
	if ((imaging_config.imageCtrl.channel - MIN_CHANNEL) <=  RANGE_CHANNEL) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.channel = imaging_config.imageCtrl.channel;  
	}else
	{
		return FALSE;
	}
	// set up frame count to be acquired
	if (imaging_config.imageCtrl.frameCnt  <=  MAX_FRAME_NUM) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.frame_number = imaging_config.imageCtrl.frameCnt;  
	}else
	{
		return FALSE;
	}
	// set up horizontal pixel density
	if ((imaging_config.imageCtrl.imgHSize - MIN_PIXEL_X) <=  RANGE_PIXEL_X) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.hor_pix_num = imaging_config.imageCtrl.imgHSize;  
	}else
	{
		return FALSE;
	}
	if (imaging_config.imageCtrl.dataHSize <= MAX_DATA_HSIZE) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.dataHSize = imaging_config.imageCtrl.dataHSize;  
	}else
	{
		return FALSE;
	}
	// set up vertical pixel density
	if ((imaging_config.imageCtrl.imgVSize - MIN_PIXEL_Y) <=  RANGE_PIXEL_Y) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.vrt_pix_num = imaging_config.imageCtrl.imgVSize;  
	}else
	{
		return FALSE;
	}
	if ((imaging_config.imageCtrl.linesPerStripe - MIN_PIXEL_Y) <=  RANGE_PIXEL_Y) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.linesPerStripe = imaging_config.imageCtrl.linesPerStripe;  
	}else
	{
		return FALSE;
	}
	//set up frame rate
	if ((imaging_config.imageCtrl.frameNumPerSec - MIN_FRAME_RATE) <=  RANGE_FRAME_RATE) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.frm_per_sec		= imaging_config.imageCtrl.frameNumPerSec;
		gPtrAcqCtrl->gblCtrl.frm_per_txn		= imaging_config.imageCtrl.frameNumPerTransfer;
	}else
	{
		return FALSE;
	}
	//set up debug mode
	if (imaging_config.imageCtrl.defaultMode - MIN_DEBUG_MODE <=  RANGE_DEBUG_MODE) // bounds check
	{
		gPtrAcqCtrl->gblCtrl.dbg_test_mode		= imaging_config.imageCtrl.defaultMode; // Disable Debug Mode (only used with GUI)
	}else
	{
		return FALSE;
	}
	if (imaging_config.imageCtrl.defaultMode == MIN_DEBUG_MODE)
	{
		//Set up Scan direction, Bi-Scan mode, hardware trigger mode
		if (imaging_config.imageCtrl.scanMode < 2   &&  imaging_config.imageCtrl.scanDir < 2 && imaging_config.imageCtrl.triggerMode < 2) //bounds check
		{
			{
				gPtrAcqCtrl->gblCtrl.img_scan_mode = imaging_config.imageCtrl.scanMode << 1 | (imaging_config.imageCtrl.scanDir << 2) | ((imaging_config.imageCtrl.triggerMode ^ 0x01) << 4);
			}
		}else
		{
			return FALSE;
		}

		//set gpio
		gPtrAcqCtrl->gblCtrl.gpio_cfg = ((static_cast<ULONG64>(CH0))) 
			| ((static_cast<ULONG64>(CH1)) << 8)
			| ((static_cast<ULONG64>(CH2)) << 16)
			| ((static_cast<ULONG64>(CH3)) << 24)
			| ((static_cast<ULONG64>(CH4)) << 32)
			| ((static_cast<ULONG64>(CH5)) << 40)
			| ((static_cast<ULONG64>(CH6)) << 48)
			| ((static_cast<ULONG64>(CH7)) << 56);
	}
	
	return TRUE;
}

LONG ThordaqDFLIM::SetScanSettings(IMAGING_CONFIGURATION_STRUCT	imaging_config)
{
	//Set up GG/GR scan mode, frame count
	if (imaging_config.imageCtrl.system_mode < 2)
	{
		gPtrAcqCtrl->scan.sync_ctrl = (gPtrAcqCtrl->scan.sync_ctrl & 0x1D) | (imaging_config.imageCtrl.system_mode << 5);
		if (imaging_config.imageCtrl.frameCnt != (ULONG32)MAX_FRAME_NUM) // Continuously scan
		{
			gPtrAcqCtrl->scan.sync_ctrl = gPtrAcqCtrl->scan.sync_ctrl | 0x02;
			gPtrAcqCtrl->scan.frm_cnt	= imaging_config.imageCtrl.frameCnt;
		}
		//aditional DFLIM settings
		gPtrAcqCtrl->scan.sync_ctrl |= (0xbc << 8);
	}
	else
	{
		return FALSE;
	}
	
	/**** Write to log Frm_Cnt & FrameCnt for trouble-shooting 
	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg,MSG_SIZE,L"ThorDAQ \nFrm_Cnt: %d \nFrameCnt: %d", gPtrAcqCtrl->scan.frm_cnt, imaging_config.imageCtrl.frameCnt);
	CThordaq::LogMessage(errMsg,VERBOSE_EVENT);
	****/

	//gPtrAcqCtrl->scan.pll_fltr_ctrl   = 0x42; //Default Thordaq ADPLL Loop Filter settings
	//gPtrAcqCtrl->scan.pll_cntr_freq   = MAXULONG32 /imaging_config.imageCtrl.sample_rate  * 8000; //8khz
	
	gPtrAcqCtrl->scan.pll_sync_offset = max(1, imaging_config.imageCtrl.alignmentOffset);

	//Setup Galvo Settings
	if (imaging_config.imageCtrl.system_mode == SYSTEM_MODE::INTERNAL_GALVO_GALVO)
	{
		gPtrAcqCtrl->scan.galvo_pixel_delay = static_cast<ULONG32>(round(imaging_config.galvoGalvoCtrl.pixelDelayCnt));
		gPtrAcqCtrl->scan.galvo_pixel_dwell = static_cast<ULONG32>(round(imaging_config.galvoGalvoCtrl.dwellTime * (double)SYS_CLOCK_FREQ));
		gPtrAcqCtrl->scan.galvo_intra_line_delay  = static_cast<ULONG32>(round((imaging_config.galvoGalvoCtrl.turnaroundTime * (double)SYS_CLOCK_FREQ / 2.0  - 1.0) / 16.0));
		gPtrAcqCtrl->scan.galvo_intra_frame_delay = static_cast<ULONG32>(round(imaging_config.galvoGalvoCtrl.flybackTime * (double)SYS_CLOCK_FREQ  - 2.0) / 16.0);// Flyback_time resgiter = (flyback time  * acq_clk_freq - 2) / 16
	}
	else
	{
		gPtrAcqCtrl->scan.galvo_intra_frame_delay =static_cast<ULONG32>(imaging_config.resonantGalvoCtrl.flybackTime * imaging_config.streamingCtrl.scan_period);
	}

	return TRUE;
}

LONG ThordaqDFLIM::SetCoherentSampleingSettings(IMAGING_CONFIGURATION_STRUCT imaging_config)
{
	//Dont consider GG Scan with Conherent Sampling
	if(imaging_config.coherentSamplingCtrl.phaseIncrementMode > (USHORT)(0))
	{
		gPtrAcqCtrl->samplingClock.phase_ctrl   = static_cast<UCHAR>  (imaging_config.coherentSamplingCtrl.phaseIncrementMode - 1);
		gPtrAcqCtrl->samplingClock.phase_ctrl |= (imaging_config.imageCtrl.threePhotonMode == TRUE)? 0x08 : 0x00;
		gPtrAcqCtrl->samplingClock.phase_offset = static_cast<USHORT> (imaging_config.coherentSamplingCtrl.phaseOffset);
		gPtrAcqCtrl->samplingClock.phase_step   = static_cast<UCHAR>  (imaging_config.coherentSamplingCtrl.phaseStep);
		gPtrAcqCtrl->samplingClock.phase_limit  = static_cast<USHORT> (imaging_config.coherentSamplingCtrl.phaseLimit);
			
		gPtrAcqCtrl->streamProcessing.pulse_interleave_offset = 0;
		gPtrAcqCtrl->streamProcessing.pulse_interleave = 0;
		if (imaging_config.streamingCtrl.channel_multiplexing_enabled == TRUE)		{

			gPtrAcqCtrl->streamProcessing.pulse_interleave |= 0x0f;
			gPtrAcqCtrl->streamProcessing.pulse_interleave_offset = 0x11 ;
		}
		gPtrAcqCtrl->streamProcessing.pulse_interleave |= (imaging_config.imageCtrl.threePhotonMode == TRUE)? 0x80 : 0x00;
	}
	return TRUE;
}

LONG ThordaqDFLIM::SetStreamProcessingSettings(IMAGING_CONFIGURATION_STRUCT	imaging_config)
{
	double pixel_frequency = 0;
	double SampleRate = imaging_config.imageCtrl.clockRate;
	double lineTime = 0;

	if (imaging_config.imageCtrl.system_mode == SYSTEM_MODE::INTERNAL_GALVO_GALVO) // GG Settings
	{
		gPtrAcqCtrl->streamProcessing.stream_ctrl = 0x02; // Enable DC Offset Correction pre-FIR filter
		pixel_frequency = 1.0 / imaging_config.galvoGalvoCtrl.dwellTime;
		double lineSweepTime  = ((double)gPtrAcqCtrl->scan.galvo_pixel_dwell * (double)gPtrAcqCtrl->gblCtrl.hor_pix_num) + ((double)gPtrAcqCtrl->scan.galvo_pixel_delay * 16.0 + 1.0) * ((double)(gPtrAcqCtrl->gblCtrl.hor_pix_num - 1)); // (Dwell Time + Pixel Delay Time) * PixelX - Pixel Delay Time
		double turnAroundTime = 2.0 * ((double)gPtrAcqCtrl->scan.galvo_intra_line_delay * 16.0 + 1.0);
		lineTime = (lineSweepTime + turnAroundTime) / SYS_CLOCK_FREQ;
		/*wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg,MSG_SIZE,L"ThorDAQGalvoGalvo SetStreamProcessingSettings \n LineTime: %f", lineTime);
		LogMessage(errMsg,ERROR_EVENT);	*/		
	}
	else // GR Settings
	{
		gPtrAcqCtrl->streamProcessing.stream_ctrl  = 0x03; // enable Scan Period Source and DC Offset Correction pre-FIR filter
		lineTime = 1.0 / 2.0 / imaging_config.streamingCtrl.scan_period;
		pixel_frequency = 1.0 / (lineTime / static_cast<double>(imaging_config.imageCtrl.imgHSize));	
	}

	if (imaging_config.imageCtrl.threePhotonMode) //this part can be integrated into main logic. This block is for test purpose.
	{
		gPtrAcqCtrl->streamProcessing.scan_period = static_cast<USHORT>(round((double)imaging_config.imageCtrl.clockRate * lineTime)); //static_cast<USHORT>( imaging_config.imageCtrl.clockRate * lineTime );
		ULONG32 downSampleRate =  static_cast<ULONG32>(floor(ADC_MAX_SAMPLE_RATE / (double)imaging_config.imageCtrl.clockRate));
		if ((downSampleRate % 2) != 0)
		{
			--downSampleRate;
		}
		gPtrAcqCtrl->streamProcessing.downsample_rate = downSampleRate;

		gPtrAcqCtrl->streamProcessing.threePhoton_sample_offset = imaging_config.imageCtrl.threePhotonPhaseAlignment;
		return TRUE;
	}

	while (lineTime * SampleRate > USHRT_MAX)//16Bits
	{
		SampleRate = SampleRate / 2.0;
	}
	

	gPtrAcqCtrl->streamProcessing.downsample_rate = static_cast<ULONG32>(((ADC_MAX_SAMPLE_RATE / SampleRate) - 1) /** 16843009*/); //* (1+2^8+2^16+2^24);
	gPtrAcqCtrl->streamProcessing.scan_period     = static_cast<USHORT>(ceil(lineTime * SampleRate));          //Round the value and set in the galvo galvo system
	return TRUE;
}

LONG ThordaqDFLIM::SetWaveformPlayback( DAC_CRTL_STRUCT dac_setting, int channel, UINT8 set_flag )
{
	ULONG64 GalvoBitsMask = 0x000000000000ffff;
	USHORT park_mid = dac_setting.park_val > 0? 0x7fff: 0x8000;
	USHORT offset_mid = dac_setting.offset_val > 0? 0x7fff: 0x8000;
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;

	int bits_to_shift = ((channel  % 4) * 16);
	int index = static_cast<int>(floor(channel / 4));
	
	gPtrAcqCtrl->galvoCtrl.dacUpdateRate[index] = (gPtrAcqCtrl->galvoCtrl.dacUpdateRate[index] & ~(GalvoBitsMask << bits_to_shift)) |((ULONG64)round(SYS_CLOCK_FREQ/ dac_setting.update_rate - 1) << bits_to_shift);
	gPtrAcqCtrl->galvoCtrl.dacOffset[index] = (gPtrAcqCtrl->galvoCtrl.dacOffset[index] & ~(GalvoBitsMask << bits_to_shift)) |(static_cast<ULONG64>((dac_setting.offset_val / GALVO_RESOLUTION) + offset_mid) << bits_to_shift);
	gPtrAcqCtrl->galvoCtrl.dacParkValue[index] = (gPtrAcqCtrl->galvoCtrl.dacParkValue[index] & ~(GalvoBitsMask << bits_to_shift)) |(static_cast<ULONG64>((dac_setting.park_val / GALVO_RESOLUTION) + park_mid) << bits_to_shift);
	
	if (set_flag)
	{
		ULONGLONG update_rate_addr = 0x248 + index * 8; 
		ULONGLONG park_addr = 0x268 + index * 8;
		ULONGLONG offset_addr = 0x280 + index * 8;

		BYTE* buffer = new BYTE[8];
		STAT_STRUCT StatusInfo;
		
		memcpy(buffer,&gPtrAcqCtrl->galvoCtrl.dacUpdateRate[index],sizeof(ULONG64));
		WriteReadRegister(WRITE_TO_CARD,3,update_rate_addr,buffer,0,8,&StatusInfo);
		if (status != STATUS_SUCCESSFUL)
		{
			SAFE_DELETE_ARRAY(buffer);
			return FALSE;
		}

		memcpy(buffer,&gPtrAcqCtrl->galvoCtrl.dacOffset[index],sizeof(ULONG64));
		WriteReadRegister(WRITE_TO_CARD,3,offset_addr,buffer,0,8,&StatusInfo);
		if (status != STATUS_SUCCESSFUL)
		{
			SAFE_DELETE_ARRAY(buffer);
			return FALSE;
		}

		//TODO: Parking should only be set at startup and when it changes, probably should be the same for update rate
		//and offset. Figure out what is the best way forward.
		//memcpy(buffer,&gPtrAcqCtrl->galvoCtrl.dacParkValue[index],sizeof(ULONG64));
		//ThordaqErrChk(L"WriteReadRegister", status = WriteReadRegister(WRITE_TO_CARD,3,park_addr,buffer,0,8,&StatusInfo))
		//if (status != STATUS_SUCCESSFUL)
		//{
		//	SAFE_DELETE_ARRAY(buffer);
		//	return FALSE;
		//}

		SAFE_DELETE_ARRAY(buffer);
	}
	
	return TRUE;
}

LONG ThordaqDFLIM::SetDACSettings(IMAGING_CONFIGURATION_STRUCT	imaging_config)
{
	ULONG64 DMAChannelCountBitsMask = 0x000000000000000F;
	ULONG64 DMAChannelBitsMask		= 0x0000000000000010;
	ULONG64 LineSyncBitsMask		= 0x0000000040000000;

	ULONG64 DmaChannelCount  = 0x0000000000000000;
	ULONG64 max_flyback_samples = 80;
	gPtrAcqCtrl->galvoCtrl.dacChannelMap = 0;
	gPtrAcqCtrl->galvoCtrl.ctrlReg = 0;
	for (int i = 0; i < DAC_CHANNEL_COUNT; i++)
	{
			DAC_CRTL_STRUCT dac_setting = imaging_config.dacCtrl[i];
			if (dac_setting.output_port== 10)
			{
				dac_setting.output_port = 11;
			}
			else if (dac_setting.output_port == 11)
			{
				dac_setting.output_port = 10;
			}
			gPtrAcqCtrl->galvoCtrl.dacChannelMap += (dac_setting.output_port << (i * 4));
			
			gPtrAcqCtrl->galvoCtrl.ctrlReg = gPtrAcqCtrl->galvoCtrl.ctrlReg & (~DMAChannelCountBitsMask) | ((DmaChannelCount));
			gPtrAcqCtrl->galvoCtrl.ctrlReg = gPtrAcqCtrl->galvoCtrl.ctrlReg | (DMAChannelBitsMask << i);
			//if (imaging_config.imageCtrl.system_mode == SYSTEM_MODE::INTERNAL_RESONANT_GALVO || (imaging_config.imageCtrl.system_mode == SYSTEM_MODE::INTERNAL_GALVO_GALVO && imaging_config.imageCtrl.threePhotonMode))
			//{
				gPtrAcqCtrl->galvoCtrl.ctrlReg = gPtrAcqCtrl->galvoCtrl.ctrlReg | (LineSyncBitsMask << i);
			//}
			DmaChannelCount++;
			if (DAC_CHANNEL_COUNT - 1 > i)
			{
				if ((SYSTEM_MODE::INTERNAL_GALVO_GALVO == imaging_config.imageCtrl.system_mode && (int)GalvoResonantY == i) ||
					(SYSTEM_MODE::INTERNAL_RESONANT_GALVO == imaging_config.imageCtrl.system_mode && (int)GalvoGalvoX == i))
				{ 
					// If it is GG imaging, don't move the GR Y Galvo to offset 0, keep it in park instead. For GR imaging, keep GG X Galvo at park position (10V)
					dac_setting.offset_val = (double)GALVO_PARK_POSITION;
				}
				else if (SYSTEM_MODE::INTERNAL_RESONANT_GALVO == imaging_config.imageCtrl.system_mode && (int)GalvoGalvoY == i)
				{
					// If it is GR imaging, keep the GG Y galvo in park position (-10V)
					dac_setting.offset_val = -1.0 * (double)GALVO_PARK_POSITION;
				}
				SetWaveformPlayback(dac_setting,i, false);
			}
			max_flyback_samples = max(max_flyback_samples,dac_setting.flyback_samples);
	}	

	//gPtrAcqCtrl->galvoCtrl.dacChannelMap += (DAC_CHANNEL::DO0 << (12 * 4));

	gPtrAcqCtrl->galvoCtrl.doOffset = 0;
	gPtrAcqCtrl->galvoCtrl.doParkValue = 0;
	gPtrAcqCtrl->galvoCtrl.doUpdateRate = 0;//static_cast<ULONG64>(round(SYS_CLOCK_FREQ/ imaging_config.dacCtrl[DAC_CHANNEL_COUNT - 1].update_rate - 1));
	int bits_to_shift = ((0  % 4) * 16);
	int bits_to_shift2 = ((1  % 4) * 16);
	int bits_to_shift3 = ((2  % 4) * 16);
	int bits_to_shift4 = ((3  % 4) * 16);
	ULONG64 GalvoBitsMask = 0x000000000000ffff;
	gPtrAcqCtrl->galvoCtrl.doUpdateRate = (gPtrAcqCtrl->galvoCtrl.doUpdateRate & ~(GalvoBitsMask << bits_to_shift)) |((ULONG64)round(SYS_CLOCK_FREQ/ imaging_config.dacCtrl[DAC_CHANNEL_COUNT - 1].update_rate - 1) << bits_to_shift);
	gPtrAcqCtrl->galvoCtrl.doUpdateRate = (gPtrAcqCtrl->galvoCtrl.doUpdateRate & ~(GalvoBitsMask << bits_to_shift2)) |((ULONG64)round(SYS_CLOCK_FREQ/ imaging_config.dacCtrl[DAC_CHANNEL_COUNT - 1].update_rate - 1) << bits_to_shift2);
	gPtrAcqCtrl->galvoCtrl.doUpdateRate = (gPtrAcqCtrl->galvoCtrl.doUpdateRate & ~(GalvoBitsMask << bits_to_shift3)) |((ULONG64)round(SYS_CLOCK_FREQ/ imaging_config.dacCtrl[DAC_CHANNEL_COUNT - 1].update_rate - 1) << bits_to_shift3);
	gPtrAcqCtrl->galvoCtrl.doUpdateRate = (gPtrAcqCtrl->galvoCtrl.doUpdateRate & ~(GalvoBitsMask << bits_to_shift4)) |((ULONG64)round(SYS_CLOCK_FREQ/ imaging_config.dacCtrl[DAC_CHANNEL_COUNT - 1].update_rate - 1) << bits_to_shift4);
	
	gPtrAcqCtrl->galvoCtrl.dacStepSize[0] = 1;
	gPtrAcqCtrl->galvoCtrl.dacStepSize[1] = 1;
	gPtrAcqCtrl->galvoCtrl.dacStepSize[2] = 1;

	gPtrAcqCtrl->galvoCtrl.dacAmFilterWindow = max_flyback_samples;

	return TRUE;
}


/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM::StartAcquisition()
 *
 * @brief	Start acquisition.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS ThordaqDFLIM::StartAcquisition()
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;

	//TODO: check if lines below are necessary or can be removed
	/////////////////////////////////////////////////////////////////////////////
	//// line 42
	/////////////////////////////////////////////////////////////////////////////
	//SetI2C_0x2C0(0x01990000);
	//SetI2C_0x2C4(0x000529);
	//SetI2C_0x2C8_232();

	//Sleep(250);

	ResetBackEndAcq();  // added by gy
	
	if (gPtrAcqCtrl != nullptr)
	{
		gPtrAcqCtrl->gblCtrl.run_stop_mode = PACKET_GEN_RUN;
	}
	status = SetPacketMode(0,true, NULL, PACKET_MODE_ADDRESSABLE, 8192);
	return status;
}

THORDAQ_STATUS ThordaqDFLIM::SetPacketModeAddressable(bool enableStreamToDMA)
{
	THORDAQ_STATUS status;
	status = SetPacketMode(0, enableStreamToDMA, NULL, PACKET_MODE_ADDRESSABLE, 8192);
	return status;
}

/**********************************************************************************************//**
 * @fn	THORDAQ_STATUS ThordaqDFLIM::StopAcquisition()
 *
 * @brief	Stop acquisition.
 *
 * @author	Cge
 * @date	3/17/2017
 *
 * @return	STATUS_SUCCESSFUL if Thordaq returns Completion status. Others if problem happened.
 **************************************************************************************************/

THORDAQ_STATUS ThordaqDFLIM::StopAcquisition()
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	OVERLAPPED					os;			// OVERLAPPED structure for the operation
	DWORD						bytes_returned = 0;
	DWORD						last_error_status = 0;
	os.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (gPtrAcqCtrl != nullptr)
	{
		gPtrAcqCtrl->gblCtrl.run_stop_mode = PACKET_GEN_STOP;
		if (!DeviceIoControl(gHdlDevice, IMG_ACQ_CONF_IOCTL, (LPVOID)gPtrAcqCtrl, sizeof(DATA_ACQ_CTRL_STRUCT), NULL, 0, &bytes_returned, &os))
		{
			last_error_status = GetLastError();
			if (last_error_status == ERROR_IO_PENDING)
			{
				// Wait here (forever) for the Overlapped I/O to complete
				if (!GetOverlappedResult(gHdlDevice, &os, &bytes_returned, TRUE))
				{
					last_error_status = GetLastError();
	#if _DEBUG		
					printf("Packet Gen/Chk Control failed. Error = %d\n", last_error_status);
	#endif // _DEBUG
					status = STATUS_PARAMETER_SETTINGS_ERROR;
				}
			}
			else
			{
				// ioctl failed	
	#if _DEBUG	
				printf("Packet Generator Control failed. Error = %d\n", last_error_status);
	#endif		
				status = STATUS_PARAMETER_SETTINGS_ERROR;
			}
		}
		CloseHandle(os.hEvent);
		//Set Up LUT
		if (status == STATUS_SUCCESSFUL)
		{
			status = ReleasePacketBuffers(0);
		}
	}
	return status;
}

LONG ThordaqDFLIM::LoadDACDescriptors(DAC_CRTL_STRUCT* dac_settings)
{
	// Sort the _dacDescpList first
	DAC_DESCP_TABLE dmaDescpTable;
	memset(dmaDescpTable.descp, 0, sizeof(ULONG64) * DAC_DESCP_MAX_LEN); // initiate the table which is used to load into the Device
	DMADescriptor despTable[DAC_CHANNEL_COUNT];
	memset(despTable, 0, sizeof(DMADescriptor) * DAC_CHANNEL_COUNT);

	_dacDescpListIndex = DAC_CHANNEL_COUNT - 1;
	bool is_set_flag = true;
	for (ULONG64 i = 0; i < DAC_CHANNEL_COUNT; i++)
	{
		DAC_CRTL_STRUCT dac_setting = *(dac_settings+i);
		if (dac_setting.waveform_buffer_length == 0)
		{
			continue;
		}
		despTable[i].buf_addr  = dac_setting.waveform_buffer_start_address;
		despTable[i].length    = dac_setting.waveform_buffer_length;
		despTable[i].next_node = i;
		SetDMADescriptors(despTable[i], i, dmaDescpTable.descp, is_set_flag);
		if (!is_set_flag)
		{
			return FALSE; 
		}
	}

	OVERLAPPED					os;			// OVERLAPPED structure for the operation
	DWORD						bytes_returned = 0;
	DWORD						last_error_status = 0;
	os.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	// Send Write PCI Configuration IOCTL
	if (!DeviceIoControl( gHdlDevice, DAC_DESC_SETUP_IOCTL, (LPVOID) &dmaDescpTable, sizeof(DAC_DESCP_TABLE), NULL, 0, &bytes_returned, &os))
	{
		last_error_status = GetLastError();
		if (last_error_status != ERROR_IO_PENDING)
		{			
			//Status->CompletedByteCount = 0;	
			printf("ScanLUTSetup IOCTL call failed. Error = %d\n", last_error_status);
			return FALSE;
		}
		// Wait here (forever) for the Overlapped I/O to complete
		if (!GetOverlappedResult(gHdlDevice, &os, &bytes_returned, TRUE))
		{
			last_error_status = GetLastError();
#if _DEBUG		
			printf("Packet Gen/Chk Control failed. Error = %d\n", last_error_status);
#endif // _DEBUG
			return FALSE;
		}
	}

	CloseHandle(os.hEvent);
	return TRUE;
}

void ThordaqDFLIM::SetDMADescriptors(DMADescriptor& dmaDescp, ULONG64 index, ULONG64* dmaDescpTable, bool& status)
{
	if (dmaDescp.length <= DAC_TRANSMIT_BUFFER_MAX)
	{
		*(dmaDescpTable+index) = dmaDescp.next_node << 52 | dmaDescp.length << 36 | dmaDescp.buf_addr;
	}else
	{
		if (_dacDescpListIndex > DAC_DESCP_MAX_LEN - 2) // reach the end of the discriptor table
		{
			status = false;
			return;
		}
		// Because waveform data includes 1024 words at the end to leave the enough space to flush the FIFO. So must leave 1024 words space at the end of last discriptor. 
		// If it doesn't, put some data (choose 2048 samples here) to the next discriptor.
		if ((dmaDescp.length - DAC_TRANSMIT_BUFFER_MAX) <= (DAC_FIFO_DEPTH / 2))
		{
			*(dmaDescpTable+index) = (_dacDescpListIndex + 1) << 52 | (ULONG64)(DAC_TRANSMIT_BUFFER_MAX - DAC_FIFO_DEPTH) << 36 | dmaDescp.buf_addr;
			dmaDescp.length = dmaDescp.length - (ULONG64)(DAC_TRANSMIT_BUFFER_MAX - DAC_FIFO_DEPTH);
			dmaDescp.buf_addr = dmaDescp.buf_addr + (ULONG64)(DAC_TRANSMIT_BUFFER_MAX - DAC_FIFO_DEPTH);
		}else
		{
			*(dmaDescpTable+index) = (_dacDescpListIndex + 1) << 52 | (ULONG64)DAC_TRANSMIT_BUFFER_MAX << 36 | dmaDescp.buf_addr;
			dmaDescp.length = dmaDescp.length - (ULONG64)DAC_TRANSMIT_BUFFER_MAX;
			dmaDescp.buf_addr = dmaDescp.buf_addr + (ULONG64)DAC_TRANSMIT_BUFFER_MAX;
		}
		SetDMADescriptors(dmaDescp,++_dacDescpListIndex,dmaDescpTable,status);
	}
}

THORDAQ_STATUS ThordaqDFLIM::SetDACParkValue(ULONG32 outputChannel, double parkValue)
{
	//if (outputChannel > 11 || parkValue < -10.0 || parkValue > 10.0 )
	//{
	//	return STATUS_PARAMETER_SETTINGS_ERROR;
	//}
	//DAC_CRTL_STRUCT dac_setting;
	//memset(&dac_setting,0,sizeof(DAC_CRTL_STRUCT));
	//dac_setting.park_val = outputValue;
	//SetWaveformPlayback(dac_setting, outputChannel, true);


	if (outputChannel > 11 || parkValue < -10.0 || parkValue > 10.0 )
	{
		return STATUS_PARAMETER_SETTINGS_ERROR;
	}

	USHORT park_mid = parkValue > 0? 0x7fff: 0x8000;
	int index = static_cast<int>(floor(outputChannel / 4));
	USHORT dacUpdateRate = static_cast<USHORT>(std::floor(SYS_CLOCK_FREQ/ DAC_MIN_UPDATERATE + 0.5));
	USHORT dacParkValue = static_cast<USHORT>(std::floor(parkValue / GALVO_RESOLUTION + 0.5) + park_mid);

	ULONGLONG update_rate_addr = 0x248 + outputChannel * 2; 
	ULONGLONG park_addr = 0x268  + outputChannel * 2;

	STAT_STRUCT StatusInfo;
	BYTE* buffer = new BYTE[2];		
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;
	memcpy(buffer,&dacUpdateRate,sizeof(USHORT));

	status = WriteReadRegister(WRITE_TO_CARD,3,update_rate_addr,buffer,0,2,&StatusInfo);

	if (status!= STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		return status;
	}

	memcpy(buffer,&dacParkValue,sizeof(USHORT));
	status = WriteReadRegister(WRITE_TO_CARD,3,park_addr,buffer,0,2,&StatusInfo);
	if (status != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		return status;
	}

	SAFE_DELETE_ARRAY(buffer);

	return STATUS_SUCCESSFUL;
}


LONG ThordaqDFLIM::SetDACChannelMapping()
{
	ULONG64 dacChannelMap = 0x0000BA9876543210;
	STAT_STRUCT StatusInfo;

	gPtrAcqCtrl->galvoCtrl.dacChannelMap = dacChannelMap;

	if (WriteReadRegister(WRITE_TO_CARD, 3, 0x298, (BYTE*)&dacChannelMap , 0, 8, &StatusInfo) != STATUS_SUCCESSFUL) return FALSE;
	
	return TRUE;
}

THORDAQ_STATUS ThordaqDFLIM::SetADCChannelMapping()
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	INT32 error = 0;

	gPtrAcqCtrl->gblCtrl.gpio_cfg = (ULONG64)0; //0x0706050403020100;

	gPtrAcqCtrl->gblCtrl.gpio_cfg = ((static_cast<ULONG64>(CH0))) 
		| ((static_cast<ULONG64>(CH1)) << 8)
		| ((static_cast<ULONG64>(CH2)) << 16)
		| ((static_cast<ULONG64>(CH3)) << 24)
		| ((static_cast<ULONG64>(CH4)) << 32)
		| ((static_cast<ULONG64>(CH5)) << 40)
		| ((static_cast<ULONG64>(CH6)) << 48)
		| ((static_cast<ULONG64>(CH7)) << 56);

	BYTE* GPIOBuffer = new BYTE[8];
	STAT_STRUCT StatusInfo;
	memcpy(GPIOBuffer,&gPtrAcqCtrl->gblCtrl.gpio_cfg,sizeof(ULONG64));
	status = WriteReadRegister(WRITE_TO_CARD, 3, 0x020, GPIOBuffer, 0 , 8, &StatusInfo);
	if (status != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY( GPIOBuffer);
		return STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY( GPIOBuffer);
	
	return status;
}


THORDAQ_STATUS ThordaqDFLIM::GetLineTriggerFrequency(UINT32 sample_rate, double& frequency)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	BYTE* buffer = new BYTE[2];
	USHORT val = 0x0000;
	memcpy(buffer,&val,sizeof(USHORT));
	STAT_STRUCT StatusInfo;
	status = WriteReadRegister(READ_FROM_CARD,3,0x140,buffer,0,2,&StatusInfo);
	memcpy(&val,buffer,sizeof(USHORT));
	if (status == STATUS_SUCCESSFUL)
	{
		if (val != 0 && val < 0x5120 && val > 0x4D2B)
		{
			frequency = static_cast<double>(sample_rate) /  static_cast<double>(val);
		}else
		{
			frequency = CrsFrequencyHighPrecision;
		}
		
	}else
	{
		status = STATUS_READWRITE_REGISTER_ERROR;;	
	}
	SAFE_DELETE_ARRAY(buffer);
	return 	status;
}


THORDAQ_STATUS ThordaqDFLIM::GetTotalFrameCount(UINT32& frame_count)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	BYTE* buffer = new BYTE[2];
	memcpy(buffer,&frame_count,sizeof(USHORT));
	STAT_STRUCT StatusInfo;
	status = WriteReadRegister(READ_FROM_CARD,3,0x142,buffer,0,2,&StatusInfo);
	memcpy(&frame_count,buffer,sizeof(USHORT));
	if (status != STATUS_SUCCESSFUL)
	{
		status = STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return status;
}


THORDAQ_STATUS ThordaqDFLIM::GetClockFrequency(int clockIndex, double& frequency)
{
	ULONG clock = (ULONG)clockIndex;
	frequency = 0;
	double averagedValue = 0;
	const long AVGTIMES = 5;
	for (int i = 0; i < AVGTIMES; ++i)
	{	
		SetNONI2C_0x1C0(clock);
		SetNONI2C_0x1C4(0x10000600);
		SetNONI2C_0x1C8(0x01);
		SetNONI2C_0x1C8(0x00);

		Sleep(5);

		SetNONI2C_0x1C0(0x00000000);
		SetNONI2C_0x1C4(0x20000601);
		SetNONI2C_0x1C8(0x01);
		SetNONI2C_0x1C8(0x00);

		THORDAQ_STATUS status = STATUS_SUCCESSFUL;
		const long readLength = 8;
		ULONG64 value;
		STAT_STRUCT StatusInfo;
		status = WriteReadRegister(READ_FROM_CARD,3,0x1c0,(BYTE*)(&value),0,readLength,&StatusInfo);
		if (status != STATUS_SUCCESSFUL)
		{
			frequency = 0;
			status = STATUS_READWRITE_REGISTER_ERROR;
		}

		averagedValue+=(value & 0xFFFF);
	}

	averagedValue/=AVGTIMES;

	frequency = 200*(averagedValue + 1) / 8192;

	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqDFLIM::SetCoarseShift(ULONG32 shift, int channel)
{
	STAT_STRUCT StatusInfo;
	USHORT shortShift = static_cast<USHORT>(shift);


	switch (channel)
	{
	case 0:
		if (WriteReadRegister(WRITE_TO_CARD,3,0x318,(BYTE*)(&shortShift),0,2,&StatusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
		break;
	case 1:
		if (WriteReadRegister(WRITE_TO_CARD,3,0x358,(BYTE*)(&shortShift),0,2,&StatusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
		break;
	case 2:
		if (WriteReadRegister(WRITE_TO_CARD,3,0x398,(BYTE*)(&shortShift),0,2,&StatusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
		break;
	case 3:
		if (WriteReadRegister(WRITE_TO_CARD,3,0x3D8,(BYTE*)(&shortShift),0,2,&StatusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
		break;
	}

	ReSync();

	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqDFLIM::SetFineShift(LONG32 shift, int channel)
{
	STAT_STRUCT StatusInfo;

	SHORT shortShift = static_cast<SHORT>(shift) * 4;

	switch (channel)
	{
	case 0:
		if (WriteReadRegister(WRITE_TO_CARD,3,0x320,(BYTE*)(&shortShift),0,2,&StatusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
		break;
	case 1:
		if (WriteReadRegister(WRITE_TO_CARD,3,0x360,(BYTE*)(&shortShift),0,2,&StatusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
		break;
	case 2:
		if (WriteReadRegister(WRITE_TO_CARD,3,0x3A0,(BYTE*)(&shortShift),0,2,&StatusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
		break;
	case 3:
		if (WriteReadRegister(WRITE_TO_CARD,3,0x3E0,(BYTE*)(&shortShift),0,2,&StatusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
		break;
	}

	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqDFLIM::SetDFLIMSyncingSettings(ULONG32 syncDelay, ULONG32 resyncDelay, bool forceResyncEverLine)
{
	USHORT shortShift = static_cast<USHORT>(syncDelay);

	ULONG value = (USHORT)syncDelay | ((USHORT)resyncDelay << 16) | (forceResyncEverLine << 31);

	SetNONI2C_0x1C0(value); 
	SetNONI2C_0x1C4(0x10000011);
	SetNONI2C_0x1C8(0x01);
	SetNONI2C_0x1C8(0x00);

	ReSync();

	return STATUS_SUCCESSFUL;
}


THORDAQ_STATUS ThordaqDFLIM::ResetBackEndAcq()
{
	STAT_STRUCT StatusInfo;

	LONG backEndReset = 1;
	LONG backEndNorm  = 0;

	if (WriteReadRegister(WRITE_TO_CARD,3,0x300,(BYTE*)(&backEndReset),0,4,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (WriteReadRegister(WRITE_TO_CARD,3,0x340,(BYTE*)(&backEndReset),0,4,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (WriteReadRegister(WRITE_TO_CARD,3,0x380,(BYTE*)(&backEndReset),0,4,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (WriteReadRegister(WRITE_TO_CARD,3,0x3C0,(BYTE*)(&backEndReset),0,4,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (WriteReadRegister(WRITE_TO_CARD,3,0x300,(BYTE*)(&backEndNorm),0,4,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (WriteReadRegister(WRITE_TO_CARD,3,0x340,(BYTE*)(&backEndNorm),0,4,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (WriteReadRegister(WRITE_TO_CARD,3,0x380,(BYTE*)(&backEndNorm),0,4,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (WriteReadRegister(WRITE_TO_CARD,3,0x3C0,(BYTE*)(&backEndNorm),0,4,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	return STATUS_SUCCESSFUL;
}


THORDAQ_STATUS ThordaqDFLIM::ReSync()
{
	// recalibrates the VCO (in case the laser freq is very different or was absent before)
	//   and then re-syncs the digitizer

	/////////////////////////////////////////////////////////////////////////////
	//// line 42
	/////////////////////////////////////////////////////////////////////////////
	//if (SetI2C_0x2C0(0x01990000) != STATUS_SUCCESSFUL)
	//{
	//	return STATUS_READWRITE_REGISTER_ERROR;
	//}

	//if (SetI2C_0x2C4(0x000529) != STATUS_SUCCESSFUL)
	//{
	//	return STATUS_READWRITE_REGISTER_ERROR;
	//}

	//if (SetI2C_0x2C8_232() != STATUS_SUCCESSFUL)
	//{
	//	return STATUS_READWRITE_REGISTER_ERROR;
	//}

	//SetDLIMFrontEndSettings();

	if (1) 
	{
	//	// this section recalibrates the VCO.  But it seems to have created more trouble
	//	// perhaps because the wait after recalibration is not quite long enough

	//	///////////////////////////////////////////////////////////////////////////
	//	// line 41 Set SPI mode
	//	///////////////////////////////////////////////////////////////////////////
	//	SetI2C_0x2C0(0xf0ffff0b);
	//	SetI2C_0x2C4(0x000329);
	//	SetI2C_0x2C8_232();

	//	Sleep(FRONT_END_SETUP_SLEEP1);

		//////////////////////////////////////////////////////////////////////////
		// line 81    Clock chip: 	0x018	0x26	set VCO cal now 0
		///////////////////////////////////////////////////////////////////////////
		SetI2C_0x2C0(0x01261800);
		SetI2C_0x2C4(0x000529);
		SetI2C_0x2C8_232();

		Sleep(FRONT_END_SETUP_SLEEP1);
	
		//////////////////////////////////////////////////////////////////////////
		// line 89    Clock chip: 	0x232	0x01	update all registers
		///////////////////////////////////////////////////////////////////////////
		SetI2C_0x2C0(0x01013202);
		SetI2C_0x2C4(0x000529);
		SetI2C_0x2C8_232();

		Sleep(FRONT_END_SETUP_SLEEP2);

		//////////////////////////////////////////////////////////////////////////
		// line 81    Clock chip: 	0x018	0x27	set VCO cal now 1
		///////////////////////////////////////////////////////////////////////////
		SetI2C_0x2C0(0x01271800);
		SetI2C_0x2C4(0x000529);
		SetI2C_0x2C8_232();

		Sleep(FRONT_END_SETUP_SLEEP3);

		//////////////////////////////////////////////////////////////////////////
		// line 89    Clock chip: 	update all reg
		///////////////////////////////////////////////////////////////////////////
		SetI2C_0x2C0(0x01013202);
		SetI2C_0x2C4(0x000529);
		SetI2C_0x2C8_232();

		Sleep(30);
	};

	//////////////////////////////////////////////////////////////////////////
	// ReSync Command
	///////////////////////////////////////////////////////////////////////////
	SetNONI2C_0x1C0(0x00000080);
	SetNONI2C_0x1C4(0x10000010);
	SetNONI2C_0x1C8(0x01);
	SetNONI2C_0x1C8(0x00);

	Sleep(FRONT_END_SETUP_SLEEP2);

	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS GetDACSamplesPerLine(UINT32& samples,double& dac_rate, double line_time)
{
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqDFLIM::GetExternClockStatus(ULONG32& isClockedSynced)
{
	STAT_STRUCT StatusInfo;
	BYTE* buffer_read = new BYTE[1];
	if (WriteReadRegister(READ_FROM_CARD,3,0x180,buffer_read,0,1,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer_read);
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	isClockedSynced = (*buffer_read & 0x10) ? 1 : 0;
	SAFE_DELETE_ARRAY(buffer_read);
	return STATUS_SUCCESSFUL;
}



THORDAQ_STATUS ThordaqDFLIM::StartImaging()
{
	STAT_STRUCT StatusInfo;
	// run dma
	BYTE runDMAByte=  0x13;
	if (WriteReadRegister(WRITE_TO_CARD,2,0x000,(BYTE*)(&runDMAByte),0,1,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}
	//To Set GIGCR0_STOP_RUN (RUN)
	BYTE startByte = 0x01;
	if (WriteReadRegister(WRITE_TO_CARD,3,0x00,(BYTE*)(&startByte),0,1,&StatusInfo) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqDFLIM::SetDLIMFrontEndSettings()
{
	int mode = 0;
	if (mode == 2)
	{
//		WriteMem 3 0x1C0 1 0x00
//Delay 250
//
/////////////////////////////////////////////////////////////////////////////
//// set max_cnt (#of beats)
/////////////////////////////////////////////////////////////////////////////
//WriteMem 3 0x1C8 4 0x00001824
//WriteMem 3 0x1Cc 4 0x00000000
		STAT_STRUCT statusInfo;
		BYTE byte = 0x00;
		if (WriteReadRegister(WRITE_TO_CARD, 3, 0x1C0, (BYTE*)(&byte), 0 , 1, &statusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}

		ULONG mxBeatsCount = 0x00001824;
		if (WriteReadRegister(WRITE_TO_CARD, 3, 0x1C8, (BYTE*)(&mxBeatsCount), 0 , 4, &statusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}

		ULONG mxBeatsCount2 = 0x00001824;
		if (WriteReadRegister(WRITE_TO_CARD, 3, 0x1Cc, (BYTE*)(&mxBeatsCount2), 0 , 4, &statusInfo) != STATUS_SUCCESSFUL)
		{
			return STATUS_READWRITE_REGISTER_ERROR;
		}
	}
	else 
	{
	// set up ThorDAQ_I2C_Mode and perform a !RESET to PCA9548A
	SetI2C_0x2C8(0x00);
	SetI2C_0x2C8(0x02);

	///////////////////////////////////////////////////////////////////////////
	// Access channel 1 PCA9548A
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0xffffff02);
	SetI2C_0x2C4(0x000271);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 33    FMC12x CPLD: 	0x00	0x2B	sets reset bit for SPIclocktree
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x08ff2b00);
	SetI2C_0x2C4(0x000429);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 34    FMC12x CPLD: 	0x00	0x0B	clear reset
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x08ff0b00);
	SetI2C_0x2C4(0x000429);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 35    FMC12x CPLD: 	0x00	0x4B	sets reset bit for SPIadc
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x08ff4b00);
	SetI2C_0x2C4(0x000429);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 36    FMC12x CPLD: 	0x00	0x0B	clear reset and set clock routing: internal with ext ref, sync from host
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x08ff0b00);
	SetI2C_0x2C4(0x000429);
	SetI2C_0x2C8_232();
	
	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 37    FMC12x CPLD: 	0x01	0x00	all fans enabled, all HDMI (frontio) signals are inputs
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x08ff0001);
	SetI2C_0x2C4(0x000429);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 38    FMC12x CPLD: 	0x02	0x00	LED off
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x08ff0202);
	SetI2C_0x2C4(0x000429);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 41 Set SPI mode
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0xf0ffff0b);
	SetI2C_0x2C4(0x000329);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 42	Clock chip:		0x00	0x99	Set AD9517 in 4-wire mode (SDO enabled)
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01990000);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 43   Clock chip:	0x10	0x7C	CP 4.8mA, normal op
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x017c1000);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 44    Clock chip: 	0x11	0x01	R lo
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01011100);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 45    Clock chip: 	0x12	0x00	R hi
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01001200);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 46    Clock chip: 	0x13	0x02	A
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01021300);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 47    Clock chip: 	0x14	0x0F	B lo
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x010f1400);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 48    Clock chip: 	0x15	0x00	B hi
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01001500);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 49    Clock chip: 	0x16	0x06	Prescaler dual modulo 32
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01061600);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 50    Clock chip: 	0x17	0xB4	Status = DLD
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01b41700);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 51    Clock chip: 	0x19	0x00	No SYNC pin reset of dividers
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01001900);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 52    Clock chip: 	0x1A	0x00	LD = DLD
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01001a00);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 53    Clock chip: 	0x1B	0x00	REFMON = GND
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01001b00);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 54    Clock chip: 	0x1C	0x87	Diff ref input
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01871c00);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 55    Clock chip: 	0x1D	0x00	PLL control refs off
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01001d00);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(205);

	//////////////////////////////////////////////////////////////////////////
	// line 56    Clock chip: 	0x0F0	0x02	out0, safe power down
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x0102f000);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 57    Clock chip: 	0x0F1	0x0C	out1, lvpecl 960mW
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x010cf100);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 58    Clock chip: 	0x0F4	0x02	out2, safe power down
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x0102f400);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 59    Clock chip: 	0x0F5	0x0C	out3, adc, lvpecl 960mW
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x010cf500);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 60    Clock chip: 	0x140	0x01	out4, sync, pd
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01014001);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 61    Clock chip: 	0x141	0x01	out5, pd
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01014101);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 62    Clock chip: 	0x142	0x00	out6, lvds 1.75mA
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01004201);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 63    Clock chip: 	0x143	0x01	out7, pd
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01014301);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 64    Clock chip: 	0x190	0xBC	div0, clk out, /50 (50MHz) :  clk out (external clock output) no longer used 
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01bc9001);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();
	
	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 65    Clock chip: 	0x191	0x80	div0, clk out, bypass
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01809101);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 66    Clock chip: 	0x192	0x02	div0, clk out, clk to output
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01029201);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 67    Clock chip: 	0x196	0x22	div1, adc, /2
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01229601);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 68    Clock chip: 	0x197	0x80	div1, adc, divider bypassed
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01809701);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 69    Clock chip: 	0x198	0x02	div1, adc, clk to output
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01029801);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 70    Clock chip: 	0x199	0x00	div2.1, /2
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01009901);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 71    Clock chip: 	0x19A	0x00	phase
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01009a01);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 72    Clock chip: 	0x19B	0x00	div2.2, /2
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01009b01);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 73    Clock chip: 	0x19C	0x00	div2.1 on, div2.2 on
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01009c01);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 74    Clock chip: 	0x19D	0x00	div2 dcc on
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01009d01);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 75    Clock chip: 	0x19E	0x00	div3.1, /2
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01009e01);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 76    Clock chip: 	0x19F	0x00	phase
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01009f01);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 77    Clock chip: 	0x1A0	0x00	div3.2, /2
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x0100a001);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 78    Clock chip: 	0x1A1	0x00	div3.1 on, div3.2 on
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x0100a101);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 79    Clock chip: 	0x1A2	0x00	div3 dcc on
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x0100a201);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 80    Clock chip: 	0x1E0	0x00	vco div /2
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x0100e001);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 81    Clock chip: 	0x018	0x26	set VCO cal now 0
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01261800);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 82    Clock chip: 	0x1E1	0x02	use internal vco and vco divider
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x0102e101);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 83    Clock chip: 	0x19E	0x00	div3.1 /2
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01009e01);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 84    Clock chip: 	0x19F	0x00	phase
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01009f01);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 85    Clock chip: 	0x1A0	0x00	div3.2 /2
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x0100a001);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 86    Clock chip: 	0x1A1	0x20	div3.1 on, div3.2 BYPASS (gy for Drift; was 0x00 for both dividers on)
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x0120a101);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 87    Clock chip: 	0x1A2	0x00	div3 dcc on
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x0100a201);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 88    Clock chip: 	0x230	0x00	no pwd, no sync
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01003002);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 89    Clock chip: 	0x232	0x01	update all registers
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01013202);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP2);

	//////////////////////////////////////////////////////////////////////////
	// line 81    Clock chip: 	0x018	0x27	set VCO cal now 1
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01271800);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 89    Clock chip: 	update all reg
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x01013202);
	SetI2C_0x2C4(0x000529);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP2);



	//////////////////////////////////////////////////////////////////////////
	// line 93 Set SPI mode
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0xf0ffff03);
	SetI2C_0x2C4(0x000329);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	///////////////////////////////////////////////////////////////////////////
	// line 94 non-I2C:  set the IDELAY for the Trigger
	///////////////////////////////////////////////////////////////////////////
	//SetNONI2C_0x1C0(0x00000000);  // sets IDELAY and SyncDelay - see register docs for FrontEnd register 0x0000011
	//SetNONI2C_0x1C4(0x10000011);
	//SetNONI2C_0x1C8(0x01);
	//SetNONI2C_0x1C8(0x00);


	//Sleep(FRONT_END_SETUP_SLEEP1);

	////////////////////////////////////////////////////////////////////////////
	//// line 95	EV10 chip:	0x05	0x01	Set flashing pattern for test.   GY: Need to check these addresses?
	/////////////////////////////////////////////////////////////////////////////
	//SetI2C_0x2C0(0x02010085);
	//SetI2C_0x2C4(0x000529);
	//SetI2C_0x2C8_232();

	//Sleep(FRONT_END_SETUP_SLEEP1);

	////////////////////////////////////////////////////////////////////////////
	//// line 96	EV10 chip:	0x01	0x0100	Set test mode on.   GY: Need to check these addresses/values ?
	/////////////////////////////////////////////////////////////////////////////
	//SetI2C_0x2C0(0x02401081);
	//SetI2C_0x2C4(0x000529);
	//SetI2C_0x2C8_232();


	///////////////////////////////////////////////////////////////////////////
	// line 97 non-I2C:	FrontEnd register 0x0000010: reset the clock buffer and IDELAYs
	///////////////////////////////////////////////////////////////////////////
	SetNONI2C_0x1C0(0x00000003);
	SetNONI2C_0x1C4(0x10000010);
	SetNONI2C_0x1C8(0x01);
	SetNONI2C_0x1C8(0x00);

	Sleep(FRONT_END_SETUP_SLEEP2);

	/////////////////////////////////////////////////////////////////////////////
	//// line 98 non-I2C:	FrontEnd register 0x0000010: request firmware automatic phasing of inputs
	/////////////////////////////////////////////////////////////////////////////
	//SetNONI2C_0x1C0(0x00000008);
	//SetNONI2C_0x1C4(0x10000010);
	//SetNONI2C_0x1C8(0x01);
	//SetNONI2C_0x1C8(0x00);

	//Sleep(FRONT_END_SETUP_SLEEP1);

	////////////////////////////////////////////////////////////////////////////
	//// line 99	EV10 chip:	0x01	0x0000	Set test mode off.   GY: Need to check these addresses?
	/////////////////////////////////////////////////////////////////////////////
	//SetI2C_0x2C0(0x02400081);
	//SetI2C_0x2C4(0x000529);
	//SetI2C_0x2C8_232();

	//Sleep(FRONT_END_SETUP_SLEEP2);

	Sleep(FRONT_END_SETUP_SLEEP3);

	///////////////////////////////////////////////////////////////////////////
	// line 100 non-I2C:	FrontEnd register 0x0000010: request firmware to generate ADC SYNC
	///////////////////////////////////////////////////////////////////////////
	SetNONI2C_0x1C0(0x00000080);
	SetNONI2C_0x1C4(0x10000010);
	SetNONI2C_0x1C8(0x01);
	SetNONI2C_0x1C8(0x00);

	Sleep(FRONT_END_SETUP_SLEEP2);

	//////////////////////////////////////////////////////////////////////////
	// line 104  Temperature monitor setup ????? GY ?????
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x18ffff2d);
	SetI2C_0x2C4(0x00034b);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 105  Temperature monitor setup ????? GY ?????
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0x1affff10);
	SetI2C_0x2C4(0x00034b);
	SetI2C_0x2C8_232();

	Sleep(FRONT_END_SETUP_SLEEP1);

	//////////////////////////////////////////////////////////////////////////
	// line 106  Temperature monitor read ????? GY ?????
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0xffffff07);
	SetI2C_0x2C4(0x0142cb);
	SetI2C_0x2C8_232();


	Sleep(FRONT_END_SETUP_SLEEP1);

	ReadI2C_0x2C0(1);

	//////////////////////////////////////////////////////////////////////////
	// line 107  Temperature monitor read ????? GY ?????
	///////////////////////////////////////////////////////////////////////////
	SetI2C_0x2C0(0xffffff08);
	SetI2C_0x2C4(0x0142cb);
	SetI2C_0x2C8_232();


	Sleep(FRONT_END_SETUP_SLEEP1);

	ReadI2C_0x2C0(1);

	///////////////////////////////////////////////////////////////////////////
	// verify   Checking phasing of inputs ????? GY ????? not used
	///////////////////////////////////////////////////////////////////////////
	SetNONI2C_0x1C0(0x00000000);
	SetNONI2C_0x1C4(0x20000010);
	SetNONI2C_0x1C8(0x01);
	SetNONI2C_0x1C8(0x00);

	Sleep(FRONT_END_SETUP_SLEEP1);

	ReadNONI2C_0x1C0(8);

	Sleep(FRONT_END_SETUP_SLEEP1);

	// later this (and coarseShifts, fineShifts, etc.) should be loaded from a config file and set separately for each channel

	///////////////////////////////////////////////////////////////////////////
	// Delay val for SYNC signal (IDELAY giving output SYNC delay)
	///////////////////////////////////////////////////////////////////////////
	SetNONI2C_0x1C0(0x00000003); // value is here
	SetNONI2C_0x1C4(0x10000011);
	SetNONI2C_0x1C8(0x01);
	SetNONI2C_0x1C8(0x00);

	//SetCoarseShift(3); 
	//SetFineShift(-24);

	Sleep(FRONT_END_SETUP_SLEEP1);

	
	}
	return STATUS_SUCCESSFUL;
}


THORDAQ_STATUS ThordaqDFLIM::SetNONI2C_0x1C0(ULONG bytes)
{
	STAT_STRUCT statusInfo;
	BYTE * buffer = new BYTE[4];  //0x00;
	memcpy(buffer,&bytes,sizeof(UCHAR)*4);
	if (WriteReadRegister(WRITE_TO_CARD, 3, 0x1C0, buffer, 0 , 4, &statusInfo) != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		return STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqDFLIM::SetNONI2C_0x1C4(ULONG bytes)
{
	STAT_STRUCT statusInfo;
	BYTE * buffer = new BYTE[4];  //0x00;
	memcpy(buffer,&bytes,sizeof(UCHAR)*4);
	if (WriteReadRegister(WRITE_TO_CARD, 3, 0x1C4, buffer, 0 , 4, &statusInfo) != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		return STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqDFLIM::SetNONI2C_0x1C8(BYTE byte)
{
	STAT_STRUCT statusInfo;
	BYTE * buffer = new BYTE[1];  //0x00;
	memcpy(buffer,&byte,sizeof(UCHAR));
	if (WriteReadRegister(WRITE_TO_CARD, 3, 0x1C8, buffer, 0 , 1, &statusInfo) != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		return STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqDFLIM::SetI2C_0x2C0(ULONG bytes)
{
	STAT_STRUCT statusInfo;
	BYTE * buffer = new BYTE[4];  //0x00;
	memcpy(buffer,&bytes,sizeof(UCHAR)*4);
	if (WriteReadRegister(WRITE_TO_CARD, 3, 0x2C0, buffer, 0 , 4, &statusInfo) != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		return STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqDFLIM::SetI2C_0x2C4(ULONG bytes)
{
	STAT_STRUCT statusInfo;
	BYTE * buffer = new BYTE[3];  //0x00;
	memcpy(buffer,&bytes,sizeof(UCHAR)*3);
	if (WriteReadRegister(WRITE_TO_CARD, 3, 0x2C4, buffer, 0 , 3, &statusInfo) != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		return STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqDFLIM::SetI2C_0x2C8(BYTE byte)
{
	STAT_STRUCT statusInfo;
	BYTE * buffer = new BYTE[1];  //0x00;
	memcpy(buffer,&byte,sizeof(UCHAR));
	if (WriteReadRegister(WRITE_TO_CARD, 3, 0x2C8, buffer, 0 , 1, &statusInfo) != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		return STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return STATUS_SUCCESSFUL;
}


THORDAQ_STATUS ThordaqDFLIM::SetI2C_0x2C8_232()
{
	if (SetI2C_0x2C8(0x02) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (SetI2C_0x2C8(0x03) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	if (SetI2C_0x2C8(0x02) != STATUS_SUCCESSFUL)
	{
		return STATUS_READWRITE_REGISTER_ERROR;
	}

	return STATUS_SUCCESSFUL;
}

THORDAQ_STATUS ThordaqDFLIM::ReadI2C_0x2C0(ULONG length)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	BYTE* buffer = new BYTE[length];
	STAT_STRUCT StatusInfo;
	status = WriteReadRegister(READ_FROM_CARD,3,0x2c0,buffer,0,length,&StatusInfo);
	if (status != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		status = STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return status;
}

THORDAQ_STATUS ThordaqDFLIM::ReadNONI2C_0x1C0(ULONG length)
{
	THORDAQ_STATUS status = STATUS_SUCCESSFUL;
	BYTE* buffer = new BYTE[length];
	STAT_STRUCT StatusInfo;
	status = WriteReadRegister(READ_FROM_CARD,3,0x1c0,buffer,0,length,&StatusInfo);
	if (status != STATUS_SUCCESSFUL)
	{
		SAFE_DELETE_ARRAY(buffer);
		status = STATUS_READWRITE_REGISTER_ERROR;
	}
	SAFE_DELETE_ARRAY(buffer);
	return status;
}

//THORDAQ_STATUS status = STATUS_SUCCESSFUL;
//	BYTE* buffer = new BYTE[2];
//	memcpy(buffer,&frame_count,sizeof(USHORT));
//	STAT_STRUCT StatusInfo;
//	status = WriteReadRegister(READ_FROM_CARD,3,0x142,buffer,0,2,&StatusInfo);
//	memcpy(&frame_count,buffer,sizeof(USHORT));
//	if (status != STATUS_SUCCESSFUL)
//	{
//		status = STATUS_READWRITE_REGISTER_ERROR;
//	}
//	SAFE_DELETE_ARRAY(buffer);
//	return status;

template <class T>
string to_string(T t, ios_base & (*f)(ios_base&))
{
  ostringstream oss;
  oss << f << t;
  return oss.str();
}


LONG ThordaqDFLIM::ExportScript(DATA_ACQ_CTRL_STRUCT* gPtrAcqCtrl, SCAN_LUT scanLUT)
{
	ofstream myfile;
	myfile.open ("script.txt");

	myfile << "writemem 3 0x00 1 0x00\n";
	myfile << "writemem 3 0x08 1 0x" << to_string<USHORT>(gPtrAcqCtrl->gblCtrl.img_scan_mode, hex) << endl;
	myfile << "writemem 3 0x10 1 0x" << to_string<USHORT>(gPtrAcqCtrl->gblCtrl.hor_pix_num - 1, hex) << endl;
	myfile << "writemem 3 0x18 1 0x" << to_string<USHORT>(gPtrAcqCtrl->gblCtrl.vrt_pix_num - 1, hex) << endl;
	myfile << "writemem 3 0x20 8 0x" << to_string<ULONG64>(gPtrAcqCtrl->gblCtrl.gpio_cfg, hex) << endl;
	myfile << "writemem 3 0x140 1 0x" << to_string<USHORT>(gPtrAcqCtrl->scan.sync_ctrl, hex) << endl;
	myfile << "writemem 3 0x142 2 0x" << to_string<USHORT>(gPtrAcqCtrl->scan.frm_cnt, hex) << endl;
	myfile << "writemem 3 0x148 1 0x42\n";
	myfile << "writemem 3 0x150 2 0x" << to_string<USHORT>(gPtrAcqCtrl->scan.pll_sync_offset, hex) << endl;
	myfile << "writemem 3 0x160 4 0x" << to_string<ULONG32>(gPtrAcqCtrl->scan.galvo_pixel_dwell, hex) << endl;
	myfile << "writemem 3 0x168 4 0x" << to_string<ULONG32>(gPtrAcqCtrl->scan.galvo_pixel_delay, hex) << endl;
	myfile << "writemem 3 0x170 4 0x" << to_string<ULONG32>(gPtrAcqCtrl->scan.galvo_intra_line_delay, hex) << endl;
	myfile << "writemem 3 0x178 4 0x" << to_string<ULONG32>(gPtrAcqCtrl->scan.galvo_intra_frame_delay, hex) << endl;

	myfile << "writemem 3 0x180 1 0x" << to_string<USHORT>(gPtrAcqCtrl->samplingClock.phase_ctrl, hex) << endl;
	myfile << "writemem 3 0x188 2 0x" << to_string<USHORT>(gPtrAcqCtrl->samplingClock.phase_offset, hex) << endl;
	myfile << "writemem 3 0x190 1 0x" << to_string<USHORT>(gPtrAcqCtrl->samplingClock.phase_step, hex) << endl;
	myfile << "writemem 3 0x198 2 0x" << to_string<USHORT>(gPtrAcqCtrl->samplingClock.phase_limit, hex) << endl;

	myfile << "writemem 3 0x1c0 1 0x" << to_string<USHORT>(gPtrAcqCtrl->streamProcessing.stream_ctrl, hex) << endl;
	myfile << "writemem 3 0x1c2 1 0x" << to_string<USHORT>(gPtrAcqCtrl->streamProcessing.pulse_interleave, hex) << endl;
	myfile << "writemem 3 0x1c8 2 0x" << to_string<USHORT>(gPtrAcqCtrl->streamProcessing.scan_period, hex) << endl;
	myfile << "writemem 3 0x1d0 4 0x" << to_string<ULONG32>(gPtrAcqCtrl->streamProcessing.downsample_rate, hex) << endl;

	myfile << "writemem 3 0x200 2 0x8043\n";
	myfile << "writemem 3 0x200 2 0x8040\n";


	for (int j  = 0; j  < 4; j ++)
	{
		myfile << "\n\\\\LUT\n\n";
		USHORT startAddress = 0x2000 + j * 0x2000;
		for (ULONG32 i = 0; i < gPtrAcqCtrl->gblCtrl.hor_pix_num; i++)
		{
			myfile << "writemem 3 0x" << to_string<USHORT>(startAddress + i * 2, hex) << " 2 0x" << to_string<USHORT>(scanLUT.lut[i], hex) << endl;
		}
	}
	
	myfile.close();
	return TRUE;
}