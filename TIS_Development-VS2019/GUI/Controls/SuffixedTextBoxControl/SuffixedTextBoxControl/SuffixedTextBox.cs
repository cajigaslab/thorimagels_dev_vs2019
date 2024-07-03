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

/* The reason for using Themes\Generic.xaml can be found here: https://stackoverflow.com/questions/1228875/what-is-so-special-about-generic-xaml
 * In short, When using a custom control, WPF looks for "Generic.xaml" in the custom control's 
 *  Themes\ folder to use as the default theme. 
*/

namespace SuffixedTextBoxControl
{
    /// <summary>
    /// Emulates a text box but with an additional Suffix that is added on to the end.
    /// </summary>


    public class SuffixedTextBox : TextBox
    {
        static SuffixedTextBox()
        {
            DefaultStyleKeyProperty.OverrideMetadata(typeof(SuffixedTextBox), new FrameworkPropertyMetadata(typeof(SuffixedTextBox)));
        }

        public static readonly DependencyProperty SuffixProperty = DependencyProperty.Register("Suffix", typeof(string), typeof(SuffixedTextBox), null);

        public string Suffix { get; set; }

    }
}
