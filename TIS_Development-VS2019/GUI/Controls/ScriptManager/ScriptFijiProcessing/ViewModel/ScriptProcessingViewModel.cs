namespace ScriptFijiProcessingDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Resources;
    using System.Text;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;
    using System.Xml.Serialization;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ScriptFijiProcessingDll.Model;

    using ThorImageInfastructure;

    using FijiLauncher;

    using System.Timers;
    using ThorLogging;

    /// <summary>
    /// ViewModel class for the ExpSetup model object
    /// </summary>
    public class ScriptFijiProcessingViewModel : ViewModelBase
    {
        #region Fields

        // wrapped ScriptProcessing object
        private readonly ScriptFijiProcessing _scriptProcessing;

        private SubscriptionToken subscriptionToken;
        private BackgroundWorker _bw = new BackgroundWorker();
        private IUnityContainer _container;
        private IEventAggregator _eventAggregator;
        private IRegionManager _regionManager;
        private FijiLauncherControl _fijiLauncher = new FijiLauncherControl();

        private Timer _statusTimer = new Timer();

        #endregion Fields


        #region Events
        public event Action<bool> UpdateMenuBarButton;
        #endregion Events

        #region Constructors

        /// <summary>
        /// Create a new instance of the ExpSetupViewModel class
        /// </summary>
        /// <param name="ExpSetup">Wrapped ExpSetup object</param>
        public ScriptFijiProcessingViewModel(IEventAggregator eventAggregator, IRegionManager regionManager, IUnityContainer container, ScriptFijiProcessing ScriptProcessing)
        {
            this._eventAggregator = eventAggregator;
            this._regionManager = regionManager;
            this._container = container;

            if (ScriptProcessing != null)
            {
                this._scriptProcessing = ScriptProcessing;
            }
            else
            {

                ScriptProcessing = new ScriptFijiProcessing();

                if (ScriptProcessing == null)
                {
                    ResourceManager rm = new ResourceManager("ScriptProcessing.Properties.Resources", Assembly.GetExecutingAssembly());
                    throw new NullReferenceException("ScriptProcessing");
                }

                this._scriptProcessing = ScriptProcessing;
            }

            UpdateMenuBarButton += new Action<bool>(ScriptProcessing_UpdateMenuBarButton);
            SubscribeToCommandEvent();
        }

        #endregion Constructors

        #region Properties

        public ScriptFijiProcessing ScriptProcessing
        {
            get
            {
                return this._scriptProcessing;
            }
        }

        #endregion Properties

        #region Methods

        public void CommandEventHandler(Command command)
        {
            //check the guid of the incomming event
            Guid guid = new Guid("C1391459-132F-45ea-AE72-D7BEB2BD8086");

            if (command.CommandGUID != guid)
            {
                return;
            }

            if (command.Payload.Count > 1)
            {
                if (command.Payload[0].Equals("RunImmediately"))
                {

                    XmlDocument doc = new XmlDocument();

                    doc.LoadXml(command.Payload[1]);

                    const string CMD_MACRO = "/Command/Macro";
                    const string CMD_INPUT = "/Command/DataFolder";
                    const string CMD_FIJI = "/Command/FijiExe";
                    const string CMD_HEADLESS = "/Command/Headless";
                    string macro = string.Empty;
                    string input = string.Empty;
                    string fiji = string.Empty;
                    string headless = string.Empty;

                    XmlNode node = doc.SelectSingleNode(CMD_FIJI);
                    if (null != node)
                        GetAttribute(node, doc, "value", ref fiji);

                    node = doc.SelectSingleNode(CMD_HEADLESS);
                    if (null != node)
                        GetAttribute(node, doc, "value", ref headless);

                    node = doc.SelectSingleNode(CMD_INPUT);
                    if (null != node)
                        GetAttribute(node, doc, "value", ref input);

                    node = doc.SelectSingleNode(CMD_MACRO);
                    if (null != node)
                        GetAttribute(node, doc, "value", ref macro);

                    if (!File.Exists(fiji))
                    {
                        MessageBox.Show("The Fiji executable was not found at the following path: \"" + fiji + "\". Please update the path and run again.");
                        const string str = "1F914CCD-33DE-4f40-907A-4511AA145D8A";

                        Command commandErr = new Command();
                        command.Message = "ScriptManager";
                        command.CommandGUID = new Guid(str);
                        command.Payload = new List<string>();

                        command.Payload.Add("Error");

                        _eventAggregator.GetEvent<CommandFinishedDialogEvent>().Publish(command);
                        return;
                    }

                    if (!File.Exists(macro))
                    {
                        MessageBox.Show("The Macro file was not found at the following path: \"" + macro + "\". Please update the path and run again.");
                        const string str = "1F914CCD-33DE-4f40-907A-4511AA145D8A";

                        Command commandErr = new Command();
                        command.Message = "ScriptManager";
                        command.CommandGUID = new Guid(str);
                        command.Payload = new List<string>();

                        command.Payload.Add("Error");

                        _eventAggregator.GetEvent<CommandFinishedDialogEvent>().Publish(command);
                        return;
                    }


                    if ((0 == fiji.Length) ||
                        (0 == headless.Length) ||
                        (0 == input.Length) ||
                        (0 == macro.Length))
                    {
                    }
                    else
                    {
                        string path = string.Empty;

                        //convert the path alias value into
                        //the actual path
                        path = input;
                        string strFile = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\VariableList.xml";
                        XmlDocument varDoc = new XmlDocument();
                        varDoc.Load(strFile);

                        XmlNodeList ndList = varDoc.SelectNodes("/VariableList/Path");

                        foreach (XmlNode nd in ndList)
                        {
                            string str = nd.Attributes["name"].Value;
                            if (str.Equals(path))
                            {
                                path = nd.Attributes["value"].Value;
                            }
                        }

                        if (false == File.Exists(path + "\\Experiment.xml"))
                        {
                            //stop the script with an error
                            MessageBox.Show("an experiment was not found at the provided path: \"" + path + "\". Please update the path and run again.");
                            const string str = "1F914CCD-33DE-4f40-907A-4511AA145D8A";

                            Command commandErr = new Command();
                            commandErr.Message = "ScriptManager";
                            commandErr.CommandGUID = new Guid(str);
                            commandErr.Payload = new List<string>();

                            commandErr.Payload.Add("Error");

                            _eventAggregator.GetEvent<CommandFinishedDialogEvent>().Publish(commandErr);
                            return;
                        }

                        if (path[path.Length - 1] != '\\')
                        {
                            path += "\\";
                        }

                        _fijiLauncher.InputDir = path;
                        _fijiLauncher.FijiExeFile = fiji;
                        _fijiLauncher.IjmFile = macro;

                        if (headless.Equals("0"))
                        {
                            _fijiLauncher.Headless = false;
                        }
                        else
                        {
                            _fijiLauncher.Headless = true;
                        }

                        UpdateMenuBarButton(false); // no menubar updating as fiji script excuting

                        _fijiLauncher.LaunchFiji();
                        _statusTimer.Interval = .5;
                        _statusTimer.Elapsed += new ElapsedEventHandler(_statusTimer_Elapsed);
                        _statusTimer.Start();
                    }
                }
            }
        }

        void ScriptProcessing_UpdateMenuBarButton(bool status)
        {
            bool btnStatus = status;
            ChangeEvent changeEvent = new ChangeEvent();
            changeEvent.ModuleName = "ScriptProcessing";

            if (btnStatus)
            {
                changeEvent.IsChanged = true;
            }
            else
            {
                changeEvent.IsChanged = false;
            }

            //command published to change the status of the menu buttons in the Menu Control
            _eventAggregator.GetEvent<MenuModuleChangeEvent>().Publish(changeEvent);
        }

        string _statusString;

        public string StatusString
        {
            get
            {
                return _statusString;
            }
            set
            {
                _statusString = value;

                OnPropertyChanged("StatusString");
            }
        }

        void _statusTimer_Elapsed(object sender, ElapsedEventArgs e)
        {
            StatusString = _fijiLauncher.StatusString;
        }

        void FijiLauncherFinished(object sender, EventArgs e)
        {
            _statusTimer.Stop();

            const string str = "1F914CCD-33DE-4f40-907A-4511AA145D8A";

            Command command = new Command();
            command.Message = "ScriptManager";
            command.CommandGUID = new Guid(str);
            command.Payload = new List<string>();

            command.Payload.Add("Complete");

            _eventAggregator.GetEvent<CommandFinishedDialogEvent>().Publish(command);

            // UpdateMenuBarButton(true);  // release menubar update lock
        }

        public void EnableHandlers()
        {
            _bw.ProgressChanged += new ProgressChangedEventHandler(_bw_ProgressChanged);
            _bw.DoWork += new DoWorkEventHandler(_bw_DoWork);
            _bw.WorkerReportsProgress = true;
            _bw.WorkerSupportsCancellation = true;
            _fijiLauncher.IjmFinished += new IjmFinishedEventHandler(FijiLauncherFinished);
        }

        public void ReleaseHandlers()
        {
            _bw.ProgressChanged -= new ProgressChangedEventHandler(_bw_ProgressChanged);
            _bw.DoWork -= new DoWorkEventHandler(_bw_DoWork);
            _fijiLauncher.IjmFinished -= new IjmFinishedEventHandler(FijiLauncherFinished);
        }

        public void SubscribeToCommandEvent()
        {
            CommandShowDialogEvent commandEvent = _eventAggregator.GetEvent<CommandShowDialogEvent>();

            if (subscriptionToken != null)
            {
                commandEvent.Unsubscribe(subscriptionToken);
            }

            subscriptionToken = commandEvent.Subscribe(CommandEventHandler, ThreadOption.UIThread, false);
        }

        private string ConvertXmlDocumentToString(XmlDocument doc)
        {
            StringWriter sw = new StringWriter();
            XmlTextWriter tx = new XmlTextWriter(sw);
            doc.WriteTo(tx);
            return sw.ToString();
        }

        private bool GetAttribute(XmlNode node, XmlDocument doc, string attrName, ref string attrValue)
        {
            bool ret;

            if (null == node.Attributes.GetNamedItem(attrName))
            {
                ret = false;
            }
            else
            {
                attrValue = node.Attributes[attrName].Value;
                ret = true;
            }

            return ret;
        }

        private void SetAttribute(XmlNode node, XmlDocument doc, string attrName, string attrValue)
        {
            if (null == node.Attributes.GetNamedItem(attrName))
            {
                XmlAttribute attr = doc.CreateAttribute(attrName);
                attr.Value = attrValue;
                node.Attributes.Append(attr);
            }

            node.Attributes[attrName].Value = attrValue;
        }

        void _bw_DoWork(object sender, DoWorkEventArgs e)
        {
            _bw.ReportProgress(100);
        }

        void _bw_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
        }

        #endregion Methods
    }
}