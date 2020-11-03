namespace XYTileControl.Converter
{
    using System;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.Linq;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Markup;

    using ThorLogging;

    public class BooleanToVisibilityConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var flag = false;
            if (value is bool)
            {
                flag = (bool)value;
            }
            else if (value is bool?)
            {
                var nullable = (bool?)value;
                flag = nullable.GetValueOrDefault();
            }
            else if (value is int)
            {
                int temp = (int)value;
                flag = (0 == temp) ? false : true;
            }
            if (parameter != null)
            {
                if (bool.Parse((string)parameter))
                {
                    flag = !flag;
                }
            }
            if (flag)
            {
                return Visibility.Visible;
            }
            else
            {
                return Visibility.Collapsed;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var back = ((value is Visibility) && (((Visibility)value) == Visibility.Visible));
            if (parameter != null)
            {
                if ((bool)parameter)
                {
                    back = !back;
                }
            }
            return back;
        }

        #endregion Methods
    }

    public class DoubleCultureConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            try
            {
                if (targetType == typeof(string))
                {
                    return (Double.Parse(value.ToString()).ToString());
                }
                else if (targetType == typeof(double))
                {
                    return (Double.Parse(value.ToString()));
                }
                else if (targetType == typeof(object))
                {
                    return (object)value.ToString();
                }
                else
                {
                    throw new InvalidOperationException("The target must be a string or double");
                }
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "XYTileControl, Double Culture Converter error. Exception thrown: " + ex.Message);
                return null;
            }
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            try
            {
                if (targetType == typeof(string))
                {
                    return (Double.Parse(value.ToString())).ToString();
                }
                else if (targetType == typeof(double))
                {
                    return (Double.Parse(value.ToString()));
                }
                else if (targetType == typeof(object))
                {
                    return (object)value.ToString();
                }
                else
                {
                    throw new InvalidOperationException("The target must be a string or double");
                }
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "XYTileControl, Convert Back error. Exception thrown: " + ex.Message);
                return null;
            }
        }

        #endregion Methods
    }

    public class FontSizeConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return (double)value + 20.0;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return (double)value - 20.0;
        }

        #endregion Methods
    }

    [ValueConversion(typeof(bool), typeof(Visibility))]
    public class InvertableBooleanToVisibilityConverter : IValueConverter
    {
        #region Enumerations

        enum Parameters
        {
            Normal, Inverted
        }

        #endregion Enumerations

        #region Methods

        public object Convert(object value, Type targetType,
            object parameter, CultureInfo culture)
        {
            var boolValue = (bool)value;
            var direction = (Parameters)Enum.Parse(typeof(Parameters), (string)parameter);

            if (direction == Parameters.Inverted)
                return !boolValue ? Visibility.Visible : Visibility.Collapsed;

            return boolValue ? Visibility.Visible : Visibility.Collapsed;
        }

        public object ConvertBack(object value, Type targetType,
            object parameter, CultureInfo culture)
        {
            return null;
        }

        #endregion Methods
    }

    public class StringToDoubleConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            double temp = 0;
            double.TryParse(value.ToString(), out temp);
            return temp;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            double temp = 0;
            double.TryParse(value.ToString(), out temp);
            return temp.ToString();
        }

        #endregion Methods
    }

    public class ValueToEqualsParameterConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return value == parameter;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return null;
        }

        #endregion Methods
    }
}