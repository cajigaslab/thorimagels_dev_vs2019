namespace ScriptZMove
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

        const string CMD_DORELATIVE_MOVE = "doRelativeMove";
        const string CMD_ENABLE_LOAD_TEMPLATE = "loadTemplateEnabled";
        const string CMD_INPUT_PATH = "templateInputPath";
        const string CMD_STEPS_IN_UM = "isStepsInUM";
        const string CMD_ZMOVE = "Command/ZMove";
        const string CMD_Z_NEW_POS_ATTRIBUTE = "zNewPos";

        private int _doRelativeMove = 0;
        private int _enableLoadFromTemplate = 0;
        private string _inputPath = string.Empty;
        private XmlDocument _settingsDocument = null;
        private int _stepsInUM = 0;
        private double _zPos = 0;

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

        public int DoRelativeMove
        {
            get
            {
                return _doRelativeMove;
            }
            set
            {
                _doRelativeMove = value;
                SetCommandParameter(_settingsDocument, CMD_ZMOVE, CMD_DORELATIVE_MOVE, _doRelativeMove.ToString());

                OnPropertyChanged("DoRelativeMove");
            }
        }

        public int EnableLoadFromTemplate
        {
            get
            {
                return _enableLoadFromTemplate;
            }
            set
            {
                _enableLoadFromTemplate = value;
                SetCommandParameter(_settingsDocument, CMD_ZMOVE, CMD_ENABLE_LOAD_TEMPLATE, _enableLoadFromTemplate.ToString());
                OnPropertyChanged("EnableLoadFromTemplate");
                OnPropertyChanged("IsLoadFromTemplateDissabled");
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
                    SetCommandParameter(_settingsDocument, CMD_ZMOVE, CMD_INPUT_PATH, _inputPath.ToString());
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
                    XmlNode node = _settingsDocument.SelectSingleNode(CMD_ZMOVE);

                    if (null != node)
                    {
                        string str = string.Empty;

                        if (GetAttribute(node, _settingsDocument, CMD_Z_NEW_POS_ATTRIBUTE, ref str))
                        {
                            double tmp = 0;
                            if (double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                            {
                                ZPos = tmp;
                            }
                        }

                        if (GetAttribute(node, _settingsDocument, CMD_DORELATIVE_MOVE, ref str))
                        {
                            int tmp = 0;
                            if (int.TryParse(str, out tmp))
                            {
                                DoRelativeMove = tmp;
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
                            SetCommandParameter(_settingsDocument, CMD_ZMOVE, CMD_ENABLE_LOAD_TEMPLATE, _enableLoadFromTemplate.ToString());
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
                            SetCommandParameter(_settingsDocument, CMD_ZMOVE, CMD_STEPS_IN_UM, _stepsInUM.ToString());
                        }

                        if (GetAttribute(node, _settingsDocument, CMD_INPUT_PATH, ref str))
                        {
                            InputPath = str;
                        }
                        else
                        {
                            SetCommandParameter(_settingsDocument, CMD_ZMOVE, CMD_INPUT_PATH, _inputPath.ToString());
                        }
                    }
                }
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
                SetCommandParameter(_settingsDocument, CMD_ZMOVE, CMD_STEPS_IN_UM, _stepsInUM.ToString());
                OnPropertyChanged("StepsInMM");
                OnPropertyChanged("Units");
            }
        }

        public string Units
        {
            get
            {
                if (1 == StepsInUM)
                {
                    return "[um]";
                }
                return "[mm]";
            }
        }

        public double ZPos
        {
            get
            {
                return _zPos;
            }
            set
            {
                _zPos = value;
                SetCommandParameter(_settingsDocument, CMD_ZMOVE, CMD_Z_NEW_POS_ATTRIBUTE, _zPos.ToString(CultureInfo.InvariantCulture));

                OnPropertyChanged("ZPos");
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

        void OnPropertyChanged(string propertyName)
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