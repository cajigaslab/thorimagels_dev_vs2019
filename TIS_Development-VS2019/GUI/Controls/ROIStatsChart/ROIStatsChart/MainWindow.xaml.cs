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
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.ComponentModel;

namespace ROIStatsChart
{
    using ROIStatsChart.ViewModel;
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : UserControl
    {
        #region Fields
        ChartViewModel _vm = new ChartViewModel();
        #endregion Fields

        public MainWindow()
        {
            InitializeComponent();
            this.Loaded += MW_Loaded;
        }

        void MW_Loaded(object sender, RoutedEventArgs e)
        {
            if (_vm != null)
            {
                this.DataContext = _vm;
                this.sciChartView.DataContext = _vm;
                this.ctrlButtonView.DataContext = _vm;
                this.legendView.DataContext = _vm;

                _vm.PropertyChanged += VM_PropertyChanged;
            }
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
    }
}
