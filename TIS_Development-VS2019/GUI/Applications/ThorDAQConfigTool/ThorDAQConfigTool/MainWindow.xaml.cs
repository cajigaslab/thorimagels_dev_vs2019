using System.Windows;

namespace ThorDAQConfigTool
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();

            ThorDAQConfigControlWindow.Content = new ThorDAQConfigControl.MainWindow();
        }
    }
}
