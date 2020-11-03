namespace CaptureSetupDll.View
{
    using System;
    using System.Collections.Generic;
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
    using System.Windows.Threading;
    using System.Xml;

    using CaptureSetupDll.ViewModel;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for BleachControlView.xaml
    /// </summary>
    public partial class BleachControlView : UserControl
    {
        #region Fields

        CaptureSetupViewModel _vm;

        #endregion Fields

        #region Constructors

        public BleachControlView(CaptureSetupViewModel vm)
        {
            InitializeComponent();

            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this) == true)
                return;

            //_RecROIUpdateTimer = new DispatcherTimer();
            //_RecROIUpdateTimer.Interval = TimeSpan.FromSeconds(0.5);
            _vm = vm;
            this.Loaded += new RoutedEventHandler(BleachControlView_Loaded);
            this.Unloaded += new RoutedEventHandler(BleachControlView_Unloaded);
        }

        #endregion Constructors

        #region Methods

        void BleachControlView_Loaded(object sender, RoutedEventArgs e)
        {
            //_RecROIUpdateTimer.Tick += new EventHandler(_RecROIUpdateTimer_Tick);
            //_RecROIUpdateTimer.Start();

            //Use the visibitlity settings in application settings to setup the visibility on the controls
            //Bleach scanner control can be hidden
            stpBleacherControl.Visibility = CaptureSetupViewModel.GetVisibility("/ApplicationSettings/DisplayOptions/CaptureSetup/BleachView/BleachCalibrationTool", "Visibility");

            stkBleachPockel.DataContext = MVMManager.Instance["PowerControlViewModel", _vm];
            lblCalibrationDate.DataContext = MVMManager.Instance["AreaControlViewModel", _vm];
        }

        void BleachControlView_Unloaded(object sender, RoutedEventArgs e)
        {
            //_RecROIUpdateTimer.Stop();
            //_RecROIUpdateTimer.Tick -= new EventHandler(_RecROIUpdateTimer_Tick);
        }

        #endregion Methods

        #region Other

        /*void _RecROIUpdateTimer_Tick(object sender, EventArgs e)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            if (OverlayManagerClass.InstanceClass.Instance.ActiveTypeIsRec)
            {
                double Width = 0, Height = 0, OffsetX = 0, OffsetY = 0;
                OverlayManagerClass.InstanceClass.Instance.GetRecActivationParams(ref Width, ref Height, ref OffsetX, ref OffsetY);
                vm.ROIWidth = Math.Round(Width,2);
                vm.ROIHeight = Math.Round(Height,2);
                vm.ROIOffsetX = Math.Round(OffsetX,2);
                vm.ROIOffsetY = Math.Round(OffsetY,2);
            }
        }*/
        //private DispatcherTimer _RecROIUpdateTimer;

        #endregion Other
    }
}