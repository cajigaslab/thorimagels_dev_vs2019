namespace ThorIPCModules
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.IO.Pipes;
    using System.Linq;
    using System.Security.Principal;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorImageInfastructure;

    using ThorLogging;

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
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "IPCModules\\ThorIPCThread, IOStream ReadByte failed. Returning empty string. Exception thrown: " + ex.Message);
                return string.Empty;
            }
        }

        public int WriteString(string outString)
        {
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
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "IPCModules\\ThorIPCThread, IOStream WriteByte failed. Exception thrown: " + ex.Message);
            }

            return outBuffer.Length + 2;
        }

        #endregion Methods
    }

    public class ThorIPCThread
    {
        #region Fields

        public readonly int DataLength = 4;

        #endregion Fields

        #region Enumerations

        /// <summary>
        /// message command type
        /// </summary>
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
            ChangeRemoteApp
        }

        /// <summary>
        /// message destination type
        /// </summary>
        public enum ThorPipeDst
        {
            Local,
            Remote
        }

        /// <summary>
        /// message source type
        /// </summary>
        public enum ThorPipeSrc
        {
            Local,
            Remote
        }

        /// <summary>
        ///  Status
        /// </summary>
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

        /// <summary>
        /// thorsync mode
        /// </summary>
        public enum ThorSyncMode
        {
            FreeRun,
            HardwareTriggerSingle,
            HardwareTriggerRetriggerable,
            HardwareSynchronizable
        }

        #endregion Enumerations

        #region Methods

        public string GetCmd(String[] msg)
        {
            return msg[2];
        }

        public string[] GetData(string msg)
        {
            if (msg.Contains("~"))
            {
                return msg.Split('~');
            }
            return null;
        }

        public string GetData(String[] msg)
        {
            return msg[3];
        }

        public string GetDst(String[] msg)
        {
            return msg[1];
        }

        public string GetHostName()
        {
            return (System.Environment.MachineName);
        }

        public string GetSrc(String[] msg)
        {
            return msg[0];
        }

        public bool VerifyNamedPipeRouting(String[] msg, string src, string dst)
        {
            bool ret = false;
            if (msg.Length == DataLength && GetSrc(msg) == src && GetDst(msg) == dst)
            {
                ret = true;
            }
            return ret;
        }

        protected void sendData(IEventAggregator eventAggregator, String source, String destination, string commandType, string data = "")
        {
            IPCCommand command = new IPCCommand();
            command.Source = source;
            command.Destination = destination;
            command.CommandType = commandType;
            command.Data = data;
            //command published to change the status of the menu buttons in the Menu Control
            eventAggregator.GetEvent<CommandIPCEvent>().Publish(command);
        }

        #endregion Methods
    }
}