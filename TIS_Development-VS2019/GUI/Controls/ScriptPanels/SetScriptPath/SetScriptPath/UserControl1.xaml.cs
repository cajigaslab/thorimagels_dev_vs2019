namespace SetScriptPath
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
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

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class UserControl1 : UserControl, INotifyPropertyChanged
    {
        #region Fields
        
        private ObservableCollection<PathItem> _pathItems;
        private string _variableListFile;
        private XmlDocument _variablesDocument = null;

        #endregion Fields

        #region Constructors

        public UserControl1()
        {
            InitializeComponent();

            this.DataContext = this;

            _pathItems = new ObservableCollection<PathItem>();            
            this.Loaded += new RoutedEventHandler(UserControl1_Loaded);
            this.Unloaded += new RoutedEventHandler(UserControl1_Unloaded);
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public ObservableCollection<PathItem> CollectionPath
        {
            get
            {
                return _pathItems;
            }

            set
            {
                _pathItems = value;
                OnPropertyChanged("CollectionPath");
            }
        }

        #endregion Properties

        #region Methods

        public void SetAttribute(XmlNode node, XmlDocument doc, string attrName, string attValue)
        {
            XmlNode tempNode = node.Attributes[attrName];

            if (null == tempNode)
            {
                XmlAttribute attr = doc.CreateAttribute(attrName);

                attr.Value = attValue;

                node.Attributes.Append(attr);
            }
            else
            {
                node.Attributes[attrName].Value = attValue;
            }
        }

        private void Add_Click(object sender, RoutedEventArgs e)
        {

            PathItem pi = new PathItem();
            
            EditPath dlg = new EditPath();
            dlg.Title = "Add Name and Path";
            if (true == dlg.ShowDialog())
            {
                pi.Alias = dlg.Alias;
                pi.Value = dlg.Path;
                CollectionPath.Add(pi); 
            }
            
        }

        private void CreateXmlNode(XmlDocument doc, string nodeName)
        {
            XmlNode node = doc.CreateNode(XmlNodeType.Element, nodeName, null);
            doc.DocumentElement.AppendChild(node);
        }

        private void Delete_Click(object sender, RoutedEventArgs e)
        {
            if (lvPath.SelectedIndex >= 0)
            {
                CollectionPath.RemoveAt(lvPath.SelectedIndex);
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

        private void lvPath_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if(lvPath.SelectedIndex >=0)
            {
                EditPath dlg = new EditPath();
                dlg.Title = "Edit Name and Path";
                dlg.Path = _pathItems[lvPath.SelectedIndex].Value;
                dlg.Alias = _pathItems[lvPath.SelectedIndex].Alias;

                if (true == dlg.ShowDialog())
                {
                    CollectionPath[lvPath.SelectedIndex].Value = dlg.Path;
                    CollectionPath[lvPath.SelectedIndex].Alias = dlg.Alias;
                }
            }
        }

        void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
        }

        public void SaveNameAndPath()
        {
            if (null == _pathItems)
            {
                return;
            }
            XmlNodeList ndList = _variablesDocument.SelectNodes("/VariableList/Path");

            foreach (XmlNode node in ndList)
            {
                node.ParentNode.RemoveChild(node);
            }

            for (int i = 0; i < _pathItems.Count; i++)
            {
                CreateXmlNode(_variablesDocument, "Path");
            }

            ndList = _variablesDocument.SelectNodes("/VariableList/Path");

            for (int i = 0; i < ndList.Count; i++)
            {
                SetAttribute(ndList[i], _variablesDocument, "name", _pathItems[i].Alias);
                SetAttribute(ndList[i], _variablesDocument, "value", _pathItems[i].Value);
            }

            _variablesDocument.Save(_variableListFile);
        }

        private void SetCommandParameter(XmlDocument doc, string tag, string attrib, string value)
        {
            if (null != doc)
            {
                XmlNode node = doc.SelectSingleNode(tag);

                if (null != node)
                {
                    node = node.Attributes.GetNamedItem(attrib);
                    if (null != node)
                    {
                        node.Value = value;
                    }
                }
            }
        }

        void UserControl1_Loaded(object sender, RoutedEventArgs e)
        {
            _variablesDocument = new XmlDocument();

            if (false == Application.Current.Resources.Contains("ApplicationSettingsFolder"))
            {
                return;
            }

             _variableListFile = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "/VariableList.xml";

             if (File.Exists(_variableListFile))
            {
                _variablesDocument.Load(_variableListFile);

                XmlNodeList ndList = _variablesDocument.SelectNodes("/VariableList/Path");

                int i = 0;
                foreach (XmlNode node in ndList)
                {
                    string str = string.Empty;
                    string strPath = string.Empty;
                    if (GetAttribute(node, _variablesDocument, "name", ref str))
                    {
                        if (GetAttribute(node, _variablesDocument, "value", ref strPath))
                        {
                            PathItem pi = new PathItem();
                            pi.Alias = str;
                            pi.Value = strPath;
                            CollectionPath.Add(pi);
                        }
                    }
                   i++;
                }
            }
        }

        private void UserControl1_LostFocus(object sender, RoutedEventArgs e)
        {
            SaveNameAndPath();
        }

        void UserControl1_Unloaded(object sender, RoutedEventArgs e)
        {
            
        }

        #endregion Methods
    }
}