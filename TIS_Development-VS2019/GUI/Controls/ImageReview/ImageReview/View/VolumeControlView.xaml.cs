namespace ImageReviewDll.View
{
    using System;
    using System.Collections.Generic;
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
    using System.Windows.Navigation;
    using System.Windows.Shapes;

    using ImageReviewDll.Model;
    using ImageReviewDll.ViewModel;

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

        void ViewModel_ColorMappingChanged(bool obj)
        {
            VolumeInterface.UpdateVolumeColor();
        }

        void VolumeControlView_Loaded(object sender, RoutedEventArgs e)
        {
            //retrieve the hardware settings and zstack cache complete path and file name
            VolumeInterface.HardwareSettingsFile = ((ImageReviewViewModel)this.DataContext).HardwareSettingPath;
            ((ImageReviewViewModel)this.DataContext).ColorMappingChanged += new Action<bool>(ViewModel_ColorMappingChanged);
            ((ImageReviewViewModel)this.DataContext).RenderVolume += new Action(VolumeControlView_RenderVolume);
            this.SizeChanged += new SizeChangedEventHandler(VolumeControlView_SizeChanged);
        }

        void VolumeControlView_RenderVolume()
        {
            List<List<int>> histogramWPBP = (List<List<int>>)MVMManager.Instance["ImageViewReviewVM", "VolumeViewHistogramBPWP"];
            int channelSelection = (int)MVMManager.Instance["ImageViewReviewVM", "VolumeViewVisibleChannels"];
            int index = 0;

            VolumeInterface.IsChanASelected = false;
            VolumeInterface.IsChanBSelected = false;
            VolumeInterface.IsChanCSelected = false;
            VolumeInterface.IsChanDSelected = false;

            for (int i = 0; i < ImageReview.MAX_CHANNELS; i++)
            {
                if ((channelSelection & (0x0001 << i)) > 0)
                {
                    switch(i)
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

            VolumeInterface.ResetView = true;
            VolumeInterface.RenderVolume();
        }

        void VolumeControlView_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            volumeMainGrid.Width = e.NewSize.Width;
            volumeMainGrid.Height = e.NewSize.Height;
        }

        void VolumeControlView_Unloaded(object sender, RoutedEventArgs e)
        {
            ((ImageReviewViewModel)this.DataContext).ColorMappingChanged -= new Action<bool>(ViewModel_ColorMappingChanged);
            ((ImageReviewViewModel)this.DataContext).RenderVolume -= new Action(VolumeControlView_RenderVolume);
            this.SizeChanged -= new SizeChangedEventHandler(VolumeControlView_SizeChanged);
        }

        #endregion Methods
    }
}