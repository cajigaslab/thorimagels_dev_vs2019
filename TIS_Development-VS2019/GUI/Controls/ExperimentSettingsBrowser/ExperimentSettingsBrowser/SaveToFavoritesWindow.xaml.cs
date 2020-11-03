namespace ExperimentSettingsBrowser
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;

    /// <summary>
    /// Interaction logic for SaveToFavoritesWindow.xaml
    /// </summary>
    public partial class SaveToFavoritesWindow : Window
    {
        #region Fields

        private string _alias;
        private string _dirctory;
        private string _templateOriginalName;

        #endregion Fields

        #region Constructors

        public SaveToFavoritesWindow()
        {
            InitializeComponent();

            this.DataContext = this;
        }

        #endregion Constructors

        #region Properties

        public string Alias
        {
            get { return _alias; }
            set { _alias = value; }
        }

        public string Directory
        {
            set { _dirctory = value; }
        }

        public string TemplateOriginalName
        {
            set { _templateOriginalName = value; }
        }

        #endregion Properties

        #region Methods

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            this.Close();
        }

        private void btnSave_Click(object sender, RoutedEventArgs e)
        {
            if (_alias == null || _alias == String.Empty)
            {
                MessageBox.Show("Settings Template Name is empty! Please enter a valid Name.");
            }
            else
            {
                if (File.Exists(_dirctory + "\\" + _alias + ".xml") && _templateOriginalName != _alias)
                {
                    MessageBoxResult m = MessageBox.Show("A settings template already exists with the name: " + _alias + ". Do you wish to replace the existing settings template?", "File Already Exists", MessageBoxButton.YesNo);
                    if (m == MessageBoxResult.Yes)
                    {
                        DialogResult = true;
                        this.Close();
                    }
                    else
                    {
                        return;
                    }
                }
                else
                {
                    DialogResult = true;
                    this.Close();
                }
            }
        }

        #endregion Methods
    }
}