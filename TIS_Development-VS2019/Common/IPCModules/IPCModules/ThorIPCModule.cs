namespace ThorIPCModules
{
    using System;
    using System.Diagnostics;
    using System.IO;
    using System.Text;

    using ThorLogging;

    using ThorSharedTypes;

    // Defines the data protocol for reading and writing strings on our stream
    public class StreamString
    {
        #region Fields

        private const int DEFAULT_LENGTH = 256;

        private Stream ioStream;
        private UnicodeEncoding streamEncoding;

        #endregion Fields

        #region Constructors

        public StreamString(Stream ioStream)
        {
            this.ioStream = ioStream;
            streamEncoding = new UnicodeEncoding();
        }

        #endregion Constructors

        #region Methods

        public string ReadString()
        {
            int len;
            try
            {
                len = ioStream.ReadByte() * DEFAULT_LENGTH;
                len += ioStream.ReadByte();

                //len can become negative. Check before using.
                if (len < 1) len = DEFAULT_LENGTH;

                byte[] inBuffer = new byte[len];
                ioStream.Read(inBuffer, 0, len);
                return streamEncoding.GetString(inBuffer);
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "IPCModules\\ThorIPCModule StreamString class: IOStream ReadByte failed. Returning empty string. Exception thrown: " + ex.Message);
                return string.Empty;
            }
        }

        public int WriteString(string outString)
        {
            if (outString == null)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "IPCModules\\ThorIPCModule can't send null string over IPC");
                return -1;
            }

            byte[] outBuffer = streamEncoding.GetBytes(outString);
            int len = outBuffer.Length;
            if (len > UInt16.MaxValue)
            {
                len = (int)UInt16.MaxValue;
            }
            try
            {
                ioStream.WriteByte((byte)(len / DEFAULT_LENGTH));
                ioStream.WriteByte((byte)(len & 255));
                ioStream.Write(outBuffer, 0, len);
                ioStream.Flush();
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "IPCModules\\ThorIPCModule StreamString class: IOStream WriteByte failed. Exception thrown: " + ex.Message);
            }

            return outBuffer.Length + 2;
        }

        #endregion Methods
    }

    public partial class ThorIPCModule
    {
        #region Fields

        private static readonly ThorIPCModule _instance = new ThorIPCModule();

        #endregion Fields

        #region Constructors

        public ThorIPCModule()
        {
            InitIPCConnectionTimer();
            SetClientServerNames();

            _remoteHostName = GetHostName();
        }

        #endregion Constructors

        #region Properties

        public static ThorIPCModule Instance
        {
            get
            {
                return _instance;
            }
        }

        #endregion Properties

        #region Methods

        public void SetClientServerNames()
        {
            if (_remoteAppName.CompareTo("") == 0)
            {
                _serverName = "ThorImageThorSyncPipe";
                _clientName = "ThorSyncThorImagePipe";
            }
            else
            {
                _serverName = "ThorImage" + _remoteAppName + "Pipe";
                _clientName = _remoteAppName + "ThorImagePipe";
            }

            // In tablet mode, ThorImageLS in the tablet is the client and ThorImageLS in the user computer is the server
            if (ResourceManagerCS.Instance.TabletModeEnabled)
            {
                if (_remoteAppName.CompareTo("") == 0)
                {
                    _serverName = "ThorSyncThorImagePipe";
                    _clientName = "ThorImageThorSyncPipe";
                }
                else
                {
                    _serverName = _remoteAppName + "ThorImagePipe";
                    _clientName = "ThorImage" + _remoteAppName + "Pipe";
                }
            }
        }

        #endregion Methods
    }
}