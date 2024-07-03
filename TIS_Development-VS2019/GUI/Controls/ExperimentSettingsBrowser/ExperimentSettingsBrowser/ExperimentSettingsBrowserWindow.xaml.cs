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
    using Validations;
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
    using System.Diagnostics.Contracts;
    using System.Windows.Media.Animation;

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
        private int _pageNumber;
        private DirectoryInfo _settingTemplatesDirInfo;
        private Visibility _spinVisible;
        private int ExpListCount;

        #endregion Fields

        #region Constructors

        public ExperimentSettingsBrowserWindow()
        {
            InitializeComponent();

            _settingTemplatesDirInfo = new DirectoryInfo(Application.Current.Resources["TemplatesFolder"].ToString() + "\\Template Favorites");
            
            ExpListCount = 15;
            if (DataStore.Instance.ExperimentsDataSet.Tables[0].Rows.Count < ExpListCount)
            {
                this.ExpListCount = DataStore.Instance.ExperimentsDataSet.Tables[0].Rows.Count;
            }
            this.Loaded += new RoutedEventHandler(ExperimentSettingsBrowserWindow_Loaded);
            this.Unloaded += new RoutedEventHandler(ExperimentSettingsBrowserWindow_Unloaded);
            listingDataView = (CollectionViewSource)(this.Resources["listingDataView"]);
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
            _browserType = BrowserTypeEnum.EXPERIMENT;
            this.Closing += new CancelEventHandler(ExperimentSettingsBrowserWindow_Closing);
            this.Title = "Settings Browser";
            this.DataContext = this;
            _pageNumber = MaxPages;
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

        public Int32 ExpAmount
        {
            get { return this.ExpListCount; }
            set {
                int currPage = JumpToPosition;
                ExpListCount = value;
                OnPropertyChanged("ExpAmount");

                OnPropertyChanged("MaxPages");
                OnPropertyChanged("MaxPagesStr");

                if (ExpAmount > DataStore.Instance.ExperimentsDataSet.Tables[0].Rows.Count)
                {
                    this.ExpListCount = DataStore.Instance.ExperimentsDataSet.Tables[0].Rows.Count;
                    JumpToPosition = 1;
                    LoadDatabase();
                }
                else
                {

                    if (currPage > MaxPages)
                    {
                        JumpToPosition = MaxPages;
                    }
                    else
                    {
                        JumpToPosition = currPage;
                    }
 
                }
                listingDataView.View.Refresh();
                OnPropertyChanged("ExpAmount");

                OnPropertyChanged("MaxPages");
                OnPropertyChanged("MaxPagesStr");
            }
        }

        public Int32 MaxPages
        {
            get
            {
                double pageFrac = (double)DataStore.Instance.ExperimentsDataSet.Tables[0].Rows.Count / (double)ExpAmount;

                int retMaxPg = (int)Math.Ceiling(pageFrac);


                if(retMaxPg == 0)
                {
                    retMaxPg = 1;
                }

                return retMaxPg;
            }
            set
            {
            }
        }

        public string MaxPagesStr
        {
            get
            {
                return "/" + MaxPages;
            }
            set
            {
            }
        }

        

        public Int32 JumpToPosition
        {
            get {
                return (MaxPages + 1) - this._pageNumber;  

                  }
            set
            {

                int reverseJ = (MaxPages + 1) - value;

                if (ExpAmount >= DataStore.Instance.ExperimentsDataSet.Tables[0].Rows.Count)
                {
                    LoadDatabase();
                    this._pageNumber = 1;
                    return;
                }


                if (reverseJ >= 1 && reverseJ <= MaxPages)
                {
                    this._pageNumber = reverseJ;
                }
                else
                {
                    ColorAnimation colorAnimation = new ColorAnimation
                    {
                        To = Colors.Red,
                        Duration = TimeSpan.FromSeconds(0.5), // Duration of the animation (0.5 seconds in this example)
                        AutoReverse = true, // Animation plays in reverse after completing
                        RepeatBehavior = new RepeatBehavior(2) // Repeat twice (original + once more)
                    };
                    
                    Storyboard.SetTargetProperty(colorAnimation, new PropertyPath("(Border.BorderBrush).(SolidColorBrush.Color)"));// Set the target property of the animation (BorderBrushProperty in this case)

                    Storyboard storyboard = new Storyboard();// Create a Storyboard and add the ColorAnimation to it
                    storyboard.Children.Add(colorAnimation);
                    TextBox PageTextBox = this.FindName("PageTextBox") as TextBox;

                    Storyboard.SetTarget(colorAnimation, PageTextBox);   // Set the target of the animation to your TextBox's border
                    storyboard.Begin();
                    return;
                }
                                
                LoadPage(value); 
                


                listingDataView.View.Refresh();
                SetExperimentSettingsViewer(); 
                OnPropertyChanged("JumpToPosition");
                OnPropertyChanged("MaxPages");
                OnPropertyChanged("MaxPagesStr");
            }
        }

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

        private void btnAnimation(string buttonName, string templateName, string rectangleName)
        {
            ColorAnimation colorAnimation = new ColorAnimation
            {
                To = Colors.Red,
                Duration = TimeSpan.FromSeconds(.12), // Duration of the animation (0.5 seconds in this example)
                AutoReverse = true, // Animation plays in reverse after completing
                RepeatBehavior = new RepeatBehavior(2) // Repeat twice (original + once more)
            };

            Storyboard.SetTargetProperty(colorAnimation, new PropertyPath("Fill.Color"));// Set the target property of the animation (BorderBrushProperty in this case)

            Storyboard storyboard = new Storyboard();// Create a Storyboard and add the ColorAnimation to it
            storyboard.Children.Add(colorAnimation);

            Button LeftButton = FindName(buttonName) as Button;
            ControlTemplate LeftButtonTemplate = LeftButton.FindName(templateName) as ControlTemplate; // Replace "YourButtonTemplate" with the actual key of your Button template
            LeftButton.Template = LeftButtonTemplate;

            Rectangle backgroundRectLeft = LeftButtonTemplate.FindName(rectangleName, LeftButton) as Rectangle;

            Storyboard.SetTarget(colorAnimation, backgroundRectLeft);   // Set the target of the animation to your TextBox's border
            storyboard.Begin();
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
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Exception thrown, ExperimentSettingsBrowser window, btnBrowse_Click: " + ex.Message);
            }
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            _experimentPath = "";
            DialogResult = false;
            this.Close();
        }
        
        private void btnLeft_Click(object sender, RoutedEventArgs e)
        {
            if (JumpToPosition > 1)
            {
                JumpToPosition -= 1;
            }
            else
            {
                btnAnimation("Left", "LeftTemplate", "BackgroundRectLeft");
                return;
            }
        }

        private void btnRight_Click(object sender, RoutedEventArgs e)
        {
            if (JumpToPosition < MaxPages)
            {
                JumpToPosition += 1;
            }
            else
            {
                btnAnimation("RightButton","RightTemplate","BackgroundRectRight");
                return;
            }

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
                if (null == databaseItem) return;
                _experimentSettingsPath = databaseItem.ExpPath + "\\Experiment.xml";
                _databaseExperimentPath = System.IO.Directory.GetParent(_experimentSettingsPath).ToString();
                _databaseExperimentName = databaseItem.ExpName;
                SetExperimentSettingsViewer();
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Exception thrown, ExperimentSettingsBrowser window, ChooseFromDatabase: " + ex.Message);
            }
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
                if (null == cbChooseFromTamplate) return;
                _experimentSettingsName = cbChooseFromTamplate.SelectedItem.ToString();
                _experimentSettingsPath = _settingTemplatesDirInfo.ToString() + "\\" + _experimentSettingsName + ".xml";
                SetExperimentSettingsViewer();
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Exception thrown, ExperimentSettingsBrowser window, ChooseFromTemplates: " + ex.Message);
            }
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
            JumpToPosition = 1;
            SetExperimentSettingsViewer();
           
            listingDataView.SortDescriptions.Clear();
            listingDataView.SortDescriptions.Add(new SortDescription("ID", ListSortDirection.Descending));
            BrowserWin.IsEnabled = true;
            SpinVisible = Visibility.Collapsed;
            _allowExit = true;
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
                    if (null == databaseItem) return;
                    _experimentSettingsPath = databaseItem.ExpPath + "Experiment.xml";
                    _databaseExperimentPath = System.IO.Directory.GetParent(_experimentSettingsPath).ToString();
                    _databaseExperimentName = databaseItem.ExpName;
                    SetExperimentSettingsViewer();
                }
                catch (Exception ex)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Exception thrown, ExperimentSettingsBrowser window, ListBoxSelectionChanged: " + ex.Message);
                }
            }
        }

        private void ListBox_GotFocus(object sender, RoutedEventArgs e)
        {
            rdChooseFromDatabase.IsChecked = true;
        }

        private void LoadPage(int page, int offset = 0)
        {
            _databaseItems.Clear();
            List<DatabaseItem> databaseItems = new List<DatabaseItem>();
            try
            {
                DataStore.Instance.ConnectionString = "URI=file:" + Application.Current.Resources["ThorDatabase"].ToString();
                DataStore.Instance.Open();


                DataTable table = new DataTable("Experiments");

                page = page - 1; //Displayed page numbers are indexed at 1, the logic below is for 0 based index

                int totalItems = DataStore.Instance.ExperimentsDataSet.Tables[0].Rows.Count;
                int totalPages = (totalItems + ExpListCount - 1) / ExpListCount;  // Calculate total pages

                int startIndex;
                int itemsToLoad;

                if (page == totalPages)
                {
                    // Last page, load the remainder
                    startIndex = totalItems - ((totalPages - 1) * ExpListCount);
                    itemsToLoad = startIndex;
                }
                else
                {
                    // Not the last page, load ExpListCount items
                    startIndex = totalItems - (page * ExpListCount);
                    itemsToLoad = ExpListCount;
                }

                for (int x = startIndex - 1; x >= startIndex - itemsToLoad; x--)
                {
                    DataRow row = DataStore.Instance.ExperimentsDataSet.Tables[0].Rows[x];

                    DatabaseItem databaseItem = new DatabaseItem();
                    databaseItem.ID = x;
                    databaseItem.ExpName = row["Name"].ToString();
                    databaseItem.ExpPath = row["Path"].ToString();
                    databaseItems.Add(databaseItem);
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Exception thrown, ExperimentSettingsBrowser window, LoadDatabase: " + ex.Message);
            }
            _databaseItems.AddRange(databaseItems);
        }

        private void LoadDatabase(int pos, int offset = -1)
        {
            _databaseItems.Clear();
            List<DatabaseItem> databaseItems = new List<DatabaseItem>();
            try
            {
                DataStore.Instance.ConnectionString = "URI=file:" + Application.Current.Resources["ThorDatabase"].ToString();
                DataStore.Instance.Open();


                DataTable table = new DataTable("Experiments");

                int diff = ExpListCount;



                if(offset >= 0)
                {
                    diff = offset;
                }
                //get the size of the DB for looping
                //int lastIndex = DataStore.Instance.ExperimentsDataSet.Tables[0].Rows.Count;
                //loop from the end of the database to (diff) amount, diff is a GUI var that defaults to 15
                for (int x = pos; x > pos - diff; x--)
                {
                    DataRow row = DataStore.Instance.ExperimentsDataSet.Tables[0].Rows[x];

                    DatabaseItem databaseItem = new DatabaseItem();
                    databaseItem.ID = x;
                    databaseItem.ExpName = row["Name"].ToString();
                    databaseItem.ExpPath = row["Path"].ToString();
                    databaseItems.Add(databaseItem);
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Exception thrown, ExperimentSettingsBrowser window, LoadDatabase: " + ex.Message);
            }
            _databaseItems.AddRange(databaseItems);
        }

        private void LoadDatabase()
        {
            _databaseItems.Clear();
            List<DatabaseItem> databaseItems = new List<DatabaseItem>();
            try
            {
                DataStore.Instance.ConnectionString = "URI=file:" + Application.Current.Resources["ThorDatabase"].ToString();
                DataStore.Instance.Open();

                
                DataTable table = new DataTable("Experiments");
                int diff = ExpListCount;
                //get the size of the DB for looping
                int lastIndex = DataStore.Instance.ExperimentsDataSet.Tables[0].Rows.Count;
                //loop from the end of the database to (diff) amount, diff is a GUI var that defaults to 15
                
                for (int x = lastIndex - 1; x >lastIndex - 1 - diff; x--)
                {
                    DataRow row = DataStore.Instance.ExperimentsDataSet.Tables[0].Rows[x];

                    DatabaseItem databaseItem = new DatabaseItem();
                    databaseItem.ID = x;
                    databaseItem.ExpName = row["Name"].ToString();
                    databaseItem.ExpPath = row["Path"].ToString();
                    databaseItems.Add(databaseItem);
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Exception thrown, ExperimentSettingsBrowser window, LoadDatabase: " + ex.Message);
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
                        if (1 > strList.Length) return;

                        for (int i = 0; i < strList.Length; i++)
                        {
                            string str = System.IO.Path.GetFileNameWithoutExtension(strList[i]);
                            cbChooseFromTamplate.Items.Add(str);
                        }
                        cbChooseFromTamplate.SelectedIndex = 0;
                        _experimentSettingsName = cbChooseFromTamplate.SelectedItem.ToString();
                        _experimentSettingsPath = _settingTemplatesDirInfo.ToString() + "\\" + _experimentSettingsName + ".xml";
                    }
                    catch (Exception ex)
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Exception thrown, ExperimentSettingsBrowser window, LoadTemplateCombox, SETTINGS: " + ex.Message);
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
                        if (null == databaseItem) return;
                        _experimentSettingsPath = databaseItem.ExpPath + "\\Experiment.xml";
                        _databaseExperimentPath = System.IO.Directory.GetParent(_experimentSettingsPath).ToString();
                        _databaseExperimentName = databaseItem.ExpName;
                    }
                    catch (Exception ex)
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Exception thrown, ExperimentSettingsBrowser window, LoadTemplateCombox, EXPERIMENT: " + ex.Message);
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
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Exception thrown, ExperimentSettingsBrowser window, SetExperimentSettingsViewer: " + ex.Message);
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