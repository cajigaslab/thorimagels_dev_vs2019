namespace ForLoop
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

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class UserControl1 : UserControl, INotifyPropertyChanged
    {
        #region Fields

        const string CMD_FOREND = "Command/ForEnd";
        const string CMD_FOREND_ATTRIBUTE = "value";
        const string CMD_ITERATIONS = "Command/Iterations";
        const string CMD_ITERATIONS_ATTRIBUTE = "value";

        private string _forLabel;
        private int _interations;
        private XmlDocument _settingsDocument = null;
    
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

        public string ForLabel
        {
            get
            {
                return _forLabel;
            }
            set
            {
                _forLabel = value;

                SetCommandParameter(_settingsDocument, CMD_FOREND, CMD_FOREND_ATTRIBUTE, _forLabel);

                OnPropertyChanged("ForLabel");
            }
        }

        public int Iterations
        {
            get
            {
                return _interations;
            }
            set
            {
                _interations = value;

                SetCommandParameter(_settingsDocument, CMD_ITERATIONS, CMD_ITERATIONS_ATTRIBUTE, _interations.ToString());

                OnPropertyChanged("Iterations");
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

                //extract the path to the experiment file
                //and the path for the output
                if (null != _settingsDocument)
                {
                    XmlNode node = _settingsDocument.SelectSingleNode(CMD_ITERATIONS);

                    if (null != node)
                    {
                        string str = string.Empty;

                        if (GetAttribute(node, _settingsDocument, CMD_ITERATIONS_ATTRIBUTE, ref str))
                        {
                            Iterations = Convert.ToInt32(str);
                        }

                    }

                    node = _settingsDocument.SelectSingleNode(CMD_FOREND);

                    if (null != node)
                    {
                        string str = string.Empty;

                        if (GetAttribute(node, _settingsDocument, CMD_FOREND_ATTRIBUTE, ref str))
                        {
                            ForLabel = str;
                        }

                    }
                }
            }
        }

        #endregion Properties

        #region Methods

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

        void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
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
            try
            {
            }
            catch
            {
            }
        }

        void UserControl1_Unloaded(object sender, RoutedEventArgs e)
        {
            try
            {
            }
            catch (Exception ex)
            {
                string str = ex.Message;
            }
        }

        #endregion Methods
    }
}