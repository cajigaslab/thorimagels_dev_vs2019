using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using HelloWorldInfastructure;
using Microsoft.Practices.Composite.Events;

namespace ScriptModule
{
    public class ShowHelloPresenter
    {
        private IShowHelloView _view;
        private IEventAggregator eventAggregator;

        public ShowHelloPresenter(IEventAggregator eventAggregator)
        {
            this.eventAggregator = eventAggregator;
        }

        void ShowHello(object sender, EventArgs e)
        {
            Command command = new Command();
            command.Message = "NewCommand";

            eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);
        }

        public IShowHelloView View
        {
            get { return _view; }
            set
            {
                _view = value;
                _view.ShowHello += ShowHello;
            }
        }
    }
}
