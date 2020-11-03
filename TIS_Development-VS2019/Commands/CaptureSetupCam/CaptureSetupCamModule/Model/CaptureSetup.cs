namespace CaptureSetupDll.Model
{
    using System;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Timers;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;
    using System.Xml.Serialization;

    using ThorLogging;

    public class CaptureSetup
    {
        #region Fields

        private Guid _commandGuid;
        private String _expPath;

        #endregion Fields

        #region Constructors

        public CaptureSetup()
        {
            try
            {
                GetCommandGUID(ref _commandGuid);

                CaptureSetupSetupCommand();

                //GetCustomParamsBinary(ref _CaptureSetupParams);
            }
            catch (System.DllNotFoundException)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " DllNotFoundException");
            }

            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetActiveExperimentPathAndName(sb, PATH_LENGTH);

            _expPath = sb.ToString();

            //choose the 96well template if the active experiment is empty
            if (_expPath.Length == 0)
            {
                string templatesFolder = Application.Current.Resources["TemplatesFolder"].ToString();

                _expPath = templatesFolder + "\\Active.xml";

                //update the active experiment path
                CaptureSetupCustomParams escParams;

                escParams.version = 1.0;

                escParams.path = _expPath;

                SetCustomParamsBinary(ref escParams);
                CaptureSetupExecute();
            }

             ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Created");
        }

        #endregion Constructors

        #region Properties

        public Guid CommandGuid
        {
            get { return _commandGuid; }
        }

        public String ExpPath
        {
            get
            {
                return _expPath;
            }

            set
            {
                _expPath = value;

                CaptureSetupCustomParams escParams;

                escParams.version = 1.0;

                escParams.path = _expPath;

                SetCustomParamsBinary(ref escParams);
                CaptureSetupExecute();
            }
        }

        #endregion Properties

        #region Methods

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "Execute")]
        private static extern int CaptureSetupExecute();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetupCommand")]
        private static extern int CaptureSetupSetupCommand();

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetActiveExperimentPathAndName", CharSet = CharSet.Unicode)]
        private static extern int GetActiveExperimentPathAndName(StringBuilder path, int length);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetCommandGUID")]
        private static extern int GetCommandGUID(ref Guid guid);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetCustomParamsBinary")]
        private static extern int GetCustomParamsBinary(ref CaptureSetupCustomParams lidParams);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetCustomParamsBinary")]
        private static extern int SetCustomParamsBinary(ref CaptureSetupCustomParams lidParams);

        #endregion Methods

        #region Nested Types

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        struct CaptureSetupCustomParams
        {
            public double version;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
            public string path;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct DoubleParam
        {
            public double val;
            public int alias;
            public int useAlias;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct IntParam
        {
            public int val;
            public int alias;
            public int useAlias;
        }

        #endregion Nested Types
    }
}