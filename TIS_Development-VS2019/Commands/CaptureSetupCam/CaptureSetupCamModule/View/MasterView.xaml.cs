namespace CaptureSetupDll.View
{
    using System;
    using System.Collections.Generic;
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
    using System.Windows.Media;
    using System.Xml;

    using CaptureSetupDll.View;
    using CaptureSetupDll.ViewModel;

    using FolderDialogControl;

    using HelpProvider;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Win32;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for MasterView.xaml
    /// </summary>
    public partial class MasterView : UserControl
    {
        #region Fields

        private string _appSettingsFile;
        object _dataContextliveImageVM = null;
        private string _hwSettingsFile;
        LiveImageViewModel _liveVM = null;
        private TwoWaySettings _twoWayDialog = new TwoWaySettings();

        #endregion Fields

        #region Constructors

        public MasterView()
        {
            InitializeComponent();
            this.Loaded += new RoutedEventHandler(MasterView_Loaded);

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");

            //retrieve the hardware settings complete path and file name
            string hwSettings = Application.Current.Resources["HardwareSettingsFile"].ToString();

            _hwSettingsFile = hwSettings;

            if (!File.Exists(_hwSettingsFile))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " Unable to load hardware setttings");
            }

            //retrieve the application settings complete path and file name
            string appSettings = Application.Current.Resources["ApplicationSettingsFile"].ToString();

            _appSettingsFile = appSettings;

            if (!File.Exists(_appSettingsFile))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " Unable to load application setttings");
            }

            this.Unloaded += new RoutedEventHandler(MasterView_Unloaded);
        }

        #endregion Constructors

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

                //setting initial position and size of the positionRectangle of the canvas.
                _liveVM = (LiveImageViewModel)_dataContextliveImageVM;

            }
        }

        #endregion Properties

        #region Methods

        public void OnLoadExperiment(object sender, RoutedEventArgs e)
        {
            //stopping the live capture
            _liveVM.LiveImageStart(false);

            string expFile = null;

            BrowseForFolderDialog dlg = new BrowseForFolderDialog();
            dlg.Title = "Select the Experiment Directory";
            dlg.InitialExpandedFolder = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\ThorImageLS";
            dlg.OKButtonText = "OK";

            if (true == dlg.ShowDialog())
            {
                expFile = dlg.SelectedFolder + "\\Experiment.xml";
            }

            if (File.Exists(expFile))
            {
                _liveVM.ExperimentDoc = new XmlDocument();

                _liveVM.ExperimentDoc.Load(expFile);

                XmlNodeList ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/ZStage");

                if (ndList.Count > 0)
                {
                    int numSteps = 0;
                    double scanStep = 0;
                    string str = string.Empty;

                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "startPos", ref str))
                    {
                        _liveVM.ZScanStart = Convert.ToDouble(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "steps", ref str))
                    {
                        numSteps = Convert.ToInt32(str);
                    }

                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "stepSizeUM", ref str))
                    {
                        scanStep = Convert.ToDouble(str);

                        //display step as an unsigned value
                        _liveVM.ZScanStep = Math.Abs(scanStep);
                    }
                    const double UM_TO_MM = .001;
                    _liveVM.ZScanStop = _liveVM.ZScanStart + numSteps * scanStep * UM_TO_MM;
                }

                ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/LSM");

                if (ndList.Count > 0)
                {
                    string str = string.Empty;

                    //set the scan mode first to ensure the pixel slider has the correct range
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "scanMode", ref str))
                    {
                        _liveVM.LSMScanMode = Convert.ToInt32(str);
                    }
                    //if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pixelX", ref str))
                    //{
                    //    _liveVM.LSMPixelX = Convert.ToInt32(str);
                    //}
                    string temp = string.Empty;

                    loadPixelCountXY(ndList, ref str, ref temp);

                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "inputRange1", ref str))
                    {
                        _liveVM.InputRangeChannel1 = Convert.ToInt32(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "inputRange2", ref str))
                    {
                        _liveVM.InputRangeChannel2 = Convert.ToInt32(str);
                    }

                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "inputRange3", ref str))
                    {
                        _liveVM.InputRangeChannel3 = Convert.ToInt32(str);
                    }

                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "inputRange4", ref str))
                    {
                        _liveVM.InputRangeChannel4 = Convert.ToInt32(str);
                    }

                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "fieldSize", ref str))
                    {
                        _liveVM.LSMFieldSize = Convert.ToInt32(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "channel", ref str))
                    {
                        int val = Convert.ToInt32(str);
                        selectChannelUsingConverter(val);
                    }

                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "offsetX", ref str))
                    {
                        _liveVM.LSMFieldOffsetXActual = Convert.ToInt32(Convert.ToDouble(str)); // handle the case that floating number passed in
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "offsetY", ref str))
                    {
                        _liveVM.LSMFieldOffsetYActual = Convert.ToInt32(Convert.ToDouble(str)); // handle the case that floating number passed in
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "averageMode", ref str))
                    {
                        _liveVM.LSMSignalAverage = Convert.ToInt32(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "averageNum", ref str))
                    {
                        _liveVM.LSMSignalAverageFrames = Convert.ToInt32(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "twoWayAlignment", ref str))
                    {
                        _liveVM.LSMTwoWayAlignment = Convert.ToInt32(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "clockSource", ref str))
                    {
                        _liveVM.LSMClockSource = Convert.ToInt32(str) - 1;
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "extClockRate", ref str))
                    {
                        _liveVM.LSMExtClockRate = Convert.ToInt32(str) / 1000000;
                    }
                }

                ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/PMT");

                if (ndList.Count > 0)
                {
                    string str = string.Empty;

                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "gainA", ref str))
                    {
                        _liveVM.PMT1Gain = Convert.ToInt32(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "gainB", ref str))
                    {
                        _liveVM.PMT2Gain = Convert.ToInt32(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "gainC", ref str))
                    {
                        _liveVM.PMT3Gain = Convert.ToInt32(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "gainD", ref str))
                    {
                        _liveVM.PMT4Gain = Convert.ToInt32(str);
                    }
                }

                ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/PowerRegulator");

                if (ndList.Count > 0)
                {
                    string str = string.Empty;
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "start", ref str))
                    {
                        _liveVM.PowerStart = Convert.ToInt32(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "stop", ref str))
                    {
                        _liveVM.PowerStop = Convert.ToInt32(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "type", ref str))
                    {
                        _liveVM.PowerMode = Convert.ToInt32(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pockelsBlankPercentage", ref str))
                    {
                        _liveVM.PowerPockelsBlankPercentage = Convert.ToInt32(str);
                    }
                }

                ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/MCLS");

                loadMCLSSettings(ndList);   // use the same method as when loading the MasterView

                ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/ImageCorrection");

                if (ndList.Count > 0)
                {
                    string str = string.Empty;

                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "enablePincushion", ref str))
                    {
                        _liveVM.EnablePincushionCorrection = Convert.ToInt32(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pinCoeff1", ref str))
                    {
                        _liveVM.Coeff1 = Convert.ToDouble(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pinCoeff2", ref str))
                    {
                        _liveVM.Coeff2 = Convert.ToDouble(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pinCoeff3", ref str))
                    {
                        _liveVM.Coeff3 = Convert.ToDouble(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "enableBackgroundSubtraction", ref str))
                    {
                        _liveVM.EnableBackgroundSubtraction = Convert.ToInt32(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pathBackgroundSubtraction", ref str))
                    {
                        _liveVM.PathBackgroundSubtraction = str;
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "enableFlatField", ref str))
                    {
                        _liveVM.EnableFlatField = Convert.ToInt32(str);
                    }
                    if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pathFlatField", ref str))
                    {
                        _liveVM.PathFlatField = str;
                    }
                }

                ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/Magnification");

                if (ndList.Count > 0)
                {
                    for (int i = 0; i < magComboBox.Items.Count; i++)
                    {
                        string str = magComboBox.Items[i].ToString();
                        str = str.Remove(str.Length - 1, 1);// remove trailing 'X', by Ming

                        string strMag = string.Empty;

                        if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "mag", ref strMag))
                        {
                            if (str.Equals(strMag))
                            {
                                _liveVM.TurretPosition = i;
                            }
                        }
                    }
                }

                ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/PinholeWheel");

                loadPinholeLocation(ndList);

                ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/Wavelengths/Wavelength");

                LoadCameraExposureTime(ndList);

                ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/Camera");

                LoadCameraSettings(ndList);
            }
            else
            {
                MessageBox.Show("Experiment does not exist. Choose a different folder", "No Experiment", MessageBoxButton.OK, MessageBoxImage.Information);
            }
        }

        public void OnSaveExperiment(object sender, RoutedEventArgs e)
        {
        }

        public void SetPath(string pathIn)
        {
            path.Text = pathIn;
            //path.GetBindingExpression(TextBox.TextProperty).UpdateSource();

            //XmlDataProvider dataProvider = (XmlDataProvider)this.FindResource("Experiment");

            XmlDocument experimentDoc = new XmlDocument();

            if (File.Exists(path.Text))
            {
                experimentDoc.Load(path.Text);
            }
            else
            {
                string templatesFolder = Application.Current.Resources["TemplatesFolder"].ToString();
                //setting the previous run experiment xml file inorder to load the xmldataprovider
                path.Text = templatesFolder + "\\temp.xml";

                experimentDoc.Load(path.Text);
            }

            //dataProvider.Document = experimentDoc;

            if (_liveVM != null)
            {
                _liveVM.ExperimentDoc = experimentDoc;
            }
        }

        public void UpdateHardwareListView()
        {
            XmlDataProvider dataProvider = (XmlDataProvider)this.FindResource("HardwareSettings");

            XmlDocument hwSettingsDoc = new XmlDocument();

            hwSettingsDoc.Load(_hwSettingsFile);

            dataProvider.Document = hwSettingsDoc;

            using (dataProvider.DeferRefresh())
            {
                dataProvider.Document.Save(_hwSettingsFile);
            }
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "Execute")]
        private static extern int CaptureSetupExecute();

        void AFScanStart_Update(string scanstart, string focusoffset)
        {
            XmlNodeList ndList = _liveVM.HardwareDoc.DocumentElement["Objectives"].GetElementsByTagName("Objective");

            //turret position is 1 based index
            int objectiveIndex = _liveVM.TurretPosition;

            if ((objectiveIndex > 0) && (objectiveIndex < ndList.Count))
            {
                ndList.Item(objectiveIndex).Attributes["afScanStartMM"].Value = scanstart;
                ndList.Item(objectiveIndex).Attributes["afFocusOffsetMM"].Value = focusoffset;
            }

            _liveVM.HardwareDoc.Save(_hwSettingsFile);
        }

        private void areaControlView_FieldSizeChanged(object sender, EventArgs e)
        {
            try
            {
                if (_liveVM != null)
                {
                    _twoWayDialog.SelectedIndex = Convert.ToInt32((_liveVM.LSMFieldSize));
                }
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
            }
        }

        void AutoExposure_Update(double exposure)
        {
        }

        void AutoFocus_Update(bool val)
        {
            XmlNodeList ndList = _liveVM.HardwareDoc.DocumentElement["Objectives"].GetElementsByTagName("Objective");

            double currentPosition = _liveVM.ZPosition;

            if (_liveVM.TurretPosition < ndList.Count)
            {
                _liveVM.ZPosition = Convert.ToDouble(ndList.Item(_liveVM.TurretPosition).Attributes["afScanStartMM"].Value);
            }

            if (false == _liveVM.AutoFocus())
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " Autofocus failed returning to previous Z position");

                _liveVM.ZPosition = currentPosition;
            }

            _liveVM.HardwareDoc.Save(_hwSettingsFile);
        }

        private void BinningComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            switch (((ComboBox)sender).SelectedIndex)
            {
                case 0:
                    _liveVM.BinX = 1;
                    _liveVM.BinY = 1;
                    break;
                case 1:
                    _liveVM.BinX = 2;
                    _liveVM.BinY = 2;
                    break;
                case 2:
                    _liveVM.BinX = 4;
                    _liveVM.BinY = 4;
                    break;
                case 3:
                    _liveVM.BinX = 8;
                    _liveVM.BinY = 8;
                    break;
            }
        }

        private void Browse_Click(object sender, RoutedEventArgs e)
        {
            BrowseForFolderDialog dlg = new BrowseForFolderDialog();
            dlg.Title = "Select the Streaming Directory";
            dlg.InitialExpandedFolder = Application.Current.Resources["AppRootFolder"].ToString();
            dlg.OKButtonText = "OK";

            if (true == dlg.ShowDialog())
            {
                _liveVM.PathSaveSnapshot = dlg.SelectedFolder;
            }
        }

        private void Button_Click_Add_Wavelength(object sender, RoutedEventArgs e)
        {
        }

        private void Button_Click_Remove_Wavelength(object sender, RoutedEventArgs e)
        {
        }

        private void butTwoWaySetup_Click(object sender, RoutedEventArgs e)
        {
            if (_twoWayDialog.IsLoaded == false)
            {
                //stop the current capture
                _liveVM.StopCommand.Execute(null);

                _liveVM.LSMTwoWayAlignment = 0;

                _twoWayDialog = new TwoWaySettings();
                _twoWayDialog.Closing += new System.ComponentModel.CancelEventHandler(_twoWayDialog_Closing);
                _twoWayDialog.Show();
                _twoWayDialog.CurrentOffset = _liveVM.LSMTwoWayAlignment;
                _twoWayDialog.SelectedIndex = _liveVM.LSMFieldSize;
            }
        }

        private void cbGain_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            _liveVM.Gain = ((ComboBox)sender).SelectedIndex;
        }

        private void cbLightMode_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            _liveVM.LightMode = ((ComboBox)sender).SelectedIndex;
        }

        private void colorImageSettings_Click(object sender, RoutedEventArgs e)
        {
            LoadColorImageSettings();

            SnapshotSettings dlg = new SnapshotSettings();

            XmlNodeList listRed = _liveVM.HardwareDoc.DocumentElement["ColorChannels"].GetElementsByTagName("Red");

            dlg.RedChannel = listRed[0].Attributes["name"].Value.ToString();

            XmlNodeList listGreen = _liveVM.HardwareDoc.DocumentElement["ColorChannels"].GetElementsByTagName("Green");

            dlg.GreenChannel = listGreen[0].Attributes["name"].Value.ToString();

            XmlNodeList listBlue = _liveVM.HardwareDoc.DocumentElement["ColorChannels"].GetElementsByTagName("Blue");

            dlg.BlueChannel = listBlue[0].Attributes["name"].Value.ToString();

            XmlNodeList listCyan = _liveVM.HardwareDoc.DocumentElement["ColorChannels"].GetElementsByTagName("Cyan");

            dlg.CyanChannel = listCyan[0].Attributes["name"].Value.ToString();

            XmlNodeList listMagenta = _liveVM.HardwareDoc.DocumentElement["ColorChannels"].GetElementsByTagName("Magenta");

            dlg.MagentaChannel = listMagenta[0].Attributes["name"].Value.ToString();

            XmlNodeList listYellow = _liveVM.HardwareDoc.DocumentElement["ColorChannels"].GetElementsByTagName("Yellow");

            dlg.YellowChannel = listYellow[0].Attributes["name"].Value.ToString();

            XmlNodeList listGray = _liveVM.HardwareDoc.DocumentElement["ColorChannels"].GetElementsByTagName("Gray");

            dlg.GrayChannel = listGray[0].Attributes["name"].Value.ToString();

            XmlNodeList ndList = _liveVM.HardwareDoc.GetElementsByTagName("Wavelength");

            for (int i = 0; i < ndList.Count; i++)
            {
                dlg.comboRed.Items.Add(ndList[i].Attributes["name"]);
                dlg.comboGreen.Items.Add(ndList[i].Attributes["name"]);
                dlg.comboBlue.Items.Add(ndList[i].Attributes["name"]);
                dlg.comboCyan.Items.Add(ndList[i].Attributes["name"]);
                dlg.comboMagenta.Items.Add(ndList[i].Attributes["name"]);
                dlg.comboYellow.Items.Add(ndList[i].Attributes["name"]);
                dlg.comboGray.Items.Add(ndList[i].Attributes["name"]);
            }

            dlg.comboRed.Items.Add("None");
            dlg.comboGreen.Items.Add("None");
            dlg.comboBlue.Items.Add("None");
            dlg.comboCyan.Items.Add("None");
            dlg.comboMagenta.Items.Add("None");
            dlg.comboYellow.Items.Add("None");
            dlg.comboGray.Items.Add("None");

            //Select "None" by default for all the combo boxes
            dlg.comboRed.SelectedIndex = ndList.Count;
            dlg.comboGreen.SelectedIndex = ndList.Count;
            dlg.comboBlue.SelectedIndex = ndList.Count;
            dlg.comboCyan.SelectedIndex = ndList.Count;
            dlg.comboMagenta.SelectedIndex = ndList.Count;
            dlg.comboYellow.SelectedIndex = ndList.Count;
            dlg.comboGray.SelectedIndex = ndList.Count;

            for (int i = 0; i < ndList.Count; i++)
            {
                if (ndList[i].Attributes["name"].Value.Equals(dlg.RedChannel))
                {
                    dlg.comboRed.SelectedIndex = i;
                }
                if (ndList[i].Attributes["name"].Value.Equals(dlg.GreenChannel))
                {
                    dlg.comboGreen.SelectedIndex = i;
                };

                if (ndList[i].Attributes["name"].Value.Equals(dlg.BlueChannel))
                {
                    dlg.comboBlue.SelectedIndex = i;
                }

                if (ndList[i].Attributes["name"].Value.Equals(dlg.CyanChannel))
                {
                    dlg.comboCyan.SelectedIndex = i;
                }

                if (ndList[i].Attributes["name"].Value.Equals(dlg.MagentaChannel))
                {
                    dlg.comboMagenta.SelectedIndex = i;
                }

                if (ndList[i].Attributes["name"].Value.Equals(dlg.YellowChannel))
                {
                    dlg.comboYellow.SelectedIndex = i;
                }

                if (ndList[i].Attributes["name"].Value.Equals(dlg.GrayChannel))
                {
                    dlg.comboGray.SelectedIndex = i;
                }
            }

            if (true == dlg.ShowDialog())
            {
                listRed[0].Attributes["name"].Value = dlg.RedChannel;
                listGreen[0].Attributes["name"].Value = dlg.GreenChannel;
                listBlue[0].Attributes["name"].Value = dlg.BlueChannel;
                listCyan[0].Attributes["name"].Value = dlg.CyanChannel;
                listMagenta[0].Attributes["name"].Value = dlg.MagentaChannel;
                listYellow[0].Attributes["name"].Value = dlg.YellowChannel;
                listGray[0].Attributes["name"].Value = dlg.GrayChannel;

                XmlNodeList wavelengths = _liveVM.HardwareDoc.GetElementsByTagName("Wavelength");

                const string RED_COLORREF = "#FFFF0000";
                const string GREEN_COLORREF = "#FF00FF00";
                const string BLUE_COLORREF = "#FF0000FF";
                const string CYAN_COLORREF = "#FF00FFFF";
                const string MAGENTA_COLORREF = "#FFFF00FF";
                const string YELLOW_COLORREF = "#FFFFFF00";
                const string GRAY_COLORREF = "#FFFFFFFF";

                if (wavelengths.Count > 0)
                {
                    Brush brush = new SolidColorBrush(Colors.Gray);

                    for (int i = 0; i < wavelengths.Count; i++)
                    {
                        if (dlg.RedChannel.Equals(wavelengths[i].Attributes["name"].Value.ToString()))
                        {
                            wavelengths[i].Attributes["color"].Value = RED_COLORREF;
                        }
                        if (dlg.GreenChannel.Equals(wavelengths[i].Attributes["name"].Value.ToString()))
                        {
                            wavelengths[i].Attributes["color"].Value = GREEN_COLORREF;
                        }
                        if (dlg.BlueChannel.Equals(wavelengths[i].Attributes["name"].Value.ToString()))
                        {
                            wavelengths[i].Attributes["color"].Value = BLUE_COLORREF;
                        }
                        if (dlg.CyanChannel.Equals(wavelengths[i].Attributes["name"].Value.ToString()))
                        {
                            wavelengths[i].Attributes["color"].Value = CYAN_COLORREF;
                        }
                        if (dlg.MagentaChannel.Equals(wavelengths[i].Attributes["name"].Value.ToString()))
                        {
                            wavelengths[i].Attributes["color"].Value = MAGENTA_COLORREF;
                        }
                        if (dlg.YellowChannel.Equals(wavelengths[i].Attributes["name"].Value.ToString()))
                        {
                            wavelengths[i].Attributes["color"].Value = YELLOW_COLORREF;
                        }
                        if (dlg.GrayChannel.Equals(wavelengths[i].Attributes["name"].Value.ToString()))
                        {
                            wavelengths[i].Attributes["color"].Value = GRAY_COLORREF;
                        }
                    }

                }
                _liveVM.HardwareDoc.Save(_hwSettingsFile);

                LoadColorImageSettings();
            }
        }

        private void Expander_Expanded(object sender, RoutedEventArgs e)
        {
            bool bExpand = false;
            object obj = Application.Current.Resources["ApplicationSettingsFile"];

            if (obj != null)
            {
                string appSettings = obj.ToString();

                XmlDocument doc = new XmlDocument();

                doc.Load(appSettings);

                XmlNodeList ndList = doc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/MultiPanelView");
                if (ndList.Count > 0)
                {
                    if (ndList[0].Attributes["Visibility"].Value.Equals("Visible"))
                        bExpand = true;
                }
            }

            if (bExpand)
                return;
        }

        private void Exposure_Update(double exposureTime)
        {
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

        private string GetInputRangeString(int val)
        {
            string str = string.Empty;

            switch (_liveVM.DigitizerBoardName)
            {
                case CaptureSetupDll.Model.LiveImage.DigitizerBoardNames.ATS9440:
                    {
                        //names for the ATS9440 board
                        switch (val)
                        {
                            case 1: str = "100mV"; break;
                            case 2: str = "200mV"; break;
                            case 3: str = "400mV"; break;
                            case 4: str = "1V"; break;
                            case 5: str = "2V"; break;
                            case 6: str = "4V"; break;
                        }
                    }
                    break;

                default:
                    {
                        //names for the ATS460 board
                        switch (val)
                        {
                            case 1: str = "40mV"; break;
                            case 2: str = "50mV"; break;
                            case 3: str = "80mV"; break;
                            case 4: str = "100mV"; break;
                            case 5: str = "200mV"; break;
                            case 6: str = "400mV"; break;
                            case 7: str = "500mV"; break;
                            case 8: str = "800mV"; break;
                            case 9: str = "1V"; break;
                            case 10: str = "2V"; break;
                            case 11: str = "4V"; break;
                        }
                    }
                    break;

            }

            return str;
        }

        private void LoadActiveCameraID()
        {
            XmlNodeList ndList = this._liveVM.HardwareDoc.SelectNodes("/HardwareSettings/ImageDetectors/Camera");
            foreach (XmlNode node in ndList)
            {
                string active = string.Empty;
                GetAttribute(node, _liveVM.HardwareDoc, "active", ref active);
                if (active.Equals("1"))
                {
                    string id = string.Empty;
                    GetAttribute(node, _liveVM.HardwareDoc, "id", ref id);
                    _liveVM.ActiveCameraID = Convert.ToInt32(id);
                }
            }
        }

        private void LoadCameraExposureTime(XmlNodeList ndList)
        {
            if (ndList.Count > 0)
            {
                string temp = string.Empty;
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "exposureTimeMS", ref temp))
                {
                    _liveVM.ExposureTimeCam0 = Double.Parse(temp);
                }
            }
        }

        private void LoadCameraSettings(XmlNodeList ndList)
        {
            double pixelSize = 0;

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "left", ref str))
                {
                    _liveVM.Left = Int32.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "right", ref str))
                {
                    _liveVM.Right = Int32.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "top", ref str))
                {
                    _liveVM.Top = Int32.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "bottom", ref str))
                {
                    _liveVM.Bottom = Int32.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "width", ref str))
                {
                    _liveVM.Width = Int32.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "height", ref str))
                {
                    _liveVM.Height = Int32.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pixelSizeUM", ref str))
                {
                    pixelSize = Double.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "binningX", ref str))
                {
                    _liveVM.BinX = Int32.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "binningY", ref str))
                {
                    _liveVM.BinY = Int32.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "gain", ref str))
                {
                    _liveVM.Gain = Int32.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "lightmode", ref str))
                {
                    _liveVM.LightMode = Int32.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "blacklevel", ref str))
                {
                    _liveVM.OpticalBlackLevel = Int32.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "tdiWidth", ref str))
                {
                    _liveVM.TDIWidthMM = Double.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "tdiHeight", ref str))
                {
                    _liveVM.TDIHeightMM = Double.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "readoutSpeedIndex", ref str))
                {
                    _liveVM.ReadOutSpeedIndex = Int32.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "readoutTapIndex", ref str))
                {
                    _liveVM.ReadOutTapIndex = Int32.Parse(str);
                }
                else
                {
                    _liveVM.ReadOutTapIndex = 0;
                }
            }
        }

        private void LoadColorImageSettings()
        {
            XmlNodeList listRed = _liveVM.HardwareDoc.DocumentElement["ColorChannels"].GetElementsByTagName("Red");
            XmlNodeList listGreen = _liveVM.HardwareDoc.DocumentElement["ColorChannels"].GetElementsByTagName("Green");
            XmlNodeList listBlue = _liveVM.HardwareDoc.DocumentElement["ColorChannels"].GetElementsByTagName("Blue");
            XmlNodeList listCyan = _liveVM.HardwareDoc.DocumentElement["ColorChannels"].GetElementsByTagName("Cyan");
            XmlNodeList listMagenta = _liveVM.HardwareDoc.DocumentElement["ColorChannels"].GetElementsByTagName("Magenta");
            XmlNodeList listYellow = _liveVM.HardwareDoc.DocumentElement["ColorChannels"].GetElementsByTagName("Yellow");
            XmlNodeList listGray = _liveVM.HardwareDoc.DocumentElement["ColorChannels"].GetElementsByTagName("Gray");

            XmlNodeList wavelengths = _liveVM.HardwareDoc.GetElementsByTagName("Wavelength");

            if (wavelengths.Count > 0)
            {
                Brush brush = new SolidColorBrush(Colors.Gray);

                XmlNodeList ndList;
                string str = string.Empty;
                string wlString = string.Empty;
                for (int i = 0; i < Math.Min(wavelengths.Count, _liveVM.ColorChannels); i++)
                {
                    if (GetAttribute(wavelengths[i], _liveVM.HardwareDoc, "name", ref wlString))
                    {
                        ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/ColorChannels/Red");

                        if (ndList.Count > 0)
                        {
                            if (GetAttribute(ndList[0], _liveVM.HardwareDoc, "name", ref str))
                            {
                                if (str.Equals(wlString))
                                {
                                    _liveVM.SetColorAssignment(i, 0);
                                    brush = new SolidColorBrush(Colors.Red);
                                }
                            }
                        }

                        ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/ColorChannels/Green");

                        if (ndList.Count > 0)
                        {
                            if (GetAttribute(ndList[0], _liveVM.HardwareDoc, "name", ref str))
                            {
                                if (str.Equals(wlString))
                                {
                                    _liveVM.SetColorAssignment(i, 1);
                                    brush = new SolidColorBrush(Colors.Green);
                                }
                            }
                        }
                        ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/ColorChannels/Blue");

                        if (ndList.Count > 0)
                        {
                            if (GetAttribute(ndList[0], _liveVM.HardwareDoc, "name", ref str))
                            {
                                if (str.Equals(wlString))
                                {
                                    _liveVM.SetColorAssignment(i, 2);
                                    brush = new SolidColorBrush(Colors.Blue);
                                }
                            }
                        }
                        ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/ColorChannels/Cyan");

                        if (ndList.Count > 0)
                        {
                            if (GetAttribute(ndList[0], _liveVM.HardwareDoc, "name", ref str))
                            {
                                if (str.Equals(wlString))
                                {
                                    _liveVM.SetColorAssignment(i, 3);
                                    brush = new SolidColorBrush(Colors.Cyan);
                                }
                            }
                        }
                        ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/ColorChannels/Magenta");

                        if (ndList.Count > 0)
                        {
                            if (GetAttribute(ndList[0], _liveVM.HardwareDoc, "name", ref str))
                            {
                                if (str.Equals(wlString))
                                {
                                    _liveVM.SetColorAssignment(i, 4);
                                    brush = new SolidColorBrush(Colors.Magenta);
                                }
                            }
                        }
                        ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/ColorChannels/Yellow");

                        if (ndList.Count > 0)
                        {
                            if (GetAttribute(ndList[0], _liveVM.HardwareDoc, "name", ref str))
                            {
                                if (str.Equals(wlString))
                                {
                                    _liveVM.SetColorAssignment(i, 5);
                                    brush = new SolidColorBrush(Colors.Yellow);
                                }
                            }
                        }
                        ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/ColorChannels/Gray");

                        if (ndList.Count > 0)
                        {
                            if (GetAttribute(ndList[0], _liveVM.HardwareDoc, "name", ref str))
                            {
                                if (str.Equals(wlString))
                                {
                                    _liveVM.SetColorAssignment(i, 6);
                                    brush = new SolidColorBrush(Colors.Gray);
                                }
                            }
                        }
                    }
                }
            }
        }

        private void loadMCLSSettings(XmlNodeList ndList)
        {
            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "enable1", ref str))
                {
                    _liveVM.Laser1Enable = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "power1", ref str))
                {
                    _liveVM.Laser1Power = Math.Min(_liveVM.Laser1Max, Math.Max(_liveVM.Laser1Min, Convert.ToDouble(str)));
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "enable2", ref str))
                {
                    _liveVM.Laser2Enable = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "power2", ref str))
                {
                    _liveVM.Laser2Power = Math.Min(_liveVM.Laser2Max, Math.Max(_liveVM.Laser2Min, Convert.ToDouble(str)));
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "enable3", ref str))
                {
                    _liveVM.Laser3Enable = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "power3", ref str))
                {
                    _liveVM.Laser3Power = Math.Min(_liveVM.Laser3Max, Math.Max(_liveVM.Laser3Min, Convert.ToDouble(str)));
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "enable4", ref str))
                {
                    _liveVM.Laser4Enable = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "power4", ref str))
                {
                    _liveVM.Laser4Power = Math.Min(_liveVM.Laser4Max, Math.Max(_liveVM.Laser4Min, Convert.ToDouble(str)));
                }
            }
        }

        private void loadPinholeLocation(XmlNodeList ndList)
        {
            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "position", ref str))
                {
                    _liveVM.PinholePosition = Convert.ToInt32(str);
                }
            }
        }

        private void loadPixelCountXY(XmlNodeList ndList, ref string str, ref string temp)
        {
            if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pixelX", ref str) &&
                GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pixelY", ref temp))
            {
                int x = Convert.ToInt32(str);
                int y = Convert.ToInt32(temp);
                if (y == 1 && x > 1)
                    _liveVM.LSMAreaMode = (int)CaptureSetupDll.Model.LiveImage.AreaMode.LINE;
                else if (x == y)
                    _liveVM.LSMAreaMode = (int)CaptureSetupDll.Model.LiveImage.AreaMode.SQUARE;
                else
                    _liveVM.LSMAreaMode = (int)CaptureSetupDll.Model.LiveImage.AreaMode.RECTANGLE;
                _liveVM.LSMPixelX = x;
                _liveVM.LSMPixelY = y;
            }
        }

        private void MagComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            int location = ((ComboBox)e.Source).SelectedIndex;
        }

        void MasterView_Loaded(object sender, RoutedEventArgs e)
        {
            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this) == true)
                return;

            XmlDocument hwSettingsDoc = new XmlDocument();
            //FocusManager.SetFocusedElement(
            hwSettingsDoc.Load(_hwSettingsFile);

            _liveVM.HardwareDoc = hwSettingsDoc;

            //loading the active experiment settings to the ExperimentDoc Xmlprovider
            string templatesFolder = Application.Current.Resources["TemplatesFolder"].ToString();
            string strActiveExp = templatesFolder + "\\Active.xml";

            XmlDocument expSettingsDoc = new XmlDocument();
            expSettingsDoc.Load(strActiveExp);

            _liveVM.ExperimentDoc = expSettingsDoc;

            XmlDocument appSettingsDoc = new XmlDocument();
            appSettingsDoc.Load(_appSettingsFile);

            _liveVM.ApplicationDoc = appSettingsDoc;

            //set default save options for Snapshot
            _liveVM.PathSaveSnapshot = Application.Current.Resources["AppRootFolder"].ToString();
            _liveVM.SnapshotImagePrefix = "Image";

            //Load and adjust tems from application settings
            SetDisplayOptions();

            XmlNodeList ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/ZStage");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                double scanStep = 0;
                int numSteps = 0;
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "startPos", ref str))
                {
                    _liveVM.ZScanStart = Convert.ToDouble(str);
                }

                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "steps", ref str))
                {
                    numSteps = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "stepSizeUM", ref str))
                {
                    scanStep = Convert.ToDouble(str);
                }
                //display step as an unsigned value
                _liveVM.ZScanStep = Math.Abs(scanStep);
                const double UM_TO_MM = .001;
                _liveVM.ZScanStop = _liveVM.ZScanStart + numSteps * scanStep * UM_TO_MM;
            }

            //set visibility of controls based on number of wavelengths available to the system
            XmlNodeList ndWLList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/Wavelength");

            _liveVM.NumChannelsAvailableForDisplay = ndWLList.Count;

            //set input range based on number of channels available
            switch (_liveVM.NumChannelsAvailableForDisplay)
            {
                case 4: { _liveVM.DigitizerBoardName = Model.LiveImage.DigitizerBoardNames.ATS9440; _liveVM.InputRangeMin = 1; _liveVM.InputRangeMax = 6; } break;
                default: { _liveVM.DigitizerBoardName = Model.LiveImage.DigitizerBoardNames.ATS460; _liveVM.InputRangeMin = 1; _liveVM.InputRangeMax = 11; } break;
            }

            ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/ImageDetectors/Camera");

            //if a camera is not in the list of available cameras
            //disable the panel
            masterPanel.IsEnabled = (0 == ndList.Count)?false:true;

            ndList = hwSettingsDoc.SelectNodes("/HardwareSettings/LSM");

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if (GetAttribute(ndList[0], hwSettingsDoc, "fieldSizeCalibration", ref str))
                {
                    _liveVM.LSMFieldSizeCalibration = Convert.ToDouble(str);
                    //_liveVM.LSMFieldSizeCalibration = Convert.ToDouble(ndList[0].Attributes["fieldSizeCalibration"].Value);
                }
                if (GetAttribute(ndList[0], hwSettingsDoc, "pixelXLimit", ref str))
                {
                    _liveVM.LSMPixelXMax = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], hwSettingsDoc, "pixelYLimit", ref str))
                {
                    _liveVM.LSMPixelYMax = Convert.ToInt32(str);
                }
            }

            ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/LSM");

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                //set the scan mode first to ensure the pixel slider has the correct range

                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "scanMode", ref str))
                {
                    _liveVM.LSMScanMode = Convert.ToInt32(str);
                }
                //if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pixelX", ref str))
                //{
                //    _liveVM.LSMPixelX = Convert.ToInt32(str);
                //}
                //if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pixelY", ref str))
                //{
                //    _liveVM.LSMPixelY = Convert.ToInt32(str);
                //}
                string temp = string.Empty;
                loadPixelCountXY(ndList, ref str, ref temp);

                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "inputRange1", ref str))
                {
                    _liveVM.InputRangeChannel1 = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "inputRange2", ref str))
                {
                    _liveVM.InputRangeChannel2 = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "inputRange3", ref str))
                {
                    _liveVM.InputRangeChannel3 = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "inputRange4", ref str))
                {
                    _liveVM.InputRangeChannel4 = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "fieldSize", ref str))
                {
                    _liveVM.LSMFieldSize = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "channel", ref str))
                {
                    int val = Convert.ToInt32(str);

                    selectChannelUsingConverter(val);   // This method is extracted because will also be used in onLoadExperiment()
                }

                const int MAX_GUI_CHANNELS = 4;

                for (int i = 0; i < MAX_GUI_CHANNELS; i++)
                {
                    if (i < ndWLList.Count)
                    {
                        _liveVM.IsChannelVisible[i] = Visibility.Visible;
                    }
                    else
                    {
                        _liveVM.IsChannelVisible[i] = Visibility.Collapsed;
                        _liveVM.LSMChannelEnable[i] = false;
                    }
                }

                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "offsetX", ref str))
                {
                    _liveVM.LSMFieldOffsetXActual = Convert.ToInt32(Convert.ToDouble(str));// handle the case that floating number passed in
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "offsetY", ref str))
                {
                    _liveVM.LSMFieldOffsetYActual = Convert.ToInt32(Convert.ToDouble(str));// handle the case that floating number passed in
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "averageMode", ref str))
                {
                    _liveVM.LSMSignalAverage = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "averageNum", ref str))
                {
                    _liveVM.LSMSignalAverageFrames = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "twoWayAlignment", ref str))
                {
                    _liveVM.LSMTwoWayAlignment = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "clockSource", ref str))
                {
                    _liveVM.LSMClockSource = Convert.ToInt32(str) - 1;
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "extClockRate", ref str))
                {
                    _liveVM.LSMExtClockRate = Convert.ToInt32(str) / 1000000;
                }
            }

            ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/PMT");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "gainA", ref str))
                {
                    _liveVM.PMT1Gain = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "gainB", ref str))
                {
                    _liveVM.PMT2Gain = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "gainC", ref str))
                {
                    _liveVM.PMT3Gain = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "gainD", ref str))
                {
                    _liveVM.PMT4Gain = Convert.ToInt32(str);
                }
            }

            ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/PowerRegulator");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "start", ref str))
                {
                    _liveVM.PowerStart = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "stop", ref str))
                {
                    _liveVM.PowerStop = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "type", ref str))
                {
                    _liveVM.PowerMode = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pockelsBlankPercentage", ref str))
                {
                    _liveVM.PowerPockelsBlankPercentage = Convert.ToInt32(str);
                }
            }

            ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/ImageCorrection");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "enablePincushion", ref str))
                {
                    _liveVM.EnablePincushionCorrection = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pinCoeff1", ref str))
                {
                    _liveVM.Coeff1 = Convert.ToDouble(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pinCoeff2", ref str))
                {
                    _liveVM.Coeff2 = Convert.ToDouble(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pinCoeff3", ref str))
                {
                    _liveVM.Coeff3 = Convert.ToDouble(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "enableBackgroundSubtraction", ref str))
                {
                    _liveVM.EnableBackgroundSubtraction = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pathBackgroundSubtraction", ref str))
                {
                    _liveVM.PathBackgroundSubtraction = str;
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "enableFlatField", ref str))
                {
                    _liveVM.EnableFlatField = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pathFlatField", ref str))
                {
                    _liveVM.PathFlatField = str;
                }
            }

            ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/MCLS");

            loadMCLSSettings(ndList);

            ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/MultiPhotonLaser");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "pos", ref str))
                {
                    _liveVM.Laser1Position = Convert.ToInt32(str);
                }
            }

            ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/PinholeWheel");

            loadPinholeLocation(ndList);

            if (_liveVM.IsStoredCameraInfoValid)
            {
                ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/Wavelengths/Wavelength");

                LoadCameraExposureTime(ndList);

                ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/Camera");

                LoadCameraSettings(ndList);
            }
            else
            {
                _liveVM.UpdateUIWithCameraParameters();
            }

            LoadActiveCameraID();

            double magnification = 1;

            ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/Magnification");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "mag", ref str))
                {
                    magnification = Double.Parse(str);
                }
            }

            //load the magnification combo box using the hardware settings document
            XmlDataProvider dataProvider = (XmlDataProvider)this.FindResource("HardwareSettings");

            dataProvider.Document = hwSettingsDoc;

            XmlNodeList ndListObj = hwSettingsDoc.DocumentElement["Objectives"].GetElementsByTagName("Objective");

            magComboBox.Items.Clear();

            foreach (XmlElement element in ndListObj)
            {
                magComboBox.Items.Add(element.GetAttribute("name").ToString());
            }

            //set the active magnifiation for the combo box
            ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/Magnification");

            if (ndList.Count > 0)
            {
                int i = 0;
                foreach (XmlElement element in ndListObj)
                {
                    string str = element.GetAttribute("mag").ToString();
                    if (str.Equals(ndList[0].Attributes["mag"].Value))
                    {
                        _liveVM.TurretPosition = i;
                        break;
                    }
                    i++;
                }
            }

            ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/Sample");

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "type", ref str))
                {
                    _liveVM.SelectedSampleType = Convert.ToInt32(str);

                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "offsetXMM", ref str))
                {
                    _liveVM.SampleOffsetXMM = Convert.ToDouble(str);

                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "offsetYMM", ref str))
                {
                    _liveVM.SampleOffsetYMM = Convert.ToDouble(str);
                }
            }

            ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/Sample/Wells");

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "startRow", ref str))
                {
                    _liveVM.StartRow = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "startColumn", ref str))
                {
                    _liveVM.StartColumn = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "wellOffsetXMM", ref str))
                {
                    _liveVM.WellOffsetXMM = Double.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "wellOffsetYMM", ref str))
                {
                    _liveVM.WellOffsetYMM = Double.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "rows", ref str))
                {
                    _liveVM.SelectedWellRowCount = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "columns", ref str))
                {
                    _liveVM.SelectedWellColumnCount = Convert.ToInt32(str);
                }
            }

            ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/Sample/Wells/SubImages");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "transOffsetXMM", ref str))
                {
                    _liveVM.TransOffsetXMM = Double.Parse(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "transOffsetYMM", ref str))
                {
                    _liveVM.TransOffsetYMM = Double.Parse(str);
                }

                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "subRows", ref str))
                {
                    _liveVM.SubRows = Convert.ToInt32(str);
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "subColumns", ref str))
                {
                    _liveVM.SubColumns = Convert.ToInt32(str);
                }

                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "subOffsetXMM", ref str))
                {
                    if (0 != _liveVM.LSMPixelX * _liveVM.MMPerPixel)
                    {
                        _liveVM.SubOffsetX = _liveVM.LSMPixelX * _liveVM.MMPerPixel;
                        double val = Convert.ToDouble(str) / _liveVM.SubOffsetX;
                        _liveVM.SubSpacingXPercent = 100.0 * Convert.ToDouble(Decimal.Round(Convert.ToDecimal(val - 1.0), 2));
                    }
                }
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "subOffsetYMM", ref str))
                {
                    if (0 != _liveVM.LSMPixelX * _liveVM.MMPerPixel)
                    {
                        _liveVM.SubOffsetY = _liveVM.LSMPixelY * _liveVM.MMPerPixel;
                        double val = Convert.ToDouble(str) / _liveVM.SubOffsetY;
                        _liveVM.SubSpacingYPercent = 100.0 * Convert.ToDouble(Decimal.Round(Convert.ToDecimal(val - 1.0), 2));
                    }
                }
            }

            ndList = _liveVM.ExperimentDoc.SelectNodes("/ThorImageExperiment/Sample");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (GetAttribute(ndList[0], _liveVM.ExperimentDoc, "type", ref str))
                {
                    _liveVM.SelectedSampleType = Convert.ToInt32(str);
                }
            }
            //Default the tiles control mode to Top/Left
            _liveVM.TileControlMode = 0;

            //loading the default color image settings
            LoadColorImageSettings();

            //_liveVM.LoadCameraParameters();

            _liveVM.ConnectHandlers();
        }

        void MasterView_Unloaded(object sender, RoutedEventArgs e)
        {
            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this) == true)
                return;

            _liveVM.PersistData();

            //update the active experiment for the experiment manager
            CaptureSetupExecute();

            //stopping the live capture
            _liveVM.LiveImageStart(false);

            _twoWayDialog.Close();

            _liveVM.ReleaseHandlers();
        }

        private void Pseudocolor_Click(object sender, RoutedEventArgs e)
        {
        }

        private void SampleComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
        }

        private void SampleOffset_Update(double sampleOffsetXMM, double sampleOffsetYMM)
        {
        }

        private void selectChannelUsingConverter(int val)
        {
            _liveVM.LSMChannelEnable[0] = true;
            _liveVM.LSMChannelEnable[1] = true;
            _liveVM.LSMChannelEnable[2] = true;
            _liveVM.LSMChannelEnable[3] = true;

            switch (_liveVM.DigitizerBoardName)
            {
                case Model.LiveImage.DigitizerBoardNames.ATS9440:
                    {
                        switch (val)
                        {
                            case 0x1: _liveVM.LSMChannel = 0; break;
                            case 0x2: _liveVM.LSMChannel = 1; break;
                            case 0x4: _liveVM.LSMChannel = 2; break;
                            case 0x8: _liveVM.LSMChannel = 3; break;
                            default:
                                {
                                    _liveVM.LSMChannel = 4;
                                    _liveVM.LSMChannelEnable[0] = Convert.ToBoolean(val & 0x1);
                                    _liveVM.LSMChannelEnable[1] = Convert.ToBoolean(val & 0x2);
                                    _liveVM.LSMChannelEnable[2] = Convert.ToBoolean(val & 0x4);
                                    _liveVM.LSMChannelEnable[3] = Convert.ToBoolean(val & 0x8);
                                }
                                break;
                        }
                    }
                    break;
                case Model.LiveImage.DigitizerBoardNames.ATS460:
                    {
                        switch (val)
                        {
                            case 0x1: _liveVM.LSMChannel = 0; break;
                            case 0x2: _liveVM.LSMChannel = 1; break;
                            default:
                                {
                                    _liveVM.LSMChannel = 2;
                                    _liveVM.LSMChannelEnable[0] = Convert.ToBoolean(val & 0x1);
                                    _liveVM.LSMChannelEnable[1] = Convert.ToBoolean(val & 0x2);
                                    _liveVM.LSMChannelEnable[2] = false;
                                    _liveVM.LSMChannelEnable[3] = false;
                                }
                                break;
                        }
                    }
                    break;
            }
        }

        private void SetDisplayOptions()
        {
            try
            {
                XmlNodeList ndList;

                //configure visibility of the Magnification ComboBox
                ndList = _liveVM.ApplicationDoc.SelectNodes("ApplicationSettings/DisplayOptions/CaptureSetup/MagnificationView");

                if (ndList.Count > 0)
                {
                    spMagnification.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }
                else
                {
                    spMagnification.Visibility = Visibility.Collapsed;
                }

                //configure visibility of the TDI control panel
                tdiBorder.Visibility = Visibility.Collapsed;

                ndList = _liveVM.ApplicationDoc.SelectNodes("ApplicationSettings/DisplayOptions/CaptureSetup/TDIView");

                if (ndList.Count > 0)
                {
                    if (ndList[0].Attributes["Visibility"].Value.Equals("Visible") && _liveVM.IsDecoderPresent)
                    {
                        tdiBorder.Visibility = Visibility.Visible;
                    }
                }

                //configure the visibility of the XY control panel
                xyBorder.Visibility = Visibility.Collapsed;

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/XYView");
                if (ndList.Count > 0)
                {
                    if (ndList[0].Attributes["Visibility"].Value.Equals("Visible") && _liveVM.IsXYStagePresent)
                    {
                        xyBorder.Visibility = Visibility.Visible;
                    }
                }
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
            }
        }

        private void Shutter_Click(object sender, RoutedEventArgs e)
        {
        }

        private void txtTwoWay_TextChanged(object sender, TextChangedEventArgs e)
        {
            try
            {
                if (_liveVM != null)
                {
                    _twoWayDialog.CurrentOffset = Convert.ToInt32(((TextBox)sender).Text);
                }
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
            }
        }

        private void UserControl_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                e.Handled = true;
            }
        }

        void _twoWayDialog_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (_liveVM.IsLive && (false == e.Cancel))
            {
                _liveVM.StopCommand.Execute(null);
                _liveVM.LSMTwoWayAlignment = 0;
            }
        }

        #endregion Methods
    }
}