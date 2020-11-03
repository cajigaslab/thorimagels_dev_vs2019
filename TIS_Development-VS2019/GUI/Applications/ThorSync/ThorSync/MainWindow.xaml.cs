using System;
using System.Collections.Generic;
using System.Linq;
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
using System.Diagnostics;
using RealTimeLineChart;
using System.IO.Pipes;
using System.Security.Principal;

namespace ThorSync
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
            System.Reflection.Assembly assembly = System.Reflection.Assembly.GetExecutingAssembly();
            Version version = assembly.GetName().Version;
            AboutDll.SyncSplashScreen splash = new AboutDll.SyncSplashScreen(String.Format("v{0}.{1}.{2}.{3}", version.Major, version.Minor, version.Build, version.Revision));
            splash.Show();

            System.Threading.Thread.Sleep(2000);

            splash.Close();

            this.WindowState = WindowState.Normal;
        }

        private void miExit_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void About_Click(object sender, RoutedEventArgs e)
        {
            System.Reflection.Assembly assembly = System.Reflection.Assembly.GetExecutingAssembly();
            Version version = assembly.GetName().Version;
            AboutDll.SyncSplashScreen splash = new AboutDll.SyncSplashScreen(String.Format("v{0}.{1}.{2}.{3}", version.Major, version.Minor, version.Build, version.Revision));
            splash.Show();
        }

        private void Support_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                Process proc = new Process();
                proc.StartInfo.FileName = "ThorlabsSupport.exe";
                proc.Start();
            }
            catch
            {
                MessageBoxResult result = MessageBox.Show("Thorlabs QuickSupport could not be found in the current folder.", System.Environment.CurrentDirectory, MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }
    }
}
