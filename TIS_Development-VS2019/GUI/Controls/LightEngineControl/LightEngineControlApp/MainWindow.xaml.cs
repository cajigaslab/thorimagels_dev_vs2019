using System.Windows;

namespace LightEngineControlTestApp
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            this.InitializeComponent();

            var model = new LightEngineControl.ViewModel.LightEngineControlViewModel()
            {
                //MasterBrightness = 50.0
            };
            //this.lecUC.DataContext = model;
        }
    }
}