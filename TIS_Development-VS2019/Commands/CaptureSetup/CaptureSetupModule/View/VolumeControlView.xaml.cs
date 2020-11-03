namespace CaptureSetupDll.View
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
    using System.Xml;

    using CaptureSetupDll.ViewModel;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for VolumeControlView.xaml
    /// </summary>
    public partial class VolumeControlView : UserControl
    {
        #region Constructors

        public VolumeControlView()
        {
            InitializeComponent();

            if (!System.ComponentModel.DesignerProperties.GetIsInDesignMode(this))
            {

                this.Loaded += new RoutedEventHandler(VolumeControlView_Loaded);
                this.Unloaded += new RoutedEventHandler(VolumeControlView_Unloaded);

            }
        }

        #endregion Constructors

        #region Methods

        void vm_ColorMappingChanged(bool obj)
        {
            VolumeInterface.UpdateVolumeColor();
        }

        void vm_ZStackCaptureFinished(bool obj)
        {
            if (true == obj)
            {
                this.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal,
                    new Action(
                        delegate()
                        {
                            VolumeInterface.DataSpacingZ = (double)MVMManager.Instance["ZControlViewModel", "VolumeSpacingZ", (object)0.0];
                            VolumeInterface.ResetView = true;
                            VolumeInterface.DataSpacingZMultiplier = 1;
                            VolumeInterface.RenderVolume();
                        }
                    )
                );

            }
        }

        void VolumeControlView_Loaded(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;

            //retrieve the hardware settings complete path and file name
            VolumeInterface.HardwareSettingsFile = ThorSharedTypes.ResourceManagerCS.GetHardwareSettingsFileString();

            VolumeInterface.ZStackCacheDirectory = (string)MVMManager.Instance["ZControlViewModel", "ZStackCacheDirectory", (object)string.Empty];
            vm.ColorMappingChanged += new Action<bool>(vm_ColorMappingChanged);
            vm.ZStackCaptureFinished += new Action<bool>(vm_ZStackCaptureFinished);
            this.SizeChanged += new SizeChangedEventHandler(VolumeControlView_SizeChanged);
        }

        void VolumeControlView_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            volumeUserControl.Width = e.NewSize.Width;
            volumeUserControl.Height = e.NewSize.Height;
        }

        void VolumeControlView_Unloaded(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            vm.ColorMappingChanged -= new Action<bool>(vm_ColorMappingChanged);
            vm.ZStackCaptureFinished -= new Action<bool>(vm_ZStackCaptureFinished);
            this.SizeChanged -= new SizeChangedEventHandler(VolumeControlView_SizeChanged);
        }

        #endregion Methods
    }
}