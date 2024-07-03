namespace RunSampleLSDll.View
{
    using System;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Interop;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using Infragistics.Controls.Editors;

    using RunSampleLSDll.ViewModel;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for RunSampleLSView.xaml
    /// </summary>
    public partial class RunSampleLSView : UserControl
    {
        #region Fields

        public bool[] channelEnable = new bool[4];

        private double _lastRemovedThumb = -1;
        private int _numAvailableCameras = 0;
        private RunSampleLSViewModel _viewModel;

        #endregion Fields

        #region Constructors

        public RunSampleLSView()
        {
            InitializeComponent();

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");

            //    FocusManager.SetFocusedElement(this, lstWavelength);

            this.KeyDown += RunSampleLSView_KeyDown;
            this.Loaded += new System.Windows.RoutedEventHandler(RunSampleLSView_Loaded);
            this.Unloaded += new RoutedEventHandler(RunSampleLSView_Unloaded);
        }

        #endregion Constructors

        #region Methods

        public void RunSampleLSView_KeyDown(object sender, KeyEventArgs e)
        {
            //filter for the enter or return key:
            if ((e.Key == Key.Enter) || (e.Key == Key.Return))
            {
                e.Handled = true;
                TraversalRequest trNext = new TraversalRequest(FocusNavigationDirection.Next);

                UIElement keyFocus = (UIElement)Keyboard.FocusedElement;

                //move the focus to the next element:
                if ((keyFocus != null) && (keyFocus.GetType() == typeof(TextBox)))
                {
                    keyFocus.MoveFocus(trNext);
                }
            }
        }

        private void AddThumb(double offset)
        {
            if (null != _viewModel && false == _viewModel.RemoteFocusCustomSelectedPlanes.Contains(offset))
            {
                // Define New Thumb and Add to xamSlider's Thumbs Collection
                XamSliderNumericThumb thumb = new XamSliderNumericThumb();
                thumb.GotFocus += new RoutedEventHandler(thumb_GotFocus);

                // Set the Thumb Style
                thumb.Style = remoteFocusGrid.Resources["ThumbStyle"] as Style;

                thumb.InteractionMode = SliderThumbInteractionMode.Lock;

                _viewModel.RemoteFocusCustomSelectedPlanes.Add(offset);
                _viewModel.UpdateNumberOfPlanes();

                // Add to the Slider's Thumbs Collection
                remoteFocusSlider.Thumbs.Add(thumb);

                // Set the Thumb to be active
                remoteFocusSlider.ActiveThumb = thumb;

                // Define Gradient Stop and add to linearBrush's GradientStops Collection
                GradientStop gs = new GradientStop();
                gs.Offset = offset;

                // Define Offset Binding and Apply to Thumb
                Binding binding = new Binding() { Source = gs, Path = new PropertyPath("Offset"), Mode = BindingMode.TwoWay };
                thumb.SetBinding(XamSliderThumb<double>.ValueProperty, binding);
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            Microsoft.Win32.SaveFileDialog dlg = new Microsoft.Win32.SaveFileDialog();
            dlg.Title = "Select a destination folder for the experiment";
            dlg.InitialDirectory = vm.OutputPath;
            dlg.FileName = "select.folder";
            dlg.Filter = "All Files (*.*)|*.*";
            try
            {
                if (false == dlg.ShowDialog())
                {
                    return;
                }
                else
                {
                    vm.OutputPath = System.IO.Path.GetDirectoryName(dlg.FileName);
                }
            }
            catch
            {
                MessageBox.Show("Hard drive doesn't exist. Please specify proper output path.");
                Microsoft.Win32.SaveFileDialog defaultdlg = new Microsoft.Win32.SaveFileDialog();
                defaultdlg.Title = "Select a destination folder for the experiment";
                string myDocumentsPath = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
                defaultdlg.InitialDirectory = myDocumentsPath;
                defaultdlg.FileName = "select.folder";
                defaultdlg.Filter = "All Files (*.*)|*.*";
                if (true == defaultdlg.ShowDialog())
                {
                    vm.OutputPath = System.IO.Path.GetDirectoryName(defaultdlg.FileName);
                }
            }
        }

        /// <summary>
        /// Prevent from user changing CaptureMode by key Up or Down.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbCaptureMode_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            e.Handled = true;
        }

        /// <summary>
        /// Prevent from user changing CaptureMode by mouse wheel.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cbCaptureMode_PreviewMouseWheel(object sender, MouseWheelEventArgs e)
        {
            e.Handled = true;
        }

        private void cbCaptureMode_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if ((cbCaptureMode != null) && (_viewModel != null))
            {
                groupPanel.Children.Remove(streamGroup);
                groupPanel.Children.Remove(zStackGroup);
                groupPanel.Children.Remove(timeGroup);
                groupPanel.Children.Remove(bleachingGroup);
                groupPanel.Children.Remove(hyperspectralGroup);
                switch (cbCaptureMode.SelectedIndex)
                {
                    case 0:
                        {
                            groupPanel.Children.Insert(0, zStackGroup);
                            groupPanel.Children.Insert(1, timeGroup);

                        }
                        break;
                    case 1:
                        {
                            groupPanel.Children.Insert(0, streamGroup);
                        }
                        break;
                    case 3:
                        {
                            groupPanel.Children.Insert(0, bleachingGroup);
                        }
                        break;
                    case 4:
                        {
                            groupPanel.Children.Insert(0, hyperspectralGroup);
                        }
                        break;
                }

                SetImageUpdaterVisibility(cbCaptureMode.SelectedIndex);

            }
        }

        private void cbCustomRF_Checked(object sender, RoutedEventArgs e)
        {
            ClearCustomSelectionThumbs();
        }

        private void cbCustomRF_Unchecked(object sender, RoutedEventArgs e)
        {
            if (null != _viewModel)
            {
                double dVal = 0;
                string[] valuesArray = _viewModel.RemoteFocusCustomOrder.Split(':');

                foreach (string value in valuesArray)
                {
                    if (Double.TryParse(value, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal) && dVal <= _viewModel.ZMax && dVal >= _viewModel.ZMin)
                    {
                        AddThumb(dVal);
                    }
                }
            }
        }

        private void cbSimultaneousBleachingAndImaging_Checked(object sender, RoutedEventArgs e)
        {
            //if the trigger mode is set to SWtrigger switch it to HWTriggerFirst
            //when selecting the simultaneous acquisition and bleaching option
            if (0 == _viewModel.BleachTrigger)
            {
                _viewModel.BleachTrigger = 1;
            }
        }

        private void cbStorageModes_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (cbStorageModes.SelectedIndex.Equals(0))     //Finite
            {
                chbExtStimulusTrig.IsChecked = false;
                chbExtStimulusTrig.GetBindingExpression(CheckBox.IsCheckedProperty).UpdateSource();
                if (cbCaptureMode.SelectedIndex == 1)
                {
                    try
                    {
                        _viewModel.StreamFrames = Convert.ToInt32(tbStreamFrames.Text);
                    }
                    catch { }
                }

            }
            else if (cbStorageModes.SelectedIndex.Equals(1)) //Stimulus
            {
                chbExtStimulusTrig.IsChecked = true;
                chbExtStimulusTrig.GetBindingExpression(CheckBox.IsCheckedProperty).UpdateSource();
                cbStreamTriggerModes.SelectedIndex = 1;
                cbStreamTriggerModes.GetBindingExpression(ComboBox.SelectedIndexProperty).UpdateSource();
            }
        }

        private void ClearCustomSelectionThumbs()
        {
            if (null != _viewModel)
            {
                ObservableCollection<XamSliderNumericThumb> thumbsToDelete = new ObservableCollection<XamSliderNumericThumb>();
                foreach (XamSliderNumericThumb thumb in remoteFocusSlider.Thumbs)
                {
                    if ("rfThumbStart" != thumb.Name && "rfThumbStop" != thumb.Name)
                    {
                        thumbsToDelete.Add(thumb);
                    }
                }

                foreach (var item in thumbsToDelete)
                {
                    remoteFocusSlider.Thumbs.Remove(item);
                }
                _viewModel.RemoteFocusCustomSelectedPlanes.Clear();
                _viewModel.UpdateNumberOfPlanes();
            }
        }

        private void GetMCLSChannelInfo(XmlDocument expDoc, XmlNodeList nodeList, string enableString, string powerString, ref Visibility vis, ref double power)
        {
            string strTemp = string.Empty;

            if (XmlManager.GetAttribute(nodeList[0], expDoc, enableString, ref strTemp))
            {
                if (strTemp.Equals("1"))
                {
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, powerString, ref strTemp))
                    {
                        vis = Visibility.Visible;
                        power = Convert.ToDouble(strTemp);
                    }
                }
                else
                {
                    vis = Visibility.Collapsed;
                    power = 0;
                }
            }
        }

        private void OpenNewReviewer_Click(object sender, RoutedEventArgs e)
        {
            // Use ProcessStartInfo class
            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.CreateNoWindow = false;
            startInfo.UseShellExecute = true;
            startInfo.FileName = ".\\ExperimentReview.exe";
            try
            {
                Process.Start(startInfo);
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        private void RemoteFocusCB_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ComboBox comboBox = sender as ComboBox;
            if (0 == comboBox.SelectedIndex)
            {
                ClearCustomSelectionThumbs();
            }
        }

        private void remoteFocusSlider_TrackClick(object sender, Infragistics.Controls.Editors.TrackClickEventArgs<double> e)
        {
            double val = Math.Round(e.Value);
            bool checkBox = (bool)cbCustomRF.IsChecked;
            if (_lastRemovedThumb != val && !checkBox && cbCustomRF.IsVisible)
            {
                AddThumb(val);
            }
            _lastRemovedThumb = -1;
        }

        private void RunSampleLSView_Loaded(object sender, System.Windows.RoutedEventArgs e)
        {
            try
            {
                if (System.Environment.OSVersion.Version.Major <= 5)
                {
                    //force the control into software rendering mode
                    //there is a memory leak in the .net 3.51 version
                    //*TODO* remove when new frame is used and memory leak is resolved

                    HwndSource hwndSource = PresentationSource.FromVisual(this) as HwndSource;

                    if (null != hwndSource)
                    {
                        HwndTarget hwndTarget = hwndSource.CompositionTarget;

                        // this is the new WPF API to force render mode.

                        hwndTarget.RenderMode = RenderMode.SoftwareOnly;
                    }
                }

                if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this))
                {
                    return;
                }
                //tilesUserControl.ControlType = (int)TilesDisplay.TilingControlType.LABEL;

                RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
                _viewModel = vm;

                vm.EnableHandlers();

                string str = string.Empty;
                int iVal = 0; uint uiVal = 0; double dVal = 0, dVal1 = 0;
                XmlNodeList nodeList;
                string expPath = ResourceManagerCS.GetCaptureTemplatePathString() + "\\Active.xml";

                //Load active.xml accounting for possible lockup during scripting
                XmlDocument expDoc = new XmlDocument();
                if (!RunSampleLS.TryLoadDocument(expDoc, expPath))
                {
                    MessageBox.Show("SCRIPT ERROR");
                    return;
                }

                vm.ResetImage();

                //active modality may have been changed, load settings
                MVMManager.Instance.LoadSettings();

                vm.HardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

                nodeList = vm.HardwareDoc.SelectNodes("/HardwareSettings/ImageDetectors/*");
                _numAvailableCameras = nodeList.Count;

                //if a camera or LSM is not in the list of available cameras
                //disable the window
                mainPanel.IsEnabled = (_numAvailableCameras > 0) ? true : false;

                double fieldSizeCal = 100.0;
                if (vm.GetLSMFieldSizeCalibration(ref fieldSizeCal))
                {
                    vm.LSMFieldSizeCalibration = fieldSizeCal;
                    ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " GetLSMFieldSizeCalibration returns TRUE, FieldSizeCalibration = " + fieldSizeCal.ToString());
                }
                else
                {
                    nodeList = vm.HardwareDoc.SelectNodes("/HardwareSettings/LSM");
                    if (nodeList.Count > 0)
                    {
                        if (XmlManager.GetAttribute(nodeList[0], vm.HardwareDoc, "fieldSizeCalibration", ref str))
                        {
                            vm.LSMFieldSizeCalibration = Convert.ToDouble(str, CultureInfo.InvariantCulture);
                        }
                    }
                }

                // check if the streaming path specified in hardwareSettings exists
                // use user's temp folder if not exists already
                nodeList = vm.HardwareDoc.SelectNodes("/HardwareSettings/Streaming");
                if (nodeList.Count > 0)
                {
                    string streamingPath = nodeList[0].Attributes["path"].Value;
                    if (Directory.Exists(streamingPath))
                    {
                        vm.StreamingPath = streamingPath;
                    }
                    else
                    {
                        vm.StreamingPath = System.Environment.GetEnvironmentVariable("TEMP") + "\\Thorlabs";
                        if (!Directory.Exists(vm.StreamingPath))
                        {
                            Directory.CreateDirectory(vm.StreamingPath);
                        }
                        // update the hardwareSettings doc
                        nodeList[0].Attributes["path"].Value = vm.StreamingPath;
                        MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS, true);
                    }
                }

                nodeList = vm.HardwareDoc.SelectNodes("/HardwareSettings/PowerControllers/PowerControl");

                if (nodeList.Count > 0)
                {
                    for (int i = 0; i < vm.MAX_POWER_CONTROLS; i++)
                    {

                        if (XmlManager.GetAttribute(nodeList[i], vm.HardwareDoc, "name", ref str))
                        {
                            vm.PowerControlNames[i].Value = str;
                        }
                    }
                }

                //Save the modality used going into RunSample. In case a modality change happens, is our only way to know.
                vm.InitialModality = ResourceManagerCS.Instance.ActiveModality;

                if (0 == vm.StartAfterLoading.Count)
                {
                    //only update path and experiment name while not in script mode
                    nodeList = expDoc.SelectNodes("/ThorImageExperiment/Name");

                    if (nodeList.Count > 0)
                    {
                        if (XmlManager.GetAttribute(nodeList[0], expDoc, "path", ref str) && !string.IsNullOrEmpty(str))
                        {
                            vm.OutputPath = str.Remove(str.LastIndexOf('\\'));
                        }
                        if (XmlManager.GetAttribute(nodeList[0], expDoc, "name", ref str) && !string.IsNullOrEmpty(str))
                        {
                            vm.ExperimentName = Regex.Replace(str, vm.ExperimentNameNumberPattern, "");
                        }
                    }
                }

                nodeList = expDoc.SelectNodes("/ThorImageExperiment/CaptureMode");

                if (nodeList.Count > 0)
                {
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "mode", ref str))
                    {
                        // set twice to force CaptureMode combobox selectionchanged event to fire
                        vm.CaptureMode = 1;
                        vm.CaptureMode = Convert.ToInt32(str);
                    }
                }

                nodeList = expDoc.SelectNodes("/ThorImageExperiment/ZStage");

                if (nodeList.Count > 0)
                {
                    vm.ZStartPosition = Convert.ToDouble(nodeList[0].Attributes["startPos"].Value, CultureInfo.InvariantCulture);
                    int steps = Math.Max(1, Convert.ToInt32(nodeList[0].Attributes["steps"].Value));
                    double stepSize = Convert.ToDouble(nodeList[0].Attributes["stepSizeUM"].Value, CultureInfo.InvariantCulture);
                    vm.ZStepSize = Math.Abs(stepSize);
                    vm.ZStopPosition = vm.ZStartPosition + stepSize / (double)Constants.UM_TO_MM * (steps - 1);

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "enable", ref str))
                    {
                        int val = Convert.ToInt32(str);

                        if (1 == val)
                        {
                            vm.ZEnable = 1;
                        }
                        else
                        {
                            vm.ZEnable = 0;
                        }
                    }
                    else
                    {
                        //if the attribute in the tag does not exist use the
                        //legacy logic for determining the ZEnable state
                        if (1 == steps)
                        {
                            vm.ZEnable = 0;
                        }
                        else
                        {
                            vm.ZEnable = 1;
                        }
                    }

                    vm.ZStreamEnable = (Convert.ToInt32(nodeList[0].Attributes["zStreamMode"].Value) == 0) ? false : true;
                    vm.ZStreamFrames = Convert.ToInt32(nodeList[0].Attributes["zStreamFrames"].Value);
                    vm.CurrentZCount = 0;
                    vm.ZPosition = vm.ZStartPosition;

                    // diregard what's in the file, always set the file read checkbox to off
                    vm.ZFileEnable = 0;

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "zFilePosScale", ref str))
                    {
                        vm.ZFilePosScale = Convert.ToDouble(nodeList[0].Attributes["zFilePosScale"].Value, CultureInfo.InvariantCulture);
                    }
                    else
                    {
                        vm.ZFilePosScale = 1.0;
                    };
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "z2StageLock", ref str))
                    {
                        vm.Z2StageLock = (Convert.ToInt32(nodeList[0].Attributes["z2StageLock"].Value) == 0) ? false : true;
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "z2StageMirror", ref str))
                    {
                        vm.Z2StageMirror = (Convert.ToInt32(nodeList[0].Attributes["z2StageMirror"].Value) == 0) ? false : true;
                    }
                }

                nodeList = expDoc.SelectNodes("/ThorImageExperiment/Magnification");

                if (nodeList.Count > 0)
                {
                    vm.TurretMagnification = Convert.ToDouble(nodeList[0].Attributes["mag"].Value, CultureInfo.InvariantCulture);
                }

                nodeList = expDoc.SelectNodes("/ThorImageExperiment/Sample");

                if (nodeList.Count > 0)
                {
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "homeOffsetX", ref str))
                    {
                        vm.XYHomeOffsetX = Convert.ToDouble(nodeList[0].Attributes["homeOffsetX"].Value);
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "homeOffsetY", ref str))
                    {
                        vm.XYHomeOffsetY = Convert.ToDouble(nodeList[0].Attributes["homeOffsetY"].Value);
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "centerToCenterX", ref str))
                    {
                        vm.CarrierCenterToCenterDistanceX = Convert.ToDouble(nodeList[0].Attributes["centerToCenterX"].Value);
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "centerToCenterY", ref str))
                    {
                        vm.CarrierCenterToCenterDistanceY = Convert.ToDouble(nodeList[0].Attributes["centerToCenterY"].Value);
                    }
                    vm.CarrierType = Convert.ToString(nodeList[0].Attributes["type"].Value);
                }
                vm.TileControl = tileControl;

                nodeList = expDoc.SelectNodes("/ThorImageExperiment/LSM");

                if (nodeList.Count > 0)
                {
                    vm.LSMChannel = Convert.ToInt32(nodeList[0].Attributes["channel"].Value);
                    vm.LSMPixelX = Convert.ToInt32(nodeList[0].Attributes["pixelX"].Value);
                    vm.LSMPixelY = Convert.ToInt32(nodeList[0].Attributes["pixelY"].Value);
                    vm.LSMFieldSize = Convert.ToInt32(nodeList[0].Attributes["fieldSize"].Value);
                    vm.LSMAverageMode = Convert.ToInt32(nodeList[0].Attributes["averageMode"].Value);
                    vm.LSMAverageNum = Convert.ToInt32(nodeList[0].Attributes["averageNum"].Value);
                    vm.LSMScanMode = Convert.ToInt32(nodeList[0].Attributes["scanMode"].Value);
                    vm.LSMAreaMode = Convert.ToInt32(nodeList[0].Attributes["areaMode"].Value);
            /*                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "pixelSizeUM", ref str))
                    {
                        vm.LSMUMPerPixel = Convert.ToDouble(str, CultureInfo.InvariantCulture);
                    }*/
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "pixelWidthUM", ref str))
                    {
                        vm.LSMUMPerPixel.PixelWidthUM = Convert.ToDouble(str, CultureInfo.InvariantCulture);
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "pixelHeightUM", ref str))
                    {
                        vm.LSMUMPerPixel.PixelHeightUM = Convert.ToDouble(str, CultureInfo.InvariantCulture);
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "timeBasedLineScan", ref str))
                    {
                        vm.TimeBasedLineScanVisibility = ("1" == str || Boolean.TrueString == str) ? Visibility.Visible : Visibility.Collapsed;
                        vm.TimeBasedLineScan = ("1" == str || Boolean.TrueString == str) ? 1 : 0;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "tbLineScanTimeMS", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal))
                    {
                        vm.TimeBasedLSTimeMS = dVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "ThreePhotonEnable", ref str))
                    {
                        vm.ThreePhotonEnable = str == "1";
                    }
                    else
                    {
                        vm.ThreePhotonEnable = false;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "NumberOfPlanes", ref str) && int.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out int nplanes))
                    {
                        vm.NumberOfPlanes = nplanes;
                    }
                    else
                    {
                        vm.NumberOfPlanes = 1;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "pixelAspectRatioYScale", ref str) && double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out double pixelAspectRatio))
                    {
                        vm.PixelAspectRatioYScale = pixelAspectRatio;
                    }
                    else
                    {
                        vm.PixelAspectRatioYScale = 1;
                    }
                }

                nodeList = expDoc.SelectNodes("/ThorImageExperiment/Camera");

                if (nodeList.Count > 0)
                {
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "width", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.CamImageWidth = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "height", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.CamImageHeight = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "pixelSizeUM", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal))
                    {
                        vm.CamPixelSizeUM = new PixelSizeUM(dVal, dVal);
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "binningX", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.BinX = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "binningY", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.BinY = iVal;
                    }
                }

                nodeList = expDoc.SelectNodes("/ThorImageExperiment/Timelapse");

                if (nodeList.Count > 0)
                {
                    vm.TFrames = Convert.ToInt32(nodeList[0].Attributes["timepoints"].Value);
                    vm.TInterval = Convert.ToDouble(nodeList[0].Attributes["intervalSec"].Value, CultureInfo.InvariantCulture);

                    if (1 == vm.TFrames)
                    {
                        vm.TEnable = false;
                    }
                    else
                    {
                        vm.TEnable = true;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "triggerMode", ref str))
                    {
                        vm.TriggerModeTimelapse = Convert.ToInt32(str);
                    }

                    vm.CurrentTCount = 0;
                }

                nodeList = expDoc.SelectNodes("/ThorImageExperiment/Streaming");

                if (nodeList.Count > 0)
                {
                    vm.StreamEnable = Convert.ToInt32(nodeList[0].Attributes["enable"].Value);
                    vm.StreamFrames = Convert.ToInt32(nodeList[0].Attributes["frames"].Value);

                    if (null != nodeList[0].Attributes.GetNamedItem("rawData"))
                    {
                        vm.RawDataCapture = Convert.ToInt32(nodeList[0].Attributes["rawData"].Value);
                    }

                    if (null != nodeList[0].Attributes.GetNamedItem("displayRollingAveragePreview"))
                    {
                        vm.StreamingDisplayRollingAveragePreview = Convert.ToInt32(nodeList[0].Attributes["displayRollingAveragePreview"].Value);
                    }

                    if (null != nodeList[0].Attributes.GetNamedItem("triggerMode"))
                    {
                        const int SOFTWARE_MULTIFRAME = 1;
                        const int HARDWARE_MULTIFRAME_TRIGGER_FIRST = 4;
                        const int HARDWARE_MULTIFRAME_TRIGGER_EACH = 5;
                        const int HARDWARE_MULTIFRAME_TRIGGER_EACH_BULB = 6;
                        switch (Convert.ToInt32(nodeList[0].Attributes["triggerMode"].Value))
                        {
                            case SOFTWARE_MULTIFRAME:
                                {
                                    vm.TriggerModeStreaming = 0;
                                }
                                break;
                            case HARDWARE_MULTIFRAME_TRIGGER_FIRST:
                                {
                                    vm.TriggerModeStreaming = 1;
                                }
                                break;
                            case HARDWARE_MULTIFRAME_TRIGGER_EACH:
                                {
                                    vm.TriggerModeStreaming = 2;
                                }
                                break;
                            case HARDWARE_MULTIFRAME_TRIGGER_EACH_BULB:
                                {
                                    vm.TriggerModeStreaming = 3;
                                }
                                break;
                        }
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "storageMode", ref str))
                    {
                        const int STREAM_MODE = 1;

                        if (STREAM_MODE == vm.CaptureMode)
                        {
                            vm.StreamStorageMode = Convert.ToInt32(str);
                        }
                        else
                        {
                            //force the storage mode to finite so that the gui updates
                            //the save button properly. There may be a better way to do this later
                            //However the visibility bindinng currently only takes one property
                            const int FINITE_STORAGE = 0;
                            vm.StreamStorageMode = FINITE_STORAGE;
                        }

                        if (vm.IsCamera())  // LSM/Camera
                        {
                            vm.DMAFramesVisibility = 1; // collapse
                        }
                        else
                        {
                            vm.DMAFramesVisibility = 1; // visible
                        }
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "zFastEnable", ref str))
                    {
                        vm.ZFastEnable = (0 == Convert.ToInt32(str)) ? false : true;
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "zFastMode", ref str))
                    {
                        vm.FastZStaircase = ((int)ZPiezoAnalogMode.ANALOG_MODE_STAIRCASE_WAVEFORM == Convert.ToInt32(str)) ? true : false;
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "flybackFrames", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.FlybackFrames = iVal;
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "flybackLines", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.FlybackLines = iVal;
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "flybackTimeAdjustMS", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal))
                    {
                        vm.FlybackTimeAdjustMS = dVal;
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "volumeTimeAdjustMS", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal))
                    {
                        vm.VolumeTimeAdjustMS = dVal;
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "stepTimeAdjustMS", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal))
                    {
                        vm.StepTimeAdjustMS = dVal;
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "previewIndex", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.PreviewIndex = iVal;
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "stimulusTriggering", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.StimulusTriggering = iVal;
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "dmaFrames", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.DMAFrames = iVal;
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "stimulusMaxFrames", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.StimulusMaxFrames = iVal;
                    }
                    if (0 != vm.ZNumSteps + vm.FlybackFrames)
                    {
                        vm.StreamVolumes = Convert.ToInt32(Math.Round(vm.StreamFrames / (double)(vm.ZNumSteps + vm.FlybackFrames)));
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "useReferenceVoltageForFastZPockels", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.UseReferenceVoltageForFastZPockels = iVal;
                    }
                }

                nodeList = expDoc.SelectNodes("/ThorImageExperiment/Photobleaching");

                if (nodeList.Count > 0)
                {

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "enable", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.PhotobleachEnable = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "laserPos", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.PhotobleachLaserPosition = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "durationMS", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                    {
                        //convert the millisecond value to seconds
                        vm.PhotobleachDurationSec = dVal / 1000.0;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "powerPos", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                    {
                        vm.PhotobleachPowerPosition = dVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "bleachTrigger", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.BleachTrigger = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "bleachPostTrigger", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.BleachPostTrigger = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "preBleachFrames", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.PreBleachFrames = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "preBleachInterval", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                    {
                        vm.PreBleachInterval = dVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "preBleachStream", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.PreBleachStream = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "bleachFrames", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.BleachFrames = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "postBleachFrames1", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.PostBleachFrames1 = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "postBleachInterval1", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                    {
                        vm.PostBleachInterval1 = dVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "postBleachStream1", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.PostBleachStream1 = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "postBleachFrames2", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.PostBleachFrames2 = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "postBleachInterval2", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                    {
                        vm.PostBleachInterval2 = dVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "postBleachStream2", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.PostBleachStream2 = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "rawOption", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.RawOption = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "EnableSimultaneous", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.SimultaneousBleachingAndImaging = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "powerShiftUS", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal))
                    {
                        vm.PowerShiftUS = dVal;
                    }

                    if (vm.PreBleachStream != 0 && vm.PostBleachStream1 != 0 && vm.PostBleachStream2 != 0)
                    {
                        vm.ShowRawOption = Visibility.Visible;
                    }
                    else
                    {
                        vm.ShowRawOption = Visibility.Hidden;
                        vm.RawOption = 0;
                    }
                }

                nodeList = expDoc.SelectNodes("/ThorImageExperiment/SpectralFilter");
                if (nodeList.Count > 0)
                {
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "wavelengthStart", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.SFStartWavelength = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "wavelengthStop", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.SFStopWavelength = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "wavelengthStepSize", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.SFWavelengthStepSize = iVal;
                    }

                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "steps", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.SFSteps = iVal;
                    }
                }

                //SLM:
                nodeList = expDoc.SelectNodes("/ThorImageExperiment/SLM");
                if (nodeList.Count > 0)
                {
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "cycleDelay", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                    {
                        vm.SLMBleachDelay = dVal;
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "advanceMode", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.SLMSequenceOn = (1 == iVal);
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "randomEpochs", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.SLMRandomEpochs = (1 == iVal);
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "holoGen3D", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.SLM3D = (1 == iVal);
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "sequenceEpochDelay", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                    {
                        vm.SLMSequenceEpochDelay = dVal;
                    }
                }
                nodeList = expDoc.SelectNodes("/ThorImageExperiment/SLM/Pattern");
                if (nodeList.Count > 0)
                {
                    vm.SLMBleachWaveParams.Clear();

                    for (int i = 0; i < nodeList.Count; i++)
                    {
                        GeometryUtilities.SLMParams sparam = new GeometryUtilities.SLMParams();
                        if (XmlManager.GetAttribute(nodeList[i], expDoc, "name", ref str))
                        {
                            sparam.Name = str;
                        }

                        sparam.PhaseType = (XmlManager.GetAttribute(nodeList[i], expDoc, "phaseType", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal)) ? iVal : 0;

                        if (XmlManager.GetAttribute(nodeList[i], expDoc, "roiWidthUM", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                        {
                            sparam.BleachWaveParams.ROIWidthUM = dVal;
                        }
                        if (XmlManager.GetAttribute(nodeList[i], expDoc, "roiHeightUM", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                        {
                            sparam.BleachWaveParams.ROIHeightUM = dVal;
                        }
                        if (XmlManager.GetAttribute(nodeList[0], expDoc, "pxSpacing", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                        {
                            sparam.PixelSpacing = iVal;
                        }
                        if (XmlManager.GetAttribute(nodeList[i], expDoc, "durationMS", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                        {
                            sparam.Duration = dVal;
                        }
                        if (XmlManager.GetAttribute(nodeList[0], expDoc, "iterations", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                        {
                            sparam.BleachWaveParams.Iterations = iVal;
                        }
                        if (XmlManager.GetAttribute(nodeList[i], expDoc, "preIteIdleMS", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                        {
                            sparam.BleachWaveParams.PreIdleTime = dVal;
                        }
                        if (XmlManager.GetAttribute(nodeList[i], expDoc, "postIteIdleMS", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                        {
                            sparam.BleachWaveParams.PostIdleTime = dVal;
                        }
                        if (XmlManager.GetAttribute(nodeList[i], expDoc, "power", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                        {
                            sparam.BleachWaveParams.Power = dVal;
                            sparam.BleachWaveParams.Power1 = (XmlManager.GetAttribute(nodeList[i], expDoc, "power1", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1))) ?
                                                            dVal1 : -1.0;
                        }
                        if (XmlManager.GetAttribute(nodeList[i], expDoc, "measurePowerMW", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                        {
                            sparam.BleachWaveParams.MeasurePower = dVal;
                            if (XmlManager.GetAttribute(nodeList[i], expDoc, "measurePower1MW", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal1)))
                                sparam.BleachWaveParams.MeasurePower1 = dVal1;
                        }
                        if (XmlManager.GetAttribute(nodeList[i], expDoc, "red", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                        {
                            sparam.Red = dVal;
                        }
                        if (XmlManager.GetAttribute(nodeList[i], expDoc, "green", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                        {
                            sparam.Green = dVal;
                        }
                        if (XmlManager.GetAttribute(nodeList[i], expDoc, "blue", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)))
                        {
                            sparam.Blue = dVal;
                        }
                        if (XmlManager.GetAttribute(nodeList[i], expDoc, "fileID", ref str) && UInt32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out uiVal))
                        {
                            sparam.BleachWaveParams.ID = uiVal;
                        }
                        vm.SLMBleachWaveParams.Add(sparam);
                    }
                }

                //SelectedChannels:
                nodeList = expDoc.SelectNodes("/ThorImageExperiment/Wavelengths/ChannelEnable");
                if (nodeList.Count > 0)
                {
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "Set", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.ChannelSelection = iVal;
                    }
                }

                nodeList = expDoc.SelectNodes("/ThorImageExperiment/RemoteFocus");

                if (nodeList.Count > 0 && vm.IsRemoteFocus)
                {
                    int steps = 1;
                    int stepSize = 1;
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "startPlane", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal))
                    {
                        vm.RemoteFocusStartPosition = dVal;
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "steps", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        steps = iVal;
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "stepSize", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        stepSize = iVal;
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "captureMode", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.RemoteFocusCaptureMode = iVal;
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "customSequence", ref str))
                    {
                        vm.RemoteFocusCustomOrder = str;
                    }
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "customSequenceEnabled", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.RemoteFocusCustomChecked = 1 == iVal;
                    }
                    vm.RemoteFocusStepSize = Math.Abs(stepSize);
                    vm.RemoteFocusStopPosition = vm.RemoteFocusStartPosition + stepSize * (steps - 1);

                    vm.ZStartPosition = vm.RemoteFocusStartPosition / (double)Constants.UM_TO_MM;
                    vm.ZStepSize = vm.RemoteFocusStepSize;
                    vm.ZStopPosition = vm.RemoteFocusStopPosition / (double)Constants.UM_TO_MM;

                    if ((int)RemoteFocusCaptureModes.Custom == vm.RemoteFocusCaptureMode && !vm.RemoteFocusCustomChecked)
                    {
                        string[] valuesArray = vm.RemoteFocusCustomOrder.Split(':');

                        foreach (string value in valuesArray)
                        {
                            if (Double.TryParse(value, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal) && dVal <= vm.ZMax && dVal >= vm.ZMin)
                            {
                                AddThumb(dVal);
                            }
                        }
                    }
                }
                vm.LoadOverlayManagerSettings();
                vm.LoadMROISettings(expDoc);

                //load selected MVMs
                MVMManager.Instance.LoadMVMSettings(vm.MVMNames);

                //hide digital switch trigger button
                MVMManager.Instance["DigitalOutputSwitchesViewModel", "ExperimentMode"] = 1;

                SetDisplayOptions(expDoc);

                vm.LoadRemoteFocusPositionValues();

                vm.UpdateUIBindings();

                Keyboard.Focus(this.btnStart);

                SetNormalization(vm);

                vm.ConnectHandlers();

                vm.RunSampleLSView_UpdateStageLocation();

                if (vm.StartAfterLoading.Count > 0)
                {
                    //Update the global magnification value to keep track of the mover's position, in case user closes ThorImage
                    MVMManager.Instance.UpdateMVMXMLSettings(ref MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS], new string[] { "ObjectiveControlViewModel" });
                    vm.RunSampleLSStartCommand.Execute(null);
                }

                if (vm.CaptureSetupStartAfterLoading.Count > 0)
                {
                    vm.RunSampleLSStartCommand.Execute(null);
                    vm.CaptureSetupStartAfterLoading.RemoveAt(0);
                }

            }
            catch (NullReferenceException ex)
            {
                string str = ex.Message;
            }
            catch (FileNotFoundException ex)
            {
                MessageBox.Show("Failed to load Active.xml file." + ex.Message.ToString());
            }

            int channel = 0;
            for (int i = 0; i < _viewModel.RunSampleLS.GetMaxChannels(); i++)
            {

                int val = _viewModel.ChannelSelection;

                if (((val >> i) & 0x1) > 0)
                {
                    channel++;
                    channelEnable[i] = true;
                }
                else
                {
                    channelEnable[i] = false;
                }
            }

            _viewModel.LSMChannelEnable0 = channelEnable[0];
            _viewModel.LSMChannelEnable1 = channelEnable[1];
            _viewModel.LSMChannelEnable2 = channelEnable[2];
            _viewModel.LSMChannelEnable3 = channelEnable[3];

            //load all MVMs:
            MVMManager.Instance.LoadMVMCaptureSettings();
        }

        void RunSampleLSView_Unloaded(object sender, RoutedEventArgs e)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;

            // make sure z file read button is off
            vm.ZFileEnable = 0;

            // -do not update experiment file if no camera available because many values
            //  in that case are zeros
            // -do not update experiment/hardwaresettings file if a StartAfterLoading has occured.
            //  The active.xml has already been updated by the script and should not be overwritten
            //  with the unloaded
            // -When the StartAfterLoading container is empty the user is manually changing tabs. The
            //  files can be saved at that time.
            // -do not update the experiment/hardwaresettings file if a modality change has occured
            //  This is important, InitialModality is the only way to know the modality combobox is being used
            //  after switching modalities, MVMUpdate will be called across all MVMs used in RunSample
            //  The function below would called them again, but at this point the modality active has already been replaced
            if ((_numAvailableCameras > 0) && (ThorSharedTypes.ResourceManagerCS.Instance.ActiveModality.Equals(vm.InitialModality)) && (0 == vm.StartAfterLoading.Count))
            {
                vm.UpdateExperimentFile();
                SaveNormalization(vm);
            }

            //:TODO: Need to figure out which MVMs need to be updated in RunSample, for now we will just update RemoteIPCControlViewModelBase because we are required to.
            // also MVMNames is empty at this point because it gets set to an empty string array at MainWindow unloaded
            string[] mvmsToUpdate = { "RemoteIPCControlViewModelBase" };
            MVMManager.Instance.UpdateMVMXMLSettings(ref MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS], mvmsToUpdate);

            //Decrement the script start container when the unload occurs
            if (vm.StartAfterLoading.Count > 0)
            {
                vm.StartAfterLoading.RemoveAt(0);
            }

            MVMManager.Instance.LoadSettings();
            vm.ReleaseHandlers();
        }

        private void SaveNormalization(RunSampleLSViewModel vm)
        {
            //TODO:IV

            //XmlNodeList ndListHW = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS].SelectNodes("/HardwareSettings/Wavelength");

            //if (ndListHW.Count > 0)
            //{
            //    for (int i = 0; i < ndListHW.Count; i++)
            //    {
            //        double bp = 0;
            //        double wp = 255;
            //        string autoTog = string.Empty;
            //        string logTog = string.Empty;

            //        switch (i)
            //        {
            //            case 0:
            //                bp = vm.BlackPoint0;
            //                wp = vm.WhitePoint0;
            //                autoTog = (vm.AutoManualTog1Checked) ? "1" : "0";
            //                logTog = (vm.LogScaleEnabled0) ? "1" : "0";
            //                break;
            //            case 1:
            //                bp = vm.BlackPoint1;
            //                wp = vm.WhitePoint1;
            //                autoTog = (vm.AutoManualTog2Checked) ? "1" : "0";
            //                logTog = (vm.LogScaleEnabled1) ? "1" : "0";
            //                break;
            //            case 2:
            //                bp = vm.BlackPoint2;
            //                wp = vm.WhitePoint2;
            //                autoTog = (vm.AutoManualTog3Checked) ? "1" : "0";
            //                logTog = (vm.LogScaleEnabled2) ? "1" : "0";
            //                break;
            //            case 3:
            //                bp = vm.BlackPoint3;
            //                wp = vm.WhitePoint3;
            //                autoTog = (vm.AutoManualTog4Checked) ? "1" : "0";
            //                logTog = (vm.LogScaleEnabled3) ? "1" : "0";
            //                break;
            //        }
            //        XmlManager.SetAttribute(ndListHW[i], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "bp", bp.ToString());
            //        XmlManager.SetAttribute(ndListHW[i], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "wp", wp.ToString());
            //        XmlManager.SetAttribute(ndListHW[i], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "AutoSta", autoTog);
            //        XmlManager.SetAttribute(ndListHW[i], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "LogSta", logTog);
            //    }
            //    MVMManager.Instance.ReloadSettings(SettingsFileType.HARDWARE_SETTINGS, true);
            //}
        }

        private void SetDisplayOptions(XmlDocument expDoc)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;

            //if (vm.IsCamera())
            //{
            //    cameraUIPanel.Visibility = Visibility.Visible;
            //    groupPanel.Visibility = Visibility.Collapsed;
            //}
            //else
            {
                cameraUIPanel.Visibility = Visibility.Collapsed;
                groupPanel.Visibility = Visibility.Visible;
            }

            string str = string.Empty;
            int iVal = 0;
            XmlDocument appDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNodeList nodeList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/XYView");
            if (nodeList.Count > 0)
            {
                int tilesCount = 0;
                XmlNodeList ndList = expDoc.SelectNodes("/ThorImageExperiment/Sample/Wells");
                for (int i = 0; i < ndList.Count; i++)
                {
                    //Get all well positions
                    XmlNodeList innderNdList = ndList[i].SelectNodes("SubImages");
                    for (int j = 0; j < innderNdList.Count; j++)
                    {
                        XmlManager.GetAttribute(innderNdList[j], expDoc, "isEnabled", ref str);
                        if (str == Boolean.TrueString || str == "1")
                        {
                            str = string.Empty;
                            int subRows = 1, subColumns = 1;
                            if (XmlManager.GetAttribute(innderNdList[j], expDoc, "subRows", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                            {
                                subRows = iVal;
                            }
                            str = string.Empty;
                            if (XmlManager.GetAttribute(innderNdList[j], expDoc, "subColumns", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                            {
                                subColumns = iVal;
                            }
                            tilesCount += subRows * subColumns;
                        }
                    }
                }

                if (XmlManager.GetAttribute(nodeList[0], appDoc, "Visibility", ref str))
                {
                    //Don't display tile area when there are no defined tiles
                    if (str.Equals("Visible") && tilesCount > 0)
                    {
                        xyBorder.Visibility = Visibility.Visible;
                    }
                    else
                    {
                        xyBorder.Visibility = Visibility.Collapsed;
                    }
                }
            }

            //depending on values in ApplicationSettings.xml, TDI combobox item would be added or removed, accordingly
            str = string.Empty;

            vm.TDIOptionsVisibility = Visibility.Collapsed;

            //hide Z stack group box according to ApplicationSettings.xml, visibility not changed if no attributes found
            //display string for Z and T is also changed according to visibility of Z stack group
            nodeList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ZView");
            if (nodeList.Count > 0)
            {
                if (XmlManager.GetAttribute(nodeList[0], appDoc, "Visibility", ref str) && (str.Equals("Visible")))
                {
                    cbiZAndT.Content = "Z and T Series";
                    vm.ZOptionsVisibility = Visibility.Visible;
                }
                else
                {
                    //disable z stack capture and hide z panel in Capture tab
                    vm.ZStopPosition = vm.ZStartPosition;
                    vm.ZEnable = 0;
                    vm.ZOptionsVisibility = Visibility.Collapsed;

                    cbiZAndT.Content = "Time Series";
                }

                if (XmlManager.GetAttribute(nodeList[0], appDoc, "InvertLimitsZ1", ref str) && Int32.TryParse(str, out iVal))
                {
                    vm.ZInvertLimits = 1 == iVal;
                }

                if (XmlManager.GetAttribute(nodeList[0], appDoc, "Invert", ref str) && Int32.TryParse(str, out iVal))
                {
                    vm.ZInvert = 1 == iVal;
                }
            }

            nodeList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/BleachView");

            if ((nodeList.Count > 0) && XmlManager.GetAttribute(nodeList[0], appDoc, "Visibility", ref str) && (str.Equals("Visible")))
            {
                vm.BleachOptionsVisibility = Visibility.Visible;
            }
            else //remove bleaching option by default
            {
                vm.BleachOptionsVisibility = Visibility.Collapsed;

                if (3 == cbCaptureMode.SelectedIndex && (0 == vm.StartAfterLoading.Count))
                {
                    cbCaptureMode.SelectedIndex = 0;
                }
            }

            nodeList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/DFLIMView");

            if ((nodeList.Count > 0) && XmlManager.GetAttribute(nodeList[0], appDoc, "Visibility", ref str) && (str.Equals("Visible")))
            {
                dflimBorder.Visibility = Visibility.Visible;
            }
            else
            {
                dflimBorder.Visibility = Visibility.Collapsed;
            }

            //Check if RGG and scan areas available to determine if mROI should show
            Visibility mROIVisibility = Visibility.Collapsed;
            nodeList = expDoc.SelectNodes("/ThorImageExperiment/LSM");
            if ((nodeList.Count > 0) && XmlManager.GetAttribute(nodeList[0], expDoc, "name", ref str) && str.Equals("ThorDAQ RGG"))
            {
                //RGG enabled, now check if scan areas
                try
                {
                    nodeList = expDoc.SelectNodes("/ThorImageExperiment/TemplateScans/ScanInfo");

                    XmlNodeList scanAreaNodes;
                    foreach (XmlNode node in nodeList)
                    {
                        if (XmlManager.GetAttribute(node, expDoc, "Name", ref str) && str.Equals("Micro"))
                        {
                            scanAreaNodes = node.SelectNodes("descendant::ScanAreas/ScanArea");
                            foreach (XmlNode scanAreaNode in scanAreaNodes)
                            {
                                if (XmlManager.GetAttribute(scanAreaNodes[0], expDoc, "IsEnable", ref str) && str.Equals("true"))
                                {
                                    mROIVisibility = Visibility.Visible;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
                catch (Exception)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 0, "ImageReview: Exception Thrown while checking mROI experiment doc");
                    mROIVisibility = Visibility.Collapsed;
                }
            }
            mROIDisplay.Visibility = mROIVisibility;
            if (mROIVisibility == Visibility.Visible)
            {
                //Only allow streaming in mROI mode for now
                cbCaptureMode.SelectedIndex = 1;
                ((ComboBoxItem)cbCaptureMode.Items[0]).Visibility = Visibility.Collapsed;

                //Only allow raw for now
                cbStreamingCaptureDataType.SelectedIndex = 1;
                ((ComboBoxItem)cbStreamingCaptureDataType.Items[0]).Visibility = Visibility.Collapsed;
                ((ComboBoxItem)cbStreamingCaptureDataType.Items[2]).Visibility = Visibility.Collapsed;
            }
            else
            {
                ((ComboBoxItem)cbCaptureMode.Items[0]).Visibility = Visibility.Visible;

                ((ComboBoxItem)cbStreamingCaptureDataType.Items[0]).Visibility = Visibility.Visible;
                ((ComboBoxItem)cbStreamingCaptureDataType.Items[2]).Visibility = Visibility.Visible;
            }

            bool kuriosActive = false;
            XmlNodeList hwNdList = vm.HardwareDoc.SelectNodes("/HardwareSettings/Devices/SpectrumFilter");
            if (hwNdList.Count > 0)
            {
                for (int i = 0; i < hwNdList.Count; i++)
                {
                    string strActive = string.Empty;
                    XmlManager.GetAttribute(hwNdList[i], vm.HardwareDoc, "active", ref strActive);

                    string strName = string.Empty;
                    XmlManager.GetAttribute(hwNdList[i], vm.HardwareDoc, "dllName", ref strName);

                    if ("1" == strActive && "ThorKurios" == strName)
                    {
                        kuriosActive = true;
                    }
                }
            }

            //Hyperspectral acquisition is only available when there is an active CCD camera and spectral filter
            if (kuriosActive && vm.IsCamera())
            {
                ((ComboBoxItem)cbCaptureMode.Items[4]).Visibility = Visibility.Visible;
            }
            else
            {
                ((ComboBoxItem)cbCaptureMode.Items[4]).Visibility = Visibility.Collapsed;
                if (4 == cbCaptureMode.SelectedIndex && (0 == vm.StartAfterLoading.Count))
                {
                    cbCaptureMode.SelectedIndex = 0;
                }
            }

            //Mirror power position visibility from capture setup settings
            nodeList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/Pockels1");
            vm.Power0Visibility = ((nodeList.Count > 0) && XmlManager.GetAttribute(nodeList[0], appDoc, "Visibility", ref str) && (str.Equals("Visible"))) ?
                Visibility.Visible : Visibility.Collapsed;

            nodeList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/Pockels2");
            vm.Power1Visibility = ((nodeList.Count > 0) && XmlManager.GetAttribute(nodeList[0], appDoc, "Visibility", ref str) && (str.Equals("Visible"))) ?
                Visibility.Visible : Visibility.Collapsed;

            nodeList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/Pockels3");
            vm.Power2Visibility = ((nodeList.Count > 0) && XmlManager.GetAttribute(nodeList[0], appDoc, "Visibility", ref str) && (str.Equals("Visible"))) ?
                Visibility.Visible : Visibility.Collapsed;

            nodeList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/Pockels4");
            vm.Power3Visibility = ((nodeList.Count > 0) && XmlManager.GetAttribute(nodeList[0], appDoc, "Visibility", ref str) && (str.Equals("Visible"))) ?
                Visibility.Visible : Visibility.Collapsed;

            nodeList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/PowerReg");
            vm.Power4Visibility = ((nodeList.Count > 0) && XmlManager.GetAttribute(nodeList[0], appDoc, "Visibility", ref str) && (str.Equals("Visible"))) ?
                Visibility.Visible : Visibility.Collapsed;

            nodeList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/PowerReg2");
            vm.Power5Visibility = ((nodeList.Count > 0) && XmlManager.GetAttribute(nodeList[0], appDoc, "Visibility", ref str) && (str.Equals("Visible"))) ?
                Visibility.Visible : Visibility.Collapsed;

            nodeList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/PinholeView");

            if ((nodeList.Count > 0) && XmlManager.GetAttribute(nodeList[0], appDoc, "Visibility", ref str) && (str.Equals("Visible")))
            {
                nodeList = expDoc.SelectNodes("/ThorImageExperiment/PinholeWheel");

                if (nodeList.Count > 0)
                {
                    if (XmlManager.GetAttribute(nodeList[0], expDoc, "micrometers", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iVal))
                    {
                        vm.PinholePosition = iVal;
                        vm.PinholeVisibility = Visibility.Visible;
                    }
                }
            }

            nodeList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/MultiLaserControlView");

            bool mclsSettingsFound = false;
            if ((nodeList.Count > 0) && XmlManager.GetAttribute(nodeList[0], appDoc, "Visibility", ref str) && (str.Equals("Visible")))
            {
                nodeList = expDoc.SelectNodes("/ThorImageExperiment/MCLS");

                if (nodeList.Count > 0)
                {
                    Visibility vis = Visibility.Collapsed;
                    double pow = 0;
                    GetMCLSChannelInfo(expDoc, nodeList, "enable1", "power1percent", ref vis, ref pow);
                    vm.MCLS1Visibility = vis;
                    vm.MCLS1Power = pow;
                    GetMCLSChannelInfo(expDoc, nodeList, "enable2", "power2percent", ref vis, ref pow);
                    vm.MCLS2Visibility = vis;
                    vm.MCLS2Power = pow;
                    GetMCLSChannelInfo(expDoc, nodeList, "enable3", "power3percent", ref vis, ref pow);
                    vm.MCLS3Visibility = vis;
                    vm.MCLS3Power = pow;
                    GetMCLSChannelInfo(expDoc, nodeList, "enable4", "power4percent", ref vis, ref pow);
                    vm.MCLS4Visibility = vis;
                    vm.MCLS4Power = pow;
                    mclsSettingsFound = true;
                }
            }

            if (false == mclsSettingsFound)
            {
                vm.MCLS1Visibility = Visibility.Collapsed;
                vm.MCLS2Visibility = Visibility.Collapsed;
                vm.MCLS3Visibility = Visibility.Collapsed;
                vm.MCLS4Visibility = Visibility.Collapsed;
            }

            nodeList = vm.HardwareDoc.SelectNodes("/HardwareSettings/Lasers/Laser");

            if (nodeList.Count > 0)
            {
                for (int i = 0; i < nodeList.Count; i++)
                {
                    if (XmlManager.GetAttribute(nodeList[i], vm.HardwareDoc, "name", ref str))
                    {
                        switch (i)
                        {
                            case 0: vm.MCLS1Name = str + ":"; break;
                            case 1: vm.MCLS2Name = str + ":"; break;
                            case 2: vm.MCLS3Name = str + ":"; break;
                            case 3: vm.MCLS4Name = str + ":"; break;
                        }
                    }
                }
            }

            //based on ApplicationSettings, FastZ panel visibility would be changed accordingly

            nodeList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/RunSample/FastZView");
            if (XmlManager.GetAttribute(nodeList[0], appDoc, "Visibility", ref str) && (str.EndsWith("Visible")))
            {
                groupBoxFastZ.Visibility = Visibility.Visible;
                nodeList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/RunSample/FastZView/VolumeTimeAdjustment");
                if (XmlManager.GetAttribute(nodeList[0], appDoc, "Visibility", ref str) && (str.EndsWith("Visible")))
                {
                    stackPanelVolumeTimeAdjustment.Visibility = Visibility.Visible;
                }
                else
                {
                    stackPanelVolumeTimeAdjustment.Visibility = Visibility.Collapsed;
                    vm.VolumeTimeAdjustMS = vm.StepTimeAdjustMS = 0;
                }
            }
            else
            {
                groupBoxFastZ.Visibility = stackPanelVolumeTimeAdjustment.Visibility = Visibility.Collapsed;
                vm.ZFastEnable = false;
                vm.VolumeTimeAdjustMS = vm.StepTimeAdjustMS = 0;
            }

            ckbStaircase.Visibility = (vm.IsCamera()) ? Visibility.Collapsed : Visibility.Visible;

            nodeList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/DigitalSwitchesView");

            if (nodeList.Count > 0)
            {
                digOutputBorder.Visibility = (XmlManager.GetAttribute(nodeList[0], appDoc, "Visibility", ref str) && (str.EndsWith("Visible"))) ?
                    Visibility.Visible : Visibility.Collapsed;
            }

            nodeList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIChartWindow");

            if (nodeList.Count > 0)
            {
                vm.ROIStatsChartActive = (XmlManager.GetAttribute(nodeList[0], appDoc, "display", ref str) && ("1" == str || Boolean.TrueString == str)) ?
                    true : false;
            }

            vm.ZFileVisibility = Visibility.Collapsed;
            nodeList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/RunSample/ZReadFromFile");
            if (XmlManager.GetAttribute(nodeList[0], appDoc, "Visibility", ref str) && (str.EndsWith("Visible")))
            {
                vm.ZFileVisibility = Visibility.Visible;
            }
        }

        private void SetImageUpdaterVisibility(int captureMode)
        {
            RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;

            vm.ImageUpdaterVisibility = Visibility.Visible;
        }

        private void SetNormalization(RunSampleLSViewModel vm)
        {
            //TODO:IV
            //XmlNodeList wl = vm.HardwareDoc.SelectNodes("/HardwareSettings/Wavelength");

            //if (wl.Count > 0)
            //{
            //    for (int i = 0; i < wl.Count; i++)
            //    {
            //        string bp = string.Empty;
            //        string wp = string.Empty;
            //        string autoTog = string.Empty;
            //        string logTog = string.Empty;
            //        XmlManager.GetAttribute(wl[i], vm.HardwareDoc, "bp", ref bp);
            //        XmlManager.GetAttribute(wl[i], vm.HardwareDoc, "wp", ref wp);
            //        XmlManager.GetAttribute(wl[i], vm.HardwareDoc, "AutoSta", ref autoTog);
            //        XmlManager.GetAttribute(wl[i], vm.HardwareDoc, "LogSta", ref logTog);
            //        switch (i)
            //        {
            //            case 0:
            //                vm.BlackPoint0 = Convert.ToDouble(bp);
            //                vm.WhitePoint0 = Convert.ToDouble(wp);
            //                vm.AutoManualTog1Checked = (string.Empty != autoTog && "1" == autoTog) ? true : false;
            //                vm.LogScaleEnabled0 = (string.Empty != logTog && "1" == logTog) ? true : false;
            //                break;
            //            case 1:
            //                vm.BlackPoint1 = Convert.ToDouble(bp);
            //                vm.WhitePoint1 = Convert.ToDouble(wp);
            //                vm.AutoManualTog2Checked = (string.Empty != autoTog && "1" == autoTog) ? true : false;
            //                vm.LogScaleEnabled1 = (string.Empty != logTog && "1" == logTog) ? true : false;
            //                break;
            //            case 2:
            //                vm.BlackPoint2 = Convert.ToDouble(bp);
            //                vm.WhitePoint2 = Convert.ToDouble(wp);
            //                vm.AutoManualTog3Checked = (string.Empty != autoTog && "1" == autoTog) ? true : false;
            //                vm.LogScaleEnabled2 = (string.Empty != logTog && "1" == logTog) ? true : false;
            //                break;
            //            case 3:
            //                vm.BlackPoint3 = Convert.ToDouble(bp);
            //                vm.WhitePoint3 = Convert.ToDouble(wp);
            //                vm.AutoManualTog4Checked = (string.Empty != autoTog && "1" == autoTog) ? true : false;
            //                vm.LogScaleEnabled3 = (string.Empty != logTog && "1" == logTog) ? true : false;
            //                break;
            //        }
            //        string str = string.Empty;
            //        XmlManager.GetAttribute(wl[i], vm.HardwareDoc, "name", ref str);
            //        vm.ChannelName[i].Value = str;
            //    }
            //}
        }

        void thumb_GotFocus(object sender, RoutedEventArgs e)
        {
            XamSliderNumericThumb thumb = sender as XamSliderNumericThumb;
            if (null != thumb && null != _viewModel && false == (bool)cbCustomRF.IsChecked)
            {
                thumb.IsActive = true;
                _lastRemovedThumb = thumb.Value;
                remoteFocusSlider.Thumbs.Remove(remoteFocusSlider.ActiveThumb);
                _viewModel.RemoteFocusCustomSelectedPlanes.Remove(_lastRemovedThumb);
                _viewModel.UpdateNumberOfPlanes();
            }
        }

        #endregion Methods
    }
}