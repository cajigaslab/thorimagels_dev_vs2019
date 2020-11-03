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
    /// Interaction logic for SnapshotSettings.xaml
    /// </summary>
    public partial class ZoomSettings : Window, INotifyPropertyChanged
    {
        #region Fields

        private int _zoomLevel;

        #endregion Fields

        #region Constructors

        public ZoomSettings()
        {
            InitializeComponent();

            this.Loaded += new RoutedEventHandler(ZoomSettings_Loaded);
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public int ZoomLevel
        {
            get
            {
                return _zoomLevel;
            }
            set
            {
                _zoomLevel = value;
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

        private void Button_OnCancel(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            Close();
        }

        private void Button_OnOK(object sender, RoutedEventArgs e)
        {
            Regex regexNumberOnly = new Regex("^[0-9]*$");

            if (true == regexNumberOnly.IsMatch(txtZoom.Text))
            {
                _zoomLevel = Convert.ToInt32(txtZoom.Text);

                DialogResult = true;
                Close();
            }
            else
            {
                MessageBox.Show("Enter only numbers", "Invalid Value", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        void ZoomSettings_Loaded(object sender, RoutedEventArgs e)
        {
            txtZoom.Text = _zoomLevel.ToString();
            txtZoom.Focus();
            txtZoom.Select(txtZoom.Text.Length,1);
        }

        #endregion Methods
    }
}