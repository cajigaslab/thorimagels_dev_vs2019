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

    /// <summary>
    /// Interaction logic for CameraControlView.xaml
    /// </summary>
    public partial class CameraControlView : UserControl
    {
        #region Constructors

        public CameraControlView()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Methods

        private void btnFullFrame_Click(object sender, RoutedEventArgs e)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

            vm.SetCameraFullFrame();
        }

        private void CameraControlView_Loaded(object sender, RoutedEventArgs e)
        {
            try
            {
                LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

                if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this) == true)
                    return;

                XmlNodeList ndList = ((LiveImageViewModel)this.DataContext).ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/CameraControlView/CmdConsolePanel");
                                
                if ((ndList != null) && (ndList.Count > 0))
                {
                    cmdConsolePanel.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }
                else
                {
                    cmdConsolePanel.Visibility = Visibility.Collapsed;
                }

                int entries = vm.ReadOutTapIndexMax - vm.ReadOutTapIndexMin + 1;

                if (entries == 1)
                {
                    spReadoutTaps.Visibility = Visibility.Collapsed;
                    spTapBalance.Visibility = Visibility.Collapsed;
                }
                else
                {
                    spReadoutTaps.Visibility = Visibility.Visible;
                    spTapBalance.Visibility = Visibility.Visible;
                }

                // update the visibility of cooling mode stack panel
                BindingExpression cm = spCoolMode.GetBindingExpression(StackPanel.VisibilityProperty);
                cm.UpdateTarget();

                // update the visibility of the NIR Boost stack panel
                BindingExpression nir = spNIRBoost.GetBindingExpression(StackPanel.VisibilityProperty);
                nir.UpdateTarget();
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
            }
        }

        private void rbExpoRange1_Checked(object sender, RoutedEventArgs e)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;
            vm.ExposureTimeSliderMax = 10;
            double m = Math.Min(vm.ExposureTimeCam0, vm.ExposureTimeSliderMax);
            vm.ExposureTimeCam0 = Math.Max(vm.ExposureTimeMin, m);
        }

        private void rbExpoRange2_Checked(object sender, RoutedEventArgs e)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;
            vm.ExposureTimeSliderMax = 100;
            double m = Math.Min(vm.ExposureTimeCam0, vm.ExposureTimeSliderMax);
            vm.ExposureTimeCam0 = Math.Max(vm.ExposureTimeMin, m);
        }

        private void rbExpoRange3_Checked(object sender, RoutedEventArgs e)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;
            vm.ExposureTimeSliderMax = 1000;
            double m = Math.Min(vm.ExposureTimeCam0, vm.ExposureTimeSliderMax);
            vm.ExposureTimeCam0 = Math.Max(vm.ExposureTimeMin, m);
        }

        private void sliderExposure_DragCompleted(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;
            //IsExposureSliderMouseCaptured must be set false before the property ExposureTimeCam0 is set
            vm.IsExposureSliderMouseCaptured = false;
            vm.ExposureTimeCam0 = ((Slider)sender).Value;
        }

        private void sliderExposure_DragStarted(object sender, System.Windows.Controls.Primitives.DragStartedEventArgs e)
        {
            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;
            vm.IsExposureSliderMouseCaptured = true;
        }

        #endregion Methods
    }
}
