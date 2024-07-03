using System;
using System.IO; // file operations
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Globalization;
using System.Runtime.InteropServices; // import DLL, etc.


// Thordaq board object organization
// TDcomponent -- main board, DAC or ADC or 3P mezannine card, BreakoutBox which has EEPROM for S/N or Version Number
// TDassembly  -- all possible TDcomponents combined on single PCIe board
// TDcontroller   -- a TDassembly with all control methods and state info (for read-only config registers)
//

namespace ThorDAQConfigControl.Model
{
    public enum THORDAQ_STATUS  // should match thordaqcmd.h _THORDAQ_STATUS
    {
        // Return status defines
        // The operation completed successfully.
        STATUS_SUCCESSFUL = 0,
        // Borad Software in initiated unsuccessfully.
        STATUS_INITIATION_INCOMPLETE = 1,
        // No Hardware is installed.
        STATUS_DEVICE_NOT_EXISTS = 2,
        // Board number does not exist
        STATUS_INVALID_BOARDNUM = 3,
        // Retrieve Board device information error 
        STATUS_GET_BOARD_CONFIG_ERROR = 4,
        // ReadWrite registers unsuccessfully
        STATUS_READWRITE_REGISTER_ERROR = 5,
        // Set up DMA Packet Mode unsuccessfully
        STATUS_SETUP_PACKET_MODE_INCOMPLETE = 6,
        // Release DMA Packet Mode unsuccessfully
        STATUS_RELEASE_PACKET_MODE_INCOMPLETE = 7,
        // Write buffer data to on-board DRAM unsuccessfully
        STATUS_WRITE_BUFFER_ERROR = 8,
        // Read buffer data from on-board DRAM unsuccessfully
        STATUS_READ_BUFFER_ERROR = 9,
        // Read buffer data timeout error
        STATUS_READ_BUFFER_TIMEOUT_ERROR = 10,
        // Set up Acquisition settings unsuccessfully
        STATUS_PARAMETER_SETTINGS_ERROR = 11,
        // Overflow 
        STATUS_OVERFLOW = 12,
        // NWL code
        STATUS_INVALID_MODE = 13,
        // NWL code
        STATUS_INCOMPLETE = 14,

        STATUS_I2C_INVALIDDEV = 16,
        STATUS_I2C_TIMEOUT_ERR = 17
    };

    public class TDcontroller : TDassembly
    {
        public enum ImageSyncMasterTimingMode
        {
            Resonant_Galvo = 0,  // defined in hardware registers
            Galvo_Galvo = 1,
            External_Inputs = 2
        };
        public BARs BAR;
        public FPGAregister FPGAregs;
        public S2MM_adcDMA S2MMadcDMA;
        public DAC_control DAC_Control;
        
        public TDcontroller(uint boardnum, string boardModel)
            : base(boardnum, boardModel)
        {
            BAR = new BARs(boardnum);
            FPGAregs = new FPGAregister(boardnum);
            S2MMadcDMA = new S2MM_adcDMA(boardnum, FPGAregs);
            DAC_Control = new DAC_control(boardnum, FPGAregs);
        }

    }

    // must match "thordaqcmd.h" _S2MM_CONFIG
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct S2MM_CONFIG
    {
        public Int32 ChannelMask;  // e.g. 0x6 for channels 1&2, 0xF for chan 0-3
        public Int32 HSize;
        public Int32 VSize;
        public Int32 NumberOfDescriptorsPerBank; // a.k.a. # of frames (per bank)
        public Int32 DRAMstartingAddress; // physical address in DDR3 for separate DMA to host
        public Int32 DRAMendingAddress;   // returned by S2MM DMA setup
    };

    // must match "thordaqcmd.h" _S2MM_ADCSAMPLE_LUT
    // NOTE!!  .NET fails with "Cannot marshal:  Internal limitation: structure is too complex or too large"
    // when total size hits 64k
//    [StructLayout(LayoutKind.Sequential, Pack = 1)]
//    public unsafe struct S2MM_ADCSAMPLE_LUT
  //  {
    //    public fixed UInt16 ADCsampleLUTchan0[32768];
    //}

    public unsafe struct DERIVED_TIMING // i.e. computed or measured output based on config
    {
        public UInt64 Pixel_to_Pixel_timeNS; // Galvo-Galvo calculated micro-secs between pixels
        public UInt64 T_frame;
        public UInt64 T_line;
        public UInt64 ADCStreamDownsamplingRate;  // reduce rate when needed for keep line-sample buffer below 65532 samples
        public UInt64 ADCStreamScanningPeriodReg; // only valid for Galvo-Galvo

    };

    public class DAC_control // ThorDAQ higher functions manipulating DAC mezannine board registers/hardware
    {
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIsetDACvoltage")]
        public extern static uint ThorDAQAPIsetDACvoltage(
                    UInt32 board,                  // Board number to target
                    UInt32 channel,                // DAC chan 0-11
                    double Volts);                 // -10.0 to +10.0 
        
        // our variables
        FPGAregister _FPGAregs; // local copies of Board object
        UInt32 _PCIboardIndex;
        public string sErrMsg = "OK"; // public diagnostic message for last error

        // constructor - get pointers for all board hardware control
        public DAC_control(UInt32 boardNum, FPGAregister FPGAregisters)
        {
            _PCIboardIndex = boardNum;
            _FPGAregs = FPGAregisters;
        }

        // set voltages for DAC channels
        public bool SetStaticDACvoltage(UInt32 channel, double VoltsDC)
        {
            bool bStatus = true;

            uint uiStatus = ThorDAQAPIsetDACvoltage(_PCIboardIndex, channel, VoltsDC);
            if( uiStatus != (uint)THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                bStatus = false;
                sErrMsg = "ThorDAQAPIsetDACvoltage(): failed";
            }
            return bStatus;
        }

    }
    public class S2MM_adcDMA // ThorDAQ's Stream to Mapped Memory Setup and control
    {
        const UInt64 FPGA_SYSCLOCKFREQ = 200000000;
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIS2MMconfig")]
        public extern static uint ThorDAQAPIS2MMconfig(
                    UInt32 board,                  // Board number to target
                    ref S2MM_CONFIG s2mmConfig);       // size of waveform (must be same for all channels)  
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIADCsampleImagizerLUT")]
        public extern static uint ThorDAQAPIADCsampleImagizerLUT(
                    UInt32 board,                  // Board number to target
                    IntPtr ADCsampleLUT);  // one channel of 64k samples (new Reverb/MP format)
        
        public string sErrMsg; // public diagnostic message for last error
        public UInt64 T_frame = 0;
        public UInt64 T_line = 0;

        UInt32 _PCIboardIndex;
        // local copies of variables sent to API DLL
        Int32 _HSize;
        Int32 _VSize;
        Int32 _DDR3tartingAddr;
        FPGAregister _FPGAregs;
        DERIVED_TIMING DerivedTiming;
        
        public S2MM_adcDMA( UInt32 boardNum, FPGAregister FPGAregisters) // Constructor
        {
            _PCIboardIndex = boardNum;
            _FPGAregs = FPGAregisters;
            DerivedTiming = new DERIVED_TIMING();
            // DISABLE FPGA (USER) interrupts until we initialize S2MM DMA
            // ENABLE NWL global interrupts required for DDR3 Read/Write TD API
	        _FPGAregs.Write("NWL_GlobalDMAIntEnable", 1); 
	        _FPGAregs.Write("NWL_UserIntEnable", 0); 
        }

        // convert Galvo-Galvo timing Dwell in nano-secs to FPGA register value
        // nominal limits (per ThorImageLS app) 400 ns - 20,000 ns
        // the register size is 28 bits

        private bool DwellTimeInRegisterTicks(UInt64 Dwell_ns, ref UInt64 DwellTicks)
        {
            bool bStatus = true;

            // convert nano-seconds to seconds
            double dDwellSecs = Dwell_ns / 1000000000.0; // nano-secs per second
            double dTicks = dDwellSecs * FPGA_SYSCLOCKFREQ;

            Int64 RoundedTicks = (Int64)Math.Round(dTicks);
            // range check for valid FPGA entry
            if (RoundedTicks < 0)
            {
                RoundedTicks = 1;
                bStatus = false; // value was outside range
            }
            if (RoundedTicks > (2 ^ 28 - 1))
            {
                RoundedTicks = 2 ^ 28 - 1;
                bStatus = false; // value was outside range
            }
            DwellTicks = (UInt64)RoundedTicks;
            return bStatus;
        }

