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
using Microsoft.Win32;
using System.IO;
using LineProfileControl;

namespace CaptureSetupDll.ViewModel
{
    /// <summary>
    /// Interaction logic for PlotterControl.xaml
    /// </summary>
    public partial class LineProfileView : Window
    {
        public LineProfileView()
        {
            InitializeComponent();                
            lineProfile.HorizontalAxisTitle = "";
            lineProfile.VerticalAxisTitle = "";
            lineProfile.Title = "";    
        }  

        public void AddLineGraph(double[] dataX, double[] dataY)
        {
            lineProfile.DataX = dataX;
            lineProfile.DataY = dataY;
        }

        private void btnClose_Click(object sender, RoutedEventArgs e)
        {
            Close();          
        }

        private void btnLineWidth_Click(object sender, RoutedEventArgs e)
        {
            OverlayManager.LineWidth = int.Parse(txtLineWidth.Text);
        }
    }
}
