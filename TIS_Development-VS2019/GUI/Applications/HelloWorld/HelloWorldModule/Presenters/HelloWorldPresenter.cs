
using System.Diagnostics;
using System.Globalization;
using HelloWorldInfastructure;
using Microsoft.Practices.Composite.Events;
using Microsoft.Practices.Composite.Wpf.Events;
using Microsoft.Practices.Composite.Regions;
using Microsoft.Practices.Unity;

namespace HelloWorldModule
{
    public class HelloWorldPresenter
    {
        private string _message;
        private IEventAggregator eventAggregator;
        private SubscriptionToken subscriptionToken;
        private IRegionManager regionManager;
        private IUnityContainer container;

        public HelloWorldPresenter(IEventAggregator eventAggregator, IRegionManager regionManager,IUnityContainer container)
        {
            this.eventAggregator = eventAggregator;
            this.regionManager = regionManager;
            this.container = container;
        }

        public void CommandShowDialogEventHandler(Command command)
        {
            Debug.Assert(View != null);
            
            IRegion settingsRegion = regionManager.Regions["SettingsRegion"];

            object helloWorldView = settingsRegion.GetView("HelloWorldView");

            if (helloWorldView != null)
            {
                settingsRegion.Remove(helloWorldView);
            }
            else
            {
                HelloWorldView hwView = container.Resolve<HelloWorldView>();

                settingsRegion.Add(hwView, "HelloWorldView");
            }
             
        }

        public IHellowWorldView View { get; set; }

        public string Message
        {
            get { return _message; }
            set
            {
                _message = value;

                CommandShowDialogEvent commandEvent = eventAggregator.GetEvent<CommandShowDialogEvent>();

                if (subscriptionToken != null)
                {
                    commandEvent.Unsubscribe(subscriptionToken);
                }

                subscriptionToken = commandEvent.Subscribe(CommandShowDialogEventHandler, ThreadOption.UIThread, false);
            }
        }


    }
}
