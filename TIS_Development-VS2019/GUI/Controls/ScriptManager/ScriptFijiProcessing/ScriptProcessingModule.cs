namespace ScriptFijiProcessingDll
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

    using ScriptFijiProcessingDll;

    using ThorImageInfastructure;

    [Module(ModuleName = "ScriptProcessing")]
    public class ScriptFijiProcessing : IModule
    {
        #region Fields

        private SubscriptionToken subscriptionToken;
        private IEventAggregator _eventAggregator;

        #endregion Fields

        #region Constructors

        public ScriptFijiProcessing(IUnityContainer container, IRegionManager regionManager,IEventAggregator eventAggregator)
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

        public void CommandEventHandler(Command command)
        {
            //check the guid of the incomming event
            Guid guid = new Guid("C1391459-132F-45ea-AE72-D7BEB2BD8086");

            if (command.CommandGUID != guid)
            {
                return;
            }

            if ((null != command.Payload) && (command.Payload.Count > 0))
            {
                if (command.Payload[0].Equals("Complete"))
                {
                }
                else
                {
                    foreach (object view in new List<object>(RegionManager.Regions["SettingsRegion"].Views))
                    {
                        this.SettingsRegion.Remove(view);
                    }

                    this.SettingsRegion.Add(this.MainWindowView, "ScriptProcessing", true);
                    this.SettingsRegion.Activate(this.MainWindowView);
                }
            }
            else
            {
                    foreach (object view in new List<object>(RegionManager.Regions["SettingsRegion"].Views))
                    {
                        this.SettingsRegion.Remove(view);
                    }

                    this.SettingsRegion.Add(this.MainWindowView, "ScriptProcessing", true);
                    this.SettingsRegion.Activate(this.MainWindowView);
            }
        }

        public void Initialize()
        {
            SubscribeToCommandEvent();
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

        #endregion Methods
    }
}