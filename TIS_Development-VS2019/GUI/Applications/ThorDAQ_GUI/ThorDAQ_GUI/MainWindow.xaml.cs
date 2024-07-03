namespace thordaqGUI
{
    using System;
    using System.Data;
    using System.Data.Common;
    using System.Runtime.InteropServices; // DllImport
                                          //  using NationalInstruments;
                                          //  using NationalInstruments.Controls.Data;
                                          //  using NationalInstruments.Controls.Rendering;

    using System.Deployment.Application;
    using System.Management;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Xml;

    using ThorLogging;
    using ThorSharedTypes;  // e.g. generic ICamera, and also includes ThorDAQ specifics

    using System.Diagnostics;

    using TIDP.SAA;
    using NWLogic.DeviceLib;
    using System.Net;
    using System.Net.Sockets;
    using tdDFT;
    using Microsoft.SqlServer.Server;

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIWaitForUserIRQ")]
        // Wait blocks forever waiting for kernel driver to receive FPGA User interrupt, or a Cancel via "timeout"
        // Returns FPGA interrupt count per S2MM channel
        public extern static THORDAQ_STATUS ThorDAQAPIWaitForUserIRQ(UInt32 board, ref UInt64 Chan0_IRQcnt, ref UInt64 Chan1_IRQcnt, ref UInt64 Chan2_IRQcnt, ref UInt64 Chan3_IRQcnt );
        public UInt64[] FPGAUserInterruptCountByChan = new ulong[] { 0, 0, 0, 0 };

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPICancelWaitForUserIRQ")]
        public extern static THORDAQ_STATUS ThorDAQAPICancelWaitForUserIRQ(UInt32 board); // cancels the infinite wait for FPGA interrupt by completing IOCTL with Timeout

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIGetDDR3status")]
        public extern static THORDAQ_STATUS ThorDAQAPIGetDDR3status(UInt32 board, IntPtr DDR3statusString, Int32 TotalRecordSize); // DDR3status fields in SharedTypes.cs

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIGetBOBstatus")]
        public extern static THORDAQ_STATUS ThorDAQAPIGetBOBstatus(UInt32 board, IntPtr BOBstatusString, Int32 TotalRecordSize); // BOBstatus fields in SharedTypes.cs
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPISetAIOConfig")]
        public extern static THORDAQ_STATUS ThorDAQAPISetAIOConfig(UInt32 board, IntPtr DIOconfig, Int32 FieldSize); // DIOconfig is single DIO definition - send our field size

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIGetDIOConfig")]
        public extern static THORDAQ_STATUS ThorDAQAPIGetDIOConfig(UInt32 board, IntPtr DIOconfig, Int32 RecordsSize); // DIOconfig is entire DLL char array, Max DIOs * FieldSize

        // CHAR config[32][12]  e.g. "DnnXmmMccAgg", where "n" is 0-31 (BNC label D0 through "D31", "X" is Input/Output direction "I" or "O",
        // "mm" is Index of Output source, "cc" is FPGA/CPLD MUX code, "gg" is FPGA GPIO MUX index (0-4 valid)
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPISetDIOConfig")]
        public extern static THORDAQ_STATUS ThorDAQAPISetDIOConfig(UInt32 board, IntPtr DIOconfig, Int32 FieldSize); // DIOconfig is single DIO definition - send our field size

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIGetDIO")]  // gets Input or last Output setting, if DIOspec is Output
        public extern static THORDAQ_STATUS ThorDAQAPIGetDIO(UInt32 board, IntPtr DIOconfig, UInt32 FieldSize, UInt32 NumDIOs); // DIOspec (value in AUX field), DIOspec field size, Number of DIOs

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPISetDO")]
        public extern static THORDAQ_STATUS ThorDAQAPISetDO(UInt32 board, IntPtr DIOconfigs, UInt32 FieldSize, UInt32 NumDIOs); // DIOspec (value in AUX field), DIOspec field size, Number of DIOs

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIGetAI")]
        public extern static uint ThorDAQAPIGetAI(UInt32 board, Int32 BNCindex, bool bVolts, ref double VoltsOrADCcounts);
        // boardNum,                  // Board to target
        // BNCindex,                  // BNC index (e.g. 6 for "AI6) on BBox for FPGA reg (legacy) or MAX127 channel
        // BiPolar,                   // True (1) if Bipolar, 0 for unipolar
        // ADCcounts                  // raw 12-bit count received from MAX127 chip

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIReleaseBoard")]
        public extern static uint ThorDAQAPIReleaseBoard(UInt32 board);

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIProgressiveScan")]
        public extern static uint ThorDAQAPIProgressiveScan(UInt32 board, bool bProgressiveScan);

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIXI2CReadWrite")]
        public extern static uint ThorDAQAPIXI2CReadWrite(UInt32 board,
            bool bI2C_ReadDir,
            UInt32 MasterMUXAddr, UInt32 MasterMUXChan,
            Int32 SlaveMUXAddr, UInt32 SlaveMUXChan,    // Slave Addr -1 means target on Master MUX bus
            UInt32 TargetAddr, UInt32 I2CbusHz, Int32 PageSize,
            IntPtr OpcodeBuffer, ref UInt32 OpcodeLen,
            IntPtr DataBuffer, ref UInt32 DataLen); // DataLen returns total Opcode+Data bytes written


        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIFlashI2CSlave")]
        public extern static uint ThorDAQAPIFlashI2CSlave(UInt32 board, UInt32 MasterChan, Int32 SlaveChan, UInt32 DevAddr, UInt32 SlaveDevCmdByte, IntPtr Buffer, ref UInt32 BufByteLen);

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIBreakOutBoxLED")]
        public extern static uint ThorDAQAPIBreakOutBoxLED(UInt32 board, Int32 LEDenum, Byte State);  // 0 off, 1 on, 2 blink

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIFPGAregisterWrite")]
        public extern static uint ThorDAQAPIFPGAregisterWrite(UInt32 board,
            IntPtr pRegisterFieldName,
            Int32 FldNameSize, // in bytes
            IntPtr Value);     // DLL/driver Register/Field (unManagedMem) value to write

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIFPGAregisterRead")]
        public extern static uint ThorDAQAPIFPGAregisterRead(UInt32 board,
            IntPtr pRegisterFieldName,
            Int32 FldNameSize, // in bytes
            IntPtr pValue);    // DLL/driver returned value of REGISTER or FIELD    

        // these constants are indexes to the names of Labview .VI programs
        // that are run on the NI1071, in coordination with test logic in ManufTest
        public enum NI1071_LabviewVItestNums
        {
            DACwTest = 3,
            ADCwTest = 4,
            ABBxTest = 5,
            DBB1outputTest = 6,   // NI 1071 test Digital Outputs
            DBB1inputTest = 7 // Input 
        };


        #region TexasInstruments SAA event handlers
        void Adapters_USBReattached(TIBusAdapters.USBReattachedEventArgs e)
        {
            string sMsg = "ThorDAQcl-GUI: TI USBAdapter Reattached: Adapter_Number= " + +e.Adapter_Number + "; Message=" + e.Message;
            UpdateConsoleStatus(sMsg);
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, sMsg);
        }

        void Adapters_USBDetached(TIBusAdapters.USBDetachedEventArgs e)
        {
            string sMsg = "ThorDAQcl-GUI: TI USBAdapter Detached: Adapter_Number=" + e.Adapter_Number + "; Message=" + e.Message;
            UpdateConsoleStatus(sMsg);
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, sMsg);
        }

        void Adapters_SMBusAlertLineUpdated(TIBusAdapters.SMBusAlertLineUpdatedEventArgs e)
        {
            string sMsg = "ThorDAQcl-GUI: TI USBAdapter SMBusAlertLineUpdated: Adapter_Number=" + e.Adapter_Number + "; Message=" + e.Message;
            UpdateConsoleStatus(sMsg);
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, sMsg);
        }

        void Adapters_ControlLineUpdated(TIBusAdapters.ControlLineUpdatedEventArgs e)
        {
            string sMsg = "ThorDAQcl-GUI: TI USBAdapter ControlLineUpdated: Adapter_Number=" + e.Adapter_Number + "; Message=" + e.Message;
            UpdateConsoleStatus(sMsg);
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, sMsg);
        }

        void Adapters_Warning(TIBusAdapters.WarningEventArgs e)
        {
            string sMsg = "ThorDAQcl-GUI: TI USBAdapter Warning: Adapter_Number=" + e.Adapter_Number + "; Message=" + e.Message + "; Warning_Type=" + e.Warning_Type;
            UpdateConsoleStatus(sMsg);
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, sMsg);
        }

        void Adapters_RequestComplete(TIBusAdapters.RequestCompleteEventArgs e)
        {
            // string sMsg = "ThorDAQcl-GUI: TI USBAdapter RequestComplete: Adapter_Number=" + e.Adapter_Number + "; Message=" + e.Message + "; Is_Success=" + e.Is_Success;
            //UpdateConsoleStatus(sMsg);
            //ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, sMsg);
        }
        #endregion

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


        public class XilinxI2CXfer
        {
            public UInt32 I2CbusHz;
            public XilinxI2CXfer() { I2CbusHz = 100; }

            public THORDAQ_STATUS I2CTransfer(uint MasterMUXChan, uint SlaveMUXChan, uint TargetSlaveAddr,
                bool bI2C_SlaveRead, int PageSize, byte[] OpcodeBytes, uint OpCodeLen, byte[] DataBytes, uint DataLen, out uint I2CtransferLen)
            {
                THORDAQ_STATUS status = 0;
                IntPtr UnManagedOpCodeBuffer = Marshal.AllocHGlobal((int)OpCodeLen);
                IntPtr UnManagedDataBuffer = Marshal.AllocHGlobal(PageSize);
                // Hardcoded I2C slave MUXes
                for (int ocIndx = 0; ocIndx < OpcodeBytes.Length; ocIndx++) // write OpCodes to UnManaged Mem
                {
                    System.Runtime.InteropServices.Marshal.WriteByte(UnManagedOpCodeBuffer, ocIndx, OpcodeBytes[ocIndx]);
                }
                if (!bI2C_SlaveRead) // when writing, copy data bytes to DLL
                {
                    for (int dIndx = 0; dIndx < DataLen; dIndx++) // write DATA bytes to UnManaged Mem
                    {
                        // e.g., start of TD main board Data buffer to send, 0x54 0x44 0x30 ... ("TD0...")
                        System.Runtime.InteropServices.Marshal.WriteByte(UnManagedDataBuffer, (int)(dIndx), (byte)DataBytes[dIndx]);
                    }
                }
                I2CtransferLen = DataLen;
                status = (THORDAQ_STATUS)ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead,
                    TDMasterI2CMUXAddr, MasterMUXChan,
                    TDSlaveI2CMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, PageSize,
                    UnManagedOpCodeBuffer, ref OpCodeLen, // "OpCodeLen" IN: Opcode byte count (in buffer)
                    UnManagedDataBuffer, ref I2CtransferLen); // IN: "DataLen" of expected READ bytes,  OUT: ALL Opcodes + Data bytes transfered


                // on READ, return the data to caller.  DataLen was the read count passed to I2C master
                if (bI2C_SlaveRead == true)
                {
                    for (int CPPbyteIndx = 0; CPPbyteIndx < DataLen; CPPbyteIndx++)
                    {
                        DataBytes[CPPbyteIndx] = Marshal.ReadByte(UnManagedDataBuffer, CPPbyteIndx);
                    }

                }
                Marshal.FreeHGlobal(UnManagedOpCodeBuffer);
                Marshal.FreeHGlobal(UnManagedDataBuffer);
                return status;
            }
        }


        private TIBusAdapters TIusbAdapters = new TIBusAdapters();

        public tdDFT.DiscreteFourierTrans dft;
        enum TDexitCode : int
        {
            Success = 0,
            NoBoardDetected = 1,
            UnknownError = 10
        };

        #region Fields
        enum EEPromCMDs
        {
            MB_SER,
            ADC_SER,
            DAC_SER,
            CPLD_VER,
            BBA1_SER,
            BBA2_SER,
            BBA3_SER,
            BBD1_SER,
            INV_CMD,
        };


        public tdDACwaveform tdWaveForm = new tdDACwaveform();

        //      public  TDcomponent MainBrd = new TDcomponent("MainPCB");
        //        public TDassembly ThorDAQadapter = new TDassembly(0, "ThorDAQ-2586");
        // public ArrayList 
        public static TDcontroller[] ThorDAQAdapters = new TDcontroller[4];
        //        public TDassembly[] ThorDAQAdapters = new TDassembly[4];
        //public static TDassembly ThorDAQadapter;

        public const string sEEPROMFileSpec = @"c:\temp\ThorDAQeeproms.txt";

        public static readonly string[] THORDAQ_COMMANDLINE_COMMAND =
        {
            "Exit",
            "SelectBoard",
            "ADCSampleClock_setup",
            "ADCStream_setup",
            "ScanControl_setup",
            "BuildWaveform",
            "ImageScanTest",
            "ReadMem",
            "WriteMem",
            "GetBoardCfg",
            "GetAllFPGAregisters",
            "GetFPGAreg",
            "SetFPGAreg",
            "APIBreakoutBoxLED",
            "APIDDR3MemTest",
            "APIGetAI",
            "APIGetAIOConfig",
            "APIGetBOBstatus",
            "APIGetDDR3status",
            "APIGetDIOConfig",
            "APISetAIOConfig",
            "APISetDIOConfig",
            "APISetDO",
            "APIGetLFT_JAstatus",
            "APISetLFT_JAconfig",
            "APIReadEEPROM",
            "APIWriteEEPROM",
            "APIReadProdSN",
            "APIWriteProdSN",
            "APIUpdateLFT_AppFirmware",
            "APIUpdateBOB_CPLD",
            "APIUpdateDAC_CPLD",
            "APIUpdateLFT_CPLD",
            "DFT",
            "UpdateCentralFPGAfirmware",
            "Load",
            "DACWaveform_setup",
            "PacketRead",
            "GlobalScan",
            "KDstressTest",
            "ManufTest",
            "ReadDRam",
            "Clear",
            "LoadScript",
            "LiveCapture",
            "StopCapture",
            "SetParkValue",
            "Sleep",
            "S2MM_DMA_setup",
//            "WriteDram",
            "WriteSPI",
            "Delay",
            "XI2Cread",
            "XI2Cwrite"
        };

        public enum ScanHeadType
        {
            Galvo_Galvo,
            Res_Galvo,
            Res_Galvo_Galvo,
        };
        /// <summary>Instance number for this board. </summary>
        public UInt16 devInstance; // a.k.a boardIndx (0-based)
        static public UInt16 MasterBoardIndex = 0xFEED;
        static public ScanHeadType ScanHeadMode = ScanHeadType.Res_Galvo; // i.e. Galvo-Galvo, Res-Galvo, Res-Galvo-Galvo
        static public UInt16 NumThorDAQboards;


        const string HELP_MSSAGE = @"ThorDAQ commands:
  Help | <Command Below> | -?  (Many commands have extensive help documnation, simple commands don't)
  Exit
  ADCSampleClock_setup
  ADCStream_setup
  ScanControl_setup
  S2MM_DMA_setup 
  BuildWaveform
  DACWaveform_setup 
  GlobalScan 
  ImageScanTest -d WxH         Default RG image test, calls ADCSample+Stream+Scan+S2mmDMA+DACWave, to DDR3
  ReadDDR3 <DDR3_Address> <bytecount>
  ReadMem <BarNum> <Card Offset> <Length>
  LoadScript <-f filename>
  GetBoardCfg
  GetAllFPGAregisters <-f [filename]>
  GetFPGAreg <Name> 
  SetFPGAreg <Name> <value>
  APIBreakoutBoxLED <-Label|-all> <on|off|blink> 
  APIDDR3MemTest
  APIGetAI  
  APIReadEEPROM [-f]                        (option -f: overwrite  c:\temp\ThorDAQeeproms.txt file)
  APIWriteEEPROM                            writes contents of c:\temp\ThorDAQeeproms.txt file
  APIReadProdSN [-f]                        (option -f: overwrite  c:\temp\TD_ProdSN.txt file)
  APIWriteProdSN                            writes contents of c:\temp\TD_ProdSN.txt file
  APIUpdateDAC_CPLD                         RePrograms DAC CPLD
  APIUpdateBOB_CPLD                         RePrograms 3U BreakOutBox CPLD
  APIUpdateLFT_AppFirmware [-f {filespec}]  Update LFT (e.g. 3P) mezz. card MCU F/W Application
  APIUpdateLFT_CPLD                         RePrograms LFT's (Low Freq. Trigger, e.g. 3P) CPLD
  APIGetDIO                                 Get digital input(s) or last set output(s) (3U or DBB1 BOB)
  APIGetBOBstatus                           Full Information on BreakOutBox type & connection status
  APIGetDDR3status                          Info on DDR3 image memory card on ThorDAQ board
  APIGetDIOConfig                           Get current DIO configuration MUX
  APIGetLFT_JAstatus                        Get Version & snapshot of current JA config, LFT thresholds, freq, etc.
  APISetAIOConfig                           Set 'slow' ADC range/polarity, etc.  
  APISetDIOConfig                           Set input/output direction and output source
  APISetDO                                  Set digital output(s) (3U or DBB1 BOB)
  APISetLFT_JAconfig                        Sets config for JA (Jitter Attenuator)
  KDstressTest                               Kernel Driver stress test, esp. for new MotherBoard
  ManufTest Thorlabs manufacturing test (requires custom test bench)
  ReadMem
  UpdateCentralFPGAfirmware [-g] [-f {filespec}] (options: Golden flash image area, filespec)
  SelectBoard BoardIndx (0-based board index, max 4 boards)
  SetParkValue [-c Channel -v ParkVoltage]   Channel 0 - 11, Volts -10.0 to +10.0
  WriteDDR3 <DDR3_Address> <bytecount> <data_byte [data_byte ...] >   writes 'bytecount' bytes for one 'data_byte'
  WriteMem
  WriteSPI <index> <address> <data> %index: 0 -> clock synthsizer; 1-3 -> adc converter
  XI2Cwrite
  XI2Cread
  DFT  (Discrete Fourier Transform [eng. test])

ThorDAQ hotkeys:
  Tab: Quick find command
  Up/Down: Quick find latest command
  Ctrl+C: Quick stop threads
  Note: Data is written and data read is displayed from low to high address order";

        //        readonly int MAX_BAR_INDEX = 3;
        //        readonly int MIN_BAR_INDEX = 0;

        ObservableCollection<string> consoleOutput = new ObservableCollection<string>() { "<<< ThorDAQ CL-GUI Console  \n<<< Copyright 2016-2021 Thorlabs Imaging Systems Research Group" };
        bool[] _lsmChannelEnable = new bool[4];
        double[] _sliderADCgain = new double[4];
        double _frameRate = 1.0;
        bool _bLiveAcqFlag = false; // StartStop button logic
        bool _bNewBitMapRequired = true;
        bool _acquisitionActiveFlag = true;
        ObservableCollection<string> _commandOutput = new ObservableCollection<string>();
        private int _last_recorded_command = 0;
        private string _last_recorded_tabbed_command = string.Empty;
        private int _last_recorded_tabbed_command_index = 0;

        #endregion Fields

        #region Constructors

        public MainWindow()
        {
            InitializeComponent();
            DataContext = this;
            this.Loaded += MainWindow_Loaded;
            this.Closed += MainWindow_Closed;
            InputBlock.PreviewKeyDown += InputBlock_PreviewKeyDown;
            InputBlock.Focus();
            LSMChannelEnable0 = true; // channel A enabled at start

        }

        #endregion Constructors

        #region Events

        // Declare the event
        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public ObservableCollection<string> ConsoleOutput
        {
            get
            {
                return consoleOutput;
            }
            set
            {
                consoleOutput = value;
                OnPropertyChanged("ConsoleOutput");
            }
        }

        public String SettingPath
        {
            get; set;
        }

        #endregion Properties

        #region Methods

        public void ClearConsole()
        {
            string[] beginString = new string[3];
            if (consoleOutput.Count >= 3)
            {
                for (int i = 0; i < 3; i++)
                {
                    beginString[i] = ConsoleOutput[i];
                }
            }
            ConsoleOutput.Clear();
            for (int i = 0; i < 3; i++)
            {
                ConsoleOutput.Add(beginString[i]);
            }
            _last_recorded_command = 0;
            _last_recorded_tabbed_command = string.Empty;
            _last_recorded_tabbed_command_index = 0;
        }

        //Get Board configuration
        public void GetBoardConfiguration(int boardIndex) // 0-based index of referenced board
        {
            BOARD_INFO_STRUCT BoardConfig = new BOARD_INFO_STRUCT();
            try
            {
                if (Win32.GetBoardCfg(devInstance, ref BoardConfig) == Win32.STATUS_SUCCESSFUL)
                {
                    UpdateConsoleStatus("Board Configuration Information:");
                    string FPGAver = "FPGA (booted) Version = 0x" + BoardConfig.UserVersion.ToString("X4");
                    UpdateConsoleStatus(FPGAver);
                    ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, FPGAver);
                    // mezzanine board config?

                }
                else
                {
                    UpdateConsoleStatus("Get Board Configuration Error");
                }
            }
            catch (Exception)
            {
                throw;
            }
        }


        public async Task AutomatedCmdsWithDelays(int Delay, string NextCommand)
        {
            Task<int> runPreviousCommandDuration = RunAsyncDelay(Delay);

            int result = await runPreviousCommandDuration;

            UpdateConsoleStatus(NextCommand);
            UpdateConsoleCommand(NextCommand);
        }

        public async Task<int> RunAsyncDelay(int Delay)
        {
            await Task.Delay(Delay); //milliseconds delay
            return 1;
        }

        public void SleepMainThread(List<String> argumentsList)
        {
            int SleepInMS = 10;
            if (argumentsList.Count > 1) // 
            {
                SleepInMS = Int32.Parse(argumentsList[1]);
            }

            Task runTask = AutomatedCmdsWithDelays(5000, "stopcapture");

            //            System.Threading.Thread.Sleep(SleepInMS);
        }

        // args  <channel(s)>  <Width_Height> <FrameRate>
        //        0x1  256_256 2 
        public void LiveCapture(List<String> argumentsList)
        {
            // set defaults
            string channelMask = "0x1";
            BitmapWidth = BitmapHeight = 512;

            if (_bw.IsBusy)
            {
                UpdateConsoleStatus("LiveCapture is Busy - exit");
                return;
            }
            if (_acquisitionActiveFlag == false)
            {
                _acquisitionActiveFlag = true;
            }
            if (argumentsList.Count > 1) // 
            {
                channelMask = argumentsList[1];
            }
            if (argumentsList.Count > 2) // image dimensions
            {
                //_liveCaptureWindow.DataContext = this;
                string[] image_params_args_array = argumentsList[2].Split('_');
                BitmapWidth = Convert.ToUInt16(image_params_args_array[0]);
                BitmapHeight = Convert.ToUInt16(image_params_args_array[1]);
            }

            //            Bitmap = new WriteableBitmap(BitmapWidth, BitmapHeight, 96, 96, PixelFormats.Gray8, null);

            // arg[3] should be frame rate
            //           argumentsList.Add("4294967295 ");

            //ReadAddressablePacket(argumentsList); // old method of thread-based application NOT based on "CopyAcquisition"
            StartStopACQ_Click(null, null);
        }

        public void LoadScript(List<String> argumentsList)
        {
            // if filename provided, simply use it
            // -f  filename
            if (argumentsList.Count > 1 && argumentsList[1] == "-f")
            {
                LoadScript(argumentsList[2]);
                return;
            }

            //Create OpenFileDialog
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.DefaultExt = ".txt";
            dlg.Filter = "txt Files (.txt)|*.txt";

            // Display OpenFileDialog by calling ShowDialog method
            Nullable<bool> result = dlg.ShowDialog();
            if (result == true)
            {
                LoadScript(dlg.FileName);
            }
        }

        // first arg in register/field name to read
        // arg1 - Name
        // arg2 - Value
        public void SetFPGAreg(List<String> argumentsList)
        {
            string RegFldName = argumentsList[1];
            // value - may be in HEX
            UInt64 Value;
            if (argumentsList[2].Contains("0x"))
            {
                Value = ulong.Parse(argumentsList[2].Replace("0x", string.Empty), NumberStyles.HexNumber);
            }
            else
            {
                Value = ulong.Parse(argumentsList[2]);
            }
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(RegFldName, Value);
        }
        // first arg in register/field name to read
        // arg1 - Name
        public void GetFPGAreg(List<String> argumentsList)
        {
            string RegFldName = argumentsList[1];
            UInt64 Value = 0;

            bool bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read(RegFldName, ref Value);
            if (bStatus != true)
                UpdateConsoleStatus("Invalid Register/Field: " + RegFldName);
            else
                UpdateConsoleStatus(string.Format("{0}: {1} (0x{2})", RegFldName, Value, Value.ToString("X")));

        }

        // query the API DLL for all Register and Field definitions for the hardware board
        public void GetAllFPGARegisters(List<String> argumentsList)
        {
            REG_FIELD_DESC RegFldDesc = new REG_FIELD_DESC();
            uint uiStatus;
            uint TotalVariableCount = 0;
            string RegFldName = "";
            bool bCreateFile = false;
            string fileSpec = "c:\\temp\\FPGAregs.txt";
            var fileFormatStrings = new List<string>();

            // optional command line args?

            if (argumentsList.Count > 1)
            {
                if (argumentsList[1] == "-f")
                {
                    // save the Register/Field list to file
                    bCreateFile = true;
                    if (argumentsList.Count > 2)
                        fileSpec = argumentsList[2];
                }
            }
            for (int r = 0; r < 256; r++) // arbitrarily high upper bound
            {
                // first get the register
                uiStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Query(r, -1, ref RegFldName, 48, ref RegFldDesc);
                if (uiStatus != 0)
                {
                    break; // done
                }
                // we have a REGISTER
                TotalVariableCount++;
                string str = string.Format("Reg: {0}({0:x} h) Name: {1,-34} --> BAR: {2} BARoffset: 0x{3} Register Byte Len: {4}, WO:{5} RO:{6}",
                                                    r, RegFldName, RegFldDesc.BAR, RegFldDesc.BARoffset.ToString("X3"), RegFldDesc.RegisterByteSize,
                                                    RegFldDesc.bWriteOnly.ToString(), RegFldDesc.bReadOnly.ToString());
                UpdateConsoleStatus(str, 1);
                if (bCreateFile) fileFormatStrings.Add(str);

                for (int f = 0; f < 64; f++)
                {
                    uiStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Query(r, f, ref RegFldName, 48, ref RegFldDesc);
                    if (uiStatus != 0)
                    {
                        break; // next Register
                    }
                    // we have a FIELD
                    TotalVariableCount++;
                    str = string.Format("  Fld: {0} Name: {1,-32} --> Bit Field Offset: {2} Bit Field Len: {3}", f, RegFldName, RegFldDesc.BitFieldOffset, RegFldDesc.BitFieldLen);
                    UpdateConsoleStatus(str, 1);
                    if (bCreateFile) fileFormatStrings.Add(str);
                }
            }
            UpdateConsoleStatus("Total Reg/Field variables: " + TotalVariableCount);
            if (bCreateFile == true)
            {
                File.WriteAllLines(fileSpec, fileFormatStrings);
                UpdateConsoleStatus("Register/Field descriptions written to file: " + fileSpec);
            }
        }

        // arg1  RegisterIndex
        // arg2  FieldIndex (or -1 if RegIndex only)
        // arg3  
        const string HELP_ReadMem_MSSAGE = @"ReadMem BAR BARoffset LengthInBytes 
Usage Example:
To read 16 zeros to BAR 1, BarOffset 0x20:
   readmem 1 0x20 16 "
