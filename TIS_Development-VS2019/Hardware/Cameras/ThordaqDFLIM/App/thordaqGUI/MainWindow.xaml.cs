namespace thordaqGUI
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Globalization;
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

        public static readonly string[] THORDAQ_COMMANDLINE_COMMAND = 
        {
            "ReadMem",
            "WriteMem",
            "GetBoardCfg",
            "UpdateFirmware",
            "UpdateCPLD",
            "Load",
            "PacketRead",
            "ReadChannel",
            "ReadDRam",
            "WriteDRam",
            "Clear",
            "LiveCapture",
            "Stop",
            "WriteDram",
            "WriteSPI",
            "Delay"
        };

        /// <summary>Instance number for this board. </summary>
        public UInt16 devInstance;

        const string HELP_MSSAGE = @"ThorDAQ commands:
        ReadMem <BarNum> <Card Offset> <Length>
        WriteMem <BarNum> <Card Offset> <Length> <Data...>
        WriteSPI <index> <address> <data> %index: 0 -> clock synthsizer; 1-3 -> adc converter
        PacketRead <Channel> <HSize_VSize> <Number> <FrameRate>
        ReadChannel <Channel> <Size in bytes>
        ReadDRam <CardOffset> <TranferSize>
        WriteDRam <CardOffset> <Data>
        LiveCapture <Channel> <HSize_VSize>
        GetBoardCfg
        Stop
        UpdateFirmware
        UpdateCPLD
        ThorDAQ hotkeys:
        Tab: Quick find command
        Up/Down: Quick find latest command
        Ctrl+C: Quick stop threads
        Note: Data is written and data read is displayed from low to high address order";

        readonly int MAX_BAR_INDEX = 3;
        readonly int MIN_BAR_INDEX = 0;

        ObservableCollection<string> consoleOutput = new ObservableCollection<string>() { "<<< ThorDAQ Console\n<<< Copyright 2016 thorlabs imaging research group" };
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

        //Get Board configuration
        public void GetBoardConfiguration()
        {
            BOARD_INFO_STRUCT BoardConfig = new BOARD_INFO_STRUCT();
            try
            {
                if (Win32.GetBoardCfg(devInstance, ref BoardConfig) == Win32.STATUS_SUCCESSFUL)
                {
                    UpdateConsoleStatus("Board Configuration Information:");
                    UpdateConsoleStatus("Driver Version = "
                        + BoardConfig.DriverVersionMajor.ToString() + "."
                        + BoardConfig.DriverVersionMinor.ToString() + "."
                        + BoardConfig.DriverVersionSubMinor.ToString() + ".");
                    UpdateConsoleStatus("User Version = 0x" + BoardConfig.UserVersion.ToString("X4"));
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
            string[] image_params_args_array = argumentsList[2].Split('_');
            BitmapWidth = Convert.ToUInt16(image_params_args_array[0]);
            BitmapHeight = Convert.ToUInt16(image_params_args_array[1]);
            Bitmap = new WriteableBitmap(BitmapWidth, BitmapHeight, 96, 96, PixelFormats.Gray8, null);
            ImageSource = new short[BitmapWidth * BitmapHeight];
            ImageSourceByte = new byte[BitmapWidth * BitmapHeight];
            if (argumentsList.Count > 3)
            {
                argumentsList.RemoveAt(3);
                argumentsList.Insert(3, "4294967295 ");
            }
            else
            {
                argumentsList.Add("4294967295 ");
            }
            ReadAddressablePacket(argumentsList);
            StartAcqusition();
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
                                ReadAddressablePacket(argumentsList);
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
                        case "stop": 
                            {
                                StopAcquisition();
                            };
                            break;
                        case "setpacketmodeaddressable":
                            {
                                SetPacketModeAddressable();
                            };
                            break;
                        case "readdram": 
                            {
                                ReadDram(argumentsList);
                            };
                            break;
                        case "writedram":
                            {
                                WriteDram(argumentsList);
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
                            };break;
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
                    UpdateConsoleStatus("ThorDAQ is connected, the current version is " + BoardInfo.DriverVersionMajor.ToString() + "." + BoardInfo.DriverVersionMinor.ToString() + "." + BoardInfo.DriverVersionSubMinor.ToString());
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