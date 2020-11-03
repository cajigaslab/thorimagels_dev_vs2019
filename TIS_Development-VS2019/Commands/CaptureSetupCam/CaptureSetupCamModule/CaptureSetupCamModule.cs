namespace CaptureSetupDll
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Windows;
    using System.Windows.Data;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorImageInfastructure;

    using ThorLogging;

    [Module(ModuleName = "CaptureSetupModule")]
    public class CaptureSetupModule : IModule
    {
        #region Fields

        private SubscriptionToken subscriptionToken;
        private IEventAggregator _eventAggregator;

        #endregion Fields

        #region Constructors

        public CaptureSetupModule(IUnityContainer container, IRegionManager regionManager,IEventAggregator eventAggregator)
        {
            Container = container;
            this.RegionManager = regionManager;
            this._eventAggregator = eventAggregator;

            this.MainWindowView = Container.Resolve<MainWindow>();
            this.SettingsRegion = regionManager.Regions["SettingsRegion"];
        }

        #endregion Constructors

        #region Properties

        public IUnityContainer Container
        {
            get; private set;
        }

        public IRegionManager RegionManager
        {
            get; private set;
        }

        private MainWindow MainWindowView
        {
            get; set;
        }

        private IRegion SettingsRegion
        {
            get; set;
        }

        #endregion Properties

        #region Methods

        public void ChangeEventHandler(ChangeEvent change)
        {
            if (change.IsChanged)
            {
                MainWindow mw = new MainWindow();
                mw.UpdateHardwareListView();
            }
            else
            {
                return;
            }
        }

        public void CommandEventHandler(Command command)
        {
            //check the guid of the incomming event
            Guid CaptureSetupGUID = new Guid("6ecb028e-754e-4b50-b0ef-df8f344b668e");

            if (command.CommandGUID != CaptureSetupGUID)
            {
                return;
            }

            foreach (object view in new List<object>(RegionManager.Regions["SettingsRegion"].Views))
            {
                this.SettingsRegion.Remove(view);
            }

            this.SettingsRegion.Add(this.MainWindowView, "CaptureSetup", true);
            this.SettingsRegion.Activate(this.MainWindowView);
        }

        public void Initialize()
        {
            SubscribeToCommandEvent();
            SubscribeToHardwareSettingsChangeEvent();
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        public void SubscribeToCommandEvent()
        {
            CommandShowDialogEvent commandEvent = _eventAggregator.GetEvent<CommandShowDialogEvent>();

            if (subscriptionToken != null)
            {
                commandEvent.Unsubscribe(subscriptionToken);
            }

            subscriptionToken = commandEvent.Subscribe(CommandEventHandler, ThreadOption.UIThread, true);
        }

        public void SubscribeToHardwareSettingsChangeEvent()
        {
            HardwareSettingsChangeEvent changeEvent = _eventAggregator.GetEvent<HardwareSettingsChangeEvent>();

            if (subscriptionToken != null)
            {
                changeEvent.Unsubscribe(subscriptionToken);
            }

            subscriptionToken = changeEvent.Subscribe(ChangeEventHandler, ThreadOption.UIThread, true);
        }

        #endregion Methods
    }
}