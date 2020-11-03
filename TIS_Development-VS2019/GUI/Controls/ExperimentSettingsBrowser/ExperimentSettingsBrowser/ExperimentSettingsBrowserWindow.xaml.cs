namespace ExperimentSettingsBrowser
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Collections.Specialized;
    using System.ComponentModel;
    using System.Configuration;
    using System.Data;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Windows.Threading;
    using System.Xml;

    using DatabaseInterface;

    using FolderDialogControl;

    using Microsoft.Win32;

    using SpinnerProgress;

    using ThorLogging;

    /// <summary>
    /// Interaction logic for ExperimentSettingsBrowserWindow.xaml
    /// </summary>
    public partial class ExperimentSettingsBrowserWindow : Window, INotifyPropertyChanged
    {
        #region Fields

        CollectionViewSource listingDataView;
        private bool _allowExit = false;
        private bool _browseBool;
        private string _browseExperimentPath;
        private BrowserTypeEnum _browserType;
        private bool _chooseFromDatabaseBool;
        private bool _chooseFromTemplateBool;
        private string _databaseExperimentName;
        private string _databaseExperimentPath;
        private RangeEnabledObservableCollection<DatabaseItem> _databaseItems = new RangeEnabledObservableCollection<DatabaseItem>();
        private string _experimentPath;
        private string _experimentSettingsName;
        private string _experimentSettingsPath;
        private double _listBoxOpacity;
        private DirectoryInfo _settingTemplatesDirInfo;
        private Visibility _spinVisible;

        #endregion Fields

        #region Constructors

        public ExperimentSettingsBrowserWindow()
        {
            InitializeComponent();

            _settingTemplatesDirInfo = new DirectoryInfo(Application.Current.Resources["TemplatesFolder"].ToString() + "\\Template Favorites");

            this.Loaded += new RoutedEventHandler(ExperimentSettingsBrowserWindow_Loaded);
            this.Unloaded += new RoutedEventHandler(ExperimentSettingsBrowserWindow_Unloaded);
            listingDataView = (CollectionViewSource)(this.Resources["listingDataView"]);
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
            _browserType = BrowserTypeEnum.EXPERIMENT;
            this.Closing += new CancelEventHandler(ExperimentSettingsBrowserWindow_Closing);
            this.Title = "Settings Browser";
            this.DataContext = this;
        }

        #endregion Constructors

        #region Enumerations

        public enum BrowserTypeEnum
        {
            SETTINGS,
            EXPERIMENT
        }

        #endregion Enumerations

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public bool BrowseBool
        {
            get { return this._browseBool; }
            set
            {
                this._browseBool = value;
                OnPropertyChanged("BrowseBool");
            }
        }

        public string BrowseExperimentPath
        {
            get { return this._browseExperimentPath; }
            set
            {
                this._browseExperimentPath = value;
                OnPropertyChanged("BrowseExperimentPath");
                SetExperimentSettingsViewer();
            }
        }

        public BrowserTypeEnum BrowserType
        {
            get { return this._browserType; }
            set
            {
                this._browserType = value;
            }
        }

        public bool ChooseFromDatabaseBool
        {
            get { return this._chooseFromDatabaseBool; }
            set
            {
                this._chooseFromDatabaseBool = value;
                OnPropertyChanged("ChooseFromDatabaseBool");
            }
        }

        public bool ChooseFromTemplateBool
        {
            get { return this._chooseFromTemplateBool; }
            set
            {
                this._chooseFromTemplateBool = value;
                OnPropertyChanged("ChooseFromTemplateBool");
            }
        }

        public string DatabaseExperimentName
        {
            get { return this._databaseExperimentName; }
            set
            {
                this._databaseExperimentName = value;
                OnPropertyChanged("DatabaseExperimentName");
            }
        }

        public string DatabaseExperimentPath
        {
            get { return this._databaseExperimentPath; }
            set
            {
                this._databaseExperimentPath = value;
                OnPropertyChanged("DatabaseExperimentPath");
            }
        }

        public RangeEnabledObservableCollection<DatabaseItem> DatabaseItems
        {
            get { return this._databaseItems; }
        }

        public string ExperimentPath
        {
            get { return this._experimentPath; }
        }

        public string ExperimentSettingsName
        {
            get { return this._experimentSettingsName; }
        }

        public string ExperimentSettingsPath
        {
            get { return this._experimentSettingsPath; }
        }

        public double ListBoxOpacity
        {
            get { return this._listBoxOpacity; }
            set
            {
                this._listBoxOpacity = value;
                OnPropertyChanged("ListBoxOpacity");
            }
        }

        public Visibility SpinVisible
        {
            get { return this._spinVisible; }
            set
            {
                this._spinVisible = value;
                OnPropertyChanged("SpinVisible");
            }
        }

        #endregion Properties

        #region Methods

        public bool GetAttribute(XmlNode node, XmlDocument doc, string attrName, ref string attrValue)
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

        //assign the attribute value to the input node and document
        //if the attribute does not exist add it to the document
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

        protected virtual void OnPropertyChanged(string propertyName)
        {
            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        private void AscendingOrder(object sender, RoutedEventArgs args)
        {
            if (_databaseItems.Count > 1)
            {
                listingDataView.SortDescriptions.Clear();
                listingDataView.SortDescriptions.Add(new SortDescription("ID", ListSortDirection.Ascending));
            }
        }

        private void Browse(object sender, RoutedEventArgs args)
        {
            BrowseBool = true;
            ChooseFromDatabaseBool = false;
            ChooseFromTemplateBool = false;
            ListBoxOpacity = 0.5;
            ListBox.UnselectAll();

            //Pass Experiment Settings File Location to Settings Viewer
            _experimentSettingsPath = _browseExperimentPath + "\\Experiment.xml";
            SetExperimentSettingsViewer();
        }

        private void btnBrowse_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog ofd = new Microsoft.Win32.OpenFileDialog();
            ofd.Title = "Select the Experiment Directory";
            ofd.InitialDirectory = BrowseExperimentPath;
            ofd.DefaultExt = "xml";
            ofd.Filter = "XML Files (*.xml)|*.xml";
            ofd.FileName = "Experiment.xml";
            try
            {
                if (true == ofd.ShowDialog())
                {
                    rdBrowse.IsChecked = true;
                    this.ListBox.UnselectAll();
                    BrowseExperimentPath = System.IO.Path.GetDirectoryName(ofd.FileName);
                    _experimentSettingsPath = BrowseExperimentPath + "\\Experiment.xml";
                    SetExperimentSettingsViewer();
                }
            }
            catch { }
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            _experimentPath = "";
            DialogResult = false;
            this.Close();
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            if (_chooseFromDatabaseBool)
            {
                _experimentPath = _databaseExperimentPath;
                if (null == _experimentPath)
                {
                    this.Close();
                    return;
                }
                _experimentSettingsName = new DirectoryInfo(_experimentPath).Name;
            }
            else if (_browseBool)
            {
                _experimentPath = _browseExperimentPath;
                _experimentSettingsPath = _experimentPath + "\\Experiment.xml";
                _experimentSettingsName = new DirectoryInfo(_experimentPath).Name;
            }
            else
            {
                if (cbChooseFromTamplate.Items.Count <= 0)
                {
                    this.Close();
                    return;
                }
                _experimentSettingsName = cbChooseFromTamplate.SelectedItem.ToString();
                _experimentSettingsPath = _settingTemplatesDirInfo.ToString() + "\\" + _experimentSettingsName + ".xml";
            }
            if (!Directory.Exists(_settingTemplatesDirInfo.ToString()))
            {
                Directory.CreateDirectory(_settingTemplatesDirInfo.ToString());
            }

            if (File.Exists(_experimentSettingsPath))
            {
                DialogResult = true;
                this.Close();
            }
            else
            {
                MessageBoxResult result = MessageBox.Show("The experiment settings file was not found.", "File Not Found", MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }
        }

        private void btnUpdateNotes_Click(object sender, RoutedEventArgs e)
        {
            //Check if there are templates and one is selected before trying to update the notes
            if (1 > cbChooseFromTamplate.Items.Count)
            {
                MessageBox.Show("Please add a template and select it before pressing the Update Notes button.", "No Templates", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            if (0 > cbChooseFromTamplate.SelectedIndex)
            {
                MessageBox.Show("Please select an experiment before pressing Update Notes button", "No Template Selected", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            UpdateNotesWindow dlg = new UpdateNotesWindow();
            XmlDocument expFile = new XmlDocument();
            expFile.Load(_experimentSettingsPath);
            XmlNodeList ndList = expFile.SelectNodes("/ThorImageExperiment/ExperimentNotes");
            string str = string.Empty;
            if (ndList.Count > 0)
            {
                if (true == GetAttribute(ndList[0], expFile, "text", ref str))
                {
                    dlg.Notes = str;
                }
            }
            if (null != expFile && true == dlg.ShowDialog())
            {
                if (ndList.Count <= 0)
                {
                    CreateXmlNode(expFile, "ExperimentNotes");
                    ndList = expFile.SelectNodes("/ThorImageExperiment/ExperimentNotes");
                }

                if (ndList.Count > 0)
                {
                    SetAttribute(ndList[0], expFile, "text", dlg.Notes);
                }
                expFile.Save(_experimentSettingsPath);
                SetExperimentSettingsViewer();
            }
        }

        private void cbChooseFromTamplate_GotFocus(object sender, RoutedEventArgs e)
        {
            rdChooseFromFavorites.IsChecked = true;
        }

        private void cbChooseFromTamplate_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            //Pass Experiment Settings File Location to Settings Viewer
            _experimentSettingsName = cbChooseFromTamplate.SelectedItem.ToString();
            _experimentSettingsPath = _settingTemplatesDirInfo.ToString() + "\\" + _experimentSettingsName + ".xml";
            SetExperimentSettingsViewer();

            rdChooseFromFavorites.IsChecked = true;
        }

        private void ChooseFromDatabase(object sender, RoutedEventArgs args)
        {
            BrowseBool = false;
            ChooseFromDatabaseBool = true;
            ChooseFromTemplateBool = false;
            ListBoxOpacity = 1;
            try
            {
                //Pass Experiment Settings File Location to Settings Viewer
                DatabaseItem databaseItem = ((DatabaseItem)this.ListBox.SelectedItem);
                _experimentSettingsPath = databaseItem.ExpPath + "\\Experiment.xml";
                _databaseExperimentPath = System.IO.Directory.GetParent(_experimentSettingsPath).ToString();
                _databaseExperimentName = databaseItem.ExpName;
                SetExperimentSettingsViewer();
            }
            catch { };
        }

        private void ChooseFromTemplates(object sender, RoutedEventArgs args)
        {
            BrowseBool = false;
            ChooseFromDatabaseBool = false;
            ChooseFromTemplateBool = true;
            ListBoxOpacity = 0.5;
            try
            {
                //Pass Experiment Settings File Location to Settings Viewer
                _experimentSettingsName = cbChooseFromTamplate.SelectedItem.ToString();
                _experimentSettingsPath = _settingTemplatesDirInfo.ToString() + "\\" + _experimentSettingsName + ".xml";
                SetExperimentSettingsViewer();
            }
            catch { };
        }

        private void CreateXmlNode(XmlDocument doc, string nodeName)
        {
            XmlNode node = doc.CreateNode(XmlNodeType.Element, nodeName, null);
            doc.DocumentElement.AppendChild(node);
        }

        private void DescendingOrder(object sender, RoutedEventArgs args)
        {
            if (_databaseItems.Count > 1)
            {
                listingDataView.SortDescriptions.Clear();
                listingDataView.SortDescriptions.Add(new SortDescription("ID", ListSortDirection.Descending));
            }
        }

        void ExperimentSettingsBrowserWindow_Closing(object sender, CancelEventArgs e)
        {
            if (false == _allowExit)
            {
                e.Cancel = true;
            }
        }

        void ExperimentSettingsBrowserWindow_Loaded(object sender, RoutedEventArgs e)
        {
            BrowserWin.IsEnabled = false;
            _allowExit = false;
            SpinVisible = Visibility.Visible;
            BackgroundWorker settingsLoader = new BackgroundWorker();
            settingsLoader.DoWork += (o, ea) =>
            {
                LoadDatabase();
                SetExperimentSettingsViewer();
            };

            settingsLoader.RunWorkerCompleted += (o, ea) =>
            {
                listingDataView.SortDescriptions.Clear();
                listingDataView.SortDescriptions.Add(new SortDescription("ID", ListSortDirection.Descending));
                BrowserWin.IsEnabled = true;
                SpinVisible = Visibility.Collapsed;
                _allowExit = true;
            };
            settingsLoader.RunWorkerAsync();
            LoadTemplateCombox();
        }

        private void ExperimentSettingsBrowserWindow_Unloaded(object sender, RoutedEventArgs e)
        {
            DataStore.Instance.Close();
        }

        private void ListBoxSelectionChanged(object sender, RoutedEventArgs e)
        {
            if (_chooseFromDatabaseBool)
            {
                try
                {
                    DatabaseItem databaseItem = ((DatabaseItem)this.ListBox.SelectedItem);
                    _experimentSettingsPath = databaseItem.ExpPath + "Experiment.xml";
                    _databaseExperimentPath = System.IO.Directory.GetParent(_experimentSettingsPath).ToString();
                    _databaseExperimentName = databaseItem.ExpName;
                    SetExperimentSettingsViewer();
                }
                catch { };
            }
        }

        private void ListBox_GotFocus(object sender, RoutedEventArgs e)
        {
            rdChooseFromDatabase.IsChecked = true;
        }

        private void LoadDatabase()
        {
            List<DatabaseItem> databaseItems = new List<DatabaseItem>();
            try
            {
                DataStore.Instance.ConnectionString = "URI=file:" + Application.Current.Resources["ThorDatabase"].ToString();
                DataStore.Instance.Open();

                int i = 0;
                DataTable table = new DataTable("Experiments");

                foreach (DataRow row in DataStore.Instance.ExperimentsDataSet.Tables[0].Rows)
                {
                    i++;
                    DatabaseItem databaseItem = new DatabaseItem();
                    databaseItem.ID = i;
                    databaseItem.ExpName = row["Name"].ToString();
                    databaseItem.ExpPath = row["Path"].ToString();
                    databaseItems.Add(databaseItem);
                }
            }
            catch (Exception e)
            {
                e.ToString();
            }
            _databaseItems.AddRange(databaseItems);
        }

        private void LoadTemplateCombox()
        {
            switch (_browserType)
            {
                case BrowserTypeEnum.SETTINGS:
                    try
                    {
                        string[] strList = Directory.GetFiles(_settingTemplatesDirInfo.FullName, "*.xml");
                        FileInfo[] experimentNames = _settingTemplatesDirInfo.GetFiles("*.xml");

                        for (int i = 0; i < strList.Length; i++)
                        {
                            string str = System.IO.Path.GetFileNameWithoutExtension(strList[i]);
                            cbChooseFromTamplate.Items.Add(str);
                        }
                        cbChooseFromTamplate.SelectedIndex = 0;
                        _experimentSettingsName = cbChooseFromTamplate.SelectedItem.ToString();
                        _experimentSettingsPath = _settingTemplatesDirInfo.ToString() + "\\" + _experimentSettingsName + ".xml";
                    }
                    catch (Exception e)
                    {
                        e.ToString();
                    }

                    break;
                case BrowserTypeEnum.EXPERIMENT:
                    spFavorites.Visibility = Visibility.Collapsed;
                    spFavorites.Height = 0;
                    rdChooseFromDatabase.IsChecked = true;
                    try
                    {
                        //Pass Experiment Settings File Location to Settings Viewer
                        DatabaseItem databaseItem = ((DatabaseItem)this.ListBox.SelectedItem);
                        _experimentSettingsPath = databaseItem.ExpPath + "\\Experiment.xml";
                        _databaseExperimentPath = System.IO.Directory.GetParent(_experimentSettingsPath).ToString();
                        _databaseExperimentName = databaseItem.ExpName;
                    }
                    catch (Exception e)
                    {
                        e.ToString();
                    }
                    break;
            }
        }

        private void SaveToSettingsTemplates()
        {
            XmlDocument experimentSettingsDoc = new XmlDocument();
            experimentSettingsDoc.Load(ExperimentSettingsPath);
            experimentSettingsDoc.Save(_settingTemplatesDirInfo.ToString() + "\\" + _experimentSettingsName + ".xml");
        }

        //Pass Experiment Settings File Location to Settings Viewer
        private void SetExperimentSettingsViewer()
        {
            try
            {
                XmlDocument doc = new XmlDocument();
                doc.LoadXml("<Command><Experiment value=\"" + _experimentSettingsPath + "\"></Experiment></Command>");
                doc.PreserveWhitespace = true;
                settingsPreview.EditEnable = false;
                settingsPreview.SettingsDocument = doc;
            }
            catch (Exception e)
            {
                e.ToString();
            }
        }

        private void TextBox_GotFocus(object sender, RoutedEventArgs e)
        {
            rdBrowse.IsChecked = true;
        }

        #endregion Methods
    }

    public class RangeEnabledObservableCollection<T> : ObservableCollection<T>
    {
        #region Methods

        public void AddRange(IEnumerable<T> items)
        {
            foreach (var item in items)
                this.Items.Add(item);
            //Not necessary to call OnCollectionChanged Event for the purposes of this project
            //this.OnCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
        }

        protected override void OnCollectionChanged(NotifyCollectionChangedEventArgs e)
        {
        }

        #endregion Methods
    }
}