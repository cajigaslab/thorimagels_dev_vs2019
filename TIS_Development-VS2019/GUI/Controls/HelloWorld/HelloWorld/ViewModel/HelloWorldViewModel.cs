namespace HelloWorldDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Resources;
    using System.Text;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;
    using System.Xml.Serialization;

    using HelloWorldDll.Model;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorImageInfastructure;

    using ThorLogging;


    /// <summary>
    /// ViewModel class for the ExpSetup model object
    /// </summary>
    public class HelloWorldViewModel : ViewModelBase
    {
        #region Fields

        // wrapped HelloWorld object
        private readonly HelloWorld _HelloWorld;

        private IUnityContainer _container;
        private IEventAggregator _eventAggregator;
        private IRegionManager _regionManager;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the ExpSetupViewModel class
        /// </summary>
        /// <param name="ExpSetup">Wrapped ExpSetup object</param>
        public HelloWorldViewModel(IEventAggregator eventAggregator, IRegionManager regionManager, IUnityContainer container, HelloWorld HelloWorld)
        {
            this._eventAggregator = eventAggregator;
            this._regionManager = regionManager;
            this._container = container;

            if (HelloWorld != null)
            {
                this._HelloWorld = HelloWorld;
            }
            else
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " ExpSetup is null. Creating a new HelloWorld object.");

                HelloWorld = new HelloWorld();

                if (HelloWorld == null)
                {
                    ResourceManager rm = new ResourceManager("HelloWorld.Properties.Resources", Assembly.GetExecutingAssembly());
                    ThorLog.Instance.TraceEvent(TraceEventType.Critical, 1, this.GetType().Name + " " + rm.GetString("CreateHelloWorldViewModelFailed"));
                    throw new NullReferenceException("HelloWorld");
                }

                this._HelloWorld = HelloWorld;
            }

            EnableHandlers();

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Enumerations

        #endregion Enumerations

        #region Events
        #endregion Events

        #region Properties

        /// <summary>
        /// Gets the wrapped HelloWorld object
        /// </summary>
        public HelloWorld HelloWorld
        {
            get
            {
                return this._HelloWorld;
            }
        }

        #endregion Properties

        #region Methods

        private BackgroundWorker _bw = new BackgroundWorker();

        public void EnableHandlers()
        {
            _bw.ProgressChanged += new ProgressChangedEventHandler(_bw_ProgressChanged);
            _bw.DoWork += new DoWorkEventHandler(_bw_DoWork);
            _bw.WorkerReportsProgress = true;
            _bw.WorkerSupportsCancellation = true;
        }

        void _bw_DoWork(object sender, DoWorkEventArgs e)
        {
        }

        void _bw_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
        }


        public void ReleaseHandlers()
        {
            _bw.ProgressChanged -= new ProgressChangedEventHandler(_bw_ProgressChanged);
            _bw.DoWork -= new DoWorkEventHandler(_bw_DoWork);
        }

        private ICommand _myCommand;

        public ICommand MyCommand
        {
            get
            {
                if (this._myCommand == null)
                    this._myCommand = new RelayCommand(() => DoMyCommand());

                return this._myCommand;
            }
        }

        private void DoMyCommand()
        {
           _buttonState = !_buttonState;
            OnPropertyChanged("ButtonImagePath");
            OnPropertyChanged("ButtonText");
        }

        private bool _buttonState = false;

        public string ButtonImagePath
        {
            get
            {

                if (true == _buttonState)
                {
                    return @"/HelloWorld;component/Icons/Stop.png";
                }
                else
                {
                    return @"/HelloWorld;component/Icons/Play.png";
                }
            }
        }
        public string ButtonText
        {
            get
            {
                if (true == _buttonState)
                {
                    return "Stop";
                }
                else
                {
                    return "Play";
                }
            }
        }


        #endregion Methods
    }
}