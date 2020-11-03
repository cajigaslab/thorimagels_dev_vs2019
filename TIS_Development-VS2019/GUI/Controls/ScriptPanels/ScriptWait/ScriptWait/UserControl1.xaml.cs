namespace ScriptWait
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Globalization;
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
    /// 
    public partial class UserControl1 : UserControl, INotifyPropertyChanged
    {
        #region Fields

        const string CMD_WAIT = "Command/WaitTime";
        const string CMD_WAIT_ATTRIBUTE = "value";

        private XmlDocument _settingsDocument = null;
        private double _waitTime;

        #endregion Fields

        #region Constructors

        public UserControl1()
        {
            InitializeComponent();
            this.DataContext = this;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

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
                    XmlNode node = _settingsDocument.SelectSingleNode(CMD_WAIT);

                    if (null != node)
                    {
                        string str = string.Empty;

                        if (GetAttribute(node, _settingsDocument, CMD_WAIT_ATTRIBUTE, ref str))
                        {
                            WaitTime = Convert.ToDouble(str, CultureInfo.InvariantCulture);
                        }

                    }
                }
            }
        }

        public double WaitTime
        {
            get
            {
                return _waitTime;
            }
            set
            {
                _waitTime = value;
                SetCommandParameter(_settingsDocument, CMD_WAIT, CMD_WAIT_ATTRIBUTE, _waitTime.ToString(CultureInfo.InvariantCulture));

                OnPropertyChanged("WaitTime");
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

        #endregion Methods
    }
}