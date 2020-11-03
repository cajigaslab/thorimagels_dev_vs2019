namespace VtkVolumeApplication
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

    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        #region Constructors

        public Window1()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Methods

        private void btnChannel1_Click(object sender, RoutedEventArgs e)
        {
            VolumeControl.viewVolumeFromChannel(0);
        }

        private void btnChannel2_Click(object sender, RoutedEventArgs e)
        {
            VolumeControl.viewVolumeFromChannel(1);
        }

        private void btnChannel3_Click(object sender, RoutedEventArgs e)
        {
            VolumeControl.viewVolumeFromChannel(2);
        }


        private void btnChannel4_Click(object sender, RoutedEventArgs e)
        {
            VolumeControl.viewVolumeFromChannel(3);
        }

        private void btnloadAppSettings_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.FileName = "HardwareSettings";
            dlg.DefaultExt = ".xml";
            dlg.Filter = "XML Documents (.xml)|*.xml";

            Nullable<bool> result = dlg.ShowDialog();
            if (result == true)
            {
                string filename = dlg.FileName;

                VolumeControl.SetApplicationColorMapping();

                VolumeControl.HardwareSettingsFile = dlg.FileName.ToString();
                VolumeControl.ApplicationSettingsDirectory = dlg.FileName.Remove(dlg.FileName.LastIndexOf("\\"));
                //MessageBox.Show(VolumeControl.ApplicationSettingsDirectory);
            }

            VolumeControl.setToDefault();
        }

        private void btnRenderVolume_Click(object sender, RoutedEventArgs e)
        {
            VolumeControl.RenderVolume();
        }

        private void btnSelectXML_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.FileName = "Experiment";
            dlg.DefaultExt = ".xml";
            dlg.Filter = "XML Documents (.xml)|*.xml";

            Nullable<bool> result = dlg.ShowDialog();
            if (result == true)
            {
                string filename = dlg.FileName;
                VolumeControl.FolderDirectory = filename.Remove(filename.LastIndexOf("\\")); 
                //MessageBox.Show("Selected FileName: " + filename);
                VolumeControl.loadParametersFromXML(filename);
            }
        }

        //private void btnTest_Click(object sender, RoutedEventArgs e)
        //{
        //    //VolumeControl.renderVolume();
        //    string folderPath = "C:/Users/mgao/Documents/Untitled006";
        //    VolumeControl.setToDefault();
        //    VolumeControl.renderVolumeFrom(folderPath);
        //}
        private void btnShowPlanes_Click(object sender, RoutedEventArgs e)
        {
            showCroppingPlanes();
        }

        //private void textZSpacing_LostFocus(object sender, RoutedEventArgs e)
        //{
        //    VolumeControl.updateReaders();
        //}
        private void btnViewAllChannels_Click(object sender, RoutedEventArgs e)
        {
            //VolumeControl.viewVolumeAllChannels();
            VolumeControl.viewAllChannels();
        }

        private void showCroppingPlanes()
        {
            MessageBox.Show("Cropping Planes: \nX: " + VolumeControl.RenderedVolumeXMin + ", " + VolumeControl.RenderedVolumeXMax + "\nY: " +
                            VolumeControl.RenderedVolumeYMin + ", " + VolumeControl.RenderedVolumeYMax + "\nZ: " +
                            VolumeControl.RenderedVolumeZMin + ", " + VolumeControl.RenderedVolumeZMax);
        }

        private void SliderCroppingRegion_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            VolumeControl.updateCroppingRegionPlanes();
            VolumeControl.ForceWindowToRender();
            //MessageBox.Show(VolumeControl.RenderedVolumeZMax.ToString());
        }

        private void SliderTimePointIndex_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            //MessageBox.Show("TimePointIndex of " + e.NewValue.ToString() + " Selected.");
            if (VolumeControl.IsWindowRendered)
            {
                //string folderPath = "C:/Users/mgao/Documents/Untitled006";
                VolumeControl.RenderVolume();
            }
        }

        #endregion Methods
    }
}