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

namespace MenuLSDll
{
    [Module(ModuleName = "MenuModuleLS")]
    public class MenuModuleLS : IModule
    {
        public IUnityContainer Container { get; private set; }
        public IRegionManager RegionManager { get; private set; }
        private IRegion MenuRegion { get; set; }
        private MainWindow MainWindowView { get; set; }

        public MenuModuleLS(IUnityContainer container, IRegionManager regionManager,IEventAggregator eventAggregator)
        {
            Container = container;
            RegionManager = regionManager;

            this.MainWindowView = Container.Resolve<MainWindow>();
            this.MenuRegion = regionManager.Regions["MenuRegion"];
        }

        public void Initialize()
        {
            this.MenuRegion.Add(this.MainWindowView, "MenuLS", true);
            this.MenuRegion.Activate(this.MainWindowView); 

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

    }
}
