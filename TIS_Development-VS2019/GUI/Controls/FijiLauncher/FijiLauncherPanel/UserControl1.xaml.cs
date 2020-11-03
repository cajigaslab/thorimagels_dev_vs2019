namespace FijiLauncherPanel
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Xml;

    using FolderDialogControl;

    using Microsoft.Win32;

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class UserControl1 : UserControl, INotifyPropertyChanged
    {
        #region Fields

        const string CMD_DATAFOLDER = "/Command/DataFolder";
        const string CMD_FIJI = "/Command/FijiExe";
        const string CMD_HEADLESS = "/Command/Headless";
        const string CMD_MACRO = "/Command/Macro";

        private string _dataFolder = "C:/Temp/";
        private string _macroFile = "C:/Temp/FFTBatch.ijm";
        private XmlDocument _settingsDocument = new XmlDocument();
        private XmlDocument _variableDoc;
        private string _variableFile;

        #endregion Fields

        #region Constructors

        public UserControl1()
        {
            InitializeComponent();

            this.DataContext = this;
            this.Loaded += new RoutedEventHandler(UserControl1_Loaded);
            this.Unloaded += new RoutedEventHandler(UserControl1_Unloaded);
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public string DataFolder
        {
            get
            {
                return _dataFolder;
            }
            set
            {
                _dataFolder = value;

                if (null != _settingsDocument)
                {
                    XmlNode node = _settingsDocument.SelectSingleNode(CMD_DATAFOLDER);
                    if (null != node)
                        SetAttribute(node, _settingsDocument, "value", _dataFolder);
                }
                OnPropertyChanged("DataFolder");
            }
        }

        public string MacroFile
        {
            get
            {
                return _macroFile;
            }
            set
            {
                _macroFile = value;

                if (null != _settingsDocument)
                {
                    XmlNode node = _settingsDocument.SelectSingleNode(CMD_MACRO);
                    if (null != node)
                        SetAttribute(node, _settingsDocument, "value", _macroFile);
                }

                UpdateMacroPath();
                OnPropertyChanged("MacroFile");
            }
        }

        public XmlDocument SettingsDocument
        {
            get
            {

                return _settingsDocument;
            }
            set
            {
                _settingsDocument = value;

                string macro = string.Empty;
                string input = string.Empty;
                string fiji = string.Empty;
                string headless = string.Empty;

                if (null != _settingsDocument)
                {
                    XmlNode node = _settingsDocument.SelectSingleNode(CMD_FIJI);
                    if (null != node)
                        GetAttribute(node, _settingsDocument, "value", ref fiji);

                    node = _settingsDocument.SelectSingleNode(CMD_HEADLESS);
                    if (null != node)
                        GetAttribute(node, _settingsDocument, "value", ref headless);

                    node = _settingsDocument.SelectSingleNode(CMD_DATAFOLDER);
                    if (null != node)
                        GetAttribute(node, _settingsDocument, "value", ref input);

                    node = _settingsDocument.SelectSingleNode(CMD_MACRO);
                    if (null != node)
                        GetAttribute(node, _settingsDocument, "value", ref macro);

                    DataFolder = input;
                    MacroFile = macro;
                }
            }
        }

        #endregion Properties

        #region Methods

        private void btnDataBrowse_Click(object sender, RoutedEventArgs e)
        {
            BrowseForFolderDialog dlg = new BrowseForFolderDialog();

            dlg.InitialFolder = DataFolder;

            Nullable<bool> result = dlg.ShowDialog();

            if (true == result)
            {
                DataFolder = dlg.SelectedFolder;
            }
        }

        private void btnMacroBrowse_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();

            dlg.FileName = MacroFile;
            dlg.DefaultExt = ".ijm";
            dlg.Filter = "ImageJ Macro (.ijm)|*.ijm";

            Nullable<bool> result = dlg.ShowDialog();

            if (true == result)
            {
                MacroFile = dlg.FileName;
            }
        }

        private void btnPathSetup_Click(object sender, RoutedEventArgs e)
        {
            SetupPathVariable dlg = new SetupPathVariable();

            dlg.ShowDialog();

            LoadPathList();
        }

        private void cbDataPath_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (cbDataPath.SelectedIndex >= 0)
            {
                DataFolder = cbDataPath.Items[cbDataPath.SelectedIndex].ToString();
            }
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

        private void LoadPathList()
        {
            cbDataPath.Items.Clear();

            _variableDoc = new XmlDocument();

            if (File.Exists(_variableFile))
            {
                _variableDoc.Load(_variableFile);

                XmlNodeList ndList = _variableDoc.SelectNodes("/VariableList/Path");

                int i = 0;
                foreach (XmlNode node in ndList)
                {
                    string str = string.Empty;
                    if (GetAttribute(node, _variableDoc, "name", ref str))
                    {
                        cbDataPath.Items.Add(str);
                        if (str.Equals(DataFolder))
                        {
                            cbDataPath.SelectedIndex = i;
                        }
                    }
                    i++;
                }
            }
        }

        void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
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

        private void UpdateMacroPath()
        {
            if (true == File.Exists(_macroFile))
            {
                FlowDocument myFlowDoc = new FlowDocument();

                StreamReader sr = new StreamReader(_macroFile);

                myFlowDoc.Blocks.Add(new Paragraph(new Run(sr.ReadToEnd().Replace("\n", ""))));

                rtbMacro.Document = myFlowDoc;
            }
        }

        void UserControl1_Loaded(object sender, RoutedEventArgs e)
        {
            _variableFile = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "/VariableList.xml";

            LoadPathList();
        }

        void UserControl1_Unloaded(object sender, RoutedEventArgs e)
        {
        }

        #endregion Methods
    }
}