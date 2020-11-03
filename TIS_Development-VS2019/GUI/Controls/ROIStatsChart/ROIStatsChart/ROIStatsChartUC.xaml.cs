namespace ROIStatsChart
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Drawing;
    using System.IO;
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

    using Abt.Controls.SciChart;
    using Abt.Controls.SciChart.Model.DataSeries;
    using Abt.Controls.SciChart.Visuals;
    using Abt.Controls.SciChart.Visuals.RenderableSeries;

    using Microsoft.Win32;

    using ROIStatsChart.ViewModel;

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class ROIStatsChartUC : UserControl
    {
        #region Fields

        private ChartViewModel _vm;

        #endregion Fields

        #region Constructors

        public ROIStatsChartUC()
        {
            InitializeComponent();
            _vm = new ChartViewModel();
            Application.Current.Exit += Current_Exit;
            this.Loaded += MW_Loaded;
            this.Unloaded += MW_UnLoaded;
        }

        #endregion Constructors

        #region Properties

        public string[] ArithmeticNames
        {
            get
            {
                return _vm.ArithmeticNames;
            }
        }

        public double[] ArithmeticStats
        {
            get
            {
                return _vm.ArithmeticData;
            }
        }

        #endregion Properties

        #region Methods

        public void AppendROIStats(string[] statsNames, double[] stats, bool reLoad)
        {
            if ((null == _vm) || (null == statsNames) || (null == stats))
            {
                return;
            }

            _vm.StatsNames = statsNames;
            _vm.StatsData = stats;
            //using (_vm.SciChartSurface.SuspendUpdates())
            //{
            _vm.LoadDataFromDevice(reLoad);
            //}
        }

        public void ClearChart()
        {
            if (null == _vm)
            {
                return;
            }
            _vm.ClearData();
            _vm.DataStoreLoadComplete = false;
        }

        public void ClearLegendGroup(bool roiOnly)
        {
            if (null == _vm)
            {
                return;
            }
            _vm.ClearLegendGroup(roiOnly);
        }

        public void LoadSettings()
        {
            if (null != _vm)
            {
                _vm.LoadSettings();
            }
        }

        public void PersistSettings()
        {
            _vm.PersistSettings();
        }

        public void ResetChart(bool editable)
        {
            if (null == _vm)
            {
                return;
            }
            _vm.ClearData();
            _vm.Editable = editable;
            _vm.LoadSettings();
            _vm.DataStoreLoadComplete = false;
        }

        public void SaveArithmeticStats()
        {
            _vm.SaveROIArithmeticDataToH5();
        }

        public void SaveStatChart()
        {
            if (null == _vm || null == _vm.SciChartSurface)
            {
                return;
            }

            var svFlDlg = new SaveFileDialog
            {
                Filter = "Png|*.png|JPG|*.jpg|Bmp|*.bmp",
                Title = "Save as ..."
            };
            Nullable<bool> result = svFlDlg.ShowDialog();
            if (result == true && svFlDlg.FileName != "")
            {
                _vm.SciChartSurface.ExportToFile(svFlDlg.FileName, (ExportType)(svFlDlg.FilterIndex - 1));
            }
        }

        public void SetChartXLabel(string str)
        {
            if (null == _vm)
            {
                return;
            }
            _vm.ChartXLabel = str;
        }

        public void SetClockAsXAxis(bool val)
        {
            if (null == _vm)
            {
                return;
            }
            _vm.ClockAsXAxis = val;
        }

        public void SetDataStoreLoadComplete(bool complete)
        {
            _vm.DataStoreLoadComplete = complete;
        }

        public void SetFifoSize(int size)
        {
            if (null == _vm)
            {
                return;
            }
            _vm.FifoSize = size;
            _vm.ZoomExtendChartSeries();
        }

        public void SetFifoVisible(bool visible)
        {
            if (null == _vm)
            {
                return;
            }
            _vm.IsFifoVisible = visible;
        }

        public void SetInLoading(bool inLoading)
        {
            if (null == _vm)
            {
                return;
            }
            _vm.IsInLoad = inLoading;
        }

        public void SetLegendGroup(int groupID, string[] Names, bool[] Enables)
        {
            if ((null == _vm) || (null == Names) || (null == Enables))
            {
                return;
            }
            _vm.SetLegendGroup(groupID, Names, Enables);
        }

        public void SetPath(string path)
        {
            if (null == _vm)
            {
                return;
            }
            _vm.Path = path;
        }

        public void SetTag(int tag)
        {
            _vm.Tag = tag;
        }

        public void SkipGeometricInfo(bool val)
        {
            if (null != _vm)
            {
                _vm.SkipGeometricInfo = val;
            }
        }

        public void UpdataXReviewPosition(double x)
        {
            _vm.XReviewPosition = x * 1000;
        }

        public void UpdataXVisibleRange(int xMin, int xMax)
        {
            if (!_vm.ClockAsXAxis)
            {
                xMin *= 1000;
                xMax *= 1000;
            }
            _vm.XVisibleRange = new DoubleRange(xMin, xMax);
        }

        public void ZoomExtend()
        {
            if (null == _vm)
            {
                return;
            }
            _vm.ZoomExtendChartSeries();
        }

        void Current_Exit(object sender, ExitEventArgs e)
        {
            _vm.SaveROIArithmeticDataToH5();
            _vm.PersistSettings();
        }

        void MW_Loaded(object sender, RoutedEventArgs e)
        {
            if (_vm != null)
            {
                this.DataContext = _vm;
                this.sciChartView.DataContext = _vm;

                _vm.PropertyChanged += VM_PropertyChanged;
            }
        }

        void MW_UnLoaded(object sender, RoutedEventArgs e)
        {
            _vm.PropertyChanged -= VM_PropertyChanged;
            _vm.PersistSettings();
        }

        private void VM_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            switch (e.PropertyName)
            {
                case "Legend":

                    break;
                default:
                    break;
            }
        }

        #endregion Methods
    }
}