        private bool ComputeGalvoLineTime()
        {
            bool bStatus;
            UInt64 Galvo_Pixel_Dwell = 0;
            UInt64 Galvo_Pixel_Delay = 0;
            UInt64 Intra_Line_Delay = 0;
            UInt64 Intra_Frame_Delay = 0;
            // read current values of FPGA Registers 
            bStatus = _FPGAregs.Read("ScanningGalvoPixelDwell", ref Galvo_Pixel_Dwell);
            _FPGAregs.Read("ScanningGalvoPixelDelay", ref Galvo_Pixel_Delay);
            _FPGAregs.Read("ScanningIntraLineDelay", ref Intra_Line_Delay);
            _FPGAregs.Read("ScanningIntraFrameDelay", ref Intra_Frame_Delay);

            // see Pg. 60 SWUG -- compute FRAME time in nano-seconds
            // Register counts are min. of 5ns "clk_tick" - some counts (like Galvo_Pixel_Delay) are blocks_of_16 ticks
            T_line = ((Galvo_Pixel_Dwell* (UInt64)_HSize) + ((Galvo_Pixel_Delay*16 +1) * ((UInt64)_HSize-1)) + 2*(Intra_Line_Delay*16 +1));
            T_frame = ( (T_line * (UInt64)_VSize) - ((UInt64)Intra_Line_Delay * 16 + 1) + ((UInt64)Intra_Frame_Delay * 16 + 2) ) * 5; // 5ns per clk_tick
            DerivedTiming.T_frame = T_frame;
            DerivedTiming.T_line = T_line * 5;  // both in nano-secs

            DerivedTiming.Pixel_to_Pixel_timeNS = (((Galvo_Pixel_Delay * 16 + 1) + Galvo_Pixel_Dwell) * 5); // ns
            // Now we can compute ADCStreamScanningPeriodReg
            // ADC sampling from free-running 80MHz internal clock (X2) (no downsampling): 
            double dSamplesPerLine = DerivedTiming.T_line / 6.25;  // 160MHz - 6.25 ns period
            // test value of "samples per line" -- if it exceeds max allowable of 65532 then apply downsampling

            uint downSampleRate = 0;
            uint downSampleFactor = downSampleRate + 1;
            while ((uint)dSamplesPerLine / downSampleFactor > 65532)
            {
                downSampleFactor = ++downSampleRate + 1;
            }; 
            DerivedTiming.ADCStreamDownsamplingRate = downSampleRate;

            DerivedTiming.ADCStreamScanningPeriodReg = (ulong)Math.Ceiling(dSamplesPerLine / (double)downSampleFactor);

            return bStatus;
        }
        // configure and setup to make ADC stream DMA to DDR3 function --
        // that is, after this routine completes, setting GIGCR0_STOP_RUN bit is enough 
        // 
        public int Init(ref S2MM_CONFIG s2mmConfiguration, ref DERIVED_TIMING ThisDerivedTiming)
        {
            UInt64 ImageSyncTimingMode = 0;
            int iStatus = 0; // 0 assumes success
            _FPGAregs.Read("TIME_SRCE_SELECT", ref ImageSyncTimingMode);
            
            _HSize = s2mmConfiguration.HSize;
            _VSize = s2mmConfiguration.VSize;
            _DDR3tartingAddr = s2mmConfiguration.DRAMstartingAddress;

            // compute the ADCStreamScanningPeriodReg for GalvoGalvo
            if (ImageSyncTimingMode == (UInt32)TDcontroller.ImageSyncMasterTimingMode.Galvo_Galvo)
            {
                ComputeGalvoLineTime();
                ThisDerivedTiming = DerivedTiming; // return copy to caller and keep ours
            }


            uint uiStatus = ThorDAQAPIS2MMconfig(_PCIboardIndex, ref s2mmConfiguration);
            if (uiStatus != 0)
                sErrMsg = string.Format("ThorDAQAPIS2MMconfig() error: {0} (0x{1}", uiStatus, uiStatus.ToString("X8"));
            return iStatus;
        }
        // LOAD the computed ADC sample line LookupTables
        public int LoadLUT(IntPtr ADCsampleLUT)
        {
            int iStatus = (int)THORDAQ_STATUS.STATUS_SUCCESSFUL;

            uint uiStatus = ThorDAQAPIADCsampleImagizerLUT(_PCIboardIndex, ADCsampleLUT);
            if (uiStatus != 0)
            {
                sErrMsg = string.Format("ThorDAQAPIADCsampleImagizerLUT() error: {0} (0x{1}", uiStatus, uiStatus.ToString("X8"));
                iStatus = (int)THORDAQ_STATUS.STATUS_PARAMETER_SETTINGS_ERROR;
            }
            return iStatus;
        }
        public void IRQcontrol(bool GlobalDMAIntEnable, bool UserIntEnable)
        {
            bool bStatus;
            if( GlobalDMAIntEnable == true)
                bStatus = _FPGAregs.Write("NWL_GlobalDMAIntEnable", 1);
            else
                _FPGAregs.Write("NWL_GlobalDMAIntEnable", 0);

            if( UserIntEnable == true)
                bStatus =_FPGAregs.Write("NWL_UserIntEnable", 1);
            else
                _FPGAregs.Write("NWL_UserIntEnable", 0); 
        }
    };


    public class DACwave // ThorDAQ's Digital Analog Converter 12-channel BAR3
    {
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIDACwaveInit")]
        public extern static uint ThorDAQAPIDACwaveInit(
                    UInt32 board,                  // Board number to target
                    UInt64 DDR3startAddr,          // upper DDR3 memory segment for DAC use
                    UInt32 WaveformByteLen);       // size of waveform (must be same for all channels)  

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIDACwaveLoad")]
        public extern static uint ThorDAQAPIDACwaveLoad(
                    UInt32 board,                  // Board number to target
                    UInt32 DACchannel,             // DAC hardware channel 
                    IntPtr DACsampleBuffer,        // array of 16-bit waveform samples to load
                    UInt32 BufferSize,             // Total size (bytes) of sample buffer
                    out UInt64 DDR3startAddr);     // if successful, the DDR3 start addr where waveform is loaded
        public string sErrMsg; // public diagnostic message for last error
        private UInt32 _PCIboardIndex;
        public DACwave(UInt32 boardNum)
        {
            _PCIboardIndex = boardNum;
        }

        public bool Init(UInt64 DDR3startAddress, UInt32 MaxWaveformSize)
        {
            try  // e.g. in case DLL is missing function
            {
                uint uiStatus = ThorDAQAPIDACwaveInit(_PCIboardIndex, DDR3startAddress, MaxWaveformSize);
            }
            catch (Exception e)
            {
                sErrMsg = "ThorDAQAPIDACwaveInit error (non-functioning Waveforms): " + e.Message;
                return false;
            }
            return true;
        }
        // load waveform from ASCII (text) file
        public int Load(uint DACchannelIndex, string filespec, out UInt64 DDR3startAddress)
        {
            int iStatus = 0; // 0 assumes no samples loaded (failed)
            // Can file be read?
            int DMAtransferSizeBytes = 0;
            UInt16[] Waveform16bitSamplesArray; // the raw DAC sample array
            UInt32 uiNumDACsamples = 0;
            IntPtr WaveFormINTptr = IntPtr.Zero; // marshalled version for C++ DLL passing
            DDR3startAddress = 0; // default is failed load

            try
            {   // Open the text file using a stream reader.
                uiNumDACsamples = (uint)File.ReadAllLines(filespec).Count(); // 
                Waveform16bitSamplesArray = new UInt16[uiNumDACsamples];
                DMAtransferSizeBytes = (int)uiNumDACsamples * 2;
                //Win32.AllocateBuffer(ref _WaveFormINTptr, (DMAtransferSizeBytes));
                WaveFormINTptr = Marshal.AllocHGlobal(DMAtransferSizeBytes);

                using (StreamReader sr = new StreamReader(filespec))
                {
                    for (int i = 0; i < uiNumDACsamples; i++)
                    {
                        // Read the stream to a string, and write the string to the console.
                        String line = sr.ReadLine(); // one line per 16-bit DAC sample
                        Waveform16bitSamplesArray[i] = UInt16.Parse(line);
                        //    move 'i' index by 2 bytes each write
                        System.Runtime.InteropServices.Marshal.WriteInt16(WaveFormINTptr, i * 2, (Int16)Waveform16bitSamplesArray[i]); // unmanaged memory copy
                    }
                }
            }
            catch (Exception e)
            {
                Marshal.FreeHGlobal(WaveFormINTptr);
                sErrMsg = "Failed reading DAC wave file '" + filespec + "' Error: " + e.Message;
                return -1;
            }

            // load waveform into DDR3 and create CPLD waveform descriptor table
            uint uiStatus = ThorDAQAPIDACwaveLoad(_PCIboardIndex, DACchannelIndex, WaveFormINTptr, (uint)DMAtransferSizeBytes, out DDR3startAddress);
            if (uiStatus == 0) // were we able to describe waveform in table?
            {
                iStatus = (int)uiNumDACsamples;
            }
            else
                sErrMsg = "ThorDAQAPIDACwaveLoad() error: invalid Waveform Size?";

            Marshal.FreeHGlobal(WaveFormINTptr);
            return iStatus;  // load failed, assume no samples
        }
        // function loads data array of 16-bit DAC samples to hardware channel
        // if successful returns DDR3 start address
        // all channels must be loaded 
        public bool Load(uint DACchannelIndex, IntPtr UnManagedDataArray, uint WaveformByteLen, out UInt64 DDR3startAddress)
        {
            uint uiStatus = ThorDAQAPIDACwaveLoad(_PCIboardIndex, DACchannelIndex, UnManagedDataArray, WaveformByteLen, out DDR3startAddress);

            return true;
        }
        //  start/stop one or more channels
        public bool Control(UInt32 DACchannelMask, bool Start)
        {

            return true;
        }
    }
    // 

