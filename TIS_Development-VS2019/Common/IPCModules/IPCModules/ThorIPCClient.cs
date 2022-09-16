namespace ThorIPCModules
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.IO.Pipes;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Security.Principal;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// IPC Client , which send message from ThroSync to ThorImageLS 
    /// </summary>
    public partial class ThorIPCModule : ThorIPCThread
    {
        #region Fields

        public bool _captureScriptManager;
        public string _thorSyncConfiguratureInformation = string.Empty;
        public bool _thorSyncConnection = false;
        public int _uncheckIndex;
        public bool _uncheckRemoteConnection;

        string _clientName = string.Empty;
        string _modeThorImage = string.Empty;
        Thread _pipeClient = null;
        string _remoteAppName = string.Empty;
        string _remoteHostName = string.Empty;

        #endregion Fields

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
                ex.ToString();
                return true; // assume it exists
            }
        }

        public void RestartNamePipeClient()
        {
            StopNamePipeClient();
            StartNamedPipeClient();
        }

        /// <summary>
        /// Start Client Endless-loop Thread, which waits and receive message from thorSync
        /// </summary>
        ///
        /// <param>NONE</param>
        ///
        /// <exception>NONE</exception>
        public void StartNamedPipeClient()
        {
            if (_pipeClient == null)
            {
                _pipeClient = new Thread(ClientThread);
            }
            _pipeClient.Start(); //Start thread
        }

        /// <summary>
        /// Stop Client Endless-loop Thread, which waits and receive message from thorSync
        /// </summary>
        ///
        /// <param>NONE</param>
        ///
        /// <exception>NONE</exception>
        public void StopNamePipeClient()
        {
            if (_pipeClient != null)
            {
                _pipeClient.Abort(); // Abort thread
                _pipeClient = null;
            }
        }

        [return: MarshalAs(UnmanagedType.Bool)]
        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern bool WaitNamedPipe(string name, int timeout);

        /// <summary>
        /// Client Thread, which waits and receive message from thorSync
        /// </summary>
        ///
        /// <param name="data"></param>
        ///
        /// <exception>Exception</exception>
        private void ClientThread(object data)
        {
            while (true)//endless loop
            {
                // New Server NamedPipeClientStream Instance
                 NamedPipeClientStream _namedPipeClient ;
                if (_remoteHostName == GetHostName())
                {
                    _namedPipeClient = new NamedPipeClientStream(".", _clientName, PipeDirection.InOut, PipeOptions.None, TokenImpersonationLevel.Impersonation);
                }
                else
                {
                    _namedPipeClient = new NamedPipeClientStream(_remoteHostName, _clientName, PipeDirection.InOut, PipeOptions.None, TokenImpersonationLevel.Impersonation);
                }

                if (NamedPipeDoesNotExist(_clientName, _remoteHostName))
                {
                    //sleep to lessen CPU load
                    System.Threading.Thread.Sleep(20);
                    continue;
                }
                // Wait for a Server to connect
                StreamString ss = new StreamString(_namedPipeClient);
                try
                {
                    _namedPipeClient.Connect();
                }
                catch (Exception)
                {
                    continue;
                }
                try
                {
                    // Read the request from the Server. Once the Server has
                    // written to the pipe its security token will be available
                    string msg = ss.ReadString();
                    if (false == ReceiveIPCCommand(msg, Enum.GetName(typeof(ThorPipeSrc), ThorPipeSrc.Remote), Enum.GetName(typeof(ThorPipeDst), ThorPipeDst.Local),ss))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "IPC Client Error");
                    }
                }
                catch (IOException)
                {
                    ss.WriteString(String.Join("~", new String[]{Enum.GetName(typeof(ThorPipeSrc), ThorPipeSrc.Remote), Enum.GetName(typeof(ThorPipeDst), ThorPipeDst.Local),
                        Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.Error), "20"}));
                }
                catch (ArgumentNullException)
                {
                    ss.WriteString(String.Join("~", new String[]{Enum.GetName(typeof(ThorPipeSrc), ThorPipeSrc.Remote), Enum.GetName(typeof(ThorPipeDst), ThorPipeDst.Local),
                        Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.Error), "2"}));
                }
                catch (NotSupportedException)
                {
                    ss.WriteString(String.Join("~", new String[]{Enum.GetName(typeof(ThorPipeSrc), ThorPipeSrc.Remote), Enum.GetName(typeof(ThorPipeDst), ThorPipeDst.Local),
                        Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.Error), "3"}));
                }
                catch (Exception)
                {
                    ss.WriteString(String.Join("~", new String[]{Enum.GetName(typeof(ThorPipeSrc), ThorPipeSrc.Remote), Enum.GetName(typeof(ThorPipeDst), ThorPipeDst.Local),
                        Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.Error), "99"}));
                }
                _namedPipeClient.Close();

                //sleep to lessen CPU load
                System.Threading.Thread.Sleep(2);

            }
        }

        /// <summary>
        /// Send IPC message from ThroSync to ThorImageLS 
        /// </summary>
        ///
        /// <param name="msg">message</param>
        /// <param name="src">information source, which is used to verify message source</param>
        /// <param name="dst">information destination, which is used to verify message destination</param>
        ///
        /// <exception>Exception</exception>
        private bool ReceiveIPCCommand(string msg, string src, string dst, StreamString ss)
        {
            string[] data = GetData(msg); // get splited data from msg
            if (data != null)
            {
                if (VerifyNamedPipeRouting(data, src, dst)) // verify data routing information
                {
                    bool ret = SendDownlinkCommand(data); // send information
                    ss.WriteString(String.Join("~", new String[]{Enum.GetName(typeof(ThorPipeSrc), ThorPipeSrc.Remote), Enum.GetName(typeof(ThorPipeDst), ThorPipeDst.Local),
                        GetCmd(data), "1"}));
                    return ret;
                }
                else
                {
                    ss.WriteString(String.Join("~", new String[]{Enum.GetName(typeof(ThorPipeSrc), ThorPipeSrc.Remote), Enum.GetName(typeof(ThorPipeDst), ThorPipeDst.Local),
                        Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.Error), "11"}));
                    return false;
                }
            }
            ss.WriteString(String.Join("~", new String[]{Enum.GetName(typeof(ThorPipeSrc), ThorPipeSrc.Remote), Enum.GetName(typeof(ThorPipeDst), ThorPipeDst.Local),
                Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.Error), "2"}));
            return false;
        }

        /// <summary>
        /// send message to Low-level modules ("Run Sample LS"~"Capture Setup"~"ImageReview")
        /// </summary>
        ///
        /// <param name="msg">payload </param>
        ///
        /// <exception>NONE</exception>
        private bool SendDownlinkCommand(string[] msg)
        {
            bool ret = false;
            ThorPipeCommand cmd = (ThorPipeCommand)(Enum.Parse(typeof(ThorPipeCommand), GetCmd(msg)));// Convert command from String type to Enumeration(ThorPipeCommand)
            switch (cmd)
            {
                case ThorPipeCommand.Establish:
                    if (ResourceManagerCS.Instance.TabletModeEnabled)
                    {
                        MVMManager.Instance["RemoteIPCControlViewModel", "RemoteConnection"] = true;
                        ret = true;
                    }
                    else if (_modeThorImage == "Run Sample LS")
                    {
                        sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "Establish");
                        ret = true;
                    }
                    //Resets the boolean for unchecking remote connections checkbox if checked in ThorSync within any of these tabs
                    else if (_modeThorImage == "ScriptManager" || _modeThorImage == "ImageReview" || _modeThorImage == "Capture Setup")
                    {
                        _uncheckRemoteConnection = false;
                    }
                    _thorSyncConnection = true;
                    break;
                case ThorPipeCommand.TearDown:
                    if (ResourceManagerCS.Instance.TabletModeEnabled)
                    {
                        MVMManager.Instance["RemoteIPCControlViewModel", "RemoteConnection"] = false;
                        ret = true;
                    }
                    else if (_modeThorImage == "Run Sample LS")
                    {
                        sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "TearDown");
                        ret = true;
                    }
                    _thorSyncConnection = false;
                    break;
                case ThorPipeCommand.UpdataInformation:
                    if (_modeThorImage == "Run Sample LS")
                    {
                        sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "UpdataInformation", msg[3]);
                        ret = true;
                    }
                    _thorSyncConfiguratureInformation = msg[3];
                    break;
                case ThorPipeCommand.StartAcquiring:
                    if (_modeThorImage == "Run Sample LS")
                    {
                        sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "StartAcquiring",msg[3]);
                        ret = true;
                    }
                    break;
                case ThorPipeCommand.StopAcquiring:
                    if (_modeThorImage == "Run Sample LS")
                    {
                        sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "StopAcquiring");
                        ret = true;
                    }
                    break;
                case ThorPipeCommand.StartBleach:
                    if (_modeThorImage == "Run Sample LS")
                    {
                        sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "StartBleach");
                        ret = true;
                    }
                    break;
                case ThorPipeCommand.StopBleach:
                    if (_modeThorImage == "Run Sample LS")
                    {
                        sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "StopBleach");
                        ret = true;
                    }
                    break;
                case ThorPipeCommand.LoadExperimentFile:
                    if (_modeThorImage == "Run Sample LS")
                    {
                        sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "LoadExperimentFile", msg[3]);
                        ret = true;
                    }
                    break;
                case ThorPipeCommand.MoveX:
                    if (_modeThorImage == "Run Sample LS")
                    {
                        sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "MoveX", msg[3]);
                        ret = true;
                    }
                    break;
                case ThorPipeCommand.MoveY:
                    if (_modeThorImage == "Run Sample LS")
                    {
                        sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "MoveY", msg[3]);
                        ret = true;
                    }
                    break;
                case ThorPipeCommand.MoveZ:
                    if (_modeThorImage == "Run Sample LS")
                    {
                        sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "MoveZ", msg[3]);
                        ret = true;
                    }
                    break;
                case ThorPipeCommand.MoveSecondaryZ:
                    if (_modeThorImage == "Run Sample LS")
                    {
                        sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "MoveSecondaryZ", msg[3]);
                        ret = true;
                    }
                    break;
                case ThorPipeCommand.AcquireInformation:
                    {
                        ret = true;
                    };
                    break;
                case ThorPipeCommand.NotifySavedFile:
                    if (_modeThorImage == "Run Sample LS")
                    {
                        sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "NotifySavedFile", msg[3]);
                        ret = true;
                    }
                    break;
                default:
                    break;
            }
            return ret;
        }

        #endregion Methods
    }
}