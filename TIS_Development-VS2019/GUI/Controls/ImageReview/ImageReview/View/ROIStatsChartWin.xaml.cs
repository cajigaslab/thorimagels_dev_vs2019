namespace ImageReviewDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Text;
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

        public ROIStatsChartWin(string tittle = "")
        {
            InitializeComponent();
            if (tittle != "")
            {
                this.Title = tittle;
            }
            // Make this window the topmost within the app
            this.Owner = Application.Current.MainWindow;
            this.Closed += new EventHandler(ROIStatsChartWin_Closed);
        }

        #endregion Constructors

        #region Methods

        public void SetData(string[] names, double[] data, bool reload)
        {
            ROIChart.AppendROIStats(names, data, reload);
        }

        public void SetPath(string path)
        {
            ROIChart.SetPath(path);
        }

        public void UpdataXReviewPosition(double x)
        {
            ROIChart.UpdataXReviewPosition(x);
        }

        public void UpdataXVisibleRange(int xMin, int xMax)
        {
            ROIChart.UpdataXVisibleRange(xMin, xMax);
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void btnSave_Click(object sender, RoutedEventArgs e)
        {
            ROIChart.SaveStatChart();
        }

        private void ROIStatsChartWin_Closed(object sender, EventArgs e)
        {
            Application.Current.MainWindow.Activate();
        }

        #endregion Methods
    }
}