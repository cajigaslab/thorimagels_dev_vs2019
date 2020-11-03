using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;


namespace OTMControl
{
    public class TrapModeToVisibility : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(Visibility))
                throw new InvalidOperationException("The target must be a Visibility");

            int val = 0, param = 0;
            Visibility ret = Visibility.Collapsed;
            ret = (Int32.TryParse(value.ToString(), out val) && (Int32.TryParse(parameter.ToString(), out param)) && (val == param)) ? Visibility.Visible : Visibility.Collapsed;
            return ret;
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }
}
