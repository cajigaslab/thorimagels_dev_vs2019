namespace RealTimeLineChart.InputValidation
{
    using System;
    using System.Collections.ObjectModel;
    using System.Globalization;
    using System.Linq;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Markup;
    using System.Xml;

    #region Enumerations

    enum Parameters
    {
        Normal, Inverted
    }

    #endregion Enumerations

    public class DoubleCultureConverter : IValueConverter
    {
        #region Methods

        /// <summary>
        /// convert double value based on current culture, and use parameter to do decimal rounding.
        /// </summary>
        /// <param name="value"></param>
        /// <param name="targetType"></param>
        /// <param name="parameter"></param>
        /// <param name="culture"></param>
        /// <returns></returns>
        object IValueConverter.Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                double tVal;
                System.Globalization.CultureInfo originalCultureInfo = (System.Globalization.CultureInfo)System.Threading.Thread.CurrentThread.CurrentCulture.Clone();
                if (0 == originalCultureInfo.NumberFormat.NumberDecimalSeparator.CompareTo(","))
                {
                    //Do math rounding if available:
                    if (!Double.TryParse(value.ToString().Replace(".", ","), out tVal))
                    {
                        throw new FormatException("Invalid input conversion");
                    }
                    if (parameter != null)
                    {
                        tVal = Math.Round(tVal, Convert.ToInt32(parameter));
                    }

                    if (targetType == typeof(string))
                    {
                        return (tVal.ToString());
                    }
                    else if (targetType == typeof(double))
                    {
                        return tVal;
                    }
                    else if (targetType == typeof(object))
                    {
                        return (object)tVal.ToString();
                    }
                    else
                    {
                        throw new FormatException("The target must be a string or double");
                    }
                }
                else
                {
                    //Do math rounding if available:
                    if (!Double.TryParse(value.ToString(), out tVal))
                    {
                        throw new FormatException("Invalid input conversion");
                    }
                    if (parameter != null)
                    {
                        tVal = Math.Round(tVal, Convert.ToInt32(parameter));
                    }

                    if (targetType == typeof(string))
                    {
                        return (tVal.ToString());
                    }
                    else if (targetType == typeof(double))
                    {
                        return (tVal);
                    }
                    else if (targetType == typeof(object))
                    {
                        return (object)tVal.ToString();
                    }
                    else
                    {
                        throw new FormatException("The target must be a string or double");
                    }
                }

            }
            catch (FormatException ex)
            {
                string str = ex.Message;
                return null;
            }
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            System.Globalization.CultureInfo originalCultureInfo = (System.Globalization.CultureInfo)System.Threading.Thread.CurrentThread.CurrentCulture.Clone();
            try
            {
                double tVal;
                System.Globalization.CultureInfo switchedCultureInfo = (System.Globalization.CultureInfo)System.Threading.Thread.CurrentThread.CurrentCulture.Clone();
                object returnValue;
                if (0 == switchedCultureInfo.NumberFormat.NumberDecimalSeparator.CompareTo(","))
                {
                    switchedCultureInfo.NumberFormat.NumberDecimalSeparator = ".";
                    System.Threading.Thread.CurrentThread.CurrentCulture = switchedCultureInfo;
                    //Do math rounding if available:
                    if (parameter != null)
                    {
                        tVal = Double.Parse(value.ToString().Replace(",", "."));
                        tVal = Math.Round(tVal, Convert.ToInt32(parameter));
                    }
                    else
                    {
                        tVal = Double.Parse(value.ToString().Replace(",", "."));
                    }

                    if (targetType == typeof(string))
                    {
                        returnValue = (Double.Parse(tVal.ToString(), CultureInfo.InvariantCulture).ToString());
                    }
                    else if (targetType == typeof(double))
                    {
                        returnValue = (Double.Parse(tVal.ToString(), CultureInfo.InvariantCulture));
                    }
                    else if (targetType == typeof(object))
                    {
                        returnValue = (object)tVal.ToString();
                    }
                    else
                    {
                        System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;

                        throw new InvalidOperationException("The target must be a string or double");
                    }
                    System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;
                }
                else
                {
                    //Do math rounding if available:
                    if (parameter != null)
                    {
                        tVal = Double.Parse(value.ToString(), CultureInfo.InvariantCulture);
                        tVal = Math.Round(tVal, Convert.ToInt32(parameter));
                    }
                    else
                    {
                        tVal = Double.Parse(value.ToString(), CultureInfo.InvariantCulture);
                    }

                    if (targetType == typeof(string))
                    {
                        returnValue = tVal.ToString();
                    }
                    else if (targetType == typeof(double))
                    {
                        returnValue = tVal;
                    }
                    else if (targetType == typeof(object))
                    {
                        returnValue = (object)tVal.ToString();
                    }
                    else
                    {
                        System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;
                        throw new InvalidOperationException("The target must be a string or double");
                    }
                }
                return returnValue;
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
                System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;
                return null;
            }
        }

        #endregion Methods
    }

    [ValueConversion(typeof(int), typeof(bool))]
    public class InverseBooleanConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType,
            object parameter, CultureInfo culture)
        {
            bool ret = true;
            switch (value.GetType().Name)
            {
                case "Boolean":
                    ret = (bool)value;
                    break;
                case "Int32":
                    ret = ((int)value == 0) ? true : false;
                    break;
            }
            if (parameter != null)
            {
                var direction = (Parameters)Enum.Parse(typeof(Parameters), (string)parameter);
                ret = (direction == Parameters.Inverted) ? !ret : ret;
            }
            return ret;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return null;
        }

        #endregion Methods
    }

    [ValueConversion(typeof(bool), typeof(Visibility))]
    public class InvertableBooleanToVisibilityConverter : IValueConverter
    {
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

    [ValueConversion(typeof(int), typeof(Visibility))]
    public class InvertableIntegralToVisibilityConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType,
            object parameter, CultureInfo culture)
        {
            var direction = (Parameters)Enum.Parse(typeof(Parameters), (string)parameter);

            if (direction == Parameters.Inverted)
                return ((int)value == 0) ? Visibility.Visible : Visibility.Collapsed;

            return ((int)value == 0) ? Visibility.Collapsed : Visibility.Visible;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return null;
        }

        #endregion Methods
    }

    public class InvertableIntToVisibilityANDConverter : IMultiValueConverter
    {
        #region Methods

        public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            bool ret = true;
            var direction = (Parameters)Enum.Parse(typeof(Parameters), (string)parameter);

            foreach (object value in values)
            {
                if (((value is int) && ((int)value == 0)) || ((value is bool) && ((bool)value == false)))
                {
                    ret = false;
                }
            }

            if (direction == Parameters.Inverted)
                return !ret ? Visibility.Visible : Visibility.Collapsed;

            return ret ? Visibility.Visible : Visibility.Collapsed;
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException("InvertableBooleanToVisibilityORConverter is a OneWay converter.");
        }

        #endregion Methods
    }

    public class InvertableVisibilityGateConverter : IMultiValueConverter
    {
        #region Methods

        public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            bool ret = true;
            var direction = (Parameters)Enum.Parse(typeof(Parameters), (string)parameter);

            //use first item as gate, true then continue, otherwise return
            if (((values[0] is int) && ((int)values[0] == 0)) || ((values[0] is bool) && ((bool)values[0] == false)))
            {
                return Visibility.Collapsed;
            }
            //AND logic for the rest
            for (int i = 1; i < values.Length; i++)
            {
                if (((values[i] is int) && ((int)values[i] == 0)) || ((values[i] is bool) && ((bool)values[i] == false)))
                {
                    ret = false;
                }
            }

            if (direction == Parameters.Inverted)
                return !ret ? Visibility.Visible : Visibility.Collapsed;

            return ret ? Visibility.Visible : Visibility.Collapsed;
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException("InvertableBooleanToVisibilityORConverter is a OneWay converter.");
        }

        #endregion Methods
    }

    [ValueConversion(typeof(XmlAttribute), typeof(string))]
    public class NumberRoundFormatter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object
            parameter, System.Globalization.CultureInfo culture)
        {
            return conversion(value, targetType, parameter, culture);
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            return conversion(value, targetType, parameter, culture);
        }

        private object conversion(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            double val = 0;
            int digit = 1;
            if (typeof(string) == value.GetType())
            {
                if (Double.TryParse((string)(value), out val) && Int32.TryParse((string)parameter, out digit))
                {
                    return Math.Round(val, digit).ToString();
                }
            }
            else if (typeof(double) == value.GetType())
            {
                return Math.Round((double)value, digit).ToString();
            }
            else if (Double.TryParse((string)((XmlAttribute)value).Value, out val) && Int32.TryParse((string)parameter, out digit))
            {
                return Math.Round(val, digit).ToString();
            }
            return val;
        }

        #endregion Methods
    }

    [ValueConversion(typeof(Collection<ValidationError>), typeof(string))]
    public class ValidationErrorsToStringConverter : MarkupExtension, IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            CultureInfo culture)
        {
            Collection<ValidationError> errors =
                value as Collection<ValidationError>;

            if (errors == null)
            {
                return string.Empty;
            }

            return string.Join("\n", (from e in errors
                                      select e.ValidationResult.ToString()).ToArray());
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            CultureInfo culture)
        {
            throw new NotImplementedException();
        }

        public override object ProvideValue(IServiceProvider serviceProvider)
        {
            return new ValidationErrorsToStringConverter();
        }

        #endregion Methods
    }
}