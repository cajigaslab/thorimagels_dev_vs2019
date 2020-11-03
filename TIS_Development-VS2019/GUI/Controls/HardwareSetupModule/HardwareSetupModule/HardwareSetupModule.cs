using Microsoft.Practices.Composite.Modularity;
using Microsoft.Practices.Composite.Regions;
using Microsoft.Practices.Unity;
using System;
using System.Windows;
using System.IO;
using System.Diagnostics;
using System.Globalization;
using ThorImageInfastructure;
using Microsoft.Practices.Composite.Events;
using Microsoft.Practices.Composite.Wpf.Events;
using ThorLogging;

namespace HardwareSetupDll
{
    [Module(ModuleName = "HardwareSetupModule")]
    public class HardwareSetupModule : IModule
    {
        private IEventAggregator _eventAggregator;
        private SubscriptionToken _subscriptionToken;
        public IUnityContainer Container { get; private set; }
        public IRegionManager RegionManager { get; private set; }

        public HardwareSetupModule(IUnityContainer container, IRegionManager regionManager,IEventAggregator eventAggregator)
        {
            Container = container;
            RegionManager = regionManager;
            this._eventAggregator = eventAggregator;
        }

        public void Initialize()
        {
            SubscribeToCommandEvent();
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }


        public void CommandEventHandler(Command command)
        {
            //check the guid of the incomming event
            Guid guid = new Guid("8F8DF2CE-3911-4730-8EC0-32C4AC8E4FD7");

            if (command.CommandGUID != guid)
            {
                return;
            }

            MainWindow mwView = Container.Resolve<MainWindow>();

            bool? ret = mwView.ShowDialog();

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

    }
}
