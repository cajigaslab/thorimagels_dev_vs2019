namespace HardwareSetupDll.View
{
    using System;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Xml;

    using FolderDialogControl;

    using HardwareSetupDll.ViewModel;

    using ThorImageInfastructure;

    using ThorLogging;

    public class EnumBooleanConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var ParameterString = parameter as string;

            if (ParameterString == null)

                return DependencyProperty.UnsetValue;

            if (Enum.IsDefined(value.GetType(), value) == false)

                return DependencyProperty.UnsetValue;

            object paramvalue = Enum.Parse(value.GetType(), ParameterString);

            if (paramvalue.Equals(value))
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var ParameterString = parameter as string;

            if (ParameterString == null)

                return DependencyProperty.UnsetValue;

            return Enum.Parse(targetType, ParameterString);
        }

        #endregion Methods
    }

    /// <summary>
    /// Interaction logic for HardwareSetupView.xaml
    /// </summary>
    public partial class HardwareSetupView : UserControl
    {
        #region Fields

        //XmlDocument _hwSettingsDoc = new XmlDocument();
        XmlDocument _appSettingsDoc = new XmlDocument();
        HardwareSetupViewModel _vm;

        #endregion Fields

        #region Constructors

        public HardwareSetupView()
        {
            InitializeComponent();

            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this))
            {
                return;
            }

            this.Loaded += new RoutedEventHandler(HardwareSetupView_Loaded);
            this.Unloaded += new RoutedEventHandler(HardwareSetupView_Unloaded);

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Methods

        private void Browse_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog ofd = new Microsoft.Win32.OpenFileDialog();
            ofd.Title = "Select the Streaming Directory";
            ofd.InitialDirectory = _vm.StreamingPath;
            ofd.CheckFileExists = false;
            ofd.FileName = "select.folder";
            ofd.DefaultExt = ".*";
            ofd.Filter = "All Files (*.*)|*.*";
            try
            {
                if (true == ofd.ShowDialog())
            {
                    _vm.StreamingPath = System.IO.Path.GetDirectoryName(ofd.FileName);
            }
            }
            catch { }
        }

        void HardwareSetupView_Loaded(object sender, RoutedEventArgs e)
        {
            _vm = (HardwareSetupViewModel)this.DataContext;

            //retrieve the hardware settings complete path and file name
            string hwSettings = _vm.GetHardwareSettingsFileString();

            XmlDocument _hwSettingsDoc = new XmlDocument();
            _hwSettingsDoc.Load(hwSettings);

            XmlNodeList ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Streaming");

            if(ndList.Count > 0)
            {
                _vm.StreamingPath = ndList[0].Attributes["path"].Value;

                //if the string is empty. set the path as the temp path for the current user
                if (_vm.StreamingPath.Length == 0)
                {
                    _vm.StreamingPath = System.Environment.GetEnvironmentVariable("TEMP");
                }
            }
        }

        void HardwareSetupView_Unloaded(object sender, RoutedEventArgs e)
        {
            //Check to see if StreamingPath exists:
            if (false == Directory.Exists(_vm.StreamingPath))
            {
                _vm.StreamingPath = System.Environment.GetEnvironmentVariable("TEMP");
            }
            //retrieve the hardware settings complete path and file name
            string hwSettings = _vm.GetHardwareSettingsFileString();
            XmlDocument _hwSettingsDoc = new XmlDocument();
            _hwSettingsDoc.Load(hwSettings);
            XmlNodeList ndList = _hwSettingsDoc.SelectNodes("/HardwareSettings/Streaming");

            if (ndList.Count > 0)
            {
                ndList[0].Attributes["path"].Value = _vm.StreamingPath;
            }

            _hwSettingsDoc.Save(hwSettings);
            ThorSharedTypes.MVMManager.Instance.LoadSettings();

            //Check tis tmp files on the drive of Stream Temp Path:
            LoadStreamTempPath();
        }

        void LoadStreamTempPath()
        {
            if (_vm.StreamingPath.Length>0)
            {
                string[] fileList = Directory.GetFiles(_vm.StreamingPath, "tis*.tmp");
                if (fileList.Length > 0)
                {
                    MessageBoxResult result = MessageBox.Show("Temporary files from a previous session(s) are on the drive, would you like to clean them up now?", _vm.StreamingPath, MessageBoxButton.YesNo, MessageBoxImage.Information);
                    if (1 == String.Compare(result.ToString(), "yes", false))
                    {
                        //try to delete all tis .tmp files:
                        try
                        {
                            foreach (string filename in fileList)
                            {
                                File.Delete(filename);
                            }
                        }
                        catch (Exception e)
                        {
                            ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, e.Message);
                            MessageBoxResult Error = MessageBox.Show("Some temporary files can not be deleted.", e.Message, MessageBoxButton.OK, MessageBoxImage.Warning);
                        }
                    }
                }
            }
        }

        #endregion Methods
    }
}