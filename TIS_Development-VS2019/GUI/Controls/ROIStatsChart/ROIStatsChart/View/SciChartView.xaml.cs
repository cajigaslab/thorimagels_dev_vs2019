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

    using Abt.Controls.SciChart;
    using Abt.Controls.SciChart.Visuals.Axes;

    using ROIStatsChart.ViewModel;

    /// <summary>
    /// Interaction logic for SciChartView.xaml
    /// </summary>
    public partial class SciChartView : UserControl
    {
        #region Constructors

        public SciChartView()
        {
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