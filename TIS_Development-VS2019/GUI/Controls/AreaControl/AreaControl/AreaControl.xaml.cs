namespace AreaControl
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Globalization;
    using System.IO;
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

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class AreaControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty CenterROICommandProperty = 
        DependencyProperty.Register(
        "CenterROICommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty CenterScannersCommandProperty = 
        DependencyProperty.Register(
        "CenterScannersCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty CloseShutterCommandProperty = 
        DependencyProperty.Register(
        "CloseShutterCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty Coeff1Property = 
           DependencyProperty.Register(
           "Coeff1",
           typeof(double),
           typeof(AreaControlUC));
        public static readonly DependencyProperty Coeff2Property = 
          DependencyProperty.Register(
          "Coeff2",
          typeof(double),
          typeof(AreaControlUC));
        public static readonly DependencyProperty Coeff3Property = 
          DependencyProperty.Register(
          "Coeff3",
          typeof(double),
          typeof(AreaControlUC));
        public static readonly DependencyProperty EnableBackgroundSubtractionProperty = 
         DependencyProperty.Register(
         "EnableBackgroundSubtraction",
         typeof(int),
         typeof(AreaControlUC));
        public static readonly DependencyProperty EnableFlatFieldProperty = 
        DependencyProperty.Register(
        "EnableFlatField",
        typeof(int),
        typeof(AreaControlUC));
        public static readonly DependencyProperty EnablePincushionCorrectionProperty = 
          DependencyProperty.Register(
          "EnablePincushionCorrection",
          typeof(int),
          typeof(AreaControlUC));
        public static readonly DependencyProperty FieldSizeVisibleProperty = 
        DependencyProperty.Register(
        "FieldSizeVisible",
        typeof(Visibility),
        typeof(AreaControlUC));
        public static readonly DependencyProperty GGRegistrationClearAllCommandProperty = 
        DependencyProperty.Register(
        "GGRegistrationClearAllCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty GGRegistrationClearCommandProperty = 
        DependencyProperty.Register(
        "GGRegistrationClearCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMFieldOffsetFineResetCommandProperty = 
        DependencyProperty.Register(
        "LSMFieldOffsetFineResetCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMFieldOffsetXFineMinusCommandProperty = 
        DependencyProperty.Register(
        "LSMFieldOffsetXFineMinusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMFieldOffsetXFinePlusCommandProperty = 
        DependencyProperty.Register(
        "LSMFieldOffsetXFinePlusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMFieldOffsetXMinusCommandProperty = 
        DependencyProperty.Register(
        "LSMFieldOffsetXMinusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMFieldOffsetXPlusCommandProperty = 
        DependencyProperty.Register(
        "LSMFieldOffsetXPlusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMFieldOffsetYFineMinusCommandProperty = 
        DependencyProperty.Register(
        "LSMFieldOffsetYFineMinusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMFieldOffsetYFinePlusCommandProperty = 
        DependencyProperty.Register(
        "LSMFieldOffsetYFinePlusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMFieldOffsetYMinusCommandProperty = 
        DependencyProperty.Register(
        "LSMFieldOffsetYMinusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMFieldOffsetYPlusCommandProperty = 
        DependencyProperty.Register(
        "LSMFieldOffsetYPlusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMFieldScaleFineResetCommandProperty = 
        DependencyProperty.Register(
        "LSMFieldScaleFineResetCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMFieldScaleXFineMinusCommandProperty = 
        DependencyProperty.Register(
        "LSMFieldScaleXFineMinusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMFieldScaleXFinePlusCommandProperty = 
        DependencyProperty.Register(
        "LSMFieldScaleXFinePlusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMFieldScaleYFineMinusCommandProperty = 
        DependencyProperty.Register(
        "LSMFieldScaleYFineMinusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMFieldScaleYFinePlusCommandProperty = 
        DependencyProperty.Register(
        "LSMFieldScaleYFinePlusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMFieldSizeMinusCommandProperty = 
        DependencyProperty.Register(
        "LSMFieldSizeMinusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMFieldSizePlusCommandProperty = 
        DependencyProperty.Register(
        "LSMFieldSizePlusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMSaveCalibrationCommandProperty = 
        DependencyProperty.Register(
        "LSMSaveCalibrationCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMScanAreaAngleMinusCommandProperty = 
        DependencyProperty.Register(
        "LSMScanAreaAngleMinusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMScanAreaAnglePlusCommandProperty = 
        DependencyProperty.Register(
        "LSMScanAreaAnglePlusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMZoomMinusCommandProperty = 
        DependencyProperty.Register(
        "LSMZoomMinusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty LSMZoomPlusCommandProperty = 
        DependencyProperty.Register(
        "LSMZoomPlusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty MesoStripPixelsMinusCommandProperty = 
        DependencyProperty.Register(
        "MesoStripPixelsMinusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty MesoStripPixelsPlusCommandProperty = 
        DependencyProperty.Register(
        "MesoStripPixelsPlusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty NyquistCommandProperty = 
        DependencyProperty.Register(
        "NyquistCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty PathBackgroundSubtractionProperty = 
         DependencyProperty.Register(
         "PathBackgroundSubtraction",
         typeof(string),
         typeof(AreaControlUC));
        public static readonly DependencyProperty PathFlatFieldProperty = 
        DependencyProperty.Register(
        "PathFlatField",
        typeof(string),
        typeof(AreaControlUC));
        public static readonly DependencyProperty ResolutionAddCommandProperty = 
        DependencyProperty.Register(
        "ResolutionAddCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty ReturnToOriginalAreaCommandCommandProperty = 
        DependencyProperty.Register(
        "ReturnToOriginalAreaCommandCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty RoiZoomInCommandProperty = 
        DependencyProperty.Register(
        "RoiZoomInCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty SelectBackgroundCommandProperty = 
         DependencyProperty.Register(
         "SelectBackgroundCommand",
         typeof(ICommand),
         typeof(AreaControlUC));
        public static readonly DependencyProperty SelectFlatFieldCommandProperty = 
        DependencyProperty.Register(
        "SelectFlatFieldCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty TimeBasedLSTimeMSMinusCommandProperty = 
        DependencyProperty.Register(
        "TimeBasedLSTimeMSMinusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static readonly DependencyProperty TimeBasedLSTimeMSPlusCommandProperty = 
        DependencyProperty.Register(
        "TimeBasedLSTimeMSPlusCommand",
        typeof(ICommand),
        typeof(AreaControlUC));

        public static DependencyProperty AreaAngleVisibilityProperty = 
        DependencyProperty.RegisterAttached("AreaAngleVisibility",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onAreaAngleVisibilityChanged)));
        public static DependencyProperty AreaModeProperty = 
        DependencyProperty.RegisterAttached("AreaMode",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onAreaModeChanged)));
        public static DependencyProperty ConfigMicroScanAreaProperty = 
        DependencyProperty.Register(
        "ConfigMicroScanArea",
        typeof(bool),
        typeof(AreaControlUC));
        public static DependencyProperty CurrentResolutionProperty = 
        DependencyProperty.RegisterAttached("CurrentResolution",
        typeof(ImageResolution),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onCurrentResolutionChanged)));
        public static DependencyProperty EnablePixelDensityChangeProperty = 
        DependencyProperty.RegisterAttached("EnablePixelDensityChange",
        typeof(bool),
        typeof(AreaControlUC));
        public static DependencyProperty EnableReferenceChannelProperty = 
        DependencyProperty.RegisterAttached("EnableReferenceChannel",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onEnableReferenceChannelChanged)));
        public static DependencyProperty EnableResolutionPresetsProperty = 
        DependencyProperty.RegisterAttached("EnableResolutionPresets",
        typeof(bool),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onEnableResolutionPresetsChanged)));
        public static DependencyProperty FieldFromROIEnableProperty = 
        DependencyProperty.RegisterAttached("FieldFromROIEnable",
        typeof(bool),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onFieldFromROIEnableChanged)));
        public static DependencyProperty FielSizeAdjustMentEnableProperty = 
        DependencyProperty.RegisterAttached("FielSizeAdjustMentEnable",
        typeof(bool),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onFielSizeAdjustMentEnableChanged)));
        public static DependencyProperty GGLSMScanVisibilityProperty = 
        DependencyProperty.RegisterAttached("GGLSMScanVisibility",
        typeof(Visibility),
        typeof(AreaControlUC));
        public static DependencyProperty GGRegistrationAbleToClearProperty = 
        DependencyProperty.RegisterAttached("GGRegistrationAbleToClear",
        typeof(bool),
        typeof(AreaControlUC));
        public static DependencyProperty GGRegistrationIndexProperty = 
        DependencyProperty.RegisterAttached("GGRegistrationIndex",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onGGRegistrationIndexChanged)));
        public static DependencyProperty GGRegistrationItemsProperty = 
        DependencyProperty.RegisterAttached("GGRegistrationItems",
        typeof(IList),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onGGRegistrationItemsChanged)));
        public static DependencyProperty GRLSMScanVisibilityProperty = 
        DependencyProperty.RegisterAttached("GRLSMScanVisibility",
        typeof(Visibility),
        typeof(AreaControlUC));
        public static DependencyProperty ImageStartStatusAreaProperty = 
        DependencyProperty.RegisterAttached("ImageStartStatusArea",
        typeof(bool),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onImageStartStatusAreaChanged)));
        public static DependencyProperty LockFieldOffsetProperty = 
        DependencyProperty.RegisterAttached("LockFieldOffset",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLockFieldOffsetChanged)));
        public static DependencyProperty LSMAreaModeProperty = 
        DependencyProperty.RegisterAttached("LSMAreaMode",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMAreaModeChanged)));
        public static DependencyProperty LSMFieldOffsetXActualProperty = 
        DependencyProperty.RegisterAttached("LSMFieldOffsetXActual",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFieldOffsetXActualChanged)));
        public static DependencyProperty LSMFieldOffsetXDisplayProperty = 
        DependencyProperty.RegisterAttached("LSMFieldOffsetXDisplay",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFieldOffsetXDisplayChanged)));
        public static DependencyProperty LSMFieldOffsetXFineProperty = 
        DependencyProperty.RegisterAttached("LSMFieldOffsetXFine",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFieldOffsetXFineChanged)));
        public static DependencyProperty LSMFieldOffsetXProperty = 
        DependencyProperty.RegisterAttached("LSMFieldOffsetX",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFieldOffsetXChanged)));
        public static DependencyProperty LSMFieldOffsetYActualProperty = 
        DependencyProperty.RegisterAttached("LSMFieldOffsetYActual",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFieldOffsetYActualChanged)));
        public static DependencyProperty LSMFieldOffsetYDisplayProperty = 
        DependencyProperty.RegisterAttached("LSMFieldOffsetYDisplay",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFieldOffsetYDisplayChanged)));
        public static DependencyProperty LSMFieldOffsetYFineProperty = 
        DependencyProperty.RegisterAttached("LSMFieldOffsetYFine",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFieldOffsetYFineChanged)));
        public static DependencyProperty LSMFieldOffsetYProperty = 
        DependencyProperty.RegisterAttached("LSMFieldOffsetY",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFieldOffsetYChanged)));
        public static DependencyProperty LSMFieldScaleXFineProperty = 
        DependencyProperty.RegisterAttached("LSMFieldScaleXFine",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFieldScaleXFineChanged)));
        public static DependencyProperty LSMFieldScaleYFineProperty = 
        DependencyProperty.RegisterAttached("LSMFieldScaleYFine",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFieldScaleYFineChanged)));
        public static DependencyProperty LSMFieldSizeDisplayXProperty = 
        DependencyProperty.RegisterAttached("LSMFieldSizeDisplayX",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFieldSizeDisplayXChanged)));
        public static DependencyProperty LSMFieldSizeDisplayYProperty = 
        DependencyProperty.RegisterAttached("LSMFieldSizeDisplayY",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFieldSizeDisplayYChanged)));
        public static DependencyProperty LSMFieldSizeMaxProperty = 
        DependencyProperty.RegisterAttached("LSMFieldSizeMax",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFieldSizeMaxChanged)));
        public static DependencyProperty LSMFieldSizeMinProperty = 
        DependencyProperty.RegisterAttached("LSMFieldSizeMin",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFieldSizeMinChanged)));
        public static DependencyProperty LSMFieldSizeProperty = 
        DependencyProperty.RegisterAttached("LSMFieldSize",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFieldSizeChanged)));
        public static DependencyProperty LSMFieldSizeXUMProperty = 
        DependencyProperty.RegisterAttached("LSMFieldSizeXUM",
        typeof(double),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFieldSizeXUMChanged)));
        public static DependencyProperty LSMFieldSizeYUMProperty = 
        DependencyProperty.RegisterAttached("LSMFieldSizeYUM",
        typeof(double),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFieldSizeYUMChanged)));
        public static DependencyProperty LSMFlipHorizontalProperty = 
        DependencyProperty.RegisterAttached("LSMFlipHorizontal",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFlipHorizontalChanged)));
        public static DependencyProperty LSMFlipVerticalScanProperty = 
        DependencyProperty.RegisterAttached("LSMFlipVerticalScan",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMFlipVerticalScanChanged)));
        public static DependencyProperty LSMLastCalibrationDateProperty = 
        DependencyProperty.RegisterAttached("LSMLastCalibrationDate",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMLastCalibrationDateChanged)));
        public static DependencyProperty LSMPixelXMaxProperty = 
        DependencyProperty.RegisterAttached("LSMPixelXMax",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMPixelXMaxChanged)));
        public static DependencyProperty LSMPixelXMinProperty = 
        DependencyProperty.RegisterAttached("LSMPixelXMin",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMPixelXMinChanged)));
        public static DependencyProperty LSMPixelXProperty = 
        DependencyProperty.RegisterAttached("LSMPixelX",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMPixelXChanged)));
        public static DependencyProperty LSMPixelYMaxProperty = 
        DependencyProperty.RegisterAttached("LSMPixelYMax",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMPixelYMaxChanged)));
        public static DependencyProperty LSMPixelYMinProperty = 
        DependencyProperty.RegisterAttached("LSMPixelYMin",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMPixelYMinChanged)));
        public static DependencyProperty LSMPixelYMultipleProperty = 
        DependencyProperty.RegisterAttached("LSMPixelYMultiple",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMPixelYMultiplehanged)));
        public static DependencyProperty LSMPixelYProperty = 
        DependencyProperty.RegisterAttached("LSMPixelY",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMPixelYChanged)));
        public static DependencyProperty LSMScaleYScanProperty = 
        DependencyProperty.RegisterAttached("LSMScaleYScan",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMScaleYScanChanged)));
        public static DependencyProperty LSMScanAreaAngleProperty = 
        DependencyProperty.RegisterAttached("LSMScanAreaAngle",
        typeof(double),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMScanAreaAngleChanged)));
        public static DependencyProperty LSMUMPerPixelProperty = 
        DependencyProperty.RegisterAttached("LSMUMPerPixel",
        typeof(double),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMUMPerPixelChanged)));
        public static DependencyProperty LSMZoomProperty = 
        DependencyProperty.RegisterAttached("LSMZoom",
        typeof(string),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onLSMZoomChanged)));
        public static DependencyProperty MesoMicroVisibleProperty = 
        DependencyProperty.RegisterAttached("MesoMicroVisible",
        typeof(int),
        typeof(AreaControlUC));
        public static DependencyProperty MesoStripPixelsProperty = 
        DependencyProperty.RegisterAttached("MesoStripPixels",
        typeof(long),
        typeof(AreaControlUC));
        public static DependencyProperty MesoStripPixelsRangeProperty = 
        DependencyProperty.RegisterAttached("MesoStripPixelsRange",
        typeof(int[]),
        typeof(AreaControlUC));
        public static DependencyProperty MesoStripPixelsStepProperty = 
        DependencyProperty.RegisterAttached("MesoStripPixelsStep",
        typeof(int),
        typeof(AreaControlUC));
        public static DependencyProperty MicroScanAreasProperty = 
        DependencyProperty.RegisterAttached("MicroScanAreas",
        typeof(IList),
        typeof(AreaControlUC));
        public static DependencyProperty OverviewVisibleProperty = 
        DependencyProperty.Register(
        "OverviewVisible",
        typeof(bool),
        typeof(AreaControlUC));
        public static DependencyProperty PixelXSliderVibilityProperty = 
        DependencyProperty.RegisterAttached("PixelXSliderVibility",
        typeof(Visibility),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPixelXSliderVibilityChanged)));
        public static DependencyProperty PixelYSliderVibilityProperty = 
        DependencyProperty.RegisterAttached("PixelYSliderVibility",
        typeof(Visibility),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPixelYSliderVibilityChanged)));
        public static DependencyProperty PolylineScanVisibilityProperty = 
        DependencyProperty.RegisterAttached("PolylineScanVisibility",
        typeof(Visibility),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onPolylineScanVisibilityChanged)));
        public static DependencyProperty RectangleAreaModeVisibilityProperty = 
        DependencyProperty.RegisterAttached("RectangleAreaModeVisibility",
        typeof(Visibility),
        typeof(AreaControlUC));
        public static DependencyProperty ResolutionPresetsProperty = 
        DependencyProperty.RegisterAttached("ResolutionPresets",
        typeof(IList),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onResolutionPresetsChanged)));
        public static DependencyProperty ROIFrameRateProperty = 
        DependencyProperty.RegisterAttached("ROIFrameRate",
        typeof(string),
        typeof(AreaControlUC));
        public static DependencyProperty RSInitModeProperty = 
        DependencyProperty.RegisterAttached("RSInitMode",
        typeof(int),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onRSInitModeChanged)));
        public static DependencyProperty RSLineProbeOnProperty = 
        DependencyProperty.Register(
        "RSLineProbeOn",
        typeof(bool),
        typeof(AreaControlUC));
        public static DependencyProperty RSLineRateProperty = 
        DependencyProperty.Register(
        "RSLineRate",
        typeof(double),
        typeof(AreaControlUC));
        public static DependencyProperty RSLineVisibleProperty = 
        DependencyProperty.Register(
        "RSLineVisible",
        typeof(bool),
        typeof(AreaControlUC));
        public static DependencyProperty SelectedScanAreaProperty = 
        DependencyProperty.RegisterAttached("SelectedScanArea",
        typeof(int),
        typeof(AreaControlUC));
        public static DependencyProperty SelectedStripSizeProperty = 
        DependencyProperty.RegisterAttached("SelectedStripSize",
        typeof(Decimal),
        typeof(AreaControlUC));
        public static DependencyProperty SelectedViewModeProperty = 
        DependencyProperty.RegisterAttached("SelectedViewMode",
        typeof(int),
        typeof(AreaControlUC));
        public static DependencyProperty StoreRSRateCommandProperty = 
        DependencyProperty.Register(
        "StoreRSRateCommand",
        typeof(ICommand),
        typeof(AreaControlUC));
        public static DependencyProperty StripSizesProperty = 
        DependencyProperty.RegisterAttached("StripSizes",
        typeof(IList),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onStripSizesChanged)));
        public static DependencyProperty StripVisibleProperty = 
        DependencyProperty.RegisterAttached("StripVisible",
        typeof(bool),
        typeof(AreaControlUC));
        public static DependencyProperty TimeBasedLineScanProperty = 
        DependencyProperty.RegisterAttached("TimeBasedLineScan",
        typeof(bool),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onTimeBasedLineScanChanged)));
        public static DependencyProperty TimeBasedLSTimeMSProperty = 
        DependencyProperty.RegisterAttached("TimeBasedLSTimeMS",
        typeof(double),
        typeof(AreaControlUC),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onTimeBasedLSTimeMSChanged)));
        public static DependencyProperty TimedBasedVisiblityProperty = 
        DependencyProperty.RegisterAttached("TimedBasedVisiblity",
        typeof(Visibility),
        typeof(AreaControlUC));

        #endregion Fields

        #region Constructors

        public AreaControlUC()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public int AreaAngleVisibility
        {
            get { return (int)GetValue(AreaAngleVisibilityProperty); }
            set { SetValue(AreaAngleVisibilityProperty, value); }
        }

        public int AreaMode
        {
            get { return (int)GetValue(AreaModeProperty); }
            set { SetValue(AreaModeProperty, value); }
        }

        public ICommand CenterROICommand
        {
            get { return (ICommand)GetValue(CenterROICommandProperty); }
            set { SetValue(CenterROICommandProperty, value); }
        }

        public ICommand CenterScannersCommand
        {
            get { return (ICommand)GetValue(CenterScannersCommandProperty); }
            set { SetValue(CenterScannersCommandProperty, value); }
        }

        public ICommand CloseShutterCommand
        {
            get { return (ICommand)GetValue(CloseShutterCommandProperty); }
            set { SetValue(CloseShutterCommandProperty, value); }
        }

        public double Coeff1
        {
            get { return (double)GetValue(Coeff1Property); }
            set { SetValue(Coeff1Property, value); }
        }

        public double Coeff2
        {
            get { return (double)GetValue(Coeff2Property); }
            set { SetValue(Coeff2Property, value); }
        }

        public double Coeff3
        {
            get { return (double)GetValue(Coeff3Property); }
            set { SetValue(Coeff3Property, value); }
        }

        public bool ConfigMicroScanArea
        {
            get { return (bool)GetValue(ConfigMicroScanAreaProperty); }
            set { SetValue(ConfigMicroScanAreaProperty, value); }
        }

        public ImageResolution CurrentResolution
        {
            get { return (ImageResolution)GetValue(CurrentResolutionProperty); }
            set { SetValue(CurrentResolutionProperty, value); }
        }

        public int EnableBackgroundSubtraction
        {
            get { return (int)GetValue(EnableBackgroundSubtractionProperty); }
            set { SetValue(EnableBackgroundSubtractionProperty, value); }
        }

        public int EnableFlatField
        {
            get { return (int)GetValue(EnableFlatFieldProperty); }
            set { SetValue(EnableFlatFieldProperty, value); }
        }

        public int EnablePincushionCorrection
        {
            get { return (int)GetValue(EnablePincushionCorrectionProperty); }
            set { SetValue(EnablePincushionCorrectionProperty, value); }
        }

        public bool EnablePixelDensityChange
        {
            get { return (bool)GetValue(EnablePixelDensityChangeProperty); }
            set { SetValue(EnablePixelDensityChangeProperty, value); }
        }

        public int EnableReferenceChannel
        {
            get { return (int)GetValue(EnableReferenceChannelProperty); }
            set { SetValue(EnableReferenceChannelProperty, value); }
        }

        public bool EnableResolutionPresets
        {
            get { return (bool)GetValue(EnableResolutionPresetsProperty); }
            set { SetValue(EnableResolutionPresetsProperty, value); }
        }

        public bool FieldFromROIEnable
        {
            get { return (bool)GetValue(FieldFromROIEnableProperty); }
            set { SetValue(FieldFromROIEnableProperty, value); }
        }

        public Visibility FieldSizeVisible
        {
            get { return (Visibility)GetValue(FieldSizeVisibleProperty); }
            set { SetValue(FieldSizeVisibleProperty, value); }
        }

        public bool FielSizeAdjustMentEnable
        {
            get { return (bool)GetValue(FielSizeAdjustMentEnableProperty); }
            set { SetValue(FielSizeAdjustMentEnableProperty, value); }
        }

        public Visibility GGLSMScanVisibility
        {
            get { return (Visibility)GetValue(GGLSMScanVisibilityProperty); }
            set { SetValue(GGLSMScanVisibilityProperty, value); }
        }

        public bool GGRegistrationAbleToClear
        {
            get { return (bool)GetValue(GGRegistrationAbleToClearProperty); }
            set { SetValue(GGRegistrationAbleToClearProperty, value); }
        }

        public ICommand GGRegistrationClearAllCommand
        {
            get { return (ICommand)GetValue(GGRegistrationClearAllCommandProperty); }
            set { SetValue(GGRegistrationClearAllCommandProperty, value); }
        }

        public ICommand GGRegistrationClearCommand
        {
            get { return (ICommand)GetValue(GGRegistrationClearCommandProperty); }
            set { SetValue(GGRegistrationClearCommandProperty, value); }
        }

        public int GGRegistrationIndex
        {
            get { return (int)GetValue(GGRegistrationIndexProperty); }
            set { SetValue(GGRegistrationIndexProperty, value); }
        }

        public IList GGRegistrationItems
        {
            get { return (IList)GetValue(GGRegistrationItemsProperty); }
            set { SetValue(GGRegistrationItemsProperty, value); }
        }

        public Visibility GRLSMScanVisibility
        {
            get { return (Visibility)GetValue(GRLSMScanVisibilityProperty); }
            set { SetValue(GRLSMScanVisibilityProperty, value); }
        }

        public bool ImageStartStatusArea
        {
            get { return (bool)GetValue(ImageStartStatusAreaProperty); }
            set { SetValue(ImageStartStatusAreaProperty, value); }
        }

        public int LockFieldOffset
        {
            get { return (int)GetValue(LockFieldOffsetProperty); }
            set { SetValue(LockFieldOffsetProperty, value); }
        }

        public int LSMAreaMode
        {
            get { return (int)GetValue(LSMAreaModeProperty); }
            set { SetValue(LSMAreaModeProperty, value); }
        }

        public ICommand LSMFieldOffsetFineResetCommand
        {
            get { return (ICommand)GetValue(LSMFieldOffsetFineResetCommandProperty); }
            set { SetValue(LSMFieldOffsetFineResetCommandProperty, value); }
        }

        public int LSMFieldOffsetX
        {
            get { return (int)GetValue(LSMFieldOffsetXProperty); }
            set { SetValue(LSMFieldOffsetXProperty, value); }
        }

        public int LSMFieldOffsetXActual
        {
            get { return (int)GetValue(LSMFieldOffsetXActualProperty); }
            set { SetValue(LSMFieldOffsetXActualProperty, value); }
        }

        public int LSMFieldOffsetXDisplay
        {
            get { return (int)GetValue(LSMFieldOffsetXDisplayProperty); }
            set { SetValue(LSMFieldOffsetXDisplayProperty, value); }
        }

        public int LSMFieldOffsetXFine
        {
            get { return (int)GetValue(LSMFieldOffsetXFineProperty); }
            set { SetValue(LSMFieldOffsetXFineProperty, value); }
        }

        public ICommand LSMFieldOffsetXFineMinusCommand
        {
            get { return (ICommand)GetValue(LSMFieldOffsetXFineMinusCommandProperty); }
            set { SetValue(LSMFieldOffsetXFineMinusCommandProperty, value); }
        }

        public ICommand LSMFieldOffsetXFinePlusCommand
        {
            get { return (ICommand)GetValue(LSMFieldOffsetXFinePlusCommandProperty); }
            set { SetValue(LSMFieldOffsetXFinePlusCommandProperty, value); }
        }

        public ICommand LSMFieldOffsetXMinusCommand
        {
            get { return (ICommand)GetValue(LSMFieldOffsetXMinusCommandProperty); }
            set { SetValue(LSMFieldOffsetXMinusCommandProperty, value); }
        }

        public ICommand LSMFieldOffsetXPlusCommand
        {
            get { return (ICommand)GetValue(LSMFieldOffsetXPlusCommandProperty); }
            set { SetValue(LSMFieldOffsetXPlusCommandProperty, value); }
        }

        public int LSMFieldOffsetY
        {
            get { return (int)GetValue(LSMFieldOffsetYProperty); }
            set { SetValue(LSMFieldOffsetYProperty, value); }
        }

        public int LSMFieldOffsetYActual
        {
            get { return (int)GetValue(LSMFieldOffsetYActualProperty); }
            set { SetValue(LSMFieldOffsetYActualProperty, value); }
        }

        public int LSMFieldOffsetYDisplay
        {
            get { return (int)GetValue(LSMFieldOffsetYDisplayProperty); }
            set { SetValue(LSMFieldOffsetYDisplayProperty, value); }
        }

        public int LSMFieldOffsetYFine
        {
            get { return (int)GetValue(LSMFieldOffsetYFineProperty); }
            set { SetValue(LSMFieldOffsetYFineProperty, value); }
        }

        public ICommand LSMFieldOffsetYFineMinusCommand
        {
            get { return (ICommand)GetValue(LSMFieldOffsetYFineMinusCommandProperty); }
            set { SetValue(LSMFieldOffsetYFineMinusCommandProperty, value); }
        }

        public ICommand LSMFieldOffsetYFinePlusCommand
        {
            get { return (ICommand)GetValue(LSMFieldOffsetYFinePlusCommandProperty); }
            set { SetValue(LSMFieldOffsetYFinePlusCommandProperty, value); }
        }

        public ICommand LSMFieldOffsetYMinusCommand
        {
            get { return (ICommand)GetValue(LSMFieldOffsetYMinusCommandProperty); }
            set { SetValue(LSMFieldOffsetYMinusCommandProperty, value); }
        }

        public ICommand LSMFieldOffsetYPlusCommand
        {
            get { return (ICommand)GetValue(LSMFieldOffsetYPlusCommandProperty); }
            set { SetValue(LSMFieldOffsetYPlusCommandProperty, value); }
        }

        public ICommand LSMFieldScaleFineResetCommand
        {
            get { return (ICommand)GetValue(LSMFieldScaleFineResetCommandProperty); }
            set { SetValue(LSMFieldScaleFineResetCommandProperty, value); }
        }

        public int LSMFieldScaleXFine
        {
            get { return (int)GetValue(LSMFieldScaleXFineProperty); }
            set { SetValue(LSMFieldScaleXFineProperty, value); }
        }

        public ICommand LSMFieldScaleXFineMinusCommand
        {
            get { return (ICommand)GetValue(LSMFieldScaleXFineMinusCommandProperty); }
            set { SetValue(LSMFieldScaleXFineMinusCommandProperty, value); }
        }

        public ICommand LSMFieldScaleXFinePlusCommand
        {
            get { return (ICommand)GetValue(LSMFieldScaleXFinePlusCommandProperty); }
            set { SetValue(LSMFieldScaleXFinePlusCommandProperty, value); }
        }

        public int LSMFieldScaleYFine
        {
            get { return (int)GetValue(LSMFieldScaleYFineProperty); }
            set { SetValue(LSMFieldScaleYFineProperty, value); }
        }

        public ICommand LSMFieldScaleYFineMinusCommand
        {
            get { return (ICommand)GetValue(LSMFieldScaleYFineMinusCommandProperty); }
            set { SetValue(LSMFieldScaleYFineMinusCommandProperty, value); }
        }

        public ICommand LSMFieldScaleYFinePlusCommand
        {
            get { return (ICommand)GetValue(LSMFieldScaleYFinePlusCommandProperty); }
            set { SetValue(LSMFieldScaleYFinePlusCommandProperty, value); }
        }

        public int LSMFieldSize
        {
            get { return (int)GetValue(LSMFieldSizeProperty); }
            set { SetValue(LSMFieldSizeProperty, value); }
        }

        public int LSMFieldSizeDisplayX
        {
            get { return (int)GetValue(LSMFieldSizeDisplayXProperty); }
            set { SetValue(LSMFieldSizeDisplayXProperty, value); }
        }

        public int LSMFieldSizeDisplayY
        {
            get { return (int)GetValue(LSMFieldSizeDisplayYProperty); }
            set { SetValue(LSMFieldSizeDisplayYProperty, value); }
        }

        public int LSMFieldSizeMax
        {
            get { return (int)GetValue(LSMFieldSizeMaxProperty); }
            set { SetValue(LSMFieldSizeMaxProperty, value); }
        }

        public int LSMFieldSizeMin
        {
            get { return (int)GetValue(LSMFieldSizeMinProperty); }
            set { SetValue(LSMFieldSizeMinProperty, value); }
        }

        public ICommand LSMFieldSizeMinusCommand
        {
            get { return (ICommand)GetValue(LSMFieldSizeMinusCommandProperty); }
            set { SetValue(LSMFieldSizeMinusCommandProperty, value); }
        }

        public ICommand LSMFieldSizePlusCommand
        {
            get { return (ICommand)GetValue(LSMFieldSizePlusCommandProperty); }
            set { SetValue(LSMFieldSizePlusCommandProperty, value); }
        }

        public double LSMFieldSizeXUM
        {
            get { return (double)GetValue(LSMFieldSizeXUMProperty); }
            set { SetValue(LSMFieldSizeXUMProperty, value); }
        }

        public double LSMFieldSizeYUM
        {
            get { return (double)GetValue(LSMFieldSizeYUMProperty); }
            set { SetValue(LSMFieldSizeYUMProperty, value); }
        }

        public int LSMFlipHorizontal
        {
            get { return (int)GetValue(LSMFlipHorizontalProperty); }
            set { SetValue(LSMFlipHorizontalProperty, value); }
        }

        public int LSMFlipVerticalScan
        {
            get { return (int)GetValue(LSMFlipVerticalScanProperty); }
            set { SetValue(LSMFlipVerticalScanProperty, value); }
        }

        public int LSMLastCalibrationDate
        {
            get { return (int)GetValue(LSMLastCalibrationDateProperty); }
            set { SetValue(LSMLastCalibrationDateProperty, value); }
        }

        public int LSMPixelX
        {
            get { return (int)GetValue(LSMPixelXProperty); }
            set { SetValue(LSMPixelXProperty, value); }
        }

        public int LSMPixelXMax
        {
            get { return (int)GetValue(LSMPixelXMaxProperty); }
            set { SetValue(LSMPixelXMaxProperty, value); }
        }

        public int LSMPixelXMin
        {
            get { return (int)GetValue(LSMPixelXMinProperty); }
            set { SetValue(LSMPixelXMinProperty, value); }
        }

        public int LSMPixelY
        {
            get { return (int)GetValue(LSMPixelYProperty); }
            set { SetValue(LSMPixelYProperty, value); }
        }

        public int LSMPixelYMax
        {
            get { return (int)GetValue(LSMPixelYMaxProperty); }
            set { SetValue(LSMPixelYMaxProperty, value); }
        }

        public int LSMPixelYMin
        {
            get { return (int)GetValue(LSMPixelYMinProperty); }
            set { SetValue(LSMPixelYMinProperty, value); }
        }

        public int LSMPixelYMultiple
        {
            get { return (int)GetValue(LSMPixelYMultipleProperty); }
            set { SetValue(LSMPixelYMultipleProperty, value); }
        }

        public ICommand LSMSaveCalibrationCommand
        {
            get { return (ICommand)GetValue(LSMSaveCalibrationCommandProperty); }
            set { SetValue(LSMSaveCalibrationCommandProperty, value); }
        }

        public int LSMScaleYScan
        {
            get { return (int)GetValue(LSMScaleYScanProperty); }
            set { SetValue(LSMScaleYScanProperty, value); }
        }

        public double LSMScanAreaAngle
        {
            get { return (double)GetValue(LSMScanAreaAngleProperty); }
            set { SetValue(LSMScanAreaAngleProperty, value); }
        }

        public ICommand LSMScanAreaAngleMinusCommand
        {
            get { return (ICommand)GetValue(LSMScanAreaAngleMinusCommandProperty); }
            set { SetValue(LSMScanAreaAngleMinusCommandProperty, value); }
        }

        public ICommand LSMScanAreaAnglePlusCommand
        {
            get { return (ICommand)GetValue(LSMScanAreaAnglePlusCommandProperty); }
            set { SetValue(LSMScanAreaAnglePlusCommandProperty, value); }
        }

        public double LSMUMPerPixel
        {
            get { return (double)GetValue(LSMUMPerPixelProperty); }
            set { SetValue(LSMUMPerPixelProperty, value); }
        }

        public string LSMZoom
        {
            get { return (string)GetValue(LSMZoomProperty); }
            set { SetValue(LSMZoomProperty, value); }
        }

        public ICommand LSMZoomMinusCommand
        {
            get { return (ICommand)GetValue(LSMZoomMinusCommandProperty); }
            set { SetValue(LSMZoomMinusCommandProperty, value); }
        }

        public ICommand LSMZoomPlusCommand
        {
            get { return (ICommand)GetValue(LSMZoomPlusCommandProperty); }
            set { SetValue(LSMZoomPlusCommandProperty, value); }
        }

        public int MesoMicroVisible
        {
            get { return (int)GetValue(MesoMicroVisibleProperty); }
            set { SetValue(MesoMicroVisibleProperty, value); }
        }

        public long MesoStripPixels
        {
            get { return (long)GetValue(MesoStripPixelsProperty); }
            set { SetValue(MesoStripPixelsProperty, value); }
        }

        public ICommand MesoStripPixelsMinusCommand
        {
            get { return (ICommand)GetValue(MesoStripPixelsMinusCommandProperty); }
            set { SetValue(MesoStripPixelsMinusCommandProperty, value); }
        }

        public ICommand MesoStripPixelsPlusCommand
        {
            get { return (ICommand)GetValue(MesoStripPixelsPlusCommandProperty); }
            set { SetValue(MesoStripPixelsPlusCommandProperty, value); }
        }

        public int[] MesoStripPixelsRange
        {
            get { return (int[])GetValue(MesoStripPixelsRangeProperty); }
            set { SetValue(MesoStripPixelsRangeProperty, value); }
        }

        public int MesoStripPixelsStep
        {
            get { return (int)GetValue(MesoStripPixelsStepProperty); }
            set { SetValue(MesoStripPixelsStepProperty, value); }
        }

        public IList MicroScanAreas
        {
            get { return (IList)GetValue(MicroScanAreasProperty); }
            set { SetValue(MicroScanAreasProperty, value); }
        }

        public ICommand NyquistCommand
        {
            get { return (ICommand)GetValue(NyquistCommandProperty); }
            set { SetValue(NyquistCommandProperty, value); }
        }

        public bool OverviewVisible
        {
            get { return (bool)GetValue(OverviewVisibleProperty); }
            set { SetValue(OverviewVisibleProperty, value); }
        }

        public string PathBackgroundSubtraction
        {
            get { return (string)GetValue(PathBackgroundSubtractionProperty); }
            set { SetValue(PathBackgroundSubtractionProperty, value); }
        }

        public string PathFlatField
        {
            get { return (string)GetValue(PathFlatFieldProperty); }
            set { SetValue(PathFlatFieldProperty, value); }
        }

        public Visibility PixelXSliderVibility
        {
            get { return (Visibility)GetValue(PixelXSliderVibilityProperty); }
            set { SetValue(PixelXSliderVibilityProperty, value); }
        }

        public Visibility PixelYSliderVibility
        {
            get { return (Visibility)GetValue(PixelYSliderVibilityProperty); }
            set { SetValue(PixelYSliderVibilityProperty, value); }
        }

        public Visibility PolylineScanVisibility
        {
            get { return (Visibility)GetValue(PolylineScanVisibilityProperty); }
            set { SetValue(PolylineScanVisibilityProperty, value); }
        }

        public Visibility RectangleAreaModeVisibility
        {
            get { return (Visibility)GetValue(RectangleAreaModeVisibilityProperty); }
            set { SetValue(RectangleAreaModeVisibilityProperty, value); }
        }

        public ICommand ResolutionAddCommand
        {
            get { return (ICommand)GetValue(ResolutionAddCommandProperty); }
            set { SetValue(ResolutionAddCommandProperty, value); }
        }

        public IList ResolutionPresets
        {
            get { return (IList)GetValue(ResolutionPresetsProperty); }
            set { SetValue(ResolutionPresetsProperty, value); }
        }

        public ICommand ReturnToOriginalAreaCommandCommand
        {
            get { return (ICommand)GetValue(ReturnToOriginalAreaCommandCommandProperty); }
            set { SetValue(ReturnToOriginalAreaCommandCommandProperty, value); }
        }

        public string ROIFrameRate
        {
            get { return (string)GetValue(ROIFrameRateProperty); }
            set { SetValue(ROIFrameRateProperty, value); }
        }

        public ICommand RoiZoomInCommand
        {
            get { return (ICommand)GetValue(RoiZoomInCommandProperty); }
            set { SetValue(RoiZoomInCommandProperty, value); }
        }

        public int RSInitMode
        {
            get { return (int)GetValue(RSInitModeProperty); }
            set { SetValue(RSInitModeProperty, value); }
        }

        public bool RSLineProbeOn
        {
            get { return (bool)GetValue(RSLineProbeOnProperty); }
            set { SetValue(RSLineProbeOnProperty, value); }
        }

        public double RSLineRate
        {
            get { return (double)GetValue(RSLineRateProperty); }
            set { SetValue(RSLineRateProperty, value); }
        }

        public bool RSLineVisible
        {
            get { return (bool)GetValue(RSLineVisibleProperty); }
            set { SetValue(RSLineVisibleProperty, value); }
        }

        public ICommand SelectBackgroundCommand
        {
            get { return (ICommand)GetValue(SelectBackgroundCommandProperty); }
            set { SetValue(SelectBackgroundCommandProperty, value); }
        }

        public int SelectedScanArea
        {
            get { return (int)GetValue(SelectedScanAreaProperty); }
            set { SetValue(SelectedScanAreaProperty, value); }
        }

        public Decimal SelectedStripSize
        {
            get { return (Decimal)GetValue(SelectedStripSizeProperty); }
            set { SetValue(SelectedStripSizeProperty, value); }
        }

        public int SelectedViewMode
        {
            get { return (int)GetValue(SelectedViewModeProperty); }
            set { SetValue(SelectedViewModeProperty, value); }
        }

        public ICommand SelectFlatFieldCommand
        {
            get { return (ICommand)GetValue(SelectFlatFieldCommandProperty); }
            set { SetValue(SelectFlatFieldCommandProperty, value); }
        }

        public ICommand StoreRSRateCommand
        {
            get { return (ICommand)GetValue(StoreRSRateCommandProperty); }
            set { SetValue(StoreRSRateCommandProperty, value); }
        }

        public IList StripSizes
        {
            get { return (IList)GetValue(StripSizesProperty); }
            set { SetValue(StripSizesProperty, value); }
        }

        public bool StripVisible
        {
            get { return (bool)GetValue(StripVisibleProperty); }
            set { SetValue(StripVisibleProperty, value); }
        }

        public bool TimeBasedLineScan
        {
            get { return (bool)GetValue(TimeBasedLineScanProperty); }
            set { SetValue(TimeBasedLineScanProperty, value); }
        }

        public double TimeBasedLSTimeMS
        {
            get { return (double)GetValue(TimeBasedLSTimeMSProperty); }
            set { SetValue(TimeBasedLSTimeMSProperty, value); }
        }

        public ICommand TimeBasedLSTimeMSMinusCommand
        {
            get { return (ICommand)GetValue(TimeBasedLSTimeMSMinusCommandProperty); }
            set { SetValue(TimeBasedLSTimeMSMinusCommandProperty, value); }
        }

        public ICommand TimeBasedLSTimeMSPlusCommand
        {
            get { return (ICommand)GetValue(TimeBasedLSTimeMSPlusCommandProperty); }
            set { SetValue(TimeBasedLSTimeMSPlusCommandProperty, value); }
        }

        public Visibility TimedBasedVisiblity
        {
            get { return (Visibility)GetValue(TimedBasedVisiblityProperty); }
            set { SetValue(TimedBasedVisiblityProperty, value); }
        }

        #endregion Properties

        #region Methods

        public static void onAreaAngleVisibilityChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onAreaModeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onCurrentResolutionChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onEnableReferenceChannelChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onEnableResolutionPresetsChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onFieldFromROIEnableChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onFielSizeAdjustMentEnableChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onImageStartStatusAreaChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLockFieldOffsetChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMAreaModeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFieldOffsetXActualChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFieldOffsetXChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFieldOffsetXDisplayChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFieldOffsetXFineChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFieldOffsetYActualChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFieldOffsetYChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFieldOffsetYDisplayChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFieldOffsetYFineChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFieldScaleXFineChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFieldScaleYFineChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFieldSizeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFieldSizeDisplayXChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFieldSizeDisplayYChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFieldSizeMaxChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFieldSizeMinChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFieldSizeXUMChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFieldSizeYUMChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFlipHorizontalChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMFlipVerticalScanChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMLastCalibrationDateChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMPixelXChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMPixelXMaxChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMPixelXMinChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMPixelYChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMPixelYMaxChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMPixelYMinChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMPixelYMultiplehanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMScaleYScanChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMScanAreaAngleChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMUMPerPixelChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onLSMZoomChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPixelXSliderVibilityChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPixelYSliderVibilityChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onPolylineScanVisibilityChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onResolutionPresetsChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onRSInitModeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onStripSizesChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onTimeBasedLineScanChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        public static void onTimeBasedLSTimeMSChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        private static void onGGRegistrationIndexChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        private static void onGGRegistrationItemsChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        private void btnMesoView_Click(object sender, RoutedEventArgs e)
        {
            SelectedViewMode = 0;
        }

        private void btnMicroView_Click(object sender, RoutedEventArgs e)
        {
            SelectedViewMode = ((int)MesoScanTypes.Micro - (int)MesoScanTypes.Meso);
        }

        private void cbEnablePixelDensityChange_Checked(object sender, RoutedEventArgs e)
        {
            sliderPixelX.IsEnabled = true;
            sliderPixelY.IsEnabled = true;
            ResolutionSelector.IsEnabled = true;
            lbAreaMode.IsEnabled = true;
        }

        private void cbEnablePixelDensityChange_Unchecked(object sender, RoutedEventArgs e)
        {
            sliderPixelX.IsEnabled = false;
            sliderPixelY.IsEnabled = false;
            ResolutionSelector.IsEnabled = false;
            lbAreaMode.IsEnabled = false;
        }

        //public static DependencyProperty **REPLACE**Property =
        //DependencyProperty.RegisterAttached("**REPLACE**",
        //typeof(int),
        //typeof(AreaControlUC),
        //new FrameworkPropertyMetadata(new PropertyChangedCallback(on**REPLACE**Changed)));
        //public int **REPLACE**
        //{
        //    get { return (int)GetValue(**REPLACE**Property); }
        //    set { SetValue(**REPLACE**Property, value); }
        //}
        //public static void on**REPLACE**Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        //{
        //}
        //public static DependencyProperty **REPLACE**Property =
        //DependencyProperty.RegisterAttached("**REPLACE**",
        //typeof(int),
        //typeof(AreaControlUC),
        //new FrameworkPropertyMetadata(new PropertyChangedCallback(on**REPLACE**Changed)));
        //public int **REPLACE**
        //{
        //    get { return (int)GetValue(**REPLACE**Property); }
        //    set { SetValue(**REPLACE**Property, value); }
        //}
        //public static void on**REPLACE**Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        //{
        //}
        //public static DependencyProperty **REPLACE**Property =
        //DependencyProperty.RegisterAttached("**REPLACE**",
        //typeof(int),
        //typeof(AreaControlUC),
        //new FrameworkPropertyMetadata(new PropertyChangedCallback(on**REPLACE**Changed)));
        //public int **REPLACE**
        //{
        //    get { return (int)GetValue(**REPLACE**Property); }
        //    set { SetValue(**REPLACE**Property, value); }
        //}
        //public static void on**REPLACE**Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        //{
        //}
        //public static readonly DependencyProperty **REPLACE**Property =
        //DependencyProperty.Register(
        //"**REPLACE**",
        //typeof(int),
        //typeof(AreaControlUC));
        //public static DependencyProperty **REPLACE**Property =
        //DependencyProperty.RegisterAttached("**REPLACE**",
        //typeof(int),
        //typeof(AreaControlUC),
        //new FrameworkPropertyMetadata(new PropertyChangedCallback(on**REPLACE**Changed)));
        //public int **REPLACE**
        //{
        //    get { return (int)GetValue(**REPLACE**Property); }
        //    set { SetValue(**REPLACE**Property, value); }
        //}
        //public static void on**REPLACE**Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        //{
        //}
        /// <summary>
        /// Responds to the resolution preset selector being selected by updating the LSM resolutions and
        /// deselecting the just selected resolution, causing the selector to show the default text again,
        /// which is the current resolution
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ResolutionSelector_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ComboBox resolutions = sender as ComboBox;
            ImageResolution selectedResolution = resolutions.SelectedItem as ImageResolution;

            if (selectedResolution != null && !selectedResolution.HasInvalidPixelDimension())
            {
                LSMPixelX = selectedResolution.XPixels;
                LSMPixelY = selectedResolution.YPixels;
                resolutions.SelectedIndex = -1;
            }
        }

        private void sliderPixelX_PreviewMouseUp(object sender, MouseButtonEventArgs e)
        {
            if (sliderPixelX.Value > LSMPixelX)
            {
                LSMPixelX += 32;
            }
            else if (sliderPixelX.Value < LSMPixelX)
            {
                LSMPixelX -= 32;
            }
        }

        private void sliderPixelX_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            LSMPixelX = (int)e.NewValue;
        }

        private void sliderPixelY_PreviewMouseUp(object sender, MouseButtonEventArgs e)
        {
            if (sliderPixelY.Value > LSMPixelY)
            {
                LSMPixelY += LSMPixelYMultiple;
            }
            else if (sliderPixelY.Value < LSMPixelY)
            {
                LSMPixelY -= LSMPixelYMultiple;
            }
        }

        private void sliderPixelY_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            LSMPixelY = (int)e.NewValue;
        }

        private void txtFieldSize_LostFocus(object sender, RoutedEventArgs e)
        {
            int temp = LSMFieldSize;
            if (Int32.TryParse(txtFieldSize.Text, out temp))
            {
                LSMFieldSize = temp;
            }
        }

        #endregion Methods
    }

    public class AreaVisibilityConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            //if (null == AreaControlView._liveVM)
            //{
            //    return Visibility.Collapsed;
            //}

            int val = Convert.ToInt32(value);

            int param = Convert.ToInt32(parameter);

            if (val == param)
            {
                return Visibility.Visible;
            }
            else
            {
                return Visibility.Collapsed;
            }
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return 0;
        }

        #endregion Methods
    }
}