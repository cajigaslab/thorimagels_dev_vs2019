namespace CaptureSetupDll.ViewModel
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

    /// <summary>
    /// Interaction logic for ROIStatsChartWin.xaml
    /// </summary>
    public partial class ROIStatsChartWin : Window
    {
        #region Constructors

        public ROIStatsChartWin(CaptureSetupViewModel captureSetupViewModel)
        {
            InitializeComponent();
            this.DataContext = captureSetupViewModel;
            try
            {
                // Make this window the topmost within the app
                this.Owner = Application.Current.MainWindow;
            }
            catch (Exception e)
            {
                e.ToString();
            }
            this.Closing += new System.ComponentModel.CancelEventHandler(ROIStatsChartWin_Closing);
            this.Closed += new EventHandler(ROIStatsChartWin_Closed);
        }

        #endregion Constructors

        #region Methods

        public void SetData(string[] names, double[] data, bool reload)
        {
            ROIChart.AppendROIStats(names, data, reload);
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        void ROIStatsChartWin_Closed(object sender, EventArgs e)
        {
            try
            {
                Application.Current.MainWindow.Activate();
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        void ROIStatsChartWin_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            CaptureSetupViewModel csVm = (CaptureSetupViewModel)this.DataContext;
            if (!csVm.UnloadWholeStats) csVm.ROIStatsChartActive = false;
            csVm.PersistROIStatsChartWindowSettings();
            e.Cancel = true;
            ROIChart.PersistSettings();
            this.Hide();
        }

        #endregion Methods
    }
}