;
        public void ReadBoardMemoryData(List<String> argumentsList)
        {
            IntPtr buffer = IntPtr.Zero;
            try
            {
                UInt16 value = (UInt16)Convert.ToInt32(argumentsList[1]);
                {
                    UInt32 barNum = value; // get bar number
                    int registerAddress = (int)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[2]);
                    int transferDataLength = (int)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[3]);
                    if (buffer == IntPtr.Zero)
                    {
                        Win32.AllocateBuffer(ref buffer, (uint)transferDataLength);
                    }
                    string sStatus = ThorDAQAdapters[MasterBoardIndex].BAR.Read(barNum, (ulong)registerAddress, buffer, (ulong)transferDataLength);
                    if (sStatus == "OK")
                    {
                        Byte[] ManagedMem = new Byte[transferDataLength];
                        Marshal.Copy(buffer, ManagedMem, 0, transferDataLength);
                        foreach (string Str in format16ByteLinesOnConsole((ulong)registerAddress, ManagedMem, (uint)transferDataLength))
                        {
                            UpdateConsoleStatus(Str);
                        }
                    }
                    else
                        UpdateConsoleStatus(sStatus);

                    if (buffer != IntPtr.Zero)
                        Win32.FreeBuffer(buffer);
                }
            }
            catch (Exception e)
            {
                if (buffer != IntPtr.Zero)
                {
                    Win32.FreeBuffer(buffer);
                }
                UpdateConsoleStatus("Readmem Error: " + e.Message);
            }
        }


        public void UpdateConsoleCommand(String input)
        {
            List<String> argumentsList = input.Split(' ').ToList();

            if (argumentsList.Count > 0)
            {
                try
                {
                    switch (argumentsList[0].ToLower())
                    {
                        case "help":
                            {
                                if (argumentsList.Count > 1) // help <additional command>?
                                {
                                    if (argumentsList[1].ToLower() == "buildwaveform")
                                    {
                                        UpdateConsoleStatus(HELP_BuildWaveform_MSSAGE);
                                        break;
                                    }
                                    else if (argumentsList[1].ToLower() == "dacwaveform_setup")
                                    {
                                        UpdateConsoleStatus(HELP_DACWaveform_setup_MSSAGE);
                                        break;
                                    }
                                    else if (argumentsList[1].ToLower() == "globalscan")
                                    {
                                        UpdateConsoleStatus(HELP_GlobalScan_MSSAGE);
                                        break;
                                    }
                                    else if (argumentsList[1].ToLower() == "apigetdioconfig")
                                    {
                                        UpdateConsoleStatus(HELP_APIGetSetDIOConfig_MSSAGE);
                                        break;
                                    }
                                    else if (argumentsList[1].ToLower() == "apisetdioconfig")
                                    {
                                        UpdateConsoleStatus(HELP_APIGetSetDIOConfig_MSSAGE);
                                        break;
                                    }
                                    else if (argumentsList[1].ToLower() == "apisetaioconfig")
                                    {
                                        UpdateConsoleStatus(HELP_APIGetSetAIOConfig_MSSAGE);
                                        break;
                                    }

                                    else if (argumentsList[1].ToLower() == "apigetdio" || argumentsList[1].ToLower() == "apisetdo")
                                    {
                                        UpdateConsoleStatus(HELP_APIGetSetDIO_MSSAGE);
                                        break;
                                    }

                                    else if (argumentsList[1].ToLower() == "apigetai")
                                    {
                                        UpdateConsoleStatus(HELP_APIGetAI_MSSAGE);
                                    }

 
                                    else if (argumentsList[1].ToLower() == "manuftest")
                                    {
                                        UpdateConsoleStatus(HELP_ManufTest_MSSAGE);
                                        break;
                                    }

                                    else if (argumentsList[1].ToLower() == "s2mm_dma_setup")
                                    {
                                        UpdateConsoleStatus(HELP_S2MM_DMA_setup_MSSAGE);
                                        break;
                                    }
                                    else if (argumentsList[1].ToLower() == "adcsampleclock_setup")
                                    {
                                        UpdateConsoleStatus(HELP_ADCSampleClock_setup_MSSAGE);
                                        break;
                                    }
                                    else if (argumentsList[1].ToLower() == "adcstream_setup")
                                    {
                                        UpdateConsoleStatus(HELP_ADCStream_setup_MSSAGE);
                                        break;
                                    }
                                    else if (argumentsList[1].ToLower() == "scancontrol_setup")
                                    {
                                        UpdateConsoleStatus(HELP_ScanControl_setup_MSSAGE);
                                        break;
                                    }
                                    else if (argumentsList[1].ToLower() == "readmem")
                                    {
                                        UpdateConsoleStatus(HELP_ReadMem_MSSAGE);
                                        break;
                                    }
                                    else if (argumentsList[1].ToLower() == "writemem")
                                    {
                                        UpdateConsoleStatus(HELP_WriteMem_MSSAGE);
                                        break;
                                    }
                                    else if (argumentsList[1].ToLower() == "xi2cread")
                                    {
                                        UpdateConsoleStatus(HELP_XI2CReadWrite_MSSAGE);
                                        break;
                                    }
                                    else if (argumentsList[1].ToLower() == "xi2cwrite")
                                    {
                                        UpdateConsoleStatus(HELP_XI2CReadWrite_MSSAGE);
                                        break;
                                    }

                                    else if (argumentsList[1].ToLower() == "apiupdatelft_appfirmware")
                                    {
                                        UpdateConsoleStatus(HELP_UpdateLFT_AppFirmware_MSSAGE);
                                        break;
                                    }
                                    else if (argumentsList[1].ToLower() == "apigetlft_jastatus" || argumentsList[1].ToLower() == "apisetlft_jaconfig")
                                    {
                                        UpdateConsoleStatus(HELP_APIgetsetLFT_JA_MSSAGE);
                                    }
                                    else if (argumentsList[1].ToLower() == "apireadeeprom" || argumentsList[1].ToLower() == "apiwriteeeprom")
                                    {
                                        UpdateConsoleStatus(HELP_APIReadWriteEEPROM_MSSAGE);
                                    }
                                    else if (argumentsList[1].ToLower() == "kdstresstest")
                                    {
                                        UpdateConsoleStatus(HELP_KDstressTest_MSSAGE);

                                    }
                                }
                                else
                                    UpdateConsoleStatus(HELP_MSSAGE);
                            }
                            break;
                        case "-?":
                        case "?":
                            UpdateConsoleStatus(HELP_MSSAGE);
                            break;

                        case "exit": // if used in automated script, make sure we return error if no ThorDAQ was detected

                            if (NumThorDAQboards == 0) // no boards found
                                System.Environment.Exit((int)TDexitCode.NoBoardDetected);

                            System.Environment.Exit((int)TDexitCode.Success);
                            break;

                        case "selectboard":
                            string sNum = argumentsList[1].ToString();
                            UInt16 uiIndx;
                            bool bSuccess = UInt16.TryParse(sNum, out uiIndx);
                            if (bSuccess)
                            {
                                if (uiIndx < NumThorDAQboards)
                                {
                                    MasterBoardIndex = uiIndx;
                                    // briefly blink the 2 LEDs on PCIe card backplane
                                    //BlinkAdapterBackplaneLEDs(); // takes 1 second - two blinks of lights
                                }
                                else
                                    UpdateConsoleStatus("Zero-base Board index invalid - total boards " + NumThorDAQboards);

                            }
                            else
                                MasterBoardIndex = 0;
                            break;
                        case "getboardcfg":
                            {
                                GetBoardConfiguration(0);
                            };
                            break;

                        case "getfpgareg":
                            GetFPGAreg(argumentsList);
                            break;
                        case "setfpgareg":
                            SetFPGAreg(argumentsList);
                            break;
                        case "getallfpgaregisters":
                            GetAllFPGARegisters(argumentsList);
                            break;

                        case "writemem":
                            {
                                if (_bw.IsBusy)
                                {
                                    StopAcquisition();
                                }
                                WriteBoardMemoryData(argumentsList);
                            };
                            break;

                        case "xi2cread":
                            XI2Cread(argumentsList);
                            break;
                        case "xi2cwrite":
                            XI2Cwrite(argumentsList);
                            break;

                        case "readmem":
                            {
                                ReadBoardMemoryData(argumentsList);
                            };
                            break;
                        case "dft":
                            DFT(argumentsList);
                            break;

                  //      case "breakoutboxled":
                    //        ControlBBleds(argumentsList);
                      //      break;

                        case "apireadprodsn":
                            APIReadProdSN(argumentsList);
                            break;

                        case "apiwriteprodsn":
                            APIWriteProdSN(argumentsList);
                            break;

                        case "apireadeeprom":
                            APIReadEEprom(argumentsList);
                            break;
                        case "apiwriteeeprom":
                            APIWriteEEprom(argumentsList);
                            break;

                        case "apibreakoutboxled":
                            APIBreakOutBoxLEDControl(argumentsList);
                            break;
                        case "apigetddr3status":
                            APIGetDDR3status(argumentsList);
                            break;
                        case "apigetbobstatus":
                            foreach (string sRec in APIGetBOBstatus(argumentsList))
                            {
                                UpdateConsoleStatus(sRec);
                            }
                            break;

                        case "kdstresstest":
                            KDstressTest(argumentsList);
                            break;

                        case "apigetdioconfig":
                            APIGetDIOConfig(argumentsList);
                            break;
                        case "apiddr3memtest":
                            APIDDR3MemTest(argumentsList);
                            break;

                        case "apisetaioconfig":
                            APISetAIOConfig(argumentsList);
                            break;

                         case "apisetdioconfig":
                            foreach (string Str in APISetDIOConfig(argumentsList))
                            {
                                UpdateConsoleStatus(Str);
                            }
                            break;

                        case "imagescantest":
                            foreach (string Str in ImageScanTest(argumentsList))
                            {
                                UpdateConsoleStatus(Str);
                            }
                            break;

                        case "apisetdo":
                            APISetDO(argumentsList);
                            break;
                        case "apigetdio":
                            APIGetDIO(argumentsList);
                            break;

                        case "apigetai":
                            APIGetAI(argumentsList);
                            break;

                        case "apiupdatebob_cpld":
                            APIupdateBOB_CPLD(argumentsList);
                            break;

                        case "apiupdatelft_cpld":
                            APIupdateLFT_CPLD(argumentsList);
                            break;

                        case "apiupdatelft_appfirmware":
                            UpdateLFT_AppFirmware(argumentsList);
                            break;

                        case "apigetlft_jastatus":
                            foreach (string Str in APIgetLFT_JAstatus(argumentsList))
                            {
                                UpdateConsoleStatus(Str);
                            }
                            break;
                        case "apisetlft_jaconfig":
                            foreach (string Str in APIsetLFT_JAconfig(argumentsList))
                            {
                                UpdateConsoleStatus(Str);
                            }
                            break;

                        case "updatecentralfpgafirmware":
                            {
                                if (_bw.IsBusy)
                                {
                                    StopAcquisition();
                                }
                                UpdateCentralFPGAfirmware(argumentsList);
                            };
                            break;
                        case "apiupdatedac_cpld":
                            {
                                if (_bw.IsBusy)
                                {
                                    StopAcquisition();
                                }
                                APIUpdateDAC_CPLD();
                            };
                            break;
                        case "buildwaveform":
                            BuildWaveform(argumentsList);
                            break;


                        case "manuftest":
                            DateTime iterTestStart = DateTime.Now;  // meaure time lapse
                            ManufTest(argumentsList);
                            DateTime iterTestEnd = DateTime.Now; ;
                            System.TimeSpan TimeDiff = iterTestEnd - iterTestStart;
                            UpdateConsoleStatus("Elapsed Time: " + TimeDiff + " (Days.Hours:Minutes:Seconds)");
                            break;

                        // NOTE the motivation for making return type a "list" of strings is so
                        // we can call these functions from C# objects in Manfucturing test
                        // (i.e. we don't want to have to instantiate legacy CL-GUI console functions)
                        // there is no need for this with functions NOT used, e.g., in manuf-test classes
                        case "adcstream_setup":
                            foreach (string Str in ADCStream_setup(argumentsList))
                            {
                                UpdateConsoleStatus(Str);
                            }
                            break;
                        case "adcsampleclock_setup":
                            foreach (string Str in ADCSampleClock_setup(argumentsList))
                            {
                                UpdateConsoleStatus(Str);
                            }
                            break;
                        case "scancontrol_setup":
                            foreach (string Str in ScanControl_setup(argumentsList))
                            {
                                UpdateConsoleStatus(Str);
                            }
                            break;

                        case "s2mm_dma_setup":
                            foreach (string Str in S2MM_DMA_setup(argumentsList))
                            {
                                UpdateConsoleStatus(Str);
                            }
                            break;
                        case "dacwaveform_setup":
                            foreach (string Str in DACWaveform_setup(argumentsList))
                            {
                                UpdateConsoleStatus(Str);
                            }
                            break;
                        case "globalscan":
                            foreach (string Str in GlobalScan(argumentsList, null))
                            {
                                UpdateConsoleStatus(Str);
                            }
                            break;
                        //   case "load":
                        //       {
                        //           LoadScript(argumentsList);
                        //       };
                        //       break;
                        case "packetread":
                            {
                                if (_bw.IsBusy)
                                {
                                    StopAcquisition();
                                }
                                _isFileSavedEnabled = true;
                                ReadAddressablePacket(argumentsList);
                            };
                            break;
                        case "clear":
                            {
                                ClearConsole();
                            };
                            break;
                        case "loadscript":
                            LoadScript(argumentsList);
                            break;
                        case "sleep":
                            SleepMainThread(argumentsList);

                            break;

                        case "livecapture":
                            if (_bLiveAcqFlag == false) // only start if it's stopped
                            {
                                LiveCapture(argumentsList);
                            };
                            break;
                        case "stopcapture":
                            if (_bLiveAcqFlag == true) // only stop if it's running
                            {
                                StartStopACQ_Click(null, null);
                            };
                            break;
                        case "writeddr3":
                            {
                                WriteDDR3(argumentsList);
                            };
                            break;
                        case "readddr3":
                            byte[] NullByteRef = null;
                            {
                                foreach (string Str in ReadDDR3(argumentsList, ref NullByteRef))
                                {
                                    UpdateConsoleStatus(Str);
                                }
                            };
                            break;
                        case "readdram":
                            {
                                ReadDram(argumentsList);
                            };
                            break;
                        //                        case "writedram":
                        //                          {
                        //                                WriteDram(argumentsList);
                        //                            };
                        //                            break;
                        case "write_mezzanine_synth":
                            {
                                WriteMezzanineSynth(argumentsList);
                            }
                            break;

                        case "setparkvalue":
                            if (_bw.IsBusy)
                            {
                                StopAcquisition();
                            }
                            SetDAC_ParkValue(argumentsList);
                            break;

                        case "writespi":
                            {
                                if (_bw.IsBusy)
                                {
                                    StopAcquisition();
                                }
                                if (WriteSPIBus(argumentsList))
                                {
                                    UpdateConsoleStatus("Write SPI Bus Successfully");
                                }
                            };
                            break;
                        case "delay":
                            {
                                Delay(argumentsList);
                            }; break;
                        default:
                            break;
                    }
                }
                catch (Exception e)
                {
                    UpdateConsoleStatus(String.Format("Command Style Error: {0}", e.Message));
                }
            }
        }

        // output 
        public void UpdateConsoleStatus(String input, int option)
        {
            ConsoleOutput.Add(input);
            //_last_recorded_command = ConsoleOutput.Count - 1;
            Scroller.ScrollToBottom();
        }

        public void UpdateConsoleStatus(String input)
        {
            ConsoleOutput.Add("<<< " + input);
            //_last_recorded_command = ConsoleOutput.Count - 1;
            Scroller.ScrollToBottom();
        }



        const int SerStringDash0 = (int)(COMPONENT_SZ + SER_NUM_SZ);
        const int SerStringDash1 = (int)(SerStringDash0 + MANUF1);
        const int SerStringDash2 = (int)(SerStringDash1 + MANUF2);
        const int SerStringDash3 = (int)(SerStringDash2 + DATE_SZ);
        const int SerStringDash4 = (int)(SerStringDash3 + PART_NUM_SZ);
        const int SerStringDash5 = (int)(SerStringDash4 + AIX_SER_NUM_SZ);
        const int SerStringDash6 = (int)(SerStringDash5 + REMARKS);

        string FormatSerStr(string serStr, bool bSterlineFormat) // if TRUE use CCC-pppppppppp-ssssss  updated Sterling format
        {
            string serStrOut = string.Empty;

            //            UpdateConsoleStatus1("Length = " + serStr.Length);
            //        const uint TOT_SERIAL_SZ = COMPONENT_SZ + SER_NUM_SZ + MANUF1 + MANUF2 + DATE_SZ + PART_NUM_SZ + AIX_SER_NUM_SZ + REMARKS;
            if (bSterlineFormat == true)
            {
                if (serStr.Length >= COMPONENT_SZ)
                {
                    serStrOut = serStr.Substring(0, (int)COMPONENT_SZ) + "-";
                }
                if (serStr.Length >= SerStringDash3)
                {
                    serStrOut = serStrOut + serStr.Substring(SerStringDash3, (int)PART_NUM_SZ) + "-";
                }
                if (serStr.Length >= SerStringDash4)
                {
                    serStrOut = serStrOut + serStr.Substring(SerStringDash4, (int)AIX_SER_NUM_SZ);
                }

            }
            else
            {
                if (serStr.Length >= SerStringDash0)
                {
                    serStrOut = serStr.Substring(0, SerStringDash0) + "-";
                }

                if (serStr.Length >= SerStringDash1)
                {
                    serStrOut = serStrOut + serStr.Substring(SerStringDash0, (int)MANUF1) + "-";
                }
                if (serStr.Length >= SerStringDash2)
                {
                    serStrOut = serStrOut + serStr.Substring(SerStringDash1, (int)MANUF2) + "-";
                }
                if (serStr.Length >= SerStringDash3)
                {
                    serStrOut = serStrOut + serStr.Substring(SerStringDash2, (int)DATE_SZ) + "-";
                }
                if (serStr.Length >= SerStringDash4)
                {
                    serStrOut = serStrOut + serStr.Substring(SerStringDash3, (int)PART_NUM_SZ) + "-";
                }
                if (serStr.Length >= SerStringDash5)
                {
                    serStrOut = serStrOut + serStr.Substring(SerStringDash4, (int)AIX_SER_NUM_SZ) + "-";
                }
                if (serStr.Length >= SerStringDash6)
                {
                    serStrOut = serStrOut + serStr.Substring(SerStringDash5, (int)REMARKS); // END
                }
            }
            return serStrOut;
        }


        const string HELP_ADCStream_setup_MSSAGE = @"SC_setup: <-d WxH> <-n NumDescriptors> <-b bank> 
ADC Streaming setup for Filters, Coefficients, Lookup Table (LUT)
Options:
  -r  Resonant-Galvo mode (def. is Galvo-Galvo mode)
  -l  Low Frequency 3P Sampling (def. is 2P/Confocal ~80MHz ref)
  -j  Configure (by FPGA) and Release JESD204B cores
  -g  Gain (attenuation) 0 = 1dB, 1 = 5dB ... 7 = 29dB (def. 0)
";
        static List<String> ADCStream_setup(List<String> argumentsList)
        {
            List<String> ReturnStrings = new List<string> { };
            int arg = 1;
            bool bJESDreset = false;
            UInt64 uiGain = 0; // default

            UInt64 RegValue = 0;
            bool bStatus, b3Psampling = false, bResGalvo = false;
            int argCount = argumentsList.Count;
            while (arg < argCount)
            {
                switch (argumentsList[arg])
                {
                    case "-j":
                        bJESDreset = true;
                        break;

                    case "-l":
                        b3Psampling = true;
                        break;

                    case "-r":
                        bResGalvo = true;
                        break;

                    case "-g": // must have Int arg following
                        arg++; // get number
                        UInt64.TryParse(argumentsList[arg], out uiGain);
                        break;
                    default:
                        break;
                };
                arg++;
            }


            // Check for component reset flags...
            // looking at the Xilinx FAN side of board, LEDs on top -- 2 LEDs on far left are the JESD core indicators
            // they are turned of until the cores are "reset"/enabled
            // Non-reset (i.e. after hard power cycle) cores will deliver random 16-bit garbage through the 
            // ADC stream right into the 14-bit ADC image pixel samples
            if (bJESDreset == true)
            {
                // RESET the JESD cores 
                // sequence: JESD 0, delay JESD 1, delay, JESD 0 delay
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("JESD204B_config_core1", 0);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("JESD204B_config_core2", 0);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCFMCInterfaceControlReg", ref RegValue);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCFMCInterfaceControlReg", RegValue);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCFMCInterfaceControlReg", ref RegValue);
                ReturnStrings.Add(string.Format("ADCFMCInterfaceControlReg: 0x{0}", RegValue.ToString("X4")));
                Thread.Sleep(125); // ms
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("JESD204B_config_core1", 1);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("JESD204B_config_core1", ref RegValue);
                ReturnStrings.Add(string.Format("JESD204B_config_core1: {0}", RegValue.ToString()));
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("JESD204B_config_core2", 1);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("JESD204B_config_core2", ref RegValue);
                ReturnStrings.Add(string.Format("JESD204B_config_core2: {0}", RegValue.ToString()));
                // write entire register (to reset JESD cores)
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCFMCInterfaceControlReg", ref RegValue);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCFMCInterfaceControlReg", RegValue);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCFMCInterfaceControlReg", ref RegValue);
                ReturnStrings.Add(string.Format("ADCFMCInterfaceControlReg: 0x{0}", RegValue.ToString("X4")));
                Thread.Sleep(125); // ms
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("JESD204B_config_core1", 0);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("JESD204B_config_core2", 0);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCFMCInterfaceControlReg", ref RegValue);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCFMCInterfaceControlReg", RegValue);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCFMCInterfaceControlReg", ref RegValue);
                ReturnStrings.Add(string.Format("ADCFMCInterfaceControlReg: 0x{0}", RegValue.ToString("X4")));
                Thread.Sleep(125); // ms, copied from legacy code
            }


            if (bResGalvo == true)
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_SCAN_PER_SRCE", 1);
            else
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_SCAN_PER_SRCE", 0);

            // set the Gain(attentuation)
            string ADCgainChanx;
            for (int ch = 0; ch < 4; ch++)
            {
                ADCgainChanx = "ADCGainChan" + ch.ToString();
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(ADCgainChanx, uiGain);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read(ADCgainChanx, ref RegValue);
                ReturnStrings.Add(ADCgainChanx + RegValue.ToString());
            }

            // we require the Pre-filter DC offset for ThorDAQ PMTs (biased to negative voltage)
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_DCoffsetPreFIR", 1);
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStream_DCoffsetPreFIR", ref RegValue);
            ReturnStrings.Add(string.Format("    ADCStream_DCoffsetPreFIR: {0}", RegValue));

            // no Post-filter offset
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_DCoffsetPostFIR", 0);
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStream_DCoffsetPostFIR", ref RegValue);
            ReturnStrings.Add(string.Format("    ADCStream_DCoffsetPostFIR: {0}", RegValue));

            if (b3Psampling == true)
            {
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_LASER3P_MARKER_EN", 1);
                // must be computed according to 3P laser specifics
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStreamDownsampleReg", 1);
            }
            else // 2P / confocal
            {
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_LASER3P_MARKER_EN", 0);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_REVERB_MP_EN", 0);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStreamDownsampleReg", 0);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream3PReverbMPCyclesReg", 0);
            }
            // READ BACK our settings
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStream_SCAN_PER_SRCE", ref RegValue);
            ReturnStrings.Add(string.Format("    ADCStream_SCAN_PER_SRCE: {0}", RegValue));
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStream_LASER3P_MARKER_EN", ref RegValue);
            ReturnStrings.Add(string.Format("    ADCStream_LASER3P_MARKER_EN: {0}", RegValue));
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStream_REVERB_MP_EN", ref RegValue);
            ReturnStrings.Add(string.Format("    ADCStream_REVERB_MP_EN: {0}", RegValue));
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStream3PReverbMPCyclesReg", ref RegValue);
            ReturnStrings.Add(string.Format("    ADCStream3PReverbMPCyclesReg: {0}", RegValue));

            // New REVERB_MP_EN
            // NOTE: exist GG-DLL code sets 3P_Markers_RDivide even when 3P disabled
            //add 1% to the clock rate to have some margin when calculating the RDivideFactor
            double clockRate = 160000000 * 1.01 / 1000000; // assume clock at 160 MHz
            Byte rdivideFactor = 0;
            const Double RDivideLaserFreqLimit = 1.25; // MHz
            if (clockRate > RDivideLaserFreqLimit)
            {
                if ((clockRate / Math.Floor(clockRate)) <= RDivideLaserFreqLimit)
                {
                    rdivideFactor = (Byte)((Byte)Math.Floor(clockRate) - 1);
                }
                else
                {
                    rdivideFactor = (Byte)(Math.Floor(clockRate));
                }
            }

            // ADC Stream Processing
            // copy Carl's FIR logic from IOCTL
            // Two Filters, Pre and Post (p)
            // Four Channels
            // 16 coefficients per channel

            UInt64 c, f;
            UInt64 StreamControlReg = 0;

            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStreamControlReg", ref StreamControlReg);
            ushort[] PreFilter_FIR_coeff = new ushort[16] { 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512 };
            ushort[] PostFilter_FIR_coeff = new ushort[16] { 0, 0, 0, 819, 819, 819, 819, 819, 819, 819, 819, 819, 819, 0, 0, 0 };
            for (c = 0; c < 4; c++)
            {
                // turn on the AnalogFrontEnd (AFE) for the channel
                string AFEbit = "ADCAFEenChan" + c.ToString();
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(AFEbit, 0); // e.g. starts at 0x208

                UInt64 regValue = StreamControlReg | (c << 3);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStreamControlReg", regValue); // 0x1C0
                // Pre FIR coeffs
                for (f = 0; f < 16; f++)
                {
                    bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStreamFIRcoeffReg", PreFilter_FIR_coeff[f]); // 0x1E8
                }
                // Post FIR coeffs
                regValue = StreamControlReg | ((c + 4) << 3);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStreamControlReg", regValue); // 0x1C0
                for (f = 0; f < 16; f++)
                {
                    bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStreamFIRcoeffReg", PostFilter_FIR_coeff[f]); // 0x1E8
                }
            }
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCInterface3PMarkersRDivide", rdivideFactor); // 0x230
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCInterface3PMarkersRDivide", ref RegValue);
            ReturnStrings.Add(string.Format("    ADCInterface3PMarkersRDivide: {0}", RegValue));

            // Downsampling factor (0 for none)...
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStreamDownsampleReg", ref RegValue);
            ReturnStrings.Add(string.Format("    ADCStreamDownsampleReg: {0}", RegValue));

            ReturnStrings.Add("SUCCESS");
            return ReturnStrings;
        }


        const string HELP_ADCSampleClock_setup_MSSAGE = @"ADCSampleClock_setup:  
ADC (PMT) Sampling Clock Generation setup for 3P (low frequency) or 2P/Confocal (~80 MHz freq)
and PLL/MMCM signal selection (ENABLE/DISABLE) if using external LASER source.  Otherwise,
use internally generated (Confocal) clock source.
Options:
  -c  Confocal (Internal) Synchronized Sampling @160MHz from internal 80MHz osc. (def)
  -2  Use 2P LASER Sync (from ADC mezzanine card 'REF' connector)
  -3  Use 3P LASER Sync (from 3P mezzanine card)
