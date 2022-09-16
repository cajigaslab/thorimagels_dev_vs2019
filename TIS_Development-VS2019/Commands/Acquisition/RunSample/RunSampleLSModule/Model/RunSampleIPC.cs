namespace RunSampleLSDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.IO.Pipes;
    using System.Linq;
    using System.Net;
    using System.Net.Sockets;
    using System.Runtime.InteropServices;
    using System.Security.Principal;
    using System.Text;
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

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorImageInfastructure;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class RunSampleLS
    {
        #region Fields

        public IEventAggregator _eventAggregator;
        public string[] _selectRemodePCIPAddr = new string[4] { "", "", "", "" };
        public string[] _selectRemotePCName = new string[4] { "", "", "", "" };

        private int _IDMode = 0;
        private string _remoteAppName = "ThorSync";
        private bool _remoteConnection = false;
        private string _remotePCHostName;
        private bool _remoteSavingStats = false;
        private int _selectedRemotePCNameIndex;
        private ThorSyncMode _thorSyncSamplingMode = ThorSyncMode.FreeRun;

        #endregion Fields

        #region Enumerations

        public enum ThorPipeCommand
        {
            Establish,
            TearDown,
            AcquireInformation,
            UpdataInformation,
            FilePath,
            StartAcquiring,
            StopAcquiring,
            StartBleach,
            StopBleach,
            Receive,
            Error,
            ChangeRemotePC,
            ChangeRemoteApp,
            LoadExperimentFile,
            MoveX,
            MoveY,
            MoveZ,
            MoveSecondaryZ,
            NotifySavedFile
        }

        public enum ThorPipeDst
        {
            ThorImage,
            ThorSync
        }

        public enum ThorPipeSrc
        {
            ThorImage,
            ThorSync
        }

        public enum ThorPipeStatus
        {
            ThorPipeStsNoError = 0,
            ThorPipeStsBusy = 1,

            ThorPipeFormatError = 10,
            ThorPipeFormatRoutingError = 11,

            ThorPipeError = 99,
        }

        public enum ThorSyncMode
        {
            FreeRun,
            HardwareTriggerSingle,
            HardwareTriggerRetriggerable,
            HardwareSynchronizable
        }

        #endregion Enumerations

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
                if (_IDMode != value)
                {
                    _IDMode = value;
                    OnPropertyChanged("IDMode");
                    if (_IDMode == 0)
                    {
                        MessageBox.Show("Please Input Other side's Computer Name. And Set the Same mode on the other Side.", "Switch Mode Warning", MessageBoxButton.OK, MessageBoxImage.Warning);
                        if (_selectRemotePCName[_selectedRemotePCNameIndex] != "")
                        {
                            RemotePCHostName = _selectRemotePCName[_selectedRemotePCNameIndex];
                        }
                    }
                    else
                    {
                        MessageBox.Show("Please Input Other side's Computer IP Address. And Set the Same mode on the other Side.", "Switch Mode Warning", MessageBoxButton.OK, MessageBoxImage.Warning);
                        if (_selectRemodePCIPAddr[_selectedRemotePCNameIndex] != "")
                        {
                            RemotePCHostName = _selectRemodePCIPAddr[_selectedRemotePCNameIndex];
                        }
                    }
                    SaveRemotePCHostNameToXML();
                }

            }
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
                if (_remoteAppName != value || _remoteConnection == false)
                {
                    SendToIPCController(ThorPipeCommand.ChangeRemoteApp, value);
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
                    if (IPCDownlinkFlag == false)
                    {
                        if (value == true)
                        {
                            SendToIPCController(ThorPipeCommand.Establish);
                        }
                        else
                        {
                            SendToIPCController(ThorPipeCommand.TearDown);
                        }
                    }
                    else
                    {
                        _remoteConnection = value;
                        if (UpdateRemoteConnection != null)
                        {
                            UpdateRemoteConnection(_remoteConnection);
                        }
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
                    SendToIPCController(ThorPipeCommand.ChangeRemotePC, value);
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

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetApplicationSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        public static extern int GetApplicationSettingsFilePathAndName(StringBuilder sb, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetCaptureTemplatePath", CharSet = CharSet.Unicode)]
        public static extern int GetCaptureTemplatePath(StringBuilder sb, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetHardwareSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        public static extern int GetHardwareSettingsFilePathAndName(StringBuilder sb, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetMyDocumentsThorImageFolder", CharSet = CharSet.Unicode)]
        public static extern int GetMyDocumentsThorImageFolder(StringBuilder sb, int length);

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
        /// Loads the remote pc host name from XML.
        /// </summary>
        public void LoadRemotePCHostNameFromXML()
        {
            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            if (null == doc)
            {
                return;
            }

            string strTemp = string.Empty;
            XmlNodeList nodeList = doc.SelectNodes("ApplicationSettings/IPCRemoteHostPCName");
            if (nodeList.Count != 0)
            {
                foreach (XmlNode node in nodeList)
                {
                    if (XmlManager.GetAttribute(node, doc, "name", ref strTemp))
                    {
                        string[] hostname = strTemp.Split('/');
                        for (int i = 0; i < hostname.Length; i++)
                        {
                            _selectRemotePCName[i] = hostname[i];
                        }
                    }
                    if (XmlManager.GetAttribute(node, doc, "IP", ref strTemp))
                    {
                        string[] ipAddr = strTemp.Split('/');
                        for (int i = 0; i < ipAddr.Length; i++)
                        {
                            _selectRemodePCIPAddr[i] = ipAddr[i];
                        }
                    }
                    if (XmlManager.GetAttribute(node, doc, "IDMode", ref strTemp))
                    {
                        _IDMode = Convert.ToInt32(strTemp);
                        OnPropertyChanged("IDMode");
                    }
                    if (XmlManager.GetAttribute(node, doc, "activeIndex", ref strTemp))
                    {
                        SelectedRemotePCNameIndex = Convert.ToInt32(strTemp);
                    }
                    if (XmlManager.GetAttribute(node, doc, "remoteAppName", ref strTemp))
                    {
                        _remoteAppName = strTemp;
                        OnPropertyChanged("RemoteAppName");
                    }
                }
            }
            else
            {
                _selectRemotePCName[0] = System.Environment.MachineName;
                _selectRemodePCIPAddr[0] = GetLocalIP();
                SaveRemotePCHostNameToXML();
            }
        }

        /// <summary>
        /// Receive and analyze the message, which comes from IPC controller
        /// </summary>
        ///
        /// <param name="command">type of IPC message</param>
        /// <param name="data">Payload </param>
        ///
        /// <exception>NONE</exception>
        public void ReceiveFromIPCController(string command, string data)
        {
            ThorPipeCommand cmd = (ThorPipeCommand)(Enum.Parse(typeof(ThorPipeCommand), command));
            double val = 0;
            IPCDownlinkFlag = true;
            switch (cmd)
            {
                case ThorPipeCommand.Establish:
                    RemoteConnection = true;
                    break;
                case ThorPipeCommand.TearDown:
                    RemoteConnection = false;
                    break;
                case ThorPipeCommand.AcquireInformation:
                    break;
                case ThorPipeCommand.UpdataInformation:
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
                    break;
                case ThorPipeCommand.StartAcquiring:
                    if (RemoteSavingStats == true)
                    {
                        this.OutputPath = System.IO.Path.GetDirectoryName(data);
                        if (String.IsNullOrEmpty(this.ExperimentName.FullName) || String.IsNullOrWhiteSpace(this.ExperimentName.FullName))
                        {
                            this.ExperimentName.FullName = "ThorImage_001";
                        }

                        if (Directory.Exists(new StringBuilder(_outputPath + "\\" + _experimentName.FullName).ToString()))
                        {
                            this.ExperimentName.MakeUnique(_outputPath);
                        }

                        if (UpdataFilePath != null)
                        {
                            UpdataFilePath();
                        }
                        Start();
                    }
                    break;
                case ThorPipeCommand.StopAcquiring:
                    if (RunComplete == false)
                    {
                        Stop();
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

                        const int NAME_LENGTH = 256;
                        StringBuilder startModality = new StringBuilder(NAME_LENGTH);

                        if (null != node)
                        {
                            if (ThorSharedTypes.XmlManager.GetAttribute(node, doc, "name", ref str))
                            {
                                //Get the current modality and store it to
                                //startModality
                                //GetModality(startModality, NAME_LENGTH);

                                if (str.Equals(startModality.ToString()))
                                {
                                }
                                else
                                {
                                    //if the modality differs fromt the modality
                                    //in the script attempt to set the modality
                                    StringBuilder sb = new StringBuilder(NAME_LENGTH);
                                    sb.Append(str);
                                    //SetModality(sb);

                                    //ensure the HW matches the available HW

                                    RunSampleLS.UpdateAndPersistCurrentDevices();

                                    //Update the PMT Switch box (for LSM only) to GG or GR depending on current camera
                                    RunSampleLS.UpdatePMTSwitchBox();
                                }
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

                        CaptureMode = captureModeIndex;
                        //OnPropertyChanged("CaptureMode");

                        Command commandIPC = new Command();
                        commandIPC.Message = "Run Sample LS";
                        commandIPC.CommandGUID = new Guid("30db4357-7508-46c9-84eb-3ca0900aa4c5");

                        _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(commandIPC);

                    }
                    break;
                case ThorPipeCommand.MoveX:
                    val = Convert.ToDouble(data);
                    if (1 == ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_X_POS, val, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT))
                    {

                    }
                    break;
                case ThorPipeCommand.MoveY:
                    val = Convert.ToDouble(data);
                    if (1 == ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_Y_POS, val, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT))
                    {

                    }
                    break;
                case ThorPipeCommand.MoveZ:
                    val = Convert.ToDouble(data);
                    if (1 == ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_ZSTAGE, (int)IDevice.Params.PARAM_Z_POS, val, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT))
                    {

                    }
                    break;
                case ThorPipeCommand.MoveSecondaryZ:
                    val = Convert.ToDouble(data);
                    if (1 == ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_ZSTAGE2, (int)IDevice.Params.PARAM_Z_POS, val, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT))
                    {

                    }
                    break;
                case ThorPipeCommand.NotifySavedFile:
                    break;
                default:
                    break;
            }
            IPCDownlinkFlag = false;
        }

        /// <summary>
        /// Saves the remote pc host name to XML.
        /// </summary>
        public void SaveRemotePCHostNameToXML()
        {
            string remotePCHostNameList = String.Join("/", _selectRemotePCName);
            string remotePCIPAddressList = String.Join("/", _selectRemodePCIPAddr);

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
            node.SetAttribute("name", remotePCHostNameList.ToString());
            node.SetAttribute("IP", remotePCIPAddressList.ToString());
            node.SetAttribute("IDMode", IDMode.ToString());
            node.SetAttribute("activeIndex", SelectedRemotePCNameIndex.ToString());
            node.SetAttribute("remoteAppName", RemoteAppName.ToString());
            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS, true);
        }

        /// <summary>
        /// Sends the data.
        /// </summary>
        /// <param name="source">The source.</param>
        /// <param name="destination">The destination.</param>
        /// <param name="commandType">Type of the command.</param>
        /// <param name="data">The data.</param>
        public void sendData(String source, String destination, string commandType, string data = "")
        {
            IPCCommand command = new IPCCommand();
            command.Source = source;
            command.Destination = destination;
            command.CommandType = commandType;
            command.Data = data;
            //command published to change the status of the menu buttons in the Menu Control
            _eventAggregator.GetEvent<CommandIPCEvent>().Publish(command);
        }

        /// <summary>
        /// Sends to ipc controller.
        /// </summary>
        /// <param name="command">The command.</param>
        /// <param name="data">The data.</param>
        public void SendToIPCController(ThorPipeCommand command, string data = "")
        {
            switch (command)
            {
                case ThorPipeCommand.Establish:
                    sendData("RUN_SAMPLE", "IPC_CONTROLLER", Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.Establish));
                    break;
                case ThorPipeCommand.TearDown:
                    sendData("RUN_SAMPLE", "IPC_CONTROLLER", Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.TearDown));
                    break;
                case ThorPipeCommand.AcquireInformation:
                    sendData("RUN_SAMPLE", "IPC_CONTROLLER", Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.AcquireInformation));
                    break;
                case ThorPipeCommand.UpdataInformation:
                    break;
                case ThorPipeCommand.FilePath:
                    break;
                case ThorPipeCommand.StartAcquiring:
                    sendData("RUN_SAMPLE", "IPC_CONTROLLER", Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.StartAcquiring), this.OutputPath + "\\" + this.ExperimentName);
                    break;
                case ThorPipeCommand.StopAcquiring:
                    sendData("RUN_SAMPLE", "IPC_CONTROLLER", Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.StopAcquiring));
                    break;
                case ThorPipeCommand.ChangeRemotePC:
                    sendData("RUN_SAMPLE", "IPC_CONTROLLER", Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.ChangeRemotePC), data);
                    break;
                case ThorPipeCommand.ChangeRemoteApp:
                    sendData("RUN_SAMPLE", "IPC_CONTROLLER", Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.ChangeRemoteApp), data);
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
                    break;
                case ThorPipeCommand.MoveX:
                    break;
                case ThorPipeCommand.MoveY:
                    break;
                case ThorPipeCommand.MoveZ:
                    break;
                case ThorPipeCommand.MoveSecondaryZ:
                    break;
                case ThorPipeCommand.NotifySavedFile:
                    sendData("RUN_SAMPLE", "IPC_CONTROLLER", Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.NotifySavedFile), data);
                    break;
                default:
                    break;
            }
        }

        #endregion Methods
    }
}