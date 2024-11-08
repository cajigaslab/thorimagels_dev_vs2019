﻿namespace ThorSharedTypes
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Resources;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Xml;

    #region Enumerations

    enum Parameters
    {
        Normal, Inverted
    }

    #endregion Enumerations

    public class BooleanLogicalAndConverter : IMultiValueConverter
    {
        #region Methods

        object IMultiValueConverter.Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            foreach (object value in values)
            {
                if (value is bool && (bool)value == false)
                {
                    return false;
                }
            }
            return true;
        }

        object[] IMultiValueConverter.ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            throw new NotSupportedException("BooleanLogicalAndConverter is a OneWay converter.");
        }

        #endregion Methods
    }

    public class BooleanLogicalOrConverter : IMultiValueConverter
    {
        #region Methods

        object IMultiValueConverter.Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            foreach (object value in values)
            {
                if (value is bool && (bool)value == true)
                {
                    return true;
                }
            }
            return false;
        }

        object[] IMultiValueConverter.ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            throw new NotSupportedException("BooleanLogicalOrConverter is a OneWay converter.");
        }

        #endregion Methods
    }

    public sealed class BooleanToVisibilityConverter : IValueConverter
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
                Parameters directionParam = Parameters.Normal;
                bool directionBool = false;
                if (bool.TryParse((string)parameter, out directionBool))
                {
                    flag = directionBool ? !flag : flag;
                }
                else if (Enum.TryParse((string)parameter, out directionParam))
                {
                    flag = (directionParam == Parameters.Inverted) ? !flag : flag;
                }
            }
            return (flag) ? Visibility.Visible : Visibility.Collapsed;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var back = ((value is Visibility) && (((Visibility)value) == Visibility.Visible));
            if (parameter != null)
            {
                Parameters directionParam = Parameters.Normal;
                bool directionBool = false;
                if (bool.TryParse((string)parameter, out directionBool))
                {
                    back = directionBool ? !back : back;
                }
                else if (Enum.TryParse((string)parameter, out directionParam))
                {
                    back = (directionParam == Parameters.Inverted) ? !back : back;
                }
            }
            return back;
        }

        #endregion Methods
    }

    public sealed class BooleanToVisibilityConverter2 : IValueConverter
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
                return Visibility.Hidden;
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

    public sealed class BooleanToVisibilityConverter3 : IMultiValueConverter
    {
        #region Methods

        public object Convert(object[] value, Type targetType, object parameter, CultureInfo culture)
        {
            bool[] flag = new bool[2] { false, false };

            for (int i = 0; i < 2; i++)
            {
                if (value[i] is bool)
                {
                    flag[i] = (bool)value[i];
                }
                else if (value[i] is bool?)
                {
                    var nullable = (bool?)value[i];
                    flag[i] = nullable.GetValueOrDefault();
                }
                else if (value[i] is int)
                {
                    int temp = (int)value[i];
                    flag[i] = (0 == temp) ? false : true;
                }
            }

            if (bool.Parse((string)parameter))
            {
                if (true == flag[1])
                {
                    return (flag[0] == flag[1]) ? Visibility.Visible : Visibility.Collapsed;
                }
                else
                {
                    return (flag[0] == flag[1]) ? Visibility.Collapsed : Visibility.Visible;
                }
            }
            else
            {
                if (true == flag[1])
                {
                    return (flag[0] == flag[1]) ? Visibility.Collapsed : Visibility.Visible;
                }
                else
                {
                    return (flag[0] == flag[1]) ? Visibility.Visible : Visibility.Collapsed;
                }
            }
        }

        public object[] ConvertBack(object value, Type[] targetType, object parameter, CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class BoolToOppositeBoolConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(bool) && targetType != typeof(bool?))
                throw new InvalidOperationException("The target must be a boolean");

            return !(bool)value;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class CaptureModesEnumDescriptionValueConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var type = typeof(CaptureModes);
            var name = Enum.GetName(type, value);
            FieldInfo fi = type.GetField(name);
            var descriptionAttrib = (DescriptionAttribute)
                Attribute.GetCustomAttribute(fi, typeof(DescriptionAttribute));

            return descriptionAttrib.Description;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotSupportedException();
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
                //use parameter to display number of decimal places:
                string tmp = string.Empty;
                if (targetType == typeof(string))
                {
                    if (null != parameter)
                    {
                        tmp = "N" + Int32.Parse(parameter.ToString()).ToString();
                        return (Double.Parse(value.ToString()).ToString(tmp));
                    }
                    else
                    {
                        return (Double.Parse(value.ToString()).ToString());
                    }
                }
                else if (targetType == typeof(double))
                {
                    if (null != parameter)
                    {
                        tmp = "N" + Int32.Parse(parameter.ToString()).ToString();
                        return (Double.Parse(value.ToString()).ToString(tmp));
                    }
                    else
                    {
                        return (Double.Parse(value.ToString()));
                    }
                }
                else if (targetType == typeof(object))
                {
                    if (null != parameter)
                    {
                        tmp = "N" + Int32.Parse(parameter.ToString()).ToString();
                        return (Double.Parse(value.ToString()).ToString(tmp));
                    }
                    else
                    {
                        return (object)value.ToString();
                    }
                }
                else
                {
                    throw new InvalidOperationException("The target must be a string or double");
                }
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
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
                return null;
            }
        }

        #endregion Methods
    }

    public class DoubleScaledCultureConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            try
            {
                if (value == null) return null;
                double scaleFactor = Double.Parse(parameter.ToString());

                if (targetType == typeof(string) || targetType == typeof(object))
                {
                    double d = Double.Parse(value.ToString()) * scaleFactor;
                    return d.ToString();
                }
                else if (targetType == typeof(double))
                {
                    double d = Double.Parse(value.ToString()) * scaleFactor;
                    return d;
                }
                else
                {
                    throw new InvalidOperationException("The target must be a string or double");
                }
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
                return null;
            }
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            try
            {
                double scaleFactor = Double.Parse(parameter.ToString());

                if (targetType == typeof(string) || targetType == typeof(object))
                {
                    double d = Double.Parse(value.ToString()) / scaleFactor;
                    return d.ToString();
                }
                else if (targetType == typeof(double))
                {
                    double d = Double.Parse(value.ToString()) / scaleFactor;
                    return d;
                }
                else
                {
                    throw new InvalidOperationException("The target must be a string or double");
                }
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
                return null;
            }
        }

        #endregion Methods
    }

    public class DoubleScaledMultiConverter : IMultiValueConverter
    {
        #region Fields

        private double _scaleFactor = 1.0;

        #endregion Fields

        #region Properties

        public double ScaleFactor
        {
            get { return _scaleFactor; }
            set { _scaleFactor = value; }
        }

        #endregion Properties

        #region Methods

        object IMultiValueConverter.Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            try
            {
                ScaleFactor = Double.Parse(values[1].ToString());

                if (targetType == typeof(string) || targetType == typeof(object))
                {
                    double d = Double.Parse(values[0].ToString()) * ScaleFactor;
                    return d.ToString();
                }
                else if (targetType == typeof(double))
                {
                    double d = Double.Parse(values[0].ToString()) * ScaleFactor;
                    return d;
                }
                else
                {
                    throw new InvalidOperationException("The target must be a string or double");
                }
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
                return null;
            }
        }

        object[] IMultiValueConverter.ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            try
            {
                object[] retVals = new object[2];
                retVals[0] = value;
                retVals[1] = ScaleFactor;

                if (targetTypes[0] == typeof(string) || targetTypes[0] == typeof(object))
                {
                    double d = Double.Parse(value.ToString()) / ScaleFactor;
                    retVals[0] = d;
                }
                else if (targetTypes[0] == typeof(double))
                {
                    double d = Double.Parse(value.ToString()) / ScaleFactor;
                    retVals[0] = d;
                }
                else
                {
                    throw new InvalidOperationException("The target must be a string or double");
                }
                return retVals;
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
                return null;
            }
        }

        #endregion Methods
    }

    [ValueConversion(typeof(bool?), typeof(bool))]
    public class EnumToBooleanConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            string ParameterString = parameter as string;

            if (ParameterString == null)
                return DependencyProperty.UnsetValue;

            if (Enum.IsDefined(value.GetType(), value) == false)
                return DependencyProperty.UnsetValue;

            object paramvalue = Enum.Parse(value.GetType(), ParameterString);

            return paramvalue.Equals(value);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            string ParameterString = parameter as string;
            if (ParameterString == null)
                return DependencyProperty.UnsetValue;

            return Enum.Parse(targetType, ParameterString);
        }

        #endregion Methods
    }

    [ValueConversion(typeof(int), typeof(bool))]
    public class InverseBooleanConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
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

    public sealed class InverseVisibilityConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var flag = false;
            var outVal = (Visibility)value;
            if (parameter != null)
            {
                Parameters directionParam = Parameters.Normal;
                bool directionBool = false;
                if (bool.TryParse((string)parameter, out directionBool))
                {
                    flag = directionBool ? !flag : flag;
                }
                else if (Enum.TryParse((string)parameter, out directionParam))
                {
                    flag = (directionParam == Parameters.Inverted) ? !flag : flag;
                }
            }
            return (flag) ? (Visibility.Visible == outVal ? Visibility.Collapsed : Visibility.Visible) : outVal;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var isVisible = ((value is Visibility) && (((Visibility)value) == Visibility.Visible));
            if (parameter != null)
            {
                Parameters directionParam = Parameters.Normal;
                bool directionBool = false;
                if (bool.TryParse((string)parameter, out directionBool))
                {
                    isVisible = directionBool ? !isVisible : isVisible;
                }
                else if (Enum.TryParse((string)parameter, out directionParam))
                {
                    isVisible = (directionParam == Parameters.Inverted) ? !isVisible : isVisible;
                }
            }
            return isVisible ? value : Visibility.Collapsed;
        }

        #endregion Methods
    }

    public class NullToBooleanConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(bool))
                throw new InvalidOperationException("The target must be a boolean");

            if (null == value)
            {
                return false;
            }
            else
            {
                return true;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class NullToOpacityConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(double))
                throw new InvalidOperationException("The target must be a double");

            if (null == value)
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

    public class NullToVisibilityConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(Visibility))
                throw new InvalidOperationException("The target must be a Visibility");

            if (null == value)
            {
                return Visibility.Hidden;
            }
            else
            {
                return Visibility.Visible;
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

    public class StringToOrientationConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (!string.IsNullOrEmpty((string)value))
            {
                string orientation = value as string;

                if (orientation == "Horizontal")
                {
                    return System.Windows.Controls.Orientation.Horizontal;
                }
            }
            return System.Windows.Controls.Orientation.Vertical;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return null;
        }

        #endregion Methods
    }

    [ValueConversion(typeof(string), typeof(Visibility))]
    public class StringToVisibilityConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if ((null == parameter) ? true : (Parameters.Normal == (Parameters)Enum.Parse(typeof(Parameters), (string)parameter)))
            {
                return (string.IsNullOrEmpty((string)value)) ? Visibility.Collapsed : Visibility.Visible;
            }
            else
            {
                return (string.IsNullOrEmpty((string)value)) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return null;
        }

        #endregion Methods
    }
}