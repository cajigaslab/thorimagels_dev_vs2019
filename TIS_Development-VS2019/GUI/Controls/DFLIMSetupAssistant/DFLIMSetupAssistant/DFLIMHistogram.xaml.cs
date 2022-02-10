namespace DFLIMSetupAssistant
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Collections.Specialized;
    using System.Globalization;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;

    using ROIStatsChart.Model;

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
    using SciChart.Charting.Visuals.PointMarkers;
    using SciChart.Charting.Visuals.RenderableSeries;
    using SciChart.Core;
    using SciChart.Data.Model;
    using SciChart.Drawing.HighSpeedRasterizer;
    using SciChart.Drawing.Utility;
    using SciChart.Drawing.VisualXcceleratorRasterizer;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for DFLIMHistogram.xaml
    /// </summary>
    public partial class DFLIMHistogram : UserControl
    {
        #region Constructors

        public DFLIMHistogram()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Methods

        public void Clear()
        {
            SciChartSurface.SetRuntimeLicenseKey("TCqHmTkInF/fDQuv2IRL2jISc44wjQP46+iIvQjEtY21jW+X66HmcupG3FzPOD39A8zSj8i8vKIUgW2r9wgDzzuy3RK/gQsogW5d2SN0QVo0tnTzAd/uEWHLFeS2W17/2hf//FVKxwU4704JENFsCxYbOoPZHbpNwbTJovnl1QjEabIjy1KzBkA2fJMJbWF8wPRTD0ruKUEnrHpXOuvpTOQlr7a6XSmUlJ5o/Vsx7oJRcIYm70L7shDDXu1hHEqICpBtcCb91kpgNMaAZoWJwhYiBmowdHbgszC9lm3o6hlLi35y88379sblqhR1b7rIh80hoc3XwfQUmPydvU6RAwLUyIYT/z28JOl3kx0pReVdlLQd5bfdldNeNrI6J3ajng427j2udkQpNqQxNUEbLH9D/qqr5xeez+F/O4FWIYiYJvs9pgMamA6GYfGnV1sQ2spekHboGxh5PWfNgAWTuqFU/arLx5W1LYhT75WcXUe8pSXX1JD6qGD7/G4l9KpN+CYuZrXh1Zl9ND5KLicMDvfX65W+B8ka0TZbLIFExmsWSwNt+n6osLwE48Q8JsPb1+WCzy+1oCaFnyGXcpK5LlVB0Dcg9VdcDnwmrEQ=");

            sciChartSurface.RenderableSeries.Clear();
        }

        public void Plot(KeyValuePair<int, SolidColorBrush> channel, uint[] data)
        {
            ObservableCollection<IRenderableSeries> chartSeries = new ObservableCollection<IRenderableSeries>();

            IXyDataSeries<double, double> ds0;

            ds0 = new XyDataSeries<double, double> { FifoCapacity = null, SeriesName = channel.ToString() };
            double binDuration = 5.0 * (double)data[255] / 128.0 / (double)data[254];
            double[] y = new double[240];
            double[] x = new double[240];
            for (int i = 0; i < x.Length; ++i)
            {
                if (data[i] < 0 || (i + 1) * binDuration < 0)
                {
                    //invalid value, return
                    return;
                }
                x[i] = (i + 1) * binDuration;
                y[i] = data[i];
            }

            ds0.Append(x, y);
            var color = channel.Value.Color;

            var l = new XyScatterRenderableSeries() { DataSeries = ds0, Tag = channel.Key.ToString(), StrokeThickness = 1, IsVisible = true, Stroke = color, PointMarker = new CrossPointMarker() { Width = 1.5, Height = 1.5, StrokeThickness = 2 } };
            chartSeries.Add(l);
            sciChartSurface.RenderableSeries = chartSeries;
            sciChartSurface.ZoomExtents();
        }

        /// <summary>
        /// Zooms the chart to the extent of the current data
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void FitChartToData(object sender, EventArgs e)
        {
            sciChartSurface.ZoomExtents();
        }

        #endregion Methods
    }

    public class LogarithmicBaseConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var str = (string)value;

            var result = str.ToUpperInvariant().Equals("E") ? Math.E : Double.Parse(str, CultureInfo.InvariantCulture);

            return result;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }

        #endregion Methods
    }
}