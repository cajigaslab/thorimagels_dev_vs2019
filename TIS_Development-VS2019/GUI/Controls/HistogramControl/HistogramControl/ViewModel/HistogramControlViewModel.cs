namespace HistogramControl.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.Linq;
    using System.Windows;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Shapes;
    using System.Windows.Threading;

    using SciChart.Charting.Model.ChartSeries;
    using SciChart.Charting.Model.DataSeries;
    using SciChart.Charting.Visuals.Axes;
    using SciChart.Charting.Visuals.Axes.LogarithmicAxis;
    using SciChart.Charting.Visuals.PaletteProviders;
    using SciChart.Data.Model;

    using ThorLogging;

    using ThorSharedTypes;

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

    public partial class HistogramControlViewModel : VMBase
    {
        #region Fields

        public bool _continuousAuto = false;

        const int MAX_HISTOGRAM_HEIGHT = 250;
        const int MAX_HISTOGRAM_WIDTH = 444;
        const int MIN_HISTOGRAM_HEIGHT = 115;
        const int MIN_HISTOGRAM_POS = 0;
        const int MIN_HISTOGRAM_WIDTH = 210;
        const double MIN_LOG = 0.00001;

        private bool _allHistogramsExpanded = false;
        bool _allowImageUpdate = true;
        private double _annotationBoxWidth;
        private string _autoButtonContent = "Auto";
        ICommand _autoCommand;
        private int _binThreshold = 255;
        private double _bottomPercentileReduction = 0;
        ICommand _bpLeftCommand;
        ICommand _bpRightCommand;
        private string _continuousAutoContent = "Continuous Auto";
        double _dataSeriesYMax = 1;
        private double _gamma = 1.0;
        Style _gridLineStyle;
        private ObservableCollection<HistogramChannel> _histogramChannels;
        Visibility _histogramVisibility = Visibility.Collapsed;
        int[] _indexes;
        private bool _isLineChart = false;
        private bool _largeHistogram = false;
        IAxis _linearAxis;
        ConstantTickProvider _linearTickProvider;
        bool _log = true;
        IAxis _logAxis;
        ConstantTickProvider _logTickProvider;
        private int _numberOfBins = 256;
        private int _numGridLines;
        private ICommand _openAutoAdvancedWindowCommand;
        private double _rangeHeight;
        ICommand _resetCommand;
        private double _scalingMax;
        private double _scalingMin;
        private int _scichartSurfaceHeight = MIN_HISTOGRAM_HEIGHT;
        private int _scichartSurfaceWidth = MIN_HISTOGRAM_WIDTH;
        int _shiftValue = 6; //most common case is a 14bit pixel making the shift value 6
        private double _thresholdBP;
        private double _thresholdWP;
        private double _topPercentileReduction = 0;
        private string _unitSymbol = string.Empty;
        ICommand _wpLeftCommand;
        ICommand _wpRightCommand;
        RemappedLabelProvider _xAxesLabelProvider;
        ConstantTickProvider _xAxesTickProvider;
        IAxis _xAxis;
        RemappedLabelProvider _yAxesLabelProvider;
        int _yAxesMinWidth;

        #endregion Fields

        #region Constructors

        public HistogramControlViewModel()
        {
            _histogramChannels = new ObservableCollection<HistogramChannel>();

            RenderableSeriesViewModels = new ObservableCollection<IRenderableSeriesViewModel>();

            // Tick providers
            _linearTickProvider = new ConstantTickProvider();
            _linearTickProvider.LogBase = 0;
            _logTickProvider = new ConstantTickProvider();
            _logTickProvider.LogBase = 10;
            _xAxesTickProvider = new ConstantTickProvider();
            _xAxesTickProvider.LogBase = 0;
            // Label providers
            _yAxesLabelProvider = new RemappedLabelProvider();
            _yAxesLabelProvider.RoundTo = _log ? 3 : 0;
            _yAxesLabelProvider.Slope = 1.0;
            _yAxesLabelProvider.Offset = 0.0;
            _xAxesLabelProvider = new RemappedLabelProvider();
            _xAxesLabelProvider.RoundTo = 0;
            _xAxesLabelProvider.Slope = 1.0;
            _xAxesLabelProvider.Offset = 0.0;
            // Create grid style for axes
            _gridLineStyle = new Style(typeof(Line));
            Setter setter1 = new Setter(Line.StrokeProperty, Brushes.White);
            _gridLineStyle.Setters.Add(setter1);
            DoubleCollection col = new DoubleCollection() { 1, 5 };
            Setter setter2 = new Setter(Line.StrokeDashArrayProperty, col);
            _gridLineStyle.Setters.Add(setter2);

            // TODO: Adjust the width based on FrameData's data length and the Scichart axes label font / style
            _yAxesMinWidth = 60;

            _linearAxis = new NumericAxis
            {
                DrawMinorTicks = false,
                DrawMinorGridLines = false,
                DrawMajorGridLines = false,
                DrawMajorTicks = false,
                DrawLabels = false,
                DrawMajorBands = false,
                AxisAlignment = AxisAlignment.Left,
                HorizontalAlignment = HorizontalAlignment.Right,
                AutoRange = AutoRange.Never,
                MajorGridLineStyle = _gridLineStyle,
                TickProvider = _linearTickProvider,
                LabelProvider = _yAxesLabelProvider
            };

            _logAxis = new LogarithmicNumericAxis
            {
                LogarithmicBase = 10.0,
                DrawMinorTicks = false,
                DrawMinorGridLines = false,
                DrawMajorGridLines = false,
                DrawMajorTicks = false,
                DrawLabels = false,
                DrawMajorBands = false,
                AxisAlignment = AxisAlignment.Left,
                AutoRange = AutoRange.Never,
                MajorGridLineStyle = _gridLineStyle,
                TickProvider = _logTickProvider,
                LabelProvider = _yAxesLabelProvider
            };

            _xAxis = new NumericAxis
            {
                DrawMinorTicks = false,
                DrawMinorGridLines = false,
                DrawMajorGridLines = false,
                DrawMajorTicks = false,
                DrawLabels = false,
                DrawMajorBands = false,
                AutoRange = AutoRange.Never,
                VisibleRange = new DoubleRange(0.0, 256.0),
                MajorGridLineStyle = _gridLineStyle,
                TickProvider = _xAxesTickProvider,
                LabelProvider = _xAxesLabelProvider
            };

            ThresholdWP = NumberOfBins - 1;
            ThresholdBP = MIN_HISTOGRAM_POS;

            _indexes = Enumerable.Range(MIN_HISTOGRAM_POS, NumberOfBins).ToArray();
        }

        #endregion Constructors

        #region Events

        public event Action<ImageIdentifier, bool> AutoClicked;

        public event Action<ImageIdentifier, double> BlackPointThresholdUpdated;

        public event Action<ImageIdentifier, bool> ContinuousAutoChanged;

        public event Action<bool> ExpandAllHistograms;

        public event Action<ImageIdentifier, double> GammaUpdated;

        public event Action<ImageIdentifier, double, double> HistogramFittingUpdated;

        public event Action<bool> WhitePointIncreased;

        public event Action<ImageIdentifier, double> WhitePointThresholdUpdated;

        #endregion Events

        #region Properties

        public bool AllHistogramsExpanded
        {
            get
            {
                return _allHistogramsExpanded;
            }
            set
            {
                _allHistogramsExpanded = value;
                ExpandAllHistograms?.Invoke(value);
                OnPropertyChanged("AllHistogramsExpanded");
            }
        }

        public double AnnotationBoxWidth
        {
            get
            {
                return _annotationBoxWidth;
            }
            set
            {
                _annotationBoxWidth = value;
                OnPropertyChanged("BendableLineGeometry");
            }
        }

        ///<summary>
        /// This is used by ViewControlViewModel to update the BP without disabling Continuous Auto and invoking BlackPointUpdated
        ///</summary>
        public double AutoBP
        {
            get
            {
                return ThresholdBP;
            }
            set
            {
                if (MIN_HISTOGRAM_POS <= value && NumberOfBins > value)
                {
                    foreach (var histogramChannel in _histogramChannels)
                    {
                        histogramChannel.PaletteProvider.ThresholdBP = value;
                    }
                    _thresholdBP = value;
                    OnPropertyChanged("ThresholdBP");
                    OnPropertyChanged("ThresholdBPString");
                    OnPropertyChanged("BlackPoint");
                    RedrawHistogram();
                }
            }
        }

        public string AutoButtonContent
        {
            get
            {
                return _autoButtonContent;
            }
            set
            {
                _autoButtonContent = value;
                OnPropertyChanged("AutoButtonContent");
            }
        }

        public ICommand AutoCommand
        {
            get
            {
                if (null != _autoCommand)
                {
                    return _autoCommand;
                }

                _autoCommand = new RelayCommand(() =>
                {
                    ContinuousAuto = false;
                    foreach (var channel in _histogramChannels)
                    {
                        var imageIdentifier = channel.ImageIdentifier;
                        AutoClicked?.Invoke(imageIdentifier, true);
                    }
                });

                return _autoCommand;
            }
        }

        ///<summary>
        /// This is used by ViewControlViewModel to update the WP without disabling Continuous Auto and invoking WhitePointUpdated
        ///</summary>
        public double AutoWP
        {
            get
            {
                return ThresholdWP;
            }
            set
            {
                if (MIN_HISTOGRAM_POS <= value && NumberOfBins > value)
                {
                    if (value > _thresholdWP)
                    {
                        WhitePointIncreased?.Invoke(true);
                    }
                    foreach (var histogramChannel in _histogramChannels)
                    {
                        histogramChannel.PaletteProvider.ThresholdWP = value;
                    }
                    _thresholdWP = value;
                    OnPropertyChanged("ThresholdWP");
                    OnPropertyChanged("WhitePoint");
                    RedrawHistogram();
                }
            }
        }

        public PathGeometry BendableLineGeometry
        {
            get
            {
                if (AnnotationBoxWidth == 0)
                {
                    return null;
                }

                PathGeometry pathGeometry = new PathGeometry();
                PathFigure pathFigure;
                double heightGamma = Math.Pow(ScichartSurfaceHeight, Gamma);
                double step = ScichartSurfaceHeight / AnnotationBoxWidth;
                if (ThresholdWP >= ThresholdBP)
                {
                    pathFigure = new PathFigure
                    {
                        StartPoint = new Point(5, ScichartSurfaceHeight) // Starting point of the line
                    };

                    for (double x = 0; x <= AnnotationBoxWidth; x++)
                    {
                        double y = ScichartSurfaceHeight - Math.Pow(step * x, Gamma) * ScichartSurfaceHeight / heightGamma;
                        pathFigure.Segments.Add(new LineSegment(new Point(x, y), true));
                    }
                }
                else
                {
                    pathFigure = new PathFigure
                    {
                        StartPoint = new Point(AnnotationBoxWidth, ScichartSurfaceHeight) // Starting point of the line
                    };

                    for (double x = 0; x <= AnnotationBoxWidth; x++)
                    {
                        double y = ScichartSurfaceHeight - Math.Pow(step * x, Gamma) * ScichartSurfaceHeight / heightGamma;
                        pathFigure.Segments.Add(new LineSegment(new Point(AnnotationBoxWidth - x, y), true));
                    }
                }

                pathGeometry.Figures.Add(pathFigure);

                return pathGeometry;
            }
        }

        // This is read from Application Settings -> ReducedBinValue, it is used only for histogram
        // autoscale. Autoscale will ignore any values above the set bin. This is useful for the
        // cameras, which might have burn-in pixels
        public int BinThreshold
        {
            get
            {
                return _binThreshold;
            }
            set
            {
                if (value < NumberOfBins && value > MIN_HISTOGRAM_POS)
                {
                    _binThreshold = value;
                }
            }
        }

        ///<summary>
        /// This is the value to be displayed in the BP label annotation
        ///</summary>
        public int BlackPoint
        {
            get
            {
                return (int)GetUnitValueFromData(ThresholdBP);
            }
            set
            {
                ThresholdBP = (int)GetDataValueFromUnit(value);
            }
        }

        public double BottomPercentileReduction
        {
            get
            {
                return _bottomPercentileReduction;
            }
            set
            {
                _bottomPercentileReduction = value;
                AutoButtonContent = (_topPercentileReduction > 0 || _bottomPercentileReduction > 0) ? "Fit" : "Auto";
                ContinuousAutoContent = (_topPercentileReduction > 0 || _bottomPercentileReduction > 0) ? "Continuous Fit" : "Continuous Auto";
            }
        }

        public ICommand BPLeftCommand
        {
            get => _bpLeftCommand ?? (_bpLeftCommand = new RelayCommand(() => { ThresholdBP--; }));
        }

        public Thickness BPMarkerMargin
        {
            get
            {
                return new Thickness(-5, ScichartSurfaceHeight - 11, 0, 0);
            }
        }

        public ICommand BPRightCommand
        {
            get => _bpRightCommand ?? (_bpRightCommand = new RelayCommand(() => { ThresholdBP++; }));
        }

        public string ChannelName
        {
            get;
            set;
        }

        public bool ContinuousAuto
        {
            get
            {
                return _continuousAuto;
            }
            set
            {
                if (value != _continuousAuto)
                {
                    _continuousAuto = value;
                    foreach (var channel in _histogramChannels)
                    {
                        var imageIdentifer = channel.ImageIdentifier;
                        ContinuousAutoChanged?.Invoke(imageIdentifer, ContinuousAuto);
                    }

                    OnPropertyChanged("ContinuousAuto");
                }
            }
        }

        public string ContinuousAutoContent
        {
            get
            {
                return _continuousAutoContent;
            }
            set
            {
                _continuousAutoContent = value;
                OnPropertyChanged("ContinuousAutoContent");
            }
        }

        public double Gamma
        {
            get
            {
                return _gamma;
            }
            set
            {
                if (value > 0 && value < 16)
                {
                    _gamma = value;
                    if (_allowImageUpdate)
                    {
                        foreach (var channel in _histogramChannels)
                        {
                            var imageIdentifer = channel.ImageIdentifier;
                            GammaUpdated?.Invoke(imageIdentifer, Gamma);
                        }
                    }
                    OnPropertyChanged("Gamma");
                    OnPropertyChanged("BendableLineGeometry");
                }
            }
        }

        public ObservableCollection<HistogramChannel> HistogramChannels
        {
            get => _histogramChannels;
        }

        public int HistogramID
        {
            get;
            set;
        }

        public Visibility HistogramVisibility
        {
            get
            {
                return _histogramVisibility;
            }
            set
            {
                _histogramVisibility = value;
                OnPropertyChanged("HistogramVisibility");
            }
        }

        public IAxis HistogramXAxis
        {
            get
            {
                return _xAxis;
            }
        }

        public IAxis HistogramYAxis
        {
            get
            {
                return Log ? _logAxis : _linearAxis;
            }
        }

        public bool IsChannelVisibilityVisible
        {
            get
            {
                // Only show the individual channel visibility toggles if there is more than one histogram channel
                return _histogramChannels.Count > 1;
            }
        }

        public bool IsLineChart
        {
            get => _isLineChart;
            set
            {
                _isLineChart = value;
                foreach (var histogramChannel in _histogramChannels)
                {
                    histogramChannel.IsLineChart = _isLineChart;
                }
            }
        }

        public bool LargeHistogram
        {
            get
            {
                return _largeHistogram;
            }
            set
            {
                _largeHistogram = value;
                ScichartSurfaceWidth = _largeHistogram ? MAX_HISTOGRAM_WIDTH : MIN_HISTOGRAM_WIDTH;
                ScichartSurfaceHeight = _largeHistogram ? MAX_HISTOGRAM_HEIGHT : MIN_HISTOGRAM_HEIGHT;
                UpdateAxes();
                OnPropertyChanged("LargeHistogram");
            }
        }

        public bool Log
        {
            get
            {
                return _log;
            }
            set
            {
                _log = value;
                _yAxesLabelProvider.RoundTo = _log ? 3 : 0;
                OnPropertyChanged("Log");
                OnPropertyChanged("HistogramYAxis");
                RedrawHistogram();
            }
        }

        //When using log scale axis any value can't be 0. Otherwise an exception is thrown for trying to set a value to infinity
        public double MinLog
        {
            get
            {
                return MIN_LOG;
            }
        }

        public int NumberOfBins
        {
            get
            {
                return _numberOfBins;
            }
            set
            {
                _numberOfBins = value;
            }
        }

        public int NumGridLines
        {
            get => _numGridLines;
            set
            {
                _numGridLines = value;
                _xAxesTickProvider.MajorTickCount = _numGridLines;
                _linearTickProvider.MajorTickCount = _numGridLines;
                _logTickProvider.MajorTickCount = _numGridLines;
                UpdateAxes();
                OnPropertyChanged("NumGridLines");
            }
        }

        public ICommand OpenAutoAdvancedWindowCommand
        {
            get => _openAutoAdvancedWindowCommand ?? (_openAutoAdvancedWindowCommand = new RelayCommand(() => OpenAutoAdvancedWindow()));
        }

        public double RangeHeight
        {
            get
            {
                //Only recalculate the range height if the max value is greater than the 1.02 ceiling or if it drops below 10% of the current ceiling
                if (_rangeHeight > _dataSeriesYMax && _rangeHeight / 1.1 < _dataSeriesYMax)
                {
                    return _rangeHeight;
                }
                _rangeHeight = _dataSeriesYMax * 1.02;
                return _rangeHeight;
            }
        }

        public ObservableCollection<IRenderableSeriesViewModel> RenderableSeriesViewModels
        {
            get; set;
        }

        public ICommand ResetCommand
        {
            get => _resetCommand ?? (_resetCommand = new RelayCommand(() =>
            {
                //We need to load the new values before we trigger an image upddate. Otherwise the bitmap doesn't update correctly
                _allowImageUpdate = false;
                Gamma = 1.0;
                ThresholdBP = MIN_HISTOGRAM_POS;
                ThresholdWP = NumberOfBins - 1;
                foreach (var channel in _histogramChannels)
                {
                    var imageIdentifer = channel.ImageIdentifier;
                    BlackPointThresholdUpdated?.Invoke(imageIdentifer, ThresholdBP);
                    WhitePointThresholdUpdated?.Invoke(imageIdentifer, ThresholdWP);
                    GammaUpdated?.Invoke(imageIdentifer, Gamma);
                }
                _allowImageUpdate = true;
            }));
        }

        public double ScalingMax
        {
            get => _scalingMax;
            set
            {
                _scalingMax = value;
                _xAxesLabelProvider.Slope = (_scalingMax - _scalingMin) / 256.0;
                OnPropertyChanged("WhitePoint");
                OnPropertyChanged("BlackPoint");
            }
        }

        public double ScalingMin
        {
            get => _scalingMin;
            set
            {
                _scalingMin = value;
                _xAxesLabelProvider.Slope = (_scalingMax - _scalingMin) / 256.0;
                _xAxesLabelProvider.Offset = _scalingMin;
                OnPropertyChanged("WhitePoint");
                OnPropertyChanged("BlackPoint");
            }
        }

        public int ScichartSurfaceHeight
        {
            get
            {
                return _scichartSurfaceHeight;
            }
            set
            {
                _scichartSurfaceHeight = value;
                OnPropertyChanged("ScichartSurfaceHeight");
                OnPropertyChanged("BendableLineGeometry");
                OnPropertyChanged("BPMarkerMargin");
            }
        }

        public int ScichartSurfaceWidth
        {
            get
            {
                return _scichartSurfaceWidth;
            }
            set
            {
                _scichartSurfaceWidth = value;
                OnPropertyChanged("ScichartSurfaceWidth");
            }
        }

        public int ShiftValue
        {
            get
            {
                {
                    return _shiftValue;
                }
            }
            set
            {
                if (_shiftValue != value)
                {
                    _shiftValue = value;
                }

                // The black point and white point will be changed by the new shift value
                foreach (var channel in _histogramChannels)
                {
                    var imageIdentifier = channel.ImageIdentifier;
                    BlackPointThresholdUpdated?.Invoke(imageIdentifier, ThresholdBP);
                    WhitePointThresholdUpdated?.Invoke(imageIdentifier, ThresholdWP);
                }
                OnPropertyChanged("BlackPoint");
                OnPropertyChanged("WhitePoint");
            }
        }

        ///<summary>
        /// This is the value of the GUI control for the BP draggable line. Input: [0-Max Num of Bins]. We pass through the value to the PaletteProvider, which 
        /// alters the color of columns on render
        ///</summary>
        public double ThresholdBP
        {
            get
            {
                // all palettes should have the same thresholdBP
                // _linePalette is the only one garunteed to exist
                return _thresholdBP;
            }
            set
            {
                if (MIN_HISTOGRAM_POS <= value && NumberOfBins > value)
                {
                    foreach (var histogramChannel in _histogramChannels)
                    {
                        histogramChannel.PaletteProvider.ThresholdBP = value;
                    }
                    _thresholdBP = value;
                    OnPropertyChanged("ThresholdBP");
                    OnPropertyChanged("ThresholdBPString");
                    OnPropertyChanged("BlackPoint");
                    RedrawHistogram();
                    ContinuousAuto = false;
                    if (_allowImageUpdate)
                    {
                        foreach (var channel in _histogramChannels)
                        {
                            var imageIdentifer = channel.ImageIdentifier;
                            BlackPointThresholdUpdated?.Invoke(imageIdentifer, ThresholdBP);
                        }
                    }
                }
            }
        }

        public string ThresholdBPString
        {
            get
            {
                return ThresholdBP.ToString();
            }
        }

        ///<summary>
        /// This is the value of the GUI control for the WP draggable line. Input: [0-Max Num of Bins]. We pass through the value to the PaletteProvider, which 
        /// alters the color of columns on render
        ///</summary>
        public double ThresholdWP
        {
            get
            {
                // all palettes should have the same thresholdWP
                // _linePalette is the only one guaranteed to exist
                return _thresholdWP;
            }
            set
            {
                if (MIN_HISTOGRAM_POS <= value && NumberOfBins > value)
                {
                    if (value > _thresholdWP)
                    {
                        WhitePointIncreased?.Invoke(true);
                    }
                    foreach (var histogramChannel in _histogramChannels)
                    {
                        histogramChannel.PaletteProvider.ThresholdWP = value;
                    }
                    _thresholdWP = value;
                    OnPropertyChanged("ThresholdWP");
                    OnPropertyChanged("WhitePoint");
                    RedrawHistogram();
                    ContinuousAuto = false;
                    if (_allowImageUpdate)
                    {
                        foreach (var channel in _histogramChannels)
                        {
                            var imageIdentifer = channel.ImageIdentifier;
                            WhitePointThresholdUpdated?.Invoke(imageIdentifer, ThresholdWP);
                        }
                    }
                }
            }
        }

        public double TopPercentileReduction
        {
            get
            {
                return _topPercentileReduction;
            }
            set
            {
                _topPercentileReduction = value;
                AutoButtonContent = (_topPercentileReduction > 0 || _bottomPercentileReduction > 0) ? "Fit" : "Auto";
                ContinuousAutoContent = (_topPercentileReduction > 0 || _bottomPercentileReduction > 0) ? "Continuous Fit" : "Continuous Auto";
            }
        }

        public string UnitSymbol
        {
            get => _unitSymbol;
            set
            {
                _unitSymbol = value;
                OnPropertyChanged("UnitSymbol");
            }
        }

        ///<summary>
        /// This is the value to be displayed in the WP label annotation
        ///</summary>
        public int WhitePoint
        {
            get
            {
                return (int)GetUnitValueFromData(ThresholdWP);
            }
            set
            {
                ThresholdWP = (int)GetDataValueFromUnit(value);
            }
        }

        public ICommand WPLeftCommand
        {
            get => _wpLeftCommand ?? (_wpLeftCommand = new RelayCommand(() => { ThresholdWP--; }));
        }

        public ICommand WPRightCommand
        {
            get => _wpRightCommand ?? (_wpRightCommand = new RelayCommand(() => { ThresholdWP++; }));
        }

        #endregion Properties

        #region Methods

        public void ActivateChannel(int dataChannel, Color color, double opacity, ImageIdentifier imageIdentifier)
        {
            var activeChannel = GetChannelOrNull(dataChannel);
            if (activeChannel != null)
            {
                activeChannel.FillColor = color;
                activeChannel.Opacity = opacity;
                activeChannel.ImageIdentifier = imageIdentifier;
            }
            else
            {
                var newChannel = new HistogramChannel(color, opacity, imageIdentifier, dataChannel);
                newChannel.PaletteProvider.ThresholdWP = _thresholdWP;
                newChannel.PaletteProvider.ThresholdBP = _thresholdBP;
                _histogramChannels.Add(newChannel);
                ResetRenderableViewModels();
                OnPropertyChanged("HistogramChannels");
                OnPropertyChanged("IsChannelVisibilityVisible");
                // need to tell observers that BP / WP changed to trigger pixel data adjustments
                BlackPointThresholdUpdated?.Invoke(imageIdentifier, ThresholdBP);
                WhitePointThresholdUpdated?.Invoke(imageIdentifier, ThresholdWP);
                GammaUpdated?.Invoke(imageIdentifier, Gamma);
            }
            RedrawHistogram();
        }

        public void ApplyChannelSettings(HistogramChannelSettings settings)
        {
            foreach (var channel in _histogramChannels)
            {
                if (channel.ImageIdentifier.AsKeyString() == settings.ImageIdentifierKey)
                {
                    ThresholdBP = settings.ThresholdBP;
                    ThresholdWP = settings.ThresholdWP;
                    Gamma = settings.Gamma;
                    ContinuousAuto = settings.ContinuousAuto;
                    Log = settings.Log;
                    TopPercentileReduction = settings.TopPercentileReduction;
                    BottomPercentileReduction = settings.BottomPercentileReduction;
                    UnitSymbol = settings.UnitSymbol;
                    NumGridLines = settings.NumGridLines;
                }
            }
        }

        public void DeactivateAllChannels()
        {
            _histogramChannels.Clear();
            ResetRenderableViewModels();
            OnPropertyChanged("HistogramChannels");
            OnPropertyChanged("IsChannelVisibilityVisible");
        }

        public void DeactivateChannel(int dataChannel)
        {
            foreach (var channel in _histogramChannels)
            {
                if (channel.DataChannel == dataChannel)
                {
                    _ = _histogramChannels.Remove(channel);
                    ResetRenderableViewModels();
                    break;
                }
            }
            OnPropertyChanged("HistogramChannels");
            OnPropertyChanged("IsChannelVisibilityVisible");
        }

        public List<int> GetAllDataChannels()
        {
            List<int> dataChannels = new List<int>();
            foreach (var channel in _histogramChannels)
            {
                dataChannels.Add(channel.DataChannel);
            }

            return dataChannels;
        }

        public HistogramChannelSettings GetChannelSettings(int dataChannel)
        {
            HistogramChannel channel = GetChannelOrNull(dataChannel);
            if (channel == null)
            {
                return null;
            }

            var settings = new HistogramChannelSettings();

            settings.ImageIdentifierKey = channel.ImageIdentifier.AsKeyString();
            settings.ThresholdBP = ThresholdBP;
            settings.ThresholdWP = ThresholdWP;
            settings.Gamma = Gamma;
            settings.ContinuousAuto = ContinuousAuto;
            settings.Log = Log;
            settings.TopPercentileReduction = TopPercentileReduction;
            settings.BottomPercentileReduction = BottomPercentileReduction;
            settings.UnitSymbol = UnitSymbol;
            settings.NumGridLines = NumGridLines;

            return settings;
        }

        public Color GetDataBrushColor(int dataChannel)
        {
            HistogramChannel channel = GetChannelOrNull(dataChannel);
            if (channel == null)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, $"Cannot get brush color for histogram channel {dataChannel} because it does not exist.");
                return Colors.Gray;
            }
            return channel.FillColor;
        }

        /// <summary>
        /// Retrieve the image identification info for a given channel. 
        /// Returns null if channel does not exist.
        /// </summary>
        /// <param name="dataChannel"></param>
        /// <returns></returns>
        public ImageIdentifier GetImageIdentifier(int dataChannel)
        {
            HistogramChannel channel = GetChannelOrNull(dataChannel);
            if (channel == null)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, $"Cannot get image identifier for histogram channel {dataChannel} because it does not exist.");
                return null;
            }
            return channel.ImageIdentifier;
        }

        // We use a PaletteProvider type to override render colors for the column depending on threshold
        public IPaletteProvider GetThresholdPaletteProvider(int dataChannel)
        {
            HistogramChannel channel = GetChannelOrNull(dataChannel);
            return channel == null ? null : (IPaletteProvider)channel.PaletteProvider;
        }

        public bool IsChannelActive(int dataChannel)
        {
            HistogramChannel channel = GetChannelOrNull(dataChannel);
            return channel != null;
        }

        public void OpenAutoAdvancedWindow()
        {
            AutoAdvancedWindow win = new AutoAdvancedWindow(this);
            if (false == win.ShowDialog())
            {
                return;
            }
            foreach (var channel in _histogramChannels)
            {
                var imageIdentifer = channel.ImageIdentifier;
                HistogramFittingUpdated?.Invoke(imageIdentifer, TopPercentileReduction, BottomPercentileReduction);
            }
        }

        public void RedrawHistogram()
        {
            if (null == _histogramChannels)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Histogram could not be redrawn because there were none to draw.");
                return;
            }

            _ = (Application.Current?.Dispatcher.InvokeAsync((Action)(() =>
              {
                  if (RangeHeight > MinLog)
                  {
                      HistogramYAxis.VisibleRange?.SetMinMax(MinLog, RangeHeight);
                  }
              }), DispatcherPriority.Normal));
        }

        /// <summary>
        /// Array Containing Bin Values
        /// </summary>
        public void SetData(int dataChannel, int[] data)
        {
            if (null == data)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, $"Cannot set data for histogram channel {dataChannel} to null.");
                return;
            }

            HistogramChannel channel = GetChannelOrNull(dataChannel);
            if (channel == null)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, $"Cannot set data for histogram channel {dataChannel} because it does not exist.");
                return;
            }

            if (_indexes.Length != data.Length)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, $"Cannot set data for histogram channel {dataChannel} because the data argument had length {data.Length} which does not match recorded index length {_indexes.Length}.");
                return;
            }

            UniformXyDataSeries<int> series = new UniformXyDataSeries<int>(0, null) { FifoCapacity = null, XStart = 0, XStep = 1 };
            series.Append(data);
            channel.SeriesViewModel.DataSeries = series;
            _dataSeriesYMax = GetYMaxAmongAllChannels(); //update the data series max
            RedrawHistogram();
        }

        public void SetDataBrushColor(int dataChannel, Color color)
        {
            HistogramChannel channel = GetChannelOrNull(dataChannel);
            if (channel == null)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, $"Cannot set brush color for histogram channel {dataChannel} because it does not exist.");
                return;
            }

            channel.FillColor = color;
            RedrawHistogram();
        }

        public void SetDataChannelIsVisible(int dataChannel, bool isVisible)
        {
            HistogramChannel channel = GetChannelOrNull(dataChannel);
            if (channel == null)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, $"Cannot set visibility for histogram channel {dataChannel} because it does not exist.");
                return;
            }

            channel.SeriesViewModel.IsVisible = isVisible;
        }

        ///<summary>
        /// This is used by ViewControlViewModel to update the state of all ExpandAll buttons without invoking ExpandAllHistograms
        ///</summary>
        public void UpdateAllHistogramExpandButton(bool value)
        {
            _allHistogramsExpanded = value;
            OnPropertyChanged("AllHistogramsExpanded");
        }

        private HistogramChannel GetChannelOrNull(int channel)
        {
            foreach (var histChannel in _histogramChannels)
            {
                if (histChannel.DataChannel == channel)
                {
                    return histChannel;
                }
            }

            return null;
        }

        private double GetDataValueFromUnit(double unitValue)
        {
            unitValue = Math.Min(Math.Max(unitValue, _scalingMin), _scalingMax);

            var returnVal = (unitValue - ScalingMin) * 255 / (_scalingMax - ScalingMin);

            return returnVal;
        }

        private double GetUnitValueFromData(double dataValue)
        {
            var returnVal = (_scalingMax - _scalingMin) * dataValue / 255 + _scalingMin;

            return Math.Min(Math.Max(returnVal, _scalingMin), _scalingMax);
        }

        /// <summary>
        /// Finds the highest Y value out of every series and every channel
        /// </summary>
        /// <returns></returns>
        private double GetYMaxAmongAllChannels()
        {
            double yMax = 0.0;
            foreach (var histogramChannel in _histogramChannels)
            {
                if (double.TryParse(histogramChannel.SeriesViewModel.DataSeries?.YMax.ToString(), out double channelYMax))
                {
                    if (channelYMax > yMax)
                    {
                        yMax = channelYMax;
                    }
                }
            }
            return yMax;
        }

        private void ResetRenderableViewModels()
        {
            Application.Current.Dispatcher.Invoke(new Action(() =>
            {
                RenderableSeriesViewModels.Clear();
                foreach (var histogramChannel in _histogramChannels)
                {
                    RenderableSeriesViewModels.Add(histogramChannel.SeriesViewModel);
                }
            }));

            OnPropertyChanged("HistogramChannels");
            OnPropertyChanged("IsChannelVisibilityVisible");
        }

        private void UpdateAxes()
        {
            if (0 == _numGridLines)
            {
                // no gridlines or anything
                _linearAxis.DrawMajorGridLines = false;
                _logAxis.DrawMajorGridLines = false;
                _xAxis.DrawMajorGridLines = false;
                _linearAxis.DrawLabels = false;
                _logAxis.DrawLabels = false;
                _xAxis.DrawLabels = false;

            }
            else
            {
                // draw gridlines; draw labels if in large histogram mode
                _linearAxis.DrawMajorGridLines = true;
                _logAxis.DrawMajorGridLines = true;
                _xAxis.DrawMajorGridLines = true;
                _logAxis.DrawLabels = _largeHistogram;
                _linearAxis.DrawLabels = _largeHistogram;
                _xAxis.DrawLabels = _largeHistogram;

                if (_largeHistogram)
                {
                    // when labels are active, set the min width to prevent resize spam
                    ((NumericAxis)_logAxis).MinWidth = Log ? _yAxesMinWidth : 0;
                    ((NumericAxis)_linearAxis).MinWidth = Log ? 0 : _yAxesMinWidth;
                }
                else
                {
                    ((NumericAxis)_logAxis).MinWidth = 0;
                    ((NumericAxis)_linearAxis).MinWidth = 0;
                }
            }
        }

        #endregion Methods
    }
}