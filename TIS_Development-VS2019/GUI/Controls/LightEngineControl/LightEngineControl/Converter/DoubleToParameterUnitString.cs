namespace LightEngineControl.Converter
{
    using System;
    using System.Globalization;
    using System.Text;
    using System.Windows;
    using System.Windows.Data;

    [ValueConversion(typeof(Double), typeof(String))]
    public class DoubleToParameterUnitString : IValueConverter
    {
        #region Methods

        public Object Convert(Object value, Type targetType, Object parameter, CultureInfo culture)
        {
            String strUnit = parameter as String;

            StringBuilder str = new StringBuilder();
            str.Append(((Double)value).ToString("F1", culture));

            if (!String.IsNullOrWhiteSpace(strUnit))
            {
                str.Append(" ");
                str.Append(strUnit);
            }

            return str.ToString();
        }

        public Object ConvertBack(Object value, Type targetType, Object parameter, CultureInfo culture)
        {
            String strVal = value as String;
            String strUnit = parameter as String;

            if (!String.IsNullOrWhiteSpace(strUnit))
            {
                String[] splitStr = strVal.Split(" ".ToCharArray(), 2, StringSplitOptions.RemoveEmptyEntries);
                strVal = splitStr[0];
            }
            Double num;
            return Double.TryParse(strVal, NumberStyles.Float, culture, out num) ? num : DependencyProperty.UnsetValue;
        }

        #endregion Methods
    }
}