namespace LightEngineControl.Converter
{
    using System;
    using System.Globalization;
    using System.Windows.Data;
    using System.Windows.Media;

    [ValueConversion(typeof(bool), typeof(SolidColorBrush))]
    public class BooleanToTransparentBrush : IMultiValueConverter
    {
        #region Methods

        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            bool isChecked = false;
            try
            {
                isChecked = (bool)values[0];
            }
            catch (InvalidCastException icEx)
            {
                isChecked = false;
                Console.WriteLine(icEx.ToString());
            }

            SolidColorBrush trackFill = (values[1] != null) ? values[1] as SolidColorBrush : Brushes.DarkGray;
            try
            {
                trackFill = values[1] as SolidColorBrush;
            }
            catch (InvalidCastException icEx)
            {
                trackFill = Brushes.DarkGray;
                Console.WriteLine(icEx.ToString());
            }

            if (!isChecked)
            {
                trackFill = Brushes.Transparent;
            }

            return trackFill;
        }

        public object[] ConvertBack(object value, Type[] targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }

        #endregion Methods
    }
}