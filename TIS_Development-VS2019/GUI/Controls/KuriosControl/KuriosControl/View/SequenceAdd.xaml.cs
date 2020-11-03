namespace KuriosControl.View
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
    using System.Windows.Shapes;

    /// <summary>
    /// Interaction logic for CreateSequenceWindow.xaml
    /// </summary>
    public partial class SequenceAdd : Window, INotifyPropertyChanged
    {
        #region Fields

        double _exporsureMin = 0;
        double _exposureDefault = 30;
        double _exposureMax = 1000;
        double _exposureStart = 0;
        double _exposureStop = 0;

        #endregion Fields

        #region Constructors

        public SequenceAdd()
        {
            InitializeComponent();
            this.DataContext = this;
            this.Loaded += SequenceAdd_Loaded;
        }

        #endregion Constructors

        #region Events

        /// Raised when a property on this object has a new value.
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public double ExposureDefault
        {
            set
            {
                _exposureDefault = value;
            }
        }

        public double ExposureMax
        {
            get
            {
                return _exposureMax;
            }

            set
            {
                _exposureMax = value;
            }
        }

        public double ExposureMin
        {
            get
            {
                return _exporsureMin;
            }

            set
            {
                _exporsureMin = value;
            }
        }

        public double ExposureStart
        {
            get
            {
                return _exposureStart;
            }
            set
            {
                _exposureStart = value;
                OnPropertyChanged("ExposureStart");
            }
        }

        public double ExposureStop
        {
            get
            {
                return _exposureStop;
            }
            set
            {
                _exposureStop = value;
                OnPropertyChanged("ExposureStop");
            }
        }

        public string SequenceName
        {
            get
            {
                return tbName.Text;
            }
        }

        public int WavelengthMax
        {
            get;
            set;
        }

        public int WavelengthMin
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Called when [property changed].
        /// </summary>
        /// <param name="propertyName">Name of the property.</param>
        public void OnPropertyChanged(string propertyName)
        {
            this.VerifyPropertyName(propertyName);

            if (SequenceAddWin.PropertyChanged != null)
            {
                SequenceAddWin.PropertyChanged(SequenceAddWin, new PropertyChangedEventArgs(propertyName));
            }
        }

        /// <summary>
        /// Verifies the name of the property.
        /// </summary>
        /// <param name="propertyName">Name of the property.</param>
        public void VerifyPropertyName(string propertyName)
        {
            if (TypeDescriptor.GetProperties(SequenceAddWin)[propertyName] == null)
            {
                Debug.Fail("Invalid property name: " + propertyName);
            }
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
            this.Close();
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            if (string.Empty == tbName.Text)
            {
                MessageBox.Show("Please enter a valid name.");
                return;
            }
            this.DialogResult = true;
            this.Close();
        }

        void SequenceAdd_Loaded(object sender, RoutedEventArgs e)
        {
            ExposureStart = _exposureDefault;
            ExposureStop = _exposureDefault;
        }

        #endregion Methods
    }
}