    // This class enables access to the on-board DDR3 memory device
    // While BAR address space may be accessed with DoMem() CPU based writes,
    // DRAM can only be accessed by DMA
    // The 4 basic NWL API calls to access DRAM through DMA follow -- 
    // SetupPacketMode MUST SUCCEED before calls to read/write
    //
    // SetupPacketMode() -- allocates DMA descriptors (in Host mem) to describe virtual memory buffers
    //                      for performance, call once in constructor
    // PacketReadEx()
    // PacketWriteEx()
    // ShutdownPacketMode() -- for performance, call once in destructor

    public class TDmemoryDDR3 // ThorDAQ's on board DRAM,accessible through NWL (AXI) DMA
    {

        // NWL SDK functions for Packet (Addressable) DMA to ThorDAQ on-board DDR3 DRAM
     //   [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPISetupPacketMode")]
      /*  public extern static uint ThorDAQAPISetupPacketMode(
                    UInt32 board,                  // Board number to target
                    Int32 EngineOffset,            // DMA Engine number offset to use
                    IntPtr Buffer,                 // Data buffer
                    ref UInt32 BufferSize,         // Total size of Recieve Buffer
                    ref UInt32 MaxPacketSize,      // Largest single packet size
                    Int32 PacketMode,              // Sets mode, FIFO or Addressable Packet mode
                    Int32 NumberDescriptors);      // Number of DMA Descriptors to allocate (Addressable mode)*/

        
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIReadDDR3")]
        public extern static uint ThorDAQAPIReadDDR3(
                    UInt32 boardNum,               // Board to target
                    UInt64 CardOffset,             // (ThorDAQ DRAM) Card Address to start read from
                    IntPtr Buffer,                 // Address of (host) unmanaged data buffer
                    ref UInt32 Length);            // Length to Read

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIWriteDDR3")]
        public extern static uint ThorDAQAPIWriteDDR3(
                    UInt32 boardNum,               // Board to target
                    IntPtr Buffer,                 // Address of (host) unmanaged data buffer
                    UInt64 CardOffset,             // (ThorDAQ DRAM) Card Address to start read from
                    UInt32 Length);                // Length to Read

  /*      [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIShutdownPacketMode")]
        public extern static uint ThorDAQAPIShutdownPacketMode(
                    UInt32 board,     // Board number to target
                    UInt32 EngineOffset);            // DMA Engine number offset to use
  */
        // class variables
        private UInt32 _boardIndx;
        private UInt64 _DDR3offset; // ThorDAQ DDR3 physical address (starts @ 0x0)

        public string sErrMsg; // public diagnostic message for last error

        // constructor
        public TDmemoryDDR3(UInt32 boardNum)
        {
            _boardIndx = boardNum;
        }

        public bool ReadDDR3(UInt64 DDR3offset, ref UInt32 CallersLengthInBytes, ref Byte[] ManagedByteBuff)
        {
            bool bStatus = true;
            uint uiStatus;
            IntPtr ReadBuffer = IntPtr.Zero;
            _DDR3offset = DDR3offset;
            // setup the READ MDL descriptors
            try
            {
                Win32.AllocateBuffer(ref ReadBuffer, CallersLengthInBytes);
                uiStatus = ThorDAQAPIReadDDR3(_boardIndx, _DDR3offset, ReadBuffer, ref CallersLengthInBytes);
                // caller checks to see if all bytes done - DMA may transfer less
                if (uiStatus != 0)
                {
                    bStatus = false;
                }
                else // success - copy unmanaged memory back to caller
                {
                    Marshal.Copy(ReadBuffer, ManagedByteBuff, 0, (int)CallersLengthInBytes);
                }
                Win32.FreeBuffer(ReadBuffer); // release unmanaged mem back to heap
            }
            catch (Exception e)
            {
                sErrMsg = "ThorDAQAPIPacketReadEx() error: " + e.Message;
                bStatus = false;
            }
            return bStatus;



        }
        public bool WriteDDR3(UInt64 DDR3offset, ref UInt32 CallersLengthInBytes, Byte[] ManagedByteBuff)
        {
            bool bStatus = true;
            uint uiStatus;
            IntPtr UnmanagedWriteBuffer = IntPtr.Zero;
            _DDR3offset = DDR3offset;
            // setup the READ MDL descriptors
            try
            {
                Win32.AllocateBuffer(ref UnmanagedWriteBuffer, CallersLengthInBytes);
                // copy .NET managed memory to unmanaged memory
                Marshal.Copy(ManagedByteBuff, 0, UnmanagedWriteBuffer, (int)CallersLengthInBytes); // caller check's to see if all bytes done
                uiStatus = ThorDAQAPIWriteDDR3(_boardIndx, UnmanagedWriteBuffer, _DDR3offset, CallersLengthInBytes);
                if (uiStatus != 0)
                {
                    bStatus = false;
                }
                else // success - copy unmanaged memory back to caller
                {
                }
                Win32.FreeBuffer(UnmanagedWriteBuffer); // release unmanaged mem back to heap
            }
            catch (Exception e)
            {
                sErrMsg = "ThorDAQAPIPacketWriteEx() error: " + e.Message;
                bStatus = false;
            }
            return bStatus;
        }


    }

    // must match "thordaqcmd.h" REG_FIELD_DESC
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public unsafe struct REG_FIELD_DESC
    {
        public Int32 BAR;
        public Int32 BARoffset;
        public Int32 RegisterByteSize;   // byte length (1-8)
        public Int32 BitFieldOffset; // -1 if REGISTER reference
        public Int32 BitFieldLen;    // only for BitField
        public Int32 bWriteOnly;
        public Int32 bReadOnly;
    };

    public class BARs  // the PCIe Base Address Registers as discovered by kernel driver
    {
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIDoMem")]
        public extern static uint ThorDAQAPIDoMem(
            UInt32 board,
            UInt32 Rd_Wr_n,        // 1==Read, 0==Write
            UInt32 BarNum,         // Base Address Register (BAR) to access
            IntPtr Buffer,         // Data buffer
            UInt64 Offset,         // Offset in data buffer to start transfer
            UInt64 CardOffset,     // Offset in BAR to start transfer
            UInt64 Length,         // Byte length of transfer
            ref UInt64 completeByteCnt);      // returned count

        private uint _BoardIndex;
        public BARs(UInt32 BoardIndex)
        {
            _BoardIndex = BoardIndex;
        }

        // BARnum - zero-based Index to PCI Base Address Register
        // Offset - byte offset from start of BAR
        // Buffer - unmanaged memory pointer for read/write BAR data
        // Length - length of read/write data buffer
        public string Read(uint BARnum, ulong BARoffset, IntPtr Buffer, UInt64 Length)
        {
            UInt64 completedByteCnt = 0;

            uint uiStatus = ThorDAQAPIDoMem(_BoardIndex, 1, BARnum, Buffer, 0, BARoffset, Length, ref completedByteCnt);

            if (uiStatus == (uint)THORDAQ_STATUS.STATUS_SUCCESSFUL)
                return "OK";
            else
                return string.Format("BAR read Win32 error 0x{0:X}, Invalid parameter?", uiStatus);
        }
        public string Write(uint BARnum, ulong BARoffset, IntPtr Buffer, UInt64 Length)
        {
            UInt64 completedByteCnt = 0;

            uint uiStatus = ThorDAQAPIDoMem(_BoardIndex, 0, BARnum, Buffer, 0, BARoffset, Length, ref completedByteCnt);

            if (uiStatus == (uint)THORDAQ_STATUS.STATUS_SUCCESSFUL)
                return "OK";
            else
                return string.Format("BAR write Win32 error 0x{0:X}, Invalid parameter?", uiStatus); ;
        }

    }
    // 
    public class FPGAregister
    {
//        THORDAQ_API THORDAQ_STATUS ThorDAQAPIFPGAregisterRead(
//	UINT32 board,					// Board index to target
//	const char* pName,				// name of register/field (if found)
//	int nameSize,					// char array size
//	UINT64  *Value					// "shadow" DLL register copy value (REG or FIELD)
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIFPGAregisterWrite")]
        public extern static uint FPGAregisterWrite(UInt32 board,
            IntPtr pRegisterFieldName,
            Int32 FldNameSize, // in bytes
            IntPtr Value);     // DLL/driver Register/Field (unManagedMem) value to write

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIFPGAregisterRead")]
        public extern static uint FPGAregisterRead(UInt32 board,
            IntPtr pRegisterFieldName,
            Int32 FldNameSize, // in bytes
            IntPtr pValue);    // DLL/driver returned value of REGISTER or FIELD    


        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIFPGAregisterQuery")]
        public extern static uint FPGAregisterQuery(UInt32 board,
            Int32 RegIndex,  // 0-based array of Registers
            Int32 FldIndex,  // bit field in Register
            IntPtr pRegisterFieldName,
            Int32 FldNameSize, // in bytes
            ref REG_FIELD_DESC RegFldDesc);
 //       private string _RegName; // i.e. From ThorDAQ Software User Guide
        private uint _BoardIndex;
 //       private UInt32 _BAR;
 //       private UInt64 _OffsetInBAR;
        private IntPtr _NativeBuffer; // "unmanaged" data copy for C++ DLL
        private IntPtr _UINT64_NativeBuffer;
        private int _RegFldNameBufLen;
 //       private UInt64 _Offset;  
 //       private UInt64 _Length;  // len in bytes
        // constructor for ByteLen size FPGA register (i.e. 1, 2, or 4)
        public FPGAregister(UInt32 BoardIndex)
        {
            _BoardIndex = BoardIndex;
            _RegFldNameBufLen = 48;
            _NativeBuffer = Marshal.AllocHGlobal( _RegFldNameBufLen); // should never fail
            _UINT64_NativeBuffer = Marshal.AllocHGlobal(sizeof(UInt64)*2); // should never fail

        }
        ~FPGAregister()
        {
            Marshal.FreeHGlobal(_NativeBuffer);
            Marshal.FreeHGlobal(_UINT64_NativeBuffer);
        }

