namespace ThorImage
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for SetupHardware.xaml
    /// </summary>
    public partial class SetupHardware : Window
    {
        #region Constructors

        public SetupHardware()
        {
            InitializeComponent();
            HSWindow.ResizeMode = (ResourceManagerCS.Instance.TabletModeEnabled) ? ResizeMode.CanResizeWithGrip : ResizeMode.NoResize;
            HSWindow.SizeToContent = (ResourceManagerCS.Instance.TabletModeEnabled) ? SizeToContent.Manual : SizeToContent.WidthAndHeight;
            if (ResourceManagerCS.Instance.TabletModeEnabled)
            {
                HSWindow.Height = HSWindow.Width = 600;
            }
        }

        #endregion Constructors

        #region Methods

        private void butOk_Click(object sender, RoutedEventArgs e)
        {
            if (hardwareSetupUC.IsXYStageReady == true)
            {
                this.DialogResult = true;
            }
            else
            {
                this.DialogResult = false;
            }
            this.Close();
            HardwareState.HwState.GetInstance(hardwareSetupUC.SelectedModalityName).ConfigPhase();
        }

        #endregion Methods
    }
}