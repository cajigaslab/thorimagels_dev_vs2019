/// <summary>
/// 
/// </summary>
namespace RealTimeLineChart.ViewModel
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
    using System.Xml;
    using System.Xml.Linq;
    using ThorLogging;

    using global::RealTimeLineChart.Model;

    public class ReadFileToStream
    {
        #region Fields

        private string fn;
        private StreamString ss;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="ReadFileToStream"/> class.
        /// </summary>
        /// <param name="str">The string.</param>
        /// <param name="filename">The filename.</param>
        public ReadFileToStream(StreamString str, string filename)
        {
            fn = filename;
            ss = str;
        }

        #endregion Constructors

        #region Methods

        /// <summary>
        /// Starts this instance.
        /// </summary>
        public void Start()
        {
            string contents = File.ReadAllText(fn);
            ss.WriteString(contents);
        }

        #endregion Methods
    }

    /// <summary>
    /// 
    /// </summary>
    public partial class RealTimeLineChartViewModel : ViewModelBase
    {
        #region Fields

        public readonly int DataLength = 4;

        public int _selectedRemotePCNameIndex;
        public string[] _selectRemodePCIPAddr = new string[4] { "", "", "", "" };
        public string[] _selectRemotePCName = new string[4] { "", "", "", "" };

        string _clientPipeName = string.Empty;
        string _connectionClientID;
        string _connectionServerID;
        System.Timers.Timer _connectionTimer;
        int _IDMode = 0;
        Thread _pipeClient = null;
        NamedPipeServerStream _pipeServer;
        bool _receiveIPCCommandActive = false;
        string _remotePCHostName;
        string[] _sendBuffer = null;
        string _serverPipeName = string.Empty;
        Thread _serverThread = null;
        bool _thorImageLSConnectionStats = false;
        bool _sendServerRunning = false;
        private bool _disconnectedPopup = false;
        bool _disableDuplicateIPC = false;

        Queue<String[]> sendCmdBuffer = new Queue<String[]>(); //Queue for sending TS cmds
        private object sendCmdBufferLock = new object();       //Lock for the Queue above

        int syncCMDCount = 0;

        #endregion Fields

        #region Enumerations

        public enum ThorPipeCommand
        {
            Establish,
            TearDown,
            AcquireInformation,
            UpdateInformation,
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
            NotifySavedFile,
            ReportPositionX,
            ReportPositionY,
            ReportPositionZ,
            ReportPositionSecondaryZ,
            PositionReportX,
            PositionReportY,
            PositionReportZ,
            PositionReportSecondaryZ,

            ShowMostRecent,
            SyncFrame,
            IsSaving
        }

        public enum ThorPipeDst
        {
            Remote,
            Local
        }

        public enum ThorPipeSrc
        {
            Remote,
            Local
        }

        public enum ThorPipeStatus
        {
            ThorPipeStsNoError = 0,
            ThorPipeStsBusy = 1,
            ThorPipeStsBlankCommandError = 2,
            ThorPipeStreamNotSupportedError = 3,
            ThorPipeFormatError = 10,
            ThorPipeFormatRoutingError = 11,
            ThorpipeIOError = 20,
            ThorPipeError = 99,
        }

        #endregion Enumerations

        #region Properties

        /// <summary>
        /// Gets or sets the name of the client pipe.
        /// </summary>
        /// <value>
        /// The name of the client pipe.
        /// </value>
        public string ClientPipeName
        {
            get
            {
                return _clientPipeName;
            }
            set
            {
                _clientPipeName = value;
                OnPropertyChanged("ClientPipeName");
            }
        }

        public Visibility IPCEnabled
        {
            get
            {
                if (IPCDisabled)
                {
                    return Visibility.Collapsed;
                }
                else
                {
                    return Visibility.Visible; 
                }

            }
        }

        public bool IPCDisabled
        {
            get
            {
                return _disableDuplicateIPC;
            }
            set
            {
                if(true == _disableDuplicateIPC && false == value)
                {
                    //run check for other ThorSync process to init IPC

                    if (true == checkToDisableIPC()) //IPC wants to be turned on however there are still other thorsync running
                    {
                        _disableDuplicateIPC = true;
                    }
                    else
                    {
                        InitIPC();
                        _disableDuplicateIPC = value;
                    }
                }
                else
                {
                    _disableDuplicateIPC = value;
                }
                OnPropertyChanged("IPCDisabled");
                OnPropertyChanged("IPCEnabled");
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
        /// Gets the name of the local pc host.
        /// </summary>
        /// <value>
        /// The name of the local pc host.
        /// </value>
        public string LocalPCHostName
        {
            get
            {
                return GetHostName();
            }
        }

        /// <summary>
        /// Gets the local PC IPv4 Address.
        /// </summary>
        /// <value>
        /// The local PC IPv4 Address.
        /// </value>
        public string LocalPCIPv4
        {
            get
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
                if (IPCDisabled)
                {
                    StopNamedPipeClient();
                    return;
                }

                if (value == "")
                {
                    return;
                }
                if (IDMode == 0)
                {
                    Disconnect();
                    Thread.Sleep(100);
                    _thorImageLSConnectionStats = false;
                    _connectionServerID = "ThorSyncThorImagePipe";
                    _connectionClientID = "ThorImageThorSyncPipe";
                    StopNamedPipeClient();
                    StartNamedPipeClient();
                }
                else
                {
                    if (CheckIPValid(value))
                    {
                        Disconnect();
                        Thread.Sleep(100);
                        _thorImageLSConnectionStats = false;
                        _connectionServerID = "ThorSyncThorImagePipe";
                        _connectionClientID = "ThorImageThorSyncPipe";
                        StopNamedPipeClient();
                        StartNamedPipeClient();
                    }
                    else
                    {
                        MessageBox.Show("IP Address Format Error", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                        return;
                    }
                }
                _remotePCHostName = value;
                OnPropertyChanged("RemotePCHostName");
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
                OnPropertyChanged("SelectedRemotePCNameIndex");
            }
        }

        /// <summary>
        /// Gets or sets the name of the server pipe.
        /// </summary>
        /// <value>
        /// The name of the server pipe.
        /// </value>
        public string ServerPipeName
        {
            get
            {
                return _serverPipeName;
            }
            set
            {
                _serverPipeName = value;
                OnPropertyChanged("ServerPipeName");
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether [thor image ls connection stats].
        /// </summary>
        /// <value>
        /// <c>true</c> if [thor image ls connected]; otherwise, <c>false</c>.
        /// </value>
        public bool ThorImageLSConnectionStats
        {
            get
            {
                return _thorImageLSConnectionStats;
            }
            set
            {
                if (_thorImageLSConnectionStats != value) // value changed
                {
                    if (_receiveIPCCommandActive == false) //controlled by thorsync
                    {
                        if (value == true) // Connect
                        {
                            Connect();
                        }
                        else// Disconnect
                        {
                            Disconnect();
                        }
                    }
                        _thorImageLSConnectionStats = value;
                        OnPropertyChanged("ThorImageLSConnectionStats");
                    OnPropertyChanged("IsBlinking");
                }
            }
        }

        #endregion Properties

        #region Methods

        public static bool NamedPipeDoesNotExist(string pipeName, string serverName)
        {
            try
            {
                int timeout = 0;
                string normalizedPath = System.IO.Path.GetFullPath(
                 string.Format(@"\\{0}\pipe\{1}", serverName, pipeName));
                bool exists = WaitNamedPipe(normalizedPath, timeout);
                if (!exists)
                {
                    int error = Marshal.GetLastWin32Error();
                    if (error == 0) // pipe does not exist
                        return true;
                    else if (error == 2) // win32 error code for file not found
                        return true;
                    // all other errors indicate other issues
                }
                return false;
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Verbose, 1, "NamedPipeDoesNotExist error: " + ex.Message);
                return true; // assume it exists
            }
        }

        /// <summary>
        /// Checks the ip valid.
        /// </summary>
        /// <param name="ip">The ip.</param>
        /// <returns></returns>
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

        public bool checkToDisableIPC() //set this function equal to IPCDisabled property
        {
            try
            {
                Application.Current.ShutdownMode = ShutdownMode.OnMainWindowClose;

                System.Diagnostics.Process[] process = System.Diagnostics.Process.GetProcessesByName("ThorSync");

                System.Diagnostics.Process myprocess = System.Diagnostics.Process.GetCurrentProcess();

                foreach (System.Diagnostics.Process p in process)
                {
                    StringBuilder sb = new StringBuilder();

                    sb.AppendFormat("Process id {0} MyProcess id {1}", p.Id, myprocess.Id);
                    if (false == p.Id.Equals(myprocess.Id))
                    {
                        //disable IPC for ThorSync
                        return true;
                    }

                }
            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message);
            }
            return false;
        }

        /// <summary>
        /// Connects IPC.
        /// </summary>
        public void Connect()
        {
            SendToClient(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.Establish), GetHostName());
        }

        /// <summary>
        /// Disconnects IPC.
        /// </summary>
        public void Disconnect()
        {
            SendToClient(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.TearDown));
        }




        /// <summary>
        /// Excutes the namedpipe data.
        /// </summary>
        /// <param name="msg">The MSG.</param>
        /// <param name="ss">The ss.</param>
        /// <returns></returns>
        public bool ExcuteNamedPipeData(String[] msg, StreamString ss, bool isAcknowledgment)
        {
            
            
            
            _receiveIPCCommandActive = true;
            if (msg.Length == 4)
            {
                switch ((ThorPipeCommand)(Enum.Parse(typeof(ThorPipeCommand), msg[2])))
                {
                    case ThorPipeCommand.Establish:
                        {
                            ThorImageLSConnectionStats = true;
                        };
                        break;
                    case ThorPipeCommand.TearDown:
                        {
                            ThorImageLSConnectionStats = false;
                        };
                        break;
                    case ThorPipeCommand.StartAcquiring:
                        {
                            if (IsCapturing == false && (IsSaving || forceIPC)) //Is capturing
                            {
                                this.SavePath = System.IO.Path.GetDirectoryName(msg[3]);
                                if (String.IsNullOrEmpty(this.SaveName) || String.IsNullOrWhiteSpace(this.SaveName))
                                {
                                    this.SaveName = "SyncData_0000";
                                }
                                if (!Directory.Exists(SavePath))
                                {
                                    Directory.CreateDirectory(SavePath);
                                }
                                if (Directory.Exists(new StringBuilder(SavePath + "\\" + SaveName).ToString()))
                                {
                                    do
                                    {
                                        this.SaveName = CreateUniqueFilename(SavePath, this.SaveName);
                                    }
                                    while (Directory.Exists(SavePath + "\\" + this.SaveName));
                                }
                                Application.Current.Dispatcher.Invoke((Action)(() =>
                                {
                                    ChartMode = (int)ChartModes.CAPTURE;
                                    StartCapturing(forceIPC);
                                }));
                            }
                        };
                        break;
                    case ThorPipeCommand.StopAcquiring:
                        {
                            Application.Current.Dispatcher.Invoke((Action)(() =>
                            {
                                StopCapturing(forceIPC, false);//second parameter is false because we don't wan't to send the same command we just recieved
                            }));
                        };
                        break;
                    case ThorPipeCommand.AcquireInformation:
                        {
                            Application.Current.Dispatcher.Invoke((Action)(() =>
                            {
                                SendThorSyncConfiguration();
                            }));
                        };
                        break;
                    case ThorPipeCommand.ShowMostRecent:
                        if(false == ThorImageLSConnectionStats) //dont show most recent if not connected
                        {
                            break;
                        }
                        
                        Application.Current.Dispatcher.Invoke((Action)(() =>
                        {
                            ChartMode = (int)ChartModes.REVIEW;
                            FilePath = msg[3];
                            LoadDataFile();
                        }));

                        string newStr = msg[3]; 

                        if (newStr.Contains("\\\\"))
                        {
                            string[] split = newStr.Split(new string[] { "\\\\" }, StringSplitOptions.None);

                            string acc = "";

                            for(int x = 0; x < split.Length; x++)
                            {
                                acc += split[x];
                                if (x != split.Length - 1)
                                {
                                    acc += "\\";
                                }

                            }

                            newStr = acc;
                        }

                        XmlDocument doc = new XmlDocument();
                        try 
                        {

                            doc.Load(newStr);
                            XmlNodeList loadData = doc.SelectNodes("ThorImageExperiment/LSM");

                            int avgMode = int.Parse(loadData.Item(0).Attributes.GetNamedItem("averageMode").Value);
                            int avgNum = int.Parse(loadData.Item(0).Attributes.GetNamedItem("averageNum").Value);

                            Application.Current.Dispatcher.Invoke((Action)(() =>
                            {
                                if (1 == avgMode)
                                {
                                    Average = avgNum;
                                }
                                else
                                {
                                    Average = 1;  //No Averaging
                                }
                            }));
                        } catch (Exception ex)
                        {
                            ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, GetType().Name + " Could not load h5 settings file: " + newStr + "\n" + ex.Message);
                        }

                        break;
                    case ThorPipeCommand.SyncFrame:
                        if (false == ThorImageLSConnectionStats) //dont sync the frame if there is no connection
                        {
                            break;
                        }
                        int frameNum = -1;

                        string[] payload = msg[3].Split('/');

                        int TVal = int.Parse(payload[0]);

                        int ZVal = int.Parse(payload[1]);

                        int ZMax = int.Parse(payload[2]);

                        frameNum = ZVal + ((TVal-1) * ZMax);

                        Application.Current.Dispatcher.Invoke((Action)(() =>
                        {
                            CustomModifiers.ChartCanvasModifier.UpdateFrameCursorXAction(frameNum); 

                        }));

                        break;
                    default:
                        _receiveIPCCommandActive = false;
                        return false;
                }
                _receiveIPCCommandActive = false;
                return true;
            }
            else
            {
                _receiveIPCCommandActive = false;
                return false;
            }
        }

        /// <summary>
        /// Gets the name of the host.
        /// </summary>
        /// <returns></returns>
        public string GetHostName()
        {
            return (System.Environment.MachineName);
        }

        /// <summary>
        /// Initializes the ipc.
        /// </summary>
        public void InitIPC()
        {
            _remotePCHostName = GetHostName();
            InitIPCConnectionTimer();
            LoadRemotePCHostNameFromXML();
            _connectionClientID = GetHostName() + "ThorSyncPipe";
            if (_selectedRemotePCNameIndex >= 0)
            {
                if (IDMode == 0)
                {
                    if (_selectRemotePCName[_selectedRemotePCNameIndex] != "")
                    {
                        RemotePCHostName = _selectRemotePCName[_selectedRemotePCNameIndex];

                    }
                    else
                    {
                        RemotePCHostName = GetHostName();
                    }
                }
                else
                {
                    if (_selectRemodePCIPAddr[_selectedRemotePCNameIndex] != "")
                    {
                        RemotePCHostName = _selectRemodePCIPAddr[_selectedRemotePCNameIndex];

                    }
                    else
                    {
                        RemotePCHostName = LocalPCIPv4;
                    }
                }
            }
        }

        /// <summary>
        /// Initializes the ipc connection timer.
        /// </summary>
        public void InitIPCConnectionTimer()
        {
            _connectionTimer = new System.Timers.Timer();
            _connectionTimer.AutoReset = false;
            _connectionTimer.Interval = 1000;
            _connectionTimer.Elapsed += timer_Elapsed;
        }

        /// <summary>
        /// Loads the remote pc host name from XML.
        /// </summary>
        public void LoadRemotePCHostNameFromXML()
        {
            XmlDocument doc = new XmlDocument();

            if (false == File.Exists(Constants.ThorRealTimeData.SETTINGS_FILE_NAME))
                return;
            doc.Load(Constants.ThorRealTimeData.SETTINGS_FILE_NAME);
            string strTemp = string.Empty;
            XmlNodeList nodeList = doc.SelectNodes("RealTimeDataSettings/UserSettings/IPCRemoteHostPCName");
            if (nodeList.Count != 0)
            {
                foreach (XmlNode node in nodeList)
                {
                    if (GetAttribute(node, doc, "name", ref strTemp))
                    {
                        string[] hostname = strTemp.Split('/');
                        for (int i = 0; i < hostname.Length; i++)
                        {
                            _selectRemotePCName[i] = hostname[i];
                        }
                    }
                    if (GetAttribute(node, doc, "IP", ref strTemp))
                    {
                        string[] ipAddr = strTemp.Split('/');
                        for (int i = 0; i < ipAddr.Length; i++)
                        {
                            _selectRemodePCIPAddr[i] = ipAddr[i];
                        }
                    }
                    if (GetAttribute(node, doc, "IDMode", ref strTemp))
                    {
                        _IDMode = Convert.ToInt32(strTemp);
                        OnPropertyChanged("IDMode");
                    }
                    if (GetAttribute(node, doc, "activeIndex", ref strTemp))
                    {
                        SelectedRemotePCNameIndex = Convert.ToInt32(strTemp);
                    }
                }
            }
            else
            {
                _selectRemotePCName[0] = GetHostName();
                _selectRemodePCIPAddr[0] = LocalPCIPv4;
                SaveRemotePCHostNameToXML();
            }
        }

        /// <summary>
        /// Receives the ipc command.
        /// </summary>
        /// <param name="thorImagePipeRecv">The thor image pipe recv.</param>
        /// <param name="ss">The ss.</param>
        public void ReceiveIPCCommand(String thorImagePipeRecv, StreamString ss)
        {
            if (thorImagePipeRecv.Contains("~"))
            {
                String[] msgRecv = thorImagePipeRecv.Split('~');
                if (VerifyNamedPipeRouting(msgRecv))
                {
                    if (ExcuteNamedPipeData(msgRecv, ss, true))
                    {
                        ss.WriteString(String.Join("~", new String[]{Enum.GetName(typeof(ThorPipeSrc), ThorPipeSrc.Remote), Enum.GetName(typeof(ThorPipeDst), ThorPipeDst.Local),
                                           msgRecv[2],  msgRecv[3]}));
                    }
                    else
                    {
                        ss.WriteString(String.Join("~", new String[]{Enum.GetName(typeof(ThorPipeSrc), ThorPipeSrc.Remote), Enum.GetName(typeof(ThorPipeDst), ThorPipeDst.Local),
                                   Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.Error), "2"}));
                    }
                }
                else
                {
                    ss.WriteString(String.Join("~", new String[]{Enum.GetName(typeof(ThorPipeSrc), ThorPipeSrc.Remote), Enum.GetName(typeof(ThorPipeDst), ThorPipeDst.Local),
                                       Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.Error), "11"}));
                }
            }
            else
            {
                ss.WriteString(String.Join("~", new String[]{Enum.GetName(typeof(ThorPipeSrc), ThorPipeSrc.Remote), Enum.GetName(typeof(ThorPipeDst), ThorPipeDst.Local),
                                   Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.Error), "3"}));
            }
        }

        /// <summary>
        /// Receives the ipc ack message.
        /// </summary>
        /// <param name="msg">The MSG.</param>
        public void ReceiveIPCMessageACK(string msg)
        {
            if (msg.Contains("~"))
            {
                String[] msgRecv = msg.Split('~');
                if (msgRecv.Length == 4)
                {
                    if (VerifyNamedPipeRouting(msgRecv))
                    {
                        if (msgRecv[2] == _sendBuffer[2] && msgRecv[3] == "1")
                        {

                        }
                        else
                        {
                            switch ((ThorPipeStatus)(Convert.ToInt32(msgRecv[3])))
                            {
                                case ThorPipeStatus.ThorPipeStsNoError:
                                    break;
                                case ThorPipeStatus.ThorPipeStsBusy:
                                    break;
                                case ThorPipeStatus.ThorPipeStsBlankCommandError:
                                    break;
                                case ThorPipeStatus.ThorPipeStreamNotSupportedError:
                                    break;
                                case ThorPipeStatus.ThorPipeFormatError:
                                    break;
                                case ThorPipeStatus.ThorPipeFormatRoutingError:
                                    break;
                                case ThorPipeStatus.ThorpipeIOError:
                                    break;
                                case ThorPipeStatus.ThorPipeError:
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Saves the remote pc host name to XML.
        /// </summary>
        public void SaveRemotePCHostNameToXML()
        {
            string remotePCHostNameList = String.Join("/", _selectRemotePCName);
            string remotePCIPAddressList = String.Join("/", _selectRemodePCIPAddr);
            XmlDocument doc = new XmlDocument();
            if (false == File.Exists(Constants.ThorRealTimeData.SETTINGS_FILE_NAME))
                return;
            doc.Load(Constants.ThorRealTimeData.SETTINGS_FILE_NAME);
            var root = doc.DocumentElement;//Get to the root node
            XmlElement node = (XmlElement)doc.SelectSingleNode("RealTimeDataSettings/UserSettings/IPCRemoteHostPCName");
            if (node == null)
            {
                XmlElement elementRoot = doc.CreateElement(string.Empty, "IPCRemoteHostPCName", string.Empty);
                XmlElement rootNode = (XmlElement)doc.SelectSingleNode("RealTimeDataSettings/UserSettings");
                rootNode.AppendChild(elementRoot);
                node = (XmlElement)doc.SelectSingleNode("RealTimeDataSettings/UserSettings/IPCRemoteHostPCName");
            }
            node.SetAttribute("name", remotePCHostNameList.ToString());
            node.SetAttribute("IP", remotePCIPAddressList.ToString());
            node.SetAttribute("IDMode", IDMode.ToString());
            node.SetAttribute("activeIndex", SelectedRemotePCNameIndex.ToString());
            doc.Save(Constants.ThorRealTimeData.SETTINGS_FILE_NAME);
        }

        /// <summary>
        /// Sends the ThorSync configuration.
        /// </summary>
        public void SendThorSyncConfiguration()
        {
            SendToClient(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.UpdateInformation), string.Join("/", new string[] { (IsSaving || forceIPC).ToString(), TriggerMode.ToString() }));
        }


        /// <summary>
        /// Sends to client.
        /// </summary>
        /// <param name="command">The command.</param>
        /// <param name="data">The data.</param>
        public void SendToClient(String command, String data = "0")
        {
            String[] cmdAndData = {command, data };
            if(command == "SyncFrame")
                syncCMDCount++;

            lock (sendCmdBufferLock)
            {
                sendCmdBuffer.Enqueue(cmdAndData); //add the cmd to queue


                
                if (_sendServerRunning == true) //if the _serverThread is running the CMD was added to the queue and will be output from that thread
                {
                    return;
                }
                

                _sendServerRunning = true;
                //default case, need to restart or initalize _serverThread
                _serverThread = new Thread(sendCMD);
                _serverThread.Start();
            }
            
        }

        public void sendCMD()
        {
            do
            {
                bool dequeueFailed = false;
                string[] currentCMD = { "", "" };

                lock (sendCmdBufferLock)
                {
                    int count = sendCmdBuffer.Count;
                    if (3 < count && syncCMDCount == count) //case for SyncFrame cmds in queue
                    {
                        for (int x = 0; x < count - 1; x ++)
                        {
                            sendCmdBuffer.Dequeue(); //remove all but the latest
                        }
                        syncCMDCount = 0;
                        currentCMD = sendCmdBuffer.Dequeue();
                    }
                    else if (syncCMDCount == count) { //case for SyncFrame cmds in queue
                        currentCMD = sendCmdBuffer.Dequeue();
                        syncCMDCount--;
                    }
                    else if (0 < count) // if the queue has a cmd store it in local var regular case, there could be a SyncFrame cmd potentially
                    {
                        currentCMD = sendCmdBuffer.Dequeue();
                        if (currentCMD[0] == "SyncFrame")
                            syncCMDCount--;
                    }
                    else
                    {
                        dequeueFailed = true;
                    }
                }

                string command = currentCMD[0];
                string data = currentCMD[1];

                if (false == dequeueFailed) //a cmd was succesfully dequeued earlier
                {
                    _sendBuffer = new string[] { Enum.GetName(typeof(ThorPipeSrc), ThorPipeSrc.Remote), Enum.GetName(typeof(ThorPipeDst), ThorPipeDst.Local), command, data };
                    ServerThread(); //this function is no longer its own thread but the sendCMD function is now the new Thread
                }
                

                lock (sendCmdBufferLock)
                {
                    if (sendCmdBuffer.Count == 0)
                    {

                        _sendServerRunning = false;
                        return;

                    }
                }
            } while (true); //not great but there is an exit condition, this is for being able to queue calling threads and not starting another
        }

        /// <summary>
        /// Starts the aquiring.
        /// </summary>
        /// <param name="filePath">The file path.</param>
        public void StartAquiring(string filePath)
        {
            SendToClient(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.StartAcquiring), filePath);
        }

        /// <summary>
        /// Starts the namedpipe client.
        /// </summary>
        public void StartNamedPipeClient()
        {
            if (_pipeClient == null)
            {
                _pipeClient = new Thread(ClientThread);
            }
            _pipeClient.Start();
        }

        /// <summary>
        /// Stops the aquiring.
        /// </summary>
        public void StopAquiring()
        {
            SendToClient(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.StopAcquiring));
        }

        /// <summary>
        /// Stops the namedpipe client.
        /// </summary>
        public void StopNamedPipeClient()
        {
            if (_pipeClient != null)
            {
                _pipeClient.Abort();
                _pipeClient = null;
            }
        }

        private void createDisconnectBackgroundBox()
        {

            _disconnectedPopup = true;
            // Start the time-consuming operation.
            if (_sendBuffer[2] != Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.TearDown))
            {
                //This MessageBoxOptions.DefaultDesktopOnly parameter forces the popup box to the front
                MessageBox.Show("ThorImageLS is Disconnected","Connection Error",MessageBoxButton.OK,MessageBoxImage.Error,MessageBoxResult.OK,MessageBoxOptions.DefaultDesktopOnly);
                
            }
            _disconnectedPopup = false;

        }

        /// <summary>
        /// Send Message Out
        /// </summary>
        public void StreamOutNamedPipe()
        {
            string msgRecv = string.Empty;

            try
            {
                _pipeServer = new NamedPipeServerStream(_connectionServerID, PipeDirection.InOut, 4, PipeTransmissionMode.Byte, PipeOptions.Asynchronous);
                _connectionTimer.Start();
                _pipeServer.WaitForConnection();
                _connectionTimer.Stop();
            }
            catch (IOException)// release the pipe resource while it's connecting, throw the IOEception
            {
                _thorImageLSConnectionStats = false;
                OnPropertyChanged("ThorImageLSConnectionStats");
                if(_disconnectedPopup == false)
                {
                    new Thread(() => createDisconnectBackgroundBox()) { IsBackground = true }.Start();
                }
                
                    
                return;
            }
            catch (Exception)
            {
                return;
            }
            // Send Message
            try
            {
                // Read the request from the client. Once the client has
                // written to the pipe its security token will be available.
                _thorImageLSConnectionStats = true;
                StreamString ss = new StreamString(_pipeServer);
                // Verify our identity to the connected client using a
                // string that the client anticipates.
                ss.WriteString(String.Join("~", _sendBuffer));
                msgRecv = ss.ReadString();
                ReceiveIPCMessageACK(msgRecv);
            }
            // Catch the IOException that is raised if the pipe is broken
            // or disconnected.
            catch (Exception)
            {
            }
            _pipeServer.Close();
        }

        /// <summary>
        /// Verifies the named pipe routing.
        /// </summary>
        /// <param name="msg">The MSG.</param>
        /// <returns></returns>
        public bool VerifyNamedPipeRouting(String[] msg)
        {
            if (msg.Length == DataLength && msg[0] == Enum.GetName(typeof(ThorPipeSrc), ThorPipeSrc.Remote)
                && msg[1] == Enum.GetName(typeof(ThorPipeDst), ThorPipeDst.Local))
            {
                return true;
            }
            return false;
        }

        [return: MarshalAs(UnmanagedType.Bool)]
        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern bool WaitNamedPipe(string name, int timeout);

        /// <summary>
        /// Clients the thread.
        /// </summary>
        /// <param name="data">The data.</param>
        private void ClientThread(object data)
        {
            while (true)//endless loop
            {
                // New Server NamedPipeClientStream Instance
                NamedPipeClientStream _namedPipeClient;
                if (RemotePCHostName == GetHostName())
                {
                    _namedPipeClient = new NamedPipeClientStream(".", _connectionClientID, PipeDirection.InOut, PipeOptions.None, TokenImpersonationLevel.Impersonation);
                }
                else
                {
                    _namedPipeClient = new NamedPipeClientStream(RemotePCHostName, _connectionClientID, PipeDirection.InOut, PipeOptions.None, TokenImpersonationLevel.Impersonation);
                }

                if (NamedPipeDoesNotExist(_connectionClientID, RemotePCHostName))
                {
                    //sleep to lessen CPU load
                    System.Threading.Thread.Sleep(20);
                    continue;
                }
                // Wait for a Server to connect
                try
                {
                    _namedPipeClient.Connect();
                    //if (ChartMode == (int)ChartModes.CAPTURE) // make sure in the capture mode           <--This if was removed for the framesync feature -Ryan
                    //{
                        // Read the request from the Server. Once the Server has
                        // written to the pipe its security token will be available                         
                        StreamString ss = new StreamString(_namedPipeClient);
                        string msg = ss.ReadString();
                        ReceiveIPCCommand(msg, ss);
                    //} 
                }
                catch (InvalidOperationException)
                {

                }
                catch (IOException)
                {

                }
                catch (Exception)
                {

                }
                finally
                {
                    _namedPipeClient.Close();
                }
            }
        }

        /// <summary>
        /// Servers the thread.
        /// </summary>
        private void ServerThread()
        {
            StreamOutNamedPipe();
            if (_thorImageLSConnectionStats == true)
            {
                switch ((ThorPipeCommand)(Enum.Parse(typeof(ThorPipeCommand), _sendBuffer[2])))
                {
                    case ThorPipeCommand.Establish:
                        {
                            _thorImageLSConnectionStats = true;
                            OnPropertyChanged("ThorImageLSConnectionStats");
                            Thread.Sleep(50);
                            String[] configurarionInformation = { (IsSaving || forceIPC).ToString(), TriggerMode.ToString() };
                            _sendBuffer = new string[] { Enum.GetName(typeof(ThorPipeSrc),ThorPipeSrc.Remote),  Enum.GetName(typeof(ThorPipeDst),ThorPipeDst.Local),
                                        Enum.GetName(typeof(ThorPipeCommand),ThorPipeCommand.UpdateInformation), string.Join("/",configurarionInformation) };
                            StreamOutNamedPipe();
                        }
                        break;
                    case ThorPipeCommand.TearDown:
                        {
                            _thorImageLSConnectionStats = false;
                            OnPropertyChanged("ThorImageLSConnectionStats");
                        }
                        break;
                    case ThorPipeCommand.FilePath:
                        //_thorImageLSConnectionStats = false;
                        OnPropertyChanged("ThorImageLSConnectionStats");
                        Thread.Sleep(50);
                        _sendBuffer = new string[] { Enum.GetName(typeof(ThorPipeSrc),ThorPipeSrc.Remote),  Enum.GetName(typeof(ThorPipeDst),ThorPipeDst.Local),
                                        Enum.GetName(typeof(ThorPipeCommand),ThorPipeCommand.FilePath), _sendBuffer[3] };
                        StreamOutNamedPipe();
                        break;
                    case ThorPipeCommand.IsSaving:
                        _sendBuffer = new string[] { Enum.GetName(typeof(ThorPipeSrc),ThorPipeSrc.Remote),  Enum.GetName(typeof(ThorPipeDst),ThorPipeDst.Local),
                                        Enum.GetName(typeof(ThorPipeCommand),ThorPipeCommand.IsSaving), _sendBuffer[3] };
                        StreamOutNamedPipe();
                        break;
                    default:
                        break;
                }
            }
        }

        /// <summary>
        /// Handles the Elapsed event of the timer control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="ElapsedEventArgs"/> instance containing the event data.</param>
        void timer_Elapsed(object sender, ElapsedEventArgs e)
        {
            System.Timers.Timer timer = (System.Timers.Timer)sender; // Get the timer that fired the event
            timer.Stop(); // Stop the timer that fired the event
            _pipeServer.Close();
        }

        #endregion Methods
    }

    /// <summary>
    ///  Defines the data protocol for reading and writing strings on our stream
    /// </summary>
    public class StreamString
    {
        #region Fields

        private Stream ioStream;
        private UnicodeEncoding streamEncoding;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="StreamString"/> class.
        /// </summary>
        /// <param name="ioStream">The io stream.</param>
        public StreamString(Stream ioStream)
        {
            this.ioStream = ioStream;
            streamEncoding = new UnicodeEncoding();
        }

        #endregion Constructors

        #region Methods

        /// <summary>
        /// Reads the string.
        /// </summary>
        /// <returns></returns>
        public string ReadString()
        {
            int len;
            len = ioStream.ReadByte() * 256;
            len += ioStream.ReadByte();
            byte[] inBuffer = new byte[len];
            ioStream.Read(inBuffer, 0, len);

            return streamEncoding.GetString(inBuffer);
        }

        /// <summary>
        /// Writes the string.
        /// </summary>
        /// <param name="outString">The out string.</param>
        /// <returns></returns>
        public int WriteString(string outString)
        {
            byte[] outBuffer = streamEncoding.GetBytes(outString);
            int len = outBuffer.Length;
            if (len > UInt16.MaxValue)
            {
                len = (int)UInt16.MaxValue;
            }
            ioStream.WriteByte((byte)(len / 256));
            ioStream.WriteByte((byte)(len & 255));
            ioStream.Write(outBuffer, 0, len);
            ioStream.Flush();

            return outBuffer.Length + 2;
        }

        #endregion Methods
    }
}