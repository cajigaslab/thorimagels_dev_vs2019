namespace CSN210_Control
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Globalization;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Timers;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Windows.Threading;
    using System.Xml;

    using Microsoft.Win32;

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        #region Fields

        private const int FALSE = 0;
        private const string THORLABS_BOOTLOADER_PID = "2F02";
        private const string THORLABS_VID = "1313";
        private const int TRUE = 1;

        private bool _allowRequest = true;
        private bool _btnFileEnabled = true;
        private bool _btnHomeEnabled = true;
        private bool _btnPosition1Enabled = true;
        private bool _btnPosition2Enabled = true;
        private bool _btnStopEnabled = true;
        private Visibility _button1Visibility = Visibility.Visible;
        private Visibility _button2Visibility = Visibility.Visible;
        private string _buttonPos1Name = "Position 1";
        private string _buttonPos2Name = "Position 2";
        private Visibility _controlsVisibility = Visibility.Visible;
        string _firmwareVersion = string.Empty;
        Timer _homeQuery = new Timer();
        Timer _movePos1Query = new Timer();
        Timer _movePos2Query = new Timer();
        private string _objChangerPos;
        private string _objStatus;
        Timer _posQuery = new Timer();
        private string _serialNum;
        private int _statusColor = 0;
        private Visibility _txtbox1Visibility = Visibility.Collapsed;
        private Visibility _txtbox2Visibility = Visibility.Collapsed;

        #endregion Fields

        #region Constructors

        public MainWindow()
        {
            InitializeComponent();
            _homeQuery.Interval = 9000;
            _movePos1Query.Interval = _movePos2Query.Interval = 4500;
            // After 9 seconds of doing the home rutine change the GUI to match the new position.
            _homeQuery.Elapsed += new ElapsedEventHandler((eventSender, eventArgs) =>
            {
                _homeQuery.Stop();
                StatusColor = 0;
                Status = "At Home";
                StopButtonEnabled = true;
                EnableAllButtons();
            });
            // After 4.5 seconds of moving to Position 1 change the GUI to match the new position.
            _movePos1Query.Elapsed += new ElapsedEventHandler((eventSender, eventArgs) =>
            {
                _movePos1Query.Stop();
                _allowRequest = false;
                StatusColor = 0;
                double val = -1;
                CSN210Functions.GetParam((int)DeviceParams.PARAM_TURRET_POS_CURRENT, ref val);
                int position = Convert.ToInt32(val);
                if ((int)ChangerPositions.POS1 == position)
                {
                    Status = "At " + Button1PosName;
                }
                else
                {
                    Status = "Not at " + Button1PosName;
                }
                EnableAllButtons();
                _allowRequest = true;
            });
            // After 4.5 seconds of moving to Position 2 change the GUI to match the new position.
            _movePos2Query.Elapsed += new ElapsedEventHandler((eventSender, eventArgs) =>
            {
                _movePos2Query.Stop();
                _allowRequest = false;
                StatusColor = 0;
                double val = -1;
                CSN210Functions.GetParam((int)DeviceParams.PARAM_TURRET_POS_CURRENT, ref val);
                int position = Convert.ToInt32(val);
                if ((int)ChangerPositions.POS2 == position)
                {
                    Status = "At " + Button2PosName;
                }
                else
                {
                    Status = "Not at " + Button2PosName;
                }
                EnableAllButtons();
                _allowRequest = true;
            });
            this.Closing += new System.ComponentModel.CancelEventHandler(Window1_Closing);
            Application.Current.Exit += new ExitEventHandler(Current_Exit);
            this.Loaded += MainWindow_Loaded;

            _posQuery.Interval = 1000;
            _posQuery.Elapsed += new ElapsedEventHandler((sender, eventArgs) =>
            {
                double val = -1, col = -1;
                if (_allowRequest)
                {
                    CSN210Functions.GetParam((int)DeviceParams.PARAM_TURRET_COLLISION, ref col);
                    CSN210Functions.GetParam((int)DeviceParams.PARAM_TURRET_POS_CURRENT, ref val);
                    int position = Convert.ToInt32(val);

                    //NOTE: Collision detection doesn't work when homing, this is by design
                    // objective changer waits for a collision to know it has reached the home position
                    // in this case it will think the place of collision is home. We move to the second
                    // position in the lower level as a homing routine to detect any collition
                    // that might have happened

                    // Poll for collision, do not mark the collision of the device is doing a home
                    //routine it might need take a second to mark for collision
                    if (1 == col && (int)ChangerPositions.MOVING_POS1 != position)
                    {
                        _movePos1Query.Stop();
                        _movePos2Query.Stop();
                        _homeQuery.Stop();
                        Status = "Collision Detected";
                        StatusColor = 3;
                        HomeButtonEnabled = true;
                        FileButtonEnabled = true;
                        Position1ButtonEnabled = false;
                        Position2ButtonEnabled = false;
                        StopButtonEnabled = false;
                    }
                    if ((int)ChangerPositions.DISCONNECTED == position)
                    {
                        _posQuery.Stop();
                        _movePos1Query.Stop();
                        _movePos2Query.Stop();
                        _homeQuery.Stop();
                        ControlsVisibility = Visibility.Collapsed;
                        MessageBox.Show("Device appears to be disconnected, check the USB connection. Make sure the device is connected and visible in Device Manager, then select \"Refresh Connection\".");
                        EnableAllButtons();
                        StopButtonEnabled = true;
                    }
                }
            });

            this.DataContext = this;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public string Button1PosName
        {
            get
            {
                return _buttonPos1Name;
            }
            set
            {
                _buttonPos1Name = value;
                OnPropertyChanged("Button1PosName");
            }
        }

        public Visibility Button1Visibility
        {
            get
            {
                return _button1Visibility;
            }
            set
            {
                _button1Visibility = value;
                OnPropertyChanged("Button1Visibility");
            }
        }

        public string Button2PosName
        {
            get
            {
                return _buttonPos2Name;
            }
            set
            {
                _buttonPos2Name = value;
                OnPropertyChanged("Button2PosName");
            }
        }

        public Visibility Button2Visibility
        {
            get
            {
                return _button2Visibility;
            }
            set
            {
                _button2Visibility = value;
                OnPropertyChanged("Button2Visibility");
            }
        }

        public Visibility ControlsVisibility
        {
            get
            {
                return _controlsVisibility;
            }
            set
            {
                _controlsVisibility = value;
                OnPropertyChanged("ControlsVisibility");
            }
        }

        public bool FileButtonEnabled
        {
            get
            {
                return _btnFileEnabled;
            }
            set
            {
                _btnFileEnabled = value;
                OnPropertyChanged("FileButtonEnabled");
            }
        }

        public string FirmwareVersion
        {
            get
            {
                return _firmwareVersion;
            }
            set
            {
                _firmwareVersion = value;
                OnPropertyChanged("FirmwareVersion");
            }
        }

        public bool HomeButtonEnabled
        {
            get
            {
                return _btnHomeEnabled;
            }
            set
            {
                _btnHomeEnabled = value;
                OnPropertyChanged("HomeButtonEnabled");
            }
        }

        public string ObjectiveChangerPosText
        {
            get
            {
                return _objChangerPos;
            }
            set
            {
                _objChangerPos = value;
                OnPropertyChanged("ObjectiveChangerPosText");
            }
        }

        public bool Position1ButtonEnabled
        {
            get
            {
                return _btnPosition1Enabled;
            }
            set
            {
                _btnPosition1Enabled = value;
                OnPropertyChanged("Position1ButtonEnabled");
            }
        }

        public bool Position2ButtonEnabled
        {
            get
            {
                return _btnPosition2Enabled;
            }
            set
            {
                _btnPosition2Enabled = value;
                OnPropertyChanged("Position2ButtonEnabled");
            }
        }

        public Visibility RenameButton1Visibility
        {
            get
            {
                return _txtbox1Visibility;
            }
            set
            {
                _txtbox1Visibility = value;
                OnPropertyChanged("RenameButton1Visibility");
            }
        }

        public Visibility RenameButton2Visibility
        {
            get
            {
                return _txtbox2Visibility;
            }
            set
            {
                _txtbox2Visibility = value;
                OnPropertyChanged("RenameButton2Visibility");
            }
        }

        public string SerialNum
        {
            get
            {
                return _serialNum;
            }
            set
            {
                _serialNum = value;
                OnPropertyChanged("SerialNum");
            }
        }

        public string Status
        {
            get
            {
                return _objStatus;
            }
            set
            {
                _objStatus = value;
                OnPropertyChanged("Status");
            }
        }

        // 1: Green, 2: Yellow, 3: Red
        public int StatusColor
        {
            get
            {
                return _statusColor;
            }
            set
            {
                _statusColor = value;
                OnPropertyChanged("StatusColor");
            }
        }

        public bool StopButtonEnabled
        {
            get
            {
                return _btnStopEnabled;
            }
            set
            {
                _btnStopEnabled = value;
                OnPropertyChanged("StopButtonEnabled");
            }
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Get one xml attribute value
        /// </summary>
        /// <param name="node"></param>
        /// <param name="doc"></param>
        /// <param name="attrName"></param>
        /// <param name="attrValue"></param>
        /// <returns></returns>
        public static bool GetAttribute(XmlNode node, XmlDocument doc, string attrName, ref string attrValue)
        {
            bool ret;
            if ((null == node) || (null == doc))
                return false;

            if (null == node.Attributes.GetNamedItem(attrName))
            {
                ret = false;
            }
            else
            {
                attrValue = node.Attributes[attrName].Value;
                ret = true;
            }

            return ret;
        }

        /// <summary>
        /// Set one xml attribute value
        /// </summary>
        /// <param name="node">The node.</param>
        /// <param name="doc">The document.</param>
        /// <param name="attrName">Name of the attribute.</param>
        /// <param name="attValue">The att value.</param>
        public static void SetAttribute(XmlNode node, XmlDocument doc, string attrName, string attValue)
        {
            if ((null == node) || (null == doc))
                return;

            XmlNode tempNode = node.Attributes[attrName];

            if (null == tempNode)
            {
                XmlAttribute attr = doc.CreateAttribute(attrName);

                attr.Value = attValue;

                node.Attributes.Append(attr);
            }
            else
            {
                node.Attributes[attrName].Value = attValue;
            }
        }

        /// <summary>
        /// Handles the Click event of the btnExit control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnExit_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        /// <summary>
        /// Handles the Click event of the btnHome control.
        /// Move to Home event.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnHome_Click(object sender, RoutedEventArgs e)
        {
            _allowRequest = false;
            DissableAllButtons();
            BackgroundWorker worker = new BackgroundWorker();
            worker.DoWork += (obj, evetnArg) =>
                {
                    if (1 != SetParam((int)DeviceParams.PARAM_TURRET_POS, (double)ChangerPositions.HOME))
                    {
                        MessageBox.Show("Moving to Home failed");
                    }
                };
            worker.RunWorkerAsync();
            StopButtonEnabled = false;
            Status = "Homing";
            StatusColor = 1;
            _homeQuery.Start();
            _allowRequest = true;
        }

        /// <summary>
        /// Handles the Click event of the btnPos1 control.
        /// Move to Position 1 event.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnPos1_Click(object sender, RoutedEventArgs e)
        {
            DissableAllButtons();
            BackgroundWorker worker = new BackgroundWorker();
            worker.DoWork += (obj, evetnArg) =>
                {
                    if (1 != SetParam((int)DeviceParams.PARAM_TURRET_POS, (double)ChangerPositions.POS1))
                    {
                        MessageBox.Show("Moving to " + Button1PosName + " failed");
                    }
                };
            worker.RunWorkerAsync();
            Status = "Moving to " + Button1PosName;
            StatusColor = 1;
            _movePos1Query.Start();
        }

        /// <summary>
        /// Handles the Click event of the btnPos2 control.
        /// Move to Position 2 event.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnPos2_Click(object sender, RoutedEventArgs e)
        {
            DissableAllButtons();
            BackgroundWorker worker = new BackgroundWorker();
            worker.DoWork += (obj, evetnArg) =>
            {
                if (1 != SetParam((int)DeviceParams.PARAM_TURRET_POS, (double)ChangerPositions.POS2))
                {
                    MessageBox.Show("Moving to " + Button2PosName + " failed");
                }
            };
            worker.RunWorkerAsync();
            Status = "Moving to " + Button2PosName;
            StatusColor = 1;
            _movePos2Query.Start();
        }

        /// <summary>
        /// Handles the Click event of the btnStop control.
        /// Move to Home event.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnStop_Click(object sender, RoutedEventArgs e)
        {
            _movePos1Query.Stop();
            _movePos2Query.Stop();
            _homeQuery.Stop();
            Status = "Stopped";
            if (1 != SetParam((int)DeviceParams.PARAM_TURRET_STOP, 0))
            {
                MessageBox.Show("Failed to stop");
            }
            Status = "Stopped";
            StatusColor = 3;
            EnableAllButtons();
        }

        /// <summary>
        /// Handles the Click event of the btnUpdateFirmware control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnUpdateFirmware_Click(object sender, RoutedEventArgs e)
        {
            int sleepTime = 3500;
            string comPort = string.Empty;
            string fileName = string.Empty;
            string firmwareFile = string.Empty;
            Microsoft.Win32.OpenFileDialog FWDialog = new Microsoft.Win32.OpenFileDialog();
            FWDialog.Title = "Select .hex file using this Dialog Box";
            FWDialog.DefaultExt = ".hex";
            FWDialog.Filter = "HEX Files|*.hex";
            FWDialog.AddExtension = true;
            FWDialog.CheckFileExists = true;
            FWDialog.CheckPathExists = true;
            if (true == FWDialog.ShowDialog())
            {
                firmwareFile = FWDialog.FileName.ToString();
                fileName = FWDialog.SafeFileName.ToString();
                MessageBoxResult messageResult = MessageBox.Show("Are you sure " + fileName + " is the correct Firmware file for this device?", "Confirmation", MessageBoxButton.YesNo);
                Mouse.OverrideCursor = Cursors.Wait;
                if (MessageBoxResult.No == messageResult || MessageBoxResult.Cancel == messageResult || MessageBoxResult.None == messageResult)
                {
                    Mouse.OverrideCursor = null;
                    return;
                }
                //Check if any device was found and selected
                if (Visibility.Visible == ControlsVisibility)
                {
                    _posQuery.Stop(); //Stop the polling for collision status
                    ControlsVisibility = Visibility.Collapsed;
                    SetParam((int)DeviceParams.PARAM_TURRET_FIRMWAREUPDATE, 1);
                    sleepTime = 10000; // In case Windows has to match the bootloader to its driver
                }
                System.Threading.Thread.Sleep(sleepTime);
                Mouse.OverrideCursor = null;

                comPort = FindBootloaderPortNames();
                if (0 != comPort.CompareTo(string.Empty))
                {
                    UpdateFirmwareWindow.UpdateFirmware updateFirmwareWindow = new UpdateFirmwareWindow.UpdateFirmware(comPort, firmwareFile, fileName);

                    updateFirmwareWindow.ShowDialog();
                    System.Threading.Thread.Sleep(3500);
                    //Need to think about this if the user closes the window
                    Window1_Closing(null, null);

                }
                else
                {
                }
            }
        }

        /// <summary>
        /// Handles the Exit event of the Current control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="ExitEventArgs"/> instance containing the event data.</param>
        void Current_Exit(object sender, ExitEventArgs e)
        {
            try
            {
                XmlDocument settingsFile = new XmlDocument();
                settingsFile.Load("./ThorObjectiveChangerSettings.xml");
                XmlNodeList ndList = settingsFile.SelectNodes("ThorObjectiveChangerSettings/PositionNames");
                if (0 < ndList.Count)
                {
                    SetAttribute(ndList[0], settingsFile, "position1", Button1PosName);
                    SetAttribute(ndList[0], settingsFile, "position2", Button2PosName);
                    settingsFile.Save("./ThorObjectiveChangerSettings.xml");
                }
                else
                {
                    XmlNode node = settingsFile.CreateNode(XmlNodeType.Element, "PositionNames", null);
                    XmlNode parent = settingsFile.SelectSingleNode("ThorObjectiveChangerSettings");
                    parent.AppendChild(node);
                    XmlNodeList nodes = settingsFile.SelectNodes("ThorObjectiveChangerSettings/PositionNames");
                    SetAttribute(nodes[0], settingsFile, "position1", Button1PosName);
                    SetAttribute(nodes[0], settingsFile, "position2", Button2PosName);
                    settingsFile.Save("./ThorObjectiveChangerSettings.xml");
                }
            }
            catch (Exception exc)
            {
                MessageBox.Show("Exception thrown while trying to save ThorObjectiveChangerSettings.xml\n" + exc.ToString());
            }

            TearDown();
            this.Close();
        }

        private void DissableAllButtons()
        {
            Position1ButtonEnabled = false;
            Position2ButtonEnabled = false;
            HomeButtonEnabled = false;
            FileButtonEnabled = false;
        }

        private void EnableAllButtons()
        {
            Position1ButtonEnabled = true;
            Position2ButtonEnabled = true;
            HomeButtonEnabled = true;
            FileButtonEnabled = true;
        }

        /// <summary>
        /// Finds the COM port that matches the bootloader's PID and VID 2F02 & 1313.
        /// </summary>
        /// <param returned> The number of a COM port if only one bootloader type of device is connected. Format e.g. COM27</param>
        private string FindBootloaderPortNames()
        {
            int index = 0, deviceCount = 0;
            string nameData = string.Empty;
            string portName = string.Empty;
            string hashName = string.Empty;
            string[] portNames;
            String pattern = String.Format("VID_{0}.*&.*PID_{1}", THORLABS_VID, THORLABS_BOOTLOADER_PID);
            Regex _rx = new Regex(pattern, RegexOptions.IgnoreCase);
            RegistryKey rk1 = Registry.LocalMachine;
            RegistryKey usbserEnum;
            try
            {
                usbserEnum = rk1.OpenSubKey("SYSTEM\\CurrentControlSet\\services\\usbser\\Enum");
            }
            catch (Exception ex)
            {
                MessageBox.Show("Bootloader device is not connected. Please contact support.\n" + "Exception thrown: " + ex.Message, "Error");
                return portName;
            }
            // Find how many devices in bootloader mode are connected and save the HID hashvalue for each one
            while (null != usbserEnum.GetValue((string)index.ToString(), null))
            {
                nameData = (string)usbserEnum.GetValue((string)index.ToString());
                if (_rx.Match(nameData).Success)
                {
                    deviceCount++;
                    portNames = _rx.Split(nameData);
                    // Save the string after PID_###&VID###, this is the hashValue of the bootloader device, use substring to ignore the '\'
                    if (1 < portNames.Length)
                    {
                        hashName = portNames[1].Substring(1);
                    }
                }
                index++;
            }
            if (1 < deviceCount)
            {
                MessageBox.Show("Too many devices in bootloader mode are connected, make sure only the device you want to update is connected. All other devices in bootloader mode should be disconnected.");
                return portName;
            }

            if (1 > deviceCount)
            {
                MessageBox.Show("Could not find any connected device in Bootloader mode", "Error");
                return portName;
            }

            //Grab the HID generated by Windows as a hashvalue and use it to find the correct COM port
            RegistryKey rk2 = rk1.OpenSubKey("SYSTEM\\CurrentControlSet\\Enum");
            foreach (String s3 in rk2.GetSubKeyNames())
            {
                RegistryKey rk3 = rk2.OpenSubKey(s3);
                foreach (String s in rk3.GetSubKeyNames())
                {
                    if (_rx.Match(s).Success)
                    {
                        RegistryKey rk4 = rk3.OpenSubKey(s);
                        foreach (String s2 in rk4.GetSubKeyNames())
                        {
                            if (0 == s2.CompareTo(hashName))
                            {
                                RegistryKey rk5 = rk4.OpenSubKey(s2);
                                RegistryKey rk6 = rk5.OpenSubKey("Device Parameters");
                                portName = (string)rk6.GetValue("PortName");
                            }
                        }
                    }
                }
            }
            if (0 == portName.CompareTo(string.Empty))
            {
                MessageBox.Show("Cannot retrieve COM port Number from device HID: " + hashName, "Error");
            }
            return portName;
        }

        /// <summary>
        /// Gets the desired parameter from the device dll.
        /// </summary>
        /// <param name="paramId">The parameter identifier.</param>
        /// <param name="paramString">The parameter string.</param>
        /// <returns>System.Int32.</returns>
        int GetParameterString(int paramId, ref string paramString)
        {
            const int LENGTH = 255;
            StringBuilder paramSB = new StringBuilder(LENGTH);
            if (TRUE == CSN210Functions.GetParamString(paramId, paramSB, LENGTH))
            {
                paramString = paramSB.ToString();
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }

        /// <summary>
        /// Gets the desired parameter from the device dll.
        /// </summary>
        /// <param name="paramId">The parameter identifier.</param>
        /// <param name="paramString">The parameter string.</param>
        /// <returns>System.Int32.</returns>
        int GetSerialNumber(int paramId, ref string paramString)
        {
            const int LENGTH = 255;
            StringBuilder paramSB = new StringBuilder(LENGTH);
            if (TRUE == CSN210Functions.GetSerialNumberString(paramId, paramSB, LENGTH))
            {
                paramString = paramSB.ToString();
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }

        // <summary>
        /// Initializes the connections and views.
        /// </summary>
        void Initialize()
        {
            int devices = 0;
            int deviceSelected = 0;
            string temp = System.IO.Directory.GetCurrentDirectory();
            if (FALSE == CSN210Functions.FindDevices(ref devices))
            {
                ControlsVisibility = Visibility.Collapsed;
                return;
            }
            // if more than one device with the same VID&PID was found create a selection window
            if (1 < devices)
            {
                DeviceSelectionWindow devSelect = new DeviceSelectionWindow();
                string serialNum = string.Empty;
                for (int i = 0; i < devices; i++)
                {
                    if (TRUE == GetSerialNumber(i, ref serialNum))
                    {
                        devSelect.SerialListBox.Items.Add(serialNum);
                    }
                    else
                    {
                        MessageBox.Show("Could not retrieve serial number from dll, make sure ThorObjectiveChanger.dll is in the same folder, and Visual Studio Runtime 2012 + .NET 4.5.1 are installed");
                    }
                    serialNum = string.Empty;
                }
                devSelect.ShowDialog();
                deviceSelected = devSelect.SerialListBox.SelectedIndex;
                if (0 > deviceSelected) deviceSelected = 0;
            }
            //Select the devices
            if (FALSE == CSN210Functions.SelectDevice(deviceSelected))
            {
                ControlsVisibility = Visibility.Collapsed;
                return;
            }

            try
            {
                XmlDocument settingsFile = new XmlDocument();
                settingsFile.Load("./ThorObjectiveChangerSettings.xml");
                XmlNodeList ndList = settingsFile.SelectNodes("ThorObjectiveChangerSettings/PositionNames");
                if (0 < ndList.Count)
                {
                    string str = string.Empty;
                    if (GetAttribute(ndList[0], settingsFile, "position1", ref str))
                    {
                        Button1PosName = str;
                    }
                    if (GetAttribute(ndList[0], settingsFile, "position2", ref str))
                    {
                        Button2PosName = str;
                    }
                }
            }
            catch (Exception exc)
            {
                MessageBox.Show("Exception thrown while trying to open ThorObjectiveChangerSettings.xml\n" + exc.ToString());
            }
            string firmwareVersion = string.Empty;

            GetParameterString((int)DeviceParams.PARAM_TURRET_FIRMWAREVERSION, ref firmwareVersion);

            //don't show anything after the second '.'
            int counter = 0, secondDotPos = 0;
            for (int i = 0; i < firmwareVersion.Length; i++)
            {
                if ('.' == firmwareVersion[i])
                {
                    counter++;
                    if (2 == counter)
                    {
                        counter = 0;
                        secondDotPos = i;
                    }
                }
            }

            if (0 == secondDotPos)
            {
                this.FirmwareVersion = firmwareVersion;
            }
            else
            {
                this.FirmwareVersion = firmwareVersion.Substring(0, secondDotPos);
            }

            string serialNumber = string.Empty;

            GetParameterString((int)DeviceParams.PARAM_TURRET_SERIALNUMBER, ref serialNumber);

            this.SerialNum = serialNumber;

            ControlsVisibility = Visibility.Visible;

            StatusColor = 0;
            double val = -1;
            CSN210Functions.GetParam((int)DeviceParams.PARAM_TURRET_HOMED, ref val);
            // If the device is not homed dissable all the position buttons
            if (0 == val)
            {
                Status = "Not Homed";
                HomeButtonEnabled = true;
                FileButtonEnabled = true;
                StopButtonEnabled = false;
                Position1ButtonEnabled = false;
                Position2ButtonEnabled = false;
            }
            else
            {
                val = -1;
                CSN210Functions.GetParam((int)DeviceParams.PARAM_TURRET_POS_CURRENT, ref val);
                int position = Convert.ToInt32(val);
                if ((int)ChangerPositions.POS1 == position)
                {
                    Status = "At " + Button1PosName;
                }
                else if ((int)ChangerPositions.POS2 == position)
                {
                    Status = "At " + Button2PosName;
                }
                else
                {
                    // The device is not at either position and it's homed, so that means it must
                    // have stopped somehwere in the middle
                    Status = "Stopped";
                }
            }

            _allowRequest = true;
            _posQuery.Enabled = true;
        }

        // <summary>
        /// Handles the Loaded event of the Window1 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            Initialize();
        }

        /// <summary>
        /// Called when [property changed].
        /// </summary>
        /// <param name="propertyName">Name of the property.</param>
        private void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        /// <summary>
        /// Called when the temporarily visible textbox loses focus.
        /// </summary>
        /// <param name="propertyName">Name of the property.</param>
        private void pos1Rename_LostFocus(object sender, RoutedEventArgs e)
        {
            Button1PosName = pos1Rename.Text;
            Button1Visibility = Visibility.Visible;
            RenameButton1Visibility = Visibility.Collapsed;
        }

        /// <summary>
        /// Called when the temporarily visible textbox loses focus.
        /// </summary>
        /// <param name="propertyName">Name of the property.</param>
        private void pos2Rename_LostFocus(object sender, RoutedEventArgs e)
        {
            Button2PosName = pos2Rename.Text;
            Button2Visibility = Visibility.Visible;
            RenameButton2Visibility = Visibility.Collapsed;
        }

        /// <summary>
        /// Handles the Click event of the refresh control.
        /// Here the connections and views get refreshed
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void refresh_Click(object sender, RoutedEventArgs e)
        {
            TearDown();
            System.Threading.Thread.Sleep(100);
            Initialize();
        }

        /// <summary>
        /// Handles the Click event of the renameButton1 control.
        /// Hides the position button and makes the textbox for renaming visible. Then sets it in focus.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void renameButton1_Click(object sender, RoutedEventArgs e)
        {
            Button1Visibility = Visibility.Collapsed;
            RenameButton1Visibility = Visibility.Visible;
            pos1Rename.Focus();
        }

        /// <summary>
        /// Handles the Click event of the renameButton2 control.
        /// Hides the position button and makes the textbox for renaming visible. Then sets it in focus.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void renameButton2_Click(object sender, RoutedEventArgs e)
        {
            Button2Visibility = Visibility.Collapsed;
            RenameButton2Visibility = Visibility.Visible;
            pos2Rename.Focus();
        }

        /// <summary>
        /// Sets and executes a parameter.
        /// </summary>
        /// <param name="paramID">The parameter ID.</param>
        /// <param name="alignment"> The parameter.</param>
        private int SetParam(int paramID, double param)
        {
            int ret = FALSE, status = 0;
            ret = CSN210Functions.SetParam(paramID, param);
            CSN210Functions.PreflightPosition();
            CSN210Functions.SetupPosition();
            do
            {
                CSN210Functions.StatusPosition(ref status);
            }
            while (0 == status);
            CSN210Functions.StartPosition();
            CSN210Functions.PostflightPosition();
            return ret;
        }

        /// <summary>
        /// Desables Scanner and tears down the devices
        /// </summary>
        private void TearDown()
        {
            _posQuery.Stop();
            _movePos1Query.Stop();
            _movePos2Query.Stop();
            _homeQuery.Stop();
            ControlsVisibility = Visibility.Collapsed;
            EnableAllButtons();
            StopButtonEnabled = true;
            CSN210Functions.TeardownDevice();
        }

        /// <summary>
        /// Handles the Closing event of the Window1 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="CancelEventArgs"/> instance containing the event data.</param>
        void Window1_Closing(object sender, CancelEventArgs e)
        {
            TearDown();
            Application.Current.Shutdown();
        }

        #endregion Methods
    }
}