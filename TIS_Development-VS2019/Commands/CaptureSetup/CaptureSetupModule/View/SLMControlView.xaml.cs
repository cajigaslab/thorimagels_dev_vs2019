namespace CaptureSetupDll.View
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
    using System.Xml;

    using CaptureSetupDll.ViewModel;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for SLMControlView.xaml
    /// </summary>
    public partial class SLMControlView : UserControl
    {
        #region Fields

        CaptureSetupViewModel _vm;

        #endregion Fields

        #region Constructors

        public SLMControlView(CaptureSetupViewModel vm)
        {
            InitializeComponent();
            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this) == true)
                return;
            _vm = vm;
            this.Loaded += new RoutedEventHandler(SLMControlView_Loaded);
        }

        #endregion Constructors

        #region Methods

        private void SLMControlView_Loaded(object sender, RoutedEventArgs e)
        {
            //Use the visibitlity settings in application settings to setup the visibility on the controls
            //Bleach scanner control can be hidden
            stpSLMBleacherControl.Visibility = CaptureSetupViewModel.GetVisibility("/ApplicationSettings/DisplayOptions/CaptureSetup/BleachView/BleachCalibrationTool", "Visibility");

            stkBleachPockelSLM.DataContext = MVMManager.Instance["PowerControlViewModel", _vm];
        }

        #endregion Methods
    }
}