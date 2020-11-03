namespace DigitalOutputSwitchesTestApp
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
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

        private int _switchState0;

        #endregion Fields

        #region Constructors

        public MainWindow()
        {
            InitializeComponent();
            layoutRoot.DataContext = this;
        }

        #endregion Constructors

        #region Events

        /// Raised when a property on this object has a new value.
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public int SwitchStateTest
        {
            get
            {
                return _switchState0;
            }
            set
            {
                _switchState0 = value;
                OnPropertyChanged("SwitchStateTest");
            }
        }

        #endregion Properties

        #region Methods

        public void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        #endregion Methods
    }
}