";


        static List<String> ADCSampleClock_setup(List<String> argumentsList)
        {
            List<String> ReturnStrings = new List<string> { };
            int argCount = argumentsList.Count;
            int arg = 1;
            UInt64 RegValue = 0;
            bool bStatus, bLASER_sync_sampling = false;
            bool Use2P_ref = false;
            bool Use3P_ref = false;

            while (arg < argCount)
            {
                switch (argumentsList[arg])
                {
                    case "-c":
                        break;
                    case "-2":
                        Use2P_ref = true;
                        break;
                    case "-3":
                        Use3P_ref = true;
                        break;

                    default:
                        break;
                };
                arg++;
            }

            if (Use2P_ref == true || Use3P_ref == true)
                bLASER_sync_sampling = true;

            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SamplingClockControlReg", ref RegValue);
            string sBuf = string.Format("Initial value of SamplingClockControlReg: 0x{0}", RegValue.ToString("X2"));
            ReturnStrings.Add(sBuf);
            //            UpdateConsoleStatus(string.Format("Initial value of SamplingClockControlReg: 0x{0}", RegValue.ToString("X2")));
            // select the sampling clock as constructed by the AD9517 Synthesizer (pg. 26 "Thordaq System Timing Architecture")
            // a sequence of register settings is necessary to enable or disable the FPGA's PLL/MMCM circuitry
            // and/or the internal 80MHz reference;
            // the PLL/MMCM is required for "LASER sync" mode, where either 2P or 3P LASER pulses are causing the
            // PMT signal we need to capture via ADC stream.  In "Confocal" mode, there is only an internal 
            // 80MHz clock (for 160MHz ADC sampling) input to Synthesizer, and PLL/MMCM is disabled
            if (bLASER_sync_sampling == true) // 2P/3P external LASER input required
            {
                // is the external LASER 2P (from "REF" on ADC mezzanine board) or 3P (from 3P Mezzanine board)
                // ADC FMC settings required - selects between 3p/2p or Internal 80MHz clock and PLL/MMCM enable (SWUG Pg. 39)
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("JESD204B_Sysref_sync_to_laser", 1);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("JESD204B_Sysref_sync_to_laser", ref RegValue);
                ReturnStrings.Add(string.Format("JESD204B_Sysref_sync_to_laser: {0}", RegValue.ToString()));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADC_GPIO1_INT_REF_EN", 0); // enable selection
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADC_GPIO1_INT_REF_EN", ref RegValue);
                ReturnStrings.Add(string.Format("ADC_GPIO1_INT_REF_EN: {0}", RegValue.ToString()));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADC_GPIO0_INT_REF_SEL", 0); // DISABLE internal 80MHz ref
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADC_GPIO0_INT_REF_SEL", ref RegValue);
                ReturnStrings.Add(string.Format("ADC_GPIO0_INT_REF_SEL: {0}", RegValue.ToString()));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADC_GPIO2_L_FPGA_REF_EN", 0); // LASER sync enable
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADC_GPIO2_L_FPGA_REF_EN", ref RegValue);
                ReturnStrings.Add(string.Format("ADC_GPIO2_L_FPGA_REF_EN: {0}", RegValue.ToString()));

                // SWUG Pg. 30
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SC_PHASE_INCREMENT_MODE", 0); // not used in 'confocal'?
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SC_PHASE_INCREMENT_MODE", ref RegValue);
                ReturnStrings.Add(string.Format("    SC_PHASE_INCREMENT_MODE: {0}", RegValue));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SC_FREQ_MEAS_SEL", 1); // use internal 80MHz ref
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SC_FREQ_MEAS_SEL", ref RegValue);
                ReturnStrings.Add(string.Format("    SC_FREQ_MEAS_SEL: {0}", RegValue));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SC_3P_EN", 0);  // don't "source select" 3P 
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SC_3P_EN", ref RegValue);
                ReturnStrings.Add(string.Format("    SC_3P_EN: {0}", RegValue));


            }
            else // configuration for "confocal" using internal 80MHz reference
            {
                // ADC FMC settings required - selects between 3p/2p or Internal 80MHz clock and PLL/MMCM enable (SWUG Pg. 39)
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("JESD204B_Sysref_sync_to_laser", 0);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("JESD204B_Sysref_sync_to_laser", ref RegValue);
                ReturnStrings.Add(string.Format("JESD204B_Sysref_sync_to_laser: {0}", RegValue.ToString()));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADC_GPIO1_INT_REF_EN", 0);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADC_GPIO1_INT_REF_EN", ref RegValue);
                ReturnStrings.Add(string.Format("ADC_GPIO1_INT_REF_EN: {0}", RegValue.ToString()));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADC_GPIO0_INT_REF_SEL", 1); // internal 80MHz ref
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADC_GPIO0_INT_REF_SEL", ref RegValue);
                ReturnStrings.Add(string.Format("ADC_GPIO0_INT_REF_SEL: {0}", RegValue.ToString()));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADC_GPIO2_L_FPGA_REF_EN", 1); // LASER sync disable
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADC_GPIO2_L_FPGA_REF_EN", ref RegValue);
                ReturnStrings.Add(string.Format("ADC_GPIO2_L_FPGA_REF_EN: {0}", RegValue.ToString()));

                // SWUG Pg. 30
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SC_PHASE_INCREMENT_MODE", 0); // not used in 'confocal'?
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SC_PHASE_INCREMENT_MODE", ref RegValue);
                ReturnStrings.Add(string.Format("    SC_PHASE_INCREMENT_MODE: {0}", RegValue));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SC_FREQ_MEAS_SEL", 1); // use internal 80MHz ref
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SC_FREQ_MEAS_SEL", ref RegValue);
                ReturnStrings.Add(string.Format("    SC_FREQ_MEAS_SEL: {0}", RegValue));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SC_3P_EN", 0);  // don't "source select" 3P 
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SC_3P_EN", ref RegValue);
                ReturnStrings.Add(string.Format("    SC_3P_EN: {0}", RegValue));
                // 
                // now clear LASER_MARKEN_EN Sync. for 3P  (SWUG Pg. 34)
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_LASER3P_MARKER_EN", 0);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStream_LASER3P_MARKER_EN", ref RegValue);
                ReturnStrings.Add(string.Format("    ADCStream_LASER3P_MARKER_EN: {0}", RegValue));

            }

            // All modes...
            // default setting SWUG Pg. 40 -- when would it change?
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADC_GPIO3_FiltAB_MODE", 2); // LASER ref input clock B/W
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADC_GPIO3_FiltAB_MODE", ref RegValue);
            ReturnStrings.Add(string.Format("ADC_GPIO3_FiltAB_MODE: {0}", RegValue.ToString()));
            // Add final string for "SUCCESS" or "FAILURE" status of entire function
            ReturnStrings.Add("SUCCESS");
            return ReturnStrings;
        }

        const string HELP_ScanControl_setup_MSSAGE = @"ScanControl_setup: 
Scan Control subsystem
Options:
  -m  Image Synchronization Master Timing Mode (0 = Res-Galvo, 1 = Galvo-Galvo, 2= External  <def. Galvo-Galvo>)
  -e  Enable External Hardware Trigger HW_TRIG_MODE (BAR3, Offset 0x8)
  -p  'ScanningGalvoPixelDwell', BAR3 0x0160 (Galvo-Galvo mode only) (def. 10)
  -P  'ScanningGalvoPixelDelay', BAR3 0x0168 (Galvo-Galvo mode only) (def. 1)
  -l  'ScanningIntraLineDelay',  BAR3 0x0170 (def. 12)
  -f  'ScanningIntraFrameDelay', BAR3 0x0178 (def. 25)
  -s  'ScanningPreSOFDelay',     BAR3 0x017B (def. 1)
  -B  BiDirectional mode 'BIDIR_SCAN_MODE' (def. single direction)
";
        static List<string> ScanControl_setup(List<String> argumentsList)
        {
            List<string> ReturnStrings = new List<string> { };
            int argCount = argumentsList.Count;
            int arg = 1;
            UInt64 RegValue = 0;
            UInt64 HW_Trig_Mode = 0; // def. normal
            bool bStatus;
            UInt32 PixelDelay = 1;   // 16ns Tick_Blk (valid range 1 -> (2^24-1) (ThorImageLS constant at 1)
            UInt32 PixelDwell = 10; // 5 ns Ticks
            UInt32 ScanningIntraLineDelay = 12; //   12 // 0x4410 from 4096x4096
            UInt32 ScanningIntraFrameDelay = 25; // 25 //  0x1d23c from 4096
            UInt32 ScanningPreSOFDelay = 1; // minimum 1 per SWUG
            UInt32 BiDirectionMode = 0;  // single direction default
            UInt32 MasterTimingScanMode = (UInt32)TDcontroller.ImageSyncMasterTimingMode.Galvo_Galvo; // galvo-galvo

            while (arg < argCount)
            {
                switch (argumentsList[arg])
                {
                    case "-e":
                        HW_Trig_Mode = 1; // "HW Trigger Mode Enabled" in SWUG
                        break;
                    case "-m":
                        UInt32.TryParse(argumentsList[arg + 1], out MasterTimingScanMode);
                        arg++; // next flag
                        break;
                    case "-p":
                        UInt32.TryParse(argumentsList[arg + 1], out PixelDwell);
                        arg++; // next flag
                        break;
                    case "-P":
                        UInt32.TryParse(argumentsList[arg + 1], out PixelDelay);
                        arg++; // next flag
                        break;
                    case "-l":
                        UInt32.TryParse(argumentsList[arg + 1], out ScanningIntraLineDelay);
                        arg++; // next flag
                        break;
                    case "-f":
                        UInt32.TryParse(argumentsList[arg + 1], out ScanningIntraFrameDelay);
                        arg++; // next flag
                        break;

                    case "-s":
                        UInt32.TryParse(argumentsList[arg + 1], out ScanningPreSOFDelay);
                        arg++; // next flag
                        break;

                    case "-B":
                        UInt32.TryParse(argumentsList[arg + 1], out BiDirectionMode);
                        arg++; // next flag
                        break;

                    default:
                        ReturnStrings.Add(string.Format("Unknown option: {0}", argumentsList[arg]));
                        break;
                };
                arg++;
            }

            // APISetBOBstatus() writes default ABBX MUX values according to attached BOB
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalABBXmuxReg", ref RegValue);
            ReturnStrings.Add(string.Format("GlobalABBXmuxReg: 0x{0}", RegValue.ToString("X16")));

            // Command line args set...
            // Global Image Sync  "GlobalImageSyncControlReg" (0x08)
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("BIDIR_SCAN_MODE", BiDirectionMode); // 
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("BIDIR_SCAN_MODE", ref RegValue);
            ReturnStrings.Add(string.Format("BIDIR_SCAN_MODE: {0}", RegValue.ToString()));
            // GlobalImageSyncControlReg (0x08) HW_TRIG_MODE deprecated in 20220901 and later F/W
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ACQ_HW_TRIGGER_MODE", HW_Trig_Mode); 
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ACQ_HW_TRIGGER_MODE", ref RegValue);
            ReturnStrings.Add(string.Format("HW_TRIG_MODE: {0}", RegValue.ToString()));
            // set appropriate Registers for Image Sync timing mode
            if (MasterTimingScanMode == (UInt32)TDcontroller.ImageSyncMasterTimingMode.Galvo_Galvo)
            {
                // (also set in ADCStream API)
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_SCAN_PER_SRCE", 0);

                // pixel timing
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ScanningGalvoPixelDwell", PixelDwell); // 5ns tick 
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ScanningGalvoPixelDwell", ref RegValue);
                ReturnStrings.Add(string.Format("ScanningGalvoPixelDwell: {0}", RegValue.ToString()));
                // "PixelDelay" always 1 in TILS
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ScanningGalvoPixelDelay", PixelDelay); // 16ns tick_block 
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ScanningGalvoPixelDelay", ref RegValue);
                ReturnStrings.Add(string.Format("ScanningGalvoPixelDelay: {0}", RegValue.ToString()));
                // line timing
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ScanningIntraLineDelay", ScanningIntraLineDelay); // 2405ns tick_block match 16x16 timing diagram
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ScanningIntraLineDelay", ref RegValue);
                ReturnStrings.Add(string.Format("ScanningIntraLineDelay: {0}", RegValue.ToString()));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ScanningIntraFrameDelay", ScanningIntraFrameDelay);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ScanningIntraFrameDelay", ref RegValue);
                ReturnStrings.Add(string.Format("ScanningIntraFrameDelay: {0}", RegValue.ToString()));

                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ScanningPreSOFDelay", ScanningPreSOFDelay);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ScanningPreSOFDelay", ref RegValue);
                ReturnStrings.Add(string.Format("ScanningPreSOFDelay: {0}", RegValue.ToString()));


                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SCAN_DIR", 0); // "forward" scan
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SCAN_DIR", ref RegValue);
                ReturnStrings.Add(string.Format("SCAN_DIR: {0}", RegValue.ToString()));

            }
            else if (MasterTimingScanMode == (UInt32)TDcontroller.ImageSyncMasterTimingMode.Resonant_Galvo)
            {
                // (also set in ADCStream API)
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_SCAN_PER_SRCE", 1);

                // note that normal Resonant line scan controller needs to "warm up" for thermal drift stability
                // so the usual circumstance is the ADPLL is locking onto the Resonant feedback signal before 
                // ThorDAQ software starts.
                // Configure settings are here for test purposes
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("APDLL_ControlReg", 0x42); // Ki(4) and Kp(2) constants
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("APDLL_ControlReg", ref RegValue);
                ReturnStrings.Add(string.Format("APDLL_ControlReg: {0}", RegValue.ToString()));

                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ScanningIntraFrameDelay", ScanningIntraFrameDelay);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ScanningIntraFrameDelay", ref RegValue);
                ReturnStrings.Add(string.Format("ScanningIntraFrameDelay: {0}", RegValue.ToString()));

                // Set the center frequency for the ADPLL control loop
                UInt64 uFreqTarget = 7900;  // 7.9 kHz for "8 KHz" scanner - what about 12 KHz ?
                double pow32 = Math.Pow(2, 32);
                UInt64 uiCenterFreq = uFreqTarget * (UInt64)pow32 / 160000000;
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("APDLL_DCO_CenterFreq", uiCenterFreq);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("APDLL_DCO_CenterFreq", ref RegValue);
                ReturnStrings.Add(string.Format("APDLL_DCO_CenterFreq: {0}", RegValue.ToString()));

                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("APDLL_SyncDelay", 0);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("APDLL_PhaseOffset", 0);

                // mix it up -- GG uses "forward" scan
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SCAN_DIR", 1); // "reverse" scan
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SCAN_DIR", ref RegValue);
                ReturnStrings.Add(string.Format("SCAN_DIR: {0}", RegValue.ToString()));
            }
            else  // External Source
            {

            }
            // Scanning sync control bits
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SRCE_SOF_MODE", 1); // 1 = on 1st LineSync after Start
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SRCE_SOF_MODE", ref RegValue);
            ReturnStrings.Add(string.Format("SRCE_SOF_MODE: {0}", RegValue.ToString()));
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("FRM_GEN_MODE", 0);
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("FRM_GEN_MODE", ref RegValue);
            ReturnStrings.Add(string.Format("FRM_GEN_MODE: {0}", RegValue.ToString()));
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("TIME_SRCE_SELECT", MasterTimingScanMode);
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("TIME_SRCE_SELECT", ref RegValue);
            ReturnStrings.Add(string.Format("TIME_SRCE_SELECT: {0}", RegValue.ToString()));

            ReturnStrings.Add("SUCCESS");
            return ReturnStrings;
        }


        const string HELP_S2MM_DMA_setup_MSSAGE = @"S2MM_DMA_setup: <-d WxH> ...
DMA setup of all four image channels is in three phases:  
    1. Configure the S2MM FPGA to DDR3 BRAM descriptors (both banks)
    2. Write the sample LUT (Look-Up Table)
    3. Enable the S2MM Interrupts which trigger NWA common User_Interrupt
There is one Win O/S device driver vector for all interrupts, including
NWA read to DDR3, NWA write to DDR3, and FPGA (ThorDAQ) user int.
Options:
  -d  Width by Height dimension, e.g. '-d 32x16' means 32-pixels by 16 lines; def. 32x32
  -c  Channel mask (e.g. 0x6 for channels 1&2, 0xF for chan 0-3), def. 0x1
  -a  Physical starting address of DDR3 (def. 0x0)
  -n  Number of descriptors (or 'frames') per bank (two banks), def. 3
";
        static List<string> S2MM_DMA_setup(List<String> argumentsList)
        {
            List<string> ReturnStrings = new List<string> { };
            string FunctionStatus = "SUCCESS";
            int ChannelMask = 1; // default channel 0 only (4 total)
            int DDR3startAddr = 0x0; // 0 is valid DDR3 start
            int arg = 1;
            int BRAMdescriptorCount = 3; // number of BRAM descriptors per bank (per channel)
            int argCount = argumentsList.Count;
            string[] FrameDimension;
            int PixelsInLine = 0;   // Width dimension (0 means read shadow register value)
            int TotalLines = 0;     // Height dimension (can be 1 for single line scan)
            S2MM_CONFIG s2mmConfig = new S2MM_CONFIG();
            IntPtr ADClineSamplesLUT;
            // parse args...
            while (arg < argCount)
            {
                switch (argumentsList[arg])
                {
                    case "-a":
                        if (argumentsList[arg + 1].Contains("0x"))
                        {
                            DDR3startAddr = int.Parse(argumentsList[arg + 1].Replace("0x", string.Empty), NumberStyles.HexNumber);
                        }
                        else
                        {
                            DDR3startAddr = int.Parse(argumentsList[arg + 1]);
                        }
                        arg++; // next flag
                        break;
                    case "-c": // channel mask
                        if (argumentsList[arg + 1].Contains("0x"))
                        {
                            ChannelMask = int.Parse(argumentsList[arg + 1].Replace("0x", string.Empty), NumberStyles.HexNumber);
                        }
                        else
                        {
                            ChannelMask = int.Parse(argumentsList[arg + 1]);
                        }
                        arg++; // next flag
                        break;
                    case "-n":
                        Int32.TryParse(argumentsList[arg + 1], out BRAMdescriptorCount);
                        arg++; // next flag
                        break;

                    case "-d":  // get frame dimensions
                        FrameDimension = argumentsList[arg + 1].Split('x');
                        Int32.TryParse(FrameDimension[0], out PixelsInLine);
                        Int32.TryParse(FrameDimension[1], out TotalLines);
                        arg++; // next flag
                        break;
                }
                arg++; // next flag
            }
            // variables set ...
            UInt64 Value = 0;
            if (PixelsInLine == 0) // user wants existing dimensions...
            {
                // if user hasn't already set WxH dimension, notify and exit
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageHSIZE", ref Value);
                if (Value == 0)
                {
                    ReturnStrings.Add("Fatal Error: Image dimensions HSIZE/VSIZE must be defined");
                    ReturnStrings.Add("FAIL");
                    return ReturnStrings;
                }
                // Value is HSIZE, the pixel (image size -1) per SWUG
                PixelsInLine = (int)(Value + 1);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageVSIZE", ref Value);
                TotalLines = (int)(Value + 1);
            }
            else // user specifies (new) dimensions
            {
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageHSIZE", (ulong)PixelsInLine - 1);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageAcqHSIZE", (ulong)PixelsInLine - 1); // assume single Plane!
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageVSIZE", (ulong)TotalLines - 1);
            }
            // review the settings...
            UInt64 S2MMwidth = 0, S2MMheight = 0; // temp variables
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageHSIZE", ref S2MMwidth);
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageAcqHSIZE", ref Value);
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageVSIZE", ref S2MMheight);

            string S2MM_imageDimensions = string.Format("Image FPGA Global HSIZE x VSIZE {0}w x {1}h (GlobalImageAcqHSIZE: {2})", S2MMwidth, S2MMheight, Value);
            ReturnStrings.Add(S2MM_imageDimensions);

            s2mmConfig.HSize = PixelsInLine;
            s2mmConfig.VSize = TotalLines;
            s2mmConfig.ChannelMask = ChannelMask;
            s2mmConfig.NumberOfDescriptorsPerBank = BRAMdescriptorCount;
            s2mmConfig.DRAMstartingAddress = DDR3startAddr;

            DERIVED_TIMING DerivedTiming = new DERIVED_TIMING();
            double dFramesPerSec = 0;
            double dSecondsPerFrame = 0;
            double dMBytesPerSec = 0;

            int iStatus = ThorDAQAdapters[MasterBoardIndex].S2MMadcDMA.Init(ref s2mmConfig, ref DerivedTiming);
            if (iStatus != 0)
            {
                ReturnStrings.Add(ThorDAQAdapters[MasterBoardIndex].S2MMadcDMA.sErrMsg);
                FunctionStatus = "FAIL";
            }
            else
            {
                string Msg;
                bool bStatus;
                UInt64 RegValue = 0;
                // T_frame is in NANO seconds -- get Frames/Sec rate
                if (DerivedTiming.T_frame != 0)
                {
                    dSecondsPerFrame = (double)(DerivedTiming.T_frame * Math.Pow(10, -9));
                    dFramesPerSec = 1.0 / dSecondsPerFrame;
                    dMBytesPerSec = dFramesPerSec * (PixelsInLine * TotalLines * 2) / 1024 / 1024;
                    Msg = string.Format("  DMA rate: Frames per second {0,10:#.00}, ({1:#0.000,000} sec/frame, {2:#.000} usec/line) {3,10:#.00} MB/sec",
                        dFramesPerSec, dSecondsPerFrame, ((double)DerivedTiming.T_line / 1000.0), dMBytesPerSec);
                    ReturnStrings.Add(Msg);
                    Msg = string.Format("  Pixel to Pixel timing: {0:#0.000} micro-sec",
                        ((double)DerivedTiming.Pixel_to_Pixel_timeNS / 1000.0));
                    ReturnStrings.Add(Msg);

                    // "Scanning Period" register (for GalvoGalvo)
                    ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("TIME_SRCE_SELECT", ref RegValue);
                    if (RegValue == (UInt32)TDcontroller.ImageSyncMasterTimingMode.Galvo_Galvo)
                    {
                        bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStreamDownsampleReg", DerivedTiming.ADCStreamDownsamplingRate);
                        bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStreamScanningPeriodReg", DerivedTiming.ADCStreamScanningPeriodReg);
                        if (bStatus != true)
                            ReturnStrings.Add("Error write register 'ADCStreamScanningPeriodReg' -- outdated 'thordaq' DLL ?? ");

                        // LookUp Table required for DMA - for Galvo-Galvo, just linearly divide up lineADCsampleAcqBuffer
                        // NOTE!  If samples too close to START or END of line ("ADCStreamScanningPeriodReg" # of samples)
                        // symptom is ADC samples close to end of frame are NOT DMA'd!
                        // We know this from setting DDR3 DMA destination memory to constants like 0xAA,
                        // then when DMA for frame completes, "0xAA" values are present near end of frame
                        UInt32 FPGA_ADCsampleLineAcqBuffStep = (UInt16)((DerivedTiming.ADCStreamScanningPeriodReg - 1) / (ulong)PixelsInLine);
                        UInt32 nextAcqBufSamplePickoff = 1;

                        ///// TEST!!!!!!!!!!!!!!!!
                        //    FPGA_ADCsampleLineAcqBuffStep = 40;
                        //    nextAcqBufSamplePickoff = 32142;

                        short[] LUTbuffer; // for debug/copy
                        const int LUTbyteChannelSize = 4096 * 2 * 8; // (4k pixels) new 64k REVERB/MP size in bytes
                        LUTbuffer = new short[LUTbyteChannelSize / 2];
                        // NOTE!  Reduced LUT array to SINGLE channel because 4 channels exceeds Marshalling size limit
                        unsafe // required to access the fixed arrays...
                        {
                            for (int pixel = 0; pixel < PixelsInLine;
                                     pixel++, nextAcqBufSamplePickoff += FPGA_ADCsampleLineAcqBuffStep)
                            {
                                LUTbuffer[pixel] = (short)nextAcqBufSamplePickoff; // (debug)
                                //LUTbuffer[pixel] = (ushort)((ushort)LUTbuffer[pixel]  & (ushort)0xFFFF); // limit 
                            }
                        }
                        ReturnStrings.Add(string.Format("    LUTbuffer[0]: {0} LUTbuffer[{1}]: {2} (First and last LUT indexes)",
                                                            LUTbuffer[0], (ushort)(PixelsInLine) - 1, LUTbuffer[(ushort)(PixelsInLine) - 1]));

                        // Write to FPGA
                        int size = Marshal.SizeOf(LUTbuffer[0]) * LUTbuffer.Length;
                        ADClineSamplesLUT = Marshal.AllocHGlobal(size); // allocated "unmanaged" buffer
                        // copy the "managed" LUT into unmanaged space
                        Marshal.Copy(LUTbuffer, 0, ADClineSamplesLUT, LUTbuffer.Length);
                        // (the .NET managed buffer shows 32-bit sign-extended "ushorts", but only lower 16-bits sent to DLL)
                        iStatus = ThorDAQAdapters[MasterBoardIndex].S2MMadcDMA.LoadLUT(ADClineSamplesLUT);
                        if (iStatus != (int)THORDAQ_STATUS.STATUS_SUCCESSFUL)
                            ReturnStrings.Add("Error on S2MMadcDMA.LoadLUT(): can't load LUT");
                        else
                        {
                            /*                   UpdateConsoleStatus("LookUpTable (LUT) contents:");  // DEBUG LUT
                                               for (int p = 0; p < PixelsInLine; p += 8)
                                               {
                                                   UpdateConsoleStatus(string.Format("    [{0}]: {1,5} {2,5} {3,5} {4,5} {5,5} {6,5} {7,5} {8,5}", 
                     p, LUTbuffer[p], LUTbuffer[p+1],LUTbuffer[p+2],LUTbuffer[p+3],LUTbuffer[p+4],LUTbuffer[p+5],LUTbuffer[p+6],LUTbuffer[p+7]));
                                               }*/
                            ReturnStrings.Add("S2MMadcDMA.LoadLUT() successfully loaded LUT (all channels)");
                        }
                        Marshal.FreeHGlobal(ADClineSamplesLUT);
                    }
                    // ADCStreamScanningPeriodReg should be "0" if NOT GalvoGalvo mode
                    ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStreamDownsampleReg", ref RegValue);
                    ReturnStrings.Add(string.Format("    ADCStreamDownsamplingRate: {0} (D/S Factor = Rate+1)", RegValue));
                    ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStreamScanningPeriodReg", ref RegValue);
                    ReturnStrings.Add(string.Format("    ADCStreamScanningPeriodReg: {0} (ADC samples per line)", RegValue));

                }
                Msg = string.Format("Configured S2MM DMA (both banks), ChannelMask 0x{0} for {1} Descriptors (Frames) to DDR3 @ {2}h - {3}h",
                    ChannelMask.ToString("X"), BRAMdescriptorCount, s2mmConfig.DRAMstartingAddress.ToString("X8"),
                    s2mmConfig.DRAMendingAddress.ToString("X8"));
                ReturnStrings.Add(Msg);

                ThorDAQAdapters[MasterBoardIndex].S2MMadcDMA.IRQcontrol(true, true); // enable IRQs
                ReturnStrings.Add("Enabled DMA / user Interrupts");
            }
            ReturnStrings.Add(FunctionStatus); // SUCCESS or FAIL for entire function
            return ReturnStrings;
        }

        const string HELP_BuildWaveform_MSSAGE = @"BuildWaveform: <-f int> <-l int> <-h int> <-a int> <-r int> <-t float> <-s int>
Options:
  -C  DACcounts - def 65536 cnts (16-bit resolution)
  -V  Volts range peak to peak - def. 20 VDC (i.e. +/- 10 VDC)
  -f  outputFileSpec (def. 'c:\temp\Test_Wave.txt')
  -l  lowAmplitude  (def. 0 DAC cnts)
  -h  highAmplitude (def. DACcounts-1)
  -a  peak to peak amplitude (center around [DACcounts/2] DAC cnts, e.g. ~0 VDC)
  -r  frequency (def. 14 Hz)
  -t  totalPlaybackTime (def. 1.0 sec)
  -s  sample playback fequency (def. 1,000,000 sample/sec (fastest for FPGA DAC) , slowest 3,050) 
  -v  output file in units of Volts (def. DAC counts) AND in NI1071 format of single line with CSV voltage values 
      (def. output is 16-bit DAC counts, 1 sample/line )"
