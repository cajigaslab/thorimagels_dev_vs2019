namespace RunSampleLSDll
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Windows;
    using System.Xml;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using RunSampleLSDll.ViewModel;

    using ThorImageInfastructure;

    using ThorLogging;

    using ThorSharedTypes;

    [Module(ModuleName = "RunSampleModule")]
    public class RunSampleModule : IModule
    {
        #region Fields

        private IEventAggregator _eventAggregator;
        private SubscriptionToken _subscriptionIPCToken;
        private SubscriptionToken _subscriptionToken;

        #endregion Fields

        #region Constructors

        public RunSampleModule(IUnityContainer container, IRegionManager regionManager, IEventAggregator eventAggregator)
        {
            Container = container;
            RegionManager = regionManager;
            _eventAggregator = eventAggregator;

            this.MainWindowView = Container.Resolve<MainWindow>();
            this.SettingsRegion = regionManager.Regions["SettingsRegion"];

            ((RunSampleLSViewModel)(this.MainWindowView).RunSampleLSView.DataContext).RunSampleLS._eventAggregator = _eventAggregator;
        }

        #endregion Constructors

        #region Properties

        public IUnityContainer Container
        {
            get;
            private set;
        }

        public IRegionManager RegionManager
        {
            get;
            private set;
        }

        private MainWindow MainWindowView
        {
            get;
            set;
        }

        private IRegion SettingsRegion
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Show or hide the  RunSample view
        /// </summary>
        /// <param name="command"></param>
        public void CommandEventHandler(Command command)
        {
            //check the guid of the incomming event
            Guid runSampleGUID = new Guid("30db4357-7508-46c9-84eb-3ca0900aa4c5");

            if (command.CommandGUID != runSampleGUID)
            {
                return;
            }

            foreach (object view in new List<object>(RegionManager.Regions["SettingsRegion"].Views))
            {
                this.SettingsRegion.Remove(view);
            }

            this.SettingsRegion.Add(this.MainWindowView, "RunSample", true);
            this.SettingsRegion.Activate(this.MainWindowView);

            try
            {
                if (command.Payload != null)
                {
                    string path = string.Empty;
                    string str = string.Empty;

                    if (command.Payload.Count > 0)
                    {
                        if (command.Payload[0].Equals("RunImmediately"))
                        {
                            if (command.Payload.Count > 1)
                            {
                                XmlDocument doc = new XmlDocument();

                                doc.LoadXml(command.Payload[1]);

                                const string CMD_OUTPUTPATH = "/Command/OutputPath";
                                const string CMD_OUTPUTPATH_ATTRIBUTE = "value";

                                XmlNode node = doc.SelectSingleNode(CMD_OUTPUTPATH);

                                XmlAttribute attrib = node.Attributes[CMD_OUTPUTPATH_ATTRIBUTE];

                                string strFile = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\VariableList.xml";
                                XmlDocument varDoc = new XmlDocument();

                                if ((null != attrib) && (attrib.Value.Length > 0))
                                {
                                    //convert the path alias value into
                                    //the actual path
                                    path = attrib.Value;
                                    varDoc.Load(strFile);

                                    XmlNodeList ndList = varDoc.SelectNodes("/VariableList/Path");

                                    foreach (XmlNode nd in ndList)
                                    {
                                        str = nd.Attributes["name"].Value;
                                        if (str.Equals(path))
                                        {
                                            path = nd.Attributes["value"].Value;
                                            break;
                                        }
                                    }

                                }

                                const string CMD_EXPERIMENT = "/Command/Experiment";
                                const string CMD_EXPERIMENT_ATTRIBUTE = "value";

                                node = doc.SelectSingleNode(CMD_EXPERIMENT);

                                attrib = node.Attributes[CMD_EXPERIMENT_ATTRIBUTE];

                                if ((null != attrib) && (attrib.Value.Length > 0))
                                {
                                    ResourceManagerCS.Instance.ReplaceActiveXML(attrib.Value);

                                    doc.Load(attrib.Value);

                                    const string EXP_NAME = "ThorImageExperiment/Name";
                                    const string EXP_NAME_ATTRIBUTE = "name";

                                    node = doc.SelectSingleNode(EXP_NAME);

                                    if (null != node)
                                    {
                                        attrib = node.Attributes[EXP_NAME_ATTRIBUTE];

                                        string strUnique = ((null != attrib) && (attrib.Value.Length > 0)) ? attrib.Value : "Untitled_001";

                                        FileName uniqueName = new FileName(strUnique);
                                        uniqueName.MakeUnique(path);

                                        this.MainWindowView.RunSampleLSStart(path, strUnique);
                                    }
                                }
                            }
                            // When called to runimmediately from Capture Setup, no command.xml is passed, but it still needs to Start an acquisition
                            // immediately after switching tabs.
                            else if (1 == command.Payload.Count)
                            {
                                string tempFolder = Application.Current.Resources["TemplatesFolder"].ToString();
                                string pathActiveXML = tempFolder + "\\Active.xml";
                                const string EXP_NAME = "ThorImageExperiment/Name";
                                const string EXP_NAME_ATTRIBUTE = "name";
                                const string MODALITY = "ThorImageExperiment/Modality";
                                XmlDocument doc = new XmlDocument();

                                if (File.Exists(pathActiveXML))
                                {
                                    doc.Load(pathActiveXML);
                                }

                                XmlNode node = doc.SelectSingleNode(MODALITY);

                                if (null != node)
                                {
                                    if (XmlManager.GetAttribute(node, doc, "name", ref str) && (!str.Equals(ResourceManagerCS.Instance.ActiveModality)))
                                    {
                                        //if the modality differs fromt the modality
                                        //in the script attempt to set the modality
                                        ResourceManagerCS.Instance.ActiveModality = str;
                                    }
                                }

                                node = doc.SelectSingleNode(EXP_NAME);

                                if (null != node)
                                {
                                    XmlAttribute attrib = node.Attributes[EXP_NAME_ATTRIBUTE];

                                    string strUnique = ((null != attrib) && (attrib.Value.Length > 0)) ? attrib.Value : "Untitled_001";

                                    FileName uniqueName = new FileName(strUnique);
                                    uniqueName.MakeUnique(path);

                                    this.MainWindowView.RunSampleLSCaptureSetupStart(path, strUnique);

                                }
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Failed to RunImmediately" + ex.Message);
            }
        }

        public void Initialize()
        {
            SubscribeToCommandEvent();
            SubscribeToIPCCommandEvent();
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
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

            _subscriptionToken = commandEvent.Subscribe(CommandEventHandler, ThreadOption.UIThread, true);
        }

        /// <summary>
        /// Capture events for InterProcess Communication 
        /// </summary>
        public void SubscribeToIPCCommandEvent()
        {
            CommandIPCEvent commandIPC = _eventAggregator.GetEvent<CommandIPCEvent>();

            if (_subscriptionIPCToken != null)
            {
                commandIPC.Unsubscribe(_subscriptionIPCToken);
            }

            _subscriptionIPCToken = commandIPC.Subscribe(CommandIPCEventHandler, ThreadOption.UIThread, true);
        }

        private void CommandIPCEventHandler(IPCCommand command)
        {
            if (command.Destination != "RUN_SAMPLE")
            {
                return;
            }
            this.MainWindowView.RunSampleLSIPC(command.CommandType, command.Data);
        }

        #endregion Methods
    }
}