namespace CaptureSetupDll.View
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Reflection;
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

    using CaptureSetupDll.Model;
    using CaptureSetupDll.ViewModel;

    using ImageViewControl;

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
                        delegate ()
                        {
                            List<List<int>> histogramWPBP = (List<List<int>>)MVMManager.Instance["ImageViewCaptureSetupVM", "VolumeViewHistogramBPWP"];
                            int channelSelection = (int)MVMManager.Instance["ImageViewCaptureSetupVM", "VolumeViewVisibleChannels"];
                            int index = 0;

                            VolumeInterface.IsChanASelected = false;
                            VolumeInterface.IsChanBSelected = false;
                            VolumeInterface.IsChanCSelected = false;
                            VolumeInterface.IsChanDSelected = false;

                            for (int i = 0; i < CaptureSetup.MAX_CHANNELS; i++)
                            {
                                if (CaptureSetup.MAX_CHANNELS == histogramWPBP[0].Count || (channelSelection & (0x0001 << i)) > 0)
                                {
                                    switch (i)
                                    {
                                        case 0:
                                            VolumeInterface.BlackPoint0 = histogramWPBP[0][index];
                                            VolumeInterface.WhitePoint0 = histogramWPBP[1][index];
                                            VolumeInterface.IsChanASelected = true;
                                            break;
                                        case 1:
                                            VolumeInterface.BlackPoint1 = histogramWPBP[0][index];
                                            VolumeInterface.WhitePoint1 = histogramWPBP[1][index];
                                            VolumeInterface.IsChanBSelected = true;
                                            break;
                                        case 2:
                                            VolumeInterface.BlackPoint2 = histogramWPBP[0][index];
                                            VolumeInterface.WhitePoint2 = histogramWPBP[1][index];
                                            VolumeInterface.IsChanCSelected = true;
                                            break;
                                        case 3:
                                            VolumeInterface.BlackPoint3 = histogramWPBP[0][index];
                                            VolumeInterface.WhitePoint3 = histogramWPBP[1][index];
                                            VolumeInterface.IsChanDSelected = true;
                                            break;
                                    }
                                    index++;
                                }
                            }

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

            MVMManager.Instance["ImageViewCaptureSetupVM", "ColorMappingChangedAddHandler"] =  (Action<bool>)(vm_ColorMappingChanged);

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

            MVMManager.Instance["ImageViewCaptureSetupVM", "ColorMappingChangedRemoveHandler"] = (Action<bool>)(vm_ColorMappingChanged);

            vm.ZStackCaptureFinished -= new Action<bool>(vm_ZStackCaptureFinished);
            this.SizeChanged -= new SizeChangedEventHandler(VolumeControlView_SizeChanged);
        }

        #endregion Methods
    }
}