;

        void BuildWaveform(List<String> argumentsList)
        {
            List<String> DACsamples = new List<String>();
            int uiDACmaxCounts = 65536 - 1;
            int uiVoltsRange = 20;
            int lowAmplitude = 0;
            int highAmplitude = uiDACmaxCounts;
            int centeredAmplitude = -1; // amplitude centered around 32768
            int frequency = 14; // cycles/sec
            double totalPlaybackTime = 1.0; // sec
            int playbackFreq = 1000000; // samples/sec
            string fileSpec = @"c:\temp\Test_Wave.txt";
            bool bVoltsUnits = false;
            int arg = 1;
            int argCount = argumentsList.Count;
            // parse args...
            while (arg < argCount)
            {
                switch (argumentsList[arg])
                {
                    case "-C":
                        Int32.TryParse(argumentsList[arg + 1], out uiDACmaxCounts);
                        uiDACmaxCounts -= 1; // inclusive of "0" counts
                        arg++; // next flag
                        break;
                    case "-V":
                        Int32.TryParse(argumentsList[arg + 1], out uiVoltsRange);
                        arg++; // next flag
                        break;
                    case "-v":
                        bVoltsUnits = true;
                        break;
                    case "-a":
                        Int32.TryParse(argumentsList[arg + 1], out centeredAmplitude);
                        arg++; // next flag
                        break;

                    case "-l":  // low amplitude
                        Int32.TryParse(argumentsList[arg + 1], out lowAmplitude);
                        arg++; // next flag
                        break;
                    case "-h":
                        Int32.TryParse(argumentsList[arg + 1], out highAmplitude);
                        arg++; // next flag
                        break;
                    case "-f":
                        fileSpec = argumentsList[arg + 1];
                        arg++; // next flag
                        break;
                    case "-r":  // frequency
                        Int32.TryParse(argumentsList[arg + 1], out frequency);
                        arg++; // next flag
                        break;
                    case "-t":  // frequency
                        Double.TryParse(argumentsList[arg + 1], out totalPlaybackTime);
                        arg++; // next flag
                        break;
                    case "-s":  // sample-playback frequency
                        Int32.TryParse(argumentsList[arg + 1], out playbackFreq);
                        arg++; // next flag
                        break;
                }
                ++arg;
            };
            if (bVoltsUnits == true)
            {
                fileSpec += ".Volts";
            }
            UpdateConsoleStatus("Writing file " + fileSpec + " ...");

            double period;
            int amplitude;
            int MidPoint;
            if (centeredAmplitude != -1) // user provided "Amplitude" arg?
            {
                //                            int uiDACmaxCounts = 65536 - 1;
                //                          int uiVoltsRange = 20;
                highAmplitude = (uiDACmaxCounts + 1) / 2 + (centeredAmplitude / 2);
                lowAmplitude = (uiDACmaxCounts + 1) / 2 - (centeredAmplitude / 2);
            }
            amplitude = highAmplitude - lowAmplitude;
            MidPoint = (highAmplitude - lowAmplitude) / 2 + lowAmplitude;

            if (frequency > 0)
                period = 1.0 / (double)frequency;
            else
            {
                UpdateConsoleStatus("Param Error: frequency must be > 0");
                return;
            }
            ulong totalSamples = (ulong)(totalPlaybackTime * (double)playbackFreq);
            ulong samplesPerPeriod = (ulong)(period * playbackFreq);
            ulong sampleCount = 0;
            // TEST!!!
            //samplesPerPeriod = 32;
            double PIdivisor = (2 * Math.PI) / (double)samplesPerPeriod;
            // The FPGA hardware expects the waveform to start at its lowest value (i.e. "0" DAC counts),
            // because the "Park" value is also expected to be low (like 0 DAC counts, -10 VDC)
            // "0" DAC Counts in waveform places the waveform at the "Offset" voltage setting.
            // The FPGA intends to gradually increase (in ~ 1-2 microsecs) the DAC from
            // park voltage to the "offset" voltage (defined and changeable outside the waveform defintion).
            // If the waveform does not start (and end) with "0", there will be a step
            // voltage back to the "offset" DAC value which will likely "trip" a Gavlo
            // servo controller
            double PIangleRadians = 3 * Math.PI / 2;
            Int64 ASample;
            do
            {
                for (; sampleCount < totalSamples; sampleCount++, PIangleRadians += PIdivisor)
                {
                    double SignedRatio = Math.Sin(PIangleRadians);
                    if (SignedRatio > 0)
                    {
                        double AboveDelta = (highAmplitude - MidPoint) * SignedRatio;
                        ASample = MidPoint + (int)(AboveDelta);
                    }
                    else if (SignedRatio < 0)
                    {
                        double BelowDelta = (MidPoint - lowAmplitude) * SignedRatio;
                        ASample = MidPoint + (int)(BelowDelta);
                    }
                    else
                        ASample = MidPoint;
                    // write it to list
                    DACsamples.Add(string.Format("{0}", ASample.ToString()));
                }
            } while (sampleCount < totalSamples);

            // convert to Volts?
            if (bVoltsUnits == true) // write VoltsDC list to file
            {
                System.IO.StreamWriter VoltsFile = null;
                List<String> dVoltsSamples = new List<String> { };
                double dVolts = 0.0;
                try
                {
                    using (VoltsFile = new System.IO.StreamWriter(fileSpec))
                    {
                        foreach (string sSample in DACsamples)
                        {
                            UInt16 DACcnts = UInt16.Parse(sSample);
                            // will voltage be positive or negative?

                            dVolts = (((double)uiVoltsRange) / ((double)(uiDACmaxCounts + 1)) * (DACcnts)) - ((double)uiVoltsRange / 2.0);
                            dVoltsSamples.Add(string.Format("{0}", dVolts.ToString()));


                        }
                        // now join all the Volt samples into a single line separated by ','
                        // this is the easiest format for NI Labview processing
                        String LineOfAllVoltsCSV;
                        LineOfAllVoltsCSV = String.Join(",", dVoltsSamples);
                        VoltsFile.WriteLine(LineOfAllVoltsCSV);
                    }
                }
                catch (Exception e)
                {
                    UpdateConsoleStatus(string.Format("Exception writing {0}: {1}", fileSpec, e.Message));
                }
                finally
                {
                    if (VoltsFile != null)
                    {
                        VoltsFile.Close();
                    }
                }
            }
            else  // write DAC-counts list to file
            {
                File.WriteAllLines(fileSpec, DACsamples, Encoding.UTF8);
            }
        }

        static ulong channelPlaybackMask = 0; // accumulated mask of enabled channels
        static List<Enum> channelPlaybackLEDs = new List<Enum> { };

        static ulong DDR3_playbackMemStart = 0x50000000;
        static bool WaveformPlaybackInitializedOnce = false; // too easy for user to miss this

        const string HELP_DACWaveform_setup_MSSAGE = @"DACWaveform_setup: <-i> <-j> <-c ChannelMask> <-f {filename}> 
Options:
  -i               DO NOT Initialize the Waveform Playback Table 
  -o DACcnts       Offset setting (def. set to 0 DAC counts)
  -c ChannelMask   e.g. 0xF for all ABB1 outputs, 0xF16 for all ABB3 + AO5 + AO2-3, default 0xFFF (all channels ABBx)
  -f {outFileSpec} Def. 'c:\temp\Test_Wave.txt', or if NULL, dialog box for file
  -z PlaybackFreq  Def. 1000000 (in Hz)
"
;

        static List<string> DACWaveform_setup(List<String> argumentsList)
        {
            List<string> ReturnStrings = new List<string> { };
            string playbackFilespec = "c:\\temp\\test_wave.txt";
            int argNum;
            uint iChannelIndexMask = 0xFFF; // Index 0 - 12 (for 13 hardware channels), 0x6 is channelIndex 2,1, 0x9 is 8,0
            bool bInitFlag = true;
            bool bStatus;
            ulong RegValue = 0;
            ulong OffsetDACcntsValue = 0;
            uint PlaybackFreq = 1000000;
            string sFunctionStatus = "FAIL";

            channelPlaybackLEDs.Clear();
            channelPlaybackMask = 0; // reset all channels - reset below per user request
                                     // route the Analog (ABBx) I/O signals
                                     // 

            for (argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-z":
                        PlaybackFreq = uint.Parse(argumentsList[++argNum]); // (advance ARG list)
                        break;
                    case "-i":
                        bInitFlag = false;
                        break;

                    case "-o":
                        OffsetDACcntsValue = uint.Parse(argumentsList[++argNum]);
                        break;

                    case "-f":
                        if (argNum + 1 < argumentsList.Count)
                            playbackFilespec = argumentsList[++argNum]; // next arg after "-f" is filespec
                        else
                        {
                            //Create OpenFileDialog - get filename from user
                            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
                            dlg.DefaultExt = ".mcs";
                            dlg.Filter = "Text Files (.txt)|*.txt";
                            // Display OpenFileDialog by calling ShowDialog method
                            Nullable<bool> result = dlg.ShowDialog();
                            // Get the selected file name and display in a TextBox
                            if (result == true)
                            {
                                // Open document
                                playbackFilespec = dlg.FileName;
                            }
                        }

                        break;
                    case "-c":
                        // anticipate HEX mask
                        if (argumentsList[argNum + 1].Contains("0x"))
                        {
                            iChannelIndexMask = uint.Parse(argumentsList[argNum + 1].Replace("0x", string.Empty), NumberStyles.HexNumber);
                        }
                        else
                        {
                            iChannelIndexMask = uint.Parse(argumentsList[argNum + 1]);
                        }
                        argNum++;
                        break;
                }
            }
            // Check for component reset flags...
            if (bInitFlag == true || (WaveformPlaybackInitializedOnce == false)) // failure to init once is mass confusion
            {
                WaveformPlaybackInitializedOnce = true;
                for (int chIndx = 0; chIndx < 13; chIndx++)
                {
                    string Playback_EnX = "DAC_DMA_Playback_En" + chIndx.ToString();

                    bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(Playback_EnX, 0); // (reset)
                    ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read(Playback_EnX, ref RegValue);
                    //                    ReturnStrings.Add(string.Format("    {0}: 0x{1}", Playback_EnX, RegValue.ToString("X3")));
                }
                // reset the table
                bStatus = ThorDAQAdapters[MasterBoardIndex].DACcard.DACWave.Init(DDR3_playbackMemStart, 0); // unknown WaveformLen
                if (bStatus != true)
                    ReturnStrings.Add(ThorDAQAdapters[MasterBoardIndex].DACcard.DACWave.sErrMsg);
                else
                    ReturnStrings.Add(string.Format("DACWave.Init() @ {0}: Success", DDR3_playbackMemStart.ToString("X8")));
            }

            // Waveform sample count delays -- kludge design to attempt hardware solution to Galvo control tripping
            // because of too rapid voltage changes, such as when transitioning to park voltage to first DAC sample
            // in waveform
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("DACWaveGen_SOF_DelaySysClkCnt", 399); // 1 = 5ns, 399 = 2us (used? try 0)
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("DACWaveGen_SOF_DelaySysClkCnt", ref RegValue);
            ReturnStrings.Add(string.Format("DACWaveGen_SOF_DelaySysClkCnt: {0}", RegValue.ToString()));

            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("DACWaveGen_EOF_DelaySampleCnt", 1);
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("DACWaveGen_EOF_DelaySampleCnt", ref RegValue);
            ReturnStrings.Add(string.Format("DACWaveGen_EOF_DelaySampleCnt: {0}", RegValue.ToString()));



            // load the waveforms in preparation for playback
            // which waveform channels will be used (from 1 to 13, 12 DAC analog + 1 Digital Output (DBB1 BrkoutBox)
            ulong chanSelector = 1;
            ulong MaxChannelOrdinal = 0;        // HIGHEST enabled channel! FPGA "feature" is you must enable ALL channels up to the one you want
                                                // so if you only want channel 12 (Digital IO) you must enable channels 0 through 11

            // For now - no BANK SWITCHING support
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("DAC_Bank_Switch_EN", 0);

            for (ulong chan = 0; chan < 13; chan++, chanSelector <<= 1)
            {
                UInt64 DDR3starAddr;
                int iStatus;
                if ((chanSelector & iChannelIndexMask) != 0)
                {
                    ReturnStrings.Add("  Load DAC waveform '" + playbackFilespec + "' in channel " + chan + " ...");
                    iStatus = ThorDAQAdapters[MasterBoardIndex].DACcard.DACWave.Load((uint)chan, playbackFilespec, out DDR3starAddr);
                    if (iStatus <= 0)  // error, or no DAC samples loaded?
                    {
                        ReturnStrings.Add(ThorDAQAdapters[MasterBoardIndex].DACcard.DACWave.sErrMsg);
                        sFunctionStatus = "FAILED to Load DAC waveform: " + playbackFilespec;
                    }
                    else // channel waveform is loaded... update other DAC registers
                    {
                        sFunctionStatus = "SUCCESS";
                        MaxChannelOrdinal = chan + 1;
                        ReturnStrings.Add(string.Format(" {0}(0x{1} bytes) DAC samples starting @ DDR3 addr 0x{2}, Playback Freq {3} Hz", iStatus, (iStatus * 2).ToString("X"), DDR3starAddr.ToString("X8"), PlaybackFreq));
                        // read shadow reg of waveform playback table channel list start
                        string DACdescChan = "DACWavePlayDMAdescriptorChan" + chan.ToString();
                        bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read(DACdescChan, ref RegValue);
                        ReturnStrings.Add(string.Format("    {0}: 0x{1}", DACdescChan, RegValue.ToString("X16")));

                        // set the update rate - FPGA says must be >= 199 (typo fixed in later SWUG)
                        // compute the FPGA register setting...
                        UInt64 DAC_UpdateRate = (200000000 / PlaybackFreq) - 1; // 200MHz sysclock
                                                                                // set offset

                        // DAC_Sync_hsync0 (Set to 1)   DAC_UpdateRate_Chan0

                        // Set Park and Offset for DAC 
                        if (chan == 12) // special case for Digital Outputs channel
                        {
                            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("DACWaveGenDigitOutputsPark", 0);
                            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("DACWaveGenDigitOutputsOffset", 0);
                            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("DACWaveGenDigitOutputsRate", DAC_UpdateRate);
                        }
                        else
                        {
                            string strOffset = "DAC_Park_Chan" + chan.ToString();
                            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(strOffset, 0);

                            strOffset = "DAC_Offset_Chan" + chan.ToString();
                            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(strOffset, OffsetDACcntsValue);
                            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read(strOffset, ref RegValue);
                            ReturnStrings.Add(string.Format("    {0}: {1}", strOffset, RegValue.ToString()));

                            string strUpdateRate = "DAC_UpdateRate_Chan" + chan.ToString();
                            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(strUpdateRate, DAC_UpdateRate);
                            if (bStatus != true)
                                ReturnStrings.Add("FAILED FPGAregs.Write() for " + strUpdateRate);
                            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read(strUpdateRate, ref RegValue);
                            if (bStatus != true)
                                ReturnStrings.Add("FAILED FPGAregs.Read() for " + strUpdateRate);
                            ReturnStrings.Add(string.Format("    {0}: {1}", strUpdateRate, RegValue.ToString()));
                        }
                        // "enable" the DMA playback on the activated channel (bitwise)
                        channelPlaybackMask |= chanSelector;

                        string PlayBackEnXx = "DAC_DMA_Playback_En" + chan.ToString();

                        bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(PlayBackEnXx, 1);
                        ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read(PlayBackEnXx, ref RegValue);
                        ReturnStrings.Add(string.Format("    {0}: 0x{1}", PlayBackEnXx, RegValue.ToString("X3")));

                        // (re)synchronize DAC update clock with HSync
                        string Sync_hsyncXx = "DAC_Sync_hsync" + chan.ToString();
                        bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(Sync_hsyncXx, 0);
                        //                            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("DAC_Sync_hsync", channelPlaybackMask);
                        ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read(Sync_hsyncXx, ref RegValue);
                        ReturnStrings.Add(string.Format("    {0}: 0x{1}", Sync_hsyncXx, RegValue.ToString("X3")));
                    }
                }
            }

            // all waveform channels loaded/enabled...
            if (MaxChannelOrdinal > 0) // ANY channel selected?
            {
                // store the LED enum to control on/off on GlobalStart/Stop
                // chan selector mask bit pattern, e.g. b100101010011 is channels 0,1,4,6,8,11
                for (int iForLED = 0; iForLED < 12; iForLED++)
                {
                    if ((chanSelector & 0x1) != 0)
                    {
                        string LEDlabel = "AO" + (iForLED + 1).ToString();
                        BBoxLEDenum LEDenum = (BBoxLEDenum)Enum.Parse(typeof(BBoxLEDenum), LEDlabel);
                        object oLEDenum = LEDenum;
                        channelPlaybackLEDs.Add((Enum)oLEDenum);
                    }
                }

                // FPGA register defined as "N-1" channels set (see SWUG): i.e., to enable only 1st channel, write "0"
                // You cannot "activate" higher channel number without also activating all lower channels
                // i.e. per hardware, you cannot activate, say, channel 5 without also activating channels 0-4.
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("DAC_DMA_Chans_Active", MaxChannelOrdinal - 1);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("DAC_DMA_Chans_Active", ref RegValue);
                ReturnStrings.Add(string.Format("DAC_DMA_Chans_Active: {0}", RegValue.ToString()));
            }
            ReturnStrings.Add(sFunctionStatus); // last string is always final function status
            return ReturnStrings;
        }


        // Function to bundle the entire collection of test functions to configure
        // Galvo-Galvo image scan with test waveforms, using single argument of
        // image dimension
        static void SetConsoleStrings(List<String> FunctionStrings, bool bVerbose, ref List<String> retStrings)
        {
            if (bVerbose) // add all the strings
            {
                foreach (string Str in FunctionStrings)
                {
                    retStrings.Add(Str);
                }
            }
            else
                retStrings.Add(FunctionStrings.Last()); // add only the SUCCESS/FAIL result
        }



        // Thread to wait on FPGA User Interrupt
        public void WaitForFPGAUserIRQ()
        {
            THORDAQ_STATUS status = ThorDAQAPIWaitForUserIRQ(MasterBoardIndex, ref FPGAUserInterruptCountByChan[0], ref FPGAUserInterruptCountByChan[1], ref FPGAUserInterruptCountByChan[2], ref FPGAUserInterruptCountByChan[3]);
        }


        List<string> ImageScanTest(List<String> argumentsList)
        {

            List<string> ReturnStrings = new List<string> { };
            List<string> ConsoleStrings = new List<string> { };

            bool bVerbose = true; // for testing, TRUE
            bool bResonantScan = false;
            int argNum;
            string FrameDimensionArg = "512x512";

            int PixelsInLine = 0, TotalLines = 0;
            String[] FrameDimension;
            for (argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-r": // Resonant-Galvo mode
                        bResonantScan = true;
                        break;

                    case "-d":  // get frame dimensions
                        FrameDimensionArg = argumentsList[argNum + 1]; 
                        FrameDimension = argumentsList[argNum + 1].Split('x');
                        Int32.TryParse(FrameDimension[0], out PixelsInLine);
                        Int32.TryParse(FrameDimension[1], out TotalLines);
                        argNum++; // next flag
                        break;
                    case "-v": // verbose
                        bVerbose = true;
                        break;
                    default:
                        break;
                }
            }

            // now call the separate functions in series
            List<string> ArgList = new List<string> { "InternalCmd " };  // i.e., just arg0 means no args
            GlobalScan(new List<string> { "InternalCmd", "-s" }, null); // make sure ThorDAQ global scan stopped
                                                                        // setup the ThorDAQ hardware...
                                                                        // argument list for setup calls
            ReturnStrings = ADCSampleClock_setup(ArgList);
            SetConsoleStrings(ReturnStrings, bVerbose, ref ConsoleStrings);
            string sScanType = (bResonantScan == true) ? "-r" : "";

            ReturnStrings = ADCStream_setup(new List<string> { "InternalCmd ", "-j", sScanType }); // MUST reset JESD204B once after system restart
            SetConsoleStrings(ReturnStrings, bVerbose, ref ConsoleStrings);

            // NOTE!!  -e option means external hardware trigger needed to start image scan!
            String sMode = (bResonantScan == true) ? "0" : "1"; // ) is Resonant-Galvo, 1 is Galvo-Galvo
            ReturnStrings = ScanControl_setup(new List<string> { "InternalCmd ", "-m", sMode,"-l", "800", "-f", "600", "-P", "1", "-p", "206" });
         //   ReturnStrings = ScanControl_setup(new List<string> { "InternalCmd ", "-m", sMode, "-P", "1", "-p", "206" });
            SetConsoleStrings(ReturnStrings, bVerbose, ref ConsoleStrings);
            ReturnStrings = S2MM_DMA_setup(new List<string> { "InternalCmd", "-d", FrameDimensionArg, "-c", "0xF", "-n", "1" });   // (Image DMA not required for DAC waveforms...)
            SetConsoleStrings(ReturnStrings, bVerbose, ref ConsoleStrings);
            ArgList = new List<string> { "InternalCmd", "-c", "0xFFF", "-z", "1000000", "-f", "c:\\temp\\DACwave_1240Hz_1MHz_250ms_a65500.txt" }; // always 1MHz sample rate, all channels
            ReturnStrings = DACWaveform_setup(ArgList);
            SetConsoleStrings(ReturnStrings, bVerbose, ref ConsoleStrings);

            // Send WAIT for IRQ IOCTL which enables FPGA user interrupt (which DDR3 image DMA complete)
            Thread WaitOnFPGAintThread = new Thread(new ThreadStart(WaitForFPGAUserIRQ));
            WaitOnFPGAintThread.Start();

            // to emulate TILS, set up a thread which does IOCTL call to wait on FPGA
            // "user" interrupt indicating a new frame is ready, then read the DDR3 image data
            // and increment frame counter
            //hereeee
            GlobalScan(new List<string> { "InternalCmd", "-r" }, null); // START the image scan

            Thread.Sleep(1750);

            GlobalScan(new List<string> { "InternalCmd", "-s" }, null); // START the image scan
            THORDAQ_STATUS status = ThorDAQAPICancelWaitForUserIRQ(MasterBoardIndex);

            UsrIntCount.Content = FPGAUserInterruptCountByChan[0].ToString(); // update GUI

            return ConsoleStrings;
        }

        // make calls through new API, which in DLL detects Legacy vs. 3U Panel BOB
        static bool SetThorImageLS_BOBConfig()
        {
            bool bStatus;

            // configure minimum Global Registers needed to run
            // route the Digital (DBB1) I/O signals
            // MATCH ThorImageLS with DIO1 12   digit_output_2 (software)               IDevice.TD_DIO_MUXedSLAVE_PORTS.Digital_Output_2
            //                        DIO2 13   digit_output_3 (software)               TD_DIO_MUXedSLAVE_PORTS .Digital_Output_3
            //                        DIO3 9    pixel clock pulse output                TD_DIO_MUXedSLAVE_PORTS .Pixel_clock_pulse_output
            //                        DIO4 10   digit_output_0 (software)               TD_DIO_MUXedSLAVE_PORTS .Digital_Output_0
            //                        DIO5 0    resonant scanner line trigger input     TD_DIO_MUXedSLAVE_PORTS .Resonant_scanner_line_trigger_input
            //                        DIO6 7    external hardware frame trigger input   TD_DIO_MUXedSLAVE_PORTS .Hardware_trigger_input
            //                        DIO7 3    scan direction                          TD_DIO_MUXedSLAVE_PORTS .Start_of_frame_output [replace "3" for ManufTest]
            //                        DIO8 11   digit_output_1 (software)               IDevice.TD_DIO_MUXedSLAVE_PORTS .Digital_Output_1

            // read the current BOB config table and setup BOB hardware according to Legacy or 3U

            THORDAQ_STATUS status = 0;
            List<String> strDIOlist = new List<string> { };

            IntPtr UnManagedConfigBuffer = Marshal.AllocHGlobal((int)TD_BOBDIODef.NumBOB_DIOs * (int)TD_BOBDIODef.CharPerBOB_DIO);
            char[] DIOconfig = new char[(int)TD_BOBDIODef.CharPerBOB_DIO];

            status = ThorDAQAPIGetDIOConfig(MasterBoardIndex, UnManagedConfigBuffer, ((int)TD_BOBDIODef.NumBOB_DIOs * (int)TD_BOBDIODef.CharPerBOB_DIO)); // DIOconfig is DLL char (e.g. [32][6] array) - send our size

            if (status == THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                int i, j;
                for (i = 0; i < (int)TD_BOBDIODef.NumBOB_DIOs; i++)
                {
                    for (j = 0; j < (int)TD_BOBDIODef.CharPerBOB_DIO; j++)
                    {
                        DIOconfig[j] = (char)Marshal.ReadByte(UnManagedConfigBuffer, (i * (int)TD_BOBDIODef.CharPerBOB_DIO + j));
                    }
                    // convert the array into strings for easier display
                    strDIOlist.Add(new string(DIOconfig));
                }
            }

            // we now have the entire CURRENT DIO config definition - write the entire configuration back,
            // one DIO at a time
            
            byte[] bytes;
            int iDIOsConfigured = 0;
            int k;
            for ( k= 15 /*(int)TD_BOBDIODef.NumBOB_DIOs-1*/; k >=0; k--) // for each of the 32 3U DIOs (or 8 for Legacy DIOs)
            {
                bytes = Encoding.ASCII.GetBytes(strDIOlist[k]);
                int Idx = 0;
                foreach (char Chr in strDIOlist[k])
                {
                    Marshal.WriteByte(UnManagedConfigBuffer, Idx++, (byte)Chr);
                }
                status = ThorDAQAPISetDIOConfig(MasterBoardIndex, UnManagedConfigBuffer, (int)TD_BOBDIODef.CharPerBOB_DIO); // DIOconfig is DLL char (e.g. [32][] array) - send our size
                if (status == THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    ++iDIOsConfigured;
                }
                Thread.Sleep(1);
            }
            bStatus = (iDIOsConfigured >= 8) ? true : false;
            return bStatus;
        }


        const string HELP_GlobalScan_MSSAGE = @"GlobalScan: <-r> <-s> <-d WxH>
Options:
  -n  ScanningFrameCount (number of frames to capture (def. 0xFFFF inf.)
  -r  Run (set Global STOP_RUN control bit), default
  -s  Stop (possible to exit program with Global 'scan' running, but not recommended)
  -d  Width by Height dimension, e.g. '-d 32x16' means 32-pixels by 16 lines, no 'default' defined
  -T  Trigger NI-1071's PFI0 (through BOB's D31)
  -e  External Hardware Trigger 'GC_HW_TriggerMode' asserted (is this actually used by FPGA?)
