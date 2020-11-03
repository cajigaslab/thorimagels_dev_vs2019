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

    public class PercentConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            double dVal = Convert.ToDouble(value);
            Decimal dec = new Decimal();
            //Avoid division by 0
            LaserControlView.slider1Max = (0 == LaserControlView.slider1Max) ? 1 : LaserControlView.slider1Max;
            LaserControlView.slider2Max = (0 == LaserControlView.slider2Max) ? 1 : LaserControlView.slider2Max;
            LaserControlView.slider3Max = (0 == LaserControlView.slider3Max) ? 1 : LaserControlView.slider3Max;
            LaserControlView.slider4Max = (0 == LaserControlView.slider4Max) ? 1 : LaserControlView.slider4Max; 

            switch (Convert.ToInt32(parameter))
            {
                case 1: dec = new Decimal((dVal - LaserControlView.slider1Min) * 100 / (LaserControlView.slider1Max - LaserControlView.slider1Min)); break;
                case 2: dec = new Decimal((dVal - LaserControlView.slider2Min) * 100 / (LaserControlView.slider2Max - LaserControlView.slider2Min)); break;
                case 3: dec = new Decimal((dVal - LaserControlView.slider3Min) * 100 / (LaserControlView.slider3Max - LaserControlView.slider3Min)); break;
                case 4: dec = new Decimal((dVal - LaserControlView.slider4Min) * 100 / (LaserControlView.slider4Max - LaserControlView.slider4Min)); break;
            }

            if (targetType == typeof(string))
            {
                return Decimal.Round(dec, 2).ToString();
            }
            else
            {
                return Convert.ToDouble(Decimal.Round(dec, 2).ToString());
            }
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            double dVal = Convert.ToDouble(value);
            Decimal dec = new Decimal();

            switch (Convert.ToInt32(parameter))
            {
                case 1: dec = new Decimal((dVal - 0) * (LaserControlView.slider1Max - LaserControlView.slider1Min) / 100 + LaserControlView.slider1Min); break;
                case 2: dec = new Decimal((dVal - 0) * (LaserControlView.slider2Max - LaserControlView.slider2Min) / 100 + LaserControlView.slider2Min); break;
                case 3: dec = new Decimal((dVal - 0) * (LaserControlView.slider3Max - LaserControlView.slider3Min) / 100 + LaserControlView.slider3Min); break;
                case 4: dec = new Decimal((dVal - 0) * (LaserControlView.slider4Max - LaserControlView.slider4Min) / 100 + LaserControlView.slider4Min); break;
            }

            if (targetType == typeof(string))
            {
                return Decimal.Round(dec, 2).ToString();
            }
            else
            {
                return Convert.ToDouble(Decimal.Round(dec, 2).ToString());
            }
        }

        #endregion Methods
    }
}