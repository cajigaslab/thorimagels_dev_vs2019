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
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Shapes;
    using System.Xml;
    using System.Xml.Linq;
    using System.Xml.Schema;
    using global::RealTimeLineChart.Model;
    using global::RealTimeLineChart.View;

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
    using SciChart.Core;
    using SciChart.Core.Framework;
    using SciChart.Data.Model;
    using SciChart.Drawing.HighSpeedRasterizer;
    using SciChart.Drawing.Utility;

    using ThorLogging;

    public static class ExtentionMethods
    {
        #region Methods

        public static void Each<T>(this IEnumerable<T> items, Action<T> action, Predicate<T> predicate = null)
        {
            if (0 >= items.Count())
                return;

            foreach (var item in items)
            {
                if (null != predicate)
                {
                    if (predicate(item))
                    {
                        action(item);
                    }
                }
                else
                {
                    action(item);
                }
            }
        }

        public static void Last<T>(this IEnumerable<T> items, Action<T> action, Predicate<T> predicate = null)
        {
            if (0 >= items.Count())
                return;

            T item = default(T);
            if (null == predicate)
            {
                item = items.Last();
            }
            else
            {
                item = items.Where(s => predicate(s)).LastOrDefault();
            }
            if (null != item)
            {
                action(item);
            }
        }

        #endregion Methods
    }

    /// <summary>
    /// 
    /// </summary>
    public partial class RealTimeLineChartViewModel : ViewModelBase
    {
        #region Fields

        //TODO: clean up and make this private with properties if needed
        public List<double> _coordinationVerticalMarkers = null;
        public VerticalSliceModifier _sliceModifier;
        public List<string> _tooltipVerticalMarkers = null;
        public ObservableCollection<VerticalMarker> _verticalmarker = new ObservableCollection<VerticalMarker>();
        public double _visibleYAxisMax = 0;
        public double _visibleYAxisMin = 0;




        static Dictionary<string, MarkerType> MarkerTypeDictionary = new Dictionary<string, MarkerType>()
        {
        {"MARKER_ADD", MarkerType.Add},
        {"MARKER_DELETE", MarkerType.Delete},
        {"MARKER_DELETEALL", MarkerType.DeleteAll},
        {"MARKER_HIDEALL", MarkerType.HideAll},
        {"MARKER_DISPLAYALL", MarkerType.DisplayAll},
        {"MARKER_SAVE", MarkerType.Save},
        {"MARKER_LOAD", MarkerType.Load},
        };

        double[] _aiData;
        private double[] _chartViewSize = new double[2];
        int[] _ciData;
        byte[] _diData;
        private double[] _fileVisibleYAxis = new double[2];
        private bool _isMeasureCursorVisible = false;
        private bool _isFrameCursorVisible = false;
        private bool _isRollOverEnabled = false;
        private ICommand _isRollOverEnabledCommand;
        private bool _isVerticalMarkerVisible = true;
        private RelayCommandWithParam _markerRelayCommand;
        private ICommand _removeChannelCommand;
        private ICommand _showMeasureCursorCommand;
        private ICommand _showFrameCursorCommand;
        private ICommand _showExportCommand;
        private bool _statsPanelEnable = true;
        private string _verticalMarkerTooltipText = string.Empty;
        double[] _viData;
        long[] _xdata;
        double[] _xdataDouble;
        IRange _xDataRange = new DoubleRange(0, 0);
        IRange _xVisibleRangeChart = new DoubleRange(0, 0);
        IRange _xVisibleRangeStack = new DoubleRange(0, 0);
        IRange _yVisibleRangeChart = new DoubleRange(0, 0);

        #endregion Fields

        #region Enumerations

        /// <summary>
        /// 
        /// </summary>
        public enum MarkerType
        {
            Add,
            Delete,
            DeleteAll,
            HideAll,
            DisplayAll,
            Save,
            Load
        }

        #endregion Enumerations

        #region Events

        /// <summary>
        /// Occurs when [event print screen].
        /// </summary>
        public event Action EventPrintScreen;

        #endregion Events

        #region Properties

        /// <summary>
        /// Gets the automatic range x setting.
        /// </summary>
        /// <value>
        /// The automatic range x setting.
        /// </value>
        public string AutoRangeX
        {
            get
            {
                return (ChartModes.CAPTURE == _chartMode) ? "Always" : "Never";
            }
        }

        /// <summary>
        /// Gets the automatic range y setting.
        /// </summary>
        /// <value>
        /// The automatic range y setting.
        /// </value>
        public string AutoRangeY
        {
            get
            {
                return (ChartModes.CAPTURE == _chartMode) ? "Always" : "Never";
            }
        }

        public double[] ChartViewSize
        {
            get
            {
                return _chartViewSize;
            }
            set
            {
                _chartViewSize = value;
                OnPropertyChanged("ChartViewSize");
            }
        }

        public double[] FileVisibleYAxis
        {
            get
            {
                return _fileVisibleYAxis;
            }
            set
            {
                _fileVisibleYAxis = value;
                OnPropertyChanged("FileVisibleYAxis");
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether measurement cursor is visible.
        /// </summary>
        /// <value>
        /// <c>true</c> if measurement cursor is visible; otherwise, <c>false</c>.  
        /// </value>
        public bool IsMeasureCursorVisible
        {
            get
            {
                return _isMeasureCursorVisible;
            }
            set
            {
                if (_isMeasureCursorVisible != value)
                {
                    _isMeasureCursorVisible = value;
                    OnPropertyChanged("IsMeasureCursorVisible");
                }
            }
        }

        public bool IsFrameCursorVisible
        {
            get
            {
                return _isFrameCursorVisible;
            }
            set
            {
                if (_isFrameCursorVisible != value)
                {
                    _isFrameCursorVisible = value;
                    OnPropertyChanged("IsFrameCursorVisible");
                }
            }
        }

        public Visibility FrameButtonVisible
        {
            get
            {
                if(RealTimeDataCapture.Instance.FrameTimes != null)
                {
                    return Visibility.Visible; 
                }
                else
                {
                    return Visibility.Collapsed;

                }

            }

        }

        /// <summary>
        /// Gets or sets a value indicating whether this instance RollOver is enabled.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance RollOver is enabled; otherwise, <c>false</c>.
        /// </value>
        public bool IsRollOverEnabled
        {
            get
            {
                return _isRollOverEnabled;
            }
            set
            {
                _isRollOverEnabled = value;
                OnPropertyChanged("IsRollOverEnabled");
            }
        }

        public ICommand IsRollOverEnabledCommand
        {
            get
            {
                if (this._isRollOverEnabledCommand == null)
                    this._isRollOverEnabledCommand = new RelayCommand(UpdateRollOverEnable);

                return this._isRollOverEnabledCommand;
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this instance vertical markers are visible.
        /// </summary>
        /// <value>
        /// <c>true</c> if this instance vertical markers are visible; otherwise, <c>false</c>.
        /// </value>
        public bool IsVerticalMarkerVisible
        {
            get
            {
                return _isVerticalMarkerVisible;
            }
            set
            {
                _isVerticalMarkerVisible = value;
                if (true == _isVerticalMarkerVisible)
                {
                    ChangeVerticalMarkersMode(MarkerType.DisplayAll);
                }
                else
                {
                    ChangeVerticalMarkersMode(MarkerType.HideAll);
                }
                OnPropertyChanged("IsVerticalMarkerVisible");
            }
        }

        public SciChartSurface MainChartSurface
        {
            get; set;
        }

        public RelayCommandWithParam MarkerRelayCommand
        {
            get
            {
                if (this._markerRelayCommand == null)
                    this._markerRelayCommand = new RelayCommandWithParam(MarkerCommandSwitch);
                return this._markerRelayCommand;
            }
        }

        public ICommand RemoveChannelCommand
        {
            get
            {
                if (this._removeChannelCommand == null)
                    this._removeChannelCommand = new RelayCommandWithParam((x) => RemoveChannel(x));

                return this._removeChannelCommand;
            }
        }

        public int RemovedSpectralChannel
        {
            get
            {
                return _removedSpectralChannel;
            }
            set
            {
                if (0 <= value)
                {
                    _removedSpectralChannel = value;

                    XmlNodeList ndList = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SpectralDomain/SpectralChannel");
                    for (int i = 0; i < ndList.Count; i++)
                    {
                        string name = string.Empty;
                        GetAttribute(ndList[i], SettingsDoc, "alias", ref name);
                        if (name.CompareTo(SpecChViewModels[_removedSpectralChannel].ChannelName) == 0)
                        {
                            ndList[i].ParentNode.RemoveChild(ndList[i]);
                            break;
                        }
                    }
                    SettingsDoc.Save(Constants.ThorRealTimeData.SETTINGS_FILE_NAME);
                    LoadDocumentSettings();
                    CreateFreqChartSeries();
                    UpdateSpectralCharts();
                }
            }
        }

        public ICommand ShowMeasureCursorCommand
        {
            get
            {
                if (this._showMeasureCursorCommand == null)
                    this._showMeasureCursorCommand = new RelayCommand(ShowMeasureCursor);

                return this._showMeasureCursorCommand;
            }
        }

        public ICommand ShowFrameCursorCommand
        {
            get
            {

                if (this._showFrameCursorCommand == null)
                    this._showFrameCursorCommand = new RelayCommand(ShowFrameCursor);

                return this._showFrameCursorCommand;
            }
        }

        public ICommand ShowExportCommand
        {
            get
            {   

                if (this._showExportCommand == null)
                    this._showExportCommand = new RelayCommand(ExportFrameTimes);

                return this._showExportCommand;
            }
        }

        /// <summary>
        /// Gets the vertical marker list.
        /// </summary>
        /// <value>
        /// The vertical marker list.
        /// </value>
        public ObservableCollection<VerticalMarker> VerticalMarkerList
        {
            get { return _verticalmarker; }
        }

        /// <summary>
        /// Gets or sets the vertical marker tooltip text.
        /// </summary>
        /// <value>
        /// The vertical marker tooltip text.
        /// </value>
        public string VerticalMarkerTooltipText
        {
            get
            {
                return _verticalMarkerTooltipText;
            }
            set
            {
                _verticalMarkerTooltipText = value;
                OnPropertyChanged("VerticalMarkerTooltipText");
            }
        }

        /// <summary>
        /// Gets or sets the visible y axis maximum.
        /// </summary>
        /// <value>
        /// The visible y axis maximum.
        /// </value>
        public double VisibleYAxisMax
        {
            get
            {
                return _visibleYAxisMax;
            }
            set
            {
                if (_visibleYAxisMax == value)
                {
                    OnPropertyChanged("VisibleYAxisMax");
                    return;
                }
                _visibleYAxisMax = value;
                if (EventChartYAxisRangeChanged != null)
                {
                    EventChartYAxisRangeChanged(VisibleYAxisMin, _visibleYAxisMax);
                }
                OnPropertyChanged("VisibleYAxisMax");
            }
        }

        /// <summary>
        /// Gets or sets the visible y axis minimum.
        /// </summary>
        /// <value>
        /// The visible y axis minimum.
        /// </value>
        public double VisibleYAxisMin
        {
            get
            {
                return _visibleYAxisMin;
            }
            set
            {
                if (_visibleYAxisMin == value)
                {
                    OnPropertyChanged("VisibleYAxisMin");
                    return;
                }
                _visibleYAxisMin = value;
                if (EventChartYAxisRangeChanged != null)
                {
                    EventChartYAxisRangeChanged(_visibleYAxisMin, VisibleYAxisMax);
                }
                OnPropertyChanged("VisibleYAxisMin");
            }
        }

        /// <summary>
        /// Gets or sets the data range of stack charts.
        /// </summary>
        public IRange XDataRange
        {
            get
            {
                return _xDataRange;
            }
            set
            {
                _xDataRange = value;
                OnPropertyChanged("XDataRange");
            }
        }

        /// <summary>
        /// Gets or sets the visible x axis minimum.
        /// </summary>
        /// <value>
        /// The visible x axis minimum.
        /// </value>
        public IRange XVisibleRangeChart
        {
            get
            {
                return _xVisibleRangeChart;
            }
            set
            {
                switch ((ChartModes)_chartMode)
                {
                    case ChartModes.CAPTURE:
                        _xVisibleRangeChart = value;
                        OnPropertyChanged("XVisibleRangeChart");
                        break;
                    case ChartModes.REVIEW:
                        if (((_xVisibleRangeChart.Min != value.Min) || (_xVisibleRangeChart.Max != value.Max)) && (0 < _chartSeries.Count))
                        {
                            double lowerRange = (0 < ((DoubleRange)_xDataRange).Diff) ? Math.Max(((DoubleRange)_xDataRange).Min, ((DoubleRange)value).Min) : ((DoubleRange)value).Min;
                            double upperRange = (0 < ((DoubleRange)_xDataRange).Diff) ? Math.Min(((DoubleRange)_xDataRange).Max, ((DoubleRange)value).Max) : ((DoubleRange)value).Max;

                            if ((lowerRange != upperRange) && ((double)_xVisibleRangeChart.Min != lowerRange || (double)_xVisibleRangeChart.Max != upperRange))
                            {
                                _xVisibleRangeChart = new DoubleRange(lowerRange, upperRange);
                                OnPropertyChanged("XVisibleRangeChart");
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        public double XVisibleRangeChartMax
        {
            get
            {
                return (double)XVisibleRangeChart.Max;
            }
            set
            {
                XVisibleRangeChart.Max = value;
                OnPropertyChanged("XVisibleRangeChartMax");
            }
        }

        public double XVisibleRangeChartMin
        {
            get
            {
                return (double)XVisibleRangeChart.Min;
            }
            set
            {
                XVisibleRangeChart.Min = value;
                OnPropertyChanged("XVisibleRangeChartMin");
            }
        }

        /// <summary>
        /// Gets or sets the visible x axis of stack charts.
        /// </summary>
        public IRange XVisibleRangeStack
        {
            get
            {
                return _xVisibleRangeStack;
            }
            set
            {
                //_xVisibleRangeStack = value;
                //OnPropertyChanged("XVisibleRangeStack");
                switch (_chartMode)
                {
                    case ChartModes.CAPTURE:
                        //realtime update need to limit x visible range, otherwise visible
                        //range will be more than specified when display in high resolution
                        if (FifoSize < ((DoubleRange)value).Diff)
                            value = new DoubleRange(((DoubleRange)value).Max - FifoSize, ((DoubleRange)value).Max);
                        _xVisibleRangeStack = value;

                        for (int i = 0; i < _channelViewModels.Count; i++)
                        {
                            if (_channelViewModels[i].XVisibleRange.Min != _xVisibleRangeStack.Min || _channelViewModels[i].XVisibleRange.Max != _xVisibleRangeStack.Max)
                            {
                                _channelViewModels[i].XVisibleRange = _xVisibleRangeStack;
                            }
                        }
                        OnPropertyChanged("XVisibleRangeStack");
                        break;
                    case ChartModes.REVIEW:
                        if (((_xVisibleRangeStack.Min != value.Min) || (_xVisibleRangeStack.Max != value.Max)) && (0 < _channelViewModels.Count))
                        {
                            _xVisibleRangeStack = value;

                            for (int i = 0; i < _channelViewModels.Count; i++)
                            {
                                if (_channelViewModels[i].XVisibleRange.Min != _xVisibleRangeStack.Min || _channelViewModels[i].XVisibleRange.Max != _xVisibleRangeStack.Max)
                                {
                                    _channelViewModels[i].XVisibleRange = _xVisibleRangeStack;
                                }
                            }

                            OnPropertyChanged("XVisibleRangeStack");
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        /// <summary>
        /// Gets or sets the visible y axis minimum.
        /// </summary>
        /// <value>
        /// The visible x axis minimum.
        /// </value>
        public IRange YVisibleRangeChart
        {
            get
            {
                return _yVisibleRangeChart;
            }
            set
            {
                switch (_chartMode)
                {
                    case ChartModes.CAPTURE:
                        _yVisibleRangeChart = value;
                        OnPropertyChanged("YVisibleRangeChart");
                        break;
                    case ChartModes.REVIEW:
                        if (((_yVisibleRangeChart.Min != value.Min) || (_yVisibleRangeChart.Max != value.Max)) && (0 < _chartSeries.Count))
                        {
                            double lowerRange = (0 < ((DoubleRange)YDataRange).Diff) ? Math.Max(((DoubleRange)YDataRange).Min, ((DoubleRange)value).Min) : ((DoubleRange)value).Min;
                            double upperRange = (0 < ((DoubleRange)YDataRange).Diff) ? Math.Min(((DoubleRange)YDataRange).Max, ((DoubleRange)value).Max) : ((DoubleRange)value).Max;

                            if ((lowerRange != upperRange) && ((double)_yVisibleRangeChart.Min != lowerRange || (double)_yVisibleRangeChart.Max != upperRange))
                            {
                                _yVisibleRangeChart = new DoubleRange(lowerRange, upperRange);
                                OnPropertyChanged("YVisibleRangeChart");
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        #endregion Properties

        #region Methods

        public void AddChartSeries(XmlNode node, int insertIndex)
        {
            string str = string.Empty;
            int iVal = 0;
            int enable = 0;
            string group = string.Empty;
            string dataset = string.Empty;
            string alias = string.Empty;
            int signalType = 0;
            string physicalChannel = string.Empty;
            int red = 0, green = 0, blue = 0;
            string yLabel = string.Empty;
            bool visible = true;

            try
            {
                if (!(GetAttribute(node, SettingsDoc, "alias", ref alias) && (0 < alias.Length)))
                    throw new System.ArgumentException("empty alias");

                if (!(GetAttribute(node, SettingsDoc, "group", ref group) && (0 < group.Length)))
                    throw new System.ArgumentException("empty group", "group of " + alias);

                if (!(GetAttribute(node, SettingsDoc, "physicalChannel", ref physicalChannel) && (0 < physicalChannel.Length)))
                    throw new System.ArgumentException("empty physicalChannel", "physicalChannel of " + alias);

                if (!(GetAttribute(node, SettingsDoc, "enable", ref str) && (Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out enable))))
                    throw new System.ArgumentException("invalid Parameter", "enable of " + alias);

                if (!(GetAttribute(node, SettingsDoc, "signalType", ref str) && (Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out signalType))))
                    throw new System.ArgumentException("invalid Parameter", "signalType of " + alias);

                if (GetAttribute(node, SettingsDoc, "visible", ref str) && (Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))
                    visible = (0 == iVal) ? false : true;

                if (!(GetAttribute(node, SettingsDoc, "red", ref str) && (Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out red)))) { }
                if (!(GetAttribute(node, SettingsDoc, "green", ref str) && (Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out green)))) { }
                if (!(GetAttribute(node, SettingsDoc, "blue", ref str) && (Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out blue)))) { }

                GetAttribute(node, SettingsDoc, "yLabel", ref yLabel);

                if (1 == enable)
                {
                    SeriesMetadata sm = new SeriesMetadata();
                    sm.Alias = alias;
                    sm.Group = group;
                    sm.SignalType = (SignalType)Convert.ToInt32(signalType);
                    sm.PhysicalChannel = physicalChannel;
                    sm.LineColor = Color.FromRgb(Convert.ToByte(red), Convert.ToByte(green), Convert.ToByte(blue));

                    //TODO: might not need logic below as the data series is created and assigned elsewhere
                    IDataSeries ds0;// = CreateSingleDataSeries(alias, sm.SignalType);

                    if (ChartModes.REVIEW == _chartMode)
                    {
                        ds0 = CreateSingleUniformXyDataSeries(alias, sm.SignalType);
                    }
                    else
                    {
                        ds0 = CreateSingleDataSeries(alias, sm.SignalType);
                    }

                    switch (sm.SignalType)
                    {
                        case SignalType.ANALOG_IN:
                        case SignalType.DIGITAL_IN:
                        case SignalType.COUNTER_IN:
                        case SignalType.VIRTUAL:
                            {
                                bool found = false;

                                for (int i = 0; i < _channelViewModels.Count; i++)
                                {
                                    if (_channelViewModels[i].ChannelSeries.SeriesName == alias && !string.IsNullOrWhiteSpace(alias))
                                    {
                                        found = true;
                                    }
                                }

                                if (!found)
                                {
                                    _chartSeriesMetadata.Insert(insertIndex, sm);

                                    //stack view
                                    ChannelViewModel channelViewModel = new ChannelViewModel(sm.LineColor, visible) { };
                                    channelViewModel.ChannelSeries = ds0;
                                    channelViewModel.XAxisVisibleChanged += new Action(XAxis_Changed);
                                    channelViewModel.YLabel = yLabel;

                                    //unlike list, observable collection must use add if index out of bound
                                    if (_channelViewModels.Count <= insertIndex)
                                    {
                                        _channelViewModels.Add(channelViewModel);
                                    }
                                    else
                                    {
                                        _channelViewModels.Insert(insertIndex, channelViewModel);
                                    }
                                }
                            }
                            break;
                        case SignalType.SPECTRAL:
                        case SignalType.SPECTRAL_VIRTUAL:
                            {
                                _specSeriesMetadata.Insert(insertIndex, sm);

                                //not creating lines for fitting
                                if (!sm.PhysicalChannel.Contains(Constants.ThorRealTimeData.LORENTZIANFITX) && !sm.PhysicalChannel.Contains(Constants.ThorRealTimeData.LORENTZIANFITY))
                                {
                                    SpectralViewModel spVM = new SpectralViewModel(sm.LineColor, visible) { ChannelSeries = ds0, ChannelSeries2 = CreateSingleDataSeries(alias, sm.SignalType), XVisibleRange = new DoubleRange(_freqMin, _freqMax), YLabel = yLabel };
                                    spVM.XAxisVisibleChanged += new Action(XAxis_Changed);

                                    //unlike list, observable collection must use add if index out of bound
                                    if (_specChViewModels.Count <= insertIndex)
                                    {
                                        _specChViewModels.Add(spVM);
                                    }
                                    else
                                    {
                                        _specChViewModels.Insert(insertIndex, spVM);
                                    }
                                }
                                else
                                {
                                    string tAlias = sm.PhysicalChannel.Contains(Constants.ThorRealTimeData.LORENTZIANFITX) ? Constants.ThorRealTimeData.POWERSPECTRUMX : Constants.ThorRealTimeData.POWERSPECTRUMY;
                                    SeriesMetadata tSm = _specSeriesMetadata.LastOrDefault(s => s.PhysicalChannel.Contains(tAlias));
                                    if (null != tSm)
                                        _specChViewModels.Last(s => { s.Stroke[1] = sm.LineColor; }, (s => true == s.ChannelName.Contains(tSm.Alias)));
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart AddChartSeries Error: " + ex.Message);
                MessageBox.Show("Check settings file for " + ex.Message, "RealTimeDataSettings.xml Error", MessageBoxButton.OK, MessageBoxImage.Error);
                Environment.Exit(0);
            }
        }

        public void CalculateSpectral(bool resetRange = false)
        {
            if ((_bwSpecAnalyzer.IsBusy) || (!_isLoaded) || (0 == EnabledSpectralChannel.Count))
                return;

            //persist settings to both current episode and active
            SaveGlobalSettings();

            //background worker to update progress:
            BackgroundWorker splashWkr = new BackgroundWorker();
            splashWkr.WorkerSupportsCancellation = true;
            _bwLoadDone = false;

            try
            {
                _splash = new SplashProgress();
                _splash.DisplayText = "Please wait for spectral analysis ...";
                _splash.DisplayProgress = false;
                _splash.ShowInTaskbar = false;
                _splash.Owner = Application.Current.MainWindow;
                _splash.Show();
                EnableUIPanel(false);
                _splash.CancelSplashProgress += new EventHandler(CancelLoading);

                //enable handlers of background worker to load file:
                _bwSpecAnalyzer.DoWork += new DoWorkEventHandler(_bwSpecAnalyzer_DoWork);
                _bwSpecAnalyzer.RunWorkerCompleted += new RunWorkerCompletedEventHandler(_bwSpecAnalyzer_RunWorkerCompleted);
                _bwSpecAnalyzer.WorkerSupportsCancellation = true;

                _bwSpecAnalyzer.RunWorkerAsync(resetRange);
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
                        System.Threading.Thread.Sleep(10);

                    }
                    while ((true == _bwSpecAnalyzer.IsBusy) || RealTimeDataCapture.Instance.IsLoading());
                };

                splashWkr.RunWorkerCompleted += delegate (object sender, RunWorkerCompletedEventArgs e)
                {
                    _splash.Close();
                    // App inherits from Application, and has a Window property called MainWindow
                    // and a List<Window> property called OpenWindows.
                    Application.Current.MainWindow.Activate();

                    //Release handlers of background loader:
                    _bwSpecAnalyzer.DoWork -= _bwSpecAnalyzer_DoWork;
                    _bwSpecAnalyzer.RunWorkerCompleted -= _bwSpecAnalyzer_RunWorkerCompleted;

                    EnableUIPanel(true);
                };
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart CalculateSpectral Error: " + ex.Message);
                _bwSpecAnalyzer.CancelAsync();
                splashWkr.CancelAsync();
            }
        }

        public void CreateFreqChartSeries()
        {
            try
            {
                string str = string.Empty;
                int iVal = 0;

                //clear series
                _specChViewModels.Each(s => { s.Clear(); });

                //create freq series on stack view, [NOTE] avoid recreation of stack channel with sciChart since it causes memory leak
                XmlNodeList ndListAll = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SpectralDomain/SpectralChannel | /RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SpectralDomain/VirtualChannel");
                string[] aliasAll = ndListAll.Cast<XmlNode>().Select(node => node.Attributes["alias"].Value).ToArray();
                XmlNodeList ndListToAdd = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SpectralDomain/SpectralChannel[@enable=1] | /RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SpectralDomain/VirtualChannel[@enable=1]");
                string[] aliasToAdd = ndListToAdd.Cast<XmlNode>().Select(node => node.Attributes["alias"].Value).ToArray();

                //remove disabled existing channels
                if (0 != _specSeriesMetadata.Count)
                {
                    for (int i = _specSeriesMetadata.Count - 1; i >= 0; i--)
                    {
                        if (!aliasToAdd.Contains(_specSeriesMetadata[i].Alias))
                        {
                            _specChViewModels.RemoveAt(i);
                            _specSeriesMetadata.RemoveAt(i);
                        }
                    }
                }

                //create series
                bool initSetup = (0 == _specSeriesMetadata.Count);
                for (int i = 0; i < ndListToAdd.Count; i++)
                {
                    int tgtID = Array.FindIndex(aliasAll, a => a == aliasToAdd[i]);
                    if (null == _specSeriesMetadata.FirstOrDefault(d => 0 == d.Alias.CompareTo(aliasToAdd[i])))
                    {
                        if (!initSetup)
                        {
                            bool insert = false;
                            for (int j = 0; j < _specSeriesMetadata.Count; j++)
                            {
                                if (Array.FindIndex(aliasAll, a => a == _specSeriesMetadata[j].Alias) > tgtID)
                                {
                                    AddChartSeries(ndListToAdd[i], j);
                                    insert = true;
                                    break;
                                }
                            }
                            if (!insert)
                                AddChartSeries(ndListToAdd[i], _specSeriesMetadata.Count);
                        }
                        else
                        {
                            AddChartSeries(ndListToAdd[i], _specSeriesMetadata.Count);
                        }
                    }
                }

                //update existing data series and meta data with settings other than alias
                for (int i = 0; i < _specSeriesMetadata.Count; i++)
                {
                    int id = Array.FindIndex(aliasToAdd, a => 0 == a.CompareTo(_specSeriesMetadata[i].Alias));
                    int red = ((GetAttribute(ndListToAdd[id], SettingsDoc, "red", ref str) && (Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))) ? iVal : 0;
                    int green = ((GetAttribute(ndListToAdd[id], SettingsDoc, "green", ref str) && (Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))) ? iVal : 0;
                    int blue = ((GetAttribute(ndListToAdd[id], SettingsDoc, "blue", ref str) && (Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))) ? iVal : 0;
                    _specSeriesMetadata[i].Group = ((GetAttribute(ndListToAdd[id], SettingsDoc, "group", ref str) && (0 < str.Length))) ? str : _specSeriesMetadata[i].Group;
                    _specSeriesMetadata[i].SignalType = ((GetAttribute(ndListToAdd[id], SettingsDoc, "signalType", ref str) && (Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))) ? (SignalType)iVal : _specSeriesMetadata[i].SignalType;
                    _specSeriesMetadata[i].PhysicalChannel = ((GetAttribute(ndListToAdd[id], SettingsDoc, "physicalChannel", ref str) && (0 < str.Length))) ? str : _specSeriesMetadata[i].PhysicalChannel;
                    _specSeriesMetadata[i].LineColor = Color.FromRgb(Convert.ToByte(red), Convert.ToByte(green), Convert.ToByte(blue));

                    //since not creating lines for fitting, update spec channel 2nd line stroke by fitting
                    if (!_specSeriesMetadata[i].PhysicalChannel.Contains(Constants.ThorRealTimeData.LORENTZIANFITX) && !_specSeriesMetadata[i].PhysicalChannel.Contains(Constants.ThorRealTimeData.LORENTZIANFITY))
                    {
                        _specChViewModels.Last(s =>
                        {
                            s.Stroke[1] = _specSeriesMetadata[i].LineColor;
                            s.IsVisible = ((GetAttribute(ndListToAdd[id], SettingsDoc, "visible", ref str) && (Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)) && (0 == iVal))) ? false : true;
                            s.YLabel = ((GetAttribute(ndListToAdd[id], SettingsDoc, "yLabel", ref str) && (0 < str.Length))) ? str : s.YLabel;
                        }, (s => true == s.ChannelName.Contains(_specSeriesMetadata[i].Alias)));
                    }
                    else
                    {
                        string tAlias = _specSeriesMetadata[i].PhysicalChannel.Contains(Constants.ThorRealTimeData.LORENTZIANFITX) ? Constants.ThorRealTimeData.POWERSPECTRUMX : Constants.ThorRealTimeData.POWERSPECTRUMY;
                        SeriesMetadata tSm = _specSeriesMetadata.LastOrDefault(s => s.PhysicalChannel.Contains(tAlias));
                        if (null != tSm)
                            _specChViewModels.Last(s => { s.Stroke[1] = _specSeriesMetadata[i].LineColor; }, (s => true == s.ChannelName.Contains(tSm.Alias)));
                    }
                }

                //no fifo for spectral channels
                _specChViewModels.Each(s => { s.ChannelSeries.FifoCapacity = (int?)null; s.ChannelSeries2.FifoCapacity = (int?)null; s.Clear(); s.Refresh(); });

                //display single x axis in stack mode, show scrollbar and XAxis at bottom:
                _specChViewModels.Each(s => { s.XAxisVisible = false; });
                _specChViewModels.Last(s => { s.XAxisVisible = true; }, (s => true == s.IsVisible));

                //show or hide spec panel
                if (0 >= _specChViewModels.Count)
                    SpecPanelHeight = new GridLength(0.0, GridUnitType.Pixel);
                else if (0 >= SpecPanelHeight.Value)
                    SpecPanelHeight = new GridLength(_specChViewModels[0].Height, GridUnitType.Pixel);
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart Create Spectral Series Error: " + ex.Message);
                MessageBox.Show("Unable to Create Spectral Series due to " + ex.Message, "RealTimeDataSettings.xml Error", MessageBoxButton.OK, MessageBoxImage.Error);
                Environment.Exit(0);
            }
        }

        public IDataSeries CreateSingleDataSeries(string alias, SignalType type)
        {
            switch (type)
            {
                case SignalType.DIGITAL_IN:
                    {
                        IXyDataSeries<double, int> series = (ChartModes.CAPTURE == _chartMode) ?
                            new XyDataSeries<double, int> { FifoCapacity = _fifoSize, SeriesName = alias } :
                            new XyDataSeries<double, int> { FifoCapacity = null, SeriesName = alias };
                        series.AcceptsUnsortedData = false;
                        series.DataDistributionCalculator = new UserDefinedDistributionCalculator<double, int>
                        {
                            ContainsNaN = false,
                            IsEvenlySpaced = false,
                            IsSortedAscending = true,
                        };
                        return series;
                    }
                case SignalType.COUNTER_IN:
                    {
                        IXyDataSeries<double, int> series = (ChartModes.CAPTURE == _chartMode) ?
                            new XyDataSeries<double, int> { FifoCapacity = _fifoSize, SeriesName = alias } :
                            new XyDataSeries<double, int> { FifoCapacity = null, SeriesName = alias };
                        series.AcceptsUnsortedData = false;
                        series.DataDistributionCalculator = new UserDefinedDistributionCalculator<double, int>
                        {
                            ContainsNaN = false,
                            IsEvenlySpaced = false,
                            IsSortedAscending = true,
                        };
                        return series;
                    }

                default:
                    {
                        //FIFO will always be used with the Capture Mode. Non FIFO is specific to the review mode
                        IXyDataSeries<double, double> series = (ChartModes.CAPTURE == _chartMode) ?
                            new XyDataSeries<double, double> { FifoCapacity = _fifoSize, SeriesName = alias } :
                            new XyDataSeries<double, double> { FifoCapacity = null, SeriesName = alias };
                        series.AcceptsUnsortedData = false;
                        series.DataDistributionCalculator = new UserDefinedDistributionCalculator<double, double>
                        {
                            ContainsNaN = false,
                            IsEvenlySpaced = false,
                            IsSortedAscending = true,
                        };
                        return series;
                    }
            }
        }

        /// <summary>
        /// Creates the single data series. called in review mode
        /// </summary>
        /// <param name="alias">The alias of lines.</param>
        /// <returns></returns>
        public IDataSeries CreateSingleUniformXyDataSeries(string alias, SignalType type)
        {
            IDataSeries series;
            double spacing = 1 / (double)_sampleRateValue[_sampleRate];
            //FIFO will always be used with the Capture Mode. Non FIFO is specific to the review mode
            switch (type)
            {
                case SignalType.ANALOG_IN:
                    {
                        var args = new UniformDataDistributionArgs<double>(false, -10, 10);
                        series = new UniformXyDataSeries<double>(0, args) { FifoCapacity = null, SeriesName = alias, XStart = 0, XStep = spacing };
                        break;
                    }
                case SignalType.DIGITAL_IN:

                    {
                        var args = new UniformDataDistributionArgs<byte>(false, 0, 1);
                        series = new UniformXyDataSeries<byte>(0, args) { FifoCapacity = null, SeriesName = alias, XStart = 0, XStep = spacing };
                        break;
                    }
                case SignalType.COUNTER_IN:

                    {
                        series = new UniformXyDataSeries<int> { FifoCapacity = null, SeriesName = alias, XStart = 0, XStep = spacing };
                        break;
                    }
                case SignalType.VIRTUAL:
                case SignalType.SPECTRAL:
                case SignalType.SPECTRAL_VIRTUAL:

                    {
                        series = new UniformXyDataSeries<double> { FifoCapacity = null, SeriesName = alias, XStart = 0, XStep = spacing };
                        break;
                    }
                default:
                    {
                        series = new UniformXyDataSeries<double> { FifoCapacity = null, SeriesName = alias, XStart = 0, XStep = spacing };
                        break;
                    }
            }
            return series;
        }

        public void CreateTimeChartSeries()
        {
            try
            {
                string str = string.Empty;
                int iVal = 0;

                //clear series
                    Dictionary<String, IRange> seriesNameToRange = new Dictionary<String, IRange>();
                Dictionary<String, Boolean> seriesNameToYLock = new Dictionary<String, Boolean>();
                //Save the Visible range and if the Ylock is enabled before the chart series is reset.
                //Line editor may remove or add lines, so associate range with channel name.
                _channelViewModels.Each(s => {
                    seriesNameToRange.Add(s.ChannelName, s.YVisibleRange);
                    seriesNameToYLock.Add(s.ChannelName, s.YVisibleLock);
                    s.Clear(); 
                });

                ChannelViewModels = new ObservableCollection<ChannelViewModel>();
                _chartSeries?.Each(s => { s.DataSeries?.Clear(); });
                ChartSeries = new ObservableCollection<IRenderableSeries>();
                _chartSeriesMetadata.Clear();
                //get time range
                _dataSeriesSize = 0;
                UInt64 size = 0;
                DoubleRange xRange = null;

                if ((true == (IsSaving || forceIPC)) && (null != _hdf5Reader))
                {
                    if (true == _hdf5Reader.CheckH5GrpDataset("/Global", "/GCtr", ref size))
                    {
                        if (0 == _dataSeriesSize)
                        {
                            _dataSeriesSize = size;
                            FileVisibleYAxis[0] = FileVisibleYAxis[1] = 0;

                            if (_dataSeriesSize != 0)
                            {
                                xRange = new DoubleRange(0, ((double)_dataSeriesSize / (ulong)(_sampleRateValue[_sampleRate])));
                                XDataRange = new DoubleRange(0.0, 0.0); //reset here for later update
                            }
                        }
                    }
                    _hdf5Reader.CloseH5();
                }

                //[NOTE] avoid recreation of stack channel with sciChart since it causes memory leak
                XmlNodeList ndListAll = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/DataChannel | /RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/VirtualChannel");
                string[] aliasAll = ndListAll.Cast<XmlNode>().Select(node => node.Attributes["alias"].Value).ToArray();
                XmlNodeList ndListToAdd = SettingsDoc.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/DataChannel[@enable=1] | /RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/VirtualChannel[@enable=1]");
                string[] aliasToAdd = ndListToAdd.Cast<XmlNode>().Select(node => node.Attributes["alias"].Value).ToArray();

                //create time series on both view, stack or non-stack display
                bool initSetup = (0 == _chartSeriesMetadata.Count);
                for (int i = 0; i < ndListToAdd.Count; i++)
                {
                    AddChartSeries(ndListToAdd[i], _chartSeriesMetadata.Count);
                }

                //update existing data series and meta data with settings other than alias
                for (int i = 0; i < _chartSeriesMetadata.Count; i++)
                {
                    int id = Array.FindIndex(aliasToAdd, a => 0 == a.CompareTo(_chartSeriesMetadata[i].Alias));
                    int red = ((GetAttribute(ndListToAdd[id], SettingsDoc, "red", ref str) && (Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))) ? iVal : 0;
                    int green = ((GetAttribute(ndListToAdd[id], SettingsDoc, "green", ref str) && (Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))) ? iVal : 0;
                    int blue = ((GetAttribute(ndListToAdd[id], SettingsDoc, "blue", ref str) && (Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))) ? iVal : 0;
                    _chartSeriesMetadata[i].Group = ((GetAttribute(ndListToAdd[id], SettingsDoc, "group", ref str) && (0 < str.Length))) ? str : _chartSeriesMetadata[i].Group;
                    _chartSeriesMetadata[i].SignalType = ((GetAttribute(ndListToAdd[id], SettingsDoc, "signalType", ref str) && (Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)))) ? (SignalType)iVal : _chartSeriesMetadata[i].SignalType;
                    _chartSeriesMetadata[i].PhysicalChannel = ((GetAttribute(ndListToAdd[id], SettingsDoc, "physicalChannel", ref str) && (0 < str.Length))) ? str : _chartSeriesMetadata[i].PhysicalChannel;
                    _chartSeriesMetadata[i].LineColor = Color.FromRgb(Convert.ToByte(red), Convert.ToByte(green), Convert.ToByte(blue));

                    _channelViewModels.Last(s =>
                    {
                        s.Stroke = _chartSeriesMetadata[i].LineColor;
                        s.IsVisible = ((GetAttribute(ndListToAdd[id], SettingsDoc, "visible", ref str) && (Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)) && (0 == iVal))) ? false : true;
                        bool ylock = ((GetAttribute(ndListToAdd[id], SettingsDoc, "yLock", ref str) && (Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)) && (0 == iVal))) ? false : true;
                        ylock = ChartModes.CAPTURE == _chartMode && IsStackedDisplay ? ylock : false;
                        s.YVisibleLock = ylock;
                        s.YLabel = ((GetAttribute(ndListToAdd[id], SettingsDoc, "yLabel", ref str) && (0 < str.Length))) ? str : s.YLabel;
                    }, (s => true == s.ChannelName.Contains(_chartSeriesMetadata[i].Alias)));
                }

                //create data series based on current chartMode
                _channelViewModels.Each(s => { s.ChannelSeries.FifoCapacity = ((ChartModes.CAPTURE == _chartMode) ? _fifoSize : (int?)null); s.Clear(); s.Refresh(); });
                _chartSeries.Each(s => { s.DataSeries.FifoCapacity = ((ChartModes.CAPTURE == _chartMode) ? _fifoSize : (int?)null); s.DataSeries.Clear(); });

                //display single x axis in stack mode, show scrollbar and XAxis at bottom
                _channelViewModels.Each(s => { s.XAxisVisible = false; });
                _channelViewModels.Last(s => { s.XAxisVisible = true; }, (s => true == s.IsVisible));

                //recreate chart series for legend in order, cannot share data with stack since it will slow down rendering
                for (int i = 0; i < _chartSeriesMetadata.Count; i++)
                {
                    var l = new FastLineRenderableSeries();
                    if (ChartModes.REVIEW == _chartMode)
                    {
                        l.DataSeries = _channelViewModels[i].ChannelSeries = CreateSingleUniformXyDataSeries(_channelViewModels[i].ChannelName, _chartSeriesMetadata[i].SignalType);
                    }
                    else
                    {
                        l.DataSeries = _channelViewModels[i].ChannelSeries = CreateSingleDataSeries(_channelViewModels[i].ChannelName, _chartSeriesMetadata[i].SignalType);
                    }
                    l.IsDigitalLine = _channelViewModels[i].IsDigitalLine = SignalType.DIGITAL_IN == _chartSeriesMetadata[i].SignalType;
                    l.StrokeThickness = 2;
                    if (IsStackedDisplay)
                    {
                        l.IsVisible = false;
                    }
                    else
                    {
                        l.IsVisible = _channelViewModels[i].IsVisible;

                        _channelViewModels[i].IsVisible = false;
                    }
                    l.Stroke = _channelViewModels[i].Stroke;
                    l.ResamplingMode = SciChart.Data.Numerics.ResamplingMode.Auto;
                    l.AntiAliasing = false;

                    _chartSeries.Add(l);
                }

                //Restore the Y axis if the selected chart is in YLock
                //also restore the YLock
                _channelViewModels.Each(s => {
                    IRange i;
                    if (seriesNameToRange.TryGetValue(s.ChannelName, out i))
                    {
                        s.YVisibleRange = i;
                    }
                    else
                    {
                        s.YVisibleRange = new DoubleRange(-5, 5);
                    }
                    Boolean b;
                    if (seriesNameToYLock.TryGetValue(s.ChannelName, out b))
                    {
                        s.YVisibleLock = b;
                    }
                    else
                    {
                        s.YVisibleLock = false;
                    }
                });
                //update display range
                if (null != xRange)
                {
                    XVisibleRangeChart = xRange;
                    XVisibleRangeStack = xRange;
                    XDataRange = XVisibleRangeChart;
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart Create Time Series Error: " + ex.Message);
                MessageBox.Show("Unable to Create Time Series due to " + ex.Message, "RealTimeDataSettings.xml Error", MessageBoxButton.OK, MessageBoxImage.Error);
                //Environment.Exit(0);
            }
        }

        /// <summary>
        /// Creates the vertical marker.
        /// </summary>
        /// <param name="index">The index.</param>
        /// <param name="xCoordination">The x coordination.</param>
        public void CreateVerticalMarker(long index, double xCoordination)
        {
            _verticalmarker.Add(new VerticalMarker("Marker " + index.ToString(), xCoordination));
            OnPropertyChanged("VerticalMarkerList");
        }

        /// <summary>
        /// Creates the vertical marker.
        /// </summary>
        /// <param name="index">The index.</param>
        /// <param name="xCoordination">The x coordination.</param>
        /// <param name="comments">The comments.</param>
        public void CreateVerticalMarker(long index, double xCoordination, string comments)
        {
            _verticalmarker.Add(new VerticalMarker("Marker " + index.ToString(), xCoordination, comments));
            OnPropertyChanged("VerticalMarkerList");
        }

        /// <summary>
        /// Creates the XML vertical markers.
        /// </summary>
        /// <param name="filePath">The file path.</param>
        /// <param name="coordinationVerticalMarkers">The coordination vertical markers.</param>
        /// <param name="tooltipVerticalMarkers">The tooltip vertical markers.</param>
        public void CreateXMLVerticalMarkers(String filePath, List<double> coordinationVerticalMarkers, List<string> tooltipVerticalMarkers)
        {
            if (!Directory.Exists(filePath))
            {
                Directory.CreateDirectory(filePath);
            }
            String fileName = filePath + "\\" + "VerticalMarkers" + ".xml";

            if (!File.Exists(fileName))
            {
                //FileStream fs = new FileStream(fileName, FileMode.Create);
            }

            XmlDocument doc = new XmlDocument();
            XmlDeclaration xmlDeclaration = doc.CreateXmlDeclaration("1.0", "UTF-8", null);
            XmlElement root = doc.DocumentElement;
            doc.InsertBefore(xmlDeclaration, root);
            //doc.Save(fileName);

            XmlElement elementRoot = doc.CreateElement(string.Empty, "VerticalMarkers", string.Empty);
            elementRoot.SetAttribute("TotalNumber", coordinationVerticalMarkers.Count.ToString());
            doc.AppendChild(elementRoot);

            XmlElement elementChild;
            for (int i = 0; i < coordinationVerticalMarkers.Count; i++)
            {
                elementChild = doc.CreateElement(string.Empty, "Marker".ToString(), string.Empty);
                elementChild.SetAttribute("XCoordination", coordinationVerticalMarkers[i].ToString());
                elementRoot.AppendChild(elementChild);
                elementChild.SetAttribute("Comments", tooltipVerticalMarkers[i].ToString());
                elementRoot.AppendChild(elementChild);
            }
            doc.Save(fileName);
        }

        /// <summary>
        /// Deletes the vertical marker.
        /// </summary>
        /// <param name="index">The index.</param>
        public void DeleteVerticalMarker(int index)
        {
            _verticalmarker.RemoveAt(index);
            OnPropertyChanged("VerticalMarkerList");
        }

        /// <summary>
        /// Loads the XML vertical markers.
        /// </summary>
        /// <param name="file">The file.</param>
        /// <param name="coordinationVerticalMarkers">The coordination vertical markers.</param>
        /// <param name="tooltipVerticalMarkers">The tooltip vertical markers.</param>
        public void LoadXMLVerticalMarkers(String file, List<double> coordinationVerticalMarkers, List<string> tooltipVerticalMarkers)
        {
            XmlDocument doc = new XmlDocument();
            if (!File.Exists(file))
            {
                return;
            }
            doc.Load(file);
            string strTemp = string.Empty;
            XmlNodeList nodeList = doc.SelectNodes("/VerticalMarkers");
            XmlNodeList elemList = nodeList[0].SelectNodes("Marker");
            foreach (XmlNode node in elemList)
            {
                if (GetAttribute(node, doc, "XCoordination", ref strTemp))
                {
                    coordinationVerticalMarkers.Add(Convert.ToDouble(strTemp));
                }
                if (GetAttribute(node, doc, "Comments", ref strTemp))
                {
                    tooltipVerticalMarkers.Add(strTemp);
                }
            }
        }

        /// <summary>
        /// Fullfill relay commands for vertical markers.
        /// </summary>
        /// <param name="type"></param>
        public void MarkerCommandSwitch(object type)
        {
            string str = (string)type;
            switch (MarkerTypeDictionary[str])
            {
                case MarkerType.DisplayAll:
                case MarkerType.HideAll:
                    ChangeVerticalMarkersMode(MarkerTypeDictionary[str]);
                    break;
                case MarkerType.Load:
                    ChangeVerticalMarkersMode(MarkerTypeDictionary[str]);
                    break;
                case MarkerType.Save:
                    if (0 >= _chartSeries.Count)
                        return;
                    if (0 >= _chartSeries[0].DataSeries.Count)
                        return;
                    ChangeVerticalMarkersMode(MarkerTypeDictionary[str], 0);
                    break;
                case MarkerType.Add:
                case MarkerType.Delete:
                    ChangeVerticalMarkersMode(MarkerTypeDictionary[str]);
                    MarkerCommandSwitch("MARKER_SAVE");
                    break;
                case MarkerType.DeleteAll:
                    string msg = string.Format("Do you want to delete all vertical markers?");
                    if (MessageBoxResult.Yes == MessageBox.Show(msg, "Delete All Markers", MessageBoxButton.YesNo))
                    {
                        ChangeVerticalMarkersMode(MarkerTypeDictionary[str]);
                        MarkerCommandSwitch("MARKER_SAVE");
                    }
                    break;
                default:
                    break;
            }
        }

        /// <summary>
        /// Prints the screen.
        /// </summary>
        public void PrintScreen()
        {
            if (null != EventPrintScreen)
            {
                EventPrintScreen();
            }
        }

        /// <summary>
        /// Resets the scichart settings.
        /// </summary>
        public void ResetScichartSettings()
        {
            ChangeVerticalMarkersMode(MarkerType.DeleteAll);
            IsMeasureCursorVisible = false;
            IsFrameCursorVisible = false;
            ChangeVerticalMarkersMode(MarkerType.DisplayAll);
            IsVerticalMarkerVisible = true;
            Mouse.OverrideCursor = null;
            IsRollOverEnabled = false;
            EnableScrollbar(false);
            ImageCounterNumber = 0;
        }

        /// <summary>
        /// Revises the vertical marker Settings.
        /// </summary>
        /// <param name="index">The index.</param>
        /// <param name="xCoordination">The x coordination.</param>
        public void ReviseVerticalMarker(int index, double xCoordination)
        {
            //_verticalmarker[index].XCoordination = xCoordination;
            string nameTemp = _verticalmarker[index].Name;

            if (_verticalmarker[index].Comments != null)
            {
                string comments = _verticalmarker[index].Comments;
                _verticalmarker.RemoveAt(index);
                _verticalmarker.Insert(index, new VerticalMarker(nameTemp, xCoordination, comments));
            }
            else
            {
                _verticalmarker.RemoveAt(index);
                _verticalmarker.Insert(index, new VerticalMarker(nameTemp, xCoordination));
            }

            OnPropertyChanged("VerticalMarkerList");
        }

        /// <summary>
        /// Revises the vertical marker.
        /// </summary>
        /// <param name="index">The index.</param>
        /// <param name="comments">The comments.</param>
        public void ReviseVerticalMarker(int index, string comments)
        {
            //_verticalmarker[index].XCoordination = xCoordination;
            string nameTemp = _verticalmarker[index].Name;
            double coordination = _verticalmarker[index].XCoordination;
            _verticalmarker.RemoveAt(index);
            _verticalmarker.Insert(index, new VerticalMarker(nameTemp, coordination, comments));
            OnPropertyChanged("VerticalMarkerList");
        }

        private void RemoveChannel(object x)
        {
            RemovedSpectralChannel = _specChViewModels.IndexOf((SpectralViewModel)x);
        }

        private void ShowMeasureCursor()
        {
            IsMeasureCursorVisible = !IsMeasureCursorVisible;
        }

        private void ShowFrameCursor()
        {
            IsFrameCursorVisible = !IsFrameCursorVisible;
        }

        private void ExportFrameTimes()
        {
            if (RealTimeDataCapture.Instance.FrameTimes == null)
            {
                return;
            }
            //using (//var dialog = new System.Windows.Forms.FolderBrowserDialog())
            //{
            //System.Windows.Forms.DialogResult result = dialog.ShowDialog();
            Microsoft.Win32.SaveFileDialog dlg = new Microsoft.Win32.SaveFileDialog();
            dlg.Title = "Select a destination to export Frame Timings";
            dlg.InitialDirectory = "C:\\temp";//vm.OutputPath;
            dlg.FileName = "FrameTiming";
            dlg.DefaultExt = ".csv";
            dlg.AddExtension = true;
            dlg.Filter = "CSV Files (*.csv)|*.csv|Text Files (*.txt)|*.txt|All Files (*.*)|*.*";

            Nullable<bool> result = dlg.ShowDialog();

            try
            {
                if (false == result)
                {
                    return;
                }
                else
                {
                    string path = System.IO.Path.GetDirectoryName(dlg.FileName);

                    writeFrameTiming(path + "\\" + dlg.FileName, dlg);

                }
            }
            catch 
            {

            }

        }

        private void writeFrameTiming(string path, Microsoft.Win32.SaveFileDialog dlg)
        {
            string[] seperators = { "" , ".", ""};
            bool csv = true;
            
            if (!path.EndsWith(".csv"))
            {
                seperators[0] = "\t";
                seperators[1] = ":";
                seperators[2] = ":";
                csv = false;
            }

            using (System.IO.FileStream writer = (System.IO.FileStream)dlg.OpenFile())
            {
                int i = 1; //framecounter, start at one most frame as counted starting at one
                foreach (double time in RealTimeDataCapture.Instance.FrameTimes)
                {
                    TimeSpan t = TimeSpan.FromSeconds(time);
                    string millisAndUS = t.ToString(@"hh\:mm\:ss\:ffffff").Split(':')[3];
                    string millis = millisAndUS.Substring(0,3);
                    string USec = millisAndUS.Substring(3);
                    string hoursAndSeconds = t.ToString(@"hh\:mm\:ss");
                    string printCount = "";
                    if (false == csv)
                    {
                        printCount = "" + i;
                    }
                    byte[] info = new UTF8Encoding(true).GetBytes(printCount + seperators[0] + hoursAndSeconds + seperators[1] + millis + seperators[2] + USec + "\n");
                    writer.Write(info, 0, info.Length);
                    writer.Flush();
                    i++;
                }
            }
        }

        private void UpdateRollOverEnable()
        {
            IsRollOverEnabled = !IsRollOverEnabled;
            _channelViewModels.Each(s => { s.IsRollOverEnabled = !s.IsRollOverEnabled; });
            _specChViewModels.Each(s => { s.IsRollOverEnabled = !s.IsRollOverEnabled; });
        }

        /// <summary>
        /// Callback to update frequency domain chart series
        /// </summary>
        /// <param name="spectralData"></param>
        private void UpdateSpectral(ref RealTimeDataCapture.SpectralDataStruct spectralData)
        {
            try
            {
                ////not downsampling, might allow it in the future,
                ////if spectral analysis is taking too long
                //const ulong DATA_SCALE_RATIO = 3;
                ulong dataLength = 0; // (ulong)ChartViewSize[0] * DATA_SCALE_RATIO;
                ulong stepRatio = 1;  // (0 < dataLength) ? Math.Max(1, (ulong)(spectralData.freqLength / dataLength)) : 0;
                dataLength = ((0 < dataLength) && (1 < stepRatio)) ? dataLength : spectralData.freqLength;

                //copy buffers
                double[] xdata = new double[spectralData.freqLength];
                Marshal.Copy(spectralData.freqData, xdata, 0, (int)spectralData.freqLength);
                //use first diffrence for freq resolution
                DeltaFreqHz = (1 < spectralData.freqLength) ? (xdata[1] - xdata[0]) : 0;

                double[] sdata = new double[spectralData.specDataLength];
                Marshal.Copy(spectralData.specDataRe, sdata, 0, (int)spectralData.specDataLength);

                double[] vsdata = null;
                if (0 < spectralData.vspecDataLength)
                {
                    vsdata = new double[spectralData.vspecDataLength];
                    Marshal.Copy(spectralData.vSpecData, vsdata, 0, (int)spectralData.vspecDataLength);
                }

                double[] xfdata = null;
                if (0 < spectralData.freqFitLength)
                {
                    xfdata = new double[spectralData.freqFitLength];
                    Marshal.Copy(spectralData.freqFitData, xfdata, 0, (int)spectralData.freqFitLength);
                }
                ulong xfdataLength = (null == xfdata) ? 0 : (ulong)xfdata.Length;

                ulong[] signalTypeCount = new ulong[(int)SignalType.LAST_SIGNAL_TYPE];

                bool checkStackOnce = (IsStackedDisplay) ? true : false;

                for (int j = 0; j < _specSeriesMetadata.Count; j++)
                {
                    //determine if line is a fitting line
                    int isFittingLine = 0;
                    if (_specSeriesMetadata[j].PhysicalChannel.Contains(Constants.ThorRealTimeData.LORENTZIANFITX))
                    {
                        isFittingLine = 1;
                    }
                    else if (_specSeriesMetadata[j].PhysicalChannel.Contains(Constants.ThorRealTimeData.LORENTZIANFITY))
                    {
                        isFittingLine = 2;
                    }

                    ulong finalLength = (0 < isFittingLine) ? xfdataLength : dataLength;
                    double[] yRange = new double[2] { 0.0001, 0.0 };

                    DoubleSeries ds = new DoubleSeries();

                    switch (_specSeriesMetadata[j].SignalType)
                    {
                        case SignalType.SPECTRAL:
                            if (0 < spectralData.specDataLength)
                            {
                                for (ulong i = 0; i < finalLength; i++)
                                {
                                    var xy = new XYPoint();
                                    xy.X = xdata[(i * stepRatio)];
                                    xy.Y = Math.Max(sdata[(i * stepRatio) + (signalTypeCount[(int)SignalType.SPECTRAL] * spectralData.freqLength)], Constants.ThorRealTimeData.MIN_VALUE_LOG);
                                    yRange[0] = (yRange[0] >= xy.Y) ? xy.Y : yRange[0];
                                    yRange[1] = (yRange[1] <= xy.Y) ? xy.Y : yRange[1];
                                    ds.Add(xy);
                                }
                                signalTypeCount[(int)SignalType.SPECTRAL]++;
                            }
                            break;
                        case SignalType.SPECTRAL_VIRTUAL:
                            if (0 < spectralData.vspecDataLength)
                            {
                                for (ulong i = 0; i < finalLength; i++)
                                {
                                    var xy = new XYPoint();
                                    xy.X = (0 < isFittingLine) ? (xfdata[(i * stepRatio)]) : (xdata[(i * stepRatio)]);
                                    xy.Y = Math.Max(vsdata[(i * stepRatio) + (signalTypeCount[(int)SignalType.SPECTRAL_VIRTUAL] * spectralData.freqLength)], Constants.ThorRealTimeData.MIN_VALUE_LOG);
                                    yRange[0] = (yRange[0] >= xy.Y) ? xy.Y : yRange[0];
                                    yRange[1] = (yRange[1] <= xy.Y) ? xy.Y : yRange[1];
                                    ds.Add(xy);
                                }
                                signalTypeCount[(int)SignalType.SPECTRAL_VIRTUAL]++;
                            }
                            break;
                        default:
                            break;
                    }

                    //locate respective line to add doubleSeries:
                    SpectralViewModel specVM = (0 == isFittingLine) ?
                        (_specChViewModels.FirstOrDefault(s => 0 == s.ChannelName.CompareTo(_specSeriesMetadata[j].Alias))) :
                        (_specChViewModels.FirstOrDefault(s => (1 == isFittingLine) ?
                            (0 == s.ChannelName.CompareTo(_specSeriesMetadata.FirstOrDefault(x => x.PhysicalChannel.Contains(Constants.ThorRealTimeData.POWERSPECTRUMX)).Alias)) :
                            (0 == s.ChannelName.CompareTo(_specSeriesMetadata.FirstOrDefault(x => x.PhysicalChannel.Contains(Constants.ThorRealTimeData.POWERSPECTRUMY)).Alias))));

                    if (null != specVM)
                    {
                        if (0 < isFittingLine)
                        {
                            Application.Current.Dispatcher.Invoke(new Action(() =>
                            {
                                ((IXyDataSeries<double, double>)specVM.ChannelSeries2).Clear();
                                ((IXyDataSeries<double, double>)specVM.ChannelSeries2).Append(ds.XData, ds.YData);
                                ((IXyDataSeries<double, double>)specVM.ChannelSeries2).SeriesName = _specSeriesMetadata[j].Alias;

                            }));
                        }
                        else
                        {
                            Application.Current.Dispatcher.Invoke(new Action(() =>
                            {
                                ((IXyDataSeries<double, double>)specVM.ChannelSeries).Clear();
                                ((IXyDataSeries<double, double>)specVM.ChannelSeries).Append(ds.XData, ds.YData);
                                if ((specVM.YVisibleRange.AsDoubleRange().Min > yRange[0]) || (specVM.YVisibleRange.AsDoubleRange().Max < yRange[1]))
                                {
                                    double min = Math.Min(specVM.YVisibleRange.AsDoubleRange().Min, yRange[0]);
                                    double max = Math.Max(Math.Max(specVM.YVisibleRange.AsDoubleRange().Max, yRange[1]), 2 * Constants.ThorRealTimeData.MIN_VALUE_LOG);
                                    specVM.YVisibleRange = new DoubleRange(min, max);
                                }
                            }));
                        }
                    }
                }

                if ((int)ChartModes.REVIEW == ChartMode)
                    ReloadProvider("OTM");
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart UpdateSpectral Error: " + ex.Message);
                MessageBox.Show("An error occurs while doing spectral analysis, please check your settings.", "Spectral Analysis Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        /// <summary>
        /// Callback to update time domain chart series
        /// </summary>
        /// <param name="fileDataStruct"></param>
        private void UpdateTimeDomainData(ref RealTimeDataCapture.CompoundDataStruct fileDataStruct)
        {
            if ((0 == _dataSeriesSize) || (0 == fileDataStruct.gcLength))
                return;

            ulong[] signalTypeCount = new ulong[(int)SignalType.LAST_SIGNAL_TYPE];

            if (fileDataStruct.diLength > 0 && _diData?.Length != (int)fileDataStruct.gcLength)
            {
                _diData = new byte[fileDataStruct.gcLength];
            }
            if (fileDataStruct.aiLength > 0 && _aiData?.Length != (int)fileDataStruct.gcLength)
            {
                _aiData = new double[fileDataStruct.gcLength];
            }
            if (fileDataStruct.ciLength > 0 && _ciData?.Length != (int)fileDataStruct.gcLength)
            {
                _ciData = new int[fileDataStruct.gcLength];
            }
            if (fileDataStruct.viLength > 0 && _viData?.Length != (int)fileDataStruct.gcLength)
            {
                _viData = new double[fileDataStruct.gcLength];
            }

            ulong currentSize = 0;
            int progressPercent = 0;

            using (MainChartSurface.SuspendUpdates())
            {
                for (int j = 0; j < _chartSeriesMetadata.Count; j++)
                {
                    switch (_chartSeriesMetadata[j].SignalType)
                    {
                        case SignalType.DIGITAL_IN:
                            {
                                if (fileDataStruct.diLength > 0)
                                {

                                    int offset = (int)signalTypeCount[(int)SignalType.DIGITAL_IN] * (int)fileDataStruct.gcLength * sizeof(byte);
                                    Marshal.Copy(IntPtr.Add(fileDataStruct.diData, offset), _diData, 0, (int)fileDataStruct.gcLength);

                                    Application.Current.Dispatcher.Invoke(new Action(() =>
                                    {
                                        ((UniformXyDataSeries<byte>)ChartSeries[j].DataSeries).Append(_diData);
                                    }));

                                    FileVisibleYAxis[0] = 0;
                                    FileVisibleYAxis[1] = 1;
                                    signalTypeCount[(int)_chartSeriesMetadata[j].SignalType]++;
                                }
                                break;
                            }
                        case SignalType.ANALOG_IN:
                            {
                                if (fileDataStruct.aiLength > 0)
                                {
                                    int offset = (int)signalTypeCount[(int)SignalType.ANALOG_IN] * (int)fileDataStruct.gcLength * sizeof(double);
                                    Marshal.Copy(IntPtr.Add(fileDataStruct.aiData, offset), _aiData, 0, (int)fileDataStruct.gcLength);

                                    Application.Current.Dispatcher.Invoke(new Action(() =>
                                    {
                                        ((UniformXyDataSeries<double>)ChartSeries[j].DataSeries).Append(_aiData);
                                    }));

                                    signalTypeCount[(int)_chartSeriesMetadata[j].SignalType]++;
                                }
                                break;
                            }
                        case SignalType.COUNTER_IN:
                            {
                                if (fileDataStruct.ciLength > 0)
                                {
                                    int offset = (int)signalTypeCount[(int)SignalType.COUNTER_IN] * (int)fileDataStruct.gcLength * sizeof(int);
                                    Marshal.Copy(IntPtr.Add(fileDataStruct.ciData, offset), _ciData, 0, (int)fileDataStruct.gcLength);
                                    for (int nC = 0; nC < IsCounterLinePlotEnabled.Count; nC++)
                                    {
                                        if (IsCounterLinePlotEnabled[nC])
                                        {
                                            Application.Current.Dispatcher.Invoke(new Action(() =>
                                            {
                                                ((UniformXyDataSeries<int>)ChartSeries[j].DataSeries).Append(_ciData);
                                            }));
                                        }
                                    }

                                    ImageCounterNumber = _ciData[fileDataStruct.gcLength - 1];
                                    signalTypeCount[(int)_chartSeriesMetadata[j].SignalType]++;
                                }
                            }
                            break;
                        case SignalType.VIRTUAL:
                            {
                                if (fileDataStruct.viLength > 0)
                                {
                                    int offset = (int)signalTypeCount[(int)SignalType.VIRTUAL] * (int)fileDataStruct.gcLength * sizeof(double);
                                    Marshal.Copy(IntPtr.Add(fileDataStruct.viData, offset), _viData, 0, (int)fileDataStruct.gcLength);

                                    Application.Current.Dispatcher.Invoke(new Action(() =>
                                    {
                                        ((UniformXyDataSeries<double>)ChartSeries[j].DataSeries).Append(_viData);
                                    }));

                                    signalTypeCount[(int)_chartSeriesMetadata[j].SignalType]++;
                                }
                                break;
                            }
                    }

                    Application.Current.Dispatcher.Invoke(new Action(() =>
                    {

                        currentSize = Math.Max((UInt64)(ChartSeries[j].DataSeries).Count, currentSize);

                    }));
                    progressPercent = (int)(currentSize * Constants.ThorRealTimeData.HUNDRED_PERCENT / _dataSeriesSize);
                }
            }
            //user request to cancel:
            if (_bwLoader.CancellationPending == true)
            {
                return;
            }
            //inform current progress:
            Interlocked.Exchange(ref _progressPercentage, progressPercent);
        }

        /// <summary>
        /// keep only the bottom line chart with x axis
        /// </summary>
        private void XAxis_Changed()
        {
            _channelViewModels.Each(s => { s.XAxisVisible = false; });
            _channelViewModels.Last(s => { s.XAxisVisible = true; }, (s => true == s.IsVisible));

            _specChViewModels.Each(s => { s.XAxisVisible = false; });
            _specChViewModels.Last(s => { s.XAxisVisible = true; }, (s => true == s.IsVisible));
        }

        private void _bwSpecAnalyzer_DoWork(object sender, DoWorkEventArgs e)
        {
            if (!_bwLoadDone)
            {
                //invoke once, use flag of 1 to avoid repeating
                RealTimeDataCapture.Instance.LoadSpectral();
                _bwLoadDone = true;
                e.Result = e.Argument;
            }
        }

        private void _bwSpecAnalyzer_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            // check error, check cancel, then use result
            if ((e.Error != null) || (e.Cancelled))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart CalculateSpectral Error: " + e.Error.Message);
            }
            else
            {
                UpdateSpectralCharts((bool)e.Result);
            }
        }

        #endregion Methods
    }

    /// <summary>
    /// Meta data for chart series
    /// </summary>
    public class SeriesMetadata
    {
        #region Properties

        public string Alias
        {
            get;
            set;
        }

        public string Group
        {
            get;
            set;
        }

        public Color LineColor
        {
            get;
            set;
        }

        public string PhysicalChannel
        {
            get;
            set;
        }

        public SignalType SignalType
        {
            get;
            set;
        }

        #endregion Properties
    }
}