namespace ImageViewControl
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Resources;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    public class BoolToOpacityConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(double))
                throw new InvalidOperationException("The target must be a double");

            if (false == (bool)value)
            {
                return 0.5;
            }
            else
            {
                return 1;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class PercentStringConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            try
            {
                if (targetType == typeof(string))
                {
                    double percentValue = Convert.ToDouble(value);
                    string formatedPercentValue = String.Format("{0:P0}", percentValue).Replace(" ", "").Replace(CultureInfo.CurrentCulture.NumberFormat.CurrencyGroupSeparator, "");
                    return formatedPercentValue;
                }
                else
                {
                    throw new InvalidOperationException("The target must be a string");
                }
            }
            catch (FormatException ex)
            {
                ex.ToString();
                return null;
            }
            catch (InvalidCastException ex)
            {
                ex.ToString();
                return null;
            }
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            try
            {
                if (targetType == typeof(double))
                {
                    string valueString = value.ToString();
                    valueString = valueString.Replace(System.Globalization.CultureInfo.CurrentCulture.NumberFormat.PercentSymbol, "");
                    valueString = valueString.Replace(" ", "");
                    double percentValue = double.Parse(valueString) / 100d;
                    return percentValue;
                }
                else
                {
                    throw new InvalidOperationException("The target must be a string");
                }
            }
            catch (FormatException ex)
            {
                ex.ToString();
                return null;
            }
            catch (InvalidCastException ex)
            {
                ex.ToString();
                return null;
            }
        }

        #endregion Methods
    }
}