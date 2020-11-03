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

namespace ImageTilerTestApp
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        public Window1()
        {
            InitializeComponent();

            //path is just the top most folder of an experiment (untitled001, etc)
            //make sure path ends in "\"
            string path = @"C:\TDI_data_set\20x25x40\";
            myControl.LoadData(path);
        }

        private void Window_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            myControl.Width = this.ActualWidth;
            myControl.Height = this.ActualHeight;
        }
    }
}
