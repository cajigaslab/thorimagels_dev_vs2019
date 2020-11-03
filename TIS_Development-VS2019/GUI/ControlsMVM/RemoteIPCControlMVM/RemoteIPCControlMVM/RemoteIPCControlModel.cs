namespace RemoteIPCControl.Model
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Net;
    using System.Net.Sockets;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Timers;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorImageInfastructure;

    using ThorLogging;

    using ThorSharedTypes;

    public class RemoteIPCControlModel
    {
        #region Fields

        public IEventAggregator _eventAggregator;

        private string _experimentPath = "C:\\temp\\exp01";
        private int _IDMode = 0;
        private bool _IPCDownlinkFlag = false;
        private string _remoteAppName = "ThorSync";
        private bool _remoteConnection = false;
        private string _remotePCHostName;
        private bool _remoteSavingStats = false;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the RemoteIPCControlModel class
        /// </summary>
        public RemoteIPCControlModel()
        {
            if (ResourceManagerCS.Instance.TabletModeEnabled)
            {
                // When in tablet mode, initialize
                _remoteAppName = "Tablet";
            }
        }

        #endregion Constructors

        #region Properties

        public IEventAggregator EventAggregator
        {
            get
            {
                return _eventAggregator;
            }
            set
            {
                _eventAggregator = value;
            }
        }

        public string ExperimentPath
        {
            get
            {
                return _experimentPath;
            }
            set
            {
                _experimentPath = value;
            }
        }

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
                    //OnPropertyChanged("IDMode");
                    if (_IDMode == 0)
                    {
                        MessageBox.Show("Please Input Other side's Computer Name. And Set the Same mode on the other Side.", "Switch Mode Warning", MessageBoxButton.OK, MessageBoxImage.Warning);
                    }
                    else
                    {
                        MessageBox.Show("Please Input Other side's Computer IP Address. And Set the Same mode on the other Side.", "Switch Mode Warning", MessageBoxButton.OK, MessageBoxImage.Warning);
                    }
                    //SaveRemotePCHostNameToXML();
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
                SendToIPCController(ThorPipeCommand.ChangeRemoteApp, value);

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
                    //if (IPCDownlinkFlag == false)
                    //{
                    _remoteConnection = value;
                        if (value == true)
                        {
                            SendToIPCController(ThorPipeCommand.Establish);
                        }
                        else
                        {
                            SendToIPCController(ThorPipeCommand.TearDown);
                        }
                    //}
                    //else
                    //{
                    //    _remoteConnection = value;
                        //if (UpdateRemoteConnection != null)
                        //{
                        //    UpdateRemoteConnection(_remoteConnection);
                        //}
                    //}
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

        public string StartAcquisition
        {
            set
            {
                if(value != string.Empty)
                {
                    ExperimentPath = value;
                    SendToIPCController(ThorPipeCommand.StartAcquiring, ExperimentPath);
                }
            }
        }

        #endregion Properties

        #region Methods

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
        public void ReceiveFromIPCController(string command, string data)
        {
            ThorPipeCommand cmd = (ThorPipeCommand)(Enum.Parse(typeof(ThorPipeCommand), command));
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
                            //ThorSyncSamplingMode = (ThorSyncMode)(Convert.ToInt32(remoteConfigurationData[1]));
                        }
                    }
                    break;
                case ThorPipeCommand.FilePath:
                    break;
                case ThorPipeCommand.StartAcquiring:
                    if (RemoteSavingStats == true)
                    {
                        //this.OutputPath = System.IO.Path.GetDirectoryName(data);
                        //if (String.IsNullOrEmpty(this.ExperimentName.FullName) || String.IsNullOrWhiteSpace(this.ExperimentName.FullName))
                        //{
                        //    this.ExperimentName.FullName = "ThorImage_001";
                        //}

                        //if (Directory.Exists(new StringBuilder(_outputPath + "\\" + _experimentName.FullName).ToString()))
                        //{
                        //    this.ExperimentName.MakeUnique(_outputPath);
                        //}

                        //if (UpdataFilePath != null)
                        //{
                        //    UpdataFilePath();
                        //}
                        //Start();
                    }
                    break;
                case ThorPipeCommand.StopAcquiring:
                    //if (RunComplete == false)
                    //{
                    //    Stop();
                    //}
                    break;
                case ThorPipeCommand.StartBleach:
                    break;
                case ThorPipeCommand.StopBleach:
                    break;
                case ThorPipeCommand.Receive:
                    break;
                case ThorPipeCommand.Error:
                    break;
                default:
                    break;
            }
            IPCDownlinkFlag = false;
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
            if (null != _eventAggregator)
            {
                _eventAggregator.GetEvent<CommandIPCEvent>().Publish(command);
            }
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
                    sendData("RUN_SAMPLE", "IPC_CONTROLLER", Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.StartAcquiring), ExperimentPath);
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
                default:
                    break;
            }
        }

        #endregion Methods
    }
}