namespace MenuLSDll.View
{
    using System;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Windows;
    using System.Windows.Automation.Peers;
    using System.Windows.Automation.Provider;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Threading;
    using System.Xml;

    using MenuLSDll.ViewModel;

    using ThorImageInfastructure;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for MenuLSView.xaml
    /// </summary>
    public partial class MenuLSView : UserControl
    {
        #region Fields

        private DispatcherTimer _memoryReadTimer;
        bool _noLSMImageDetectors;
        bool _viewLoaded;

        #endregion Fields

        #region Constructors

        public MenuLSView()
        {
            InitializeComponent();

            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this) == true)
                return;

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Methods

        public void LoadTabCtrlSettings()
        {
            MenuLSViewModel vm = (MenuLSViewModel)this.DataContext;

            string hwSettings = string.Empty;

            if (vm != null)
            {
                hwSettings = ResourceManagerCS.GetHardwareSettingsFileString();
            }
            if (!File.Exists(hwSettings))
            {
                btnCapture.IsEnabled = false;
                btnScript.IsEnabled = false;
                btnCaptureSetup.IsEnabled = false;
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " Unable to load hardware setttings");
            }

            XmlDocument hwSettingsDoc = new XmlDocument();
            hwSettingsDoc.Load(hwSettings);
            XmlNodeList ndList = hwSettingsDoc.SelectNodes("/HardwareSettings/ImageDetectors/LSM");
            XmlNodeList ndListCam = hwSettingsDoc.SelectNodes("/HardwareSettings/ImageDetectors/Camera");

