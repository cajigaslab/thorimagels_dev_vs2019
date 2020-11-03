namespace XYTileControlTestApp
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
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
    using System.Windows.Navigation;
    using System.Windows.Shapes;

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        #region Fields

        public double _scanFieldHeight = 0;
        public double _scanFieldWidth = 0;

        #endregion Fields

        #region Constructors

        public MainWindow()
        {
            InitializeComponent();
            this.DataContext = this;
            this.Closing += MainWindow_Closing;
        }

        #endregion Constructors

        #region Events

        /// <summary>
        /// Raised when a property on this object has a new value.
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public double ScanFieldHeight
        {
            get
            {
                return _scanFieldHeight;
            }
            set
            {
                _scanFieldHeight = value;
                OnPropertyChanged("ScanFieldHeight");
            }
        }

        public double ScanFieldWidth
        {
            get
            {
                return _scanFieldWidth;
            }
            set
            {
                _scanFieldWidth = value;
                OnPropertyChanged("ScanFieldWidth");
            }
        }

        #endregion Properties

        #region Methods

        public void OnPropertyChanged(string propertyName)
        {
            this.VerifyPropertyName(propertyName);

            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        public void VerifyPropertyName(string propertyName)
        {
            // verify that the property name matches a real,
            // public, instance property on this object.
            if (TypeDescriptor.GetProperties(this)[propertyName] == null)
            {
                Debug.Fail("Invalid property name: " + propertyName);
            }
        }

        private void ComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ComboBox comboBox = sender as ComboBox;
            switch (comboBox.SelectedIndex)
            {
                case 0: xyTileControl.Mode = XYTileControl.TileDisplayMode.Edit; break;
                case 1: xyTileControl.Mode = XYTileControl.TileDisplayMode.Capture; break;
                case 2: xyTileControl.Mode = XYTileControl.TileDisplayMode.View; break;
                default:
                    break;
            }
        }

        void MainWindow_Closing(object sender, CancelEventArgs e)
        {
        }

        #endregion Methods
    }
}