namespace HardwareSetupDll
{
    using System;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Windows;
    using System.Windows.Input;

    using HardwareSetupDll.ViewModel;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorImageInfastructure;

    using ThorLogging;

    [Module(ModuleName = "HardwareSetupModuleLS")]
    public class HardwareSetupModuleLS : IModule
    {
        #region Fields

        private IEventAggregator _eventAggregator;
        private ICommand _refreshCommand;
        private SubscriptionToken _subscriptionToken;

        #endregion Fields

        #region Constructors

        public HardwareSetupModuleLS(IUnityContainer container, IRegionManager regionManager, IEventAggregator eventAggregator)
        {
            Container = container;
            RegionManager = regionManager;
            this._eventAggregator = eventAggregator;
        }

        #endregion Constructors

        #region Properties

        public IUnityContainer Container
        {
            get;
            private set;
        }

        public ICommand RefreshCommand
        {
            get
            {
                if (this._refreshCommand == null)
                    this._refreshCommand = new RelayCommand((x) => RefereshCaptureSetup());

                return this._refreshCommand;
            }
        }

        public IRegionManager RegionManager
        {
            get;
            private set;
        }

        #endregion Properties

        #region Methods

        public void CommandEventHandler(Command command)
        {
            //check the guid of the incomming event
            Guid guid = new Guid("64c695e6-8959-496c-91f7-5a9a95d91e0d");

            if (command.CommandGUID != guid)
            {
                return;
            }

            MainWindow mwView = Container.Resolve<MainWindow>();

            bool? ret = mwView.ShowDialog();
        }

        public void Initialize()
        {
            SubscribeToCommandEvent();
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        // refresh CaptureSetup UI after display option modified
        public void RefereshCaptureSetup()
        {
            Command command = new Command();
            command.Message = "Menu Module LS";
            command.CommandGUID = new Guid("{A20F0FAD-2245-42BE-94EB-96F0D03A0527}");

            _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);
        }

        public void SubscribeToCommandEvent()
        {
            CommandShowDialogEvent commandEvent = _eventAggregator.GetEvent<CommandShowDialogEvent>();

            if (_subscriptionToken != null)
            {
                commandEvent.Unsubscribe(_subscriptionToken);
            }

            _subscriptionToken = commandEvent.Subscribe(CommandEventHandler, ThreadOption.UIThread, true);
        }

        #endregion Methods
    }
}