namespace MCM6000_Control
{
    using System;
    using System.Collections.Generic;
    using System.Configuration;
    using System.Data;
    using System.Diagnostics;
    using System.Linq;
    using System.Text;
    using System.Windows;

    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        public App()
        {
            try
            {
                Application.Current.ShutdownMode = ShutdownMode.OnExplicitShutdown;

            //    System.Diagnostics.Process[] process = System.Diagnostics.Process.GetProcessesByName("MCM6000_Control");

            //    System.Diagnostics.Process myprocess = System.Diagnostics.Process.GetCurrentProcess();

            //    foreach (System.Diagnostics.Process p in process)
            //    {
            //        StringBuilder sb = new StringBuilder();

            //        sb.AppendFormat("Process id {0} MyProcess id {1}", p.Id, myprocess.Id);

            //        if (false == p.Id.Equals(myprocess.Id))
            //        {
            //            if (MessageBoxResult.Yes == MessageBox.Show("An existing version of MCM6000_Control is running. Do you want to close the existing MCM6000_Control.exe?", "Close existing application", MessageBoxButton.YesNo, MessageBoxImage.Warning, MessageBoxResult.Yes))
            //            {
            //                ProcessUtility.KillTree(p.Id);
            //            }
            //        }

            //    }
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message);

            }
        }

        private void Application_Startup(object sender, StartupEventArgs e)
        {
            ComPortWindow CompWin = new ComPortWindow();
            CompWin.ShowDialog();
        }
    }
}