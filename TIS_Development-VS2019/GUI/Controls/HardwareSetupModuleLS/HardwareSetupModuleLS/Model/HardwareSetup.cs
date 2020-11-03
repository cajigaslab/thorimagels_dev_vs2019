namespace HardwareSetupDll.ViewModel
{
    using System;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Timers;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;

    using ThorLogging;

    public class HardwareSetup : INotifyPropertyChanged
    {
        #region Fields

        private string _streamingPath;

        #endregion Fields

        #region Constructors

        public HardwareSetup()
        {
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Created");
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public string StreamingPath
        {
            get
            {
                return _streamingPath;
            }
            set
            {
                _streamingPath = value;
            }
        }

        #endregion Properties

        #region Methods

        protected virtual void OnPropertyChanged(string propertyName)
        {
            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        #endregion Methods
    }
}