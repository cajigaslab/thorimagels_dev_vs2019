namespace CaptureSetupDll.View
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
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
    using System.Xml;

    using CaptureSetupDll.ViewModel;

    using GeometryUtilities;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    using Validations;

    public class CenterBleachParams
    {
        #region Fields

        double _dwellTime;
        double _power;

        #endregion Fields

        #region Constructors

        public CenterBleachParams(double pwr, double dwTime)
        {
            _power = pwr;
            _dwellTime = dwTime;
        }

        #endregion Constructors

        #region Properties

        public double DwellTime
        {
            get { return _dwellTime; }
            set { _dwellTime = value; }
        }

        public double Power
        {
            get { return _power; }
            set { _power = value; }
        }

        #endregion Properties

        #region Methods

        bool CompareParams(CenterBleachParams paramIn)
        {
            bool ret = true;
            if ((_power != paramIn._power) ||
                (_dwellTime != paramIn._dwellTime))
            {
                return false;
            }
            return ret;
        }

        #endregion Methods
    }

    public class SLMBrowsePanel
    {
        #region Constructors

        public SLMBrowsePanel(string browse, string ok, string cancel, string path)
        {
            Browse = browse; Import = ok; Cancel = cancel; ImportFilePath = path;
        }

        #endregion Constructors

        #region Properties

        public string Browse
        {
            get;
            set;
        }

        public string Cancel
        {
            get;
            set;
        }

        public string Import
        {
            get;
            set;
        }

        public string ImportFilePath
        {
            get;
            set;
        }

        #endregion Properties
    }

    public class SLMCalibPanel
    {
        #region Constructors

        public SLMCalibPanel(string calibYes, string calibNo, string title = null, string body = null)
        {
            CalibYes = calibYes; CalibNo = calibNo; CalibTitle = title; CalibBody = body;
        }

        #endregion Constructors

        #region Properties

        public string CalibBody
        {
            get;
            set;
        }

        public string CalibNo
        {
            get;
            set;
        }

        public string CalibTitle
        {
            get;
            set;
        }

        public string CalibYes
        {
            get;
            set;
        }

        #endregion Properties
    }

    /// <summary>
    /// Interaction logic for SLMParamEditWin.xaml
    /// </summary>
    public partial class SLMParamEditWin : Window
    {
        #region Fields

        public static Dictionary<string, SLMPatternType> SLMPatternTypeDictionary = new Dictionary<string, SLMPatternType>()
        {
        {"SLM_PATTERN_ADD", SLMPatternType.Add},
        {"SLM_PATTERN_EDIT", SLMPatternType.Edit},
        {"SLM_PATTERN_DELETE", SLMPatternType.Delete},
        {"SLM_PATTERN_SHOWHIDE", SLMPatternType.ShowHide},
        {"SLM_EXECUTE", SLMPatternType.Execute},
        {"SLM_CALIBRATION", SLMPatternType.Calibration},
        {"SLM_IMPORT", SLMPatternType.Import},
        {"SLM_BUILD", SLMPatternType.Build},
        {"SLM_BLANK", SLMPatternType.Blank},
        {"SLM_CONFIG_SEQUENCE",SLMPatternType.ConfigSequence}
        };

        const double MIN_PATTERN_MS = 8.0; //[msec], minimum time for SLM runtime calculation
        const int SLM_CALIB_PTS = 9;

        private BackgroundWorker slmBuilder;
        private BackgroundWorker slmCalibrator;
        private BackgroundWorker slmWorker;
        private SLMPanelMode _panelMode = SLMPanelMode.ParamEdit;
        private SLMParams _slmParamsCurrent;
        private List<Point> _slmPatternPoints = new List<Point>();
        private List<double> _slmPatternZPos = new List<double>();
        private CaptureSetupViewModel _vm;
        private bool _waveParamsUpdated = true; //prevent changes without clicking OK

        #endregion Fields

        #region Constructors

        public SLMParamEditWin(CaptureSetupViewModel vm)
        {
            InitializeComponent();
            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this) == true)
                return;
            _vm = vm;
            _vm.SLMPanelAvailable = false;
            this.Loaded += SLMParamEditWin_Loaded;
            this.Unloaded += SLMParamEditWin_Unloaded;
            this.Closed += SLMParamEditWin_Closed;
        }

        #endregion Constructors

        #region Enumerations

        public enum SLMPanelMode
        {
            ParamEdit,
            Calibration,
            Browse,
            Build,
            Sequence
        }

        public enum SLMPatternType
        {
            Add,
            Edit,
            Delete,
            ShowHide,
            Execute,
            Calibration,
            Import,
            Build,
            Blank,
            ConfigSequence
        }

        #endregion Enumerations

        #region Delegates

        private delegate void SLMSavePattern();

        private delegate string SLMWorkerGetStatus();

        private delegate void SLMWorkerSetGUI();

        private delegate void SLMWorkerSetStatus();

        private delegate bool SLMWorkerStateChecker();

        #endregion Delegates

        #region Properties

        public SLMPanelMode PanelMode
        {
            get { return _panelMode; }
            set
            {
                switch (value)
                {
                    case SLMPanelMode.ParamEdit:
                        _panelMode = value;
                        this.Title = "Edit SLM Parameters";
                        stpParams.Visibility = System.Windows.Visibility.Visible;
                        bdrROI.Visibility = stpSpacing.Visibility = _vm.IsStimulator ? System.Windows.Visibility.Collapsed : System.Windows.Visibility.Visible;
                        stpRGB.Visibility = System.Windows.Visibility.Visible;
                        stpSLMStatus.Visibility = System.Windows.Visibility.Visible;
                        stpSLMLabel.Visibility = System.Windows.Visibility.Visible;
                        stpSLMButton.Visibility = System.Windows.Visibility.Visible;
                        stpSLMSpin.Visibility = System.Windows.Visibility.Visible;
                        stpCalib.Visibility = System.Windows.Visibility.Collapsed;
                        stpImport.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMSequence.Visibility = System.Windows.Visibility.Collapsed;
                        break;
                    case SLMPanelMode.Calibration:
                        _panelMode = value;
                        this.Title = "SLM Calibration";
                        stpParams.Visibility = System.Windows.Visibility.Collapsed;
                        stpRGB.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMStatus.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMLabel.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMButton.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMSpin.Visibility = System.Windows.Visibility.Collapsed;
                        stpCalib.Visibility = System.Windows.Visibility.Visible;
                        stpImport.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMSequence.Visibility = System.Windows.Visibility.Collapsed;
                        break;
                    case SLMPanelMode.Browse:
                        _panelMode = value;
                        this.Title = "SLM Import";
                        stpParams.Visibility = System.Windows.Visibility.Collapsed;
                        stpRGB.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMStatus.Visibility = System.Windows.Visibility.Visible;
                        stpSLMLabel.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMButton.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMSpin.Visibility = System.Windows.Visibility.Collapsed;
                        stpCalib.Visibility = System.Windows.Visibility.Collapsed;
                        stpImport.Visibility = System.Windows.Visibility.Visible;
                        stpSLMSequence.Visibility = System.Windows.Visibility.Collapsed;
                        break;
                    case SLMPanelMode.Build:
                        _panelMode = value;
                        this.Title = "SLM Build Waveform";
                        stpParams.Visibility = System.Windows.Visibility.Collapsed;
                        stpRGB.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMStatus.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMLabel.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMButton.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMSpin.Visibility = System.Windows.Visibility.Visible;
                        stpCalib.Visibility = System.Windows.Visibility.Collapsed;
                        stpImport.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMSequence.Visibility = System.Windows.Visibility.Collapsed;
                        SLMEpochSequences = new List<SLMEpochSequence>(_vm.EpochSequence.Select(s => s.Clone()).Cast<SLMEpochSequence>()).Select(s => { s.MaxEpochValue = _vm.SLMBleachWaveParams.Count; return s; }).ToList();
                        break;
                    case SLMPanelMode.Sequence:
                        _panelMode = value;
                        this.Title = "Multiple Epochs Configuration";
                        stpParams.Visibility = System.Windows.Visibility.Collapsed;
                        stpRGB.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMStatus.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMLabel.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMButton.Visibility = System.Windows.Visibility.Visible;
                        stpSLMSpin.Visibility = System.Windows.Visibility.Collapsed;
                        stpCalib.Visibility = System.Windows.Visibility.Collapsed;
                        stpImport.Visibility = System.Windows.Visibility.Collapsed;
                        stpSLMSequence.Visibility = System.Windows.Visibility.Visible;
                        SLMEpochSequences = new List<SLMEpochSequence>(_vm.EpochSequence.Select(s => s.Clone()).Cast<SLMEpochSequence>()).Select(s => { s.MaxEpochValue = _vm.SLMBleachWaveParams.Count; return s; }).ToList();
                        SetupLinePanel();
                        break;
                    default:
                        break;
                }
            }
        }

        public List<SLMEpochSequence> SLMEpochSequences
        {
            get;
            set;
        }

        public bool SLMFileSaved
        {
            get;
            set;
        }

        public bool SLMGenResult
        {
            get;
            set;
        }

        public string SLMImportPath
        {
            get
            {
                return (string)this.Dispatcher.Invoke((SLMWorkerGetStatus)delegate
                {
                    return tbImport.Text.ToString();
                });
            }
            set
            {
                this.Dispatcher.Invoke((SLMWorkerSetStatus)delegate
                {
                    tbImport.Text = value;
                });
            }
        }

        public int SLMParamID
        {
            get;
            set;
        }

        public SLMParams SLMParamsCurrent
        {
            get { return _slmParamsCurrent; }
            set
            {
                if (null != _slmParamsCurrent)
                {
                    _slmParamsCurrent.BleachParamsChangedEvent -= SLMParamsCurrent_BleachParamsChangedEvent;
                }
                _slmParamsCurrent = value;
                _slmParamsCurrent.BleachParamsChangedEvent += SLMParamsCurrent_BleachParamsChangedEvent;
            }
        }

        public string SLMPatternStatus
        {
            get
            {
                return (string)this.Dispatcher.Invoke((SLMWorkerGetStatus)delegate
                {
                    return lblSLMPatternStatus.Content.ToString();
                });
            }
            set
            {
                try
                {
                    this.Dispatcher.Invoke((SLMWorkerSetStatus)delegate
                    {
                        stpSLMStatus.Visibility = (string.Empty == value) ? System.Windows.Visibility.Collapsed : System.Windows.Visibility.Visible;
                        lblSLMPatternStatus.Content = value;
                    });
                }
                catch (Exception ex)
                {
                    ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "SLMPatternStatus: " + ex.Message);
                }
            }
        }

        public bool SLMSpinProgressVisible
        {
            set
            {
                this.Dispatcher.Invoke((SLMWorkerSetGUI)delegate
                {
                    canvasSLMSpinProgress.Visibility = stpSLMSpin.Visibility = (value) ? System.Windows.Visibility.Visible : System.Windows.Visibility.Collapsed;
                    stpName.IsEnabled = (!value);
                    stpSpec.IsEnabled = (!value);
                    stpRGB.IsEnabled = (!value);
                    stpMPower.IsEnabled = (!value);
                    stpImportBrowse.IsEnabled = btnImport.IsEnabled = (!value);
                    stpCalibBtns.IsEnabled = (!value);
                    stpSLMSequence.IsEnabled = (!value);
                });
            }
        }

        public string[] WaveFileNameAndPathCurrent
        {
            get;
            set;
        }

        public FileName[] WaveFileNameToSave
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Visit all patterns in one waveform.
        /// </summary>
        /// <returns></returns>
        public bool BuildSequences(bool skipCompare = false)
        {
            if (null == slmBuilder)
                return false;

            if (!slmBuilder.IsBusy)
            {
                //check calibration has been done or valid:
                if (null == _vm.BleachCalibrateFineScaleXY)
                {
                    SLMPatternStatus = String.Format("Galvo calibration has not been done.\n");
                    SLMGenResult = false;
                    return false;
                }
                else if ((0 == _vm.BleachCalibrateFineScaleXY[0]) || (0 == _vm.BleachCalibrateFineScaleXY[1]) ||
                    (0 == _vm.BleachCalibrateFieldSize) || (null == _vm.BleachCalibratePixelXY))
                {
                    SLMPatternStatus = String.Format("Galvo calibration is not valid.\n");
                    SLMGenResult = false;
                    return false;
                }
                else if (_vm.SLMSequenceOn && 0 == SLMEpochSequences.Count)
                {
                    //user cleared epoch sequences, update view model copy
                    _vm.EpochSequence = new List<SLMEpochSequence>();
                    _vm.ClearSLMFiles(_vm.SLMWaveformFolder[1], "txt");
                    _vm.ClearSLMFiles(_vm.SLMWaveformFolder[1], "raw");
                    this.Close();
                    return false;
                }

                if (skipCompare || !_vm.CompareSLMFiles(SLMEpochSequences))
                {
                    SLMPatternStatus = String.Format("Start building waveform ... \n");

                    slmBuilder.DoWork += new DoWorkEventHandler(SLMBuilder_DoWork);
                    slmBuilder.RunWorkerCompleted += new RunWorkerCompletedEventHandler(SLMBuilder_RunWorkerCompleted);
                    slmBuilder.RunWorkerAsync();
                }
                else
                {
                    //close if no waveform to build
                    this.Close();
                }
            }
            return true;
        }

        /// <summary>
        /// create a float array with interleaved x, y point coordinates.
        /// </summary>
        /// <param name="pts"></param>
        /// <returns></returns>
        public float[] PointsToFloatVec(List<Point> pts)
        {
            float[] outArray = new float[pts.Count * 2];
            for (int i = 0; i < pts.Count; i++)
            {
                outArray[2 * i] = (float)pts[i].X;
                outArray[2 * i + 1] = (float)pts[i].Y;
            }
            return outArray;
        }

        /// <summary>Determines if the path contains invalid characters.</summary>
        /// <param name="filePath">File path.</param>
        /// <returns>True if file path contains invalid characters.</returns>
        private static bool ContainsInvalidPathCharacters(string filePath)
        {
            for (var i = 0; i < filePath.Length; i++)
            {
                int c = filePath[i];

                if (c == '\"' || c == '<' || c == '>' || c == '|' || c == '*' || c == '?' || c < 32)
                    return true;
            }
            return false;
        }

        private StackPanel AddEpochLineGUI(SLMEpochSequence slmEpoch, int lineNumber)
        {
            StackPanel sp = new StackPanel();
            sp.Orientation = System.Windows.Controls.Orientation.Horizontal;
            sp.HorizontalAlignment = System.Windows.HorizontalAlignment.Left;

            //sequnce
            Label epochLabel = new Label() { Margin = new Thickness(5), Content = lineNumber, HorizontalContentAlignment = HorizontalAlignment.Center, VerticalAlignment = VerticalAlignment.Center, Width = 25 };//Margin = new Thickness(10), Content = _lineNumber,  HorizontalAlignment=HorizontalAlignment.Left, VerticalAlignment = VerticalAlignment.Center, Width = 35

            TextBox epochText = new TextBox() { Margin = new Thickness(5), Width = 260, Height = 25, HorizontalAlignment = HorizontalAlignment.Left };
            epochText.Background = new SolidColorBrush(Colors.White);
            epochText.Foreground = new SolidColorBrush(Colors.Black);
            epochText.Name = "epochSequence" + lineNumber;

            Binding epochTextBinding = new Binding();
            epochTextBinding.Source = slmEpoch;
            epochTextBinding.Path = new PropertyPath("SequenceStr");
            epochTextBinding.Mode = BindingMode.TwoWay;
            epochTextBinding.UpdateSourceTrigger = UpdateSourceTrigger.PropertyChanged;
            epochText.SetBinding(TextBox.TextProperty, epochTextBinding);

            Binding epochBruchBinding = new Binding();
            epochBruchBinding.Source = slmEpoch;
            epochBruchBinding.Path = new PropertyPath("SequenceStrBrush");
            epochBruchBinding.Mode = BindingMode.OneWay;
            epochBruchBinding.UpdateSourceTrigger = UpdateSourceTrigger.PropertyChanged;
            epochText.SetBinding(TextBox.BorderBrushProperty, epochBruchBinding);

            //epoch
            Label epochCount = new Label() { Content = "Count", Height = 25 };
            TextBox epochCountText = new TextBox() { Margin = new Thickness(5), Width = 50, Height = 25 };
            epochCountText.Background = new SolidColorBrush(Colors.White);
            epochCountText.Foreground = new SolidColorBrush(Colors.Black);

            Binding epochCountTextBinding = new Binding();
            epochCountTextBinding.Source = slmEpoch;
            epochCountTextBinding.Path = new PropertyPath("EpochCount");
            epochCountTextBinding.Mode = BindingMode.TwoWay;
            epochCountTextBinding.UpdateSourceTrigger = UpdateSourceTrigger.PropertyChanged;
            epochCountText.SetBinding(TextBox.TextProperty, epochCountTextBinding);

            Binding epochCountBruchBinding = new Binding();
            epochCountBruchBinding.Source = slmEpoch;
            epochCountBruchBinding.Path = new PropertyPath("EpochCountBrush");
            epochCountBruchBinding.Mode = BindingMode.OneWay;
            epochCountBruchBinding.UpdateSourceTrigger = UpdateSourceTrigger.PropertyChanged;
            epochCountText.SetBinding(TextBox.BorderBrushProperty, epochCountBruchBinding);

            if (System.Windows.Controls.Validation.GetHasError(epochCountText) == true)
            {
                epochCountText.BorderBrush = new SolidColorBrush(Colors.Red);
            }

            Button removeLine = new Button() { Content = "X", Width = 20, Height = 20, VerticalAlignment = VerticalAlignment.Center, HorizontalAlignment = HorizontalAlignment.Center, VerticalContentAlignment = VerticalAlignment.Center };
            removeLine.Foreground = new SolidColorBrush(Colors.White);

            removeLine.Click += (sender, EventArgs) => { remove_Click(sender, EventArgs, System.Convert.ToInt32(epochLabel.Content.ToString()) - 1); };

            sp.Children.Add(epochLabel);
            sp.Children.Add(epochText);

            sp.Children.Add(epochCount);
            sp.Children.Add(epochCountText);

            sp.Children.Add(removeLine);

            Grid.SetRow(epochLabel, 0);
            Grid.SetRow(epochText, 0);

            Grid.SetColumn(epochLabel, 0);
            Grid.SetColumn(epochText, 1);

            return sp;
        }

        private void AddEpochLine_Click(object sender, RoutedEventArgs e)
        {
            if (null == SLMEpochSequences)
                SLMEpochSequences = new List<SLMEpochSequence>();

            SLMEpochSequences.Add(new SLMEpochSequence(_vm.SLMBleachWaveParams.Count));

            linePanel.Children.Add(AddEpochLineGUI(SLMEpochSequences.Last(), SLMEpochSequences.Count));
        }

        private void btnCalibration_Click(object sender, RoutedEventArgs e)
        {
            string note = (null != (sender as FrameworkElement)) ? (sender as FrameworkElement).Tag as string : (string)(sender);
            SLMCalibPanel slmCalib;
            string roiPathAndName = _vm.BleachROIPath + "SLMCalibROIs.xaml";
            string strBody;

            switch (note)
            {
                case "BURN":
                    //limit only one task is running:
                    if (slmCalibrator.IsBusy)
                    { return; }

                    OverlayManagerClass.Instance.DisplayPatternROI(ref CaptureSetupViewModel.OverlayCanvas, OverlayManagerClass.Instance.PatternID - 1, true);
                    slmCalibrator.DoWork += new DoWorkEventHandler(SLMCalibrator_DoWork);
                    slmCalibrator.RunWorkerCompleted += new RunWorkerCompletedEventHandler(SLMCalibrator_RunWorkerCompleted);
                    slmCalibrator.RunWorkerAsync(roiPathAndName);
                    break;
                case "REDO":
                    //reset affine values if not matched:
                    _vm.ResetSLMCalibration();
                    OverlayManagerClass.Instance.DisplayPatternROI(ref CaptureSetupViewModel.OverlayCanvas, OverlayManagerClass.Instance.PatternID - 1, true);

                    slmCalib = new SLMCalibPanel("BURN", "SELECT", "New Calibration", "Move to another clear area on the\ncalibration slide then Press Yes to\ncreate calibration spots on the slide,\nPress No to continue.");
                    this.DataContext = slmCalib;
                    this.Show();
                    break;
                case "SELECT":
                    OverlayManagerClass.Instance.DeletePatternROI(ref CaptureSetupViewModel.OverlayCanvas, OverlayManagerClass.Instance.PatternID);
                    OverlayManagerClass.Instance.ClearNonSaveROIs(ref CaptureSetupViewModel.OverlayCanvas);
                    OverlayManagerClass.Instance.DisplayPatternROI(ref CaptureSetupViewModel.OverlayCanvas, OverlayManagerClass.Instance.PatternID - 1, false);

                    strBody = "Using Cross Hair ROI tool, mark the \ncenter points of the burned calibration \nspots on the image. Press YES when\ncomplete, and press No to break.\n\n\n\tx4\tx5\tx6\n\n\tx3\n\t\t\t\tx7\n\n\tx2\n\t\t     x8\n\n\n\tx1\t\t\tx9\n ";
                    slmCalib = new SLMCalibPanel("FINISH_SELECT", "DONE", "New Calibration", strBody);
                    this.DataContext = slmCalib;
                    this.Show();
                    break;
                case "FINISH_SELECT":
                    //retrieve user selected points (without offset):
                    string roiType = string.Empty;
                    Point offCenter = new Point(-1, -1);
                    List<Point> pts = OverlayManagerClass.Instance.GetPatternROICenters(OverlayManagerClass.Instance.PatternID, ref roiType, ref offCenter);
                    if (SLM_CALIB_PTS != pts.Count)
                    {
                        OverlayManagerClass.Instance.ClearNonSaveROIs(ref CaptureSetupViewModel.OverlayCanvas);
                        OverlayManagerClass.Instance.DisplayPatternROI(ref CaptureSetupViewModel.OverlayCanvas, OverlayManagerClass.Instance.PatternID - 1, false);
                        strBody = "Error: Number of your selections is invalid.\n\nUsing Cross Hair ROI tool, mark the \ncenter points of the burned calibration \nspots on the image. Press YES when\ncomplete, and press No to break.\n\n\n\tx4\tx5\tx6\n\n\tx3\n\t\t\t\tx7\n\n\tx2\n\t\t     x8\n\n\n\tx1\t\t\tx9\n ";
                        slmCalib = new SLMCalibPanel("FINISH_SELECT", "DONE", "New Calibration", strBody);
                        this.DataContext = slmCalib;
                        this.Show();
                    }
                    else
                    {
                        OverlayManagerClass.Instance.SetPatternToSaveROI(OverlayManagerClass.Instance.PatternID);
                        OverlayManagerClass.Instance.SaveROIs(roiPathAndName);

                        //valid count, send for calibration,
                        if (slmCalibrator.IsBusy)
                        { return; }

                        slmCalibrator.DoWork += new DoWorkEventHandler(SLMCalibrator_DoWork);
                        slmCalibrator.RunWorkerCompleted += new RunWorkerCompletedEventHandler(SLMCalibrator_RunWorkerCompleted);
                        slmCalibrator.RunWorkerAsync(roiPathAndName);
                    }
                    break;
                case "DONE":
                    CancelCalibrator();
                    this.Close();
                    break;
                default:
                    break;
            }
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void btnOk_Click(object sender, RoutedEventArgs e)
        {
            switch ((SLMPanelMode)this.PanelMode)
            {
                case SLMPanelMode.ParamEdit:
                    if (slmWorker.IsBusy)
                    {
                        CancelSLMBackgroundWorker(slmWorker, true);
                        this.Close();
                    }
                    else
                    {
                        GenSLMPattern();
                    }
                    break;
                case SLMPanelMode.Sequence:
                    {
                        BuildSequences();
                    }
                    break;
                default:
                    break;
            }
        }

        private void btnSLMBrowse_Click(object sender, RoutedEventArgs e)
        {
            string note = (null != (sender as FrameworkElement)) ? (sender as FrameworkElement).Tag as string : (string)(sender);
            switch (note)
            {
                case "BROWSE":
                    Microsoft.Win32.OpenFileDialog ofd = new Microsoft.Win32.OpenFileDialog();

                    ofd.FileName = "*.xml";
                    ofd.InitialDirectory = System.IO.Path.GetDirectoryName(_vm.SLMImportFilePathName);
                    ofd.DefaultExt = ".xml";
                    ofd.Filter = "XML Files (*.xml)|*.xml";

                    Nullable<bool> result = ofd.ShowDialog();

                    if (true == result)
                    {
                        SLMImportPath = ofd.FileName;
                        SLMPatternStatus = string.Empty;
                    }
                    break;
                case "IMPORT":
                    SLMGenResult = false;

                    if (!File.Exists(SLMImportPath))
                    {
                        SLMPatternStatus = String.Format("Selected file does not exist.");
                        return;
                    }
                    if (".xml" != System.IO.Path.GetExtension(SLMImportPath))
                    {
                        SLMPatternStatus = String.Format("Invalid file format, please select another xml.");
                        return;
                    }
                    if (null == _vm.BleachLSMFieldScaleXYFine)
                    {
                        SLMPatternStatus = String.Format("Calibration must be done before import.\n");
                        return;
                    }
                    _vm.SLMImportFilePathName = SLMImportPath;

                    //load xml
                    XmlDocument xmlSLMImportDoc = new XmlDocument();
                    xmlSLMImportDoc.Load(SLMImportPath);

                    //load SLM params and determine the import type
                    if (!TryImportPatternSequences(xmlSLMImportDoc))
                    {
                        if (!TryImportWaveforms(xmlSLMImportDoc))
                            return;
                    }
                    this.DataContext = _vm;
                    RebuildSLMWaveform(true);
                    break;
                case "CANCEL":
                    this.Close();
                    break;
            }
        }

        /// <summary>
        /// Build a waveform to bleach center FOV, used for SLM calibration; return with failed reason or "0"
        /// </summary>
        /// <returns></returns>
        private string BuildCenterBleachWaveform()
        {
            //check calibration dwell time:
            if (0 >= _vm.SLMCalibDwell)
            {
                return "Dwell Time Per Calibration spot cannot be 0 or negative.";
            }

            SLMParams sWaveParams = new SLMParams();
            sWaveParams.BleachWaveParams.ClockRate = WaveformBuilder.ClkRate = _vm.IsStimulator ? _vm.BleachInternalClockRate : (int)(_vm.BleachLSMPixelXY[0] * WaveformBuilder.MS_TO_S);  //[Hz], keep PixelSpacing as 1
            sWaveParams.BleachWaveParams.PrePatIdleTime = sWaveParams.BleachWaveParams.PreIdleTime = 0;       //[ms]
            sWaveParams.BleachWaveParams.DwellTime = (WaveformBuilder.MinDwellTime < _vm.SLMCalibDwell) ?     //[us]
                (_vm.SLMCalibDwell * (int)WaveformBuilder.MS_TO_S) :
                (WaveformBuilder.MinDwellTime * (int)WaveformBuilder.MS_TO_S);
            sWaveParams.BleachWaveParams.PostPatIdleTime = sWaveParams.BleachWaveParams.PostIdleTime = 0;     //[ms]
            sWaveParams.BleachWaveParams.Power = _vm.SLMSelectWavelength ? new double[2] { 0.0, _vm.SLMCalibPower } : new double[2] { _vm.SLMCalibPower, 0.0 };     //[%]
            sWaveParams.BleachWaveParams.shapeType = "Crosshair";
            sWaveParams.BleachWaveParams.UMPerPixel = (double)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (object)1.0];
            sWaveParams.BleachWaveParams.UMPerPixelRatio = _vm.BleachPixelSizeUMRatio;
            int[] pixelXY = _vm.IsStimulator ? _vm.SLMPixelXY : _vm.BleachLSMPixelXY;
            sWaveParams.BleachWaveParams.Center = new Point(Math.Floor((double)pixelXY[0] / 2), Math.Floor((double)(pixelXY[1] / 2)));
            sWaveParams.BleachWaveParams.Iterations = 1;
            sWaveParams.BleachWaveParams.ROIHeight = sWaveParams.BleachWaveParams.ROIWidth = 1;

            if (_vm.SLMCalibWaveParam.CompareTo(sWaveParams.BleachWaveParams))
            {
                //no need to re-build waveform:
                return "0";
            }

            //check calibration has been done or valid, for Galvo only:
            if (!_vm.IsStimulator)
            {
                if (null == _vm.BleachCalibrateFineScaleXY)
                {
                    return "Galvo calibration has not been done.\n";
                }
                else if ((0 == _vm.BleachCalibrateFineScaleXY[0]) || (0 == _vm.BleachCalibrateFineScaleXY[1]) ||
                    (0 == _vm.BleachCalibrateFieldSize) || (null == _vm.BleachCalibratePixelXY))
                {
                    return "Invalid Galvo calibration.\n";
                }
            }
            double[] powerVal = new double[2] { WaveformBuilder.GetPockelsPowerValue(sWaveParams.BleachWaveParams.Power[0], _vm.BleachCalibratePockelsVoltageMin0[0], _vm.BleachCalibratePockelsVoltageMax0[0], (PockelsResponseType)MVMManager.Instance["PowerControlViewModel", "BleacherPowerResponse0", 0]),
             WaveformBuilder.GetPockelsPowerValue(sWaveParams.BleachWaveParams.Power[1], _vm.BleachCalibratePockelsVoltageMin0[1], _vm.BleachCalibratePockelsVoltageMax0[1], (PockelsResponseType)MVMManager.Instance["PowerControlViewModel", "BleacherPowerResponse0", 0]) };

            //***   Start Generation (center crosshair only)    ***//
            _vm.InitializeWaveformBuilder(sWaveParams.BleachWaveParams.ClockRate);
            WaveformBuilder.ResetWaveform();

            SLMPatternStatus = String.Format("Creating Calibration Waveform ...\n");

            //start from center:
            if (!_vm.IsStimulator)
                WaveformBuilder.BuildTravel(new Point(sWaveParams.BleachWaveParams.Center.X, sWaveParams.BleachWaveParams.Center.Y), 0, 0, 0);

            //pattern pre-idle:
            WaveformBuilder.BuildPrePatIdle(sWaveParams.BleachWaveParams, true, true);

            //dwell crosshair:
            WaveformBuilder.BuildSpot(sWaveParams.BleachWaveParams, powerVal);

            //pattern post-idle:
            WaveformBuilder.BuildPostPatIdle(sWaveParams.BleachWaveParams, true, true);

            //Return to start position & signal cycle completed:
            WaveformBuilder.ReturnHome(true);

            //NI limit data length per channel or stop request:
            if ((WaveformBuilder.GetWaveform().Count > Int32.MaxValue) || (true == slmCalibrator.CancellationPending))
            {
                return "Invalid waveform size or cancel request.\n";
            }

            //save to file:
            SLMPatternStatus = String.Format("Saving Calibration Waveform ...\n");
            string calibWaveName = _vm.BleachROIPath + "SLMCalibWaveform.raw";
            WaveformBuilder.SaveWaveform(calibWaveName, true, new bool[3] { !_vm.IsStimulator, true, true });

            while (!WaveformBuilder.CheckSaveState())
            {
                System.Threading.Thread.Sleep(50);

                if (true == slmCalibrator.CancellationPending)
                {
                    WaveformBuilder.StopSave();
                }
            }

            if (!WaveformBuilder.GetSaveResult())
            {
                return "Save Calibration Waveform Failed.\n";
            }

            //update local params for next comparison:
            _vm.SLMCalibWaveParam = sWaveParams.BleachWaveParams;
            SLMPatternStatus = string.Empty;
            return "0";
        }

        private void CancelCalibrator()
        {
            SLMWorkerStateChecker stillWorking = () => { return slmCalibrator.IsBusy; };
            slmCalibrator.CancelAsync();
            SLMPatternStatus = String.Format("Terminating task, please wait ...");
            while ((bool)this.Dispatcher.Invoke(stillWorking, null))
            {
                _vm.StopBleach();
                System.Threading.Thread.Sleep(50);
            }
        }

        private void CancelSLMBackgroundWorker(BackgroundWorker worker, bool updateFile)
        {
            SLMWorkerStateChecker stillWorking = () => { return worker.IsBusy; };
            worker.CancelAsync();
            SLMPatternStatus = String.Format("Terminating task, please wait ...");
            while ((bool)this.Dispatcher.Invoke(stillWorking, null))
            {
                SLMGenResult = false;
                System.Threading.Thread.Sleep(50);
                if (!updateFile)
                    break;      //closing app or switching tabs
            }
            if (updateFile)
                UpdateWaveParamsAndFile();
        }

        private void ColorDelta_Changed(object sender, MouseWheelEventArgs e)
        {
            if ((null != SLMParamsCurrent) && (this.IsActive))
            {
                if (0 == ((Slider)sender).Name.CompareTo("sliderR"))
                {
                    sliderR.Value += (int)(e.Delta / 120);
                    SLMParamsCurrent.Red = (int)sliderR.Value;
                }
                if (0 == ((Slider)sender).Name.CompareTo("sliderG"))
                {
                    sliderG.Value += (int)(e.Delta / 120);
                    SLMParamsCurrent.Green = (int)sliderG.Value;
                }
                if (0 == ((Slider)sender).Name.CompareTo("sliderB"))
                {
                    sliderB.Value += (int)(e.Delta / 120);
                    SLMParamsCurrent.Blue = (int)sliderB.Value;
                }

                System.Collections.Specialized.BitVector32 bitVec32 = new System.Collections.Specialized.BitVector32(Convert.ToByte(SLMParamsCurrent.Red));
                bitVec32[OverlayManager.OverlayManagerClass.SecG] = Convert.ToByte(SLMParamsCurrent.Green);
                bitVec32[OverlayManager.OverlayManagerClass.SecB] = Convert.ToByte(SLMParamsCurrent.Blue);
                OverlayManagerClass.Instance.ColorRGB = bitVec32.Data;
                OverlayManagerClass.Instance.UpdatePatternROIColor(ref CaptureSetupViewModel.OverlayCanvas,
                    (_vm.IsStimulator ? ThorSharedTypes.Mode.PATTERN_WIDEFIELD : ThorSharedTypes.Mode.PATTERN_NOSTATS));
            }
        }

        private void Color_Changed(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if ((null != SLMParamsCurrent) && (this.IsActive))
            {
                if (0 == ((Slider)sender).Name.CompareTo("sliderR"))
                {
                    SLMParamsCurrent.Red = (int)sliderR.Value;
                }
                if (0 == ((Slider)sender).Name.CompareTo("sliderG"))
                {
                    SLMParamsCurrent.Green = (int)sliderG.Value;
                }
                if (0 == ((Slider)sender).Name.CompareTo("sliderB"))
                {
                    SLMParamsCurrent.Blue = (int)sliderB.Value;
                }

                System.Collections.Specialized.BitVector32 bitVec32 = new System.Collections.Specialized.BitVector32(Convert.ToByte(SLMParamsCurrent.Red));
                bitVec32[OverlayManager.OverlayManagerClass.SecG] = Convert.ToByte(SLMParamsCurrent.Green);
                bitVec32[OverlayManager.OverlayManagerClass.SecB] = Convert.ToByte(SLMParamsCurrent.Blue);
                OverlayManagerClass.Instance.ColorRGB = bitVec32.Data;
                OverlayManagerClass.Instance.UpdatePatternROIColor(ref CaptureSetupViewModel.OverlayCanvas,
                    (_vm.IsStimulator ? ThorSharedTypes.Mode.PATTERN_WIDEFIELD : ThorSharedTypes.Mode.PATTERN_NOSTATS));
            }
        }

        private void GenSLMPattern()
        {
            if ((null == _vm.Bitmap) || (null == _vm.BleachLSMFieldScaleXYFine))
            {
                SLMPatternStatus = String.Format("Unable to start SLM Generation...\n");
                return;
            }

            SLMFileSaved = false;

            if (!slmWorker.IsBusy)
            {
                SLMPatternStatus = String.Format("Checking ... ");

                SLMSpinProgressVisible = true;

                slmWorker.DoWork += new DoWorkEventHandler(SLMWorker_DoWork);
                slmWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(SLMWorker_RunWorkerCompleted);
                slmWorker.RunWorkerAsync();
            }
        }

        private int GetDwellCount(int clockRateHz)
        {
            //check calibration has been done or valid:
            if (null == _vm.BleachCalibrateFineScaleXY)
            { return 0; }
            else if ((0 == _vm.BleachCalibrateFineScaleXY[0]) || (0 == _vm.BleachCalibrateFineScaleXY[1]))
            { return 0; }

            if ((0 == _vm.BleachCalibrateFieldSize) || (null == _vm.BleachCalibratePixelXY))
            { return 0; }

            double[] powerVal = new double[1] { WaveformBuilder.GetPockelsPowerValue(SLMParamsCurrent.BleachWaveParams.Power[0], _vm.BleachCalibratePockelsVoltageMin0[0], _vm.BleachCalibratePockelsVoltageMax0[0], (PockelsResponseType)MVMManager.Instance["PowerControlViewModel", "BleacherPowerResponse0", (object)0]) };

            _vm.InitializeWaveformBuilder(clockRateHz);
            WaveformBuilder.ResetWaveform();
            SLMParamsCurrent.BleachWaveParams.DwellTime = WaveformBuilder.MinDwellTime;   //purpose to get minimum dwell count
            SLMParamsCurrent.BleachWaveParams.Fill = 1;     //always do fill ellipse

            //dwell countour fill, check dwell count once:
            switch (SLMParamsCurrent.BleachWaveParams.ROIWidthUM.ToString())
            {
                case "0":   //crosshair
                    return 1;
                default:    //ellipse
                    WaveformBuilder.BuildEllipse(SLMParamsCurrent.BleachWaveParams, powerVal);
                    break;
            }
            return (WaveformBuilder.GetWaveform().Count / 2);    //half for travel
        }

        void OverlayManager_ClearedObjectEvent()
        {
            UpdateSLMParamEdit();
        }

        void OverlayManager_ObjectSizeChangedEvent(double arg2, double arg3)
        {
            if (null != _slmParamsCurrent)
            {
                _slmParamsCurrent.BleachWaveParams.ROIWidth = arg2;
                _slmParamsCurrent.BleachWaveParams.ROIHeight = arg3;
                UpdateSLMParamEdit();
            }
        }

        void OverlayManager_UpdatingObjectEvent(bool obj)
        {
            if (true == obj)
                UpdateSLMParamEdit();
        }

        private void RebuildSLMWaveform(bool skipCompare = false)
        {
            if (skipCompare || (0 >= Directory.EnumerateFiles(_vm.SLMActiveFolder, "*.raw ", SearchOption.TopDirectoryOnly).Count()) || !_vm.CompareSLMParams())
            {
                this.PanelMode = SLMParamEditWin.SLMPanelMode.Build;
                this.BuildSequences(skipCompare);
            }
            else if (SLMGenResult)
            {
                //persist slm params in all modalities:
                _vm.PersistGlobalExperimentXML(GlobalExpAttribute.SLM_BLEACH);

                this.Close();
            }
        }

        private void remove_Click(object sender, RoutedEventArgs e, int i)
        {
            if (null != SLMEpochSequences)
                SLMEpochSequences.RemoveAt(i);

            linePanel.Children.Clear();
            SetupLinePanel();
        }

        private void SaveSLMFiles()
        {
            string roiType = string.Empty;
            //double zPos;

            //check params:
            if (0 >= SLMParamsCurrent.PixelSpacing)
            {
                SLMPatternStatus = String.Format("Pixel Spacing cannot be {0}.", SLMParamsCurrent.PixelSpacing.ToString());
                return;
            }

            //create sub-directory:
            if (!Directory.Exists(_vm.SLMWaveformFolder[0]))
                Directory.CreateDirectory(_vm.SLMWaveformFolder[0]);

            _vm.RoiPlaneList.Clear();
            _vm.RoiPointList.Clear();

            //get pattern offset:
            this.Dispatcher.Invoke((SLMWorkerSetGUI)delegate
            {
                Point offCenter = new Point(-1, -1);
                _slmPatternPoints = OverlayManagerClass.Instance.GetPatternROICenters(OverlayManagerClass.Instance.PatternID, ref roiType, ref offCenter);
                _slmPatternZPos = OverlayManagerClass.Instance.GetPatternROIZPos(OverlayManagerClass.Instance.PatternID, ref roiType);
                SLMParamsCurrent.BleachWaveParams.shapeType = roiType;
                Point centerOffset = _vm.GetSLMPatternBoundROICenter(ref _slmPatternPoints, offCenter);
                SLMParamsCurrent.BleachWaveParams.Center = new Point(Math.Round((_vm.BleachLSMPixelXY[0] / 2) - centerOffset.X, 3), Math.Round((_vm.BleachLSMPixelXY[1] / 2) - centerOffset.Y, 3));

                for (int j = 0; j < _slmPatternPoints.Count(); j++)
                {
                    Shape roiPoint = (roiType == "Crosshair") ? (Shape)new ROICrosshair() : new ROIEllipse();
                    //zPos = _slmPatternZPos[j];
                    if (roiType == "Crosshair")
                    {
                        ((ROICrosshair)roiPoint).CenterPoint = new Point(_slmPatternPoints[j].X, _slmPatternPoints[j].Y);
                        ((ROICrosshair)roiPoint).ZValue = _slmPatternZPos[j];

                    }
                    else
                    {
                        ((ROIEllipse)roiPoint).Center = new Point(_slmPatternPoints[j].X, _slmPatternPoints[j].Y);
                        ((ROIEllipse)roiPoint).ZValue = _slmPatternZPos[j];
                        ((ROIEllipse)roiPoint).Width = _slmParamsCurrent.BleachWaveParams.ROIWidth;
                        ((ROIEllipse)roiPoint).Height = _slmParamsCurrent.BleachWaveParams.ROIHeight;

                    }
                    int k = 0;
                    int idx = 0;
                    List<Shape> roiPointList = new List<Shape>();
                    while (k <= _vm.RoiPlaneList.Count())
                    {
                        if (_vm.RoiPlaneList.Count() <= 0)
                        {
                            _vm.RoiPointList.Add(roiPoint);
                            _vm.RoiPlaneList.Add(_vm.RoiPointList);
                            k = _vm.RoiPlaneList.Count() + 1;
                        }
                        else if (roiType == "Crosshair")
                        {
                            if (_slmPatternZPos[j] == ((ROICrosshair)(_vm.RoiPlaneList.ElementAt(idx).ElementAt(0))).ZValue)
                            {
                                _vm.RoiPlaneList.ElementAt(idx).Add(roiPoint);
                                k = _vm.RoiPlaneList.Count() + 1;
                            }
                            else if (idx == _vm.RoiPlaneList.Count() - 1)
                            {
                                roiPointList.Add(roiPoint);
                                _vm.RoiPlaneList.Add(roiPointList);
                                k = _vm.RoiPlaneList.Count() + 1;
                            }
                            idx++;

                        }
                        else if (roiType == "Ellipse")
                        {
                            if (_slmPatternZPos[j] == ((ROIEllipse)(_vm.RoiPlaneList.ElementAt(idx).ElementAt(0))).ZValue)
                            {
                                _vm.RoiPlaneList.ElementAt(idx).Add(roiPoint);
                                k = _vm.RoiPlaneList.Count() + 1;
                            }
                            else if (idx == _vm.RoiPlaneList.Count() - 1)
                            {
                                roiPointList.Add(roiPoint);
                                _vm.RoiPlaneList.Add(roiPointList);
                                k = _vm.RoiPlaneList.Count() + 1;
                            }
                            idx++;
                        }
                    }
                }
            });

            //configure file path and name
            WaveFileNameToSave[0].FileExtension = ".raw";
            WaveFileNameToSave[1].FileExtension = ".bmp";
            for (int i = 0; i < WaveFileNameToSave.Count(); i++)
            {
                if (string.IsNullOrEmpty(WaveFileNameToSave[i].NameNumber))
                {
                    WaveFileNameToSave[i].Increment();
                }
                WaveFileNameToSave[i].MakeUnique(_vm.SLMWaveformFolder[0]);
            }
            FileName txtFileName = new FileName(WaveFileNameToSave[1].NameWithoutExtension);    //txt file name will follow bmp file name
            txtFileName.FileExtension = ".txt";

            WaveFileNameAndPathCurrent[0] = (0 > SLMParamID) ? null : (_vm.SLMWaveBaseName[0] + "_" + SLMParamsCurrent.BleachWaveParams.ID.ToString("D" + FileName.GetDigitCounts().ToString()));    //waveform name, old ID if in edit
            WaveFileNameAndPathCurrent[1] = _vm.SLMWaveformFolder[0] + "\\" + WaveFileNameToSave[0].FullName;    //waveform path
            WaveFileNameAndPathCurrent[2] = (0 > SLMParamID) ? null : (_vm.SLMWaveBaseName[1] + "_" + SLMParamsCurrent.BleachWaveParams.ID.ToString("D" + FileName.GetDigitCounts().ToString()));    //bmp name, old ID if in edit
            WaveFileNameAndPathCurrent[3] = _vm.SLMWaveformFolder[0] + "\\" + WaveFileNameToSave[1].FullName;    //bmp path

            ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_NA, (double)MVMManager.Instance["ObjectiveControlViewModel", "NA", (object)0], (int)IDevice.DeviceSetParamType.NO_EXECUTION);

            //save txt files
            if (_vm.RoiPlaneList.Count > 0)
            {
                System.IO.StreamWriter file = new System.IO.StreamWriter((null == WaveFileNameAndPathCurrent[2]) ? (_vm.SLMWaveformFolder[0] + "\\" + txtFileName.FullName) : (_vm.SLMWaveformFolder[0] + "\\" + WaveFileNameAndPathCurrent[2] + ".txt"));
                String tmpString;

                for (int p = 0; p < _vm.RoiPlaneList.Count; p++)
                {
                    if (SLMParamsCurrent.BleachWaveParams.shapeType == "Crosshair")
                    {
                        tmpString = ((ROICrosshair)(_vm.RoiPlaneList.ElementAt(p).ElementAt(0))).ZValue.ToString();
                        for (int q = 0; q < _vm.RoiPlaneList.ElementAt(p).Count; q++)
                        {
                            tmpString = tmpString + " " + ((ROICrosshair)(_vm.RoiPlaneList.ElementAt(p).ElementAt(q))).CenterPoint.ToString();
                        }
                    }
                    else
                    {
                        tmpString = ((ROIEllipse)(_vm.RoiPlaneList.ElementAt(p).ElementAt(0))).ZValue.ToString();
                        for (int q = 0; q < _vm.RoiPlaneList.ElementAt(p).Count; q++)
                        {
                            tmpString = tmpString + " " + ((ROIEllipse)(_vm.RoiPlaneList.ElementAt(p).ElementAt(q))).ROICenter.ToString();
                        }
                    }

                    file.WriteLine(tmpString);
                }
                file.Close();
            }
        }

        private bool SaveSLMPattern()
        {
            //apply preset name if empty and check unique:
            if (null == SLMParamsCurrent.Name)
            {
                SLMParamsCurrent.Name = WaveFileNameToSave[1].NameWithoutExtension;
            }
            else
            {
                if (string.Empty == SLMParamsCurrent.Name)
                {
                    SLMParamsCurrent.Name = WaveFileNameToSave[1].NameWithoutExtension;
                }
            }

            //set pattern unique name:
            FileName compName = new FileName(SLMParamsCurrent.Name, '_');
            compName.FileExtension = ".bmp";
            int currentNameNumberInt = -1;
            if (0 < SLMParamsCurrent.Name.LastIndexOf('_'))     //assume name number separator '_'
                Int32.TryParse(SLMParamsCurrent.Name.Substring(SLMParamsCurrent.Name.LastIndexOf('_') + 1, SLMParamsCurrent.Name.Length - SLMParamsCurrent.Name.LastIndexOf('_') - 1), out currentNameNumberInt);

            /// ===========================================
            /// [unique name #1]: check if unique in folder
            /// ===========================================
            if (SLMParamsCurrent.Name.LastIndexOf('_') > 0)
            {
                //use the name after checking folder for file name unique:
                if ((compName.NameWithoutNumber == WaveFileNameToSave[1].NameWithoutNumber) && (compName.NameNumberInt < WaveFileNameToSave[1].NameNumberInt))
                {
                    SLMParamsCurrent.Name = WaveFileNameToSave[1].NameWithoutExtension;
                }
                else
                {
                    //check if unique on list by filename in folder:
                    for (int i = 0; i < _vm.SLMBleachWaveParams.Count; i++)
                    {
                        if (compName.NameWithoutNumber == _vm.SLMBleachWaveParams[i].Name)
                        {
                            compName.MakeUnique(_vm.SLMWaveformFolder[0]);
                            SLMParamsCurrent.Name = compName.NameWithoutExtension;
                        }
                    }
                }
            }

            /// ===========================================
            /// [unique name #2]: check if unique on list
            /// ===========================================
            List<int> nameNumIdxList = new List<int>();
            int lastMatchIdx = -1;
            for (int i = 0; i < _vm.SLMBleachWaveParams.Count; i++)
            {
                FileName localName = new FileName(_vm.SLMBleachWaveParams[i].Name, '_');

                if (compName.NameWithoutNumber == localName.NameWithoutNumber)          //match name body
                {
                    lastMatchIdx = i;
                    if ((0 < _vm.SLMBleachWaveParams[i].Name.LastIndexOf('_')) &&        //name have name number, assuming name number separator '_'
                        (!nameNumIdxList.Contains((int)(localName.NameNumberInt))))
                    {
                        nameNumIdxList.Add((int)localName.NameNumberInt);
                    }
                }
            }

            //name already on the list with number:
            if (0 < nameNumIdxList.Count)
            {
                int intIdx = -1;

                //sort to have number in ascending
                nameNumIdxList.Sort();

                //find if there is any gap in number
                List<int> availableIdxList = Enumerable.Range(1, nameNumIdxList.Max()).Except(nameNumIdxList).ToList();

                //determine range of currentNameNumberInt: [-1: no number, nameNumIdxList.Min(0) ~ Max(), > Max()]
                if (0 > currentNameNumberInt)
                {
                    //if in edit, leave it:
                    if (0 > SLMParamID)
                    {
                        intIdx = (0 < availableIdxList.Count) ? availableIdxList.First() : (int)(nameNumIdxList.Max() + 1);
                    }
                }
                else if (nameNumIdxList.Max() < currentNameNumberInt)
                {
                    intIdx = currentNameNumberInt;
                }
                else
                {
                    if (0 <= SLMParamID)
                    {
                        //in edit mode, leave it unchanged
                        intIdx = currentNameNumberInt;
                    }
                    else
                    {
                        if (0 < availableIdxList.Count)
                        {
                            intIdx = (availableIdxList.Contains(currentNameNumberInt)) ? currentNameNumberInt : availableIdxList.First();
                        }
                        else
                        {
                            intIdx = (int)(nameNumIdxList.Max() + 1);
                        }
                    }
                }
                SLMParamsCurrent.Name = (0 > intIdx) ? SLMParamsCurrent.Name : (compName.NameWithoutNumber + '_' + intIdx.ToString("D" + FileName.GetDigitCounts().ToString()));
            }
            else if ((0 <= lastMatchIdx) && (0 > SLMParamID))  //name is found without number in add mode
            {
                SLMParamsCurrent.Name = (0 > currentNameNumberInt) ? (compName.NameWithoutNumber + "_001") : (compName.NameWithoutNumber + '_' + currentNameNumberInt.ToString("D" + FileName.GetDigitCounts().ToString()));
            }

            //check if valid name:
            if (ContainsInvalidPathCharacters(SLMParamsCurrent.Name))
            {
                SLMPatternStatus = String.Format("Invalid name, SLM Generation: FAILED.\n");
                SLMGenResult = false;
                return false;
            }

            //save bitmap of pattern centers:
            this.Dispatcher.Invoke((SLMSavePattern)delegate
            {
                System.Drawing.Bitmap bmp;
                string bmpPath = (null == WaveFileNameAndPathCurrent[2]) ? WaveFileNameAndPathCurrent[3] : _vm.SLMWaveformFolder[0] + "\\" + WaveFileNameAndPathCurrent[2] + ".bmp";
                string txtPath = (null == WaveFileNameAndPathCurrent[2]) ? WaveFileNameAndPathCurrent[3] : _vm.SLMWaveformFolder[0] + "\\" + WaveFileNameAndPathCurrent[2] + ".txt";

                if (_vm.IsStimulator)
                {
                    //save phase masks to expedite loading time, expecially combining two sets (of light source) as one,
                    //fetch wavelength by altering selected wavelength (give back user selection afterward),
                    //expected to be regenerated if new calibration.
                    bool backupWavelength = _vm.SLMSelectWavelength;
                    string[] bmpPathLamda = new string[_vm.SLMWavelengthCount];
                    for (int i = 0; i < _vm.SLMWavelengthCount; i++)
                    {
                        bmpPathLamda[i] = (null == WaveFileNameAndPathCurrent[2]) ? WaveFileNameAndPathCurrent[3] : _vm.SLMWaveformFolder[0] + "\\" + WaveFileNameAndPathCurrent[2] + i + ".bmp";
                        _vm.SLMSelectWavelength = (1 == i);
                        bmp = ProcessBitmap.CreateBinaryBitmap(new int[2] { _vm.ImageWidth, _vm.ImageHeight }, OverlayManagerClass.Instance.GetModeROIs(Mode.PATTERN_WIDEFIELD, OverlayManagerClass.Instance.PatternID, _vm.SLMWavelengthNM));
                        bmp.Save(bmpPathLamda[i], System.Drawing.Imaging.ImageFormat.Bmp);
                        _vm.SaveSLMPatternName(bmpPathLamda[i]);     //save phase masks with calibration
                    }
                    _vm.SLMSelectWavelength = backupWavelength;

                    //combine phase masks and rename afterward
                    if (1 < _vm.SLMWavelengthCount)
                    {
                        _vm.CombineHolograms(bmpPathLamda[0], bmpPathLamda[1]);
                        _vm.DeleteFile(bmpPathLamda[1]);
                    }
                    if (File.Exists(bmpPathLamda[0])) File.Move(bmpPathLamda[0], bmpPath);
                }
                else
                {
                    string strTmp = string.Empty;
                    int intTmp = 0;
                    Point offCenter = new Point(-1, -1);
                    List<Point> pts = OverlayManagerClass.Instance.GetPatternROICenters(OverlayManagerClass.Instance.PatternID, ref strTmp, ref offCenter); //before offset
                    bmp = ProcessBitmap.CreateBinaryBitmap(_vm.BleachLSMPixelXY[0], _vm.BleachLSMPixelXY[1], _slmPatternPoints);                            //after offset

                    //We save selected point image instead of phase mask,
                    //so that user can run history experiments with new calibrations:
                    bmp.Save(bmpPath, System.Drawing.Imaging.ImageFormat.Bmp);
                    _vm.SaveSLMPatternName(bmpPath);

                    //here we save non-offset patterns in a sub-folder if enabled in settings,
                    //for user to do roi analysis:
                    XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                    XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/BleachView");
                    if (XmlManager.GetAttribute(ndList[0], appSettings, "SLMPatternUnshifted", ref strTmp) && Int32.TryParse(strTmp, out intTmp))
                    {
                        if (1 == intTmp)
                        {
                            bmp = ProcessBitmap.CreateBinaryBitmap(_vm.BleachLSMPixelXY[0], _vm.BleachLSMPixelXY[1], pts);                                  //before offset

                            if (!Directory.Exists(bmpPath.Substring(0, bmpPath.LastIndexOf("\\")) + _vm.SLMbmpSubFolders[1]))
                            {
                                Directory.CreateDirectory(bmpPath.Substring(0, bmpPath.LastIndexOf("\\")) + _vm.SLMbmpSubFolders[1]);
                            }

                            bmp.Save(bmpPath.Substring(0, bmpPath.LastIndexOf("\\")) + _vm.SLMbmpSubFolders[1] + bmpPath.Substring(bmpPath.LastIndexOf("\\")), System.Drawing.Imaging.ImageFormat.Bmp);
                        }
                        else if (Directory.Exists(bmpPath.Substring(0, bmpPath.LastIndexOf("\\")) + _vm.SLMbmpSubFolders[1]))
                        {
                            Directory.Delete(bmpPath.Substring(0, bmpPath.LastIndexOf("\\")) + _vm.SLMbmpSubFolders[1], true);
                            Directory.Delete(txtPath.Substring(0, txtPath.LastIndexOf("\\")) + _vm.SLMbmpSubFolders[1], true);
                        }
                    }
                }
            });
            return true;
        }

        private void SetupLinePanel()
        {
            if (null != SLMEpochSequences)
            {
                for (int i = 0; i < SLMEpochSequences.Count; i++)
                {
                    linePanel.Children.Add(AddEpochLineGUI(SLMEpochSequences.ElementAt(i), i + 1));
                }
            }
        }

        private void SLMBuilder_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker worker = sender as BackgroundWorker;
            if (worker.CancellationPending != true)
            {
                SLMSpinProgressVisible = true;
                SLMGenResult = false;

                //determine clock rate for all patterns:
                int minPixelSpacing = Int32.MaxValue;
                uint maxWaveID = 0;
                for (int i = 0; i < _vm.SLMBleachWaveParams.Count; i++)
                {
                    minPixelSpacing = Math.Min(minPixelSpacing, _vm.SLMBleachWaveParams[i].PixelSpacing);
                    maxWaveID = Math.Max(maxWaveID, _vm.SLMBleachWaveParams[i].BleachWaveParams.ID);
                    _vm.SLMBleachWaveParams[i].BleachWaveParams.Fill = 1;   //always do fill ellipse
                }

                //verify durations for dwellTimes:
                for (int i = 0; i < _vm.SLMBleachWaveParams.Count; i++)
                {
                    WaveformBuilder.ClkRate = _vm.IsStimulator ? _vm.BleachInternalClockRate : (int)(_vm.BleachLSMPixelXY[0] * WaveformBuilder.MS_TO_S / minPixelSpacing);   //[Hz]
                    SLMParamsCurrent = new SLMParams(_vm.SLMBleachWaveParams[i]);
                    int stepCount = GetDwellCount(WaveformBuilder.ClkRate);
                    _vm.SLMBleachWaveParams[i].BleachWaveParams.DwellTime = _vm.SLMBleachWaveParams[i].Duration * (_vm.IsStimulator ? 1 : WaveformBuilder.MS_TO_S / stepCount);  //Duration [ms], DwellTime [us]
                    _vm.SLMBleachWaveParams[i].BleachWaveParams.ClockRate = WaveformBuilder.ClkRate;
                    _vm.SLMBleachWaveParams[i].BleachWaveParams.DeltaX_Px = WaveformBuilder.DeltaX_Px;

                    if (WaveformBuilder.MinDwellTime > _vm.SLMBleachWaveParams[i].BleachWaveParams.DwellTime)
                    {
                        SLMPatternStatus = String.Format("Duration of pattern name: " + SLMParamsCurrent.Name + " is not valid.\n");
                        SLMPatternStatus += String.Format("Minimum duration is {0} ms,\nwhile pixel spacing = {1}, width = {2} um, height = {3} um.\n",
                        Decimal.Round((Decimal)(WaveformBuilder.MinDwellTime * stepCount / WaveformBuilder.MS_TO_S), 3), minPixelSpacing,
                        SLMParamsCurrent.BleachWaveParams.ROIWidthUM, SLMParamsCurrent.BleachWaveParams.ROIHeightUM);
                        return;
                    }
                }

                //***   Start Generation   ***//
                bool firstEpoch = false, firstPattern = false, firstIteration = false, lastIteration = false, cycleBegin = false, cycleEnd = false, lastEpoch = false, lastPattern = false;
                byte patnTrig = (byte)0, epochTrig = (byte)0, cycleTrig = (byte)0;
                string digits = "D" + FileName.GetDigitCounts().ToString();
                int sequenceCount = _vm.SLMSequenceOn ? SLMEpochSequences.Count : 1; //build multiple epoch sequences
                for (int seq = 0; seq < sequenceCount; seq++)
                {
                    SLMFileSaved = false;
                    _vm.InitializeWaveformBuilder(WaveformBuilder.ClkRate);
                    WaveformBuilder.ResetWaveform();

                    //determine epoch count, wave params and file name
                    int epochCount = _vm.EpochCount;
                    ObservableCollection<SLMParams> slmBleachWaveParams = new ObservableCollection<SLMParams>(_vm.SLMBleachWaveParams.ToArray());
                    string pathName = _vm.SLMWaveformFolder[0] + "\\" + _vm.SLMWaveBaseName[0] + "_" + maxWaveID.ToString(digits) + ".raw";
                    string seqTextName = _vm.SLMWaveformFolder[1] + "\\" + _vm.SLMWaveBaseName[2] + "_" + (seq + 1).ToString(digits) + ".txt";

                    if (_vm.SLMSequenceOn)
                    {
                        if (0 >= SLMEpochSequences.ElementAt(seq).EpochCountInt)
                        {
                            SLMPatternStatus = String.Format("SLM sequence #" + (seq + 1).ToString() + " epoch count is not valid.\n");
                            return;
                        }
                        epochCount = SLMEpochSequences.ElementAt(seq).EpochCountInt;

                        int[] patternIDs = SLMEpochSequence.ParseForIntArray(SLMEpochSequences.ElementAt(seq).SequenceStr);
                        if (0 >= patternIDs.Length)
                        {
                            SLMPatternStatus = String.Format("SLM sequence #" + (seq + 1).ToString() + " epoch sequence is not valid.\n");
                            return;
                        }
                        for (int i = 0; i < patternIDs.Length; i++)
                        {
                            if ((0 >= patternIDs.Min()) || (_vm.SLMBleachWaveParams.Count < patternIDs[i]))
                            {
                                SLMPatternStatus = String.Format("SLM sequence #" + (seq + 1).ToString() + " epoch sequence is not valid.\n");
                                return;
                            }
                        }

                        if (!Directory.Exists(_vm.SLMWaveformFolder[1]))
                            Directory.CreateDirectory(_vm.SLMWaveformFolder[1]);

                        // build bleach params & save sequence txt file
                        _vm.DeleteFile(seqTextName);
                        slmBleachWaveParams = new ObservableCollection<SLMParams>();
                        StreamWriter seqTextFile = new StreamWriter(seqTextName);

                        SLMPatternStatus = String.Format("Saving SLM Sequence # " + (seq + 1).ToString() + " ...\n");

                        for (int i = 0; i < patternIDs.Length; i++)
                        {
                            //verify pattern time to be long enough for SLM runtime calculation
                            if (MIN_PATTERN_MS > _vm.SLMBleachWaveParams[patternIDs[i] - 1].BleachWaveParams.EstDuration * _vm.SLMBleachWaveParams[patternIDs[i] - 1].BleachWaveParams.Iterations)
                            {
                                SLMPatternStatus = String.Format("SLM sequence #" + (seq + 1).ToString() + " pattern time of ID #" + patternIDs[i].ToString() + " should be > " + MIN_PATTERN_MS.ToString() + " msec.\n");
                                seqTextFile.Close();
                                seqTextFile.Dispose();
                                return;
                            }
                            slmBleachWaveParams.Add(_vm.SLMBleachWaveParams[patternIDs[i] - 1]);
                            seqTextFile.WriteLine(patternIDs[i]);
                        }
                        seqTextFile.Close();
                        seqTextFile.Dispose();

                        pathName = _vm.SLMWaveformFolder[1] + "\\" + _vm.SLMWaveBaseName[0] + "_" + (seq + 1).ToString(digits) + ".raw";
                    }

                    _vm.DeleteFile(pathName);

                    for (int j = 0; j < epochCount; j++)
                    {
                        firstEpoch = (0 == j) ? true : false;
                        lastEpoch = (epochCount - 1 == j) ? true : false;

                        for (int i = 0; i < slmBleachWaveParams.Count; i++)
                        {
                            SLMPatternStatus = String.Format("Building SLM ...\nWaveform # " + (seq + 1).ToString() + ", Epoch # " + (j + 1).ToString() + ", Pattern # " + (i + 1).ToString() + "\n");

                            SLMParams slmParam = slmBleachWaveParams[i];
                            List<double> powerVal = new List<double>();
                            for (int pid = 0; pid < slmParam.BleachWaveParams.Power.Count(); pid++)
                            {
                                powerVal.Add(WaveformBuilder.GetPockelsPowerValue(slmParam.BleachWaveParams.Power[pid], _vm.BleachCalibratePockelsVoltageMin0[pid], _vm.BleachCalibratePockelsVoltageMax0[pid], (PockelsResponseType)MVMManager.Instance["PowerControlViewModel", "BleacherPowerResponse" + pid, PockelsResponseType.SINE_RESPONSE]));
                            }
                            firstPattern = (0 == i) ? true : false;
                            lastPattern = (slmBleachWaveParams.Count - 1 == i) ? true : false;

                            //Each iteration including PreIdle, Dwell, PostIdle:
                            for (int it = 0; it < slmParam.BleachWaveParams.Iterations; it++)
                            {
                                //First or last iteration / pattern:
                                firstIteration = (0 == it) ? true : false;
                                cycleBegin = (firstEpoch && firstPattern && firstIteration) ? true : false;
                                cycleTrig = (cycleBegin) ? (byte)0 : (byte)1;
                                epochTrig = (firstPattern && firstIteration) ? (byte)0 : (byte)1;
                                patnTrig = (firstIteration) ? (byte)0 : (byte)1;

                                //start from center which has been offset, for Galvo only:
                                if (!_vm.IsStimulator)
                                    WaveformBuilder.BuildTravel(new Point(slmParam.BleachWaveParams.Center.X + (slmParam.BleachWaveParams.ROIWidth / 2), slmParam.BleachWaveParams.Center.Y), cycleTrig, epochTrig, patnTrig);

                                //pattern pre-idle:
                                if (firstIteration)
                                    WaveformBuilder.BuildPrePatIdle(slmParam.BleachWaveParams, cycleBegin, firstPattern);

                                //iteration pre-idle:
                                WaveformBuilder.BuildPreIdle(slmParam.BleachWaveParams);

                                //dwell countour fill:
                                if (!_vm.IsStimulator)
                                {
                                    switch (slmParam.BleachWaveParams.ROIWidthUM.ToString())
                                    {
                                        case "0":
                                            WaveformBuilder.BuildSpot(slmParam.BleachWaveParams, powerVal.ToArray());
                                            break;
                                        default:
                                            WaveformBuilder.BuildEllipse(slmParam.BleachWaveParams, powerVal.ToArray());
                                            break;
                                    }
                                }
                                else
                                {
                                    WaveformBuilder.BuildSpot(slmParam.BleachWaveParams, powerVal.ToArray());
                                }

                                lastIteration = ((slmParam.BleachWaveParams.Iterations - 1) == it) ? true : false;
                                cycleEnd = (lastEpoch && lastIteration && lastPattern) ? true : false;

                                //iteration post-idle:
                                WaveformBuilder.BuildPostIdle(slmParam.BleachWaveParams);

                                //pattern post-idle:
                                if (lastIteration)
                                    WaveformBuilder.BuildPostPatIdle(slmParam.BleachWaveParams, lastIteration && lastPattern, cycleEnd);

                                //NI limit data length per channel or stop request:
                                if ((WaveformBuilder.GetWaveform().Count > Int32.MaxValue) || (true == slmBuilder.CancellationPending))
                                {
                                    SLMPatternStatus = String.Format("Waveform is too large.\n");
                                    return;
                                }
                            }
                        }
                    }
                    //Return to start position & signal cycle completed:
                    WaveformBuilder.ReturnHome(true);

                    //save waveform:
                    SLMPatternStatus = String.Format("Saving SLM Waveform # " + (seq + 1).ToString() + " ...\n");

                    WaveformBuilder.SaveWaveform(pathName, true, new bool[3] { !_vm.IsStimulator, true, true });

                    while (!WaveformBuilder.CheckSaveState())
                    {
                        System.Threading.Thread.Sleep(50);

                        if (true == slmWorker.CancellationPending)
                        {
                            WaveformBuilder.StopSave();
                        }
                    }

                    SLMFileSaved = WaveformBuilder.GetSaveResult();

                    if (!SLMFileSaved)
                    {
                        _vm.DeleteFile(seqTextName);
                        SLMPatternStatus = String.Format("Saving SLM Waveform # " + (seq + 1).ToString() + " failed.\n");
                        return;
                    }
                }
            }
        }

        private void SLMBuilder_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            SLMSpinProgressVisible = false;
            if (e.Cancelled)
            {
                SLMPatternStatus = String.Format("Cancelled building waveforms.\n ");
            }
            else if (!SLMFileSaved)
            {
                SLMPatternStatus += String.Format("Failed to save or build one or more waveforms.\n ");
            }
            else
            {
                _vm.UpdateSLMCompParams();
                SLMGenResult = true;
            }
            slmBuilder.DoWork -= new DoWorkEventHandler(SLMBuilder_DoWork);
            slmBuilder.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(SLMBuilder_RunWorkerCompleted);
            if (SLMGenResult)
            {
                //update epoch sequences in view model:
                _vm.EpochSequence = new List<SLMEpochSequence>(SLMEpochSequences.Select(s => s.Clone()).Cast<SLMEpochSequence>()).Select(s =>
                {
                    s.SequenceStr = SLMEpochSequence.IntArrayToString(SLMEpochSequence.ParseForIntArray(s.SequenceStr));
                    return s;
                }).ToList();

                //persist slm params in all modalities:
                _vm.PersistGlobalExperimentXML(GlobalExpAttribute.SLM_BLEACH);
                this.Close();
            }
        }

        void SLMCalibrator_DoWork(object sender, DoWorkEventArgs e)
        {
            //create mask, PatternID 1 are targets to be burnt,
            //PatternID 2 are sources for affine calibration:
            List<Point> calPointsTo = new List<Point>();
            List<Point> calPointsFrom = new List<Point>();
            SLMSpinProgressVisible = true;
            _vm.SLMCalibZPos = (double)MVMManager.Instance["ZControlViewModel", "ZPosition", (object)0];
            this.Dispatcher.Invoke((SLMSavePattern)delegate
            {
                //check if calibration ROI is available:
                OverlayManager.ROICapsule roiCapsule = OverlayManager.OverlayManagerClass.LoadXamlROIs((string)e.Argument);
                if (null == roiCapsule)
                {
                    e.Result = "No Calibration ROIs.\n";
                    return;
                }
                //From pattern preset to user-select:
                for (int i = 0; i < roiCapsule.ROIs.Length; i++)
                {
                    if (roiCapsule.ROIs[i].GetType() == typeof(OverlayManager.ROICrosshair))
                    {
                        //do not include center (subID == 0)
                        if (0 == ((int[])(((OverlayManager.ROICrosshair)(roiCapsule.ROIs[i])).Tag))[(int)ThorSharedTypes.Tag.SUB_PATTERN_ID])
                            continue;

                        switch (((int[])(((OverlayManager.ROICrosshair)(roiCapsule.ROIs[i])).Tag))[(int)ThorSharedTypes.Tag.PATTERN_ID])
                        {
                            case 1:
                                calPointsTo.Add(((OverlayManager.ROICrosshair)(roiCapsule.ROIs[i])).CenterPoint);
                                break;
                            case 2:
                                calPointsFrom.Add(((OverlayManager.ROICrosshair)(roiCapsule.ROIs[i])).CenterPoint);
                                break;
                        }
                    }
                    else if (roiCapsule.ROIs[i].GetType() == typeof(OverlayManager.ROIEllipse))
                    {
                        //do not include center (subID == 0)
                        if (0 == ((int[])(((OverlayManager.ROICrosshair)(roiCapsule.ROIs[i])).Tag))[(int)ThorSharedTypes.Tag.SUB_PATTERN_ID])
                            continue;

                        switch (((int[])(((OverlayManager.ROICrosshair)(roiCapsule.ROIs[i])).Tag))[(int)ThorSharedTypes.Tag.PATTERN_ID])
                        {
                            case 1:
                                calPointsTo.Add(((OverlayManager.ROIEllipse)(roiCapsule.ROIs[i])).ROICenter);
                                break;
                            case 2:
                                calPointsFrom.Add(((OverlayManager.ROIEllipse)(roiCapsule.ROIs[i])).ROICenter);
                                break;
                        }
                    }
                }
            });

            float[] ptsTo = PointsToFloatVec(calPointsTo);
            float[] ptsFrom = (0 == calPointsFrom.Count) ? ptsTo : PointsToFloatVec(calPointsFrom);

            //check if both are valid counts:
            if (((SLM_CALIB_PTS * 2) != ptsTo.Length) || ((SLM_CALIB_PTS * 2) != ptsFrom.Length))
            {
                e.Result = "Invalid count of points.\n";
                return;
            }

            //create mask for later used slm:
            int[] pixelXY = _vm.IsStimulator ? _vm.SLMPixelXY : _vm.BleachLSMPixelXY;
            System.Drawing.Bitmap bmp = ProcessBitmap.CreateBinaryBitmap(pixelXY[0], pixelXY[1], calPointsTo);
            string bmpPath = _vm.BleachROIPath + "SLMCalibROIs.bmp";
            bmp.Save(bmpPath, System.Drawing.Imaging.ImageFormat.Bmp);

            //create center Galvo waveform file, also determine if need reload:
            e.Result = BuildCenterBleachWaveform();

            //execute calibration:
            if (0 == e.Result.ToString().CompareTo("0"))
            {
                SLMPatternStatus = String.Format("Loading SLM Calibration Waveform, \nplease wait until done ... \n");
                e.Result = (_vm.SLMCalibration(bmpPath, ptsFrom, ptsTo, ptsTo.Length, pixelXY[0], pixelXY[1], _vm.SLMCalibZPos)) ? "0" : "Calibration Failed\n";
            }
        }

        private void SLMCalibrator_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            SLMCalibPanel slmCalib;
            string roiPathName = _vm.BleachROIPath + "SLMCalibROIs.xaml";
            //clear user-selected points:
            OverlayManagerClass.Instance.DeletePatternROI(ref CaptureSetupViewModel.OverlayCanvas, OverlayManagerClass.Instance.PatternID);
            OverlayManagerClass.Instance.SaveROIs(roiPathName);
            _vm.DisplayROI(roiPathName);
            OverlayManagerClass.Instance.DisplayPatternROI(ref CaptureSetupViewModel.OverlayCanvas, OverlayManagerClass.Instance.PatternID - 1, true);

            // check error, cancel, then result:
            if (e.Error != null)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "SLMCalibrator: " + e.Error.Message);
            }
            else if (!e.Cancelled)
            {
                try
                {
                    slmCalib = (0 == ((string)e.Result).CompareTo("0")) ?
                        new SLMCalibPanel("DONE", "REDO", "Verify Calibration", "Do the calibration spots align with the overlaid \ncalibration pattern?\n\n[NOTE: Click 'NO' will reset calibrations.]\n") :
                        new SLMCalibPanel("REDO", "DONE", "Verify Calibration", "Error: " + e.Result + "Click No to exit or \nYes to try again?");
                    this.DataContext = slmCalib;
                    this.Show();

                    if (0 == ((string)e.Result).CompareTo("0"))
                    {
                        //update calibration datetime:
                        _vm.SLMLastCalibTimeUnix = ResourceManagerCS.DateTimeToUnixTimestamp(DateTime.Now);

                        //persist slm params in all modalities:
                        _vm.PersistGlobalExperimentXML(GlobalExpAttribute.SLM_BLEACH);
                    }
                    SLMPatternStatus = string.Empty;
                }
                catch (Exception ex)
                {
                    ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "SLMCalibrator: " + ex.Message);
                }
            }

            //done:
            SLMSpinProgressVisible = false;
            slmCalibrator.DoWork -= new DoWorkEventHandler(SLMCalibrator_DoWork);
            slmCalibrator.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(SLMCalibrator_RunWorkerCompleted);
        }

        void SLMParamEditWin_Closed(object sender, EventArgs e)
        {
            try
            {
                CancelSLMBackgroundWorker(slmBuilder, false);
                CancelSLMBackgroundWorker(slmWorker, true);
                CancelCalibrator();

                _vm.ROIToolVisible = new bool[14] { true, true, true, true, true, true, true, true, true, true, true, true, true, true };
                _vm.SLMPanelAvailable = true;

                //OverlayManagerClass.Instance.ClearNonSaveROIs(ref _vm.OverlayCanvas);
                OverlayManagerClass.Instance.RevokeROIs(ref CaptureSetupViewModel.OverlayCanvas);
                OverlayManagerClass.Instance.CurrentMode = ThorSharedTypes.Mode.STATSONLY;

                if (SLMPanelMode.Calibration != PanelMode)
                {
                    //persist ActiveROIs:
                    OverlayManagerClass.Instance.PersistSaveROIs();
                }

                //display ActiveROIs:
                _vm.DisplayROI();

                Application.Current.MainWindow.Activate();
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "SLMParamEditWin_Closed: " + ex.Message);
            }
        }

        void SLMParamEditWin_Loaded(object sender, RoutedEventArgs e)
        {
            stpWavelength.DataContext = _vm;
            SLMSpinProgressVisible = false;
            WaveFileNameToSave = new FileName[] { new FileName(_vm.SLMWaveBaseName[0]), new FileName(_vm.SLMWaveBaseName[1]) };
            WaveFileNameAndPathCurrent = new string[4];

            slmWorker = new BackgroundWorker();
            slmWorker.WorkerReportsProgress = false;
            slmWorker.WorkerSupportsCancellation = true;

            slmCalibrator = new BackgroundWorker();
            slmCalibrator.WorkerReportsProgress = false;
            slmCalibrator.WorkerSupportsCancellation = true;

            slmBuilder = new BackgroundWorker();
            slmBuilder.WorkerReportsProgress = false;
            slmBuilder.WorkerSupportsCancellation = true;

            OverlayManagerClass.Instance.ObjectSizeChangedEvent += OverlayManager_ObjectSizeChangedEvent;
            OverlayManagerClass.Instance.UpdatingObjectEvent += OverlayManager_UpdatingObjectEvent;
            OverlayManagerClass.Instance.ClearedObjectEvent += OverlayManager_ClearedObjectEvent;
        }

        void SLMParamEditWin_Unloaded(object sender, RoutedEventArgs e)
        {
            _vm.SLMParamEditWin = null;
            OverlayManagerClass.Instance.ObjectSizeChangedEvent -= OverlayManager_ObjectSizeChangedEvent;
            OverlayManagerClass.Instance.UpdatingObjectEvent -= OverlayManager_UpdatingObjectEvent;
            OverlayManagerClass.Instance.ClearedObjectEvent -= OverlayManager_ClearedObjectEvent;
        }

        void SLMParamsCurrent_BleachParamsChangedEvent()
        {
            //update size of all ROIs under current pattern ID
            OverlayManagerClass.Instance.UpdatePatternROISize(ref CaptureSetupViewModel.OverlayCanvas, _slmParamsCurrent.BleachWaveParams.ROIWidth, _slmParamsCurrent.BleachWaveParams.ROIHeight);
            UpdateSLMParamEdit();
        }

        private void SLMWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker worker = sender as BackgroundWorker;
            if (worker.CancellationPending != true)
            {
                SaveSLMFiles();
                SLMGenResult = ValidateWaveformBeforeBuild();
                if (SLMGenResult)
                {
                    SLMFileSaved = true;
                    _waveParamsUpdated = false;
                    UpdateWaveParamsAndFile();
                }
            }
        }

        private void SLMWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            SLMSpinProgressVisible = false;
            if (e.Cancelled == true)
            {
                SLMPatternStatus = String.Format("Cancelled SLM Generation.\n ");
                UpdateWaveParamsAndFile();
                SLMGenResult = false;
            }

            slmWorker.DoWork -= new DoWorkEventHandler(SLMWorker_DoWork);
            slmWorker.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(SLMWorker_RunWorkerCompleted);

            if (SLMGenResult)
            {
                RebuildSLMWaveform();
            }
        }

        private bool TryImportPatternSequences(XmlDocument xmlSLMImportDoc)
        {
            string str1 = string.Empty, str2 = string.Empty, str3 = string.Empty;
            double dVal1 = 0, dVal2 = 0, dVal3 = 0, dVal4 = 0;
            int iVal = 0;
            bool workStatus = true;

            if (0 >= _vm.BleachLSMUMPerPixel)
            {
                SLMPatternStatus = String.Format("Calibration must be done before import.");
                return false;
            }

            XmlNodeList ndList = xmlSLMImportDoc.SelectNodes("/ThorImageSLM/SLMPatterns/Pattern");
            if (0 < ndList.Count)
            {
                //load patterns on top of waveforms, request permission to overwrite
                if (0 < _vm.SLMBleachWaveParams.Count)
                {
                    MessageBoxResult dresult = MessageBox.Show("Old patterns will be cleared, do you want to continue?", "Import SLM Patterns", MessageBoxButton.YesNo, MessageBoxImage.Question);
                    if (dresult == MessageBoxResult.No)
                    {
                        SLMPatternStatus = String.Format("Please backup the SLM patterns (as a template) before import or select another xml.");
                        return false;
                    }
                    else if (dresult == MessageBoxResult.Yes)
                    {
                        _vm.SLMBleachWaveParams.Clear();
                    }
                }
                //clear history
                _vm.SLM3D = _vm.SLMSequenceOn = false;
                OverlayManagerClass.Instance.DeletePatternROI(ref CaptureSetupViewModel.OverlayCanvas);
                for (int i = 0; i < _vm.SLMWaveformFolder.Length; i++)
                    _vm.ClearSLMFiles(_vm.SLMWaveformFolder[i]);

                //backup properties
                ThorSharedTypes.Mode backupMode = OverlayManagerClass.Instance.CurrentMode;
                OverlayManagerClass.Instance.CurrentMode = ThorSharedTypes.Mode.PATTERN_NOSTATS;

                //start to draw
                OverlayManagerClass.Instance.InitSelectROI(ref CaptureSetupViewModel.OverlayCanvas);
                for (int i = 0; i < ndList.Count; i++)
                {
                    //locate patternID in order, [1-based]
                    XmlNode xNode = null;
                    foreach (XmlNode nd in ndList)
                    {
                        if (XmlManager.GetAttribute(nd, xmlSLMImportDoc, "patternID", ref str1) && Int32.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal) && i + 1 == iVal)
                        {
                            xNode = nd;
                            break;
                        }
                    }
                    if (null == xNode)
                    {
                        SLMPatternStatus = String.Format("'patternID' is required and must be in order: ID(" + (i + 1).ToString() + ") is missing.");
                        workStatus = false;
                        break;
                    }
                    GeometryUtilities.SLMParams sparam = new GeometryUtilities.SLMParams();
                    sparam.Name = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "name", ref str1) && !string.IsNullOrEmpty(str1) && !ContainsInvalidPathCharacters(str1)) ? str1 : ("Pattern" + string.Format("{0:000}", iVal));
                    SLMParamID = iVal - 1;      //index in waveform param list [0-based]
                    OverlayManagerClass.Instance.PatternID = iVal;
                    sparam.BleachWaveParams.ID = (uint)iVal;
                    sparam.BleachWaveParams.UMPerPixel = (double)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (object)1.0];
                    sparam.BleachWaveParams.UMPerPixelRatio = (0 < _vm.BleachLSMUMPerPixel) ? sparam.BleachWaveParams.UMPerPixel / _vm.BleachLSMUMPerPixel : 1.0;

                    if ((XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "red", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) &&
                        (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "green", ref str2) && Double.TryParse(str2, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal2)) &&
                        (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "blue", ref str3) && Double.TryParse(str3, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal3)))
                    {
                        System.Collections.Specialized.BitVector32 bitVec32 = new System.Collections.Specialized.BitVector32(Convert.ToByte(dVal1));
                        bitVec32[OverlayManager.OverlayManagerClass.SecG] = Convert.ToByte(dVal2);
                        bitVec32[OverlayManager.OverlayManagerClass.SecB] = Convert.ToByte(dVal3);
                        OverlayManagerClass.Instance.ColorRGB = bitVec32.Data;

                        sparam.Red = dVal1;
                        sparam.Green = dVal2;
                        sparam.Blue = dVal3;
                    }
                    else
                    {
                        SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": 'red','green' or 'blue'\n");
                        workStatus = false;
                        break;
                    }

                    dVal1 = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "roiWidthPx", ref str1) && Double.TryParse(str1, out dVal1)) ? dVal1 : 0;
                    dVal2 = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "roiHeightPx", ref str2) && Double.TryParse(str2, out dVal2)) ? dVal2 : 0;

                    Type stype = ((dVal1 == 0 && dVal2 == 0) ||
                        (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "shape", ref str3) && 0 <= str3.IndexOf("Crosshair", StringComparison.OrdinalIgnoreCase))) ?
                        typeof(ROICrosshair) : typeof(ROIEllipse);

                    sparam.BleachWaveParams.ROIWidth = (typeof(ROICrosshair) == stype) ? 0 : dVal1;
                    sparam.BleachWaveParams.ROIHeight = (typeof(ROICrosshair) == stype) ? 0 : dVal2;

                    sparam.PixelSpacing = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "pxSpacing", ref str1) && Int32.TryParse(str1, out iVal) && (1 <= iVal)) ? iVal : 1;

                    if (!XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "iterations", ref str1) || !Int32.TryParse(str1, out iVal))
                    {
                        SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": iterations\n");
                        workStatus = false;
                        break;
                    }
                    sparam.BleachWaveParams.Iterations = iVal;

                    if (!XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "prePatIdleMS", ref str1) || !Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))
                    {
                        SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": prePatIdleMS\n");
                        workStatus = false;
                        break;
                    }
                    sparam.BleachWaveParams.PrePatIdleTime = dVal1;

                    if (!XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "postPatIdleMS", ref str1) || !Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))
                    {
                        SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": postPatIdleMS\n");
                        workStatus = false;
                        break;
                    }
                    sparam.BleachWaveParams.PostPatIdleTime = dVal1;

                    if (!XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "preIteIdleMS", ref str1) || !Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))
                    {
                        SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": preIteIdleMS\n");
                        workStatus = false;
                        break;
                    }
                    sparam.BleachWaveParams.PreIdleTime = dVal1;

                    if (!XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "postIteIdleMS", ref str1) || !Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))
                    {
                        SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": postIteIdleMS\n");
                        workStatus = false;
                        break;
                    }
                    sparam.BleachWaveParams.PostIdleTime = dVal1;

                    if (!XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "power", ref str1) || !Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))
                    {
                        SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": power\n");
                        workStatus = false;
                        break;
                    }
                    sparam.BleachWaveParams.Power = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "power1", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal2)) ?
                        new double[2] { dVal1, dVal2 } : new double[1] { dVal1 };

                    dVal1 = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "measurePowerMW", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ? dVal1 : 0.0;
                    sparam.BleachWaveParams.MeasurePower = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "measurePower1MW", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal2)) ?
                        new double[2] { dVal1, dVal2 } : new double[1] { dVal1 };

                    dVal1 = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "measurePowerMWPerUM2", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ? dVal1 : 0.0;
                    sparam.SLMMeasurePowerArea = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "measurePower1MWPerUM2", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal2)) ?
                         new double[2] { dVal1, dVal2 } : new double[1] { dVal1 };

                    sparam.Red = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "red", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ? dVal1 : 128;

                    sparam.Green = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "green", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ? dVal1 : 128;

                    sparam.Blue = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "blue", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ? dVal1 : 128;

                    if (!XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "durationMS", ref str1) || !Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))
                    {
                        SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": durationMS\n");
                        workStatus = false;
                        break;
                    }
                    sparam.Duration = dVal1;

                    //draw in order of subID from 1 and draw 0 as the last if exist
                    Dictionary<int, int> subIDdic = new Dictionary<int, int>();
                    int iLoc = 0;
                    foreach (XmlNode childNode in xNode.ChildNodes)
                    {
                        if (0 > childNode.Name.IndexOf("ROI", StringComparison.OrdinalIgnoreCase))
                            continue;

                        if (XmlManager.GetAttribute(childNode, xmlSLMImportDoc, "subID", ref str1) && Int32.TryParse(str1, out iVal))
                            subIDdic.Add(iVal, iLoc);

                        iLoc++;
                    }
                    bool firstDraw = true;  //only the first ellipse takes start-end, the rest center only
                    for (int id = 1; id < subIDdic.Count; id++)
                    {
                        if (subIDdic.ContainsKey(id))
                        {
                            if (XmlManager.GetAttribute(xNode.ChildNodes[subIDdic[id]], xmlSLMImportDoc, "centerX", ref str1) && Double.TryParse(str1, out dVal3) &&
                                XmlManager.GetAttribute(xNode.ChildNodes[subIDdic[id]], xmlSLMImportDoc, "centerY", ref str2) && Double.TryParse(str2, out dVal4))
                            {
                                OverlayManagerClass.Instance.CreateROIShape(ref CaptureSetupViewModel.OverlayCanvas,
                                    stype,
                                    firstDraw ? new Point(dVal3 - sparam.BleachWaveParams.ROIWidth / 2, dVal4 - sparam.BleachWaveParams.ROIHeight / 2) : new Point(dVal3, dVal4),
                                    new Point(dVal3 + sparam.BleachWaveParams.ROIWidth / 2, dVal4 + sparam.BleachWaveParams.ROIHeight / 2));
                                OverlayManagerClass.Instance.InitSelectROI(ref CaptureSetupViewModel.OverlayCanvas);
                                firstDraw = false;
                            }
                        }
                    }
                    //set ROI center at the end if provided as subID = 0, fetch otherwise
                    if (subIDdic.ContainsKey(0))
                    {
                        if (XmlManager.GetAttribute(xNode.ChildNodes[subIDdic[0]], xmlSLMImportDoc, "centerX", ref str1) && Double.TryParse(str1, out dVal3) &&
                             XmlManager.GetAttribute(xNode.ChildNodes[subIDdic[0]], xmlSLMImportDoc, "centerY", ref str2) && Double.TryParse(str2, out dVal4))
                        {
                            OverlayManagerClass.Instance.CreateROIShape(ref CaptureSetupViewModel.OverlayCanvas, typeof(ROICrosshair), new Point(dVal3, dVal4), new Point(dVal3, dVal4));
                            OverlayManagerClass.Instance.InitSelectROI(ref CaptureSetupViewModel.OverlayCanvas);
                            sparam.BleachWaveParams.Center = new Point(dVal3, dVal4);
                        }
                    }
                    else
                    {
                        OverlayManagerClass.Instance.CreateROICenter(ref CaptureSetupViewModel.OverlayCanvas);
                        OverlayManagerClass.Instance.ValidateROIs(ref CaptureSetupViewModel.OverlayCanvas);
                        Point offCenter = new Point(-1, -1);
                        OverlayManagerClass.Instance.GetPatternROICenters(OverlayManagerClass.Instance.PatternID, ref str1, ref offCenter);
                        if ((0 > offCenter.X) || (0 > offCenter.Y))
                        {
                            SLMPatternStatus = String.Format("Unable to calculate ROIs center.");
                            workStatus = false;
                            break;
                        }
                        sparam.BleachWaveParams.Center = offCenter;
                    }
                    sparam.BleachWaveParams.Fill = 1;

                    SLMParamsCurrent = new SLMParams(sparam);
                    this.Dispatcher.Invoke((SLMWorkerSetGUI)delegate
                    {
                        _vm.SLMBleachWaveParams.Add(SLMParamsCurrent);
                    });

                    SaveSLMFiles();
                    if (!ValidateWaveformBeforeBuild())
                    {
                        SLMPatternStatus = String.Format("Unable to save SLM Pattern: " + sparam.Name);
                        workStatus = false;
                        break;
                    }
                    SLMFileSaved = true;
                    _waveParamsUpdated = false;
                    UpdateWaveParamsAndFile();
                }

                //end for loop of ndList, give back properties,
                //and clear-then-return if break due to error
                OverlayManagerClass.Instance.CurrentMode = backupMode;
                if (workStatus)
                {
                    OverlayManagerClass.Instance.PersistSaveROIs();
                }
                else
                {
                    this.Dispatcher.Invoke((SLMWorkerSetGUI)delegate
                    {
                        _vm.SLMBleachWaveParams.Clear();
                    });
                    OverlayManagerClass.Instance.DeletePatternROI(ref CaptureSetupViewModel.OverlayCanvas);
                    for (int id = 0; id < _vm.SLMWaveformFolder.Length; id++)
                        _vm.ClearSLMFiles(_vm.SLMWaveformFolder[id]);

                    return false;
                }
            }

            //import sequences, clear first since new patterns are loaded,
            //must accompanied with new sequences
            _vm.EpochSequence = new List<SLMEpochSequence>();
            _vm.ClearSLMFiles(_vm.SLMWaveformFolder[1], "txt");
            _vm.ClearSLMFiles(_vm.SLMWaveformFolder[1], "raw");
            ndList = xmlSLMImportDoc.SelectNodes("/ThorImageSLM/SLMSequences/SequenceEpoch");
            if (0 < ndList.Count)
            {
                _vm.SLMSequenceOn = true;
                for (int i = 0; i < ndList.Count; i++)
                {
                    if (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "sequence", ref str1) &&
                        (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "sequenceEpochCount", ref str2) && Int32.TryParse(str2, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                        )
                    {
                        _vm.EpochSequence.Add(new SLMEpochSequence(str1, iVal, _vm.SLMBleachWaveParams.Count));
                    }
                }
            }
            return true;
        }

        private bool TryImportWaveforms(XmlDocument xmlSLMImportDoc)
        {
            string str1 = string.Empty, str2 = string.Empty;
            double dVal1 = 0, dVal2 = 0;
            int iVal = 0;

            //find current file id
            uint fID = 0;
            for (int j = 0; j < _vm.SLMBleachWaveParams.Count(); j++)
            {
                fID = Math.Max(fID, _vm.SLMBleachWaveParams[j].BleachWaveParams.ID);
            }

            XmlNodeList ndList = xmlSLMImportDoc.SelectNodes("/ThorImageExperiment/SLM/Pattern");
            if (0 >= ndList.Count)
                return false;

            ObservableCollection<SLMParams> xmlSLMParams = new ObservableCollection<SLMParams>();
            for (int i = 0; i < ndList.Count; i++)
            {
                GeometryUtilities.SLMParams sparam = new GeometryUtilities.SLMParams();
                if (!XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "name", ref str1))
                {
                    SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": name\n");
                    return false;
                }
                if (ContainsInvalidPathCharacters(str1))
                {
                    SLMPatternStatus = String.Format("Invalid pattern name: " + str1);
                    return false;
                }
                sparam.Name = str1;

                if (!XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "pixelSizeUM", ref str1) || !Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))
                {
                    SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": pixelSizeUM\n");
                    return false;
                }
                sparam.BleachWaveParams.UMPerPixel = dVal1;
                sparam.BleachWaveParams.UMPerPixelRatio = (0 < _vm.BleachLSMUMPerPixel) ? sparam.BleachWaveParams.UMPerPixel / _vm.BleachLSMUMPerPixel : 1.0;

                if (!XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "xOffsetUM", ref str1) || !XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "yOffsetUM", ref str2))
                {
                    SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": xOffsetUM\n");
                    return false;
                }
                if (!Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1) || !Double.TryParse(str2, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal2))
                {
                    SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": OffsetUM\n");
                    return false;
                }
                sparam.BleachWaveParams.CenterUM = new Point(dVal1, dVal2);

                if (!XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "roiWidthUM", ref str1) || !Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))
                {
                    SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": roiWidthUM\n");
                    return false;
                }
                sparam.BleachWaveParams.ROIWidthUM = dVal1;

                if (!XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "roiHeightUM", ref str1) || !Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))
                {
                    SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": roiHeightUM\n");
                    return false;
                }
                sparam.BleachWaveParams.ROIHeightUM = dVal1;

                if (!XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "pxSpacing", ref str1) || !Int32.TryParse(str1, out iVal) || (1 > iVal))
                {
                    SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": pxSpacing\n");
                    return false;
                }
                sparam.PixelSpacing = iVal;

                if (!XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "iterations", ref str1) || !Int32.TryParse(str1, out iVal))
                {
                    SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": iterations\n");
                    return false;
                }
                sparam.BleachWaveParams.Iterations = iVal;

                if (!XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "prePatIdleMS", ref str1) || !Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))
                {
                    SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": prePatIdleMS\n");
                    return false;
                }
                sparam.BleachWaveParams.PrePatIdleTime = dVal1;

                if (!XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "postPatIdleMS", ref str1) || !Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))
                {
                    SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": postPatIdleMS\n");
                    return false;
                }
                sparam.BleachWaveParams.PostPatIdleTime = dVal1;

                if (!XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "preIteIdleMS", ref str1) || !Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))
                {
                    SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": preIteIdleMS\n");
                    return false;
                }
                sparam.BleachWaveParams.PreIdleTime = dVal1;

                if (!XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "postIteIdleMS", ref str1) || !Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))
                {
                    SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": postIteIdleMS\n");
                    return false;
                }
                sparam.BleachWaveParams.PostIdleTime = dVal1;

                if (!XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "power", ref str1) || !Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))
                {
                    SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": power\n");
                    return false;
                }
                sparam.BleachWaveParams.Power = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "power1", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal2)) ?
                    new double[2] { dVal1, dVal2 } : new double[1] { dVal1 };

                dVal1 = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "measurePowerMW", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ? dVal1 : 0.0;
                sparam.BleachWaveParams.MeasurePower = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "measurePower1MW", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal2)) ?
                    new double[2] { dVal1, dVal2 } : new double[1] { dVal1 };

                dVal1 = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "measurePowerMWPerUM2", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ? dVal1 : 0.0;
                sparam.SLMMeasurePowerArea = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "measurePower1MWPerUM2", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal2)) ?
                     new double[2] { dVal1, dVal2 } : new double[1] { dVal1 };

                sparam.Red = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "red", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ? dVal1 : 128;

                sparam.Green = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "green", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ? dVal1 : 128;

                sparam.Blue = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "blue", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ? dVal1 : 128;

                if (!XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "durationMS", ref str1) || !Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))
                {
                    SLMPatternStatus = String.Format("Error at getting pattern " + sparam.Name + ": durationMS\n");
                    return false;
                }
                sparam.Duration = dVal1;
                sparam.BleachWaveParams.ID = (uint)(fID + i + 1);
                sparam.BleachWaveParams.Fill = 1;
                xmlSLMParams.Add(sparam);
            }

            //get duration varified
            for (int i = 0; i < xmlSLMParams.Count; i++)
            {
                SLMParamsCurrent = new SLMParams(xmlSLMParams[i]);
                if (!ValidateWaveformBeforeBuild())
                    return false;
            }

            for (int i = 0; i < xmlSLMParams.Count(); i++)
            {
                this.Dispatcher.Invoke((SLMWorkerSetGUI)delegate
                {
                    SLMParams paramCopy = new SLMParams(xmlSLMParams[i]);
                    _vm.SLMBleachWaveParams.Add(paramCopy);
                });
            }
            return true;
        }

        void UpdateSLMParamEdit()
        {
            if (null != _slmParamsCurrent)
            {
                //calculate power area density
                double areaUM2 = (0 == _slmParamsCurrent.BleachWaveParams.ROIWidth && 0 == _slmParamsCurrent.BleachWaveParams.ROIHeight) ?
                    (Math.Pow((_slmParamsCurrent.BleachWaveParams.UMPerPixel / _slmParamsCurrent.BleachWaveParams.UMPerPixelRatio), 2)) :
                    Math.PI * Math.Max(1, _slmParamsCurrent.BleachWaveParams.ROIWidth) * Math.Max(1, _slmParamsCurrent.BleachWaveParams.ROIHeight) / 4 * Math.Pow((_slmParamsCurrent.BleachWaveParams.UMPerPixel / _slmParamsCurrent.BleachWaveParams.UMPerPixelRatio), 2);
                _slmParamsCurrent.SLMMeasurePowerArea = new double[_slmParamsCurrent.BleachWaveParams.MeasurePower.Length];
                for (int i = 0; i < _slmParamsCurrent.BleachWaveParams.MeasurePower.Length; i++)
                {
                    _slmParamsCurrent.SLMMeasurePowerArea[i] = (0 >= areaUM2 || 0 >= OverlayManagerClass.Instance.GetPatternROICount()) ? 0.0 : Math.Round(_slmParamsCurrent.BleachWaveParams.MeasurePower[i] / OverlayManagerClass.Instance.GetPatternROICount() / areaUM2, 6);
                }
            }
        }

        private void UpdateWaveParamsAndFile()
        {
            //force to be invoked once per:
            if (_waveParamsUpdated)
                return;

            //save LSM pattern to a 8bit bitmap:
            if (false == SaveSLMPattern())
                return;

            //handle WaveParams before return:
            if (SLMFileSaved)
            {
                if (null == WaveFileNameAndPathCurrent[2])
                {
                    //update ID with unique:
                    SLMParamsCurrent.BleachWaveParams.ID = WaveFileNameToSave[1].NameNumberInt;
                }

                //persist WaveParams:
                this.Dispatcher.Invoke((SLMWorkerSetGUI)delegate
                {
                    //add or edit list item:
                    SLMParams paramCopy = new SLMParams(SLMParamsCurrent);

                    if (0 > SLMParamID)
                    {
                        _vm.SLMBleachWaveParams.Add(paramCopy);
                    }
                    else
                    {
                        _vm.SLMBleachWaveParams[SLMParamID] = paramCopy;
                    }

                    //update list view:
                    _vm.UpdateSLMListGUI();

                    //save ROIs:
                    OverlayManagerClass.Instance.SetPatternToSaveROI(OverlayManagerClass.Instance.PatternID,
                        (_vm.IsStimulator ? ThorSharedTypes.Mode.PATTERN_WIDEFIELD : ThorSharedTypes.Mode.PATTERN_NOSTATS));

                    //backup ROIs for not being revoked:
                    OverlayManagerClass.Instance.BackupROIs();

                });

            }
            else
            {
                //clear ROIs:
                OverlayManagerClass.Instance.ClearNonSaveROIs(ref CaptureSetupViewModel.OverlayCanvas);
            }

            _waveParamsUpdated = true;
        }

        private bool ValidateWaveformBeforeBuild()
        {
            bool ret = true;
            //check waveform:
            int stepCount = GetDwellCount((int)(_vm.BleachLSMPixelXY[0] * WaveformBuilder.MS_TO_S / SLMParamsCurrent.PixelSpacing));  //[Hz]
            if (0 == stepCount)
            {
                SLMPatternStatus = String.Format("Invalid Galvo calibration.\n");
                return false;
            }
            //Verify duration:
            SLMParamsCurrent.BleachWaveParams.DwellTime = SLMParamsCurrent.Duration * WaveformBuilder.MS_TO_S / stepCount;  //Duration [ms], DwellTime [us]
            if (WaveformBuilder.MinDwellTime > SLMParamsCurrent.BleachWaveParams.DwellTime)
            {
                SLMPatternStatus = String.Format("Minimum duration is {0} ms,\nwhile pixel spacing = {1}, width = {2} um, height = {3} um.\n",
                Decimal.Round((Decimal)(WaveformBuilder.MinDwellTime * stepCount / WaveformBuilder.MS_TO_S), 3), SLMParamsCurrent.PixelSpacing,
                SLMParamsCurrent.BleachWaveParams.ROIWidthUM, SLMParamsCurrent.BleachWaveParams.ROIHeightUM);

                SLMPatternStatus += String.Format("SLM Generation: FAILED due to invalid dwell time.\n");
                ret = false;
            }
            else if (WaveformBuilder.GetWaveform().Count > Int32.MaxValue)
            {
                SLMPatternStatus = String.Format("SLM Generation: FAILED due to limited memory space.\n");
                ret = false;
            }
            return ret;
        }

        #endregion Methods
    }
}