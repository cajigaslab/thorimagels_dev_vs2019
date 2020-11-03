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

    using Abt.Controls.SciChart;
    using Abt.Controls.SciChart.Model.DataSeries;
    using Abt.Controls.SciChart.Visuals;
    using Abt.Controls.SciChart.Visuals.RenderableSeries;

    using ROIStatsChart.Model;

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
            sciChartSurface.RenderableSeries.Clear();
        }

        public void Plot(KeyValuePair<int, SolidColorBrush> channel, uint[] data)
        {
            ObservableCollection<IChartSeriesViewModel> chartSeries = new ObservableCollection<IChartSeriesViewModel>();

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
            var vm = new ChartSeriesViewModel(ds0, new XyScatterRenderableSeries() { Tag = channel.Key.ToString(), StrokeThickness = 1, IsVisible = true, SeriesColor = color, PointMarker = new Abt.Controls.SciChart.Visuals.PointMarkers.CrossPointMarker() { Width = 1.5, Height = 1.5, StrokeThickness = 2 } });

            vm.RenderSeries.IsVisible = true;

            chartSeries.Add(vm);

            sciChartSurface.SeriesSource = chartSeries;

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