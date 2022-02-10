namespace ConsoleTest
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.IO.Pipes;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Security.Principal;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;

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

    class IpcConnection
    {
        #region Fields

        public readonly int DataLength = 4;

        public string FullSaveName;
        public string RemotePCHostName;
        public string _connectionClientID;
        public string _connectionServerID;

        Thread _pipeClient = null;
        NamedPipeServerStream _pipeServer;
        bool _receiveIPCCommandActive = false;
        string[] _sendBuffer = null;
        Thread _serverThread = null;
        bool _thorImageLSConnectionStats = false;
        bool _threadCompleted = true;

        #endregion Fields

        #region Methods

        public static bool NamedPipeDoesNotExist(string pipeName)
        {
            try
            {
                int timeout = 0;
                string normalizedPath = System.IO.Path.GetFullPath(
                 string.Format(@"\\.\pipe\{0}", pipeName));
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
                return true; // assume it exists
            }
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

                        };
                        break;
                    case ThorPipeCommand.TearDown:
                        {

                        };
                        break;
                    case ThorPipeCommand.StartAcquiring:
                        {

                        };
                        break;
                    case ThorPipeCommand.StopAcquiring:
                        {

                        };
                        break;
                    case ThorPipeCommand.AcquireInformation:
                        {

                        };
                        break;
                    case ThorPipeCommand.LoadExperimentFile:
                        {

                        };
                        break;
                    case ThorPipeCommand.MoveX:
                        {

                        };
                        break;
                    case ThorPipeCommand.MoveY:
                        {

                        };
                        break;
                    case ThorPipeCommand.MoveZ:
                        {

                        };
                        break;
                    case ThorPipeCommand.MoveSecondaryZ:
                        {

                        };
                        break;
                    case ThorPipeCommand.NotifySavedFile:
                        {
                            Console.WriteLine(msg[3]);
                        };
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
                                           msgRecv[2], "1"}));
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
        /// Sends to client.
        /// </summary>
        /// <param name="command">The command.</param>
        /// <param name="data">The data.</param>
        public void SendToClient(String command, String data = "0")
        {
            if (_threadCompleted == true)
            {
                if (_serverThread != null)
                {
                    _serverThread = null;
                }
                _sendBuffer = new string[] { Enum.GetName(typeof(ThorPipeSrc), ThorPipeSrc.Remote), Enum.GetName(typeof(ThorPipeDst), ThorPipeDst.Local), command, data };

                _serverThread = new Thread(ServerThread);
                _serverThread.Start();
            }
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

        /// <summary>
        /// Send Message Out
        /// </summary>
        public void StreamOutNamedPipe()
        {
            string msgRecv = string.Empty;

            try
            {
                _pipeServer = new NamedPipeServerStream(_connectionServerID, PipeDirection.InOut, 4, PipeTransmissionMode.Byte, PipeOptions.Asynchronous);

                _pipeServer.WaitForConnection();

            }
            catch (IOException)// release the pipe resource while it's connecting, throw the IOEception
            {
                _thorImageLSConnectionStats = false;

                if (_sendBuffer[2] != Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.TearDown))
                {
                    Console.WriteLine("The ThorImage is Disconnected. --Connection Error");
                }
                _threadCompleted = true;
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

                if (NamedPipeDoesNotExist(_connectionClientID))
                {
                    //sleep to lessen CPU load
                    System.Threading.Thread.Sleep(20);
                    continue;
                }

                // Wait for a Server to connect
                try
                {
                    _namedPipeClient.Connect();

                    // Read the request from the Server. Once the Server has
                    // written to the pipe its security token will be available
                    StreamString ss = new StreamString(_namedPipeClient);
                    string msg = ss.ReadString();
                    ReceiveIPCCommand(msg, ss);
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
            _threadCompleted = false;
            StreamOutNamedPipe();
            if (_thorImageLSConnectionStats == true)
            {
                switch ((ThorPipeCommand)(Enum.Parse(typeof(ThorPipeCommand), _sendBuffer[2])))
                {
                    case ThorPipeCommand.Establish:
                        {
                            _thorImageLSConnectionStats = true;

                            Thread.Sleep(50);
                            String[] configurarionInformation = { "true", "0" };
                            _sendBuffer = new string[] { Enum.GetName(typeof(ThorPipeSrc),ThorPipeSrc.Remote),  Enum.GetName(typeof(ThorPipeDst),ThorPipeDst.Local),
                                        Enum.GetName(typeof(ThorPipeCommand),ThorPipeCommand.UpdataInformation), string.Join("/",configurarionInformation) };
                            StreamOutNamedPipe();
                        }
                        break;
                    case ThorPipeCommand.TearDown:
                        {
                            _thorImageLSConnectionStats = false;

                        }
                        break;
                    default:
                        break;
                }
            }
            _threadCompleted = true;
        }

        #endregion Methods
    }

    class Program
    {
        #region Methods

        static void Main(string[] args)
        {
            var conn = new IpcConnection();
            string path = string.Empty;

            if (args.Length == 3)
            {
                conn.RemotePCHostName = args[1];
                conn.FullSaveName = args[2];

            }
            else if (args.Length == 2)
            {
                conn.RemotePCHostName = args[1]; // remote host name should be fed in command line.
                Console.WriteLine("Default save name used: C:\\temp\\exp01");
                conn.FullSaveName = "C:\\temp\\exp01";
            }
            else
            {
                Console.WriteLine("Default remote host name used: '.' ");
                conn.RemotePCHostName = ".";
                Console.WriteLine("Default save name used: C:\\temp\\exp01\\");
                conn.FullSaveName = "C:\\temp\\exp01\\";
            }
            //This is the name of the application you have to write in the remote setup, in ThorImage/Capture
            string applicationName = "ConsoleTest";
            conn._connectionServerID = applicationName + "ThorImagePipe";
            conn._connectionClientID = "ThorImage" + applicationName + "Pipe";
            conn.StartNamedPipeClient();
            conn.SendToClient(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.Establish), conn.GetHostName());

            Console.WriteLine("s - start acquisition.");
            Console.WriteLine("l - load Experiment File.");
            Console.WriteLine("c - change experiment path.");
            Console.WriteLine("x - stop acquisition.");
            Console.WriteLine("m - move stage.");
            Console.WriteLine("Esc - end application.");

            do
            {
                if (Console.ReadKey(true).Key == ConsoleKey.S)
                {
                    conn.SendToClient(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.StartAcquiring), conn.FullSaveName);
                    Console.WriteLine("Starting Experiment");
                }
                else if (Console.ReadKey(true).Key == ConsoleKey.L)
                {
                    Console.WriteLine("Enter the new experiment path:");
                    path = Console.ReadLine();
                    conn.SendToClient(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.LoadExperimentFile), path);
                    Console.WriteLine("Experiment Path was successfully loaded from " + path);
                }
                else if (Console.ReadKey(true).Key == ConsoleKey.C)
                {
                    Console.WriteLine("Enter the new experiment path:");
                    conn.FullSaveName = Console.ReadLine();
                    Console.WriteLine("Experiment Path was successfully changed to " + conn.FullSaveName);
                }
                else if (Console.ReadKey(true).Key == ConsoleKey.H)
                {
                    Console.WriteLine("Enter the new Local Computer Name:");
                    conn.RemotePCHostName = Console.ReadLine();
                    Console.WriteLine("PC Host Name was successfully changed to " + conn.RemotePCHostName);
                }
                else if (Console.ReadKey(true).Key == ConsoleKey.X)
                {
                    conn.SendToClient(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.StopAcquiring));
                    Console.WriteLine("Experiment Stopped");
                }
                else if (Console.ReadKey(true).Key == ConsoleKey.M)
                {
                    Console.WriteLine("Select the stage you want to move \n Options are: X, Y, Z, Secondary Z");
                    string optionSelected = Console.ReadLine();
                    if ("X" != optionSelected && "Y" != optionSelected && "Z" != optionSelected && "Secondary Z" != optionSelected)
                    {
                        Console.WriteLine("That stage doesn't exist");
                    }
                    else
                    {
                        Console.WriteLine("Set the distance you want to move by in micrometers: ");
                        string distanceUM = Console.ReadLine();
                        switch (optionSelected)
                        {
                            case ("X"):
                                conn.SendToClient(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.MoveX), distanceUM);
                                Console.WriteLine("Stage X was successfully moved by " + distanceUM);
                                break;
                            case ("Y"):
                                conn.SendToClient(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.MoveY), distanceUM);
                                Console.WriteLine("Stage Y was successfully moved by " + distanceUM);
                                break;
                            case ("Z"):
                                conn.SendToClient(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.MoveZ), distanceUM);
                                Console.WriteLine("Stage Z was successfully moved by " + distanceUM);
                                break;
                            case ("Secondary Z"):
                                conn.SendToClient(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.MoveSecondaryZ), distanceUM);
                                Console.WriteLine("Stage Secondary Z was successfully moved by " + distanceUM);
                                break;
                        }
                    }
                }
            } while (Console.ReadKey(true).Key != ConsoleKey.Escape);

            conn.SendToClient(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.TearDown));

            Console.WriteLine("Bye, see you next time.");
            System.Threading.Thread.Sleep(5000);
            Environment.Exit(0);
            return;
        }

        #endregion Methods
    }
}