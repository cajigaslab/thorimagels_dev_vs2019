namespace ScriptXYMove
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
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

    using ThorLogging;

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class UserControl1 : UserControl, INotifyPropertyChanged
    {
        #region Fields

        const string CMD_ENABLE_LOAD_TEMPLATE = "loadTemplateEnabled";
        const string CMD_INPUT_PATH = "templateInputPath";
        const string CMD_STEPS_IN_MM = "isStepsInMM";
        const string CMD_STEPS_IN_PIXELS = "isStepsInPixels";
        const string CMD_STEPS_IN_UM = "isStepsInUM";
        const string CMD_XYMOVE = "Command/XYMove";
        const string CMD_X_DORELATIVE_MOVE = "doRelativeXMove";
        const string CMD_X_ENABLE_MOVE_ATTRIBUTE = "enableXMove";
        const string CMD_X_NEW_POS_ATTRIBUTE = "xNewPos";
        const string CMD_Y_DORELATIVE_MOVE = "doRelativeYMove";
        const string CMD_Y_ENABLE_MOVE_ATTRIBUTE = "enableYMove";
        const string CMD_Y_NEW_POS_ATTRIBUTE = "yNewPos";

        private int _enableLoadFromTemplate = 0;
        private int _enableXMove = 0;
        private int _enableYMove = 0;
        private string _inputPath = string.Empty;
        private XmlDocument _settingsDocument = null;
        private int _stepsInMM = 1;
        private int _stepsInPixels = 0;
        private int _stepsInUM = 0;
        private int _xDoRelativeMove = 0;
        private double _xPos = 0;
        private int _yDoRelativeMove = 0;
        private double _yPos = 0;

        #endregion Fields

        #region Constructors

        public UserControl1()
        {
            InitializeComponent();
            this.DataContext = this;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public int EnableLoadFromTemplate
        {
            get
            {
                return _enableLoadFromTemplate;
            }
            set
            {
                _enableLoadFromTemplate = value;
                SetCommandParameter(_settingsDocument, CMD_XYMOVE, CMD_ENABLE_LOAD_TEMPLATE, _enableLoadFromTemplate.ToString());
                OnPropertyChanged("EnableLoadFromTemplate");
                OnPropertyChanged("IsLoadFromTemplateDissabled");
            }
        }

        public int EnableXMove
        {
            get
            {
                return _enableXMove;
            }
            set
            {
                _enableXMove = value;
                SetCommandParameter(_settingsDocument, CMD_XYMOVE, CMD_X_ENABLE_MOVE_ATTRIBUTE, _enableXMove.ToString());

                OnPropertyChanged("EnableXMove");
            }
        }

        public int EnableYMove
        {
            get
            {
                return _enableYMove;
            }
            set
            {
                _enableYMove = value;
                SetCommandParameter(_settingsDocument, CMD_XYMOVE, CMD_Y_ENABLE_MOVE_ATTRIBUTE, _enableYMove.ToString());

                OnPropertyChanged("EnableYMove");
            }
        }

        public string InputPath
        {
            get
            {
                return _inputPath;
            }
            set
            {
                if (_inputPath != value)
                {
                    _inputPath = value;
                    SetCommandParameter(_settingsDocument, CMD_XYMOVE, CMD_INPUT_PATH, _inputPath.ToString());
                    OnPropertyChanged("InputPath");
                }
            }
        }

        public bool IsLoadFromTemplateDissabled
        {
            get
            {
                return (1 == EnableLoadFromTemplate) ? false : true;
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
                    XmlNode node = _settingsDocument.SelectSingleNode(CMD_XYMOVE);

                    if (null != node)
                    {
                        string str = string.Empty;

                        //Retrieve X Settings
                        if (GetAttribute(node, _settingsDocument, CMD_X_ENABLE_MOVE_ATTRIBUTE, ref str))
                        {
                            int tmp = 0;
                            if (int.TryParse(str, out tmp))
                            {
                                EnableXMove = tmp;
                            }
                        }

                        if (GetAttribute(node, _settingsDocument, CMD_X_DORELATIVE_MOVE, ref str))
                        {
                            int tmp = 0;
                            if (int.TryParse(str, out tmp))
                            {
                                XDoRelativeMove = tmp;
                            }
                        }

                        if (GetAttribute(node, _settingsDocument, CMD_X_NEW_POS_ATTRIBUTE, ref str))
                        {
                            double tmp = 0;
                            if (double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                            {
                                XPos = tmp;
                            }
                        }

                        //Retrieve Y Settings
                        if (GetAttribute(node, _settingsDocument, CMD_Y_NEW_POS_ATTRIBUTE, ref str))
                        {
                            double tmp = 0;
                            if (double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                            {
                                YPos = tmp;
                            }
                        }

                        if (GetAttribute(node, _settingsDocument, CMD_Y_DORELATIVE_MOVE, ref str))
                        {
                            int tmp = 0;
                            if (int.TryParse(str, out tmp))
                            {
                                YDoRelativeMove = tmp;
                            }
                        }

                        if (GetAttribute(node, _settingsDocument, CMD_Y_ENABLE_MOVE_ATTRIBUTE, ref str))
                        {
                            int tmp = 0;
                            if (int.TryParse(str, out tmp))
                            {
                                EnableYMove = tmp;
                            }
                        }

                        if (GetAttribute(node, _settingsDocument, CMD_ENABLE_LOAD_TEMPLATE, ref str))
                        {
                            int tmp = 0;
                            if (int.TryParse(str, out tmp))
                            {
                                EnableLoadFromTemplate = tmp;
                            }
                        }
                        else
                        {
                            SetCommandParameter(_settingsDocument, CMD_XYMOVE, CMD_ENABLE_LOAD_TEMPLATE, _enableLoadFromTemplate.ToString());
                        }

                        if (GetAttribute(node, _settingsDocument, CMD_STEPS_IN_MM, ref str))
                        {
                            int tmp = 0;
                            if (int.TryParse(str, out tmp))
                            {
                                StepsInMM = tmp;
                            }
                        }
                        else
                        {
                            SetCommandParameter(_settingsDocument, CMD_XYMOVE, CMD_STEPS_IN_MM, _stepsInMM.ToString());
                        }

                        if (GetAttribute(node, _settingsDocument, CMD_STEPS_IN_UM, ref str))
                        {
                            int tmp = 0;
                            if (int.TryParse(str, out tmp))
                            {
                                StepsInUM = tmp;
                            }
                        }
                        else
                        {
                            SetCommandParameter(_settingsDocument, CMD_XYMOVE, CMD_STEPS_IN_UM, _stepsInUM.ToString());
                        }

                        if (GetAttribute(node, _settingsDocument, CMD_STEPS_IN_PIXELS, ref str))
                        {
                            int tmp = 0;
                            if (int.TryParse(str, out tmp))
                            {
                                StepsInPixels = tmp;
                            }
                        }
                        else
                        {
                            SetCommandParameter(_settingsDocument, CMD_XYMOVE, CMD_STEPS_IN_PIXELS, _stepsInPixels.ToString());
                        }

                        if (GetAttribute(node, _settingsDocument, CMD_INPUT_PATH, ref str))
                        {
                            InputPath = str;
                        }
                        else
                        {
                            SetCommandParameter(_settingsDocument, CMD_XYMOVE, CMD_INPUT_PATH, _inputPath.ToString());
                        }

                    }
                }
            }
        }

        public int StepsInMM
        {
            get
            {
                return _stepsInMM;
            }
            set
            {
                _stepsInMM = value;
                if (1 == _stepsInMM)
                {
                    StepsInPixels = 0;
                    StepsInUM = 0;
                    OnPropertyChanged("Units");
                }
                SetCommandParameter(_settingsDocument, CMD_XYMOVE, CMD_STEPS_IN_MM, _stepsInMM.ToString());
                OnPropertyChanged("StepsInMM");
            }
        }

        public int StepsInPixels
        {
            get
            {
                return _stepsInPixels;
            }
            set
            {
                _stepsInPixels = value;
                if (1 == _stepsInPixels)
                {
                    StepsInMM = 0;
                    StepsInUM = 0;
                    MessageBox.Show("Warning: Pixel size will be mesured using the PixelSizeUM value in the selected template. Make sure the template was created with corret Modality and Image Detector. The pixel to um conversion will depend on the image detector and Objective configuration.", "Step in Pixels Warning");
                    OnPropertyChanged("Units");
                }
                SetCommandParameter(_settingsDocument, CMD_XYMOVE, CMD_STEPS_IN_PIXELS, _stepsInPixels.ToString());
                OnPropertyChanged("StepsInPixels");
            }
        }

        public int StepsInUM
        {
            get
            {
                return _stepsInUM;
            }
            set
            {
                _stepsInUM = value;
                if (1 == _stepsInUM)
                {
                    StepsInMM = 0;
                    StepsInPixels = 0;
                    OnPropertyChanged("Units");
                }
                SetCommandParameter(_settingsDocument, CMD_XYMOVE, CMD_STEPS_IN_UM, _stepsInUM.ToString());
                OnPropertyChanged("StepsInUM");
            }
        }

        public string Units
        {
            get
            {
                if (1 == StepsInMM)
                {
                    return "[mm]";
                }
                else if (1 == StepsInUM)
                {
                    return "[um]";
                }
                else if (1 == StepsInPixels)
                {
                    return "[px]";
                }
                return string.Empty;
            }
        }

        public int XDoRelativeMove
        {
            get
            {
                return _xDoRelativeMove;
            }
            set
            {
                _xDoRelativeMove = value;
                SetCommandParameter(_settingsDocument, CMD_XYMOVE, CMD_X_DORELATIVE_MOVE, _xDoRelativeMove.ToString());

                OnPropertyChanged("XDoRelativeMove");
            }
        }

        public double XPos
        {
            get
            {
                return _xPos;
            }
            set
            {
                _xPos = value;
                SetCommandParameter(_settingsDocument, CMD_XYMOVE, CMD_X_NEW_POS_ATTRIBUTE, _xPos.ToString(CultureInfo.InvariantCulture));

                OnPropertyChanged("XPos");
            }
        }

        public int YDoRelativeMove
        {
            get
            {
                return _yDoRelativeMove;
            }
            set
            {
                _yDoRelativeMove = value;
                SetCommandParameter(_settingsDocument, CMD_XYMOVE, CMD_Y_DORELATIVE_MOVE, _yDoRelativeMove.ToString());

                OnPropertyChanged("YDoRelativeMove");
            }
        }

        public double YPos
        {
            get
            {
                return _yPos;
            }
            set
            {
                _yPos = value;
                SetCommandParameter(_settingsDocument, CMD_XYMOVE, CMD_Y_NEW_POS_ATTRIBUTE, _yPos.ToString(CultureInfo.InvariantCulture));

                OnPropertyChanged("YPos");
            }
        }

        #endregion Properties

        #region Methods

        private void BrowseButton_Click(object sender, RoutedEventArgs e)
        {
            ExperimentSettingsBrowser.ExperimentSettingsBrowserWindow settingsDlg = new ExperimentSettingsBrowser.ExperimentSettingsBrowserWindow();
            settingsDlg.Title = "Experiment Browser";
            settingsDlg.BrowserType = ExperimentSettingsBrowser.ExperimentSettingsBrowserWindow.BrowserTypeEnum.SETTINGS;
            settingsDlg.Owner = System.Windows.Application.Current.MainWindow;
            try
            {
                if (true == settingsDlg.ShowDialog())
                {
                    InputPath = settingsDlg.ExperimentSettingsPath;
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Failed to get template. Exception Message:\n" + ex.Message);
            }
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

        private void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
        }

        private void SetCommandParameter(XmlDocument doc, string tag, string attrib, string value)
        {
            if (null != doc)
            {
                XmlNode cmdNode = doc.SelectSingleNode(tag);

                if (null != cmdNode)
                {
                    XmlNode node = cmdNode.Attributes.GetNamedItem(attrib);
                    if (null != node)
                    {
                        node.Value = value;
                    }
                    else
                    {
                        XmlAttribute attr = doc.CreateAttribute(attrib);
                        attr.Value = value;
                        cmdNode.Attributes.Append(attr);
                    }
                }
            }
        }

        #endregion Methods
    }
}