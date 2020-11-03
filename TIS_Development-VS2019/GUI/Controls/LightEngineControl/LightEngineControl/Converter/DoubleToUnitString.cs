namespace LightEngineControl.Converter
{
    using System;
    using System.Globalization;
    using System.Windows;
    using System.Windows.Data;

    [ValueConversion(typeof(Double), typeof(String))]
    public class DoubleToUnitString : IMultiValueConverter
    {
        #region Methods

        public Object Convert(Object[] values, Type targetType, Object parameter, CultureInfo culture)
        {
            Double valueString = (Double)values[0];
            String unitString = values[1] as String;

            return String.Format(culture, "{0:F1} {1}", valueString, unitString).ToString();
        }

        public Object[] ConvertBack(Object value, Type[] targetTypes, Object parameter, CultureInfo culture)
        {
            String strVal = value as String;
            var splitStr = strVal.Split(" ".ToCharArray(), 2, StringSplitOptions.RemoveEmptyEntries);

            Object[] results = new Object[2];
            Double val;
            if (!Double.TryParse(splitStr[0] as String, NumberStyles.Float, culture, out val))
            {
                results[0] = val;
                results[1] = splitStr[1] as String;
            }
            else
            {
                results[0] = DependencyProperty.UnsetValue;
                results[1] = "";
            }
            return results;
        }

        #endregion Methods
    }
}