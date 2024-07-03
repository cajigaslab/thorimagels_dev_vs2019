using System.Windows;
using System.Windows.Controls;
using ThorDAQConfigControl.ViewModel;
using System;
using System.Windows.Input;
using System.Globalization;
using ThorLogging;
using System.Diagnostics;
using ThorDAQConfigControl.Model;
using System.Xml;
using System.IO;
using System.Threading.Tasks;
using TIDP.SAA;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using Telerik.Windows.Controls;
using System.Deployment.Application;

namespace ThorDAQConfigControl.View
{
    /// <summary>
    /// Interaction logic for MainContent.xaml
    /// </summary>
    public partial class MainContent : UserControl
    {
        private ThorDAQCommandProvider commandProvider;
        private MainContentViewModel vm;
        private int NumThorDAQboards;
        private int _last_recorded_command = 0;
        private string _last_recorded_tabbed_command = string.Empty;
        private int _last_recorded_tabbed_command_index = 0;
        private TIBusAdapters TIusbAdapters = new TIBusAdapters(); // Not know whether still needed?

        enum TDexitCode
        {
            Success = 0,
            NoBoardDetected = 1,
            UnknownError = 10
        };

        public MainContent()
        {
            InitializeComponent();
            vm = MainContentViewModel.GetInstance();
            this.DataContext = vm;

            // Init some UI elements
            InitGUI();

            // Init commandProvider
            commandProvider = new ThorDAQCommandProvider();
            ThorDAQCommandProvider.UpdateConsoleStatusDel updateConsole = (input) =>
              {
                  Application.Current?.Dispatcher.BeginInvoke(new Action(() =>
                  {
                      UpdateConsoleStatus(input);
                  }));
              };
            commandProvider.SetUpdateConsoleStatusDelegate(updateConsole);

            vm.SetCommandProvider(commandProvider);

            InputBlock.Focus();
            this.Loaded += MainContent_Loaded;
        }

        private void InitGUI()
        {
            // Add combo box items for channel selection 0~11
            for (int i = 0; i <= 11; i++)
            {
                var item = new RadComboBoxItem();
                item.Content = i;
                channelListBox.Items.Add(item);
            }
            channelListBox.SelectedIndex = 0;
        }

        ~MainContent()
        {
            InputBlock.PreviewKeyDown -= InputBlock_PreviewKeyDown;
            uint status = ThorDAQCommandProvider.ThorDAQAPIReleaseBoard(0);
        }

        public void ShowHideCommandLine(bool toShow)
        {
            if (toShow)
            {
                Grid.SetRowSpan(this.responseDisplayArea, 2);
                this.commandLineArea.Visibility = Visibility.Visible;
                this.responseDisplayArea.BorderThickness = new Thickness(2, 0, 0, 2);

                UpdateConsoleStatus(CommonDefinition.HELP_MSSAGE); // Show available commands only when command line panel displayed
            }
            else
            {
                Grid.SetRowSpan(this.responseDisplayArea, 3);
                this.commandLineArea.Visibility = Visibility.Collapsed;
                this.responseDisplayArea.BorderThickness = new Thickness(2, 0, 0, 0);
            }
        }

        public void ShowHideConfigureArea(bool toShow)
        {
            if (toShow)
            {
                Grid.SetRowSpan(this.buttonsArea, 1);
                this.buttonsArea.BorderThickness = new Thickness(0, 0, 2, 2);
                this.systemconfigArea.Visibility = Visibility.Visible;
                this.waveformArea.Visibility = Visibility.Visible;
            }
            else
            {
                Grid.SetRowSpan(this.buttonsArea, 3);
                this.buttonsArea.BorderThickness = new Thickness(0, 0, 2, 0);
                this.systemconfigArea.Visibility = Visibility.Collapsed;
                this.waveformArea.Visibility = Visibility.Collapsed;
            }
        }

        #region GUI operations

