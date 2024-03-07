namespace ScriptManagerDll.ViewModel
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
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;
    using System.Xml.Serialization;

    using GongSolutions.Wpf.DragDrop;

    using MatlabEngineWrapper;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ScriptManagerDll.Model;

    using ThorImageInfastructure;

    using ThorLogging;

    using ThorSharedTypes;

    using DragDrop = GongSolutions.Wpf.DragDrop.DragDrop;

    /// <summary>
    /// ViewModel class for the ExpSetup model object
    /// </summary>
    public class ScriptManagerViewModel : ViewModelBase, IDropTarget
    {
        #region Fields

        private const string CMD_DATA_FOLDER = "/Command/DataFolder";
        private const string CMD_DESCRIPTION = "description";
        private const string CMD_EXPERIMENT_TEMPLATE = "/Command/Experiment";
        private const string CMD_FIJI_EXE = "/Command/FijiExe";
        private const string CMD_GUID = "guid";
        private const string CMD_MACRO = "/Command/Macro";
        private const string CMD_MATLAB_MACRO = "Command/MatlabMacro";
        private const string CMD_NAME = "name";
        private const string CMD_OUTPUT_PATH = "/Command/OutputPath";
        private const string CMD_ROOT = "/CommandList";
        private const string CMD_TAG = "/Command";
        private const string CMD_WAIT = "Command/WaitTime";
        private const string CMD_XYMOVE = "Command/XYMove";
        private const string CMD_Z2MOVE = "Command/Z2Move";
        private const string CMD_ZMOVE = "Command/ZMove";
        private const string SCRIPT_ROOT = "ThorImageScript";

        // wrapped ScriptManager object
        private readonly ScriptManager _ScriptManager;

        private SubscriptionToken subscriptionToken;
        private BackgroundWorker _bw = new BackgroundWorker();
        private ScriptItem _clipboardScriptItem = null;
        private ObservableCollection<ScriptItem> _cmds;
        private string _commandListPath;
        private IUnityContainer _container;
        private ICommand _copyCommand;
        private ICommand _deleteAllCommand;
        private ICommand _deleteCommand;
        private ScriptItem _draggedScript;
        private IEventAggregator _eventAggregator;
        private bool _isDraggingScript;
        private ICommand _loadScriptCommand;
        private ICommand _pasteCommand;
        private int _previousSelectedLine;
        private IRegionManager _regionManager;
        private ICommand _runScriptCommand;
        private ICommand _saveScriptCommand;
        private ObservableCollection<ScriptItem> _script;
        private ScriptStates _scriptState = ScriptStates.COMPLETE;
        private int _selectedLine;
        private DispatcherTimer _timer = new DispatcherTimer();
        private bool _waitComplete = false;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the ExpSetupViewModel class
        /// </summary>
        /// <param name="ExpSetup">Wrapped ExpSetup object</param>
        public ScriptManagerViewModel(IEventAggregator eventAggregator, IRegionManager regionManager, IUnityContainer container, ScriptManager ScriptManager)
        {
            this._eventAggregator = eventAggregator;
            this._regionManager = regionManager;
            this._container = container;

            if (ScriptManager != null)
            {
                this._ScriptManager = ScriptManager;
            }
            else
            {

                ScriptManager = new ScriptManager();

                if (ScriptManager == null)
                {
                    ResourceManager rm = new ResourceManager("ScriptManager.Properties.Resources", Assembly.GetExecutingAssembly());
                    throw new NullReferenceException("ScriptManager");
                }

                this._ScriptManager = ScriptManager;
            }

            UpdateMenuBarButton += new Action<bool>(ScriptManager_UpdateMenuBarButton);

            SubscribeToCommandEvent();

            //create the observable collections that will store all of
            //the script items
            _script = new ObservableCollection<ScriptItem>();
            _cmds = new ObservableCollection<ScriptItem>();
            _isDraggingScript = false;
        }

        #endregion Constructors

        #region Enumerations

        enum ScriptStates
        {
            WAITING,
            COMPLETE,
            ERROR,
            PAUSE,
            STOP,
        }

        #endregion Enumerations

        #region Events

        public event Action<bool> UpdateMenuBarButton;

        #endregion Events

        #region Properties

        public bool CanAcceptChildren
        {
            get;
            set;
        }

        public ObservableCollection<ScriptItem> CollectionCmds
        {
            get
            {
                return _cmds;
            }

            set
            {
                _cmds = value;
                OnPropertyChanged("CollectionCmds");
            }
        }

        public ObservableCollection<ScriptItem> CollectionScript
        {
            get
            {
                return _script;
            }

            set
            {
                _script = value;
                OnPropertyChanged("CollectionScript");
            }
        }

        public string CommandListPath
        {
            get
            {
                return _commandListPath;
            }
            set
            {
                _commandListPath = value;

                //create the collection from the new
                //file containing the commands
                XmlDocument doc = new XmlDocument();

                try
                {

                    doc.Load(value);
                }
                catch (Exception ex)
                {
                    string str = ex.Message;
                    return;
                }

                _cmds.Clear();

                XmlNodeList nList = doc.SelectNodes(CMD_ROOT + CMD_TAG);

                foreach (XmlNode node in nList)
                {
                    string str = string.Empty;

                    if (XmlManager.GetAttribute(node, doc, CMD_NAME, ref str))
                    {
                        ScriptItem si = new ScriptItem();
                        si.Name = str;

                        XmlDocument paramDoc = new XmlDocument();

                        string importString = CMD_ROOT + CMD_TAG + "[@" + CMD_NAME + "='" + str + "']";
                        XmlNode matchedNode = doc.SelectSingleNode(importString);
                        XmlNode importItem = paramDoc.ImportNode(matchedNode, true);

                        paramDoc.AppendChild(importItem);

                        si.Paramters = ConvertXmlDocumentToString(paramDoc);

                        XmlManager.GetAttribute(node, doc, CMD_DESCRIPTION, ref str);
                        si.Description = str;

                        _cmds.Add(si);
                    }
                }
            }
        }

        public ICommand CopyCommand
        {
            get
            {
                if (this._copyCommand == null)
                    this._copyCommand = new RelayCommand(() => Copy());

                return this._copyCommand;
            }
        }

        public ICommand DeleteAllCommand
        {
            get
            {
                if (this._deleteAllCommand == null)
                    this._deleteAllCommand = new RelayCommand(() => DeleteAll());

                return this._deleteAllCommand;
            }
        }

        public ICommand DeleteCommand
        {
            get
            {
                if (this._deleteCommand == null)
                    this._deleteCommand = new RelayCommand(() => Delete());

                return this._deleteCommand;
            }
        }

        public ICommand LoadScriptCommand
        {
            get
            {
                if (this._loadScriptCommand == null)
                    this._loadScriptCommand = new RelayCommand(() => LoadScript());

                return this._loadScriptCommand;
            }
        }

        public ICommand PasteCommand
        {
            get
            {
                if (this._pasteCommand == null)
                    this._pasteCommand = new RelayCommand(() => Paste());

                return this._pasteCommand;
            }
        }

        public ICommand RunScriptCommand
        {
            get
            {
                if (this._runScriptCommand == null)
                    this._runScriptCommand = new RelayCommand(() => RunScript());

                return this._runScriptCommand;
            }
        }

        public ICommand SaveScriptCommand
        {
            get
            {
                if (this._saveScriptCommand == null)
                    this._saveScriptCommand = new RelayCommand(() => SaveScript());

                return this._saveScriptCommand;
            }
        }

        public ScriptManager ScriptManager
        {
            get
            {
                return this._ScriptManager;
            }
        }

        public int SelectedLine
        {
            get
            {
                return _selectedLine;
            }
            set
            {
                _selectedLine = value;

                OnPropertyChanged("SelectedLine");
            }
        }

        #endregion Properties

        #region Methods

        public void CommandEventHandler(Command command)
        {
            //check the guid of the incomming event
            Guid guid = new Guid("1F914CCD-33DE-4f40-907A-4511AA145D8A");

            if (command.CommandGUID != guid)
            {
                return;
            }
            const string RESPONSE_COMPLETE = "Complete";
            const string RESPONSE_ERROR = "Error";
            const string RESPONSE_PAUSECONTINUE = "PauseContinue";
            const string RESPONSE_STOP = "Stop";

            if ((null != command.Payload) && (command.Payload.Count > 0))
            {
                if (command.Payload[0].Equals(RESPONSE_COMPLETE))
                {
                    _scriptState = ScriptStates.COMPLETE;
                }
                else if (command.Payload[0].Equals(RESPONSE_ERROR))
                {
                    _scriptState = ScriptStates.ERROR;
                }
                else if (command.Payload[0].Equals(RESPONSE_PAUSECONTINUE))
                {
                    _scriptState = ScriptStates.PAUSE;
                }
                else if (command.Payload[0].Equals(RESPONSE_STOP))
                {
                    _scriptState = ScriptStates.STOP;
                }
            }
        }

        public void EnableHandlers()
        {
            _bw.ProgressChanged += new ProgressChangedEventHandler(_bw_ProgressChanged);
            _bw.DoWork += new DoWorkEventHandler(_bw_DoWork);
            _bw.WorkerReportsProgress = true;
            _bw.WorkerSupportsCancellation = true;
        }

        void IDropTarget.DragOver(IDropInfo dropInfo)
        {
            DragDrop.DefaultDropHandler.DragOver(dropInfo);
            if (dropInfo.DragInfo.SourceCollection.Equals(_script) && false == _isDraggingScript)
            {
                //Persist the dragged script item
                _previousSelectedLine = _selectedLine;
                if (_script[_previousSelectedLine] != null)
                {
                    _draggedScript = _script[_previousSelectedLine];
                    _isDraggingScript = true;
                }
            }
        }

        void IDropTarget.Drop(IDropInfo dropInfo)
        {
            if (dropInfo.DragInfo.SourceCollection.Equals(_script))
            {
                if (_draggedScript != null)
                {
                    //Persist the dragged script item
                    _script[_previousSelectedLine] = _draggedScript;
                    _isDraggingScript = false;
                }
                //moving an existing item in the list
                DragDrop.DefaultDropHandler.Drop(dropInfo);
            }
            else
            {
                //adding a new item from the command list
                ScriptItem si = new ScriptItem();
                ScriptItem data = (ScriptItem)dropInfo.Data;
                si.Name = data.Name;
                si.LineNumber = dropInfo.InsertIndex;
                si.Id = data.Id;
                si.Icon = data.Icon;
                si.Notes = data.Notes;
                si.Description = data.Description;
                si.Paramters = data.Paramters;

                _script.Insert(dropInfo.InsertIndex, si);

            }

            ReassignLineNumbers(dropInfo.InsertIndex);
        }

        public void ReleaseHandlers()
        {
            _bw.ProgressChanged -= new ProgressChangedEventHandler(_bw_ProgressChanged);
            _bw.DoWork -= new DoWorkEventHandler(_bw_DoWork);
        }

        public void SubscribeToCommandEvent()
        {
            CommandFinishedDialogEvent commandEvent = _eventAggregator.GetEvent<CommandFinishedDialogEvent>();

            if (subscriptionToken != null)
            {
                commandEvent.Unsubscribe(subscriptionToken);
            }

            subscriptionToken = commandEvent.Subscribe(CommandEventHandler, ThreadOption.UIThread, false);
        }

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamDouble")]
        private static extern bool GetDeviceParamDouble(int deviceSelection, int paramId, ref double param);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamDouble")]
        private static extern bool SetDeviceParamDouble(int deviceSelection, int paramId, double param, bool wait);

        private void CheckAndCloseMatlabEngine()
        {
            foreach (var it in _script)
            {
                if (it.Name == "Run Matlab Script")
                {
                    MatlabEngine.Instance.StopEngine();
                    return;
                }
            }
        }

        private void CheckAndInitMatlabEngine()
        {
            foreach (var it in _script)
            {
                if (it.Name == "Run Matlab Script")
                {
                    MatlabEngine.Instance.InitEngine();
                    return;
                }
            }
        }

        private bool CheckMatlabEngineValid()
        {
            if (!MatlabEngine.Instance.IsEngineValid())
            {
                MessageBox.Show("Cannot start Engine: Matlab or it's respective Runtime is not installed");
                return false;
            }
            return true;
        }

        private bool CheckScriptSettingsFields()
        {
            for (int i = 0; i < _script.Count; i++)
            {
                try
                {
                    _scriptState = ScriptStates.WAITING;
                    XmlDocument doc = new XmlDocument();

                    string str = string.Empty;
                    doc.LoadXml(_script[i].Paramters);

                    if ("Capture" == _script[i].Name)
                    {
                        XmlNode node = doc.SelectSingleNode(CMD_EXPERIMENT_TEMPLATE);

                        if (XmlManager.GetAttribute(node, doc, "value", ref str))
                        {
                            if (string.Empty == str || null == str)
                            {
                                MessageBox.Show(string.Format("Please select an Experiment Template for Command #{0}.", i + 1), "Missing Experiment Template", MessageBoxButton.OK, MessageBoxImage.Information);
                                return false;
                            }
                        }
                        else
                        {
                            MessageBox.Show(string.Format("Please select an Experiment Template for Command #{0}.", i + 1), "Missing Experiment Template", MessageBoxButton.OK, MessageBoxImage.Information);
                            return false;
                        }

                        node = doc.SelectSingleNode(CMD_OUTPUT_PATH);
                        if (XmlManager.GetAttribute(node, doc, "value", ref str))
                        {
                            if (string.Empty == str || null == str)
                            {
                                MessageBox.Show(string.Format("Please select an Output Path for Command #{0}.", i + 1), "Missing Output Path", MessageBoxButton.OK, MessageBoxImage.Information);
                                return false;
                            }
                        }
                        else
                        {
                            MessageBox.Show(string.Format("Please select an Output Path for Command #{0}.", i + 1), "Missing Output Path", MessageBoxButton.OK, MessageBoxImage.Information);
                            return false;
                        }
                    }

                    else if ("Run ImageJ Macro" == _script[i].Name)
                    {

                        XmlNode experimentTemplateNode = doc.SelectSingleNode(CMD_EXPERIMENT_TEMPLATE);
                        XmlNode node = doc.SelectSingleNode(CMD_MACRO);
                        if (XmlManager.GetAttribute(node, doc, "value", ref str))
                        {
                            if (string.Empty == str || null == str)
                            {
                                MessageBox.Show(string.Format("Please select a Macro for Command #{0}.", i + 1), "Missing Macro", MessageBoxButton.OK, MessageBoxImage.Information);
                                return false;
                            }
                        }
                        else
                        {
                            MessageBox.Show(string.Format("Please select a Macro for Command #{0}.", i + 1), "Missing Macro", MessageBoxButton.OK, MessageBoxImage.Information);
                            return false;
                        }

                        node = doc.SelectSingleNode(CMD_DATA_FOLDER);
                        if (XmlManager.GetAttribute(node, doc, "value", ref str))
                        {
                            if (string.Empty == str || null == str)
                            {
                                MessageBox.Show(string.Format("Please select a Data Folder for Command #{0}.", i + 1), "Missing Data Folder", MessageBoxButton.OK, MessageBoxImage.Information);
                                return false;
                            }
                        }
                        else
                        {
                            MessageBox.Show(string.Format("Please select a Data Folder for Command #{0}.", i + 1), "Missing Data Folder", MessageBoxButton.OK, MessageBoxImage.Information);
                            return false;
                        }
                        node = doc.SelectSingleNode(CMD_FIJI_EXE);
                        if (XmlManager.GetAttribute(node, doc, "value", ref str))
                        {
                            if (string.Empty == str || null == str)
                            {
                                MessageBox.Show("Invalid Fiji executable location! Please restart the app and update the location of the Fiji executable", "Invalid Fiji Location", MessageBoxButton.OK, MessageBoxImage.Information);
                                return false;
                            }
                            else if (!File.Exists(str))
                            {
                                MessageBox.Show("Invalid Fiji executable location! Please restart the app and update the location of the Fiji executable", "Invalid Fiji Location", MessageBoxButton.OK, MessageBoxImage.Information);
                                return false;
                            }
                        }
                        else
                        {
                            MessageBox.Show("Invalid Fiji executable location! Please restart the app and update the location of the Fiji executable", "Invalid Fiji Location", MessageBoxButton.OK, MessageBoxImage.Information);
                            return false;
                        }

                    }
                    else if ("Run Matlab Script" == _script[i].Name)
                    {
                        if (File.Exists(".\\Modules\\MatlabEngine.dll") && File.Exists(".\\NativeMatlabEngine.dll"))
                        {
                            if (CheckMatlabEngineValid())
                            {

                                XmlNode experimentTemplateNode = doc.SelectSingleNode(CMD_EXPERIMENT_TEMPLATE);
                                XmlNode node = doc.SelectSingleNode(CMD_MATLAB_MACRO);
                                if (XmlManager.GetAttribute(node, doc, "value", ref str))
                                {
                                    if (string.IsNullOrEmpty(str))
                                    {
                                        MessageBox.Show(string.Format("Please select a Matlab script for Command #{0}.", i + 1), "Missing Script", MessageBoxButton.OK, MessageBoxImage.Information);
                                        return false;
                                    }
                                }
                                else
                                {
                                    MessageBox.Show(string.Format("Please select a Matlab script for Command #{0}.", i + 1), "Missing Script", MessageBoxButton.OK, MessageBoxImage.Information);
                                    return false;
                                }
                            }
                        }
                        else
                        {
                            MessageBox.Show("Failed to Run Matlab Script. MatlabEngine.dll or NativeMatlabEngine.dll is missing. Please make sure the Matlab Feature was selected during the installation of ThorImageLS");
                            return false;
                        }
                    }
                }
                catch
                {
                    MessageBox.Show("Please complete the script setup before pressing Run.", "Setup Incomplete", MessageBoxButton.OK, MessageBoxImage.Information);
                    return false;
                }

            }

            return true;
        }

        private string ConvertXmlDocumentToString(XmlDocument doc)
        {
            StringWriter sw = new StringWriter();
            XmlTextWriter tx = new XmlTextWriter(sw);
            doc.WriteTo(tx);
            return sw.ToString();
        }

        private void Copy()
        {
            if ((SelectedLine >= 0) && (SelectedLine < _script.Count))
            {
                _clipboardScriptItem = _script[SelectedLine];
            }
        }

        private void Delete()
        {
            if ((SelectedLine >= 0) && (SelectedLine < _script.Count))
            {
                int currentSelection = SelectedLine;
                _script.RemoveAt(SelectedLine);

                ReassignLineNumbers(currentSelection);
            }
        }

        private void DeleteAll()
        {
            _script.Clear();
        }

        private void LoadScript()
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();

            dlg.FileName = "";
            dlg.DefaultExt = ".xml";
            dlg.Filter = "ThorImageScript (.xml)|*.xml";

            Nullable<bool> result = dlg.ShowDialog();

            if (true == result)
            {
                string fileName = dlg.FileName;

                XmlDocument openDoc = new XmlDocument();

                try
                {
                    openDoc.Load(fileName);
                }
                catch (Exception ex)
                {
                    string msg = ex.Message;
                    MessageBox.Show("Unable to open script! {0}", msg);
                    return;
                }

                XmlNodeList ndList = openDoc.SelectNodes(SCRIPT_ROOT + CMD_TAG);

                if (ndList.Count <= 0)
                {
                    return;
                }

                //clear the exisiting script collection
                //and rebuild a new collection by
                //parsing the selected file
                _script.Clear();

                for (int i = 0; i < ndList.Count; i++)
                {
                    XmlNode node = ndList[i];
                    ScriptItem si = new ScriptItem();
                    XmlDocument paramDoc = new XmlDocument();

                    XmlNode importedItem = paramDoc.ImportNode(node, true);
                    paramDoc.AppendChild(importedItem);
                    string str = string.Empty;

                    if (XmlManager.GetAttribute(node, openDoc, CMD_NAME, ref str))
                    {
                        si.Name = str;
                        si.Paramters = ConvertXmlDocumentToString(paramDoc);
                        si.LineNumber = i + 1;

                        if (XmlManager.GetAttribute(node, openDoc, CMD_DESCRIPTION, ref str))
                        {
                            si.Description = str;
                        }
                        _script.Add(si);
                    }
                }

                //select the first command in the script
                SelectedLine = 0;
            }
        }

        private void Paste()
        {
            if ((SelectedLine < _script.Count) && (_clipboardScriptItem != null))
            {
                ScriptItem si = new ScriptItem();

                si.Description = _clipboardScriptItem.Description;
                si.Icon = _clipboardScriptItem.Icon;
                si.Id = _clipboardScriptItem.Id;
                si.Name = _clipboardScriptItem.Name;
                si.Notes = _clipboardScriptItem.Notes;
                si.Paramters = _clipboardScriptItem.Paramters;

                _script.Insert(SelectedLine + 1, si);

                ReassignLineNumbers(SelectedLine + 1);
            }
        }

        private void ReassignLineNumbers(int index)
        {
            for (int i = 0; i < _script.Count; i++)
            {
                _script[i].LineNumber = i + 1;
            }
            this.SelectedLine = Math.Min(index, _script.Count - 1);
        }

        private void RunScript()
        {
            if (true == _bw.IsBusy)
            {
                MessageBox.Show("Script is already running!");

                _bw.CancelAsync();

            }
            else
            {
                if (true == CheckScriptSettingsFields())
                {
                    _bw.RunWorkerAsync();
                    UpdateMenuBarButton(false); // lock menubar
                }
            }
        }

        private void SaveScript()
        {
            Microsoft.Win32.SaveFileDialog dlg = new Microsoft.Win32.SaveFileDialog();

            dlg.FileName = "Untitled";
            dlg.DefaultExt = ".xml";
            dlg.Filter = "ThorImageScript (.xml)|*.xml";

            Nullable<bool> result = dlg.ShowDialog();

            if (true == result)
            {
                string fileName = dlg.FileName;

                //create a new document and
                //output the command collection
                XmlDocument saveDoc = new XmlDocument();

                XmlDeclaration declaration = saveDoc.CreateXmlDeclaration("1.0", "", "");
                XmlElement rootElement = saveDoc.CreateElement(SCRIPT_ROOT);

                saveDoc.AppendChild(declaration);
                saveDoc.AppendChild(rootElement);

                for (int i = 0; i < _script.Count; i++)
                {
                    XmlDocument doc = new XmlDocument();

                    doc.LoadXml(_script[i].Paramters);
                    XmlNode matchedNode = doc.SelectSingleNode(CMD_TAG);

                    XmlManager.SetAttribute(matchedNode, doc, CMD_DESCRIPTION, _script[i].Description);

                    XmlNode importedItem = saveDoc.ImportNode(matchedNode, true);

                    saveDoc.DocumentElement.AppendChild(importedItem);
                }

                saveDoc.Save(fileName);
            }
        }

        private bool ScriptLogicDigitalSwitch(ref int i, XmlDocument doc)
        {
            ScriptManager_UpdateMenuBarButton(false);
            XmlNode node = null;
            string str = string.Empty;
            string enStr = string.Empty;
            node = doc.SelectSingleNode("Command/DigiSwitch");
            //get status of digital switches
            if (false == XmlManager.GetAttribute(node, doc, "status", ref str))
            {
                return false;
            }

            //get enable of digital switches
            if (false == XmlManager.GetAttribute(node, doc, "enabled", ref enStr))
            {
                return false;
            }

            if (enStr.CompareTo("1") == 0)
            {
                for (int k = 0; k < str.Length; k++)
                {
                    ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_EPHYS,
                                                        (int)IDevice.Params.PARAM_EPHYS_DIG_LINE_OUT_1 + k,
                                                        str[k] - '0',
                                                        (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
                }
            }

            return true;
        }

        private bool ScriptLogicFor(Dictionary<string, ForLoopStart> forLoops, int i, XmlDocument doc)
        {
            const string CMD_FOR_END = "/Command/ForEnd";
            const string VALUE = "value";
            const string CMD_ITERATIONS = "/Command/Iterations";
            XmlNode node = null;

            //if a for command is encountered addit it to the forloop dictionary

            string forDesc = string.Empty;

            node = doc.SelectSingleNode(CMD_FOR_END);

            if (null == node)
            {
                //For loop is missing an end parameter
                return false;
            }

            if (true == XmlManager.GetAttribute(node, doc, VALUE, ref forDesc))
            {
                node = doc.SelectSingleNode(CMD_ITERATIONS);

                if (null == node)
                {
                    //For loop is missing an iterations parameter
                    return false;
                }

                string forIterations = string.Empty;
                if (true == XmlManager.GetAttribute(node, doc, VALUE, ref forIterations))
                {
                    //nested for loop
                    //remove the previous instance
                    if (forLoops.ContainsKey(forDesc))
                    {
                        forLoops.Remove(forDesc);
                    }

                    forLoops.Add(forDesc, new ForLoopStart() { Count = Convert.ToInt32(forIterations), StartIndex = i });
                }
            }
            return true;
        }

        private bool ScriptLogicForEnd(Dictionary<string, ForLoopStart> forLoops, ref int i, XmlDocument doc)
        {
            XmlNode node = null;
            string forEndDesc = string.Empty;

            const string CMD = "/Command";

            node = doc.SelectSingleNode(CMD);

            if (false == XmlManager.GetAttribute(node, doc, CMD_DESCRIPTION, ref forEndDesc))
            {
                return true;
            }

            for (int j = 0; j < forLoops.Count; j++)
            {
                if (forLoops.ContainsKey(forEndDesc))
                {
                    //located a for loop matched to the forend
                    ForLoopStart val = forLoops[forEndDesc];

                    //decrement the count
                    forLoops[forEndDesc].Count--;

                    //remove the forloops entry when the count reaches zero
                    if (forLoops[forEndDesc].Count <= 0)
                    {
                        forLoops.Remove(forEndDesc);
                    }
                    else
                    {
                        //move the index to the start of the loop
                        i = val.StartIndex;
                    }
                    break;
                }
            }

            return true;
        }

        private bool ScriptLogicMethods(Dictionary<string, ForLoopStart> forLoops, ref int i, XmlDocument doc, string str)
        {
            bool ret;

            switch (str)
            {
                case "For":
                    {
                        ret = ScriptLogicFor(forLoops, i, doc);
                    }
                    break;

                case "ForEnd":
                    {
                        ret = ScriptLogicForEnd(forLoops, ref i, doc);
                    }
                    break;
                case "Wait":
                    {
                        ret = ScriptLogicWait(ref i, doc);
                    }
                    break;
                case "XY Move":
                    {
                        ret = ScriptLogicMoveXY(ref i, doc);
                    }
                    break;
                case "Z Move":
                    {
                        ret = ScriptLogicMoveZ(ref i, doc);
                    }
                    break;
                case "Secondary Z Move":
                    {
                        ret = ScriptLogicMoveZ2(ref i, doc);
                    }
                    break;
                case "Digital Switches":
                    {
                        ret = ScriptLogicDigitalSwitch(ref i, doc);
                    }
                    break;
                default:
                    {
                        ret = true;
                    }
                    break;
            }

            return ret;
        }

        private bool ScriptLogicMoveXY(ref int i, XmlDocument doc)
        {
            ScriptManager_UpdateMenuBarButton(false);
            XmlNode node = null;
            string str = string.Empty;
            node = doc.SelectSingleNode(CMD_XYMOVE);
            const string CMD_ENABLE_LOAD_TEMPLATE = "loadTemplateEnabled";
            const string CMD_INPUT_PATH = "templateInputPath";
            const string CMD_STEPS_IN_PIXELS = "isStepsInPixels";
            const string CMD_STEPS_IN_UM = "isStepsInUM";
            const string CMD_X_ENABLE_MOVE_ATTRIBUTE = "enableXMove";
            const string CMD_X_DORELATIVE_MOVE = "doRelativeXMove";
            const string CMD_X_NEW_POS_ATTRIBUTE = "xNewPos";
            const string CMD_Y_ENABLE_MOVE_ATTRIBUTE = "enableYMove";
            const string CMD_Y_DORELATIVE_MOVE = "doRelativeYMove";
            const string CMD_Y_NEW_POS_ATTRIBUTE = "yNewPos";

            //get new X position
            if (false == XmlManager.GetAttribute(node, doc, CMD_X_NEW_POS_ATTRIBUTE, ref str))
            {
                return false;
            }
            double newXPos = 0;
            double tmpX = 0;
            if (double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpX))
            {
                newXPos = tmpX;
            }

            //get new Y position
            if (false == XmlManager.GetAttribute(node, doc, CMD_Y_NEW_POS_ATTRIBUTE, ref str))
            {
                return false;
            }
            double newYPos = 0;
            double tmpY = 0;
            if (double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpY))
            {
                newYPos = tmpY;
            }

            //get X move enable flag
            if (false == XmlManager.GetAttribute(node, doc, CMD_X_ENABLE_MOVE_ATTRIBUTE, ref str))
            {
                return false;
            }
            bool enableXMove = ("1" == str) ? true : false;

            //get Y move enable flag
            if (false == XmlManager.GetAttribute(node, doc, CMD_Y_ENABLE_MOVE_ATTRIBUTE, ref str))
            {
                return false;
            }
            bool enableYMove = ("1" == str) ? true : false;

            //get X relative move flag
            if (false == XmlManager.GetAttribute(node, doc, CMD_X_DORELATIVE_MOVE, ref str))
            {
                return false;
            }
            bool relativeXMove = ("1" == str) ? true : false;

            //get Y relative move flag
            if (false == XmlManager.GetAttribute(node, doc, CMD_Y_DORELATIVE_MOVE, ref str))
            {
                return false;
            }
            bool relativeYMove = ("1" == str) ? true : false;

            //get Load Template enable flag
            if (false == XmlManager.GetAttribute(node, doc, CMD_ENABLE_LOAD_TEMPLATE, ref str))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + ": Failed to get load template boolean. Returning false");
                return false;
            }
            bool loadFromTemplate = ("1" == str) ? true : false;

            //get Steps in UM enable flag
            if (false == XmlManager.GetAttribute(node, doc, CMD_STEPS_IN_UM, ref str))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + ": Failed to get steps in um boolean. Returning false");
                return false;
            }
            bool isStepsInUM = ("1" == str) ? true : false;

            //get Steps in Pixels enable flag
            if (false == XmlManager.GetAttribute(node, doc, CMD_STEPS_IN_PIXELS, ref str))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + ": Failed to get steps in pixels boolean. Returning false");
                return false;
            }
            bool isStepsInPixels = ("1" == str) ? true : false;

            //get Template Path
            str = string.Empty;
            if (false == XmlManager.GetAttribute(node, doc, CMD_INPUT_PATH, ref str))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + ": Failed to get template's path. Returning false");
                return false;
            }
            string templatePath = str;

            //Load positions values from template
            if (loadFromTemplate)
            {
                XmlDocument template = new XmlDocument();
                template.Load(templatePath);
                XmlNodeList ndList;
                if (null != template)
                {
                    ndList = template.SelectNodes("/ThorImageExperiment/Sample");
                }
                else
                {
                    MessageBox.Show("Template " + templatePath + " could not be opened or doesn't exist", "Template could not be opened");
                    return false;
                }

                if (null != ndList)
                {
                    str = string.Empty;
                    if (XmlManager.GetAttribute(ndList[0], template, "initialStageLocationX", ref str))
                    {
                        if (double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpX))
                        {
                            newXPos = tmpX;
                        }
                    }
                    else
                    {
                        MessageBox.Show("Could not find the value 'initialStageLocationX' under the Tag Sample in the template " + templatePath, "Value initialStageLocationX not found");
                        return false;
                    }

                    str = string.Empty;
                    if (XmlManager.GetAttribute(ndList[0], template, "initialStageLocationY", ref str))
                    {
                        if (double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpY))
                        {
                            newYPos = tmpY;
                        }
                    }
                    else
                    {
                        MessageBox.Show("Could not find the value 'initialStageLocationY' under the Tag Sample in the template " + templatePath, "Value initialStageLocationY not found");
                        return false;
                    }
                }
                else
                {
                    MessageBox.Show("Could not find the Tag 'Sample' in the template " + templatePath, "Tag 'Sample' not found");
                    return false;
                }

                if (isStepsInPixels)
                {
                    bool ret = false;
                    ndList = template.SelectNodes("/ThorImageExperiment/Modality");
                    if (null != ndList)
                    {
                        str = string.Empty;
                        if (XmlManager.GetAttribute(ndList[0], template, "primaryDetectorType", ref str))
                        {
                            double pixelSizeUM = 0.0;
                            if ("1" == str) //LSM
                            {
                                ndList = template.SelectNodes("/ThorImageExperiment/LSM");
                                if (null != ndList)
                                {
                                    str = string.Empty;
                                    if (XmlManager.GetAttribute(ndList[0], template, "pixelSizeUM", ref str))
                                    {
                                        if (double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out pixelSizeUM))
                                        {
                                            newXPos = newXPos * pixelSizeUM / 1000.00;
                                            newYPos = newYPos * pixelSizeUM / 1000.00;
                                            ret = true;
                                        }
                                    }
                                }
                            }
                            else if ("0" == str) // Camera
                            {
                                ndList = template.SelectNodes("/ThorImageExperiment/Camera");
                                if (null != ndList)
                                {
                                    str = string.Empty;
                                    if (XmlManager.GetAttribute(ndList[0], template, "pixelSizeUM", ref str))
                                    {
                                        if (double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out pixelSizeUM))
                                        {
                                            newXPos = newXPos * pixelSizeUM / 1000.00;
                                            newYPos = newYPos * pixelSizeUM / 1000.00;
                                            ret = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (false == ret)
                    {
                        MessageBox.Show("Could not get Pixel Size", "Pixel unit movement failed");
                    }
                }
            }

            //Convert to UM if enabled
            if (isStepsInUM && !isStepsInPixels)
            {
                newXPos = newXPos / 1000.00;
                newYPos = newYPos / 1000.00;
            }

            //if the move X enable flag is set to true, set the new position for the X Stage
            if (true == enableXMove)
            {
                if (true == relativeXMove)
                {
                    double currentXPos = 0.0;
                    GetDeviceParamDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_X_POS_CURRENT, ref currentXPos);
                    SetDeviceParamDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_X_POS, newXPos + currentXPos, true);
                }
                else
                {
                    SetDeviceParamDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_X_POS, newXPos, true);
                }
            }

            //if the move Y enable flag is set to true, set the new position for the Y Stage
            if (true == enableYMove)
            {
                if (true == relativeYMove)
                {
                    double currentYPos = 0.0;
                    GetDeviceParamDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_Y_POS_CURRENT, ref currentYPos);
                    SetDeviceParamDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_Y_POS, newYPos + currentYPos, true);
                }
                else
                {
                    SetDeviceParamDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_Y_POS, newYPos, true);
                }
            }

            return true;
        }

        private bool ScriptLogicMoveZ(ref int i, XmlDocument doc)
        {
            ScriptManager_UpdateMenuBarButton(false);
            XmlNode node = null;
            string str = string.Empty;
            node = doc.SelectSingleNode(CMD_ZMOVE);
            const string CMD_ENABLE_LOAD_TEMPLATE = "loadTemplateEnabled";
            const string CMD_INPUT_PATH = "templateInputPath";
            const string CMD_STEPS_IN_UM = "isStepsInUM";
            const string CMD_Z_NEW_POS_ATTRIBUTE = "zNewPos";
            const string CMD_DORELATIVE_MOVE = "doRelativeMove";

            //get new Z position
            if (false == XmlManager.GetAttribute(node, doc, CMD_Z_NEW_POS_ATTRIBUTE, ref str))
            {
                return false;
            }
            double newZPos = 0;
            double tmpZ = 0;
            if (double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpZ))
            {
                newZPos = tmpZ;
            }

            //get Z relative move flag
            if (false == XmlManager.GetAttribute(node, doc, CMD_DORELATIVE_MOVE, ref str))
            {
                return false;
            }
            bool relativeMove = ("1" == str) ? true : false;

            //get Load Template enable flag
            if (false == XmlManager.GetAttribute(node, doc, CMD_ENABLE_LOAD_TEMPLATE, ref str))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + ": Failed to get load template boolean. Returning false");
                return false;
            }
            bool loadFromTemplate = ("1" == str) ? true : false;

            //get Steps in UM enable flag
            if (false == XmlManager.GetAttribute(node, doc, CMD_STEPS_IN_UM, ref str))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + ": Failed to get steps in MM boolean. Returning false");
                return false;
            }
            bool isStepsInUM = ("1" == str) ? true : false;

            //get Template Path
            str = string.Empty;
            if (false == XmlManager.GetAttribute(node, doc, CMD_INPUT_PATH, ref str))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + ": Failed to get template's path. Returning false");
                return false;
            }
            string templatePath = str;

            //Load position value from Template
            if (loadFromTemplate)
            {
                XmlDocument template = new XmlDocument();
                template.Load(templatePath);
                XmlNodeList ndList;
                if (null != template)
                {
                    ndList = template.SelectNodes("/ThorImageExperiment/ZStage");
                }
                else
                {
                    MessageBox.Show("Template " + templatePath + " could not be opened or doesn't exist", "Template could not be opened");
                    return false;
                }

                if (null != ndList)
                {
                    str = string.Empty;
                    if (XmlManager.GetAttribute(ndList[0], template, "startPos", ref str))
                    {
                        if (double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpZ))
                        {
                            newZPos = tmpZ;
                        }
                    }
                    else
                    {
                        MessageBox.Show("Could not find the value 'startPos' under the Tag ZStage in the template " + templatePath, "Value startPos not found");
                        return false;
                    }
                }
                else
                {
                    MessageBox.Show("Could not find the Tag 'ZStage' in the template " + templatePath, "Tag 'ZStage' not found");
                    return false;
                }
            }

            //Convert to MM if enabled
            if (isStepsInUM)
            {
                newZPos = newZPos / 1000.0;
            }

            //Set the new position for the Z Stage
            if (true == relativeMove)
            {
                double currentZPos = 0.0;
                GetDeviceParamDouble((int)SelectedHardware.SELECTED_ZSTAGE, (int)IDevice.Params.PARAM_Z_POS_CURRENT, ref currentZPos);
                SetDeviceParamDouble((int)SelectedHardware.SELECTED_ZSTAGE, (int)IDevice.Params.PARAM_Z_POS, newZPos + currentZPos, true);
            }
            else
            {
                SetDeviceParamDouble((int)SelectedHardware.SELECTED_ZSTAGE, (int)IDevice.Params.PARAM_Z_POS, newZPos, true);
            }

            return true;
        }

        private bool ScriptLogicMoveZ2(ref int i, XmlDocument doc)
        {
            ScriptManager_UpdateMenuBarButton(false);
            XmlNode node = null;
            string str = string.Empty;
            node = doc.SelectSingleNode(CMD_Z2MOVE);
            const string CMD_ENABLE_LOAD_TEMPLATE = "loadTemplateEnabled";
            const string CMD_INPUT_PATH = "templateInputPath";
            const string CMD_STEPS_IN_UM = "isStepsInUM";
            const string CMD_Z2_NEW_POS_ATTRIBUTE = "z2NewPos";
            const string CMD_DORELATIVE_MOVE = "doRelativeMove";

            //get new Z position
            if (false == XmlManager.GetAttribute(node, doc, CMD_Z2_NEW_POS_ATTRIBUTE, ref str))
            {
                return false;
            }
            double newZ2Pos = 0;
            double tmpZ2 = 0;
            if (double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpZ2))
            {
                newZ2Pos = tmpZ2;
            }

            //get Z2 relative move flag
            if (false == XmlManager.GetAttribute(node, doc, CMD_DORELATIVE_MOVE, ref str))
            {
                return false;
            }
            bool relativeMove = ("1" == str) ? true : false;

            //get Load Template enable flag
            if (false == XmlManager.GetAttribute(node, doc, CMD_ENABLE_LOAD_TEMPLATE, ref str))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + ": Failed to get load template boolean. Returning false");
                return false;
            }
            bool loadFromTemplate = ("1" == str) ? true : false;

            //get Steps in UM enable flag
            if (false == XmlManager.GetAttribute(node, doc, CMD_STEPS_IN_UM, ref str))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + ": Failed to get steps in MM boolean. Returning false");
                return false;
            }
            bool isStepsInUM = ("1" == str) ? true : false;

            //get Template Path
            str = string.Empty;
            if (false == XmlManager.GetAttribute(node, doc, CMD_INPUT_PATH, ref str))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + ": Failed to get template's path. Returning false");
                return false;
            }
            string templatePath = str;

            //Load position value from Template
            if (loadFromTemplate)
            {
                XmlDocument template = new XmlDocument();
                template.Load(templatePath);
                XmlNodeList ndList;
                if (null != template)
                {
                    ndList = template.SelectNodes("/ThorImageExperiment/ZStage2");
                }
                else
                {
                    MessageBox.Show("Template " + templatePath + " could not be opened or doesn't exist", "Template could not be opened");
                    return false;
                }

                if (null != ndList)
                {
                    str = string.Empty;
                    if (XmlManager.GetAttribute(ndList[0], template, "pos", ref str))
                    {
                        if (double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpZ2))
                        {
                            newZ2Pos = tmpZ2;
                        }
                    }
                    else
                    {
                        MessageBox.Show("Could not find the value 'pos' under the Tag ZStage2 in the template " + templatePath, "Value pos not found");
                        return false;
                    }
                }
                else
                {
                    MessageBox.Show("Could not find the Tag 'ZStage2' in the template " + templatePath, "Tag 'ZStage2' not found");
                    return false;
                }
            }

            //Convert to MM if enabled
            if (isStepsInUM)
            {
                newZ2Pos = newZ2Pos / 1000.0;
            }

            //Set the new position for the Z2 Stage
            if (true == relativeMove)
            {
                double currentZ2Pos = 0.0;
                GetDeviceParamDouble((int)SelectedHardware.SELECTED_ZSTAGE2, (int)IDevice.Params.PARAM_Z_POS_CURRENT, ref currentZ2Pos);
                SetDeviceParamDouble((int)SelectedHardware.SELECTED_ZSTAGE2, (int)IDevice.Params.PARAM_Z_POS, newZ2Pos + currentZ2Pos, true);
            }
            else
            {
                SetDeviceParamDouble((int)SelectedHardware.SELECTED_ZSTAGE2, (int)IDevice.Params.PARAM_Z_POS, newZ2Pos, true);
            }

            return true;
        }

        private bool ScriptLogicWait(ref int i, XmlDocument doc)
        {
            ScriptManager_UpdateMenuBarButton(false);
            XmlNode node = null;
            string str = string.Empty;
            node = doc.SelectSingleNode(CMD_WAIT);
            const string CMD_WAIT_ATTRIBUTE = "value";
            if (false == XmlManager.GetAttribute(node, doc, CMD_WAIT_ATTRIBUTE, ref str))
            {
                return true;
            }
            double waitTime = Convert.ToDouble(str, CultureInfo.InvariantCulture);

            Command command = new Command();
            command.Message = _script[i].Name;
            command.Payload = new List<string>();
            command.Payload.Add("ScriptWaiting");
            command.Payload.Add(ConvertXmlDocumentToString(doc));

            _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);

            _scriptState = ScriptStates.WAITING;

            _timer.Interval = new TimeSpan(0, 0, 0, 0, (int)Math.Round(waitTime * 1000));
            _timer.Tick += _timer_Tick;
            _waitComplete = false;
            _timer.Start();

            while (ScriptStates.PAUSE == _scriptState || false == _waitComplete)
            {
                if (ScriptStates.STOP == _scriptState)
                    return false;

                //allow cpu time for other activities
                //failure to do so hangs the script
                System.Threading.Thread.Sleep(5);
            }
            _timer.Stop();
            _timer.Tick -= _timer_Tick;
            return true;
        }

        void ScriptManager_UpdateMenuBarButton(bool status)
        {
            bool btnStatus = status;
            ChangeEvent changeEvent = new ChangeEvent();
            changeEvent.ModuleName = "ScriptManager";

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

        void _bw_DoWork(object sender, DoWorkEventArgs e)
        {
            Dictionary<string, ForLoopStart> forLoops = new Dictionary<string, ForLoopStart>();
            if (File.Exists(".\\Modules\\MatlabEngine.dll") && File.Exists(".\\NativeMatlabEngine.dll"))
            {
                CheckAndInitMatlabEngine();
            }
            for (int i = 0; i < _script.Count; i++)
            {
                _scriptState = ScriptStates.WAITING;
                XmlDocument doc = new XmlDocument();
                string str = string.Empty;

                doc.LoadXml(_script[i].Paramters);

                XmlNode node = doc.SelectSingleNode(CMD_TAG);

                XmlManager.SetAttribute(node, doc, CMD_DESCRIPTION, _script[i].Description);

                if (XmlManager.GetAttribute(node, doc, CMD_GUID, ref str))
                {
                    bool waitForResponse = false;

                    if (str.Equals("none") || (0 == str.Length))
                    {
                        if (true == XmlManager.GetAttribute(node, doc, CMD_NAME, ref str))
                        {
                            if (false == ScriptLogicMethods(forLoops, ref i, doc, str))
                            {
                                //stop the script if an error is returned
                                break;
                            }
                        }

                        _scriptState = ScriptStates.COMPLETE;
                    }
                    else
                    {

                        Command command = new Command();
                        command.Message = _script[i].Name;
                        command.CommandGUID = new Guid(str);
                        command.Payload = new List<string>();
                        command.Payload.Add("RunImmediately");
                        command.Payload.Add(ConvertXmlDocumentToString(doc));

                        _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);

                        waitForResponse = true;

                    }
                    SelectedLine = i;

                    bool stopPressed = false;
                    bool pausePressed = false;
                    if (waitForResponse)
                    {
                        while (ScriptStates.WAITING == _scriptState || ScriptStates.STOP == _scriptState || ScriptStates.PAUSE == _scriptState)
                        {
                            if (ScriptStates.PAUSE == _scriptState)
                                pausePressed = true;
                            if (ScriptStates.STOP == _scriptState)
                                stopPressed = true;
                            //allow cpu time for other activities
                            //failure to do so hangs the script
                            System.Threading.Thread.Sleep(5);
                        };
                    }

                    if ((ScriptStates.PAUSE == _scriptState || true == pausePressed) && ScriptStates.ERROR != _scriptState)
                    {
                        //change the script state to waiting
                        //this will block until another pause
                        //stop or error arrives
                        _scriptState = ScriptStates.WAITING;

                        do
                        {
                            if ((ScriptStates.PAUSE == _scriptState) ||
                                (ScriptStates.ERROR == _scriptState) ||
                                (ScriptStates.STOP == _scriptState) ||
                                true == stopPressed)
                            {
                                break;
                            }
                            //allow cpu time for other activities
                            //failure to do so hangs the script
                            UpdateMenuBarButton(false);
                            System.Threading.Thread.Sleep(5);
                        } while (true);
                    }

                    if ((ScriptStates.ERROR == _scriptState) ||
                        (ScriptStates.STOP == _scriptState) ||
                        true == stopPressed)
                    {
                        //    //stop the script
                        break;
                    }
                }
            }

            //If the script toolbar stops the script the state will be "STOP" if a command errors or stops the state will be "ERROR"
            //Only tell the menu to refresh when the script completes or stops to prevent interference with command events
            //Also tell the menu to refresh when the script has been paused
            if (ScriptStates.COMPLETE == _scriptState || ScriptStates.STOP == _scriptState || ScriptStates.PAUSE == _scriptState || ScriptStates.ERROR == _scriptState)
            {
                Command finishedCommand = new Command();
                finishedCommand.Message = "Finished";
                _eventAggregator.GetEvent<CommandFinishedScriptEvent>().Publish(finishedCommand);
            }

            UpdateMenuBarButton(true); // release menubar
            if (File.Exists(".\\Modules\\MatlabEngine.dll") && File.Exists(".\\NativeMatlabEngine.dll"))
            {
                CheckAndCloseMatlabEngine();
            }
            _bw.ReportProgress(100);
        }

        void _bw_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
        }

        private void _timer_Tick(object sender, EventArgs e)
        {
            _waitComplete = true;
        }

        #endregion Methods

        #region Nested Types

        private class ForLoopStart
        {
            #region Fields

            public int Count;
            public int StartIndex;

            #endregion Fields
        }

        #endregion Nested Types
    }
}