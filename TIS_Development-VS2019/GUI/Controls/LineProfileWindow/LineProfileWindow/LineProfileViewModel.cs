namespace LineProfileWindow.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows.Media;

    using SciChart;
    using SciChart.Charting;
    using SciChart.Charting.ChartModifiers;
    using SciChart.Charting.Common.Extensions;
    using SciChart.Charting.Model.DataSeries;
    using SciChart.Charting.Themes;
    using SciChart.Charting.Visuals;
    using SciChart.Charting.Visuals.Annotations;
    using SciChart.Charting.Visuals.Axes;
    using SciChart.Charting.Visuals.Axes.LabelProviders;
    using SciChart.Charting.Visuals.Events;
    using SciChart.Charting.Visuals.RenderableSeries;
    using SciChart.Core;
    using SciChart.Data.Model;
    using SciChart.Drawing.HighSpeedRasterizer;
    using SciChart.Drawing.Utility;
    using SciChart.Drawing.VisualXcceleratorRasterizer;

    using ThorSharedTypes;

    internal class LineProfileViewModel : ThorSharedTypes.VMBase
    {
        #region Fields

        AutoRange _autoRangeY = AutoRange.Always;
        private ObservableCollection<IRenderableSeries> _chartSeries;
        private Color[] _colorAssigment = null;
        private int _displayChannelIndex = 0;
        private string _horizontalAxisTitle = string.Empty;
        private int _initialChildrenCount = 0;
        private bool _isAutoScaleActive = true;
        private bool _isConversionActive = false;
        private bool _isDisplaying = true;
        LineProfileData _lineProfileData;
        private int _lineWidth = 1;
        private int _lineWidthMax = int.MaxValue;
        private int _maxChannels = 4;
        private int _numChannel = 1;
        private string _title;
        private string _verticalAxisTitle;

        private double _ymax = 17000; //Max pixel depth is 14bit
        private double _ymin = 0;
        IRange _yVisibleRange;

        #endregion Fields

        #region Constructors

        public LineProfileViewModel(Color[] colorAssigment)
        {
            _chartSeries = new ObservableCollection<IRenderableSeries>();
            _colorAssigment = colorAssigment;
            _yVisibleRange = new DoubleRange();
        }

        #endregion Constructors

        #region Events

        public event Action<int> LineWidthChange;

        #endregion Events

        #region Properties

        string _chartXLabel = "Line Length (pixels)";
        
        public string ChartXLabel
        {
            get => _chartXLabel;
            set=> SetProperty(ref _chartXLabel, value);
        }
        public AutoRange AutoRangeY
        {
            get => _autoRangeY;
            set => SetProperty(ref _autoRangeY, value);
        }

        public ObservableCollection<IRenderableSeries> ChartSeries
        {
            get { return _chartSeries; }
        }

        public Color[] ColorAssigment
        {
            get
            {
                return _colorAssigment;
            }
            set
            {
                _colorAssigment = value;
                Redraw();
            }
        }

        public int DisplayChannelIndex
        {
            get { return _displayChannelIndex; }
            set { _displayChannelIndex = value; }
        }

        public string HorizontalAxisTitle
        {
            get
            { return _horizontalAxisTitle; }
            set
            { _horizontalAxisTitle = value; }
        }

        public int InitialChildrenCount
        {
            get { return _initialChildrenCount; }
            set { _initialChildrenCount = value; }
        }

        public bool IsAutoScaleActive
        {
            get { return _isAutoScaleActive; }
            set
            {
                _isAutoScaleActive = value;
                AutoRangeY = _isAutoScaleActive ? AutoRange.Always : AutoRange.Never;
                OnPropertyChanged("IsAutoScaleActive");
            }
        }

        public bool IsConversionActive
        {
            get { return _isConversionActive; }
            set
            {
                _isConversionActive = value;
                Redraw();
                if (value)
                {
                    ChartXLabel = "Line Length (µm)";
                }
                else
                {
                    ChartXLabel = "Line Length (pixels)";
                }
            }
        }

        public bool IsDisplaying
        {
            get { return _isDisplaying; }
            set { _isDisplaying = value; }
        }

        public LineProfileData LineProfileData
        {
            get { return _lineProfileData; }
            set { _lineProfileData = value; }
        }

        public int LineWidth
        {
            get
            {
                return _lineWidth;
            }
            set
            {
                _lineWidth = Math.Max(1, value);
                _lineWidth = Math.Min(_lineWidth, _lineWidthMax);
                if (_lineWidth % 2 == 0) _lineWidth--;
                LineWidthChange(_lineWidth);
                OnPropertyChanged("LineWidth");
            }
        }

        public int LineWidthMax
        {
            get { return _lineWidthMax; }
            set
            {
                _lineWidthMax = value;
                _lineWidth = (_lineWidth > _lineWidthMax) ? _lineWidthMax : _lineWidth;
            }
        }

        public string LTitle
        {
            get
            { return _title; }
            set
            { _title = value; }
        }

        public int MaxChannels
        {
            get { return _maxChannels; }
            set { _maxChannels = value; }
        }

        public int NumChannel
        {
            get { return _numChannel; }
            set { _numChannel = value; }
        }

        public string VerticalAxisTitle
        {
            get
            { return _verticalAxisTitle; }
            set
            { _verticalAxisTitle = value; }
        }

        public double YmaxValue
        {
            get { return _ymax; }
            set
            {
                if (value > _ymin)
                {
                    _ymax = value;
                    YVisibleRange = new DoubleRange(_ymin, _ymax);
                }
            }
        }

        public double YminValue
        {
            get { return _ymin; }
            set
            {
                if (value < _ymax)
                {
                    _ymin = value;
                    YVisibleRange = new DoubleRange(_ymin, _ymax);
                }
            }
        }

        public IRange YVisibleRange
        {
            get { return _yVisibleRange; }
            set
            {
                if (_yVisibleRange != value)
                {
                    _yVisibleRange = value;
                    OnPropertyChanged("YVisibleRange");
                    _ymin = (double)value.Min;
                    OnPropertyChanged("YminValue");
                    _ymax = (double)value.Max;
                    OnPropertyChanged("YmaxValue");
                }
            }
        }

        #endregion Properties

        #region Methods

        public void Redraw()
        {
            _chartSeries.Clear();
            int j = 0;

                for (int i = 0; i < _maxChannels; i++)
            {
                if (Convert.ToBoolean(_lineProfileData.channelEnable & (int)Math.Pow(2, i)) && _lineProfileData.profileDataY != null && _lineProfileData.profileDataY[j] != null)
                {
                    UniformXyDataSeries<double> series;

                    if (_isConversionActive)
                    {
                        series = new UniformXyDataSeries<double>(0) { FifoCapacity = null, XStart = 0, XStep = _lineProfileData.PixeltoµmConversionFactor };
                    }
                    else
                    {
                        series = new UniformXyDataSeries<double>(0) { FifoCapacity = null, XStart = 0, XStep = 1 };
                    }
                    var l = new FastLineRenderableSeries
                    {
                        StrokeThickness = 2,
                        Stroke = ColorAssigment[i],
                        IsVisible = true,
                        ResamplingMode = SciChart.Data.Numerics.ResamplingMode.Auto,
                        AntiAliasing = false,
                        DataSeries = series
                    };
                    series.Append(_lineProfileData.profileDataY[j]);
                    _chartSeries.Add(l);
                    ++j;
                }
            }
            OnPropertyChanged("ChartSeries");
        }

        public void SaveAs()
        {
            Microsoft.Win32.SaveFileDialog dlg = new Microsoft.Win32.SaveFileDialog();
            dlg.FileName = "LineProfileData";
            dlg.DefaultExt = ".csv";
            dlg.Filter = "CSV files (.csv)|*.csv";

            if (true == dlg.ShowDialog())
            {
                using (var fileStream = new FileStream(dlg.FileName, FileMode.Create))
                {
                    // write to just created file
                    string head = "";
                    long chEnable = LineProfileData.channelEnable;
                    if ((chEnable & 0x01) != 0) head += "Channel A,";
                    if ((chEnable & 0x02) != 0) head += "Channel B,";
                    if ((chEnable & 0x04) != 0) head += "Channel C,";
                    if ((chEnable & 0x08) != 0) head += "Channel D,";
                    byte[] h = new UTF8Encoding(true).GetBytes(head + '\n');
                    fileStream.Write(h, 0, h.Length);
                    for (int i = 0; i < LineProfileData.profileDataX.Length; i++)
                    {
                        string dataLine = "";
                        for (int j = 0; j < LineProfileData.profileDataY.Length; j++)
                        {
                            dataLine += (LineProfileData.profileDataY[j][i]).ToString() + ',';
                        }
                        byte[] d = new UTF8Encoding(true).GetBytes(dataLine + '\n');
                        fileStream.Write(d, 0, d.Length);
                    }
                }
            }
        }

        public void SetData(LineProfileData lineprofileData)
        {
            _lineProfileData = lineprofileData;
            Redraw();
        }

        #endregion Methods
    }
}