        private void InputBlock_PreviewKeyDown(object sender, KeyEventArgs e)
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
                    vm.ConsoleOutput.Add(">>> " + input);
                    Scroller.ScrollToBottom();
                    UpdateConsoleCommand(input);
                    _last_recorded_command = vm.ConsoleOutput.Count - 1;
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
                for (int i = _last_recorded_tabbed_command_index; i < CommonDefinition.THORDAQ_COMMANDLINE_COMMAND.Count(); i++)
                {
                    if (Regex.IsMatch(CommonDefinition.THORDAQ_COMMANDLINE_COMMAND[i], _last_recorded_tabbed_command, RegexOptions.IgnoreCase))
                    {
                        InputBlock.Text = CommonDefinition.THORDAQ_COMMANDLINE_COMMAND[i];
                        _last_recorded_tabbed_command_index++;
                        if (_last_recorded_tabbed_command_index >= CommonDefinition.THORDAQ_COMMANDLINE_COMMAND.Count())
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
                        if (vm.ConsoleOutput[i].Contains(">>>"))
                        {
                            InputBlock.Text = vm.ConsoleOutput[i].Substring(3);
                            _last_recorded_command = Math.Max(i - 1, 0);
                            break;
                        }
                    }
                }
            }
            else if (e.Key == Key.Down)
            {
                for (int i = _last_recorded_command; i < vm.ConsoleOutput.Count; i++)
                {
                    if (vm.ConsoleOutput[i].Contains(">>>"))
                    {
                        InputBlock.Text = vm.ConsoleOutput[i].Substring(3);
                        _last_recorded_command = Math.Min(i + 1, vm.ConsoleOutput.Count - 1);
                        break;
                    }
                }
            }
            else if (e.Key == Key.C && (Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control)
            {
                InputBlock.Clear();
            }
            _last_recorded_tabbed_command = string.Empty;
            _last_recorded_tabbed_command_index = 0;
        }


        private void commandBtn_Click(object sender, RoutedEventArgs e)
        {
            switch ((sender as RadButton).Name)
            {
                case "firmwareVersionBtn":
                    commandProvider.GetBoardConfiguration(0);
                    break;
                case "getBOBStatusBtn":
                    commandProvider.APIGetBOBstatus(null); // argumentsList is not used
                    break;
                case "turnOnLEDBtn":
                    commandProvider.APIbreakOutBoxLEDControl(new List<string> { "APIBreakoutBoxLED", "-all", "on" });
                    break;
                case "turnOffLEDBtn":
                    commandProvider.APIbreakOutBoxLEDControl(new List<string> { "APIBreakoutBoxLED", "-all", "off" });
                    break;
                case "getDIOConfigBtn":
                    commandProvider.APIGetDIOConfig(null);
                    break;
                case "getDIOAllBtn":
                    commandProvider.APIGetDIO(new List<string> { "APIGetDIO", "D99" });
                    break;
                case "setPartValueBtn":
                    commandProvider.SetDAC_ParkValue(new List<string> { "SetParkValue", "-c", channelListBox.SelectedIndex.ToString(), "-v", vm.ParkValue.ToString() });
                    break;
                default:
                    vm.ConsoleOutput.Add("<<< Not implemented");
                    break;
            }
            Scroller.ScrollToBottom();
        }
        #endregion

