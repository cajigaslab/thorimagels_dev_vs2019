namespace RealTimeLineChart.ViewModel
{
    using System;
    using System.Windows.Media;

    using SciChart;
    using SciChart.Charting;
    using SciChart.Charting.ChartModifiers;
    using SciChart.Charting.Common.Extensions;
    using SciChart.Charting.Model.DataSeries;
    using SciChart.Charting.Themes;
    using SciChart.Charting.Utility;
    using SciChart.Charting.Visuals;
    using SciChart.Charting.Visuals.Annotations;
    using SciChart.Charting.Visuals.Axes;
    using SciChart.Charting.Visuals.Axes.LabelProviders;
    using SciChart.Charting.Visuals.Events;
    using SciChart.Core;
    using SciChart.Data.Model;
    using SciChart.Drawing.HighSpeedRasterizer;
    using SciChart.Drawing.Utility;

    public interface IChannelViewModel
    {
        #region Events

        event Action XAxisVisibleChanged;

        #endregion Events

        #region Properties

        string ChannelName
        {
            get;
        }

        IDataSeries ChannelSeries
        {
            get;
            set;
        }

        int Height
        {
            get;
            set;
        }

        bool IsRollOverEnabled
        {
            get;
            set;
        }

        bool IsVisible
        {
            get;
            set;
        }

        bool XAxisVisible
        {
            get;
            set;
        }

        string YLabel
        {
            get;
            set;
        }

        IRange YVisibleRange
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        void Clear();

        void Refresh();

        void Update();

        #endregion Methods
    }

    /// <summary>
    /// This class contains properties that a View can data bind to.
    /// <para>
    /// See http://www.galasoft.ch/mvvm
    /// </para>
    /// </summary>
    public class ChannelViewModel : ViewModelBase, IChannelViewModel
    {
        #region Fields

        private IDataSeries _channelSeries = null;
        private Color _color;
        private int _height = 150;
        bool _idDigitalLine = false;
        private bool _isRollOverEnabled = false;
        private bool _isVisible = true;
        private bool _xAxisVisible = false;
        private IRange _xVisibleRange = new DoubleRange((double)0, (double)10);
        private string _yLabel = string.Empty;
        private bool _yVisibleLock = false;
        private IRange _yVisibleRange = new DoubleRange((double)-0.5, (double)0.5);

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the ChannelViewModel class.
        /// </summary>
        public ChannelViewModel(Color color, bool visible = true)
        {
            Stroke = color;
            IsVisible = visible;
            YLabel = "nm^2/Hz";
        }

        #endregion Constructors

        #region Events

        public event Action XAxisVisibleChanged;

        #endregion Events

        #region Properties

        public string AutoRangeX
        {
            get
            {
                if (null == _channelSeries)
                    return "Never";

                return (null == _channelSeries?.FifoCapacity) ? "Never" : "Always";
            }
        }

        public string AutoRangeY
        {
            get
            {
                if (null == _channelSeries)
                    return "Never";

                if (null == _channelSeries?.FifoCapacity || 0 ==  _channelSeries?.FifoCapacity)
                {
                    return "Never";
                }
                else
                {
                    return (false == _yVisibleLock) ? "Always" : "Never";
                }
            }
        }

        public string ChannelName
        {
            get
            {
                return (null == _channelSeries) ? "" : _channelSeries.SeriesName;
            }
        }

        public IDataSeries ChannelSeries
        {
            get { return _channelSeries; }
            set
            {
                _channelSeries = value;
                OnPropertyChanged("ChannelSeries");
            }
        }

        public int Height
        {
            get { return _height; }
            set
            {
                _height = value;
                OnPropertyChanged("Height");
            }
        }

        public bool IsDigitalLine
        {
            get => _idDigitalLine;
            set => SetProperty(ref _idDigitalLine, value);
        }

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

        public bool IsVisible
        {
            get { return _isVisible; }
            set
            {
                if (_isVisible != value)
                {
                    _isVisible = value;
                    OnPropertyChanged("IsVisible");
                    if (null != XAxisVisibleChanged)
                        XAxisVisibleChanged();
                }
            }
        }

        public Color Stroke
        {
            get { return _color; }
            set
            {
                _color = value;
                OnPropertyChanged("Stroke");
                OnPropertyChanged("StrokeBrush");
            }
        }

        public Brush StrokeBrush
        {
            get { return new SolidColorBrush(_color); }
        }

        public string TextForScientificNotation
        {
            get
            {
                if (_yVisibleRange.AsDoubleRange().Max < 10000 && _yVisibleRange.AsDoubleRange().Min > -10000)
                {
                    return "0.0###";
                }
                else
                {
                    return "#.##E+0";
                }
            }
        }

        public ScientificNotation ThresholdValue
        {
            get
            {
                if (_yVisibleRange.AsDoubleRange().Max < 10000 && _yVisibleRange.AsDoubleRange().Min > -10000)
                {
                    return ScientificNotation.None;
                }
                else
                {
                    return ScientificNotation.Normalized;
                }
            }
        }

        public bool XAxisVisible
        {
            get
            {
                return _xAxisVisible;
            }
            set
            {
                _xAxisVisible = value;
                OnPropertyChanged("XAxisVisible");
                OnPropertyChanged("XScrollVisible");
            }
        }

        public bool XScrollVisible
        {
            get
            {
                if (null != _channelSeries.FifoCapacity)
                {
                    return false;
                }
                else
                {
                    return _xAxisVisible;
                }
            }
        }

        public IRange XVisibleRange
        {
            get { return _xVisibleRange; }
            set
            {
                SetProperty(ref _xVisibleRange, value);
            }
        }

        public string YLabel
        {
            get
            {
                return _yLabel;
            }
            set
            {
                _yLabel = value;
                OnPropertyChanged("YLabel");
            }
        }

        public bool YVisibleLock
        {
            get { return _yVisibleLock; }
            set
            {
                _yVisibleLock = value;
                OnPropertyChanged("YVisibleLock");
                OnPropertyChanged("AutoRangeY");
            }
        }

        public IRange YVisibleRange
        {
            get { return _yVisibleRange; }
            set
            {
                _yVisibleRange = value;
                OnPropertyChanged("YVisibleRange");
                OnPropertyChanged("ThresholdValue");
                OnPropertyChanged("TextForScientificNotation");
            }
        }

        #endregion Properties

        #region Methods

        public void Clear()
        {
            if (null != _channelSeries)
                _channelSeries.Clear();
        }

        public void Refresh()
        {
            OnPropertyChanged("");
        }

        public void Update()
        {
            OnPropertyChanged("ChannelSeries");
            OnPropertyChanged("AutoRangeY");
        }

        #endregion Methods
    }

    public class SpectralViewModel : ViewModelBase, IChannelViewModel
    {
        #region Fields

        private IDataSeries _channelSeries = null;
        private IDataSeries _channelSeries2 = null;
        private Color[] _color = new Color[2];
        private int _height = 250;
        private bool _isRollOverEnabled = false;
        private bool _isVisible = true;
        private bool _xAxisVisible = false;
        private IRange _xVisibleRange = new DoubleRange(1.0, 100.0);
        private string _yLabel;
        private IRange _yVisibleRange = new DoubleRange(Constants.ThorRealTimeData.MIN_VALUE_LOG, 2 * Constants.ThorRealTimeData.MIN_VALUE_LOG);

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the ChannelViewModel class.
        /// </summary>
        public SpectralViewModel(Color color, bool visible = true)
        {
            Stroke[0] = color;
            IsVisible = visible;
            YLabel = "arb. unit";//"nm^2/Hz";
        }

        #endregion Constructors

        #region Events

        public event Action XAxisVisibleChanged;

        #endregion Events

        #region Properties

        public string ChannelName
        {
            get
            {
                return (null == _channelSeries) ? "" : _channelSeries.SeriesName;
            }
        }

        public IDataSeries ChannelSeries
        {
            get { return _channelSeries; }
            set
            {
                _channelSeries = value;
                OnPropertyChanged("ChannelSeries");
            }
        }

        public IDataSeries ChannelSeries2
        {
            get { return _channelSeries2; }
            set
            {
                _channelSeries2 = value;
                OnPropertyChanged("ChannelSeries2");
            }
        }

        public int Height
        {
            get { return _height; }
            set
            {
                _height = value;
                OnPropertyChanged("Height");
            }
        }

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

        public bool IsVisible
        {
            get { return _isVisible; }
            set
            {
                if (_isVisible != value)
                {
                    _isVisible = value;

                    XAxisVisibleChanged?.Invoke();
                    OnPropertyChanged("IsVisible");
                }
            }
        }

        public Color[] Stroke
        {
            get { return _color; }
            set
            {
                _color = value;
                OnPropertyChanged("Stroke");
                OnPropertyChanged("StrokeBrush");
            }
        }

        public Brush StrokeBrush
        {
            get { return new SolidColorBrush(_color[0]); }
        }

        public bool XAxisVisible
        {
            get
            {
                return _xAxisVisible;
            }
            set
            {
                _xAxisVisible = value;
                OnPropertyChanged("XAxisVisible");
            }
        }

        public IRange XVisibleRange
        {
            get { return _xVisibleRange; }
            set
            {
                if ((_xVisibleRange.Min != value.Min) || (_xVisibleRange.Max != value.Max))
                {
                    double lowerRange = (0 < _channelSeries.Count) ? Math.Max((double)_channelSeries.XRange.Min, ((DoubleRange)value).Min) : ((DoubleRange)value).Min;
                    double upperRange = (0 < _channelSeries.Count) ? Math.Min((double)_channelSeries.XRange.Max, ((DoubleRange)value).Max) : ((DoubleRange)value).Max;

                    if ((lowerRange != upperRange) && ((double)_xVisibleRange.Min != lowerRange || (double)_xVisibleRange.Max != upperRange))
                    {
                        _xVisibleRange = new DoubleRange(lowerRange, upperRange);
                        OnPropertyChanged("XVisibleRange");
                    }
                }
            }
        }

        public string YLabel
        {
            get
            {
                return _yLabel;
            }
            set
            {
                _yLabel = value;
                OnPropertyChanged("YLabel");
            }
        }

        public IRange YVisibleRange
        {
            get { return _yVisibleRange; }
            set
            {
                _yVisibleRange = value;
                OnPropertyChanged("YVisibleRange");
            }
        }

        #endregion Properties

        #region Methods

        public void Clear()
        {
            if (null != _channelSeries)
                _channelSeries.Clear();
            if (null != _channelSeries2)
                _channelSeries2.Clear();
        }

        public void Refresh()
        {
            OnPropertyChanged("");
        }

        public void Update()
        {
            OnPropertyChanged("ChannelSeries");
            OnPropertyChanged("ChannelSeries2");
        }

        #endregion Methods
    }
}