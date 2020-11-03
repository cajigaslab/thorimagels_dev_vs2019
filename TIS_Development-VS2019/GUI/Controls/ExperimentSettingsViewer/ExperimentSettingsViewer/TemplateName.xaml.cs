namespace ExperimentSettingsViewer
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
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

    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class TemplateName : Window, INotifyPropertyChanged
    {
        #region Constructors

        public TemplateName()
        {
            InitializeComponent();
            this.DataContext = this;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public string NewName
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            e.Handled = true;
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = true;
            this.Close();
        }

        //   private void btnCancel_Click(object sender, RoutedEventArgs e)
        //   {
        //       Window_Closing(sender, (CancelEventArgs)e);
        //   }
        //   void Window_Closing(object sender, CancelEventArgs e)
        //   {
        //// If user doesn't want to close, cancel closure
        //        e.Cancel = true;
        //   }
        void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
        }

        #endregion Methods
    }
}