namespace ROIStatsChart.ViewModel
{
    using System;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Media;

    public class FifoVisibleConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if ((targetType != typeof(Visibility)) && (targetType != typeof(Double)) && (targetType != typeof(bool)))
                throw new InvalidOperationException("The target must be a Visibility or Double or bool");
            if(targetType == typeof(Visibility))
            {
                Visibility ret = Visibility.Collapsed;

                switch (System.Convert.ToBoolean(value))
                {
                    case false: //Fifo visible false
                        {
                            ret = (0 == System.Convert.ToInt32(parameter)) ? Visibility.Collapsed : Visibility.Visible;
                        }
                        break;
                    case true:  //Fifo visible true
                        {
                            ret = (1 == System.Convert.ToInt32(parameter)) ? Visibility.Collapsed : Visibility.Visible;
                        }
                        break;
                    default:
                        {
                        }
                        break;
                }

                return ret;
            }
            else if (targetType == typeof(Double))
            {
                switch (System.Convert.ToBoolean(value))
                {
                    case false: //Fifo visible false
                        {
                            return System.Convert.ToDouble(parameter, culture);
                        }
                    case true:  //Fifo visible true
                        {
                            return 0;
                        }
                    default:
                        {
                            return null;
                        }
                }
            }
            else if (targetType == typeof(bool))
            {
                bool ret = true;

                switch (System.Convert.ToBoolean(value))
                {
                    case false: //Fifo visible false
                        {
                            ret = (0 == System.Convert.ToInt32(parameter)) ? false :true;
                            break;
                        }
                    case true:  //Fifo visible true
                        {
                            ret = (1 == System.Convert.ToInt32(parameter)) ? false : true;
                            break;
                        }
                    default:
                        {
                            ret = false;
                            break;
                        }
                }

                return ret;
            }
            else
            {
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }
}