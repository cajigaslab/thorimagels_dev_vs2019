namespace CaptureSetupDll
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Net;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Animation;
    using System.Windows.Shapes;

    using CaptureSetupDll.Model;
    using CaptureSetupDll.ViewModel;

    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Unity;

    using ThorLogging;

    using ThorSharedTypes;

    #region Enumerations

    enum ViewTypes
    {
        ViewType2D = 0,
        ViewType3D = 1,
        ViewTypeTiles = 2
    }

    #endregion Enumerations

    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class MainWindow : UserControl
    {
        #region Fields

        int OFFSET_FOR_RESIZE_SCROLL = 110;

        #endregion Fields

        #region Constructors

        public MainWindow()
        {
            if (ResourceManagerCS.Instance.TabletModeEnabled)
            {
                // When hidding the multiple tab selection in Menu Module there is a gap in the bottom of CS
                OFFSET_FOR_RESIZE_SCROLL = 76;
            }
            InitializeComponent();
        }

        public MainWindow(CaptureSetupViewModel captureSetupViewModel)
            : this()
        {
            this.DataContext = captureSetupViewModel;

            this.MasterView.DataContext = captureSetupViewModel;

            MVMManager.Instance.AddMVM("CaptureSetupViewModel", captureSetupViewModel);

            this.MasterView.PowerControlView.DataContext = MVMManager.Instance["PowerControlViewModel", captureSetupViewModel];

            this.MasterView.AreaControlView.DataContext = MVMManager.Instance["AreaControlViewModel", captureSetupViewModel];

            this.MasterView.ZControlView.DataContext = MVMManager.Instance["ZControlViewModel", captureSetupViewModel];

            this.MasterView.ScanControlView.DataContext = MVMManager.Instance["ScanControlViewModel", captureSetupViewModel];

            this.MasterView.LaserControlView.DataContext = MVMManager.Instance["LaserControlViewModel", captureSetupViewModel];

            this.MasterView.MultiphotonControlView.DataContext = MVMManager.Instance["MultiphotonControlViewModel", captureSetupViewModel];

            this.MasterView.DigitalSwitchesView.DataContext = MVMManager.Instance["DigitalOutputSwitchesViewModel", captureSetupViewModel];

            this.MasterView.CameraControlView.DataContext = MVMManager.Instance["CameraControlViewModel", captureSetupViewModel];

            this.MasterView.LampControlView.DataContext = MVMManager.Instance["LampControlViewModel", captureSetupViewModel];

            this.MasterView.PinholeControlView.DataContext = MVMManager.Instance["PinholeControlViewModel", captureSetupViewModel];

            this.MasterView.TilesControlView.DataContext = MVMManager.Instance["XYTileControlViewModel", captureSetupViewModel];

            this.MasterView.kuriosFilterView.DataContext = MVMManager.Instance["KuriosControlViewModel", captureSetupViewModel];

            this.MasterView.QuickTemplate.DataContext = MVMManager.Instance["QuickTemplatesControlViewModel", captureSetupViewModel];

            this.MasterView.CaptureOptionsView.DataContext = MVMManager.Instance["CaptureOptionsControlViewModel", captureSetupViewModel];

            this.MasterView.ThreePhotonControlView.DataContext = MVMManager.Instance["ThreePhotonControlViewModel", captureSetupViewModel];

            this.MasterView.LightEngineControlView.DataContext = MVMManager.Instance["LightEngineControlViewModel", captureSetupViewModel];

            this.MasterView.EpiTurretControlView.DataContext = MVMManager.Instance["EpiTurretControlViewModel", captureSetupViewModel];

            this.MasterView.ObjectiveSettingsView.DataContext = MVMManager.Instance["ObjectiveControlViewModel", captureSetupViewModel];

            //this.MasterView.RemoteIPC.DataContext = MVMManager.Instance["RemoteIPCControlViewModel", captureSetupViewModel];

            this.MasterView.DFLIMControlView.DataContext = MVMManager.Instance["DFLIMControlViewModel", captureSetupViewModel];

            this.MasterView.AutoFocusControlView.DataContext = MVMManager.Instance["AutoFocusControlViewModel", captureSetupViewModel];

            this.volumeView.DataContext = captureSetupViewModel;

            this.imageView.DataContext = captureSetupViewModel;

            this.toolBarView.DataContext = captureSetupViewModel;

            captureSetupViewModel.LiveImageCapturing += new Action<bool>(ChangeTo2DViewType);
            captureSetupViewModel.ZStackCapturing += new Action<bool>(ChangeTo2DViewType);
            captureSetupViewModel.ZStackCaptureFinished += new Action<bool>(ChangeTo3DViewType);
            captureSetupViewModel.BleachFinished += new Action<bool>(ChangeTo2DViewType);
            captureSetupViewModel.IncreaseViewArea += new Action<bool>(AdjustImageViewSize);
            // liveViewModel.ImageDataChanged += new Action<bool>(ChangeTo2DViewType);

            //force the path setting from the viewmodel into the view
            string path = ((CaptureSetupViewModel)this.DataContext).ExpPath;

            //choose the default template if the current run experiment does not exists
            if (!File.Exists(path))
            {
                path = ResourceManagerCS.GetLastExperimentSettingsFileString();
            }

            ((CaptureSetupViewModel)this.DataContext).ExpPath = path;

            //force the update menu message to be published
            this.MasterView.SetPath(path);

            this.Loaded += new RoutedEventHandler(MainWindow_Loaded);
            this.Unloaded += new RoutedEventHandler(MainWindow_Unloaded);
            Application.Current.Exit += new ExitEventHandler(Current_Exit);

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
            this.KeyDown += new KeyEventHandler(MainWindow_KeyDown);
            this.KeyUp += new KeyEventHandler(MainWindow_KeyUp);
            this.MasterView.PreviewMouseDown += new MouseButtonEventHandler(MasterView_PreviewMouseDown);
        }

        #endregion Constructors

        #region Properties

        public Guid CommandGuid
        {
            get { return ((CaptureSetupViewModel)this.DataContext).CommandGuid; }
        }

        #endregion Properties

        #region Methods

        public void DisableDeviceQuery()
        {
            CaptureSetupViewModel vm = ((CaptureSetupViewModel)this.DataContext);
            if (null == vm)
            {
                return;
            }
            vm.EnableDeviceQuery = false;
        }

        public void EnableDeviceQuery()
        {
            CaptureSetupViewModel vm = ((CaptureSetupViewModel)this.DataContext);
            if (null == vm)
            {
                return;
            }
            vm.EnableDeviceQuery = true;
        }

        public void ReconnectCamera()
        {
            ((ICommand)MVMManager.Instance["CameraControlViewModel", "ReconnectCameraCommand"]).Execute(null);
        }

        public void UpdateHardwareListView()
        {
            CaptureSetupDll.View.MasterView mv = new CaptureSetupDll.View.MasterView();
            mv.UpdateHardwareListView();
        }

        //When the image is bigger than the display area, increase the display area so the scroll bar
        // can see the entire image
        void AdjustImageViewSize(bool obj)
        {
            if (true == obj)
            {
                CaptureSetupViewModel vm = ((CaptureSetupViewModel)this.DataContext);
                if (null == vm)
                {
                    return;
                }
                this.imageView.Height = Math.Max(vm.IVScrollBarHeight, Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_SCROLL);
            }
        }

        private void AdjustViewSizes()
        {
            this.MasterView.Height = Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_SCROLL;
            this.imageView.Height = Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_SCROLL;
            this.volumeView.Height = Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_SCROLL;
            this.volumeView.Width = Application.Current.MainWindow.ActualWidth - this.MasterView.Width - OFFSET_FOR_RESIZE_SCROLL;
            CaptureSetupViewModel vm = ((CaptureSetupViewModel)this.DataContext);
            if (null == vm)
            {
                return;
            }
            vm.IVHeight = this.imageView.Height;
        }

        void ChangeTo2DViewType(bool obj)
        {
            if (true == obj)
            {
                this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal,
                    new Action(
                        delegate()
                        {
                            ((CaptureSetupViewModel)this.DataContext).ViewType = Convert.ToInt32(ViewTypes.ViewType2D);
                        }
                    )
                );
            }
        }

        void ChangeTo3DViewType(bool obj)
        {
            if (true == obj)
            {
                this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal,
                    new Action(
                        delegate()
                        {

                            ((CaptureSetupViewModel)this.DataContext).ViewType = Convert.ToInt32(ViewTypes.ViewType3D);

                        }
                    )
                );

            }
        }

        void ChangeToTilesViewType(bool obj)
        {
            if (true == obj)
            {
                this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal,
                    new Action(
                        delegate()
                        {

                            ((CaptureSetupViewModel)this.DataContext).ViewType = Convert.ToInt32(ViewTypes.ViewTypeTiles);

                        }
                    )
                );
            }
        }

        void Current_Exit(object sender, ExitEventArgs e)
        {
            ((CaptureSetupViewModel)this.imageView.DataContext).ClearBleachFiles();
            ((CaptureSetupViewModel)this.imageView.DataContext).ReleaseHandlers();

            if (this.IsLoaded)
            {
                ((CaptureSetupViewModel)this.imageView.DataContext).LiveCapture(false);
                ResourceManagerCS.Instance.ActiveModality = ResourceManagerCS.GetModality();
            }
        }

        void MainWindow_KeyDown(object sender, KeyEventArgs e)
        {
            imageView.ImageView_KeyDown(sender, e);
        }

        void MainWindow_KeyUp(object sender, KeyEventArgs e)
        {
            imageView.ImageView_KeyUp(sender, e);
        }

        void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            Application.Current.MainWindow.SizeChanged += new SizeChangedEventHandler(MainWindow_SizeChanged);

            CaptureSetupViewModel vm = ((CaptureSetupViewModel)this.DataContext);
            vm.RefreshAllUIBindings();

            AdjustViewSizes();
        }

        void MainWindow_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            AdjustViewSizes();
        }

        void MainWindow_Unloaded(object sender, RoutedEventArgs e)
        {
            Application.Current.MainWindow.SizeChanged -= new SizeChangedEventHandler(MainWindow_SizeChanged);
        }

        void MasterView_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            this.imageView.SetSelectROI();
        }

        private void scrollViewImage_ScrollChanged(object sender, ScrollChangedEventArgs e)
        {
            CaptureSetupViewModel vm = ((CaptureSetupViewModel)this.DataContext);
            if (null == vm)
            {
                return;
            }
            vm.ROItoolbarMargin = new Thickness(0, scrollViewImage.ContentVerticalOffset, 0, 0);
        }

        #endregion Methods
    }

    public class ViewTypeToVisibilityConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            Visibility val = Visibility.Hidden;
            switch (Convert.ToInt32(value))
            {
                case 0:
                    {
                        if (0 == Convert.ToInt32(parameter))
                            val = Visibility.Visible;
                        else
                            val = Visibility.Hidden;
                    }
                    break;
                case 1:
                    {
                        if (1 == Convert.ToInt32(parameter))
                            val = Visibility.Visible;
                        else
                            val = Visibility.Hidden;
                    }
                    break;
                case 2:
                    {
                        if (2 == Convert.ToInt32(parameter))
                            val = Visibility.Visible;
                        else
                            val = Visibility.Hidden;
                    }
                    break;

            }
            return val;
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            int val = 0;
            switch ((Visibility)value)
            {
                case Visibility.Visible:
                    {
                        if (0 == Convert.ToInt32(parameter))
                            val = 0;
                        else
                            val = 1;
                    }
                    break;
                case Visibility.Hidden:
                    {
                        if (1 == Convert.ToInt32(parameter))
                            val = 0;
                        else
                            val = 1;
                    }
                    break;

            }
            return val;
        }

        #endregion Methods
    }
}