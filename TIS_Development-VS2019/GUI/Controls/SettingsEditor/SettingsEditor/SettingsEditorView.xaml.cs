namespace SettingsEditor
{
    using System;
    using System.Collections.Generic;
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
    public partial class SettingsEditorView : UserControl
    {
        #region Fields

        private List<string> _list;
        private Int32 _maxFileSize;
        private string _path;

        #endregion Fields

        #region Constructors

        public SettingsEditorView()
        {
            InitializeComponent();

            _list = new List<string>();
        }

        #endregion Constructors

        #region Properties

        public Int32 maxFileSize
        {
            set
            {
                _maxFileSize = value;
            }
            get
            {
                return _maxFileSize;
            }
        }

        public string Path
        {
            set
            {
                _path = value;

                _list.Clear();
                lstSettings.Items.Clear();

                string[] filePaths = Directory.GetFiles(_path, "*.xml");
                foreach (string str in filePaths)
                {
                    FileInfo fi = new FileInfo(str);
                    if ((fi.Length < maxFileSize) && (!fi.Name.Contains("Microsoft")))
                    {
                        _list.Add(str);
                        lstSettings.Items.Add(System.IO.Path.GetFileName(str));
                    }
                }

            }

            get
            {
                return _path;
            }
        }

        public List<string> SettingsList
        {
            get
            {
                _list.Clear();

                string[] filePaths = Directory.GetFiles(_path, "*.xml");
                foreach (string str in filePaths)
                {
                    _list.Add(str);
                }

                return _list;
            }
        }

        #endregion Properties

        #region Methods

        public void ForceUpdate()
        {
            UpdateDocument();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            UpdateDocument();
        }

        private void lstSettings_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            xmlViewer.XmlDocument = null;

            ListBox lb = (ListBox)sender;

            if (lb.SelectedIndex >= 0)
            {
                XmlDocument xmlDoc = new XmlDocument();

                xmlDoc.Load(_list[lb.SelectedIndex]);
                xmlViewer.XmlDocument = xmlDoc;
                xmlViewer.ExpandAllAttributes = true;
            }
        }

        private void UpdateDocument()
        {
            //force the document to save
            xmlViewer.XmlDocument = null;

            if (lstSettings.SelectedIndex >= 0)
            {
                //reload the document
                XmlDocument xmlDoc = new XmlDocument();

                xmlDoc.Load(_list[lstSettings.SelectedIndex]);
                xmlViewer.XmlDocument = xmlDoc;
            }
        }

        #endregion Methods
    }
}