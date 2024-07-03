namespace ThorDetector_Control
{
    using System;
    using System.Collections.Generic;
    using System.Configuration;
    using System.Data;
    using System.Linq;
    using System.Threading.Tasks;
    using System.Windows;

    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        #region Constructors

        public App()
        {
            try
            {
                Application.Current.ShutdownMode = ShutdownMode.OnExplicitShutdown;
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message);

            }
        }

        #endregion Constructors

        #region Methods

        private void Application_Startup(object sender, StartupEventArgs e)
        {
            ConnectedDevicesWindow cw = new ConnectedDevicesWindow();
            cw.ShowDialog();

            if (COMPortManager.comPorts == null)
                COMPortManager.comPorts = new List<ComPort>();
            else
                COMPortManager.comPorts.Clear();
        }

        #endregion Methods
    }
}