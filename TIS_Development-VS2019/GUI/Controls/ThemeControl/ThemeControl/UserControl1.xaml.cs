namespace ThemeControl
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;

    public class Extensions
    {
        #region Fields

        public static readonly DependencyProperty OutOfRangeProperty = 
            DependencyProperty.RegisterAttached("OutOfRange", typeof(int), typeof(Extensions), new PropertyMetadata(default(int)));

        #endregion Fields

        #region Methods

        public static int GetOutOfRange(UIElement element)
        {
            return (int)element.GetValue(OutOfRangeProperty);
        }

        public static void SetOutOfRange(UIElement element, int value)
        {
            element.SetValue(OutOfRangeProperty, value);
        }

        #endregion Methods
    }

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class UserControl1 : UserControl
    {
        #region Constructors

        public UserControl1()
        {
            InitializeComponent();
        }

        #endregion Constructors
    }
}