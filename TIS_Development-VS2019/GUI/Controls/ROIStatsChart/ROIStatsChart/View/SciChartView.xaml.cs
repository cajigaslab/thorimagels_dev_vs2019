namespace ROIStatsChart.View
{
    using System;
    using System.Collections.Generic;
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

    using ROIStatsChart.ViewModel;

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
    using SciChart.Core;
    using SciChart.Data.Model;
    using SciChart.Drawing.HighSpeedRasterizer;
    using SciChart.Drawing.Utility;
    using SciChart.Drawing.VisualXcceleratorRasterizer;

    /// <summary>
    /// Interaction logic for SciChartView.xaml
    /// </summary>
    public partial class SciChartView : UserControl
    {
        #region Constructors

        public SciChartView()
        {
            SciChartSurface.SetRuntimeLicenseKey("TCqHmTkInF/fDQuv2IRL2jISc44wjQP46+iIvQjEtY21jW+X66HmcupG3FzPOD39A8zSj8i8vKIUgW2r9wgDzzuy3RK/gQsogW5d2SN0QVo0tnTzAd/uEWHLFeS2W17/2hf//FVKxwU4704JENFsCxYbOoPZHbpNwbTJovnl1QjEabIjy1KzBkA2fJMJbWF8wPRTD0ruKUEnrHpXOuvpTOQlr7a6XSmUlJ5o/Vsx7oJRcIYm70L7shDDXu1hHEqICpBtcCb91kpgNMaAZoWJwhYiBmowdHbgszC9lm3o6hlLi35y88379sblqhR1b7rIh80hoc3XwfQUmPydvU6RAwLUyIYT/z28JOl3kx0pReVdlLQd5bfdldNeNrI6J3ajng427j2udkQpNqQxNUEbLH9D/qqr5xeez+F/O4FWIYiYJvs9pgMamA6GYfGnV1sQ2spekHboGxh5PWfNgAWTuqFU/arLx5W1LYhT75WcXUe8pSXX1JD6qGD7/G4l9KpN+CYuZrXh1Zl9ND5KLicMDvfX65W+B8ka0TZbLIFExmsWSwNt+n6osLwE48Q8JsPb1+WCzy+1oCaFnyGXcpK5LlVB0Dcg9VdcDnwmrEQ=");

            InitializeComponent();

            Loaded += SciChartView_Loaded;
            Unloaded += SciChartView_Unloaded;
        }

        #endregion Constructors

        #region Properties

        /// <summary>
        /// The view model currently set to be this view's DataContext
        /// </summary>
        private ChartViewModel ViewModel
        {
            get
            {
                return (ChartViewModel)this.DataContext;
            }
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Zooms the chart to the extent of the current data
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void FitChartToData(object sender, EventArgs e)
        {
            sciChartSurface.ZoomExtents();
        }

        /// <summary>
        /// Passes mouse events to the vertical line annotation, enabling movement of it by dragging other objects such as the label
        /// </summary>
        private void RedirectToCurrentFrameAnnotation(object sender, MouseEventArgs args)
        {
            CurrentFrameLineAnnotation.RaiseEvent(args);
        }

        /// <summary>
        /// Setup all event handlers related to ViewModel events
        /// </summary>
        private void RegisterViewModelEventHandlers()
        {
            if (ViewModel != null)
            {
                ViewModel.ArithmeticDataChanged += FitChartToData;

                ViewModel.SciChartSurface = sciChartSurface;
            }
        }

        /// <summary>
        /// Execute when loaded
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void SciChartView_Loaded(object sender, RoutedEventArgs e)
        {
            RegisterViewModelEventHandlers();
            XAxis.LabelProvider = new CustomModifiers.CustomNumericLabelProvider();
        }

        /// <summary>
        /// Execute when unloaded
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void SciChartView_Unloaded(object sender, RoutedEventArgs e)
        {
            UnregisterViewModelEventHandlers();
        }

        /// <summary>
        /// Remove all event handlers related to ViewModel events
        /// </summary>
        private void UnregisterViewModelEventHandlers()
        {
            if (ViewModel != null)
            {
                ViewModel.ArithmeticDataChanged -= FitChartToData;
            }
        }

        #endregion Methods
    }
}