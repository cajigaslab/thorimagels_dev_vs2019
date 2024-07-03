namespace thordaqGUI
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Globalization;
    using System.Runtime.InteropServices;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Text.RegularExpressions;
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


    using NWLogic.DeviceLib;

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        #region Fields
        public static TDcontroller[] ThorDAQAdapters = new TDcontroller[4];

        public static readonly string[] THORDAQ_COMMANDLINE_COMMAND = 
        {
            "APIGetEEPROMlabel",
            "APIReadEEPROM",
            "APIWriteEEPROM",
            "APIReadProdSN",
            "APIWriteProdSN",
            "dFSetFrontEndSettings",
            "dFLIMReSync",
            "GetClockFrequency",
            "GetBoardCfg",
            "ReadMem",
            "WriteMem",
            "DDR3MemTest",
            "GetFPGAreg",
            "GetAllFPGAregisters",
            "SetFPGAreg",
            "UpdateFirmware",
            "UpdateCPLD",
            "Load",
            "PacketRead",
            "ReadChannel",
            "ReadDDR3",
            "WriteDDR3",
            "Clear",
            "LiveCapture",
            "StopCapture",
            "SetParkValue",
            "WriteSPI",
//            "Delay",
            "XI2CRead",
            "XI2CWrite"
        };

        /// <summary>Instance number for this board. </summary>
        public UInt16 devInstance;
        static public UInt16 MasterBoardIndex = 0;

        const string HELP_MSSAGE = @"ThorDAQ commands:
        APIGetEEPROMlabel 0_index  (index arg required - if index valid, 3 char label returned)
        APIReadEEPROM
        APIWriteEEPROM
        APIReadProdSN
        APIWriteProdSN
        dFSetFrontEndSettings
        dFLIMResync
        ReadMem <BarNum> <Card Offset> <Length>
        WriteMem <BarNum> <Card Offset> <Length> <Data...>
        GetAllFPGAregisters <-f [filename]> (default filename \temp\FPGAregs.txt)
        GetFPGAreg <Name> 
        SetFPGAreg <Name> <value>
        WriteSPI <index> <address> <data> %index: 0 -> clock synthsizer; 1-3 -> adc converter
        ReadChannel <Channel> <Size in bytes>
        ReadDDR3 <DDR3_Address> <bytecount>              
        WriteDDR3 <DDR3_Address> <bytecount> <data_byte [data_byte ...] >   writes 'bytecount' bytes for one 'data_byte'
        DDR3MemTest   (test of random byte write/read, DDR3 Addr Bit30, etc)
        LiveCapture <Channel> <HSIZExVSIZE> [PixelDwell(us)] {FrameCount} {FrameRate}
        GetBoardCfg
        GetClockFrequency  (returns clock frequencies from FMC126, requires dFSetFrontEndSettings)
        StopCapture
        SetParkValue [-c BNC -v Volts] BNC values 0 - 11 (BNC label A1-A12), -10.0 to +10.0
        UpdateFirmware
        XI2CRead
        XI2CWrite
        ThorDAQ hotkeys:
        Tab: Quick find command
        Up/Down: Quick find latest command
        Ctrl+C: Quick stop threads
        Note: Data is written and data read is displayed from low to high address order";

        readonly int MAX_BAR_INDEX = 3;
        readonly int MIN_BAR_INDEX = 0;

        ObservableCollection<string> consoleOutput = new ObservableCollection<string>() { "<<< ThorDAQ Console ver. 1.7.0 (dFLIM4002-NWL)\n<<< Copyright 2016-2024 thorlabs imaging research group" };
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

        // set the FMC126 CPLD/AD9517 Chip config
        public void dFSetFrontEndSettings()
        {
            uint status;
            status = Win32.ThorDAQAPIdFLIMSetFrontEndSettings(MasterBoardIndex);
            if (status != Win32.STATUS_SUCCESSFUL)
            {
                UpdateConsoleStatus("error ThorDAQAPIdFLIMSetFrontEndSettings " + status.ToString() + " Status " + status.ToString("X"));
            }
        }

        // reset the FMC126 register settings
        public void dFLIMReSync()
        {
            uint status;
            status = Win32.ThorDAQAPIdFLIMReSync(MasterBoardIndex);
            if (status != Win32.STATUS_SUCCESSFUL)
            {
                UpdateConsoleStatus("error configuring FMC126 " + status.ToString() + " Status " + status.ToString("X"));
            }
        }

        // Get dFLIM FMC126 clock frequencies
        public void GetClockFrequency()
        {
            uint status;
            double freq;
            // read and display all available clocks
            for (int clkIndex = 0; clkIndex < 5; clkIndex++)  // total of 6 clocks; 200 MHz FPGA sys clock is first
            {
                status = Win32.ThorDAQAPIdFLIMGetClockFrequency(MasterBoardIndex, clkIndex, out freq);
                if (status == Win32.STATUS_SUCCESSFUL)
                {
                    UpdateConsoleStatus("dFLIM Clk " + clkIndex.ToString() +  ":\t" + freq.ToString() + " MHz");
                }
                else
                    UpdateConsoleStatus("error reading ClkIndex " + clkIndex.ToString() + " Status " + status.ToString("X"));
            }
        }
            //Get Board configuration
        public void GetBoardConfiguration()
        {
            BOARD_INFO_STRUCT BoardConfig = new BOARD_INFO_STRUCT();
            try
            {
                if (Win32.GetBoardCfg(devInstance, ref BoardConfig) == Win32.STATUS_SUCCESSFUL)
                {
                    UpdateConsoleStatus("Board Configuration Information:");
                    UpdateConsoleStatus("Win10 Kernel Driver Version = "
                        + BoardConfig.DriverVersionMajor.ToString() + "."
                        + BoardConfig.DriverVersionMinor.ToString() + "."
                        + BoardConfig.DriverVersionSubMinor.ToString() + ".");
                    UpdateConsoleStatus("FPGA Firmware Version = 0x" + BoardConfig.UserVersion.ToString("X4") + " PCI ProductID 0x" +  BoardConfig.PCIVendorDeviceID.ToString("X4"));
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

        const string HELP_APIReadWriteEEPROM_MSSAGE =
@"
API Functions to read one or more device EEPROMs with file write option
Hardware supported EEPROM devices for this board can be discovered with APIGetEEPROMlabel(), by Index
Usage Examples:
APIReadEEPROM  (no args - attempts to read/display all EEPROM devices hardware supports
APIReadEEPROM -f (reads all and creates/overwrites to file c:\temp\DFLIMeeproms.txt)
APIReadEEPROM -l FMC   (reads FMC126's mezzanine EEPROM)
APIReadEEPROM -l DAC -f (reads DAC's mezz. EERPOM and creates or updates File)
APIWriteEEPROM -l dac DACnnnnnnnnnnnnnnnnnnnnnnnnn  (Writes command line sting bytes 'DACn...' (max 64) to the EEPROM
APIWriteEEPROM -f -l ab2 AB2nnnnnnnn  (Writes command line sting bytes to EEPROM; if successful updates file
APIWriteEEPROM -c  (reads disk file and writes contents to all EEPROMs - error if no file exists)
APIReadProdSN -f (reads all and creates/overwrites to file c:\temp\DF_ProdSN.txt)
APIWriteProdSN -l dac -i ProdID -s ProdSN (ommitting -i or -s leaves field unchanged)
APIWriteProdSN -c (reads disk file and writes contents to 
";

        string ReadSingleEEPROM(string EEPROMlabel, UInt32 UseFile, UInt32 FieldCode)  // 0 entire EEPROM contents, 1 ProdID 2 ProdSN
        {
            string RetBuf = EEPROMlabel + " error"; // EEPROM string buf contents placeholder
            UInt32 status = 0;
            ASCIIEncoding ascii = new ASCIIEncoding();  // for converting EEPROM bytes to ASCII
            const int LabelByteCNT = 16;
            const int EEPROMByteCNT = 65;
            IntPtr UnManagedLabelBuf = IntPtr.Zero;
            IntPtr UnManagedEEPROMBuf = IntPtr.Zero;
            IntPtr UnManagedProdIDbuf = IntPtr.Zero;
            IntPtr UnManagedProdSNbuf = IntPtr.Zero;

            Int32 EEPROMreadLen = 64;  // default, read entire EEPROM

            // prepare NonMangagedBuff for DLL transfer
            Win32.AllocateBuffer(ref UnManagedLabelBuf, LabelByteCNT);
            Win32.AllocateBuffer(ref UnManagedEEPROMBuf, EEPROMByteCNT);
            Win32.AllocateBuffer(ref UnManagedProdIDbuf, EEPROMByteCNT); // (sub fields in EEPROM)
            Win32.AllocateBuffer(ref UnManagedProdSNbuf, EEPROMByteCNT);
            char[] EEPromLabelBytes = new char[LabelByteCNT]; // only 4 needed
            byte[] EEPromBytes = new byte[EEPROMByteCNT];


                                                                                 // code the 3-char label for DLL function
            EEPromLabelBytes = EEPROMlabel.ToCharArray();
            for (int i = 0; i < 3; i++)
                System.Runtime.InteropServices.Marshal.WriteByte(UnManagedLabelBuf, i, (byte)EEPromLabelBytes[i]);

            // Now call the DLL to read the EEPROM
            if (FieldCode == 0)  // entire EEPROM contents
            {
                status = Win32.ThorDAQAPIReadEEPROM(MasterBoardIndex, UseFile, UnManagedLabelBuf, UnManagedEEPROMBuf);
                if (status == 0)
                {
                    for (int CPPbyteIndx = 0; CPPbyteIndx < EEPROMreadLen; CPPbyteIndx++)
                    {
                        EEPromBytes[CPPbyteIndx] = Marshal.ReadByte(UnManagedEEPROMBuf, CPPbyteIndx);
                    }
                    RetBuf = ascii.GetString(EEPromBytes);
                }
            }
            else
            {
                status = Win32.ThorDAQAPIReadProdSN(MasterBoardIndex, UseFile, UnManagedLabelBuf, UnManagedProdIDbuf, UnManagedProdSNbuf);
                if (status == 0) // concatenate the strings for simplicity of utility
                {
                    for (int CPPbyteIndx = 0; CPPbyteIndx < 10; CPPbyteIndx++)
                    {
                        EEPromBytes[CPPbyteIndx] = Marshal.ReadByte(UnManagedProdIDbuf, CPPbyteIndx);
                    }
                    EEPromBytes[10] = 45; // hyphen
                    for (int CPPbyteIndx = 0; CPPbyteIndx < 6; CPPbyteIndx++)
                    {
                        EEPromBytes[CPPbyteIndx+11] = Marshal.ReadByte(UnManagedProdSNbuf, CPPbyteIndx);
                    }

                    RetBuf = ascii.GetString(EEPromBytes);
                }
            }

            // free buffers
            Win32.FreeBuffer(UnManagedLabelBuf);
            Win32.FreeBuffer(UnManagedEEPROMBuf);
            Win32.FreeBuffer(UnManagedProdIDbuf);
            Win32.FreeBuffer(UnManagedProdSNbuf);
            return RetBuf; // EEPROM contents or def. error
        }


        public void APIReadEEPROM(List<String> argumentsList)
        {
            Int32 Index;
            //Int32 EEPROMreadLen, EEPROMstartByte;
            string EEPROMlabel = null;
            UInt32 UseFile = 0; // default, don't use disk file (in C++ DLL)
            string sEEPROMBuf; // a single EEPROM string buf contents placeholder

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-l":  // label arg must follow
                        EEPROMlabel = argumentsList[++argNum].ToUpper();  // access next arg
                        break;
                    case "-f":
                        UseFile = 1; // disk operation
                        break;

                    default:
                        UpdateConsoleStatus( HELP_APIReadWriteEEPROM_MSSAGE);
                        return;

                }
            }

            if (EEPROMlabel == null)  // read ALL EEPROMs!
            {
                for (Index = 0; Index < 8; Index++)  // max Index set by hardware
                {
                    // get the EEPROM device label by index
                    EEPROMlabel = APIGetEEPROMlabel(new List<string> { "unused", Index.ToString(), "-q" });
                    sEEPROMBuf = ReadSingleEEPROM(EEPROMlabel, UseFile, 0); // entire EEPROM contents
                    UpdateConsoleStatus(sEEPROMBuf);
                }
            }
            else   // Read SINGLE EEPROM
            {
                sEEPROMBuf = ReadSingleEEPROM(EEPROMlabel, UseFile, 0);
                UpdateConsoleStatus(sEEPROMBuf);

            }
        }


        public void APIWriteEEPROM(List<String> argumentsList)
        {
            string EEPROMlabel = null;
            string sEEPROMContentsToWrite = null;
            UInt32 UseFile = 0; // default, don't use disk file (in C++ DLL)
            Int32 EEPROMwriteLen = 0;
            const int LabelByteCNT = 16;
            const int EEPROMByteCNT = 65;
            char[] EEPromLabelBytes = new char[LabelByteCNT]; // only 4 needed
            char[] EEPromChars = new char[EEPROMByteCNT];
            IntPtr UnManagedLabelBuf = IntPtr.Zero;
            IntPtr UnManagedEEPROMBuf = IntPtr.Zero;

            try
            {
                for (int argNum = 1; argNum < argumentsList.Count; argNum++)
                {
                    switch (argumentsList[argNum])
                    {
                        case "-l":  // label arg must follow
                            EEPROMlabel = argumentsList[++argNum].ToUpper();  // access next arg
                            // contents to write must follow lable
                            sEEPROMContentsToWrite = argumentsList[++argNum].ToUpper();
                            EEPROMwriteLen  = sEEPROMContentsToWrite.Length;
                            // code the 3-char label for DLL function
                            EEPromLabelBytes = EEPROMlabel.ToCharArray();
                            Win32.AllocateBuffer(ref UnManagedLabelBuf, LabelByteCNT);
                            for (int i = 0; i < 3; i++)
                                System.Runtime.InteropServices.Marshal.WriteByte(UnManagedLabelBuf, i, (byte)EEPromLabelBytes[i]);
                            break;
                        case "-f":
                            UseFile = 1; // disk operation - if WRITE is successful, disk file will be updated
                            break;
                        case "-c":  
                            UseFile = 2; // disk operation - use existing disk file for ALL EEPROM writes, does NOT update file
                            break;

                        default:
                            UpdateConsoleStatus(HELP_APIReadWriteEEPROM_MSSAGE);
                            return;

                    }
                }
            }
            catch( Exception e)
            {
                UpdateConsoleStatus(HELP_APIReadWriteEEPROM_MSSAGE + "\n" + e.Message);
                return;
            }



            string RetBuf = EEPROMlabel + " error"; // EEPROM string buf contents placeholder
            UInt32 status = 0;
            ASCIIEncoding ascii = new ASCIIEncoding();  // for converting EEPROM bytes to ASCII

            // prepare NonMangagedBuff for DLL transfer (bi-directional)
            Win32.AllocateBuffer(ref UnManagedEEPROMBuf, EEPROMByteCNT);

            if(UseFile == 2)  // caller wants to read from existing disk file, and write those contents to EEPROM hardware
            {

                // call the DLL to WRITE the EEPROM 
                //           We are NOT passing EEPROM data to API - it comes from file
                status = Win32.ThorDAQAPIWriteEEPROM(MasterBoardIndex, UseFile, UnManagedLabelBuf, IntPtr.Zero);
                if (status != 0) UpdateConsoleStatus("does readable file exist, and are all cables/cards plugged in?");
            }
            else // we are passing string to write to EEPROM
            {
                // copy the string to unmanaged mem
                EEPromChars = sEEPROMContentsToWrite.ToCharArray();
                for (int i = 0; i < EEPROMwriteLen; i++)  // user's byte count to write (from 0 position)
                    System.Runtime.InteropServices.Marshal.WriteByte(UnManagedEEPROMBuf, i, (byte)EEPromChars[i]);
                for( int iPad= EEPROMwriteLen; iPad < 64; iPad++) // fill remaining buffer with spaces
                {
                    System.Runtime.InteropServices.Marshal.WriteByte(UnManagedEEPROMBuf, iPad, 32);
                }

                // send the EEPROM device label and contents to write - with FileOp flag
                status = Win32.ThorDAQAPIWriteEEPROM(MasterBoardIndex, UseFile, UnManagedLabelBuf, UnManagedEEPROMBuf);
            }


            // if read is successful, process EEPROM bytes
            if (status != 0)
            {
                UpdateConsoleStatus("Error on  ThorDAQAPIWriteEEPROM(), code 0x" + status.ToString("X"));
            }
            // free buffers
            Win32.FreeBuffer(UnManagedLabelBuf);
            Win32.FreeBuffer(UnManagedEEPROMBuf);
            return;
        }


        public void APIReadProdSN(List<String> argumentsList)
        {
            Int32 Index;
            //Int32 EEPROMreadLen, EEPROMstartByte;
            string EEPROMlabel = null;
            UInt32 UseFile = 0; // default, don't use disk file (in C++ DLL)
            string sProdID = null, sProdSN = null;

            try
            {
                for (int argNum = 1; argNum < argumentsList.Count; argNum++)
                {
                    switch (argumentsList[argNum])
                    {
                        case "-l":  // label arg must follow
                            EEPROMlabel = argumentsList[++argNum].ToUpper();  // access next arg
                            break;
                        case "-f":
                            UseFile = 1; // disk operation
                            break;

                        case "-i":  //  -i ProdID
                            sProdID = argumentsList[++argNum];
                            break;

                        case "-s":  // -s ProdSN
                            sProdSN = argumentsList[++argNum];
                            break;

                        default:
                            UpdateConsoleStatus(HELP_APIReadWriteEEPROM_MSSAGE);
                            return;

                    }
                }
            }
            catch(Exception e)
            {
                UpdateConsoleStatus(HELP_APIReadWriteEEPROM_MSSAGE + "\n" + e.Message);
                return;
            }

            string sEEPROMBuf;
            if (EEPROMlabel == null)  // read ALL EEPROMs!
            {
                for (Index = 0; Index < 8; Index++)  // max Index set by hardware
                {
                    // get the EEPROM device label by index
                    EEPROMlabel = APIGetEEPROMlabel(new List<string> { "unused", Index.ToString(), "-q" });
                    sEEPROMBuf = ReadSingleEEPROM(EEPROMlabel, UseFile, 3); // get both ProdID and ProdSN
                    UpdateConsoleStatus(sEEPROMBuf);
                }
            }
            else   // Read SINGLE EEPROM
            {
                sEEPROMBuf = ReadSingleEEPROM(EEPROMlabel, UseFile, 3);
                UpdateConsoleStatus(sEEPROMBuf);

            }
        }


        // requires 0-based index of an EEPROM; used to query the hardware as to how many devices - and their 3-char label - are supported
        public string APIGetEEPROMlabel(List<String> argumentsList)
        {
            UInt32 status;
            Int32 Index;
            bool bQuietMode = false; // to supress UpdateConsole
            NumberStyles styles = NumberStyles.Integer;
            IntPtr UnManagedBuf = IntPtr.Zero;
            if (argumentsList.Count < 2)
            {
                UpdateConsoleStatus("Error: insufficient command line args, need 0-based index");
                return "error";
            }
            Index = Int32.Parse(argumentsList[1], styles);
            Win32.AllocateBuffer(ref UnManagedBuf, 16);

            if(argumentsList.Count > 2 && argumentsList[2] == "-q")
            {
                bQuietMode = true;
            }
            status = Win32.ThorDAQAPIGetEEPROMlabel(MasterBoardIndex, Index, UnManagedBuf);
            if( status != 0)
            {
                UpdateConsoleStatus("Error on index " + Index + " status code = 0x" + status.ToString("X"));
                Win32.FreeBuffer(UnManagedBuf);
                return "error";
            }
            // convert raw DLL data to managed bytes
            byte[] EEPROMbytes = new byte[16];  // only 4 needed
            ASCIIEncoding ascii = new ASCIIEncoding();  // for converting EEPROM bytes to ASCII

            for (int CPPbyteIndx = 0; CPPbyteIndx < 4; CPPbyteIndx++)
            {
                EEPROMbytes[CPPbyteIndx] = Marshal.ReadByte(UnManagedBuf, CPPbyteIndx);
            }
            Win32.FreeBuffer(UnManagedBuf);  // done with C++/DLL memory buff
            string rawBuf = ascii.GetString(EEPROMbytes);  

            if( !bQuietMode) UpdateConsoleStatus(rawBuf);
            return rawBuf;
        }


        //                    requires two arguments - the 0-11 channel num, and -10.0 to +10.0 voltage
        // usage:  SetDAC_ParkValue [-c ChanNum] [-v ParkVoltage]
        public void SetDAC_ParkValue(List<String> argumentsList)
        {
            UInt32 status;
            double dParkVoltage = 0;
            UInt32 chan = 0;
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
                        chan = UInt32.Parse(argumentsList[argNum + 1], styles);
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
            status = Win32.ThorDAQAPISetDACParkValue(MasterBoardIndex, chan, dParkVoltage);
            //int iStatus = SetDACParkValue((UInt32)MasterBoardIndex, (UInt32)chan, dParkVoltage);
            if (status != 0)
            {
                UpdateConsoleStatus("Error: SetDAC_ParkValue() ");
            }
        }


// test function for image DMA and galvo operation - does not render any DMA image data returned to this tester
        public void LiveCapture(List<String> argumentsList)
        {
            if (_bw.IsBusy)
            {
                UpdateConsoleStatus("LiveCapture is Busy");
                return;
            }
            if (_acquisitionActiveFlag == false)
            {
                _acquisitionActiveFlag = true;
            }
            //_liveCaptureWindow.DataContext = this;
            string[] image_params_args_array = argumentsList[2].Split('x');
            BitmapWidth = Convert.ToUInt16(image_params_args_array[0]);
            BitmapHeight = Convert.ToUInt16(image_params_args_array[1]);

            Bitmap = new WriteableBitmap(BitmapWidth, BitmapHeight, 96, 96, PixelFormats.Gray8, null);
            ImageSource = new short[BitmapWidth * BitmapHeight];
            ImageSourceByte = new byte[BitmapWidth * BitmapHeight];

            LiveCaptureDDR3(argumentsList);

        }

        public void LoadScript()
        {
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

        public void ReadBoardMemoryData(List<String> argumentsList)
        {
            IntPtr buffer = IntPtr.Zero;
            try
            {
                UInt16 value = (UInt16)Convert.ToInt32(argumentsList[1]);
                if (value <= MAX_BAR_INDEX && value >= MIN_BAR_INDEX)
                {
                    UInt32 barNum = value; // get bar number
                    int registerAddress = (int)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[2]);
                    int transferDataLength = (int)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[3]);
                    if (buffer == IntPtr.Zero)
                    {
                        Win32.AllocateBuffer(ref buffer, (uint)transferDataLength);
                    }
                    // INVOKE DLL to Kernel driver
                    if (Win32.MemoryRead(0, buffer, barNum, (UInt64)registerAddress, 0, (UInt64)transferDataLength)) 
                    {
                        List<byte> result = new List<byte>();
                        for (int m = 0; m < (int)transferDataLength; m++)
                        {
                            result.Add(System.Runtime.InteropServices.Marshal.ReadByte(buffer, transferDataLength - m - 1));
                        }
                        UpdateConsoleStatus("0x" + BitConverter.ToString(result.ToArray()));
                    }
                    Win32.FreeBuffer(buffer);
                }
            }
            catch (Exception)
            {
                if (buffer != IntPtr.Zero)
                {
                    Win32.FreeBuffer(buffer);
                }
                UpdateConsoleStatus("Readmem Error");
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
                            if (argumentsList.Count > 1) // help <additional command>?
                            {
                                if (argumentsList[1].ToLower() == "xi2cread" || argumentsList[1].ToLower() == "xi2cwrite")
                                {
                                    UpdateConsoleStatus(HELP_XI2CReadWrite_MSSAGE);
                                    break;
                                }
                                else if(argumentsList[1].ToLower() == "apireadeeprom"  || argumentsList[1].ToLower() == "apiwriteeeprom")
                                {
                                    UpdateConsoleStatus(HELP_APIReadWriteEEPROM_MSSAGE);
                                }
                            }
                            else
                            {
                                UpdateConsoleStatus(HELP_MSSAGE);
                            }
                            break;

                        case "ddr3memtest":
                            DDR3MemTest(argumentsList);

                            break;

                        case "dfsetfrontendsettings":
                            dFSetFrontEndSettings();
                            break;

                        case "dflimresync":
                            dFLIMReSync();
                            break;
                        case "getclockfrequency":
                            GetClockFrequency();
                            break;

                        case "getboardcfg":
                            {
                                GetBoardConfiguration();
                            };
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
                        case "readmem":
                            {
                                ReadBoardMemoryData(argumentsList);
                            };
                            break;
                        case "updatefirmware":
                            {
                                if (_bw.IsBusy)
                                {
                                    StopAcquisition();
                                }
                                UpdateFirmware();
                            };
                            break;
                        case "updatecpld": 
                            {
                                if (_bw.IsBusy) 
                                {
                                    StopAcquisition();
                                }
                                UpdateCPLD();
                            };
                            break;
                        case "load":
                            {
                                LoadScript();
                            };
                            break;
                        case "packetread":
                            {
                                if (_bw.IsBusy)
                                {
                                    StopAcquisition();
                                }
                                _isFileSavedEnabled = true;
//                                ReadAddressablePacket(argumentsList);
                            };
                            break;
                        case "clear":
                            {
                                ClearConsole();
                            };
                            break;
                        case "livecapture":
                            {
                                LiveCapture(argumentsList);
                            };
                            break;
                        case "apigeteepromlabel":
                            APIGetEEPROMlabel(argumentsList);

                            break;
                        case "apireadeeprom":
                            APIReadEEPROM(argumentsList);
                            break;
                        case "apiwriteeeprom":
                            APIWriteEEPROM(argumentsList);
                            break;

                        case "apireadprodsn":
                            APIReadProdSN(argumentsList);
                            break;

                        case "stopcapture": 
                                StopAcquisition();
                            break;
                        case "setparkvalue":
                            if (_bw.IsBusy)
                            {
                                StopAcquisition();
                            }
                            SetDAC_ParkValue(argumentsList);
                            break;

                        case "setpacketmodeaddressable":
                            {
                                SetPacketModeAddressable();
                            };
                            break;
                        case "getallfpgaregisters":
                            {
                                GetAllFPGARegisters(argumentsList);
                            }
                            break;
                        case "getfpgareg":
                            {
                                GetFPGAreg(argumentsList);
                            }
                            break;
                        case "setfpgareg":
                            {
                                SetFPGAreg(argumentsList);
                            }
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
                        case "writeddr3":
                            {
                                foreach (string Str in WriteDDR3(argumentsList))
                                {
                                    UpdateConsoleStatus(Str);
                                }
                            };
                            break;
                        case "readchannel":
                            {
                                ReadChannel(argumentsList);
                            };
                            break;
                        case "write_mezzanine_synth" :
                            {
                                WriteMezzanineSynth(argumentsList);
                            }
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
                            };
                            break;

                        case "xi2cread":
                            XI2Cread(argumentsList);
                            break;
                        case "xi2cwrite":
                            XI2Cwrite(argumentsList);
                            break;

                        default:
                            break;
                    }
                }
                catch (Exception ex)
                {
                    ex.ToString();
                    UpdateConsoleStatus("Command Style Error");
                }
            }
        }
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

        public void UpdateFirmware()
        {
            //Create OpenFileDialog
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.DefaultExt = ".mcs";
            dlg.Filter = "MCS Files (.mcs)|*.mcs";

            // Display OpenFileDialog by calling ShowDialog method
            Nullable<bool> result = dlg.ShowDialog();

            // Get the selected file name and display in a TextBox
            if (result == true)
            {
                // Open document
                string fileName = dlg.FileName;
                ProgramFPGAFlash(fileName);
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
            }
        }
        public void Delay(List<String> argumentsList) 
        {
            UInt16 value = (UInt16)Convert.ToInt32(argumentsList[1]);
            System.Threading.Thread.Sleep(value);
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
XI2Cread  0x80 0xFF 0x54 -o 0x0 -n 28  
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
                        while (ArgIndx < argumentsList.Count)
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
                status = (THORDAQ_STATUS)Win32.ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_ReadDir, MasterMUXAddr, MasterMUXChan,
                                                                                SlaveMUXAddr, SlaveMUXChan, DeviceAddr, I2CbusHz, PageReadWriteLen,
                                                                                UnManagedOpCodeBuffer, ref OpcodeByteLen,  // Opcode(s) means READ operation
                                                                                UnManagedDataBuffer, ref TransferBufferLen);
                TotalBytesTransfered += TransferBufferLen;
            }
            else      // READ case
            {
                status = (THORDAQ_STATUS)Win32.ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_ReadDir, MasterMUXAddr, MasterMUXChan,
                                                                            SlaveMUXAddr, SlaveMUXChan, DeviceAddr, I2CbusHz, PageReadWriteLen,
                                                                            UnManagedOpCodeBuffer, ref OpcodeByteLen,  // Opcode(s) means READ operation
                                                                            UnManagedDataBuffer, ref TransferBufferLen); // ret. value includes OpCode len
                TotalBytesTransfered += TransferBufferLen;
            }
            if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                string sStatus = "I2C NAK - no device, check hardware with debugger";
                if (status == THORDAQ_STATUS.STATUS_I2C_TIMEOUT_ERR)
                    sStatus = "I2C Timeout";
                UpdateConsoleStatus(" ThorDAQAPIXI2CReadWrite() I2C write failure " + sStatus + " [bytes transfered?:] " + TotalBytesTransfered.ToString());
            }

            Marshal.FreeHGlobal(UnManagedOpCodeBuffer);
            Marshal.FreeHGlobal(UnManagedDataBuffer);
        }

        // call the API for DDR3 memory test
        public void DDR3MemTest(List<String> argumentsList)
        {
            const int ErrStringSize = 200;
            uint status;
            // convoluted buffers and code to get string back from C++
            IntPtr UnManagedErrStringBuffer = Marshal.AllocHGlobal(ErrStringSize);
            char[] ErrMsg = new char[ErrStringSize];

            status = Win32.ThorDAQAPIMemTest(MasterBoardIndex, UnManagedErrStringBuffer);
            // if status is SUCCESS we don't care about return string
            if (status == 0)
            {
                UpdateConsoleStatus("SUCCESS");
            }
            else // we need to translate the returned string
            {
                int i;
                for(i=0; i< 130; i++)
                {
                    ErrMsg[i] = (char)Marshal.ReadByte(UnManagedErrStringBuffer, i);
                }
                UpdateConsoleStatus(new string(ErrMsg));
            }
            Marshal.FreeHGlobal(UnManagedErrStringBuffer);
        }

        public void WriteBoardMemoryData(List<String> argumentsList)
        {
            IntPtr buffer = IntPtr.Zero;
            try
            {
                UInt16 value = (UInt16)Convert.ToInt32(argumentsList[1]);
                if (value <= MAX_BAR_INDEX && value >= MIN_BAR_INDEX)
                {
                    UInt32 barNum = value; // get bar number
                    int registerAddress = (int)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[2]);
                    int transferDataLength = (int)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[3]);
                    if (buffer == IntPtr.Zero)
                    {
                        Win32.AllocateBuffer(ref buffer, (uint)transferDataLength);
                    }
                    string buffer_hexString = argumentsList[4];
                    buffer_hexString = buffer_hexString.Replace("0x", string.Empty);
                    for (int i = 0; i < transferDataLength * 2 - buffer_hexString.Length; i++)
                    {
                        buffer_hexString = '0' + buffer_hexString;
                    }
                    for (int i = 0; i < transferDataLength; i++)
                    {
                        byte byteValue = byte.Parse(buffer_hexString.Substring(i * 2, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture);
                        System.Runtime.InteropServices.Marshal.WriteByte(buffer, transferDataLength - i - 1, byteValue);
                    }
                    if (Win32.MemoryWrite(0, buffer, barNum, (UInt64)registerAddress, 0, (UInt64)transferDataLength))
                    {
                        UpdateConsoleStatus("Write register successfully");
                    }
                    else
                    {
                        UpdateConsoleStatus("Writemem Error");
                    }
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
                InputBlock.Clear() ;
            }
            _last_recorded_tabbed_command = string.Empty;
            _last_recorded_tabbed_command_index = 0;
        }

        void MainWindow_Closed(object sender, EventArgs e)
        {
            if (_bw.IsBusy)
            {
                StopAcquisition();
            }
            ReleaseHandles();
            InputBlock.PreviewKeyDown -= InputBlock_PreviewKeyDown;
            uint status = Win32.DisconnectFromBoard(0);

            Application.Current.Shutdown();
        }

        void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            uint status = 0;
            BOARD_INFO_STRUCT BoardInfo = new BOARD_INFO_STRUCT();
            try
            {
                XmlDocument xmlDoc = new XmlDocument();
                xmlDoc.Load("ThorDAQSettings.xml");
                XmlNode node = xmlDoc.SelectSingleNode("/ThorDAQSettings/File");
                SettingPath = node.Attributes["Path"].Value.ToString();
                status = Win32.DsConnectToBoard(0, ref BoardInfo);
                if (status == Win32.STATUS_SUCCESSFUL)
                {
                    devInstance = 0;
                    ThorDAQAdapters[0] = new TDcontroller(0, "dFLIM_4002");
           //         UpdateConsoleStatus("dFLIM is connected, the current version is " + BoardInfo.DriverVersionMajor.ToString() + "." + BoardInfo.DriverVersionMinor.ToString() + "." + BoardInfo.DriverVersionSubMinor.ToString());
                    GetBoardConfiguration();

                    UpdateConsoleStatus(HELP_MSSAGE);
                    InitHandles();
                }
                else
                {
                    //throw new NullReferenceException();
                    UpdateConsoleStatus("Device doesn't exist");
                }
            }
            catch (Exception error)
            {
                MessageBox.Show(error.Message);
                this.Close();
            }
        }

        #endregion Methods
    }
}