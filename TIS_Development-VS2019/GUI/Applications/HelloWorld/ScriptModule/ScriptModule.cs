
using Microsoft.Practices.Composite.Modularity;
using Microsoft.Practices.Composite.Regions;
using Microsoft.Practices.Unity;

namespace ScriptModule
{
    public class ScriptModule : IModule
    {
        public ScriptModule(IUnityContainer container, IRegionManager regionManager)
        {
            Container = container;
            RegionManager = regionManager;
        }

        public void Initialize()
        {
            ScriptView scriptView = Container.Resolve<ScriptView>();

            IRegion testRegion = RegionManager.Regions["TestRegion"];
            testRegion.Add(scriptView);
        }

        public IUnityContainer Container { get; private set; }
        public IRegionManager RegionManager { get; private set; }

    }
}
