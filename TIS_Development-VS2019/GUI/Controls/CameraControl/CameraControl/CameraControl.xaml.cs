namespace CameraControl
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Text.RegularExpressions;
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

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class CameraControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty AutoExposurePercentProperty = DependencyProperty.Register("AutoExposurePercent", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty AutoWhiteBalanceCommandProperty = DependencyProperty.Register("AutoWhiteBalanceCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty BinIndexProperty = DependencyProperty.Register("BinIndex", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty BinListProperty = DependencyProperty.Register("BinList", typeof(ObservableCollection<string>), typeof(CameraControlUC));
        public static readonly DependencyProperty BinningOrcaTypeProperty = DependencyProperty.Register("BinningOrcaType", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty BinXMinusCommandProperty = DependencyProperty.Register("BinXMinusCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty BinXPlusCommandProperty = DependencyProperty.Register("BinXPlusCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty BinXProperty = DependencyProperty.Register("BinX", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty BinYMinusCommandProperty = DependencyProperty.Register("BinYMinusCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty BinYPlusCommandProperty = DependencyProperty.Register("BinYPlusCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty BinYProperty = DependencyProperty.Register("BinY", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty BlueGainProperty = DependencyProperty.Register("BlueGain", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty BottomMaxProperty = DependencyProperty.Register("BottomMax", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty BottomMinProperty = DependencyProperty.Register("BottomMin", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty BottomProperty = DependencyProperty.Register("Bottom", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CamAddResolutionCommandProperty = DependencyProperty.Register("CamAddResolutionCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty CamAverageModeProperty = DependencyProperty.Register("CamAverageMode", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CamAverageNumMinusCommandProperty = DependencyProperty.Register("CamAverageNumMinusCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty CamAverageNumPlusCommandProperty = DependencyProperty.Register("CamAverageNumPlusCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty CamAverageNumProperty = DependencyProperty.Register("CamAverageNum", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CamBitsPerPixelProperty = DependencyProperty.Register("CamBitsPerPixel", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CamBlackLevelProperty = DependencyProperty.Register("CamBlackLevel", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CameraColorImageTypeIndexProperty = DependencyProperty.Register("CameraColorImageTypeIndex", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CameraColorImageTypeListProperty = DependencyProperty.Register("CameraColorImageTypeList", typeof(ObservableCollection<string>), typeof(CameraControlUC));
        public static readonly DependencyProperty CameraCSTypeProperty = DependencyProperty.Register("CameraCSType", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty CameraImageAngleMinusCommandProperty = DependencyProperty.Register("CameraImageAngleMinusCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty CameraImageAnglePlusCommandProperty = DependencyProperty.Register("CameraImageAnglePlusCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty CameraImageAngleProperty = DependencyProperty.Register("CameraImageAngle", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CameraPolarImageTypeIndexProperty = DependencyProperty.Register("CameraPolarImageTypeIndex", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CameraPolarImageTypeListProperty = DependencyProperty.Register("CameraPolarImageTypeList", typeof(ObservableCollection<string>), typeof(CameraControlUC));
        public static readonly DependencyProperty CameraRegionHeightUMProperty = DependencyProperty.Register("CameraRegionHeightUM", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty CameraRegionWidthUMProperty = DependencyProperty.Register("CameraRegionWidthUM", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty CamExposureTimeMinusCommandProperty = DependencyProperty.Register("CamExposureTimeMinusCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty CamExposureTimePlusCommandProperty = DependencyProperty.Register("CamExposureTimePlusCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty CamFullFrameCommandProperty = DependencyProperty.Register("CamFullFrameCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty CamHorizontalFlipProperty = DependencyProperty.Register("CamHorizontalFlip", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CamImageHeightProperty = DependencyProperty.Register("CamImageHeight", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CamImageWidthProperty = DependencyProperty.Register("CamImageWidth", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CamLedAvailableProperty = DependencyProperty.Register("CamLedAvailable", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty CamLedEnableProperty = DependencyProperty.Register("CamLedEnable", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CamOrcaFrameRateProperty = DependencyProperty.Register("CamOrcaFrameRate", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty CamPixelSizeUMProperty = DependencyProperty.Register("CamPixelSizeUM", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty CamReadoutSpeedIndexProperty = DependencyProperty.Register("CamReadoutSpeedIndex", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CamReadoutSpeedListProperty = DependencyProperty.Register("CamReadoutSpeedList", typeof(ObservableCollection<string>), typeof(CameraControlUC));
        public static readonly DependencyProperty CamRegionFromROICommandProperty = DependencyProperty.Register("CamRegionFromROICommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty CamResolutionPresetsProperty = DependencyProperty.Register("CamResolutionPresets", typeof(List<string>), typeof(CameraControlUC));
        public static readonly DependencyProperty CamResolutionProperty = DependencyProperty.Register("CamResolution", typeof(StringPC), typeof(CameraControlUC));
        public static readonly DependencyProperty CamTapBalanceProperty = DependencyProperty.Register("CamTapBalance", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CamTapindexMaxProperty = DependencyProperty.Register("CamTapindexMax", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CamTapindexMinProperty = DependencyProperty.Register("CamTapindexMin", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CamTapIndexProperty = DependencyProperty.Register("CamTapIndex", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CamVerticalFlipProperty = DependencyProperty.Register("CamVerticalFlip", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty ContinuousWhiteBalanceNumFramesDecrementCommandProperty = DependencyProperty.Register("ContinuousWHiteBalanceNumFramesDecrementCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty ContinuousWhiteBalanceNumFramesIncrementCommandProperty = DependencyProperty.Register("ContinuousWHiteBalanceNumFramesIncrementCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty ContinuousWhiteBalanceNumFramesProperty = DependencyProperty.Register("ContinuousWhiteBalanceNumFrames", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CoolingModeIndexProperty = DependencyProperty.Register("CoolingModeIndex", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty CoolingModeListProperty = DependencyProperty.Register("CoolingModeList", typeof(ObservableCollection<string>), typeof(CameraControlUC));
        public static readonly DependencyProperty CoolingVisibilityProperty = DependencyProperty.Register("CoolingVisibility", typeof(Visibility), typeof(CameraControlUC));
        public static readonly DependencyProperty EqualExposurePulseWidthProperty = DependencyProperty.Register("EqualExposurePulseWidth", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty ExposureTimeCamProperty = DependencyProperty.Register("ExposureTimeCam", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty ExposureTimeMaxProperty = DependencyProperty.Register("ExposureTimeMax", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty ExposureTimeMinProperty = DependencyProperty.Register("ExposureTimeMin", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty FrameRateControlEnabledProperty = DependencyProperty.Register("FrameRateControlEnabled", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty FrameRateControlMaxProperty = DependencyProperty.Register("FrameRateControlMax", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty FrameRateControlMinProperty = DependencyProperty.Register("FrameRateControlMin", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty FrameRateControlMinusCommandProperty = DependencyProperty.Register("FrameRateControlMinusCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty FrameRateControlPlusCommandProperty = DependencyProperty.Register("FrameRateControlPlusCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty FrameRateControlValueProperty = DependencyProperty.Register("FrameRateControlValue", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty FrameRateControlVisibilityProperty = DependencyProperty.Register("FrameRateControlVisibility", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty GainDecibelsProperty = DependencyProperty.Register("GainDecibels", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty GainDiscreteOptionsProperty = DependencyProperty.Register("GainDiscreteOptions", typeof(List<string>), typeof(CameraControlUC));
        public static readonly DependencyProperty GainProperty = DependencyProperty.Register("Gain", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty GreenGainProperty = DependencyProperty.Register("GreenGain", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty HotPixelCBVisibilityProperty = DependencyProperty.Register("HotPixelCBVisibility", typeof(Visibility), typeof(CameraControlUC));
        public static readonly DependencyProperty HotPixelEnabledProperty = DependencyProperty.Register("HotPixelEnabled", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty HotPixelLevelIndexProperty = DependencyProperty.Register("HotPixelLevelIndex", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty HotPixelLevelListProperty = DependencyProperty.Register("HotPixelLevelList", typeof(ObservableCollection<string>), typeof(CameraControlUC));
        public static readonly DependencyProperty HotPixelMinusCommandProperty = DependencyProperty.Register("HotPixelMinusCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty HotPixelPlusCommandProperty = DependencyProperty.Register("HotPixelPlusCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty HotPixelValProperty = DependencyProperty.Register("HotPixelVal", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty HotPixelVisProperty = DependencyProperty.Register("HotPixelVis", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty ImageStartStatusCameraProperty = DependencyProperty.Register("ImageStartStatusCamera", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsAutoExposureSettingEnabledProperty = DependencyProperty.Register("IsAutoExposureSettingEnabled", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsAutoExposureSupportedProperty = DependencyProperty.Register("IsAutoExposureSupported", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsBlackLevelVisibleProperty = DependencyProperty.Register("IsBlackLevelVisible", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsCameraColorImageTypeVisibleProperty = DependencyProperty.Register("IsCameraColorImageTypeVisible", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsCameraPolarImageTypeVisibleProperty = DependencyProperty.Register("IsCameraPolarImageTypeVisible", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsColorGainsVisibleProperty = DependencyProperty.Register("IsColorGainsVisible", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsContinuousWhiteBalanceEnabledProperty = DependencyProperty.Register("IsContinuousWhiteBalanceEnabled", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsEqualExposurePulseEnabledProperty = DependencyProperty.Register("IsEqualExposurePulseEnabled", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsEqualExposurePulseVisibleProperty = DependencyProperty.Register("IsEqualExposurePulseVisible", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsExposureSettingEnabledProperty = DependencyProperty.Register("IsExposureSettingEnabled", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsFrameRateVisibleProperty = DependencyProperty.Register("IsFrameRateVisible", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsGainContiguousProperty = DependencyProperty.Register("IsGainContiguous", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsGainInDecibelsProperty = DependencyProperty.Register("IsGainInDecibels", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsGainVisibleProperty = DependencyProperty.Register("IsGainVisible", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsNirBoostEnabledProperty = DependencyProperty.Register("IsNirBoostEnabled", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsNirBoostVisibleProperty = DependencyProperty.Register("IsNirBoostVisible", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsReadoutVisibleProperty = DependencyProperty.Register("IsReadoutVisible", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty IsTapsVisibleProperty = DependencyProperty.Register("IsTapsVisible", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty LeftMaxProperty = DependencyProperty.Register("LeftMax", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty LeftMinProperty = DependencyProperty.Register("LeftMin", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty LeftProperty = DependencyProperty.Register("Left", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty OrcaFrameRateEnabledProperty = DependencyProperty.Register("OrcaFrameRateEnabled", typeof(bool), typeof(CameraControlUC));
        public static readonly DependencyProperty PerformAutoWhiteBalanceCommandProperty = DependencyProperty.Register("PerformAutoWhiteBalanceCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty PixelSizeUMProperty = DependencyProperty.Register("PixelSizeUM", typeof(string), typeof(CameraControlUC));
        public static readonly DependencyProperty PulseWidthMinusCommandProperty = DependencyProperty.Register("PulseWidthMinusCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty PulseWidthPlusCommandProperty = DependencyProperty.Register("PulseWidthPlusCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty RedGainProperty = DependencyProperty.Register("RedGain", typeof(double), typeof(CameraControlUC));
        public static readonly DependencyProperty RightMaxProperty = DependencyProperty.Register("RightMax", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty RightMinProperty = DependencyProperty.Register("RightMin", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty RightProperty = DependencyProperty.Register("Right", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty RunAutoExposureCommandProperty = DependencyProperty.Register("RunAutoExposureCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty SetDefaultColorGainsCommandProperty = DependencyProperty.Register("SetDefaultColorGainsCommand", typeof(ICommand), typeof(CameraControlUC));
        public static readonly DependencyProperty TopMaxProperty = DependencyProperty.Register("TopMax", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty TopMinProperty = DependencyProperty.Register("TopMin", typeof(int), typeof(CameraControlUC));
        public static readonly DependencyProperty TopProperty = DependencyProperty.Register("Top", typeof(int), typeof(CameraControlUC));

        #endregion Fields

        #region Constructors

        public CameraControlUC()
        {
            InitializeComponent();
            this.Loaded += new RoutedEventHandler(CameraControl_Loaded);
        }

        #endregion Constructors

        #region Properties

        public double AutoExposurePercent
        {
            get { return (double)GetValue(AutoExposurePercentProperty); }
            set { SetValue(AutoExposurePercentProperty, value); }
        }

        public ICommand AutoWhiteBalanceCommand
        {
            get { return (ICommand)GetValue(AutoWhiteBalanceCommandProperty); }
            set { SetValue(AutoWhiteBalanceCommandProperty, value); }
        }

        public int BinIndex
        {
            get { return (int)GetValue(BinIndexProperty); }
            set { SetValue(BinIndexProperty, value); }
        }

        public ObservableCollection<string> BinList
        {
            get { return (ObservableCollection<string>)GetValue(BinListProperty); }
            set { SetValue(BinListProperty, value); }
        }

        public bool BinningOrcaType
        {
            get { return (bool)GetValue(BinningOrcaTypeProperty); }
            set { SetValue(BinningOrcaTypeProperty, value); }
        }

        public int BinX
        {
            get { return (int)GetValue(BinXProperty); }
            set { SetValue(BinXProperty, value); }
        }

        public ICommand BinXMinusCommand
        {
            get { return (ICommand)GetValue(BinXMinusCommandProperty); }
            set { SetValue(BinXMinusCommandProperty, value); }
        }

        public ICommand BinXPlusCommand
        {
            get { return (ICommand)GetValue(BinXPlusCommandProperty); }
            set { SetValue(BinXPlusCommandProperty, value); }
        }

        public int BinY
        {
            get { return (int)GetValue(BinYProperty); }
            set { SetValue(BinYProperty, value); }
        }

        public ICommand BinYMinusCommand
        {
            get { return (ICommand)GetValue(BinYMinusCommandProperty); }
            set { SetValue(BinYMinusCommandProperty, value); }
        }

        public ICommand BinYPlusCommand
        {
            get { return (ICommand)GetValue(BinYPlusCommandProperty); }
            set { SetValue(BinYPlusCommandProperty, value); }
        }

        public double BlueGain
        {
            get { return (double)GetValue(BlueGainProperty); }
            set { SetValue(BlueGainProperty, value); }
        }

        public int Bottom
        {
            get { return (int)GetValue(BottomProperty); }
            set { SetValue(BottomProperty, value); }
        }

        public int BottomMax
        {
            get { return (int)GetValue(BottomMaxProperty); }
            set { SetValue(BottomMaxProperty, value); }
        }

        public int BottomMin
        {
            get { return (int)GetValue(BottomMinProperty); }
            set { SetValue(BottomMinProperty, value); }
        }

        public ICommand CamAddResolutionCommand
        {
            get { return (ICommand)GetValue(CamAddResolutionCommandProperty); }
            set { SetValue(CamAddResolutionCommandProperty, value); }
        }

        public int CamAverageMode
        {
            get { return (int)GetValue(CamAverageModeProperty); }
            set { SetValue(CamAverageModeProperty, value); }
        }

        public int CamAverageNum
        {
            get { return (int)GetValue(CamAverageNumProperty); }
            set { SetValue(CamAverageNumProperty, value); }
        }

        public ICommand CamAverageNumMinusCommand
        {
            get { return (ICommand)GetValue(CamAverageNumMinusCommandProperty); }
            set { SetValue(CamAverageNumMinusCommandProperty, value); }
        }

        public ICommand CamAverageNumPlusCommand
        {
            get { return (ICommand)GetValue(CamAverageNumPlusCommandProperty); }
            set { SetValue(CamAverageNumPlusCommandProperty, value); }
        }

        public int CamBitsPerPixel
        {
            get { return (int)GetValue(CamBitsPerPixelProperty); }
            set { SetValue(CamBitsPerPixelProperty, value); }
        }

        public int CamBlackLevel
        {
            get { return (int)GetValue(CamBlackLevelProperty); }
            set { SetValue(CamBlackLevelProperty, value); }
        }

        public int CameraColorImageTypeIndex
        {
            get { return (int)GetValue(CameraColorImageTypeIndexProperty); }
            set { SetValue(CameraColorImageTypeIndexProperty, value); }
        }

        public ObservableCollection<string> CameraColorImageTypeList
        {
            get { return (ObservableCollection<string>)GetValue(CameraColorImageTypeListProperty); }
            set { SetValue(CameraColorImageTypeListProperty, value); }
        }

        public bool CameraCSType
        {
            get { return (bool)GetValue(CameraCSTypeProperty); }
            set { SetValue(CameraCSTypeProperty, value); }
        }

        public int CameraImageAngle
        {
            get { return (int)GetValue(CameraImageAngleProperty); }
            set { SetValue(CameraImageAngleProperty, value); }
        }

        public ICommand CameraImageAngleMinusCommand
        {
            get { return (ICommand)GetValue(CameraImageAngleMinusCommandProperty); }
            set { SetValue(CameraImageAngleMinusCommandProperty, value); }
        }

        public ICommand CameraImageAnglePlusCommand
        {
            get { return (ICommand)GetValue(CameraImageAnglePlusCommandProperty); }
            set { SetValue(CameraImageAnglePlusCommandProperty, value); }
        }

        public int CameraPolarImageTypeIndex
        {
            get { return (int)GetValue(CameraPolarImageTypeIndexProperty); }
            set { SetValue(CameraPolarImageTypeIndexProperty, value); }
        }

        public ObservableCollection<string> CameraPolarImageTypeList
        {
            get { return (ObservableCollection<string>)GetValue(CameraPolarImageTypeListProperty); }
            set { SetValue(CameraPolarImageTypeListProperty, value); }
        }

        public double CameraRegionHeightUM
        {
            get { return (double)GetValue(CameraRegionHeightUMProperty); }
            set { SetValue(CameraRegionHeightUMProperty, value); }
        }

        public double CameraRegionWidthUM
        {
            get { return (double)GetValue(CameraRegionWidthUMProperty); }
            set { SetValue(CameraRegionWidthUMProperty, value); }
        }

        public ICommand CamExposureTimeMinusCommand
        {
            get { return (ICommand)GetValue(CamExposureTimeMinusCommandProperty); }
            set { SetValue(CamExposureTimeMinusCommandProperty, value); }
        }

        public ICommand CamExposureTimePlusCommand
        {
            get { return (ICommand)GetValue(CamExposureTimePlusCommandProperty); }
            set { SetValue(CamExposureTimePlusCommandProperty, value); }
        }

        public ICommand CamFullFrameCommand
        {
            get { return (ICommand)GetValue(CamFullFrameCommandProperty); }
            set { SetValue(CamFullFrameCommandProperty, value); }
        }

        public int CamHorizontalFlip
        {
            get { return (int)GetValue(CamHorizontalFlipProperty); }
            set { SetValue(CamHorizontalFlipProperty, value); }
        }

        public int CamImageHeight
        {
            get { return (int)GetValue(CamImageHeightProperty); }
            set { SetValue(CamImageHeightProperty, value); }
        }

        public int CamImageWidth
        {
            get { return (int)GetValue(CamImageWidthProperty); }
            set { SetValue(CamImageWidthProperty, value); }
        }

        public bool CamLedAvailable
        {
            get { return (bool)GetValue(CamLedAvailableProperty); }
            set { SetValue(CamLedAvailableProperty, value); }
        }

        public int CamLedEnable
        {
            get { return (int)GetValue(CamLedEnableProperty); }
            set { SetValue(CamLedEnableProperty, value); }
        }

        public double CamOrcaFrameRate
        {
            get { return (double)GetValue(CamOrcaFrameRateProperty); }
            set { SetValue(CamOrcaFrameRateProperty, value); }
        }

        public double CamPixelSizeUM
        {
            get { return (double)GetValue(CamPixelSizeUMProperty); }
            set { SetValue(CamPixelSizeUMProperty, value); }
        }

        public int CamReadoutSpeedIndex
        {
            get { return (int)GetValue(CamReadoutSpeedIndexProperty); }
            set { SetValue(CamReadoutSpeedIndexProperty, value); }
        }

        public ObservableCollection<string> CamReadoutSpeedList
        {
            get { return (ObservableCollection<string>)GetValue(CamReadoutSpeedListProperty); }
            set { SetValue(CamReadoutSpeedListProperty, value); }
        }

        public ICommand CamRegionFromROICommand
        {
            get { return (ICommand)GetValue(CamRegionFromROICommandProperty); }
            set { SetValue(CamRegionFromROICommandProperty, value); }
        }

        public StringPC CamResolution
        {
            get { return (StringPC)GetValue(CamResolutionProperty); }
            set { SetValue(CamResolutionProperty, value); }
        }

        public List<string> CamResolutionPresets
        {
            get { return (List<string>)GetValue(CamResolutionPresetsProperty); }
            set { SetValue(CamResolutionPresetsProperty, value); }
        }

        public int CamTapBalance
        {
            get { return (int)GetValue(CamTapBalanceProperty); }
            set { SetValue(CamTapBalanceProperty, value); }
        }

        public int CamTapIndex
        {
            get { return (int)GetValue(CamTapIndexProperty); }
            set { SetValue(CamTapIndexProperty, value); }
        }

        public int CamTapindexMax
        {
            get { return (int)GetValue(CamTapindexMaxProperty); }
            set { SetValue(CamTapindexMaxProperty, value); }
        }

        public int CamTapindexMin
        {
            get { return (int)GetValue(CamTapindexMinProperty); }
            set { SetValue(CamTapindexMinProperty, value); }
        }

        public int CamVerticalFlip
        {
            get { return (int)GetValue(CamVerticalFlipProperty); }
            set { SetValue(CamVerticalFlipProperty, value); }
        }

        public int ContinuousWhiteBalanceNumFrames
        {
            get { return (int)GetValue(ContinuousWhiteBalanceNumFramesProperty); }
            set { SetValue(ContinuousWhiteBalanceNumFramesProperty, value); }
        }

        public ICommand ContinuousWhiteBalanceNumFramesDecrementCommand
        {
            get { return (ICommand)GetValue(ContinuousWhiteBalanceNumFramesDecrementCommandProperty); }
            set { SetValue(ContinuousWhiteBalanceNumFramesDecrementCommandProperty, value); }
        }

        public ICommand ContinuousWhiteBalanceNumFramesIncrementCommand
        {
            get { return (ICommand)GetValue(ContinuousWhiteBalanceNumFramesIncrementCommandProperty); }
            set { SetValue(ContinuousWhiteBalanceNumFramesIncrementCommandProperty, value); }
        }

        public int CoolingModeIndex
        {
            get { return (int)GetValue(CoolingModeIndexProperty); }
            set { SetValue(CoolingModeIndexProperty, value); }
        }

        public ObservableCollection<string> CoolingModeList
        {
            get { return (ObservableCollection<string>)GetValue(CoolingModeListProperty); }
            set { SetValue(CoolingModeListProperty, value); }
        }

        public Visibility CoolingVisibility
        {
            get { return (Visibility)GetValue(CoolingVisibilityProperty); }
            set { SetValue(CoolingVisibilityProperty, value); }
        }

        public double EqualExposurePulseWidth
        {
            get { return (double)GetValue(EqualExposurePulseWidthProperty); }
            set { SetValue(EqualExposurePulseWidthProperty, value); }
        }

        public double ExposureTimeCam
        {
            get { return (double)GetValue(ExposureTimeCamProperty); }
            set { SetValue(ExposureTimeCamProperty, value); }
        }

        public double ExposureTimeMax
        {
            get { return (double)GetValue(ExposureTimeMaxProperty); }
            set { SetValue(ExposureTimeMaxProperty, value); }
        }

        public double ExposureTimeMin
        {
            get { return (double)GetValue(ExposureTimeMinProperty); }
            set { SetValue(ExposureTimeMinProperty, value); }
        }

        public int FrameRateControlEnabled
        {
            get { return (int)GetValue(FrameRateControlEnabledProperty); }
            set { SetValue(FrameRateControlEnabledProperty, value); }
        }

        public double FrameRateControlMax
        {
            get { return (double)GetValue(FrameRateControlMaxProperty); }
            set { SetValue(FrameRateControlMaxProperty, value); }
        }

        public double FrameRateControlMin
        {
            get { return (double)GetValue(FrameRateControlMinProperty); }
            set { SetValue(FrameRateControlMinProperty, value); }
        }

        public ICommand FrameRateControlMinusCommand
        {
            get { return (ICommand)GetValue(FrameRateControlMinusCommandProperty); }
            set { SetValue(FrameRateControlMinusCommandProperty, value); }
        }

        public ICommand FrameRateControlPlusCommand
        {
            get { return (ICommand)GetValue(FrameRateControlPlusCommandProperty); }
            set { SetValue(FrameRateControlPlusCommandProperty, value); }
        }

        public double FrameRateControlValue
        {
            get { return (double)GetValue(FrameRateControlValueProperty); }
            set { SetValue(FrameRateControlValueProperty, value); }
        }

        public bool FrameRateControlVisibility
        {
            get { return (bool)GetValue(FrameRateControlVisibilityProperty); }
            set { SetValue(FrameRateControlVisibilityProperty, value); }
        }

        public double Gain
        {
            get { return (int)GetValue(GainProperty); }
            set { SetValue(GainProperty, value); }
        }

        public double GainDecibels
        {
            get { return (double)GetValue(GainDecibelsProperty); }
            set { SetValue(GainDecibelsProperty, value); }
        }

        public List<string> GainDiscreteOptions
        {
            get { return (List<string>)GetValue(GainDiscreteOptionsProperty); }
            set { SetValue(GainDiscreteOptionsProperty, value); }
        }

        public double GreenGain
        {
            get { return (double)GetValue(GreenGainProperty); }
            set { SetValue(GreenGainProperty, value); }
        }

        public Visibility HotPixelCBVisibility
        {
            get { return (Visibility)GetValue(HotPixelCBVisibilityProperty); }
            set { SetValue(HotPixelCBVisibilityProperty, value); }
        }

        public int HotPixelEnabled
        {
            get { return (int)GetValue(HotPixelEnabledProperty); }
            set { SetValue(HotPixelEnabledProperty, value); }
        }

        public int HotPixelLevelIndex
        {
            get { return (int)GetValue(HotPixelLevelIndexProperty); }
            set { SetValue(HotPixelLevelIndexProperty, value); }
        }

        public ObservableCollection<string> HotPixelLevelList
        {
            get { return (ObservableCollection<string>)GetValue(HotPixelLevelListProperty); }
            set { SetValue(HotPixelLevelListProperty, value); }
        }

        public ICommand HotPixelMinusCommand
        {
            get { return (ICommand)GetValue(HotPixelMinusCommandProperty); }
            set { SetValue(HotPixelMinusCommandProperty, value); }
        }

        public ICommand HotPixelPlusCommand
        {
            get { return (ICommand)GetValue(HotPixelPlusCommandProperty); }
            set { SetValue(HotPixelPlusCommandProperty, value); }
        }

        public double HotPixelVal
        {
            get { return (double)GetValue(HotPixelValProperty); }
            set { SetValue(HotPixelValProperty, value); }
        }

        public bool HotPixelVis
        {
            get { return (bool)GetValue(HotPixelVisProperty); }
            set { SetValue(HotPixelVisProperty, value); }
        }

        public bool ImageStartStatusCamera
        {
            get { return (bool)GetValue(ImageStartStatusCameraProperty); }
            set { SetValue(ImageStartStatusCameraProperty, value); }
        }

        public bool IsAutoExposureSettingEnabled
        {
            get { return (bool)GetValue(IsAutoExposureSettingEnabledProperty); }
            set { SetValue(IsAutoExposureSettingEnabledProperty, value); }
        }

        public bool IsAutoExposureSupported
        {
            get { return (bool)GetValue(IsAutoExposureSupportedProperty); }
            set { SetValue(IsAutoExposureSupportedProperty, value); }
        }

        public bool IsBlackLevelVisible
        {
            get { return (bool)GetValue(IsBlackLevelVisibleProperty); }
            set { SetValue(IsBlackLevelVisibleProperty, value); }
        }

        public bool IsCameraColorImageTypeVisible
        {
            get { return (bool)GetValue(IsCameraColorImageTypeVisibleProperty); }
            set { SetValue(IsCameraColorImageTypeVisibleProperty, value); }
        }

        public bool IsCameraPolarImageTypeVisible
        {
            get { return (bool)GetValue(IsCameraPolarImageTypeVisibleProperty); }
            set { SetValue(IsCameraPolarImageTypeVisibleProperty, value); }
        }

        public bool IsColorGainsVisible
        {
            get { return (bool)GetValue(IsColorGainsVisibleProperty); }
            set { SetValue(IsColorGainsVisibleProperty, value); }
        }

        public bool IsContinuousWhiteBalanceEnabled
        {
            get { return (bool)GetValue(IsContinuousWhiteBalanceEnabledProperty); }
            set { SetValue(IsContinuousWhiteBalanceEnabledProperty, value); }
        }

        public bool IsEqualExposurePulseEnabled
        {
            get { return (bool)GetValue(IsEqualExposurePulseEnabledProperty); }
            set { SetValue(IsEqualExposurePulseVisibleProperty, value); }
        }

        public bool IsEqualExposurePulseVisible
        {
            get { return (bool)GetValue(IsEqualExposurePulseVisibleProperty); }
            set { SetValue(IsEqualExposurePulseVisibleProperty, value); }
        }

        public bool IsExposureSettingEnabled
        {
            get { return (bool)GetValue(IsExposureSettingEnabledProperty); }
            set { SetValue(IsExposureSettingEnabledProperty, value); }
        }

        public bool IsFrameRateVisible
        {
            get { return (bool)GetValue(IsFrameRateVisibleProperty); }
            set { SetValue(IsFrameRateVisibleProperty, value); }
        }

        public bool IsGainContiguous
        {
            get { return (bool)GetValue(IsGainContiguousProperty); }
            set { SetValue(IsGainContiguousProperty, value); }
        }

        public bool IsGainInDecibels
        {
            get { return (bool)GetValue(IsGainInDecibelsProperty); }
            set { SetValue(IsGainInDecibelsProperty, value); }
        }

        public bool IsGainVisible
        {
            get { return (bool)GetValue(IsGainVisibleProperty); }
            set { SetValue(IsGainVisibleProperty, value); }
        }

        public bool IsNirBoostEnabled
        {
            get { return (bool)GetValue(IsNirBoostEnabledProperty); }
            set { SetValue(IsNirBoostEnabledProperty, value); }
        }

        public bool IsNirBoostVisible
        {
            get { return (bool)GetValue(IsNirBoostVisibleProperty); }
            set { SetValue(IsNirBoostVisibleProperty, value); }
        }

        public bool IsReadoutVisible
        {
            get { return (bool)GetValue(IsReadoutVisibleProperty); }
            set { SetValue(IsReadoutVisibleProperty, value); }
        }

        public bool IsTapsVisible
        {
            get { return (bool)GetValue(IsTapsVisibleProperty); }
            set { SetValue(IsTapsVisibleProperty, value); }
        }

        public int Left
        {
            get { return (int)GetValue(LeftProperty); }
            set { SetValue(LeftProperty, value); }
        }

        public int LeftMax
        {
            get { return (int)GetValue(LeftMaxProperty); }
            set { SetValue(LeftMaxProperty, value); }
        }

        public int LeftMin
        {
            get { return (int)GetValue(LeftMinProperty); }
            set { SetValue(LeftMinProperty, value); }
        }

        public bool OrcaFrameRateEnabled
        {
            get { return (bool)GetValue(OrcaFrameRateEnabledProperty); }
            set { SetValue(OrcaFrameRateEnabledProperty, value); }
        }

        public ICommand PerformAutoWhiteBalanceCommand
        {
            get { return (ICommand)GetValue(PerformAutoWhiteBalanceCommandProperty); }
            set { SetValue(PerformAutoWhiteBalanceCommandProperty, value); }
        }

        public string PixelSizeUM
        {
            get { return (string)GetValue(PixelSizeUMProperty); }
            set { SetValue(PixelSizeUMProperty, value); }
        }

        public ICommand PulseWidthMinusCommand
        {
            get { return (ICommand)GetValue(PulseWidthMinusCommandProperty); }
            set { SetValue(PulseWidthMinusCommandProperty, value); }
        }

        public ICommand PulseWidthPlusCommand
        {
            get { return (ICommand)GetValue(PulseWidthPlusCommandProperty); }
            set { SetValue(PulseWidthPlusCommandProperty, value); }
        }

        public double RedGain
        {
            get { return (double)GetValue(RedGainProperty); }
            set { SetValue(RedGainProperty, value); }
        }

        public int Right
        {
            get { return (int)GetValue(RightProperty); }
            set { SetValue(RightProperty, value); }
        }

        public int RightMax
        {
            get { return (int)GetValue(RightMaxProperty); }
            set { SetValue(RightMaxProperty, value); }
        }

        public int RightMin
        {
            get { return (int)GetValue(RightMinProperty); }
            set { SetValue(RightMinProperty, value); }
        }

        public ICommand RunAutoExposureCommand
        {
            get { return (ICommand)GetValue(RunAutoExposureCommandProperty); }
            set { SetValue(RunAutoExposureCommandProperty, value); }
        }

        public ICommand SetDefaultColorGainsCommand
        {
            get { return (ICommand)GetValue(SetDefaultColorGainsCommandProperty); }
            set { SetValue(SetDefaultColorGainsCommandProperty, value); }
        }

        public int Top
        {
            get { return (int)GetValue(TopProperty); }
            set { SetValue(TopProperty, value); }
        }

        public int TopMax
        {
            get { return (int)GetValue(TopMaxProperty); }
            set { SetValue(TopMaxProperty, value); }
        }

        public int TopMin
        {
            get { return (int)GetValue(TopMinProperty); }
            set { SetValue(TopMinProperty, value); }
        }

        #endregion Properties

        #region Methods

        private void CameraControl_Loaded(object sender, RoutedEventArgs e)
        {
            int currentTapIndex = CamTapIndex;
            int maxTapIndex = CamTapindexMax;

            switch (maxTapIndex)
            {
                case 0:
                    (cbCamTaps.Items[2] as ComboBoxItem).Visibility = Visibility.Collapsed;
                    (cbCamTaps.Items[1] as ComboBoxItem).Visibility = Visibility.Collapsed;
                    break;
                case 1:
                    (cbCamTaps.Items[2] as ComboBoxItem).Visibility = Visibility.Collapsed;
                    (cbCamTaps.Items[1] as ComboBoxItem).Visibility = Visibility.Visible;
                    break;
                case 2:
                    (cbCamTaps.Items[2] as ComboBoxItem).Visibility = Visibility.Visible;
                    (cbCamTaps.Items[1] as ComboBoxItem).Visibility = Visibility.Visible;
                    break;
            }
        }

        private void CamResolutionSelector_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ComboBox resolutions = sender as ComboBox;
            string selectedResolution = resolutions.SelectedItem as string;

            if ((selectedResolution != null) && (selectedResolution != string.Empty))
            {
                string pattern = @"(\d+)";

                MatchCollection mc = Regex.Matches(selectedResolution, pattern);

                if (4 == mc.Count)
                {
                    Top = Convert.ToInt32(mc[0].ToString());
                    Left = Convert.ToInt32(mc[1].ToString());
                    Bottom = Convert.ToInt32(mc[2].ToString());
                    Right = Convert.ToInt32(mc[3].ToString());
                    if(284 == Top && 704 == Left && 796 == Bottom && 1216 == Right)
                    {
                        cbCamResolutionSelector.ToolTip = "sCMOS CS2100M/C 512x512 centered";
                    }
                    else if (768 == Top && 968 == Left && 1280 == Bottom && 1480 == Right)
                    {
                        cbCamResolutionSelector.ToolTip = "CMOS CS505M/C 512x512 centered";
                    }
                    else if (824 == Top && 1792 == Left && 1336 == Bottom && 2304 == Right)
                    {
                        cbCamResolutionSelector.ToolTip = "CMOS CS895M/C 512x512 centered";
                    }
                    else if (264 == Top && 440 == Left && 776 == Bottom && 952 == Right)
                    {
                        cbCamResolutionSelector.ToolTip = "1501M/C 512x512 centered";
                    }
                    else if (768 == Top && 768 == Left && 1280 == Bottom && 1280 == Right)
                    {
                        cbCamResolutionSelector.ToolTip = "4070M/C 512x512 centered";
                    }
                    else if (980 == Top && 1392 == Left && 1492 == Bottom && 1904 == Right)
                    {
                        cbCamResolutionSelector.ToolTip = "8050M/C 512x512 centered";
                    }
                    else
                    {
                        cbCamResolutionSelector.ToolTip = string.Empty;
                    }
                }
            }
        }

        private void cbCamResolutionSelector_DropDownOpened(object sender, EventArgs e)
        {
            ComboBox resolutions = sender as ComboBox;
            resolutions.SelectedItem = null;
        }

        private void FormattedSlider_DragCompleted(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
        {
            Application.Current.Dispatcher.Invoke((Action)(() =>
            {
                BindingExpression be = ((Slider)sender).GetBindingExpression(Slider.ValueProperty);
                be.UpdateSource();
            }));
            //EnableDevReading = true;
        }

        private void FormattedSlider_DragStarted(object sender, System.Windows.Controls.Primitives.DragStartedEventArgs e)
        {
            //EnableDevReading = false;
        }

        private void FormattedSlider_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            double newVal = Math.Round(((Slider)e.Source).Value);

            if (e.Delta > 0)
            {
                newVal += 1;
            }
            else if (e.Delta < 0)
            {
                newVal -= 1;
            }

            ((Slider)sender).Value = newVal;
            BindingExpression be = ((Slider)sender).GetBindingExpression(Slider.ValueProperty);
            be.UpdateSource();

            e.Handled = true;
        }

        private void GainDiscreteOptions_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            // with discrete gain, "Gain" is used like an index into an array of gain options
            ComboBox discreteGainOptions = sender as ComboBox;
            int selectedGainIndex = discreteGainOptions.SelectedIndex;
            Gain = selectedGainIndex;
        }

        #endregion Methods

        #region Other

        //    public static readonly DependencyProperty $2CommandProperty = DependencyProperty.Register("$2Command",typeof($1),typeof(CameraControlUC));public $1 $2{ get { return ($1)GetValue($2CommandProperty); }set { SetValue($2CommandProperty, value); }}
        //public static readonly DependencyProperty **REPLACE**Property =
        //      DependencyProperty.Register(
        //      "**REPLACE**",
        //      typeof(int),
        //      typeof(CameraControlUC));
        //      public int **REPLACE**
        //      {
        //          get { return (int)GetValue(**REPLACE**Property); }
        //          set { SetValue(**REPLACE**Property, value); }
        //      }
        //public static DependencyProperty **REPLACE**Property =
        //DependencyProperty.RegisterAttached("**REPLACE**",
        //typeof(int),
        //typeof(CameraControlUC),
        //new FrameworkPropertyMetadata(new PropertyChangedCallback(on**REPLACE**Changed)));
        //public int **REPLACE**
        //{
        //    get { return (int)GetValue(**REPLACE**Property); }
        //    set { SetValue(**REPLACE**Property, value); }
        //}
        //public static void on**REPLACE**Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        //{
        //}

        #endregion Other
    }
}