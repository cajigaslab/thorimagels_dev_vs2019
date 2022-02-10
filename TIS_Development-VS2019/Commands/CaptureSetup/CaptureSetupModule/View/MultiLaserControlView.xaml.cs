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
    /// Interaction logic for MultiLaserControlView.xaml
    /// </summary>
    public partial class MultiLaserControlView : UserControl
    {
        #region Constructors

        public MultiLaserControlView()
        {
            InitializeComponent();
        }

        #endregion Constructors
    }

    public class PercentConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            //Updates the min/max of each laser each time called
            double slider1Max = (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser1Max", (object)0.0];
            double slider1Min = (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser1Min", (object)0.0];
            double slider2Max = (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser2Max", (object)0.0];
            double slider2Min = (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser2Min", (object)0.0];
            double slider3Max = (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser3Max", (object)0.0];
            double slider3Min = (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser3Min", (object)0.0];
            double slider4Max = (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser4Max", (object)0.0];
            double slider4Min = (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser4Min", (object)0.0];

            double dVal = Convert.ToDouble(value);
            Decimal dec = new Decimal();
            //Avoid division by 0
            slider1Max = (0 == slider1Max) ? 1 : slider1Max;
            slider2Max = (0 == slider2Max) ? 1 : slider2Max;
            slider3Max = (0 == slider3Max) ? 1 : slider3Max;
            slider4Max = (0 == slider4Max) ? 1 : slider4Max;

            switch (Convert.ToInt32(parameter))
            {
                case 1: dec = new Decimal((dVal - slider1Min) * 100 / (slider1Max - slider1Min)); break;
                case 2: dec = new Decimal((dVal - slider2Min) * 100 / (slider2Max - slider2Min)); break;
                case 3: dec = new Decimal((dVal - slider3Min) * 100 / (slider3Max - slider3Min)); break;
                case 4: dec = new Decimal((dVal - slider4Min) * 100 / (slider4Max - slider4Min)); break;
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

            //Updates the min/max of each laser each time called
            double slider1Max = (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser1Max", (object)0.0];
            double slider1Min = (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser1Min", (object)0.0];
            double slider2Max = (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser2Max", (object)0.0];
            double slider2Min = (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser2Min", (object)0.0];
            double slider3Max = (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser3Max", (object)0.0];
            double slider3Min = (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser3Min", (object)0.0];
            double slider4Max = (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser4Max", (object)0.0];
            double slider4Min = (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser4Min", (object)0.0];
            switch (Convert.ToInt32(parameter))
            {
                case 1: dec = new Decimal((dVal - 0) * (slider1Max - slider1Min) / 100 + slider1Min); break;
                case 2: dec = new Decimal((dVal - 0) * (slider2Max - slider2Min) / 100 + slider2Min); break;
                case 3: dec = new Decimal((dVal - 0) * (slider3Max - slider3Min) / 100 + slider3Min); break;
                case 4: dec = new Decimal((dVal - 0) * (slider4Max - slider4Min) / 100 + slider4Min); break;
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