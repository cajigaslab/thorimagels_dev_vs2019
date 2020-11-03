namespace MultiROIStats
{
    using System;
    using System.Collections.Generic;
    using System.Data;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
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
    using System.Windows.Threading;
    using System.Xml;

    using MultiROIStats.ViewModel;

    //xmlns:dg="clr-namespace:Microsoft.Windows.Controls;assembly=WpfToolkit"
    /// <summary>
    /// Interaction logic for MultiROIStats.xaml
    /// </summary>
    public partial class MultiROIStatsUC : Window
    {
        #region Fields

        public MultiROIStatsViewModel _vm;

        private DispatcherTimer _timer = new DispatcherTimer();

        #endregion Fields

        #region Constructors

        public MultiROIStatsUC(string name = "")
        {
            InitializeComponent();
            _vm = new MultiROIStatsViewModel();
            DataContext = _vm;
            ((MultiROIStatsViewModel)DataContext).SavingStats += new Action (MultiROIStats_SavingStats);

            // Automatically resize width relative to content
            this.SizeToContent = SizeToContent.Width;

            // Automatically resize height relative to content
            this.SizeToContent = SizeToContent.Height;

            try
            {
                // Make this window the topmost within the app
                this.Owner = Application.Current.MainWindow;
            }
            catch (Exception e)
            {
                e.ToString();
            }

            if (name != "")
            {
                this.Title = name;
            }

            LoadAppSettingsAttributes();

            this.Loaded += new RoutedEventHandler(ROIStats_loaded);
            this.Unloaded += new RoutedEventHandler(MultiROIStatsUC_Unloaded);
            this.Closed += new EventHandler(MultiROIStatsUC_Closed);
        }

        #endregion Constructors

        #region Events

        public event Action SavingStats;

        #endregion Events

        #region Methods

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetApplicationSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        public static extern int GetApplicationSettingsFilePathAndName(StringBuilder sb, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetHardwareSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        public static extern int GetHardwareSettingsFilePathAndName(StringBuilder sb, int length);

        public string GetApplicationSettingsFileString()
        {
            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetApplicationSettingsFilePathAndName(sb, PATH_LENGTH);

            return sb.ToString();
        }

        public string GetHardwareSettingsFileString()
        {
            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetHardwareSettingsFilePathAndName(sb, PATH_LENGTH);

            return sb.ToString();
        }

        public void SetArithmeticsData(string[] statsNames, double[] stats)
        {
            if (null != DataContext)
            {
                ((MultiROIStatsViewModel)DataContext).SetArithmeticsData(statsNames, stats);
            }
        }

        public void SetData(string[] statsNames, double[] stats)
        {
            if (null != DataContext)
            {
                ((MultiROIStatsViewModel)DataContext).SetData(statsNames,stats);
            }
        }

        public void SetFieldSize(int fieldSize)
        {
            if (null != DataContext)
            {
                ((MultiROIStatsViewModel)DataContext).FieldSize = fieldSize;
            }
        }

        private void arithmeticsDg_AutoGeneratingColumn(object sender, DataGridAutoGeneratingColumnEventArgs e)
        {
            using (var d = this.ArithmeticsDg.Dispatcher.DisableProcessing())
            {
                if (e.PropertyType == typeof(double))
                {
                    DataGridTextColumn dataGridTextColumn = e.Column as DataGridTextColumn;
                    if (dataGridTextColumn != null)
                    {
                        dataGridTextColumn.Binding.StringFormat = "F5";
                    }
                }

                if (e.Column.Header.ToString().Equals("Index"))
                {
                    e.Column.Visibility = Visibility.Collapsed;
                }

                if (e.Column.Header.ToString().Equals("Variable"))
                {
                    e.Column.MinWidth = 55;
                }
            }
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void dg_AutoGeneratingColumn(object sender, DataGridAutoGeneratingColumnEventArgs e)
        {
            using (var d = this.dg.Dispatcher.DisableProcessing())
            {
                if (e.Column.Header.ToString().Contains("height") || e.Column.Header.ToString().Contains("width"))
                {
                    e.Column.MinWidth = 45;
                }
                else if (e.Column.Header.ToString().Contains("min") || e.Column.Header.ToString().Contains("max")
                || e.Column.Header.ToString().Contains("top") || e.Column.Header.ToString().Contains("left"))
                {
                    e.Column.MinWidth = 40;
                }
                else if (e.PropertyType == typeof(double))
                {
                    DataGridTextColumn dataGridTextColumn = e.Column as DataGridTextColumn;
                    if (dataGridTextColumn != null)
                    {
                        dataGridTextColumn.Binding.StringFormat = "F4";
                    }
                }
                else if (e.Column.Header.ToString().Equals("Index"))
                {
                    e.Column.Width = 40;
                }

            }
        }

        void LoadAppSettingsAttributes()
        {
            string appSettings = GetApplicationSettingsFileString();
            XmlDocument doc = new XmlDocument();
            doc.Load(appSettings);
            XmlNodeList ndList = doc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/StatsTableSaveButtonView");
            if (ndList.Count > 0)
            {
                if (ndList[0].Attributes["Visibility"].Value.Equals("Visible"))
                {
                    btnSave.Visibility = Visibility.Visible;
                }
                else
                {
                    btnSave.Visibility = Visibility.Collapsed;
                }
            }
        }

        void MultiROIStatsUC_Closed(object sender, EventArgs e)
        {
            try
            {
                Application.Current.MainWindow.Activate();
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        void MultiROIStatsUC_Unloaded(object sender, RoutedEventArgs e)
        {
            _timer.Tick -= _timer_Tick;
            _timer.Stop();
            _vm.ClearData();
            dg = null;
            ArithmeticsDg = null;
        }

        private void MultiROIStats_SavingStats()
        {
            if (SavingStats != null)
            {
                SavingStats();
            }
        }

        void ROIStats_loaded(object sender, RoutedEventArgs e)
        {
            _timer.Interval = new TimeSpan(0, 0, 0, 0, 500);
            _timer.Tick += _timer_Tick;
            _timer.Start();
        }

        void _timer_Tick(object sender, EventArgs e)
        {
            using (var d = this.dg.Items.Dispatcher.DisableProcessing())
            {
                _vm.UpdateDataTable();
            }
        }

        #endregion Methods
    }

    public class MyBkColorConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            SolidColorBrush brush = Brushes.Transparent;

            DataRowView drv = value as DataRowView;

            if ((null != drv) && (null != drv[0]))
            {
                int val = System.Convert.ToInt32(drv[0]);
                val = (val - 1) % 8;

                switch (val)
                {
                    case 0: brush = Brushes.Yellow; break;
                    case 1: brush = Brushes.Lime; break;
                    case 2: brush = Brushes.DodgerBlue; break;
                    case 3: brush = Brushes.DeepPink; break;
                    case 4: brush = Brushes.DarkOrange; break;
                    case 5: brush = Brushes.Khaki; break;
                    case 6: brush = Brushes.LightGreen; break;
                    case 7: brush = Brushes.SteelBlue; break;
                }

            }
            return brush;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }

        #endregion Methods
    }
}