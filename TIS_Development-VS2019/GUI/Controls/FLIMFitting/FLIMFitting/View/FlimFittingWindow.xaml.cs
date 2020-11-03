namespace FLIMFitting.View
{
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

    /// <summary>
    /// Interaction logic for FlimFittingWindow.xaml
    /// </summary>
    public partial class FlimFittingWindow : Window
    {
        #region Constructors

        public FlimFittingWindow()
        {
            InitializeComponent();
            Owner = System.Windows.Application.Current.MainWindow;
            flimFittingView.UpdateTZero += flimFittingView_UpdateTZero;
        }

        #endregion Constructors

        #region Events

        public event Action<Dictionary<int, double>> UpdateTZero;

        #endregion Events

        #region Properties

        public bool AutoFitOnce
        {
            set
            {
                flimFittingView.AutoFitOnce = value;
            }
        }

        public List<FLIMHistogramGroupData> FLIMHistogramGroups
        {
            set
            {
                flimFittingView.FLIMHistogramGroups = value;
            }
        }

        #endregion Properties

        #region Methods

        void flimFittingView_UpdateTZero(Dictionary<int, double> tZeroDictionary)
        {
            if (null != UpdateTZero)
            {
                UpdateTZero(tZeroDictionary);
            }
        }

        #endregion Methods
    }
}