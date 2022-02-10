namespace CaptureSetupDll.View
{
    using System;
    using System.Collections.Generic;
    using System.Globalization;
    using System.Linq;
    using System.Text;
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
    /// Interaction logic for MCLSControlView.xaml
    /// </summary>
    public partial class LaserControlView : UserControl
    {
        #region Fields

        public static double slider1Max = (double)MVMManager.Instance["LaserControlViewModel", "Laser1Max", (object)0.0];
        public static double slider1Min = (double)MVMManager.Instance["LaserControlViewModel", "Laser1Min", (object)0.0];
        public static double slider2Max = (double)MVMManager.Instance["LaserControlViewModel", "Laser2Max", (object)0.0];
        public static double slider2Min = (double)MVMManager.Instance["LaserControlViewModel", "Laser2Min", (object)0.0];
        public static double slider3Max = (double)MVMManager.Instance["LaserControlViewModel", "Laser3Max", (object)0.0];
        public static double slider3Min = (double)MVMManager.Instance["LaserControlViewModel", "Laser3Min", (object)0.0];
        public static double slider4Max = (double)MVMManager.Instance["LaserControlViewModel", "Laser4Max", (object)0.0];
        public static double slider4Min = (double)MVMManager.Instance["LaserControlViewModel", "Laser4Min", (object)0.0];

        #endregion Fields

        #region Constructors

        public LaserControlView()
        {
            InitializeComponent();
            //this.Loaded += new RoutedEventHandler(MCLSControlView_Loaded);
        }

        #endregion Constructors
    }
}