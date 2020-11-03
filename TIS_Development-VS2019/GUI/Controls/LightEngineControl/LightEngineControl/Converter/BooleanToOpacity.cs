namespace LightEngineControl.Converter
{
    using System;
    using System.Globalization;
    using System.Windows.Data;

    [ValueConversion(typeof(bool), typeof(double))]
    public class BooleanToOpacity : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            bool val = (bool)value;
            if (val)
                return 1.0;
            else
                return 0.5;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            double opa = (double)value;
            if (0.5 >= opa)
                return false;
            else
                return true;
        }

        #endregion Methods
    }
}