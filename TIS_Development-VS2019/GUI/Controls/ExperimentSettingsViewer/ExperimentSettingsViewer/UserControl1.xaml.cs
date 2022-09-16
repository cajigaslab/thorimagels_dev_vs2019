namespace ExperimentSettingsViewer
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Collections.Specialized;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Threading;
    using System.Xml;
    using System.Xml.Linq;

    using FolderDialogControl;

    using Microsoft.Win32;

    using SetScriptPath;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class UserControl1 : UserControl, INotifyPropertyChanged
    {
        #region Fields

        const string CMD_EXPERIMENT = "Command/Experiment";
        const string CMD_EXPERIMENT_ATTRIBUTE = "value";
        const string CMD_OUTPUTPATH = "Command/OutputPath";
        const string CMD_OUTPUTPATH_ATTRIBUTE = "value";
        const double UM_TO_MM = 0.001;

        private string _activeCameraName = string.Empty;
        private int _averageMode;
        private int _cameraType;
        private ObservableCollection<LightPathSequenceStep> _captureSequence = new ObservableCollection<LightPathSequenceStep>();
        private bool _captureSequenceEnable = false;
        private bool _changeSettingsEnable;
        private bool _editEnable;
        private string _experimentName;
        private string _exposurePath;
        private double _hsBandwithMode;
        private bool _isCSType = false;
        private int _laser1Wavelength;
        private int _laser2Wavelength;
        private int _laser3Wavelength;
        private int _laser4Wavelength;
        private int _laserAllAnalog;
        private int _laserAllTTL;
        private string _outputPath;
        private string _pockels1RampFileName = string.Empty;
        private string _pockels2RampFileName = string.Empty;
        private string _pockels3RampFileName = string.Empty;
        private string _pockels4RampFileName = string.Empty;
        private string _powerRegRampFileName = string.Empty;
        private int _readoutSpeedIndex;
        private int _readoutTapIndex;
        private XmlDocument _settingsDocument = null;
        private bool _settingsDocUpdated = false;
        private int _streamVolumes;
        private string _strTemplates;
        private bool _tEnable;
        XmlDocument _variableDoc;
        private string _variableFile;
        private double _z2Pos;
        private bool _zEnable;
        private double _zStartPosition;
        private int _zSteps;
        private double _zStepSize;
        private double _zStopPosition;

        #endregion Fields

        #region Constructors

        public UserControl1()
        {
            InitializeComponent();

            this.DataContext = this;
            this.Loaded += new RoutedEventHandler(UserControl1_Loaded);
            this.Unloaded += new RoutedEventHandler(UserControl1_Unloaded);
            zStackGroup.Visibility = Visibility.Visible;
            timeGroup.Visibility = Visibility.Visible;
            streamGroup.Visibility = Visibility.Collapsed;
            bleachingGroup.Visibility = Visibility.Collapsed;
            hyperspectralGroup.Visibility = Visibility.Collapsed;

            PockelsRampFileName = new ObservableCollection<StringPC>();

            const int MAX_POCKELS_COUNT = 4;
            for (int i = 0; i < MAX_POCKELS_COUNT; i++)
            {
                PockelsRampFileName.Add(new StringPC());
            }

            _zStartPosition = 0;
            _zSteps = 0;
            _zStepSize = 0;
            _zStopPosition = 0;
            _z2Pos = 0;
            _editEnable = true;
            _zEnable = false; ;
            _tEnable = false;
            _changeSettingsEnable = false;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public String ApplicationSettingPath
        {
            get
            {
                return GetApplicationSettingsFileString();
            }
        }

        public string AverageMode
        {
            get
            {
                if (0 == _averageMode)
                {
                    return "None";
                }
                else if (1 == _averageMode)
                {
                    return "Cumulative";
                }
                return "";
            }
        }

        public int CameraType
        {
            get
            {
                return _cameraType; //0 for CCD or 1 for LSM
            }
        }

        public bool CaptureSequenceEnable
        {
            get
            {
                return _captureSequenceEnable;
            }
            set
            {
                _captureSequenceEnable = value;
                OnPropertyChanged("CaptureSequenceEnable");
            }
        }

        public ObservableCollection<LightPathSequenceStep> CollectionCaptureSequence
        {
            get
            {
                return _captureSequence;
            }

            set
            {
                _captureSequence = value;
                OnPropertyChanged("CollectionCaptureSequence");
            }
        }

        public bool EditEnable
        {
            get
            {
                return _editEnable;
            }
            set
            {
                _editEnable = value;
                OnPropertyChanged("EditEnable");
            }
        }

        public string ExperimentName
        {
            get
            {
                return _experimentName;
            }
            set
            {
                _experimentName = value;

                try
                {
                    XmlDataProvider provider = (XmlDataProvider)FindResource("expData");
                    provider.Source = new Uri(_experimentName);
                    provider.Refresh();

                    //clear data grid:
                    dgBleach.Columns.Clear();
                    dgBleach.Items.Clear();

                    //determine bleach/slm ROIs based on files:
                    string bleachName = Path.GetDirectoryName(_experimentName) + "\\" + "BleachROIs.xaml";
                    string slmName = Path.GetDirectoryName(_experimentName) + "\\" + "SLMWaveforms";
                    string roiName = Path.GetDirectoryName(_experimentName) + "\\" + "ROIs.xaml";

                    if (File.Exists(bleachName))
                    {
                        lbBleach.Content = "Bleach ROI Settings";
                        PopulateBleachROIDataGrid(bleachName);
                    }
                    else if (Directory.Exists(slmName) && File.Exists(roiName))
                    {
                        lbBleach.Content = "SLM ROI Settings";
                        PopulateSLMROIDataGrid(_experimentName, roiName);
                    }
                }
                catch (Exception ex)
                {
                    string str = ex.Message;
                }

                SetCommandParameter(_settingsDocument, CMD_EXPERIMENT, CMD_EXPERIMENT_ATTRIBUTE, _experimentName);
                tileControl.SelectExpFile(ExperimentName);
                OnPropertyChanged("ExperimentName");
            }
        }

        public string ExposureTable
        {
            get
            {
                int start = 0, stop = 0, i = 0;
                string exposureTable = String.Empty;
                if (null != _exposurePath)
                {
                    foreach (char c in _exposurePath)
                    {
                        if ('\\' == c)
                        {
                            start = i;
                        }
                        else if ('.' == c)
                        {
                            stop = i;
                        }
                        i++;
                    }
                    start++;
                    exposureTable = _exposurePath.Substring(start, stop - start);
                }
                return exposureTable;
            }
        }

        public Visibility GainBLVis
        {
            get
            {
                if (_activeCameraName.Contains("CS2100"))
                {
                    return Visibility.Collapsed;
                }
                return Visibility.Visible;
            }
        }

        public string HSBandwithMode
        {
            get
            {
                if (2 == _hsBandwithMode)
                {
                    return "WIDE";
                }
                else if (4 == _hsBandwithMode)
                {
                    return "MEDIUM";
                }
                else if (8 == _hsBandwithMode)
                {
                    return "NARROW";
                }
                return "";
            }
        }

        public bool isCSType
        {
            get
            {
                return _isCSType;
            }
            set
            {
                _isCSType = value;
            }
        }

        public string OutputPath
        {
            get
            {
                return _outputPath;
            }
            set
            {
                _outputPath = value;

                SetCommandParameter(_settingsDocument, CMD_OUTPUTPATH, CMD_OUTPUTPATH_ATTRIBUTE, _outputPath);

                OnPropertyChanged("OutputPath");
            }
        }

        public ObservableCollection<StringPC> PockelsRampFileName
        {
            get;
            set;
        }

        public string PowerRegRampFileName
        {
            get
            {
                return _powerRegRampFileName;
            }
            set
            {
                _powerRegRampFileName = value;
                OnPropertyChanged("PowerRegRampFileName");
            }
        }

        public string ReadoutSpeed
        {
            get
            {
                if (0 == _readoutSpeedIndex)
                {
                    if (isCSType)
                    {
                        return "60 MS/S";
                    }
                    else
                    {
                        return "20 MHz";
                    }
                }
                else if (1 == _readoutSpeedIndex)
                {
                    if (isCSType)
                    {
                        return "100 MS/S";
                    }
                    else
                    {
                        return "40 MHz";
                    }
                }
                return "";
            }
        }

        public Visibility ReadoutSpeedVis
        {
            get
            {
                if (!isCSType || _activeCameraName.Contains("CS2100"))
                {
                    return Visibility.Visible;
                }
                return Visibility.Collapsed;
            }
        }

        public XmlDocument SettingsDocument
        {
            get
            {
                return _settingsDocument;
            }
            set
            {
                _settingsDocument = value;

                //extract the path to the experiment file
                //and the path for the output
                if (null != _settingsDocument)
                {
                    XmlNode node = _settingsDocument.SelectSingleNode(CMD_EXPERIMENT);

                    if (null != node)
                    {
                        string str = string.Empty;

                        if (XmlManager.GetAttribute(node, _settingsDocument, CMD_EXPERIMENT_ATTRIBUTE, ref str))
                        {
                            ExperimentName = str;
                        }
                    }

                    node = _settingsDocument.SelectSingleNode(CMD_OUTPUTPATH);

                    if (null != node)
                    {
                        string str = string.Empty;

                        if (XmlManager.GetAttribute(node, _settingsDocument, CMD_OUTPUTPATH_ATTRIBUTE, ref str))
                        {
                            OutputPath = str;
                        }

                    }
                }
                if (!EditEnable)
                {
                    LoadExperimentOptions();
                }
                LoadPowerSettings();
                LoadCaptureSequence();
                _settingsDocUpdated = true;
            }
        }

        public int StreamVolumes
        {
            get
            {
                return _streamVolumes;
            }
            set
            {
                _streamVolumes = value;
            }
        }

        public string Taps
        {
            get
            {
                if (0 == _readoutTapIndex)
                {
                    return "1";
                }
                else if (1 == _readoutTapIndex)
                {
                    return "2";
                }
                else if (2 == _readoutTapIndex)
                {
                    return "4";
                }
                return "";
            }
        }

        public Visibility TapsVis
        {
            get
            {
                if (isCSType)
                {
                    return Visibility.Collapsed;
                }
                return Visibility.Visible;
            }
        }

        public bool TEnable
        {
            get
            {
                return _tEnable;
            }
            set
            {
                _tEnable = value;
                OnPropertyChanged("TEnable");
            }
        }

        public double Z2Position
        {
            get
            {
                return Math.Round(_z2Pos * 1000.0, 1);
            }
        }

        public bool ZEnable
        {
            get
            {
                return _zEnable;
            }
            set
            {
                _zEnable = value;
                OnPropertyChanged("ZEnable");
            }
        }

        public double ZStartPosition
        {
            get
            {
                return _zStartPosition * 1000.0;
            }
            set
            {
                _zStartPosition = (value / 1000.0);
                UpdateZSteps();
            }
        }

        public int ZSteps
        {
            get
            {
                return _zSteps;
            }
            set
            {
                _zSteps = value;
                OnPropertyChanged("ZSteps");
            }
        }

        public double ZStepSize
        {
            get
            {
                return _zStepSize;
            }
            set
            {
                _zStepSize = value;
                UpdateZSteps();
            }
        }

        public double ZStopPosition
        {
            get
            {
                return _zStopPosition * 1000;
            }
            set
            {
                _zStopPosition = (value / 1000.0);
                UpdateZSteps();
            }
        }

        #endregion Properties

        #region Methods

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetApplicationSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        public static extern int GetApplicationSettingsFilePathAndName(StringBuilder sb, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetHardwareSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        public static extern int GetHardwareSettingsFilePathAndName(StringBuilder sb, int length);

        public static XElement RemoveAllNamespaces(XElement e)
        {
            return new XElement(e.Name.LocalName,
              (from n in e.Nodes()
               select ((n is XElement) ? RemoveAllNamespaces(n as XElement) : n)),
                  (e.HasAttributes) ?
                    (from a in e.Attributes()
                     where (!a.IsNamespaceDeclaration)
                     select new XAttribute(a.Name.LocalName, a.Value)) : null);
        }

        public string GetApplicationSettingsFileString()
        {
            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetApplicationSettingsFilePathAndName(sb, PATH_LENGTH);

            return sb.ToString();
        }

        public string GetHardwareSettingsFileString()
        {
            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetHardwareSettingsFilePathAndName(sb, PATH_LENGTH);

            return sb.ToString();
        }

        public void LoadPowerSettings()
        {
            try
            {
                if (!File.Exists(ExperimentName)) return;
                XmlDocument xDoc = new XmlDocument();
                xDoc.Load(ExperimentName);
                if (null != xDoc)
                {
                    XmlNodeList ndList = xDoc.SelectNodes("/ThorImageExperiment/Pockels");
                    string str = string.Empty;

                    for (int i = 0; i < ndList.Count; i++)
                    {
                        str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[i], xDoc, "path", ref str))
                        {
                            PockelsRampFileName[i].Value = Path.GetFileNameWithoutExtension(str);
                        }
                    }

                    ndList = xDoc.SelectNodes("/ThorImageExperiment/PowerRegulator");
                    if (0 < ndList.Count)
                    {
                        if (XmlManager.GetAttribute(ndList[0], xDoc, "path", ref str))
                        {
                            PowerRegRampFileName = Path.GetFileNameWithoutExtension(str);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        private void btnPathSetup_Click(object sender, RoutedEventArgs e)
        {
            SetupPathVariable dlg = new SetupPathVariable();

            dlg.ShowDialog();

            LoadPathList();
        }

        private void btnTemplateSetup_Click(object sender, RoutedEventArgs e)
        {
            string dllName = ".\\Modules\\ExperimentSettingsBrowser.dll";
            string panelClassName = "ExperimentSettingsBrowserWindow";

            System.Reflection.Assembly pluginAssembly = System.Reflection.Assembly.LoadFile(Path.GetFullPath(dllName));
            string moduleName = Path.GetFileNameWithoutExtension(dllName);
            Type ucType = pluginAssembly.GetType(moduleName + "." + panelClassName);
            object uc = Activator.CreateInstance(ucType);

            MethodInfo methodInfo = ucType.GetMethod("ShowDialog");
            object[] paramter = new object[0];
            var result = methodInfo.Invoke(uc, paramter);

            try
            {
                if (true == (bool?)result)
                {

                    PropertyInfo propInfoExpSettPath = ucType.GetProperty("ExperimentSettingsPath");
                    var strOriginalName = propInfoExpSettPath.GetValue(uc, null);

                    string strOriginalNameNoExt = System.IO.Path.GetFileNameWithoutExtension(strOriginalName.ToString());

                    do
                    {
                        TemplateName dlgTemplate = new TemplateName();

                        dlgTemplate.NewName = strOriginalNameNoExt;

                        if (false == dlgTemplate.ShowDialog())
                        {
                            break;   //cancel button hits this
                        }

                        string strResult = _strTemplates + "\\" + dlgTemplate.NewName + ".xml";

                        if (false == File.Exists(strResult))
                        {
                            //make a copy of the file with the new name into the template directory
                            File.Copy(strOriginalName.ToString(), strResult);

                            string expFolder = Path.GetDirectoryName(strOriginalName.ToString());
                            string expRoisXAML = expFolder + "\\ROIs.xaml";
                            string expBleachingROIsXAML = expFolder + "\\BleachROIs.xaml";
                            string expBleachingWaveFormH5 = expFolder + "\\BleachWaveform.h5";

                            string expTemplatesFldr = Path.GetDirectoryName(strResult);
                            string templateName = Path.GetFileNameWithoutExtension(strResult);
                            string pathTemplateROIsXAML = expTemplatesFldr + "\\" + templateName + "\\ROIs.xaml";
                            string pathTemplateBleachingROIsXAML = expTemplatesFldr + "\\" + templateName + "\\BleachROIs.xaml";
                            string pathTemplateBleachingWaveFormH5 = expTemplatesFldr + "\\" + templateName + "\\BleachWaveform.h5";
                            Directory.CreateDirectory(expTemplatesFldr + "\\" + templateName);
                            if (File.Exists(strOriginalName.ToString()))
                            {
                                File.Copy(strOriginalName.ToString(), strResult, true);
                            }
                            if (File.Exists(expRoisXAML))
                            {
                                File.Copy(expRoisXAML, pathTemplateROIsXAML, true);
                            }
                            if (File.Exists(expBleachingROIsXAML))
                            {
                                File.Copy(expBleachingROIsXAML, pathTemplateBleachingROIsXAML, true);
                            }
                            if (File.Exists(expBleachingWaveFormH5))
                            {
                                File.Copy(expBleachingWaveFormH5, pathTemplateBleachingWaveFormH5, true);
                            }

                            break;
                        }
                        else
                        {
                            MessageBox.Show("Template name already exists! The new template will require a unique name.");
                        }
                    }
                    while (true);
                    LoadTemplateList();
                }
            }
            catch { }
        }

        private void ButtonOutput_Click(object sender, RoutedEventArgs e)
        {
            BrowseForFolderDialog dlg = new BrowseForFolderDialog();

            Nullable<bool> result = dlg.ShowDialog();

            if (true == result)
            {
                OutputPath = dlg.SelectedFolder;
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();

            dlg.FileName = "Experiment";
            dlg.DefaultExt = ".xml";
            dlg.Filter = "ThorImageExperiment (.xml)|*.xml";

            Nullable<bool> result = dlg.ShowDialog();

            if (true == result)
            {
                ExperimentName = dlg.FileName;
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
            switch (cbCaptureMode.SelectedIndex)
            {
                case 0:
                    zStackGroup.Visibility = Visibility.Visible;
                    timeGroup.Visibility = Visibility.Visible;
                    streamGroup.Visibility = Visibility.Collapsed;
                    bleachingGroup.Visibility = Visibility.Collapsed;
                    hyperspectralGroup.Visibility = Visibility.Collapsed;
                    LoadExperimentOptions();
                    break;
                case 1:
                    zStackGroup.Visibility = Visibility.Collapsed;
                    timeGroup.Visibility = Visibility.Collapsed;
                    streamGroup.Visibility = Visibility.Visible;
                    bleachingGroup.Visibility = Visibility.Collapsed;
                    hyperspectralGroup.Visibility = Visibility.Collapsed;
                    LoadExperimentOptions();
                    break;
                case 2:
                    zStackGroup.Visibility = Visibility.Collapsed;
                    timeGroup.Visibility = Visibility.Collapsed;
                    streamGroup.Visibility = Visibility.Collapsed;
                    bleachingGroup.Visibility = Visibility.Collapsed;
                    hyperspectralGroup.Visibility = Visibility.Collapsed;
                    break;
                case 3:
                    zStackGroup.Visibility = Visibility.Collapsed;
                    timeGroup.Visibility = Visibility.Collapsed;
                    streamGroup.Visibility = Visibility.Collapsed;
                    bleachingGroup.Visibility = Visibility.Visible;
                    hyperspectralGroup.Visibility = Visibility.Collapsed;
                    break;
                case 4:
                    zStackGroup.Visibility = Visibility.Collapsed;
                    timeGroup.Visibility = Visibility.Collapsed;
                    streamGroup.Visibility = Visibility.Collapsed;
                    bleachingGroup.Visibility = Visibility.Collapsed;
                    hyperspectralGroup.Visibility = Visibility.Visible;
                    break;
            }
        }

        private void cbOutputPath_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if ((cbOutputPath.SelectedIndex >= 0) && (cbOutputPath.SelectedIndex < cbOutputPath.Items.Count))
            {
                OutputPath = cbOutputPath.Items[cbOutputPath.SelectedIndex].ToString();
            }
        }

        private void cbTemplate_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if ((cbTemplate.SelectedIndex >= 0) && (cbTemplate.SelectedIndex < cbTemplate.Items.Count))
            {
                string strResult = _strTemplates + "\\" + cbTemplate.Items[cbTemplate.SelectedIndex].ToString() + ".xml";
                cbCaptureMode.SelectedIndex = -1;
                ExperimentName = strResult;

                if (EditEnable)
                {
                    LoadExperimentOptions();
                }
            }
        }

        private void LoadCaptureSequence()
        {
            try
            {
                if (!File.Exists(ExperimentName)) return;
                XmlDocument xDoc = new XmlDocument();
                xDoc.Load(ExperimentName);
                if (null != xDoc)
                {
                    XmlNodeList ndList = xDoc.SelectNodes("/ThorImageExperiment/CaptureSequence");
                    if (ndList.Count > 0)
                    {
                        string str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], xDoc, "enable", ref str))
                        {
                            this.CaptureSequenceEnable = ("1" == str) ? true : false;
                        }
                        else
                        {
                            this.CaptureSequenceEnable = false;
                        }
                    }

                    ndList = xDoc.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep");
                    this.CollectionCaptureSequence = new ObservableCollection<LightPathSequenceStep>();

                    if (true == _captureSequenceEnable && ndList.Count > 0)
                    {
                        for (int i = 0; i < ndList.Count; i++)
                        {
                            string str = string.Empty;
                            if (XmlManager.GetAttribute(ndList[i], xDoc, "name", ref str))
                            {
                                //We want the channel step line numbers to start at 1
                                LightPathSequenceStep si = new LightPathSequenceStep(str, ndList[i], i + 1);
                                _captureSequence.Add(si);
                            }
                        }
                        gbSequentialCapture.Visibility = Visibility.Visible;
                        gbPMTGains.Visibility = Visibility.Collapsed;
                        gbMCLS.Visibility = Visibility.Collapsed;
                        gbMultiPhotonLaser.Visibility = Visibility.Collapsed;
                        gbPinhole.Visibility = Visibility.Collapsed;
                    }
                    else
                    {
                        string appSettings = GetApplicationSettingsFileString();

                        XmlDocument appDoc = new XmlDocument();
                        appDoc.Load(appSettings);

                        gbSequentialCapture.Visibility = Visibility.Collapsed;
                        ndList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/MultiLaserControlView");
                        if (0 < ndList.Count)
                        {
                            gbMCLS.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                        }

                        ndList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/MultiphotonView");
                        if (0 < ndList.Count)
                        {
                            gbMultiPhotonLaser.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                        }

                        ndList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/PinholeView");
                        if (0 < ndList.Count)
                        {
                            gbPinhole.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                        }
                        if (0 == CameraType) //if it is a CCD camera don't show the pmts
                        {
                            gbPMTGains.Visibility = Visibility.Collapsed;
                        }
                        else if (1 == CameraType) //if it is a LSM camera show the pmts
                        {
                            gbPMTGains.Visibility = Visibility.Visible;
                        }
                    }
                    OnPropertyChanged("CollectionCaptureSequence");
                }
            }
            catch (Exception e)
            {
                e.ToString();
            }
        }

        private void LoadExperimentOptions()
        {
            try
            {
                if (!File.Exists(ExperimentName)) return;
                XmlDocument xDoc = new XmlDocument();
                xDoc.Load(ExperimentName);
                if (null != xDoc)
                {
                    XmlNode node = xDoc.SelectSingleNode("ThorImageExperiment/ZStage");
                    string str = string.Empty;

                    if (XmlManager.GetAttribute(node, xDoc, "steps", ref str))
                    {
                        _zSteps = Int32.Parse(str, CultureInfo.InvariantCulture);
                    }

                    str = string.Empty;

                    if (XmlManager.GetAttribute(node, xDoc, "startPos", ref str))
                    {
                        _zStartPosition = Double.Parse(str, CultureInfo.InvariantCulture);
                    }

                    str = string.Empty;
                    if (XmlManager.GetAttribute(node, xDoc, "stepSizeUM", ref str))
                    {
                        _zStepSize = Double.Parse(str, CultureInfo.InvariantCulture);
                    }

                    str = string.Empty;
                    if (XmlManager.GetAttribute(node, xDoc, "enable", ref str))
                    {
                        ZEnable = (1 == Convert.ToInt32(str));
                    }
                    else
                    {
                        //for legacy experiments set enable flag based on num Z steps
                        ZEnable = (1 < ZSteps) ? true : false;
                    }

                    node = xDoc.SelectSingleNode("ThorImageExperiment/Timelapse");

                    str = string.Empty;
                    int tFrames = 1;
                    if (XmlManager.GetAttribute(node, xDoc, "timepoints", ref str))
                    {
                        tFrames = Int32.Parse(str);
                    }

                    int streamFrames = 0;
                    int flybackFrames = 0;

                    str = string.Empty;
                    node = xDoc.SelectSingleNode("ThorImageExperiment/Streaming");
                    if (XmlManager.GetAttribute(node, xDoc, "frames", ref str))
                    {
                        streamFrames = Int32.Parse(str);
                    }

                    str = string.Empty;
                    if (XmlManager.GetAttribute(node, xDoc, "flybackFrames", ref str))
                    {
                        flybackFrames = Int32.Parse(str);
                    }

                    str = string.Empty;
                    node = xDoc.SelectSingleNode("ThorImageExperiment/ZStage2");
                    if (null != node)
                    {
                        if (XmlManager.GetAttribute(node, xDoc, "pos", ref str))
                        {
                            _z2Pos = Double.Parse(str, CultureInfo.InvariantCulture);
                        }
                    }
                    else
                    {
                        _z2Pos = 0.0;
                    }

                    str = string.Empty;
                    node = xDoc.SelectSingleNode("ThorImageExperiment/SpectralFilter");
                    if (null != node)
                    {
                        if (XmlManager.GetAttribute(node, xDoc, "bandwidthMode", ref str))
                        {
                            _hsBandwithMode = Double.Parse(str, CultureInfo.InvariantCulture);
                        }
                        str = string.Empty;
                        if (XmlManager.GetAttribute(node, xDoc, "path", ref str))
                        {
                            _exposurePath = str;
                        }
                    }

                    str = string.Empty;
                    node = xDoc.SelectSingleNode("ThorImageExperiment/Modality");
                    if (null != node)
                    {
                        if (XmlManager.GetAttribute(node, xDoc, "primaryDetectorType", ref str))
                        {
                            _cameraType = Int32.Parse(str);
                        }
                    }

                    str = string.Empty;
                    node = xDoc.SelectSingleNode("ThorImageExperiment/Camera");
                    if (null != node)
                    {
                        if (XmlManager.GetAttribute(node, xDoc, "name", ref str))
                        {
                            _activeCameraName = str;
                        }
                        str = string.Empty;
                        if (XmlManager.GetAttribute(node, xDoc, "readoutTapIndex", ref str))
                        {
                            _readoutTapIndex = Int32.Parse(str);
                        }
                        str = string.Empty;
                        if (XmlManager.GetAttribute(node, xDoc, "readoutSpeedIndex", ref str))
                        {
                            _readoutSpeedIndex = Int32.Parse(str);
                        }
                        str = string.Empty;
                        if (XmlManager.GetAttribute(node, xDoc, "averageMode", ref str))
                        {
                            _averageMode = Int32.Parse(str);
                        }
                        str = string.Empty;
                        if (XmlManager.GetAttribute(node, xDoc, "isCSType", ref str))
                        {
                            isCSType = (0 == str.CompareTo("True"));
                        }
                        OnPropertyChanged("ReadoutSpeedVis");
                        OnPropertyChanged("GainBLVis");
                        OnPropertyChanged("TapsVis");
                    }

                    str = string.Empty;
                    node = xDoc.SelectSingleNode("ThorImageExperiment/EPITurret");
                    if (null != node)
                    {
                        if (XmlManager.GetAttribute(node, xDoc, "pos", ref str))
                        {
                            gbEpiTurret.Visibility = (0 < Int32.Parse(str)) ? Visibility.Visible : Visibility.Collapsed;

                        }
                    }
                    //Set the laser labels based off of whether there is a queryable wavelength
                    str = string.Empty;
                    node = xDoc.SelectSingleNode("ThorImageExperiment/MCLS");
                    if (null != node)
                    {
                        if (XmlManager.GetAttribute(node, xDoc, "wavelength1", ref str))
                        {
                            _laser1Wavelength = Int32.Parse(str);
                            if (_laser1Wavelength == 0)
                            {
                                laser1Label.Content = "1";
                            }
                            else
                            {
                                laser1Label.Content = _laser1Wavelength + " nm";
                            }
                        }
                        str = string.Empty;
                        if (XmlManager.GetAttribute(node, xDoc, "wavelength2", ref str))
                        {
                            _laser2Wavelength = Int32.Parse(str);
                            if (_laser2Wavelength == 0)
                            {
                                laser2Label.Content = "2";
                            }
                            else
                            {
                                laser2Label.Content = _laser2Wavelength + " nm";
                            }
                        }
                        str = string.Empty;
                        if (XmlManager.GetAttribute(node, xDoc, "wavelength3", ref str))
                        {
                            _laser3Wavelength = Int32.Parse(str);
                            if (_laser3Wavelength == 0)
                            {
                                laser3Label.Content = "3";
                            }
                            else
                            {
                                laser3Label.Content = _laser3Wavelength + " nm";
                            }
                        }
                        str = string.Empty;
                        if (XmlManager.GetAttribute(node, xDoc, "wavelength4", ref str))
                        {
                            _laser4Wavelength = Int32.Parse(str);
                            if (_laser4Wavelength == 0)
                            {
                                laser4Label.Content = "4";
                            }
                            else
                            {
                                laser4Label.Content = _laser4Wavelength + " nm";
                            }
                        }
                        //Set the visibility for the Analog/TTL stackpanel based on whether there is a queryable laser wavelength (MCLS vs Toptica)
                        if (_laser1Wavelength == 0 && _laser2Wavelength == 0 && _laser3Wavelength == 0 && _laser4Wavelength == 0)
                        {
                            splaserAnalogTTL.Visibility = Visibility.Collapsed;
                        }
                        else
                        {
                            splaserAnalogTTL.Visibility = Visibility.Visible;
                        }
                        //Set the checked status of Analog/TTL mode
                        str = string.Empty;
                        if (XmlManager.GetAttribute(node, xDoc, "allanalog", ref str))
                        {
                            _laserAllAnalog = Int32.Parse(str);
                            if (_laserAllAnalog == 0)
                            {
                                cbLaserAnalog.IsChecked = false;
                            }
                            else
                            {
                                cbLaserAnalog.IsChecked = true;
                            }
                        }
                        str = string.Empty;
                        if (XmlManager.GetAttribute(node, xDoc, "allttl", ref str))
                        {
                            _laserAllTTL = Int32.Parse(str);
                            if (_laserAllTTL == 0)
                            {
                                cbLaserTTL.IsChecked = false;
                            }
                            else
                            {
                                cbLaserTTL.IsChecked = true;
                            }
                        }
                    }

                    str = string.Empty;

                    _streamVolumes = Math.Max(1, Convert.ToInt32(Math.Round(streamFrames / (double)(_zSteps + flybackFrames))));

                    _zStopPosition = _zStartPosition + _zStepSize * UM_TO_MM * (_zSteps - 1);

                    _zStepSize = Math.Abs(_zStepSize);

                    tbZStartPosition.Text = (_zStartPosition * 1000.0).ToString();
                    tbZStopPosition.Text = (_zStopPosition * 1000.0).ToString();
                    tbZStepSize.Text = _zStepSize.ToString();

                    tbStreamVolumes.Text = _streamVolumes.ToString();

                    tb2ZStartPosition.Text = (_zStartPosition * 1000.0).ToString();
                    tb2ZStopPosition.Text = (_zStopPosition * 1000.0).ToString();
                    tb2ZStepSize.Text = _zStepSize.ToString();

                    TEnable = (1 < tFrames) ? true : false;

                    OnPropertyChanged("ZSteps");
                    OnPropertyChanged("TTotalTime");
                    OnPropertyChanged("StreamTotalTime");
                    OnPropertyChanged("Z2Position");
                    OnPropertyChanged("ExposureTable");
                    OnPropertyChanged("HSBandwithMode");
                    OnPropertyChanged("Taps");
                    OnPropertyChanged("AverageMode");
                    OnPropertyChanged("ReadoutSpeed");
                }
            }
            catch (Exception e)
            {
                e.ToString();
            }
        }

        private void LoadPathList()
        {
            cbOutputPath.Items.Clear();

            _variableDoc = new XmlDocument();

            if (File.Exists(_variableFile))
            {
                _variableDoc.Load(_variableFile);

                XmlNodeList ndList = _variableDoc.SelectNodes("/VariableList/Path");

                int i = 0;
                foreach (XmlNode node in ndList)
                {
                    string str = string.Empty;
                    if (XmlManager.GetAttribute(node, _variableDoc, "name", ref str))
                    {
                        cbOutputPath.Items.Add(str);
                        if (str.Equals(OutputPath))
                        {
                            cbOutputPath.SelectedIndex = i;
                        }
                    }
                    i++;
                }
            }
        }

        private void LoadTemplateList()
        {
            cbTemplate.Items.Clear();
            string[] strList = Directory.GetFiles(_strTemplates, "*.xml");

            for (int i = 0; i < strList.Length; i++)
            {
                string str = System.IO.Path.GetFileNameWithoutExtension(strList[i]);

                //do not populate the list with the active and default templates
                if (str.Equals("Active") || str.Equals("Default"))
                {
                    continue;
                }

                cbTemplate.Items.Add(str);

                if (strList[i].Equals(ExperimentName))
                {
                    cbTemplate.SelectedIndex = cbTemplate.Items.Count - 1;
                }
            }
        }

        void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
        }

        private void PopulateBleachROIDataGrid(string bleachName)
        {
            XmlDocument bleachDoc = new XmlDocument();

            bleachDoc.Load(bleachName);

            for (int i = 0; i < 12; i++)
            {
                DataGridTextColumn dgc = new DataGridTextColumn();

                switch (i)
                {
                    case 0: dgc.Header = ""; dgc.Binding = new Binding("Name"); break;
                    case 1: dgc.Header = "Shape"; dgc.Binding = new Binding("Shape"); break;
                    case 2: dgc.Header = "Dwell Time"; dgc.Binding = new Binding("DwellTime"); break;
                    case 3: dgc.Header = "Iterations"; dgc.Binding = new Binding("Iterations"); break;
                    case 4: dgc.Header = "Pre Idle"; dgc.Binding = new Binding("PreIdle"); break;
                    case 5: dgc.Header = "Post Idle"; dgc.Binding = new Binding("PostIdle"); break;
                    case 6: dgc.Header = "Width"; dgc.Binding = new Binding("Width"); break;
                    case 7: dgc.Header = "Height"; dgc.Binding = new Binding("Height"); break;
                    case 8: dgc.Header = "TopLeft"; dgc.Binding = new Binding("TopLeft"); break;
                    case 9: dgc.Header = "BottomRight"; dgc.Binding = new Binding("BottomRight"); break;
                    case 10: dgc.Header = "Center"; dgc.Binding = new Binding("Center"); break;
                    case 11: dgc.Header = "Points"; dgc.Binding = new Binding("Points"); break;

                }
                dgBleach.Columns.Add(dgc);
            }

            string xmlTitle = @"<?xml version=""1.0"" encoding=""utf-8""?>";
            var newDoc = xmlTitle + RemoveAllNamespaces(XElement.Parse(bleachDoc.DocumentElement.OuterXml));

            var bDoc = new XmlDocument { XmlResolver = null };
            bDoc.LoadXml((string)newDoc);

            string xPathStr = "ROICapsule/ROICapsule.ROIs/Array/*";

            XmlNodeList ndList = bDoc.SelectNodes(xPathStr);

            for (int i = 0; i < ndList.Count; i++)
            {
                //Check Mode(0: STATS_ONLY) for bleach ROIs:
                int val;
                Int32.TryParse(ndList[i].ChildNodes[0].ChildNodes[0].ChildNodes[(int)ThorSharedTypes.Tag.MODE].ChildNodes[0].Value, out val);
                BitVector32 bVec = new BitVector32(val);
                if ((int)ThorSharedTypes.Mode.STATSONLY != bVec[BitVector32.CreateSection(255)])
                    continue;

                string name = string.Empty;
                XmlManager.GetAttribute(ndList[i], bDoc, "ToolTip", ref name);

                string shape = ndList[i].Name;

                string dwelltime = string.Empty;
                XmlManager.GetAttribute(ndList[i], bDoc, "BleachClass.DwellTime", ref dwelltime);

                RoundDoubleString(ref dwelltime, 2);

                string iterations = string.Empty;
                XmlManager.GetAttribute(ndList[i], bDoc, "BleachClass.Iterations", ref iterations);

                string preidle = string.Empty;
                XmlManager.GetAttribute(ndList[i], bDoc, "BleachClass.PreIdleTime", ref preidle);

                RoundDoubleString(ref preidle, 2);

                string postidle = string.Empty;
                XmlManager.GetAttribute(ndList[i], bDoc, "BleachClass.PostIdleTime", ref postidle);

                RoundDoubleString(ref postidle, 2);

                string roiwidth = string.Empty;
                XmlManager.GetAttribute(ndList[i], bDoc, "ROIWidth", ref roiwidth);

                RoundDoubleString(ref roiwidth, 2);

                string roiheight = string.Empty;
                XmlManager.GetAttribute(ndList[i], bDoc, "ROIHeight", ref roiheight);

                RoundDoubleString(ref roiheight, 2);

                string topleft = string.Empty;
                string bottomright = string.Empty;

                if (ndList[i].Name.Equals("ROIEllipse"))
                {
                    XmlManager.GetAttribute(ndList[i], bDoc, "StartPoint", ref topleft);
                    XmlManager.GetAttribute(ndList[i], bDoc, "EndPoint", ref bottomright);
                }
                else
                {
                    XmlManager.GetAttribute(ndList[i], bDoc, "TopLeft", ref topleft);
                    XmlManager.GetAttribute(ndList[i], bDoc, "BottomRight", ref bottomright);
                }
                RoundDoubleString(ref topleft, 2);
                RoundDoubleString(ref bottomright, 2);

                string center = string.Empty;

                if (ndList[i].Name.Equals("ROIEllipse"))
                {
                    XmlManager.GetAttribute(ndList[i], bDoc, "Center", ref center);
                }
                else
                {
                    XmlManager.GetAttribute(ndList[i], bDoc, "CenterPoint", ref center);
                }
                RoundDoubleString(ref center, 2);

                string points = string.Empty;

                if (ndList[i].Name.Equals("Line"))
                {
                    string str = string.Empty;
                    XmlManager.GetAttribute(ndList[i], bDoc, "X1", ref str);
                    points = str;
                    XmlManager.GetAttribute(ndList[i], bDoc, "Y1", ref str);
                    points += "," + str;

                    XmlManager.GetAttribute(ndList[i], bDoc, "X2", ref str);
                    points += " " + str;
                    XmlManager.GetAttribute(ndList[i], bDoc, "Y2", ref str);
                    points += "," + str;
                }
                else
                {
                    XmlManager.GetAttribute(ndList[i], bDoc, "Points", ref points);
                }

                var data = new BleachROI { Name = name, Shape = shape, DwellTime = dwelltime, Iterations = iterations, PreIdle = preidle, PostIdle = postidle, Width = roiwidth, Height = roiheight, TopLeft = topleft, BottomRight = bottomright, Center = center, Points = points };
                dgBleach.Items.Add(data);
            }
        }

        private void PopulateSLMROIDataGrid(string expName, string roiName)
        {
            XmlDocument expDoc = new XmlDocument();
            expDoc.Load(expName);

            XmlDocument roiDoc = new XmlDocument();
            roiDoc.Load(roiName);

            //set up data grid columns:
            for (int i = 0; i < 13; i++)
            {
                DataGridTextColumn dgc = new DataGridTextColumn();

                switch (i)
                {
                    case 0: dgc.Header = ""; dgc.Binding = new Binding("Name"); break;
                    case 1: dgc.Header = "Name"; dgc.Binding = new Binding("PatternName"); break;
                    case 2: dgc.Header = "Shape"; dgc.Binding = new Binding("Shape"); break;
                    case 3: dgc.Header = "Durations"; dgc.Binding = new Binding("Durations"); break;
                    case 4: dgc.Header = "Iterations"; dgc.Binding = new Binding("Iterations"); break;
                    case 5: dgc.Header = "Pre Idle"; dgc.Binding = new Binding("PreIdle"); break;
                    case 6: dgc.Header = "Post Idle"; dgc.Binding = new Binding("PostIdle"); break;
                    case 7: dgc.Header = "Power"; dgc.Binding = new Binding("Power"); break;
                    case 8: dgc.Header = "Width"; dgc.Binding = new Binding("Width"); break;
                    case 9: dgc.Header = "Height"; dgc.Binding = new Binding("Height"); break;
                    case 10: dgc.Header = "TopLeft"; dgc.Binding = new Binding("TopLeft"); break;
                    case 11: dgc.Header = "BottomRight"; dgc.Binding = new Binding("BottomRight"); break;
                    case 12: dgc.Header = "Center"; dgc.Binding = new Binding("Center"); break;

                }
                dgBleach.Columns.Add(dgc);
            }

            string xmlTitle = @"<?xml version=""1.0"" encoding=""utf-8""?>";
            var newDoc = xmlTitle + RemoveAllNamespaces(XElement.Parse(roiDoc.DocumentElement.OuterXml));

            var bDoc = new XmlDocument { XmlResolver = null };
            bDoc.LoadXml((string)newDoc);

            string xPathStr = "ROICapsule/ROICapsule.ROIs/Array/*";

            XmlNodeList ndList = bDoc.SelectNodes(xPathStr);

            //ROIs:
            for (int i = 0; i < ndList.Count; i++)
            {
                string str = string.Empty;
                string name = string.Empty;
                XmlManager.GetAttribute(ndList[i], bDoc, "ToolTip", ref name);

                string shape = ndList[i].Name;

                //Check Mode(1: PATTERN_NOSTATS) for SLM ROIs:
                int val;
                Int32.TryParse(ndList[i].ChildNodes[0].ChildNodes[0].ChildNodes[(int)ThorSharedTypes.Tag.MODE].ChildNodes[0].Value, out val);
                BitVector32 bVec = new BitVector32(val);
                if ((int)ThorSharedTypes.Mode.PATTERN_NOSTATS != bVec[BitVector32.CreateSection(255)])
                    continue;

                //Get Pattern ID:
                Int32.TryParse(ndList[i].ChildNodes[0].ChildNodes[0].ChildNodes[(int)ThorSharedTypes.Tag.PATTERN_ID].ChildNodes[0].Value, out val);

                //Find params based on Pattern ID:
                string patternName = string.Empty, durations = string.Empty, iterations = string.Empty;
                string preidle = string.Empty, postidle = string.Empty, power = string.Empty;
                XmlNodeList ndListExp = expDoc.SelectNodes("/ThorImageExperiment/SLM/Pattern");
                for (int j = 0; j < ndListExp.Count; j++)
                {
                    int id = 0;
                    if (XmlManager.GetAttribute(ndListExp[j], expDoc, "fileID", ref str) && (Int32.TryParse(str, out id)) && (id == val))
                    {
                        XmlManager.GetAttribute(ndListExp[j], expDoc, "name", ref patternName);

                        XmlManager.GetAttribute(ndListExp[j], expDoc, "durationMS", ref durations);
                        RoundDoubleString(ref durations, 2);

                        XmlManager.GetAttribute(ndListExp[j], expDoc, "iterations", ref iterations);

                        XmlManager.GetAttribute(ndListExp[j], expDoc, "preIteIdleMS", ref preidle);
                        RoundDoubleString(ref preidle, 2);

                        XmlManager.GetAttribute(ndListExp[j], expDoc, "postIteIdleMS", ref postidle);
                        RoundDoubleString(ref postidle, 2);

                        XmlManager.GetAttribute(ndListExp[j], expDoc, "power", ref power);
                        RoundDoubleString(ref power, 2);
                    }
                }

                string roiwidth = string.Empty, roiheight = string.Empty;
                XmlManager.GetAttribute(ndList[i], bDoc, "ROIWidth", ref roiwidth);
                RoundDoubleString(ref roiwidth, 2);

                XmlManager.GetAttribute(ndList[i], bDoc, "ROIHeight", ref roiheight);
                RoundDoubleString(ref roiheight, 2);

                string topleft = string.Empty, bottomright = string.Empty;
                if (ndList[i].Name.Equals("ROIEllipse"))
                {
                    XmlManager.GetAttribute(ndList[i], bDoc, "StartPoint", ref topleft);
                    XmlManager.GetAttribute(ndList[i], bDoc, "EndPoint", ref bottomright);
                }
                else
                {
                    XmlManager.GetAttribute(ndList[i], bDoc, "TopLeft", ref topleft);
                    XmlManager.GetAttribute(ndList[i], bDoc, "BottomRight", ref bottomright);
                }
                RoundDoubleString(ref topleft, 2);
                RoundDoubleString(ref bottomright, 2);

                string center = string.Empty;
                if (ndList[i].Name.Equals("ROIEllipse"))
                {
                    XmlManager.GetAttribute(ndList[i], bDoc, "Center", ref center);
                }
                else
                {
                    XmlManager.GetAttribute(ndList[i], bDoc, "CenterPoint", ref center);
                }
                RoundDoubleString(ref center, 2);

                var data = new SLMROI { Name = name, PatternName = patternName, Shape = shape, Durations = durations, Iterations = iterations, PreIdle = preidle, PostIdle = postidle, Power = power, Width = roiwidth, Height = roiheight, TopLeft = topleft, BottomRight = bottomright, Center = center };
                dgBleach.Items.Add(data);
            }
        }

        private void RoundDoubleString(ref string val, int places)
        {
            try
            {
                Decimal dec = new Decimal(Convert.ToDouble(val));

                val = Decimal.Round(dec, places).ToString();
            }
            catch (Exception ex)
            {
                string err = ex.Message;
            }
        }

        void SaveExperimentSettings()
        {
            try
            {
                XmlDataProvider provider = (XmlDataProvider)FindResource("expData");
                if (true == _changeSettingsEnable && null != provider.Document)
                {

                    double zStepSize = _zStepSize;

                    if (_zStartPosition > _zStopPosition)
                    {
                        zStepSize *= -1;
                    }
                    if (cbCaptureMode.SelectedIndex == 0 && ZEnable == false)
                    {
                        ZSteps = 1;
                    }
                    else if ((cbCaptureMode.SelectedIndex == 1 && chFastZ.IsChecked == false))
                    {
                        ZSteps = 1;
                    }
                    else if (cbCaptureMode.SelectedIndex > 1)
                    {
                        ZSteps = 1;
                    }
                    int tFrames = Convert.ToInt32(tbTFrames.Text);
                    if (false == TEnable || cbCaptureMode.SelectedIndex != 0)
                    {
                        tFrames = 1;
                    }

                    XmlNode node = provider.Document.SelectSingleNode("ThorImageExperiment/ZStage");
                    SetAttribute(node, provider.Document, "steps", _zSteps.ToString());
                    SetAttribute(node, provider.Document, "startPos", _zStartPosition.ToString().Replace(",", "."));
                    SetAttribute(node, provider.Document, "stepSizeUM", zStepSize.ToString().Replace(",", "."));

                    node = provider.Document.SelectSingleNode("ThorImageExperiment/Timelapse");
                    SetAttribute(node, provider.Document, "steps", tFrames.ToString());

                    node = provider.Document.SelectSingleNode("ThorImageExperiment/Streaming");
                    if (cbCaptureMode.SelectedIndex == 1)
                    {
                        //update the stream frames if the fast z is enabled in Finite Stream mode(0):
                        if ((this.cbStorageModes.SelectedIndex == 0) && (true == chFastZ.IsChecked))
                        {
                            int streamFrames = (_zSteps + Convert.ToInt32(tbFlybackFrames.Text)) * _streamVolumes;
                            SetAttribute(node, provider.Document, "frames", streamFrames.ToString());
                        }
                    }

                    string streamEnable = (cbCaptureMode.SelectedIndex == 1) ? "1" : "0";
                    SetAttribute(node, provider.Document, "enable", streamEnable);
                    provider.Document.Save(_experimentName);

                }
                else if (null != provider.Document)
                {

                    provider.Document.Save(_experimentName);

                }
            }
            catch (Exception ex)
            {
                string str = ex.Message;
            }
        }

        //Create the attribute if it does not exisit.
        //Set the attribute value
        private void SetAttribute(XmlNode node, XmlDocument doc, string attrName, string attrValue)
        {
            if (null == node.Attributes.GetNamedItem(attrName))
            {
                XmlAttribute attr = doc.CreateAttribute(attrName);
                attr.Value = attrValue;
                node.Attributes.Append(attr);
            }

            node.Attributes[attrName].Value = attrValue;
        }

        private void SetCommandParameter(XmlDocument doc, string tag, string attrib, string value)
        {
            if (null != doc)
            {
                XmlNode node = doc.SelectSingleNode(tag);

                if (null != node)
                {
                    node = node.Attributes.GetNamedItem(attrib);
                    if (null != node)
                    {
                        node.Value = value;
                    }
                }
            }
        }

        private void UpdateZSteps()
        {
            ZSteps = (Math.Max(1, (int)Math.Abs(Math.Round((_zStopPosition - _zStartPosition), 5) / (_zStepSize * UM_TO_MM)) + 1));
        }

        void UserControl1_Loaded(object sender, RoutedEventArgs e)
        {
            try
            {
                _strTemplates = Application.Current.Resources["TemplatesFolder"].ToString() + "\\Template Favorites";

                LoadTemplateList();

                _variableFile = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "/VariableList.xml";

                LoadPathList();

                string appSettingsPath = GetApplicationSettingsFileString();
                XmlDataProvider appSettingsProvider = (XmlDataProvider)FindResource("appSettings");
                appSettingsProvider.Source = new Uri(appSettingsPath);
                appSettingsProvider.Refresh();

                string hwSettingsPath = GetHardwareSettingsFileString();
                XmlDataProvider hwSettingsProvider = (XmlDataProvider)FindResource("hwSettings");
                hwSettingsProvider.Source = new Uri(hwSettingsPath);
                hwSettingsProvider.Refresh();

                if (true == _settingsDocUpdated)
                {
                    LoadPowerSettings();
                    LoadExperimentOptions();
                    LoadCaptureSequence();
                    _settingsDocUpdated = false;
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        private void UserControl1_LostFocus(object sender, RoutedEventArgs e)
        {
            SaveExperimentSettings();
            OnPropertyChanged("TTotalTime");
            OnPropertyChanged("StreamTotalTime");
            OnPropertyChanged("Z2Position");
        }

        void UserControl1_Unloaded(object sender, RoutedEventArgs e)
        {
        }

        #endregion Methods

        #region Nested Types

        public class BleachROI
        {
            #region Properties

            public string BottomRight
            {
                get;
                set;
            }

            public string Center
            {
                get;
                set;
            }

            public string DwellTime
            {
                get;
                set;
            }

            public string Height
            {
                get;
                set;
            }

            public string Iterations
            {
                get;
                set;
            }

            public string Name
            {
                get;
                set;
            }

            public string Points
            {
                get;
                set;
            }

            public string PostIdle
            {
                get;
                set;
            }

            public string PreIdle
            {
                get;
                set;
            }

            public string Shape
            {
                get;
                set;
            }

            public string TopLeft
            {
                get;
                set;
            }

            public string Width
            {
                get;
                set;
            }

            #endregion Properties
        }

        public class SLMROI
        {
            #region Properties

            public string BottomRight
            {
                get;
                set;
            }

            public string Center
            {
                get;
                set;
            }

            public string Durations
            {
                get;
                set;
            }

            public string Height
            {
                get;
                set;
            }

            public string Iterations
            {
                get;
                set;
            }

            public string Name
            {
                get;
                set;
            }

            public string PatternName
            {
                get;
                set;
            }

            public string PostIdle
            {
                get;
                set;
            }

            public string Power
            {
                get;
                set;
            }

            public string PreIdle
            {
                get;
                set;
            }

            public string Shape
            {
                get;
                set;
            }

            public string TopLeft
            {
                get;
                set;
            }

            public string Width
            {
                get;
                set;
            }

            #endregion Properties
        }

        #endregion Nested Types
    }
}