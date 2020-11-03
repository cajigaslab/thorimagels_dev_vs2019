namespace CaptureSetupDll
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.ServiceModel;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;
    using System.Windows.Threading;
    using System.Xml;

    using CaptureSetupDll.ViewModel;

    using ThorLogging;


    /// <summary>
    /// Interaction logic for SnapshotSettings.xaml
    /// </summary>
    public partial class CameraConsoleDlg : Window
    {
        #region Fields

        DispatcherTimer readTimer;

        #endregion Fields

        #region Constructors

        public CameraConsoleDlg()
        {
            InitializeComponent();

            this.Loaded += new RoutedEventHandler(CameraConsole_Loaded);
            this.Closed += new EventHandler(CameraConsole_Closed);
        }

        #endregion Constructors

        #region Properties

        #endregion Properties

        #region Methods

        private void Button_OnOK(object sender, RoutedEventArgs e)
        {

            Close();
        }

        void CameraConsole_Closed(object sender, EventArgs e)
        {
            readTimer.Stop();
            readTimer.Tick -= new EventHandler(readTimer_Tick);
        }

        void CameraConsole_Loaded(object sender, RoutedEventArgs e)
        {
            readTimer = new DispatcherTimer();

            readTimer.Interval = TimeSpan.FromSeconds(1);
            readTimer.Tick += new EventHandler(readTimer_Tick);
            readTimer.Start();
        }

        void readTimer_Tick(object sender, EventArgs e)
        {

            StringBuilder sb = new StringBuilder(256);

            GetFromCameraConsole(sb, sb.Capacity);

            if (sb.Length > 0)
            {
                lvResponse.Items.Insert(0,sb.ToString());
            }

        }


        #endregion Methods

        private void tbCommand_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Return)
            {
                SendToCameraConsole(tbCommand.Text.ToString() + "\r");

                tbCommand.Text = string.Empty;

                e.Handled = true;
            }
        }
        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SendToCameraConsole", CharSet = CharSet.Unicode)]
        private static extern bool SendToCameraConsole(string str);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetFromCameraConsole", CharSet = CharSet.Unicode)]
        private static extern bool GetFromCameraConsole(StringBuilder lpString, int length);
    }

}