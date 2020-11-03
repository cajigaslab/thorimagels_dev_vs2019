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
            _shell.Closing += _shell_Closing;
            Command command = new Command();
            command.Message = "Capture Setup";
            command.CommandGUID = new Guid("6ecb028e-754e-4b50-b0ef-df8f344b668e");

            var eventAggregator = Container.Resolve<IEventAggregator>();
            var viewRequestedEvent = eventAggregator.GetEvent<CommandShowDialogEvent>();
            viewRequestedEvent.Publish(command);
            StartThorIPC();
        }

        public void ShutDownThorIPC()
        {
            var eventAggregator = Container.Resolve<IEventAggregator>();
            ChangeEvent changeEvent = new ChangeEvent();
            changeEvent.ModuleName = "IPC_STOP";
            changeEvent.IsChanged = true;
            //command published to change the status of the menu buttons in the Menu Control
            eventAggregator.GetEvent<IPCModuleChangeEvent>().Publish(changeEvent);
        }

        public void StartThorIPC()
        {
            var eventAggregator = Container.Resolve<IEventAggregator>();
            ChangeEvent changeEvent = new ChangeEvent();
            changeEvent.ModuleName = "IPC_START";
            changeEvent.IsChanged = true;
            //command published to change the status of the menu buttons in the Menu Control
            eventAggregator.GetEvent<IPCModuleChangeEvent>().Publish(changeEvent);
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

        void _shell_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            ShutDownThorIPC();
            System.Threading.Thread.Sleep(300);
        }

        #endregion Methods
    }
}