        public void ClearConsole()
        {
            string[] beginString = new string[3];
            if (vm.ConsoleOutput.Count >= 3)
            {
                for (int i = 0; i < 3; i++)
                {
                    beginString[i] = vm.ConsoleOutput[i];
                }
            }
            vm.ConsoleOutput.Clear();
            for (int i = 0; i < 3; i++)
            {
                vm.ConsoleOutput.Add(beginString[i]);
            }
            _last_recorded_command = 0;
            _last_recorded_tabbed_command = string.Empty;
            _last_recorded_tabbed_command_index = 0;
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
                                    switch (argumentsList[1].ToLower())
                                    {
                                        case "buildwaveform":
                                            UpdateConsoleStatus(CommonDefinition.HELP_BuildWaveform_MSSAGE);
                                            break;
                                        case "dacwaveform_setup":
                                            UpdateConsoleStatus(CommonDefinition.HELP_DACWaveform_setup_MSSAGE);
                                            break;
                                        case "globalscan":
                                            UpdateConsoleStatus(CommonDefinition.HELP_GlobalScan_MSSAGE);
                                            break;
                                        case "apigetdioconfig":
                                            UpdateConsoleStatus(CommonDefinition.HELP_APIGetSetDIOConfig_MSSAGE);
                                            break;
                                        case "apisetdioconfig":
                                            UpdateConsoleStatus(CommonDefinition.HELP_APIGetSetDIOConfig_MSSAGE);
                                            break;
                                        case "apisetaioconfig":
                                            UpdateConsoleStatus(CommonDefinition.HELP_APIGetSetAIOConfig_MSSAGE);
                                            break;
                                        case "apigetdio":
                                        case "apisetdo":
                                            UpdateConsoleStatus(CommonDefinition.HELP_APIGetSetDIO_MSSAGE);
                                            break;
                                        case "apigetai":
                                            UpdateConsoleStatus(CommonDefinition.HELP_APIGetAI_MSSAGE);
                                            break;
                                        case "manuftest":
                                            UpdateConsoleStatus(CommonDefinition.HELP_ManufTest_MSSAGE);
                                            break;
                                        case "s2mm_dma_setup":
                                            UpdateConsoleStatus(CommonDefinition.HELP_S2MM_DMA_setup_MSSAGE);
                                            break;
                                        case "adcsampleclock_setup":
                                            UpdateConsoleStatus(CommonDefinition.HELP_ADCSampleClock_setup_MSSAGE);
                                            break;
                                        case "adcstream_setup":
                                            UpdateConsoleStatus(CommonDefinition.HELP_ADCStream_setup_MSSAGE);
                                            break;
                                        case "scancontrol_setup":
                                            UpdateConsoleStatus(CommonDefinition.HELP_ScanControl_setup_MSSAGE);
                                            break;
                                        case "readmem":
                                            UpdateConsoleStatus(CommonDefinition.HELP_ReadMem_MSSAGE);
                                            break;
                                        case "writemem":
                                            UpdateConsoleStatus(CommonDefinition.HELP_WriteMem_MSSAGE);
                                            break;
                                        case "xi2cread":
                                            UpdateConsoleStatus(CommonDefinition.HELP_XI2CReadWrite_MSSAGE);
                                            break;
                                        case "xi2cwrite":
                                            UpdateConsoleStatus(CommonDefinition.HELP_XI2CReadWrite_MSSAGE);
                                            break;
                                        case "apiupdatelft_appfirmware":
                                            UpdateConsoleStatus(CommonDefinition.HELP_UpdateLFT_AppFirmware_MSSAGE);
                                            break;
                                        case "apigetlft_jastatus":
                                        case "apisetlft_jaconfig":
                                            UpdateConsoleStatus(CommonDefinition.HELP_APIgetsetLFT_JA_MSSAGE);
                                            break;
                                        case "apireadeeprom":
                                        case "apiwriteeeprom":
                                            UpdateConsoleStatus(CommonDefinition.HELP_APIReadWriteEEPROM_MSSAGE);
                                            break;
                                    }
                                }
                                else
                                    UpdateConsoleStatus(CommonDefinition.HELP_MSSAGE);
                            }
                            break;
                        case "-?":
                        case "?":
                            UpdateConsoleStatus(CommonDefinition.HELP_MSSAGE);
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
                                    ThorDAQCommandProvider.MasterBoardIndex = uiIndx;
                                    // briefly blink the 2 LEDs on PCIe card backplane
                                    //BlinkAdapterBackplaneLEDs(); // takes 1 second - two blinks of lights
                                }
                                else
                                    UpdateConsoleStatus("Zero-base Board index invalid - total boards " + NumThorDAQboards);

                            }
                            else
                                ThorDAQCommandProvider.MasterBoardIndex = 0;
                            break;
                        case "getboardcfg":
                            commandProvider.GetBoardConfiguration(0);
                            break;
                        case "getfpgareg":
                            commandProvider.GetFPGAreg(argumentsList);
                            break;
                        case "setfpgareg":
                            commandProvider.SetFPGAreg(argumentsList);
                            break;
                        case "getallfpgaregisters":
                            commandProvider.GetAllFPGARegisters(argumentsList);
                            break;

                        case "writemem":
                            commandProvider.WriteBoardMemoryData(argumentsList);
                            break;

                        case "xi2cread":
                            commandProvider.XI2Cread(argumentsList);
                            break;
                        case "xi2cwrite":
                            commandProvider.XI2Cwrite(argumentsList);
                            break;

                        case "readmem":
                            commandProvider.ReadBoardMemoryData(argumentsList);
                            break;
