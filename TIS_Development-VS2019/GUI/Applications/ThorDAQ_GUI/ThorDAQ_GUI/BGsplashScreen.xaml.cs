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
using System.ComponentModel;

namespace thordaqGUI
{
    /// <summary>
    /// Summary description for Class1
    /// </summary>
    /// 

    public partial class BGsplashScreen : Window
    {
        private string _displayText;
        private string _progressText;
        private int _progressVal;
        private BackgroundWorker _splashWkr;
        //#region Properties

        public string DisplayText
        {
            get { return _displayText; }
            set
            {
                _displayText = value;
                this.BGlblloading.Content = value;
            }
        }

        public string ProgressText
        {
            get
            {
                return _progressText;
            }
            set
            {
                _progressText = value;
                this.BGlblProgress.Content = value;
            }
        }

        public int ProgressValue
        {
            get
            {
                return _progressVal;
            }
            set
            {
                _progressVal = value;
                this.BGpbProgress.Value = value;
            }
        }

        public BGsplashScreen(BackgroundWorker splashWkr)
        {
            _splashWkr = splashWkr; // copy in the worker thread handle for cancel option
            InitializeComponent();
        }


        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            if (_splashWkr != null)
            {
                _splashWkr.CancelAsync();
            }
        }

    }
}
