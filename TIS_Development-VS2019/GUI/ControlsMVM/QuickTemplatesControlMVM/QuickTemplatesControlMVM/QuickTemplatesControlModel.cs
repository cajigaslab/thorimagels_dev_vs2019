namespace QuickTemplatesControl.Model
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Timers;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using ExperimentSettingsBrowser;

    using ThorLogging;

    using ThorSharedTypes;

    #region Enumerations

    public enum ConfigurationState
    {
        UNLOAD,
        LOADING,
        LOADED,
        ACTIVE//must be loaded
    }

    #endregion Enumerations

    public class QuickConfig : INotifyPropertyChanged
    {
        #region Fields

        private bool _autoStart;
        private CaptureModes _captureMode;
        private QuickTempConfigDialog _curDia;
        private string _describeName;
        private string _filePath;
        private int _id;
        private bool _isActiveTemplate = false;
        private ConfigurationState _state;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the QuickTemplatesControlModel class
        /// </summary>
        public QuickConfig(int pId)
        {
            Id = pId;
            FilePath = "";
            DescribeName = "";
            AutoStart = false;
            CaptureMode = CaptureModes.T_AND_Z;
            State = ConfigurationState.UNLOAD;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public bool AutoStart
        {
            get { return _autoStart; }
            set
            {
                if (value != _autoStart)
                {
                    _autoStart = value;
                    OnPropertyChanged("AutoStart");
                }
            }
        }

        public CaptureModes CaptureMode
        {
            get { return _captureMode; }
            set { _captureMode = value; }
        }

        public QuickTempConfigDialog CurrentDiag
        {
            get
            {
                return _curDia;
            }
            set
            {
                _curDia = value;
            }
        }

        public string DescribeName
        {
            get { return _describeName; }
            set { _describeName = value; }
        }

        public string FilePath
        {
            get
            {
                return _filePath;
            }
            set
            {
                if (value != _filePath)
                {
                    _filePath = value;
                    OnPropertyChanged("FilePath");
                }
            }
        }

        public int Id
        {
            get { return _id; }
            set { _id = value; }
        }

        public ConfigurationState State
        {
            get { return _state; }
            set
            {
                if (value != _state)
                {
                    _state = value;
                    OnPropertyChanged("State");
                }
            }
        }

        #endregion Properties

        #region Methods

        public static bool IsSatisfiedFile(string pPath)
        {
            try
            {
                XmlDocument doc = new XmlDocument();
                doc.Load(pPath);
                var root = doc.DocumentElement;
                if (root.Name == "ThorImageExperiment")
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            catch (IOException)
            {
                return false;
            }
        }

        public void CaneleCommand()
        {
            _curDia.DialogResult = false;
            if (State == ConfigurationState.UNLOAD || _isActiveTemplate == false)
            {
                Clean();
            }
        }

        public void Clean()
        {
            this.FilePath = string.Empty;
            this.DescribeName = string.Empty;
            this.AutoStart = false;
            this.CaptureMode = CaptureModes.T_AND_Z;
            this.State = ConfigurationState.UNLOAD;
            MVMManager.Instance.ReloadSettings(SettingsFileType.APPLICATION_SETTINGS);
            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNodeList nChildren = doc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/QuickTemplate/Template");
            XmlManager.SetAttribute(nChildren[this.Id - 1], doc, "FilePath", this.FilePath);
            XmlManager.SetAttribute(nChildren[this.Id - 1], doc, "AutoStart", this.AutoStart.ToString());
            XmlManager.SetAttribute(nChildren[this.Id - 1], doc, "CaptureMode", ((int)this.CaptureMode).ToString());
            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
        }

        public QuickConfig Clone()
        {
            var ret = new QuickConfig(Id)
               {
               FilePath = this.FilePath,
               DescribeName = this.DescribeName,
               AutoStart = this.AutoStart,
               CaptureMode = this.CaptureMode,
               State = this.State,
               CurrentDiag = this.CurrentDiag
               };
            if (CurrentDiag != null)
            {
                CurrentDiag.expCanele.Command = new RelayCommand(CaneleCommand, null); //.SetBinding(Button.CommandProperty, bC);
                OnPropertyChanged("CaneleCommand");

                CurrentDiag.expOK.Command = new RelayCommand(OKCommand, null);
                OnPropertyChanged("OKCommand");

                CurrentDiag.expLoadFile.Command = new RelayCommand(LoadFileCommand, null);
                OnPropertyChanged("LoadFileCommand");

            }
            // Check the state of the button before loading the setup window. In case the cancel command is send, we need to revert to the initial state.
            if (State == ConfigurationState.UNLOAD)
            {
                _isActiveTemplate = false;
            }

            if (State == ConfigurationState.LOADED)
            {
                _isActiveTemplate = true;
            }

            return ret;
        }

        public void Copy(QuickConfig qc)
        {
            this.Id = qc.Id;
            this.FilePath = qc.FilePath;
            this.DescribeName = qc.DescribeName;
            this.AutoStart = qc.AutoStart;
            this.CaptureMode = qc.CaptureMode;
            this.State = qc.State;
        }

        public void LoadFileCommand()
        {
            ExperimentSettingsBrowserWindow settingsDlg = new ExperimentSettingsBrowserWindow();
            settingsDlg.Title = "Settings Browser";
            settingsDlg.BrowserType = ExperimentSettingsBrowserWindow.BrowserTypeEnum.SETTINGS;
            settingsDlg.Owner = Application.Current.MainWindow;
            settingsDlg.BrowseExperimentPath = ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "Capture Templates\\Template Favorites\\";
            bool? result = settingsDlg.ShowDialog();
            if (result == true)
            {
                if (QuickConfig.IsSatisfiedFile(settingsDlg.ExperimentSettingsPath))
                {
                    _curDia.expTxtFilePath.Text = FilePath = settingsDlg.ExperimentSettingsPath;
                    State = ConfigurationState.LOADED;
                }
                else
                {
                    _curDia.expTxtFilePath.Text = FilePath = string.Empty;
                    State = ConfigurationState.UNLOAD;
                    MessageBox.Show("Illegal File!");
                }
            }
        }

        public void OKCommand()
        {
            if (FilePath != string.Empty)
            {
                State = ConfigurationState.LOADED;
            }
            else
            {
                State = ConfigurationState.UNLOAD;
            }
            AutoStart = (bool)_curDia.expCbAutoStart.IsChecked;
            _curDia.DialogResult = true;
        }

        public void OnPropertyChanged(string propertyName)
        {
            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        #endregion Methods
    }
}