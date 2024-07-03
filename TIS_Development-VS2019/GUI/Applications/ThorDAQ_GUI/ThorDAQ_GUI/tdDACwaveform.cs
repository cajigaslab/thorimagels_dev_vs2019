namespace thordaqGUI
{
    using System;
    using System.IO; // file operations
    using System.Runtime.InteropServices; // DllImport
    using System.Threading; // Sleep
    using System.Linq;      // Count file lines
    using ThorLogging;        // required for logging
    using System.Diagnostics; // required for logging
    using ThorSharedTypes;
    using NWLogic.DeviceLib;  // Marshal .NET memory pointers, structs, FPGA register functions, etc.


    public class tdDACwaveform
    {
        private UInt32 _tdBoardNum; // zero based board index, 0 - 3 (max 4 boards)
        public IMAGING_CONFIGURATION_STRUCT _PacketGenChk = new IMAGING_CONFIGURATION_STRUCT();
        DAC_CRTL_STRUCT[] _DACctrlArray = new DAC_CRTL_STRUCT[Win32.DAC_CHANNEL_COUNT];
        // allocate the DAC channels, expected by Thordaq DLL
        private UInt64 _cardDDRmemStart = 0x50000000; // hardware address of ThorDAQ DDR3 DAC dedicated memory area
        private ulong _DACchannel;
        IntPtr _WaveFormINTptr = IntPtr.Zero; // unmanaged mem for moving Waveform (file) to DLL
        private IntPtr _hDACstructures = IntPtr.Zero; // unmanaged mem for the DAC_CRTL_STRUCT for all channels
        private uint _uiNumDACsamples = 0;  // total samples to DMA to FPGA memory (e.g. from playback file)
        private double _BitmapWidth = 512;
        private double _BitmapHeight = 512;

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIBindBoard")]
        public extern static uint ConnectToBoard(UInt32 board);
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIReleaseBoard")]
        public extern static uint ThorDAQAPIReleaseBoard(UInt32 board );

        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "FindCameras")]
        public static extern int FindGgCameras(out long cameraCnt);


        /// <summary>
        /// ThordaqPacketReadex - Calls the driver to Write DMA Data.
        /// </summary>
        /// <param name="board">Board number</param>
        /// <param name="ChannelIndex">Channel offset (0 based)</param>
        /// <param name="BufferSize">size of Data Buffer</param>
        /// <param name="Buffer">data buffer</param>
        /// <param name="Timeout">read timeout</param>
        /// <returns>Status of the operation</returns>
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIPacketWriteBuffer")]
        public extern static uint TDPacketWriteBuffer(UInt32 board,
                            ulong CardOffset, uint BufferSize, IntPtr Buffer, int Timeout);
        // _IMAGING_CONFIGURATION_STRUCT is the comparable C++ typedef for IMAGING_CONFIGURATION_STRUCT
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPISetImagingConfiguration")]
        public extern static uint TDSetImagingConfiguration(uint boardNum, ref IMAGING_CONFIGURATION_STRUCT PacketGenChk);  // a.k.a. IMAGING_CONFIGURATION_STRUCT, 

        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "PreflightAcquisition")]
        public static extern int TDPreflightAcquisition(byte[] pBuffer);  // arg not used
        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "SetupAcquisition")]
        public static extern int TDSetupAcquisition(byte[] pBuffer);  // arg not used

        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "PostflightAcquisition")]
        public static extern int TDPostflightAcquisition(byte[] pBuffer);  // arg not used


        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIStartAcquisitionImage")]
        public extern static uint TDStartAcquisition(UInt32 board, bool imageDMAstart);
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIStopAcquisition")]
        public extern static uint TDStopAcquisition(UInt32 board);
        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "SelectCamera")]
        public static extern int SelectGgCamera(long cameraCnt);
        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "SetParam")]
        public static extern int SetGgParam(long paramID, double value);


        private void TDSetParams()
        {
            int retStatus;
            retStatus = SetGgParam((int)ICamera.Params.PARAM_LSM_PIXEL_X, _BitmapWidth);
            retStatus = SetGgParam((int)ICamera.Params.PARAM_LSM_PIXEL_Y, _BitmapHeight);
            retStatus = SetGgParam((int)ICamera.Params.PARAM_LSM_NUMBER_OF_PLANES, 1);
            retStatus = SetGgParam((int)ICamera.Params.PARAM_LSM_CHANNEL, 1);

            retStatus = SetGgParam((int)ICamera.Params.PARAM_LSM_SCANMODE, 1); // one-way

            // ADC gains
            retStatus = SetGgParam((int)ICamera.Params.PARAM_LSM_INPUTRANGE1, 1); // 
            retStatus = SetGgParam((int)ICamera.Params.PARAM_LSM_INPUTRANGE2, 1); // 
            retStatus = SetGgParam((int)ICamera.Params.PARAM_LSM_INPUTRANGE3, 1); // 
            retStatus = SetGgParam((int)ICamera.Params.PARAM_LSM_INPUTRANGE4, 1); // 

            // set a reasonable PIXEL DWELL time
            retStatus = SetGgParam((int)ICamera.Params.PARAM_LSM_DWELL_TIME, 0.4);

            // set minimum averaging
            retStatus = SetGgParam((int)ICamera.Params.PARAM_LSM_AVERAGENUM, 2);
            // set minimum averaging
            retStatus = SetGgParam((int)ICamera.Params.PARAM_LSM_CLOCKSOURCE, 1);  // 1 internal, 2 external
            // set minimum averaging
            retStatus = SetGgParam((int)ICamera.Params.PARAM_LSM_EXTERNALCLOCKRATE, 160000000);
            // set minimum averaging
            retStatus = SetGgParam((int)ICamera.Params.PARAM_LSM_GG_TURNAROUNDTIME_US, 400);

            //PARAM_TRIGGER_MODE, 2
            // PARAM_LSM_GALVO_ENABLE, 1 (GG)
            // PARAM_MULTI_FRAME_COUNT, 1024562.0
        }

        public string playback()
        {
            string retStr = "Success";
            uint status;

            try
            {
                byte[] UnusedAcqArg = null;
                TDSetParams();
                status = (uint)TDPreflightAcquisition(UnusedAcqArg);
                status = (uint)TDSetupAcquisition(UnusedAcqArg);
                status = TDStartAcquisition(_tdBoardNum, true); // SUPPRESS image DMA
            }
            catch (Exception e)
            {
                retStr = "TDStartAcquisition() failed, error: " + e.Message;
            }
            return retStr;
        }
        public string stop()
        {
            string retStr = "Success";
            uint status;

            byte[] UnusedAcqArg = null;
            status = (uint)TDPostflightAcquisition(UnusedAcqArg);
            status = TDStopAcquisition(_tdBoardNum);
            return retStr;
        }

        
        
        // load DMA samples for Playback - return "Success" or error string
        public string LoadFromFile(UInt32 TDboardNum, string filename, int iFreq, ulong DACchannel) 
        {
            UInt16[] Waveform16bitSamplesArray; // the raw DAC sample array
            _WaveFormINTptr = IntPtr.Zero; // marshalled version for C++ DLL passing
            _tdBoardNum = TDboardNum; // remember for destructor
            _DACchannel = DACchannel;
            string retStr = "Success";
            _uiNumDACsamples = 0;   // total number of DAC wave samples read from file
            long cameraCnt;

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "tdDACwaveform: ThorDAQcl-GUI: call FindCameras()");
            int iStatus = FindGgCameras(out cameraCnt);
            if (iStatus <= 0)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ThorDAQ PCIe adapter not found");  // report error to log file
                return "Failed ConnectToBoard";
            }
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ThorDAQcl-GUI: call SelectCamera()");
            iStatus = SelectGgCamera(_tdBoardNum);
            //uint uiStatus;
