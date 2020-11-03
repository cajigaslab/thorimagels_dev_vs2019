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
using System.Windows.Threading;

namespace ROIChartTest
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            this.Loaded += MainWindow_Loaded;
        }

        void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            this.ucChart.SetFifoVisible(false);
        }

        void _timer_Tick(object sender, EventArgs e)
        {
            Random rnd = new Random();

            string[] name = {"Mean_1_1","Min_1_1","Max_1_1","SD_1_1"};
            double[] data = {rnd.Next(0,10),rnd.Next(0,10),rnd.Next(0,10),rnd.Next(0,10)};

            this.ucChart.AppendROIStats(name, data, false);
        }

        private DispatcherTimer _timer = new DispatcherTimer();

        private void btnLoad_Click(object sender, RoutedEventArgs e)
        {

            Random rnd = new Random();

            string[] name = { "Mean_1_1", "Min_1_1", "Max_1_1", "SD_1_1" };
            double[] data = { rnd.Next(0, 10), rnd.Next(0, 10), rnd.Next(0, 10), rnd.Next(0, 10) };

            this.ucChart.AppendROIStats(name, data, false);
        }

        private void btnTimer_Click(object sender, RoutedEventArgs e)
        {
            this.ucChart.SetFifoVisible(true);
            this.ucChart.ClearChart();
            _timer.Interval = new TimeSpan(0, 0, 0, 0, 30);
            _timer.Tick += _timer_Tick;
            _timer.Start();
        }

        private void btnPerformance_Click(object sender, RoutedEventArgs e)
        {
            this.ucChart.SetFifoVisible(true);
            this.ucChart.ClearChart();
            Random rnd = new Random();
            for (int i = 0; i < 100000; i++)
            {

                string[] name = { "Mean_1_1", "Min_1_1", "Max_1_1", "SD_1_1" };
                double[] data = { rnd.Next(0, 10), rnd.Next(0, 10), rnd.Next(0, 10), rnd.Next(0, 10) };

                this.ucChart.AppendROIStats(name, data, false);
            }
        }

        private void btnClear_Click(object sender, RoutedEventArgs e)
        {
            this.ucChart.ClearChart();
        }

        private void btnFifo100_Click(object sender, RoutedEventArgs e)
        {
            this.ucChart.SetFifoSize(100);
        }

        private void btnFifo0_Click(object sender, RoutedEventArgs e)
        {
            this.ucChart.SetFifoSize(0);
        }
    }
}
