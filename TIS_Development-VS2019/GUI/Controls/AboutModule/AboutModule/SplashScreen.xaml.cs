namespace AboutDll
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.ServiceModel;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for SplashScreen.xaml
    /// </summary>
    public partial class SplashScreen : Window
    {
        #region Fields

        bool _closing = false;
        bool _hardwareConnectionsOpened = false;
        bool _hides;

        #endregion Fields

        #region Constructors

        public SplashScreen(string title, bool hardwareconnectionsopened)
        {
            InitializeComponent();

            //Line below gets the build date from assembly files
            DateLabel.Content = "Thorlabs Inc. - " + (new FileInfo(Assembly.GetExecutingAssembly().Location).LastWriteTime).Date.ToString("yyyy");
            bool isBeta = false;
            txtTitle.Text = isBeta ? title + "-Beta" : title;
            txtTILS.Margin = isBeta ? new Thickness(5, 5, 30, 5) : txtTILS.Margin;
            _hides = false;
            _hardwareConnectionsOpened = hardwareconnectionsopened;
            this.Closing += new CancelEventHandler(SplashScreen_Closing);
            this.Deactivated += new EventHandler(SplashScreen_Deactivated);
            this.Loaded += SplashScreen_Loaded;
            this.ShowInTaskbar = false;
        }

        #endregion Constructors

        #region Properties

        public bool Hides
        {
            get { return _hides; }
            set { _hides = value; }
        }

        #endregion Properties

        #region Methods

        void LoadDAQInfo()
        {
            if (_hardwareConnectionsOpened)
            {
                bool daqAvailable = 1 == ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_DAQ_NAME);
                daqPanel.Visibility = daqAvailable ? Visibility.Visible : Visibility.Collapsed;
                if (daqAvailable)
                {
                    daqName.Text = ResourceManagerCS.GetCameraParamString((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_DAQ_NAME);
                    DAQFirmwareVer.Text = ResourceManagerCS.GetCameraParamString((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_DAQ_FW_VER);
                    DAQDriverVer.Text = ResourceManagerCS.GetCameraParamString((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_DAQ_DRIVER_VER);
                }
                bool lftAvailable = 1 == ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_LOW_FREQ_TRIG_BOARD_FW_VER);
                lftPanel.Visibility = lftAvailable ? Visibility.Visible : Visibility.Collapsed;
                if (lftAvailable)
                {
                    lftFWVer.Text = ResourceManagerCS.GetCameraParamString((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_LOW_FREQ_TRIG_BOARD_FW_VER);
                    lftCPLDVer.Text = ResourceManagerCS.GetCameraParamString((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_LOW_FREQ_TRIG_BOARD_CPLD_VER);
                }
            }
            else
            {
                daqPanel.Visibility = Visibility.Collapsed;
                lftPanel.Visibility = Visibility.Collapsed;
            }
        }

        void SplashScreen_Closing(object sender, CancelEventArgs e)
        {
            _closing = true;
        }

        void SplashScreen_Deactivated(object sender, EventArgs e)
        {
            try
            {
                if (false == _hides && false == _closing)
                {
                    _closing = true;
                    this.Close();
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        private void SplashScreen_Loaded(object sender, RoutedEventArgs e)
        {
            LoadDAQInfo();
        }

        private void SplashScreen_MouseDown(object sender, MouseButtonEventArgs e)
        {
            try
            {
                if (false == _hides && false == _closing)
                {
                    _closing = true;
                    this.Close();
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        #endregion Methods
    }
}