//            uiStatus = ConnectToBoard(_tdBoardNum);
//            if (uiStatus != 0)
  //          {
    //            ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ThorDAQ PCIe adapter not found");  // report error to log file
      //          return "Failed ConnectToBoard";
        //    }

            // Load the waveform...
            // Can file be read?
            int DMAtransferSizeBytes = 0;
            try
            {   // Open the text file using a stream reader.
                _uiNumDACsamples = (uint)File.ReadAllLines(filename).Count(); // 
                Waveform16bitSamplesArray = new UInt16[_uiNumDACsamples];
                DMAtransferSizeBytes = (int)_uiNumDACsamples * 2;
                //Win32.AllocateBuffer(ref _WaveFormINTptr, (DMAtransferSizeBytes));
                _WaveFormINTptr = Marshal.AllocHGlobal(DMAtransferSizeBytes);

                using (StreamReader sr = new StreamReader(filename))
                {
                    for (int i = 0; i < _uiNumDACsamples; i++)
                    {
                        // Read the stream to a string, and write the string to the console.
                        String line = sr.ReadLine(); // one line per 16-bit DAC sample
                        Waveform16bitSamplesArray[i] = UInt16.Parse(line);
                                //    move 'i' index by 2 bytes each write
                        System.Runtime.InteropServices.Marshal.WriteInt16(_WaveFormINTptr, i*2, (Int16)Waveform16bitSamplesArray[i]);
                    }
                }
            }
            catch (IOException e)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ThorDAQ PCIe adapter not found");  // report error to log file
                retStr = "Failed reading DAC wave file " + filename + " Error: " + e.Message;
                return retStr;
            }
            // DAC waveform successfully loaded - now setup DMA
            // first, setup host memory side - our virtual memory pointer to MDL (Memory Descriptor List)
            // for SOURCE ("gather") DMA side
            
            //for (int i = 0; i < _uiNumDACsamples; i++)
            //{
             //   System.Runtime.InteropServices.Marshal.WriteInt16(WaveFormINTptr, i, (Int16)Waveform16bitSamplesArray[i]);
           // }
