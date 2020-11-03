namespace ScriptMatlab.ViewModels
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Windows.Forms;
    using System.Windows.Input;
    using System.Xml;

    public class MainViewModel : ViewModelBase
    {
        #region Fields

        const string CMD_ASYNCHRONOUS = "Command/Asynchronous";
        const string CMD_INPUTDATAFOLDER = "Command/MatlabDataFolder";
        const string CMD_INPUTDATAFOLDERISMOSTRECSENT = "Command/MatlabDataFolderIsNotMostRecent";
        const string CMD_MATLAB_MACRO = "Command/MatlabMacro";
        const string CMD_VALUE = "value";

        private ICommand _browseInputFolder;
        private ICommand _browseMacro;
        private string _inputFolder;
        private bool _isAsynchronous;
        private bool _isNotMostRecentExp;
        private string _macroScriptPath;
        private XmlDocument _settingsDocument;

        #endregion Fields

        #region Constructors

        public MainViewModel()
        {
        }

        #endregion Constructors

        #region Properties

        public ICommand BrowseInputFolder
        {
            get
            {
                if (_browseInputFolder == null)
                    _browseInputFolder = new RelayCommand(() => OnBrowseInputFolder());
                return _browseInputFolder;
            }
        }

        public ICommand BrowseMacro
        {
            get
            {
                if (_browseMacro == null)
                    _browseMacro = new RelayCommand(() => OnBrowseMacro());
                return _browseMacro;
            }
        }

        public string InputFolder
        {
            set
            {
                if (_inputFolder != value)
                {
                    _inputFolder = value;
                    OnPropertyChanged("InputFolder");
                    SetCommandParameter(_settingsDocument, CMD_INPUTDATAFOLDER, CMD_VALUE, _inputFolder);
                }
            }
            get { return _inputFolder; }
        }

        public bool IsAsynchronous
        {
            set
            {
                if (_isAsynchronous != value)
                {
                    _isAsynchronous = value;
                    OnPropertyChanged("IsAsynchronous");
                    SetCommandParameter(_settingsDocument, CMD_ASYNCHRONOUS, CMD_VALUE, _isAsynchronous.ToString());
                }
            }
            get { return _isAsynchronous; }
        }

        public bool IsNotMostRecentExp
        {
            set
            {
                if (_isNotMostRecentExp != value)
                {
                    _isNotMostRecentExp = value;
                    OnPropertyChanged("IsNotMostRecentExp");
                    SetCommandParameter(_settingsDocument, CMD_INPUTDATAFOLDERISMOSTRECSENT, CMD_VALUE, IsNotMostRecentExp.ToString());
                }
            }
            get { return _isNotMostRecentExp; }
        }

        public string MacroScriptPath
        {
            set
            {
                if (_macroScriptPath != value)
                {
                    _macroScriptPath = value;
                    OnPropertyChanged("MacroScriptPath");
                    SetCommandParameter(_settingsDocument, CMD_MATLAB_MACRO, CMD_VALUE, _macroScriptPath);
                }
            }
            get { return _macroScriptPath; }
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
                if (null != _settingsDocument)
                {
                    // get script path
                    XmlNode node = _settingsDocument.SelectSingleNode(CMD_MATLAB_MACRO);
                    if (null != node)
                    {
                        string str = string.Empty;
                        if (GetAttribute(node, _settingsDocument, CMD_VALUE, ref str))
                        {
                            MacroScriptPath = str;
                        }
                    }

                    // get isMostrecentExperiment value
                    node = _settingsDocument.SelectSingleNode(CMD_INPUTDATAFOLDERISMOSTRECSENT);
                    if (null != node)
                    {
                        string str = string.Empty;

                        if (GetAttribute(node, _settingsDocument, CMD_VALUE, ref str))
                        {
                            try
                            {
                                IsNotMostRecentExp = Convert.ToBoolean(str);
                            }
                            catch (Exception)
                            {
                                IsNotMostRecentExp = false;
                            }
                        }
                    }

                    // get input folder value
                    node = _settingsDocument.SelectSingleNode(CMD_INPUTDATAFOLDER);
                    if (null != node)
                    {
                        string str = string.Empty;

                        if (IsNotMostRecentExp && GetAttribute(node, _settingsDocument, CMD_VALUE, ref str))
                        {
                            InputFolder = str;
                        }
                    }

                    // get asynchronous value
                    node = _settingsDocument.SelectSingleNode(CMD_ASYNCHRONOUS);
                    if (null != node)
                    {
                        string str = string.Empty;

                        if (GetAttribute(node, _settingsDocument, CMD_VALUE, ref str))
                        {
                            IsAsynchronous = Convert.ToBoolean(str);
                        }
                    }
                }
            }
        }

        #endregion Properties

        #region Methods

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetMyDocumentsThorImageFolder", CharSet = CharSet.Unicode)]
        public static extern int GetMyDocumentsThorImageFolder(StringBuilder sb, int length);

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

        private void OnBrowseInputFolder()
        {
            ExperimentSettingsBrowser.ExperimentSettingsBrowserWindow settingsDlg = new ExperimentSettingsBrowser.ExperimentSettingsBrowserWindow();
            settingsDlg.Title = "Experiment Browser";
            settingsDlg.BrowserType = ExperimentSettingsBrowser.ExperimentSettingsBrowserWindow.BrowserTypeEnum.EXPERIMENT;
            settingsDlg.Owner = System.Windows.Application.Current.MainWindow;
            try
            {
                if (true == settingsDlg.ShowDialog())
                {
                    InputFolder = (settingsDlg.ExperimentPath) + "\\";
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        private void OnBrowseMacro()
        {
            var dlg = new OpenFileDialog();
            string scriptDirectory = string.Empty;

            dlg.FileName = MacroScriptPath;
            dlg.DefaultExt = ".m";
            dlg.Filter = "Matlab Script (.m)|*.m";

            if (string.Empty != MacroScriptPath)
            {
                scriptDirectory = Path.GetDirectoryName(MacroScriptPath);
            }
            else
            {
                const int PATH_LENGTH = 261;
                StringBuilder sb = new StringBuilder(PATH_LENGTH);
                GetMyDocumentsThorImageFolder(sb, PATH_LENGTH);
                scriptDirectory = sb.ToString() + "Matlab Scripts\\Script Command Samples\\";
            }

            if (Directory.Exists(scriptDirectory))
            {
                dlg.InitialDirectory = scriptDirectory;
            }

            var result = dlg.ShowDialog();

            if (DialogResult.OK == result)
            {
                MacroScriptPath = dlg.FileName;
            }
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

        #endregion Methods
    }
}