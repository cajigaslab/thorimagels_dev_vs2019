namespace HistogramControl.ViewModel
{
    using System.Windows.Media;

    using SciChart.Charting.Model.ChartSeries;

    using ThorSharedTypes;

    public class HistogramChannel
    {
        #region Fields

        static Color _thresholdOverrideColor = Colors.LightGray;

        int _dataChannel;
        Color _fillColor;
        bool _isLineChart;
        double _opacity;

        #endregion Fields

        #region Constructors

        public HistogramChannel(Color fillColor, double opacity, ImageIdentifier imageIdentifier, int dataChannel)
        {
            _isLineChart = true;
            _opacity = opacity;
            _fillColor = fillColor;
            ImageIdentifier = imageIdentifier;
            _dataChannel = dataChannel;

            PaletteProvider = new MarkersThresholdPaletteProvider(_thresholdOverrideColor);
            SeriesViewModel = new ColumnRenderableSeriesViewModel
            {
                PaletteProvider = PaletteProvider,
                Opacity = _opacity,
                IsVisible = true,
                //Fill is determined by PaletteProvider
            };

            UpdateHistogramViewModel();
        }

        #endregion Constructors

        #region Properties

        public int DataChannel => _dataChannel;

        public Color FillColor
        {
            get => _fillColor;
            set
            {
                if (_fillColor != value)
                {
                    // change fill color and fire event
                    _fillColor = value;
                    UpdateHistogramViewModel();
                }
            }
        }

        public ImageIdentifier ImageIdentifier
        {
            get; set;
        }

        public bool IsLineChart
        {
            get => _isLineChart;
            set
            {
                if(_isLineChart != value)
                {
                    _isLineChart = value;
                    UpdateHistogramViewModel();// TODO: dispatcher needed?
                }
            }
        }

        public bool IsVisible
        {
            get
            {
                return SeriesViewModel.IsVisible;
            }

            set
            {
                SeriesViewModel.IsVisible = value;
            }
        }

        public double Opacity
        {
            get => _opacity;
            set
            {
                if (_opacity != value)
                {
                    _opacity = value;
                    SeriesViewModel.Opacity = _opacity;
                }
            }
        }

        public MarkersThresholdPaletteProvider PaletteProvider
        {
            get;
        }

        public BaseRenderableSeriesViewModel SeriesViewModel
        {
            get;
        }

        #endregion Properties

        #region Methods

        public void UpdateHistogramViewModel()
        {
            if(_isLineChart)
            {
                SeriesViewModel.IsDigitalLine = false;
                PaletteProvider.NormalFillColor = Colors.Transparent;
                PaletteProvider.OverridenFillColor = Colors.Transparent;
            }
            else
            {
                SeriesViewModel.IsDigitalLine = true; // makes the line step-like
                PaletteProvider.NormalFillColor = _fillColor;
                PaletteProvider.OverridenFillColor = _thresholdOverrideColor;
            }
            PaletteProvider.NormalStrokeColor = _fillColor;
        }

        #endregion Methods
    }
}