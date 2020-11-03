namespace SetScriptPath
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
    using System.Windows.Shapes;

    using FolderDialogControl;

    /// <summary>
    /// Interaction logic for EditPath.xaml
    /// </summary>
    public partial class EditPath : Window, INotifyPropertyChanged
    {
        #region Fields

        private string _alias;
        private string _path;

        #endregion Fields

        #region Constructors

        public EditPath()
        {
            InitializeComponent();
            this.DataContext = this;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public string Alias
        {
            get
            {
                return _alias;
            }
            set
            {
                _alias = value;
                OnPropertyChanged("Alias");
            }
        }

        public string Path
        {
            get
            {
                return _path;
            }
            set
            {
                _path = value;
                OnPropertyChanged("Path");
            }
        }

        #endregion Properties

        #region Methods

        private void btnBrowse_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog ofd = new Microsoft.Win32.OpenFileDialog();
            ofd.Title = "Select Folder";
            ofd.CheckFileExists = false;
            ofd.FileName = "select.folder";
            ofd.InitialDirectory = Path;
            ofd.DefaultExt = ".*";
            ofd.Filter = "All Files (*.*)|*.*";
            Nullable<bool> result = ofd.ShowDialog();

            if (true == result)
            {
                Path = System.IO.Path.GetDirectoryName(ofd.FileName);
            }
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            this.Close();
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            if (Alias == null || Alias == String.Empty)
            {
                if (Path == null || Path == String.Empty)
                    MessageBox.Show("Name and Path are empty! Please enter a valid Name and Path.");
                else
                {
                    MessageBox.Show("Name is empty! Please enter a valid Name.");
                }
            }
            else if (Path == null || Path == String.Empty)
            {
                MessageBox.Show("Path is empty! Please enter a valid Path.");
            }
            else if (!Directory.Exists(Path))
            {
                MessageBox.Show("Path does not exist! Please enter a valid Path.");
            }
            else
            {
                DialogResult = true;
                this.Close();
            }
        }

        void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
        }

        #endregion Methods
    }
}