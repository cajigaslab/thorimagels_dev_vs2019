namespace ImageReviewDll.View
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Data;
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

    using DatabaseInterface;

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

        const int TIMEOUT_MILLISECONDS = 5000;

        private static int _maxPreviewDisplayFrameRate = 120;
        private static bool _spIndexThreadRun = false;
        private static int _spTimerValue;
        private static bool _streamIndexThreadRun = false;
        private static int _streamTimerValue;
        private static bool _tIndexThreadRun = false;
        private static int _tTimerValue;
        private static bool _zIndexThreadRun = false;
        private static int _zTimerValue;

        object _dataContextliveImageVM = null;
        private string _expFile;
        private string _expPath;
        ImageReviewViewModel _imageReviewVM = null;
        Thread _spIndexLive;
        Thread _streamIndexLive;
        Thread _tIndexLive;
        Thread _zIndexLive;

        #endregion Fields

        #region Constructors

        public MasterView()
        {
            InitializeComponent();
            this.KeyDown += ImageReviewView_KeyDown;
            this.Loaded += new RoutedEventHandler(MasterView_Loaded);
            this.Unloaded += new RoutedEventHandler(MasterView_Unloaded);
            System.Windows.Application.Current.Exit += Stop_PlayMovie_Threads;
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

        public bool ForceBitMap
        {
            get
            {
                ForceUpdateBitmap();
                return false;
            }
            set
            {
                ForceUpdateBitmap();
            }
        }

        #endregion Properties

        #region Methods

        public void Stop_PlayMovie_Threads(object sender, ExitEventArgs e)
        {
            StopSPIndexThread();
            StopStreamIndexThread();
            StopTIndexThread();
            StopZIndexThread();

        }

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

        public void LoadRemoteFocusPositionValues()
        {
            //Load Dynamic Labels for Remote Focus
            if (ExperimentData.IsRemoteFocus)
            {
                string posFile = _expPath + "\\RemoteFocus\\RemoteFocusPositionsValues.txt";

                if (File.Exists(posFile))
                {
                    var reader = new StreamReader(File.OpenRead(posFile));

                    double tmp = 0;
                    _imageReviewVM._remoteFocusPositions.Clear();

                    while (!reader.EndOfStream)
                    {
                        string l = reader.ReadLine();

                        if (Double.TryParse(l, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                        {
                            _imageReviewVM._remoteFocusPositions.Add(tmp);
                        }

                    }
                }
            }
        }

        public void OnConvertRawToTIFF(object sender, RoutedEventArgs e)
        {
            if (_imageReviewVM == null)
            {
                return;
            }
            string tiffExperimentFilePth = "";

            XmlDocument experimentDoc = _imageReviewVM.LoadDocAsReadOnly(_imageReviewVM.ImageReviewObject.ExperimentXMLPath);
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
                        _imageReviewVM.CreateProgressWindow("Converting", Brushes.DimGray);
                    });
                }
            }
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

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetActiveExperimentPath", CharSet = CharSet.Unicode)]
        private static extern int GetActiveExperimentPath(StringBuilder path, int length);

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetActiveExperimentPathAndName", CharSet = CharSet.Unicode)]
        private static extern int GetActiveExperimentPathAndName(StringBuilder path, int length);

        private static void SPIndexThreadProc(Object obj)
        {
            ImageReviewViewModel imageReviewVM = (ImageReviewViewModel)obj;
            int minTimeBetweenFrames = (int)Math.Floor((Decimal)Constants.MS_TO_SEC / _maxPreviewDisplayFrameRate);

            while (_spIndexThreadRun)
            {
                ImageReviewViewModel._bitmapIsLoading = true;

                _spTimerValue = _spTimerValue % imageReviewVM.SpMax + 1;

                imageReviewVM.SpValue = _spTimerValue;

                DateTime startTime = DateTime.Now;
                TimeSpan elapsedTime = DateTime.Now - startTime;
                while ((ImageReviewViewModel._bitmapIsLoading || elapsedTime.TotalMilliseconds < minTimeBetweenFrames) && elapsedTime.TotalMilliseconds < TIMEOUT_MILLISECONDS)
                {
                    elapsedTime = DateTime.Now - startTime;
                    Thread.Sleep(2);
                }
            }
        }

        private static void StreamIndexThreadProc(Object obj)
        {
            ImageReviewViewModel imageReviewVM = (ImageReviewViewModel)obj;
            int minTimeBetweenFrames = (int)Math.Floor((Decimal)Constants.MS_TO_SEC / _maxPreviewDisplayFrameRate);

            while (_streamIndexThreadRun)
            {
                ImageReviewViewModel._bitmapIsLoading = true;

                _streamTimerValue = _streamTimerValue % imageReviewVM.ZStreamMax + 1;

                imageReviewVM.ZStreamValue = _streamTimerValue;

                DateTime startTime = DateTime.Now;
                TimeSpan elapsedTime = DateTime.Now - startTime;
                while ((ImageReviewViewModel._bitmapIsLoading || elapsedTime.TotalMilliseconds < minTimeBetweenFrames) && elapsedTime.TotalMilliseconds < TIMEOUT_MILLISECONDS)
                {
                    elapsedTime = DateTime.Now - startTime;
                    Thread.Sleep(2);
                }
            }
        }

        private static void TIndexThreadProc(Object obj)
        {
            ImageReviewViewModel imageReviewVM = (ImageReviewViewModel)obj;
            int minTimeBetweenFrames = (int)Math.Floor((Decimal)Constants.MS_TO_SEC / _maxPreviewDisplayFrameRate);

            while (_tIndexThreadRun)
            {
                ImageReviewViewModel._bitmapIsLoading = true;

                _tTimerValue = _tTimerValue % imageReviewVM.TMax + 1;

                imageReviewVM.TValue = _tTimerValue;

                DateTime startTime = DateTime.Now;
                TimeSpan elapsedTime = DateTime.Now - startTime;
                while ((ImageReviewViewModel._bitmapIsLoading || elapsedTime.TotalMilliseconds < minTimeBetweenFrames) && elapsedTime.TotalMilliseconds < TIMEOUT_MILLISECONDS)
                {
                    elapsedTime = DateTime.Now - startTime;
                    Thread.Sleep(2);
                }
            }
        }

        private static void ZIndexThreadProc(Object obj)
        {
            ImageReviewViewModel imageReviewVM = (ImageReviewViewModel)obj;
            int minTimeBetweenFrames = (int)Math.Floor((Decimal)Constants.MS_TO_SEC / _maxPreviewDisplayFrameRate);

            while (_zIndexThreadRun)
            {
                ImageReviewViewModel._bitmapIsLoading = true;

                _zTimerValue = _zTimerValue % imageReviewVM.ZMax + 1;

                imageReviewVM.ZValue = _zTimerValue;

                DateTime startTime = DateTime.Now;
                TimeSpan elapsedTime = DateTime.Now - startTime;
                while ((ImageReviewViewModel._bitmapIsLoading || elapsedTime.TotalMilliseconds < minTimeBetweenFrames) && elapsedTime.TotalMilliseconds < TIMEOUT_MILLISECONDS)
                {
                    elapsedTime = DateTime.Now - startTime;
                    Thread.Sleep(2);
                }
            }
        }

        private void btnColor_Checked(object sender, RoutedEventArgs e)
        {
        }

        private void btnGreyScale_Checked(object sender, RoutedEventArgs e)
        {
        }

        //Called from ThorImage -> Review tab
        private void btnShowMostRecent_Click(object sender, RoutedEventArgs e)
        {
            string expPath = LoadActiveExp();
            MVMManager.Instance["RemoteIPCControlViewModelBase", "ShowMostRecent"] = expPath;
            //load the active experiment last run
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

                            StopZIndexThread();
                            _imageReviewVM.ZIsLive = false;

                            StopTIndexThread();
                            _imageReviewVM.TIsLive = false;

                            StopSPIndexThread();
                            _imageReviewVM.SpIsLive = false;

                            StopStreamIndexThread();
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
                    StopZIndexThread();
                    _imageReviewVM.ZIsLive = false;

                    StopTIndexThread();
                    _imageReviewVM.TIsLive = false;

                    StopSPIndexThread();
                    _imageReviewVM.SpIsLive = false;

                    StopStreamIndexThread();
                    _imageReviewVM.ZStreamIsLive = false;

                    _imageReviewVM.InscribeScaleToImages();
                }
            }
        }

        private string LoadActiveExp()
        {
            string experimentPath = "-1";
            //Open connection to the database
            DataStore.Instance.ConnectionString = "URI=file:" + Application.Current.Resources["ThorDatabase"].ToString();
            DataStore.Instance.Open();

            //Not currently implemented
            //DataStore.Instance.VerifyExperiments();

            //Search through database to ensure that listed experiments exist. If they do, add them to list
            int i = 0;
            List<DatabaseItem> databaseItems = new List<DatabaseItem>();
            foreach (DataRow row in DataStore.Instance.ExperimentsDataSet.Tables[0].Rows)
            {
                if (File.Exists(row["Path"].ToString() + "\\" + "Experiment.xml"))
                {
                    DatabaseItem databaseItem = new DatabaseItem();
                    databaseItem.ID = i;
                    databaseItem.ExpName = row["Name"].ToString();
                    databaseItem.ExpPath = row["Path"].ToString();
                    databaseItems.Add(databaseItem);
                    i++;
                }
            }
            DataStore.Instance.Close();

            if (0 < databaseItems.Count)
            {
                experimentPath = databaseItems[databaseItems.Count - 1].ExpPath;

                _imageReviewVM.ExperimentDoc = _imageReviewVM.LoadDocAsReadOnly(experimentPath + "\\Experiment.xml");

                //Once ExperimentDoc is set, load the view model settings
                _imageReviewVM.LoadViewModelSettingsDoc();

                if (!LoadExperiment(experimentPath))
                {
                    MessageBox.Show("Application failed to load the last experiment. Please choose an experiment using Select Experiment button.", "Experiment Not Loaded", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
            else
            {
                MessageBox.Show("Application failed to load the last experiment. Please choose an experiment using Select Experiment button.", "Experiment Not Loaded", MessageBoxButton.OK, MessageBoxImage.Error);
            }

            return experimentPath + "\\Experiment.xml";
        }

        private void LoadData(string expfile, bool resetSliders)
        {
            if (_imageReviewVM == null)
            {
                return;
            }

            // to speed up load don't allow updates while loading experiment
            _imageReviewVM.AllowImageUpdate = false;

            MVMManager.Instance["ImageViewReviewVM", "ResetBitmap"] = true;
            MVMManager.Instance["ImageViewReviewVM", "StopBitmapBuildingThread"] = true;
            //enabling the main control
            controlContainer.IsEnabled = true;
            XmlDataProvider dataProvider = (XmlDataProvider)this.FindResource("Experiment");

            if (File.Exists(expfile))
            {
                XmlDocument experimentDoc = _imageReviewVM.LoadDocAsReadOnly(expfile);
                ExperimentData.Populate(experimentDoc, _imageReviewVM.HardwareDoc);

                dataProvider.Document = experimentDoc;
                _imageReviewVM.ExperimentDoc = experimentDoc;
            }

            _imageReviewVM.ImageReview.AllocateImageDataMemoryFormROI();

            _imageReviewVM.ImageInfo = ExperimentData.ImageInfo;

            _imageReviewVM.BitsPerPixel = ExperimentData.BitsPerPixel;

            _imageReviewVM.CaptureMode = ExperimentData.CaptureMode;

            _imageReviewVM.SpMax = ExperimentData.SpMax;
            _imageReviewVM.SpMin = 1;

            _imageReviewVM.ZMax = ExperimentData.ZMax;
            _imageReviewVM.ZMin = 1;

            _imageReviewVM.ZStreamMode = ExperimentData.ZStreamMode;
            _imageReviewVM.ZStreamMax = ExperimentData.ZStreamMax;
            _imageReviewVM.ZStreamMin = 1;

            _imageReviewVM.IsRemoteFocus = ExperimentData.IsRemoteFocus;
            _imageReviewVM.PixelAspectRatioYScale = ExperimentData.PixelAspectRatioYScale;
            LoadRemoteFocusPositionValues();

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
            else if (_imageReviewVM.CaptureMode == CaptureModes.STREAMING || _imageReviewVM.CaptureMode == CaptureModes.T_AND_Z
                || _imageReviewVM.CaptureMode == CaptureModes.BLEACHING)
            {
                _imageReviewVM.MStart = _imageReviewVM.TMin;
                /*_imageReviewVM.MEnd = _imageReviewVM.TMax;*/
                _imageReviewVM.MStartValue = _imageReviewVM.TMin;
                /*_imageReviewVM.MEndValue = _imageReviewVM.TMax;*/

                btnT.IsChecked = true;
            }
            else
            {
                //loading the default movie slider parameters for LSM configurationi
                _imageReviewVM.MStart = _imageReviewVM.ZMin;
                /*_imageReviewVM.MEnd = _imageReviewVM.ZMax;*/
                _imageReviewVM.MStartValue = _imageReviewVM.ZMin;
                /*_imageReviewVM.MEndValue = _imageReviewVM.ZMax;*/

                btnZ.IsChecked = true;
            }

            Color wlColor = Color.FromArgb(0xFF, 0xFF, 0xFF, 0xFF);
            _imageReviewVM.WavelengthColor = wlColor;

            _imageReviewVM.UpdateBitmapAndEventSubscribers();

            //load userControl
            if (File.Exists(expfile))
            {
                tileControl.SelectExpFile(expfile);
            }

            OverlayManagerClass.Instance.ClearAllObjects();
            if (ExperimentData.IsmROICapture)
            {
                OverlayManagerClass.Instance.PixelUnitSizeXY = new int[] { (int)ExperimentData.mROIStripLength, 1 };
                mROIDisplay.Visibility = Visibility.Visible;
            }
            else
            {
                mROIDisplay.Visibility = Visibility.Collapsed;
            }

            //If the ROI's file exists, load it.
            //Loading the ROI's will reset the overlay manager which is needed at this point
            string roiPath = Directory.GetParent(expfile).FullName + "\\ROIs.xaml";
            OverlayManagerClass.Instance.UserLoadROIs(roiPath);


            /*OverlayManagerClass.Instance.InitOverlayManagerClass(ExperimentData.ImageInfo.pixelX, ExperimentData.ImageInfo.pixelY, ExperimentData.PixelSizeUM, false);*/

            // At this point allow data update
            _imageReviewVM.AllowImageUpdate = true;
            _imageReviewVM.ExperimentDataLoaded();
            MVMManager.Instance["ImageViewReviewVM", "StartBitmapBuildingThread"] = true;

        }

        /// <summary>
        /// Loads an experiment from file
        /// </summary>
        /// <param name="experimentPath"> The path to the experiment to load </param>
        /// <returns> True if the experiment was successfully loaded </returns>
        private bool LoadExperiment(string experimentPath)
        {
            _imageReviewVM.IsExperimentLoading = true;
            ResetPlayStopButtons();
            if (!String.IsNullOrEmpty(experimentPath))
            {
                _expPath = experimentPath;
                _expFile = _expPath + "\\Experiment.xml";
            }

            if (!File.Exists(_expFile) || (_imageReviewVM == null))
            {
                _imageReviewVM.IsExperimentLoading = false;
                return false;
            }

            bool resetSliders = (_expPath != _imageReviewVM.ExperimentFolderPath) ? true : false;
            if (resetSliders)
            {
                ResetImageSliders();
            }

            _imageReviewVM.ExperimentXMLPath = _expFile;
            _imageReviewVM.ExperimentFolderPath = _expPath;

            XmlDocument experimentDoc = _imageReviewVM.LoadDocAsReadOnly(_expFile);

            XmlNodeList nodeList = experimentDoc.SelectNodes("/ThorImageExperiment/ThorSyncDataPath");
            if (nodeList.Count != 0)
            {
                XmlAttribute thorSyncDataPath = nodeList[0].Attributes[0];
                if (thorSyncDataPath != null && thorSyncDataPath.Value != "" && (bool)MVMManager.Instance["RemoteIPCControlViewModelBase", "RemoteConnection"])
                {
                    MVMManager.Instance["RemoteIPCControlViewModelBase", "ShowMostRecent"] = thorSyncDataPath.Value;
                    //ThorIPCModule.Instance.SendIPCCommand("ShowMostRecent", thorSyncDataPath.Value);
                }
            }

            LoadData(_expFile, resetSliders);
            _imageReviewVM.LoadExpFolder(_expPath);

            if (CaptureModes.HYPERSPECTRAL == ExperimentData.CaptureMode)
            {
                _imageReviewVM.MEnd = _imageReviewVM.SpMax;
                _imageReviewVM.MEndValue = _imageReviewVM.SpMax;

            }
            else if (_imageReviewVM.CaptureMode == CaptureModes.STREAMING || _imageReviewVM.CaptureMode == CaptureModes.T_AND_Z 
                || _imageReviewVM.CaptureMode == CaptureModes.BLEACHING)
            {
                _imageReviewVM.MEnd = _imageReviewVM.TMax;
                _imageReviewVM.MEndValue = _imageReviewVM.TMax;

            } 
            else
            {

                _imageReviewVM.MEnd = _imageReviewVM.ZMax;
                _imageReviewVM.MEndValue = _imageReviewVM.ZMax;
            }



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

            _imageReviewVM.IsExperimentLoading = false;
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

            _imageReviewVM.ReloadSettingsByExpModality();

            this.tileControl.TilesIndexChangedEvent += tileControl_TilesIndexChangedEvent;
            this.tileControl.Loaded += tileControl_Loaded;
            _imageReviewVM.EnableHandlers();

            // Show/hide "Open In ThorAnalysis" button
            string str = string.Empty;
            XmlNode node = _imageReviewVM.ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/Review/OpenInThorAnalysisButton");
            if (null != node && XmlManager.GetAttribute(node, _imageReviewVM.ApplicationDoc, "Visibility", ref str) && (str.EndsWith("Visible")))
            {
                btLoadThorAnalysis.Visibility = Visibility.Visible;
            }
            else
            {
                btLoadThorAnalysis.Visibility = Visibility.Collapsed;
            }

            node = _imageReviewVM.ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/Review/PreviewDisplay");
            if (null != node && XmlManager.GetAttribute(node, _imageReviewVM.ApplicationDoc, "MaxFrameRate", ref str))
            {
                _maxPreviewDisplayFrameRate = Int32.Parse(str);
            }

            _imageReviewVM.StartImageLoadingThread();
            ResourceManagerCS.BackupDirectory(ResourceManagerCS.GetMyDocumentsThorImageFolderString());
        }

        void MasterView_Unloaded(object sender, RoutedEventArgs e)
        {
            tileControl.TilesIndexChangedEvent -= tileControl_TilesIndexChangedEvent;
            tileControl.Loaded -= tileControl_Loaded;

            _imageReviewVM.ReleaseHandlers();
            if (_imageReviewVM.TIsLive)
            {
                StopTIndexThread();
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
                StopSPIndexThread();
                _imageReviewVM.SpIsLive = false;
                _imageReviewVM.ZIsEnabled = true;
                _imageReviewVM.TIsEnabled = true;
                _imageReviewVM.ZStreamIsEnabled = true;
            }

            if (_imageReviewVM.ZIsLive)
            {
                StopZIndexThread();
                _imageReviewVM.ZIsLive = false;
                _imageReviewVM.TIsEnabled = true;
                _imageReviewVM.ZStreamIsEnabled = true;
                _imageReviewVM.SpIsEnabled = true;
            }

            if (_imageReviewVM.ZStreamIsLive)
            {
                StopStreamIndexThread();

                //changed the click image control
                _imageReviewVM.ZStreamIsLive = false;

                //enable T Z back when stream play is finished
                _imageReviewVM.ZIsEnabled = true;
                _imageReviewVM.TIsEnabled = true;
                _imageReviewVM.SpIsEnabled = true;
            }

            _imageReviewVM.StopImageLoadingThread();
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
            // _imageReviewVM.ScanAreaID = _imageReviewVM.ScanAreaIDMin;

            //Reset all timers
            _tTimerValue = 1;
            _zTimerValue = 1;
            _streamTimerValue = 1;
            _spTimerValue = 1;
        }

        private void ResetPlayStopButtons()
        {
            //Reset play/stop buttos
            StopTIndexThread();
            StopZIndexThread();
            StopStreamIndexThread();
            StopSPIndexThread();
            //if (null != _scanAreaTimer) _scanAreaTimer.Stop();
            _imageReviewVM.TIsLive = false;
            _imageReviewVM.ZIsLive = false;
            _imageReviewVM.ZStreamIsLive = false;
            _imageReviewVM.SpIsLive = false;
            //_imageReviewVM.ScanAreaIsLive = false;
            _imageReviewVM.TIsEnabled = true;
            _imageReviewVM.ZIsEnabled = true;
            _imageReviewVM.ZStreamIsEnabled = true;
            _imageReviewVM.SpIsEnabled = true;
            //_imageReviewVM.ScanAreaIsEnabled = true;

            //Reset all timers
            _tTimerValue = 1;
            _zTimerValue = 1;
            _streamTimerValue = 1;
            _spTimerValue = 1;
            // _scanAreaTimerValue = _imageReviewVM.ScanAreaIDMin;

            //Reset sliders to 1 in case the same experiment is being loaded
            _imageReviewVM.TValue = 1;
            _imageReviewVM.ZValue = 1;
            _imageReviewVM.ZStreamValue = 1;
            _imageReviewVM.SpValue = 1;
            //_imageReviewVM.ScanAreaID = _imageReviewVM.ScanAreaIDMin;
        }

        //private void ScanAreaSliderText_LostFocus(object sender, RoutedEventArgs e)
        //{
        //    if (_imageReviewVM == null)
        //        return;
        //    Regex regex = new Regex("^[0-9]*$");
        //    if (scanAreaSliderText.Text != "" && regex.IsMatch(scanAreaSliderText.Text))
        //    {
        //        if ((Int32.Parse(scanAreaSliderText.Text) < _imageReviewVM.ScanAreaIDMin) || (Int32.Parse(spSliderText.Text) > _imageReviewVM.ScanAreaIDMax))
        //        {
        //            MessageBox.Show("Enter values between " + _imageReviewVM.ScanAreaIDMin + " and " + _imageReviewVM.ScanAreaIDMax, "Invalid Range", MessageBoxButton.OK, MessageBoxImage.Error);
        //            scanAreaSliderText.Text = _imageReviewVM.ScanAreaID.ToString();
        //        }
        //    }
        //    else if (scanAreaSliderText.Text == "")
        //    {
        //        //Do nothing, just wait for number to be entered
        //    }
        //    else
        //    {
        //        MessageBox.Show("Enter only numbers", "Invalid Value", MessageBoxButton.OK, MessageBoxImage.Error);
        //        scanAreaSliderText.Text = _imageReviewVM.ScanAreaID.ToString();
        //        return;
        //    }
        //}
        //private void ScanAreaStart_Click(object sender, RoutedEventArgs e)
        //{
        //    if (_imageReviewVM == null)
        //        return;
        //    if (!_imageReviewVM.ScanAreaIsLive)
        //    {
        //        if (_imageReviewVM.ScanAreaIDMax == _imageReviewVM.ScanAreaID)
        //        {
        //            _scanAreaTimerValue = _imageReviewVM.ScanAreaIDMin;
        //        }
        //        else
        //        {
        //            _scanAreaTimerValue = _imageReviewVM.ScanAreaID;
        //        }
        //        _scanAreaTimer.Interval = TimeSpan.FromMilliseconds(Convert.ToDouble(0));
        //        _scanAreaTimer.Start();
        //        //change the click image control
        //        _imageReviewVM.ScanAreaIsLive = true;
        //        _imageReviewVM.TIsEnabled = false;
        //        _imageReviewVM.ZIsEnabled = false;
        //        _imageReviewVM.ZStreamIsEnabled = false;
        //        _imageReviewVM.SpIsEnabled = false;
        //    }
        //    else
        //    {
        //        _scanAreaTimer.Stop();
        //        _imageReviewVM.ScanAreaIsLive = false;
        //        _imageReviewVM.TIsEnabled = true;
        //        _imageReviewVM.ZIsEnabled = true;
        //        _imageReviewVM.ZStreamIsEnabled = true;
        //        _imageReviewVM.SpIsEnabled = true;
        //    }
        //}
        //void scanAreaTimer_Tick(object sender, EventArgs e)
        //{
        //    if (_imageReviewVM == null)
        //        return;
        //    _imageReviewVM.ScanAreaID = _scanAreaTimerValue;
        //    if (_scanAreaTimerValue < _imageReviewVM.ScanAreaIDMax)
        //    {
        //        _scanAreaTimerValue++;
        //    }
        //    else
        //    {
        //        _scanAreaTimerValue = _imageReviewVM.ScanAreaIDMin;
        //    }
        //}
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

                _spIndexThreadRun = true;
                _spIndexLive = new Thread(SPIndexThreadProc);
                _spIndexLive.Start(_imageReviewVM);

                //change the click image control
                _imageReviewVM.SpIsLive = true;
                _imageReviewVM.TIsEnabled = false;
                _imageReviewVM.ZStreamIsEnabled = false;
                _imageReviewVM.ZIsEnabled = false;
            }
            else
            {
                ImageReviewViewModel._stopRequested = true;
                StopSPIndexThread();
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

        private void StopSPIndexThread()
        {
            if (_spIndexThreadRun)
            {
                _spIndexThreadRun = false;
                _spIndexLive.Join();
                ImageReviewViewModel._stopRequested = false;
            }
        }

        private void StopStreamIndexThread()
        {
            if (_streamIndexThreadRun)
            {
                _streamIndexThreadRun = false;
                _streamIndexLive.Join();
                ImageReviewViewModel._stopRequested = false;
            }
        }

        private void StopTIndexThread()
        {
            if (_tIndexThreadRun)
            {
                _tIndexThreadRun = false;
                _tIndexLive.Join();
                ImageReviewViewModel._stopRequested = false;
            }
        }

        private void StopZIndexThread()
        {
            if (_zIndexThreadRun)
            {
                _zIndexThreadRun = false;
                _zIndexLive.Join();
                ImageReviewViewModel._stopRequested = false;
            }
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

                _streamIndexThreadRun = true;
                _streamIndexLive = new Thread(StreamIndexThreadProc);
                _streamIndexLive.Start(_imageReviewVM);

                //change the click image control
                _imageReviewVM.ZStreamIsLive = true;

                //disable T Z while stream is playing
                _imageReviewVM.ZIsEnabled = false;
                _imageReviewVM.TIsEnabled = false;
                _imageReviewVM.SpIsEnabled = false;
            }
            else
            {
                ImageReviewViewModel._stopRequested = true;
                StopStreamIndexThread();
                //changed the click image control
                _imageReviewVM.ZStreamIsLive = false;
                //enable T Z back when stream play is finished
                _imageReviewVM.ZIsEnabled = true;
                _imageReviewVM.TIsEnabled = true;
                _imageReviewVM.SpIsEnabled = true;
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

                _tIndexThreadRun = true;
                _tIndexLive = new Thread(TIndexThreadProc);
                _imageReviewVM.RXFlag = false;
                _tIndexLive.Start(_imageReviewVM);

                //change the click image control
                _imageReviewVM.TIsLive = true;

                _imageReviewVM.ZIsEnabled = false;
                _imageReviewVM.ZStreamIsEnabled = false;
                _imageReviewVM.SpIsEnabled = false;
            }
            else
            {
                ImageReviewViewModel._stopRequested = true;
                StopTIndexThread();
                _imageReviewVM.RXFlag = true;
                _imageReviewVM.TIsLive = false;
                _imageReviewVM.ZIsEnabled = true;
                _imageReviewVM.ZStreamIsEnabled = true;
                _imageReviewVM.SpIsEnabled = true;
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

                _zIndexThreadRun = true;
                _zIndexLive = new Thread(ZIndexThreadProc);
                _imageReviewVM.RXFlagZ = false;
                _zIndexLive.Start(_imageReviewVM);

                //change the click image control
                _imageReviewVM.ZIsLive = true;
                _imageReviewVM.TIsEnabled = false;
                _imageReviewVM.ZStreamIsEnabled = false;
                _imageReviewVM.SpIsEnabled = false;
            }
            else
            {
                ImageReviewViewModel._stopRequested = true;
                StopZIndexThread();
                _imageReviewVM.RXFlagZ = true;
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

        #endregion Methods
    }
}