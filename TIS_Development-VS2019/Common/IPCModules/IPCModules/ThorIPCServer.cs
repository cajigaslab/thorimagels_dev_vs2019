namespace ThorIPCModules
{
    using System;
    using System.Diagnostics;
    using System.IO;
    using System.IO.Pipes;
    using System.Threading;
    using System.Threading.Tasks;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// IPC Server , which send message from ThorImageLS to ThroSync
    /// </summary>
    public partial class ThorIPCModule : ThorIPCThread
    {
        #region Fields

        private const int MAX_NUM_SERVERS = 4;
        private const int TIMER_INTERVAL = 1000;

        System.Timers.Timer _connectionTimer;
        NamedPipeServerStream _pipeServer;
        string[] _sendBuffer = null;
        string _serverName = string.Empty;
        Thread _serverThread = null;
        bool _threadCompleted = true;

        #endregion Fields

        #region Methods

        /// <summary>
        /// Disconnect ThorSync
        /// </summary>
        public void DisconnectThorSync()
        {
            if (_thorSyncConnection == true)
                SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.TearDown));
        }

        /// <summary>
        /// Initializes the ipc connection timer.
        /// </summary>
        public void InitIPCConnectionTimer()
        {
            _connectionTimer = new System.Timers.Timer();
            _connectionTimer.AutoReset = false;
            _connectionTimer.Interval = TIMER_INTERVAL;
            _connectionTimer.Elapsed += _connectionTimer_Elapsed;
        }

        /// <summary>
        /// Receives the ipc message ack.
        /// </summary>
        /// <param name="msg">The MSG.</param>
        public void ReceiveIPCMessageACK(string msg)
        {
            if (msg.Contains("~"))
            {
                String[] msgRecv = msg.Split('~');
                if (msgRecv.Length == 4)
                {
                    if (VerifyNamedPipeRouting(msgRecv, Enum.GetName(typeof(ThorPipeSrc), ThorPipeSrc.Remote), Enum.GetName(typeof(ThorPipeDst), ThorPipeDst.Local)))
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
        /// Receive and analyze the message, which comes from Low-level modules ("Run Sample LS"~"Capture Setup"~"ImageReview")
        /// </summary>
        ///
        /// <param name="command">type of message</param>
        /// <param name="data">payload </param>
        ///
        /// <exception>NONE</exception>
        public void ReceiveUplinkCommand(string command, string data)
        {
            ThorPipeCommand cmd = (ThorPipeCommand)(Enum.Parse(typeof(ThorPipeCommand), command)); // Convert command from String type to Enumeration(ThorPipeCommand)
            switch (cmd)
            {
                case ThorPipeCommand.Establish:
                    {
                        SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.Establish));
                        _thorSyncConnection = true;
                        SetClientServerNames();
                    }
                    break;
                case ThorPipeCommand.TearDown:
                    {
                        SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.TearDown));
                        _thorSyncConnection = false;
                        SetClientServerNames();
                    }
                    break;
                case ThorPipeCommand.AcquireInformation:
                    {
                        Thread.Sleep(100);
                        SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.AcquireInformation));
                    }
                    break;
                case ThorPipeCommand.StartAcquiring:
                    {
                        SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.StartAcquiring), data);
                    }
                    break;
                case ThorPipeCommand.StopAcquiring:
                    {
                        SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.StopAcquiring));
                    }
                    break;
                case ThorPipeCommand.StartBleach:
                    break;
                case ThorPipeCommand.StopBleach:
                    break;
                case ThorPipeCommand.ChangeRemotePC:
                    SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.TearDown));
                    Thread.Sleep(100);
                    _thorSyncConnection = false;
                    _remoteHostName = data;
                    SetClientServerNames();

                    RestartNamePipeClient();
                    break;
                case ThorPipeCommand.ChangeRemoteApp:
                    SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.TearDown));
                    Thread.Sleep(100);
                    _thorSyncConnection = false;
                    _remoteAppName = data;
                    SetClientServerNames();

                    RestartNamePipeClient();
                    break;
                case ThorPipeCommand.NotifySavedFile:
                    {
                        SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.NotifySavedFile), data);
                    }
                    break;
                default:
                    break;
            }
        }

        /// <summary>
        /// Send IPC message from ThorImageLS to ThroSync
        /// </summary>
        ///
        /// <param name="command">type of message</param>
        /// <param name="data">payload </param>
        ///
        /// <exception>IOException</exception>
        public void SendIPCCommand(String command, String data = "0")
        {
            if (_threadCompleted == true)
            {
                if (_serverThread != null)
                {
                    _serverThread = null;
                }
                _sendBuffer = new string[]{Enum.GetName(typeof(ThorPipeSrc),ThorPipeSrc.Remote),//message source
                                          Enum.GetName(typeof(ThorPipeDst),ThorPipeDst.Local),//message destination
                                          command,//message command
                                          data};//message data
                _serverThread = new Thread(ServerThread);
                _serverThread.Start();
            }
        }

        public void StreamOutNamedPipe()
        {
            string msgRecv = string.Empty;
            _pipeServer = new NamedPipeServerStream(_serverName, PipeDirection.InOut, MAX_NUM_SERVERS, PipeTransmissionMode.Byte, PipeOptions.Asynchronous);
            // Wait for a client to connect
            try
            {
                _connectionTimer.Start();
                _pipeServer.WaitForConnection();
                _connectionTimer.Stop();
            }
            catch (IOException)// release the pipe resource while it's connecting, throw the IOEception
            {
                if (ResourceManagerCS.Instance.TabletModeEnabled)
                {
                    MVMManager.Instance["RemoteIPCControlViewModel", "RemoteConnection"] = false;
                }
                else
                {
                    sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "TearDown");
                }
                _thorSyncConnection = false;
                _threadCompleted = true;
                return;
            }
            try
            {
                // Read the request from the client. Once the client has
                // written to the pipe its security token will be available.

                StreamString ss = new StreamString(_pipeServer);
                // Verify our identity to the connected client using a
                // string that the client anticipates.
                ss.WriteString(String.Join("~", _sendBuffer));
                msgRecv = ss.ReadString();
                ReceiveIPCMessageACK(msgRecv);
                _thorSyncConnection = true;
            }
            // Catch the IOException that is raised if the pipe is broken
            // or disconnected.
            catch (IOException e)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, e.Message);
            }
            _pipeServer.Close();
        }

        private void ServerThread()
        {
            _threadCompleted = false;
            StreamOutNamedPipe();
            if (_thorSyncConnection == true)
            {
                switch ((ThorPipeCommand)(Enum.Parse(typeof(ThorPipeCommand), _sendBuffer[2])))
                {
                    case ThorPipeCommand.Establish:
                        {
                            Thread.Sleep(50);
                            if (ResourceManagerCS.Instance.TabletModeEnabled)
                            {
                                MVMManager.Instance["RemoteIPCControlViewModel", "RemoteConnection"] = true;
                            }
                            else
                            {
                                _sendBuffer = new string[] { Enum.GetName(typeof(ThorPipeSrc),ThorPipeSrc.Remote),  Enum.GetName(typeof(ThorPipeDst),ThorPipeDst.Local),
                                        Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.AcquireInformation),"0" };
                                sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "Establish");
                            }
                            StreamOutNamedPipe();
                        }
                        break;
                    case ThorPipeCommand.TearDown:
                        {
                            if (ResourceManagerCS.Instance.TabletModeEnabled)
                            {
                                MVMManager.Instance["RemoteIPCControlViewModel", "RemoteConnection"] = false;
                            }
                            else
                            {
                                sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "TearDown");
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
            _threadCompleted = true;
        }

        void _connectionTimer_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            _pipeServer.Close();
            System.Timers.Timer timer = (System.Timers.Timer)sender; // Get the timer that fired the event
            timer.Stop(); // Stop the timer that fired the event
        }

        #endregion Methods
    }
}