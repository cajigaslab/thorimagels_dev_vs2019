namespace RealTimeLineChart.ViewModel
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Collections.Specialized;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Timers;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Threading;
    using System.Xml;
    using FolderDialogControl;
    using DatabaseInterface;
    using global::RealTimeLineChart.Model;
    using global::RealTimeLineChart.View;

    using HDF5CS;

    using SciChart;
    using SciChart.Charting;
    using SciChart.Charting.ChartModifiers;
    using SciChart.Charting.Common.Extensions;
    using SciChart.Charting.Model.ChartSeries;
    using SciChart.Charting.Model.DataSeries;
    using SciChart.Charting.Themes;
    using SciChart.Charting.Utility;
    using SciChart.Charting.Visuals;
    using SciChart.Charting.Visuals.Annotations;
    using SciChart.Charting.Visuals.Axes;
    using SciChart.Charting.Visuals.Axes.LabelProviders;
    using SciChart.Charting.Visuals.Events;
    using SciChart.Charting.Visuals.RenderableSeries;
    using SciChart.Core.Framework;
    using SciChart.Data.Model;
    using SciChart.Drawing.HighSpeedRasterizer;
    using SciChart.Drawing.Utility;

    using ThorLogging;

    public struct mostRecentInfo
    {
        #region Fields

        public string mostRecentName;
        public string mostRecentPath;

        #endregion Fields
    }

    public static class Extensions
    {
        #region Methods

        public static T[] SubArray<T>(this T[] array, int offset, int length)
        {
            T[] result = new T[length];
            Array.Copy(array, offset, result, 0, length);
            return result;
        }

        #endregion Methods
    }

    public partial class RealTimeLineChartViewModel : ViewModelBase
    {
        #region Fields

        private static RealTimeDataCapture.ReportNewData _dataCallBack;
        private static RealTimeDataCapture.ReportNewSpectral _spectralCallBack;

        List<double> Xmax = new List<double> { };
        List<double> Xmin = new List<double> { };
        List<double> Ymax = new List<double> { };
        List<double> Ymin = new List<double> { };
        private string _boardType;
        private BackgroundWorker _bw = null;
        private bool _bwLoadDone = true;
        private BackgroundWorker _bwLoader = null;
        private BackgroundWorker _bwSpecAnalyzer = null;
        private BackgroundWorker _bwWaiter = null;
        private ObservableCollection<ChannelViewModel> _channelViewModels;
        List<bool> _channelVisibilityChoice;
        private ChartModes _chartMode = ChartModes.CAPTURE;
        private ObservableCollection<IRenderableSeries> _chartSeries;
        ObservableCollection<IRenderableSeries> _chartSeriesForScrollBar;
        private List<SeriesMetadata> _chartSeriesMetadata;
        private long _clockRate = Constants.ThorRealTimeData.CLK_RATE_20MHZ; // clock rate defined by type of NI card, 20M Hz for NI 6363
        private double _closingPMTShutterDuration;
        private bool _configVisibility = true;
        private RealTimeDataCapture.CompoundDataStruct _dataCaptureStruct;
        private UInt64 _dataSeriesSize = 0;
        bool _displayDataWhileReviewLoading = false;
        private ObservableCollection<string> _displayOptionList = new ObservableCollection<string>();
        private int _displayOptionSelectedIndex;
        private List<int> _displayParamValue = new List<int>();
        private int _fifoSize = 0;
        private string _filePath = String.Empty;
        private bool _forceIPC = false;
        private H5CSWrapper _hdf5Reader = null;
        private int _imageCounterNumber;
        private bool _isAiTriggerEnabled;
        private ObservableCollection<bool> _isCounterLinePlotEnabled = new ObservableCollection<bool>();
        private bool _isDragToScaleEnabled = false;
        private bool _isPanelsEnable = true;
        private bool _isStimulusEnabled;
        private UInt64 _lastGlobalCounter;
        private ICommand _markerDisplayCommand;
        private mostRecentInfo _mostRecentInfo;
        private int _progressPercentage = 0;
        private DispatcherTimer _readCaptureTimer;
        private int _removedSpectralChannel = 0;
        private bool _renameSavingFileDontShowAgain = false;
        private int _sampleRate = 0;
        private ObservableCollection<string> _sampleRateList = new ObservableCollection<string>();
        private List<int> _sampleRateValue = new List<int>();
        private double _samplingDuration;
        private string _saveEpisode;
        private string _saveName;
        private string _savePath;
        private ChannelViewModel _selectedFreqCh;
        private int _selectedSpectralChannel;
        private ICommand _setSavePathCommand;
        private SeriesMetaMode _smmode = null;
        private ObservableCollection<SpectralViewModel> _specChViewModels;
        private GridLength _specPanelHeight = new GridLength(100, GridUnitType.Pixel);
        private List<SeriesMetadata> _specSeriesMetadata;
        private SplashProgress _splash;
        private double _stimulusLimit;
        private int _triggerMode = 0;
        IRange _yDataRange = new DoubleRange(-5, 5);

        public static RealTimeLineChartViewModel _instance;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="RealTimeLineChartViewModel"/> class.
        /// </summary>
        public RealTimeLineChartViewModel()
        {
            _instance = this;

            _chartSeries = new ObservableCollection<IRenderableSeries>();

            _chartSeriesMetadata = new List<SeriesMetadata>();
            _specSeriesMetadata = new List<SeriesMetadata>();

            _readCaptureTimer = new System.Windows.Threading.DispatcherTimer();
            _readCaptureTimer.Interval = new TimeSpan(0, 0, 0, 0, 200);
            _readCaptureTimer.Tick += _readCaptureTimer_Tick;

            _readBleachTimer = new System.Windows.Threading.DispatcherTimer();
            _readBleachTimer.Interval = new TimeSpan(0, 0, 0, 0, 1000);
            _readBleachTimer.Tick += _readBleachTimer_Tick;

            _dataCaptureStruct = new RealTimeDataCapture.CompoundDataStruct();

            _bw = new BackgroundWorker();
            _bw.WorkerSupportsCancellation = true;
            _bw.DoWork += _bw_DoWork;

            _bwLoader = new BackgroundWorker();
            _bwSpecAnalyzer = new BackgroundWorker();
            _smmode = new SeriesMetaMode();
            _specChViewModels = new ObservableCollection<SpectralViewModel>();
            _channelViewModels = new ObservableCollection<ChannelViewModel>();
            _spectralCallBack = new RealTimeDataCapture.ReportNewSpectral(UpdateSpectral);
            _dataCallBack = new RealTimeDataCapture.ReportNewData(UpdateTimeDomainData);
            XVisibleRangeStack = new DoubleRange(0, 0);
        }

        #endregion Constructors

        #region Enumerations

        /// <summary>
        /// 
        /// </summary>
        public enum ChartModes : int
        {
            CAPTURE = 0,
            REVIEW = 1
        }

        /// <summary>
        /// 
        /// </summary>
        public enum MeasurementCursorMode
        {
            invisible,
            visible,
        }

        #endregion Enumerations

        #region Delegates

        /// <summary>
        /// report the progress
        /// </summary>
        /// <param name="percentage">The percentage.</param>
        public delegate void UpdateProgressDelegate(int percentage);

        #endregion Delegates

        #region Events

        /// <summary>
        /// Occurs when [event chart y axis range changed].
        /// </summary>
        public event Action<double, double> EventChartYAxisRangeChanged;

        /// <summary>
        /// Occurs when [event vertical marker mode changed].
        /// </summary>
        public event Action<int, double> EventVerticalMarkerModeChanged;

        /// <summary>
        /// Occurs when [event vertical marker selected index changed].
        /// </summary>
        public event Action<int> EventVerticalMarkerSelectedIndexChanged;

        /// <summary>
        /// Occurs when [event xy axis scrollbar enabled changed].
        /// </summary>
        public event Action<bool> EventXYAxisScrollbarEnabledChanged;

        public event Action<RenderPriority> UpdateRenderPriority;

        #endregion Events

        #region Properties

        public int Average
        {
            get
            {
                return CustomModifiers.ChartCanvasModifier.Average;
            }
            set
            {
                CustomModifiers.ChartCanvasModifier.Average = value;
                OnPropertyChanged("Average");
            }
        }

        public DoubleRange AxisVisibleRange
        {
            get
            {
                DoubleRange dr = new DoubleRange(0.1, _sampleRate * 5);
                return dr;
            }
        }

        /// <summary>
        /// Gets or sets the type of the NI board.
        /// </summary>
        /// <value>
        /// The type of the board.
        /// </value>
        public string BoardType
        {
            get
            { return _boardType; }
            set
            {
                _boardType = value;
                OnPropertyChanged("BoardType");
            }
        }

        
        /// <summary>
        /// Gets the capturing icon.
        /// </summary>
        /// <value>
        /// The capturing icon.
        /// </value>
        public string CapturingIcon
        {
            get
            {
                if (IsCapturing)
                {
                    return "/RealTimeLineChart;component/Icons/Stop.png";
                }
                else
                {
                    return "/RealTimeLineChart;component/Icons/Play.png";
                }
            }
        }

        public ObservableCollection<ChannelViewModel> ChannelViewModels
        {
            get { return _channelViewModels; }
            set
            {
                _channelViewModels = value;
                OnPropertyChanged("ChannelViewModels");
            }
        }

        /// <summary>
        /// Gets or sets the chart mode. 0: RealTime Capture 1: Offline Review
        /// </summary>
        /// <value>
        /// The chart mode.
        /// </value>
        public int ChartMode
        {
            get
            {
                return (int)_chartMode;
            }
            set
            {
                if (_chartMode != (ChartModes)value)
                {
                    _chartMode = (ChartModes)value;
                    switch ((ChartModes)value)
                    {
                        case ChartModes.REVIEW:
                            {
                                StopCapturing();
                                StopBleaching();
                                SaveDocumentSettings();
                                IsDragToScaleEnabled = true;
                                break;
                            }
                        case ChartModes.CAPTURE:
                            {
                                LoadDocumentSettings();
                                SettingPath = Constants.ThorRealTimeData.SETTINGS_FILE_NAME;
                                OTMSettingPath = Constants.ThorRealTimeData.OTM_SETTINGS_FILE;
                                IsDragToScaleEnabled = false;
                                UpdateRenderPriority(RenderPriority.Normal);
                                break;
                            }
                        default:
                            break;
                    }
                }

                OnPropertyChanged("ChartMode");
                OnPropertyChanged("AutoRangeX");
                OnPropertyChanged("AutoRangeY");
                OnPropertyChanged("StackHorizontalDataScrollerVisibility");
            }
        }

        /// <summary>
        /// Gets the chart series Data.
        /// </summary>
        /// <value>
        /// The chart series.
        /// </value>
        public ObservableCollection<IRenderableSeries> ChartSeries
        {
            get
            {
                return _chartSeries;
            }
            private set
            {
                _chartSeries = value;
                OnPropertyChanged("ChartSeries");
            }
        }

        /// <summary>
        /// Gets the chart series Data.
        /// </summary>
        /// <value>
        /// The chart series.
        /// </value>
        public ObservableCollection<IRenderableSeries> ChartSeriesForScrollBar
        {
            get
            {
                return _chartSeriesForScrollBar;
            }
            private set
            {
                _chartSeriesForScrollBar = value;
                OnPropertyChanged("ChartSeriesForScrollBar");
            }
        }

        /// <summary>
        /// Gets or sets the duration of the closing PMT shutter.
        /// </summary>
        /// <value>
        /// The duration of the closing PMT shutter.
        /// </value>
        public double ClosingPMTShutterDuration
        {
            get
            { return _closingPMTShutterDuration; }
            set
            {
                _closingPMTShutterDuration = value;
                OnPropertyChanged("ClosingPMTShutterDuration");
            }
        }

        public bool ConfigVisibility
        {
            get { return _configVisibility; }
            set
            {
                _configVisibility = value;
                OnPropertyChanged("ConfigVisibility");
            }
        }

        /// <summary>
        /// Gets or sets the database file.
        /// </summary>
        /// <value>
        /// The database file.
        /// </value>
        public string DatabaseFile
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the display option list.
        /// </summary>
        /// <value>
        /// The display option list.
        /// </value>
        public ObservableCollection<string> DisplayOptionList
        {
            get
            {
                return _displayOptionList;
            }
            set
            {
                _displayOptionList = value;
                OnPropertyChanged("DisplayOptionList");
            }
        }

        /// <summary>
        /// Gets or sets the selected index of the display option .
        /// </summary>
        /// <value>
        /// The display index of the option selected.
        /// </value>
        public int DisplayOptionSelectedIndex
        {
            get
            {
                return _displayOptionSelectedIndex;
            }
            set
            {
                _displayOptionSelectedIndex = value;
                OnPropertyChanged("DisplayOptionSelectedIndex");
                OnPropertyChanged("FifoSize");
                OnPropertyChanged("DisplayResolution");
            }
        }

        /// <summary>
        /// Gets or sets the fifo size.
        /// </summary>
        /// <value>
        /// The fifo size.
        /// </value>
        public double FifoSize
        {
            get
            {
                if ((_fifoSize > 0) && (SampleRate >= 0) && (DisplayOptionSelectedIndex >= 0) && (_sampleRateValue.Count > 0)
                    && (_sampleRateValue.Count > SampleRate) && (_sampleRateValue[SampleRate] > 0) && (DisplayOptionSelectedIndex < _displayParamValue.Count))
                {
                    return Math.Round(_fifoSize * (double)_displayParamValue[DisplayOptionSelectedIndex] / (double)_sampleRateValue[SampleRate], 5);
                }

                return 1000;
            }
            set
            {
                if ((SampleRate >= 0) && (_sampleRateValue[SampleRate] > 0) && (DisplayOptionSelectedIndex >= 0))
                {
                    // minimum fifoSize to be 2 for line range display:
                    _fifoSize = Convert.ToInt32(value * (double)_sampleRateValue[SampleRate] / (double)_displayParamValue[DisplayOptionSelectedIndex]);
                    _fifoSize = (_fifoSize <= 1) ? 2 : _fifoSize;

                    for (int i = 0; i < ChartSeries.Count; i++)
                    {
                        ChannelViewModels[i].ChannelSeries.FifoCapacity = ChartSeries[i].DataSeries.FifoCapacity = _fifoSize;
                    }
                }
                OnPropertyChanged("FifoSize");
            }
        }

        /// <summary>
        /// Gets or sets the data file Path.
        /// </summary>
        /// <value>
        /// The data file Path.
        /// </value>
        public string FilePath
        {
            get
            {
                return _filePath;
            }
            set
            {
                try
                {
                    if (null != _hdf5Reader)
                    {
                        _hdf5Reader.DestroyH5();
                        _hdf5Reader = null;
                    }
                    _hdf5Reader = new H5CSWrapper(value);
                }
                catch (Exception)
                {
                    MessageBox.Show("Unable to load file (" + value + ")");
                    return;
                }
                finally
                {
                    _filePath = value;
                    Match match = Regex.Match(_filePath, @"\b\w*\b.h5", RegexOptions.IgnoreCase);
                    SaveEpisode = match.Value.Substring(0, match.Length - 3);
                }
            }
        }

        public bool forceIPC
        {
            get
            {
                return _forceIPC;
            }
            set
            {
                _forceIPC = value;
                OnPropertyChanged("forceIPC");
                OnPropertyChanged("IsBlinking");
            }
        }

        /// <summary>
        /// Gets or sets the image counter number.
        /// </summary>
        /// <value>
        /// The image counter number.
        /// </value>
        public int ImageCounterNumber
        {
            get
            {
                return _imageCounterNumber;
            }
            set
            {
                _imageCounterNumber = value;
                OnPropertyChanged("ImageCounterNumber");
            }
        }

        public RealTimeLineChartViewModel Instance
        {
            get
            {
                return _instance;
            }
            set
            {
                
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this instance is ai trigger enabled.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance is ai trigger enabled; otherwise, <c>false</c>.
        /// </value>
        public bool IsAiTriggerEnabled
        {
            get { return _isAiTriggerEnabled; }
            set
            {
                _isAiTriggerEnabled = value;
                OnPropertyChanged("IsAiTriggerEnabled");
            }
        }

        public bool IsBlinking
        {
            get
            {
                return ThorImageLSConnectionStats && (forceIPC && !IsSaving);
            }
        }

        /// <summary>
        /// Gets a value indicating whether this instance is capturing.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance is capturing; otherwise, <c>false</c>.
        /// </value>
        public bool IsCapturing
        {
            get
            {
                bool val = RealTimeDataCapture.Instance.IsAcquiring();
                if (false == val)
                {
                    StopCaptureWorkers();
                }
                return val;
            }
        }

        /// <summary>
        /// Gets a value indicating whether this instance is capturing stopped.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance is capturing stopped; otherwise, <c>false</c>.
        /// </value>
        public bool IsCapturingStopped
        {
            get
            {
                return (true == IsCapturing) ? false : true;
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this instance CounterLine plot is enabled.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance CounterLine plot is enabled; otherwise, <c>false</c>.
        /// </value>
        public ObservableCollection<bool> IsCounterLinePlotEnabled
        {
            get { return _isCounterLinePlotEnabled; }
            set
            {
                _isCounterLinePlotEnabled = value;
                OnPropertyChanged("IsCounterLinePlotEnabled");
            }
        }

        public bool IsCounting
        {
            get
            {
                if (null != SettingsDoc)
                {
                    XmlNodeList ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/DataChannel[@group='/CI']");
                    if (0 < ndList.Count)
                    {
                        string str = string.Empty; int iVal = 0;
                        if (GetAttribute(ndList[0], SettingsDoc, "enable", ref str) && Int32.TryParse(str, out iVal))
                        {
                            return (1 == iVal) ? true : false;
                        }
                    }
                }
                return false;
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this instance is drag to scale enabled.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance is drag to scale enabled; otherwise, <c>false</c>.
        /// </value>
        public bool IsDragToScaleEnabled
        {
            get { return _isDragToScaleEnabled; }
            set
            {
                _isDragToScaleEnabled = value;
                OnPropertyChanged("IsDragToScaleEnabled");
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this instance is panels enable.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance is panels enable; otherwise, <c>false</c>.
        /// </value>
        public bool IsPanelsEnable
        {
            get
            { return _isPanelsEnable; }
            set
            {
                _isPanelsEnable = value;
                OnPropertyChanged("IsPanelsEnable");
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this instance is saving.
        /// </summary>
        /// <value>
        ///   <c>true</c> if this instance is saving; otherwise, <c>false</c>.
        /// </value>
        public bool IsSaving
        {
            get
            {
                return RealTimeDataCapture.Instance.GetSaving();
            }
            set
            {
                RealTimeDataCapture.Instance.SetSaving(value);
                if (ThorImageLSConnectionStats == true)
                {
                    SendThorSyncConfiguration(); // Send configuration Information to ThorImage If the connection is established
                }
                OnPropertyChanged("IsSaving");
                OnPropertyChanged("IsBlinking");
            }
        }

        public Visibility IsSelectedFreqChNull
        {
            get
            {
                return (SelectedFreqCh == null) ? Visibility.Collapsed : Visibility.Visible;
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this instance is stimulus enabled.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance is stimulus enabled; otherwise, <c>false</c>.
        /// </value>
        public bool IsStimulusEnabled
        {
            get { return _isStimulusEnabled; }
            set
            {
                _isStimulusEnabled = value;
                OnPropertyChanged("IsStimulusEnabled");
            }
        }

        public ICommand MarkerDisplayCommand
        {
            get
            {
                if (this._markerDisplayCommand == null)
                    this._markerDisplayCommand = new RelayCommand(() => IsVerticalMarkerVisible = !IsVerticalMarkerVisible);
                return this._markerDisplayCommand;
            }
        }

        /// <summary>
        /// Gets or sets the progress percentage.
        /// </summary>
        /// <value>
        /// The progress percentage.
        /// </value>
        public int ProgressPercentage
        {
            get
            { return _progressPercentage; }
            set
            {
                _progressPercentage = value;
                OnPropertyChanged("ProgressPercentage");
            }
        }

        public bool RefreshFrameTimes
        {
            get
            {
 
                return false;
            }
            set
            {
                List<double> frameTimes = new List<double>();

                RealTimeDataCapture.Instance.LoadDataFromFile();
                for (int j = 0; j < _chartSeriesMetadata.Count; j++)
                {
                    if (_channelViewModels[j].ChannelName.Equals("Line1") || _channelViewModels[j].ChannelName.Equals("FrameOut"))
                    {
                        int count = _channelViewModels[j].ChannelSeries.XValues.Count;

                        for (int x = 0; x < count - 1; x++)
                        {
                            byte digYLeft = (byte)_channelViewModels[j].ChannelSeries.YValues[x];
                            byte digYRight = (byte)_channelViewModels[j].ChannelSeries.YValues[x + 1];

                            double X = (double)_channelViewModels[j].ChannelSeries.XValues[x];

                            if (digYLeft < digYRight)
                            {
                                frameTimes.Add(X);

                            }
                            
                        }
                        RealTimeDataCapture.Instance.FrameTimes = frameTimes;
                    }
                }
            }
        }

        public bool RenameSavingFileDontShowAgain
        {
            get
            {
                return _renameSavingFileDontShowAgain;
            }

            set
            {
                _renameSavingFileDontShowAgain = value;
                OnPropertyChanged("RenameSavingFileDontShowAgain");
            }
        }

        /// <summary>
        /// Gets or sets the sample rate.
        /// </summary>
        /// <value>
        /// The sample rate.
        /// </value>
        public int SampleRate
        {
            get
            {
                return _sampleRate;
            }
            set
            {
                if ((value >= 0) || (_sampleRate != value))
                {
                    for (int i = 0; i < _displayParamValue.Count; i++)
                    {
                        if (value >= 0)
                        { _displayParamValue[i] = Convert.ToInt32(_sampleRateValue[value] / (Math.Pow(10, _displayParamValue.Count + 1 - i))); }
                    }
                    _sampleRate = value;
                    OnPropertyChanged("SampleRate");
                    OnPropertyChanged("SamplingRate");
                }
                OnPropertyChanged("FifoSize");
                OnPropertyChanged("AxisVisibleRange");

            }
        }

        /// <summary>
        /// Gets or sets the sample rate list.
        /// </summary>
        /// <value>
        /// The sample rate list.
        /// </value>
        public ObservableCollection<string> SampleRateList
        {
            get
            {
                return _sampleRateList;
            }
            set
            {
                _sampleRateList = value;
                OnPropertyChanged("SampleRateList");
                OnPropertyChanged("SampleRate");
            }
        }

        /// <summary>
        /// Gets or sets the duration of the sampling.
        /// </summary>
        /// <value>
        /// The duration of the sampling.
        /// </value>
        public double SamplingDuration
        {
            get
            {
                return _samplingDuration;
            }
            set
            {
                _samplingDuration = (double)(Math.Round(value, 1));
                OnPropertyChanged("SamplingDuration");
            }
        }

        /// <summary>
        /// Gets or sets the save episode.
        /// </summary>
        /// <value>
        /// The save episode.
        /// </value>
        public string SaveEpisode
        {
            get { return _saveEpisode; }
            set { _saveEpisode = value; OnPropertyChanged("SaveEpisode"); }
        }

        /// <summary>
        /// Gets or sets the name of the save.
        /// </summary>
        /// <value>
        /// The name of the save.
        /// </value>
        public string SaveName
        {
            get { return _saveName; }
            set { _saveName = value; OnPropertyChanged("SaveName"); }
        }

        /// <summary>
        /// Gets or sets the save path.
        /// </summary>
        /// <value>
        /// The save path.
        /// </value>
        public string SavePath
        {
            get { return _savePath; }
            set { _savePath = value; OnPropertyChanged("SavePath"); }
        }

        public ChannelViewModel SelectedFreqCh
        {
            get
            {
                return _selectedFreqCh;
            }
            set
            {
                _selectedFreqCh = value;
                if (_selectedFreqCh != null)
                {
                }
                OnPropertyChanged("IsSelectedFreqChNull");
            }
        }

        public int SelectedSpectralChannel
        {
            get
            {
                return _selectedSpectralChannel;
            }
            set
            {
                _selectedSpectralChannel = value;
                //if (SpecChViewModels.IndexOf(ChannelViewModels[_selectedSpectralChannel]) == -1)
                //{
                //    if (_selectedSpectralChannel == 2)
                //        _selectedSpectralChannel = 4;

                //    SpecChViewModels.Add(ChannelViewModels[_selectedSpectralChannel]);

                //    XmlNodeList ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/DataChannel[@enable=1]");
                //    foreach (XmlNode node in ndList)
                //    {
                //        string name = string.Empty;
                //        GetAttribute(node, SettingsDoc, "alias", ref name);
                //        if (name.CompareTo(ChannelViewModels[_selectedSpectralChannel].ChannelName) == 0)
                //        {
                //            SetAttribute(node, SettingsDoc, "freqEnable", "1");
                //            break;
                //        }
                //    }
                //    SettingsDoc.Save(SETTINGS_FILE_NAME);
                //}
            }
        }

        public ChannelViewModel SelectedTimeCh
        {
            get
            {
                return _channelViewModels[0];
            }
            set
            {
                _channelViewModels[0] = value;
                if (_channelViewModels[0] != null)
                {
                }
                OnPropertyChanged("IsSelectedFreqChNull");
            }
        }

        /// <summary>
        /// Gets the set save path command.
        /// </summary>
        /// <value>
        /// The set save path command.
        /// </value>
        public ICommand SetSavePathCommand
        {
            get
            {
                if (this._setSavePathCommand == null)
                    this._setSavePathCommand = new RelayCommand(() => SetSavePath());

                return this._setSavePathCommand;
            }
        }

        public XmlDocument SettingsDoc
        {
            get;
            set;
        }

        public string SimulatorFilePath
        {
            get
            {
                string str = string.Empty, str1 = string.Empty, fPath = string.Empty;
                XmlNodeList ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]");
                if (0 < ndList.Count)
                {
                    if (GetAttribute(ndList[0], SettingsDoc, "type", ref str) && 0 == str.CompareTo("Simulator") &&
                        GetAttribute(ndList[0], SettingsDoc, "devID", ref str1) && 0 == str1.CompareTo("FILE"))
                    {
                        ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/FilePath");
                        if (0 < ndList.Count)
                        {
                            if (GetAttribute(ndList[0], SettingsDoc, "folder", ref str) && 0 < str.Length)
                            {
                                fPath = str + "\\" + Constants.ThorRealTimeData.SETTINGS_FILE_NAME;
                            }
                        }
                    }
                }
                return File.Exists(fPath) ? fPath : "";
            }
        }

        public ObservableCollection<SpectralViewModel> SpecChViewModels
        {
            get { return _specChViewModels; }
            set
            {
                _specChViewModels = value;
                OnPropertyChanged("SpecChViewModels");
            }
        }

        public GridLength SpecPanelHeight
        {
            get { return _specPanelHeight; }
            set
            {
                _specPanelHeight = value;
                OnPropertyChanged("SpecPanelHeight");
            }
        }

        /// <summary>
        /// Gets or sets the stimulus limit.
        /// </summary>
        /// <value>
        /// The stimulus limit.
        /// </value>
        public double StimulusLimit
        {
            get { return _stimulusLimit; }
            set
            {
                _stimulusLimit = value;
                OnPropertyChanged("StimulusLimit");
            }
        }

        public double ThreadCallbackTime
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the trigger mode.
        /// </summary>
        /// <value>
        /// The trigger mode.
        /// </value>
        public int TriggerMode
        {
            get { return _triggerMode; }
            set
            {
                _triggerMode = value;
                if (ThorImageLSConnectionStats == true)
                {
                    SendThorSyncConfiguration();
                }
                OnPropertyChanged("TriggerMode");
                OnPropertyChanged("TriggerModeText");
            }
        }

        /// <summary>
        /// Gets the trigger mode text.
        /// </summary>
        /// <value>
        /// The trigger mode text.
        /// </value>
        public string TriggerModeText
        {
            get
            {
                switch (_triggerMode)
                {
                    case 0: return "Free Run";
                    case 1: return "HW Trigger Single";
                    case 2: return "HW Trigger Retriggerable";
                    default: return "HW Synchronizable";
                }
            }
        }

        public IRange YDataRange
        {
            get
            {
                var (min, max) = GetMinMaxEnabledChannels(Ymin, Ymax);
                if (min != null && max != null)
                {
                    _yDataRange.SetMinMax(min.Value - 0.2, max.Value + 0.2);
                }
                return _yDataRange;
            }
            set => SetProperty(ref _yDataRange, value);
        }

        #endregion Properties

        #region Methods

        /// <summary>Determines if the path contains invalid characters.</summary>
        /// <param name="filePath">File path.</param>
        /// <returns>True if file path contains invalid characters.</returns>
        public static bool ContainsInvalidCharacters(string filePath, ref string error)
        {
            if (0 == filePath.Length)
            {
                error = "Empty";
                return true;
            }

            for (var i = 0; i < filePath.Length; i++)
            {
                int c = filePath[i];

                switch (c)
                {
                    case '\"':
                        error = "'\'";
                        return true;
                    case '<':
                        error = "'<'";
                        return true;
                    case '>':
                        error = "'>'";
                        return true;
                    case '|':
                        error = "'|'";
                        return true;
                    case '*':
                        error = "'*'";
                        return true;
                    case '?':
                        error = "'?'";
                        return true;
                    case '+':
                        error = "'+'";
                        return true;
                    case '-':
                        error = "'-'";
                        return true;
                    case '/':
                        error = "'/'";
                        return true;
                    case '^':
                        error = "'^'";
                        return true;
                    case '&':
                        error = "'&'";
                        return true;
                    case ' ':
                        error = "' '";
                        return true;
                    default:
                        break;
                }
                if (c < 32)
                {
                    error = "Unknown";
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        /// Create a xml node
        /// </summary>
        /// <param name="doc"></param>
        /// <param name="nodeName"></param>
        public static void CreateXmlNode(XmlDocument doc, string nodeName, XmlNode source = null)
        {
            XmlNode node = doc.CreateNode(XmlNodeType.Element, nodeName, null);
            if (null == source)
            {
                doc.DocumentElement.AppendChild(node);
            }
            else
            {
                source.AppendChild(node);
            }
        }

        public static void FileCopy(string source, string destination)
        {
            if (File.Exists(source))
                File.Copy(source, destination, true);
        }

        /// <summary>
        /// Gets the attribute.
        /// </summary>
        /// <param name="node">The node.</param>
        /// <param name="doc">The document.</param>
        /// <param name="attrName">Name of the attribute.</param>
        /// <param name="attrValue">The attribute value.</param>
        /// <returns></returns>
        public static bool GetAttribute(XmlNode node, XmlDocument doc, string attrName, ref string attrValue)
        {
            bool ret = false;

            if ((node == null) || (doc == null))
                return ret;

            if (null == node.Attributes.GetNamedItem(attrName))
            {
                ret = false;
            }
            else
            {
                attrValue = node.Attributes[attrName].Value;
                ret = true;
            }

            return ret;
        }

        /// <summary>
        /// Sets the attribute.
        /// </summary>
        /// <param name="node">The node.</param>
        /// <param name="doc">The document.</param>
        /// <param name="attrName">Name of the attribute.</param>
        /// <param name="attValue">The att value.</param>
        public static void SetAttribute(XmlNode node, XmlDocument doc, string attrName, string attValue)
        {
            XmlNode tempNode = node.Attributes[attrName];

            if (null == tempNode)
            {
                XmlAttribute attr = doc.CreateAttribute(attrName);

                attr.Value = attValue;

                node.Attributes.Append(attr);
            }
            else
            {
                node.Attributes[attrName].Value = attValue;
            }
        }

        /// <summary>
        /// Adds the comments options.
        /// </summary>
        /// <returns></returns>
        public string AddCommentsOptions()
        {
            CommentView commendViewDialog = new CommentView();
            commendViewDialog.DataContext = this;
            if (commendViewDialog.ShowDialog() == true)
            {

            }
            return _verticalMarkerTooltipText;
        }

        /// <summary>
        /// Changes the vertical markers mode.
        /// </summary>
        /// <param name="type">The type.</param>
        public void ChangeVerticalMarkersMode(MarkerType type, double value = -1)
        {
            if (null != EventVerticalMarkerModeChanged)
            {
                EventVerticalMarkerModeChanged((int)type, value);
            }
        }

        /// <summary>
        /// Connections the settings options.
        /// </summary>
        public void ConnectionSettingsOptions()
        {
            EditPipeDialog editPipeDialog = new EditPipeDialog();
            editPipeDialog.DataContext = this;
            editPipeDialog.ShowDialog();
        }

        /// <summary>
        /// load chart lines of active board from the specified path.
        /// persistVis: true to keep original visible lines.
        /// loadH5: true when loading H5 files
        /// </summary>
        /// <param name="persistVis">if set to <c>true</c> [keep original visible lines].</param>
        /// <param name="loadH5">if set to <c>true</c> [load h5 files].</param>
        public void CreateChartLines()
        {
            XmlNodeList ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/DataChannel | /RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/VirtualChannel");
            if ((ndList.Count <= 0) && (0 == SimulatorFilePath.Length))
            {
                MessageBox.Show("No Channel is Active");
                return;
            }
            //Re-Create CharSeries and MetaData:
            CreateTimeChartSeries();
            CreateFreqChartSeries();

            //Reset chart settings after create lines:
            ResetScichartSettings();
            OnPropertyChanged("IsCounting");
            UpdateCharts();
            UpdateSpectralCharts();
        }

        /// <summary>
        /// Displays the settings options.
        /// </summary>
        public void DisplaySettingsOptions()
        {
            EditDisplayOption editOptionDialog = new EditDisplayOption();
            editOptionDialog.DataContext = this;
            editOptionDialog.ShowDialog();
        }

        /// <summary>
        /// Edits the lines settings.
        /// </summary>
        public void EditLinesSettings()
        {
            EditLinesDialog dlg = new EditLinesDialog();

            dlg.DataContext = this;

            if (false == File.Exists(Constants.ThorRealTimeData.SETTINGS_FILE_NAME))
                return;

            SaveDocumentSettings();
            dlg.Settings = SettingsDoc;
            dlg.windowCorner = LoadSubWindowPos();
            dlg._realVM = this;
            dlg.LinesDialogClosed += new Action<int, int>(PersistSubWindowPos);

            if (true == dlg.ShowDialog())
            {
                SaveDocumentSettings();
                //Load XML after success edit line:
                LoadDocumentSettings();
                CreateChartLines();
            }
        }

        /// <summary>
        /// Enables the scrollbar.
        /// </summary>
        /// <param name="flag">if set to <c>true</c> [flag].</param>
        public void EnableScrollbar(bool flag)
        {
            EventXYAxisScrollbarEnabledChanged?.Invoke(flag);
        }

        /// <summary>
        /// Enables the UI panel.
        /// </summary>
        /// <param name="isPanelEnable">if set to <c>true</c> [is panel enable].</param>
        public void EnableUIPanel(bool isPanelEnable)
        {
            IsPanelsEnable = isPanelEnable;
        }

        /// <summary>
        /// Exits the acquisition. Release the relative resources.
        /// </summary>
        /// <returns></returns>
        public bool ExitAcquisition()
        {
            return RealTimeDataCapture.Instance.ExitAcquisition();
        }

        /// <summary>
        /// Initializes the acquisition.
        /// </summary>
        /// <returns></returns>
        public bool InitAcquisition()
        {
            RealTimeDataCapture.Instance.CreateCallback(_spectralCallBack, _dataCallBack);
            return RealTimeDataCapture.Instance.EnterAcquisition();
        }

        /// <summary>
        /// Load data from the DataCaptureStruct
        /// </summary>
        /// <param name="lastGlobalCounter">The last global counter.</param>
        /// <returns></returns>
        public bool LoadDataFromDevice(ref UInt64 lastGlobalCounter)
        {
            if (_dataCaptureStruct.gcLength > 0)
            {

                if (_xdata?.Length != (int)_dataCaptureStruct.gcLength)
                {
                    _xdata = new Int64[_dataCaptureStruct.gcLength];
                    _xdataDouble = new double[_dataCaptureStruct.gcLength];
                }
                Marshal.Copy(_dataCaptureStruct.gCtr64, _xdata, 0, (int)_dataCaptureStruct.gcLength);

                if ((UInt64)_xdata[0] < lastGlobalCounter)
                {
                    if (_chartMode == ChartModes.CAPTURE)
                        return false;
                }

                lastGlobalCounter = (UInt64)_xdata[_dataCaptureStruct.gcLength - 1];
            }
            else
            {
                return false;
            }

            ulong[] signalTypeCount = new ulong[(int)SignalType.LAST_SIGNAL_TYPE];

            if (_dataCaptureStruct.diLength > 0 && _diData?.Length != (int)_dataCaptureStruct.gcLength)
            {
                _diData = new byte[_dataCaptureStruct.gcLength];
            }
            if (_dataCaptureStruct.aiLength > 0 && _aiData?.Length != (int)_dataCaptureStruct.gcLength)
            {
                _aiData = new double[_dataCaptureStruct.gcLength];
            }
            if (_dataCaptureStruct.ciLength > 0 && _ciData?.Length != (int)_dataCaptureStruct.gcLength)
            {
                _ciData = new int[_dataCaptureStruct.gcLength];
            }
            if (_dataCaptureStruct.viLength > 0 && _viData?.Length != (int)_dataCaptureStruct.gcLength)
            {
                _viData = new double[_dataCaptureStruct.gcLength];
            }

            bool isXSet = false;
            using (MainChartSurface.SuspendUpdates())
            {
                for (int j = 0; j < ChartSeries.Count; j++)
                {
                    if (!isXSet)
                    {
                        for (UInt64 i = 0; i < (UInt64)_xdata.Length; i++)
                        {
                            _xdataDouble[i] = (double)_xdata[i] / (double)_clockRate;
                        }
                        isXSet = true;
                    }

                    switch (_chartSeriesMetadata[j].SignalType)
                    {
                        case SignalType.DIGITAL_IN:
                            {
                                if (_dataCaptureStruct.diLength > 0)
                                {
                                    int offset = (int)signalTypeCount[(int)SignalType.DIGITAL_IN] * (int)_dataCaptureStruct.gcLength * sizeof(byte);
                                    Marshal.Copy(IntPtr.Add(_dataCaptureStruct.diData, offset), _diData, 0, (int)_dataCaptureStruct.gcLength);

                                    Application.Current.Dispatcher.Invoke(new Action(() =>
                                    {
                                        for (int i = 0; i < _xdataDouble.Length; ++i)
                                        {
                                            ((IXyDataSeries<double, int>)ChartSeries[j].DataSeries).Append(_xdataDouble[i], _diData[i]);
                                        }
                                    }));

                                    signalTypeCount[(int)_chartSeriesMetadata[j].SignalType]++;
                                }
                                break;
                            }
                        case SignalType.ANALOG_IN:
                            {
                                if (_dataCaptureStruct.aiLength > 0)
                                {
                                    int offset = (int)signalTypeCount[(int)SignalType.ANALOG_IN] * (int)_dataCaptureStruct.gcLength * sizeof(double);
                                    Marshal.Copy(IntPtr.Add(_dataCaptureStruct.aiData, offset), _aiData, 0, (int)_dataCaptureStruct.gcLength);

                                    Application.Current.Dispatcher.Invoke(new Action(() =>
                                    {
                                        ((IXyDataSeries<double, double>)ChartSeries[j].DataSeries).Append(_xdataDouble, _aiData);
                                    }));

                                    signalTypeCount[(int)_chartSeriesMetadata[j].SignalType]++;
                                }
                                break;
                            }
                        case SignalType.COUNTER_IN:
                            {
                                if (_dataCaptureStruct.ciLength > 0)
                                {
                                    int offset = (int)signalTypeCount[(int)SignalType.COUNTER_IN] * (int)_dataCaptureStruct.gcLength * sizeof(int);
                                    Marshal.Copy(IntPtr.Add(_dataCaptureStruct.ciData, offset), _ciData, 0, (int)_dataCaptureStruct.gcLength);

                                    for (int nC = 0; nC < IsCounterLinePlotEnabled.Count; nC++)
                                    {
                                        if (IsCounterLinePlotEnabled[nC])
                                        {
                                            Application.Current.Dispatcher.Invoke(new Action(() =>
                                            {
                                                ((IXyDataSeries<double, int>)ChartSeries[j].DataSeries).Append(_xdataDouble, _ciData);
                                            }));
                                        }
                                    }

                                    ImageCounterNumber = _ciData[_ciData.Length - 1];

                                    signalTypeCount[(int)_chartSeriesMetadata[j].SignalType]++;
                                }
                            }
                            //continue;
                            break;
                        case SignalType.VIRTUAL:
                            {
                                if (_dataCaptureStruct.viLength > 0)
                                {
                                    int offset = (int)signalTypeCount[(int)SignalType.VIRTUAL] * (int)_dataCaptureStruct.gcLength * sizeof(double);
                                    Marshal.Copy(IntPtr.Add(_dataCaptureStruct.viData, offset), _viData, 0, (int)_dataCaptureStruct.gcLength);

                                    Application.Current.Dispatcher.Invoke(new Action(() =>
                                    {
                                        ((IXyDataSeries<double, double>)ChartSeries[j].DataSeries).Append(_xdataDouble, _viData);
                                    }));

                                    signalTypeCount[(int)_chartSeriesMetadata[j].SignalType]++;
                                }
                                break;
                            }
                    }
                }
            }
            return true;
        }

        /// <summary>
        /// Loads the document settings.
        /// </summary>
        /// <param name="isLoadData">if set to <c>true</c> [is load data].</param>
        /// <param name="path">The path.</param>
        public void LoadDocumentSettings(string path = "")
        {
            string strTemp = string.Empty, strTemp1 = string.Empty;
            int iVal = 0;
            short sVal = 0;
            double dVal = 0;

            string docPath = (path.Length > 0) ? (path + "\\" + Constants.ThorRealTimeData.SETTINGS_FILE_NAME) : Constants.ThorRealTimeData.SETTINGS_FILE_NAME;
            SettingsDoc = new XmlDocument();
            SettingsDoc.Load(docPath);

            //load from target settings if simulator file mode
            if (0 < SimulatorFilePath.Length)
            {
                XmlDocument tmpDoc = new XmlDocument();
                tmpDoc.Load(SimulatorFilePath);

                //remove all nodes except FilePath
                XmlNodeList ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/*[not(self::FilePath)]");
                for (int i = 0; i < ndList.Count; i++)
                {
                    ndList[i].ParentNode.RemoveChild(ndList[i]);
                }

                //append all other nodes from file except FilePath
                XmlNode tgNode = tmpDoc.SelectSingleNode("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]");
                XmlNode copiedNode = SettingsDoc.ImportNode(tgNode, true);

                XmlNode nd = SettingsDoc.SelectSingleNode("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]");

                var attributesFromFile = new List<string> { "totalAI", "totalDI" };

                for (int i = 0; i < tgNode.Attributes.Count; i++)
                {
                    if (attributesFromFile.Contains(tgNode.Attributes[i].Name, StringComparer.Ordinal))
                    {
                        SetAttribute(nd, SettingsDoc, tgNode.Attributes[i].Name, tgNode.Attributes[i].Value);
                    }
                }
                while (0 < copiedNode.ChildNodes.Count)
                {
                    if ("FilePath" == copiedNode.ChildNodes[0].Name)
                    {
                        copiedNode.RemoveChild(copiedNode.ChildNodes[0]);
                    }
                    else
                    {
                        nd.AppendChild(copiedNode.ChildNodes[0]);
                    }
                }
                SettingsDoc.Save(docPath);
            }

            //start load general settings from root xml document
            XmlNodeList settingsList = SettingsDoc.SelectNodes("/RealTimeDataSettings/UserSettings/SciChart");

            if (settingsList.Count <= 0)
                return;

            //start load settings, lock by flag
            _isLoaded = false;
            if (GetAttribute(settingsList[0], SettingsDoc, "IsStackedDisplay", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
            {
                IsStackedDisplay = (1 == iVal) ? true : false;
            }
            if (GetAttribute(settingsList[0], SettingsDoc, "fifoSize", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
            {
                _fifoSize = (iVal <= 0) ? 1000 : iVal;
                OnPropertyChanged("FifoSize");
            }
            if (GetAttribute(settingsList[0], SettingsDoc, "TimeStackHeight", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
            {
                ChannelViewModels.Each(s => s.Height = iVal);
            }
            if (GetAttribute(settingsList[0], SettingsDoc, "SpecStackHeight", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
            {
                SpecChViewModels.Each(s => s.Height = iVal);
            }
            if (GetAttribute(settingsList[0], SettingsDoc, "SpecPanelHeight", ref strTemp) && (Double.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
            {
                SpecPanelHeight = new GridLength(dVal, GridUnitType.Pixel);
            }
            if (GetAttribute(settingsList[0], SettingsDoc, "StatsPanelEnable", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
            {
                StatsPanelEnable = (1 == iVal) ? true : false;
            }

            settingsList = SettingsDoc.SelectNodes("/RealTimeDataSettings/Database");

            if (settingsList.Count > 0)
            {
                if (GetAttribute(settingsList[0], SettingsDoc, "pathAndName", ref strTemp))
                {
                    DatabaseFile = strTemp;
                }
            }

            settingsList = SettingsDoc.SelectNodes("/RealTimeDataSettings/UserSettings/Save");

            if (settingsList.Count > 0)
            {
                if (GetAttribute(settingsList[0], SettingsDoc, "forceIPC", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
                {
                    forceIPC = (1 == iVal) ? true : false;
                }

                if (GetAttribute(settingsList[0], SettingsDoc, "enable", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
                {
                    IsSaving = (1 == iVal) ? true : false;
                }

                if (GetAttribute(settingsList[0], SettingsDoc, "path", ref strTemp))
                {
                    SavePath = strTemp;
                }

                if (GetAttribute(settingsList[0], SettingsDoc, "name", ref strTemp))
                {
                    SaveName = strTemp;
                }

                if (GetAttribute(settingsList[0], SettingsDoc, "CallbackSec", ref strTemp) && (Double.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                {
                    dVal = (double)Decimal.Round((decimal)dVal, 2);
                }
                ThreadCallbackTime = (Constants.ThorRealTimeData.MIN_THREAD_TIME < dVal) ? dVal : Constants.ThorRealTimeData.MIN_THREAD_TIME;

                if (GetAttribute(settingsList[0], SettingsDoc, "DontShowAgain", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
                {
                    RenameSavingFileDontShowAgain = (1 == iVal) ? true : false;
                }
            }

            settingsList = SettingsDoc.SelectNodes("/RealTimeDataSettings/UserSettings/Configuration");

            if (settingsList.Count > 0)
            {
                if (GetAttribute(settingsList[0], SettingsDoc, "VisibleChoices", ref strTemp) && Int16.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out sVal))
                {
                    BitVector32 secVal = new BitVector32(sVal);
                    for (int i = 0; i < _visibilityCollection.Count; i++)
                    {
                        _visibilityCollection[i] = secVal[((1 > i) ? i : 1 << i)];
                    }
                }
                if (GetAttribute(settingsList[0], SettingsDoc, "OTM", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
                {
                    Configuration[0] = (1 == iVal) ? true : false;
                }
                if (GetAttribute(settingsList[0], SettingsDoc, "DisplayDataWhileReviewLoading", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
                {
                    _displayDataWhileReviewLoading = (1 == iVal) ? true : false;
                }
            }

            //start load active board settings from file folder if simulator file mode
            XmlNodeList daqList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]");

            if (daqList.Count <= 0)
                return;

            if (GetAttribute(daqList[0], SettingsDoc, "type", ref strTemp))
            {
                BoardType = strTemp;
            }

            if (GetAttribute(daqList[0], SettingsDoc, "hwTrigMode", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
            {
                TriggerMode = iVal;
            }

            if (GetAttribute(daqList[0], SettingsDoc, "duration", ref strTemp) && (Double.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
            {
                SamplingDuration = dVal;
            }

            if (GetAttribute(daqList[0], SettingsDoc, "StimulusLimit", ref strTemp) && (Double.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
            {
                StimulusLimit = dVal;
            }

            XmlNodeList chanList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/DataChannel[@group='/CI']");
            IsCounterLinePlotEnabled.Clear();

            for (int i = 0; i < chanList.Count; i++)
            {
                if (GetAttribute(chanList[0], SettingsDoc, "plot", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
                {
                    _isCounterLinePlotEnabled.Add((1 == iVal) ? true : false);
                }
            }
            OnPropertyChanged("IsCounterLinePlotEnabled");

            chanList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/DataChannel");
            AllDataChannel.Clear();
            EnabledDataChannel.Clear();

            for (int i = 0; i < chanList.Count; i++)
            {
                if (GetAttribute(chanList[i], SettingsDoc, "alias", ref strTemp))
                {
                    _allDataChannel.Add(strTemp);
                }
                if (GetAttribute(chanList[i], SettingsDoc, "enable", ref strTemp1) && (Int32.TryParse(strTemp1, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)) && (1 == iVal))
                {
                    _enabledDataChannel.Add(strTemp);
                }
            }
            OnPropertyChanged("AllDataChannel");
            OnPropertyChanged("EnabledDataChannel");

            //virtual channels:
            chanList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/VirtualChannel");
            AllVirtualChannel.Clear();
            EnabledVirtualChannel.Clear();

            for (int i = 0; i < chanList.Count; i++)
            {
                if (GetAttribute(chanList[i], SettingsDoc, "alias", ref strTemp))
                {
                    _allVirtualChannel.Add(strTemp);
                }
                if (GetAttribute(chanList[i], SettingsDoc, "enable", ref strTemp1) && (Int32.TryParse(strTemp1, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)) && (1 == iVal))
                {
                    _enabledVirtualChannel.Add(strTemp);
                }
            }
            OnPropertyChanged("AllVirtualChannel");
            OnPropertyChanged("EnabledVirtualChannel");

            XmlNodeList rateList = daqList[0].SelectNodes("SampleRate");

            SampleRateList.Clear();
            _sampleRateValue.Clear();

            foreach (XmlNode node in rateList)
            {
                if (GetAttribute(node, SettingsDoc, "name", ref strTemp))
                {
                    SampleRateList.Add(strTemp);
                }
                if (GetAttribute(node, SettingsDoc, "rate", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
                {
                    _sampleRateValue.Add(iVal);
                }
            }

            for (int i = 0; i < rateList.Count; i++)
            {
                if (GetAttribute(rateList[i], SettingsDoc, "enable", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)) && (1 == iVal))
                {
                    SampleRate = i;
                }
            }

            if (SampleRate < 0)
            {
                SampleRate = 0;
            }

            XmlNodeList DisplayList = daqList[0].SelectNodes("Display");

            DisplayOptionList.Clear();
            _displayParamValue.Clear();

            foreach (XmlNode node in DisplayList)
            {
                if (GetAttribute(node, SettingsDoc, "resolution", ref strTemp))
                {
                    switch (strTemp)
                    {
                        case "High":
                            _displayParamValue.Add(Convert.ToInt32(_sampleRateValue[SampleRate] / (Math.Pow(10, 4)), CultureInfo.InvariantCulture));
                            break;
                        case "Medium":
                            _displayParamValue.Add(Convert.ToInt32(_sampleRateValue[SampleRate] / (Math.Pow(10, 3)), CultureInfo.InvariantCulture));
                            break;
                        case "Low":
                            _displayParamValue.Add(Convert.ToInt32(_sampleRateValue[SampleRate] / (Math.Pow(10, 2)), CultureInfo.InvariantCulture));
                            break;
                    }
                    DisplayOptionList.Add(strTemp);
                }
            }

            for (int i = 0; i < DisplayList.Count; i++)
            {
                if (GetAttribute(DisplayList[i], SettingsDoc, "enable", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)) && (1 == iVal))
                {
                    DisplayOptionSelectedIndex = i;
                }
            }
            OnPropertyChanged("SampleRate");
            OnPropertyChanged("DisplayOptionList");

            //Bleach params:
            daqList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]");
            if (daqList.Count < 0)
            { return; }

            XmlNodeList BleachList = daqList[0].SelectNodes("Bleach");
            if (BleachList.Count > 0)
            {
                foreach (XmlNode node in BleachList)
                {
                    if (GetAttribute(daqList[0], SettingsDoc, "bleachTrigMode", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
                    {
                        TriggerBleachMode = iVal;
                    }
                    if (GetAttribute(node, SettingsDoc, "pmtCloseTime", ref strTemp) && (Double.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                    {
                        ClosingPMTShutterDuration = dVal;
                    }
                    if (GetAttribute(node, SettingsDoc, "bleachTime", ref strTemp) && (Double.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                    {
                        BleachTime = dVal;
                    }
                    if (GetAttribute(node, SettingsDoc, "bleachIdleTime", ref strTemp) && (Double.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                    {
                        BleachIdleTime = dVal;
                    }
                    if (GetAttribute(node, SettingsDoc, "bleachIteration", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
                    {
                        BleachIteration = iVal;
                    }
                    if (GetAttribute(node, SettingsDoc, "outDelayTime", ref strTemp) && (Double.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                    {
                        DelayTime = dVal;
                    }
                    if (GetAttribute(node, SettingsDoc, "cycle", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
                    {
                        BleachCycle = iVal;
                    }
                    if (GetAttribute(node, SettingsDoc, "interval", ref strTemp) && (Double.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                    {
                        BleachCycleInterval = dVal;
                    }
                }
            }

            //spectral channels:
            daqList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SpectralDomain");
            if (0 < daqList.Count)
            {
                if (GetAttribute(daqList[0], SettingsDoc, "liveSampleSec", ref strTemp) && (Double.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                {
                    LiveSampleSec = dVal;
                }
                if (GetAttribute(daqList[0], SettingsDoc, "freqMin", ref strTemp) && (Double.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                {
                    FreqMin = dVal;
                }
                if (GetAttribute(daqList[0], SettingsDoc, "freqMax", ref strTemp) && (Double.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                {
                    FreqMax = dVal;
                }
                if (GetAttribute(daqList[0], SettingsDoc, "sampleMinSec", ref strTemp) && (Double.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                {
                    FreqSampleSecMin = dVal;
                }
                if (GetAttribute(daqList[0], SettingsDoc, "sampleMaxSec", ref strTemp) && (Double.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                {
                    FreqSampleSecMax = dVal;
                }
                if (GetAttribute(daqList[0], SettingsDoc, "freqAvgNum", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
                {
                    FreqAverageNum = iVal;
                }
                if (GetAttribute(daqList[0], SettingsDoc, "freqAvgMode", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
                {
                    FreqAverageMode = iVal;
                }
                if (GetAttribute(daqList[0], SettingsDoc, "blockNum", ref strTemp) && (Int32.TryParse(strTemp, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
                {
                    Freqblock = iVal;
                }
            }

            chanList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SpectralDomain/SpectralChannel");
            AllSpectralChannel.Clear();
            AllSpectralPhysicalChannel.Clear();
            EnabledSpectralChannel.Clear();
            for (int i = 0; i < chanList.Count; i++)
            {
                if (GetAttribute(chanList[i], SettingsDoc, "physicalChannel", ref strTemp))
                {
                    _allSpectralPhysicalChannel.Add(strTemp);
                }
                if (GetAttribute(chanList[i], SettingsDoc, "alias", ref strTemp))
                {
                    _allSpectralChannel.Add(strTemp);
                }
                if (GetAttribute(chanList[i], SettingsDoc, "enable", ref strTemp1) && (Int32.TryParse(strTemp1, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)) && (1 == iVal))
                {
                    _enabledSpectralChannel.Add(strTemp);
                }
            }
            OnPropertyChanged("AllSpectralChannel");
            OnPropertyChanged("AllSpectralPhysicalChannel");
            OnPropertyChanged("EnabledSpectralChannel");

            chanList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SpectralDomain/VirtualChannel[@enable=1]");
            EnabledSpectralVirtualChannel.Clear();
            for (int i = 0; i < chanList.Count; i++)
            {
                if (GetAttribute(chanList[i], SettingsDoc, "alias", ref strTemp))
                {
                    _enabledSpectralVirtualChannel.Add(strTemp);
                }
            }
            OnPropertyChanged("EnabledSpectralVirtualChannel");

            _isLoaded = true;
        }

        /// <summary>
        /// Loads the file.
        /// </summary>
        public void LoadFile()
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.DefaultExt = ".h5";
            dlg.Filter = "HDF5 Files (*.h5)|*.h5|Text Files(*.txt)|*.txt";

            Nullable<bool> result = dlg.ShowDialog();
            if (result == true)
            {
                FilePath = dlg.FileName;
                LoadDataFile();
            }
        }

        /// <summary>
        /// Loads the most recent file.
        /// </summary>
        public void LoadMostRecentFile()
        {
            //Get most recent path and name:
            string tmpPath = _mostRecentInfo.mostRecentPath + "\\" + _mostRecentInfo.mostRecentName;
            try
            {
                string[] fnames = Directory.GetFiles(tmpPath, "*.h5", SearchOption.AllDirectories);
                if (fnames.Length > 0)
                {
                    FilePath = fnames[fnames.Length - 1];
                    LoadDataFile();
                }
            }
            catch (Exception)
            {
                MessageBox.Show("Path is not valid: " + tmpPath, "Load File Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        /// <summary>
        /// Samplings the settings options.
        /// </summary>
        public void SamplingSettingsOptions()
        {
            EditSamplingDialog samplingDialog = new EditSamplingDialog();
            samplingDialog.DataContext = this;
            samplingDialog.ShowDialog();
        }

        public void SaveChartVisibilities()
        {
            try
            {
                string str = string.Empty;

                //data channel visibilities
                XmlNodeList ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/DataChannel[@enable='1']");

                for (int i = 0; i < ndList.Count; i++)
                {
                    GetAttribute(ndList[i], SettingsDoc, "alias", ref str);

                    if (_isStackedDisplay)
                    {
                        var series = _channelViewModels.FirstOrDefault(x => x.ChannelName == str);
                        if (series != null)
                        {
                            SetAttribute(ndList[i], SettingsDoc, "visible", series.IsVisible ? "1" : "0");
                            SetAttribute(ndList[i], SettingsDoc, "yLock", series.YVisibleLock ? "1" : "0");
                        }
                    }
                    else
                    {
                        var series = _chartSeries.FirstOrDefault(x => x.DataSeries.SeriesName == str);
                        if (series != null)
                        {
                            SetAttribute(ndList[i], SettingsDoc, "visible", series.IsVisible ? "1" : "0");
                            SetAttribute(ndList[i], SettingsDoc, "yLock", "0");
                        }
                    }
                }
                //virtual channel visibilities
                ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/VirtualChannel[@enable='1']");
                for (int i = 0; i < ndList.Count; i++)
                {
                    GetAttribute(ndList[i], SettingsDoc, "alias", ref str);

                    if (_isStackedDisplay)
                    {
                        SetAttribute(ndList[i], SettingsDoc, "visible", ((_channelViewModels.FirstOrDefault(x => x.ChannelName == str).IsVisible) ? "1" : "0"));
                        SetAttribute(ndList[i], SettingsDoc, "yLock", (_channelViewModels.FirstOrDefault(x => x.ChannelName == str).YVisibleLock ? "1" : "0"));
                    }
                    else
                    {
                        SetAttribute(ndList[i], SettingsDoc, "visible", ((_chartSeries.FirstOrDefault(x => x.DataSeries.SeriesName == str).IsVisible) ? "1" : "0"));
                        SetAttribute(ndList[i], SettingsDoc, "yLock", "0");
                    }
                }
                //spectral channel visibilities
                ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SpectralDomain/SpectralChannel[@enable='1']");
                for (int i = 0; i < ndList.Count; i++)
                {
                    GetAttribute(ndList[i], SettingsDoc, "alias", ref str);
                    SetAttribute(ndList[i], SettingsDoc, "visible", ((_specChViewModels.FirstOrDefault(x => x.ChannelName == str).IsVisible) ? "1" : "0"));
                }
                //spectral virtual channel visibilities
                ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SpectralDomain/VirtualChannel[@enable='1']");
                for (int i = 0; i < ndList.Count; i++)
                {
                    //skip fitting lines since they will be displayed with spectral channel
                    if (GetAttribute(ndList[i], SettingsDoc, "physicalChannel", ref str) &&
                        (str.Contains(Constants.ThorRealTimeData.LORENTZIANFITX) || str.Contains(Constants.ThorRealTimeData.LORENTZIANFITY)))
                        continue;

                    GetAttribute(ndList[i], SettingsDoc, "alias", ref str);
                    SetAttribute(ndList[i], SettingsDoc, "visible", ((_specChViewModels.FirstOrDefault(x => x.ChannelName == str).IsVisible) ? "1" : "0"));
                }

                //Do Save & Copy:
                SettingsDoc.Save(Constants.ThorRealTimeData.SETTINGS_FILE_NAME);
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart SaveChartVisibilities Error: " + ex.Message);
            }
        }

        /// <summary>
        /// Update the XML document with the latest settings.
        /// </summary>
        public void SaveDocumentSettings()
        {
            bool tempSwitchCI = false;
            System.Globalization.CultureInfo originalCultureInfo = (System.Globalization.CultureInfo)System.Threading.Thread.CurrentThread.CurrentCulture.Clone();
            string str = string.Empty;
            try
            {
                //Keep decimal dot in xml:
                if (0 == originalCultureInfo.NumberFormat.NumberDecimalSeparator.CompareTo(","))
                {
                    originalCultureInfo.NumberFormat.NumberDecimalSeparator = ".";
                    System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;
                    tempSwitchCI = true;
                }

                XmlNodeList ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/UserSettings/SciChart");

                if (ndList.Count > 0)
                {
                    SetAttribute(ndList[0], SettingsDoc, "fifoSize", _fifoSize.ToString());
                    SetAttribute(ndList[0], SettingsDoc, "IsStackedDisplay", ((IsStackedDisplay) ? "1" : "0"));
                    string height = (0 < ChannelViewModels.Count) ? ChannelViewModels[0].Height.ToString() : "200";
                    SetAttribute(ndList[0], SettingsDoc, "TimeStackHeight", height);
                    height = (0 < SpecChViewModels.Count) ? SpecChViewModels[0].Height.ToString() : "250";
                    SetAttribute(ndList[0], SettingsDoc, "SpecStackHeight", height);
                    SetAttribute(ndList[0], SettingsDoc, "SpecPanelHeight", SpecPanelHeight.ToString());
                    int statsPanelEnable = StatsPanelEnable ? 1 : 0;
                    SetAttribute(ndList[0], SettingsDoc, "StatsPanelEnable", statsPanelEnable.ToString());

                }

                ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/UserSettings/Save");

                if (ndList.Count > 0)
                {
                    SetAttribute(ndList[0], SettingsDoc, "name", SaveName);
                    SetAttribute(ndList[0], SettingsDoc, "path", SavePath);
                    SetAttribute(ndList[0], SettingsDoc, "enable", (true == IsSaving) ? "1" : "0");
                    SetAttribute(ndList[0], SettingsDoc, "CallbackSec", ThreadCallbackTime.ToString("N" + 2));
                    SetAttribute(ndList[0], SettingsDoc, "DontShowAgain", (true == RenameSavingFileDontShowAgain) ? "1" : "0");

                    //update most recent path and name:
                    if (true == IsSaving)
                    {
                        _mostRecentInfo.mostRecentPath = SavePath;
                        _mostRecentInfo.mostRecentName = SaveName;
                    }
                }

                ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/UserSettings/Configuration");
                if (0 >= ndList.Count)
                {
                    ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/UserSettings");
                    RealTimeLineChartViewModel.CreateXmlNode(SettingsDoc, "Configuration", ndList[0]);
                    ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/UserSettings/Configuration");
                    SetAttribute(ndList[0], SettingsDoc, "OTM", "0");   //not configurable by user
                }
                BitVector32 vec = new BitVector32();
                for (int i = 0; i < _visibilityCollection.Count; i++)
                {
                    vec[((2 > i) ? BitVector32.CreateMask(i) : (1 << i))] = _visibilityCollection[i];
                }
                SetAttribute(ndList[0], SettingsDoc, "VisibleChoices", vec.Data.ToString());

                //update variables from RealTimeProvider
                XmlNodeList ndListDoc = RealTimeProvider.Document.SelectNodes("/RealTimeDataSettings/Variables/Var");
                ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/Variables/Var");
                if ((0 < ndListDoc.Count) && (0 < ndList.Count) && (ndListDoc.Count == ndList.Count))
                {
                    for (int i = 0; i < ndListDoc.Count; i++)
                    {
                        if (GetAttribute(ndListDoc[i], RealTimeProvider.Document, "ID", ref str))
                        {
                            SetAttribute(ndList[i], SettingsDoc, "ID", str);
                        }
                        if (GetAttribute(ndListDoc[i], RealTimeProvider.Document, "Value", ref str))
                        {
                            SetAttribute(ndList[i], SettingsDoc, "Value", str);
                        }
                        if (GetAttribute(ndListDoc[i], RealTimeProvider.Document, "Name", ref str))
                        {
                            SetAttribute(ndList[i], SettingsDoc, "Name", str);
                        }
                    }
                }

                ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]");
                if (ndList.Count > 0)
                {
                    SetAttribute(ndList[0], SettingsDoc, "hwTrigMode", TriggerMode.ToString());
                    SetAttribute(ndList[0], SettingsDoc, "duration", SamplingDuration.ToString());
                    SetAttribute(ndList[0], SettingsDoc, "StimulusLimit", StimulusLimit.ToString());
                }

                //do not change channel settings in simulator file mode
                if (0 >= SimulatorFilePath.Length)
                {
                    //counter
                    ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/DataChannel[@group='/CI']");

                    for (int i = 0; i < ndList.Count; i++)
                    {
                        if (0 < IsCounterLinePlotEnabled.Count)
                        {
                            SetAttribute(ndList[i], SettingsDoc, "plot", (IsCounterLinePlotEnabled[i] ? "1" : "0"));
                        }
                    }

                    XmlNodeList ndSRList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SampleRate");

                    if (ndSRList.Count > 0)
                    {

                        //clear the enable flags for the sample rates
                        foreach (XmlNode node in ndSRList)
                        {
                            SetAttribute(node, SettingsDoc, "enable", "0");
                        }

                        //set the single sample rate enable flag
                        SetAttribute(ndSRList[SampleRate], SettingsDoc, "enable", "1");
                    }

                    XmlNodeList ndDisplayList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/Display");

                    if (ndDisplayList.Count > 0)
                    {
                        //clear the enable flags for the sample rates
                        foreach (XmlNode node in ndDisplayList)
                        {
                            SetAttribute(node, SettingsDoc, "enable", "0");
                            GetAttribute(node, SettingsDoc, "resolution", ref str);
                            int idx;
                            switch (str)
                            {
                                case "High":
                                    idx = _displayParamValue.Count - 3;
                                    if (idx >= 0)
                                        SetAttribute(node, SettingsDoc, "value", _displayParamValue[idx].ToString());
                                    break;
                                case "Medium":
                                    idx = _displayParamValue.Count - 2;
                                    if (idx >= 0)
                                        SetAttribute(node, SettingsDoc, "value", _displayParamValue[idx].ToString());
                                    break;
                                case "Low":
                                    idx = _displayParamValue.Count - 1;
                                    if (idx >= 0)
                                        SetAttribute(node, SettingsDoc, "value", _displayParamValue[idx].ToString());
                                    break;
                            }
                        }

                        //set the single sample rate enable flag
                        SetAttribute(ndDisplayList[DisplayOptionSelectedIndex], SettingsDoc, "enable", "1");
                    }

                    //Bleach:
                    ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/Bleach");
                    if (ndList.Count > 0)
                    {
                        SetAttribute(ndList[0], SettingsDoc, "bleachTrigMode", TriggerBleachMode.ToString());
                        SetAttribute(ndList[0], SettingsDoc, "pmtCloseTime", ClosingPMTShutterDuration.ToString());
                        SetAttribute(ndList[0], SettingsDoc, "bleachTime", BleachTime.ToString());
                        SetAttribute(ndList[0], SettingsDoc, "bleachIdleTime", BleachIdleTime.ToString());
                        SetAttribute(ndList[0], SettingsDoc, "bleachIteration", BleachIteration.ToString());
                        SetAttribute(ndList[0], SettingsDoc, "outDelayTime", DelayTime.ToString());
                        SetAttribute(ndList[0], SettingsDoc, "cycle", BleachCycle.ToString());
                        SetAttribute(ndList[0], SettingsDoc, "interval", BleachCycleInterval.ToString());
                    }

                    //virtual channel: verify physicalChannels are acquired, disable otherwise
                    ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/VirtualChannel[@enable='1']");
                    for (int i = 0; i < ndList.Count; i++)
                    {
                        //verify physicalChannels are acquired, disable otherwise
                        GetAttribute(ndList[i], SettingsDoc, "physicalChannel", ref str);

                        for (int j = 0; j < _allDataChannel.Count; j++)
                        {
                            if (str.Contains(_allDataChannel[j]) && !_enabledDataChannel.Contains(_allDataChannel[j]))
                            {
                                SetAttribute(ndList[i], SettingsDoc, "enable", "0");
                                SetAttribute(ndList[i], SettingsDoc, "visible", "0");
                                break;
                            }
                        }
                    }

                    //spectral channel
                    ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SpectralDomain");
                    if (0 >= ndList.Count)
                    {
                        ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]");
                        RealTimeLineChartViewModel.CreateXmlNode(SettingsDoc, "SpectralDomain", ndList[0]);
                        ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SpectralDomain");
                    }
                    if (ndList.Count > 0)
                    {
                        SetAttribute(ndList[0], SettingsDoc, "liveSampleSec", LiveSampleSec.ToString());
                        SetAttribute(ndList[0], SettingsDoc, "freqMin", FreqMin.ToString());
                        SetAttribute(ndList[0], SettingsDoc, "freqMax", FreqMax.ToString());
                        SetAttribute(ndList[0], SettingsDoc, "sampleMinSec", FreqSampleSecMin.ToString());
                        SetAttribute(ndList[0], SettingsDoc, "sampleMaxSec", FreqSampleSecMax.ToString());
                        SetAttribute(ndList[0], SettingsDoc, "freqAvgNum", FreqAverageNum.ToString());
                        SetAttribute(ndList[0], SettingsDoc, "freqAvgMode", FreqAverageMode.ToString());
                        SetAttribute(ndList[0], SettingsDoc, "blockNum", Freqblock.ToString());
                    }
                    //spectral channel: verify physicalChannels are acquired, disable otherwise
                    ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SpectralDomain/SpectralChannel[@enable='1']");
                    for (int i = 0; i < ndList.Count; i++)
                    {
                        //verify physicalChannels are acquired, disable otherwise
                        GetAttribute(ndList[i], SettingsDoc, "physicalChannel", ref str);

                        for (int j = 0; j < _allDataChannel.Count; j++)
                        {
                            if (str.Contains(_allDataChannel[j]) && !_enabledDataChannel.Contains(_allDataChannel[j]))
                            {
                                SetAttribute(ndList[i], SettingsDoc, "enable", "0");
                                SetAttribute(ndList[i], SettingsDoc, "visible", "0");
                                break;
                            }
                        }
                        for (int j = 0; j < _allVirtualChannel.Count; j++)
                        {
                            if (str.Contains(_allVirtualChannel[j]) && !_enabledVirtualChannel.Contains(_allVirtualChannel[j]))
                            {
                                SetAttribute(ndList[i], SettingsDoc, "enable", "0");
                                SetAttribute(ndList[i], SettingsDoc, "visible", "0");
                                break;
                            }
                        }
                    }
                    //spectral virtual channel: verify physicalChannels are acquired, disable otherwise
                    ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SpectralDomain/VirtualChannel[@enable='1']");
                    for (int i = 0; i < ndList.Count; i++)
                    {
                        //verify physicalChannels are acquired, disable otherwise
                        GetAttribute(ndList[i], SettingsDoc, "physicalChannel", ref str);

                        for (int j = 0; j < _allSpectralChannel.Count; j++)
                        {
                            if ((str.Contains(_allSpectralChannel[j]) && !_enabledSpectralChannel.Contains(_allSpectralChannel[j])))
                            {
                                SetAttribute(ndList[i], SettingsDoc, "enable", "0");
                                SetAttribute(ndList[i], SettingsDoc, "visible", "0");
                                break;
                            }
                        }
                    }
                }

                //Do Save & Copy:
                SettingsDoc.Save(Constants.ThorRealTimeData.SETTINGS_FILE_NAME);

            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart SaveDocumentSettings Error: " + ex.Message);
                MessageBox.Show("Error occured at saving settings.\nSome of your settings may not be saved.");
            }

            //give back CultureInfo:
            if (tempSwitchCI)
            {
                originalCultureInfo.NumberFormat.NumberDecimalSeparator = ",";
                System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;
            }
        }

        /// <summary>
        /// Save global variables into current episode and active settings
        /// </summary>
        public void SaveGlobalSettings()
        {
            ObservableCollection<string> settingFiles = new ObservableCollection<string>();
            settingFiles.Add(Constants.ThorRealTimeData.SETTINGS_FILE_NAME);
            if (0 < FilePath.Length)
            {
                if (File.Exists(Path.GetDirectoryName(FilePath) + "\\" + Constants.ThorRealTimeData.SETTINGS_FILE_NAME))
                    settingFiles.Add(Path.GetDirectoryName(FilePath) + "\\" + Constants.ThorRealTimeData.SETTINGS_FILE_NAME);
            }
            foreach (string file in settingFiles)
            {
                XmlDocument xDoc = new XmlDocument();
                xDoc.Load(file);

                XmlNodeList ndList = xDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SpectralDomain");
                if (ndList.Count > 0)
                {
                    SetAttribute(ndList[0], xDoc, "liveSampleSec", LiveSampleSec.ToString());
                    SetAttribute(ndList[0], xDoc, "freqMin", FreqMin.ToString());
                    SetAttribute(ndList[0], xDoc, "freqMax", FreqMax.ToString());
                    SetAttribute(ndList[0], xDoc, "sampleMinSec", FreqSampleSecMin.ToString());
                    SetAttribute(ndList[0], xDoc, "sampleMaxSec", FreqSampleSecMax.ToString());
                    SetAttribute(ndList[0], xDoc, "freqAvgNum", FreqAverageNum.ToString());
                    SetAttribute(ndList[0], xDoc, "freqAvgMode", FreqAverageMode.ToString());
                    SetAttribute(ndList[0], xDoc, "blockNum", Freqblock.ToString());
                }

                xDoc.Save(file);
            }
        }

        /// <summary>
        /// Selects the vertical marker.
        /// </summary>
        /// <param name="index">The index.</param>
        public void SelectVerticalMarker(int index)
        {
            if (null != EventVerticalMarkerSelectedIndexChanged)
            {
                EventVerticalMarkerSelectedIndexChanged(index);
            }
        }

        /// <summary>
        /// Starts the capturing.
        /// </summary>
        public void StartCapturing(bool forceSave = false)
        {
            if (_bw.IsBusy)
                return;
            if (ThorImageLSConnectionStats)
                SendToClient(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.IsSaving), IsSaving ? "1" : "0");

            H5CSWrapper tmpH5 = new H5CSWrapper();
            string str = SaveName;
            string strFileName = "Episode_0000.h5";
            if (IsSaving || forceSave)
            {
                if (str == null)
                {
                    MessageBox.Show("Folder name is not specified.");
                    return;
                }

                if (Directory.Exists(SavePath + "\\" + SaveName))
                {
                    do
                    {
                        str = CreateUniqueFilename(SavePath, str);
                    }
                    while (Directory.Exists(SavePath + "\\" + str));

                    if ((!RenameSavingFileDontShowAgain) && (0 == SimulatorFilePath.Length))    //always create folder if simulator file mode
                    {
                        string msg = string.Format("A data folder already exists at this location.Do you want to use the name {0} instead? Click Yes to change, No to create another file in the same folder, or Close the dialog to cancel the action.", str);

                        CustomMessageBox.CustomMessageBox mBox = new CustomMessageBox.CustomMessageBox(msg, "Data already exists", "Don't ask again", "Yes", "No");
                        mBox.ShowDialog();
                        RenameSavingFileDontShowAgain = mBox.CheckBoxChecked;
                        if (-1 == mBox.ButtonFlag)
                        {
                            //do not modify the path. Allow the user to save into the existing path
                            str = SaveName;

                            if (File.Exists(SavePath + "\\" + str + "\\" + strFileName))
                            {
                                do
                                {
                                    strFileName = CreateUniqueFilename(SavePath + "\\" + str + "\\", strFileName);

                                }
                                while (File.Exists(SavePath + "\\" + str + "\\" + strFileName));
                            }
                        }
                        else if (0 == mBox.ButtonFlag)
                        {
                            return;
                        }
                    }
                }
                SaveName = str;
                string pathAndFile = SavePath + "\\" + str + "\\" + strFileName;
                string path = SavePath + "\\" + str;
                Directory.CreateDirectory(path);

                //update settings before copy:
                SaveDocumentSettings();

                //persist selections of chart:
                SaveChartVisibilities();

                FileCopy(Constants.ThorRealTimeData.SETTINGS_FILE_NAME, path + "\\" + Constants.ThorRealTimeData.SETTINGS_FILE_NAME);
                FileCopy(Constants.ThorRealTimeData.OTM_SETTINGS_FILE, path + "\\" + Constants.ThorRealTimeData.OTM_SETTINGS_FILE);

                if ((DatabaseFile != null) && (DatabaseFile.Length <= 0))
                {
                    DataStore.Instance.ConnectionString = DatabaseFile;
                    DataStore.Instance.Open();
                    DataStore.Instance.AddEpisode(Path.GetFileNameWithoutExtension(strFileName), SavePath, SaveName);
                    DataStore.Instance.Close();
                }
                tmpH5.SetSavePathAndFileName(pathAndFile);
            }
            else
            {
                //avoid overwriting last saved file:
                tmpH5.SetSavePathAndFileName("");
                SaveDocumentSettings();
                SaveChartVisibilities();
            }
            //verify settings before start:
            string verifyResult = VerifyDocumentSettings();
            if (verifyResult != "Success")
            {
                MessageBox.Show(verifyResult, "Line Settings Error");
                return;
            }
            ChangeVerticalMarkersMode(MarkerType.DeleteAll);
            //If connect to ThorImage, Send the folder Name to ThorImage
            if (_receiveIPCCommandActive == false && ThorImageLSConnectionStats == true && (IsSaving || forceSave) == true)
            {
                StartAquiring(SavePath + "\\" + SaveName);
            }
            //Reset ChartSeries:
            CreateChartLines();

            if (RealTimeDataCapture.Instance.StartAcquire())
            {
                _readCaptureTimer.Start();
                _bw.WorkerSupportsCancellation = true;
                _lastGlobalCounter = 0;
                _bw.RunWorkerAsync();
            }

            //_thorImageLSConnectionStats = false;
            if (IsSaving && _thorImageLSConnectionStats == true)
                SendToClient(Enum.GetName(typeof(ThorPipeCommand), ThorPipeCommand.FilePath), SavePath + "\\" + SaveName + "\\" + strFileName);

            ImageCounterNumber = 0;
            OnPropertyChanged("CapturingIcon");
            OnPropertyChanged("IsCapturingStopped");
            OnPropertyChanged("AutoRangeX");
            OnPropertyChanged("AutoRangeY");
        }

        public void StopCaptureWorkers()
        {
            if (_readCaptureTimer.IsEnabled)
            {
                _readCaptureTimer.Stop();
            }
            if (_bw.IsBusy)
            {
                _bw.CancelAsync();
            }
            if ((_bwLoader.IsBusy) || (_bwSpecAnalyzer.IsBusy))
            {
                StopLoad();
            }
        }

        /// <summary>
        /// Stops the capturing.
        /// </summary>
        public void StopCapturing(bool forceSave = false, bool sendIPCStop = true)
        {
            if (true == IsCapturing)
            {
                //_readCaptureTimer.Stop();
                //_bw.CancelAsync();
                if (null != _bwWaiter)
                {
                    if (_bwWaiter.IsBusy)
                        return;
                }
                //background worker to update progress, wait until task is done:
                BackgroundWorker splashWkr = new BackgroundWorker();
                splashWkr.WorkerSupportsCancellation = true;

                try
                {
                    _splash = new SplashProgress();
                    _splash.DisplayText = "Please wait ...";
                    _splash.DisplayProgress = false;
                    _splash.DisplayCancel = false;
                    _splash.ShowInTaskbar = false;
                    _splash.Owner = Application.Current.MainWindow;
                    _splash.Show();
                    EnableUIPanel(false);

                    _bwWaiter = new BackgroundWorker();
                    _bwWaiter.WorkerSupportsCancellation = true;
                    _bwWaiter.DoWork += delegate (object sender, DoWorkEventArgs e)
                    {
                        do
                        {
                            RealTimeDataCapture.Instance.StopAcquire(); //this will hold until done ...
                            OnPropertyChanged("IsCapturingStopped");
                        }
                        while (_bw.IsBusy);
                    };

                    _bwWaiter.RunWorkerCompleted += delegate (object sender, RunWorkerCompletedEventArgs e)
                    {
                        if (IsSaving || forceSave)
                        {
                            ChangeVerticalMarkersMode(MarkerType.Save);
                        }
                        if ((ThorImageLSConnectionStats == true) && sendIPCStop)
                        {
                            StopAquiring();
                        }
                        OnPropertyChanged("CapturingIcon");
                        OnPropertyChanged("AutoRangeX");
                        OnPropertyChanged("AutoRangeY");
                    };

                    splashWkr.DoWork += delegate (object sender, DoWorkEventArgs e)
                    {
                        do
                        {
                            if (splashWkr.CancellationPending == true)
                            {
                                e.Cancel = true;
                                break;
                            }
                            //wait for loading progress:
                            System.Threading.Thread.Sleep(10);

                        }
                        while (true == _bwWaiter.IsBusy);
                    };

                    splashWkr.RunWorkerCompleted += delegate (object sender, RunWorkerCompletedEventArgs e)
                    {
                        _splash.Close();
                        // App inherits from Application, and has a Window property called MainWindow
                        // and a List<Window> property called OpenWindows.
                        Application.Current.MainWindow.Activate();

                        EnableUIPanel(true);
                    };

                    _bwWaiter.RunWorkerAsync();
                    splashWkr.RunWorkerAsync();

                }
                catch (Exception ex)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart Error: " + ex.Message);
                    _bwWaiter.CancelAsync();
                    splashWkr.CancelAsync();
                }
            }
        }

        public void StopLoad()
        {
            RealTimeDataCapture.Instance.StopLoading();

            if (_bwLoader.IsBusy)
            {
                _bwLoader.CancelAsync();
            }
            if (_bwSpecAnalyzer.IsBusy)
            {
                _bwSpecAnalyzer.CancelAsync();
            }
        }

        public void UpdateCharts()
        {
            if (false == IsStackedDisplay)
            {
                //single surface
                OnPropertyChanged("ChartSeries");
            }
            else
            {
                //stack view in time domain
                _channelViewModels.Each(s => { s.Update(); });
                OnPropertyChanged("ChannelViewModels");
            }
        }

        /// <summary>
        /// this is the method that the UpdateProgressDelegate will execute
        /// </summary>
        /// <param name="percentage"></param>
        public void UpdateProgressText(int percentage)
        {
            //set our progress dialog text and value
            _splash.ProgressText = string.Format("{0}%", percentage.ToString());
            _splash.ProgressValue = percentage;
        }

        /// <summary>
        /// update stack view in freq domain
        /// </summary>
        public void UpdateSpectralCharts(bool resetRange = true)
        {
            if (ChartModes.REVIEW == _chartMode)
            {
                if (resetRange)
                {
                    _specChViewModels.Each(s => { s.XVisibleRange = s.ChannelSeries.XRange; });
                }
                else
                {
                    _specChViewModels.Each(s =>
                    {
                        s.XVisibleRange = new DoubleRange(Math.Max((double)s.ChannelSeries.XRange.Min, (double)s.XVisibleRange.Min),
                            Math.Min((double)s.ChannelSeries.XRange.Max, (double)s.XVisibleRange.Max));
                    });
                }
            }

            _specChViewModels.Each(s => { s.Update(); });
            OnPropertyChanged("SpecChViewModels");
        }

        /// <summary>
        /// Verify user settings before start acquisition.
        /// </summary>
        public string VerifyDocumentSettings()
        {
            string retStr = "Success";
            XmlDocument doc = new XmlDocument();

            string docPath = Constants.ThorRealTimeData.SETTINGS_FILE_NAME;

            doc.Load(docPath);

            XmlNodeList daqList = doc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]");
            if (daqList.Count == 0)
            {
                return retStr = "No active board. ";
            }
            XmlNodeList sampleList = daqList[0].SelectNodes("SampleRate[@enable=1]");
            XmlNodeList aiList = daqList[0].SelectNodes("DataChannel[@enable=1 and @signalType=0]");
            XmlNodeList diList = daqList[0].SelectNodes("DataChannel[@enable=1 and @signalType=1]");
            XmlNodeList ciList = daqList[0].SelectNodes("DataChannel[@enable=1 and @signalType=2]");
            if (daqList.Count <= 0 || sampleList.Count <= 0)
                return retStr = "Verify Document Failed";

            //Check total number of channels:
            string totalAI = string.Empty;
            string totalDI = string.Empty;
            if (GetAttribute(daqList[0], doc, "totalAI", ref totalAI) && GetAttribute(daqList[0], doc, "totalDI", ref totalDI))
            {
                if ((aiList.Count != Convert.ToInt32(totalAI)) || (diList.Count != Convert.ToInt32(totalDI)))
                    return retStr = "Inconsistent total channel numbers. ";
            }

            //Check channel numbers vs. sample rate:
            string type = string.Empty;
            int maxSampleRate = 0;

            if ((GetAttribute(daqList[0], doc, "type", ref type)) && (type != "Simulator"))
            {
                if (type == "NI6363" || type == "NI6361" || type == "NI6363-USB")
                {
                    maxSampleRate = 2000002;
                }
                else if (type == "NI6321" || type == "NI6323")
                {
                    maxSampleRate = 250000;
                }
                else if (type == "NI6341" || type == "NI6343")
                {
                    maxSampleRate = 500000;
                }
                if (maxSampleRate == 0)
                    return retStr = "Invalid board type.";

                string rate = string.Empty;
                if (GetAttribute(sampleList[0], doc, "rate", ref rate))
                {
                    int irate = Convert.ToInt32(rate);
                    if ((aiList.Count > 0) && (aiList.Count * irate > maxSampleRate))
                    {
                        return retStr = "Too many analog channels for selected rate.";
                    }
                }
            }

            //Check analog trigger limitation:
            if (type != "Simulator")
            {
                string hwTrigType = string.Empty;
                if (GetAttribute(daqList[0], doc, "hwTrigType", ref hwTrigType) && (hwTrigType == "1"))
                {
                    string duration = string.Empty;
                    if ((ciList.Count > 0) && (GetAttribute(daqList[0], doc, "duration", ref duration)) && duration != "0")
                        return retStr = "Finite counter is not availble under analog HW trigger. ";

                    string hwTrigMode = string.Empty;
                    if ((aiList.Count > 1) && (GetAttribute(daqList[0], doc, "hwTrigMode", ref hwTrigMode)) && hwTrigMode == "2" && duration != "0")  //retriggerable finite mode
                        return retStr = "Only the analog trigger channel is allowed in retriggerable finite mode. ";
                }
            }
            return retStr;
        }

        /// <summary>
        /// Cancels the image loading.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void CancelLoading(object sender, EventArgs e)
        {
            StopLoad();
        }

        /// <summary>
        /// Creates the unique filename.
        /// </summary>
        /// <param name="currentPath">the current save path</param>
        /// <param name="currentFilename">he current filename.</param>
        /// <returns></returns>
        private string CreateUniqueFilename(string currentPath, string currentFilename)
        {
            ThorSharedTypes.FileName name = new ThorSharedTypes.FileName(currentFilename, false);

            name.MakeUnique(currentPath);

            return name.FullName;
        }

        /// <summary>
        /// Gets the min and max values for the enabled channels
        /// </summary>
        /// <param name="minList"></param>
        /// <param name="maxList"></param>
        /// <returns></returns>
        private (double?, double?) GetMinMaxEnabledChannels(List<double> minList, List<double> maxList)
        {
            if (minList?.Count > 0 && maxList?.Count > 0 && ChartSeries?.Count == maxList?.Count && ChartSeries?.Count == minList?.Count)
            {
                double min = double.MaxValue;
                double max = double.MinValue;
                Application.Current.Dispatcher.Invoke(new Action(() =>
                {
                    for (int i = 0; i < ChartSeries?.Count; i++)
                    {
                        bool canCalculateMinMax = false;
                        if (!_bwLoadDone)
                        {
                            if (_channelVisibilityChoice?.Count == ChartSeries?.Count && ChartSeries[i]?.DataSeries?.Count > 0 && _channelVisibilityChoice[i])
                            {
                                canCalculateMinMax = true;
                            }
                        }
                        else
                        {
                            if (IsStackedDisplay &&
                                _channelViewModels?.Count > 0 &&
                                _channelViewModels.Count == ChartSeries?.Count &&
                                _channelViewModels[i].ChannelSeries?.Count > 0 &&
                                _channelViewModels[i].IsVisible)
                            {
                                canCalculateMinMax = true;
                            }
                            else
                            {
                                if (ChartSeries[i]?.DataSeries?.Count > 0 && ChartSeries[i].IsVisible)
                                {
                                    canCalculateMinMax = true;
                                }
                            }
                        }

                        if (canCalculateMinMax)
                        {
                            if (minList[i] < min)
                            {
                                min = minList[i];
                            }
                            if (maxList[i] > max)
                            {
                                max = maxList[i];
                            }
                        }
                    }
                }));

                return (min, max);
            }
            return (null, null);
        }

        /// <summary>
        /// Load H5 file based on DataFile property.
        /// </summary>
        private void LoadDataFile()
        {
            if (null == _hdf5Reader)
            {
                return;
            }
            if (false == _hdf5Reader.SetSavePathAndFileName(FilePath))
            {
                return;
            }
            if (_bwLoader.IsBusy)
            {
                return;
            }

            //import the settings from the folder without overwriting the active settings:
            if (File.Exists(Path.GetDirectoryName(FilePath) + "\\" + Constants.ThorRealTimeData.SETTINGS_FILE_NAME))
            {
                LoadDocumentSettings(Path.GetDirectoryName(FilePath));
                SettingPath = Path.GetDirectoryName(FilePath) + "\\" + Constants.ThorRealTimeData.SETTINGS_FILE_NAME;
                OTMSettingPath = Path.GetDirectoryName(FilePath) + "\\" + Constants.ThorRealTimeData.OTM_SETTINGS_FILE;
                CreateChartLines();
            }
            else
            {
                MessageBox.Show("No Settings File.");
                return;
            }

            //background worker to update progress:
            BackgroundWorker splashWkr = new BackgroundWorker();
            splashWkr.WorkerSupportsCancellation = true;
            _bwLoadDone = false;
            ProgressPercentage = 0;
            EnableScrollbar(!IsStackedDisplay);
            ChangeVerticalMarkersMode(MarkerType.DeleteAll);

            try
            {
                _splash = new SplashProgress();
                _splash.DisplayText = "Please wait while loading episode ...";
                _splash.DisplayProgress = true;
                _splash.ShowInTaskbar = false;
                _splash.Owner = Application.Current.MainWindow;
                _splash.Show();
                EnableUIPanel(false);
                _splash.CancelSplashProgress += new EventHandler(CancelLoading);

                //get dispatcher to update the contents that was created on the UI thread:
                System.Windows.Threading.Dispatcher spDispatcher = _splash.Dispatcher;

                //enable handlers of background worker to load file:
                _bwLoader.DoWork += new DoWorkEventHandler(_bwLoader_DoWork);
                _bwLoader.RunWorkerCompleted += new RunWorkerCompletedEventHandler(_bwLoader_RunWorkerCompleted);
                _bwLoader.WorkerSupportsCancellation = true;
                UpdateRenderPriority(RenderPriority.Low);

                _channelVisibilityChoice = new List<bool>();
                if (_chartMode == ChartModes.REVIEW)
                {
                    for (int i = 0; i < _channelViewModels.Count; ++i)
                    {
                        if (IsStackedDisplay)
                        {
                            _channelVisibilityChoice.Add(_channelViewModels[i].IsVisible);
                        }
                        else
                        {
                            _channelVisibilityChoice.Add(_chartSeries[i].IsVisible);
                        }
                        if (!_displayDataWhileReviewLoading)
                        {
                            _channelViewModels[i].IsVisible = false;
                            _chartSeries[i].IsVisible = false;
                        }
                        else
                        {
                            if (IsStackedDisplay)
                            {
                                _chartSeries[i].IsVisible = false;
                            }
                            else
                            {
                                _channelViewModels[i].IsVisible = false;
                            }
                        }
                    }
                }

                GC.Collect(GC.MaxGeneration, GCCollectionMode.Forced, true);

                _bwLoader.RunWorkerAsync();
                splashWkr.RunWorkerAsync();

                splashWkr.DoWork += delegate (object sender, DoWorkEventArgs e)
                {
                    do
                    {
                        if (splashWkr.CancellationPending == true)
                        {
                            e.Cancel = true;
                            break;
                        }
                        //wait for loading progress:
                        System.Threading.Thread.Sleep(50);

                        //create a new delegate for updating our progress text
                        UpdateProgressDelegate update = new UpdateProgressDelegate(UpdateProgressText);

                        //invoke the dispatcher and pass the percentage
                        spDispatcher.BeginInvoke(update, ProgressPercentage);
                    }
                    while ((true == _bwLoader.IsBusy) || (false == _bwLoadDone));
                };

                splashWkr.RunWorkerCompleted += delegate (object sender, RunWorkerCompletedEventArgs e)
                {
                    _splash.Close();
                    // App inherits from Application, and has a Window property called MainWindow
                    // and a List<Window> property called OpenWindows.
                    Application.Current.MainWindow.Activate();

                    //Release handlers of background loader:
                    _bwLoader.DoWork -= _bwLoader_DoWork;
                    _bwLoader.RunWorkerCompleted -= _bwLoader_RunWorkerCompleted;

                    EnableUIPanel(true);

                    //Invoke spectral analysis after data loading
                    CalculateSpectral(true);
                };
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart Error: " + ex.Message);
                _bwLoader.CancelAsync();
                splashWkr.CancelAsync();
            }
        }

        /// <summary>
        /// Loads the subwindow position.
        /// </summary>
        /// <returns></returns>
        private Point LoadSubWindowPos()
        {
            Point ptTemp = new Point(0, 0);

            XmlNodeList settingsList = SettingsDoc.SelectNodes("/RealTimeDataSettings/UserSettings/SubWindow");

            string strTemp = string.Empty;

            if (settingsList.Count > 0)
            {
                if (GetAttribute(settingsList[0], SettingsDoc, "left", ref strTemp))
                {
                    ptTemp.X = Convert.ToDouble(strTemp, CultureInfo.InvariantCulture);
                }

                if (GetAttribute(settingsList[0], SettingsDoc, "top", ref strTemp))
                {
                    ptTemp.Y = Convert.ToDouble(strTemp, CultureInfo.InvariantCulture);
                }
            }
            return ptTemp;
        }

        /// <summary>
        /// Persists the sub window position.
        /// </summary>
        /// <param name="left">The left.</param>
        /// <param name="top">The top.</param>
        void PersistSubWindowPos(int left, int top)
        {
            try
            {
                XmlNodeList ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/UserSettings/SubWindow");
                if (ndList.Count > 0)
                {
                    SetAttribute(ndList[0], SettingsDoc, "left", left.ToString());
                    SetAttribute(ndList[0], SettingsDoc, "top", top.ToString());
                }
                SettingsDoc.Save(Constants.ThorRealTimeData.SETTINGS_FILE_NAME);
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Verbose, 1, "RealTimeLineChart PersistSubWindowPos error: " + ex.Message);
            }
        }



        /// <summary>
        /// Parse given NodePath to create node in doc xml.
        /// </summary>
        /// <param name="doc"></param>
        /// <param name="NodePath"></param>
        /// <returns></returns>
        private XmlNodeList SetNode(XmlDocument doc, string NodePath)
        {
            XmlNodeList ndList = doc.SelectNodes(NodePath.ToString());
            if (ndList.Count > 0)
            {
                return ndList;
            }
            XmlNodeList pList;
            string parentPath = string.Empty;
            string[] words = NodePath.Split('/');
            for (int i = 1; i < words.Length; i++)
            {
                parentPath += "/";
                parentPath += words[i];
                pList = doc.SelectNodes(parentPath.ToString());
                if (pList.Count <= 0)
                {
                    XmlNode newNode = doc.CreateElement(words[i]);
                    ndList[0].AppendChild(newNode);
                }

                ndList = doc.SelectNodes(parentPath.ToString());
            }
            return ndList = doc.SelectNodes(parentPath.ToString());
        }

        /// <summary>
        /// Sets the save path.
        /// </summary>
        private void SetSavePath()
        {
            BrowseForFolderDialog dlg = new BrowseForFolderDialog();

            dlg.InitialExpandedFolder = SavePath;
            dlg.InitialFolder = SavePath;
            if (true == dlg.ShowDialog())
            {
                SavePath = dlg.SelectedFolder;
            }
        }

        /// <summary>
        /// Updates the GUI.
        /// </summary>
        private void UpdateGUI()
        {
            //UpdateCharts();
            UpdateSpectralCharts();
            //OnPropertyChanged("AutoRangeX");  //do whenever _fifoEnable is updated.
            //OnPropertyChanged("AutoRangeY");
            OnPropertyChanged("SavePath");
            OnPropertyChanged("SaveName");
            OnPropertyChanged("SaveEpisode");
            OnPropertyChanged("TriggerMode");
            OnPropertyChanged("SamplingDuration");
            OnPropertyChanged("SampleRate");
        }

        /// <summary>
        /// Zooms the extend chart series.
        /// </summary>
        private void ZoomExtendChartSeries()
        {
            try //TODO: do Y and X separately
            {
                if (ChartSeries.Count <= 0)
                {
                    return;
                }

                int num = 0;
                Ymax = new List<double> { };
                Ymin = new List<double> { };
                Xmax = new List<double> { };
                Xmin = new List<double> { };
                Application.Current.Dispatcher.Invoke(new Action(() =>
                {
                    for (int i = 0; i < ChartSeries?.Count; i++)
                    {
                        if (ChartSeries[i]?.DataSeries?.Count > 0)
                        {
                            //if (ChartSeries[i].DataSeries.SeriesName == "Counter")
                            //{
                            //    continue;
                            //}

                            //TODO need to get min and max for visible channels
                            Ymax.Add((double.Parse(ChartSeries[i].DataSeries.YMax.ToString())));
                            Ymin.Add((double.Parse(ChartSeries[i].DataSeries.YMin.ToString())));
                            num = ChartSeries[i].DataSeries.Count;

                        }
                    }
                }));

                if (Ymax.Count > 0 && Ymin.Count > 0 && ((double)(Ymin.Min()) - 0.2) < ((double)(Ymax.Max()) + 0.2))
                {
                    var (min, max) = GetMinMaxEnabledChannels(Ymin, Ymax);
                    if (min != null && max != null)
                    {
                        EventChartYAxisRangeChanged?.Invoke(min.Value - 0.2, max.Value + 0.2);
                    }
                }
                YDataRange = new DoubleRange((double)(Ymin.Min()) - 0.2, (double)(Ymax.Max()) + 0.2);
                double spacing = 1 / (double)_sampleRateValue[_sampleRate];
                double maxX = spacing * num;
                IRange XMinMax = new DoubleRange(0, maxX);
                double[] x = new double[num];
                double[] y = new double[num];
                double yMin = Ymin.Min() - 0.2;
                double yMax = Ymax.Max() + 0.2;
                double yDiff = yMax - yMin;
                for (int i = 0; i < num; i++)
                {
                    x[i] = i * spacing;
                    y[i] = i * (yDiff) / ((double)num) + yMin;
                }
                Application.Current.Dispatcher.Invoke(new Action(() =>
                {
                    ChartSeriesForScrollBar = new ObservableCollection<IRenderableSeries>();

                    var l = new FastLineRenderableSeries();

                    l.DataSeries = CreateSingleUniformXyDataSeries("dummy", SignalType.ANALOG_IN);

                    l.IsVisible = true;

                    l.ResamplingMode = SciChart.Data.Numerics.ResamplingMode.Auto;
                    l.AntiAliasing = false;

                    ((UniformXyDataSeries<double>)l.DataSeries).Append(y);
                    ChartSeriesForScrollBar.Add(l);

                    for (int i = 0; i < _channelViewModels?.Count; i++)
                    {
                        if (ChartSeries[i]?.DataSeries?.Count > 0)
                        {
                            _channelViewModels[i].YVisibleRange = new DoubleRange(Ymin[i] - 0.2, Ymax[i] + 0.2);
                        }
                    }
                }));

                XVisibleRangeStack = new DoubleRange(0, maxX);

                Application.Current.Dispatcher.Invoke(new Action(() =>
                {
                    _specChViewModels.Each(s => { s.ChannelSeries.InvalidateParentSurface(RangeMode.ZoomToFit); s.XVisibleRange = s.ChannelSeries.XRange; }, (s => 0 < s.ChannelSeries.Count));
                }));
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart ZoomExtendChartSeries Error: " + ex.Message);
            }
        }

        /// <summary>
        /// Handles the DoWork event of the _bwLoader control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="DoWorkEventArgs"/> instance containing the event data.</param>
        private void _bwLoader_DoWork(object sender, DoWorkEventArgs e)
        {
            List<double> frameTimes = new List<double>();

            Thread.CurrentThread.Priority = ThreadPriority.AboveNormal;
           // IRange tmp = (IRange) new Object();
            RealTimeDataCapture.Instance.LoadDataFromFile();
            for (int j = 0; j < _chartSeriesMetadata.Count; j++)
            {
               
                if (_channelViewModels[j].ChannelName.Equals("Line1") || _channelViewModels[j].ChannelName.Equals("FrameOut")) //better to look in the xml here for the ADC port#
                {
                    int count = _channelViewModels[j].ChannelSeries.XValues.Count;
                    
                    for (int x = 0; x < count - 1; x++)
                    {
                        byte digYLeft = (byte)_channelViewModels[j].ChannelSeries.YValues[x];
                        byte digYRight = (byte)_channelViewModels[j].ChannelSeries.YValues[x + 1];

                        double X = (double)_channelViewModels[j].ChannelSeries.XValues[x];
                        
                        if(digYLeft < digYRight)
                        {
                            frameTimes.Add(X);
                            
                        }

                    }
                    RealTimeDataCapture.Instance.FrameTimes = frameTimes;
                    //MessageBox.Show();
                }



                bool done = false;
                Application.Current.Dispatcher.Invoke(new Action(() =>
                {
                    
                    

                    if (IsStackedDisplay)
                    {
                        Application.Current.Dispatcher.Invoke(new Action(() =>
                        {
                            if (_channelViewModels[j].IsVisible)
                            {
                                XVisibleRangeStack = _channelViewModels[j].ChannelSeries.XRange;
                                XDataRange = XVisibleRangeChart = XVisibleRangeStack;
                                done = true;
                            }
                        }));
                    }
                    else
                    {
                        Application.Current.Dispatcher.Invoke(new Action(() =>
                        {
                            if (ChartSeries[j].IsVisible)
                            {
                                XVisibleRangeChart = ((IDataSeries)ChartSeries[j].DataSeries).XRange;
                                XDataRange = XVisibleRangeStack = XVisibleRangeChart;
                                done = true;
                            }
                        }));
                    }
                }));
                if (done)
                {
                    break;
                }
            }
            ChangeVerticalMarkersMode(MarkerType.Load);
            IsVerticalMarkerVisible = true;
            
            ZoomExtendChartSeries();
            UpdateGUI();
            OnPropertyChanged("FrameButtonVisible");
            //make Buttons visible
        }

    /// <summary>
    /// Handles the RunWorkerCompleted event of the _bwLoader control.
    /// </summary>
    /// <param name="sender">The source of the event.</param>
    /// <param name="e">The <see cref="RunWorkerCompletedEventArgs"/> instance containing the event data.</param>
    private void _bwLoader_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (_chartMode == ChartModes.REVIEW)
            {
                for (int i = 0; i < _channelViewModels.Count; ++i)
                {
                    if (IsStackedDisplay)
                    {
                        _channelViewModels[i].IsVisible = _channelVisibilityChoice[i];
                    }
                    else
                    {
                        _chartSeries[i].IsVisible = _channelVisibilityChoice[i];
                    }
                }
            }

            UpdateRenderPriority(RenderPriority.Normal);

            _bwLoadDone = true;
        }

        /// <summary>
        /// Handles the DoWork event of the _bw control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="DoWorkEventArgs"/> instance containing the event data.</param>
        private void _bw_DoWork(object sender, DoWorkEventArgs e)
        {
            do
            {
                if (true == RealTimeDataCapture.Instance.GetData(ref _dataCaptureStruct))
                {
                    LoadDataFromDevice(ref _lastGlobalCounter);
                }
                Thread.Sleep(30);
            }
            while (false == _bw.CancellationPending);
        }

        /// <summary>
        /// Handles the Tick event of the __readCaptureTimer control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void _readCaptureTimer_Tick(object sender, EventArgs e)
        {
            UpdateCharts();
            OnPropertyChanged("CapturingIcon");
            OnPropertyChanged("IsCapturingStopped");
        }

        #endregion Methods

        #region Other

        /// <summary>
        /// Occurs when [measurement cursor mode changed].
        /// </summary>
        //public event Action<int> MeasurementCursorModeChanged;

        #endregion Other
    }

    public class SeriesMetaMode
    {
        #region Properties

        public double Duration
        {
            get;
            set;
        }

        public int Interleave
        {
            get;
            set;
        }

        public double SampleRate
        {
            get;
            set;
        }

        public int TotalAI
        {
            get;
            set;
        }

        public int TotalDI
        {
            get;
            set;
        }

        #endregion Properties
    }

    public class TriggerModeToDurationVisConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(Visibility))
                throw new InvalidOperationException("The target must be a Visibility");

            Visibility ret = Visibility.Collapsed;

            switch (System.Convert.ToInt32(value))
            {
                //free run
                case 0:
                    {
                        ret = Visibility.Collapsed;
                    }
                    break;
                //hw trigger
                case 1:
                case 2:
                    {
                        ret = Visibility.Visible;
                    }
                    break;
                default:
                    {

                    }
                    break;
            }

            return ret;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }
}