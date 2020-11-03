using System.Windows.Controls;
using OverlayManagerTest.Commands;

namespace OverlayManagerTest.Controls
{
    /// <summary>
    /// Interaction logic for ToggleButtonControl.xaml
    /// </summary>
    public partial class ToggleButtonControl : UserControl
    {
        public ToggleButtonControl()
        {
            InitializeComponent();

            //listening to ReturnValueChanged event and setting the toggle button state correspondingly
            ((CustomCommand)Commands.Commands.ToggleCommand).ReturnValueChanged += (sender, e) =>
            {
                toggleBtn.IsChecked = (bool)((CustomCommand)Commands.Commands.ToggleCommand).ReturnValue;
            };
        }
    }
}