//                        case "dft":
//                            commandProvider.DFT(argumentsList);
//                            break;

                        case "breakoutboxled":
                            commandProvider.ControlBBleds(argumentsList);
                            break;

                        case "apireadeeprom":
                            commandProvider.APIReadEEprom(argumentsList);
                            break;
                        case "apiwriteeeprom":
                            commandProvider.APIWriteEEprom(argumentsList);
                            break;

                        case "apibreakoutboxled":
                            commandProvider.APIbreakOutBoxLEDControl(argumentsList);
                            break;
                        case "apigetddr3status":
                            commandProvider.APIGetDDR3status(argumentsList);
                            break;
                        case "apigetbobstatus":
                            commandProvider.APIGetBOBstatus(argumentsList);
                            break;

                        case "apigetdioconfig":
                            commandProvider.APIGetDIOConfig(argumentsList);
                            break;

                        case "apisetaioconfig":
                            commandProvider.APISetAIOConfig(argumentsList);
                            break;

                        case "apisetdioconfig":
                            foreach (string Str in ThorDAQCommandProvider.APISetDIOConfig(argumentsList))
                                UpdateConsoleStatus(Str);
                            break;
                        case "apisetdo":
                            commandProvider.APISetDO(argumentsList);
                            break;
                        case "apigetdio":
                            commandProvider.APIGetDIO(argumentsList);
                            break;

                        case "apigetai":
                            commandProvider.APIGetAI(argumentsList);
                            break;

                        case "apiupdatebob_cpld":
                            commandProvider.APIupdateBOB_CPLD(argumentsList);
                            break;

                        case "apiupdatelft_cpld":
                            commandProvider.APIupdateLFT_CPLD(argumentsList);
                            break;

                        case "apiupdatelft_appfirmware":
                            commandProvider.UpdateLFT_AppFirmware(argumentsList);
                            break;

                        case "apigetlft_jastatus":
                            foreach (string Str in commandProvider.APIgetLFT_JAstatus(argumentsList))
                                UpdateConsoleStatus(Str);
                            break;
                        case "apisetlft_jaconfig":
                            foreach (string Str in commandProvider.APIsetLFT_JAconfig(argumentsList))
                                UpdateConsoleStatus(Str);
                            break;

                        case "apiprogrammabletrigger":
                            commandProvider.APIProgrammableTrigger(argumentsList);
                            //commandProvider
                            break;

                        case "updatecentralfpgafirmware":
                            commandProvider.UpdateCentralFPGAfirmware(argumentsList);
                            break;
                        case "apiupdatedac_cpld":
                            commandProvider.APIUpdateDAC_CPLD();
                            break;
                        case "buildwaveform":
                            commandProvider.BuildWaveform(argumentsList);
                            break;
  //                      case "manuftest":
  //                          commandProvider.ManufTest(argumentsList);
  //                          break;

                        // NOTE the motivation for making return type a "list" of strings is so
                        // we can call these functions from C# objects in Manfucturing test
                        // (i.e. we don't want to have to instantiate legacy CL-GUI console functions)
                        // there is no need for this with functions NOT used, e.g., in manuf-test classes
                        case "adcstream_setup":
                            foreach (string Str in ThorDAQCommandProvider.ADCStream_setup(argumentsList))
                                UpdateConsoleStatus(Str);
                            break;
                        case "adcsampleclock_setup":
                            foreach (string Str in ThorDAQCommandProvider.ADCSampleClock_setup(argumentsList))
                                UpdateConsoleStatus(Str);
                            break;
                        case "scancontrol_setup":
                            foreach (string Str in ThorDAQCommandProvider.ScanControl_setup(argumentsList))
                                UpdateConsoleStatus(Str);
                            break;
                        case "s2mm_dma_setup":
                            foreach (string Str in ThorDAQCommandProvider.S2MM_DMA_setup(argumentsList))
                                UpdateConsoleStatus(Str);
                            break;
                        case "dacwaveform_setup":
                            foreach (string Str in ThorDAQCommandProvider.DACWaveform_setup(argumentsList))
                                UpdateConsoleStatus(Str);
                            break;
                        case "globalscan":
                            foreach (string Str in ThorDAQCommandProvider.GlobalScan(argumentsList, null))
                                UpdateConsoleStatus(Str);
                            break;
                        //   case "load":
                        //       {
                        //           LoadScript(argumentsList);
                        //       };
                        //       break;
                        case "packetread":
                            commandProvider._isFileSavedEnabled = true;
                            commandProvider.ReadAddressablePacket(argumentsList);
                            break;
                        case "clear":
                            ClearConsole();
                            break;
                        case "loadscript":
                            LoadScript(argumentsList);
                            break;
                        //case "sleep":
                        //SleepMainThread(argumentsList);
                        //break;
                        case "writeddr3":
                            commandProvider.WriteDDR3(argumentsList);
                            break;
                        case "readddr3":
                            byte[] NullByteRef = null;
                            foreach (string Str in ThorDAQCommandProvider.ReadDDR3(argumentsList, ref NullByteRef))
                                UpdateConsoleStatus(Str);
                            break;
                        case "readdram":
                            commandProvider.ReadDram(argumentsList);
                            break;
                        //                        case "writedram":
                        //                          {
                        //                                WriteDram(argumentsList);
                        //                            };
                        //                            break;
                        case "write_mezzanine_synth":
                            commandProvider.WriteMezzanineSynth(argumentsList);
                            break;

                        case "setparkvalue":
                            commandProvider.SetDAC_ParkValue(argumentsList);
                            break;

                        case "writespi":
                            if (commandProvider.WriteSPIBus(argumentsList))
                                UpdateConsoleStatus("Write SPI Bus Successfully");
                            break;
                        case "delay":
                            var value = Convert.ToInt32(argumentsList[1]);
                            System.Threading.Thread.Sleep(value);
                            break;
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
            vm.ConsoleOutput.Add(input);
            //_last_recorded_command = ConsoleOutput.Count - 1;
            Scroller.ScrollToBottom();
        }

        public void UpdateConsoleStatus(String input)
        {
            vm.ConsoleOutput.Add("<<< " + input);
            Scroller.ScrollToBottom();
        }


        void MainContent_Loaded(object sender, RoutedEventArgs e)
        {
            DateTime localDate = DateTime.Now;
            uint status = 0;
            bool bKernelDriverRead = false;
            var culture = new CultureInfo("en-US");
            string lclDate = localDate.ToString(culture);
   //         UInt64 RegVal = 0xFFFFFF;
   //         bool bStatus;

            try
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ThorDAQcl-GUI: MainWindow_Loaded at " + lclDate);
            }
            catch (Exception eRR)
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
            catch (Exception eErr)
            {
                UpdateConsoleStatus(eErr.Message);
            }
            try
            {
                XmlDocument xmlDoc = new XmlDocument();
                xmlDoc.Load("ThorDAQNewSettings.xml");
                XmlNode node = xmlDoc.SelectSingleNode("/ThorDAQSettings/File");

                NumThorDAQboards = vm.CountOfThorDAQboards();  // seach OS collect for Win32_PnPSignedDriver "ThorDAQ" types (all kernel drivers, all product codes)
                Scroller.ScrollToBottom();
                string sMsg = "ThorDAQcl-GUI: Num PCIe boards: " + NumThorDAQboards;
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, sMsg);

                if (NumThorDAQboards > 0)
                {
                    ThorDAQCommandProvider.MasterBoardIndex = 0;  // default until user changes
                    for (uint i = 0; i < NumThorDAQboards; i++)
                    {
                        commandProvider.SettingPath = node.Attributes["Path"].Value.ToString(); //ImageAcquiringThreadMethod()???

                        status = Win32.ThorDAQAPIBindBoard(i, ref BoardInfo); // goes to dllmain entry point

                        if (status == Win32.STATUS_SUCCESSFUL) // at least one board found?
                        {
                            // board connected (instantiated)...
                            if (bKernelDriverRead == false) // (one kernel driver detects ALL boards)
                            {
                                bKernelDriverRead = true;
                                UpdateConsoleStatus("ThorDAQ adapter connected, KERNEL DRIVER source version: " + BoardInfo.DriverVersionMajor.ToString() + "." + BoardInfo.DriverVersionMinor.ToString() + "." + BoardInfo.DriverVersionSubMinor.ToString());
                            }
                            ThorDAQCommandProvider.ThorDAQAdapters[i] = new TDcontroller(i, "ThorDAQ-2586");

                            commandProvider.GetBoardConfiguration(i);
                        }
                        else
                        {
                            //throw new NullReferenceException();
                            UpdateConsoleStatus("Failed DsConnectToBoard(), status: " + status + " BoardIndex" + i);

                        }
                    }

                    // if a SCRIPT file exists, execute it
                    string DefScriptFileSpec = @"\temp\TDscript.txt";
                    if (File.Exists(DefScriptFileSpec))
                    {
                        UpdateConsoleStatus("!!! Found SCRIPT file " + DefScriptFileSpec + ", EXECUTING SCRIPT....");
                        LoadScript(DefScriptFileSpec);
                    }

                }
                else
                {
                    UpdateConsoleStatus("ThorDAQ PCIe adapter not found");
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ThorDAQ PCIe adapter not found");  // report error to log file
                }

            }
            catch (Exception error)
            {
                MessageBox.Show(error.Message);
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

            }
            catch (Exception error)
            {
                MessageBox.Show(error.Message);

            }
        }

        #region Load scripts
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

        // syntax:  1st arg MSdelay, rest of line = command line
        private void LoadScript(string fileName)
        {
            System.IO.StreamReader file = new System.IO.StreamReader(fileName);
            while (!file.EndOfStream)
            {
                string line = file.ReadLine();
                if (line == "") break; // skip null lines

                if (line.StartsWith("#")) break;

                // get first arg which is INT millisecs delay
                string[] args = line.Split(' ');
                int msDelay = Int32.Parse(args[0]);

                line = line.Substring(line.IndexOf(" ") + 1); // remainder of line is the command

                for (int i = 0; i < CommonDefinition.THORDAQ_COMMANDLINE_COMMAND.Length; i++)
                {
                    if (line.ToLower().Contains(CommonDefinition.THORDAQ_COMMANDLINE_COMMAND[i].ToLower()))
                    {
                        string input = line.Trim(); // remove all white space start or end in the input string,

                        Task runTask = AutomatedCmdsWithDelays(msDelay, input);

                        break;
                    }
                }
            }
            file.Close();
        }
        #endregion

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

    }
}
