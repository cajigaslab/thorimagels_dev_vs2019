namespace HistogramControl.ViewModel
{
    using System.Windows.Media;

    using SciChart.Charting.Model.DataSeries;
    using SciChart.Charting.Visuals.PaletteProviders;
    using SciChart.Charting.Visuals.RenderableSeries;

    ///<summary>
    /// Defines a paletter provider to return a red color if the Y-Value is over a threshold value
    ///</summary>
    public class MarkersThresholdPaletteProvider : IStrokePaletteProvider, IFillPaletteProvider
    {
        #region Fields

        private Brush _normalFillBrush;
        private Color _normalFillColor;
        private Color _normalStrokeColor;
        private Brush _overriddenFillBrush;
        private Color _overriddenFillColor;
        private Color _overriddenStrokeColor;

        #endregion Fields

        #region Constructors

        public MarkersThresholdPaletteProvider(Color overriddenFillColor)
        {
            _overriddenFillColor = overriddenFillColor;
            _overriddenStrokeColor = overriddenFillColor;
            _overriddenFillBrush = new SolidColorBrush(_overriddenFillColor);
            _normalFillColor = Colors.LightGray;
            _normalFillBrush = new SolidColorBrush(_normalFillColor);
            _normalStrokeColor = Colors.LightGray;
            _overriddenFillBrush.Opacity = 0.8;
        }

        #endregion Constructors

        #region Properties

        public Color NormalFillColor
        {
            set
            {
                _normalFillColor = value;
                _normalFillBrush = new SolidColorBrush(_normalFillColor);
            }
        }

        public Color NormalStrokeColor
        {
            set
            {
                _normalStrokeColor = value;
            }
        }

        public Color OverridenFillColor
        {
            set
            {
                _overriddenFillColor = value;
                _overriddenFillBrush = new SolidColorBrush(_overriddenFillColor);
            }
        }

        public Color OverridenStrokeColor
        {
            set
            {
                _overriddenStrokeColor = value;
            }
        }

        public double ThresholdBP
        {
            get; set;
        }

        public double ThresholdWP
        {
            get; set;
        }

        #endregion Properties

        #region Methods

        public void OnBeginSeriesDraw(IRenderableSeries series)
        {
            //var dataSeries = series.DataSeries as UniformXyDataSeries<int>;
        }

        public Brush OverrideFillBrush(IRenderableSeries series, int index, IPointMetadata metadata)
        {
            if (index < ThresholdBP || index > ThresholdWP)
            {
                var colorBrush = _overriddenFillBrush;
                colorBrush.Opacity = series.Opacity;
                return colorBrush;
            }
            else
            {
                var colorBrush = _normalFillBrush;
                colorBrush.Opacity = series.Opacity;
                return colorBrush;
            }

            // Returning null means use the default color when rendering
            //return null;
        }

        public Color? OverrideStrokeColor(IRenderableSeries series, int index, IPointMetadata metadata)
        {
            if (_overriddenStrokeColor == Colors.Transparent && ThresholdBP >= ThresholdWP)
            {
                if (index >= ThresholdWP && index <= ThresholdBP)
                {
                    var color = _normalStrokeColor;
                    color.A = (byte)_normalStrokeColor.A;
                    return color;
                }
                else
                {
                    var color = _overriddenStrokeColor;
                    color.A = (byte)_overriddenStrokeColor.A;
                    return color;
                }
            }

            if (index < ThresholdBP || index > ThresholdWP)
            {
                var color = _overriddenStrokeColor;
                color.A = (byte)_overriddenStrokeColor.A;
                return color;
            }
            else
            {
                var color = _normalStrokeColor;
                color.A = (byte)_normalStrokeColor.A;
                return color;
            }

            // Returning null means use the default color when rendering
            //return null;
        }

        #endregion Methods
    }
}