            if ((0 == ndList.Count)&&(0 == ndListCam.Count))
            {
                if (!_viewLoaded)
                {
                    btnCapture.IsEnabled = false;
                    btnScript.IsEnabled = false;
                    btnCaptureSetup.IsEnabled = false;
                    tiReview.IsSelected = true; //select Review TabItem
                    tabCtrl.IsEnabled = false;
                    _viewLoaded = true;
                    //Run ImageReviewCommand

                    if (vm != null)
                    {
                        vm.ImageReviewCommand.Execute(null);
                    }
                }
                vm.NoLSMImageDetectors = true;
                _noLSMImageDetectors = true;

            }
            else
            {
                if (_viewLoaded)
                {
                    btnCapture.IsEnabled = true;
                    btnScript.IsEnabled = true;
                    btnCaptureSetup.IsEnabled = true;
                    tabCtrl.IsEnabled = true;
                    _viewLoaded = false;
                }
                vm.NoLSMImageDetectors = false;
                _noLSMImageDetectors = false;
            }
        }

        public void SelectCaptureTab()
        {
            if (!_noLSMImageDetectors)
            {
                tiCapture.IsSelected = true;
            }
        }

        public void SetCaptureSetup()
        {
            if (!_noLSMImageDetectors)
            {
                MenuLSViewModel vm = (MenuLSViewModel)this.DataContext;
                if (vm != null)
                {
                    vm.CaptureSetupCommand.Execute(null);
                    tiCaptureSetup.IsSelected = true;   //select Capture Setup TabItem

                }
            }
        }

        public void SetScriptTab()
        {
            if (!_noLSMImageDetectors)
            {
                MenuLSViewModel vm = (MenuLSViewModel)this.DataContext;
                if (vm != null)
                {
                    vm.ScriptCommand.Execute(null);
                    tiScript.IsSelected = true;   //select Script TabItem
                }
            }
        }

        private void btnCaptureSetup_Click(object sender, RoutedEventArgs e)
        {
            if (!_noLSMImageDetectors)
            {
                tiCaptureSetup.IsSelected = true;   //select Capture Setup TabItem

                //Run CaptureSetupCommand
                MenuLSViewModel vm = (MenuLSViewModel)this.DataContext;

                if (vm != null)
                {
                    if (vm.NeedToReconnectCamera)
                    {
                        // For script, once it is done with the captures it will return to script tab, if user tries
                        // to switch to Capture Setup it needs to reconnect the camera to pick the correct image routine
                        vm.RefreshCameraAndCaptureSetup();
                        vm.SelectedMenuTab = 0;
                        vm.NeedToReconnectCamera = false;
                    }
                    else
                    {
                        vm.CaptureSetupCommand.Execute(null);
                    }
                }
            }
        }

        private void btnCapture_Click(object sender, RoutedEventArgs e)
        {
            if (!_noLSMImageDetectors)
            {
                tiCapture.IsSelected = true;    //select Capture TabItem

                //Run RunSampleLSCommand
                MenuLSViewModel vm = (MenuLSViewModel)this.DataContext;

                if (vm != null)
                {
                    vm.RunSampleLSCommand.Execute(null);
                }
            }
        }

        private void btnReview_Click(object sender, RoutedEventArgs e)
        {
            tiReview.IsSelected = true; //select Review TabItem

            //Run ImageReviewCommand
            MenuLSViewModel vm = (MenuLSViewModel)this.DataContext;

            if (vm != null)
            {
                vm.ImageReviewCommand.Execute(null);
            }
        }

        private void btnScript_Click(object sender, RoutedEventArgs e)
        {
            if (!_noLSMImageDetectors)
            {
                tiScript.IsSelected = true; //select Script TabItem

                //Run ImageReviewCommand
                MenuLSViewModel vm = (MenuLSViewModel)this.DataContext;

                if (vm != null)
                {
                    vm.ScriptCommand.Execute(null);
                }
            }
        }

        private void cbModality_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            MenuLSViewModel vm = (MenuLSViewModel)this.DataContext;

            if (vm != null)
            {
                if (cbModality.IsDropDownOpen)
                {
                    // RefreshCameraAndCaptureSetup sends a command to reload Capture Setup's GUI
                    // should only do this if the modality was changed through the dropdown or gearbox
                    vm.RefreshCameraAndCaptureSetup();
                    vm.SelectedMenuTab = 0;
                    vm.NeedToReconnectCamera = false;
                }
            }
        }

        private void gearButton_Click(object sender, RoutedEventArgs e)
        {
            tiCaptureSetup.IsSelected = true;
        }

        private void MenuItem_Click(object sender, RoutedEventArgs e)
        {
            if (!_noLSMImageDetectors)
            {
                tiCaptureSetup.IsSelected = true;   //select Capture Setup TabItem
            }
        }

        private void TabControl_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (!_noLSMImageDetectors)
            {
                //Run CaptureSetupCommand
                MenuLSViewModel vm = (MenuLSViewModel)this.DataContext;

                if (vm != null)
                {
                    switch (((TabControl)sender).SelectedIndex)
                    {
                        case 0:
                            {
                                vm.CaptureSetupCommand.Execute(null);
                            }
                            break;
                        case 1:
                            {
                                vm.RunSampleLSCommand.Execute(null);
                            }
                            break;
                        case 2:
                            {
                                vm.ImageReviewCommand.Execute(null);
                            }
                            break;
                        case 3:
                            {
                                vm.ScriptCommand.Execute(null);
                            }
                            break;
                    }
                }
            }
        }

        private void UserControl_Loaded(object sender, RoutedEventArgs e)
        {
            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this) == true)
                return;

            _memoryReadTimer = new DispatcherTimer();
            _memoryReadTimer.Interval = TimeSpan.FromMilliseconds(10000);
            _memoryReadTimer.Tick += new EventHandler(_memoryReadTimer_Tick);
            _memoryReadTimer.Start();
            _viewLoaded = false;
            LoadTabCtrlSettings();

            ThorSharedTypes.ResourceManagerCS.LoadModalities();
        }

        private void UserControl_Unloaded(object sender, RoutedEventArgs e)
        {
            _memoryReadTimer.Stop();
            _memoryReadTimer.Tick -= new EventHandler(_memoryReadTimer_Tick);
        }

        void _memoryReadTimer_Tick(object sender, EventArgs e)
        {
            Process proc = System.Diagnostics.Process.GetCurrentProcess();

            if (proc != null)
            {
                proc.Refresh();

                const long BYTES_TO_MEGABYTES = 1048576;

                long val = proc.WorkingSet64/(BYTES_TO_MEGABYTES);

                lblMemoryUsage.Content = val.ToString() + " MB";
            }
        }

        #endregion Methods
    }
}