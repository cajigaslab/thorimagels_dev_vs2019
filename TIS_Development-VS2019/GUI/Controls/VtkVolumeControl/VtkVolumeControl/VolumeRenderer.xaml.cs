namespace VtkVolumeControl
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
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
    using System.Xml;

    using Kitware.mummy.Runtime;
    using Kitware.VTK;

    /// <summary>
    /// Interaction logic for VolumeRenderer.xaml
    /// </summary>
    public partial class VolumeRenderer : UserControl
    {
        #region Fields

        public const int MAX_14BIT = 16383;
        public const int MAX_16BIT = 65535;
        public const int MAX_8BIT = 255;

        public static DependencyProperty ApplicationSettingsDirectoryProperty = 
                     DependencyProperty.RegisterAttached("ApplicationSettingsDirectory",
                     typeof(string),
                     typeof(VolumeRenderer));
        public static DependencyProperty DataExtentXProperty = 
                          DependencyProperty.RegisterAttached("DataExtentX",
                          typeof(int),
                          typeof(VolumeRenderer));
        public static DependencyProperty DataExtentYProperty = 
                          DependencyProperty.RegisterAttached("DataExtentY",
                          typeof(int),
                          typeof(VolumeRenderer));
        public static DependencyProperty DataExtentZProperty = 
                          DependencyProperty.RegisterAttached("DataExtentZ",
                          typeof(int),
                          typeof(VolumeRenderer));
        public static DependencyProperty DataSpacingZMultiplierProperty = 
                DependencyProperty.RegisterAttached("DataSpacingZMultiplier",
                typeof(double),
                typeof(VolumeRenderer),
                new FrameworkPropertyMetadata(1.0, new PropertyChangedCallback(onDataSpacingZMultiplierChanged)));
        public static DependencyProperty DataSpacingZProperty = 
                        DependencyProperty.RegisterAttached("DataSpacingZ",
                        typeof(double),
                        typeof(VolumeRenderer),
                        new FrameworkPropertyMetadata(1.0, new PropertyChangedCallback(onDataSpacingZChanged)));
        public static DependencyProperty FolderDirectoryProperty = 
                     DependencyProperty.RegisterAttached("FolderDirectory",
                     typeof(string),
                     typeof(VolumeRenderer),
                     new FrameworkPropertyMetadata(null, new PropertyChangedCallback(OnFolderDirectoryChanged)));
        public static DependencyProperty HardwareSettingsFileProperty = 
                    DependencyProperty.RegisterAttached("HardwareSettingsFile",
                    typeof(string),
                    typeof(VolumeRenderer));
        public static DependencyProperty IndexOfZStreamProperty = 
                   DependencyProperty.RegisterAttached("IndexOfZStream",
                   typeof(int),
                   typeof(VolumeRenderer), new FrameworkPropertyMetadata(1));
        public static DependencyProperty IsChanASelectedProperty = 
             DependencyProperty.RegisterAttached("IsChanASelected",
             typeof(bool),
             typeof(VolumeRenderer),
             new FrameworkPropertyMetadata(true, new PropertyChangedCallback(OnIsChanASelectedChanged)));
        public static DependencyProperty IsChanBSelectedProperty = 
             DependencyProperty.RegisterAttached("IsChanBSelected",
             typeof(bool),
             typeof(VolumeRenderer),
             new FrameworkPropertyMetadata(true, new PropertyChangedCallback(OnIsChanBSelectedChanged)));
        public static DependencyProperty IsChanCSelectedProperty = 
             DependencyProperty.RegisterAttached("IsChanCSelected",
             typeof(bool),
             typeof(VolumeRenderer),
             new FrameworkPropertyMetadata(true, new PropertyChangedCallback(OnIsChanCSelectedChanged)));
        public static DependencyProperty IsChanDSelectedProperty = 
             DependencyProperty.RegisterAttached("IsChanDSelected",
             typeof(bool),
             typeof(VolumeRenderer),
             new FrameworkPropertyMetadata(true, new PropertyChangedCallback(OnIsChanDSelectedChanged)));
        public static DependencyProperty LowerThresholdChanAProperty = 
                      DependencyProperty.RegisterAttached("LowerThresholdChanA",
                      typeof(int),
                      typeof(VolumeRenderer),
                      new FrameworkPropertyMetadata(0, new PropertyChangedCallback(onLowerThresholdChanAChanged)));
        public static DependencyProperty LowerThresholdChanBProperty = 
                      DependencyProperty.RegisterAttached("LowerThresholdChanB",
                      typeof(int),
                      typeof(VolumeRenderer),
                      new FrameworkPropertyMetadata(0, new PropertyChangedCallback(onLowerThresholdChanBChanged)));
        public static DependencyProperty LowerThresholdChanCProperty = 
                      DependencyProperty.RegisterAttached("LowerThresholdChanC",
                      typeof(int),
                      typeof(VolumeRenderer),
                      new FrameworkPropertyMetadata(0, new PropertyChangedCallback(onLowerThresholdChanCChanged)));
        public static DependencyProperty LowerThresholdChanDProperty = 
                      DependencyProperty.RegisterAttached("LowerThresholdChanD",
                      typeof(int),
                      typeof(VolumeRenderer),
                      new FrameworkPropertyMetadata(0, new PropertyChangedCallback(onLowerThresholdChanDChanged)));
        public static DependencyProperty NumberOfComponentsProperty = 
                     DependencyProperty.RegisterAttached("NumberOfComponents",
                     typeof(int),
                     typeof(VolumeRenderer), new FrameworkPropertyMetadata(2));
        public static DependencyProperty NumberOfZStreamProperty = 
                    DependencyProperty.RegisterAttached("NumberOfZStream",
                    typeof(int),
                    typeof(VolumeRenderer), new FrameworkPropertyMetadata(1));
        public static DependencyProperty RenderedVolumeXMaxProperty = 
                          DependencyProperty.RegisterAttached("RenderedVolumeXMax",
                          typeof(int),
                          typeof(VolumeRenderer));
        public static DependencyProperty RenderedVolumeXMinProperty = 
                           DependencyProperty.RegisterAttached("RenderedVolumeXMin",
                           typeof(int),
                           typeof(VolumeRenderer));
        public static DependencyProperty RenderedVolumeYMaxProperty = 
                          DependencyProperty.RegisterAttached("RenderedVolumeYMax",
                          typeof(int),
                          typeof(VolumeRenderer));
        public static DependencyProperty RenderedVolumeYMinProperty = 
                          DependencyProperty.RegisterAttached("RenderedVolumeYMin",
                          typeof(int),
                          typeof(VolumeRenderer));
        public static DependencyProperty RenderedVolumeZMaxProperty = 
                          DependencyProperty.RegisterAttached("RenderedVolumeZMax",
                          typeof(int),
                          typeof(VolumeRenderer));
        public static DependencyProperty RenderedVolumeZMinProperty = 
                          DependencyProperty.RegisterAttached("RenderedVolumeZMin",
                          typeof(int),
                          typeof(VolumeRenderer));
        public static DependencyProperty TileIndexProperty = 
                      DependencyProperty.RegisterAttached("TileIndex",
                      typeof(int),
                      typeof(VolumeRenderer), new FrameworkPropertyMetadata(1, new PropertyChangedCallback(OnTileIndexChanged)));
        public static DependencyProperty TimePointIndexProperty = 
                     DependencyProperty.RegisterAttached("TimePointIndex",
                     typeof(int),
                     typeof(VolumeRenderer), new FrameworkPropertyMetadata(1, new PropertyChangedCallback(OnTimePointIndexChanged)));
        public static DependencyProperty TimepointsProperty = 
                     DependencyProperty.RegisterAttached("Timepoints",
                     typeof(int),
                     typeof(VolumeRenderer), new FrameworkPropertyMetadata(1));
        public static DependencyProperty TotalSystemChannelsProperty = 
                    DependencyProperty.RegisterAttached("TotalSystemChannels",
                    typeof(int),
                    typeof(VolumeRenderer), new FrameworkPropertyMetadata(4));
        public static DependencyProperty UpperThresholdChanAProperty = 
                      DependencyProperty.RegisterAttached("UpperThresholdChanA",
                      typeof(int),
                      typeof(VolumeRenderer),
                      new FrameworkPropertyMetadata(MaxGrayScaleLevel, new PropertyChangedCallback(onUpperThresholdChanAChanged)));
        public static DependencyProperty UpperThresholdChanBProperty = 
                      DependencyProperty.RegisterAttached("UpperThresholdChanB",
                      typeof(int),
                      typeof(VolumeRenderer),
                      new FrameworkPropertyMetadata(MaxGrayScaleLevel, new PropertyChangedCallback(onUpperThresholdChanBChanged)));
        public static DependencyProperty UpperThresholdChanCProperty = 
                      DependencyProperty.RegisterAttached("UpperThresholdChanC",
                      typeof(int),
                      typeof(VolumeRenderer),
                      new FrameworkPropertyMetadata(MaxGrayScaleLevel, new PropertyChangedCallback(onUpperThresholdChanCChanged)));
        public static DependencyProperty UpperThresholdChanDProperty = 
                      DependencyProperty.RegisterAttached("UpperThresholdChanD",
                      typeof(int),
                      typeof(VolumeRenderer),
                      new FrameworkPropertyMetadata(MaxGrayScaleLevel, new PropertyChangedCallback(onUpperThresholdChanDChanged)));
        public static DependencyProperty WellIndexProperty = 
                      DependencyProperty.RegisterAttached("WellIndex",
                      typeof(int),
                      typeof(VolumeRenderer), new FrameworkPropertyMetadata(1, new PropertyChangedCallback(OnWellIndexChanged)));
        public static DependencyProperty ZStreamModeProperty = 
                    DependencyProperty.RegisterAttached("ZStreamMode",
                    typeof(int),
                    typeof(VolumeRenderer), new FrameworkPropertyMetadata(0));

        private static int[] _componentIndexForChannel = { -1, -1, -1, -1 }; // -1 for empty channel

        double angle;
        private vtkAxesActor axes;
        private int[] BlackPoints = { 0, 0, 0, 0 };
        private vtkCamera camera;
        private List<vtkImageChangeInformation> changeFilters;

        //private vtkTIFFReader reader;
        private vtkImageAppendComponents componentAdaptor;
        private double[] defaultCamPos = { 0, 0, 1 };
        private double[] defaultCamViewup = { 0, 1, 0 };
        private List<vtkStringArray> fileNameArrays;
        private List<vtkImageFlip> flippers;
        private vtkRenderWindowInteractor iren;
        private int[] lowerThreshold = { 0, 0, 0, 0 };
        private List<vtkTIFFReader> readers;
        private vtkRenderer renderer;
        private vtkRenderWindow renderWindow;
        private RenderWindowControl renWindowControl;
        private vtkTransform transform;
        private int[] upperThreshold = { MaxGrayScaleLevel, MaxGrayScaleLevel, MaxGrayScaleLevel, MaxGrayScaleLevel };
        private vtkVolumeProperty volProperty;
        private vtkVolume volume;
        private vtkFixedPointVolumeRayCastMapper volumeMapper;
        private int[] WhitePoints = { MaxGrayScaleLevel, MaxGrayScaleLevel, MaxGrayScaleLevel, MaxGrayScaleLevel };
        private vtkOrientationMarkerWidget widget;
        private int[] _isChannelSelected = { 1, 1, 1, 1 }; //array indication selected status for the indexed component, 0 for ChanA
        private bool _isVtkPipelineReady = false;
        private bool _isZStackDataExist = false;

        // these two arrays specify the color range for the components specified by the array indexes
        byte[][] _pal = new byte[4][];
        private bool _resetCamera;
        private bool _resetView;
        private int _scannerChoice = 0; // 0 - LSM only; 1 - Camera only; 2 - Both Camera and LSM
        private double _xy2zRatio;
        private double _zMultiplierLow;
        private double _zSpacingTuner;

        #endregion Fields

        #region Constructors

        //private int[] upperThreshold = { MAX_8BIT, MAX_8BIT, MAX_8BIT, MAX_8BIT };
        public VolumeRenderer()
        {
            InitializeComponent();

            if (!IsInDesignMode)    // to disable this block in design mode
            {
                this.Loaded += new RoutedEventHandler(VolumeRenderer_Loaded);
                this.Unloaded += new RoutedEventHandler(VolumeRenderer_Unloaded);
                renWindowControl = new RenderWindowControl();
                renderWindow = renWindowControl.RenderWindow;

                formHolder.Child = renWindowControl;

                volumeMapper = vtkFixedPointVolumeRayCastMapper.New();
                volume = vtkVolume.New();
                volProperty = vtkVolumeProperty.New();
                volume.SetProperty(volProperty);
                componentAdaptor = vtkImageAppendComponents.New();
                fileNameArrays = new List<vtkStringArray>();
                readers = new List<vtkTIFFReader>();
                flippers = new List<vtkImageFlip>();
                changeFilters = new List<vtkImageChangeInformation>();
                axes = vtkAxesActor.New();
                widget = vtkOrientationMarkerWidget.New();
                camera = vtkCamera.New();
                transform = vtkTransform.New();
                DataExtentX = -1;
                DataExtentY = -1;
                DataExtentZ = -1;
                _resetView = false;
                _resetCamera = false;
                angle = Math.PI / 4;
                _zSpacingTuner = 1;
                _zMultiplierLow = 0.005;

            }
        }

        #endregion Constructors

        #region Properties

        public static int MaxGrayScaleLevel
        {
            get
            {
                //return MAX_8BIT;
                //return MAX_16BIT;
                return MAX_14BIT;
            }
        }

        public static int NumberOfColorBins
        {
            get
            {
                return MAX_8BIT;
            }
        }

        public string ApplicationSettingsDirectory
        {
            get
            {
                return (string)GetValue(ApplicationSettingsDirectoryProperty);
            }
            set
            {
                SetValue(ApplicationSettingsDirectoryProperty, value);
            }
        }

        public int DataExtentX
        {
            get
            {
                return (int)GetValue(DataExtentXProperty);
            }
            set
            {
                SetValue(DataExtentXProperty, value);
            }
        }

        public int DataExtentY
        {
            get
            {
                return (int)GetValue(DataExtentYProperty);
            }
            set
            {
                SetValue(DataExtentYProperty, value);
            }
        }

        public int DataExtentZ
        {
            get
            {
                return (int)GetValue(DataExtentZProperty);
            }
            set
            {
                SetValue(DataExtentZProperty, value);
            }
        }

        public double DataSpacingZ
        {
            get
            {
                return (double)GetValue(DataSpacingZProperty);
            }
            set
            {
                SetValue(DataSpacingZProperty, value);
            }
        }

        public double DataSpacingZMultiplier
        {
            get
            {
                return (double)GetValue(DataSpacingZMultiplierProperty);
            }
            set
            {
                SetValue(DataSpacingZMultiplierProperty, value);
            }
        }

        public string FolderDirectory
        {
            get
            {
                return (string)GetValue(FolderDirectoryProperty);
            }
            set
            {
                SetValue(FolderDirectoryProperty, value);
            }
        }

        public string HardwareSettingsFile
        {
            get
            {
                return (string)GetValue(HardwareSettingsFileProperty);
            }
            set
            {
                SetValue(HardwareSettingsFileProperty, value);
            }
        }

        public int IndexOfZStream
        {
            get
            {
                return (int)GetValue(IndexOfZStreamProperty);
            }
            set
            {
                SetValue(IndexOfZStreamProperty, value);
            }
        }

        public string IsChanASelected
        {
            get
            {
                return (string)GetValue(IsChanASelectedProperty);
            }
            set
            {
                SetValue(IsChanASelectedProperty, value);
            }
        }

        public string IsChanBSelected
        {
            get
            {
                return (string)GetValue(IsChanBSelectedProperty);
            }
            set
            {
                SetValue(IsChanBSelectedProperty, value);
            }
        }

        public string IsChanCSelected
        {
            get
            {
                return (string)GetValue(IsChanCSelectedProperty);
            }
            set
            {
                SetValue(IsChanCSelectedProperty, value);
            }
        }

        public string IsChanDSelected
        {
            get
            {
                return (string)GetValue(IsChanDSelectedProperty);
            }
            set
            {
                SetValue(IsChanDSelectedProperty, value);
            }
        }

        public bool IsVolumeRendererReady
        {
            get
            {
                return ((true == _isVtkPipelineReady) && (null != renWindowControl.RenderWindow));
            }
        }

        public bool IsWindowRendered
        {
            get
            {
                return (renWindowControl != null) && (renWindowControl.RenderWindow != null);
            }
        }

        public int LowerThresholdChanA
        {
            get
            {
                return (int)GetValue(LowerThresholdChanAProperty);
            }
            set
            {
                SetValue(LowerThresholdChanAProperty, value);
            }
        }

        public int LowerThresholdChanB
        {
            get
            {
                return (int)GetValue(LowerThresholdChanBProperty);
            }
            set
            {
                SetValue(LowerThresholdChanBProperty, value);
            }
        }

        public int LowerThresholdChanC
        {
            get
            {
                return (int)GetValue(LowerThresholdChanCProperty);
            }
            set
            {
                SetValue(LowerThresholdChanCProperty, value);
            }
        }

        public int LowerThresholdChanD
        {
            get
            {
                return (int)GetValue(LowerThresholdChanDProperty);
            }
            set
            {
                SetValue(LowerThresholdChanDProperty, value);
            }
        }

        public int NumberOfComponents
        {
            get
            {
                return (int)GetValue(NumberOfComponentsProperty);
            }
            set
            {
                SetValue(NumberOfComponentsProperty, value);
            }
        }

        public int NumberOfZStream
        {
            get
            {
                return (int)GetValue(NumberOfZStreamProperty);
            }
            set
            {
                SetValue(NumberOfZStreamProperty, value);
            }
        }

        public int RenderedVolumeXMax
        {
            get
            {
                return (int)GetValue(RenderedVolumeXMaxProperty);
            }
            set
            {
                SetValue(RenderedVolumeXMaxProperty, value);
                DataExtentX = RenderedVolumeXMax - RenderedVolumeXMin;
            }
        }

        public int RenderedVolumeXMin
        {
            get
            {
                return (int)GetValue(RenderedVolumeXMinProperty);
            }
            set
            {
                SetValue(RenderedVolumeXMinProperty, value);
                DataExtentX = RenderedVolumeXMax - RenderedVolumeXMin;
            }
        }

        public int RenderedVolumeYMax
        {
            get
            {
                return (int)GetValue(RenderedVolumeYMaxProperty);
            }
            set
            {
                SetValue(RenderedVolumeYMaxProperty, value);
            }
        }

        public int RenderedVolumeYMin
        {
            get
            {
                return (int)GetValue(RenderedVolumeYMinProperty);
            }
            set
            {
                SetValue(RenderedVolumeYMinProperty, value);
            }
        }

        public int RenderedVolumeZMax
        {
            get
            {
                return (int)GetValue(RenderedVolumeZMaxProperty);
            }
            set
            {
                SetValue(RenderedVolumeZMaxProperty, value);
            }
        }

        public int RenderedVolumeZMin
        {
            get
            {
                return (int)GetValue(RenderedVolumeZMinProperty);
            }
            set
            {
                SetValue(RenderedVolumeZMinProperty, value);
            }
        }

        public bool ResetView
        {
            get
            {
                return _resetView;
            }
            set
            {
                _resetView = value;
            }
        }

        public int TileIndex
        {
            get
            {
                return (int)GetValue(TileIndexProperty);
            }
            set
            {
                SetValue(TileIndexProperty, value);
            }
        }

        public int TimePointIndex
        {
            get
            {
                return (int)GetValue(TimePointIndexProperty);
            }
            set
            {
                SetValue(TimePointIndexProperty, value);
            }
        }

        public int Timepoints
        {
            get
            {
                return (int)GetValue(TimepointsProperty);
            }
            set
            {
                SetValue(TimepointsProperty, value);
            }
        }

        public int TotalSystemChannels
        {
            get
            {
                return (int)GetValue(TotalSystemChannelsProperty);
            }
            set
            {
                SetValue(TotalSystemChannelsProperty, value);
            }
        }

        public int UpperThresholdChanA
        {
            get
            {
                return (int)GetValue(UpperThresholdChanAProperty);
            }
            set
            {
                SetValue(UpperThresholdChanAProperty, value);
            }
        }

        public int UpperThresholdChanB
        {
            get
            {
                return (int)GetValue(UpperThresholdChanBProperty);
            }
            set
            {
                SetValue(UpperThresholdChanBProperty, value);
            }
        }

        public int UpperThresholdChanC
        {
            get
            {
                return (int)GetValue(UpperThresholdChanCProperty);
            }
            set
            {
                SetValue(UpperThresholdChanCProperty, value);
            }
        }

        public int UpperThresholdChanD
        {
            get
            {
                return (int)GetValue(UpperThresholdChanDProperty);
            }
            set
            {
                SetValue(UpperThresholdChanDProperty, value);
            }
        }

        public int WellIndex
        {
            get
            {
                return (int)GetValue(WellIndexProperty);
            }
            set
            {
                SetValue(WellIndexProperty, value);
            }
        }

        public double Xy2zRatio
        {
            get
            {
                return _xy2zRatio;
            }
            set
            {
                _xy2zRatio = value;
            }
        }

        public double ZMultiplierLow
        {
            get
            {
                return _zMultiplierLow;
            }
            set
            {
                _zMultiplierLow = value;
            }
        }

        public double ZSpacingTuner
        {
            get
            {
                return _zSpacingTuner;
            }
            set
            {
                _zSpacingTuner = value;
            }
        }

        public int ZStreamMode
        {
            get
            {
                return (int)GetValue(ZStreamModeProperty);
            }
            set
            {
                SetValue(ZStreamModeProperty, value);
            }
        }

        private bool IsInDesignMode
        {
            get
            {
                return DesignerProperties.GetIsInDesignMode(this);
            }
        }

        #endregion Properties

        #region Methods

        public static void onDataSpacingZChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            (d as VolumeRenderer).updateCroppingRegionPlanes();
            (d as VolumeRenderer).updateChangeFilters();
        }

        public static void onDataSpacingZMultiplierChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if ((d as VolumeRenderer)._zMultiplierLow >= (d as VolumeRenderer).DataSpacingZMultiplier)
            {
                (d as VolumeRenderer).DataSpacingZMultiplier = (d as VolumeRenderer)._zMultiplierLow;
            }

            (d as VolumeRenderer).updateCroppingRegionPlanes();
            (d as VolumeRenderer).updateChangeFilters();
        }

        public static void OnFolderDirectoryChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            (d as VolumeRenderer).VolumeRenderReset();  // reset some of the parameters from previous experiment if new data are loaded
            (d as VolumeRenderer).RenderVolume();
        }

        public static void OnIsChanASelectedChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            (d as VolumeRenderer).selectChannel(0, Convert.ToBoolean(e.NewValue));
            (d as VolumeRenderer).UpdateVolumeColor();
        }

        public static void OnIsChanBSelectedChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            (d as VolumeRenderer).selectChannel(1, Convert.ToBoolean(e.NewValue));
            (d as VolumeRenderer).UpdateVolumeColor();
        }

        public static void OnIsChanCSelectedChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            (d as VolumeRenderer).selectChannel(2, Convert.ToBoolean(e.NewValue));
            (d as VolumeRenderer).UpdateVolumeColor();
        }

        public static void OnIsChanDSelectedChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            (d as VolumeRenderer).selectChannel(3, Convert.ToBoolean(e.NewValue));
            (d as VolumeRenderer).UpdateVolumeColor();
        }

        public static void onLowerThresholdChanAChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int value = Convert.ToInt32(e.NewValue);
            (d as VolumeRenderer).setLowerThreshold(_componentIndexForChannel[0], value, 0);
        }

        public static void onLowerThresholdChanBChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int value = Convert.ToInt32(e.NewValue);
            (d as VolumeRenderer).setLowerThreshold(_componentIndexForChannel[1], value, 1);
        }

        public static void onLowerThresholdChanCChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int value = Convert.ToInt32(e.NewValue);
            (d as VolumeRenderer).setLowerThreshold(_componentIndexForChannel[2], value, 2);
        }

        public static void onLowerThresholdChanDChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int value = Convert.ToInt32(e.NewValue);
            (d as VolumeRenderer).setLowerThreshold(_componentIndexForChannel[3], value, 3);
        }

        public static void OnTileIndexChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            (d as VolumeRenderer).RenderVolume();
        }

        public static void OnTimePointIndexChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            (d as VolumeRenderer).RenderVolume();
        }

        public static void onUpperThresholdChanAChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int value = Convert.ToInt32(e.NewValue);
            (d as VolumeRenderer).setUpperThreshold(_componentIndexForChannel[0], value, 0);
        }

        public static void onUpperThresholdChanBChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int value = Convert.ToInt32(e.NewValue);
            (d as VolumeRenderer).setUpperThreshold(_componentIndexForChannel[1], value, 1);
        }

        public static void onUpperThresholdChanCChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int value = Convert.ToInt32(e.NewValue);
            (d as VolumeRenderer).setUpperThreshold(_componentIndexForChannel[2], value, 2);
        }

        public static void onUpperThresholdChanDChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int value = Convert.ToInt32(e.NewValue);
            (d as VolumeRenderer).setUpperThreshold(_componentIndexForChannel[3], value, 3);
        }

        public static void OnWellIndexChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            (d as VolumeRenderer).RenderVolume();
        }

        public IntPtr DoubleArrayToIntPtr(double[] d)
        {
            IntPtr p = Marshal.AllocCoTaskMem(sizeof(double) * d.Length);
            Marshal.Copy(d, 0, p, d.Length);
            return p;
        }

        /// <summary>
        /// tells the VTK RenderWindow to re-render
        /// this method should be called every time after the setApplicationColorMapping or ReadVolume is called
        /// </summary>
        public void ForceWindowToRender()
        {
            if ((null != renWindowControl) && (null != renWindowControl.RenderWindow))
            {
                renWindowControl.RenderWindow.Render();
            }
        }

        public void loadParametersFromXML(string xmlFileName)
        {
            if (System.IO.File.Exists(xmlFileName))
            {
                //FolderDirectory = xmlFileName.Remove(xmlFileName.LastIndexOf("\\"));

                XmlDocument xmlDoc = new XmlDocument();
                xmlDoc.Load(xmlFileName);
                _componentIndexForChannel[0] = -1;
                _componentIndexForChannel[1] = -1;
                _componentIndexForChannel[2] = -1;
                _componentIndexForChannel[3] = -1;
                XmlNodeList wavelengthsList = xmlDoc.SelectNodes("/ThorImageExperiment/Wavelengths");
                XmlNodeList chanList = null;
                if (0 < wavelengthsList.Count)
                {
                    chanList = wavelengthsList[0].SelectNodes("Wavelength");
                    NumberOfComponents = chanList.Count;
                }
                else
                {
                    NumberOfComponents = 0;
                }

                for (int i = 0; i < NumberOfComponents; i++)
                {
                    for (int j = 0; j < chanList[i].Attributes.Count; j++)
                    {
                        if (chanList[i].Attributes[j].Name == "name")
                        {
                            if (chanList[i].Attributes[j].Value == "ChanA")
                            {
                                _componentIndexForChannel[0] = i;
                            }
                            else if (chanList[i].Attributes[j].Value == "ChanB")
                            {
                                _componentIndexForChannel[1] = i;
                            }
                            else if (chanList[i].Attributes[j].Value == "ChanC")
                            {
                                _componentIndexForChannel[2] = i;
                            }
                            else if (chanList[i].Attributes[j].Value == "ChanD")
                            {
                                _componentIndexForChannel[3] = i;
                            }
                        }
                    }
                }

                ParseScannerChoice(xmlDoc);

                switch (_scannerChoice)
                {
                    case 0:
                        {
                            XmlNodeList lsmNodelist = xmlDoc.GetElementsByTagName("LSM");
                            XmlNode lsmNode = lsmNodelist[0];

                            if (DataExtentX < 0)    // DataExtent only load xml to reset when new data are loaded,
                            {                       // otherwise as is
                                DataExtentX = Convert.ToInt32(lsmNode.Attributes["pixelX"].Value) - 1;
                                RenderedVolumeXMax = DataExtentX;
                                RenderedVolumeXMin = 0;
                            }

                            if (DataExtentY < 0)
                            {
                                DataExtentY = Convert.ToInt32(lsmNode.Attributes["pixelY"].Value) - 1;
                                RenderedVolumeYMax = DataExtentY;
                                RenderedVolumeYMin = 0;
                            }
                        }
                        break;

                    case 1:
                        XmlNodeList camNodeList = xmlDoc.GetElementsByTagName("Camera");
                        if (camNodeList.Count > 0)
                        {
                            int left = 0; int right = 0; int top = 0; int bottom = 0;
                            int binX = 0; int binY = 0; int imageAngle = 0;
                            string str = string.Empty;
                            if (GetAttribute(camNodeList[0], xmlDoc, "left", ref str))
                            {
                                left = Convert.ToInt32(str);
                            }
                            if (GetAttribute(camNodeList[0], xmlDoc, "right", ref str))
                            {
                                right = Convert.ToInt32(str);
                            }
                            if (GetAttribute(camNodeList[0], xmlDoc, "top", ref str))
                            {
                                top = Convert.ToInt32(str);
                            }
                            if (GetAttribute(camNodeList[0], xmlDoc, "bottom", ref str))
                            {
                                bottom = Convert.ToInt32(str);
                            }
                            if (GetAttribute(camNodeList[0], xmlDoc, "binningX", ref str))
                            {
                                binX = Convert.ToInt32(str);
                            }
                            if (GetAttribute(camNodeList[0], xmlDoc, "binningY", ref str))
                            {
                                binY = Convert.ToInt32(str);
                            }
                            if (GetAttribute(camNodeList[0], xmlDoc, "imageAngle", ref str))
                            {
                                imageAngle = Convert.ToInt32(str);
                            }
                            if (binX != 0)
                            {
                                if(0 == imageAngle || 180 == imageAngle)
                                {
                                    DataExtentX = (right - left) / binX - 1;
                                }
                                else if (0 != binY)
                                {
                                    DataExtentX = (bottom - top) / binY - 1;
                                }
                                RenderedVolumeXMax = DataExtentX;
                                RenderedVolumeXMin = 0;
                            }
                            if (binY != 0)
                            {
                                if(0 == imageAngle || 180 == imageAngle)
                                {
                                    DataExtentY = (bottom - top) / binY - 1;
                                }
                                else if (0 != binY)
                                {
                                    DataExtentY = (right - left) / binX - 1;
                                }
                                RenderedVolumeXMax = DataExtentX;
                                RenderedVolumeXMin = 0;
                            }
                        }
                        break;
                }

                //RenderedVolumeXMax = DataExtentX;
                //RenderedVolumeXMin = 0;
                //RenderedVolumeYMax = DataExtentY;
                //RenderedVolumeYMin = 0;

                XmlNodeList ndList = xmlDoc.SelectNodes("/ThorImageExperiment/ZStage");
                if (ndList.Count > 0)
                {
                    if (DataExtentZ < 0 || _resetView)
                    {
                        string strSteps = string.Empty;
                        if (GetAttribute(ndList[0], xmlDoc, "steps", ref strSteps))
                        {
                            DataExtentZ = Convert.ToInt32(strSteps);
                            RenderedVolumeZMax = DataExtentZ;
                            RenderedVolumeZMin = 0;
                        }
                    }

                    string strStreamMode = string.Empty;
                    ZStreamMode = 0;
                    NumberOfZStream = 1;

                    //update zstreamMode and NumberOfZStream values if it is supported
                    //otherwise, use default values.
                    if (GetAttribute(ndList[0], xmlDoc, "zStreamMode", ref strStreamMode))
                    {
                        ZStreamMode = Int32.Parse(strStreamMode);
                        string strZStream = string.Empty;
                        if ((ZStreamMode > 0) && GetAttribute(ndList[0], xmlDoc, "zStreamFrames", ref strZStream))
                        {
                            NumberOfZStream = Math.Max(Int32.Parse(strZStream), 1);
                        }
                    }
                }

                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/Timelapse");
                if (ndList.Count > 0)
                {
                    string strTimepoints = string.Empty;
                    if (GetAttribute(ndList[0], xmlDoc, "timepoints", ref strTimepoints))
                    {
                        Timepoints = Convert.ToInt32(strTimepoints);
                    }
                }

            }
        }

        /// <summary>
        /// This method read the data from all (less than or equal to 4) channels 
        /// for the sample specified by the well index, tile index, and time index
        /// </summary>
        /// <param name="folderPath"></param>
        /// <param name="channels"></param>
        /// <param name="well"></param>
        /// <param name="tile"></param>
        /// <param name="time"></param>
        public void readVolumeChannels(string folderPath, int channels, int well, int tile, int time)
        {
            if (channels > 4 && channels < 1)
                MessageBox.Show("More than 4 channels are not supported for now.");
            try
            {
                if (!System.IO.Directory.Exists(folderPath))
                {
                    return;
                }

                //First dispose all readers, flippers and change filters so that before
                // rendering a different volume
                disposeFileNameArrays();
                disposeReaders();
                disposeFlippers();
                disposeChangeFilters();
                componentAdaptor.RemoveAllInputs();

                for (int i = 0; i < TotalSystemChannels; i++)
                {
                    if (_componentIndexForChannel[i] > -1)
                    {
                        vtkStringArray tiffNames = vtkStringArray.New();
                        for (int z = 1; z <= DataExtentZ; z++)
                        {
                            string filename = GetFileName(folderPath, i, well, tile, z, time);
                            tiffNames.InsertNextValue(filename);
                        }

                        // VTK BUG: has to insert something at end of vtkStringArray
                        // otherwise, won't be able to get last image out
                        tiffNames.InsertNextValue("Null");

                        fileNameArrays.Add(tiffNames);

                        // start pipeline visualization and read from tiff images
                        vtkTIFFReader reader = vtkTIFFReader.New();

                        reader.SetFileNames(tiffNames);

                        //reader.SetDataExtent(0, DataExtentX, 0, DataExtentY, 0, DataExtentZ);
                        //MessageBox.Show(reader.GetOrientationType().ToString() + ", " + reader.GetOrientationTypeSpecifiedFlag().ToString());
                        readers.Add(reader);
                        //reader.Update();

                        // Flip the images because the VTK reads images upside down
                        vtkImageFlip flipY = vtkImageFlip.New();
                        flipY.SetFilteredAxes(1);
                        flipY.SetInputConnection(reader.GetOutputPort());
                        flippers.Add(flipY);
                        //flipY.Update();

                        vtkImageChangeInformation changeFilter = vtkImageChangeInformation.New();
                        changeFilter.SetInputConnection(flipY.GetOutputPort());
                        changeFilter.SetOutputSpacing(1, 1, DataSpacingZ * DataSpacingZMultiplier * _zSpacingTuner);
                        changeFilters.Add(changeFilter);
                        componentAdaptor.AddInputConnection(changeFilter.GetOutputPort());
                        //componentAdaptor.Update();
                    }
                }

                if (0 < channels)
                {
                    _isZStackDataExist = true;
                }
                else
                {
                    _isZStackDataExist = false;
                }

                if (folderPath.Contains("ZStackCache"))
                {
                    _resetView = true;
                }

                //componentAdaptor.Update();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "User Exception Error in readVolumeChannels function.");
            }
        }

        public bool ReadVolumeData()
        {
            if (Directory.Exists(FolderDirectory))
            {
                if ((WellIndex > 0) && (TileIndex > 0) && (TimePointIndex > 0))
                {
                    //readAVolume(FolderDirectory, WellIndex, TileIndex, TimePointIndex);
                    readVolumeChannels(FolderDirectory, NumberOfComponents, WellIndex, TileIndex, TimePointIndex);
                }
            }
            return false;
        }

        /// <summary>
        /// this is the method should be called from other classes in order
        /// to view 3d volume
        /// </summary>
        public void RenderVolume()
        {
            try
            {
                if (false == IsVolumeRendererReady)
                    return;

                string appPath = System.IO.Path.GetDirectoryName(System.Diagnostics.Process.GetCurrentProcess().MainModule.FileName);

                if (Directory.Exists(appPath))
                {
                    SetDllDirectory(appPath + "\\Lib");
                }

                if ((null == FolderDirectory) || (null == HardwareSettingsFile))
                {
                    throw new NullReferenceException("Reference to null ZStackCache or HardwareSettingsFile");
                }
                if ((!System.IO.Directory.Exists(FolderDirectory)) || (!System.IO.File.Exists(HardwareSettingsFile)))
                {
                    throw new System.IO.FileNotFoundException("ZStackCache or HardwareSettingsFile does not exist");
                }

                string expXML = FolderDirectory.ToString() + "\\Experiment.xml";

                if (_resetView)
                {
                    DataExtentX = -1;   // reset width, height and thickness, so to load from xml
                    DataExtentY = -1;
                    DataExtentZ = -1;

                    loadParametersFromXML(expXML);
                }

                if (DataExtentZ < 1)
                    return;

                updateCroppingRegionPlanes();

                setToDefault();

                ReadVolumeData();

                SetupScene();

                SetApplicationColorMapping();

                if (_resetView)
                {
                    _resetView = false;
                    Reset3DView();
                }

                ForceWindowToRender();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        public void Reset3DView()
        {
            DataSpacingZMultiplier = 1;

            if (null != camera && null != renderer)
            {
                double[] pos;
                cameraAutoFocus();
                pos = camera.GetFocalPoint();
                pos[2] += camera.GetDistance();
                camera.SetPosition(DoubleArrayToIntPtr(pos));
                camera.SetViewUp(DoubleArrayToIntPtr(defaultCamViewup));

                renderer.ResetCamera();
                renderer.GetActiveCamera();
                ForceWindowToRender();
            }
        }

        public void RotateToDown()
        {
            if (null != renderer)
            {
                double cos = Math.Cos(angle);
                double sin = Math.Sin(angle);

                double[] rot = { 1,    0,   0, 0,
                             0,  cos, sin, 0,
                             0, -sin, cos, 0,
                             0,    0,   0, 1};

                transform.SetMatrix(DoubleArrayToIntPtr(rot));
                camera.ApplyTransform(transform);
                renderer.ResetCamera();
                renderer.GetActiveCamera();
                ForceWindowToRender();
            }
        }

        public void RotateToLeft()
        {
            if (null != renderer)
            {
                double cos = Math.Cos(angle);
                double sin = Math.Sin(angle);

                double[] rot = {  cos, 0, sin, 0,
                                0, 1,   0, 0,
                             -sin, 0, cos, 0,
                                0, 0,   0, 1};

                transform.SetMatrix(DoubleArrayToIntPtr(rot));
                camera.ApplyTransform(transform);
                renderer.ResetCamera();
                renderer.GetActiveCamera();
                ForceWindowToRender();
            }
        }

        public void RotateToRight()
        {
            if (null != renderer)
            {
                double cos = Math.Cos(angle);
                double sin = Math.Sin(angle);

                double[] rot = { cos, 0, -sin, 0,
                               0, 1,    0, 0,
                             sin, 0,  cos, 0,
                               0, 0,    0, 1};

                transform.SetMatrix(DoubleArrayToIntPtr(rot));
                camera.ApplyTransform(transform);
                renderer.ResetCamera();
                renderer.GetActiveCamera();
                ForceWindowToRender();
            }
        }

        public void RotateToUp()
        {
            if (null != renderer)
            {
                double cos = Math.Cos(angle);
                double sin = Math.Sin(angle);

                double[] rot = { 1,   0,    0, 0,
                             0, cos, -sin, 0,
                             0, sin,  cos, 0,
                             0,   0,    0, 1};

                transform.SetMatrix(DoubleArrayToIntPtr(rot));
                camera.ApplyTransform(transform);
                renderer.ResetCamera();
                renderer.GetActiveCamera();
                ForceWindowToRender();
            }
        }

        //select or un-select channel
        public void selectChannel(int component, bool b)
        {
            if ((-1 < component) && (4 > component))
            {
                _isChannelSelected[component] = Convert.ToInt32(b);
            }
        }

        /// <summary>
        /// set the color for 3d volume according to the color settings specified in hardwaresettings.xml file
        /// This method should be called after render() because it relies on render() to associate colors with
        /// components for 3D rendering
        /// </summary>
        /// <param name="hardwareSettingsXml"></param>
        public void SetApplicationColorMapping()
        {
            lowerThreshold[0] = LowerThresholdChanA * MaxGrayScaleLevel / NumberOfColorBins;
            lowerThreshold[1] = LowerThresholdChanB * MaxGrayScaleLevel / NumberOfColorBins;
            lowerThreshold[2] = LowerThresholdChanC * MaxGrayScaleLevel / NumberOfColorBins;
            lowerThreshold[3] = LowerThresholdChanD * MaxGrayScaleLevel / NumberOfColorBins;

            upperThreshold[0] = UpperThresholdChanA * MaxGrayScaleLevel / NumberOfColorBins;
            upperThreshold[1] = UpperThresholdChanB * MaxGrayScaleLevel / NumberOfColorBins;
            upperThreshold[2] = UpperThresholdChanC * MaxGrayScaleLevel / NumberOfColorBins;
            upperThreshold[3] = UpperThresholdChanD * MaxGrayScaleLevel / NumberOfColorBins;

            string[] channel_color = new string[4];
            getChannelColors(channel_color);

            for (int i = 0; i < _componentIndexForChannel.Length; i++)
            {
                int tmp = _componentIndexForChannel[i];
                if ((-1 != tmp) && (0 < _isChannelSelected[i]))
                {
                    setColorEx(i, channel_color[i], lowerThreshold[i], upperThreshold[i]);
                }
            }
        }

        public void setDefaultColorMapping()
        {
            if (volume == null)
            {
                MessageBox.Show("No volume exists");
                return;
            }
            // vtkVolumeProperty volProperty = volume.GetProperty();

            vtkColorTransferFunction colorFunctionA = vtkColorTransferFunction.New();
            colorFunctionA.AddRGBSegment(0, 0, 0, 0, MaxGrayScaleLevel, 1, 0, 0);
            volProperty.SetColor(0, colorFunctionA);

            vtkColorTransferFunction colorFunctionB = vtkColorTransferFunction.New();
            colorFunctionB.AddRGBSegment(0, 0, 0, 0, MaxGrayScaleLevel, 0, 1, 0);
            volProperty.SetColor(1, colorFunctionB);

            vtkColorTransferFunction colorFunctionC = vtkColorTransferFunction.New();
            colorFunctionC.AddRGBSegment(0, 0, 0, 0, MaxGrayScaleLevel, 0, 0, 1);
            volProperty.SetColor(2, colorFunctionC);

            vtkColorTransferFunction colorFunctionD = vtkColorTransferFunction.New();
            colorFunctionD.AddRGBSegment(0, 0, 0, 0, MaxGrayScaleLevel, 0.5, 0.5, 0);
            volProperty.SetColor(3, colorFunctionD);

            //volProperty.SetInterpolationTypeToNearest();
            //renWindowControl.RenderWindow.Render();
        }

        public void SetRotAngle(double angleInRad)
        {
            angle = angleInRad;
        }

        public void setToDefault()
        {
            if (null == renWindowControl.RenderWindow)
                return;

            volumeMapper.SetBlendModeToMaximumIntensity();
            volumeMapper.SetCropping(1);
            volumeMapper.SetCroppingRegionFlagsToSubVolume();

            //setDefaultColorMapping();
            //setDefaultOpacityfunction();

            widget.SetOutlineColor(0.93, 0.57, 0.13);
            widget.SetOrientationMarker(axes);
            widget.SetInteractor(renWindowControl.RenderWindow.GetInteractor());
            widget.SetEnabled(1);

            iren = renWindowControl.RenderWindow.GetInteractor();
        }

        public void updateChangeFilters()
        {
            if (false == IsVolumeRendererReady)
                return;

            double meanXYsize = Convert.ToDouble(DataExtentX + DataExtentY) / 2.0;
            double xy2z = meanXYsize / (DataSpacingZ * DataSpacingZMultiplier);

            if (changeFilters.Count > 0)
            {
                foreach (vtkImageChangeInformation changeFilter in changeFilters)
                {

                    if (_zMultiplierLow >= DataSpacingZMultiplier)
                    {
                        changeFilter.SetOutputSpacing(1, 1, DataSpacingZ * DataSpacingZMultiplier);
                    }
                    else
                    {
                        changeFilter.SetOutputSpacing(1, 1, DataSpacingZ * DataSpacingZMultiplier * _zSpacingTuner);
                    }
                }
                componentAdaptor.Update();
            }
        }

        public void updateCroppingRegionPlanes()
        {
            if (false == IsVolumeRendererReady)
            {
                return;
            }

            double meanXYExtent = Convert.ToDouble(DataExtentX + DataExtentY) / 2.0;
            double VTKDataExtentZ = (RenderedVolumeZMax - RenderedVolumeZMin) * DataSpacingZMultiplier * DataSpacingZ;
            double actualxy2zRatio;
            double xy2zRatioLow = 10 * Math.Log(meanXYExtent) / Math.Log(2);    // this lower bound is empirical, based on experiments

            if (0 != VTKDataExtentZ)
            {
                _xy2zRatio = meanXYExtent / VTKDataExtentZ;

                if (_xy2zRatio >= xy2zRatioLow)
                {
                    _zSpacingTuner = meanXYExtent / (xy2zRatioLow * RenderedVolumeZMax * DataSpacingZMultiplier * DataSpacingZ);
                }
                else
                {
                    _zSpacingTuner = 1;
                }
            }

            //volumeMapper.SetCroppingRegionPlanes(RenderedVolumeXMin, RenderedVolumeXMax,
            //                                         RenderedVolumeYMin, RenderedVolumeYMax,
            //                                         RenderedVolumeZMin * DataSpacingZMultiplier * DataSpacingZ,
            //                                         RenderedVolumeZMax * DataSpacingZMultiplier * DataSpacingZ);

            actualxy2zRatio = meanXYExtent / (RenderedVolumeZMax * DataSpacingZMultiplier * DataSpacingZ * _zSpacingTuner);

            volumeMapper.SetCroppingRegionPlanes(RenderedVolumeXMin, RenderedVolumeXMax,
                                                 RenderedVolumeYMin, RenderedVolumeYMax,
                                                 RenderedVolumeZMin * DataSpacingZMultiplier * DataSpacingZ,
                                                 RenderedVolumeZMax * DataSpacingZMultiplier * DataSpacingZ * _zSpacingTuner);
        }

        public void UpdateVolumeColor()
        {
            if (false == IsVolumeRendererReady)
                return;

            if (null == HardwareSettingsFile)
                return;
            SetApplicationColorMapping();
            ForceWindowToRender();
        }

        public void viewAllChannels()
        {
            SetApplicationColorMapping();

            ForceWindowToRender();
        }

        public void viewVolumeFromChannel(int ch)
        {
            //vtkVolumeProperty volProperty = volume.GetProperty();

            for (int i = 0; i < NumberOfComponents; i++)
            {
                if (i == ch)
                {
                    vtkColorTransferFunction colorFunction = vtkColorTransferFunction.New();
                    colorFunction.AddRGBSegment(lowerThreshold[i], 0, 0, 0, upperThreshold[i], 1, 1, 1);
                    volProperty.SetColor(i, colorFunction);
                }
                else
                {
                    vtkColorTransferFunction colorFunction = vtkColorTransferFunction.New();
                    colorFunction.AddRGBSegment(lowerThreshold[i], 0, 0, 0, upperThreshold[i], 0, 0, 0);
                    volProperty.SetColor(i, colorFunction);
                }
            }
            ForceWindowToRender();
        }

        public void VolumeRenderReset()
        {
            _resetView = true;
            _resetCamera = true;
            DataExtentX = -1;
            DataExtentY = -1;
            DataExtentZ = -1;

            _zSpacingTuner = 1;
            DataSpacingZMultiplier = 1;

            if (null != renderer)
                renderer.ResetCamera();

            if (null != camera)
                camera.Zoom(1);

            Reset3DView();
        }

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        static extern bool SetDllDirectory(string lpPathName);

        private void BuildNormalizationPalettes()
        {
            int shiftValue;
            double shiftValueResult;

            shiftValue = 6;
            shiftValueResult = Math.Pow(2, shiftValue);

            for (int i = 0; i < 4; i++)
            {
                _pal[i] = new byte[ushort.MaxValue + 1];
            }

            for (int j = 0; j < 4; j++)
            {
                for (int i = 0; i < ushort.MaxValue + 1; i++)
                {
                    double val = (255.0 / (shiftValueResult * (WhitePoints[j] - BlackPoints[j]))) * (i - BlackPoints[j] * shiftValueResult);
                    val = (val >= 0) ? val : 0;
                    val = (val <= 255) ? val : 255;

                    _pal[j][i] = (byte)val;
                }
            }
        }

        private void cameraAutoFocus()
        {
            if (camera != null)
            {
                camera.SetFocalPoint(
                                     (RenderedVolumeXMax + RenderedVolumeXMin) / 2,
                                     (RenderedVolumeYMax + RenderedVolumeYMin) / 2,
                                     (RenderedVolumeZMax + RenderedVolumeZMin) * DataSpacingZ * DataSpacingZMultiplier / 2
                                     );
            }
        }

        private void disposeChangeFilters()
        {
            if (changeFilters == null || changeFilters.Count == 0)
                return;
            foreach (vtkImageChangeInformation changeFilter in changeFilters)
            {
                changeFilter.Dispose();
            }
            changeFilters.Clear();
        }

        private void disposeFileNameArrays()
        {
            if (fileNameArrays == null || fileNameArrays.Count == 0)
                return;
            foreach (vtkStringArray strArray in fileNameArrays)
            {
                strArray.Dispose();
            }
            fileNameArrays.Clear();
        }

        private void disposeFlippers()
        {
            if (flippers == null || flippers.Count == 0)
                return;
            foreach (vtkImageFlip flip in flippers)
            {
                flip.Dispose();
            }
            flippers.Clear();
        }

        private void disposeReaders()
        {
            if (readers == null || readers.Count == 0)
                return;
            foreach (vtkTIFFReader reader in readers)
            {
                reader.Dispose();
            }
            readers.Clear();
        }

        private bool GetAttribute(XmlNode node, XmlDocument doc, string attrName, ref string attrValue)
        {
            bool ret;

            if (null == node.Attributes.GetNamedItem(attrName))
            {
                ret = false;
            }
            else
            {
                attrValue = node.Attributes[attrName].Value;
                ret = true;
            }

            return ret;
        }

        private void getChannelColors(string[] ch_col)
        {
            if ((!System.IO.File.Exists(HardwareSettingsFile)) || (null == HardwareSettingsFile) || (string.Empty == HardwareSettingsFile))
                return;
            if (volume == null)
            {
                MessageBox.Show("No volume exists");
                return;
            }
            if (ch_col.Length != 4) return;

            //MessageBox.Show(xmlFileName);
            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.Load(HardwareSettingsFile);

            XmlNodeList colorChannelsNode = xmlDoc.GetElementsByTagName("ColorChannels");
            int colorsCount = colorChannelsNode.Item(0).ChildNodes.Count;

            for (int i = 0; i < colorsCount; i++)
            {
                if (colorChannelsNode.Item(0).ChildNodes.Item(i).Attributes["name"].Value.Contains("ChanA"))
                {
                    ch_col[0] = colorChannelsNode.Item(0).ChildNodes.Item(i).Name;
                }
                if (colorChannelsNode.Item(0).ChildNodes.Item(i).Attributes["name"].Value.Contains("ChanB"))
                {
                    ch_col[1] = colorChannelsNode.Item(0).ChildNodes.Item(i).Name;
                }
                if (colorChannelsNode.Item(0).ChildNodes.Item(i).Attributes["name"].Value.Contains("ChanC"))
                {
                    ch_col[2] = colorChannelsNode.Item(0).ChildNodes.Item(i).Name;
                }
                if (colorChannelsNode.Item(0).ChildNodes.Item(i).Attributes["name"].Value.Contains("ChanD"))
                {
                    ch_col[3] = colorChannelsNode.Item(0).ChildNodes.Item(i).Name;
                }
            }
        }

        // get the file name based on the ZStreamMode
        string GetFileName(string folderPath, int channel, int well, int subWell, int z, int t)
        {
            string filename = folderPath + "\\";

            switch (channel)
            {
                case 0:
                    filename += "ChanA";
                    break;
                case 1:
                    filename += "ChanB";
                    break;
                case 2:
                    filename += "ChanC";
                    break;
                case 3:
                    filename += "ChanD";
                    break;
                default:
                    MessageBox.Show("No Channels Are Available");
                    break;
            }

            filename += String.Format("_{0:0000}", well);
            filename += String.Format("_{0:0000}", subWell);
            filename += String.Format("_{0:0000}", z);

            //use new naming scheme if z stream mode is turned on
            int tIndex = t;
            if ((ZStreamMode > 0) && (NumberOfZStream > 1))
            {
                tIndex = (t - 1) * DataExtentZ * NumberOfZStream + (z - 1) * NumberOfZStream + IndexOfZStream;
            }

            filename += String.Format("_{0:0000}", tIndex);
            filename += ".tif";

            return filename;
        }

        private int getLowerThreshold(int component, int ch)
        {
            if (component < 0 || component > NumberOfComponents)
            {
                MessageBox.Show("Bad Index - getLowerThreshold(int)");
                return -1;
            }
            return lowerThreshold[ch];
        }

        private int getUpperThreshold(int component, int ch)
        {
            if (component < 0 || component >= NumberOfComponents)
            {
                MessageBox.Show("Bad Index - getUpperThreshold(int)");
                return -1;
            }
            return upperThreshold[ch];
        }

        private void ParseScannerChoice(XmlDocument doc)
        {
            string str = string.Empty;
            int detectorType = 1;   //LSM by default

            XmlNodeList nodeList = doc.SelectNodes("/ThorImageExperiment/Modality");
            if (nodeList.Count > 0)
            {
                if (GetAttribute(nodeList[0], doc, "primaryDetectorType", ref str))
                {
                    detectorType = Int32.Parse(str);
                    if (1 == detectorType)
                    {
                        _scannerChoice = 0;
                    }
                    else if (0 == detectorType)
                    {
                        _scannerChoice = 1;
                    }
                    else
                    {
                        // this Mode is not supported yet.
                    }
                }
            }
        }

        /// <summary>
        /// This method creates the file pattern for vtkTIFFReader
        /// </summary>
        /// <param name="reader">vtkTIFFReader</param>
        /// <param name="channel">int, 0-ChanA, 1-ChanB, 2-ChanC, etc</param>
        /// <param name="well">int</param>
        /// <param name="tile">int</param>
        /// <param name="timeIndex">int</param>
        /// <returns>pattern in string</returns>
        private string produceFilePattern(int channel, int well, int tile, int timeIndex)
        {
            string pattern = "%s/";
            switch (channel)
            {
                case 0:
                    pattern += "ChanA";
                    break;
                case 1:
                    pattern += "ChanB";
                    break;
                case 2:
                    pattern += "ChanC";
                    break;
                case 3:
                    pattern += "ChanD";
                    break;
                default:
                    MessageBox.Show("No Channels Are Available");
                    break;
            }

            pattern += String.Format("_{0:0000}", well);
            pattern += String.Format("_{0:0000}", tile);
            pattern += "_%04d";
            pattern += String.Format("_{0:0000}", timeIndex);
            pattern += ".tif";

            return pattern;
        }

        private string produceTestFilePattern(int channel, int well, int tile, int timeIndex)
        {
            string pattern = "%s/";
            switch (channel)
            {
                case 0:
                    pattern += "ChanA";
                    break;
                case 1:
                    pattern += "ChanB";
                    break;
                case 2:
                    pattern += "ChanC";
                    break;
                case 3:
                    pattern += "ChanD";
                    break;
                default:
                    MessageBox.Show("No Channels Are Available");
                    break;
            }

            pattern += String.Format("_{0:0000}", well);
            pattern += String.Format("_{0:0000}", tile);
            pattern += "_%04d";
            //pattern += String.Format("_{0:0000}", timeIndex);
            pattern += ".tif";

            return pattern;
        }

        /// <summary>
        /// Given the location information and the timepoint (one time point), this method is able to collect data
        /// from all channels for the sample and renders the volume
        /// </summary>
        /// <param name="folderPath">specifies where the tiff images are</param>
        /// <param name="well">Well index to locate the files</param>
        /// <param name="tile">Tile index to locate the files</param>
        /// <param name="timeIndex">Time index to lcoate the files</param>
        private void readAVolume(string folderPath, int well, int tile, int timeIndex)
        {
            readVolumeChannels(folderPath, NumberOfComponents, well, tile, timeIndex);

            renderer = renWindowControl.RenderWindow.GetRenderers().GetFirstRenderer();
            renderer.RemoveAllViewProps();

            if (true == _isZStackDataExist)
            {
                volumeMapper.SetInputConnection(componentAdaptor.GetOutputPort());
                //volumeMapper.Update();

                volume.SetMapper(volumeMapper);
                //volume.SetOrigin(DataExtentX / 2, DataExtentY / 2, DataExtentZ / 2);

                renderer.AddVolume(volume);
                renderer.ResetCamera();
                camera = renderer.GetActiveCamera();
            }

            renderer.SetBackground(0, 0, 0);
            //renderer.GetActiveCamera().Zoom(3);
            //deleteAllVTKObjects();
        }

        /// <summary>
        /// This method should be used when rendering a volume, Dependency Properties, WellIndex,
        /// TileIndex, and TimePointIndex, would be used to specify the sources, make sure these
        /// properties have valid value before you use this method
        /// </summary>
        /// <param name="imageFolderPath">The folder path for all the images</param>
        private void readVolumeFromPath(string imageFolderPath)
        {
            if ((WellIndex > 0) && (TileIndex > 0) && (TimePointIndex > 0))
            {
                readAVolume(imageFolderPath, WellIndex, TileIndex, TimePointIndex);
            }
        }

        /// <summary>
        /// This function sets the color transfer function for the component specified
        /// </summary>
        /// <param name="componentIndex">index of the component, starting from 0</param>
        /// <param name="color">hexdecimal color value in string format, e.g. "#FF00FFFF" </param>
        private void setColorEx(int chIndex, string color, int min = 0, int max = MAX_14BIT, vtkColorTransferFunction colorFunction = null)
        {
            string str = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\luts\\" + color + ".txt";

            if (File.Exists(str))
            {
                if (colorFunction == null) colorFunction = vtkColorTransferFunction.New();
                StreamReader fs = new StreamReader(str);
                double[,] ci = new double[256, 3];
                string line;

                int counter = 0;
                try
                {
                    while ((line = fs.ReadLine()) != null)
                    {
                        string[] split = line.Split(',');

                        if (split[0] != null)
                        {
                            if (split[1] != null)
                            {
                                if (split[2] != null)
                                {
                                    int r = Convert.ToByte(split[0]);
                                    int g = Convert.ToByte(split[1]);
                                    int b = Convert.ToByte(split[2]);

                                    ci[counter, 0] = (double)r / MAX_8BIT;
                                    ci[counter, 1] = (double)g / MAX_8BIT;
                                    ci[counter, 2] = (double)b / MAX_8BIT;
                                }
                            }
                        }
                        counter++;
                        if (counter == 256) break;
                    }
                }
                catch (Exception ex)
                {
                    string msg = ex.Message;
                }

                fs.Close();

                colorFunction.RemoveAllPoints();
                if (max >= min)
                {
                    colorFunction.AddRGBSegment(0, ci[0, 0], ci[0, 1], ci[0, 2], Math.Max(0, min - 1), ci[0, 0], ci[0, 1], ci[0, 2]);
                    for (int i = 0; i < 256; i++)
                    {
                        colorFunction.AddRGBSegment(min + i * (max - min + 1.0) / 256, ci[i, 0], ci[i, 1], ci[i, 2], min + (i + 1.0) * (max - min + 1.0) / 256 - 1.0, ci[i, 0], ci[i, 1], ci[i, 2]);
                    }
                    colorFunction.AddRGBSegment(max + 1, ci[255, 0], ci[255, 1], ci[255, 2], 16384, ci[255, 0], ci[255, 1], ci[255, 2]);

                }
                else
                {

                    colorFunction.AddRGBSegment(0, ci[255, 0], ci[255, 1], ci[255, 2], Math.Max(0, max - 1), ci[255, 0], ci[255, 1], ci[255, 2]);
                    for (int i = 0; i < 256; i++)
                    {
                        colorFunction.AddRGBSegment(max + i * (min - max + 1.0) / 256, ci[255 - i, 0], ci[255 - i, 1], ci[255 - i, 2], max + (i + 1.0) * (min - max + 1.0) / 256 - 1.0, ci[255 - i, 0], ci[255 - i, 1], ci[255 - i, 2]);
                    }
                    colorFunction.AddRGBSegment(min + 1, ci[0, 0], ci[0, 1], ci[0, 2], 16384, ci[0, 0], ci[0, 1], ci[0, 2]);

                }

                volProperty.SetColor(_componentIndexForChannel[chIndex], colorFunction);
            }
        }

        private void setDefaultOpacityfunction()
        {
            if (volume == null)
            {
                MessageBox.Show("No volume exists");
                return;
            }
            //vtkVolumeProperty volProperty = volume.GetProperty();

            vtkPiecewiseFunction opacityFunctionR = vtkPiecewiseFunction.New();
            opacityFunctionR.AddSegment(0, 1, MaxGrayScaleLevel, 1);
            volProperty.SetScalarOpacity(0, opacityFunctionR);

            vtkPiecewiseFunction opacityFunctionG = vtkPiecewiseFunction.New();
            opacityFunctionG.AddSegment(0, 1, MaxGrayScaleLevel, 1);
            volProperty.SetScalarOpacity(1, opacityFunctionG);

            vtkPiecewiseFunction opacityFunctionB = vtkPiecewiseFunction.New();
            opacityFunctionB.AddSegment(0, 1, MaxGrayScaleLevel, 1);
            volProperty.SetScalarOpacity(2, opacityFunctionB);

            vtkPiecewiseFunction opacityFunctionA = vtkPiecewiseFunction.New();
            opacityFunctionA.AddSegment(0, 1, MaxGrayScaleLevel, 1);
            volProperty.SetScalarOpacity(3, opacityFunctionA);
        }

        private void setLowerThreshold(int component, int value, int ch)
        {
            if (component < 0 || component >= NumberOfComponents)
            {
                //MessageBox.Show("Bad Index - setLowerThreshold(int, int)");
                return;
            }
            updateColorMapping(component, value * MaxGrayScaleLevel / NumberOfColorBins, upperThreshold[ch]);
            lowerThreshold[ch] = value * MaxGrayScaleLevel / NumberOfColorBins;
        }

        private void setUpperThreshold(int component, int value, int ch)
        {
            if (component < 0 || component >= NumberOfComponents)
            {
                //MessageBox.Show("Bad Index - setUpperThreshold(int, int)");
                return;
            }
            updateColorMapping(component, lowerThreshold[ch], value * MaxGrayScaleLevel / NumberOfColorBins);
            upperThreshold[ch] = value * MaxGrayScaleLevel / NumberOfColorBins;
        }

        private void SetupScene()
        {
            renderer = renWindowControl.RenderWindow.GetRenderers().GetFirstRenderer();
            renderer.RemoveAllViewProps();

            if (_isZStackDataExist == true)
            {
                volumeMapper.SetInputConnection(componentAdaptor.GetOutputPort());
                //volumeMapper.Update();

                volume.SetMapper(volumeMapper);
                //volume.SetOrigin(DataExtentX / 2, DataExtentY / 2, DataExtentZ / 2);

                renderer.AddVolume(volume);

                if (_resetCamera)
                {
                    renderer.ResetCamera();
                    _resetCamera = false;
                }

                camera = renderer.GetActiveCamera();
            }

            renderer.SetBackground(0, 0, 0);
            //renderer.GetActiveCamera().Zoom(3);
            //deleteAllVTKObjects();
        }

        private void updateColorMapping(int component, int min, int max)
        {
            if (false == IsVolumeRendererReady)
            {
                //MessageBox.Show("No volume exists");
                return;
            }
            if (component < 0 || component >= NumberOfComponents)
            {
                MessageBox.Show("Bad Index - updateColorMapping(int, int, int)");
                return;
            }
            if (null == volProperty)
            {
                return;
            }

            string[] channel_color = new string[4];
            getChannelColors(channel_color);
            for (int i = 0; i < _componentIndexForChannel.Length; i++)
            {
                int tmp = _componentIndexForChannel[i];
                if ((component == tmp) && (0 < _isChannelSelected[i]))
                {
                    vtkColorTransferFunction ctf = volProperty.GetRGBTransferFunction(component);
                    setColorEx(i, channel_color[i], min, max, ctf);
                }
            }
            ForceWindowToRender();
        }

        void VolumeRenderer_Loaded(object sender, RoutedEventArgs e)
        {
            _isVtkPipelineReady = true;
            BuildNormalizationPalettes();
        }

        void VolumeRenderer_Unloaded(object sender, RoutedEventArgs e)
        {
            _isVtkPipelineReady = false;
        }

        #endregion Methods
    }
}