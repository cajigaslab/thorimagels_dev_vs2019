namespace FLIMFitting.View
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

    using FLIMFitting.ViewModels;

    /// <summary>
    /// Interaction logic for FittingView.xaml
    /// </summary>
    public partial class FLIMFittingView : UserControl
    {
        #region Fields

        FLIMFittingViewModel _viewModel;

        #endregion Fields

        #region Constructors

        public FLIMFittingView()
        {
            InitializeComponent();
            _viewModel = new FLIMFittingViewModel();
            DataContext = _viewModel;
            _viewModel.UpdateTZero += _viewModel_UpdateTZero;
            Loaded += FLIMFittingView_Loaded;
            Unloaded += FLIMFittingView_Unloaded;
        }

        #endregion Constructors

        #region Events

        public event Action<Dictionary<int, double>> UpdateTZero;

        #endregion Events

        #region Properties

        public bool AutoFitOnce
        {
            set
            {
                _viewModel.AutoFitOnce = value;
            }
        }

        public List<FLIMHistogramGroupData> FLIMHistogramGroups
        {
            set
            {
                _viewModel.FLIMHistogramGroups = value;
            }
        }

        #endregion Properties

        #region Methods

        void FLIMFittingView_Loaded(object sender, RoutedEventArgs e)
        {
            _viewModel.StartFlimHistogramUpdateThread();
        }

        void FLIMFittingView_Unloaded(object sender, RoutedEventArgs e)
        {
            _viewModel.StopFlimHistogramUpdateThread();
        }

        private void LogarithmicNumericAxis_VisibleRangeChanged(object sender, Abt.Controls.SciChart.VisibleRangeChangedEventArgs e)
        {
            sciChartSurface.ZoomExtents();
        }

        void _viewModel_UpdateTZero(Dictionary<int, double> tZeroDictionary)
        {
            if (null != UpdateTZero)
            {
                UpdateTZero(tZeroDictionary);
            }
        }

        #endregion Methods
    }
}