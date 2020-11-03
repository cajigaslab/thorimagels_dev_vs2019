using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.ComponentModel;

namespace TestApp
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window, INotifyPropertyChanged
    {

        public Window1()
        {
            InitializeComponent();

            this.DataContext = this;

            VolumeInterface.HardwareSettingsFile = "C:\\Users\\mgao\\Documents\\ThorImageLS\\Application Settings\\HardwareSettings.xml";

            VolumeInt.HardwareSettingsFile = "C:\\Users\\mgao\\Documents\\ThorImageLS\\Application Settings\\HardwareSettings.xml";


            VolumeInterface.DataSpacingZ = 1;
            MyWhitePoint0 = 200;
            MyWhitePoint1 = 200;
            MyWhitePoint2 = 200;
            MyWhitePoint3 = 200;
            this.SizeChanged += new SizeChangedEventHandler(Window1_SizeChanged);
        }


        void Window1_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            VolumeInterface.Width = e.NewSize.Width/2;
            VolumeInterface.Height = e.NewSize.Height -spTest.Height - 30;
            VolumeInt.Width = e.NewSize.Width / 2;
            VolumeInt.Height = e.NewSize.Height - spTest.Height - 30;
        }

        private int _myWhitePoint0;

        public int MyWhitePoint0 {

            get
            {
                return _myWhitePoint0;
            }
            set
            {
                _myWhitePoint0 = value;
                OnPropertyChanged("MyWhitePoint0");
            }
        }

        private int _myWhitePoint1;

        public int MyWhitePoint1
        {

            get
            {
                return _myWhitePoint1;
            }
            set
            {
                _myWhitePoint1 = value;
                OnPropertyChanged("MyWhitePoint1");
            }
        }

        private int _myWhitePoint2;

        public int MyWhitePoint2
        {

            get
            {
                return _myWhitePoint2;
            }
            set
            {
                _myWhitePoint2 = value;
                OnPropertyChanged("MyWhitePoint2");
            }
        }

        private int _myWhitePoint3;

        public int MyWhitePoint3
        {

            get
            {
                return _myWhitePoint3;
            }
            set
            {
                _myWhitePoint3 = value;
                OnPropertyChanged("MyWhitePoint3");
            }
        }


        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Raises this object's PropertyChanged event.
        /// </summary>
        /// <param name="propertyName">The name of the property that has a new value.</param>
        protected virtual void OnPropertyChanged(string propertyName)
        {
            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            VolumeInterface.ZStackCacheDirectory = "C:\\Users\\mgao\\Documents\\ThorImageLS\\ZStackCache";
            //VolumeInterface.RenderVolume();
            VolumeInt.ZStackCacheDirectory = "C:\\Users\\mgao\\Documents\\ThorImageLS\\ZStackCache";
            //VolumeInt.RenderVolume();
        }
    }
}
