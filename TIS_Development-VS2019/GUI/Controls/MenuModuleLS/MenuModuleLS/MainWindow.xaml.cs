namespace MenuLSDll
{
    using System;
    using System.Diagnostics;
    using System.Windows.Controls;

    using MenuLSDll.ViewModel;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorImageInfastructure;

    using ThorLogging;

    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class MainWindow : UserControl
    {
        #region Fields

        private MenuLSViewModel viewModel;
        private IEventAggregator _eventAggregator;
        private SubscriptionToken _subscriptionToken;
        private SubscriptionToken _subscriptionTokenFinishedScript;

        #endregion Fields

        #region Constructors

        public MainWindow()
        {
            InitializeComponent();
        }

        public MainWindow(MenuLSViewModel MenuLSViewModel, IEventAggregator eventAggregator)
            : this()
        {
            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this) == true)
                return;

            this.viewModel = MenuLSViewModel;
            this._eventAggregator = eventAggregator;

            // create the ViewModel object and setup the DataContext to it
            this.MenuLSView.DataContext = MenuLSViewModel;


            SubscribeToCommandEvent();
            SubscribeToFinishedScriptEvent();
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Methods

        /// <summary>
        /// Show or hide the  MenuLS view
        /// </summary>
        /// <param name="command"></param>
        public void CommandEventHandler(Command command)
        {
            //check the guid of the incomming event
            Guid RunSampleGUID = new Guid("30db4357-7508-46c9-84eb-3ca0900aa4c5");
            Guid MenuModuleLSGUID = new Guid("A20F0FAD-2245-42BE-94EB-96F0D03A0527");

            if (command.CommandGUID == MenuModuleLSGUID)
            {
                this.MenuLSView.LoadTabCtrlSettings();
                this.MenuLSView.SetCaptureSetup();
            }
            else if (command.CommandGUID == RunSampleGUID)
            {
                if (command.Payload != null)
                {
                    foreach (string str in command.Payload)
                    {
                        if (str.Equals("RunImmediately"))
                        {
                            this.MenuLSView.SelectCaptureTab();
                        }
                    }
                }
            }
        }

        public void CommandFinishedScriptHandler(Command command)
        {
            this.MenuLSView.SetScriptTab();
        }

        /// <summary>
        /// Capture events for showing the commands view 
        /// </summary>
        public void SubscribeToCommandEvent()
        {
            CommandShowDialogEvent commandEvent = _eventAggregator.GetEvent<CommandShowDialogEvent>();

            if (_subscriptionToken != null)
            {
                commandEvent.Unsubscribe(_subscriptionToken);
            }

            _subscriptionToken = commandEvent.Subscribe(CommandEventHandler, ThreadOption.UIThread, false);
        }

        private void SubscribeToFinishedScriptEvent()
        {
            CommandFinishedScriptEvent commandEvent = _eventAggregator.GetEvent<CommandFinishedScriptEvent>();

            if (_subscriptionTokenFinishedScript != null)
            {
                commandEvent.Unsubscribe(_subscriptionTokenFinishedScript);
            }

            _subscriptionTokenFinishedScript = commandEvent.Subscribe(CommandFinishedScriptHandler, ThreadOption.UIThread, false);
        }

        #endregion Methods

    }
}