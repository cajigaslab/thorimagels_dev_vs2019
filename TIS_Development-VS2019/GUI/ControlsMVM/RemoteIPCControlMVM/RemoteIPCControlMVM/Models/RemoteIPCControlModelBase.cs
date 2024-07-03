namespace RemoteIPCControl.Models
{
    using System;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.IO;
    using System.Net;
    using System.Net.Sockets;
    using System.Runtime.InteropServices;
    using System.Windows;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Xml;

    using ThorIPCModules;

    using ThorLogging;

    using ThorSharedTypes;

    public class RemoteIPCControlModelBase
    {
        #region Enums 
        public enum IPCSaveSettings
        {
            Name,
            IP,
            IDMode,
            ActiveIndex,
            RemoteAppName

        }
        #endregion Enums
        #region Fields

        private double MM_TO_UM = 1000;
        private int _IDMode = 0;
        private bool _IPCDownlinkFlag = false;
        private string _remoteAppName = "";
        private bool _remoteConnection = false;
        private string _remotePCHostName;
        private bool _remoteSavingStats = false;
        private int _selectedRemotePCNameIndex;
        private ThorSyncMode _thorSyncSamplingMode = ThorSyncMode.FreeRun;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the RemoteIPCControlModel class
        /// </summary>
        public RemoteIPCControlModelBase()
        {
            if (ResourceManagerCS.Instance.TabletModeEnabled)
            {
                // When in tablet mode, initialize
                _remoteAppName = "Tablet";
            }
            SelectRemodePCIPAddr = new ObservableCollection<string>();
            SelectRemotePCName = new ObservableCollection<string>();
        }

        ~RemoteIPCControlModelBase()
        {
            TearDownIPC = string.Empty;
        }

        #endregion Constructors

        #region Properties

        /// <summary>
        /// Gets or sets the identifier mode.
        /// </summary>
        /// <value>
        /// The identifier mode.
        /// </value>
        public int IDMode
        {
            get
            {
                return _IDMode;
            }
            set
            {
                _IDMode = value;
                if (_IDMode == 0)
                {
                    MessageBox.Show("Please Input Other side's Computer Name. And Set the Same mode on the other Side.", "Switch Mode Warning", MessageBoxButton.OK, MessageBoxImage.Warning);
                    if (SelectRemotePCName?[_selectedRemotePCNameIndex] != "")
                    {
                        RemotePCHostName = SelectRemotePCName[_selectedRemotePCNameIndex];
                    }
                }
                else
                {
                    MessageBox.Show("Please Input Other side's Computer IP Address. And Set the Same mode on the other Side.", "Switch Mode Warning", MessageBoxButton.OK, MessageBoxImage.Warning);
                    if (SelectRemodePCIPAddr?[_selectedRemotePCNameIndex] != "")
                    {
                        RemotePCHostName = SelectRemodePCIPAddr[_selectedRemotePCNameIndex];
                    }
                }
            }
        }

        public bool IPCDownlinkFlag
        {
            get { return _IPCDownlinkFlag; }
            set { _IPCDownlinkFlag = value; }
        }

        /// <summary>
        /// Gets the local pc IPV4 Address.
        /// </summary>
        /// <value>
        /// The local pc IPV4 Address.
        /// </value>
        public string LocalPCIPv4
        {
            get
            {
                return GetLocalIP();
            }
        }

        public string NotifyOfSavedFile
        {
            set
            {
                if (value != string.Empty)
                {
                    SendToIPCController(ThorPipeCommand.NotifySavedFile, value);
                }
            }
        }

        /// <summary>
        /// Gets or sets the name of the remote application.
        /// </summary>
        /// <value>
        /// The name of the remote application.
        /// </value>
        public string RemoteAppName
        {
            get
            {
                return _remoteAppName;
            }
            set
            {
                if(_remoteAppName != value)
                {
                    SendToIPCController(ThorPipeCommand.ChangeRemoteApp, value);
                    //add change to xml
                    SaveIPCSettingToXML(IPCSaveSettings.RemoteAppName, value);
                }
                _remoteAppName = value;
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether [thor synchronize connection].
        /// </summary>
        /// <value>
        /// <c>true</c> if [thor synchronize connection]; otherwise, <c>false</c>.
        /// </value>
        public bool RemoteConnection
        {
            get
            {
                return _remoteConnection;
            }
            set
            {
                if (_remoteConnection != value)
                {
                    _remoteConnection = value;
                    if (value == true)
                    {
                        SendToIPCController(ThorPipeCommand.Establish);
                        SendToIPCController(ThorPipeCommand.AcquireInformation);
                    }
                    else
                    {
                        SendToIPCController(ThorPipeCommand.TearDown);
                    }
                }
            }
        }

        /// <summary>
        /// Gets or sets the name of the remote pc host.
        /// </summary>
        /// <value>
        /// The name of the remote pc host.
        /// </value>
        public string RemotePCHostName
        {
            get
            {
                return _remotePCHostName;
            }
            set
            {

                if (IDMode == 0)
                {
                    if(value != _remotePCHostName)
                    {
                        SendToIPCController(ThorPipeCommand.ChangeRemotePC, value);
                        SaveIPCSettingToXML(IPCSaveSettings.Name, value);
                    }
                    
                }
                else
                {
                    if (CheckIPValid(value))
                    {
                        SendToIPCController(ThorPipeCommand.ChangeRemotePC, value);
                    }
                    else
                    {
                        MessageBox.Show("IP Address Format Error", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                        return;
                    }
                }
                _remotePCHostName = value;
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether [thorsync saving statds].
        /// </summary>
        /// <value>
        /// <c>true</c> if [thorsync saving statds]; otherwise, <c>false</c>.
        /// </value>
        public bool RemoteSavingStats
        {
            get
            {
                return _remoteSavingStats;
            }
            set
            {
                _remoteSavingStats = value;
            }
        }

        /// <summary>
        /// Gets or sets the index of the selected remote pc name.
        /// </summary>
        /// <value>
        /// The index of the selected remote pc name.
        /// </value>
        public int SelectedRemotePCNameIndex
        {
            get
            {
                return _selectedRemotePCNameIndex;
            }
            set
            {
                _selectedRemotePCNameIndex = value;
            }
        }

        public ObservableCollection<string> SelectRemodePCIPAddr
        {
            get;
            set;
        }

        public ObservableCollection<string> SelectRemotePCName
        {
            get;
            set;
        }

        public string ShowMostRecent
        {
            set
            {
                if (value != string.Empty)
                {
                    SendToIPCController(ThorPipeCommand.ShowMostRecent, value);
                }
            }
        }

        public string StartAcquisition
        {
            set
            {
                if (value != string.Empty)
                {
                    SendToIPCController(ThorPipeCommand.StartAcquiring, value);
                }
            }
        }

        public bool StopAcquisition
        {
            set
            {
                if (value)
                {
                    SendToIPCController(ThorPipeCommand.StopAcquiring);
                }
            }
        }

        public string TearDownIPC
        {
            set
            {
                SendToIPCController(ThorPipeCommand.TearDown, value);
                ThorIPCModule.Instance.StopNamePipeClient();
            }
        }

        public string ThorsyncFrameSync
        {
            set
            {
                if (value != string.Empty)
                {
                    SendToIPCController(ThorPipeCommand.SyncFrame, value);
                }
            }
        }

        /// <summary>
        /// Gets or sets the thor synchronize sampling mode.
        /// </summary>
        /// <value>
        /// The thor synchronize sampling mode.
        /// </value>
        public ThorSyncMode ThorSyncSamplingMode
        {
            get
            {
                return _thorSyncSamplingMode;
            }
            set
            {
                _thorSyncSamplingMode = value;
            }
        }

        #endregion Properties

        #region Methods

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "UpdateAndPersistCurrentDevices")]
        public static extern int UpdateAndPersistCurrentDevices();

        public static void UpdatePMTSwitchBox()
        {
            ICamera.LSMType lsmType = (ICamera.LSMType)MVMManager.Instance["RunSampleLSViewModel", "LSMType", (object)ICamera.LSMAreaMode.SQUARE];
            //switch depending on LSM camera type
            switch (lsmType)
            {
                case ICamera.LSMType.GALVO_RESONANCE:
                    ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_PMTSWITCH, (int)IDevice.Params.PARAM_PMT_SWITCH_POS, 0, 0);
                    break;
                case ICamera.LSMType.GALVO_GALVO:
                    ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_PMTSWITCH, (int)IDevice.Params.PARAM_PMT_SWITCH_POS, 1, 0);
                    break;
            }
        }

        public static void SaveIPCSettingToXML(IPCSaveSettings settingToChange, string newValue)
        {
            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            if (null == doc)
            {
                return;
            }
            var root = doc.DocumentElement;//Get to the root node
            XmlElement node = (XmlElement)doc.SelectSingleNode("ApplicationSettings/IPCRemoteHostPCName");
            if (node == null)
            {
                XmlElement elementRoot = doc.CreateElement(string.Empty, "IPCRemoteHostPCName", string.Empty);
                XmlElement rootNode = (XmlElement)doc.SelectSingleNode("ApplicationSettings");
                rootNode.AppendChild(elementRoot);
                node = (XmlElement)doc.SelectSingleNode("ApplicationSettings/IPCRemoteHostPCName");
            }

            node.SetAttribute(ConvertEnumToString(settingToChange), newValue);
            

            
            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS, true);
        }

        public static string ConvertEnumToString(IPCSaveSettings settingToChange)
        {
            string returnVal = "";

            switch (settingToChange)
            {
                case IPCSaveSettings.Name:
                    returnVal = "name";
                    break;
                case IPCSaveSettings.ActiveIndex:
                    returnVal = "activeIndex";
                    break;
                case IPCSaveSettings.RemoteAppName:
                    returnVal = "remoteAppName";
                    break;
                case IPCSaveSettings.IDMode:
                    returnVal = "IDMode";
                    break;
                case IPCSaveSettings.IP:
                    returnVal = "IP";
                    break;

            }
            return returnVal;
        }

        /// <summary>
        /// Check Input IP is valid or not
        /// </summary>
        ///
        /// <param name="ip">IP message</param>
        ///
        /// <exception>NONE</exception>
        public bool CheckIPValid(string ip)
        {
            string[] parts = ip.Split('.');
            if (parts.Length == 4)
            {
                foreach (string part in parts)
                {
                    byte checkPart = 0;
                    if (!byte.TryParse(part, out checkPart))
                    {
                        return false;
                    }
                }
                return true;
            }
            else
            {
                return false;
            }
        }

        /// <summary>
        /// Get Local IPv4 Address
        /// </summary>
        ///
        /// <param>None</param>
        ///
        /// <exception>NONE</exception>
        public string GetLocalIP()
        {
            if (true == System.Net.NetworkInformation.NetworkInterface.GetIsNetworkAvailable())
            {
                var host = Dns.GetHostEntry(Dns.GetHostName());
                foreach (var ip in host.AddressList)
                {
                    if (ip.AddressFamily == AddressFamily.InterNetwork)
                    {
                        return ip.ToString();
                    }
                }
            }
            return "";
        }

        /// <summary>
        /// Receive and analyze the message, which comes from IPC controller
        /// </summary>
        ///
        /// <param name="command">type of IPC message</param>
        /// <param name="data">Payload </param>
        ///
        /// <exception>NONE</exception>
        public void ReceiveFromIPCController(ThorPipeCommand command, string data)
        {
            //ThorPipeCommand cmd = (ThorPipeCommand)(Enum.Parse(typeof(ThorPipeCommand), command));
            double val = 0;
            IPCDownlinkFlag = true;
            switch (command)
            {
                case ThorPipeCommand.Establish:
                    _remoteConnection = true;
                    break;
                case ThorPipeCommand.TearDown:
                    _remoteConnection = false;
                    break;
                case ThorPipeCommand.AcquireInformation:
                    break;
                case ThorPipeCommand.UpdateInformation:
                    if (data.Contains("/"))
                    {
                        string[] remoteConfigurationData = data.Split('/');
                        if (remoteConfigurationData.Length == 2)
                        {
                            RemoteSavingStats = Convert.ToBoolean(remoteConfigurationData[0]);
                            ThorSyncSamplingMode = (ThorSyncMode)(Convert.ToInt32(remoteConfigurationData[1]));
                        }
                    }
                    break;
                case ThorPipeCommand.FilePath:
                    if ((bool)MVMManager.Instance["RemoteIPCControlViewModelBase", "RemoteSavingStats", (object)false])
                        MVMManager.Instance["RemoteIPCControlViewModelBase", "ThorSyncFilePath", (object)false] = data;

                    break;
                case ThorPipeCommand.StartAcquiring:

                    if (((int)MVMManager.Instance["RemoteIPCControlViewModelBase", "SelectedTabIndex", (object)-1]) != 1)
                    {
                        break;
                    }

                    if (RemoteSavingStats == true)
                    {
                        if (!string.IsNullOrEmpty(data))
                        {
                            if (data.Contains("\0"))
                            {
                                data = data.Split('\0')[0];
                            }
                            string outputPath = Path.GetDirectoryName(data);
                            MVMManager.Instance["RunSampleLSViewModel", "OutputPath"] = outputPath;
                        }

                        ICommand startCommand = (ICommand)MVMManager.Instance["RunSampleLSViewModel", "RunSampleLSStartCommand", (object)null];
                        startCommand.Execute(null);
                    }
                    break;
                case ThorPipeCommand.StopAcquiring:
                    
                    if (((int)MVMManager.Instance["RemoteIPCControlViewModelBase", "SelectedTabIndex", (object)-1]) != 1)
                    {
                        //show pop up box to Move TI to capture for IPC acquisition
                        break;
                    }

                    if (!(bool)MVMManager.Instance["RunSampleLSViewModel", "RunComplete", (object)false])
                    {
                        ICommand stopCommand = (ICommand)MVMManager.Instance["RunSampleLSViewModel", "RunSampleLSStopCommand", (object)null];
                        stopCommand.Execute(null);
                    }
                    break;
                case ThorPipeCommand.StartBleach:
                    break;
                case ThorPipeCommand.StopBleach:
                    break;
                case ThorPipeCommand.Receive:
                    break;
                case ThorPipeCommand.Error:
                    break;
                case ThorPipeCommand.LoadExperimentFile:
                    if (data.Contains("\0"))
                    {
                        data = data.Split('\0')[0];
                    }
                    if (File.Exists(data))
                    {
                        XmlDocument doc = new XmlDocument();

                        //Replace all the active files with the ones from the template.
                        ResourceManagerCS.Instance.ReplaceActiveXML(data);

                        if (File.Exists(data))
                        {
                            doc.Load(data);
                        }
                        const string MODALITY = "ThorImageExperiment/Modality";

                        XmlNode node = doc.SelectSingleNode(MODALITY);

                        string str = string.Empty;

                        if (null != node)
                        {
                            if (ThorSharedTypes.XmlManager.GetAttribute(node, doc, "name", ref str))
                            {
                                //ensure the HW matches the available HW
                                UpdateAndPersistCurrentDevices();

                                //Update the PMT Switch box (for LSM only) to GG or GR depending on current camera
                                UpdatePMTSwitchBox();
                            }
                        }

                        int captureModeIndex = 0;
                        node = doc.SelectSingleNode("/ThorImageExperiment/CaptureMode");

                        if (null != node)
                        {
                            if (ThorSharedTypes.XmlManager.GetAttribute(node, doc, "mode", ref str))
                            {
                                captureModeIndex = Int32.Parse(str);
                            }
                        }

                        MVMManager.Instance["RunSampleLSViewModel", "CaptureMode"] = captureModeIndex;
                    }
                    break;
                case ThorPipeCommand.IsSaving:
                    MVMManager.Instance["RemoteIPCControlViewModelBase", "IsSaving", (object)false] = data == "1" ? true : false;
                    break;
                case ThorPipeCommand.MoveX:
                    val = Convert.ToDouble(data) / MM_TO_UM;
                    if (0 == ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_X_POS, val, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RemoteIPCControlMVM Model, could not set PARAM_X_POS");
                    }
                    break;
                case ThorPipeCommand.MoveY:
                    val = Convert.ToDouble(data) / MM_TO_UM;
                    if (0 == ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_Y_POS, val, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RemoteIPCControlMVM Model, could not set PARAM_Y_POS");
                    }
                    break;
                case ThorPipeCommand.MoveZ:
                    val = Convert.ToDouble(data) / MM_TO_UM;
                    if (0 == ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_ZSTAGE, (int)IDevice.Params.PARAM_Z_POS, val, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RemoteIPCControlMVM Model, could not set PARAM_Z_POS for ZSTAGE");
                    }
                    break;
                case ThorPipeCommand.MoveSecondaryZ:
                    val = Convert.ToDouble(data) / MM_TO_UM;
                    if (0 == ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_ZSTAGE2, (int)IDevice.Params.PARAM_Z_POS, val, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RemoteIPCControlMVM Model, could not set PARAM_Z_POS for ZSTAGE2");
                    }
                    break;
                case ThorPipeCommand.NotifySavedFile:
                    break;
                case ThorPipeCommand.ReportPositionX:
                    val = 0;
                    if (0 == ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_X_POS, ref val))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RemoteIPCControlMVM Model, could not get PARAM_X_POS");
                    }
                    SendToIPCController(ThorPipeCommand.PositionReportX, (val * MM_TO_UM).ToString());
                    break;
                case ThorPipeCommand.ReportPositionY:
                    val = 0;
                    if (0 == ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_Y_POS, ref val))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RemoteIPCControlMVM Model, could not get PARAM_Y_POS");
                    }
                    SendToIPCController(ThorPipeCommand.PositionReportY, (val * MM_TO_UM).ToString());
                    break;
                case ThorPipeCommand.ReportPositionZ:
                    val = 0;
                    if (0 == ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_ZSTAGE, (int)IDevice.Params.PARAM_Z_POS, ref val))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RemoteIPCControlMVM Model, could not get PARAM_Z_POS for ZSTAGE");
                    }
                    SendToIPCController(ThorPipeCommand.PositionReportZ, (val * MM_TO_UM).ToString());
                    break;
                case ThorPipeCommand.ReportPositionSecondaryZ:
                    val = 0;
                    if (0 == ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_ZSTAGE2, (int)IDevice.Params.PARAM_Z_POS, ref val))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RemoteIPCControlMVM Model, could not get PARAM_Z_POS for ZSTAGE2");
                    }
                    SendToIPCController(ThorPipeCommand.PositionReportSecondaryZ, (val * MM_TO_UM).ToString());
                    break;
                case ThorPipeCommand.PositionReportX:
                    break;
                case ThorPipeCommand.PositionReportY:
                    break;
                case ThorPipeCommand.PositionReportZ:
                    break;
                case ThorPipeCommand.PositionReportSecondaryZ:
                    break;
                case ThorPipeCommand.ShowMostRecent:
                    break;
                case ThorPipeCommand.SyncFrame:

                    int ZMax = (int)MVMManager.Instance["ImageReviewViewModel", "ZMax"];

                    if (ZMax == 1) //if series mode <- there is likely a better condition
                    {
                        MVMManager.Instance["ImageReviewViewModel", "TValue"] = int.Parse(data);
                    }
                    else //calculate Z and T Val from frame # in data arg
                    {
                        int FrameNum = int.Parse(data);

                        double newTVal = (FrameNum / ZMax) + 1;

                        //force update the BitMap image the sliders had the correct Z and T values but the BitMap frame was not correctly corresponding to them
                        Application.Current.Dispatcher.Invoke((Action)(() =>
                        {

                            if (FrameNum == 1 || FrameNum == 0)
                            {
                                MVMManager.Instance["ImageReviewViewModel", "ChangeFromIPCZValue"] = 1;

                                MVMManager.Instance["ImageReviewViewModel", "ChangeFromIPCTValue"] = 1;
                            }
                            else if (FrameNum % ZMax == 0)
                            {

                                MVMManager.Instance["ImageReviewViewModel", "ChangeFromIPCZValue"] = ZMax;

                                MVMManager.Instance["ImageReviewViewModel", "ChangeFromIPCTValue"] = ((FrameNum - 1) / ZMax) + 1;
                            }
                            else
                            {
                                MVMManager.Instance["ImageReviewViewModel", "ChangeFromIPCZValue"] = FrameNum % ZMax;

                                MVMManager.Instance["ImageReviewViewModel", "ChangeFromIPCTValue"] = ((FrameNum - 1) / ZMax) + 1;
                            }

                        }));

                    }

                    break;
                default:
                    break;
            }
            IPCDownlinkFlag = false;
        }

        /// <summary>
        /// Sends to ipc controller.
        /// </summary>
        /// <param name="command">The command.</param>
        /// <param name="data">The data.</param>
        public void SendToIPCController(ThorPipeCommand command, string data = "")
        {
            //Only allow these commands to be sent if the _remoteConnection is down
            if (_remoteConnection || command == ThorPipeCommand.Establish ||
                command == ThorPipeCommand.AcquireInformation ||
                command == ThorPipeCommand.UpdateInformation ||
                command == ThorPipeCommand.TearDown ||
                command == ThorPipeCommand.ChangeRemoteApp ||
                command == ThorPipeCommand.ChangeRemotePC)
            {
                ThorIPCModule.Instance.ReceiveUplinkCommand(command, data);
            }
        }

        #endregion Methods
    }
}