namespace FLIMFitting.Utils
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows.Data;
    using System.Windows.Media;

    public class ColorToSolidColorBrushValueConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            SolidColorBrush brush = Brushes.Transparent;
            if (null == value)
            {
                return brush;
            }
            // For a more sophisticated converter, check also the targetType and react accordingly..
            if (value is Color)
            {
                Color color = (Color)value;
                return new SolidColorBrush(color);
            }
            return brush;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            // If necessary, here you can convert back. Check if which brush it is (if its one),
            // get its Color-value and return it.

            throw new NotImplementedException();
        }

        #endregion Methods
    }
}