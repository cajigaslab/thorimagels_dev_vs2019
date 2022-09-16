namespace ThorIPCModules
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.Reflection;
    using System.Resources;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Threading;
    using System.Xml;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorImageInfastructure;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// IPC Module
    /// </summary>
    [Module(ModuleName = "ThorIPC")]
    public partial class ThorIPCModule : IModule
    {
        #region Fields

        private IEventAggregator _eventAggregator; // Defines an interface to get instances of an event type.
        private SubscriptionToken _subscriptionTokenIPCControl; // IPC On/Off Subscription token
        private SubscriptionToken _subscriptionTokenIPCData; // IPC downlink/uplink Subscription token
        private SubscriptionToken _subscriptionTokenShowDialog; // ThorImage Active Modules Subscription token

        #endregion Fields

        #region Constructors

        public ThorIPCModule(IUnityContainer container, IRegionManager regionManager, IEventAggregator eventAggregator)
        {
            Container = container;
            this.RegionManager = regionManager;
            this._eventAggregator = eventAggregator;

            InitIPCConnectionTimer();
            SetClientServerNames();

            _remoteHostName = GetHostName();
        }

        #endregion Constructors

        #region Properties

        public IUnityContainer Container
        {
            get; private set;
        }

        public IRegionManager RegionManager
        {
            get; private set;
        }

        private IRegion MenuRegion
        {
            get; set;
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// ThorImage Active Modules Change Event
        /// </summary>
        ///
        /// <param name="command">Command inherit from ThorImageInfastructure</param>
        ///
        /// <exception>NONE</exception>
        public void CommandShowDialogHandler(Command command)
        {
            if (command.Message == "Run Sample LS" || command.Message == "Capture Setup" || command.Message == "ImageReview" || command.Message == "ScriptManager" || command.Message == "Capture")
            {
                if (_thorSyncConnection == true && command.Message == "Run Sample LS")
                {
                    sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "Establish"); // connect to thorsync when Capture Mode is Active
                    sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "UpdataInformation", _thorSyncConfiguratureInformation);
                }
                else if (_thorSyncConnection == false  && command.Message == "Run Sample LS")
                {
                    sendData(_eventAggregator, "IPC_CONTROLLER", "RUN_SAMPLE", "TearDown"); // disconnect to thorsync when Capture Mode is Active
                    _uncheckRemoteConnection = false;
                }
                //Fix for remote checkbox being checked even when off after running a script and heading back into Capture tab
                //There is most likely a better fix - better logic as to when pipes should be established or when remote connection booleans are enabled/disabled
                if (_thorSyncConnection == false && command.Message == "ScriptManager")
                {
                    _uncheckRemoteConnection = true;
                }
                //Update remote connection to false when switching back to Capture tab from Script tab
                else if (_uncheckRemoteConnection == true && command.Message == "ScriptManager")
                {
                    _uncheckRemoteConnection = false;
                    _thorSyncConnection = false;
                }
                //Set a boolean to know whether the Capture tab has been entered from Scripting while the remote connection was disabled
                if (command.Message == "Capture" && _uncheckRemoteConnection == true)
                {
                    _captureScriptManager = true;
                }
                //Update the remote connection to false when it was disabled upon entering the Capture tab within a script
                if (_captureScriptManager == true && command.Message == "ScriptManager")
                {
                    _captureScriptManager = false;
                    _thorSyncConnection = false;
                }
                _modeThorImage = command.Message;
            }
        }

        /// <summary>
        /// Subscribe Event to Container
        /// </summary>
        ///
        /// <param>NONE</param>
        ///
        /// <exception>NONE</exception>
        public void Initialize()
        {
            SubscribeToIPCControlEvent();
            SubscribeToShowDialogEvent();
            SubscribeToIPCDataEvent();
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        /// <summary>
        /// Receive IPC On/Off Event
        /// </summary>
        ///
        /// <param name="change">ChangEvent inherit from ThorImageInfastructure</param>
        ///
        /// <exception>NONE</exception>
        public void IPCControlEventHandler(ChangeEvent change)
        {
            if ((change.ModuleName == "IPC_START" || change.ModuleName == "IPC_STOP") && change.IsChanged == true)
            {
               if (change.ModuleName == "IPC_START")
               {
                   StartNamedPipeClient();
               }
               else if (change.ModuleName == "IPC_STOP")
               {
                    DisconnectThorSync();
                    StopNamePipeClient();
               }
            }
        }

        public void SetClientServerNames()
        {
            if (_remoteAppName.CompareTo("") == 0)
            {
                _serverName = "ThorImageThorSyncPipe";
                _clientName = "ThorSyncThorImagePipe";
            }
            else
            {
                _serverName = "ThorImage" + _remoteAppName + "Pipe";
                _clientName = _remoteAppName + "ThorImagePipe";
            }
            if (ResourceManagerCS.Instance.TabletModeEnabled)
            {
                if (_remoteAppName.CompareTo("") == 0)
                {
                    _serverName = "ThorSyncThorImagePipe";
                    _clientName = "ThorImageThorSyncPipe";
                }
                else
                {
                    _serverName = _remoteAppName + "ThorImagePipe";
                    _clientName = "ThorImage" + _remoteAppName + "Pipe";
                }
            }
        }

        /// <summary>
        /// Subscribe IPC message control Event
        /// </summary>
        ///
        /// <param>NONE</param>
        ///
        /// <exception>NONE</exception>
        public void SubscribeToIPCControlEvent()
        {
            IPCModuleChangeEvent changeEvent = _eventAggregator.GetEvent<IPCModuleChangeEvent>();

            if (_subscriptionTokenIPCControl != null)
            {
                changeEvent.Unsubscribe(_subscriptionTokenIPCControl);
            }

            _subscriptionTokenIPCControl = changeEvent.Subscribe(IPCControlEventHandler, ThreadOption.UIThread, true);
        }

        /// <summary>
        /// Subscribe uplink/downlink data/command Event
        /// </summary>
        ///
        /// <param>NONE</param>
        ///
        /// <exception>NONE</exception>
        public void SubscribeToIPCDataEvent()
        {
            CommandIPCEvent commandIPC = _eventAggregator.GetEvent<CommandIPCEvent>();

            if (_subscriptionTokenIPCData != null)
            {
                commandIPC.Unsubscribe(_subscriptionTokenIPCData);
            }

            _subscriptionTokenIPCData = commandIPC.Subscribe(IPCDataEventHandler, ThreadOption.UIThread, true);
        }

        /// <summary>
        /// Subscribe show dialog Event
        /// </summary>
        ///
        /// <param>NONE</param>
        ///
        /// <exception>NONE</exception>
        public void SubscribeToShowDialogEvent()
        {
            CommandShowDialogEvent commandEvent = _eventAggregator.GetEvent<CommandShowDialogEvent>();

            if (_subscriptionTokenShowDialog != null)
            {
                commandEvent.Unsubscribe(_subscriptionTokenShowDialog);
            }

            _subscriptionTokenShowDialog = commandEvent.Subscribe(CommandShowDialogHandler, ThreadOption.UIThread, true);
        }

        /// <summary>
        ///  Uplink/downlink data/command Event
        /// </summary>
        ///
        /// <param name="command">IPCCommand inherit from ThorImageInfastructure</param>
        ///
        /// <exception>NONE</exception>
        private void IPCDataEventHandler(IPCCommand command)
        {
            if (command.Destination != "IPC_CONTROLLER")
            {
                return;
            }
            ReceiveUplinkCommand(command.CommandType, command.Data);
        }

        #endregion Methods
    }
}