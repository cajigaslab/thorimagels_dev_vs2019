using Microsoft.Practices.Composite.Modularity;
using Microsoft.Practices.Composite.Regions;
using Microsoft.Practices.Unity;
using System.Diagnostics;
using System.Globalization;
using HelloWorldInfastructure;
using Microsoft.Practices.Composite.Events;
using Microsoft.Practices.Composite.Wpf.Events;

namespace HelloWorldModule
{
    [Module(ModuleName = "HelloWorldModule")]
    public class HelloWorldModule : IModule
    {
        public IUnityContainer Container { get; private set; }
        public IRegionManager RegionManager { get; private set; }

        public HelloWorldModule(IUnityContainer container, IRegionManager regionManager,IEventAggregator eventAggregator)
        {
            Container = container;
            RegionManager = regionManager;
        }

        public void Initialize()
        {
            HelloWorldView helloWorldView = Container.Resolve<HelloWorldView>();

            helloWorldView.Message = "Test";

            IRegion settingsRegion = RegionManager.Regions["SettingsRegion"];
            settingsRegion.Add(helloWorldView,"HelloWorldView");
        }



    }
}
