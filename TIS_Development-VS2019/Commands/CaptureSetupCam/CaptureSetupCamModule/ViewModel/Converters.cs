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

    public class BoolToHiddenConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            return ((bool)value == true ? Visibility.Hidden : Visibility.Visible);
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
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

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
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

        #endregion Methods
    }
}