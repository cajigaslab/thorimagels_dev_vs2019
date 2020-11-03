namespace CaptureSetupDll
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.ServiceModel;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;
    using System.Xml;

    using ThorLogging;

    /// <summary>
    /// Interaction logic for SnapSettings.xaml
    /// </summary>
    public partial class SnapSettings : Window, INotifyPropertyChanged
    {
        #region Fields

        private bool _autoSave;
        private string _fileName;
        private string _filePath;
        private bool _includeExperimentInfo;

        #endregion Fields

        #region Constructors

        public SnapSettings()
        {
            InitializeComponent();
            this.Loaded += SnapSettings_Loaded;

            this.DataContext = this;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public bool AutoSave
        {
            get
            {
                return _autoSave;
            }
            set
            {
                _autoSave = value;
                OnPropertyChanged("AutoSave");
            }
        }

        public string FileName
        {
            get
            {
                return _fileName;
            }
            set
            {
                _fileName = value;
                OnPropertyChanged("FileName");
            }
        }

        public string FilePath
        {
            get
            {
                return _filePath;
            }
            set
            {
                _filePath = value;
                OnPropertyChanged("FilePath");
            }
        }

        public bool IncludeExperimentInfo
        {
            get
            {
                return _includeExperimentInfo;
            }
            set
            {
                _includeExperimentInfo = value;
                OnPropertyChanged("IncludeExperimentInfo");
            }
        }

        #endregion Properties

        #region Methods

        protected virtual void OnPropertyChanged(String propertyName)
        {
            if (System.String.IsNullOrEmpty(propertyName))
            {
                return;
            }
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        private void btnBrowse_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.SaveFileDialog ofd = new Microsoft.Win32.SaveFileDialog();

            ofd.FileName = "select.folder";
            ofd.InitialDirectory = FilePath;
            ofd.DefaultExt = ".*";
            ofd.Filter = "All Files (*.*)|*.*";

            Nullable<bool> result = ofd.ShowDialog();

            if (true == result)
            {
                FilePath = System.IO.Path.GetDirectoryName(ofd.FileName);
            }
        }

        private void Button_OnCancel(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }

        private void Button_OnOK(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }

        void SnapSettings_Loaded(object sender, RoutedEventArgs e)
        {
            string str = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\luts\\";

            if (!Directory.Exists(str))
            {
                return;
            }
        }

        #endregion Methods
    }
}