//            uiStatus = TDPacketWriteBuffer(_tdBoardNum, _cardDDRmemStart, (uint)DMAtransferSizeBytes, _WaveFormINTptr, 0xffff);

            // second, setup FPGA-card memory side - WafePlay DMA descriptor linked list of 64k segments
            // for DESTINATION ("scatter") DMA side
            

            // (fixed arrays require special access method...)
            unsafe
            {
                fixed (uint* gainStart = _PacketGenChk.imageCtrl.ADCGain)
                {
                    uint *g1 = gainStart;
                    *g1++ = 0x1;  // ADC gain channel 1
                    *g1++ = 0x2;  //          channel 2
                    *g1++ = 0x3;  //
                    *g1++ = 0x4;  //          channel 3
                }
            }
            
            // C# (.NET) cannot "fix" the array of non-basic types for managed/non-managed Marshalling
            // so use bitmask
            if( (DACchannel & 0x1) !=0)
            {
                // populate minumum fields for DAC waveform
                // set minimum params necessary just to start DAC waveform...

                _PacketGenChk.imageCtrl.channel = 1; // ADC channel, 1 based?
                _PacketGenChk.imageCtrl.frameCnt = 0x7fffffff;

                _PacketGenChk.imageCtrl.imgHSize = 0x200; // e.g. 512
                _PacketGenChk.imageCtrl.imgVSize = 0x200; // e.g. 512
                _PacketGenChk.imageCtrl.numPlanes = 1;
                _PacketGenChk.imageCtrl.frameNumPerSec = 1;
                _PacketGenChk.imageCtrl.frameNumPerTransfer = 1;

                _PacketGenChk.imageCtrl.clockRate = 160000000;
                _PacketGenChk.imageCtrl.clock_source = 1;
                _PacketGenChk.imageCtrl.triggerMode = 0;  // i.e. 2 is SW_FREE_RUN_MODE (see RadioButtons) - deliberately cause error

                // "SetScanSettings()"
                _PacketGenChk.imageCtrl.system_mode = 1; // force to galvo galvo

                _PacketGenChk.galvoGalvoCtrl.dwellTime = 2.275e-6;
                _PacketGenChk.galvoGalvoCtrl.pixelDelayCnt = 1;
                _PacketGenChk.galvoGalvoCtrl.turnaroundTime = .00200985;
                _PacketGenChk.galvoGalvoCtrl.flybackTime = 0.00643618;
                _PacketGenChk.galvoGalvoCtrl.lineTime = 0.001609;
               
                _PacketGenChk.dacCtrl2.output_port = 2;
                _PacketGenChk.dacCtrl2.offset_val = -8.11475;
                _PacketGenChk.dacCtrl2.park_val = -9.256;
                _PacketGenChk.dacCtrl2.update_rate = 1000000; // 1MHz;
                _PacketGenChk.dacCtrl2.amplitude = 0.0;
                _PacketGenChk.dacCtrl2.waveform_buffer_start_address = _cardDDRmemStart;
                _PacketGenChk.dacCtrl2.waveform_buffer_length = (ulong)DMAtransferSizeBytes;
                _PacketGenChk.dacCtrl2.flyback_samples = 0x1203;
            }
            // call SetDMAdescriptors to scatter 16-bit waveform sample array
            // into 64k segments in ThorDAQs DDR3 memory
            // This does not move the data, but rather completes DMA config
            /*try
            {
                uiStatus = TDSetImagingConfiguration(_tdBoardNum,  ref _PacketGenChk); // includes SetDMADescriptors()
            }
            catch (Exception e)
            {
                retStr = "TDSetImagingConfiguration() error: " + e.Message;
                return retStr;
            }
            if (uiStatus != Win32.STATUS_SUCCESSFUL)
            {
                retStr = "TDSetImagingConfiguration() error: " + uiStatus;
                return retStr;
            }*/

            return retStr;
        }


        ~tdDACwaveform()
        {
            // all done
            if (_WaveFormINTptr != IntPtr.Zero)
                Marshal.FreeHGlobal(_WaveFormINTptr);
            if (_hDACstructures != IntPtr.Zero)
                Marshal.FreeHGlobal(_hDACstructures);
            uint uiStatus = ThorDAQAPIReleaseBoard(_tdBoardNum);
            if (uiStatus != 0)
            {
                return;
            }
            return;
        }
    }

}