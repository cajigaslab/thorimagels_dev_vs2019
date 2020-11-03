namespace LightEngineControl.Converter
{
    using System;
    using System.Globalization;
    using System.Windows;
    using System.Windows.Data;

    [ValueConversion(typeof(bool), typeof(Visibility))]
    public class BooleanToVisibility : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            bool val = (bool)value;
            if (val)
                return Visibility.Visible;
            else
                return Visibility.Collapsed;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            Visibility visi = (Visibility)value;
            if ((Visibility.Hidden == visi) || (Visibility.Collapsed == visi))
                return false;
            else
                return true;
        }

        #endregion Methods
    }
}