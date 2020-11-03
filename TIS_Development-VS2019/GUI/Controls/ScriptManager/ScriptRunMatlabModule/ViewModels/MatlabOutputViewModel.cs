namespace ScriptRunMatlabModule.ViewModels
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Timers;
    using System.Windows;
    using System.Windows.Media;
    using System.Xml;
    using System.Collections.ObjectModel;

    using MatlabEngineWrapper;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Wpf.Events;

    using ThorImageInfastructure;

    public class TextLine
    {
        private Brush _color;
        private string _statusString;

        public Brush DisplayColor
        {
            get
            {
                return _color;
            }
            set
            {
                _color = value;
            }
        }

        public string StatusString
        {
            get
            {
                return _statusString;
            }
            set
            {
                _statusString = value;
            }
        }
    }

    public class MatlabOutputViewModel : ViewModelBase
    {
        #region Fields

        // protect async log file id
        private static int AsynchronousId = 0;

        private IEventAggregator _eventAggregator;
        private string _logFilePath;
        private string _logHeadInfo;
        private int _lineCounter = 0;
        private bool _promptStop = false;
        Timer _logTimer = new Timer(250);
        private ObservableCollection<TextLine> _outputLines;

        #endregion Fields

        #region Constructors

        public MatlabOutputViewModel(IEventAggregator eventAggregator)
        {
            _eventAggregator = eventAggregator;
            _eventAggregator.GetEvent<CommandShowDialogEvent>().Subscribe(CommandEventHandler, ThreadOption.UIThread, false);
            _logTimer.Elapsed += new ElapsedEventHandler(OnTimedEvent);
            _outputLines = new ObservableCollection<TextLine>();
        }

        #endregion Constructors

        #region Properties

        public ObservableCollection<TextLine> Lines
        {
            get
            {
                return _outputLines;
            }
            set
            {
                _outputLines = value;
                OnPropertyChanged("Lines");
            }
        }

        private Brush GetMessageColor(string str)
        {
            String s = str;
            if (s.Contains("[INFO]"))
            {
                return new SolidColorBrush(Colors.DarkGreen);
            }
            else if (s.Contains("[WARNING]"))
            {
                return new SolidColorBrush(Colors.DarkOrange);
            }
            else if (s.Contains("[ERROR]"))
            {
                _promptStop = true;
                return new SolidColorBrush(Colors.Red);
            }
            else
            {
                return new SolidColorBrush(Colors.Transparent);
            }
        }

        #endregion Properties

        #region Methods

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetApplicationSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        public static extern int GetApplicationSettingsFilePathAndName(StringBuilder sb, int length);

        private async void CommandEventHandler(Command command)
        {
            //check the guid of the incomming event
            Guid guid = new Guid("b3f2f8a4-cf8f-4fce-a195-b163a3c20c75");
            if (command.CommandGUID != guid) return;
            if (!MatlabEngine.Instance.IsEngineValid())
            {
                SendErrorMsg(command, string.Format("The matlab executable is not installed."));
                return;
            }
            if (command.Payload.Count > 1)
            {
                if (command.Payload[0].Equals("RunImmediately"))
                {
                    string macro = string.Empty;
                    string input = string.Empty;
                    bool asynchronous = false;
                    _promptStop = false;
                    _lineCounter = 0;
                    Lines.Clear();
                    GetMatlabScriptParameters(command.Payload[1], ref macro, ref input, ref asynchronous);
                    if (!File.Exists(macro))
                    {
                        SendErrorMsg(command, string.Format("The Matlab script file was not found at the following path: \"{0}\". Please update the path and run again.", macro));
                        return;
                    }
                    if (!Directory.Exists(input))
                    {
                        SendErrorMsg(command, string.Format("The Input Folder was not found: \"{0}\". Please update the path and run again.", input));
                        return;
                    }
                    _logHeadInfo = string.Format(" Start matlab script...\n Script: {0}\n Input Folder: {1} \n Asynchronous: {2}\n", macro, input, asynchronous);
                    Lines.Add(new TextLine() { DisplayColor = Brushes.Transparent, StatusString = _logHeadInfo });
                    Lines.Add(new TextLine() { DisplayColor = Brushes.Transparent, StatusString = "Log Info:" });
                    EnableMenu(false);
                    var logPath = "";
                    if (!asynchronous)
                    {
                        _logFilePath = Path.Combine(Path.GetDirectoryName(macro), "SyncLog.txt");
                        if (!CheckLogFile(command, _logFilePath))
                        {
                            return;
                        }
                        _logTimer.Start();
                        logPath = _logFilePath;
                    }
                    else
                    {
                        AsynchronousId++;
                        logPath = Path.Combine(Path.GetDirectoryName(macro), string.Format("AsyncLog{0}.txt", AsynchronousId));
                        if (!CheckLogFile(command, logPath))
                        {
                            return;
                        }
                    }
                    var result = await Task.Run(() => MatlabEngine.Instance.RunScript(macro, input, logPath, asynchronous, false));
                    if (!asynchronous && !result)
                    {
                        // In synchronous mode, if the script has exception.
                        _logTimer.Stop();
                        var msgResultException = MessageBox.Show(string.Format("Exception thrown when running script: \"{0}\". \nDo you want to continue with the next Script command?", macro), "Run Matlab Script error", MessageBoxButton.YesNo);
                        if (msgResultException == MessageBoxResult.No)
                        {
                            SendErrorEvent(command);
                            return;
                        }
                    }
                    if (!asynchronous && _promptStop)
                    {
                        // In synchronous mode, if the script has exception.
                        _logTimer.Stop();
                        var msgResult = MessageBox.Show(string.Format("Error found while running script: \"{0}\". \nDo you want to continue with the next Script command?", macro), "Run Matlab Script error", MessageBoxButton.YesNo);
                        if (msgResult == MessageBoxResult.No)
                        {
                            SendErrorEvent(command);
                            return;
                        }
                    }
                    RunScriptFinished();
                    if (!asynchronous)
                    {
                        _logTimer.Stop();
                        Lines.Add(new TextLine() { DisplayColor = Brushes.Transparent, StatusString = "Run finished!" });
                    }
                }
            }
        }

        private bool CheckLogFile(Command command, string path)
        {
            if (File.Exists(path))
            {
                try
                {
                    File.Delete(path);
                }
                catch (Exception)
                {
                    SendErrorMsg(command, string.Format("The log file is being used by another process: \"{0}\". Please close any other instace of Matlab that could be locking it.", path));
                    return false;
                }

            }
            return true;
        }

        private void EnableMenu(bool enable)
        {
            ChangeEvent changeEvent = new ChangeEvent();
            changeEvent.ModuleName = "ScriptProcessing";
            changeEvent.IsChanged = enable;
            _eventAggregator.GetEvent<MenuModuleChangeEvent>().Publish(changeEvent);
        }

        private bool GetAttribute(XmlNode node, XmlDocument doc, string attrName, ref string attrValue)
        {
            bool ret = node.Attributes.GetNamedItem(attrName) != null;
            if (ret)
            {
                attrValue = node.Attributes[attrName].Value;
            }
            return ret;
        }

        private void GetMatlabScriptParameters(string xmlPath, ref string macro, ref string input, ref bool asynchronous)
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml(xmlPath);
            const string CMD_MACRO = "/Command/MatlabMacro";
            const string CMD_INPUTDATAFOLDERISNOTMOSTRECSENT = "Command/MatlabDataFolderIsNotMostRecent";
            const string CMD_INPUT = "/Command/MatlabDataFolder";
            const string CMD_ASCYNCHRONOUS = "/Command/Asynchronous";

            XmlNode node = doc.SelectSingleNode(CMD_MACRO);
            if (null != node)
                GetAttribute(node, doc, "value", ref macro);

            string astr = string.Empty;
            node = doc.SelectSingleNode(CMD_INPUTDATAFOLDERISNOTMOSTRECSENT);
            if (null != node)
                GetAttribute(node, doc, "value", ref astr);
            var isNotMosetRecentData = false;

            try
            {
                isNotMosetRecentData = Convert.ToBoolean(astr);
            }
            catch (Exception) { }

            if (isNotMosetRecentData)
            {
                node = doc.SelectSingleNode(CMD_INPUT);
                if (null != node)
                    GetAttribute(node, doc, "value", ref input);
            }
            else
            {
                input = GetMostRecentExperimentFolder();
            }

            node = doc.SelectSingleNode(CMD_ASCYNCHRONOUS);
            if (null != node)
                GetAttribute(node, doc, "value", ref astr);
            asynchronous = Convert.ToBoolean(astr);
        }

        private string GetMostRecentExperimentFolder()
        {
            int pathLenth = 261;
            StringBuilder sb = new StringBuilder(pathLenth);
            GetApplicationSettingsFilePathAndName(sb, pathLenth);
            var xmlDoc = new XmlDocument();
            xmlDoc.Load(sb.ToString());
            XmlNode node = xmlDoc.SelectSingleNode("/ApplicationSettings/LastExperiment");
            string experimentPath = "";
            if (node != null && node.Attributes.GetNamedItem("path") != null)
            {
                experimentPath = node.Attributes["path"].Value;
            }
            return experimentPath;
        }

        private void OnTimedEvent(object sender, ElapsedEventArgs e)
        {
            if (File.Exists(_logFilePath))
            {
                using (var stream = File.Open(_logFilePath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
                {
                    // Use stream
                    var reader = new StreamReader(stream);
                    Application.Current.Dispatcher.Invoke((Action)(() =>
                    {
                        string line = string.Empty;
                        int currCounter = 0;
                        while (false == reader.EndOfStream)
                        {
                            line = reader.ReadLine();
                            if (currCounter >= _lineCounter && null != line)
                            {
                                Lines.Add(new TextLine() { DisplayColor = GetMessageColor(line), StatusString = line });
                            }
                            currCounter++; //Avoid repeating lines by keeping track of the last line in the steam
                        }
                        _lineCounter = currCounter;
                    }));
                }
            }
        }

        private void RunScriptFinished()
        {
            const string str = "1F914CCD-33DE-4f40-907A-4511AA145D8A";
            Command command = new Command();
            command.Message = "ScriptManager";
            command.CommandGUID = new Guid(str);
            command.Payload = new List<string>();
            command.Payload.Add("Complete");
            _eventAggregator.GetEvent<CommandFinishedDialogEvent>().Publish(command);
        }

        private void SendErrorEvent(Command command)
        {
            const string str = "1F914CCD-33DE-4f40-907A-4511AA145D8A";

            command.Message = "ScriptManager";
            command.CommandGUID = new Guid(str);
            command.Payload = new List<string>();
            command.Payload.Add("Error");

            _eventAggregator.GetEvent<CommandFinishedDialogEvent>().Publish(command);
        }

        private void SendErrorMsg(Command command, string message)
        {
            MessageBox.Show(message);
            SendErrorEvent(command);
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

        #endregion Methods
    }
}