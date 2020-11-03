namespace FijiLauncherTester.ViewModel
{
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
    using FijiLauncher;
    using System.Windows.Forms;
    using System.IO;

    public class FijiLaunchTesterViewModel : ViewModelBase
    {
        private bool _ijmFinihsed;
        private bool _isLogOn;

        public bool IjmFinished
        {
            get
            {
                // return _fl.ProcessFinished;

                return _ijmFinihsed;
            }
            set
            {
                _ijmFinihsed = value;
                OnPropertyChanged("IjmFinished");
            }
        }

        public bool IsLogOn
        {
            get
            {
                // return _fl.ProcessFinished;

                return _isLogOn;
            }
            set
            {
                _isLogOn = value;
                OnPropertyChanged("IsLogOn");
            }
        }

    }
 
}