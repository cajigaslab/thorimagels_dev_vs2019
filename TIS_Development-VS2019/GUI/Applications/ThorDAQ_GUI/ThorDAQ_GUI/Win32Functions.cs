// -------------------------------------------------------------------------
// 
// PRODUCT:			Expresso GUI
// MODULE NAME:		Win32Functions.cs
// 
// MODULE DESCRIPTION: 
// 
// Contains the Windows and DLL  Interface functions.
// 
// $Revision:  $
//
// ------------------------- CONFIDENTIAL ----------------------------------
// 
//              Copyright (c) 2011 by Northwest Logic, Inc.   
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

using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace NWLogic.DeviceLib
{
    #region Win32 Type definitions

    #region old defs
    /// <summary>
    ///  MessageBeep System Constants
    /// </summary>
    public enum BeepType
    {
        /// <summary>Simple Beep type</summary>
        SimpleBeep = -1,
        /// <summary>System Asterisk Beep type</summary>
        SystemAsterisk = 0x00000040,
        /// <summary>System Exclamation Beep type</summary>
        SystemExclamation = 0x00000030,
        /// <summary>System Hand Beep type</summary>
        SystemHand = 0x00000010,
        /// <summary>System Question Beep type</summary>
        SystemQuestion = 0x00000020,
        /// <summary>Default Beep type</summary>
        SystemDefault = 0
    }

    #endregion

    #region Driver Defs
    /// <summary>
    /// The size of the Read/Write Transfer
    /// </summary>
    public enum TransferSizeEnum : uint
    {
        /// <summary>8 Bit transfer</summary>
        transSize8Bit = 0x01,
        /// <summary>16 Bit transfer</summary>
        transSize16Bit = 0x02,
        /// <summary>32 Bit transfer</summary>
        transSize32Bit = 0x04,
        /// <summary>64 Bit transfer (not currently supported)</summary>
        transSize64Bit = 0x08,
        /// <summary>32 Bit DMA transfer</summary>
        transSize32BitDma = 0x10,
        /// <summary>64 Bit DMA transfer</summary>
        transSize64BitDma = 0x20,

        /// <summary>First Transfer Size in list (used for looping)</summary>
        firstTransSize = transSize8Bit,
        /// <summary>Last Transfer Size in list (used for looping)</summary>
        lastTransSize = transSize64BitDma,
        /// <summary>Number of Transfer Sizes</summary>
        numTransSize = 6
    }

    /// <summary>
    /// The direction of the DMA engine
    /// </summary>
    public enum DmaDirectionEnum : uint
    {
        /// <summary>Read from card (C2S)</summary>
        dmaDirRead = 1,
        /// <summary>Write to card (S2C)</summary>
        dmaDirWrite = 0,
    }

    /// <summary>
    /// The type of the DMA engine
    /// </summary>
    public enum DmaTypeEnum : uint
    {
        /// <summary>Packet DMA Engine</summary>
        Packet = 1,
    }

    /// <summary>
    /// The type of the BAR
    /// </summary>
    public enum BarTypeEnum : uint
    {
        /// <summary>Memory Bar with Registers</summary>
        RegisterMemBarType = 0x00,
        /// <summary>Memory Bar with RAM</summary>
        RamMemBarType = 0x01,
        /// <summary>IO Bar with Registers</summary>
        RegisterIoBarType = 0x02,
        /// <summary>Disabled Bar</summary>
        DisabledBarType = 0x03,

        /// <summary>First BAR Type in list (used for looping)</summary>
        firstBarType = RegisterMemBarType,
        /// <summary>Last BAR Type in list (used for looping)</summary>
        lastTransSize = DisabledBarType
    }


    public enum BBoxADchannelEnum : Int32
    {
        // the legacy "Slow" ADC read by DAC mez. card (same for ABBx and 3U IOPanel)
        eAI0 = 0,
        eAI1,
        eAI2,
        eI3,
        eI4,
        eI5,
        // These "slow" AI channels only exist on 3U IO Panel (read by MAX127 on I2C)
        eAI6,
        eAI7,
        eAI8,
        eAI9,
        eAI10,
        eAI11,
        eAI12,
        eAI13,
        // These 1 MSPS DAC set AO channels same on ABBx AND 3U IO Panel
        eAO0 = 14,
        eAO1,
        eAO2,
        eAO3,
        eAO4,
        eAO5,
        eAO6,
        eAO7,
        eAO8,
        eAO9,
        eAO10,
        eAO11,
    };

    
    /// <summary>
    /// Card Configuration Info
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct CARD_CONFIG_INFO
    {
        /// <summary>Vendor ID (for PCI cards)</summary>
        public ushort VendorID;
        /// <summary>Device ID (for PCI cards)</summary>
        public ushort DeviceID;
        /// <summary>Driver version value</summary>
        public uint DriverVersion;
        /// <summary>Driver DLL version value</summary>
        public uint DriverDLLVersion;
        /// <summary>PCI Express Core version value</summary>
        public uint PCIExpressCoreVersion;
        /// <summary>DMA Back-End Core version value</summary>
        public uint DMABackEndCoreVersion;
        /// <summary>User Core version value</summary>
        public uint UserCoreVersion;
        /// <summary>Number of DMA channels</summary>
        public uint NumDmaChannels;
    }

    /// <summary>
    /// Card Configuration DMA Info
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct CARD_CONFIG_DMA_INFO
    {
        /// <summary>DMA engine number</summary>
        public uint EngineNum;
        /// <summary>DMA transfer size</summary>
        public ushort TransferSize;
        /// <summary>DMA engine direction (Read/Write)</summary>
        public ushort Direction;
        /// <summary>Maximum size for DMA</summary>
        public uint MaxDmaSize;
    }
    #endregion

    /// <summary>
    /// PCI Configuration Header
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct BOARD_CONFIG_STRUCT
    {
        /// <summary>Number of S2C DMA Engines</summary>
        public byte NumDmaWriteEngines;
        /// <summary>Number of 1st S2C DMA Engine</summary>
        public byte FirstDmaWriteEngine;
        /// <summary>Number of C2S DMA Engines</summary>
        public byte NumDmaReadEngines;
        /// <summary>Number of 1st C2S DMA Engine</summary>
        public byte FirstDmaReadEngine;
        /// <summary>Number of bidirectional DMA Engines</summary>
        public byte NumDmaRWEngines;
        /// <summary>Number of 1st bidirectional DMA Engine</summary>
        public byte FirstDmaRWEngine;
        /// <summary>Driver Version - Major</summary>
        public byte DriverVersionMajor;
        /// <summary>Driver Version - Minor</summary>
        public byte DriverVersionMinor;
        /// <summary>Driver Version - Sub Minor</summary>
        public byte DriverVersionSubMinor;
        /// <summary>Driver Version - Build Number</summary>
        public byte DriverVersionBuildNumber;
        /// <summary>Card Status Information (cached)</summary>
        public uint CardStatusInfo;
        /// <summary>DMA Back-end Core Version number</summary>
        public uint DMABackEndCoreVersion;
        /// <summary>PCI Express Core Version</summary>
        public uint PCIExpressCoreVersion;
        /// <summary>User Version number (optional)</summary>
        public uint UserVersion;
        /// <summary>PCI VendorID field</summary>
        public ushort VendorID;
        /// <summary>PCI DeviceID field</summary>
        public ushort DeviceID;
        /// <summary>PCI Command field</summary>
        public ushort Command;
        /// <summary>PCI Status field</summary>
        public ushort Status;
        /// <summary>PCI RevisionID field</summary>
        public byte RevisionID;
        /// <summary>PCI ProgIf field</summary>
        public byte ProgIf;
        /// <summary>PCI SubClass field</summary>
        public byte SubClass;
        /// <summary>PCI BaseClass field</summary>
        public byte BaseClass;
        /// <summary>SubsystemVendorId[15:0]</summary>
        public ushort SubsystemVendorId;
        /// <summary>SubsystemId </summary>
        public ushort SubsystemId;
        /// <summary>BAR[0] Configuration </summary>
        public uint BarCfg0;
        /// <summary>BAR[1] Configuration </summary>
        public uint BarCfg1;
        /// <summary>BAR[2] Configuration </summary>
        public uint BarCfg2;
        /// <summary>BAR[3] Configuration </summary>
        public uint BarCfg3;
        /// <summary>BAR[4] Configuration </summary>
        public uint BarCfg4;
        /// <summary>BAR[5] Configuration </summary>
        public uint BarCfg5;
        /// <summary>Expansion ROM Configuration </summary>
        public uint ExpRomCfg;
        /// <summary>PCIe Capability: Device Capabilities</summary>
        public uint PCIeDeviceCap;
        /// <summary>PCIe Capability: Device Control</summary>
        public ushort PCIeDeviceControl;
        /// <summary>PCIe Capability: Device Status</summary>
        public ushort PCIeDeviceStatus;
        /// <summary>PCIe Capability: Link Capabilities</summary>
        public uint PCIeLinkCap;
        /// <summary>PCIe Capability: Link Control</summary>
        public ushort PCIeLinkControl;
        /// <summary>PCIe Capability: Link Status</summary>
        public ushort PCIeLinkStatus;

    }

    /// <summary>
    /// DMA Info structure (Unsafe format)
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct DMA_INFO_STRUCT
    {
        /// <summary>Packet DMA Engine Send Count</summary>
        public byte PacketSendEngineCount;
        /// <summary>Packet DMA Engine Read Count</summary>
        public byte PacketRecvEngineCount;
        /// <summary>Packet DMA Send Engine numbers</summary>
        public fixed byte PacketSendEngine[Win32.MAX_DMA_CHANNELS];
        /// <summary>Packet DMA Read Engine numbers</summary>
        public fixed byte PacketRecvEngine[Win32.MAX_DMA_CHANNELS];
        /// <summary>DLL Major Version number</summary>
        public byte DLLMajorVersion;
        /// <summary>DLL Minor Version number</summary>
        public byte DLLMinorVersion;
        /// <summary>DLL SubMinor Version number</summary>
        public byte DLLSubMinorVersion;
        /// <summary>DLL Build Version number</summary>
        public byte DLLBuildNumberVersion;
        /// <summary>Addressable Packet Mode supported</summary>
        public byte AddressablePacketMode;
    }

    /// <summary>
    /// DMA Info structure (Unsafe format)
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct BOARD_INFO_STRUCT
    {
        /// <summary>Driver Version - Major</summary>
        public byte DriverVersionMajor;
        /// <summary>Driver Version - Minor</summary>
        public byte DriverVersionMinor;
        /// <summary>Driver Version - Sub Minor</summary>
        public byte DriverVersionSubMinor;
        /// <summary>Driver Version - Build Number</summary>
        public byte DriverVersionBuildNumber;
        /// <summary>User Version number (optional)</summary>
        public UInt32 UserVersion;
        public UInt32 PCIVendorDeviceID;  // from BAR, i.e. 0x1DDD (vendor) 0x4001 (device)
    }

    /// <summary>
    /// DMA Info structure (Safe format)
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct DMA_INFO_STRUCT_SAFE
    {
        /// <summary>Packet DMA Engine Send Count</summary>
        public byte PacketSendEngineCount;
        /// <summary>Packet DMA Engine Read Count</summary>
        public byte PacketRecvEngineCount;
        /// <summary>Packet DMA Send Engine numbers</summary>
        public byte[] PacketSendEngine;
        /// <summary>Packet DMA Read Engine numbers</summary>
        public byte[] PacketRecvEngine;
        /// <summary>DLL Major Version number</summary>
        public byte DLLMajorVersion;
        /// <summary>DLL Minor Version number</summary>
        public byte DLLMinorVersion;
        /// <summary>DLL SubMinor Version number</summary>
        public byte DLLSubMinorVersion;
        /// <summary>DLL Build Version number</summary>
        public byte DLLBuildNumberVersion;
        /// <summary>Addressable Packet Mode supported</summary>
        public byte AddressablePacketMode;
    }

    /// <summary>
    /// DMA Capabilities structure
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct DMA_CAP_STRUCT
    {
        /// <summary>DMA Capabilities</summary>
        public UInt32 DmaCapabilities;
    }

    /// <summary>
    /// Status Structure
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct STAT_STRUCT
    {
        /// <summary>Number of bytes transferred</summary>
        ulong CompletedByteCount;
    }

    /// <summary>
    /// DMA Status Structure
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct DMA_STAT_STRUCT
    {
        /// <summary>Number of bytes transferred</summary>
        public ulong CompletedByteCount;
        /// <summary>Number of nanoseconds for driver</summary>
        public ulong DriverTime;
        /// <summary>Number of nanoseconds for hardware</summary>
        public ulong HardwareTime;
        /// <summary>Number of Interrupts per second</summary>
        public ulong IntsPerSecond;
        /// <summary>Number of DPCs/Tasklets per second</summary>
        public ulong DPCsPerSecond;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct GLOBAL_IMAGE_CTRL_STRUCT
    {
        public ushort system_mode;  // e.g. INTERNAL_GALVO_GALVO or INTERNAL_RESONANT_GALVO
        public uint clockRate;
        public uint clock_source;
        public ushort channel;
        public ushort imgHSize;
        public ushort imgVSize;
        public byte scanMode;      // UNIDIRECTION_SCAN or BIDIRECTION_SCAN
        public byte scanDir;       // FORWARD_SCAN or REVERSE_SCAN (if UNIDIRECTION_SCAN)
        public ushort alignmentOffset;
        public uint frameCnt;
        public uint frameNumPerSec;
        public uint frameNumPerTransfer;
        public uint defaultMode;
        public byte triggerMode;    // SW_FREE_RUN_MODE (def.), SOFTWARE_RUN_MODE, HARDWARE_TRIGGER_MODE, MULTI_FRAME_TRIGGER_EACH, etc.
        public uint threePhotonMode;
        public uint threePhotonPhaseAlignment;
        public fixed uint ADCGain[Win32.MAX_CHANNEL_COUNT]; //  dimension Win32.MAX_CHANNEL_COUNT
        public byte numPlanes;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct STREAM_PROCESSING_CTRL_STRUCT
    {
        public double scan_period;
        public uint fir_filter_enabled;
        public fixed double fir_filter[Win32.FIR_FILTER_COUNT * Win32.MAX_CHANNEL_COUNT * Win32.FIR_FILTER_TAP_COUNT] ;  
        public uint channel_multiplexing_enabled;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct COHERENT_SAMPLING_CTRL_STRUCT
    {
        public ushort phaseIncrementMode;
        public ushort phaseOffset;
        public ushort phaseStep;
        public ushort phaseLimit;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct RESONANT_GALVO_CTRL_STRUCT
    {
        public double flybackTime;
        public double vGalvoAmpVal;
        public double vGalvoParkVal;
        public double vGalvoOffset;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct GALVO_GALVO_CTRL_STRUCT
    {
        public double dwellTime; //unit: s
        public double pixelDelayCnt;
        public double turnaroundTime;  //unit: s
        public double flybackTime;
        public double flybackCycle;
        public double lineTime;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct DAC_CRTL_STRUCT
    {
        public ulong output_port;
        public double offset_val;
        public double park_val;
        public double update_rate;
        public double amplitude;
        public ulong waveform_buffer_start_address;
        public ulong waveform_buffer_length;
        public ulong flyback_samples;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct IMAGING_CONFIGURATION_STRUCT  // a.k.a. IMAGING_CONFIGURATION_STRUCT, imaging_config
    {
        public GLOBAL_IMAGE_CTRL_STRUCT imageCtrl;
        public STREAM_PROCESSING_CTRL_STRUCT streamingCtrl;
        public COHERENT_SAMPLING_CTRL_STRUCT coherentSamplingCtrl;
        public GALVO_GALVO_CTRL_STRUCT galvoGalvoCtrl;
        public RESONANT_GALVO_CTRL_STRUCT resonantGalvoCtrl;
        //  public DAC_CRTL_STRUCT[] dacCtrl; // array causes .NET ARG exception
        // packing (Pack=8) separate structures makes DLL
        // see the struct array correctly
        public DAC_CRTL_STRUCT dacCtrl0; // array causes .NET ARG exception
        public DAC_CRTL_STRUCT dacCtrl1; // array causes .NET ARG exception
        public DAC_CRTL_STRUCT dacCtrl2; // array causes .NET ARG exception
        public DAC_CRTL_STRUCT dacCtrl3; // array causes .NET ARG exception
        public DAC_CRTL_STRUCT dacCtrl4; // array causes .NET ARG exception
        public DAC_CRTL_STRUCT dacCtrl5; // array causes .NET ARG exception
        public DAC_CRTL_STRUCT dacCtrl6; // array causes .NET ARG exception
        public DAC_CRTL_STRUCT dacCtrl7; // array causes .NET ARG exception
        public DAC_CRTL_STRUCT dacCtrl8; // array causes .NET ARG exception
        public DAC_CRTL_STRUCT dacCtrl9; // array causes .NET ARG exception
        public DAC_CRTL_STRUCT dacCtrl10; // array causes .NET ARG exception
        public DAC_CRTL_STRUCT dacCtrl11; // array causes .NET ARG exception
        public DAC_CRTL_STRUCT dacCtrl12; // array causes .NET ARG exception


    }


    #endregion

    /// <summary>
    /// Win32 API functions
    /// </summary>
    public sealed class Win32
    {
        #region Win32 Defines
        // (from winioctl.h)
        // Define the various device type values.  Note that values used by Microsoft
        // Corporation are in the range 0-32767, and 32768-65535 are reserved for use
        // by customers.
        //
        // THORDAQ hardware...
        public const int MAX_CHANNEL_COUNT = 4;  // i.e. Thordaq PCI backplane ADC channels
        public const int DAC_CHANNEL_COUNT = 13; // DACs including the Breakout Boxes
        public const int FIR_FILTER_COUNT = 2;
        public const int FIR_FILTER_TAP_COUNT = 16;
        

        // The MEM constants are passed to the Win32
        // VirtualAlloc and VirtualFree functions.  They
        // define the type of memory operation to perform (e.g.
        // release memory).

        private const int MEM_COMMIT = 0x1000;
        private const int MEM_RESERVE = 0x2000;
        private const int MEM_DECOMMIT = 0x4000;
        private const int MEM_RELEASE = 0x8000;

        // The PAGE constants are used with the VirtualProtect
        // function.  They define the access rights to a chunk
        // of virtual memory.

        private const int PAGE_NOACCESS = 0x001;
        private const int PAGE_READONLY = 0x002;
        private const int PAGE_READWRITE = 0x004;
        private const int PAGE_WRITECOPY = 0x008;
        private const int PAGE_EXECUTE = 0x010;
        private const int PAGE_EXECUTE_READ = 0x020;
        private const int PAGE_EXECUTE_READWRITE = 0x040;
        private const int PAGE_EXECUTE_WRITECOPY = 0x080;
        private const int PAGE_GUARD = 0x100;
        private const int PAGE_NOCACHE = 0x200;

        /// <summary>Maximum number of DMA channels expected</summary>
        public const int MAX_DMA_CHANNELS = 64;
        /// <summary>Maximum size of DMA</summary>
        public const uint MAX_DMA_SIZE = 0x100000;

        // CardConfigId values
        //        /// <summary>Standard PCI card functionality</summary>
        //        public const ushort CardConfigId_StandardPci = 1;
        /// <summary>PCI Express card functionality</summary>
        public const ushort CardConfigId_PciExpress = 2;

        /// <summary>BAR Size offset</summary>
        public const int BAR_CFG_BAR_SIZE_OFFSET = 24;
        /// <summary>BAR is present</summary>
        public const uint BAR_CFG_BAR_PRESENT = 0x0001;
        /// <summary>BAR is a Memory type</summary>
        public const uint BAR_CFG_BAR_TYPE_MEMORY = 0x0000;
        /// <summary>BAR is an IO type</summary>
        public const uint BAR_CFG_BAR_TYPE_IO = 0x0002;
        /// <summary>BAR Memory is prefetchable</summary>
        public const uint BAR_CFG_MEMORY_PREFETCHABLE = 0x0004;
        /// <summary>BAR is 64 bit capable</summary>
        public const uint BAR_CFG_MEMORY_64BIT_CAPABLE = 0x0008;

        /// <summary>Status - Success</summary>
        public const ushort STATUS_SUCCESSFUL = 0x00;
        /// <summary>Status - Incomplete</summary>
        public const ushort STATUS_INCOMPLETE = 0x01;
        /// <summary>Status - Invalid Bar Num</summary>
        public const ushort STATUS_INVALID_BARNUM = 0x02;
        /// <summary>Status - Invalid Card Offset</summary>
        public const ushort STATUS_INVALID_CARDOFFSET = 0x04;
        /// <summary>Status - Overflow</summary>
        public const ushort STATUS_OVERFLOW = 0x08;
        /// <summary>Status - Invalid Board Number</summary>
        public const ushort STATUS_INVALID_BOARDNUM = 0x10;
        /// <summary>Status - Invalid Board Number</summary>
        public const ushort STATUS_MEMALLOC_FAILED = 0x20;
        public const ushort STATUS_I2C_INVALIDMUX = 15;
        public const ushort STATUS_I2C_INVALIDDEV = 16;
        public const ushort STATUS_I2C_TIMEOUT_ERR = 17;


        /// <summary>Memory Flag - System to Card</summary>
        public const UInt32 MEM_FLAG_SYSTEM_TO_CARD = 0x00000000;
        /// <summary>Memory Flag - Card to System</summary>
        public const UInt32 MEM_FLAG_CARD_TO_SYSTEM = 0x00000001;

        /// <summary>DMA Flag - System to Card</summary>
        public const UInt16 DMA_FLAG_SYSTEM_TO_CARD = 0x0000;
        /// <summary>DMA Flag - Card to System</summary>
        public const UInt16 DMA_FLAG_CARD_TO_SYSTEM = 0x0002;

        /// <summary>DMA Capabilities - Card Address Size Shift </summary>
        public const int DMA_CAP_CARD_ADDR_SIZE_SHIFT = 16;
        /// <summary>DMA Capabilities - Card Address Size Mask </summary>
        public const int DMA_CAP_CARD_ADDR_SIZE_MASK = 0xff;

        /// <summary>DMA Capabilities - Engine Type Mask </summary>
        public const UInt32 DMA_CAP_ENGINE_TYPE_MASK = 0x00000030;
        /// <summary>DMA Capabilities - Engine Type Mask </summary>
        public const UInt32 DMA_CAP_BLOCK_DMA = 0x00000000;
        /// <summary>DMA Capabilities - Engine Type Mask </summary>
        public const UInt32 DMA_CAP_PACKET_DMA = 0x00000010;
        /// <summary>DMA Capabilities - Statistics Scaling Mask </summary>
        public const UInt32 DMA_CAP_STATS_SCALING_FACTOR_MASK = 0xC0000000;
        /// <summary>DMA Capabilities - Statistics Scaling Factor 0 </summary>
        public const UInt32 DMA_CAP_STATS_SCALING_FACTOR_0 = 0x00000000;
        /// <summary>DMA Capabilities - Statistics Scaling Factor 2x </summary>
        public const UInt32 DMA_CAP_STATS_SCALING_FACTOR_2 = 0x40000000;
        /// <summary>DMA Capabilities - Statistics Scaling Factor 4x </summary>
        public const UInt32 DMA_CAP_STATS_SCALING_FACTOR_4 = 0x80000000;
        /// <summary>DMA Capabilities - Statistics Scaling Factor 8x </summary>
        public const UInt32 DMA_CAP_STATS_SCALING_FACTOR_8 = 0xC0000000;

        // Packet Generator Control DWORD Defines
        /// <summary>Packet Generator Control - Enable </summary>
        public const byte PACKET_GEN_ENABLE = 0x01;
        /// <summary>Packet Generator Control - Loopback Enable </summary>
        public const UInt32 PACKET_GEN_LOOPBACK_ENABLE = 0x00000002;
        /// <summary>Packet Generator Control - Entries Mask </summary>
        public const UInt32 PACKET_GEN_TABLE_ENTRIES_MASK = 0x00000030;
        /// <summary>Packet Generator Control - Pattern Mask </summary>
        public const UInt32 PACKET_GEN_DATA_PATTERN_MASK = 0x00000700;
        /// <summary>Packet Generator Control - Pattern Constant </summary>
        public const UInt32 PACKET_GEN_DATA_PATTERN_CONSTANT = 0x00000000;
        /// <summary>Packet Generator Control - Pattern Increment Byte </summary>
        public const UInt32 PACKET_GEN_DATA_PATTERN_INC_BYTE = 0x00000100;
        /// <summary>Packet Generator Control - Pattern LFSR </summary>
        public const UInt32 PACKET_GEN_DATA_PATTERN_LFSR = 0x00000200;
        /// <summary>Packet Generator Control - Pattern Increment DWORD </summary>
        public const UInt32 PACKET_GEN_DATA_PATTERN_INC_DWORD = 0x00000300;
        /// <summary>Packet Generator Control - Continuous Data Pattern </summary>
        public const UInt32 PACKET_GEN_CONT_DATA_PATTERN = 0x00000800;
        /// <summary>Packet Generator Control - User Pattern Mask </summary>
        public const UInt32 PACKET_GEN_USER_PATTERN_MASK = 0x00007000;
        /// <summary>Packet Generator Control - User Pattern Constant </summary>
        public const UInt32 PACKET_GEN_USER_PATTERN_CONSTANT = 0x00000000;
        /// <summary>Packet Generator Control - User Pattern Increment Byte </summary>
        public const UInt32 PACKET_GEN_USER_PATTERN_INC_BYTE = 0x00001000;
        /// <summary>Packet Generator Control - User Pattern LFSR </summary>
        public const UInt32 PACKET_GEN_USER_PATTERN_LFSR = 0x00002000;
        /// <summary>Packet Generator Control - User Pattern Increment Word </summary>
        public const UInt32 PACKET_GEN_USER_PATTERN_INC_DWORD = 0x00003000;
        /// <summary>Packet Generator Control - Continuous User Pattern </summary>
        public const UInt32 PACKET_GEN_CONT_USER_PATTERN = 0x00008000;
        /// <summary>Packet Generator Control - Active Clock Mask </summary>
        public const UInt32 PACKET_GEN_ACTIVE_CLOCK_MASK = 0x00FF0000;
        /// <summary>Packet Generator Control - Inactive Clock Mask </summary>
        public const UInt32 PACKET_GEN_INACTIVE_CLOCK_MASK = 0xFF000000;

        /// <summary>Bytes Transferred Scaling Factor</summary>
        public static uint StatsMultiplier = 1;

        #endregion

        #region Wrapper Methods

        /// <summary>
        /// Returns true if all threads stopped.  
        /// </summary>
        /// <returns></returns>
        public static uint ScalingFactorGet()
        {
            return (StatsMultiplier);
        }

        /// <summary>
        /// AllocateBuffer - Allocates a protected buffer.
        /// </summary>
        /// <param name="Buffer">Address of returned allocated buffer</param>
        /// <param name="length">Length of buffer</param>
        /// <returns>Status of the operation</returns>
        public unsafe static void AllocateBuffer(ref IntPtr Buffer, uint length)
        {
            // Allocate the desired memory.  The first parameter
            // specifies the address; if null/zero, the system
            // decides where to locate the memory.

            Buffer = VirtualAlloc(
                        IntPtr.Zero,
                        length,
                        MEM_COMMIT | MEM_RESERVE,
                        PAGE_READWRITE);
        }

        /// <summary>
        /// FreeBuffer - Frees a previously Allocated buffer.
        /// </summary>
        /// <param name="Buffer">Address of returned allocated buffer</param>
        /// <returns>Status of the operation</returns>
        public unsafe static void FreeBuffer(IntPtr Buffer)
        {
            VirtualFree(Buffer, 0, MEM_DECOMMIT | MEM_RELEASE);
        }


        /// <summary>
        /// ConnectToBoard.  Opens the interface to the device.
        /// </summary>
        /// <param name="board">Instance number (0 based)</param>
        /// <param name="DmaInfo">Returned DMA Info from Board</param>
        /// <returns>Handle to open device or NULL</returns>
        public unsafe static uint DsConnectToBoard(UInt32 board, ref BOARD_INFO_STRUCT BoardInfo)
        {
            // get the data in unsafe format
            uint status = ConnectToBoard(board);

            if (status == STATUS_SUCCESSFUL)
            {
                status = GetBoardCfg(board, ref BoardInfo);
            }
            return status;
        }

        /// <summary>
        /// MemoryRead.  Start a Memory Read transfer on the device.
        /// </summary>
        /// <param name="board">board number</param>
        /// <param name="buf">Address of buffer</param>
        /// <param name="bar">Card BAR</param>
        /// <param name="cardAddress">Card Address</param>
        /// <param name="bufferOffset">Offset into Buffer</param>
        /// <param name="bufferLength">Buffer Length</param>
        /// <returns>Status = true if successful, false if failed</returns>
        public unsafe static bool MemoryRead
            (
            UInt32 board,
            IntPtr buf,
            UInt32 bar,
            UInt64 cardAddress,
            UInt64 bufferOffset,
            UInt64 bufferLength
            )
        {
            uint status = 0;
            STAT_STRUCT Status = new STAT_STRUCT();
            status = DoMem(board,
                        MEM_FLAG_CARD_TO_SYSTEM,
                        bar,
                        buf,
                        bufferOffset,
                        cardAddress,
                        bufferLength,
                        ref Status);
            if (status == Win32.STATUS_SUCCESSFUL)
            {
                return true;
            }
            else
                return false;
        }

        /// <summary>
        /// MemoryWrite.  Start a Memory Write transfer on the device.
        /// </summary>
        /// <param name="board">Board number</param>
        /// <param name="buf">Address of buffer</param>
        /// <param name="bar">Card BAR</param>
        /// <param name="cardAddress">Card Address</param>
        /// <param name="bufferOffset">Offset into Buffer</param>
        /// <param name="bufferLength">Buffer Length</param>
        /// <returns>Status = true if successful, false if failed</returns>
        public unsafe static bool MemoryWrite
            (
            UInt32 board,
            IntPtr buf,
            UInt32 bar,
            UInt64 cardAddress,
            UInt64 bufferOffset,
            UInt64 bufferLength
            )
        {
            uint status = 0;
            STAT_STRUCT Status = new STAT_STRUCT();
            status = DoMem(board,
                        MEM_FLAG_SYSTEM_TO_CARD,
                        bar,
                        buf,
                        bufferOffset,
                        cardAddress,
                        bufferLength,
                        ref Status);
            if (status == Win32.STATUS_SUCCESSFUL)
            {
                return true;
            }
            else
                return false;
        }
        /// <summary>
        /// MemoryWrite.  Start a Memory Write transfer on the device.
        /// </summary>
        /// <param name="board">Board number</param>
        /// <param name="buf">Address of buffer</param>
        /// <param name="bar">Card BAR</param>
        /// <param name="cardAddress">Card Address</param>
        /// <param name="bufferOffset">Offset into Buffer</param>
        /// <param name="bufferLength">Buffer Length</param>
        /// <returns>Status = true if successful, false if failed</returns>
        public unsafe static uint MemoryWriteRetStatus
            (
            UInt32 board,
            IntPtr buf,
            UInt32 bar,
            UInt64 cardAddress,
            UInt64 bufferOffset,
            UInt64 bufferLength
            )
        {
            uint status = 0;
            STAT_STRUCT Status = new STAT_STRUCT();
            status = DoMem(board,
                        MEM_FLAG_SYSTEM_TO_CARD,
                        bar,
                        buf,
                        bufferOffset,
                        cardAddress,
                        bufferLength,
                        ref Status);
            return status;
        }


        /// <summary>
        /// memRead.  Start a Memory Read transfer on the device.
        /// </summary>
        /// <param name="board">board number</param>
        /// <param name="buf">Address of buffer</param>
        /// <param name="bar">Card BAR</param>
        /// <param name="cardAddress">Card Address</param>
        /// <param name="bufferOffset">Offset into Buffer</param>
        /// <param name="bufferLength">Buffer Length</param>
        /// <returns>Status = true if successful, false if failed</returns>
        public unsafe static bool memRead
            (
            UInt32 board,
            ref byte[] buf,
            UInt32 bar,
            UInt64 cardAddress,
            UInt64 bufferOffset,
            UInt64 bufferLength
            )
        {
            uint status = 0;
            STAT_STRUCT Status = new STAT_STRUCT();
            fixed (byte* pOut = buf)
            {
                status = DoMem(board,
                    MEM_FLAG_CARD_TO_SYSTEM,
                    bar,
                    (IntPtr)pOut,
                    bufferOffset,
                    cardAddress,
                    bufferLength,
                    ref Status);
            }
            if (status == Win32.STATUS_SUCCESSFUL)
            {
                return true;
            }
            else
                return false;
        }


        /// <summary>
        /// memWrite.  Start a Memory Write transfer on the device.
        /// </summary>
        /// <param name="board">Board number</param>
        /// <param name="buf">Address of buffer</param>
        /// <param name="bar">Card BAR</param>
        /// <param name="cardAddress">Card Address</param>
        /// <param name="bufferOffset">Offset into Buffer</param>
        /// <param name="bufferLength">Buffer Length</param>
        /// <returns>Status = true if successful, false if failed</returns>
        public unsafe static bool memWrite
            (
            UInt32 board,
            ref byte[] buf,
            UInt32 bar,
            UInt64 cardAddress,
            UInt64 bufferOffset,
            UInt64 bufferLength
            )
        {
            uint status = 0;
            STAT_STRUCT Status = new STAT_STRUCT();
            fixed (byte* pOut = buf)
            {
                status = DoMem(board,
                    MEM_FLAG_SYSTEM_TO_CARD,
                    bar,
                    (IntPtr)pOut,
                    bufferOffset,
                    cardAddress,
                    bufferLength,
                    ref Status);
            }
            if (status == Win32.STATUS_SUCCESSFUL)
            {
                return true;
            }
            else
                return false;
        }


        public unsafe static uint PacketRead
            (
                UInt32 board,
                ref UInt64 UserStatus,
                Byte ChannelIndex,
                IntPtr Buffer,
                ref UInt32 BufferSize
            )
        {
            uint status = 0;
            fixed (uint* pBufferSize = &BufferSize)
            {
                fixed (UInt64* pUserStatus = &UserStatus)
                {
                    status = PacketReadex(board,
                            (int)ChannelIndex,
                            (IntPtr)pBufferSize,
                            Buffer,
                            0xffffff
                            );
                }
            }
            return status;
        }

        public unsafe static uint ReadDRamex
            (
                UInt32 board,
                UInt64 CardOffset,
                IntPtr Buffer,
                ref UInt32 BufferSize
            )
        {
            uint status = 0;
            fixed (uint* pBufferSize = &BufferSize)
            {
                status = PacketReadDRam(board,
                            CardOffset,
                            (IntPtr)pBufferSize,
                            Buffer,
                            0xffffff
                            );
            }
            return status;
        }

        public unsafe static uint WriteDRamex
           (
               UInt32 board,
               UInt64 CardOffset,
               IntPtr Buffer,
               UInt32 BufferSize
           )
        {
            uint status = 0;

            status = PacketWriteDRam(board,
                        CardOffset,
                        BufferSize,
                        Buffer,
                        0xffffff
                        );

            return status;
        }

        #endregion

        #region Win32 Methods
        /// <summary>
        /// QueryPerformanceFrequency - Get the frequency of the performance timer
        /// </summary>
        /// <param name="lpFrequency"></param>
        /// <returns>performance counter frequency</returns>
        [DllImport("Kernel32.dll")]
        public static extern int QueryPerformanceFrequency(ref Int64 lpFrequency);

        /// <summary>
        /// Get Performance Counter
        /// </summary>
        /// <param name="lpPerformanceCount"></param>
        /// <returns>Timer count</returns>
        [DllImport("Kernel32.dll")]
        public static extern int QueryPerformanceCounter(ref Int64 lpPerformanceCount);

        /// <summary>
        /// MessageBeep method.
        /// </summary>
        /// <param name="Type">Beep Type</param>
        /// <returns>Boolean</returns>
        //[DllImport("User32.dll", SetLastError=true)]
        //public static extern bool MessageBeep(BeepType Type);


        // The VirtualAlloc function reserves a region of 
        // memory pages in the calling process.  If the starting
        // address is zero/null (first parameter), the OS
        // decides where to allocate the region. Note: memory
        // is allocated in pages; refer to the Platform SDK for
        // details.  A zero/null return value indicates failure.

        /// <summary>
        /// VirtualAlloc - Reserves and Commits a region of pages in the virtual address space.
        /// </summary>
        /// <param name="lpStartAddr">address of the region to allocate</param>
        /// <param name="size">The size of the region, in bytes</param>
        /// <param name="flAllocationType">The type of memory allocation</param>
        /// <param name="flProtect">The memory protection for the region of pages to be allocated</param>
        /// <returns></returns>
        [DllImport("kernel32", SetLastError = true)]
        private static extern IntPtr VirtualAlloc(
            IntPtr lpStartAddr,
            UInt32 size,
            UInt32 flAllocationType,
            UInt32 flProtect);


        // VirtualFree releases/decommits the memory pages allocated by 
        // a call to VirtualAlloc.  When releasing, the dwSize
        // value must be zero.  A return value of zero indicates
        // failure.

        /// <summary>
        /// VirtualFree - Frees a region of pages in the virtual address space.
        /// </summary>
        /// <param name="lpAddress">address of the region to allocate</param>
        /// <param name="dwSize">The size of the region, in bytes</param>
        /// <param name="dwFreeType">The type of memory free</param>
        /// <returns></returns>
        [DllImport("kernel32", SetLastError = true)]
        private static extern int VirtualFree(
            IntPtr lpAddress,
            UInt32 dwSize,
            UInt32 dwFreeType);

        /// <summary>
        /// ConnectToBoard.  Opens the interface to the device.
        /// </summary>
        /// <param name="board">Instance number (0 based)</param>
        /// <param name="DmaInfo">Returned DMA Info from Board</param>
        /// <returns>Handle to open device or NULL</returns>
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIBindBoard")]
        public extern static uint ConnectToBoard(UInt32 board);

        /// <summary>
        /// DisconnectFromBoard.  Closes the interface to the device.
        /// </summary>
        /// <param name="board">Instance number (0 based)</param>
        /// <returns>Handle to open device or NULL</returns>
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIDisconnectFromBoard")]
        public extern static uint DisconnectFromBoard(UInt32 board);

        /// <summary>
        /// GetBoardCfg - Calls the driver to get the board config (including PCI header).
        /// </summary>
        /// <param name="board">Instance number (0 based)</param>
        /// <param name="Board">Board Config structure to return data</param>
        /// <returns>Status of the operation</returns>
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIGetBoardCfg")]
        public extern static uint GetBoardCfg(UInt32 board,
                          ref BOARD_INFO_STRUCT Board);

        /// <summary>
        /// DoMem - Calls the driver to perform a Memory transfer.
        /// </summary>
        /// <param name="board">board number</param>
        /// <param name="Rd_Wr_n">Read/Write flag</param>
        /// <param name="BarNum">Bar number</param>
        /// <param name="Buffer">data buffer</param>
        /// <param name="Offset">Offset into the data buffer</param>
        /// <param name="CardOffset">Card offset for beginning of transfer</param>
        /// <param name="Length">Length of transfer</param>
        /// <param name="Status">Status structure</param>
        /// <returns>Status of the operation</returns>
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIDoMem")]
        public extern static uint DoMem(UInt32 board,
                            UInt32 Rd_Wr_n,
                            UInt32 BarNum,
                            IntPtr Buffer,
                            UInt64 Offset,
                            UInt64 CardOffset,
                            UInt64 Length,
                            ref STAT_STRUCT Status);


        /// <summary>
        /// ThordaqStartAcquisition - Calls the driver to setup a buffer for DMA transfer.
        /// </summary>
        /// <param name="board">Board number</param>
        /// <returns>Status of the operation</returns>
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThordaqStartAcquisition")]
        public extern static uint StartAcquisition(
                            UInt32 board);

        ///// <summary>
        ///// ThordaqStopAcquisition - Calls the driver to shutdown packet mode.
        ///// </summary>
        ///// <param name="board">Board number</param>
        ///// <returns>Status of the operation</returns>
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThordaqStopAcquisition")]
        public extern static uint StopAcquisition(UInt32 board);

        /// <summary>
        /// SetupPacketGenerator - Calls the driver to setup the packet generator.
        /// </summary>
        /// <param name="board">Board number</param>
        /// <param name="EngineOffset">Instance offset (0 based)</param>
        /// <param name="SendDirection">Direction of transfer</param>
        /// <param name="PacketGenChk">Packet Generator/Checker control structure</param>
        /// <returns>Status of the operation</returns>
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThordaqSetImagingConfiguration")]
        public extern static uint SetupPacketGenerator(UInt32 board,
                            ref IMAGING_CONFIGURATION_STRUCT PacketGenChk);

        /// <summary>
        /// PacketRead - Calls the driver to Read DMA Data.
        /// </summary>
        /// <param name="board">Board number</param>
        /// <param name="ChannelIndex">Channel offset (0 based)</param>
        /// <param name="BufferSize">size of Data Buffer</param>
        /// <param name="Buffer">data buffer</param>
        /// <param name="Timeout">read timeout</param>
        /// <returns>Status of the operation</returns>
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThordaqReadChannel")]
        public extern static uint PacketReadex(UInt32 board,
                            int ChannelIndex,
                            IntPtr BufferSize,
                            IntPtr Buffer,
                            double Timeout_ms);

        /// <summary>
        /// ThordaqPacketReadex - Calls the driver to Read DMA Data.
        /// </summary>
        /// <param name="board">Board number</param>
        /// <param name="ChannelIndex">Channel offset (0 based)</param>
        /// <param name="BufferSize">size of Data Buffer</param>
        /// <param name="Buffer">data buffer</param>
        /// <param name="Timeout">read timeout</param>
        /// <returns>Status of the operation</returns>
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThordaqPacketReadBuffer")]
        public extern static uint PacketReadDRam(UInt32 board,
                            ulong CardOffset,
                            IntPtr BufferSize,
                            IntPtr Buffer,
                            int Timeout);


        /// <summary>
        /// ThordaqPacketReadex - Calls the driver to Write DMA Data.
        /// </summary>
        /// <param name="board">Board number</param>
        /// <param name="ChannelIndex">Channel offset (0 based)</param>
        /// <param name="BufferSize">size of Data Buffer</param>
        /// <param name="Buffer">data buffer</param>
        /// <param name="Timeout">read timeout</param>
        /// <returns>Status of the operation</returns>
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThordaqPacketWriteBuffer")]
        public extern static uint PacketWriteDRam(UInt32 board,
                            ulong CardOffset,
                            uint BufferSize,
                            IntPtr Buffer,
                            int Timeout);
        
        //////////////////////////////////////////////////////////////////////////////////////
        // Packet Mode defines and Structures

        public const Int32 PACKET_MODE_FIFO = 0x00;
        public const Int32 PACKET_MODE_ADDRESSABLE = 0x02;
        public const Int32 PACKET_MODE_STREAMING = 0x40;

        // Restore the NWL SDK functions
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThordaqSetupPacketMode")]
        public extern static uint ThordaqSetupPacketMode(
                    UInt32 board,                  // Board number to target
                    Int32 EngineOffset,            // DMA Engine number offset to use
                    IntPtr Buffer,                 // Data buffer
                    ref UInt32 BufferSize,         // Total size of Recieve Buffer
                    ref UInt32 MaxPacketSize,      // Largest single packet size
                    Int32 PacketMode,              // Sets mode, FIFO or Addressable Packet mode
                    Int32 NumberDescriptors);      // Number of DMA Descriptors to allocate (Addressable mode)

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThordaqPacketReadEx")]
        public extern static uint ThordaqPacketReadEx(
                    UInt32 boardNum,               // Board to target
                    Int32 EngineOffset,           // DMA Engine number offset to use
                    ref UInt64 UserStatus,          // User Status returned from the EOP DMA Descriptor
                    UInt64 CardOffset,             // (ThorDAQ DRAM) Card Address to start read from
                    UInt32 Mode,                   // Control Mode Flags
                    IntPtr Buffer,                 // Address of (host) unmanaged data buffer
                    ref UInt32 Length);            // Length to Read

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThordaqPacketWriteEx")]
        public extern static uint ThordaqPacketWriteEx(
                    UInt32 boardNum,               // Board to target
                    Int32 EngineOffset,            // DMA Engine number offset to use
                    ref UInt64 UserStatus,         // User Status returned from the EOP DMA Descriptor
                    UInt64 CardOffset,             // (ThorDAQ DRAM) Card Address to start read from
                    UInt32 Mode,                   // Control Mode Flags
                    IntPtr Buffer,                 // Address of (host) unmanaged data buffer
                    UInt32 Length);                // Length to Read

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThordaqShutdownPacketMode")]
        public extern static uint ThordaqShutdownPacketMode(
                    UInt32 board,     // Board number to target
                    UInt32 EngineOffset);            // DMA Engine number offset to use
        

        //////////////////////////////////
        #endregion
    }
}
