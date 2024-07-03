namespace HardwareSetupUserControl
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
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

    using HardwareSetupDll;
    using HardwareSetupDll.View;

    using SpinnerProgress;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class UserControl1 : UserControl, INotifyPropertyChanged
    {
        #region Fields

        private const int PATH_LENGTH = 261;

        private bool lbModalityInitialized = false;
        private string _applicationSettingsFile;
        private XmlDocument _appSettingsDoc;
        private string _detectorName;
        private string _hardwareSettingsFile;
        private XmlDocument _hwSettingsDoc;
        private BackgroundWorker _refreshHardwareWorker = null;
        private bool _refreshLock = false;
        private Window _renameDetectorPopup;

        #endregion Fields

        #region Constructors

        public UserControl1()
        {
            InitializeComponent();

            this.Loaded += new RoutedEventHandler(SetupHardware_Loaded);
            this.Unloaded += UserControl1_Unloaded;

            SetDataProvider();
        }

        #endregion Constructors

        #region Enumerations

        public enum CameraType
        {
            CCD = 0,
            LSM = 1
        }

        #endregion Enumerations

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public bool EnableControls
        {
            get
            {
                return !_refreshLock;
            }
        }

        public bool IsXYStageReady
        {
            get;
            set;
        }

        public bool RefreshLock
        {
            get
            {
                return _refreshLock;
            }
            set
            {
                _refreshLock = value;
                OnPropertyChanged("RefreshLock");
                OnPropertyChanged("EnableControls");
            }
        }

        public string SelectedModalityName
        {
            get
            {
                return ((ListBoxItem)lbModalities.SelectedItem).Content.ToString();
            }
            set
            {
                lbModalities.SelectedItem = value;
            }
        }

        #endregion Properties

        #region Methods

        public void loadDevs()
        {
            RefreshLock = true;

            //In the event that a live capture is still active stop it immediately
            StopLiveCapture();

            LoadModalities();

            _hwSettingsDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
            _appSettingsDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            settingEditor.maxFileSize = 10 * 1024;
            XmlNodeList ndList = _appSettingsDoc.SelectNodes("/ApplicationSettings/SettingEditorXmlFiles");
            if (ndList.Count > 0)
            {
                settingEditor.maxFileSize = Convert.ToInt32(ndList[0].Attributes["maxSize"].Value);
            }

            settingEditor.Path = System.IO.Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);

            RefreshLock = false;

            VerifyModalitiesSelectedDevices();

            UpdateXMLBindings();

            LoadHardwareSelection();

            SetHardwareIcons();

            this.CanvasSpinProgress.Visibility = Visibility.Hidden;
        }

        protected virtual void OnPropertyChanged(String propertyName)
        {
            if (System.String.IsNullOrEmpty(propertyName))
            {
                return;
            }
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "GetBleachID")]
        private static extern int GetBleachID();

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "GetCameraID")]
        private static extern int GetCameraID();

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "GetCameraParameter")]
        private static extern int GetCameraParameter(int camID, int paramID, ref double param);

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "GetCameraType")]
        private static extern int GetCameraType(int camID, ref int cameraType);

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "GetDeviceID")]
        private static extern int GetDeviceID(int deviceType);

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "GetDeviceParamInfo")]
        private static extern int GetDeviceParamInfo(int deviceID, int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamLong")]
        private static extern int GetDeviceParamInt(int cameraSelection, int param, ref int value);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetupCommand")]
        private static extern int LISetupCommand();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "TeardownCommand")]
        private static extern int LITeardownCommand();

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "LoadCamera")]
        private static extern int LoadCamera(int cameraIndex);

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "LoadDevice")]
        private static extern int LoadDevice(uint deviceType, int deviceIndex);

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "SetActiveBleachingScanner")]
        private static extern int SetActiveBleachingScanner(int scannerIndex);

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "SetDetectorName", CharSet = CharSet.Unicode)]
        private static extern int SetDetectorName(int detectorID, StringBuilder detectorName);

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "SetupCommand")]
        private static extern int SetupCommand();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "StopLiveCapture")]
        private static extern int StopLiveCapture();

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "TeardownCommand")]
        private static extern int TeardownCommand();

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "UpdateAndPersistCurrentDevices")]
        private static extern int UpdateAndPersistCurrentDevices();

        private void btnAddModality_Click(object sender, RoutedEventArgs e)
        {
            AddModality dlg = new AddModality();

            if (true == dlg.ShowDialog())
            {
                LoadModalities();
                dlg.WindowState = WindowState.Normal;
            }
        }

        private void btnAppSettings_Click(object sender, RoutedEventArgs e)
        {
            ApplicationSettings dlg = new ApplicationSettings();

            MVMManager.Instance.LoadSettings(SettingsFileType.APPLICATION_SETTINGS);
            _appSettingsDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            if (true == dlg.ShowDialog())
            {
                UpdateXMLBindings();
            }
        }

        void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            _renameDetectorPopup.DialogResult = false;
        }

        private void btnDelModality_Click(object sender, RoutedEventArgs e)
        {
            if (lbModalities.Items.Count == 1)
            {
                MessageBox.Show("This is the last modality, cannot be deleted.");
                return;
            }

            if (MessageBoxResult.Yes == MessageBox.Show("Are you sure you want to delete the selected modality?", "Delete Modality", MessageBoxButton.YesNo))
            {
                string SelectedFolder = ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "\\Modalities\\" + ((ListBoxItem)lbModalities.SelectedItem).Content.ToString();
                Directory.Delete(SelectedFolder, true);
                lbModalities.Items.Remove(lbModalities.SelectedItem);
                lbModalities.SelectedIndex = 0;
            }
        }

        private void btnDisplay_Click(object sender, RoutedEventArgs e)
        {
            RefreshLock = true;
            //load the settings for the selected modality
            SetDataProvider();

            RefreshLock = false;

            LoadHardwareSelection();

            SetHardwareIcons();

            DisplayedDevices dlg = new DisplayedDevices();

            MVMManager.Instance.LoadSettings(SettingsFileType.APPLICATION_SETTINGS);
            _appSettingsDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNode node = _appSettingsDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/HardwareSetup");
            string str = string.Empty;

            if (null != node)
            {
                XmlManager.GetAttribute(node, _appSettingsDoc, "ImageDetector", ref str);
                dlg.ImageDetector = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "BleachingScanner", ref str);
                dlg.BleachingScanner = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "ControlUnit", ref str);
                dlg.ControlUnit = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "PMT1", ref str);
                dlg.PMT1 = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "PMT2", ref str);
                dlg.PMT2 = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "PMT3", ref str);
                dlg.PMT3 = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "PMT4", ref str);
                dlg.PMT4 = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "Shutter", ref str);
                dlg.Shutter = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "ZStage", ref str);
                dlg.ZStage = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "SecondaryZStage", ref str);
                dlg.SecondaryZStage = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "PowerRegulator", ref str);
                dlg.PowerRegulator = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "PowerRegulator2", ref str);
                dlg.PowerRegulator2 = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "BeamExpander", ref str);
                dlg.BeamExpander = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "LaserSource", ref str);
                dlg.LaserSource = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "PinholeWheel", ref str);
                dlg.PinholeWheel = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "LightPath", ref str);
                dlg.LightPath = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "XYStage", ref str);
                dlg.XYStage = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "SpectrumFilter", ref str);
                dlg.SpectrumFilter = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "EpiTurret", ref str);
                dlg.EpiTurret = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "ObjectiveTurret", ref str);
                dlg.ObjectiveTurret = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "BleachingSLM", ref str);
                dlg.BleachingSLM = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "BeamStabilizer", ref str);
                dlg.BeamStabilizer = str.Equals("Visible");
                XmlManager.GetAttribute(node, _appSettingsDoc, "LAMP", ref str);
                dlg.Lamp = str.Equals("Visible");
            }

            if (true == dlg.ShowDialog())
            {
                node = _appSettingsDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/HardwareSetup");

                if (node == null)
                {
                    node = _appSettingsDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions");

                    if (null != node)
                    {
                        XmlElement newElement = _appSettingsDoc.CreateElement("HardwareSetup");

                        node.AppendChild(newElement);

                        node = _appSettingsDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/HardwareSetup");
                    }
                }

                if (null != node)
                {
                    XmlManager.SetAttribute(node, _appSettingsDoc, "ImageDetector", (true == dlg.ImageDetector) ? "Visible" : "Collapsed");
                    XmlManager.SetAttribute(node, _appSettingsDoc, "BleachingScanner", (true == dlg.BleachingScanner) ? "Visible" : "Collapsed");
                    XmlManager.SetAttribute(node, _appSettingsDoc, "ControlUnit", (true == dlg.ControlUnit) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/ControlUnit"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "PMT1", (true == dlg.PMT1) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/PMT1"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "PMT2", (true == dlg.PMT2) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/PMT2"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "PMT3", (true == dlg.PMT3) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/PMT3"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "PMT4", (true == dlg.PMT4) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/PMT4"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "Shutter", (true == dlg.Shutter) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/Shutter"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "ZStage", (true == dlg.ZStage) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/ZStage"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "SecondaryZStage", (true == dlg.SecondaryZStage) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/ZStage2"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "PowerRegulator", (true == dlg.PowerRegulator) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/PowerRegulator"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "PowerRegulator2", (true == dlg.PowerRegulator2) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/PowerRegulator2"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "BeamExpander", (true == dlg.BeamExpander) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/BeamExpander"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "LaserSource", (true == dlg.LaserSource) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/MCLS"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "PinholeWheel", (true == dlg.PinholeWheel) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/PinholeWheel"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "LightPath", (true == dlg.LightPath) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/LightPath"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "XYStage", (true == dlg.XYStage) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/XYStage"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "SpectrumFilter", (true == dlg.SpectrumFilter) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/SpectrumFilter"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "EpiTurret", (true == dlg.EpiTurret) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/EpiTurret"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "ObjectiveTurret", (true == dlg.ObjectiveTurret) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/Turret"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "BleachingSLM", (true == dlg.BleachingSLM) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/SLM"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "BeamStabilizer", (true == dlg.BeamStabilizer) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/BeamStabilizer"));
                    XmlManager.SetAttribute(node, _appSettingsDoc, "LAMP", (true == dlg.Lamp) ? "Visible" : SetDeviceCollapsedAndDisconnected("/HardwareSettings/Devices/LAMP"));
                    MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
                    MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);

                    RefreshLock = true;

                    UpdateXMLBindings();

                    RefreshLock = false;

                    LoadHardwareSelection();
                    SetHardwareIcons();
                }
            }
        }

        private void btnHardwareSettings_Click(object sender, RoutedEventArgs e)
        {
            HardwareSettings dlg = new HardwareSettings();

            MVMManager.Instance.LoadSettings(SettingsFileType.HARDWARE_SETTINGS);
            _hwSettingsDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

            if (true == dlg.ShowDialog())
            {
                UpdateXMLBindings();

            }
        }

        private void btnObjectives_Click(object sender, RoutedEventArgs e)
        {
            int deviceID = GetDeviceID((int)ThorSharedTypes.DeviceType.BEAM_EXPANDER);
            SetupObjectives dlg = new SetupObjectives();

            if (0 < deviceID) //deviceID = 0 when there is not device for that category
            {
                int pID = (int)ThorSharedTypes.IDevice.Params.PARAM_EXP_RATIO;
                int pType = 0;
                int pAvailable = 0;
                int pReadOnly = 0;
                double pMin = 0;
                double pMax = 0;
                double pDefault = 0;

                if (1 == GetDeviceParamInfo(deviceID, pID, ref pType, ref pAvailable, ref pReadOnly, ref pMin, ref pMax, ref pDefault))
                {
                    dlg.MinExp = pMin / 100.0; //values stored in the settings file have to be devided by 100
                    dlg.MaxExp = pMax / 100.0; //values stored in the settings file have to be devided by 100
                }
            }

            dlg.HardwareDoc = _hwSettingsDoc;

            if (true == dlg.ShowDialog())
            {
            }
        }

        void btnOK_Click(object sender, RoutedEventArgs e)
        {
            _renameDetectorPopup.DialogResult = true;
        }

        private void Button_RefreshHardware(object sender, RoutedEventArgs e)
        {
            if (false == _refreshLock)
            {
                RefreshHardware();
            }
        }

        private void Button_RenameDetector(object sender, RoutedEventArgs e)
        {
            CreateRenameDetectorWindow();
        }

        private void Button_ShowDevManger(object sender, RoutedEventArgs e)
        {
            System.Diagnostics.Process proc = new System.Diagnostics.Process();
            proc.StartInfo.FileName = "devmgmt.msc";
            proc.StartInfo.UseShellExecute = true;
            proc.Start();
        }

        private void cbBleachingScanner_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (false == _refreshLock)
            {
                _hwSettingsDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
                XmlNodeList ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/ImageDetectors/LSM[not(@cameraName='ResonanceGalvo\') and not(@cameraName='ResonanceGalvoGalvo\') and not(@cameraName='ThorDAQResonantGalvo\') ]");
                int index = (sender as ComboBox).SelectedIndex;

                for (int i = 0; i < ndList.Count; i++)
                {
                    if ((index == i) && ("LSM" == ndList[i].Name))
                    {
                        string str = string.Empty;
                        LITeardownCommand();
                        if (XmlManager.GetAttribute(ndList[i], _hwSettingsDoc, "id", ref str))
                        {
                            int id = Convert.ToInt32(str);
                            SetActiveBleachingScanner(id);
                        }
                        LISetupCommand();
                        XmlManager.SetAttribute(ndList[i], _hwSettingsDoc, "activation", "1");
                    }
                    else
                    {
                        XmlManager.SetAttribute(ndList[i], _hwSettingsDoc, "activation", "0");
                    }
                }

                MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
            }
        }

        private void cbCamera_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (false == _refreshLock)
            {
                _hwSettingsDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
                XmlNodeList ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/ImageDetectors/*");

                int index = (sender as ComboBox).SelectedIndex;

                for (int i = 0; i < ndList.Count; i++)
                {
                    if ((index == i))
                    {
                        // select camera for TSI camera
                        // set activecamera1 in SelectHardware for LSM
                        LITeardownCommand();
                        string id = string.Empty;
                        XmlManager.GetAttribute(ndList[i], _hwSettingsDoc, "id", ref id);
                        LoadCamera(Convert.ToInt32(id) - 1);

                        //setup to use the activeCamera1 specified in SelectHardware
                        LISetupCommand();
                        XmlManager.SetAttribute(ndList[i], _hwSettingsDoc, "active", "1");
                    }
                    else
                    {
                        XmlManager.SetAttribute(ndList[i], _hwSettingsDoc, "active", "0");
                    }
                }

                MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
                SetPockelsIcons();
                ResourceManagerCS.Instance.UpdatePMTSwitchBox();
            }
        }

        private void cbDevice_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (false == _refreshLock)
            {
                uint deviceType = 0;
                ComboBox cbHandler = sender as ComboBox;
                switch (cbHandler.Name)
                {
                    case "cbControlUnit": deviceType = Convert.ToUInt32(DeviceType.CONTROL_UNIT); break;
                    case "cbPMT1": deviceType = Convert.ToUInt32(DeviceType.PMT1); break;
                    case "cbPMT2": deviceType = Convert.ToUInt32(DeviceType.PMT2); break;
                    case "cbPMT3": deviceType = Convert.ToUInt32(DeviceType.PMT3); break;
                    case "cbPMT4": deviceType = Convert.ToUInt32(DeviceType.PMT4); break;
                    case "cbZStage": deviceType = Convert.ToUInt32(DeviceType.STAGE_Z); break;
                    case "cbZStage2": deviceType = Convert.ToUInt32(DeviceType.STAGE_Z2); break;
                    case "cbPowerReg": deviceType = Convert.ToUInt32(DeviceType.POWER_REG); break;
                    case "cbPowerReg2": deviceType = Convert.ToUInt32(DeviceType.POWER_REG2); break;
                    case "cbBeamExpan": deviceType = Convert.ToUInt32(DeviceType.BEAM_EXPANDER); break;
                    case "cbLaserSource": deviceType = Convert.ToUInt32(DeviceType.LASER1); break;
                    case "cbPinhole": deviceType = Convert.ToUInt32(DeviceType.PINHOLE_WHEEL); break;
                    case "cbXYStage": deviceType = Convert.ToUInt32(DeviceType.STAGE_X | DeviceType.STAGE_Y); break;
                    case "cbShutter": deviceType = Convert.ToUInt32(DeviceType.SHUTTER); break;
                    case "cbLightPath": deviceType = Convert.ToUInt32(DeviceType.LIGHT_PATH); break;
                    case "cbSpectrumFilter": deviceType = Convert.ToUInt32(DeviceType.SPECTRUM_FILTER); break;
                    case "cbEpiTurret": deviceType = Convert.ToUInt32(DeviceType.FILTER_WHEEL_DIC); break;
                    case "cbTurret": deviceType = Convert.ToUInt32(DeviceType.TURRET); break;
                    case "cbSLM": deviceType = Convert.ToUInt32(DeviceType.SLM); break;
                    case "cbBeamStabilizer": deviceType = Convert.ToUInt32(DeviceType.BEAM_STABILIZER); break;
                    case "cbLamp": deviceType = Convert.ToUInt32(DeviceType.LAMP); break;

                }

                LoadDevice(deviceType, cbHandler.SelectedIndex);
                SetHardwareIcon(cbHandler.Name);
            }
        }

        private void ChangeDetectorName(string name)
        {
            RefreshLock = true;
            _hwSettingsDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
            XmlNodeList ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/ImageDetectors");

            //Change detector names for all detectors
            if (ndList.Count > 0)
            {
                XmlNode detectors = ndList[0];
                foreach (XmlNode node in detectors.ChildNodes)
                {
                    string strId = string.Empty;
                    string strActive = string.Empty;
                    string strName = string.Empty;
                    if (XmlManager.GetAttribute(node, _hwSettingsDoc, "active", ref strActive) && XmlManager.GetAttribute(node, _hwSettingsDoc, "id", ref strId) && XmlManager.GetAttribute(node, _hwSettingsDoc, "cameraName", ref strName))
                    {
                        StringBuilder sbName = new StringBuilder();
                        sbName.Capacity = 64;
                        sbName.Append(name);

                        //change the name if the camera is active and name is edittable
                        if ((Convert.ToInt32(strActive) == 1) && (SetDetectorName(Convert.ToInt32(strId), sbName) > 0))
                        {
                            _detectorName = name;
                            XmlManager.SetAttribute(node, _hwSettingsDoc, "cameraName", _detectorName);
                            MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
                        }
                    }
                }
            }

            UpdateXMLBindings();
            LoadHardwareSelection();
            RefreshLock = false;
        }

        private void CreateRenameDetectorWindow()
        {
            _renameDetectorPopup = new Window();
            _renameDetectorPopup.Title = "Rename Detector";
            _renameDetectorPopup.Background = Brushes.DimGray;
            _renameDetectorPopup.Foreground = Brushes.White;
            _renameDetectorPopup.ResizeMode = ResizeMode.NoResize;
            _renameDetectorPopup.Width = 230;
            _renameDetectorPopup.Height = 120;
            _renameDetectorPopup.Topmost = true;

            Border border = new Border();
            border.BorderThickness = new Thickness(2);
            _renameDetectorPopup.Content = border;
            StackPanel spMain = new StackPanel();
            spMain.Margin = new Thickness(3);
            border.Child = spMain;

            StackPanel spNames = new StackPanel();
            spNames.Orientation = Orientation.Horizontal;
            spNames.HorizontalAlignment = HorizontalAlignment.Stretch;
            spNames.Margin = new Thickness(3);
            Label lblName = new Label();
            lblName.Foreground = Brushes.White;
            lblName.Content = "Detector:";
            TextBox tbDetectorName = new TextBox();
            XmlNode cameraNode = (XmlNode)cbCamera.SelectedItem;
            if (null != cameraNode.Attributes.GetNamedItem("cameraName"))
            {
                _detectorName = cameraNode.Attributes["cameraName"].Value;
                tbDetectorName.Text = _detectorName;
            }
            tbDetectorName.Width = 150;
            spNames.Children.Add(lblName);
            spNames.Children.Add(tbDetectorName);
            spMain.Children.Add(spNames);

            StackPanel spButtons = new StackPanel();
            spButtons.Orientation = Orientation.Horizontal;
            spButtons.HorizontalAlignment = HorizontalAlignment.Right;
            spButtons.Margin = new Thickness(3);
            Button btnOK = new Button();
            btnOK.Content = "OK";
            btnOK.Width = 57;
            btnOK.Height = 30;
            btnOK.Margin = new Thickness(3);
            btnOK.Click += new RoutedEventHandler(btnOK_Click);
            Button btnCancel = new Button();
            btnCancel.Content = "Cancel";
            btnCancel.Width = 57;
            btnCancel.Height = 30;
            btnCancel.Margin = new Thickness(3);
            btnCancel.Click += new RoutedEventHandler(btnCancel_Click);
            spButtons.Children.Add(btnOK);
            spButtons.Children.Add(btnCancel);
            spMain.Children.Add(spButtons);

            //Locate the popup window relative to its parent or host window
            Window hostWindow = Window.GetWindow(this);
            _renameDetectorPopup.Owner = hostWindow;
            _renameDetectorPopup.Left = _renameDetectorPopup.Owner.Left + 80;
            _renameDetectorPopup.Top = _renameDetectorPopup.Owner.Top + 60;

            if (true == _renameDetectorPopup.ShowDialog())
            {
                _detectorName = tbDetectorName.Text;
                ChangeDetectorName(_detectorName);
            }

            _renameDetectorPopup.Close();
        }

        private Brush GetCameraIconColor(int check)
        {
            Brush statBrush = Brushes.Red;

            switch ((ICamera.ConnectionStatusType)check)
            {
                case ICamera.ConnectionStatusType.CONNECTION_READY:
                    {
                        statBrush = Brushes.LimeGreen;
                    }
                    break;
                case ICamera.ConnectionStatusType.CONNECTION_WARMING_UP:
                    {
                        statBrush = Brushes.Yellow;
                    }
                    break;
                case ICamera.ConnectionStatusType.CONNECTION_UNAVAILABLE:
                    {
                        statBrush = Brushes.Red;
                    }
                    break;
                case ICamera.ConnectionStatusType.CONNECTION_ERROR_STATE:
                    {
                        statBrush = Brushes.Orange;
                    }
                    break;
            }

            return statBrush;
        }

        private int GetDeviceConnectionStatus(int selectedDevice)
        {
            int connectStatus = (int)IDevice.ConnectionStatusType.CONNECTION_UNAVAILABLE;

            if (1 == GetDeviceParamInt(selectedDevice, (int)IDevice.Params.PARAM_CONNECTION_STATUS, ref connectStatus) || 1 == connectStatus)
            {

            }
            else
            {
                connectStatus = (int)IDevice.ConnectionStatusType.CONNECTION_UNAVAILABLE;
            }

            return connectStatus;
        }

        private Brush GetDeviceIconColor(int check)
        {
            Brush statBrush = Brushes.Red;

            switch ((IDevice.ConnectionStatusType)check)
            {
                case IDevice.ConnectionStatusType.CONNECTION_READY:
                    {
                        statBrush = Brushes.LimeGreen;
                    }
                    break;
                case IDevice.ConnectionStatusType.CONNECTION_WARMING_UP:
                    {
                        statBrush = Brushes.Yellow;
                    }
                    break;
                case IDevice.ConnectionStatusType.CONNECTION_UNAVAILABLE:
                    {
                        statBrush = Brushes.Red;
                    }
                    break;
                case IDevice.ConnectionStatusType.CONNECTION_ERROR_STATE:
                    {
                        statBrush = Brushes.Orange;
                    }
                    break;
            }

            return statBrush;
        }

        private Brush GetIconColor(int check)
        {
            if (check > 0)
            {
                return Brushes.LimeGreen;
            }
            else
            {
                return Brushes.Red;
            }
        }

        private void lbModalities_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            //only apply the selected logic after the list box has been fully initialized.
            if (false == lbModalityInitialized)
            {
                return;
            }

            if (0 <= lbModalities.SelectedIndex)
            {
                //update the HWSettings.xml file for previous modality with currently connected devices
                UpdateAndPersistCurrentDevices();

                ResourceManagerCS.SetModality(((ListBoxItem)lbModalities.Items[lbModalities.SelectedIndex]).Content.ToString());

                RefreshLock = true;
                //load the settings for the new selected modality
                SetDataProvider();

                RefreshLock = false;

                LoadHardwareSelection();
                SetHardwareIcons();
            }
        }

        private void LoadHardwareSelection()
        {
            try
            {
                MVMManager.Instance.LoadSettings(SettingsFileType.HARDWARE_SETTINGS);
                _hwSettingsDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
                ResourceManagerCS.BackupDirectory(ResourceManagerCS.GetMyDocumentsThorImageFolderString());

            }
            catch
            {
                MessageBox.Show("There is a problem loading " + _hardwareSettingsFile);
                return;
            }

            string s = string.Empty;
            XmlNodeList ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/ImageDetectors/*");
            int i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbCamera.SelectedIndex = i;

                    XmlManager.GetAttribute(node, _hwSettingsDoc, "cameraName", ref _detectorName);
                    break;
                }
                i++;
            }

            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/ImageDetectors/LSM[not(@cameraName='ResonanceGalvo\') and not(@cameraName='ResonanceGalvoGalvo\')  and not(@cameraName='ThorDAQResonantGalvo\')  ]");
            for (i = 0; i < ndList.Count; i++)
            {
                string str = string.Empty;
                if (XmlManager.GetAttribute(ndList[i], _hwSettingsDoc, "activation", ref str))
                {
                    if (Convert.ToInt32(str) == 1)
                    {
                        cbBleachingScanner.SelectedIndex = i;
                        break;
                    }
                }
            }

            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/ControlUnit");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbControlUnit.SelectedIndex = i;
                    break;
                }
                i++;
            }

            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/PMT1");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbPMT1.SelectedIndex = i;
                    break;
                }
                i++;
            }
            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/PMT2");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbPMT2.SelectedIndex = i;
                    break;
                }
                i++;
            }

            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/PMT3");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbPMT3.SelectedIndex = i;
                    break;
                }
                i++;
            }

            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/PMT4");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbPMT4.SelectedIndex = i;
                    break;
                }
                i++;
            }

            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/ZStage");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbZStage.SelectedIndex = i;
                    break;
                }
                i++;
            }

            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/ZStage2");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbZStage2.SelectedIndex = i;
                    break;
                }
                i++;
            }

            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/PowerRegulator");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbPowerReg.SelectedIndex = i;
                    break;
                }
                i++;
            }

            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/PowerRegulator2");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbPowerReg2.SelectedIndex = i;
                    break;
                }
                i++;
            }


            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/BeamExpander");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbBeamExpan.SelectedIndex = i;
                    break;
                }
                i++;
            }

            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/MCLS");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbLaserSource.SelectedIndex = i;
                    break;
                }
                i++;
            }


            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/PinholeWheel");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbPinhole.SelectedIndex = i;
                    break;
                }
                i++;
            }


            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/XYStage");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbXYStage.SelectedIndex = i;
                    break;
                }
                i++;
            }
            //if ((ndList.Count == 1) && (ndList[0].Attributes["dllName"].Value == "SimDeviceXY"))
            //{
            //    _isSimDeviceXYOnly = true;
            //}
            //else
            //{
            //    _isSimDeviceXYOnly = false;
            //}

            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/Shutter");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbShutter.SelectedIndex = i;
                    break;
                }
                i++;
            }

            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/LightPath");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbLightPath.SelectedIndex = i;
                    break;
                }
                i++;
            }

            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/SpectrumFilter");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbSpectrumFilter.SelectedIndex = i;
                    break;
                }
                i++;
            }


            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/EpiTurret");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbEpiTurret.SelectedIndex = i;
                    break;
                }
                i++;
            }


            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/Turret");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbTurret.SelectedIndex = i;
                    break;
                }
                i++;
            }

            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/SLM");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbSLM.SelectedIndex = i;
                    break;
                }
                i++;
            }

            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/BeamStabilizer");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbBeamStabilizer.SelectedIndex = i;
                    break;
                }
                i++;
            }


            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Devices/LAMP");
            i = 0;
            foreach (XmlNode node in ndList)
            {
                if (Convert.ToInt32(node.Attributes["active"].Value) == 1)
                {
                    cbLamp.SelectedIndex = i;
                    break;
                }
                i++;
            }

            ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Objectives/Objective");
            foreach (XmlNode n in ndList)
            {
                if (!XmlManager.GetAttribute(n, _hwSettingsDoc, "fineAfPercentDecrease", ref s))
                {
                    XmlManager.SetAttribute(n, _hwSettingsDoc, "fineAfPercentDecrease", ".15");
                    MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
                }
            }
        }

        private void LoadModalities()
        {
            string modalitiesFolder = ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "\\Modalities";

            lbModalities.Items.Clear();

            if (Directory.Exists(modalitiesFolder))
            {
                string[] mods = Directory.GetDirectories(modalitiesFolder);

                foreach (string mod in mods)
                {
                    ListBoxItem li = new ListBoxItem();
                    li.Width = 120;
                    li.Height = 45;
                    li.HorizontalContentAlignment = HorizontalAlignment.Center;
                    li.VerticalContentAlignment = VerticalAlignment.Center;
                    li.Background = (Brush)Resources["BackgroundBrush"];
                    li.Foreground = (Brush)Resources["TextForegroundBrush"];
                    li.Content = new DirectoryInfo(mod).Name;
                    lbModalities.Items.Add(li);
                }
            }

            ReadModality();
        }

        private void ReadModality()
        {
            string strModality = ResourceManagerCS.Instance.ActiveModality;

            if (lbModalities.Items.Count <= 0)
            {
                lbModalityInitialized = true;
                return;
            }

            if (string.IsNullOrEmpty(strModality))
            {
                lbModalities.SelectedIndex = 0;
                lbModalityInitialized = true;
                return;
            }

            for (int i = 0; i < lbModalities.Items.Count; i++)
            {
                if (((ListBoxItem)lbModalities.Items[i]).Content.ToString().Equals(strModality))
                {
                    lbModalities.SelectedIndex = i;
                    lbModalityInitialized = true;
                    ResourceManagerCS.SetActiveModality = strModality;
                    return;
                }
            }

            lbModalities.SelectedIndex = 0;
            lbModalityInitialized = true;
        }

        private void RefreshHardware()
        {
            if (null != _refreshHardwareWorker)
            {
                if (_refreshHardwareWorker.IsBusy)
                {
                    return;
                }
            }

            this.CanvasSpinProgress.Visibility = Visibility.Visible;

            _refreshHardwareWorker = new BackgroundWorker();

            _refreshHardwareWorker.DoWork += (obj, eventArg) =>
            {
                RefreshLock = true;
                StopLiveCapture();
                //Teardown all before setup for SelectHardware:
                TeardownCommand();
                LITeardownCommand();
                SetupCommand();
                ResourceManagerCS.Instance.ConnectToPMTSwitchBox();
                LISetupCommand();
                Application.Current.Dispatcher.Invoke((Action)(() =>
                        {
                            VerifyModalitiesSelectedDevices();
                            UpdateXMLBindings();
                            LoadHardwareSelection();
                            SetHardwareIcons();
                            System.Threading.Thread.Sleep(30);
                        }));
            };

            _refreshHardwareWorker.RunWorkerCompleted += (obj, eventArg) =>
            {
                RefreshLock = false;
                this.CanvasSpinProgress.Visibility = Visibility.Hidden;
            };

            _refreshHardwareWorker.RunWorkerAsync();
            ResourceManagerCS.BackupDirectory(ResourceManagerCS.GetMyDocumentsThorImageFolderString());
        }

        private void SetDataProvider()
        {
            //retrieve the hardware settings complete path and file name
            _hardwareSettingsFile = ResourceManagerCS.GetModalityHardwareSettingsFileString(ResourceManagerCS.GetModality());
            _applicationSettingsFile = ResourceManagerCS.GetModalityApplicationSettingsFileString(ResourceManagerCS.GetModality());

            var provider = (XmlDataProvider)this.Resources["dataProvider"];
            provider.IsAsynchronous = false; //perform node collection creation in the active context in stead of a worker thread
            provider.Source = new Uri(_hardwareSettingsFile);

            var providerAppSet = (XmlDataProvider)this.Resources["dataProviderAppSettings"];
            providerAppSet.IsAsynchronous = false; //perform node collection creation in the active context in stead of a worker thread
            providerAppSet.Source = new Uri(_applicationSettingsFile);
        }

        private string SetDeviceCollapsedAndDisconnected(string str)
        {
            if (_hwSettingsDoc != null)
            {
                XmlNodeList nodes = _hwSettingsDoc.SelectNodes(str);

                if (nodes.Count > 0)
                {
                    foreach (XmlNode node in nodes)
                    {
                        string strTemp = string.Empty;
                        if (XmlManager.GetAttribute(node, _hwSettingsDoc, "dllName", ref strTemp))
                        {
                            bool IsDisconnected = strTemp.Equals("Disconnected");

                            XmlManager.SetAttribute(node, _hwSettingsDoc, "active", IsDisconnected ? "1" : "0");
                        }
                    }
                }
            }

            return "Collapsed";
        }

        /*
         * @fn	    SetHardwareIcon(string deviceName)
         *
         * @brief	This method is implemented specifically for the function cbDevice_SelectionChanged. There were performance issues when cbDevice_SelectionChanged
         *          would call SetHardwareIcons().cbDevice_SelectionChanged. It was updating every icon for each device, instead of updating
         *          the icon of the device that was recently changed.
         *
         * @param	The name of the parameter which was changed.
         *
         * @return	void.
         */
        private void SetHardwareIcon(string deviceName)
        {
            switch (deviceName)
            {
                case "cbControlUnit": imgControlUnit.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_CONTROLUNIT)); break;
                case "cbPMT1": imgPMT1.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_PMT1)); break;
                case "cbPMT2": imgPMT2.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_PMT2)); break;
                case "cbPMT3": imgPMT3.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_PMT3)); break;
                case "cbPMT4": imgPMT4.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_PMT4)); break;
                case "cbShutter": imgShutter.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_SHUTTER1)); break;
                case "cbZStage": imgZStage.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_ZSTAGE)); break;
                case "cbZStage2": imgZStage2.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_ZSTAGE2)); break;
                case "cbPowerReg": imgPowerReg.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_POWERREGULATOR)); break;
                case "cbPowerReg2": imgPowerReg2.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_POWERREGULATOR2)); break;
                case "cbBeamExpan": imgBeamExpan.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_BEAMEXPANDER)); break;
                case "cbLaserSource": imgLaserSource.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_LASER1)); break;
                case "cbLightPath": imgLightPath.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_LIGHTPATH)); break;
                case "cbPinhole": imgPinhole.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_PINHOLEWHEEL)); break;
                case "cbXYStage":
                    int xyStatus = GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_XYSTAGE);
                    if ((int)IDevice.ConnectionStatusType.CONNECTION_UNAVAILABLE == xyStatus)
                    {
                        imgXYStage.Fill = GetDeviceIconColor(xyStatus);

                        IsXYStageReady = true;

                        //MessageBox.Show("An XY stage is required for ThorImageLS to run. Please either install SimDeviceXY.dll or deploy MLS stage with ThorMLSStage.dll",
                        //    "XY Stage component or dll is missing", MessageBoxButton.OK, MessageBoxImage.Error);
                    }
                    else
                    {
                        imgXYStage.Fill = GetDeviceIconColor(xyStatus);
                        IsXYStageReady = true;
                    }
                    break;
                case "cbSpectrumFilter": imgSpectrumFilter.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_SPECTRUMFILTER)); break;
                case "cbEpiTurret": imgEpiTurret.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_EPITURRET)); break;
                case "cbTurret": imgTurret.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)(int)SelectedHardware.SELECTED_TURRET)); break;
                case "cbSLM": imgSLM.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_SLM)); break;
                case "cbBeamStabilizer": imgBeamStabilizer.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_BEAMSTABILIZER)); break;
                case "cbLamp": imgLamp.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_BFLAMP)); break;
            }
        }

        private void SetHardwareIcons()
        {
            int camID = GetCameraID();
            int type = 0;

            if (1 == GetCameraType(camID, ref type))
            {
                imgCamera.Fill = GetCameraIconColor((int)ICamera.ConnectionStatusType.CONNECTION_READY);
            }
            else
            {
                imgCamera.Fill = GetCameraIconColor((int)ICamera.ConnectionStatusType.CONNECTION_UNAVAILABLE);
            }

            int BleachID = GetBleachID();
            GetCameraType(BleachID, ref type);
            CameraType camType = (CameraType)type;

            if (CameraType.LSM == camType)
            {
                imgBleachingScanner.Fill = GetCameraIconColor((int)ICamera.ConnectionStatusType.CONNECTION_READY);
            }
            else
            {
                imgBleachingScanner.Fill = GetCameraIconColor((int)ICamera.ConnectionStatusType.CONNECTION_UNAVAILABLE);
            }

            SetPockelsIcons();

            imgControlUnit.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_CONTROLUNIT));

            imgPMT1.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_PMT1));

            imgPMT2.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_PMT2));

            imgPMT3.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_PMT3));

            imgPMT4.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_PMT4));

            imgZStage.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_ZSTAGE));

            imgZStage2.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_ZSTAGE2));

            imgPowerReg.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_POWERREGULATOR));

            imgPowerReg2.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_POWERREGULATOR2));

            imgBeamExpan.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_BEAMEXPANDER));

            imgPinhole.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_PINHOLEWHEEL));

            imgLaserSource.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_LASER1));

            imgShutter.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_SHUTTER1));

            imgLightPath.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_LIGHTPATH));

            int xyStatus = GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_XYSTAGE);
            if ((int)IDevice.ConnectionStatusType.CONNECTION_UNAVAILABLE == xyStatus)
            {
                imgXYStage.Fill = GetDeviceIconColor(xyStatus);

                IsXYStageReady = true;

                //MessageBox.Show("An XY stage is required for ThorImageLS to run. Please either install SimDeviceXY.dll or deploy MLS stage with ThorMLSStage.dll",
                //    "XY Stage component or dll is missing", MessageBoxButton.OK, MessageBoxImage.Error);
            }
            else
            {
                imgXYStage.Fill = GetDeviceIconColor(xyStatus);
                IsXYStageReady = true;
            }

            imgSpectrumFilter.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_SPECTRUMFILTER));

            imgEpiTurret.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_EPITURRET));

            imgTurret.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)(int)SelectedHardware.SELECTED_TURRET));

            imgSLM.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_SLM));

            imgBeamStabilizer.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_BEAMSTABILIZER));

            imgLamp.Fill = GetDeviceIconColor(GetDeviceConnectionStatus((int)SelectedHardware.SELECTED_BFLAMP));
        }

        private void SetPockelsIcons()
        {
            int camID = GetCameraID();

            double val = 0;

            if (0 == GetCameraParameter(camID, (int)ICamera.Params.PARAM_LSM_POCKELS_CONNECTED_0, ref val))
            {
            }

            imgPockels1.Visibility = (0 == (int)val) ? Visibility.Collapsed : Visibility.Visible;

            val = 0;

            if (0 == GetCameraParameter(camID, (int)ICamera.Params.PARAM_LSM_POCKELS_CONNECTED_1, ref val))
            {
            }

            imgPockels2.Visibility = (0 == (int)val) ? Visibility.Collapsed : Visibility.Visible;

            val = 0;

            if (0 == GetCameraParameter(camID, (int)ICamera.Params.PARAM_LSM_POCKELS_CONNECTED_2, ref val))
            {
            }

            imgPockels3.Visibility = (0 == (int)val) ? Visibility.Collapsed : Visibility.Visible;

            val = 0;

            if (0 == GetCameraParameter(camID, (int)ICamera.Params.PARAM_LSM_POCKELS_CONNECTED_3, ref val))
            {
            }

            imgPockels4.Visibility = (0 == (int)val) ? Visibility.Collapsed : Visibility.Visible;
        }

        void SetupHardware_Loaded(object sender, RoutedEventArgs e)
        {
            loadDevs();
            if (ResourceManagerCS.Instance.TabletModeEnabled)
            {
                settingEditor.Height = 300;
            }
        }

        private void UpdateXMLBindings()
        {
            var xmldataProvider = (XmlDataProvider)this.Resources["dataProvider"];
            //xmldataProvider.IsAsynchronous = false; //perform node collection creation in the active context in stead of a worker thread
            xmldataProvider.Refresh();

            var xmldataProviderAppSet = (XmlDataProvider)this.Resources["dataProviderAppSettings"];
            //xmldataProvider.IsAsynchronous = false; //perform node collection creation in the active context in stead of a worker thread
            xmldataProviderAppSet.Refresh();
        }

        void UserControl1_Unloaded(object sender, RoutedEventArgs e)
        {
            //Update settings file in current modality
            UpdateAndPersistCurrentDevices();

            if (lbModalities.SelectedIndex >= 0)
            {
                ResourceManagerCS.SetModality(((ListBoxItem)lbModalities.Items[lbModalities.SelectedIndex]).Content.ToString());
            }
        }

        //Use to ensure that all the devices showing in the GUI are connected
        //This will update the HW Settings for each modality
        //Use after HW Refresh or at start up
        private void VerifyModalitiesSelectedDevices()
        {
            if (0 < lbModalities.Items.Count)
            {
                //Go through each of the modalities available, update all the devices
                //in the HWSettings file to match the connected devices.
                for (int i = 0; i < lbModalities.Items.Count; i++)
                {
                    ResourceManagerCS.SetModality(((ListBoxItem)lbModalities.Items[i]).Content.ToString());

                    //update the HWSettings.xml file with currently connected devices
                    UpdateAndPersistCurrentDevices();
                }
            }
            //set back to the selected modality
            if (0 <= lbModalities.SelectedIndex)
            {
                ResourceManagerCS.SetModality(((ListBoxItem)lbModalities.Items[lbModalities.SelectedIndex]).Content.ToString());

                //update the HWSettings.xml file with currently connected devices
                UpdateAndPersistCurrentDevices();
            }
        }

        #endregion Methods
    }
}