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
using MultiROIStats;
using System.Timers;
using System.Threading;
using System.Windows.Threading;

namespace MultiROIStatsTest
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        public MultiROIStatsUC uc = null;

        private DispatcherTimer _timer = new DispatcherTimer();

        private void btn_Click(object sender, RoutedEventArgs e)
        {
            _timer.Interval = new TimeSpan(0, 0, 0, 0, 100);
            _timer.Tick += _timer_Tick;
            _timer.Start();

            uc = new MultiROIStatsUC();
            uc.Show();       
            
        }

        void _timer_Tick(object sender, EventArgs e)
        { 
            const int NUM_ROW = 30;
            const int NUM_COLS = 20;

            string[] names = new string[NUM_ROW*NUM_COLS];
            double[] stats = new double[NUM_COLS*NUM_ROW];



            int i=0;
            int k = 0;
            for(int r=0; r<NUM_ROW; r++)
            {
                for (int c = 0; c < NUM_COLS; c++)
                {
                    names[k] = string.Format("{0}_{1}_{2}",i, (r % NUM_ROW) + 1, (r % NUM_COLS) + 1);
                    k++;
                }
                i++;
            }


                Random rand = new Random();
                i = 0;
                for (int r = 0; r < NUM_ROW; r++)
                {
                    for (int c = 0; c < NUM_COLS; c++)
                    {
                        stats[i] = rand.NextDouble();
                        i++;
                    }
                }
                uc.SetData(names, stats);
        }

    }
}
