namespace BesselBeamGen
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

    using BesselBeamGen.ViewModel;

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        #region Fields

        BesselBeamViewModel _vm = new BesselBeamViewModel();

        #endregion Fields

        #region Constructors

        public MainWindow()
        {
            InitializeComponent();

            this.DataContext = _vm;
            this.Loaded += MainWindow_Loaded;
            this.Unloaded += MainWindow_Unloaded;
            Application.Current.Exit += new ExitEventHandler(App_Exit);
        }

        #endregion Constructors

        #region Methods

        private void App_Exit(object sender, ExitEventArgs e)
        {
            _vm.UpdateSettings();
            this.Close();
        }

        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            _vm.LoadSettings();
        }

        void MainWindow_Unloaded(object sender, RoutedEventArgs e)
        {
            _vm.UpdateSettings();
        }

        #endregion Methods
    }
}