namespace OTMControlTestApp
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
    using System.Windows.Navigation;
    using System.Windows.Shapes;

    using OTMControl.ViewModel;

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        #region Fields

        private OTMControlViewModel _vm = null;

        #endregion Fields

        #region Constructors

        public MainWindow()
        {
            InitializeComponent();
            _vm = new OTMControlViewModel();
            layoutRoot.DataContext = _vm;
        }

        #endregion Constructors
    }
}