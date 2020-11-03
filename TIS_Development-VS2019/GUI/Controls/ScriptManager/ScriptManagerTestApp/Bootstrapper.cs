namespace ScriptManagerTestApp
{
    using System;
    using System.Windows;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.UnityExtensions;

    using ThorImageInfastructure;

    class Bootstrapper : UnityBootstrapper
    {
        #region Methods

        public void ShowInitialPanel()
        {
            Command command = new Command();
            command.Message = "ScriptManager";
            command.CommandGUID = new Guid("1F914CCD-33DE-4f40-907A-4511AA145D8A");

            var eventAggregator = Container.Resolve<IEventAggregator>();
            var viewRequestedEvent = eventAggregator.GetEvent<CommandShowDialogEvent>();
            viewRequestedEvent.Publish(command);
        }

        protected override DependencyObject CreateShell()
        {
            Shell shell = new Shell();
            shell.Show();
            return shell;
        }

        protected override IModuleEnumerator GetModuleEnumerator()
        {
            return new DirectoryLookupModuleEnumerator(@".\");
        }

        #endregion Methods
    }
}