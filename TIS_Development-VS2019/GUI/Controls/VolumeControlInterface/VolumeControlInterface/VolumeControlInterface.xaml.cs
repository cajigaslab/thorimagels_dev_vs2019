namespace VolumeControlInterface
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Text.RegularExpressions;
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

    using FolderDialogControl;

    /// <summary>
    /// Interaction logic for VolumeControl.xaml
    /// </summary>
    public partial class VolumeControlInterface : UserControl, INotifyPropertyChanged
    {
        #region Fields

        public static DependencyProperty ApplicationSettingsDirectoryProperty = 
             DependencyProperty.RegisterAttached("ApplicationSettingsDirectory",
             typeof(string),
             typeof(VolumeControlInterface));
        public static DependencyProperty BlackPoint0Property = 
                      DependencyProperty.RegisterAttached("BlackPoint0",
                      typeof(int),
                      typeof(VolumeControlInterface));
        public static DependencyProperty BlackPoint1Property = 
                      DependencyProperty.RegisterAttached("BlackPoint1",
                      typeof(int),
                      typeof(VolumeControlInterface));
        public static DependencyProperty BlackPoint2Property = 
                      DependencyProperty.RegisterAttached("BlackPoint2",
                      typeof(int),
                      typeof(VolumeControlInterface));
        public static DependencyProperty BlackPoint3Property = 
                      DependencyProperty.RegisterAttached("BlackPoint3",
                      typeof(int),
                      typeof(VolumeControlInterface));
        public static DependencyProperty DataExtentXProperty = 
                          DependencyProperty.RegisterAttached("DataExtentX",
                          typeof(int),
                          typeof(VolumeControlInterface));
        public static DependencyProperty DataExtentYProperty = 
                          DependencyProperty.RegisterAttached("DataExtentY",
                          typeof(int),
                          typeof(VolumeControlInterface));
        public static DependencyProperty DataExtentZProperty = 
                          DependencyProperty.RegisterAttached("DataExtentZ",
                          typeof(int),
                          typeof(VolumeControlInterface));
        public static DependencyProperty DataSpacingZMultiplierProperty = 
                        DependencyProperty.RegisterAttached("DataSpacingZMultiplier",
                        typeof(double),
                        typeof(VolumeControlInterface),
                        new FrameworkPropertyMetadata(1.0), new ValidateValueCallback(IsValuePositive));
        public static DependencyProperty DataSpacingZProperty = 
                        DependencyProperty.RegisterAttached("DataSpacingZ",
                        typeof(double),
                        typeof(VolumeControlInterface));
        public static DependencyProperty HardwareSettingsFileProperty = 
                        DependencyProperty.RegisterAttached("HardwareSettingsFile",
                        typeof(string),
                        typeof(VolumeControlInterface));
        public static DependencyProperty IsChanASelectedProperty = 
             DependencyProperty.RegisterAttached("IsChanASelected",
             typeof(bool),
             typeof(VolumeControlInterface),
             new FrameworkPropertyMetadata(true));
        public static DependencyProperty IsChanBSelectedProperty = 
             DependencyProperty.RegisterAttached("IsChanBSelected",
             typeof(bool),
             typeof(VolumeControlInterface),
             new FrameworkPropertyMetadata(true));
        public static DependencyProperty IsChanCSelectedProperty = 
             DependencyProperty.RegisterAttached("IsChanCSelected",
             typeof(bool),
             typeof(VolumeControlInterface),
             new FrameworkPropertyMetadata(true));
        public static DependencyProperty IsChanDSelectedProperty = 
             DependencyProperty.RegisterAttached("IsChanDSelected",
             typeof(bool),
             typeof(VolumeControlInterface),
             new FrameworkPropertyMetadata(true));
        public static DependencyProperty OutputDirectoryProperty = 
                       DependencyProperty.RegisterAttached("OutputDirectory",
                       typeof(string),
                       typeof(VolumeControlInterface));
        public static DependencyProperty OutputExperimentProperty = 
                       DependencyProperty.RegisterAttached("OutputExperiment",
                       typeof(string),
                       typeof(VolumeControlInterface));
        public static DependencyProperty RenderedVolumeXMaxProperty = 
                          DependencyProperty.RegisterAttached("RenderedVolumeXMax",
                          typeof(int),
                          typeof(VolumeControlInterface));
        public static DependencyProperty RenderedVolumeXMinProperty = 
                           DependencyProperty.RegisterAttached("RenderedVolumeXMin",
                           typeof(int),
                           typeof(VolumeControlInterface));
        public static DependencyProperty RenderedVolumeYMaxProperty = 
                          DependencyProperty.RegisterAttached("RenderedVolumeYMax",
                          typeof(int),
                          typeof(VolumeControlInterface));
        public static DependencyProperty RenderedVolumeYMinProperty = 
                          DependencyProperty.RegisterAttached("RenderedVolumeYMin",
                          typeof(int),
                          typeof(VolumeControlInterface));
        public static DependencyProperty RenderedVolumeZMaxProperty = 
                          DependencyProperty.RegisterAttached("RenderedVolumeZMax",
                          typeof(int),
                          typeof(VolumeControlInterface));
        public static DependencyProperty RenderedVolumeZMinProperty = 
                          DependencyProperty.RegisterAttached("RenderedVolumeZMin",
                          typeof(int),
                          typeof(VolumeControlInterface));
        public static DependencyProperty TileIndexProperty = 
                      DependencyProperty.RegisterAttached("TileIndex",
                      typeof(int),
                      typeof(VolumeControlInterface), new FrameworkPropertyMetadata(1));
        public static DependencyProperty TimePointIndexProperty = 
                     DependencyProperty.RegisterAttached("TimePointIndex",
                     typeof(int),
                     typeof(VolumeControlInterface), new FrameworkPropertyMetadata(1));
        public static DependencyProperty TimepointsProperty = 
                     DependencyProperty.RegisterAttached("Timepoints",
                     typeof(int),
                     typeof(VolumeControlInterface), new FrameworkPropertyMetadata(1));
        public static DependencyProperty TotalSystemChannelsProperty = 
                    DependencyProperty.RegisterAttached("TotalSystemChannels",
                    typeof(int),
                    typeof(VolumeControlInterface), new FrameworkPropertyMetadata(4));
        public static DependencyProperty WellIndexProperty = 
                      DependencyProperty.RegisterAttached("WellIndex",
                      typeof(int),
                      typeof(VolumeControlInterface), new FrameworkPropertyMetadata(1));
        public static DependencyProperty WhitePoint0Property = 
                      DependencyProperty.RegisterAttached("WhitePoint0",
                      typeof(int),
                      typeof(VolumeControlInterface));
        public static DependencyProperty WhitePoint1Property = 
                      DependencyProperty.RegisterAttached("WhitePoint1",
                      typeof(int),
                      typeof(VolumeControlInterface));
        public static DependencyProperty WhitePoint2Property = 
                      DependencyProperty.RegisterAttached("WhitePoint2",
                      typeof(int),
                      typeof(VolumeControlInterface));
        public static DependencyProperty WhitePoint3Property = 
                      DependencyProperty.RegisterAttached("WhitePoint3",
                      typeof(int),
                      typeof(VolumeControlInterface));
        public static DependencyProperty ZStackCacheDirectoryProperty = 
                        DependencyProperty.RegisterAttached("ZStackCacheDirectory",
                        typeof(string),
                        typeof(VolumeControlInterface));
        public static DependencyProperty ZStreamIndexProperty = 
             DependencyProperty.RegisterAttached("ZStreamIndex",
             typeof(int),
             typeof(VolumeControlInterface), new FrameworkPropertyMetadata(1));

        private int _renderedVolumeXMax;
        private int _renderedVolumeXMin;
        private int _renderedVolumeYMax;
        private int _renderedVolumeYMin;
        private int _renderedVolumeZMax;
        private int _renderedVolumeZMin;
        private bool _resetView;
        private DispatcherTimer _VolumeControlInterfaceTimer;

        #endregion Fields

        #region Constructors

        public VolumeControlInterface()
        {
            InitializeComponent();
            //if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this) == true)
            //    return;
            if (!System.ComponentModel.DesignerProperties.GetIsInDesignMode(this))
            {
                _VolumeControlInterfaceTimer = new DispatcherTimer();
                _VolumeControlInterfaceTimer.Interval = TimeSpan.FromSeconds(0.5);

                this.Loaded += new RoutedEventHandler(VolumeControlInterface_Loaded);
                this.Unloaded += new RoutedEventHandler(VolumeControlInterface_Unloaded);
                this.KeyDown += VolumeControlInterface_KeyDown;
                PropertyChanged += interface_PropertyChanged;
                txtZSpacingMultiplier.TargetUpdated += btnZMultiplierOk_Click;
                txtZSpacingMultiplier.SourceUpdated += btnZMultiplierOk_Click;
            }

            _resetView = false;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

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

        public int BlackPoint0
        {
            get
            {
                return (int)GetValue(BlackPoint0Property);
            }
            set
            {
                SetValue(BlackPoint0Property, value);
            }
        }

        public int BlackPoint1
        {
            get
            {
                return (int)GetValue(BlackPoint1Property);
            }
            set
            {
                SetValue(BlackPoint1Property, value);
            }
        }

        public int BlackPoint2
        {
            get
            {
                return (int)GetValue(BlackPoint2Property);
            }
            set
            {
                SetValue(BlackPoint2Property, value);
            }
        }

        public int BlackPoint3
        {
            get
            {
                return (int)GetValue(BlackPoint3Property);
            }
            set
            {
                SetValue(BlackPoint3Property, value);
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

                if (0 == RenderedVolumeXMax || RenderedVolumeXMax != DataExtentX)
                {
                    RenderedVolumeXMax = DataExtentX;
                }
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

                if (0 == RenderedVolumeYMax || RenderedVolumeYMax != DataExtentY)
                {
                    RenderedVolumeYMax = DataExtentY;
                }
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

                if (RenderedVolumeZMax == 0)
                {
                    RenderedVolumeZMax = DataExtentZ;
                }
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
                VolumeControl.ForceWindowToRender();
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

        public bool IsChanASelected
        {
            get
            {
                return (bool)GetValue(IsChanASelectedProperty);
            }
            set
            {
                SetValue(IsChanASelectedProperty, value);
            }
        }

        public bool IsChanBSelected
        {
            get
            {
                return (bool)GetValue(IsChanBSelectedProperty);
            }
            set
            {
                SetValue(IsChanBSelectedProperty, value);
            }
        }

        public bool IsChanCSelected
        {
            get
            {
                return (bool)GetValue(IsChanCSelectedProperty);
            }
            set
            {
                SetValue(IsChanCSelectedProperty, value);
            }
        }

        public bool IsChanDSelected
        {
            get
            {
                return (bool)GetValue(IsChanDSelectedProperty);
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
                return VolumeControl.IsVolumeRendererReady;
            }
        }

        public String OutputDirectory
        {
            get
            {
                return (string)GetValue(OutputDirectoryProperty);
            }
            set
            {
                SetValue(OutputDirectoryProperty, value);
            }
        }

        public String OutputExperiment
        {
            get
            {
                return (string)GetValue(OutputExperimentProperty);
            }
            set
            {
                SetValue(OutputExperimentProperty, value);
            }
        }

        public int RenderedVolumeXMax
        {
            get
            {
                _renderedVolumeXMax = Convert.ToInt32(SliderXMax.Value);
                SetValue(RenderedVolumeXMaxProperty, _renderedVolumeXMax);
                return _renderedVolumeXMax;
            }
            set
            {
                SliderXMax.Value = value;
                SetValue(RenderedVolumeXMaxProperty, value);
            }
        }

        public int RenderedVolumeXMin
        {
            get
            {
                _renderedVolumeXMin = Convert.ToInt32(SliderXMin.Value);
                SetValue(RenderedVolumeXMinProperty, _renderedVolumeXMin);
                return _renderedVolumeXMin;
            }
            set
            {
                SliderXMin.Value = value;
                SetValue(RenderedVolumeXMinProperty, value);
            }
        }

        public int RenderedVolumeYMax
        {
            get
            {
                _renderedVolumeYMax = Convert.ToInt32(SliderYMax.Value);
                SetValue(RenderedVolumeYMaxProperty, _renderedVolumeYMax);
                return _renderedVolumeYMax;
            }
            set
            {
                SliderYMax.Value = value;
                SetValue(RenderedVolumeYMaxProperty, value);
            }
        }

        public int RenderedVolumeYMin
        {
            get
            {
                _renderedVolumeYMin = Convert.ToInt32(SliderYMin.Value);
                SetValue(RenderedVolumeYMinProperty, _renderedVolumeYMin);
                return _renderedVolumeYMin;
            }
            set
            {
                SliderYMin.Value = value;
                SetValue(RenderedVolumeYMinProperty, value);
            }
        }

        public int RenderedVolumeZMax
        {
            get
            {
                _renderedVolumeZMax = Convert.ToInt32(SliderZMax.Value);
                SetValue(RenderedVolumeZMaxProperty, _renderedVolumeZMax);
                return _renderedVolumeZMax;
            }
            set
            {
                SliderZMax.Value = value;
                SetValue(RenderedVolumeZMaxProperty, value);
            }
        }

        public int RenderedVolumeZMin
        {
            get
            {
                _renderedVolumeZMin = Convert.ToInt32(SliderZMin.Value);
                SetValue(RenderedVolumeZMinProperty, _renderedVolumeZMin);
                return _renderedVolumeZMin;
            }
            set
            {
                SliderZMin.Value = value;
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

        public int WhitePoint0
        {
            get
            {
                return (int)GetValue(WhitePoint0Property);
            }
            set
            {
                SetValue(WhitePoint0Property, value);
            }
        }

        public int WhitePoint1
        {
            get
            {
                return (int)GetValue(WhitePoint1Property);
            }
            set
            {
                SetValue(WhitePoint1Property, value);
            }
        }

        public int WhitePoint2
        {
            get
            {
                return (int)GetValue(WhitePoint2Property);
            }
            set
            {
                SetValue(WhitePoint2Property, value);
            }
        }

        public int WhitePoint3
        {
            get
            {
                return (int)GetValue(WhitePoint3Property);
            }
            set
            {
                SetValue(WhitePoint3Property, value);
            }
        }

        public string ZStackCacheDirectory
        {
            get
            {
                return (string)GetValue(ZStackCacheDirectoryProperty);
            }
            set
            {
                SetValue(ZStackCacheDirectoryProperty, value);
            }
        }

        public int ZStreamIndex
        {
            get
            {
                return (int)GetValue(ZStreamIndexProperty);
            }
            set
            {
                SetValue(ZStreamIndexProperty, value);
            }
        }

        #endregion Properties

        #region Methods

        public void RenderVolume()
        {
            VolumeControl.ResetView = _resetView;
            VolumeControl.RenderVolume();
        }

        public void UpdateVolumeColor()
        {
            VolumeControl.UpdateVolumeColor();
        }

        private static bool DirectoryCopy(string sourceDirName, string destDirName, bool copySubDirs)
        {
            // If the source directory does not exist, let the user know and return false.
            if (!Directory.Exists(sourceDirName))
            {
                MessageBox.Show(
                    "Source directory does not exist or could not be found. Please ensure you have chosen an experiment to review (if you are in Experiment Review) or you have previewed a Z Stack (if you are in Capture Setup).", sourceDirName, MessageBoxButton.OK, MessageBoxImage.Error);
                return false;
            }

            DirectoryInfo dir = new DirectoryInfo(sourceDirName);
            DirectoryInfo[] dirs = dir.GetDirectories();

            // If the destination directory does not exist, create it.
            if (!Directory.Exists(destDirName))
            {
                Directory.CreateDirectory(destDirName);
            }

            // Get the file contents of the directory to copy.
            FileInfo[] files = dir.GetFiles();

            foreach (FileInfo file in files)
            {
                // Create the path to the new copy of the file.
                string temppath = System.IO.Path.Combine(destDirName, file.Name);

                // Copy the file.
                file.CopyTo(temppath, false);
            }

            // If copySubDirs is true, copy the subdirectories.
            if (copySubDirs)
            {

                foreach (DirectoryInfo subdir in dirs)
                {
                    // Create the subdirectory.
                    string temppath = System.IO.Path.Combine(destDirName, subdir.Name);

                    // Copy the subdirectories.
                    DirectoryCopy(subdir.FullName, temppath, copySubDirs);
                }
            }
            return true;
        }

        private static bool IsValuePositive(object value)
        {
            if (null == value)
            {
                return false;
            }
            else if (Convert.ToDouble(value) < 0)
            {
                return false;
            }
            return true;
        }

        private void Browse_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog ofd = new Microsoft.Win32.OpenFileDialog();
            ofd.Title = "Select a destination folder for the experiment";
            ofd.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString();
            ofd.ValidateNames = false;
            ofd.CheckFileExists = false;
            ofd.FileName = "Folder Selection";
            try
            {
                if (true == ofd.ShowDialog())
                {
                    OutputDirectory = System.IO.Path.GetDirectoryName(ofd.FileName);
                }
            }
            catch { }
        }

        private void btnABPMinus_Click(object sender, RoutedEventArgs e)
        {
            if (BlackPoint0 > SliderLowerChanA.Minimum)
            {
                BlackPoint0--;
            }

            OnPropertyChanged("BlackPoint0");
        }

        private void btnABPPlus_Click(object sender, RoutedEventArgs e)
        {
            if (BlackPoint0 < SliderLowerChanA.Maximum)
            {
                BlackPoint0++;
            }

            OnPropertyChanged("BlackPoint0");
        }

        private void btnAWPMinus_Click(object sender, RoutedEventArgs e)
        {
            if (WhitePoint0 > SliderUpperChanA.Minimum)
            {
                WhitePoint0--;
            }

            OnPropertyChanged("WhitePoint0");
        }

        private void btnAWPPlus_Click(object sender, RoutedEventArgs e)
        {
            if (WhitePoint0 < SliderUpperChanA.Maximum)
            {
                WhitePoint0++;
            }

            OnPropertyChanged("WhitePoint0");
        }

        private void btnBBPMinus_Click(object sender, RoutedEventArgs e)
        {
            if (BlackPoint1 > SliderLowerChanB.Minimum)
            {
                BlackPoint1--;
            }

            OnPropertyChanged("BlackPoint1");
        }

        private void btnBBPPlus_Click(object sender, RoutedEventArgs e)
        {
            if (BlackPoint1 < SliderLowerChanB.Maximum)
            {
                BlackPoint1++;
            }

            OnPropertyChanged("BlackPoint1");
        }

        private void btnBWPMinus_Click(object sender, RoutedEventArgs e)
        {
            if (WhitePoint1 > SliderUpperChanB.Minimum)
            {
                WhitePoint1--;
            }

            OnPropertyChanged("WhitePoint1");
        }

        private void btnBWPPlus_Click(object sender, RoutedEventArgs e)
        {
            if (WhitePoint1 < SliderUpperChanB.Maximum)
            {
                WhitePoint1++;
            }

            OnPropertyChanged("WhitePoint1");
        }

        private void btnCBPMinus_Click(object sender, RoutedEventArgs e)
        {
            if (BlackPoint2 > SliderLowerChanC.Minimum)
            {
                BlackPoint2--;
            }

            OnPropertyChanged("BlackPoint2");
        }

        private void btnCBPPlus_Click(object sender, RoutedEventArgs e)
        {
            if (BlackPoint2 < SliderLowerChanC.Maximum)
            {
                BlackPoint2++;
            }

            OnPropertyChanged("BlackPoint2");
        }

        private void btnCWPMinus_Click(object sender, RoutedEventArgs e)
        {
            if (WhitePoint2 > SliderUpperChanC.Minimum)
            {
                WhitePoint2--;
            }

            OnPropertyChanged("WhitePoint2");
        }

        private void btnCWPPlus_Click(object sender, RoutedEventArgs e)
        {
            if (WhitePoint2 < SliderUpperChanC.Maximum)
            {
                WhitePoint2++;
            }

            OnPropertyChanged("WhitePoint2");
        }

        private void btnDBPMinus_Click(object sender, RoutedEventArgs e)
        {
            if (BlackPoint3 > SliderLowerChanD.Minimum)
            {
                BlackPoint3--;
            }

            OnPropertyChanged("BlackPoint3");
        }

        private void btnDBPPlus_Click(object sender, RoutedEventArgs e)
        {
            if (BlackPoint3 < SliderLowerChanD.Maximum)
            {
                BlackPoint3++;
            }

            OnPropertyChanged("BlackPoint3");
        }

        private void btnDownArrow_Click(object sender, RoutedEventArgs e)
        {
            VolumeControl.RotateToDown();
        }

        private void btnDWPMinus_Click(object sender, RoutedEventArgs e)
        {
            if (WhitePoint3 > SliderUpperChanD.Minimum)
            {
                WhitePoint3--;
            }

            OnPropertyChanged("WhitePoint3");
        }

        private void btnDWPPlus_Click(object sender, RoutedEventArgs e)
        {
            if (WhitePoint3 < SliderUpperChanD.Maximum)
            {
                WhitePoint3++;
            }

            OnPropertyChanged("WhitePoint3");
        }

        private void btnLeftArrow_Click(object sender, RoutedEventArgs e)
        {
            VolumeControl.RotateToLeft();
        }

        // reset 3D crop sliders
        private void btnResetCrop3D_Click(object sender, RoutedEventArgs e)
        {
            VolumeControl.Reset3DView();

            SliderXMin.Value = SliderXMin.Minimum;
            SliderXMax.Value = SliderXMax.Maximum;
            SliderYMin.Value = SliderYMin.Minimum;
            SliderYMax.Value = SliderYMax.Maximum;
            SliderZMin.Value = SliderZMin.Minimum;
            SliderZMax.Value = SliderZMax.Maximum;
            OnPropertyChanged("RenderedVolume");
        }

        private void btnRightArrow_Click(object sender, RoutedEventArgs e)
        {
            VolumeControl.RotateToRight();
        }

        private void btnUpArrow_Click(object sender, RoutedEventArgs e)
        {
            VolumeControl.RotateToUp();
        }

        private void btnXMaxCrop3DMinus_Click(object sender, RoutedEventArgs e)
        {
            if (RenderedVolumeXMax > RenderedVolumeXMin)
            {
                RenderedVolumeXMax--;
            }

            OnPropertyChanged("RenderedVolumeXMax");
        }

        private void btnXMaxCrop3DPlus_Click(object sender, RoutedEventArgs e)
        {
            if (RenderedVolumeXMax < Convert.ToInt32(SliderXMax.Maximum))
            {
                RenderedVolumeXMax++;
            }

            OnPropertyChanged("RenderedVolumeXMax");
        }

        private void btnXMinCrop3DMinus_Click(object sender, RoutedEventArgs e)
        {
            if (RenderedVolumeXMin > 0)
            {
                RenderedVolumeXMin--;
            }

            OnPropertyChanged("RenderedVolumeXMin");
        }

        // 3D crop +/- button events
        private void btnXMinCrop3DPlus_Click(object sender, RoutedEventArgs e)
        {
            if (RenderedVolumeXMin < RenderedVolumeXMax)
            {
                RenderedVolumeXMin++;
            }

            OnPropertyChanged("RenderedVolumeXMin");
        }

        private void btnYMaxCrop3DMinus_Click(object sender, RoutedEventArgs e)
        {
            if (RenderedVolumeYMax > RenderedVolumeYMin)
            {
                RenderedVolumeYMax--;
            }

            OnPropertyChanged("RenderedVolumeYMax");
        }

        private void btnYMaxCrop3DPlus_Click(object sender, RoutedEventArgs e)
        {
            if (RenderedVolumeYMax < Convert.ToInt32(SliderYMax.Maximum))
            {
                RenderedVolumeYMax++;
            }

            OnPropertyChanged("RenderedVolumeYMax");
        }

        private void btnYMinCrop3DMinus_Click(object sender, RoutedEventArgs e)
        {
            if (RenderedVolumeYMin > 0)
            {
                RenderedVolumeYMin--;
            }

            OnPropertyChanged("RenderedVolumeYMin");
        }

        private void btnYMinCrop3DPlus_Click(object sender, RoutedEventArgs e)
        {
            if (RenderedVolumeYMin < RenderedVolumeYMax)
            {
                RenderedVolumeYMin++;
            }
            OnPropertyChanged("RenderedVolumeYMin");
        }

        private void btnZMaxCrop3DMinus_Click(object sender, RoutedEventArgs e)
        {
            if (RenderedVolumeZMax > RenderedVolumeZMin)
            {
                RenderedVolumeZMax--;
            }

            OnPropertyChanged("RenderedVolumeZMax");
        }

        private void btnZMaxCrop3DPlus_Click(object sender, RoutedEventArgs e)
        {
            if (RenderedVolumeZMax < Convert.ToInt32(SliderZMax.Maximum))
            {
                RenderedVolumeZMax++;
            }

            OnPropertyChanged("RenderedVolumeZMax");
        }

        private void btnZMinCrop3DMinus_Click(object sender, RoutedEventArgs e)
        {
            if (RenderedVolumeZMin > 0)
            {
                RenderedVolumeZMin--;
            }

            OnPropertyChanged("RenderedVolumeZMin");
        }

        private void btnZMinCrop3DPlus_Click(object sender, RoutedEventArgs e)
        {
            if (RenderedVolumeZMin < RenderedVolumeZMax)
            {
                RenderedVolumeZMin++;
            }

            OnPropertyChanged("RenderedVolumeZMin"); ;
        }

        private void btnZMultiplierMinus_Click(object sender, RoutedEventArgs e)
        {
            if (1 < DataSpacingZMultiplier)
            {
                DataSpacingZMultiplier = Math.Ceiling(DataSpacingZMultiplier - 1);
                //VolumeControl.ForceWindowToRender();
            }
        }

        private void btnZMultiplierOk_Click(object sender, RoutedEventArgs e)
        {
            VolumeControl.ForceWindowToRender();
        }

        private void btnZMultiplierPlus_Click(object sender, RoutedEventArgs e)
        {
            DataSpacingZMultiplier = Math.Floor(DataSpacingZMultiplier + 1);
            //VolumeControl.ForceWindowToRender();
        }

        private string CreateUniqueName(string str)
        {
            Match match = Regex.Match(str, "(.*)([0-9]{3,})");

            if (match.Groups.Count > 2)
            {
                int val = Convert.ToInt32(match.Groups[2].Value);

                val++;

                str = match.Groups[1].Value + String.Format("{0:000}", val);
            }
            else
            {
                str = str + "001";
            }

            return str;
        }

        private void interface_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (!e.PropertyName.Contains("OutputDirectory"))
            {
                if (e.PropertyName.Contains("RenderedVolume"))
                {
                    VolumeControl.updateCroppingRegionPlanes();
                    VolumeControl.ForceWindowToRender();
                }
                else if (e.PropertyName.CompareTo("DataSpacingZMultiplier") == 0)
                {
                    VolumeControl.ForceWindowToRender();
                }
            }
            return;
        }

        private void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        private void Save_Click(object sender, RoutedEventArgs e)
        {
            string desPath = System.IO.Path.Combine(OutputDirectory, OutputExperiment);
            if (Directory.Exists(desPath.ToString()))
            {
                string str = OutputExperiment;
                do
                {
                    str = CreateUniqueName(str);
                }
                while (Directory.Exists(OutputDirectory + "\\" + str));

                string msg = string.Format("A data set already exists at this location. Would you like to use the name {0} instead?", str);
                if (MessageBoxResult.No == MessageBox.Show(msg, "Data already exists", MessageBoxButton.YesNo))
                {
                    return;
                }
                OutputExperiment = str;
                desPath = System.IO.Path.Combine(OutputDirectory, OutputExperiment);
            }
            if (DirectoryCopy(ZStackCacheDirectory, desPath, true))
            {
                MessageBox.Show("Your Experiment '" + OutputExperiment + "' is saved!");
            }

            /// node.Attributes[attrName].Value = attrValue;
        }

        private void SliderCroppingXMaxRegion_ValueChanged(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
        {
            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this))
                return;
            RenderedVolumeXMax = Convert.ToInt32(((Slider)sender).Value);
            VolumeControl.updateCroppingRegionPlanes();
            VolumeControl.ForceWindowToRender();
        }

        // 3D crop slider value change events
        private void SliderCroppingXMinRegion_ValueChanged(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
        {
            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this))
                return;
            RenderedVolumeXMin = Convert.ToInt32(((Slider)sender).Value);
            VolumeControl.updateCroppingRegionPlanes();
            VolumeControl.ForceWindowToRender();
        }

        private void SliderCroppingYMaxRegion_ValueChanged(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
        {
            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this))
                return;
            _renderedVolumeYMax = Convert.ToInt32(((Slider)sender).Value);
            VolumeControl.updateCroppingRegionPlanes();
            VolumeControl.ForceWindowToRender();
        }

        private void SliderCroppingYMinRegion_ValueChanged(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
        {
            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this))
                return;
            RenderedVolumeYMin = Convert.ToInt32(((Slider)sender).Value);
            VolumeControl.updateCroppingRegionPlanes();
            VolumeControl.ForceWindowToRender();
        }

        private void SliderCroppingZMaxRegion_ValueChanged(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
        {
            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this))
                return;
            RenderedVolumeZMax = Convert.ToInt32(((Slider)sender).Value);
            VolumeControl.updateCroppingRegionPlanes();
            VolumeControl.ForceWindowToRender();
        }

        private void SliderCroppingZMinRegion_ValueChanged(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
        {
            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this))
                return;

            RenderedVolumeZMin = Convert.ToInt32(((Slider)sender).Value);
            VolumeControl.updateCroppingRegionPlanes();
            VolumeControl.ForceWindowToRender();
        }

        void VolumeControlInterface_KeyDown(object sender, KeyEventArgs e)
        {
            //filter for the enter or return key
            if ((e.Key == Key.Enter) || (e.Key == Key.Return))
            {
                e.Handled = true;
                TraversalRequest trNext = new TraversalRequest(FocusNavigationDirection.Next);

                UIElement keyboardFocus = (UIElement)Keyboard.FocusedElement;

                //move the focus to the next element
                if (keyboardFocus != null)
                {
                    if (keyboardFocus.GetType() == typeof(TextBox))
                    {
                        keyboardFocus.MoveFocus(trNext);
                    }
                }
            }
        }

        void VolumeControlInterface_Loaded(object sender, RoutedEventArgs e)
        {
            _VolumeControlInterfaceTimer.Tick += new EventHandler(_VolumeControlInterfaceTimer_Tick);
            _VolumeControlInterfaceTimer.Start();
        }

        void VolumeControlInterface_Unloaded(object sender, RoutedEventArgs e)
        {
            _VolumeControlInterfaceTimer.Stop();
            _VolumeControlInterfaceTimer.Tick -= new EventHandler(_VolumeControlInterfaceTimer_Tick);
        }

        void _VolumeControlInterfaceTimer_Tick(object sender, EventArgs e)
        {
            OnPropertyChanged("OutputDirectory");
            OnPropertyChanged("OutputExperiment");
        }

        #endregion Methods
    }
}