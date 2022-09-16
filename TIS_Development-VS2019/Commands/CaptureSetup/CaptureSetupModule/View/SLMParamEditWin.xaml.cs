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
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Shapes;
    using System.Xml;

    using CaptureSetupDll.ViewModel;

    using GeometryUtilities;

    using OverlayManager;

    using ThorSharedTypes;

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

        public SLMBrowsePanel(string browse, string ok, string cancel, string exportChecked, string includeSequences, string path, string exportFilename)
        {
            Browse = browse; Import = ok; Cancel = cancel; ExportChecked = exportChecked; SLMImportSequences = includeSequences;
            SLMImportPath = path; SLMExportFileName = exportFilename;
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

        public string ExportChecked
        {
            get;
            set;
        }

        public string Import
        {
            get;
            set;
        }

        public string SLMExportFileName
        {
            get;
            set;
        }

        public string SLMImportPath
        {
            get;
            set;
        }

        public string SLMImportSequences
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
        {"SLM_CONFIG_SEQUENCE",SLMPatternType.ConfigSequence},
        {"SLM_PATTERN_DUMPLICATE",SLMPatternType.Duplicate},
        {"SLM_Z_REF",SLMPatternType.ZRef},
        {"SLM_SAVEZOFFSET",SLMPatternType.SaveZOffset}
        };

        private BackgroundWorker slmBuildAllWorker;
        private BackgroundWorker slmBuilder;
        private BackgroundWorker slmCalibrator;
        private BackgroundWorker slmImportWorker;
        private BackgroundWorker slmPreviewWorker;
        private BackgroundWorker slmWorker;
        private int _calibratePointCount = 9; //exclude center [subID == 0]
        private SLMPanelMode _panelMode = SLMPanelMode.ParamEdit;
        private bool _slmCalibIsReset = false;
        private SLMParams _slmParamsCurrent;
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
            ConfigSequence,
            Duplicate,
            ZRef,
            SaveZOffset
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

        public int CalibratePointCount
        {
            get { return _calibratePointCount; }
            set { if (0 < value) _calibratePointCount = value; }
        }

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
                        stpPreview.Visibility = (_vm.IsStimulator) ? System.Windows.Visibility.Visible : System.Windows.Visibility.Collapsed;
                        SLMPreviewPanel slmPreview = new SLMPreviewPanel("PREVIEW", "CANCEL");
                        stpPreview.DataContext = slmPreview;
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
                        this.Title = "SLM Import or Export";
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

        public double PatternMinMS
        {
            get
            {
                string strTmp = string.Empty;
                const double DEFAULT_PATTERN_MS = 8.0;  //[msec], default minimum time for SLM runtime calculation
                double dVal = DEFAULT_PATTERN_MS;
                try
                {
                    XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                    XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/BleachView");
                    if (null != ndList)
                    {
                        if (!(XmlManager.GetAttribute(ndList[0], appSettings, "PatternMinMS", ref strTmp) && Double.TryParse(strTmp, out dVal)))
                        {
                            dVal = DEFAULT_PATTERN_MS;
                            XmlManager.SetAttribute(ndList[0], appSettings, "PatternMinMS", dVal.ToString());
                            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
                        }
                    }
                }
                catch (Exception ex)
                {
                    ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Verbose, 1, "Fail to get PatternMinMS: " + ex.Message);
                }
                return dVal;
            }
        }

        public List<SLMEpochSequence> SLMEpochSequences
        {
            get;
            set;
        }

        public string SLMExportFileName
        {
            get
            {
                return (string)this.Dispatcher.Invoke((SLMWorkerGetStatus)delegate
                {
                    return tbExportName.Text.ToString();
                });
            }
            set
            {
                this.Dispatcher.Invoke((SLMWorkerSetStatus)delegate
                {
                    tbExportName.Text = value;
                });
            }
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
                    _slmParamsCurrent.PowerParamsChangedEvent -= SLMParamsCurrent_PowerParamsChangedEvent;
                }
                _slmParamsCurrent = value;
                _slmParamsCurrent.BleachParamsChangedEvent += SLMParamsCurrent_BleachParamsChangedEvent;
                _slmParamsCurrent.PowerParamsChangedEvent += SLMParamsCurrent_PowerParamsChangedEvent;
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
                    stpSLMLabel.IsEnabled = (!value);
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
                if (!_vm.IsStimulator)
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
                }
                if (_vm.SLMSequenceOn && 0 == SLMEpochSequences.Count)
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

        /// <summary>
        /// rebuild all slm patterns and then waveform(s)
        /// </summary>
        public void ReBuildAll()
        {
            if (null == slmBuildAllWorker)
                return;

            if (slmBuildAllWorker.IsBusy)
            {
                CancelSLMBackgroundWorker(slmBuildAllWorker);
            }
            else
            {
                SLMSpinProgressVisible = true;
                SLMPatternStatus = String.Format("Start building all existing patterns ... \n");

                slmBuildAllWorker.DoWork += new DoWorkEventHandler(slmBuildAllWorker_DoWork);
                slmBuildAllWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(slmBuildAllWorker_RunWorkerCompleted);
                slmBuildAllWorker.RunWorkerAsync();
            }
        }

        public void UpdateSLMParamEdit()
        {
            if (null != _slmParamsCurrent)
            {
                if (_vm.IsStimulator)
                {
                    //alert user to manually update (measured) power
                    _slmParamsCurrent.SLMPowerAlert = true;
                }
                else
                {
                    //calculate area through image
                    CalculateSLMPowerDensity((0 == _slmParamsCurrent.BleachWaveParams.ROIWidth && 0 == _slmParamsCurrent.BleachWaveParams.ROIHeight) ?
                        (Math.Pow((_slmParamsCurrent.BleachWaveParams.UMPerPixel / _slmParamsCurrent.BleachWaveParams.UMPerPixelRatio), 2)) :
                        Math.PI * Math.Max(1, _slmParamsCurrent.BleachWaveParams.ROIWidth) * Math.Max(1, _slmParamsCurrent.BleachWaveParams.ROIHeight) / 4 * Math.Pow((_slmParamsCurrent.BleachWaveParams.UMPerPixel / _slmParamsCurrent.BleachWaveParams.UMPerPixelRatio), 2));
                }
            }
        }

        /// <summary>
        /// Calculate SLM power based on patterns' area power density
        /// </summary>
        public void UpdateSLMParamPower()
        {
            List<Shape> rois = new List<Shape>();
            this.Dispatcher.Invoke((SLMSavePattern)delegate
            { rois = OverlayManagerClass.Instance.GetModeROIs(Mode.PATTERN_WIDEFIELD, OverlayManagerClass.Instance.PatternID, _vm.SLMWavelengthNM); });
            if (0 < rois.Count)
            {
                System.Drawing.Bitmap bmp = ProcessBitmap.CreateBinaryBitmap(new int[2] { _vm.ImageWidth, _vm.ImageHeight }, rois);
                CalculateSLMPowerDensity(ProcessBitmap.BinaryBitmapNonZeroCount(bmp) * Math.Pow((double)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (object)1.0], 2));
            }
            else if (!_slmParamsCurrent.PowerEntryPreferred)
                CalculateSLMPowerDensity(0.0);

            if (null != _slmParamsCurrent)
                _slmParamsCurrent.SLMPowerAlert = false;
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
            CalibrationProc((null != (sender as FrameworkElement)) ? (sender as FrameworkElement).Tag as string : (string)(sender));
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
            string ext = ".xml";
            string note = (null != (sender as FrameworkElement)) ? (sender as FrameworkElement).Tag as string : (string)(sender);
            switch (note)
            {
                case "BROWSE":
                    if (true == tbtnImExport.IsChecked)  //export: checked
                    {
                        System.Windows.Forms.FolderBrowserDialog fbd = new System.Windows.Forms.FolderBrowserDialog();
                        fbd.SelectedPath = System.IO.Path.GetDirectoryName(_vm.SLMImportFilePathName);

                        if (string.Empty != fbd.ShowDialog().ToString())
                        {
                            SLMImportPath = fbd.SelectedPath;
                            SLMPatternStatus = string.Empty;
                        }
                    }
                    else                                 //import
                    {
                        Microsoft.Win32.OpenFileDialog ofd = new Microsoft.Win32.OpenFileDialog();

                        ofd.FileName = "*.xml";
                        ofd.InitialDirectory = System.IO.Path.GetDirectoryName(_vm.SLMImportFilePathName);
                        ofd.DefaultExt = ext;
                        ofd.Filter = "XML Files (*.xml)|*.xml";

                        if (true == ofd.ShowDialog())
                        {
                            SLMImportPath = ofd.FileName;
                            SLMPatternStatus = string.Empty;
                        }
                    }
                    break;
                case "IMPORT":      //both SLM export or import
                    SLMGenResult = false;

                    if (true == tbtnImExport.IsChecked)     //export: checked
                    {
                        if ((File.GetAttributes(SLMImportPath) & FileAttributes.Directory) == FileAttributes.Directory)
                        {
                            if (!Directory.Exists(SLMImportPath))
                            {
                                SLMPatternStatus = String.Format("Selected directory does not exist.");
                                return;
                            }
                        }
                        else
                        {
                            if (!Directory.Exists(System.IO.Path.GetDirectoryName(SLMImportPath)))
                            {
                                SLMPatternStatus = String.Format("Selected directory does not exist.");
                                return;
                            }
                            SLMImportPath = System.IO.Path.GetDirectoryName(SLMImportPath);
                        }
                    }
                    else                                    //import: un-checked
                    {
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

                        if (!_vm.IsStimulator && null == _vm.BleachLSMFieldScaleXYFine)
                        {
                            SLMPatternStatus = String.Format("Calibration must be done before import or export.\n");
                            return;
                        }
                    }

                    if (slmImportWorker.IsBusy)
                    {
                        CancelSLMBackgroundWorker(slmImportWorker);
                    }
                    else
                    {
                        SLMSpinProgressVisible = true;
                        slmImportWorker.DoWork += new DoWorkEventHandler(ImportWorker_DoWork);
                        slmImportWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(ImportWorker_RunWorkerCompleted);

                        List<object> arguments = new List<object>();
                        arguments.Add(tbtnImExport.IsChecked);
                        arguments.Add(chbImportSeq.IsChecked);
                        arguments.Add((0 < SLMExportFileName.Length ? SLMExportFileName : _vm.SLMExportFileName) + ".xml");
                        slmImportWorker.RunWorkerAsync(arguments);
                    }
                    break;
                case "CANCEL":
                    this.Close();
                    break;
            }
        }

        private void btnSLMPreview_Click(object sender, RoutedEventArgs e)
        {
            string note = (null != (sender as FrameworkElement)) ? (sender as FrameworkElement).Tag as string : (string)(sender);
            switch (note)
            {
                case "PREVIEW":
                    switch ((SLMPanelMode)this.PanelMode)
                    {
                        case SLMPanelMode.ParamEdit:
                            if (slmPreviewWorker.IsBusy)
                            {
                                CancelSLMBackgroundWorker(slmPreviewWorker);
                            }
                            else
                            {
                                SLMSpinProgressVisible = true;
                                slmPreviewWorker.DoWork += new DoWorkEventHandler(PreviewWorker_DoWork);
                                slmPreviewWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(PreviewWorker_RunWorkerCompleted);
                                slmPreviewWorker.RunWorkerAsync(argument: note);
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case "CANCEL":
                    _vm.IdleSLM();
                    break;
                default:
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

            //check calibration has been done or valid, for Galvo only:
            SLMParams sWaveParams = new SLMParams();
            double[] powerVal;
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
                sWaveParams.BleachWaveParams.ClockRate = WaveformBuilder.ClkRate = (int)(_vm.BleachLSMPixelXY[0] * WaveformBuilder.MS_TO_S);  //[Hz], keep PixelSpacing as 1
                sWaveParams.BleachWaveParams.Power = _vm.SLMCalibPower;                       //[%]
                powerVal = new double[1] { WaveformBuilder.GetPockelsPowerValue(sWaveParams.BleachWaveParams.Power, _vm.BleachCalibratePockelsVoltageMin0[0], _vm.BleachCalibratePockelsVoltageMax0[0], (PockelsResponseType)MVMManager.Instance["PowerControlViewModel", "BleacherPowerResponse0", 0]) };
            }
            else
            {
                sWaveParams.BleachWaveParams.ClockRate = WaveformBuilder.ClkRate = _vm.BleachInternalClockRate;  //[Hz], keep PixelSpacing as 1
                sWaveParams.BleachWaveParams.Power = _vm.SLMSelectWavelength ? 0.0 : _vm.SLMCalibPower;      //[%]
                sWaveParams.BleachWaveParams.Power1 = _vm.SLMSelectWavelength ? _vm.SLMCalibPower : 0.0;     //[%]
                powerVal = (1 < _vm.BleachCalibratePockelsVoltageMin0.Length) ?
                    new double[2] { WaveformBuilder.GetPockelsPowerValue(sWaveParams.BleachWaveParams.Power, _vm.BleachCalibratePockelsVoltageMin0[0], _vm.BleachCalibratePockelsVoltageMax0[0], (PockelsResponseType)MVMManager.Instance["PowerControlViewModel", "BleacherPowerResponse0", 1]),
                    WaveformBuilder.GetPockelsPowerValue(sWaveParams.BleachWaveParams.Power1, _vm.BleachCalibratePockelsVoltageMin0[1], _vm.BleachCalibratePockelsVoltageMax0[1], (PockelsResponseType)MVMManager.Instance["PowerControlViewModel", "BleacherPowerResponse1", 1]) } :
                    new double[1] { WaveformBuilder.GetPockelsPowerValue(sWaveParams.BleachWaveParams.Power, _vm.BleachCalibratePockelsVoltageMin0[0], _vm.BleachCalibratePockelsVoltageMax0[0], (PockelsResponseType)MVMManager.Instance["PowerControlViewModel", "BleacherPowerResponse0", 1]) };
            }

            sWaveParams.BleachWaveParams.PrePatIdleTime = sWaveParams.BleachWaveParams.PreIdleTime = 0;         //[ms]
            sWaveParams.BleachWaveParams.DwellTime = (WaveformBuilder.MinDwellTime < _vm.SLMCalibDwell) ?       //[us]
                (_vm.SLMCalibDwell * (int)WaveformBuilder.MS_TO_S) : (WaveformBuilder.MinDwellTime * (int)WaveformBuilder.MS_TO_S);
            sWaveParams.BleachWaveParams.PostPatIdleTime = sWaveParams.BleachWaveParams.PostIdleTime = 0;       //[ms]
            sWaveParams.BleachWaveParams.shapeType = "Crosshair";
            sWaveParams.BleachWaveParams.UMPerPixel = (double)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (object)1.0];
            sWaveParams.BleachWaveParams.UMPerPixelRatio = _vm.BleachPixelSizeUMRatio;
            sWaveParams.BleachWaveParams.Center = new Point(Math.Floor((double)_vm.BleachLSMPixelXY[0] / 2), Math.Floor((double)(_vm.BleachLSMPixelXY[1] / 2)));
            sWaveParams.BleachWaveParams.Iterations = 1;
            sWaveParams.BleachWaveParams.ROIHeight = sWaveParams.BleachWaveParams.ROIWidth = 1;

            if (_vm.SLMCalibWaveParam.CompareTo(sWaveParams.BleachWaveParams))
            {
                //no need to re-build waveform:
                return "0";
            }

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
            WaveformBuilder.SaveWaveform(calibWaveName, true, new bool[(int)SignalType.SIGNALTYPE_LAST] { !_vm.IsStimulator, true, true, false });

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

        private void CalculateSLMPowerDensity(double areaUM2)
        {
            if (null != _slmParamsCurrent)
            {
                // PowerDensity[mW/mm^2] = MeasuredPower[mW] * Power[%] / 100 * [um-to-mm]^2 / Area[um^2]
                if (_slmParamsCurrent.PowerEntryPreferred)
                {
                    //calculate power density based on power entry
                    double powerMW = (double)_slmParamsCurrent.BleachWaveParams.GetType().GetProperty(_vm.SLMSelectWavelengthProp ? "MeasurePower" + 1 : "MeasurePower").GetValue(_slmParamsCurrent.BleachWaveParams) *
                        (double)_slmParamsCurrent.BleachWaveParams.GetType().GetProperty(_vm.SLMSelectWavelengthProp ? "Power" + 1 : "Power").GetValue(_slmParamsCurrent.BleachWaveParams) / (double)Constants.HUNDRED_PERCENT;

                    _slmParamsCurrent.GetType().GetProperty(_vm.SLMSelectWavelengthProp ? "SLMMeasurePowerArea" + 1 : "SLMMeasurePowerArea").SetValue(_slmParamsCurrent,
                        (0 >= areaUM2) ? 0.0 : Math.Round(powerMW * Math.Pow((double)Constants.UM_TO_MM, 2) / areaUM2, 6)); // [mW/mm^2]
                }
                else
                {
                    //calculate power based on power density entry
                    double measuredPower = (double)_slmParamsCurrent.BleachWaveParams.GetType().GetProperty(_vm.SLMSelectWavelengthProp ? "MeasurePower" + 1 : "MeasurePower").GetValue(_slmParamsCurrent.BleachWaveParams);

                    double powerPercentValue = (0 >= measuredPower) ? 0.0 :
                        Math.Round(areaUM2 * (double)_slmParamsCurrent.GetType().GetProperty(_vm.SLMSelectWavelengthProp ? "SLMMeasurePowerArea" + 1 : "SLMMeasurePowerArea").GetValue(_slmParamsCurrent) * (double)Constants.HUNDRED_PERCENT / measuredPower / Math.Pow((double)Constants.UM_TO_MM, 2), 3); // [%]
                    if ((double)Constants.HUNDRED_PERCENT < powerPercentValue)
                    {
                        powerPercentValue = (double)Constants.HUNDRED_PERCENT;
                        _slmParamsCurrent.GetType().GetProperty(_vm.SLMSelectWavelengthProp ? "SLMMeasurePowerArea" + 1 : "SLMMeasurePowerArea").SetValue(_slmParamsCurrent,
                            (0 >= areaUM2) ? 0.0 :
                            Math.Round(measuredPower * powerPercentValue * Math.Pow((double)Constants.UM_TO_MM, 2) / areaUM2 / (double)Constants.HUNDRED_PERCENT, 6)); // [mW/mm^2]
                    }

                    _slmParamsCurrent.BleachWaveParams.GetType().GetProperty(_vm.SLMSelectWavelengthProp ? "Power" + 1 : "Power").SetValue(_slmParamsCurrent.BleachWaveParams, powerPercentValue); // [%]
                }
            }
        }

        private void CalibrationProc(string note)
        {
            SLMCalibPanel slmCalib;
            string roiPathAndName = _vm.BleachROIPath + _vm.SLMCalibFile;
            string strBody = string.Empty, roiType = string.Empty;
            Point offCenter = new Point(-1, -1);
            List<Point> pts = new List<Point>();
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
                    _slmCalibIsReset = _vm.ResetSLMCalibration();
                    OverlayManagerClass.Instance.DisplayPatternROI(ref CaptureSetupViewModel.OverlayCanvas, OverlayManagerClass.Instance.PatternID - 1, true);

                    slmCalib = _vm.SLM3D ?
                        new SLMCalibPanel("SELECT", "DONE", "New Calibration", "Keep Z at reference and use Cross Hair\nROI tool to mark the single imaging bead\nthen Press Yes to continue calibration, \nPress No to cancel.") :
                        new SLMCalibPanel("BURN", "SELECT", "New Calibration", "Move to another clear area on the\ncalibration slide then Press Yes to\ncreate calibration spots on the slide,\nPress No to continue.");
                    this.DataContext = slmCalib;
                    this.Show();
                    break;
                //[For 3D calibration method]
                case "RESET_CALIBRATION3D":
                    _vm.ResetSLMCalibration();
                    //skip affine fittings at loading calibration pattern,
                    //since 3D fittings will be applied at 3D holo gen.
                    _vm.SLMSkipFitting = true;

                    slmCalib = new SLMCalibPanel("SPOT_SELECT", "DONE", "New Calibration", "Keep Z at reference and use Cross Hair\nROI tool to mark the single imaging bead\nthen Press Yes to continue calibration,\nPress No to cancel.");
                    this.DataContext = slmCalib;
                    this.Show();
                    break;
                //[For 3D calibration method]
                case "SPOT_SELECT":
                    //expect center spot selected only:
                    pts = OverlayManagerClass.Instance.GetPatternROICenters(OverlayManagerClass.Instance.PatternID, ref roiType, ref offCenter);
                    if ((-1 != offCenter.X || -1 != offCenter.Y) && 0 == pts.Count)
                    {
                        //create mask:
                        List<Shape> rois = OverlayManagerClass.Instance.GetModeROIs(Mode.PATTERN_NOSTATS, OverlayManagerClass.Instance.PatternID - 1);
                        System.Drawing.Bitmap bmp = ProcessBitmap.CreateBinaryBitmap(new int[2] { _vm.ImageWidth, _vm.ImageHeight }, rois);
                        string bmpPath = _vm.BleachROIPath + "SLMCalibROIs.bmp";
                        bmp.Save(bmpPath, System.Drawing.Imaging.ImageFormat.Bmp);

                        //have mask loaded with holo gen:
                        _vm.LoadSLMPatternName(0, 0, bmpPath, true, false);

                        strBody = "Using Cross Hair ROI tool, mark the \ncenter points of the burned calibration \nspots on the image. Move to a differnt\nZ value and repeat. Press YES when\ncomplete, and press No to break.\n\n\n\tx4\tx5\tx6\n\n\tx3\n\t\t\t\tx7\n\n\tx2\n\t\t     x8\n\n\n\tx1\t\t\tx9\n ";
                        slmCalib = new SLMCalibPanel("TRY_CALIBRATE", "DONE", "New Calibration", strBody);
                    }
                    else
                    {
                        _vm.SLMSetBlank();
                        OverlayManagerClass.Instance.DeletePatternROI(ref CaptureSetupViewModel.OverlayCanvas, OverlayManagerClass.Instance.PatternID);
                        OverlayManagerClass.Instance.ClearNonSaveROIs(ref CaptureSetupViewModel.OverlayCanvas);
                        OverlayManagerClass.Instance.DisplayPatternROI(ref CaptureSetupViewModel.OverlayCanvas, OverlayManagerClass.Instance.PatternID - 1, false);
                        strBody = "Error: Only one spot should be selected.\n\nKeep Z at reference and use Cross Hair\nROI tool to mark the single imaging bead\nthen Press Yes to continue calibration,\nPress No to cancel.";
                        slmCalib = new SLMCalibPanel("SPOT_SELECT", "DONE", "New Calibration", strBody);
                    }
                    this.DataContext = slmCalib;
                    this.Show();
                    break;
                //[For 3D calibration method]
                case "TRY_CALIBRATE":
                    if (slmCalibrator.IsBusy)
                    { return; }

                    slmCalibrator.DoWork += new DoWorkEventHandler(SLMCalibrator3D_DoWork);
                    slmCalibrator.RunWorkerCompleted += new RunWorkerCompletedEventHandler(SLMCalibrator3D_RunWorkerCompleted);
                    slmCalibrator.RunWorkerAsync();
                    break;
                case "SELECT":
                    OverlayManagerClass.Instance.DeletePatternROI(ref CaptureSetupViewModel.OverlayCanvas, OverlayManagerClass.Instance.PatternID);
                    OverlayManagerClass.Instance.ClearNonSaveROIs(ref CaptureSetupViewModel.OverlayCanvas);
                    OverlayManagerClass.Instance.DisplayPatternROI(ref CaptureSetupViewModel.OverlayCanvas, OverlayManagerClass.Instance.PatternID - 1, false);

                    strBody = "Using Cross Hair ROI tool, mark the \ncenter points of the burned calibration \nspots on the image. Press YES when\ncomplete, and press No to break.\n\n\n\tx4\tx5\tx6\n\n\tx3\n\t\t\t\tx7\n\n\tx2\n\n\t\t\t     x8\n\n\tx1\t\t\tx9\n ";
                    slmCalib = new SLMCalibPanel("FINISH_SELECT", "DONE", "New Calibration", strBody);
                    this.DataContext = slmCalib;
                    this.Show();
                    break;
                case "FINISH_SELECT":
                    //retrieve user selected points (without offset):
                    pts = OverlayManagerClass.Instance.GetPatternROICenters(OverlayManagerClass.Instance.PatternID, ref roiType, ref offCenter);
                    if (CalibratePointCount != pts.Count)
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
                case "REBUILD":
                    ReBuildAll();
                    break;
                case "CHECK":
                    if (_slmCalibIsReset && _vm.SLMPhaseDirect && 0 < _vm.SLMBleachWaveParams.Count)
                    {
                        OverlayManagerClass.Instance.RevokeROIs(ref CaptureSetupViewModel.OverlayCanvas);
                        _vm.DisplayROI();
                        OverlayManagerClass.Instance.CurrentMode = _vm.IsStimulator ? ThorSharedTypes.Mode.PATTERN_WIDEFIELD : ThorSharedTypes.Mode.PATTERN_NOSTATS;
                        slmCalib = new SLMCalibPanel("REBUILD", "DONE", "After Calibration", "The calibration may be altered.\nPress Yes if you want the patterns to be rebuilt.");
                        this.DataContext = slmCalib;
                        this.Show();
                    }
                    else
                    {
                        CalibrationProc("DONE");
                    }
                    break;
                case "DONE":
                    CancelCalibrator();

                    //reset sub ID to regular one-based (zero-based for 3D calibration):
                    OverlayManagerClass.Instance.SkipRedrawCenter = false;

                    //no skip fitting other than 3D calibration:
                    _vm.SLMSkipFitting = false;

                    this.Close();
                    break;
                default:
                    break;
            }
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

        private void CancelSLMBackgroundWorker(BackgroundWorker worker, bool updateFile = false)
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

        /// <summary>
        /// copy ROIs from one wavelength into other wavelenth
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void CopyWaveLengthBtn_Click(object sender, RoutedEventArgs e)
        {
            //retrieve rois for current wavelength:
            List<Shape> rois = OverlayManagerClass.Instance.GetModeROIs(Mode.PATTERN_WIDEFIELD, OverlayManagerClass.Instance.PatternID, _vm.SLMWavelengthNM);

            //back up current wavelength selection:
            bool backup = _vm.SLMSelectWavelengthProp;

            //force SLM select the other wavelength:
            _vm.SLMSelectWavelengthProp = !backup;

            //append rois:
            OverlayManagerClass.Instance.AppendModeROIs(ref CaptureSetupViewModel.OverlayCanvas, Mode.PATTERN_WIDEFIELD, OverlayManagerClass.Instance.PatternID, rois, _vm.SLMWavelengthNM);

            _vm.SLMSelectWavelengthProp = backup;

            OverlayManagerClass.Instance.DimWavelengthROI(ref CaptureSetupViewModel.OverlayCanvas);
        }

        private void GenSLMPattern()
        {
            if ((null == _vm.Bitmap) || (null == _vm.BleachLSMFieldScaleXYFine && !_vm.IsStimulator))
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
            if (!_vm.IsStimulator)
            {
                //check calibration has been done or valid:
                if (null == _vm.BleachCalibrateFineScaleXY)
                { return 0; }
                else if ((0 == _vm.BleachCalibrateFineScaleXY[0]) || (0 == _vm.BleachCalibrateFineScaleXY[1]))
                { return 0; }

                if ((0 == _vm.BleachCalibrateFieldSize) || (null == _vm.BleachCalibratePixelXY))
                { return 0; }
            }
            double[] powerVal = new double[1] { WaveformBuilder.GetPockelsPowerValue(SLMParamsCurrent.BleachWaveParams.Power, _vm.BleachCalibratePockelsVoltageMin0[0], _vm.BleachCalibratePockelsVoltageMax0[0], (PockelsResponseType)MVMManager.Instance["PowerControlViewModel", "BleacherPowerResponse0", (object)0]) };

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

        private void ImportWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker worker = sender as BackgroundWorker;
            if (worker.CancellationPending != true)
            {
                List<object> arguments = (List<object>)e.Argument;
                if ((Boolean)arguments[0])      //IsChecked true: Export
                {
                    FileName oFile = new FileName((string)arguments[2]);
                    oFile.MakeUnique(SLMImportPath);
                    SLMGenResult = TryExportPatternSequences(SLMImportPath + "\\" + oFile.FullName, (Boolean)arguments[1]);
                }
                else                            //Import
                {
                    //load xml
                    XmlDocument xmlSLMImportDoc = new XmlDocument();
                    xmlSLMImportDoc.Load(SLMImportPath);

                    //load SLM params and determine the import type
                    if (!TryImportPatternSequences(xmlSLMImportDoc, (Boolean)arguments[1]))
                        SLMGenResult = TryImportWaveforms(xmlSLMImportDoc);
                    else
                        SLMGenResult = true;
                }
            }
            //pass export(true) or import(false) to completed event
            e.Result = (Boolean)((List<object>)e.Argument)[0];
        }

        private void ImportWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (e.Error != null)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "SLMImportor: " + e.Error.Message);
            }

            SLMSpinProgressVisible = false;
            slmImportWorker.DoWork -= new DoWorkEventHandler(ImportWorker_DoWork);
            slmImportWorker.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(ImportWorker_RunWorkerCompleted);

            if (SLMGenResult && !e.Cancelled)
            {
                //update viewmodel properties
                _vm.SLMImportFilePathName = SLMImportPath;
                _vm.SLMExportChecked = (true == tbtnImExport.IsChecked);
                _vm.SLMImportSequences = (true == chbImportSeq.IsChecked);

                if ((Boolean)e.Result)  //Export
                {
                    _vm.SLMExportFileName = SLMExportFileName;
                    this.Close();
                }
                else                    //Import
                {
                    this.DataContext = _vm;
                    RebuildSLMWaveform(true);
                }
            }
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

        private void PreviewWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker worker = sender as BackgroundWorker;
            if (worker.CancellationPending != true)
            {
                switch ((string)e.Argument)
                {
                    case "PREVIEW":
                    default:
                        //try save pattern bmp
                        if (SaveSLMPatternImage(_vm.SLMPreviewFileName))
                        {
                            //push pattern to SLM device
                            _vm.LoadSLMPatternName(0, 0, _vm.SLMPreviewFileName, true, _vm.IsStimulator);

                            //update power at bleach scanner
                            UpdateSLMParamPower();
                            MVMManager.Instance["PowerControlViewModel", "BleacherPower0"] = Math.Max(0.0, SLMParamsCurrent.BleachWaveParams.Power);
                            MVMManager.Instance["PowerControlViewModel", "BleacherPower1"] = Math.Max(0.0, SLMParamsCurrent.BleachWaveParams.Power1);
                        }
                        break;
                }
            }
        }

        private void PreviewWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            ResourceManagerCS.DeleteFile(_vm.SLMPreviewFileName);
            SLMSpinProgressVisible = false;
            slmPreviewWorker.DoWork -= new DoWorkEventHandler(PreviewWorker_DoWork);
            slmPreviewWorker.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(PreviewWorker_RunWorkerCompleted);
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
            ResourceManagerCS.SafeCreateDirectory(_vm.SLMWaveformFolder[0]);

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

            return SaveSLMPatternImage((null == WaveFileNameAndPathCurrent[2]) ? WaveFileNameAndPathCurrent[3] : _vm.SLMWaveformFolder[0] + "\\" + WaveFileNameAndPathCurrent[2] + ".bmp");
        }

        /// <summary>
        /// either save phase masks as an image or combined two wavelengths as one (left-right)
        /// </summary>
        private bool SaveSLMPatternImage(string bmpPath, bool phaseGen = true)
        {
            try
            {
                string strTmp = string.Empty;
                System.Drawing.Bitmap bmp;
                float dpi = (float)((double)Constants.UM_PER_INCH / (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)1.0] / (double)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (object)1.0]);

                //get pattern offset & shapes:
                Point offCenter = new Point(-1, -1);
                List<Point> patternPoints = new List<Point>(), patternPointsUnshifted = new List<Point>();
                this.Dispatcher.Invoke((SLMWorkerSetGUI)delegate
                {
                    patternPoints = OverlayManagerClass.Instance.GetPatternROICenters(OverlayManagerClass.Instance.PatternID, ref strTmp, ref offCenter);
                });
                patternPointsUnshifted = patternPoints;
                SLMParamsCurrent.BleachWaveParams.shapeType = strTmp;

                //clean up not-in-use attributes
                CaptureSetupViewModel.RemoveApplicationAttribute("/ApplicationSettings/DisplayOptions/CaptureSetup/BleachView", "AllowSavePattern");

                //here we save non-offset patterns in a sub-folder if enabled in settings,
                //for user to do roi analysis:
                int saveUnshifted = 0;
                XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/BleachView");
                if (XmlManager.GetAttribute(ndList[0], appSettings, "SLMPatternUnshifted", ref strTmp) && Int32.TryParse(strTmp, out saveUnshifted))
                {
                    if (1 == saveUnshifted)
                    {
                        ResourceManagerCS.SafeCreateDirectory(System.IO.Path.GetDirectoryName(bmpPath) + _vm.SLMbmpSubFolders[1]);
                    }
                    else
                    {
                        ResourceManagerCS.DeleteDirectory(bmpPath.Substring(0, bmpPath.LastIndexOf("\\")) + _vm.SLMbmpSubFolders[1]);
                    }
                }

                //create the subfolder if not exist
                string workingDir = System.IO.Path.GetDirectoryName(bmpPath) + _vm.SLMbmpSubFolders[(_vm.SLM3D ? 2 : 1)];
                ResourceManagerCS.SafeCreateDirectory(workingDir);
                string zbmpPath = bmpPath;

                if (_vm.IsStimulator)
                {
                    //save phase masks to expedite loading time, especially combining two sets (of light source) as one,
                    //fetch wavelength by altering selected wavelength (give back user selection afterward),
                    //expected to be regenerated if new calibration.
                    bool backupWavelength = _vm.SLMSelectWavelengthProp;
                    string[] bmpPathLamda = new string[_vm.SLMWavelengthCount], bmpPathLamdaUnshifted = new string[_vm.SLMWavelengthCount];
                    List<Shape>[] rois = new List<Shape>[_vm.SLMWavelengthCount];
                    this.Dispatcher.Invoke((SLMSavePattern)delegate
                    {
                        //update SLM panel display
                        SLMParamsCurrent.BleachWaveParams.Center = new Point(0.0, 0.0);
                        SLMParamsCurrent.BleachWaveParams.ROIWidth = SLMParamsCurrent.BleachWaveParams.ROIHeight = 0.0;

                        for (int i = 0; i < _vm.SLMWavelengthCount; i++)
                        {
                            _vm.SLMSelectWavelengthProp = (1 == i);     //to retrieve rois of selected wavelength
                            bmpPathLamdaUnshifted[i] = workingDir + "\\" + (String.IsNullOrEmpty(System.IO.Path.GetFileNameWithoutExtension(bmpPath)) ? "" : System.IO.Path.GetFileNameWithoutExtension(bmpPath) + "_") + _vm.SLMWavelengthNM + "nm.bmp";
                            bmpPathLamda[i] = workingDir + "\\" + (String.IsNullOrEmpty(System.IO.Path.GetFileNameWithoutExtension(bmpPath)) ? "" : System.IO.Path.GetFileNameWithoutExtension(bmpPath) + "_preview") + _vm.SLMWavelengthNM + "nm.bmp";
                            rois[i] = OverlayManagerClass.Instance.GetModeROIs(Mode.PATTERN_WIDEFIELD, OverlayManagerClass.Instance.PatternID, _vm.SLMWavelengthNM);
                        }
                    });
                    //separate create bmp to allow spinner wheel
                    for (int i = 0; i < _vm.SLMWavelengthCount; i++)
                    {
                        _vm.SLMSelectWavelengthProp = (1 == i);         //to apply coefficients of selected wavelength
                        bmp = ProcessBitmap.CreateBinaryBitmap(new int[2] { _vm.ImageWidth, _vm.ImageHeight }, rois[i]);
                        bmp.SetResolution(dpi, dpi);                    //dots per inch
                        if (1 == saveUnshifted) bmp.Save(bmpPathLamdaUnshifted[i], System.Drawing.Imaging.ImageFormat.Bmp);
                        bmp.Save(bmpPathLamda[i], System.Drawing.Imaging.ImageFormat.Bmp);
                        if (phaseGen) _vm.SaveSLMPatternName(bmpPathLamda[i]);        //save phase masks with calibration
                    }
                    //combine phase masks and rename afterward
                    if (phaseGen)
                    {
                        if (1 < _vm.SLMWavelengthCount)
                        {
                            _vm.CombineHolograms(bmpPathLamda[0], bmpPathLamda[1]);
                            ResourceManagerCS.DeleteFile(bmpPathLamda[1]);
                        }
                        if (File.Exists(bmpPathLamda[0]))
                        {
                            ResourceManagerCS.DeleteFile(bmpPath);
                            File.Move(bmpPathLamda[0], bmpPath);
                        }
                    }
                    _vm.SLMSelectWavelengthProp = backupWavelength;
                }
                else
                {
                    Point centerOffset = _vm.GetSLMPatternBoundROICenter(ref patternPoints, offCenter);
                    SLMParamsCurrent.BleachWaveParams.Center = new Point(Math.Round((_vm.BleachLSMPixelXY[0] / 2) - centerOffset.X, 3), Math.Round((_vm.BleachLSMPixelXY[1] / 2) - centerOffset.Y, 3));

                    if (_vm.SLM3D)
                    {
                        Dictionary<double, List<Shape>> zShapesNoCenter = new Dictionary<double, List<Shape>>();
                        this.Dispatcher.Invoke((SLMSavePattern)delegate
                        {
                            zShapesNoCenter = OverlayManagerClass.Instance.GetPatternZROIs(-1, OverlayManagerClass.Instance.PatternID);
                        });
                        foreach (KeyValuePair<double, List<Shape>> entry in zShapesNoCenter)
                        {
                            zbmpPath = workingDir + "\\" + (String.IsNullOrEmpty(System.IO.Path.GetFileNameWithoutExtension(bmpPath)) ? "" : System.IO.Path.GetFileNameWithoutExtension(bmpPath) + "_Z[") + entry.Key.ToString("F3") + "]um.bmp";
                            bmp = ProcessBitmap.CreateBinaryBitmap(_vm.BleachLSMPixelXY, entry.Value, ProcessBitmap.SaveShapeType.Center_Only, centerOffset);
                            bmp.SetResolution(dpi, dpi);                    //dots per inch
                            bmp.Save(zbmpPath, System.Drawing.Imaging.ImageFormat.Bmp);
                        }
                        //request SLM to generate 3D mask using the file name
                        _vm.SaveSLMPatternName(zbmpPath);
                    }
                    else
                    {
                        //save bitmap of pattern centers:
                        bmp = ProcessBitmap.CreateBinaryBitmap(_vm.BleachLSMPixelXY[0], _vm.BleachLSMPixelXY[1], patternPoints);                        //after offset

                        //We save selected point image instead of phase mask,
                        //so that user can run history experiments with new calibrations:
                        bmp.SetResolution(dpi, dpi);                        //dots per inch
                        bmp.Save(bmpPath, System.Drawing.Imaging.ImageFormat.Bmp);
                        _vm.SaveSLMPatternName(bmpPath);

                        //save unshifted version:
                        if (1 == saveUnshifted)
                        {
                            bmp = ProcessBitmap.CreateBinaryBitmap(_vm.BleachLSMPixelXY[0], _vm.BleachLSMPixelXY[1], patternPointsUnshifted);           //before offset
                            bmp.SetResolution(dpi, dpi);                    //dots per inch
                            bmp.Save(bmpPath.Substring(0, bmpPath.LastIndexOf("\\")) + _vm.SLMbmpSubFolders[1] + bmpPath.Substring(bmpPath.LastIndexOf("\\")), System.Drawing.Imaging.ImageFormat.Bmp);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "SaveSLMPatternImage: " + ex.Message);
                return false;
            }
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

        private void slmBuildAllWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker worker = sender as BackgroundWorker;
            if (worker.CancellationPending != true)
            {
                for (SLMParamID = 0; SLMParamID < _vm.SLMBleachWaveParams.Count; SLMParamID++)              //index in waveform param list [0-based]
                {
                    SLMParamsCurrent = new SLMParams(_vm.SLMBleachWaveParams[SLMParamID]);
                    OverlayManagerClass.Instance.PatternID = (int)SLMParamsCurrent.BleachWaveParams.ID;     //pattern ID: 1 based
                    SaveSLMFiles();
                    SLMGenResult = ValidateWaveformBeforeBuild();
                    SLMPatternStatus = SLMGenResult ? String.Format("Saving SLM Pattern: " + SLMParamsCurrent.Name) : String.Format("Unable to save SLM Pattern: " + SLMParamsCurrent.Name);
                    if (!SLMGenResult)
                        break;
                    SLMFileSaved = true;
                    _waveParamsUpdated = false;
                    UpdateWaveParamsAndFile();
                }
            }
        }

        private void slmBuildAllWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            SLMSpinProgressVisible = false;
            if (e.Error != null)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "SLMBuildAll: " + e.Error.Message);
            }
            else if (e.Cancelled == true)
            {
                SLMPatternStatus = String.Format("Cancelled SLM build all.\n ");
                UpdateWaveParamsAndFile();
                SLMGenResult = false;
            }

            slmBuildAllWorker.DoWork -= new DoWorkEventHandler(slmBuildAllWorker_DoWork);
            slmBuildAllWorker.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(slmBuildAllWorker_RunWorkerCompleted);

            if (SLMGenResult)
            {
                if (_vm.SLMSequenceOn)
                    BuildSequences();
                else
                    RebuildSLMWaveform();
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
                    _vm.SLMBleachWaveParams[i].BleachWaveParams.DwellTime = _vm.SLMBleachWaveParams[i].Duration * WaveformBuilder.MS_TO_S / (_vm.IsStimulator ? 1 : stepCount);  //Duration [ms], DwellTime [us]
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

                        ResourceManagerCS.SafeCreateDirectory(_vm.SLMWaveformFolder[1]);

                        // build bleach params & save sequence txt file
                        ResourceManagerCS.DeleteFile(seqTextName);
                        slmBleachWaveParams = new ObservableCollection<SLMParams>();
                        StreamWriter seqTextFile = new StreamWriter(seqTextName);

                        SLMPatternStatus = String.Format("Saving SLM Sequence # " + (seq + 1).ToString() + " ...\n");

                        for (int i = 0; i < patternIDs.Length; i++)
                        {
                            //verify pattern time to be long enough for SLM runtime calculation
                            if (PatternMinMS > (_vm.IsStimulator ? _vm.SLMBleachWaveParams[patternIDs[i] - 1].Duration : _vm.SLMBleachWaveParams[patternIDs[i] - 1].BleachWaveParams.EstDuration) * _vm.SLMBleachWaveParams[patternIDs[i] - 1].BleachWaveParams.Iterations)
                            {
                                SLMPatternStatus = String.Format("SLM sequence #" + (seq + 1).ToString() + " pattern time of ID #" + patternIDs[i].ToString() + " should be > " + PatternMinMS.ToString() + " msec.\n");
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

                    ResourceManagerCS.DeleteFile(pathName);

                    for (int j = 0; j < epochCount; j++)
                    {
                        firstEpoch = (0 == j) ? true : false;
                        lastEpoch = (epochCount - 1 == j) ? true : false;

                        for (int i = 0; i < slmBleachWaveParams.Count; i++)
                        {
                            SLMPatternStatus = String.Format("Building SLM ...\nWaveform # " + (seq + 1).ToString() + ", Epoch # " + (j + 1).ToString() + ", Pattern # " + (i + 1).ToString() + "\n");

                            SLMParams slmParam = slmBleachWaveParams[i];
                            List<double> powerVal = new List<double>();
                            for (int pid = 0; pid < _vm.BleachCalibratePockelsVoltageMin0.Length; pid++)
                            {
                                powerVal.Add(WaveformBuilder.GetPockelsPowerValue((double)slmParam.BleachWaveParams.GetType().GetProperty((0 < pid) ? "Power" + pid : "Power").GetValue(slmParam.BleachWaveParams),
                                    _vm.BleachCalibratePockelsVoltageMin0[pid], _vm.BleachCalibratePockelsVoltageMax0[pid],
                                    (PockelsResponseType)MVMManager.Instance["PowerControlViewModel", "BleacherPowerResponse" + pid, PockelsResponseType.SINE_RESPONSE]));
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

                    WaveformBuilder.SaveWaveform(pathName, true, new bool[(int)SignalType.SIGNALTYPE_LAST] { !_vm.IsStimulator, true, true, false });

                    while (!WaveformBuilder.CheckSaveState())
                    {
                        System.Threading.Thread.Sleep(50);

                        if (true == slmWorker.CancellationPending)
                        {
                            WaveformBuilder.StopSave();
                        }
                    }

                    //true if nothing to build
                    SLMFileSaved = (0 >= slmBleachWaveParams.Count) || WaveformBuilder.GetSaveResult();

                    if (!SLMFileSaved)
                    {
                        ResourceManagerCS.DeleteFile(seqTextName);
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

        void SLMCalibrator3D_DoWork(object sender, DoWorkEventArgs e)
        {
            string roiType = string.Empty;
            Point offCenter = new Point(-1, -1);
            List<Point> pts = new List<Point>();

            SLMSpinProgressVisible = true;

            //retrieve user selected points:
            this.Dispatcher.Invoke((SLMSavePattern)delegate
             {
                 pts = OverlayManagerClass.Instance.GetPatternROICenters(OverlayManagerClass.Instance.PatternID, ref roiType, ref offCenter);
             });
            //validate count first:
            if (0 != pts.Count % CalibratePointCount)
            {
                e.Result = "Number of your selections(" + pts.Count.ToString() + ") is invalid.\n";
                return;
            }
            else
            {
                SLMPatternStatus = String.Format("Doing SLM Calibration,\nplease wait until done ... \n");

                //create mask, PatternID 1 are targets to be burnt,
                //PatternID 2 are sources for affine calibration:
                Dictionary<double, List<Shape>> tROIs = new Dictionary<double, List<Shape>>(), zROIs = new Dictionary<double, List<Shape>>();
                List<float> ptsTo = new List<float>(), ptsFrom = new List<float>(), ptsTmp = new List<float>();
                float kz = 0;

                _vm.SLMCalibZPos = (double)MVMManager.Instance["ZControlViewModel", "ZPosition", (object)0];
                Point center = new Point(_vm.ImageWidth / 2, _vm.ImageHeight / 2);

                this.Dispatcher.Invoke((SLMSavePattern)delegate
                {
                    tROIs = OverlayManagerClass.Instance.GetPatternZROIs(-1, OverlayManagerClass.Instance.PatternID - 1);
                    zROIs = OverlayManagerClass.Instance.GetPatternZROIs(-1, OverlayManagerClass.Instance.PatternID);
                });

                ptsTo.Add((float)center.X);
                ptsTo.Add((float)center.Y);
                ptsTo.Add((float)0.0);
                foreach (KeyValuePair<double, List<Shape>> entry in tROIs)
                {
                    kz = (float)(2 * Math.PI * entry.Key * (float)Constants.UM_TO_MM / _vm.SLMWavelengthNM);
                    foreach (Shape slist in entry.Value)
                    {
                        if (slist.GetType() == typeof(OverlayManager.ROICrosshair))
                        {
                            ptsTmp.Add((float)((OverlayManager.ROICrosshair)slist).CenterPoint.X);
                            ptsTmp.Add((float)((OverlayManager.ROICrosshair)slist).CenterPoint.Y);
                            ptsTmp.Add(kz);
                        }
                        else if (slist.GetType() == typeof(OverlayManager.ROIEllipse))
                        {
                            ptsTmp.Add((float)((OverlayManager.ROIEllipse)slist).ROICenter.X);
                            ptsTmp.Add((float)((OverlayManager.ROIEllipse)slist).ROICenter.Y);
                            ptsTmp.Add(kz);
                        }
                    }
                }

                bool centerAdded = false;
                foreach (KeyValuePair<double, List<Shape>> entry in zROIs)
                {
                    kz = (float)(2 * Math.PI * entry.Key * (float)Constants.UM_TO_MM / _vm.SLMWavelengthNM);
                    if (!centerAdded)
                    {
                        ptsTo[2] = (float)entry.Key;
                        ptsFrom.Add((float)offCenter.X);
                        ptsFrom.Add((float)offCenter.Y);
                        ptsFrom.Add(kz);
                        centerAdded = true;
                    }

                    //update reference pattern with z um value:
                    for (int i = 2; i < ptsTmp.Count; i += 3)
                    {
                        ptsTmp[i] = (float)entry.Key;
                    }
                    ptsTo.AddRange(ptsTmp);

                    foreach (Shape slist in entry.Value)
                    {
                        if (slist.GetType() == typeof(OverlayManager.ROICrosshair))
                        {
                            ptsFrom.Add((float)((OverlayManager.ROICrosshair)slist).CenterPoint.X);
                            ptsFrom.Add((float)((OverlayManager.ROICrosshair)slist).CenterPoint.Y);
                            ptsFrom.Add(kz);
                        }
                        else if (slist.GetType() == typeof(OverlayManager.ROIEllipse))
                        {
                            ptsFrom.Add((float)((OverlayManager.ROIEllipse)slist).ROICenter.X);
                            ptsFrom.Add((float)((OverlayManager.ROIEllipse)slist).ROICenter.Y);
                            ptsFrom.Add(kz);
                        }
                    }
                }
                //execute calibration:
                e.Result = (_vm.SLMCalibration("", ptsFrom.ToArray(), ptsTo.ToArray(), ptsTo.Count)) ? "0" : String.Format("Calibration Failed\n");
            }
        }

        private void SLMCalibrator3D_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            // check error, cancel, then result:
            if (e.Error != null)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "SLMCalibrator: " + e.Error.Message);
            }
            else if (!e.Cancelled)
            {
                try
                {
                    if (0 != ((string)e.Result).CompareTo("0"))
                    {
                        _vm.SLMSetBlank();
                        OverlayManagerClass.Instance.ClearNonSaveROIs(ref CaptureSetupViewModel.OverlayCanvas);
                        OverlayManagerClass.Instance.DisplayPatternROI(ref CaptureSetupViewModel.OverlayCanvas, OverlayManagerClass.Instance.PatternID - 1, false);
                        string strBody = e.Result + "\n\nKeep Z at reference and use Cross Hair\nROI tool to mark the single imaging bead\nthen Press Yes to continue calibration,\nPress No to cancel.";
                        SLMCalibPanel slmCalib = new SLMCalibPanel("SPOT_SELECT", "DONE", "New Calibration", strBody);
                        this.DataContext = slmCalib;
                        this.Show();
                    }
                    else
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
            slmCalibrator.DoWork -= new DoWorkEventHandler(SLMCalibrator3D_DoWork);
            slmCalibrator.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(SLMCalibrator3D_RunWorkerCompleted);
            if (0 == ((string)e.Result).CompareTo("0")) { CalibrationProc("DONE"); }
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
                if (null == roiCapsule.ROIs)
                    return;

                //clear null items:
                roiCapsule.ROIs = roiCapsule.ROIs.Where(item => item != null).ToArray();

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
                        if (0 == ((int[])(((OverlayManager.ROIEllipse)(roiCapsule.ROIs[i])).Tag))[(int)ThorSharedTypes.Tag.SUB_PATTERN_ID])
                            continue;

                        switch (((int[])(((OverlayManager.ROIEllipse)(roiCapsule.ROIs[i])).Tag))[(int)ThorSharedTypes.Tag.PATTERN_ID])
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

            //expect above are in Sub Pattern ID order,
            //keep the first as image center for later transform in SLM
            List<Point> calPointsToMap = new List<Point>();
            calPointsToMap = calPointsTo;
            calPointsTo.Insert(0, new Point(_vm.ImageWidth / 2, _vm.ImageHeight / 2));
            calPointsFrom.Insert(0, new Point(_vm.ImageWidth / 2, _vm.ImageHeight / 2));

            float[] ptsTo = PointsToFloatVec(calPointsTo);
            float[] ptsFrom = (1 >= calPointsFrom.Count) ? ptsTo : PointsToFloatVec(calPointsFrom);

            //check if both are valid counts, one more for image center:
            if (((CalibratePointCount + 1) * 2) != ptsTo.Length || ((CalibratePointCount + 1) * 2) != ptsFrom.Length)
            {
                e.Result = "Invalid count of points.\n";
                return;
            }

            //create mask for later used slm:
            System.Drawing.Bitmap bmp = ProcessBitmap.CreateBinaryBitmap(_vm.ImageWidth, _vm.ImageHeight, calPointsToMap);
            string bmpPath = _vm.BleachROIPath + "SLMCalibROIs.bmp";
            bmp.Save(bmpPath, System.Drawing.Imaging.ImageFormat.Bmp);

            //create center Galvo waveform file, also determine if need reload:
            e.Result = BuildCenterBleachWaveform();

            //execute calibration:
            if (0 == e.Result.ToString().CompareTo("0"))
            {
                SLMPatternStatus = String.Format("Loading SLM Calibration Waveform, \nplease wait until done ... \n");
                e.Result = (_vm.SLMCalibration(bmpPath, ptsFrom, ptsTo, ptsTo.Length)) ? "0" : "Calibration Failed\n";
            }
        }

        private void SLMCalibrator_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            SLMCalibPanel slmCalib;
            string roiPathName = _vm.BleachROIPath + _vm.SLMCalibFile;
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
                        new SLMCalibPanel("CHECK", "REDO", "Verify Calibration", "Do the calibration spots align with the overlaid \ncalibration pattern?\n\n[NOTE: Click 'NO' will reset calibrations.]\n") :
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
                CancelSLMBackgroundWorker(slmBuilder);
                CancelSLMBackgroundWorker(slmWorker, true);
                CancelSLMBackgroundWorker(slmPreviewWorker);
                CancelSLMBackgroundWorker(slmBuildAllWorker);
                CancelSLMBackgroundWorker(slmImportWorker);
                CancelCalibrator();

                _vm.IdleSLM();

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

                if (Application.Current.MainWindow != null)
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

            slmPreviewWorker = new BackgroundWorker();
            slmPreviewWorker.WorkerReportsProgress = false;
            slmPreviewWorker.WorkerSupportsCancellation = true;

            slmImportWorker = new BackgroundWorker();
            slmImportWorker.WorkerReportsProgress = false;
            slmImportWorker.WorkerSupportsCancellation = true;

            slmBuildAllWorker = new BackgroundWorker();
            slmBuildAllWorker.WorkerReportsProgress = false;
            slmBuildAllWorker.WorkerSupportsCancellation = true;

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

        /// <summary>
        /// event to handle SLM bleach (measured) power update
        /// </summary>
        void SLMParamsCurrent_PowerParamsChangedEvent()
        {
            UpdateSLMParamPower();
        }

        private void SLMWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker worker = sender as BackgroundWorker;
            if (worker.CancellationPending != true)
            {
                SaveSLMFiles();
                UpdateSLMParamPower();
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

        private bool TryExportPatternSequences(string XmlPath, bool includeSequences)
        {
            string str = string.Empty;
            int iVal = 0;
            double dVal = 0.0;
            XmlDocument activeXml = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];
            XmlNodeList ndActiveParent = activeXml.SelectNodes("/ThorImageExperiment/SLM");
            XmlNodeList ndActive = activeXml.SelectNodes("/ThorImageExperiment/SLM/Pattern");
            if (0 >= ndActive.Count)
            {
                SLMPatternStatus = String.Format("No SLM patterns to export.");
                return false;
            }

            XmlDocument doc = new XmlDocument();
            doc.AppendChild(doc.CreateProcessingInstruction("xml", "version=\"1.0\""));
            XmlElement root = doc.CreateElement("ThorImageSLM");
            doc.AppendChild(root);
            XmlNodeList ndList = doc.SelectNodes("/ThorImageSLM");

            XmlManager.CreateXmlNodeWithinNode(doc, ndList[0], "SLMPatterns");
            XmlNodeList ndDoc = doc.SelectNodes("/ThorImageSLM/SLMPatterns");
            XmlManager.SetAttribute(ndDoc[0], doc, "BleacherType", ((int)ResourceManagerCS.GetBleacherType()).ToString());
            XmlManager.SetAttribute(ndDoc[0], doc, "holoGen3D", (XmlManager.GetAttribute(ndActiveParent[0], activeXml, "holoGen3D", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)) ? iVal.ToString() : "0");

            for (int i = 0; i < ndActive.Count; i++)
                XmlManager.CreateXmlNodeWithinNode(doc, ndDoc[0], "Pattern");

            int idx = 0;
            int patternID = 1;
            double pxSizeUM = 1.0, roiWidthPx = 0, roiHeightPx = 0;
            ndList = doc.SelectNodes("/ThorImageSLM/SLMPatterns/Pattern");
            for (int p = 0; p < ndList.Count; p++)
            {
                XmlManager.SetAttribute(ndList[p], doc, "name",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "name", ref str) && !string.IsNullOrEmpty(str) && !ContainsInvalidPathCharacters(str)) ? str : ("Pattern" + string.Format("{0:000}", ++idx))
                    );

                patternID = (XmlManager.GetAttribute(ndActive[p], activeXml, "fileID", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)) ? iVal : p + 1;
                XmlManager.SetAttribute(ndList[p], doc, "patternID", patternID.ToString());

                XmlManager.SetAttribute(ndList[p], doc, "red",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "red", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal.ToString() : "255"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "green",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "green", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal.ToString() : "255"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "blue",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "blue", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal.ToString() : "255"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "pxSpacing",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "pxSpacing", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal.ToString() : "1"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "durationMS",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "durationMS", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal.ToString() : "100"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "iterations",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "iterations", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)) ? iVal.ToString() : "10"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "power",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "power", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal.ToString() : "0"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "prePatIdleMS",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "prePatIdleMS", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal.ToString() : "0"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "postPatIdleMS",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "postPatIdleMS", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal.ToString() : "0"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "preIteIdleMS",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "preIteIdleMS", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal.ToString() : "0"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "postIteIdleMS",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "postIteIdleMS", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal.ToString() : "0"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "measurePowerMW",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "measurePowerMW", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal.ToString() : "0"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "measurePowerMWPerUM2",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "measurePowerMWPerUM2", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal.ToString() : "0"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "power1",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "power1", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal.ToString() : "0"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "measurePower1MW",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "measurePower1MW", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal.ToString() : "0"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "measurePower1MWPerUM2",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "measurePower1MWPerUM2", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal.ToString() : "0"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "pixelSizeUM",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "pixelSizeUM", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out pxSizeUM)) ? pxSizeUM.ToString() : "1"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "xOffsetUM",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "xOffsetUM", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal.ToString() : "0"
                    );

                XmlManager.SetAttribute(ndList[p], doc, "yOffsetUM",
                    (XmlManager.GetAttribute(ndActive[p], activeXml, "yOffsetUM", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal.ToString() : "0"
                    );

                if (XmlManager.GetAttribute(ndActive[p], activeXml, "shape", ref str)) XmlManager.SetAttribute(ndList[p], doc, "shape", str);

                if (XmlManager.GetAttribute(ndActive[p], activeXml, "roiWidthUM", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal))
                {
                    roiWidthPx = (dVal / pxSizeUM);
                    XmlManager.SetAttribute(ndList[p], doc, "roiWidthPx", roiWidthPx.ToString());
                }
                if (XmlManager.GetAttribute(ndActive[p], activeXml, "roiHeightUM", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal))
                {
                    roiHeightPx = (dVal / pxSizeUM);
                    XmlManager.SetAttribute(ndList[p], doc, "roiHeightPx", roiHeightPx.ToString());
                }

                bool backupWavelength = _vm.SLMSelectWavelengthProp;
                this.Dispatcher.Invoke((SLMSavePattern)delegate
                {
                    List<Shape> rois = OverlayManagerClass.Instance.GetModeROIs((_vm.IsStimulator ? Mode.PATTERN_WIDEFIELD : Mode.PATTERN_NOSTATS), patternID);
                    for (int r = 0; r < rois.Count; r++)
                        XmlManager.CreateXmlNodeWithinNode(doc, ndList[p], "ROI");

                    for (int i = 0; i < _vm.SLMWavelengthCount; i++)
                    {
                        _vm.SLMSelectWavelengthProp = (1 == i);

                        XmlNodeList ndROI = ndList[p].SelectNodes("ROI");
                        for (int r = 0; r < rois.Count; r++)
                        {
                            Dictionary<double, List<Shape>> zSubRoi = OverlayManagerClass.Instance.GetPatternZROIs((int)(_vm.IsStimulator ? Mode.PATTERN_WIDEFIELD : Mode.PATTERN_NOSTATS), patternID, _vm.SLMWavelengthNM, (r + 1).ToString());

                            foreach (KeyValuePair<double, List<Shape>> subRoi in zSubRoi)
                            {
                                if (0 < subRoi.Value.Count)   //should be only one per subID
                                {
                                    string strVertice = str = string.Empty;
                                    Point center = new Point();
                                    double left = double.MaxValue, right = 0, top = double.MaxValue, bottom = 0;
                                    KeyValuePair<string, Type> shapeType = OverlayManagerClass.ROITypeDictionary.FirstOrDefault(x => x.Value == subRoi.Value[0].GetType());
                                    XmlManager.SetAttribute(ndROI[r], doc, "subID", (r + 1).ToString());
                                    if (typeof(ROIPoly) == shapeType.Value)
                                    {
                                        foreach (Point pt in ((ROIPoly)rois[r]).Points)
                                        {
                                            strVertice += pt.X + "," + pt.Y + " ";
                                            left = Math.Min(left, pt.X);
                                            right = Math.Max(right, pt.X);
                                            top = Math.Min(top, pt.Y);
                                            bottom = Math.Min(bottom, pt.Y);
                                        }
                                        XmlManager.SetAttribute(ndROI[r], doc, "vertices", strVertice);
                                        center.X = (left + right) / 2;
                                        center.Y = (top + bottom) / 2;
                                        roiWidthPx = Math.Max(roiWidthPx, Math.Abs(right - left));
                                        roiHeightPx = Math.Max(roiHeightPx, Math.Abs(bottom - top));
                                    }
                                    else if (typeof(Polyline) == shapeType.Value)
                                    {
                                        foreach (Point pt in ((Polyline)rois[r]).Points)
                                        {
                                            strVertice += pt.X + "," + pt.Y + " ";
                                            left = Math.Min(left, pt.X);
                                            right = Math.Max(right, pt.X);
                                            top = Math.Min(top, pt.Y);
                                            bottom = Math.Min(bottom, pt.Y);
                                        }
                                        XmlManager.SetAttribute(ndROI[r], doc, "vertices", strVertice);
                                        center.X = (left + right) / 2;
                                        center.Y = (top + bottom) / 2;
                                        roiWidthPx = Math.Max(roiWidthPx, Math.Abs(right - left));
                                        roiHeightPx = Math.Max(roiHeightPx, Math.Abs(bottom - top));
                                    }
                                    else if (typeof(Line) == shapeType.Value)
                                    {
                                        strVertice += ((Line)subRoi.Value[0]).X1 + "," + ((Line)subRoi.Value[0]).Y1 + " ";
                                        strVertice += ((Line)subRoi.Value[0]).X2 + "," + ((Line)subRoi.Value[0]).Y2 + " ";
                                        XmlManager.SetAttribute(ndROI[r], doc, "vertices", strVertice);
                                        center.X = (((Line)subRoi.Value[0]).X1 + ((Line)subRoi.Value[0]).X2) / 2;
                                        center.Y = (((Line)subRoi.Value[0]).Y1 + ((Line)subRoi.Value[0]).Y2) / 2;
                                        roiWidthPx = Math.Max(roiWidthPx, Math.Abs(((Line)subRoi.Value[0]).X2 - ((Line)subRoi.Value[0]).X1));
                                        roiHeightPx = Math.Max(roiHeightPx, Math.Abs(((Line)subRoi.Value[0]).Y2 - ((Line)subRoi.Value[0]).Y1));
                                    }
                                    else if (typeof(ROIRect) == shapeType.Value)
                                    {
                                        center.X = (((ROIRect)subRoi.Value[0]).StartPoint.X + ((ROIRect)subRoi.Value[0]).EndPoint.X) / 2;
                                        center.Y = (((ROIRect)subRoi.Value[0]).StartPoint.Y + ((ROIRect)subRoi.Value[0]).EndPoint.Y) / 2;
                                        roiWidthPx = ((ROIRect)subRoi.Value[0]).ROIWidth;
                                        roiHeightPx = ((ROIRect)subRoi.Value[0]).ROIHeight;
                                    }
                                    else if (typeof(ROIEllipse) == shapeType.Value)
                                    {
                                        center.X = ((ROIEllipse)subRoi.Value[0]).ROICenter.X;
                                        center.Y = ((ROIEllipse)subRoi.Value[0]).ROICenter.Y;
                                        roiWidthPx = ((ROIEllipse)subRoi.Value[0]).ROIWidth;
                                        roiHeightPx = ((ROIEllipse)subRoi.Value[0]).ROIHeight;
                                    }
                                    else if (typeof(ROICrosshair) == shapeType.Value)
                                    {
                                        center.X = ((ROICrosshair)subRoi.Value[0]).CenterPoint.X;
                                        center.Y = ((ROICrosshair)subRoi.Value[0]).CenterPoint.Y;
                                        roiWidthPx = roiHeightPx = 0;
                                    }
                                    XmlManager.SetAttribute(ndROI[r], doc, "ZUM", subRoi.Key.ToString());       //relative Z[um] in overlay manager
                                    XmlManager.SetAttribute(ndROI[r], doc, "centerX", center.X.ToString());
                                    XmlManager.SetAttribute(ndROI[r], doc, "centerY", center.Y.ToString());
                                    XmlManager.SetAttribute(ndROI[r], doc, "shape", shapeType.Key);
                                    XmlManager.SetAttribute(ndROI[r], doc, "wavelengthNM", _vm.SLMWavelengthNM.ToString());
                                    XmlManager.SetAttribute(ndROI[r], doc, "roiWidthPx", roiWidthPx.ToString());
                                    XmlManager.SetAttribute(ndROI[r], doc, "roiHeightPx", roiHeightPx.ToString());
                                }
                            } //end of foreach [Z, ROIs] dictionary
                        } //end of for rois
                    } //end of for wavelength
                });
                _vm.SLMSelectWavelengthProp = backupWavelength;
            }

            if (includeSequences)
            {
                //export SLM sequences
                ndList = doc.SelectNodes("/ThorImageSLM");

                XmlManager.CreateXmlNodeWithinNode(doc, ndList[0], "SLMSequences");
                ndDoc = doc.SelectNodes("/ThorImageSLM/SLMSequences");

                for (int i = 0; i < _vm.EpochSequence.Count; i++)
                    XmlManager.CreateXmlNodeWithinNode(doc, ndDoc[0], "SequenceEpoch");

                ndList = doc.SelectNodes("/ThorImageSLM/SLMSequences/SequenceEpoch");

                for (int i = 0; i < ndList.Count; i++)
                {
                    XmlManager.SetAttribute(ndList[i], doc, "sequenceID", i.ToString());
                    XmlManager.SetAttribute(ndList[i], doc, "sequence", _vm.EpochSequence[i].SequenceStr);
                    XmlManager.SetAttribute(ndList[i], doc, "sequenceEpochCount", _vm.EpochSequence[i].EpochCount);
                }
            }

            doc.Save(XmlPath);
            return true;
        }

        /// <summary>
        /// Import SLM patterns from custom xml
        /// </summary>
        /// <param name="xmlSLMImportDoc"></param>
        /// <returns></returns>
        private bool TryImportPatternSequences(XmlDocument xmlSLMImportDoc, bool includeSequences)
        {
            string str1 = string.Empty, str2 = string.Empty, str3 = string.Empty;
            double dVal1 = 0, dVal2 = 0, dVal3 = 0, dVal4 = 0, zUM = 0;
            int iVal = 0;
            bool workStatus = true;
            ICamera.LSMType bType = ICamera.LSMType.GALVO_GALVO;

            if (!_vm.IsStimulator && 0 >= _vm.BleachLSMUMPerPixel)
            {
                SLMPatternStatus = String.Format("Calibration must be done before import.");
                return false;
            }

            //load general settings
            XmlNodeList ndParent = xmlSLMImportDoc.SelectNodes("/ThorImageSLM/SLMPatterns");
            if (0 >= ndParent.Count)
            {
                SLMPatternStatus = String.Format("SLMPatterns node is missing.");
                return false;
            }
            //verify bleacher type
            bType = (XmlManager.GetAttribute(ndParent[0], xmlSLMImportDoc, "BleacherType", ref str1) && Int32.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)) ? (ICamera.LSMType)iVal : bType;
            if (bType != (ICamera.LSMType)ResourceManagerCS.GetBleacherType())
            {
                SLMPatternStatus = String.Format("Stimulator must be configured the same as import.");
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
                        //SLMPatternStatus = String.Format("Please backup the SLM patterns (as a template) before import or select another xml.");
                        return false;
                    }
                }
                //backup properties
                bool backupWavelength = _vm.SLMSelectWavelengthProp;
                ThorSharedTypes.Mode backupMode = OverlayManagerClass.Instance.CurrentMode;
                OverlayManagerClass.Instance.CurrentMode = (ICamera.LSMType.STIMULATE_MODULATOR == bType) ? ThorSharedTypes.Mode.PATTERN_WIDEFIELD : ThorSharedTypes.Mode.PATTERN_NOSTATS;
                OverlayManagerClass.Instance.SimulatorMode = true;   //Make overlay manager not accessing hardware mvms, but properties only

                //clear history
                _vm.SLM3D = (XmlManager.GetAttribute(ndParent[0], xmlSLMImportDoc, "holoGen3D", ref str1) && Int32.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)) ? (1 == iVal) : false;
                _vm.SLMSequenceOn = false;
                this.Dispatcher.Invoke((SLMWorkerSetGUI)delegate
                {
                    _vm.SLMBleachWaveParams.Clear();
                    OverlayManagerClass.Instance.DeletePatternROI(ref CaptureSetupViewModel.OverlayCanvas, -1, OverlayManagerClass.Instance.CurrentMode);
                    for (int i = 0; i < _vm.SLMWaveformFolder.Length; i++)
                        _vm.ClearSLMFiles(_vm.SLMWaveformFolder[i]);

                    //start to draw
                    OverlayManagerClass.Instance.InitSelectROI(ref CaptureSetupViewModel.OverlayCanvas);
                });

                //through pattern nodes
                for (int i = 0; i < ndList.Count; i++)
                {
                    //locate patternID in order, [1-based]
                    //Pattern nodes for galvo waveform params, ROI nodes for individual pattern info
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

                    if (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "red", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1) &&
                        XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "green", ref str2) && Double.TryParse(str2, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal2) &&
                        XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "blue", ref str3) && Double.TryParse(str3, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal3))
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

                    sparam.BleachWaveParams.UMPerPixel = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "pixelSizeUM", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                        dVal1 : (double)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (object)1.0];

                    sparam.BleachWaveParams.UMPerPixelRatio = (0 < _vm.BleachLSMUMPerPixel) ? sparam.BleachWaveParams.UMPerPixel / _vm.BleachLSMUMPerPixel : 1.0;

                    //[NOTE]: shape, roiWidthPx, roiHeightPx duplicate in the ROI sub nodes,
                    //        roiWidthPx & roiHeightPx here used for GG and those in ROI sub nodes for pattern drawing
                    dVal1 = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "roiWidthPx", ref str1) && Double.TryParse(str1, out dVal1)) ? dVal1 : 0;
                    dVal2 = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "roiHeightPx", ref str2) && Double.TryParse(str2, out dVal2)) ? dVal2 : 0;

                    Type stype = typeof(ROIEllipse);
                    if (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "shape", ref str3) && OverlayManagerClass.ROITypeDictionary.ContainsKey(str3.ToUpper()))
                    {
                        stype = OverlayManagerClass.ROITypeDictionary[str3.ToUpper()];
                    }
                    else
                    {
                        stype = ((dVal1 == 0 && dVal2 == 0) ||
                        (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "shape", ref str3) && 0 <= str3.IndexOf("Crosshair", StringComparison.OrdinalIgnoreCase))) ?
                        typeof(ROICrosshair) : typeof(ROIEllipse);
                    }

                    sparam.BleachWaveParams.ROIWidth = (typeof(ROICrosshair) == stype) ? 0 : dVal1;
                    sparam.BleachWaveParams.ROIHeight = (typeof(ROICrosshair) == stype) ? 0 : dVal2;

                    sparam.PixelSpacing = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "pxSpacing", ref str1) && Int32.TryParse(str1, out iVal) && (1 <= iVal)) ? iVal : 1;

                    sparam.BleachWaveParams.Iterations = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "iterations", ref str1) && Int32.TryParse(str1, out iVal)) ?
                        Math.Max(1, iVal) : 1;

                    sparam.BleachWaveParams.PrePatIdleTime = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "prePatIdleMS", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                        Math.Max(0, dVal1) : 0;

                    sparam.BleachWaveParams.PostPatIdleTime = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "postPatIdleMS", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                        Math.Max(0, dVal1) : 0;

                    sparam.BleachWaveParams.PreIdleTime = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "preIteIdleMS", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                        Math.Max(0, dVal1) : 0;

                    sparam.BleachWaveParams.PostIdleTime = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "postIteIdleMS", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                        Math.Max(0, dVal1) : 0;

                    sparam.BleachWaveParams.Power = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "power", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                        dVal1 : 0;
                    sparam.BleachWaveParams.Power1 = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "power1", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal2)) ?
                        dVal2 : -1.0;

                    sparam.BleachWaveParams.MeasurePower = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "measurePowerMW", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                        dVal1 : 0.0;
                    sparam.BleachWaveParams.MeasurePower1 = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "measurePower1MW", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal2)) ?
                        dVal2 : -1.0;

                    sparam.SLMMeasurePowerArea = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "measurePowerMWPerUM2", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                        dVal1 : 0.0;
                    sparam.SLMMeasurePowerArea1 = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "measurePower1MWPerUM2", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal2)) ?
                        dVal2 : -1.0;

                    sparam.Red = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "red", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                        dVal1 : 128;

                    sparam.Green = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "green", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                        dVal1 : 128;

                    sparam.Blue = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "blue", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                        dVal1 : 128;

                    sparam.Duration = (XmlManager.GetAttribute(xNode, xmlSLMImportDoc, "durationMS", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                        Math.Max(0, dVal1) : 0;

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
                    //In Galvo-Galvo, only the first takes start-end, the rest center only
                    bool firstDraw = true;
                    for (int id = 1, j = 0; j < subIDdic.Count; j++, id++)  //leave id = 0 for later
                    {
                        if (subIDdic.ContainsKey(id))
                        {
                            iVal = (XmlManager.GetAttribute(xNode.ChildNodes[subIDdic[id]], xmlSLMImportDoc, "wavelengthNM", ref str1) && Int32.TryParse(str1, out iVal)) ? iVal : _vm.SLMWavelengthNM;
                            _vm.SLMSelectWavelengthProp = (_vm.SLMWavelengthNM == iVal) ? _vm.SLMSelectWavelengthProp : !_vm.SLMSelectWavelengthProp;
                            OverlayManagerClass.Instance.WavelengthNM = _vm.SLMWavelengthNM;

                            //update Z[um] before drawing
                            OverlayManagerClass.Instance.ZUM = ((XmlManager.GetAttribute(xNode.ChildNodes[subIDdic[id]], xmlSLMImportDoc, "ZUM", ref str1) && Double.TryParse(str1, out zUM)) ? zUM : 0.0)
                                                                + (_vm.SLMZRefMM * (double)Constants.UM_TO_MM);

                            dVal1 = (XmlManager.GetAttribute(xNode.ChildNodes[subIDdic[id]], xmlSLMImportDoc, "roiWidthPx", ref str1) && Double.TryParse(str1, out dVal1)) ? dVal1 : sparam.BleachWaveParams.ROIWidth;
                            dVal2 = (XmlManager.GetAttribute(xNode.ChildNodes[subIDdic[id]], xmlSLMImportDoc, "roiHeightPx", ref str2) && Double.TryParse(str2, out dVal2)) ? dVal2 : sparam.BleachWaveParams.ROIHeight;

                            stype = (XmlManager.GetAttribute(xNode.ChildNodes[subIDdic[id]], xmlSLMImportDoc, "shape", ref str3) && OverlayManagerClass.ROITypeDictionary.ContainsKey(str3.ToUpper())) ?
                                OverlayManagerClass.ROITypeDictionary.FirstOrDefault(x => x.Key == str3.ToUpper()).Value : stype;

                            if (XmlManager.GetAttribute(xNode.ChildNodes[subIDdic[id]], xmlSLMImportDoc, "centerX", ref str1) && Double.TryParse(str1, out dVal3) &&
                                XmlManager.GetAttribute(xNode.ChildNodes[subIDdic[id]], xmlSLMImportDoc, "centerY", ref str2) && Double.TryParse(str2, out dVal4))
                            {
                                this.Dispatcher.Invoke((SLMWorkerSetGUI)delegate
                                {
                                    //use vertices to determine shape drawing: polygon, polyline, line
                                    if (typeof(ROIPoly) == stype || typeof(Polyline) == stype || typeof(Line) == stype)
                                    {
                                        if (!firstDraw)
                                        {
                                            OverlayManagerClass.Instance.CreateROIShape(ref CaptureSetupViewModel.OverlayCanvas,
                                                stype,
                                                new Point(dVal3, dVal4),
                                                new Point(dVal3 + dVal1 / 2, dVal4 + dVal2 / 2));
                                        }
                                        else
                                        {
                                            //start draw polys and lines
                                            if (XmlManager.GetAttribute(xNode.ChildNodes[subIDdic[id]], xmlSLMImportDoc, "vertices", ref str1))
                                            {
                                                string[] strVertices = str1.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                                                for (int sV = 0; sV < strVertices.Count(); sV++)
                                                {
                                                    string[] vXY = strVertices[sV].Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries);
                                                    if (2 <= vXY.Count())
                                                    {
                                                        if (Double.TryParse(vXY[0], out dVal3) && Double.TryParse(vXY[1], out dVal4))
                                                        {
                                                            if (0 == sV)
                                                            {
                                                                //define range based on start and end point
                                                                if (typeof(Line) == stype)
                                                                {
                                                                    //keep start point at [X1=dVal1, Y1=dVal2]
                                                                    dVal1 = dVal3; dVal2 = dVal4;
                                                                    continue;
                                                                }
                                                                OverlayManagerClass.Instance.CreateROIShape(ref CaptureSetupViewModel.OverlayCanvas,
                                                                    stype,
                                                                    new Point(dVal3, dVal4),
                                                                    new Point(dVal3 + dVal1 / 2, dVal4 + dVal2 / 2));
                                                            }
                                                            else
                                                            {
                                                                if (typeof(Line) == stype)
                                                                {
                                                                    OverlayManagerClass.Instance.CreateROIShape(ref CaptureSetupViewModel.OverlayCanvas,
                                                                      stype,
                                                                      new Point(dVal1, dVal2),
                                                                      new Point(dVal3, dVal4));
                                                                }
                                                                else
                                                                    OverlayManagerClass.Instance.AddPointToObject(ref CaptureSetupViewModel.OverlayCanvas, new Point(dVal3, dVal4));
                                                            }
                                                        }
                                                    }
                                                }
                                                //add a dummy then close for ROIPoly
                                                OverlayManagerClass.Instance.AddPointToObject(ref CaptureSetupViewModel.OverlayCanvas, new Point(0, 0));
                                                OverlayManagerClass.Instance.CloseObject(ref CaptureSetupViewModel.OverlayCanvas, new Point(0, 0));
                                            }
                                        }
                                    }
                                    else    //use roi size to define shape drawing: rectangle, ellipse
                                    {
                                        OverlayManagerClass.Instance.CreateROIShape(ref CaptureSetupViewModel.OverlayCanvas,
                                            stype,
                                            firstDraw ? new Point(dVal3 - dVal1 / 2, dVal4 - dVal2 / 2) : new Point(dVal3, dVal4),
                                            new Point(dVal3 + dVal1 / 2, dVal4 + dVal2 / 2));
                                    }
                                    OverlayManagerClass.Instance.InitSelectROI(ref CaptureSetupViewModel.OverlayCanvas);
                                });
                                firstDraw = (ICamera.LSMType.GALVO_GALVO == bType) ? false : true;
                            }
                        }
                    }
                    //set ROI center at the end if provided as subID = 0, fetch otherwise,
                    //no need if STIMULATE_MODULATOR type
                    sparam.BleachWaveParams.Center = new Point(0, 0);
                    if (ICamera.LSMType.GALVO_GALVO == bType)
                    {
                        if (subIDdic.ContainsKey(0))
                        {
                            if (XmlManager.GetAttribute(xNode.ChildNodes[subIDdic[0]], xmlSLMImportDoc, "centerX", ref str1) && Double.TryParse(str1, out dVal3) &&
                                 XmlManager.GetAttribute(xNode.ChildNodes[subIDdic[0]], xmlSLMImportDoc, "centerY", ref str2) && Double.TryParse(str2, out dVal4))
                            {
                                this.Dispatcher.Invoke((SLMWorkerSetGUI)delegate
                                {
                                    OverlayManagerClass.Instance.CreateROIShape(ref CaptureSetupViewModel.OverlayCanvas, typeof(ROICrosshair), new Point(dVal3, dVal4), new Point(dVal3, dVal4));
                                    OverlayManagerClass.Instance.InitSelectROI(ref CaptureSetupViewModel.OverlayCanvas);
                                });
                                sparam.BleachWaveParams.Center = new Point(dVal3, dVal4);
                            }
                        }
                        else
                        {
                            Point offCenter = new Point(-1, -1);
                            this.Dispatcher.Invoke((SLMWorkerSetGUI)delegate
                            {
                                OverlayManagerClass.Instance.CreateROICenter(ref CaptureSetupViewModel.OverlayCanvas);
                                OverlayManagerClass.Instance.ValidateROIs(ref CaptureSetupViewModel.OverlayCanvas);
                                OverlayManagerClass.Instance.GetPatternROICenters(OverlayManagerClass.Instance.PatternID, ref str1, ref offCenter);
                            });
                            if ((0 > offCenter.X) || (0 > offCenter.Y))
                            {
                                SLMPatternStatus = String.Format("Unable to calculate ROIs center.");
                                workStatus = false;
                                break;
                            }
                            sparam.BleachWaveParams.Center = offCenter;
                        }
                    }
                    sparam.BleachWaveParams.Fill = 1;

                    SLMParamsCurrent = new SLMParams(sparam);
                    this.Dispatcher.Invoke((SLMWorkerSetGUI)delegate { _vm.SLMBleachWaveParams.Add(SLMParamsCurrent); });

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

                //end for loop of ndList,
                //clear-then-return if break due to error
                this.Dispatcher.Invoke((SLMWorkerSetGUI)delegate
                {
                    if (workStatus)
                    {
                        OverlayManagerClass.Instance.PersistSaveROIs();
                    }
                    else
                    {
                        _vm.SLMBleachWaveParams.Clear();
                        OverlayManagerClass.Instance.DeletePatternROI(ref CaptureSetupViewModel.OverlayCanvas, -1, OverlayManagerClass.Instance.CurrentMode);

                        for (int id = 0; id < _vm.SLMWaveformFolder.Length; id++)
                            _vm.ClearSLMFiles(_vm.SLMWaveformFolder[id]);
                    }
                });

                //give back properties
                OverlayManagerClass.Instance.CurrentMode = backupMode;
                _vm.SLMSelectWavelengthProp = backupWavelength;
                OverlayManagerClass.Instance.SimulatorMode = false;      //give back overlay manager access to hardware mvms

                if (!workStatus) return false;
            }

            if (includeSequences)
            {
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
            }
            return true;
        }

        /// <summary>
        /// Import waveform generation parameters from experiment only
        /// </summary>
        /// <param name="xmlSLMImportDoc"></param>
        /// <returns></returns>
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

                sparam.BleachWaveParams.UMPerPixel = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "pixelSizeUM", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                   dVal1 : (double)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (object)1.0];

                sparam.BleachWaveParams.UMPerPixelRatio = (0 < _vm.BleachLSMUMPerPixel) ? sparam.BleachWaveParams.UMPerPixel / _vm.BleachLSMUMPerPixel : 1.0;

                sparam.BleachWaveParams.CenterUM = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "xOffsetUM", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1) &&
                    XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "yOffsetUM", ref str2) && Double.TryParse(str2, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal2)) ?
                    new Point(dVal1, dVal2) : new Point(0, 0);

                if (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "roiWidthUM", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))
                {
                    sparam.BleachWaveParams.ROIWidthUM = dVal1;
                }
                else
                {
                    sparam.BleachWaveParams.ROIWidthUM = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "roiWidthPx", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                        dVal1 * sparam.BleachWaveParams.UMPerPixel : 0.0;
                }

                if (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "roiHeightUM", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))
                {
                    sparam.BleachWaveParams.ROIHeightUM = dVal1;
                }
                else
                {
                    sparam.BleachWaveParams.ROIHeightUM = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "roiHeightPx", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                        dVal1 * sparam.BleachWaveParams.UMPerPixel : 0.0;
                }

                sparam.PixelSpacing = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "pxSpacing", ref str1) && Int32.TryParse(str1, out iVal)) ?
                    Math.Max(1, iVal) : 1;

                sparam.BleachWaveParams.Iterations = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "iterations", ref str1) && Int32.TryParse(str1, out iVal)) ?
                    Math.Max(1, iVal) : 1;

                sparam.BleachWaveParams.PrePatIdleTime = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "prePatIdleMS", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                    Math.Max(0, dVal1) : 0.0;

                sparam.BleachWaveParams.PostPatIdleTime = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "postPatIdleMS", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                    Math.Max(0, dVal1) : 0.0;

                sparam.BleachWaveParams.PreIdleTime = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "preIteIdleMS", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                    Math.Max(0, dVal1) : 0.0;

                sparam.BleachWaveParams.PostIdleTime = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "postIteIdleMS", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                    Math.Max(0, dVal1) : 0.0;

                sparam.BleachWaveParams.Power = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "power", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                    Math.Max(0, dVal1) : 0.0;
                sparam.BleachWaveParams.Power1 = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "power1", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal2)) ?
                    dVal2 : -1.0;

                sparam.BleachWaveParams.MeasurePower = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "measurePowerMW", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                    Math.Max(0, dVal1) : 0.0;
                sparam.BleachWaveParams.MeasurePower1 = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "measurePower1MW", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal2)) ?
                    dVal2 : -1.0;

                sparam.SLMMeasurePowerArea = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "measurePowerMWPerUM2", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ? dVal1 : 0.0;
                sparam.SLMMeasurePowerArea1 = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "measurePower1MWPerUM2", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal2)) ?
                    dVal2 : -1.0;

                sparam.Red = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "red", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ? dVal1 : 128;

                sparam.Green = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "green", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ? dVal1 : 128;

                sparam.Blue = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "blue", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ? dVal1 : 128;

                sparam.Duration = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "durationMS", ref str1) && Double.TryParse(str1, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)) ?
                    Math.Max(0, dVal1) : 0.0;

                sparam.BleachWaveParams.ID = (XmlManager.GetAttribute(ndList[i], xmlSLMImportDoc, "patternID", ref str1) && Int32.TryParse(str1, out iVal)) ?
                    (uint)iVal : (uint)(fID + i + 1);

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

        private void UpdatePropProc(string note)
        {
            switch (note.ToUpper(new CultureInfo("en-US", false)))
            {
                case "POWER":
                    UpdateSLMParamPower();
                    break;
            }
        }

        private void UpdateProp_Click(object sender, RoutedEventArgs e)
        {
            UpdatePropProc((null != (sender as FrameworkElement)) ? (sender as FrameworkElement).Tag as string : (string)(sender));
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
                this.Dispatcher.Invoke((SLMWorkerSetGUI)delegate
                {
                    //clear ROIs:
                    OverlayManagerClass.Instance.ClearNonSaveROIs(ref CaptureSetupViewModel.OverlayCanvas);
                });
            }

            _waveParamsUpdated = true;
        }

        private bool ValidateWaveformBeforeBuild()
        {
            bool ret = true;
            //check waveform:
            int stepCount = GetDwellCount(_vm.IsStimulator ? _vm.BleachInternalClockRate : (int)(_vm.BleachLSMPixelXY[0] * WaveformBuilder.MS_TO_S / SLMParamsCurrent.PixelSpacing));  //[Hz]
            if (0 == stepCount)
            {
                SLMPatternStatus = String.Format("Invalid Galvo calibration.\n");
                return false;
            }
            //Verify duration:
            SLMParamsCurrent.BleachWaveParams.DwellTime = SLMParamsCurrent.Duration * WaveformBuilder.MS_TO_S / (_vm.IsStimulator ? 1 : stepCount);  //Duration [ms], DwellTime [us]
            if (WaveformBuilder.MinDwellTime > SLMParamsCurrent.BleachWaveParams.DwellTime)
            {
                SLMPatternStatus = String.Format("Minimum duration is {0} ms,\nwhile pixel spacing = {1}, width = {2} um, height = {3} um.\n",
                Decimal.Round((Decimal)(WaveformBuilder.MinDwellTime * stepCount / WaveformBuilder.MS_TO_S), 3), SLMParamsCurrent.PixelSpacing,
                SLMParamsCurrent.BleachWaveParams.ROIWidthUM, SLMParamsCurrent.BleachWaveParams.ROIHeightUM);

                SLMPatternStatus += String.Format("SLM Generation: FAILED due to invalid dwell time.\n");
                ret = false;
            }
            else if (PatternMinMS > SLMParamsCurrent.Duration * SLMParamsCurrent.BleachWaveParams.Iterations)
            {
                SLMPatternStatus = String.Format("Minimum total duration is {0} ms,\nwhile duration per iteration = {1}, multiply by {2} of iterations.\n",
                   PatternMinMS, SLMParamsCurrent.Duration, SLMParamsCurrent.BleachWaveParams.Iterations);
                ret = false;
            }
            else if (WaveformBuilder.GetWaveform().Count > Int32.MaxValue)
            {
                SLMPatternStatus = String.Format("SLM Generation: FAILED due to limited memory space.\n");
                ret = false;
            }
            //Verify power with wavelength ROIs [WIDEFIELD ONLY]:
            //zero power if no ROIs to avoid unwanted wavelength
            if (_vm.IsStimulator)
            {
                bool backupWavelength = _vm.SLMSelectWavelengthProp;
                this.Dispatcher.Invoke((SLMSavePattern)delegate
                {
                    for (int i = 0; i < _vm.SLMWavelengthCount; i++)
                    {
                        _vm.SLMSelectWavelengthProp = (1 == i);
                        List<Shape> rois = OverlayManagerClass.Instance.GetModeROIs(Mode.PATTERN_WIDEFIELD, OverlayManagerClass.Instance.PatternID, _vm.SLMWavelengthNM);
                        if (0 == rois.Count)
                            SLMParamsCurrent.BleachWaveParams.GetType().GetProperty(0 < i ? "Power" + i : "Power").SetValue(SLMParamsCurrent.BleachWaveParams, 0.0);
                    }
                });
                _vm.SLMSelectWavelengthProp = backupWavelength;
            }
            return ret;
        }

        #endregion Methods
    }

    public class SLMPreviewPanel
    {
        #region Constructors

        public SLMPreviewPanel(string preview, string cancel)
        {
            Preview = preview; Cancel = cancel;
        }

        #endregion Constructors

        #region Properties

        public string Cancel
        {
            get;
            set;
        }

        public string Preview
        {
            get;
            set;
        }

        #endregion Properties
    }
}