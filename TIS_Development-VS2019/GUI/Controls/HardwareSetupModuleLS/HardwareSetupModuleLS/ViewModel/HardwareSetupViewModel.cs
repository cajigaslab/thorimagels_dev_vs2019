namespace HardwareSetupDll.ViewModel
{
    using System;
    using System.ComponentModel;
    using System.Diagnostics;
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

    using HardwareSetupDll.View;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorImageInfastructure;

    using ThorLogging;

    /// <summary>
    /// ViewModel class for the HardwareSetup model object
    /// </summary>
    public class HardwareSetupViewModel : ViewModelBase
    {
        #region Fields

        public static readonly RoutedCommand CommandOne = new RoutedCommand();
        public static readonly RoutedCommand CommandThree = new RoutedCommand();
        public static readonly RoutedCommand CommandTwo = new RoutedCommand();

        // wrapped HardwareSetup object
        private readonly HardwareSetup _HardwareSetup;

        private ICommand _applicationSettingsCommand;
        private IUnityContainer _container;
        private ICommand _displayOptionsCommand;
        private IEventAggregator _eventAggregator;
        private ICommand _hardwareSettingsCommand;
        private IRegionManager _regionManager;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the HardwareSetupViewModel class
        /// </summary>
        /// <param name="HardwareSetup">Wrapped HardwareSetup object</param>
        public HardwareSetupViewModel(IEventAggregator eventAggregator, IRegionManager regionManager, IUnityContainer container, HardwareSetup HardwareSetup)
        {
            this._eventAggregator = eventAggregator;
            this._regionManager = regionManager;
            this._container = container;

            if (HardwareSetup != null)
            {
                this._HardwareSetup = HardwareSetup;
            }
            else
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " HardwareSetup is null. Creating a new HardwareSetup object.");
                HardwareSetup = new HardwareSetup();

                if (HardwareSetup == null)
                {
                    ResourceManager rm = new ResourceManager("HardwareSetupModuleLS.Properties.Resources", Assembly.GetExecutingAssembly());
                    ThorLog.Instance.TraceEvent(TraceEventType.Critical, 1, this.GetType().Name + " " + rm.GetString("CreateHardwareSetupModelFailed"));
                    throw new NullReferenceException("HardwareSetup");
                }

                this._HardwareSetup = HardwareSetup;
            }

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Properties

        public ICommand ApplicationSettingsCommand
        {
            get
            {
                if (this._applicationSettingsCommand == null)
                    this._applicationSettingsCommand = new RelayCommand((x) => ApplicationSettings());

                return this._applicationSettingsCommand;
            }
        }

        public ICommand DisplayOptionsCommand
        {
            get
            {
                if (this._displayOptionsCommand == null)
                    this._displayOptionsCommand = new RelayCommand((x) => DisplayOptions());

                return this._displayOptionsCommand;
            }
        }

        public ICommand HardwareSettingsCommand
        {
            get
            {
                if (this._hardwareSettingsCommand == null)
                    this._hardwareSettingsCommand = new RelayCommand((x) => HardwareSettings());

                return this._hardwareSettingsCommand;
            }
        }



        /// <summary>
        /// Gets the wrapped HardwareSetup object
        /// </summary>
        public HardwareSetup HardwareSetup
        {
            get
            {
                return this._HardwareSetup;
            }
        }

        public string StreamingPath
        {
            get
            {
                return this._HardwareSetup.StreamingPath;
            }
            set
            {
                this._HardwareSetup.StreamingPath = value;
                OnPropertyChanged("StreamingPath");
            }
        }

        #endregion Properties

        #region Methods

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetApplicationSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        public static extern int GetApplicationSettingsFilePathAndName(StringBuilder sb, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetHardwareSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        public static extern int GetHardwareSettingsFilePathAndName(StringBuilder sb, int length);

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

        private void ApplicationSettings()
        {
            ApplicationSettings appSettings = new ApplicationSettings();

            appSettings.Topmost = true;
            appSettings.ShowDialog();
        }

        private void DisplayOptions()
        {
            DisplayOptions dispOpt = new DisplayOptions();

            dispOpt.Topmost = true;
            dispOpt.ShowDialog();
        }

        private void HardwareSettings()
        {
            HardwareSettings hardwareSettings = new HardwareSettings();

            hardwareSettings.Topmost = true;
            hardwareSettings.ShowDialog();
        }



        #endregion Methods
    }
}