"
 ;
        // the BBoxLEDs list can be NULL; if not, it designates which LEDs to turn ON or OFF with Scan
        static List<string> GlobalScan(List<String> argumentsList, List<Enum> BBoxLEDs)
        {
            List<string> ReturnStrings = new List<string> { };
            bool bRun = true;
            bool bTriggerNI1071 = false;
            int argNum;
            int PixelsInLine = 0, TotalLines = 0;
            string[] FrameDimension;
            UInt64 ScanFrameCount = 0xFFFF;
            UInt64 HW_TriggerMode = 0; // def. off

            for (argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-e":
                        HW_TriggerMode = 1;
                        break;
                    case "-T":
                        bTriggerNI1071 = true;
                        break;
                    case "-n":
                        UInt64.TryParse(argumentsList[argNum + 1], out ScanFrameCount);
                        argNum++; // next flag
                        break;

                    case "-d":  // get frame dimensions
                        FrameDimension = argumentsList[argNum + 1].Split('x');
                        Int32.TryParse(FrameDimension[0], out PixelsInLine);
                        Int32.TryParse(FrameDimension[1], out TotalLines);
                        argNum++; // next flag

                        // go ahead and set registers because there is no "default" frame dimensions required for ScanControl
                        ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageHSIZE", (ulong)PixelsInLine - 1);
                        ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageAcqHSIZE", (ulong)PixelsInLine - 1); // assume single Plane!
                        ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageVSIZE", (ulong)TotalLines - 1);

                        break;

                    case "-s":  // stop waveform playback
                        bRun = false;
                        break;
                    case "-r":  // start waveform playback
                        bRun = true;
                        break;

                }
            }
            // check args
            // if WxH not already set in FPGA, and not given in command arg, fatal exit
            ulong RegValue = 0;
            bool bStatus;

            // count of requested frames (0xFFFF is "infinite")
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ScanningFrameCount", ScanFrameCount); // 1 = on 1st LineSync after Start
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ScanningFrameCount", ref RegValue);
            ReturnStrings.Add(string.Format("ScanningFrameCount: {0}", RegValue.ToString()));


            if (bRun == false) // STOP
            {
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GIGCR0_STOP_RUN", 0); // 0 = stop
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GIGCR0_STOP_RUN", ref RegValue);
                ReturnStrings.Add(string.Format("GIGCR0_STOP_RUN: {0}", RegValue.ToString()));

                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GIGCR0_LED2", 0); // 1 = on
                if (BBoxLEDs != null)
                {
                    foreach (Enum LEDenum in BBoxLEDs)
                    {
                        object enumObject = LEDenum;
                        int intValue = (int)enumObject;
                        uint uiStatus;
                        Byte State = 0; // off
                        {
                            uiStatus = ThorDAQAPIBreakOutBoxLED(MasterBoardIndex, intValue, State); // LED OFF if activated
                        }
                    }
                }
            }
            else //  START (RUN)
            {
                if (PixelsInLine == 0)
                {
                    // user wants existing HSIZE/VSIZE
                    ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageHSIZE", ref RegValue);
                    if (RegValue == 0) // undefined dimensions - exit
                    {
                        ReturnStrings.Add("FAIL: GlobalImageHSIZE/VSIZE not specified");
                        return ReturnStrings;
                    }
                    else
                    {
                        PixelsInLine = (int)RegValue + 1;
                        ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageVSIZE", ref RegValue);
                        TotalLines = (int)RegValue + 1;
                    }
                }

                bool bMUXset = SetThorImageLS_BOBConfig(); // (Legacy or 3U) if false, DIO config may be incorrect

                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalDBB1muxReg", ref RegValue);
                ReturnStrings.Add(string.Format("GlobalDBB1muxReg: 0x{0}", RegValue.ToString("X16")));

                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalABBXmuxReg", ref RegValue);
                ReturnStrings.Add(string.Format("GlobalABBXmuxReg: 0x{0}", RegValue.ToString("X16")));


                // Image dimensions (must be set even when NOT acquiring the S2MM DDR3 image DMA
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageHSIZE", (ulong)(PixelsInLine - 1)); // pixels-1
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageAcqHSIZE", (ulong)PixelsInLine - 1); // assume single Plane!
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageHSIZE", ref RegValue);
                ReturnStrings.Add(string.Format("GlobalImageHSIZE: {0}", RegValue.ToString()));

                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageVSIZE", (ulong)(TotalLines - 1)); // lines-1
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageVSIZE", ref RegValue);
                ReturnStrings.Add(string.Format("GlobalImageVSIZE: {0}", RegValue.ToString()));

                // Trigger Mode
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GC_HW_TriggerMode", HW_TriggerMode); // 0 = off
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GC_HW_TriggerMode", ref RegValue);
                ReturnStrings.Add(string.Format("GC_HW_TriggerMode: {0}", RegValue.ToString()));

                // control LEDs
                ///ulong AOchannelMaskShift = channelPlaybackMask; // use the static global configured in DACwaveform
                ///
                if (BBoxLEDs != null)
                {
                    Byte LEDstate = 1;
                    foreach (Enum LEDenum in BBoxLEDs)
                    {
                        object enumObject = LEDenum;
                        int intValue = (int)enumObject;
                        uint uiStatus;
                        {
                            uiStatus = ThorDAQAPIBreakOutBoxLED(MasterBoardIndex, intValue, LEDstate); // LED ON if activated
                        }
                    }
                }
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GIGCR0_LED2", 1); // 1 = on

                // START the "image scan", which may or may not include both useful DAC wave output and ADC input
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GIGCR0_STOP_RUN", 1); // 1 = start
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GIGCR0_STOP_RUN", ref RegValue);
                ReturnStrings.Add(string.Format("GIGCR0_STOP_RUN: {0}", RegValue.ToString()));

                if (bTriggerNI1071)
                {
                    // Configure the PFI0 trigger (e.g. D31), strictly hardware based
                    // Labview responds on rising edge, starting a sine waveform for our PMT channel
                    MTAPISetDIOConfig(new List<string> { "31", "o", "31", "48" });
                    MTAPISetDO(new List<string> { "D31", "1" });
                }
                if (bTriggerNI1071)
                {
                    MTAPISetDO(new List<string> { "D31", "0" });
                }

            }  // end of RUN
            ReturnStrings.Add("SUCCESS");
            return ReturnStrings;
        }


        string APIReadProdSN(List<String> argumentsList)
        {
            // add the "-r" arg for Read
            argumentsList.Add("-r");
            argumentsList.Add("-S"); // Sterling manufacturing format (3 digit model, 10 digit production, 6 digit SN)
            filespec = @"C:\temp\TD_ProdSN.txt"; // Q4 2022 Sterling product code format            filespec = @"C:\temp\ThorDAQeeproms.txt";
            return APIReadWriteEEprom(argumentsList);
        }
        string APIWriteProdSN(List<String> argumentsList)
        {
            // 
            argumentsList.Add("-w"); // write
            argumentsList.Add("-S"); // Sterling manufacturing format (3 digit model, 10 digit production, 6 digit SN)
            argumentsList.Add("-p"); // EEPROM "page" len (def. 16)
            argumentsList.Add("16");
            filespec = @"C:\temp\TD_ProdSN.txt"; // Q4 2022 Sterling product code format
            return APIReadWriteEEprom(argumentsList);
        }



        const string HELP_APIReadWriteEEPROM_MSSAGE = @"APIReadEEPROM <-f> <-c COMP>
APIWriteEEPROM <-c COMP String> [default - write all with file contents]
  where COMP is a 4-digit alphanumeric ThorDAQ component:
    TD   
    ADC
    DBB1
    DAC
    ABB1
    ABB2
    ABB3
    LFT
  and String:
    TD002025-239044-0002-10/02/2018 (ex. TD main board)
    DB002025-b30274-0010-07/01/2019 (ex. DAC card)
    TR002025-g39044-0012-01/01/2022 (ex. 3P trigger card)
    AB202025-x39044-0012-01/01/2022 (ex. ABB2 BOB)
Ex.
    APIWriteEEPROM -ABB3 AB202025-x39044-0012-01/01/2022

String fields:  
    TD: 2 or 3-digit component code
    002025:  the ThorDAQ main board Serial Num
    -2       for main board, indicates TDQ1, 2, 3, etc.
             for mezzanine cards, 'x' means untested/unknown, 'g' or 'b' means last tested good or bad on ThorDAQ S/N
    -n39044  board manufacturer batch number
    -0002    board manufacturer serial num in batch
    -10/02/2018  board date of manufacture
"
;
        // API function to replace legacy I2C function
        // We define these 5-char fields in the EEPROM.txt file (for production reference)
        // 0 "TD   "
        // 1 "ADC  "
        // 2 "DBB1 "
        // OMIT CPLD on DAC because it makes no sense! (Change Legacy operation)
        // 3 "DAC  "
        // 4 "ABB1 "
        // 5 "ABB2 "
        // 6 "ABB3 "
        // 7 "LFT  "

        string filespec;

        string APIReadEEprom(List<String> argumentsList)
        {
            // add the "-r" arg for Read
            argumentsList.Add("-r");
            filespec = @"C:\temp\ThorDAQeeproms.txt";
            return APIReadWriteEEprom(argumentsList);
        }
        void APIWriteEEprom(List<String> argumentsList)
        {
            // add the "-w" arg for Write
            argumentsList.Add("-w");
            argumentsList.Add("-p"); // EEPROM "page" len (def. 16)
            argumentsList.Add("16");
            filespec = @"C:\temp\ThorDAQeeproms.txt";
            APIReadWriteEEprom(argumentsList);
        }

        string APIReadWriteEEprom(List<String> argumentsList)
        {
            THORDAQ_STATUS status = THORDAQ_STATUS.STATUS_SUCCESSFUL;
            uint TotalDataPlusOpcodesByteCount;
            var fileFormatStrings = new List<string>(); // human-readable format
            uint MasterMUXAddr, MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, OpcodeByteLen, TransferBufferLen;
            int SlaveMUXAddr, PageReadWriteLen = 16;
            byte[] OpcodeBytes = new byte[] { 0xC0, 0x0, 0x0, 0x0 }; // max opcode count by ThorDAQ definition
            byte[] DataBytes = new byte[TOT_SERIAL_SZ]; // expected EEPROM data size by ThorDAQ definition
            IntPtr UnManagedOpCodeBuffer = Marshal.AllocHGlobal(OpcodeBytes.Length);
            IntPtr UnManagedDataBuffer = Marshal.AllocHGlobal(DataBytes.Length);
            ASCIIEncoding ascii = new ASCIIEncoding();  // for converting EEPROM bytes to ASCII
            bool bI2C_SlaveRead = true, bCreateFile = false;
            bool bSterlingFormat = false;
            int TDcomponentIndex = -1; // default, all
            ushort TDboardIndex = MasterBoardIndex; // which PCIe board? (def. global selector)
            UInt32 I2CbusHz = 400; // EEPROMs run at higher speed

            string[] EEPROMcontentsToWrite = { "TD   ", "ADC  ", "DBB1 ", "DAC  ", "ABB1 ", "ABB2 ", "ABB3 ", "LFT  " };
            string sFormattedBuf = " "; // return string

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-b": // [option to add] next arg PCIe Board index (def. 0)
                        TDboardIndex = (ushort)new System.ComponentModel.Int16Converter().ConvertFromString(argumentsList[++argNum]);
                        break;
                    case "-p": // [option to add] next arg as pagelen of hardware EEPROM
                        PageReadWriteLen = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[++argNum]);
                        break;
                    case "-c": // [option to add] single component (def. entire ThorDAQ system)
                        TDcomponentIndex = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[++argNum]);
                        break;
                    case "-S":
                        bSterlingFormat = true;
                       
                        break;
                    case "-f": // [option to add] next arg as filename
                        bCreateFile = true;
                        break;
                    case "-w": // 
                        bI2C_SlaveRead = false;
                        break;
                    default:
                        break;
                }
            }

            string[] EEpromStringsToWrite = new string[9];  // space for all TD components
            // if we are WRITING, get contents from the file...
            if (bI2C_SlaveRead == false)
            {
                EEpromStringsToWrite = File.ReadAllLines(filespec);
            }

            // Initial hardware based constants
            MasterMUXAddr = 0x71;
            SlaveMUXAddr = 0x70;
            int iBoardComponentStart, iBoardComponentEnd = 7;
            if (TDcomponentIndex == -1) // do all components
            {
                iBoardComponentStart = 0;
                // what TDQ model?  TD1, TD3, etc.?
                string SNstring;
                if( ThorDAQAdapters[MasterBoardIndex].TDnType == -1)
                {
                    // Attempt to discover TDn type
                    if (GetI2CeepromSerNum(MasterBoardIndex, 0x80, 0xFF, 0x54, 0, out SNstring))
                    {
                        try
                        {
                            int iTDQtype = Int32.Parse(SNstring.Substring(2, 1));
                            // if card is TDQ1 it's impossible to have 3P slot
                            ThorDAQAdapters[MasterBoardIndex].TDnType = iTDQtype;
                        }
                        catch( Exception )  // do nothing - EEPROM may be unitialized
                        {

                        }
                    }

                }
                if(ThorDAQAdapters[MasterBoardIndex].TDnType == 1) // if we KNOW it's a TDQ1, 
                    iBoardComponentEnd = 6; // ignore LFT slot

            }
            else
            {
                iBoardComponentStart = TDcomponentIndex;
                iBoardComponentEnd = TDcomponentIndex;

            }

            for (int iBoardComponent = iBoardComponentStart; iBoardComponent <= iBoardComponentEnd; iBoardComponent++)
            {
                TransferBufferLen = (uint)DataBytes.Length;
                OpcodeBytes[0] = 0x0; // starting address in ALL EEPROMs (default - note CPLD command exception)
                OpcodeByteLen = 1;    // default single byte address for EEPROMs
                switch (iBoardComponent)
                {
                    case 0: // TDboard
                        MasterMUXChan = 0x80;
                        SlaveMUXChan = 0xFF;
                        TargetSlaveAddr = 0x54;
                        break;
                    case 1: // ADC
                        MasterMUXChan = 0x02;
                        SlaveMUXChan = 0xFF;
                        TargetSlaveAddr = 0x54;
                        break;
                    case 2: // DBB1
                        MasterMUXChan = 0x02;
                        SlaveMUXChan = 0xFF;
                        TargetSlaveAddr = 0x50;
                        break;
                    case 3: // DAC 
                        MasterMUXChan = 0x08;
                        SlaveMUXChan = 0x04;
                        TargetSlaveAddr = 0x54;
                        break;
                    case 4: // ABB1 
                        MasterMUXChan = 0x08;
                        SlaveMUXChan = 0x01;
                        TargetSlaveAddr = 0x50;
                        break;
                    case 5: // ABB2
                        MasterMUXChan = 0x08;
                        SlaveMUXChan = 0x02;
                        TargetSlaveAddr = 0x50;
                        break;
                    case 6: // ABB3 
                        MasterMUXChan = 0x08;
                        SlaveMUXChan = 0x08;
                        TargetSlaveAddr = 0x50;
                        break;
                    case 7: // 3P mez. card  (if present)
                        MasterMUXChan = 0x01;
                        SlaveMUXChan = 0xFF;
                        TargetSlaveAddr = 0x50;
                        break;
                    default:
                        MasterMUXChan = 0x80;
                        SlaveMUXChan = 0xFF;
                        TargetSlaveAddr = 0x54;
                        break;
                }

                // write OpCode(s) to UnManaged Opcode Mem
                for (int byteIndx = 0; byteIndx < OpcodeBytes.Length; byteIndx++)
                {
                    System.Runtime.InteropServices.Marshal.WriteByte(UnManagedOpCodeBuffer, byteIndx, OpcodeBytes[byteIndx]);
                }
                if (bI2C_SlaveRead == true)
                {
                    TotalDataPlusOpcodesByteCount = TransferBufferLen;
                    status = (THORDAQ_STATUS)ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead, MasterMUXAddr, MasterMUXChan,
                                                                                    SlaveMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, PageReadWriteLen,
                                                                                    UnManagedOpCodeBuffer, ref OpcodeByteLen,  // OpcodeLen >0 means READ operation
                                                                                    UnManagedDataBuffer, ref TotalDataPlusOpcodesByteCount); // returns ALL bytes transfered
                }
                else  // WRITING!  contents came from file
                {
                    // now discard the 5 char prefix
                    EEpromStringsToWrite[iBoardComponent] = EEpromStringsToWrite[iBoardComponent].Remove(0, 5);
                    // discard the added format "-" chars
                    EEpromStringsToWrite[iBoardComponent] = EEpromStringsToWrite[iBoardComponent].Replace("-", "");
                    // convert ASCII chars to hex bytes
                    char[] EEPromBytes = new char[64];
                    EEPromBytes = EEpromStringsToWrite[iBoardComponent].ToCharArray();

                    int Ofset = 0;
                    OpcodeByteLen = 1;  // i.e., starting EEPROM address of 0x0
                    if ( bSterlingFormat == true)  // write 3 fields into EEPROM area
                    {
                        int SterlingFldIndx;
                        // initialize ENTIRE EEPROM field to 0xFF, then replace the Sterling Product-code fields
                        while (Ofset < TOT_SERIAL_SZ)
                        {
                            System.Runtime.InteropServices.Marshal.WriteByte(UnManagedDataBuffer, Ofset++, 255);
                        };
                        // 3 byte Component done
                        for( Ofset = 0, SterlingFldIndx = 0; Ofset < 3; Ofset++, SterlingFldIndx++)
                        {
                            System.Runtime.InteropServices.Marshal.WriteByte(UnManagedDataBuffer, Ofset, (byte)EEPromBytes[SterlingFldIndx]);
                        }
                        // 10 byte Product code
                        for (Ofset = 28, SterlingFldIndx = 3; Ofset < 38; Ofset++, SterlingFldIndx++)
                        {
                            System.Runtime.InteropServices.Marshal.WriteByte(UnManagedDataBuffer, Ofset, (byte)EEPromBytes[SterlingFldIndx]);
                        }
                        // 6 byte SerNum 
                        for (Ofset = 38, SterlingFldIndx = 13; Ofset < 44; Ofset++, SterlingFldIndx++)
                        {
                            System.Runtime.InteropServices.Marshal.WriteByte(UnManagedDataBuffer, Ofset, (byte)EEPromBytes[SterlingFldIndx]);
                        }

                        TotalDataPlusOpcodesByteCount = TransferBufferLen;  // initialized to DATA len - but RETURNS a total len transfered including required OpCodes (i.e. by PageSize)
                        status = (THORDAQ_STATUS)ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead,
                            MasterMUXAddr, MasterMUXChan,
                            SlaveMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, PageReadWriteLen,
                            UnManagedOpCodeBuffer, ref OpcodeByteLen,
                            UnManagedDataBuffer, ref TotalDataPlusOpcodesByteCount); // returns ALL bytes transfered  


                    }
                    else  // write entire EEPROM (e.g. 64 bytes) 
                    {
                        foreach (byte CharByte in EEPromBytes)
                        {
                            // e.g., start of TD main board Data buffer to send, 0x54 0x44 0x30 ... ("TDn...")
                            System.Runtime.InteropServices.Marshal.WriteByte(UnManagedDataBuffer, Ofset++, CharByte);
                        }
                        // pad unused bytes up to total len (64?)
                        while (Ofset < TOT_SERIAL_SZ)
                        {
                            System.Runtime.InteropServices.Marshal.WriteByte(UnManagedDataBuffer, Ofset++, 255);
                        };

                        TotalDataPlusOpcodesByteCount = TransferBufferLen;  // initialized to DATA len - but RETURNS a total len transfered including required OpCodes (i.e. by PageSize)
                        status = (THORDAQ_STATUS)ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead,
                            MasterMUXAddr, MasterMUXChan,
                            SlaveMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, PageReadWriteLen,
                            UnManagedOpCodeBuffer, ref OpcodeByteLen,
                            UnManagedDataBuffer, ref TotalDataPlusOpcodesByteCount); // returns ALL bytes transfered  
                    }
                }
                string sWriteDir = (bI2C_SlaveRead == true) ? "READ" : "WRITE";
                if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    UpdateConsoleStatus(" ThorDAQAPIXI2CReadWrite() I2C " + sWriteDir + " failure @M:" + MasterMUXChan.ToString("X") + " S:" + SlaveMUXChan.ToString("X") + " TargetAddr: 0x" + TargetSlaveAddr.ToString("X") + 
                                        "  " + status.ToString() + " Bytes: " + TotalDataPlusOpcodesByteCount.ToString());
          //          fileFormatStrings.Add(EEPROMcontentsToWrite[iBoardComponent] + "ERROR: I2C transfer error, status 0x" + status.ToString("X") + " ByteXferLen = " + TotalDataPlusOpcodesByteCount.ToString());
                }
                else  // format/display the EEPROM string
                {
                    // bring unmanaged buffer (the C++ passed DATA array) into C# managed...
                    for (int CPPbyteIndx = 0; CPPbyteIndx < TransferBufferLen; CPPbyteIndx++)
                    {
                        DataBytes[CPPbyteIndx] = Marshal.ReadByte(UnManagedDataBuffer, CPPbyteIndx);
                    }
                    // convert hex digits to ASCII
                    string rawBuf = ascii.GetString(DataBytes);
                    sFormattedBuf = " ";
                    if (EEPROMcontentsToWrite[iBoardComponent] != "CPLD2")
                    {
                        sFormattedBuf = EEPROMcontentsToWrite[iBoardComponent] + FormatSerStr(rawBuf, bSterlingFormat);
                    }
                    // special cases... [ MOVE CPLD versions away from EEPROM functions ]
                    else if (EEPROMcontentsToWrite[iBoardComponent] == "CPLD2")
                    {
                        sFormattedBuf = EEPROMcontentsToWrite[iBoardComponent] + string.Format("Read-only Ver. 0x{0}.{1}.{2}.{3}",
                            DataBytes[0].ToString("X"), DataBytes[1].ToString("X"), DataBytes[2].ToString("X"), DataBytes[3].ToString("X"));
                    }
                    fileFormatStrings.Add(sFormattedBuf);
                }
            }
            if (bI2C_SlaveRead == true)
            {
                foreach (string Line in fileFormatStrings)
                {
                    UpdateConsoleStatus(Line);
                }
            }
            if (bCreateFile)
            {
                File.WriteAllLines(filespec, fileFormatStrings);
                UpdateConsoleStatus("EEPROM contents written to " + filespec);
            }
            Marshal.FreeHGlobal(UnManagedOpCodeBuffer);
            Marshal.FreeHGlobal(UnManagedDataBuffer);
            return sFormattedBuf;
        }


        const string HELP_DDR3_MemTest_MSSAGE = @"APIDDR3MemTest
Test DDR3 memory module (rudimentary test)
Options:
"
 ;
        void APIDDR3MemTest(List<String> argumentsList)
        {
            ThorDAQmainBoardTestAndReport TDmemTest = new ThorDAQmainBoardTestAndReport(MasterBoardIndex);
            string[] sStatus = new string [] { };
            sStatus = TDmemTest.TestDDR3_Memory(2, 0x0);
            UpdateConsoleStatus("Testing @0x0: " + sStatus[1]);

            sStatus = TDmemTest.TestDDR3_Memory(2, 0x05000000);
            TDmemTest.TestDDR3_Memory(2, 0x05000000);
            UpdateConsoleStatus("Testing @0x05000000: " + sStatus[1]);


        }
        string HELP_APIGetSetAIOConfig_MSSAGE = @"APIGetAIOConfig APISetAIOConfig  
APIGetAIOConfig 
APISetAIOConfig BNCindx Dir Pol <newBNCorRange>
  BNCindx - 0-based index of the BNC DIO (e.g., 2 is D2 for 3U, DIO3 for Legacy)
  Dir     - 'I' for input, 'O' for Output (NOT case sensitive)
  Pol     - 'B' for bi-polar (+/- voltage), 'U' for unipolar
  O-R     - for Output Dir, required field for DMAch switch; Input, optional Full Scale range of 5 (def. is 10 VDC)
Example use: 
APISetAIOConfig 6 I B 10   : BNC AI6 (N/A to Legacy), Input, 'b'ipolar, +/- '10' VDC range
APISetAIOConfig 13 i u 5   : BNC AI13 (N/A to Legacy), Input, 'u'nipolar,  5 VDC range (neg. volts ret. 0)
APISetAIOConfig 3 o b 9    : Switch DAC waveform that was BNC AO03 to AO09 (changes TWO AOnn definitions!)
"
;

        void APISetAIOConfig(List<String> argumentsList)
        {
            string strAIOconfig;
            int iBNClabel;
            Int32 iNum;
            int range = 10;
            int MUX_DMAchan = 0xFF; // invalid default
            string sPolarity;

            if (argumentsList.Count < 4) // at least <BNCIndex> <Dir> <Polarity> 
            {
                UpdateConsoleStatus(HELP_APIGetSetAIOConfig_MSSAGE);
                return;
            }

            string sDIR = argumentsList[2].ToUpper();
            if (sDIR == "O" || sDIR == "o") // Required arg for OUTPUT?
            {
                if (argumentsList.Count < 5) // at least <BNCIndex> <Dir> <Polarity> <MUX_DMAchan>
                {
                    UpdateConsoleStatus(HELP_APIGetSetAIOConfig_MSSAGE);
                    return;
                }
                MUX_DMAchan = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[4]);
            }

            sPolarity = argumentsList[3].ToUpper(); // Unipolar or Bipolar (single char)

            //       01234567890
            // e.g. "AnnXDVmmMcc", where	"n" is 0-11 (BNC label "nn", e.g. "A12" or "A03"),
            //							"X" is Input/Output direction "I" or "O", 
            //							"D" is "U"nipolar or "B"ipolar, 
            //							"V" is Volts "mm" range e.g. "10"
            //							"M" is (Output only) MUX'd DAC Waveform DMAcc [channel (00-11)] location for BNC label in FPGA's GlobalABBXmuxReg (all BOBs)            iNum = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[1]);
            iNum = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[1]);
            iBNClabel = iNum;
            strAIOconfig = String.Format("A{0,2:D2}", iNum); // 'D' plus two chars
            strAIOconfig += sDIR + sPolarity ; // single DIR char

            strAIOconfig += String.Format("V{0,2:D2}", range);
            // for OUTPUT, we have 5th arg (the MUX)
            if (argumentsList.Count >= 5)  // 4th arg is Range
            {
                strAIOconfig += String.Format("M{0,2:D2}", MUX_DMAchan);
            }

            // Must send as "unmanaged" char array
            IntPtr UnManagedConfigBuffer = Marshal.AllocHGlobal((int)TD_BOBAIDef.NumBOB_AIs); // (includes NULL byte)
            //byte[] bytes = Encoding.ASCII.GetBytes(strDIOconfig);
            int Idx = 0;
            foreach (char Chr in strAIOconfig)
            {
                Marshal.WriteByte(UnManagedConfigBuffer, Idx++, (byte)Chr);
            }

            THORDAQ_STATUS status;
            status = ThorDAQAPISetAIOConfig(MasterBoardIndex, UnManagedConfigBuffer, (int)TD_BOBAIDef.CharPerBOB_AI); // DIOconfig is DLL char (e.g. [32][9] array) - send our size
            if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                if (status == THORDAQ_STATUS.STATUS_DEVICE_NOT_EXISTS)
                    UpdateConsoleStatus(String.Format("ERROR: BOB hardware does not support BNC D{0} specified connection", iBNClabel));
                else
                    UpdateConsoleStatus("ERROR: BOB hardware device not found on I2C bus - cable problem?");
            }
            Marshal.FreeHGlobal(UnManagedConfigBuffer); // release our temp buffer
        }

        // allows for configuring entire 3U BOB 31 DIOs as intput or output
        // sets the D0-D4 to GPIO AUX MUXes 27-31
        static bool TestDIOs(bool Output, int DIOgroup)  // DIOgroup 99 is D0-D3, 100 is D4-D7 
        {
            bool bStatus;
           
            List<string> retString = new List<string> { }; ;
            int i;
            
            bool bD0_D3 = (DIOgroup == 99) ? true : false ;

            // special cases of how to configure (for test) last 3 DIOs -
            // we can pick whatever is most usable for test...
            /*
            iResonant_scanner_line_trigger = 0x00,
            iExtern_line_trigger = 0x01,
            iExtern_pixel_clock = 0x02,
            oScan_direction = 0x03,
            oHorizontal_line_pulse = 0x04,
            oPixel_integration = 0x05,
            oStart_of_frame = 0x06,
            iFrame_hardware_trigger = 0x07,
            iExternal_SOF = 0x08,
            oPixel_clock_pulse = 0x09,
            oDigital_Waveform_0 = 0x0A, (10d)
            oDigital_Waveform_1 = 0x0B,
            oDigital_Waveform_2 = 0x0C,
            oDigital_Waveform_3 = 0x0D,
            oDigital_Waveform_4 = 0x0E,
            oDigital_Waveform_5 = 0x0F,
            oDigital_Waveform_6 = 0x10,
            oDigital_Waveform_7 = 0x11,
            oDigital_Waveform_8 = 0x12,
            oDigital_Waveform_9 = 0x13,
            oDigital_Waveform_10 = 0x14,
            oDigital_Waveform_11 = 0x15,
            oDigital_Waveform_12 = 0x16,
            oDigital_Waveform_13 = 0x17, // 23d
            oDigital_Waveform_14 = 0x18,
            oDigital_Waveform_15 = 0x19,
            oCapture_Active = 0x1A, // 26d
                            */
            // for the high speed DIOs, create a set of Input and Output "defaults"
            int[] iGPIOinputMUXD0_D3 = new int[8] { 27, 28, 29, 30, 1, 2, 8, 7 };
            int[] iGPIOinputMUXD4_D7 = new int[8] { 1, 2, 7, 8, 27, 28, 29, 30 };
            int[] iGPIOoutputMUXD0_D3 = new int[8] { 27, 28, 29, 30, 10, 11, 12, 13 };
            int[] iGPIOoutputMUXD4_D7 = new int[8] { 10, 11, 12, 13, 27, 28, 29, 30 };

            int AUXMUXindex = -1;
            string sIOdir = (Output == true) ? "O" : "I";
            for ( i=0; i< 32; i++)
            {
                if (i < 8)  // confuring "high speed" FPGA DIOs
                {
                    // configuring D0-D3 or D4-D7?
                    if( Output )
                    {
                        if (bD0_D3)
                        {
                            if (i < 4) AUXMUXindex++; else AUXMUXindex = -0;
                            retString = APISetDIOConfig(new List<string> { "InternalCmd", i.ToString(), sIOdir, i.ToString(), iGPIOoutputMUXD0_D3[i].ToString(), AUXMUXindex.ToString() });
                        }
                        else
                        {
                            if (i > 3) AUXMUXindex++; 
                            retString = APISetDIOConfig(new List<string> { "InternalCmd", i.ToString(), sIOdir, i.ToString(), iGPIOoutputMUXD4_D7[i].ToString(), AUXMUXindex.ToString() });
                        }
                    }
                    else // Inputs
                    {
                        if (bD0_D3)
                        {
                            if (i < 4) AUXMUXindex++; else AUXMUXindex = -0;
                            retString = APISetDIOConfig(new List<string> { "InternalCmd", i.ToString(), sIOdir, i.ToString(), iGPIOinputMUXD0_D3[i].ToString(), AUXMUXindex.ToString() });
                        }
                        else
                        {
                            if (i > 3) AUXMUXindex++;
                            retString = APISetDIOConfig(new List<string> { "InternalCmd", i.ToString(), sIOdir, i.ToString(), iGPIOinputMUXD4_D7[i].ToString(), AUXMUXindex.ToString() });
                        }
                    }
                }
                else // config the CPLD controlled DIOs
                { 
                    retString = APISetDIOConfig(new List<string> { "InternalCmd", i.ToString(), sIOdir, i.ToString() });
                    if (i >= 8 && (retString.Last() != "SUCCESS"))
                        retString[0] = "SUCCESS"; // ignore errors after 1st 8 DIO config successful (support Legacy DBB1)
                }
                if (retString.Last() != "SUCCESS") break;
            } // end FOR loop, all DIOs

            bStatus = (retString.Last() == "SUCCESS") ? true : false;
            return bStatus;
        }


        static string HELP_APIGetSetDIOConfig_MSSAGE = @"APIGetDIOConfig APISetDIOConfig  
