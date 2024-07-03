namespace ThorIPCModules
{
    using System;
    using System.Diagnostics;

    using ThorLogging;

    public class ThorIPCThread
    {
        #region Fields

        private readonly int _dataLength = 4;

        #endregion Fields

        #region Enumerations

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
            return (Environment.MachineName);
        }

        public string GetSrc(String[] msg)
        {
            return msg[0];
        }

        public bool VerifyNamedPipeRouting(String[] msg, string src, string dst)
        {
            bool ret = false;
            if (msg.Length == _dataLength && GetSrc(msg) == src && GetDst(msg) == dst)
            {
                ret = true;
            }
            else
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "VerifyNamedPipeRouting error in IPCModules\\ThorIPCThread \nMessage Lenght: " + msg.Length + " Source: " + GetSrc(msg) + " Destination: " + GetDst(msg));
            }
            return ret;
        }

        #endregion Methods
    }
}