using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Microsoft.Practices.Composite.Events;
using Microsoft.Practices.Composite.Modularity;
using Microsoft.Practices.Composite.Regions;
using Microsoft.Practices.Composite.Wpf.Events;
using Microsoft.Practices.Unity;
using ThorImageInfastructure;
using System.IO;

namespace ScriptRunMatlabModule
{
    [Module(ModuleName = "ScriptRunMatlabModule")]
    public class ScriptRunMatlabModule : IModule
    {
        #region Fileds

        private IEventAggregator _eventAggregator;
        private SubscriptionToken _subscriptionToken;
        private IRegionManager _regionManager;
        private MatlabOutputView _mainView;

        #endregion Fileds

        #region Constructors

        public ScriptRunMatlabModule(IUnityContainer container, IRegionManager regionManager, IEventAggregator eventAggregator)
        {
            _regionManager = regionManager;
            _eventAggregator = eventAggregator;
            _mainView = container.Resolve<MatlabOutputView>();
        }

        #endregion Constructors

        #region Method

        public void Initialize()
        {
            var commandEvent = _eventAggregator.GetEvent<CommandShowDialogEvent>();
            if (_subscriptionToken != null)
            {
                commandEvent.Unsubscribe(_subscriptionToken);
            }
            _subscriptionToken = commandEvent.Subscribe(SwitchView, ThreadOption.UIThread, true);
        }

        private void SwitchView(Command command)
        {
            //check the guid of the incomming event
            Guid guid = new Guid("b3f2f8a4-cf8f-4fce-a195-b163a3c20c75");

            if (command.CommandGUID != guid)
            {
                return;
            }

            var settingsRegion = _regionManager.Regions["SettingsRegion"];
            if ((null != command.Payload) && (command.Payload.Count > 0))
            {
                if (!command.Payload[0].Equals("Complete"))
                {
                    foreach (object view in new List<object>(settingsRegion.Views))
                    {
                        settingsRegion.Remove(view);
                    }

                    settingsRegion.Add(_mainView, "ScriptRunMatlabModule", true);
                    settingsRegion.Activate(_mainView);
                }
            }
            else
            {
                foreach (object view in new List<object>(settingsRegion.Views))
                {
                    settingsRegion.Remove(view);
                }

                settingsRegion.Add(_mainView, "ScriptRunMatlabModules", true);
                settingsRegion.Activate(_mainView);
            }
        }

        #endregion Method
    }
}