APIGetDIOConfig 
APISetDIOConfig BNC dir <FPGAidx> {MUX} {AUX}
  chan    - 0-based index of the BNC DIO (e.g., 2 is D2 for 3U, DIO3 for Legacy)
  dir     - 'I' for input, 'O' for Output
  FPGAidx - 0-based index of FPGA (default 1:1 indexing); indexes 0-7 are the
            high speed wires from FPGA's DIO MUX register; 8-31 are 'slow' CPLD registers
            accesses via I2C
  MUX     - Enum of 'TD_DIO_MUXedSLAVE_PORTS'; defines hardware source config as 
            FPGA-type or CPLD (def. CPLD, code 48)
        iResonant_scanner_line_trigger = 0x00,
        iExtern_line_trigger = 0x01,
        iExtern_pixel_clock = 0x02,
        oScan_direction = 0x03,
        oHorizontal_line_pulse = 0x04,
        oPixel_integration = 0x05,
        oStart_of_frame = 0x06,
        iFrame_hardware_trigger = 0x07,
        iExternal_SOF = 0x08,
        oPixel_clock_pulse = 0x09,
        oDigital_Waveform_0 = 0x0A, (10d)
        oDigital_Waveform_1 = 0x0B,
        oDigital_Waveform_2 = 0x0C,
        oDigital_Waveform_3 = 0x0D,
        oDigital_Waveform_4 = 0x0E,
        oDigital_Waveform_5 = 0x0F,
        oDigital_Waveform_6 = 0x10,
        oDigital_Waveform_7 = 0x11,
        oDigital_Waveform_8 = 0x12,
        oDigital_Waveform_9 = 0x13,
        oDigital_Waveform_10 = 0x14,
        oDigital_Waveform_11 = 0x15,
        oDigital_Waveform_12 = 0x16,
        oDigital_Waveform_13 = 0x17, // 23d
        oDigital_Waveform_14 = 0x18,
        oDigital_Waveform_15 = 0x19,
        oCapture_Active = 0x1A, // 26d
        Aux_GPIO_0 = 0x1B,      // 27d
        Aux_GPIO_1 = 0x1C,  
        Aux_GPIO_2 = 0x1D,      // 29d
        Aux_GPIO_3 = 0x1E,
        Aux_GPIO_4 = 0x1F,
        BOB3U_GPIO = 0x30  (48d)
  AUX   GPIO AUX index for FPGA; use 0 for 27, 1 for 28 ...  (default is unused)
Sets or Returns the current DIO config - supports both Legacy DBB1 and 3U BOB 
(Break Out Box) for DIO 0 through 31 (DBB1 designated DIO1-8 for D0-7)
Outputs can be 'sourced' from other outputs, e.g. below, D2 configured as Output and
driven by the Output value of D7 (or DIO8 if DBB1)
3U BOB I/O is TTL logic levels ( ~ under 0.76 V '0',  over 1.4 V '1')
Inputs are not multiplexed
Example use: 
APISetDIOConfig  8 O 8 48   : BNC D8 (N/A to Legacy), Output, default 1:1 mapping, CPLD controlled
APISetDIOConfig  5 i 5 0   :  BNC D5, Input, FPGA high-speed index 5, config FPGA MUX 0 'iResonant_scanner_line_trigger' 
                              (typical config on Legacy BBox)
                              configured and controlled by CPLD (the NUX default, 48)
APISetDIOConfig  13 o 3 28 1: BNC D13 Output, FPGA MUX 28 @ index 3 (AUX GPIO index 1, second GPIO)
APISetDIOConfig  2 I 2 31 4 : BNC D2 (or DIO3) Input, FPGA AUX GPIO 31 (index 4) at FPGA index (line) 2, the fifth GPIO
                              (AUX GPIO index 4)
APISetDIOConfig  15 o 1 6   : BNC D15, Output, FPGA high-speed index 1, FPGA MUX definition 'oStart_of_frame'
APISetDIOConfig  14 i 4 0   : BNC D14, Input, FPGA high-speed index 4, config FPGA MUX 0 'iResonant_scanner_line_trigger' 
                              (iResonant_scanner_line_trigger FPGA Input)
APISetDIOConfig  16 I       : BNC D16, Input, CPLD register index 16, config MUX by CPLD (def. 48)
APISetDIOConfig 99 i        : Sets all DIOs to INPUT (or 'o' for Output) (testing only), and 
                              DIOs D0 - D3 as AUX GPIO 0-3 (MUX 27-30)
APISetDIOConfig 100 o       : Sets all DIOs to OUTPUT (or 'i' for input) (testing only), and 
                              DIOs D4 - D7 as AUX GPIO 0-3 (MUX 27-30)                             
"
;
        // in DLL, CHAR config[32][6]  e.g. "DnnXmmMxxAii", where "n" is 0-31 (BNC label D0 through "D31", "X" is
        // Input/Output direction "I" or "O", "mm" is Index of Output source, "M" is MUX code,
        //                        "Aii" is Aux GPIO index (only for TD_DIO_MUXedSLAVE_PORTS::Aux_GPIO_x codes 27-31)
        //       const int NumDIO = 32;
        //       const int CharsPerDIO = 9 + 1;  // field width plus NULL (must match ThorDAQAPIGetDIOConfig() DLL)

        static List<string> APISetDIOConfig(List<String> argumentsList)
        {
            List<string> RetStrings = new List<string> { };
            string strDIOconfig;
            int iBNClabel;
            Int32 iNum;
            Int32 iOutputSrc = 0; // (unused for Input)
            Int32 iMUXcode = 48;  // def. CPLD configured (3U BOB)
            Int32 iAUXcode = -1;  // needed only for AUX_GPIO in FPGA

            if (argumentsList.Count < 3) // at least <Chan>  <Dir>  <CpySrc>
            {
                RetStrings.Add(HELP_APIGetSetDIOConfig_MSSAGE);
                return RetStrings;
            }

            string sDIR = argumentsList[2].ToUpper();
            if (sDIR != "I" && sDIR != "O") // Only options
            {
                RetStrings.Add(HELP_APIGetSetDIOConfig_MSSAGE);
                return RetStrings;
            }

            iNum = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[1]);
            iBNClabel = iNum;
            strDIOconfig = String.Format("D{0,2:D2}", iNum); // 'D' plus two chars
            strDIOconfig += sDIR; // single DIR char

            if (argumentsList.Count >= 4)  // 4th arg is CpySrc
            {
                iOutputSrc = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[3]);
                strDIOconfig += String.Format("{0,2:D2}", iOutputSrc);
            }
            else
            {
                strDIOconfig += String.Format("{0,2:D2}", iNum);  // CpySrc default is always the Dn index itself
            }

            if (argumentsList.Count >= 5)  // 5th arg is MUX code
            {
                iMUXcode = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[4]);
            }
            strDIOconfig += String.Format("M{0,2:D2}", iMUXcode); // arg or the default

            if (argumentsList.Count >= 6)  // 6th arg is AUX index for FPGA's AUX_GPIO
            {
                iAUXcode = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[5]);
            }
            strDIOconfig += String.Format("A{0,2:D2}", iAUXcode); // arg or the default

            // check for TEST case!!
            if(iBNClabel == 99 || iBNClabel == 100)
            {
                bool bOutputs = (sDIR == "O") ? true : false;
                bool bStatus = TestDIOs(bOutputs, iBNClabel);  // recursive function (calls this function with separate BNC indexes)
                if(bStatus == true) RetStrings.Add("SUCCESS");
                else RetStrings.Add("FAIL");
                return RetStrings;
            }

            // Must send as "unmanaged" char array
            IntPtr UnManagedConfigBuffer = Marshal.AllocHGlobal((int)TD_BOBDIODef.CharPerBOB_DIO); // (includes NULL byte)
            //byte[] bytes = Encoding.ASCII.GetBytes(strDIOconfig);
            int Idx = 0;
            foreach (char Chr in strDIOconfig)
            {
                Marshal.WriteByte(UnManagedConfigBuffer, Idx++, (byte)Chr);
            }

            THORDAQ_STATUS status;
            status = ThorDAQAPISetDIOConfig(MasterBoardIndex, UnManagedConfigBuffer, (int)TD_BOBDIODef.CharPerBOB_DIO); // DIOconfig is DLL char (e.g. [32][9] array) - send our size
            if( status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                if(status == THORDAQ_STATUS.STATUS_DEVICE_NOT_EXISTS)
                    RetStrings.Add("ERROR: BOB hardware device not found on I2C bus - cable problem?");
                else
                    RetStrings.Add(String.Format("ERROR: BOB hardware does not support BNC D{0} specified connection", iBNClabel));
                return RetStrings;
            }
            Marshal.FreeHGlobal(UnManagedConfigBuffer); // release our temp buffer
            RetStrings.Add("SUCCESS");
            return RetStrings;
        }

        void APIGetDDR3status(List<String> argumentsList)
        {
            foreach( string Str in GetDDR3status(new List<string> { "InternalCmd", " " }))
            {
                UpdateConsoleStatus(Str);
            }
        }
        // SharedTypes.cs is the source of the TD_DDR3_SPD (also used by DLL)
        static List<string> GetDDR3status(List<String> argumentsList)
        {
            List<string> RetStrings = new List<string> { };
            THORDAQ_STATUS status = 0;
            List<String> strDDR3status = new List<string> { };

            IntPtr UnManagedConfigBuffer = Marshal.AllocHGlobal((int)TD_DDR3_SPD.DDR3_SPD_LEN);
            for (int k = 0; k < (int)TD_DDR3_SPD.DDR3_SPD_LEN; k++)
                Marshal.WriteByte(UnManagedConfigBuffer, k, 0);

            char[] DDR3status = new char[(int)TD_DDR3_SPD.DDR3_SPD_LEN];

            status = ThorDAQAPIGetDDR3status(MasterBoardIndex, UnManagedConfigBuffer, (int)TD_DDR3_SPD.DDR3_SPD_LEN); // 
            if (status == THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                int i;
                for( i=0; i< (int)TD_DDR3_SPD.DDR3_SPD_LEN; i++)
                    DDR3status[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, i);
                string EntireStatus = new string(DDR3status);
                string[] DDR3fields = EntireStatus.Split(';');
                i = 0;
                foreach( string Str in DDR3fields)
                {
                    RetStrings.Add(Str);
                 //   UpdateConsoleStatus(Str);
                    if (++i >= (int)TD_DDR3_SPD.SPD_FieldCount) break; // currently only 6 fields defined.
                }
            }
            else
            {
                RetStrings.Add("Error reading I2C for DDR3 mem module EEPROM");
            };
            return RetStrings;
        }


        // SharedTypes.cs is the source of the TD_BOBstatusDef (also used by DLL)
        /* TD_BOBstatusDef
           BOBtypeCharLen = 13, // e.g. "3U Panel BOB"
            BOBhardwareRev = 4,  // e.g. "1.2"
            BOB_DDB1_SN    = 29, // e.g. DB100105225601014205/10/2019 (defined field as of Oct-2021 28 chars)
            BOB_ADB1_SN = 29, // e.g. AB1...
            BOB_ADB2_SN = 29, // e.g. AB2...
            BOB_ADB3_SN = 29, // e.g. AB3...
            BOB_status =  32, // e.g. "CABLE SWAPPED", "no CPLD program", "Missing DIO/AIO", "OK"
            DAC_CPLD    = 16,  // e.g. "DACCPLD 1.0.0.1" or "DACCPLD error"
            // valid for 3U BOB only...
            BOB_CPLD = 16     // e.g. BOB...
    */
        static List<String> APIGetBOBstatus(List<String> argumentsList) // e.g. could use arg list to limit to single returned string
        {
            THORDAQ_STATUS status = 0;
            List<String> strBOBstatus = new List<string> { };

            IntPtr UnManagedConfigBuffer = Marshal.AllocHGlobal((int)TD_BOBstatusDef.BOB_ALL);
            for (int k = 0; k < (int)TD_BOBstatusDef.BOB_ALL; k++)
                Marshal.WriteByte(UnManagedConfigBuffer, k, 0);

            char[] BOBstatus = new char[(int)TD_BOBstatusDef.BOB_ALL];

            status = ThorDAQAPIGetBOBstatus(MasterBoardIndex, UnManagedConfigBuffer, (int)TD_BOBstatusDef.BOB_ALL); // DIOconfig is DLL char (e.g. [32][6] array) - send our size
                                                                                                                    //if (status == THORDAQ_STATUS.STATUS_SUCCESSFUL)

            int i, Accumlator = 0;
            for (i = 0; i < (int)TD_BOBstatusDef.BOBtypeCharLen; i++)
            {
                BOBstatus[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, Accumlator++);
            }
            // convert the array into strings for easier display
            strBOBstatus.Add(new string(BOBstatus));
            for (i = 0; i < (int)TD_BOBstatusDef.BOB_DBB1_SN; i++)
            {
                BOBstatus[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, Accumlator++);
            }
            strBOBstatus.Add(new string(BOBstatus));
            for (i = 0; i < (int)TD_BOBstatusDef.BOB_ABB1_SN; i++)
            {
                BOBstatus[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, Accumlator++);
            }
            strBOBstatus.Add(new string(BOBstatus));
            for (i = 0; i < (int)TD_BOBstatusDef.BOB_ABB2_SN; i++)
            {
                BOBstatus[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, Accumlator++);
            }
            strBOBstatus.Add(new string(BOBstatus));
            for (i = 0; i < (int)TD_BOBstatusDef.BOB_ABB3_SN; i++)
            {
                BOBstatus[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, Accumlator++);
            }
            strBOBstatus.Add(new string(BOBstatus));
            for (i = 0; i < (int)TD_BOBstatusDef.BOB_status; i++)
            {
                BOBstatus[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, Accumlator++);
            }
            strBOBstatus.Add(new string(BOBstatus));
            // reset buffer
            Array.Clear(BOBstatus, 0, BOBstatus.Length);
            for (i = 0; i < (int)TD_BOBstatusDef.DAC_CPLD; i++)
            {
                BOBstatus[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, Accumlator++);
            }
            strBOBstatus.Add(new string(BOBstatus));
            for (i = 0; i < (int)TD_BOBstatusDef.BOB_CPLD; i++)
            {
                BOBstatus[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, Accumlator++);
            }
            strBOBstatus.Add(new string(BOBstatus));
            // now read the ABBX mux (in some instances it is changed to default, sometimes not)
            UInt64 RegValue = 0;
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalABBXmuxReg", ref RegValue);
            strBOBstatus.Add(string.Format("GlobalABBXmuxReg: 0x{0}", RegValue.ToString("X16")));

            Marshal.FreeHGlobal(UnManagedConfigBuffer); // release our temp buffer
            return strBOBstatus;
        }

        // SharedTypes.cs is the source of the TD_BOBDIODef (also used by DLL)
        void APIGetDIOConfig(List<String> argumentsList)
        {
            THORDAQ_STATUS status = 0;
            List<String> strDIOlist = new List<string> { };
            
            IntPtr UnManagedConfigBuffer = Marshal.AllocHGlobal((int)TD_BOBDIODef.NumBOB_DIOs * (int)TD_BOBDIODef.CharPerBOB_DIO);
            char[] DIOconfig = new char[(int)TD_BOBDIODef.CharPerBOB_DIO];

            status = ThorDAQAPIGetDIOConfig(MasterBoardIndex, UnManagedConfigBuffer, ((int)TD_BOBDIODef.NumBOB_DIOs * (int)TD_BOBDIODef.CharPerBOB_DIO)); // DIOconfig is DLL char (e.g. [32][6] array) - send our size

            if (status == THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                int i, j;
                for (i = 0; i < (int)TD_BOBDIODef.NumBOB_DIOs; i++)
                {
                    for (j = 0; j < (int)TD_BOBDIODef.CharPerBOB_DIO; j++)
                    {
                        DIOconfig[j] = (char)Marshal.ReadByte(UnManagedConfigBuffer, (i * (int)TD_BOBDIODef.CharPerBOB_DIO + j));
                    }
                    // convert the array into strings for easier display
                    strDIOlist.Add(new string(DIOconfig));
                }

                // format as the Dx appears on 3U
                // for Legacy DBB1, expect single row populated by DLL
                for (int row = 0; row < 4; row++)
                {
                    string DIOline = null;
                    for (int col = 0; col < 8; col++)
                    {
                        DIOline += String.Format(" {0}", strDIOlist[row * 8 + col]);
                    }
                    UpdateConsoleStatus(DIOline);
                }
            }
            Marshal.FreeHGlobal(UnManagedConfigBuffer); // release our temp buffer
            return;
        }



        static string HELP_APIGetSetDIO_MSSAGE = @"APIGetDIO APISetDO  
APIGetDIO <DBNCindex> <DBNCindex> ... <DBNCindex> (D99 gets ALL)
APISetDO <DBNCindex> <value>  <DBNCindex> <value> ... 
  DBNCindex    - 0-based index of the BNC DIO (e.g., D2 is BNC 'D2' for 3U, 'DIO3' for Legacy)
  value        - 0 or 1
Example:
  APISetDO D3 1 D17 0 D0 1  -- Sets D3 and D0, clear D17
 ";
        // Get the "live" input value for INPUT, or last written value if OUTPUT
        // Because this integrates very fast (microsecs) mezz. card DIO with slow (millisecs) I2C
        // read latency time, allow multiple args of any combination, with note to user
        // that mixing fast and slow means the function will not return until all values are
        // read.  If mixed, read SLOW values first, then FAST
        void APIGetDIO(List<String> argumentsList)
        {
            IntPtr UnManagedConfigBuffer = IntPtr.Zero;
            List<String> strDIOlist = new List<string> { };
            char[] DIOconfig = new char[(int)TD_BOBDIODef.CharPerBOB_DIO];
            Int32 iNum; 
                             // allocate the MAX buffer needed...
            UnManagedConfigBuffer = Marshal.AllocHGlobal((int)TD_BOBDIODef.NumBOB_DIOs * (int)TD_BOBDIODef.CharPerBOB_DIO);

            // a legal "field" is a BNC index follow by "0" or "1" value to set
            UInt32 NumberOfBNCsToGET = 0;
            string DO_BNCindex;
            List<String> strBNCindex_Value = new List<string> { };
            if (argumentsList.Count < 2)
            {
                UpdateConsoleStatus(HELP_APIGetSetDIO_MSSAGE);
                return;
            }
            string sNum;
            int k;
            int i, j;

            try
            {
                // special case of getting them all...
                if (argumentsList[1] == "D99")
                {
                    for (k = 0; k < 8; k++)  // the first 8 must work on Legacy and 3U Panel BOB
                    {
                        DO_BNCindex = String.Format("D{0,2:D2}Xxx      ", k); // 'D' plus two chars, "xx" is default Unknown
                        NumberOfBNCsToGET++;
                        strBNCindex_Value.Add(DO_BNCindex);
                    }
                }
                else
                {
                    // process Dn one at a time...
                    for (int argNum = 1; argNum < argumentsList.Count;)
                    {
                        sNum = argumentsList[argNum++].Remove(0, 1); // i.e. omit leading "d" or "D"
                        iNum = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(sNum);
                        DO_BNCindex = String.Format("D{0,2:D2}Xxx      ", iNum); // 'D' plus two chars, "xx" is default Unknown
                                                                                 // e.g. "D17X        "
                        NumberOfBNCsToGET++;
                        strBNCindex_Value.Add(DO_BNCindex);
                    }
                }
                // we have the Output(s) to get...
                THORDAQ_STATUS status = 0;
                int Idx = 0, iDIOsCopied = 0;
                foreach (string BNCrecord in strBNCindex_Value)
                {
                    foreach (char Chr in BNCrecord)
                    {
                        Marshal.WriteByte(UnManagedConfigBuffer, Idx++, (byte)Chr);
                    }
                    Idx = ++iDIOsCopied * (int)TD_BOBDIODef.CharPerBOB_DIO;
                }
                // send the "list" of DIO read request... the 1st 8 must succeed
                status = ThorDAQAPIGetDIO(MasterBoardIndex, UnManagedConfigBuffer, (uint)TD_BOBDIODef.CharPerBOB_DIO, NumberOfBNCsToGET);
                if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    UpdateConsoleStatus(String.Format("ERROR: {0:X}", status));
                }
                else // print the first 8 DIOs
                {
                    for (i = 0; i < NumberOfBNCsToGET; i++)
                    {
                        for (j = 0; j < (int)TD_BOBDIODef.CharPerBOB_DIO; j++)
                        {
                            DIOconfig[j] = (char)Marshal.ReadByte(UnManagedConfigBuffer, (i * (int)TD_BOBDIODef.CharPerBOB_DIO + j));
                        }
                        // convert the array into strings for easier display
                        strDIOlist.Add(new string(DIOconfig));
                    }
                    int NumBNCsProcessed = 0;
                    for (int row = 0; row < 4 && NumBNCsProcessed < NumberOfBNCsToGET; row++)
                    {
                        string DIOline = null;
                        for (int col = 0; col < 8; col++)
                        {
                            DIOline += String.Format(" {0}", strDIOlist[row * 8 + col]);
                            if (++NumBNCsProcessed >= NumberOfBNCsToGET) break; // done
                        }
                        UpdateConsoleStatus(DIOline);
                    }
                }
                if (argumentsList[1] == "D99") // "All DIOs" ?
                {
                    strBNCindex_Value.Clear();
                    for (k = 8, NumberOfBNCsToGET = 0; k < 32; k++)  // only the 0-7 must work on Legacy and 3U Panel BOB
                    {
                        DO_BNCindex = String.Format("D{0,2:D2}Xxx      ", k); // 'D' plus two chars, "xx" is default Unknown
                        NumberOfBNCsToGET++;
                        strBNCindex_Value.Add(DO_BNCindex);
                    }
                    iDIOsCopied = 0;
                    Idx = 0;
                    foreach (string BNCrecord in strBNCindex_Value)
                    {
                        foreach (char Chr in BNCrecord)
                        {
                            Marshal.WriteByte(UnManagedConfigBuffer, Idx++, (byte)Chr);
                        }
                        Idx = ++iDIOsCopied * (int)TD_BOBDIODef.CharPerBOB_DIO;
                    }

                    // send the "list" of DIO read request...
                    status = ThorDAQAPIGetDIO(MasterBoardIndex, UnManagedConfigBuffer, (uint)TD_BOBDIODef.CharPerBOB_DIO, NumberOfBNCsToGET);
                    // these commands will fail if we're connected to Legacy BOB
                    if (status == THORDAQ_STATUS.STATUS_SUCCESSFUL)
                    {
                        strDIOlist.Clear();
                        // output the results
                        {
                            for (i = 0; i < NumberOfBNCsToGET; i++)
                            {
                                for (j = 0; j < (int)TD_BOBDIODef.CharPerBOB_DIO; j++)
                                {
                                    DIOconfig[j] = (char)Marshal.ReadByte(UnManagedConfigBuffer, (i * (int)TD_BOBDIODef.CharPerBOB_DIO + j));
                                }
                                // convert the array into strings for easier display
                                strDIOlist.Add(new string(DIOconfig));
                            }
                            int NumBNCsProcessed = 0;
                            for (int row = 0; row < 4 && NumBNCsProcessed < NumberOfBNCsToGET; row++)
                            {
                                string DIOline = null;
                                for (int col = 0; col < 8; col++)
                                {
                                    DIOline += String.Format(" {0}", strDIOlist[row * 8 + col]);
                                    if (++NumBNCsProcessed >= NumberOfBNCsToGET) break; // done
                                }
                                UpdateConsoleStatus(DIOline);
                            }
                        }

                    }

                }
            }
            catch (ArgumentException e)
            {
                UpdateConsoleStatus(String.Format("ERROR: {0}", e.Message));
                UpdateConsoleStatus(HELP_APIGetSetDIO_MSSAGE);
            }
            finally
            {
                if (UnManagedConfigBuffer != IntPtr.Zero)
                    Marshal.FreeHGlobal(UnManagedConfigBuffer); // release our temp buffer
            }
        }
    
        void APISetDO(List<String> argumentsList)
        {
            IntPtr UnManagedConfigBuffer = IntPtr.Zero;
            // a legal "field" is a BNC index follow by "0" or "1" value to set
            UInt32 NumberOfBNCsToSet = 0;
            Int32 iNum;
            string DO_BNCandValue;
            List<String> strBNCindex_Value = new List<string> { };
            if (argumentsList.Count < 3)
            {
                UpdateConsoleStatus(HELP_APIGetSetDIO_MSSAGE);
                return;
            }
            try
            {
                // process args 2 at a time, e.g. "D3 1 D7 0", set BNC D3 to 1, next then iteration, D7 to 0
                for (int argNum = 1; argNum < argumentsList.Count;)
                {
                    string sNum = argumentsList[argNum++].Remove(0, 1); // i.e. omit leading "d" or "D"
                    iNum = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(sNum);
                    DO_BNCandValue = String.Format("D{0,2:D2}", iNum); // 'D' plus two chars
                    DO_BNCandValue += "X"; // DLL tests configured Direction of I/O (must be configured Output)
                    iNum = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[argNum++]);
                    DO_BNCandValue += String.Format("{0,2:D2}", iNum); // 'D' plus two chars
                    DO_BNCandValue += "   "; // pad 3 blanks (Mcc)
                    NumberOfBNCsToSet++;
                    strBNCindex_Value.Add(DO_BNCandValue);
                }
                // we have the Output(s) to set...
                THORDAQ_STATUS status = 0;
                UnManagedConfigBuffer = Marshal.AllocHGlobal((int)NumberOfBNCsToSet * (int)TD_BOBDIODef.CharPerBOB_DIO);
                int Idx = 0, iDIOsCopied = 0;
                foreach (string BNCrecord in strBNCindex_Value)
                {
                    foreach (char Chr in BNCrecord)
                    {
                        Marshal.WriteByte(UnManagedConfigBuffer, Idx++, (byte)Chr);
                    }
                    Idx = ++iDIOsCopied * (int)TD_BOBDIODef.CharPerBOB_DIO;
                }
                // send the "list" of DO commands...
                status = ThorDAQAPISetDO(MasterBoardIndex, UnManagedConfigBuffer, (uint)TD_BOBDIODef.CharPerBOB_DIO, NumberOfBNCsToSet);
                if( status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    UpdateConsoleStatus(String.Format("ERROR: {0:X}", status));
                }
            }
            catch (ArgumentException e)
            {
                UpdateConsoleStatus(String.Format("ERROR: {0}", e.Message));
                UpdateConsoleStatus(HELP_APIGetSetDIO_MSSAGE);
            }
            finally
            {
                if(UnManagedConfigBuffer != IntPtr.Zero)
                    Marshal.FreeHGlobal(UnManagedConfigBuffer); // release our temp buffer
            }
        }


        string HELP_APIGetAI_MSSAGE = @"APIGetAI <-option> <BNClabel>