        // Usage:  RegIndex -- 0 to MaxRegisters (increment until error returned
        //         FldIndex -- (-1) to get Register, 0 to MaxField (increment until error on query)
        //         sName    -- string of Register or Field Name, or null to query by Reg/Fld
        //                  -- string is returned if Reg/Fld is valid
        //         RegFldDesc -- structure with details of Reg/Fld
        public UInt32 Query(Int32 RegIndex, Int32 FldIndex, ref string sName, uint sNameLen, ref REG_FIELD_DESC UnsafeRegFldDesc)
        {
            uint uiStatus = FPGAregisterQuery(_BoardIndex, RegIndex, FldIndex, _NativeBuffer, (int)_RegFldNameBufLen, ref UnsafeRegFldDesc);
            if( uiStatus == (uint)THORDAQ_STATUS.STATUS_SUCCESSFUL)
                sName = Marshal.PtrToStringAnsi(_NativeBuffer);
            return uiStatus;
        }
        public bool Read(string sRegFldName, ref UInt64 Value)
        {
            bool bStatus = true;
            IntPtr NativeNameBuf = Marshal.StringToCoTaskMemAnsi(sRegFldName);
            uint uiStatus = FPGAregisterRead(_BoardIndex, NativeNameBuf, sRegFldName.Length, _UINT64_NativeBuffer);
            if (uiStatus != 0)
            {
                bStatus = false;
            }
            else
            {
                Value = (UInt64)Marshal.ReadInt64(_UINT64_NativeBuffer);
            }
            // remember to free unmanaged mem
            Marshal.FreeCoTaskMem(NativeNameBuf);
            return bStatus;
        }
        public bool Write(string sRegFldName, UInt64 value)
        {
            bool bStatus = true;
            IntPtr NativeNameBuf = Marshal.StringToCoTaskMemAnsi(sRegFldName);
            Marshal.WriteInt64(_UINT64_NativeBuffer, (long)value);
            uint uiStatus = FPGAregisterWrite(_BoardIndex, NativeNameBuf, sRegFldName.Length, _UINT64_NativeBuffer);
            if (uiStatus != 0)
                bStatus = false;  // e.g. Name not found
            // remember to free unmanaged mem
            Marshal.FreeCoTaskMem(NativeNameBuf);
            return bStatus;
        }
    }

    public class TDcomponent
    {
        const uint BUSY_STATUS = 0x00000100;
        const uint NO_DEV = 0x00000200;
        const uint UNKNOWN_ERR = 0x00000400;
        const uint SER_NUM_SZ = 6;
        const uint BATCH_NUM_SZ = 6;
        const uint PART_NUM_SZ = 6;
        const uint DATE_SZ = 10;
        public const uint TOT_SERIAL_SZ = SER_NUM_SZ + BATCH_NUM_SZ + PART_NUM_SZ + DATE_SZ;
        const int WAIT_TIME = 15; // was 50
        const int NUM_ITER = 10;
        const uint BAR3 = 3;

        // every ThorDAQ component MUST be accessed through kernel driver index 
        // which identifies the complete hardware board
        protected uint _PCIeCardIndex;     // every component accessed through driver index for entire card 
        protected string _ModuleName;     // e.g. "MainBoard", "ADCcard", "BBA1", etc.
        protected string _ProductionData; // e.g. SerialNum, model, batch, date ...
        protected string _ExceptionMessage; // last critical object error
        // Every component with I2C accessible EEPROM requires physical address location
        protected string _MasterI2Cchannel;  // component device channels plus slave channels
        protected string _SlaveI2Cchannel;   // currently DAC card plus analog breakout boxes
        protected string _I2C_EEPROMreadDevAddr;    // bit mask to address and read I2C connected device
        protected string _I2C_EEPROMwriteDevAddr;   // address mask for write
        protected string _I2C_BB_LEDwriteDevAddr; // TI PCA9554 LED controller I2C Dev Addr
        private bool _bPCA9554_initialized;  // BBox power-on default must be switch once to OUTPUT mode
        private byte _bPCA9554_state;        // current bit-mask for the 8 outputs on BBox LED chip

        public TDcomponent(string ModuleName, uint PCIeCardIndex)
        {
            _ModuleName = ModuleName;
            _PCIeCardIndex = PCIeCardIndex;
            _ProductionData = "Unknown";
            _ExceptionMessage = "no error";
            _bPCA9554_state = 0xFF; // power-on state is OFF, wired as inverted polarity (1 = off)
            _bPCA9554_initialized = false; // 9554 configuration register C7-C0 requires one time init

        }
        // Product name for component
        public string ModuleName
        {
            get { return _ModuleName; }
            set { _ModuleName = value; }
        }
        // Provide for a general error message for object
        public string ExceptionMessage
        {
            get { return _ExceptionMessage; }
            set { _ExceptionMessage = value; }
        }
        // The serial number, batch, date and other manufacturing data
        public string ProductionData // access the local variable
        {
            get { return _ProductionData; }
            set { _ProductionData = value; }
        }

