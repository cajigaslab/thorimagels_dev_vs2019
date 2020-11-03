namespace KuriosControl.Common
{
    using System;
    using System.Globalization;
    using System.Windows.Data;
    using System.Windows.Media;

    public class WavelengthToColorConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            double wl = 0;
            if (Double.TryParse(System.Convert.ToString(value), out wl))
            {
                return new SolidColorBrush(WavelengthHelper.Wavelength2Color((int)Math.Round(wl)));
            }

            return new SolidColorBrush(System.Windows.Media.Colors.AliceBlue);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }

        #endregion Methods
    }
}