Options:
  -r   Raw ADC counts (12-bit, 2-s complement), def. is Volts
  -T   Continuous read test to measure sample rate of ADC acq (depends on I2C)
Example use: 
  APIGetAI         Read Volts for all AIs (def.)
  APIGetAI -r AI6  Return ADC counts read from first MAX127 channel (AI6)
"
;
        void APIGetAI(List<String> argumentsList)
        {
            uint status = 0;
            double dVolts = 99.9;
            double dADCcounts = 0;
            string AIlabel = " ";
            int iBNCindex = 0;
            bool bReturnVolts = true; // def.
            bool bTimeTest = false;
            if (argumentsList.Count < 2)
            {
                AIlabel = "ALL";
            }

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-T":
                        bTimeTest = true;
                        AIlabel = "ALL";
                        break;
                    case "-r":  // range
                        bReturnVolts = false;
                        break;
                    default: // get channel #
                        char[] charsToTrim = { '-', 'a', 'i', 'A', 'I' };
                        AIlabel = argumentsList[argNum].TrimStart(charsToTrim);
                        iBNCindex = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(AIlabel);
                        break;
                }
            }
            if( AIlabel == "ALL")
            {
                int idx;
                // read all possible AI channels...
                // (For now ONLY the MAX127 channels on 3U IO Panel)
                for( idx = 6; idx < 14; idx++)
                {
                    status = ThorDAQAPIGetAI(MasterBoardIndex, idx, true, ref dVolts);
                    if (status != (uint)THORDAQ_STATUS.STATUS_SUCCESSFUL)
                    {
                        UpdateConsoleStatus("ERROR: status = " + status);
                        return;
                    }
                    status = ThorDAQAPIGetAI(MasterBoardIndex, idx, false, ref dADCcounts);
                    int iADCcounts = (int)dADCcounts;
                    UpdateConsoleStatus("AI" + idx + " ADCcnts: " + iADCcounts + " (0x" + iADCcounts.ToString("X") + ")  VDC: " + dVolts.ToString("0.##"));
                }
                if(bTimeTest == true) // time test to measure max frequency of sample ACQ via I2C?
                {
                    int iTerations = 0;
                    Stopwatch s = new Stopwatch();
                    double MilliSecs = 2500.0;
                    s.Start();
                    while (s.Elapsed < TimeSpan.FromMilliseconds(MilliSecs))
                    {
                        for (idx = 6; idx < 14; idx++, iTerations++)
                        {
                            status = ThorDAQAPIGetAI(MasterBoardIndex, idx, true, ref dVolts);
                            if (status != (uint)THORDAQ_STATUS.STATUS_SUCCESSFUL)
                            {
                                break;
                            }
                            status = ThorDAQAPIGetAI(MasterBoardIndex, idx, false, ref dADCcounts);
                        }
                    }
                    s.Stop();
                    if (status != (uint)THORDAQ_STATUS.STATUS_SUCCESSFUL)
                    {
                        UpdateConsoleStatus("ERROR in Time test at sample# #" + iTerations.ToString() + " : status = " + status);
                    }
                    else // good measurement
                    {
                        double dHz = iTerations / MilliSecs * 1000.0;   // Hz
                        UpdateConsoleStatus("Sampling speed of MAX127 ADC via I2C: " + dHz.ToString("F0") + " Hz" );
                    }
                }
            }
            else // read the single channel...
            {
                // what is the numerical ENUM value for the passed string (e.g. "eAI9")?
                status = ThorDAQAPIGetAI(MasterBoardIndex, iBNCindex, bReturnVolts, ref dVolts);
                if (status != (uint)THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    UpdateConsoleStatus("ERROR: status = " + status);
                    return;
                }
                if (bReturnVolts)
                    UpdateConsoleStatus("AI" + iBNCindex + " VDC: " + dVolts.ToString("0.##"));
                else
                {
                    UInt32 ADCcnts = (UInt32)dVolts;
                    UpdateConsoleStatus("AI" + iBNCindex + " ADCcnts: " + ADCcnts + " (0x" + ADCcnts.ToString("X") + ")"); // CNTS were returned
                }

            }
        }

        // State can be 0 (off), 1 (on), 2 (blink)
        void APIBreakOutBoxLEDControl(List<String> argumentsList)
        {
            uint status;
            Byte State = 1; // on
            //bool bLEDon = true;
            bool bAllLEDs = false;
            string LEDlabel = argumentsList[1].TrimStart('-');  // label must be first arg
            Int32 iterations = 1;
            string LEDstring = " "; // e.g. "AO12" instead of the enum int value for that (debug aid)

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-all": // turn on or off ALL the BreakoutBox LEDs
                        // test case  
                        bAllLEDs = true;

                        break;
                    case "-i":  // iterations?
                        iterations = Int32.Parse(argumentsList[argNum + 1]);
                        break;
                    case "off":
                    case "OFF":
                    case "Off":
                        State = 0;
                        break;
                    case "blink":
                    case "Blink":
                    case "BLINK":
                        State = 2;
                        break;

                    default:

                        break;
                }
            }
            if (bAllLEDs)
            {
                
                status = ThorDAQAPIBreakOutBoxLED(MasterBoardIndex, 0xFF, State);
            }
            else // individual LED
            {
                // what is the numerical ENUM value for the passed string (e.g. "AO2")?
                // scan the entire enum range...
                Int32 LEDenum;
                for (LEDenum = 0; LEDenum <= (Int32)BBoxLEDenum.AI13; LEDenum++)
                {
                    LEDstring = Enum.GetName(typeof(BBoxLEDenum), LEDenum);
                    if (LEDlabel == LEDstring)
                        break;
                }
                // range check
                if (LEDenum > (Int32)BBoxLEDenum.AI13 || LEDenum < (Int32)BBoxLEDenum.D0)
                {
                    UpdateConsoleStatus("ERROR - invalid BBoxLEDenum {0} ", LEDenum);
                }
                status = ThorDAQAPIBreakOutBoxLED(MasterBoardIndex, LEDenum, State);
            }
            if (status != (uint)THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                UpdateConsoleStatus("ERROR: LED " + LEDstring + " Not found");
            }
        }

        // Breakout Box LED controls
        public void ControlBBleds(List<String> argumentsList)
        {
            bool bLEDon = true;
            bool bAllLEDs = false;
            string LEDlabel = argumentsList[1].TrimStart('-');  // label must be first arg
            Int32 iterations = 1;

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-all": // turn on or off ALL the BreakoutBox LEDs
                        // test case  
                        bAllLEDs = true;

                        break;
                    case "-i":  // iterations?
                        iterations = Int32.Parse(argumentsList[argNum + 1]);
                        break;
                    case "off":
                    case "OFF":
                    case "Off":
                        bLEDon = false;
                        break;
                    default:

                        break;
                }
            }
            if (bAllLEDs)
            {
                do
                {
                    if (bLEDon)
                    {
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon("DIO1");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon("DIO2");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon("DIO3");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon("DIO4");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon("DIO5");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon("DIO6");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon("DIO7");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon("DIO8");

                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDon("AO1");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDon("AO2");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDon("AO3");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDon("AO4");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDon("AI1");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDon("AI2");

                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDon("AO5");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDon("AO6");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDon("AO7");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDon("AO8");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDon("AI3");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDon("AI4");

                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDon("AO9");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDon("AO10");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDon("AO11");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDon("AO12");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDon("AI5");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDon("AI6");
                    }
                    else // off
                    {
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff("DIO1");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff("DIO2");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff("DIO3");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff("DIO4");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff("DIO5");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff("DIO6");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff("DIO7");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff("DIO8");

                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDoff("AO1");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDoff("AO2");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDoff("AO3");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDoff("AO4");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDoff("AI1");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDoff("AI2");

                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDoff("AO5");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDoff("AO6");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDoff("AO7");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDoff("AO8");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDoff("AI3");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDoff("AI4");

                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDoff("AO9");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDoff("AO10");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDoff("AO11");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDoff("AO12");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDoff("AI5");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDoff("AI6");
                    }
                    bLEDon = !bLEDon;
                } while (--iterations > 0);
                return;
            }
            switch (LEDlabel)
            {
                case "DIO1":
                case "DIO2":
                case "DIO3":
                case "DIO4":
                case "DIO5":
                case "DIO6":
                case "DIO7":
                case "DIO8":
                    if (bLEDon)
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon(LEDlabel);
                    else
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff(LEDlabel);

                    break;
                case "AO1":
                case "AO2":
                case "AO3":
                case "AO4":
                case "AI1":
                case "AI2":
                    if (bLEDon)
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDon(LEDlabel);
                    else
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDoff(LEDlabel);

                    break;
            }
        }

        //                    requires two arguments - the 0-11 channel num, and -10.0 to +10.0 voltage
        // usage:  SetDAC_ParkValue [-c ChanNum] [-v ParkVoltage]
        public void SetDAC_ParkValue(List<String> argumentsList)
        {
            double dParkVoltage = 0;
            Int32 chan = 0;
            NumberStyles styles;

            if (argumentsList.Count < 4)
            {
                UpdateConsoleStatus("Error: insufficient command line args, [-c ChanNum] [-v ParkVoltage]");
                return;
            }

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-v":
                        styles = NumberStyles.Float;
                        dParkVoltage = Double.Parse(argumentsList[argNum + 1], styles);
                        if (dParkVoltage > 10.0 || dParkVoltage < -10.0)
                        {
                            UpdateConsoleStatus("Error: Park Voltage must be between +10 and -10 Volts");
                            return;
                        }
                        break;
                    case "-c": // must have next arg as filename
                        styles = NumberStyles.Integer;
                        chan = Int32.Parse(argumentsList[argNum + 1], styles);
                        if (chan > 11 || chan < 0)
                        {
                            UpdateConsoleStatus("Error: Channel must be between 0 and 11");
                            return;
                        }
                        break;

                    default:
                        break;
                }
            }
            // we have settings - call the DLL function
            bool bStatus = ThorDAQAdapters[MasterBoardIndex].DAC_Control.SetStaticDACvoltage((UInt32)chan, dParkVoltage);
            //int iStatus = SetDACParkValue((UInt32)MasterBoardIndex, (UInt32)chan, dParkVoltage);
            if (bStatus != true)
            {
                UpdateConsoleStatus("Error: SetStaticDACvoltage() " + ThorDAQAdapters[MasterBoardIndex].DAC_Control.sErrMsg);
            }
        }




        const uint TDMasterI2CMUXAddr = 0x71;
        const int TDSlaveI2CMUXAddr = 0x70;

        // HEX file format source FLASH: Copied from original Xilinx FPGA flash, modified for I2C
        string s8000HexWarningMsg = "!!! WARNING !!!  This HEX firmware file loads at 0x8000, the address used by " + System.Environment.NewLine +
                            "legacy 3P firmware applications that do NOT support update through FPGA" + System.Environment.NewLine +
                            "Loading this .HEX may make your 3P card NOT updateable again" + System.Environment.NewLine +
                            "If you want to continue re-issue command with '-F' flag";

        private uint APIprogram3P_MCUappFirmware(string fileName, bool bOriginal, bool bForce8000Hex)
        {
            APIupdateLFT_MCUFirmware(fileName, bOriginal, bForce8000Hex);
            return (0);
        }


        public void APIupdateLFT_CPLD(List<String> argumentsList)
        {
            string fileSpec = "notSpecified.jed"; // (not expected)
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.DefaultExt = ".jed";
            dlg.Filter = "Xilinx JEDEC Files (.jed)|*.jed";
            // Display OpenFileDialog by calling ShowDialog method
            Nullable<bool> bResult = dlg.ShowDialog();
            // Get the selected file name and display in a TextBox

            if (bResult == true)
            {
                // Open document
                fileSpec = dlg.FileName;
            }

            APIupdateLFT_CPLDFirmware(fileSpec);
        }

        const string HELP_UpdateLFT_AppFirmware_MSSAGE = @"APIUpdateLFT_AppFirmware  <-o> <-F> <-f filename>
Update SAMD21 Firmware on 3P clock/jitter attentuation mezzanine card
Options:
  -o           Restore 'original' .HEX file included with this software package distribution (if found)
  -F           Forces a legacy firmware load @ 0x8000; FPGA download compatible load starts @ 0x2000
  -C           Update the LFT_JA (3P) App CPLD with .jed file
  -f filespec  Specify a full filespec to the file
NOTE!
  The 'legacy' bootloader which was distributed to R&D customers used the USB interface; you identify
  this bootloader version by inspecting 2 LEDs on 3P board (above USB connector), which will be OFF if 
  no bootloader exists or if legacy bootloader is running, waiting for USB command for App flash.
  The I2_SLAVE_BOOTLOADER.HEX program, when starting after power cycle, briefly lights both
  RED and GREEN LEDs (about 1 second), then rapidly flashes green (about 5x per sec)
  To install I2C_SLAVE_BOOTLOADER for first time:
  1. Connect Segger J-Link probe to 3P board (above USB connector, not to lower right of connector)
  2. Using Atmel Studio, connect to SAMD21 MCU and make sure 'fuse' BOOTPROT unprotects all NVRAM
  3. ERASE entire NVRAM chip
  4. Flash I2C_SLAVE_BOOTLOADER.HEX
  5. Run ThorDAQ-CL_GUI utility version 2.1.x.x or higher, run 'Update3PFirmware'
  6. Load CLKRX_3P or other CLKRX_xx.HEX image with I2C slave support
  
  Loading a LEGACY version of clk_rx_rev2.hex at NVRAM 0x8000 will prevent future I2C updates - a 
  warning message cautions, and -F flag allows you to overwrite
";

        public void UpdateLFT_AppFirmware(List<String> argumentsList)
        {
            Nullable<bool> result;
            string fileName = null;
            bool bOriginalProgram = false;  // default is we are loading NEW flash program
            bool bForce8000Hex = false;
            bool bUpdateCPLD = false;

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-C":
                        bUpdateCPLD = true;
                        break;

                    case "-o":
                        bOriginalProgram = true;
                        break;
                    case "-F":
                        bForce8000Hex = true;  // forces new Bootloader to accept legacy App FW code @ 0x8000
                        break;

                    case "-f": // must have next arg as filename

                        fileName = argumentsList[argNum + 1];
                        break;
                    default:
                        break;
                }
            }
            if (fileName == null) // Dialog box for filespec
            {
                if (bOriginalProgram)
                {
                    fileName = @"c:\temp\CLKRX_3Pat2000.hex";
                }
                else
                {
                    //Create OpenFileDialog
                    string sFileExtension = ".hex";
                    string sFileFilter = "Intel HEX Files (.hex)|*.hex";
                    if ( bUpdateCPLD )
                    {
                        sFileExtension = ".jed";
                        sFileFilter = "Xilinx JEDEC Files (.jed)|*.jed";
                    }

                    Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
                    dlg.DefaultExt = sFileExtension;
                    dlg.Filter = sFileFilter;
                    // Display OpenFileDialog by calling ShowDialog method
                    result = dlg.ShowDialog();
                    // Get the selected file name and display in a TextBox

                    if (result == true)
                    {
                        // Open document
                        fileName = dlg.FileName;
                    }
                }
            }
            if (bUpdateCPLD)
            {
                APIupdateLFT_CPLDFirmware(fileName);
            }
            else
            {
                APIprogram3P_MCUappFirmware(fileName, bOriginalProgram, bForce8000Hex);
            }
            
        }



        //                    e.g.  arg1 arg2  arg3
        // usage:  UpdateCentralFPGAfirmware [-g] [-f <filespec>]
        public void UpdateCentralFPGAfirmware(List<String> argumentsList)
        {
            Nullable<bool> result;
            string fileName = null;
            bool bProduction = true;  // if no command line arg, assume production

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-g":
                        bProduction = false;
                        break;
                    case "-f": // must have next arg as filename

                        fileName = argumentsList[argNum + 1];
                        break;
                    default:
                        break;
                }
            }
            if (fileName == null) // Dialog box for filespec
            {
                //Create OpenFileDialog
                Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
                dlg.DefaultExt = ".mcs";
                dlg.Filter = "MCS Files (.mcs)|*.mcs";
                // Display OpenFileDialog by calling ShowDialog method
                result = dlg.ShowDialog();
                // Get the selected file name and display in a TextBox

                if (result == true)
                {
                    // Open document
                    fileName = dlg.FileName;
                }
            }

            ProgramFPGAFlash(fileName, bProduction);
        }

        public void APIupdateBOB_CPLD(List<String> argumentsList)
        {
            //Create OpenFileDialog
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.DefaultExt = ".jed";
            dlg.Filter = "JED Files (.jed)|*.jed";

            // Display OpenFileDialog by calling ShowDialog method
            Nullable<bool> result = dlg.ShowDialog();
            string fileName;
            // Get the selected file name and display in a TextBox
            if (result == true)
            {
                // Open document
                fileName = dlg.FileName;

                // Identify the CPLD component on the I2C network board (on DAC mez. card)
                const uint MasterMUXChan = 0x2;
                const uint SlaveMUXChan = 0xff;
                const uint TargetSlaveAddr = 0x44; // CPLD address on BOB

                API_ProgramLatticeCPLD(fileName, MasterMUXChan, SlaveMUXChan, TargetSlaveAddr);
                dlg.Reset();
            }
        }


        public void APIUpdateDAC_CPLD()
        {
            //Create OpenFileDialog
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.DefaultExt = ".jed";
            dlg.Filter = "JED Files (.jed)|*.jed";

            // Display OpenFileDialog by calling ShowDialog method
            Nullable<bool> result = dlg.ShowDialog();

            // Get the selected file name and display in a TextBox
            if (result == true)
            {
                // Open document
                string fileName = dlg.FileName;

                // Identify the CPLD component on the I2C network board (on DAC mez. card)
                const uint MasterMUXChan = 0x8;
                const uint SlaveMUXChan = 0xff;
                const uint TargetSlaveAddr = 0x40; // CPLD address on DAC board

                API_ProgramLatticeCPLD(fileName, MasterMUXChan, SlaveMUXChan, TargetSlaveAddr);
            }
        }

        public void UpdateCPLD()
        {
            //Create OpenFileDialog
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.DefaultExt = ".jed";
            dlg.Filter = "JED Files (.jed)|*.jed";

            // Display OpenFileDialog by calling ShowDialog method
            Nullable<bool> result = dlg.ShowDialog();

            // Get the selected file name and display in a TextBox
            if (result == true)
            {
                // Open document
                string fileName = dlg.FileName;
                ProgramCPLD(fileName);
                //ProgramCPLD(fileName);
            }
        }
        public void Delay(List<String> argumentsList)
        {
            UInt16 value = (UInt16)Convert.ToInt32(argumentsList[1]);
            System.Threading.Thread.Sleep(value);
        }

        const string HELP_KDstressTest_MSSAGE =
@"
Debugging tool which rapidly cycles through config-start-stop&reconfig-restart;
This tool is best used with kernel debugger output to diagnose potential hangs/BSODs
First devised for ASUS Prime H670 motherboard
Options:
  -r Resonant-Galvo mode (default)
  -g Galvo-Galvo mode
  -d WxH max image size (e.g. -d 4096x4096, def. is 2k x 2k max)
";

        public void KDstressTest(List<String> argumentsList)
        {
            uint status;
            int iGenericTestIterations = 1;
            int argNum = 1;
            int argCount = argumentsList.Count;
            while (argNum < argCount)
            {
                switch (argumentsList[argNum])
                {
                    case "-I":
                        // get iteration count (next arg)
                        iGenericTestIterations = Int32.Parse(argumentsList[++argNum]);
                        break;
                }
                ++argNum; // next arg
            }
            int iterations;
            for (iterations = 1; iterations <= iGenericTestIterations; ++iterations)
            {
                // turn on diagnostic LED
                //bool bOn = true;
                Byte bState = 1; // on
                status = ThorDAQAPIBreakOutBoxLED(MasterBoardIndex, (Int32)BBoxLEDenum.DC3, bState);
                UpdateConsoleStatus("Iteration: " + iterations.ToString());
                if (status != 0)
                {
                    UpdateConsoleStatus("LED error");
                    break; 
                }
                foreach (string sRec in APIGetBOBstatus(argumentsList))
                {
                    UpdateConsoleStatus(sRec);
                }
                // diagnostic LED off
                bState = 0;
                status = ThorDAQAPIBreakOutBoxLED(MasterBoardIndex, (Int32)BBoxLEDenum.DC3, bState);
                if (status != 0)
                {
                    UpdateConsoleStatus("LED error");
                    break;
                }
            }
        }


        const string HELP_XI2CReadWrite_MSSAGE =
@"
Debugging tool to be used with I2C protocol analyzer... reports only errors, not data
Use SWUG I2C Map for all possible Slave devices
Options:
  -s Slow speed (100 Hz), def. is 400 Hz
  -o Hex OpCode bytes to WRITE prior to the READ (i.e. command bytes)
  -p Page size (i.e. for writing) - def. is 16
  -n Number of bytes we expect to READ
  -d Hex data bytes to WRITE
Examples:
XI2Cread  MasterMuxChan SlaveMuxChan TargetSlaveI2CAddress -o OpCode0 ... OpCodeN -n ByteCount
XI2Cwrite MasterMuxChan SlaveMuxChan TargetSlaveI2CAddress -o OpCode0 ... OpCodeN  
    (does NOT send I2C stoP bit)
XI2Cwrite MasterMuxChan SlaveMuxChan TargetSlaveI2CAddress -d Data0 Data1 ... DataN
    (sends I2C stoP bit)
Read/Write with AIX (Xilinx) I2C Master (see I2C Device Map in SWUG for MUX/Address config)
XI2Cread MasterMuxChan SlaveMuxChan TargetSlaveI2CAddress -o byte0 byte1 ... byteN -n ByteLen
Usage Examples:  
XI2Cwrite  0x08 0x1 0x3C -s -d 0x0 0x1   
  XI=> MasterMUX chan 0x8, SlaveMUX 0x1, DevAddr 0x3c (IS31FL3236), 100 Hz
    Reg 0 (Shutdown Cmd), Data 1 (enable
XI2Cread  0x08 0xFF 0x54 -o 0x0 -n 28  
  XI=> MasterMUX chan 0x80, No SlaveMUX, DevAddr 0x54 (TDboard), StartIndx 0x0, 
    read 28 data bytes SN @0x0 
XI2Cread  0x08 0x08 0x50 -o 0x0 -n 28  
  => MasterMUX chan 0x08, SlaveMUX 0x08, DevAddr 0x50 (ABB3), read 28 data bytes SN @0x0 
XI2Cread  0x08 0xff 0x40 -o 0xC0 0 0 0 -n 4  (Reads DAC CPLD USERCODE)
";
        public void XI2Cwrite(List<String> argumentsList)
        {
            XI2Ctransfer(argumentsList);
        }
        public void XI2Cread(List<String> argumentsList)
        {
            XI2Ctransfer(argumentsList);
        }

        // to WRITE, the Xilinx master will receive a separate buffer of "opcodes" (if any)
        // and "data" bytes to write, to facilitate ReStart Write->Read I2C direction change
        // TxBufferLen is returned - from that number we can determine if all bytes
        // (opcode + data) were written, and if not at what byte index it failed
        // Note that hardware characteristics of I2C slaves very GREATLY
        // from relatively simply MUX slave to Complex Lattice CPLD
        // And ThorDAQ EEPROMs, for instance, can handle sequential 16-byte writes,
        // so the 28 byte write requires 2 I2C write sequences
        public void XI2Ctransfer(List<String> argumentsList)
        {
            THORDAQ_STATUS status = THORDAQ_STATUS.STATUS_SUCCESSFUL;
            UInt32 MasterMUXAddr = 0x71; // coded in hardware
            UInt32 MasterMUXChan = 0;
            Int32 SlaveMUXAddr = 0x70; // coded in hardware
            UInt32 SlaveMUXChan = 0xFF; // default NO slave MUX
            UInt32 DeviceAddr;
            UInt32 TransferBufferLen = 0; // read or write
            UInt32 OpcodeByteLen = 0; // 
            Int32 PageReadWriteLen = 16; // default for EEPROM
            bool bI2C_ReadDir = false;
            int ArgIndx = 4;  // i.e.  -o or -n;
            UInt32 I2CbusHz = 400; // DEFAULT!

            // required arg list...
            if (argumentsList.Count >= 4) // 
            {
                MasterMUXChan = (UInt32)new System.ComponentModel.UInt32Converter().ConvertFromString(argumentsList[1]);
                SlaveMUXChan = (UInt32)new System.ComponentModel.UInt32Converter().ConvertFromString(argumentsList[2]);
                DeviceAddr = (UInt32)new System.ComponentModel.UInt32Converter().ConvertFromString(argumentsList[3]);
            }
            else
            {
                UpdateConsoleStatus("Missing args: " + HELP_XI2CReadWrite_MSSAGE);
                return;
            }

            // 

            // 
            byte[] OpcodeBytes = new byte[8]; // max opcode count by ThorDAQ definition
            byte[] DataBytes = new byte[64]; // max DATA count by ThorDAQ definition
            IntPtr UnManagedOpCodeBuffer = Marshal.AllocHGlobal(OpcodeBytes.Length);
            IntPtr UnManagedDataBuffer = Marshal.AllocHGlobal(DataBytes.Length);
            uint i;
            while (ArgIndx < argumentsList.Count)
            {
                switch (argumentsList[ArgIndx])
                {
                    case "-s": // slower 100 kHz speed
                        I2CbusHz = 100;
                        break;
                    case "-p":
                        PageReadWriteLen = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[++ArgIndx]);
                        break;
                    case "-n":  // number of bytes we expect to READ
                        bI2C_ReadDir = true;
                        TransferBufferLen = (uint)new System.ComponentModel.UInt32Converter().ConvertFromString(argumentsList[++ArgIndx]);
                        break;
                    case "-d": // count all DATA bytes
                        bI2C_ReadDir = false;
                        ArgIndx++;
                        i = 0;
                        while (ArgIndx < argumentsList.Count )
                        {
                            // DATA includes OpCode(s)
                            DataBytes[i] = (byte)new System.ComponentModel.ByteConverter().ConvertFromString(argumentsList[ArgIndx]);
                            System.Runtime.InteropServices.Marshal.WriteByte(UnManagedDataBuffer, (int)i, DataBytes[i]);
                            ArgIndx++; i++;
                        }
                        TransferBufferLen = i;
                        break;

                    case "-o":
                        //bI2C_ReadDir = true;
                        ArgIndx++;
                        i = 0;
                        while (ArgIndx < argumentsList.Count && argumentsList[ArgIndx].Substring(0, 1) != "-")
                        {
                            OpcodeBytes[i] = (byte)new System.ComponentModel.ByteConverter().ConvertFromString(argumentsList[ArgIndx]);
                            System.Runtime.InteropServices.Marshal.WriteByte(UnManagedOpCodeBuffer, (int)i, OpcodeBytes[i]);
                            ArgIndx++; i++;
                        }
                        OpcodeByteLen = i;
                        ArgIndx--; // back up one Arg (in case -n follows -o)
                        break;

                    default:
                        break;
                };
                ArgIndx++; // next arg
            }
            status = THORDAQ_STATUS.STATUS_SUCCESSFUL;
            uint TotalBytesTransfered = 0;  // for diagnostic/error check

            if (bI2C_ReadDir == false)  // WRITE case 
            {
                status = (THORDAQ_STATUS)ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_ReadDir, MasterMUXAddr, MasterMUXChan,
                                                                                SlaveMUXAddr, SlaveMUXChan, DeviceAddr, I2CbusHz, PageReadWriteLen,
                                                                                UnManagedOpCodeBuffer, ref OpcodeByteLen,  // Opcode(s) means READ operation
                                                                                UnManagedDataBuffer, ref TransferBufferLen);
                TotalBytesTransfered += TransferBufferLen;
            }
            else      // READ case
            {
                status = (THORDAQ_STATUS)ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_ReadDir, MasterMUXAddr, MasterMUXChan,
                                                                            SlaveMUXAddr, SlaveMUXChan, DeviceAddr, I2CbusHz, PageReadWriteLen,
                                                                            UnManagedOpCodeBuffer, ref OpcodeByteLen,  // Opcode(s) means READ operation
                                                                            UnManagedDataBuffer, ref TransferBufferLen); // ret. value includes OpCode len
                TotalBytesTransfered += TransferBufferLen;
            }
            if ( status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                string sStatus = "I2C NAK - no device, check connection";
                if (status == THORDAQ_STATUS.STATUS_I2C_TIMEOUT_ERR)
                    sStatus = "I2C Timeout";
                UpdateConsoleStatus(" ThorDAQAPIXI2CReadWrite() I2C write failure " + sStatus + " [bytes transfered?:] " + TotalBytesTransfered.ToString());
            }

            Marshal.FreeHGlobal(UnManagedOpCodeBuffer);
            Marshal.FreeHGlobal(UnManagedDataBuffer);
        }



        const string HELP_WriteMem_MSSAGE = @"WriteMem BAR# BARoffset LengthInBytes ByteString
