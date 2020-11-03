namespace ImageReviewDll
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Windows;
    using System.Windows.Data;

    using ImageReviewDll;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorImageInfastructure;

    using ThorLogging;

    [Module(ModuleName = "ImageReviewModule")]
    public class ImageReviewModule : IModule
    {
        #region Fields

        private SubscriptionToken subscriptionToken;
        private IEventAggregator _eventAggregator;

        #endregion Fields

        #region Constructors

        public ImageReviewModule(IUnityContainer container, IRegionManager regionManager, IEventAggregator eventAggregator)
        {
            Container = container;
            RegionManager = regionManager;
            this._eventAggregator = eventAggregator;

            this.MainWindowView = Container.Resolve<MainWindow>();
            this.SettingsRegion = regionManager.Regions["SettingsRegion"];
        }

        #endregion Constructors

        #region Properties

        public IUnityContainer Container
        {
            get;
            private set;
        }

        public IRegionManager RegionManager
        {
            get;
            private set;
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

        public void CommandEventHandler(Command command)
        {
            //check the guid of the incomming event
            Guid expSetupGUID = new Guid("c31db764-799d-4c19-95f5-ed1a514b3c3b");

            if (command.CommandGUID != expSetupGUID)
            {
                return;
            }

            foreach (object view in new List<object>(RegionManager.Regions["SettingsRegion"].Views))
            {
                this.SettingsRegion.Remove(view);
            }

            this.SettingsRegion.Add(this.MainWindowView, "ImageReview", true);
            this.SettingsRegion.Activate(this.MainWindowView);
        }

        public void Initialize()
        {
            SubscribeToCommandEvent();
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        public void SubscribeToCommandEvent()
        {
            if (null != _eventAggregator)
            {
                CommandShowDialogEvent commandEvent = _eventAggregator.GetEvent<CommandShowDialogEvent>();

                if (subscriptionToken != null)
                {
                    commandEvent.Unsubscribe(subscriptionToken);
                }

                subscriptionToken = commandEvent.Subscribe(CommandEventHandler, ThreadOption.UIThread, true);
            }
        }

        #endregion Methods
    }
}