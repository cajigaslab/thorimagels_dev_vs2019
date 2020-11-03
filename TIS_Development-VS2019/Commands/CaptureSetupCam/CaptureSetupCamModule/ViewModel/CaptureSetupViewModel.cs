namespace CaptureSetupDll.ViewModel
{
    using System;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Resources;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;
    using System.Xml.Serialization;

    using CaptureSetupDll.Model;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorImageInfastructure;

    using ThorLogging;

    /// <summary>
    /// ViewModel class for the CaptureSetup model object
    /// </summary>
    public class CaptureSetupViewModel : ViewModelBase
    {
        #region Fields

        // wrapped CaptureSetup object
        private readonly CaptureSetup _CaptureSetup;

        private IUnityContainer _container;
        private IEventAggregator _eventAggregator;
        private IRegionManager _regionManager;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the CaptureSetupViewModel class
        /// </summary>
        /// <param name="CaptureSetup">Wrapped CaptureSetup object</param>
        public CaptureSetupViewModel(IEventAggregator eventAggregator, IRegionManager regionManager, IUnityContainer container, CaptureSetup CaptureSetup)
        {
            this._eventAggregator = eventAggregator;
            this._regionManager = regionManager;
            this._container = container;

            if (CaptureSetup != null)
            {
                this._CaptureSetup = CaptureSetup;
            }
            else
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " CaptureSetup is null. Creating a new CaptureSetup object.");
                CaptureSetup = new CaptureSetup();

                if (CaptureSetup == null)
                {
                    ResourceManager rm = new ResourceManager("CaptureSetupModule.Properties.Resources", Assembly.GetExecutingAssembly());
                    ThorLog.Instance.TraceEvent(TraceEventType.Critical, 1, this.GetType().Name + " " + rm.GetString("CreateCaptureSetupModelFailed"));
                    throw new NullReferenceException("CaptureSetup");
                }

                this._CaptureSetup = CaptureSetup;
            }

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Properties

        /// <summary>
        /// Gets the wrapped CaptureSetup object
        /// </summary>
        public CaptureSetup CaptureSetup
        {
            get
            {
                return this._CaptureSetup;
            }
        }

        public Guid CommandGuid
        {
            get { return this._CaptureSetup.CommandGuid; }
        }

        public String ExpPath
        {
            get
            {
                return this._CaptureSetup.ExpPath;

            }

            set
            {
                this._CaptureSetup.ExpPath = value;
                PublishChangeEvent();
            }
        }

        #endregion Properties

        #region Methods

        public void PublishChangeEvent()
        {
            ChangeEvent changeEvent = new ChangeEvent();
            changeEvent.ModuleName = "CaptureSetup";
            changeEvent.IsChanged = true;

            //command published to change the status of the menu buttons in the Menu Control
            _eventAggregator.GetEvent<MenuModuleChangeEvent>().Publish(changeEvent);
        }

        #endregion Methods
    }
}