Usage Example:
(HEX 0xNN.. format only) starting at same destination - data shows address order):
Write single value, starting at Address, to contiguous byte locations:
To write 16 zeros to BAR 1, BarOffset 0x4000:
   writemem 0 0x4000 16 0x00
Write string of bytes starting at Address, LSB to MSB from left to right:
To write 4 bytes 
   writemem 1 0x10000 4 0xFEED2189"
;
        public void WriteBoardMemoryData(List<String> argumentsList)
        {
            IntPtr buffer = IntPtr.Zero;

            if (argumentsList.Count < 4)
            {
                UpdateConsoleStatus(HELP_WriteMem_MSSAGE);
                return;
            }
            try
            {
                UInt16 value = (UInt16)Convert.ToInt32(argumentsList[1]);
                //    if (value <= MAX_BAR_INDEX && value >= MIN_BAR_INDEX)
                {
                    UInt32 barNum = value; // get bar number
                    int registerAddress = (int)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[2]);
                    int NumBytesToWrite = (int)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[3]);
                    // 2 Chars per 8-bit value, so 4 bytes means 8 chars were passed
                    if (buffer == IntPtr.Zero)
                    {
                        Win32.AllocateBuffer(ref buffer, (uint)NumBytesToWrite);
                    }
                    string buffer_hexString = argumentsList[4];
                    byte byteValue;
                    // 
                    buffer_hexString = buffer_hexString.Replace("0x", string.Empty);
                    // if there is a SINGLE hex data byte, copy that single value instead of processing string of bytes
                    if ((buffer_hexString.Length == 2) )
                    {
                        byteValue = byte.Parse(buffer_hexString, NumberStyles.HexNumber, CultureInfo.InvariantCulture);
                        for (int j=0; j < NumBytesToWrite; j++)
                            System.Runtime.InteropServices.Marshal.WriteByte(buffer, j, byteValue);
                    }
                    else
                    {
                        for (int i = 0, j = 0; j < NumBytesToWrite; i+=2, j++)
                        {
                            byteValue = byte.Parse(buffer_hexString.Substring(i, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture);
                            System.Runtime.InteropServices.Marshal.WriteByte(buffer, j, byteValue);
                        }
                    }
                    string sStatus = ThorDAQAdapters[MasterBoardIndex].BAR.Write(barNum, (ulong)registerAddress, buffer, (ulong)NumBytesToWrite);
                    if (sStatus != "OK")
                    {
                        UpdateConsoleStatus(sStatus);
                    }
                    if (buffer != IntPtr.Zero)
                        Win32.FreeBuffer(buffer);
                }
            }
            catch (Exception)
            {
                if (buffer != IntPtr.Zero)
                {
                    Win32.FreeBuffer(buffer);
                }
                UpdateConsoleStatus("Writemem Error");
            }
        }

        // Create the OnPropertyChanged method to raise the event
        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        void InputBlock_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                string text = InputBlock.Text;
                string[] lines = text.Split(new string[] { "\r\n", "\n" }, StringSplitOptions.None);
                for (int i = 0; i < lines.Length; i++)
                {
                    string input = lines[i].Trim(); // remove all white space start or end in the input string,
                    input = input.Replace("dmadriverclix64 ", string.Empty);
                    input = System.Text.RegularExpressions.Regex.Replace(input, @"\s{2,}", " ");// replace multiple space with one
                    ConsoleOutput.Add(">>> " + input);
                    Scroller.ScrollToBottom();
                    UpdateConsoleCommand(input);
                    _last_recorded_command = ConsoleOutput.Count - 1;
                }
                InputBlock.Clear();
                e.Handled = true;
            }
            else if (e.Key == Key.Tab)
            {
                string input = InputBlock.Text.Trim(); // remove all white space start or end in the input string,
                if (_last_recorded_tabbed_command == string.Empty)
                {
                    _last_recorded_tabbed_command = input;
                }
                for (int i = _last_recorded_tabbed_command_index; i < THORDAQ_COMMANDLINE_COMMAND.Count(); i++)
                {
                    if (Regex.IsMatch(THORDAQ_COMMANDLINE_COMMAND[i], _last_recorded_tabbed_command, RegexOptions.IgnoreCase))
                    {
                        InputBlock.Text = THORDAQ_COMMANDLINE_COMMAND[i];
                        _last_recorded_tabbed_command_index++;
                        if (_last_recorded_tabbed_command_index >= THORDAQ_COMMANDLINE_COMMAND.Count())
                        {
                            _last_recorded_tabbed_command_index = 0;
                        }
                        InputBlock.Focus();
                        InputBlock.CaretIndex = InputBlock.Text.Length;
                        e.Handled = true;
                        break;
                    }
                }
                return;
            }
            else if (e.Key == Key.Up)
            {
                if (InputBlock.GetLineIndexFromCharacterIndex(InputBlock.CaretIndex) == InputBlock.GetFirstVisibleLineIndex())
                {
                    for (int i = _last_recorded_command; i >= 0; i--)
                    {
                        if (ConsoleOutput[i].Contains(">>>"))
                        {
                            InputBlock.Text = ConsoleOutput[i].Substring(3);
                            _last_recorded_command = Math.Max(i - 1, 0);
                            break;
                        }
                    }
                }
            }
            else if (e.Key == Key.Down)
            {
                for (int i = _last_recorded_command; i < ConsoleOutput.Count; i++)
                {
                    if (ConsoleOutput[i].Contains(">>>"))
                    {
                        InputBlock.Text = ConsoleOutput[i].Substring(3);
                        _last_recorded_command = Math.Min(i + 1, consoleOutput.Count - 1);
                        break;
                    }
                }
            }
            else if (e.Key == Key.C && (Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control)
            {
                StopAcquisition();
                InputBlock.Clear();
            }
            _last_recorded_tabbed_command = string.Empty;
            _last_recorded_tabbed_command_index = 0;
        }

        void MainWindow_Closed(object sender, EventArgs e)
        {
            if (_bw != null && _bw.IsBusy)
            {
                StopAcquisition();
            }
            ReleaseHandles();

            if (!ADC_unmanaged_DMAbuffer.Equals(null))
            {
                Marshal.FreeHGlobal(ADC_unmanaged_DMAbuffer); // free unmanaged memory (no garbage collection)
            }

            InputBlock.PreviewKeyDown -= InputBlock_PreviewKeyDown;

            // we are *lucky* that the test "customer" computer enumerates the PCI extended slot (outside chassis) as index 0
            uint Indx = 0;
            while (NumThorDAQboards-- > 0) // in case of multiple boards
            {
                uint status = ThorDAQAPIReleaseBoard(Indx++);
            }
        }



        // query Windows O/S for existence of ThorDAQ board(s)
        ushort CountOfThorDAQboards()
        {
            ushort iRetCode = 0; // no boards

            // Get the installed kernel device driver date stamp (from .INF, not from driver code)
            // This stamp is typically done from Visual Studio build, stampinf step, where base .INF file variable has "Driver = " (null)
            // and this stamp is far more reliable indicating any change to driver (including packaging)
            ManagementObjectSearcher objSearcher = new ManagementObjectSearcher("Select * from Win32_PnPSignedDriver");
            ManagementObjectCollection objCollection = objSearcher.Get();

            foreach (ManagementObject obj in objCollection)
            {
                string HardwareID = String.Format("{0}", obj["HardwareID"]);
                string Manuf = String.Format("{0}", obj["Manufacturer"]);
                if (Manuf.StartsWith("ThorLabs") || Manuf.StartsWith("Thorlabs")) // older Win7 driver is "Thorlabs"
                {
                    string ThorDAQdevice = String.Format("{0}", obj["DeviceName"]);    // e.g. ThorDAQ-2586
                    if (ThorDAQdevice.StartsWith("ThorDAQ"))
                    {
                        // only accumulate count of booards THIS program can bind to
                        // (i.e. 0x4001 firmwave boards)
                        if( HardwareID.Contains("DEV_4001")) iRetCode++;
                        UpdateConsoleStatus(HardwareID); // display ALL ThorDAQs
                        // No Thorlabs board can be discovered unless our kernel driver binds to it...
                        string sBuf = String.Format("{0}", obj["DriverDate"]);
                        string pDate = String.Format("{1}/{2}/{0}", sBuf.Substring(0, 4), sBuf.Substring(4, 2), sBuf.Substring(6, 2));
                        string info = String.Format("{0}, Driver Date: {1}  Version: {2}", obj["DeviceName"], pDate, obj["DriverVersion"]);
                        UpdateConsoleStatus(info);
                    }
                }
            }
            return iRetCode;
        }



        void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            DateTime localDate = DateTime.Now;
            uint status = 0;
            bool bKernelDriverRead = false;
            var culture = new CultureInfo("en-US");
            string lclDate = localDate.ToString(culture);
            UInt64 RegVal = 0xFFFFFF;
            bool bStatus;

            //            MessageBox.Show("debug");
            try
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ThorDAQcl-GUI: MainWindow_Loaded at " + lclDate);
            }
            catch( Exception eRR)
            {
                MessageBox.Show("ThorLog.Instance.TraceEvent CRASHED: " + eRR.Message);
            }
            BOARD_INFO_STRUCT BoardInfo = new BOARD_INFO_STRUCT();
            string sVar = "Dev/DEBUG version";
            try
            {
                if (ApplicationDeployment.IsNetworkDeployed)
                {
                    sVar = string.Format("{0}", ApplicationDeployment.CurrentDeployment.CurrentVersion);
                }
                string sPublishedVer = "Console Application Published Version: " + sVar;
                UpdateConsoleStatus(sPublishedVer); 
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, sPublishedVer);
            }
            catch( Exception eErr )
            {
                UpdateConsoleStatus(eErr.Message);
            }
            try
            {
                XmlDocument xmlDoc = new XmlDocument();
                xmlDoc.Load("ThorDAQSettings.xml");
                XmlNode node = xmlDoc.SelectSingleNode("/ThorDAQSettings/File");

                NumThorDAQboards = CountOfThorDAQboards();
                string sMsg = "ThorDAQcl-GUI: Num PCIe boards: " + NumThorDAQboards;
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, sMsg);

                if (NumThorDAQboards > 0)
                {
                    UInt32 BARdevID = 0;
                    MasterBoardIndex = 0;  // default until user changes
                    for (uint i = 0; i < NumThorDAQboards; i++)
                    {
                        SettingPath = node.Attributes["Path"].Value.ToString(); //ImageAcquiringThreadMethod()???

                        status = Win32.DsConnectToBoard(i, ref BoardInfo);

                        if (status == Win32.STATUS_SUCCESSFUL) // at least one board found?
                        {
                            // board connected (instantiated)...
                            if (bKernelDriverRead == false) // (one kernel driver detects ALL boards)
                            {
                                bKernelDriverRead = true;
                                UpdateConsoleStatus("ThorDAQ adapter connected, KERNEL DRIVER source version: " + BoardInfo.DriverVersionMajor.ToString() + "." + BoardInfo.DriverVersionMinor.ToString() + "." + BoardInfo.DriverVersionSubMinor.ToString());
                            }
                            ThorDAQAdapters[i] = new TDcontroller(i, "ThorDAQ-2586");

                            GetBoardConfiguration((int)i);
                            InitHandles(); // create background worker to handle imaging (e.g. @30 FPS)
                                           // DEBUG - what's status of NWL GlobalInt enable?
                            bStatus = ThorDAQAdapters[0].FPGAregs.Read("NWL_DDR3_DMAcore", ref RegVal);
                            UpdateConsoleStatus(string.Format("NWL_DDR3_DMAcore: 0x{0}", RegVal.ToString("X")));
                            UpdateConsoleStatus(HELP_MSSAGE);

                            // if a SCRIPT file exists, execute it
                            string DefScriptFileSpec = @"\temp\TDscript.txt";
                            if (File.Exists(DefScriptFileSpec))
                            {
                                UpdateConsoleStatus("!!! Found SCRIPT file " + DefScriptFileSpec + ", EXECUTING SCRIPT....");
                                LoadScript(DefScriptFileSpec);
                            }
                        }
                        else if(status == Win32.STATUS_INVALID_BOARDNUM && BARdevID != 0) // was unexpected board found?
                        {
                            UpdateConsoleStatus("Incompatible ThorDAQ adapter, PCI BAR ID: " + BARdevID.ToString("X"));
                        }
                        else
                        {
                            //throw new NullReferenceException();
                            UpdateConsoleStatus("Failed DsConnectToBoard(), status: " + status + " BoardIndex" + i);

                        }
                    }


                }
                else
                {
                    UpdateConsoleStatus("No ThorDAQ PCIe adapters found");
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "No ThorDAQ PCIe adapters found");  // report error to log file
                }

            }
            catch (Exception error)
            {
                MessageBox.Show(error.Message);
                this.Close();
            }

            try
            {
                // Do we have a TI USB adapter interface?  (Typically we don't)
                // Wire SAA events that tell us of interesting changes
                TIusbAdapters.RequestComplete += new TIBusAdapters.RequestCompleteEventHandler(Adapters_RequestComplete);
                TIusbAdapters.Warning += new TIBusAdapters.WarningEventHandler(Adapters_Warning);
                TIusbAdapters.ControlLineUpdated += new TIBusAdapters.ControlLineUpdatedEventHandler(Adapters_ControlLineUpdated);
                TIusbAdapters.SMBusAlertLineUpdated += new TIBusAdapters.SMBusAlertLineUpdatedEventHandler(Adapters_SMBusAlertLineUpdated);
                TIusbAdapters.USBDetached += new TIBusAdapters.USBDetachedEventHandler(Adapters_USBDetached);
                TIusbAdapters.USBReattached += new TIBusAdapters.USBReattachedEventHandler(Adapters_USBReattached);

                // This ensures that all Texas Instruments SMBusAdapter events get fired on main thread,
                // making for much simpler development
                // SMBusAdapter.Run_Events_On_UI_Thread();
                if (TIusbAdapters.Discover() > 0)
                {
                    UpdateConsoleStatus("Found " + TIusbAdapters.Num_Adapters + " Texas Instrument USB adapter(s)");
                }

                // Do we have a USB attached Digilent 2 ?
                Digilent2 DigilentUSBdevice = new Digilent2();
                if( DigilentUSBdevice.DigilentFound())
                {
                    string sDevSerNum, sDevName;
                    int iStatus, numDevices;
                    iStatus = dwf.FDwfEnum( (0x8000000 | dwf.enumfilterUSB), out numDevices); // nDevice is COUNT of USB devices
                    // int idxDevice, DEVID * pDeviceId, DEVVER * pDeviceRevision
                    Int32 iDevID, iDevRev;
                    iStatus = dwf.FDwfEnumDeviceType(0, out iDevID, out iDevRev);
                    dwf.FDwfEnumSN(0, out sDevSerNum);
                    dwf.FDwfEnumDeviceName(0, out sDevName);
                    UpdateConsoleStatus("Found " + numDevices.ToString() + " USB connected " + sDevName + " Wave Generator, Rev " + iDevRev.ToString() + ", " + sDevSerNum);
                    DigilentUSBdevice.CloseDigilent();
                } 

            }
            catch (Exception error)
            {
                if( !error.Message.Contains("Unable to load DLL 'dwf.dll'"))  // only required by ManufTest
                    MessageBox.Show(error.Message);

            }
        }






        #endregion Methods

        // bind API functions to the correct DLL depending on scan head mode selected
        private void Galvo_Galvo_radiobutton_Checked(object sender, RoutedEventArgs e)
        {
            StopACQifRunning();
            UpdateConsoleStatus("ThorDAQ select Galvo-Galvo");
            ScanHeadMode = ScanHeadType.Galvo_Galvo;
        }

        private void Res_Galvo_radiobutton_Checked(object sender, RoutedEventArgs e)
        {
            StopACQifRunning();
            UpdateConsoleStatus("ThorDAQ select Resonance-Galvo");
            ScanHeadMode = ScanHeadType.Res_Galvo;
        }

        private void Res_Galvo_Galvo_Checked(object sender, RoutedEventArgs e)
        {
            StopACQifRunning();
            UpdateConsoleStatus("ThorDAQ select Resonance-Galvo-Galvo");
            ScanHeadMode = ScanHeadType.Res_Galvo_Galvo;

        }

        // The customer's ThorImageLS allows resizing of the image during acquisition -
        // This happens so quickly that it's not obvious that there's a complete
        // shutdown, reconfig of FPGA parameters and waveforms, and restart.
        //
        private void Res512x512_Checked(object sender, RoutedEventArgs e)
        {
            StopACQifRunning();

            BitmapWidth = BitmapHeight = 512;
            _bNewBitMapRequired = true;
            Bitmap = new WriteableBitmap(BitmapWidth, BitmapHeight, 96, 96, PixelFormats.Indexed8, BuildPaletteGrayscale());
            // restart if necessary

        }

        private void Res32x32_Checked(object sender, RoutedEventArgs e)
        {
            StopACQifRunning();
            BitmapWidth = BitmapHeight = 32;
            _bNewBitMapRequired = true;
            Bitmap = new WriteableBitmap(BitmapWidth, BitmapHeight, 96, 96, PixelFormats.Indexed8, BuildPaletteGrayscale());
        }

        private void Res256x256_Checked(object sender, RoutedEventArgs e)
        {
            StopACQifRunning();
            BitmapWidth = BitmapHeight = 256;
            _bNewBitMapRequired = true;
            Bitmap = new WriteableBitmap(BitmapWidth, BitmapHeight, 96, 96, PixelFormats.Indexed8, BuildPaletteGrayscale());
        }

        private void Res2048x2048_Checked(object sender, RoutedEventArgs e)
        {
            StopACQifRunning();
            BitmapWidth = BitmapHeight = 2048;
            _bNewBitMapRequired = true;
            Bitmap = new WriteableBitmap(BitmapWidth, BitmapHeight, 96, 96, PixelFormats.Indexed8, BuildPaletteGrayscale());
        }

        private void Res4096x4096_Checked(object sender, RoutedEventArgs e)
        {
            StopACQifRunning();
            BitmapWidth = BitmapHeight = 4096;
            _bNewBitMapRequired = true;
            Bitmap = new WriteableBitmap(BitmapWidth, BitmapHeight, 96, 96, PixelFormats.Indexed8, BuildPaletteGrayscale());
        }

        // some variables require a STOP of acquisition (such as changing DMA buffer)
        private void StopACQifRunning()
        {
            if (_bLiveAcqFlag == true)
            {
                StopAcquisition();

                StartStopACQ.Content = "Start";
                _bLiveAcqFlag = false;
            }
        }
        private void StartStopACQ_Click(object sender, RoutedEventArgs e)
        {
            if (_bLiveAcqFlag == false) // Acquisition not running, button text "Start" - start it
            {
                StartAcqusition();
                // SetupTDcl_GUIacq();
                UsrIntCount.Content = "";


                StartStopACQ.Content = "Stop";
                _bLiveAcqFlag = true;
            }
            else // Acquisition is Running, button text "Start" - STOP it
            {
                StopAcquisition();

                StartStopACQ.Content = "Start";
                _bLiveAcqFlag = false;
            }
        }


        public bool LSMChannelEnable0
        {
            get
            {
                return _lsmChannelEnable[0];
            }
            set
            {
                _lsmChannelEnable[0] = value;
                SetChannelFromEnable();
            }
        }
        public bool LSMChannelEnable1
        {
            get
            {
                return _lsmChannelEnable[1];
            }
            set
            {
                _lsmChannelEnable[1] = value;
                SetChannelFromEnable();
            }
        }
        public bool LSMChannelEnable2
        {
            get
            {
                return _lsmChannelEnable[2];
            }
            set
            {
                _lsmChannelEnable[2] = value;
                SetChannelFromEnable();
            }
        }
        public bool LSMChannelEnable3
        {
            get
            {
                return _lsmChannelEnable[3];
            }
            set
            {
                _lsmChannelEnable[3] = value;
                SetChannelFromEnable();
            }
        }



        // routine from ...
        private void SetChannelFromEnable()
        {
            StopACQifRunning();
            //update the channel value also
            ADCbitwiseChanIndex = (Convert.ToInt32(_lsmChannelEnable[0]) | (Convert.ToInt32(_lsmChannelEnable[1]) << 1) | (Convert.ToInt32(_lsmChannelEnable[2]) << 2) | (Convert.ToInt32(_lsmChannelEnable[3]) << 3));
            int DMAcopyMask; // because CopyAcquisition re-arranges selected channels to be returned contiguously,
                             // we have to enable ALL DMA channels if more than 1 channel selected
                             // NOTE: we either copy 1 DMA channel from low level driver, or all 4; _lsChannelEnable determines userApp visibility
            switch (ADCbitwiseChanIndex)
            {
                case 0:  // if all channels disabled, select single channel so DMA copy is still exercised
                case 1:
                case 2:
                case 4:
                case 8:
                    {
                        ColorChannels = 1;
                        DMAcopyMask = ADCbitwiseChanIndex;
                        break;
                    }
                default:
                    {
                        ColorChannels = 4;  // 2 or more ADC channels
                        DMAcopyMask = 0xF;
                    }
                    break;
            }
            _bNewBitMapRequired = true; // force reconfig of bitmap, whether RGB24 or Indexed8
            // This is an early reference to required DLL - trap failure so we don't have an application
            // start that flickers and disappears without a clue.  New systems without Nat'l Instruments DAX-mx drivers
            // also fail with "DLL not found" when OUR DLL is found.
            // "ICamera" is from "ThorSharedTypes"
            try
            {
                if (MasterBoardIndex != 0xFEED) // don't allow call until initialized
                {
                    // hereeee
                    long CameraCnt;
                    GGFindCameras(out CameraCnt);

                    int retStatus = SetParam((int)ICamera.Params.PARAM_LSM_CHANNEL, DMAcopyMask); // notify Driver/DLL
                }
            }
            catch (Exception e)
            {
                MessageBox.Show("SetParam() failed, exception message: " + e.Message + " ... CL-GUI operation deprecated for TD Config Tool");
                // log errror
            }
        }
        // mimic the ThorImageLS methods
        private void LSMChannelSelect0(object sender, RoutedEventArgs e)
        {
            _lsmChannelEnable[0] = (bool)LSMChannelSelectA.IsChecked;
            SetChannelFromEnable(); // set in ThorDAQ DLL(s)
        }
        private void LSMChannelSelect1(object sender, RoutedEventArgs e)
        {
            _lsmChannelEnable[1] = (bool)LSMChannelSelectB.IsChecked;
            SetChannelFromEnable(); // set in ThorDAQ DLL(s)
        }
        private void LSMChannelSelect2(object sender, RoutedEventArgs e)
        {
            _lsmChannelEnable[2] = (bool)LSMChannelSelectC.IsChecked;
            SetChannelFromEnable(); // set in ThorDAQ DLL(s)
        }
        private void LSMChannelSelect3(object sender, RoutedEventArgs e)
        {
            _lsmChannelEnable[3] = (bool)LSMChannelSelectD.IsChecked;
            SetChannelFromEnable(); // set in ThorDAQ DLL(s)
        }

        // there are 8 GAIN levels, corresponding to 8 ticks on slider
        // the AFE (Analog Front End) control is bit 3, so gain is bits 0-2
        private void ADCgainSliderA_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            //           _sliderADCgain[0] = ADCgainSliderA.Value;
            Math.Round(_sliderADCgain[0]);
            // NOTE!  The first time we hit a "SetParam", prior to main() early execution, the app will
            // silently crash if DLL is has DEBUG symbols or is somehow not found
            try
            {

                int retStatus = SetParam((int)ICamera.Params.PARAM_LSM_INPUTRANGE1, _sliderADCgain[0]); // 
            }
            catch (Exception eMsg)
            {
                MessageBox.Show("FATAL ERROR! err: " + eMsg.Message); // inform user we are DEAD
            }
        }

        private void ADCgainSliderB_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            //           _sliderADCgain[1] = ADCgainSliderB.Value;
            Math.Round(_sliderADCgain[1]);
            int retStatus = SetParam((int)ICamera.Params.PARAM_LSM_INPUTRANGE2, _sliderADCgain[1]); // 
        }

        private void ADCgainSliderC_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            //           _sliderADCgain[2] = ADCgainSliderC.Value;
            Math.Round(_sliderADCgain[2]);
            int retStatus = SetParam((int)ICamera.Params.PARAM_LSM_INPUTRANGE3, _sliderADCgain[2]); // 
        }

        private void ADCgainSliderD_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            //           _sliderADCgain[3] = ADCgainSliderD.Value;
            Math.Round(_sliderADCgain[3]);
            int retStatus = SetParam((int)ICamera.Params.PARAM_LSM_INPUTRANGE4, _sliderADCgain[3]); // 

        }

        private void Graph_PlotAreaMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {



        }

        private void triggerMode_Capture_Checked(object sender, RoutedEventArgs e)
        {
            uint status = 0xF;
            status = ThorDAQAPIProgressiveScan(MasterBoardIndex, false);
        }

        private void triggerMode_Live_Checked(object sender, RoutedEventArgs e)
        {
            uint status = Win32.STATUS_INCOMPLETE;
            status = ThorDAQAPIProgressiveScan(MasterBoardIndex, true);
        }
    }



}