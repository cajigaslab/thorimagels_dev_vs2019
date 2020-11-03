namespace MenuLSDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.Net;
    using System.Reflection;
    using System.Resources;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using AboutDll;

    using LogFileWindow;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorImageInfastructure;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// ViewModel class for the MenuLS model object
    /// </summary>
    public class MenuLSViewModel : ViewModelBase
    {
        #region Fields

        // wrapped MenuLS object
        private readonly MenuLS _MenuLS;

        private ICommand _aboutCommand;
        private bool _captureButtonStatus = true;
        private bool _captureSetupButtonStatus = true;
        private ICommand _captureSetupCommand;
        private ICommand _checkForUpdatesCommand;
        private IUnityContainer _container;
        private bool _editMenuStatus = true;
        private IEventAggregator _eventAggregator;
        private ICommand _fileExitCommand;
        private bool _fileMenuStatus = true;
        private ICommand _hardwareConnectionsCommand;
        private ICommand _hardwareSetupLSCommand;
        private bool _helpMenuStatus = true;
        private bool _hwSetupButtonStatus = true;
        private ICommand _imageReviewCommand;
        private ICommand _logFileCommand;
        private bool _modalityComboBoxStatus = true;
        private bool _needToReconnectCamera = false;
        private bool _noLSMImageDetectors = true;
        private bool _paused;
        private IRegionManager _regionManager;
        private bool _reviewButtonStatus = true;
        private string _reviewModality = string.Empty;
        private ICommand _runSampleLSCommand;
        private bool _scriptButtonStatus = true;
        private ICommand _scriptCommand;
        private int _scriptCommandCounter = 0;
        private string _scriptCommandName;
        private ICommand _scriptPauseContinueCommand;
        private bool _scriptPlaying;
        private ICommand _scriptStopCommand;
        private int _selectedMenuTab = 0;
        private SubscriptionToken _subscriptionToken;
        private SubscriptionToken _subscriptionTokenCommandReviewModality;
        private SubscriptionToken _subscriptionTokenFinishedDialog;
        private SubscriptionToken _subscriptionTokenFinishedScript;
        private SubscriptionToken _subscriptionTokenShowDialog;
        private ICommand _supportCommand;
        private ICommand _webUpdateCommand;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the MenuLSViewModel class
        /// </summary>
        /// <param name="MenuLS">Wrapped MenuLS object</param>
        public MenuLSViewModel(IEventAggregator eventAggregator, IRegionManager regionManager, IUnityContainer container, MenuLS MenuLS)
        {
            this._eventAggregator = eventAggregator;
            this._regionManager = regionManager;
            this._container = container;

            if (MenuLS != null)
            {
                this._MenuLS = MenuLS;
            }
            else
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " MenuLS is null. Creating a new MenuLS object.");
                MenuLS = new MenuLS();

                if (MenuLS == null)
                {
                    ResourceManager rm = new ResourceManager("MenuModuleLS.Properties.Resources", Assembly.GetExecutingAssembly());
                    ThorLog.Instance.TraceEvent(TraceEventType.Critical, 1, this.GetType().Name + " " + rm.GetString("CreateMenuLSModelFailed"));
                    throw new NullReferenceException("MenuLS");
                }

                this._MenuLS = MenuLS;
            }

            //            this._MenuLS.Update += new Action<string>(MenuLS_Update);

            SubscribeToMenuModuleLSChangeEvent();

            SubscribeToShowDialogEvent();

            SubscribeToFinishedDialogEvent();

            SubscribeToFinishedScriptEvent();

            SubscribeToReviewModalityEvent();

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Properties

        public ICommand AboutCommand
        {
            get
            {
                if (this._aboutCommand == null)
                    this._aboutCommand = new RelayCommand(() => About());

                return this._aboutCommand;
            }
        }

        public bool CaptureButtonStatus
        {
            get
            {
                return _captureButtonStatus;
            }
            set
            {
                this._captureButtonStatus = value;
                OnPropertyChanged("CaptureButtonStatus");
            }
        }

        public bool CaptureSetupButtonStatus
        {
            get
            {
                return _captureSetupButtonStatus;
            }
            set
            {
                this._captureSetupButtonStatus = value;
                OnPropertyChanged("CaptureSetupButtonStatus");
            }
        }

        public ICommand CaptureSetupCommand
        {
            get
            {
                if (this._captureSetupCommand == null)
                    this._captureSetupCommand = new RelayCommand(() => CaptureSetup());

                return this._captureSetupCommand;
            }
        }

        public ICommand CheckForUpdatesCommand
        {
            get
            {
                if (this._checkForUpdatesCommand == null)
                    this._checkForUpdatesCommand = new RelayCommand(() => CheckForUpdates());

                return this._checkForUpdatesCommand;
            }
        }

        public bool EditMenuStatus
        {
            get
            {
                return _editMenuStatus;
            }
            set
            {
                this._editMenuStatus = value;
                OnPropertyChanged("EditMenuStatus");
            }
        }

        public ICommand FileExitCommand
        {
            get
            {
                if (this._fileExitCommand == null)
                    this._fileExitCommand = new RelayCommand(() => FileExit());

                return this._fileExitCommand;
            }
        }

        public bool FileMenuStatus
        {
            get
            {
                return _fileMenuStatus;
            }
            set
            {
                this._fileMenuStatus = value;
                OnPropertyChanged("FileMenuStatus");
            }
        }

        public ICommand HardwareConnectionsCommand
        {
            get
            {
                if (this._hardwareConnectionsCommand == null)
                    this._hardwareConnectionsCommand = new RelayCommand(() => HardwareConnectDlg());

                return this._hardwareConnectionsCommand;
            }
        }

        public ICommand HardwareSetupLSCommand
        {
            get
            {
                if (this._hardwareSetupLSCommand == null)
                    this._hardwareSetupLSCommand = new RelayCommand(() => HardwareSetupLS());

                return this._hardwareSetupLSCommand;
            }
        }

        public bool HelpMenuStatus
        {
            get
            {
                return _helpMenuStatus;
            }
            set
            {
                this._helpMenuStatus = value;
                OnPropertyChanged("HelpMenuStatus");
            }
        }

        public bool HWSetupButtonStatus
        {
            get
            {
                return _hwSetupButtonStatus;
            }
            set
            {
                this._hwSetupButtonStatus = value;
                OnPropertyChanged("HWSetupButonStatus");
            }
        }

        public string ImagePathPause
        {
            get
            {
                if (false == _paused)
                {
                    return "/MenuModuleLS;component/Icons/Pause.png";
                }
                else
                {
                    return "/MenuModuleLS;component/Icons/Play.png";
                }
            }
        }

        public ICommand ImageReviewCommand
        {
            get
            {
                if (this._imageReviewCommand == null)
                    this._imageReviewCommand = new RelayCommand(() => ImageReview());

                return this._imageReviewCommand;
            }
        }

        public ICommand LogFileCommand
        {
            get
            {
                if (this._logFileCommand == null)
                    this._logFileCommand = new RelayCommand(() => LogFile());

                return this._logFileCommand;
            }
        }

        /// <summary>
        /// Gets the wrapped MenuLS object
        /// </summary>
        public MenuLS MenuLS
        {
            get
            {
                return this._MenuLS;
            }
        }

        public bool ModalityComboBoxStatus
        {
            get
            {
                return _modalityComboBoxStatus;
            }
            set
            {
                this._modalityComboBoxStatus = value;
                OnPropertyChanged("ModalityComboBoxStatus");
            }
        }

        /// <summary>
        /// Gets or sets the modality previous.
        /// </summary>
        /// <value>The modality before a script command execute. This modality will be restored when the script execute completes</value>
        public string ModalityPrevious
        {
            get;
            set;
        }

        public bool NeedToReconnectCamera
        {
            get
            {
                return _needToReconnectCamera;
            }
            set
            {
                _needToReconnectCamera = value;
            }
        }

        public bool NoLSMImageDetectors
        {
            get
            {
                return _noLSMImageDetectors;
            }
            set
            {
                _noLSMImageDetectors = value;
            }
        }

        public bool Paused
        {
            get
            {
                return _paused;
            }
            set
            {
                _paused = value;
                OnPropertyChanged("Paused");
                OnPropertyChanged("ImagePathPause");
            }
        }

        public bool ReviewButtonStatus
        {
            get
            {
                return _reviewButtonStatus;
            }
            set
            {
                this._reviewButtonStatus = value;
                OnPropertyChanged("ReviewButtonStatus");
            }
        }

        public string ReviewModality
        {
            get
            {
                return _reviewModality;
            }
            set
            {
                _reviewModality = (2 != _selectedMenuTab) ? string.Empty : value;
                OnPropertyChanged("ReviewModality");
            }
        }

        public ICommand RunSampleLSCommand
        {
            get
            {
                if (this._runSampleLSCommand == null)
                    this._runSampleLSCommand = new RelayCommand(() => RunSampleLS());

                return this._runSampleLSCommand;
            }
        }

        public bool ScriptButtonStatus
        {
            get
            {
                return _scriptButtonStatus;
            }
            set
            {
                _scriptButtonStatus = value;
                OnPropertyChanged("ScriptButtonStatus");
            }
        }

        public ICommand ScriptCommand
        {
            get
            {
                if (this._scriptCommand == null)
                    this._scriptCommand = new RelayCommand(() => Script());

                return this._scriptCommand;
            }
        }

        public string ScriptCommandName
        {
            get
            {
                return _scriptCommandName;
            }
            set
            {
                _scriptCommandName = value;
                OnPropertyChanged("ScriptCommandName");
            }
        }

        public ICommand ScriptPauseContinueCommand
        {
            get
            {
                if (this._scriptPauseContinueCommand == null)
                    this._scriptPauseContinueCommand = new RelayCommand(() => ScriptPauseContinue());

                return this._scriptPauseContinueCommand;
            }
        }

        public bool ScriptPlaying
        {
            get
            {
                return _scriptPlaying;
            }
            set
            {
                _scriptPlaying = value;

                OnPropertyChanged("ScriptPlaying");
            }
        }

        public ICommand ScriptStopCommand
        {
            get
            {
                if (this._scriptStopCommand == null)
                    this._scriptStopCommand = new RelayCommand(() => ScriptStop());

                return this._scriptStopCommand;
            }
        }

        public int SelectedMenuTab
        {
            get
            {
                return _selectedMenuTab;
            }
            set
            {
                if ((0 == _selectedMenuTab) && (_selectedMenuTab != value))
                {
                    //update current modality before switching away from CaptureSetup
                    ResourceManagerCS.Instance.ActiveModality = ResourceManagerCS.GetModality();
                }
                _selectedMenuTab = value;
                OnPropertyChanged("SelectedMenuTab");
            }
        }

        public ICommand SupportCommand
        {
            get
            {
                if (this._supportCommand == null)
                    this._supportCommand = new RelayCommand(() => Support());

                return this._supportCommand;
            }
        }

        public ICommand WebUpdateCommand
        {
            get
            {
                if (this._webUpdateCommand == null)
                    this._webUpdateCommand = new RelayCommand(() => WebUpdate());

                return this._webUpdateCommand;
            }
        }

        #endregion Properties

        #region Methods

        public void About()
        {
            System.Reflection.Assembly assembly = System.Reflection.Assembly.GetExecutingAssembly();
            Version version = assembly.GetName().Version;
            if (ResourceManagerCS.Instance.TabletModeEnabled)
            {
                AboutDll.TabletSplashScreen splash = new AboutDll.TabletSplashScreen(String.Format("v{0}.{1}.{2}.{3}", version.Major, version.Minor, version.Build, version.Revision));
                splash.Show();
            }
            else
            {
                AboutDll.SplashScreen splash = new AboutDll.SplashScreen(String.Format("v{0}.{1}.{2}.{3}", version.Major, version.Minor, version.Build, version.Revision));
                splash.Show();
            }
        }

        public void CaptureSetup()
        {
            if (!_noLSMImageDetectors)
            {
                Command command = new Command();
                command.Message = "Capture Setup";
                command.CommandGUID = new Guid("6ecb028e-754e-4b50-b0ef-df8f344b668e");

                _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);
            }
        }

        public void ChangeEventHandler(ChangeEvent change)
        {
            if (change.ModuleName == "RunSampleLS" ||
                change.ModuleName == "ImageReview" ||
                change.ModuleName == "ScriptManager" ||
                change.ModuleName == "ScriptProcessing")
            {
                //check the message of the incoming event to modify the status of the menu bar button

                _captureSetupButtonStatus = (change.IsChanged) ? true : false;
                OnPropertyChanged("CaptureSetupButtonStatus");

                _captureButtonStatus = (change.IsChanged) ? true : false;
                OnPropertyChanged("CaptureButtonStatus");

                _reviewButtonStatus = (change.IsChanged) ? true : false;
                OnPropertyChanged("ReviewButtonStatus");

                _scriptButtonStatus = (change.IsChanged) ? true : false;
                OnPropertyChanged("ScriptButtonStatus");

                _fileMenuStatus = (change.IsChanged) ? true : false;
                OnPropertyChanged("FileMenuStatus");

                _editMenuStatus = (change.IsChanged) ? true : false;
                OnPropertyChanged("EditMenuStatus");

                _helpMenuStatus = (change.IsChanged) ? true : false;
                OnPropertyChanged("HelpMenuStatus");

                _hwSetupButtonStatus = (change.IsChanged) ? true : false;
                OnPropertyChanged("HWSetupButtonStatus");

                _modalityComboBoxStatus = (change.IsChanged) ? true : false;
                OnPropertyChanged("ModalityComboBoxStatus");

            }
            else
            {
                return;
            }
        }

        public void CommandFinishedDialogHandler(Command command)
        {
        }

        public void CommandFinishedScriptHandler(Command command)
        {
            ScriptPlaying = false;
            Paused = false;
            _scriptCommandCounter = 0;

            if (null != ModalityPrevious)
            {
                try
                {
                    //replace the Active Modality with the original modality even if they are the same,
                    //so that original modality settings can be pushed down
                    ResourceManagerCS.FileCopyWithExistCheck(ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "\\Modalities\\" + ModalityPrevious + "\\Active.xml", ResourceManagerCS.GetCaptureTemplatePathString() + "\\Active.xml", true);
                    ResourceManagerCS.SetActiveModality = ModalityPrevious;
                    NeedToReconnectCamera = true;
                }
                catch (Exception ex)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Replace modality failed, error: " + ex.Message);
                }
            }
        }

        public void CommandReviewModalityHandler(Command command)
        {
            ReviewModality = command.Message;
        }

        public void CommandShowDialogHandler(Command command)
        {
            if (command.Payload != null)
            {
                if (command.Payload.Count > 1)
                {
                    if (command.Payload[0].Equals("RunImmediately") || command.Payload[0].Equals("ScriptWaiting"))
                    {
                        XmlDocument doc = new XmlDocument();

                        doc.LoadXml(command.Payload[1]);

                        const string CMD = "/Command";
                        string name = string.Empty;
                        string description = string.Empty;

                        XmlNode node = doc.SelectSingleNode(CMD);
                        if (null != node)
                        {
                            if (XmlManager.GetAttribute(node, doc, "name", ref name))
                                ScriptCommandName = name;
                            if (XmlManager.GetAttribute(node, doc, "description", ref description))
                                ScriptCommandName += " | " + description;
                        }
                        ScriptPlaying = true;
                        if (0 == _scriptCommandCounter)
                        {
                            //Save the first active modality before the script starts.
                            ModalityPrevious = ResourceManagerCS.Instance.ActiveModality;
                        }
                        _scriptCommandCounter++;
                    }
                }
            }
        }

        public void FileExit()
        {
            Application.Current.MainWindow.Close();
        }

        public void HardwareSetupLS()
        {
            ResourceManagerCS.Instance.ActiveModality = ResourceManagerCS.GetModality(); //update current modality before edit->Settings

            RefreshCaptureSetup(); // refresh panel when hardware setting window is loaded.

            Command command = new Command();
            command.Message = "HardwareSetupLS";
            command.CommandGUID = new Guid("64c695e6-8959-496c-91f7-5a9a95d91e0d");

            _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);
        }

        public void ImageReview()
        {
            Command command = new Command();
            command.Message = "ImageReview";
            command.CommandGUID = new Guid("c31db764-799d-4c19-95f5-ed1a514b3c3b");

            _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);
        }

        // reload camera into capturesetup
        public void RefreshCameraAndCaptureSetup()
        {
            Command command = new Command();
            command.Message = "ReloadCamera";
            command.CommandGUID = new Guid("{6ecb028e-754e-4b50-b0ef-df8f344b668e}");

            _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);
        }

        // refresh CaptureSetup UI
        public void RefreshCaptureSetup()
        {
            if (!_noLSMImageDetectors)
            {
                Command command = new Command();
                command.Message = "Capture Setup";
                command.CommandGUID = new Guid("{6ecb028e-754e-4b50-b0ef-df8f344b668e}");

                _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);
            }
        }

        public void RunSampleLS()
        {
            if (!_noLSMImageDetectors)
            {
                Command command = new Command();
                command.Message = "Run Sample LS";
                command.CommandGUID = new Guid("30db4357-7508-46c9-84eb-3ca0900aa4c5");

                _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);
            }
        }

        public void Script()
        {
            if (!_noLSMImageDetectors)
            {
                Command command = new Command();
                command.Message = "ScriptManager";
                command.CommandGUID = new Guid("1F914CCD-33DE-4f40-907A-4511AA145D8A");

                _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);
            }
        }

        public void SubscribeToMenuModuleLSChangeEvent()
        {
            MenuModuleChangeEvent changeEvent = _eventAggregator.GetEvent<MenuModuleChangeEvent>();

            if (_subscriptionToken != null)
            {
                changeEvent.Unsubscribe(_subscriptionToken);
            }

            _subscriptionToken = changeEvent.Subscribe(ChangeEventHandler, ThreadOption.UIThread, true);
        }

        public void SubscribeToShowDialogEvent()
        {
            CommandShowDialogEvent commandEvent = _eventAggregator.GetEvent<CommandShowDialogEvent>();

            if (_subscriptionTokenShowDialog != null)
            {
                commandEvent.Unsubscribe(_subscriptionTokenShowDialog);
            }

            _subscriptionTokenShowDialog = commandEvent.Subscribe(CommandShowDialogHandler, ThreadOption.UIThread, false);
        }

        public void Support()
        {
            try
            {
                Process proc = new Process();
                proc.StartInfo.FileName = "ThorlabsSupport.exe";
                proc.Start();

            }
            catch
            {
                MessageBoxResult result = MessageBox.Show("Thorlabs QuickSupport could not be found in the current folder.", System.Environment.CurrentDirectory, MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        public void WebUpdate()
        {
            Process proc = new Process();

            proc.StartInfo.FileName = "WebUpdater.exe";
            proc.StartInfo.Arguments = string.Format("\"{0}\"", System.Reflection.Assembly.GetEntryAssembly().Location);
            proc.Start();
        }

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamLong")]
        private static extern int GetCameraParamInt(int cameraSelection, int param, ref int value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamLong")]
        private static extern int SetDeviceParamInt(int deviceSelection, int paramId, int param, bool wait);

        private void CheckForUpdates()
        {
            ServicePointManager.Expect100Continue = true;
            ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls12;

            using (WebClient client = new WebClient())
            {
                client.DownloadDataAsync(new Uri("https://www.thorlabs.com/Software/ThorImageLS%20Reference/ThorImageLS.xml"),
                                    ResourceManagerCS.GetMyDocumentsThorImageFolderString());
                client.DownloadDataCompleted += client_DownloadDataCompleted;
            }
        }

        void client_DownloadDataCompleted(object sender, DownloadDataCompletedEventArgs e)
        {
            if (e.Error != null)
            {
                MessageBox.Show("Unable to check for updates. Please check your network connection.");
                return;
            }

            byte[] raw = e.Result;

            string result = System.Text.Encoding.UTF8.GetString(raw);

            XmlDocument doc = new XmlDocument();

            doc.LoadXml(result);

            XmlNodeList ndList = doc.SelectNodes("/Software/Version");

            string str = string.Empty;

            if (XmlManager.GetAttribute(ndList[0], doc, "value", ref str))
            {
                Version v = Assembly.GetExecutingAssembly().GetName().Version;
                Version v_p = new Version(str);

                string strMessage = string.Format("Your software is up to date. Current ThorImageLS version {1}.{2}.{3}.{4}", str, v.Major, v.Minor, v.Build, v.Revision);
                if (v_p.CompareTo(v) > 0)
                    strMessage = string.Format("New Version Available!\rThorImageLS version available {0}\rCurrent ThorImageLS version {1}.{2}.{3}.{4}\rContact ImagingTechSupport@thorlabs.com today to schedule your free software upgrade", str, v.Major, v.Minor, v.Build, v.Revision);

                MessageBox.Show(strMessage, "Check For Updates", MessageBoxButton.OK);
            }
        }

        private void HardwareConnectDlg()
        {
            //Return to the CaptureSetup panel and reload active camera
            RefreshCameraAndCaptureSetup();

            //Stop all of the reading of device position while the hardware setup dialog is open
            Command command = new Command();
            command.Message = "DisableHardwareReading";
            command.CommandGUID = new Guid("8958AFB8-030C-43D5-B190-78130EA0D5A5");
            _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);

            //update current modality before switching away from CaptureSetup
            ResourceManagerCS.Instance.ActiveModality = ResourceManagerCS.GetModality();

            HardwareConnections hc = new HardwareConnections();
            hc.DataContext = this;
            hc.Topmost = true;
            hc.ShowDialog();

            //settings could have been changed, do load
            MVMManager.Instance.LoadSettings();
            //Before loading capture setup, load any possible changes to the list of modalities
            ResourceManagerCS.LoadModalities();
            RefreshCameraAndCaptureSetup();
            NeedToReconnectCamera = false;
        }

        private void LogFile()
        {
            LogFile logWindow = new LogFile();

            logWindow.Show();
        }

        private void MenuLSStart()
        {
            this._MenuLS.Start();
        }

        private void MenuLSStop()
        {
            this._MenuLS.Stop();
        }

        void MenuLS_Update(string statusMessage)
        {
        }

        private void ScriptPauseContinue()
        {
            Command command = new Command();
            command.Message = "ScriptManager";
            command.CommandGUID = new Guid("1F914CCD-33DE-4f40-907A-4511AA145D8A");
            command.Payload = new List<string>();
            command.Payload.Add("PauseContinue");
            _eventAggregator.GetEvent<CommandFinishedDialogEvent>().Publish(command);

            Paused = !Paused;
        }

        private void ScriptStop()
        {
            //for safety stop all stages when stopping the script
            SetDeviceParamInt((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_X_STOP, 1, false);
            SetDeviceParamInt((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_Y_STOP, 1, false);
            SetDeviceParamInt((int)SelectedHardware.SELECTED_ZSTAGE, (int)IDevice.Params.PARAM_Z_STOP, 1, false);
            SetDeviceParamInt((int)SelectedHardware.SELECTED_ZSTAGE2, (int)IDevice.Params.PARAM_Z_STOP, 1, false);

            Command command = new Command();
            command.Message = "ScriptManager";
            command.CommandGUID = new Guid("1F914CCD-33DE-4f40-907A-4511AA145D8A");
            command.Payload = new List<string>();
            command.Payload.Add("Stop");
            _eventAggregator.GetEvent<CommandFinishedDialogEvent>().Publish(command);
        }

        private void SubscribeToFinishedDialogEvent()
        {
            CommandFinishedDialogEvent commandEvent = _eventAggregator.GetEvent<CommandFinishedDialogEvent>();

            if (_subscriptionTokenFinishedDialog != null)
            {
                commandEvent.Unsubscribe(_subscriptionTokenFinishedDialog);
            }

            _subscriptionTokenFinishedDialog = commandEvent.Subscribe(CommandFinishedDialogHandler, ThreadOption.UIThread, false);
        }

        private void SubscribeToFinishedScriptEvent()
        {
            CommandFinishedScriptEvent commandEvent = _eventAggregator.GetEvent<CommandFinishedScriptEvent>();

            if (_subscriptionTokenFinishedScript != null)
            {
                commandEvent.Unsubscribe(_subscriptionTokenFinishedScript);
            }

            _subscriptionTokenFinishedScript = commandEvent.Subscribe(CommandFinishedScriptHandler, ThreadOption.UIThread, false);
        }

        private void SubscribeToReviewModalityEvent()
        {
            CommandReviewModalityEvent commandEvent = _eventAggregator.GetEvent<CommandReviewModalityEvent>();

            if (_subscriptionTokenCommandReviewModality != null)
            {
                commandEvent.Unsubscribe(_subscriptionTokenCommandReviewModality);
            }

            _subscriptionTokenCommandReviewModality = commandEvent.Subscribe(CommandReviewModalityHandler, ThreadOption.UIThread, false);
        }

        #endregion Methods
    }
}