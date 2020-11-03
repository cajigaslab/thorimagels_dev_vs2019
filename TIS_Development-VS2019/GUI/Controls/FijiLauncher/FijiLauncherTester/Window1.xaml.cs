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
using FijiLauncherTester.ViewModel;

namespace FijiLauncherTester
{
    

    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        private FijiLauncherControl _fl;
        //private System.Windows.Forms.Timer _statusTimer;

        private FijiLaunchTesterViewModel _vm = null;
        //private static bool _ijmFinihsed;


        //private bool _isLogOn;

        //public  bool IjmFinished
        //{
        //    get 
        //    {
        //       // return _fl.ProcessFinished;

        //        return _isLogOn;
        //    }
        //    set 
        //    {
        //        _isLogOn = value;
        //       OnPropertyChanged("IjmFinished");
        //    }
        //}

        public Window1()
        {
            InitializeComponent();
         
            this.Loaded += new RoutedEventHandler(Window1_Loaded);
            this.Unloaded += new RoutedEventHandler(Window1_Unloaded);          
        }

        void Window1_Unloaded(object sender, RoutedEventArgs e)
        {

            //_fl = new FijiLauncherControl();
           
       //     _fl.IjmFinished -= new Action<bool>(FlFinished);
            _fl.IjmFinished -= new IjmFinishedEventHandler(_fl_IjmFinished);

          //  this._statusTimer.Stop();

           // throw new NotImplementedException();
        }

        void Window1_Loaded(object sender, RoutedEventArgs e)
        {

            _fl = new FijiLauncherControl();
            _vm = new FijiLaunchTesterViewModel();

            _vm.IjmFinished = false;
            _vm.IsLogOn = false;
           // _fl.IjmFinished += new Action<bool>(FlFinished);
            _fl.IjmFinished += new IjmFinishedEventHandler(_fl_IjmFinished);

            //this._statusTimer = new System.Windows.Forms.Timer();  // read log 4 times per sec 
            //this._statusTimer.Interval = 125;
            //this._statusTimer.Tick += new EventHandler(_statusTimer_Tick);
           
            //throw new NotImplementedException();
        }

        void _fl_IjmFinished(object sender, EventArgs e)
        {
            _vm.IjmFinished = true;
           
            //throw new NotImplementedException();
        }


        //void FlFinished(bool val)
        //{
        //    //System.Windows.MessageBox.Show(System.Windows.Application.Current.MainWindow, "Finished");
        //    _vm.IjmFinished = true;
        //    System.Media.SystemSounds.Beep.Play();
        //}

        //void _statusTimer_Tick(object sender, EventArgs e)
        //{
        //    try
        //    {
        //        //_vm.IjmFinished = true;
                
        //        //if (_fl.ProcessFinished)
        //        //{
        //        //    _statusTimer.Stop();

        //        //    System.Windows.MessageBox.Show(System.Windows.Application.Current.MainWindow, "Finished");
        //        //    //System.Windows.MessageBox.Show("Process is finished");
                    
        //        //}
        //    }
        //    catch (Exception ex)
        //    {
        //    }
        //}      

        private void FijiLaucherButton_Click(object sender, RoutedEventArgs e)
        {         
            //_statusTimer.Start();           
            _fl.LaunchFiji();
           
        }

        private void bOpenFijiDialog_Click(object sender, RoutedEventArgs e)
        {
             // Create an instance of the open file dialog box.
            OpenFileDialog openFijiDlg = new OpenFileDialog();
            Stream myStream = null;

            // Set filter options and filter index.
            openFijiDlg.Filter = "Application (.exe)|*.exe|All Files (*.*)|*.*";
            openFijiDlg.FilterIndex = 1;

            openFijiDlg.Multiselect = false;

            if (openFijiDlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                try
                {
                    if ((myStream = openFijiDlg.OpenFile()) != null)
                    {
                        using (myStream)
                        {
                            // Insert code to read the stream here.
                            _fl.FijiExeFile = openFijiDlg.FileName;
                        }
                    }
                }
                catch (Exception ex)
                {
                    System.Windows.Forms.MessageBox.Show("Error: Could not read fiji application file from disk. Original error: " + ex.Message);
                }
            }
        }

        private void bOpenIjmFileDialog_Click(object sender, RoutedEventArgs e)
        {

            // Create an instance of the open file dialog box.
            OpenFileDialog openIjmDlg = new OpenFileDialog();
            Stream myStream = null;

            // Set filter options and filter index.
            openIjmDlg.Filter = "ImageJ Macro (.ijm)|*.ijm|All Files (*.*)|*.*";
            openIjmDlg.FilterIndex = 1;

            openIjmDlg.Multiselect = false;

            if (openIjmDlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                try
                {
                    if ((myStream = openIjmDlg.OpenFile()) != null)
                    {
                        using (myStream)
                        {
                            // Insert code to read the stream here.
                            _fl.IjmFile = openIjmDlg.FileName;
                        }
                    }
                }
                catch (Exception ex)
                {
                    System.Windows.Forms.MessageBox.Show("Error: Could not read ijm file from disk. Original error: " + ex.Message);
                }
            }

        }

        private void bOpenFileDialog_Click(object sender, RoutedEventArgs e)
        {
            // Create an instance of the open file dialog box.
            FolderBrowserDialog filePath = new FolderBrowserDialog();

            if (filePath.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                tbFile.Text = filePath.SelectedPath + "\\";
                _fl.InputDir = filePath.SelectedPath + "\\";
            }
            else
            {
                tbFile.Text = "Please Specify The Profile Path";
            }           
        }

        private void LoggingNo_Checked(object sender, RoutedEventArgs e)
        {
         //   FijiLaunchTesterViewModel vm = (FijiLaunchTesterViewModel)this.DataContext;
            _vm.IsLogOn = false;
            _fl.IsLogOn = false;
            
        }

        private void LoggingYes_Checked(object sender, RoutedEventArgs e)
        {
          //  FijiLaunchTesterViewModel vm = (FijiLaunchTesterViewModel)this.DataContext;
            _vm.IsLogOn = true;
            _fl.IsLogOn = true;
            
        }

        
    }

    public class BoolToStringConverter : IValueConverter
    {
        public Object FalseValue { get; set; }
        public Object TrueValue { get; set; }

        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if (value == null)
                return FalseValue;
            else
                return (bool)value ? TrueValue : FalseValue;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return value != null ? value.Equals(TrueValue) : false;
        }
    }
}
