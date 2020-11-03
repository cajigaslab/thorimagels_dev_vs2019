using ScriptMatlab.ViewModels;
using System.Windows.Controls;
using System.Xml;

namespace ScriptMatlab
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    /// 

    //why named UserControl1 : as the scriptmanger need to load class "Usercontrol1"
    public partial class UserControl1 : UserControl
    {
        private MainViewModel _mainViewModel;
        public XmlDocument SettingsDocument {
            set { _mainViewModel.SettingsDocument = value; }
            get { return _mainViewModel.SettingsDocument; }
        }

        public UserControl1()
        {
            InitializeComponent();
            _mainViewModel = new MainViewModel();
            DataContext = _mainViewModel;
        }
    }

    
}
