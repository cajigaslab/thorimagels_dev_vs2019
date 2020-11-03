namespace CaptureSetupDll.ViewModel
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

    using CaptureSetupDll.Model;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorImageInfastructure;

    using ThorLogging;

    public class BinningConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            switch (Convert.ToInt32(value))
            {
                case 1:
                    return 0;
                case 2:
                    return 1;
                case 4:
                    return 2;
                case 8:
                    return 3;
            }
            return 0;
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            switch (Convert.ToInt32(value))
            {
                case 0:
                    return 1;
                case 1:
                    return 2;
                case 2:
                    return 4;
                case 3:
                    return 8;
            }
            return 1;
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
            bool[] flag = new bool[2]{false,false};

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

    public class BoolToOppositeBoolConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(bool))
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

    public class BrushColorConverter : IMultiValueConverter
    {
        #region Methods

        object IMultiValueConverter.Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            return new SolidColorBrush(
                Color.FromArgb(255, Convert.ToByte(values[0]),
                    Convert.ToByte(values[1]), Convert.ToByte(values[2])));
        }

        object[] IMultiValueConverter.ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            object[] retVals = new object[3];
            retVals[0] = ((SolidColorBrush)value).Color.R;
            retVals[1] = ((SolidColorBrush)value).Color.G;
            retVals[2] = ((SolidColorBrush)value).Color.B;
            return retVals;
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

    public class HistogramConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType,
            object parameter, CultureInfo culture)
        {
            double val = 0;

            //make sure the string is a well formed double before converting
            if (System.Double.TryParse(value.ToString(), out val))
            {
                return System.Convert.ToInt32((System.Convert.ToDouble(value) * 64)).ToString();
            }
            else
            {
                return value;
            }
        }

        public object ConvertBack(object value, Type targetType,
            object parameter, CultureInfo culture)
        {
            if (value.ToString() == "")
            {
                return 0;
            }

            double val = 0;

            //make sure the string is a well formed double before converting
            if (System.Double.TryParse(value.ToString(), out val))
            {
                return System.Convert.ToDouble(value) / 64;
            }
            else
            {
                return value;
            }
        }

        #endregion Methods
    }

    public class HistogramLinLogToTextConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType == typeof(object))
            {
                if (value.GetType().Equals(typeof(bool)))
                {
                    bool val = (bool)value;

                    return (true == val) ? "Log" : "Lin";
                }
            }
            else
            {
                throw new InvalidOperationException("The target must be an object");
            }

            return "Lin";
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            return null;
        }

        #endregion Methods
    }

    public class HistogramAutoManualToTextConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType == typeof(object))
            {
                if (value.GetType().Equals(typeof(bool)))
                {
                    bool val = (bool)value;

                    return (true == val) ? "Auto" : "Manual";
                }
            }
            else
            {
                throw new InvalidOperationException("The target must be an object");
            }

            return "Manual";
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            return null;
        }

        #endregion Methods
    }

    public class MagConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            switch (Convert.ToInt32(value))
            {
                case 4:
                    return 0;
                case 10:
                    return 1;
                case 20:
                    return 2;
                case 40:
                    return 3;
            }
            return 0;
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            switch (Convert.ToInt32(value))
            {
                case 0:
                    return 4;
                case 1:
                    return 10;
                case 2:
                    return 20;
                case 3:
                    return 40;
            }
            return 1;
        }

        #endregion Methods
    }

    public class MosaicConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            switch (Convert.ToInt32(value))
            {
                case 1:
                    return 0;
                case 2:
                    return 1;
                case 3:
                    return 2;
                case 4:
                    return 3;
                case 5:
                    return 4;
                case 6:
                    return 5;
                case 7:
                    return 6;
                case 8:
                    return 7;
                case 9:
                    return 8;
                case 10:
                    return 9;
                case 11:
                    return 10;
                case 12:
                    return 11;
            }
            return 0;
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            switch (Convert.ToInt32(value))
            {
                case 0:
                    return 1;
                case 1:
                    return 2;
                case 2:
                    return 3;
                case 3:
                    return 4;
                case 4:
                    return 5;
                case 5:
                    return 6;
                case 6:
                    return 7;
                case 7:
                    return 8;
                case 8:
                    return 9;
                case 9:
                    return 10;
                case 10:
                    return 11;
                case 11:
                    return 12;
            }
            return 1;
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

    public class PowerModeToVisibility : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType == typeof(Visibility))
            {
                if (value.GetType().Equals(typeof(int)))
                {
                    int val = (int)value;

                    return (0 == val) ? Visibility.Collapsed : Visibility.Visible;
                }
            }
            else
            {
                throw new InvalidOperationException("The target must be a Visibility");
            }

            return Visibility.Visible;
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            return null;
        }

        #endregion Methods
    }

    public class ROIControlDisplayConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (targetType == typeof(int))
            {
                return ((int)value + 128);
            }

            return 0;
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (targetType == typeof(int))
            {
                return ((int)value - 128);
            }

            return 0;
        }

        #endregion Methods
    }

    public class SpacingToOverlapConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            try
            {
                if (targetType == typeof(string))
                {
                    return (-1.0 * (double)value).ToString();
                }
                else if (targetType == typeof(double))
                {
                    return (-1.0 * (double)value);
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
                    return (-1.0 * Convert.ToDouble(value)).ToString();
                }
                else if (targetType == typeof(double))
                {
                    return (-1.0 * Convert.ToDouble(value));
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

    public class ZProgressPixelConverter : IMultiValueConverter
    {
        #region Methods

        object IMultiValueConverter.Convert(object[] value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            try
            {
                ProgressBar bar = (ProgressBar)value[1];

                var percentPixe = 200 / 100;

                double offset = (bar.Value - bar.Minimum) / (bar.Maximum - bar.Minimum);

                return percentPixe * offset * 100;
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
                return null;
            }
        }

        object[] IMultiValueConverter.ConvertBack(object value, Type[] targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            return null;
        }

        #endregion Methods
    }
}