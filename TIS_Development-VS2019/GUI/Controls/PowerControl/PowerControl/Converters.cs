namespace PowerControl
{
    using System;
    using System.Collections.Generic;
    using System.Globalization;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;

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
}