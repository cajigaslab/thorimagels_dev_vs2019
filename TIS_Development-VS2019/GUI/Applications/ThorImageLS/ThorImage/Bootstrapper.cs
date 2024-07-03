namespace ThorImage
{
    using System;
    using System.Windows;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.UnityExtensions;
    using Microsoft.Practices.Composite.Wpf.Events;

    using ThorImageInfastructure;

    class Bootstrapper : UnityBootstrapper
    {
        #region Fields

        private Shell _shell;

        #endregion Fields

        #region Methods

        public void ShowInitialPanel()
        {
            _shell.Show();
            Command command = new Command();
            command.Message = "Capture Setup";
            command.CommandGUID = new Guid("6ecb028e-754e-4b50-b0ef-df8f344b668e");

            var eventAggregator = Container.Resolve<IEventAggregator>();
            var viewRequestedEvent = eventAggregator.GetEvent<CommandShowDialogEvent>();
            viewRequestedEvent.Publish(command);
        }

        protected override DependencyObject CreateShell()
        {
            _shell = new Shell();
            return _shell;
        }

        protected override IModuleEnumerator GetModuleEnumerator()
        {
            return new DirectoryLookupModuleEnumerator(@".\Modules");
        }

        #endregion Methods
    }
}