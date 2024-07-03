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

namespace UpDownButtonsControl
{
    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class UpDownButtonsControl : UserControl
    {
        public UpDownButtonsControl()
        {
            InitializeComponent();
        }

        public static readonly DependencyProperty UpTextProperty =
            DependencyProperty.Register
            (
                "UpText",
                typeof(string),
                typeof(UpDownButtonsControl),
                new FrameworkPropertyMetadata(null)
                {
                    BindsTwoWayByDefault = true,
                    DefaultValue = "+"
                }
            );

        public string UpText
        {
            get { return (string)GetValue(UpTextProperty); }
            set { SetValue(UpTextProperty, value); }
        }

        public static readonly DependencyProperty UpCommandProperty =
            DependencyProperty.Register
            (
                "UpCommand",
                typeof(ICommand),
                typeof(UpDownButtonsControl),
                new FrameworkPropertyMetadata(null)
                {
                    BindsTwoWayByDefault = false
                }
            );

        public ICommand UpCommand
        {
            get { return (ICommand)GetValue(UpCommandProperty); }
            set { SetValue(UpCommandProperty, value); }
        }

        public static readonly DependencyProperty DownTextProperty =
            DependencyProperty.Register
            (
                "DownText",
                typeof(string),
                typeof(UpDownButtonsControl),
                new FrameworkPropertyMetadata(null)
                {
                    BindsTwoWayByDefault = true,
                    DefaultValue = "-"
                }
            );

        public string DownText
        {
            get { return (string)GetValue(DownTextProperty); }
            set { SetValue(DownTextProperty, value); }
        }

        public static readonly DependencyProperty DownCommandProperty =
            DependencyProperty.Register
            (
                "DownCommand",
                typeof(ICommand),
                typeof(UpDownButtonsControl),
                new FrameworkPropertyMetadata(null)
                {
                    BindsTwoWayByDefault = false
                }
            );

        public ICommand DownCommand
        {
            get { return (ICommand)GetValue(DownCommandProperty); }
            set { SetValue(DownCommandProperty, value); }
        }
    }
}
