namespace ThorIPCModules
{
    using System;
    using System.Diagnostics;
    using System.IO;
    using System.IO.Pipes;
    using System.Runtime.InteropServices;
    using System.Security.Principal;
    using System.Threading;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// IPC Client, reads message from external app 
    /// </summary>
    public partial class ThorIPCModule : ThorIPCThread
    {
        #region Fields

        private string _clientName = string.Empty;
        private Thread _pipeClient = null;
        private string _remoteAppName = string.Empty;
        private string _remoteHostName = string.Empty;

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
                    else if (error == 53) //0x35  ERROR_BAD_NETPATH
                        return true;

                    // all other errors indicate other issues


                }
                return false;
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "IPC Client Error, NamedPipeDoesNotExist. Exception: \n" + ex.ToString());
                return true; // assume it exists
            }
        }

        public void RestartNamePipeClient()
        {
            StopNamePipeClient();
            StartNamedPipeClient();
        }

        /// <summary>
        /// Start Client Endless-loop Thread, which waits and receive message from any external app
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
        /// Stop Client Endless-loop Thread, which waits and receive message from any external app
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
        /// Client Thread, which waits and receive message from any external app
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
                NamedPipeClientStream _namedPipeClient;
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
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "IPC Client Error, Couldn't connect to pipe " + _namedPipeClient);
                    continue;
                }
                try
                {
                    // Read the request from the Server. Once the Server has
                    // written to the pipe its security token will be available
                    string msg = ss.ReadString();
                    if (false == ReceiveIPCCommand(msg, Enum.GetName(typeof(ThorPipeSrc), ThorPipeSrc.Remote), Enum.GetName(typeof(ThorPipeDst), ThorPipeDst.Local), ss))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "IPC Client Error, ReceiveIPCCommand");
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
        /// Receive IPC message from External app
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
                    bool ret = SendDownCommand(data); // send information to ThorImageLS
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
        /// send message to RemoteIPCControlMVM
        /// </summary>
        ///
        /// <param name="msg">payload </param>
        ///
        /// <exception>NONE</exception>
        private bool SendDownCommand(string[] msg)
        {
            if (msg[2] == "FilePath")
            {
                MVMManager.Instance["RemoteIPCControlViewModelBase", "ReceivedCommandThroughIPC"] = (ThorPipeCommand.FilePath, msg[3]);
                return true;
            } 
            else if (msg[2] == "IsSaving")
            {
                MVMManager.Instance["RemoteIPCControlViewModelBase", "ReceivedCommandThroughIPC"] = (ThorPipeCommand.IsSaving, msg[3]);

            }
            ThorPipeCommand cmd = (msg[2] == "UpdateInformation") ? ThorPipeCommand.UpdateInformation : (ThorPipeCommand)(Enum.Parse(typeof(ThorPipeCommand), GetCmd(msg)));// Convert command from String type to Enumeration(ThorPipeCommand)
            MVMManager.Instance["RemoteIPCControlViewModelBase", "ReceivedCommandThroughIPC"] = (cmd, msg[3]);
            return true;
        }

        #endregion Methods
    }
}