namespace ImageReviewDll.View
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Threading;
    using System.Xml;

    using ExperimentSettingsBrowser;

    using FolderDialogControl;
    using ImageReviewDll.OME;
    using ImageReviewDll.View;
    using ImageReviewDll.ViewModel;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Win32;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    using TilesDisplay;

    /// <summary>
    /// Interaction logic for MasterView.xaml
    /// </summary>
    public partial class MasterView : UserControl
    {
        #region Fields

        private const int PATH_LENGTH = 261;

        object _dataContextliveImageVM = null;
        private string _expFile;
        private string _expPath;
        ImageReviewViewModel _imageReviewVM = null;
        DispatcherTimer _scanAreaTimer;
        private int _scanAreaTimerValue;
        DispatcherTimer _spTimer;
        private int _spTimerValue;
        DispatcherTimer _streamTimer;
        private int _streamTimerValue;
        DispatcherTimer _tTimer;
        private int _tTimerValue;
        DispatcherTimer _zTimer;
        private int _zTimerValue;

        #endregion Fields

        #region Constructors

        public MasterView()
        {
            InitializeComponent();
            this.KeyDown += ImageReviewView_KeyDown;
            this.Loaded += new RoutedEventHandler(MasterView_Loaded);
            this.Unloaded += new RoutedEventHandler(MasterView_Unloaded);
        }

        #endregion Constructors

        #region Delegates

        //Create a Delegate that matches
        //the Signature of the ProgressBar's SetValue method
        private delegate void UpdateProgressBarDelegate(System.Windows.DependencyProperty dp, Object value);

        #endregion Delegates

        #region Properties

        public object DataContextliveImageVM
        {
            get
            {
                return _dataContextliveImageVM;
            }
            set
            {
                _dataContextliveImageVM = value;

                _imageReviewVM = (ImageReviewViewModel)_dataContextliveImageVM;

            }
        }

        #endregion Properties

        #region Methods

        public void ImageReviewView_KeyDown(object sender, KeyEventArgs e)
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

        //Called from MainWindow at start of standalone app
        public void LoadLastExperiment()
        {
            LoadActiveExp();
        }

        public void OnSelectCapture(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }

            ExperimentSettingsBrowserWindow settingsDlg = new ExperimentSettingsBrowserWindow();
            settingsDlg.Title = "Experiment Browser";
            settingsDlg.BrowserType = ExperimentSettingsBrowserWindow.BrowserTypeEnum.EXPERIMENT;
            settingsDlg.Owner = Application.Current.MainWindow;
            try
            {
                if (true == settingsDlg.ShowDialog())
                {
                    LoadExperiment(settingsDlg.ExperimentPath);
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }
        public void OnConvertRawToTIFF(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }
            string tiffExperimentFilePth = "";

            //Get experiment folder and type
            var experimentDoc = new XmlDocument();
            experimentDoc.Load(_imageReviewVM.ImageReviewObject.ExperimentXMLPath);
            var pathXmlNode = experimentDoc.SelectSingleNode("/ThorImageExperiment/Name");
            if (pathXmlNode != null)
            {
                string pathString = string.Empty;
                if (XmlManager.GetAttribute(pathXmlNode, experimentDoc, "path", ref pathString))
                {
                    // Start converting
                    Task task = Task.Run(() =>
                    {
                        // Convert experiment from RAW to TIFF
                        ClassicTiffConverter tiffConverter = new ClassicTiffConverter(pathString);
                        long result = tiffConverter.DoConvertRawToTiff(ref tiffExperimentFilePth);
                    });
                    task.ContinueWith(x =>
                    {
                        Application.Current.Dispatcher.BeginInvoke((Action)delegate ()
                        {
                            _imageReviewVM.CloseProgressWindow();

                        // Show message to load new converted tiff experiment
                        if (MessageBox.Show("Would you like to open the converted experiment?", "", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                            LoadExperiment(tiffExperimentFilePth); // open new .tif experiment
                        });
                    });

                    // Show progress window
                    Application.Current.Dispatcher.BeginInvoke((Action)delegate ()
                    {
                        _imageReviewVM.CreateProgressWindow();
                    });
                }
            }
        }

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetActiveExperimentPath", CharSet = CharSet.Unicode)]
        private static extern int GetActiveExperimentPath(StringBuilder path, int length);

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetActiveExperimentPathAndName", CharSet = CharSet.Unicode)]
        private static extern int GetActiveExperimentPathAndName(StringBuilder path, int length);

        private void btnColor_Checked(object sender, RoutedEventArgs e)
        {
        }

        private void btnGreyScale_Checked(object sender, RoutedEventArgs e)
        {
        }

        //Called from ThorImage -> Review tab
        private void btnShowMostRecent_Click(object sender, RoutedEventArgs e)
        {
            //load the active experiment last run
            LoadActiveExp();
        }

        private void btnSp_Checked(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }

            _imageReviewVM.currentMovieParameter = ImageReviewViewModel.CurrentMovieParameterEnum.Sp;
            _imageReviewVM.MStart = _imageReviewVM.SpMin;
            _imageReviewVM.MEnd = _imageReviewVM.SpMax;
            _imageReviewVM.MStartValue = _imageReviewVM.SpMin;
            _imageReviewVM.MEndValue = _imageReviewVM.SpMax;
        }

        private void btnT_Checked(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }

            _imageReviewVM.currentMovieParameter = ImageReviewViewModel.CurrentMovieParameterEnum.T;
            _imageReviewVM.MStart = _imageReviewVM.TMin;
            _imageReviewVM.MEnd = _imageReviewVM.TMax;
            _imageReviewVM.MStartValue = _imageReviewVM.TMin;
            _imageReviewVM.MEndValue = _imageReviewVM.TMax;
        }

        private void btnZStream_Checked(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }

            _imageReviewVM.currentMovieParameter = ImageReviewViewModel.CurrentMovieParameterEnum.ZStream;
            _imageReviewVM.MStart = _imageReviewVM.ZStreamMin;
            _imageReviewVM.MEnd = _imageReviewVM.ZStreamMax;
            _imageReviewVM.MStartValue = _imageReviewVM.ZStreamMin;
            _imageReviewVM.MEndValue = _imageReviewVM.ZStreamMax;
        }

        private void btnZ_Checked(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }

            _imageReviewVM.currentMovieParameter = ImageReviewViewModel.CurrentMovieParameterEnum.Z;
            _imageReviewVM.MStart = _imageReviewVM.ZMin;
            _imageReviewVM.MEnd = _imageReviewVM.ZMax;
            _imageReviewVM.MStartValue = _imageReviewVM.ZMin;
            _imageReviewVM.MEndValue = _imageReviewVM.ZMax;
        }

        private void colorImageSettings_Click(object sender, RoutedEventArgs e)
        {
            SnapshotSettings dlg = new SnapshotSettings();

            XmlNodeList ndList = _imageReviewVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/GrayscaleForSingleChannel");

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                XmlManager.GetAttribute(ndList[0], _imageReviewVM.ApplicationDoc, "value", ref str);

                dlg.GrayscaleForSingleChannel = ("1" == str || Boolean.TrueString == str) ? true : false;
            }

            dlg.HardwareDoc = _imageReviewVM.HardwareDoc;

            if (true == dlg.ShowDialog())
            {

                if (ndList.Count > 0)
                {
                    string str = (true == dlg.GrayscaleForSingleChannel) ? "1" : "0";

                    XmlManager.SetAttribute(ndList[0], _imageReviewVM.ApplicationDoc, "value", str);
                }

                _imageReviewVM.ApplicationDoc.Save(_imageReviewVM.ApplicationSettingPath);

                //To load the color settings, force GrayscaleForSingleChannel to be false
                //this will allow to load the color channels colors instead of grey scale
                //then change it back to its original value
                _imageReviewVM.HardwareDoc.Save(_imageReviewVM.HardwareSettingPath);
                _imageReviewVM.GrayscaleForSingleChannel = false;
                _imageReviewVM.BuildChannelPalettes();
                _imageReviewVM.LoadColorImageSettings();
                if (true == dlg.GrayscaleForSingleChannel)
                {
                    _imageReviewVM.GrayscaleForSingleChannel = true;
                    _imageReviewVM.BuildChannelPalettes();
                }

                _imageReviewVM.UpdateBitmapAndEventSubscribers();
                _imageReviewVM.UpdateOrthogonalBitmapAndEventSubscribers();

                _imageReviewVM.FireColorMappingChangedAction(true);
            }
        }

        private void createMovie_Click(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM != null)
            {
                if (_imageReviewVM.ExperimentDoc != null)
                {
                    if (_imageReviewVM.MStartValue <= _imageReviewVM.MEndValue)
                    {
                        // Dialog box of movie file saving
                        SaveFileDialog dlg = new SaveFileDialog();
                        dlg.Title = "Save an Movie File";
                        dlg.FileName = string.Format("{0}{1}{2}",
                            "Movie",
                            "_" + ((int)1).ToString(_imageReviewVM.ImageNameFormat),
                            "_" + _imageReviewVM.SubTileIndex.ToString(_imageReviewVM.ImageNameFormat));
                        dlg.Filter = "Avi Movie file (*.avi)|*.avi";
                        Nullable<bool> result = dlg.ShowDialog();
                        if (result == true && dlg.FileName != "")
                        {
                            string destFileNamePath = dlg.FileName;
                            DirectoryInfo di = new DirectoryInfo(_imageReviewVM.ExperimentFolderPath);
                            string[][] fileNameList = new string[ExperimentData.NumberOfChannels][];
                            string temp;
                            switch (_imageReviewVM.ImageInfo.imageType)
                            {
                                case CaptureFile.FILE_BIG_TIFF:
                                    temp = string.Format("{0}", "Image");
                                    fileNameList[0] = new string[1 + _imageReviewVM.MEndValue - _imageReviewVM.MStartValue];
                                    fileNameList[0][0] = (di.GetFiles(temp + "*.tif*")[0].FullName);
                                    break;
                                case CaptureFile.FILE_RAW:
                                    temp = string.Format("{0}{1}{2}",
                                            "Image",
                                            "_" + _imageReviewVM.SampleSiteIndex.ToString(_imageReviewVM.ImageNameFormat),
                                            "_" + _imageReviewVM.SubTileIndex.ToString(_imageReviewVM.ImageNameFormat));
                                    fileNameList[0] = new string[1 + _imageReviewVM.MEndValue - _imageReviewVM.MStartValue];
                                    fileNameList[0][0] = (di.GetFiles(temp + "*.raw*")[0].FullName);
                                    break;
                                case CaptureFile.FILE_TIFF:
                                    for (int j = 0; j < ExperimentData.NumberOfChannels; j++)
                                    {
                                        fileNameList[j] = new string[1 + _imageReviewVM.MEndValue - _imageReviewVM.MStartValue];
                                        for (int i = _imageReviewVM.MStartValue, k = 0; i <= _imageReviewVM.MEndValue; i++, k++)
                                        {
                                            StringBuilder selectedFileName = new StringBuilder();
                                            int ZV = (btnZ.IsChecked.HasValue && (bool)btnZ.IsChecked) ? i : _imageReviewVM.ZValue;
                                            int TV = (btnT.IsChecked.HasValue && (bool)btnT.IsChecked) ? i : _imageReviewVM.TValue;
                                            int ZS = (btnZStream.IsChecked.HasValue && (bool)btnZStream.IsChecked) ? i : _imageReviewVM.ZStreamValue;
                                            int SP = (btnSp.IsChecked.HasValue && (bool)btnSp.IsChecked) ? i : _imageReviewVM.SpValue;

                                            int timeIndex = TV;
                                            if ((_imageReviewVM.ZStreamMode > 0) && (_imageReviewVM.ZStreamMax > 1))
                                            {
                                                timeIndex = ((TV - 1) * _imageReviewVM.ZMax + (ZV - 1)) * _imageReviewVM.ZStreamMax + ZS;
                                            }
                                            selectedFileName.AppendFormat("{0}{1}{2}{3}{4}{5}",
                                                                            _expPath.ToString() + "\\",
                                                                            ExperimentData.WaveLengthNames[j],
                                                                            "_" + SP.ToString(_imageReviewVM.ImageNameFormat),
                                                                            "_" + _imageReviewVM.SubTileIndex.ToString(_imageReviewVM.ImageNameFormat),
                                                                            "_" + ZV.ToString(_imageReviewVM.ImageNameFormat),
                                                                            "_" + timeIndex.ToString(_imageReviewVM.ImageNameFormat) + ".tif");
                                            fileNameList[j][k] = selectedFileName.ToString();
                                        }
                                    }
                                    break;
                                default:
                                    break;
                            }

                            _zTimer.Stop();
                            _imageReviewVM.ZIsLive = false;

                            _tTimer.Stop();
                            _imageReviewVM.TIsLive = false;

                            _spTimer.Stop();
                            _imageReviewVM.SpIsLive = false;

                            _streamTimer.Stop();
                            _imageReviewVM.ZStreamIsLive = false;

                            _imageReviewVM.CreateColorMovie(fileNameList, destFileNamePath, Double.Parse(fpsText.Text));
                        }
                    }
                    else
                    {
                        MessageBox.Show("Start cannot be greater than End Value", "Invalid values", MessageBoxButton.OK, MessageBoxImage.Error);
                    }

                }
            }
        }

        private void endSliderText_LostFocus(object sender, RoutedEventArgs e)
        {
            Regex regex = new Regex("^[0-9]*$");
            if (endSliderText.Text != "" && regex.IsMatch(endSliderText.Text))
            {
                if (btnT.IsChecked == true)
                {
                    if (!((Int32.Parse(endSliderText.Text) >= _imageReviewVM.TMin) && (Int32.Parse(endSliderText.Text) <= _imageReviewVM.TMax)))
                    {
                        MessageBox.Show("Enter T values between " + _imageReviewVM.TMin + " and " + _imageReviewVM.TMax, "Invalid Range", MessageBoxButton.OK, MessageBoxImage.Error);
                        endSliderText.Text = _imageReviewVM.MEndValue.ToString();
                    }
                }
                else if (btnSp.IsChecked == true)
                {
                    if (!((Int32.Parse(endSliderText.Text) >= _imageReviewVM.SpMin) && (Int32.Parse(endSliderText.Text) <= _imageReviewVM.SpMax)))
                    {
                        MessageBox.Show("Enter Sp values between " + _imageReviewVM.SpMin + " and " + _imageReviewVM.SpMax, "Invalid Range", MessageBoxButton.OK, MessageBoxImage.Error);
                        endSliderText.Text = _imageReviewVM.MEndValue.ToString();
                    }
                }
                else if (btnZ.IsChecked == true)
                {
                    if (!((Int32.Parse(endSliderText.Text) >= _imageReviewVM.ZMin) && (Int32.Parse(endSliderText.Text) <= _imageReviewVM.ZMax)))
                    {
                        MessageBox.Show("Enter Z values between " + _imageReviewVM.ZMin + " and " + _imageReviewVM.ZMax, "Invalid Range", MessageBoxButton.OK, MessageBoxImage.Error);
                        endSliderText.Text = _imageReviewVM.MEndValue.ToString();
                    }
                }
                else
                {
                    if (!((Int32.Parse(endSliderText.Text) >= _imageReviewVM.ZStreamMin) && (Int32.Parse(endSliderText.Text) <= _imageReviewVM.ZStreamMax)))
                    {
                        MessageBox.Show("Enter Z Stream values between " + _imageReviewVM.ZStreamMin + " and " + _imageReviewVM.ZStreamMax, "Invalid Range", MessageBoxButton.OK, MessageBoxImage.Error);
                        endSliderText.Text = _imageReviewVM.MEndValue.ToString();
                    }
                }
            }
            else if (endSliderText.Text == "")
            {
                //Do nothing, just wait for number to be entered
            }
            else
            {
                MessageBox.Show("Enter only numbers", "Invalid Value", MessageBoxButton.OK, MessageBoxImage.Error);
                endSliderText.Text = _imageReviewVM.ZValue.ToString();
                return;
            }
        }

        private void endSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }

            _imageReviewVM.MEndValue = (int)((Slider)e.Source).Value;
        }

        /// <summary>
        /// Updates the models bitmap to current parameters, overriding the minimum time spacing that
        /// is enforced in the bitmap setter
        /// </summary>
        private void ForceUpdateBitmap()
        {
            _imageReviewVM.UpdateBitmap(0);
        }

        private void fpsText_TextChanged(object sender, TextChangedEventArgs e)
        {
            Regex regex = new Regex("^[0-9]*$");
            if (regex.IsMatch(fpsText.Text))
            {
                return;
            }
            else
            {
                MessageBox.Show("Enter only numbers", "Invalid Value", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void inscribeScale_Click(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM != null)
            {
                if (_imageReviewVM.ExperimentDoc != null)
                {
                    _zTimer.Stop();
                    _imageReviewVM.ZIsLive = false;

                    _tTimer.Stop();
                    _imageReviewVM.TIsLive = false;

                    _spTimer.Stop();
                    _imageReviewVM.SpIsLive = false;

                    _streamTimer.Stop();
                    _imageReviewVM.ZStreamIsLive = false;

                    _imageReviewVM.InscribeScaleToImages();
                }
            }
        }

        private void LoadActiveExp()
        {
            XmlNode node;

            //reads experiment folder path from ApplicationSettings.xml
            // Use the application settings file from the current active modality
            if (null != MVMManager.Instance && null != MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS])
            {
                node = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS].SelectSingleNode("/ApplicationSettings/LastExperiment");
            }
            else
            {
                node = _imageReviewVM.ApplicationDoc.SelectSingleNode("/ApplicationSettings/LastExperiment");
            }
            string experimentPath = "";
            if (node != null && node.Attributes.GetNamedItem("path") != null)
            {
                experimentPath = node.Attributes["path"].Value;
                //Set ExperimentDoc to the path save in the curent modality's Application Settings
                //for when Show Most Recent is clicked
                XmlDocument experimentDoc = new XmlDocument();

                if (File.Exists(experimentPath + "\\Experiment.xml"))
                {
                    experimentDoc.Load(experimentPath + "\\Experiment.xml");
                    _imageReviewVM.ExperimentDoc = experimentDoc;
                }

                //Once ExperimentDoc is set, load the view model settings
                _imageReviewVM.LoadViewModelSettingsDoc();

                if (!LoadExperiment(experimentPath))
                {
                    MessageBox.Show("Application failed to load the last experiment. Please choose an experiment using Select Experiment button.", "Experiment Not Loaded", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        private void LoadData(string expfile, bool resetSliders)
        {
            if (_imageReviewVM == null)
            {
                return;
            }

            // to speed up load don't allow updates while loading experiment
            _imageReviewVM.AllowImageUpdate = false;

            //enabling the main control
            controlContainer.IsEnabled = true;
            XmlDataProvider dataProvider = (XmlDataProvider)this.FindResource("Experiment");
            XmlDocument experimentDoc = new XmlDocument();

            if (File.Exists(expfile))
            { experimentDoc.Load(expfile); }

            dataProvider.Document = experimentDoc;
            _imageReviewVM.ExperimentDoc = experimentDoc;

            ExperimentData.Populate(experimentDoc, _imageReviewVM.HardwareDoc);

            _imageReviewVM.ImageInfo = ExperimentData.ImageInfo;

            _imageReviewVM.ScanAreaIDMin = (CaptureFile.FILE_BIG_TIFF == _imageReviewVM.ImageInfo.imageType && 1 < _imageReviewVM.ImageInfo.scanAreaIDList.Count) ? _imageReviewVM.ImageInfo.scanAreaIDList[0].RegionID : 0;
            _imageReviewVM.ScanAreaIDMax = (CaptureFile.FILE_BIG_TIFF == _imageReviewVM.ImageInfo.imageType && 1 < _imageReviewVM.ImageInfo.scanAreaIDList.Count) ? _imageReviewVM.ImageInfo.scanAreaIDList[_imageReviewVM.ImageInfo.scanAreaIDList.Count - 1].RegionID : 0;
            _imageReviewVM.ScanAreaVisible = (CaptureFile.FILE_BIG_TIFF == _imageReviewVM.ImageInfo.imageType && MesoScanTypes.Micro == ExperimentData.ViewMode && 1 < _imageReviewVM.ImageInfo.scanAreaIDList.Count) ? true : false;

            _imageReviewVM.BitsPerPixel = ExperimentData.BitsPerPixel;

            _imageReviewVM.CaptureMode = ExperimentData.CaptureMode;

            _imageReviewVM.SpMax = ExperimentData.SpMax;
            _imageReviewVM.SpMin = 1;

            _imageReviewVM.ZMax = ExperimentData.ZMax;
            _imageReviewVM.ZMin = 1;

            _imageReviewVM.ZStreamMode = ExperimentData.ZStreamMode;
            _imageReviewVM.ZStreamMax = ExperimentData.ZStreamMax;
            _imageReviewVM.ZStreamMin = 1;

            if (ExperimentData.MMPerPixel != 0)
            {
                _imageReviewVM.ZVolumeSpacing = ExperimentData.ZStepSizeUM / (ExperimentData.MMPerPixel * 1000);
            }

            _imageReviewVM.TMax = ExperimentData.TMax;
            _imageReviewVM.TMin = 1;

            _imageReviewVM.SubTileIndex = 1;

            if (true == resetSliders)
            {
                ResetImageSliders();
            }

            _imageReviewVM.WavelengthNames = ExperimentData.WaveLengthNames;

            OverlayManagerClass.Instance.BinX = ExperimentData.BinX;
            OverlayManagerClass.Instance.BinY = ExperimentData.BinY;

            //If it is a hyperspectral capture then do spectral bar
            if (CaptureModes.HYPERSPECTRAL == ExperimentData.CaptureMode)
            {
                // keep watching whenever we have a camera without hyperSpectrum application.
                _imageReviewVM.MStart = _imageReviewVM.SpMin;
                _imageReviewVM.MEnd = _imageReviewVM.SpMax;
                _imageReviewVM.MStartValue = _imageReviewVM.SpMin;
                _imageReviewVM.MEndValue = _imageReviewVM.SpMax;

                btnSp.IsChecked = true;
            }
            else
            {
                //loading the default movie slider parameters for LSM configurationi
                _imageReviewVM.MStart = _imageReviewVM.ZMin;
                _imageReviewVM.MEnd = _imageReviewVM.ZMax;
                _imageReviewVM.MStartValue = _imageReviewVM.ZMin;
                _imageReviewVM.MEndValue = _imageReviewVM.ZMax;

                btnZ.IsChecked = true;
            }

            Color wlColor = Color.FromArgb(0xFF, 0xFF, 0xFF, 0xFF);
            _imageReviewVM.WavelengthColor = wlColor;

            _imageReviewVM.UpdateBitmapAndEventSubscribers();

            for (int i = 0; i < ExperimentData.ImageInfo.channelEnable.Length; ++i)
            {
                switch (i)
                {
                    case 0:
                        _imageReviewVM.ChannelEnableA = ExperimentData.ImageInfo.channelEnable[i];
                        chanAEnable.Visibility = _imageReviewVM.ChannelEnableA ? Visibility.Visible : Visibility.Collapsed;
                        break;
                    case 1:
                        _imageReviewVM.ChannelEnableB = ExperimentData.ImageInfo.channelEnable[i];
                        chanBEnable.Visibility = _imageReviewVM.ChannelEnableB ? Visibility.Visible : Visibility.Collapsed;
                        break;
                    case 2:
                        _imageReviewVM.ChannelEnableC = ExperimentData.ImageInfo.channelEnable[i];
                        chanCEnable.Visibility = _imageReviewVM.ChannelEnableC ? Visibility.Visible : Visibility.Collapsed;
                        break;
                    case 3:
                        _imageReviewVM.ChannelEnableD = ExperimentData.ImageInfo.channelEnable[i];
                        chanDEnable.Visibility = _imageReviewVM.ChannelEnableD ? Visibility.Visible : Visibility.Collapsed;
                        break;
                }
            }

            //load userControl
            if (File.Exists(expfile))
            {
                tileControl.SelectExpFile(expfile);
            }
            // At this point allow data update
            _imageReviewVM.AllowImageUpdate = true;
            _imageReviewVM.ExperimentDataLoaded();
        }

        /// <summary>
        /// Loads an experiment from file
        /// </summary>
        /// <param name="experimentPath"> The path to the experiment to load </param>
        /// <returns> True if the experiment was successfully loaded </returns>
        private bool LoadExperiment(string experimentPath)
        {
            ResetPlayStopButtons();
            if (!String.IsNullOrEmpty(experimentPath))
            {
                _expPath = experimentPath;
                _expFile = _expPath + "\\Experiment.xml";
            }

            if (!File.Exists(_expFile) || (_imageReviewVM == null))
            {
                return false;
            }

            bool resetSliders = (_expPath != _imageReviewVM.ExperimentFolderPath) ? true : false;
            if (resetSliders)
            {
                ResetImageSliders();
            }

            _imageReviewVM.ExperimentXMLPath = _expFile;
            _imageReviewVM.ExperimentFolderPath = _expPath;

            LoadData(_expFile, resetSliders);
            _imageReviewVM.LoadExpFolder(_expPath);

            Match match = Regex.Match(_expPath.TrimEnd('\\'), "(.*)\\\\(.*)");

            //Experiment Folder Exists
            if (match.Groups.Count > 2)
            {

                string experimentName = match.Groups[2].Value; //Split by file separator

                //----------------------------------------------------------------
                // Labels do not show every other underscore, as they are used for
                // accelerator keys. Putting an extra underscore before each underscore
                // in the experiment name ensures all underscores are still displayed
                //----------------------------------------------------------------
                experimentName = experimentName.Replace("_", "__");

                labelSelectedExperiment.Content = experimentName; // Set the label content

            }

            UpdateUserControls();

            // Show/hide ConvertToTIFF button
            _imageReviewVM.IsRawExperiment = _imageReviewVM.ImageInfo.imageType == CaptureFile.FILE_RAW;

            return true;
        }

        private void LoadPaletteColor()
        {
            // implement this later
        }

        //init function
        void MasterView_Loaded(object sender, RoutedEventArgs e)
        {
            if (null == ((MasterView)sender).DataContext)
            {
                return;
            }

            _spTimer = new DispatcherTimer();

            _zTimer = new DispatcherTimer();

            _tTimer = new DispatcherTimer();

            _streamTimer = new DispatcherTimer();

            _scanAreaTimer = new DispatcherTimer();

            _imageReviewVM.ReloadSettingsByExpModality();

            _zTimer.Tick += new EventHandler(zTimerTick);
            _tTimer.Tick += new EventHandler(tTimerTick);
            _spTimer.Tick += new EventHandler(spTimerTick);
            _streamTimer.Tick += new EventHandler(streamTimer_Tick);
            _scanAreaTimer.Tick += new EventHandler(scanAreaTimer_Tick);
            this.tileControl.TilesIndexChangedEvent += tileControl_TilesIndexChangedEvent;
            this.tileControl.Loaded += tileControl_Loaded;
            _imageReviewVM.EnableHandlers();

            // Show/hide "Open In ThorAnalysis" button
            string str = string.Empty;
            var node = _imageReviewVM.ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/Review/OpenInThorAnalysisButton");
            if (XmlManager.GetAttribute(node, _imageReviewVM.ApplicationDoc, "Visibility", ref str) && (str.EndsWith("Visible")))
            {
                btLoadThorAnalysis.Visibility = Visibility.Visible;                
            }
            else
            {
                btLoadThorAnalysis.Visibility = Visibility.Collapsed;
            }
        }

        void MasterView_Unloaded(object sender, RoutedEventArgs e)
        {
            _zTimer.Tick -= new EventHandler(zTimerTick);
            _tTimer.Tick -= new EventHandler(tTimerTick);
            _spTimer.Tick -= new EventHandler(spTimerTick);
            _streamTimer.Tick -= new EventHandler(streamTimer_Tick);
            this.tileControl.TilesIndexChangedEvent -= tileControl_TilesIndexChangedEvent;
            this.tileControl.Loaded -= tileControl_Loaded;

            _imageReviewVM.ReleaseHandlers();

            if (_imageReviewVM.TIsLive)
            {
                _tTimer.Stop();
                _imageReviewVM.TIsLive = false;
                _imageReviewVM.ZIsEnabled = true;
                _imageReviewVM.ZStreamIsEnabled = true;
                _imageReviewVM.SpIsEnabled = true;
            }

            if (_imageReviewVM == null)
            {
                return;
            }

            if (_imageReviewVM.SpIsLive)
            {
                _spTimer.Stop();
                _imageReviewVM.SpIsLive = false;
                _imageReviewVM.ZIsEnabled = true;
                _imageReviewVM.TIsEnabled = true;
                _imageReviewVM.ZStreamIsEnabled = true;
            }

            if (_imageReviewVM.ZIsLive)
            {
                _zTimer.Stop();
                _imageReviewVM.ZIsLive = false;
                _imageReviewVM.TIsEnabled = true;
                _imageReviewVM.ZStreamIsEnabled = true;
                _imageReviewVM.SpIsEnabled = true;
            }

            if (_imageReviewVM.ZStreamIsLive)
            {
                _streamTimer.Stop();

                //changed the click image control
                _imageReviewVM.ZStreamIsLive = false;

                //enable T Z back when stream play is finished
                _imageReviewVM.ZIsEnabled = true;
                _imageReviewVM.TIsEnabled = true;
                _imageReviewVM.SpIsEnabled = true;
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

        /// <summary>
        /// Forces an update of the bitmap, overriding any normal time spacing 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void PlayButton_IsEnabledChange(object sender, DependencyPropertyChangedEventArgs e)
        {
            Button button = (Button)sender;
            if (button.IsEnabled)
            {
                ForceUpdateBitmap();
            }
        }

        /// <summary>
        /// Resets the z,t, and stream sliders that control the currently previewed iamge
        /// </summary>
        private void ResetImageSliders()
        {
            //Reset all Sliders
            _imageReviewVM.TValue = _imageReviewVM.TMin;
            _imageReviewVM.ZValue = _imageReviewVM.ZMax > 1 ? (int)Math.Ceiling(_imageReviewVM.ZMax / 2.0) : 1;
            _imageReviewVM.ZStreamValue = _imageReviewVM.ZStreamMin;
            _imageReviewVM.SpValue = _imageReviewVM.SpMin;
            _imageReviewVM.ScanAreaID = _imageReviewVM.ScanAreaIDMin;

            //Reset all timers
            _tTimerValue = 1;
            _zTimerValue = 1;
            _streamTimerValue = 1;
            _spTimerValue = 1;
        }

        private void ResetPlayStopButtons()
        {
            //Reset play/stop buttos
            if (null != _tTimer) _tTimer.Stop();
            if (null != _zTimer) _zTimer.Stop();
            if (null != _streamTimer) _streamTimer.Stop();
            if (null != _spTimer) _spTimer.Stop();
            if (null != _scanAreaTimer) _scanAreaTimer.Stop();
            _imageReviewVM.TIsLive = false;
            _imageReviewVM.ZIsLive = false;
            _imageReviewVM.ZStreamIsLive = false;
            _imageReviewVM.SpIsLive = false;
            _imageReviewVM.ScanAreaIsLive = false;
            _imageReviewVM.TIsEnabled = true;
            _imageReviewVM.ZIsEnabled = true;
            _imageReviewVM.ZStreamIsEnabled = true;
            _imageReviewVM.SpIsEnabled = true;
            _imageReviewVM.ScanAreaIsEnabled = true;

            //Reset all timers
            _tTimerValue = 1;
            _zTimerValue = 1;
            _streamTimerValue = 1;
            _spTimerValue = 1;
            _scanAreaTimerValue = _imageReviewVM.ScanAreaIDMin;

            //Reset sliders to 1 in case the same experiment is being loaded
            _imageReviewVM.TValue = 1;
            _imageReviewVM.ZValue = 1;
            _imageReviewVM.ZStreamValue = 1;
            _imageReviewVM.SpValue = 1;
            _imageReviewVM.ScanAreaID = _imageReviewVM.ScanAreaIDMin;
        }

        private void ScanAreaSliderText_LostFocus(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM == null)
                return;

            Regex regex = new Regex("^[0-9]*$");
            if (scanAreaSliderText.Text != "" && regex.IsMatch(scanAreaSliderText.Text))
            {
                if ((Int32.Parse(scanAreaSliderText.Text) < _imageReviewVM.ScanAreaIDMin) || (Int32.Parse(spSliderText.Text) > _imageReviewVM.ScanAreaIDMax))
                {
                    MessageBox.Show("Enter values between " + _imageReviewVM.ScanAreaIDMin + " and " + _imageReviewVM.ScanAreaIDMax, "Invalid Range", MessageBoxButton.OK, MessageBoxImage.Error);
                    scanAreaSliderText.Text = _imageReviewVM.ScanAreaID.ToString();
                }
            }
            else if (scanAreaSliderText.Text == "")
            {
                //Do nothing, just wait for number to be entered
            }
            else
            {
                MessageBox.Show("Enter only numbers", "Invalid Value", MessageBoxButton.OK, MessageBoxImage.Error);
                scanAreaSliderText.Text = _imageReviewVM.ScanAreaID.ToString();
                return;
            }
        }

        private void ScanAreaStart_Click(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM == null)
                return;

            if (!_imageReviewVM.ScanAreaIsLive)
            {
                if (_imageReviewVM.ScanAreaIDMax == _imageReviewVM.ScanAreaID)
                {
                    _scanAreaTimerValue = _imageReviewVM.ScanAreaIDMin;
                }
                else
                {
                    _scanAreaTimerValue = _imageReviewVM.ScanAreaID;
                }

                _scanAreaTimer.Interval = TimeSpan.FromMilliseconds(Convert.ToDouble(0));
                _scanAreaTimer.Start();

                //change the click image control
                _imageReviewVM.ScanAreaIsLive = true;
                _imageReviewVM.TIsEnabled = false;
                _imageReviewVM.ZIsEnabled = false;
                _imageReviewVM.ZStreamIsEnabled = false;
                _imageReviewVM.SpIsEnabled = false;
            }
            else
            {
                _scanAreaTimer.Stop();
                _imageReviewVM.ScanAreaIsLive = false;
                _imageReviewVM.TIsEnabled = true;
                _imageReviewVM.ZIsEnabled = true;
                _imageReviewVM.ZStreamIsEnabled = true;
                _imageReviewVM.SpIsEnabled = true;
            }
        }

        void scanAreaTimer_Tick(object sender, EventArgs e)
        {
            if (_imageReviewVM == null)
                return;

            _imageReviewVM.ScanAreaID = _scanAreaTimerValue;

            if (_scanAreaTimerValue < _imageReviewVM.ScanAreaIDMax)
            {
                _scanAreaTimerValue++;
            }
            else
            {
                _scanAreaTimerValue = _imageReviewVM.ScanAreaIDMin;
            }
        }

        /// <summary>
        /// Forces an update of the bitmap, overriding any normal time spacing 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Slider_DragComplete(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
        {
            ForceUpdateBitmap();
        }

        private void spSliderText_LostFocus(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }

            Regex regex = new Regex("^[0-9]*$");
            if (spSliderText.Text != "" && regex.IsMatch(spSliderText.Text))
            {
                if ((Int32.Parse(spSliderText.Text) < _imageReviewVM.SpMin) || (Int32.Parse(spSliderText.Text) > _imageReviewVM.SpMax))
                {
                    MessageBox.Show("Enter values between " + _imageReviewVM.SpMin + " and " + _imageReviewVM.SpMax, "Invalid Range", MessageBoxButton.OK, MessageBoxImage.Error);
                    spSliderText.Text = _imageReviewVM.SpValue.ToString();
                }
            }
            else if (spSliderText.Text == "")
            {
                //Do nothing, just wait for number to be entered
            }
            else
            {
                MessageBox.Show("Enter only numbers", "Invalid Value", MessageBoxButton.OK, MessageBoxImage.Error);
                spSliderText.Text = _imageReviewVM.SpValue.ToString();
                return;
            }
        }

        private void spStart_Click(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }
            if (!_imageReviewVM.SpIsLive)
            {
                if (_imageReviewVM.SpMax == _imageReviewVM.SpValue)
                {
                    _spTimerValue = _imageReviewVM.SpMin;
                }
                else
                {
                    _spTimerValue = _imageReviewVM.SpValue;
                }

                _spTimer.Interval = TimeSpan.FromMilliseconds(Convert.ToDouble(0));
                _spTimer.Start();

                //change the click image control
                _imageReviewVM.SpIsLive = true;
                _imageReviewVM.TIsEnabled = false;
                _imageReviewVM.ZStreamIsEnabled = false;
                _imageReviewVM.ZIsEnabled = false;
            }
            else
            {
                _spTimer.Stop();
                _imageReviewVM.SpIsLive = false;
                _imageReviewVM.TIsEnabled = true;
                _imageReviewVM.ZStreamIsEnabled = true;
                _imageReviewVM.ZIsEnabled = true;
            }
        }

        private void spTimerTick(object sender, EventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }
            _imageReviewVM.SpValue = _spTimerValue;

            _spTimerValue = _spTimerValue % _imageReviewVM.SpMax + 1;
        }

        private void startSliderText_LostFocus(object sender, RoutedEventArgs e)
        {
            Regex regex = new Regex("^[0-9]*$");

            if (startSliderText.Text != "" && regex.IsMatch(startSliderText.Text))
            {
                if (btnT.IsChecked == true)
                {
                    if (!((Int32.Parse(startSliderText.Text) >= _imageReviewVM.TMin) && (Int32.Parse(startSliderText.Text) <= _imageReviewVM.TMax)))
                    {
                        MessageBox.Show("Enter T values between " + _imageReviewVM.TMin + " and " + _imageReviewVM.TMax, "Invalid Range", MessageBoxButton.OK, MessageBoxImage.Error);
                        startSliderText.Text = _imageReviewVM.MStartValue.ToString();
                    }
                }
                else if (btnSp.IsChecked == true)
                {
                    if (!((Int32.Parse(startSliderText.Text) >= _imageReviewVM.SpMin) && (Int32.Parse(startSliderText.Text) <= _imageReviewVM.SpMax)))
                    {
                        MessageBox.Show("Enter Sp values between " + _imageReviewVM.SpMin + " and " + _imageReviewVM.SpMax, "Invalid Range", MessageBoxButton.OK, MessageBoxImage.Error);
                        startSliderText.Text = _imageReviewVM.MStartValue.ToString();
                    }
                }
                else if (btnZ.IsChecked == true)
                {
                    if (!((Int32.Parse(startSliderText.Text) >= _imageReviewVM.ZMin) && (Int32.Parse(startSliderText.Text) <= _imageReviewVM.ZMax)))
                    {
                        MessageBox.Show("Enter Z values between " + _imageReviewVM.ZMin + " and " + _imageReviewVM.ZMax, "Invalid Range", MessageBoxButton.OK, MessageBoxImage.Error);
                        startSliderText.Text = _imageReviewVM.MStartValue.ToString();
                    }
                }
                else
                {
                    if (!((Int32.Parse(startSliderText.Text) >= _imageReviewVM.ZStreamMin) && (Int32.Parse(startSliderText.Text) <= _imageReviewVM.ZStreamMax)))
                    {
                        MessageBox.Show("Enter Z Stream values between " + _imageReviewVM.ZStreamMin + " and " + _imageReviewVM.ZStreamMax, "Invalid Range", MessageBoxButton.OK, MessageBoxImage.Error);
                        startSliderText.Text = _imageReviewVM.MStartValue.ToString();
                    }
                }
            }
            else if (startSliderText.Text == "")
            {
                //Do nothing, just wait for number to be entered
            }
            else
            {
                MessageBox.Show("Enter only numbers", "Invalid Value", MessageBoxButton.OK, MessageBoxImage.Error);
                startSliderText.Text = _imageReviewVM.ZValue.ToString();
                return;
            }
        }

        private void startSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }
            _imageReviewVM.MStartValue = (int)((Slider)e.Source).Value;
        }

        private void streamStart_Click(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }
            if (!_imageReviewVM.ZStreamIsLive)
            {
                if (_imageReviewVM.ZStreamMax == _imageReviewVM.ZStreamValue)
                {
                    _streamTimerValue = _imageReviewVM.ZStreamMin;
                }
                else
                {
                    _streamTimerValue = _imageReviewVM.ZStreamValue;
                }

                _streamTimer.Interval = TimeSpan.FromMilliseconds(Convert.ToDouble(0));
                _streamTimer.Start();

                //change the click image control
                _imageReviewVM.ZStreamIsLive = true;

                //disable T Z while stream is playing
                _imageReviewVM.ZIsEnabled = false;
                _imageReviewVM.TIsEnabled = false;
                _imageReviewVM.SpIsEnabled = false;
            }
            else
            {
                _streamTimer.Stop();

                //changed the click image control
                _imageReviewVM.ZStreamIsLive = false;

                //enable T Z back when stream play is finished
                _imageReviewVM.ZIsEnabled = true;
                _imageReviewVM.TIsEnabled = true;
                _imageReviewVM.SpIsEnabled = true;
            }
        }

        void streamTimer_Tick(object sender, EventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }
            _imageReviewVM.ZStreamValue = _streamTimerValue;

            if (_streamTimerValue < _imageReviewVM.ZStreamMax)
            {
                _streamTimerValue++;
            }
            else
            {
                _streamTimerValue = 1;
            }
        }

        void tileControl_Loaded(object sender, RoutedEventArgs e)
        {
            if (File.Exists(_expFile))
            {
                tileControl.TileIndex = 1;
                tileControl.SelectExpFile(_expFile);
            }
        }

        void tileControl_TilesIndexChangedEvent(int wellIndex, int tilesIndex)
        {
            if (wellIndex != _imageReviewVM.SampleSiteIndex || tilesIndex != _imageReviewVM.SubTileIndex)
            {
                _imageReviewVM.SubTileIndex = tilesIndex;
                _imageReviewVM.SampleSiteIndex = wellIndex;
                _imageReviewVM.UpdateBitmapAndEventSubscribers();
                _imageReviewVM.SyncROIChart();
            }
        }

        private void tSliderText_LostFocus(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }

            Regex regex = new Regex("^[0-9]*$");
            if (tSliderText.Text != "" && regex.IsMatch(tSliderText.Text))
            {
                if ((Int32.Parse(tSliderText.Text) < _imageReviewVM.TMin) || (Int32.Parse(tSliderText.Text) > _imageReviewVM.TMax))
                {
                    MessageBox.Show("Enter values between " + _imageReviewVM.TMin + " and " + _imageReviewVM.TMax, "Invalid Range", MessageBoxButton.OK, MessageBoxImage.Error);
                    tSliderText.Text = _imageReviewVM.TValue.ToString();
                }
            }
            else if (tSliderText.Text == "")
            {
                //Do nothing, just wait for number to be entered
            }
            else
            {
                MessageBox.Show("Enter only numbers", "Invalid Value", MessageBoxButton.OK, MessageBoxImage.Error);
                tSliderText.Text = _imageReviewVM.ZValue.ToString();
                return;
            }
        }

        private void tStart_Click(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }
            if (!_imageReviewVM.TIsLive)
            {
                if (_imageReviewVM.TMax == _imageReviewVM.TValue)
                {
                    _tTimerValue = _imageReviewVM.TMin;
                }
                else
                {
                    _tTimerValue = _imageReviewVM.TValue;
                }

                _tTimer.Interval = TimeSpan.FromMilliseconds(Convert.ToDouble(0));
                _tTimer.Start();

                //change the click image control
                _imageReviewVM.TIsLive = true;
                _imageReviewVM.ZIsEnabled = false;
                _imageReviewVM.ZStreamIsEnabled = false;
                _imageReviewVM.SpIsEnabled = false;
            }
            else
            {
                _tTimer.Stop();
                _imageReviewVM.TIsLive = false;
                _imageReviewVM.ZIsEnabled = true;
                _imageReviewVM.ZStreamIsEnabled = true;
                _imageReviewVM.SpIsEnabled = true;
            }
        }

        private void tTimerTick(object sender, EventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }
            _imageReviewVM.TValue = _tTimerValue;

            if (_tTimerValue < _imageReviewVM.TMax)
            {
                _tTimerValue++;
            }
            else
            {
                _tTimerValue = 1;
            }
        }

        private void UpdateUserControls()
        {
            try
            {
                XmlDocument doc = new XmlDocument();
                doc.LoadXml("<Command><Experiment value=\"" + _imageReviewVM.ExperimentXMLPath + "\"></Experiment></Command>");
                doc.PreserveWhitespace = true;
                settingsPreview.EditEnable = false;
                settingsPreview.SettingsDocument = doc;
            }
            catch { }
        }

        private void zSliderText_LostFocus(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }

            Regex regex = new Regex("^[0-9]*$");
            if (zSliderText.Text != "" && regex.IsMatch(zSliderText.Text))
            {
                if (!((Int32.Parse(zSliderText.Text) >= _imageReviewVM.ZMin) && (Int32.Parse(zSliderText.Text) <= _imageReviewVM.ZMax)))
                {
                    MessageBox.Show("Enter values between " + _imageReviewVM.ZMin + " and " + _imageReviewVM.ZMax, "Invalid Range", MessageBoxButton.OK, MessageBoxImage.Error);
                    zSliderText.Text = _imageReviewVM.ZValue.ToString();
                }
            }
            else if (zSliderText.Text == "")
            {
                //Do nothing, just wait for text to be entered
            }
            else
            {
                MessageBox.Show("Enter only numbers", "Invalid Value", MessageBoxButton.OK, MessageBoxImage.Error);
                tSliderText.Text = _imageReviewVM.ZValue.ToString();
                return;
            }
        }

        private void zStart_Click(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }
            if (!_imageReviewVM.ZIsLive)
            {
                if (_imageReviewVM.ZMax == _imageReviewVM.ZValue)
                {
                    _zTimerValue = _imageReviewVM.ZMin;
                }
                else
                {
                    _zTimerValue = _imageReviewVM.ZValue;
                }

                _zTimer.Interval = TimeSpan.FromMilliseconds(Convert.ToDouble(0));
                _zTimer.Start();

                //change the click image control
                _imageReviewVM.ZIsLive = true;
                _imageReviewVM.TIsEnabled = false;
                _imageReviewVM.ZStreamIsEnabled = false;
                _imageReviewVM.SpIsEnabled = false;
            }
            else
            {
                _zTimer.Stop();
                _imageReviewVM.ZIsLive = false;
                _imageReviewVM.TIsEnabled = true;
                _imageReviewVM.ZStreamIsEnabled = true;
                _imageReviewVM.SpIsEnabled = true;
            }
        }

        private void zStreamSliderText_LostFocus(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }

            Regex regex = new Regex("^[0-9]*$");
            if (zStreamSliderText.Text != "" && regex.IsMatch(zStreamSliderText.Text))
            {
                if (!((Int32.Parse(zStreamSliderText.Text) >= _imageReviewVM.ZStreamMin) && (Int32.Parse(zStreamSliderText.Text) <= _imageReviewVM.ZStreamMax)))
                {
                    MessageBox.Show("Enter values between " + _imageReviewVM.ZStreamMin + " and " + _imageReviewVM.ZStreamMax, "Invalid Range", MessageBoxButton.OK, MessageBoxImage.Error);
                    zStreamSliderText.Text = _imageReviewVM.ZStreamValue.ToString();
                }
            }
            else if (zStreamSliderText.Text == "")
            {
                //Do nothing, just wait for number to be entered
            }
            else
            {
                MessageBox.Show("Enter only numbers", "Invalid Value", MessageBoxButton.OK, MessageBoxImage.Error);
                tSliderText.Text = _imageReviewVM.ZValue.ToString();
                return;
            }
        }

        private void zTimerTick(object sender, EventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }
            _imageReviewVM.ZValue = _zTimerValue;

            if (_zTimerValue < _imageReviewVM.ZMax)
            {
                _zTimerValue++;
            }
            else
            {
                _zTimerValue = 1;
            }
        }

        #endregion Methods
    }
}