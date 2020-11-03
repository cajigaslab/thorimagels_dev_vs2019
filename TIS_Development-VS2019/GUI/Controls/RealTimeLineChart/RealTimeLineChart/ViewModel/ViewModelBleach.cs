namespace RealTimeLineChart.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Timers;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Threading;
    using System.Xml;

    using global::RealTimeLineChart.Model;
    using global::RealTimeLineChart.View;

    using ThorLogging;

    public partial class RealTimeLineChartViewModel : ViewModelBase
    {
        #region Fields

        private int _bleachCycle;
        private double _bleachCycleInterval;
        private double _bleachIdleTime;
        private int _bleachIteration;
        private double _bleachTime;
        private double _delayTime;
        private DispatcherTimer _readBleachTimer;
        private int _triggerBleachMode;

        #endregion Fields

        #region Properties

        /// <summary>
        /// Gets or sets the bleach cycle / Repeat times.
        /// </summary>
        /// <value>
        /// The bleach cycle.
        /// </value>
        public int BleachCycle
        {
            get
            {
                return _bleachCycle;
            }
            set
            {
                _bleachCycle = value;
                OnPropertyChanged("BleachCycle");
            }
        }

        /// <summary>
        /// Gets or sets the bleach cycle interval.
        /// </summary>
        /// <value>
        /// The bleach cycle interval.
        /// </value>
        // Idle Times between two bleachs
        public double BleachCycleInterval
        {
            get
            {
                return _bleachCycleInterval;
            }
            set
            {
                _bleachCycleInterval = value;
                OnPropertyChanged("BleachCycleInterval");
            }
        }

        /// <summary>
        /// Gets or sets the bleach idle time.
        /// </summary>
        /// <value>
        /// The bleach idle time.
        /// </value>
        public double BleachIdleTime
        {
            get
            {
                return _bleachIdleTime;
            }
            set
            {
                _bleachIdleTime = value;
                OnPropertyChanged("BleachIdleTime");
            }
        }

        /// <summary>
        /// Gets the bleaching icon.
        /// </summary>
        /// <value>
        /// The bleaching icon.
        /// </value>
        public string BleachingIcon
        {
            get
            {
                if (IsBleaching)
                {
                    return "/RealTimeLineChart;component/Icons/Stop.png";
                }
                else
                {
                    return "/RealTimeLineChart;component/Icons/Bleach2.png";
                }
            }
        }

        /// <summary>
        /// Gets or sets the bleach iteration.
        /// </summary>
        /// <value>
        /// The bleach iteration.
        /// </value>
        public int BleachIteration
        {
            get
            {
                return _bleachIteration;
            }
            set
            {
                _bleachIteration = value;
                OnPropertyChanged("BleachIteration");
            }
        }

        /// <summary>
        /// Gets or sets the bleach time.
        /// </summary>
        /// <value>
        /// The bleach time.
        /// </value>
        public double BleachTime
        {
            get
            {
                return _bleachTime;
            }
            set
            {
                _bleachTime = value;
                OnPropertyChanged("BleachTime");
            }
        }

        /// <summary>
        /// Gets or sets the delay time. Delay Time indicates the amount of time of delay after the bleaching event
        /// </summary>
        /// <value>
        /// The delay time.
        /// </value>
        public double DelayTime
        {
            get
            { return _delayTime; }
            set
            {
                if (value + _bleachTime > 0)
                {
                    _delayTime = value;
                    OnPropertyChanged("DelayTime");
                }
            }
        }

        /// <summary>
        /// Gets a value indicating whether this instance is bleaching.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance is bleaching; otherwise, <c>false</c>.
        /// </value>
        public bool IsBleaching
        {
            get
            {
                bool val = RealTimeDataCapture.Instance.IsAsyncAcquiring();
                if (false == val)
                {
                    if (_readBleachTimer.IsEnabled)
                    {
                        _readBleachTimer.Stop();
                    }
                }
                return val;
            }
        }

        /// <summary>
        /// Gets a value indicating whether this instance is bleaching stopped.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance is bleaching stopped; otherwise, <c>false</c>.
        /// </value>
        public bool IsBleachingStopped
        {
            get
            {
                return (true == IsBleaching) ? false : true;
            }
        }

        /// <summary>
        /// Gets or sets the trigger bleach mode.
        /// </summary>
        /// <value>
        /// The trigger bleach mode.
        /// </value>
        public int TriggerBleachMode
        {
            get { return _triggerBleachMode; }
            set
            {
                _triggerBleachMode = value;
                OnPropertyChanged("TriggerBleachMode");
            }
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Edits the bleach settings.
        /// </summary>
        public void EditBleachSettings()
        {
            EditBleachDialog dlg = new EditBleachDialog();

            dlg.DataContext = this;
            dlg.RealTimeVM = this;

            if (false == File.Exists(Constants.ThorRealTimeData.SETTINGS_FILE_NAME))
                return;

            dlg.Settings = SettingsDoc;
            dlg.windowCorner = LoadSubWindowPos();
            dlg.BleachDialogClosed += new Action<int, int>(PersistSubWindowPos);

            if (true == dlg.ShowDialog())
            {
                SaveDocumentSettings();
            }
        }

        /// <summary>
        /// Starts the bleaching.
        /// </summary>
        public void StartBleaching()
        {
            if (false == IsBleaching)
            {
                if (RealTimeDataCapture.Instance.StartAsyncAcquire())
                {
                    _readBleachTimer.Start();
                    OnPropertyChanged("BleachingIcon");
                    OnPropertyChanged("IsBleachingStopped");
                }
            }
            else
            {
                StopBleaching();
            }
        }

        /// <summary>
        /// Stops the bleaching.
        /// </summary>
        private void StopBleaching()
        {
            if (true == IsBleaching)
            {
                _splash = new SplashProgress();
                _splash.DisplayText = "Please wait while running ...";
                _splash.ShowInTaskbar = false;
                _splash.Owner = Application.Current.MainWindow;
                _splash.Show();
                EnableUIPanel(false);

                //background worker to update progress:
                BackgroundWorker waitWorker = new BackgroundWorker();
                BackgroundWorker splashWkr = new BackgroundWorker();
                splashWkr.WorkerSupportsCancellation = true;
                bool splashWkrDone = false;

                try
                {
                    waitWorker.DoWork += delegate(object sender, DoWorkEventArgs e)
                    {
                        do
                        {
                            System.Threading.Thread.Sleep(1);
                        }
                        while ((false == splashWkrDone) || (true == splashWkr.IsBusy));
                    };

                    splashWkr.DoWork += delegate(object sender, DoWorkEventArgs e)
                    {
                        RealTimeDataCapture.Instance.StopAsyncAcquire();
                        OnPropertyChanged("BleachingIcon");
                        OnPropertyChanged("IsBleachingStopped");
                    };
                    splashWkr.RunWorkerCompleted += delegate(object sender, RunWorkerCompletedEventArgs e)
                    {
                        splashWkrDone = true;
                        _splash.Close();
                        Application.Current.MainWindow.Activate();
                        EnableUIPanel(true);
                    };

                    splashWkr.RunWorkerAsync();
                    waitWorker.RunWorkerAsync();
                }
                catch (Exception ex)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart Error: " + ex.Message);
                    splashWkrDone = true;
                    splashWkr.CancelAsync();
                }
            }
        }

        /// <summary>
        /// Handles the Tick event of the _readBleachTimer control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void _readBleachTimer_Tick(object sender, EventArgs e)
        {
            OnPropertyChanged("IsBleaching");
            OnPropertyChanged("BleachingIcon");
            OnPropertyChanged("IsBleachingStopped");
        }

        #endregion Methods
    }
}