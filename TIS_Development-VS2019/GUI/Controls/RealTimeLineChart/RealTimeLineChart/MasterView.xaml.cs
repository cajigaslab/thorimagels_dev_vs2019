namespace RealTimeLineChart
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.IO.Pipes;
    using System.Linq;
    using System.Security.Principal;
    using System.Text;
    using System.Threading;
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

    using RealTimeLineChart.ViewModel;

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class MasterView : UserControl
    {
        #region Fields

        RealTimeLineChartViewModel _vm = new RealTimeLineChartViewModel();

        #endregion Fields

        #region Constructors

        public MasterView()
        {
            InitializeComponent();
            this.Loaded += MasterView_Loaded;
            this.Dispatcher.ShutdownStarted += Dispatcher_ShutdownStarted;
            Application.Current.Exit += Current_Exit;
        }

        #endregion Constructors

        #region Methods

        private void btnCapture_Click(object sender, RoutedEventArgs e)
        {
            _vm.ChartMode = (int)RealTimeLineChartViewModel.ChartModes.CAPTURE;
        }

        private void btnReview_Click(object sender, RoutedEventArgs e)
        {
            _vm.ChartMode = (int)RealTimeLineChartViewModel.ChartModes.REVIEW;
        }

        private void ConfigPanel_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (null != _vm)
            {
                _vm.ConfigVisibility ^= true;
            }
        }

        void Current_Exit(object sender, ExitEventArgs e)
        {
            _vm.ThorImageLSConnectionStats = false;
            _vm.StopNamedPipeClient();
        }

        void Dispatcher_ShutdownStarted(object sender, EventArgs e)
        {
            if (null != _vm)
            {
                _vm.StopCaptureWorkers();
                _vm.ExitAcquisition();
                //only save settings in capture mode
                if ((int)RealTimeLineChartViewModel.ChartModes.CAPTURE == _vm.ChartMode)
                    _vm.SaveDocumentSettings();
            }
        }

        void MasterView_Loaded(object sender, RoutedEventArgs e)
        {
            if (null != _vm)
            {
                try
                {
                    this.DataContext = _vm;
                    this.sciChartView.DataContext = _vm;
                    this.channelsSetupView.DataContext = _vm;
                    this.channelsReviewView.DataContext = _vm;
                    this.StatsView.DataContext = _vm;
                    _vm.LoadDocumentSettings();
                    _vm.CreateChartLines();
                    _vm.InitAcquisition();
                    _vm.InitIPC();
                }
                catch (Exception ex)
                {
                    ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Verbose, 1, "RealTimeLineChart MasterView load error: " + ex.Message);
                    MessageBox.Show("There was an error at loading ThorSync. Some of your properties may not have been updated.");
                }
            }
        }

        private void StatsPanel_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (null != _vm)
            {
                _vm.StatsPanelEnable ^= true;
            }
        }

        #endregion Methods
    }
}