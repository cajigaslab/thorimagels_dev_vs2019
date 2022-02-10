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

    using SciChart.Charting.Visuals;

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

            SciChartSurface.SetRuntimeLicenseKey("TCqHmTkInF/fDQuv2IRL2jISc44wjQP46+iIvQjEtY21jW+X66HmcupG3FzPOD39A8zSj8i8vKIUgW2r9wgDzzuy3RK/gQsogW5d2SN0QVo0tnTzAd/uEWHLFeS2W17/2hf//FVKxwU4704JENFsCxYbOoPZHbpNwbTJovnl1QjEabIjy1KzBkA2fJMJbWF8wPRTD0ruKUEnrHpXOuvpTOQlr7a6XSmUlJ5o/Vsx7oJRcIYm70L7shDDXu1hHEqICpBtcCb91kpgNMaAZoWJwhYiBmowdHbgszC9lm3o6hlLi35y88379sblqhR1b7rIh80hoc3XwfQUmPydvU6RAwLUyIYT/z28JOl3kx0pReVdlLQd5bfdldNeNrI6J3ajng427j2udkQpNqQxNUEbLH9D/qqr5xeez+F/O4FWIYiYJvs9pgMamA6GYfGnV1sQ2spekHboGxh5PWfNgAWTuqFU/arLx5W1LYhT75WcXUe8pSXX1JD6qGD7/G4l9KpN+CYuZrXh1Zl9ND5KLicMDvfX65W+B8ka0TZbLIFExmsWSwNt+n6osLwE48Q8JsPb1+WCzy+1oCaFnyGXcpK5LlVB0Dcg9VdcDnwmrEQ=");

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

        private void LogarithmicNumericAxis_VisibleRangeChanged(object sender, SciChart.Charting.Visuals.Events.VisibleRangeChangedEventArgs e)
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