        // Methods to read and write I2C bus chip devices like
        // EEPROM Production data, e.g. manuf. date, batch, serial num., etc.
        // Components are connected to "channels" on a Master I2C multiplex bus, or behind 
        // a Slave I2C Multiplexed bus accessed via a single Master I2C channel
        // This means the Slave EEPROM component access needs to setup steps.
        protected bool WriteI2Cdevice(uint devInst, IntPtr buffer, UInt64 bufferLen, UInt64 address, string commmand)
        {
            commmand = commmand.Replace("0x", string.Empty);
            for (int i = 0; i < ((int)bufferLen * 2 - commmand.Length); i++)
            {
                commmand = '0' + commmand;
            }
            for (int i = 0; i < (int)bufferLen; i++)
            {
                byte byteValue = byte.Parse(commmand.Substring(i * 2, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture);
                System.Runtime.InteropServices.Marshal.WriteByte(buffer, (int)bufferLen - 1 - i, byteValue);
            }
            return Win32.MemoryWrite((uint)devInst, buffer, BAR3, address, 0, bufferLen);
        }
        private UInt32 ReadI2CstatusRegister(uint devInst, IntPtr buffer)
        {
            if (!(Win32.MemoryRead((uint)devInst, buffer, BAR3, (UInt64)0x02c0, 0, 4)))
            {
                return UNKNOWN_ERR;
            }

            UInt32 result = (UInt32)System.Runtime.InteropServices.Marshal.ReadInt32(buffer);
            return result;
        }

        // I2C multiplexed channel devices must be setup in series process -
        // ALWAYS set up the master, then poll "busy" bits for success
        // If successful, and slave is needed, setup slave, then read "busy"
        // for success.  For Slave connected components, repeat to set up slave channel
        // TI 9548 Master and 9546 Slave I2C MUX setup function
        protected bool Setup_I2C_mux(bool bMaster, string i2cChannel)
        {
            IntPtr i2CBuffer = IntPtr.Zero;
            IntPtr stbuffer = IntPtr.Zero;
            Win32.AllocateBuffer(ref stbuffer, 4);
            Win32.AllocateBuffer(ref i2CBuffer, 4);
            bool bRetVal = false;

            // ThorDAQ I2C Subsystem Registers
            // 0x2C8 bits:
            // 0:  I2C Command Valid Pulse (active high)
            // 1:  I2C MUX RESET (active low)
            if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 4, 0x02C8, "0x00000002")) // stop
            {
                goto FreeBuffers;
            }
            if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 4, 0x02C0, i2cChannel)) // command(channel)
            {
                goto FreeBuffers;
            }

            string i2cMultiplexer; // i.e., either the Master TI 9548, or slave TI 9546
            if (bMaster) i2cMultiplexer = "0x00000271"; else i2cMultiplexer = "0x00000270";
            if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 4, 0x02C4, i2cMultiplexer)) // device addr
            {
                goto FreeBuffers;
            }
            if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 4, 0x02C8, "0x00000003")) // start
            {
                goto FreeBuffers;
            }
            bRetVal = true; // success - all other paths failed
        FreeBuffers:
            if (stbuffer != IntPtr.Zero) Win32.FreeBuffer(stbuffer);
            if (i2CBuffer != IntPtr.Zero) Win32.FreeBuffer(i2CBuffer);

            return bRetVal;
        }
        // check setup status of EEPROM (or I2C slave)
        // iterate multiple times with brief wait checking for good status
        protected bool I2CendpointDevicePresent_Ready()
        {
            bool bRetVal = false;
            int iter1 = 0;
            UInt32 busyStatus;
            IntPtr stbuffer = IntPtr.Zero;
            Win32.AllocateBuffer(ref stbuffer, 4);
            do
            {
                System.Threading.Thread.Sleep(WAIT_TIME);
                busyStatus = ReadI2CstatusRegister(_PCIeCardIndex, stbuffer);
                if (Convert.ToBoolean(busyStatus & NO_DEV) || Convert.ToBoolean(busyStatus & UNKNOWN_ERR) || iter1 >= NUM_ITER)
                {
                    goto FreeBuffers;
                }
                iter1++;
            } while (Convert.ToBoolean(busyStatus & BUSY_STATUS) && iter1 < NUM_ITER);
            bRetVal = true; // success - all other paths failed
        FreeBuffers:
            if (stbuffer != IntPtr.Zero) Win32.FreeBuffer(stbuffer);

            return bRetVal;
        }

        //private bool Setup_I2CMUX_forBB_LED(string MasterI2Cchan, string SlaveI2Cchan, string I2CdevAddr)
        private bool Setup_I2CMUX_forBB_LED()
        {
            bool bRetVal = true;
            // MASTER I2C must ALWAYS be set up
            if (!Setup_I2C_mux(true, _MasterI2Cchannel)) // e.g. "0x00000080" for MainBrd EEPROM
            {
                ExceptionMessage = "Failed I2C mux setup";
                return false;
            }
            if (!I2CendpointDevicePresent_Ready())
            {
                ExceptionMessage = "Failed - LED missing or error";
                return false;
            }
            // if component is attached through Slave TI 9546 I2C bus, set that I2C up after
            // the master is successfully setup...
            if (_SlaveI2Cchannel != null)
            {
                if (!Setup_I2C_mux(false, _SlaveI2Cchannel)) // e.g. "0x00000004" for DAC EEPROM
                {
                    ExceptionMessage = "Failed SLAVE I2C mux setup";
                    return false;
                }
                if (!I2CendpointDevicePresent_Ready())
                {
                    ExceptionMessage = "Device NOT FOUND";
                    return false;
                }
            }
            // make certain PCA9554 is configured as OUTPUT (once)
            // and then READ current state of 9554 Output Register (match static state in chip)
            // 
            if (!_bPCA9554_initialized)
            {
                string sData = "00";
                bRetVal = I2C_DevCommand("03", ref sData);
                // 0 configures as OUTPUT
                if (bRetVal)
                    _bPCA9554_initialized = true;
            }
            return bRetVal;
        }


        // SYNTAX for LEDAddr is text label printed on Breakout Box, e.g. "AO1", "DIO8", "AI5"
        // The PCA9554 must be initialized for OUTPUT mode because power-on default is INPUT
        public bool BreakoutBoxLEDon(string LEDAddr)
        {
            return BBoxControlLED(LEDAddr, true);
        }
        public bool BreakoutBoxLEDoff(string LEDAddr)
        {
            return BBoxControlLED(LEDAddr, false);
        }

        //tesst
        public bool BBoxControlLEDs(string LEDAddr, string Pattern)
        {
            bool bRetVal = true;  // expected to return false if Breakout Box unplugged
            string sData = Pattern;

            switch (LEDAddr)
            {
                case "DIO1":
                    //if (Setup_I2CMUX_forBB_LED("0x00000002", null, "0x00000323") == false)
                    if (Setup_I2CMUX_forBB_LED() == false)
                        return false;
                    bRetVal = I2C_DevCommand("01", ref sData);

                    break;
                default:
                    break;

            }

            return bRetVal;
        }

        protected bool BBoxLEDonOff(byte BitPositionMask, bool bTurnOn)
        {
            string sData;
            //if (Setup_I2CMUX_forBB_LED(MasterI2Cchan, SlaveI2Cchan, I2CbusDevAddr) == false)
            if (Setup_I2CMUX_forBB_LED() == false)
                return false;
            if (bTurnOn) // on or off?  
            {
                if ((_bPCA9554_state & BitPositionMask) == BitPositionMask)
                {
                    _bPCA9554_state &= (byte)~BitPositionMask;
                }
            }
            else // turn OFF (set bit)
            {
                _bPCA9554_state |= BitPositionMask;
            }
            sData = _bPCA9554_state.ToString("X2");
            return (I2C_DevCommand("01", ref sData));
        }

        // NOTE: expected to return false if Breakout Box unplugged or invalid LED arg
        protected bool BBoxControlLED(string LEDAddr, bool bTurnOn)
        {
            const byte LED1 = 0x02;
            const byte LED2 = 0x01;
            const byte LED3 = 0x08;
            const byte LED4 = 0x04;
            const byte LED5 = 0x20; // note: ABBx boxes skip 5 and 6 (only 6 LEDs)
            const byte LED6 = 0x10;
            const byte LED7 = 0x80;
            const byte LED8 = 0x40;


            string BBox = LEDAddr.Substring(0, 3); // DIO or (AOx or AIx)
            string LEDnum;

            if (BBox == "DIO") // control a DBB1 breakout box LED
            {
                // set the bit mask for individual LED on box
                LEDnum = LEDAddr.Substring(3, 1); // i.e. the single digit label number
                switch (LEDnum)
                {
                    case "1":
                        return (BBoxLEDonOff(LED1, bTurnOn));
                    case "2":
                        return (BBoxLEDonOff(LED2, bTurnOn));
                    case "3":
                        return (BBoxLEDonOff(LED3, bTurnOn));
                    case "4":
                        return (BBoxLEDonOff(LED4, bTurnOn));
                    case "5":
                        return (BBoxLEDonOff(LED5, bTurnOn));
                    case "6":
                        return (BBoxLEDonOff(LED6, bTurnOn));
                    case "7":
                        return (BBoxLEDonOff(LED7, bTurnOn));
                    case "8":
                        return (BBoxLEDonOff(LED8, bTurnOn));
                    default:
                        return false; // bogus input
                };
            }
            else // ABBx breakout box LED, e.g. A07
            {
                switch (LEDAddr) // need all 4 characters
                {
                    case "AO1":
                    case "AO5":
                    case "AO9":
                        return (BBoxLEDonOff(LED1, bTurnOn));
                    case "AO2":
                    case "AO6":
                    case "AO10":
                        return (BBoxLEDonOff(LED2, bTurnOn));
                    case "AO3":
                    case "AO7":
                    case "AO11":
                        return (BBoxLEDonOff(LED3, bTurnOn));
                    case "AO4":
                    case "AO8":
                    case "AO12":
                        return (BBoxLEDonOff(LED4, bTurnOn));
                    case "AI1":
                    case "AI3":
                    case "AI5":
                        return (BBoxLEDonOff(LED7, bTurnOn));
                    case "AI2":
                    case "AI4":
                    case "AI6":
                        return (BBoxLEDonOff(LED8, bTurnOn));
                }

            }
            return false;  // invalid arg
        }

        // read the USERCODE (CMD 0xC0) from Lattic CPLD
        // Lattice "UseFlashM_EFB_MachX02_RefGuide.pdf" Table 74. pg 63 of 82
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        //                 EN        CMD    Operands   
        // UFM CFG NVR   Required   (Hex)     (Hex)    Data Mode   Data Size
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        //      x          Y/N       C0     00 00 00       R          4B
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // result to _ProductionData
        protected bool Read_Lattice_Usercode()
        {
            IntPtr i2CBuffer = IntPtr.Zero;
            IntPtr stbuffer = IntPtr.Zero;
            Win32.AllocateBuffer(ref stbuffer, 4);
            Win32.AllocateBuffer(ref i2CBuffer, 4);
            bool bRetVal = false;

            // MASTER I2C must ALWAYS be set up
            if (!Setup_I2C_mux(true, _MasterI2Cchannel)) // i.e. CPLD on DAC card, master I2C chan "08"
            {
                ExceptionMessage = "Failed I2C mux setup";
                goto FreeBuffers;
            }
            if (!I2CendpointDevicePresent_Ready())
            {
                ExceptionMessage = "Failed - CPLD missing or error";
                goto FreeBuffers;
            }

            // I2C bus should be ready - interact with CPLD; all addresses fixed in hardware

            if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 4, 0x02C0, "C0000000")) // 
            {
                ExceptionMessage = "I2C write failed at 0x02C0 DATA write register ";
                goto FreeBuffers;
            }
            if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 3, 0x02C4, "0x04A5C0")) // e.g. "0x0000353"
            {
                ExceptionMessage = "I2C write failed at 0x02C4 DevAddr reg ";
                goto FreeBuffers;
            }
            if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 1, 0x02C8, "0x02"))
            {
                ExceptionMessage = "I2C write failed at 0x02C8 command STOP";
                goto FreeBuffers;
            }
            if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 1, 0x02C8, "0x03"))
            {
                ExceptionMessage = "I2C write failed at 0x02C8 command START ";
                goto FreeBuffers;
            }
            int iter1 = 0;
            UInt32 busyStatus;
            do
            {
                System.Threading.Thread.Sleep(WAIT_TIME);
                busyStatus = ReadI2CstatusRegister(_PCIeCardIndex, stbuffer);
                if (Convert.ToBoolean(busyStatus & NO_DEV) || Convert.ToBoolean(busyStatus & UNKNOWN_ERR) || iter1 >= NUM_ITER)
                {
                    ExceptionMessage = "I2C write byte command failed ACK, status: 0x" + busyStatus.ToString("x");
                    goto FreeBuffers;
                }
                iter1++;
            } while (Convert.ToBoolean(busyStatus & BUSY_STATUS) && iter1 < NUM_ITER);

            // success - copy data
            ProductionData = busyStatus.ToString("X8");

            bRetVal = true;
        FreeBuffers:
            if (stbuffer != IntPtr.Zero) Win32.FreeBuffer(stbuffer);
            if (i2CBuffer != IntPtr.Zero) Win32.FreeBuffer(i2CBuffer);
            return bRetVal;
        }
        ////////////////////////////////////////////////////////////////////////////////////////////
        //
        // There must be at least 3 bytes for I2C bus write command:  
        //  ~~~~~~~~~~~~~~~~~~~~~    ~~~~~~~~~~~~~~~   ~~~~~~~~~~~~
        // |  i2c SlaveDevAddr   |  | Command/Index | | WriteData  |
        //  ~~~~~~~~~~~~~~~~~~~~~    ~~~~~~~~~~~~~~~   ~~~~~~~~~~~~
        // For EEPROM, Command/Index is always the byte Index for read/write location
        // For PCA9554 LED controller, Command code is either "configure" device, or select OutputRegister
        // Parameterized I2C device command (i.e., not assuming sequential addresses from 0)
        // The PCA 9554 defaults to "INPUT" and must be configured for OUTPUT at startup
        // e.g. to initialize PCA9554 LED controller, 0x353, 0x03 (Command-Config), 0x00 (config as OUTPUT)
        //      to set LED pattern,                   0x353, 0x01 (Output port Reg), 0xAA (turn on bottom LEDs)
        // See "TI PCA 9554 Remote 8-Bit I2C and SMBus I/O Epander With Int. Output and Config Registers", 
        // Rev. June 2014, pg 18.
        // The LEDs wiring results in reverse logic polarity when writing to 9554 output register (O7-O0)
        // 0 -- LED on
        // 1 -- LED off
        // ("Polarity inversion" in 9554 is only for inputs)
        // ALSO, while the label says (for instance) DIO1 on top, and DIO2 on bottom, the data register
        // bits are inverted (the least significant bit controls bottom LED, not the top per label)
        // LED
        // * DIO1 (bit 1)   DIO3 (bit 3) ...
        // * DIO2 (bit 0)   DIO4 (bit 2) 
        /// //////////////////////////////////////////////////////
        /// </remarks>
        protected bool I2C_DevCommand(string CommandByte, ref string Data)
        {
            IntPtr i2CBuffer = IntPtr.Zero;
            IntPtr stbuffer = IntPtr.Zero;
            Win32.AllocateBuffer(ref stbuffer, 4);
            Win32.AllocateBuffer(ref i2CBuffer, 4);
            bool bRetVal = true;
            string intvalstr = "0x" + CommandByte + "0000" + Data;
            //(Convert.ToByte(Data)).ToString("X2");

            if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 4, 0x02C8, "0x00000002"))
            {
                Data = "I2C write failed command STOP";
                goto FreeBuffers;
            }
            if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 4, 0x02C0, intvalstr)) // e.g. 0x03000000
            {
                Data = "I2C write failed at index DATA write register ";
                goto FreeBuffers;
            }
            if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 4, 0x02C4, _I2C_BB_LEDwriteDevAddr)) // e.g. "0x0000353"
            {
                Data = "I2C write failed at index EEPROM DevAddr reg " + _I2C_BB_LEDwriteDevAddr;
                goto FreeBuffers;
            }
            if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 4, 0x02C8, "0x00000003"))
            {
                Data = "I2C write failed at index command START " + _I2C_BB_LEDwriteDevAddr;
                goto FreeBuffers;
            }
            int iter1 = 0;
            UInt32 busyStatus;
            do
            {
                System.Threading.Thread.Sleep(WAIT_TIME);
                busyStatus = ReadI2CstatusRegister(_PCIeCardIndex, stbuffer);
                if (Convert.ToBoolean(busyStatus & NO_DEV) || Convert.ToBoolean(busyStatus & UNKNOWN_ERR) || iter1 >= NUM_ITER)
                {
                    Data = "I2C write byte command failed ACK, status: 0x" + busyStatus.ToString("x");
                    goto FreeBuffers;
                }
                iter1++;
            } while (Convert.ToBoolean(busyStatus & BUSY_STATUS) && iter1 < NUM_ITER);
        FreeBuffers:
            if (stbuffer != IntPtr.Zero) Win32.FreeBuffer(stbuffer);
            if (i2CBuffer != IntPtr.Zero) Win32.FreeBuffer(i2CBuffer);

            return bRetVal;
        }
        // WRITE data to device on I2C bus
        protected bool I2C_SetDataBytes(int length_of_string, string I2CdevAddr, ref string DataToWrite)
        {
            IntPtr i2CBuffer = IntPtr.Zero;
            IntPtr stbuffer = IntPtr.Zero;
            Win32.AllocateBuffer(ref stbuffer, 4);
            Win32.AllocateBuffer(ref i2CBuffer, 4);
            bool bRetVal = false;
            int indx1 = 0;

            foreach (char ch in DataToWrite)
            {
                string intvalstr = "0x" + indx1.ToString("X2") + "0000" +
                    (Convert.ToByte(ch)).ToString("X2");

                if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 4, 0x02C8, "0x00000002"))
                {
                    DataToWrite = "I2C write failed at index " + indx1 + " : command STOP";
                    goto FreeBuffers;
                }
                if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 4, 0x02C0, intvalstr))
                {
                    DataToWrite = "I2C write failed at index " + indx1 + " : DATA write register ";
                    goto FreeBuffers;
                }
                if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 4, 0x02C4, I2CdevAddr)) // e.g. "0x0000354"
                {
                    DataToWrite = "I2C write failed at index " + indx1 + " : EEPROM DevAddr reg" + I2CdevAddr;
                    goto FreeBuffers;
                }
                if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 4, 0x02C8, "0x00000003"))
                {
                    DataToWrite = "I2C write failed at index " + indx1 + " : command START " + I2CdevAddr;
                    goto FreeBuffers;
                }
                int iter1 = 0;
                UInt32 busyStatus;
                do
                {
                    System.Threading.Thread.Sleep(WAIT_TIME);
                    busyStatus = ReadI2CstatusRegister(_PCIeCardIndex, stbuffer);
                    if (Convert.ToBoolean(busyStatus & NO_DEV) || Convert.ToBoolean(busyStatus & UNKNOWN_ERR) || iter1 >= NUM_ITER)
                    {
                        DataToWrite = "I2C write failed at index [" + indx1 + "]  DATA written=>" + DataToWrite.Substring(0, indx1);
                        goto FreeBuffers;
                    }
                    iter1++;
                } while (Convert.ToBoolean(busyStatus & BUSY_STATUS) && iter1 < NUM_ITER);

                // next byte to write or done
                if (++indx1 >= length_of_string) break;
            }
            // clear buffer memory from lib (NW Logic) functions
            bRetVal = true; // success - all other paths failed
        FreeBuffers:
            if (stbuffer != IntPtr.Zero) Win32.FreeBuffer(stbuffer);
            if (i2CBuffer != IntPtr.Zero) Win32.FreeBuffer(i2CBuffer);

            return bRetVal;
        }
        // READ the data from I2C channel selected component EEPROM
        protected bool I2C_GetDataBytes(int length_of_string, string EEPromDevAddr, ref string serialStr)
        {
            serialStr = string.Empty;
            IntPtr i2CBuffer = IntPtr.Zero;
            IntPtr stbuffer = IntPtr.Zero;
            Win32.AllocateBuffer(ref stbuffer, 4);
            Win32.AllocateBuffer(ref i2CBuffer, 4);
            bool bRetVal = false;

            for (int i = 0; i < length_of_string; i++)
            {
                string intvalstr = "0x" + i.ToString("X8");
                if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 4, 0x02C8, "0x00000002"))
                {
                    serialStr = "EEPROM read failed at index " + i + " : command STOP";
                    goto FreeBuffers;
                }
                if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 4, 0x02C0, intvalstr))
                {
                    serialStr = "EEPROM read failed at index " + i + " : ADDR command register ";
                    goto FreeBuffers;
                }
                if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 4, 0x02C4, EEPromDevAddr)) // e.g. "0x000122D4"
                {
                    serialStr = "EEPROM read failed at index " + i + " : Writing EEPROM Addr " + EEPromDevAddr;
                    goto FreeBuffers;
                }
                if (!WriteI2Cdevice(_PCIeCardIndex, i2CBuffer, 4, 0x02C8, "0x00000003"))
                {
                    serialStr = "EEPROM read failed at index " + i + " : command START " + EEPromDevAddr;
                    goto FreeBuffers;
                }
                int iter1 = 0;
                UInt32 busyStatus;
                do
                {
                    System.Threading.Thread.Sleep(WAIT_TIME);
                    busyStatus = ReadI2CstatusRegister(_PCIeCardIndex, stbuffer);
                    if (Convert.ToBoolean(busyStatus & NO_DEV) || Convert.ToBoolean(busyStatus & UNKNOWN_ERR) || iter1 >= NUM_ITER)
                    {
                        string sFailReason;
                        if (Convert.ToBoolean(busyStatus & NO_DEV)) sFailReason = "NO DEVICE"; else sFailReason = "UNKNOWN_ERR";
                        serialStr = "EEPROM " + sFailReason + " read failure at index [" + i + "]  DATA read=>" + serialStr;
                        goto FreeBuffers;
                    }
                    iter1++;
                } while (Convert.ToBoolean(busyStatus & BUSY_STATUS) && iter1 < NUM_ITER);

                serialStr = serialStr + ((char)busyStatus).ToString();
            }
            // clear buffer memory from lib (NW Logic) functions
            bRetVal = true; // success - all other paths failed
        FreeBuffers:
            if (stbuffer != IntPtr.Zero) Win32.FreeBuffer(stbuffer);
            if (i2CBuffer != IntPtr.Zero) Win32.FreeBuffer(i2CBuffer);

            return bRetVal;
        }


        // Divide EEPROM access into three steps:
        // 1. Make sure TI I2C MUX accepts proper setup/command to connect to EEPROM component
        // 2. Make sure the EEPROM device responds when accessed through MUX
        // 3. Ensure all expected EEPROM data bytes are correctly read/written 
        //    - if not, note byte index err pos.
        // In case the end device is behind a SLAVE I2C, make additional TI I2C MUX setup
        // READ, or WRITE if 'DataToWrite' array is non-null
        // Return status or the Read EEPROM data and store object copy
        protected bool ReadOrWriteSelectedEEPROM(string DataToWrite)
        {
            // MASTER I2C must ALWAYS be set up
            if (!Setup_I2C_mux(true, _MasterI2Cchannel)) // e.g. "0x00000080" for MainBrd EEPROM
            {
                ExceptionMessage = "Failed I2C mux setup";
                return false;
            }
            if (!I2CendpointDevicePresent_Ready())
            {
                ExceptionMessage = "Failed - EEPROM missing or error";
                return false;
            }
            // if component is attached through Slave TI 9546 I2C bus, set that I2C up after
            // the master is successfully setup...
            if (_SlaveI2Cchannel != null)
            {
                if (!Setup_I2C_mux(false, _SlaveI2Cchannel)) // e.g. "0x00000004" for DAC EEPROM
                {
                    ExceptionMessage = "Failed SLAVE I2C mux setup";
                    return false;
                }
                if (!I2CendpointDevicePresent_Ready())
                {
                    ExceptionMessage = "Device NOT FOUND";
                    return false;
                }
            }
            // I2C(s) are successfully setup - ready to READ or WRITE
            if (DataToWrite != null)
            {
                if (!I2C_SetDataBytes((int)TOT_SERIAL_SZ, _I2C_EEPROMwriteDevAddr, ref DataToWrite))
                {
                    ExceptionMessage = DataToWrite;
                    return false;
                }
            }
            else if (!I2C_GetDataBytes((int)TOT_SERIAL_SZ, _I2C_EEPROMreadDevAddr, ref _ProductionData))
            {
                ExceptionMessage = _ProductionData;
                return false;
            }
            ExceptionMessage = "no error";
            return true;
        }
    }  // END class TDcomponent


    public class TDmainBrd : TDcomponent // features the main board and all modules have, e.g. serial number
    {
        private const string cName = "MainBrd"; // name the component
        public TDmainBrd(uint PCIeCardIndex)
            : base(cName, PCIeCardIndex)
        {
            _ProductionData = "Unknown";
            _ExceptionMessage = " ";
            // default physical I2C address
            _MasterI2Cchannel = "0x00000080"; // (hardware channel address does NOT match I2C chip docs - see schematics)
            _SlaveI2Cchannel = null;
            _I2C_EEPROMreadDevAddr = "0x000122D4";
            _I2C_EEPROMwriteDevAddr = "0x00000354";
        }
        // Read this component off the Master I2C chip...
        public bool ReadEEPROM()
        {
            // string sMasterChannel, string sSlaveChannel, string EEPROMdevAddr)
            return ReadOrWriteSelectedEEPROM(null);
        }
        public bool WriteEEPROM(string sPROMvalue)
        {
            // string sMasterChannel, string sSlaveChannel, string EEPROMdevAddr)
            return ReadOrWriteSelectedEEPROM(sPROMvalue);
        }
    }

    public class TD3Pcard : TDcomponent // features the main board and all modules have, e.g. serial number
    {
        private const string cName = "TRG3Pcard"; // name the component
        public TD3Pcard(uint PCIeCardIndex)
            : base(cName, PCIeCardIndex)
        {
            _ProductionData = "Unknown";
            _ExceptionMessage = " ";
            // default physical I2C address
            _MasterI2Cchannel = "0x00000001"; // (hardware channel address does NOT match I2C chip docs - see schematics)
            _SlaveI2Cchannel = null;
            _I2C_EEPROMreadDevAddr = "0x000122D0";
            _I2C_EEPROMwriteDevAddr = "0x00000350";
        }
        // Read this component off the Master I2C chip...
        public bool ReadEEPROM()
        {
            // string sMasterChannel, string sSlaveChannel, string EEPROMdevAddr)
            return ReadOrWriteSelectedEEPROM(null);
        }
        public bool WriteEEPROM(string sPROMvalue)
        {
            // string sMasterChannel, string sSlaveChannel, string EEPROMdevAddr)
            return ReadOrWriteSelectedEEPROM(sPROMvalue);
        }
    }
    public class TDADCcard : TDcomponent // features the main board and all modules have, e.g. serial number
    {
        private const string cName = "ADCcard"; // name the component
        public TDADCcard(uint PCIeCardIndex)
            : base(cName, PCIeCardIndex)
        {
            _ProductionData = "Unknown";
            _ExceptionMessage = " ";
            // default physical I2C address
            _MasterI2Cchannel = "0x00000002"; // (hardware channel address does NOT match I2C chip docs - see schematics)
            _SlaveI2Cchannel = null;
            _I2C_EEPROMreadDevAddr = "0x000122D4";
            _I2C_EEPROMwriteDevAddr = "0x00000354";
        }
        // Read this component off the Master I2C chip...
        public bool ReadEEPROM()
        {
            // string sMasterChannel, string sSlaveChannel, string EEPROMdevAddr)
            return ReadOrWriteSelectedEEPROM(null);
        }
        public bool WriteEEPROM(string sPROMvalue)
        {
            // string sMasterChannel, string sSlaveChannel, string EEPROMdevAddr)
            return ReadOrWriteSelectedEEPROM(sPROMvalue);
        }
    }
    public class TDDBB1extBox : TDcomponent // features the main board and all modules have, e.g. serial number
    {
        private const string cName = "DBB1box"; // name the component
        public TDDBB1extBox(uint PCIeCardIndex)
            : base(cName, PCIeCardIndex)
        {
            _ProductionData = "Unknown";
            _ExceptionMessage = " ";
            // default physical I2C address
            _MasterI2Cchannel = "0x00000002"; // (hardware channel address does NOT match I2C chip docs - see schematics)
            _SlaveI2Cchannel = null;
            _I2C_EEPROMreadDevAddr = "0x000122D0";
            _I2C_EEPROMwriteDevAddr = "0x00000350";
            _I2C_BB_LEDwriteDevAddr = "0x00000323";
        }
        // Read this component off the Master I2C chip...
        public bool ReadEEPROM()
        {
            // string sMasterChannel, string sSlaveChannel, string EEPROMdevAddr)
            return ReadOrWriteSelectedEEPROM(null);
        }
        public bool WriteEEPROM(string sPROMvalue)
        {
            // string sMasterChannel, string sSlaveChannel, string EEPROMdevAddr)
            return ReadOrWriteSelectedEEPROM(sPROMvalue);
        }
        public bool LEDon(string LEDlabel) // e.g. "DIO1"
        {
            return BreakoutBoxLEDon(LEDlabel);
        }
        public bool LEDoff(string LEDlabel)
        {
            return BreakoutBoxLEDoff(LEDlabel);
        }

    }

    // DAC mezannine card CPLD for 2P -- this only yields a VERSION code, no S/N
    public class TDCPLD2card : TDcomponent // features the main board and all modules have, e.g. serial number
    {
        private const string cName = "CPLD2";  // name the component
        public TDCPLD2card(uint PCIeCardIndex)
            : base(cName, PCIeCardIndex)
        {
            _ProductionData = "Unknown";
            _ExceptionMessage = " ";
            // default physical I2C address
            _MasterI2Cchannel = "0x0x00000008";  // in Leo code 0xC0000000
            _SlaveI2Cchannel = null;
            _I2C_EEPROMreadDevAddr = "0x000122C0";
            _I2C_EEPROMwriteDevAddr = "0x00000340";
        }
        public bool ReadCPLDver()  // 4 byte USERCODE, 0xC0 command
        {
            return Read_Lattice_Usercode();
        }
        // We do NOT write USERCODE to CPLD as this is part of flashed/programmed image
    }
    public class TDDACcard : TDcomponent // Digital to Analog Converter (2P) mezannine card
    {


        public DACwave DACWave; 
        private const string cName = "DACcard";  // name the component
        public TDDACcard(uint PCIeCardIndex)
            : base(cName, PCIeCardIndex)
        {
            _ProductionData = "Unknown";
            _ExceptionMessage = " ";
            // default physical I2C address
            _MasterI2Cchannel = "0x00000008";
            _SlaveI2Cchannel = "0x00000004";
            _I2C_EEPROMreadDevAddr = "0x000122D4";
            _I2C_EEPROMwriteDevAddr = "0x00000354";

            DACWave = new DACwave(PCIeCardIndex);  // instantiate DAC WAVEFORM functions
        }




        // Read this component off the Master->Slave I2C chip...
        public bool ReadEEPROM()
        {
            // string sMasterChannel, string sSlaveChannel, string EEPROMdevAddr)
            return ReadOrWriteSelectedEEPROM(null);
        }
        public bool WriteEEPROM(string sPROMvalue)
        {
            // string sMasterChannel, string sSlaveChannel, string EEPROMdevAddr)
            return ReadOrWriteSelectedEEPROM(sPROMvalue);
        }
    }
    public class TDABB1extBox : TDcomponent // External Break Out Box
    {
        private const string cName = "BBA1";  // name the component
        public TDABB1extBox(uint PCIeCardIndex)
            : base(cName, PCIeCardIndex)
        {
            _ProductionData = "Unknown";
            _ExceptionMessage = " ";
            // default physical I2C address
            _MasterI2Cchannel = "0x00000008";
            _SlaveI2Cchannel = "0x00000001";
            _I2C_EEPROMreadDevAddr = "0x000122D0";
            _I2C_EEPROMwriteDevAddr = "0x00000350";
            _I2C_BB_LEDwriteDevAddr = "0x00000323";

        }
        // Read this component off the Master->Slave I2C chip...
        public bool ReadEEPROM()
        {
            return ReadOrWriteSelectedEEPROM(null);
        }
        public bool WriteEEPROM(string sPROMvalue)
        {
            return ReadOrWriteSelectedEEPROM(sPROMvalue);
        }
        public bool LEDon(string LEDlabel) // 
        {
            return BreakoutBoxLEDon(LEDlabel);
        }
        public bool LEDoff(string LEDlabel)
        {
            return BreakoutBoxLEDoff(LEDlabel);
        }

    }
    public class TDABB2extBox : TDcomponent // External Break Out Box
    {
        private const string cName = "BBA2";  // name the component
        public TDABB2extBox(uint PCIeCardIndex)
            : base(cName, PCIeCardIndex)
        {
            _ProductionData = "Unknown";
            _ExceptionMessage = " ";
            // default physical I2C address
            _MasterI2Cchannel = "0x00000008";
            _SlaveI2Cchannel = "0x00000002";
            _I2C_EEPROMreadDevAddr = "0x000122D0";
            _I2C_EEPROMwriteDevAddr = "0x00000350";
            _I2C_BB_LEDwriteDevAddr = "0x00000323";
        }
        // Read this component off the Master->Slave I2C chip...
        public bool ReadEEPROM()
        {
            // string sMasterChannel, string sSlaveChannel, string EEPROMdevAddr)
            return ReadOrWriteSelectedEEPROM(null);
        }
        public bool WriteEEPROM(string sPROMvalue)
        {
            // string sMasterChannel, string sSlaveChannel, string EEPROMdevAddr)
            return ReadOrWriteSelectedEEPROM(sPROMvalue);
        }
        public bool LEDon(string LEDlabel) // 
        {
            return BreakoutBoxLEDon(LEDlabel);
        }
        public bool LEDoff(string LEDlabel)
        {
            return BreakoutBoxLEDoff(LEDlabel);
        }
    }
    public class TDABB3extBox : TDcomponent // External Break Out Box
    {
        private const string cName = "BBA3";  // name the component
        public TDABB3extBox(uint PCIeCardIndex)
            : base(cName, PCIeCardIndex)
        {
            _ProductionData = "Unknown";
            _ExceptionMessage = " ";
            // default physical I2C address
            _MasterI2Cchannel = "0x00000008";
            _SlaveI2Cchannel = "0x00000008";
            _I2C_EEPROMreadDevAddr = "0x000122D0";
            _I2C_EEPROMwriteDevAddr = "0x00000350";
            _I2C_BB_LEDwriteDevAddr = "0x00000323";
        }
        // Read this component off the Master->Slave I2C chip...
        public bool ReadEEPROM()
        {
            // string sMasterChannel, string sSlaveChannel, string EEPROMdevAddr)
            return ReadOrWriteSelectedEEPROM(null);
        }
        public bool WriteEEPROM(string sPROMvalue)
        {
            // string sMasterChannel, string sSlaveChannel, string EEPROMdevAddr)
            return ReadOrWriteSelectedEEPROM(sPROMvalue);
        }
        public bool LEDon(string LEDlabel) // 
        {
            return BreakoutBoxLEDon(LEDlabel);
        }
        public bool LEDoff(string LEDlabel)
        {
            return BreakoutBoxLEDoff(LEDlabel);
        }
    }


    public class TDassembly   // object for complete ThorDAQ board assembly
    {
        private uint _DriverPCIeCardIndex; // 0-based array index to boards discovered by kernel device driver

        private string _ThorDAQmodel; // e.. "ThorDAQ-2586"

        public TDmainBrd MainBrd;
        public TD3Pcard TRG3Pcard;
        public TDADCcard ADCcard;
        public TDDBB1extBox DBB1box;
        public TDDACcard DACcard;
        public TDABB1extBox ABB1box;
        public TDABB2extBox ABB2box;
        public TDABB3extBox ABB3box;
        public TDCPLD2card CPLD2dev; // CPLD on the DAC card (2P design)

        public TDmemoryDDR3 DDR3RAM; // the DDR3 memory bank

        public TDassembly(uint DevIndex, string ThorDAQmodel)
        {
            _DriverPCIeCardIndex = DevIndex;
            _ThorDAQmodel = ThorDAQmodel;
            MainBrd = new TDmainBrd(_DriverPCIeCardIndex);
            ADCcard = new TDADCcard(_DriverPCIeCardIndex);
            TRG3Pcard = new TD3Pcard(_DriverPCIeCardIndex);
            DBB1box = new TDDBB1extBox(_DriverPCIeCardIndex);
            DACcard = new TDDACcard(_DriverPCIeCardIndex);
            ABB1box = new TDABB1extBox(_DriverPCIeCardIndex);
            ABB2box = new TDABB2extBox(_DriverPCIeCardIndex);
            ABB3box = new TDABB3extBox(_DriverPCIeCardIndex);
            CPLD2dev = new TDCPLD2card(_DriverPCIeCardIndex);
            DDR3RAM = new TDmemoryDDR3(_DriverPCIeCardIndex);
        }
    }

}
