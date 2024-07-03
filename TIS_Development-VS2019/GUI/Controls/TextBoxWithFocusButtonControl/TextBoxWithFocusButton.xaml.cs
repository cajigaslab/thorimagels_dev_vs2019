using System;
using System.Collections.Generic;
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

namespace TextBoxWithFocusButtonControl
{
    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class TextBoxWithFocusButton : UserControl
    {
        public TextBoxWithFocusButton()
        {
            InitializeComponent();
        }

        public static readonly DependencyProperty TextProperty =
            DependencyProperty.Register
            (
                "Text",
                typeof(string),
                typeof(TextBoxWithFocusButton),
                new FrameworkPropertyMetadata(null)
                {
                    BindsTwoWayByDefault = true
                }
            );

        public string Text
        {
            get { return (string)GetValue(TextProperty); }
            set { SetValue(TextProperty, value); }
        }

        public static readonly DependencyProperty ButtonTextProperty =
        DependencyProperty.Register
        (
            "ButtonText",
            typeof(string),
            typeof(TextBoxWithFocusButton),
            new FrameworkPropertyMetadata(null)
            {
                BindsTwoWayByDefault = true,
                DefaultValue = "✓"
            }
        );

        public string ButtonText
        {
            get { return (string)GetValue(ButtonTextProperty); }
            set { SetValue(ButtonTextProperty, value); }
        }
    }
}
