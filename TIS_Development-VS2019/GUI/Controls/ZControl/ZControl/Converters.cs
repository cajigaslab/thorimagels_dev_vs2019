namespace ZControl
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows.Controls;
    using System.Windows.Data;

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