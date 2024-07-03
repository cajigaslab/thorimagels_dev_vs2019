namespace ThorIPCModules
{
    using System;
    using System.Collections;
    using System.Diagnostics;
    using System.IO;
    using System.IO.Pipes;
    using System.Threading;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// IPC Server sends messages from ThorImageLS to an external app 
    /// </summary>
    public partial class ThorIPCModule : ThorIPCThread
    {
        #region Fields

        private const int MAX_NUM_SERVERS = 4;
        private const int TIMER_INTERVAL = 1000;

        static readonly object _object = new object();

        System.Timers.Timer _connectionTimer;
        NamedPipeServerStream _pipeServer;
        string _sendBuffer = string.Empty;
        Queue _sendBufferQueue = new Queue();
        string _serverName = string.Empty;
        Thread _serverThread = null;

        #endregion Fields

        #region Methods

        /// <summary>
        /// Initializes the IPC connection timer.
        /// </summary>
        public void InitIPCConnectionTimer()
        {
            _connectionTimer = new System.Timers.Timer();
            _connectionTimer.AutoReset = false;
            _connectionTimer.Interval = TIMER_INTERVAL;
            _connectionTimer.Elapsed += _connectionTimer_Elapsed;
        }

        /// <summary>
        /// Receive and analyze the message, which comes from RemoteIPCControlMVM
        /// </summary>
        ///
        /// <param name="command">type of message</param>
        /// <param name="data">payload </param>
        ///
        /// <exception>NONE</exception>
        public void ReceiveUplinkCommand(ThorPipeCommand command, string data)
        {
            switch (command)
            {
                case ThorPipeCommand.Establish:
                    {
                        SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.Establish));
                        SetClientServerNames();
                        _sendBufferQueue.Clear();
                    }
                    break;
                case ThorPipeCommand.TearDown:
                    {
                        SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.TearDown));
                        SetClientServerNames();
                        _sendBufferQueue.Clear();
                    }
                    break;
                case ThorPipeCommand.AcquireInformation:
                    {
                        Thread.Sleep(50);
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
                case ThorPipeCommand.ShowMostRecent:
                    SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.ShowMostRecent), data);
                    break;
                case ThorPipeCommand.StartBleach:
                    break;
                case ThorPipeCommand.StopBleach:
                    break;
                case ThorPipeCommand.ChangeRemotePC:
                    SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.TearDown));
                    Thread.Sleep(100);
                    _remoteHostName = data;
                    SetClientServerNames();

                    RestartNamePipeClient();
                    break;
                case ThorPipeCommand.ChangeRemoteApp:
                    SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.TearDown));
                    Thread.Sleep(100);
                    _remoteAppName = data;
                    SetClientServerNames();

                    RestartNamePipeClient();
                    
                    break;
                case ThorPipeCommand.NotifySavedFile:
                    SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.NotifySavedFile), data);
                    break;
                case ThorPipeCommand.PositionReportX:
                    SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.PositionReportX), data);
                    break;
                case ThorPipeCommand.PositionReportY:
                    SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.PositionReportY), data);
                    break;
                case ThorPipeCommand.PositionReportZ:
                    SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.PositionReportZ), data);
                    break;
                case ThorPipeCommand.PositionReportSecondaryZ:
                    SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.PositionReportSecondaryZ), data);
                    break;
                case ThorPipeCommand.SyncFrame:
                    SendIPCCommand(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.SyncFrame), data);
                    break;
            }
        }

        /// <summary>
        /// Send IPC message from ThorImageLS to an external app
        /// </summary>
        ///
        /// <param name="command">type of message</param>
        /// <param name="data">payload </param>
        ///
        /// <exception>IOException</exception>
        public void SendIPCCommand(String command, String data = "0")
        {
            if (_serverThread != null)
            {
                _serverThread = null;
            }
            string[] stringBuffer = new string[]{Enum.GetName(typeof(ThorPipeSrc),ThorPipeSrc.Remote),//message source
                                          Enum.GetName(typeof(ThorPipeDst),ThorPipeDst.Local),//message destination
                                          command,//message command
                                          data};//message data
            //Queue any commands that are not Establish, AcquireInformation or TearDown
            if (command == "Establish" || command == "AcquireInformation" || command == "TearDown")
            {
                _sendBuffer = String.Join("~", stringBuffer);
            }
            else
            {
                _sendBufferQueue.Enqueue(String.Join("~", stringBuffer));
            }

            //ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Starting Thread for command: " + command + "data: " + data);
            _serverThread = new Thread(ServerThread);
            _serverThread.Start();
        }

        private void ServerThread()
        {
            lock (_object)
            {
                string msgRecv = string.Empty;
                _pipeServer = new NamedPipeServerStream(_serverName, PipeDirection.InOut, MAX_NUM_SERVERS, PipeTransmissionMode.Byte, PipeOptions.Asynchronous);
                // Wait for a client to connect
                try
                {
                    _connectionTimer.Start();
                    //ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Thread waiting for connection: " + String.Join("~", _sendBuffer));
                    _pipeServer.WaitForConnection();
                    _connectionTimer.Stop();
                }
                catch (IOException)// release the pipe resource while it's connecting, throw the IOEception
                {
                    MVMManager.Instance["RemoteIPCControlViewModelBase", "RemoteConnection"] = false;
                    return;
                }
                try
                {
                    //Store a local copy for the command string to send
                    string bufferToSend = (_sendBufferQueue.Count > 0) ? (string)_sendBufferQueue.Dequeue() : _sendBuffer;

                    // Read the request from the client. Once the client has
                    // written to the pipe its security token will be available.

                    StreamString ss = new StreamString(_pipeServer);
                    // Verify our identity to the connected client using a string that the client anticipates.
                    //ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Running Thread for buffer: " + bufferToSend);
                    ss.WriteString(bufferToSend);
                    //msgRecv = ss.ReadString();
                    //ReceiveIPCMessageACK(msgRecv);
                    _pipeServer.WaitForPipeDrain();
                }
                // Catch the IOException that is raised if the pipe is broken
                // or disconnected.
                catch (IOException e)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, e.Message);
                }
                _pipeServer.Close();
            }
        }

        void _connectionTimer_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            _pipeServer.Close();
            System.Timers.Timer timer = (System.Timers.Timer)sender; // Get the timer that fired the event
            timer.Stop(); // Stop the timer that fired the event
        }

        #endregion Methods

        #region Other

        ///TODO: Might be useful later to get an acknowledge from the server pipe
        ///// <summary>
        ///// Receives the ipc ack message.
        ///// </summary>
        ///// <param name="msg">The MSG.</param>
        //public void ReceiveIPCMessageACK(string msg)
        //{
        //    if (msg.Contains("~"))
        //    {
        //        String[] msgRecv = msg.Split('~');
        //        if (msgRecv.Length == 4)
        //        {
        //            if (VerifyNamedPipeRouting(msgRecv))
        //            {
        //                if (msgRecv[2] == _sendBuffer[2] && msgRecv[3] == "1")
        //                {
        //                }
        //                else
        //                {
        //                    switch ((ThorPipeStatus)(Convert.ToInt32(msgRecv[3])))
        //                    {
        //                        case ThorPipeStatus.ThorPipeStsNoError:
        //                            break;
        //                        case ThorPipeStatus.ThorPipeStsBusy:
        //                            break;
        //                        case ThorPipeStatus.ThorPipeStsBlankCommandError:
        //                            break;
        //                        case ThorPipeStatus.ThorPipeStreamNotSupportedError:
        //                            break;
        //                        case ThorPipeStatus.ThorPipeFormatError:
        //                            break;
        //                        case ThorPipeStatus.ThorPipeFormatRoutingError:
        //                            break;
        //                        case ThorPipeStatus.ThorpipeIOError:
        //                            break;
        //                        case ThorPipeStatus.ThorPipeError:
        //                            break;
        //                        default:
        //                            break;
        //                    }
        //                }
        //            }
        //        }
        //    }
        //}

        #endregion Other
    }
}