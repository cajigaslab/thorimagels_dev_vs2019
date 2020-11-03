namespace ExperimentSettingsBrowser
{
    using System;
    using System.Collections.Generic;
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
    /// Interaction logic for UpdateNotesWindow.xaml
    /// </summary>
    public partial class UpdateNotesWindow : Window
    {
        #region Fields

        private string _notes;

        #endregion Fields

        #region Constructors

        public UpdateNotesWindow()
        {
            InitializeComponent();

            this.DataContext = this;
        }

        #endregion Constructors

        #region Properties

        public string Notes
        {
            get { return _notes; }
            set { _notes = value; }
        }

        #endregion Properties

        #region Methods

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            this.Close();
        }

        private void btnUpdate_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
            this.Close();
        }

        #endregion Methods
    }
}