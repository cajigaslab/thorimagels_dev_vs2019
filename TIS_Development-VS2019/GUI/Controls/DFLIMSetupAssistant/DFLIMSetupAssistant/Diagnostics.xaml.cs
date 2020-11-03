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
    /// Interaction logic for Diagnostics.xaml
    /// </summary>
    public partial class Diagnostics : UserControl
    {
        #region Constructors

        public Diagnostics()
        {
            InitializeComponent();
            YAxis.VisibleRange = new DoubleRange(0, 255);
            YAxis.VisibleRangeLimit = new DoubleRange(0, 255);
            YAxis.VisibleRangeLimitMode = RangeClipMode.MinMax;
            YAxis.AutoRange = Abt.Controls.SciChart.Visuals.Axes.AutoRange.Never;
        }

        #endregion Constructors

        #region Methods

        public void Clear()
        {
            sciChartSurface.RenderableSeries.Clear();
        }

        public void Plot(KeyValuePair<int, SolidColorBrush> channel, ushort[] data)
        {
            ObservableCollection<IChartSeriesViewModel> chartSeries = new ObservableCollection<IChartSeriesViewModel>();

            double[] y = new double[data.Length];
            double[] x = new double[data.Length];
            for (int i = 0; i < x.Length; ++i)
            {
                x[i] = i;
                y[i] = data[i];
            }

            IXyDataSeries<double, double> ds0 = new XyDataSeries<double, double> { FifoCapacity = null, SeriesName = channel.ToString() };
            ds0.Append(x, y);
            var color = channel.Value.Color;
            var vm = new ChartSeriesViewModel(ds0, new FastLineRenderableSeries() { Tag = channel.Key.ToString(), StrokeThickness = 2, IsVisible = true, SeriesColor = color });

            vm.RenderSeries.IsVisible = true;
            chartSeries.Add(vm);

            sciChartSurface.SeriesSource = chartSeries;

            sciChartSurface.ZoomExtentsX();

            YAxis.VisibleRange = new DoubleRange(0, 255);
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
}