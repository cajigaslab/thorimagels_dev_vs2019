namespace LightEngineControl.Converter
{
    using System;
    using System.Globalization;
    using System.Windows;
    using System.Windows.Data;

    [ValueConversion(typeof(bool), typeof(Visibility))]
    public class BooleanToInvertedVisibility : IValueConverter
    {
        #region Constructors

        public BooleanToInvertedVisibility()
        {
            this.HideVisibility = Visibility.Hidden;
        }

        #endregion Constructors

        #region Properties

        public Visibility HideVisibility
        {
            get; set;
        }

        #endregion Properties

        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            bool val = (bool)value;
            if (!val)
                return Visibility.Visible;
            else
                return this.HideVisibility;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            Visibility visi = (Visibility)value;
            if (this.HideVisibility == visi)
                return true;
            else
                return false;
        }

        #endregion Methods
    }
}