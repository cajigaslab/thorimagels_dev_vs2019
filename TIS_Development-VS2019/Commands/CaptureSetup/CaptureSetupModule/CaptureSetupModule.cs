namespace CaptureSetupDll
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Windows;
    using System.Windows.Data;

    using CaptureSetupDll.ViewModel;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorImageInfastructure;

    //   using Abt.Controls.SciChart.Visuals;
    using ThorLogging;

    [Module(ModuleName = "CaptureSetupModule")]
    public class CaptureSetupModule : IModule
    {
        #region Fields

        private SubscriptionToken subscriptionToken;
        private IEventAggregator _eventAggregator;

        #endregion Fields

        #region Constructors

        public CaptureSetupModule(IUnityContainer container, IRegionManager regionManager, IEventAggregator eventAggregator)
        {
            // Ensure SetLicenseKey is called once, before any SciChartSurface instance is created
            // Check this code into your version-control and it will enable SciChart
            // for end-users of your application.
            //
            // You can test the Runtime Key is installed correctly by Running your application
            // OUTSIDE Of Visual Studio (no debugger attached). Trial watermarks should be removed.

            //            SciChartSurface.SetRuntimeLicenseKey(@"<LicenseContract>
            //            <Customer>Thorlabs</Customer>
            //            <OrderId>ABT150916-7903-84104</OrderId>
            //            <LicenseCount>1</LicenseCount>
            //            <IsTrialLicense>false</IsTrialLicense>
            //            <SupportExpires>01/24/2016 00:00:00</SupportExpires>
            //            <ProductCode>SC-WPF-PRO</ProductCode>
            //            <KeyCode>lwAAAAEAAADkBWp2vvDQAWUAQ3VzdG9tZXI9VGhvcmxhYnM7T3JkZXJJZD1BQlQxNTA5MTYtNzkwMy04NDEwNDtTdWJzY3JpcHRpb25WYWxpZFRvPTI0LUphbi0yMDE2O1Byb2R1Y3RDb2RlPVNDLVdQRi1QUk99dyKgCYuoNoXOUhV2ki594OYNW3lLPWgUFOImlnAvh1jHrW+N0AThdTBNfZJkhIA=</KeyCode>
            //           </LicenseContract>");
            Container = container;
            this.RegionManager = regionManager;
            this._eventAggregator = eventAggregator;

            this.MainWindowView = Container.Resolve<MainWindow>();
            Container.RegisterInstance<MainWindow>(this.MainWindowView);
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
            get;
            set;
        }

        private IRegion SettingsRegion
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        public void ChangeEventHandler(ChangeEvent change)
        {
            if (change.IsChanged)
            {
                MainWindow mw = new MainWindow();
                Container.RegisterInstance<MainWindow>(mw);
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

            Guid HWSetupGUID = new Guid("64c695e6-8959-496c-91f7-5a9a95d91e0d");

            Guid DisableHWReadingGUID = new Guid("8958AFB8-030C-43D5-B190-78130EA0D5A5");

            if (command.CommandGUID == HWSetupGUID)
            {
                this.MainWindowView.DisableDeviceQuery();
            }

            if (command.CommandGUID == DisableHWReadingGUID)
            {
                this.MainWindowView.DisableDeviceQuery();
            }

            if (command.CommandGUID != CaptureSetupGUID)
            {
                return;
            }

            if (command.Message.Contains("ReloadCamera"))
            {
                this.MainWindowView.ReconnectCamera();
            }
            foreach (object view in new List<object>(RegionManager.Regions["SettingsRegion"].Views))
            {
                this.SettingsRegion.Remove(view);
            }

            this.SettingsRegion.Add(this.MainWindowView, "CaptureSetup", true);
            this.SettingsRegion.Activate(this.MainWindowView);
            this.MainWindowView.EnableDeviceQuery();
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